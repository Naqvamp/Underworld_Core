/*
 *	Old version based off of names
 */


/*
    Voter Exchange NPC
    
    This Npc will exchange a set list of items in for another item based on the chosen cost of the item.
	The NPC will also exchange spells as well based off of the loaded spells
    The exchange rate is defaulted to 35% but is easily changeable.

	Notes: Spells - Returning racials. free souls for those who start with them. ie Shadowmeld on Night Elves
	Could implement a system to limit returns based on race and or class. Would end up with a vector<uint32> 
	per voter. 2 if races and classes were both checked


    Copyright (C) <2009>  <Christopher Turcotte>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    I can be contacted at syrathia@hotmail.com with any questions
    regarding this software.
//*/


//    Queries (moved to accompanying SQL files)

#include "StdAfx.h"
#include "Setup.h"

#ifdef WIN32
#pragma warning(disable:4305)        // warning C4305: 'argument' : truncation from 'double' to 'float'
#endif

#define EXCHANGE_RATE .35 //Percentage converted to decimal 35% of the cost is number of items returned would be .35



//for debug output
//outputs a string (msg)
#define DEBUGS(msg) sChatHandler.RedSystemMessage(plr->GetSession(), "%s", msg);
//outputs string (msg): uint (num)
#define DEBUGN(msg,num) sChatHandler.RedSystemMessage(plr->GetSession(), "%s %u", msg, num);

//--------------------------------
//This is a general container for voter info
struct VoterData
{
//	uitn32 npc_id; //Dont need to hold this. 
	uint32 id;
//	uint32 type; //Will be one vector per type
	uint32 cost;
	uint32 unallowedclass;
	uint32 unallowedrace;
};

//intIds
//1 = main menu
//2 = close gossip
//3 = handle item exchange
//4 = handle spell exchange
//5 = Admin menu
//6 = Reload Price data
//7 = Update item entries


class SCRIPT_DECL VoterReturnNPC : public GossipScript
{
private:
	enum VoterType
	{
		VOTER_TYPE_ITEM = 0,
		VOTER_TYPE_SPELL = 1,
//		VOTER_TYPE_OTHER = 2
	};

	vector<VoterData*> items;
	vector<VoterData*> spells; 
	
	uint32 npcID;

	bool HandleExchange(Object * pObject, Player * plr, VoterType vType, uint32 vID);
	int32 ParsePlayerInput(Player * plr, const char * code, VoterType vType);
	uint32 GetItemCost(uint32 id);
	void LogReturn(Player * plr, char notes[1024], uint32 retAmt);

	//admin section
	void LoadData();
	void UpdateItemEntries(Player * plr);
	

public:
	VoterReturnNPC(uint32 id): npcID(id)	{ LoadData(); }
	~VoterReturnNPC() {}
	void Destroy() { delete this; }

	void GossipHello(Object* pObject, Player * plr, bool AutoSend);
	void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 intId, const char * code);	
	void GossipEnd(Object* pObject, Player * plr);
};


void VoterReturnNPC::GossipHello(Object* pObject, Player * plr, bool AutoSend)
{
	GossipMenu *Menu;
	objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30030, plr);
	bool isAny = false; //tracks if any menu items are added		

	if (items.size() > 0)
	{
		isAny = true;
		Menu->AddItemBox(3, "Item Name", 3, "Type in the name exactly", 0, true);
	}

	if (spells.size() > 0)
	{
		isAny = true;
		Menu->AddItemBox(3, "Spell Name", 4, "Type in the name exactly", 0, true);
	}

	if (plr->GetSession()->m_gmData->rank >= RANK_ADMIN)
	{
		isAny = true;
		Menu->AddItem(7, "Admin Menu", 5);
	}

	//do class and skill checks here {future work}
    

	if (isAny) 
	{
		Menu->AddItem(4, "[Close]", 2);
		if(AutoSend)
		    Menu->SendTo(plr);
	}
	else //No spells/items loaded from the DB.
	{
		static_cast<Unit*>(pObject)->SendChatMessageToPlayer(15, 0, "Begone! You do not have anything that I am willing to accept.", plr);
		GossipEnd(pObject, plr);
	}
}

