/***********************************************************************
 * pc_pgsql.c
 *
 *  Utility functions to bind pc_api.h functions to PgSQL, including
 *  memory management and serialization/deserializations.
 *
 *  PgSQL Pointcloud is free and open source software provided
 *  by the Government of Canada
 *  Copyright (c) 2013 Natural Resources Canada
 *
 ***********************************************************************/

#include "pc_pgsql.h"

#include "access/hash.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "access/sysattr.h"

#include "catalog/indexing.h"
#include "catalog/namespace.h"
#include "catalog/pg_extension.h"

#include "commands/extension.h"

#include "executor/spi.h"

#include "utils/fmgroids.h"
#include "utils/hsearch.h"
#include "utils/lsyscache.h"
#include "utils/memutils.h"
#include "utils/regproc.h"

#include <assert.h>

PG_MODULE_MAGIC;

/**********************************************************************************
 * Pointcloud constants cache (mainly based on PostGIS source code)
 */

static PC_CONSTANTS *pc_constants_cache = NULL;

static Oid pointcloud_get_full_version_schema()
{
  const char *proname = "pointcloud_full_version";
#if PGSQL_VERSION < 160
  List *names = stringToQualifiedNameList(proname);
#else
  List *names = stringToQualifiedNameList(proname, NULL);
#endif
#if PGSQL_VERSION < 140
  FuncCandidateList clist =
      FuncnameGetCandidates(names, -1, NIL, false, false, false);
#else
  FuncCandidateList clist =
      FuncnameGetCandidates(names, -1, NIL, false, false, false, false);
#endif
  if (!clist)
    return InvalidOid;

  return get_func_namespace(clist->oid);
}

static Oid pointcloud_get_extension_schema(Oid ext_oid)
{
  Oid result;
  SysScanDesc scandesc;
  HeapTuple tuple;
  ScanKeyData entry[1];

#if PGSQL_VERSION < 120
  Relation rel = heap_open(ExtensionRelationId, AccessShareLock);
  ScanKeyInit(&entry[0], ObjectIdAttributeNumber, BTEqualStrategyNumber,
              F_OIDEQ, ObjectIdGetDatum(ext_oid));
#else
  Relation rel = table_open(ExtensionRelationId, AccessShareLock);
  ScanKeyInit(&entry[0], Anum_pg_extension_oid, BTEqualStrategyNumber, F_OIDEQ,
              ObjectIdGetDatum(ext_oid));
#endif

  scandesc = systable_beginscan(rel, ExtensionOidIndexId, true, NULL, 1, entry);

  tuple = systable_getnext(scandesc);

  /* We assume that there can be at most one matching tuple */
  if (HeapTupleIsValid(tuple))
    result = ((Form_pg_extension)GETSTRUCT(tuple))->extnamespace;
  else
    result = InvalidOid;

  systable_endscan(scandesc);

#if PGSQL_VERSION < 120
  heap_close(rel, AccessShareLock);
#else
  table_close(rel, AccessShareLock);
#endif

  return result;
}

void pointcloud_init_constants_cache(void)
{
  char *nsp_name;
  Oid ext_oid;
  Oid nsp_oid = InvalidOid;
  MemoryContext context;

  if (pc_constants_cache)
    return;

  ext_oid = get_extension_oid("pointcloud", true);
  if (ext_oid != InvalidOid)
  {
    nsp_oid = pointcloud_get_extension_schema(ext_oid);
  }
  else
  {
    nsp_oid = pointcloud_get_full_version_schema();
  }

  /* early exit if we cannot lookup nsp_name */
  if (nsp_oid == InvalidOid)
    elog(ERROR, "Unable to determine 'pointcloud' install schema");

  nsp_name = get_namespace_name(nsp_oid);

  /* Put constants cache in a child of the CacheContext */
  context = AllocSetContextCreate(
      CacheMemoryContext, "Pointcloud Constants Context", ALLOCSET_SMALL_SIZES);

  /* Allocate in the CacheContext so we don't lose this at the end of the
   * statement */
  pc_constants_cache = MemoryContextAlloc(context, sizeof(PC_CONSTANTS));

  pc_constants_cache->schema =
      MemoryContextStrdup(CacheMemoryContext, nsp_name);

  pc_constants_cache->formats =
      MemoryContextStrdup(CacheMemoryContext, "pointcloud_formats");
  pc_constants_cache->formats_srid =
      MemoryContextStrdup(CacheMemoryContext, "srid");
  pc_constants_cache->formats_schema =
      MemoryContextStrdup(CacheMemoryContext, "schema");
}

