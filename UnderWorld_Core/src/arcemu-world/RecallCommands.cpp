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

bool GetRecallLocation(const char* location, uint32 &map, LocationVector &v)
{
	QueryResult *result = WorldDatabase.Query( "SELECT * FROM recall ORDER BY name" );

	if( result == NULL)
		return false;

	do
	{
		Field* fields = result->Fetch();
		const char* locname = fields[1].GetString();
		uint32 locmap = fields[2].GetUInt32();
		float x = fields[3].GetFloat();
		float y = fields[4].GetFloat();
		float z = fields[5].GetFloat();
		float o = fields[6].GetFloat();

		if( strnicmp( const_cast< char* >( location ), locname, strlen( location ) ) == 0 )
		{
			map = locmap;
			v.x = x;
			v.y = y;
			v.z = z;
			v.o = o;
			delete result;
			return true;
		}

	}while (result->NextRow());

	delete result;
	return false;

}

bool ChatHandler::HandleRecallGoCommand(const char* args, WorldSession *m_session)
{
	if( args == NULL )
		return false;

	if( !*args )
		return false;

	if( m_session == NULL )
		return false;

	uint32 map;
	LocationVector v;
	if (GetRecallLocation(args, map, v))
	{
		if( m_session->GetPlayer() != NULL )
		{
			if (!m_session->GetPlayer()->AllowedPortTo(map, true))
				return true;

			if (m_session->GetPlayer()->DuelingWith != NULL) 
			{ 
				RedSystemMessage(m_session, "ERROR: You cannot use this command while dueling."); 
				return true; 
			} 

			if (map == 389 && !RANK_CHECK(RANK_COADMIN))
			{
				RedSystemMessage(m_session, "ERROR: You cannot use this command to enter the mall. Please use the Telebook.");
				return true;
			}
			m_session->GetPlayer()->SafeTeleport(map, 0, v);
			return true;
		}
		return false;
	}
	return false;
}

bool ChatHandler::HandleRecallPortUsCommand(const char* args, WorldSession * m_session)
{
	if( args == NULL )
		return false;

	if( !*args )
		return false;

	if( m_session == NULL )
		return false;

	uint32 map;
	LocationVector v;
	Player *player = m_session->GetPlayer();
	if (m_session->GetPlayer()->DuelingWith != NULL) 
	{ 
		RedSystemMessage(m_session, "ERROR: You cannot use this command while dueling."); 
		return true; 
	}

	if (GetRecallLocation(args, map, v))
	{
		Player * target = objmgr.GetPlayer((uint32)m_session->GetPlayer()->GetSelection());
		if (!target)
		{
			SystemMessage(m_session, "Select a player first.");
			return true;
		}
		player->SafeTeleport(map, 0, v);
		target->SafeTeleport(map, 0, v);
		return true;
	}
	return false;
}

bool ChatHandler::HandleRecallAddCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;
	
	uint32 adds;
	bool new_gm = false;
	if(!RANK_CHECK(RANK_ADMIN))
	{
		QueryResult *res = WorldDatabase.Query( "SELECT adds FROM _recall_adds WHERE acct=%u", m_session->GetAccountId() );
		if(res)
		{
			adds = res->Fetch()[0].GetUInt32();
			if(adds <= 0)
			{
				RedSystemMessage(m_session,"ERROR: You no longer have any recall add saves left.");
				return true;
			}
		}
		else
		{
			adds=5;
			new_gm=true;
		}
		delete res;
	}

	QueryResult *result = WorldDatabase.Query( "SELECT name FROM recall" );
	if(!result)
		return false;
	do
	{
		Field *fields = result->Fetch();
		const char * locname = fields[0].GetString();
		if (strncmp((char*)args,locname,strlen(locname))== 0)
		{
			RedSystemMessage(m_session, "Name in use, please use another name for your location.");
			delete result;
			return true;
		}
	}while (result->NextRow());
	delete result;


	Player* plr = m_session->GetPlayer();
	std::stringstream ss;
	
	string rc_locname = string(args);

	ss << "INSERT INTO recall (name, mapid, positionX, positionY, positionZ, Orientation) VALUES ('"
	<< WorldDatabase.EscapeString(rc_locname).c_str() << "' , "
	<< plr->GetMapId()     << ", "
	<< plr->GetPositionX() << ", " 
	<< plr->GetPositionY() << ", "
	<< plr->GetPositionZ() << ", "
	<< plr->GetOrientation() << ");";
	WorldDatabase.Execute( ss.str( ).c_str( ) );

	char buf[256]; 
	snprintf((char*)buf, 256, "Added location to DB with MapID: %d, X: %f, Y: %f, Z: %f, O: %f",
		(unsigned int)plr->GetMapId(), plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetOrientation());
	GreenSystemMessage(m_session, buf);
	//sGMLog.writefromsession(m_session, "used recall add, added \'%s\' location to database.", rc_locname.c_str());
	char command[100]="recall add";
	char notes[1024];
	snprintf(notes, 1024, "%s", rc_locname.c_str());
	GMLog(m_session, command, notes);

	if(!RANK_CHECK(RANK_ADMIN))
	{
		adds-=1;
		if(adds < 0)
			adds=0;
		if(!new_gm)
			WorldDatabase.Execute("UPDATE _recall_adds SET adds=%u WHERE acct=%u", adds, m_session->GetAccountId() );
		else
			WorldDatabase.Execute("INSERT INTO _recall_adds VALUES(%u,%u)", m_session->GetAccountId(), adds);
	}

	return true;
}

