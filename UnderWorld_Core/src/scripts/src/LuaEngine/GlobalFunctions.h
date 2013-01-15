/*
 * ArcScript Scripts for Arcemu MMORPG Server
 * Copyright (C) 2008-2009 Arcemu Team
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.com/>
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
 */

#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include "StdAfx.h"
#include "LUAEngine.h"
#include "svn_revision.h"

#define ENGINE_NAME "LuaHypArc" //You should check in your scripts that GetLuaEngine() == "LuaHypArc"
#ifdef LUA_SVN_REVISION_H
#define ENGINE_VERSION LUA_REVISION //If you need a specific version for your script to work, check GetLuaEngineVersion() >= number
#else
#define ENGINE_VERSION 0 //linux people
#endif
//now the ArcScripts revision

namespace luaGlobalFunctions 
{
	static int PerformIngameSpawn(lua_State * L)
	{
		uint32 spawntype = luaL_checkint(L, 1);
		uint32 entry_id = luaL_checkint(L, 2);
		uint32 map = luaL_checkint(L, 3);
		float x = CHECK_FLOAT(L, 4);
		float y = CHECK_FLOAT(L, 5);
		float z = CHECK_FLOAT(L, 6);
		float o = CHECK_FLOAT(L, 7);
		uint32 faction = luaL_checkint(L, 8); //also scale as percentage
		uint32 duration = luaL_checkint(L, 9);
		uint32 equip1 = luaL_optint(L, 10, 1);
		uint32 equip2 = luaL_optint(L, 11, 1);
		uint32 equip3 = luaL_optint(L, 12, 1);
		if(x && y && z && entry_id)
		{
			if (spawntype == 1) //Unit
			{ 
				CreatureProto *p = CreatureProtoStorage.LookupEntry(entry_id);
				//CreatureInfo *i = CreatureNameStorage.LookupEntry(entry_id);
				if (p == NULL)// || i == NULL) 
					return 0;
				MapMgr *mapMgr = sInstanceMgr.GetMapMgr(map);
				if (!mapMgr)
					return 0;
				int32 instanceid = luaL_optint(L, 13, mapMgr->GetInstanceID());
				Creature * pCreature = mapMgr->GetInterface()->SpawnCreature(entry_id,x,y,z,o,true,true,0,0);
				pCreature->SetFaction(faction);
				pCreature->_setFaction();
				pCreature->SetInstanceID(instanceid);
				pCreature->SetMapId(map);
				pCreature->SetEquippedItem(MELEE,equip1);
				pCreature->SetEquippedItem(OFFHAND,equip2);
				pCreature->SetEquippedItem(RANGED,equip3);
				if (duration>0) 
					pCreature->Despawn(duration,0);
				PUSH_UNIT(L,pCreature);
			} 
			else if (spawntype == 2) //GO
			{ 
				GameObjectInfo *n = GameObjectNameStorage.LookupEntry(entry_id);
				if (n == NULL)
					return 0;
				MapMgr *mapMgr = sInstanceMgr.GetMapMgr(map);
				if (!mapMgr)
					return 0;
				int32 instanceid = luaL_optint(L, 13, mapMgr->GetInstanceID());
				GameObject * pC = mapMgr->GetInterface()->SpawnGameObject(entry_id,x,y,z,o,true,0,0); //phase
				pC->SetInstanceID(instanceid);
				pC->SetMapId(map);
				pC->SetFloatValue(OBJECT_FIELD_SCALE_X, faction/100*1.0f); 
				pC->Spawn(mapMgr);
				if (duration > 0) pC->Despawn(duration, 0);
				PUSH_GO(L,pC);
			}
			else
				lua_pushnil(L);
		}
		else
			lua_pushnil(L);
		return 1;
	}
	
	static int GetGameTime(lua_State * L)
	{
		lua_pushnumber(L, ((uint32)sWorld.GetGameTime())); //in seconds.
		return 1;
	}

