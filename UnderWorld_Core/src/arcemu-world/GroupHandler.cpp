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

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GROUP_INVITE
//////////////////////////////////////////////////////////////
void WorldSession::HandleGroupInviteOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	CHECK_PACKET_SIZE(recv_data, 1);
	WorldPacket data(100);
	std::string membername;
	Group *group = NULL;

	recv_data >> membername;
	if( _player->HasBeenInvited() )
		return;

	Player * player = objmgr.GetPlayer( membername.c_str(), false );

	if( player == NULL)
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		return;
	}

	if(player->GetSession()->m_gmData->rank == RANK_SYRA && m_gmData->rank < RANK_ADMIN)
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		return;
	}

	if( player == _player )
	{
		return;
	}

	if ( _player->InGroup() && !_player->IsGroupLeader() )
	{
		SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
		return;
	}

	group = _player->GetGroup();
	if ( group != NULL )
	{
		if (group->IsFull())
		{
			SendPartyCommandResult(_player, 0, "", ERR_PARTY_IS_FULL);
			return;
		}
	}
	
	if ( player->InGroup() )
	{
		SendPartyCommandResult(_player, player->GetGroup()->GetGroupType(), membername, ERR_PARTY_ALREADY_IN_GROUP);
		data.SetOpcode(SMSG_GROUP_INVITE);
		data << uint8(0);
		data << GetPlayer()->GetName();
		player->GetSession()->SendPacket(&data);
		return;
	}
	
	if(sWorld.dark_light_enabled)
	{
		if( (_player->bLight && player->bDark)  || (_player->bDark && player->bLight))
		{
			sChatHandler.RedSystemMessage(this,"ERROR: Your invitee is not of the same faction.");
			SendPartyCommandResult(_player, 0, membername, ERR_PARTY_WRONG_FACTION);
			return;
		}
	}
	else
	{
		if(player->GetTeam()!=_player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.interfaction_group)
		{
			SendPartyCommandResult(_player, 0, membername, ERR_PARTY_WRONG_FACTION);
			return;
		}
	}

	if ( player->HasBeenInvited() )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_ALREADY_IN_GROUP);
		return;
	}

	if( player->Social_IsIgnoring( _player->GetLowGUID() ) )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_IGNORING_YOU);
		return;
	}
	
	if( player->bGMTagOn && !_player->GetSession()->HasPermissions())
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		return;
	}
	
	data.SetOpcode(SMSG_GROUP_INVITE);
	data << uint8(1);
	data << GetPlayer()->GetName();

	player->GetSession()->SendPacket(&data);

	uint32 gtype = 0;
	if(group)
		gtype = group->GetGroupType();

	SendPartyCommandResult(_player, gtype, membername, ERR_PARTY_NO_ERROR);

	// 16/08/06 - change to guid to prevent very unlikely event of a crash in deny, etc
	player->SetInviter(_player->GetLowGUID());
}

///////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_CANCEL:
///////////////////////////////////////////////////////////////
void WorldSession::HandleGroupCancelOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	sLog.outDebug( "WORLD: got CMSG_GROUP_CANCEL." );
}

////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_ACCEPT:
////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupAcceptOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;

	Player *player = objmgr.GetPlayer(_player->GetInviter());
	if ( !player )
		return;
	
	player->SetInviter(0);
	_player->SetInviter(0);
	
	Group *grp = player->GetGroup();

	if(grp)
	{
		grp->AddMember(_player->m_playerInfo);
		_player->iInstanceType = grp->m_difficulty;
		_player->SendDungeonDifficulty();

        //sInstanceSavingManager.ResetSavedInstancesForPlayer(_player);
		return;
	}
	
	// If we're this far, it means we have no existing group, and have to make one.
	grp = new Group(true);
	grp->m_difficulty = static_cast<uint8>( player->iInstanceType );
	grp->AddMember(player->m_playerInfo);		// add the inviter first, therefore he is the leader
	grp->AddMember(_player->m_playerInfo);	   // add us.
	_player->iInstanceType = grp->m_difficulty;
	_player->SendDungeonDifficulty();

	Instance *instance = sInstanceMgr.GetInstanceByIds(player->GetMapId(), player->GetInstanceID());
	if(instance != NULL && instance->m_creatorGuid == player->GetLowGUID())
	{
		grp->m_instanceIds[instance->m_mapId][instance->m_difficulty] = instance->m_instanceId;
		instance->m_creatorGroup = grp->GetID();
		instance->m_creatorGuid = 0;
		instance->SaveToDB();
	}

    //sInstanceSavingManager.ResetSavedInstancesForPlayer(_player);

	// Currentgroup and all that shit are set by addmember.
}

