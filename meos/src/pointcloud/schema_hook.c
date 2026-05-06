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

#include <postgres.h>
#if ! MEOS
  #include <utils/memutils.h>  /* TopMemoryContext */
#endif
/* pc_api.h gives the full PCSCHEMA struct definition; it lives in
 * pointcloud-pg/lib/ which is in this target's include path. */
#include "pc_api.h"
#include <meos.h>
/* SRID_INVALID lives in PostGIS liblwgeom.h, which is not in this
 * CMake target's include path.  Define the sentinel locally. */
#ifndef SRID_INVALID
#define SRID_INVALID (-1)
#endif
#include "pointcloud/meos_schema_hook.h"

/*****************************************************************************
 * Cache state
 *****************************************************************************/

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
 *   or NULL when @p xml is NULL.
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
