//UNDERWORLD Custom commands and scripts

#include "StdAfx.h"

#define EVENT_RING			"|cffa335ee|Hitem:70010:0:0:0:0:0:0:0|h[Event Winner Ring]|h|r"
#define EVENT_TRINKET		"|cffa335ee|Hitem:70011:0:0:0:0:0:0:0|h[Event Winner Trinket]|h|r"
#define EVENT_NECKLACE		"|cffa335ee|Hitem:70012:0:0:0:0:0:0:0|h[Event Winner Neckpiece]|h|r"


bool ChatHandler::PInfoAdminCheck(uint32 acct)
{
	if(
		   acct == 1145		//me
		|| acct == 101750	//fuser
		|| acct == 1		//admin
		|| acct == 2		//miranda
		|| acct == 4		//hades
		)return true;

	return false;
}


bool ChatHandler::HandleClaimStakeCommand(const char* args, WorldSession *m_session)
{
	/*if(m_session->CanUseCommand('z'))  //uncomment after testing
	{
		RedSystemMessage(m_session,"ERROR: You're a goof-ball.");
		return true;
	}*/
	m_session->GetPlayer()->StakeClaim();
	return true;	
}

const char * ChatHandler::GetRankColor(WorldSession *m_session)
{
	const char * color = "";
	switch (m_session->m_gmData->rank)
	{
	case RANK_SYRA:
		color = m_session->m_gmData->chatColor.c_str();
		break;

	case RANK_ADMIN:
		color = MSG_COLOR_ADMIN;
		break;

	case RANK_COADMIN:
		color = MSG_COLOR_COADMIN;
		break;

	default:
		color = MSG_COLOR_RED;
	}
	return color;
}

const char * ChatHandler::GetRankTitle(WorldSession *m_session)
{
	const char * rank = "";
	switch (m_session->m_gmData->rank)
	{
	case RANK_SYRA:
		rank = "Architect";
		break;

	case RANK_ADMIN:
		switch (m_session->GetAccountId())
		{
		case 1:
		case 5:
			rank = "DARK LORD";
			break;
		case 2:
			rank = "Goddess";
			break;
		case 101750: //fusers acct
			rank = "Lead Admin";
			break;
		default:
			rank = "Admin";
		}
		break;

	case RANK_COADMIN:
		rank = "CoAdmin";
		break;

	default:
		if (m_session->m_gmData->dev)
			rank = "Dev";
		else
			rank = "GM";
	}
	return rank;
}


bool ChatHandler::HandleAllTitleCommand(const char *args, WorldSession *m_session)
{
	Player* plr = getSelectedChar( m_session, true );
	if( plr == NULL )
		return false;
	//(uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
	for(int32 i=1; i < PVPTITLE_END; ++i)
	{
		int32 title = i;		
		if( title > 0 )
			plr->SetKnownTitle( static_cast< RankTitles >( title ), true );
		else
			plr->SetKnownTitle( static_cast< RankTitles >( -title ), false );

		plr->SetUInt32Value( PLAYER_CHOSEN_TITLE, 0 ); // better remove chosen one
	}
	SystemMessage( m_session, "All titles learned.");
	return true;
}


bool ChatHandler::HandleFreezeCommand(const char *args, WorldSession *m_session)
{
	Player * plr = getSelectedChar(m_session,false);
	if(!plr)return false;
	if(m_session->GetPlayer() == plr)
	{
		m_session->SendNotification("You cannot freeze yourself.");
		return true;
	}
	if(!EnoughRankCheck(m_session->GetPlayer(),plr))return true;
	
	for(uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
	{
		if(plr->m_auras[i] != 0) plr->m_auras[i]->Remove();
	}
	plr->Neutralize();
	plr->CastSpellOnSelf(50224); //visual aura
	plr->CastSpellOnSelf(9454);  //freeze
	plr->bFROZEN=true;			 //tag, you're it mother fucker

	SystemMessage(m_session, "%s is now frozen. After your business is concluded use .removeaura to remove the effect.  This action has been logged.", plr->GetName());
   if (!RANK_CHECK(RANK_SYRA))
		SystemMessageToPlr(plr, "<%s>%s has permanently frozen you. Please direct your attention to the %s for further information.", GetRankTitle(m_session), m_session->GetPlayer()->GetName(), GetRankTitle(m_session));

	char command[100]="freeze";
	char notes[1024];
	snprintf(notes, 1024, "none");
	GMLog(m_session, plr->GetSession(), command, notes);
	
	return true;
}
bool ChatHandler::HandleSilenceCommand(const char* args, WorldSession *m_session)
{
	Player * plr = getSelectedChar(m_session, false);
	if(!plr)return false;
	if(m_session->GetPlayer() == plr)return false;
	if(!EnoughRankCheck(m_session->GetPlayer(), plr))return true;

	plr->CastSpellOnSelf(1852);

	BlueSystemMessage(m_session,"%s has been silenced.", plr->GetNameClick());
	BlueSystemMessageToPlr(plr,"<%s>%s has silenced you.", GetRankTitle(m_session), plr->GetNameClick());
	return true;
}
bool ChatHandler::HandleTimeOutCommand(const char *args, WorldSession *m_session)
{
	Player * plr = getSelectedChar(m_session, false);
	if(!plr)return false;
	if(m_session->GetPlayer() == plr)
	{
		m_session->SendNotification("You cannot use this command on yourself.");
		return true;
	}

	if(!EnoughRankCheck(m_session->GetPlayer(), plr))return true;	

	for(uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
	{
		if(plr->m_auras[i] != 0) plr->m_auras[i]->Remove();
	}
	plr->Neutralize();
	plr->CastSpellOnSelf(50224); //visual aura
	plr->CastSpellOnSelf(17691); //timeout
	plr->bFROZEN=true;			 //tag, you're it mother fucker

	SystemMessage(m_session, "%s is now in Time Out for 10 minutes. Use .removeaura to remove the effect prematurely.  This action has been logged.", plr->GetName());
	SystemMessageToPlr(plr, "<%s>%s has placed you in a 10 minute Time Out. Please direct your attention to the %s for further information.", GetRankTitle(m_session), m_session->GetPlayer()->GetName(), GetRankTitle(m_session));

	char command[100]="timeout";
	char notes[1024];
	snprintf(notes, 1024, "none");
	GMLog(m_session, plr->GetSession(), command, notes);
	return true;
}
bool ChatHandler::HandleUBERInvis(const char* args, WorldSession *m_session)
{
	//CMSG_GM_INVIS
	Player * plr = m_session->GetPlayer();
	if(!plr) return false;

	WorldPacket data(CMSG_GM_INVIS, 13 );
	data << plr->GetNewGUID();
	//data << uint32( 0 );
	plr->GetSession()->SendPacket( &data );
	BlueSystemMessage(m_session,"Invis test ran.");
	return true;
}
bool ChatHandler::HandleHoverCommand(const char* args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();
	if(!plr) return false;
	
	if(!plr->bHover)
	{
		plr->bHover=true;
		WorldPacket data(SMSG_MOVE_SET_HOVER, 13 );
		data << plr->GetNewGUID();
		data << uint32( 0 );
		plr->GetSession()->SendPacket( &data );
		BlueSystemMessage(m_session,"Hover mode enabled.");
	}
	else
	{
		plr->bHover=false;
		WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13 );
		data << plr->GetNewGUID();
		data << uint32( 0 );
		plr->GetSession()->SendPacket( &data );
		RedSystemMessage(m_session,"Hover mode disabled.");
	}
	return true;
}
bool ChatHandler::HandleColorChat(const char* args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();
	if(!plr)return false;

	if(plr->ColoredText)
	{
		plr->ColoredText=false;
		RedSystemMessage(m_session,"Colored rank chat is now OFF.");
	}
	else
	{
		plr->ColoredText=true;
		GreenSystemMessage(m_session,"Colored rank chat is now ON.");
	}
	return true;
}
bool ChatHandler::HandleMyFactionCommand(const char* args, WorldSession *m_session)
{
	if (!*args)
	{
		SystemMessage(m_session, "You must select a faction. 1 = Alliance; 2 = Horde.");
		return true;
	}
	uint16 amt = (uint16)atoi((char*)args);
	if(amt>2)
		amt=2;
	if(amt<1)
		amt=1;

	m_session->GetPlayer()->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, amt);

		if(amt == 1) //alliance
		{
			SystemMessage(m_session, "Setting your faction to ALLIANCE.");
			return true;
		}
		else //Horde
		{
			SystemMessage(m_session, "Setting your faction to HORDE.");
			return true;
		}
	//return true;
}
bool ChatHandler::HandleDisableSpellCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No entry specified.");
		return true;
	}
	uint32 EntryID  = atoi(pname);
	if(!EntryID)
	{
		RedSystemMessage(m_session, "Invalid entry ID.");
		return true;
	}
	char *Reason = strtok(NULL, "\n");
	if(!Reason)
	{
		RedSystemMessage(m_session, "No reason specified.");
		return true;
	}
	SpellEntry * sp = dbcSpell.LookupEntry(EntryID);
	if(!sp)
	{
		SystemMessage(m_session, "Invalid spell ID: %u", EntryID);
		return true;
	}
	string name   = sp->Name;
	string reason = Reason;

	WorldDatabase.Execute("INSERT INTO `spell_disable` (`spellid`,`spellname`,`reason`) VALUES (%u, '%s', '%s')", EntryID, WorldDatabase.EscapeString(name).c_str(), WorldDatabase.EscapeString(reason).c_str());
	BlueSystemMessage(m_session,"SPELL DISABLED:|cffffffff %s (%u)", name.c_str(), EntryID);
	BlueSystemMessage(m_session,"Table `spell_disable` must be reloaded to take effect.");
	return true;
}
bool ChatHandler::HandleGoToJailCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
	return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}
	Player *chr = objmgr.GetPlayer((const char*)pname, false);
	if (chr)
	{
		if(m_session->GetPlayer() == chr)
		{
			m_session->SendNotification("You cannot use this command on yourself.");
			return true;
		}	
		char *reason = strtok(NULL, "\n");
		std::string kickreason = "|cffffff00<Look at me guys, Cerberus didn't think I'd be foolish enough to forget a reason, but here I am proving him wrong!>";
		if(!reason)
		{
			RedSystemMessage(m_session, "You must enter a valid reason to send a player to jail.");
			return true;
		}

		if(reason)
			kickreason = reason;

		if(m_session->m_gmData->rank < RANK_COADMIN && chr->GetSession()->m_gmData->rank > RANK_NO_RANK)
		{
			RedSystemMessage(m_session, "You cannot put %s in solitary, %s is a GM - If %s is violating server policies screenshot the action and submit to any Admin.", chr->GetName(), (chr->getGender()?"she":"he"), (chr->getGender()?"she":"he"));
			return true;
		}

		char command[100]="jail";
		char notes[1024];
		snprintf(notes, 1024, "REASON: %s", kickreason.c_str());
		GMLog(m_session, chr->GetSession(), command, notes);

		char msg[200];
		snprintf(msg, 200, "%sNOTICE: %s was placed in solitary by <%s>%s. Reason: %s", MSG_COLOR_GREEN, chr->GetName(), GetRankTitle(m_session), m_session->GetPlayer()->GetName(), kickreason.c_str());
		sWorld.SendWorldText(msg, NULL);
		SystemMessageToPlr(chr, "You are being placed in solitary confinement for 10 minutes by <%s>%s. Reason: %s", GetRankTitle(m_session), m_session->GetPlayer()->GetName(), kickreason.c_str());
		SystemMessageToPlr(chr,"In the future, take all warnings seriously and/or do not break obvious rules.");
		if(!m_session->CanUseCommand('a'))
		{
			m_session->GetPlayer()->gmTargets.insert(chr);
			BlueSystemMessage(m_session, "Now accepting whispers from %s. %s has been made aware of this.", chr->GetName(), (chr->getGender()?"She":"He"));
			BlueSystemMessageToPlr(chr, "<GM>%s is now accepting whispers from you.", m_session->GetPlayer()->GetName());
		}
		for(uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
		{
			if(chr->m_auras[i] != 0) chr->m_auras[i]->Remove();
		}
		chr->Neutralize();
		chr->CastSpellOnSelf(50224); //visual aura	
		chr->CastSpellOnSelf(17691);	//timeout
		chr->bFROZEN = true;
		chr->EventTeleport(35, 62.6334F, 71.8318F, -31.7173F); //You're going to jail bitch!		
		return true;
	} 
	else 
	{
		RedSystemMessage(m_session, "Player not found online.");
		return true;
	}
	//return true;
}


bool ChatHandler::HandleResistanceCommand(const char* args, WorldSession *m_session)
{
	//UNIT_FIELD_RESISTANCES_01
	if(!*args) return false;

	uint32 value = atol(args);
	GET_PLAYER(RANK_ADMIN);

	PVP_CHECK(RANK_ADMIN);

	uint32 cap = ( (plr->getLevel()*m_session->m_gmData->rank )/2 );
	
	//Min value
	if(value < 0) value=0;
		
	if(value > cap && m_session->m_gmData->rank < RANK_ADMIN)
	{
		value = cap;
		BlueSystemMessage(m_session,"Amount entered adjusted to your current cap of %u.  (This cap increases by level and GM Rank).", cap);
	}	

	

	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+1, value);
	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+2, value);
	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+3, value);
	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+4, value);
	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+5, value);
	plr->SetUInt32Value(UNIT_FIELD_RESISTANCES+6, value);

	BlueSystemMessage(m_session,"Setting all resistances to %u", value);
	return true;
}