	static int GetPlayer(lua_State * L)
	{
		const char * plName = luaL_checkstring(L,1);
		Player * plr = objmgr.GetPlayer(plName);
		if (plr)
		{
			if (plr->IsInWorld()) 
			{
				PUSH_UNIT(L, plr);
				return 1;
			}
		}
		lua_pushboolean(L, 0);
		return 1;
	}

	static int GetLUAEngine(lua_State * L) //also mapped to GetLuaEngine()
	{
		lua_pushstring(L, ENGINE_NAME);
		return 1;
	}

	static int GetLuaEngineVersion(lua_State * L)
	{
		lua_pushnumber(L, ENGINE_VERSION);
		return 1;
	}

	static int logcol(lua_State * L)
	{
		int color = luaL_checkint(L,1);
		Log.Color(color);
		return 0;
	}
	
	static int WorldDBQuery(lua_State * L)
	{
		const char * qStr = luaL_checkstring(L,1);
		uint32 fID = luaL_optint(L,2,0); //column
		uint32 rID = luaL_optint(L,3,0); //row
		if (!qStr) return 0;
		QueryResult * result = WorldDatabase.Query(qStr);
		PUSH_SQLRESULT(L,result);
		return 1;
	}

	static int CharDBQuery(lua_State * L)
	{
		const char * qStr = luaL_checkstring(L,1);
		uint32 fID = luaL_optint(L,2,0); //column
		uint32 rID = luaL_optint(L,3,0); //row
		if (!qStr) return 0;
		QueryResult * result = CharacterDatabase.Query(qStr);
		PUSH_SQLRESULT(L,result);
		return 1;
	}

	static int WorldDBQueryTable(lua_State * L)
	{
		const char * qStr = luaL_checkstring(L,1);
		lua_newtable(L);
		if(!qStr) return 0;
		QueryResult * result = WorldDatabase.Query(qStr);
		PUSH_SQLRESULT(L,result);
		return 1;
	}

	static int CharDBQueryTable(lua_State * L)
	{
		const char * qStr = luaL_checkstring(L,1);
		lua_newtable(L);
		if(!qStr) return 0;
		QueryResult * result = CharacterDatabase.Query(qStr);
		PUSH_SQLRESULT(L,result);
		return 1;
	}
	
	static int SendWorldMessage(lua_State * L)
	{
		const char * message = luaL_checkstring(L,1);
		uint32 MsgType = luaL_checkint(L,2);
		if(!message || !MsgType)
		{
			lua_pushnil(L);
			return 1;
		}
		if (MsgType == 1)
			sWorld.SendWorldWideScreenText(message);
		else if (MsgType == 2)
			sWorld.SendWorldText(message);
		return 0;
	}

	static int ReloadTable(lua_State * L)
	{
		const char * TableName = luaL_checkstring(L,1);
		if (!stricmp(TableName, "spell_disable"))
		{
			objmgr.ReloadDisabledSpells();
		} 
		else if (!stricmp(TableName, "vendors"))
		{
			objmgr.ReloadVendors();
		}
		else
		{
			if (!stricmp(TableName, "items"))					// Items
				ItemPrototypeStorage.Reload();
			else if (!stricmp(TableName, "creature_proto"))		// Creature Proto
				CreatureProtoStorage.Reload();
			else if (!stricmp(TableName, "creature_names"))		// Creature Names
				CreatureNameStorage.Reload();
			else if (!stricmp(TableName, "gameobject_names"))	// GO Names
				GameObjectNameStorage.Reload();
			else if (!stricmp(TableName, "areatriggers"))		// Areatriggers
				AreaTriggerStorage.Reload();
			else if (!stricmp(TableName, "itempages"))			// Item Pages
				ItemPageStorage.Reload();
			else if (!stricmp(TableName, "worldstring_tables"))			// wst
				WorldStringTableStorage.Reload();
			else if (!stricmp(TableName, "worldbroadcast"))			// wb
				WorldBroadCastStorage.Reload();
			else if (!stricmp(TableName, "quests"))				// Quests
				QuestStorage.Reload();
			else if (!stricmp(TableName, "npc_text"))			// NPC Text Storage
				NpcTextStorage.Reload();
			else if (!stricmp(TableName, "fishing"))				// Fishing Zones
				FishingZoneStorage.Reload();
			else if(!stricmp(TableName, "teleport_coords"))		// Teleport coords
				TeleportCoordStorage.Reload();
			else if (!stricmp(TableName, "graveyards"))			// Graveyards
				GraveyardStorage.Reload();
			else if (!stricmp(TableName, "worldmap_info"))		// WorldMapInfo
				WorldMapInfoStorage.Reload();
			else if (!stricmp(TableName, "zoneguards"))
				ZoneGuardStorage.Reload();
			else if (!stricmp(TableName, "unit_display_sizes"))
				UnitModelSizeStorage.Reload();
			else if (!stricmp(TableName, "command_overrides"))	// Command Overrides
			{
				CommandTableStorage::getSingleton().Dealloc();
				CommandTableStorage::getSingleton().Init();
				CommandTableStorage::getSingleton().Load();
			}
		}
		return 0;
	}

