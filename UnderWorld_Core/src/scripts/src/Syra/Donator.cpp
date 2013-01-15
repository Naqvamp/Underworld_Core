#include "StdAfx.h"
#include "Setup.h"


#define NPC_ENTRY		29089 
#define THREE_DAYS		259200
#define ONE_WEEK		604800
#define TWO_WEEKS		1209600
#define FOUR_WEEKS		2419200
#define POINTS			sess->m_points
#define CHANGEME		99
#define END				{plr->Gossip_Complete(); return;}

#define HEAT_WAVE		"|Hspell:35236|h[Heat Wave]|h|r"
#define HIT_CHANCE		"|Hspell:43689|h[+20% Hit Chance]|h|r"
#define UNDER_WATER		"|Hspell:11789|h[Underwater Breathing]|h|r"
#define IMP_POISON		"|Hspell:36839|h[Impairing Poison]|h|r"
#define CHAOS_FLAMES	"|Hspell:39055|h[Chaos Flames]|h|r"
#define CHAOS_CHARGE	"|Hspell:40497|h[Chaos Charge]|h|r"
#define CHAIN_F_BALL	"|Hspell:35853|h[Chain Fireball]|h|r"

//temp_gm prices TO-DO: enum this
enum GMCosts
{
	BRONZE_3D = 20,
	BRONZE_1W = 40,
	BRONZE_2W = 60,
	BRONZE_4W = 80,

	SILVER_3D = 30,
	SILVER_1W = 60,
	SILVER_2W = 90,
	SILVER_4W = 120,

	GOLD_3D = 40,
	GOLD_1W = 80,
	GOLD_2W = 120,
	GOLD_4W = 160,

	PERM_BRONZE = 250,
	PERM_SILVER = 500,
	PERM_GOLD = 750,
	PERM_PLAT = 1250,

	ALL_PLAT = 3000,

	ADDON_1_PERM = 100,
	ADDON_2_PERM = 150,
	ADDON_3_PERM = 200,
	ADDON_4_PERM = 250
};


class SCRIPT_DECL Donator : public GossipScript
{
public:
void GossipHello(Object * pObject, Player* plr, bool AutoSend);
void GossipSelectOption(Object * pObject, Player* plr, uint32 Id, uint32 IntId, const char * Code);
void GossipEnd(Object * pObject, Player* plr);
bool EnoughPoints(Player*plr, uint32 price);
void MakeGM(Player *plr, GMRank rank, uint32 length = 0);
void AddAddOn(Player *plr, uint8 addon);
void PlayAll(Player * plr, uint32 sound, uint32 price);
void GiveAllyRep(Player * plr, Creature * pCreature, int32 amt, uint32 price);
void MorphX(Player *plr, Creature * pCreature, uint32 morphID, uint32 price);
void LearnSpell(Player * plr, Creature * pCreature, uint32 spell, uint32 price);
void GiveBuff(Player *plr, Creature * pCreature, uint32 spellid, uint32 price);
void GiveALLBuff(Player *plr, Creature * pCreature, uint32 spellid, uint32 price);
void GiveAllSkills(Player *plr, Creature * pCreature, uint32 amt, uint32 price);
void GiveRep(Player *plr, Creature * pCreature, uint32 FactionID, int32 amt, uint32 price);
void GiveHordeRep(Player * plr, Creature * pCreature, int32 amt, uint32 price);
void DeleteNPC(Creature * unit);
uint32 UpgradePrice(GMRank low, GMRank high);
uint32 UpgradeLength(GMRank oldRank, GMRank newRank, uint32 curLength, uint32 addLength);
void Destroy()
{
	delete this;
}
};
#pragma warning(disable:4305)  
void Donator::AddAddOn(Player * plr, uint8 addon)
{
	WorldSession * sess = plr->GetSession();

	sess->m_gmData->addons[addon-1] = true;
	
	char out[256];
	if (sess->m_gmData->temp)
		snprintf(out, 256, "%c-", 't');
	else
		snprintf(out, 256, "%c-", 'p');


	switch(sess->m_gmData->rank)
	{
	case RANK_BRONZE: //bronze
		{
			snprintf(out, 256, "%s%s-", out, "bronze");
		}
		break;
	case RANK_SILVER: //silver
		{
			snprintf(out, 256, "%s%s-", out, "silver");
		}
		break;
	case RANK_GOLD: //gold
		{
			snprintf(out, 256, "%s%s-", out, "gold");
		}
		break;
	case RANK_PLAT: //plat
		{
			snprintf(out, 256, "%s%s-", out, "platinum");
		}
		break;
	default:
		sChatHandler.BlueSystemMessageToPlr(plr,"Report to Syrathia: Invalid RANK value in AddAddon() function - donor shop");
	}

	if (sess->m_gmData->addons[0])
		snprintf(out, 256, "%s%c", out, '1');
	if (sess->m_gmData->addons[1])
		snprintf(out, 256, "%s%c", out, '2');
	if (sess->m_gmData->addons[2])
		snprintf(out, 256, "%s%c", out, '3');
	if (sess->m_gmData->addons[3])
		snprintf(out, 256, "%s%c", out, '4');
	if (sess->m_gmData->mute)
		snprintf(out, 256, "%s%c", out, 'm');

	CharacterDatabase.Execute("REPLACE INTO account_forced_permissions VALUES('%s', '%s', %u, %u)", sess->GetAccountNameS(), out, (int)0, (int)0);
}