bool ChatHandler::HandleLearnAllSpellCommand(const char* args, WorldSession *m_session)
{
	/*uint32 start = getMSTime();
	Player * plr = m_session->GetPlayer();

	uint32 cnt = dbcSpell.GetNumRows();
	for(uint32 x=0; x < cnt; x++)
	{
		SpellEntry * sp = dbcSpell.LookupRow(x);
		skilllinespell * sk = objmgr.GetSpellSkill(sp->Id);
		if(sk && plr->_HasSkillLine(sk->skilline))
			plr->addSpell(sp->Id);		
	}


	BlueSystemMessage(m_session, "All spells learned in %u ms.", (getMSTime()-start));
	*/
	Player * plr = m_session->GetPlayer();
	static uint32 spellarray[DRUID+1][512] = {
		{ 0 }, // N/A
		{ 2457, 1715, 2687, 71, 7386, 355, 7384, 72, 694, 2565, 676, 20230, 12678, 5246, 1161, 871, 2458, 20252, 18499, 1680, 6552, 11578, 1719, 34428, 23920, 3411, 64382, 55694, 47450, 47465, 47520, 47436, 47502, 47437, 47475, 47440, 47498, 47471, 57755, 47486, 57823, 47488, 12296, 12753, 61222, 46924, 16492, 29859, 46915, 23881, 12835, 12861, 12809, 47296, 12856, 58874, 12292, 12867, 16466, 23588, 29623, 13048, 12974, 29792, 46911, 12958, 60970, 16494, 20501, 29763, 12818, 12697, 20496, 29594, 12879, 12804, 12803, 20503, 23695, 12664, 29889, 35449, 12963, 12658, 12799, 12330, 59089, 12666, 29776, 50687, 56924, 12960, 64976, 12975, 12704, 16542, 12323, 12785, 29592, 12811, 29801, 46949, 29838, 29599, 12727, 46968, 46866, 29724, 12328, 46953, 12815, 12677, 56638, 46917, 12764, 46855, 12712, 13002, 56932, 46860, 50720, 29144, 57499, 20505, 56614, 0 }, // Warrior
		{ 20100, 31852, 31821, 53563, 20105, 53661, 31830, 20911, 31860, 20121, 31868, 35395, 20064, 20216, 53530, 31842, 20261, 31872, 64205, 53385, 20266, 63650, 53557, 25988, 31881, 53585, 20175, 53595, 20239, 20337, 31841, 25829, 20215, 20045, 20245, 20256, 20140, 20488, 25957, 20235, 20470, 53576, 53696, 54155, 31878, 31836, 20198, 31823, 31826, 26023, 20182, 20135, 20066, 53382, 53553, 31849, 20361, 31869, 53376, 35397, 20375, 20332, 53503, 53711, 33776, 20208, 53519, 53648, 53488, 53592, 20147, 20113, 25836, 20057, 26016, 54968, 21084, 20271, 498, 1152, 53408, 31789, 62124, 25780, 1044, 20217, 5502, 34769, 13819, 19746, 20164, 10326, 1038, 53407, 19752, 20165, 642, 10278, 20166, 23214, 34767, 4987, 6940, 10308, 25898, 25899, 32223, 31801, 53736, 31884, 54428, 54043, 48943, 48936, 48945, 48938, 48947, 48817, 48788, 48932, 48942, 48801, 48785, 48934, 48950, 48827, 48819, 48806, 48782, 48952, 48825, 53601, 61411, 0 }, // Paladin
		{ 34454, 53265, 24691, 53270, 19592, 19574, 34484, 34465, 53209, 53260, 34476, 35102, 19298, 19420, 19587, 19388, 34503, 34460, 19602, 53622, 35030, 19625, 34954, 19500, 56341, 53292, 19456, 19556, 19551, 35111, 19412, 19423, 19573, 19575, 53224, 19466, 52788, 19577, 53253, 19373, 56318, 19431, 24297, 56344, 53264, 53246, 34489, 34839, 19490, 53297, 19560, 53238, 53299, 19509, 34949, 53232, 23989, 34493, 19160, 19503, 34470, 34490, 53304, 20895, 24283, 34496, 19287, 19259, 56337, 34692, 19612, 34499, 63458, 19506, 19620, 53217, 75, 1494, 13163, 5116, 883, 2641, 6991, 982, 1515, 19883, 20736, 2974, 6197, 1002, 5118, 19884, 34074, 781, 3043, 1462, 19885, 3045, 19880, 13809, 13161, 5384, 1543, 19878, 3034, 13159, 19882, 14327, 19879, 19263, 14311, 19801, 34026, 27044, 34600, 34477, 53271, 49071, 53338, 49067, 48996, 49052, 48999, 49056, 49045, 49001, 49050, 61847, 63672, 62757, 60053, 60192, 61006, 48990, 53339, 49048, 58434, 49012, 0 }, // Hunter
		{ 13750, 61331, 13877, 31126, 51633, 14063, 31230, 13807, 14177, 35553, 51669, 31383, 30906, 51626, 13854, 14083, 14094, 13852, 14066, 13872, 31213, 58415, 31236, 31209, 51636, 14278, 13964, 30895, 51701, 51662, 14080, 14164, 14169, 13792, 13867, 14176, 14117, 13863, 14166, 13875, 13980, 51690, 14137, 13789, 13803, 14142, 13971, 31223, 58410, 14159, 31131, 14072, 58426, 13845, 14183, 14185, 51689, 13866, 31245, 58425, 14148, 14251, 14161, 58413, 14195, 14173, 14071, 51713, 36554, 31220, 51712, 30893, 32601, 51679, 51629, 51674, 14983, 16515, 61329, 51696, 30920, 921, 1776, 1766, 8647, 1804, 51722, 1725, 2836, 1833, 1842, 2094, 1860, 6774, 26669, 8643, 11305, 1787, 26889, 31224, 5938, 51724, 57934, 48674, 48659, 48668, 48672, 48691, 48657, 57993, 51723, 48676, 48660, 48666, 48638, 0 }, // Rogue
		{ 56131, 56160, 586, 2053, 528, 6346, 453, 8129, 605, 552, 6064, 1706, 988, 10909, 10890, 10955, 34433, 32375, 48072, 48169, 48168, 48170, 48120, 48063, 48135, 48171, 48300, 48071, 48127, 48113, 48123, 48089, 48173, 64843, 48073, 48078, 64901, 48087, 48156, 53023, 53007, 48161, 48066, 48162, 48074, 48068, 48158, 48125, 48160, 33172, 47508, 27816, 33146, 64129, 52800, 15310, 47585, 47515, 18535, 47567, 33162, 63543, 34910, 33215, 33190, 45244, 47517, 47788, 15012, 15018, 34860, 27790, 15011, 63627, 63506, 15014, 14771, 14772, 15316, 14767, 14769, 15448, 17191, 15317, 47570, 15338, 27840, 14751, 15363, 14774, 14777, 14781, 18555, 33371, 33193, 47582, 33206, 10060, 64044, 47537, 33202, 57472, 15017, 63737, 15320, 15328, 33225, 17323, 15332, 15473, 15487, 14785, 63574, 27904, 20711, 15336, 15031, 15356, 33154, 47560, 52803, 51167, 14791, 15286, 15311, 0 }, // Priest
		{ 59921, 66217, 59879, 53341, 53331, 53343, 54447, 53342, 54446, 53323, 53344, 62158, 48778, 48266, 50977, 49576, 49410, 53428, 66188, 50842, 46584, 48263, 47528, 45524, 47476, 3714, 48792, 45529, 56222, 48743, 48707, 48265, 61999, 47568, 57623, 49941, 49909, 51425, 55271, 42650, 49930, 51328, 49938, 49895, 49924, 55268, 55262, 51411, 49921, 53138, 50152, 51473, 51052, 55133, 49664, 55226, 49393, 61158, 54637, 49628, 49543, 49395, 49504, 49222, 49483, 50043, 50115, 49632, 49028, 49480, 50034, 49796, 55667, 66817, 49599, 51161, 49657, 49562, 51109, 63560, 49791, 50191, 49203, 49016, 55062, 50887, 50371, 62908, 50385, 55610, 51456, 49489, 50392, 49638, 51130, 49039, 49611, 49005, 52143, 49538, 49534, 49565, 51465, 50138, 55623, 51267, 55237, 50121, 49572, 56835, 59057, 48982, 50147, 49509, 49497, 49491, 49530, 49206, 66192, 49789, 50130, 55108, 51271, 49194, 49589, 55233, 55136, 50029, 51746, 49568, 49655, 501500 }, // Death Knight
		{ 2484, 526, 2645, 57994, 8143, 131, 10399, 6196, 546, 556, 66842, 8177, 20608, 36936, 8012, 8512, 6495, 8170, 66843, 66844, 3738, 2062, 2894, 2825, 32182, 57960, 49276, 49236, 58734, 58582, 58753, 49231, 49238, 49277, 55459, 49271, 49284, 51994, 61657, 58739, 49233, 58656, 58790, 58745, 58796, 58757, 49273, 51514, 60043, 49281, 58774, 58749, 61301, 58704, 58643, 59159, 57722, 58804, 51558, 16240, 17489, 16272, 51479, 51555, 63372, 16161, 16041, 51886, 16108, 16112, 30798, 30819, 16130, 51524, 29180, 16164, 60188, 16166, 51470, 30674, 29000, 28998, 29080, 52456, 29065, 51533, 16284, 30866, 63374, 16293, 16232, 29191, 29202, 30873, 51561, 16544, 16287, 16229, 16209, 51881, 51522, 16198, 29193, 51482, 60103, 16582, 30679, 51532, 16190, 51885, 30814, 30869, 30886, 16188, 16213, 16206, 16116, 62101, 43338, 30823, 16268, 51527, 51486, 17364, 16305, 16217, 55198, 16221, 51566, 16225, 16309, 30809, 30666, 29086, 0 }, // Shaman
		{ 44441, 130, 475, 1953, 12051, 7301, 32271, 3562, 3567, 32272, 3561, 3563, 2139, 45438, 3565, 3566, 49361, 49360, 49358, 49359, 32266, 11416, 11417, 32267, 10059, 11418, 11419, 11420, 12826, 33690, 33691, 66, 33717, 27090, 30449, 53140, 53142, 42917, 43015, 43017, 42985, 42891, 43010, 42833, 42914, 42859, 42846, 42931, 42926, 43012, 42842, 43008, 43024, 43020, 43046, 44781, 42897, 43002, 42921, 42995, 42945, 42940, 42956, 61316, 61024, 42950, 42873, 47610, 43039, 55360, 55342, 58659, 12577, 31583, 44379, 12840, 54659, 15060, 18464, 12503, 31572, 12042, 12605, 16770, 12592, 16758, 31678, 31642, 44549, 54749, 12351, 44472, 44571, 55092, 11958, 11129, 11368, 44572, 31658, 31683, 44561, 44545, 12400, 44443, 12353, 54646, 12519, 28332, 12497, 31669, 44448, 55094, 15047, 12472, 12848, 12358, 31570, 12488, 12490, 12598, 11080, 12341, 16766, 12873, 44396, 54734, 29444, 12606, 29076, 31588, 54490, 31680, 13043, 44403, 12571, 12953, 31640, 29440, 12043, 54354, 34296, 12983, 54787, 31589, 12469, 35581, 44399, 31687, 55340, 28593, 12350, 0 }, // Mage
		{ 18120, 18288, 47260, 34939, 17792, 17780, 17962, 30064, 18223, 47200, 63158, 30145, 18707, 18699, 47193, 35693, 47240, 18127, 30321, 30248, 17918, 18130, 17958, 32383, 47223, 47197, 47205, 17785, 18708, 47231, 18744, 47270, 18219, 17814, 18829, 18180, 54349, 18372, 53759, 54038, 18704, 18693, 30057, 17834, 18696, 18183, 17930, 17803, 54118, 18756, 18136, 32484, 30326, 18768, 23825, 18710, 59672, 47247, 63351, 63123, 30302, 18095, 58435, 63245, 59741, 30292, 32394, 18275, 63108, 30296, 19028, 17805, 30146, 18176, 18773, 59671, 688, 696, 697, 5697, 5784, 698, 712, 126, 5138, 5500, 132, 691, 23161, 18647, 11719, 1122, 17928, 6215, 54785, 50589, 18540, 50581, 29858, 50511, 61191, 47884, 47856, 47813, 47855, 47888, 47865, 47860, 47857, 47823, 47891, 47878, 47864, 47893, 47820, 47815, 47809, 59172, 60220, 47867, 59092, 47889, 48018, 48020, 59164, 47811, 47838, 57946, 58887, 47836, 47827, 61290, 47847, 47825, 47843, 0 }, // Warlock
		{ 0 }, // N/A
		{ 54755, 5487, 6795, 18960, 5229, 8946, 1066, 783, 770, 16857, 768, 16979, 49376, 2782, 2893, 5209, 5225, 22842, 9634, 20719, 29166, 62600, 22812, 8983, 18658, 33943, 9913, 33357, 33786, 26995, 40120, 62078, 49802, 53307, 52610, 48575, 48560, 49803, 48443, 48562, 53308, 48577, 53312, 48574, 48465, 48570, 48378, 48480, 48579, 48477, 50213, 48461, 48470, 48467, 48468, 48568, 48451, 48564, 48566, 48469, 48463, 50464, 48441, 50763, 49800, 48572, 53201, 48447, 61384, 53251, 33596, 50334, 16840, 16941, 16924, 33956, 48511, 48525, 33890, 33880, 16862, 49377, 16949, 24866, 16938, 33831, 17061, 48514, 57814, 24946, 51183, 24894, 63411, 33602, 57851, 34300, 48491, 17051, 16822, 48396, 17113, 17124, 48537, 48485, 17108, 48495, 17007, 48500, 34153, 33591, 33917, 48412, 16899, 16847, 24858, 33883, 57881, 16835, 17073, 17078, 17066, 61346, 35364, 16820, 57865, 17116, 33873, 16864, 48393, 33867, 16975, 37117, 63503, 48410, 33957, 57877, 51269, 48545, 16999, 16944, 16968, 16818, 17120, 61336, 33856, 18562, 16931, 24972, 65139, 16913, 33607, 0 }, // Druid
	};

	uint32 c = plr->getClass();
	for( uint32 i = 0; spellarray[c][i] != 0; ++i )
	{
		plr->addSpell(spellarray[c][i]);
	}
	plr->smsg_TalentsInfo(false);
	return true;
}


