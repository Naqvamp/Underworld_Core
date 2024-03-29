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

//
// WorldSession.cpp
//

#include "StdAfx.h"

OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];

WorldSession::WorldSession(uint32 id, string Name, WorldSocket *sock)
:
_player(0),
_socket(sock),
_accountId(id),
_accountName(Name),
_logoutTime(0),
permissions(NULL),
permissioncount(0),
_loggingOut(false),
LoggingOut(false),
instanceId(0),

m_currMsTime(getMSTime()),
bDeleted(false),
m_bIsWLevelSet(false),
floodLines(0),
floodTime(UNIXTIME),
_updatecount(0),
m_moveDelayTime(0),
m_clientTimeDelay(0),
m_loggingInPlayer(NULL),
language(0),
m_muted(0),
_side(-1),
has_level_55_char(false)

{
	memset(movement_packet, 0, sizeof(movement_packet));

	m_gmData = new GMStatus();
	m_uwflags = new UWFlags();
	m_points = 0;
	floodMuteCheck = false;

	movement_info.FallTime = 0;

	for( uint8 x = 0; x < 8; x++ )
		sAccountData[x].data = NULL;

}

WorldSession::~WorldSession()
{
	deleteMutex.Acquire();

	if(HasGMPermissions())
		sWorld.gmList.erase(this);

	if(_player)
	{
		sLog.outError("warning: logged out player in worldsession destructor");
		LogoutPlayer(true);
	}

	if(permissions)
		delete [] permissions;

	WorldPacket *packet;

	while((packet = _recvQueue.Pop()) != 0)
		delete packet;

	for(uint32 x= 0;x<8;x++)
	{
		if(sAccountData[x].data)
			delete [] sAccountData[x].data;
	}

	if(_socket)
		_socket->SetSession(0);

	if(m_loggingInPlayer)
		m_loggingInPlayer->SetSession(NULL);

	if (m_gmData)
		delete m_gmData;

	SavePoints();

	deleteMutex.Release();
}

int WorldSession::Update(uint32 InstanceID)
{
	m_currMsTime = getMSTime();

	if(!((++_updatecount) % 2) && _socket)
		_socket->UpdateQueuedPackets();

	WorldPacket *packet;
	OpcodeHandler * Handler;

	if(InstanceID != instanceId)
	{
		// We're being updated by the wrong thread.
		// "Remove us!" - 2
		return 2;
	}

	// Socket disconnection.
	if(!_socket)
	{
		// Check if the player is in the process of being moved. We can't delete him
		// if we are.
		if(_player && _player->m_beingPushed)
		{
			// Abort..
			return 0;
		}

		if(!_logoutTime)
			_logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;

/*
				if(_player && _player->DuelingWith)
					_player->EndDuel(DUEL_WINNER_RETREAT);
		
				bDeleted = true;
				LogoutPlayer(true);
				// 1 - Delete session completely.
				return 1;*/
		
	}

	while ((packet = _recvQueue.Pop()) != 0)
	{
		Arcemu::Util::ARCEMU_ASSERT(   packet != NULL );

		if(packet->GetOpcode() >= NUM_MSG_TYPES)
			sLog.outError("[Session] Received out of range packet with opcode 0x%.4X", packet->GetOpcode());
		else
		{
			Handler = &WorldPacketHandlers[packet->GetOpcode()];
			if(Handler->status == STATUS_LOGGEDIN && !_player && Handler->handler != 0)
			{
				sLog.outError("[Session] Received unexpected/wrong state packet with opcode %s (0x%.4X)",
					LookupName(packet->GetOpcode(), g_worldOpcodeNames), packet->GetOpcode());
			}
			else
			{
				// Valid Packet :>
				if(Handler->handler == 0)
				{
					sLog.outError("[Session] Received unhandled packet with opcode %s (0x%.4X)",
						LookupName(packet->GetOpcode(), g_worldOpcodeNames), packet->GetOpcode());
				}
				else
				{
					(this->*Handler->handler)(*packet);
				}
			}
		}

		delete packet;

		if(InstanceID != instanceId)
		{
			// If we hit this -> means a packet has changed our map.
			return 2;
		}

		if( bDeleted )
		{
			return 1;
		}
	}

	if(InstanceID != instanceId)
	{
		// If we hit this -> means a packet has changed our map.
		return 2;
	}

	if( _logoutTime && (m_currMsTime >= _logoutTime) && instanceId == InstanceID)
	{
		// Check if the player is in the process of being moved. We can't delete him
		// if we are.
		if(_player && _player->m_beingPushed)
		{
			// Abort..
			return 0;
		}

		if( _socket == NULL )
		{
			bDeleted = true;
			LogoutPlayer(true);
			return 1;
		}
		else
			LogoutPlayer(true);
	}

	if(m_lastPing + WORLDSOCKET_TIMEOUT < (uint32)UNIXTIME)
	{
		// Check if the player is in the process of being moved. We can't delete him
		// if we are.
		if(_player && _player->m_beingPushed)
		{
			// Abort..
			return 0;
		}

		// ping timeout!
		if( _socket != NULL )
		{
			Disconnect();
			_socket = NULL;
		}

		m_lastPing = (uint32)UNIXTIME;		// Prevent calling this code over and over.
		if(!_logoutTime)
			_logoutTime = m_currMsTime + PLAYER_LOGOUT_DELAY;
	}

	return 0;
}


void WorldSession::LogoutPlayer(bool Save)
{
	Player* pPlayer = GetPlayer();

	if( _loggingOut )
		return;

	_loggingOut = true;

	if( _player != NULL )
	{
		objmgr.RemovePlayer( _player );
		_player->ok_to_remove = true;
		
		sHookInterface.OnLogout( pPlayer );
		if( _player->DuelingWith )
			_player->EndDuel( DUEL_WINNER_RETREAT );

		if( _player->m_currentLoot && _player->IsInWorld() )
		{
			Object* obj = _player->GetMapMgr()->_GetObject( _player->m_currentLoot );
			if( obj != NULL )
			{
				switch( obj->GetTypeId() )
				{
				case TYPEID_UNIT:
					static_cast< Creature* >( obj )->loot.looters.erase( _player->GetLowGUID() );
					break;
				case TYPEID_GAMEOBJECT:
					static_cast< GameObject* >( obj )->loot.looters.erase( _player->GetLowGUID() );
					break;
				}
			}
		}

#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
		GM_Ticket * ticket = objmgr.GetGMTicketByPlayer(_player->GetGUID());
		if(ticket != NULL)
		{
			//Send status change to gm_sync_channel
			Channel *chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), _player);
			if(chn)
			{
				std::stringstream ss;
				ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ONLINESTATE;
				ss << ":" << ticket->guid;
				ss << ":0";
				chn->Say(_player, ss.str().c_str(), NULL, true);
			}
		}
