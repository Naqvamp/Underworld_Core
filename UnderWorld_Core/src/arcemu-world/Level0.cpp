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

/////////////////////////////////////////////////
//  Normal User Chat Commands
//

#include "StdAfx.h"
#include <svn_revision.h>

bool ChatHandler::ShowHelpForCommand(WorldSession *m_session, ChatCommand *table, const char* cmd)
{
	for(uint32 i = 0; table[i].Name != NULL; i++)
	{
		if(!hasStringAbbr(table[i].Name, cmd))
			continue;

		if(m_session->CanUseCommand(table[i].CommandGroup))
			continue;

		if(table[i].ChildCommands != NULL)
		{
			cmd = strtok(NULL, " ");
			if(cmd && ShowHelpForCommand(m_session, table[i].ChildCommands, cmd))
				return true;
		}

		if(table[i].Help == "")
		{
			SystemMessage(m_session, "There is no help for that command");
			return true;
		}

		SendMultilineMessage(m_session, table[i].Help.c_str());

		return true;
	}

	return false;
}

bool ChatHandler::HandleHelpCommand(const char* args, WorldSession *m_session)
{
//	ChatCommand *table = getCommandTable();
	WorldPacket data;

	if(!*args)
		return false;

	char* cmd = strtok((char*)args, " ");
	if(!cmd)
		return false;

	if(!ShowHelpForCommand(m_session, CommandTableStorage::getSingleton().Get(), cmd))
	{
		RedSystemMessage(m_session, "Sorry, no help was found for this command, or that command does not exist.");
	}

	return true;
}


