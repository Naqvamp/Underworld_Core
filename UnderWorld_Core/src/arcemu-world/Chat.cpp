/*
 * ArcEmu MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2010 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

initialiseSingleton( ChatHandler );
initialiseSingleton(CommandTableStorage);

ChatCommand * ChatHandler::getCommandTable()
{
	Arcemu::Util::ARCEMU_ASSERT(   false);
	return 0;
}

ChatCommand * CommandTableStorage::GetSubCommandTable(const char * name)
{
	if(!stricmp(name, "modify"))
		return _modifyCommandTable;
	else if(!stricmp(name, "waypoint"))
		return _waypointCommandTable;
	else if(!stricmp(name, "debug"))
		return _debugCommandTable;
	else if(!stricmp(name, "gmTicket"))
		return _GMTicketCommandTable;
	else if(!stricmp(name, "gobject"))
		return _GameObjectCommandTable;
	else if(!stricmp(name, "battleground"))
		return _BattlegroundCommandTable;
	else if(!stricmp(name, "npc"))
		return _NPCCommandTable;
	else if(!stricmp(name, "vehicle"))
		return _VehicleCommandTable;
	else if(!stricmp(name, "cheat"))
		return _CheatCommandTable;
	else if(!stricmp(name, "account"))
		return _accountCommandTable;
	else if(!stricmp(name, "honor"))
		return _honorCommandTable;
	else if(!stricmp(name, "quest"))
		return _questCommandTable;
	else if(!stricmp(name, "pet"))
		return _petCommandTable;
	else if(!stricmp(name, "recall"))
		return _recallCommandTable;
	else if(!stricmp(name, "guild"))
		return _GuildCommandTable;
	else if(!stricmp(name, "gm"))
		return _gmCommandTable;
	else if(!stricmp(name, "server"))
		return _serverCommandTable;
	else if(!stricmp(name, "character"))
		return _characterCommandTable;
	else if(!stricmp(name, "lookup"))
		return _lookupCommandTable;
	else if(!stricmp(name, "admin"))
		return _adminCommandTable;
	else if(!stricmp(name, "kick"))
		return _kickCommandTable;
	else if(!stricmp(name, "ban"))
		return _banCommandTable;
	else if(!stricmp(name, "unban"))
		return _unbanCommandTable;
	else if(!stricmp(name, "instance"))
		return _instanceCommandTable;
	else if(!stricmp(name, "arena"))
		return _arenaCommandTable;
	else if(!stricmp(name, "achieve"))
		return _achievementCommandTable;
	else if(!strcmp(name, "spells"))
		return _spellsCommandTable;
	else if(!strcmp(name, "packone"))
		return _packoneCommandTable;
	else if(!strcmp(name, "packtwo"))
		return _packtwoCommandTable;
	else if(!strcmp(name, "packthree"))
		return _packthreeCommandTable;
	else if(!strcmp(name, "event"))
		return _eventCommandTable;
	else if(!strcmp(name, "syra"))
		return _syraCommandTable;
	else if(!strcmp(name, "odnetnin"))
		return _odnetninCommandTable;
	else if(!strcmp(name, "dev"))
		return _devCommandTable;

	return 0;
}

#define dupe_command_table(ct, dt) this->dt = (ChatCommand*)allocate_and_copy(sizeof(ct)/* / sizeof(ct[0])*/, ct)
ARCEMU_INLINE void* allocate_and_copy(uint32 len, void * pointer)
{
	void * data = (void*)malloc(len);
	memcpy(data, pointer, len);
	return data;
}

void CommandTableStorage::Load()
{
	QueryResult * result = WorldDatabase.Query("SELECT * FROM command_overrides");
	if(!result) return;

	do
	{
		const char * name = result->Fetch()[0].GetString();
		const char * level = result->Fetch()[1].GetString();
		Override(name, level);
	} while(result->NextRow());
	delete result;
}

void CommandTableStorage::Override(const char * command, const char * level)
{
	Arcemu::Util::ARCEMU_ASSERT(   level[0] != '\0');
	char * cmd = strdup(command);

	// find the command we're talking about
	char * sp = strchr(cmd, ' ');
	const char * command_name = cmd;
	const char * subcommand_name = 0;

	if(sp != 0)
	{
		// we're dealing with a subcommand.
		*sp = 0;
		subcommand_name = sp + 1;
	}

	size_t len1 = strlen(command_name);
	size_t len2 = subcommand_name ? strlen(subcommand_name) : 0;

	// look for the command.
	ChatCommand * p = &_commandTable[0];
	while(p->Name != 0)
	{
		if(!strnicmp(p->Name, command_name, len1))
		{
			// this is the one we wanna modify
			if(!subcommand_name)
			{
				// no subcommand, we can change it.
				p->CommandGroup = level[0];
				printf("Changing command level of command `%s` to %c.\n", p->Name, level[0]);
			}
			else
			{
				// assume this is a subcommand, loop the second set.
				ChatCommand * p2 = p->ChildCommands;
				if(!p2)
				{
					printf("Invalid command specified for override: %s\n", command_name);
				}
				else
				{
					while(p2->Name != 0)
					{
						if(!strnicmp("*",subcommand_name,1))
						{
							p2->CommandGroup = level[0];
							printf("Changing command level of command (wildcard) `%s`:`%s` to %c.\n", p->Name, p2->Name, level[0]);
						}else{
							if(!strnicmp(p2->Name, subcommand_name, len2))
							{
								// change the level
								p2->CommandGroup = level[0];
								printf("Changing command level of command `%s`:`%s` to %c.\n", p->Name, p2->Name, level[0]);
								break;
							}
						}
						p2++;
					}
					if(p2->Name == 0)
					{
						if(strnicmp("*",subcommand_name,1)) //Hacky.. meh.. -DGM
						{
							printf("Invalid subcommand referenced: `%s` under `%s`.\n", subcommand_name, p->Name);
						}
						break;
					}
				}
			}
			break;
		}
		++p;
	}

	if(p->Name == 0)
	{
		printf("Invalid command referenced: `%s`\n", command_name);
	}

	free(cmd);
}

void CommandTableStorage::Dealloc()
{
	free( _modifyCommandTable );
	free( _debugCommandTable );
	free( _waypointCommandTable );
	free( _GMTicketCommandTable );
	free( _GuildCommandTable );
	free( _GameObjectCommandTable );
	free( _BattlegroundCommandTable );
	free( _NPCCommandTable );
	free( _VehicleCommandTable );
	free( _CheatCommandTable );
	free( _accountCommandTable );
	free( _honorCommandTable );
	free( _petCommandTable );
	free( _recallCommandTable );
	free( _questCommandTable );
	free( _serverCommandTable );
	free( _gmCommandTable );
	free( _characterCommandTable );
	free( _lookupCommandTable );
	free( _adminCommandTable );
	free( _kickCommandTable );
	free( _banCommandTable );
	free( _unbanCommandTable );
	free( _instanceCommandTable );
	free( _arenaCommandTable );
	free( _achievementCommandTable );
	free( _commandTable );
	free( _spellsCommandTable );
	free( _packoneCommandTable );
	free( _packtwoCommandTable );
	free( _packthreeCommandTable );
	free( _eventCommandTable );
	free( _syraCommandTable );
	free( _odnetninCommandTable );
	free( _devCommandTable );
}