void Donator::MakeGM(Player *plr, GMRank rank, uint32 length)
{
	WorldSession * sess = plr->GetSession();
	if (length) //if its a temp rank this will be >0
		length += (uint32)UNIXTIME;

	if(sess==NULL)
		return;

	string ranks = "0"; //just get the rank name in here. we will build the actual string at the end
	switch(rank)
	{
	case RANK_BRONZE: //bronze
		{
			sess->m_gmData->rank = RANK_BRONZE;
			ranks = "bronze";
		}
		break;
	case RANK_SILVER: //silver
		{
			sess->m_gmData->rank = RANK_SILVER;
			ranks = "silver";
		}
		break;
	case RANK_GOLD: //gold
		{
			sess->m_gmData->rank = RANK_GOLD;
			ranks = "gold";
		}
		break;
	case RANK_PLAT: //plat
		{
			sess->m_gmData->rank = RANK_PLAT;
			ranks = "platinum";
		}
		break;
	default:
		sChatHandler.BlueSystemMessageToPlr(plr,"Report to Syrathia: Invalid RANK value in MakeGM() function - donor shop");
	}

	bool add = false; //need to track is any addons are added to add 'x' otherwise
	char perms[256];
	snprintf(perms, 256, "%c-%s-", (length ? 't' : 'p'), ranks.c_str());

	for (uint32 ii = 0; ii < 4; ii++)
	{
		if (sess->m_gmData->addons[ii])
		{
			add = true;
			snprintf(perms, 256, "%s%u", perms, ii);
		}
	}
	if (sess->m_gmData->mute)
	{
		snprintf(perms, 256, "%s%c", perms, 'm');
	}
	else if (!add)
		snprintf(perms, 256, "%s%c", perms, 'x');

	sess->m_gmData->temp = length;
	sess->m_gmData->t_checked = true;
	sess->m_gmData->t_checkDelay = (uint32)UNIXTIME;

	sess->SetSecurity(perms);

	if (length)
		WorldDatabase.Execute("REPLACE INTO _acct_tempgm VALUES ('%s',%u,'%s')", sess->GetAccountNameS(), length, perms);		
	else		
		WorldDatabase.Execute("DELETE FROM _acct_tempgm WHERE login='%s'", sess->GetAccountNameS());
	
	CharacterDatabase.Execute("REPLACE INTO account_forced_permissions VALUES('%s', '%s', %u, %u)", sess->GetAccountNameS(), perms, (int)0, (int)0);
	CHECK_ITEM_ADD(138001, plr, 1);
	sess->GMGearCheck();
}
bool Donator::EnoughPoints(Player*plr, uint32 price)
{
	
	if (sWorld.realmID & REALM_ALPHA_SANDBOX)
		return true;

	WorldSession * sess = plr->GetSession();
	if(sess==NULL)return false;
	if(price > sess->m_points)
	{
		sChatHandler.RedSystemMessage(sess, "ERROR: You do not possess enough points for this purchase. You need %u more.", price - sess->m_points);
		return false;
	}
	return true;
}
void Donator::GiveAllyRep(Player * plr, Creature * pCreature, int32 amt, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	else
	{
		char purchase[1024];
		snprintf(purchase,1024,"ALLIANCE REP: %d", amt);
		plr->GetSession()->LogPurchase(purchase,price);

		int32 factamt1 = (plr->GetStanding(69) + amt);
		int32 factamt2 = (plr->GetStanding(72) + amt);
		int32 factamt3 = (plr->GetStanding(930) + amt);
		int32 factamt4 = (plr->GetStanding(47) + amt);
		int32 factamt5 = (plr->GetStanding(54) + amt);

		plr->SetStanding( 69, factamt1);
		plr->SetStanding( 72, factamt2);
		plr->SetStanding( 930, factamt3);
		plr->SetStanding( 47, factamt4);
		plr->SetStanding( 54, factamt5);

		char msg[250];
		snprintf(msg, 250, "Thank you %s, I have increased your standing with the Alliance by %d points.", plr->GetName(), amt);
		pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
		plr->Gossip_Complete();
	}
}
void Donator::GiveHordeRep(Player * plr, Creature * pCreature, int32 amt, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	else
	{
		char purchase[1024];
		snprintf(purchase,1024,"HORDE REP: %d", amt);
		plr->GetSession()->LogPurchase(purchase,price);
		
		int32 factamt1 = (plr->GetStanding(76) + amt);
		int32 factamt2 = (plr->GetStanding(530) + amt);
		int32 factamt3 = (plr->GetStanding(81) + amt);
		int32 factamt4 = (plr->GetStanding(68) + amt);
		int32 factamt5 = (plr->GetStanding(911) + amt);

		plr->SetStanding( 76, factamt1);
		plr->SetStanding( 530, factamt2);
		plr->SetStanding( 81, factamt3);
		plr->SetStanding( 68, factamt4);
		plr->SetStanding( 911, factamt5);

		char msg[250];
		snprintf(msg, 250, "Than you %s, I have increased your standing with the Horde by %d points.", plr->GetName(), amt);
		pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
		plr->Gossip_Complete();
	}
}
void Donator::GiveRep(Player *plr, Creature * pCreature, uint32 FactionID, int32 amt, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	
	int32 origvalue = amt;
	amt += plr->GetStanding(FactionID);

	plr->SetStanding(FactionID, amt);

	char purchase[1024];
	snprintf(purchase,1024,"FACTION: %u, AMT: %d", FactionID, origvalue);
	plr->GetSession()->LogPurchase(purchase,price);

	char msg[250];
	snprintf(msg, 250, "Thank you %s, I have advanced your faction standing by %d points.", plr->GetName(), origvalue);
	pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
	plr->Gossip_Complete();
}
void Donator::GiveAllSkills(Player *plr, Creature * pCreature, uint32 amt, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}

	plr->_AdvanceAllSkills(amt);

	char purchase[1024];
	snprintf(purchase,1024,"ALL SKILLS: %u", amt);
	plr->GetSession()->LogPurchase(purchase,price);

	char msg[250];
	snprintf(msg, 250, "Than you %s, I have advanced all your skills by %u.", plr->GetName(), amt);
	pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
	plr->Gossip_Complete();
}
void Donator::GiveBuff(Player *plr, Creature * pCreature, uint32 spellid, uint32 price)
{/*	DEPRECATED
	if( !EnoughPoints(plr, price) )
		{END;}
	
	Unit *caster = plr;
	Unit *target = plr;
	
	SpellEntry *spellentry = dbcSpell.LookupEntry(spellid);
		
	Spell *sp = new Spell(target, spellentry, false, NULL);
	if(!sp)
	{
		delete sp;
		return;
	}
	
	SpellCastTargets targets;
	targets.m_unitTarget = target->GetGUID();
	sp->prepare(&targets);

	plr->Gossip_Complete();*/
}
void Donator::GiveALLBuff(Player *plr, Creature * pCreature, uint32 spellid, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}

	//We dont want them to double up a buff and waste or cause issues
	if(plr->HasAura(spellid))
	{
		plr->GetSession()->SystemMessage("You already have this buff active.  Please wait until it expires before selecting another.");
		return;
	}	

	//Passed all checks, begin transaction
	Player * plr2;	
	SpellEntry * info = dbcSpell.LookupEntry(spellid);
	
	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		plr2 = itr->second;
		if(plr2->GetSession() && plr2->IsInWorld())
		{
			if(plr2->GetMapMgr() != plr->GetMapMgr())
			{
				sEventMgr.AddEvent( static_cast< Unit* >( plr2 ), &Unit::EventCastSpell, static_cast< Unit* >( plr2 ), info, EVENT_PLAYER_CHECKFORCHEATS, 100, 1,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT );
			}
			else
			{/*  DEPRECATED
				Spell * sp = new Spell(plr2, info, true, 0);
				SpellCastTargets targets(plr2->GetGUID());
				sp->prepare(&targets);*/
				plr2->CastSpellOnSelf(spellid);
			}
		}
	}
	objmgr._playerslock.ReleaseReadLock();

	char msg[1024];
	snprintf(msg, 1024, "<God of Death>THANATOS: On behalf of %s, I bestow upon you all a portion of my strength with the spell [%s].", plr->GetName(), info->Name);

	char pAnnounce[1024];
	string input2;
	input2 = "|cff00ff00";
	snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
	sWorld.SendWorldText(pAnnounce); 

	char purchase[1024];
	snprintf(purchase,1024,"BUFF: %s", info->Name);
	plr->GetSession()->LogPurchase(purchase,price);

	plr->Gossip_Complete();
}
void Donator::PlayAll(Player * plr, uint32 sound, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	else
	{
		char msg[1024];
		snprintf(msg, 1024, "<God of Death>THANATOS: On behalf of %s, allow me to entertain you with a song entitled, '%s'", plr->GetName(), (sound == 10896) ? "Lament of the Highborne" : "For the Horde!");

		char pAnnounce[1024];
		string input2;
		input2 = "|cff00ff00";
		snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
		sWorld.SendWorldText(pAnnounce); 
		
		//Play song
		WorldPacket data(SMSG_PLAY_SOUND, 4);
		data << sound;
		sWorld.SendGlobalMessage(&data, 0);

		//Wrap this up, remove the price and close the window
		
		char purchase[1024];
		snprintf(purchase,1024,"MUSIC: L70ETC");
		plr->GetSession()->LogPurchase(purchase,price);
		plr->Gossip_Complete();
	}
}
void Donator::LearnSpell(Player * plr, Creature * pCreature, uint32 spell, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	else
	{
		SpellEntry * sp = dbcSpell.LookupEntry(spell);
		if(!sp)
		{
			sChatHandler.BlueSystemMessageToPlr(plr,"ERROR: Spell (%u) is invalid - report to devs.", spell);
			plr->BroadcastMessage("Due to an error your transaction has been cancelled, no points have been deducted from your point balance.");
			return;
		}
		
		if (plr->HasSpell(spell)) // check to see if char already knows
		{
			char msg[250];
			snprintf(msg, 250, "%s, you fool, you already know %s.", plr->GetName(), sp->Name);
			pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
			plr->Gossip_Complete();
			return;
		}
		
		plr->addSpell(spell);
		

		char purchase[1024];
		snprintf(purchase,1024,"LEARNSPELL: %s", sp->Name);
		plr->GetSession()->LogPurchase(purchase,price);

		char msg[250];
		snprintf(msg, 250, "%s, I have now taught you %s.  I, and the UNDERWORLD, thank you for your contribution.", plr->GetName(), sp->Name);
		pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
		plr->Gossip_Complete();
		
	}
}//end learnspell function
void Donator::MorphX(Player *plr, Creature * pCreature, uint32 morphID, uint32 price)
{
	if( !EnoughPoints(plr, price) )
		{END;}
	plr->SetUInt32Value(UNIT_FIELD_DISPLAYID, morphID);

	char purchase[1024];
	snprintf(purchase,1024,"MORPH: %u", morphID);
	plr->GetSession()->LogPurchase(purchase,price);

	plr->Gossip_Complete();
	char msg2[250];
	snprintf(msg2, 250, "%s, you have now been morphed.  Be aware, should you zone into the UNDERWORLD Captial or logout, the morph will be removed.", plr->GetName());
	pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg2, plr);

}

void Donator::DeleteNPC(Creature * unit)
{
	if(unit->GetSQL_id() > 0)return;
	
	if(unit->IsInWorld())
	{
		if(unit->m_spawn)
		{
			uint32 cellx = uint32(((_maxX-unit->m_spawn->x)/_cellSize));
			uint32 celly = uint32(((_maxY-unit->m_spawn->y)/_cellSize));

			if(cellx <= _sizeX && celly <= _sizeY)
			{
				CellSpawns * sp = unit->GetMapMgr()->GetBaseMap()->GetSpawnsList(cellx, celly);
				if( sp != NULL )
				{
					for( CreatureSpawnList::iterator itr = sp->CreatureSpawns.begin(); itr != sp->CreatureSpawns.end(); ++itr )
						if( (*itr) == unit->m_spawn )
						{
							sp->CreatureSpawns.erase( itr );
							break;
						}
				}
				delete unit->m_spawn;
			}
		}
		unit->RemoveFromWorld(false,true);
	}
	delete unit;
}