#endif

		// part channels
		_player->CleanupChannels();

		if( _player->m_CurrentTransporter != NULL )
			_player->m_CurrentTransporter->RemovePlayer( _player );

		// cancel current spell
		if( _player->m_currentSpell != NULL )
			_player->m_currentSpell->cancel();

		_player->Social_TellFriendsOffline();

		// Decrement the global player count
		sWorld.decrementPlayerCount(_player->GetTeam());

		if( _player->m_bgIsQueued )
			BattlegroundManager.RemovePlayerFromQueues( _player );

		// Repop or Resurrect and remove from battlegrounds
		if( _player->m_bg )
		{
			if( pPlayer->getDeathState() == JUST_DIED )
				pPlayer->RemoteRevive();
			if( pPlayer->getDeathState() != ALIVE )
				pPlayer->ResurrectPlayer();
			_player->m_bg->RemovePlayer( _player, true );
		}
		else if( _player->IsDead() && _player->getDeathState() == JUST_DIED )
			_player->RepopRequestedPlayer();

		//Duel Cancel on Leave
		if( _player->DuelingWith != NULL )
			_player->EndDuel( DUEL_WINNER_RETREAT );

		//Issue a message telling all guild members that this player signed off
		if( _player->IsInGuild() )
		{
			Guild* pGuild = _player->m_playerInfo->guild;
			if( pGuild != NULL )
				pGuild->LogGuildEvent( GUILD_EVENT_HASGONEOFFLINE, 1, _player->GetName() );
		}

		_player->GetItemInterface()->EmptyBuyBack();
		
		sLfgMgr.RemovePlayerFromLfgQueues( _player );
		
		// Save HP/Mana
		_player->load_health = _player->GetUInt32Value( UNIT_FIELD_HEALTH );
		_player->load_mana = _player->GetPower(POWER_TYPE_MANA);

		_player->DismissActivePets();
		_player->RemoveAllGuardians();

		//_player->SaveAuras();

		if( Save )
			_player->SaveToDB(false);
		
		_player->RemoveAllAuras();
		if( _player->IsInWorld() )
			_player->RemoveFromWorld();
		
		_player->m_playerInfo->m_loggedInPlayer = NULL;

		if( _player->m_playerInfo->m_Group != NULL )
			_player->m_playerInfo->m_Group->Update();
	  
		// Remove the "player locked" flag, to allow movement on next login
		GetPlayer()->RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER );

		// Save Honor Points
		//_player->SaveHonorFields();

		// Update any dirty account_data fields.
		bool dirty = false;
		if( sWorld.m_useAccountData )
		{
			std::stringstream ss;
			ss << "UPDATE account_data SET ";
			for(uint32 ui= 0;ui<8;ui++)
			{
				if(sAccountData[ui].bIsDirty)
				{
					if(dirty)
						ss <<",";
					ss << "uiconfig"<< ui <<"=\"";
					if(sAccountData[ui].data)
					{
						CharacterDatabase.EscapeLongString(sAccountData[ui].data, sAccountData[ui].sz, ss);
						//ss.write(sAccountData[ui].data,sAccountData[ui].sz);
					}
					ss << "\"";
					dirty = true;
					sAccountData[ui].bIsDirty = false;
				}
			}			
			if(dirty)
			{
				ss	<<" WHERE acct="<< _accountId <<";";
				CharacterDatabase.ExecuteNA(ss.str().c_str());
			}
		}

		delete _player;
		_player = NULL;

		OutPacket(SMSG_LOGOUT_COMPLETE, 0, NULL);
		sLog.outDebug( "SESSION: Sent SMSG_LOGOUT_COMPLETE Message" );
	}
	_loggingOut = false;

	SetLogoutTimer(0);
}

void WorldSession::SendBuyFailed(uint64 guid, uint32 itemid, uint8 error)
{
	WorldPacket data(13);
	data.SetOpcode(SMSG_BUY_FAILED);
	data << guid << itemid << error;
	SendPacket(&data);
}

void WorldSession::SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error)
{
	WorldPacket data(17);
	data.SetOpcode(SMSG_SELL_ITEM);
	data << vendorguid << itemid << error;
	SendPacket(&data);
}

string IntToString(uint8 num)
{
	stringstream ss;
	ss << num;
	return ss.str();
}

void WorldSession::LoadSecurity(std::string securitystring)
{
	std::list<char> tmp;
	bool hasa = false;
	uint8 cnt = 0;
	for(uint32 i = 0; i < securitystring.length(); ++i)
	{
		char c = securitystring.at(i);
		if (c == '-')
		{
			cnt++;
			tmp.push_back(c);
			continue;
		}

		switch (cnt)
		{
		case 0: // <0>-1-2 This section is temp/perm or special strings
			switch (c)
			{
			case 'z':
			case 'w':
				if (!hasa)
					continue;
			case 'a':
				hasa = true;
			case 'd':
			case 't':
			case 'p':
				tmp.push_back(c);
			default:
				break;
			}
			break;
		case 1: // 0-<1>-2 anything allowed in here its the rank title is all. if busted -shtug-
			tmp.push_back(c);
			break;

		case 2: // 0-1-<2> set addon status and push anything back;
			if((c == '1') || (c == 'f'))
				m_gmData->addons[0] = true;
			else if((c == '2') || (c == 'b'))
				m_gmData->addons[1] = true;
			else if((c == '3') || (c == 'c'))
				m_gmData->addons[2] = true;
			else if((c == '4') || (c == 'e'))
				m_gmData->addons[3] = true;
			else if(c == 'm')
				m_gmData->mute = true;

			tmp.push_back(c);

				
		}
	}

	permissions = new char[tmp.size()+1];
	memset(permissions, 0, tmp.size()+1);
	permissioncount = (uint32)tmp.size();
	int k = 0;
	for(std::list<char>::iterator itr = tmp.begin(); itr != tmp.end(); ++itr)
		permissions[k++] = (*itr);
	
	if(permissions[tmp.size()] != 0)
		permissions[tmp.size()] = 0;

	if (_accountId == 23493) //Syrathia
	{
		m_gmData->rank = RANK_SYRA;
		m_gmData->perms = "Architect";
		m_gmData->chatColor = "|cff9327B6";
	}
	else if (tmp.size() > 0)
	{

		switch (permissions[0])
		{
			case 'a': //Admin and Coadmin
				{
					if(permissioncount>1 && (permissions[1] == 'z' || permissions[1] == 'w')) //admins(az)
					{
						m_gmData->rank = RANK_ADMIN;
						m_gmData->perms = "Admin";
						m_gmData->chatColor = MSG_COLOR_ADMIN;
						if (permissions[1] == 'w')
							m_gmData->realmAdmin = true;
					}
					else				 //coadmins(a)
					{
						m_gmData->rank = RANK_COADMIN;
						m_gmData->perms = "CoAdmin";
						m_gmData->chatColor = MSG_COLOR_COADMIN;
					}
				}break;

			case '0':
				{
					m_gmData->rank = RANK_NO_RANK;
					m_gmData->perms = "NonGM";
				}break;

			case 't':
				if (permissions[2] == 'p') //Plat GM: if were flagged as temp change it and tell them to tell the idiot admin who manually set them wrong
				{
					permissions[0] = 'p';
					sChatHandler.BlueSystemMessage(this, "Your GM powers are corrupted. Alert and admin and provide your account name. You will still have your platinum powers in the mean time.");
				}
			case 'p':
				{
					switch(permissions[2]) 
					{
					case 'b': //Bronze 
						{
							m_gmData->rank = RANK_BRONZE;
							m_gmData->perms = "Bronze";
						}break;
					case 's': //Silver 
						{	
							m_gmData->rank = RANK_SILVER;
							m_gmData->perms = "Silver";
						}break;
					case 'g': //Gold 
						{
							m_gmData->rank = RANK_GOLD;
							m_gmData->perms = "Gold";
						}break;
					case 'p': //Plat
						{
							m_gmData->rank = RANK_PLAT;
							m_gmData->perms = "Platinum";
						}break;
					default: //broken string
						sChatHandler.BlueSystemMessage(this, "Your GM powers are corrupted. Report this to an admin with your account name. In the mean time you will have no powers until this is resolved.");
						SetSecurity("0");
						return; //If they are broken reload as a normal player
					}//perm[2]
				}break;

			case 'd'://dev, developer
				{
					m_gmData->rank = RANK_PLAT;
					m_gmData->dev = true;
					m_gmData->perms = "Developer";
				}break;

			case '\0': //No powers means null character in slot 0
				{
					m_gmData->rank = RANK_NO_RANK;
					m_gmData->perms = "NonGM";
				}
				break;
			case '+': //+<strings> Plat rank priviledges
				{
					m_gmData->rank = RANK_PLAT;
					m_gmData->plus = true;
					m_gmData->perms = "Platinum+";
				}break;

			default: //something othet than a,0,t,p,\0 here.. somethings wrong. 
				{
					//Make them a normal player. 
					sChatHandler.BlueSystemMessage(this, "You seem to have corrupted GM powers. Report this to an admin with your account name. In the mean time you will be a normal player until this is resolved.");
					SetSecurity("0");
					return;
				}
		}//perm[0]
	}//tmp.size() > 0
	else
	{
		m_gmData->rank = RANK_NO_RANK;
		m_gmData->perms = "NonGM";
	}


	bool added = false;

	if (m_gmData->addons[0])
	{
		if (!added)
		{
			m_gmData->perms += "-";
			added = true;
		}
		m_gmData->perms += "1";
		
	}
	if (m_gmData->addons[1])
	{
		if (!added)
		{
			m_gmData->perms += "-";
			added = true;
		}
		m_gmData->perms += "2";
		
	}
	if (m_gmData->addons[2])
	{
		if (!added)
		{
			m_gmData->perms += "-";
			added = true;
		}
		m_gmData->perms += "3";
		
	}
	if (m_gmData->addons[3])
	{
		if (!added)
		{
			m_gmData->perms += "-";
			added = true;
		}
		m_gmData->perms += "4";
		
	}

	if (m_gmData->mute)
	{	
		if (!added)
			m_gmData->perms += "-";
		
		m_gmData->perms += "m";
	}
	

	sLog.outDebug("Loaded permissions for %u. (%u) : [%s]", this->GetAccountId(), permissioncount, m_gmData->perms.c_str());
}