	static int ReloadLuaEngine(lua_State * L)
	{
		/*g_luaMgr.Restart();
		MapMgr * mgr;
		LuaUnitBinding * m_binding;
		for (uint32 i = 0; i < NUM_MAPS; ++i)
        {
			if (!sInstanceMgr.GetMapMgr(i))
				continue;
			mgr = sInstanceMgr.GetMapMgr(i);
			for(uint32 guid=1; guid < mgr->m_CreatureArraySize; guid++)
			{
				Creature *pCreature = mgr->GetCreature(GET_LOWGUID_PART(guid));
				if(pCreature)
				{
					m_binding = g_luaMgr.GetUnitBinding(pCreature->GetEntry());
					if (m_binding != NULL)
						g_engine->OnUnitEvent( pCreature, m_binding->Functions[CREATURE_EVENT_ON_LOAD], CREATURE_EVENT_ON_LOAD, NULL, 0, 0, 0, 0, NULL );		
				}
			}
		}
		mgr->KillThread();*/
		return 0;
	}

	static int GetPlayersInWorld(lua_State * L)
	{
		Player * ret = NULL;
		uint32 count = 0;
		lua_newtable(L);
		objmgr._playerslock.AcquireReadLock();

        HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;
        for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
        {
			count++,
			ret = (*itr).second;
			lua_pushinteger(L,count);
			PUSH_UNIT(L,((Unit*)ret));
			lua_rawset(L,-3);
		}
		objmgr._playerslock.ReleaseReadLock();	
		return 1;
	}

	static int Rehash(lua_State * L)
	{
		sWorld.Rehash(true);
		return 0;
	}

	static int GetArcemuRevision(lua_State * L)
	{
		lua_pushnumber(L,BUILD_REVISION);
		return 1;
	}

	/*static int GetInstanceIdsByMap(lua_State * L)
	{
		uint32 mapid = luaL_checkint(L,1);
		uint32 ret = NULL;
		uint32 count = 0;
		lua_newtable(L);

		InstanceMap * instancemap = sInstanceMgr.GetInstanceMap(mapid);
		for(InstanceMap::iterator itr = instancemap->begin(); itr != instancemap->end(); ++itr)
		{
			count++,
			ret = itr->second->m_instanceId;
			lua_pushinteger(L,count);
			lua_pushinteger(L,ret);
			lua_rawset(L,-3);
		}
		return 1;
	}*/
	//////////////////////////////////////////////////////////////////////////
	// WORLD PVP NOT SUPPORTED!
	//////////////////////////////////////////////////////////////////////////
	/*
	static int SendPvPCaptureMessage(lua_State * L)
	{
		uint32 zoneid = luaL_checkint(L, 1);
		const char* msg = luaL_checkstring(L, 2);
		AreaTable * at = dbcArea.LookupEntry(zoneid);
		if(!zoneid || !msg || !at)
			return 1;
		MapMgr* mapmgr = sInstanceMgr.GetMapMgr(at->mapId);
		if (mapmgr)
			mapmgr->SendPvPCaptureMessage(ZONE_MASK_ALL, zoneid, msg);
		return 1;
	}
	*/
	static int GetPlayersInMap(lua_State * L)
	{
		Player * ret = NULL;
		uint32 count = 0;
		lua_newtable(L);
		uint32 mapid = luaL_checkint(L,1);
		MapMgr * mgr = sInstanceMgr.GetMapMgr(mapid);
		if (!mgr)	
			return 0;
		for (PlayerStorageMap::iterator itr = mgr->m_PlayerStorage.begin(); itr != mgr->m_PlayerStorage.end(); ++itr)
		{
			count++,
			ret = (*itr).second;
			lua_pushinteger(L,count);
			PUSH_UNIT(L,((Unit*)ret));
			lua_rawset(L,-3);
		}
		return 1;
	}