/**********************************************************************************
 * POSTGRESQL MEMORY MANAGEMENT HOOKS
 */

static void *pgsql_alloc(size_t size)
{
  void *result;
  result = palloc(size);

  if (!result)
  {
    ereport(ERROR,
            (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Out of virtual memory")));
  }

  return result;
}

static void *pgsql_realloc(void *mem, size_t size)
{
  void *result;
  result = repalloc(mem, size);
  if (!result)
  {
    ereport(ERROR,
            (errcode(ERRCODE_OUT_OF_MEMORY), errmsg("Out of virtual memory")));
  }
  return result;
}

static void pgsql_free(void *ptr) { pfree(ptr); }

static void pgsql_msg_handler(int sig, const char *fmt, va_list ap)
    __attribute__((format(printf, 2, 0)));

static void pgsql_msg_handler(int sig, const char *fmt, va_list ap)
{
#define MSG_MAXLEN 1024
  char msg[MSG_MAXLEN] = {0};
  vsnprintf(msg, MSG_MAXLEN, fmt, ap);
  msg[MSG_MAXLEN - 1] = '\0';
  ereport(sig, (errmsg_internal("%s", msg)));
}

static void pgsql_error(const char *fmt, va_list ap)
    __attribute__((format(printf, 1, 0)));

static void pgsql_error(const char *fmt, va_list ap)
{
  pgsql_msg_handler(ERROR, fmt, ap);
}

static void pgsql_warn(const char *fmt, va_list ap)
    __attribute__((format(printf, 1, 0)));

static void pgsql_warn(const char *fmt, va_list ap)
{
  pgsql_msg_handler(WARNING, fmt, ap);
}

static void pgsql_info(const char *fmt, va_list ap)
    __attribute__((format(printf, 1, 0)));

static void pgsql_info(const char *fmt, va_list ap)
{
  pgsql_msg_handler(NOTICE, fmt, ap);
}

/**********************************************************************************
 * POINTCLOUD START-UP/SHUT-DOWN CALLBACKS
 */

/**
 * On module load we want to hook the message writing and memory allocation
 * functions of libpc to the PostgreSQL ones.
 * TODO: also hook the libxml2 hooks into PostgreSQL.
 */
void _PG_init(void);
void _PG_init(void)
{
  elog(LOG, "Pointcloud (%s) module loaded", POINTCLOUD_VERSION);
  pc_set_handlers(pgsql_alloc, pgsql_realloc, pgsql_free, pgsql_error,
                  pgsql_info, pgsql_warn);
}

/* Module unload callback */
void _PG_fini(void);
void _PG_fini(void)
{
  elog(LOG, "Pointcloud (%s) module unloaded", POINTCLOUD_VERSION);
}

/* Mask pcid from bottom of typmod */
uint32 pcid_from_typmod(const int32 typmod)
{
  if (typmod == -1)
    return 0;
  else
    return (typmod & 0x0000FFFF);
}

/**********************************************************************************
 * PCPOINT WKB Handling
 */

PCPOINT *
#if PGSQL_VERSION < 120
pc_point_from_hexwkb(const char *hexwkb, size_t hexlen,
                     FunctionCallInfoData *fcinfo)
#else
pc_point_from_hexwkb(const char *hexwkb, size_t hexlen, FunctionCallInfo fcinfo)
#endif
{
  PCPOINT *pt;
  PCSCHEMA *schema;
  uint32 pcid;
  uint8 *wkb = pc_bytes_from_hexbytes(hexwkb, hexlen);
  size_t wkblen = hexlen / 2;
  pcid = pc_wkb_get_pcid(wkb);
  schema = pc_schema_from_pcid(pcid, fcinfo);
  pt = pc_point_from_wkb(schema, wkb, wkblen);
  pfree(wkb);
  return pt;
}

char *pc_point_to_hexwkb(const PCPOINT *pt)
{
  uint8 *wkb;
  size_t wkb_size;
  char *hexwkb;

  wkb = pc_point_to_wkb(pt, &wkb_size);
  hexwkb = pc_hexbytes_from_bytes(wkb, wkb_size);
  pfree(wkb);
  return hexwkb;
}

/**********************************************************************************
 * PCPATCH WKB Handling
 */

PCPATCH *
#if PGSQL_VERSION < 120
pc_patch_from_hexwkb(const char *hexwkb, size_t hexlen,
                     FunctionCallInfoData *fcinfo)
#else
pc_patch_from_hexwkb(const char *hexwkb, size_t hexlen, FunctionCallInfo fcinfo)
#endif
{
  PCPATCH *patch;
  PCSCHEMA *schema;
  uint32 pcid;
  uint8 *wkb = pc_bytes_from_hexbytes(hexwkb, hexlen);
  size_t wkblen = hexlen / 2;
  pcid = pc_wkb_get_pcid(wkb);
  if (!pcid)
    elog(ERROR, "%s: pcid is zero", __func__);

  schema = pc_schema_from_pcid(pcid, fcinfo);
  if (!schema)
    elog(ERROR, "%s: unable to look up schema entry", __func__);

  patch = pc_patch_from_wkb(schema, wkb, wkblen);
  pfree(wkb);
  return patch;
}

char *pc_patch_to_hexwkb(const PCPATCH *patch)
{
  uint8 *wkb;
  size_t wkb_size;
  char *hexwkb;

  wkb = pc_patch_to_wkb(patch, &wkb_size);
  hexwkb = pc_hexbytes_from_bytes(wkb, wkb_size);
  pfree(wkb);
  return hexwkb;
}

/**********************************************************************************
 * PCID <=> PCSCHEMA translation via POINTCLOUD_FORMATS
 */

uint32 pcid_from_datum(Datum d)
{
  SERIALIZED_POINT *serpart;
  if (!d)
    return 0;
  /* Serializations are int32_t <size> uint32_t <pcid> == 8 bytes */
  /* Cast to SERIALIZED_POINT for convenience, SERIALIZED_PATCH shares same
   * header */
  serpart = (SERIALIZED_POINT *)PG_DETOAST_DATUM_SLICE(d, 0, 8);
  return serpart->pcid;
}

PCSCHEMA *pc_schema_from_pcid_uncached(uint32 pcid)
{
  char sql[256];
  char *xml, *xml_spi, *srid_spi, *formats;
  int err, srid;
  size_t size;
  PCSCHEMA *schema;

  if (SPI_OK_CONNECT != SPI_connect())
  {
    elog(ERROR, "%s: could not connect to SPI manager", __func__);
    return NULL;
  }

  if (!pc_constants_cache)
  {
    elog(ERROR, "%s: constants cache is not initialized", __func__);
    return NULL;
  }

  formats = quote_qualified_identifier(pc_constants_cache->schema,
                                       pc_constants_cache->formats);
  sprintf(sql, "select %s, %s from %s where pcid = %d",
          pc_constants_cache->formats_schema, pc_constants_cache->formats_srid,
          formats, pcid);
  err = SPI_exec(sql, 1);

  if (err < 0)
  {
    elog(ERROR, "%s: error (%d) executing query: %s", __func__, err, sql);
    return NULL;
  }

  /* No entry in POINTCLOUD_FORMATS */
  if (SPI_processed <= 0)
  {
    elog(ERROR, "no entry in \"%s\" for pcid = %d", formats, pcid);
    return NULL;
  }

  /* Result  */
  xml_spi = SPI_getvalue(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1);
  srid_spi = SPI_getvalue(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 2);

  /* NULL result */
  if (!(xml_spi && srid_spi))
  {
    elog(ERROR, "unable to read row from \"%s\" for pcid = %d", formats, pcid);
    return NULL;
  }

  /* Copy result to upper executor context */
  size = strlen(xml_spi) + 1;
  xml = SPI_palloc(size);
  memcpy(xml, xml_spi, size);

  /* Parse the SRID string into the function stack */
  srid = atoi(srid_spi);

  /* Disconnect from SPI, losing all our SPI-allocated memory now... */
  SPI_finish();

  /* Build the schema object */
  schema = pc_schema_from_xml(xml);

  if (!schema)
  {
    ereport(ERROR, (errcode(ERRCODE_NOT_AN_XML_DOCUMENT),
                    errmsg("unable to parse XML for pcid = %d in \"%s\"", pcid,
                           formats)));
  }

  schema->pcid = pcid;
  schema->srid = srid;

  return schema;
}

/**
 * Hold the schema references in a list.
 * We'll just search them linearly, because
 * usually we'll have only one per statement
 */
#define SchemaCacheSize 16

typedef struct
{
  int next_slot;
  int pcids[SchemaCacheSize];
  PCSCHEMA *schemas[SchemaCacheSize];
} SchemaCache;

/**
 * Get the schema entry from the schema cache if one exists.
 * If it doesn't exist, make a new empty one, cache it, and
 * return it.
 */
static SchemaCache *
#if PGSQL_VERSION < 120
GetSchemaCache(FunctionCallInfoData *fcinfo)
#else
GetSchemaCache(FunctionCallInfo fcinfo)
#endif
{
  SchemaCache *cache = fcinfo->flinfo->fn_extra;
  if (!cache)
  {
    cache = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(SchemaCache));
    memset(cache, 0, sizeof(SchemaCache));
    fcinfo->flinfo->fn_extra = cache;
  }
  return cache;
}

PCSCHEMA *
#if PGSQL_VERSION < 120
pc_schema_from_pcid(uint32 pcid, FunctionCallInfoData *fcinfo)
#else
pc_schema_from_pcid(uint32 pcid, FunctionCallInfo fcinfo)
#endif
{
  SchemaCache *schema_cache = GetSchemaCache(fcinfo);
  int i;
  PCSCHEMA *schema;
  MemoryContext oldcontext;

  /* Unable to find/make a schema cache? Odd. */
  if (!schema_cache)
  {
    ereport(ERROR,
            (errcode(ERRCODE_INTERNAL_ERROR),
             errmsg("unable to create/load statement level schema cache")));
  }

  /* Find our PCID if it's in there (usually it will be first) */
  for (i = 0; i < SchemaCacheSize; i++)
  {
    if (schema_cache->pcids[i] == pcid)
    {
      return schema_cache->schemas[i];
    }
  }

  elog(DEBUG1, "schema cache miss, use pc_schema_from_pcid_uncached");

  /* Not in there, load one the old-fashioned way. */
  oldcontext = MemoryContextSwitchTo(fcinfo->flinfo->fn_mcxt);
  pointcloud_init_constants_cache();
  schema = pc_schema_from_pcid_uncached(pcid);
  MemoryContextSwitchTo(oldcontext);

  /* Failed to load the XML? Odd. */
  if (!schema)
  {
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
                    errmsg("unable to load schema for pcid %u", pcid)));
  }

  /* Save the XML in the next unused slot */
  schema_cache->schemas[schema_cache->next_slot] = schema;
  schema_cache->pcids[schema_cache->next_slot] = pcid;
  schema_cache->next_slot = (schema_cache->next_slot + 1) % SchemaCacheSize;
  return schema;
}

