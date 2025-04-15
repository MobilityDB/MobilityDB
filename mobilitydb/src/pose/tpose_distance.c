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
 * @brief Temporal distance for temporal poses
 */

#include "pose/tpose_distance.h"

/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_point_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_point_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the temporal distance between a geometry point and a temporal
 * pose
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_point_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tpose_point(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tpose_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tpose_point);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the temporal distance between a temporal pose and a geometry
 * point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tpose_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = distance_tpose_point(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_pose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the temporal distance between a pose and a temporal pose
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_pose_tpose(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tpose_pose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the temporal distance between a temporal pose and a pose
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tpose_pose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  Temporal *result = distance_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the temporal distance between two temporal poses
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tpose_tpose(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * pose
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_geo_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tpose_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tpose_geo);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach instant between a temporal pose and a
 * geometry
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tpose_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = nai_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_pose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach instant between a pose and a temporal
 * pose
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_pose_tpose(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tpose_pose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach instant between a temporal pose and a
 * pose
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tpose_pose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  TInstant *result = nai_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach instant between two temporal poses
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tpose_tpose(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

PGDLLEXPORT Datum NAD_geo_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach distance between a geometry and a
 * temporal pose
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_geo_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpose_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpose_geo);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach distance between a temporal pose and a
 * geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tpose_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = nad_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_pose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach distance between a pose and a temporal
 * pose
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_pose_tpose(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpose_pose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach distance between a temporal pose and a
 * pose
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tpose_pose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  double result = nad_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the nearest approach distance between two temporal poses
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tpose_tpose(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal pose
 * @sqlfn shortestLine()
 */
Datum
Shortestline_geo_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tpose_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tpose_geo);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal pose and a geometry
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tpose_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result = shortestline_tpose_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_pose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_pose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the line connecting the nearest approach point between a pose
 * and a temporal pose
 * @sqlfn shortestLine()
 */
Datum
Shortestline_pose_tpose(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tpose_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tpose_pose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the line connecting the nearest approach point between a 
 * temporal pose and a pose
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tpose_pose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  GSERIALIZED *result = shortestline_tpose_pose(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Shortestline_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal poses
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tpose_tpose(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
