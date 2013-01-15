/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009 ArcEmu Team <http://www.arcemu.org/>
 * Copyright (C) 2008-2009 Sun++ Team <http://www.sunscripting.com/>
 * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
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
#include "../Common/Base.h"

/************************************************************************/
/* Instance_Arcatraz.cpp Script											*/
/************************************************************************/

// Zereketh the UnboundAI
#define CN_ZEREKETH			20870
#define CN_VOIDZONEARC		21101

#define SEED_OF_C			36123	//32865, 36123
#define SHADOW_NOVA			36127 // 30533, 39005, 36127 (normal mode), 39005 (heroic mode?)
#define SHADOW_NOVA_H		39005
#define CONSUMPTION			30498
#define CONSUMPTION_H		39004
// #define VOID_ZONE 36119	// DBC: 36119; it's not fully functionl without additional core support (for dmg and random place targeting).

class ZerekethAI : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(ZerekethAI);
	SP_AI_Spell spells[2];
	bool m_spellcheck[2];

    ZerekethAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {

		nrspells = 2;
		for(int i=0;i<nrspells;i++)
		{
			m_spellcheck[i] = false;
		}
			
		spells[0].info = dbcSpell.LookupEntry(SEED_OF_C);
		spells[0].targettype = TARGET_RANDOM_SINGLE;
		spells[0].instant = false;
		spells[0].cooldown = 20;
		spells[0].perctrigger = 6.0f;
		spells[0].attackstoptimer = 2000;
		spells[0].mindist2cast = 0.0f;
		spells[0].maxdist2cast = 100.0f;

		if(_unit->GetMapMgr()->iInstanceMode != MODE_HEROIC)
		{

			spells[1].info = dbcSpell.LookupEntry(SHADOW_NOVA);
			spells[1].targettype = TARGET_VARIOUS;
			spells[1].instant = false;
			spells[1].cooldown = 15;
			spells[1].perctrigger = 15.0f;
			spells[1].attackstoptimer = 1500;
		}
		else
		{
			spells[1].info = dbcSpell.LookupEntry(SHADOW_NOVA_H);
			spells[1].targettype = TARGET_VARIOUS;
			spells[1].instant = false;
			spells[1].cooldown = 15;
			spells[1].perctrigger = 15.0f;
			spells[1].attackstoptimer = 1500;
		}
	}
    void OnCombatStart(Unit* mTarget)
    {
		for(int i=0;i<nrspells;i++)
			spells[i].casttime = spells[i].cooldown;

		RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
		_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Life energy to... consume.");
		_unit->PlaySoundToSet(11250);

		uint32 t = (uint32)time(NULL);
		VoidTimer = t + RandomUInt(10)+30;
		SpeechTimer = t + RandomUInt(10)+40;
    }

    void OnCombatStop(Unit* mTarget)
    {
        _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
        _unit->GetAIInterface()->SetAIState(STATE_IDLE);
        RemoveAIUpdateEvent();
	}

    void OnDied(Unit* mKiller)
    {
		//despawn voids
		Creature* creature = NULL;
		for(set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
		{
			if((*itr)->GetTypeId() == TYPEID_UNIT)
			{
				creature = TO_CREATURE((*itr));

				if(creature && creature->GetCreatureInfo() && creature->GetCreatureInfo()->Id == 21101 && creature->isAlive())
				{
					creature->Despawn(0, 0);
					//creature->SafeDelete();
				}
			}
		}

		_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The Void... beckons.");
		_unit->PlaySoundToSet(11255);

		RemoveAIUpdateEvent();
    }

	void OnTargetDied(Unit* mTarget)
    {
		if (_unit->GetHealthPct() > 0)	// Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
		{
			int RandomSpeach;
			RandomUInt(1000);
			RandomSpeach=rand()%2;
			switch (RandomSpeach)
			{
			case 0:
				_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "This vessel...is empty.");
				_unit->PlaySoundToSet(11251);
				break;
			case 1:
				_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "No... more... life.");	// not sure
				_unit->PlaySoundToSet(11252);
				break;
			}
		}
    }
	
	void Speech()
	{
		switch (RandomUInt(1))
		{
		case 0:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The shadow... will engulf you.");
			_unit->PlaySoundToSet(11253);
			break;
		case 1:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Darkness... consumes all.");
			_unit->PlaySoundToSet(11254);
			break;
		}
		SpeechTimer = (uint32)time(NULL) + RandomUInt(10)+40;
	}

	void VoidZoneArc()
	{
		VoidTimer = (uint32)time(NULL) + RandomUInt(10)+30;
		
		CreatureInfo * ci = CreatureNameStorage.LookupEntry(CN_VOIDZONEARC);
		CreatureProto * cp = CreatureProtoStorage.LookupEntry(CN_VOIDZONEARC);
		if(!cp || !ci)
			return;

		std::vector<Player*> TargetTable;
		set< Object* >::iterator Itr = _unit->GetInRangePlayerSetBegin();
		for(; Itr != _unit->GetInRangePlayerSetEnd(); Itr++) 
		{ 
			Player *RandomTarget = NULL;
			RandomTarget = static_cast< Player* >(*Itr);
			if(RandomTarget && RandomTarget->isAlive() && isHostile(*Itr, _unit))
				TargetTable.push_back(RandomTarget);
			RandomTarget = NULL;
		}

		if (!TargetTable.size())
			return;

		size_t RandTarget = rand()%TargetTable.size();

		Player*  RTarget = TargetTable[RandTarget];

		if (!RTarget)
			return;

		float vzX = RandomUInt(5) * cos(RandomFloat(6.28f))+RTarget->GetPositionX();
		float vzY = RandomUInt(5) * cos(RandomFloat(6.28f))+RTarget->GetPositionY();
		float vzZ = RTarget->GetPositionZ();
		Creature* VoidZone = _unit->GetMapMgr()->CreateCreature(cp->Id);
		VoidZone->Load(cp, vzX, vzY, vzZ);
		VoidZone->SetInstanceID(_unit->GetInstanceID());
		VoidZone->SetZoneId(_unit->GetZoneId());
		VoidZone->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
		VoidZone->m_noRespawn = true;
		if(VoidZone->CanAddToWorld())
		{
			VoidZone->PushToWorld(_unit->GetMapMgr());
		}
		else
		{
			VoidZone->SafeDelete();
			return;
		}
		RTarget = NULL;
		VoidZone->Despawn( 60000, 0 );
	}

    void AIUpdate()
	{
		uint32 t = (uint32)time(NULL);
		if(t > SpeechTimer)
			Speech();

		if(t > VoidTimer)
			VoidZoneArc();
		
		float val = (float)RandomFloat(100.0f);
		SpellCast(val);
    }

	void SpellCast(float val)
    {
        if(_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->GetNextTarget())
        {
			float comulativeperc = 0;
		    Unit* target = NULL;
			for(int i=0;i<nrspells;i++)
			{
				if(!spells[i].perctrigger) continue;
				
				if(m_spellcheck[i])
				{
					target = _unit->GetAIInterface()->GetNextTarget();
					switch(spells[i].targettype)
					{
						case TARGET_SELF:
						case TARGET_VARIOUS:
							_unit->CastSpell(_unit, spells[i].info, spells[i].instant); break;
						case TARGET_ATTACKING:
							_unit->CastSpell(target, spells[i].info, spells[i].instant); break;
						case TARGET_DESTINATION:
							_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), spells[i].info, spells[i].instant); break;
						case TARGET_RANDOM_SINGLE:
						case TARGET_RANDOM_DESTINATION:
							CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast); break;
					}
					m_spellcheck[i] = false;
					return;
				}

				uint32 t = (uint32)time(NULL);
				if(val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
				{
					_unit->setAttackTimer(spells[i].attackstoptimer, false);
					spells[i].casttime = t + spells[i].cooldown;
					m_spellcheck[i] = true;
				}
				comulativeperc += spells[i].perctrigger;
			}
			target = NULL;
        }
    }

	void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
	{
		if (!maxdist2cast) maxdist2cast = 100.0f;
		if (!maxhp2cast) maxhp2cast = 100;

		if(_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->GetNextTarget())
        {
			std::vector<Player* > TargetTable;
			for(set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr) 
			{ 
				Player *RandomTarget = NULL;
				RandomTarget = static_cast< Player* >(*itr);

				if (RandomTarget && RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast*mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast*maxdist2cast)
					TargetTable.push_back(RandomTarget);
				RandomTarget = NULL;
			}

			if (!TargetTable.size())
				return;

			size_t RandTarget = rand()%TargetTable.size();

			Unit*  RTarget = TargetTable[RandTarget];

			if (!RTarget)
				return;

			switch (spells[i].targettype)
			{
			case TARGET_RANDOM_SINGLE:
				_unit->CastSpell(RTarget, spells[i].info, spells[i].instant); break;
			case TARGET_RANDOM_DESTINATION:
				_unit->CastSpellAoF(RTarget->GetPositionX(), RTarget->GetPositionY(), RTarget->GetPositionZ(), spells[i].info, spells[i].instant); break;
			}
			RTarget = NULL;
			TargetTable.clear();
		}
	}

	void Destroy()
	{
		delete this;
	}

