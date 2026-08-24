/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief MEOS-owned cache of parsed pgPointCloud PCSCHEMA values,
 *   keyed by pcid.  See @c meos/include/pointcloud/meos_schema_hook.h
 *   for the design rationale.
 */

/* PostgreSQL */
#include <postgres.h>
#if ! MEOS
  #include <utils/memutils.h>  /* TopMemoryContext */
#endif
/* pgPointcloud */
#include "pc_api.h"
#include "pc_api_internal.h"
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"
#include "pointcloud/meos_schema_hook.h"

/*****************************************************************************
 * Cache state
 *****************************************************************************/

/**
 * @brief
 */
typedef struct schema_entry {
  uint32_t pcid;
  PCSCHEMA *schema;
  int32_t srid;      /* cached so callers need not dereference PCSCHEMA */
  char *xml_text;    /* NULL if registered without XML */
} schema_entry;

/* Small dynamic array; linear scan.  Workloads rarely exceed a handful
 * of pcids loaded at once, so a hash table is over-engineering. */
static schema_entry *cache_buf = NULL;
static int cache_count = 0;
static int cache_cap = 0;

meos_pc_schema_fn_t meos_pc_schema_fn = NULL;
meos_pc_parse_xml_fn_t meos_pc_parse_xml_fn = NULL;

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @brief Internal helper — copy @p xml into long-lived memory.
 * @return palloc'd cstring (TopMemoryContext on PG, malloc on standalone)
 * or NULL when @p xml is NULL.
 */
static char *
copy_xml_long_lived(const char *xml)
{
  if (! xml)
    return NULL;
  size_t len = strlen(xml);
#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  char *out = palloc(len + 1);
  memcpy(out, xml, len + 1);
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif
  return out;
}

/**
 * @brief Internal helper — make room for one more cache entry.
 */
static void
ensure_cache_capacity(void)
{
  if (cache_count < cache_cap)
    return;
  int new_cap = cache_cap ? cache_cap * 2 : 8;
#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  schema_entry *new_buf = palloc(sizeof(schema_entry) * new_cap);
  if (cache_buf)
  {
    memcpy(new_buf, cache_buf, sizeof(schema_entry) * cache_count);
    pfree(cache_buf);
  }
  cache_buf = new_buf;
  cache_cap = new_cap;
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register a parsed PCSCHEMA in the MEOS-owned cache.
 */
void
meos_pc_schema_register(uint32_t pcid, PCSCHEMA *schema)
{
  meos_pc_schema_register_xml(pcid, schema, NULL);
}

/**
 * @brief Return a copy of a string allocated by the point cloud library, so
 * that freeing the schema releases it
 */
static char *
pcschema_string_copy(const char *str)
{
  size_t size = strlen(str) + 1;
  char *result = pcalloc(size);
  memcpy(result, str, size);
  return result;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register the schema that a point cloud identifier names, stated as
 *   its dimensions
 * @param[in] pcid Identifier the schema is registered under
 * @param[in] srid Spatial reference identifier every value of the schema
 *   carries
 * @param[in] compression Name of the compression applied to the data,
 *   @p NULL for none
 * @param[in] dims Dimensions the schema states, in any order
 * @param[in] ndims Number of dimensions
 * @return A newly allocated schema, @p NULL on error
 * @note The schema is built through the constructor of the point cloud
 *   library, so the size of a dimension, the offset of a dimension within a
 *   point and the width of a point are the ones that library computes
 */
PCSCHEMA *
meos_pc_schema_from_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) dims))
    return NULL;
  if (ndims < 1)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The point cloud schema of pcid %u must state at least one dimension",
      pcid);
    return NULL;
  }

  int comp = pc_compression_number(compression);
  PCSCHEMA *schema = pc_schema_new((uint32_t) ndims);
  schema->pcid = pcid;
  schema->srid = (uint32_t) srid;
  schema->compression = (uint32_t) comp;

  for (int i = 0; i < ndims; i++)
  {
    const PCDimensionSpec *spec = &dims[i];
    /* A position is stated from 1 while the schema holds it from 0 */
    if (spec->position < 1 || spec->position > ndims)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states position %d, which is "
        "outside the %d dimensions it declares", pcid, spec->position, ndims);
      return NULL;
    }
    if (! spec->name || ! *spec->name)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states a dimension with no name "
        "at position %d", pcid, spec->position);
      return NULL;
    }
    int interp = pc_interpretation_number(spec->interpretation);
    if (interp == PC_UNKNOWN)
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states the unknown interpretation "
        "\"%s\" for dimension \"%s\"", pcid,
        spec->interpretation ? spec->interpretation : "", spec->name);
      return NULL;
    }
    if (schema->dims[spec->position - 1])
    {
      pc_schema_free(schema);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The point cloud schema of pcid %u states position %d twice",
        pcid, spec->position);
      return NULL;
    }

    PCDIMENSION *dim = pcalloc(sizeof(PCDIMENSION));
    dim->position = (uint32_t) spec->position - 1;
    dim->name = pcschema_string_copy(spec->name);
    dim->description = spec->description ?
      pcschema_string_copy(spec->description) : NULL;
    dim->interpretation = (uint32_t) interp;
    dim->scale = spec->scale;
    dim->offset = spec->offset;
    dim->active = spec->active ? 1 : 0;
    /* Sets the dimension, its name in the hash table of the schema, and the
     * size and the byte offset of every dimension */
    pc_schema_set_dimension(schema, dim);
  }

  /* Resolve the X, Y, Z and M dimensions from the names */
  pc_schema_check_xyzm(schema);
  return schema;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register the schema that a point cloud identifier names, stated as
 *   its dimensions
 * @param[in] pcid Identifier the schema is registered under
 * @param[in] srid Spatial reference identifier every value of the schema
 *   carries
 * @param[in] compression Name of the compression applied to the data,
 *   @p NULL for none
 * @param[in] dims Dimensions the schema states, in any order
 * @param[in] ndims Number of dimensions
 * @return True on success, false on error
 * @csqlfn #Pointcloud_schema_register_dims()
 */
