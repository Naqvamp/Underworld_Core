/*
* arcemu MMORPG Server
* Copyright (C) 2005-2007 arcemu Team <http://www.arcemuemu.com/>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "StdAfx.h"

void Player::SendTalentResetConfirm()
{
	WorldPacket data( MSG_TALENT_WIPE_CONFIRM, 12 );
	
    data << uint64( GetGUID() );
	data << uint32( CalcTalentResetCost( GetTalentResetTimes() ) );
	
    m_session->SendPacket(&data);
}

void Player::SendPetUntrainConfirm()
{
    Pet* pPet = GetSummon();

	if( pPet == NULL )
		return;

	WorldPacket data( SMSG_PET_UNLEARN_CONFIRM, 12 );

	data << uint64( pPet->GetGUID() );
	data << uint32( pPet->GetUntrainCost() );

	m_session->SendPacket( &data );
}

void Player::SendWorldStateUpdate(uint32 WorldState, uint32 Value)
{
    WorldPacket data( SMSG_UPDATE_WORLD_STATE, 8 );
    
    data << uint32( WorldState );
    data << uint32( Value );

    m_session->SendPacket( &data );
}


void Player::Gossip_SendPOI(float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char* Name)
{
	size_t namelen = 0;

	if( Name != NULL )
		namelen = strlen( Name );
   
	WorldPacket data( SMSG_GOSSIP_POI, 11 + namelen );

	data << uint32( Flags );
	data << float( X );
	data << float( Y );
	data << uint32( Icon );
	data << uint32( Data );

	if( namelen == 0 )
		data << uint8( 0 );
	else
		data.append( (const uint8*)Name, namelen + 1 );

	m_session->SendPacket( &data );
}
  
void Player::SendLevelupInfo(uint32 level, uint32 Hp, uint32 Mana, uint32 Stat0, uint32 Stat1, uint32 Stat2, uint32 Stat3, uint32 Stat4)
{
    WorldPacket data( SMSG_LEVELUP_INFO, 14 * 4 );

    data << uint32( level );
    data << uint32( Hp );
    data << uint32( Mana );
    
    for( int i = 0; i < 6; ++i )
        data << uint32( 0 );

    data << uint32( Stat0 );
    data << uint32( Stat1 );
    data << uint32( Stat2 );
    data << uint32( Stat3 );
    data << uint32( Stat4 );

    m_session->SendPacket( &data );
}

void Player::SendLogXPGain(uint64 guid, uint32 NormalXP, uint32 RestedXP, bool type)
{
    WorldPacket data( SMSG_LOG_XPGAIN, 24 );

    if (type == false)
    {
        data << uint64( guid );
        data << uint32( NormalXP );
        
        if( type )
            data << uint8( 1 );
        else
            data << uint8( 0 );

        data << uint32( RestedXP );
        data << float( 1.0f );

    }
    else if (type == true)
    {
        data << uint64( 0 );          // does not need to be set for questxp
        data << uint32( NormalXP );
        
        if( type )
            data << uint8( 1 );
        else
            data << uint8( 0 );

        data << uint8( 0 );

    }

    m_session->SendPacket( &data );
}

// this one needs to be send inrange...
void Player::SendEnvironmentalDamageLog(const uint64 & guid, uint8 type, uint32 damage){

    WorldPacket data( SMSG_ENVIRONMENTALDAMAGELOG, 20 );

    data << uint64( guid );
    data << uint8( type );
    data << uint32( damage );
    data << uint64( 0 );

    SendMessageToSet( &data, true, false );
}


void Player::SendCastResult(uint32 SpellId, uint8 ErrorMessage, uint8 MultiCast, uint32 Extra){

    WorldPacket data( SMSG_CAST_FAILED, 80 );

	data << uint8( MultiCast );
    data << uint32( SpellId );
    data << uint8( ErrorMessage );
    
    if( Extra )
        data << uint32( Extra );

    m_session->SendPacket( &data );
}

void Player::SendSpellCooldownEvent(uint32 SpellId){
    
    WorldPacket data( SMSG_COOLDOWN_EVENT, 12 );

    data << uint32( SpellId );
    data << uint64( GetGUID() );

    m_session->SendPacket( &data );
}


void Player::SendFlatSpellModifier(uint8 spellgroup, uint8 spelltype, int32 v){
    
    WorldPacket data( SMSG_SET_FLAT_SPELL_MODIFIER, 48 );

    data << uint8( spellgroup );
    data << uint8( spelltype );
    data << uint32( v );

    m_session->SendPacket( &data );
}

void Player::SendItemPushResult( bool created, bool recieved, bool sendtoset, bool newitem, uint8 destbagslot, uint32 destslot, uint32 count, uint32 entry, uint32 suffix, uint32 randomprop, uint32 stack ){

    WorldPacket data( SMSG_ITEM_PUSH_RESULT, 8 + 4 + 4 + 4 + 1 + 4 + 4 + 4 + 4 + 4 + 4 );

    data << uint64( GetGUID() );

    if( recieved )
        data << uint32( 1 );
    else
        data << uint32( 0 );

    if( created )
        data << uint32( 1 );
    else
        data << uint32( 0 );

    data << uint32( 1 );
    data << uint8( destbagslot );

    if( newitem )
        data << uint32( destslot );
    else
        data << uint32( -1 );

    data << uint32( entry );
    data << uint32( suffix );
    data << uint32( randomprop );
    data << uint32( count );
    data << uint32( stack );

    if( sendtoset && InGroup() )
        GetGroup()->SendPacketToAll( &data );
    else
        m_session->SendPacket( &data );

}

void Player::SendSetProficiency( uint8 ItemClass, uint32 Proficiency ){

    WorldPacket data( SMSG_SET_PROFICIENCY, 40 );

    data << uint8( ItemClass );
    data << uint32( Proficiency );

    m_session->SendPacket( &data );
}

void Player::SendLoginVerifyWorld(uint32 MapId, float X, float Y, float Z, float O){

    WorldPacket data( SMSG_LOGIN_VERIFY_WORLD, 20 );

    data << uint32( MapId );
    data << float( X );
    data << float( Y );
    data << float( Z );
    data << float( O );

    m_session->SendPacket( &data );
}

void Player::SendPlaySpellVisual(uint64 guid, uint32 visualid){

    WorldPacket data( SMSG_PLAY_SPELL_VISUAL, 12 );

    data << uint64( guid );
    data << uint32( visualid );

    SendMessageToSet( &data, true, false );
}

void Player::SendDungeonDifficulty(){

    WorldPacket data(MSG_SET_DUNGEON_DIFFICULTY, 12);
	
	data << uint32( iInstanceType );
    data << uint32( 1 );
    data << uint32( InGroup() );

    m_session->SendPacket(&data);
}

void Player::SendRaidDifficulty()
{
    WorldPacket data(MSG_SET_RAID_DIFFICULTY, 12);
	
	data << uint32( m_RaidDifficulty );
    data << uint32( 1 );
    data << uint32( InGroup() );
    
	m_session->SendPacket(&data);
}

void Player::SendNewDrunkState(uint32 state, uint32 itemid){

    WorldPacket data( SMSG_CROSSED_INEBRIATION_THRESHOLD, (8+4+4) );

	data << GetNewGUID();
	data << uint32( state );
	data << uint32( itemid );

	SendMessageToSet( &data, true );
}

/*Loot type MUST be
1-corpse, go
2-skinning/herbalism/minning
3-Fishing
*/
void Player::SendLoot(uint64 guid,uint8 loot_type)
{
	Group * m_Group = m_playerInfo->m_Group;

	if( !IsInWorld() )
		return;

	Loot * pLoot = NULL;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);
	int8 loot_method = -1;

	if(guidtype == HIGHGUID_TYPE_UNIT)
	{
		Creature* pCreature = GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(!pCreature)return;
		pLoot=&pCreature->loot;
		m_currentLoot = pCreature->GetGUID();
		loot_method = pCreature->m_lootMethod;
		if ( loot_method < 0 )
		{
			loot_method = PARTY_LOOT_FFA;
			pCreature->m_lootMethod = PARTY_LOOT_FFA;
		}
	}else if(guidtype == HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* pGO = GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(!pGO)return;
		pGO->SetByte(GAMEOBJECT_BYTES_1, 0,0);
		pLoot=&pGO->loot;
		m_currentLoot = pGO->GetGUID();
	}
	else if((guidtype == HIGHGUID_TYPE_PLAYER) )
	{
		Player *p=GetMapMgr()->GetPlayer((uint32)guid);
		if(!p)return;
		pLoot=&p->loot;
		m_currentLoot = p->GetGUID();
	}
	else if( (guidtype == HIGHGUID_TYPE_CORPSE))
	{
		Corpse *pCorpse = objmgr.GetCorpse((uint32)guid);
		if(!pCorpse)return;
		pLoot=&pCorpse->loot;
		m_currentLoot = pCorpse->GetGUID();
	}
	else if( (guidtype == HIGHGUID_TYPE_ITEM) )
	{
		Item *pItem = GetItemInterface()->GetItemByGUID(guid);
		if(!pItem)
			return;
		pLoot = pItem->loot;
		m_currentLoot = pItem->GetGUID();
	}

	if(!pLoot)
	{
		// something whack happened.. damn cheaters..
		return;
	}

	// add to looter set
	pLoot->looters.insert( GetLowGUID() );

	WorldPacket data, data2(32);
	data.SetOpcode( SMSG_LOOT_RESPONSE );


	m_lootGuid = guid;


	data << uint64( guid );
	data << uint8( loot_type );//loot_type;
	data << uint32( pLoot->gold );
	data << uint8( 0 ); //loot size reserve


	std::vector<__LootItem>::iterator iter=pLoot->items.begin();
	uint32 count= 0;
	uint8 slottype = 0;

	for(uint32 x= 0;iter!=pLoot->items.end();iter++,x++)
	{
		if (iter->iItemsCount == 0)
			continue;

		LooterSet::iterator itr = iter->has_looted.find(GetLowGUID());
		if (iter->has_looted.end() != itr)
			continue;

		ItemPrototype* itemProto =iter->item.itemproto;
		if (!itemProto)
			continue;

		// check if it's on ML if so only quest items and ffa loot should be shown based on mob
		if ( loot_method == PARTY_LOOT_MASTER && m_Group && m_Group->GetLooter() != m_playerInfo )
			// pass on all ffa_loot and the grey / white items
			if ( !iter->ffa_loot && !(itemProto->Quality < m_Group->GetThreshold()) )
				continue;

		//quest items check. type 4/5
		//quest items that don't start quests.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;
        if((itemProto->Bonding == ITEM_BIND_QUEST2) && !(itemProto->QuestId) && !HasQuestForItem(itemProto->ItemId))
            continue;

        //quest items that start quests need special check to avoid drops all the time.
        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;
        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && GetQuestLogForEntry(itemProto->QuestId))
            continue;

        if((itemProto->Bonding == ITEM_BIND_QUEST) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;
        if((itemProto->Bonding == ITEM_BIND_QUEST2) && (itemProto->QuestId) && HasFinishedQuest(itemProto->QuestId))
            continue;

        //check for starting item quests that need questlines.
        if((itemProto->QuestId && itemProto->Bonding != ITEM_BIND_QUEST && itemProto->Bonding != ITEM_BIND_QUEST2))
        {
            bool HasRequiredQuests = true;
            Quest * pQuest = QuestStorage.LookupEntry(itemProto->QuestId);
            if(pQuest)
            {
				uint32 finishedCount = 0;

                //check if its a questline.
                for(uint32 i = 0; i < pQuest->count_requiredquests; i++)
                {
                    if(pQuest->required_quests[i])
                    {
                        if(!HasFinishedQuest(pQuest->required_quests[i]) || GetQuestLogForEntry(pQuest->required_quests[i]))
                        {
							if (!(pQuest->quest_flags & QUEST_FLAG_ONLY_ONE_REQUIRED)) {
								HasRequiredQuests = false;
								break;
							}
						}
						else
						{
							finishedCount++;
						}
                    }
                }

				if (pQuest->quest_flags & QUEST_FLAG_ONLY_ONE_REQUIRED) {
					if (finishedCount == 0) continue;
				} else {
	                if(!HasRequiredQuests)
    	                continue;
				}
            }
        }

		slottype = 0;
		if(m_Group != NULL && loot_type < 2)
		{
			switch(loot_method)
			{
			case PARTY_LOOT_MASTER:
				slottype = 2;
				break;
			case PARTY_LOOT_GROUP:
			case PARTY_LOOT_RR:
			case PARTY_LOOT_NBG:
				slottype = 1;
				break;
			default:
				slottype = 0;
				break;
			}
			// only quality items are distributed
			if(itemProto->Quality < m_Group->GetThreshold())
			{
				slottype = 0;
			}

			// if all people passed anyone can loot it? :P
			if(iter->passed)
				slottype = 0;					// All players passed on the loot

			//if it is ffa loot and not an masterlooter
			if(iter->ffa_loot)
				slottype = 0;
		}

		data << uint8( x );
		data << uint32( itemProto->ItemId );
		data << uint32( iter->iItemsCount );//nr of items of this type
		data << uint32( iter->item.displayid );

        if(iter->iRandomSuffix)
		{
			data << uint32( Item::GenerateRandomSuffixFactor( itemProto ) );
			data << uint32( -int32( iter->iRandomSuffix->id ) );
		}
		else if(iter->iRandomProperty)
		{
			data << uint32( 0 );
			data << uint32( iter->iRandomProperty->ID );
		}
		else
		{
			data << uint32( 0 );
			data << uint32( 0 );
		}

		data << slottype;   // "still being rolled for" flag

		if(slottype == 1)
		{
			if(iter->roll == NULL && !iter->passed)
			{
				int32 ipid = 0;
				uint32 factor= 0;
				if(iter->iRandomProperty)
					ipid=iter->iRandomProperty->ID;
				else if(iter->iRandomSuffix)
				{
					ipid = -int32(iter->iRandomSuffix->id);
					factor=Item::GenerateRandomSuffixFactor(iter->item.itemproto);
				}

				if(iter->item.itemproto)
				{
					iter->roll = new LootRoll(60000, (m_Group != NULL ? m_Group->MemberCount() : 1),  guid, x, itemProto->ItemId, factor, uint32(ipid), GetMapMgr());

					data2.Initialize(SMSG_LOOT_START_ROLL);
					data2 << guid;
					data2 << uint32( x );
					data2 << uint32( itemProto->ItemId );
					data2 << uint32( factor );
					if(iter->iRandomProperty)
						data2 << uint32(iter->iRandomProperty->ID);
					else if(iter->iRandomSuffix)
						data2 << uint32( ipid );
					else
						data2 << uint32( 0 );

					data2 << uint32( iter->iItemsCount );
					data2 << uint32( 60000 ); // countdown
				}

				Group * pGroup = m_playerInfo->m_Group;
				if(pGroup)
				{
					pGroup->Lock();
					for(uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
					{
						for(GroupMembersSet::iterator itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
						{

                            PlayerInfo *pinfo = *itr;

							if( pinfo->m_loggedInPlayer && pinfo->m_loggedInPlayer->GetItemInterface()->CanReceiveItem( itemProto, iter->iItemsCount ) == 0 )
							{
								if( pinfo->m_loggedInPlayer->m_passOnLoot )
									iter->roll->PlayerRolled( pinfo->m_loggedInPlayer, 3 );		// passed
								else
									pinfo->m_loggedInPlayer->SendPacket( &data2 );
							}
						}
					}
					pGroup->Unlock();
				}
				else
				{
					m_session->SendPacket(&data2);
				}
			}
		}
		count++;
	}
	data.wpos(13);
	data << uint8( count );

	m_session->SendPacket(&data);

	SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING );
}

