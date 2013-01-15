//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"

void GossipMenu::AddItemBox(uint8 Icon, std::string Message, int32 Id, std::string BoxMessage, uint32 BoxMoney, bool Coded)
{
	GossipMenuItem Item;
	Item.Icon = Icon;
	Item.Extra = Coded;
	Item.Text = Message.c_str();
	Item.m_gBoxMessage = BoxMessage;
	Item.m_gBoxMoney = BoxMoney;
	Item.Id = (uint32)Menu.size();
	if(Id > 0)
		Item.IntId = Id;
	else
		Item.IntId = Item.Id;;

	Menu.push_back(Item);
}

void ObjectMgr::LoadNonScalingSpells()
{
	if(m_non_scaling_spells.size() > 0) //This is here for reloads while server is running
	{
		m_non_scaling_spells.clear();
	}

	QueryResult * result = WorldDatabase.Query("SELECT * FROM _spell_non_scale");
	if(result)
	{
		do 
		{
			m_non_scaling_spells.insert( result->Fetch()[0].GetUInt32() );
		} while(result->NextRow());
		delete result;
	}

	Log.Notice("ObjectMgr", "%u non scaling spells.", m_non_scaling_spells.size());
}