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
 * @brief Temporal distance for temporal network points.
 */

#include "npoint/tnpoint_distance.h"

/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_point_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_point_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a geometry point and a
 * temporal network point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_point_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_point(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tnpoint_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tnpoint_point);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal network point and
 * a geometry point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tnpoint_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = distance_tnpoint_point(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a network point and a
 * temporal network point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal network point and
 * the network point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Temporal *result = distance_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal network point and
 * the network point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * network point
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a temporal network point
 * and a geometry
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = nai_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a network point and a
 * temporal network point
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a temporal network point
 * and a network point
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  TInstant *result = nai_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between two temporal network
 * points
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

PGDLLEXPORT Datum NAD_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a geometry and a
 * temporal network point
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal network point
 * and a geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = nad_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a network point and a
 * temporal network point
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal network point
 * and a network point
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  double result = nad_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between two temporal network
 * points
 * @sqlfn nearestApproachDistance()
 * @sqlop |=|
 */
Datum
NAD_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal network point
 * @sqlfn shortestLine()
 */
Datum
Shortestline_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a temporal
 * network point and a geometry
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result = shortestline_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a network
 * point and a temporal network point
 * @sqlfn shortestLine()
 */
Datum
Shortestline_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a temporal
 * network point and a network point
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  GSERIALIZED *result = shortestline_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Shortestline_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal networks
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