protected:

	uint32 SpeechTimer;
	uint32 VoidTimer;
	int nrspells;
};

class VoidZoneARC : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(VoidZoneARC);
	VoidZoneARC(Creature* pCreature) : CreatureAIScript(pCreature)
    {
		_unit->Root();
		_unit->DisableAI();
		RegisterAIUpdateEvent( 1000 );
	};

	void AIUpdate()
	{
		// M4ksiu: I'm not sure if it should be cast once, on start
		uint32 SpellId = CONSUMPTION;
		if ( _unit->GetMapMgr()->iInstanceMode == MODE_HEROIC )
			SpellId = CONSUMPTION_H;

		_unit->CastSpell( _unit, SpellId, true );
		RemoveAIUpdateEvent();
	};
};


// Dalliah the DoomsayerAI

#define CN_DALLIAH_THE_DOOMSAYER 20885	

#define GIFT_OF_THE_DOOMSAYER 36173 // DBC: 36173
#define WHIRLWIND 36175	// DBC: 36142, 36175
#define HEAL 36144
#define SHADOW_WAVE 39016	// Heroic mode spell
// sounds missing related to Wrath... (look on script below this one)

class DalliahTheDoomsayerAI : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(DalliahTheDoomsayerAI);
	SP_AI_Spell spells[4];
	bool m_spellcheck[4];

	DalliahTheDoomsayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
	{
		if(_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC)
		{
				nrspells = 4;
			}
			else
			{
				nrspells = 3;
			}
			for(int i=0;i<nrspells;i++)
			{
				m_spellcheck[i] = false;
			}

			spells[0].info = dbcSpell.LookupEntry(GIFT_OF_THE_DOOMSAYER);
			spells[0].targettype = TARGET_ATTACKING;
			spells[0].instant = false;
			spells[0].cooldown = -1;
			spells[0].perctrigger = 8.0f;
			spells[0].attackstoptimer = 1000;
			
			spells[1].info = dbcSpell.LookupEntry(WHIRLWIND);
			spells[1].targettype = TARGET_VARIOUS;
			spells[1].instant = false;
			spells[1].cooldown = -1;
			spells[1].perctrigger = 15.0f;
			spells[1].attackstoptimer = 1000;
			
			spells[2].info = dbcSpell.LookupEntry(HEAL);
			spells[2].targettype = TARGET_SELF;
			spells[2].instant = false;
			spells[2].cooldown = -1;
			spells[2].perctrigger = 0.0f;
			spells[2].attackstoptimer = 1000;
			
			if(_unit->GetMapMgr()->iInstanceMode == MODE_HEROIC)
			{
				spells[3].info = dbcSpell.LookupEntry(SHADOW_WAVE);
				spells[3].targettype = TARGET_ATTACKING;
				spells[3].instant = false;
				spells[3].cooldown = -1;
				spells[3].perctrigger = 8.0f;
				spells[3].attackstoptimer = 1000;
			}
			
	}
	
	void OnCombatStart(Unit* mTarget)
	{
			CastTime();
			RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "It is unwise to anger me.");	// verification needed
			_unit->PlaySoundToSet(11086);
	}

	void CastTime()
	{
			for(int i=0;i<nrspells;i++)
			spells[i].casttime = spells[i].cooldown;
		}
		
		void OnTargetDied(Unit* mTarget)
	{
			if (_unit->GetHealthPct() > 0)
			{
				int RandomSpeach;
				RandomSpeach=rand()%2;
				switch (RandomSpeach)
				{
					case 0:
						_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Completely ineffective! Just like someone else I know!");	// need verif.
						_unit->PlaySoundToSet(11087);
						break;
					case 1:
						_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You chose the wrong opponent!");	// verification needed
						_unit->PlaySoundToSet(11088);
						break;
				}
			}
		}
		
		void OnCombatStop(Unit* mTarget)
		{
			CastTime();
			_unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
			_unit->GetAIInterface()->SetAIState(STATE_IDLE);
			RemoveAIUpdateEvent();
		}
		
		void OnDied(Unit* mKiller)
		{
			CastTime();
			RemoveAIUpdateEvent();
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Now I'm really... angry...");	// verification needed
			_unit->PlaySoundToSet(11093);
			
			GameObject* door2 = NULL;
			door2 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 199.969f, 118.5837f, 22.379f, 184319 );
			if(door2)
				door2->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
		}

		void AIUpdate()
		{
			float val = (float)RandomFloat(100.0f);
			SpellCast(val);
		}

		void HealSound()
		{
			int RandomSpeach;
			RandomSpeach=rand()%20;
			switch (RandomSpeach)
			{
				case 0:
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "That is much better.");
					_unit->PlaySoundToSet(11091);
					break;
				case 1:
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Ah, just what I needed.");	// both need verif.
					_unit->PlaySoundToSet(11092);
					break;
			}
		}
		
		void WhirlwindSound()
		{
			int RandomWhirlwind;
			RandomWhirlwind=rand()%20;
			switch (RandomWhirlwind)
			{
				case 0:
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Reap the Whirlwind!");
					_unit->PlaySoundToSet(11089);
					break;
				case 1:
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I'll cut you to pices!");	// all to verification
					_unit->PlaySoundToSet(11090);
					break;
			}
		}
		
		void SpellCast(float val)
		{
			if(_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->GetNextTarget())
			{
				float comulativeperc = 0;
				Unit* target = NULL;
				for(int i=0;i<nrspells;i++)
				{
					spells[i].casttime--;
					
					if (m_spellcheck[i])
					{		
						if (m_spellcheck[2] == true)
							HealSound();
						
						spells[i].casttime = spells[i].cooldown;
						target = _unit->GetAIInterface()->GetNextTarget();
						switch(spells[i].targettype)
						{
							case TARGET_SELF:
							case TARGET_VARIOUS:
								_unit->CastSpell(_unit, spells[i].info, spells[i].instant); break;
							case TARGET_ATTACKING:
								_unit->CastSpell(target, spells[i].info, spells[i].instant); break;
							case TARGET_DESTINATION:
								_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), spells[i].info, spells[i].instant); break;
						}
						
						if (m_spellcheck[1] == true)
						{
							WhirlwindSound();
							int NextAttack;
							NextAttack=rand()%100+1;
							if (NextAttack <= 25 && NextAttack > 0)
								m_spellcheck[2] = true;
						}
						
						if (spells[i].speech != "")
						{
							_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
							_unit->PlaySoundToSet(spells[i].soundid); 
						}
						
						m_spellcheck[i] = false;
						return;
					}
					
					if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
					{
						_unit->setAttackTimer(spells[i].attackstoptimer, false);
						m_spellcheck[i] = true;
					}
					comulativeperc += spells[i].perctrigger;
				}
			}
		}

	void Destroy()
	{
		delete this;
	};

