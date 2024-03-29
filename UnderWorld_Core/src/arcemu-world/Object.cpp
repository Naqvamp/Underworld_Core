/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2010 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Unit.h"
using namespace std;

//#define DEG2RAD (M_PI/180.0)
#define M_PI		3.14159265358979323846
#define M_H_PI		1.57079632679489661923
#define M_Q_PI		0.785398163397448309615

void Object::SetRotation( uint64 guid )
{
	WorldPacket data(SMSG_AI_REACTION, 12);
	data << guid;
	data << uint32(2);
	SendMessageToSet(&data, false);
}

Object::Object() : m_position(0,0,0,0), m_spawnLocation(0,0,0,0)
{
	m_mapId = 0;
	m_zoneId = 0;

	m_uint32Values = 0;
	m_objectUpdated = false;


	m_valuesCount = 0;

	//official Values
	m_walkSpeed = 2.5f;
	m_runSpeed = 7.0f;
	m_base_runSpeed = m_runSpeed;
	m_base_walkSpeed = m_walkSpeed;

	m_flySpeed = 7.0f;
	m_backFlySpeed = 4.5f;

	m_backWalkSpeed = 4.5f;	// this should really be named m_backRunSpeed
	m_swimSpeed = 4.722222f;
	m_backSwimSpeed = 2.5f;
	m_turnRate = 3.141593f;

	m_phase = 1; //Set the default phase: 00000000 00000000 00000000 00000001

	m_mapMgr = 0;
	m_mapCell = 0;

	mSemaphoreTeleport = false;


	m_faction = NULL;
	m_factionDBC = NULL;

	m_instanceId = 0;
	Active = false;
	m_inQueue = false;
	m_extensions = NULL;
	m_loadedFromDB = false;

	m_faction = dbcFactionTemplate.LookupRow( 0 );
	m_factionDBC = dbcFaction.LookupRow( 0 );

	m_objectsInRange.clear();
	m_inRangePlayers.clear();
	m_oppFactsInRange.clear();
	m_sameFactsInRange.clear();

	Active = false;
}

Object::~Object( )
{
	if(m_objectTypeId != TYPEID_ITEM)
		Arcemu::Util::ARCEMU_ASSERT(   !m_inQueue);

	Arcemu::Util::ARCEMU_ASSERT( !IsInWorld() );

	// for linux
	m_instanceId = -1;
	m_objectTypeId=TYPEID_UNUSED;

	if( m_extensions != NULL )
		delete m_extensions;

	//avoid leaving traces in eventmanager. Have to work on the speed. Not all objects ever had events so list iteration can be skipped
	sEventMgr.RemoveEvents( this );
}


void Object::_Create( uint32 mapid, float x, float y, float z, float ang )
{
	m_mapId = mapid;
	m_position.ChangeCoords(x, y, z, ang);
	m_spawnLocation.ChangeCoords(x, y, z, ang);
	m_lastMapUpdatePosition.ChangeCoords(x,y,z,ang);
}

uint32 Object::BuildCreateUpdateBlockForPlayer(ByteBuffer *data, Player *target)
{
	uint16 flags = 0;
	uint32 flags2 = 0;

	uint8 updatetype = UPDATETYPE_CREATE_OBJECT;
	if(m_objectTypeId == TYPEID_CORPSE)
	{
		if(static_cast<Corpse*>(this)->GetDisplayId() == 0)
			return 0;
		updatetype = UPDATETYPE_CREATE_YOURSELF;
	}

	// any other case
	switch(m_objectTypeId)
	{
		// items + containers: 0x8
	case TYPEID_ITEM:
	case TYPEID_CONTAINER:
		flags = 0x10;
		break;

		// player/unit: 0x68 (except self)
	case TYPEID_UNIT:
		flags = 0x70;
		break;

	case TYPEID_PLAYER:
		flags = 0x70;
		break;

		// gameobject/dynamicobject
	case TYPEID_GAMEOBJECT:
		flags = 0x0350;
		if( static_cast<GameObject*>(this)->GetDisplayId()==3831 ) flags= 0x0252; //Deeprun Tram proper flags as of 3.2.0.
		break;

	case TYPEID_DYNAMICOBJECT:
		flags = 0x0150;
		break;

	case TYPEID_CORPSE:
		flags = 0x0150;
		break;
		// anyone else can get fucked and die!
	}

	if(target == this)
	{
		// player creating self
		flags |= 0x01;
		updatetype = UPDATETYPE_CREATE_YOURSELF;
	}

	// gameobject stuff
	if(m_objectTypeId == TYPEID_GAMEOBJECT)
	{
//		switch( GetByte(GAMEOBJECT_BYTES_1,GAMEOBJECT_BYTES_TYPEID) )
		switch(m_uint32Values[GAMEOBJECT_BYTES_1])
		{
			case GAMEOBJECT_TYPE_MO_TRANSPORT:  
				{
					if(GetTypeFromGUID() != HIGHGUID_TYPE_TRANSPORTER)
						return 0;   // bad transporter
					else
						flags = 0x0352;
				}break;

			case GAMEOBJECT_TYPE_TRANSPORT:
				{
					/* deeprun tram, etc */
					flags = 0x252;
				}break;

			case GAMEOBJECT_TYPE_DUEL_ARBITER:
				{
					// duel flags have to stay as updatetype 3, otherwise
					// it won't animate
					updatetype = UPDATETYPE_CREATE_YOURSELF;
				}break;
		}
		//The above 3 checks FAIL to identify transports, thus their flags remain 0x58, and this is BAAAAAAD! Later they don't get position x,y,z,o updates, so they appear randomly by a client-calculated path, they always face north, etc... By: VLack
		if( flags != 0x0352 && GetTypeId() == TYPEID_GAMEOBJECT && static_cast<GameObject*>(this)->GetInfo()->Type == GAMEOBJECT_TYPE_TRANSPORT && !(static_cast<GameObject*>(this)->GetOverrides() & GAMEOBJECT_OVERRIDE_PARENTROT) )
			flags = 0x0352;
	}

	// build our actual update
	*data << updatetype;

	// we shouldn't be here, under any circumstances, unless we have a wowguid..
	Arcemu::Util::ARCEMU_ASSERT(   m_wowGuid.GetNewGuidLen() > 0);
	*data << m_wowGuid;
	
	*data << m_objectTypeId;

	_BuildMovementUpdate(data, flags, flags2, target);

	// we have dirty data, or are creating for ourself.
	UpdateMask updateMask;
	updateMask.SetCount( m_valuesCount );
	_SetCreateBits( &updateMask, target );

	if(GetTypeId() == TYPEID_GAMEOBJECT && (static_cast<GameObject*>(this)->GetOverrides() & GAMEOBJECT_OVERRIDE_PARENTROT) )
	{
		updateMask.SetBit(GAMEOBJECT_PARENTROTATION_02);
		updateMask.SetBit(GAMEOBJECT_PARENTROTATION_03);
	}

	// this will cache automatically if needed
	_BuildValuesUpdate( data, &updateMask, target );

	// update count: 1 ;)
	return 1;
}


//That is dirty fix it actually creates update of 1 field with
//the given value ignoring existing changes in fields and so on
//useful if we want update this field for certain players
//NOTE: it does not change fields. This is also very fast method
WorldPacket *Object::BuildFieldUpdatePacket( uint32 index,uint32 value)
{
   // uint64 guidfields = GetGUID();
   // uint8 guidmask = 0;
	WorldPacket * packet=new WorldPacket(1500);
	packet->SetOpcode( SMSG_UPDATE_OBJECT );

	*packet << (uint32)1;//number of update/create blocks
//	*packet << (uint8)0;//unknown //VLack: removed for 3.1

	*packet << (uint8) UPDATETYPE_VALUES;		// update type == update
	*packet << GetNewGUID();

	uint32 mBlocks = index/32+1;
	*packet << (uint8)mBlocks;

	for(uint32 dword_n=mBlocks-1;dword_n;dword_n--)
	*packet <<(uint32)0;

	*packet <<(((uint32)(1))<<(index%32));
	*packet << value;

	return packet;
}

void Object::BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value)
{
	ByteBuffer buf(500);
	buf << uint8(UPDATETYPE_VALUES);
	buf << GetNewGUID();

	uint32 mBlocks = Index/32+1;
	buf << (uint8)mBlocks;

	for(uint32 dword_n=mBlocks-1;dword_n;dword_n--)
		buf <<(uint32)0;

	buf <<(((uint32)(1))<<(Index%32));
	buf << Value;

	Target->PushUpdateData(&buf, 1);
}

void Object::BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value)
{
	*buf << uint8(UPDATETYPE_VALUES);
	*buf << GetNewGUID();

	uint32 mBlocks = Index/32+1;
	*buf << (uint8)mBlocks;

	for(uint32 dword_n=mBlocks-1;dword_n;dword_n--)
		*buf <<(uint32)0;

	*buf <<(((uint32)(1))<<(Index%32));
	*buf << Value;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer *data, Player *target)
{
	UpdateMask updateMask;
	updateMask.SetCount( m_valuesCount );
	_SetUpdateBits( &updateMask, target );
	for(uint32 x = 0; x < m_valuesCount; ++x)
	{
		if(updateMask.GetBit(x))
		{
			*data << (uint8) UPDATETYPE_VALUES;		// update type == update
			Arcemu::Util::ARCEMU_ASSERT(   m_wowGuid.GetNewGuidLen() > 0);
			*data << m_wowGuid;

			_BuildValuesUpdate( data, &updateMask, target );
			return 1;
		}
	}

	return 0;
}

uint32 Object::BuildValuesUpdateBlockForPlayer(ByteBuffer * buf, UpdateMask * mask )
{
	// returns: update count
	// update type == update
	*buf << (uint8) UPDATETYPE_VALUES;		

	Arcemu::Util::ARCEMU_ASSERT(   m_wowGuid.GetNewGuidLen() > 0);
	*buf << m_wowGuid;

	_BuildValuesUpdate( buf, mask, 0 );

	// 1 update.
	return 1;
}

void Object::DestroyForPlayer(Player * target) const
{
	// If the target doesn't exist, return. If the target exists, but it session is null return
	 if( target == NULL || target->GetSession() == NULL )
		return;

	Arcemu::Util::ARCEMU_ASSERT(   target != NULL );

	WorldPacket data( SMSG_DESTROY_OBJECT, 9 );
	data << GetGUID();
	data << uint8( 0 ); //TODO: unk bool

	target->GetSession()->SendPacket( &data );
}


///////////////////////////////////////////////////////////////
/// Build the Movement Data portion of the update packet
/// Fills the data with this object's movement/speed info
/// TODO: rewrite this stuff, document unknown fields and flags
uint32 TimeStamp();

void Object::_BuildMovementUpdate(ByteBuffer * data, uint16 flags, uint32 flags2, Player* target )
{
	/* ByteBuffer *splinebuf = (m_objectTypeId == TYPEID_UNIT) ? target->GetAndRemoveSplinePacket(GetGUID()) : 0; */
	uint16 flag16 = 0;	// some other flag

	*data << (uint16)flags;

	Player * pThis = NULL;
	MovementInfo* moveinfo = NULL;
	if(GetTypeId() == TYPEID_PLAYER)
	{
		pThis = static_cast< Player* >( this );
		if(pThis->GetSession())
			moveinfo = pThis->GetSession()->GetMovementInfo();
		if(target == this)
		{
			// Updating our last speeds.
			pThis->UpdateLastSpeeds();
		}
	}
	Creature * uThis = NULL;
	if (GetTypeId() == TYPEID_UNIT)
		uThis = static_cast<Creature*>(this);

	if (flags & UPDATEFLAG_LIVING) //0x20
	{
		if(pThis && pThis->m_TransporterGUID != 0)
			flags2 |= MOVEFLAG_TAXI; //0x200
		else if(uThis != NULL && uThis->m_transportGuid != 0 && uThis->m_transportPosition != NULL)
			flags2 |= MOVEFLAG_TAXI; //0x200

		if(uThis != NULL)
		{
			//		 Don't know what this is, but I've only seen it applied to spirit healers.
			//		 maybe some sort of invisibility flag? :/
			switch(GetEntry())
			{
			case 6491:  // Spirit Healer
			case 13116: // Alliance Spirit Guide
			case 13117: // Horde Spirit Guide
				{
					flags2 |= MOVEFLAG_WATER_WALK; //0x10000000
				}break;
			}

			if(uThis->GetAIInterface()->IsFlying())
				flags2 |= MOVEFLAG_NO_COLLISION; //0x400 Zack : Teribus the Cursed had flag 400 instead of 800 and he is flying all the time 
			if(uThis->GetProto() && uThis->GetProto()->extra_a9_flags)
			{
//do not send shit we can't honor
#define UNKNOWN_FLAGS2 ( 0x00002000 | 0x04000000 | 0x08000000 )
				uint32 inherit = uThis->GetProto()->extra_a9_flags & UNKNOWN_FLAGS2;
				flags2 |= inherit;
			}
		}

		*data << (uint32)flags2;

		*data << (uint16)flag16;

		*data << getMSTime(); // this appears to be time in ms but can be any thing. Maybe packet serializer ?

		// this stuff:
		//   0x01 -> Enable Swimming?
		//   0x04 -> ??
		//   0x10 -> disables movement compensation and causes players to jump around all the place
		//   0x40 -> disables movement compensation and causes players to jump around all the place

		//Send position data, every living thing has these
		*data << (float)m_position.x;
		*data << (float)m_position.y;
		*data << (float)m_position.z;
		*data << (float)m_position.o;

		if(flags2 & MOVEFLAG_TAXI) //0x0200
		{
			if(pThis)
			{
				WoWGuid wowguid(pThis->m_TransporterGUID);
				*data << wowguid;
				*data << pThis->m_TransporterX << pThis->m_TransporterY << pThis->m_TransporterZ << pThis->m_TransporterO;
				*data << pThis->m_TransporterUnk;
				*data << (uint8)0;
			}
			else if(uThis != NULL && uThis->m_transportPosition != NULL)
			{
				uint64 tguid = ((uint64)HIGHGUID_TYPE_TRANSPORTER << 32) | uThis->m_transportGuid;
				WoWGuid wowguid( tguid );
				*data << wowguid;
				*data << uThis->m_transportPosition->x << uThis->m_transportPosition->y <<
					uThis->m_transportPosition->z << uThis->m_transportPosition->o;
				*data << uint32(0);
				*data << uint8(0);
			}
		}

		if( (flags2 & (MOVEFLAG_SWIMMING | MOVEFLAG_AIR_SWIMMING)) || (flag16 & 0x20) ) // 0x2000000+0x0200000 flying/swimming, && unk sth to do with vehicles?
		{
			if(pThis && moveinfo)
				*data << moveinfo->pitch;
			else 
				*data << (float)0; //pitch
		}

		if(pThis && moveinfo)
			*data << moveinfo->unklast;
		else
			*data << (uint32)0; //last fall time

		if( flags2 & MOVEFLAG_JUMPING ) // 0x00001000
		{

				*data << (float)0;
				*data << (float)1.0;
				*data << (float)0;
				*data << (float)0;			
		}

		if( flags2 & MOVEFLAG_SPLINE_MOVER ) // 0x4000000
		{
			int err1, err2; err2= 0; err1=10/err2; //FAIL please with divide by zero :)
		}

		if( m_walkSpeed == 0 )
			*data << 8.0f;
		else
			*data << m_walkSpeed;	// walk speed
		if( m_runSpeed == 0 )
			*data << 8.0f;
		else
			*data << m_runSpeed;	// run speed
		*data << m_backWalkSpeed;	// backwards walk speed
		*data << m_swimSpeed;		// swim speed
		*data << m_backSwimSpeed;	// backwards swim speed
		if( m_flySpeed == 0 )
			*data << 8.0f;
		else
			*data << m_flySpeed;	// fly speed
		*data << m_backFlySpeed;	// back fly speed
		*data << m_turnRate;		// turn rate
		*data << float(7);		// pitch rate, now a constant...

		if( flags2 & 0x08000000 ) //VLack: On Mangos this is a nice spline movement code, but we never had such... Also, at this point we haven't got this flag, that's for sure, but fail just in case...
		{
			int err1, err2; err2= 0; err1=10/err2; //FAIL please with divide by zero :)
		}
	}
	else //----------------------------------- No UPDATEFLAG_LIVING -----------------------------------
	{
		if( flags & UPDATEFLAG_POSITION ) //0x0100
		{
			*data << uint8( 0 ); //some say it is like parent guid ?
			*data << (float)m_position.x;
			*data << (float)m_position.y;
			*data << (float)m_position.z;
			*data << (float)m_position.x;
			*data << (float)m_position.y;
			*data << (float)m_position.z;
			*data << (float)m_position.o;

			if (m_objectTypeId == TYPEID_CORPSE)
				*data << (float)m_position.o; //VLack: repeat the orientation!
			else
				*data << (float)0;
		}
		else if (flags & UPDATEFLAG_HAS_POSITION) //0x40
		{
			if(flags & UPDATEFLAG_TRANSPORT && m_uint32Values[GAMEOBJECT_BYTES_1]==GAMEOBJECT_TYPE_MO_TRANSPORT)
			{
				*data << (float)0;
				*data << (float)0;
				*data << (float)0;
			}
			else
			{
				*data << (float)m_position.x;
				*data << (float)m_position.y;
				*data << (float)m_position.z;
			}
			*data << (float)m_position.o;
		}
	}


	if( flags & UPDATEFLAG_LOWGUID ) //0x08
		*data << GetLowGUID();

	if( flags & UPDATEFLAG_HIGHGUID ) //0x10
		*data << GetHighGUID();

	if( flags & UPDATEFLAG_HAS_TARGET ) //0x04
	{
		if (IsUnit())
			FastGUIDPack(*data, TO_UNIT(this)->GetTargetGUID());	//some compressed GUID
		else
			*data << uint64( 0 );
	}


	if( flags & UPDATEFLAG_TRANSPORT ) //0x2
	{
        *data << getMSTime();
    }
	if( flags & UPDATEFLAG_VEHICLE ) //0x80
	{
		*data << uint32( 0 ); //Vehicle ID
		*data << float( 0 ); //Facing
	}

	if( flags & UPDATEFLAG_ROTATION ) //0x0200
	{
		if( IsGameObject() )
			*data << static_cast< GameObject* >( this )->m_rotation;
		else
			*data << uint64( 0 ); //?
	}
}


