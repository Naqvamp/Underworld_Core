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

#include "StdAfx.h"
#include "LUAEngine.h"
#include <ScriptSetup.h>

#ifdef WIN32
#pragma warning(disable:4129)
#pragma warning(disable:4244)
#endif

#if PLATFORM != PLATFORM_WIN32
#include <dirent.h>
#endif

ScriptMgr * m_scriptMgr = NULL;
LuaEngine g_luaMgr;
	

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{ 
	return SCRIPT_TYPE_SCRIPT_ENGINE | SCRIPT_TYPE_SCRIPT_ENGINE_LUA;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
	m_scriptMgr = mgr;
	sLuaMgr.Startup();
}
extern "C" SCRIPT_DECL void _export_engine_reload()
{
	sLuaMgr.Restart();
}

template<typename T> const char * GetTClassName() { return "UNKNOWN"; }
template<> const char * GetTClassName<Unit>() { return "Unit"; }
template<> const char * GetTClassName<Item>() { return "Item"; }
template<> const char * GetTClassName<GameObject>() { return "GameObject"; }
template<> const char * GetTClassName<WorldPacket>() { return "LuaPacket"; }
template<> const char * GetTClassName<TaxiPath>() { return "LuaTaxi"; }
template<> const char * GetTClassName<Spell>() { return "Spell"; }
template<> const char * GetTClassName<Field>() { return "SQL_Field"; }
template<> const char * GetTClassName<QueryResult>() { return "SQL_QResult"; }

template<typename T> RegType<T>* GetMethodTable();
template<> RegType<Unit>* GetMethodTable<Unit>();
template<> RegType<Item>* GetMethodTable<Item>();
template<> RegType<GameObject>* GetMethodTable<GameObject>();
template<> RegType<WorldPacket>* GetMethodTable<WorldPacket>();
template<> RegType<TaxiPath>* GetMethodTable<TaxiPath>();
template<> RegType<Spell>* GetMethodTable<Spell>();
template<> RegType<Field>* GetMethodTable<Field>();
template<> RegType<QueryResult>* GetMethodTable<QueryResult>();

void report(lua_State * L)
{
	uint32 count = 20;
	const char * msg= lua_tostring(L,-1);
	while(msg && count > 0)
	{
		lua_pop(L,-1);
		printf("\t%s\n", msg);
		msg=lua_tostring(L,-1);
		count--;
	}
}

void LuaEngine::ScriptLoadDir(char* Dirname, LUALoadScripts *pak)
{
	#ifdef WIN32
		Log.Success("LuaEngine", "Scanning Directory %s", Dirname);
		HANDLE hFile;
		WIN32_FIND_DATA FindData;
		memset(&FindData,0,sizeof(FindData));

		char SearchName[MAX_PATH];
	        
		strcpy(SearchName,Dirname);
		strcat(SearchName,"\\*.*");

		hFile = FindFirstFile(SearchName,&FindData);
		FindNextFile(hFile, &FindData);
	    
		while( FindNextFile(hFile, &FindData) )
		{
			if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) //Credits for this 'if' go to Cebernic from ArcScripts Team. Thanks, you saved me some work ;-)
			{
				strcpy(SearchName,Dirname);
				strcat(SearchName,"\\");
				strcat(SearchName,FindData.cFileName);
				ScriptLoadDir(SearchName, pak);
			}
			else
			{
						string fname = Dirname;
						fname += "\\";
						fname += FindData.cFileName;

			  		int len = strlen(fname.c_str());
					  int i=0;
					  char ext[MAX_PATH];
					  
					  while(len > 0)
					  {  
					   ext[i++] = fname[--len];
					   if(fname[len] == '.')
		  		   break;
	  			  }
	  			  ext[i++] = '\0';
	  			  if ( !_stricmp(ext,"aul.") ) pak->luaFiles.insert(fname);
			}
		}
	  FindClose(hFile);
	#else
		char *pch=strrchr(Dirname,'/');
		if (strcmp(Dirname, "..")==0 || strcmp(Dirname, ".")==0) return; //Against Endless-Loop
		if (pch != NULL && (strcmp(pch, "/..")==0 || strcmp(pch, "/.")==0 || strcmp(pch, "/.svn")==0)) return;
		struct dirent ** list;
		int filecount = scandir(Dirname, &list, 0, 0);

		if(filecount <= 0 || !list)
			return;

		struct stat attributes;
		bool err;
		Log.Success("LuaEngine", "Scanning Directory %s", Dirname);
		while(filecount--)
		{
			char dottedrelpath[200];
			sprintf(dottedrelpath, "%s/%s", Dirname, list[filecount]->d_name);
			if(stat(dottedrelpath, &attributes) == -1) {
				err=true;
				Log.Error("LuaEngine","Error opening %s: %s\n", dottedrelpath, strerror(errno));
			} else err=false;

			if (!err && S_ISDIR(attributes.st_mode))
			{
				ScriptLoadDir((char *)dottedrelpath, pak); //Subdirectory
			} else {
				char* ext = strrchr(list[filecount]->d_name, '.');
				if(ext != NULL && !strcmp(ext, ".lua"))
				{
					pak->luaFiles.insert(dottedrelpath);
				}
			}

			free(list[filecount]);
		}
		free(list);
	#endif
}

void LuaEngine::LoadScripts()
{
	LUALoadScripts rtn;
	Log.Notice("LuaEngine", "Scanning Script-Directories...");
	ScriptLoadDir((char*)"scripts", &rtn);

	unsigned int cnt_uncomp=0;

	luaL_openlibs(lu);
	RegisterCoreFunctions();
	Log.Notice("LuaEngine", "Loading Scripts...");

	char filename[200];

	for(set<string>::iterator itr = rtn.luaFiles.begin(); itr != rtn.luaFiles.end(); ++itr)
	{ 
		strcpy(filename, itr->c_str());
		if(luaL_loadfile(lu, filename) != 0)
		{
#ifdef WIN32
			Log.Error("LuaEngine", "loading %s failed.(could not load)", itr->c_str());
			SetConsoleTextAttribute(stdout_handle, (WORD)TRED);
#else
			Log.Notice("LuaEngine", "\033[22;31m loading %s failed.(could not load)", itr->c_str());
			printf("\033[22;31m");
#endif
			report(lu);
#ifdef WIN32
			SetConsoleTextAttribute(stdout_handle, (WORD)TWHITE);
#else
			printf("\033[01;37m");
#endif
		}
		else
		{
			if(lua_pcall(lu, 0, 0, 0) != 0)
			{
#ifdef WIN32
				Log.Error("LuaEngine", "%s failed.(could not run)", itr->c_str());
				SetConsoleTextAttribute(stdout_handle, (WORD)TRED);
#else
				Log.Notice("LuaEngine", "\033[22;31m %s failed.(could not run)", itr->c_str());
				printf("\033[22;31m");

#endif

				report(lu);
#ifdef WIN32
				SetConsoleTextAttribute(stdout_handle, (WORD)TWHITE);
#else
				printf("\033[01;37m");
#endif
			}
			else
					Log.Debug("LuaEngine", "loaded %s.", itr->c_str());
		}
		cnt_uncomp++;
	}
	Log.Notice("LuaEngine","Loaded %u Lua scripts.", cnt_uncomp);
}


/*******************************************************************************
	FUNCTION CALL METHODS
*******************************************************************************/

bool LuaEngine::BeginCall(const char * func) 
{
	string sFuncName = string(func);
	char * copy = strdup(func);
	char * token = strtok(copy,".:");
	bool colon = false;
	if (strpbrk(func,".:") == NULL )
		lua_getglobal(lu,func);
	else
	{
		lua_getglobal(lu, "_G"); //start out with the global table.
		int top = 1;
		while (token != NULL)
		{
			lua_getfield(lu, -1, token); //get the (hopefully) table/func
			if ((int)sFuncName.find(token)-1 > 0) //if it isn't the first token
			{
				if (sFuncName.at(sFuncName.find(token)-1) == '.') //if it was a .
					colon = false;
				else if (sFuncName.at(sFuncName.find(token)-1) == ':')
					colon = true;
			}
			else //if it IS the first token, we're OK to remove the "_G" from the stack
				colon = false;

			if (lua_isfunction(lu,-1) && !lua_iscfunction(lu,-1)) //if it's a Lua function
			{
				lua_replace(lu,top);
				if (colon)
				{
					lua_pushvalue(lu, -1); //make the table the first arg
					lua_replace(lu,top+1);
					lua_settop(lu,top+1);
				}
				else
					lua_settop(lu,top);
				break; //we don't need anything else
			}
			else if(lua_istable(lu,-1) )
				token = strtok(NULL,".:");
		}
	}
	return colon;
}
bool LuaEngine::ExecuteCall(uint8 params, uint8 res)
{
	bool ret = true;
	if(lua_pcall(lu,params,res,0) )
	{
		report(lu);
		ret = false;
	}
	return ret;
}
void LuaEngine::EndCall(uint8 res) 
{
	for(int i = res; i > 0; i--)
	{
		if(!lua_isnone(lu,res))
			lua_remove(lu,res);
	}
}
/*******************************************************************************
	END FUNCTION CALL METHODS
*******************************************************************************/

/******************************************************************************
	PUSH METHODS
******************************************************************************/

void LuaEngine::PUSH_UNIT(Object * unit, lua_State * L) 
{
	Unit * pUnit = NULL;
	if(unit != NULL && unit->IsUnit() ) 
		pUnit = TO_UNIT(unit);
	if(L == NULL)
		ArcLuna<Unit>::push(lu,pUnit);
	else
		ArcLuna<Unit>::push(L,pUnit);
}
void LuaEngine::PUSH_GO(Object *go, lua_State *L)
{
	GameObject * pGo = NULL;
	if(go != NULL && go->IsGameObject() )
		pGo = static_cast<GameObject*>(go);
	if(L == NULL)
		ArcLuna<GameObject>::push(lu,pGo);
	else
		ArcLuna<GameObject>::push(L,pGo);
}
void LuaEngine::PUSH_ITEM(Object * item, lua_State *L)
{
	Item * pItem = NULL;
	if(item != NULL && (item->GetTypeId() == TYPEID_ITEM || item->GetTypeId() == TYPEID_CONTAINER))
		pItem = static_cast<Item*>(item);
	if(L == NULL)
		ArcLuna<Item>::push(lu,pItem);
	else
		ArcLuna<Item>::push(L,pItem);
}
void LuaEngine::PUSH_GUID(uint64 guid, lua_State * L) 
{
	if(L == NULL)
		GUID_MGR::push(lu,guid);
	else
		GUID_MGR::push(L,guid);
}
void LuaEngine::PUSH_PACKET(WorldPacket * pack, lua_State * L) 
{
	if(L == NULL)
		ArcLuna<WorldPacket>::push(lu,pack,true);
	else
		ArcLuna<WorldPacket>::push(L,pack,true);
}
void LuaEngine::PUSH_TAXIPATH(TaxiPath * tp, lua_State * L) 
{
	if(L == NULL)
		ArcLuna<TaxiPath>::push(lu,tp,true);
	else
		ArcLuna<TaxiPath>::push(L,tp,true);
}
void LuaEngine::PUSH_SPELL(Spell * sp, lua_State * L) 
{
	if(L == NULL)
		ArcLuna<Spell>::push(lu,sp);
	else
		ArcLuna<Spell>::push(L,sp);
}
void LuaEngine::PUSH_SQLFIELD(Field *field, lua_State *L)
{
	if(L == NULL)
		ArcLuna<Field>::push(lu,field);
	else
		ArcLuna<Field>::push(L,field);
}
void LuaEngine::PUSH_SQLRESULT(QueryResult * res, lua_State * L)
{
	if(L == NULL)
		ArcLuna<QueryResult>::push(lu,res,true);
	else
		ArcLuna<QueryResult>::push(L,res,true);
}

