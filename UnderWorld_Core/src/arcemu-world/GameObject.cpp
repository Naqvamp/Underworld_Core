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
GameObject::GameObject(uint64 guid)
{
	m_objectTypeId = TYPEID_GAMEOBJECT;
	m_valuesCount = GAMEOBJECT_END;
	m_uint32Values = _fields;
	memset(m_uint32Values, 0,(GAMEOBJECT_END)*sizeof(uint32));
	m_updateMask.SetCount(GAMEOBJECT_END);
	SetUInt32Value( OBJECT_FIELD_TYPE,TYPE_GAMEOBJECT|TYPE_OBJECT);
	SetGUID( guid );
	SetByte(GAMEOBJECT_BYTES_1, 3, 100);
	m_wowGuid.Init(GetGUID());

	SetScale(  1);//info->Size  );
	SetByte(GAMEOBJECT_BYTES_1, 3, 100);

	counter= 0;//not needed at all but to prevent errors that var was not initialized, can be removed in release

	bannerslot = bannerauraslot = -1;

	m_summonedGo = false;
	invisible = false;
	invisibilityFlag = INVIS_FLAG_NORMAL;
	spell = 0;
	m_summoner = NULL;
	charges = -1;
	m_ritualcaster = 0;
	m_ritualtarget = 0;
	m_ritualmembers = NULL;
	m_ritualspell = 0;

	m_quests = NULL;
	pInfo = NULL;
	myScript = NULL;
	m_spawn = 0;
	loot.gold = 0;
	m_deleted = false;
	usage_remaining = 1;
	m_respawnCell= NULL;
	m_rotation = 0;

	m_overrides = 0;
}

GameObject::~GameObject()
{
	sEventMgr.RemoveEvents(this);
	if(m_ritualmembers) {
		delete[] m_ritualmembers;
		m_ritualmembers = NULL;
	}

	uint32 guid = GetUInt32Value(OBJECT_FIELD_CREATED_BY);
	if(guid)
	{
		Player *plr = objmgr.GetPlayer(guid);
		if(plr && plr->GetSummonedObject() == this)
			plr->SetSummonedObject(NULL);

		if(plr == m_summoner)
			m_summoner = 0;
	}

	if(m_respawnCell!= NULL)
		m_respawnCell->_respawnObjects.erase(this);

	if (m_summonedGo && m_summoner)
		for(int i = 0; i < 4; i++)
			if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
				m_summoner->m_ObjectSlots[i] = 0;
}

bool GameObject::CreateFromProto(uint32 entry,uint32 mapid, float x, float y, float z, float ang, float r0, float r1, float r2, float r3, uint32 overrides)
{
	pInfo= GameObjectNameStorage.LookupEntry(entry);
	if(!pInfo)return false;

	Object::_Create( mapid, x, y, z, ang );
	SetEntry(  entry );
	
	m_overrides=overrides;
//	SetFloatValue( GAMEOBJECT_POS_X, x );
//	SetFloatValue( GAMEOBJECT_POS_Y, y );
//	SetFloatValue( GAMEOBJECT_POS_Z, z );
//	SetFloatValue( GAMEOBJECT_FACING, ang );
	SetPosition(x, y, z, ang);
	SetParentRotation(0, r0);
	SetParentRotation(1, r1);
	SetParentRotation(2, r2);
	SetParentRotation(3, r3);
	UpdateRotation();
    SetByte( GAMEOBJECT_BYTES_1, 3, 0 );
	SetByte( GAMEOBJECT_BYTES_1, 0, 1 );
	SetDisplayId(pInfo->DisplayID );
	SetByte( GAMEOBJECT_BYTES_1, 1, static_cast<uint8>( pInfo->Type ));
   
	InitAI();

	 return true;
}

void GameObject::EventCastSpell(uint32 guid, uint32 sp, bool triggered)
{
	Spell * spp = new Spell(this,dbcSpell.LookupEntry(sp),false,NULL);
	SpellCastTargets tars(guid);
	spp->prepare(&tars);
}

void GameObject::TrapSearchTarget()
{
	Update(100);
}

