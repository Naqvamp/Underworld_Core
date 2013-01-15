/* KingGossip.h
 *	When talking to the king it will reset the quests he gives
 *	Quest ID: 50058
 *	Phases Used:
 *		16 = 1 Hydra
 *		32 = 2 Hydras
 *		64 = 3 Hydras
 */

/*

class SCRIPT_DECL KingEurystheus : public GossipScript
{
public:
	void GossipHello(Object * pObject, Player* plr, bool AutoSend)
	{
		sChatHandler.SystemMessageToPlr(plr, "LowGUID: %u GUID: %u HighGUID: %u", plr->GetLowGUID(), plr->GetGUID(), plr->GetHighGUID());
		QueryResult * res = CharacterDatabase.Query("SELECT timestamp FROM _dailies_reset WHERE guid = %u LIMIT 1", plr->GetLowGUID());
		if (res)
		{
			uint32 lastTime = res->Fetch()[0].GetUInt32();
			if (lastTime <= (uint32)UNIXTIME)
			{
				CharacterDatabase.Execute("UPDATE _dailies_reset SET timestamp = %u WHERE guid = %u", 86400 + (uint32)UNIXTIME, plr->GetLowGUID());
			}
			delete res;
		}

		//create the menu. The quest handler will fill it in and send the packet
		GossipMenu *Menu;
		objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, plr);

	}

	void GossipEnd(Object * pObject, Player* plr)
	{
		GossipScript::GossipEnd(pObject, plr);
		plr->Gossip_Complete();
	}

	void Destroy()
	{
		delete this;
	}
};



//*/