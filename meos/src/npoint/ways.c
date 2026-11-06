/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
#include "temporal/type_inout.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

#define SQL_ROUTE_MAXLEN  128

/* Global variable saving the SRID of the ways table */
static int32_t SRID_WAYS = SRID_INVALID;

/*****************************************************************************
 * Ways cache
 *
 * The routes read from the ways table are kept in a cache for the life of the
 * backend, as the MEOS build keeps the routes read from the ways CSV file in
 * ways_meos.c. Without it every network point answers its geometry, its
 * length or its existence with a statement of its own, which is parsed,
 * planned and executed anew, where the network it reads is the same for the
 * whole session.
 *
 * The cache lies in a child of the CacheMemoryContext, as the PROJ cache of
 * PostGIS does in postgis/libpgcommon/lwgeom_transform.c, and holds the
 * geometries of the routes in that same context, so that a reset of the
 * context releases them with it.
 *****************************************************************************/

/**
 * @brief Structure to represent a record of the ways table
 */
typedef struct
{
  int64 gid;              /**< Identifier of the route */
  GSERIALIZED *the_geom;  /**< Geometry of the route */
  double length;          /**< Length of the route, DBL_MAX when it states
                               none */
} ways_record;

/**
 * @brief Structure to represent an entry in the ways cache
 */
typedef struct struct_WaysCacheEntry
{
  int64 gid;              /**< Identifier of the route */
  GSERIALIZED *the_geom;  /**< Geometry of the route */
  double length;          /**< Length of the route */
  uint64_t hits;          /**< Number of hits of the route */
} WaysCacheEntry;

/* Number of items stored in the ways cache */
#define WAYS_CACHE_SIZE 128

/**
 * @brief The ways cache holds a fixed number of ways records read from the
 * ways table
 * @note In normal usage we do not expect it to have many entries, so we
 * linearly scan the list
 */
typedef struct struct_WaysCache
{
  WaysCacheEntry routes[WAYS_CACHE_SIZE];
  uint32_t count;
  MemoryContext context;  /**< Context holding the cache and its geometries */
} WaysCache;

/* Global variable to hold the ways cache */
static WaysCache *WAYS_CACHE = NULL;

/*****************************************************************************
 * Cache management functions
 *****************************************************************************/

/**
 * @brief Forget the ways cache when the context holding it is reset
 * @details The geometries of the routes lie in that same context, so the
 * reset releases them and only the global variable naming the cache is left
 * to drop. The next route read then builds the cache anew.
 */
static void
ForgetWaysCache(void *cache UNUSED)
{
  WAYS_CACHE = NULL;
  return;
}

/**
 * @brief Get the ways cache from the global variable
 * @details If it does not exist, a new blank one is made and returned.
 */
static WaysCache *
GetWaysCache(void)
{
  WaysCache *ways_cache = WAYS_CACHE;
  if (! ways_cache)
  {
    /* Put the ways cache in a child of the CacheMemoryContext */
    MemoryContext context = AllocSetContextCreate(CacheMemoryContext,
      "Ways Context", ALLOCSET_SMALL_SIZES);
    /* Allocate in the context, which holds the cache and its geometries */
    ways_cache = (WaysCache *) MemoryContextAllocZero(context,
      sizeof(WaysCache));
    ways_cache->context = context;
    /* Use this to forget the cache in the event of a MemoryContext reset */
    MemoryContextCallback *callback = (MemoryContextCallback *)
      MemoryContextAlloc(context, sizeof(MemoryContextCallback));
    callback->func = ForgetWaysCache;
    callback->arg = NULL;
    MemoryContextRegisterResetCallback(context, callback);
    WAYS_CACHE = ways_cache;
  }
  return ways_cache;
}

/**
 * @brief Get a route from the ways cache, if not found return `NULL`
 */