void Player::SendInitialLogonPackets()
{
	// Initial Packets... they seem to be re-sent on port.
	//m_session->OutPacket(SMSG_SET_REST_START_OBSOLETE, 4, &m_timeLogoff); // Seem to be unused by client

    StackWorldPacket<32> data( SMSG_BINDPOINTUPDATE );

    data << float( m_bind_pos_x );
    data << float( m_bind_pos_y );
    data << float( m_bind_pos_z );
    data << uint32( m_bind_mapid );
    data << uint32( m_bind_zoneid );

    m_session->SendPacket( &data );

	//Proficiencies
    SendSetProficiency( 4, armor_proficiency);
    SendSetProficiency( 2, weapon_proficiency);
	
	//Tutorial Flags
	data.Initialize( SMSG_TUTORIAL_FLAGS );
	
    for (int i = 0; i < 8; i++)
		data << uint32( m_Tutorials[i] );

	m_session->SendPacket(&data);

	smsg_TalentsInfo( false );
	smsg_InitialSpells();

	data.Initialize(SMSG_SEND_UNLEARN_SPELLS);
	data << uint32(0); // count, for(count) uint32;
	GetSession()->SendPacket( &data );

	SendInitialActions();
	smsg_InitialFactions();

    /* Some minor documentation about the time field
    // MOVE THIS DOCUMENTATION TO THE WIKI

    minute's = 0x0000003F                  00000000000000000000000000111111
    hour's   = 0x000007C0                  00000000000000000000011111000000
    weekdays = 0x00003800                  00000000000000000011100000000000
    days     = 0x000FC000                  00000000000011111100000000000000
    months   = 0x00F00000                  00000000111100000000000000000000
    years    = 0x1F000000                  00011111000000000000000000000000
    unk	     = 0xE0000000                  11100000000000000000000000000000
    */

	data.Initialize(SMSG_LOGIN_SETTIMESPEED);

	time_t minutes = sWorld.GetGameTime( ) / 60;
	time_t hours = minutes / 60; minutes %= 60;
    time_t gameTime = 0;

    // TODO: Add stuff to handle these variables

	time_t basetime = UNIXTIME;
	uint32 DayOfTheWeek;
	if(localtime(&basetime)->tm_wday == 0)
		DayOfTheWeek = 6;
	else
		   DayOfTheWeek = localtime(&basetime)->tm_wday - 1;
	uint32 DayOfTheMonth = localtime(&basetime)->tm_mday - 1;
	uint32 CurrentMonth = localtime(&basetime)->tm_mon;
	uint32 CurrentYear = localtime(&basetime)->tm_year - 100;

	#define MINUTE_BITMASK      0x0000003F
    #define HOUR_BITMASK        0x000007C0
    #define WEEKDAY_BITMASK     0x00003800
    #define DAY_BITMASK         0x000FC000
    #define MONTH_BITMASK       0x00F00000
    #define YEAR_BITMASK        0x1F000000
    #define UNK_BITMASK         0xE0000000

    #define MINUTE_SHIFTMASK    0
    #define HOUR_SHIFTMASK      6
    #define WEEKDAY_SHIFTMASK   11
    #define DAY_SHIFTMASK       14
    #define MONTH_SHIFTMASK     20
    #define YEAR_SHIFTMASK      24
    #define UNK_SHIFTMASK       29

    gameTime = ((minutes << MINUTE_SHIFTMASK) & MINUTE_BITMASK);
    gameTime|= ((hours << HOUR_SHIFTMASK) & HOUR_BITMASK);
    gameTime|= ((DayOfTheWeek << WEEKDAY_SHIFTMASK) & WEEKDAY_BITMASK);
    gameTime|= ((DayOfTheMonth << DAY_SHIFTMASK) & DAY_BITMASK);
    gameTime|= ((CurrentMonth << MONTH_SHIFTMASK) & MONTH_BITMASK);
    gameTime|= ((CurrentYear << YEAR_SHIFTMASK) & YEAR_BITMASK);

    data << uint32( gameTime );
	data << float( 0.0166666669777748f );  // Normal Game Speed
	data << uint32( 0 ); // 3.1.2
	
    m_session->SendPacket( &data );

	// cebernic for speedhack bug
	m_lastRunSpeed = 0;
	UpdateSpeed();

    WorldPacket ArenaSettings(SMSG_UPDATE_WORLD_STATE, 16);

    ArenaSettings << uint32( 0xC77 );
    ArenaSettings << uint32( sWorld.Arena_Progress );
    ArenaSettings << uint32( 0xF3D );
    ArenaSettings << uint32( sWorld.Arena_Season );

    m_session->SendPacket( &ArenaSettings );

	sLog.outDetail("WORLD: Sent initial logon packets for %s.", GetName());
}
