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
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "npoint/ways_cache.h"

#define SQL_ROUTE_MAXLEN  128

/* Global variable saving the SRID of the ways table */
static int32_t SRID_WAYS = SRID_INVALID;
/* Global variable saving the largest absolute coordinate of the routes of the
 * ways table, negative before it is read (#ways_magnitude) */
static double WAYS_MAGNITUDE = -1.0;

/*****************************************************************************
 * Ways cache
 *
 * The routes read from the ways table are kept in the ways cache of
 * ways_cache.c for the life of the backend, as the MEOS build keeps the
 * routes read from the ways CSV file in ways_meos.c. Without it every network
 * point answers its geometry, its length or its existence with a statement
 * of its own, which is parsed, planned and executed anew, where the network
 * it reads is the same for the whole session.
 *
 * The cache lies in a child of the CacheMemoryContext, as the PROJ cache of
 * PostGIS does in postgis/libpgcommon/lwgeom_transform.c, and holds the
 * geometries of the routes in that same context, so that a reset of the
 * context releases them with it.
 *****************************************************************************/

/* Global variable to hold the ways cache */
static WaysCache *WAYS_CACHE = NULL;
/* Global variable to hold the context of the ways cache */
static MemoryContext WAYS_CONTEXT = NULL;

/*****************************************************************************
 * Cache management functions
 *****************************************************************************/

/**
 * @brief Forget the ways cache when the context holding it is reset
 * @details The callback of the reset, as #PROJSRSDestroyPortalCache is for
 * the PROJ cache of PostGIS. The cache and the geometries of the routes lie
 * in that context, so the reset releases them and only the global variables
 * naming them are left to drop. The next route read then builds the cache
 * anew.
 */
static void
ForgetWaysCache(void *cache UNUSED)
{
  WAYS_CACHE = NULL;
  WAYS_CONTEXT = NULL;
  return;
}

/**
 * @brief Get the ways cache from the global variable
 * @details If it does not exist, a new blank one is made and returned.
 */
static WaysCache *
GetWaysCache(void)
{
  if (! WAYS_CACHE)
  {
    /* Put the ways cache in a child of the CacheMemoryContext */
    MemoryContext context = AllocSetContextCreate(CacheMemoryContext,
      "Ways Context", ALLOCSET_SMALL_SIZES);
    /* Use this to forget the cache in the event of a MemoryContext reset */
    MemoryContextCallback *callback = (MemoryContextCallback *)
      MemoryContextAlloc(context, sizeof(MemoryContextCallback));
    callback->func = ForgetWaysCache;
    callback->arg = NULL;
    MemoryContextRegisterResetCallback(context, callback);
    /* Allocate in the context, which holds the cache and its geometries */
    MemoryContext oldcontext = MemoryContextSwitchTo(context);
    WAYS_CACHE = ways_cache_make();
    MemoryContextSwitchTo(oldcontext);
    WAYS_CONTEXT = context;
  }
  return WAYS_CACHE;
}

/**
 * @brief Access the ways table to get the record of a route
 * @param[in] rid Route identifier
 * @param[in] any_gid True when the record of any route is asked for
 * @param[out] rec Record to store on the cache
 */
static bool
get_ways_record(int64 rid, bool any_gid, ways_record *rec)
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
      MemoryContext oldcontext = MemoryContextSwitchTo(WAYS_CONTEXT);
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
 * @brief Return in the last argument the record of a route read from the
 * ways cache, and return false when there is no such route
 * @details The twin of the function of the MEOS build in ways_meos.c, both
 * calling #ways_cache_lookup with their own reading of the network.
 */