//=======================================================================================
//  Creates an update block with the values of this object as
//  determined by the updateMask.
//=======================================================================================
void Object::_BuildValuesUpdate(ByteBuffer * data, UpdateMask *updateMask, Player* target)
{
	bool activate_quest_object = false;
	bool reset = false;
	uint32 oldflags = 0;

	if(updateMask->GetBit(OBJECT_FIELD_GUID) && target)	   // We're creating.
	{
		Creature * pThis = static_cast<Creature*>(this);
		if(GetTypeId() == TYPEID_UNIT && pThis->Tagged && (pThis->loot.gold || pThis->loot.items.size()))
		{
			// Let's see if we're the tagger or not.
			oldflags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
			uint32 Flags = m_uint32Values[UNIT_DYNAMIC_FLAGS];
			uint32 oldFlags = 0;

			if(pThis->TaggerGuid == target->GetGUID())
			{
				// Our target is our tagger.
				oldFlags = U_DYN_FLAG_TAGGED_BY_OTHER;

				if(Flags & U_DYN_FLAG_TAGGED_BY_OTHER)
					Flags &= ~oldFlags;

				if( !(Flags & U_DYN_FLAG_LOOTABLE) && pThis->HasLootForPlayer( target ) )
					Flags |= U_DYN_FLAG_LOOTABLE;
			}
			else
			{
				// Target is not the tagger.
				oldFlags = U_DYN_FLAG_LOOTABLE;

				if(!(Flags & U_DYN_FLAG_TAGGED_BY_OTHER))
					Flags |= U_DYN_FLAG_TAGGED_BY_OTHER;

				if(Flags & U_DYN_FLAG_LOOTABLE)
					Flags &= ~oldFlags;
			}

			m_uint32Values[UNIT_DYNAMIC_FLAGS] = Flags;

			updateMask->SetBit(UNIT_DYNAMIC_FLAGS);

			reset = true;
		}

		if( target && GetTypeId() == TYPEID_GAMEOBJECT )
		{
			GameObject *go = ((GameObject*)this);
			QuestLogEntry *qle;
			GameObjectInfo *info;
			if( go->HasQuests() )
			{
				std::list<QuestRelation *>::iterator itr;
				for( itr = go->QuestsBegin(); itr != go->QuestsEnd(); ++itr ) {
					QuestRelation * qr = (*itr);
					if( qr != NULL )
					{
						Quest * qst = qr->qst;
						if( qst != NULL )
						{
							if(		( qr->type & QUESTGIVER_QUEST_START && !target->HasQuest(qst->id) ) 
								||	( qr->type & QUESTGIVER_QUEST_END && target->HasQuest(qst->id) )
								)
							{
								activate_quest_object = true;
								break;
							}
						}
					}
				}
			}
			else
			{
				info = go->GetInfo();
				if( info && ( info->goMap.size() || info->itemMap.size() ) )
				{
					for( GameObjectGOMap::iterator itr = go->GetInfo()->goMap.begin(); itr != go->GetInfo()->goMap.end(); ++itr )
					{
						qle = target->GetQuestLogForEntry( itr->first->id );
						if( qle != NULL )
						{
							if( qle->GetQuest()->count_required_mob == 0 )
								continue;
							for( uint32 i = 0; i < 4; ++i )
							{
								if( qle->GetQuest()->required_mob[i] == static_cast<int32>( go->GetEntry() ) && qle->GetMobCount(i) < qle->GetQuest()->required_mobcount[i])
								{
									activate_quest_object = true;
									break;
								}
							}
							if( activate_quest_object )
								break;
						}
					}

					if( !activate_quest_object )
					{
						for( GameObjectItemMap::iterator itr = go->GetInfo()->itemMap.begin();
							itr != go->GetInfo()->itemMap.end();
							++itr )
						{
							for( std::map<uint32, uint32>::iterator it2 = itr->second.begin();
								it2 != itr->second.end();
								++it2 )
							{
								if( ( qle = target->GetQuestLogForEntry( itr->first->id ) ) != 0 )
								{
									if( target->GetItemInterface()->GetItemCount(it2->first) < it2->second )
									{
										activate_quest_object = true;
										break;
									}
								}
							}
							if( activate_quest_object )
								break;
						}
					}
				}
			}
		}
	}


	if(activate_quest_object)
	{
		oldflags = m_uint32Values[GAMEOBJECT_DYNAMIC];
		if(!updateMask->GetBit(GAMEOBJECT_DYNAMIC))
			updateMask->SetBit(GAMEOBJECT_DYNAMIC);
		m_uint32Values[GAMEOBJECT_DYNAMIC] = 1 | 8; // 8 to show sparkles
		reset = true;
	}

	Arcemu::Util::ARCEMU_ASSERT(    updateMask && updateMask->GetCount()  == m_valuesCount );
	uint32 bc;
	uint32 values_count;
	if( m_valuesCount > ( 2 * 0x20 ) )//if number of blocks > 2->  unit and player+item container
	{
		bc = updateMask->GetUpdateBlockCount();
		values_count = min<uint32>( bc * 32, m_valuesCount );

	}
	else
	{
		bc=updateMask->GetBlockCount();
		values_count=m_valuesCount;
	}

	*data << (uint8)bc;
	data->append( updateMask->GetMask(), bc*4 );

	for( uint32 index = 0; index < values_count; index ++ )
	{
		if( updateMask->GetBit( index ) )
		{
			*data << m_uint32Values[ index ];
		}
	}

	if(reset)
	{
		switch(GetTypeId())
		{
		case TYPEID_UNIT:
			m_uint32Values[UNIT_DYNAMIC_FLAGS] = oldflags;
			break;
		case TYPEID_GAMEOBJECT:
			m_uint32Values[GAMEOBJECT_DYNAMIC] = oldflags;
			break;
		}
	}
}

void Object::BuildHeartBeatMsg(WorldPacket *data) const
{
	data->Initialize(MSG_MOVE_HEARTBEAT);

	*data << GetGUID();

	*data << uint32(0); // flags
//	*data << uint32(0); // mysterious value #1
	*data << getMSTime();
	*data << m_position;
	*data << m_position.o;
}

bool Object::SetPosition(const LocationVector & v, bool allowPorting /* = false */)
{
	bool updateMap = false, result = true;

	if (m_position.x != v.x || m_position.y != v.y)
		updateMap = true;

	m_position = const_cast<LocationVector&>(v);

	if (!allowPorting && v.z < -500)
	{
		m_position.z = 500;
		sLog.outError( "setPosition: fell through map; height ported" );

		result = false;
	}

	if (IsInWorld() && updateMap)
	{
		m_mapMgr->ChangeObjectLocation(this);
	}

	return result;
}

bool Object::SetPosition( float newX, float newY, float newZ, float newOrientation, bool allowPorting )
{
	bool updateMap = false, result = true;

	//It's a good idea to push through EVERY transport position change, no matter how small they are. By: VLack aka. VLsoft
	if( GetTypeId() == TYPEID_GAMEOBJECT && static_cast<GameObject*>(this)->GetInfo()->Type == GAMEOBJECT_TYPE_TRANSPORT )
		updateMap = true;

	//if (m_position.x != newX || m_position.y != newY)
		//updateMap = true;
	if(m_lastMapUpdatePosition.Distance2DSq(newX, newY) > 4.0f)		/* 2.0f */
		updateMap = true;

	m_position.ChangeCoords(newX, newY, newZ, newOrientation);

	if (!allowPorting && newZ < -500)
	{
		m_position.z = 500;
		sLog.outError( "setPosition: fell through map; height ported" );

		result = false;
	}

	if (IsInWorld() && updateMap)
	{
		m_lastMapUpdatePosition.ChangeCoords(newX,newY,newZ,newOrientation);
		m_mapMgr->ChangeObjectLocation(this);

		if( m_objectTypeId == TYPEID_PLAYER && static_cast< Player* >( this )->GetGroup() && static_cast< Player* >( this )->m_last_group_position.Distance2DSq(m_position) > 25.0f ) // distance of 5.0
		{
            static_cast< Player* >( this )->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_POSITION, static_cast< Player* >( this ) );
		}
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////
/// Fill the object's Update Values from a space delimitated list of values.
void Object::LoadValues(const char* data)
{
	// thread-safe ;) strtok is not.
	std::string ndata = data;
	std::string::size_type last_pos = 0, pos = 0;
	uint32 index = 0;
	uint32 val;
	do
	{
		// prevent overflow
		if(index >= m_valuesCount)
		{
			break;
		}
		pos = ndata.find(" ", last_pos);
		val = atol(ndata.substr(last_pos, (pos-last_pos)).c_str());
		if(m_uint32Values[index] == 0)
			m_uint32Values[index] = val;
		last_pos = pos+1;
		++index;
	} while(pos != std::string::npos);
}

void Object::_SetUpdateBits(UpdateMask *updateMask, Player *target) const
{
	*updateMask = m_updateMask;
}


void Object::_SetCreateBits(UpdateMask *updateMask, Player *target) const
{

	for(uint32 i = 0; i < m_valuesCount; ++i)
		if(m_uint32Values[i] != 0)
			updateMask->SetBit(i);
}

void Object::AddToWorld()
{
	MapMgr *mapMgr = sInstanceMgr.GetInstance(this);
	if(!mapMgr)
		return;

	if( IsPlayer() )
	{
		Player *plr = static_cast< Player* >( this );
		if(mapMgr->pInstance != NULL && plr->GetSession()->m_gmData->rank < RANK_PLAT)
		{
			// Player limit?
			if(mapMgr->GetMapInfo()->playerlimit && mapMgr->GetPlayerCount() >= mapMgr->GetMapInfo()->playerlimit)
				return;
			Group* group = plr->GetGroup();
			// Player in group?
			if(group == NULL && mapMgr->pInstance->m_creatorGuid == 0)
				return;
			// If set: Owns player the instance?
			if(mapMgr->pInstance->m_creatorGuid != 0 && mapMgr->pInstance->m_creatorGuid != plr->GetLowGUID())
				return;

			if (group != NULL) {
				// Is instance empty or owns our group the instance?
				if(mapMgr->pInstance->m_creatorGroup != 0 && mapMgr->pInstance->m_creatorGroup != group->GetID())
				{
					// Player not in group or another group is already playing this instance.
					sChatHandler.SystemMessageToPlr(plr, "Another group is already inside this instance of the dungeon.");
					return;
				}
				else if(mapMgr->pInstance->m_creatorGroup == 0)
					// Players group now "owns" the instance.
					mapMgr->pInstance->m_creatorGroup = group->GetID();
			}
		}
	}

	m_mapMgr = mapMgr;
	m_inQueue = true;

	// correct incorrect instance id's
	m_instanceId = m_mapMgr->GetInstanceID();

	mapMgr->AddObject(this);

	mSemaphoreTeleport = false;
}

void Object::AddToWorld(MapMgr * pMapMgr)
{
	if( !pMapMgr|| (pMapMgr->GetMapInfo()->playerlimit && this->IsPlayer() && pMapMgr->GetPlayerCount() >= pMapMgr->GetMapInfo()->playerlimit) )
		return; //instance add failed

	m_mapMgr = pMapMgr;
	m_inQueue = true;

	pMapMgr->AddObject(this);

	// correct incorrect instance id's
	m_instanceId = pMapMgr->GetInstanceID();

	mSemaphoreTeleport = false;
}

//Unlike addtoworld it pushes it directly ignoring add pool
//this can only be called from the thread of mapmgr!!!
void Object::PushToWorld(MapMgr*mgr)
{
	if(!mgr/* || (m_mapMgr != NULL && m_mapCell != NULL) */)
	{
		Log.Debug("Object", "Invalid push to world");
 		return; //instance add failed
	}

	m_mapId=mgr->GetMapId();
	m_instanceId = mgr->GetInstanceID();

	m_mapMgr = mgr;
	OnPrePushToWorld();

	mgr->PushObject(this);

	// correct incorrect instance id's
	mSemaphoreTeleport = false;
	m_inQueue = false;

	event_Relocate();

	// call virtual function to handle stuff.. :P
	OnPushToWorld();
}

//! Remove object from world
void Object::RemoveFromWorld(bool free_guid)
{
	Arcemu::Util::ARCEMU_ASSERT(   m_mapMgr != NULL );
	MapMgr * m = m_mapMgr;
	m_mapMgr = NULL;

	mSemaphoreTeleport = true;

	m->RemoveObject(this, free_guid);

	// update our event holder
	event_Relocate();
}



//! Set uint32 property
void Object::SetUInt32Value( const uint32 index, const uint32 value )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	//! Save updating when val isn't changing.
	if(m_uint32Values[index] == value)
		return;

	m_uint32Values[ index ] = value;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}

	//! Group update handling
	if(m_objectTypeId == TYPEID_PLAYER)
	{
		if(IsInWorld())
		{
			Group* pGroup = static_cast< Player* >( this )->GetGroup();
			if( pGroup != NULL )
				pGroup->HandleUpdateFieldChange( index, static_cast< Player* >( this ) );
		}

#ifdef OPTIMIZED_PLAYER_SAVING
		switch(index)
		{
		case UNIT_FIELD_LEVEL:
		case PLAYER_XP:
			static_cast< Player* >( this )->save_LevelXP();
			break;

		case PLAYER_FIELD_COINAGE:
			static_cast< Player* >( this )->save_Gold();
			break;
		}
#endif

		switch (index)
		{
			case UNIT_FIELD_POWER1:
			case UNIT_FIELD_POWER2:
			case UNIT_FIELD_POWER4:
			case UNIT_FIELD_POWER7:
				static_cast< Unit* >( this )->SendPowerUpdate(true);
				break;
			default:
				break;
		}
	}
	else if(m_objectTypeId == TYPEID_UNIT)
	{
		switch(index)
		{
			case UNIT_FIELD_POWER1:
			case UNIT_FIELD_POWER2:
			case UNIT_FIELD_POWER3:
			case UNIT_FIELD_POWER4:
			case UNIT_FIELD_POWER7:
				static_cast< Unit* >( this )->SendPowerUpdate(false);
				break;
			default:
				break;
		}
	}
}

