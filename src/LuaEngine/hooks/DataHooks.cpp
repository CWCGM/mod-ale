/*
 * Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ALETemplate.h"
#include "ALEUtility.h"
#include "ALEDataRegistry.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<DataEvents>(EVENT, ENTRY);\
    if (!DataEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

void ALE::OnDataTableLoad(std::string const& storeName)
{
    // The core hands over the DBC file name, extension included. Scripts name
    // tables without it.
    std::string name = storeName;
    if (name.size() > 4 && name.compare(name.size() - 4, 4, ".dbc") == 0)
        name.erase(name.size() - 4);

    int32 tableId = FindDataTableId(name);
    if (tableId < 0)
        return;

    DataDefinition const& definition = dataRegistry[tableId];

    // Says whether a handler is actually waiting on this table, so a silent
    // no-op can be told apart from a table the registry does not carry.
    bool const bound = DataEventBindings && DataEventBindings->HasBindingsFor(
        EntryKey<DataEvents>(DATA_EVENT_ON_TABLE_LOAD, uint32(tableId)));

    ALE_LOG_INFO("[ALE]: Data table `{}` loaded, {}",
        definition.name, bound ? "calling handlers" : "no handler registered");

    START_HOOK(DATA_EVENT_ON_TABLE_LOAD, uint32(tableId));

    Push(definition.name);

    CallAllFunctions(DataEventBindings, key);
}