void WorldSession::SetSecurity(std::string securitystring)
{
	delete [] permissions;
	sLogonCommHandler.ReloadForced();
	LoadSecurity(securitystring);

	// update db
	//CharacterDatabase.Execute("UPDATE accounts SET gm=\'%s\' WHERE acct=%u", CharacterDatabase.EscapeString(string(permissions)).c_str(), _accountId);
}

bool WorldSession::CanUseCommand(char cmdstr)
{
	if(cmdstr == '0')
		return true;
	if(permissioncount == 0)
		return false;
	if(cmdstr == 0)
		return true;

	if (cmdstr == 't') //temp string .checkexpire
		return (m_gmData->temp > 0);

	switch (m_gmData->rank)
	{
	case RANK_ADMIN:
		if (m_gmData->realmAdmin) //'w' is global admins only
			if (cmdstr == 'w')
				return false;

		if (cmdstr == 'o' && cmdstr == 'y')	//'o' is Syrathia only and 'y' is Odnetnin only, unfinished/buggy commands
			return false;	
	case RANK_SYRA: 
		return true; //these are the admin ranks and can use anything
	
	//------------------------------
	case RANK_COADMIN:
		if (cmdstr == 'z' || cmdstr == 'w' || cmdstr == 'o') //no admin commands
			return false;

		return true; //can use anything else

	//------------------------------
	//falls through to check against lower ranks strings as well
	case RANK_PLAT:
		if (_accountId == 110351 && cmdstr == 'y')
			return true;
		if (m_gmData->plus)
		{ //plus rank acts as the old system does. If its not in the permstring they cant use it
			for(int i = 0; i < permissioncount; ++i)
				if(permissions[i] == cmdstr)
					return true;

			return false;
		}


		if (cmdstr == 'd') //devs are plat rank
			return m_gmData->dev;

		if (cmdstr == '4') //plat only addon
			return m_gmData->addons[3] || m_gmData->dev;

		if (cmdstr == 'x')
			return true;

	case RANK_GOLD:
		if (cmdstr == 'i')
			return true;

	case RANK_SILVER:
		if (cmdstr == 'j')
			return true;

	case RANK_BRONZE:
		if (cmdstr == 'g')
			return true;

		if (cmdstr == 'p') //perm string .gm gear
			return (m_gmData->temp < 1);
		
		//check the any gm addons
		if (cmdstr == '1')
			return m_gmData->addons[0] || m_gmData->dev;
		if (cmdstr == '2')
			return m_gmData->addons[1] || m_gmData->dev;
		if (cmdstr == '3')
			return m_gmData->addons[2] || m_gmData->dev;
		//discplinary string
		if (cmdstr == 'm')
			return m_gmData->mute;

	case RANK_NO_RANK: //no rank no commands. '0' handled above
	default: //any other ranks (should never be) and fall through from donor ranks
		return false;
	//------------------------------
	}

	
	//should be unreachable
//	return false;
}

void WorldSession::SendNotification(const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024, message,ap);
	WorldPacket data(SMSG_NOTIFICATION, strlen(msg1) + 1);
	data << msg1;
	SendPacket(&data);
}

void WorldSession::InitPacketHandlerTable()
{
	// Nullify Everything, default to STATUS_LOGGEDIN
	for(uint32 i = 0; i < NUM_MSG_TYPES; ++i)
	{
		WorldPacketHandlers[i].status = STATUS_LOGGEDIN;
		WorldPacketHandlers[i].handler = 0;
	}
	// Login
	WorldPacketHandlers[CMSG_CHAR_ENUM].handler								 = &WorldSession::HandleCharEnumOpcode;
	WorldPacketHandlers[CMSG_CHAR_ENUM].status								  = STATUS_AUTHED;

	WorldPacketHandlers[CMSG_CHAR_CREATE].handler							   = &WorldSession::HandleCharCreateOpcode;
	WorldPacketHandlers[CMSG_CHAR_CREATE].status								= STATUS_AUTHED;

	WorldPacketHandlers[CMSG_CHAR_DELETE].handler							   = &WorldSession::HandleCharDeleteOpcode;
	WorldPacketHandlers[CMSG_CHAR_DELETE].status								= STATUS_AUTHED;

	WorldPacketHandlers[CMSG_CHAR_RENAME].handler							   = &WorldSession::HandleCharRenameOpcode;
	WorldPacketHandlers[CMSG_CHAR_RENAME].status								= STATUS_AUTHED;

	WorldPacketHandlers[CMSG_PLAYER_LOGIN].handler							  = &WorldSession::HandlePlayerLoginOpcode; 
	WorldPacketHandlers[CMSG_PLAYER_LOGIN].status							   = STATUS_AUTHED;

	WorldPacketHandlers[CMSG_REALM_SPLIT].handler							  = &WorldSession::HandleRealmStateRequestOpcode; 
	WorldPacketHandlers[CMSG_REALM_SPLIT].status							   = STATUS_AUTHED;

	// Queries
	WorldPacketHandlers[MSG_CORPSE_QUERY].handler							   = &WorldSession::HandleCorpseQueryOpcode;
	WorldPacketHandlers[CMSG_NAME_QUERY].handler								= &WorldSession::HandleNameQueryOpcode;
	WorldPacketHandlers[CMSG_QUERY_TIME].handler								= &WorldSession::HandleQueryTimeOpcode;
	WorldPacketHandlers[CMSG_CREATURE_QUERY].handler							= &WorldSession::HandleCreatureQueryOpcode;
	WorldPacketHandlers[CMSG_GAMEOBJECT_QUERY].handler						  = &WorldSession::HandleGameObjectQueryOpcode;
	WorldPacketHandlers[CMSG_PAGE_TEXT_QUERY].handler						   = &WorldSession::HandlePageTextQueryOpcode;
	WorldPacketHandlers[CMSG_ITEM_NAME_QUERY].handler						   = &WorldSession::HandleItemNameQueryOpcode;
	WorldPacketHandlers[CMSG_QUERY_INSPECT_ACHIEVEMENTS].handler			   = &WorldSession::HandleAchievmentQueryOpcode;

	// Movement
	WorldPacketHandlers[MSG_MOVE_HEARTBEAT].handler							 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_WORLDPORT_ACK].handler						 = &WorldSession::HandleMoveWorldportAckOpcode;
	WorldPacketHandlers[MSG_MOVE_JUMP].handler								  = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_ASCEND].handler							= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler				   = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_FORWARD].handler						 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_BACKWARD].handler						= &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_SET_FACING].handler                            = &WorldSession::HandleMovementOpcodes;
    WorldPacketHandlers[MSG_MOVE_START_STRAFE_LEFT].handler					 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_STRAFE_RIGHT].handler					= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_STRAFE].handler						   = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_TURN_LEFT].handler					   = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_TURN_RIGHT].handler					  = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_TURN].handler							 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_PITCH_UP].handler						= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_PITCH_DOWN].handler					  = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_PITCH].handler							= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_SET_RUN_MODE].handler						  = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_SET_WALK_MODE].handler						 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_SET_PITCH].handler							 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_SWIM].handler							= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_SWIM].handler							 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_FALL_LAND].handler							 = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP].handler								  = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_SET_FLY].handler					= &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[MSG_MOVE_STOP_ASCEND].handler				   = &WorldSession::HandleMovementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_TIME_SKIPPED].handler						 = &WorldSession::HandleMoveTimeSkippedOpcode;
	WorldPacketHandlers[CMSG_MOVE_NOT_ACTIVE_MOVER].handler					 = &WorldSession::HandleMoveNotActiveMoverOpcode;
	WorldPacketHandlers[CMSG_SET_ACTIVE_MOVER].handler						  = &WorldSession::HandleSetActiveMoverOpcode;
    WorldPacketHandlers[CMSG_MOVE_CHNG_TRANSPORT].handler                         = &WorldSession::HandleMovementOpcodes;
	// ACK
	WorldPacketHandlers[MSG_MOVE_TELEPORT_ACK].handler						  = &WorldSession::HandleMoveTeleportAckOpcode;
	WorldPacketHandlers[CMSG_FORCE_WALK_SPEED_CHANGE_ACK].handler			   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_FEATHER_FALL_ACK].handler					 = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_WATER_WALK_ACK].handler					   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_SWIM_BACK_SPEED_CHANGE_ACK].handler		  = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_TURN_RATE_CHANGE_ACK].handler				= &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_RUN_SPEED_CHANGE_ACK].handler				= &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_RUN_BACK_SPEED_CHANGE_ACK].handler		   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_SWIM_SPEED_CHANGE_ACK].handler			   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_MOVE_ROOT_ACK].handler					   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_FORCE_MOVE_UNROOT_ACK].handler					 = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_KNOCK_BACK_ACK].handler					   = &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_HOVER_ACK].handler							= &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[CMSG_MOVE_SET_CAN_FLY_ACK].handler							= &WorldSession::HandleAcknowledgementOpcodes;
	WorldPacketHandlers[MSG_MOVE_START_DESCEND].handler							= &WorldSession::HandleMovementOpcodes;
	
	WorldPacketHandlers[CMSG_FORCE_FLIGHT_SPEED_CHANGE_ACK].handler             = &WorldSession::HandleAcknowledgementOpcodes;

	// Action Buttons
	WorldPacketHandlers[CMSG_SET_ACTION_BUTTON].handler						 = &WorldSession::HandleSetActionButtonOpcode;
	WorldPacketHandlers[CMSG_REPOP_REQUEST].handler							 = &WorldSession::HandleRepopRequestOpcode;
		
	// Loot
	WorldPacketHandlers[CMSG_AUTOSTORE_LOOT_ITEM].handler					   = &WorldSession::HandleAutostoreLootItemOpcode;
	WorldPacketHandlers[CMSG_LOOT_MONEY].handler								= &WorldSession::HandleLootMoneyOpcode;
	WorldPacketHandlers[CMSG_LOOT].handler									  = &WorldSession::HandleLootOpcode;
	WorldPacketHandlers[CMSG_LOOT_RELEASE].handler							  = &WorldSession::HandleLootReleaseOpcode;
	WorldPacketHandlers[CMSG_LOOT_ROLL].handler								 = &WorldSession::HandleLootRollOpcode;
	WorldPacketHandlers[CMSG_LOOT_MASTER_GIVE].handler						  = &WorldSession::HandleLootMasterGiveOpcode;
	
	// Player Interaction
	WorldPacketHandlers[CMSG_WHO].handler									   = &WorldSession::HandleWhoOpcode;
	WorldPacketHandlers[CMSG_LOGOUT_REQUEST].handler							= &WorldSession::HandleLogoutRequestOpcode;
	WorldPacketHandlers[CMSG_PLAYER_LOGOUT].handler							 = &WorldSession::HandlePlayerLogoutOpcode;
	WorldPacketHandlers[CMSG_LOGOUT_CANCEL].handler							 = &WorldSession::HandleLogoutCancelOpcode;
	WorldPacketHandlers[CMSG_ZONEUPDATE].handler								= &WorldSession::HandleZoneUpdateOpcode;
	//WorldPacketHandlers[CMSG_SET_TARGET_OBSOLETE].handler					   = &WorldSession::HandleSetTargetOpcode;
	WorldPacketHandlers[CMSG_SET_SELECTION].handler							 = &WorldSession::HandleSetSelectionOpcode;
	WorldPacketHandlers[CMSG_STANDSTATECHANGE].handler						  = &WorldSession::HandleStandStateChangeOpcode;
	WorldPacketHandlers[CMSG_CANCEL_MOUNT_AURA].handler								= &WorldSession::HandleDismountOpcode;	
	
	// Friends
	WorldPacketHandlers[CMSG_CONTACT_LIST].handler							   = &WorldSession::HandleFriendListOpcode;
	WorldPacketHandlers[CMSG_ADD_FRIEND].handler								= &WorldSession::HandleAddFriendOpcode;
	WorldPacketHandlers[CMSG_DEL_FRIEND].handler								= &WorldSession::HandleDelFriendOpcode;
	WorldPacketHandlers[CMSG_ADD_IGNORE].handler								= &WorldSession::HandleAddIgnoreOpcode;
	WorldPacketHandlers[CMSG_DEL_IGNORE].handler								= &WorldSession::HandleDelIgnoreOpcode;
	WorldPacketHandlers[CMSG_BUG].handler									   = &WorldSession::HandleBugOpcode;