void VoterReturnNPC::GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 intId, const char * code)
{
	switch(intId)
	{
	case 1:     // Return to start
		GossipHello(pObject, plr, true);
		break;

	case 2:		//close
		GossipEnd(pObject, plr);
		break;

	case 3: //handle item return
		{	
			int32 id = ParsePlayerInput(plr, code, VOTER_TYPE_ITEM);
			if (id > -1)
			{
				HandleExchange(pObject, plr, VOTER_TYPE_ITEM, id);
			}
			else
			{
				static_cast<Unit*>(pObject)->SendChatMessageToPlayer(15, 0, "The item name you entered is not a valid item.", plr);
			}
			GossipEnd(pObject, plr);
			break;
		}
	case 4: //Handle spell return
		{
			int32 id = ParsePlayerInput(plr, code, VOTER_TYPE_SPELL);
			if (id > 1)
			{
				HandleExchange(pObject, plr, VOTER_TYPE_SPELL, id);
			}
			else
			{
				static_cast<Unit*>(pObject)->SendChatMessageToPlayer(15, 0, "The spell name you entered is not a valid spell.", plr);
			}
			GossipEnd(pObject, plr);
			break;
		}

		
	case 5: //admin menu
		{
			GossipMenu *Menu;
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 30031, plr);
			 
			Menu->AddItem(4, "TOOL: Reload Price Table.", 6);
			Menu->AddItem(4, "TOOL: Update Voter Item Entries.", 7);

			Menu->SendTo(plr);

			break;
		}

	case 7: //update table with non existant itemids
		UpdateItemEntries(plr);
		sChatHandler.GreenSystemMessage(plr->GetSession(), "VoterExchange: Voter item entries updated.");
		//fall through to reload the table

	case 6: //reload the data from the table
		LoadData();
		sChatHandler.GreenSystemMessage(plr->GetSession(), "VoterExchange: Price data reloaded.");

		GossipHello(pObject, plr, true);

	default: //You did something wrong to get here
		GossipEnd(pObject, plr);
		break;
    }
}


void VoterReturnNPC::GossipEnd(Object* pObject, Player * plr)
{
	GossipScript::GossipEnd(pObject, plr); //call base classs
	plr->Gossip_Complete();
}