	static int GetPlayersInZone(lua_State * L)
	{
		Player * ret = NULL;
		uint32 count = 0;
		lua_newtable(L);
		uint32 zoneid = luaL_checkint(L,1);
		objmgr._playerslock.AcquireReadLock();
		HM_NAMESPACE::hash_map<uint32, Player*>::const_iterator itr;
        for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
		{
			if ((*itr).second->GetZoneId() == zoneid)
			{
				count++,
				ret = (*itr).second;
				lua_pushinteger(L,count);
				PUSH_UNIT(L,((Unit*)ret));
				lua_rawset(L,-3);
			}
		}
		objmgr._playerslock.ReleaseReadLock();
		return 1;
	}

	static int SendMail(lua_State * L)
	{
		uint32 type = luaL_checkint(L,1);
		uint64 sender_guid = CHECK_GUID(L,2);
		uint64 recipient_guid = CHECK_GUID(L,3);
		string subject = luaL_checkstring(L,4);
		string body = luaL_checkstring(L,5);
		uint32 money = luaL_checkint(L,6);
		uint32 cod = luaL_checkint(L,7);
		uint64 item_guid = CHECK_GUID(L,8);
		uint32 stationery = luaL_checkint(L,9);
		sMailSystem.SendAutomatedMessage(type, sender_guid, recipient_guid, subject, body, money, cod, item_guid, stationery);
		return 0;
	}

	static int GetTaxiPath(lua_State * L)
	{
		uint32 path = luaL_checkint(L, 1);
		TaxiPath * tp = sTaxiMgr.GetTaxiPath(path);
		if (tp != NULL)
			PUSH_TAXIPATH(L, tp);
		else
			lua_pushnil(L);
		return 1;
	}

	static int SetDBCSpellVar(lua_State * L)
	{
		uint32 entry = luaL_checkinteger(L,1);
		const char* var = luaL_checkstring(L,2);
		int subindex = 0;
		if (lua_gettop(L) == 4)
		{
			subindex = luaL_optint(L,3,0);
		}
		SpellEntry * proto = dbcSpell.LookupEntryForced(entry);
		if (!entry || !var || subindex < 0 || !proto) 
		{ 
			lua_pushboolean(L, 0); 
			return 1; 
		}
		for (uint32 itr = 0; luaSpellVars[itr].name != NULL; itr++)
		{
			LuaSpellEntry l = luaSpellVars[itr];
			if (strcmp(l.name, var) == 0) //they entered a correct var name
			{
				int valindex = 3;
				if (subindex)
					valindex++;
				switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
				{
				case 0:
					*(int*)((char*)proto + l.offset + subindex) = luaL_checkinteger(L, valindex);
					lua_pushboolean(L, 1);
					break;
				case 1:
					strcpy((*(char**)((char*)proto + l.offset + subindex)), luaL_checkstring(L, valindex));
					lua_pushboolean(L, 1);
					break;
				case 2:
					*(bool*)((char*)proto + l.offset + subindex) = CHECK_BOOL(L, valindex);
					lua_pushboolean(L, 1);
					break;
				case 3:
					*(float*)((char*)proto + l.offset + subindex) = (float)luaL_checknumber(L, valindex);
					lua_pushboolean(L, 1);
					break;
				}
				return 1;
			}
		}
		lua_pushboolean(L, 0); //wrong var name
		return 1;
	}

