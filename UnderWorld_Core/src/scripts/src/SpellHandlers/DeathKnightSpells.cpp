/*
 * ArcScript Scripts for Arcemu MMORPG Server
 * Copyright (C) 2008-2009 Arcemu Team
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "Setup.h"

#define BLOOD_PLAGUE 55078
#define FROST_FEVER 55095

bool Pestilence(uint32 i, Spell* pSpell)
{
	if(i == 1) // Grr....
	{
		if(!pSpell->u_caster || !pSpell->u_caster->GetTargetGUID() || !pSpell->u_caster->IsInWorld())
			return true;

		Unit* u_caster = pSpell->u_caster;
		Unit* Main = u_caster->GetMapMgr()->GetUnit(u_caster->GetTargetGUID());
		bool blood = Main->HasAura(BLOOD_PLAGUE);
		bool frost = Main->HasAura(FROST_FEVER);
		int inc = (u_caster->HasAura(59309)?10:5);
		for (Object::InRangeSet::iterator itr = u_caster->GetInRangeSetBegin(); itr != u_caster->GetInRangeSetEnd(); ++itr)
		{
			if(!(*itr)->IsUnit())
				continue;
			Unit* Target = static_cast<Unit*>((*itr));
			if(Main->GetGUID() == Target->GetGUID() && !u_caster->HasAura(63334))
				continue;
			if(isAttackable(Target, u_caster) && u_caster->CalcDistance((*itr)) <= (pSpell->GetRadius(i) + inc))
			{
				if(blood)
					u_caster->CastSpell(Target, BLOOD_PLAGUE, true);
				if(frost)
					u_caster->CastSpell(Target, FROST_FEVER, true);
			}
		}
		return true;
	}
	return true;
}

void SetupDeathKnightSpells(ScriptMgr * mgr)
{
    mgr->register_dummy_spell(50842, &Pestilence);
}
