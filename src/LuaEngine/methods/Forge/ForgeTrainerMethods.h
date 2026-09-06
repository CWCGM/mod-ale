/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef FORGE_TRAINER_METHODS_H
#define FORGE_TRAINER_METHODS_H

#include "ForgeTypes.h"
#include "ObjectMgr.h"
#include "Trainer.h"
#include "ALEUtility.h"

/*
 * trainer - what every trainer of the world teaches.
 *
 * Consumption: runtime. Trainer::SendSpells and Trainer::CanTeachSpell read
 * the spell list on every visit, so an edit here reaches the next player who
 * talks to the npc.
 *
 * A row is one SPELL of one trainer, not one trainer: that is the grain
 * scripts work at, and it carries the trainer it came from so a walk still
 * knows which class is being taught.
 *
 * The same spell is taught by every trainer of a class, once per city and
 * race, so a walk hands it out many times. Deduplicating is the caller's job:
 * which key to fold on depends on what the script is building.
 *
 * What cannot be done here: adding or removing a spell, or moving one to
 * another trainer. ObjectMgr indexes trainers by class when it loads them,
 * and that index is not rebuilt afterwards.
 */
class ForgeTrainerSpellRow
{
public:
    ForgeTrainerSpellRow(Trainer::Trainer const* trainer, uint32 trainerId, Trainer::Spell const* source)
        : _trainerId(trainerId), _type(uint32(trainer->GetTrainerType())),
          _requirement(trainer->GetTrainerRequirement()), _dirty(false), _value(*source) { }

    uint32 GetTrainerId() const { return _trainerId; }
    uint32 GetTrainerType() const { return _type; }
    uint32 GetTrainerRequirement() const { return _requirement; }
    bool IsDirty() const { return _dirty; }

    Trainer::Spell const& Value() const { return _value; }
    Trainer::Spell& Edit() { _dirty = true; return _value; }

    // ReqAbility is a fixed array of three; slot is 1 based to match the
    // ReqAbility1..ReqAbility3 columns.
    bool HasAbilitySlot(uint32 slot) const
    {
        return slot >= 1 && slot <= _value.ReqAbility.size();
    }

    // The live spell is found again through the store rather than kept as a
    // pointer: a script is free to hold a row for as long as it likes, and a
    // stale pointer would be written blindly.
    bool Push()
    {
        if (!_dirty)
            return false;

        auto const& trainers = sObjectMgr->GetTrainerStore();
        auto itr = trainers.find(_trainerId);
        if (itr == trainers.end())
            return false;

        // SpellId identifies the row, which is why it has no setter: changing
        // it would leave nothing to look the live spell up by.
        Trainer::Spell const* live = itr->second.GetSpell(_value.SpellId);
        if (!live)
            return false;

        *const_cast<Trainer::Spell*>(live) = _value;
        _dirty = false;
        return true;
    }

private:
    uint32 _trainerId;
    uint32 _type;
    uint32 _requirement;
    bool _dirty;
    Trainer::Spell _value;
};

class ForgeTrainer
{
public:
    static ForgeTrainer* Instance()
    {
        static ForgeTrainer instance;
        return &instance;
    }

    static char const* GetTableName() { return "trainer"; }

    uint32 GetNumRows() const
    {
        return uint32(sObjectMgr->GetTrainerStore().size());
    }
};

namespace LuaForgeTrainer
{
    /**
     * Returns how many trainers the server loaded.
     *
     * @return uint32 rows
     */
    inline int GetNumRows(lua_State* L, ForgeTrainer* store)
    {
        ALE::Push(L, store->GetNumRows());
        return 1;
    }

    // Walks the store, handing one row per taught spell to the callback.
    //
    // `classId` below zero means every trainer. Above zero it keeps only class
    // trainers of that class, which is the same set ObjectMgr indexes: type 0
    // and a requirement naming a playable class.
    inline int WalkStore(lua_State* L, int32 classId)
    {
        luaL_checktype(L, 2, LUA_TFUNCTION);

        uint32 visited = 0;

        for (auto const& pair : sObjectMgr->GetTrainerStore())
        {
            Trainer::Trainer const& trainer = pair.second;

            if (classId >= 0)
            {
                if (trainer.GetTrainerType() != Trainer::Type::Class)
                    continue;

                if (int32(trainer.GetTrainerRequirement()) != classId)
                    continue;
            }

            for (Trainer::Spell const& spell : trainer.GetSpells())
            {
                lua_pushvalue(L, 2);
                ALE::Push(L, new ForgeTrainerSpellRow(&trainer, pair.first, &spell));

                // pcall rather than call: one faulty spell should not abort
                // the whole walk halfway through.
                if (lua_pcall(L, 1, 0, 0) != 0)
                {
                    ALE_LOG_ERROR("[ALE]: Forge trainer walk failed on trainer {} spell {}: {}",
                        pair.first, spell.SpellId, lua_tostring(L, -1));
                    lua_pop(L, 1);
                }

                ++visited;
            }
        }

        ALE::Push(L, visited);
        return 1;
    }

    /**
     * Calls the handler once per spell of every trainer.
     *
     * @param function handler : receives a [ForgeTrainerSpellRow]
     * @return uint32 visited
     */
    inline int ForEach(lua_State* L, ForgeTrainer* /*store*/)
    {
        return WalkStore(L, -1);
    }

