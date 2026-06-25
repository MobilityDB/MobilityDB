/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief Per-backend cache of pgpointcloud `PCSCHEMA` keyed by pcid.
 *
 * Loads the XML from @c pointcloud_formats via SPI on first miss,
 * parses via libpc.a's @c pc_schema_from_xml, stashes the result in
 * @c CacheMemoryContext so it survives the SPI transaction. Subsequent
 * lookups are O(1) hashtable hits. Same-session only — if a user
 * ALTERs a pcid's XML we would continue serving the stale parse; that
 * is vanishingly rare and matches pgpointcloud's own internal caching
 * behavior.
 */

#ifndef __MOBILITYDB_PC_SCHEMA_CACHE_H__
#define __MOBILITYDB_PC_SCHEMA_CACHE_H__

/* C */
#include <stdint.h>
/* pgpointcloud */
#include "pc_api.h"

/**
 * @brief Return the cached (or freshly-loaded) PCSCHEMA for @p pcid.
 * @details Raises a PG error if @p pcid has no row in
 *   @c pointcloud_formats or the XML fails to parse. Returned pointer
 *   is owned by the cache — do not free.
 */
extern PCSCHEMA *mobilitydb_pc_schema(uint32_t pcid);

/**
 * @brief Parse a pgPointCloud schema XML string into a long-lived
 *   @c PCSCHEMA*. Installed as @c meos_pc_parse_xml_fn so the WKB
 *   decoder can absorb XML embedded in incoming WKB blobs.
 */
extern PCSCHEMA *mobilitydb_pc_parse_xml(uint32_t pcid, const char *xml);

#endif /* __MOBILITYDB_PC_SCHEMA_CACHE_H__ */