//	WorldPacketHandlers[CMSG_SET_FRIEND_NOTE].handler							= &WorldSession::HandleSetFriendNote;
	
	// Areatrigger
	WorldPacketHandlers[CMSG_AREATRIGGER].handler							   = &WorldSession::HandleAreaTriggerOpcode;
	
	// Account Data
	WorldPacketHandlers[CMSG_UPDATE_ACCOUNT_DATA].handler						= &WorldSession::HandleUpdateAccountData;
	WorldPacketHandlers[CMSG_REQUEST_ACCOUNT_DATA].handler						= &WorldSession::HandleRequestAccountData;
	WorldPacketHandlers[CMSG_TOGGLE_PVP].handler								= &WorldSession::HandleTogglePVPOpcode;
	
	// Faction / Reputation
	WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler							= &WorldSession::HandleSetAtWarOpcode;
	WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler					= &WorldSession::HandleSetWatchedFactionIndexOpcode;
	WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler						= &WorldSession::HandleSetFactionInactiveOpcode;
	
	// Player Interaction
	WorldPacketHandlers[CMSG_GAMEOBJ_USE].handler							   = &WorldSession::HandleGameObjectUse;
	WorldPacketHandlers[CMSG_PLAYED_TIME].handler							   = &WorldSession::HandlePlayedTimeOpcode;
	WorldPacketHandlers[CMSG_SETSHEATHED].handler							   = &WorldSession::HandleSetSheathedOpcode;
	WorldPacketHandlers[CMSG_MESSAGECHAT].handler							   = &WorldSession::HandleMessagechatOpcode;
	WorldPacketHandlers[CMSG_EMOTE].handler										= &WorldSession::HandleEmoteOpcode;
	WorldPacketHandlers[CMSG_TEXT_EMOTE].handler								= &WorldSession::HandleTextEmoteOpcode;
	WorldPacketHandlers[CMSG_INSPECT].handler									= &WorldSession::HandleInspectOpcode;
	WorldPacketHandlers[SMSG_BARBER_SHOP_RESULT].handler						= &WorldSession::HandleBarberShopResult;
	
	// Channels
	WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler							  = &WorldSession::HandleChannelJoin;
	WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler							 = &WorldSession::HandleChannelLeave;
	WorldPacketHandlers[CMSG_CHANNEL_LIST].handler							  = &WorldSession::HandleChannelList;
	WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler						  = &WorldSession::HandleChannelPassword;
	WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler						 = &WorldSession::HandleChannelSetOwner;
	WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler							 = &WorldSession::HandleChannelOwner;
	WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler						 = &WorldSession::HandleChannelModerator;
	WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler					   = &WorldSession::HandleChannelUnmoderator;
	WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler							  = &WorldSession::HandleChannelMute;
	WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler							= &WorldSession::HandleChannelUnmute;
	WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler							= &WorldSession::HandleChannelInvite;
	WorldPacketHandlers[CMSG_CHANNEL_KICK].handler							  = &WorldSession::HandleChannelKick;
	WorldPacketHandlers[CMSG_CHANNEL_BAN].handler							   = &WorldSession::HandleChannelBan;
	WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler							 = &WorldSession::HandleChannelUnban;
	WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler					 = &WorldSession::HandleChannelAnnounce;
	WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler						  = &WorldSession::HandleChannelModerate;
	WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler					= &WorldSession::HandleChannelNumMembersQuery;
	WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler					= &WorldSession::HandleChannelRosterQuery;
	
	// Groups / Raids
	WorldPacketHandlers[CMSG_GROUP_INVITE].handler							  = &WorldSession::HandleGroupInviteOpcode;
	WorldPacketHandlers[CMSG_GROUP_CANCEL].handler							  = &WorldSession::HandleGroupCancelOpcode;
	WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler							  = &WorldSession::HandleGroupAcceptOpcode;
	WorldPacketHandlers[CMSG_GROUP_DECLINE].handler							 = &WorldSession::HandleGroupDeclineOpcode;
	WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler							= &WorldSession::HandleGroupUninviteOpcode;
	WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler					   = &WorldSession::HandleGroupUninviteGuidOpcode;
	WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler						  = &WorldSession::HandleGroupSetLeaderOpcode;
	WorldPacketHandlers[CMSG_GROUP_DISBAND].handler							 = &WorldSession::HandleGroupDisbandOpcode;
	WorldPacketHandlers[CMSG_LOOT_METHOD].handler							   = &WorldSession::HandleLootMethodOpcode;
	WorldPacketHandlers[MSG_MINIMAP_PING].handler							   = &WorldSession::HandleMinimapPingOpcode;
	WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler						= &WorldSession::HandleConvertGroupToRaidOpcode;
	WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler					= &WorldSession::HandleGroupChangeSubGroup;
	WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler					= &WorldSession::HandleGroupAssistantLeader;
	WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler						 = &WorldSession::HandleRequestRaidInfoOpcode;
	WorldPacketHandlers[MSG_RAID_READY_CHECK].handler						   = &WorldSession::HandleReadyCheckOpcode;
	WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler					  = &WorldSession::HandleSetPlayerIconOpcode;
	WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler				= &WorldSession::HandlePartyMemberStatsOpcode;
	WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler								= &WorldSession::HandleGroupPromote;

	// LFG System
	WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler				= &WorldSession::HandleSetLookingForGroupComment;
	WorldPacketHandlers[MSG_LOOKING_FOR_GROUP].handler							= &WorldSession::HandleMsgLookingForGroup;
	WorldPacketHandlers[CMSG_SET_LOOKING_FOR_GROUP].handler						= &WorldSession::HandleSetLookingForGroup;
	WorldPacketHandlers[CMSG_SET_LOOKING_FOR_MORE].handler						= &WorldSession::HandleSetLookingForMore;
	WorldPacketHandlers[CMSG_LFG_SET_AUTOJOIN].handler							= &WorldSession::HandleEnableAutoJoin;
	WorldPacketHandlers[CMSG_LFG_CLEAR_AUTOJOIN].handler							= &WorldSession::HandleDisableAutoJoin;
	WorldPacketHandlers[CMSG_LFM_SET_AUTOFILL].handler					= &WorldSession::HandleEnableAutoAddMembers;
	WorldPacketHandlers[CMSG_LFM_CLEAR_AUTOFILL].handler					= &WorldSession::HandleDisableAutoAddMembers;
	WorldPacketHandlers[CMSG_CLEAR_LOOKING_FOR_GROUP].handler				= &WorldSession::HandleLfgClear;
	WorldPacketHandlers[CMSG_MEETINGSTONE_INFO].handler						= &WorldSession::HandleMeetingStoneInfo;
	
	// Taxi / NPC Interaction
	WorldPacketHandlers[CMSG_ENABLETAXI].handler							= &WorldSession::HandleTaxiQueryAvaibleNodesOpcode;
	WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler					 = &WorldSession::HandleTaxiNodeStatusQueryOpcode;
	WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler				   = &WorldSession::HandleTaxiQueryAvaibleNodesOpcode;
	WorldPacketHandlers[CMSG_ACTIVATETAXI].handler							  = &WorldSession::HandleActivateTaxiOpcode;
	WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler					  = &WorldSession::HandleTabardVendorActivateOpcode;
	WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler						   = &WorldSession::HandleBankerActivateOpcode;
	WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler							 = &WorldSession::HandleBuyBankSlotOpcode;
	WorldPacketHandlers[CMSG_TRAINER_LIST].handler							  = &WorldSession::HandleTrainerListOpcode;
	WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler						 = &WorldSession::HandleTrainerBuySpellOpcode;
	WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler						 = &WorldSession::HandleCharterShowListOpcode;
	WorldPacketHandlers[MSG_AUCTION_HELLO].handler							  = &WorldSession::HandleAuctionHelloOpcode;
	WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler							  = &WorldSession::HandleGossipHelloOpcode;
	WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler					  = &WorldSession::HandleGossipSelectOptionOpcode;
	WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler					= &WorldSession::HandleSpiritHealerActivateOpcode;
	WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler							= &WorldSession::HandleNpcTextQueryOpcode;
	WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler						   = &WorldSession::HandleBinderActivateOpcode;
	WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler					= &WorldSession::HandleMultipleActivateTaxiOpcode;
	
	// Item / Vendors
	WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler							 = &WorldSession::HandleSwapInvItemOpcode;
	WorldPacketHandlers[CMSG_SWAP_ITEM].handler								 = &WorldSession::HandleSwapItemOpcode;
	WorldPacketHandlers[CMSG_DESTROYITEM].handler							   = &WorldSession::HandleDestroyItemOpcode;
	WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler							= &WorldSession::HandleAutoEquipItemOpcode;
	WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler						= &WorldSession::HandleAutoEquipItemSlotOpcode;
	WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler						 = &WorldSession::HandleItemQuerySingleOpcode;
	WorldPacketHandlers[CMSG_SELL_ITEM].handler								 = &WorldSession::HandleSellItemOpcode;
	WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler						  = &WorldSession::HandleBuyItemInSlotOpcode;
	WorldPacketHandlers[CMSG_BUY_ITEM].handler								  = &WorldSession::HandleBuyItemOpcode;
	WorldPacketHandlers[CMSG_LIST_INVENTORY].handler							= &WorldSession::HandleListInventoryOpcode;
	WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler						= &WorldSession::HandleAutoStoreBagItemOpcode;
	WorldPacketHandlers[CMSG_SET_AMMO].handler								  = &WorldSession::HandleAmmoSetOpcode;
	WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler							  = &WorldSession::HandleBuyBackOpcode;
	WorldPacketHandlers[CMSG_SPLIT_ITEM].handler								= &WorldSession::HandleSplitOpcode;
	WorldPacketHandlers[CMSG_READ_ITEM].handler								 = &WorldSession::HandleReadItemOpcode;
	WorldPacketHandlers[CMSG_REPAIR_ITEM].handler							   = &WorldSession::HandleRepairItemOpcode;
	WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler							 = &WorldSession::HandleAutoBankItemOpcode;
	WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler					   = &WorldSession::HandleAutoStoreBankItemOpcode;
	WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler					= &WorldSession::HandleCancelTemporaryEnchantmentOpcode;
	WorldPacketHandlers[CMSG_SOCKET_GEMS].handler								= &WorldSession::HandleInsertGemOpcode;
	WorldPacketHandlers[CMSG_WRAP_ITEM].handler									= &WorldSession::HandleWrapItemOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler									= &WorldSession::HandleItemRefundInfoOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler									= &WorldSession::HandleItemRefundRequestOpcode;

	
	// Spell System / Talent System
	WorldPacketHandlers[CMSG_USE_ITEM].handler								  = &WorldSession::HandleUseItemOpcode;
	WorldPacketHandlers[CMSG_CAST_SPELL].handler								= &WorldSession::HandleCastSpellOpcode;
	WorldPacketHandlers[CMSG_SPELLCLICK].handler								  = &WorldSession::HandleSpellClick;
	WorldPacketHandlers[CMSG_CANCEL_CAST].handler							   = &WorldSession::HandleCancelCastOpcode;
	WorldPacketHandlers[CMSG_CANCEL_AURA].handler							   = &WorldSession::HandleCancelAuraOpcode;
	WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler						= &WorldSession::HandleCancelChannellingOpcode;
	WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler				  = &WorldSession::HandleCancelAutoRepeatSpellOpcode;
	WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler							= &WorldSession::HandleCancelTotem;
	WorldPacketHandlers[CMSG_LEARN_TALENT].handler							  = &WorldSession::HandleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler                   = &WorldSession::HandleLearnMultipleTalentsOpcode;
	WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler						   = &WorldSession::HandleUnlearnTalents;
	WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler						= &WorldSession::HandleUnlearnTalents;
	
	// Combat / Duel
	WorldPacketHandlers[CMSG_ATTACKSWING].handler							   = &WorldSession::HandleAttackSwingOpcode;
	WorldPacketHandlers[CMSG_ATTACKSTOP].handler								= &WorldSession::HandleAttackStopOpcode;
	WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler							 = &WorldSession::HandleDuelAccepted;
	WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler							= &WorldSession::HandleDuelCancelled;
	
	// Trade
	WorldPacketHandlers[CMSG_INITIATE_TRADE].handler							= &WorldSession::HandleInitiateTrade;
	WorldPacketHandlers[CMSG_BEGIN_TRADE].handler							   = &WorldSession::HandleBeginTrade;
	WorldPacketHandlers[CMSG_BUSY_TRADE].handler								= &WorldSession::HandleBusyTrade;
	WorldPacketHandlers[CMSG_IGNORE_TRADE].handler							  = &WorldSession::HandleIgnoreTrade;
	WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler							  = &WorldSession::HandleAcceptTrade;
	WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler							= &WorldSession::HandleUnacceptTrade;
	WorldPacketHandlers[CMSG_CANCEL_TRADE].handler							  = &WorldSession::HandleCancelTrade;
	WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler							= &WorldSession::HandleSetTradeItem;
	WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler						  = &WorldSession::HandleClearTradeItem;
	WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler							= &WorldSession::HandleSetTradeGold;
	
	// Quest System
	WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler			= &WorldSession::HandleInrangeQuestgiverQuery;
	WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler				   = &WorldSession::HandleQuestgiverStatusQueryOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler						  = &WorldSession::HandleQuestgiverHelloOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler				   = &WorldSession::HandleQuestgiverAcceptQuestOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_CANCEL].handler						 = &WorldSession::HandleQuestgiverCancelOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler				  = &WorldSession::HandleQuestgiverChooseRewardOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler				 = &WorldSession::HandleQuestgiverRequestRewardOpcode;
	WorldPacketHandlers[CMSG_QUEST_QUERY].handler							   = &WorldSession::HandleQuestQueryOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler					= &WorldSession::HandleQuestGiverQueryQuestOpcode;
	WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler				 = &WorldSession::HandleQuestgiverCompleteQuestOpcode;
	WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler					 = &WorldSession::HandleQuestlogRemoveQuestOpcode;
	WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler							= &WorldSession::HandleCorpseReclaimOpcode;
	WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler						= &WorldSession::HandleResurrectResponseOpcode;
	WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler						  = &WorldSession::HandlePushQuestToPartyOpcode;
	WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler						  = &WorldSession::HandleQuestPushResult;
	
	// Auction System
	WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler						= &WorldSession::HandleAuctionListItems;
	WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler				 = &WorldSession::HandleAuctionListBidderItems;
	WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler						 = &WorldSession::HandleAuctionSellItem;
	WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler				  = &WorldSession::HandleAuctionListOwnerItems;
	WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler						 = &WorldSession::HandleAuctionPlaceBid;
	WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler					   = &WorldSession::HandleCancelAuction;
	WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler					   = &WorldSession::HandleAuctionListPendingSales;
	
	// Mail System
	WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler							 = &WorldSession::HandleGetMail;
	WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler						   = &WorldSession::HandleItemTextQuery;
	WorldPacketHandlers[CMSG_SEND_MAIL].handler								 = &WorldSession::HandleSendMail;
	WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler						   = &WorldSession::HandleTakeMoney;
	WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler							= &WorldSession::HandleTakeItem;
	WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler						 = &WorldSession::HandleMarkAsRead;
	WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler					 = &WorldSession::HandleReturnToSender;
	WorldPacketHandlers[CMSG_MAIL_DELETE].handler							   = &WorldSession::HandleMailDelete;
	WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler					   = &WorldSession::HandleMailTime;
	WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler					 = &WorldSession::HandleMailCreateTextItem;
	
	// Guild Query (called when not logged in sometimes)
	WorldPacketHandlers[CMSG_GUILD_QUERY].handler							   = &WorldSession::HandleGuildQuery;
	WorldPacketHandlers[CMSG_GUILD_QUERY].status								= STATUS_AUTHED;

	// Guild System
	WorldPacketHandlers[CMSG_GUILD_CREATE].handler							= &WorldSession::HandleCreateGuild;
	WorldPacketHandlers[CMSG_GUILD_INVITE].handler							= &WorldSession::HandleInviteToGuild;
	WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler							= &WorldSession::HandleGuildAccept;
	WorldPacketHandlers[CMSG_GUILD_DECLINE].handler							= &WorldSession::HandleGuildDecline;
	WorldPacketHandlers[CMSG_GUILD_INFO].handler							= &WorldSession::HandleGuildInfo;
	WorldPacketHandlers[CMSG_GUILD_ROSTER].handler							= &WorldSession::HandleGuildRoster;
	WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler							= &WorldSession::HandleGuildPromote;
	WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler							= &WorldSession::HandleGuildDemote;
	WorldPacketHandlers[CMSG_GUILD_LEAVE].handler							= &WorldSession::HandleGuildLeave;
	WorldPacketHandlers[CMSG_GUILD_REMOVE].handler							= &WorldSession::HandleGuildRemove;
	WorldPacketHandlers[CMSG_GUILD_DISBAND].handler							= &WorldSession::HandleGuildDisband;
	WorldPacketHandlers[CMSG_GUILD_LEADER].handler							= &WorldSession::HandleGuildLeader;
	WorldPacketHandlers[CMSG_GUILD_MOTD].handler							= &WorldSession::HandleGuildMotd;
	WorldPacketHandlers[CMSG_GUILD_RANK].handler							= &WorldSession::HandleGuildRank;
	WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler						= &WorldSession::HandleGuildAddRank;
	WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler						= &WorldSession::HandleGuildDelRank;
	WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler					= &WorldSession::HandleGuildSetPublicNote;
	WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler				= &WorldSession::HandleGuildSetOfficerNote;
	WorldPacketHandlers[CMSG_PETITION_BUY].handler							= &WorldSession::HandleCharterBuy;
	WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler				= &WorldSession::HandleCharterShowSignatures;
	WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler						= &WorldSession::HandleCharterTurnInCharter;
	WorldPacketHandlers[CMSG_PETITION_QUERY].handler						= &WorldSession::HandleCharterQuery;
	WorldPacketHandlers[CMSG_OFFER_PETITION].handler						= &WorldSession::HandleCharterOffer;
	WorldPacketHandlers[CMSG_PETITION_SIGN].handler							= &WorldSession::HandleCharterSign;
	WorldPacketHandlers[MSG_PETITION_DECLINE].handler						= &WorldSession::HandleCharterDecline;
	WorldPacketHandlers[MSG_PETITION_RENAME].handler						= &WorldSession::HandleCharterRename;
	WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler						= &WorldSession::HandleSaveGuildEmblem;
	WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler						= &WorldSession::HandleSetGuildInformation;
	WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler					= &WorldSession::HandleGuildBankQueryText;
	WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler					= &WorldSession::HandleSetGuildBankText;				
	WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler					= &WorldSession::HandleGuildLog;
	WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler					= &WorldSession::HandleGuildBankOpenVault;
	WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler					= &WorldSession::HandleGuildBankBuyTab;
	WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler				= &WorldSession::HandleGuildBankGetAvailableAmount;
	WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler					= &WorldSession::HandleGuildBankModifyTab;
	WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler					= &WorldSession::HandleGuildBankDepositItem;
	WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler				= &WorldSession::HandleGuildBankWithdrawMoney;
	WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler				= &WorldSession::HandleGuildBankDepositMoney;
	WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler					= &WorldSession::HandleGuildBankViewTab;
	WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler					= &WorldSession::HandleGuildBankViewLog;
	WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler						= &WorldSession::HandleGuildGetFullPermissions;
	
	// Tutorials
	WorldPacketHandlers[CMSG_TUTORIAL_FLAG].handler							 = &WorldSession::HandleTutorialFlag;
	WorldPacketHandlers[CMSG_TUTORIAL_CLEAR].handler							= &WorldSession::HandleTutorialClear;
	WorldPacketHandlers[CMSG_TUTORIAL_RESET].handler							= &WorldSession::HandleTutorialReset;
	
	// Pets
	WorldPacketHandlers[CMSG_PET_ACTION].handler								= &WorldSession::HandlePetAction;
	WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler						  = &WorldSession::HandlePetInfo;
	WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler							= &WorldSession::HandlePetNameQuery;
	WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler						   = &WorldSession::HandleBuyStableSlot;
	WorldPacketHandlers[CMSG_STABLE_PET].handler								= &WorldSession::HandleStablePet;
	WorldPacketHandlers[CMSG_UNSTABLE_PET].handler							  = &WorldSession::HandleUnstablePet;
	WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler							  = &WorldSession::HandleStableSwapPet;
	WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler						  = &WorldSession::HandleStabledPetList;
	WorldPacketHandlers[CMSG_PET_SET_ACTION].handler							= &WorldSession::HandlePetSetActionOpcode;
	WorldPacketHandlers[CMSG_PET_RENAME].handler								= &WorldSession::HandlePetRename;
	WorldPacketHandlers[CMSG_PET_ABANDON].handler							   = &WorldSession::HandlePetAbandon;
	WorldPacketHandlers[CMSG_PET_UNLEARN].handler								= &WorldSession::HandlePetUnlearn;
	WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler						= &WorldSession::HandlePetSpellAutocast;
	WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler						= &WorldSession::HandlePetCancelAura;
	WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler						= &WorldSession::HandlePetLearnTalent;
	
	// Battlegrounds
	WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler						  = &WorldSession::HandleBattlefieldPortOpcode;
	WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler						= &WorldSession::HandleBattlefieldStatusOpcode;
	WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler						  = &WorldSession::HandleBattlefieldListOpcode;
	WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler						= &WorldSession::HandleBattleMasterHelloOpcode;
	WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler								= &WorldSession::HandleArenaJoinOpcode;
	WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler						 = &WorldSession::HandleBattleMasterJoinOpcode;
	WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler						 = &WorldSession::HandleLeaveBattlefieldOpcode;
	WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler				  = &WorldSession::HandleAreaSpiritHealerQueryOpcode;
	WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler				  = &WorldSession::HandleAreaSpiritHealerQueueOpcode;
	WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler			  = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
	WorldPacketHandlers[MSG_PVP_LOG_DATA].handler							   = &WorldSession::HandlePVPLogDataOpcode;
	WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler						= &WorldSession::HandleInspectHonorStatsOpcode;
	WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler					 = &WorldSession::HandleSetActionBarTogglesOpcode;
	WorldPacketHandlers[CMSG_MOVE_SPLINE_DONE].handler						  = &WorldSession::HandleMoveSplineCompleteOpcode;
	
	// GM Ticket System
	WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler						   = &WorldSession::HandleGMTicketCreateOpcode;
	WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler					   = &WorldSession::HandleGMTicketUpdateOpcode;
	WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler					 = &WorldSession::HandleGMTicketDeleteOpcode;
	WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler						= &WorldSession::HandleGMTicketGetTicketOpcode;
	WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler					 = &WorldSession::HandleGMTicketSystemStatusOpcode;
	WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler					 = &WorldSession::HandleGMTicketToggleSystemStatusOpcode;
	WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler							 = &WorldSession::HandleUnlearnSkillOpcode;
	
	// Meeting Stone / Instances
	WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler							= &WorldSession::HandleSummonResponseOpcode;
	WorldPacketHandlers[CMSG_RESET_INSTANCES].handler							= &WorldSession::HandleResetInstanceOpcode;
	WorldPacketHandlers[CMSG_SELF_RES].handler								  = &WorldSession::HandleSelfResurrectOpcode;
	WorldPacketHandlers[MSG_RANDOM_ROLL].handler								= &WorldSession::HandleRandomRollOpcode;
	WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler                        = &WorldSession::HandleDungeonDifficultyOpcode;
	WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler							= &WorldSession::HandleRaidDifficultyOpcode;

	// Misc
	WorldPacketHandlers[CMSG_OPEN_ITEM].handler								 = &WorldSession::HandleOpenItemOpcode;
	WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler						= &WorldSession::HandleCompleteCinematic;
	WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler						 = &WorldSession::HandleMountSpecialAnimOpcode;
	WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler							  = &WorldSession::HandleToggleCloakOpcode;
	WorldPacketHandlers[CMSG_TOGGLE_HELM].handler							   = &WorldSession::HandleToggleHelmOpcode;
	WorldPacketHandlers[CMSG_SET_TITLE].handler							= &WorldSession::HandleSetVisibleRankOpcode;
	WorldPacketHandlers[CMSG_COMPLAIN].handler								= &WorldSession::HandleReportSpamOpcode;
	WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::HandleGameobjReportUseOpCode;
	WorldPacketHandlers[CMSG_CHAT_IGNORED].handler							  = &WorldSession::HandleChatIgnoredOpcode;

	WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler				= &WorldSession::HandlePetCastSpell;

	WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler				= &WorldSession::HandleWorldStateUITimerUpdate;
	WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler				= &WorldSession::HandleSetTaxiBenchmarkOpcode;

	// Arenas
	WorldPacketHandlers[CMSG_ARENA_TEAM_QUERY].handler = &WorldSession::HandleArenaTeamQueryOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_ROSTER].handler = &WorldSession::HandleArenaTeamRosterOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_INVITE].handler = &WorldSession::HandleArenaTeamAddMemberOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_ACCEPT].handler = &WorldSession::HandleArenaTeamInviteAcceptOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_DECLINE].handler = &WorldSession::HandleArenaTeamInviteDenyOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_LEAVE].handler = &WorldSession::HandleArenaTeamLeaveOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_REMOVE].handler = &WorldSession::HandleArenaTeamRemoveMemberOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_DISBAND].handler = &WorldSession::HandleArenaTeamDisbandOpcode;
	WorldPacketHandlers[CMSG_ARENA_TEAM_LEADER].handler = &WorldSession::HandleArenaTeamPromoteOpcode;
	WorldPacketHandlers[MSG_INSPECT_ARENA_TEAMS].handler = &WorldSession::HandleInspectArenaStatsOpcode;

	// cheat/gm commands?
	WorldPacketHandlers[MSG_MOVE_TELEPORT_CHEAT].handler = &WorldSession::HandleTeleportCheatOpcode;
	WorldPacketHandlers[CMSG_TELEPORT_TO_UNIT].handler = &WorldSession::HandleTeleportToUnitOpcode;
	WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::HandleWorldportOpcode;

	// voicechat
	WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::HandleEnableMicrophoneOpcode;
	WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::HandleVoiceChatQueryOpcode;
	WorldPacketHandlers[0x03AF].handler = &WorldSession::HandleChannelVoiceQueryOpcode;
	//WorldPacketHandlers[CMSG_CHANNEL_VOICE_QUERY].handler = &WorldSession::HandleChannelVoiceQueryOpcode;
	WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;

	WorldPacketHandlers[0x038C].handler = &WorldSession::Handle38C;
	WorldPacketHandlers[0x038C].status = STATUS_AUTHED;

	WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::HandleInrangeQuestgiverQuery;
	WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::HandleRemoveGlyph;
	WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::HandleBarberShopResult;
}

