/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef GTREGENMPPERSPTENTRYMETHODS_H
#define GTREGENMPPERSPTENTRYMETHODS_H

/***
 * Mana regeneration per point of Spirit, by class and level.
 *
 *     Player::OCTRegenMPPerSpirit() -> sqrt(Intellect) * Spirit * ratio
 *
 * Provides access to the DBC table `gtRegenMPPerSpt.dbc`. Rows for warrior,
 * rogue and death knight hold zero, which is the whole reason those classes
 * never regenerate mana: the product falls to zero whatever their Spirit.
 *
 * The row index is (class - 1) * 100 + (level - 1).
 *
 * The row carries no id of its own: the "df" format drops the index column,
 * so the struct holds nothing but the ratio. The index is what you looked the
 * row up by.
 *
 * Worth knowing: the client file holds a single field while the format
 * expects two, so DBCFileLoader rejects it outright and only the
 * gtregenmpperspt_dbc table ever reaches the store. Patching the .dbc changes
 * nothing; writing here is downstream of both.
 *
 * Inherits all methods from: none
 */
namespace LuaGtRegenMPPerSptEntry
{
    /**
     * Returns the mana regeneration ratio per point of Spirit.
     *
     * @return float ratio
     */
    int GetRatio(lua_State* L, GtRegenMPPerSptEntry* entry)
    {
        ALE::Push(L, entry->ratio);
        return 1;
    }

    /**
     * Sets the mana regeneration ratio per point of Spirit.
     *
     * The write lands in the store the core reads on every stat update, so it
     * takes effect on the next one.
     *
     * @param float ratio
     */
    int SetRatio(lua_State* L, GtRegenMPPerSptEntry* entry)
    {
        entry->ratio = ALE::CHECKVAL<float>(L, 2);
        return 0;
    }
}
#endif
