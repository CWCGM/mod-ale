/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef FORGE_ITEMTEMPLATE_METHODS_H
#define FORGE_ITEMTEMPLATE_METHODS_H

#include "ForgeTypes.h"
#include "ObjectMgr.h"
#include "ALEUtility.h"

/*
 * item_template - the world item definitions.
 *
 * Consumption: runtime. ObjectMgr::GetItemTemplate reads the store on every
 * call, so a template patched here is picked up by everything afterwards.
 *
 * Two things worth knowing before changing an item.
 *
 * The client keeps its own item cache. Server side mechanics follow this
 * store immediately, but a player who already saw the item keeps the old
 * name, icon and tooltip until their cache is cleared.
 *
 * Rows are written in place. Unlike the DBC stores, item templates already
 * live as individual objects owned by ObjectMgr, so there is nothing to
 * reallocate - the working copy is published field by field.
 */
class ForgeItemTemplateRow
{
public:
    ForgeItemTemplateRow(uint32 entry, ItemTemplate const* source)
        : _entry(entry), _dirty(false), _value(*source) { }

    uint32 GetEntry() const { return _entry; }
    bool IsDirty() const { return _dirty; }

    ItemTemplate const& Value() const { return _value; }
    ItemTemplate& Edit() { _dirty = true; return _value; }

    // Item stats are stored as pairs in a fixed array; slot is 1 based to
    // match the stat_type1..stat_type10 columns.
    bool HasStatSlot(uint32 slot) const
    {
        return slot >= 1 && slot <= MAX_ITEM_PROTO_STATS;
    }

    bool HasDamageSlot(uint32 slot) const
    {
        return slot >= 1 && slot <= MAX_ITEM_PROTO_DAMAGES;
    }

    bool HasSpellSlot(uint32 slot) const
    {
        return slot >= 1 && slot <= MAX_ITEM_PROTO_SPELLS;
    }

    bool Push()
    {
        if (!_dirty)
            return false;

        ItemTemplate const* live = sObjectMgr->GetItemTemplate(_entry);
        if (!live)
            return false;

        *const_cast<ItemTemplate*>(live) = _value;
        _dirty = false;
        return true;
    }

private:
    uint32 _entry;
    bool _dirty;
    ItemTemplate _value;
};

class ForgeItemTemplate
{
public:
    static ForgeItemTemplate* Instance()
    {
        static ForgeItemTemplate instance;
        return &instance;
    }

    static char const* GetTableName() { return "item_template"; }

    uint32 GetNumRows() const
    {
        return uint32(sObjectMgr->GetItemTemplateStore()->size());
    }

    ForgeItemTemplateRow* GetByID(uint32 entry) const
    {
        ItemTemplate const* item = sObjectMgr->GetItemTemplate(entry);
        return item ? new ForgeItemTemplateRow(entry, item) : nullptr;
    }
};

namespace LuaForgeItemTemplate
{
    /**
     * Returns how many item templates the server loaded.
     *
     * @return uint32 rows
     */
    inline int GetNumRows(lua_State* L, ForgeItemTemplate* store)
    {
        ALE::Push(L, store->GetNumRows());
        return 1;
    }

    /**
     * Returns the template with the given entry, or nil.
     *
     * @param uint32 entry
     * @return [ForgeItemTemplateRow] row
     */
    inline int GetByID(lua_State* L, ForgeItemTemplate* store)
    {
        uint32 entry = ALE::CHECKVAL<uint32>(L, 2);
        ALE::Push(L, store->GetByID(entry));
        return 1;
    }

    // Walks the store, handing one row per item to the callback.
    //
    // `itemClass` below zero means every class. Filtering here rather than in
    // the script matters: a row carries a full copy of the template, so
    // building one for an item the script discards immediately is pure waste
    // on a table this size.
    inline int WalkStore(lua_State* L, int32 itemClass)
    {
        luaL_checktype(L, 2, LUA_TFUNCTION);

        std::vector<ItemTemplate*> const* items = sObjectMgr->GetItemTemplateStoreFast();
        uint32 visited = 0;

        for (uint32 entry = 0; entry < items->size(); ++entry)
        {
            ItemTemplate const* item = (*items)[entry];
            if (!item)
                continue;

            if (itemClass >= 0 && int32(item->Class) != itemClass)
                continue;

            lua_pushvalue(L, 2);
            ALE::Push(L, new ForgeItemTemplateRow(entry, item));

            // pcall rather than call: one faulty item should not abort the
            // whole walk halfway through, leaving the table half patched.
            if (lua_pcall(L, 1, 0, 0) != 0)
            {
                ALE_LOG_ERROR("[ALE]: Forge item_template walk failed on entry {}: {}",
                    entry, lua_tostring(L, -1));
                lua_pop(L, 1);
            }

            ++visited;
        }

        ALE::Push(L, visited);
        return 1;
    }