static bool
route_lookup(int64 gid, bool any_gid, ways_record *rec)
{
  /* Get or initialize the cache for this round */
  WaysCache *ways_cache = GetWaysCache();
  /* The cache grows its arrays in its own context */
  MemoryContext oldcontext = MemoryContextSwitchTo(WAYS_CONTEXT);
  bool result = ways_cache_lookup(ways_cache, gid, any_gid, get_ways_record,
    rec);
  MemoryContextSwitchTo(oldcontext);
  return result;
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
  ways_record rec;
  if (! route_lookup(rid, false, &rec))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route " INT64_FORMAT, rid);
    return NULL;
  }
  if (! ensure_not_empty(rec.the_geom))
    return NULL;
  return rec.the_line;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return the largest absolute coordinate of the routes in the ways
 * table
 * @details The value is read once per backend from the extent of the table,
 * whose boxes PostGIS rounds outwards, so that it bounds every coordinate of
 * every route; the ways cache reads the network as the same one for the
 * whole session. It must be called within an SPI connection.
 */
static double
ways_magnitude(void)
{
  if (WAYS_MAGNITUDE >= 0.0)
    return WAYS_MAGNITUDE;
  const char *sql =
    "SELECT GREATEST(abs(ST_XMin(e)), abs(ST_XMax(e)), abs(ST_YMin(e)), "
    "abs(ST_YMax(e))) FROM (SELECT ST_Extent(the_geom) AS e "
    "FROM public.ways) AS s";
  int ret = SPI_execute(sql, true, 1);
  if (ret == SPI_OK_SELECT && SPI_processed == 1 && SPI_tuptable)
  {
    bool isnull;
    Datum value = SPI_getbinval(SPI_tuptable->vals[0],
      SPI_tuptable->tupdesc, 1, &isnull);
    WAYS_MAGNITUDE = isnull ? 0.0 : DatumGetFloat8(value);
  }
  return Max(WAYS_MAGNITUDE, 0.0);
}

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

  /* The table answers the routes whose bounding box meets the square of the
   * pad around the point, which a spatial index on the routes serves. The
   * boxes of PostGIS are rounded outwards, so that these routes include every
   * route the point lies on, and #route_nearest_update then decides, as the
   * MEOS build decides it. For a tolerance the pad is the tolerance. For the
   * casts it bounds the box of #route_constructs_point for every segment of
   * the table, 2.5 DBL_EPSILON times the largest coordinate M of the routes,
   * with the widening of that box and the rounding of the pad: 4 DBL_EPSILON
   * times M. The point is passed in binary, which keeps every bit of its
   * coordinates, made in the context of the caller before SPI_connect(),
   * whose context SPI_finish() deletes. */
  size_t size;
  uint8_t *wkb = geo_as_ewkb(gs, "NDR", &size);
  bytea *point = bstring2bytea(wkb, size);
  pfree(wkb);
  SPI_connect();
  double pad = constructed ? 4.0 * DBL_EPSILON * ways_magnitude() :
    tolerance;
  Oid argtypes[2] = {BYTEAOID, FLOAT8OID};
  Datum values[2] = {PointerGetDatum(point), Float8GetDatum(pad)};
  const char *sql =
    "SELECT gid, the_geom FROM public.ways "
    "WHERE the_geom && ST_Expand(ST_GeomFromEWKB($1), $2)";

  /* Route, distance and position of the nearest route */
  int64 min_gid = 0;
  double min_dist = DBL_MAX, min_pos = 0.0;
  int ret = SPI_execute_with_args(sql, 2, argtypes, values, NULL, true, 0);
  if (ret == SPI_OK_SELECT && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    for (uint64 i = 0; i < SPI_processed; i++)
    {
      bool gid_isnull, geom_isnull;
      Datum gid = SPI_getbinval(tuptable->vals[i], tuptable->tupdesc, 1,
        &gid_isnull);
      Datum geom = SPI_getbinval(tuptable->vals[i], tuptable->tupdesc, 2,
        &geom_isnull);
      if (gid_isnull || geom_isnull)
        continue;
      GSERIALIZED *line = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
      if (! ensure_valid_route(line, srid_ways))
      {
        SPI_finish();
        pfree(point);
        return NULL;
      }
      route_nearest_update(gs, DatumGetInt64(gid), line, tolerance,
        constructed, &min_gid, &min_dist, &min_pos);
    }
  }
  SPI_finish();
  pfree(point);
  /* The result is made after SPI_finish(), in the context of the caller */
  return (min_dist == DBL_MAX) ? NULL : npoint_make(min_gid, min_pos);
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
