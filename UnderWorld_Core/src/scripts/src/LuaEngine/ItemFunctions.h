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

#ifndef ITEMFUNCTIONS_H
#define ITEMFUNCTIONS_H

#include "StdAfx.h"
#include "ItemFunctions.h"

namespace luaItem
{
	///////GOSSIP///////
	int GossipCreateMenu(lua_State * L, Item * ptr)
	{
		int text_id = luaL_checkint(L, 1);
		Player * plr = CHECK_PLAYER(L,2);
 		int autosend = luaL_checkint(L, 3);
		objmgr.CreateGossipMenuForPlayer(&Menu, ptr->GetGUID(), text_id, plr);
		if(autosend)
			Menu->SendTo(plr);
		return 1;
	}

	int GossipMenuAddItem(lua_State * L, Item * ptr)
	{
		int icon = luaL_checkint(L, 1);
		const char * menu_text = luaL_checkstring(L, 2);
		int IntId = luaL_checkint(L, 3);
		int extra = luaL_checkint(L, 4);

		Menu->AddItem(icon, menu_text, IntId, extra);
		return 1;
	}

	int GossipSendMenu(lua_State * L, Item * ptr)
	{
		Player * plr = CHECK_PLAYER(L,1);
		Menu->SendTo(plr);
		return 1;
	}

	int GossipComplete(lua_State * L, Item * ptr)
	{
		Player * plr = CHECK_PLAYER(L,1);
		plr->Gossip_Complete();
		return 1;
	}

	int GossipSendPOI(lua_State * L, Item * ptr)
	{
		Player * plr = CHECK_PLAYER(L,1);
		float x = CHECK_FLOAT(L, 2);
		float y = CHECK_FLOAT(L, 3);
		int icon = luaL_checkint(L, 4);
		int flags = luaL_checkint(L, 5);
		int data = luaL_checkint(L, 6);
		const char * name = luaL_checkstring(L, 7);

		plr->Gossip_SendPOI(x, y, icon, flags, data, name);
		return 1;
	}

	int PerformWorldDBQuery(lua_State * L, Item* ptr)
	{
		const char * qStr = luaL_checkstring(L,1);
		if(!qStr) return 0;
		if (!Config.OptionalConfig.GetBoolDefault("LUAppArc", "AllowWorldDBQueries", false)) {
			lua_pushnumber(L, (lua_Number)0); //0=Disabled
			return 1;
		}
		QueryResult * result = WorldDatabase.Query(qStr);
		if (result) {
			lua_pushnumber(L, (lua_Number)1); //1=GotData
		} else lua_pushnumber(L, (lua_Number)2); //2=NoData
		return 1;
	}

	int PerformCharDBQuery(lua_State * L, Item* ptr)
	{
		const char * qStr = luaL_checkstring(L,1);
		if(!qStr) return 0;
		if (!Config.OptionalConfig.GetBoolDefault("LUAppArc", "AllowCharDBQueries", false)) {
			lua_pushnumber(L, (lua_Number)0); //0=Disabled
			return 1;
		}
		QueryResult * result = CharacterDatabase.Query(qStr);
		if (result) {
			lua_pushnumber(L, (lua_Number)1); //1=GotData
		} else lua_pushnumber(L, (lua_Number)2); //2=NoData
		return 1;
	}

	int GetOwner(lua_State * L, Item* ptr)
	{
		Player * owner = ptr->GetOwner();
		if(owner != NULL)
			PUSH_UNIT(L,owner);
		else
			lua_pushnil(L);
		return 1;
	}

	int AddEnchantment(lua_State * L, Item* ptr)
	{
		int entry = luaL_checkint(L,1);
		int duration = luaL_checkint(L,2);
		bool permanent = (duration == 0) ? true : false;
		bool temp = (luaL_checkint(L, 3) == 1) ? true : false;

		EnchantEntry *eentry = dbcEnchant.LookupEntry( entry );

		lua_pushinteger(L, ptr->AddEnchantment(eentry, duration, permanent, true, temp)); //Return the enchantment Slot back to LUA
		return 1;
	}

	int GetGUID(lua_State * L, Item* ptr)
	{
		PUSH_GUID(L,ptr->GetGUID());
		return 1;
	}

	int RemoveEnchantment(lua_State * L, Item* ptr)
	{
		int slot = luaL_checkint(L,1);
		bool temp = (luaL_IsInt(L,2)==1) ? true : false;

		if (slot == -1)	ptr->RemoveAllEnchantments(temp);
		else if (slot == -2) ptr->RemoveProfessionEnchant();
		else if (slot == -3) ptr->RemoveSocketBonusEnchant();
		else if (slot >= 0) ptr->RemoveEnchantment(slot);

		return 0;
	}

	int GetEntryId(lua_State * L, Item* ptr)
	{
		if (!ptr) return 0;
		ItemPrototype * proto = ptr->GetProto();
		lua_pushnumber(L, proto->ItemId);
		return 1;
	}

	int GetName(lua_State * L, Item* ptr)
	{
		if (!ptr) 
			return 0;
		ItemPrototype * proto = ptr->GetProto();
		lua_pushstring(L, proto->Name1);
		return 1;
	}

	int GetSpellId(lua_State * L, Item* ptr)
	{
		uint32 index = luaL_checkint(L, 1);
		if (!ptr || index < 0 || index > 5)
			return 0;
		ItemPrototype * proto = ptr->GetProto();
		lua_pushnumber(L, proto->Spells[index].Id);
		return 1;
	}

