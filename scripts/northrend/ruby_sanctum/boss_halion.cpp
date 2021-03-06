/* Copyright (C) 2010 /dev/rsa for ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: boss_halion
SD%Complete: 
SDComment: ToDo: implement corporeallity
SDCategory: ruby_sanctum
EndScriptData */

#include "precompiled.h"
#include "ruby_sanctum.h"

enum 
{
	//yells
    SAY_SPAWN					= -1666100,
    SAY_AGGRO					= -1666101,
    SAY_SLAY1					= -1666102,
    SAY_SLAY2					= -1666103,
    SAY_DEATH					= -1666104,
    SAY_BERSERK					= -1666105,
    SAY_SPECIAL1				= -1666106,
    SAY_SPECIAL2				= -1666107,
    SAY_PHASE2					= -1666108,
    SAY_PHASE3					= -1666109,

	// all phases
    SPELL_TWILIGHT_PRECISION            = 78243, 
    SPELL_BERSERK                       = 26663,
	SPELL_CLEAVE						= 74524,
	SPELL_TAIL_LASH						= 74531,

	// real realm
	SPELL_FLAME_BREATH					= 74525,
	SPELL_METEOR_STRIKE					= 75877,
	SPELL_FIERY_COMBUSTION				= 74562,
	SPELL_COMBUSTION_MARK				= 74567,
	SPELL_COMBUSTION_TRIGG				= 74629,
	SPELL_COMBUSTION_SUMMON				= 74610,
	SPELL_COMBUSTION_EXPL				= 74607,
	NPC_COMBUSTION                      = 40001,

	// twilight realm
	SPELL_DARK_BREATH					= 74806,
	SPELL_DUSK_SHROUD					= 75484,
	SPELL_SOUL_CONSUMPTION				= 74792,
	SPELL_CONSUMPTON_TRIGG				= 74803,	// leaves a shadow on the ground
	SPELL_CONSUMPTON_SUMMON				= 74800,	
	SPELL_CONSUMTION_EXPL				= 74799,
	SPELL_CONSUMTION_MARK				= 74795,
	NPC_CONSUMPTION                     = 40135,

	// twilight auras
	SPELL_TWILIGHT_PHASING              = 74808,	// start phase 2
	SPELL_TWILIGHT_CUTTER				= 74768,
	SPELL_TWILIGHT_DIVISION             = 75063,	// start phase 3
    SPELL_TWILIGHT_REALM                = 74807,

	// meteor
	SPELL_METEOR_DMG					= 75877,	// crashes in flames
	SPELL_METEOR_SUMMON					= 74637,	// summons the meteor
	SPELL_METEOR_AURA					= 74641,	// marks the impact zone
	SPELl_METEOR_INFERNO				= 75879,	// on heroic instead of dmg?
	SPELL_METEOR_FLAME					= 74718,	// aura for the ground flames

	// meteor npcs
	NPC_METEOR_STRIKE                   = 40029,
	NPC_METEORFLAME                     = 36673,	//meteor flame FIX THIS!!
	NPC_METEOR_STRIKE_1                 = 40041,
    NPC_METEOR_STRIKE_2                 = 40042,

	// corporeality
    SPELL_CORPOREALITY_EVEN             = 74826, // Deals & receives normal damage
    SPELL_CORPOREALITY_20I              = 74827, // Damage dealt increased by 10% & Damage taken increased by 15%
    SPELL_CORPOREALITY_40I              = 74828, // Damage dealt increased by 30% & Damage taken increased by 50%
    SPELL_CORPOREALITY_60I              = 74829, // Damage dealt increased by 60% & Damage taken increased by 100%
    SPELL_CORPOREALITY_80I              = 74830, // Damage dealt increased by 100% & Damage taken increased by 200%
    SPELL_CORPOREALITY_100I             = 74831, // Damage dealt increased by 200% & Damage taken increased by 400%
    SPELL_CORPOREALITY_20D              = 74832, // Damage dealt reduced by 10% & Damage taken reduced by 15%
    SPELL_CORPOREALITY_40D              = 74833, // Damage dealt reduced by 30% & Damage taken reduced by 50%
    SPELL_CORPOREALITY_60D              = 74834, // Damage dealt reduced by 60% & Damage taken reduced by 100%
    SPELL_CORPOREALITY_80D              = 74835, // Damage dealt reduced by 100% & Damage taken reduced by 200%
    SPELL_CORPOREALITY_100D             = 74836, // Damage dealt reduced by 200% & Damage taken reduced by 400%     
};