/*******************************************************************************
	END PUSH METHODS
*******************************************************************************/

void LuaEngine::HyperCallFunction(const char * FuncName, int ref) //hyper as in hypersniper :3
{
	//m_Lock.Acquire();
	int top = lua_gettop(lu);
	int args = 0;
	string sFuncName = string(FuncName); //for convenience of string funcs
	char * copy = strdup(FuncName);
	char * token = strtok(copy, ".:"); //we should strtok on the copy
	bool colon = false; //whether we should keep or remove the previous table
	if (strpbrk(FuncName,".:") == NULL)
		lua_getglobal(lu,FuncName);
	else
	{
		lua_getglobal(lu, "_G"); //start out with the global table.
		while (token != NULL)
		{
			lua_getfield(lu, -1, token); //get the (hopefully) table/func
			if ((int)sFuncName.find(token)-1 > 0) //if it isn't the first token
			{
				if (sFuncName.at(sFuncName.find(token)-1) == '.') //if it was a .
					colon = false;
				else if (sFuncName.at(sFuncName.find(token)-1) == ':')
					colon = true;
			}
			else //if it IS the first token, we're OK to remove the "_G" from the stack
				colon = false;

			if (lua_isfunction(lu,-1) && !lua_iscfunction(lu,-1)) //if it's a Lua function
			{
				if (colon)
				{
					lua_pushvalue(lu, -2); //make the table the first arg
					lua_remove(lu, -3); //remove the thing we copied from (just to keep stack nice)
					++args;
				}
				else
				{
					lua_remove(lu, -2);
				}
				break; //we don't need anything else
			}
			else if (lua_istable(lu,-1))
			{
					token = strtok(NULL, ".:");
			}
		}
	}
	lua_rawgeti(lu, LUA_REGISTRYINDEX, ref);
	lua_State * M = lua_tothread(lu, -1); //repeats, args
	int thread = lua_gettop(lu);
	int repeats = luaL_checkinteger(M, 1); //repeats, args
	int nargs = lua_gettop(M)-1;
	if (nargs != 0) //if we HAVE args...
	{
		for (int i = 2; i <= nargs+1; i++)
		{
			lua_pushvalue(M,i);
		}
		lua_xmove(M, lu, nargs);
	}
	if (--repeats == 0) //free stuff, then
	{
		free((void*)FuncName);
		luaL_unref(lu, LUA_REGISTRYINDEX, ref);
	}
	else
	{
		lua_remove(M, 1); //args
		lua_pushinteger(M, repeats); //args, repeats
		lua_insert(M, 1); //repeats, args
	}
	lua_remove(lu, thread); //now we can remove the thread object
	int r = lua_pcall(lu,nargs+args,0,0);
	if (r)
		report(lu);

	free((void*)copy);
	lua_settop(lu,top);
	//m_Lock.Release();
}

static int RegisterServerHook(lua_State * L);
static int RegisterUnitEvent(lua_State * L);
static int RegisterQuestEvent(lua_State * L);
static int RegisterGameObjectEvent(lua_State * L);
static int RegisterUnitGossipEvent(lua_State * L);
static int RegisterItemGossipEvent(lua_State * L);
static int RegisterGOGossipEvent(lua_State * L);
static int SuspendLuaThread(lua_State * L);
static int RegisterTimedEvent(lua_State * L);
static int RegisterDummySpell(lua_State * L);
void RegisterGlobalFunctions(lua_State*);

void LuaEngine::RegisterCoreFunctions()
{
	lua_register(lu,"RegisterUnitEvent",RegisterUnitEvent);
	lua_register(lu,"RegisterGameObjectEvent",RegisterGameObjectEvent);
	lua_register(lu,"RegisterQuestEvent",RegisterQuestEvent);
	lua_register(lu,"RegisterUnitGossipEvent",RegisterUnitGossipEvent);
	lua_register(lu,"RegisterItemGossipEvent",RegisterItemGossipEvent);
	lua_register(lu,"RegisterGOGossipEvent",RegisterGOGossipEvent);
	lua_register(lu,"RegisterServerHook",RegisterServerHook);
	lua_register(lu,"SuspendThread",&SuspendLuaThread);
	lua_register(lu,"RegisterTimedEvent",&RegisterTimedEvent);
	lua_register(lu,"RegisterDummySpell",&RegisterDummySpell);

	RegisterGlobalFunctions(lu);

	ArcLuna<Unit>::Register(lu);
	ArcLuna<Item>::Register(lu);
	ArcLuna<GameObject>::Register(lu);
	ArcLuna<WorldPacket>::Register(lu);
	ArcLuna<TaxiPath>::Register(lu);
	ArcLuna<Spell>::Register(lu);
	ArcLuna<Field>::Register(lu);
	ArcLuna<QueryResult>::Register(lu);

	GUID_MGR::Register(lu);

	//set the suspendluathread a coroutine function
	lua_getglobal(lu,"coroutine");
	if(lua_istable(lu,-1) )
	{
		lua_pushcfunction(lu,SuspendLuaThread);
		lua_setfield(lu,-2,"wait");
		lua_pushcfunction(lu,SuspendLuaThread);
		lua_setfield(lu,-2,"WAIT");
	}
	lua_pop(lu,1);
}

