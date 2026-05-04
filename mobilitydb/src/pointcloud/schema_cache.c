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
 * @brief PG-side resolver for pgPointCloud schemas — installed as the
 *   @c meos_pc_schema_fn hook by @c mobilitydb_init.
 *
 * The cache itself lives at the MEOS layer
 * (@c meos/src/pointcloud/schema_hook.c), so this file is now a thin
 * "fetch from @c pointcloud_formats and parse" implementation.  MEOS
 * registers the parsed schema into its cache automatically on hook
 * miss — we don't keep our own duplicate cache on the PG side.
 *
 * Reads @c pointcloud_formats rows via a direct heap scan
 * (@c table_open + @c systable_beginscan), parses the XML via libpc.a's
 * @c pc_schema_from_xml.  The direct heap scan approach matches what
 * pgpointcloud does internally for its own lookups and — crucially —
 * avoids @c SPI_connect entirely.  @c SPI_connect was observed to
 * crash when called from inside the executor during a SELECT over a
 * tpcpoint column, even though the identical SPI pattern worked from
 * simpler pcpoint-arg call sites.  Direct catalog access sidesteps
 * the whole SPI machinery and works in every executor context we've
 * tested.
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
#include <utils/lsyscache.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
/* pgpointcloud */
#include "pc_api.h"
/* MEOS */
#include <meos_pointcloud.h>  /* meos_pc_schema_register_xml */
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

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
  return get_extension_schema(ext_oid);
}

/**
 * @brief Fetch the XML schema text for a given pcid from
 *   @c pointcloud_formats via a direct heap scan.
 * @return palloc'd cstring on hit, NULL on miss.  The memory context
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

  /* Column layout: (pcid int4, srid int4, schema text).  Scan by pcid. */
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

/**
 * @brief Resolve a pgPointCloud schema by pcid (PG-layer hook impl).
 *
 * Installed at @c mobilitydb_init time as the @c meos_pc_schema_fn
 * function pointer, this is invoked on a MEOS-cache miss.  Fetches the
 * XML from @c pointcloud_formats, parses it via libpc.a's
 * @c pc_schema_from_xml in @c TopMemoryContext (so the parsed schema
 * outlives the current query), and returns the result.  The MEOS
 * caller registers the return value into its cache automatically.
 *
 * @param[in] pcid pgPointCloud schema identifier
 * @return Parsed @c PCSCHEMA pointer, or @p NULL if the pcid is
 *   unknown (caller decides whether that's an error).
 */
/**
 * @brief Parse pgPointCloud schema XML into a long-lived @c PCSCHEMA*.
 *
 * Installed at @c mobilitydb_init time as @c meos_pc_parse_xml_fn,
 * this is invoked by the WKB decoder when an incoming WKB blob carries
 * an embedded schema XML for a pcid not yet registered in the backend.
 * Allocates the parsed schema in @c TopMemoryContext so it outlives
 * the current query.
 *
 * @param[in] xml NUL-terminated schema XML
 * @return Parsed @c PCSCHEMA pointer, or @p NULL on parse failure.
 */
PCSCHEMA *
mobilitydb_pc_parse_xml(uint32_t pcid, const char *xml)
{
  if (! xml)
    return NULL;
  MemoryContext old_ctx = MemoryContextSwitchTo(TopMemoryContext);
  PCSCHEMA *schema = pc_schema_from_xml(xml);
  MemoryContextSwitchTo(old_ctx);
  if (schema)
    schema->pcid = pcid;
  return schema;
}

PCSCHEMA *
mobilitydb_pc_schema(uint32_t pcid)
{
  char *xml = fetch_schema_xml(pcid);
  if (xml == NULL)
    return NULL;

  /* Parse into TopMemoryContext so the PCSCHEMA outlives the current
   * query — the MEOS-level cache holds the pointer for the rest of
   * the backend's lifetime. */
  MemoryContext old_ctx = MemoryContextSwitchTo(TopMemoryContext);
  PCSCHEMA *schema = pc_schema_from_xml(xml);
  MemoryContextSwitchTo(old_ctx);

  if (schema == NULL)
  {
    pfree(xml);
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Failed to parse pgpointcloud schema XML for pcid=%u", pcid)));
  }

  /* pc_schema_from_xml doesn't populate schema->pcid — wire it up. */
  schema->pcid = pcid;
  /* Register both parsed schema AND XML — the MEOS WKB encoder needs
   * the XML to embed in cross-cluster-portable WKB blobs. */
  meos_pc_schema_register_xml(pcid, schema, xml);
  pfree(xml);
  return schema;
}
