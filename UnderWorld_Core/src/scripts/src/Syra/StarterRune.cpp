/*
 * StarterRune.cpp		
 *	Ports players to their starter area
 *
 */

#include "StdAfx.h"
#include "Setup.h"

class StarterRune : public GameObjectAIScript
{
public:
	StarterRune(GameObject * goinstance) : GameObjectAIScript(goinstance) { }
	static GameObjectAIScript *Create(GameObject * GO) { return new StarterRune(GO); }

	void OnSpawn()
	{
		RegisterAIUpdateEvent( 3000 );
	}

	void AIUpdate()
	{
		std::set<Object*>::iterator itr = _gameobject->GetInRangePlayerSetBegin();
		std::set<Object*>::iterator it_end = _gameobject->GetInRangePlayerSetEnd();
		//send the message to players who have not done the quest and are high enough level to do so
		for(; itr != it_end; ++itr)
		{
			Player * plr = static_cast<Player*>((*itr));
			if(plr && _gameobject->CalcDistance( _gameobject, plr ) <= 1.5f)
				plr->EventTeleport(230, 687.273F, 164.165F, -72.5007F);
		}
	}
};
void SetupStarterRune(ScriptMgr * mgr)
{
	mgr->register_gameobject_script(153359, &StarterRune::Create);
}