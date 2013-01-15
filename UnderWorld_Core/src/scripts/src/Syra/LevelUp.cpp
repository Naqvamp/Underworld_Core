/*	LevelUp.cpp
 *	This script is a seres of functions based using the OnlevelUp Shook
 *
 */

#include "StdAfx.h"
#include "Setup.h"

#define MSG(msg) sChatHandler.GreenSystemMessageToPlr(plr, msg)

/*Not doing talents...
void EarlyLevels(Player * plr)
{
	plr->LearnAllTalents();
	MSG("You have just been given all of your talents.");
	
}*/

void Level70(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("You can now get level 70 gear from Kuros the big white dog in the bar. Do not forget to get your Tele Book from the quest at the end.");
		MSG("You can as well travel now to Tower of the Damned or the Seven Deadly Sinss to continue training.");
	}
}

void Level80(Player * plr)
{
	if (plr->getClass() == DRUID && sWorld.m_levelCap > 100)
	{
		MSG("You should change the Base Stats pane in your character window. If you dont by level 101 you cant open it without fixing it manually.");
		MSG("For more info search for the Druid Bug Fix on the forums.");
	}
}

void Level100(Player * plr)
{
	if (plr->getClass() == DRUID && sWorld.m_levelCap > 100)
	{
		MSG("You should change the Base Stats pane in your character window. If you dont by level 101 you cant open it without fixing it manually.");
		MSG("For more info search for the Druid Bug Fix on the forums.");
	}
}

void Level130(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("You can now get your level 130 gear in the Greed Mall found in SDS or the Tomb in Tower of the Damned.");
	}
}

void Level150(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("You are now of a level to continue on to Purgatory or Catacombs of Death Lord Kaidon.");
	}
}

void Level160(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("You are now able to purchase your Level 160 gear in the beginning of Purgatory.");
	}
}

void Level200(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("Now, you may wear the gear that drops from the bosses in Catacombs and Purgatory.");
	}
}

void Level240(Player * plr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		MSG("To get your 240 gear do the quest chain that begins at the end of Purgatory.");
	}
}

void LevelUpMain(Player * plr)
{
	//Log.Notice("LevelUp", "Player %s", plr->GetName());
	uint32 lvl = plr->getLevel();

	switch (lvl)
	{
	case 70:
		Level70(plr);
		break;
	case 80:
		Level80(plr);
		break;
	case 100:
		Level100(plr);
		break;
	case 130:
		Level130(plr);
		break;
	case 150:
		Level150(plr);
		break;
	case 160:
		Level160(plr);
		break;
	case 200:
		Level200(plr);
		break;
	case 240:
		Level240(plr);
		break;
	}
}

void SetupLevelUp(ScriptMgr * mgr)
{
	mgr->register_hook(SERVER_HOOK_EVENT_ON_POST_LEVELUP, (void*)LevelUpMain);
}