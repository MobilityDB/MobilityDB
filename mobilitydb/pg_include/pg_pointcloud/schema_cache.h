/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
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
