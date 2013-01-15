#include "StdAfx.h"
#include "Setup.h"
#include <ScriptSetup.h>

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{
return SCRIPT_TYPE_MISC;
}

/*extern "C" SCRIPT_DECL uint32 _exp_get_version()
{
return MAKE_SCRIPT_VERSION(SCRIPTLIB_VERSION_MAJOR, SCRIPTLIB_VERSION_MINOR);
}*/

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
	//GodQuest
#ifdef _ENABLE_GOD_QUEST	
	SetupSyraDefense(mgr);
#endif
	Setup12Labours(mgr);

	//All
	SetupDonator(mgr);
	SetupLevelUp(mgr);
	SetupSpellFixes(mgr);
	SetupVoterExchange(mgr);
	SetupTelebooks(mgr);
	SetupAnnouncers(mgr);

	//AlphaX
	if (sWorld.realmID & REALM_ALPHA_X)
	{
		//SetupStarterRune(mgr);
	}
	//DOH
	if (sWorld.realmID & REALM_DOH)
	{
		SetupPvPToken(mgr);
	}
}

#ifdef WIN32

BOOL APIENTRY DllMain( HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
return TRUE;
}

#endif

