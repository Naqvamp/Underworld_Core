/* SpellFixes.cpp  -Syra
 *	This is a collection of spell fixes that need scripting. 
 * All other simple spell alterations are done in arcemu-world>SpellFixes.cpp World::ApplyNormalFixes() line @8263
 */

#include "StdAfx.h"

//Shadowstep Triggered Spell 36563 Affect[0] (Port Units)
bool ShadowstepPort(uint32 i, Spell * pSpell)
{
	if ( pSpell == NULL || pSpell->p_caster == NULL )
		return true;
	
	Player * plr = pSpell->p_caster;
	if( plr == NULL )
		return true;

	uint64 guid = plr->GetSelection();
	Unit * tar;

	if (guid == 0) //no target?! How did we cast the spell?
		return true;
	else
		tar = plr->GetMapMgr()->GetUnit((uint32)guid);

	if(tar == NULL) //...wtf
		return true;

	if (tar->GetInstanceID() == plr->GetInstanceID())
		plr->SafeTeleport(tar->GetMapId(), tar->GetInstanceID(), tar->GetPosition());
	
	return true;
}

void SetupSpellFixes(ScriptMgr * mgr)
{
	mgr->register_dummy_spell(36563, &ShadowstepPort);
}