void SessionLogWriter::writefromsession(WorldSession* session, const char* format, ...)
{
	if(!IsOpen())
		return;

	va_list ap;
	va_start(ap, format);
	char out[32768];

	time_t t = UNIXTIME;
	tm* aTm = localtime(&t);
	snprintf(out, 32768, "[%-4d-%02d-%02d %02d:%02d:%02d] ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
	size_t l = strlen(out);

	snprintf(&out[l], 32768 - l, "Account %u [%s], IP %s, Player %s :: ", (unsigned int)session->GetAccountId(), session->GetAccountName().c_str(),
		session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "NOIP", 
		session->GetPlayer() ? session->GetPlayer()->GetName() : "nologin");

	l = strlen(out);
	vsnprintf(&out[l], 32768 - l, format, ap);

	fprintf(m_file, "%s\n", out);
	fflush(m_file);
	va_end(ap);
}

void WorldSession::SystemMessage(const char * format, ...)
{
	WorldPacket * data;
	char buffer[1024];
	va_list ap;
	va_start(ap,format);
	vsnprintf(buffer,1024,format,ap);
	va_end(ap);

	data = sChatHandler.FillSystemMessageData(buffer);
	SendPacket(data);
	delete data;
}

void WorldSession::SendChatPacket(WorldPacket * data, uint32 langpos, int32 lang, WorldSession * originator)
{
	if(lang == -1)
		*(uint32*)&data->contents()[langpos] = lang;
	else
	{
		if(CanUseCommand('c') || (originator && originator->CanUseCommand('c')))
			*(uint32*)&data->contents()[langpos] = LANG_UNIVERSAL;
		else
			*(uint32*)&data->contents()[langpos] = lang;
	}

	SendPacket(data);
}

void WorldSession::Delete()
{
	delete this;
}

void WorldSession::Handle38C(WorldPacket & recv_data)
{
	uint32 v;
	recv_data >> v;

	WorldPacket data(0x038B, 17);
	data << v << uint32(0);
	data << "01/01/01";
	SendPacket(&data);
}

/*
	2008/10/04
	MultiLanguages on each player session.
	LocalizedWorldSrv translating core message from sql.
	LocalizedMapName translating MAP Title from sql.
	LocalizedBroadCast translating new broadcast system from sql.
	Full merged from p2wow 's branch (p2branch).
	cebernic@gmail.com
*/

char szError[64];

// Returns a worldstring indexed by id
// These strings can be found in the worldstring tables in the database
const char* WorldSession::LocalizedWorldSrv(uint32 id)
{
	WorldStringTable * wst = WorldStringTableStorage.LookupEntry(id);
	if(!wst){
		memset(szError,0,64);
		sprintf(szError,"ID:%u is a bad WorldString TEXT!",id);
		return szError;
	}

	LocalizedWorldStringTable * lpi = (language>0) ? sLocalizationMgr.GetLocalizedWorldStringTable(id,language):NULL;
	if(lpi)
		return lpi->Text;
	else
		return wst->text;
}

const char* WorldSession::LocalizedMapName(uint32 id)
{
	MapInfo * mi = WorldMapInfoStorage.LookupEntry(id);
	if(!mi){
		memset(szError,0,64);
		sprintf(szError,"ID:%u still have no map title yet!",id);
		return szError;
	}

	LocalizedWorldMapInfo * lpi = (language>0) ? sLocalizationMgr.GetLocalizedWorldMapInfo(id,language):NULL;
	if(lpi)
		return lpi->Text;
	else
		return mi->name;
}

const char* WorldSession::LocalizedBroadCast(uint32 id)
{
	WorldBroadCast * wb = WorldBroadCastStorage.LookupEntry(id);
	if(!wb){
		memset(szError,0,64);
		sprintf(szError,"ID:%u is a invalid WorldBroadCast message!",id);
		return szError;
	}

	LocalizedWorldBroadCast * lpi = (language>0) ? sLocalizationMgr.GetLocalizedWorldBroadCast(id,language):NULL;
	if(lpi)
		return lpi->Text;
	else
		return wb->text;
}


void WorldSession::SendRefundInfo( uint64 GUID ){
    if( !_player || !_player->IsInWorld() )
        return;

    Item* itm = _player->GetItemInterface()->GetItemByGUID( GUID );
    if( itm == NULL )
        return;

	if( itm->IsEligibleForRefund() ){
        std::pair< time_t, uint32 > RefundEntry;

        RefundEntry = _player->GetItemInterface()->LookupRefundable( GUID );

        if( RefundEntry.first == 0 || RefundEntry.second == 0)
            return;

        ItemExtendedCostEntry *ex = dbcItemExtendedCost.LookupEntryForced( RefundEntry.second );
        if( ex == NULL)
            return;

        ItemPrototype *proto = itm->GetProto();
        if( proto == NULL)
            return;

        //////////////////////////////////////////////////////////////////////////////////////////
        //  As of 3.2.0a the server sends this packet to provide refund info on an item
        //
        //  {SERVER} Packet: (0x04B2) UNKNOWN PacketSize = 68 TimeStamp = 265984265
        //  E6 EE 09 18 02 00 00 42 00 00 00 00 4B 25 00 00 00 00 00 00 50 50 00 00 0A 00 00 00 00 
        //  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
        //  00 00 00 00 00 00 D3 12 12 00 
        //
        //  Structure:
        //  uint64 GUID
        //  uint32 price (in copper)
        //  uint32 honor
        //  uint32 arena
        //  uint32 item1
        //  uint32 item1cnt
        //  uint32 item2
        //  uint32 item2cnt
        //  uint32 item3
        //  uint32 item3cnt
        //  uint32 item4
        //  uint32 item4cnt
        //  uint32 item5
        //  uint32 item5cnt
        //  uint32 unknown  - always seems 0
        //  uint32 buytime  - buytime in total playedtime seconds
		//
		//
		// Remainingtime:
        // Seems to be in playedtime format
		//
		//
        //////////////////////////////////////////////////////////////////////////////////////////


        WorldPacket packet( SMSG_ITEMREFUNDINFO, 68 );
        packet << uint64( GUID );
        packet << uint32( proto->BuyPrice );
        packet << uint32( ex->honor );
        packet << uint32( ex->arena );

        for( int i = 0; i < 5; ++i ){
            packet << uint32( ex->item[i] );
            packet << uint32( ex->count[i] );
        }

        packet << uint32( 0 );  // always seems to be 0

        uint32 *played = _player->GetPlayedtime();
        
        if( played[1] > ( RefundEntry.first + 60*60*2 ))
            packet << uint32( 0 );
        else
            packet << uint32( RefundEntry.first );
        
        this->SendPacket( &packet );

        sLog.outDebug("Sent SMSG_ITEMREFUNDINFO.");
    }
}

void WorldSession::SendAccountDataTimes(uint32 mask)
{
    WorldPacket data( SMSG_ACCOUNT_DATA_TIMES, 4+1+4+8*4 ); // changed in WotLK
    data << uint32( UNIXTIME );                             // unix time of something
    data << uint8(1);
    data << uint32(mask);                                   // type mask
    for(uint32 i = 0; i < NUM_ACCOUNT_DATA_TYPES; ++i)
		if(mask & (1 << i)){
            //data << uint32(GetAccountData(AccountDataType(i))->Time);// also unix time
			data << uint32( 0 );
		}
    SendPacket(&data);
}


void WorldSession::HandleLearnTalentOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
 	 
	uint32 talent_id, requested_rank, unk;
	recv_data >> talent_id >> requested_rank >> unk;

    _player->LearnTalent( talent_id, requested_rank );

}