uint32 Donator::UpgradePrice(GMRank low, GMRank high)
{
	uint32 lowCost, highCost = 0;
	switch (low)
	{
	case RANK_NO_RANK:
		lowCost = 0;
		break;
	case RANK_BRONZE:
		lowCost = PERM_BRONZE;
		break;
	case RANK_SILVER:
		lowCost = PERM_SILVER;
		break;
	case RANK_GOLD:
		lowCost = PERM_GOLD;
		break;
	}

	switch (high)
	{
	case RANK_BRONZE:
		highCost = PERM_BRONZE;
		break;
	case RANK_SILVER:
		highCost = PERM_SILVER;
		break;
	case RANK_GOLD:
		highCost = PERM_GOLD;
		break;
	case RANK_PLAT:
		highCost = PERM_PLAT;
		break;
	}

	return highCost - lowCost;
}
//cur is how much time is left and add is how much we are adding.
uint32 Donator::UpgradeLength(GMRank oldRank, GMRank newRank, uint32 curLength, uint32 addLength)
{
	if (oldRank == RANK_NO_RANK || curLength < 100) 
		return addLength;

	if (oldRank == newRank)
		return curLength + addLength; //this has UNIXTIME

	//this was using unixtime in it O_o
	uint32 updated = ((newRank / oldRank) * (curLength - (uint32)UNIXTIME)); //the old time updated with the multiplier per rank

	//we want to scale the amount being carried over
	//so we dont have someone purchase lots of bronze time and just up grade to gold temp and get a huge bonus
	float scale = 0.0F;
	if (updated < 86400) //under a day added
		scale = 1.0F;
	else if (updated < THREE_DAYS) //3 days
		scale = 0.8F;
	else if (updated < ONE_WEEK) //7 days
		scale = 0.6F;
	else if (updated < TWO_WEEKS) //14 days
		scale = 0.4F;
	else if (updated < FOUR_WEEKS) //28 days
		scale = 0.2F;
	else
		scale = 0.1F;



	return uint32(addLength + (updated * scale)); 
}
void Donator::GossipHello(Object * pObject, Player* plr, bool AutoSend)
{
	WorldSession * sess = plr->GetSession();
	if(sess==NULL)return;
	GossipMenu *Menu;
	objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80000, plr);
	Menu->AddItem(1, "Browse Donor Shop", 2);
//	Menu->AddItem(12, "F.A.Q.", 1);


	char npc_text[1024];
	snprintf(npc_text, 1024, "Your Total Points: %u", POINTS);
	Menu->AddItem(12, npc_text, 3);
		
	//if(AutoSend)
		Menu->SendTo(plr);
}

