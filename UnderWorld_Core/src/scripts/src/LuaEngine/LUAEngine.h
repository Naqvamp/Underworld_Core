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

#ifndef __LUAENGINE_H
#define __LUAENGINE_H

extern "C" {		// we're C++, and LUA is C, so the compiler needs to know to use C function names.
#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
};

#include <sys/stat.h>
#include <sys/types.h>

class LuaEngine;
class LuaCreature;
class LuaGameObject;
class LuaQuest;
class LuaGossip;
class ArcLuna;

#ifdef WIN32
	HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#define dropNotice sLog.outDebug
#define dropWarning sLog.outDetail
#define dropError sLog.outString
#define dropFatal sLog.outError

extern LuaEngine g_luaMgr;
#define sLuaMgr g_luaMgr

GossipMenu * Menu;

#define GET_LOCK sLuaMgr.getLock().Acquire();
#define RELEASE_LOCK sLuaMgr.getLock().Release();
#define CHECK_BINDING_ACQUIRELOCK GET_LOCK if(m_binding == NULL) { RELEASE_LOCK return; }

#define RegisterHook(evt, _func) { \
	if(EventAsToFuncName[(evt)].size() > 0 && !sLuaMgr.HookInfo.hooks[(evt)]) { \
		sLuaMgr.HookInfo.hooks[(evt)] = true; \
		m_scriptMgr->register_hook( (ServerHookEvents)(evt), (_func) ); } }

/** Quest Events
 */
enum QuestEvents
{
	QUEST_EVENT_ON_ACCEPT = 1,
	QUEST_EVENT_ON_COMPLETE = 2,
	QUEST_EVENT_ON_CANCEL = 3,
	QUEST_EVENT_GAMEOBJECT_ACTIVATE = 4,
	QUEST_EVENT_ON_CREATURE_KILL  = 5,
	QUEST_EVENT_ON_EXPLORE_AREA = 6,
	QUEST_EVENT_ON_PLAYER_ITEMPICKUP = 7,
	QUEST_EVENT_COUNT,
};

/** Creature Events
 */
enum CreatureEvents
{
	CREATURE_EVENT_ON_ENTER_COMBAT		= 1,
	CREATURE_EVENT_ON_LEAVE_COMBAT		= 2,
	CREATURE_EVENT_ON_TARGET_DIED		= 3,
	CREATURE_EVENT_ON_DIED		= 4,
	CREATURE_EVENT_ON_TARGET_PARRIED		= 5,
	CREATURE_EVENT_ON_TARGET_DODGED		= 6,
	CREATURE_EVENT_ON_TARGET_BLOCKED		= 7,
	CREATURE_EVENT_ON_TARGET_CRIT_HIT		= 8,
	CREATURE_EVENT_ON_PARRY		= 9,
	CREATURE_EVENT_ON_DODGED		= 10,
	CREATURE_EVENT_ON_BLOCKED		= 11,
	CREATURE_EVENT_ON_CRIT_HIT		= 12,
	CREATURE_EVENT_ON_HIT		= 13,
	CREATURE_EVENT_ON_ASSIST_TARGET_DIED		= 14,
	CREATURE_EVENT_ON_FEAR		= 15,
	CREATURE_EVENT_ON_FLEE		= 16,
	CREATURE_EVENT_ON_CALL_FOR_HELP		= 17,
	CREATURE_EVENT_ON_LOAD		= 18,
	CREATURE_EVENT_ON_REACH_WP		= 19,
	CREATURE_EVENT_ON_LOOT_TAKEN		= 20,
	CREATURE_EVENT_ON_AIUPDATE		= 21,
	CREATURE_EVENT_ON_EMOTE		= 22,
	CREATURE_EVENT_ON_DAMAGE_TAKEN = 23,
	CREATURE_EVENT_COUNT,
};

/** GameObject Events
 */
