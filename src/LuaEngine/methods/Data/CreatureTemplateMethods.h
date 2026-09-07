/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef CREATURETEMPLATEMETHODS_H
#define CREATURETEMPLATEMETHODS_H

#include "CreatureData.h"

/***
 * Creature data from the world database table `creature_template`.
 *
 * Describes the creature TYPE, not a creature standing in the world.
 * ObjectMgr::GetCreatureTemplate reads the store on every use, but a creature
 * copies what it needs from the template when it is created: a change here
 * reaches the ones spawned AFTER it, not those already on their feet.
 *
 * The client keeps its own creature cache, so a player who has already seen
 * one keeps the old name and subname until their cache is cleared.
 *
 * Inherits all methods from: none
 */
namespace LuaCreatureTemplate
{
    /**
     * Returns the creature entry.
     *
     * @return uint32 entry
     */
    int GetEntry(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->Entry);
        return 1;
    }

    /**
     * @return string name
     */
    int GetName(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->Name);
        return 1;
    }

    /**
     * @param string name
     */
    int SetName(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->Name = ALE::CHECKVAL<std::string>(L, 2);
        return 0;
    }

    /**
     * Returns the title shown under the name, such as "Innkeeper".
     *
     * @return string subName
     */
    int GetSubName(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->SubName);
        return 1;
    }

    /**
     * @param string subName
     */
    int SetSubName(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->SubName = ALE::CHECKVAL<std::string>(L, 2);
        return 0;
    }

    /**
     * Returns the lowest level the creature spawns at.
     *
     * @return uint32 minLevel
     */
    int GetMinLevel(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, uint32(creatureTemplate->minlevel));
        return 1;
    }

    /**
     * minlevel is a uint8 in the core, so anything above 255 is refused
     * rather than silently wrapped.
     *
     * @param uint32 minLevel
     */
    int SetMinLevel(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        uint32 level = ALE::CHECKVAL<uint32>(L, 2);
        if (level > 255)
        {
            luaL_argerror(L, 2, "level above the uint8 the core stores it in");
            return 0;
        }

        creatureTemplate->minlevel = uint8(level);
        return 0;
    }

    /**
     * @return uint32 maxLevel
     */
    int GetMaxLevel(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, uint32(creatureTemplate->maxlevel));
        return 1;
    }

    /**
     * @param uint32 maxLevel
     */
    int SetMaxLevel(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        uint32 level = ALE::CHECKVAL<uint32>(L, 2);
        if (level > 255)
        {
            luaL_argerror(L, 2, "level above the uint8 the core stores it in");
            return 0;
        }

        creatureTemplate->maxlevel = uint8(level);
        return 0;
    }

    /**
     * @return uint32 faction
     */
    int GetFaction(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->faction);
        return 1;
    }

    /**
     * @param uint32 faction
     */
    int SetFaction(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->faction = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns what the creature offers: vendor, trainer, quest giver and so
     * on. A mask of NPCFlags.
     *
     * @return uint32 npcFlag
     */
    int GetNpcFlag(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->npcflag);
        return 1;
    }

    /**
     * @param uint32 npcFlag
     */
    int SetNpcFlag(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->npcflag = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 unitFlags : mask of UnitFlags
     */
    int GetUnitFlags(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->unit_flags);
        return 1;
    }

    /**
     * @param uint32 unitFlags
     */
    int SetUnitFlags(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->unit_flags = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns how tough the creature is billed as.
     *
     * @return uint32 rank : 0 normal, 1 elite, 2 rare elite, 3 boss, 4 rare
     */
    int GetRank(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->rank);
        return 1;
    }

    /**
     * @param uint32 rank
     */
    int SetRank(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->rank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 type : enum CreatureType, 7 = humanoid, 6 = undead ...
     */
    int GetType(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->type);
        return 1;
    }

    /**
     * @param uint32 type
     */
    int SetType(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->type = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 family : enum CreatureFamily, for beasts and pets
     */
    int GetFamily(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->family);
        return 1;
    }

    /**
     * @param uint32 family
     */
    int SetFamily(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->family = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 unitClass : 1 warrior, 2 paladin, 4 rogue, 8 mage
     */
    int GetUnitClass(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->unit_class);
        return 1;
    }

    /**
     * Only four classes exist for creatures, and the class drives the base
     * stats the core reads out of creature_classlevelstats.
     *
     * @param uint32 unitClass : 1 warrior, 2 paladin, 4 rogue, 8 mage
     */
    int SetUnitClass(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->unit_class = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns the multiplier applied to the health the creature draws from
     * creature_classlevelstats.
     *
     * @return float modHealth
     */
    int GetModHealth(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->ModHealth);
        return 1;
    }

    /**
     * @param float modHealth
     */
    int SetModHealth(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->ModHealth = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return float modMana
     */
    int GetModMana(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->ModMana);
        return 1;
    }

    /**
     * @param float modMana
     */
    int SetModMana(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->ModMana = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return float modArmor
     */
    int GetModArmor(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->ModArmor);
        return 1;
    }

    /**
     * @param float modArmor
     */
    int SetModArmor(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->ModArmor = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return float modExperience : multiplier on the experience given
     */
    int GetModExperience(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->ModExperience);
        return 1;
    }

    /**
     * @param float modExperience
     */
    int SetModExperience(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->ModExperience = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return float damageModifier
     */
    int GetDamageModifier(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->DamageModifier);
        return 1;
    }

    /**
     * @param float damageModifier
     */
    int SetDamageModifier(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->DamageModifier = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return uint32 baseAttackTime : in milliseconds
     */
    int GetBaseAttackTime(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->BaseAttackTime);
        return 1;
    }

    /**
     * @param uint32 baseAttackTime : in milliseconds
     */
    int SetBaseAttackTime(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->BaseAttackTime = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 rangeAttackTime : in milliseconds
     */
    int GetRangeAttackTime(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->RangeAttackTime);
        return 1;
    }

    /**
     * @param uint32 rangeAttackTime : in milliseconds
     */
    int SetRangeAttackTime(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->RangeAttackTime = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 lootId : 0 when the creature drops nothing
     */
    int GetLootId(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->lootid);
        return 1;
    }

    /**
     * @param uint32 lootId : 0 to drop nothing
     */
    int SetLootId(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->lootid = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 minGold : in copper
     */
    int GetMinGold(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->mingold);
        return 1;
    }

    /**
     * @param uint32 minGold : in copper
     */
    int SetMinGold(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->mingold = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 maxGold : in copper
     */
    int GetMaxGold(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->maxgold);
        return 1;
    }

    /**
     * @param uint32 maxGold : in copper
     */
    int SetMaxGold(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->maxgold = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return float speedWalk
     */
    int GetSpeedWalk(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->speed_walk);
        return 1;
    }

    /**
     * @param float speedWalk
     */
    int SetSpeedWalk(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->speed_walk = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * @return float speedRun
     */
    int GetSpeedRun(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->speed_run);
        return 1;
    }

    /**
     * @param float speedRun
     */
    int SetSpeedRun(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->speed_run = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }

    /**
     * Returns whether the creature regenerates health out of combat.
     *
     * @return bool regenHealth
     */
    int GetRegenHealth(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->RegenHealth);
        return 1;
    }

    /**
     * @param bool regenHealth
     */
    int SetRegenHealth(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->RegenHealth = ALE::CHECKVAL<bool>(L, 2);
        return 0;
    }

    /**
     * @return uint32 flagsExtra : mask of CreatureFlagsExtra
     */
    int GetFlagsExtra(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->flags_extra);
        return 1;
    }

    /**
     * @param uint32 flagsExtra
     */
    int SetFlagsExtra(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->flags_extra = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns the name of the C++ AI driving the creature, empty when it has
     * none.
     *
     * @return string aiName
     */
    int GetAIName(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->AIName);
        return 1;
    }

    /**
     * @return uint32 movementType : 0 idle, 1 random, 2 waypoint
     */
    int GetMovementType(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        ALE::Push(L, creatureTemplate->MovementType);
        return 1;
    }

    /**
     * @param uint32 movementType : 0 idle, 1 random, 2 waypoint
     */
    int SetMovementType(lua_State* L, CreatureTemplate* creatureTemplate)
    {
        creatureTemplate->MovementType = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }
}
#endif