static int RegisterServerHook(lua_State * L)
{
	uint32 ev = luaL_checkint(L, 1);
	const char * str = luaL_checkstring(L, 2);

	if(!ev || !str)
		return 0;
	//Lets validate the string here so the scripter doesn't have to wait until the script event fires just to be slapped with an error
	//such as a typo or non function passed in.
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy,".:");
	if(strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_SERVHOOK,0,ev,str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterServerHook failed! %s is not a valid Lua function.\n",str);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_SERVHOOK,0,ev,str);
				break;
			}
			else if(lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterServerHook failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int RegisterDummySpell(lua_State * L)
{
	uint32 entry = luaL_checkint(L, 1);
	const char * str = luaL_checkstring(L, 2);
	
	if(!entry || !str)
		return 0;

	if (m_luaDummySpells.find(entry) != m_luaDummySpells.end())
	{
		Log.Color(TRED);
		luaL_error(L,"LuaEngineMgr : RegisterDummySpell failed! Spell %d already has a registered Lua function!",entry);
		Log.Color(TWHITE);
	}
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy, ".:");
	if (strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if (lua_isfunction(L,-1) && !lua_iscfunction(L,-1))
			sLuaMgr.RegisterEvent(REGTYPE_DUMMYSPELL,entry,1,str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterDummySpell failed! %s is not a valid Lua function. \n",str);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while (token != NULL)
		{
			lua_getfield(L,-1,token);
			if (lua_isfunction(L,-1) && !lua_iscfunction(L,-1))
			{
				sLuaMgr.RegisterEvent(REGTYPE_DUMMYSPELL,entry,1,str);
				break;
			}
			else if (lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterDummySpell failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
				break;
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int RegisterUnitEvent(lua_State * L)
{
	int entry = luaL_checkint(L, 1);
	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);

	if(!entry || !ev || !str)
		return 0;
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy, ".:");
	if( strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_UNIT,entry,ev,str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterUnitEvent failed! %s is not a valid Lua function. \n",str);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_UNIT,entry,ev,str);
				break;
			}
			else if(lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterUnitEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
				break;
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int RegisterQuestEvent(lua_State * L)
{
	int entry = luaL_checkint(L, 1);
	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);

	if(!entry || !ev || !str)
		return 0;
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy, ".:");
	if(strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_QUEST,entry,ev,str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterQuestEvent failed! %s is not a valid Lua function. \n",copy);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_QUEST,entry,ev,str);
				break;
			}
			else if(lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterQuestEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
				break;
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int RegisterGameObjectEvent(lua_State * L)
{
	int entry = luaL_checkint(L, 1);
	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);

	if(!entry || !ev || !str)
		return 0;

	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy,".:");
	if( strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_GO,entry,ev,str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterGameObjectEvent failed! %s is not a valid Lua function. \n",copy);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_GO,entry,ev,str);
				break;
			}
			else if(lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterGameObjectEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
				break;
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int RegisterUnitGossipEvent(lua_State * L)
{
	int entry = luaL_checkint(L, 1);
	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);
	if(!entry || !ev || !str)
		return 0;
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token = strtok(copy,".:");
	if( strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			 sLuaMgr.RegisterEvent(REGTYPE_UNIT_GOSSIP,entry, ev, str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterUnitGossipEvent failed! %s is not a valid Lua function. \n",copy);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_UNIT_GOSSIP,entry, ev, str);
				break;
			}
			else if(lua_istable(L,-1) )
			{
				token = strtok(NULL,".:");
				continue;
			}
			else
			{
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterUnitGossipEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
				break;
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}
static int RegisterItemGossipEvent(lua_State * L)
 {
 	int entry = luaL_checkint(L, 1);
 	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);

 	if(!entry || !ev || !str)
 		return 0;

	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token  = strtok(copy,".:");

	if( strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_ITEM_GOSSIP,entry, ev, str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterItemGossipEvent failed! %s is not a valid Lua function. \n",copy);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_ITEM_GOSSIP,entry, ev, str);
				break;
			}
			else if(lua_istable(L,-1) ) {
				token = strtok(NULL,".:");
				continue;
			}
			else {
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterItemGossipEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
 	return 0;
 }
static int RegisterGOGossipEvent(lua_State * L)
{
	int entry = luaL_checkint(L, 1);
	int ev = luaL_checkint(L, 2);
	const char * str = luaL_checkstring(L, 3);

	if(!entry || !ev || !str)
		return 0;
	int top = lua_gettop(L);
	char * copy = strdup(str);
	char * token  = strtok(copy,".:");
	if( strpbrk(str,".:") == NULL)
	{
		lua_getglobal(L,str);
		if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			sLuaMgr.RegisterEvent(REGTYPE_GO_GOSSIP,entry, ev, str);
		else
		{
			Log.Color(TRED);
			luaL_error(L,"LuaEngineMgr : RegisterGOGossipEvent failed! %s is not a valid Lua function. \n",copy);
			Log.Color(TWHITE);
		}
	}
	else
	{
		lua_getglobal(L,"_G");
		while(token != NULL)
		{
			lua_getfield(L,-1,token);
			if(lua_isfunction(L,-1) && !lua_iscfunction(L,-1) )
			{
				sLuaMgr.RegisterEvent(REGTYPE_GO_GOSSIP,entry, ev, str);
				break;
			}
			else if(lua_istable(L,-1)) {
				token = strtok(NULL,".:");
				continue;
			}
			else {
				Log.Color(TRED);
				luaL_error(L,"LuaEngineMgr : RegisterGOGossipEvent failed! %s is not a valid Lua function. \n",token);
				Log.Color(TWHITE);
			}
		}
	}
	free((void*)copy);
	lua_settop(L,top);
	return 0;
}

static int SuspendLuaThread(lua_State * L) {
	lua_State * thread = (lua_isthread(L,1)) ? lua_tothread(L,1) : NULL;
	if(thread == NULL) {
		return luaL_error(L,"LuaEngineMgr","SuspendLuaThread expected Lua coroutine, got NULL. \n");
	}
	int waitime = luaL_checkinteger(L,2);
	if(waitime <= 0) {
		return luaL_error(L,"LuaEngineMgr","SuspendLuaThread expected timer > 0 instead got (%d) \n",waitime);
	}
	lua_pushvalue(L,1);
	int ref = luaL_ref(L,LUA_REGISTRYINDEX);
	if(ref == LUA_REFNIL || ref == LUA_NOREF)
		return luaL_error(L,"Error in SuspendLuaThread! Failed to create a valid reference.");
	TimedEvent * evt = TimedEvent::Allocate(thread,new CallbackP1<LuaEngine,int>(&g_luaMgr,&LuaEngine::ResumeLuaThread,ref),0,waitime,1);
	sWorld.event_AddEvent(evt);
	lua_remove(L,1); // remove thread object
	lua_remove(L,1); // remove timer.
	//All that remains now are the extra arguments passed to this function.
	lua_xmove(L,thread,lua_gettop(L));
	g_luaMgr.getThreadRefs().insert(ref);
	return lua_yield(thread,lua_gettop(L));
}

static int RegisterTimedEvent(lua_State * L) //in this case, L == lu
{
	const char * funcName = strdup(luaL_checkstring(L,1));
	int delay = luaL_checkint(L,2);
	int repeats = luaL_checkint(L,3);
	if (!delay || !repeats || !funcName)
		return 0;
	lua_remove(L, 1); 
	lua_remove(L, 1);//repeats, args
	lua_State * thread = lua_newthread(L); //repeats, args, thread
	lua_insert(L,1); //thread, repeats, args
	lua_xmove(L,thread,lua_gettop(L)-1); //thread
	int ref = luaL_ref(L, LUA_REGISTRYINDEX); //empty
	TimedEvent *te = TimedEvent::Allocate(&sLuaMgr, new CallbackP2<LuaEngine, const char*, int>(&sLuaMgr, &LuaEngine::HyperCallFunction, funcName, ref), 0, delay, repeats);
	sWorld.event_AddEvent(te);
	return 0;
}



//all of these run similarly, they execute OnServerHook for all the functions in their respective event's list.
bool LuaHookOnNewCharacter(uint32 Race, uint32 Class, WorldSession * Session, const char * Name)
{
	GET_LOCK
	bool result = true;
	for(vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_NEW_CHARACTER].begin(); itr != EventAsToFuncName[SERVER_HOOK_NEW_CHARACTER].end(); ++itr) {
		uint8 args = 0;
		if(sLuaMgr.BeginCall(itr->c_str()) )
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_NEW_CHARACTER);
		sLuaMgr.PUSH_STRING(Name);
		sLuaMgr.PUSH_UINT(Race);
		sLuaMgr.PUSH_UINT(Class);
		args+=4;
		if(sLuaMgr.ExecuteCall(args,1) )
		{
			lua_State * L = sLuaMgr.getluState();
			if(!lua_isnoneornil(L,1) && !lua_toboolean(L,1) )
				result = false;
			sLuaMgr.EndCall(1);
		}
	}
	RELEASE_LOCK
	return result;
}

void LuaHookOnKillPlayer(Player * pPlayer, Player * pVictim)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_KILL_PLAYER].begin(); itr != EventAsToFuncName[SERVER_HOOK_KILL_PLAYER].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_KILL_PLAYER);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pVictim);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnFirstEnterWorld(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_FIRST_ENTER_WORLD].begin(); itr != EventAsToFuncName[SERVER_HOOK_FIRST_ENTER_WORLD].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_FIRST_ENTER_WORLD);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnEnterWorld(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ENTER_WORLD].begin(); itr != EventAsToFuncName[SERVER_HOOK_ENTER_WORLD].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ENTER_WORLD);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnGuildJoin(Player * pPlayer, Guild * pGuild)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_GUILD_JOIN].begin(); itr != EventAsToFuncName[SERVER_HOOK_GUILD_JOIN].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_GUILD_JOIN);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_STRING(pGuild->GetGuildName());
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnDeath(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_DEATH].begin(); itr != EventAsToFuncName[SERVER_HOOK_DEATH].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_DEATH);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

bool LuaHookOnRepop(Player * pPlayer)
{
	GET_LOCK
	bool result = true;
	for(vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_REPOP].begin(); itr != EventAsToFuncName[SERVER_HOOK_REPOP].end(); ++itr) {
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_REPOP);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		if (sLuaMgr.ExecuteCall(args,1)) {
			lua_State * L = sLuaMgr.getluState();
			if(!lua_isnoneornil(L,1) && !lua_toboolean(L,1) )
				result = false;
			sLuaMgr.EndCall(1);
		}
	}
	RELEASE_LOCK
	return result;
}

void LuaHookOnEmote(Player * pPlayer, uint32 Emote, Unit * pUnit)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_EMOTE].begin(); itr != EventAsToFuncName[SERVER_HOOK_EMOTE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_EMOTE);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pUnit);
		sLuaMgr.PUSH_UINT(Emote);
		args+=4;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnEnterCombat(Player * pPlayer, Unit * pTarget)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ENTER_COMBAT].begin(); itr != EventAsToFuncName[SERVER_HOOK_ENTER_COMBAT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ENTER_COMBAT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pTarget);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

bool LuaHookOnCastSpell(Player * pPlayer, SpellEntry* pSpell)
{
	GET_LOCK
	bool result = true;
	for(vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_CAST_SPELL].begin(); itr != EventAsToFuncName[SERVER_HOOK_CAST_SPELL].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_CAST_SPELL);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(pSpell->Id);
		args+=3;
		if( sLuaMgr.ExecuteCall(args,1) ) {
			lua_State * L = sLuaMgr.getluState();
			if(!lua_isnoneornil(L,1) && !lua_toboolean(L,1) )
				result = false;
			sLuaMgr.EndCall(1);
		}
	}
	RELEASE_LOCK
	return result;
}

void LuaHookOnTick()
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_TICK].begin(); itr != EventAsToFuncName[SERVER_HOOK_TICK].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

bool LuaHookOnLogoutRequest(Player * pPlayer)
{
	GET_LOCK
	bool result = true;
	for(vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_LOGOUT_REQUEST].begin(); itr != EventAsToFuncName[SERVER_HOOK_LOGOUT_REQUEST].end(); itr++)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_LOGOUT_REQUEST);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		if(sLuaMgr.ExecuteCall(args,1) )
		{
			lua_State * L = sLuaMgr.getluState();
			if(!lua_isnoneornil(L,1) && !lua_toboolean(L,1) )
				result = false;
			sLuaMgr.EndCall(1);
		}
	}
	RELEASE_LOCK
	return result;
}

void LuaHookOnLogout(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_LOGOUT].begin(); itr != EventAsToFuncName[SERVER_HOOK_LOGOUT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_LOGOUT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnQuestAccept(Player * pPlayer, Quest * pQuest, Object * pQuestGiver)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_QUEST_ACCEPT].begin(); itr != EventAsToFuncName[SERVER_HOOK_QUEST_ACCEPT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_QUEST_ACCEPT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(pQuest->id);
		if(pQuestGiver->IsUnit() )
			sLuaMgr.PUSH_UNIT(pQuestGiver);
		else if(pQuestGiver->IsGameObject() )
			sLuaMgr.PUSH_GO(pQuestGiver);
		else if(pQuestGiver->GetTypeId() == TYPEID_ITEM)
			sLuaMgr.PUSH_ITEM(pQuestGiver);
		else
			sLuaMgr.PUSH_NIL();
		args+=4;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnZone(Player * pPlayer, uint32 Zone)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ZONE].begin(); itr != EventAsToFuncName[SERVER_HOOK_ZONE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ZONE);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(Zone);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

bool LuaHookOnChat(Player * pPlayer, uint32 Type, uint32 Lang, const char * Message, const char * Misc)
{
	GET_LOCK
	bool result = true;
	for(vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_CHAT].begin(); itr != EventAsToFuncName[SERVER_HOOK_CHAT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_CHAT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_STRING(Message);
		sLuaMgr.PUSH_UINT(Type);
		sLuaMgr.PUSH_UINT(Lang);
		sLuaMgr.PUSH_STRING(Misc);
		args+=6;
		if( sLuaMgr.ExecuteCall(args,1)) {
			lua_State * L = sLuaMgr.getluState();
			if(!lua_isnoneornil(L,1) && !lua_toboolean(L,1) )
				result = false;
			sLuaMgr.EndCall(1);
		}
	}
	RELEASE_LOCK
	return result;
}

