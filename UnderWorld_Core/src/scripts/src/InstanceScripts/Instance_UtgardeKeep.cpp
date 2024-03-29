/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009 ArcEmu Team <http://www.arcemu.org/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
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
/*
	To-Do:
	Ingvar the Plunderer - Ressurection Event
	Fix despawn(i think all bosses)
	Add sound ID's
*/
#include "StdAfx.h"
#include "Setup.h"
#include "../Common/Base.h"

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Forge Master

#define CN_DRAGONFLAYER_FORGE_MASTER							27389
#define DRAGONFLAYER_FORGE_MASTER_BURNING_BRAND		43757

class DragonflayerForgeMasterAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerForgeMasterAI, MoonScriptCreatureAI);
 	DragonflayerForgeMasterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
 	{
		AddSpell(DRAGONFLAYER_FORGE_MASTER_BURNING_BRAND, Target_Current, 8, 0, 40, 0, 30 );
 	}
 };
 
//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer HeartSplitter

#define CN_DRAGONFLAYER_HEARTSPLITTER							24071
#define DRAGONFLAYER_HEARTSPLITTER_PIERCING_JAB		31551
#define DRAGONFLAYER_HEARTSPLITTER_THROW					43665
#define DRAGONFLAYER_HEARTSPLITTER_WING_CLIP			32908

class DragonflayerHeartSplitterAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerHeartSplitterAI, MoonScriptCreatureAI);
 	DragonflayerHeartSplitterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
 	{
		AddSpell(DRAGONFLAYER_HEARTSPLITTER_PIERCING_JAB, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_HEARTSPLITTER_THROW, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_HEARTSPLITTER_WING_CLIP, Target_Current, 8, 0, 40, 0, 30 );
 	}
 };


//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Ironhelm

#define CN_DRAGONFLAYER_IRONHELM							23961
#define DRAGONFLAYER_IRONHELM_HEROIC_STRIKE		29426
#define DRAGONFLAYER_IRONHELM_RINGING_SLAP		42780

class DragonflayerIronhelmAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerIronhelmAI, MoonScriptCreatureAI);
 	DragonflayerIronhelmAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
 	{
		AddSpell(DRAGONFLAYER_IRONHELM_HEROIC_STRIKE, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_IRONHELM_RINGING_SLAP, Target_Current, 8, 0, 40, 0, 30 );
	}
};


//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Metalworker

#define CN_DRAGONFLAYER_METALWORKER						24078
#define DRAGONFLAYER_METALWORKER_ENRAGE				8599
#define	DRAGONFLAYER_METALWORKER_SUNDER_ARMOR	15572

class DragonflayerMetalworkerAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerMetalworkerAI, MoonScriptCreatureAI);
 	DragonflayerMetalworkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
 	{
		AddSpell(DRAGONFLAYER_METALWORKER_SUNDER_ARMOR, Target_Current, 8, 0, 40, 0, 30 );
		mDfEnrage = AddSpell(DRAGONFLAYER_METALWORKER_ENRAGE, Target_Self, 0, 0, 0 );
		Enrage = true;
	}
	
	void AIUpdate()
	{
		if(GetHealthPercent() <= 20 && Enrage)
		{
			CastSpell(mDfEnrage);
			Enrage = false;
		}

		ParentClass::AIUpdate();
	}
	
	bool Enrage;
	SpellDesc*	mDfEnrage;
};

//////////////////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Overseer

#define CN_DRAGONFLAYER_OVERSEER									24085
#define DRAGONFLAYER_OVERSEER_BATTLE_SHOUT				38232
#define DRAGONFLAYER_OVERSEER_CHARGE							35570
#define DRAGONFLAYER_OVERSEER_DEMORALIZING_SHOUT	16244

class DragonflayerOverseerAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerOverseerAI, MoonScriptCreatureAI);
 	DragonflayerOverseerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(DRAGONFLAYER_OVERSEER_BATTLE_SHOUT, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_OVERSEER_CHARGE, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_OVERSEER_DEMORALIZING_SHOUT, Target_Current, 8, 0, 40, 0, 30 );
	}
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Runecaster

#define CN_DRAGONFLAYER_RUNECASTER													23960
#define DRAGONFLAYER_RUNECASTER_BOLTHORNS_RUNE_OF_FLAME			54965			
#define DRAGONFLAYER_RUNECASTER_NJORDS_RUNE_OF_PROTECTION		42740

class DragonflayerRunecasterAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerRunecasterAI, MoonScriptCreatureAI);
 	DragonflayerRunecasterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(DRAGONFLAYER_RUNECASTER_BOLTHORNS_RUNE_OF_FLAME, Target_Self , 100, 0, 0 );
		AddSpell(DRAGONFLAYER_RUNECASTER_NJORDS_RUNE_OF_PROTECTION, Target_Self , 100, 0, 0 );
	}
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Spiritualist

#define CN_DRAGONFLAYER_SPIRITUALIST							28410
#define DRAGONFLAYER_SPIRITUALIST_FLAME_SHOCK			51588
#define DRAGONFLAYER_SPIRITUALIST_HEALING_WAVE		51586
#define DRAGONFLAYER_SPIRITUALIST_LIGHTNING_BOLT	51587

class DragonflayerSpiritualistAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerSpiritualistAI, MoonScriptCreatureAI);
 	DragonflayerSpiritualistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(DRAGONFLAYER_SPIRITUALIST_FLAME_SHOCK, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_SPIRITUALIST_LIGHTNING_BOLT, Target_RandomPlayerNotCurrent, 8, 0, 40, 0, 30 );
		mHealDf = AddSpell(DRAGONFLAYER_SPIRITUALIST_HEALING_WAVE, Target_Self, 0, 0, 2 );
		Heal = true;
	}
	
	void AIUpdate()
	{
		if(GetHealthPercent() <= 42 && Heal)
		{
			CastSpell(mHealDf);
			Heal = false;
		}

		ParentClass::AIUpdate();
	}
	
	bool Heal;
	SpellDesc*	mHealDf;
};

//////////////////////////////////////////////////////////////////////////
//////// Dragonflayer Weaponsmith

#define CN_DRAGONFLAYER_WEAPONSMITH							23956
#define DRAGONFLAYER_WEAPONSMITH_BLIND					42972
#define DRAGONFLAYER_WEAPONSMITH_TICKING_BOMB		54955

class DragonflayerWeaponSmithAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DragonflayerWeaponSmithAI, MoonScriptCreatureAI);
 	DragonflayerWeaponSmithAI(Creature*  pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(DRAGONFLAYER_WEAPONSMITH_BLIND, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(DRAGONFLAYER_WEAPONSMITH_TICKING_BOMB, Target_Current, 8, 0, 40, 0, 30 );
	}
};

//////////////////////////////////////////////////////////////////////////
//////// Enslaved Proto-Drake

#define CN_ESLAVED_PROTO_DRAKE								24083
#define ESLAVED_PROTO_DRAKE_FLAME_BREATH			50653
#define ESLAVED_PROTO_DRAKE_KNOCK_AWAY				49722
#define ESLAVED_PROTO_DRAKE_REND							43931

class EnslavedProto_DrakeAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(EnslavedProto_DrakeAI, MoonScriptCreatureAI);
 	EnslavedProto_DrakeAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(ESLAVED_PROTO_DRAKE_FLAME_BREATH, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(ESLAVED_PROTO_DRAKE_KNOCK_AWAY, Target_Current, 8, 0, 40, 0, 30 );
		AddSpell(ESLAVED_PROTO_DRAKE_REND, Target_Current, 8, 0, 40, 0, 30 );
	}
};

//////////////////////////////////////////////////////////////////////////
//////// Frenzied Geist

#define CN_FRENZIED_GEIST					28419
#define FRENZIED_GEIST_FIXATE			40414

class FrenziedGeistAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(FrenziedGeistAI, MoonScriptCreatureAI);
 	FrenziedGeistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell( FRENZIED_GEIST_FIXATE, Target_Current, 8, 0, 40, 0, 30 );
	}
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////// Skarvald the Constructor  & Dalronn the Controller  ///////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Skarvald the Constructor 
#define CN_SKARVALD 24200
#define CN_SKARVALD_GHOST 27390
#define SKARVALD_CHARGE 43651
#define STONE_STRIKE 48583

// Dalronn the Controller
#define CN_DALRONN 24201
#define CN_DALRONN_GHOST 27389
#define DEBILITATE 43650
#define SHADOW_BOLT 43649
#define SHADOW_BOLT_HC 59575
#define SKELETON_ADD 28878

class SkarvaldTheConstructorAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(SkarvaldTheConstructorAI, MoonScriptCreatureAI);
 	SkarvaldTheConstructorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(SKARVALD_CHARGE, Target_RandomPlayerNotCurrent, 35, 0, 8);
		AddSpell(STONE_STRIKE, Target_ClosestPlayer, 25, 0, 10);

		AddEmote(Event_OnCombatStart, "Dalronn! See if you can muster the nerve to join my attack!", Text_Yell, 13229);

		mReplyTimer = INVALIDATE_TIMER;
	};

	void OnCombatStart(Unit* pTarget)
	{
		pDalronn = (MoonScriptBossAI*)GetNearestCreature(CN_DALRONN); 
		mReplyTimer = AddTimer(2500);

		ParentClass::OnCombatStart(pTarget);
	};

	void AIUpdate()
	{
		if(IsTimerFinished(mReplyTimer) && pDalronn != NULL)
		{
			pDalronn->Emote("By all means, don't assess the situation, you halfwit! Just jump into the fray!", Text_Yell, 13199);
			RemoveTimer(mReplyTimer);
		};

		ParentClass::AIUpdate();
	};

	void OnDied(Unit* pKiller)
	{
		if(pDalronn != NULL && pDalronn->IsAlive())
		{
			Emote("Not... over... yet.", Text_Yell, 0);
			pDalronn->Emote("Skarvald, you incompetent slug! Return and make yourself useful!", Text_Yell, 13203);
			SpawnCreature(CN_SKARVALD_GHOST, true);
			_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
			
		}
		else if(pDalronn != NULL && !pDalronn->IsAlive()) 
		{
			Emote("A warrior's death.", Text_Yell, 13231);

			pDalronnGhost = (MoonScriptBossAI*)GetNearestCreature(CN_DALRONN_GHOST);

			if(pDalronnGhost != NULL)
				pDalronnGhost->Despawn(1000, 0);
		}

		ParentClass::OnDied(pKiller);
	};
    
	void OnCombatStop(Unit* pTarget)
	{
		if( pDalronn != NULL )
		{	
			if( pDalronn->IsAlive() ) 
				MoveToSpawnOrigin();
			else
				SpawnCreature( CN_DALRONN, pDalronn->GetUnit()->GetSpawnX(), pDalronn->GetUnit()->GetSpawnY(), pDalronn->GetUnit()->GetSpawnZ(), pDalronn->GetUnit()->GetSpawnO());
		};

		if( pDalronnGhost != NULL && pDalronnGhost->IsAlive() )
			pDalronnGhost->Despawn();
	};

	void Destroy()
	{
		delete this;
	};

 private:
	 int32					mReplyTimer;
	 MoonScriptBossAI*		pDalronn;
	 MoonScriptBossAI*		pDalronnGhost;
};
		
class DalronnTheControllerAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DalronnTheControllerAI, MoonScriptCreatureAI);
 	DalronnTheControllerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		if(IsHeroic())
		{
			AddSpell(SHADOW_BOLT_HC, Target_RandomPlayer, 85, 2, 3);
			AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
			mSummonTimer = AddTimer(15000);
		}
		else
		{
			AddSpell(SHADOW_BOLT, Target_RandomPlayer, 35, 2, 8);
			AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
			mSummonTimer = AddTimer(15000);
		}
	};

	void OnCombatStart(Unit* pTarget)
	{
		pSkarvald = (MoonScriptBossAI*)GetNearestCreature(CN_SKARVALD); 

		ParentClass::OnCombatStart(pTarget);
	};

	void AIUpdate()
	{
		if(IsTimerFinished(mSummonTimer))
		{
			SpawnCreature(SKELETON_ADD, _unit->GetPositionX() + 6, _unit->GetPositionY() + 4, _unit->GetPositionZ(), 0, true);
			SpawnCreature(SKELETON_ADD, _unit->GetPositionX() - 6, _unit->GetPositionY() + 4, _unit->GetPositionZ(), 0, true);
			ResetTimer(mSummonTimer, 15000);
		};

		ParentClass::AIUpdate();
	};
	
	void OnDied(Unit* pKiller)
	{
		if(pSkarvald != NULL && pSkarvald->IsAlive())
		{
			Emote("See... you... soon.", Text_Yell, 0);
			pSkarvald->Emote("Pagh! What sort of necromancer lets death stop him? I knew you were worthless!", Text_Yell, 13233);
			SpawnCreature(CN_DALRONN_GHOST, true);
			_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		}
		else if(pSkarvald != NULL && !pSkarvald->IsAlive()) 
		{
			Emote("There's no... greater... glory.", Text_Yell, 13201);

			pSkarvaldGhost = (MoonScriptBossAI*)GetNearestCreature(CN_SKARVALD_GHOST);

			if(pSkarvaldGhost != NULL)
				pSkarvaldGhost->Despawn(1000, 0);
		}

		ParentClass::OnDied(pKiller);
	};

	void OnCombatStop(Unit* pTarget)
	{
		if( pSkarvald != NULL )
		{
			if( pSkarvald->IsAlive() )
				MoveToSpawnOrigin();
			else
				SpawnCreature( CN_DALRONN, pSkarvald->GetUnit()->GetSpawnX(), pSkarvald->GetUnit()->GetSpawnY(), pSkarvald->GetUnit()->GetSpawnZ(), pSkarvald->GetUnit()->GetSpawnO());
		};

		if( pSkarvaldGhost != NULL && pSkarvaldGhost->IsAlive() )
			pSkarvaldGhost->Despawn();
	};

	void Destroy()
	{
		delete this;
	};

 private:
	int32					mSummonTimer;
	MoonScriptBossAI*		pSkarvald;
	MoonScriptBossAI*		pSkarvaldGhost;
};

class SkarvaldTheConstructorGhostAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(SkarvaldTheConstructorGhostAI, MoonScriptCreatureAI);
 	SkarvaldTheConstructorGhostAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		AddSpell(SKARVALD_CHARGE, Target_RandomPlayerNotCurrent, 35, 0, 8);
		AddSpell(STONE_STRIKE, Target_ClosestPlayer, 25, 0, 10);
	};

	void OnLoad()
	{
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);

		Player* pTarget = GetNearestPlayer();
		if(pTarget != NULL)
			_unit->GetAIInterface()->AttackReaction(pTarget, 50, 0);

		ParentClass::OnLoad();
	};

	void Destroy()
	{
		delete this;
	}
};

class DalronnTheControllerGhostAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(DalronnTheControllerGhostAI, MoonScriptCreatureAI);
 	DalronnTheControllerGhostAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		if(IsHeroic())
		{
			AddSpell(SHADOW_BOLT_HC, Target_RandomPlayer, 85, 2, 3);
			AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
		}
		else
		{
			AddSpell(SHADOW_BOLT, Target_RandomPlayer, 35, 2, 8);
			AddSpell(DEBILITATE, Target_RandomPlayer, 25, 0, 12);
		}
	};

	void OnLoad()
	{
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);

		Player* pTarget = GetNearestPlayer();
		if(pTarget != NULL)
			_unit->GetAIInterface()->AttackReaction(pTarget, 50, 0);

		ParentClass::OnLoad();
	};

	void Destroy()
	{
		delete this;
	};

};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Prince Keleseth ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define CN_PRINCE_KELESETH 23953
#define CN_FROST_TOMB 23965
#define KELESETH_SHADOW_BOLT 43667
#define KELESETH_SHADOW_BOLT_HC 59389
#define FROST_TOMB_SPELL 48400

#define KELESETH_SKELETON_ADD 23970
#define DECREPIFY 42702
#define DECREPIFY_HC 59397

void SpellFunc_KelesethFrostTomb( SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType )
{
	if(pCreatureAI != NULL)
	{
		if(pTarget == NULL || !pTarget->IsPlayer() || pTarget->IsDead())
			return;
		
		pCreatureAI->GetUnit()->CastSpell( pTarget, FROST_TOMB_SPELL, true );
		pTarget->GetMapMgr()->GetInterface()->SpawnCreature( CN_FROST_TOMB, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation(), true, false, 0,0 );
		pCreatureAI->Emote( "Not so fast.", Text_Yell, 0 );
	};
};