	static int GetDBCSpellVar(lua_State * L)
	{
		uint32 entry = luaL_checkinteger(L,1);
		const char* var = luaL_checkstring(L,2);
		int subindex = luaL_optint(L,3,0);
		SpellEntry * proto = dbcSpell.LookupEntryForced(entry);
		if (!entry || !var || subindex < 0 || !proto) 
		{ 
			lua_pushnil(L);
			return 1; 
		}
		for (uint32 itr = 0; luaSpellVars[itr].name != NULL; itr++)
		{
			LuaSpellEntry l = luaSpellVars[itr];
			if (strcmp(l.name, var) == 0) //they entered a correct var name
			{
				switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
				{
				case 0:
					lua_pushinteger(L, *(int*)((char*)proto + l.offset + subindex));
					break;
				case 1:
					Log.Notice("LHA DEBUG", "%s", proto->Name);
					lua_pushstring(L, *(char**)((char*)proto + l.offset + subindex));
					break;
				case 2:
					lua_pushboolean(L, (*(bool*)((char*)proto + l.offset + subindex)) ? 1 : 0);
					break;
				case 3:
					lua_pushnumber(L, *(float*)((char*)proto + l.offset + subindex));
					break;
				}
				return 1;
			}
		}
		lua_pushnil(L); //wrong var name
		return 1;
	}

