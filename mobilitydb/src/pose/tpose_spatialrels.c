/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Ever spatial relationships for temporal poses
 *
 * These functions dispatch to the temporal geometry point implementation
 * through the lossless conversion of a temporal pose into a temporal geometry
 * point. They are defined as C functions (rather than SQL wrappers casting to
 * `tgeompoint`) so that the planner support function `tspatial_supportfn`
 * fires and the spatial index (GiST/SP-GiST) accelerates the predicate.
 */

/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "temporal/temporal.h"
#include "geo/tgeo_spatialrels.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tpose);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if a geometry and a temporal pose ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_geo_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *tpoint = tpose_to_tpoint(temp);
  int result = ea_intersects_tgeo_geo(tpoint, gs, EVER);
  pfree(tpoint);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Eintersects_tpose_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpose_geo);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if a temporal pose and a geometry ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tpose_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *tpoint = tpose_to_tpoint(temp);
  int result = ea_intersects_tgeo_geo(tpoint, gs, EVER);
  pfree(tpoint);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Eintersects_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if two temporal poses ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *tpoint1 = tpose_to_tpoint(temp1);
  Temporal *tpoint2 = tpose_to_tpoint(temp2);
  int result = ea_intersects_tgeo_tgeo(tpoint1, tpoint2, EVER);
  pfree(tpoint1); pfree(tpoint2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************
 * Ever dwithin
 *****************************************************************************/

PGDLLEXPORT Datum Edwithin_geo_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tpose);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if a geometry and a temporal pose are ever within a
 * distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_geo_tpose(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint = tpose_to_tpoint(temp);
  int result = ea_dwithin_tgeo_geo(tpoint, gs, dist, EVER);
  pfree(tpoint);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edwithin_tpose_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpose_geo);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if a temporal pose and a geometry are ever within a
 * distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tpose_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint = tpose_to_tpoint(temp);
  int result = ea_dwithin_tgeo_geo(tpoint, gs, dist, EVER);
  pfree(tpoint);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edwithin_tpose_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpose_tpose);
/**
 * @ingroup mobilitydb_pose_rel_ever
 * @brief Return true if two temporal poses are ever within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tpose_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint1 = tpose_to_tpoint(temp1);
  Temporal *tpoint2 = tpose_to_tpoint(temp2);
  int result = ea_dwithin_tgeo_tgeo(tpoint1, tpoint2, dist, EVER);
  pfree(tpoint1); pfree(tpoint2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************/
