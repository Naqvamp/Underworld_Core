#include "StdAfx.h"
#include "Setup.h"
#pragma warning(disable:4305) //truncation double to float

class SCRIPT_DECL AntiAirSystem : public GameObjectAIScript
{
public:
	static GameObjectAIScript *Create(GameObject * GO) { return new AntiAirSystem(GO); }
	AntiAirSystem (GameObject* goinstance) : GameObjectAIScript(goinstance)
	{	
		RegisterAIUpdateEvent(1);
	}

	void AIUpdate()
	{
		set<Object*>::iterator itr = _gameobject->GetInRangePlayerSetBegin();
		for(; itr != _gameobject->GetInRangePlayerSetEnd(); ++itr)
		{
			Player * Plr = (Player*)(*itr);
			if(Plr->flying_aura || Plr->FlyCheat)
			{
				WorldPacket * chat = sChatHandler.FillMessageData(CHAT_MSG_MONSTER_YELL, 0, "Flying is not permitted!", _gameobject->GetGUID());
				_gameobject->SendMessageToSet(chat, false);
				delete chat;

				Plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
				Plr->KillPlayer();
				Plr->EventTeleport(1, -7133.02, -1266.69, -198.435);
			}
		}
	}
};

class SCRIPT_DECL AntiCliffSystem : public GameObjectAIScript
{
public:
	static GameObjectAIScript *Create(GameObject * GO) { return new AntiCliffSystem(GO); }
	AntiCliffSystem (GameObject* goinstance) : GameObjectAIScript(goinstance)
	{	
		RegisterAIUpdateEvent(1);
	}

	void AIUpdate()
	{
		set<Object*>::iterator itr = _gameobject->GetInRangePlayerSetBegin();
		for(; itr != _gameobject->GetInRangePlayerSetEnd(); ++itr)
		{
			Player * Plr = (Player*)(*itr);	

			if( _gameobject->CalcDistance( _gameobject, Plr ) <= 20.0f )
			{	
				if (!Plr->IsBeingTeleported())
				{
					Plr->BroadcastMessage("Nice try. You cannot approach the goddess from the cliffs.");
					Plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
					Plr->KillPlayer();
					Plr->EventTeleport(1, -7133.02, -1266.69, -198.435);
				}
			}

		
		}
	}
};

class SCRIPT_DECL AntiPlayerWard : public GameObjectAIScript
{
public:
	static GameObjectAIScript *Create(GameObject * GO) { return new AntiPlayerWard(GO); }
	AntiPlayerWard (GameObject* goinstance) : GameObjectAIScript(goinstance)
	{	
		RegisterAIUpdateEvent(3000);
	}

	void AIUpdate()
	{
		set<Object*>::iterator itr = _gameobject->GetInRangePlayerSetBegin();
		for(; itr != _gameobject->GetInRangePlayerSetEnd(); ++itr)
		{
			Player * Plr = (Player*)(*itr);	

			if(Plr->GetItemInterface()->GetItemCount(250001) < 1)
			{	
					Plr->BroadcastMessage("You must have a Crystal of Membership to enter here.");
					//Plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
					//Plr->KillPlayer();
					Plr->EventTeleport(1, -7133.02, -1266.69, -198.435);
			}

		
		}
	}
};

void SetupSyraDefense(ScriptMgr * mgr)
{
	//mgr->register_gameobject_script(250024, &AntiAirSystem::Create);
	//mgr->register_gameobject_script(250024, &AntiPlayerWard::Create);
}