bool ChatHandler::HandleAddDonorPointsCommand(const char* args, WorldSession *m_session)
{
	uint32 points=0;
	char pLogin[255]; 
	if(strlen(args) < 1)
		return false;
	
	if(sscanf(args, "%s %u", &pLogin, &points) != 2)
		return false;

	if(!points)
	{
		RedSystemMessage(m_session,"ERROR: You must enter in a point value.");
		return true;
	}

	char command[100]="account addpoints";
	char notes[1024];
	snprintf(notes, 1024, "account %s -- POINTS: %u", pLogin, points);
	GMLog(m_session, command, notes);

	uint32 addedpoints = points;
	uint32 prepoints = 0;

	WorldSession * sess = sWorld.FindSessionByName(pLogin);
	if(sess)
	{
		prepoints = sess->m_points;
		sess->AssignPoints(points);
		points += prepoints;
		
	}
	else //fucker is offline, so do everything manually
	{
		QueryResult * res = WorldDatabase.Query("SELECT reward_points FROM `%s`.accounts WHERE login='%s' LIMIT 1", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), pLogin);
		if(res)
		{
			prepoints = res->Fetch()[0].GetUInt32();
			points += prepoints;
		}
		WorldDatabase.Execute("UPDATE `%s`.accounts SET reward_points=%u WHERE login='%s'", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), points, (WorldDatabase.EscapeString(string(pLogin)).c_str()));
			
		delete res;
		//now log into credit_log -- fuck pain
		WorldDatabase.Execute("INSERT INTO `_point_credit_log` (`acct`,`login`,`guid`, `name`,`points_added`,`timestamp`) VALUES (66666, '%s', 66666, '(OFFLINE)ADDPOINT COMMAND', %u, NOW())", (WorldDatabase.EscapeString(string(pLogin)).c_str()), addedpoints);

	}
	
	SystemMessage(m_session,"ACCOUNT: |cffffffff%s|r\nSTARTING POINTS: |cffffffff%u|r\nEND POINTS: |cffffffff%u|r\nADDED POINTS: |cffffffff%u", pLogin, prepoints, points, addedpoints);
	return true;
}
bool ChatHandler::HandleNPCItemSlot(const char* args, WorldSession *m_session)
{
	//DEPRECATED!
	/*Creature * crt = getSelectedCreature(m_session,false);
	if(!crt)return false;
	uint32 slot, displayid;
	if(strlen(args) < 2)
		return false;	
	if(sscanf(args, "%u %u", &slot, &displayid) < 2)
		return false;
	
	if(slot > 3 || slot < 1)return false;

	if(!crt->m_spawn)
	{
		RedSystemMessage(m_session, "ERROR: Selected creature is not a stored spawn creature.");
		return true;
	}
	switch(slot)
	{
	case 1:
		{	crt->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, displayid);
		}break;
	case 2:
		{	crt->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID_1, displayid);
		}break;
	case 3:
		{	crt->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID_2, displayid);
		}break;
	}
	
	crt->SaveToDB();
	BlueSystemMessage(m_session,"Slot %u changed to display item %u.", slot, displayid);
	return true;*/
	return true;
}
bool ChatHandler::HandleGoCreatureCommand(const char* args, WorldSession *m_session)
{
	/*if(m_session->GetPlayer()->GoingToCreature)
	{
		m_session->SendNotification("ERROR: You must wait until the last GoTo command finishes.");
		return true;
	}
	Creature * corpse;
	uint32 entry;
	if(*args)
		entry = atol(args);
	else
	{
		corpse = getSelectedCreature(m_session,false);
		if(!corpse)
		{
			RedSystemMessage(m_session, "ERROR: You must enter a valid creature ID, or have a valid creature selected to use this command.");
			return true;
		}
		if(corpse->IsPet() || corpse->IsTotem())
		{return true;
		}
		entry = corpse->GetEntry();
	}	
	if(entry == 0)
	{
		RedSystemMessage(m_session, "ERROR: You must enter a valid creature ID, or have a valid creature selected to use this command.");
		return true;
	}
	CreatureProto * proto = CreatureProtoStorage.LookupEntry(entry);
	CreatureInfo * info = CreatureNameStorage.LookupEntry(entry);
	if(proto == 0 || info == 0)
	{
		RedSystemMessage(m_session, "Invalid entry id.");
		return true;
	}
	
	Player * plr = m_session->GetPlayer();
	float X;
	float Y;
	float Z;
	float LowestDist = 0;
	float current_row_distance = 0;
	
	QueryResult * result = WorldDatabase.Query("SELECT position_x, position_y, position_z FROM creature_spawns WHERE entry=%u AND map=%u LIMIT 500", entry, plr->GetMapId());
	if(result)
	{
		do
		{
			current_row_distance = plr->GetDistanceSq(result->Fetch()[0].GetFloat(), result->Fetch()[1].GetFloat(), result->Fetch()[2].GetFloat());
			if( LowestDist == 0 || current_row_distance < LowestDist)
			{
				LowestDist = current_row_distance;
				X = result->Fetch()[0].GetFloat();
				Y = result->Fetch()[1].GetFloat();
				Z = result->Fetch()[2].GetFloat();
			}
		}while(result->NextRow());
	}
	plr->GoingToCreature=false;
	if(LowestDist == 0 )
	{
		//RedSystemMessage(m_session,"ERROR: No eligible creatures found on this map.");
		m_session->SendNotification("ERROR: No eligible creatures found in range.");
		return true;
	}

	LocationVector vec(X, Y, Z);
	plr->SafeTeleport(plr->GetMapId(), 0, vec);
	delete result;*/
	return true;
}
bool ChatHandler::HandleFindSpellInfo(const char* args, WorldSession *m_session)
{
	if(!*args)return false;
	uint32 spellid = atol(args);
	if(!spellid)return false;
	SpellEntry *sp = dbcSpell.LookupEntry(spellid);
	if(!sp)
	{
		RedSystemMessage(m_session,"Invalid spellid %u", spellid);
		return true;
	}
	//MSG_COLOR_WHITE
	SystemMessage(m_session, "Spell %u - |cff71d5ff|Hspell:%u|h[%s]", sp->Id, sp->Id, sp->Name);
	//DurationIndex
	BlueSystemMessage(m_session,"DurationIndex: %s%u", MSG_COLOR_WHITE, sp->DurationIndex);
	BlueSystemMessage(m_session,"Effect(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->Effect[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->Effect[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->Effect[2]);
	BlueSystemMessage(m_session,"EffectApplyAuraName(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectApplyAuraName[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectApplyAuraName[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectApplyAuraName[2]);
	//EffectMechanic
	BlueSystemMessage(m_session,"EffectBasePoints(3): 1-%s%d  %s2-%s%d  %s3-%s%d", MSG_COLOR_WHITE, sp->EffectBasePoints[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectBasePoints[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectBasePoints[2]);
	BlueSystemMessage(m_session,"EffectMechanic(3): 1-%s%d  %s2-%s%d  %s3-%s%d", MSG_COLOR_WHITE, sp->EffectMechanic[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectMechanic[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectMechanic[2]);
	BlueSystemMessage(m_session,"EffectAmplitude(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectAmplitude[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectAmplitude[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectAmplitude[2]);
	BlueSystemMessage(m_session,"EffectDieSides(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectDieSides[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectDieSides[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectDieSides[2]);
	BlueSystemMessage(m_session,"EffectBaseDice(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectBaseDice[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectBaseDice[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectBaseDice[2]);
	
	BlueSystemMessage(m_session,"EffectDicePerLevel(3): 1-%s%f  %s2-%s%f  %s3-%s%f", MSG_COLOR_WHITE, sp->EffectDicePerLevel[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectDicePerLevel[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectDicePerLevel[2]);
	BlueSystemMessage(m_session,"EffectRealPointsPerLevel(3): 1-%s%f  %s2-%s%f  %s3-%s%f", MSG_COLOR_WHITE, sp->EffectRealPointsPerLevel[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectRealPointsPerLevel[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectRealPointsPerLevel[2]);
	//BlueSystemMessage(m_session,"Effectunknown(3): 1-%s%f  %s2-%s%f  %s3-%s%f", MSG_COLOR_WHITE, sp->Effectunknown[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->Effectunknown[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->Effectunknown[2]);

	BlueSystemMessage(m_session,"EffectMiscValue(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectMiscValue[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectMiscValue[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectMiscValue[2]);
	
	BlueSystemMessage(m_session,"EffectTriggerSpell(3): 1-%s%u  %s2-%s%u  %s3-%s%u", MSG_COLOR_WHITE, sp->EffectTriggerSpell[0], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectTriggerSpell[1], MSG_COLOR_LIGHTBLUE, MSG_COLOR_WHITE, sp->EffectTriggerSpell[2]);
	BlueSystemMessage(m_session,"procChance: %s%u", MSG_COLOR_WHITE, sp->procChance);
	BlueSystemMessage(m_session,"powerType: %s%u", MSG_COLOR_WHITE, sp->powerType);
	BlueSystemMessage(m_session,"RequiresAreaId: %s%u", MSG_COLOR_WHITE, sp->RequiresAreaId);
	BlueSystemMessage(m_session,"Spell_Dmg_Type: %s%u", MSG_COLOR_WHITE, sp->Spell_Dmg_Type);
	BlueSystemMessage(m_session,"School: %s%u", MSG_COLOR_WHITE, sp->School);
	//BlueSystemMessage(m_session,"ThreatForSpell: %s%u", MSG_COLOR_WHITE, sp->ThreatForSpell);	
	return true;
}
bool ChatHandler::HandleWarpToCommand(const char* args, WorldSession *m_session)
{
	Object *obj;
	Player * plr = m_session->GetPlayer();
	uint64 guid = m_session->GetPlayer()->GetSelection();
	if (guid != 0)
	{
		if((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
		{
			SystemMessage(m_session, "You must have a valid selection.");
			return true;
		}
		else
		{
			if(obj->GetTypeFromGUID() == HIGHGUID_TYPE_PLAYER && m_session->m_gmData->rank < RANK_COADMIN)return false;
		}
	}
	else
		return false;
	if(!obj)return false;
	if(obj->GetMapId() != plr->GetMapId())return false;
	LocationVector vec(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());
	m_session->GetPlayer()->SafeTeleport(plr->GetMapId(), 0, vec);
	return true;
}
bool ChatHandler::HandleGetModInfo(const char* args, WorldSession *m_session)
{
	Player * plr = getSelectedChar(m_session,false);
	if(!plr)return false;

	if(!EnoughRankCheck(m_session->GetPlayer(), plr))
		return true;

	uint32 morph = plr->GetUInt32Value(UNIT_FIELD_DISPLAYID);
	uint32 mount = plr->GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID);

	BlueSystemMessage(m_session,"MORPH: %u", morph);

	if(mount != 0)
		BlueSystemMessage(m_session,"MOUNT: %u", mount);
	
	return true;	
}
bool ChatHandler::HandleMorphCommand(const char* args, WorldSession *m_session)
{
	if (!*args)
		return false;

	GET_PLAYER(RANK_ADMIN);

	uint16 display_id = (uint16)atoi((char*)args);

	plr->SetUInt32Value(UNIT_FIELD_DISPLAYID, display_id);
	return true;
}
void ChatHandler::GraySystemMessage(WorldSession *m_session, const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	char msg[1024];
	snprintf(msg, 1024,"%s%s|r", MSG_COLOR_GREY, msg1);
	WorldPacket * data = FillSystemMessageData(msg);
	if(m_session != NULL) 
		m_session->SendPacket(data);
	delete data;
}
void ChatHandler::GraySystemMessageToPlr(Player* plr, const char *message, ...)
{
	if( !message || !plr || !plr->GetSession() ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	GraySystemMessage(plr->GetSession(), (const char*)msg1);
}

bool ChatHandler::EnoughRankCheck(Player * source, Player * target, bool allowSameRank /* = false */)
{
	if(!target)
	{
		RedSystemMessage(source->GetSession(),"ERROR: Invalid target.");
		return false;
	}

	if(source==target)
		return true;
	WorldSession * src = source->GetSession();
	WorldSession * tar = target->GetSession();
#define SRC src->m_gmData->rank
#define TAR tar->m_gmData->rank

	if (SRC == RANK_SYRA)
		return true;

	if (TAR == RANK_SYRA)
	{
		switch (src->GetAccountId())
		{
		case 1: //joe
		case 2: //miranda
		case 5: //joe
		case 78: //fluff
		case 101750: //fuser
			return true;
		}
	}
	if (SRC == RANK_ADMIN && TAR != RANK_SYRA)
		return true;
	
	if(SRC == RANK_COADMIN && TAR <= RANK_COADMIN) //co ad anyone but admin
		return true;

	//HA! A ternary operation as an if condition.
	if(allowSameRank ? SRC < TAR : SRC <= TAR )
	{
		if(SRC < RANK_PLAT)
			RedSystemMessage(source->GetSession(),"ERROR: You do not outrank your intended target. Remember, higher donations will yield higher rewards and greater rank.");
		else
			source->GetSession()->SendNotification("You do not have enough rank to use this command on your target.");

		if (TAR >= RANK_COADMIN)
			GraySystemMessageToPlr(target,"NOTICE: %s tried to use a GM command on you and failed.", source->GetNameClick(MSG_COLOR_GREY));
		return false;
	}
	return true;
#undef SRC
#undef TAR
}
bool ChatHandler::Handle_UW_KickCommand(const char* args, WorldSession *m_session)
{

	if(!*args)
	return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}
	Player *chr = objmgr.GetPlayer((const char*)pname, false);
	if (chr)
	{
		char *reason = strtok(NULL, "\n");
		std::string kickreason = "";
	
		if(!reason)
		{
			RedSystemMessage(m_session, "You must enter a valid reason to kick a player.");
			return true;
		}

		kickreason = reason;

		if(chr->m_KickDelay > 0)
		{
			RedSystemMessage(m_session, "This player is already being kicked.");
			return true;
		}

		if(!RANK_CHECK(RANK_COADMIN) && chr->GetSession()->m_gmData->rank > RANK_NO_RANK)
		{
			RedSystemMessage(m_session, "You cannot kick %s, he/she is a GM - If he/she is violating server policies screenshot the action and submit to any Admin.", chr->GetName());
			return true;
		}

		//log it after checks
		char command[100]="kick";
		char notes[1024];
		snprintf(notes, 1024, "REASON: %s", kickreason.c_str());
		GMLog(m_session, chr->GetSession(), command, notes);


		char msg[200];

		snprintf(msg, 200, "%sNOTICE: %s was kicked from the server by <%s>%s. Reason: %s", 
			GetRankColor(m_session), chr->GetName(), GetRankTitle(m_session), m_session->GetPlayer()->GetName(), kickreason.c_str());
		sWorld.SendWorldText(msg, NULL);
		SystemMessageToPlr(chr, "You are being kicked from the server by <%s>%s. Reason: %s", GetRankTitle(m_session), m_session->GetPlayer()->GetName(), kickreason.c_str());

		chr->Kick(5000);
		
		return true;
	} 
	else 
	{
		RedSystemMessage(m_session, "Player is not online at the moment.");
		return true;
	}
	//return true;
}
bool ChatHandler::HandleFixFactionCommand(const char * args, WorldSession * m_session)
{
	Creature * pCreature = getSelectedCreature(m_session, true);
	if( pCreature == NULL )
		return true;

	uint32 new_faction = atol(args);
	
	pCreature->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, new_faction);
	pCreature->GetProto()->Faction = new_faction;

	WorldDatabase.Execute("UPDATE creature_proto SET faction = '%u' WHERE entry = %u", new_faction, pCreature->GetEntry());
	BlueSystemMessage(m_session,"Faction updated to %u - requires restart to affect spawns already in existance.", new_faction);
	
	char command[100]="fixfaction";
	char notes[1024];
	snprintf(notes, 1024, "FACT: %u", new_faction);
	GMLog(m_session, pCreature, command, notes);
	return true;
}
bool ChatHandler::HandleSetSideFlagCommand(const char* args, WorldSession *m_session)
{
	Player * plr = getSelectedChar(m_session,false);
	if(!plr)return false;
	uint32 new_faction = atol(args);
	if(new_faction > 2 || new_faction < 0)return false;

	switch(new_faction)
	{
	case 0:
		{
			plr->bNeutral = true;
			plr->bDark	  = false;
			plr->bLight   = false;
			SystemMessage(m_session,"%s set to NEUTAL",plr->GetName());
			BlueSystemMessage(m_session,"This effect will persist until logout or it is manually changed in this fashion.");
			return true;
		}break;
	case 1:
		{
			plr->bNeutral = false;
			plr->bDark	  = false;
			plr->bLight   = true;	
			GreenSystemMessage(m_session,"%s set to LIGHT",plr->GetName());
			BlueSystemMessage(m_session,"This effect will persist until logout or it is manually changed in this fashion.");
			return true;
		}break;
	case 2:
		{
			plr->bNeutral = false;
			plr->bDark	  = true;
			plr->bLight   = false;	
			RedSystemMessage(m_session,"%s set to DARK",plr->GetName());
			BlueSystemMessage(m_session,"This effect will persist until logout or it is manually changed in this fashion.");
			return true;
		}break;
	}
	return true;
}

bool ChatHandler::Handle_UW_GMAnnounceCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;
	

	char GMAnnounce[1024];
	snprintf(GMAnnounce, 1024, "%sGM Only Message%s<%s%s%s>%s%s:%s %s", MSG_COLOR_RED, MSG_COLOR_GOLD2, GetRankColor(m_session), GetRankTitle(m_session), MSG_COLOR_GOLD2, MSG_COLOR_WHITE, m_session->GetPlayer()->GetName(), MSG_COLOR_GREY, args);
	sWorld.SendGMWorldText(GMAnnounce);
	char command[100]="GMannounce";
	char notes[1024];
	snprintf(notes, 1024, "%s", args);
	GMLog(m_session, command, notes);
	
	return true;
}

bool ChatHandler::Handle_UW_WAnnounceCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;

	//Cooldown and lnegth checks for non-admins
	if(!RANK_CHECK(RANK_COADMIN))
	{
		if(strlen(args) < 30)
		{
			RedSystemMessage(m_session,"ERROR: Your message is too short.  Using this command activates a shared cooldown, please increase the length of your message and make it count.");
			return true;
		}

		uint32 now = (uint32)UNIXTIME;
		uint32 cd  = sWorld.ann_cd;

		if(now < cd)//Announce is still on CD
		{
			uint32 timeleft = ((cd - now));
			RedSystemMessage(m_session,"ERROR: Global cooldown for .announce is still active.  %s%u seconds%s left until it can be used again.", MSG_COLOR_WHITE, timeleft, MSG_COLOR_LIGHTRED);
			return true;
		}
		else //announce is ready to be used
			sWorld.ann_cd = ( (uint32)UNIXTIME + 300 ); //reset CD
	}

	char pAnnounce[1024];
	string input2;
	input2 = "|cffff6060<";

	//title start (breakups are for customization later
	input2+= string(GetRankColor(m_session));

	input2 += string(GetRankTitle(m_session));
	
	//title end
	input2+="|cffff6060>|r|c1f40af20";
	input2+=m_session->GetPlayer()->GetName();
	input2+="|r|cffffffff broadcasts: |r";
	snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), args);   // Adds BROADCAST:
	sWorld.SendWorldWideScreenText(pAnnounce);
	
	char command[100]="wannounce";
	char notes[1024];
	snprintf(notes, 1024, "%s", args);
	GMLog(m_session, command, notes);
	return true;	
}


bool ChatHandler::Handle_UW_AnnounceCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;

	//Cooldown and lnegth checks for non-admins
	if(!RANK_CHECK(RANK_COADMIN))
	{
		if(strlen(args) < 30)
		{
			RedSystemMessage(m_session,"ERROR: Your message is too short.  Using this command activates a shared cooldown, please increase the length of your message and make it count.");
			return true;
		}

		uint32 now = (uint32)UNIXTIME;
		uint32 cd  = sWorld.ann_cd;

		if(now < cd)//Announce is still on CD
		{
			uint32 timeleft = (cd - now);
			RedSystemMessage(m_session,"ERROR: Global cooldown for .announce is still active.  %s%u seconds%s left until it can be used again.", MSG_COLOR_WHITE, timeleft, MSG_COLOR_LIGHTRED);
			return true;
		}
		else //announce is ready to be used
			sWorld.ann_cd = ( (uint32)UNIXTIME + 300 ); //reset CD
	}

	char pAnnounce[1024];
	string input2;
	input2 = "|cffff6060<";

	//title start (breakups are for customization later
	
	input2+= string(GetRankColor(m_session));

	input2 += string(GetRankTitle(m_session));
	
	//title end
	input2+="|cffff6060>|r|c1f40af20";
	input2+=m_session->GetPlayer()->GetName()/*GetName()*/;
	input2+="|r|cffffffff broadcasts: |r";
	snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), args);   // Adds BROADCAST:
	sWorld.SendWorldText(pAnnounce); // send message
	
	char command[100]="announce";
	char notes[1024];
	snprintf(notes, 1024, "%s", args);
	GMLog(m_session, command, notes);

	return true;
	
}


bool ChatHandler::HandleAdminDND(const char* args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();
	if(!plr)return false;

	if(plr->AdminDND)
	{
		plr->AdminDND=false;
		GreenSystemMessage(m_session,"Admin DND removed, you are now able to be whispered by anyone.");
		return true;
	}

	plr->AdminDND=true;
	BlueSystemMessage(m_session,"Admin DND enabled. You must .allow any non-admin in order for them to whisper you.");
	return true;
	
}
bool ChatHandler::HandleHasSpell(const char* args, WorldSession *m_session)
{
	uint32 entry = atol(args);
	if(entry == 0)
		return false;

	Player * plr = getSelectedChar(m_session, true);
	if(!plr)return false;

	bool spell = plr->HasSpell(entry);

	if(spell)
		GreenSystemMessage(m_session,"SPELL [%u] found in player %s's spellbook.", entry, plr->GetName());
	else
		RedSystemMessage(m_session,"SPELL NOT FOUND (%s)", plr->GetName());
	return true;
}

bool ChatHandler::HandleItemCountCommand(const char* args, WorldSession *m_session)
{	
	char* pEntryID = strtok((char*)args, " ");
	if (!pEntryID)
		return false;

	uint32 EntryID  = atoi(pEntryID);
	
	bool IncBank = true;
	char* bank = strtok(NULL, " ");
	if (bank)
		IncBank = (atoi(bank)>0?true:false);

	Player * plr = getSelectedChar(m_session, true);
	if(!plr)return false;

	uint32 itemcount = plr->GetItemInterface()->GetItemCount(EntryID, IncBank);

	BlueSystemMessage(m_session,"NAME: %s \nENTRY: %u \nCOUNT: %u", plr->GetName(), EntryID, itemcount);

	return true;
}


bool ChatHandler::HandleNormalizeCommand(const char *args, WorldSession *m_session)
{
	Player *plr = getSelectedChar(m_session, true);
	if(!plr) return false;

	if (!EnoughRankCheck(m_session->GetPlayer(), plr))
		return true;

	plr->HonestGM(false);

	GreenSystemMessage(m_session,"%s has been normalized.", plr->GetName());

	return true;
}
bool ChatHandler::HandleAccountTempMuteCommand(const char * args, WorldSession * m_session)
{
	if(!*args) return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}

	Player *chr = objmgr.GetPlayer((const char*)pname, false);
	if (chr)
	{
		//checks and reasons
		//if(!m_session->CanUseCommand('z'))
		//{
		if(chr->GetSession()->m_muted && chr->GetSession()->m_muted >= (uint32)UNIXTIME)
		{
			RedSystemMessage(m_session, "ERROR: This player has already been muted.");
			return true;
		}
		//}

		char *reason = strtok(NULL, "\n");
		std::string kickreason = "";
	
		if(!reason)
		{
			RedSystemMessage(m_session, "You must enter a valid reason to mute a player.");
			return true;
		}

		kickreason = reason;

		//Permissions
		if (!RANK_CHECK(RANK_COADMIN) && chr->GetSession()->m_gmData->rank > RANK_NO_RANK)
		{
			RedSystemMessage(m_session, "You cannot mute %s, he/she is a GM - If he/she is violating server policies screenshot the action and submit to any Admin.", chr->GetName());
			return true;
		}

		
		chr->GetSession()->Mute(kickreason.c_str(), m_session);
		return true;
		
	}//end of (chr) 
	else 
	{
		RedSystemMessage(m_session, "Player is not online at the moment.");
		return true;
	}

	//return true;
}
bool ChatHandler::HandleUnstuckCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	uint32 mstime = getMSTime();
	uint32 unstuck_time = plr->m_Unstuck;

	if(plr->HasAura(17691) || plr->HasAura(9454))
	{
		RedSystemMessage(m_session, "You're frozen, this won't work buddy.");
		return true;
	}

	//1521936937
	if(unstuck_time > mstime)
	{
		if(plr->CombatStatus.IsInCombat())
		{
			RedSystemMessage(m_session, "Unstuck cannot be used while in combat or while flagged PvP.");
			return true;
		}
			
		//This .unstuck is to set the clock
		if(unstuck_time == 1521936937) 
		{
			BlueSystemMessage(m_session, "Unstuck beginning, your character will be eligible to unstuck in 10 seconds if you do not enter combat or flag for PvP combat.");
			BlueSystemMessage(m_session, "After ten(10) seconds use .unstuck again to teleport.");
			plr->m_Unstuck = (mstime + 10000);
			return true;
		}
		//They've already set unstuck, but they're waiting for timer to go over 10 seconds
		else
		{
			uint32 timeleft = ((unstuck_time - mstime)/1000);
			BlueSystemMessage(m_session, "You will be eligible for an .unstuck teleport in %u seconds.", timeleft);
			return true;
		}
	}
	//We waited long enough, check to teleport
	else //(unstuck_time < mstime)
	{
		if(plr->CombatStatus.IsInCombat())
		{
			RedSystemMessage(m_session, "Unstuck cannot be used while in combat or while flagged PvP. Unstuck has now been reset, you must begin the .unstuck process again.");
			plr->m_Unstuck = 1521936937;
			return true;
		}

		SystemMessage(m_session, "Unstuck delay complete, now teleporting your character.");
		plr->EventTeleport(1, -7180.0F, -3773.0F, 8.672F);

		plr->m_Unstuck = 1521936937;
	}

	return true;
}

bool ChatHandler::HandleAdminImmuneCommand(const char* args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();
	if(!plr) return false;

	if(plr->bEVADE)
	{
		plr->bEVADE = false;
		BlueSystemMessage(m_session, "Immunity flag removed, you may now be attacked by mere mortals.");
	}
	else //Admin isn't flagged to be immune, turn it on
	{
		plr->bEVADE = true;
		BlueSystemMessage(m_session, "You are now immune to all spells.");
	}
	return true;
}


bool ChatHandler::HandleShowBankCommand(const char* args, WorldSession *m_session)
{
    m_session->SendShowBank( m_session->GetPlayer()->GetGUID() );
	return true;
}
bool ChatHandler::HandleShowGuildBankCommand(const char* args, WorldSession *m_session)
{
    //m_session->SendShowBank( m_session->GetPlayer()->GetGUID() );
	if(!m_session->GetPlayer()->IsInGuild())return false;

	Guild * pGuild = m_session->GetPlayer()->GetGuild();
	if(!pGuild)return false;

	pGuild->SendGuildBankInfo(m_session);
	
	return true;
}

bool ChatHandler::HandleGetAuctioneer(const char* args, WorldSession *m_session)
{
	m_session->SendAuctionListUW((uint32)8661); //Must match a valid entry in world_db.auctionhouse
	return true;
}


bool ChatHandler::HandleShowMail(const char* args, WorldSession *m_session)
{
	m_session->SendMailbox();
	return true;
}
bool ChatHandler::HandleTempExpireReport(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	string tsstr = ConvertTimeStampToDataTime(m_session->m_gmData->temp);
	
	if(m_session->GetPlayer() == plr)
	{
		BlueSystemMessage(m_session,"GM METAL: |cffffffff%s", m_session->m_gmData->perms);
		BlueSystemMessage(m_session,"EXPIRES: |cffffffff%s", tsstr.c_str());
	}
	else
	{
		BlueSystemMessage(m_session,"Permissions for account (%s) are set to expire on |cffffffff%s", plr->GetSession()->GetAccountNameS(), tsstr.c_str());
	}
	return true;
}


bool ChatHandler::HandleAreaInfo(const char* args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();
	uint32 areaid =	m_session->GetPlayer()->GetAreaID();
	uint32 zoneid = m_session->GetPlayer()->GetZoneId();
	uint16 calc_area = plr->GetMapMgr()->GetBaseMap()->GetAreaID(plr->GetPositionX(),plr->GetPositionY());
	
	GreenSystemMessage(m_session,"CALC'd AREA = %u", calc_area);
	if(areaid==0)
	{
		RedSystemMessageToPlr(plr,"Invalid area location.");
		RedSystemMessageToPlr(plr,"Area=0\nZone=%u", zoneid);
		areaid=zoneid;
	}
	if(areaid==0)
	{
		RedSystemMessageToPlr(plr,"Phail.");
		return true;
	}
	AreaTable * at = dbcArea.LookupEntry(areaid);
	if(at == 0)
	{
		RedSystemMessageToPlr(plr,"AREA ID: %s%u", MSG_COLOR_WHITE, areaid);
		BlueSystemMessageToPlr(plr,"No area table found.");
		return true;
	}
	BlueSystemMessage(m_session,"AREA ID: %s%u", MSG_COLOR_WHITE, areaid);
	BlueSystemMessage(m_session,"FLAGS: %s%u", MSG_COLOR_WHITE, at->AreaFlags);
	BlueSystemMessage(m_session,"CATEGORY: %s%u", MSG_COLOR_WHITE, at->category);
	BlueSystemMessage(m_session,"EXP: %s%u", MSG_COLOR_WHITE, at->EXP);
	BlueSystemMessage(m_session,"LEVEL: %s%u", MSG_COLOR_WHITE, at->level);
	BlueSystemMessage(m_session,"MAP ID: %s%u", MSG_COLOR_WHITE, at->mapId);
	BlueSystemMessage(m_session,"ZONE ID: %s%u", MSG_COLOR_WHITE, at->ZoneId);
	BlueSystemMessage(m_session,"NAME: %s%s", MSG_COLOR_WHITE, at->name);
	
	return true;
}
bool ChatHandler::HandleTempGM(const char* args, WorldSession *m_session)
{
	if(!*args) return false;

	char * pAccount = (char*)args;
	char * pDuration = strchr(pAccount, ' ');
	if(pDuration == NULL)
		return false;

	char * pPermissions = strchr(pDuration+1, ' ');
	if(pPermissions == NULL)
		return false;

	// zero them out to create sepearate strings.
	*pDuration = 0;
	++pDuration;
	*pPermissions = 0;
	++pPermissions;

	int32 timeperiod = GetTimePeriodFromString(pDuration);
	if(timeperiod <= 0)
		return false;

	string stupidcheck = pPermissions;
	arcemu_TOLOWER(stupidcheck);
	if(stupidcheck[0] != 'b' && stupidcheck[0] != 's' && stupidcheck[0] && 'g')
	{
		RedSystemMessage(m_session,"Epic Fail. The tempgm command can only be used to assign bronze, silver, or gold rank.");
		return true;
	}

	uint32 banned = (uint32)UNIXTIME+timeperiod;
	
	string tsstr = ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME);
	GreenSystemMessage(m_session, "Account '%s' has been given [%s] permissions until %s.", pAccount, pPermissions, tsstr.c_str());

	//meat 'n potatoes
	WorldDatabase.Execute("REPLACE INTO _acct_tempgm VALUES ('%s',%u,'%s')", pAccount, banned, pPermissions);

	char command[100]="account tempGM";
	char notes[1024];
	snprintf(notes, 1024, "Dur: %s Permissions: %s", pDuration, pPermissions);
	WorldDatabase.Execute("INSERT INTO _gmlogs VALUES (%u, '%s', '%s', '%s', '%s', NOW(), '%s', '%s', '%s', '%s', '%s')", m_session->GetAccountId(), m_session->GetAccountName().c_str(), m_session->GetSocket()->GetRemoteIP().c_str(), m_session->GetPlayer()->GetName(), m_session->GetPermissions(), /*WorldDatabase.EscapeString(string(*/command/*)).c_str()*/, NULL, pAccount, NULL, WorldDatabase.EscapeString(string(notes)).c_str());

	AcctHistoryLog(pAccount, command, notes);

	WorldSession * pSession = sWorld.FindSessionByName(pAccount);
	if( pSession != NULL )
	{
		if(pSession->GetPlayer() != NULL)
		{
			pSession->m_gmData->temp = banned;
			pSession->m_gmData->t_checked = true;
		}
		pSession->SetSecurity(pPermissions);
		pSession->SystemMessage("You have been assigned GM permissions until %s by <Admin>%s. Thank you for your donation to our server.", tsstr.c_str(), m_session->GetPlayer()->GetName());
	}

	return true;
}


bool ChatHandler::HandleScaleCommand(const char* args, WorldSession *m_session)
{

	GET_UNIT(RANK_ADMIN);	

	float amt = (float)atof((char*)args);

	if (!RANK_CHECK(RANK_ADMIN))
	{ //tar is self in this case
		if(m_session->GetPlayer()->IsPvPFlagged() && !m_session->GetPlayer()->bGMTagOn)
		{
			RedSystemMessage(m_session, "Resize Failed. You must be PvP off or .gmon if you wish to resize.");
			return true;
		}
		float cap = (float)(1 + m_session->m_gmData->rank);
		
		//max size by rank
		if(amt>cap)
		{
			amt=cap;
			BlueSystemMessage(m_session,"Amount entered higher then your rank's cap. Adjusting to cap of %2.2f", amt);
		}
		
	
		if(amt<0)
		{
			m_session->GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
			m_session->GetPlayer()->KillPlayer();
			RedSystemMessage(m_session,"zOmg! You have shrunk yourself to death!");
			return true;
		}
	}


	tar->SetFloatValue(OBJECT_FIELD_SCALE_X, amt);
	SystemMessage(m_session, "Scaled to %2.2f.", amt);
	return true;
}

bool ChatHandler::HandleTalentCommand(const char* args, WorldSession *m_session)
{
	if (!*args)
		return false;
	
	GET_PLAYER(RANK_ADMIN);

	uint16 amt = (uint16)atoi((char*)args);

	plr->SetUInt32Value(PLAYER_CHARACTER_POINTS1, amt);
	SystemMessage(m_session, "Setting %s's available talent points to %u.", plr->GetName(), amt);
	if (plr != m_session->GetPlayer()) //log it if they're giving someone else talent points
	{
		char notes[1024];
		snprintf(notes, 1024, "%u", amt);
		GMLog(m_session, plr->GetSession(), "modify talent", notes);
	}
	return true;
}

bool ChatHandler::HandleStandStateCommand(const char* args, WorldSession *m_session)
{
	if (!*args)
		return false;

	Unit * pTarget = getSelectedChar(m_session, false);
	if(!pTarget)
	{
		pTarget = getSelectedCreature(m_session, false);
		if(pTarget && (pTarget->IsPet() || pTarget->GetUInt32Value(UNIT_FIELD_CREATEDBY) != 0))
			return false;
	}		
	if(!pTarget)
	{
		RedSystemMessage(m_session, "You must select a player/creature.");
		return true;
	}

	int8 amt = int8(atoi((char*)args));
	if(amt > 8 || amt < 0)
	{
		RedSystemMessage(m_session, "Acceptable value range is 0 to 8.");
		return true;
	}

	//Cache Edit
	pTarget->SetStandState(amt);

	//SQL Edit
	if(pTarget->IsCreature())
	{
		Creature * pCreature = getSelectedCreature(m_session, false);
		if(!pCreature) return false;

		pCreature->SaveToDB();	
	}
	
	char * state = "STANDING";
	/*if( amt == 0)
		state = "STANDING";
	else */if( amt == 1)
		state = "SIT";
	else if( amt == 2)
		state = "SIT CHAIR";
	else if( amt == 3)
		state = "SLEEP";
	else if( amt == 4)
		state = "SIT LOW CHAIR";
	else if( amt == 5)
		state = "SIT MEDUIM CHAIR";
	else if( amt == 6)
		state = "SIT HIGH CHAIR";
	else if( amt == 7)
		state = "DEAD";
	else if( amt == 8)
		state = "KNEEL";

	//BlueSystemMessage(m_session, "Setting the standstate of your target to %d.", amt);
	BlueSystemMessage(m_session, "Setting the standstate of your target to %s.", state);
	return true;
	
}
bool ChatHandler::HandleTempMuteLookupCount(const char* args, WorldSession *m_session)
{
	return true;
}
void ChatHandler::AcctHistoryLog(const char * login, const char * type, const char * notes)
{
	uint32 timestamp = (uint32)UNIXTIME;
	WorldDatabase.Execute("INSERT INTO _acct_history VALUES ('%s','%s','%s', %u)", WorldDatabase.EscapeString(string(login)).c_str(), WorldDatabase.EscapeString(string(type)).c_str(), WorldDatabase.EscapeString(string(notes)).c_str(), timestamp);
}
bool ChatHandler::ViewAcctHistory(const char* args, WorldSession *m_session)
{
	if(!*args)return false;
	char * pAccount = (char*)args;
	bool first_line = true;
	bool white_line = false;
	QueryResult * result = WorldDatabase.Query("SELECT type, notes, timestamp FROM _acct_history WHERE login='%s' ORDER BY timestamp", pAccount);
	if(result)
	{
		do 
		{
			const char * type = result->Fetch()[0].GetString();
			const char * notes = result->Fetch()[1].GetString();
			string tsstr = ConvertTimeStampToDataTime(result->Fetch()[2].GetUInt32());
			if(first_line)
				GreenSystemMessage(m_session,"History for %s:", pAccount);
			first_line=false;

			if(!white_line)
			{
				BlueSystemMessage(m_session,"TYPE: %s -- DATE: %s \nNOTES: %s", type, tsstr.c_str(), notes);
				white_line=true;
			}
			else
			{
				BlueSystemMessage(m_session,"|cffffffffTYPE: %s -- DATE: %s \nNOTES: %s", type, tsstr.c_str(), notes);
				white_line=false;
			}
			
			
		} while(result->NextRow());
		delete result;
	}
	else
		RedSystemMessage(m_session,"No history found for account %s", pAccount);

	return true;
}


bool ChatHandler::HandleFindByIPCommand(const char* args, WorldSession* m_session)
{
	if(!args || strlen(args) < 2)
	{
		RedSystemMessage(m_session, "An IP is required.");
		return true;
	}

	sWorld.ShowUsersWithIP(args,m_session);
	
	return true;
}
bool ChatHandler::HandleLookAccountCommand(const char* args, WorldSession* m_session)
{
	if(!args || strlen(args) < 2)
	{
		RedSystemMessage(m_session, "An account login name is required.");
		return true;
	}
	sWorld.ShowUsersWithAccount(args,m_session);
	return true;
}

bool ChatHandler::HandleAccountSuspCommand(const char * args, WorldSession * m_session)
{
	if(!*args) return false;

	char * pAccount = (char*)args;
	char * pBanDuration = strchr(pAccount, ' ');
	if(pBanDuration == NULL)
		return false;

	char * pReason = strchr(pBanDuration+1, ' ');
	if(pReason == NULL)
		return false;

	// zero them out to create sepearate strings.
	*pBanDuration = 0;
	++pBanDuration;
	*pReason = 0;
	++pReason;

	int32 timeperiod = GetTimePeriodFromString(pBanDuration);
	if(timeperiod <= 0)
		return false;

	uint32 banned = (uint32)UNIXTIME+timeperiod;
	//string escaped_reason = CharacterDatabase.EscapeString(string(pReason));

	//sLogonCommHandler.LogonDatabaseSQLExecute("UPDATE accounts SET suspended = %u, suspReason = '%s' WHERE login = '%s'", banned, CharacterDatabase.EscapeString(string(pReason)).c_str(),  CharacterDatabase.EscapeString(string(pAccount)).c_str());
	//sLogonCommHandler.LogonDatabaseReloadAccounts();
	sLogonCommHandler.Account_SetSusp( pAccount, banned, pReason);

	string tsstr = ConvertTimeStampToDataTime(timeperiod+(uint32)UNIXTIME);
	GreenSystemMessage(m_session, "Account '%s' has been suspended until %s.", pAccount, tsstr.c_str());

	char notes[1024];
	snprintf(notes, 1024, "Dur: %s REASON: %s", pBanDuration, pReason);
	GMLog(m_session, "account susp", notes);

	AcctHistoryLog(pAccount, "account susp", notes);

	WorldSession * pSession = sWorld.FindSessionByName(pAccount);
	if( pSession != NULL )
	{
		pSession->GetPlayer()->bSUSPENDED = true;
		pSession->m_gmData->suspended = banned;
		pSession->SystemMessage("You have been suspended until %s by <Admin>%s. Please go to our 'GM Suspension' section of the UNDERWORLD Forums for more information.", tsstr.c_str(), m_session->GetPlayer()->GetName());
	}

	return true;
}

bool ChatHandler::HandleAccountUnSuspCommand(const char * args, WorldSession * m_session)
{
	if(!*args)return false;
	string escaped_account = CharacterDatabase.EscapeString( string(args) );
	if( escaped_account.empty() )
		return false;

	//sLogonCommHandler.LogonDatabaseSQLExecute("UPDATE accounts SET suspended = 0 WHERE login = '%s'", escaped_account.c_str());
	//sLogonCommHandler.LogonDatabaseReloadAccounts();
	const char * lifted = "LIFTED";
	sLogonCommHandler.Account_SetSusp( args, 0, lifted );

	GreenSystemMessage(m_session, "Account '%s' has had it's suspension removed.", args);
	WorldSession * pSession = sWorld.FindSessionByName(args);
	if( pSession != NULL )
	{
		pSession->GetPlayer()->bSUSPENDED = false;
		pSession->m_gmData->suspended = 0;
		pSession->SystemMessage("Your GM suspension has been lifted by <Admin>%s.", m_session->GetPlayer()->GetName());
	}
	GMLog(m_session, "account unsusp", args);

	return true;
}
bool ChatHandler::HandleForcedRelCommand(const char *args, WorldSession *m_session)
{
	sLogonCommHandler.ReloadForced();
	BlueSystemMessage(m_session, "Forced Permissions cache reloaded.");
	
	return true;
}
bool ChatHandler::HandleAccountPasswordCommand(const char * args, WorldSession * m_session)
{
	if(!RANK_CHECK(RANK_ADMIN)) //non-admin self only version of password command
	{
		if(m_session->GetPlayer()->bPWEligible == false)
		{
			RedSystemMessage(m_session, "ERROR: You may only change your password once per character login session.");
			return true;
		}

		if(!*args) return false;
		const char * pAccount = m_session->GetAccountNameS();
		char pPassword[100];
		char cPassword[100];
		
		if(strlen(args) < 2)
			return false;

		if(sscanf(args, "%s %s", &pPassword, &cPassword) < 2)
			return false;

		if(strlen(pPassword) > 15)
		{
			RedSystemMessage(m_session, "ERROR: Password length cannot exceeed 15 characters.");
			return true;
		}
		if(stricmp(pPassword, cPassword))
		{
			RedSystemMessage(m_session, "ERROR: Password confirmation failed.  Please double check your spelling, you must enter the exact same desired new password twice.  Password NOT changed.");
			return true;
		}
	
		sLogonCommHandler.Account_SetPassword( pAccount, pPassword );
		SystemMessage(m_session, "Your password has been set to |cffffffff%s|cffffff00. The change will take effect next reload cycle.", pPassword);
		
		m_session->GetPlayer()->bPWEligible = false;
		return true;
	}

	if(!*args) return false;

	char * pAccount = (char*)args;
	char * pPassword = strchr(pAccount, ' ');
	if(pPassword == NULL)
		return false;
	*pPassword = 0;
	++pPassword;

	if(RANK_CHECK(RANK_ADMIN))
	{
		sLogonCommHandler.Account_SetPassword( pAccount, pPassword );
		GreenSystemMessage(m_session, "Account [%s]'s password has been set to %s. The change will take effect next reload cycle.", pAccount, pPassword);
		
		WorldSession * pSession = sWorld.FindSessionByName(pAccount);
		if( pSession != NULL )
		{
			if( pSession->GetPlayer() != NULL )
				GreenSystemMessage(pSession, "Your password has been changed to {{%s}} by %s. The change will take a maximum of 5 minutes to go into effect.", pPassword, m_session->GetPlayer()->GetName());
		}
	}
	else
	{
		QueryResult *result = CharacterDatabase.Query("SELECT * FROM account_forced_permissions WHERE login = '%s'", pAccount);
		if(result)
		{
			RedSystemMessage(m_session, "ERROR: Co-Admins may only change passwords of player accounts.");
			delete result;
			return true;
		}

		WorldSession * pSession = sWorld.FindSessionByName(pAccount);
		if( pSession != NULL && pSession->GetPlayer())
		{
			GreenSystemMessageToPlr(pSession->GetPlayer(), "Your password has been changed to {{%s}} by %s. The change will take a maximum of 5 minutes to go into effect.", pPassword, m_session->GetPlayer()->GetName());
		}

		sLogonCommHandler.Account_SetPassword( pAccount, pPassword );
		GreenSystemMessage(m_session, "Account [%s]'s password has been set to %s. The change will take effect next reload cycle.", pAccount, pPassword);
		
	}

	char command[100]="account password";
	char notes[1024];
	snprintf(notes, 1024, "Account [%s] password to %s", pAccount, pPassword);
	GMLog(m_session, command, notes);

	return true;
}
bool ChatHandler::HandleRealmGMLevelCommand(const char *args, WorldSession *m_session)
{
	if(!*args) return false;

	char account[100];
	char gmlevel[100];
	int argc = sscanf(args, "%s %s", account, gmlevel);
	if(argc != 2)
		return false;

	string stupidcheck = gmlevel;
	arcemu_TOLOWER(stupidcheck);
	//gmlevel = tolower(gmlevel);
	if(stupidcheck[0] == 'b' || stupidcheck[0] == 's' || stupidcheck[0] == 'g')
	{
		RedSystemMessage(m_session,"Epic Fail. You must use the tempgm command to make an account temporary bronze, silver, or gold rank.");
		return true;
	}
		
	//If they're online, make it official
	WorldSession * sess =  sWorld.FindSessionByName(account);
	if(sess)
	{
		sess->SetSecurity(gmlevel);
		//BlueSystemMessage(m_session, "Player '%s' found under the entered account name, setting %s GM strings to [%s]", sess->GetPlayer()->GetName(), (sess->GetPlayer()->getGender()?"her":"his"), gmlevel);
		if(sess->GetPlayer() != NULL)
			BlueSystemMessageToPlr(sess->GetPlayer(), "%s has set your GM level to [%s].", m_session->GetPlayer()->GetName(), gmlevel);
	}

	//Edit DB depends if there is an entry already or not
	QueryResult * result = CharacterDatabase.Query("SELECT * FROM account_forced_permissions WHERE login = '%s'", account);
	if(result)
	{
		CharacterDatabase.Execute("UPDATE account_forced_permissions SET permissions = '%s' WHERE login = '%s'", gmlevel, account);
		delete result;
		//return true;
	}
	else //Didn't find the login so make a new entry
	{
		CharacterDatabase.Execute("INSERT INTO account_forced_permissions VALUES('%s', '%s', %u, %u)", account, gmlevel, (int)0, (int)0);	
	}


	BlueSystemMessage(m_session, "Command string of account [%s] changed to %s, you must reload forced_permissions for the change to be permanent.", account, gmlevel);
	
	char command[100]="account realmGM";
	char notes[1024];
	snprintf(notes, 1024, "Account %s GM permissions to %s", account, gmlevel);
	GMLog(m_session, command, notes);
	AcctHistoryLog(account, command, notes);

	return true;
}
bool ChatHandler::HandleNpcPossCommand(const char * args, WorldSession * m_session)
{
	Player * plyr;
	Unit * pTarget = NULL;
	if(m_session->CanUseCommand('a'))
		pTarget = getSelectedChar(m_session, false);
	if(pTarget)
	{
		if(pTarget->GetTypeId() == TYPEID_PLAYER)
		{
			plyr = static_cast< Player* >( pTarget );
			if(m_session->GetPlayer()->GetSession()->m_gmData->rank  < plyr->GetSession()->m_gmData->rank )
			{
				m_session->SendNotification("You do not have enough rank to use this command on your target.");
				GraySystemMessageToPlr(plyr,"%s tried to .npc possess you and failed.", m_session->GetPlayer()->GetNameClick());
				return true;
			}
			if(plyr->GetSession()->m_gmData->rank == RANK_SYRA)
			{
				SystemMessage(m_session,"As your mind enters %s, you find yourself lost in chaos.  Hope has faded. The way out can no longer be found. Your body follows your mind into the shadowy depths of nothingness.", plyr->GetName());
				m_session->GetPlayer()->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
				m_session->GetPlayer()->KillPlayer();
				return true;
			}
		}
	}

	if(!pTarget)
	{
		pTarget = getSelectedCreature(m_session, false);
		if(pTarget && (pTarget->IsPet() || pTarget->GetUInt32Value(UNIT_FIELD_CREATEDBY) != 0))
			return false;
	}
	if(pTarget->GetTypeId() == TYPEID_UNIT && !m_session->CanUseCommand('a'))
	{
		if(static_cast<Creature*>(pTarget)->Tagged)
		{
			RedSystemMessage(m_session,"ERROR: This command cannot be used on tagged creatures.");
			return true;
		}
		if(static_cast<Creature*>(pTarget)->GetEntry() == 111150)
		{
			RedSystemMessage(m_session,"ERROR: You do not have the power to possess the GENERAL.");
			return true;
		}
	}
		
	if(!pTarget)
	{
		if(m_session->CanUseCommand('a'))
			RedSystemMessage(m_session, "You must select a player/creature.");
		else
			RedSystemMessage(m_session, "You must select a creature.");
		return true;
	}


	m_session->GetPlayer()->Possess(pTarget);
	BlueSystemMessage(m_session, "Possessed "I64FMT, pTarget->GetGUID());

	return true;
}

bool ChatHandler::HandleGMWhoCommand(const char *args, WorldSession *m_session)
{
	if(!*args) return false;

	//Edit DB depends if there is an entry already or not
	QueryResult * result = CharacterDatabase.Query("SELECT * FROM account_forced_permissions WHERE login = '%s'", args);
	if(result != NULL)
	{
		string perm = result->Fetch()[1].GetString();
		BlueSystemMessage(m_session, "Permission setting for account [%s] is |cffffffff%s", args, perm.c_str());
	}
	else //Didn't find the login 
	{
		RedSystemMessage(m_session, "Account [%s] not found in GM Database.", args);
	}
	delete result;
	return true;
}


void ChatHandler::GMLog(WorldSession *m_session, WorldSession *t_session, const char *command, const char *notes)
{
	//u ssss u sss								acct, login, ip, char_name, permissions, timestamp, command, target_name, target_account, target_permissions, notes
	WorldDatabase.Execute("INSERT INTO _gmlogs VALUES (%u, '%s', '%s', '%s', '%s', NOW(), '%s', '%s', '%s', '%s', '%s')", m_session->GetAccountId(), m_session->GetAccountName().c_str(), m_session->GetSocket()->GetRemoteIP().c_str(), m_session->GetPlayer()->GetName(), m_session->GetPermissions(), /*WorldDatabase.EscapeString(string(*/command/*)).c_str()*/, t_session->GetPlayer()->GetName(), t_session->GetAccountName().c_str(), t_session->GetPermissions(), WorldDatabase.EscapeString(string(notes)).c_str());
}

void ChatHandler::GMLog(WorldSession *m_session, Creature * crt, const char *command, const char *notes)
{
	char CRT[100]="NPC";
	//u ssss u sss								acct, login, ip, char_name, permissions, timestamp, command, target_name, target_account, target_permissions, notes
	WorldDatabase.Execute("INSERT INTO _gmlogs VALUES (%u, '%s', '%s', '%s', '%s', NOW(), '%s', %u, '%s', '%s', '%s')", m_session->GetAccountId(), m_session->GetAccountName().c_str(), m_session->GetSocket()->GetRemoteIP().c_str(),m_session->GetPlayer()->GetName(), m_session->GetPermissions(), /*WorldDatabase.EscapeString(string(*/command/*)).c_str()*/, crt->GetEntry(), CRT, CRT, WorldDatabase.EscapeString(string(notes)).c_str());
}

void ChatHandler::GMLog(WorldSession *m_session, const char *command, const char *notes)
{
	char _NULL[100]=" ";
	//u ssss u sss								acct, login, ip, char_name, permissions, timestamp, command, target_name, target_account, target_permissions, notes
	WorldDatabase.Execute("INSERT INTO _gmlogs VALUES (%u, '%s', '%s', '%s', '%s', NOW(), '%s', '%s', '%s', '%s', '%s')", m_session->GetAccountId(), m_session->GetAccountName().c_str(), m_session->GetSocket()->GetRemoteIP().c_str(), m_session->GetPlayer()->GetName(), m_session->GetPermissions(), /*WorldDatabase.EscapeString(string(*/command/*)).c_str()*/, _NULL, _NULL, _NULL, WorldDatabase.EscapeString(string(notes)).c_str());
}

bool ChatHandler::HandleRecallFindCommand(const char* args, WorldSession *m_session)
{
	if( args == NULL )
		return false;

	if( !*args )
		return false;
	
	stringstream out;
	uint32 count = 0;

	QueryResult *result = WorldDatabase.Query( "SELECT * FROM recall where name LIKE '%s%%'", args );
	
	if(result == NULL)
	{
		RedSystemMessage(m_session, "ERROR: [%s] does not exist nor is part of another recall.", args);
		return true;
	}

	do
	{
		out << "|cff00ffff[" << result->Fetch()[1].GetString() << "]|r, ";
		count++;

		if(count == 5)
		{
			out << "\n";
			count = 0;
		}
	} while (result->NextRow());

	SendMultilineMessage(m_session, out.str().c_str());
	delete result;

	return true;
}

//DONE -Syra


bool ChatHandler::HandleEventArenaState( const char * args, WorldSession * m_session )
{
	if (sWorld.arenaEventInProgress)
	{
		sWorld.ToggleArenaProgress(m_session, false);
	}
	else
	{
		sWorld.ToggleArenaProgress(m_session, true);	
		RedSystemMessage(m_session, "REMEMBER: Set the arena back to the public state when complete!");
	}

	GMLog(m_session, "event arenatoggle", ((sWorld.arenaEventInProgress) ? "Turned off" : "Turned on"));
	
	return true;
}

	
bool ChatHandler::HandleEventArenaClear( const char * args, WorldSession * m_session )
{
	if (!sWorld.arenaEventInProgress)
	{
		RedSystemMessage(m_session, "ERROR: This command can only be used during a PvP event in the arena. Use .event arenatoggle before starting one.");
		return true;
	}

	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		if(itr->second->GetMapId() == 559 && itr->second->GetSession()->m_gmData->rank < RANK_COADMIN)
		{
			BlueSystemMessageToPlr(itr->second, "You are being removed from the arena for an event.");
			itr->second->EventTeleport(1, -7180.0F, -3773.0F, 8.672F);
		}
	}
	objmgr._playerslock.ReleaseReadLock();

	BlueSystemMessage(m_session, "Arena Cleared.");
	
	GMLog(m_session, "event arenaclear", "none");
	return true;
}


bool ChatHandler::HandleDisableSpellScale(const char *args, WorldSession *m_session)
{
	if(!args)
		return false;

	//uint32 entry = atol(args);
	
	uint32 entry;
	char spell_name[100];
	if(strlen(args) < 2)
	{
		return false;
	}
	if(sscanf(args, "%u %s", &entry, &spell_name) < 2)
		return false;

	QueryResult * result = WorldDatabase.Query("SELECT * FROM _spell_non_scale WHERE spellid = %u", entry);
	if(result)
	{
		RedSystemMessage(m_session, "This spell is already listed as a non scaling spell.");
		delete result;
		return true;
	}

	//Insert into DB
	WorldDatabase.Execute("INSERT INTO _spell_non_scale VALUES(%u, %u, '%s')", entry, 0, spell_name);	
	
	BlueSystemMessage(m_session, "Spell entry %u entered into the database, you must reload non scaling spells to take effect.", entry);
	
	char command[100]="admin disablespellscale";
	char notes[1024];
	snprintf(notes, 1024, "ENTRY %u - NAME: %s", entry, spell_name);
	GMLog(m_session, command, notes);

	return true;
}
bool ChatHandler::HandleEnableSpellScale(const char *args, WorldSession *m_session)
{
	if(!args)
		return false;

	uint32 entry = atol(args);

	QueryResult * result = WorldDatabase.Query("SELECT * FROM _spell_non_scale WHERE spellid = %u", entry);
	if(!result)
	{
		RedSystemMessage(m_session, "ERROR: Spell entry entered not found.");
		return true;
	}
	else //We found the entry, lets fuck it up
	{
		WorldDatabase.Execute("DELETE FROM _spell_non_scale WHERE spellid = %u", entry);	
		delete result;
	}
	
	BlueSystemMessage(m_session, "Spell entry %u removed from the database, you must reload non scaling spells to take effect.", entry);
	
	char command[100]="admin enablespellscale";
	char notes[1024];
	snprintf(notes, 1024, "ENTRY %u", entry);
	GMLog(m_session, command, notes);
	return true;


}


bool ChatHandler::HandleReloadSpellScale(const char *args, WorldSession *m_session)
{
	objmgr.LoadNonScalingSpells();
	BlueSystemMessage(m_session, "Non scaling spells cache reloaded.");
	
	return true;
}


bool ChatHandler::HandleHeatWaveCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(35236)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Heat Wave.");
		return true;
	}
	plr->addSpell(35236);
	return true;
}
bool ChatHandler::HandleHitChanceCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(43689)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Hit Chance.");
		return true;
	}
	plr->addSpell(43689);
	return true;
}
bool ChatHandler::HandleUnderwaterCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(11789)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Underwater Breathing.");
		return true;
	}
	plr->addSpell(11789);
	return true;
}
bool ChatHandler::HandleSpellsImpPoisonCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(36839)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Impairing Poison.");
		return true;
	}
	plr->addSpell(36839);
	return true;
}

