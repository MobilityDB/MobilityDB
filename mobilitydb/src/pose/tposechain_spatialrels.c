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
 * @brief Ever spatial relationships for temporal pose chains
 * @details These functions dispatch to the temporal geometry point
 * implementation through the temporal pose a temporal pose chain composes to
 * (#tposechain_to_tpose) and the temporal geometry point of its position
 * (#tpose_to_tpoint), the conversion the casts `tposechain::tpose` and
 * `tpose::tgeompoint` perform. The box of a temporal pose chain bounds every
 * joint, the composed pose among them, so it bounds the temporal geometry
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

/**
 * @brief Return the temporal geometry point of the temporal pose a temporal
 * pose chain composes to
 */
static Temporal *
tposechain_tpoint(const Temporal *temp)
{
  Temporal *tpose = tposechain_to_tpose(temp);
  Temporal *result = tpose_to_tpoint(tpose);
  pfree(tpose);
  return result;
}

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tposechain);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if a geometry and a temporal pose chain ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_geo_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *tpoint = tposechain_tpoint(temp);
  int result = eintersects_geo_tgeo(gs, tpoint);
  pfree(tpoint);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Eintersects_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if a temporal pose chain and a geometry ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tposechain_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *tpoint = tposechain_tpoint(temp);
  int result = eintersects_tgeo_geo(tpoint, gs);
  pfree(tpoint);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Eintersects_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if two temporal pose chains ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *tpoint1 = tposechain_tpoint(temp1);
  Temporal *tpoint2 = tposechain_tpoint(temp2);
  int result = eintersects_tgeo_tgeo(tpoint1, tpoint2);
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

PGDLLEXPORT Datum Edwithin_geo_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tposechain);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if a geometry and a temporal pose chain are ever within a
 * distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_geo_tposechain(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint = tposechain_tpoint(temp);
  int result = edwithin_geo_tgeo(gs, tpoint, dist, true);
  pfree(tpoint);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edwithin_tposechain_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tposechain_geo);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if a temporal pose chain and a geometry are ever within a
 * distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tposechain_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint = tposechain_tpoint(temp);
  int result = edwithin_tgeo_geo(tpoint, gs, dist, true);
  pfree(tpoint);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edwithin_tposechain_tposechain(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tposechain_tposechain);
/**
 * @ingroup mobilitydb_posechain_rel_ever
 * @brief Return true if two temporal pose chains are ever within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tposechain_tposechain(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Temporal *tpoint1 = tposechain_tpoint(temp1);
  Temporal *tpoint2 = tposechain_tpoint(temp2);
  int result = edwithin_tgeo_tgeo(tpoint1, tpoint2, dist, true);
  pfree(tpoint1); pfree(tpoint2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************/
