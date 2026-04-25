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
#include <meos.h>
#include "pointcloud/meos_schema_hook.h"

/*****************************************************************************
 * Cache state
 *****************************************************************************/

typedef struct schema_entry {
  uint32_t pcid;
  PCSCHEMA *schema;
} schema_entry;

/* Small dynamic array; linear scan.  Workloads rarely exceed a handful
 * of pcids loaded at once, so a hash table is over-engineering. */
static schema_entry *cache_buf = NULL;
static int cache_count = 0;
static int cache_cap = 0;

meos_pc_schema_fn_t meos_pc_schema_fn = NULL;

/*****************************************************************************
 * Public API
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_schema_cache
 * @brief Register a parsed PCSCHEMA in the MEOS-owned cache.
 */
void
meos_pc_schema_register(uint32_t pcid, PCSCHEMA *schema)
{
  /* If already present, replace */
  for (int i = 0; i < cache_count; i++)
  {
    if (cache_buf[i].pcid == pcid)
    {
      cache_buf[i].schema = schema;
      return;
    }
  }
  /* Grow if needed.  Allocate in TopMemoryContext (PG backend) so the
   * cache buffer outlives the per-statement context.  In MEOS standalone
   * builds palloc maps to malloc and the explicit context switch is a
   * no-op. */
  if (cache_count == cache_cap)
  {
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
  cache_buf[cache_count].pcid = pcid;
  cache_buf[cache_count].schema = schema;
  cache_count++;
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
    pfree(cache_buf);
    cache_buf = NULL;
  }
  cache_count = 0;
  cache_cap = 0;
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