bool ChatHandler::HandleSpellsChaosFlamesCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(39055)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Flames of Chaos.");
		return true;
	}
	plr->addSpell(39055);
	return true;
}

bool ChatHandler::HandleSpellsChaosChargeCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(40497)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Chaos Charge.");
		return true;
	}
	plr->addSpell(40497);
	return true;
}

bool ChatHandler::HandleInvisDetectCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(9436)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Invisibilty Detection.");
		return true;
	}
	plr->addSpell(9436);
	return true;
}
bool ChatHandler::HandleSpellsInvisibleCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	if (plr->HasSpell(41253)) // check to see if char already knows
	{
		RedSystemMessage(m_session, "This character already knows Invisibilty.");
		return true;
	}
	plr->addSpell(41253);
	return true;
}


bool ChatHandler::HandleAllTalentsCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	if(!plr)
	{
		plr = m_session->GetPlayer();
		SystemMessage(m_session, "Auto-targeting self.");
	}
	if(!plr) return false;

	static uint32 spellarray[DRUID+1][9999] = {
		{ 0 },						// CLASS 0    ---
		{ 12664, 16466, 12659, 12697, 12962, 12666, 12963, 12296, 12867, 12714, 16494, 12785, 12292, 12704, 12815, 29889, 23695, 29725, 29859, 12294, 29838, 35451, 29623, 12838, 12856, 12879, 13002, 20496, 12323, 16492, 12861, 23588, 20503, 13048, 12330, 12328, 20505, 20501, 12974, 29592, 23881, 29776, 29763, 29801, 12818, 12677, 12753, 12727, 12764, 12975, 12945, 12800, 12789, 12811, 12807, 12765, 12803, 12809, 12958, 29600, 16542, 29595, 23922, 29792, 29146, 20243, 0 },		// CLASS 1  (Warrior)
		{ 20266, 20261, 20208, 20332, 20239, 31821, 20235, 25836, 20215, 20245, 31824, 20216, 20361, 31826, 25829, 31836, 20473, 31830, 31841, 31842, 20142, 20137, 20193, 20175, 20147, 20217, 20470, 20150, 20100, 31845, 20489, 20256, 31847, 20911, 20182, 31849, 20200, 41026, 20925, 31854, 31862, 31935, 20048, 20105, 25957, 20337, 20064, 26021, 20121, 20375, 44414, 25988, 20092, 31868, 20113, 20218, 31870, 20059, 31878, 35397, 20066, 31873, 31883, 35395, 0 },		// CLASS 2  (Pally)
		{ 19556, 19587, 35030, 19551, 19612, 19575, 19560, 19596, 19620, 19573, 19602, 20895, 19577, 19592, 34454, 19625, 34460, 19574, 34465, 34470, 34692, 19415, 19431, 19425, 19420, 34954, 19458, 19434, 34949, 19468, 19490, 35103, 19503, 24691, 34476, 19511, 34484, 19506, 35111, 34489, 34490, 24295, 19153, 19500, 19160, 19388, 19300, 19233, 19245, 19259, 19263, 19377, 24283, 19287, 34496, 19373, 19306, 34493, 24297, 34499, 19386, 34503, 34839, 23989, 0 },		// CLASS 3  (Huntter)
		{ 14164, 14148, 14142, 14161, 14159, 13866, 14179, 14169, 14137, 16720, 14117, 31209, 14177, 14176, 31245, 14195, 31227, 14983, 31385, 31242, 1329, 13792, 13863, 13791, 14167, 13856, 13845, 13872, 14251, 13875, 13867, 13807, 13852, 13803, 13877, 13964, 13969, 31126, 30920, 18429, 31123, 13750, 31131, 35553, 32601, 13973, 14075, 30893, 14094, 14065, 13980, 14278, 14081, 14071, 14066, 14173, 30895, 14185, 14083, 16511, 31223, 30906, 31213, 14183, 31230, 31220, 36554, 0 },						// CLASS 4  (Rogue)
		{ 52803, 14781, 63574, 33202, 34910, 63506, 19236, 64129, 63543, 63737, 47560, 47567, 47788, 14785, 14791, 14528, 14787, 14767, 14769, 14774, 33172, 14751, 14777, 14771, 14783, 14772, 18555, 14752, 33182, 33190, 18550, 45244, 10060, 33205, 34912, 33206, 15012, 17191, 15011, 27904, 18535, 15237, 27816, 15363, 27790, 15014, 15017, 15018, 20711, 15031, 33154, 15356, 34860, 724, 33146, 33162, 34861, 15338, 15326, 15320, 15317, 15330, 15448, 15316, 15407, 15311, 17323, 15334, 15487, 15286, 27840, 33215, 33371, 15310, 15473, 33225, 33195, 34914, 0 },						// CLASS 5  (Priest)
		{ 53138, 50152, 51473, 51052, 55133, 49664, 55226, 49393, 61158, 54637, 49628, 49543, 49395, 49504, 49222, 49483, 50043, 50115, 49632, 49028, 49480, 50034, 49796, 55667, 66817, 49599, 51161, 49657, 49562, 51109, 63560, 49791, 50191, 49203, 49016, 55062, 50887, 50371, 62908, 50385, 55610, 51456, 49489, 50392, 49638, 51130, 49039, 49611, 49005, 52143, 49538, 49534, 49565, 51465, 50138, 55623, 51267, 55237, 50121, 49572, 56835, 59057, 48982, 50147, 45909, 49497, 49491, 49530, 49206, 66192, 49789, 50130, 55108, 51271, 49194, 49589, 55233, 55136, 50029, 51746, 49568, 49655, 50150, 0 },						// CLASS 6    ---
		{ 16112, 16108, 16130, 28998, 16161, 16164, 16116, 16120, 16544, 29065, 29180, 29000, 16089, 30668, 30674, 16582, 16166, 30671, 30681, 30706, 17489, 16301, 16293, 16305, 16287, 16291, 16295, 43338, 16274, 16284, 16309, 29193, 16268, 29080, 30814, 29088, 30819, 30798, 17364, 30811, 30823, 16229, 16217, 16209, 16240, 16225, 16198, 16234, 16189, 29191, 16208, 16221, 29202, 16188, 30866, 16213, 16190, 30886, 30869, 30873, 974, 0 },						// CLASS 7  (Shaman)
		{ 12592, 12842, 16770, 6085, 29447, 12577, 12606, 12469, 28574, 12605, 12598, 18464, 31570, 12043, 12503, 31575, 15060, 31573, 31583, 12042, 35581, 31588, 31589, 12341, 12360, 12848, 12353, 12342, 18460, 12350, 11366, 12351, 12873, 13043, 29076, 31640, 11368, 11113, 31642, 12400, 34296, 11129, 31680, 31660, 31661, 28332, 16766, 29440, 15053, 12497, 12475, 12571, 12953, 12472, 12488, 16758, 12519, 12985, 31669, 11958, 12490, 31672, 28595, 11426, 31678, 31686, 31687, 0 },						// CLASS 8  (Mage)
		{ 18178, 17814, 18180, 18372, 18183, 17805, 18829, 17787, 18288, 18219, 18095, 32383, 32394, 18265, 18223, 18275, 30064, 18220, 30057, 32484, 30108, 18693, 18696, 18701, 18704, 18707, 18744, 18756, 18708, 18750, 30145, 18710, 18773, 18822, 18788, 18768, 30328, 23825, 30321, 19028, 35693, 30248, 30146, 17803, 17782, 17792, 18123, 18127, 18129, 18134, 17877, 18136, 17918, 17930, 18073, 17836, 17959, 30302, 17958, 34939, 17962, 30296, 30292, 30283, 0 },						// CLASS 9  (Warlock)
		{ 0 },						// CLASS 10   ---
		{ 16818, 16689, 17249, 16920, 35364, 16822, 16840, 5570, 16820, 16913, 16924, 33591, 16880, 16847, 16901, 33596, 33956, 24858, 33602, 33607, 33831, 16938, 16862, 16949, 16941, 16931, 24866, 16979, 16944, 16968, 16975, 37117, 16999, 16857, 33873, 24894, 33856, 33957, 17007, 34300, 33869, 33917, 17055, 17061, 17073, 17068, 16835, 17108, 17122, 16864, 24972, 17113, 17116, 24946, 17124, 33880, 17078, 34153, 18562, 33883, 33890, 33891, 0 },						// CLASS 11 (Druid)
	};

	uint32 c = plr->getClass();
	for(uint32 i = 0; spellarray[c][i] != 0; ++i)
	{
		plr->addSpell(spellarray[c][i]);
	}
	return true;
}
bool ChatHandler::HandleSelfRenameCommand(const char* args, WorldSession *m_session)
{
	Player *plr = m_session->GetPlayer();
	plr->rename_pending = true;
	plr->SaveToDB(false);
	BlueSystemMessage(m_session, "You will now change your name upon next login. Your current name has NOT been banned.");
	return true;
}