enum GameObjectEvents
{
	GAMEOBJECT_EVENT_ON_CREATE			= 1,
	GAMEOBJECT_EVENT_ON_SPAWN			= 2,
	GAMEOBJECT_EVENT_ON_LOOT_TAKEN		= 3,
	GAMEOBJECT_EVENT_ON_USE				= 4,
	GAMEOBJECT_EVENT_AIUPDATE			= 5,
	GAMEOBJECT_EVENT_ON_DESPAWN			= 6,
	GAMEOBJECT_EVENT_COUNT,
};

enum GossipEvents
{
	GOSSIP_EVENT_ON_TALK			= 1,
	GOSSIP_EVENT_ON_SELECT_OPTION	= 2,
	GOSSIP_EVENT_ON_END             = 3,
	GOSSIP_EVENT_COUNT,
};

enum RandomFlags
{
	RANDOM_ANY           = 0,
	RANDOM_IN_SHORTRANGE = 1,
	RANDOM_IN_MIDRANGE   = 2,
	RANDOM_IN_LONGRANGE  = 3,
	RANDOM_WITH_MANA     = 4,
	RANDOM_WITH_RAGE     = 5,
	RANDOM_WITH_ENERGY   = 6,
	RANDOM_NOT_MAINTANK  = 7,
	RANDOM_COUNT,
};

enum ServerHooks
{
	SERVER_HOOK_NEW_CHARACTER		= 1,
	SERVER_HOOK_KILL_PLAYER		= 2,
	SERVER_HOOK_FIRST_ENTER_WORLD	= 3,
	SERVER_HOOK_ENTER_WORLD		= 4,
	SERVER_HOOK_GUILD_JOIN			= 5,
	SERVER_HOOK_DEATH				= 6,
	SERVER_HOOK_REPOP				= 7,
	SERVER_HOOK_EMOTE				= 8,
	SERVER_HOOK_ENTER_COMBAT		= 9,
	SERVER_HOOK_CAST_SPELL			= 10,
	SERVER_HOOK_TICK				= 11,
	SERVER_HOOK_LOGOUT_REQUEST		= 12,
	SERVER_HOOK_LOGOUT				= 13,
	SERVER_HOOK_QUEST_ACCEPT		= 14,
	SERVER_HOOK_ZONE				= 15,
	SERVER_HOOK_CHAT				= 16,
	SERVER_HOOK_LOOT				= 17,
	SERVER_HOOK_GUILD_CREATE		= 18,
	SERVER_HOOK_ENTER_WORLD_2		= 19,
	SERVER_HOOK_CHARACTER_CREATE	= 20,
	SERVER_HOOK_QUEST_CANCELLED	= 21,
	SERVER_HOOK_QUEST_FINISHED		= 22,
	SERVER_HOOK_HONORABLE_KILL		= 23,
	SERVER_HOOK_ARENA_FINISH		= 24,
	SERVER_HOOK_OBJECTLOOT			= 25,
	SERVER_HOOK_AREATRIGGER		= 26,
	SERVER_HOOK_POST_LEVELUP       = 27,
	SERVER_HOOK_PRE_DIE	        = 28,	//general unit die, not only based on players
	SERVER_HOOK_ADVANCE_SKILLLINE  = 29,

	SERVER_HOOK_COUNT,
};
//reg type defines
#define REGTYPE_UNIT (1 << 0)
#define REGTYPE_GO (1 << 1)
#define REGTYPE_QUEST (1 << 2)
#define REGTYPE_SERVHOOK (1 << 3)
#define REGTYPE_ITEM (1 << 4)
#define REGTYPE_GOSSIP (1 << 5)
#define REGTYPE_DUMMYSPELL (1 << 6)
#define REGTYPE_UNIT_GOSSIP (REGTYPE_UNIT | REGTYPE_GOSSIP)
#define REGTYPE_GO_GOSSIP (REGTYPE_GO | REGTYPE_GOSSIP)
#define REGTYPE_ITEM_GOSSIP (REGTYPE_ITEM | REGTYPE_GOSSIP)