bool ChatHandler::HandleCommandsCommand(const char* args, WorldSession *m_session)
{
	ChatCommand *table = CommandTableStorage::getSingleton().Get();
	WorldPacket data;

	std::string output;
	uint32 count = 0;

	output = "Available commands: \n\n";

	for(uint32 i = 0; table[i].Name != NULL; i++)
	{
		if(*args && !hasStringAbbr(table[i].Name, (char*)args))
			continue;

		if(table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
			continue;

		switch(table[i].CommandGroup)
		{
		case 'z':
			{
				output+="|cffff6060";
				output+=table[i].Name;
				output+="|r, ";
			}
			break;
		case 'm':
			{
				output+="|cff00ffff";
				output+=table[i].Name;
				output+=", ";
			}
			break;
		case 'c':
			{
				output += "|cff00ff00";
				output += table[i].Name;
				output += "|r, ";
			}break;
		default:
			{
				output+="|cff00ccff";
				output+=table[i].Name;
				output+="|r, ";
			}
			break;
		}

		count++;
		if(count == 5)  // 5 per line
		{
			output += "\n";
			count = 0;
		}
	}
	if(count)
		output += "\n";


		//FillSystemMessageData(&data, table[i].Name);
		//m_session->SendPacket(&data);
	//}

	SendMultilineMessage(m_session, output.c_str());

	return true;
}

bool ChatHandler::HandleStartCommand(const char* args, WorldSession *m_session)
{
	std::string race;
	uint32 raceid = 0;

	GET_PLAYER(RANK_COADMIN);

	PVP_CHECK(RANK_ADMIN);

	if (plr && args && strlen(args) < 2)
	{
		raceid = plr->getRace();
		switch (raceid)
		{
		case 1:
			race = "human";
		break;
		case 2:
			race = "orc";
		break;
		case 3:
			race = "dwarf";
		break;
		case 4:
			race = "nightelf";
		break;
		case 5:
			race = "undead";
		break;
		case 6:
			race = "tauren";
		break;
		case 7:
			race = "gnome";
		break;
		case 8:
			race = "troll";
		break;
		case 10:
			race = "bloodelf";
		break;
		case 11:
			race = "draenei";
		break;
		default:
			return false;
		break;
		}
	}
	else if (plr && args && strlen(args) > 2)
	{
		race = args;
		arcemu_TOLOWER(race);

		// Teleport to specific race
		if(race == "human")
			raceid = 1;
		else if(race == "orc")
			raceid = 2;
		else if(race == "dwarf")
			raceid = 3;
		else if(race == "nightelf")
			raceid = 4;
		else if(race == "undead")
			raceid = 5;
		else if(race == "tauren")
			raceid = 6;
		else if(race == "gnome")
			raceid = 7;
		else if(race == "troll")
			raceid = 8;
		else if(race == "bloodelf")
			raceid = 10;
		else if(race == "draenei")
			raceid = 11;
		else
		{
			RedSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, tauren, undead, bloodelf");
			return true;
		}
	}
	else
	{
		return false;
	}

	// Try to find a class that works
	PlayerCreateInfo *info = NULL;
	for(uint32 i=1;i<11;i++)
	{
		 info = objmgr.GetPlayerCreateInfo(static_cast<uint8>( raceid ), static_cast<uint8>( i ));
		 if(info != NULL) break;
	}

	if(info == NULL)
	{
		RedSystemMessage(m_session, "Internal error: Could not find create info.");
		return false;
	}


	GreenSystemMessage(m_session, "Teleporting %s to %s starting location.", plr->GetName(), race.c_str());

	plr->SafeTeleport(info->mapId, 0, LocationVector(info->positionX, info->positionY, info->positionZ));
	return true;
}


bool ChatHandler::HandleInfoCommand(const char* args, WorldSession *m_session)
{
	WorldPacket data;


	//uint32 clientsNum = (uint32)sWorld.GetSessionCount();

	int gm = 0;
	int count = 0;
	int avg = 0;
	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		if(itr->second->GetSession())
		{
			count++;
			avg += itr->second->GetSession()->GetLatency();
			if(itr->second->GetSession()->GetPermissionCount())
				gm++;
		}
	}
	objmgr._playerslock.ReleaseReadLock();
	GreenSystemMessage(m_session, "Server Revision: |r%sArcEmu r%u/%s-%s-%s %s(www.arcemu.org)", MSG_COLOR_WHITE,
		BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH, MSG_COLOR_LIGHTBLUE);
	GreenSystemMessage(m_session, "Server Uptime: |r%s", sWorld.GetUptimeString().c_str());
	GreenSystemMessage(m_session, "Current Players: |r%d (%d GMs) (%d Peak)", count, gm,(int)sWorld.PeakSessionCount);
	GreenSystemMessage(m_session, "Active Thread Count: |r%u", ThreadPool.GetActiveThreadCount());
	GreenSystemMessage(m_session, "Free Thread Count: |r%u", ThreadPool.GetFreeThreadCount());
	GreenSystemMessage(m_session, "Average Latency: |r%.3fms", (float)((float)avg / (float)count));
	GreenSystemMessage(m_session, "SQL Query Cache Size (World): |r%u queries delayed", WorldDatabase.GetQueueSize());
	GreenSystemMessage(m_session, "SQL Query Cache Size (Character): |r%u queries delayed", CharacterDatabase.GetQueueSize());
	
	return true;
}

bool ChatHandler::HandleNetworkStatusCommand(const char* args, WorldSession *m_session)
{
	//sSocketMgr.ShowStatus();
	return true;
}

bool ChatHandler::HandleNYICommand(const char* args, WorldSession *m_session)
{
	RedSystemMessage(m_session, "Not yet implemented.");
	return true;
}

bool ChatHandler::HandleDismountCommand(const char* args, WorldSession *m_session)
{
	Unit *m_target = NULL;

	GET_PLAYER(RANK_COADMIN);

	if(plr)
		m_target = plr;
	else if (RANK_CHECK(RANK_ADMIN))
	{
		Creature *m_crt = getSelectedCreature(m_session, false);
		if(m_crt)
			m_target = m_crt;
	}

	if(!m_target)
	{
		RedSystemMessage(m_session, "No target found.");
		return true;
	}

	if(m_target->GetMount() == 0)
	{
		RedSystemMessage(m_session, "Target is not mounted.");
		return true;
	}

	if(plr && plr->m_MountSpellId)
		plr->RemoveAura(plr->m_MountSpellId);

	m_target->SetUInt32Value( UNIT_FIELD_MOUNTDISPLAYID , 0);
	BlueSystemMessage(m_session, "Now unmounted.");
	return true;
}