bool ChatHandler::HandleToggleInvisCommand(const char *args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();

	if(plr->GetMapId() == 230)	
	{
		RedSystemMessage(m_session, "This command may not be used in the UNDERWORLD Capital.");
		return true;
	}

	if(plr->IsPvPFlagged())
	{
		RedSystemMessage(m_session, "You are flagged PvP and considered a hostile target.  You must remove your flag if you wish to use this command.");
		return true;
	}

	if(plr->CombatStatus.IsInCombat())
	{
		RedSystemMessage(m_session, "You cannot use this spell while in combat.");
		return true;
	}

	if(plr->HasAura(41253))
	{
		plr->RemoveAura(41253);
		BlueSystemMessage(m_session, "Invisibility Removed.");
		return true;
	}
	else
	{
		// Cast Invisibilty (41253)
		SpellEntry * se = dbcSpell.LookupEntry(41253);
		if(se == 0) return false;
		SpellCastTargets targets(plr->GetGUID());
		Spell * sp = new Spell(m_session->GetPlayer(), se, true, 0);
		sp->prepare(&targets);
		
		BlueSystemMessage(m_session, "Invisibility Enabled.");
	}


	return true;
}

bool ChatHandler::HandleCreatureKillCommand(const char *args, WorldSession *m_session)
{
	Player * plr = m_session->GetPlayer();

	if(plr->InGroup()) //Make sure the Command is not used while in a group to help players
	{
		RedSystemMessage(m_session, "This command cannot be used while in a group, leave your group if you wish to use this command.");
		return true;
	}
//For now, leave off this stipulation since we have 15 second cooldown
/*	if(plr->GetMapId() == 230)	
	{
		RedSystemMessage(m_session, "This command may not be used in the UNDERWORLD Capital.");
		return true;
	} */

	Creature * crt = getSelectedCreature(m_session);
	if(!crt) 
		return true;

	if(crt->IsTotem())
	{
		if(m_session->totemattempt)
		{
			RedSystemMessage(m_session,"Stop trying to crash my server you douche. Now eat a mute for an hour and take time to think it over.");
			uint32 banned = (uint32)UNIXTIME+3600;
			sLogonCommHandler.Account_SetMute( m_session->GetAccountNameS(), banned );
			m_session->m_muted = banned;
			plr->SetUInt32Value( UNIT_FIELD_HEALTH, 0);
			plr->KillPlayer();
		}
		else
		{
			m_session->totemattempt=true;
			RedSystemMessage(m_session,"Do not use this command on totems, consider this your only warning.");
		}

		return true;
	}

	if(crt->getLevel() == 666)
	{
		plr->SetUInt32Value( UNIT_FIELD_HEALTH, 0);
		plr->KillPlayer();
		return true;
	}
	if(crt->getLevel() > 220 && crt->GetProto()->boss)
	{
		RedSystemMessage(m_session,"ERROR: This command may not be used on end-game bosses.");
		return true;
	}
	if(crt->GetUInt32Value(OBJECT_FIELD_ENTRY) == 111150 || crt->GetUInt32Value(OBJECT_FIELD_ENTRY) == 111151)
	{
		char msg[100];
		snprintf(msg, 100, "unleashed the fury of the UNDERWORLD on %s.", crt->GetCreatureInfo()->Name);
		WorldPacket * data = this->FillMessageData(CHAT_MSG_EMOTE, LANG_UNIVERSAL, msg, plr->GetGUID(), 0);
		plr->SendMessageToSet(data, true);
		delete data;
		
		char msg2[1024];
		snprintf(msg2, 1024, "[GENERAL OF HELL] yells: %s, your pathetic \"GM\" magic is beneath me.  You should not meddle with power you do not comprehend.  Here, try your own brand of death fool.", plr->GetName());
		char pAnnounce[1024];
		string input2;
		input2 = "|cffff0000";
		snprintf((char*)pAnnounce, 1024, "%s%s", input2.c_str(), msg2);
		sWorld.SendWorldText(pAnnounce);

		char msg3[100];
		snprintf(msg3, 100, "dies from %s own spell reflected by the GENERAL OF HELL.", (plr->getGender()?"her":"his"));
		WorldPacket * data3 = this->FillMessageData(CHAT_MSG_EMOTE, LANG_UNIVERSAL, msg3, plr->GetGUID(), 0);
		plr->SendMessageToSet(data3, true);
		delete data3;
		
		//Die you cheating fucktard.
		plr->SetUInt32Value(UNIT_FIELD_HEALTH, 0);
		plr->KillPlayer();		
		
		return true;
	}
	
	//uint16 plvl = plr->getLevel();
	//uint16 clvl = crt->getLevel();
	//uint16 leveldiff = plvl - clvl;
	//if (leveldiff<0)
	//	leveldiff=0;
	//if(leveldiff > 20 && clvl<plvl) //They have no reason to kill targets 20 levels below them
	//{
	//	RedSystemMessage(m_session, "This command may not be used on targets 20 levels below you.");
	//	return true;
	//} 

	if(crt->Tagged && crt->TaggerGuid != plr->GetGUID())
	{
		RedSystemMessage(m_session, "You cannot use this command on tagged targets.");
		return true;
	}

	if(getMSTime() > plr->m_nextKillC) //if/else used to make sure they are within limits of cooldown
	{
		// Cast insta-kill.
		SpellEntry * se = dbcSpell.LookupEntry(5);
		if(se == 0) return false;
		SpellCastTargets targets(crt->GetGUID());
		Spell * sp = new Spell(m_session->GetPlayer(), se, true, 0);
		sp->prepare(&targets);

		//Letting everyone in the area know we used insta kill on a mob
		char msg[100];
		snprintf(msg, 100, "unleashed the fury of the UNDERWORLD on %s.", crt->GetCreatureInfo()->Name);

		WorldPacket * data = this->FillMessageData(CHAT_MSG_EMOTE, LANG_UNIVERSAL, msg, plr->GetGUID(), 0);
		plr->SendMessageToSet(data, true);
		delete data;

		//need to reset their cooldown timer
		plr->m_nextKillC = getMSTime() + 15000;
	}
	else
	{
		uint32 timeleft = plr->m_nextKillC - getMSTime();
		RedSystemMessage(m_session, "You may use this command again in %u ms.", timeleft);
		return true;
	}

	return true;
}


