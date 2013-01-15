/* 
 * LaboursSetup.cpp
 *	Main setup for the 12 labours
 */

#include "StdAfx.h"
#include "Setup.h"


void Setup12Labours(ScriptMgr* mgr)
{
	QuestScript * hydras = (QuestScript*) new LernaenHydra();
	mgr->register_quest_script(50058, hydras);
	/*
	GossipScript * king = (GossipScript*) new KingEurystheus();
	mgr->register_quest_script(, king);
	//*/
}