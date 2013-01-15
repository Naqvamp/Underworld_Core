/* 
 * UWGuildWars.cpp
 *	Implementation of the entire GuildWar system
 */

#include "StdAfx.h"

#define GWM GuildWarManager
#define GWN GuildWarNode

#define GW GuildWars


initialiseSingleton( GWM );

bool GWM::AssaultNode(uint32 nodeId, Player *plr)
{
	if (!plr->IsInGuild())
	{	
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: You must be in a guild to compete.");
		return false;
	}
	
	if (!plr->IsPvPFlagged() && (plr->GetSession()->m_gmData->rank < RANK_ADMIN))
	{
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: You must be flagged to compete for this objective.");
		return false;
	}
	
	m_lock.Acquire();

	GWNodeMap::iterator itr = m_nodes.find(nodeId);

	m_lock.Release();

	if (itr == m_nodes.end())
	{
		sChatHandler.BlueSystemMessageToPlr(plr, "This node does not exist internally. Report this to Syrathia. ID[%u]");
		Log.Error("GuildWarMgr", "Node id[%u] does not exist in zone[%u] area[%u] map[%u]", nodeId, plr->GetZoneId(), plr->GetAreaID(), plr->GetMapId());
		return false;
	}

	return itr->second->AssaultNode(plr);
}

void GWM::Update()
{
	
}

bool GWN::AssaultNode(Player * plr)
{
	uint32 guildId = plr->GetGuildId();

	//Player's guild is already attacking the node
	if (guildId == m_assaulter)
	{
		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: Your guild is already gaining control of this node.");
		return false;
	}

	//the guild already owns the node
	if (guildId == m_owner)
	{
		//someone is attacking the node. Instant return it
		if (m_assaulter)
		{
			sEventMgr.RemoveEvents(this, EVENT_GW_ASSAULT_NODE);
			m_assaulter = 0;
			//sGWarMgr.Announcer(GW::DEFEND_NODE, plr, this);
			return true;
		}

		sChatHandler.RedSystemMessageToPlr(plr, "ERROR: Your guild controls this node.");
		return false;
	}

	m_assaulter = guildId;
	//sGWarMgr.Announcer(GW::ASSAULT_NODE, plr, this);
//*
	//If we are the previous owners of the node add the timer at half length
	if (guildId == m_prevOwner)
		sEventMgr.AddEvent(this, &GWN::CaptureNode, m_assaulter, EVENT_GW_ASSAULT_NODE, uint32(GW::ASSAULT_TIMER/2), 1, 0);
	else
		sEventMgr.AddEvent(this, &GWN::CaptureNode, m_assaulter, EVENT_GW_ASSAULT_NODE, GW::ASSAULT_TIMER, 1, 0);
//*/
	return true;
}

void GWN::CaptureNode(uint32 guildId)
{

}