    /**
     * Calls the handler once per item template.
     *
     * @param function handler : receives a [ForgeItemTemplateRow]
     * @return uint32 visited
     */
    inline int ForEach(lua_State* L, ForgeItemTemplate* /*store*/)
    {
        return WalkStore(L, -1);
    }

    /**
     * Calls the handler once per item template of a given class.
     *
     * Cheaper than filtering in the script: rows are only built for items
     * that match.
     *
     * @param uint32 itemClass : 2 = weapon, 4 = armor
     * @param function handler : receives a [ForgeItemTemplateRow]
     * @return uint32 visited
     */
    inline int ForEachOfClass(lua_State* L, ForgeItemTemplate* /*store*/)
    {
        int32 itemClass = ALE::CHECKVAL<int32>(L, 2);

        // The callback sits at index 3 here; move it where WalkStore expects.
        luaL_checktype(L, 3, LUA_TFUNCTION);
        lua_pushvalue(L, 3);
        lua_replace(L, 2);

        return WalkStore(L, itemClass);
    }
}

namespace LuaForgeItemTemplateRow
{
    /**
     * Returns the item entry.
     *
     * @return uint32 entry
     */
    inline int GetEntry(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->GetEntry());
        return 1;
    }

    /**
     * Returns the item name.
     *
     * @return string name
     */
    inline int GetName(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Name1);
        return 1;
    }

    /**
     * Sets the item name on the working copy.
     *
     * @param string name
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetName(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Name1 = ALE::CHECKVAL<std::string>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Returns the item description.
     *
     * @return string description
     */
    inline int GetDescription(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Description);
        return 1;
    }

    /**
     * Sets the item description on the working copy.
     *
     * @param string description
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetDescription(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Description = ALE::CHECKVAL<std::string>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 class
     */
    inline int GetClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Class);
        return 1;
    }

    /**
     * @param uint32 class
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Class = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 subClass
     */
    inline int GetSubClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().SubClass);
        return 1;
    }

    /**
     * @param uint32 subClass
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetSubClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().SubClass = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 quality
     */
    inline int GetQuality(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Quality);
        return 1;
    }

    /**
     * @param uint32 quality
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetQuality(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Quality = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 displayInfoID
     */
    inline int GetDisplayInfoID(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().DisplayInfoID);
        return 1;
    }

    /**
     * @param uint32 displayInfoID
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetDisplayInfoID(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().DisplayInfoID = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 inventoryType
     */
    inline int GetInventoryType(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().InventoryType);
        return 1;
    }

    /**
     * @param uint32 inventoryType
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetInventoryType(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().InventoryType = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Flags is a typed enum in the core, so it crosses the Lua boundary as a
     * plain number in both directions.
     *
     * @return uint32 flags
     */
    inline int GetFlags(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, uint32(row->Value().Flags));
        return 1;
    }

    /**
     * @param uint32 flags
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetFlags(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Flags = ItemFlags(ALE::CHECKVAL<uint32>(L, 2));
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 buyCount
     */
    inline int GetBuyCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().BuyCount);
        return 1;
    }

    /**
     * @param uint32 buyCount
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetBuyCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().BuyCount = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 buyPrice
     */
    inline int GetBuyPrice(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().BuyPrice);
        return 1;
    }

    /**
     * @param int32 buyPrice
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetBuyPrice(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().BuyPrice = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 sellPrice
     */
    inline int GetSellPrice(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().SellPrice);
        return 1;
    }

    /**
     * @param uint32 sellPrice
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetSellPrice(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().SellPrice = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Signed on purpose: the field is a bit mask stored as uint32, but -1
     * means "everyone" and that is what the SQL column holds. Handing it back
     * as 4294967295 would be technically right and useless in a script.
     *
     * @return int32 allowableClass
     */
    inline int GetAllowableClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, int32(row->Value().AllowableClass));
        return 1;
    }

    /**
     * @param int32 allowableClass : -1 for everyone. Zero forbids the item to all.
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetAllowableClass(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().AllowableClass = uint32(ALE::CHECKVAL<int32>(L, 2));
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Signed on purpose: the field is a bit mask stored as uint32, but -1
     * means "everyone" and that is what the SQL column holds. Handing it back
     * as 4294967295 would be technically right and useless in a script.
     *
     * @return int32 allowableRace
     */
    inline int GetAllowableRace(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, int32(row->Value().AllowableRace));
        return 1;
    }

    /**
     * @param int32 allowableRace : -1 for everyone. Zero forbids the item to all.
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetAllowableRace(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().AllowableRace = uint32(ALE::CHECKVAL<int32>(L, 2));
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 itemLevel
     */
    inline int GetItemLevel(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().ItemLevel);
        return 1;
    }

    /**
     * @param uint32 itemLevel
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetItemLevel(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().ItemLevel = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredLevel
     */
    inline int GetRequiredLevel(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredLevel);
        return 1;
    }

    /**
     * @param uint32 requiredLevel
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredLevel(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredLevel = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredSkill
     */
    inline int GetRequiredSkill(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredSkill);
        return 1;
    }

    /**
     * @param uint32 requiredSkill
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredSkill(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredSkill = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredSkillRank
     */
    inline int GetRequiredSkillRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredSkillRank);
        return 1;
    }

    /**
     * @param uint32 requiredSkillRank
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredSkillRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredSkillRank = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredSpell
     */
    inline int GetRequiredSpell(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredSpell);
        return 1;
    }

    /**
     * @param uint32 requiredSpell
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredSpell(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredSpell = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 maxCount
     */
    inline int GetMaxCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().MaxCount);
        return 1;
    }

    /**
     * @param int32 maxCount
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetMaxCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().MaxCount = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 stackable
     */
    inline int GetStackable(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Stackable);
        return 1;
    }

    /**
     * @param int32 stackable
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetStackable(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Stackable = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 containerSlots
     */
    inline int GetContainerSlots(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().ContainerSlots);
        return 1;
    }

    /**
     * @param uint32 containerSlots
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetContainerSlots(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().ContainerSlots = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 armor
     */
    inline int GetArmor(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Armor);
        return 1;
    }

    /**
     * @param uint32 armor
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetArmor(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Armor = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 holyRes
     */
    inline int GetHolyRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().HolyRes);
        return 1;
    }

    /**
     * @param int32 holyRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetHolyRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().HolyRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 fireRes
     */
    inline int GetFireRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().FireRes);
        return 1;
    }

    /**
     * @param int32 fireRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetFireRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().FireRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 natureRes
     */
    inline int GetNatureRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().NatureRes);
        return 1;
    }

    /**
     * @param int32 natureRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetNatureRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().NatureRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 frostRes
     */
    inline int GetFrostRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().FrostRes);
        return 1;
    }

    /**
     * @param int32 frostRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetFrostRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().FrostRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 shadowRes
     */
    inline int GetShadowRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().ShadowRes);
        return 1;
    }

    /**
     * @param int32 shadowRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetShadowRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().ShadowRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return int32 arcaneRes
     */
    inline int GetArcaneRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().ArcaneRes);
        return 1;
    }

    /**
     * @param int32 arcaneRes
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetArcaneRes(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().ArcaneRes = ALE::CHECKVAL<int32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 delay
     */
    inline int GetDelay(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Delay);
        return 1;
    }

    /**
     * @param uint32 delay
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetDelay(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Delay = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 ammoType
     */
    inline int GetAmmoType(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().AmmoType);
        return 1;
    }

    /**
     * @param uint32 ammoType
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetAmmoType(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().AmmoType = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 bonding
     */
    inline int GetBonding(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().Bonding);
        return 1;
    }

    /**
     * @param uint32 bonding
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetBonding(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().Bonding = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 pageText
     */
    inline int GetPageText(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().PageText);
        return 1;
    }

    /**
     * @param uint32 pageText
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetPageText(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().PageText = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredHonorRank
     */
    inline int GetRequiredHonorRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredHonorRank);
        return 1;
    }

    /**
     * @param uint32 requiredHonorRank
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredHonorRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredHonorRank = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredCityRank
     */
    inline int GetRequiredCityRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredCityRank);
        return 1;
    }

    /**
     * @param uint32 requiredCityRank
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredCityRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredCityRank = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredReputationFaction
     */
    inline int GetRequiredReputationFaction(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredReputationFaction);
        return 1;
    }

    /**
     * @param uint32 requiredReputationFaction
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredReputationFaction(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredReputationFaction = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 requiredReputationRank
     */
    inline int GetRequiredReputationRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().RequiredReputationRank);
        return 1;
    }

    /**
     * @param uint32 requiredReputationRank
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetRequiredReputationRank(lua_State* L, ForgeItemTemplateRow* row)
    {
        row->Edit().RequiredReputationRank = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Returns a stat pair, or nil when the slot is out of range.
     *
     * @param uint32 slot : 1 to 10, matching stat_type1..stat_type10
     * @return uint32 type
     * @return int32 value
     */
    inline int GetStat(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasStatSlot(slot))
            return 0;

        _ItemStat const& stat = row->Value().ItemStat[slot - 1];
        ALE::Push(L, stat.ItemStatType);
        ALE::Push(L, stat.ItemStatValue);
        return 2;
    }

    /**
     * Sets a stat pair. Out of range slots raise rather than pass silently.
     *
     * StatsCount is not touched: the core uses it as the number of stats to
     * apply, so raise it yourself when filling a slot beyond the current
     * count.
     *
     * @param uint32 slot : 1 to 10
     * @param uint32 type
     * @param int32 value
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetStat(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasStatSlot(slot))
        {
            luaL_argerror(L, 2, "stat slot out of range");
            return 0;
        }

        _ItemStat& stat = row->Edit().ItemStat[slot - 1];
        stat.ItemStatType = ALE::CHECKVAL<uint32>(L, 3);
        stat.ItemStatValue = ALE::CHECKVAL<int32>(L, 4);

        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 statsCount : how many stat slots the core applies
     */
    inline int GetStatsCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Value().StatsCount);
        return 1;
    }

    /**
     * @param uint32 statsCount
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetStatsCount(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 count = ALE::CHECKVAL<uint32>(L, 2);
        if (count > MAX_ITEM_PROTO_STATS)
        {
            luaL_argerror(L, 2, "stats count above the item stat slot count");
            return 0;
        }

        row->Edit().StatsCount = count;
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Returns a damage entry, or nil when the slot is out of range.
     *
     * @param uint32 slot : 1 or 2
     * @return float min
     * @return float max
     * @return uint32 type
     */
    inline int GetDamage(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasDamageSlot(slot))
            return 0;

        _Damage const& damage = row->Value().Damage[slot - 1];
        ALE::Push(L, damage.DamageMin);
        ALE::Push(L, damage.DamageMax);
        ALE::Push(L, damage.DamageType);
        return 3;
    }

    /**
     * @param uint32 slot : 1 or 2
     * @param float min
     * @param float max
     * @param uint32 type = 0 : damage school
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetDamage(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasDamageSlot(slot))
        {
            luaL_argerror(L, 2, "damage slot out of range");
            return 0;
        }

        _Damage& damage = row->Edit().Damage[slot - 1];
        damage.DamageMin = ALE::CHECKVAL<float>(L, 3);
        damage.DamageMax = ALE::CHECKVAL<float>(L, 4);
        damage.DamageType = ALE::CHECKVAL<uint32>(L, 5, 0);

        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Returns a spell entry, or nil when the slot is out of range.
     *
     * @param uint32 slot : 1 to 5
     * @return uint32 spellId
     * @return int32 trigger
     */
    inline int GetSpell(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasSpellSlot(slot))
            return 0;

        _Spell const& spell = row->Value().Spells[slot - 1];
        ALE::Push(L, spell.SpellId);
        ALE::Push(L, spell.SpellTrigger);
        return 2;
    }

    /**
     * @param uint32 slot : 1 to 5
     * @param uint32 spellId
     * @param uint32 trigger = 0
     * @return [ForgeItemTemplateRow] self
     */
    inline int SetSpell(lua_State* L, ForgeItemTemplateRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasSpellSlot(slot))
        {
            luaL_argerror(L, 2, "spell slot out of range");
            return 0;
        }

        _Spell& spell = row->Edit().Spells[slot - 1];
        spell.SpellId = ALE::CHECKVAL<uint32>(L, 3);
        spell.SpellTrigger = ALE::CHECKVAL<uint32>(L, 4, 0);

        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Publishes the working copy into the store.
     *
     * @return bool pushed : false if nothing was modified
     */
    inline int Push(lua_State* L, ForgeItemTemplateRow* row)
    {
        ALE::Push(L, row->Push());
        return 1;
    }
}

#endif