void GameObject::Update(uint32 p_time)
{
	if(m_event_Instanceid != m_instanceId)
	{
		event_Relocate();
		return;
	}

	if(!IsInWorld())
		return;

	if(m_deleted)
		return;

	if(spell && (GetByte(GAMEOBJECT_BYTES_1, 0) == 1))
	{
		if(checkrate > 1)
		{
			if(counter++%checkrate)
				return;
		}

        for( std::set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr )
		{
            float dist;

            Object *o = *itr;

			dist = GetDistanceSq( o );

			if( o != m_summoner && o->IsUnit() && dist <= range)
			{
				if(m_summonedGo)
				{
					if(!m_summoner)
					{
						ExpireAndDelete();
						return;
					}
					
                    if(!isAttackable(m_summoner, o ))
                        continue;
				}
				
				Spell * sp = new Spell( this, spell, true, NULL );
				SpellCastTargets tgt( o->GetGUID() );
				tgt.m_destX = GetPositionX();
				tgt.m_destY = GetPositionY();
				tgt.m_destZ = GetPositionZ();
				sp->prepare(&tgt);

				// proc on trap trigger
				if( pInfo->Type == GAMEOBJECT_TYPE_TRAP )
				{
					if( m_summoner != NULL )
						m_summoner->HandleProc( PROC_ON_TRAP_TRIGGER, reinterpret_cast< Unit* >( o ), spell );
				} 

				if(m_summonedGo)
				{
					ExpireAndDelete();
					return;
				}

				if(spell->EffectImplicitTargetA[0] == 16 ||
					spell->EffectImplicitTargetB[0] == 16)
				{
					return;	 // on area don't continue.
				}
			}
		}
    }
}

void GameObject::Spawn(MapMgr * m)
{
	PushToWorld(m);	
	CALL_GO_SCRIPT_EVENT(this, OnSpawn)();
}