/**********************************************************************************
 * SERIALIZATION/DESERIALIZATION UTILITIES
 */

SERIALIZED_POINT *pc_point_serialize(const PCPOINT *pcpt)
{
  size_t serpt_size = sizeof(SERIALIZED_POINT) - 1 + pcpt->schema->size;
  SERIALIZED_POINT *serpt = palloc(serpt_size);
  serpt->pcid = pcpt->schema->pcid;
  memcpy(serpt->data, pcpt->data, pcpt->schema->size);
  SET_VARSIZE(serpt, serpt_size);
  return serpt;
}

PCPOINT *pc_point_deserialize(const SERIALIZED_POINT *serpt,
                              const PCSCHEMA *schema)
{
  PCPOINT *pcpt;
  size_t pgsize = VARSIZE(serpt) + 1 - sizeof(SERIALIZED_POINT);
  /*
   * Big problem, the size on disk doesn't match what we expect,
   * so we cannot reliably interpret the contents.
   */
  if (schema->size != pgsize)
  {
    elog(ERROR, "schema size and disk size mismatch, repair the schema");
    return NULL;
  }
  pcpt = pc_point_from_data(schema, serpt->data);
  return pcpt;
}

size_t pc_patch_serialized_size(const PCPATCH *patch)
{
  size_t stats_size = pc_stats_size(patch->schema);
  size_t common_size = BUFFERALIGN(sizeof(SERIALIZED_PATCH)) - 1;
  switch (patch->type)
  {
  case PC_NONE:
  {
    PCPATCH_UNCOMPRESSED *pu = (PCPATCH_UNCOMPRESSED *)patch;
    return common_size + stats_size + pu->datasize;
  }
  case PC_DIMENSIONAL:
  {
    return common_size + stats_size +
           pc_patch_dimensional_serialized_size((PCPATCH_DIMENSIONAL *)patch);
  }
  case PC_LAZPERF:
  {
    static size_t lazsize_size = 4;
    PCPATCH_LAZPERF *pg = (PCPATCH_LAZPERF *)patch;
    return common_size + stats_size + lazsize_size + pg->lazperfsize;
  }
  default:
  {
    pcerror("%s: unknown compresed %d", __func__, patch->type);
  }
  }
  return -1;
}

