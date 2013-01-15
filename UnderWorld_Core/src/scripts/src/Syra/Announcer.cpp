/*
 * Announcer.cpp		
 *	This will have the Servant Of Persephone announce that the player needs to talk to her every so often.
 *
 */

#include "StdAfx.h"
#include "Setup.h"

#define SOP_QUEST_ID 50030
class ServantOfPersephone : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(ServantOfPersephone);

	ServantOfPersephone(Creature * pCreature) : CreatureAIScript(pCreature)
	{
		RegisterAIUpdateEvent(120000);
	}
	
	void AIUpdate()
	{
		//could use the _unit->QuestsBegin() to look at the quest list but this is easier
		Quest * qst = QuestStorage.LookupEntry(SOP_QUEST_ID); //This is the quest the npc gives...

		if (!qst || !_unit->IsCreature()) //if it dun exist... nab.
		{
			RemoveAIUpdateEvent();
			Log.Error("ServantOfPersephone", "Quest ID [%u] does not exist.", SOP_QUEST_ID);
			return;
		}

		string msg = "Come talk to me to start your quest to earn your level 240 gear.";

		size_t UnitNameLength = 0, MessageLength = 0;
		CreatureInfo *ci = ((Creature*)_unit)->GetCreatureInfo();

		if(ci == NULL)
			return;

		UnitNameLength = strlen((char*)ci->Name) + 1;
		MessageLength = strlen((char*)msg.c_str()) + 1;

		WorldPacket data(SMSG_MESSAGECHAT, 35 + UnitNameLength + MessageLength);
		data << uint8(CHAT_MSG_MONSTER_SAY);
		data << uint32(0);
		data << _unit->GetGUID();
		data << uint32(0);			// new in 2.1.0
		data << uint32(UnitNameLength);
		data << ci->Name;
		data << uint64(0);
		data << uint32(MessageLength);
		data << msg.c_str();
		data << uint8(0x00);

		std::set<Object*>::iterator itr = _unit->GetInRangePlayerSetBegin();
		std::set<Object*>::iterator it_end = _unit->GetInRangePlayerSetEnd();
		//send the message to players who have not done the quest and are high enough level to do so
		for(; itr != it_end; ++itr)
		{
			Player * plr = static_cast<Player*>((*itr));
			if(plr && plr->GetSession() && !plr->HasFinishedQuest(SOP_QUEST_ID) && !plr->HasQuest(SOP_QUEST_ID) && plr->getLevel() >= 225)
				plr->GetSession()->SendPacket(&data);
		}
	}	
};

void SetupAnnouncers(ScriptMgr * mgr)
{
	if (sWorld.realmID & REALM_ALPHA_X)
		mgr->register_creature_script(81120, &ServantOfPersephone::Create);
}