bool ChatHandler::HandleSaveCommand(const char* args, WorldSession *m_session)
{
	Player *p_target = getSelectedChar(m_session, false);
	if(!p_target || !p_target->IsPlayer())
		return false;

	if(p_target && p_target->m_nextSave < 300000 ) //5min out of 10 left so 5 min since last save
	{
		p_target->SaveToDB(false);
		GreenSystemMessage(m_session, "Player %s saved to DB", p_target->GetName());
	}
	else
	{
		RedSystemMessage(m_session, "You can only save once every 5 minutes.");
	}
	return true;
}

bool ChatHandler::HandleGMListCommand(const char* args, WorldSession *m_session)
{
	stringstream out;
	out << MSG_COLOR_GREEN << "Online GMs:|r\n";

	WorldPacket data;
	bool found = false;
	bool admin = RANK_CHECK(RANK_ADMIN);
	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		//if(itr->second->GetSession()->GetPermissionCount())//ORIGINAL LINE
		if(itr->second->GetSession()->GetPermissionCount())
		{
			if (!itr->second->m_isGmInvisible || admin)
			{
				found = true;

				if (itr->second->GetSession()->m_gmData->dev)
					out << MSG_COLOR_LIGHTBLUE; 
				else
					switch (itr->second->GetSession()->m_gmData->rank)
					{
					case RANK_SYRA:
						out << MSG_COLOR_GREENYELLOW;
						break;
					case RANK_ADMIN: //admin
						out << MSG_COLOR_ADMIN;
						break;
					case RANK_COADMIN:
						out << MSG_COLOR_COADMIN;
						break;
					}

				if (admin) //Add acct name for admins
					out << "[Account: " << itr->second->GetSession()->GetAccountName().c_str() << "] ";


				out << itr->second->GetName();
				out << " [";
				out << itr->second->GetSession()->m_gmData->perms.c_str();
				out << "]";

				if(itr->second->GetSession()->m_gmData->suspended >= (uint32)UNIXTIME)
					out << " -- SUSPENDED";

				if(itr->second->GetSession()->m_gmData->temp >= (uint32)UNIXTIME)
					out << " -- TEMPORARY";

				out << "\n";
			}
		}
	}

	objmgr._playerslock.ReleaseReadLock();
	if(!found)
	{
		out.clear();
		out << "There are no GMs currently logged in on this server. \n";
	}

	SendMultilineMessage(m_session, out.str().c_str());
	return true;
}

bool ChatHandler::HandleGMStatusCommand(const char* args, WorldSession *m_session)
{
	if(m_session->GetPlayer()->bGMTagOn)
		BlueSystemMessage(m_session, "GM Flag: On");
	else
		BlueSystemMessage(m_session, "GM Flag: Off");
	
	if(m_session->GetPlayer()->m_isGmInvisible)
		BlueSystemMessage(m_session, "GM Invis: On");
	else
		BlueSystemMessage(m_session, "GM Invis: Off");

	return true;
}