static size_t pc_patch_stats_serialize(uint8_t *buf, const PCSCHEMA *schema,
                                       const PCSTATS *stats)
{
  size_t sz = schema->size;
  /* Copy min */
  memcpy(buf, stats->min.data, sz);
  /* Copy max */
  memcpy(buf + sz, stats->max.data, sz);
  /* Copy avg */
  memcpy(buf + 2 * sz, stats->avg.data, sz);

  return sz * 3;
}

/**
 * Stats are always three PCPOINT serializations in a row,
 * min, max, avg. Their size is the uncompressed buffer size for
 * a point, the schema->size.
 */
PCSTATS *pc_patch_stats_deserialize(const PCSCHEMA *schema, const uint8_t *buf)
{
  size_t sz = schema->size;
  const uint8_t *buf_min = buf;
  const uint8_t *buf_max = buf + sz;
  const uint8_t *buf_avg = buf + 2 * sz;

  return pc_stats_new_from_data(schema, buf_min, buf_max, buf_avg);
}

static SERIALIZED_PATCH *pc_patch_dimensional_serialize(const PCPATCH *patch_in)
{
  //  uint32_t size;
  //  uint32_t pcid;
  //  uint32_t compression;
  //  uint32_t npoints;
  //  double xmin, xmax, ymin, ymax;
  //  data:
  //    pcpoint[3] stats;
  //    serialized_pcbytes[ndims] dimensions;

  int i;
  uint8_t *buf;
  size_t serpch_size = pc_patch_serialized_size(patch_in);
  SERIALIZED_PATCH *serpch = pcalloc(serpch_size);
  const PCPATCH_DIMENSIONAL *patch = (PCPATCH_DIMENSIONAL *)patch_in;

  assert(patch_in);
  assert(patch_in->type == PC_DIMENSIONAL);

  /* Copy basics */
  serpch->pcid = patch->schema->pcid;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;
  serpch->compression = patch->type;

  /* Get a pointer to the data area */
  buf = serpch->data;

  /* Write stats into the buffer */
  if (patch->stats)
  {
    buf += pc_patch_stats_serialize(buf, patch->schema, patch->stats);
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  /* Write each dimension in after the stats */
  for (i = 0; i < patch->schema->ndims; i++)
  {
    size_t bsize = 0;
    PCBYTES *pcb = &(patch->bytes[i]);
    pc_bytes_serialize(pcb, buf, &bsize);
    buf += bsize;
  }

  SET_VARSIZE(serpch, serpch_size);
  return serpch;
}

static SERIALIZED_PATCH *pc_patch_lazperf_serialize(const PCPATCH *patch_in)
{
  size_t serpch_size = pc_patch_serialized_size(patch_in);
  SERIALIZED_PATCH *serpch = pcalloc(serpch_size);
  const PCPATCH_LAZPERF *patch = (PCPATCH_LAZPERF *)patch_in;
  uint32_t lazsize = patch->lazperfsize;
  uint8_t *buf = serpch->data;

  assert(patch);
  assert(patch->type == PC_LAZPERF);

  /* Copy basics */
  serpch->pcid = patch->schema->pcid;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;
  serpch->compression = patch->type;

  /* Write stats into the buffer first */
  if (patch->stats)
  {
    buf += pc_patch_stats_serialize(buf, patch->schema, patch->stats);
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  /* Write buffer size */
  memcpy(buf, &(lazsize), 4);
  buf += 4;

  /* Write buffer */
  memcpy(buf, patch->lazperf, patch->lazperfsize);
  SET_VARSIZE(serpch, serpch_size);

  return serpch;
}

static SERIALIZED_PATCH *
pc_patch_uncompressed_serialize(const PCPATCH *patch_in)
{
  //  uint32_t size;
  //  uint32_t pcid;
  //  uint32_t compression;
  //  uint32_t npoints;
  //  double xmin, xmax, ymin, ymax;
  //  data:
  //    pcpoint [];

  uint8_t *buf;
  size_t serpch_size;
  SERIALIZED_PATCH *serpch;
  const PCPATCH_UNCOMPRESSED *patch = (PCPATCH_UNCOMPRESSED *)patch_in;

  serpch_size = pc_patch_serialized_size(patch_in);
  serpch = pcalloc(serpch_size);

  /* Copy basic */
  serpch->compression = patch->type;
  serpch->pcid = patch->schema->pcid;
  serpch->npoints = patch->npoints;
  serpch->bounds = patch->bounds;

  /* Write stats into the buffer first */
  buf = serpch->data;
  if (patch->stats)
  {
    buf += pc_patch_stats_serialize(buf, patch->schema, patch->stats);
  }
  else
  {
    pcerror("%s: stats missing!", __func__);
  }

  /* Copy point list into data buffer */
  memcpy(buf, patch->data, patch->datasize);
  SET_VARSIZE(serpch, serpch_size);
  return serpch;
}

/**
 * Convert struct to byte array.
 * Userdata is currently only PCDIMSTATS, hopefully updated across
 * a number of iterations and saved.
 */
SERIALIZED_PATCH *pc_patch_serialize(const PCPATCH *patch_in, void *userdata)
{
  PCPATCH *patch = (PCPATCH *)patch_in;
  SERIALIZED_PATCH *serpatch = NULL;
  /*
   * Ensure the patch has stats calculated before going on
   */
  if (!patch->stats)
  {
    pcerror("%s: patch is missing stats", __func__);
    return NULL;
  }
  /*
   * Convert the patch to the final target compression,
   * which is the one in the schema.
   */
  if (patch->type != patch->schema->compression)
  {
    patch = pc_patch_compress(patch_in, userdata);
  }

  switch (patch->type)
  {
  case PC_NONE:
  {
    serpatch = pc_patch_uncompressed_serialize(patch);
    break;
  }
  case PC_DIMENSIONAL:
  {
    serpatch = pc_patch_dimensional_serialize(patch);
    break;
  }
  case PC_LAZPERF:
  {
    serpatch = pc_patch_lazperf_serialize(patch);
    break;
  }
  default:
  {
    pcerror("%s: unsupported compression type %d", __func__, patch->type);
  }
  }

  if (patch != patch_in)
    pc_patch_free(patch);

  return serpatch;
}

/**
 * Convert struct to byte array.
 * Userdata is currently only PCDIMSTATS, hopefully updated across
 * a number of iterations and saved.
 */
SERIALIZED_PATCH *pc_patch_serialize_to_uncompressed(const PCPATCH *patch_in)
{
  PCPATCH *patch = (PCPATCH *)patch_in;
  SERIALIZED_PATCH *serpatch;

  /*  Convert the patch to uncompressed, if necessary */
  if (patch->type != PC_NONE)
  {
    patch = pc_patch_uncompress(patch_in);
  }

  serpatch = pc_patch_uncompressed_serialize(patch);

  /* An uncompressed input won't result in a copy */
  if (patch != patch_in)
    pc_patch_free(patch);

  return serpatch;
}

static PCPATCH *
pc_patch_uncompressed_deserialize(const SERIALIZED_PATCH *serpatch,
                                  const PCSCHEMA *schema)
{
  // typedef struct
  // {
  //  uint32_t size;
  //  uint32_t pcid;
  //  uint32_t compression;
  //  uint32_t npoints;
  //  double xmin, xmax, ymin, ymax;
  //  data:
  //    pcpoint[3] pcstats(min, max, avg)
  //    pcpoint[npoints]
  // }
  // SERIALIZED_PATCH;

  uint8_t *buf;
  size_t stats_size = pc_stats_size(schema); // 3 pcpoints worth of stats
  PCPATCH_UNCOMPRESSED *patch = pcalloc(sizeof(PCPATCH_UNCOMPRESSED));

  /* Set up basic info */
  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = serpatch->npoints;
  patch->maxpoints = 0;
  patch->bounds = serpatch->bounds;

  buf = (uint8_t *)serpatch->data;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, buf);

  /* Advance data pointer past the stats serialization */
  patch->data = buf + stats_size;

  /* Calculate the point data buffer size */
  patch->datasize = VARSIZE(serpatch) - BUFFERALIGN(sizeof(SERIALIZED_PATCH)) +
                    1 - stats_size;
  if (patch->datasize != patch->npoints * schema->size)
    pcerror("%s: calculated patch data sizes don't match (%d != %d)", __func__,
            patch->datasize, patch->npoints * schema->size);

  return (PCPATCH *)patch;
}

static PCPATCH *
pc_patch_dimensional_deserialize(const SERIALIZED_PATCH *serpatch,
                                 const PCSCHEMA *schema)
{
  // typedef struct
  // {
  //  uint32_t size;
  //  uint32_t pcid;
  //  uint32_t compression;
  //  uint32_t npoints;
  //  double xmin, xmax, ymin, ymax;
  //  data:
  //    pcpoint[3] pcstats(min, max, avg)
  //    pcbytes[ndims];
  // }
  // SERIALIZED_PATCH;

  PCPATCH_DIMENSIONAL *patch;
  int i;
  const uint8_t *buf;
  int ndims = schema->ndims;
  int npoints = serpatch->npoints;
  size_t stats_size = pc_stats_size(schema); // 3 pcpoints worth of stats

  /* Reference the external data */
  patch = pcalloc(sizeof(PCPATCH_DIMENSIONAL));

  /* Set up basic info */
  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = npoints;
  patch->bounds = serpatch->bounds;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, serpatch->data);

  /* Set up dimensions */
  patch->bytes = pcalloc(ndims * sizeof(PCBYTES));
  buf = serpatch->data + stats_size;

  for (i = 0; i < ndims; i++)
  {
    PCBYTES *pcb = &(patch->bytes[i]);
    PCDIMENSION *dim = schema->dims[i];
    pc_bytes_deserialize(buf, dim, pcb, true /*readonly*/,
                         false /*flipendian*/);
    pcb->npoints = npoints;
    buf += pc_bytes_serialized_size(pcb);
  }

  return (PCPATCH *)patch;
}

