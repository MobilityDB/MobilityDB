/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @file
 * @brief Implementation of the per-backend pgpointcloud schema cache.
 */

/* PostgreSQL */
#include <postgres.h>
#include <executor/spi.h>
#include <utils/builtins.h>
#include <utils/hsearch.h>
#include <utils/memutils.h>
/* pgpointcloud */
#include "pc_api.h"
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

typedef struct
{
  uint32_t pcid;      /* key — must be first for HASH_BLOBS */
  PCSCHEMA *schema;   /* cached parsed schema; owned by CacheMemoryContext */
} schema_cache_entry;

static HTAB *schema_cache = NULL;

static void
init_schema_cache(void)
{
  HASHCTL info;
  info.keysize = sizeof(uint32_t);
  info.entrysize = sizeof(schema_cache_entry);
  info.hcxt = CacheMemoryContext;
  schema_cache = hash_create("MobilityDB pgpointcloud schema cache",
    64, &info, HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);
}

PCSCHEMA *
mobilitydb_pc_schema(uint32_t pcid)
{
  if (schema_cache == NULL)
    init_schema_cache();

  bool found;
  schema_cache_entry *entry = (schema_cache_entry *)
    hash_search(schema_cache, &pcid, HASH_ENTER, &found);
  if (found && entry->schema != NULL)
    return entry->schema;

  entry->pcid = pcid;
  entry->schema = NULL;

  /* Load the XML text via SPI and parse in CacheMemoryContext so the
   * PCSCHEMA (plus the palloc'd substructures pc_schema_from_xml
   * allocates) outlive the SPI transaction. */
  if (SPI_connect() != SPI_OK_CONNECT)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("SPI_connect failed while loading pgpointcloud schema")));

  char query[128];
  snprintf(query, sizeof(query),
    "SELECT schema FROM pointcloud_formats WHERE pcid = %u", pcid);
  int rc = SPI_execute(query, true, 1);
  if (rc != SPI_OK_SELECT)
  {
    SPI_finish();
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Failed to query pointcloud_formats for pcid=%u", pcid)));
  }
  if (SPI_processed == 0)
  {
    SPI_finish();
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_OBJECT),
      errmsg("No pgpointcloud schema with pcid=%u", pcid)));
  }

  bool isnull;
  Datum xml_datum = SPI_getbinval(SPI_tuptable->vals[0],
    SPI_tuptable->tupdesc, 1, &isnull);
  if (isnull)
  {
    SPI_finish();
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("pgpointcloud schema XML is NULL for pcid=%u", pcid)));
  }

  /* Copy the XML text out of the SPI context before finishing SPI. */
  MemoryContext old_ctx = MemoryContextSwitchTo(CacheMemoryContext);
  char *xml = TextDatumGetCString(xml_datum);
  PCSCHEMA *schema = pc_schema_from_xml(xml);
  pfree(xml);
  MemoryContextSwitchTo(old_ctx);

  SPI_finish();

  if (schema == NULL)
  {
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Failed to parse pgpointcloud schema XML for pcid=%u", pcid)));
  }

  /* pgpointcloud's pc_schema_from_xml doesn't set pcid from the XML
   * (the XML only carries dimension definitions); wire it up now. */
  schema->pcid = pcid;

  entry->schema = schema;
  return schema;
}
