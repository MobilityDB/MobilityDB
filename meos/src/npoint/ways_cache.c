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
 * @brief Network-based static point and segment types
 */

#include "npoint/tnpoint.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <libpq/pqformat.h>
#include <executor/spi.h>
#include <utils/memutils.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "geo/tgeo_spatialfuncs.h"

#define SQL_ROUTE_MAXLEN  64

/*****************************************************************************
 * Definitions for reading the ways.csv file
 * Notice that the file does not have header and the separator are tabs
 *****************************************************************************/

/* Maximum length in characters of a geometry in the input data */
#define MAX_LENGTH_GEOM 100001
/* Location of the ways.csv file */
#define WAYS_CSV "/usr/local/share/ways.csv"

typedef struct
{
  int64 gid;
  GSERIALIZED *the_geom;
  double length;
} ways_record;

/* An entry in the Ways cache */
typedef struct struct_WaysCacheItem
{
  int64 gid;
  GSERIALIZED *the_geom;
  double length;
  uint64_t hits;
} WaysCacheItem;

/* Ways lookup transaction cache methods */
#define WAYS_CACHE_ITEMS 128

/**
 * @brief The ways cache holds a fixed number of ways records read from the
 * ways table or CSV file
 * @details In normal usage we do not expect it to have many entries, so we
 * linearly scan the list
 */
typedef struct struct_WaysCache
{
  WaysCacheItem routes[WAYS_CACHE_ITEMS];
  uint32_t count;
  MemoryContext WaysCacheContext;
} WaysCache;

/* Global variable to hold the Ways record cache */
WaysCache *MEOS_WAYS_CACHE = NULL;

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Destroy all the malloc'ed route records stored in the Ways cache and
 * clear the cash
 * @note Function called in MobilityDB when there is a MemoryContext reset
 */
void
DestroyWaysCache(void *cache)
{
 WaysCache *ways_cache = (WaysCache *) cache;
  if (ways_cache)
  {
    for (uint32_t i = 0; i < ways_cache->count; i++)
    {
      if (ways_cache->routes[i].the_geom)
        pfree(ways_cache->routes[i].the_geom);
    }
  }
  pfree(ways_cache);
  return;
}

/**
 * @brief Get the Ways cache entry from the global variable if one exists.
 * If it doesn't exist, make a new blank one and return it.
*/
WaysCache *
GetWaysCache()
{
  WaysCache *ways_cache = MEOS_WAYS_CACHE;
  if (! ways_cache)
  {
    /* Put Ways cache in a child of the CacheContext */
    MemoryContext context = AllocSetContextCreate(
        CacheMemoryContext,
        "Ways Context",
        ALLOCSET_SMALL_SIZES);

    /* Allocate in the upper context */
    ways_cache = MemoryContextAllocZero(context, sizeof(WaysCache));

    if (!ways_cache)
      elog(ERROR, "Unable to allocate space for Ways cache in context %p", (void *)context);

    ways_cache->count = 0;
    ways_cache->WaysCacheContext = context;

    /* Use this to clean up WaysCache in event of MemoryContext reset */
    MemoryContextCallback* callback = MemoryContextAlloc(context, sizeof(MemoryContextCallback));
    callback->func = DestroyWaysCache;
    callback->arg = (void *) ways_cache;
    MemoryContextRegisterResetCallback(context, callback);
  }
  MEOS_WAYS_CACHE = ways_cache;
  return ways_cache;
}

/**
 * @brief Get a Ways cache item structure from the Ways cache
 * @return On error return `NULL`
 */
static WaysCacheItem *
GetRouteFromWaysCache(WaysCache *ways_cache, bool any_gid, int64 gid)
{
  assert(ways_cache->count <= WAYS_CACHE_ITEMS);
  for (uint32_t i = 0; i < ways_cache->count; i++)
  {
    if (any_gid || ways_cache->routes[i].gid == gid)
    {
      ways_cache->routes[i].hits++;
      return &ways_cache->routes[i];
    }
  }
  return NULL;
}

/**
 * @brief Remove an entry to the Ways cache
 */
static void
DeleteRouteFromWaysCache(WaysCache *ways_cache, uint32_t position)
{
  if (ways_cache->routes[position].the_geom)
    pfree(ways_cache->routes[position].the_geom);
  memset(&ways_cache->routes[position], 0, sizeof(WaysCacheItem));
  return;
}

/**
 * @brief Add a Route to the Ways cache
 */