///////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DECLINE:
//////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDeclineOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	WorldPacket data(SMSG_GROUP_DECLINE, 100);

	Player *player = objmgr.GetPlayer(_player->GetInviter());
	if(!player) return;

	data << GetPlayer()->GetName();

	player->GetSession()->SendPacket( &data );
	player->SetInviter(0);
	_player->SetInviter(0);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE(unused since 3.1.3):
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	CHECK_PACKET_SIZE(recv_data, 1);
	std::string membername;
	Group *group;
	Player * player;
	PlayerInfo * info;

	recv_data >> membername;

	player = objmgr.GetPlayer(membername.c_str(), false);
	info = objmgr.GetPlayerInfoByName(membername.c_str());
	if ( player == NULL && info == NULL )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		return;
	}

	if ( !_player->InGroup() || info->m_Group != _player->GetGroup() )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
		return;
	}

	if ( !_player->IsGroupLeader() )
	{
		if (player == NULL)
		{
			SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
			return;
		}
		else if(_player != player)
		{
			SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
			return;
		}
	}

	group = _player->GetGroup();

	if(group)
	{
		group->RemovePlayer(info);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE_GUID(used since 3.1.3):
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupUninviteGuidOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	CHECK_PACKET_SIZE(recv_data, 1);
	uint64 PlayerGUID;
	std::string membername = "unknown";
	Group *group;
	Player * player;
	PlayerInfo * info;

	recv_data >> PlayerGUID;

	player = objmgr.GetPlayer( Arcemu::Util::GUID_LOPART(PlayerGUID));
	info = objmgr.GetPlayerInfo( Arcemu::Util::GUID_LOPART(PlayerGUID));
	// If both conditions match the player gets thrown out of the group by the server since this means the character is deleted
	if ( player == NULL && info == NULL )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		return;
	}

	membername = player ? player->GetName() : info->name;

	if ( !_player->InGroup() || info->m_Group != _player->GetGroup() )
	{
		SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
		return;
	}

	if ( !_player->IsGroupLeader() )
	{
		if (player == NULL)
		{
			SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
			return;
		}
		else if(_player != player)
		{
			SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
			return;
		}
	}

	group = _player->GetGroup();
	if(group)
	{
		group->RemovePlayer(info);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_SET_LEADER:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupSetLeaderOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	// important note _player->GetName() can be wrong.
	CHECK_PACKET_SIZE(recv_data, 1);
	WorldPacket data;
	uint64 MemberGuid;
	Player * player;

	recv_data >> MemberGuid;
	
	player = objmgr.GetPlayer((uint32)MemberGuid);

	if ( player == NULL )
	{
		//SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
		SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_CANNOT_FIND);
		return;
	}

	if(!_player->IsGroupLeader())
	{
		SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
		return;
	}
	
	if(player->GetGroup() != _player->GetGroup())
	{
		//SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
		SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
		return;
	}

	Group *pGroup = _player->GetGroup();
	if(pGroup)
		pGroup->SetLeader(player,false);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DISBAND:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupDisbandOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	Group* pGroup = _player->GetGroup();
	if(!pGroup) return;

	//pGroup->Disband();
	pGroup->RemovePlayer(_player->m_playerInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_LOOT_METHOD:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleLootMethodOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	CHECK_PACKET_SIZE(recv_data, 16);
	uint32 lootMethod;
	uint64 lootMaster;
	uint32 threshold;

	recv_data >> lootMethod >> lootMaster >>threshold;
  
	if(!_player->IsGroupLeader())
	{
		SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
		return;
	}
	
	Group* pGroup = _player->GetGroup(); 

	if( pGroup == NULL)
		return;

	/*Player * pLootMaster = objmgr.GetPlayer((uint32)lootMaster);

	if ( pLootMaster )
		pGroup->SetLooter(pLootMaster , lootMethod, threshold );
	else
		pGroup->SetLooter(_player , lootMethod, threshold );*/

  // cebernic: Extended this code,it supports diff leader & lootmaster.
  Player *plr = objmgr.GetPlayer((uint32)lootMaster);
  if ( _player->m_playerInfo->guid == lootMaster || !plr) {
    Group* pGroup = _player->GetGroup();
    if ( !pGroup ) return;
    pGroup->SetLooter(_player, static_cast<uint8>( lootMethod ), static_cast<uint16>( threshold ));
  }
  else {
    Group* pGroup = plr->GetGroup();
    if ( !pGroup ) 
		return;
    pGroup->SetLooter(plr, static_cast<uint8>( lootMethod ), static_cast<uint16>( threshold ));
  }

}

void WorldSession::HandleMinimapPingOpcode( WorldPacket & recv_data )
{
	if( !_player->IsInWorld() ) 
		return;
	CHECK_PACKET_SIZE(recv_data, 8);
	if( !_player->InGroup() )
	return;
	Group * party= _player->GetGroup();
	if(!party)return;

	float x,y;
	recv_data >> x >>y;
	WorldPacket data;
	data.SetOpcode(MSG_MINIMAP_PING);
	data << _player->GetGUID();
	data << x << y;
	party->SendPacketToAllButOne(&data, _player);
}

void WorldSession::HandleSetPlayerIconOpcode(WorldPacket& recv_data)
{
	uint64 guid;
	uint8 icon;
	Group * pGroup = _player->GetGroup();
	if(!_player->IsInWorld() || !pGroup) return;

	recv_data >> icon;
	if(icon == 0xFF)
	{
		// client request
		WorldPacket data(MSG_RAID_TARGET_UPDATE, 73);
		data << uint8(1);
		for(uint8 i = 0; i < 8; ++i)
			data << i << pGroup->m_targetIcons[i];

		SendPacket(&data);
	}
	else if(_player->IsGroupLeader())
	{
		recv_data >> guid;
		if(icon > 7)
			return;			// whoops, buffer overflow :p

		// setting icon
		WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
		data << uint8(0) << icon << guid;
		pGroup->SendPacketToAll(&data);

		pGroup->m_targetIcons[icon] = guid;
	}
}

void WorldSession::SendPartyCommandResult(Player *pPlayer, uint32 p1, std::string name, uint32 err)
{
	if(!_player->IsInWorld()) return;
	// if error message do not work, please sniff it and leave me a message
	if(pPlayer)
	{
		WorldPacket data;
		data.Initialize(SMSG_PARTY_COMMAND_RESULT);
		data << p1;
		if(!name.length())
			data << uint8(0);
		else
			data << name.c_str();

		data << err;
		pPlayer->GetSession()->SendPacket(&data);
	}
}
