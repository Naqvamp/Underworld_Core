#include "StdAfx.h"
#include "Setup.h"

static set<string> * AbuseIPs;
static map<uint32,uint32> LastKills;
#define PVP_TOKEN_ID 191000
#define PVP_SPELL_ID 2

void OnKill(Player* killer, Player* victim) //Definds player and victim.
{
	/*
	This is uneccesary as they are done in the Object:DealDamage function from where this would be called
	But is something to keep in mind
	if (!killer->IsInWorld() || victim->IsInWorld())
		return;
	*/


	//There are suicide spells out there
	if (killer == victim)
		return;

	//Compare the two IPs of the players and will warn them and place them in a set for tracking then kick them if they are already warned
	if (killer->GetSession()->GetSocket()->GetRemoteIP().compare(victim->GetSession()->GetSocket()->GetRemoteIP()) == 0)
	{
		string IP = killer->GetSession()->GetSocket()->GetRemoteIP();
		set<string>::const_iterator it = AbuseIPs->find(IP);
		if (it != AbuseIPs->end())
		{
			sChatHandler.RedSystemMessageToPlr(killer, "ERROR: You were warned for trying to abuse the PvP token system.");
			sChatHandler.RedSystemMessageToPlr(victim, "ERROR: You were warned for trying to abuse the PvP token system.");
			killer->GetSession()->Disconnect();
			victim->GetSession()->Disconnect();
			//sWorld.DisconnectUsersWithIP(IP, NULL);
			return;
		}

		AbuseIPs->insert(IP);
		sChatHandler.RedSystemMessageToPlr(killer, "ERROR: Do not abuse the PvP token system.");
		sChatHandler.RedSystemMessageToPlr(victim, "ERROR: Do not abuse the PvP token system.");
		return;
	}

	uint32 chance = RandomUInt(3);
	if (chance == 2)
	{

		if (LastKills[killer->GetLowGUID()] > 0) //this call will add a pair for the killer but it will be set to 0
		{
			if (LastKills[killer->GetLowGUID()] > (uint32)UNIXTIME)
			{
				//Maybe add a message?
				return;	
			}
		}

		LastKills[killer->GetLowGUID()] = (uint32)UNIXTIME + 180; //3 min cd between kills
		killer->CastSpellOnSelf(PVP_SPELL_ID);

		ItemPrototype * it = ItemPrototypeStorage.LookupEntry(PVP_TOKEN_ID);
		if (it)
		{
			killer->GetItemInterface()->AddItemById(PVP_TOKEN_ID, 1, 0);
			sChatHandler.GreenSystemMessageToPlr(killer, "You have successfully taken %s's soul and earned a %s.", victim->GetNameClick(), it->Name1);
			sChatHandler.RedSystemMessageToPlr(victim, "Your soul was taken by %s.", killer->GetNameClick());
			return;
		}
		else //if the item is not in the databse this is run
		{
			Log.Error("PvPToken", "The token id [%u] does not exist.", PVP_TOKEN_ID);
			sChatHandler.GreenSystemMessageToPlr(killer, "You have successfully taken %s's soul.", victim->GetNameClick());
			sChatHandler.RedSystemMessageToPlr(victim, "Your soul was taken by %s.", killer->GetNameClick());
			return;
		}
	}

	//Not gonna do anything if they dont get a reward
}

void SetupPvPToken(ScriptMgr * mgr)
{
   mgr->register_hook(SERVER_HOOK_EVENT_ON_KILL_PLAYER, (void*)OnKill);
}