struct LUALoadScripts
{
	set<string> luaFiles;
};

struct LuaUnitBinding { const char * Functions[CREATURE_EVENT_COUNT]; };
struct LuaQuestBinding { const char * Functions[QUEST_EVENT_COUNT]; };
struct LuaGameObjectBinding { const char * Functions[GAMEOBJECT_EVENT_COUNT]; };
struct LuaUnitGossipBinding { const char * Functions[GOSSIP_EVENT_COUNT]; };
struct LuaItemGossipBinding { const char * Functions[GOSSIP_EVENT_COUNT]; };
struct LuaGOGossipBinding { const char * Functions[GOSSIP_EVENT_COUNT]; };
std::vector<string> EventAsToFuncName[SERVER_HOOK_COUNT]; //an array of string based vectors.
std::map<uint32, const char*> m_luaDummySpells;

template<typename T>
struct RegType
{
	const char * name;
	int(*mfunc)(lua_State*,T*);
};
template<typename T> RegType<T>* GetMethodTable();
template<typename T> const char * GetTClassName();
void report(lua_State*);

class LuaEngine
{
private:
	lua_State * lu; // main state.
	Mutex call_lock;
	Mutex co_lock;

	typedef HM_NAMESPACE::hash_map<uint32, LuaUnitBinding> UnitBindingMap;
	typedef HM_NAMESPACE::hash_map<uint32, LuaQuestBinding> QuestBindingMap;
	typedef HM_NAMESPACE::hash_map<uint32, LuaGameObjectBinding> GameObjectBindingMap;
	typedef HM_NAMESPACE::hash_map<uint32, LuaUnitGossipBinding> GossipUnitScriptsBindingMap;
	typedef HM_NAMESPACE::hash_map<uint32, LuaItemGossipBinding> GossipItemScriptsBindingMap;
	typedef HM_NAMESPACE::hash_map<uint32, LuaGOGossipBinding> GossipGOScriptsBindingMap;
	std::set<int> m_pendingThreads;

	//maps to creature, & go script interfaces
	std::multimap<uint32, LuaCreature*> m_cAIScripts;
	std::multimap<uint32, LuaGameObject*> m_gAIScripts;
	HM_NAMESPACE::hash_map<uint32, LuaQuest*> m_qAIScripts;

	HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_unitgAIScripts;
	HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_itemgAIScripts;
	HM_NAMESPACE::hash_map<uint32, LuaGossip*> m_gogAIScripts;

	UnitBindingMap m_unitBinding;
	QuestBindingMap m_questBinding;
	GameObjectBindingMap m_gameobjectBinding;
	GossipUnitScriptsBindingMap m_unit_gossipBinding;
	GossipItemScriptsBindingMap m_item_gossipBinding;
	GossipGOScriptsBindingMap m_go_gossipBinding;

public:
	~LuaEngine() {
		getLock().Acquire();
		getcoLock().Acquire();
		if(lu != NULL)
			Unload();
		getLock().Release();
		getcoLock().Release();
	}
	void Startup();
	void LoadScripts();
	void Restart();