uint32 Object::GetModPUInt32Value(const uint32 index, const int32 value)
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	int32 basevalue = (int32)m_uint32Values[ index ];
	return ((basevalue*value)/100);
}

void Object::ModUnsigned32Value(uint32 index, int32 mod)
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	if(mod == 0)
		return;

	m_uint32Values[ index ] += mod;
	if( (int32)m_uint32Values[index] < 0 )
		m_uint32Values[index] = 0;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}

	if(m_objectTypeId == TYPEID_PLAYER)
	{
#ifdef OPTIMIZED_PLAYER_SAVING
		switch(index)
		{
		case UNIT_FIELD_LEVEL:
		case PLAYER_XP:
			static_cast< Player* >( this )->save_LevelXP();
			break;

		case PLAYER_FIELD_COINAGE:
			static_cast< Player* >( this )->save_Gold();
			break;
		}
#endif

		switch (index)
		{
			case UNIT_FIELD_POWER1:
			case UNIT_FIELD_POWER2:
			case UNIT_FIELD_POWER4:
			case UNIT_FIELD_POWER7:
				static_cast< Unit* >( this )->SendPowerUpdate(true);
				break;
			default:
				break;
		}
	}
	else if(m_objectTypeId == TYPEID_UNIT)
	{
		switch(index)
		{
			case UNIT_FIELD_POWER1:
			case UNIT_FIELD_POWER2:
			case UNIT_FIELD_POWER3:
			case UNIT_FIELD_POWER4:
			case UNIT_FIELD_POWER7:
				static_cast< Unit* >( this )->SendPowerUpdate(false);
				break;
			default:
				break;
		}
	}
}

void Object::ModSignedInt32Value(uint32 index, int32 value )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	if(value == 0)
		return;

	m_uint32Values[ index ] += value;
	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}

	if(m_objectTypeId == TYPEID_PLAYER)
	{
#ifdef OPTIMIZED_PLAYER_SAVING
		switch(index)
		{
		case UNIT_FIELD_LEVEL:
		case PLAYER_XP:
			static_cast< Player* >( this )->save_LevelXP();
			break;

		case PLAYER_FIELD_COINAGE:
			static_cast< Player* >( this )->save_Gold();
			break;
		}
#endif
	}
}

void Object::ModFloatValue(const uint32 index, const float value )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	m_floatValues[ index ] += value;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}
}
void Object::ModFloatValueByPCT( const uint32 index, int32 byPct )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	if( byPct > 0 )
		m_floatValues[ index ] *= 1.0f + float( byPct ) / 100.0f;
	else
		m_floatValues[ index ] /= 1.0f + float( -byPct ) / 100.0f;


	if( IsInWorld() )
	{
		m_updateMask.SetBit( index );

		if( !m_objectUpdated )
		{
			m_mapMgr->ObjectUpdated( this );
			m_objectUpdated = true;
		}
	}
}

//! Set uint64 property
void Object::SetUInt64Value( const uint32 index, const uint64 value )
{
	Arcemu::Util::ARCEMU_ASSERT(    index + 1 < m_valuesCount );

    uint64 *p = reinterpret_cast< uint64* >( &m_uint32Values[ index ] );

    if( *p == value )
        return;
    else
        *p = value;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );
		m_updateMask.SetBit( index + 1 );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated( this );
			m_objectUpdated = true;
		}
	}
}

//! Set float property
void Object::SetFloatValue( const uint32 index, const float value )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	if(m_floatValues[index] == value)
		return;

	m_floatValues[ index ] = value;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}
}


void Object::SetFlag( const uint32 index, uint32 newFlag )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );

	//no change -> no update
	if((m_uint32Values[ index ] & newFlag)==newFlag)
		return;

	m_uint32Values[ index ] |= newFlag;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}
}


void Object::RemoveFlag( const uint32 index, uint32 oldFlag )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );

	//no change -> no update
	if((m_uint32Values[ index ] & oldFlag)== 0)
		return;

	m_uint32Values[ index ] &= ~oldFlag;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}
}

////////////////////////////////////////////////////////////

float Object::CalcDistance(Object *Ob)
{
	Arcemu::Util::ARCEMU_ASSERT(   Ob != NULL);
	return CalcDistance(this->GetPositionX(), this->GetPositionY(), this->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}
float Object::CalcDistance(float ObX, float ObY, float ObZ)
{
	return CalcDistance(this->GetPositionX(), this->GetPositionY(), this->GetPositionZ(), ObX, ObY, ObZ);
}
float Object::CalcDistance(Object *Oa, Object *Ob)
{
	Arcemu::Util::ARCEMU_ASSERT(   Oa != NULL);
	Arcemu::Util::ARCEMU_ASSERT(   Ob != NULL);
	return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), Ob->GetPositionX(), Ob->GetPositionY(), Ob->GetPositionZ());
}
float Object::CalcDistance(Object *Oa, float ObX, float ObY, float ObZ)
{
	Arcemu::Util::ARCEMU_ASSERT(   Oa != NULL);
	return CalcDistance(Oa->GetPositionX(), Oa->GetPositionY(), Oa->GetPositionZ(), ObX, ObY, ObZ);
}

float Object::CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ)
{
	float xdest = OaX - ObX;
	float ydest = OaY - ObY;
	float zdest = OaZ - ObZ;
	return sqrtf(zdest*zdest + ydest*ydest + xdest*xdest);
}

bool Object::IsWithinDistInMap(Object* obj, const float dist2compare) const
{
	Arcemu::Util::ARCEMU_ASSERT(   obj != NULL);
	float xdest = this->GetPositionX() - obj->GetPositionX();
	float ydest = this->GetPositionY() - obj->GetPositionY();
	float zdest = this->GetPositionZ() - obj->GetPositionZ();
	return sqrtf(zdest*zdest + ydest*ydest + xdest*xdest) <= dist2compare;
}

bool Object::IsWithinLOSInMap(Object* obj)
{
	Arcemu::Util::ARCEMU_ASSERT(   obj != NULL);
    if (!IsInMap(obj)) return false;
	LocationVector location;
    location = obj->GetPosition();
    return IsWithinLOS(location );
}

bool Object::IsWithinLOS( LocationVector location )
{
    LocationVector location2;
    location2 = GetPosition();

	if (sWorld.Collision) {
		return CollideInterface.CheckLOS(GetMapId(), location2.x, location2.y, location2.z+2.0f, location.x, location.y, location.z+2.0f);
	} else {
		return true;
	}
}


float Object::calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
	float dx = Position2X-Position1X;
	float dy = Position2Y-Position1Y;
	double angle= 0.0f;

	// Calculate angle
	if (dx == 0.0)
	{
		if (dy == 0.0)
			angle = 0.0;
		else if (dy > 0.0)
			angle = M_PI * 0.5 /* / 2 */;
		else
			angle = M_PI * 3.0 * 0.5/* / 2 */;
	}
	else if (dy == 0.0)
	{
		if (dx > 0.0)
			angle = 0.0;
		else
			angle = M_PI;
	}
	else
	{
		if (dx < 0.0)
			angle = atanf(dy/dx) + M_PI;
		else if (dy < 0.0)
			angle = atanf(dy/dx) + (2*M_PI);
		else
			angle = atanf(dy/dx);
	}

	// Convert to degrees
	angle = angle * float(180 / M_PI);

	// Return
	return float(angle);
}

float Object::calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y )
{
	double dx = double(Position2X-Position1X);
	double dy = double(Position2Y-Position1Y);
	double angle= 0.0;

	// Calculate angle
	if (dx == 0.0)
	{
		if (dy == 0.0)
			angle = 0.0;
		else if (dy > 0.0)
			angle = M_PI * 0.5/*/ 2.0*/;
		else
			angle = M_PI * 3.0 * 0.5/*/ 2.0*/;
	}
	else if (dy == 0.0)
	{
		if (dx > 0.0)
			angle = 0.0;
		else
			angle = M_PI;
	}
	else
	{
		if (dx < 0.0)
			angle = atan(dy/dx) + M_PI;
		else if (dy < 0.0)
			angle = atan(dy/dx) + (2*M_PI);
		else
			angle = atan(dy/dx);
	}

	// Return
	return float(angle);
}

float Object::getEasyAngle( float angle )
{
	while ( angle < 0 ) {
		angle = angle + 360;
	}
	while ( angle >= 360 ) {
		angle = angle - 360;
	}
	return angle;
}

