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
 * @brief Functions for spatial and spatiotemporal tiles for temporal rigid
 * geometries
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/array.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_tile.h"
#include "rgeo/trgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************/

/**
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a grid.
 *
 * Mirrors tgeo_space_time_boxes() but restricts each tile using
 * trgeo_restrict_stbox() instead of tgeo_restrict_stbox().
 */
static STBox *
trgeo_space_time_boxes_impl(const Temporal *temp, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin,
  TimestampTz torigin, bool bitmatrix, bool border_inc, int *count)
{
  /* Bitmatrix optimization is only supported for tpoint, not trgeometry */
  bitmatrix = false;
  int ntiles;
  STboxGridState *state = tgeo_space_time_tile_init(temp, xsize, ysize,
    zsize, duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
  if (! state)
    return NULL;

  int i = 0;
  STBox *result = palloc(sizeof(STBox) * ntiles);
  while (true)
  {
    if (state->done)
    {
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      break;
    }

    STBox box;
    bool found = stbox_tile_state_get(state, &box);
    if (! found)
    {
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      break;
    }
    stbox_tile_state_next(state);

    Temporal *atstbox = trgeo_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (! atstbox)
      continue;
    tspatial_set_stbox(atstbox, &box);
    pfree(atstbox);

    memcpy(&result[i++], &box, sizeof(STBox));
  }
  *count = i;
  return result;
}

/**
 * @brief Return the spatial or spatiotemporal boxes of a temporal rigid
 * geometry split with respect to a grid (internal)
 */
static Datum
Trgeo_space_time_boxes_common(FunctionCallInfo fcinfo, bool spacetiles,
  bool timetiles)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int i = 1;
  double xsize = 0, ysize = 0, zsize = 0;
  if (spacetiles)
  {
    xsize = PG_GETARG_FLOAT8(i++);
    ysize = PG_GETARG_FLOAT8(i++);
    zsize = PG_GETARG_FLOAT8(i++);
  }
  Interval *duration = timetiles ? PG_GETARG_INTERVAL_P(i++) : NULL;
  GSERIALIZED *sorigin = spacetiles ? PG_GETARG_GSERIALIZED_P(i++) : NULL;
  TimestampTz torigin = timetiles ? PG_GETARG_TIMESTAMPTZ(i++) : 0;
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);

  if (temporal_num_instants(temp) == 1)
    bitmatrix = false;
  int count;
  STBox *boxes = trgeo_space_time_boxes_impl(temp, xsize, ysize, zsize,
    duration, sorigin, torigin, bitmatrix, border_inc, &count);
  if (! boxes)
    PG_RETURN_NULL();
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Trgeo_space_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_space_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatial grid
 * @sqlfn spaceBoxes()
 */
inline Datum
Trgeo_space_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Trgeo_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to time bins
 * @sqlfn timeBoxes()
 */
inline Datum
Trgeo_time_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Trgeo_space_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_space_time_boxes);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return the spatiotemporal boxes of a temporal rigid geometry split
 * with respect to a spatiotemporal grid
 * @sqlfn spaceTimeBoxes()
 */
inline Datum
Trgeo_space_time_boxes(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_boxes_common(fcinfo, true, true);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Return a temporal rigid geometry split with respect to a spatial or
 * spatiotemporal grid (internal)
 */
static Datum
Trgeo_space_time_split_common(FunctionCallInfo fcinfo, bool timesplit)
{
  FuncCallContext *funcctx;

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = PG_GETARG_FLOAT8(2);
    double zsize = PG_GETARG_FLOAT8(3);
    int i = 4;
    Interval *duration = timesplit ? PG_GETARG_INTERVAL_P(i++) : NULL;
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    TimestampTz torigin = timesplit ? PG_GETARG_TIMESTAMPTZ(i++) : 0;
    bool bitmatrix = PG_GETARG_BOOL(i++);
    bool border_inc = PG_GETARG_BOOL(i++);
    /* Bitmatrix optimization is only supported for tpoint, not trgeometry */
    bitmatrix = false;

    int ntiles;
    STboxGridState *state = tgeo_space_time_tile_init(temp, xsize, ysize,
      zsize, duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
    assert(state);

    funcctx->user_fctx = state;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  funcctx = SRF_PERCALL_SETUP();
  STboxGridState *state = funcctx->user_fctx;
  bool isnull[3] = {0, 0, 0};
  while (true)
  {
    if (state->done)
    {
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }

    STBox box;
    bool found = stbox_tile_state_get(state, &box);
    if (! found)
    {
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }
    stbox_tile_state_next(state);

    Temporal *atstbox = trgeo_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (! atstbox)
      continue;

    bool hasz = MEOS_FLAGS_GET_Z(state->temp->flags);
    Datum values[3];
    int i = 0;
    values[i++] = PointerGetDatum(geopoint_make(box.xmin, box.ymin,
      box.zmin, hasz, false, box.srid));
    if (timesplit)
      values[i++] = box.period.lower;
    values[i++] = PointerGetDatum(atstbox);
    HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
    Datum result = HeapTupleGetDatum(tuple);
    SRF_RETURN_NEXT(funcctx, result);
  }
}

PGDLLEXPORT Datum Trgeo_space_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_space_split);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return a temporal rigid geometry split with respect to a spatial grid
 * @sqlfn spaceSplit()
 */
inline Datum
Trgeo_space_split(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_split_common(fcinfo, false);
}

PGDLLEXPORT Datum Trgeo_space_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeo_space_time_split);
/**
 * @ingroup mobilitydb_rgeo_tile
 * @brief Return a temporal rigid geometry split with respect to a
 * spatiotemporal grid
 * @sqlfn spaceTimeSplit()
 */
inline Datum
Trgeo_space_time_split(PG_FUNCTION_ARGS)
{
  return Trgeo_space_time_split_common(fcinfo, true);
}

/*****************************************************************************/
