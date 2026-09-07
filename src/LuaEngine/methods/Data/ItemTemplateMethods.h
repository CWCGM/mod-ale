/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef ITEMTEMPLATEMETHODS_H
#define ITEMTEMPLATEMETHODS_H

#include "Chat.h"

/***
 * Represents item data defined in the database and DBCs, such as stats, quality, class restrictions, and display info.
 *
 * Describes the item TYPE, not a particular item in a bag or on a character.
 *
 * ObjectMgr::GetItemTemplate reads the store on every use, so a template
 * changed here is picked up by everything afterwards. Two things are worth
 * knowing before writing to one.
 *
 * The client keeps its own item cache. Server side mechanics follow this
 * store immediately, but a player who has already seen the item keeps the old
 * name, icon and tooltip until their cache is cleared.
 *
 * AllowableClass and AllowableRace are bit MASKS, tested as
 * `!(AllowableClass & getClassMask())`. Setting one to zero forbids the item
 * to everyone, which is the opposite of what it looks like: the value meaning
 * "everyone" is -1.
 *
 * Inherits all methods from: none
 */
namespace LuaItemTemplate
{
    /**
     * Returns the [ItemTemplate]'s ID.
     *
     * @return uint32 itemId
     */
    int GetItemId(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->ItemId);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s class.
     *
     * @return uint32 class
     */
    int GetClass(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->Class);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s subclass.
     *
     * @return uint32 subClass
     */
    int GetSubClass(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->SubClass);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s name in the [Player]'s locale.
     *
     * @param [LocaleConstant] locale = DEFAULT_LOCALE : locale to return the [ItemTemplate] name in (it's optional default: LOCALE_enUS)
     * 
     * @return string name
     */
    int GetName(lua_State* L, ItemTemplate* itemTemplate)
    {
        uint32 loc_idx = ALE::CHECKVAL<uint32>(L, 2, LocaleConstant::LOCALE_enUS);

        const ItemLocale* itemLocale = eObjectMgr->GetItemLocale(itemTemplate->ItemId);
        std::string name = itemTemplate->Name1;

        if (itemLocale && !itemLocale->Name[loc_idx].empty())
            name = itemLocale->Name[loc_idx];

        ALE::Push(L, name);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s display ID.
     *
     * @return uint32 displayId
     */
    int GetDisplayId(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->DisplayInfoID);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s quality.
     *
     * @return uint32 quality
     */
    int GetQuality(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->Quality);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s flags.
     *
     * @return uint32 flags
     */
    int GetFlags(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->Flags);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s extra flags.
     *
     * @return uint32 flags
     */
    int GetExtraFlags(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->Flags2);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s default purchase count.
     *
     * @return uint32 buyCount
     */
    int GetBuyCount(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->BuyCount);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s purchase price.
     *
     * @return int32 buyPrice
     */
    int GetBuyPrice(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->BuyPrice);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s sell price.
     *
     * @return uint32 sellPrice
     */
    int GetSellPrice(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->SellPrice);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s inventory type.
     *
     * @return uint32 inventoryType
     */
    int GetInventoryType(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->InventoryType);
        return 1;
    }

    /**
     * Returns the [Player] classes allowed to use this [ItemTemplate].
     *
     * @return uint32 allowableClass
     */
    int GetAllowableClass(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->AllowableClass);
        return 1;
    }

    /**
     * Returns the [Player] races allowed to use this [ItemTemplate].
     *
     * @return uint32 allowableRace
     */
    int GetAllowableRace(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->AllowableRace);
        return 1;
    }

    /**
     * Returns the [ItemTemplate]'s item level.
     *
     * @return uint32 itemLevel
     */
    int GetItemLevel(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->ItemLevel);
        return 1;
    }

    /**
     * Returns the minimum level required to use this [ItemTemplate].
     *
     * @return uint32 requiredLevel
     */
    int GetRequiredLevel(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredLevel);
        return 1;
    }

    /**
     * Returns the icon is used by this [ItemTemplate].
     * 
     * @return string itemIcon
     */
    int GetIcon(lua_State* L, ItemTemplate* itemTemplate)
    {   
        uint32 display_id = itemTemplate->DisplayInfoID;
        
        ItemDisplayInfoEntry const* displayInfo = sItemDisplayInfoStore.LookupEntry(display_id);       
        const char* icon = displayInfo->inventoryIcon;

        ALE::Push(L, icon);
        return 1;
    }
    /**
     * Sets the level a character must reach to use the item.
     *
     * @param uint32 requiredLevel : 0 for no requirement
     */
    int SetRequiredLevel(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredLevel = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns the profession the item requires.
     *
     * @return uint32 requiredSkill : 0 for no requirement
     */
    int GetRequiredSkill(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredSkill);
        return 1;
    }

    /**
     * @param uint32 requiredSkill : 0 for no requirement
     */
    int SetRequiredSkill(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredSkill = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 requiredSkillRank
     */
    int GetRequiredSkillRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredSkillRank);
        return 1;
    }

    /**
     * @param uint32 requiredSkillRank
     */
    int SetRequiredSkillRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredSkillRank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Returns the spell a character must know to use the item.
     *
     * @return uint32 requiredSpell : 0 for no requirement
     */
    int GetRequiredSpell(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredSpell);
        return 1;
    }

    /**
     * @param uint32 requiredSpell : 0 for no requirement
     */
    int SetRequiredSpell(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredSpell = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 requiredHonorRank
     */
    int GetRequiredHonorRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredHonorRank);
        return 1;
    }

    /**
     * @param uint32 requiredHonorRank
     */
    int SetRequiredHonorRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredHonorRank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 requiredCityRank
     */
    int GetRequiredCityRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredCityRank);
        return 1;
    }

    /**
     * @param uint32 requiredCityRank
     */
    int SetRequiredCityRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredCityRank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 requiredReputationFaction : 0 for no requirement
     */
    int GetRequiredReputationFaction(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredReputationFaction);
        return 1;
    }

    /**
     * @param uint32 requiredReputationFaction : 0 for no requirement
     */
    int SetRequiredReputationFaction(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredReputationFaction = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * @return uint32 requiredReputationRank
     */
    int GetRequiredReputationRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        ALE::Push(L, itemTemplate->RequiredReputationRank);
        return 1;
    }

    /**
     * @param uint32 requiredReputationRank
     */
    int SetRequiredReputationRank(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->RequiredReputationRank = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }

    /**
     * Sets which classes may use the item.
     *
     * A bit MASK, and the value meaning "every class" is -1. Zero forbids the
     * item to everyone.
     *
     * @param int32 allowableClass : -1 for every class
     */
    int SetAllowableClass(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->AllowableClass = ALE::CHECKVAL<int32>(L, 2);
        return 0;
    }

    /**
     * Sets which races may use the item.
     *
     * A bit MASK, and the value meaning "every race" is -1. Zero forbids the
     * item to everyone.
     *
     * @param int32 allowableRace : -1 for every race
     */
    int SetAllowableRace(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->AllowableRace = ALE::CHECKVAL<int32>(L, 2);
        return 0;
    }

    /**
     * Flags is a typed enum in the core, so it crosses the Lua boundary as a
     * plain number.
     *
     * @param uint32 flags
     */
    int SetFlags(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->Flags = ItemFlags(ALE::CHECKVAL<uint32>(L, 2));
        return 0;
    }

    /**
     * @param uint32 sellPrice : in copper
     */
    int SetSellPrice(lua_State* L, ItemTemplate* itemTemplate)
    {
        itemTemplate->SellPrice = ALE::CHECKVAL<uint32>(L, 2);
        return 0;
    }
}

#endif