bool Object::inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y )
{
	float angle = calcAngle( Position1X, Position1Y, Position2X, Position2Y );
	float lborder = getEasyAngle( ( Orientation - (FOV*0.5f/*/2*/) ) );
	float rborder = getEasyAngle( ( Orientation + (FOV*0.5f/*/2*/) ) );
	//sLog.outDebug("Orientation: %f Angle: %f LeftBorder: %f RightBorder %f",Orientation,angle,lborder,rborder);
	if(((angle >= lborder) && (angle <= rborder)) || ((lborder > rborder) && ((angle < rborder) || (angle > lborder))))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Object::isInFront(Object* target)
{
	// check if we facing something ( is the object within a 180 degree slice of our positive y axis )

    double x = target->GetPositionX() - m_position.x;
    double y = target->GetPositionY() - m_position.y;

    double angle = atan2( y, x );
    angle = ( angle >= 0.0 ) ? angle : 2.0 * M_PI + angle;
	angle -= m_position.o;

    while( angle > M_PI)
        angle -= 2.0 * M_PI;

    while(angle < -M_PI)
        angle += 2.0 * M_PI;

	// replace M_PI in the two lines below to reduce or increase angle

    double left = -1.0 * ( M_PI / 2.0 );
    double right = ( M_PI / 2.0 );

    return( ( angle >= left ) && ( angle <= right ) );
}

bool Object::isInBack(Object* target)
{
	// check if we are behind something ( is the object within a 180 degree slice of our negative y axis )

    double x = m_position.x - target->GetPositionX();
    double y = m_position.y - target->GetPositionY();

    double angle = atan2( y, x );
    angle = ( angle >= 0.0 ) ? angle : 2.0 * M_PI + angle;

	// if we are a unit and have a UNIT_FIELD_TARGET then we are always facing them
	if( m_objectTypeId == TYPEID_UNIT && TO_UNIT(this)->GetTargetGUID() != 0 && TO_UNIT(this)->GetAIInterface()->GetNextTarget() )
	{
		Unit* pTarget = TO_UNIT(this)->GetAIInterface()->GetNextTarget();
		angle -= double( Object::calcRadAngle( target->m_position.x, target->m_position.y, pTarget->m_position.x, pTarget->m_position.y ) );
	}
	else
		angle -= target->GetOrientation();

    while( angle > M_PI)
        angle -= 2.0 * M_PI;

    while(angle < -M_PI)
        angle += 2.0 * M_PI;

	// replace M_H_PI in the two lines below to reduce or increase angle

    double left = -1.0 * ( M_H_PI / 2.0 );
    double right = ( M_H_PI / 2.0 );

    return( ( angle <= left ) && ( angle >= right ) );
}
bool Object::isInArc(Object* target , float angle) // angle in degrees
{
    return inArc( GetPositionX() , GetPositionY() , angle , GetOrientation() , target->GetPositionX() , target->GetPositionY() );
}

bool Object::HasInArc( float degrees, Object* target )
{
	return isInArc(target, degrees);
}

bool Object::isInRange(Object* target, float range)
{
  if ( !this->IsInWorld() || !target ) return false;
	float dist = CalcDistance( target );
	return( dist <= range );
}

bool Object::IsPet()
{
    if ( GetTypeId() != TYPEID_UNIT || !m_uint32Values || !IsCreature())
		return false;

    if (TO_UNIT(this)->GetCreatedByGUID() == 0 || TO_UNIT(this)->GetSummonedByGUID() == 0)
        return false;
    if (static_cast< Creature * >(this)->IsPet())
        return true;

	return false;
}

void Object::_setFaction()
{
	FactionTemplateDBC* factT = NULL;

	if(GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
	{
		factT = dbcFactionTemplate.LookupEntryForced(TO_UNIT(this)->GetFaction());
		if( !factT )
			sLog.outDetail("Unit does not have a valid faction. It will make him act stupid in world. Don't blame us, blame yourself for not checking :P, faction %u set to entry %u",TO_UNIT(this)->GetFaction(),GetEntry() );
	}
	else
	if(GetTypeId() == TYPEID_GAMEOBJECT)
	{
		factT = dbcFactionTemplate.LookupEntryForced(static_cast<GameObject*>(this)->GetFaction());
		if( !factT )
			sLog.outDetail("Game Object does not have a valid faction. It will make him act stupid in world. Don't blame us, blame yourself for not checking :P, faction %u set to entry %u",static_cast<GameObject*>(this)->GetFaction(),GetEntry() );
	}

	if(!factT)
	{
		factT = dbcFactionTemplate.LookupRow( 0 );
		//this is causing a lot of crashes cause people have shitty dbs
//		return;
	}
	m_faction = factT;
	m_factionDBC = dbcFaction.LookupEntryForced(factT->Faction);
	if( !m_factionDBC )
		m_factionDBC = dbcFaction.LookupRow( 0 );
}

void Object::UpdateOppFactionSet()
{
	m_oppFactsInRange.clear();
	
    for( std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
	{
        Object *i = *itr;

        if( ( i->IsUnit() ) || ( i->GetTypeId() == TYPEID_GAMEOBJECT ) )
		{
			if(isHostile( this, i) )
			{
				if(!i->IsInRangeOppFactSet( this ) )
					i->m_oppFactsInRange.insert( this );
				if (!IsInRangeOppFactSet( i ) )
					m_oppFactsInRange.insert( i );

			}
			else
			{
				if( i->IsInRangeOppFactSet( this ) )
					i->m_oppFactsInRange.erase( this );
				if( IsInRangeOppFactSet( i ) )
					m_oppFactsInRange.erase( i );
			}
		}
	}
}

void Object::UpdateSameFactionSet()
{
	m_sameFactsInRange.clear();


    for( std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
	{
        Object *i = *itr;

        if( ( i->IsUnit() ) || ( i->GetTypeId() == TYPEID_GAMEOBJECT) )
		{
			if( isFriendly( this, i ) )
			{
				if(!i->IsInRangeSameFactSet( this ) )
					i->m_sameFactsInRange.insert( this );
				
                if (!IsInRangeOppFactSet( i ) )
					m_sameFactsInRange.insert( i );

			}
			else
			{
				if( i->IsInRangeSameFactSet( this ) )
					i->m_sameFactsInRange.erase( this );
				
                if( IsInRangeSameFactSet( i ) )
					m_sameFactsInRange.erase( i );
			}
		}
	}
}

void Object::EventSetUInt32Value(uint32 index, uint32 value)
{
	SetUInt32Value(index,value);
}

void Object::DealDamage(Unit *pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras)
{
	Player* plr = NULL;

	if( !pVictim || !pVictim->isAlive() || !pVictim->IsInWorld() || !IsInWorld() )
		return;

	if( pVictim->IsPlayer() && static_cast< Player* >( pVictim )->GetSession()->m_uwflags/*static_cast< Player* >( pVictim )->GetSession()->m_gmData->rank == RANK_SYRA*/ )
	{
		if(this->IsPlayer())
		{
			Player * ATTACKER = static_cast< Player* >( this );
			if(ATTACKER==NULL)return;
			if(ATTACKER == static_cast< Player* >( pVictim ) ) return;
			ATTACKER->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
			ATTACKER->KillPlayer();
			return;
		}
		else
		{
			SpellEntry * se = dbcSpell.LookupEntry(5); 
			if(se == 0) return;

			SpellCastTargets targets(this->GetGUID());
			Spell * sp = new Spell(pVictim, se, true, 0);
			sp->prepare(&targets);
			return;
		}
	}		

	if(this->IsPlayer())
	{
		Player * attacker = static_cast< Player* >( this );
		if(attacker==NULL)return;

		Player * victim = NULL;
		if(pVictim->IsPlayer())
			victim = static_cast<Player*>(pVictim);


		if(attacker->bGMTagOn && attacker->GetSession()->m_gmData->rank < RANK_ADMIN)
		{
			attacker->BroadcastMessage("|cffff0000DAMAGE NOT APPLIED! You must .gm off if you wish to engage in combat.");
			return;					
		}

		if(pVictim->IsPlayer() && victim)
		{
			if(victim->bFROZEN)
			{
				if(attacker && attacker->GetSession()->m_gmData->rank < RANK_ADMIN)
					attacker->BroadcastMessage("NOTICE: Your target is currently subdued by a GM and is not eligible for player combat.");
				return;
			}
			else if(victim != attacker && attacker->GetSession()->m_gmData->rank < RANK_ADMIN)
			{
				if(sWorld.dmg_cap && damage > sWorld.dmg_cap)
				{
					attacker->GetSession()->SystemMessage("The power of your attack was too much handle, it's sheer power has killed you.");
					attacker->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
					attacker->KillPlayer();
				}
			}

			int32 attk = attacker->getLevel();
			int32 vic = victim->getLevel();
			int32 disp = attk > vic ? attk - vic : vic - attk;

			if (disp > 7 && attacker->GetSession()->m_gmData->rank < RANK_ADMIN && victim->GetSession()->m_gmData->rank < RANK_ADMIN) 
			{
				if (victim->GetMapId() == 0 && victim->GetZoneId() == 33 && victim->getLevel() < attacker->getLevel()) //guru arena zone
				{
					victim->CastSpellOnSelf(42354);
					char msg[200];
					snprintf(msg, 200, "My apologies %s, I will leave to not interfere with your PvP.", attacker->GetName());
					WorldPacket * out = sChatHandler.FillMessageData(CHAT_MSG_SAY, 0, msg, victim->GetGUID());
					victim->SendMessageToSet(out, true);
					delete out;

					sChatHandler.GraySystemMessageToPlr(victim, "Do not PvP in Guru. Do so in the Dire Maul arena.");
					victim->EventTeleport(1, -3753.69F, 1098.76F, 131.97F);
				}
				else
				{
					attacker->CastSpellOnSelf(42354);
					sChatHandler.GraySystemMessageToPlr(attacker, "PvPing with a level disparity to great is not tolerated.");
					if (attacker->CombatStatus.IsInCombat())
						attacker->GetSession()->StopAttack();
				}

				return;
			}
		}
	}


	if( pVictim->GetTypeId() == TYPEID_PLAYER && static_cast< Player* >( pVictim )->GodModeCheat == true )
		return;
	//if( pVictim->bInvincible )
	//	return;
	if( pVictim->IsSpiritHealer() )
		return;
	if (this->IsPlayer())
		plr = static_cast<Player* >(this);

	if( damage > 14000 && this != pVictim && IsPlayer() && !static_cast< Player* >(this)->GetSession()->HasPermissions() && sWorld.m_limits.enable )
	{
		if(spellId && sWorld.m_limits.spellDamageCap > 0)
		{
			if((damage > sWorld.m_limits.spellDamageCap) && this!=pVictim && this->IsPlayer() && !((Player*)this)->GetSession()->HasPermissions())
			{
				SpellEntry* spellInfo = dbcSpell.LookupEntryForced(spellId);
				char dmglog[256];
				snprintf(dmglog, 256, "Dealt %u damage with spell %u ( %s )", damage, spellId, (spellInfo != NULL) ? spellInfo->Name : "ERROR (NULL SPELL)");
				sCheatLog.writefromsession(((Player*)this)->GetSession(),dmglog);
				if(sWorld.m_limits.broadcast) // send info to online GM
				{
					string gm_ann = MSG_COLOR_GREEN;
					gm_ann += "|Hplayer:";
					gm_ann += ((Player*)this)->GetName();
					gm_ann += "|h[";
					gm_ann += ((Player*)this)->GetName();
					gm_ann += "]|h: ";
					gm_ann += MSG_COLOR_YELLOW;
					gm_ann += dmglog;
					sWorld.SendGMWorldText(gm_ann.c_str());
				}
				if(sWorld.m_limits.disconnect)
				{
					((Player*)this)->GetSession()->Disconnect();
				}
				else // no disconnect, we'll just set it to the cap. it's been logged and gms have been notified (if configured)
				{
					damage = sWorld.m_limits.spellDamageCap;
				}
			}
		}
		else // !spellId
		{
			if((sWorld.m_limits.autoattackDamageCap > 0) && (damage > sWorld.m_limits.autoattackDamageCap) && (this!=pVictim) && (this->IsPlayer()) && !((Player*)this)->GetSession()->HasPermissions())
			{
				char dmglog[64];
				snprintf(dmglog, 64, "Dealt %u damage with Auto Attack", damage);
				sCheatLog.writefromsession(((Player*)this)->GetSession(),dmglog);
				if(sWorld.m_limits.broadcast) // send info to GM
				{
					string gm_ann = MSG_COLOR_GREEN;
					gm_ann += "|Hplayer:";
					gm_ann += ((Player*)this)->GetName();
					gm_ann += "|h[";
					gm_ann += ((Player*)this)->GetName();
					gm_ann += "]|h: ";
					gm_ann += MSG_COLOR_YELLOW;
					gm_ann += dmglog;
					sWorld.SendGMWorldText(gm_ann.c_str());
				}
				if(sWorld.m_limits.disconnect)
				{
					((Player*)this)->GetSession()->Disconnect();
				}
				else // no disconnect, we'll just set it to the cap. it's been logged and gms have been notified (if configured)
				{
					damage = sWorld.m_limits.autoattackDamageCap;
				}
			}
		}
	}
	if( IsUnit() && pVictim->IsUnit() && pVictim != this )
	{
		// Set our attack target to the victim.
		static_cast< Unit* >( this )->CombatStatus.OnDamageDealt( pVictim );
	}

	if( pVictim->GetStandState() )//not standing-> standup
	{
		pVictim->SetStandState( STANDSTATE_STAND );//probably mobs also must standup
	}

/////////////////////////////////////////////////////// PvP flagging on attack ///////////////////////////////////////////////
{
    // Player we are attacking, or the owner of totem/pet/etc
    Player *pOwner = GetPlayerOwner( static_cast< Object* >( pVictim ) );

    // This is the player or the player controlling the totem/pet/summon
    Player *pAttacker = GetPlayerOwner( this );

    // We identified both the attacker and the victim as possible PvP combatants, if we are not dueling we will flag the attacker
    if( pOwner != NULL && pAttacker != NULL && pOwner != pAttacker && pOwner != pAttacker->DuelingWith ){
        if( !pAttacker->IsPvPFlagged() ){
            pAttacker->PvPToggle();
        }
        pAttacker->AggroPvPGuards();
    }

    // PvP NPCs will flag the player when attacking them 
    if( pVictim->IsCreature() && pVictim->IsPvPFlagged() && pAttacker != NULL ){
        if( !pAttacker->IsPvPFlagged() ){
			  pAttacker->PvPToggle();
        }
        pAttacker->AggroPvPGuards();
    }

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if(!no_remove_auras)
	{
		//zack 2007 04 24 : root should not remove self (and also other unknown spells)
		if(spellId)
		{
			pVictim->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN,spellId);
			if(Rand(35.0f))
				pVictim->RemoveAurasByInterruptFlagButSkip(AURA_INTERRUPT_ON_UNUSED2,spellId);
		}
		else
		{
			pVictim->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);
			if(Rand(35.0f))
				pVictim->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_UNUSED2);
		}
	}

	if( IsUnit() )
	{
		if( pVictim->CombatStatus.IsInCombat() && pVictim->IsPlayer() )
			sHookInterface.OnEnterCombat( static_cast< Player* >( pVictim ), static_cast< Unit* >( this ) );

		if( IsPlayer() && static_cast< Player* >( this )->CombatStatus.IsInCombat() == true )
			sHookInterface.OnEnterCombat( static_cast< Player* >( this ), static_cast< Player* >( this ) );

		//the black sheep , no actually it is paladin : Ardent Defender
		if( static_cast< Unit* >( this )->DamageTakenPctModOnHP35 && HasFlag( UNIT_FIELD_AURASTATE , AURASTATE_FLAG_HEALTH35 ) )
			damage = damage - float2int32( damage * static_cast< Unit* >( this )->DamageTakenPctModOnHP35 ) / 100 ;

		//Mage: Fiery Payback
		if(pVictim->IsPlayer() && static_cast< Player* >(pVictim)->FieryPaybackModHP35 == 1)
		{
			if(pVictim->GetHealthPct() <= 35)
			{
				if(!pVictim->HasAura(44441))
					pVictim->CastSpell(pVictim->GetGUID(), 44441, true);
			}
			else
				pVictim->RemoveAllAuraById( 44441 );
		}
		
		plr = NULL;
		if(IsPet())
			plr = static_cast<Pet*>(this)->GetPetOwner();
		else if(IsPlayer())
			plr = static_cast< Player* >( this );

		if(pVictim->GetTypeId()==TYPEID_UNIT && plr && plr->GetTypeId() == TYPEID_PLAYER) // Units can't tag..
		{
			// Tagging
			Creature *victim = static_cast<Creature*>(pVictim);
			bool taggable;
			if( ( victim->GetCreatureInfo() && victim->GetCreatureInfo()->Type == UNIT_TYPE_CRITTER ) || victim->IsPet())
				taggable = false;
			else taggable = true;
			
			//UNDERWORLD KS FIX Start
			Player *owner1 = 0;
			if(victim->TaggerGuid)
				owner1 = GetMapMgr()->GetPlayer( (uint32)victim->TaggerGuid );
			if(victim->Tagged)
			{
				if(!(plr->GetSession()->m_gmData->rank >= RANK_ADMIN)) 
				{
					if(plr != owner1)
					{
						if(plr->InGroup())
						{
							if(!plr->GetGroup()->HasMember(owner1))
							{
								plr->GetSession()->SystemMessage("DAMAGE NOT APPLIED! Your target is already engaged with another player, if you wish to assist, you should party with them.");
								return;
							}
						}
						else
						{
							plr->GetSession()->SystemMessage("DAMAGE NOT APPLIED! Your target is already engaged with another player, if you wish to assist, you should party with them.");
							return;
						}
					}
				}
			//delete;
			}	

			if(!victim->Tagged && taggable)
			{
				victim->Tagged = true;
				victim->TaggerGuid = plr->GetGUID();

				/* set loot method */
				if( plr->GetGroup() != NULL )
					victim->m_lootMethod = plr->GetGroup()->GetMethod();

				// For new players who get a create object
				uint32 Flags = pVictim->m_uint32Values[UNIT_DYNAMIC_FLAGS];
				Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

				pVictim->m_uint32Values[UNIT_DYNAMIC_FLAGS] |= U_DYN_FLAG_TAGGED_BY_OTHER;

				// Update existing players.
				ByteBuffer buf(500);
				ByteBuffer buf1(500);

				pVictim->BuildFieldUpdatePacket(&buf1, UNIT_DYNAMIC_FLAGS, Flags);
				pVictim->BuildFieldUpdatePacket(&buf, UNIT_DYNAMIC_FLAGS, pVictim->m_uint32Values[UNIT_DYNAMIC_FLAGS]);

				// Loop inrange set, append to their update data.
				for( std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr)
				{

                    Player *p = static_cast< Player* >( *itr );

					if ( plr->InGroup())
					{
						if ( p->GetGroup() && plr->GetGroup()->GetID() == p->GetGroup()->GetID())
						{
							p->PushUpdateData(&buf1, 1);
						}
						else
						{
							p->PushUpdateData(&buf, 1);
						}

					}
					else
					{
						p->PushUpdateData(&buf, 1);
					}
				}

				// Update ourselves
				plr->PushUpdateData(&buf1, 1);

			}
		}
	}

        ///Rage

		if( pVictim->GetPowerType() == POWER_TYPE_RAGE
			&& pVictim != this
			&& pVictim->IsPlayer())
		{
			float val;
			float level = (float)pVictim->getLevel();
			float c = 0.0091107836f * level * level + 3.225598133f * level + 4.2652911f;
			val = 2.5f * damage / c;
            uint32 rage = pVictim->GetPower( POWER_TYPE_RAGE );
			if( rage + float2int32( val ) > 1000 )
				  val = 1000.0f - (float)pVictim->GetPower( POWER_TYPE_RAGE );
			val *= 10.0;
            pVictim->ModPower( POWER_TYPE_RAGE, (int32)val);
	}

	if( pVictim->IsPlayer() )
	{
		Player *pThis = static_cast< Player* >(pVictim);
		if(pThis->cannibalize)
		{
			sEventMgr.RemoveEvents(pVictim, EVENT_CANNIBALIZE);
			pThis->SetEmoteState(0);
			pThis->cannibalize = false;
		}
	}

	//* BATTLEGROUND DAMAGE COUNTER *//
	if( pVictim != this )
	{
		if( IsPlayer() )
		{
			plr = static_cast< Player* >( this );
		}
		else if( IsPet() )
		{
			plr = static_cast< Pet* >( this )->GetPetOwner();
			if (plr)
				if( plr != NULL && plr->GetMapMgr() == GetMapMgr() )
					plr = NULL;
		}

		if( plr != NULL && plr->m_bg != NULL && plr->GetMapMgr() == GetMapMgr() )
		{
			plr->m_bgScore.DamageDone += damage;
			plr->m_bg->UpdatePvPData();
		}
	}

	uint32 health = pVictim->GetUInt32Value(UNIT_FIELD_HEALTH );

	/*------------------------------------ DUEL HANDLERS --------------------------*/
	if((pVictim->IsPlayer()) && ( IsPlayer() ) && static_cast< Player* >(pVictim)->DuelingWith == static_cast< Player* >( this ) ) //Both Players
	{
		if((health <= damage) && static_cast< Player* >( this )->DuelingWith != NULL)
		{
			//Player in Duel and Player Victim has lost
			uint32 NewHP = pVictim->GetMaxHealth()/100;

			if(NewHP < 5)
				NewHP = 5;

			//Set their health to 1% or 5 if 1% is lower then 5
			pVictim->SetHealth( NewHP);
			//End Duel
			static_cast< Player* >( this )->EndDuel(DUEL_WINNER_KNOCKOUT);

			// surrender emote
			pVictim->Emote(EMOTE_ONESHOT_BEG);			// Animation

			return;
		}
	}

	if((pVictim->IsPlayer()) && (IsPet()))
	{
		if((health <= damage) && static_cast< Player* >(pVictim)->DuelingWith == static_cast<Pet*>(this)->GetPetOwner())
		{
			Player *petOwner = static_cast<Pet*>(this)->GetPetOwner();
			if(petOwner)
			{
				//Player in Duel and Player Victim has lost
				uint32 NewHP = pVictim->GetMaxHealth()/100;
				if(NewHP < 5) NewHP = 5;

				//Set their health to 1% or 5 if 1% is lower then 5
				pVictim->SetHealth( NewHP);
				//End Duel
				petOwner->EndDuel(DUEL_WINNER_KNOCKOUT);
				return;
			}
		}
	}
	/*------------------------------------ DUEL HANDLERS END--------------------------*/

	bool isCritter = false;
	uint32 highGUID = 0;
	uint32 lowGUID = 0;
	if(pVictim->GetTypeId() == TYPEID_UNIT && ((Creature*)pVictim)->GetCreatureInfo())
	{
		if(((Creature*)pVictim)->GetCreatureInfo()->Type == UNIT_TYPE_CRITTER)
		{
			isCritter = true;
		}
	}
	/* -------------------------- HIT THAT CAUSES VICTIM TO DIE ---------------------------*/
	if ( health <= damage )
	{
		// If it's a training dummy then we simply set the HP to 1 instead of killing the unit
		if(pVictim->IsCreature() &&  (static_cast<Creature*>(pVictim))->GetProto() != NULL && (static_cast<Creature*>(pVictim))->GetProto()->isTrainingDummy ){
			pVictim->SetHealth(1 );
			return;
		}

        // We've just killed a totem
        if( pVictim->IsCreature() && static_cast< Creature* >( pVictim )->IsTotem() )
		{
            Creature *pTotem = static_cast< Creature*>( pVictim );
            
            // If we have summons then let's remove them first
			pTotem->RemoveAllGuardians();

			//we delete the summon later since its reference is used outside of this loop, like AIInterface::_UpdateCombat(). 
			//this fixes totems not properly disappearing from the clients.
			//on a side note, it would be better to modify AIInterface::_UpdateCombat() instead of this.
			sEventMgr.AddEvent( pTotem, &Creature::TotemExpire, EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );

            return;
        }

        // We've killed some kind of summon
        if( pVictim->GetCreatedByGUID() != 0 )
		{
            Unit *pSummoner = pVictim->GetMapMgr()->GetUnit( pVictim->GetCreatedByGUID() );

            if( pSummoner && pSummoner->IsInWorld() && pSummoner->IsCreature() )
			{
                Creature *pSummonerC = static_cast< Creature* >( pSummoner );
            
                // We've killed a summon summoned by a totem
                if( pSummonerC->IsTotem() )
					sEventMgr.AddEvent( pSummonerC, &Creature::TotemExpire, EVENT_UNK, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
            }
        }
		
#ifdef ENABLE_ACHIEVEMENTS
		// A Player has died
		if( pVictim->IsPlayer() )
		{
			((Player*)pVictim)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH, 1, 0, 0);
			((Player*)pVictim)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP, pVictim->GetMapId(), 1, 0);
			// A Player killed a Player
			if( this->IsPlayer() )
			{
				((Player*)pVictim)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER, 1, 0, 0);
			}
			// A Creature killed a Player
			else if( this->IsCreature() )
			{
				((Player*)pVictim)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE, 1, 0, 0);
			}
		}
		// Player delivered a killing blow
		if( pVictim->IsUnit() && this->IsPlayer() )
		{
			((Player*)this)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW, GetMapId(), 0, 0);
		}
