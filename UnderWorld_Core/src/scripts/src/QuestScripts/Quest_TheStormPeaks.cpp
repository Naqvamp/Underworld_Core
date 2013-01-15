/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009 ArcEmu Team <http://www.arcemu.org/>
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
#include "Setup.h"
#include "../Common/EasyFunctions.h"

// The Gifts of Loken
class LokensFury : public GameObjectAIScript
{
public:
	ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensFury);
	LokensFury(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

	void OnActivate(Player* pPlayer)
	{
		if ( sEAS.GetQuest( pPlayer, 12965) )
			sEAS.KillMobForQuest( pPlayer, 12965, 0 );
	};

	void Destroy()
	{
		delete this;
	};
};

class LokensPower : public GameObjectAIScript
{
public:
	ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensPower);
	LokensPower(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

	void OnActivate(Player* pPlayer)
	{
		if ( sEAS.GetQuest( pPlayer, 12965) )
			sEAS.KillMobForQuest( pPlayer, 12965, 1 );
	};

	void Destroy()
	{
		delete this;
	};
};

class LokensFavor : public GameObjectAIScript
{
public:
	ADD_GAMEOBJECT_FACTORY_FUNCTION(LokensFavor);
	LokensFavor(GameObject* goinstance) : GameObjectAIScript(goinstance) {};

	void OnActivate(Player* pPlayer)
	{
		if ( sEAS.GetQuest( pPlayer, 12965) )
			sEAS.KillMobForQuest( pPlayer, 12965, 2 );
	};

	void Destroy()
	{
		delete this;
	};
};

void SetupTheStormPeaks(ScriptMgr * mgr)
{
	// The Gifts of Loken
	mgr->register_gameobject_script(192120, &LokensFury::Create);
	mgr->register_gameobject_script(192121, &LokensPower::Create);
	mgr->register_gameobject_script(192122, &LokensFavor::Create);
}