bool
meos_pc_schema_register_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims)
{
  PCSCHEMA *schema = meos_pc_schema_from_dims(pcid, srid, compression, dims,
    ndims);
  if (! schema)
    return false;
  meos_pc_schema_register(pcid, schema);
  return true;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register a parsed PCSCHEMA along with its source XML in the
 *   MEOS-owned cache.
 */
void
meos_pc_schema_register_xml(uint32_t pcid, PCSCHEMA *schema,
  const char *xml_text)
{
  /* If already present, replace; preserve previously-cached XML when
   * the new call passes NULL for xml_text (so a parse-only re-register
   * doesn't accidentally drop a prior XML registration). */
  int32_t srid = schema ? (int32_t) schema->srid : SRID_INVALID;
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
    {
      cache_buf[i].schema = schema;
      cache_buf[i].srid = srid;
      if (xml_text)
      {
        if (cache_buf[i].xml_text)
          pfree(cache_buf[i].xml_text);
        cache_buf[i].xml_text = copy_xml_long_lived(xml_text);
      }
      return;
    }
  }
  ensure_cache_capacity();
  cache_buf[cache_count].pcid = pcid;
  cache_buf[cache_count].schema = schema;
  cache_buf[cache_count].srid = srid;
  cache_buf[cache_count].xml_text = copy_xml_long_lived(xml_text);
  cache_count++;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the cached XML text for a registered pcid (NULL on miss
 *   or parse-only registration).
 */
const char *
meos_pc_schema_xml(uint32_t pcid)
{
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
      return cache_buf[i].xml_text;
  }
  return NULL;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Drop every entry from the MEOS schema cache.
 */
void
meos_pc_schema_clear(void)
{
  if (cache_buf)
  {
    for (int i = 0; i < cache_count; i++)
    {
      if (cache_buf[i].xml_text)
        pfree(cache_buf[i].xml_text);
    }
    pfree(cache_buf);
    cache_buf = NULL;
  }
  cache_count = 0;
  cache_cap = 0;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Return the cached SRID for a pcid (SRID_INVALID on miss).
 *
 * The SRID is extracted from the PCSCHEMA at registration time so
 * callers (e.g. spatial_srid in tspatial_srid.c) do not need the full
 * PCSCHEMA struct definition.
 */
int32_t
meos_pc_schema_get_srid(uint32_t pcid)
{
  /* Cache hit */
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
      return cache_buf[i].srid;
  }
  /* Trigger hook so the schema is registered; then retry. */
  PCSCHEMA *s = meos_pc_schema(pcid);
  if (s)
  {
    for (int i = 0; i < cache_count; i++)
    {
      if (cache_buf[i].pcid == pcid)
        return cache_buf[i].srid;
    }
  }
  return SRID_INVALID;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Resolve a parsed PCSCHEMA by pcid, with hook fallback.
 */
PCSCHEMA *
meos_pc_schema(uint32_t pcid)
{
  /* (1) Cache hit */
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
      return cache_buf[i].schema;
  }
  /* (2) Hook fallback (e.g. PG catalog scan).  Registers on success
   * so subsequent lookups hit the cache directly. */
  if (meos_pc_schema_fn)
  {
    PCSCHEMA *s = meos_pc_schema_fn(pcid);
    if (s)
      meos_pc_schema_register(pcid, s);
    return s;
  }
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "PCSCHEMA for pcid %u not registered and no fallback hook "
    "installed — pre-populate via meos_pc_schema_register, or "
    "(in a PG backend) ensure mobilitydb_init has run", pcid);
  return NULL;
}

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Install the pgPointCloud library handlers for standalone MEOS.
 * @details The bundled libpc.a leaves its allocator, deallocator and
 *   message handlers as NULL function pointers until a host installs
 *   them. The PG backend does this in @c mobilitydb_init via
 *   @c pc_set_handlers (palloc-based). A standalone MEOS program has no
 *   such entry point, so the first libpc call that allocates or reports
 *   (@c pc_schema_from_xml, @c pc_point_get_x, …) dereferences a NULL
 *   handler and crashes. Installing the default system allocator/message
 *   handlers here makes a standalone MEOS process symmetric with the PG
 *   backend. Called from @c meos_initialize under @c #if POINTCLOUD.
 */
void
meos_initialize_pointcloud(void)
{
  pc_install_default_handlers();
}

/*****************************************************************************/