void Donator::GossipSelectOption(Object * pObject, Player* plr, uint32 Id, uint32 IntId, const char * Code)
{
	/*Declarations*/Creature * pCreature = (pObject->GetTypeId()==TYPEID_UNIT)?((Creature*)pObject):NULL;
	if(pObject==NULL)
		return;

	WorldSession * sess = plr->GetSession();
	if(sess==NULL)return;
	Log.Debug("Donator", "CODE: %s IntId: %u",Code, IntId);
	//morph catch
	if(IntId > 3000)
	{
		MorphX(plr,pCreature, IntId, 2);
		END
	}

	GossipMenu * Menu;
	/*GM Variables*/
	uint32 gm_duration=0;
	uint32 rank=0;
	uint32 price=0;
	uint32 leftover_duration = 0;
	if(sess->m_gmData->temp > 0)
		leftover_duration = (sess->m_gmData->temp - (uint32)UNIXTIME);

	switch(IntId)
	{
	case 1001:
		{
			sess->LoadPoints();
			sChatHandler.GreenSystemMessage(sess, "Account Points reloaded. You now have %u points.", sess->m_points);
		}
	case 1000: // Return to start	
		objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80000, plr);
		Menu->AddItem(1, "Browse Donor Shop", 2);
		Menu->AddItem(12, "F.A.Q.", 1);
		
		char npc_text[1024];
		snprintf(npc_text, 1024, "Your Total Points: %u", POINTS);
		Menu->AddItem(12, npc_text, 3);
		Menu->SendTo(plr);
		break;
	

	case 1: //F.A.Q. - DONE
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80001, plr);
			Menu->AddItem(12, "How much do I donate?", 4);
			Menu->AddItem(12, "Intervals?", 5);
			Menu->AddItem(12, "Are rewards per character or account?", 6);
			Menu->AddItem(12, "Permanent? Temporary? Elaborate please.", 7);
			Menu->AddItem(12, "What about rollbacks and data-loss?", 8);
			Menu->AddItem(12, "GM Commands and the metal ranks?", 11);
			Menu->AddItem(12, "What if I'm already a GM and purchase for more time?", 12);
			Menu->AddItem(12, "BACK", 1000);
			Menu->SendTo(plr);
		}break;
			case 4: //How much do I donate? - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80002, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 5: //Intervals? - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80003, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 6: //Are rewards per character or account?  - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80004, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 7: //Permanent? Temporary? Elaborate please. - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80005, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 8: //What about rollbacks and data-loss? - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80006, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 11: //GM Commands and the metal ranks? - DONE
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80010, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;
			case 12: //What if I'm already a GM and purchase for more time?
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80019, plr);
					Menu->AddItem(12, "BACK", 1);
					Menu->SendTo(plr);
				}break;

	case 3:
		{
			sChatHandler.SystemMessageToPlr(plr,"Your current point total is:  |cffffffff%u", POINTS);
			END;
		}break;

	case 2: //Browse Shop - DONE
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80007, plr);
			Menu->AddItem(7, "GM Commands", 20);
			Menu->AddItem(5, "Morphs", 60);
			Menu->AddItem(2, "Spells", 240);
			Menu->AddItem(3, "Reputation", 180);
			Menu->AddItem(4, "Skill Advancement", 170);
			Menu->AddItem(12, "Realm Announcements", 110);
			Menu->AddItem(7, "Realm Music", 130);
			Menu->AddItem(9, "Realm Buffs", 150);
			Menu->AddItem(4, "RENAME ME", 230);
			Menu->AddItem(3, "FREE Services", 400);
			Menu->AddItem(3, "Reload Account Points", 1001);
			Menu->AddItem(12, "[BACK]", 1000);
			Menu->SendTo(plr);
		}break;	

	/****************************************
	*		     GM COMMANDS(20-59)	        *
	*		    Extra IDs in(81-109)        *
	****************************************/
	case 20: //GMCommands
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80011, plr);
			Menu->AddItem(7, "Permanent Rank", 21);
			Menu->AddItem(7, "Temporary Ranks", 30);
			if (sess->m_gmData->rank > RANK_NO_RANK && sess->m_gmData->temp < 1)
				Menu->AddItem(7, "Addon Packs", 80);
			Menu->AddItem(12, "BACK", 2);
			Menu->SendTo(plr);

		}break;

	case 21: //Permanent Rank
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80012, plr);
			char config_price[1024];
			char message[1024];
			bool rankUp = (sess->m_gmData->rank > RANK_NO_RANK);
			
			
			if(sess->m_gmData->rank < RANK_PLAT)
			{
				price = ALL_PLAT;
				snprintf(config_price, 1024, "Permanent Global Platinum(%u Points)", price);
				snprintf(message, 1024, "Are you absolutely sure you wish to purchase Global Plat-Permanent rank for %u points?", price);
				Menu->AddItemBox(7, config_price, 59, message, 0);

				price = UpgradePrice(sess->m_gmData->rank, RANK_PLAT);
				snprintf(config_price, 1024, "Permanent Platinum(%u Points)", price);
				snprintf(message, 1024, "Are you absolutely sure you wish to purchase Plat-Permanent rank for %u points?", price);
				Menu->AddItemBox(7, config_price, (rankUp ? 29 : 26), message, 0);
			}

			if(sess->m_gmData->rank < RANK_GOLD) 
			{
				price = UpgradePrice(sess->m_gmData->rank, RANK_GOLD);
				snprintf(config_price, 1024, "Permanent Gold(%u Points)", price);
				snprintf(message, 1024, "Are you absolutely sure you wish to price Gold-Permanent rank for %u points?", price);
				Menu->AddItemBox(7, config_price, (rankUp ? 28 : 25), message, 0);
			}
		
			if(sess->m_gmData->rank < RANK_SILVER)
			{
				price = UpgradePrice(sess->m_gmData->rank, RANK_SILVER);
				snprintf(config_price, 1024, "Permanent Silver(%u Points)", price);
				snprintf(message, 1024, "Are you absolutely sure you wish to purchase Silver-Permanent rank for %u points?", price);
				Menu->AddItemBox(7, config_price, (rankUp ? 27 : 24), message, 0);
			}

			if(sess->m_gmData->rank < RANK_BRONZE) 
			{
				price = UpgradePrice(sess->m_gmData->rank, RANK_BRONZE);
				snprintf(config_price, 1024, "Permanent BRONZE(%u Points)", price);
				snprintf(message, 1024, "Are you absolutely sure you wish to purchase Bronze-Permanent rank for %u points?", price);
				Menu->AddItemBox(7, config_price, 23, message, 0);
			}
			
				
			//default to all
			Menu->AddItem(12, "BACK", 20);
			Menu->SendTo(plr);
		}break;
			case 23: //permanent 1-3
				{
					price = PERM_BRONZE;

					if(!EnoughPoints(plr, price))
						END
					MakeGM(plr, RANK_BRONZE);
					sess->LogPurchase("Rank: PERM-BRONZE", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 24: //permanent 1-3
				{
					price = PERM_SILVER;

					if(!EnoughPoints(plr, price))
						END
					MakeGM(plr, RANK_SILVER);
					sess->LogPurchase("Rank: PERM-SILVER", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 25: //permanent 1-3
				{
					price = PERM_GOLD;

					if(!EnoughPoints(plr, price))
						END
					MakeGM(plr, RANK_GOLD);
					sess->LogPurchase("Rank: PERM-GOLD", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 26: //Yes! Spend 900 points on PLATINUM GM rank for my account!
				{
					price = PERM_PLAT;

					if(!EnoughPoints(plr, price))
						END									
					MakeGM(plr, RANK_PLAT);
					sess->LogPurchase("Rank: PLATINUM", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 27: //perm upgrade to silver
				{
					price = UpgradePrice(sess->m_gmData->rank, RANK_SILVER);

					if(!EnoughPoints(plr, price))
						END	

					MakeGM(plr, RANK_SILVER);
					sess->LogPurchase("Rank: (upgrade)PERM-SILVER", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 28: //perm upgrade to gold
				{
					price = UpgradePrice(sess->m_gmData->rank, RANK_GOLD);

					if(!EnoughPoints(plr, price))
						END				
					MakeGM(plr, RANK_GOLD);
					sess->LogPurchase("Rank: (upgrade)PERM-GOLD", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 29: //perm upgrade to plat
				{
					price = UpgradePrice(sess->m_gmData->rank, RANK_PLAT);

					if(!EnoughPoints(plr, price))
						END
					MakeGM(plr, RANK_PLAT);
					sess->LogPurchase("Rank: (upgrade)PLATINUM", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 59: //Global plat purchase
				{
					price = ALL_PLAT;

					if(!EnoughPoints(plr, price))
						END
					MakeGM(plr, RANK_PLAT);
					sess->LogPurchase("Rank: GLOBAL PLATINUM", price);
					plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					sChatHandler.BlueSystemMessage(sess, "For your global status to be in effect you need to email Persephone.");
					sChatHandler.BlueSystemMessage(sess, "You can reach her at persephone@uwgaming.com.");
					sChatHandler.BlueSystemMessage(sess, "You need to send her your account name [%s] and explain what you did.", sess->GetAccountNameS());
					END
				}break;
			

	case 30: //Temporary Ranks
		{
			if(sess->m_gmData->temp || sess->m_gmData->rank == RANK_NO_RANK)
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80014, plr);
				Menu->AddItem(7, "Bronze (Rank 1)", 31);
				Menu->AddItem(7, "Silver (Rank 2)", 40);
				Menu->AddItem(7, "Gold (Rank 3)", 50);
				Menu->AddItem(12, "BACK", 20);
				Menu->SendTo(plr);
			}
			else //For Plat, CoAdmin, and Admin -- making sure no one accidently buys a lower temp rank
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80017, plr);
				Menu->AddItem(12, "BACK", 20);
				Menu->SendTo(plr);
			}
		}break;
			case 31: //Bronze (Rank 1)
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80015, plr);
					char config_price[1024];
					char message[1024];					
					
					snprintf(config_price, 1024, "3 DAYS(%u Points)", BRONZE_3D);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase three days of Bronze for %u points?", BRONZE_3D);
					Menu->AddItemBox(7, config_price, 36, message, 0);

					snprintf(config_price, 1024, "1 WEEK(%u Points)", BRONZE_1W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase one week of Bronze for %u points?", BRONZE_1W);
					Menu->AddItemBox(7, config_price, 37, message, 0);

					snprintf(config_price, 1024, "2 WEEKS(%u Points)", BRONZE_2W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase two weeks of Bronze for %u points?", BRONZE_2W);
					Menu->AddItemBox(7, config_price, 38, message, 0);

					snprintf(config_price, 1024, "4 WEEKS(%u Points)", BRONZE_4W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase four weeks of Bronze for %u points?", BRONZE_4W);
					Menu->AddItemBox(7, config_price, 39, message, 0);

					Menu->AddItem(12, "BACK", 30);
					Menu->SendTo(plr);
				}break;
				case 36: //bronze 3 DAYS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_BRONZE, sess->m_gmData->temp, THREE_DAYS);
						price = BRONZE_3D;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_BRONZE, gm_duration);
						sess->LogPurchase("Rank: bronze -- DUR: 3 days",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 37: //bronze ONE_WEEK
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_BRONZE, sess->m_gmData->temp, ONE_WEEK);
						price = BRONZE_1W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_BRONZE, gm_duration);
						sess->LogPurchase("Rank: bronze -- DUR: ONE WEEK",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 38: //bronze TWO_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_BRONZE, sess->m_gmData->temp, TWO_WEEKS);
						price = BRONZE_2W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_BRONZE, gm_duration);
						sess->LogPurchase("Rank: bronze -- DUR: TWO_WEEKS", price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 39: //bronze FOUR_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_BRONZE, sess->m_gmData->temp, FOUR_WEEKS);
						price = BRONZE_4W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_BRONZE, gm_duration);
						sess->LogPurchase("Rank: bronze -- DUR: FOUR_WEEKS",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
			case 40: //Silver (Rank 2)
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80015, plr);
					char config_price[1024];
					char message[1024];					
					
					snprintf(config_price, 1024, "3 DAYS(%u Points)", SILVER_3D);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase three days of Silver for %u points?", SILVER_3D);
					Menu->AddItemBox(7, config_price, 45, message, 0);

					snprintf(config_price, 1024, "1 WEEK(%u Points)", SILVER_1W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase one week of Silver for %u points?", SILVER_1W);
					Menu->AddItemBox(7, config_price, 46, message, 0);

					snprintf(config_price, 1024, "2 WEEKS(%u Points)", SILVER_2W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase two weeks of Silver for %u points?", SILVER_2W);
					Menu->AddItemBox(7, config_price, 47, message, 0);

					snprintf(config_price, 1024, "4 WEEKS(%u Points)", SILVER_4W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase four weeks of Silver for %u points?", SILVER_4W);
					Menu->AddItemBox(7, config_price, 48, message, 0);

					Menu->AddItem(12, "BACK", 30);
					Menu->SendTo(plr);
				}break;
				case 45: //silver 3 DAYS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_SILVER, sess->m_gmData->temp, THREE_DAYS);
						price = SILVER_3D;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_SILVER, gm_duration);
						sess->LogPurchase("Rank: silver -- DUR: 3 days",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 46: //silver ONE_WEEK
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_SILVER, sess->m_gmData->temp, ONE_WEEK);
						price = SILVER_1W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_SILVER, gm_duration);
						sess->LogPurchase("Rank: silver -- DUR: ONE WEEK",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 47: //silver TWO_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_SILVER, sess->m_gmData->temp, TWO_WEEKS);
						price = SILVER_2W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_SILVER, gm_duration);
						sess->LogPurchase("Rank: silver -- DUR: TWO_WEEKS",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 48: //silver FOUR_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_SILVER, sess->m_gmData->temp, FOUR_WEEKS);
						price = SILVER_4W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_SILVER, gm_duration);
						sess->LogPurchase("Rank: silver -- DUR: FOUR_WEEKS",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
			case 50: //Gold (Rank 3)
				{
					objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80015, plr);
					char config_price[1024];
					char message[1024];					
					
					snprintf(config_price, 1024, "3 DAYS(%u Points)", GOLD_3D);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase three days of Gold for %u points?", GOLD_3D);
					Menu->AddItemBox(7, config_price, 55, message, 0);

					snprintf(config_price, 1024, "1 WEEK(%u Points)", GOLD_1W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase one week of Gold for %u points?", GOLD_1W);
					Menu->AddItemBox(7, config_price, 56, message, 0);

					snprintf(config_price, 1024, "2 WEEKS(%u Points)", GOLD_2W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase two weeks of Gold for %u points?", GOLD_2W);
					Menu->AddItemBox(7, config_price, 57, message, 0);

					snprintf(config_price, 1024, "4 WEEKS(%u Points)", GOLD_4W);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase four weeks of Gold for %u points?", GOLD_4W);
					Menu->AddItemBox(7, config_price, 58, message, 0);

					Menu->AddItem(12, "BACK", 30);
					Menu->SendTo(plr);
				}break;
				case 55: //gold 3 DAYS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_GOLD, sess->m_gmData->temp, THREE_DAYS);
						price = GOLD_3D;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_GOLD, gm_duration);
						sess->LogPurchase("Rank: gold -- DUR: 3 days",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 56: //gold ONE_WEEK
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_GOLD, sess->m_gmData->temp, ONE_WEEK);
						price = GOLD_1W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_GOLD, gm_duration);
						sess->LogPurchase("Rank: gold -- DUR: ONE WEEK",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 57: //gold TWO_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_GOLD, sess->m_gmData->temp, TWO_WEEKS);
						price = GOLD_2W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_GOLD, gm_duration);
						sess->LogPurchase("Rank: gold -- DUR: TWO_WEEKS",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;
				case 58: //gold FOUR_WEEKS
					{
						gm_duration = UpgradeLength(sess->m_gmData->rank, RANK_GOLD, sess->m_gmData->temp, FOUR_WEEKS);
						price = GOLD_4W;
						
						if(!EnoughPoints(plr, price))
							END

						MakeGM(plr, RANK_GOLD, gm_duration);
						sess->LogPurchase("Rank: gold -- DUR: FOUR_WEEKS",price);
						plr->BroadcastMessage("Congratulations!	Your new GM rank is now in effect.  Use |cffffffff.command|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
						END
					}break;

	case 80: //Addon
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 80016, plr);
			char config_price[1024];
			char message[1024];					
			
			if (sess->m_gmData->temp)
				Menu->SetTextID(80017);
			else
			{

				if (!sess->m_gmData->addons[0])
				{
					snprintf(config_price, 1024, "Addon Pack One (%u Points)", ADDON_1_PERM);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase Addon Pack One for %u points?", ADDON_1_PERM);
					Menu->AddItemBox(7, config_price, 81, message, 0);
				}
				if (!sess->m_gmData->addons[1])
				{
					snprintf(config_price, 1024, "Addon Pack Two (%u Points)", ADDON_2_PERM);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase Addon Pack Two for %u points?", ADDON_2_PERM);
					Menu->AddItemBox(7, config_price, 82, message, 0);
				}
				if (!sess->m_gmData->addons[2])
				{
					snprintf(config_price, 1024, "Addon Pack Three (%u Points)", ADDON_3_PERM);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase Addon Pack Three for %u points?", ADDON_3_PERM);
					Menu->AddItemBox(7, config_price, 83, message, 0);
				}
				if (!sess->m_gmData->addons[3] && sess->m_gmData->rank == RANK_PLAT)
				{
					snprintf(config_price, 1024, "Addon Pack Four (%u Points)", ADDON_4_PERM);
					snprintf(message, 1024, "Are you absolutely sure you wish to purchase Addon Pack Four for %u points?", ADDON_4_PERM);
					Menu->AddItemBox(7, config_price, 84, message, 0);
				}
			}

			Menu->AddItem(12, "BACK", 30);
			Menu->SendTo(plr);

		}break;
			case 81: //Addon 1
				{
					price = ADDON_1_PERM;
					
					if(!EnoughPoints(plr, price))
						END

					AddAddOn(plr, 1);
					sess->LogPurchase("AddOn Pack #1",price);
					plr->BroadcastMessage("Congratulations!	Your new Addon pack is now in effect.  Use |cffffffff.quest|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 82: //Addon 2
				{
					price = ADDON_2_PERM;
					
					if(!EnoughPoints(plr, price))
						END

					AddAddOn(plr, 2);
					sess->LogPurchase("AddOn Pack #2",price);
					plr->BroadcastMessage("Congratulations!	Your new Addon pack is now in effect.  Use |cffffffff.packone|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 83: //Addon 3
				{
					price = ADDON_3_PERM;
					
					if(!EnoughPoints(plr, price))
						END

					AddAddOn(plr, 3);
					sess->LogPurchase("AddOn Pack #3",price);
					plr->BroadcastMessage("Congratulations!	Your new Addon pack is now in effect.  Use |cffffffff.packtwo|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;
			case 84: //Addon 4
				{
					price = ADDON_4_PERM;
					
					if(!EnoughPoints(plr, price))
						END

					AddAddOn(plr, 4);
					sess->LogPurchase("AddOn Pack #4",price);
					plr->BroadcastMessage("Congratulations!	Your new Addon pack is now in effect.  Use |cffffffff.packthree|r and |cffffffff.help|r to see your list of commands or get help information on a command.");								
					END
				}break;


	/****************************************
	*		       MORPHS(60-80)		    *
	****************************************/
	case 60: //"Morphs"
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 90000, plr);
			Menu->AddItem(5, "Page 1", 62);
			Menu->AddItem(5, "Page 2", 63);
			Menu->AddItem(5, "Page 3", 64);
			Menu->AddItem(12, "Demorph(Return to Normal)", 61);			
			Menu->AddItem(12, "[BACK]", 2);
			Menu->SendTo(plr);
		}break;
	case 61: //"Demorph(Return to Normal)"
		{
			plr->DeMorph();
			END
		}break;
	case 62: //page 1
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 90001, plr);
			Menu->AddItem(5, "Robot", 15200);
			Menu->AddItem(5, "Blue Robot", 14379);
			Menu->AddItem(5, "Necro-Herald", 11070);
			Menu->AddItem(5, "Noobie", 15310);
			Menu->AddItem(5, "Felguard", 5048);
			Menu->AddItem(5, "Imp", 12345);
			Menu->AddItem(5, "Earth Elemental", 8550);
			Menu->AddItem(5, "Shade", 4629);
			Menu->AddItem(12, "[BACK]", 60); //back to morph main
			Menu->SendTo(plr);
		}break;
	case 63: //page 2
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 90002, plr);
			Menu->AddItem(5, "Thanatos", 15406);
			Menu->AddItem(5, "Blood Elf Zombie", 15513);
			Menu->AddItem(5, "Hydra", 17528);
			Menu->AddItem(5, "Medivh", 18718);
			Menu->AddItem(5, "Demonchild", 10992);
			//Menu->AddItem(5, "Whelp", 25);
			Menu->AddItem(5, "Lich", 15945);
			Menu->AddItem(5, "Blood Elf Knight", 20178);
			Menu->AddItem(5, "Kael'thas Sunstrider", 20023);
			Menu->AddItem(5, "Void Crystal", 11659);
			Menu->AddItem(12, "[BACK]", 60); //back to morph main
			Menu->SendTo(plr);
		}break;
	case 64: //page 3
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 90003, plr);
			Menu->AddItem(5, "Illidan", 21135);
			Menu->AddItem(5, "Tauren Drummer", 21661);
			Menu->AddItem(5, "Orc Singer", 21665);
			Menu->AddItem(5, "Blood Elf Guitarist", 21666);
			Menu->AddItem(5, "Ascended Deathguard", 10115);
			Menu->AddItem(5, "Death Lord Kaidon", 21576);
			Menu->AddItem(5, "Shark", 12192);
			Menu->AddItem(5, "Black Winged Demon", 16164);
			Menu->AddItem(5, "Murky", 15369);
			Menu->AddItem(5, "Headless Hero", 22352);
			Menu->AddItem(12, "[BACK]", 60); //back to morph main
			Menu->SendTo(plr);
		}break;
	
	/****************************************
	*		     ANNOUNCEMENTS(110-124)	    *
	****************************************/
	case 110: //ANNOUNCEMENTS Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100323, plr);
			Menu->AddItem(12, "Minion Announcement [6 Points]", 120);
			Menu->AddItem(12, "Soldier Announcement [8 Points]", 121);
			Menu->AddItem(12, "Elite Announcement [10 Points]", 122);
			//Menu->AddItem(12, "Legionnaire Announcement [17 Points]", 123);
			Menu->AddItem(12, "Warlord Announcement [12 Points]", 124);
								
			
			Menu->AddItem(12, "[BACK]", 2); //VENDOR main menu
			Menu->SendTo(plr);
		}break;
		case 120: //minion
				{
					uint32 price = 6;
					if( !EnoughPoints(plr, price) )
						{END}
					else
					{
						char msg[1024];
						snprintf(msg, 1024, "<God of Death>THANATOS: Let it be known, %s is a true UNDERWORLD minion.", plr->GetName());

						char pAnnounce[1024];
						string input2;
						input2 = "|cff00ff00";
						snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
						sWorld.SendWorldText(pAnnounce); 
						

						char purchase[1024];
						snprintf(purchase,1024,"ANNOUNCE: minion");
						plr->GetSession()->LogPurchase(purchase,price);
						plr->Gossip_Complete();
					}
				}break;
		case 121: //soldier10
		{
			uint32 price = 8;
			if( !EnoughPoints(plr, price) )
				{END}
			else
			{
				char msg[1024];
				snprintf(msg, 1024, "<God of Death>THANATOS: %s is an exemplory soldier of the UNDERWORLD, all should follow %s example.", plr->GetName(), (plr->getGender()?"her":"his"));

				char pAnnounce[1024];
				string input2;
				input2 = "|cff00ff00";
				snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
				sWorld.SendWorldText(pAnnounce); 
				
				char purchase[1024];
				snprintf(purchase,1024,"ANNOUNCE: soldier");
				plr->GetSession()->LogPurchase(purchase,price);
				plr->Gossip_Complete();
			}
		}break;
		case 122: //ELITE14
		{
			uint32 price = 10;
			if( !EnoughPoints(plr, price) )
				{END}
			else
			{
				char msg[1024];
				snprintf(msg, 1024, "<God of Death>THANATOS: %s is of the Elite of the UNDERWORLD, %s should be feared among mortals.", plr->GetName(), (plr->getGender()?"she":"he"));

				char pAnnounce[1024];
				string input2;
				input2 = "|cff00ff00";
				snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
				sWorld.SendWorldText(pAnnounce); 
				
				char purchase[1024];
				snprintf(purchase,1024,"ANNOUNCE: elite");
				plr->GetSession()->LogPurchase(purchase,price);
				plr->Gossip_Complete();
			}
		}break;
		case 124: //warlord20
		{
			uint32 price = 12;
			if( !EnoughPoints(plr, price) )
				{END}
			else
			{
				char msg[1024];
				snprintf(msg, 1024, "<God of Death>THANATOS: WARLORD %s has graced us with %s presence. Kneel down and pay your respects!", plr->GetName(), (plr->getGender()?"her":"his"));

				char pAnnounce[1024];
				string input2;
				input2 = "|cff00ff00";
				snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg);
				sWorld.SendWorldText(pAnnounce); 

				PlayerStorageMap::const_iterator itr;
				objmgr._playerslock.AcquireReadLock();
				for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
				{
					if(itr->second->GetSession() && (itr->second->GetSession() != plr->GetSession()) && !itr->second->GetSession()->GetPlayer()->m_isGmInvisible)
					{	//Make the message
						Player * chr = itr->second->GetSession()->GetPlayer();
						if(itr->second->GetSession()->m_gmData->rank == RANK_SYRA)
							continue;

						char msg[100];
						snprintf(msg, 100, "All hail WARLORD %s! I kneel in %s presence!", plr->GetName(), (plr->getGender()?"her":"his"));

						WorldPacket * data = /*chr->GetSession()->*/sChatHandler.FillMessageData(CHAT_MSG_YELL, LANG_UNIVERSAL, msg, chr->GetGUID(), 0);
						chr->SendMessageToSet(data, true);
						delete data;

						//make the little shit's bow down - RESPECT BITCH.
						chr->Emote(EMOTE_ONESHOT_KNEEL);						
					}
				}
				objmgr._playerslock.ReleaseReadLock();						
				
				char purchase[1024];
				snprintf(purchase,1024,"ANNOUNCE: warlord-kneeldown");
				plr->GetSession()->LogPurchase(purchase,price);
				plr->Gossip_Complete();
			}
		}break;

	/****************************************
	*		       MUSIC(130-132)		    *
	****************************************/
	case 130: //MUSIC Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100324, plr);
			Menu->AddItem(7, "For the Horde [4 Points]", 131);
			Menu->AddItem(7, "Lament of the Highborne [4 Points]", 132);			
			Menu->AddItem(12, "[BACK]", 2); //VENDOR main menu
			Menu->SendTo(plr);
		}break;
	case 131: // "For the Horde"
		{
			PlayAll(plr, 11803, 4);					
		}break;
	case 132: // "Lament of the Highborne"
		{
			PlayAll(plr, 10896, 4);					
		}break;

	/****************************************
	*		       BUFFs(150-156)		    *
	****************************************/
	case 150: //Buffs Main Menu
		{
			char msg[1024]; //|Hspell:41924|h[Berserk 100%]|h|r 
			snprintf(msg, 1024, "SPELLS INFORMATION:|cff3ac5ff |Hspell:24705|h[Wickerman]|h|r |Hspell:26035|h[Good Times]|h|r |Hspell:24383|h[Sheen of Zanza]|h|r");
			pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);

			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100325, plr);
			Menu->AddItem(9, "Berserk 30% (6 Points)", 151);
			//Menu->AddItem(9, "Berserk 500% (6 Points)", 152);
			//Menu->AddItem(9, "Enrage 900% (12 Points)", 153);
			Menu->AddItem(9, "Wickerman (3 Points)", 154);
			Menu->AddItem(9, "Good Times (3 Points)", 155);
			Menu->AddItem(9, "Sheen of Zanza (3 Points)", 156);
			
			Menu->AddItem(12, "[BACK]", 2); //VENDOR main menu
			Menu->SendTo(plr);
		}break;
	
	case 151: //rage30
		{ GiveALLBuff(plr, pCreature, 23505, 6); }break;
	case 152: //rage 500%
		{ GiveALLBuff(plr, pCreature, 26662, 6); }break;
	case 153: //rage 900%
		{ GiveALLBuff(plr, pCreature, 47008, 12); }break;
	case 154: //wickerman
		{ GiveALLBuff(plr, pCreature, 24705, 3); }break;
	case 155: //good times 1%% 30 min
		{ GiveALLBuff(plr, pCreature, 26035, 3); }break;
	case 156: //sheen of zanza (20%)
		{ GiveALLBuff(plr, pCreature, 24383, 3); }break;

	/****************************************
	*		       Skills(170-173)		    *
	****************************************/
	case 170: //Skills Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100326, plr);
			Menu->AddItem(4, "Advance All Skills by 10 (5 Points)", 171);
			Menu->AddItem(4, "Advance All Skills by 25 (10 Points)", 172);
			Menu->AddItem(4, "Advance All Skills by 50 (15 Points)", 173);
			
			
			Menu->AddItem(12, "[BACK]", 2); //VENDOR main menu
			Menu->SendTo(plr);
		}break;

	case 171: //advance 10 points
		{ GiveAllSkills( plr, pCreature, 10, 5);  }break;
	case 172: //advance 25 points
		{ GiveAllSkills( plr, pCreature, 25, 10);  }break;
	case 173: //advance 50 points
		{ GiveAllSkills( plr, pCreature, 50, 15);  }break;
	
	/****************************************
	*		       Rep(180-222)			    *
	****************************************/
	case 180: //REPUTATION Main Menu
		{   //Split this menu so players can't learn rep to enemy cities
			if(plr->GetTeam() == 0) //Alliance
			{ 
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100327, plr);
				Menu->AddItem(3, "Alliance Menu", 181);
				Menu->AddItem(3, "Factions A - K", 183);
				Menu->AddItem(3, "Factions L - The", 184);
				Menu->AddItem(3, "Factions The - Z", 185);
				Menu->AddItem(12, "[BACK]", 2); //main menu
				Menu->SendTo(plr);
			}
			else //horde
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100327, plr);
				Menu->AddItem(3, "Horde Menu", 182);
				Menu->AddItem(3, "Factions A - K", 183);
				Menu->AddItem(3, "Factions L - The", 184);
				Menu->AddItem(3, "Factions The - Z", 185);
				Menu->AddItem(12, "[BACK]", 2); //main menu
				Menu->SendTo(plr);
			}
		}break;

	case 181: //Ally rep Main Menu
		{   
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100327, plr);
			Menu->AddItem(3, "Alliance 1,000(5 Points)", 190);
			Menu->AddItem(3, "Alliance 5,000(10 Points)", 191);
			Menu->AddItem(3, "Alliance 10,000(15 Points)", 192);					
			Menu->AddItem(12, "[BACK]", 180); //Rep menu
			Menu->SendTo(plr);					
		}break;

	case 182: //horde rep Main Menu
		{   
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100327, plr);
			Menu->AddItem(3, "Horde 1,000(5 Points)", 193);
			Menu->AddItem(3, "Horde 5,000(10 Points)", 194);
			Menu->AddItem(3, "Horde 10,000(15 Points)", 195);					
			Menu->AddItem(12, "[BACK]", 180); //Rep menu
			Menu->SendTo(plr);					
		}break;

	case 190: //all alliance factions 1000
		{	GiveAllyRep(plr,pCreature, 1000, 5);	}break;
	case 191: //all alliance factions
		{	GiveAllyRep(plr,pCreature, 5000, 10);	}break;
	case 192: //all alliance factions
		{	GiveAllyRep(plr,pCreature, 10000, 15);	}break;
	case 193: //horde factions1
		{	GiveHordeRep(plr,pCreature, 1000, 5);	}break;
	case 194: //horde factions2
		{	GiveHordeRep(plr,pCreature, 5000, 10);	}break;
	case 195: //horde factions3
		{	GiveHordeRep(plr,pCreature, 10000, 15);	}break;

	case 183: //a-k
		{ 
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100328, plr);
			Menu->AddItem(3, "Argent Dawn (10 Points)", 197); //529
			Menu->AddItem(3, "Ashtongue Deathsworn (10 Points)", 198);//1012
			Menu->AddItem(3, "Brood of Nozdormu (10 Points)", 199);//910
			Menu->AddItem(3, "Cenarion Circle (10 Points)", 200);//609
			Menu->AddItem(3, "Cenarion Expedition (10 Points)", 201);//942
			Menu->AddItem(3, "Honor Hold (10 Points)", 202);//946
			Menu->AddItem(3, "Keepers of Time (10 Points)", 203);//989
			Menu->AddItem(3, "Kurenai (10 Points)", 204);//978
													
			Menu->AddItem(12, "[BACK]", 180); //main menu
			Menu->SendTo(plr);
		}break;
	
	case 184: // k-the
		{ 
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100328, plr);
			Menu->AddItem(3, "Lower City (10 Points)", 205); //1011
			Menu->AddItem(3, "Netherwing (10 Points)", 206); //1015
			Menu->AddItem(3, "Ogri'la (10 Points)", 207);	//1038
			Menu->AddItem(3, "Sha'tari Skyguard (10 Points)", 208); //1031
			Menu->AddItem(3, "Shattered Sun Offensive (10 Points)", 209); //1077
			Menu->AddItem(3, "Sporeggar (10 Points)", 210); //970
			//Menu->AddItem(3, "The Aldor (10 Points)", 211); //932
			Menu->AddItem(3, "The Consortium (10 Points)", 212); //933
			Menu->AddItem(3, "The Mag'har (10 Points)", 213); //941
													
			Menu->AddItem(12, "[BACK]", 180); //main menu
			Menu->SendTo(plr);
		}break;

	case 185: //the-z
		{ 
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100328, plr);
			Menu->AddItem(3, "The Scale of Sands (10 Points)", 214);//990
			//Menu->AddItem(3, "The Scryers (10 Points)", 215);//934
			Menu->AddItem(3, "The Sha'tar (10 Points)", 216);//935
			Menu->AddItem(3, "The Violet Eye (10 Points)", 217);//967
			Menu->AddItem(3, "Thorium Brotherhood (10 Points)", 218);//59
			Menu->AddItem(3, "Thrallmar (10 Points)", 219);//947
			Menu->AddItem(3, "Timbermaw Hold (10 Points)", 220);//576
			Menu->AddItem(3, "Tranquillien (10 Points)", 221);//922
			Menu->AddItem(3, "Zandalar Tribe (10 Points)", 222);//270
													
			Menu->AddItem(12, "[BACK]", 180); //main menu
			Menu->SendTo(plr);
		}break;

	//196
	case 197: //
		{ GiveRep(plr,pCreature, 529,10000,10); }break;
	case 198: //
		{ GiveRep(plr,pCreature, 1012,10000,10); }break;
	case 199: //
		{ GiveRep(plr,pCreature, 910,10000,10); }break;
	case 200: //
		{ GiveRep(plr,pCreature, 609,10000,10); }break;
	case 201: //
		{ GiveRep(plr,pCreature, 942,10000,10); }break;
	case 202: //
		{ GiveRep(plr,pCreature, 946,10000,10); }break;
	case 203: //
		{ GiveRep(plr,pCreature, 989,10000,10); }break;
	case 204: //Kurenai
		{ GiveRep(plr,pCreature, 978,10000,10); }break;
	/**********************************************************/
	case 205: //
		{ GiveRep(plr,pCreature, 1011,10000,10); }break;
	case 206: //
		{ GiveRep(plr,pCreature, 1015,10000,10); }break;
	case 207: //
		{ GiveRep(plr,pCreature, 1038,10000,10); }break;
	case 208: //
		{ GiveRep(plr,pCreature, 1031,10000,10); }break;
	case 209: //
		{ GiveRep(plr,pCreature, 1077,10000,10); }break;
	case 210: //
		{ GiveRep(plr,pCreature, 970,10000,10); }break;
	case 211: //
		{ GiveRep(plr,pCreature, 932,10000,10); }break;
	case 212: //
		{ GiveRep(plr,pCreature, 933,10000,10); }break;
	case 213: //
		{ GiveRep(plr,pCreature, 941,10000,10); }break;
	/**********************************************************/
	case 214: //
		{ GiveRep(plr,pCreature, 990,10000,10); }break;
	case 215: //
		{ GiveRep(plr,pCreature, 934,10000,10); }break;
	case 216: //
		{ GiveRep(plr,pCreature, 935,10000,10); }break;
	case 217: //
		{ GiveRep(plr,pCreature, 967,10000,10); }break;
	case 218: //
		{ GiveRep(plr,pCreature, 59,10000,10); }break;
	case 219: //
		{ GiveRep(plr,pCreature, 947,10000,10); }break;
	case 220: //
		{ GiveRep(plr,pCreature, 576,10000,10); }break;
	case 221: //
		{ GiveRep(plr,pCreature, 922,10000,10); }break;
	case 222: //
		{ GiveRep(plr,pCreature, 270,10000,10); }break;

	/****************************************
	*		  FORCE RENAME(230-231)         *
	****************************************/
	case 230:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100329, plr);
			Menu->AddItem(4, "Yes, rename me.", 231);			
			Menu->AddItem(12, "[BACK]", 2); //main menu
			Menu->SendTo(plr);
		}break;

	case 231:
	{
		uint32 price = 20;
		if( !EnoughPoints(plr, price) )
			{END}
		else
		{					
			plr->rename_pending = true;
			plr->SaveToDB(false);

			char purchase[1024];
			snprintf(purchase,1024,"RENAME");
			plr->GetSession()->LogPurchase(purchase,price);

			char msg[250];
			snprintf(msg, 250, "Points well spent %s, perhaps when you return we can meet again under your new name.", plr->GetName());
			pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);
			plr->Gossip_Complete();
		}
	}break;

	/****************************************
	*			 SPELLS(240-306)            *
	****************************************/
	case 240: //Spells Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100304, plr);
			Menu->AddItem(3, "Portal Spells", 241);
			Menu->AddItem(3, "Racial Spells", 242);
			Menu->AddItem(3, "Resistance Spells", 243);
			Menu->AddItem(3, "Armor Proficiencies", 244);
			Menu->AddItem(3, "THE FUN STUFF", 310);			
			Menu->AddItem(12, "[BACK]", 2); //main menu
			Menu->SendTo(plr);
		}break;

	case 241: //Portal menu
		{   //Split this menu so players can't learn portal spells to enemy cities
			if(plr->GetTeam() == 0) //Alliance
			{ 
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100305, plr);
				//Menu->SetTextID(100000);
				Menu->AddItem(3, "Shattrath (40 Points)", 250);
				Menu->AddItem(3, "Karazhan (40 Points)", 252);
				Menu->AddItem(3, "Stormwind(30 Points)", 253);
				Menu->AddItem(3, "Darnassus (30 Points)", 254);
				Menu->AddItem(3, "Ironforge (30 Points)", 255);
				Menu->AddItem(3, "Exodar (30 Points)", 256);
				Menu->AddItem(12, "[BACK]", 240); //SPELL menu
				Menu->SendTo(plr);
			}
			else //horde
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100305, plr);
				//Menu->SetTextID(100000);
				Menu->AddItem(3, "Shattrath (40 Points)", 251);
				Menu->AddItem(3, "Karazhan (40 Points)", 252);
				Menu->AddItem(3, "Orgrimmar(30 Points)", 257);
				Menu->AddItem(3, "Silvermoon (30 Points)", 258);
				Menu->AddItem(3, "Thunderbluff (30 Points)", 259);
				Menu->AddItem(3, "Undercity (30 Points)", 260);
				Menu->AddItem(12, "[BACK]", 240); //SPELL menu
				Menu->SendTo(plr);
			}
		}break;

	case 250: //Shattrath portal (alliance)
		{ LearnSpell(plr, pCreature, 33691, 40); } break;
	case 251: //Shattrath portal (horde)
		{ LearnSpell(plr, pCreature, 35717, 40); } break;
	case 252: //Karazhan Portal
		{ LearnSpell(plr, pCreature, 28148, 40); } break;
	case 253: //Stormwind
		{ LearnSpell(plr, pCreature, 10059, 30); } break;
	case 254: //Darnassus
		{ LearnSpell(plr, pCreature, 11419, 30); } break;
	case 255: //Ironforge
		{ LearnSpell(plr, pCreature, 11416, 30); } break;
	case 256: //Exodar
		{ LearnSpell(plr, pCreature, 32266, 30); } break;
	case 257: //Orgrimmar
		{ LearnSpell(plr, pCreature, 11417, 30); } break;
	case 258: //Silvermoon
		{ LearnSpell(plr, pCreature, 32267, 30); } break;
	case 259: //Thunderbluff
		{ LearnSpell(plr, pCreature, 11420, 30); } break;
	case 260: //Undercity
		{ LearnSpell(plr, pCreature, 11418, 30); } break;


	case 242: //Racials Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100306, plr);
			Menu->AddItem(3, "Hardiness", 261);
			Menu->AddItem(3, "Gift of the Naaru", 263);
			Menu->AddItem(3, "Magic Resistance", 265);
			Menu->AddItem(3, "Endurance", 267);
			Menu->AddItem(3, "Escape Artist", 269);
			Menu->AddItem(3, "Expansive Mind", 271);
			Menu->AddItem(3, "Perception", 273);
			Menu->AddItem(3, "Shadowmeld", 275);
			Menu->AddItem(3, "The Human Spirit", 277);
			Menu->AddItem(3, "Underwater Breathing", 279);
			Menu->AddItem(3, "War Stomp", 281);
			Menu->AddItem(3, "Will of the Forsaken", 283);
			Menu->AddItem(12, "[BACK]", 240); //Spells main menu
			Menu->SendTo(plr);
		}break;

	case 261:  //Hardiness spell description
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100308, plr);
			Menu->AddItem(3, "Yes, teach me.", 262);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 262: //Hardiness Learn Now
		{ LearnSpell(plr, pCreature, 20573, 25); } break;
	
	case 263:  //Gift of the Naaru spell description
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100309, plr);
			Menu->AddItem(3, "Yes, teach me.", 264);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 264: //Gift of the Naaru
		{ LearnSpell(plr, pCreature, 28880, 25); } break;

	case 265:  //Magic Resistance Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100310, plr);
			Menu->AddItem(3, "Yes, teach me.", 266);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 266: //Magic resistance
		{ LearnSpell(plr, pCreature, 822, 15); } break;

	case 267:  //Endurance Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100311, plr);
			Menu->AddItem(3, "Yes, teach me.", 268);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 268: //Endurance
		{ LearnSpell(plr, pCreature, 20550, 20); } break;
	
	case 269:  //Escape Artist Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100312, plr);
			Menu->AddItem(3, "Yes, teach me.", 270);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 270: //Escape Artist
		{ LearnSpell(plr, pCreature, 20589, 40); } break;

	case 271:  //Expansive Mind Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100313, plr);
			Menu->AddItem(3, "Yes, teach me.", 272);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 272: //Expansive Mind
		{ LearnSpell(plr, pCreature, 20591, 30); } break;

	case 273:  //Perception Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100314, plr);
			Menu->AddItem(3, "Yes, teach me.", 274);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 274: //Perception
		{ LearnSpell(plr, pCreature, 20600, 40); } break;

	case 275:  //Shadowmeld Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100315, plr);
			Menu->AddItem(3, "Yes, teach me.", 276);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 276: //Shadowmeld
		{ LearnSpell(plr, pCreature, 58984, 45); } break;

	case 277:  //Human SPirit Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100316, plr);
			Menu->AddItem(3, "Yes, teach me.", 278);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 278: //Human Spirit
		{ LearnSpell(plr, pCreature, 20598, 45); } break;

	case 279:  //Underwater Breathing Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100317, plr);
			Menu->AddItem(3, "Yes, teach me.", 280);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 280: //Underwater Breathing 
		{ LearnSpell(plr, pCreature, 5227, 15); } break;

	case 281:  //War Stomp Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100318, plr);
			Menu->AddItem(3, "Yes, teach me.", 282);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 282: //War Stomp 
		{ LearnSpell(plr, pCreature, 20549, 45); } break;

	case 283:  //Will of the Forsaken Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100319, plr);
			Menu->AddItem(3, "Yes, teach me.", 284);
			Menu->AddItem(12, "[BACK]", 242); //back to racial menu
			Menu->SendTo(plr);
		}break;
	case 284: //Will of the Forsaken
		{ LearnSpell(plr, pCreature, 7744, 45); } break;


	case 243: //RESISTANCES Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100320, plr);
			//Menu->AddItem(3, "Arcane [40 Points]", 290);
			Menu->AddItem(3, "Fire   [40 Points]", 291);
			Menu->AddItem(3, "Frost  [40 Points]", 292);
			Menu->AddItem(3, "Nature [40 Points]", 293);
			Menu->AddItem(3, "Shadow [40 Points]", 294);					
			Menu->AddItem(12, "[BACK]", 240); //main menu
			Menu->SendTo(plr);
		}break;

	case 290: //Arcane
		{ LearnSpell(plr, pCreature, 27052, 40); } break;
	case 291: //Fire
		{ LearnSpell(plr, pCreature, 27053, 40); } break;
	case 292: //Frost
		{ LearnSpell(plr, pCreature, 27054, 40); } break;
	case 293: //Nature
		{ LearnSpell(plr, pCreature, 27055, 40); } break;
	case 294: //Shadow
		{ LearnSpell(plr, pCreature, 27056, 40); } break;


	case 244: //Armor skills Main Menu
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100321, plr);
			Menu->AddItem(3, "Cloth [20 Points]", 295);
			Menu->AddItem(3, "Leather [20 Points]", 296);
			Menu->AddItem(3, "Mail [20 Points]", 297);
			Menu->AddItem(3, "Plate [20 Points]", 298);
			
			Menu->AddItem(12, "[BACK]", 240); //Spells main menu
			Menu->SendTo(plr);
		}break;

	case 295: //cloth
		{ LearnSpell(plr, pCreature, 9078, 20); } break;
	case 296: //leather
		{ LearnSpell(plr, pCreature, 9077, 20); } break;
	case 297: //Mail
		{ LearnSpell(plr, pCreature, 8737, 20); } break;
	case 298: //Plate
		{ LearnSpell(plr, pCreature, 750, 20); } break;


	case 245: //"THE FUN STUFF" Main Menu
		{
			//Spell info whisper
			char msg[1024];
			snprintf(msg, 1024, "SPELLS INFORMATION: |Hspell:35236|h[Heat Wave]|h|r |Hspell:43689|h[+20% Hit Chance]|h|r |Hspell:11789|h[Underwater Breathing]|h|r |Hspell:36839|h[Impairing Poison]|h|r |Hspell:39055|h[Chaos Flames]|h|r |Hspell:40497|h[Chaos Charge]|h|r |Hspell:35853|h[Chain Fireball]|h|r");
			pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);

			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100322, plr);
			Menu->AddItem(3, "Heat Wave [50 Points]", 300);
			Menu->AddItem(3, "20% Hit Chance [50 Points]", 301);
			Menu->AddItem(3, "Underwater Breathing [50 Points]", 302);
			Menu->AddItem(3, "Impairing Poison [50 Points]", 303);
			Menu->AddItem(3, "Chaos Flames [50 Points]", 304);
			Menu->AddItem(3, "Chaos Charge [50 Points]", 305);
			Menu->AddItem(3, "Chain Fireball [50 Points]", 306);					
			
			Menu->AddItem(12, "[BACK]", 310); //Spells main menu
			Menu->SendTo(plr);
		}break;
	
	case 300: //
		{ LearnSpell(plr, pCreature, 35236, 50); } break;
	case 301: //
		{ LearnSpell(plr, pCreature, 43689, 50); } break;
	case 302: //
		{ LearnSpell(plr, pCreature, 11789, 50); } break;
	case 303: //
		{ LearnSpell(plr, pCreature, 36839, 50); } break;
	case 304: //
		{ LearnSpell(plr, pCreature, 39055, 50); } break;
	case 305: //
		{ LearnSpell(plr, pCreature, 40497, 50); } break;
	case 306: //
		{ LearnSpell(plr, pCreature, 35853, 50); } break;

	case 310: //"THE FUN STUFF" Main Menu
	{
		objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100603, plr);
		Menu->AddItem(3, "Fun Page 1", 245);
		Menu->AddItem(3, "Fun Page 2", 311);		
		Menu->AddItem(12, "[BACK]", 240); //Spells main menu
		Menu->SendTo(plr);
	}break;

	case 311: //"FUN Page 2"
		{
			//Spell info whisper
			char msg[1024];
			snprintf(msg, 1024, "SPELLS INFORMATION: |Hspell:27124|h[Ice Armor]|h|r |Hspell:13159|h[Aspect of the Pack]|h|r |Hspell:26990|h[Mark of the Wild]|h|r |Hspell:48074|h[Prayer of Spirit]|h|r |Hspell:25898|h[Greater Blessing of Kings]|h|r |Hspell:696|h[Demon Skin]|h|r |Hspell:26992|h[Thorns]|h|r |Hspell:25392|h[Prayer of Fortitude]|h|r |Hspell:25431|h[Inner Fire]|h|r |Hspell:7164|h[Defensive Stance]|h|r");
			pCreature->SendChatMessageToPlayer(CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, msg, plr);

			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100322, plr);
			Menu->AddItem(3, "Ice Armor [50 Points]", 312);
			Menu->AddItem(3, "Aspect of the Pack [50 Points]", 313);
			Menu->AddItem(3, "Mark of the Wild [50 Points]", 314);
			Menu->AddItem(3, "Prayer of Spirit [50 Points]", 315);		//disabled
			Menu->AddItem(3, "G. Blessing of Kings [50 Points]", 316);
			Menu->AddItem(3, "Demon Skin [50 Points]", 317);
			Menu->AddItem(3, "Thorns [50 Points]", 317);					
			Menu->AddItem(3, "Prayer of Fortitude [50 Points]", 319);
			Menu->AddItem(3, "Inner Fire [50 Points]", 320);
			Menu->AddItem(3, "Defensive Stance [50 Points]", 321);
			Menu->AddItem(12, "[BACK]", 310); //Spells main menu
			Menu->SendTo(plr);
		}break;
	
	case 312: //ice armor
		{ LearnSpell(plr, pCreature, 27124, 50); } break;
	case 313: //aspect of the pack
		{ LearnSpell(plr, pCreature, 13159, 50); } break;
	case 314: //mark of the wild
		{ LearnSpell(plr, pCreature, 26990, 50); } break;
	case 315: //prayer of spirit
		{ LearnSpell(plr, pCreature, 48074, 50); } break;
	case 316: //g bok
		{ LearnSpell(plr, pCreature, 25898, 50); } break;
	case 317: //demon skin
		{ LearnSpell(plr, pCreature, 696, 50); } break;
	case 318: //thorns
		{ LearnSpell(plr, pCreature, 26992, 50); } break;
	case 319: //prayer of fort
		{ LearnSpell(plr, pCreature, 25392, 50); } break;
	case 320: //inner fire
		{ LearnSpell(plr, pCreature, 25431, 50); } break;
	case 321: //defensive stance
		{ LearnSpell(plr, pCreature, 7164, 50); } break;

	/****************************************
	*        FREE Services(400-450)         *
	****************************************/
	case 400:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100331, plr);
			if(sWorld.dark_light_enabled)
				Menu->AddItem(3, "TELEPORT: Point Mall", 402);
			Menu->AddItem(3, "Unstuck Account",403);
			Menu->AddItem(3, "Reset Spells and Talents",406);
			//Menu->AddItem(3, "Learn Quest Spells", 401);			
			Menu->AddItem(12, "[BACK]", 2); //main menu
			Menu->SendTo(plr);
		}break;
	case 401:
		{
			plr->LearnMissingSpells();
			END
		}break;
	case 402:
		{
			DeleteNPC(pCreature);
			plr->SafeTeleport(530, 0, 9511.135, -7899.663, 14.667, 1.965069);
			END
		}break;
	case 403:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100332, plr);
			Menu->AddItem(3, "Yes, do it.", 404);
			Menu->AddItem(12, "[BACK]", 400); //main menu
			Menu->SendTo(plr);
		}break;
	case 404:
		{
			plr->UnstuckAccount();
			END
		}break;
	case 407:
		{
			plr->LearnItAllUW();

			END;
		
		}break;
	case 406:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100500, plr);
			Menu->AddItem(3, "Yes, do it.", 407);
			Menu->AddItem(12, "[BACK]", 400); //main menu
			Menu->SendTo(plr);		
		}break;
	case 405:
		{
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 100333, plr);			
			Menu->SendTo(plr);
		}break;

	/**************
	**** DEBUG ****
	**************/
	case 99:
		{
			sChatHandler.BlueSystemMessageToPlr(plr,"Report to Syrathia: Gossip select function CHANGEME.");
			END
		}break;
	default:
		{
			sChatHandler.BlueSystemMessageToPlr(plr,"REPORT TO Syrathia: Invalid IntId (%u)", IntId);
			END
		}break;		
	}//close-switch	
}

void Donator::GossipEnd(Object * pObject, Player* plr)
{
GossipScript::GossipEnd(pObject, plr);
}

void SetupDonator(ScriptMgr * mgr)
{
GossipScript * gs = (GossipScript*) new Donator();
/* Teleporter List */
mgr->register_gossip_script(NPC_ENTRY, gs); // Nega
}