bool ChatHandler::HandleRangeCheckCommand( const char *args , WorldSession *m_session )
{
	WorldPacket data;
	uint64 guid = m_session->GetPlayer()->GetSelection();
	m_session->SystemMessage( "=== RANGE CHECK ===" );
	if(guid == 0)
	{
		m_session->SystemMessage("No selection.");
		return true;
	}

	Unit *unit = m_session->GetPlayer()->GetMapMgr()->GetUnit( guid );
	if(!unit)
	{
		m_session->SystemMessage("Invalid selection.");
		return true;
	}
	float DistSq = unit->GetDistanceSq( static_cast<Object*>(m_session->GetPlayer()) );
	m_session->SystemMessage( "GetDistanceSq  :   %u" , FL2UINT( DistSq ) );
	LocationVector locvec( m_session->GetPlayer()->GetPositionX() , m_session->GetPlayer()->GetPositionY() , m_session->GetPlayer()->GetPositionZ() );
	float DistReal = unit->CalcDistance( locvec );
	m_session->SystemMessage( "CalcDistance   :   %u" , FL2UINT( DistReal ) );
	float Dist2DSq = unit->GetDistance2dSq( static_cast<Object*>(m_session->GetPlayer()) );
	m_session->SystemMessage( "GetDistance2dSq:   %u" , FL2UINT( Dist2DSq ) );
	return true;
}

bool ChatHandler::HandleGmLogCommentCommand( const char *args , WorldSession *m_session )
{
	if(!args || !strlen(args)) return false;
	BlueSystemMessage(m_session, "Added Logcomment: %s",args);
	GMLog(m_session, "logcomment", args);
	return true;
}

bool ChatHandler::HandleRatingsCommand( const char *args , WorldSession *m_session )
{
	m_session->SystemMessage("Ratings!!!");
	Player* m_plyr = getSelectedChar(m_session, false);

	if( m_plyr == NULL )
		return false;

	for( uint32 i = 0; i < 24; i++ )
	{
		m_plyr->ModUnsigned32Value( PLAYER_FIELD_COMBAT_RATING_1 + i, i );
	}
	m_plyr->UpdateStats();
	return true;
}

float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float dx = x1 - x2;
	float dy = y1 - y2;
	float dz = z1 - z2;
	return sqrt(dx*dx + dy*dy + dz*dz);
}

bool ChatHandler::HandleSimpleDistanceCommand( const char *args , WorldSession *m_session )
{
	float toX, toY, toZ;
	if(sscanf(args, "%f %f %f", &toX, &toY, &toZ) != 3)
		return false;

	if(toX >= _maxX || toX <= _minX || toY <= _minY || toY >= _maxY)
		return false;

	float distance = CalculateDistance(
		m_session->GetPlayer()->GetPositionX(),
		m_session->GetPlayer()->GetPositionY(),
		m_session->GetPlayer()->GetPositionZ(),
		toX, toY, toZ);

	m_session->SystemMessage("Your distance to location (%f, %f, %f) is %0.2f meters.",
		toX, toY, toZ, distance);

	return true;
}

bool ChatHandler::HandleSendFailed( const char *args , WorldSession *m_session )
{
	Player* plr = getSelectedChar( m_session, true );
	if( plr == NULL )
		return false;

	uint32 fail = atol( args );
	if( SPELL_CANCAST_OK < fail )
	{
		RedSystemMessage( m_session, "Argument %u is out of range!", fail );
		return false;
	}
	plr->SendCastResult( 1, ( uint8 )fail, 0, 0 );
	return true;
}

bool ChatHandler::HandlePlayMovie( const char *args, WorldSession *m_session )
{
	Player* plr = getSelectedChar( m_session, true );
	if( plr == NULL )
		return false;

	uint32 movie = atol( args );
	
	plr->SendTriggerMovie( movie );

	SystemMessage( m_session, "Movie started." );
	return true;
}

