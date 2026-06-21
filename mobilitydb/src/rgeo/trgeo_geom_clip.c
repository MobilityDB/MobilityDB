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
 * @brief PG bridge for the swept-edge-polygon clip primitive (M1 + M2)
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "pose/pose.h"
#include "rgeo/trgeo_geom_clip.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

PGDLLEXPORT Datum Trgeometry_geom_clip_polygon(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_geom_clip_polygon);
/**
 * @brief Return the time-parameter intervals during which a moving
 * segment under pure translation intersects a polygon.
 * @sqlfn _trgeometry_geom_clip_polygon()
 */
Datum
Trgeometry_geom_clip_polygon(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs_a1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs_b1 = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *gs_a2 = PG_GETARG_GSERIALIZED_P(2);
  GSERIALIZED *gs_b2 = PG_GETARG_GSERIALIZED_P(3);
  GSERIALIZED *gs_poly = PG_GETARG_GSERIALIZED_P(4);

  if (gserialized_get_type(gs_a1) != POINTTYPE ||
      gserialized_get_type(gs_b1) != POINTTYPE ||
      gserialized_get_type(gs_a2) != POINTTYPE ||
      gserialized_get_type(gs_b2) != POINTTYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The first four arguments must be POINT geometries")));

  const POINT2D *a1 = GSERIALIZED_POINT2D_P(gs_a1);
  const POINT2D *b1 = GSERIALIZED_POINT2D_P(gs_b1);
  const POINT2D *a2 = GSERIALIZED_POINT2D_P(gs_a2);
  const POINT2D *b2 = GSERIALIZED_POINT2D_P(gs_b2);

  LWGEOM *geom = lwgeom_from_gserialized(gs_poly);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_lwgeom(a1, b1, a2, b2, geom, &intervals);
  lwgeom_free(geom);

  PG_FREE_IF_COPY(gs_a1, 0);
  PG_FREE_IF_COPY(gs_b1, 1);
  PG_FREE_IF_COPY(gs_a2, 2);
  PG_FREE_IF_COPY(gs_b2, 3);
  PG_FREE_IF_COPY(gs_poly, 4);

  if (n < 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Pure-translation precondition violated: "
             "(b2 - a2) must equal (b1 - a1).")));

  if (n == 0)
    PG_RETURN_NULL();

  SpanSet *result = spanset_make_free(intervals, n, NORMALIZE, ORDER_NO);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Trgeometry_geom_clip_polygon_posed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_geom_clip_polygon_posed);
/**
 * @brief Return the time-parameter intervals during which a moving
 * body's edge under interpolated 2D pose intersects a polygon.
 * @sqlfn _trgeometry_geom_clip_polygon_posed()
 */
Datum
Trgeometry_geom_clip_polygon_posed(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs_pa = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *gs_pb = PG_GETARG_GSERIALIZED_P(1);
  Pose *pose1 = PG_GETARG_POSE_P(2);
  Pose *pose2 = PG_GETARG_POSE_P(3);
  GSERIALIZED *gs_poly = PG_GETARG_GSERIALIZED_P(4);

  if (gserialized_get_type(gs_pa) != POINTTYPE ||
      gserialized_get_type(gs_pb) != POINTTYPE)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The first two arguments must be POINT geometries")));

  const POINT2D *p_a_local = GSERIALIZED_POINT2D_P(gs_pa);
  const POINT2D *p_b_local = GSERIALIZED_POINT2D_P(gs_pb);

  LWGEOM *geom = lwgeom_from_gserialized(gs_poly);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_lwgeom_posed(p_a_local, p_b_local, pose1, pose2,
    geom, &intervals);
  lwgeom_free(geom);

  PG_FREE_IF_COPY(gs_pa, 0);
  PG_FREE_IF_COPY(gs_pb, 1);
  PG_FREE_IF_COPY(gs_poly, 4);

  if (n < 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Pose input must be 2D (kernel is 2D only).")));

  if (n == 0)
    PG_RETURN_NULL();

  SpanSet *result = spanset_make_free(intervals, n, NORMALIZE, ORDER_NO);
  PG_RETURN_SPANSET_P(result);
}
