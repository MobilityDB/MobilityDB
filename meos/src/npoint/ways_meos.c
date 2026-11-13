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
#include <meos_internal_geo.h>
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
/* File the routes of the ways cache were read from */
static MEOS_TLS char *MEOS_WAYS_CACHE_PATH = NULL;

/**
 * @brief Structure to represent the position of a route in the ways CSV file
 */
typedef struct
{
  int64 gid;              /**< Identifier of the route */
  long offset;            /**< Position of its line in the file */
} WaysOffset;

/**
 * @brief Structure to represent the index of the ways CSV file: the position
 * of the line of every route, sorted by identifier and then by position, so
 * that a route is read with one seek instead of a scan of the file
 */
typedef struct
{
  char *path;             /**< File the index was built from */
  WaysOffset *offsets;    /**< Positions of the routes */
  int count;              /**< Number of routes */
} WaysIndex;

/* Global variable to hold the index of the ways CSV file */
static MEOS_TLS WaysIndex *MEOS_WAYS_INDEX = NULL;

/*****************************************************************************
 * Cache management functions
 *****************************************************************************/

/**
 * @brief Get the ways cache from the global variable
 * @details If it does not exist, or holds the routes of a file other than the
 * one #meos_set_ways_csv names last, a new blank one is made and returned, as
 * #GetWaysIndex rebuilds the index: the routes of a file and the SRID of its
 * first route answer for that file alone. The path of the file is held per
 * thread with the cache, so each thread's cache follows the file it reads.
 */
static WaysCache *
GetWaysCache(void)
{
  if (MEOS_WAYS_CACHE && MEOS_WAYS_CACHE_PATH &&
      strcmp(MEOS_WAYS_CACHE_PATH, WAYS_CSV) == 0)
    return MEOS_WAYS_CACHE;
  char *path = malloc(strlen(WAYS_CSV) + 1);
  if (! path)
  {
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the path of the ways cache");
    return NULL;
  }
  strcpy(path, WAYS_CSV);
  if (MEOS_WAYS_CACHE)
    ways_cache_free(MEOS_WAYS_CACHE);
  free(MEOS_WAYS_CACHE_PATH);
  MEOS_WAYS_CACHE = ways_cache_make();
  MEOS_WAYS_CACHE_PATH = path;
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
  if (MEOS_WAYS_INDEX)
  {
    free(MEOS_WAYS_INDEX->path);
    free(MEOS_WAYS_INDEX->offsets);
    free(MEOS_WAYS_INDEX);
    MEOS_WAYS_INDEX = NULL;
  }
  free(MEOS_WAYS_CACHE_PATH);
  MEOS_WAYS_CACHE_PATH = NULL;
  if (! MEOS_WAYS_CACHE)
    return;
  ways_cache_free(MEOS_WAYS_CACHE);
  MEOS_WAYS_CACHE = NULL;
}

/**
 * @brief Comparison of the positions of two routes, by identifier and then
 * by position in the file
 */
static int
ways_offset_cmp(const void *a, const void *b)
{
  const WaysOffset *wa = a, *wb = b;
  if (wa->gid != wb->gid)
    return wa->gid < wb->gid ? -1 : 1;
  return wa->offset < wb->offset ? -1 : (wa->offset > wb->offset ? 1 : 0);
}

/**
 * @brief Get the index of the ways CSV file, built with one scan of the file
 * the first time a route is read from it and again when
 * #meos_set_ways_csv names another file, as #GetWaysCache gets the cache
 * @details The identifier of a line is read as #get_ways_record reads it,
 * into a long long, which %lld fills on every data model
 */
