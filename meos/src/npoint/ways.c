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
#include "temporal/type_inout.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

#define SQL_ROUTE_MAXLEN  64

/* Global variable saving the SRID of the ways table */
static int32_t SRID_WAYS = SRID_INVALID;

/*****************************************************************************
 * Route functions
 *****************************************************************************/

/**
 * @ingroup meos_npoint_base_srid
 * @brief Return the SRID of the routes in the ways table
 * @return On error return SRID_INVALID
 */
int32_t
get_srid_ways()
{
  /* Get the value from the global variable if it has been already set */
  if (SRID_WAYS != SRID_INVALID)
    return SRID_WAYS;

  /* Fetch the SRID value from the table */
  int32_t result = 0; /* make compiler quiet */
  bool isNull = true;
  SPI_connect();
  int ret = SPI_execute("SELECT public.ST_SRID(the_geom) FROM public.ways LIMIT 1;",
    true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum value = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (isNull)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Cannot determine SRID of the ways table");
      return SRID_INVALID;
    }
    result = DatumGetInt32(value);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
      "Cannot determine SRID of the ways table");
    return SRID_INVALID;
  }
  SPI_finish();
  /* Save the SRID value in a global variable */
  SRID_WAYS = result;
  return result;
}

#define SQL_ROUTE_MAXLEN 64

/**
 * @ingroup meos_npoint_base_route
 * @brief Return true if the edge table contains a route with the route
 * identifier
 * @param[in] rid Route identifier
 */
bool
route_exists(int64 rid)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT true FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  bool result = false;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetBool(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();
  return result;
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
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT length FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  double result = 0.0;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    result = DatumGetFloat8(SPI_getbinval(tuptable->vals[0],
      tuptable->tupdesc, 1, &isNull));
  }
  SPI_finish();

  if (isNull)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the length for route %ld", rid);
    return -1.0;
  }
  return result;
}

/**
 * @ingroup meos_npoint_base_route
 * @brief Access the edge table to get the route geometry from corresponding
 * route identifier
 * @param[in] rid Route identifier
 * @return On error return @p NULL
 */
GSERIALIZED *
route_geom(int64 rid)
{
  char sql[SQL_ROUTE_MAXLEN];
  snprintf(sql, sizeof(sql),
    "SELECT the_geom FROM public.ways WHERE gid = %ld", rid);
  bool isNull = true;
  GSERIALIZED *result = NULL;
  SPI_connect();
  int ret = SPI_execute(sql, true, 1);
  uint64 proc = SPI_processed;
  if (ret > 0 && proc > 0 && SPI_tuptable)
  {
    SPITupleTable *tuptable = SPI_tuptable;
    Datum line = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
    if (! isNull)
    {
      /* Must allocate this in upper executor context to keep it alive after SPI_finish() */
      GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(line);
      result = (GSERIALIZED *) SPI_palloc(gs->size);
      memcpy(result, gs, gs->size);
    }
  }
  SPI_finish();

  if (isNull)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot get the geometry for route %ld", rid);
    return NULL;
  }
  if (! ensure_not_empty(result))
  {
    pfree(result);
    return NULL;
  }
  return result;
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