//----------------------------
//handles the exchanging process
bool VoterReturnNPC::HandleExchange(Object * pObject, Player * plr, VoterType vType, uint32 vID)
{	
	Unit * npcUnit = static_cast<Unit*>(pObject);
	if (plr->bGMTagOn)//dont perform the exchange if they are tagged. They will lose the item but won't receive any due to in place checks
	{ //looked into this and bug where it flags user as duping seems to be gone but better safe than sorry
		npcUnit->SendChatMessageToPlayer(15, 0, "I do not work with you and your 'GM' magic!", plr);
		return false;
	}

	uint32 returnAmt; 
	ItemPrototype * voterIt;
	SpellEntry * voterSp;

	switch (vType)
	{
	case VOTER_TYPE_ITEM:
		if (items[vID]->cost < 1)
		{
			npcUnit->SendChatMessageToPlayer(15, 0, "My boss gave me faulty data. Please report the item you are tying to give me to Syrathia.", plr);
			return false;
		}

		voterIt  = ItemPrototypeStorage.LookupEntry(items[vID]->id); //item being taken
		returnAmt = (uint32) ceil(items[vID]->cost * EXCHANGE_RATE);
		break;

	case VOTER_TYPE_SPELL:
		if (spells[vID]->cost < 1)
		{
			npcUnit->SendChatMessageToPlayer(15, 0, "My boss gave me faulty data. Please report the spell you are tying to give me to Syrathia.", plr);
			return false;
		}

		voterSp = dbcSpell.LookupEntry(spells[vID]->id);
		returnAmt = (uint32) ceil(spells[vID]->cost * EXCHANGE_RATE);
		break;
	}

	if (returnAmt < 1)
	{
		npcUnit->SendChatMessageToPlayer(15, 0, "My boss gave me faulty data. Please report the voter you are tying to give me to Syrathia.", plr);
		return false;
	}


	switch (vType) 
	{
	case VOTER_TYPE_ITEM:
		if (plr->GetItemInterface()->RemoveItemAmt(voterIt->ItemId, 1) == 1) //will fail if the player does not have the item or somehow removed something other than just one
		{
			char notes[1024];
			snprintf(notes, 1024, "item exchange %s (id: %u)", voterIt->Name1, vID);
			LogReturn(plr, notes, returnAmt);
			plr->GetSession()->AssignPoints(returnAmt);
			npcUnit->SendChatMessageToPlayer(15, 0, "Your items will be put to good use.", plr);

		}
		else
		{
			npcUnit->SendChatMessage(14, 0, "Do not toy with me you foolish mortal!");
			plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
			plr->KillPlayer();
			return false;
		}
		break;
	
	case VOTER_TYPE_SPELL:
		if (plr->removeSpell(voterSp->Id, false, false, 0)) //will fail if the player does not have the spell
		{
			char notes[1024];
			snprintf(notes, 1024, "spell exchange %s (id: %u)", voterSp->Name, vID);
			LogReturn(plr, notes, returnAmt);
			plr->GetSession()->AssignPoints(returnAmt);
			npcUnit->SendChatMessageToPlayer(15, 0, "Your magical energies will be put to good use.", plr);
		}	
		else
		{
			npcUnit->SendChatMessage(14, 0, "Do not toy with me you foolish mortal!");
			plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
			plr->KillPlayer();
			return false;
		}
	}
	
	return true; //if we haven't returned yet all went well
}
//----------------------------
//Gets the item cost based on ItemHandler code
uint32 VoterReturnNPC::GetItemCost(uint32 id)
{
	if (!id)
		return 1;

	uint32 pCost = 1;
	uint32 entry = 0;
		
	QueryResult * res = WorldDatabase.Query("SELECT entry FROM `vendors` WHERE item=%u ORDER BY entry ASC LIMIT 1", id);
	if (res)
		entry = res->Fetch()[0].GetUInt32();

	delete res;

	if(entry > 0)
	{
		if(entry <= 59019)	
			pCost = 15;
		else if(entry <= 59039)
			pCost = 25;
		else if(entry <= 59059)
			pCost = 50;
		else if(entry <= 59079)
			pCost = 60;
		else if(entry <= 59099)
			pCost = 75;
		else if(entry <= 59119)
			pCost = 100;
		else if(entry <= 59139)
			pCost = 125;
		else if(entry <= 59159)
			pCost = 150;
		else if(entry <= 59179)
			pCost = 175;
		else if(entry <= 59199)
			pCost = 200;
		else if(entry <= 59219)
			pCost = 225;
		else if(entry <= 59239)
			pCost = 250;
		else if(entry <= 59259)
			pCost = 275;
		else if(entry <= 59279)
			pCost = 300;
		else if(entry <= 59299)
			pCost = 325;
		else if(entry <= 59319)
			pCost = 350;
		else if(entry <= 59339)
			pCost = 375;
		else if(entry <= 59359)
			pCost = 400;
		else if(entry <= 59379)
			pCost = 425;
		else if(entry <= 59399)
			pCost = 450;
		else if(entry <= 59419)
			pCost = 475;
		else if(entry <= 59439)
			pCost = 500;
	}

	return pCost;
}


//----------------------------
//Returns the id of passed code
int32 VoterReturnNPC::ParsePlayerInput(Player * plr, const char * code, VoterType vType)
{
	int32 id = -1;
	if (code != NULL)
	{
		bool found = false;//short circuit for loops

		//get ID of passed name as a spell/item
		switch (vType)
		{
		case VOTER_TYPE_ITEM: 
			ItemPrototype * it;
			
			for (uint32 ii = 0; ii < items.size() && !found; ii++)
			{
				it = ItemPrototypeStorage.LookupEntry(items[ii]->id);
				if (it)
				{
					if (stricmp(it->Name1, code) == 0)
					{
						id = ii;
						found = true;
					}
				}
				else
					Log.Error("VoterExchange", "ItemId %u does not exist.", items[ii]->id);

			}	
		
			break;
		
		case VOTER_TYPE_SPELL:
			SpellEntry * se;
			
			for (uint32 ii = 0; ii < spells.size() && !found; ii++)
			{
				se = dbcSpell.LookupEntry(spells[ii]->id);
				if (se)
				{
					if (se->Name)
					{
						if (stricmp(se->Name, code) == 0)
						{
							id = ii;
							found = true;
						}
					}
					else
					{
						Log.Error("VoterExchange", "Spell [%u] does not have a name. Cancelling transaction.", spells[ii]->id);
						sChatHandler.SystemMessageToPlr(plr, "That spell is missing data. Report the Syrathia. ID [%u]", spells[ii]->id);
					}

				}
				else
					Log.Error("VoterExchange", "SpellId %u does not exist.", spells[ii]->id);
			}
		
			break;

		default:
			//should never reach this
			break;
		}
	}//code != NULL
	return id;

}