void SpellFunc_KelesethAddSummon( SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType )
{
	if(pCreatureAI != NULL)
	{	
		for(uint32 i = 0; i < 5; ++i)
			pCreatureAI->SpawnCreature( KELESETH_SKELETON_ADD, 163.376f + i + 4, 252.901f - i + 5, 42.868f, 0, true);
	};
};
class PrinceKelesethAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(PrinceKelesethAI, MoonScriptCreatureAI);
 	PrinceKelesethAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
	{
		mFrostTomb = AddSpellFunc( &SpellFunc_KelesethFrostTomb, Target_RandomPlayer, 25, 0, 15, 0, 20);
		mAddSummon = AddSpellFunc( &SpellFunc_KelesethAddSummon, Target_Self, 0, 0, 0);
		
		if(IsHeroic())
			mShadowBolt = AddSpell(KELESETH_SHADOW_BOLT_HC, Target_Current, 100, 2, 2);
		else
			mShadowBolt = AddSpell(KELESETH_SHADOW_BOLT, Target_Current, 100, 2, 2);
	
		AddEmote(Event_OnDied, "I join... the night.", Text_Yell, 13225);
	};

	void OnCombatStart(Unit* pTarget)
	{
		CastSpellNowNoScheduling(mAddSummon);
		Emote("Your blood is mine!", Text_Yell, 13221);

		ParentClass::OnCombatStart(pTarget);
	};
	
	void Destroy()
	{
		delete this;
	};

	SpellDesc*			mAddSummon;
	SpellDesc*			mShadowBolt;
	SpellDesc*			mFrostTomb;
};

class FrostTombAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(FrostTombAI, MoonScriptCreatureAI);
 	FrostTombAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) 
	{
		SetAIUpdateFreq( 1000 );
	};

	void OnLoad()
	{
		SetCanMove( false );
		plr = GetNearestPlayer();
		ParentClass::OnLoad();
	};

	void AIUpdate()
	{
		if(plr == NULL || plr->IsDead() || !plr->HasAura(FROST_TOMB_SPELL))
			Despawn( 0, 0 );

		ParentClass::AIUpdate();
	};

	void OnDied(Unit* pKilled)
	{
		if(plr != NULL && plr->HasAura(FROST_TOMB_SPELL))
			plr->RemoveAura(FROST_TOMB_SPELL);

		Despawn( 0, 0 );

		ParentClass::OnDied(pKilled);
	};

	void Destroy()
	{
		delete this;
	};

 private:
	 Player* plr;
};

class SkeletonAddAI : public MoonScriptCreatureAI
 {
 	MOONSCRIPT_FACTORY_FUNCTION(SkeletonAddAI, MoonScriptCreatureAI);
 	SkeletonAddAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) 
	{
		if( IsHeroic() )
			AddSpell(DECREPIFY_HC, Target_Current, 8, 0, 40);
		else
			AddSpell(DECREPIFY, Target_Current, 8, 0, 40);
	};

	void OnLoad()
	{	
		Player* pTarget = GetNearestPlayer();
		if(pTarget != NULL)
			_unit->GetAIInterface()->AttackReaction( pTarget, 50, 0 );

		ParentClass::OnLoad();
	};

	void OnCombatStop(Unit* pTarget)
	{
		Despawn();
	};

	void OnDied(Unit* pKiller)
	{
		Despawn();
	};

	void Destroy()
	{
		delete this;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Ingvar the Plunderer //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#define CN_INGVAR 23954
#define CN_INGVAR_UNDEAD 23980
#define CN_SHADOW_AXE 23996

// Phase 1 spells (Human Form)
#define INGVAR_CLEAVE 42724
#define INGVAR_ENRAGE 59707
#define INGVAR_SMASH 42669
#define INGVAR_ROAR 42708
#define INGVAR_ENRAGE_HC 42705
#define INGVAR_SMASH_HC 59706
#define INGVAR_ROAR_HC 59708

// Phase 2 spells (Undead Form)
#define INGVAR_DARK_SMASH 42723
#define INGVAR_DREADFUL_ROAR 42729
#define INGVAR_WOE_STRIKE 42730
#define INGVAR_DREADFUL_ROAR_HC 59734
#define INGVAR_WOE_STRIKE_HC 59735

#define SHADOW_AXE_SPELL 42751

void SpellFunc_ShadowAxe( SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType )
{
	if(pCreatureAI != NULL)
	{
		if(pTarget == NULL || !pTarget->IsPlayer() || pTarget->IsDead())
			return;
		
		Creature* pShadowAxe = pTarget->GetMapMgr()->GetInterface()->SpawnCreature(CN_SHADOW_AXE, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation(), true, false, 0, 0 );
		
		if(pShadowAxe == NULL)
			return;

		pShadowAxe->CastSpell( pShadowAxe, SHADOW_AXE_SPELL, true );
		pShadowAxe->Despawn( 10000, 0);
	};
};

class IngvarThePlundererAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(IngvarThePlundererAI, MoonScriptCreatureAI);
 	IngvarThePlundererAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) 
	{
		AddSpell(INGVAR_CLEAVE, Target_Current, 24, 0, 6 );

		if(IsHeroic())
		{
			AddSpell(INGVAR_ENRAGE_HC, Target_Self, 45, 0, 4 );
			AddSpell(INGVAR_SMASH_HC, Target_Self, 25, 3, 18 );
			AddSpell(INGVAR_ROAR_HC, Target_Self, 25, 2, 10 );
		}
		else
		{
			AddSpell(INGVAR_ENRAGE, Target_Self, 45, 0, 4 );
			AddSpell(INGVAR_SMASH, Target_Self, 25, 3, 18 );
			AddSpell(INGVAR_ROAR, Target_Self, 25, 2, 10 );
		};

		AddEmote(Event_OnTargetDied, "Mjul orm agn gjor!", Text_Yell, 13212);
		AddEmote(Event_OnCombatStart, "I'll paint my face with your blood!", Text_Yell, 13207);
		AddEmote(Event_OnDied, "My life for the... death god!", Text_Yell, 13213);

		SetAIUpdateFreq(1000);
	};

	void OnDied(Unit* pKiller)
	{
		//Ressurect event
		SpawnCreature(CN_INGVAR_UNDEAD, true);
		_unit->Despawn( 1000, 0);
	};
};