void LuaHookOnLoot(Player * pPlayer, Unit * pTarget, uint32 Money, uint32 ItemId)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_LOOT].begin(); itr != EventAsToFuncName[SERVER_HOOK_LOOT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_LOOT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pTarget);
		sLuaMgr.PUSH_UINT(Money);
		sLuaMgr.PUSH_UINT(ItemId);
		args+=5;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnGuildCreate(Player * pLeader, Guild * pGuild)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_GUILD_CREATE].begin(); itr != EventAsToFuncName[SERVER_HOOK_GUILD_CREATE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_GUILD_CREATE);
		sLuaMgr.PUSH_UNIT(pLeader);
		sLuaMgr.PUSH_STRING(pGuild->GetGuildName());
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnEnterWorld2(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ENTER_WORLD_2].begin(); itr != EventAsToFuncName[SERVER_HOOK_ENTER_WORLD_2].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ENTER_WORLD_2);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnCharacterCreate(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_CHARACTER_CREATE].begin(); itr != EventAsToFuncName[SERVER_HOOK_CHARACTER_CREATE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_CHARACTER_CREATE);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnQuestCancelled(Player * pPlayer, Quest * pQuest)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_QUEST_CANCELLED].begin(); itr != EventAsToFuncName[SERVER_HOOK_QUEST_CANCELLED].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_QUEST_CANCELLED);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(pQuest->id);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnQuestFinished(Player * pPlayer, Quest * pQuest, Object * pQuestGiver)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_QUEST_FINISHED].begin(); itr != EventAsToFuncName[SERVER_HOOK_QUEST_FINISHED].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_QUEST_FINISHED);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(pQuest->id);
		if(pQuestGiver->IsUnit() )
			sLuaMgr.PUSH_UNIT(pQuestGiver);
		else if(pQuestGiver->IsGameObject() )
			sLuaMgr.PUSH_GO(pQuestGiver);
		else if(pQuestGiver->GetTypeId() == TYPEID_ITEM)
			sLuaMgr.PUSH_ITEM(pQuestGiver);
		else
			sLuaMgr.PUSH_NIL();
		args+=4;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnHonorableKill(Player * pPlayer, Player * pKilled)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_HONORABLE_KILL].begin(); itr != EventAsToFuncName[SERVER_HOOK_HONORABLE_KILL].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_HONORABLE_KILL);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pKilled);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnArenaFinish(Player * pPlayer, ArenaTeam* pTeam, bool victory, bool rated)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ARENA_FINISH].begin(); itr != EventAsToFuncName[SERVER_HOOK_ARENA_FINISH].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ARENA_FINISH);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_STRING(pTeam->m_name.c_str());
		sLuaMgr.PUSH_BOOL(victory);
		sLuaMgr.PUSH_BOOL(rated);
		args+=5;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnObjectLoot(Player * pPlayer, Object * pTarget, uint32 Money, uint32 ItemId)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_OBJECTLOOT].begin(); itr != EventAsToFuncName[SERVER_HOOK_OBJECTLOOT].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_OBJECTLOOT);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UNIT(pTarget);
		sLuaMgr.PUSH_UINT(Money);
		sLuaMgr.PUSH_UINT(ItemId);
		args+=5;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnAreaTrigger(Player * pPlayer, uint32 areaTrigger)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_AREATRIGGER].begin(); itr != EventAsToFuncName[SERVER_HOOK_AREATRIGGER].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_AREATRIGGER);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(areaTrigger);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnPostLevelUp(Player * pPlayer)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_POST_LEVELUP].begin(); itr != EventAsToFuncName[SERVER_HOOK_POST_LEVELUP].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_POST_LEVELUP);
		sLuaMgr.PUSH_UNIT(pPlayer);
		args+=2;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnPreUnitDie(Unit *Killer, Unit *Victim)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_PRE_DIE].begin(); itr != EventAsToFuncName[SERVER_HOOK_PRE_DIE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_PRE_DIE);
		sLuaMgr.PUSH_UNIT(Killer);
		sLuaMgr.PUSH_UNIT(Victim);
		args+=3;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

void LuaHookOnAdvanceSkillLine(Player * pPlayer, uint32 SkillLine, uint32 Current)
{
	GET_LOCK
	for(std::vector<string>::iterator itr = EventAsToFuncName[SERVER_HOOK_ADVANCE_SKILLLINE].begin(); itr != EventAsToFuncName[SERVER_HOOK_ADVANCE_SKILLLINE].end(); ++itr)
	{
		uint8 args = 0;
		if( sLuaMgr.BeginCall(itr->c_str()))
			++args;
		sLuaMgr.PUSH_INT(SERVER_HOOK_ADVANCE_SKILLLINE);
		sLuaMgr.PUSH_UNIT(pPlayer);
		sLuaMgr.PUSH_UINT(SkillLine);
		sLuaMgr.PUSH_UINT(Current);
		args+=4;
		sLuaMgr.ExecuteCall(args);
	}
	RELEASE_LOCK
}

bool LuaOnDummySpell(uint32 effectIndex, Spell * pSpell)
{
	GET_LOCK
	uint8 args = 0;
	if ( sLuaMgr.BeginCall(m_luaDummySpells[pSpell->GetProto()->Id]) )
		++args;
	sLuaMgr.PUSH_UINT(effectIndex);
	sLuaMgr.PUSH_SPELL(pSpell);
	args += 2;
	sLuaMgr.ExecuteCall(args);
	RELEASE_LOCK
	return true;
}

class LuaCreature : public CreatureAIScript
{
public:
	LuaCreature(Creature* creature) : CreatureAIScript(creature) {};
	~LuaCreature()
	{
		typedef std::multimap<uint32,LuaCreature*> CMAP;
		CMAP & cMap = sLuaMgr.getLuCreatureMap();
		CMAP::iterator itr = cMap.find(_unit->GetEntry());
		CMAP::iterator itend = cMap.upper_bound(_unit->GetEntry());
		CMAP::iterator it;
		for(;itr != cMap.end() && itr != itend;)
		{
			it = itr++;
			if(it->second != NULL && it->second == this)
				cMap.erase(it);
		}
	}
	ARCEMU_INLINE void SetUnit(Creature * ncrc) { _unit = ncrc; }
	void OnCombatStart(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_ENTER_COMBAT] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_ENTER_COMBAT]) )
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_ENTER_COMBAT);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}

	void OnCombatStop(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_LEAVE_COMBAT] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_LEAVE_COMBAT]))
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_LEAVE_COMBAT);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}

	void OnTargetDied(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_TARGET_DIED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_DIED]) )
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_TARGET_DIED);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}

	void OnDied(Unit *mKiller)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_DIED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DIED]) )
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_DIED);
			sLuaMgr.PUSH_UNIT(mKiller);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnTargetParried(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_TARGET_PARRIED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_PARRIED]))
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_TARGET_PARRIED);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnTargetDodged(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_TARGET_DODGED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_DODGED]))
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_TARGET_DODGED);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnTargetBlocked(Unit* mTarget, int32 iAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_TARGET_BLOCKED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_BLOCKED]) )
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_TARGET_BLOCKED);
			sLuaMgr.PUSH_UNIT(mTarget);
			sLuaMgr.PUSH_INT(iAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnTargetCritHit(Unit* mTarget, float fAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_TARGET_CRIT_HIT] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_TARGET_CRIT_HIT]) )
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_TARGET_CRIT_HIT);
			sLuaMgr.PUSH_UNIT(mTarget);
			sLuaMgr.PUSH_FLOAT(fAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnParried(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_PARRY] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_PARRY]))
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_PARRY);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnDodged(Unit* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_DODGED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DODGED]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_DODGED);
			sLuaMgr.PUSH_UNIT(mTarget);
			args+=3;
			sLuaMgr.ExecuteCall(3);
		}
		RELEASE_LOCK
	}
	void OnBlocked(Unit* mTarget, int32 iAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_BLOCKED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_BLOCKED]) )
				args++;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_BLOCKED);
			sLuaMgr.PUSH_UNIT(mTarget);
			sLuaMgr.PUSH_INT(iAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnCritHit(Unit* mTarget, float fAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_CRIT_HIT] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_CRIT_HIT]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_CRIT_HIT);
			sLuaMgr.PUSH_UNIT(mTarget);
			sLuaMgr.PUSH_FLOAT(fAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnHit(Unit* mTarget, float fAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_HIT] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_HIT]) )
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_HIT);
			sLuaMgr.PUSH_UNIT(mTarget);
			sLuaMgr.PUSH_FLOAT(fAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnAssistTargetDied(Unit* mAssistTarget)
	{
		
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_ASSIST_TARGET_DIED] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_ASSIST_TARGET_DIED]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_ASSIST_TARGET_DIED);
			sLuaMgr.PUSH_UNIT(mAssistTarget);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnFear(Unit* mFeared, uint32 iSpellId)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_FEAR] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_FEAR]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_FEAR);
			sLuaMgr.PUSH_UNIT(mFeared);
			sLuaMgr.PUSH_UINT(iSpellId);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnFlee(Unit* mFlee)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_FLEE] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_FLEE]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_FLEE);
			sLuaMgr.PUSH_UNIT(mFlee);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnCallForHelp()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_CALL_FOR_HELP] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_CALL_FOR_HELP]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_CALL_FOR_HELP);
			args+=2;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnLoad()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_LOAD] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_LOAD]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_LOAD);
			args+=2;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnReachWP(uint32 iWaypointId, bool bForwards)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_REACH_WP] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_REACH_WP]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_REACH_WP);
			sLuaMgr.PUSH_UINT(iWaypointId);
			sLuaMgr.PUSH_BOOL(bForwards);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnLootTaken(Player* pPlayer, ItemPrototype *pItemPrototype)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_LOOT_TAKEN] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_LOOT_TAKEN]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_LOOT_TAKEN);
			sLuaMgr.PUSH_UNIT(pPlayer);
			sLuaMgr.PUSH_UINT(pItemPrototype->ItemId);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void AIUpdate()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_AIUPDATE] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_AIUPDATE]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_AIUPDATE);
			args+=2;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnEmote(Player * pPlayer, EmoteType Emote)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_EMOTE] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_EMOTE]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_EMOTE);
			sLuaMgr.PUSH_UNIT(pPlayer);
			sLuaMgr.PUSH_INT((int32)Emote);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnDamageTaken(Unit* mAttacker, float fAmount)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[CREATURE_EVENT_ON_DAMAGE_TAKEN] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[CREATURE_EVENT_ON_DAMAGE_TAKEN]))
				++args;
			sLuaMgr.PUSH_UNIT(_unit);
			sLuaMgr.PUSH_INT(CREATURE_EVENT_ON_DAMAGE_TAKEN);
			sLuaMgr.PUSH_UNIT(mAttacker);
			sLuaMgr.PUSH_FLOAT(fAmount);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void StringFunctionCall(const char * pFunction)
	{
		
		CHECK_BINDING_ACQUIRELOCK
		uint8 args = 0;
		if(sLuaMgr.BeginCall(pFunction))
			++args;
		sLuaMgr.PUSH_UNIT(_unit);
		++args;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	LuaUnitBinding * m_binding;
};

