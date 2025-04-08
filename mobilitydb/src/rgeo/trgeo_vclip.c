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
 * @brief Distance functions for temporal rigid geometries
 */

#include "rgeo/trgeo_vclip.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <fmgr.h>
#include <utils/timestamp.h>
#include <utils/float.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"
#include "general/meos_catalog.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo.h"
/* MobilityDB */
#include "pg_geo/postgis.h"

/*****************************************************************************
 * V-clip
 *****************************************************************************/

PG_FUNCTION_INFO_V1(VClip_poly_point);
/**
 * @brief Return the temporal distance between the geometry/geography 
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_poly_point(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs_poly = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs_point = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs_poly) || gserialized_is_empty(gs_point))
    PG_RETURN_NULL();
  /* TODO: check SRID, geometry type and dimension */
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly));
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs_point));
  uint32_t poly_feature = 0;
  double distance;
  v_clip_tpoly_point(poly, point, NULL, &poly_feature, &distance);
  lwpoly_free(poly);
  lwpoint_free(point);
  PG_FREE_IF_COPY(gs_poly, 0);
  PG_FREE_IF_COPY(gs_point, 1);
  PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(VClip_poly_poly);
/**
 * @brief Return the temporal distance between the geometry/geography
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_poly_poly(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs_poly1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs_poly2 = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs_poly1) || gserialized_is_empty(gs_poly2))
    PG_RETURN_NULL();
  /* TODO: check SRID, geometry type and dimension */
  LWPOLY *poly1 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly1));
  LWPOLY *poly2 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly2));
  uint32_t poly1_feature = 0, poly2_feature = 0;
  double distance;
  v_clip_tpoly_tpoly(poly1, poly2, NULL, NULL, &poly1_feature, &poly2_feature,
    &distance);
  lwpoly_free(poly1);
  lwpoly_free(poly2);
  PG_FREE_IF_COPY(gs_poly1, 0);
  PG_FREE_IF_COPY(gs_poly2, 1);
  PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(VClip_tpoly_point);
/**
 * @brief Return the temporal distance between the geometry/geography
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_tpoly_point(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs_point = PG_GETARG_GSERIALIZED_P(1);
  TimestampTz ts = PG_GETARG_TIMESTAMPTZ(2);
  if (gserialized_is_empty(gs_point))
    PG_RETURN_NULL();
  /* TODO: check SRID, geometry type and dimension */
  Datum value;
  bool found = temporal_value_at_timestamptz(temp, ts, false, &value);
  if (!found)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs_point, 1);
    PG_RETURN_NULL();
  }
  const GSERIALIZED *gs_poly = trgeo_geom_p(temp);
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly));
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs_point));
  Pose *pose = DatumGetPoseP(value);
  uint32_t poly_feature = 0;
  double distance;
  v_clip_tpoly_point(poly, point, pose, &poly_feature, &distance);
  lwpoly_free(poly); lwpoint_free(point);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs_point, 1);
  PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(VClip_tpoly_poly);
/**
 * @brief Return the temporal distance between the geometry/geography
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_tpoly_poly(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs_poly2 = PG_GETARG_GSERIALIZED_P(1);
  TimestampTz ts = PG_GETARG_TIMESTAMPTZ(2);
  if (gserialized_is_empty(gs_poly2))
    PG_RETURN_NULL();
  /* TODO: check SRID, geometry type and dimension */
  Datum value;
  bool found = temporal_value_at_timestamptz(temp, ts, false, &value);
  if (!found)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs_poly2, 1);
    PG_RETURN_NULL();
  }
  const GSERIALIZED *gs_poly1 = trgeo_geom_p(temp);
  LWPOLY *poly1 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly1));
  LWPOLY *poly2 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly2));
  Pose *pose1 = DatumGetPoseP(value);
  uint32_t poly1_feature = 0, poly2_feature = 0;
  double distance;
  v_clip_tpoly_tpoly(poly1, poly2, pose1, NULL, &poly1_feature, &poly2_feature,
    &distance);
  lwpoly_free(poly1);
  lwpoly_free(poly2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs_poly2, 1);
  PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(VClip_tpoly_tpoint);
/**
 * @brief Return the temporal distance between the geometry/geography
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_tpoly_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TimestampTz ts = PG_GETARG_TIMESTAMPTZ(2);
  /* TODO: check SRID, geometry type and dimension */
  Datum value1, value2;
  bool found1 = temporal_value_at_timestamptz(temp1, ts, false, &value1);
  bool found2 = temporal_value_at_timestamptz(temp2, ts, false, &value2);
  if (!found1 || !found2)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }
  const GSERIALIZED *gs_poly = trgeo_geom_p(temp1);
  LWPOLY *poly = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly));
  Pose *pose = DatumGetPoseP(value1);
  GSERIALIZED *gs_point = DatumGetGserializedP(value2);
  LWPOINT *point = lwgeom_as_lwpoint(lwgeom_from_gserialized(gs_point));
  uint32_t poly_feature = 0;
  double distance;
  v_clip_tpoly_point(poly, point, pose, &poly_feature, &distance);
  lwpoly_free(poly);
  lwpoint_free(point);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(distance);
}

PG_FUNCTION_INFO_V1(VClip_tpoly_tpoly);
/**
 * @brief Return the temporal distance between the geometry/geography
 * point/polygon and the temporal rigid geometry
 */
PGDLLEXPORT Datum
VClip_tpoly_tpoly(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TimestampTz ts = PG_GETARG_TIMESTAMPTZ(2);
  /* TODO: check SRID, geometry type and dimension */
  Datum value1, value2;
  bool found1 = temporal_value_at_timestamptz(temp1, ts, false, &value1);
  bool found2 = temporal_value_at_timestamptz(temp2, ts, false, &value2);
  if (!found1 || !found2)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }
  const GSERIALIZED *gs_poly1 = trgeo_geom_p(temp1);
  LWPOLY *poly1 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly1));
  Pose *pose1 = DatumGetPoseP(value1);
  const GSERIALIZED *gs_poly2 = trgeo_geom_p(temp2);
  LWPOLY *poly2 = lwgeom_as_lwpoly(lwgeom_from_gserialized(gs_poly2));
  Pose *pose2 = DatumGetPoseP(value2);
  uint32_t poly1_feature = 0, poly2_feature = 0;
  double distance;
  v_clip_tpoly_tpoly(poly1, poly2, pose1, pose2, &poly1_feature,
    &poly2_feature, &distance);
  lwpoly_free(poly1);
  lwpoly_free(poly2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************/
