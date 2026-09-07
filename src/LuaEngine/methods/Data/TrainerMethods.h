/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef TRAINERMETHODS_H
#define TRAINERMETHODS_H

#include "Trainer.h"

/***
 * A trainer and what it teaches, from the world database tables `trainer` and
 * `trainer_spell`.
 *
 * Read on every visit by Trainer::SendSpells and Trainer::CanTeachSpell, so a
 * write here reaches the next player who talks to the npc.
 *
 * What cannot be done: adding or removing a spell, or moving one to another
 * trainer. ObjectMgr indexes trainers by class as it loads them and never
 * rebuilds that index.
 *
 * Inherits all methods from: none
 */
namespace LuaTrainer
{
    /**
     * Returns what kind of trainer this is.
     *
     * @return uint32 type : 0 = class, 1 = mount, 2 = tradeskill, 3 = pet
     */
    int GetType(lua_State* L, Trainer::Trainer* trainer)
    {
        ALE::Push(L, uint32(trainer->GetTrainerType()));
        return 1;
    }

    /**
     * Returns what the trainer requires. For a class trainer this is the
     * class it teaches, 1 = warrior ... 11 = druid.
     *
     * @return uint32 requirement
     */
    int GetRequirement(lua_State* L, Trainer::Trainer* trainer)
    {
        ALE::Push(L, trainer->GetTrainerRequirement());
        return 1;
    }

    /**
     * Returns how many spells the trainer teaches.
     *
     * @return uint32 count
     */
    int GetSpellCount(lua_State* L, Trainer::Trainer* trainer)
    {
        ALE::Push(L, uint32(trainer->GetSpells().size()));
        return 1;
    }

    /**
     * Returns every spell the trainer teaches, as a table of
     * [TrainerSpell]. The same spell is taught by every trainer of a class,
     * once per city and race, so a walk hands it out many times - folding the
     * duplicates is the caller's job, since which key to fold on depends on
     * what is being built.
     *
     * @return table spells : { [1] = [TrainerSpell], ... }
     */
    int GetSpells(lua_State* L, Trainer::Trainer* trainer)
    {
        lua_newtable(L);
        int table = lua_gettop(L);
        uint32 index = 0;

        for (Trainer::Spell const& spell : trainer->GetSpells())
        {
            ALE::Push(L, ++index);
            ALE::Push(L, &spell);
            lua_settable(L, table);
        }

        lua_settop(L, table);
        return 1;
    }

    /**
     * Returns one taught spell by its spell id, or nil.
     *
     * @param uint32 spellId
     * @return [TrainerSpell] spell
     */
    int GetSpell(lua_State* L, Trainer::Trainer* trainer)
    {
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, trainer->GetSpell(spellId));
        return 1;
    }
}

/***
 * One spell taught by a trainer, from `trainer_spell`.
 *
 * Inherits all methods from: none
 */
namespace LuaTrainerSpell
{
    /**
     * Returns the taught spell.
     *
     * @return uint32 spellId
     */
    int GetSpellId(lua_State* L, Trainer::Spell* spell)
    {
        ALE::Push(L, spell->SpellId);
        return 1;
    }

    /**
     * @return uint32 moneyCost : in copper
     */
    int GetMoneyCost(lua_State* L, Trainer::Spell* spell)
    {
        ALE::Push(L, spell->MoneyCost);
        return 1;
    }

    /**
     * @param uint32 moneyCost : in copper, 0 to teach for free
     */
    int SetMoneyCost(lua_State* L, Trainer::Spell* spell)
    {
        spell->MoneyCost = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 reqSkillLine : 0 when no profession is needed
     */
    int GetReqSkillLine(lua_State* L, Trainer::Spell* spell)
    {
        ALE::Push(L, spell->ReqSkillLine);
        return 1;
    }

    /**
     * @param uint32 reqSkillLine
     */
    int SetReqSkillLine(lua_State* L, Trainer::Spell* spell)
    {
        spell->ReqSkillLine = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 reqSkillRank
     */
    int GetReqSkillRank(lua_State* L, Trainer::Spell* spell)
    {
        ALE::Push(L, spell->ReqSkillRank);
        return 1;
    }

    /**
     * @param uint32 reqSkillRank
     */
    int SetReqSkillRank(lua_State* L, Trainer::Spell* spell)
    {
        spell->ReqSkillRank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns the spell that must already be known, or nil when the slot is
     * out of range.
     *
     * @param uint32 slot : 1 to 3
     * @return uint32 reqAbility : 0 when the slot requires nothing
     */
    int GetReqAbility(lua_State* L, Trainer::Spell* spell)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (slot < 1 || slot > spell->ReqAbility.size())
            return 0;

        ALE::Push(L, spell->ReqAbility[slot - 1]);
        return 1;
    }

    /**
     * @param uint32 slot : 1 to 3
     * @param uint32 reqAbility : 0 to require nothing
     */
    int SetReqAbility(lua_State* L, Trainer::Spell* spell)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 3);

        if (slot < 1 || slot > spell->ReqAbility.size())
        {
            luaL_argerror(L, 2, "required ability slot out of range");
            return 0;
        }

        spell->ReqAbility[slot - 1] = spellId;
        return 0;
    }

    /**
     * @return uint32 reqLevel : 0 when the spell has no level requirement
     */
    int GetReqLevel(lua_State* L, Trainer::Spell* spell)
    {
        ALE::Push(L, uint32(spell->ReqLevel));
        return 1;
    }

    /**
     * ReqLevel is a uint8 in the core, so anything above 255 is refused
     * rather than silently wrapped.
     *
     * @param uint32 reqLevel
     */
    int SetReqLevel(lua_State* L, Trainer::Spell* spell)
    {
        uint32 level = ALE::CHECKVAL<uint32>(L, 2);
        if (level > 255)
        {
            luaL_argerror(L, 2, "required level above the uint8 the core stores it in");
            return 0;
        }

        spell->ReqLevel = uint8(level);
        return 0;
    }
}
#endif