enum phases
{
	PHASE_IDLE		= 0,
	PHASE_REAL		= 1,
	PHASE_TWILIGHT	= 2,
	PHASE_DUAL		= 3,
};

/*######
## boss_halion_real (Physical version)
######*/
struct MANGOS_DLL_DECL boss_halion_realAI : public ScriptedAI
{
    boss_halion_realAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
		/// temp disabled, remove for debug
		pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		m_bHasTaunted = false;
        Reset();
    }

    ScriptedInstance* m_pInstance;

	bool m_bHasTaunted;

	uint32 m_uiBerserkTimer;
	uint32 m_uiCleaveTimer;
	uint32 m_uiTailLashTimer;
	uint32 m_uiCombustionTimer;
	uint32 m_uiMeteorTimer;
	uint32 m_uiFlameBreathTimer;

    void Reset()
    {
		m_uiBerserkTimer		= 600000;
		m_uiCleaveTimer			= 5000;
		m_uiTailLashTimer		= 10000;
		m_uiCombustionTimer		= 15000;
		m_uiMeteorTimer			= 20000;
		m_uiFlameBreathTimer	= 15000;

		if(m_pInstance)
			m_pInstance->SetData(TYPE_HALION_PHASE, PHASE_IDLE);
    }

    void MoveInLineOfSight(Unit* pWho) 
    {
		if (!m_bHasTaunted && m_creature->IsWithinDistInMap(pWho, 60.0f))
        {
			DoScriptText(SAY_SPAWN, m_creature);
			m_bHasTaunted = true;
		}
    }

    void JustReachedHome()
    {
		if(m_pInstance)
			m_pInstance->SetData(TYPE_HALION, FAIL);

		if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_TWILIGHT)))
			pTarget->AI()->EnterEvadeMode();
    }

    void JustDied(Unit* pKiller)
	{
		DoScriptText(SAY_DEATH, m_creature);
		if(m_pInstance)
			m_pInstance->SetData(TYPE_HALION, DONE);
	}

	void KilledUnit(Unit* pVictim)
	{
		switch (urand(0,1))
		{
		case 0:
			DoScriptText(SAY_SLAY1, m_creature, pVictim);
			break;
		case 1:
			DoScriptText(SAY_SLAY2, m_creature, pVictim);
			break;
		}
	}

    void Aggro(Unit* pWho)
    {
		if(m_pInstance)
		{
			m_pInstance->SetData(TYPE_HALION, IN_PROGRESS);  
			m_pInstance->SetData(TYPE_HALION_PHASE, PHASE_REAL);

			if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_TWILIGHT)))
				pTarget->SetInCombatWithZone();
		}

		DoCast(m_creature, SPELL_TWILIGHT_PRECISION);

        DoScriptText(SAY_AGGRO, m_creature);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
		if(m_pInstance->GetData(TYPE_HALION_PHASE) == PHASE_TWILIGHT && pDoneBy->GetTypeId() == TYPEID_PLAYER)
		{
			uiDamage = 0;
			return;
		}

		// split damage to the twilight version
		if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_TWILIGHT)))
			m_creature->DealDamage(pTarget, uiDamage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

		if(m_creature->GetHealthPercent() < 75.0f && m_pInstance->GetData(TYPE_HALION_PHASE) == PHASE_REAL)
		{
			m_pInstance->SetData(TYPE_HALION_PHASE, PHASE_TWILIGHT);
			//DoCast(m_creature, SPELL_TWILIGHT_PHASING);
			// summon some portals
			if(GameObject* pPortal = m_creature->GetMap()->GetGameObject(m_pInstance->GetData64(GO_HALION_PORTAL_1)))
				m_pInstance->DoRespawnGameObject(pPortal->GetGUID());
		}
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

		switch(m_pInstance->GetData(TYPE_HALION_PHASE))
		{
		case PHASE_DUAL:
			// no break because he uses the same spells
		case PHASE_REAL:

			if(m_uiCleaveTimer < uiDiff)
			{
				DoCast(m_creature->getVictim(), SPELL_CLEAVE);
				m_uiCleaveTimer = urand(5000, 7000);
			}
			else m_uiCleaveTimer -= uiDiff;

			if(m_uiTailLashTimer < uiDiff)
			{
				DoCast(m_creature, SPELL_TAIL_LASH);
				m_uiTailLashTimer = urand(10000, 13000);
			}
			else m_uiTailLashTimer -= uiDiff;

			if(m_uiFlameBreathTimer < uiDiff)
			{
				DoCast(m_creature, SPELL_FLAME_BREATH);
				m_uiFlameBreathTimer = urand(6000, 9000);
			}
			else m_uiFlameBreathTimer -= uiDiff;

			if(m_uiCombustionTimer < uiDiff)
			{
				m_creature->InterruptNonMeleeSpells(true);
				if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
					DoCast(pTarget, SPELL_FIERY_COMBUSTION);
				m_uiCombustionTimer = 25000;
			}
			else m_uiCombustionTimer -= uiDiff;

			if(m_uiMeteorTimer < uiDiff)
			{
				//DoCast(m_creature, SPELL_METEOR_SUMMON);
				float angle = (float) rand()*360/RAND_MAX + 1;
                float homeX = 3155.19f + urand(15, 35)*cos(angle*(M_PI/180));
                float homeY = 538.71f + urand(15, 35)*sin(angle*(M_PI/180));
				m_creature->SummonCreature(NPC_METEOR_STRIKE, homeX, homeY, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000);
				m_uiMeteorTimer = 30000;
			}
			else m_uiMeteorTimer -= uiDiff;

			break;
		case PHASE_TWILIGHT:
			// no spells here
			break;
		}

		if(m_uiBerserkTimer < uiDiff)
		{
			m_creature->InterruptNonMeleeSpells(true);
			DoCast(m_creature, SPELL_BERSERK);
			if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_TWILIGHT)))
				pTarget->CastSpell(pTarget, SPELL_BERSERK, false);
			m_uiBerserkTimer = 30000;
		}
		else m_uiBerserkTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_halion_real(Creature* pCreature)
{
    return new boss_halion_realAI(pCreature);
}

