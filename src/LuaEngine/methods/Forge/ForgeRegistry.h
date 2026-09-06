/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef FORGE_REGISTRY_H
#define FORGE_REGISTRY_H

#include "ForgeTypes.h"
#include "ForgeGtRegenMPPerSptMethods.h"
#include "ForgeItemTemplateMethods.h"
#include "ForgeTrainerMethods.h"

#include <algorithm>
#include <string>
#include <vector>

/*
 * Maps the table name a script writes to the numeric key the binding map
 * uses. Scripts name tables, the event system keys on integers.
 *
 * Adding a table means adding its header above and one line to Tables().
 */
namespace Forge
{
    enum TableId
    {
        TABLE_NONE = 0,
        TABLE_GT_REGEN_MP_PER_SPT,
        TABLE_ITEM_TEMPLATE,
        TABLE_TRAINER
    };

    struct TableInfo
    {
        TableId id;
        char const* name;
        Consumption consumption;
    };

    inline std::vector<TableInfo> const& Tables()
    {
        static std::vector<TableInfo> const tables =
        {
            { TABLE_GT_REGEN_MP_PER_SPT, "gtRegenMPPerSpt", CONSUMED_AT_RUNTIME },
            { TABLE_ITEM_TEMPLATE,       "item_template",   CONSUMED_AT_RUNTIME },
            { TABLE_TRAINER,             "trainer",         CONSUMED_AT_RUNTIME }
        };

        return tables;
    }

    // Case insensitive: scripts should not have to remember that the table is
    // named gtRegenMPPerSpt and not gtregenmpperspt.
    inline TableInfo const* FindTable(std::string const& name)
    {
        std::string needle = name;
        std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);

        for (TableInfo const& info : Tables())
        {
            std::string candidate = info.name;
            std::transform(candidate.begin(), candidate.end(), candidate.begin(), ::tolower);

            if (candidate == needle)
                return &info;
        }

        return nullptr;
    }

}

#endif
