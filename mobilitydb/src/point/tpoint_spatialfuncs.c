/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Spatial functions for temporal points
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/type_round.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/stbox.h"
#include "point/tpoint_restrfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Trajectory function
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_trajectory(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_trajectory);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the trajectory of a temporal point
 * @sqlfn trajectory()
 */
Datum
Tpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tpoint_trajectory(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tgeogpoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal geometry point converted to a temporal geography
 * point
 * @sqlfn tgeogpoint()
 * @sqlop @p ::
 */
Datum
Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOM_TO_GEOG);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal geography point converted to a temporal geometry
 * point
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOG_TO_GEOM);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Set precision of the coordinates
 *****************************************************************************/

PGDLLEXPORT Datum Geo_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a geometry/geography with the precision of the coordinates set
 * to a number of decimal places
 * @sqlfn round()
 */
Datum
Geo_round(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum result = datum_round_geo(PointerGetDatum(gs), size);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Geoset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a geo set with the precision of the coordinates set to a
 * number of decimals places
 * @sqlfn round()
 */
Datum
Geoset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int maxdd = PG_GETARG_INT32(1);
  Set *result = geoset_round(s, maxdd);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Tpoint_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal point with the precision of the coordinates set to
 * a number of decimal places
 * @sqlfn round()
 */
Datum
Tpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tpoint_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpointarr_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointarr_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return an array of temporal points with the precision of the
 * coordinates set to a number of decimal places
 * @sqlfn round()
 */
Datum
Tpointarr_round(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int maxdd = PG_GETARG_INT32(1);

  Temporal **temparr = temparr_extract(array, &count);
  Temporal **resarr = tpointarr_round((const Temporal **) temparr, count,
      maxdd);
  ArrayType *result = temparr_to_array(resarr, count, FREE_ALL);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_to_geomeas(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_to_geomeas);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal point converted to a geometry/geography with
 * M measure
 * @sqlfn geometry(), geography()
 * @sqlop @p ::
 */
Datum
Tpoint_to_geomeas(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL_P(0);
  bool segmentize = (PG_NARGS() == 2) ? PG_GETARG_BOOL(1) : false;
  GSERIALIZED *result;
  tpoint_tfloat_to_geomeas(tpoint, NULL, segmentize, &result);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geomeas_to_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geomeas_to_tpoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a geometry/geography with M measure converted to a temporal
 * point
 * @sqlfn tgeompoint(), tgeogpoint()
 * @sqlop @p ::
 */
Datum
Geomeas_to_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *result = geomeas_to_tpoint(gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_tfloat_to_geomeas(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_tfloat_to_geomeas);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a geometry/geography with M measure converted from a temporal
 * point and a temporal float
 * @sqlfn geoMeasure()
 */
Datum
Tpoint_tfloat_to_geomeas(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL_P(0);
  Temporal *measure = PG_GETARG_TEMPORAL_P(1);
  bool segmentize = (PG_NARGS() == 3) ? PG_GETARG_BOOL(2) : false;
  GSERIALIZED *result;
  bool found = tpoint_tfloat_to_geomeas(tpoint, measure, segmentize, &result);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_FREE_IF_COPY(measure, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Mapbox Vector Tile functions for temporal points.
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_AsMVTGeom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_AsMVTGeom);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return a temporal point transformed to the Mapbox Vector Tile
 * representation
 * @sqlfn asMVTGeom()
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

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_get_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_x);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the X coordinates of a temporal point as a temporal float
 * @sqlfn getX()
 */
Datum
Tpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_get_y(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_y);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the Y coordinates of a temporal point as a temporal float
 * @sqlfn getY()
 */
Datum
Tpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 1);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_get_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_get_z);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the Z coordinates of a temporal point as a temporal float
 * @sqlfn getZ()
 */
Datum
Tpoint_get_z(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the length traversed by a temporal sequence (set) point
 * @sqlfn length()
 */
Datum
Tpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tpoint_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tpoint_cumulative_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_cumulative_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the cumulative length traversed by a temporal sequence (set)
 * point
 * @sqlfn cumulativeLength()
 */
Datum
Tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_convex_hull);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the convex hull of  a temporal point
 * @sqlfn convexHull()
 */
Datum
Tpoint_convex_hull(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tpoint_convex_hull(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_speed);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the speed of a temporal point
 * @sqlfn speed()
 */
Datum
Tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Direction function
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_direction(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_direction);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the azimuth between the first and the last points a temporal
 * point
 * @sqlfn direction()
 */
Datum
Tpoint_direction(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result;
  bool found = tpoint_direction(temp, &result);
  PG_FREE_IF_COPY(temp, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_twcentroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_twcentroid);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighed centroid of a temporal geometry point
 * @sqlfn twCentroid()
 */
Datum
Tpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tpoint_twcentroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_azimuth(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_azimuth);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal geometry point
 * @sqlfn azimuth()
 */
Datum
Tpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal angular difference
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_angular_difference(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_angular_difference);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal angular difference of a temporal geometry point
 * @sqlfn angularDifference()
 */
Datum
Tpoint_angular_difference(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_angular_difference(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

PGDLLEXPORT Datum Bearing_point_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_point_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two geometry/geography points
 * @note The following function is meant to be included in PostGIS one day
 * @sqlfn bearing()
 */
Datum
Bearing_point_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *geo2 = PG_GETARG_GSERIALIZED_P(1);
  double result;
  bool found = bearing_point_point(geo1, geo2, &result);
  PG_FREE_IF_COPY(geo1, 0);
  PG_FREE_IF_COPY(geo2, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Bearing_point_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_point_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a geometry/geography point
 * and a temporal point
 * @sqlfn bearing()
 */
Datum
Bearing_point_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = bearing_tpoint_point(temp, gs, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Bearing_tpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_tpoint_point);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between a temporal point and a
 * geometry/geography point
 * @sqlfn bearing()
 */
Datum
Bearing_tpoint_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = bearing_tpoint_point(temp, gs, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Bearing_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bearing_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal bearing between two temporal points
 * @sqlfn bearing()
 */
Datum
Bearing_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = bearing_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_is_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_is_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return true if a temporal point does not self-intersect
 * @sqlfn isSimple()
 */
Datum
Tpoint_is_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool result = tpoint_is_simple(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tpoint_make_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_make_simple);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Return the array of non self-intersecting fragments of a temporal point
 * @sqlfn makeSimple()
 */
Datum
Tpoint_make_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Temporal **fragments = tpoint_make_simple(temp, &count);
  ArrayType *result = temparr_to_array(fragments, count, FREE_ALL);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Return a temporal point restricted to (the complement of) a geometry
 * and possibly a timestamptz span
 * @note Mixing 2D/3D is enabled to compute, for example, 2.5D operations.
 * However the geometry must be in 2D.
 */
static Datum
Tpoint_restrict_geom_time(FunctionCallInfo fcinfo, bool atfunc, bool resttime)
{
  /*
  CREATE FUNCTION at/minusGeometry(tgeompoint, geometry)
  CREATE FUNCTION at/minusGeometry(tgeompoint, geometry, floatspan)
  CREATE FUNCTION at/minusGeometryTime(tgeompoint, geometry, tstzspan)
  CREATE FUNCTION at/minusGeometryTime(tgeompoint, geometry, floatspan, tstzspan)
  */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Span *zspan = NULL;
  Span *period = NULL;
  if (PG_NARGS() == 3)
  {
    if (resttime)
      period = PG_GETARG_SPAN_P(2);
    else
      zspan = PG_GETARG_SPAN_P(2);
  }
  else if (PG_NARGS() == 4)
  {
    zspan = PG_GETARG_SPAN_P(2);
    period = PG_GETARG_SPAN_P(3);
  }
  Temporal *result = tpoint_restrict_geom_time(temp, geo, zspan, period,
    atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to a geometry
 * @sqlfn atGeometry()
 */
Datum
Tpoint_at_geom(PG_FUNCTION_ARGS)
{
  return Tpoint_restrict_geom_time(fcinfo, REST_AT, REST_TIME_NO);
}

PGDLLEXPORT Datum Tpoint_at_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to a geometry and a time value
 * @sqlfn atGeometryTime()
 */
Datum
Tpoint_at_geom_time(PG_FUNCTION_ARGS)
{
  return Tpoint_restrict_geom_time(fcinfo, REST_AT, REST_TIME);
}

PGDLLEXPORT Datum Tpoint_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to the complement of a geometry
 * @sqlfn minusGeometry()
 */
Datum
Tpoint_minus_geom(PG_FUNCTION_ARGS)
{
  return Tpoint_restrict_geom_time(fcinfo, REST_MINUS, REST_TIME_NO);
}

PGDLLEXPORT Datum Tpoint_minus_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to the complement of a geometry
 * and a time value
 * @sqlfn minusGeometryTime()
 */
Datum
Tpoint_minus_geom_time(PG_FUNCTION_ARGS)
{
  return Tpoint_restrict_geom_time(fcinfo, REST_MINUS, REST_TIME);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tpoint_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_at_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to a spatiotemporal box
 * @sqlfn atStbox()
 */
Datum
Tpoint_at_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, BORDER_INC, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpoint_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_minus_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal point restricted to the complement of a
 * spatiotemporal box
 * @sqlfn minusStbox()
 */
Datum
Tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, BORDER_INC, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
