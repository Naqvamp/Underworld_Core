//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"

void WorldSession::AssignPoints(int32 count)
{
	SystemMessage("Your current point total is:  |cffffffff%u", m_points);
	m_points = (count + m_points) > 0 ? count + m_points : 0;
	SystemMessage("|cffffffff%u|r points have been added to your account's point balance.", count);
	SystemMessage("Your new point total is:  |cffffffff%u", m_points);
	
	WorldDatabase.Execute("INSERT INTO `_point_credit_log` (`acct`,`login`,`guid`, `name`,`points_added`,`timestamp`) VALUES (%u, '%s', %u, '%s', %d, NOW())", GetAccountId(), GetAccountNameS(), GetPlayer()->GetLowGUID(), GetPlayer()->GetName(), count);
	SavePoints();
}
bool WorldSession::LogPurchase(char purchase[1024], uint32 price)
{
	if (sWorld.realmID & REALM_ALPHA_SANDBOX)
		return true;

	uint32			acct		= GetAccountId();
	const char *	login		= GetAccountNameS();
	uint32			guid		= _player->GetLowGUID();
	const char*		chr_name	= _player->GetName();
	const char*		gm			= GetPermissions();

	//debug
	if(price > m_points)
	{
		sChatHandler.BlueSystemMessage(this,"Report to Syrathia: Missing price check - report purchase type to Syrathia.");
		return false;
	}
	sChatHandler.SystemMessageToPlr(_player,"Your current point total is:  |cffffffff%u", m_points);
	uint32 pre_onhand = m_points;
	m_points -= price;

	sChatHandler.SystemMessageToPlr(_player,"|cffffffff%u|r points have been deducted from your point balance.", price);
	sChatHandler.SystemMessageToPlr(_player,"Your new point total is:  |cffffffff%u", m_points);
	//(WorldDatabase.EscapeString(string(purchase)))											//    a   l    g   name      pri 
	WorldDatabase.Execute("INSERT INTO _point_usage_log VALUES (%u, '%s', %u, '%s', %u, %u, %u, '%s','%s',NOW())", acct, login, guid, chr_name, pre_onhand, price, m_points, gm, WorldDatabase.EscapeString(string(purchase)).c_str());
	SavePoints();
	return true;
}


bool WorldSession::LogTempExpire()
{
	uint32			acct		= GetAccountId();
	const char *	login		= GetAccountNameS();
	uint32			guid		= _player->GetLowGUID();
	const char*		chr_name	= _player->GetName();
	const char*		gm			= GetPermissions();

	//(WorldDatabase.EscapeString(string(purchase)))											//    a   l    g   name      pri 
	WorldDatabase.Execute("INSERT INTO _point_usage_log VALUES (%u, '%s', %u, '%s', 0, 0, %u, '%s','EXPIRED TEMPGM',NOW())", acct, login, guid, chr_name, m_points, gm);

	return true;
}

void WorldSession::SetSecurity(std::string securitystring, bool reloadforced)
{
	delete [] permissions;
	LoadSecurity(securitystring);
	if(reloadforced)
		sLogonCommHandler.ReloadForced();
}

int8 WorldSession::AssignReward(uint32 itemid)
{
	int8 result = 99;
	uint32 addto=0;
	switch(itemid)
	{
	case 70666:
		addto = 5;
		break;
	case 70667:
		addto = 10;
		break;
	case 70668:
		addto = 100;
		break;
	case 70669:
		addto = 500;
		break;
	}
	m_points += addto;
	SystemMessage("%u points have been added to your donor point total.  Check in with your nearest point vendor to check your current balance.", addto);
	SavePoints();
	return result;
}

void WorldSession::SavePoints()
{
	WorldDatabase.Execute("UPDATE `%s`.accounts SET reward_points = %u WHERE login = '%s'", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), m_points, GetAccountNameS());
}

void World::SendWorldText_UW(const char* text, ...)
{
	WorldSession *self=0;
    uint32 textLen = (uint32)strlen((char*)text) + 1;

    WorldPacket data(textLen + 40);

	data.Initialize(SMSG_MESSAGECHAT);
	data << uint8(CHAT_MSG_SYSTEM);
	data << uint32(LANG_UNIVERSAL);
	
	data << (uint64)0; // Who cares about guid when there's no nickname displayed heh ?
	data << (uint32)0;
	data << (uint64)0;

	data << textLen;
	data << text;
	data << uint8(0);

	SendGlobalMessage(&data, self);

	if(announce_output){
		sLog.outString("> %s", text);}

	//WANN
	WorldPacket data2(256);
	data2.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
	data2 << (uint32)0 << text << (uint8)0x00;
	SendGlobalMessage(&data2, self);
}

