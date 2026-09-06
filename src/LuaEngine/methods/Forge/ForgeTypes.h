/*
* Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef FORGE_TYPES_H
#define FORGE_TYPES_H

#include "LuaEngine.h"
#include "DBCStores.h"

/*
 * Forge exposes the data stores the core loads at startup, so scripts can
 * patch them in memory instead of editing the .dbc files or the *_dbc tables.
 *
 * Whether a store can be patched from a normal script depends on how the core
 * consumes it:
 *
 *   Runtime  - the core calls LookupEntry on every use, so a later write is
 *              picked up by the next read. All gt* tables work this way.
 *   Load     - ObjectMgr digests the rows once at startup and builds derived
 *              structures. Writing afterwards has no effect.
 *
 * Each table states its case in its own header. When in doubt, check who
 * calls LookupEntry on the store.
 */
namespace Forge
{
    enum Consumption
    {
        CONSUMED_AT_RUNTIME,
        CONSUMED_AT_LOAD
    };
}

#endif
