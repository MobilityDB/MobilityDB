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
 * @brief Implementation of a cache storing the route records read from a ways
 * CSV file
 */

/* C */
#include <assert.h>
#include <float.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "geo/tgeo_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/ways_cache.h"

/*****************************************************************************
 * Definitions for reading the ways CSV file
 * Notice that the CSV file does not have header and the separator are commas
 *****************************************************************************/

/* Maximum length in characters of a geometry string in the input data */
#define MAX_LEN_GEOM 100001

/* Default location of the ways CSV file, the one the build installs.
 * Overridable at run time with `meos_set_ways_csv()`, mirroring
 * `meos_set_spatial_ref_sys_csv()`, so that a MEOS embedder points at a
 * deployment-specific network file. */
char *WAYS_CSV = MEOS_DATAROOTDIR "/ways1000.csv";
/* Copy of the path given to the last call of `meos_set_ways_csv()` */
static char *WAYS_CSV_COPY = NULL;

/**
 * @ingroup meos_setup
 * @brief Set the location of the ways CSV file used to resolve npoint route
 * geometries
 * @details The function keeps a copy of the path and frees the copy it kept
 * for an earlier call, as #meos_set_spatial_ref_sys_csv does.
 * @param[in] path Full path to the ways CSV file
 */
void
meos_set_ways_csv(const char *path)
{
  char *copy = malloc(strlen(path) + 1);
  if (! copy)
  {
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the path of the ways file");
    return;
  }
  strcpy(copy, path);
  free(WAYS_CSV_COPY);
  WAYS_CSV_COPY = copy;
  WAYS_CSV = copy;
}

/* Global variable to hold the ways cache */
static MEOS_TLS WaysCache *MEOS_WAYS_CACHE = NULL;

/*****************************************************************************
 * Cache management functions
 *****************************************************************************/

/**
 * @brief Get the ways cache from the global variable
 * @details If it does not exist, a new blank one is made and returned.
 */
static WaysCache *
GetWaysCache(void)
{
  if (! MEOS_WAYS_CACHE)
    MEOS_WAYS_CACHE = ways_cache_make();
  return MEOS_WAYS_CACHE;
}

/**
 * @ingroup meos_setup
 * @brief Destroy the ways cache
 */
void
meos_finalize_ways(void)
{
  /* Idempotency: only destroy a live cache, and null the slot so a
   * second finalize call does not double-free the cache and its
   * stored geometries. */
  if (! MEOS_WAYS_CACHE)
    return;
  ways_cache_free(MEOS_WAYS_CACHE);
  MEOS_WAYS_CACHE = NULL;
}

/**
 * @brief Access the ways CSV file to get the geometry of a route and compute
 * its length
 * @details The reading of the network the MEOS build gives to
 * #ways_cache_lookup, as the PostgreSQL build reads the ways table.
 * @param[in] rid Route identifier
 * @param[in] any_gid True when the record of any route is asked for
 * @param[out] rec Record to store on the cache
 */
static bool
get_ways_record(int64 rid, bool any_gid, ways_record *rec)
{
  /* The full file path in the first argument is defined in a global variable*/
  FILE *file = fopen(WAYS_CSV, "r");
  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways CSV file (reading from %s); set its location with "
      "meos_set_ways_csv()", WAYS_CSV);
    return false;
  }

  bool result = false;
  /* Continue reading the file */
  do
  {
    char geo_buffer[MAX_LEN_GEOM];
    long long gidbuf; /* MEOS: %ld mismatches int64 on LLP64 (Windows) */
    int read = fscanf(file, "%lld,%100000s\n", &gidbuf, geo_buffer);
    rec->gid = (int64) gidbuf; /* MEOS */
    if (ferror(file))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Error reading the ways CSV file");
      return false;
    }

    /* Ignore the records with NULL values or empty geometries */
    if (read == 2 && (any_gid || rid == rec->gid))
    {
      /* Transform the geometry string into a geometry value */
      rec->the_geom = geom_in(geo_buffer, -1);
      if (! geo_is_empty(rec->the_geom))
      {
        rec->length = geom_length(rec->the_geom);
        result = true;
        break;
      }
      pfree(rec->the_geom);
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  return result;
}

/**
 * @brief Return in the last argument the record of a route read from the
 * ways cache, and return false when there is no such route
 * @details The twin of the function of the PostgreSQL build in ways.c, both
 * calling #ways_cache_lookup with their own reading of the network.
 */
static bool
route_lookup(int64 gid, bool any_gid, ways_record *rec)
{
  /* Get or initialize the cache for this round */
  WaysCache *ways_cache = GetWaysCache();
  return ways_cache_lookup(ways_cache, gid, any_gid, get_ways_record, rec);
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
 * @errval NULL
 */
const GSERIALIZED *
route_geom(int64 rid)
{
  ways_record rec;
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route %lld", (long long) rid);
    return NULL;
  }
  return rec.the_geom;
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
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
    return DBL_MAX;
  return rec.length;
}

int32_t
get_srid_ways(void)
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
      "Cannot open the ways CSV file (reading from %s); set its location with "
      "meos_set_ways_csv()", WAYS_CSV);
    return NULL;
  }

  /* Record holding one line of the file */
  ways_record rec;
  /* Minimum distance */
  double min_dist = DBL_MAX;
  /* Route and position of the shortest distance so far */
  int64 min_gid = 0;
  double min_pos = 0;
  /* Position of the point in the route of the line being read */
  double pos = 0;
  /* Read the file */
  do
  {
    /* We need to reproduce the following SQL query for a given geometry geo
     *   SELECT npoint(gid, ST_LineLocatePoint(the_geom, geo))
     *   FROM public.ways WHERE ST_DWithin(the_geom, geo, MEOS_EPSILON)
     *   ORDER BY ST_Distance(the_geom, geo) LIMIT 1;
     */
    /* Buffer for reading the geometry string */
    char geo_buffer[MAX_LEN_GEOM];
    long long gidbuf; /* MEOS: %ld mismatches int64 on LLP64 (Windows) */
    int read = fscanf(file, "%lld,%100000s\n", &gidbuf, geo_buffer);
    rec.gid = (int64) gidbuf; /* MEOS */
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
      /* A distance raising an error, -1.0, makes no candidate, as a position
       * raising one does above */
      if (dist >= 0.0 && dist < min_dist)
      {
        min_dist = dist;
        min_gid = rec.gid;
        min_pos = pos;
      }
      free(rec.the_geom);
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  /* If the point was not found */
  if (min_dist == DBL_MAX)
    return NULL;

  return npoint_make(min_gid, min_pos);
}

/*****************************************************************************/