bool ChatHandler::HandleAuraUpdateAdd( const char *args, WorldSession *m_session )
{
	if(!args)
		return false;

	uint32 SpellID = 0;
	int Flags = 0;
	int StackCount = 0;
	if(sscanf(args, "%u 0x%X %i", &SpellID, &Flags, &StackCount) != 3 && sscanf(args, "%u %u %i", &SpellID, &Flags, &StackCount) != 3)
		return false;

	Player * Pl = m_session->GetPlayer();
	if(Aura * AuraPtr = Pl->FindAura(SpellID))
	{
		uint8 VisualSlot = AuraPtr->m_visualSlot;
		WorldPacket data(SMSG_AURA_UPDATE, 20);
		FastGUIDPack(data, Pl->GetGUID());
		data << (uint8)VisualSlot;
		data << (uint32)SpellID;
		data << (uint8)Flags;
		data << (uint8)Pl->getLevel();
		data << (uint8)StackCount;
		if( !(Flags & AFLAG_NOT_CASTER) )
			data << WoWGuid(Pl->GetSelection());
		if(Flags & AFLAG_DURATION)
		{
			data << (uint32)AuraPtr->GetDuration();
			data << (uint32)AuraPtr->GetTimeLeft();
		}
		m_session->SendPacket(&data);
		SystemMessage(m_session, "SMSG_AURA_UPDATE (update): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
	}
	else
	{
		SpellEntry * Sp = dbcSpell.LookupEntryForced(SpellID);
		if(!Sp)
		{
			SystemMessage(m_session, "SpellID %u is invalid.", SpellID);
			return true;
		}
		Spell * SpellPtr = new Spell(Pl, Sp, false, NULL);
		AuraPtr = new Aura(Sp, SpellPtr->GetDuration(), Pl, Pl);
		Pl->AddAura(AuraPtr); // Serves purpose to just add the aura to our auraslots
		uint8 VisualSlot = Pl->FindVisualSlot(SpellID, AuraPtr->IsPositive());
		WorldPacket data(SMSG_AURA_UPDATE, 20);
		FastGUIDPack(data, Pl->GetGUID());
		data << (uint8)VisualSlot;
		data << (uint32)SpellID;
		data << (uint8)Flags;
		data << (uint8)Pl->getLevel();
		data << (uint8)StackCount;
		if( !(Flags & AFLAG_NOT_CASTER) )
			data << (uint8)0; // caster guid
		if(Flags & AFLAG_DURATION)
		{
			data << (uint32)SpellPtr->GetDuration();
			data << (uint32)SpellPtr->GetDuration();
		}
		m_session->SendPacket(&data);
		SystemMessage(m_session, "SMSG_AURA_UPDATE (add): VisualSlot %u - SpellID %u - Flags %i (0x%04X) - StackCount %i", VisualSlot, SpellID, Flags, Flags, StackCount);
		delete SpellPtr;
	}
	return true;
}

bool ChatHandler::HandleAuraUpdateRemove( const char *args, WorldSession *m_session )
{
	if(!args)
		return false;

	char * pArgs = strtok((char*)args, " ");
	if(!pArgs)
		return false;
	uint8 VisualSlot = (uint8)atoi(pArgs);
	Player * Pl = m_session->GetPlayer();
	Aura * AuraPtr = Pl->FindAura(Pl->m_auravisuals[VisualSlot]);
	if(!AuraPtr)
	{
		SystemMessage(m_session, "No auraid found in slot %u", VisualSlot);
		return true;
	}
	WorldPacket data(SMSG_AURA_UPDATE, 20);
	FastGUIDPack(data, Pl->GetGUID());
	data << (uint8)VisualSlot;
	data << (uint32)0;
	m_session->SendPacket(&data);
	SystemMessage(m_session, "SMSG_AURA_UPDATE (remove): VisualSlot %u - SpellID 0", VisualSlot);
	AuraPtr->Remove();
	return true;
}

bool ChatHandler::HandlePhaseCommand( const char *args , WorldSession *m_session )
{
	Player *p_target = getSelectedChar(m_session, false);
	if(!p_target || !p_target->IsPlayer())
		return false;

	if(strlen(args)<1) {
		SystemMessage(m_session, "%s phase:%s%u",MSG_COLOR_GREEN,MSG_COLOR_LIGHTBLUE,p_target->GetPhase());
		return true;
	}

	uint32 i = atoi(args);
	p_target->Phase(PHASE_SET, i);

	if( p_target->GetSession() )
	{
		WorldPacket data(SMSG_SET_PHASE_SHIFT, 4);
		data << i;
		p_target->GetSession()->SendPacket(&data);
	}

	return true;
}
