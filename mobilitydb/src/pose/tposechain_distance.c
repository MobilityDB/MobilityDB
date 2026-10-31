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
 * @brief Temporal distance for temporal pose chains
 */

/* C */
#include <float.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "geo/stbox.h"
#include "pose/posechain.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_posechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_posechain_geo);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the distance between a pose chain and a geometry
 * @sqlfn distance()
 * @sqlop @p <->
 */
Datum
Distance_posechain_geo(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double result = distance_posechain_geo(pc, gs);
  PG_FREE_IF_COPY(gs, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_geo_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_geo_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the distance between a geometry and a pose chain
 * @sqlfn distance()
 * @sqlop @p <->
 */
Datum
Distance_geo_posechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  double result = distance_posechain_geo(pc, gs);
  PG_FREE_IF_COPY(gs, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Distance_posechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_posechain_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the distance between two pose chains
 * @sqlfn distance()
 * @sqlop @p <->
 */
Datum
Distance_posechain_posechain(PG_FUNCTION_ARGS)
{
  PoseChain *pc1 = PG_GETARG_POSECHAIN_P(0);
  PoseChain *pc2 = PG_GETARG_POSECHAIN_P(1);
  double result = distance_posechain_posechain(pc1, pc2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Tdistance_point_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_point_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the temporal distance between a geometry point and a temporal
 * pose chain
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_point_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tdistance_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the temporal distance between a temporal pose chain and a
 * geometry point
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_tposechain_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tdistance_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the temporal distance between a pose chain and a temporal pose
 * chain
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_posechain_tposechain(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tdistance_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the temporal distance between a temporal pose chain and a pose
 * chain
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_tposechain_posechain(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  Temporal *result = tdistance_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the temporal distance between two temporal pose chains
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tdistance_tposechain_tposechain(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * pose chain
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_geo_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach instant between a temporal pose chain and
 * a geometry
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tposechain_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = nai_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach instant between a pose chain and a
 * temporal pose chain
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_posechain_tposechain(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach instant between a temporal pose chain and
 * a pose chain
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tposechain_posechain(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  TInstant *result = nai_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach instant between two temporal pose chains
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tposechain_tposechain(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/* GENERATED-NADOPS-BEGIN tposechain — tools/codegen/inherited/generate.py from templates/nadpairs.c.tmpl; DO NOT EDIT BY HAND;
 * edit the template + manifest.d/boxtypes.yaml and re-run. */
PGDLLEXPORT Datum NAD_posechain_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_posechain_stbox);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a pose chain and a
 * spatiotemporal box
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_posechain_stbox(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  double result = distance_posechain_stbox(pc, box);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a pose chain
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_posechain(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  double result = distance_posechain_stbox(pc, box);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum NAD_geo_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a geometry and a temporal
 * pose chain
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_geo_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a temporal pose chain and
 * a geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tposechain_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double result = nad_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum NAD_tposechain_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tposechain_stbox);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a temporal pose chain and
 * a spatiotemporal box
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tposechain_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  double result = nad_tposechain_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal pose chain
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_tposechain(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tposechain_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum NAD_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a pose chain and a
 * temporal pose chain
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_posechain_tposechain(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between a temporal pose chain and
 * a pose chain
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tposechain_posechain(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  double result = nad_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum NAD_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the nearest approach distance between two temporal pose chains
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tposechain_tposechain(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}
/* GENERATED-NADOPS-END tposechain */

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal pose chain
 * @sqlfn shortestLine()
 */
Datum
Shortestline_geo_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal pose chain and a geometry
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tposechain_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result = shortestline_tposechain_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_posechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_posechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the line connecting the nearest approach point between a pose
 * chain and a temporal pose chain
 * @sqlfn shortestLine()
 */
Datum
Shortestline_posechain_tposechain(PG_FUNCTION_ARGS)
{
  PoseChain *pc = PG_GETARG_POSECHAIN_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tposechain_posechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tposechain_posechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal pose chain and a pose chain
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tposechain_posechain(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PoseChain *pc = PG_GETARG_POSECHAIN_P(1);
  GSERIALIZED *result = shortestline_tposechain_posechain(temp, pc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Shortestline_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal pose chains
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tposechain_tposechain(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
