///*
// *	Donator.cpp - Chris Turcotte, Adam Marculaitis (c) 2010
// * Catergory => Subcatergory => entry => {cost, name, (misc)}
// *
// * * * * Icons * * * *
// * 0 = chat bubble
// * 1 = 
// * 2 = wing
// * 3 = trainer book
// * 4 = gear
// * 5 = gear
// * 6 = Bag w/ coin
// * 7 = chat bubble w/ elippsis
// * 8 = 
// * 9 = crossed swords
// * 10 = 
// * 11 = 
// * 12 = chat bubble
// * 
// * * * * IntId Ranges * * * *
// * 1-5 Meuns+Std functions
// * 100-199	Free Services
// * 200-299	GM Commands
// * 
// * 1000	Morphs
// * 2000	Rep
// * 3000	Spells
// * 4000	Skills
// * 5000	Announce
// * 6000	Music
// * 7000	Buffs
// * 
// * 
// */
//
//#include "StdAfx.h"
//#include "Setup.h"
//
//#define DEBUG(msg) sChatHandler.RedSystemMessage(plr->GetSession(), msg);
//
//
//class SCRIPT_DECL Donator2 : public GossipScript
//{
//private:
//	CategoryMap Morphs;	
//	CategoryMap Spells;	
//	CategoryMap Reputations;	
//	CategoryMap Skills;	
//	CategoryMap Announcements;	
//	CategoryMap Music;	
//	CategoryMap Buffs;	
//
//	//Track needed data about where the player is in the menus.
//	map<uint32, DonatorSession*> sessions;
//
//	void LoadData();
//
//	uint32 npcEntry;
//public:
//	Donator2(uint32 entry) : npcEntry(entry) { LoadData(); }
//
//	void GossipHello(Object * pObject, Player* plr, bool AutoSend);
//	void GossipSelectOption(Object * pObject, Player* plr, uint32 Id, uint32 IntId, const char * Code);
//
//	void GossipEnd(Object * pObject, Player* plr)
//	{
//		plr->Gossip_Complete();
//		GossipScript::GossipEnd(pObject, plr);
//	}
//	void Destroy()
//	{
//		delete this;
//	}
//};
//
//void Donator2::GossipHello(Object * pObject, Player* plr, bool AutoSend)
//{
//	GossipMenu * Menu;
//	objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, plr);
//	Menu->AddItem(6, "Points Menu", 3);
//	Menu->AddItem(7, "Free Services", 4);
//
//	char msg[256];
//	snprintf(msg, 256, "You have %u points. Click to reload them.", plr->GetSession()->m_points);
//	Menu->AddItem(6, msg, 5);
//
//	if (AutoSend)
//		Menu->SendTo(plr);
//}
//
///* * Parameters * *
// *	pObject	=	Object that the player is interacting with
// *	plr		=	Player that is the player interacting with the object
// *	Id		=	the 0 indexed id of the option clicked 
// *	IntId	=	the internal id attached to the gossip items uint32(-1) by default
// *	Code	=	the string the player enters into the box that pops up if used NULL by default
// */
//void Donator2::GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char *Code)
//{
//	DonatorSession * sess = sessions[plr->GetLowGUID()];
//
//	GossipMenu * Menu;
//	objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, plr);
//
//	switch (IntId)
//	{
//	case 1: // Return to start			
//		GossipHello(pObject, plr, true);
//		return;
//
//	case 2:		//close
//		GossipEnd(pObject, plr);
//		return;
//	
//	case 5:
//		plr->GetSession()->LoadPoints();
//		if (pObject->IsCreature())
//		{
//			char msg[256];
//			snprintf(msg, 256, "You now have %u points.", plr->GetSession()->m_points);
//			static_cast<Creature*>(pObject)->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, 0, msg, plr);
//		}
//		break;
//
//		//Free services (100-199)
//		//	Add new things
//		//	Resetting spells and talents bug talents
//	case 4:		
//		if(sWorld.dark_light_enabled)
//			Menu->AddItem(3, "TELEPORT: Point Mall", 100);
//
//		Menu->AddItem(3, "Unstuck Account", 101);
//		Menu->AddItem(3, "Reset Spells and Talents", 103);
//		Menu->AddItem(3, "Return Talent Points", 105);
//		
//		Menu->AddItem(12, "[BACK]", 1); 
//		Menu->SendTo(plr);
//		break;
//
//	case 100: //Point mall teleport
//		plr->SafeTeleport(530, 0, 9511.135F, -7899.663F, 14.667F, 1.965069F);
//		break;
//
//	case 101:	//unstuck account
//		Menu->AddItem(3, "Yes, do it.", 402);
//		Menu->AddItem(12, "[BACK]", 4); 
//		Menu->SendTo(plr);
//		break;
//
//	case 102:
//		plr->UnstuckAccount();
//		GossipEnd(pObject, plr);
//		break;
//
//	case 103:	//reset talents and spells
//		Menu->AddItem(3, "Yes, do it.", 404);
//		Menu->AddItem(12, "[BACK]", 4); 
//		Menu->SendTo(plr);
//		break;
//
//	case 104:
//		plr->LearnItAllUW();
//		GossipEnd(pObject, plr);
//		break;
//
//	case 105:
//		plr->Reset_Talents();
//		GossipEnd(pObject, plr);
//		break;
//
//
//	case 3: //Main points services menu
//				
//		//Erase the players session. The session only matters in this section so let's clear it here
//		sessions[plr->GetLowGUID()] = new DonatorSession();
//
//		Menu->AddItem(7, "GM Commands", 200);
//		
//		Menu->AddItem(5, "Morphs", 50);
//		Menu->AddItem(2, "Spells", 51);
//		Menu->AddItem(3, "Reputation", 52);
//		Menu->AddItem(4, "Skill Advancement", 53);
//		Menu->AddItem(12, "Realm Announcements", 54);
//		Menu->AddItem(7, "Realm Music", 55);
//		Menu->AddItem(9, "Realm Buffs", 56);
//			
//		Menu->AddItem(4, "RENAME ME", 300);
//		Menu->AddItem(12, "[BACK]", 1);
//		Menu->SendTo(plr);
//		
//		break;
//
//	case 50: //morphs
//		{
//
//		//this means we have been here before. We have selected the sub category.
//		//Check against null instead of comparing because it's caster and always NULLed in the menu before this one
//		if (sess->catergory != NULL)
//		{
//			//If there is no saved sub category then this is the first visit after choosing. Lets save it
//			if (sess->subcatergory == NULL)
//			{
//				CategoryMap::iterator itr;
//				uint32 ii = 0;
//
//				for (itr = Morphs.begin(); itr != Morphs.end() && ii < Id; itr++, ii++);
//
//				sess->subcatergory = itr->first;
//				sess->page = 0;
//			}
//			else
//			{
//				sess->page++;
//			}
//
//			//display the options
//			DataVector temp = Morphs.find(sess->subcatergory)->second;
//			//multiple pagesAA
//			if (temp.size() > 14)
//			{
//				uint32 start = sess->page * 13;
//				uint32 stop = (start + 13 > temp.size() ? temp.size() : start + 13);
//				for(uint32 ii = start; ii < stop; ii++)
//				{
//					Menu->AddItem(5, temp[ii]->name, 1000+ii);
//				} 
//				Menu->AddItem(4, "Next page", 50);
//				Menu->AddItem(12, "[BACK]", 3);
//			}
//			//single page
//			else
//			{
//				for(uint32 ii = 0; ii < temp.size(); ii++)
//				{
//					Menu->AddItem(5, temp[ii]->name, 1000+ii);
//				}
//
//				Menu->AddItem(12, "[BACK]", 3);
//				Menu->SendTo(plr);		
//			}
//			break;
//		}
//
//		//This is pre sub category selection. We are on the subcategory menu.
//		sess->catergory = &Morphs;
//	
//		CategoryMap::iterator itr;
//
//		for (itr = Morphs.begin(); itr != Morphs.end(); itr++)
//		{
//			Menu->AddItem(5, itr->first, 50);
//		}
//				
//		Menu->AddItem(12, "[BACK]", 3);
//		Menu->SendTo(plr);		
//		}
//		break;
//
//		/* * * * * * * * *
//		 * 1000	Morphs		
//		 * 2000	Rep
//		 * 3000	Spells
//		 * 4000	Skills
//		 * 5000	Announce	
//		 * 6000	Music
//		 * 7000	Buffs
//		 * * * * * * * * */
//	default:
//		/*
//		if (IntId >= 7000)
//
//		else if (IntId >= 6000)
//
//		else if (IntId >= 5000)
//
//		else if (IntId >= 4000)
//
//		else if (IntId >= 3000)
//
//		else if (IntId >= 2000)
//
//		else if (IntId >= 1000)
//		*/
//		break;
//	}	
//}
//
//void Donator2::LoadData()
//{
//	QueryResult * res = WorldDatabase.Query("SELECT * FROM _donator_menu WHERE npcentry = %u", npcEntry);
//	if (!res)
//	{
//		Log.Error("DonatorMenu", "Table does not exist/is not populated.");
//		return;
//	}
//
//	if (res->GetFieldCount() != 7)
//	{
//		Log.Error("DonatorMenu", "Table is not in the proper format. %u/%d rows.", res->GetFieldCount(), 7);
//		return;
//	} MOVED: UW structs and enums from WorldSession.h to UWDefines.h ADDED: RANK_IMPOSSBILE to the ranks enum for actions no one needs to do using the rankcheck macros ADDED: DUEL_CHECK(rankcheck) for GM command functions to check it he plr object is dueling MODIFIED: Commands that port players to check if the targeted player is dueling. ADDED: FlagAll command to 'o' string MODIFIED: CheckStats command MODIFIED: Donator2. WIP 
//MOVED: UW structs and enums from WorldSession.h to UWDefines.h
//ADDED: RANK_IMPOSSBILE to the ranks enum for actions no one needs to do using the rankcheck macros
//ADDED: DUEL_CHECK(rankcheck) for GM command functions to check it he plr object is dueling
//MODIFIED: Commands that port players to check if the targeted player is dueling.
//ADDED: FlagAll command to 'o' string
//MODIFIED: CheckStats command
//MODIFIED: Donator2. WIPLast 7 days
//
//	Field * f = res->Fetch();
//	DonatorData * temp;
//	CategoryMap * mapPtr;
//	do 
//	{
//		temp = new DonatorData();
//	
//		temp->entry = f[4].GetUInt32();
//		temp->name = f[3].GetString();
//		temp->cost = f[5].GetUInt32();
//
//		switch (f[1].GetUInt32())
//		{
//		case MORPHS:
//			mapPtr = &Morphs;
//			break;
//
//		case SPELLS:
//			mapPtr = &Spells;
//			break;
//
//		case REPUTATION:
//			mapPtr = &Reputations;
//			break;
//
//		case SKILLS:
//			mapPtr = &Skills;
//			break;
//
//		case ANNOUNCEMENTS:
//			temp->announce = f[6].GetString();
//			mapPtr = &Announcements;
//			break;
//
//		case MUSIC:
//			mapPtr = &Music;
//			break;
//
//		case BUFFS:
//			mapPtr = &Buffs;
//			break;
//		}
//
//		if (mapPtr->size() < 14)
//		{
//			CategoryMap::iterator itr = (*mapPtr).find(f[2].GetString());
//			if (itr == (*mapPtr).end())
//			{
//				(*mapPtr)[f[2].GetString()] = DataVector();
//				(*mapPtr).find(f[2].GetString())->second.push_back(temp);
//			}
//			else
//				(itr->second).push_back(temp);
//		}
//		else
//			Log.Error("DonatorMenu", "Too many sub categories. Check the _donator_menu table.");
//
//	} while (res->NextRow());
//
//
//}
//
//void SetupDonator2(ScriptMgr * mgr)
//{
//	GossipScript * gs = (GossipScript*) new Donator2(29089);
//	mgr->register_gossip_script(29089, gs); 
//}