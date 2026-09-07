/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef ALEDATAREGISTRY_H
#define ALEDATAREGISTRY_H

#include <functional>
#include <string>
#include <vector>

#include "DBCStores.h"
#include "LuaEngine.h"

enum DataSource
{
    SOURCE_DBC,         // DBCStorage, from the .dbc file and its *_dbc table
    SOURCE_DATABASE     // built by ObjectMgr from the world database
};

/*
 * Called once per row by a walk. Returning false stops it, so a script that
 * has found what it came for does not pay for the rest of the table.
 */
typedef std::function<bool(void const*)> DataVisitor;

struct DataDefinition
{
    std::string name;
    DataSource  source;

    // One row by id, or nullptr.
    std::function<void const*(uint32)> lookup;

    // Pushes a row as its Lua type. Always the LIVE row, never a copy, so a
    // setter called on it writes straight into the core.
    std::function<void(lua_State*, void const*)> push;

    // Walks every row. Empty when the table cannot enumerate itself.
    std::function<void(DataVisitor const&)> forEach;
};

extern std::vector<DataDefinition> dataRegistry;

/*
 * Case insensitive: a script should not have to remember that the store is
 * spelled gtRegenMPPerSpt and not gtregenmpperspt. Returns nullptr when no
 * table carries that name.
 */
DataDefinition const* FindDataTable(std::string const& name);

/*
 * The same lookup, as an index into dataRegistry, or -1. The event system
 * keys its bindings on integers, and the position in the registry is that
 * integer - so there is no second list of table ids to keep in step.
 */
int32 FindDataTableId(std::string const& name);

/*
 * A DBC store. Rows are indexed by id and the index has holes, so a walk
 * skips what LookupEntry does not resolve.
 */
#define REGISTER_DBC(dbcName, entryType, store)                                     \
    {                                                                               \
        #dbcName,                                                                   \
        SOURCE_DBC,                                                                 \
        [](uint32 id) -> void const* { return store.LookupEntry(id); },             \
        [](lua_State* L, void const* row)                                           \
        {                                                                           \
            ALE::Push(L, static_cast<entryType const*>(row));                       \
        },                                                                          \
        [](DataVisitor const& visit)                                                \
        {                                                                           \
            for (uint32 id = 0; id < store.GetNumRows(); ++id)                      \
                if (entryType const* row = store.LookupEntry(id))                   \
                    if (!visit(row))                                                \
                        return;                                                     \
        }                                                                           \
    }

/*
 * A world database table held as a map from id to row.
 *
 * The container is named, not the way to search it: a table only has to say
 * where its rows live, and this file works out the rest. That keeps a methods
 * header a pure list of methods, with no plumbing that would be repeated
 * table after table.
 */
#define REGISTER_DB_MAP(tableName, entryType, container)                            \
    {                                                                               \
        #tableName,                                                                 \
        SOURCE_DATABASE,                                                            \
        [](uint32 id) -> void const*                                                \
        {                                                                           \
            auto const& rows = container;                                           \
            auto itr = rows.find(id);                                               \
            return itr != rows.end() ? &itr->second : nullptr;                      \
        },                                                                          \
        [](lua_State* L, void const* row)                                           \
        {                                                                           \
            ALE::Push(L, static_cast<entryType const*>(row));                        \
        },                                                                          \
        [](DataVisitor const& visit)                                                \
        {                                                                           \
            for (auto const& pair : container)                                      \
                if (!visit(&pair.second))                                           \
                    return;                                                         \
        }                                                                           \
    }

/*
 * A world database table held as a flat vector of pointers indexed by id. The
 * index has holes, so both the lookup and the walk skip the empty slots.
 */
#define REGISTER_DB_INDEXED(tableName, entryType, container)                        \
    {                                                                               \
        #tableName,                                                                 \
        SOURCE_DATABASE,                                                            \
        [](uint32 id) -> void const*                                                \
        {                                                                           \
            auto const& rows = container;                                           \
            return id < rows.size() ? rows[id] : nullptr;                           \
        },                                                                          \
        [](lua_State* L, void const* row)                                           \
        {                                                                           \
            ALE::Push(L, static_cast<entryType const*>(row));                        \
        },                                                                          \
        [](DataVisitor const& visit)                                                \
        {                                                                           \
            for (auto const* row : container)                                       \
                if (row && !visit(row))                                             \
                    return;                                                         \
        }                                                                           \
    }

#endif // ALEDATAREGISTRY_H