    /**
     * Calls the handler once per spell taught by the class trainers of a
     * class. Mount, tradeskill and pet trainers are left out.
     *
     * @param uint32 classId : 1 = warrior ... 11 = druid
     * @param function handler : receives a [ForgeTrainerSpellRow]
     * @return uint32 visited
     */
    inline int ForEachOfClass(lua_State* L, ForgeTrainer* /*store*/)
    {
        int32 classId = ALE::CHECKVAL<int32>(L, 2);

        // The callback sits at index 3 here; move it where WalkStore expects.
        luaL_checktype(L, 3, LUA_TFUNCTION);
        lua_pushvalue(L, 3);
        lua_replace(L, 2);

        return WalkStore(L, classId);
    }
}

namespace LuaForgeTrainerSpellRow
{
    /**
     * Returns the id of the trainer teaching this spell.
     *
     * @return uint32 trainerId
     */
    inline int GetTrainerId(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->GetTrainerId());
        return 1;
    }

    /**
     * Returns what kind of trainer teaches it.
     *
     * @return uint32 type : 0 = class, 1 = mount, 2 = tradeskill, 3 = pet
     */
    inline int GetTrainerType(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->GetTrainerType());
        return 1;
    }

    /**
     * Returns what the trainer requires. For a class trainer this is the
     * class it teaches, 1 = warrior ... 11 = druid.
     *
     * @return uint32 requirement
     */
    inline int GetTrainerRequirement(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->GetTrainerRequirement());
        return 1;
    }

    /**
     * Returns the taught spell. Read only: it is the key the row is written
     * back by.
     *
     * @return uint32 spellId
     */
    inline int GetSpellId(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->Value().SpellId);
        return 1;
    }

    /**
     * @return uint32 moneyCost : in copper
     */
    inline int GetMoneyCost(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->Value().MoneyCost);
        return 1;
    }

    /**
     * @param uint32 moneyCost : in copper
     * @return [ForgeTrainerSpellRow] self
     */
    inline int SetMoneyCost(lua_State* L, ForgeTrainerSpellRow* row)
    {
        row->Edit().MoneyCost = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 reqSkillLine : 0 when no profession is needed
     */
    inline int GetReqSkillLine(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->Value().ReqSkillLine);
        return 1;
    }

    /**
     * @param uint32 reqSkillLine
     * @return [ForgeTrainerSpellRow] self
     */
    inline int SetReqSkillLine(lua_State* L, ForgeTrainerSpellRow* row)
    {
        row->Edit().ReqSkillLine = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 reqSkillRank
     */
    inline int GetReqSkillRank(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->Value().ReqSkillRank);
        return 1;
    }

    /**
     * @param uint32 reqSkillRank
     * @return [ForgeTrainerSpellRow] self
     */
    inline int SetReqSkillRank(lua_State* L, ForgeTrainerSpellRow* row)
    {
        row->Edit().ReqSkillRank = ALE::CHECKVAL<uint32>(L, 2);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Returns the spell that must already be known, or nil if the slot is out
     * of range.
     *
     * @param uint32 slot : 1 to 3
     * @return uint32 reqAbility : 0 when the slot requires nothing
     */
    inline int GetReqAbility(lua_State* L, ForgeTrainerSpellRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        if (!row->HasAbilitySlot(slot))
            return 0;

        ALE::Push(L, row->Value().ReqAbility[slot - 1]);
        return 1;
    }

    /**
     * @param uint32 slot : 1 to 3
     * @param uint32 reqAbility : 0 to require nothing
     * @return [ForgeTrainerSpellRow] self
     */
    inline int SetReqAbility(lua_State* L, ForgeTrainerSpellRow* row)
    {
        uint32 slot = ALE::CHECKVAL<uint32>(L, 2);
        uint32 spellId = ALE::CHECKVAL<uint32>(L, 3);

        if (!row->HasAbilitySlot(slot))
            return luaL_argerror(L, 2, "ReqAbility slot out of range, expected 1 to 3");

        row->Edit().ReqAbility[slot - 1] = spellId;
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * @return uint32 reqLevel : 0 when the spell has no level requirement
     */
    inline int GetReqLevel(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, uint32(row->Value().ReqLevel));
        return 1;
    }

    /**
     * ReqLevel is a uint8 in the core, so anything above 255 is refused
     * rather than silently wrapped.
     *
     * @param uint32 reqLevel
     * @return [ForgeTrainerSpellRow] self
     */
    inline int SetReqLevel(lua_State* L, ForgeTrainerSpellRow* row)
    {
        uint32 level = ALE::CHECKVAL<uint32>(L, 2);
        if (level > 255)
            return luaL_argerror(L, 2, "ReqLevel out of range, expected 0 to 255");

        row->Edit().ReqLevel = uint8(level);
        lua_pushvalue(L, 1);
        return 1;
    }

    /**
     * Publishes the working copy into the store.
     *
     * @return bool pushed : false if nothing was modified
     */
    inline int Push(lua_State* L, ForgeTrainerSpellRow* row)
    {
        ALE::Push(L, row->Push());
        return 1;
    }
}

#endif