	void Register(lua_State *L) {
		lua_pushcfunction(L, &PerformIngameSpawn);
		lua_setglobal(L, "PerformIngameSpawn");

		lua_pushcfunction(L, &GetPlayer);
		lua_setglobal(L, "GetPlayer");

		lua_pushcfunction(L, &GetLUAEngine);
		lua_setglobal(L, "GetLUAEngine");

		lua_pushcfunction(L, &GetLUAEngine);
		lua_setglobal(L, "GetLuaEngine");

		lua_pushcfunction(L, &GetLuaEngineVersion);
		lua_setglobal(L, "GetLuaEngineVersion");

		lua_pushcfunction(L, &GetGameTime);
		lua_setglobal(L, "GetGameTime");
		
		lua_pushcfunction(L, &WorldDBQuery);
		lua_setglobal(L, "WorldDBQuery");

		lua_pushcfunction(L, &CharDBQuery);
		lua_setglobal(L, "CharDBQuery");

		lua_pushcfunction(L, &WorldDBQueryTable);
		lua_setglobal(L, "WorldDBQueryTable");

		lua_pushcfunction(L, &CharDBQueryTable);
		lua_setglobal(L, "CharDBQueryTable");
		
		lua_pushcfunction(L, &SendWorldMessage);
		lua_setglobal(L, "SendWorldMessage");

		lua_pushcfunction(L, &ReloadTable);
		lua_setglobal(L, "ReloadTable");

		lua_pushcfunction(L, &ReloadLuaEngine);
		lua_setglobal(L, "ReloadLuaEngine");

		lua_pushcfunction(L, &Rehash);
		lua_setglobal(L, "Rehash");

		lua_pushcfunction(L, &logcol);
		lua_setglobal(L, "logcol");

		lua_pushcfunction(L, &GetPlayersInWorld);
		lua_setglobal(L, "GetPlayersInWorld");

		lua_pushcfunction(L, &GetArcemuRevision);
		lua_setglobal(L, "GetArcemuRevision");

		/*lua_pushcfunction(L, &GetInstanceIdsByMap);
		lua_setglobal(L, "GetInstanceIdsByMap");*/

		/*
		lua_pushcfunction(L, &SendPvPCaptureMessage);
		lua_setglobal(L, "SendPvPCaptureMessage");
		*/
		lua_pushcfunction(L, &GetPlayersInMap);
		lua_setglobal(L, "GetPlayersInMap");

		lua_pushcfunction(L, &GetPlayersInZone);
		lua_setglobal(L, "GetPlayersInZone");

		/*lua_pushcfunction(L, &RegisterTimedEvent);
		lua_setglobal(L, "RemoveTimedEvent");*/

		lua_pushcfunction(L, &SendMail);
		lua_setglobal(L, "SendMail");

		lua_register(L, "GetTaxiPath", &GetTaxiPath);
		lua_register(L, "SetDBCSpellVar", &SetDBCSpellVar);
		lua_register(L, "GetDBCSpellVar", &GetDBCSpellVar);
	}
}
void RegisterGlobalFunctions(lua_State *L)
{
	lua_pushcfunction(L, &luaGlobalFunctions::PerformIngameSpawn);
	lua_setglobal(L, "PerformIngameSpawn");

	lua_pushcfunction(L, &luaGlobalFunctions::GetPlayer);
	lua_setglobal(L, "GetPlayer");

	lua_pushcfunction(L, &luaGlobalFunctions::GetLUAEngine);
	lua_setglobal(L, "GetLUAEngine");

	lua_pushcfunction(L, &luaGlobalFunctions::GetLUAEngine);
	lua_setglobal(L, "GetLuaEngine");

	lua_pushcfunction(L, &luaGlobalFunctions::GetLuaEngineVersion);
	lua_setglobal(L, "GetLuaEngineVersion");

	lua_pushcfunction(L, &luaGlobalFunctions::GetGameTime);
	lua_setglobal(L, "GetGameTime");
	
	lua_pushcfunction(L, &luaGlobalFunctions::WorldDBQuery);
	lua_setglobal(L, "WorldDBQuery");

	lua_pushcfunction(L, &luaGlobalFunctions::CharDBQuery);
	lua_setglobal(L, "CharDBQuery");

	lua_pushcfunction(L, &luaGlobalFunctions::WorldDBQueryTable);
	lua_setglobal(L, "WorldDBQueryTable");

	lua_pushcfunction(L, &luaGlobalFunctions::CharDBQueryTable);
	lua_setglobal(L, "CharDBQueryTable");
	
	lua_pushcfunction(L, &luaGlobalFunctions::SendWorldMessage);
	lua_setglobal(L, "SendWorldMessage");

	lua_pushcfunction(L, &luaGlobalFunctions::ReloadTable);
	lua_setglobal(L, "ReloadTable");

	lua_pushcfunction(L, &luaGlobalFunctions::ReloadLuaEngine);
	lua_setglobal(L, "ReloadLuaEngine");

	lua_pushcfunction(L, &luaGlobalFunctions::Rehash);
	lua_setglobal(L, "Rehash");

	lua_pushcfunction(L, &luaGlobalFunctions::logcol);
	lua_setglobal(L, "logcol");

	lua_pushcfunction(L, &luaGlobalFunctions::GetPlayersInWorld);
	lua_setglobal(L, "GetPlayersInWorld");

	lua_pushcfunction(L, &luaGlobalFunctions::GetArcemuRevision);
	lua_setglobal(L, "GetArcemuRevision");

	/*lua_pushcfunction(L, &luaGlobalFunctions::GetInstanceIdsByMap);
	lua_setglobal(L, "GetInstanceIdsByMap");*/

	/*
	lua_pushcfunction(L, &luaGlobalFunctions::SendPvPCaptureMessage);
	lua_setglobal(L, "SendPvPCaptureMessage");
	*/
	lua_pushcfunction(L, &luaGlobalFunctions::GetPlayersInMap);
	lua_setglobal(L, "GetPlayersInMap");

	lua_pushcfunction(L, &luaGlobalFunctions::GetPlayersInZone);
	lua_setglobal(L, "GetPlayersInZone");

	/*lua_pushcfunction(L, &luaGlobalFunctions::RegisterTimedEvent);
	lua_setglobal(L, "RemoveTimedEvent");*/

	lua_pushcfunction(L, &luaGlobalFunctions::SendMail);
	lua_setglobal(L, "SendMail");

	lua_register(L, "GetTaxiPath", &luaGlobalFunctions::GetTaxiPath);
	lua_register(L, "SetDBCSpellVar", &luaGlobalFunctions::SetDBCSpellVar);
	lua_register(L, "GetDBCSpellVar", &luaGlobalFunctions::GetDBCSpellVar);
}
#endif