bool ChatHandler::HandleEventRingCommand(const char* args, WorldSession *m_session)
{
	
	if(!*args)
	return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}

	uint32 itemid = 70010;
	ItemPrototype* it = ItemPrototypeStorage.LookupEntry(itemid);

	Player *plr = objmgr.GetPlayer((const char*)pname, false);
	if (!it) //Better safe than sorry
	{
		RedSystemMessage(m_session, "ERROR: The item (%u) does not exist. Please alert an admin or dev.", itemid);
		return true;
	}

	if (!plr)
	{
		RedSystemMessage(m_session, "Player is not online at the moment.");
		return true;
	}

	int8 error = plr->GetItemInterface()->CanReceiveItem(it, 1);
	if(error)
	{
		RedSystemMessage(m_session, "ERROR: The player entered already has %s.", EVENT_RING);
		return true;
	}

	if(plr == m_session->GetPlayer())
	{
		RedSystemMessage(m_session, "This command may not be used on yourself.");
		return true;
	}

	char *reason = strtok(NULL, "\n");
	std::string kickreason = "Yo yo yo, I forgot a reason.";

	if(!reason)
	{
		RedSystemMessage(m_session, "You must enter a reason to use this command.");
		return true;
	}

	kickreason = reason;

	//log it after checks
	char command[100]="event ring";
	char notes[1024];
	snprintf(notes, 1024, "REASON: %s", kickreason.c_str());
	GMLog(m_session, plr->GetSession(), command, notes);

	char msg[250];
	
	string tag = string(GetRankTitle(m_session));
	

	snprintf(msg, 250, "%sEVENT NOTICE: %s was awarded an %s%s by <%s>%s. Reason: %s", MSG_COLOR_GREEN, plr->GetName(), EVENT_RING, MSG_COLOR_GREEN, tag.c_str(), m_session->GetPlayer()->GetName(), kickreason.c_str());
	sWorld.SendWorldText(msg, NULL);

	/*Award the item below
	*/

	Item *item;
	item = objmgr.CreateItem( itemid, plr);
	item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, ((1 > it->MaxCount) ? it->MaxCount : 1));
	if(it->Bonding==ITEM_BIND_ON_PICKUP)
		item->SoulBind();
	
	if(!plr->GetItemInterface()->AddItemToFreeSlot(item))
	{
		m_session->SendNotification("No free slots were found in your inventory!");
		delete item;
		return true;
	}
	char messagetext[128];
	snprintf(messagetext, 128, "Adding item %d (%s) to %s's inventory.",(unsigned int)it->ItemId,it->Name1, plr->GetName());
	SystemMessage(m_session, messagetext);
	snprintf(messagetext, 128, "%s added item %d (%s) to your inventory.", m_session->GetPlayer()->GetName(), (unsigned int)itemid, it->Name1);
	SystemMessageToPlr(plr,  messagetext);
	SlotResult *lr = plr->GetItemInterface()->LastSearchResult();
	plr->SendItemPushResult(false,true,false,true,lr->ContainerSlot,lr->Slot,1,item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

	return true; 
} 
bool ChatHandler::HandleEventTrinkCommand(const char* args, WorldSession *m_session)
{
	
	if(!*args)
	return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}

	uint32 itemid = 70011;
	ItemPrototype* it = ItemPrototypeStorage.LookupEntry(itemid);

	Player *plr = objmgr.GetPlayer((const char*)pname, false);
	if (!it) //Better safe than sorry
	{
		RedSystemMessage(m_session, "ERROR: The item (%u) does not exist. Please alert an admin or dev.", itemid);
		return true;
	}

	if (!plr)
	{
		RedSystemMessage(m_session, "Player is not online at the moment.");
		return true;
	}

	int8 error = plr->GetItemInterface()->CanReceiveItem(it, 1);
	if(error)
	{
		RedSystemMessage(m_session, "ERROR: The player entered already has %s.", EVENT_TRINKET);
		return true;
	}

	if(plr == m_session->GetPlayer())
	{
		RedSystemMessage(m_session, "This command may not be used on yourself.");
		return true;
	}

	char *reason = strtok(NULL, "\n");
	std::string kickreason = "Yo yo yo, I forgot a reason.";

	if(!reason)
	{
		RedSystemMessage(m_session, "You must enter a reason to use this command.");
		return true;
	}

	kickreason = reason;

	//log it after checks
	char command[100]="event trink";
	char notes[1024];
	snprintf(notes, 1024, "REASON: %s", kickreason.c_str());
	GMLog(m_session, plr->GetSession(), command, notes);

	char msg[250];
	
	string tag = string(GetRankTitle(m_session));

	snprintf(msg, 250, "%sEVENT NOTICE: %s was awarded an %s%s by <%s>%s. Reason: %s", MSG_COLOR_GREEN, plr->GetName(), EVENT_TRINKET, MSG_COLOR_GREEN, tag.c_str(), m_session->GetPlayer()->GetName(), kickreason.c_str());
	sWorld.SendWorldText(msg, NULL);

	/*Award the item below
	*/

	Item *item;
	item = objmgr.CreateItem( itemid, plr);
	item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, ((1 > it->MaxCount) ? it->MaxCount : 1));
	if(it->Bonding==ITEM_BIND_ON_PICKUP)
		item->SoulBind();
	
	if(!plr->GetItemInterface()->AddItemToFreeSlot(item))
	{
		m_session->SendNotification("No free slots were found in your inventory!");
		delete item;
		return true;
	}
	char messagetext[128];
	snprintf(messagetext, 128, "Adding item %d (%s) to %s's inventory.",(unsigned int)it->ItemId,it->Name1, plr->GetName());
	SystemMessage(m_session, messagetext);
	snprintf(messagetext, 128, "%s added item %d (%s) to your inventory.", m_session->GetPlayer()->GetName(), (unsigned int)itemid, it->Name1);
	SystemMessageToPlr(plr,  messagetext);
	SlotResult *lr = plr->GetItemInterface()->LastSearchResult();
	plr->SendItemPushResult(false,true,false,true,lr->ContainerSlot,lr->Slot,1,item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

	return true; 
} 
bool ChatHandler::HandleEventNeckCommand(const char* args, WorldSession *m_session)
{
	
	if(!*args)
	return false;
	char *pname = strtok((char*)args, " ");
	if(!pname)
	{
		RedSystemMessage(m_session, "No name specified.");
		return true;
	}

	uint32 itemid = 70012;
	ItemPrototype* it = ItemPrototypeStorage.LookupEntry(itemid);

	Player *plr = objmgr.GetPlayer((const char*)pname, false);
	if (!it) //Better safe than sorry
	{
		RedSystemMessage(m_session, "ERROR: The item (%u) does not exist. Please alert an admin or dev.", itemid);
		return true;
	}

	if (!plr)
	{
		RedSystemMessage(m_session, "Player is not online at the moment.");
		return true;
	}

	int8 error = plr->GetItemInterface()->CanReceiveItem(it, 1);
	if(error)
	{
		RedSystemMessage(m_session, "ERROR: The player entered already has %s.", EVENT_NECKLACE);
		return true;
	}

	if(plr == m_session->GetPlayer())
	{
		RedSystemMessage(m_session, "This command may not be used on yourself.");
		return true;
	}

	char *reason = strtok(NULL, "\n");
	std::string kickreason = "Yo yo yo, I forgot a reason.";

	if(!reason)
	{
		RedSystemMessage(m_session, "You must enter a reason to use this command.");
		return true;
	}

	kickreason = reason;

	//log it after checks
	char command[100]="event necklace";
	char notes[1024];
	snprintf(notes, 1024, "REASON: %s", kickreason.c_str());
	GMLog(m_session, plr->GetSession(), command, notes);

	char msg[250];
	
	string tag = string(GetRankTitle(m_session));

	snprintf(msg, 250, "%sEVENT NOTICE: %s was awarded an %s%s by <%s>%s. Reason: %s", MSG_COLOR_GREEN, plr->GetName(), EVENT_NECKLACE, MSG_COLOR_GREEN, tag.c_str(), m_session->GetPlayer()->GetName(), kickreason.c_str());
	sWorld.SendWorldText(msg, NULL);

	/*Award the item below
	*/

	Item *item;
	item = objmgr.CreateItem( itemid, plr);
	item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, ((1 > it->MaxCount) ? it->MaxCount : 1));
	if(it->Bonding==ITEM_BIND_ON_PICKUP)
		item->SoulBind();
	
	if(!plr->GetItemInterface()->AddItemToFreeSlot(item))
	{
		m_session->SendNotification("No free slots were found in your inventory!");
		delete item;
		return true;
	}
	char messagetext[128];
	snprintf(messagetext, 128, "Adding item %d (%s) to %s's inventory.",(unsigned int)it->ItemId,it->Name1, plr->GetName());
	SystemMessage(m_session, messagetext);
	snprintf(messagetext, 128, "%s added item %d (%s) to your inventory.", m_session->GetPlayer()->GetName(), (unsigned int)itemid, it->Name1);
	SystemMessageToPlr(plr,  messagetext);
	SlotResult *lr = plr->GetItemInterface()->LastSearchResult();
	plr->SendItemPushResult(false,true,false,true,lr->ContainerSlot,lr->Slot,1,item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());

	return true; 
} 
bool ChatHandler::HandleEventLevelUpCommand(const char* args, WorldSession *m_session)
{
	if(!*args)
		return false;

	// this is rather complicated due to ban reasons being able to have spaces. so we'll have to some c string magic
	// rather than just sscanf'ing it.
	char * pCharacter = (char*)args;
	char * pLevels = strchr(pCharacter, ' ');
	//PlayerInfo * pInfo = NULL;
	if(pLevels == NULL)
		return false;

	char * pReason = strchr(pLevels+1, ' ');
	if(pReason == NULL)
		return false;
	

	// zero them out to create sepearate strings.
	*pLevels = 0;
	++pLevels;
	*pReason = 0;
	++pReason;


	Player * plr = objmgr.GetPlayer(pCharacter, false);
	if(!plr)
	{
		RedSystemMessage(m_session, "No player found.");
		return true;
	}

	if(plr == m_session->GetPlayer())
	{
		RedSystemMessage(m_session, "This command may not be used on yourself.");
		return true;
	}

	int levels = 0;

	levels = atoi(pLevels);

	if(levels <= 0)
		return false;

	if(plr->getLevel() >= 255)return false;

	char msg[250];
	std::string reason = pReason;

	string tag = string(GetRankTitle(m_session));

	snprintf(msg, 250, "%sEVENT NOTICE: %s was given %u levels by <%s>%s. Reason: %s", MSG_COLOR_GREEN, plr->GetName(), levels, tag.c_str(), m_session->GetPlayer()->GetName(), reason.c_str());
	sWorld.SendWorldText(msg, NULL);


	char command[100]="event levelup";
	char notes[1024];
	snprintf(notes, 1024, "Levels Given: %u", levels);
	GMLog(m_session, plr->GetSession(), command, notes);	
	
	levels += plr->getLevel();
	if(levels>255)
		levels=255;

	LevelInfo * inf = objmgr.GetLevelInfo(plr->getRace(),plr->getClass(),levels);
	if(!inf)
		return false;

	plr->ApplyLevelInfo(inf,levels);

	//sSocialMgr.SendUpdateToFriends( plr );

	return true;
}