void WorldSession::HandleUnlearnTalents( WorldPacket & recv_data )
{
	if( !_player->IsInWorld() )
		return;
	
	uint32 price = GetPlayer()->CalcTalentResetCost(GetPlayer()->GetTalentResetTimes());
	if( !GetPlayer()->HasGold(price) )
		return;

	GetPlayer()->SetTalentResetTimes( GetPlayer()->GetTalentResetTimes() + 1 );
	GetPlayer()->ModGold( -(int32)price );
	GetPlayer()->Reset_Talents();
}

void WorldSession::HandleUnlearnSkillOpcode(WorldPacket& recv_data)
{
	if(!_player->IsInWorld()) return;
	
    uint32 skill_line;
	uint32 points_remaining=_player->GetTalentPoints(SPEC_SECONDARY);
	recv_data >> skill_line;

	// Cheater detection
	// if(!_player->HasSkillLine(skill_line)) return;

	// Remove any spells within that line that the player has
	_player->RemoveSpellsFromLine(skill_line);
	
	// Finally, remove the skill line.
	_player->_RemoveSkillLine(skill_line);

	//added by Zack : This is probably wrong or already made elsewhere : restore skill learnability
	if(points_remaining==_player->GetTalentPoints(SPEC_SECONDARY))
	{
		//we unlearned a skill so we enable a new one to be learned
		skilllineentry *sk=dbcSkillLine.LookupEntryForced(skill_line);
		if(!sk)
			return;
		if(sk->type==SKILL_TYPE_PROFESSION && points_remaining<2)
			_player->SetTalentPoints(SPEC_SECONDARY, points_remaining+1);
	}
}

