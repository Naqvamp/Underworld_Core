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

bool Starfall(uint32 i, Spell* pSpell)
{
	if(!pSpell->m_caster && !pSpell->m_caster->IsUnit())
		return true;
	Unit* m_caster = static_cast<Unit*>(pSpell->m_caster);
	uint32 am = 0;
	for (Object::InRangeSet::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); ++itr)
	{
		if(!(*itr)->IsUnit())
			continue;
		Unit* Target = static_cast<Unit*>((*itr));
		if(isAttackable(Target, m_caster) && m_caster->CalcDistance((*itr)) <= pSpell->GetRadius(i))
		{
			m_caster->CastSpell(Target, pSpell->CalculateEffect(i, Target), true);
			++am;
			if(am >= 20)
				return true;
		}
	}
	return true;
}

void SetupDruidSpells(ScriptMgr * mgr)
{
	mgr->register_dummy_spell(50286, &Starfall); // Starfall: Rank 1
	mgr->register_dummy_spell(53196, &Starfall); // Starfall: Rank 2
	mgr->register_dummy_spell(53197, &Starfall); // Starfall: Rank 3
	mgr->register_dummy_spell(53198, &Starfall); // Starfall: Rank 4
}