static WaysIndex *
GetWaysIndex(FILE *file)
{
  if (MEOS_WAYS_INDEX && strcmp(MEOS_WAYS_INDEX->path, WAYS_CSV) == 0)
    return MEOS_WAYS_INDEX;
  if (MEOS_WAYS_INDEX)
  {
    free(MEOS_WAYS_INDEX->path);
    free(MEOS_WAYS_INDEX->offsets);
    free(MEOS_WAYS_INDEX);
    MEOS_WAYS_INDEX = NULL;
  }
  WaysIndex *index = calloc(1, sizeof(WaysIndex));
  if (! index)
  {
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the index of the ways file");
    return NULL;
  }
  int capacity = 1024;
  index->offsets = malloc(sizeof(WaysOffset) * capacity);
  index->path = malloc(strlen(WAYS_CSV) + 1);
  if (! index->offsets || ! index->path)
  {
    free(index->offsets); free(index->path); free(index);
    meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
      "Cannot allocate the index of the ways file");
    return NULL;
  }
  strcpy(index->path, WAYS_CSV);
  rewind(file);
  /* The line of a route starts with its identifier, the geometry being
   * skipped up to the end of the line */
  while (true)
  {
    long offset = ftell(file);
    long long gidbuf;
    if (fscanf(file, "%lld,", &gidbuf) != 1)
      break;
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n')
      ;
    if (index->count == capacity)
    {
      capacity *= 2;
      WaysOffset *offsets = realloc(index->offsets,
        sizeof(WaysOffset) * capacity);
      if (! offsets)
      {
        free(index->offsets); free(index->path); free(index);
        meos_error(ERROR, MEOS_ERR_MEMORY_ALLOC_ERROR,
          "Cannot allocate the index of the ways file");
        return NULL;
      }
      index->offsets = offsets;
    }
    index->offsets[index->count].gid = gidbuf;
    index->offsets[index->count].offset = offset;
    index->count++;
    if (c == EOF)
      break;
  }
  qsort(index->offsets, index->count, sizeof(WaysOffset), ways_offset_cmp);
  MEOS_WAYS_INDEX = index;
  return index;
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
  /* The full file path in the first argument is defined in a global variable.
   * The file is read in binary mode because the index of #GetWaysIndex keeps
   * the positions ftell answers and fseek returns to them, and on Windows a
   * stream in text mode answers positions that are not the byte offsets of a
   * file whose lines end in a newline alone, so the seek lands inside another
   * line. A carriage return a line does end in is white space to the fscanf
   * that reads the line. */
  FILE *file = fopen(WAYS_CSV, "rb");
  if (! file)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot open the ways CSV file (reading from %s); set its location with "
      "meos_set_ways_csv()", WAYS_CSV);
    return false;
  }

  bool result = false;
  /* A route of the file is read at its position, the first line of the
   * route with a geometry that is not empty answering, as the scan below
   * answers it */
  if (! any_gid)
  {
    const WaysIndex *index = GetWaysIndex(file);
    if (! index)
    {
      fclose(file);
      return false;
    }
    int lo = 0, hi = index->count;
    while (lo < hi)
    {
      int mid = lo + (hi - lo) / 2;
      if (index->offsets[mid].gid < rid)
        lo = mid + 1;
      else
        hi = mid;
    }
    for (int i = lo; i < index->count && index->offsets[i].gid == rid; i++)
    {
      char geo_buffer[MAX_LEN_GEOM];
      long long gidbuf;
      if (fseek(file, index->offsets[i].offset, SEEK_SET) != 0 ||
          fscanf(file, "%lld,%100000s\n", &gidbuf, geo_buffer) != 2)
        continue;
      rec->gid = rid;
      rec->the_geom = geom_in(geo_buffer, -1);
      /* A line whose geometry #geom_in refuses, raising its error, ends the
       * reading */
      if (! rec->the_geom)
        break;
      if (! geo_is_empty(rec->the_geom))
      {
        rec->length = geom_length(rec->the_geom);
        result = true;
        break;
      }
      pfree(rec->the_geom);
    }
    fclose(file);
    return result;
  }

  /* The record of any route is the first line of the file with a geometry
   * that is not empty */
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
      break;
    }

    /* Ignore the records with NULL values or empty geometries */
    if (read == 2)
    {
      /* Transform the geometry string into a geometry value; a line whose
       * geometry #geom_in refuses, raising its error, ends the reading */
      rec->the_geom = geom_in(geo_buffer, -1);
      if (! rec->the_geom)
        break;
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
  if (! ways_cache)
    return false;
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
 * @brief Return the geometry of a route read as a line, as #route_geom
 * returns its geometry
 * @details The line belongs to the ways cache, which reads it once from the
 * geometry of the route, and is not freed by the caller
 * @param[in] rid Route identifier
 * @errval NULL
 */
const LWLINE *
route_line(int64 rid)
{
  return route_line_bound(rid, NULL);
}

/**
 * @brief Return the geometry of a route read as a line, as #route_line
 * returns it, and in the last argument, when it is not NULL, the bound of
 * the construction of a point on the route (#route_construction_bound),
 * read by the ways cache with the line
 * @param[in] rid Route identifier
 * @param[out] bound Bound of the construction
 * @errval NULL
 */
const LWLINE *
route_line_bound(int64 rid, double *bound)
{
  ways_record rec;
  if (route_lookup(rid, false, &rec) == LW_FAILURE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route %lld", (long long) rid);
    return NULL;
  }
  if (bound)
    *bound = rec.bound;
  return rec.the_line;
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
 * @brief Transform a geometry into a network point located on the route
 * nearest to it on which it lies, within a tolerance or as a point the route
 * constructs
 * @param[in] gs Geometry
 * @param[in] tolerance Maximum distance between the point and the route,
 * read where @p constructed is false
 * @param[in] constructed True for the casts (#route_constructs_point)
 * @pre The geometry is a non-empty point in the SRID of the ways network
 */
Npoint *
geompoint_route_npoint(const GSERIALIZED *gs, double tolerance,
  bool constructed)
{
  assert(gs); assert(gserialized_get_type(gs) == POINTTYPE);
  assert(! gserialized_is_empty(gs));
  int32_t srid_ways = get_srid_ways();
  assert(gserialized_get_srid(gs) == srid_ways);

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
  /* Route, distance and position of the nearest route */
  int64 min_gid = 0;
  double min_dist = DBL_MAX, min_pos = 0.0;
  /* Read the file */
  do
  {
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
      /* A line whose geometry #geom_in refuses has raised its error */
      if (! rec.the_geom)
      {
        fclose(file);
        return NULL;
      }
      if (! ensure_valid_route(rec.the_geom, srid_ways))
      {
        pfree(rec.the_geom);
        fclose(file);
        return NULL;
      }
      route_nearest_update(gs, rec.gid, rec.the_geom, tolerance, constructed,
        &min_gid, &min_dist, &min_pos);
      pfree(rec.the_geom);
    }
  } while (! feof(file));

  /* Close the input file */
  fclose(file);

  /* If the point was not found */
  if (min_dist == DBL_MAX)
    return NULL;

  return npoint_make(min_gid, min_pos);
}

/**
 * @ingroup meos_npoint_base_conversion
 * @brief Transform a geometry into a network point located on the route
 * nearest to it within a tolerance
 * @details The route is the nearest one among those whose distance to the
 * point is at most the tolerance, the one with the least identifier among
 * equally near routes, and the position is the one of the point projected on
 * that route. The result is `NULL` when no route is within the tolerance.
 * @param[in] gs Geometry
 * @param[in] tolerance Maximum distance between the point and the route, in
 * the units of the spatial reference system of the ways network
 * @csqlfn #Geompoint_to_npoint()
 */
Npoint *
geompoint_to_npoint(const GSERIALIZED *gs, double tolerance)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs) ||
      ! ensure_finite(tolerance) ||
      ! ensure_not_negative_datum(Float8GetDatum(tolerance), T_FLOAT8) ||
      ! ensure_srid_ways(gserialized_get_srid(gs)))
    return NULL;
  return geompoint_route_npoint(gs, tolerance, false);
}

/**
 * @brief Transform a geometry into a network point located on the route
 * that constructs it, the cast of a geometry to a network point
 * @details The route is the nearest one among those that construct the
 * point (#route_constructs_point), the one with the least identifier among
 * equally near routes. The result is `NULL` when no route constructs it.
 * @param[in] gs Geometry
 */
Npoint *
geompoint_to_npoint_cast(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_empty(gs) || ! ensure_point_type(gs) ||
      ! ensure_srid_ways(gserialized_get_srid(gs)))
    return NULL;
  return geompoint_route_npoint(gs, 0.0, true);
}

/*****************************************************************************/