/*######
## boss_halion_twilight (Twilight version)
######*/
struct MANGOS_DLL_DECL boss_halion_twilightAI : public ScriptedAI
{
    boss_halion_twilightAI(Creature* pCreature) : ScriptedAI(pCreature) 
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

	uint32 m_uiCleaveTimer;
	uint32 m_uiTailLashTimer;
	uint32 m_uiConsumtionTimer;
	uint32 m_uiFlameBreathTimer;

    void Reset() 
    {
		m_uiCleaveTimer			= 5000;
		m_uiTailLashTimer		= 10000;
		m_uiConsumtionTimer		= 20000;
		m_uiFlameBreathTimer	= 15000;
		DoCast(m_creature, SPELL_TWILIGHT_REALM);
    }

    void JustReachedHome()
    {
		if(m_pInstance)
			m_pInstance->SetData(TYPE_HALION, NOT_STARTED);

		DoCast(m_creature, SPELL_TWILIGHT_REALM);

		if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_REAL)))
			pTarget->AI()->EnterEvadeMode();
    }

	void Aggro(Unit* pWho)
    {
		DoCast(m_creature, SPELL_TWILIGHT_PRECISION);
    }

    void KilledUnit(Unit* pVictim)
    {
        switch (urand(0,1))
		{
		case 0:
			DoScriptText(SAY_SLAY1, m_creature, pVictim);
			break;
		case 1:
			DoScriptText(SAY_SLAY2, m_creature, pVictim);
			break;
		}
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
		if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_REAL)))
			m_creature->DealDamage(pTarget, uiDamage, NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);

		if(m_creature->GetHealthPercent() < 50.0f && m_pInstance->GetData(TYPE_HALION_PHASE) == PHASE_TWILIGHT)
		{
			m_creature->InterruptNonMeleeSpells(true);
			m_pInstance->SetData(TYPE_HALION_PHASE, PHASE_DUAL);
			DoCast(m_creature, SPELL_TWILIGHT_DIVISION);

			if(Creature* pTarget = m_creature->GetMap()->GetCreature(m_pInstance->GetData64(NPC_HALION_REAL)))
				pTarget->RemoveAurasDueToSpell(SPELL_TWILIGHT_PHASING);

			if(GameObject* pPortal = m_creature->GetMap()->GetGameObject(m_pInstance->GetData64(GO_HALION_PORTAL_2)))
				m_pInstance->DoRespawnGameObject(pPortal->GetGUID());
			if(GameObject* pPortal = m_creature->GetMap()->GetGameObject(m_pInstance->GetData64(GO_HALION_PORTAL_3)))
				m_pInstance->DoRespawnGameObject(pPortal->GetGUID());

			if(m_pInstance)
			{
				m_pInstance->DoUpdateWorldState(UPDATE_STATE_UI_SHOW, 1);
				m_pInstance->DoUpdateWorldState(UPDATE_STATE_UI_COUNT, 100);	// fix this!
			}
		}
	}

	void UpdateAI(const uint32 uiDiff)
	{
		switch(m_pInstance->GetData(TYPE_HALION_PHASE))
		{
		case PHASE_REAL:
			// do nothing
			break;
		case PHASE_DUAL:
			// no break
		case PHASE_TWILIGHT:

			if(!m_creature->HasAura(SPELL_DUSK_SHROUD, EFFECT_INDEX_0))
				DoCast(m_creature, SPELL_DUSK_SHROUD);

			if(m_uiCleaveTimer < uiDiff)
			{
				DoCast(m_creature->getVictim(), SPELL_CLEAVE);
				m_uiCleaveTimer = urand(5000, 7000);
			}
			else m_uiCleaveTimer -= uiDiff;

			if(m_uiTailLashTimer < uiDiff)
			{
				DoCast(m_creature, SPELL_TAIL_LASH);
				m_uiTailLashTimer = urand(10000, 13000);
			}
			else m_uiTailLashTimer -= uiDiff;

			if(m_uiFlameBreathTimer < uiDiff)
			{
				DoCast(m_creature, SPELL_DARK_BREATH);
				m_uiFlameBreathTimer = urand(6000, 9000);
			}
			else m_uiFlameBreathTimer -= uiDiff;

			if(m_uiConsumtionTimer < uiDiff)
			{
				m_creature->InterruptNonMeleeSpells(true);
				if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
					DoCast(pTarget, SPELL_SOUL_CONSUMPTION);
				m_uiConsumtionTimer = 25000;
			}
			else m_uiConsumtionTimer -= uiDiff;

			DoMeleeAttackIfReady();

			break;
		}
	}
};