class LuaGameObject : public GameObjectAIScript
{
public:
	LuaGameObject(GameObject * go) : GameObjectAIScript(go) {}
	~LuaGameObject() {}
	ARCEMU_INLINE GameObject * getGO() { return _gameobject; }
	void OnCreate()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_ON_CREATE] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_CREATE]))
				++args;
			sLuaMgr.PUSH_GO(_gameobject);
			args++;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnSpawn()
	{
		
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_ON_SPAWN] != NULL)
		{
			uint8 args = 0;
			if( sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_SPAWN]))
				++args;
			sLuaMgr.PUSH_GO(_gameobject);
			++args;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnDespawn()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_ON_DESPAWN] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_DESPAWN]) )
				++args;
			sLuaMgr.PUSH_GO(_gameobject);
			++args;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnLootTaken(Player * pLooter, ItemPrototype *pItemInfo)
	{
		
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_ON_LOOT_TAKEN] != NULL)
		{
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_LOOT_TAKEN]))
				++args;
			sLuaMgr.PUSH_GO(_gameobject);
			sLuaMgr.PUSH_UINT(GAMEOBJECT_EVENT_ON_LOOT_TAKEN);
			sLuaMgr.PUSH_UNIT(pLooter);
			sLuaMgr.PUSH_UINT(pItemInfo->ItemId);
			args+=4;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void OnActivate(Player * pPlayer)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_ON_USE] != NULL)
		{
			uint8 args = 0;
			sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_ON_USE]);
			sLuaMgr.PUSH_GO(_gameobject);
			sLuaMgr.PUSH_UINT(GAMEOBJECT_EVENT_ON_USE);
			sLuaMgr.PUSH_UNIT(pPlayer);
			args+=3;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	
	void AIUpdate()
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[GAMEOBJECT_EVENT_AIUPDATE] != NULL)
		{
			uint8 args = 0;
			sLuaMgr.BeginCall(m_binding->Functions[GAMEOBJECT_EVENT_AIUPDATE]);
			sLuaMgr.PUSH_GO(_gameobject);
			++args;
			sLuaMgr.ExecuteCall(args);
		}
		RELEASE_LOCK
	}
	void Destroy ()
	{
		typedef std::multimap<uint32,LuaGameObject*> GMAP;
		GMAP & gMap = sLuaMgr.getLuGameObjectMap();
		GMAP::iterator itr = gMap.find(_gameobject->GetEntry());
		GMAP::iterator itend = gMap.upper_bound(_gameobject->GetEntry());
		GMAP::iterator it;
		//uint64 guid = _gameobject->GetGUID(); Unused?
		for(; itr != itend;)
		{
			it = itr++;
			if(it->second != NULL && it->second == this)
				gMap.erase(it);
		}
		delete this;
	}
	LuaGameObjectBinding * m_binding;
};

class LuaGossip : public GossipScript
{
public:
	LuaGossip() : GossipScript() {}
	~LuaGossip() {
		typedef HM_NAMESPACE::hash_map<uint32, LuaGossip*> MapType;
		MapType gMap;
		if(this->m_go_gossip_binding != NULL)
		{
			gMap = g_luaMgr.getGameObjectGossipInterfaceMap();
			for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
			{
				if(itr->second == this) {
					gMap.erase(itr);
					break;
				}
			}
		}
		else if(this->m_unit_gossip_binding != NULL)
		{
			gMap = g_luaMgr.getUnitGossipInterfaceMap();
			for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
			{
				if(itr->second == this)
				{
					gMap.erase(itr);
					break;
				}
			}
		}
		else if(this->m_item_gossip_binding != NULL)
		{
			gMap = g_luaMgr.getItemGossipInterfaceMap();
			for(MapType::iterator itr = gMap.begin(); itr != gMap.end(); ++itr)
			{
				if(itr->second == this)
				{
					gMap.erase(itr);
					break;
				}
			}
		}
	}

	void GossipHello(Object* pObject, Player* Plr, bool AutoSend)
	{
		GET_LOCK
		if(pObject->GetTypeId() == TYPEID_UNIT)
        {
			if(m_unit_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]))
				++args;
			sLuaMgr.PUSH_UNIT(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_TALK);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_BOOL(AutoSend);
			args+=4;
			sLuaMgr.ExecuteCall(args);
        }
        else if(pObject->GetTypeId() == TYPEID_ITEM)
        {
			if(m_item_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]))
				++args;
			sLuaMgr.PUSH_ITEM(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_TALK);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_BOOL(AutoSend);
			args+=4;
			sLuaMgr.ExecuteCall(args);
        }
		else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
        {
			if(m_go_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
            if(sLuaMgr.BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_TALK]))
				++args;
			sLuaMgr.PUSH_GO(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_TALK);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_BOOL(AutoSend);
			args+=4;
			sLuaMgr.ExecuteCall(args);
        }
		RELEASE_LOCK
	}

	void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode)
	{
		GET_LOCK
		if(pObject->GetTypeId() == TYPEID_UNIT)
        {
			if(m_unit_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]))
				++args;
			sLuaMgr.PUSH_UNIT(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_UINT(Id);
			sLuaMgr.PUSH_UINT(IntId);
			sLuaMgr.PUSH_STRING(EnteredCode);
			args+=6;
			sLuaMgr.ExecuteCall(args);
        }
        else if(pObject->GetTypeId() == TYPEID_ITEM)
        {
			if(m_item_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
           if( sLuaMgr.BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]))
			   ++args;
			sLuaMgr.PUSH_ITEM(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_UINT(Id);
			sLuaMgr.PUSH_UINT(IntId);
			sLuaMgr.PUSH_STRING(EnteredCode);
			args+=6;
			sLuaMgr.ExecuteCall(args);
        }
        else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
        {
			if(m_go_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
            if(sLuaMgr.BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_SELECT_OPTION]))
				++args;
			sLuaMgr.PUSH_GO(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_SELECT_OPTION);
			sLuaMgr.PUSH_UNIT(Plr);
			sLuaMgr.PUSH_UINT(Id);
			sLuaMgr.PUSH_UINT(IntId);
			sLuaMgr.PUSH_STRING(EnteredCode);
			args+=6;
			sLuaMgr.ExecuteCall(args);
        }
		RELEASE_LOCK
	}

	void GossipEnd(Object* pObject, Player* Plr)
	{
		GET_LOCK
		if(pObject->GetTypeId() == TYPEID_UNIT)
        {
			if(m_unit_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_unit_gossip_binding->Functions[GOSSIP_EVENT_ON_END]))
				++args;
			sLuaMgr.PUSH_UNIT(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_END);
			sLuaMgr.PUSH_UNIT(Plr);
			args+=3;
			sLuaMgr.ExecuteCall(args);
        }
        else if(pObject->GetTypeId() == TYPEID_ITEM)
        {
			if(m_item_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
			if(sLuaMgr.BeginCall(m_item_gossip_binding->Functions[GOSSIP_EVENT_ON_END]))
				++args;
			sLuaMgr.PUSH_ITEM(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_END);
			sLuaMgr.PUSH_UNIT(Plr);
			args+=3;
			sLuaMgr.ExecuteCall(args);
        }
        else if(pObject->GetTypeId() == TYPEID_GAMEOBJECT)
        {
			if(m_go_gossip_binding == NULL) { RELEASE_LOCK; return; }
			uint8 args = 0;
            if(sLuaMgr.BeginCall(m_go_gossip_binding->Functions[GOSSIP_EVENT_ON_END]))
				++args;
			sLuaMgr.PUSH_GO(pObject);
			sLuaMgr.PUSH_UINT(GOSSIP_EVENT_ON_END);
			sLuaMgr.PUSH_UNIT(Plr);
			args+=3;
			sLuaMgr.ExecuteCall(args);
        }
		RELEASE_LOCK
	}

	LuaUnitGossipBinding * m_unit_gossip_binding;
	LuaItemGossipBinding * m_item_gossip_binding;
    LuaGOGossipBinding * m_go_gossip_binding;
};

class LuaQuest : public QuestScript
{
public:
	LuaQuest() : QuestScript() {}
	~LuaQuest()
	{
		typedef HM_NAMESPACE::hash_map<uint32,LuaQuest*> QuestType;
		QuestType qMap = g_luaMgr.getLuQuestMap();
		for(QuestType::iterator itr = qMap.begin(); itr != qMap.end(); ++itr)
		{
			if(itr->second == this)
			{
				qMap.erase(itr);
				break;
			}
		}
	}

	void OnQuestStart(Player* mTarget, QuestLogEntry *qLogEntry)
	{
		
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_ACCEPT] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_ACCEPT]))
			args++;
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=2;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}

	void OnQuestComplete(Player* mTarget, QuestLogEntry *qLogEntry)
	{
		
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_COMPLETE] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_COMPLETE]))
			++args;
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=2;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	void OnQuestCancel(Player* mTarget)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_CANCEL] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_CANCEL]))
			++args;
		sLuaMgr.PUSH_UNIT(mTarget);
		++args;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	void OnGameObjectActivate(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_GAMEOBJECT_ACTIVATE] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_GAMEOBJECT_ACTIVATE]))
			++args;
		sLuaMgr.PUSH_UINT(entry);
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=3;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	void OnCreatureKill(uint32 entry, Player* mTarget, QuestLogEntry *qLogEntry)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_CREATURE_KILL] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_CREATURE_KILL]))
			++args;
		sLuaMgr.PUSH_UINT(entry);
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=3;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	void OnExploreArea(uint32 areaId, Player* mTarget, QuestLogEntry *qLogEntry)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_EXPLORE_AREA] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_EXPLORE_AREA]))
			++args;
		sLuaMgr.PUSH_UINT(areaId);
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=3;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	void OnPlayerItemPickup(uint32 itemId, uint32 totalCount, Player* mTarget, QuestLogEntry *qLogEntry)
	{
		CHECK_BINDING_ACQUIRELOCK
		if (m_binding->Functions[QUEST_EVENT_ON_PLAYER_ITEMPICKUP] == NULL) { RELEASE_LOCK; return; }
		uint8 args = 0;
		if(sLuaMgr.BeginCall(m_binding->Functions[QUEST_EVENT_ON_PLAYER_ITEMPICKUP]))
			++args;
		sLuaMgr.PUSH_UINT(itemId);
		sLuaMgr.PUSH_UINT(totalCount);
		sLuaMgr.PUSH_UNIT(mTarget);
		sLuaMgr.PUSH_UINT(qLogEntry->GetQuest()->id);
		args+=4;
		sLuaMgr.ExecuteCall(args);
		RELEASE_LOCK
	}
	LuaQuestBinding * m_binding;
};

CreatureAIScript * CreateLuaCreature(Creature * src)
{
	LuaCreature * script = NULL;
	if(src != NULL)
	{
		uint32 id = src->GetEntry();
		uint64 guid = src->GetGUID();
		LuaUnitBinding * pBinding = sLuaMgr.getUnitBinding(id);
		if( pBinding != NULL)
		{
			typedef std::multimap<uint32,LuaCreature*> CRCMAP;
			CRCMAP & cMap = sLuaMgr.getLuCreatureMap();
			CRCMAP::iterator itr = cMap.find(id);
			CRCMAP::iterator itend = cMap.upper_bound(id);
			for(; itr != cMap.end() && itr != itend; ++itr)
			{
				//grab the 1st and initalize
				if(itr->second == NULL)
				{
					script = itr->second = new LuaCreature(src);
					break;
				}
				if(itr->second != NULL && itr->second->GetUnit() != NULL && itr->second->GetUnit()->GetGUID() == guid)
					script = itr->second;
			}
			if(script == NULL)
			{
				script = new LuaCreature(src);
				cMap.insert(make_pair(id,script));
			}
			script->m_binding = pBinding;
		}
	}
	return script;
}