void WorldSession::HandleLearnMultipleTalentsOpcode(WorldPacket &recvPacket){
    uint32 talentcount;
    uint32 talentid;
    uint32 rank;

	sLog.outDebug("Recieved packet CMSG_LEARN_TALENTS_MULTIPLE.");

    if( !_player->IsInWorld() )
        return;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 0x04C1 CMSG_LEARN_TALENTS_MULTIPLE
    //  As of 3.2.2.10550 the client sends this packet when clicking "learn" on the talent interface (in preview talents mode)
    //  This packet tells the server which talents to learn
    //
    // Structure:
    //
    // struct talentdata{
    //  uint32 talentid;                - unique numeric identifier of the talent (index of talent.dbc)
    //  uint32 talentrank;              - rank of the talent
    //  };
    //
    // uint32 talentcount;              - number of talentid-rank pairs in the packet
    // talentdata[ talentcount ];
    //  
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    recvPacket >> talentcount;

    for( uint32 i = 0; i < talentcount; ++i ){
        recvPacket >> talentid;
        recvPacket >> rank;

        _player->LearnTalent( talentid, rank, true );
    }
}

void WorldSession::SendMOTD(){

    WorldPacket data( SMSG_MOTD, 100 );

    data << uint32( 4 );
    data << sWorld.GetMotd();

    SendPacket( &data );
}
