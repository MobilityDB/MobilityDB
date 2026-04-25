/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief Process-global pgPointCloud schema cache, owned by MEOS.
 *
 * MEOS-layer code (e.g. tpcbox bbox computation, pcpoint accessor
 * helpers) needs the parsed @c PCSCHEMA for a given @c pcid to read
 * coordinates from a pcpoint blob.  The schema XML lives in
 * pgpointcloud's @c pointcloud_formats catalog table — only the PG
 * backend can scan it.  This module gives MEOS its own cache so that
 * standalone C programs (with no backend at all) can populate it
 * manually before calling MEOS helpers, and so that PG-backed code
 * pays the catalog-scan cost at most once per pcid per backend.
 *
 * The two entry points:
 *
 *   * @ref meos_pc_schema_register — populate the cache by hand
 *     (standalone use case).  The PG-side @c mobilitydb_pc_schema
 *     hook impl also calls it after a catalog scan.
 *
 *   * @ref meos_pc_schema — fast lookup with hook fallback.  On miss,
 *     defers to @c meos_pc_schema_fn (installed by the PG extension's
 *     @c mobilitydb_init) which does the catalog scan; the result is
 *     then registered automatically.
 *
 * The cache implementation is a small dynamic array (linear scan).
 * Real-world workloads rarely have more than a handful of pcids
 * loaded simultaneously, so a hash table would be over-engineering.
 */

#ifndef __MEOS_SCHEMA_HOOK_H__
#define __MEOS_SCHEMA_HOOK_H__

#include <stdint.h>
#include "pc_api.h"  /* PCSCHEMA */

/** @brief Hook signature: resolve a PCSCHEMA by pcid via an external
 *    facility (typically a PG catalog scan).  Returns @p NULL on
 *    not-found; raises an error directly on hard failures. */
typedef PCSCHEMA *(*meos_pc_schema_fn_t)(uint32_t pcid);

/** @brief Process-global hook pointer.  Initially NULL.  The PG
 *    extension's @c mobilitydb_init installs an impl that does the
 *    @c pointcloud_formats catalog scan; standalone MEOS programs
 *    can leave it NULL and pre-populate via @ref meos_pc_schema_register. */
extern meos_pc_schema_fn_t meos_pc_schema_fn;

/**
 * @brief Resolve a parsed PCSCHEMA by pcid.
 *
 * Lookup order: (1) MEOS-layer cache, (2) installed hook (which may
 * lazily register on hit).  Returns @p NULL only if both miss.
 */
extern PCSCHEMA *meos_pc_schema(uint32_t pcid);

/**
 * @brief Register a parsed PCSCHEMA against a pcid in the MEOS cache.
 *
 * Idempotent — a second call with the same pcid replaces the entry,
 * but does NOT free the previous schema (callers own the lifetime
 * of the PCSCHEMA values they pass in).
 *
 * Standalone C programs call this once at startup for every pcid
 * they intend to use; PG backends typically don't need to call it
 * directly — the hook impl handles registration.
 */
extern void meos_pc_schema_register(uint32_t pcid, PCSCHEMA *schema);

/**
 * @brief Drop every entry from the MEOS schema cache.
 *
 * Does NOT free the cached PCSCHEMA values themselves — they belong
 * to whoever passed them in (the PG hook caller pallocs into
 * CacheMemoryContext; standalone callers track lifetime via their
 * own allocator).  Call when shutting MEOS down or when invalidating
 * after a schema change.
 */
extern void meos_pc_schema_clear(void);

#endif /* __MEOS_SCHEMA_HOOK_H__ */
