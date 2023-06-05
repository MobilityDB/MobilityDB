/*****************************************************************************
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
 * @brief Spatial functions for temporal network points.
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_restrfuncs.h"
#include "npoint/tnpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal network point
 * @sqlfunc SRID()
 */
Datum
Tnpoint_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tnpoint_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Geometric positions (Trajectory) functions
 * Return the geometric positions covered by a temporal network point
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_trajectory(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_trajectory);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the geometry covered by a temporal network point
 * @sqlfunc trajectory()
 */
Datum
Tnpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tnpoint_geom(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Geographical equality for network points
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_same);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Determines the spatial equality for network points
 * @sqlfunc equals()
 */
Datum
Npoint_same(PG_FUNCTION_ARGS)
{
  Npoint *np1 = PG_GETARG_NPOINT_P(0);
  Npoint *np2 = PG_GETARG_NPOINT_P(1);
  PG_RETURN_BOOL(npoint_same(np1, np2));
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Length traversed by a temporal network point
 * @sqlfunc length()
 */
Datum
Tnpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tnpoint_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Tnpoint_cumulative_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_cumulative_length);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Cumulative length traversed by a temporal network point
 * @sqlfunc cumulativeLength()
 */
Datum
Tnpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_speed);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Speed of a temporal network point
 * @sqlfunc speed()
 */
Datum
Tnpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal network points
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_twcentroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_twcentroid);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighed centroid of a temporal network point
 * @sqlfunc twcentroid()
 */
Datum
Tnpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = tnpoint_twcentroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_azimuth(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_azimuth);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Temporal azimuth of a temporal network point
 * @sqlfunc azimuth()
 */
Datum
Tnpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Restrict a temporal network point to (the complement of) a geometry
 */
static Datum
tnpoint_restrict_geom_time_ext(FunctionCallInfo fcinfo, bool atfunc,
  bool resttime)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)|| (resttime && PG_ARGISNULL(2)))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Span *period = NULL;
  if (PG_NARGS() > 2)
    period = PG_GETARG_SPAN_P(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tnpoint_restrict_geom_time(temp, gs, NULL, period,
    atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tnpoint_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restricts a temporal point to a geometry
 * @sqlfunc atGeometry()
 */
Datum
Tnpoint_at_geom(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geom_time_ext(fcinfo, REST_AT, REST_TIME_NO);
}

PGDLLEXPORT Datum Tnpoint_at_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restricts a temporal point to a geometry
 * @sqlfunc atGeometry()
 */
Datum
Tnpoint_at_geom_time(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geom_time_ext(fcinfo, REST_AT, REST_TIME);
}

PGDLLEXPORT Datum Tnpoint_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a geometry
 * @sqlfunc minusGeometry()
 */
Datum
Tnpoint_minus_geom(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geom_time_ext(fcinfo, REST_MINUS, REST_TIME_NO);
}

PGDLLEXPORT Datum Tnpoint_minus_geom_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_geom_time);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Restrict a temporal point to the complement of a geometry
 * @sqlfunc minusGeometry()
 */
Datum
Tnpoint_minus_geom_time(PG_FUNCTION_ARGS)
{
  return tnpoint_restrict_geom_time_ext(fcinfo, REST_MINUS, REST_TIME);
}

/*****************************************************************************/