class IngvarUndeadAI : public MoonScriptCreatureAI
{
	MOONSCRIPT_FACTORY_FUNCTION(IngvarUndeadAI, MoonScriptCreatureAI);
 	IngvarUndeadAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) 
	{
		AddSpellFunc( &SpellFunc_ShadowAxe, Target_RandomPlayerNotCurrent, 15, 0, 21);
		AddSpell(INGVAR_DARK_SMASH, Target_Self, 12, 3, 16 );
		
		if(IsHeroic())
		{
			AddSpell(INGVAR_DREADFUL_ROAR, Target_Self, 25, 2, 10 );
			AddSpell(INGVAR_WOE_STRIKE, Target_ClosestUnit, 18, 0, 16 );
		}
		

		AddEmote(Event_OnDied, "No! I can do... better! I can...", Text_Yell, 13211);
	};

	void OnLoad()
	{
		Player* pTarget = GetNearestPlayer();
		if(pTarget != NULL)
			_unit->GetAIInterface()->AttackReaction(pTarget, 50, 0);
	};
};

void SetupUtgardeKeep(ScriptMgr * mgr)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	///////// Mobs
	mgr->register_creature_script(CN_DRAGONFLAYER_FORGE_MASTER, &DragonflayerForgeMasterAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_HEARTSPLITTER, &DragonflayerHeartSplitterAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_IRONHELM, &DragonflayerIronhelmAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_METALWORKER, &DragonflayerIronhelmAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_OVERSEER, &DragonflayerOverseerAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_RUNECASTER, &DragonflayerRunecasterAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_SPIRITUALIST, &DragonflayerSpiritualistAI::Create);
	mgr->register_creature_script(CN_DRAGONFLAYER_WEAPONSMITH, &DragonflayerWeaponSmithAI::Create);
	mgr->register_creature_script(CN_ESLAVED_PROTO_DRAKE, &EnslavedProto_DrakeAI::Create);
	mgr->register_creature_script(CN_FRENZIED_GEIST, &FrenziedGeistAI::Create);
	//////////////////////////////////////////////////////////////////////////////////////////
	///////// Bosses
	
	// Skarvald & Dalronn Encounter
	mgr->register_creature_script(CN_SKARVALD, &SkarvaldTheConstructorAI::Create);
	mgr->register_creature_script(CN_DALRONN, &DalronnTheControllerAI::Create);
	mgr->register_creature_script(CN_SKARVALD_GHOST, &SkarvaldTheConstructorGhostAI::Create);
	mgr->register_creature_script(CN_DALRONN_GHOST, &DalronnTheControllerGhostAI::Create);

	// Prince Keleseth Encounter
	mgr->register_creature_script(CN_PRINCE_KELESETH, &PrinceKelesethAI::Create);
	mgr->register_creature_script(CN_FROST_TOMB, &FrostTombAI::Create);
	mgr->register_creature_script(KELESETH_SKELETON_ADD, &SkeletonAddAI::Create);

	// Ingvar the Plunderer Encounter
	mgr->register_creature_script(CN_INGVAR, &IngvarThePlundererAI::Create);
	mgr->register_creature_script(CN_INGVAR_UNDEAD, &IngvarUndeadAI::Create);
};