	void RegisterEvent(uint8 regtype, uint32 id, uint32 evt, const char * func);
	void ResumeLuaThread(int);
	bool BeginCall(const char * func);
	void HyperCallFunction(const char * FuncName, int ref);
	ARCEMU_INLINE bool ExecuteCall(uint8 params = 0,uint8 res = 0);
	ARCEMU_INLINE void EndCall(uint8 res = 0);
	//Wrappers
	ARCEMU_INLINE Unit * CHECK_UNIT(lua_State * L,int narg) 
	{ 
		if(L == NULL) return ArcLuna<Unit>::check(lu,narg);
		else return ArcLuna<Unit>::check(L,narg);
	}
	ARCEMU_INLINE GameObject * CHECK_GO(lua_State * L,int narg) 
	{ 
		if(L == NULL) return ArcLuna<GameObject>::check(lu,narg);
		else return ArcLuna<GameObject>::check(L,narg);
	}
	ARCEMU_INLINE Item * CHECK_ITEM(lua_State * L,int narg) 
	{ 
		if(L == NULL) return ArcLuna<Item>::check(lu,narg);
		else return ArcLuna<Item>::check(L,narg);
	}
	ARCEMU_INLINE WorldPacket * CHECK_PACKET(lua_State * L,int narg) 
	{ 
		if(L == NULL) return ArcLuna<WorldPacket>::check(lu,narg);
		else return ArcLuna<WorldPacket>::check(L,narg);
	}
	ARCEMU_INLINE uint64 CHECK_GUID(lua_State * L, int narg) {
		if(L == NULL) return GUID_MGR::check(lu,narg);
		else return GUID_MGR::check(L,narg);
	}
	ARCEMU_INLINE Object * CHECK_OBJECT(lua_State * L, int narg) {
		if(L == NULL) return ArcLuna<Object>::check(lu,narg);
		else return ArcLuna<Object>::check(L,narg);
	}
	ARCEMU_INLINE TaxiPath * CHECK_TAXIPATH(lua_State * L, int narg) {
		if(L == NULL) return ArcLuna<TaxiPath>::check(lu,narg);
		else return ArcLuna<TaxiPath>::check(L,narg);
	}
	ARCEMU_INLINE Spell * CHECK_SPELL(lua_State * L, int narg) {
		if(L == NULL) return ArcLuna<Spell>::check(lu,narg);
		else return ArcLuna<Spell>::check(L,narg);
	}
	//ARCEMU_INLINE QueryResult * CHECK_QRESULT(lua_State * L, int narg);
	//ARCEMU_INLINE Field * CHECK_SQLFIELD(lua_State *L, int narg);

	void PUSH_UNIT(Object * unit, lua_State * L = NULL);
	void PUSH_GO(Object * go, lua_State * L = NULL);
	void PUSH_ITEM(Object * item, lua_State * L = NULL);
	void PUSH_GUID(uint64 guid, lua_State * L = NULL);
	void PUSH_PACKET(WorldPacket * packet, lua_State * L = NULL);
	void PUSH_TAXIPATH(TaxiPath * tp, lua_State * L = NULL);
	void PUSH_SPELL(Spell * sp, lua_State * L = NULL);
	void PUSH_SQLFIELD(Field * field, lua_State * L = NULL);
	void PUSH_SQLRESULT(QueryResult * res, lua_State * L = NULL);

	ARCEMU_INLINE void PUSH_BOOL(bool bewl) {
		if(bewl) 
			lua_pushboolean(lu,1);
		else 
			lua_pushboolean(lu,0);
	}
	ARCEMU_INLINE void PUSH_NIL(lua_State * L = NULL) {
		if(L == NULL)
			lua_pushnil(lu);
		else
			lua_pushnil(L);
	}
	ARCEMU_INLINE void PUSH_INT(int32 value) {
		lua_pushinteger(lu,value); }
	ARCEMU_INLINE void PUSH_UINT(uint32 value) {
		lua_pushnumber(lu,value); }
	ARCEMU_INLINE void PUSH_FLOAT(float value) {
		lua_pushnumber(lu,value);
	}
	ARCEMU_INLINE void PUSH_STRING(const char * str) {
		lua_pushstring(lu,str);
	}
	void RegisterCoreFunctions();

	ARCEMU_INLINE Mutex & getLock() { return call_lock; }
	ARCEMU_INLINE Mutex & getcoLock() { return co_lock; }
	ARCEMU_INLINE lua_State * getluState() { return lu; }
	
	LuaUnitBinding * getUnitBinding(uint32 Id)
	{
		UnitBindingMap::iterator itr = m_unitBinding.find(Id);
		return (itr == m_unitBinding.end()) ? NULL : &itr->second;
	}

