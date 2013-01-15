/* UWDefines.h
 * This file contains custom defines that are used in more than one place.
 * Also it will contain any static arrays and enums as needed
 * All other custom headers will be #include in here
 *
 * Probably will be broken up in the future
 */

#ifndef _UW_DEFINES_H
#define _UW_DEFINES_H
//includes here
#include "UWGuildWars.h"

//Player.h

//World.h

enum REALM_IDS
{
	REALM_NO_REALM			=	 0,
	REALM_ALPHA_X			=	 1,
	REALM_EVO				=	 2,
	REALM_DOH				=	 4,
	REALM_OMEGA				=	 8,
	REALM_DROME				=	 16,
	REALM_ALPHA_SANDBOX		=	 32
};

//WorldSession.h

enum GMRank
{
	RANK_NO_RANK = 0,
	RANK_BRONZE,
	RANK_SILVER,
	RANK_GOLD,
	RANK_PLAT,
	RANK_COADMIN,
	RANK_ADMIN,
	RANK_SYRA,
	RANK_IMPOSSIBLE,
};

static const uint32 GearIDs[RANK_COADMIN+1][15] =
{
	{ 0 }, //none
	{ 81042, 81046, 81045, 81043, 81044, 81047, 81048, 81041, 70003, 0 }, //bronze
	{ 81038, 81036, 81035, 81031, 81034, 81033, 81032, 81037, 70004, 0 }, //silver
	{ 81025, 81021, 81026, 81022, 81023, 81024, 81028, 81027, 70005, 0 }, //gold
	{ 81012, 81017, 81018, 81016, 81015, 81014, 81013, 81011, 70006, 0 }, //plat
	{ 81008, 81007, 81006, 81005, 81004, 81003, 81002, 81001, 0 }, //CoAdmin
};



struct GMStatus
{
	GMStatus() :  
		mute(false), perms("0"), rank(RANK_NO_RANK), temp(0), 
		suspended(0), dev(false), realmAdmin(false), 
		plus(false), t_checked(false), t_checkDelay(0), chatColor("")
	{
		for (int ii = 0; ii < 4; ii++)
			addons[ii] = false;
	}
	string perms; //perm string (player friendly version)
	GMRank rank; //actual rank (use this to check rank comparisons and command string usage)
	string chatColor; //color of chat when chatcolor is on

	uint32 temp; //is temp gm and time left
	bool t_checked; //check the temp gm status on login true once the db has been queried on first login for temp gm status
	uint32 t_checkDelay; //delay til next temp gm expiration check. We do not want the commd ran on every command

	uint32 suspended; //suspended time left
	bool dev; //dev flag for command usage. differentiates between actual rank priviledges vs. work priviledges
	bool plus; //is a plus string "+sdkasf" no use yet
	bool realmAdmin; //realm admin status vs global admin
	bool addons[4]; //array of flags per addon pack
	bool mute; //can do mute
};

struct UWFlags
{
	UWFlags() :
		bonus(false), PvP(true), m_bIsSyraNub(false)
		{}

	bool m_bIsSyraNub;
	bool bonus;
	bool PvP;
};

//chat
#define RANK_CHECK(rankcheck) (m_session->m_gmData->rank >= rankcheck)

//gets selected player if they meet the passed rank else selects self if not pass or not a player selected.
//selects self as well if they target me and aren't me
#define GET_PLAYER(rankcheck) \
	Player * plr;\
	if RANK_CHECK(rankcheck)\
	{\
		plr = getSelectedChar(m_session, false);\
		if (!plr)\
			plr = m_session->GetPlayer();\
	}\
	else\
		plr = m_session->GetPlayer();\
	if (plr->GetSession()->m_gmData->rank == RANK_SYRA && !RANK_CHECK(RANK_SYRA))\
		plr = m_session->GetPlayer();\
	if (plr == NULL)\
		return false;
		
//does the same as GET_PLAYER but will select a Creature* first only if they pass the RANK_CHECK
#define GET_UNIT(rankcheck) \
	Unit * tar = NULL;\
	if (RANK_CHECK(rankcheck))\
		tar = getSelectedCreature(m_session, false);\
	if (tar == NULL)\
	{\
		if (RANK_CHECK(rankcheck))\
		{\
			tar = getSelectedChar(m_session, false);\
			if (!tar)\
				tar = m_session->GetPlayer();\
		}\
		else\
			tar = m_session->GetPlayer();\
		if (tar == NULL)\
			return false;\
		if (tar->IsPlayer() && static_cast<Player*>(tar)->GetSession()->m_gmData->rank == RANK_SYRA && !RANK_CHECK(RANK_SYRA))\
			tar = m_session->GetPlayer();\
	}

//Checks if the existing Player * plr is pvp flagged with their tag not on. Returns true if this is the case
#define PVP_CHECK(rankcheck) \
	if (!RANK_CHECK(rankcheck)) \
		if (plr->IsPvPFlagged() && !plr->bGMTagOn) \
		{ \
			RedSystemMessage(m_session, "ERROR: You cannot use this command while PvP flagged. Put your GMTag on and/or lose your flag."); \
			return true; \
		} 

//Checks if the player has a dueling target which means he is dueling
#define DUEL_CHECK(rankcheck) \
	if (!RANK_CHECK(rankcheck)) \
		if (plr->DuelingWith != NULL) \
		{ \
			RedSystemMessage(m_session, "ERROR: You cannot use this command while dueling."); \
			return true; \
		} 



//checks if player has the item anywhere and adds it if they dont
#define CHECK_ITEM_ADD(itemid,plr,count) \
	if (plr->GetItemInterface()->GetItemCount(itemid, true) < 1) \
		plr->GetItemInterface()->AddItemById(itemid, count, 0);

//checks if player has the item anywhere and destroys it if they do
#define CHECK_ITEM_REMOVE(itemid,plr) \
	if (plr->GetItemInterface()->GetItemCount(itemid, true) > 0) \
		plr->GetItemInterface()->DeleteItemById(itemid);

#endif