/*
 * We don't do any radical deserialization here. Don't build out the tree, just
 * set up pointers to the start of the buffer, so we can build it out later
 * if necessary.
 */
static PCPATCH *pc_patch_lazperf_deserialize(const SERIALIZED_PATCH *serpatch,
                                             const PCSCHEMA *schema)
{
  PCPATCH_LAZPERF *patch;
  uint32_t lazperfsize;
  int npoints = serpatch->npoints;
  size_t stats_size = pc_stats_size(schema);
  uint8_t *buf = (uint8_t *)serpatch->data + stats_size;

  /* Reference the external data */
  patch = pcalloc(sizeof(PCPATCH_LAZPERF));

  /* Set up basic info */
  patch->type = serpatch->compression;
  patch->schema = schema;
  patch->readonly = true;
  patch->npoints = npoints;
  patch->bounds = serpatch->bounds;

  /* Point into the stats area */
  patch->stats = pc_patch_stats_deserialize(schema, serpatch->data);

  /* Set up buffer */
  memcpy(&lazperfsize, buf, 4);
  patch->lazperfsize = lazperfsize;
  buf += 4;

  patch->lazperf = pcalloc(patch->lazperfsize);
  memcpy(patch->lazperf, buf, patch->lazperfsize);

  return (PCPATCH *)patch;
}