	LuaQuestBinding * getQuestBinding(uint32 Id)
	{
		QuestBindingMap::iterator itr = m_questBinding.find(Id);
		return (itr == m_questBinding.end()) ? NULL : &itr->second;
	}

	LuaGameObjectBinding * getGameObjectBinding(uint32 Id)
	{
		GameObjectBindingMap::iterator itr =m_gameobjectBinding.find(Id);
		return (itr == m_gameobjectBinding.end()) ? NULL : &itr->second;
	}

	LuaUnitGossipBinding * getLuaUnitGossipBinding(uint32 Id)
 	{
		GossipUnitScriptsBindingMap::iterator itr = m_unit_gossipBinding.find(Id);
		return (itr == m_unit_gossipBinding.end()) ? NULL : &itr->second;
 	}

    LuaItemGossipBinding * getLuaItemGossipBinding(uint32 Id)
	{
		GossipItemScriptsBindingMap::iterator itr = m_item_gossipBinding.find(Id);
		return (itr == m_item_gossipBinding.end()) ? NULL : &itr->second;
	}

    LuaGOGossipBinding * getLuaGOGossipBinding(uint32 Id)
	{
		GossipGOScriptsBindingMap::iterator itr = m_go_gossipBinding.find(Id);
		return (itr == m_go_gossipBinding.end()) ? NULL : &itr->second;
	}
	LuaQuest * getLuaQuest(uint32 id) {
		HM_NAMESPACE::hash_map<uint32, LuaQuest*>::iterator itr = m_qAIScripts.find(id);
		return (itr == m_qAIScripts.end()) ? NULL: itr->second;
	}
	/*int getPendingThread(lua_State * threadtosearch) {
		set<lua_State*>::iterator itr = m_pendingThreads.find(threadtosearch);
		return (itr == m_pendingThreads.end() )? NULL : (*itr);
	}*/
	LuaGossip * getUnitGossipInterface(uint32 id) 
	{
		HM_NAMESPACE::hash_map<uint32,LuaGossip*>::iterator itr = m_unitgAIScripts.find(id);
		return (itr == m_unitgAIScripts.end()) ? NULL : itr->second;
	}
	LuaGossip * getItemGossipInterface(uint32 id) 
	{
		HM_NAMESPACE::hash_map<uint32,LuaGossip*>::iterator itr = m_itemgAIScripts.find(id);
		return (itr == m_itemgAIScripts.end()) ? NULL : itr->second;
	}
	LuaGossip * getGameObjectGossipInterface(uint32 id)
	{
		HM_NAMESPACE::hash_map<uint32,LuaGossip*>::iterator itr = m_gogAIScripts.find(id);
		return (itr == m_gogAIScripts.end()) ? NULL : itr->second;
	}
	ARCEMU_INLINE std::multimap<uint32, LuaCreature*> & getLuCreatureMap() { return m_cAIScripts; }
	ARCEMU_INLINE std::multimap<uint32, LuaGameObject*> & getLuGameObjectMap() { return m_gAIScripts; }
	ARCEMU_INLINE HM_NAMESPACE::hash_map<uint32, LuaQuest*> & getLuQuestMap() { return m_qAIScripts; }
	ARCEMU_INLINE HM_NAMESPACE::hash_map<uint32, LuaGossip*> & getUnitGossipInterfaceMap() { return m_unitgAIScripts; }
	ARCEMU_INLINE HM_NAMESPACE::hash_map<uint32, LuaGossip*> & getItemGossipInterfaceMap() { return m_itemgAIScripts; }
	ARCEMU_INLINE HM_NAMESPACE::hash_map<uint32, LuaGossip*> & getGameObjectGossipInterfaceMap() { return m_gogAIScripts; }
	ARCEMU_INLINE set<int> & getThreadRefs() { return m_pendingThreads; }

