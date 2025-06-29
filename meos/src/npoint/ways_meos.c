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
 * @brief Implementation of a cache storing the route records read from a ways
 * CSV file
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "geo/tgeo_spatialfuncs.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Definitions for reading the ways CSV file
 * Notice that the CSV file does not have header and the separator are commas
 *****************************************************************************/

/* Maximum length in characters of a geometry string in the input data */
#define MAX_LENGTH_GEOM 100001

/* Location of the ways CSV file */
// #define WAYS_CSV "/usr/local/share/ways.csv"
#define WAYS_CSV "/usr/local/share/ways1000.csv"

/**
 * @brief Structure to represent a record in the ways CSV file
 */
typedef struct
{
  int64 gid;              /**< Identifier of the route */
  GSERIALIZED *the_geom;  /**< Geometry of the route */
  double length;          /**< Length of the route */
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
 * ways CSV file
 * @note In normal usage we do not expect it to have many entries, so we
 * linearly scan the list
 */
typedef struct struct_WaysCache
{
  WaysCacheEntry routes[WAYS_CACHE_SIZE];
  uint32_t count;
} WaysCache;

/* Global variable to hold the ways cache */
WaysCache *MEOS_WAYS_CACHE = NULL;

/*****************************************************************************
 * Cache management functions
 *****************************************************************************/

/**
 * @brief Free all the malloc'ed geometries stored in the ways cache and free
 * the cache
 */
static void
DestroyWaysCache(WaysCache *ways_cache)
{
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
 * @brief Get the ways cache variable from the global variable if one exists.
 * If it doesn't exist, make a new blank one and return it.
*/
static WaysCache *
GetWaysCache()
{
  WaysCache *ways_cache = MEOS_WAYS_CACHE;
  if (! ways_cache)
  {
    /* Allocate memory */
    ways_cache = palloc0(sizeof(WaysCache));
    if (! ways_cache)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unable to allocate space for the ways cache");
      return NULL;
    }
  }
  MEOS_WAYS_CACHE = ways_cache;
  return ways_cache;
}

/**
 * @brief Destroy the ways cache
 */
void
meos_finalize_ways(void)
{
  DestroyWaysCache(MEOS_WAYS_CACHE);
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
  uint32_t hits = 1;
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
 * @brief Access the ways CSV file to get the geometry of a route and compute
 * its length 
 * @param[in] rid Route identifier
 * @param[out] rec Record to store on the cache
 */
static bool
get_ways_record(int64 rid, ways_record *rec)
{
  /* The full file path in the first argument is defined in a global variable*/
  FILE *file = fopen(WAYS_CSV, "r");
  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways CSV file");
    return false;
  }

  bool result = false;
  /* Continue reading the file */
  do
  {
    char geo_buffer[MAX_LENGTH_GEOM];
    int read = fscanf(file, "%ld,%100000s\n", &rec->gid, geo_buffer);
    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways CSV file");
      return false;
    }

    /* Ignore the records with NULL values or empty geometries */
    if (read == 2)
    {
      /* Transform the geometry string into a geometry value */
      rec->the_geom = geom_in(geo_buffer, -1);
      if (! geo_is_empty(rec->the_geom))
      {
        if (rec->gid == rid)
        {
          rec->length = geom_length(rec->the_geom);
          result = true;
          break;
        }
      }
      free(rec->the_geom);
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  return result;
}

/**
 * @brief Return true if a Ways entry was read from the ways cache,
 * return false otherwise
 */
static bool
route_lookup(int64 gid, bool any_gid, ways_record *rec)
{
  /* Get or initialize the cache for this round */
  WaysCache* ways_cache = GetWaysCache();
  if (! ways_cache)
    return true;

  /* Add the route to the cache if it is not already there */
  WaysCacheEntry *ways_entry = GetRouteFromWaysCache(ways_cache, gid, any_gid);
  if (ways_entry == NULL)
  {
    if (! get_ways_record(gid, rec))
      return false;
    ways_entry = AddRouteToWaysCache(ways_cache, rec);
    return true;
  }
  /* The route was found in the cache */
  rec->gid = gid; rec->the_geom = ways_entry->the_geom;
  rec->length = ways_entry->length;
  return true;
}

/*****************************************************************************
 * General functions
 *****************************************************************************/

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

  /* The full file path in the first argument is defined in a global variable*/
  FILE *file = fopen(WAYS_CSV, "r");
  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways CSV file");
    return NULL;
  }

  /* Record holding one line of the file */
  ways_record rec;
  /* Minimum distance */
  double min_dist = DBL_MAX;
  /* Position in the geometry with the shortest distance */
  double pos = 0;
  /* Continue reading the file */
  do
  {
    /* We need to reproduce the following SQL query for a given geometry geo
     *   SELECT npoint(gid, ST_LineLocatePoint(the_geom, geo))
     *   FROM public.ways WHERE ST_DWithin(the_geom, geo, DIST_EPSILON)
     *   ORDER BY ST_Distance(the_geom, geo) LIMIT 1;
     */
    /* Buffer for reading the geometry string */
    char geo_buffer[MAX_LENGTH_GEOM];
    int read = fscanf(file, "%ld,%100000s\n", &rec.gid, geo_buffer);
    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways CSV file");
      return NULL;
    }

    /* Ignore the records with NULL values */
    if (read == 2)
    {
      /* Transform the geometry string into a geometry value */
      rec.the_geom = geom_in(geo_buffer, -1);
      /* Continue if the geometry is empty */
      if (geo_is_empty(rec.the_geom))
      {
        free(rec.the_geom);
        continue;
      }
      /* Continue if the point is not in the line */
      pos = line_locate_point(rec.the_geom, gs);
      if (pos < 0)
      {
        free(rec.the_geom);
        continue;
      }
      /* Compute minimal distance */
      double dist = geom_distance2d(rec.the_geom, gs);
      if (dist < min_dist)
        min_dist = dist;
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  /* If the point was not found */
  if (min_dist == DBL_MAX)
    return NULL;

  Npoint *result = npoint_make(rec.gid, pos);
  free(rec.the_geom);
  return result;
}

/*****************************************************************************/