bool ChatHandler::HandleGMRingCommand(const char *args, WorldSession *m_session)
{	
	Player * Plr = getSelectedChar(m_session, false);
	Player * crt = m_session->GetPlayer();

	if(!Plr)
	{
		RedSystemMessage(m_session, "You must have a character targeted.");
		return true;
	}
	
	if(!(Plr->GetSession()->m_gmData->rank > RANK_NO_RANK))
	{
		RedSystemMessage(m_session, "This command can only be used on GMs.");
		return true;
	}

	/*if (Plr->GetSession()->m_gmData->temp)
	{
		char msg3[250];
		snprintf(msg3, 250, "Sorry, %s Temp GMs are not allowed to have GM rings.", Plr->GetName());
		
		WorldPacket * data = this->FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, msg3, crt->GetGUID(), 0);
		crt->SendMessageToSet(data, true);
		delete data; 
	}*/

	if(Plr->RingCheck())
	{
		RedSystemMessage(m_session, "%s has a ring in either their inventory or bank and must delete it to receive a ring. GMs cannot have two rings of any level.", Plr->GetName());
		RedSystemMessageToPlr(Plr, "Your ring cannot be awarded, you already possess a ring and must delete it prior to receiving an upgrade. Please note you cannot have two GM rings of any level.");
		return true;
	}
	
	uint32 id = 0;
	string rank = "";
	switch (Plr->GetSession()->m_gmData->rank)
	{
	case RANK_BRONZE: //bronze
		id = 70003;
		rank = "BRONZE";
		break;
	case RANK_SILVER: //silver
		id = 70004;
		rank = "SILVER";
		break;
	case RANK_GOLD: //Gold
		id = 70005;
		rank = "GOLD";
		break;
	case RANK_PLAT: //Plat
		id = 70006;
		rank = "PLAT";
		break;
	default: //noob
		id = 70003;
		rank = "BRONZE";
	}
	
	ItemPrototype* it = ItemPrototypeStorage.LookupEntry(id);
	if(it)
	{
		if(Plr->GetItemInterface()->CanReceiveItem(it, 1))
		{	
			char msg2[250];
			snprintf(msg2, 250, "%s I cannot give you your ring, you do not have enough space!", Plr->GetName());

			WorldPacket * data = FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, msg2, crt->GetGUID(), 0);
			crt->SendMessageToSet(data, true);
			delete data; 
			return true;
		}
		Item * item = objmgr.CreateItem(id, Plr);
		if(it->Bonding==ITEM_BIND_ON_PICKUP)
		{
			if (it->Flags & ITEM_FLAG_ACCOUNTBOUND)
				item->AccountBind();
			else
				item->SoulBind();							
		}
		if(!Plr->GetItemInterface()->AddItemToFreeSlot(item))
		{
			char msg[250];
			snprintf(msg, 250, "%s you cannot receive your ring, you have no free space to add it.", Plr->GetName());

			WorldPacket * data = this->FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, msg, crt->GetGUID(), 0);
			crt->SendMessageToSet(data, true);
			delete item;
			delete data; 
			return true;											
		}			
		SlotResult *lr = Plr->GetItemInterface()->LastSearchResult();
		Plr->SendItemPushResult(false,true,false,true,lr->ContainerSlot,lr->Slot,1,item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());		
		char msg3[250];
		snprintf(msg3, 250, "Congratulations %s on your |Hitem:%u:0:0:0:0:0:0:0|h[%s]|h|r!", Plr->GetName(), it->ItemId, it->Name1  );
		
		WorldPacket * data = this->FillMessageData(CHAT_MSG_SAY, LANG_UNIVERSAL, msg3, crt->GetGUID(), 0);
		crt->SendMessageToSet(data, true);
		delete data; 


		GMLog(m_session, Plr->GetSession(), "GMring", rank.c_str());


		return true;
	}

	return false;
}


bool ChatHandler::HandleLevelSummonCommand(const char* args, WorldSession* m_session)
{
	if(!args) return false;

	uint32 min, max=255;
	
	if(sscanf(args, "%u %u", &min, &max) < 1)
		return false;

	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	Player * summoner = m_session->GetPlayer();
	Player * plr;
	uint32 c=0;
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		plr = itr->second;
		if(plr->GetSession() && plr->IsInWorld() && plr->getLevel()>=min && plr->getLevel()<=max)
		{
			//BlueSystemMessageToPlr(plr, "");
			plr->SummonRequest(summoner->GetLowGUID(), summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
			++c;

			if(!RANK_CHECK(RANK_ADMIN))
			{
				if(c>=100)
				{
					RedSystemMessage(m_session,"Event Summon capped at 100 players. Please do not summon more then 100 players to an area at any given moment.");
					break;
				}
			}
		}
	}
	char command[100]="event levelsummon";
	char notes[1024];
	snprintf(notes, 1024, "%u players MIN: %u - MAX: %u.", c, min, max);
	GMLog(m_session, command, notes);


	objmgr._playerslock.ReleaseReadLock();
	GreenSystemMessage(m_session, "Mass Summon complete of %u players. (Levels %u to %u)", c, min, max);
	return true;
}
bool ChatHandler::HandleGMMassSummonCommand(const char* args, WorldSession* m_session)
{
	if(m_session->m_gmData->rank < RANK_ADMIN)
	{
		if(m_session->GetPlayer()->MassSumEligible == false)
		{
			RedSystemMessage(m_session, "You have already issued a class summon.  Instruct any missing GMs to use .recall port class");
			return true;
		}
	}

	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	Player * summoner = m_session->GetPlayer();
	Player * plr;
	uint32 c=0;
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		plr = itr->second;
		if(plr->GetSession() && plr->IsInWorld() && plr->GetSession()->m_gmData->rank  > RANK_NO_RANK)
		{
			BlueSystemMessageToPlr(plr, "%s is sending you a summon request and will be conducting a GM Class.  Accept the summon if you wish to attend.", summoner->GetName());
			plr->SummonRequest(summoner->GetLowGUID(), summoner->GetZoneId(), summoner->GetMapId(), summoner->GetInstanceID(), summoner->GetPosition());
			++c;
		}
	}
	char msg[100];
	snprintf(msg, 100, "%u GMs", c);
	GMLog(m_session, "classsummon", msg);
	objmgr._playerslock.ReleaseReadLock();

	m_session->GetPlayer()->MassSumEligible = false;

	return true;
}

struct GMInfo
{
	GMInfo() :
		login(""),
		rank(0),
		addon1(false),
		addon2(false),
		addon3(false),
		addon4(false),
		mute(false)
		{}
	const char * login;
	uint32 rank;
	bool addon1;
	bool addon2;
	bool addon3;
	bool addon4;
	bool mute;
};

bool ChatHandler::HandleGMStringUpdate(const char* args, WorldSession* m_session)
{
	/*
	vector<GMInfo*> gm;
	GMInfo * tmp;
	Field * f;
	string str = "";
	QueryResult * res = CharacterDatabase.Query("SELECT * FROM account_forced_permissions");
	do
	{
		f = res->Fetch();
		tmp = new GMInfo;
		tmp->login = f[0].GetString();

		str = string(f[1].GetString());
		arcemu_TOLOWER(str);

		
		char r = str[2];
		string a = str.substr(str.find("-", at+1));
		if (!r)
		{
			tmp->rank = 5;
			gm.push_back(tmp);
			continue;
		}

		switch (r)
		{
		case 'p':
			tmp->rank = 4;
			break;
		case 'g':
			tmp->rank = 3;
			break;
		case 's':
			tmp->rank = 2;
			break;
		case 'b':
			tmp->rank = 1;
			break;
		}

		for (uint32 ii = 0; ii < a.length(); ii++)
		{
			switch (a.at(ii))
			{
			case 'f':
				tmp->addon1 = true;
				break;
			case 'b':
				tmp->addon2 = true;
				break;
			case 'c':
				tmp->addon3 = true;
				break;
			case 'e':
				tmp->addon4 = true;
				break;
			case 'm':
				tmp->mute = true;
				break;

			}
		}

		
		gm.push_back(tmp);
			
	} while(res->NextRow());
	
	QueryBuffer * buf = new QueryBuffer;
	CharacterDatabase.Execute("TRUNCATE account_forced_permissions");

	for (uint32 ii = 0; ii < gm.size(); ii++)
	{
		char tmp[256];
		string rank = "";
		string addons = "";
		switch (gm[ii]->rank)
		{
		case 4:
			rank = "platinum";
			break;
		case 3:
			rank = "gold";
			break;
		case 2:
			rank = "silver";
			break;
		case 1:
			rank = "bronze";
			break;
		}

		if (gm[ii]->addon1)
			addons += "1";
		if (gm[ii]->addon2)
			addons += "2";
		if (gm[ii]->addon3)
			addons += "3";
		if (gm[ii]->addon4)
			addons += "4";
		if (gm[ii]->mute)
			addons += "m";

		if (addons.length() < 1)
			addons = "x";

		snprintf(tmp, 256, "p-%s-%s", rank.c_str(), addons.c_str());

		if (gm[ii]->rank == 6)
			snprintf(tmp, 256, "az");
		else if (gm[ii]->rank == 5)
			snprintf(tmp, 256, "a");

		SystemMessage(m_session, "INSERT INTO account_forced_permissions VALUES ('%s', '%s',0 ,0)", gm[ii]->login, tmp);

		buf->AddQuery("INSERT INTO account_forced_permissions VALUES ('%s', '%s',0 ,0)", gm[ii]->login, tmp);
	}

	CharacterDatabase.AddQueryBuffer(buf);
	
	delete res;
	delete f;
	delete tmp;

	GreenSystemMessage(m_session, "Done.");
	*/
	
	GreenSystemMessage(m_session, "Disabled");
	//m_session->GetPlayer()->GetItemInterface()->DeleteItemById(192);

	return true;
}

bool ChatHandler::HandleChangeChatColor(const char* args, WorldSession* m_session)
{
	if (args == NULL)
		return false;

	string tmp = "|cff";
	tmp += string(args);
	m_session->m_gmData->chatColor = tmp;
	BlueSystemMessage(m_session, "Your chat color string is now '%s'. Be careful this may crash the realm if not properly formatted.", args);
	return true;
}

bool ChatHandler::HandleBonusFlag(const char* args, WorldSession* m_session)
{
	m_session->m_uwflags->bonus = (!m_session->m_uwflags->bonus);
	GreenSystemMessage(m_session, "The bonus flag is now %s.", (m_session->m_uwflags->bonus ? "on" : "off"));

	return true;
}

bool ChatHandler::HandleUWPvPFlag(const char* args, WorldSession* m_session)
{
	m_session->m_uwflags->PvP = (!m_session->m_uwflags->PvP);
	GreenSystemMessage(m_session, "The PvP flag is now %s.", (m_session->m_uwflags->PvP ? "on" : "off"));

	return true;
}

