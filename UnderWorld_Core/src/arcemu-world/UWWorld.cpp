//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"
#include "ObjectMgr.h"
#include "Master.h"
#include "World.h"


void World::ShowUsersWithIP(const char * ip, WorldSession * m_session)
{
	SessionMap::iterator itr;
	WorldSession * session;
	m_sessionlock.AcquireReadLock();
	bool FoundUser = false;
	for(itr = m_sessions.begin(); itr != m_sessions.end();)
	{
		session = itr->second;
		++itr;

		if(!session->GetSocket())
			continue;

		string ip2 = session->GetSocket()->GetRemoteIP().c_str();
		if(!stricmp(ip, ip2.c_str()))
		{
			if(!m_session->CanUseCommand('z') && session->CanUseCommand('z'))continue;
			FoundUser = true;
			m_session->SystemMessage("User with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(), 
				ip2.c_str(), session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

			//session->Disconnect();
		}
	}
	if ( FoundUser == false )
		m_session->SystemMessage("There is nobody online with ip [%s]",ip);
	m_sessionlock.ReleaseReadLock();
}

void World::ShowUsersWithAccount(const char * account, WorldSession * m_session)
{
	SessionMap::iterator itr;
	WorldSession * session;
	m_sessionlock.AcquireReadLock();

	bool success = false;
	
	for(itr = m_sessions.begin(); itr != m_sessions.end();)
	{
		session = itr->second;
		++itr;

		if(session->GetAccountId() == 1145) continue; //not for me
		if(!stricmp(account, session->GetAccountNameS()))
		{
			success = true;
			m_session->SystemMessage("Account [%s] IP `%s` Player `%s`.", session->GetAccountNameS(), 
				session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");
		}
	}
	if(!success)
		m_session->SystemMessage("|cffff0000No users were found online with the account name [%s].", account);

	m_sessionlock.ReleaseReadLock();
}

void World::ToggleArenaProgress(WorldSession * m_session, bool state)
{
	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		if(itr->second->GetSession()->m_gmData->rank >= RANK_COADMIN)
		{
	
			sChatHandler.BlueSystemMessage(itr->second->GetSession(), "%s has set the arena to the %s state.", (m_session) ? m_session->GetPlayer()->GetName() : "World::Update" , state ? "event" : "public");
		}
	}
	objmgr._playerslock.ReleaseReadLock();

	
	sWorld.arenaEventInProgress = state;

	//The event crashes the server. Not sure why yet.
	m_session = NULL;
	if (state)
	{
		sWorld.arenaEventTimeout = (uint32)UNIXTIME + 7200000; //2 hours
		//sEventMgr.AddEvent(this, &World::ToggleArenaProgress, m_session,  false, EVENT_UNK, 10000, 1, 0);
	}
	//else
		//sEventMgr.RemoveEvents(this, EVENT_UNK);

}