	int GetSpellTrigger(lua_State * L, Item* ptr)
	{
		uint32 index = luaL_checkint(L, 1);
		if (!ptr || index < 0 || index > 5)
			return 0;
		ItemPrototype * proto = ptr->GetProto();
		lua_pushnumber(L, proto->Spells[index].Trigger);
		/*	
			USE				= 0,
			ON_EQUIP		= 1,
			CHANCE_ON_HIT	= 2,
			SOULSTONE		= 4,
			LEARNING		= 6,
		*/
		return 1;
	}

	int AddLoot(lua_State * L, Item* ptr)
	{
		//CHECK_TYPEID(TYPEID_UNIT);
		uint32 itemid = luaL_checkint(L,1);
		uint32 mincount = luaL_checkint(L,2);
		uint32 maxcount = luaL_checkint(L,3);
		uint32 ffa_loot = luaL_checkint(L,4);
		lootmgr.AddLoot(ptr->loot,itemid,mincount,maxcount,ffa_loot);
		return 1;
	}

	int GetItemLink(lua_State * L, Item * ptr)
	{
		uint32 lang = luaL_optint(L, 1, LANG_UNIVERSAL);
		if(!ptr)
			return 0;
		lua_pushstring(L, ptr->GetItemLink(lang).c_str());
		return 1;
	}
	int SetByteValue(lua_State * L, Item * ptr)
	{
		uint32 index = luaL_checkint(L,1);
		uint32 index1 = luaL_checkint(L,2);
		uint8 value = luaL_checkint(L,3);
		ptr->SetByte(index,index1,value);
		return 1;
	}

	int GetByteValue(lua_State * L, Item * ptr)
	{
		uint32 index = luaL_checkint(L,1);
		uint32 index1 = luaL_checkint(L,2);
		lua_pushinteger(L,ptr->GetByte(index,index1));
		return 1;
	}

	int GetItemLevel(lua_State * L, Item * ptr)
	{
		lua_pushnumber(L, ptr->GetProto()->ItemLevel);
		return 1;
	}

	int GetRequiredLevel(lua_State * L, Item * ptr)
	{
		lua_pushnumber(L, ptr->GetProto()->RequiredLevel);
		return 1;
	}

	int GetBuyPrice(lua_State * L, Item * ptr)
	{
		lua_pushnumber(L, ptr->GetProto()->BuyPrice);
		return 1;
	}

	int GetSellPrice(lua_State * L, Item * ptr)
	{
		lua_pushnumber(L, ptr->GetProto()->SellPrice);
		return 1;
	}

	int RepairItem(lua_State * L, Item * ptr)
	{
		if(!ptr)
			return 0;
		ptr->SetDurabilityToMax();
		return 1;
	}

	int GetMaxDurability(lua_State * L, Item * ptr)
	{
		if(!ptr)
			return 0;
		lua_pushnumber(L, ptr->GetDurabilityMax());
		return 1;
	}

	int GetDurability(lua_State * L, Item * ptr)
	{
		if(!ptr)
			return 0;
		lua_pushnumber(L, ptr->GetDurability());
		return 1;
	}

	int HasEnchantment(lua_State * L, Item * ptr)
	{
		if(!ptr)
			return 0;
		if(ptr->HasEnchantments())
			lua_pushboolean(L, 1);
		else
			lua_pushboolean(L, 0);
		return 1;
	}

	int ModifyEnchantmentTime(lua_State * L, Item * ptr)
	{
		uint32 slot = luaL_checkint(L, 1);
		uint32 duration = luaL_checkint(L, 2);
		if(!ptr)
			return 0;
		ptr->ModifyEnchantmentTime(slot, duration);
		return 1;
	}

	int SetStackCount(lua_State * L, Item * ptr)
	{
		uint32 count = luaL_checkint(L, 1);
		if(!ptr || !count || count > 1000)
			return 0;
		ptr->SetStackCount(count);
		return 1;
	}

	int HasFlag(lua_State * L, Item * ptr)
	{
		uint32 index = luaL_checkint(L,1);
		uint32 flag = luaL_checkint(L,2);
		lua_pushboolean(L, ptr->HasFlag(index,flag) ? 1 : 0);
		return 1;
	}

	int IsSoulbound(lua_State * L, Item * ptr)
	{
		ptr->IsSoulbound() ? lua_pushboolean(L,1) : lua_pushboolean(L,0);
		return 1;
	}

	int IsAccountbound(lua_State * L, Item * ptr)
	{
		ptr->IsAccountbound() ? lua_pushboolean(L,1) : lua_pushboolean(L,0);
		return 1;
	}

	int IsContainer(lua_State * L, Item * ptr)
	{
		ptr->IsContainer() ? lua_pushboolean(L,1) : lua_pushboolean(L,0);
		return 1;
	}

	int GetContainerItemCount(lua_State * L, Item * ptr)
	{
		uint32 itemid = CHECK_ULONG(L,1);
		if (!ptr->IsContainer() || !itemid) return 0;
		Container * pCont = static_cast<Container*>(ptr);
		int16 TotalSlots = static_cast<int16>(pCont->GetNumSlots());
		int cnt = 0;
		for (int16 i = 0; i < TotalSlots; i++)
		{
			Item *item = pCont->GetItem(i);
			if (item)
			{
				if(item->GetEntry() == itemid && item->wrapped_item_id == 0)
				{
					cnt += item->GetStackCount() ? item->GetStackCount() : 1; 
				}
			}
		}
		lua_pushinteger(L, cnt);
		return 1;
	}

	int GetEquippedSlot(lua_State * L, Item * ptr)
	{
		if (!ptr) return 0;
		lua_pushinteger(L, ptr->GetOwner()->GetItemInterface()->GetInventorySlotById(ptr->GetEntry()));
		return 1;
	}

	int GetObjectType(lua_State * L, Item * ptr)
	{
		if (!ptr) { lua_pushnil(L); return 1; }
		lua_pushstring(L, "Item");
		return 1;
	}
}
#endif