void WorldSession::SendShowBank( uint64 guid )
{
    WorldPacket data( SMSG_SHOW_BANK, 8 );
    data << guid;
    SendPacket( &data );
}
void WorldSession::SendAuctionListUW(uint32 entry)
{
	AuctionHouse* AH = sAuctionMgr.GetAuctionHouse(entry);
	if(!AH)
	{
		sChatHandler.BlueSystemMessage(this, "Report to devs: Unbound auction house npc %u.", entry);
		return;
	}

	WorldPacket data(MSG_AUCTION_HELLO, 12);
	//data << auctioneer->GetGUID();
	data << (uint64)0;
	data << uint32(AH->GetID());

	SendPacket( &data );
}

void WorldSession::SendMailbox()
{
	WorldPacket * data = _player->m_mailBox.BuildMailboxListingPacket();
	SendPacket(data);	
	delete data;
}

void WorldSession::UpdateTempGM(bool login)
{
	/* WTF IS THIS!?!?!?
	if(!m_gmData->temp) 
		return;*/
	
	if(login && !m_gmData->t_checked)
	{
		QueryResult * res = WorldDatabase.Query("SELECT timestamp, permissions FROM _acct_tempgm WHERE login='%s'", GetAccountNameS());
		if(res)
		{
			m_gmData->temp = res->Fetch()[0].GetUInt32();
			string tsstr = ConvertTimeStampToDataTime(m_gmData->temp);
			SetSecurity(res->Fetch()[1].GetString());
			if( (uint32)UNIXTIME < m_gmData->temp )
				sChatHandler.BlueSystemMessage(this,"Temporary permisisons have been applied to this account, they are set to expire on |cffffffff%s.", tsstr.c_str());
		}
		else
		{
			//this shouldn't happen, but just in case
			//TempGM=0;
			//SetSecurity("0");
		}
		m_gmData->t_checked = true;
		delete res;
	}
	//expired
	if(m_gmData->temp > 0 && ((uint32)UNIXTIME > m_gmData->temp))
	{
		LogTempExpire();//must call before permission delete!
		SetSecurity("0");
		WorldDatabase.Execute("DELETE FROM _acct_tempgm WHERE login='%s'", GetAccountNameS());
		CharacterDatabase.Execute("DELETE FROM account_forced_permissions WHERE login='%s'", GetAccountNameS());
		sChatHandler.BlueSystemMessage(this, "NOTICE: Your GM status has expired.");
		m_gmData->temp = 0;
	}
	
	m_gmData->t_checkDelay = (uint32)UNIXTIME + 300;
}

string WorldSession::HandleChatColor(string msg, GMRank rank)
{
	if (_player->ColoredText)
	{
		msg = m_gmData->chatColor + msg;
	}
	return msg;
}
//true if hits the filter
bool WorldSession::UWParseMsg(string & msg)
{
	int32 norm = msg.find("|");

	if (norm != -1) //we have a '|'
	{
		int32 open = msg.find("["); //check for the brackets of a link
		if (open > -1)
		{
			int32 close = msg.find("]", open+1);
			if (close > -1)
			{
				int32 col1 = msg.find(":"); //first colon 
				if (col1 > -1)
				{
					int32 col2 = msg.find(":", col1+1); //second colon past the id
					if (col2 > -1)
					{
						uint32 id = atoi(msg.substr(col1+1, (col2-col1)-1).c_str()); //get itemid	
						if (id > 0)
						{
							ItemPrototype * it = ItemPrototypeStorage.LookupEntry(id);
							if (it) //does this id exist?
							{
								string name = msg.substr(open+1, close-(open+1));
								if (strcmp(it->Name1, name.c_str()) == 0) //does the name match the text in the []?
								{
									int32 postr = msg.find("|", msg.find("|r")+1);
									if (postr > 0) //if theres a | past the |r in the link. No.
										return true;
									else
										return false;
								}
							}
						}
					}
				}
			}
		}
			
		return true; //if not all of the previous checks pass after finding a '|' its not valid
		

	}
	return false;
}