PCPATCH *pc_patch_deserialize(const SERIALIZED_PATCH *serpatch,
                              const PCSCHEMA *schema)
{
  switch (serpatch->compression)
  {
  case PC_NONE:
    return pc_patch_uncompressed_deserialize(serpatch, schema);
  case PC_DIMENSIONAL:
    return pc_patch_dimensional_deserialize(serpatch, schema);
  case PC_LAZPERF:
    return pc_patch_lazperf_deserialize(serpatch, schema);
  }
  pcerror("%s: unsupported compression type", __func__);
  return NULL;
}

static uint8_t *pc_patch_wkb_set_double(uint8_t *wkb, double d)
{
  memcpy(wkb, &d, 8);
  wkb += 8;
  return wkb;
}

static uint8_t *pc_patch_wkb_set_int32(uint8_t *wkb, uint32_t i)
{
  memcpy(wkb, &i, 4);
  wkb += 4;
  return wkb;
}

static uint8_t *pc_patch_wkb_set_char(uint8_t *wkb, char c)
{
  memcpy(wkb, &c, 1);
  wkb += 1;
  return wkb;
}

/* 0 = xdr | big endian */
/* 1 = ndr | little endian */
static char machine_endian(void)
{
  static int check_int = 1; /* dont modify this!!! */
  return *((char *)&check_int);
}

