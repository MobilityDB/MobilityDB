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
/* Forward decl — full PCSCHEMA layout lives in libpc.a's pc_api.h.
 * Keeping this header pc_api-free lets it be safely included from
 * MEOS TUs (e.g. type_in.c, type_out.c) that aren't on the libpc
 * include path. */
typedef struct PCSCHEMA PCSCHEMA;

/** @brief Hook signature: resolve a PCSCHEMA by pcid via an external
 *    facility (typically a PG catalog scan).  Returns @p NULL on
 *    not-found; raises an error directly on hard failures. */
typedef PCSCHEMA *(*meos_pc_schema_fn_t)(uint32_t pcid);

/** @brief Process-global hook pointer.  Initially NULL.  The PG
 *    extension's @c mobilitydb_init installs an impl that does the
 *    @c pointcloud_formats catalog scan; standalone MEOS programs
 *    can leave it NULL and pre-populate via @ref meos_pc_schema_register. */
extern meos_pc_schema_fn_t meos_pc_schema_fn;

/** @brief Hook signature: parse a pgPointCloud schema XML string into
 *    a long-lived @c PCSCHEMA* and stamp it with the given pcid (the
 *    upstream parser doesn't populate the pcid field).  Returns @p NULL
 *    on parse failure. */
typedef PCSCHEMA *(*meos_pc_parse_xml_fn_t)(uint32_t pcid, const char *xml);

/** @brief Process-global hook pointer for XML parsing.  Installed by
 *    the PG extension's @c mobilitydb_init.  Used by the WKB decoder
 *    when an incoming WKB blob carries an embedded schema XML for a
 *    pcid not yet registered in this backend. */
extern meos_pc_parse_xml_fn_t meos_pc_parse_xml_fn;

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
 * @brief Variant of @ref meos_pc_schema_register that also caches the
 *   source XML text alongside the parsed schema.
 *
 * Required when the cache feeds the WKB encoder, which embeds schema
 * XML in cross-cluster-portable WKB blobs. @p xml_text is copied
 * (palloc'd in @c TopMemoryContext on PG, malloc'd in standalone
 * MEOS) so the caller may free it after the call. Pass @p NULL for
 * @p xml_text to register a parse-only entry — equivalent to
 * @ref meos_pc_schema_register.
 */
extern void meos_pc_schema_register_xml(uint32_t pcid, PCSCHEMA *schema,
  const char *xml_text);

/**
 * @brief Return the cached XML text for a registered pcid, or @p NULL
 *   if no XML was registered with this pcid (cache miss, or registered
 *   without XML via the older @ref meos_pc_schema_register).
 *
 * Callers must NOT free the returned pointer — it is owned by the
 * cache.
 */
extern const char *meos_pc_schema_xml(uint32_t pcid);

/**
 * @brief Return the SRID cached for @p pcid (SRID_INVALID on miss).
 *
 * The SRID is extracted from the PCSCHEMA at registration time so
 * callers do not need the full PCSCHEMA struct definition (which lives
 * in pointcloud-pg/lib/pc_api.h, unavailable outside the pointcloud
 * build target).
 */
extern int32_t meos_pc_schema_get_srid(uint32_t pcid);

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
