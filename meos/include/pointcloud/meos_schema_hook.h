/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief Function-pointer hook that lets MEOS-layer code resolve a
 *   pgpointcloud PCSCHEMA by pcid.
 *
 * MEOS doesn't have PG catalog access, so it can't scan
 * @c pointcloud_formats directly. The PG extension installs an
 * implementation of this hook during @c mobilitydb_init (typically
 * pointing at @c mobilitydb_pc_schema in
 * @c mobilitydb/src/pointcloud/schema_cache.c). MEOS code — for
 * example the TPCBox-bbox computation for a tpcpoint TInstant — then
 * calls through this hook to get the schema it needs.
 *
 * The hook is NULL in MEOS-only builds; accessing a schema-dependent
 * code path in a standalone MEOS program without setting the hook
 * first produces a clear error rather than a crash.
 */

#ifndef __MEOS_SCHEMA_HOOK_H__
#define __MEOS_SCHEMA_HOOK_H__

#include <stdint.h>
#include "pc_api.h"  /* PCSCHEMA */

typedef PCSCHEMA *(*meos_pc_schema_fn_t)(uint32_t pcid);

/** @brief Process-global hook pointer. Initially NULL; set by the PG
 *    extension's @c mobilitydb_init or by a MEOS embedder. */
extern meos_pc_schema_fn_t meos_pc_schema_fn;

/** @brief Resolve a PCSCHEMA by pcid via the installed hook. Raises a
 *    MEOS error if the hook is NULL. */
extern PCSCHEMA *meos_pc_schema(uint32_t pcid);

#endif /* __MEOS_SCHEMA_HOOK_H__ */