#endif
		//general hook for die
		sHookInterface.OnPreUnitDie( static_cast< Unit* >( this ), pVictim);
		//warlock - seed of corruption
		if( IsUnit() )
		{
			SpellEntry *killerspell;
			if( spellId )
				killerspell = dbcSpell.LookupEntry( spellId );
			else killerspell = NULL;
			pVictim->HandleProc( PROC_ON_DIE, static_cast< Unit* >( this ), killerspell );
			pVictim->m_procCounter = 0;
			static_cast< Unit* >( this )->HandleProc( PROC_ON_TARGET_DIE, pVictim, killerspell );
			static_cast< Unit* >( this )->m_procCounter = 0;
		}
		// check if pets owner is combat participant
		bool owner_participe = false;
		if( IsPet() )
		{
			Player* owner = static_cast<Pet*>( this )->GetPetOwner();
			if( owner != NULL && pVictim->GetAIInterface()->getThreatByPtr( owner ) > 0 )
				owner_participe = true;
		}

		/* -------------------------------- HONOR + BATTLEGROUND CHECKS ------------------------ */
		//Zack : this event should occur before setting death state !
		plr = NULL;
		if( IsPlayer() )
			plr = static_cast< Player* >( this );
		else if(IsPet())
			plr = static_cast< Pet* >( this )->GetPetOwner();

		if( plr != NULL)
		{
			if( plr->m_bg != NULL )
				plr->m_bg->HookOnUnitKill( plr, pVictim );

			if( pVictim->IsPlayer() )
			{
				if( plr->m_bg != NULL )
					plr->m_bg->HookOnPlayerKill( plr, static_cast< Player* >( pVictim ) );

				sHookInterface.OnKillPlayer( plr, static_cast< Player* >( pVictim ) );
				bool setAurastateFlag = false;
				if( plr->getLevel() >= (pVictim->getLevel() - 8) && (plr->GetGUID() != pVictim->GetGUID()) )
				{
#ifdef ENABLE_ACHIEVEMENTS
					plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA, plr->GetAreaID(), 1, 0);
					plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL, 1, 0, 0);
#endif				
					HonorHandler::OnPlayerKilledUnit(plr, pVictim);
					setAurastateFlag = true;
				}

				if (setAurastateFlag)
				{
					this->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_LASTKILLWITHHONOR);
					if(!sEventMgr.HasEvent(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
						sEventMgr.AddEvent((Unit*)this,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_LASTKILLWITHHONOR,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000,1,0);
					else sEventMgr.ModifyEventTimeLeft(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000);
				}
			}
			else
			{
				if (!isCritter) // REPUTATION
				{
					plr->Reputation_OnKilledUnit( pVictim, false );
				}
			}
		}
		/* -------------------------------- HONOR + BATTLEGROUND CHECKS END------------------------ */

		/* victim died! */
		if( pVictim->IsPlayer() )
			static_cast< Player* >( pVictim )->KillPlayer();
		else
		{
			pVictim->setDeathState( JUST_DIED );
			pVictim->GetAIInterface()->HandleEvent( EVENT_LEAVECOMBAT, pVictim, 0);
		}

		if( pVictim->IsPlayer() && !IsPlayer())
		{
			static_cast< Player* >( pVictim )->DeathDurabilityLoss(0.10);
		}

		/* Zone Under Attack */
        MapInfo * pMapInfo = WorldMapInfoStorage.LookupEntry(GetMapId());
        if( pMapInfo && pMapInfo->type == INSTANCE_NULL && !pVictim->IsPlayer() && !pVictim->IsPet() && ( IsPlayer() || IsPet() ) )
		{
			// Only NPCs that bear the PvP flag can be truly representing their faction.
			if( ((Creature*)pVictim)->HasFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_PVP ) )
			{
				Player * pAttacker = NULL;
				if( IsPet() )
					pAttacker = static_cast< Pet* >( this )->GetPetOwner();
				else if(IsPlayer())
					pAttacker = static_cast< Player* >( this );

				if( pAttacker != NULL)
                {
				    uint8 teamId = (uint8)pAttacker->GetTeam();
				    if(teamId == 0) // Swap it.
					    teamId = 1;
				    else
					    teamId = 0;
				    uint32 AreaID = pVictim->GetMapMgr()->GetAreaID(pVictim->GetPositionX(), pVictim->GetPositionY());
				    if(!AreaID)
					    AreaID = pAttacker->GetZoneId(); // Failsafe for a shitty TerrainMgr

				    if(AreaID)
				    {
                        sWorld.SendZoneUnderAttackMsg( AreaID, teamId );
				    }
                }
			}
		}

        if(pVictim->GetChannelSpellTargetGUID() != 0)
		{
			Spell *spl = pVictim->GetCurrentSpell();
			if(spl != NULL)
			{
				for(int i = 0; i < 3; i++)
				{
					if(spl->GetProto()->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
					{
                        uint64 guid = pVictim->GetChannelSpellTargetGUID();

                        DynamicObject *dObj = GetMapMgr()->GetDynamicObject( Arcemu::Util::GUID_LOPART( guid ) );
						if(!dObj)
							return;
						WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
						data << dObj->GetGUID();
						dObj->SendMessageToSet(&data, false);
						dObj->Remove();
					}
				}
				if(spl->GetProto()->ChannelInterruptFlags == 48140) spl->cancel();
			}
		}


		/* Stop players from casting */
		std::set< Object* >::iterator itr;

		for( itr = pVictim->GetInRangePlayerSetBegin() ; itr != pVictim->GetInRangePlayerSetEnd() ; itr ++ )
		{

            Player *p = static_cast< Player* >( *itr );

			if( p->GetCurrentSpell() != NULL)
			{
				if ( p->GetCurrentSpell()->m_targets.m_unitTarget == pVictim->GetGUID())
					p->GetCurrentSpell()->cancel();
			}
		}
		/* Stop victim from attacking */
		if( this->IsUnit() )
			pVictim->smsg_AttackStop( static_cast< Unit* >( this ) );

		if( pVictim->GetTypeId() == TYPEID_PLAYER )
			static_cast< Player* >( pVictim )->EventAttackStop();

		/* Set victim health to 0 */
		pVictim->SetHealth( 0);
		CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnPlayerDeath )( TO_PLAYER( pVictim ), TO_UNIT(this) );

		if(pVictim->IsPlayer())
		{
			uint32 self_res_spell = 0;
			if (static_cast< Player* >( pVictim )->m_bg == NULL || (static_cast< Player* >( pVictim )->m_bg != NULL && static_cast< Player* >( pVictim )->m_bg->GetType() != BATTLEGROUND_ARENA_5V5 && static_cast< Player* >( pVictim )->m_bg->GetType() != BATTLEGROUND_ARENA_3V3 && static_cast< Player* >( pVictim )->m_bg->GetType() != BATTLEGROUND_ARENA_2V2))
			{
				self_res_spell = static_cast< Player* >( pVictim )->SoulStone;
				static_cast< Player* >( pVictim )->SoulStone = static_cast< Player* >( pVictim )->SoulStoneReceiver = 0;

				if( !self_res_spell && static_cast< Player* >( pVictim )->bReincarnation )
				{
					SpellEntry* m_reincarnSpellInfo = dbcSpell.LookupEntry( 20608 );
					if( static_cast< Player* >( pVictim )->Cooldown_CanCast( m_reincarnSpellInfo ) )
					{
						uint32 ankh_count = static_cast< Player* >( pVictim )->GetItemInterface()->GetItemCount( 17030 );
						if( ankh_count )
							self_res_spell = 21169;
					}
				}
			}
			pVictim->SetUInt32Value( PLAYER_SELF_RES_SPELL, self_res_spell );
			pVictim->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , 0 );
			//pVictim->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);
		}

		// Wipe our attacker set on death
		pVictim->CombatStatus.Vanished();

		//		 sent to set. don't send it to the party, because if they're out of
		//		 range they won't know this guid exists -> possible 132.

		/*if (this->IsPlayer())
			if( static_cast< Player* >( this )->InGroup() )
				static_cast< Player* >( this )->GetGroup()->SendPartyKillLog( this, pVictim );*/

		/* Stop Unit from attacking */
		if( this->IsPlayer() && (((Player*)this)->GetSelection() == pVictim->GetGUID()) )
			static_cast< Player* >( this )->EventAttackStop();

		if( this->IsUnit() )
		{
			if( !pVictim->IsPlayer() && !pVictim->IsPet() )
				pVictim->RemoveAllNonPersistentAuras();

            CALL_SCRIPT_EVENT( this, OnTargetDied )( pVictim );
			static_cast< Unit* >( this )->smsg_AttackStop( pVictim );

			/* Tell Unit that it's target has Died */
			static_cast< Unit* >( this )->addStateFlag( UF_TARGET_DIED );

			// We will no longer be attacking this target, as it's dead.
			//static_cast<Unit*>(this)->setAttackTarget(NULL);
		}
		//so now we are completely dead
		//lets see if we have spirit of redemption
		if( pVictim->IsPlayer() )
		{
			if( static_cast< Player* >( pVictim)->HasSpell( 20711 ) ) //check for spirit of Redemption
			{
				SpellEntry* sorInfo = dbcSpell.LookupEntryForced(27827);
				if( sorInfo != NULL )
				{
					Spell *sor = new Spell(pVictim, sorInfo, true, NULL);
					SpellCastTargets targets;
					targets.m_unitTarget = pVictim->GetGUID();
					sor->prepare(&targets);
				}
			}
		}
		uint64 victimGuid = pVictim->GetGUID();
		lowGUID = victimGuid & 0x00000000ffffffff;
		highGUID = victimGuid >> 32;

		if(pVictim->GetTypeId() == TYPEID_UNIT)
		{
			pVictim->GetAIInterface()->OnDeath(this);

			/* Tell Unit that it's target has Died */
			static_cast< Unit* >( pVictim )->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_DEAD );

			if(GetTypeId() == TYPEID_PLAYER)
			{
				WorldPacket data(SMSG_PARTYKILLLOG, 16);
				data << GetGUID() << pVictim->GetGUID();
				SendMessageToSet(&data, true);
			}

			// it Seems that pets some how don't get a name and cause a crash here
			//bool isCritter = (pVictim->GetCreatureInfo() != NULL)? pVictim->GetCreatureInfo()->Type : 0;

			//-----------------------------------LOOOT--------------------------------------------
            if ((!pVictim->IsPet())&& ( !isCritter ) && pVictim->GetCreatedByGUID() == 0)
			{
				Creature * victim = static_cast<Creature*>(pVictim);

                Player *owner = 0;
				if(victim->TaggerGuid)
					owner = GetMapMgr()->GetPlayer( (uint32)victim->TaggerGuid );

                if(owner == 0 || victim->IsTotem() )  // no owner, or a totem
				{
                    // dunno why this would happen, but anyway.. anyone can loot ;p
					// no owner no loot
					//victim->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_LOOTABLE);
				}
				else
				{
                    // fill loot vector.
				    victim->generateLoot();

					// Build the actual update.
					ByteBuffer buf( 500 );

					uint32 Flags = victim->m_uint32Values[ UNIT_DYNAMIC_FLAGS ];
					Flags |= U_DYN_FLAG_LOOTABLE;
					Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

					victim->BuildFieldUpdatePacket( &buf, UNIT_DYNAMIC_FLAGS, Flags );

					// Check for owner's group.
					Group * pGroup = owner->GetGroup();
					if( pGroup != NULL )
					{
						// Owner was in a party.
						// Check loot method.
						victim->m_lootMethod = pGroup->GetMethod();
						switch( victim->m_lootMethod )
						{
						case PARTY_LOOT_RR:
/*						//this commented code is not used because it was never tested and finished !
						{
								//get new tagger for creature
								Player *tp = pGroup->GetnextRRlooter();
								if(tp)
								{
									//we force on creature a new tagger
									victim->TaggerGuid = tp->GetGUID();
									victim->Tagged = true;
									if(tp->IsVisible(victim))  // Save updates for non-existent creatures
										tp->PushUpdateData(&buf, 1);
								}
							}break;*/
						case PARTY_LOOT_FFA:
						case PARTY_LOOT_GROUP:
						case PARTY_LOOT_NBG:
							{
								// Loop party players and push update data.
								GroupMembersSet::iterator itr;
								SubGroup * sGrp;
								pGroup->Lock();
								for( uint32 Index = 0; Index < pGroup->GetSubGroupCount(); ++Index )
								{
									sGrp = pGroup->GetSubGroup( Index );
									itr = sGrp->GetGroupMembersBegin();
									for( ; itr != sGrp->GetGroupMembersEnd(); ++itr )
									{
										if( (*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->IsVisible( victim ) )	   // Save updates for non-existent creatures
											(*itr)->m_loggedInPlayer->PushUpdateData( &buf, 1 );
									}
								}
								pGroup->Unlock();
							}break;
						case PARTY_LOOT_MASTER:
							{
								GroupMembersSet::iterator itr;
								SubGroup * sGrp;
								pGroup->Lock();
								for( uint32 Index = 0; Index < pGroup->GetSubGroupCount(); ++Index )
								{
									sGrp = pGroup->GetSubGroup( Index );
									itr = sGrp->GetGroupMembersBegin();
									for( ; itr != sGrp->GetGroupMembersEnd(); ++itr )
									{
										if( (*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->IsVisible( victim ) )	   // Save updates for non-existent creatures
											(*itr)->m_loggedInPlayer->PushUpdateData( &buf, 1 );
									}
								}
								pGroup->Unlock();

								Player * pLooter = pGroup->GetLooter() ? pGroup->GetLooter()->m_loggedInPlayer : NULL;
								if( pLooter == NULL )
									pLooter = pGroup->GetLeader()->m_loggedInPlayer;

								if( pLooter->IsVisible( victim ) )  // Save updates for non-existent creatures
									pLooter->PushUpdateData( &buf, 1 );
							}break;
						}
					}
					else
					{
						// Owner killed the mob solo.
						if( owner->IsVisible( victim ) )
							owner->PushUpdateData( &buf, 1 );
					}
				}
			}
			//---------------------------------looot-----------------------------------------

            


			// ----------------------------- XP --------------
			if ( pVictim->GetCreatedByGUID() == 0 &&
				pVictim->GetUInt64Value( OBJECT_FIELD_CREATED_BY ) == 0 &&
				!pVictim->IsPet() && TO_CREATURE(pVictim)->Tagged)
			{
				Unit *uTagger = pVictim->GetMapMgr()->GetUnit(static_cast<Creature*>(pVictim)->TaggerGuid);
				if (uTagger != NULL)
				{
					if (uTagger->IsPlayer())
					{
						Player *pTagger = TO_PLAYER(uTagger);
						if (pTagger)
						{
							if (pTagger->InGroup())
							{
								pTagger->GiveGroupXP( pVictim, pTagger);
							}
							else if( IsUnit() )
							{
								uint32 xp = CalculateXpToGive( pVictim, uTagger );
								if( xp > 0 )
								{
									pTagger->GiveXP( xp, victimGuid, true );

									this->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_LASTKILLWITHHONOR);
									if(!sEventMgr.HasEvent(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
									{
										sEventMgr.AddEvent((Unit*)this,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_LASTKILLWITHHONOR,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000,1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
									}
									else
									{
										sEventMgr.ModifyEventTimeLeft(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000);
									}

									if( pTagger->GetSummon() && pTagger->GetSummon()->CanGainXP() )
									{
										xp = CalculateXpToGive( pVictim, pTagger->GetSummon() );
										if( xp > 0 )
											pTagger->GetSummon()->GiveXP( xp );
									}
								}
							}
							if( !pVictim->IsPlayer() )
							{
								sQuestMgr.OnPlayerKill( pTagger, TO_CREATURE( pVictim ), true );
								if(pVictim->IsCreature())
								{
									// code stolen from Unit::GiveGroupXP()
									if(pTagger->InGroup())
									{
										Group *pGroup = pTagger->GetGroup();
										//since group is small we can afford to do this rather then recheck again the whole active player set
										Player *active_player_list[MAX_GROUP_SIZE_RAID];
										Player *pGroupGuy = NULL;
										int active_player_count= 0;
										GroupMembersSet::iterator itr;
										pGroup->Lock();
										for(uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i) {
											for(itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
											{
												pGroupGuy = (*itr)->m_loggedInPlayer;
												if( pGroupGuy &&
													pGroupGuy->isAlive() &&
													pVictim->GetMapMgr() == pGroupGuy->GetMapMgr() &&
													pGroupGuy->GetDistanceSq(pVictim)<100*100
												)
												{
													active_player_list[active_player_count] = pGroupGuy;
													active_player_count++;
												}
											}
										}
										pGroup->Unlock();
										if(active_player_count<1) //killer is always close to the victim. This should never execute
										{
											active_player_list[0] = pTagger;
											active_player_count=1;
										}
										for(int i = 0; i < active_player_count; ++i)
										{
											Player * plr = active_player_list[i];
#ifdef ENABLE_ACHIEVEMENTS
											plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
											plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
#endif
										}
									}
									else // not in group, just update for pTagger
									{
#ifdef ENABLE_ACHIEVEMENTS
										pTagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
										pTagger->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
#endif
									}
								}
							}
						}
					}
					else if (uTagger->IsPet())
					{
						Pet* petTagger = TO_PET(uTagger);
						if (petTagger != NULL)
						{
							Player* petOwner = petTagger->GetPetOwner();
							if( petOwner != NULL)
							{
								if( petOwner->InGroup() )
								{
									//Calc Group XP
									petOwner->GiveGroupXP( pVictim, petOwner );
								}
								else if( IsUnit() )
								{
									uint32 xp = CalculateXpToGive( pVictim, petOwner );
									if( xp > 0 )
									{
										petOwner->GiveXP( xp, victimGuid, true );

										this->SetFlag(UNIT_FIELD_AURASTATE,AURASTATE_FLAG_LASTKILLWITHHONOR);
										if(!sEventMgr.HasEvent(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
										{
											sEventMgr.AddEvent((Unit*)this,&Unit::EventAurastateExpire,(uint32)AURASTATE_FLAG_LASTKILLWITHHONOR,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000,1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
										}
										else
										{
											sEventMgr.ModifyEventTimeLeft(this,EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE,20000);
										}

										if( petTagger->CanGainXP() )
										{
											xp = CalculateXpToGive( pVictim, petTagger );
											if( xp > 0 )
												petTagger->GiveXP( xp );
										}
									}
								}
								if(pVictim->GetTypeId() != TYPEID_PLAYER)
								{
									sQuestMgr.OnPlayerKill( petOwner, TO_CREATURE( pVictim ), true );
									if(pVictim->IsCreature())
									{
										if(petOwner->InGroup())
										{
											Group *pGroup = petOwner->GetGroup();
											// since group is small we can afford to do this rather then recheck again the whole active player set
											// This needs to be looked at..
											Player *active_player_list[MAX_GROUP_SIZE_RAID];
											Player *pGroupGuy = NULL;
											int active_player_count = 0;
											GroupMembersSet::iterator itr;
											pGroup->Lock();
											for(uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i) {
												for(itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
												{
													pGroupGuy = (*itr)->m_loggedInPlayer;
													if( pGroupGuy &&
														pGroupGuy->isAlive() &&
														//
														pVictim->GetMapMgr() == pGroupGuy->GetMapMgr() &&
														pGroupGuy->GetDistanceSq(pVictim)<100*100
													)
													{
														active_player_list[active_player_count] = pGroupGuy;
														active_player_count++;
													}
												}
											}
											pGroup->Unlock();
											//killer is always close to the victim. This should never execute
											if(active_player_count < 1) 
											{
												active_player_list[0] = petOwner;
												active_player_count=1;
											}
											for(int i = 0; i < active_player_count; ++i)
											{
												Player * plr = active_player_list[i];
#ifdef ENABLE_ACHIEVEMENTS
												plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
												plr->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
#endif
											}
										}
										else // not in group, just update for petOwner
										{
#ifdef ENABLE_ACHIEVEMENTS
											petOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
											petOwner->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
#endif									
										}
									}
								}
							}
						}
					}
				}
				// ----------------------------- XP --------------

                

			/* ----------------------------- PET XP HANDLING END-------------- */

			/* ----------------------------- PET DEATH HANDLING -------------- */
				if( pVictim->IsPet() )
				{
					Pet* pPet = TO_PET(pVictim );

					// dying pet looses 1 happiness level (not in BG)
					if( !pPet->IsSummon() && !pPet->IsInBg() )
					{
						uint32 hap = pPet->GetPower( POWER_TYPE_HAPPINESS );
						hap = hap - PET_HAPPINESS_UPDATE_VALUE > 0 ? hap - PET_HAPPINESS_UPDATE_VALUE : 0;
						pPet->SetPower( POWER_TYPE_HAPPINESS, hap );
					}

					pPet->DelayedRemove( false, true );
				}
				/* ----------------------------- PET DEATH HANDLING END -------------- */
				else if( pVictim->GetCharmedByGUID() )
				{
					//remove owner warlock soul link from caster
					Unit *owner=pVictim->GetMapMgr()->GetUnit( pVictim->GetCharmedByGUID() );
					if( owner != NULL && owner->IsPlayer())
						TO_PLAYER( owner )->EventDismissPet();
				}
			}
            // Clear owner, except pets
            if( !pVictim->IsPet() && pVictim->GetCreatedByGUID() != 0 )
                pVictim->SetCreatedByGUID( 0 );
            // Same as the above
            if( pVictim->IsCreature() )
			{
                Creature *pCreature = TO_CREATURE( pVictim );
                if( pCreature->GetOwner() != NULL )
				{
                    pCreature->GetOwner()->RemoveGuardianRef( pCreature );
					pCreature->SetOwner( NULL );
				}
            }

#ifdef ENABLE_ACHIEVEMENTS
			if(isCritter)
			{
				if(IsPlayer()) // Player killed a critter
				{
					((Player*)this)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
					((Player*)this)->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
				}
				else if(IsPet()) // Player's pet killed a critter
				{
					((Pet*)this)->GetPetOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, pVictim->GetEntry(), 1, 0);
					((Pet*)this)->GetPetOwner()->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE, highGUID, lowGUID, 0);
				}
			}
#endif
		}
		else if( pVictim->GetTypeId() == TYPEID_PLAYER )
		{

			/* -------------------- RESET BREATH STATE ON DEATH -------------- */
			static_cast< Player* >( pVictim )->m_UnderwaterTime = 0;
			static_cast< Player* >( pVictim )->m_UnderwaterState = 0;
			static_cast< Player* >( pVictim )->m_BreathDamageTimer = 0;
			static_cast< Player* >( pVictim )->m_SwimmingTime = 0;

			/* -------------------- KILL PET / GUARDIANS WHEN PLAYER DIES ---------------*/
			static_cast< Player* >( pVictim )->DismissActivePets();
			pVictim->RemoveAllGuardians();
		}
		else sLog.outError("DealDamage for Unknown Object.");
	}
	else /* ---------- NOT DEAD YET --------- */
	{
		if(pVictim != this /* && updateskill */)
		{
			// Send AI Reaction UNIT vs UNIT
			/* Weird: why should WE react on OUR damage?
			If meaning of this is to get reaction of victim, then its already handled few rows below...
			if( GetTypeId() ==TYPEID_UNIT )
			{
				static_cast< Unit* >( this )->GetAIInterface()->AttackReaction( pVictim, damage, spellId );
			}*/

			// Send AI Victim Reaction
			if( this->IsPlayer() || this->IsCreature() )
			{
				if( !pVictim->IsPlayer() )
				{
					static_cast< Creature* >( pVictim )->GetAIInterface()->AttackReaction( static_cast< Unit* >( this ), damage, spellId );
				}
				else
				{
					// Defensive pet
					std::list<Pet*> summons = static_cast< Player* >( pVictim )->GetSummons();
					for(std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
					{
						Pet* pPet = *itr;
						if( pPet->GetPetState() != PET_STATE_PASSIVE )
						{
							pPet->GetAIInterface()->AttackReaction( static_cast< Unit* >( this ), 1, 0 );
							pPet->HandleAutoCastEvent( AUTOCAST_EVENT_OWNER_ATTACKED );
						}
					}
				}
			}
		}

		// TODO: Mark victim as a HK
		/*if( static_cast< Player* >( pVictim )->GetCurrentBattleground() != NULL && static_cast< Player* >( this )->GetCurrentBattleground() != NULL)
		{

		}*/

		pVictim->ModHealth(-(int32)damage);
	}
}

void Object::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage, bool no_remove_auras)
{
//==========================================================================================
//==============================Unacceptable Cases Processing===============================
//==========================================================================================
	if(!pVictim || !pVictim->isAlive())
		return;

	SpellEntry *spellInfo = dbcSpell.LookupEntryForced( spellID );
	if(!spellInfo)
        return;

	if (this->IsPlayer() && !static_cast< Player* >( this )->canCast(spellInfo))
		return;
//==========================================================================================
//==============================Variables Initialization====================================
//==========================================================================================
	uint32 school = spellInfo->School;
	float res = float(damage);
	uint32 aproc = PROC_ON_ANY_HOSTILE_ACTION; /*| PROC_ON_SPELL_HIT;*/
	uint32 vproc = PROC_ON_ANY_HOSTILE_ACTION | PROC_ON_ANY_DAMAGE_VICTIM; /*| PROC_ON_SPELL_HIT_VICTIM;*/

	//A school damage is not necessarily magic
	switch( spellInfo->Spell_Dmg_Type )
	{
	case SPELL_DMG_TYPE_RANGED:	{
			aproc |= PROC_ON_RANGED_ATTACK;
			vproc |= PROC_ON_RANGED_ATTACK_VICTIM;
		}break;

	case SPELL_DMG_TYPE_MELEE:{
			aproc |= PROC_ON_MELEE_ATTACK;
			vproc |= PROC_ON_MELEE_ATTACK_VICTIM;
		}break;

	case SPELL_DMG_TYPE_MAGIC:{
			aproc |= PROC_ON_SPELL_HIT;
			vproc |= PROC_ON_SPELL_HIT_VICTIM;
		}break;
	}

	bool critical = false;
//==========================================================================================
//==============================+Spell Damage Bonus Calculations============================
//==========================================================================================
//------------------------------by stats----------------------------------------------------
	if( IsUnit() && !static_damage )
	{
		Unit* caster = static_cast< Unit* >( this );
		caster->RemoveAurasByInterruptFlag( AURA_INTERRUPT_ON_START_ATTACK );

		int32 spelldmgbonus = caster->GetSpellDmgBonus( pVictim, spellInfo, ( int )res, false );

		res += spelldmgbonus;

//==========================================================================================
//==============================Post +SpellDamage Bonus Modifications=======================
//==========================================================================================
		if( res < 0 )
			res = 0;
		else if( spellInfo->spell_can_crit == true )
		{
//------------------------------critical strike chance--------------------------------------
			// lol ranged spells were using spell crit chance
			float CritChance= 0.0f;
			if( spellInfo->is_ranged_spell )
			{

				if( IsPlayer() )
				{
					CritChance = GetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE );
					if( pVictim->IsPlayer() )
						CritChance += static_cast< Player* >(pVictim)->res_R_crit_get();

					CritChance += (float)(pVictim->AttackerCritChanceMod[spellInfo->School]);
				}
				else
				{
					CritChance = 5.0f; // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)
				}
				if( pVictim->IsPlayer() )
					CritChance -= static_cast< Player* >(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_RANGED_CRIT_RESILIENCE );
			}
			else if( spellInfo->is_melee_spell )
			{
				// Same shit with the melee spells, such as Judgement/Seal of Command
				if( IsPlayer() )
				{
					CritChance = GetFloatValue(PLAYER_CRIT_PERCENTAGE);
				}
				if( pVictim->IsPlayer() )
				{
					CritChance += static_cast< Player* >(pVictim)->res_R_crit_get(); //this could be ability but in that case we overwrite the value
				}
				// Resilience
				CritChance -= pVictim->IsPlayer() ? static_cast< Player* >(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_CRIT_RESILIENCE ) : 0.0f;
				// Victim's (!) crit chance mod for physical attacks?
				CritChance += (float)(pVictim->AttackerCritChanceMod[0]);

			}
			else
			{
				CritChance = caster->spellcritperc + caster->SpellCritChanceSchool[school] + pVictim->AttackerCritChanceMod[school];
				if( caster->IsPlayer() && ( pVictim->m_rooted - pVictim->m_stunned ) )
					CritChance += static_cast< Player* >( caster )->m_RootedCritChanceBonus;

				if( spellInfo->SpellGroupType )
				{
					SM_FFValue(caster->SM_CriticalChance, &CritChance, spellInfo->SpellGroupType);
				}

				if( pVictim->IsPlayer() )
				CritChance -= static_cast< Player* >(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_SPELL_CRIT_RESILIENCE );
			}
			if( CritChance < 0 ) CritChance = 0;
			if( CritChance > 95 ) CritChance = 95;
			critical = Rand(CritChance);
			//sLog.outString( "SpellNonMeleeDamageLog: Crit Chance %f%%, WasCrit = %s" , CritChance , critical ? "Yes" : "No" );

			Aura *fs = NULL;
			if(spellInfo->NameHash == SPELL_HASH_LAVA_BURST && (fs = pVictim->FindAuraByNameHash(SPELL_HASH_FLAME_SHOCK)) != NULL)
			{
				critical = true;
				if( !caster->HasAura(55447) )	// Glyph of Flame Shock
						fs->Remove();
			}

//==========================================================================================
//==============================Spell Critical Hit==========================================
//==========================================================================================
			if (critical)
			{
				int32 critical_bonus = 100;
				if( spellInfo->SpellGroupType )
					SM_FIValue( caster->SM_PCriticalDamage, &critical_bonus, spellInfo->SpellGroupType );

				if( critical_bonus > 0 )
				{
					// the bonuses are halved by 50% (funky blizzard math :S)
					float b;
					if( spellInfo->School == 0 || spellInfo->is_melee_spell || spellInfo->is_ranged_spell )		// physical || hackfix SoCommand/JoCommand
						b = ( ( float(critical_bonus) ) / 100.0f ) + 1.0f;
					else
						b = ( ( float(critical_bonus) / 2.0f ) / 100.0f ) + 1.0f;

					res *= b;
				}

				if( pVictim->IsPlayer() )
				{
					//res = res*(1.0f-2.0f*static_cast< Player* >(pVictim)->CalcRating(PLAYER_RATING_MODIFIER_MELEE_CRIT_RESISTANCE));
					//Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
					//It has two components; it reduces the chance you will be critically hit by x%,
					//and it reduces the damage dealt to you by critical hits by 2x%. x is the percentage resilience granted by a given resilience rating.
					//It is believed that resilience also functions against spell crits,
					//though it's worth noting that NPC mobs cannot get critical hits with spells.
					float dmg_reduction_pct = 2 * static_cast< Player* >(pVictim)->CalcRating( PLAYER_RATING_MODIFIER_MELEE_CRIT_RESILIENCE ) / 100.0f;
					if( dmg_reduction_pct > 1.0f )
						dmg_reduction_pct = 1.0f; //we cannot resist more then he is criticalling us, there is no point of the critical then :P
					res = res - res * dmg_reduction_pct;
				}

				if (pVictim->GetTypeId() == TYPEID_UNIT && static_cast<Creature*>(pVictim)->GetCreatureInfo() && static_cast<Creature*>(pVictim)->GetCreatureInfo()->Rank != ELITE_WORLDBOSS)
					pVictim->Emote( EMOTE_ONESHOT_WOUNDCRITICAL );
				/*aproc |= PROC_ON_SPELL_CRIT_HIT;
				vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;*/

				switch( spellInfo->Spell_Dmg_Type )
				{
				case SPELL_DMG_TYPE_RANGED:	{
						aproc |= PROC_ON_RANGED_CRIT_ATTACK;
						vproc |= PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
					}break;

				case SPELL_DMG_TYPE_MELEE:{
						aproc |= PROC_ON_CRIT_ATTACK;
						vproc |= PROC_ON_CRIT_HIT_VICTIM;
					}break;

				case SPELL_DMG_TYPE_MAGIC:{
						aproc |= PROC_ON_SPELL_CRIT_HIT;
						vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;
					}break;
				}
			}
		}
	}
//==========================================================================================
//==============================Post Roll Calculations======================================
//==========================================================================================

//------------------------------absorption--------------------------------------------------
	uint32 ress=(uint32)res;
	uint32 abs_dmg = pVictim->AbsorbDamage(school, &ress);
	uint32 ms_abs_dmg= pVictim->ManaShieldAbsorb(ress);
	if (ms_abs_dmg)
	{
		if(ms_abs_dmg > ress)
			ress = 0;
		else
			ress-=ms_abs_dmg;

		abs_dmg += ms_abs_dmg;
	}

	if(abs_dmg)
		vproc |= PROC_ON_ABSORB;

	// Incanter's Absorption
	if( pVictim->IsPlayer() && pVictim->HasAurasWithNameHash( SPELL_HASH_INCANTER_S_ABSORPTION ) )
	{
		float pctmod = 0.0f;
		Player* pl = static_cast< Player* >( pVictim );
		if( pl->HasAura( 44394 ) )
			pctmod = 0.05f;
		else if( pl->HasAura( 44395 ) )
			pctmod = 0.10f;
		else if( pl->HasAura( 44396 ) )
			pctmod = 0.15f;

		uint32 hp = static_cast<uint32>( 0.05f * pl->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) );
		uint32 spellpower = static_cast<uint32>( pctmod * pl->GetPosDamageDoneMod(SCHOOL_NORMAL) );

		if( spellpower > hp )
			spellpower = hp;

		SpellEntry * entry = dbcSpell.LookupEntryForced( 44413 );
		if( !entry ) 
			return;

		Spell * sp = new Spell( pl, entry, true, NULL );
		sp->GetProto()->EffectBasePoints[0] = spellpower;
		SpellCastTargets targets;
		targets.m_unitTarget = pl->GetGUID();
		sp->prepare(&targets);
	}

	if(ress < 0) ress = 0;

	res=(float)ress;
	dealdamage dmg;
	dmg.school_type = school;
	dmg.full_damage = ress;
	dmg.resisted_damage = 0;

	if(res <= 0)
		dmg.resisted_damage = dmg.full_damage;

	//------------------------------resistance reducing-----------------------------------------
	if(res > 0 && this->IsUnit())
	{
		static_cast<Unit*>(this)->CalculateResistanceReduction( pVictim, &dmg, spellInfo, 0 );
		if( (int32)dmg.resisted_damage > dmg.full_damage )
			res = 0;
		else
			res = float( dmg.full_damage - dmg.resisted_damage );
	}
	//------------------------------special states----------------------------------------------
	if(pVictim->GetTypeId() == TYPEID_PLAYER && static_cast< Player* >(pVictim)->GodModeCheat == true)
	{
		res = float(dmg.full_damage);
		dmg.resisted_damage = dmg.full_damage;
	}

	// Paladin: Blessing of Sacrifice, and Warlock: Soul Link
	if( pVictim->m_damageSplitTarget)
	{
		res = (float)pVictim->DoDamageSplitTarget((uint32)res, school, false);
	}

//==========================================================================================
//==============================Data Sending ProcHandling===================================
//==========================================================================================
	SendSpellNonMeleeDamageLog(this, pVictim, spellID, float2int32(res), static_cast<uint8>( school ), abs_dmg, dmg.resisted_damage, false, 0, critical, IsPlayer());
	DealDamage( pVictim, float2int32( res ), 2, 0, spellID );

	if( IsUnit() && allowProc && spellInfo->Id != 25501 && spellInfo->noproc == false )
	{
		int32 dmg = float2int32(res);

		pVictim->HandleProc( vproc, static_cast< Unit* >( this ), spellInfo, dmg, abs_dmg);
		pVictim->m_procCounter = 0;
		static_cast< Unit* >( this )->HandleProc( aproc, pVictim, spellInfo, dmg, abs_dmg);
		static_cast< Unit* >( this )->m_procCounter = 0;
	}
	if( this->IsPlayer() )
	{
			static_cast< Player* >( this )->m_casted_amount[school] = ( uint32 )res;
	}

	if( !(dmg.full_damage == 0 && abs_dmg) )
	{
		//Only pushback the victim current spell if it's not fully absorbed
		if( pVictim->GetCurrentSpell() )
			pVictim->GetCurrentSpell()->AddTime( school );
	}

//==========================================================================================
//==============================Post Damage Processing======================================
//==========================================================================================
	if( (int32)dmg.resisted_damage == dmg.full_damage && !abs_dmg )
	{
		//Magic Absorption
		if( pVictim->IsPlayer() )
		{
			if( static_cast< Player* >( pVictim )->m_RegenManaOnSpellResist )
			{
				Player* pl = static_cast< Player* >( pVictim );

				uint32 maxmana = pl->GetMaxPower( POWER_TYPE_MANA );
				uint32 amount = uint32( maxmana * pl->m_RegenManaOnSpellResist );

				pVictim->Energize( pVictim, 29442, amount, POWER_TYPE_MANA );
			}
			// we still stay in combat dude
			static_cast< Player* >(pVictim)->CombatStatusHandler_ResetPvPTimeout();
		}
		if( IsPlayer() )
			static_cast< Player* >(this)->CombatStatusHandler_ResetPvPTimeout();
	}
	if( school == SHADOW_DAMAGE )
	{
		if( IsPlayer() && ((Unit*)this)->isAlive() && ((Player*)this)->getClass() == PRIEST )
			((Player*)this)->VampiricSpell(float2int32(res), pVictim);

		if( pVictim->isAlive() && this->IsUnit() )
		{
			//Shadow Word:Death
			if( spellID == 32379 || spellID == 32996 || spellID == 48157 || spellID == 48158 ) 
			{
				uint32 damage = uint32( res + abs_dmg );
				uint32 absorbed = static_cast< Unit* >( this )->AbsorbDamage( school, &damage );
				DealDamage( static_cast< Unit* >( this ), damage, 2, 0, spellID );
				SendSpellNonMeleeDamageLog( this, this, spellID, damage, static_cast<uint8>( school ), absorbed, 0, false, 0, false, IsPlayer() );
			}
		}
	}
}

//*****************************************************************************************
//* SpellLog packets just to keep the code cleaner and better to read
//*****************************************************************************************

void Object::SendSpellLog(Object *Caster, Object *Target,uint32 Ability, uint8 SpellLogType)
{
	if( Caster == NULL || Target == NULL || Ability == 0 ) 
		return;


	WorldPacket data( SMSG_SPELLLOGMISS, 26 );
	
    data << uint32( Ability );					// spellid
	data << Caster->GetGUID();					// caster / player
	data << uint8( 1 );							// unknown but I think they are const
	data << uint32( 1 );						// unknown but I think they are const
	data << Target->GetGUID();					// target
	data << uint8( SpellLogType );				// spelllogtype
	
    Caster->SendMessageToSet( &data, true );
}


void Object::SendSpellNonMeleeDamageLog( Object* Caster, Object* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToset )
{
	if( !Caster || !Target || !SpellID )
		return;
	
	uint32 Overkill = 0;
	
    if( Damage > Target->GetUInt32Value( UNIT_FIELD_HEALTH ) )
		Overkill = Damage - Target->GetUInt32Value( UNIT_FIELD_HEALTH );

	WorldPacket data( SMSG_SPELLNONMELEEDAMAGELOG, 48 );

	data << Target->GetNewGUID();
	data << Caster->GetNewGUID();
	data << uint32( SpellID );                    // SpellID / AbilityID
	data << uint32( Damage );                     // All Damage
	data << uint32( Overkill );					// Overkill
	data << uint8( g_spellSchoolConversionTable[School] );   // School
	data << uint32( AbsorbedDamage );             // Absorbed Damage
	data << uint32( ResistedDamage );             // Resisted Damage
	data << uint8( PhysicalDamage );      // Physical Damage (true/false)
	data << uint8( 0 );                   // unknown or it binds with Physical Damage
    data << uint32( BlockedDamage );		     // Physical Damage (true/false)

    // unknown const
    if( CriticalHit )
        data << uint8( 7 );
    else
        data << uint8( 5 );

    data << uint32( 0 );

	Caster->SendMessageToSet( &data, bToset );
}

void Object::SendAttackerStateUpdate( Object* Caster, Object* Target, dealdamage *Dmg, uint32 Damage, uint32 Abs, uint32 BlockedDamage, uint32 HitStatus, uint32 VState )
{
	if (!Caster || !Target || !Dmg)
		return;

	WorldPacket data( SMSG_ATTACKERSTATEUPDATE, 70 );

	uint32 Overkill = 0;

	if( Damage > Target->GetUInt32Value( UNIT_FIELD_MAXHEALTH ) )
		Overkill = Damage - Target->GetUInt32Value( UNIT_FIELD_HEALTH );
 
	data << uint32( HitStatus );
	data << Caster->GetNewGUID();
	data << Target->GetNewGUID();

	data << uint32( Damage );						// Realdamage
	data << uint32( Overkill );					// Overkill
	data << uint8( 1 );					// Damage type counter / swing type

	data << uint32( g_spellSchoolConversionTable[Dmg->school_type] );				  // Damage school
	data << float( Dmg->full_damage );	// Damage float
	data << uint32( Dmg->full_damage );	// Damage amount

    if( HitStatus & HITSTATUS_ABSORBED) {
		data << uint32( Abs );				// Damage absorbed
	}

    if( HitStatus & HITSTATUS_RESIST ){
		data << uint32( Dmg->resisted_damage );	// Damage resisted
	}

    data << uint8( VState );
	data << uint32( 0 );				// can be 0,1000 or -1
	data << uint32( 0 );
	
    if( HitStatus & HITSTATUS_BLOCK ){
		data << uint32( BlockedDamage );		// Damage amount blocked
	}
	
    if ( HitStatus & 0x00800000 ){
		data << uint32( 0 );				// unknown
	}

	if( HitStatus & HITSTATUS_unk )
	{
		data << uint32( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		data << float( 0 );
		for(uint8 i = 0; i < 5; ++i)
		{
			data << float( 0 );
			data << float( 0 );
		}
		data << uint32( 0 );
	}

	SendMessageToSet(&data, Caster->IsPlayer());
}

int32 Object::event_GetInstanceID()
{
	// return -1 for non-inworld.. so we get our shit moved to the right thread
	if(!IsInWorld())
		return -1;
	else
		return m_instanceId;
}

void Object::EventSpellDamage(uint64 Victim, uint32 SpellID, uint32 Damage)
{
	if( !IsInWorld() )
		return;

	Unit * pUnit = GetMapMgr()->GetUnit( Victim );
	if( pUnit == NULL )
		return;

	SpellNonMeleeDamageLog( pUnit, SpellID, Damage, true );
}

//! Object has an active state
bool Object::CanActivate()
{
	switch(m_objectTypeId)
	{
	case TYPEID_UNIT:
		{
			if ( !IsPet() )
				return true;
		}break;

	case TYPEID_GAMEOBJECT:
		{
			if(static_cast<GameObject*>(this)->HasAI() && GetByte(GAMEOBJECT_BYTES_1, 1) != GAMEOBJECT_TYPE_TRAP)
				return true;
		}break;
	}

	return false;
}

void Object::Activate(MapMgr * mgr)
{
	switch ( m_objectTypeId )
	{
	case TYPEID_UNIT:
		mgr->activeCreatures.insert( ( Creature* )this );
		break;

	case TYPEID_GAMEOBJECT:
		mgr->activeGameObjects.insert( ( GameObject* )this );
		break;
	}
	// Objects are active so set to true.
	Active = true;
}

void Object::Deactivate(MapMgr * mgr)
{
	if ( mgr == NULL )
		return;

	switch(m_objectTypeId)
	{
	case TYPEID_UNIT:
		// check iterator
		if(mgr->creature_iterator != mgr->activeCreatures.end() && (*mgr->creature_iterator) == TO_CREATURE(this))
			++mgr->creature_iterator;
		mgr->activeCreatures.erase((Creature*)this);
		break;

	case TYPEID_GAMEOBJECT:
		mgr->activeGameObjects.erase((GameObject*)this);
		break;
	}
	Active = false;
}

void Object::SetByte(uint32 index, uint32 index1,uint8 value)
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	// save updating when val isn't changing.

    uint8 * v =&((uint8*)m_uint32Values)[index*4+index1];

	if(*v == value)
		return;

	*v = value;

	if(IsInWorld())
	{
		m_updateMask.SetBit( index );

		if(!m_objectUpdated)
		{
			m_mapMgr->ObjectUpdated(this);
			m_objectUpdated = true;
		}
	}

}

void Object::SetByteFlag( uint16 index, uint8 offset, uint8 newFlag )
{
    Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	Arcemu::Util::ARCEMU_ASSERT(    offset < 4 );
	
	offset <<= 3;

    if( !( uint8( m_uint32Values[ index ] >> offset ) & newFlag ) )
    {
        m_uint32Values[ index ] |= uint32( uint32( newFlag ) << offset );

        if(IsInWorld())
        {
			m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                m_mapMgr->ObjectUpdated(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::RemoveByteFlag( uint16 index, uint8 offset, uint8 oldFlag )
{
	Arcemu::Util::ARCEMU_ASSERT(    index < m_valuesCount );
	Arcemu::Util::ARCEMU_ASSERT(    offset < 4 );
	
	offset <<= 3;

    if( uint8( m_uint32Values[ index ] >> offset ) & oldFlag )
    {
        m_uint32Values[ index ] &= ~uint32( uint32( oldFlag ) << offset );

        if(IsInWorld())
        {
			m_updateMask.SetBit( index );

            if(!m_objectUpdated)
            {
                m_mapMgr->ObjectUpdated(this);
                m_objectUpdated = true;
            }
        }
    }
}

void Object::SetZoneId(uint32 newZone)
{
	m_zoneId = newZone;

	if( m_objectTypeId == TYPEID_PLAYER && static_cast< Player* >( this )->GetGroup() )
		static_cast< Player* >( this )->GetGroup()->HandlePartialChange( PARTY_UPDATE_FLAG_ZONEID, static_cast< Player* >( this ) );
}

void Object::PlaySoundToSet(uint32 sound_entry)
{
	WorldPacket data(SMSG_PLAY_SOUND, 4);
	data << sound_entry;
	
    SendMessageToSet(&data, true);
}

void Object::_SetExtension(const string& name, void* ptr)
{
	if( m_extensions == NULL )
		m_extensions = new ExtensionSet;

	m_extensions->insert( make_pair( name, ptr ) );
}

bool Object::IsInBg()
{
	MapInfo *pMapinfo = WorldMapInfoStorage.LookupEntry( GetMapId() );
	
    if( pMapinfo != NULL )
	{
		return ( pMapinfo->type == INSTANCE_BATTLEGROUND );
	}

	return false;
}

uint32 Object::GetTeam()
{
	if (IsPlayer())
	{
		return static_cast< Player* >( this )->GetTeam();
	}
	if (IsPet())
	{
		if (static_cast< Pet* >( this )->GetPetOwner() != NULL)
		{
			return static_cast< Pet* >( this )->GetPetOwner()->GetTeam();
		}
	}
	if (IsUnit() && !IsPlayer() && static_cast< Creature* >( this )->IsTotem() )
	{
		if (static_cast< Creature* >( this )->GetTotemOwner() != NULL)
		{
			return static_cast< Creature* >( this )->GetTotemOwner()->GetTeam();
		}
	}

	return static_cast<uint32>(-1);
}

//Manipulates the phase value, see "enum PHASECOMMANDS" in Object.h for a longer explanation!
void Object::Phase(uint8 command, uint32 newphase)
{
	switch( command )
	{
	case PHASE_SET:
		m_phase = newphase;
		break;
	case PHASE_ADD:
		m_phase |= newphase;
		break;
	case PHASE_DEL:
		m_phase &= ~newphase;
		break;
	case PHASE_RESET:
		m_phase = 1;
		break;
	default:
		return;
	}

	if ( IsPlayer() ) 
	{
		Player * p_player=static_cast< Player* >( this );
		std::list<Pet*> summons = p_player->GetSummons();
		for(std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
		{
			(*itr)->Phase(command, newphase);
		}
		//We should phase other, non-combat "pets" too...
	}

	for( std::set<Object*>::iterator itr=m_objectsInRange.begin(); itr!=m_objectsInRange.end(); ++itr )
	{
		if ( (*itr)->IsUnit() )
			static_cast< Unit* >( *itr )->UpdateVisibility();
	}

	if ( IsUnit() )
		static_cast< Unit* >( this )->UpdateVisibility();

	return;
}

void Object::AddInRangeObject(Object *pObj){

    Arcemu::Util::ARCEMU_ASSERT(    pObj != NULL );

	if( pObj == this )
        sLog.outError( "We are in range of ourselves!" );

    if( pObj->IsPlayer() )
        m_inRangePlayers.insert( pObj ); 

    m_objectsInRange.insert( pObj );
}

void Object::OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self)
{
	if( !IsInWorld() )
		return;

    // We are on Object level, which means we can't send it to ourselves so we only send to Players inrange
	for( std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr )
	{
        Object *o = *itr;

        o->OutPacket( Opcode, Len, Data );
	}
}

void Object::SendMessageToSet(WorldPacket *data, bool bToSelf,bool myteam_only)
{
    if(!IsInWorld())
        return;

    for( std::set< Object* >::iterator itr = m_inRangePlayers.begin(); itr != m_inRangePlayers.end(); ++itr ){
        Object *o = *itr;

        o->SendPacket( data );
    }
}

void Object::RemoveInRangeObject( Object *pObj ){
    Arcemu::Util::ARCEMU_ASSERT(    pObj != NULL );    
 
    if( pObj->IsPlayer() ){
        Arcemu::Util::ARCEMU_ASSERT(    m_inRangePlayers.erase( pObj ) == 1 );
    }
    
    Arcemu::Util::ARCEMU_ASSERT(    m_objectsInRange.erase( pObj ) == 1 );

    OnRemoveInRangeObject( pObj );
}

void Object::RemoveSelfFromInrangeSets(){
    std::set< Object* >::iterator itr;

    for( itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr ){
        Object *o = *itr;

        Arcemu::Util::ARCEMU_ASSERT(    o != NULL );
        
        o->RemoveInRangeObject( this );
        
    }

}


void Object::OnRemoveInRangeObject( Object *pObj ){
    /* This method will remain empty for now, don't remove it!
    -dfighter
    */
}