CreatureAI* GetAI_boss_halion_twilight(Creature* pCreature)
{
    return new boss_halion_twilightAI(pCreature);
}

/*######
## halion_meteor
######*/
struct MANGOS_DLL_DECL mob_halion_meteorAI : public ScriptedAI
{
	mob_halion_meteorAI(Creature *pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
		Reset();
	}

	ScriptedInstance* m_pInstance;

	uint32 m_uiExplodeTimer;

	void Reset()
	{
		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		DoCast(m_creature, SPELL_METEOR_AURA);
		m_uiExplodeTimer = 5000;
	}

	void AttackStart(Unit *who)
	{
		return;
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if(m_uiExplodeTimer < uiDiff)
		{
			DoCast(m_creature, SPELL_METEOR_DMG);
			m_uiExplodeTimer = 60000;
		}
		else m_uiExplodeTimer -= uiDiff;
	}
};

CreatureAI* GetAI_mob_halion_meteor(Creature* pCreature)
{
    return new mob_halion_meteorAI(pCreature);
}

/*######
## halion_flame
######*/
struct MANGOS_DLL_DECL mob_halion_flameAI : public ScriptedAI
{
    mob_halion_flameAI(Creature *pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
		pCreature->setFaction(14);
        Reset();
    }

    ScriptedInstance* m_pInstance;

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		DoCast(m_creature, SPELL_METEOR_FLAME);
		m_creature->SetRespawnDelay(DAY);
    }

    void AttackStart(Unit *who)
    {
        return;
    }

    void UpdateAI(const uint32 uiDiff)
    {
		if (m_pInstance->GetData(TYPE_HALION) != IN_PROGRESS)
            m_creature->ForcedDespawn();
    }
};