	struct _ENGINEHOOKINFO { 
		bool hooks[SERVER_HOOK_COUNT];
		std::vector<uint32> dummyHooks;
		_ENGINEHOOKINFO() {
			for(int i = 0; i < SERVER_HOOK_COUNT; ++i)
				hooks[i] = false;
		}
	} HookInfo;

	std::map<int, TimedEvent*> m_registeredTimedEvents;

	//Inner Thread Class
protected:
	//Hidden methods
	void Unload(); 
	void ScriptLoadDir(char* Dirname, LUALoadScripts *pak);

	template <typename T> 
	class ArcLuna 
	{
	public:
		typedef int (*mfp)(lua_State *L, T* ptr);
		typedef struct { const char *name; mfp mfunc; } RegType;

		static void Register(lua_State *L)
		{
			lua_newtable(L);
			int methods = lua_gettop(L);

			luaL_newmetatable(L, GetTClassName<T>());
			int metatable = lua_gettop(L);
			
			luaL_newmetatable(L,"DO NOT TRASH");
			lua_pop(L,1);

			// store method table in globals so that
			// scripts can add functions written in Lua.
			lua_pushvalue(L, methods);
			lua_setfield(L, LUA_GLOBALSINDEX, GetTClassName<T>());

			// hide metatable from Lua getmetatable()
			lua_pushvalue(L, methods);
			lua_setfield(L, metatable, "__metatable");

			lua_pushvalue(L, methods);
			lua_setfield(L, metatable, "__index");

			lua_pushcfunction(L, tostring_T);
			lua_setfield(L, metatable, "__tostring");

			lua_pushcfunction(L, gc_T);
			lua_setfield(L, metatable, "__gc");

			lua_newtable(L);                // mt for method table
			lua_setmetatable(L, methods);

			// fill method table with methods from class T
			for (RegType *l = ((RegType*)GetMethodTable<T>()); l->name; l++) {
				lua_pushstring(L, l->name);
				lua_pushlightuserdata(L, (void*)l);
				lua_pushcclosure(L, thunk, 1);
				lua_settable(L, methods);
			}
			lua_pop(L, 2);  // drop metatable and method table
		}

	// push onto the Lua stack a userdata containing a pointer to T object
		static int push(lua_State *L, T *obj, bool gc=false) {
			if (!obj) { lua_pushnil(L); return lua_gettop(L); }
			luaL_getmetatable(L, GetTClassName<T>());  // lookup metatable in Lua registry
			if (lua_isnil(L, -1)) luaL_error(L, "%s missing metatable", GetTClassName<T>());
			int mt = lua_gettop(L);
			T ** ptrHold = (T**)lua_newuserdata(L,sizeof(T**));
			int ud = lua_gettop(L);
			if(ptrHold != NULL)
			{
				*ptrHold = obj;
				lua_pushvalue(L, mt);
				lua_setmetatable(L, -2);
				char name[32];
				tostring(name,obj);
				lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
				if(lua_isnil(L,-1) )
				{
					luaL_newmetatable(L,"DO NOT TRASH");
					lua_pop(L,1);
				}
				lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
				if(gc == false)
				{
					lua_pushboolean(L,1);
					lua_setfield(L,-2,name);
				}
				lua_pop(L,1);
			}
			lua_settop(L,ud);
			lua_replace(L, mt);
			lua_settop(L, mt);
			return mt;  // index of userdata containing pointer to T object
		}

	// get userdata from Lua stack and return pointer to T object
		static T *check(lua_State *L, int narg) {
			T ** ptrHold = static_cast<T**>(lua_touserdata(L,narg));
			if(ptrHold == NULL)
				return NULL;
			return *ptrHold;
		}

		private:
		ArcLuna();  // hide default constructor