void GameObject::Despawn(uint32 delay, uint32 respawntime)
{
	if(delay)
	{
		sEventMgr.AddEvent(this, &GameObject::Despawn, (uint32)0, respawntime, EVENT_GAMEOBJECT_EXPIRE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
		return;
	}

	if(!IsInWorld())
		return;

	loot.items.clear();

	//This is for go get deleted while looting
	if(m_spawn)
	{
		SetByte(GAMEOBJECT_BYTES_1, 0, static_cast<uint8>( m_spawn->state ));
		SetUInt32Value(GAMEOBJECT_FLAGS, m_spawn->flags);
	}

	CALL_GO_SCRIPT_EVENT(this, OnDespawn)();

	if(respawntime)
	{
		/* Get our originating mapcell */
		MapCell * pCell = m_mapCell;
		Arcemu::Util::ARCEMU_ASSERT(   pCell != NULL );
		pCell->_respawnObjects.insert((Object*)this);
		sEventMgr.RemoveEvents(this);
		sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnGameObject, this, pCell, EVENT_GAMEOBJECT_ITEM_SPAWN, respawntime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
		Object::RemoveFromWorld(false);
		m_respawnCell = pCell;
	}
	else
	{
		Object::RemoveFromWorld(true);
		ExpireAndDelete();
	}
}

void GameObject::SaveToDB()
{
	std::stringstream ss;

    ss << "DELETE FROM gameobject_spawns WHERE id = ";
    ss << m_spawn->id;
    ss << ";";

    WorldDatabase.ExecuteNA( ss.str().c_str() );

    ss.rdbuf()->str("");

	ss << "INSERT INTO gameobject_spawns VALUES("
		<< ((m_spawn == NULL) ? 0 : m_spawn->id) << ","
		<< GetEntry() << ","
		<< GetMapId() << ","
		<< GetPositionX() << ","
		<< GetPositionY() << ","
		<< GetPositionZ() << ","
		<< GetOrientation() << ","
//		<< GetUInt64Value(GAMEOBJECT_ROTATION) << ","
		<< uint64(0) << ","
		<< GetParentRotation(0) << ","
		<< GetParentRotation(2) << ","
		<< GetParentRotation(3) << ","
		<< GetUInt32Value(GAMEOBJECT_BYTES_1) << ","
		<< GetUInt32Value(GAMEOBJECT_FLAGS) << ","
		<< GetFaction() << ","
		<< GetScale() << ","
		<< "0,"
		<< m_phase << ","
		<< m_overrides << ")";
	WorldDatabase.Execute(ss.str().c_str());
}

void GameObject::SaveToFile(std::stringstream & name)
{

	std::stringstream ss;

	ss << "INSERT INTO gameobject_spawns VALUES("
		<< ((m_spawn == NULL) ? 0 : m_spawn->id) << ","
		<< GetEntry() << ","
		<< GetMapId() << ","
		<< GetPositionX() << ","
		<< GetPositionY() << ","
		<< GetPositionZ() << ","
		<< GetOrientation() << ","
//		<< GetUInt64Value(GAMEOBJECT_ROTATION) << ","
		<< uint64(0) << ","
		<< GetParentRotation(0) << ","
		<< GetParentRotation(2) << ","
		<< GetParentRotation(3) << ","
		<< GetByte(GAMEOBJECT_BYTES_1, 0) << ","
		<< GetUInt32Value(GAMEOBJECT_FLAGS) << ","
		<< GetFaction() << ","
		<< GetScale() << ","
		<< "0,"
		<< m_phase << ","
		<< m_overrides << ")";

	FILE * OutFile;

	OutFile = fopen(name.str().c_str(), "wb");
	if (!OutFile) return;
	fwrite(ss.str().c_str(),1,ss.str().size(),OutFile);
	fclose(OutFile);

}

void GameObject::InitAI()
{
	if(!pInfo)
		return;
	
	// this fixes those fuckers in booty bay
	if(pInfo->SpellFocus == 0 &&
		pInfo->sound1 == 0 &&
		pInfo->sound2 == 0 &&
		pInfo->sound3 != 0 &&
		pInfo->sound5 != 3 &&
		pInfo->sound9 == 1)
		return;

	if(pInfo->DisplayID == 1027)//Shaman Shrine
	{
		if(pInfo->ID != 177964 && pInfo->ID != 153556)
		{
			//Deactivate
			//SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
		}
	}


	uint32 spellid = 0;
	if(pInfo->Type==GAMEOBJECT_TYPE_TRAP)
	{	
		spellid = pInfo->sound3;
	}
	else if(pInfo->Type == GAMEOBJECT_TYPE_SPELL_FOCUS)
	{
		// get spellid from attached gameobject - by sound2 field
		if( pInfo->sound2 == 0 )
			return;
		if( GameObjectNameStorage.LookupEntry( pInfo->sound2 ) == NULL )
			return;
		spellid = GameObjectNameStorage.LookupEntry( pInfo->sound2 )->sound3;
	}
	else if(pInfo->Type == GAMEOBJECT_TYPE_RITUAL)
	{	
		m_ritualmembers = new uint32[pInfo->SpellFocus];
		memset(m_ritualmembers,0,sizeof(uint32)*pInfo->SpellFocus);
	}
    else if(pInfo->Type == GAMEOBJECT_TYPE_CHEST)
    {
        Lock *pLock = dbcLock.LookupEntryForced(GetInfo()->SpellFocus);
        if(pLock)
        {
            for(uint32 i= 0; i < 5; i++)
            {
                if(pLock->locktype[i])
                {
                    if(pLock->locktype[i] == 2) //locktype;
                    {
                        //herbalism and mining;
                        if(pLock->lockmisc[i] == LOCKTYPE_MINING || pLock->lockmisc[i] == LOCKTYPE_HERBALISM)
                        {
							CalcMineRemaining(true);
                        }
                    }
                }
            }
        }

    }
	else if ( pInfo->Type == GAMEOBJECT_TYPE_FISHINGHOLE )
	{
		CalcFishRemaining( true );
	}

	myScript = sScriptMgr.CreateAIScriptClassForGameObject(GetEntry(), this);

	// hackfix for bad spell in BWL
	if(!spellid || spellid == 22247)
		return;

	SpellEntry *sp= dbcSpell.LookupEntryForced(spellid);
	if(!sp)
	{
		spell = NULL;
		return;
	}
	else
	{
		spell = sp;
	}
	//ok got valid spell that will be casted on target when it comes close enough
	//get the range for that 
	
	float r = 0;

	for(uint32 i= 0;i<3;i++)
	{
		if(sp->Effect[i])
		{
			float t = GetRadius(dbcSpellRadius.LookupEntry(sp->EffectRadiusIndex[i]));
			if(t > r)
				r = t;
		}
	}

	if(r < 0.1)//no range
		r = GetMaxRange(dbcSpellRange.LookupEntry(sp->rangeIndex));

	range = r*r;//square to make code faster
	checkrate = 20;//once in 2 seconds
	
}

bool GameObject::Load(GOSpawn *spawn)
{
	if(!CreateFromProto(spawn->entry,0,spawn->x,spawn->y,spawn->z,spawn->facing,spawn->o,spawn->o1,spawn->o2,spawn->o3,spawn->overrides))
		return false;

	m_spawn = spawn;
	m_phase = spawn->phase;
	//SetRotation(spawn->o);
	SetUInt32Value(GAMEOBJECT_FLAGS,spawn->flags);
//	SetLevel(spawn->level);
	SetByte(GAMEOBJECT_BYTES_1, 0, static_cast<uint8>( spawn->state ));	
	if(spawn->faction)
	{
		SetFaction(spawn->faction);
		m_faction = dbcFactionTemplate.LookupEntryForced(spawn->faction);
		if(m_faction)
			m_factionDBC = dbcFaction.LookupEntry(m_faction->Faction);
	}
	SetScale( spawn->scale);
	_LoadQuests();
	CALL_GO_SCRIPT_EVENT(this, OnCreate)();
	CALL_GO_SCRIPT_EVENT(this, OnSpawn)();

	InitAI();

	_LoadQuests();
	return true;
}

void GameObject::DeleteFromDB()
{
	if( m_spawn != NULL )
	{
		WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE id=%u", m_spawn->id);
		WorldDatabase.Execute("DELETE FROM a_uw_gameobject_spawns WHERE id=%u", m_spawn->id);
	}
}

void GameObject::EventCloseDoor()
{
// gameobject_flags +1 closedoor animate restore the pointer flag.
// by cebernic
	SetByte(GAMEOBJECT_BYTES_1, 0, 1);
  SetUInt32Value(GAMEOBJECT_FLAGS, GetUInt32Value( GAMEOBJECT_FLAGS ) & ~1);
}

void GameObject::UseFishingNode(Player *player)
{
	sEventMgr.RemoveEvents( this );
	if( GetUInt32Value( GAMEOBJECT_FLAGS ) != 32 ) // Clicking on the bobber before something is hooked
	{
		player->GetSession()->OutPacket( SMSG_FISH_NOT_HOOKED );
		EndFishing( player, true );
		return;
	}
	
	/* Unused code: sAreaStore.LookupEntry(GetMapMgr()->GetAreaID(GetPositionX(),GetPositionY()))->ZoneId*/
	uint32 zone = player->GetAreaID();
	if( zone == 0 ) // If the player's area ID is 0, use the zone ID instead
		zone = player->GetZoneId();

	FishingZoneEntry *entry = FishingZoneStorage.LookupEntry( zone );
	if( entry == NULL ) // No fishing information found for area or zone, log an error, and end fishing
	{
		sLog.outError( "ERROR: Fishing zone information for zone %d not found!", zone );
		EndFishing( player, true );
		return;
	}
	uint32 maxskill = entry->MaxSkill;
	uint32 minskill = entry->MinSkill;

	if( player->_GetSkillLineCurrent( SKILL_FISHING, false ) < maxskill )	
		player->_AdvanceSkillLine( SKILL_FISHING, float2int32( 1.0f * sWorld.getRate( RATE_SKILLRATE ) ) );

	GameObject * school = NULL;
	for ( InRangeSet::iterator it = GetInRangeSetBegin(); it != GetInRangeSetEnd(); ++it )
	{
		if ( (*it) == NULL || (*it)->GetTypeId() != TYPEID_GAMEOBJECT || (*it)->GetByte(GAMEOBJECT_BYTES_1, 1) != GAMEOBJECT_TYPE_FISHINGHOLE)
			continue;
		school = static_cast<GameObject *>( *it );
		if ( !isInRange( school, (float)school->GetInfo()->sound1 ) )
		{
			school = NULL;
			continue;
		}
		else
			break;
	}

	if ( school != NULL ) // open school loot if school exists
	{
		
		if( school->GetMapMgr() != NULL )
			lootmgr.FillGOLoot( &school->loot, school->GetEntry(), school->GetMapMgr()->iInstanceMode );
		else
			lootmgr.FillGOLoot( &school->loot, school->GetEntry(), 0 );
		
		player->SendLoot( school->GetGUID(), LOOT_FISHING );
		EndFishing( player, false );
		school->CatchFish();
		
		if ( !school->CanFish() )
			sEventMgr.AddEvent( school, &GameObject::Despawn, (uint32)0, ( 1800000 + RandomUInt( 3600000 ) ), EVENT_GAMEOBJECT_EXPIRE, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT ); // respawn in 30 - 90 minutes
	}
	else if( Rand( ( ( player->_GetSkillLineCurrent( SKILL_FISHING, true ) - minskill ) * 100 ) / maxskill ) ) // Open loot on success, otherwise FISH_ESCAPED.
	{
		lootmgr.FillFishingLoot( &loot, zone );
		player->SendLoot( GetGUID(), LOOT_FISHING );
		EndFishing( player, false );
	}
	else // Failed
	{
		player->GetSession()->OutPacket( SMSG_FISH_ESCAPED );
		EndFishing( player, true );
	}

}

void GameObject::EndFishing(Player* player, bool abort )
{
	Spell * spell = player->GetCurrentSpell();
	
	if(spell)
	{
		if(abort)   // abort because of a reason
		{
			//FIX ME: here 'failed' should appear over progress bar
			spell->SendChannelUpdate(0);
			//spell->cancel();
			spell->finish(false);
		}
		else		// spell ended
		{
			spell->SendChannelUpdate(0);
			spell->finish();
		}
	}

	if(!abort)
		sEventMgr.AddEvent(this, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, 10000, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
	else
		ExpireAndDelete();
}

void GameObject::FishHooked(Player * player)
{
	WorldPacket  data(12);
	data.Initialize(SMSG_GAMEOBJECT_CUSTOM_ANIM); 
	data << GetGUID();
	data << (uint32)(0); // value < 4
	player->GetSession()->SendPacket(&data);
	//SetByte(GAMEOBJECT_BYTES_1, 0, 0);
	//BuildFieldUpdatePacket(player, GAMEOBJECT_FLAGS, 32);
	SetUInt32Value(GAMEOBJECT_FLAGS, 32);
 }

/////////////
/// Quests

void GameObject::AddQuest(QuestRelation *Q)
{
	m_quests->push_back(Q);
}

void GameObject::DeleteQuest(QuestRelation *Q)
{
	list<QuestRelation *>::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		if( ( (*it)->type == Q->type ) && ( (*it)->qst == Q->qst ) )
		{
			delete (*it);
			m_quests->erase(it);
			break;
		}
	}
}

Quest* GameObject::FindQuest(uint32 quest_id, uint8 quest_relation)
{   
	list< QuestRelation* >::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		QuestRelation* ptr = (*it);
		if( ( ptr->qst->id == quest_id ) && ( ptr->type & quest_relation ) )
		{
			return ptr->qst;
		}
	}
	return NULL;
}