//******************************************
//***		Admin Menu Functions
//******************************************

//----------------------------
//Checks for entries in voter vendors and adds them to the proper table
void VoterReturnNPC::UpdateItemEntries(Player * plr)
{
	QueryResult * res = WorldDatabase.Query("SELECT item FROM vendors WHERE entry >= 59000 AND entry <= 59999 AND item NOT IN (SELECT voter_id FROM _voter_return_npc WHERE voter_type = 0 AND npc_id = %u ) GROUP BY item", npcID);

	if (res)
	{
		do 
		{
			WorldDatabase.Execute("INSERT INTO _voter_return_npc VALUES (%u, %u, 0, 0, 0, 0)", npcID, res->Fetch()[0].GetUInt32());
			plr->BroadcastMessage("ItemId [%u] added to this vendor [id: %u].", res->Fetch()[0].GetUInt32(), npcID);
		} while (res->NextRow());

		delete res;
	}
}

//----------------------------
//Loads voter data from the DB
void VoterReturnNPC::LoadData()
{
	items.clear();
	spells.clear();

	QueryResult * res = WorldDatabase.Query("SELECT * FROM _voter_return_npc WHERE npc_id = %u;", npcID);
	if (res)
	{
		do
		{
			VoterData * temp = new VoterData();

			switch (res->Fetch()[2].GetUInt32()) 
			{
			case VOTER_TYPE_ITEM:
				temp->id = res->Fetch()[1].GetUInt32();
				temp->cost = (res->Fetch()[3].GetUInt32() ? res->Fetch()[1].GetUInt32() : GetItemCost(res->Fetch()[1].GetUInt32()));
				temp->unallowedclass = res->Fetch()[4].GetUInt32();
				temp->unallowedrace = res->Fetch()[5].GetUInt32();

				items.push_back(temp);
				break;

			case VOTER_TYPE_SPELL:
				temp->id = res->Fetch()[1].GetUInt32();
				temp->cost = (res->Fetch()[3].GetUInt32() ? res->Fetch()[1].GetUInt32() : 1); //cost of 1.. heh
				temp->unallowedclass = res->Fetch()[4].GetUInt32();
				temp->unallowedrace = res->Fetch()[5].GetUInt32();

				spells.push_back(temp);
				break;

			default:
				Log.Error("VoterExchange", "Does not support voter returns other than items and spells.");
				break;
			}

		} while (res->NextRow());
	}
	else
	{
		Log.Error("VoterExchange", "ID %u is not a valid ID!", npcID);
	}
	delete res;
}

//-----------------------------
//Cause I like logging everything here's to track the players exchanges
void VoterReturnNPC::LogReturn(Player * plr, char notes[1024], uint32 retAmt)
{
	WorldDatabase.Execute("INSERT INTO _voter_exchange_log (`acct`, `login`, `guid`, `name`, `gm`, `points`, `return`, `notes`, `timestamp`) VALUES (%u, '%s', %u, '%s', '%s', %u, %u, '%s', NOW())", 
		plr->GetSession()->GetAccountId(), 
		plr->GetSession()->GetAccountNameS(), 
		plr->GetLowGUID(), 
		plr->GetName(), 
		plr->GetSession()->GetPermissions(), 
		plr->GetSession()->m_points,
		retAmt,
		WorldDatabase.EscapeString(string(notes)).c_str()
	);
}

void SetupVoterExchange(ScriptMgr * mgr)
{
    GossipScript * voter = (GossipScript*) new VoterReturnNPC(550000);
    mgr->register_gossip_script(550000, voter);
} 