GameObjectAIScript * CreateLuaGameObject(GameObject * src)
{
	LuaGameObject * script = NULL;
	if(src != NULL) 
	{
		uint32 id = src->GetInfo()->ID;
		uint64 guid = src->GetGUID();
		LuaGameObjectBinding * pBinding = NULL;
		pBinding = sLuaMgr.getGameObjectBinding(id);
		if( pBinding != NULL) 
		{
			typedef multimap<uint32,LuaGameObject*> GMAP;
			GMAP & gMap = sLuaMgr.getLuGameObjectMap();
			GMAP::iterator itr = gMap.find(id);
			GMAP::iterator itend = gMap.upper_bound(id);
			for(; itr != gMap.end() && itr != itend; ++itr)
			{
				if(itr->second != NULL && itr->second->getGO() != NULL && itr->second->getGO()->GetGUID() == guid)
					script = itr->second;
			}
			if(script == NULL)
			{
				script = new LuaGameObject(src);
				gMap.insert(make_pair(id,script));
			}
			script->m_binding = pBinding;
			
		}
	}
	return script;
}

QuestScript * CreateLuaQuestScript(uint32 id)
{
	LuaQuest * pLua = NULL;
	LuaQuestBinding * pBinding = sLuaMgr.getQuestBinding( id );
	if(pBinding != NULL)
	{
		typedef HM_NAMESPACE::hash_map<uint32,LuaQuest*> QMAP;
		QMAP & qMap = sLuaMgr.getLuQuestMap();
		QMAP::iterator itr = qMap.find(id);
		if(itr != qMap.end() )
		{
			if(itr->second == NULL)
				pLua = itr->second = new LuaQuest();
			else
				pLua = itr->second;
		}
		else
		{
			pLua = new LuaQuest();
			qMap.insert(make_pair(id,pLua));
		}
		pLua->m_binding = pBinding;
	}
	return pLua;
}

GossipScript * CreateLuaUnitGossipScript(uint32 id)
{
	LuaGossip * pLua = NULL;
    LuaUnitGossipBinding * pBinding = sLuaMgr.getLuaUnitGossipBinding( id );
	if( pBinding != NULL )
	{
		typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
		GMAP & gMap = sLuaMgr.getUnitGossipInterfaceMap();
		GMAP::iterator itr = gMap.find(id);
		if(itr != gMap.end() )
		{
			if(itr->second == NULL)
				pLua = itr->second = new LuaGossip();
			else
				pLua = itr->second;
		}
		else
		{
			pLua = new LuaGossip();
			gMap.insert(make_pair(id,pLua));
		}
		pLua->m_unit_gossip_binding = pBinding;
	}
	return pLua;
}
GossipScript * CreateLuaItemGossipScript(uint32 id)
 {
	LuaGossip * pLua = NULL;
    LuaItemGossipBinding * pBinding = sLuaMgr.getLuaItemGossipBinding( id );
 	if( pBinding != NULL )
	{
		typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
		GMAP & gMap = sLuaMgr.getItemGossipInterfaceMap();
		GMAP::iterator itr = gMap.find(id);
		if(itr != gMap.end() )
		{
			if(itr->second == NULL)
				pLua = itr->second = new LuaGossip();
			else
				pLua = itr->second;
		}
		else
		{
			pLua = new LuaGossip();
			gMap.insert(make_pair(id,pLua));

		}
		pLua->m_item_gossip_binding = pBinding;
	}
 	return pLua;
 }
GossipScript * CreateLuaGOGossipScript(uint32 id)
{
	LuaGossip * pLua = NULL;
    LuaGOGossipBinding * pBinding = g_luaMgr.getLuaGOGossipBinding( id );
	if( pBinding != NULL )
	{
		typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
		GMAP & gMap = sLuaMgr.getGameObjectGossipInterfaceMap();
		GMAP::iterator itr = gMap.find(id);
		if(itr != gMap.end() )
		{
			if(itr->second == NULL)
				pLua = itr->second = new LuaGossip();
			else
				pLua = itr->second;
		}
		else
		{
			pLua = new LuaGossip();
			gMap.insert(make_pair(id,pLua));
		}
		pLua->m_go_gossip_binding = pBinding;
	}
	return pLua;
}