uint8_t *pc_patch_to_geometry_wkb_envelope(const SERIALIZED_PATCH *pa,
                                           const PCSCHEMA *schema,
                                           size_t *wkbsize)
{
  static uint32_t srid_mask = 0x20000000;
  static uint32_t nrings = 1;
  static uint32_t npoints = 5;
  uint32_t wkbtype = 3; /* WKB POLYGON */
  uint8_t *wkb, *ptr;
  int has_srid = false;
  size_t size =
      1 + 4 + 4 + 4 +
      2 * npoints * 8; /* endian + type + nrings + npoints + 5 dbl pts */

  /* Bounds! */
  double xmin = pa->bounds.xmin;
  double ymin = pa->bounds.ymin;
  double xmax = pa->bounds.xmax;
  double ymax = pa->bounds.ymax;

  /* Make sure they're slightly bigger than a point */
  if (xmin == xmax)
    xmax += xmax * 0.0000001;
  if (ymin == ymax)
    ymax += ymax * 0.0000001;

  if (schema->srid > 0)
  {
    has_srid = true;
    wkbtype |= srid_mask;
    size += 4;
  }

  wkb = palloc(size);
  ptr = wkb;

  ptr = pc_patch_wkb_set_char(ptr, machine_endian()); /* Endian flag */

  ptr = pc_patch_wkb_set_int32(ptr, wkbtype); /* TYPE = Polygon */

  if (has_srid)
  {
    ptr = pc_patch_wkb_set_int32(ptr, schema->srid); /* SRID */
  }

  ptr = pc_patch_wkb_set_int32(ptr, nrings);  /* NRINGS = 1 */
  ptr = pc_patch_wkb_set_int32(ptr, npoints); /* NPOINTS = 5 */

  /* Point 0 */
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.xmin);
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.ymin);

  /* Point 1 */
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.xmin);
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.ymax);

  /* Point 2 */
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.xmax);
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.ymax);

  /* Point 3 */
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.xmax);
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.ymin);

  /* Point 4 */
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.xmin);
  ptr = pc_patch_wkb_set_double(ptr, pa->bounds.ymin);

  if (wkbsize)
    *wkbsize = size;
  return wkb;
}
