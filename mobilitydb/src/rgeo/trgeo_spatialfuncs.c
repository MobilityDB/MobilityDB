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
#include <float.h>
#include <funcapi.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "geo/stbox.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"
#include "temporal/temporal_analytics.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_traversed_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_traversed_area);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the area traversed by a temporal rigid geometry
 * @sqlfn traversedArea()
 */
Datum
Trgeometry_traversed_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool unary_union = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    unary_union = PG_GETARG_BOOL(1);
  GSERIALIZED *result = trgeometry_traversed_area(temp, unary_union);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
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


/*****************************************************************************
 * Centroid and convex hull functions
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_centroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_centroid);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the centroid of a temporal rigid geometry as a temporal point
 * @sqlfn centroid()
 */
Datum
Trgeometry_centroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeometry_centroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_convex_hull);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the convex hull of a temporal rigid geometry
 * @sqlfn convexHull()
 */
Datum
Trgeometry_convex_hull(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeometry_convex_hull(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Body-frame trajectory function
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_body_point_trajectory(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_body_point_trajectory);
/**
 * @ingroup mobilitydb_rgeo_spatialfuncs
 * @brief Return the world-frame trajectory of a body-frame point on a moving
 * rigid geometry
 * @sqlfn bodyPointTrajectory()
 */
Datum
Trgeometry_body_point_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = trgeometry_body_point_trajectory(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Similarity distance functions
 *****************************************************************************/

/**
 * @brief Generic similarity distance between two temporal rigid geometries
 */
static Datum
Trgeo_similarity_distance(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }
  Temporal *tp1 = trgeometry_to_tpoint(temp1);
  Temporal *tp2 = trgeometry_to_tpoint(temp2);
  double result = (simfunc == HAUSDORFF) ?
    temporal_hausdorff_distance(tp1, tp2) :
    temporal_similarity(tp1, tp2, simfunc);
  pfree(tp1); pfree(tp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Trgeometry_frechet_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_frechet_distance);
/**
 * @ingroup mobilitydb_rgeo_analytics_similarity
 * @brief Return the discrete Frechet distance between two temporal rigid
 * geometries
 * @sqlfn frechetDistance()
 */
Datum
Trgeometry_frechet_distance(PG_FUNCTION_ARGS)
{
  return Trgeo_similarity_distance(fcinfo, FRECHET);
}

PGDLLEXPORT Datum Trgeometry_dyntimewarp_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_dyntimewarp_distance);
/**
 * @ingroup mobilitydb_rgeo_analytics_similarity
 * @brief Return the Dynamic Time Warp distance between two temporal rigid
 * geometries
 * @sqlfn dynTimeWarpDistance()
 */
Datum
Trgeometry_dyntimewarp_distance(PG_FUNCTION_ARGS)
{
  return Trgeo_similarity_distance(fcinfo, DYNTIMEWARP);
}

PGDLLEXPORT Datum Trgeometry_hausdorff_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_hausdorff_distance);
/**
 * @ingroup mobilitydb_rgeo_analytics_similarity
 * @brief Return the Hausdorff distance between two temporal rigid geometries
 * @sqlfn hausdorffDistance()
 */
Datum
Trgeometry_hausdorff_distance(PG_FUNCTION_ARGS)
{
  return Trgeo_similarity_distance(fcinfo, HAUSDORFF);
}

/*****************************************************************************
 * Similarity path functions (set-returning)
 *****************************************************************************/

static SimilarityPathState *
trgeo_similarity_path_state_make(Match *path, int size)
{
  SimilarityPathState *state = palloc0(sizeof(SimilarityPathState));
  state->done = false;
  state->size = size;
  state->i = size - 1;
  state->path = path;
  return state;
}

static void
trgeo_similarity_path_state_next(SimilarityPathState *state)
{
  if (! state || state->done)
    return;
  state->i--;
  if (state->i < 0)
    state->done = true;
}

/**
 * @brief Return the similarity path between two temporal rigid geometries
 */
static Datum
Trgeo_similarity_path(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  FuncCallContext *funcctx;
  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
    Temporal *tp1 = trgeometry_to_tpoint(temp1);
    Temporal *tp2 = trgeometry_to_tpoint(temp2);
    int count;
    Match *path = temporal_similarity_path(tp1, tp2, &count, simfunc);
    pfree(tp1); pfree(tp2);
    funcctx->user_fctx = trgeo_similarity_path_state_make(path, count);
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
  }
  funcctx = SRF_PERCALL_SETUP();
  SimilarityPathState *state = funcctx->user_fctx;
  if (state->done)
  {
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state->path);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }
  Datum values[2];
  values[0] = Int32GetDatum(state->path[state->i].i);
  values[1] = Int32GetDatum(state->path[state->i].j);
  trgeo_similarity_path_state_next(state);
  bool isnull[2] = {0, 0};
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Trgeometry_frechet_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_frechet_path);
/**
 * @ingroup mobilitydb_rgeo_analytics_similarity
 * @brief Return the discrete Frechet path between two temporal rigid
 * geometries
 * @sqlfn frechetDistancePath()
 */
Datum
Trgeometry_frechet_path(PG_FUNCTION_ARGS)
{
  return Trgeo_similarity_path(fcinfo, FRECHET);
}

PGDLLEXPORT Datum Trgeometry_dyntimewarp_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_dyntimewarp_path);
/**
 * @ingroup mobilitydb_rgeo_analytics_similarity
 * @brief Return the Dynamic Time Warp path between two temporal rigid
 * geometries
 * @sqlfn dynTimeWarpPath()
 */
Datum
Trgeometry_dyntimewarp_path(PG_FUNCTION_ARGS)
{
  return Trgeo_similarity_path(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************
 * Motion metrics
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_length);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the length traversed by the centroid of a temporal rigid
 * geometry
 * @sqlfn length()
 */
Datum
Trgeometry_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = trgeometry_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Trgeometry_cumulative_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_cumulative_length);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the cumulative length traversed by the centroid of a temporal
 * rigid geometry
 * @sqlfn cumulativeLength()
 */
Datum
Trgeometry_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeometry_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_speed);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the speed of the centroid of a temporal rigid geometry
 * @sqlfn speed()
 */
Datum
Trgeometry_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = trgeometry_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_twcentroid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_twcentroid);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return the time-weighted centroid of the centroid trajectory of a
 * temporal rigid geometry
 * @sqlfn twCentroid()
 */
Datum
Trgeometry_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeometry_twcentroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
