/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef FORGE_GTREGENMPPERSPT_METHODS_H
#define FORGE_GTREGENMPPERSPT_METHODS_H

#include "ForgeTypes.h"

/*
 * gtRegenMPPerSpt - mana regen ratio per point of Spirit, by class and level.
 *
 *     Player::OCTRegenMPPerSpirit() -> sqrt(Intellect) * Spirit * ratio
 *
 * Rows for warrior, rogue and death knight are zero, which is why those
 * classes never regenerate mana. Index is (class - 1) * 100 + (level - 1).
 *
 * Consumption: runtime. LookupEntry runs on every stat update.
 *
 * The client .dbc holds a single field while the "df" format expects two, so
 * DBCFileLoader rejects the file outright and only the gtregenmpperspt_dbc
 * table is ever loaded. Forge writes to the store itself, past both.
 */
class ForgeGtRegenMPPerSptRow
{
public:
    ForgeGtRegenMPPerSptRow(uint32 id, GtRegenMPPerSptEntry const* source)
        : _id(id), _dirty(false), _ratio(source->ratio) { }

    uint32 GetID() const { return _id; }
    float GetData() const { return _ratio; }
    bool IsDirty() const { return _dirty; }

    void SetData(float value) { _ratio = value; _dirty = true; }

    // Writes into the live row rather than calling DBCStorage::SetEntry:
    // SetEntry deletes the old entry, but loaded rows all point into one
    // contiguous block, so freeing them individually corrupts the heap.
    bool Push()
    {
        if (!_dirty)
            return false;

        GtRegenMPPerSptEntry const* live = sGtRegenMPPerSptStore.LookupEntry(_id);
        if (!live)
            return false;

        const_cast<GtRegenMPPerSptEntry*>(live)->ratio = _ratio;
        _dirty = false;
        return true;
    }

private:
    uint32 _id;
    bool _dirty;
    float _ratio;
};

/*
 * The store handed to a database event handler. Holds no state of its own:
 * every call goes straight to sGtRegenMPPerSptStore.
 */
class ForgeGtRegenMPPerSpt
{
public:
    static uint32 const LEVELS = 100;
    static uint32 const CLASSES = 11;

    static ForgeGtRegenMPPerSpt* Instance()
    {
        static ForgeGtRegenMPPerSpt instance;
        return &instance;
    }

    static Forge::Consumption GetConsumption() { return Forge::CONSUMED_AT_RUNTIME; }
    static char const* GetTableName() { return "gtRegenMPPerSpt"; }

    uint32 GetNumRows() const { return sGtRegenMPPerSptStore.GetNumRows(); }

    // Caller owns the row; Lua frees it on collection.
    ForgeGtRegenMPPerSptRow* GetByID(uint32 id) const
    {
        GtRegenMPPerSptEntry const* entry = sGtRegenMPPerSptStore.LookupEntry(id);
        return entry ? new ForgeGtRegenMPPerSptRow(id, entry) : nullptr;
    }

    ForgeGtRegenMPPerSptRow* GetByClassLevel(uint32 playerClass, uint32 level) const
    {
        if (playerClass < 1 || playerClass > CLASSES || level < 1 || level > LEVELS)
            return nullptr;

        return GetByID((playerClass - 1) * LEVELS + (level - 1));
    }
};

namespace LuaForgeGtRegenMPPerSpt
{
    /**
     * Returns the number of rows in the store.
     *
     * @return uint32 rows
     */
    inline int GetNumRows(lua_State* L, ForgeGtRegenMPPerSpt* store)
    {
        ALE::Push(L, store->GetNumRows());
        return 1;
    }

    /**
     * Returns the row with the given index, or nil.
     *
     * @param uint32 id : (class - 1) * 100 + (level - 1)
     * @return [ForgeGtRegenMPPerSptRow] row
     */
    inline int GetByID(lua_State* L, ForgeGtRegenMPPerSpt* store)
    {
        uint32 id = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, store->GetByID(id));
        return 1;
    }

    /**
     * Returns the row for a class and level, or nil.
     *
     * @param uint32 class : 1 = warrior ... 11 = druid
     * @param uint32 level : 1 to 100
     * @return [ForgeGtRegenMPPerSptRow] row
     */
    inline int GetByClassLevel(lua_State* L, ForgeGtRegenMPPerSpt* store)
    {
        uint32 playerClass = ALE::CHECKVAL<uint32>(L, 2);
        uint32 level = ALE::CHECKVAL<uint32>(L, 3);
        ALE::Push(L, store->GetByClassLevel(playerClass, level));
        return 1;
    }
}

namespace LuaForgeGtRegenMPPerSptRow
{
    /**
     * Returns the row index.
     *
     * @return uint32 id
     */
    inline int GetID(lua_State* L, ForgeGtRegenMPPerSptRow* row)
    {
        ALE::Push(L, row->GetID());
        return 1;
    }

    /**
     * Returns the mana regen ratio per point of Spirit.
     *
     * @return float data
     */
    inline int GetData(lua_State* L, ForgeGtRegenMPPerSptRow* row)
    {
        ALE::Push(L, row->GetData());
        return 1;
    }

    /**
     * Sets the ratio on the working copy. Call Push to publish it.
     *
     * @param float data
     * @return [ForgeGtRegenMPPerSptRow] self : so calls can be chained
     */
    inline int SetData(lua_State* L, ForgeGtRegenMPPerSptRow* row)
    {
        row->SetData(ALE::CHECKVAL<float>(L, 2));

        // Returns the userdata received rather than a new wrapper around the
        // same row, which would pile up objects for the collector.
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Publishes the working copy into the store.
     *
     * @return bool pushed : false if nothing was modified
     */
    inline int Push(lua_State* L, ForgeGtRegenMPPerSptRow* row)
    {
        ALE::Push(L, row->Push());
        return 1;
    }
}

#endif
