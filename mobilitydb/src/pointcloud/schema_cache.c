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
#include <utils/fmgroids.h>
#include <utils/lsyscache.h>
#include <utils/memutils.h>
#include <utils/rel.h>
#include <utils/syscache.h>
/* pgPointCloud */
#include "pc_api.h"
/* MEOS */
#include <pgtypes.h>  /* text_to_cstring — NOT utils/builtins.h, whose
                       * fmgrprotos.h json_object collides with json-c */
#include <meos_pointcloud.h>  /* meos_pc_schema_register_xml */
/* MobilityDB */
#include "pg_pointcloud/schema_cache.h"

/*****************************************************************************/

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
 * @brief Fetch the XML schema text and the SRID for a given pcid from
 *   @c pointcloud_formats via a direct heap scan.
 * @details The SRID is a column of that table and appears nowhere in the
 *   schema XML, so this is the only place it can be read from.
 * @param[in] pcid pgPointCloud schema identifier
 * @param[out] srid The SRID the row declares
 * @return palloc'd cstring on hit, NULL on miss.  The memory context
 *   is whatever was current at call time.
 */
static char *
fetch_schema_row(uint32_t pcid, int32 *srid)
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
  /* 0 is what liblwgeom spells SRID_UNKNOWN, whose header this file does not
   * read, and what a pointcloud_formats row declaring no SRID carries */
  *srid = 0;
  HeapTuple tuple = systable_getnext(scan);
  if (HeapTupleIsValid(tuple))
  {
    bool isnull;
    /* srid is the second column (attnum 2) */
    Datum srid_datum = heap_getattr(tuple, 2, tupDesc, &isnull);
    if (! isnull)
      *srid = DatumGetInt32(srid_datum);
    /* schema is the third column (attnum 3) */
    Datum xml_datum = heap_getattr(tuple, 3, tupDesc, &isnull);
    if (! isnull)
      /* TextDatumGetCString without pulling utils/builtins.h: detoast then
       * use MEOS's text_to_cstring (declared in pgtypes.h). */
      xml = text_to_cstring(DatumGetTextPP(xml_datum));
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

/**
 * @brief Return the install namespace of the mobilitydb extension, or
 *   @c InvalidOid where it cannot be determined
 */
static Oid
mobilitydb_schema_namespace_oid(void)
{
  Oid ext_oid = get_extension_oid("mobilitydb", /* missing_ok */ true);
  if (ext_oid == InvalidOid)
    return InvalidOid;
  return get_extension_schema(ext_oid);
}

/**
 * @brief Build the schema that @p pcid names from the rows stating it
 * @details Reads @c pointcloud_schemas and @c pointcloud_dimensions by direct
 *   heap scan, the way the XML path reads @c pointcloud_formats. The schema is
 *   built in @c TopMemoryContext so that it outlives the current query, as the
 *   MEOS cache holds the pointer for the lifetime of the backend.
 * @param[in] pcid Identifier the rows state the schema of
 * @return The schema, or @c NULL where no row states it, which is what sends
 *   the caller to the XML path
 */
static PCSCHEMA *
fetch_schema_dims(uint32_t pcid)
{
  Oid nsp_oid = mobilitydb_schema_namespace_oid();
  if (nsp_oid == InvalidOid)
    return NULL;
  Oid rel_oid = get_relname_relid("pointcloud_schemas", nsp_oid);
  Oid dim_oid = get_relname_relid("pointcloud_dimensions", nsp_oid);
  if (rel_oid == InvalidOid || dim_oid == InvalidOid)
    return NULL;

  /* Column layout: (pcid int4, srid int4, compression text, description text) */
  Relation rel = table_open(rel_oid, AccessShareLock);
  TupleDesc tup_desc = RelationGetDescr(rel);
  ScanKeyData key[1];
  ScanKeyInit(&key[0], 1, BTEqualStrategyNumber, F_INT4EQ,
    Int32GetDatum((int32) pcid));
  SysScanDesc scan = systable_beginscan(rel, InvalidOid, false, NULL, 1, key);

  bool found = false, isnull;
  int32 srid = 0;
  char *compression = NULL;
  HeapTuple tuple = systable_getnext(scan);
  if (HeapTupleIsValid(tuple))
  {
    found = true;
    Datum datum = heap_getattr(tuple, 2, tup_desc, &isnull);
    if (! isnull)
      srid = DatumGetInt32(datum);
    datum = heap_getattr(tuple, 3, tup_desc, &isnull);
    if (! isnull)
      compression = text_to_cstring(DatumGetTextPP(datum));
  }
  systable_endscan(scan);
  table_close(rel, AccessShareLock);
  if (! found)
    return NULL;

  /* Column layout: (pcid int4, position int4, name text, interpretation text,
   * scale float8, offset float8, active bool, description text) */
  rel = table_open(dim_oid, AccessShareLock);
  tup_desc = RelationGetDescr(rel);
  ScanKeyInit(&key[0], 1, BTEqualStrategyNumber, F_INT4EQ,
    Int32GetDatum((int32) pcid));
  scan = systable_beginscan(rel, InvalidOid, false, NULL, 1, key);

  int ndims = 0, capacity = 8;
  PCDimensionSpec *dims = palloc0(sizeof(PCDimensionSpec) * capacity);
  while (HeapTupleIsValid(tuple = systable_getnext(scan)))
  {
    if (ndims == capacity)
    {
      capacity *= 2;
      dims = repalloc(dims, sizeof(PCDimensionSpec) * capacity);
    }
    PCDimensionSpec *spec = &dims[ndims++];
    Datum datum = heap_getattr(tuple, 2, tup_desc, &isnull);
    spec->position = isnull ? 0 : DatumGetInt32(datum);
    datum = heap_getattr(tuple, 3, tup_desc, &isnull);
    spec->name = isnull ? NULL : text_to_cstring(DatumGetTextPP(datum));
    datum = heap_getattr(tuple, 4, tup_desc, &isnull);
    spec->interpretation = isnull ? NULL :
      text_to_cstring(DatumGetTextPP(datum));
    datum = heap_getattr(tuple, 5, tup_desc, &isnull);
    spec->scale = isnull ? 1.0 : DatumGetFloat8(datum);
    datum = heap_getattr(tuple, 6, tup_desc, &isnull);
    spec->offset = isnull ? 0.0 : DatumGetFloat8(datum);
    datum = heap_getattr(tuple, 7, tup_desc, &isnull);
    spec->active = isnull ? true : DatumGetBool(datum);
    datum = heap_getattr(tuple, 8, tup_desc, &isnull);
    spec->description = isnull ? NULL : text_to_cstring(DatumGetTextPP(datum));
  }
  systable_endscan(scan);
  table_close(rel, AccessShareLock);

  if (ndims == 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The point cloud schema of pcid %u states no dimension", pcid)));

  MemoryContext old_ctx = MemoryContextSwitchTo(TopMemoryContext);
  PCSCHEMA *result = meos_pc_schema_from_dims(pcid, srid, compression, dims,
    ndims);
  MemoryContextSwitchTo(old_ctx);
  return result;
}

PCSCHEMA *
mobilitydb_pc_schema(uint32_t pcid)
{
  /* The rows state the schema without an XML document, and the path below
   * errors where pgpointcloud is not installed, so they are read first */
  PCSCHEMA *stated = fetch_schema_dims(pcid);
  if (stated)
    return stated;

  int32 srid;
  char *xml = fetch_schema_row(pcid, &srid);
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

  /* pc_schema_from_xml populates neither the pcid nor the srid — both are
   * columns of pointcloud_formats rather than elements of the XML, and the
   * pgpointcloud loader assigns both after its own fetch. */
  schema->pcid = pcid;
  schema->srid = (uint32_t) srid;
  /* Register both parsed schema AND XML — the MEOS WKB encoder needs
   * the XML to embed in cross-cluster-portable WKB blobs. */
  meos_pc_schema_register_xml(pcid, schema, xml);
  pfree(xml);
  return schema;
}
