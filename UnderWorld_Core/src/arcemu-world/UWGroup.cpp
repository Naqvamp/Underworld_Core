//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"
#include "ObjectMgr.h"
#include "Master.h"
#include "World.h"


void Group::SummonGroup(Player * summoner)
{
	/*if( !pPlayer )
		return false;*/

	GroupMembersSet::iterator itr;
	m_groupLock.Acquire();

	for( uint32 i = 0; i < m_SubGroupCount; i++ )
	{
		if( m_SubGroups[i] != NULL )
		{
			//if( m_SubGroups[i]->m_GroupMembers.begin(/* m_playerInfo*/) != m_SubGroups[i]->m_GroupMembers.end() )
			for( GroupMembersSet::iterator itr2 = m_SubGroups[i]->m_GroupMembers.begin(); itr2 != m_SubGroups[i]->m_GroupMembers.end(); ++itr2 )
			{
				if( (*itr2) != NULL )
				{
					if( (*itr2)->m_loggedInPlayer)
					{
						//(*itr2)->m_loggedInPlayer->HonestGM();
						(*itr2)->m_loggedInPlayer->SummonRequest(summoner->GetLowGUID(), summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
					}
				}
			}
		}
	}

	m_groupLock.Release();
	return;
}
void Group::HonestGroup()
{
	GroupMembersSet::iterator itr;
	m_groupLock.Acquire();

	for( uint32 i = 0; i < m_SubGroupCount; i++ )
	{
		if( m_SubGroups[i] != NULL )
		{
			for( GroupMembersSet::iterator itr2 = m_SubGroups[i]->m_GroupMembers.begin(); itr2 != m_SubGroups[i]->m_GroupMembers.end(); ++itr2 )
			{
				if( (*itr2) != NULL && (*itr2)->m_loggedInPlayer && (*itr2)->m_loggedInPlayer->GetSession()->m_gmData->rank < RANK_NO_RANK )
				{
					(*itr2)->m_loggedInPlayer->HonestGM(true);
					//sChatHandler.RedSystemMessage((*itr2)->m_loggedInPlayer->GetSession(), "GM entering a group with a regular player detected, deactivating cheats...");
					/*m_groupLock.Release();
					return;*/									
				}
			}
		}
	}
	m_groupLock.Release();
	return;
}