CreatureAI* GetAI_mob_halion_flame(Creature* pCreature)
{
    return new mob_halion_flameAI(pCreature);
}

/*######
## halion_orbs
######*/
struct MANGOS_DLL_DECL mob_halion_orbAI : public ScriptedAI
{
	mob_halion_orbAI(Creature *pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
		pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
		pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
		SetCombatMovement(false);
		Reset();
	}

	ScriptedInstance* m_pInstance;
	
	uint32 m_uiCutterTimer;

	void Reset()
	{
		m_uiCutterTimer = 50000;
	}

	void AttackStart(Unit *who)
	{
		return;
	}

	void UpdateAI(const uint32 uiDiff)
	{
		if(m_pInstance->GetData(TYPE_HALION_PHASE) != PHASE_REAL)
		{
			if(m_uiCutterTimer < uiDiff)
			{
				//DoScriptText(EMOTE_WARNING, m_creature);
				// implement cutting event here
				//DoCast(m_creature, SPELL_TWILIGHT_CUTTER):
				m_uiCutterTimer = 30000;
			}
			else m_uiCutterTimer -= uiDiff;
		}
	}
};
CreatureAI* GetAI_mob_halion_orb(Creature* pCreature)
{
    return new mob_halion_orbAI(pCreature);
}

bool GOHello_go_halion_portal(Player* pPlayer, GameObject* pGo)
{
	if (pPlayer->HasAura(SPELL_TWILIGHT_REALM, EFFECT_INDEX_0))
		pPlayer->RemoveAurasDueToSpell(SPELL_TWILIGHT_REALM);
	else
		pPlayer->CastSpell(pPlayer, SPELL_TWILIGHT_REALM, false);

	return true;
}

/*######
## halion_coltroller
######*/
struct MANGOS_DLL_DECL mob_halion_controlAI : public ScriptedAI
{
	mob_halion_controlAI(Creature* pCreature) : ScriptedAI(pCreature)
	{
		m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
		Reset();
	}
	ScriptedInstance* m_pInstance;

	void Reset() 
	{
	}

	void AttackStart(Unit *who)
	{
		return;
	}

	void UpdateAI(const uint32 uiDiff)
	{
	}
};

CreatureAI* GetAI_mob_halion_control(Creature* pCreature)
{
    return new mob_halion_controlAI(pCreature);
};

void AddSC_boss_halion()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_halion_real";
    newscript->GetAI = &GetAI_boss_halion_real;
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "boss_halion_twilight";
	newscript->GetAI = &GetAI_boss_halion_twilight;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_halion_meteor";
	newscript->GetAI = &GetAI_mob_halion_meteor;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_halion_flame";
	newscript->GetAI = &GetAI_mob_halion_flame;
	newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_halion_orb";
	newscript->GetAI = &GetAI_mob_halion_orb;
	newscript->RegisterSelf();

	newscript = new Script;
    newscript->pGOHello = &GOHello_go_halion_portal;
    newscript->Name = "go_halion_portal";
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name = "mob_halion_control";
	newscript->GetAI = &GetAI_mob_halion_control;
	newscript->RegisterSelf();
}
