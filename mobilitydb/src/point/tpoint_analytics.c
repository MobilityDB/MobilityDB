/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Analytic functions for temporal points and temporal floats.
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom_internal.h>
#include <lwgeodetic_tree.h>
/* MEOS */
#include <meos.h>
#include "general/lifting.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialrels.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_to_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_to_geo);
/**
 * @brief Convert the temporal point into a PostGIS trajectory geometry or
 * geography where the M coordinates encode the timestamps in number of seconds
 * since '1970-01-01'
 */
Datum
Tpoint_to_geo(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL_P(0);
  bool segmentize = (PG_NARGS() == 2) ? PG_GETARG_BOOL(1) : false;
  GSERIALIZED *result;
  tpoint_to_geo_meas(tpoint, NULL, segmentize, &result);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Geo_to_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_to_tpoint);
/**
 * @brief Convert the PostGIS trajectory geometry or geography where the M
 * coordinates encode the timestamps in Unix epoch into a temporal point.
 */
Datum
Geo_to_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *result = geo_to_tpoint(geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpoint_to_geo_meas(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_to_geo_meas);
/**
 * @brief Construct a geometry/geography with M measure from the temporal point
 * and the temporal float
 */
Datum
Tpoint_to_geo_meas(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL_P(0);
  Temporal *measure = PG_GETARG_TEMPORAL_P(1);
  bool segmentize = (PG_NARGS() == 3) ? PG_GETARG_BOOL(2) : false;
  GSERIALIZED *result;
  bool found = tpoint_to_geo_meas(tpoint, measure, segmentize, &result);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_FREE_IF_COPY(measure, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_min_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_dist);
/**
 * @brief Simplify the temporal sequence (set) float or point ensuring that
 * consecutive values are at least a certain distance apart.
 */
Datum
Temporal_simplify_min_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  /* Store fcinfo to compute the geodetic distance */
  store_fcinfo(fcinfo);
  Temporal *result = temporal_simplify_min_dist(temp, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_tdelta);
/**
 * @brief Simplify the temporal sequence (set) float or point ensuring that
 * consecutive values are at least a certain distance apart.
 */
Datum
Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *mint = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_simplify_min_tdelta(temp, mint);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_max_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_max_dist);
/**
 * @brief Simplify the temporal sequence (set) float or point using a
 * single-pass Douglas-Peucker line simplification algorithm.
 */
Datum
Temporal_simplify_max_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_max_dist(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_dp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_dp);
/**
 * @brief Simplify the temporal sequence (set) float or point using a
 * Douglas-Peucker line simplification algorithm.
 */
Datum
Temporal_simplify_dp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_dp(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Mapbox Vector Tile functions for temporal points.
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_AsMVTGeom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_AsMVTGeom);
/**
 * @brief Transform the temporal point to Mapbox Vector Tile format
 */
Datum
Tpoint_AsMVTGeom(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *bounds = PG_GETARG_STBOX_P(1);
  int32_t extent = PG_GETARG_INT32(2);
  int32_t buffer = PG_GETARG_INT32(3);
  bool clip_geom = PG_GETARG_BOOL(4);

  GSERIALIZED *geom;
  int64 *times; /* Timestamps are returned in Unix time */
  int count;
  bool found = tpoint_AsMVTGeom(temp, bounds, extent, buffer, clip_geom,
    &geom, &times, &count);
  if (! found)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_NULL();
  }

  ArrayType *timesarr = int64arr_to_array(times, count);

  /* Build a tuple description for the function output */
  TupleDesc resultTupleDesc;
  get_call_result_type(fcinfo, NULL, &resultTupleDesc);
  BlessTupleDesc(resultTupleDesc);

  /* Construct the result */
  HeapTuple resultTuple;
  bool result_is_null[2] = {0,0}; /* needed to say no value is null */
  Datum result_values[2]; /* used to construct the composite return value */
  Datum result; /* the actual composite return value */
  /* Store geometry */
  result_values[0] = PointerGetDatum(geom);
  /* Store timestamp array */
  result_values[1] = PointerGetDatum(timesarr);
  /* Form tuple and return */
  resultTuple = heap_form_tuple(resultTupleDesc, result_values, result_is_null);
  result = HeapTupleGetDatum(resultTuple);

  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
