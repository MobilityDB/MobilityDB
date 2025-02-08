/*****************************************************************************
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
 * @brief Spatial functions for temporal network points.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "point/stbox.h"
#include "point/tpoint_restrfuncs.h"
#include "npoint/tnpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Geometric positions (Trajectory) functions
 * Return the geometric positions covered by a temporal network point
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_trajectory(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_trajectory);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the geometry covered by a temporal network point
 * @sqlfn trajectory()
 */
Datum
Tnpoint_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tnpoint_trajectory(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Geographical equality for network points
 *****************************************************************************/

PGDLLEXPORT Datum Npoint_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npoint_same);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return true if two network points are spatially equal
 * @sqlfn same()
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
 * @brief Return the length traversed by a temporal network point
 * @sqlfn length()
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
 * @brief Return the cumulative length traversed by a temporal network point
 * @sqlfn cumulativeLength()
 */
Datum
Tnpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_speed);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the speed of a temporal network point
 * @sqlfn speed()
 */
Datum
Tnpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal network points
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_twcentroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_twcentroid);
/**
 * @ingroup mobilitydb_temporal_agg
 * @brief Return the time-weighed centroid of a temporal network point
 * @sqlfn twCentroid()
 */
Datum
Tnpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tnpoint_twcentroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_azimuth(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_azimuth);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the temporal azimuth of a temporal network point
 * @sqlfn azimuth()
 */
Datum
Tnpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Return a temporal network point restricted to (the complement of) a
 * geometry
 */
static Datum
Tnpoint_restrict_geom(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tnpoint_restrict_geom(temp, gs, NULL, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnpoint_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal network point restricted to a geometry
 * @sqlfn atGeometry()
 */
Datum
Tnpoint_at_geom(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_geom(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnpoint_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_geom);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal network point restricted to the complement of a
 * geometry
 * @sqlfn minusGeometry()
 */
Datum
Tnpoint_minus_geom(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_geom(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tnpoint_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal network point restricted to a spatiotemporal box
 * @sqlfn atStbox()
 */
Datum
Tnpoint_at_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tnpoint_restrict_stbox(temp, box, border_inc, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnpoint_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_stbox);
/**
 * @ingroup mobilitydb_temporal_restrict
 * @brief Return a temporal network point restricted to the complement of a
 * spatiotemporal box
 * @sqlfn minusStbox()
 */
Datum
Tnpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = tnpoint_restrict_stbox(temp, box, border_inc, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