void WorldSession::Mute(const char * reason, WorldSession * m_session, const char * nameB)
{	
	if (!*reason) //MUST have these
		return;

	uint32 acctID;
	string acctName, IP, name, perms, title, color = "";
	if (m_session)
	{
		acctID = m_session->GetAccountId();
		acctName = m_session->GetAccountName();
		IP = m_session->GetSocket()->GetRemoteIP();
		name = string(m_session->GetPlayer()->GetName());
		perms = string( m_session->GetPermissions());
		title = string(sChatHandler.GetRankTitle(m_session));
		color = string(sChatHandler.GetRankColor(m_session));
	}
	else
	{
		acctID = 0;
		acctName = "Console";
		IP = "127.0.0.1";
		if (nameB)
			name = string(nameB);
		else
			name = "ChatSystem";
		perms = "abcdefghijklmnopqrtuvwxyz";
		title = "God";
		color = MSG_COLOR_YELLOW;
	}

	WorldDatabase.Execute("INSERT INTO _gmlogs VALUES (%u, '%s', '%s', '%s', '%s', NOW(), '%s', '%s', '%s', '%s', '%s')", 
		acctID, 
		acctName.c_str(), 
		IP.c_str(), 
		name.c_str(), 
		perms.c_str(), 
		"mute", 
		_player->GetName(), 
		_accountName.c_str(), 
		m_gmData->perms.c_str(), 
		WorldDatabase.EscapeString(string(reason)).c_str()
		);

	uint32 count, timeperiod;
	char * pDuration;
	//Check how many times they've been muted
	uint32 acctId = _accountId;
	QueryResult *result = CharacterDatabase.Query("SELECT * FROM _characters_mutes WHERE acct = %u", acctId);
	if(result) //This dipshit has been muted at least once before
	{
		count = result->Fetch()[1].GetUInt32();
		count++;
		
		if(count<=3) //|cffffffff10 Minutes
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 600;
			pDuration = "|cffffffff10 Minutes";				
		}
		else if(count>=3 && count<7) //|cffffffff30 Minutes
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 1800;
			pDuration = "|cffffffff30 Minutes";				
		}
		else if(count>=7 && count<10) //|cffffffff1 Hour
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 3600;
			pDuration = "|cffffffff1 Hour";				
		}
		else if(count>=10 && count<13) //|cffffffff2 Hours
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 7200;
			pDuration = "|cffffffff2 Hours";				
		}
		else if(count>=13 && count<16) //|cffffffff4 Hours
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 14400;
			pDuration = "|cffffffff4 Hours";				
		}
		else if(count>=16 && count<19) //|cffffffff8 Hours
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 28800;
			pDuration = "|cffffffff8 Hours";				
		}
		else if(count>=19 && count<22) //|cffffffff16 Hours
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 57600;
			pDuration = "|cffffffff16 Hours";				
		}
		else if(count>=22 && count<25) //|cffffffff32 Hours
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 115200;
			pDuration = "|cffffffff32 Hours";				
		}
		else if(count>=25 && count<28) //|cffffffff3 Days
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 259200;
			pDuration = "|cffffffff3 Days";				
		}
		else if(count>=28 && count<31)//|cffffffff1 Week
		{
			//Update their saved count first
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 604800;
			pDuration = "|cffffffff1 Week";				
		}
		else
		{
			CharacterDatabase.Execute("UPDATE _characters_mutes SET mutes = %u WHERE acct = %u", count, acctId);
			
			timeperiod = 62899200;
			pDuration = "|cffffffffPERMANENTLY";	
		}
	}//end if(result)
	else //First time this player is getting muted
	{
		//Update their saved count first
		CharacterDatabase.Execute("INSERT INTO _characters_mutes VALUES(%u, %u)", acctId,(int)1);
		
		count = 1;
		timeperiod = 600;
		pDuration = "|cffffffff10 Minutes";				
	}

	char msg[200];

	if (m_session)
	{
		snprintf(msg, 200, "%sNOTICE: %s was muted by <%s>%s for %s%s <%u LIFETIME>. Reason: %s", 
			color.c_str(), _player->GetName(), title.c_str(), name.c_str(), pDuration, color.c_str(), count, reason);

		sChatHandler.SystemMessage(this, "You are being muted by <%s>%s. Reason: %s", title.c_str(), name.c_str(), reason);
	}
	else
	{
		snprintf(msg, 200, "%s[SYSTEM] %s has been muted. [%s]",
			MSG_COLOR_LIGHTYELLOW, 
			_player->GetName(),
			reason);

		sChatHandler.SystemMessage(this, "You are being muted by the automatic chat system for %s.", reason);
	}

	sChatHandler.BlueSystemMessage(this, "Please be advised all mutes are cumulative over the lifetime of an account.  Each new incident requiring a mute will result in an equal or longer duration then the previous. 31 Total mutes will result in a permanent removal of voice permissions.");
	sWorld.SendWorldText(msg, NULL);

	uint32 banned = (uint32)UNIXTIME+timeperiod;

	sLogonCommHandler.Account_SetMute(_accountName.c_str(), banned );
	m_muted = banned;
	
	delete result;

}

void WorldSession::LoadPoints()
{
	QueryResult * res = WorldDatabase.Query("SELECT reward_points FROM `%s`.accounts WHERE login='%s' LIMIT 1", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), (CharacterDatabase.EscapeString(_accountName).c_str()));
	if(res)
		m_points = res->Fetch()[0].GetUInt32();
	
	delete res;
}

void WorldSession::GMGearCheck()
{
	//gear removal
	uint32 rank = (uint32)m_gmData->rank;
	for (uint32 ii = 1; ii < RANK_COADMIN+1; ii++) //array
		if (ii != rank) //dont check for our rank
			for (uint32 jj = 0; GearIDs[ii][jj] != 0; jj++)
			{
				CHECK_ITEM_REMOVE(GearIDs[ii][jj], _player);
			}
}

void WorldSession::StopAttack()
{
	if(!_player->IsInWorld()) return;
	uint64 guid = GetPlayer()->GetSelection();

	if( guid )
	{
		Unit* pEnemy = _player->GetMapMgr()->GetUnit( guid );
		if( pEnemy != NULL)
		{
			GetPlayer()->EventAttackStop();
			GetPlayer()->smsg_AttackStop(pEnemy);
		}
	}
}