protected:
	
	int nrspells;
};

// Wrath-Scryer SoccothratesAI
// TO DO: Add moar sounds
#define CN_WRATH_SCRYER_SOCCOTHRATES 20886	

#define IMMOLATION 35959 // DBC: 36051, 35959
#define FELFIRE_SHOCK 35759
#define FELFIRE_LINE_UP 35770	// ?
#define KNOCK_AWAY 20686 // DBC: 36512; but it uses it on himself too so changed to other
#define CHARGE 35754 // DBC: 36058, 35754 =( =(
// CHARGE_TARGETING 36038 ?
// There are more sounds connected with Dalliah and some spells, but I don't know situation in which they are used
// so haven't added them.

class WrathScryerSoccothratesAI : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(WrathScryerSoccothratesAI);
	SP_AI_Spell spells[5];
	bool m_spellcheck[5];
	
		WrathScryerSoccothratesAI(Creature* pCreature) : CreatureAIScript(pCreature)
		{
			nrspells = 5;
			for(int i=0;i<nrspells;i++)
			{
				m_spellcheck[i] = false;
			}
			
			spells[0].info = dbcSpell.LookupEntry(IMMOLATION);
			spells[0].targettype = TARGET_VARIOUS;
			spells[0].instant = false;
			spells[0].cooldown = -1;
			spells[0].perctrigger = 10.0f;
			spells[0].attackstoptimer = 1000;
			
			spells[1].info = dbcSpell.LookupEntry(FELFIRE_SHOCK);
			spells[1].targettype = TARGET_ATTACKING;
			spells[1].instant = true;
			spells[1].cooldown = -1;
			spells[1].perctrigger = 8.0f;
			spells[1].attackstoptimer = 1000;
			
			spells[2].info = dbcSpell.LookupEntry(FELFIRE_LINE_UP);	// ?
			spells[2].targettype = TARGET_SELF;
			spells[2].instant = true;
			spells[2].cooldown = -1;
			spells[2].perctrigger = 8.0f;
			spells[2].attackstoptimer = 1000;
			
			spells[3].info = dbcSpell.LookupEntry(KNOCK_AWAY);
			spells[3].targettype = TARGET_DESTINATION;	// changed from VARIOUS to prevent crashes and gives it at least half working spell
			spells[3].instant = true;
			spells[3].cooldown = -1;
			spells[3].perctrigger = 6.0f;
			spells[3].attackstoptimer = 1000;
			
			spells[4].info = dbcSpell.LookupEntry(CHARGE);
			spells[4].targettype = TARGET_ATTACKING;
			spells[4].instant = true;
			spells[4].cooldown = -1;
			spells[4].perctrigger = 4.0f;
			spells[4].attackstoptimer = 1000;
		}
		
		void OnCombatStart(Unit* mTarget)
		{
			CastTime();
			RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "At last, a target for my frustrations!");	// verified 
			_unit->PlaySoundToSet(11238);
		}
		
		void CastTime()
		{
			for(int i=0;i<nrspells;i++)
			spells[i].casttime = spells[i].cooldown;
		}
		
		void OnTargetDied(Unit* mTarget)
		{
			if (_unit->GetHealthPct() > 0)
			{
				int RandomSpeach;
				RandomSpeach=rand()%2;
				switch (RandomSpeach)
				{
					case 0:
						_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Yes, that was quiet... satisfying.");	// verified
						_unit->PlaySoundToSet(11239);
						break;
					case 1:
						_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Ha! Much better!");	// verified
						_unit->PlaySoundToSet(11240);
						break;
				}		
			}
		}
		
		void OnCombatStop(Unit* mTarget)
		{
			CastTime();
			_unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
			_unit->GetAIInterface()->SetAIState(STATE_IDLE);
			RemoveAIUpdateEvent();
		}
		
		void OnDied(Unit* mKiller)
		{
			CastTime();
			RemoveAIUpdateEvent();
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Knew this was... the only way out.");	// verified
			_unit->PlaySoundToSet(11243);
			
			GameObject* door1 = NULL;
			door1 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 199.969f, 118.5837f, 22.379f, 184318 );
			if(door1)
				door1->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
		}
		
		void AIUpdate()
		{
			float val = (float)RandomFloat(100.0f);
			SpellCast(val);
		}

		void SpellCast(float val)
		{
			if(_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->GetNextTarget())
			{
				float comulativeperc = 0;
				Unit* target = NULL;
				for(int i=0;i<nrspells;i++)
				{
					spells[i].casttime--;
					
					if (m_spellcheck[i])
					{					
						spells[i].casttime = spells[i].cooldown;
						target = _unit->GetAIInterface()->GetNextTarget();
						switch(spells[i].targettype)
						{
							case TARGET_SELF:
							case TARGET_VARIOUS:
								_unit->CastSpell(_unit, spells[i].info, spells[i].instant); break;
							case TARGET_ATTACKING:
								_unit->CastSpell(target, spells[i].info, spells[i].instant); break;
							case TARGET_DESTINATION:
								_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), spells[i].info, spells[i].instant); break;
						}
						
						if (spells[i].speech != "")
						{
							_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
							_unit->PlaySoundToSet(spells[i].soundid); 
						}
						
						m_spellcheck[i] = false;
						return;
					}
					
					if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
					{
						_unit->setAttackTimer(spells[i].attackstoptimer, false);
						m_spellcheck[i] = true;
					}
					comulativeperc += spells[i].perctrigger;
				}
			}
		}

	void Destroy()
	{
		delete this;
	};

