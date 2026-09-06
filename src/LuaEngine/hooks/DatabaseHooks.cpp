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
#include "ForgeRegistry.h"

using namespace Hooks;

#define START_HOOK(EVENT, ENTRY) \
    if (!ALEConfig::GetInstance().IsALEEnabled())\
        return;\
    auto key = EntryKey<DatabaseEvents>(EVENT, ENTRY);\
    if (!DatabaseEventBindings->HasBindingsFor(key))\
        return;\
    LOCK_ALE

/*
 * Relays the core's per-store signal.
 *
 * The core fires this once per data store, right after it has been loaded
 * from the .dbc file and the *_dbc table, so the rows are final and a script
 * can patch them in memory before anything reads them.
 *
 * Stores Forge does not know about are ignored: a script can only register on
 * a named table, so there is nothing to call for the others.
 */
void ALE::OnDatabaseTableLoad(std::string const& storeName)
{
    // The core hands over the file name, extension included. Scripts name
    // tables without it.
    std::string name = storeName;
    if (name.size() > 4 && name.compare(name.size() - 4, 4, ".dbc") == 0)
        name.erase(name.size() - 4);

    Forge::TableInfo const* info = Forge::FindTable(name);
    if (!info)
        return;

    // Says whether a handler is actually waiting on this table, so a silent
    // no-op can be told apart from a table Forge does not know about.
    bool const bound = DatabaseEventBindings && DatabaseEventBindings->HasBindingsFor(
        EntryKey<DatabaseEvents>(DATABASE_EVENT_ON_TABLE_LOAD, info->id));

    ALE_LOG_INFO("[ALE]: Forge store `{}` loaded, {}",
        info->name, bound ? "calling handlers" : "no handler registered");

    START_HOOK(DATABASE_EVENT_ON_TABLE_LOAD, info->id);

    Push(info->name);

    // Pushed here rather than from the registry: only ALE's own Push
    // increments the argument counter CallAllFunctions relies on, and it is
    // private. Pushing from outside would leave the store on the stack
    // without it ever reaching the handler.
    switch (info->id)
    {
        case Forge::TABLE_GT_REGEN_MP_PER_SPT:
            Push(ForgeGtRegenMPPerSpt::Instance());
            break;
        case Forge::TABLE_ITEM_TEMPLATE:
            Push(ForgeItemTemplate::Instance());
            break;
        case Forge::TABLE_TRAINER:
            Push(ForgeTrainer::Instance());
            break;
        default:
            CleanUpStack(1);
            return;
    }

    CallAllFunctions(DatabaseEventBindings, key);
}