uint16 GameObject::GetQuestRelation(uint32 quest_id)
{
	uint16 quest_relation = 0;
	list< QuestRelation* >::iterator it;
	for( it = m_quests->begin(); it != m_quests->end(); ++it )
	{
		if( (*it) != NULL && (*it)->qst->id == quest_id )
		{
			quest_relation |= (*it)->type;
		}
	}
	return quest_relation;
}

uint32 GameObject::NumOfQuests()
{
	return (uint32)m_quests->size();
}

void GameObject::_LoadQuests()
{
	sQuestMgr.LoadGOQuests(this);
}

/**
* Summoned Go's
*/
void GameObject::_Expire()
{
	sEventMgr.RemoveEvents(this);

	if( IsInWorld() )
		RemoveFromWorld(true);

	//sEventMgr.AddEvent(World::getSingletonPtr(), &World::DeleteObject, ((Object*)this), EVENT_DELETE_TIMER, 1000, 1);
	delete this;
	//this = NULL;
}

void GameObject::ExpireAndDelete()
{
	if( m_deleted )
		return;

	m_deleted = true;
	
	//! remove any events
	sEventMgr.RemoveEvents(this);
	sEventMgr.AddEvent(this, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

//! Deactivates selected gameobject ex. stops doors from opening/closing.
void GameObject::Deactivate()
{
	SetUInt32Value(GAMEOBJECT_DYNAMIC, 0);
}

void GameObject::CallScriptUpdate()
{
	Arcemu::Util::ARCEMU_ASSERT(   myScript != NULL );
	myScript->AIUpdate();
}

void GameObject::OnPushToWorld()
{
	Object::OnPushToWorld();
	CALL_INSTANCE_SCRIPT_EVENT( m_mapMgr, OnGameObjectPushToWorld )( this );
}

void GameObject::OnRemoveInRangeObject(Object* pObj)
{
	Object::OnRemoveInRangeObject(pObj);
	if(m_summonedGo && m_summoner == pObj)
	{
		for(int i = 0; i < 4; i++)
			if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
				m_summoner->m_ObjectSlots[i] = 0;

		m_summoner = 0;
		ExpireAndDelete();
	}
}
//! Remove gameobject from world, using their despawn animation.
void GameObject::RemoveFromWorld(bool free_guid)
{
	WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
	data << GetGUID();
	SendMessageToSet(&data,true);

	sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_TRAP_SEARCH_TARGET);
	Object::RemoveFromWorld(free_guid);
}

//! Gameobject contains loot ex. chest
bool GameObject::HasLoot()
{
	if( loot.gold > 0 )
		return true;

	for(vector<__LootItem>::iterator itr = loot.items.begin(); itr != loot.items.end(); ++itr)
	{
		if( itr->item.itemproto->Bonding == ITEM_BIND_QUEST || itr->item.itemproto->Bonding == ITEM_BIND_QUEST2 )
			continue;

		if( itr->iItemsCount > 0 )
			return true;
	}
	return false;
}

uint32 GameObject::GetGOReqSkill()  
{
	//! Hardcoded values are BAD.
	if(GetEntry() == 180215) 
		return 300;

	//! Gameobject does not require a skill, so let everyone use it.
	if(GetInfo() == NULL)
		return 0;
	
	//! Here we check the SpellFocus table against the dbcs
	Lock *lock = dbcLock.LookupEntryForced( GetInfo()->SpellFocus );
	if(!lock) return 0;
	for(uint32 i= 0;i<5;i++)
		if(lock->locktype[i] == 2 && lock->minlockskill[i])
		{
			return lock->minlockskill[i];
		}
	return 0;
}
//! Set GameObject rotational value
void GameObject::SetRotation(float rad)
{
	if (rad > (float)M_PI)
		rad -= 2*(float)M_PI;
	else if (rad < -(float)M_PI)
		rad += 2*(float)M_PI;
	float sin = sinf(rad/2.f);

	if(sin >= 0)	
		rad = 1.f + 0.125f * sin;
	else
		rad = 1.25f + 0.125f * sin;
//	SetFloatValue(GAMEOBJECT_ROTATION, rad);
}

void GameObject::UpdateRotation()
{
	static double const atan_pow = atan(pow(2.0f, -20.0f));

	double f_rot1 = sin(GetOrientation() / 2.0f);
	double f_rot2 = cos(GetOrientation() / 2.0f);

	int64 i_rot1 = int64(f_rot1 / atan_pow *(f_rot2 >= 0 ? 1.0f : -1.0f));
	int64 rotation = (i_rot1 << 43 >> 43) & 0x00000000001FFFFF;

	m_rotation = rotation;

	float r2=GetParentRotation(2);
	float r3=GetParentRotation(3);
	if(r2== 0.0f && r3== 0.0f && !(m_overrides & GAMEOBJECT_OVERRIDE_PARENTROT) )
	{
		r2 = (float)f_rot1;
		r3 = (float)f_rot2;
		SetParentRotation(2, r2);
		SetParentRotation(3, r3);
	}
}

void GameObject::SetState(uint8 state)
{
	SetByte(GAMEOBJECT_BYTES_1, 0, state);
}

uint8 GameObject::GetState()
{
	return GetByte(GAMEOBJECT_BYTES_1, 0);
}