static WaysCacheItem *
AddRouteToWaysCache(WaysCache *ways_cache, ways_record *rec)
{
  assert(ways_cache); assert(rec);

  MemoryContext oldContext = 
    MemoryContextSwitchTo(WaysCache->WaysCacheContext);

  /* If the cache is already full then find the least used element and delete it */
  uint32_t cache_position = ways_cache->count;
  uint32_t hits = 1;
  if (cache_position == WAYS_CACHE_ITEMS)
  {
    cache_position = 0;
    hits = ways_cache->routes[0].hits;
    for (uint32_t i = 1; i < WAYS_CACHE_ITEMS; i++)
    {
      if (ways_cache->routes[i].hits < hits)
      {
        cache_position = i;
        hits = ways_cache->routes[i].hits;
      }
    }
    DeleteRouteFromWaysCache(ways_cache, cache_position);
    /* To avoid the element we are introduced now being evicted next (as
     * it would have 1 hit, being most likely the lower one) we reuse the
     * hits from the evicted position and add some extra buffer */
    hits += 5;
  }
  else
    ways_cache->count++;
  assert(ways_cache->count <= WAYS_CACHE_ITEMS);

  /* Store everything in new cache entry */
  assert(cache_position < WAYS_CACHE_ITEMS);
  ways_cache->routes[cache_position].gid = rec->gid;
  ways_cache->routes[cache_position].the_geom = rec->the_geom;
  ways_cache->routes[cache_position].length = rec->length;
  ways_cache->routes[cache_position].hits = hits;

  MemoryContextSwitchTo(oldContext);

  return &ways_cache->routes[cache_position];
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the ways table or CSV file to get the route geometry and 
 * length of a route identifier
 * @param[in] rid Route identifier
 * @param[out] rec Record to store on the cache
 */
bool
get_ways_record(int64 rid, ways_record *rec)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT the_geom, length FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  rec->gid = rid;
  rec->the_geom = NULL;
  bool found = false;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  if (ret > 0 && SPI_processed > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum line = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1,
      &isNull);
    if (isNull)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "Cannot get the record for route %ld", rid);
      return false;
    }
    else
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(line);
      if (! ensure_not_empty(gs))
      {
        pfree(gs);
        return false;
      }
      rec->the_geom = (GSERIALIZED *) SPI_palloc(gs->size);
      memcpy(rec->the_geom, gs, gs->size);
    }
    Datum length = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 2,
      &isNull);
    if (isNull)
      rec->length = geom_length(rec->the_geom);
    else
      rec->length = DatumGetFloat8(length);
    found = true;
  }
  SPI_finish();
  if (! found)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the record for route %ld", rid);
    return false;
  }
  return true;
}

/**
 * @brief Return true if a Ways structure was read from the Ways cache,
 * return 0 otherwise
 */
bool
route_lookup(int64 gid, bool any_gid, ways_record *rec)
{
  /* Get or initialize the cache for this round */
  WaysCache* ways_cache = GetWaysCache();
  if (! ways_cache)
    return LW_FAILURE;

  /* Add the route to the cache if it is not already there */
  WaysCacheItem *ways_item = GetRouteFromWaysCache(ways_cache, gid, any_gid);
  if (ways_item == NULL)
  {
    if (! get_ways_record(gid, rec))
      return false;
    ways_item = AddRouteToWaysCache(ways_cache, rec);
    return true;
  }
  /* The route was found in the cache */
  rec->gid = gid; rec->the_geom = ways_item->the_geom;
  rec->length = ways_item->length;
  return true;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Return true if the ways cache contains a route with the route
 * identifier
 * @param[in] rid Route identifier
 */
bool
route_exists(int64 rid)
{
  ways_record rec;
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
    return false;
  return true;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the ways cache to get the geometry of a route identifier
 * @param[in] rid Route identifier
 * @return On error return @p NULL
 */
GSERIALIZED *
route_geom(int64 rid)
{
  ways_record rec;
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
    return NULL;
  return rec.the_geom;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the edge table to return the route length from the
 * corresponding route identifier
 * @param[in] rid Route identifier
 * @return On error return -1.0
 */
double
route_length(int64 rid)
{
  ways_record rec;
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
    return -1.0;
  return rec.length;
}

int32_t
get_srid_ways()
{
  ways_record rec;
  if (route_lookup(0, true, &rec) == LW_FAILURE)
    return SRID_INVALID;
  return gserialized_get_srid(rec.the_geom);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

#define SQL_MAXLEN 1024

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a geometry into a network point
 * @param[in] gs Geometry
 * @csqlfn #Geompoint_to_npoint()
 */
Npoint *
geompoint_to_npoint(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs))
    return NULL;
  int32_t srid_geom = gserialized_get_srid(gs);
  int32_t srid_ways = get_srid_ways();
  if (srid_ways == SRID_INVALID || ! ensure_same_srid(srid_geom, srid_ways))
    return NULL;

  char *geomstr = geo_as_wkt(gs, OUT_DEFAULT_DECIMAL_DIGITS, true);
  char sql[SQL_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    DIST_EPSILON, geomstr);
  pfree(geomstr);
  Npoint *result = palloc(sizeof(Npoint));
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (! isNull)
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      Npoint *np = DatumGetNpointP(value);
      memcpy(result, np, sizeof(Npoint));
    }
  }
  SPI_finish();
  if (isNull)
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************/