void LuaEngine::Startup()
{
	Log.Notice("LuaEngineMgr", "Spawning Lua Engine...");
	#ifdef WIN32
	Log.Color(TGREEN);
	printf(" \_\_                        \_\_  \_\_                  \_\_\_\_\_\_                 \n");
	Log.Color(TGREEN);
	printf("/\\ \\                      /\\ \\/\\ \\                /\\  \_  \\                 \n");
	Log.Color(TGREEN);
	printf("\\ \\ \\      \_\_  \_\_     \_\_  \\ \\ \\\_\\ \\  \_\_  \_\_  \_\_\_\_\_\\ \\ \\\_\\ \\  \_\_\_\_\_  \_\_\_    \n");
	Log.Color(TGREEN);
	printf(" \\ \\ \\  \_\_/\\ \\/\\ \\  /'\_\_`\\ \\ \\  \_  \\/\\ \\/\\ \\/\\  \_\_`\\ \\  \_\_ \\/\\  \_\_\\/'\_\_\_\\  \n");
	Log.Color(TGREEN);
	printf("  \\ \\ \\\_\\ \\ \\ \\\_\\ \\/\\ \\\_\\.\\\_\\ \\ \\ \\ \\ \\ \\\_\\ \\ \\ \\\_\\ \\ \\ \\/\\ \\ \\ \\//\\ \\\_\_/  \n");
	Log.Color(TGREEN);
	printf("   \\ \\\_\_\_\_/\\ \\\_\_\_\_/\\ \\\_\_/ \\\_\\\\ \\\_\\ \\\_\\/`\_\_\_\_ \\ \\  \_\_/\\ \\\_\\ \\\_\\ \\\_\\\\ \\\_\_\_\_\\ \n");
	Log.Color(TGREEN);
	printf("    \\/\_\_\_/  \\/\_\_\_/  \\/\_\_/\\/\_/ \\/\_/\\/\_/`/\_\_\_// \\ \\ \\/  \\/\_/\\/\_/\\/\_/ \\/\_\_\_\_/ \n");
	Log.Color(TGREEN);
	printf("                                         /\\\_\_\_/\\ \\\_\\                       \n");
	Log.Color(TGREEN);
	printf("                                         \\/\_\_/  \\/\_/                      \n");
	#else
	Log.Color(TGREEN);
	printf("~~~LuaHypArc~~~");
	#endif
	Log.Line();
	Log.Notice("LuaEngineMgr", "LuaHypArc Lua Engine %s: Loaded", ARCH);
	Log.Color(TNORMAL);
	Log.Line();
	Sleep(1200);
	//Create a new global state that will server as the lua universe.
	lu = lua_open();

	LoadScripts();

	// stuff is registered, so lets go ahead and make our emulated C++ scripted lua classes.
	for(UnitBindingMap::iterator itr = m_unitBinding.begin(); itr != m_unitBinding.end(); ++itr)
	{
		m_scriptMgr->register_creature_script( itr->first, CreateLuaCreature );
		sLuaMgr.getLuCreatureMap().insert(make_pair(itr->first,(LuaCreature*)NULL));
	}

	for(GameObjectBindingMap::iterator itr = m_gameobjectBinding.begin(); itr != m_gameobjectBinding.end(); ++itr)
	{
		m_scriptMgr->register_gameobject_script( itr->first, CreateLuaGameObject );
		sLuaMgr.getLuGameObjectMap().insert(make_pair(itr->first,(LuaGameObject*)NULL));
	}

	for(QuestBindingMap::iterator itr = m_questBinding.begin(); itr != m_questBinding.end(); ++itr)
	{
		QuestScript * qs = CreateLuaQuestScript( itr->first );
		if( qs != NULL )
		{
			m_scriptMgr->register_quest_script( itr->first, qs );
			sLuaMgr.getLuQuestMap().insert(make_pair(itr->first,(LuaQuest*)NULL));
		}
	}

    for(GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
 	{
		GossipScript * gs = CreateLuaUnitGossipScript( itr->first );
 		if( gs != NULL )
		{
 			m_scriptMgr->register_gossip_script( itr->first, gs );
			sLuaMgr.getUnitGossipInterfaceMap().insert(make_pair(itr->first,(LuaGossip*)NULL));
		}
	}

    for(GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
	{
		GossipScript * gs = CreateLuaItemGossipScript( itr->first );
		if( gs != NULL )
		{
			m_scriptMgr->register_item_gossip_script( itr->first, gs );
			sLuaMgr.getItemGossipInterfaceMap().insert(make_pair(itr->first,(LuaGossip*)NULL));
		}
    }

    for(GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
	{
		GossipScript * gs = CreateLuaGOGossipScript( itr->first );
		if( gs != NULL )
		{
			m_scriptMgr->register_go_gossip_script( itr->first, gs );
			sLuaMgr.getGameObjectGossipInterfaceMap().insert(make_pair(itr->first,(LuaGossip*)NULL));
		}
    }

	//big server hook chunk. it only hooks if there are functions present to save on unnecessary processing.

	RegisterHook(SERVER_HOOK_NEW_CHARACTER,(void*)LuaHookOnNewCharacter)
	RegisterHook(SERVER_HOOK_KILL_PLAYER,(void*)LuaHookOnKillPlayer)
	RegisterHook(SERVER_HOOK_FIRST_ENTER_WORLD,(void*)LuaHookOnFirstEnterWorld)
	RegisterHook(SERVER_HOOK_ENTER_WORLD,(void*)LuaHookOnEnterWorld)
	RegisterHook(SERVER_HOOK_GUILD_JOIN,(void*)LuaHookOnGuildJoin)
	RegisterHook(SERVER_HOOK_DEATH,(void*)LuaHookOnDeath)
	RegisterHook(SERVER_HOOK_REPOP,(void*)LuaHookOnRepop)
	RegisterHook(SERVER_HOOK_EMOTE,(void*)LuaHookOnEmote)
	RegisterHook(SERVER_HOOK_ENTER_COMBAT,(void*)LuaHookOnEnterCombat)
	RegisterHook(SERVER_HOOK_CAST_SPELL,(void*)LuaHookOnCastSpell)
	RegisterHook(SERVER_HOOK_TICK,(void*)LuaHookOnTick)
	RegisterHook(SERVER_HOOK_LOGOUT_REQUEST,(void*)LuaHookOnLogoutRequest)
	RegisterHook(SERVER_HOOK_LOGOUT,(void*)LuaHookOnLogout)
	RegisterHook(SERVER_HOOK_QUEST_ACCEPT,(void*)LuaHookOnQuestAccept)
	RegisterHook(SERVER_HOOK_ZONE,(void*)LuaHookOnZone)
	RegisterHook(SERVER_HOOK_CHAT,(void*)LuaHookOnChat)
	RegisterHook(SERVER_HOOK_LOOT,(void*)LuaHookOnLoot)
	RegisterHook(SERVER_HOOK_GUILD_CREATE,(void*)LuaHookOnGuildCreate)
	RegisterHook(SERVER_HOOK_ENTER_WORLD_2,(void*)LuaHookOnEnterWorld2)
	RegisterHook(SERVER_HOOK_CHARACTER_CREATE,(void*)LuaHookOnCharacterCreate)
	RegisterHook(SERVER_HOOK_QUEST_CANCELLED,(void*)LuaHookOnQuestCancelled)
	RegisterHook(SERVER_HOOK_QUEST_FINISHED,(void*)LuaHookOnQuestFinished)
	RegisterHook(SERVER_HOOK_HONORABLE_KILL,(void*)LuaHookOnHonorableKill)
	RegisterHook(SERVER_HOOK_ARENA_FINISH,(void*)LuaHookOnArenaFinish)
	RegisterHook(SERVER_HOOK_OBJECTLOOT,(void*)LuaHookOnObjectLoot)
	RegisterHook(SERVER_HOOK_AREATRIGGER,(void*)LuaHookOnAreaTrigger)
	RegisterHook(SERVER_HOOK_POST_LEVELUP,(void*)LuaHookOnPostLevelUp)
	RegisterHook(SERVER_HOOK_PRE_DIE,(void*)LuaHookOnPreUnitDie)
	RegisterHook(SERVER_HOOK_ADVANCE_SKILLLINE,(void*)LuaHookOnAdvanceSkillLine)

	for (std::map<uint32,const char*>::iterator itr = m_luaDummySpells.begin(); itr != m_luaDummySpells.end(); ++itr)
	{
		m_scriptMgr->register_dummy_spell(itr->first, &LuaOnDummySpell);
		sLuaMgr.HookInfo.dummyHooks.push_back(itr->first);
	}
}
void LuaEngine::RegisterEvent(uint8 regtype, uint32 id, uint32 evt, const char *func) 
{
	if(func != NULL && evt) 
	{
		switch(regtype) 
		{
			case REGTYPE_UNIT: 
				{
					if(id && evt < CREATURE_EVENT_COUNT) {
						LuaUnitBinding * bind = getUnitBinding(id);
						if(bind == NULL) {
							LuaUnitBinding nbind;
							memset(&nbind,0,sizeof(LuaUnitBinding));
							nbind.Functions[evt] = strdup(func);
							m_unitBinding.insert(make_pair(id,nbind));
						}
						else
						{
							if(bind->Functions[evt] != NULL)
								free((void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
			case REGTYPE_GO:
				{
					if(id && evt < GAMEOBJECT_EVENT_COUNT) {
						LuaGameObjectBinding * bind = getGameObjectBinding(id);
						if(bind == NULL) {
							LuaGameObjectBinding nbind;
							memset(&nbind,0,sizeof(LuaGameObjectBinding));
							nbind.Functions[evt] = strdup(func);
							m_gameobjectBinding.insert(make_pair(id,nbind));
						}
						else {
							if(bind->Functions[evt] != NULL)
								free( (void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
			case REGTYPE_QUEST:
				{
					if(id && evt < QUEST_EVENT_COUNT) {
						LuaQuestBinding * bind = getQuestBinding(id);
						if(bind == NULL) {
							LuaQuestBinding nbind;
							memset(&nbind,0,sizeof(LuaQuestBinding));
							nbind.Functions[evt] = strdup(func);
							m_questBinding.insert(make_pair(id,nbind));
						}
						else
						{
							if(bind->Functions[evt] != NULL)
								free( (void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
			case REGTYPE_SERVHOOK:
				{
					if(evt < SERVER_HOOK_COUNT)
						EventAsToFuncName[evt].push_back(string(func));
				}break;
			case REGTYPE_DUMMYSPELL:
				{
					if (id)
						m_luaDummySpells.insert( make_pair<uint32,const char*>(id,strdup(func)) );
				}break;
			case REGTYPE_UNIT_GOSSIP:
				{
					if(id && evt < GOSSIP_EVENT_COUNT) {
						LuaUnitGossipBinding * bind = getLuaUnitGossipBinding(id);
						if(bind == NULL) {
							LuaUnitGossipBinding nbind;
							memset(&nbind,0,sizeof(LuaUnitGossipBinding));
							nbind.Functions[evt] = strdup(func);
							m_unit_gossipBinding.insert(make_pair(id,nbind));
						}
						else {
							if(bind->Functions[evt] != NULL)
								free( (void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
				case REGTYPE_ITEM_GOSSIP:
				{
					if(id && evt < GOSSIP_EVENT_COUNT) {
						LuaItemGossipBinding * bind = getLuaItemGossipBinding(id);
						if(bind == NULL) {
							LuaItemGossipBinding nbind;
							memset(&nbind,0,sizeof(LuaItemGossipBinding));
							nbind.Functions[evt] = strdup(func);
							m_item_gossipBinding.insert(make_pair(id,nbind));
						}
						else {
							if(bind->Functions[evt] != NULL)
								free( (void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
				case REGTYPE_GO_GOSSIP:
				{
					if(id && evt < GOSSIP_EVENT_COUNT) {
						LuaGOGossipBinding * bind = getLuaGOGossipBinding(id);
						if(bind == NULL) {
							LuaGOGossipBinding nbind;
							memset(&nbind,0,sizeof(LuaGOGossipBinding));
							nbind.Functions[evt] = strdup(func);
							m_go_gossipBinding.insert(make_pair(id,nbind));
						}
						else {
							if(bind->Functions[evt] != NULL)
								free( (void*)bind->Functions[evt]);
							bind->Functions[evt] = strdup(func);
						}
					}
				}break;
		}
	}
}

void LuaEngine::Unload()
{
	// clean up the engine of any existing defined variables
	{
		UnitBindingMap::iterator itr = this->m_unitBinding.begin();
		for(; itr != m_unitBinding.end(); ++itr)
		{
			for(int i = 0; i < CREATURE_EVENT_COUNT; ++i) 
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_unitBinding.clear();
	}
	{
		GameObjectBindingMap::iterator itr = this->m_gameobjectBinding.begin();
		for(; itr != m_gameobjectBinding.end(); ++itr)
		{
			for(int i = 0; i < GAMEOBJECT_EVENT_COUNT; ++i) 
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_gameobjectBinding.clear();
	}
	{
		QuestBindingMap::iterator itr = this->m_questBinding.begin();
		for(; itr != m_questBinding.end(); ++itr)
		{
			for(int i = 0; i < 8; ++i) 
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_questBinding.clear();
	}
	{
		GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.begin();
		for(; itr != m_unit_gossipBinding.end(); ++itr)
		{
			for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_unit_gossipBinding.clear();
	}
	{
		GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.begin();
		for(; itr != m_item_gossipBinding.end(); ++itr)
		{
			for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_item_gossipBinding.clear();
	}
	{
		GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.begin();
		for(; itr != m_go_gossipBinding.end(); ++itr)
		{
			for(int i = 0; i < GOSSIP_EVENT_COUNT; ++i)
			{
				if(itr->second.Functions[i] != NULL)
					free((void*)itr->second.Functions[i]);
			}
		}
		m_go_gossipBinding.clear();
	}
	//Serv hooks : had forgotten these.
	{
		for(int i = 0; i < SERVER_HOOK_COUNT; ++i)
		{
			vector<string> & next = EventAsToFuncName[i];
			next.clear();
		}
	}
	{
		std::map<uint32, const char*>::iterator itr = m_luaDummySpells.begin();
		for (; itr != m_luaDummySpells.end(); ++itr)
		{
			free((void*)itr->second);
		}
	}
	set<int>::iterator itr = m_pendingThreads.begin();
	for(; itr != m_pendingThreads.end(); ++itr)
	{
		lua_unref(lu,(*itr));
	}
	m_pendingThreads.erase(m_pendingThreads.begin(),m_pendingThreads.end());

	lua_close(lu);
}
void LuaEngine::Restart()
{
	Log.Notice("LuaEngineMgr","Restarting Engine.");
	if(getLock().AttemptAcquire() && getcoLock().AttemptAcquire() )
	{
		Unload();
		lu = lua_open();
		LoadScripts();
		for(UnitBindingMap::iterator itr = m_unitBinding.begin(); itr != m_unitBinding.end(); ++itr)
		{
			typedef multimap<uint32,LuaCreature*> CMAP;
			CMAP & cMap = sLuaMgr.getLuCreatureMap();
			CMAP::iterator it = cMap.find(itr->first);
			CMAP::iterator itend = cMap.upper_bound(itr->first);
			if(it == cMap.end() )
			{
				m_scriptMgr->register_creature_script(itr->first,CreateLuaCreature);
				cMap.insert(make_pair(itr->first,(LuaCreature*)NULL));
			}
			else
			{
				for(;it != itend; ++it) 
				{
					if(it->second != NULL)
						it->second->m_binding = &itr->second;
				}
			}
		}
		for(GameObjectBindingMap::iterator itr = m_gameobjectBinding.begin(); itr != m_gameobjectBinding.end(); ++itr)
		{
			typedef multimap<uint32,LuaGameObject*> GMAP;
			GMAP & gMap = sLuaMgr.getLuGameObjectMap();
			GMAP::iterator it = gMap.find(itr->first);
			GMAP::iterator itend = gMap.upper_bound(itr->first);
			if(it == gMap.end() )
			{
				m_scriptMgr->register_gameobject_script(itr->first,CreateLuaGameObject);
				gMap.insert(make_pair(itr->first,(LuaGameObject*)NULL));
			}
			else
			{
				for(;it != itend; ++it)
				{
					if(it->second != NULL)
						it->second->m_binding = &itr->second;
				}
			}
		}
		for(QuestBindingMap::iterator itr = m_questBinding.begin(); itr != m_questBinding.end(); ++itr)
		{
			typedef HM_NAMESPACE::hash_map<uint32,LuaQuest*> QMAP;
			QMAP & qMap = sLuaMgr.getLuQuestMap();
			QMAP::iterator it = qMap.find(itr->first);
			if(it == qMap.end())
			{
				m_scriptMgr->register_quest_script(itr->first,CreateLuaQuestScript(itr->first));
				qMap.insert(make_pair(itr->first,(LuaQuest*)NULL));
			}
			else
			{
				LuaQuest * q_interface = it->second;
				if(q_interface != NULL)
					q_interface->m_binding = &itr->second;
			}
		}
		for(GossipUnitScriptsBindingMap::iterator itr = this->m_unit_gossipBinding.begin(); itr != m_unit_gossipBinding.end(); ++itr)
		{
			typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
			GMAP & gMap = sLuaMgr.getUnitGossipInterfaceMap();
			GMAP::iterator it = gMap.find(itr->first);
			if(it == gMap.end() ) 
			{
				GossipScript * gs = CreateLuaUnitGossipScript(itr->first);
				if(gs != NULL)
				{
					m_scriptMgr->register_gossip_script(itr->first,gs);
					gMap.insert(make_pair(itr->first,(LuaGossip*)NULL));
				}
			}
			else
			{
				LuaGossip * u_gossip = it->second;
				if(u_gossip != NULL)
					u_gossip->m_unit_gossip_binding = &itr->second;
			}
		}
		for(GossipItemScriptsBindingMap::iterator itr = this->m_item_gossipBinding.begin(); itr != m_item_gossipBinding.end(); ++itr)
		{
			typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
			GMAP & gMap = sLuaMgr.getItemGossipInterfaceMap();
			GMAP::iterator it = gMap.find(itr->first);
			if(it == gMap.end() ) 
			{
				GossipScript * gs = CreateLuaItemGossipScript(itr->first);
				if(gs != NULL)
				{
					m_scriptMgr->register_item_gossip_script(itr->first,gs);
					gMap.insert(make_pair(itr->first,(LuaGossip*)NULL));
				}
			}
			else
			{
				LuaGossip * i_gossip = it->second;
				if(i_gossip != NULL)
					i_gossip->m_item_gossip_binding = &itr->second;
			}
		}
		for(GossipGOScriptsBindingMap::iterator itr = this->m_go_gossipBinding.begin(); itr != m_go_gossipBinding.end(); ++itr)
		{
			typedef HM_NAMESPACE::hash_map<uint32,LuaGossip*> GMAP;
			GMAP & gMap = sLuaMgr.getGameObjectGossipInterfaceMap();
			GMAP::iterator it = gMap.find(itr->first);
			if(it == gMap.end() ) 
			{
				GossipScript * gs = CreateLuaGOGossipScript(itr->first);
				if(gs != NULL)
				{
					m_scriptMgr->register_go_gossip_script(itr->first,gs);
					gMap.insert(make_pair(itr->first,(LuaGossip*)NULL));
				}
			}
			else
			{
				LuaGossip * g_gossip = it->second;
				if(g_gossip != NULL)
					g_gossip->m_go_gossip_binding = &itr->second;
			}
		}
		/*
			BIG SERV HOOK CHUNK EEK 
			*/
		RegisterHook(SERVER_HOOK_NEW_CHARACTER,(void*)LuaHookOnNewCharacter)
		RegisterHook(SERVER_HOOK_KILL_PLAYER,(void*)LuaHookOnKillPlayer)
		RegisterHook(SERVER_HOOK_FIRST_ENTER_WORLD,(void*)LuaHookOnFirstEnterWorld)
		RegisterHook(SERVER_HOOK_ENTER_WORLD,(void*)LuaHookOnEnterWorld)
		RegisterHook(SERVER_HOOK_GUILD_JOIN,(void*)LuaHookOnGuildJoin)
		RegisterHook(SERVER_HOOK_DEATH,(void*)LuaHookOnDeath)
		RegisterHook(SERVER_HOOK_REPOP,(void*)LuaHookOnRepop)
		RegisterHook(SERVER_HOOK_EMOTE,(void*)LuaHookOnEmote)
		RegisterHook(SERVER_HOOK_ENTER_COMBAT,(void*)LuaHookOnEnterCombat)
		RegisterHook(SERVER_HOOK_CAST_SPELL,(void*)LuaHookOnCastSpell)
		RegisterHook(SERVER_HOOK_TICK,(void*)LuaHookOnTick)
		RegisterHook(SERVER_HOOK_LOGOUT_REQUEST,(void*)LuaHookOnLogoutRequest)
		RegisterHook(SERVER_HOOK_LOGOUT,(void*)LuaHookOnLogout)
		RegisterHook(SERVER_HOOK_QUEST_ACCEPT,(void*)LuaHookOnQuestAccept)
		RegisterHook(SERVER_HOOK_ZONE,(void*)LuaHookOnZone)
		RegisterHook(SERVER_HOOK_CHAT,(void*)LuaHookOnChat)
		RegisterHook(SERVER_HOOK_LOOT,(void*)LuaHookOnLoot)
		RegisterHook(SERVER_HOOK_GUILD_CREATE,(void*)LuaHookOnGuildCreate)
		RegisterHook(SERVER_HOOK_ENTER_WORLD_2,(void*)LuaHookOnEnterWorld2)
		RegisterHook(SERVER_HOOK_CHARACTER_CREATE,(void*)LuaHookOnCharacterCreate)
		RegisterHook(SERVER_HOOK_QUEST_CANCELLED,(void*)LuaHookOnQuestCancelled)
		RegisterHook(SERVER_HOOK_QUEST_FINISHED,(void*)LuaHookOnQuestFinished)
		RegisterHook(SERVER_HOOK_HONORABLE_KILL,(void*)LuaHookOnHonorableKill)
		RegisterHook(SERVER_HOOK_ARENA_FINISH,(void*)LuaHookOnArenaFinish)
		RegisterHook(SERVER_HOOK_OBJECTLOOT,(void*)LuaHookOnObjectLoot)
		RegisterHook(SERVER_HOOK_AREATRIGGER,(void*)LuaHookOnAreaTrigger)
		RegisterHook(SERVER_HOOK_POST_LEVELUP,(void*)LuaHookOnPostLevelUp)
		RegisterHook(SERVER_HOOK_PRE_DIE,(void*)LuaHookOnPreUnitDie)
		RegisterHook(SERVER_HOOK_ADVANCE_SKILLLINE,(void*)LuaHookOnAdvanceSkillLine)

		for (std::map<uint32,const char*>::iterator itr = m_luaDummySpells.begin(); itr != m_luaDummySpells.end(); ++itr)
		{
			if (std::find(sLuaMgr.HookInfo.dummyHooks.begin(), sLuaMgr.HookInfo.dummyHooks.end(), itr->first) 
				!= sLuaMgr.HookInfo.dummyHooks.end())
			{
				m_scriptMgr->register_dummy_spell(itr->first, &LuaOnDummySpell);
				sLuaMgr.HookInfo.dummyHooks.push_back(itr->first);
			}
		}
		getLock().Release();
		getcoLock().Release();
	}
	Log.Notice("LuaEngineMgr","Done restarting engine.");
}

void LuaEngine::ResumeLuaThread(int ref) {
	getcoLock().Acquire();
	lua_State * expectedThread = NULL;
	lua_rawgeti(lu,LUA_REGISTRYINDEX,ref);
	if(lua_isthread(lu,-1) )
		expectedThread = lua_tothread(lu,-1);
	if(expectedThread != NULL) 
	{
		//push ourself on the stack
		lua_pushthread(expectedThread);
		//move the thread to the main lu state(and pop it off)
		lua_xmove(expectedThread,lu,1);
		if(lua_rawequal(lu,-1,-2) )
		{
			lua_pop(lu,2);
			int res = lua_resume(expectedThread,lua_gettop(expectedThread));
			if(res != LUA_YIELD && res)
				report(expectedThread);
		}
		else
			lua_pop(lu,2);
		luaL_unref(lu,LUA_REGISTRYINDEX,ref);
	}
	getcoLock().Release();
}


/************************************************************************/
/* SCRIPT FUNCTION IMPLEMENTATION                                       */
/************************************************************************/
#define CHECK_TYPEID(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { return 0; }
#define CHECK_TYPEID_RET(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { lua_pushboolean(L,0); return 1; }
#define CHECK_TYPEID_RETINT(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { lua_pushinteger(L,0); return 1; }
#define CHECK_TYPEID_RETNIL(expected_type) if(!ptr || !ptr->IsInWorld() || ptr->GetTypeId() != expected_type) { lua_pushnil(L); return 1; } (void*)0
#define RET_NIL { lua_pushnil(L); return 1; } (void*)0
#define RET_BOOL(exp) { (exp) ? lua_pushboolean(L,1) : lua_pushboolean(L,0); return 1; } (void*)0
#define RET_STRING(str) { lua_pushstring(L,(str)); return 1; } (void*)0
#define RET_NUMBER(number) { lua_pushnumber(L,(number)); return 1; } (void*)0
#define RET_INT(integer) { lua_pushinteger(L,(integer)); return 1; } (void*)0

// Simplicity macros.
#define CHECK_UNIT(L,narg) sLuaMgr.CHECK_UNIT(L,narg)
#define CHECK_PLAYER(L,narg) TO_PLAYER(CHECK_UNIT(L,narg))
#define CHECK_GO(L,narg) sLuaMgr.CHECK_GO(L,narg)
#define CHECK_ITEM(L,narg) sLuaMgr.CHECK_ITEM(L,narg)
#define CHECK_PACKET(L,narg) sLuaMgr.CHECK_PACKET(L,narg)
#define CHECK_GUID(L, narg) sLuaMgr.CHECK_GUID(L,narg)
#define CHECK_OBJECT(L, narg) sLuaMgr.CHECK_OBJECT(L,narg)
#define CHECK_TAXIPATH(L, narg) sLuaMgr.CHECK_TAXIPATH(L,narg)
#define CHECK_SPELL(L, narg) sLuaMgr.CHECK_SPELL(L,narg)

//Its coming soon ^.^
//#define CHECK_SPELL(L,narg) ArcLuna<Spell>::check(L),(narg))
//This is used alot when checking for coords but Lua handles only doubles.
#define CHECK_FLOAT(L,narg) (lua_isnoneornil(L,(narg)) ) ? 0.00f : (float)luaL_checknumber(L,(narg)); 
#define CHECK_ULONG(L,narg) (uint32)luaL_checknumber((L),(narg))
#define CHECK_USHORT(L, narg) (uint16)luaL_checkinteger((L),(narg))
#define CHECK_BOOL(L,narg) (lua_toboolean((L),(narg)) > 0) ? true : false

#define PUSH_UNIT(L, unit) sLuaMgr.PUSH_UNIT(TO_UNIT(unit),L)
#define PUSH_GO(L, go) sLuaMgr.PUSH_GO(static_cast<GameObject*>(go),L)
#define PUSH_PACKET(L,pack) sLuaMgr.PUSH_PACKET(pack,L)
#define PUSH_ITEM(L,item) sLuaMgr.PUSH_ITEM(static_cast<Item*>(item),L)
#define PUSH_GUID(L, obj) sLuaMgr.PUSH_GUID(obj,L)
#define PUSH_TAXIPATH(L, tp) sLuaMgr.PUSH_TAXIPATH(tp,L)
#define PUSH_SPELL(L, sp) sLuaMgr.PUSH_SPELL(sp,L)
#define PUSH_SQLFIELD(L, field) sLuaMgr.PUSH_SQLFIELD(field,L)
#define PUSH_SQLRESULT(L, res) sLuaMgr.PUSH_SQLRESULT(res,L)

//I know its not a good idea to do it like that BUT it is the easiest way. I will make it better in steps:
#include "LUAFunctions.h"
#include "FunctionTables.h"

