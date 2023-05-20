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
 * @brief Distance functions for temporal points.
 */

#include "point/tpoint_distance.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
#include <utils/float.h>
/* PostGIS */
#include <lwgeodetic_tree.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporaltypes.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a geometry/geography point
 * and a temporal point
 * @sqlfunc temporal_distance()
 * @sqlop @p <->
 */
Datum
Distance_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Distance_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal point and the
 * geometry/geography point
 * @sqlfunc temporal_distance()
 * @sqlop @p <->
 */
Datum
Distance_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Distance_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between two temporal points
 * @sqlfunc temporal_distance()
 * @sqlop @p <->
 */
Datum
Distance_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = distance_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a geometry and
 * a temporal point
 * @sqlfunc nearestApproachInstant()
 */
Datum
NAI_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum NAI_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a temporal point
 * and a geometry
 * @sqlfunc nearestApproachInstant()
 */
Datum
NAI_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between the temporal points
 * @sqlfunc nearestApproachInstant()
 */
Datum
NAI_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

PGDLLEXPORT Datum NAD_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a geometry and
 * a temporal point
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal point and a geometry
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_geo_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a geometry and
 * a spatiotemporal box
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_geo_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_stbox_geo(box, gs);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_geo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_stbox_geo(box, gs);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between the spatio-temporal boxes
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_stbox_stbox(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a spatio-temporal box and a
 * temporal point
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_tpoint(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_tpoint_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpoint_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpoint_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal point and a
 * spatio-temporal box
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpoint_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_tpoint_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between the temporal points
 * @sqlfunc nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * geometry and the temporal instant point
 * @sqlfunc shortestLine()
 */
Datum
Shortestline_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result;
  bool found = shortestline_tpoint_geo(temp, gs, &result);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Shortestline_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * temporal instant point and a geometry/geography
 * @sqlfunc shortestLine()
 */
Datum
Shortestline_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result;
  bool found = shortestline_tpoint_geo(temp, gs, &result);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Shortestline_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * temporal points
 * @sqlfunc shortestLine()
 */
Datum
Shortestline_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  GSERIALIZED *result;
  bool found = shortestline_tpoint_tpoint(temp1, temp2, &result);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