protected:

	int nrspells;
};

// Harbinger SkyrissAI
// Full event must be scripted for this guy.
#define CN_HARBRINGER_SKYRISS 20912	

#define MIND_REND 36924 // DBC: 36859, 36924;
#define FEAR 39415
#define DOMINATION 37162
#define SUMMON_ILLUSION_66 36931	// those 2 don't work
#define SUMMON_ILLUSION_33 36932
// BLINK_VISUAL 36937 ?
// SIMPLE_TELEPORT 12980 ?
// Add sounds related to his dialog with mind controlled guy

class HarbringerSkyrissAI : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(HarbringerSkyrissAI);
	SP_AI_Spell spells[5];
	bool m_spellcheck[5];

    HarbringerSkyrissAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {

		IllusionCount = 0;
		nrspells = 5;
		for(int i=0;i<nrspells;i++)
		{
			m_spellcheck[i] = false;
		}

		spells[0].info = dbcSpell.LookupEntry(MIND_REND);
		spells[0].targettype = TARGET_ATTACKING;
		spells[0].instant = false;
		spells[0].cooldown = -1;
		spells[0].perctrigger = 15.0f;
		spells[0].attackstoptimer = 1000;

		spells[1].info = dbcSpell.LookupEntry(FEAR);
		spells[1].targettype = TARGET_ATTACKING;
		spells[1].instant = false;
		spells[1].cooldown = -1;
		spells[1].perctrigger = 8.0f;
		spells[1].attackstoptimer = 1000;

		spells[2].info = dbcSpell.LookupEntry(DOMINATION);
		spells[2].targettype = TARGET_ATTACKING;
		spells[2].instant = false;
		spells[2].cooldown = -1;
		spells[2].perctrigger = 6.0f;
		spells[2].attackstoptimer = 1000;

		spells[3].info = dbcSpell.LookupEntry(SUMMON_ILLUSION_66);
		spells[3].targettype = TARGET_SELF;
		spells[3].instant = true;
		spells[3].cooldown = -1;
		spells[3].perctrigger = 0.0f;
		spells[3].attackstoptimer = 1000;

		spells[4].info = dbcSpell.LookupEntry(SUMMON_ILLUSION_33);
		spells[4].targettype = TARGET_SELF;
		spells[4].instant = true;
		spells[4].cooldown = -1;
		spells[4].perctrigger = 0.0f;
		spells[4].attackstoptimer = 1000;

    }
    
    void OnCombatStart(Unit* mTarget)
    {
		IllusionCount = 0;
		CastTime();
		RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
		_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Bear witness to the agent of your demise!");	// used when he kills Warden Mellichar
		_unit->PlaySoundToSet(11123);
    }

	void CastTime()
	{
		for(int i=0;i<nrspells;i++)
			spells[i].casttime = spells[i].cooldown;
	}

	void OnTargetDied(Unit* mTarget)
    {
		if (_unit->GetHealthPct() > 0)
		{
			int RandomSpeach;
			RandomSpeach=rand()%2;
			switch (RandomSpeach)
			{
			case 0:
				_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your fate is written!");
				_unit->PlaySoundToSet(11124);
				break;
			case 1:
				_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "The chaos I have sown here is but a taste....");
				_unit->PlaySoundToSet(11125);
				break;
			}
		}
    }

    void OnCombatStop(Unit* mTarget)
    {
		IllusionCount = 0;
		CastTime();
        _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
        _unit->GetAIInterface()->SetAIState(STATE_IDLE);
        RemoveAIUpdateEvent();
    }

    void OnDied(Unit* mKiller)
    {
		IllusionCount = 0;
		CastTime();
       RemoveAIUpdateEvent();
		_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I am merely one of... infinite multitudes.");
		_unit->PlaySoundToSet(11126);
    }

    void AIUpdate()
	{
		if (_unit->GetHealthPct() <= 66 && !IllusionCount)
		{
			IllusionCount = 1;
			_unit->CastSpell(_unit, spells[3].info, spells[3].instant);
			//_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "We span the universe, as countless as the stars!");
			_unit->PlaySoundToSet(11131);	// Idk if those texts shouldn't be told by clones and by org. so disabled MSG to make it harder to detect =P
		}

		if (_unit->GetHealthPct() <= 33 && IllusionCount == 1)
		{
			IllusionCount = 2;
			_unit->CastSpell(_unit, spells[4].info, spells[4].instant);
			//_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "We span the universe, as countless as the stars!");
			_unit->PlaySoundToSet(11131);
		}
		
		else
		{
			float val = (float)RandomFloat(100.0f);
			SpellCast(val);
		}
    }

	void FearSound()
	{
		int RandomFear;
		RandomFear=rand()%4;
		switch (RandomFear)
		{
		case 0:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Flee in terror.");
			_unit->PlaySoundToSet(11129);
			break;
		case 1:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I will show you horrors undreamed of.");
			_unit->PlaySoundToSet(11130);
			break;
		}
	}

	void DominationSound()
	{
		int RandomDomination;
		RandomDomination=rand()%4;
		switch (RandomDomination)
		{
		case 0:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You will do my bidding, weakling.");
			_unit->PlaySoundToSet(11127);
			break;
		case 1:
			_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Your will is no longer your own.");
			_unit->PlaySoundToSet(11128);
			break;
		}
	}

	void SpellCast(float val)
	{
        if(_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->GetNextTarget())
        {
			float comulativeperc = 0;
		    Unit* target = NULL;
			for(int i=0;i<nrspells;i++)
			{
				spells[i].casttime--;
				
				if (m_spellcheck[i])
				{					
					spells[i].casttime = spells[i].cooldown;
					target = _unit->GetAIInterface()->GetNextTarget();
					switch(spells[i].targettype)
					{
						case TARGET_SELF:
						case TARGET_VARIOUS:
							_unit->CastSpell(_unit, spells[i].info, spells[i].instant); break;
						case TARGET_ATTACKING:
							_unit->CastSpell(target, spells[i].info, spells[i].instant); break;
						case TARGET_DESTINATION:
							_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), spells[i].info, spells[i].instant); break;
					}

					if (spells[i].speech != "")
					{
						_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
						_unit->PlaySoundToSet(spells[i].soundid); 
					}

					if (m_spellcheck[1] == true)
					{
						FearSound();
					}

					if (m_spellcheck[2] == true)
					{
						DominationSound();
					}

					m_spellcheck[i] = false;
					return;
				}

				if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
				{
					_unit->setAttackTimer(spells[i].attackstoptimer, false);
					m_spellcheck[i] = true;
				}
				comulativeperc += spells[i].perctrigger;
			}
		}
	}

	void Destroy()
	{
		delete this;
	};