static WaysCacheEntry *
GetRouteFromWaysCache(WaysCache *ways_cache, int64 gid, bool any_gid)
{
  assert(ways_cache->count <= WAYS_CACHE_SIZE);
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
 * @brief Remove from the ways cache the route in a given position
 */
static void
DeleteRouteFromWaysCache(WaysCache *ways_cache, uint32_t position)
{
  if (ways_cache->routes[position].the_geom)
    pfree(ways_cache->routes[position].the_geom);
  memset(&ways_cache->routes[position], 0, sizeof(WaysCacheEntry));
  return;
}

/**
 * @brief Add a route to the ways cache
 */
static WaysCacheEntry *
AddRouteToWaysCache(WaysCache *ways_cache, ways_record *rec)
{
  assert(ways_cache); assert(rec);

  /* If the cache is full, find the least used element and delete it */
  uint32_t cache_position = ways_cache->count;
  uint64_t hits = 1;
  if (cache_position == WAYS_CACHE_SIZE)
  {
    cache_position = 0;
    hits = ways_cache->routes[0].hits;
    for (uint32_t i = 1; i < WAYS_CACHE_SIZE; i++)
    {
      if (ways_cache->routes[i].hits < hits)
      {
        cache_position = i;
        hits = ways_cache->routes[i].hits;
      }
    }
    DeleteRouteFromWaysCache(ways_cache, cache_position);
    /* To avoid the element we are introduced now being evicted next (as it
     * would have 1 hit, being most likely the lower one) we reuse the hits
     * from the evicted position and add some extra buffer */
    hits += 5;
  }
  else
    ways_cache->count++;
  assert(ways_cache->count <= WAYS_CACHE_SIZE);

  /* Store everything in new cache entry */
  assert(cache_position < WAYS_CACHE_SIZE);
  ways_cache->routes[cache_position].gid = rec->gid;
  ways_cache->routes[cache_position].the_geom = rec->the_geom;
  ways_cache->routes[cache_position].length = rec->length;
  ways_cache->routes[cache_position].hits = hits;

  return &ways_cache->routes[cache_position];
}

/**
 * @brief Access the ways table to get the record of a route
 * @param[in] rid Route identifier
 * @param[in] any_gid True when the record of any route is asked for
 * @param[in] context Context the geometry of the route is copied into
 * @param[out] rec Record to store on the cache
 */
static bool
get_ways_record(int64 rid, bool any_gid, MemoryContext context,
  ways_record *rec)
{
  char sql[SQL_ROUTE_MAXLEN];
  if (any_gid)
    snprintf(sql, sizeof(sql),
      "SELECT gid, the_geom, length FROM public.ways LIMIT 1");
  else
    snprintf(sql, sizeof(sql),
      "SELECT gid, the_geom, length FROM public.ways WHERE gid = "
      INT64_FORMAT, rid);

  bool result = false;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    bool gid_isnull, geom_isnull, length_isnull;
    Datum gid = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1,
      &gid_isnull);
    Datum geom = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 2,
      &geom_isnull);
    Datum length = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 3,
      &length_isnull);
    if (! gid_isnull && ! geom_isnull)
    {
      rec->gid = DatumGetInt64(gid);
      /* The geometry is copied into the context of the cache, which keeps it
       * after SPI_finish() and for the life of the backend */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
      MemoryContext oldcontext = MemoryContextSwitchTo(context);
      rec->the_geom = geo_copy(gs);
      MemoryContextSwitchTo(oldcontext);
      /* A route stating no length answers DBL_MAX, as route_length does */
      rec->length = length_isnull ? DBL_MAX : DatumGetFloat8(length);
      result = true;
    }
  }
  SPI_finish();
  return result;
}

/**
 * @brief Return true if the record of a route was read from the ways cache,
 * return false otherwise
 */
static bool
route_lookup(int64 gid, bool any_gid, ways_record *rec)
{
  /* Get or initialize the cache for this round */
  WaysCache *ways_cache = GetWaysCache();

  /* Add the route to the cache if it is not already there */
  WaysCacheEntry *ways_entry = GetRouteFromWaysCache(ways_cache, gid, any_gid);
  if (ways_entry == NULL)
  {
    if (! get_ways_record(gid, any_gid, ways_cache->context, rec))
      return false;
    AddRouteToWaysCache(ways_cache, rec);
    return true;
  }
  /* The route was found in the cache */
  rec->gid = ways_entry->gid; rec->the_geom = ways_entry->the_geom;
  rec->length = ways_entry->length;
  return true;
}

/*****************************************************************************
 * Route functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_srid
 * @brief Return the SRID of the routes in the ways table
 * @errval SRID_INVALID
 */
int32_t
get_srid_ways(void)
{
  /* Get the value from the global variable if it has been already set */
  if (SRID_WAYS != SRID_INVALID)
    return SRID_WAYS;

  /* Fetch the SRID value from the geometry of any route */
  ways_record rec;
  if (! route_lookup(0, true, &rec))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot determine SRID of the ways table");
    return SRID_INVALID;
  }
  /* Save the SRID value in a global variable */
  SRID_WAYS = gserialized_get_srid(rec.the_geom);
  return SRID_WAYS;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Return true if the edge table contains a route with the route
 * identifier
 * @param[in] rid Route identifier
 */
bool
route_exists(int64 rid)
{
  ways_record rec;
  return route_lookup(rid, false, &rec);
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the edge table to return the route length from the
 * corresponding route identifier
 * @param[in] rid Route identifier
 * @errval DBL_MAX
 */
double
route_length(int64 rid)
{
  ways_record rec;
  if (! route_lookup(rid, false, &rec) || rec.length == DBL_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the length for route " INT64_FORMAT, rid);
    return DBL_MAX;
  }
  return rec.length;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the edge table to get the route geometry from corresponding
 * route identifier
 * @param[in] rid Route identifier
 * @errval NULL
 */
const GSERIALIZED *
route_geom(int64 rid)
{
  ways_record rec;
  if (! route_lookup(rid, false, &rec))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route " INT64_FORMAT, rid);
    return NULL;
  }
  /* The geometry belongs to the cache and is not freed here */
  if (! ensure_not_empty(rec.the_geom))
    return NULL;
  return rec.the_geom;
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

  char *geomstr = geo_as_wkt(gs, OUT_DEFAULT_DECIMAL_DIGITS, true, NULL);
  char sql[SQL_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT npoint(gid, ST_LineLocatePoint(the_geom, '%s')) "
    "FROM public.ways WHERE ST_DWithin(the_geom, '%s', %lf) "
    "ORDER BY ST_Distance(the_geom, '%s') LIMIT 1", geomstr, geomstr,
    MEOS_EPSILON, geomstr);
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