bool ChatHandler::HandleRecallDelCommand(const char* args, WorldSession *m_session)
{
	   if(!*args)
		return false;

	QueryResult *result = WorldDatabase.Query( "SELECT id,name FROM recall" );
	if(!result)
		return false;

	do
	{
		Field *fields = result->Fetch();
		float id = fields[0].GetFloat();
		const char * locname = fields[1].GetString();

		if (strnicmp((char*)args,locname,strlen(locname))== 0)
		{
			std::stringstream ss;
			ss << "DELETE FROM recall WHERE id = "<< (int)id <<";";
			WorldDatabase.Execute( ss.str( ).c_str( ) );
			GreenSystemMessage(m_session, "Recall location removed.");
			sGMLog.writefromsession(m_session, "used recall delete, removed \'%s\' location from database.", args);
			delete result;
			return true;
		}

	}while (result->NextRow());
	delete result;
	return false;
}

bool ChatHandler::HandleRecallListCommand(const char* args, WorldSession *m_session)
{
	QueryResult *result = WorldDatabase.Query( "SELECT name FROM recall");
	if( !result )
		return false;
	std::string recout;
	uint32 count = 0;

	recout = MSG_COLOR_GREEN;
	recout += "Recall locations|r:\n\n";
	do
	{
	   Field *fields = result->Fetch();
		const char * locname = fields[0].GetString();
		recout += MSG_COLOR_LIGHTBLUE;
		recout += locname;
		recout += "|r, ";
		count++;
		
		if(count == 5)
		{
		   recout += "\n";
		   count = 0;
		}
	}
	while ( result->NextRow() );
	SendMultilineMessage( m_session, recout.c_str() );

	delete result;
	return true;
}

bool ChatHandler::HandleRecallPortPlayerCommand(const char* args, WorldSession * m_session)
{
	char location[255];
	char player[255];
	if(sscanf(args, "%s %s", player, location) != 2)
		return false;

	Player * plr = objmgr.GetPlayer(player, false);
	if(!plr) return false;

	if (!EnoughRankCheck(m_session->GetPlayer(), plr))
		return true;

	DUEL_CHECK(RANK_IMPOSSIBLE);

	if (plr->GetMapId() == 44 && !RANK_CHECK(RANK_COADMIN))
	{
		char buf[256];
		snprintf(buf, 256, "%s is trying to summon me!.", m_session->GetPlayer()->GetName());
		WorldPacket * data = FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, args, plr->GetGUID(), 0);
		plr->SendMessageToSet(data, true);
		delete data;

		RedSystemMessage(m_session, "ERROR: You cannot remove a player from the jail.");
		return true;	
	}

	QueryResult *result = WorldDatabase.Query( "SELECT * FROM recall ORDER BY name" );
	if(!result)
		return false;

	do
	{
		Field *fields = result->Fetch();
		const char * locname = fields[1].GetString();
		uint32 locmap = fields[2].GetUInt32();
		float x = fields[3].GetFloat();
		float y = fields[4].GetFloat();
		float z = fields[5].GetFloat();
		float o = fields[6].GetFloat();

		if (strnicmp((char*)location,locname,strlen(args))== 0)
		{
			if (!plr->AllowedPortTo(locmap, false) && m_session->m_gmData->rank < RANK_COADMIN)
			{
				RedSystemMessage(m_session, "ERROR: That player is not allowed on that map.");
				return true;
			}
			char notes[256];
			snprintf(notes, 256, "Coords: %u %2.2f %2.2f %2.2f %2.2f", locmap, x, y, z, o);
			GMLog(m_session, plr->GetSession(), "recall portplayer", notes);

			if(plr->GetSession() && !m_session->GetPlayer()->m_isGmInvisible)
				plr->GetSession()->SystemMessage("%s teleported you to location %s!", m_session->GetPlayer()->GetName(), locname);

			if(plr->GetInstanceID() != m_session->GetPlayer()->GetInstanceID())
				sEventMgr.AddEvent(plr, &Player::EventSafeTeleport, locmap, uint32(0), LocationVector(x, y, z, o), EVENT_PLAYER_TELEPORT, 1, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
			else
				plr->SafeTeleport(locmap, 0, LocationVector(x, y, z, o));
			delete result;
			return true;
		}

	}while (result->NextRow());

	delete result;

	return false;
}