protected:

	uint32 IllusionCount;
	int nrspells;
};

#define CN_WARDEN_MELLICHAR 20904	

#define BLAZING_TRICKSTER 20905
#define WARP_STALKER 20906
#define AKKIRIS_LIGHTNING_WAKER 20908
#define SULFURON_MAGMA_THROWER 20909
#define TWILIGHT_DRAKONAAR 20910
#define BLACKWING_DRAKONAAR 20911
#define MILLHOUSE_MANASTORM 20977

class WardenMellicharAI : public MoonScriptCreatureAI
{
public:

	MOONSCRIPT_FACTORY_FUNCTION( WardenMellicharAI, MoonScriptCreatureAI );
	WardenMellicharAI( Creature* pCreature ) : MoonScriptCreatureAI( pCreature )
	{
		SetCanMove( false );
		Phase_Timer = -1;
		Phase = 0;
		Spawncounter = 0;
		NPC_orb1 = NULL;
		NPC_orb2 = NULL;
		NPC_orb3 = NULL;
		NPC_orb4 = NULL;
		NPC_orb5 = NULL;
		shield = NULL;
		orb1 = NULL;
		orb2 = NULL;
		orb3 = NULL;
		orb4 = NULL;
	}
	
	void OnCombatStart(Unit* mTarget)
	{
		Phase = 0;
		Phasepart = 0;
		SetCanMove(false);
		Phase_Timer = AddTimer(55000);

		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
		_unit->SetEmoteState(EMOTE_ONESHOT_READY1H); // to be replaced for the standstate
		
		shield = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 445.786f, -169.263f, 43.0466f, 184802 );
		if(shield)
			shield->SetByte(GAMEOBJECT_BYTES_1, 0, 1);
		
		RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
		
		_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I knew the prince would be angry but, I... I have not been myself. I had to let them out! The great one speaks to me, you see. Wait--outsiders. Kael'thas did not send you! Good... I'll just tell the prince you released the prisoners!");
		_unit->PlaySoundToSet(11222);
		sEventMgr.AddEvent(TO_UNIT(_unit), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_YELL, (uint32)LANG_UNIVERSAL, 
			"The naaru kept some of the most dangerous beings in existence here in these cells. Let me introduce you to another...", 
			EVENT_UNIT_CHAT_MSG, 27000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
		sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)11223, EVENT_UNK, 27000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
				
		ParentClass::OnCombatStart(mTarget);
	}

	void OnCombatStop(Unit* mTarget)
	{
		
		_unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
		_unit->GetAIInterface()->SetAIState(STATE_IDLE);
		RemoveAIUpdateEvent();
		
		Reset_Event();
				
		//_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Welcome, O great one. I am your humble servant");
		//_unit->PlaySoundToSet(11229);*/

	}
	
	void OnDied(Unit* mKiller)
	{
		
		RemoveAIUpdateEvent();
	}
	
	void AIUpdate()
	{
		_unit->SetUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
		SetCanMove(false);
		SetAllowMelee(false);
		SetAllowSpell(false);
		
		// ORB ONE
		if( IsTimerFinished(Phase_Timer) && Phase == 0)
		{
			if(Phasepart == 0)
			{
				Spawncounter = 0;
				orb1 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 482.929f, -151.114f, 43.654f, 183961 );
				if( orb1 )
					orb1->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
				
				switch( RandomUInt(1) )
				{
					NPC_ID_Spawn = 0;
					case 0:
						NPC_ID_Spawn = BLAZING_TRICKSTER;
						break;
					case 1:
						NPC_ID_Spawn = WARP_STALKER;
						break;
				}
				ResetTimer(Phase_Timer, 8000);
				Phasepart=1;
				return;
			}
			
			else if( Phasepart == 1 )
			{
				if ( !NPC_orb1 && NPC_ID_Spawn != 0 && Spawncounter == 0 )
				{
					++Spawncounter;
					NPC_orb1 = SpawnCreature(NPC_ID_Spawn, 475.672f, -147.086f, 42.567f, 3.184015f);
					return;
				}
				else if ( NPC_orb1 && !NPC_orb1->IsAlive() )
				{
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Yes, yes... another! Your will is mine! Behold another terrifying creature of incomprehensible power!");
					_unit->PlaySoundToSet(11224);
					Phase = 1;
					Phasepart = 0;
					ResetTimer(Phase_Timer, 6000);
					return;
				}
				else
				{
					return;
				}
				return;
			}
			//return;
		}
		
		// ORB TWO
		else if( IsTimerFinished(Phase_Timer) && Phase==1 )
		{
			if( Phasepart == 0 )
			{
				Spawncounter = 0;
				orb2 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 409.062f, -152.161f, 43.653f, 183963 );
				if(orb2)
					orb2->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
				
				ResetTimer(Phase_Timer, 8000);
				Phasepart=1;
				return;
			}
			
			else if( Phasepart == 1 )
			{
				if ( !NPC_orb2 && Spawncounter == 0 )
				{
					++Spawncounter;
					NPC_orb2 = SpawnCreature(MILLHOUSE_MANASTORM, 413.192f, -148.586f, 42.569f, 0.024347f);
					return;
				}
				else if ( NPC_orb2 && NPC_orb2->IsAlive() )
				{
					Unit* millhouse = TO_UNIT(ForceCreatureFind(MILLHOUSE_MANASTORM));
					if ( millhouse )
					{
						sEventMgr.AddEvent(TO_UNIT(millhouse), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_YELL, (uint32)LANG_UNIVERSAL,  
						"Where in Bonzo's brass buttons am I? And who are-- yaaghh, that's one mother of a headache!",
						EVENT_UNIT_CHAT_MSG, 2000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
						sEventMgr.AddEvent(TO_OBJECT(millhouse), &Object::PlaySoundToSet, (uint32)11171, EVENT_UNK, 2000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
						
						sEventMgr.AddEvent(TO_UNIT(_unit), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_YELL, (uint32)LANG_UNIVERSAL, 
						"What is this? A lowly gnome? I will do better, oh great one.", 
						EVENT_UNIT_CHAT_MSG, 13000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
						sEventMgr.AddEvent(TO_OBJECT(_unit), &Object::PlaySoundToSet, (uint32)11226, EVENT_UNK, 13000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
							
						sEventMgr.AddEvent(TO_UNIT(millhouse), &Unit::SendChatMessage, (uint8)CHAT_MSG_MONSTER_YELL, (uint32)LANG_UNIVERSAL,  
						"Lowly? Nobody refers to the mighty Millhouse Manastorm as lowly! I have no idea what goes on here, but I will gladly join your fight against this impudent imbecile!",
						EVENT_UNIT_CHAT_MSG, 22000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
						sEventMgr.AddEvent(TO_OBJECT(millhouse), &Object::PlaySoundToSet, (uint32)11172, EVENT_UNK, 22000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
					}
					Phase = 2;
					Phasepart = 0;
					ResetTimer(Phase_Timer, 25000);
					return;
				}
				else
				{
					return;
				}
				return;
				
			}
			//return;
		}
		
		// ORB THREE
		else if( IsTimerFinished(Phase_Timer) && Phase==2 )
		{
			if( Phasepart == 0 )
			{
				Spawncounter = 0;
				orb3 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 415.167f, -174.338f, 43.654f, 183964 );
				if( orb3 )
					orb3->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
				
				switch(RandomUInt(1))
				{
					NPC_ID_Spawn = 0;
					case 0:
						NPC_ID_Spawn = SULFURON_MAGMA_THROWER;
						break;
					case 1:
						NPC_ID_Spawn = AKKIRIS_LIGHTNING_WAKER;
						break;
				}
				ResetTimer(Phase_Timer, 8000);
				Phasepart = 1;
				return;
			}
			
			else if( Phasepart == 1 )
			{
				if ( !NPC_orb3 && NPC_ID_Spawn != 0 && Spawncounter == 0 )
				{
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "1");
					++Spawncounter;
					NPC_orb3 = SpawnCreature(NPC_ID_Spawn, 420.050f, -173.500f, 42.580f, 6.110f);
					return;
				}
				else if ( !NPC_orb3 )
				{
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "2");
					NPC_orb3 = GetNearestCreature(NPC_ID_Spawn);
				}
				else if ( NPC_orb3 && !NPC_orb3->IsAlive() )
				{
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Anarchy! Bedlam! Oh, you are so wise! Yes, I see it now, of course!");
					_unit->PlaySoundToSet(11227);
					Phase = 3;
					Phasepart = 0;
					ResetTimer(Phase_Timer, 8000);
					return;
				}
				else
				{
					return;
				}
				return;
			}
			//return;
		}
		
		// ORB FOUR
		else if( IsTimerFinished(Phase_Timer) && Phase==3)
		{
			if( Phasepart == 0 )
			{
				Spawncounter = 0;
				orb4 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords( 476.422f, -174.517f, 42.748f, 183962 );
				if( orb4 )
					orb4->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
				
				switch(RandomUInt(1))
				{
					NPC_ID_Spawn = 0;
					case 0:
						NPC_ID_Spawn = TWILIGHT_DRAKONAAR;
						break;
					case 1:
						NPC_ID_Spawn = BLACKWING_DRAKONAAR;
						break;
				}
				ResetTimer(Phase_Timer, 8000);
				Phasepart=1;
				return;
			}
			
			else if( Phasepart == 1 )
			{
				if ( !NPC_orb4 && NPC_ID_Spawn != 0 && Spawncounter == 0 )
				{
					++Spawncounter;
					NPC_orb4 = SpawnCreature(NPC_ID_Spawn, 471.153f, -174.715f, 42.589f, 3.097f);
					return;
				}
				else if ( !NPC_orb4 )
				{
					NPC_orb4 = GetNearestCreature(NPC_ID_Spawn);
				}
				else if ( NPC_orb4 && !NPC_orb4->IsAlive() )
				{
					_unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Yes, O great one, right away!");
					_unit->PlaySoundToSet(11228);
					Phase = 4;
					Phasepart = 0;
					ResetTimer(Phase_Timer, 6000);
					return;
				}
				else
				{
					return;
				}
				return;
			}
			//return;
		}
		
		else if( IsTimerFinished(Phase_Timer) && Phase==4 )
		{
				
		}
		
		ParentClass::AIUpdate();
		SetCanMove(false);
		SetAllowMelee(false);
		SetAllowSpell(false);
		
	}
	
	void Reset_Event()
	{
		SetCanMove(true);
		SetAllowMelee(true);
		SetAllowSpell(true);
		_unit->SetEmoteState(8); // to be replaced for the standstate
		
		if( shield )
			shield->SetByte(GAMEOBJECT_BYTES_1, 0, 0);
		
		if( orb1 )
			orb1->SetByte(GAMEOBJECT_BYTES_1, 0, 1);

		if( orb2 )
			orb2->SetByte(GAMEOBJECT_BYTES_1, 0, 1);
		
		if( orb3 )
			orb3->SetByte(GAMEOBJECT_BYTES_1, 0, 1);
		
		if( orb4 )
			orb4->SetByte(GAMEOBJECT_BYTES_1, 0, 1);
			
		if( NPC_orb1 )
		{
			NPC_orb1->Despawn(0);
			NPC_orb1 = NULL;
		}
		
		if( NPC_orb2 )
		{
			NPC_orb2->Despawn(0);
			NPC_orb2 = NULL;
		}
		
		if( NPC_orb3 )
		{
			NPC_orb3->Despawn(0);
			NPC_orb3 = NULL;
		}
		
		if( NPC_orb4 )
		{
			NPC_orb4->Despawn(0);
			NPC_orb4 = NULL;
		}
		
		if( NPC_orb5 )
		{
			NPC_orb5->Despawn(0);
			NPC_orb5 = NULL;
		}
		
	}

	void Destroy()
	{
		delete this;
	};
	
protected:

	uint32 Phase;
	uint32 Phasepart;
	uint32 NPC_ID_Spawn;
	uint32 Spawncounter;
	int32 Phase_Timer;
	
	MoonScriptCreatureAI*	NPC_orb1;
	MoonScriptCreatureAI*	NPC_orb2;
	MoonScriptCreatureAI*	NPC_orb3;
	MoonScriptCreatureAI*	NPC_orb4;
	MoonScriptCreatureAI*	NPC_orb5;
	GameObject* shield;
	GameObject* orb1;
	GameObject* orb2;
	GameObject* orb3;
	GameObject* orb4;
	
	
};

void SetupArcatraz(ScriptMgr * mgr)
{
	mgr->register_creature_script(CN_ZEREKETH, &ZerekethAI::Create);
	mgr->register_creature_script(CN_VOIDZONEARC, &VoidZoneARC::Create);

	mgr->register_creature_script(CN_DALLIAH_THE_DOOMSAYER, &DalliahTheDoomsayerAI::Create);
	mgr->register_creature_script(CN_WRATH_SCRYER_SOCCOTHRATES, &WrathScryerSoccothratesAI::Create);
	mgr->register_creature_script(CN_HARBRINGER_SKYRISS, &HarbringerSkyrissAI::Create);
	//mgr->register_creature_script(CN_WARDEN_MELLICHAR, &WardenMellicharAI::Create);
}
