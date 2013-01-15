//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"
#include "ObjectMgr.h"
#include "Master.h"
#include "World.h"

void LogonCommHandler::Account_SetPassword(const char * account, const char * password)
{
	map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
	if(logons.size() == 0 || itr->second == 0)
	{
		// No valid logonserver is connected.
		return;
	}

	WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
	data << uint32(8);		// 8 = password
	data << account;
	data << password;
	itr->second->SendPacket(&data, false);
}
void LogonCommHandler::Account_SetSusp(const char * account, uint32 duration, const char * reason)
{
	map<LogonServer*, LogonCommClientSocket*>::iterator itr = logons.begin();
	if(logons.size() == 0 || itr->second == 0)
	{
		// No valid logonserver is connected.
		return;
	}

	WorldPacket data(RCMSG_MODIFY_DATABASE, 50);
	data << uint32(9);		// 9 = susp
	data << account;
	data << duration;
	data << reason;
	itr->second->SendPacket(&data, false);
}


void LogonCommHandler::ReloadForced()
{
	forced_permissions.clear();
	
	Log.Notice("LogonCommClient", "Loading forced permission strings...");
	QueryResult * result = CharacterDatabase.Query("SELECT * FROM account_forced_permissions");
	if( result != NULL )
	{
		do 
		{
			string acct = result->Fetch()[0].GetString();
			string perm = result->Fetch()[1].GetString();

			arcemu_TOUPPER(acct);
            forced_permissions.insert(make_pair(acct,perm));

		} while (result->NextRow());
		delete result;
	}
}