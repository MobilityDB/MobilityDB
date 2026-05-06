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
 * @brief Spatial functions for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "geo/stbox.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeo_traversed_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_traversed_area);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the area traversed by a temporal rigid geometry
 * @sqlfn traversedArea()
 */
Datum
Trgeo_traversed_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool unary_union = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    unary_union = PG_GETARG_BOOL(1);
  GSERIALIZED *result = trgeo_traversed_area(temp, unary_union);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeo_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_convex_hull);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the convex hull of a temporal rigid geometry
 * @sqlfn convexHull()
 */
Datum
Trgeo_convex_hull(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeo_convex_hull(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeo_centroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_centroid);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the centroid trajectory of a temporal rigid geometry as a
 * temporal point
 * @sqlfn centroid()
 */
Datum
Trgeo_centroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeo_centroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Return a temporal rigid geometry restricted to (the complement of) a
 * geometry
 */
static Datum
Trgeo_restrict_geom(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = trgeo_restrict_geom(temp, gs, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeo_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_at_geom);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a geometry
 * @sqlfn atGeometry()
 */
inline Datum
Trgeo_at_geom(PG_FUNCTION_ARGS)
{
  return Trgeo_restrict_geom(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Trgeo_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_minus_geom);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * geometry
 * @sqlfn minusGeometry()
 */
inline Datum
Trgeo_minus_geom(PG_FUNCTION_ARGS)
{
  return Trgeo_restrict_geom(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 */
static Datum
Trgeo_restrict_stbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = trgeo_restrict_stbox(temp, box, border_inc, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeo_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_at_stbox);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 * @sqlfn atStbox()
 */
inline Datum
Trgeo_at_stbox(PG_FUNCTION_ARGS)
{
  return Trgeo_restrict_stbox(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Trgeo_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_minus_stbox);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * spatiotemporal box
 * @sqlfn minusStbox()
 */
inline Datum
Trgeo_minus_stbox(PG_FUNCTION_ARGS)
{
  return Trgeo_restrict_stbox(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Trgeo_expand_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_expand_space);
/**
 * @ingroup mobilitydb_rgeo_boxops
 * @brief Return the bounding box of a temporal rigid geometry expanded in
 * space by a distance
 * @sqlfn expandSpace()
 */
Datum
Trgeo_expand_space(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double d = PG_GETARG_FLOAT8(1);
  STBox *result = trgeo_expand_space(temp, d);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Trgeo_simplify_min_tdelta(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_simplify_min_tdelta);
/**
 * @ingroup mobilitydb_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a minimum time delta
 * @sqlfn minTimeDeltaSimplify()
 */
Datum
Trgeo_simplify_min_tdelta(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *mint = PG_GETARG_INTERVAL_P(1);
  Temporal *result = trgeo_simplify_min_tdelta(temp, mint);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeo_simplify_min_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_simplify_min_dist);
/**
 * @ingroup mobilitydb_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a minimum distance
 * @sqlfn minDistSimplify()
 */
Datum
Trgeo_simplify_min_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  Temporal *result = trgeo_simplify_min_dist(temp, dist);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeo_simplify_max_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_simplify_max_dist);
/**
 * @ingroup mobilitydb_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a maximum distance
 * @sqlfn maxDistSimplify()
 */
Datum
Trgeo_simplify_max_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = trgeo_simplify_max_dist(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeo_simplify_dp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_simplify_dp);
/**
 * @ingroup mobilitydb_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using the Douglas-Peucker
 * algorithm on the centroid trajectory
 * @sqlfn douglasPeuckerSimplify()
 */
Datum
Trgeo_simplify_dp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = trgeo_simplify_dp(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