void CommandTableStorage::Init()
{
	static ChatCommand modifyCommandTable[] =
	{
		{ "hp",       'z', NULL,                  "Modifies health points (HP) of selected target",         NULL, UNIT_FIELD_HEALTH,         UNIT_FIELD_MAXHEALTH, 1 },
		{ "gender",     'z', &ChatHandler::HandleGenderChanger,   "Changes gender of selected target. Usage: 0=male, 1=female.",   NULL, 0,                 0,          0 },
		{ "mana",      'z', NULL,                  "Modifies mana points (MP) of selected target.",          NULL, UNIT_FIELD_POWER1,         UNIT_FIELD_MAXPOWER1, 1 },
		{ "rage",      'z', NULL,                  "Modifies rage points of selected target.",            NULL, UNIT_FIELD_POWER2,         UNIT_FIELD_MAXPOWER2, 1 },
		{ "energy",     'z', NULL,                  "Modifies energy points of selected target.",           NULL, UNIT_FIELD_POWER4,         UNIT_FIELD_MAXPOWER4, 1 },
		{ "runicpower",   'z', NULL,                  "Modifies runic power points of selected target.",         NULL, UNIT_FIELD_POWER7,         UNIT_FIELD_MAXPOWER7, 1 },
		{ "level",      'x', &ChatHandler::HandleModifyLevelCommand, "Modifies the level of selected target.",             NULL, 0,                 0,          0 },
		{ "strength",    'z', NULL,                  "Modifies the strength value of the selected target.",       NULL, UNIT_FIELD_STAT0,         0,          1 },
		{ "agility",     'z', NULL,                  "Modifies the agility value of the selected target.",       NULL, UNIT_FIELD_STAT1,         0,          1 },
		{ "intelligence",  'z', NULL,                  "Modifies the intelligence value of the selected target.",     NULL, UNIT_FIELD_STAT3,         0,          1 },
		{ "spirit",     'z', NULL,                  "Modifies the spirit value of the selected target.",        NULL, UNIT_FIELD_STAT4,         0,          1 },
		{ "armor",      'z', NULL,                  "Modifies the armor of selected target.",             NULL, UNIT_FIELD_RESISTANCES,      0,          1 },
		{ "holy",      'z', NULL,                  "Modifies the holy resistance of selected target.",        NULL, UNIT_FIELD_RESISTANCES+1,     0,          1 },
		{ "fire",      'z', NULL,                  "Modifies the fire resistance of selected target.",        NULL, UNIT_FIELD_RESISTANCES+2,     0,          1 },
		{ "nature",     'z', NULL,                  "Modifies the nature resistance of selected target.",       NULL, UNIT_FIELD_RESISTANCES+3,     0,          1 },
		{ "frost",      'z', NULL,                  "Modifies the frost resistance of selected target.",        NULL, UNIT_FIELD_RESISTANCES+4,     0,          1 },
		{ "shadow",     'z', NULL,                  "Modifies the shadow resistance of selected target.",       NULL, UNIT_FIELD_RESISTANCES+5,     0,          1 },
		{ "arcane",     'z', NULL,                  "Modifies the arcane resistance of selected target.",       NULL, UNIT_FIELD_RESISTANCES+6,     0,          1 },
		{ "damage",     'z', NULL,                  "Modifies the damage done by the selected target.",        NULL, UNIT_FIELD_MINDAMAGE,       UNIT_FIELD_MAXDAMAGE, 2 },
		{ "ap",       'z', NULL,                  "Modifies the attack power of the selected target.",        NULL, UNIT_FIELD_ATTACK_POWER,      0,          1 },
		{ "rangeap",     'z', NULL,                  "Modifies the range attack power of the selected target.",     NULL, UNIT_FIELD_RANGED_ATTACK_POWER,  0,          1 },
		{ "resistances",   'j', &ChatHandler::HandleResistanceCommand, "Modifies your magic resistance levels",              NULL, 0,         0,          0 },
		{ "scale",      'g', &ChatHandler::HandleScaleCommand,    "Modifies the scale of the selected target.",           NULL, 0,            0,          2 },
		{ "gold",      'g', &ChatHandler::HandleModifyGoldCommand, "Modifies the gold amount of the selected target. Copper value.", NULL, 0,                 0,          0 },
		{ "speed",      'g', &ChatHandler::HandleModifySpeedCommand, "Modifies the movement speed of the selected target.",       NULL, 0,                 0,          0 },
		{ "nativedisplayid", 'z', NULL,                  "Modifies the native display identifier of the target.",      NULL, UNIT_FIELD_NATIVEDISPLAYID,    0,          1 },
		{ "displayid",    'z', NULL,                  "Modifies the display identifier (DisplayID) of the target.",   NULL, UNIT_FIELD_DISPLAYID,       0,          1 },
		{ "flags",      'z', NULL,                  "Modifies the flags of the selected target.",           NULL, UNIT_FIELD_FLAGS,         0,          1 },
		{ "faction",     'a', NULL,                  "Modifies the faction template of the selected target.",      NULL, UNIT_FIELD_FACTIONTEMPLATE,    0,          1 },
		{ "dynamicflags",  'z', NULL,                  "Modifies the dynamic flags of the selected target.",       NULL, UNIT_DYNAMIC_FLAGS,        0,          1 },
		{ "talentpoints",  'x', &ChatHandler::HandleModifyTPsCommand,   "Modifies the available talent points of the selected target.",  NULL, 0,           0,          1 },
		{ "happiness",    'z', NULL,                  "Modifies the happiness value of the selected target.",      NULL, UNIT_FIELD_POWER5,         UNIT_FIELD_MAXPOWER5, 1 },
		{ "boundingraidius", 'z', NULL,                  "Modifies the bounding radius of the selected target.",      NULL, UNIT_FIELD_BOUNDINGRADIUS,     0,          2 },
		{ "combatreach",   'z', NULL,                  "Modifies the combat reach of the selected target.",        NULL, UNIT_FIELD_COMBATREACH,      0,          2 },
		{ "npcemotestate",  'z', NULL,                  "Modifies the NPC emote state of the selected target.",      NULL, UNIT_NPC_EMOTESTATE,        0,          1 },
		{ "bytes0",     'z', NULL,                  "WARNING! Modifies the bytes0 entry of selected target.",     NULL, UNIT_FIELD_BYTES_0,        0,          1 },
		{ "bytes1",     'z', NULL,                  "WARNING! Modifies the bytes1 entry of selected target.",     NULL, UNIT_FIELD_BYTES_1,        0,          1 },
		{ "bytes2",     'z', NULL,                  "WARNING! Modifies the bytes2 entry of selected target.",     NULL, UNIT_FIELD_BYTES_2,        0,          1 },
		{ NULL,       '0', NULL,                  "",                                NULL, 0,                 0,          0 }
	};
	dupe_command_table(modifyCommandTable, _modifyCommandTable);

	static ChatCommand debugCommandTable[] =
	{
		{ "infront",       'z', &ChatHandler::HandleDebugInFrontCommand,   "",                                                         NULL, 0, 0, 0 },
		{ "showreact",      'z', &ChatHandler::HandleShowReactionCommand,   "",                                                         NULL, 0, 0, 0 },
		{ "aimove",       'z', &ChatHandler::HandleAIMoveCommand,      "",                                                         NULL, 0, 0, 0 },
		{ "dist",        'z', &ChatHandler::HandleDistanceCommand,     "",                                                         NULL, 0, 0, 0 },
		{ "face",        'z', &ChatHandler::HandleFaceCommand,       "",                                                         NULL, 0, 0, 0 },
		{ "moveinfo",      'z', &ChatHandler::HandleMoveInfoCommand,     "",                                                         NULL, 0, 0, 0 },
		{ "setbytes",      'z', &ChatHandler::HandleSetBytesCommand,     "",                                                         NULL, 0, 0, 0 },
		{ "getbytes",      'z', &ChatHandler::HandleGetBytesCommand,     "",                                                         NULL, 0, 0, 0 },
		{ "unroot",       'z', &ChatHandler::HandleDebugUnroot,       "",                                                         NULL, 0, 0, 0 },
		{ "root",        'z', &ChatHandler::HandleDebugRoot,        "",                                                         NULL, 0, 0, 0 },
		{ "landwalk",      'x', &ChatHandler::HandleDebugLandWalk,      "",                                                         NULL, 0, 0, 0 },
		{ "waterwalk",      'x', &ChatHandler::HandleDebugWaterWalk,     "",                                                         NULL, 0, 0, 0 },
		{ "castspell",      'z', &ChatHandler::HandleCastSpellCommand,    ".castspell <spellid> - Casts spell on target.",                                   NULL, 0, 0, 0 },
		{ "castself",      'z', &ChatHandler::HandleCastSelfCommand,     ".castself <spellId> - Target casts spell <spellId> on itself.",                           NULL, 0, 0, 0 },
		{ "castspellne",     'z', &ChatHandler::HandleCastSpellNECommand,   ".castspellne <spellid> - Casts spell on target (only plays animations, doesn't handle effects or range/facing/etc.", NULL, 0, 0, 0 },
		{ "aggrorange",     'z', &ChatHandler::HandleAggroRangeCommand,    ".aggrorange - Shows aggro Range of the selected Creature.",                             NULL, 0, 0, 0 },
		{ "knockback",      'z', &ChatHandler::HandleKnockBackCommand,    ".knockback <value> - Knocks you back.",                                       NULL, 0, 0, 0 },
		{ "fade",        'z', &ChatHandler::HandleFadeCommand,       ".fade <value> - calls ModThreatModifyer().",                                    NULL, 0, 0, 0 },
		{ "threatMod",      'z', &ChatHandler::HandleThreatModCommand,    ".threatMod <value> - calls ModGeneratedThreatModifyer().",                             NULL, 0, 0, 0 },
		{ "calcThreat",     'z', &ChatHandler::HandleCalcThreatCommand,    ".calcThreat <dmg> <spellId> - calculates threat.",                                 NULL, 0, 0, 0 },
		{ "threatList",     'z', &ChatHandler::HandleThreatListCommand,    ".threatList - returns all AI_Targets of the selected Creature.",                          NULL, 0, 0, 0 },
		{ "gettptime",      'z', &ChatHandler::HandleGetTransporterTime,   "grabs transporter travel time",                                           NULL, 0, 0, 0 },
		{ "itempushresult",   'z', &ChatHandler::HandleSendItemPushResult,   "sends item push result",                                              NULL, 0, 0, 0 },
		{ "setbit",       'z', &ChatHandler::HandleModifyBitCommand,    "",                                                         NULL, 0, 0, 0 },
		{ "setvalue",      'z', &ChatHandler::HandleModifyValueCommand,   "",                                                         NULL, 0, 0, 0 },
		{ "aispelltestbegin",  'z', &ChatHandler::HandleAIAgentDebugBegin,    "",                                                         NULL, 0, 0, 0 },
		{ "aispelltestcontinue", 'z', &ChatHandler::HandleAIAgentDebugContinue,  "",                                                         NULL, 0, 0, 0 },
		{ "aispelltestskip",   'z', &ChatHandler::HandleAIAgentDebugSkip,    "",                                                         NULL, 0, 0, 0 },
		{ "dumpcoords",     'z', &ChatHandler::HandleDebugDumpCoordsCommmand, "",                                                         NULL, 0, 0, 0 },
		{ "sendpacket",     'z', &ChatHandler::HandleSendpacket,       "<opcode ID>, <data>",                                                NULL, 0, 0, 0 },
		{ "sqlquery",      'z', &ChatHandler::HandleSQLQueryCommand,     "<sql query>",                                                    NULL, 0, 0, 0 },
		{ "rangecheck",     'z', &ChatHandler::HandleRangeCheckCommand,    "Checks the 'yard' range and internal range between the player and the target.",                   NULL, 0, 0, 0 },
		{ "setallratings",    'z', &ChatHandler::HandleRatingsCommand,     "Sets rating values to incremental numbers based on their index.",                          NULL, 0, 0, 0 },
		{ "testlos",       'z', &ChatHandler::HandleCollisionTestLOS,    "tests los",                                                     NULL, 0, 0, 0 },
		{ "testindoor",     'z', &ChatHandler::HandleCollisionTestIndoor,   "tests indoor",                                                   NULL, 0, 0, 0 },
		{ "getheight",      'z', &ChatHandler::HandleCollisionGetHeight,   "Gets height",                                                    NULL, 0, 0, 0 },
		{ "deathstate",     'z', &ChatHandler::HandleGetDeathState,      "returns current deathstate for target",                                       NULL, 0, 0, 0 },
		{ "getpos",       'z', &ChatHandler::HandleGetPosCommand,      "",                                                         NULL, 0, 0, 0 },
		{ "sendfailed",    'z', &ChatHandler::HandleSendFailed,   "",                                                         NULL, 0, 0, 0 },
		{ "playmovie",    'z', &ChatHandler::HandlePlayMovie,     "Triggers a movie for a player",         NULL, 0, 0, 0 },
		{ NULL,         '0', NULL,                    "",                                                         NULL, 0, 0, 0 }
	};
	dupe_command_table(debugCommandTable, _debugCommandTable);

	static ChatCommand waypointCommandTable[] =
	{
		{ "add",    'z', &ChatHandler::HandleWPAddCommand,     "Add wp at current pos", NULL, 0, 0, 0 },
		{ "show",   'z', &ChatHandler::HandleWPShowCommand,     "Show wp's for creature", NULL, 0, 0, 0 },
		{ "hide",   'z', &ChatHandler::HandleWPHideCommand,     "Hide wp's for creature", NULL, 0, 0, 0 },
		{ "delete",  'z', &ChatHandler::HandleWPDeleteCommand,    "Delete selected wp",   NULL, 0, 0, 0 },
		{ "movehere", 'z', &ChatHandler::HandleWPMoveHereCommand,   "Move to this wp",    NULL, 0, 0, 0 },
		{ "flags",   'z', &ChatHandler::HandleWPFlagsCommand,    "Wp flags",        NULL, 0, 0, 0 },
		{ "waittime", 'z', &ChatHandler::HandleWPWaitCommand,     "Wait time at this wp",  NULL, 0, 0, 0 },
		{ "emote",   'z', &ChatHandler::HandleWPEmoteCommand,    "Emote at this wp",    NULL, 0, 0, 0 },
		{ "skin",   'z', &ChatHandler::HandleWPSkinCommand,     "Skin at this wp",    NULL, 0, 0, 0 },
		{ "change",  'z', &ChatHandler::HandleWPChangeNoCommand,   "Change at this wp",   NULL, 0, 0, 0 },
		{ "info",   'z', &ChatHandler::HandleWPInfoCommand,     "Show info for wp",    NULL, 0, 0, 0 },
		{ "movetype", 'z', &ChatHandler::HandleWPMoveTypeCommand,   "Movement type at wp",  NULL, 0, 0, 0 },
		{ "generate", 'z', &ChatHandler::HandleGenerateWaypoints,   "Randomly generate wps", NULL, 0, 0, 0 },
		{ "save",   'z', &ChatHandler::HandleSaveWaypoints,     "Save all waypoints",   NULL, 0, 0, 0 },
		{ "deleteall", 'z', &ChatHandler::HandleDeleteWaypoints,    "Delete all waypoints",  NULL, 0, 0, 0 },
		{ "addfly",  'z', &ChatHandler::HandleWaypointAddFlyCommand, "Adds a flying waypoint", NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                   "",            NULL, 0, 0, 0 }
	};
	dupe_command_table(waypointCommandTable, _waypointCommandTable);

	static ChatCommand GMTicketCommandTable[] =
	{
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
		{ "get",       'x', &ChatHandler::HandleGMTicketListCommand,           "Gets GM Ticket list.",                     NULL, 0, 0, 0 },
		{ "getId",      'x', &ChatHandler::HandleGMTicketGetByIdCommand,         "Gets GM Ticket by player name.",                NULL, 0, 0, 0 },
		{ "delId",      'x', &ChatHandler::HandleGMTicketRemoveByIdCommand,        "Deletes GM Ticket by player name.",               NULL, 0, 0, 0 },
#else
		{ "list",      'z', &ChatHandler::HandleGMTicketListCommand,           "Lists all active GM Tickets.",                 NULL, 0, 0, 0 },
		{ "get",       'z', &ChatHandler::HandleGMTicketGetByIdCommand,         "Gets GM Ticket with ID x.",                   NULL, 0, 0, 0 },
		{ "remove",     'z', &ChatHandler::HandleGMTicketRemoveByIdCommand,        "Removes GM Ticket with ID x.",                 NULL, 0, 0, 0 },
		{ "deletepermanent", 'z', &ChatHandler::HandleGMTicketDeletePermanentCommand,     "Deletes GM Ticket with ID x permanently.",           NULL, 0, 0, 0 },
		{ "assign",     'z', &ChatHandler::HandleGMTicketAssignToCommand,         "Assigns GM Ticket with id x to GM y (if empty to your self).", NULL, 0, 0, 0 },
		{ "release",     'z', &ChatHandler::HandleGMTicketReleaseCommand,         "Releases assigned GM Ticket with ID x.",            NULL, 0, 0, 0 },
		{ "comment",     'z', &ChatHandler::HandleGMTicketCommentCommand,         "Sets comment x to GM Ticket with ID y.",            NULL, 0, 0, 0 },
#endif
		{ "toggle",     'o', &ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand, "Toggles the ticket system status.",               NULL, 0, 0, 0 },
		{ NULL,       '0', NULL,                            "",                               NULL, 0, 0, 0 }
	};
	dupe_command_table(GMTicketCommandTable, _GMTicketCommandTable);

	static ChatCommand GuildCommandTable[] =
	{
		{ "join",     'z', &ChatHandler::HandleGuildJoinCommand,     "Force joins a guild",         NULL, 0, 0, 0 },
		{ "create",    'x', &ChatHandler::CreateGuildCommand,       "Creates a guild.",          NULL, 0, 0, 0 },
		{ "rename",    'a', &ChatHandler::HandleRenameGuildCommand,    "Renames a guild.",          NULL, 0, 0, 0 },
		{ "members",   'z', &ChatHandler::HandleGuildMembersCommand,   "Lists guildmembers and their ranks.", NULL, 0, 0, 0 },
		{ "removeplayer", 'a', &ChatHandler::HandleGuildRemovePlayerCommand, "Removes a player from a guild.",   NULL, 0, 0, 0 },
		{ "disband",   'a', &ChatHandler::HandleGuildDisbandCommand,   "Disbands the guild of your target.", NULL, 0, 0, 0 },
		{ NULL,      '0', NULL,                     "",                  NULL, 0, 0, 0 }
	};
	dupe_command_table(GuildCommandTable, _GuildCommandTable);

	static ChatCommand GameObjectCommandTable[] =
	{
		{ "select",    'a', &ChatHandler::HandleGOSelect,    "Selects the nearest GameObject to you",  NULL, 0, 0, 0 },
		{ "delete",    'a', &ChatHandler::HandleGODelete,    "Deletes selected GameObject",       NULL, 0, 0, 0 },
		{ "spawn",    'a', &ChatHandler::HandleGOSpawn,    "Spawns a GameObject by ID",        NULL, 0, 0, 0 },
		{ "phase",    'z', &ChatHandler::HandleGOPhaseCommand, "<phase> <save> - Phase selected GameObject", NULL, 0, 0, 0 },
		{ "info",     'a', &ChatHandler::HandleGOInfo,     "Gives you information about selected GO", NULL, 0, 0, 0 },
		{ "activate",   'z', &ChatHandler::HandleGOActivate,   "Activates/Opens the selected GO.",     NULL, 0, 0, 0 },
		{ "enable",    'z', &ChatHandler::HandleGOEnable,    "Enables the selected GO for use.",     NULL, 0, 0, 0 },
		{ "scale",    'a', &ChatHandler::HandleGOScale,    "Sets scale of selected GO",        NULL, 0, 0, 0 },
		{ "animprogress", 'z', &ChatHandler::HandleGOAnimProgress, "Sets anim progress",            NULL, 0, 0, 0 },
		{ "export",    'a', &ChatHandler::HandleGOExport,    "Exports the current GO selected",     NULL, 0, 0, 0 },
		{ "move",     'a', &ChatHandler::HandleGOMove,     "Moves gameobject to player xyz",      NULL, 0, 0, 0 },
		{ "rotate",    'z', &ChatHandler::HandleGORotate,    "<Axis> <Value> - Rotates the object. <Axis> x,y, Default o.",       NULL, 0, 0, 0 },
		{ "sdid",     'z', &ChatHandler::HandleSpawnByDisplayId, "Spawns a generic game object with the specified display id. Clear your client cache after using this!", NULL, 0, 0, 0 },
		{ "portto",    'z', &ChatHandler::HandlePortToGameObjectSpawnCommand, "Teleports you to the gameobject with spawn id x.", NULL, 0, 0, 0 },
		{ NULL,      '0', NULL,                "",                     NULL, 0, 0, 0 }
	};
	dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

	static ChatCommand BattlegroundCommandTable[] =
	{
		{ "setbgscore",  'z', &ChatHandler::HandleSetBGScoreCommand,            "<Teamid> <Score> - Sets battleground score. 2 Arguments.",   NULL, 0, 0, 0 },
		{ "startbg",    'z', &ChatHandler::HandleStartBGCommand,             "Starts current battleground match.",              NULL, 0, 0, 0 },
		{ "pausebg",    'z', &ChatHandler::HandlePauseBGCommand,             "Pauses current battleground match.",              NULL, 0, 0, 0 },
		{ "bginfo",    'z', &ChatHandler::HandleBGInfoCommnad,              "Displays information about current battleground.",       NULL, 0, 0, 0 },
		{ "battleground", 'z', &ChatHandler::HandleBattlegroundCommand,           "Shows BG Menu",                         NULL, 0, 0, 0 },
		{ "setworldstate", 'z', &ChatHandler::HandleSetWorldStateCommand,          "<var> <val> - Var can be in hex. WS Value.",          NULL, 0, 0, 0 },
		{ "setworldstates",'z', &ChatHandler::HandleSetWorldStatesCommand,          "<var> <val> - Var can be in hex. WS Value.",          NULL, 0, 0, 0 },
		{ "playsound",   'z', &ChatHandler::HandlePlaySoundCommand,            "<val>. Val can be in hex.",                   NULL, 0, 0, 0 },
		{ "setbfstatus",  'z', &ChatHandler::HandleSetBattlefieldStatusCommand,       ".setbfstatus - NYI.",                      NULL, 0, 0, 0 },
		{ "leave",     'z', &ChatHandler::HandleBattlegroundExitCommand,         "Leaves the current battleground.",               NULL, 0, 0, 0 },
		{ "getqueue",   'z', &ChatHandler::HandleGetBattlegroundQueueCommand,       "Gets common battleground queue information.",          NULL, 0, 0, 0 },
		{ "forcestart",  'z', &ChatHandler::HandleInitializeAllQueuedBattlegroundsCommand, "Forces initialization of all battlegrounds with active queue.", NULL, 0, 0, 0 },
		{ NULL,      '0', NULL,                            "",                               NULL, 0, 0, 0 }
	};
	dupe_command_table(BattlegroundCommandTable, _BattlegroundCommandTable);

	static ChatCommand NPCCommandTable[] =
	{
		{ "standstate",   'z', &ChatHandler::HandleStandStateCommand,  "Values 0-8, alters target standstate",     NULL, 0, 0, 0},
		{ "vendoradditem",  'z', &ChatHandler::HandleItemCommand,      "Adds to vendor",                                                             NULL, 0, 0, 0 },
		{ "vendorremoveitem", 'z', &ChatHandler::HandleItemRemoveCommand,   "Removes from vendor.",                                                          NULL, 0, 0, 0 },
		{ "flags",      'z', &ChatHandler::HandleNPCFlagCommand,    "Changes NPC flags",                                                            NULL, 0, 0, 0 },
		{ "emote",      'z', &ChatHandler::HandleEmoteCommand,     ".emote - Sets emote state",                                                        NULL, 0, 0, 0 },
		{ "delete",      'z', &ChatHandler::HandleDeleteCommand,     "Deletes mob from db and world.",                                                     NULL, 0, 0, 0 },
		{ "info",       'i', &ChatHandler::HandleNpcInfoCommand,    "Displays NPC information",                                                        NULL, 0, 0, 0 },
		{ "phase",      'z', &ChatHandler::HandleCreaturePhaseCommand, "<phase> <save> - Sets phase of selected mob",                                               NULL, 0, 0, 0 },
		{ "addAgent",     'z', &ChatHandler::HandleAddAIAgentCommand,   ".npc addAgent <agent> <procEvent> <procChance> <procCount> <spellId> <spellType> <spelltargetType> <spellCooldown> <floatMisc1> <Misc2>", NULL, 0, 0, 0 },
		{ "listAgent",    'z', &ChatHandler::HandleListAIAgentCommand,  ".npc listAgent",                                                             NULL, 0, 0, 0 },
		{ "say",       'i', &ChatHandler::HandleMonsterSayCommand,   ".npc say <text> - Makes selected mob say text <text>.",                                          NULL, 0, 0, 0 },
		{ "yell",       'i', &ChatHandler::HandleMonsterYellCommand,  ".npc yell <Text> - Makes selected mob yell text <text>.",                                         NULL, 0, 0, 0 },
		{ "come",       'z', &ChatHandler::HandleNpcComeCommand,    ".npc come - Makes npc move to your position",                                               NULL, 0, 0, 0 },
		{ "return",      'a', &ChatHandler::HandleNpcReturnCommand,   ".npc return - Returns ncp to spawnpoint.",                                                NULL, 0, 0, 0 },
		{ "spawn",      'a', &ChatHandler::HandleCreatureSpawnCommand, ".npc spawn - Spawns npc of entry <id>",                                                  NULL, 0, 0, 0 },
		{ "respawn",     'z', &ChatHandler::HandleCreatureRespawnCommand,".respawn - Respawns a dead npc from its corpse.",                                             NULL, 0, 0, 0 },
		{ "spawnlink",    'z', &ChatHandler::HandleNpcSpawnLinkCommand,  ".spawnlink sqlentry",                                                           NULL, 0, 0, 0 },
		{ "possess",     'a', &ChatHandler::HandleNpcPossessCommand,   ".npc possess - Possess an npc (mind control)",                                              NULL, 0, 0, 0 },
		{ "unpossess",    'a', &ChatHandler::HandleNpcUnPossessCommand,  ".npc unpossess - Unpossess any currently possessed npc.",                                         NULL, 0, 0, 0 },
		{ "select",      'z', &ChatHandler::HandleNpcSelectCommand,   ".npc select - selects npc closest",                                                    NULL, 0, 0, 0 },
		{ "npcfollow",    'z', &ChatHandler::HandleNpcFollowCommand,   "Sets npc to follow you",                                                         NULL, 0, 0, 0 },
		{ "nullfollow",    'z', &ChatHandler::HandleNullFollowCommand,   "Sets npc to not follow anything",                                                     NULL, 0, 0, 0 },
		{ "formationlink1",  'z', &ChatHandler::HandleFormationLink1Command, "Sets formation master.",                                                         NULL, 0, 0, 0 },
		{ "formationlink2",  'z', &ChatHandler::HandleFormationLink2Command, "Sets formation slave with distance and angle",                                              NULL, 0, 0, 0 },
		{ "formationclear",  'z', &ChatHandler::HandleFormationClearCommand, "Removes formation from creature",                                                     NULL, 0, 0, 0 },
		{ "equip1",      'z', &ChatHandler::HandleNPCEquipOneCommand,  "Use: .npc equip1 <itemid> - use .npc equip1 0 to remove the item",                                    NULL, 0, 0, 0 },
		{ "equip2",      'z', &ChatHandler::HandleNPCEquipTwoCommand,  "Use: .npc equip2 <itemid> - use .npc equip2 0 to remove the item",                                    NULL, 0, 0, 0 },
		{ "equip3",      'z', &ChatHandler::HandleNPCEquipThreeCommand, "Use: .npc equip3 <itemid> - use .npc equip3 0 to remove the item",                                    NULL, 0, 0, 0 },
		{ "portto",      'z', &ChatHandler::HandlePortToCreatureSpawnCommand, "Teleports you to the creature with spawn id x.",                                           NULL, 0, 0, 0 },
		{ "loot",       'z', &ChatHandler::HandleNPCLootCommand,    ".npc loot <quality> - displays possible loot for the selected NPC.",                                   NULL, 0, 0, 0 },
		{ NULL,        '0', NULL,                   "",                                                                    NULL, 0, 0, 0 }
	};
	dupe_command_table(NPCCommandTable, _NPCCommandTable);

	static ChatCommand VehicleCommandTable[] =
	{
		{ "spawn",      'z', &ChatHandler::HandleVehicleSpawn,       "Creates a vehicle with the specified creature id",     NULL, 0, 0, 0 },
		{ "possess",     'z', &ChatHandler::HandleVehiclePossess,      "Possess the selected vehicle",               NULL, 0, 0, 0 },
		{ "unpossess",    'z', &ChatHandler::HandleVehicleUnpossess,     "Unpossess a possessed vehicle",              NULL, 0, 0, 0 },
		{ "movespeed",    'z', &ChatHandler::HandleVehicleMoveSpeed,     "Sets the integer movement speed",             NULL, 0, 0, 0 },
		{ "turnspeed",    'z', &ChatHandler::HandleVehicleTurnSpeed,     "Sets the integer turn speed",               NULL, 0, 0, 0 },
		{ "projectilespeed", 'z', &ChatHandler::HandleVehicleProtectileSpeed,  "Sets the integer speed that a projectile travels at",   NULL, 0, 0, 0 },
		{ "turnrad",     'z', &ChatHandler::HandleVehicleTurnRadians,    "Turn x:float radians in orientation (in radians)",     NULL, 0, 0, 0 },
		{ "move",      'z', &ChatHandler::HandleVehicleMove,       "Moves forwards or backwards x:float distance from present location",   NULL, 0, 0, 0 },
		{ "fire",      'z', &ChatHandler::HandleVehicleFire,       "Fires the specified game object id that uses the specified spell id",   NULL, 0, 0, 0 },
		{ NULL,       '0', NULL,                     "",                             NULL, 0, 0, 0 }
	};
	dupe_command_table(VehicleCommandTable, _VehicleCommandTable);

	static ChatCommand CheatCommandTable[] =
	{
		{ "status",   'g', &ChatHandler::HandleShowCheatsCommand,    "Shows active cheats.",              NULL, 0, 0, 0 },
		{ "taxi",    'x', &ChatHandler::HandleTaxiCheatCommand,    "Enables all taxi nodes.",             NULL, 0, 0, 0 },
		{ "cooldown",  'x', &ChatHandler::HandleCooldownCheatCommand,  "Enables no cooldown cheat.",           NULL, 0, 0, 0 },
		{ "casttime",  'j', &ChatHandler::HandleCastTimeCheatCommand,  "Enables no cast time cheat.",           NULL, 0, 0, 0 },
		{ "power",    'i', &ChatHandler::HandlePowerCheatCommand,    "Disables mana consumption etc.",         NULL, 0, 0, 0 },
		{ "god",     'z', &ChatHandler::HandleGodModeCommand,     "Sets god mode, prevents you from taking damage.", NULL, 0, 0, 0 },
		{ "fly",     'j', &ChatHandler::HandleFlyCommand,       "Sets fly mode",                  NULL, 0, 0, 0 },
		{ "explore",   'x', &ChatHandler::HandleExploreCheatCommand,   "Reveals the unexplored parts of the map.",    NULL, 0, 0, 0 },
		{ "stack",    'g', &ChatHandler::HandleAuraStackCheatCommand,  "Enables aura stacking cheat.",          NULL, 0, 0, 0 },
		{ "itemstack",  'z', &ChatHandler::HandleItemStackCheatCommand,  "Enables item stacking cheat.",          NULL, 0, 0, 0 },
		{ "triggerpass", 'j', &ChatHandler::HandleTriggerpassCheatCommand, "Ignores area trigger prerequisites.",       NULL, 0, 0, 0 },
		{ NULL,     '0', NULL,                    "",                        NULL, 0, 0, 0 }
	};
	dupe_command_table(CheatCommandTable, _CheatCommandTable);

	static ChatCommand accountCommandTable[] =
	{
		{ "level",  'w', &ChatHandler::HandleAccountLevelCommand, "Sets gm level on account. Pass it username and 0,1,2,3,az, etc.", NULL, 0, 0, 0 },
		{ "mute",  'a', &ChatHandler::HandleAccountMuteCommand,  "Mutes account for <timeperiod>.",                 NULL, 0, 0, 0 },
		{ "unmute",  'a', &ChatHandler::HandleAccountUnmuteCommand, "Unmutes account <x>",                       NULL, 0, 0, 0 },
		{ "lookup",  'a', &ChatHandler::HandleLookAccountCommand, "Shows which character an account is currently logged in under.", NULL, 0, 0, 0 },
		{ "lookbyip", 'a', &ChatHandler::HandleFindByIPCommand,  "Shows which character an account is currently logged in under BY IP.", NULL, 0, 0, 0 },
		{ "suspend", 'w', &ChatHandler::HandleAccountSuspCommand, ".account suspend <Account_Name> <Duration> <Reason>", NULL, 0, 0, 0 },
		{ "unsuspend", 'w', &ChatHandler::HandleAccountUnSuspCommand, "Removes any suspensions on <x> account. (This does not affect .security changes)", NULL, 0, 0, 0 },
		{ "reloadForced", 'w', &ChatHandler::HandleForcedRelCommand,  "Reloads the REALM-ONLY permissions, use after editing anyone's realm GM string.", NULL, 0, 0, 0 },
		{ "realmGM", 'w', &ChatHandler::HandleRealmGMLevelCommand,  "Sets gm level on account FOR THIS REALM ONLY. This string will override any settings under the account/logon DB.", NULL, 0, 0, 0 },
		{ "tempGM",  'w', &ChatHandler::HandleTempGM,    "SYNTAX: .acc temp <name> <dur> <permissions> - Sets TEMPORARY gm level on account FOR THIS REALM ONLY.", NULL, 0, 0, 0 },
		{ "history", 'a', &ChatHandler::ViewAcctHistory,     "Returns admin history of an account", NULL, 0, 0, 0 },
		{ "whois",  'a', &ChatHandler::HandleGMWhoCommand,   "Looks up permission strings for entered account whether online or not.", NULL, 0, 0, 0 },
		{ "password", 'w', &ChatHandler::HandleAccountPasswordCommand, "<account_name><password> -- Sets new password", NULL, 0, 0, 0 },
		{ "addpoints", 'w', &ChatHandler::HandleAddDonorPointsCommand, "<account_name> <points> -- adds points to entered account", NULL, 0, 0, 0 },
		{ "removepoints", 'w', &ChatHandler::HandleRemoveDonorPointsCommand, "<account_name> <points> -- removes points from entered account", NULL, 0, 0, 0 },
		{ "checkpoints", 'w', &ChatHandler::HandleCheckDonorPointsCommand, "<account_name> -- displays points of the entered account", NULL, 0, 0, 0 },
		{ NULL,   '0', NULL,                   "",                                NULL, 0, 0, 0 }
	};
	dupe_command_table(accountCommandTable, _accountCommandTable);

	static ChatCommand honorCommandTable[] =
	{
		{ "addpoints",     'z', &ChatHandler::HandleAddHonorCommand,          "Adds x amount of honor points/currency",         NULL, 0, 0, 0 },
		{ "addkills",     'z', &ChatHandler::HandleAddKillCommand,           "Adds x amount of honor kills",              NULL, 0, 0, 0 },
		{ "globaldailyupdate", 'z', &ChatHandler::HandleGlobalHonorDailyMaintenanceCommand, "Daily honor field moves",                 NULL, 0, 0, 0 },
		{ "singledailyupdate", 'z', &ChatHandler::HandleNextDayCommand,           "Daily honor field moves for selected player only",    NULL, 0, 0, 0 },
		{ "pvpcredit",     'z', &ChatHandler::HandlePVPCreditCommand,          "Sends PVP credit packet, with specified rank and points", NULL, 0, 0, 0 },
		{ NULL,        '0', NULL,                          "",                            NULL, 0, 0, 0 }
	};
	dupe_command_table(honorCommandTable, _honorCommandTable);

	static ChatCommand petCommandTable[] =
	{
		{ "createpet",  'z', &ChatHandler::HandleCreatePetCommand,   "Creates a pet with <entry>.",              NULL, 0, 0, 0 },
		{ "dismiss",   'z', &ChatHandler::HandleDismissPetCommand,   "Dismisses selected pet.",                NULL, 0, 0, 0 },
		{ "renamepet",  'z', &ChatHandler::HandleRenamePetCommand,   "Renames a pet to <name>.",                NULL, 0, 0, 0 },
		{ "addspell",  'z', &ChatHandler::HandleAddPetSpellCommand,  "Teaches pet <spell>.",                  NULL, 0, 0, 0 },
		{ "removespell", 'z', &ChatHandler::HandleRemovePetSpellCommand, "Removes pet spell <spell>.",               NULL, 0, 0, 0 },
		{ "setlevel",  'z', &ChatHandler::HandlePetLevelCommand,    "Sets pet level to <level>.",               NULL, 0, 0, 0 },
#ifdef USE_SPECIFIC_AIAGENTS
		{ "spawnbot",  'z', &ChatHandler::HandlePetSpawnAIBot,     ".pet spawnbot <type> - spawn a helper bot for your aid", NULL, 0, 0, 0 },
#endif
		{ NULL,     '0', NULL,                   "",                            NULL, 0, 0, 0 }
	};
	dupe_command_table(petCommandTable, _petCommandTable);

	static ChatCommand recallCommandTable[] =
	{
		{ "list",    'g', &ChatHandler::HandleRecallListCommand,    "List recall locations",   NULL, 0, 0, 0 },
		{ "add",    'x', &ChatHandler::HandleRecallAddCommand,    "Add a recall location",    NULL, 0, 0, 0 },
		{ "del",    'z', &ChatHandler::HandleRecallDelCommand,    "Remove a recall location", NULL, 0, 0, 0 },
		{ "port",    'g', &ChatHandler::HandleRecallGoCommand,     "Ports you to recalled location", NULL, 0, 0, 0 },
		{ "portplayer", 'a', &ChatHandler::HandleRecallPortPlayerCommand, "Ports specified player to a recalled location", NULL, 0, 0, 0 },
		{ "find",  '4', &ChatHandler::HandleRecallFindCommand,   "Look for recall by name",  NULL, 0, 0, 0},
		{ "portus",  'z', &ChatHandler::HandleRecallPortUsCommand,  "Ports you and the selected player to recalled location",    NULL, 0, 0, 0 },
		{ NULL,     '0', NULL,                    "",             NULL, 0, 0, 0 }
	};
	dupe_command_table(recallCommandTable, _recallCommandTable);

	static ChatCommand questCommandTable[] =
	{
		{ "addboth",  'z', &ChatHandler::HandleQuestAddBothCommand,  "Add quest <id> to the targeted NPC as start & finish",   NULL, 0, 0, 0 },
		{ "addfinish", 'z', &ChatHandler::HandleQuestAddFinishCommand, "Add quest <id> to the targeted NPC as finisher",      NULL, 0, 0, 0 },
		{ "addstart", 'z', &ChatHandler::HandleQuestAddStartCommand, "Add quest <id> to the targeted NPC as starter",       NULL, 0, 0, 0 },
		{ "delboth",  'z', &ChatHandler::HandleQuestDelBothCommand,  "Delete quest <id> from the targeted NPC as start & finish", NULL, 0, 0, 0 },
		{ "delfinish", 'z', &ChatHandler::HandleQuestDelFinishCommand, "Delete quest <id> from the targeted NPC as finisher",    NULL, 0, 0, 0 },
		{ "delstart", 'z', &ChatHandler::HandleQuestDelStartCommand, "Delete quest <id> from the targeted NPC as starter",    NULL, 0, 0, 0 },
		{ "complete", 'z', &ChatHandler::HandleQuestFinishCommand,  "Complete/Finish quest <id>",                NULL, 0, 0, 0 },
		{ "finisher", 'z', &ChatHandler::HandleQuestFinisherCommand, "Lookup quest finisher for quest <id>",           NULL, 0, 0, 0 },
		{ "item",   '1', &ChatHandler::HandleQuestItemCommand,   "Lookup itemid necessary for quest <id>",          NULL, 0, 0, 0 },
		{ "list",   'z', &ChatHandler::HandleQuestListCommand,   "Lists the quests for the npc <id>",             NULL, 0, 0, 0 },
		{ "load",   'z', &ChatHandler::HandleQuestLoadCommand,   "Loads quests from database",                NULL, 0, 0, 0 },
		{ "lookup",  '1', &ChatHandler::HandleQuestLookupCommand,  "Looks up quest string x",                  NULL, 0, 0, 0 },
		{ "giver",   'z', &ChatHandler::HandleQuestGiverCommand,   "Lookup quest giver for quest <id>",             NULL, 0, 0, 0 },
		{ "remove",  'z', &ChatHandler::HandleQuestRemoveCommand,  "Removes the quest <id> from the targeted player",      NULL, 0, 0, 0 },
		{ "reward",  'z', &ChatHandler::HandleQuestRewardCommand,  "Shows reward for quest <id>",                NULL, 0, 0, 0 },
		{ "status",  'z', &ChatHandler::HandleQuestStatusCommand,  "Lists the status of quest <id>",              NULL, 0, 0, 0 },
		{ "start",   'z', &ChatHandler::HandleQuestStartCommand,   "Starts quest <id>",                     NULL, 0, 0, 0 },
		{ "startspawn",'1', &ChatHandler::HandleQuestStarterSpawnCommand, "Port to spawn location for quest <id> (starter)",    NULL, 0, 0, 0 },
		{ "finishspawn",'1', &ChatHandler::HandleQuestFinisherSpawnCommand,"Port to spawn location for quest <id> (finisher)",    NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                   "",                             NULL, 0, 0, 0 }
	};
	dupe_command_table(questCommandTable, _questCommandTable);

	static ChatCommand serverCommandTable[] =
	{
		{ "disablespell", 'a', &ChatHandler::HandleDisableSpellCommand,  ".server disablespell <spell_id> <reason> -- Must reload `spell_disable` after to take effect.",                        NULL, 0, 0, 0 },
		{ "setmotd",    'z', &ChatHandler::HandleSetMotdCommand,     "Sets MOTD",                        NULL, 0, 0, 0 },
		{ "rehash",    'w', &ChatHandler::HandleRehashCommand,     "Reloads config file.",                   NULL, 0, 0, 0 },
		//{ "reloadscripts", 'z', &ChatHandler::HandleReloadScriptsCommand,  "Reloads GM Scripts",                    NULL, 0, 0, 0 },
		{ "reloadtable",  'w', &ChatHandler::HandleDBReloadCommand,    "Reloads some of the database tables",           NULL, 0, 0, 0 },
		{ "shutdown",   'w', &ChatHandler::HandleShutdownCommand,    "Initiates server shutdown in <x> seconds (5 by default).", NULL, 0, 0, 0 },
		{ "restart",    'z', &ChatHandler::HandleShutdownRestartCommand, "Initiates server restart in <x> seconds (5 by default).", NULL, 0, 0, 0 },
		{ "cancelshutdown",'z', &ChatHandler::HandleCancelShutdownCommand, "Cancels a Server Restart/Shutdown.",      NULL, 0, 0, 0 },
		{ "save",     'w', &ChatHandler::HandleSaveCommand,      "Save's target character",                  NULL, 0, 0, 0 },
		{ "saveall",    'w', &ChatHandler::HandleSaveAllCommand,     "Save's all playing characters",              NULL, 0, 0, 0 },
		{ "info",     'a', &ChatHandler::HandleInfoCommand,      "Server info",                       NULL, 0, 0, 0 },
		{ "netstatus",   'o', &ChatHandler::HandleNetworkStatusCommand,  "Shows network status.", NULL, 0, 0, 0 },
		{ NULL,      '0', NULL,                    "",                             NULL, 0, 0, 0 }
	};
	dupe_command_table(serverCommandTable, _serverCommandTable);

	static ChatCommand gmCommandTable[] =
	{
		{ "gear",     'p', &ChatHandler::HandleGMGear,    "Adds your current rank's gear set.",                  NULL, 0, 0, 0 },
		{ "list",     'g', &ChatHandler::HandleGMListCommand,    "Shows active GM's",                  NULL, 0, 0, 0 },
		{ "status",    'z', &ChatHandler::HandleGMStatusCommand,   "Shows status of your gm flags",            NULL, 0, 0, 0 },
		{ "off",      'x', &ChatHandler::HandleGMOffCommand,     "Sets GM tag off",                   NULL, 0, 0, 0 },
		{ "on",      'x', &ChatHandler::HandleGMOnCommand,     "Sets GM tag on",                   NULL, 0, 0, 0 },
		//  { "whisperblock", 'z', &ChatHandler::HandleWhisperBlockCommand, "Blocks like .gmon except without the <GM> tag",    NULL, 0, 0, 0 },
		{ "allowwhispers", 'x', &ChatHandler::HandleAllowWhispersCommand, "Allows whispers from player <s> while in gmon mode.", NULL, 0, 0, 0 },
		{ "blockwhispers", 'x', &ChatHandler::HandleBlockWhispersCommand, "Blocks whispers from player <s> while in gmon mode.", NULL, 0, 0, 0 },
		{ NULL,      '0', NULL,                   "",                          NULL, 0, 0, 0 }
	};
	dupe_command_table(gmCommandTable, _gmCommandTable);

	static ChatCommand characterCommandTable[] =
	{
		{ "learn",        'z', &ChatHandler::HandleLearnCommand,      "Learns spell",                                                   NULL, 0, 0, 0 },
		{ "learnall",      'z', &ChatHandler::HandleLearnAllSpellCommand,  "Turbo-Charged learn all",                                                   NULL, 0, 0, 0 },
		{ "unlearn",       'z', &ChatHandler::HandleUnlearnCommand,     "Unlearns spell",                                                  NULL, 0, 0, 0 },
		{ "getskillinfo",    'g', &ChatHandler::HandleGetSkillsInfoCommand,  "Gets all the skills from a player",                                         NULL, 0, 0, 0 },
		{ "learnskill",     'z', &ChatHandler::HandleLearnSkillCommand,    ".learnskill <skillid> (optional) <value> <maxvalue> - Learns skill id skillid.",                  NULL, 0, 0, 0 },
		{ "advanceskill",    'z', &ChatHandler::HandleModifySkillCommand,   "advanceskill <skillid> <amount, optional, default = 1> - Advances skill line x times..",              NULL, 0, 0, 0 },
		{ "removeskill",     'z', &ChatHandler::HandleRemoveSkillCommand,   ".removeskill <skillid> - Removes skill",                                      NULL, 0, 0, 0 },
		{ "increaseweaponskill", 'g', &ChatHandler::HandleIncreaseWeaponSkill,   ".increaseweaponskill <count> - Increase equipped weapon skill x times (defaults to 1).",               NULL, 0, 0, 0 },
		{ "resetreputation",   'a', &ChatHandler::HandleResetReputationCommand, ".resetreputation - Resets reputation to start levels. (use on characters that were made before reputation fixes.)", NULL, 0, 0, 0 },
		{ "resetspells",     'a', &ChatHandler::HandleResetSpellsCommand,   ".resetspells - Resets all spells to starting spells of targeted player. DANGEROUS.",                NULL, 0, 0, 0 },
		{ "resettalents",    'a', &ChatHandler::HandleResetTalentsCommand,   ".resettalents - Resets all talents of targeted player to that of their current level. DANGEROUS.",         NULL, 0, 0, 0 },
		{ "resetskills",     'a', &ChatHandler::HandleResetSkillsCommand,   ".resetskills - Resets all skills.",                                         NULL, 0, 0, 0 },
		{ "additem",       'x', &ChatHandler::HandleAddInvItemCommand,    "Adds item x count y",                                                         NULL, 0, 0, 0 },
		{ "removeitem",     'a', &ChatHandler::HandleRemoveItemCommand,    "Removes item %u count %u.",                                             NULL, 0, 0, 0 },
		{ "additemset",     '4', &ChatHandler::HandleAddItemSetCommand,    "Adds item set to inv.",                                               NULL, 0, 0, 0 },
		{ "advanceallskills",  'x', &ChatHandler::HandleAdvanceAllSkillsCommand, "Advances all skills <x> points.",                                          NULL, 0, 0, 0 },
		{ "getstanding",     'x', &ChatHandler::HandleGetStandingCommand,   "Gets standing of faction %u.",                                           NULL, 0, 0, 0 },
		{ "setstanding",     'x', &ChatHandler::HandleSetStandingCommand,   "Sets stanging of faction %u.",                                           NULL, 0, 0, 0 },
		{ "showitems",      'x', &ChatHandler::HandleShowItems,        "Shows items of selected Player",                                          NULL, 0, 0, 0 },
		{ "showskills",     'a', &ChatHandler::HandleShowSkills,       "Shows skills of selected Player",                                          NULL, 0, 0, 0 },
		{ "showinstances",    'z', &ChatHandler::HandleShowInstancesCommand,  "Shows persistent instances of selected Player",                                   NULL, 0, 0, 0 },
		{ "rename",       'z', &ChatHandler::HandleRenameCommand,      "Renames character x to y.",                                             NULL, 0, 0, 0 },
		{ "alltitles",      'o', &ChatHandler::HandleAllTitleCommand,     "Selected target gains all titels",                                             NULL, 0, 0, 0 },
		{ "forcerename",     'm', &ChatHandler::HandleForceRenameCommand,   "Forces character x to rename his char next login",                                 NULL, 0, 0, 0 },
		{ "repairitems",     'x', &ChatHandler::HandleRepairItemsCommand,   ".repairitems - Repair all items from selected player",                               NULL, 0, 0, 0 },
		{ "settitle",    'z', &ChatHandler::HandleSetTitle,      "Adds title to a player",                     NULL, 0, 0, 0 },
		{ "phase",        'z', &ChatHandler::HandlePhaseCommand,      "<phase> - Sets phase of selected player",                                      NULL, 0, 0, 0 },
		{ "ffatoggle",        'z', &ChatHandler::HandleFFAToggleCommand,      "Toggles the PvP flag of your target",                                      NULL, 0, 0, 0 },
		{ "pvptoggle",        'g', &ChatHandler::HandlePVPToggleCommand,      "Toggles the FFA flag of your target",                                      NULL, 0, 0, 0 },
		{ "checkstats", 'a', &ChatHandler::HandleCheckStats, "Gets the players Damage, Spell power, and Attack power.", NULL, 0, 0, 0 },
		{ "recalculatestats", 'a', &ChatHandler::HandleRecalculateStats, "Recalculates the players stats.", NULL, 0, 0, 0 },
		{ NULL,         '0', NULL,                    "",                                                         NULL, 0, 0, 0 }
	};
	dupe_command_table(characterCommandTable, _characterCommandTable);

	static ChatCommand lookupCommandTable[] =
	{
		{ "item",   '4', &ChatHandler::HandleLookupItemCommand,   "Looks up item string x.", NULL, 0, 0, 0 },
		{ "quest",  'z', &ChatHandler::HandleQuestLookupCommand,  "Looks up quest string x.", NULL, 0, 0, 0 },
		{ "creature", 'z', &ChatHandler::HandleLookupCreatureCommand, "Looks up item string x.", NULL, 0, 0, 0 },
		{ "object",  'z', &ChatHandler::HandleLookupObjectCommand,  "Looks up gameobject string x.", NULL, 0, 0 ,0},
		{ "spell",  'z', &ChatHandler::HandleLookupSpellCommand,  "Looks up spell string x.", NULL, 0, 0, 0 },
		{ "skill",  'z', &ChatHandler::HandleLookupSkillCommand,  "Looks up skill string x.", NULL, 0, 0, 0 },
		{ "faction", 'x', &ChatHandler::HandleLookupFactionCommand, "Looks up faction string x.", NULL, 0, 0, 0 },
#ifdef ENABLE_ACHIEVEMENTS
		{ "achievement", 'z', &ChatHandler::HandleLookupAchievementCmd, "Looks up achievement string x.", NULL, 0, 0, 0 },
#endif
		{ NULL,     '0', NULL,                   "",                NULL, 0, 0, 0 }
	};
	dupe_command_table(lookupCommandTable, _lookupCommandTable);

	static ChatCommand adminCommandTable[] =
	{
		{ "castall",        'z', &ChatHandler::HandleCastAllCommand,     "Makes all players online cast spell <x>.",           NULL, 0, 0, 0 },
		{ "dispelall",       'z', &ChatHandler::HandleDispelAllCommand,    "Dispels all negative (or positive w/ 1) auras on all players.", NULL, 0, 0, 0 },
		{ "renameallinvalidchars", 'z', &ChatHandler::HandleRenameAllCharacter,   "Renames all invalid character names",              NULL, 0, 0, 0 },
		{ "masssummon",      'z', &ChatHandler::HandleMassSummonCommand,   "Summons all online players to your location,add the a/A parameter for alliance or h/H for horde.",              NULL, 0, 0, 0 },
		{ "playall",        'z', &ChatHandler::HandleGlobalPlaySoundCommand, "Plays a sound to everyone on the realm.",            NULL, 0, 0, 0 },
		{ "spellinfo",       'z', &ChatHandler::HandleFindSpellInfo,   "DEBUG - lists scripted spellinfo",               NULL, 0, 0, 0 },
		{ "areainfo",       'z', &ChatHandler::HandleAreaInfo,    "DEBUG - lists areainfo ",               NULL, 0, 0, 0 },
		{ "evade",    'w', &ChatHandler::HandleAdminImmuneCommand,     "Toggles complete spell immunity.", NULL, 0, 0, 0 },
		{ "pickside", 'z', &ChatHandler::HandleSetSideFlagCommand, "Set target SIDE. 0-neutral, 1-light, 2-dark", NULL, 0, 0, 0 },
		{ NULL,          '0', NULL,                    "",                               NULL, 0, 0, 0 }
	};
	dupe_command_table(adminCommandTable, _adminCommandTable);

	static ChatCommand kickCommandTable[] =
	{
		{ "player", 'a', &ChatHandler::HandleKillByPlayerCommand, "Disconnects the player with name <s>.",     NULL, 0, 0, 0 },
		{ "account", 'a', &ChatHandler::HandleKillBySessionCommand, "Disconnects the session with account name <s>.", NULL, 0, 0, 0 },
		{ "ip",   'a', &ChatHandler::HandleKillByIPCommand,   "Disconnects the session with the ip <s>.",    NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                   "",                        NULL, 0, 0, 0 }
	};
	dupe_command_table(kickCommandTable, _kickCommandTable);

	static ChatCommand banCommandTable[] =
	{
		{ "ip",    'z', &ChatHandler::HandleIPBanCommand,     "Adds an address to the IP ban table: .ban ip <address> [duration] [reason]\nDuration must be a number optionally followed by a character representing the calendar subdivision to use (h>hours, d>days, w>weeks, m>months, y>years, default minutes)\nLack of duration results in a permanent ban.", NULL, 0, 0, 0 },
		{ "character", 'a', &ChatHandler::HandleBanCharacterCommand, "Bans character: .ban character <char> [duration] [reason]",                                                                                                                     NULL, 0, 0, 0 },
		{ "account",  'a', &ChatHandler::HandleAccountBannedCommand, "Bans account: .ban account <name> [duration] [reason]",                                                                                                                       NULL, 0, 0, 0 },
		{ "all",    'z', &ChatHandler::HandleBanAllCommand,    "Bans account, ip, and character: .ban all <char> [duration] [reason]",                                                                                                                NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                   "",                                                                                                                                                  NULL, 0, 0, 0 }
	};
	dupe_command_table(banCommandTable, _banCommandTable);

	static ChatCommand unbanCommandTable[] =
	{
		{ "ip",    'z', &ChatHandler::HandleIPUnBanCommand,    "Deletes an address from the IP ban table: <address>", NULL, 0, 0, 0 },
		{ "character", 'z', &ChatHandler::HandleUnBanCharacterCommand, "Unbans character x",                 NULL, 0, 0, 0 },
		{ "account",  'z', &ChatHandler::HandleAccountUnbanCommand,  "Unbans account x.",                  NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                   "",                          NULL, 0, 0, 0 }
	};
	dupe_command_table(unbanCommandTable, _unbanCommandTable);

	static ChatCommand instanceCommandTable[] =
	{
		{ "create",  'z', &ChatHandler::HandleCreateInstanceCommand,  "Generically instances a map that requires instancing, mapid x y z",     NULL, 0, 0, 0 },
		{ "reset",  'z', &ChatHandler::HandleResetInstanceCommand,   "Removes instance ID x from target player.",             NULL, 0, 0, 0 },
		{ "resetall", 'i', &ChatHandler::HandleResetAllInstancesCommand, "Removes all instance IDs from target player.",           NULL, 0, 0, 0 },
		{ "shutdown", 'z', &ChatHandler::HandleShutdownInstanceCommand, "Shutdown instance with ID x (default is current instance).",    NULL, 0, 0, 0 },
		//{ "delete",  'z', &ChatHandler::HandleDeleteInstanceCommand,  "Deletes instance with ID x (default is current instance).",     NULL, 0, 0, 0 },
		{ "info",   'z', &ChatHandler::HandleGetInstanceInfoCommand,  "Gets info about instance with ID x (default is current instance).", NULL, 0, 0, 0 },
		{ "exit",   'g', &ChatHandler::HandleExitInstanceCommand,   "Exits current instance, return to entry point.",          NULL, 0, 0, 0 },
		{ NULL,    '0', NULL,                     "",                                 NULL, 0, 0, 0 }
	};
	dupe_command_table(instanceCommandTable, _instanceCommandTable);

	static ChatCommand arenaCommandTable[] =
	{
		{ "createteam",   'z', &ChatHandler::HandleArenaCreateTeamCommand,   "Creates arena team",              NULL, 0, 0, 0 },
		{ "setteamleader",  'z', &ChatHandler::HandleArenaSetTeamLeaderCommand,  "Sets the arena team leader",          NULL, 0, 0, 0 },
		{ "resetallratings", 'z', &ChatHandler::HandleArenaResetAllRatingsCommand, "Resets all arena teams to their default rating", NULL, 0, 0, 0 },
		{ "toggle", 'a', &ChatHandler::HandleEventArenaState, "Toggles the state fo the arena for events. |cFFFF0000REMEMBER TO SET IT BACK TO PUBLIC STATE WHEN DONE!", NULL, 0, 0, 0},
		{ "clear", 'a', &ChatHandler::HandleEventArenaClear, "Clears the arena of all non CoAd+", NULL, 0, 0, 0},
		{ "dispel",  'a', &ChatHandler::HandleArenaDispelAllCommand, "Dispells all buffs in the arena. Only usable if the arena is in event status.",   NULL, 0, 0, 0},
		{ NULL,       '0', NULL,                      "",                       NULL, 0, 0, 0 }
	};
	dupe_command_table(arenaCommandTable, _arenaCommandTable);

	static ChatCommand achievementCommandTable[] =
	{
#ifdef ENABLE_ACHIEVEMENTS
		{ "complete", 'z', &ChatHandler::HandleAchievementCompleteCommand, "Completes the specified achievement.",     NULL, 0, 0, 0 },
		{ "criteria", 'z', &ChatHandler::HandleAchievementCriteriaCommand, "Completes the specified achievement criteria.", NULL, 0, 0, 0 },
		{ "reset",  'z', &ChatHandler::HandleAchievementResetCommand,  "Resets achievement data from the target.",   NULL, 0, 0, 0 },
#endif
		{ NULL,    '0', NULL,                      "",                       NULL, 0, 0, 0 }
	};
	dupe_command_table(achievementCommandTable, _achievementCommandTable);

	static ChatCommand spellsCommandTable[] =
	{
		{ "invisibility", 'a', &ChatHandler::HandleSpellsInvisibleCommand, "Learn Greater Invisibilty", NULL, 0, 0, 0},
		{ "detection",  'a', &ChatHandler::HandleInvisDetectCommand,  "Learn Invisibilty Detection", NULL, 0, 0, 0},
		{ "heatwave",  'a', &ChatHandler::HandleHeatWaveCommand,   "Learn Heat Wave", NULL, 0, 0, 0},
		{ "hitchance",  'a', &ChatHandler::HandleHitChanceCommand,   "Learn +20% Hit Chance", NULL, 0, 0, 0},
		{ "underwater",  'a', &ChatHandler::HandleUnderwaterCommand,   "Learn Passive Underwater Breathing", NULL, 0, 0, 0},
		{ "imppoison",  'a', &ChatHandler::HandleSpellsImpPoisonCommand, "Learn Impailing Poison", NULL, 0, 0, 0},
		{ "chaoscharge", 'a', &ChatHandler::HandleSpellsChaosChargeCommand, "Learn Chaos Charge", NULL, 0, 0, 0},
		{ "chaosflames", 'a', &ChatHandler::HandleSpellsChaosFlamesCommand, "Learn Chaos Flames", NULL, 0, 0, 0},
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(spellsCommandTable, _spellsCommandTable);

	static ChatCommand packoneCommandTable[] = //packone == Utility and eye candy
	{
		{ "hover",   '2', &ChatHandler::HandleHoverCommand,   "Makes you hover", NULL, 0, 0, 0},
		{ "bank",   '2', &ChatHandler::HandleShowBankCommand,  "Open your bank from anywhere.", NULL, 0, 0, 0},
		//{ "guildbank",  '2', &ChatHandler::HandleShowGuildBankCommand,  "Open your GUILD bank from anywhere.", NULL, 0, 0, 0},
		//{ "alltalents",  'o', &ChatHandler::HandleAllTalentsCommand,  "Learn all of your class talents.", NULL, 0, 0, 0},
		{ "selfrename", '2', &ChatHandler::HandleSelfRenameCommand, "Sets your name to change upon next login. Will not ban your current name.", NULL, 0, 0, 0},
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(packoneCommandTable, _packoneCommandTable);

	static ChatCommand packtwoCommandTable[] = //packtwo == Spells
	{
		{ "heatwave",  '3', &ChatHandler::HandleHeatWaveCommand,   "Learn Heat Wave", NULL, 0, 0, 0},
		{ "hitchance",  '3', &ChatHandler::HandleHitChanceCommand,   "Learn +20% Hit Chance", NULL, 0, 0, 0},
		{ "underwater",  '3', &ChatHandler::HandleUnderwaterCommand,   "Learn Passive Underwater Breathing", NULL, 0, 0, 0},
		{ "imppoison",  '3', &ChatHandler::HandleSpellsImpPoisonCommand, "Learn Impailing Poison", NULL, 0, 0, 0},
		{ "chaoscharge", '3', &ChatHandler::HandleSpellsChaosChargeCommand, "Learn Chaos Charge", NULL, 0, 0, 0},
		{ "chaosflames", '3', &ChatHandler::HandleSpellsChaosFlamesCommand, "Learn Chaos Flames", NULL, 0, 0, 0},
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(packtwoCommandTable, _packtwoCommandTable);

	static ChatCommand packthreeCommandTable[] = //packthree == Platinum+ Package
	{
		{ "killcreature", '4', &ChatHandler::HandleCreatureKillCommand, "Instantly kill, or severely damage, your targetted creature.", NULL, 0, 0, 0},
		{ "invisibility", '4', &ChatHandler::HandleToggleInvisCommand, "Toggle a spell activated invisibility.", NULL, 0, 0, 0},
		//{ "additemset",  '4', &ChatHandler::HandleAddItemSetCommand, "Adds (non-custom) item set to inventory.",   NULL, 0, 0, 0 },
		//{ "lookupitem",  '4', &ChatHandler::HandleLookupItemCommand, "Looks up item string x.", NULL, 0, 0, 0 },
		//{ "jail",   '4', &ChatHandler::HandleGoToJailCommand,   "Sends a player to jail and places them in timeout for 5 minutes -- always place a reason!",  NULL, 0, 0, 0},
		//recall find here as well
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(packthreeCommandTable, _packthreeCommandTable);

	static ChatCommand eventCommandTable[] = 
	{
		{ "ring",  'a', &ChatHandler::HandleEventRingCommand, "<NAME><REASON> Awards |cffa335ee|Hitem:70010:0:0:0:0:0:0:0|h[Event Winner Ring]|h|r.", NULL, 0, 0, 0},
		{ "trink",  'a', &ChatHandler::HandleEventTrinkCommand, "<NAME><REASON> Awards |cffa335ee|Hitem:70011:0:0:0:0:0:0:0|h[Event Winner Trinket]|h|r.", NULL, 0, 0, 0},
		{ "neck",  'a', &ChatHandler::HandleEventNeckCommand, "<NAME><REASON> Awards |cffa335ee|Hitem:70012:0:0:0:0:0:0:0|h[Event Winner Necklace]|h|r.", NULL, 0, 0, 0},
		{ "levelup", 'z', &ChatHandler::HandleEventLevelUpCommand, "<Name><levels><reason>",   NULL, 0, 0, 0},
		{ "GMring",  'a', &ChatHandler::HandleGMRingCommand, "Grants your selected GM their appropriate GM ring.",   NULL, 0, 0, 0},
		{ "classsummon", 'a', &ChatHandler::HandleGMMassSummonCommand, "Sends a summon request to all online GMs - use for class purposes", NULL, 0, 0, 0},
		{ "levelsummon", 'a', &ChatHandler::HandleLevelSummonCommand, "Sends a summon request by min/max level", NULL, 0, 0, 0},
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(eventCommandTable, _eventCommandTable);

	static ChatCommand syraCommandTable[] = 
	{
		{ "team",    'o', &ChatHandler::HandleSetPlayerTeam,     "Changes a player's team", NULL, 0, 0, 0 },
		{ "item",    'o', &ChatHandler::HandleSendItemPacket,     "Sends an update packet for specified itemid", NULL, 0, 0, 0 },
		{ "guardians",    'o', &ChatHandler::HandleSetGuardians,     "Sets custom guardians", NULL, 0, 0, 0 },
		{ "gbank",    'o', &ChatHandler::HandleShowGuildBankCommand,     "Open your guild bank from anywhere.", NULL, 0, 0, 0},
		{ "mail",    'o', &ChatHandler::HandleShowMail,        "Open your mailbox from anywhere.", NULL, 0, 0, 0},
		{ "auctioneer",   'o', &ChatHandler::HandleGetAuctioneer,      "Open auctionhouse from anywhere.", NULL, 0, 0, 0},
		{ "forcedperms",   'o', &ChatHandler::HandleGMStringUpdate,            "Corrects a specifc afp DONT USE WITHOUT CHECKING",                                                             NULL,           0, 0, 0 },
		{ "changecolor",   'o', &ChatHandler::HandleChangeChatColor,            "Changes chat color",                                                             NULL,           0, 0, 0 },
		{ "bonus",   'o', &ChatHandler::HandleBonusFlag,            "Toggles the bonus flag",                                                             NULL,           0, 0, 0 },
		{ "pvp",   'o', &ChatHandler::HandleUWPvPFlag,            "Toggles the PvP flag",                                                             NULL,           0, 0, 0 },
		{ "talent",   'o', &ChatHandler::HandleTalentTest,            "Test talent learning",                                                             NULL,           0, 0, 0 },
		{ "flagall",   'o', &ChatHandler::HandleFlagAll,            "Flags all in my in range set",                                                             NULL,           0, 0, 0 },
		{ "disablespellscale", 'o', &ChatHandler::HandleDisableSpellScale, ".admin disablespellscale <id> <name> You cannot use spaces when entering the name", NULL, 0, 0, 0 },
		{ "enablespellscale", 'o', &ChatHandler::HandleEnableSpellScale, ".admin enablespellscale <id> Re-Enables the spell damage scaling on a spell.", NULL, 0, 0, 0 },
		{ "reloadspellscale", 'o', &ChatHandler::HandleReloadSpellScale, "Reload non scaling spell cache.", NULL, 0, 0, 0 },
		{ "ToggleSyra", 'o', &ChatHandler::HandleToggleSyra, "Toggles instant death when hit.", NULL, 0, 0, 0 },
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(syraCommandTable, _syraCommandTable);

	static ChatCommand odnetninCommandTable[] = 
	{
		{ "slapneechee", 'y', &ChatHandler::HandleSlapNeechee, "Cast slap, only usable on neechee.", NULL, 0, 0, 0 },
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(odnetninCommandTable, _odnetninCommandTable);

	static ChatCommand devCommandTable[] = 
	{
		{ "reloaditems", 'd', &ChatHandler::HandleReloadItems, "Reloads the items table.", NULL, 0, 0, 0 },
		{ NULL,    0, NULL,       "",           NULL, 0, 0, 0 },
	};
	dupe_command_table(devCommandTable, _devCommandTable);

	static ChatCommand commandTable[] =
	{
		{ "checkexpire",   't', &ChatHandler::HandleTempExpireReport,      "checks expiration date and time of your powers",              NULL, 0,         0,          0 },
		{ "commands",    '0', &ChatHandler::HandleCommandsCommand,           "Shows commands",                                                             NULL,           0, 0, 0 },
		{ "help",      '0', &ChatHandler::HandleHelpCommand,             "Shows help for command",                                                         NULL,           0, 0, 0 },
		{ "calcdist",    'z', &ChatHandler::HandleSimpleDistanceCommand,        "Display the distance between your current position and the specified point x y z",                              NULL,           0, 0, 0 },
		{ "announce",    'a', &ChatHandler::Handle_UW_AnnounceCommand,           "Sends a normal chat message broadcast to all players.",                                                            NULL,           0, 0, 0 },
		{ "wannounce",    'x', &ChatHandler::Handle_UW_WAnnounceCommand,           "Sends a widescreen raid style announcement to all players.",                                                       NULL,           0, 0, 0 },
		{ "appear",     'j', &ChatHandler::HandleAppearCommand,            "Teleports to x's position.",                                                       NULL,           0, 0, 0 },
		{ "summon",     'a', &ChatHandler::HandleSummonCommand,            "Summons x to your position.",                                                       NULL,           0, 0, 0 },
		{ "kill",      'z', &ChatHandler::HandleKillCommand,             ".kill - Kills selected unit.",                                                      NULL,           0, 0, 0 },
		{ "killplr",     'z', &ChatHandler::HandleKillByPlrCommand,           ".killplr <name> - Kills specified player",                                                NULL,           0, 0, 0 },
		{ "revive",     'j', &ChatHandler::HandleReviveCommand,            "Revives you.",                                                              NULL,           0, 0, 0 },
		{ "reviveplr",    'x', &ChatHandler::HandleReviveStringcommand,         "Revives player specified.",                                                        NULL,           0, 0, 0 },
		{ "morph",    'j', &ChatHandler::HandleMorphCommand,             "Morphs into model X.",                               NULL,           0, 0, 0 },
		{ "demorph",     'j', &ChatHandler::HandleDeMorphCommand,            "Demorphs from morphed model.",                                                      NULL,           0, 0, 0 },
		{ "mount",      'j', &ChatHandler::HandleMountCommand,             "Mounts into modelid x.",                                                         NULL,           0, 0, 0 },
		{ "dismount",    'j', &ChatHandler::HandleDismountCommand,           "Dismounts.",                                                               NULL,           0, 0, 0 },
		{ "gps",       'i', &ChatHandler::HandleGPSCommand,              "Shows Position",                                                             NULL,           0, 0, 0 },
		{ "worldport",    'i', &ChatHandler::HandleWorldPortCommand,           "Teleports you to a location with mapid x y z",                                                                    NULL,           0, 0, 0 },
		{ "start",      '0', &ChatHandler::HandleStartCommand,             "Teleports you to a starting location",                                                  NULL,           0, 0, 0 },
		{ "invincible",   'x', &ChatHandler::HandleInvincibleCommand,          ".invincible - Toggles INVINCIBILITY (mobs won't attack you)",                                       NULL,           0, 0, 0 },
		{ "invisible",    'a', &ChatHandler::HandleInvisibleCommand,           ".invisible - Toggles INVINCIBILITY and INVISIBILITY (mobs won't attack you and nobody can see you, but they can see your chat messages)", NULL,           0, 0, 0 },
		{ "playerinfo",   'j', &ChatHandler::HandlePlayerInfo,              ".playerinfo - Displays information about the selected character (account...)",                              NULL,           0, 0, 0 },
		{ "levelup",     'g', &ChatHandler::HandleLevelUpCommand,            "Levelup x lvls",                                                             NULL,           0, 0, 0 },
		{ "modify",     'g', NULL,                           "",                                                                    modifyCommandTable,    0, 0, 0 },
		{ "syra",     'o', NULL,                           "",                                                                    syraCommandTable,   0, 0, 0 },
		{ "odnetnin",     'y', NULL,                           "",                                                                    odnetninCommandTable,   0, 0, 0 },
		{ "waypoint",    'z', NULL,                           "",                                                                    waypointCommandTable,   0, 0, 0 },
		{ "debug",      'x', NULL,                           "",                                                                    debugCommandTable,    0, 0, 0 },
		{ "gm",       'g', NULL,                           "",                                                                    gmCommandTable,      0, 0, 0 },
		{ "gmTicket",    'x', NULL,                           "",                                                                    GMTicketCommandTable,   0, 0, 0 },
		{ "gobject",     'z', NULL,                           "",                                                                    GameObjectCommandTable,  0, 0, 0 },
		{ "battleground",  'z', NULL,                           "",                                                                    BattlegroundCommandTable, 0, 0, 0 },
		{ "npc",       'j', NULL,                           "",                                                                    NPCCommandTable,     0, 0, 0 },
		{ "vehicle",     'z', NULL,                           "",                                                                    NPCCommandTable,     0, 0, 0 },
		{ "cheat",      'g', NULL,                           "",                                                                    CheatCommandTable,    0, 0, 0 },
		{ "account",     'a', NULL,                           "",                                                                    accountCommandTable,   0, 0, 0 },
		{ "honor",      'a', NULL,                           "",                                                                    honorCommandTable,    0, 0, 0 },
		{ "quest",      '1', NULL,                           "",                                                                    questCommandTable,    0, 0, 0 },
		{ "pet",       'z', NULL,                           "",                                                                    petCommandTable,     0, 0, 0 },
		{ "recall",     'g', NULL,                           "",                                                                    recallCommandTable,    0, 0, 0 },
		{ "guild",      'x', NULL,                           "",                                                                    GuildCommandTable,    0, 0, 0 },
		{ "server",     'a', NULL,                           "",                                                                    serverCommandTable,    0, 0, 0 },
		{ "character",    'g', NULL,                           "",                                                                    characterCommandTable,  0, 0, 0 },
		{ "lookup",     'g', NULL,                           "",                                                                    lookupCommandTable,    0, 0, 0 },
		{ "admin",      'z', NULL,                           "",                                                                    adminCommandTable,    0, 0, 0 },
		{ "kick",      'a', NULL,                           "",                                                                    kickCommandTable,     0, 0, 0 },
		{ "ban",       'a', NULL,                           "",                                                                    banCommandTable,     0, 0, 0 },
		{ "unban",      'z', NULL,                           "",                                                                    unbanCommandTable,    0, 0, 0 },
		{ "instance",    'g', NULL,                           "",                                                                    instanceCommandTable,   0, 0, 0 },
		{ "event",   'a', NULL,          "",  eventCommandTable, 0, 0, 0},
		{ "packthree",  '4', NULL,          "",  packthreeCommandTable, 0, 0, 0},
		{ "packtwo",  '3', NULL,          "",  packtwoCommandTable, 0, 0, 0},
		{ "packone",  '2', NULL,          "",  packoneCommandTable, 0, 0, 0},
		{ "spells",   'a', NULL,          "",     spellsCommandTable, 0, 0, 0},
		{ "dev",   'd', NULL,          "",     devCommandTable, 0, 0, 0},
		{ "arena",      'a', NULL,                           "",                                                                    arenaCommandTable,    0, 0, 0 },
		{ "kickplayer",   'm', &ChatHandler::Handle_UW_KickCommand,             "Kicks player from server",                                                        NULL,           0, 0, 0 },
		{ "gmannounce",   'a', &ChatHandler::Handle_UW_GMAnnounceCommand,          "Sends Msg to all online GMs",                                                       NULL,           0, 0, 0 },
		{ "clearcooldowns", 'j', &ChatHandler::HandleClearCooldownsCommand,        "Clears all cooldowns for your class.",                                                  NULL,           0, 0, 0 },
		{ "removeauras",   'x', &ChatHandler::HandleRemoveAurasCommand,          "Removes all auras from target",                                                      NULL,           0, 0, 0 },
		{ "paralyze",    'a', &ChatHandler::HandleParalyzeCommand,           "Roots/Paralyzes the target.",                                                       NULL,           0, 0, 0 },
		{ "unparalyze",   'a', &ChatHandler::HandleUnParalyzeCommand,          "Unroots/Unparalyzes the target.",                                                     NULL,           0, 0, 0 },
		{ "gotrig",     'a', &ChatHandler::HandleTriggerCommand,            "Warps to areatrigger <id>",                                                        NULL,           0, 0, 0 },
		{ "modperiod",    'z', &ChatHandler::HandleModPeriodCommand,           "Changes period of current transporter.",                                                 NULL,           0, 0, 0 },
		{ "logcomment",   'g', &ChatHandler::HandleGmLogCommentCommand,         "Adds a comment to the GM log for the admins to read.",                                          NULL,           0, 0, 0 },
		{ "removesickness", 'a', &ChatHandler::HandleRemoveRessurectionSickessAuraCommand, "Removes ressurrection sickness from the target",                                             NULL,           0, 0, 0 },
		{ "fixscale",    'z', &ChatHandler::HandleFixScaleCommand,           "",                                                                    NULL,           0, 0, 0 },
		{ "fixfaction",   'z', &ChatHandler::HandleFixFactionCommand,           "",                                                                    NULL,           0, 0, 0 },
		{ "hide",   'a', &ChatHandler::HandleHideCommand,      "Toggles your hide status from /who", NULL, 0, 0, 0 },
		{ "unstuck",   '0', &ChatHandler::HandleUnstuckCommand,      "", NULL, 0, 0, 0 },
		{ "loginpassword",  '0', &ChatHandler::HandleAccountPasswordCommand,    "SYNTAX: .loginpassword NEW_PASSWORD NEW_PASSWOD --- You must enter your desired new password twice to confirm a password change.", NULL, 0, 0, 0 },
		{ "mute",    'm', &ChatHandler::HandleAccountTempMuteCommand,    "<name> <reason>", NULL, 0, 0, 0 },
		{ "normalize",   'a', &ChatHandler::HandleNormalizeCommand,      "Returns your target to near-login status.", NULL, 0, 0, 0 },
		{ "itemcount",   'a', &ChatHandler::HandleItemCountCommand,      "Checks item count by entry id in a players inventory", NULL, 0, 0, 0 },
		{ "hasspell",   'a', &ChatHandler::HandleHasSpell,        ".hasspell <entry>", NULL, 0, 0, 0 },
		{ "getIDs",    'j', &ChatHandler::HandleGetModInfo,        "Displays morph and mount IDs for your current target", NULL, 0, 0, 0 },
		{ "getlinkid",    '0', &ChatHandler::HandleGetLinkId,        "Displays the id of the passed link. .getlinkid <link>", NULL, 0, 0, 0 },
		{ "adminDND",   'w', &ChatHandler::HandleAdminDND,        "Toggles complete whisperblock on/off", NULL, 0, 0, 0 },
		{ "bank",    'a', &ChatHandler::HandleShowBankCommand,      "Open your bank from anywhere.", NULL, 0, 0, 0},
		//{ "jail",    '+', &ChatHandler::HandleGoToJailCommand,      "Sends a player to jail and places them in timeout for 5 minutes -- always place a reason!",  NULL, 0, 0, 0},
		{ "timeout",   'a', &ChatHandler::HandleTimeOutCommand,      "Incapacitates your target for 5 minutes",  NULL, 0, 0, 0},
		{ "freeze",    'a', &ChatHandler::HandleFreezeCommand,      "Incapacitates your current player target - all uses are logged.",  NULL, 0, 0, 0},
		{ "myfaction",   'i', &ChatHandler::HandleMyFactionCommand,      "SYNTAX: .myfaction #\nNOTES: Change your faction. 1 = Alliance; 2 = Horde",  NULL, 0, 0, 0},
		{ "chatcolor",   'a', &ChatHandler::HandleColorChat,       "SYNTAX: None.\nNOTES: Toggles ranked chat color in chat messages on/off.",  NULL, 0, 0, 0},
		{ "hover",    'a', &ChatHandler::HandleHoverCommand,        "SYNTAX: None.\nNOTES: Toggles hover mode on/off. SELF-ONLY",  NULL, 0, 0, 0},
		//{ "gotocreature",  'g', &ChatHandler::HandleGoCreatureCommand,     "fix",  NULL, 0, 0, 0},
		//{ "warp",    'g', &ChatHandler::HandleWarpToCommand,      "warp to target, creature only fr donor",  NULL, 0, 0, 0},
		//{ "debinvis",   'o', &ChatHandler::HandleUBERInvis,        "SYNTAX: None.\nNOTES: Toggles hover mode on/off. SELF-ONLY",  NULL, 0, 0, 0},
		{ "addtrainerspell", 'z', &ChatHandler::HandleAddTrainerSpellCommand,        "",                                                                    NULL,           0, 0, 0 },
		{ "achieve",     'z', NULL,                           "",                                                                    achievementCommandTable, 0, 0, 0 },
		{ NULL,       '0', NULL,                           "",                                                                    NULL,           0, 0, 0 }
	};
	dupe_command_table(commandTable, _commandTable);

	/* set the correct pointers */
	ChatCommand * p = &_commandTable[0];
	while(p->Name != 0)
	{
		if(p->ChildCommands != 0)
		{
			// Set the correct pointer.
			ChatCommand * np = GetSubCommandTable(p->Name);
			Arcemu::Util::ARCEMU_ASSERT(   np != NULL );
			p->ChildCommands = np;
		}
		++p;
	}
}

ChatHandler::ChatHandler()
{
	new CommandTableStorage;
	CommandTableStorage::getSingleton().Init();
	SkillNameManager = new SkillNameMgr;
}

ChatHandler::~ChatHandler()
{
	CommandTableStorage::getSingleton().Dealloc();
	delete CommandTableStorage::getSingletonPtr();
	delete SkillNameManager;
}

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
	for(;;)
	{
		if( !*s2 )
			return true;
		else if( !*s1 )
			return false;
		else if( tolower( *s1 ) != tolower( *s2 ) )
			return false;
		s1++; s2++;
	}
}

void ChatHandler::SendMultilineMessage(WorldSession *m_session, const char *str)
{
	char * start = (char*)str, *end;
	for(;;)
	{
		end = strchr(start, '\n');
		if(!end)
			break;

		*end = '\0';
		SystemMessage(m_session, start);
		start = end + 1;
	}
	if(*start != '\0')
		SystemMessage(m_session, start);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text, WorldSession *m_session)
{
	std::string cmd = "";

	// get command
	while (*text != ' ' && *text != '\0')
	{
		cmd += *text;
		text++;
	}

	while (*text == ' ') text++; // skip whitespace

	if(!cmd.length())
		return false;

	for(uint32 i = 0; table[i].Name != NULL; i++)
	{
		if(!hasStringAbbr(table[i].Name, cmd.c_str()))
			continue;

		if(table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
			continue;

		if(table[i].ChildCommands != NULL)
		{
			if(!ExecuteCommandInTable(table[i].ChildCommands, text, m_session))
			{
				if(table[i].Help != "")
					SendMultilineMessage(m_session, table[i].Help.c_str());
				else
				{
					GreenSystemMessage(m_session, "Available Subcommands:");
					for(uint32 k= 0; table[i].ChildCommands[k].Name;k++)
					{
						if(table[i].ChildCommands[k].CommandGroup == '0' || (table[i].ChildCommands[k].CommandGroup != '0' && m_session->CanUseCommand(table[i].ChildCommands[k].CommandGroup)))
							BlueSystemMessage(m_session, " %s - %s", table[i].ChildCommands[k].Name, table[i].ChildCommands[k].Help.size() ? table[i].ChildCommands[k].Help.c_str() : "No Help Available");
					}
				}
			}

			return true;
		}

		// Check for field-based commands
		if(table[i].Handler == NULL && (table[i].MaxValueField || table[i].NormalValueField))
		{
			bool result = false;
			if(strlen(text) == 0)
			{
				RedSystemMessage(m_session, "No values specified.");
			}
			if(table[i].ValueType == 2)
				result = CmdSetFloatField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
			else
				result = CmdSetValueField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
			if(!result)
				RedSystemMessage(m_session, "Must be in the form of (command) <value>, or, (command) <value> <maxvalue>");
		}
		else
		{
			if(!(this->*(table[i].Handler))(text, m_session))
			{
				if(table[i].Help != "")
					SendMultilineMessage(m_session, table[i].Help.c_str());
				else
				{
					RedSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", table[i].Name);
				}
			}
		}

		return true;
	}
	return false;
}

int ChatHandler::ParseCommands(const char* text, WorldSession *session)
{
	if (!session)
		return 0;

	if(!*text)
		return 0;

	if(session->GetPermissionCount() == 0 && sWorld.m_reqGmForCommands)
		return 0;

	if(text[0] != '!' && text[0] != '.') // let's not confuse users
		return 0;

	/* skip '..' :P that pisses me off */
	if(text[1] == '.')
		return 0;

	text++;

	Player * plr = session->GetPlayer();
	if(!plr)return 0;

	if(session->m_gmData->rank < RANK_COADMIN)
	{

		if(session->m_gmData->suspended)
		{
			if(session->m_gmData->suspended >= (uint32)UNIXTIME )
			{
				string timeleft = ConvertTimeStampToDataTime(session->m_gmData->suspended);
				SystemMessage(session, "Your account is currently on GM suspension. Visit our 'GM Suspensions' area of the UNDERWORLD forums for more information.");
				SystemMessage(session, "SUSPENSION ENDS: %s%s%s.", MSG_COLOR_RED, timeleft.c_str(), MSG_COLOR_YELLOW);
				SystemMessage(session, "Please note all times and dates are PST or GMT -7 or 'Server Time'."); 
				return 1;
			}
		}

		if(plr->bFROZEN)
		{
			session->SendNotification("Cannot enter commands while GM incapacitated.");
			return 2;
		}

		if(plr->InGroup() && plr->GetGroup()->GroupHasGM && plr->GetGroup()->GroupHasPlayer)
		{
			RedSystemMessage(session,"ERROR: Your group has been flagged as mixed with GM/Player. In order to use commands you must either leave your group or reform a group with only GMs.");
			plr->HonestGM(false);
			return 2; //avoid spam
		}
		
		switch(session->GetPlayer()->GetMapId())
		{
			case 389:
				{
					if( text[0] != 'r' && text[0] != 'g' ) //Let them use recall commands in the mall
					{
						RedSystemMessage(session, "Apologies, but Donators are not permitted to use commands in the UNDERWORLD Mall.");
						session->GetPlayer()->Neutralize(false, false);
						return 2;
					}
				}
			case 571:
				{
					float x = plr->GetPositionX();
					float y = plr->GetPositionY();
					if (x <= 3510 && x >= 3460)
						if (y <=6950 && y >=6880)
						{
							RedSystemMessage(session, "GM Commands are not allowed in class.");
							session->GetPlayer()->Neutralize(false, false);
							return 2;						
						}
				}
				break;
			case 559:
				{
					if (sWorld.arenaEventInProgress)
					{
						RedSystemMessage(session, "This is the arena! No cheating!");
						return 2;					
					}
				}
				break;
			case 44:
				{
					WorldPacket * data = FillMessageData(CHAT_MSG_EMOTE, LANG_UNIVERSAL, "muttered something unintelligible.", session->GetPlayer()->GetGUID(), 0);
					session->GetPlayer()->SendMessageToSet(data, true);
					delete data;

					RedSystemMessage(session, "GM Commands are disabled here. Please direct your attention to the Staff member who summoned you.");
					session->GetPlayer()->Neutralize();
					return 2;
				}
		}
			
		//delay on donors
		if(plr->gm_command_delay > (uint32)UNIXTIME) 
			return 2; //Return>0 so GMs aren't spamming chat channels trying to use commands
		else
		{
			plr->gm_command_delay = (uint32)UNIXTIME + (4 - session->m_gmData->rank);
		}

			//tempcheck here
		if(session->m_gmData->temp && ((uint32)UNIXTIME > session->m_gmData->t_checkDelay) )
			session->UpdateTempGM(false);
	
	}
	


	
	//GMLog(session, text, "LOGGED IN ::ParseCommands()");

	if(!ExecuteCommandInTable(CommandTableStorage::getSingleton().Get(), text, session))
	{
		SystemMessage(session, "There is no such command, or you do not have access to it.");
	}

	return 1;
}

WorldPacket * ChatHandler::FillMessageData( uint32 type, uint32 language, const char *message,uint64 guid , uint8 flag) const
{
	//Packet  structure
	//uint8   type;
	//uint32 language;
	//uint64 guid;
	//uint64 guid;
	//uint32 len_of_text;
	//char   text[];   // not sure ? i think is null terminated .. not null terminated
	//uint8   afk_state;
	Arcemu::Util::ARCEMU_ASSERT(   type != CHAT_MSG_CHANNEL);
	//channels are handled in channel handler and so on
	uint32 messageLength = (uint32)strlen((char*)message) + 1;

	WorldPacket *data = new WorldPacket(SMSG_MESSAGECHAT, messageLength + 30);

	*data << (uint8)type;
	*data << language;

	*data << guid;
	*data << uint32(0);

	*data << guid;

	*data << messageLength;
	*data << message;

	*data << uint8(flag);
	return data;
}

WorldPacket* ChatHandler::FillSystemMessageData(const char *message) const
{
	uint32 messageLength = (uint32)strlen((char*)message) + 1;

	WorldPacket * data = new WorldPacket(SMSG_MESSAGECHAT, 30 + messageLength);
	*data << (uint8)CHAT_MSG_SYSTEM;
	*data << (uint32)LANG_UNIVERSAL;

	// Who cares about guid when there's no nickname displayed heh ?
	*data << (uint64)0;
	*data << (uint32)0;
	*data << (uint64)0;

	*data << messageLength;
	*data << message;

	*data << uint8(0);

	return data;
}

Player * ChatHandler::getSelectedChar(WorldSession *m_session, bool showerror)
{
	uint64 guid;
	Player *chr;

	if (m_session == NULL || m_session->GetPlayer() == NULL) return NULL;

	guid = m_session->GetPlayer()->GetSelection();

	if (guid == 0)
	{
		if(showerror)
			GreenSystemMessage(m_session, "Auto-targeting self.");
		chr = m_session->GetPlayer(); // autoselect
	}
	else
		chr = m_session->GetPlayer()->GetMapMgr()->GetPlayer((uint32)guid);

	if(chr == NULL)
	{
		if(showerror)
			RedSystemMessage(m_session, "This command requires that you select a player.");
		return NULL;
	}

	return chr;
}

Creature * ChatHandler::getSelectedCreature(WorldSession *m_session, bool showerror)
{
	uint64 guid;
	Creature *creature = NULL;

	if (m_session == NULL || m_session->GetPlayer() == NULL) return NULL;

	guid = m_session->GetPlayer()->GetSelection();
	if(GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_PET)
		creature = m_session->GetPlayer()->GetMapMgr()->GetPet( GET_LOWGUID_PART(guid) );
	else if(GET_TYPE_FROM_GUID(guid) == HIGHGUID_TYPE_UNIT)
		creature = m_session->GetPlayer()->GetMapMgr()->GetCreature( GET_LOWGUID_PART(guid) );

	if(creature != NULL)
		return creature;
	else
	{
		if(showerror)
			RedSystemMessage(m_session, "This command requires that you select a creature.");
		return NULL;
	}
}

void ChatHandler::SystemMessage(WorldSession *m_session, const char* message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024, message,ap);
	WorldPacket * data = FillSystemMessageData(msg1);
	if(m_session != NULL)
		m_session->SendPacket(data);
	delete data;
}

void ChatHandler::ColorSystemMessage(WorldSession *m_session, const char* colorcode, const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024, message,ap);
	char msg[1024];
	snprintf(msg, 1024, "%s%s|r", colorcode, msg1);
	WorldPacket * data = FillSystemMessageData(msg);
	if(m_session != NULL)
		m_session->SendPacket(data);
	delete data;
}

void ChatHandler::RedSystemMessage(WorldSession *m_session, const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	char msg[1024];
	snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTRED/*MSG_COLOR_RED*/, msg1);
	WorldPacket * data = FillSystemMessageData(msg);
	if(m_session != NULL)
		m_session->SendPacket(data);
	delete data;
}

void ChatHandler::GreenSystemMessage(WorldSession *m_session, const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024, message,ap);
	char msg[1024];
	snprintf(msg, 1024, "%s%s|r", MSG_COLOR_GREEN, msg1);
	WorldPacket * data = FillSystemMessageData(msg);
	if(m_session != NULL)
		m_session->SendPacket(data);
	delete data;
}

void ChatHandler::BlueSystemMessage(WorldSession *m_session, const char *message, ...)
{
	if( !message ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024, message,ap);
	char msg[1024];
	snprintf(msg, 1024,"%s%s|r", MSG_COLOR_LIGHTBLUE, msg1);
	WorldPacket * data = FillSystemMessageData(msg);
	if(m_session != NULL)
		m_session->SendPacket(data);
	delete data;
}

void ChatHandler::RedSystemMessageToPlr(Player* plr, const char *message, ...)
{
	if( !message || !plr || !plr->GetSession() ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	RedSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::GreenSystemMessageToPlr(Player* plr, const char *message, ...)
{
	if( !message || !plr || !plr->GetSession() ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	GreenSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::BlueSystemMessageToPlr(Player* plr, const char *message, ...)
{
	if( !message || !plr || !plr->GetSession() ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	BlueSystemMessage(plr->GetSession(), (const char*)msg1);
}

void ChatHandler::SystemMessageToPlr(Player *plr, const char* message, ...)
{
	if( !message || !plr || !plr->GetSession() ) return;
	va_list ap;
	va_start(ap, message);
	char msg1[1024];
	vsnprintf(msg1,1024,message,ap);
	SystemMessage(plr->GetSession(), msg1);
}

bool ChatHandler::CmdSetValueField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char *args)
{
	char* pvalue;
	uint32 mv, av;

	if(!args || !m_session) return false;

	pvalue = strtok((char*)args, " ");
	if (!pvalue)
		return false;
	else
		av = atol(pvalue);

	if(fieldmax)
	{
		char* pvaluemax = strtok(NULL, " ");
		if (!pvaluemax)
			return false;
		else
			mv = atol(pvaluemax);
	}
	else
	{
		mv = 0;
	}

	if (av <= 0 && mv > 0)
	{
		RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
		return true;
	}
	if(fieldmax)
	{
		if(mv < av || mv <= 0)
		{
			RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
			return true;
		}
	}

	Player *plr = getSelectedChar(m_session, false);
	if(plr)
	{
		if(fieldmax)
		{
			BlueSystemMessage(m_session, "You set the %s of %s to %d/%d.", fieldname, plr->GetName(), av, mv);
			GreenSystemMessageToPlr(plr, "%s set your %s to %d/%d.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
		}
		else
		{
			BlueSystemMessage(m_session, "You set the %s of %s to %d.", fieldname, plr->GetName(), av);
			GreenSystemMessageToPlr(plr, "%s set your %s to %d.", m_session->GetPlayer()->GetName(), fieldname, av);
		}

		if(field == UNIT_FIELD_STAT1) av /= 2;
		if(field == UNIT_FIELD_BASE_HEALTH)
		{
			plr->SetHealth( av);
		}

		plr->SetUInt32Value(field, av);

		if(fieldmax) {
			plr->SetUInt32Value(fieldmax, mv);
		}
	}
	else
	{
		Creature *cr = getSelectedCreature(m_session, false);
		if(cr)
		{
			if(!(field < UNIT_END && fieldmax < UNIT_END)) return false;
			std::string creaturename = "Unknown Being";
			if(cr->GetCreatureInfo())
				creaturename = cr->GetCreatureInfo()->Name;
			if(fieldmax)
				BlueSystemMessage(m_session, "Setting %s of %s to %d/%d.", fieldname, creaturename.c_str(), av, mv);
			else
				BlueSystemMessage(m_session, "Setting %s of %s to %d.", fieldname, creaturename.c_str(), av);
			if(field == UNIT_FIELD_STAT1) av /= 2;
			if(field == UNIT_FIELD_BASE_HEALTH)
				cr->SetHealth( av);

			switch(field)
			{
			case UNIT_FIELD_FACTIONTEMPLATE:
				{
					if(cr->m_spawn)
						WorldDatabase.Execute("UPDATE creature_spawns SET faction = %u WHERE entry = %u", av, cr->m_spawn->entry);
				}break;
			case UNIT_NPC_FLAGS:
				{
					if(cr->GetProto())
						WorldDatabase.Execute("UPDATE creature_proto SET npcflags = %u WHERE entry = %u", av, cr->GetProto()->Id);
				}break;
			}

			cr->SetUInt32Value(field, av);

			if(fieldmax) {
				cr->SetUInt32Value(fieldmax, mv);
			}
			// reset faction
			if(field == UNIT_FIELD_FACTIONTEMPLATE)
				cr->_setFaction();

			cr->SaveToDB();
		}
		else
		{
			RedSystemMessage(m_session, "Invalid Selection.");
		}
	}
	return true;
}

bool ChatHandler::CmdSetFloatField(WorldSession *m_session, uint32 field, uint32 fieldmax, const char *fieldname, const char *args)
{
	char* pvalue;
	float mv, av;

	if(!args || !m_session) return false;

	pvalue = strtok((char*)args, " ");
	if (!pvalue)
		return false;
	else
		av = (float)atof(pvalue);

	if(fieldmax)
	{
		char* pvaluemax = strtok(NULL, " ");
		if (!pvaluemax)
			return false;
		else
			mv = (float)atof(pvaluemax);
	}
	else
	{
		mv = 0;
	}

	if (av <= 0)
	{
		RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
		return true;
	}
	if(fieldmax)
	{
		if(mv < av || mv <= 0)
		{
			RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
			return true;
		}
	}

	Player *plr = getSelectedChar(m_session, false);
	if(plr)
	{
		if(fieldmax)
		{
			BlueSystemMessage(m_session, "You set the %s of %s to %.1f/%.1f.", fieldname, plr->GetName(), av, mv);
			GreenSystemMessageToPlr(plr, "%s set your %s to %.1f/%.1f.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
		}
		else
		{
			BlueSystemMessage(m_session, "You set the %s of %s to %.1f.", fieldname, plr->GetName(), av);
			GreenSystemMessageToPlr(plr, "%s set your %s to %.1f.", m_session->GetPlayer()->GetName(), fieldname, av);
		}
		plr->SetFloatValue(field, av);
		if(fieldmax) plr->SetFloatValue(fieldmax, mv);
	}
	else
	{
		Creature *cr = getSelectedCreature(m_session, false);
		if(cr)
		{
			if(!(field < UNIT_END && fieldmax < UNIT_END)) return false;
			std::string creaturename = "Unknown Being";
			if(cr->GetCreatureInfo())
				creaturename = cr->GetCreatureInfo()->Name;
			if(fieldmax)
				BlueSystemMessage(m_session, "Setting %s of %s to %.1f/%.1f.", fieldname, creaturename.c_str(), av, mv);
			else
				BlueSystemMessage(m_session, "Setting %s of %s to %.1f.", fieldname, creaturename.c_str(), av);
			cr->SetFloatValue(field, av);
			if(fieldmax) {
				cr->SetFloatValue(fieldmax, mv);
			}
			//cr->SaveToDB();
		}
		else
		{
			RedSystemMessage(m_session, "Invalid Selection.");
		}
	}
	return true;
}

bool ChatHandler::HandleGetPosCommand(const char* args, WorldSession *m_session)
{
	if(!args || !m_session) return false;

	/*if(m_session->GetPlayer()->GetSelection() == 0) return false;
	Creature *creature = objmgr.GetCreature(m_session->GetPlayer()->GetSelection());

	if(!creature) return false;
	BlueSystemMessage(m_session, "Creature Position: \nX: %f\nY: %f\nZ: %f\n", creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ());
	return true;*/

	uint32 spell = atol(args);
	SpellEntry *se = dbcSpell.LookupEntryForced(spell);
	if(se)
		BlueSystemMessage(m_session, "SpellIcon for %d is %d", se->Id, se->field114);
	return true;
}