		// member function dispatcher
		static int thunk(lua_State *L) {
			// stack has userdata, followed by method args
			T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
			lua_remove(L, 1);  // remove self so member function args start at index 1
			// get member function from upvalue
			RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
			//return (obj->*(l->mfunc))(L);  // call member function
			return l->mfunc(L,obj);
		}

		// garbage collection metamethod
		static int gc_T(lua_State *L) 
		{
			T * obj = check(L,1);
			if(obj == NULL)
				return 0;
			lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH");
			if(lua_istable(L,-1) )
			{
				char name[32];
				tostring(name,obj);
				lua_getfield(L,-1,string(name).c_str());
				if(lua_isnil(L,-1) )
				{
					delete obj;
					obj = NULL;
				}
			}
			lua_pop(L,3);
			return 0;
		}
		static int tostring_T (lua_State *L) {
			char buff[32];
			T ** ptrHold = (T**)lua_touserdata(L,1);
			T *obj = *ptrHold;
			sprintf(buff, "%p", obj);
			lua_pushfstring(L, "%s (%s)", GetTClassName<T>(), buff);
			return 1;
		}
		ARCEMU_INLINE static void tostring(char * buff,void * obj)
		{
			sprintf(buff,"%p",obj);
		}
	};
	class GUID_MGR 
	{
		static const char * GetName() { return "WoWGUID"; }
	public:
		static void Register(lua_State * L) {
			luaL_newmetatable(L,GetName());
			int mt = lua_gettop(L);
			//Hide metatable.
			lua_pushnil(L);
			lua_setfield(L,mt,"__metatable");
			//nil gc method
			lua_pushnil(L);
			lua_setfield(L,mt,"__gc");
			//set our tostring method
			lua_pushcfunction(L,_tostring);
			lua_setfield(L,mt,"__tostring");
			//nil __index field
			lua_pushnil(L);
			lua_setfield(L,mt,"__index");
			//set __newindex method
			lua_pushcfunction(L,_newindex);
			lua_setfield(L,mt,"__newindex");
			//no call method
			lua_pushnil(L);
			lua_setfield(L,mt,"__call");
		}
		static uint64 check(lua_State * L, int narg) 
		{
			uint64 GUID = 0;
			uint64 * ptrHold = (uint64*)lua_touserdata(L,narg);
			if(ptrHold != NULL)
				GUID = *ptrHold;
			return GUID;
		}
		static int push(lua_State *L, uint64 guid)
		{
			int index = 0;
			if(guid == 0) 
			{
				lua_pushnil(L);
				index = lua_gettop(L);
			}
			else
			{
				luaL_getmetatable(L,GetName());
				if(lua_isnoneornil(L,-1) )
					luaL_error(L,"%s metatable not found!. \n",GetName());
				else 
				{
					int mt = lua_gettop(L);
					uint64* guidHold = (uint64*)lua_newuserdata(L,sizeof(uint64));
					int ud = lua_gettop(L);
					if(guidHold == NULL)
						luaL_error(L,"Lua tried to allocate size %d of memory and failed! \n",sizeof(uint64*));
					else
					{
						(*guidHold) = guid;
						lua_pushvalue(L,mt);
						lua_setmetatable(L,ud);
						lua_replace(L,mt);
						lua_settop(L,mt);
						index = mt;
					}
				}
			}
			return index;
		}
	private:
		GUID_MGR() {}
		//This method prints formats the GUID in hexform and pushes to the stack.
		static int _tostring(lua_State * L) 
		{
			uint64 GUID = GUID_MGR::check(L,1);
			if(GUID == 0)
				lua_pushnil(L);
			else {
				char buff[32];
				sprintf(buff,"%X",GUID);
				lua_pushfstring(L,"%s",buff);
			}
			return 1;
		}
		static int _newindex(lua_State *L) 
		{
			//Remove table, key, and value
			lua_remove(L,1);
			lua_remove(L,1);
			lua_remove(L,1);
			luaL_error(L,"OPERATION PROHIBITED!\n");
			return 0;
		}
	};
};
#endif