bool ChatHandler::HandleSendItemPacket(const char* args, WorldSession* m_session)
{
	if (!*args)
		return false;

	uint32 i;
	uint32 itemid= atoi(args);

	if (!itemid)
		return false;


	ItemPrototype *itemProto = ItemPrototypeStorage.LookupEntry(itemid);
	if(!itemProto)
	{
		sLog.outError( "WORLD: Unknown item id 0x%.8X", itemid );
		return false;
	} 

	size_t namelens = strlen(itemProto->Name1) + strlen(itemProto->Description) + 602;

	WorldPacket data(SMSG_ITEM_QUERY_SINGLE_RESPONSE, namelens );
	data << itemProto->ItemId;
	data << itemProto->Class;
	data << itemProto->SubClass;
	data << itemProto->unknown_bc;
	data << itemProto->Name1;

	/*data << itemProto->Name2;
	data << itemProto->Name3;
	data << itemProto->Name4;*/
	data << uint8(0) << uint8(0) << uint8(0);		// name 2,3,4
	data << itemProto->DisplayInfoID;
	data << itemProto->Quality;
	data << itemProto->Flags;
	data << itemProto->BuyPrice;
	data << itemProto->Faction;
	data << itemProto->SellPrice;
	data << itemProto->InventoryType;
	data << itemProto->AllowableClass;
	data << itemProto->AllowableRace;
	data << itemProto->ItemLevel;
	data << itemProto->RequiredLevel;
	data << itemProto->RequiredSkill;
	data << itemProto->RequiredSkillRank;
	data << itemProto->RequiredSkillSubRank;
	data << itemProto->RequiredPlayerRank1;
	data << itemProto->RequiredPlayerRank2;
	data << itemProto->RequiredFaction;
	data << itemProto->RequiredFactionStanding;
	data << itemProto->Unique;
	data << itemProto->MaxCount;
	data << itemProto->ContainerSlots;
	data << itemProto->itemstatscount;
	for( i = 0; i < itemProto->itemstatscount; i++ )
	{
		data << itemProto->Stats[i].Type;
		data << itemProto->Stats[i].Value;
	}

	data << itemProto->ScalingStatsEntry;
	data << itemProto->ScalingStatsFlag;
	for(i = 0; i < 2; i++) //VLack: seen this in Aspire code, originally this went up to 5, now only to 2
	{
		data << itemProto->Damage[i].Min;
		data << itemProto->Damage[i].Max;
		data << itemProto->Damage[i].Type;
	}
	data << itemProto->Armor;
	data << itemProto->HolyRes;
	data << itemProto->FireRes;
	data << itemProto->NatureRes;
	data << itemProto->FrostRes;
	data << itemProto->ShadowRes;
	data << itemProto->ArcaneRes;
	data << itemProto->Delay;
	data << itemProto->AmmoType;
	data << itemProto->Range;
	for(i = 0; i < 5; i++) {
		data << itemProto->Spells[i].Id;
		data << itemProto->Spells[i].Trigger;
		data << itemProto->Spells[i].Charges;
		data << itemProto->Spells[i].Cooldown;
		data << itemProto->Spells[i].Category;
		data << itemProto->Spells[i].CategoryCooldown;
	}
	data << itemProto->Bonding;
	
	data << itemProto->Description;

	data << itemProto->PageId;
	data << itemProto->PageLanguage;
	data << itemProto->PageMaterial;
	data << itemProto->QuestId;
	data << itemProto->LockId;
	data << itemProto->LockMaterial;
	data << itemProto->SheathID;
	data << itemProto->RandomPropId;
	data << itemProto->RandomSuffixId;
	data << itemProto->Block;
	data << itemProto->ItemSet;
	data << itemProto->MaxDurability;
	data << itemProto->ZoneNameID;
	data << itemProto->MapID;
	data << itemProto->BagFamily;
	data << itemProto->TotemCategory;
	data << itemProto->Sockets[0].SocketColor ;
	data << itemProto->Sockets[0].Unk;
	data << itemProto->Sockets[1].SocketColor ;
	data << itemProto->Sockets[1].Unk ;
	data << itemProto->Sockets[2].SocketColor ;
	data << itemProto->Sockets[2].Unk ;
	data << itemProto->SocketBonus;
	data << itemProto->GemProperties;
	data << itemProto->DisenchantReqSkill;
	data << itemProto->ArmorDamageModifier;
	data << itemProto->ExistingDuration;								// 2.4.2 Item duration in seconds
	data << itemProto->ItemLimitCategory;
	data << itemProto->HolidayId; //MesoX: HolidayId - points to HolidayNames.dbc
	m_session->SendPacket( &data );

	GreenSystemMessage(m_session, "DOne.");
	return true;

}


bool ChatHandler::HandleArenaDispelAllCommand(const char * args, WorldSession * m_session)
{
	if (!sWorld.arenaEventInProgress)
	{
		RedSystemMessage(m_session, "ERROR: Arena is not in event mode.");
		return true;
	}
	Player * plr;

	PlayerStorageMap::const_iterator itr;
	objmgr._playerslock.AcquireReadLock();
	for (itr = objmgr._players.begin(); itr != objmgr._players.end(); itr++)
	{
		plr = itr->second;
		if(plr->GetSession() && plr->IsInWorld() && plr->GetMapId() == 530 && plr->GetSession()->m_gmData->rank <= RANK_COADMIN)
		{
			plr->RemoveAurasUW();
		}
	}
	
	objmgr._playerslock.ReleaseReadLock();

	BlueSystemMessage(m_session, "Arena Dispel Done");
	return true;
}

bool ChatHandler::HandleCheckDonorPointsCommand(const char* args, WorldSession *m_session)
{
	uint32 points = 0;
	if(strlen(args) < 1)
		return false;

	WorldSession * sess = sWorld.FindSessionByName(args);
	if(sess)
	{
		points = sess->m_points;
	}
	else //fucker is offline, so do everything manually
	{
		QueryResult * res = WorldDatabase.Query("SELECT reward_points FROM `%s`.accounts WHERE login='%s' LIMIT 1", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), (WorldDatabase.EscapeString(string(args)).c_str()));
		if(res)
		{
			points = res->Fetch()[0].GetUInt32();
		}
		else
		{
			RedSystemMessage(m_session, "ERROR: Account [%s] does not exist.", args);
			return true;
		}
		delete res;
	}
	
	SystemMessage(m_session,"ACCOUNT: |cffffffff%s|r POINTS: |cffffffff%u", args, points);
	return true;
}
bool ChatHandler::HandleTalentTest(const char* args, WorldSession* m_session)
{
	Player * plr = m_session->GetPlayer();
	plr->LearnTalent(342, 4, true);
	plr->LearnTalent(1898, 4, true);
	plr->LearnTalent(352, 2, true);
	plr->LearnTalent(346, 2, true);
	plr->LearnTalent(344, 1, true);
	plr->LearnTalent(321, 1, true);
	plr->LearnTalent(343, 2, true);
	plr->LearnTalent(348, 0, true);
	plr->LearnTalent(347, 2, true);
	plr->LearnTalent(1769, 2, true);
	plr->LearnTalent(341, 2, true);
	plr->LearnTalent(350, 1, true);
	plr->LearnTalent(351, 0, true);
	plr->LearnTalent(1201, 4, true);
	plr->LearnTalent(2268, 1, true);
	plr->LearnTalent(1771, 1, true);
	plr->LearnTalent(1772, 2, true);
	plr->LearnTalent(1858, 2, true);
	plr->LearnTalent(322, 0, true);
	plr->LearnTalent(1773, 2, true);
	plr->LearnTalent(1894, 1, true);
	plr->LearnTalent(1896, 2, true);
	plr->LearnTalent(2235, 1, true);
	plr->LearnTalent(1895, 2, true);
	plr->LearnTalent(1774, 0, true);
	plr->LearnTalent(1901, 1, true);
	plr->LearnTalent(1202, 4, true);
	plr->LearnTalent(1897, 0, true);
	return true;
}

bool ChatHandler::HandleGMGear(const char* args, WorldSession* m_session)
{
	/*if (!RANK_CHECK(RANK_COADMIN))
	{
		RedSystemMessage(m_session, "ERROR: This command is disabled until the gear is refurbished.");
		return true;
	}*/

	m_session->GMGearCheck();
	Player * plr = m_session->GetPlayer();
	uint32 setid = 0;
	switch (m_session->m_gmData->rank)
	{
	case RANK_BRONZE:
		setid = 1800;
		break;
	case RANK_SILVER:
		setid = 1801;
		break;
	case RANK_GOLD:
		setid = 1802;
		break;
	case RANK_PLAT:
		setid = 1803;
		break;
	case RANK_COADMIN:
		setid = 1804;
		break;
	}

	if (setid == 0)
	{
		RedSystemMessage(m_session, "You are an invalid rank to be using this command.");
		return true;
	}

	std::list<ItemPrototype*>* l = objmgr.GetListForItemSet(setid);
	if(!l)
	{
		RedSystemMessage(m_session, "This GM rank set does not exist.");
		return true;
	}

	for(std::list<ItemPrototype*>::iterator itr = l->begin(); itr != l->end(); ++itr)
	{
		plr->GetItemInterface()->AddItemById((*itr)->ItemId, 1, 0);
	}
	GreenSystemMessage(m_session, "Added GM set. Enjoy!");

	return true;
}

bool ChatHandler::HandleRemoveDonorPointsCommand(const char* args, WorldSession *m_session)
{
	int32 points=0;
	char pLogin[255]; 
	if(strlen(args) < 1)
		return false;
	
	if(sscanf(args, "%s %d", &pLogin, &points) != 2)
		return false;

	if(!points)
	{
		RedSystemMessage(m_session,"ERROR: You must enter in a point value.");
		return true;
	}
	if (points > 0)
		points = -points;

	char command[100]="account removepoints";
	char notes[1024];
	snprintf(notes, 1024, "account %s -- POINTS: %u", pLogin, points);
	GMLog(m_session, command, notes);

	int32 addedpoints = points;
	uint32 prepoints = 0;

	WorldSession * sess = sWorld.FindSessionByName(pLogin);
	if(sess)
	{
		prepoints = sess->m_points;
		sess->AssignPoints(points);
		points = (points + prepoints) > 0 ? prepoints + points : 0;;
		
	}
	else //fucker is offline, so do everything manually
	{
		QueryResult * res = WorldDatabase.Query("SELECT reward_points FROM `%s`.accounts WHERE login='%s' LIMIT 1", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), pLogin);
		if(res)
		{
			prepoints = res->Fetch()[0].GetUInt32();
			points = (points + prepoints) > 0 ? prepoints + points : 0;
		}
		else
		{
			RedSystemMessage(m_session, "ERROR: Account [%s] does not exist.", pLogin);
			return true;
		}

		WorldDatabase.Execute("UPDATE `%s`.accounts SET reward_points=%u WHERE login='%s'", (WorldDatabase.EscapeString(sWorld.logonDB).c_str()), points, (WorldDatabase.EscapeString(string(pLogin)).c_str()));
			
		delete res;
		//now log into credit_log -- fuck pain
		WorldDatabase.Execute("INSERT INTO `_point_credit_log` (`acct`,`login`,`guid`, `name`,`points_added`,`timestamp`) VALUES (66666, '%s', 66666, '(OFFLINE)ADDPOINT COMMAND', %u, NOW())", (WorldDatabase.EscapeString(string(pLogin)).c_str()), addedpoints);

	}
	
	SystemMessage(m_session,"ACCOUNT: |cffffffff%s|r\nSTARTING POINTS: |cffffffff%u|r\nEND POINTS: |cffffffff%u|r\nADDED POINTS: |cffffffff%u", pLogin, prepoints, points, addedpoints);
	return true;
}

bool ChatHandler::HandleGetLinkId(const char* args, WorldSession *m_session)
{
	if (!*args)
		return false;

	bool spell = false;
	uint32 id = 0;
	GetItemIDFromLink(args, &id);
	if (id == 0) //malformed. maybe a spell?
	{
		spell = true;
		id = GetSpellIDFromLink(args);
	}

	if (id == 0) //neither spell nor item link
	{
		RedSystemMessage(m_session, "ERROR: The passed text was not a valid spell or item link.");
		return true;
	}

	if (spell)
		BlueSystemMessage(m_session, "SpellId: [%u]", id);
	else
		GreenSystemMessage(m_session, "ItemId: [%u]", id);

	return true;
}

bool ChatHandler::HandlePVPToggleCommand(const char * args, WorldSession *m_session)
{
	GET_PLAYER(RANK_ADMIN);

	if (!RANK_CHECK(RANK_ADMIN))
	{
		if (plr->m_deflagCD > getMSTime())
		{
			uint32 timeleft = uint32((plr->m_deflagCD - getMSTime()) / 1000);
			RedSystemMessage(m_session, "ERROR: %u seconds until you can use this command again.", timeleft);
			return true;
		}
		if (plr->IsFFAPvPFlagged())
		{
			RedSystemMessage(m_session, "ERROR: You cannot use this comand while FFA flagged.");
			return true;
		}
	}

	if (plr->IsPvPFlagged())
	{
		plr->RemovePvPFlag();
		plr->RemoveFFAPvPFlag();
		plr->m_deflagCD = getMSTime() + 300000;
	}
	else if (RANK_CHECK(RANK_ADMIN))
		plr->SetPvPFlag();
	else
	{
		RedSystemMessage(m_session, "ERROR: PvP flag is already off.");
		return true;
	}

	BlueSystemMessage(m_session, "PvP flag is %s.", (plr->IsPvPFlagged() ? "on" : "off"));
	return true;
}

bool ChatHandler::HandleFFAToggleCommand(const char * args, WorldSession *m_session)
{
	GET_PLAYER(RANK_ADMIN);

	if (plr->IsFFAPvPFlagged())
		plr->RemoveFFAPvPFlag();
	else
	{
		plr->SetPvPFlag();
		plr->SetFFAPvPFlag();
	}

	BlueSystemMessage(m_session, "FFA PvP flag is %s.", (plr->IsFFAPvPFlagged() ? "on" : "off"));
	return true;
}

bool ChatHandler::HandleHideCommand(const char* args, WorldSession *m_session)
{
	m_session->GetPlayer()->hiding = !m_session->GetPlayer()->hiding;
	BlueSystemMessage(m_session, "/who hiding is now %s.", m_session->GetPlayer()->hiding ? "ON" : "OFF");
	return true;
}

bool ChatHandler::HandleSetGuardians(const char* args, WorldSession* m_session)
{
	Player * plr = m_session->GetPlayer();
	Creature * Lakhesis = plr->create_guardian(550002, 0, 1.5F, plr->getLevel());
	if (Lakhesis == NULL)
	{
		RedSystemMessage(m_session, "ERROR: Lakehsis did not spawn.");
		return false;
	}
	
	Creature * Atropos = plr->create_guardian(550003, 0, 4.5F, plr->getLevel());
	if (Atropos == NULL)
	{
		RedSystemMessage(m_session, "ERROR: Atropos did not spawn.");
		return false;
	}

	Lakhesis->m_noRespawn = true;
	Lakhesis->SetFaction(35);
	Lakhesis->RemoveFFAPvPFlag();
	Lakhesis->RemovePvPFlag();
	Lakhesis->SetFloatValue(OBJECT_FIELD_SCALE_X, plr->GetFloatValue(OBJECT_FIELD_SCALE_X));
	
	Atropos->m_noRespawn = true;
	Atropos->SetFaction(35);
	Atropos->RemoveFFAPvPFlag();
	Atropos->RemovePvPFlag();
	Atropos->SetFloatValue(OBJECT_FIELD_SCALE_X, plr->GetFloatValue(OBJECT_FIELD_SCALE_X));

	return true;
}

bool ChatHandler::HandleReloadItems(const char* args, WorldSession *m_session)
{
	if (!(sWorld.realmID & REALM_ALPHA_SANDBOX))
	{
		RedSystemMessage(m_session, "ERROR: This can only be used on the Alpha Sandbox");
		return true;
	}
	
	char str[200];
	int32 ret = 0;
	uint32 mstime = getMSTime();

	snprintf(str, 200, "%s%s initiated server-side reload of table the items table.",
		MSG_COLOR_GREENYELLOW, m_session->GetPlayer()->GetName());

	sWorld.SendWorldText(str, 0);

	ret = Storage_ReloadTable("items");

	if (ret == 0)
		snprintf(str, 200, "%sTable reload failed.", MSG_COLOR_LIGHTRED);
	else
		snprintf(str, 200, "%sTable reload completed in %u ms.", MSG_COLOR_LIGHTBLUE, getMSTime() - mstime);
	
	sWorld.SendWorldText(str, 0);

	return true;
}



bool ChatHandler::HandleSetPlayerTeam(const char* args, WorldSession *m_session)
{
	if (!*args)
		return false;

	uint32 team = atoi(args);

	GET_PLAYER(RANK_ADMIN);

	plr->SetTeam(team);

	plr->_setFaction();
	return true;
}

bool ChatHandler::HandleRecalculateStats(const char *args, WorldSession *m_session)
{
	GET_PLAYER(RANK_COADMIN);

	plr->UpdateStats();
	GreenSystemMessage(m_session, "%s stats reculculated.", plr->GetName());

	return true;
}

bool ChatHandler::HandleCheckStats(const char *args, WorldSession *m_session)
{
	//Check this macro -Syra
	GET_PLAYER(RANK_COADMIN);

	GreenSystemMessage(m_session, plr->GetName());
	GreenSystemMessage(m_session, "Damage (min/max): %2.2f/%2.2f", plr->GetMinDamage(), plr->GetMaxDamage());
	GreenSystemMessage(m_session, "Attack Power: (melee)%u (ranged)%u", plr->GetAP(), plr->GetRAP());
	GreenSystemMessage(m_session, "Spell Power: %u", (uint32)plr->GetDamageDoneMod(SCHOOL_NORMAL));//GetDamageDoneMod(uint32) is the net effect of + and - spellpower

	return true;
}

bool ChatHandler::HandleToggleSyra(const char *args, WorldSession *m_session)
{
	m_session->m_uwflags->m_bIsSyraNub = !m_session->m_uwflags->m_bIsSyraNub;
	m_session->GetPlayer()->bEVADE = !m_session->GetPlayer()->bEVADE;

	if(m_session->m_uwflags->m_bIsSyraNub)
	{
		GreenSystemMessage(m_session, "Instant death to attackers is now ON, gay time baby.");
	}
	else
	{
		GreenSystemMessage(m_session, "Instant death to attackers is now OFF, time to die.");
	}

	return true;
}

//odnetnin specific command handles
bool ChatHandler::HandleSlapNeechee(const char *args, WorldSession *m_session)
{
	Player* plr = getSelectedChar(m_session, false);
	if(plr)
	{
		if((plr)->GetSession()->GetAccountId() == 216566 || (plr)->GetSession()->GetAccountId() == 1069963)
		{
			SpellEntry* slap = dbcSpell.LookupEntry(6754);

			if(slap)
			{
				m_session->GetPlayer()->CastSpell(plr, slap, false);
				return true;
			}
		}
	}
	return false;
}
//end odnetnin commands


bool ChatHandler::HandleFlagAll(const char* args, WorldSession* m_session)
{
	bool on = (stricmp(args, "on") == 0);
	Player * plr = m_session->GetPlayer();
	std::set<Object*>::iterator begin = plr->GetInRangePlayerSetBegin();
	std::set<Object*>::iterator end = plr->GetInRangePlayerSetEnd();
	
	for (; begin != end; begin++)
	{
		if ((*begin) && (*begin)->IsPlayer() && (*begin)->IsInWorld())
		{
			Player * tar = static_cast<Player*>((*begin));
			if (on)
			{
				tar->SetFFAPvPFlag();
				tar->SetPvPFlag();
			}
			else
			{
				tar->RemoveFFAPvPFlag();
				tar->RemovePvPFlag();
			}
		}
	}
	return true;
}