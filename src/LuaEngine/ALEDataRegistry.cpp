/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#include "ALEDataRegistry.h"

// The push macros go through ALE::Push(T const*), which instantiates
// ALETemplate<T>. LuaEngine.h only forward declares it.
#include "ALETemplate.h"

#include "ObjectMgr.h"
#include "Trainer.h"

#include <algorithm>

/*
 * Every data table a script can reach by name, one line each.
 *
 * Adding a table means adding one line here and a methods header for its rows
 * - the same two steps whether it comes from a .dbc or from the world
 * database.
 */
std::vector<DataDefinition> dataRegistry =
{
    REGISTER_DBC(GemProperties,   GemPropertiesEntry,   sGemPropertiesStore),
    REGISTER_DBC(Spell,           SpellEntry,           sSpellStore),
    REGISTER_DBC(gtRegenMPPerSpt, GtRegenMPPerSptEntry, sGtRegenMPPerSptStore),

    REGISTER_DB_INDEXED(item_template,     ItemTemplate,      *sObjectMgr->GetItemTemplateStoreFast()),
    REGISTER_DB_MAP    (creature_template, CreatureTemplate,  *sObjectMgr->GetCreatureTemplates()),
    REGISTER_DB_MAP    (trainer,           Trainer::Trainer,   sObjectMgr->GetTrainerStore()),
};

int32 FindDataTableId(std::string const& name)
{
    std::string needle = name;
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);

    for (std::size_t index = 0; index < dataRegistry.size(); ++index)
    {
        std::string candidate = dataRegistry[index].name;
        std::transform(candidate.begin(), candidate.end(), candidate.begin(), ::tolower);

        if (candidate == needle)
            return int32(index);
    }

    return -1;
}

DataDefinition const* FindDataTable(std::string const& name)
{
    int32 index = FindDataTableId(name);
    return index < 0 ? nullptr : &dataRegistry[index];
}
