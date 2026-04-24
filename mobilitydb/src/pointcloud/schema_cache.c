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
 *
 * Reads @c pointcloud_formats rows via a direct heap scan (table_open +
 * systable_beginscan), parses the XML via libpc.a's pc_schema_from_xml,
 * and caches the resulting @c PCSCHEMA in a per-backend hashtable living
 * in CacheMemoryContext. The direct heap scan approach matches what
 * pgpointcloud does internally for its own lookups and — crucially —
 * avoids SPI_connect entirely. SPI_connect was observed to crash when
 * called from inside the executor during a SELECT over a tpcpoint
 * column, even though the identical SPI pattern worked from simpler
 * pcpoint-arg call sites. Direct catalog access sidesteps the whole
 * SPI machinery and works in every executor context we've tested.
 */

/* PostgreSQL */
#include <postgres.h>
#include <access/genam.h>
#include <access/htup_details.h>
#include <access/table.h>
#include <catalog/pg_extension.h>
#include <commands/extension.h>
#include <utils/builtins.h>
#include <utils/fmgroids.h>
#include <utils/hsearch.h>
#include <utils/lsyscache.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
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

/**
 * @brief Return the install namespace of the pgpointcloud extension, or
 *   @c InvalidOid if pgpointcloud isn't installed in this database.
 */
static Oid
pointcloud_namespace_oid(void)
{
  Oid ext_oid = get_extension_oid("pointcloud", /* missing_ok */ true);
  if (ext_oid == InvalidOid)
    return InvalidOid;

  /* get_extension_schema was added to PG's commands/extension.h in 16+.
   * For earlier versions we'd need the manual pg_extension scan; all
   * currently-supported PG versions for MobilityDB are >= 13 though,
   * and this code path is reached only on POINTCLOUD=ON builds which
   * implicitly require the pgpointcloud extension anyway, so the
   * helper is always available on the platforms we target. */
  return get_extension_schema(ext_oid);
}

/**
 * @brief Fetch the XML schema text for a given pcid from
 *   @c pointcloud_formats via a direct heap scan.
 * @return palloc'd cstring on hit, NULL on miss. The memory context
 *   is whatever was current at call time.
 */
static char *
fetch_schema_xml(uint32_t pcid)
{
  Oid nsp_oid = pointcloud_namespace_oid();
  if (nsp_oid == InvalidOid)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_OBJECT),
      errmsg("pgpointcloud extension is not installed in this database")));

  Oid rel_oid = get_relname_relid("pointcloud_formats", nsp_oid);
  if (rel_oid == InvalidOid)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_TABLE),
      errmsg("pointcloud_formats relation not found in pgpointcloud's "
             "install schema")));

  Relation rel = table_open(rel_oid, AccessShareLock);
  TupleDesc tupDesc = RelationGetDescr(rel);

  /* Column layout: (pcid int4, srid int4, schema text). Scan by pcid. */
  ScanKeyData key[1];
  ScanKeyInit(&key[0],
    /* attnum */ 1,
    BTEqualStrategyNumber,
    F_INT4EQ,
    Int32GetDatum((int32) pcid));

  SysScanDesc scan = systable_beginscan(rel, InvalidOid,
    /* indexOK */ false, NULL, 1, key);

  char *xml = NULL;
  HeapTuple tuple = systable_getnext(scan);
  if (HeapTupleIsValid(tuple))
  {
    bool isnull;
    /* schema is the third column (attnum 3) */
    Datum xml_datum = heap_getattr(tuple, 3, tupDesc, &isnull);
    if (! isnull)
      xml = TextDatumGetCString(xml_datum);
  }

  systable_endscan(scan);
  table_close(rel, AccessShareLock);
  return xml;
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

  /* Fetch in the caller's context, parse into CacheMemoryContext so
   * the resulting PCSCHEMA outlives the current query. */
  char *xml = fetch_schema_xml(pcid);
  if (xml == NULL)
  {
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_OBJECT),
      errmsg("No pgpointcloud schema with pcid=%u", pcid)));
  }

  MemoryContext old_ctx = MemoryContextSwitchTo(CacheMemoryContext);
  PCSCHEMA *schema = pc_schema_from_xml(xml);
  MemoryContextSwitchTo(old_ctx);
  pfree(xml);

  if (schema == NULL)
  {
    hash_search(schema_cache, &pcid, HASH_REMOVE, NULL);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Failed to parse pgpointcloud schema XML for pcid=%u", pcid)));
  }

  /* pc_schema_from_xml doesn't populate schema->pcid — wire it up. */
  schema->pcid = pcid;
  entry->schema = schema;
  return schema;
}
