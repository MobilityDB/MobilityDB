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
 * @brief Functions for spatial and spatiotemporal tiles
 * @details A function returning a value reads its arguments and calls the
 * MEOS function of the same grid, which validates them and answers the boxes
 * or the tile. A function returning rows lays the grid through the MEOS
 * function laying it for the MEOS function of the same grid
 * (#stbox_space_time_tile_init, #tgeo_space_time_split_init), which validates
 * the arguments, and returns one row per call as it reads the state, so a grid
 * of any size is returned in the memory of one tile. The forms
 * of a function stating one or two spatial sizes leave out the ones that
 * follow, which MEOS reads as `xsize`: PostgreSQL passes a function every
 * argument its declaration states, so the number of arguments tells the
 * forms apart
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <utils/array.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/temporal_tile.h"
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_tile.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Tile functions
 *****************************************************************************/

/**
 * @brief Lay the grid of a spatiotemporal box for the calls returning its
 * tiles
 * @details #stbox_space_time_tile_init validates the arguments and lays the
 * state #stbox_space_time_tiles reads as well
 */
static void
Stbox_tiles_start(FunctionCallInfo fcinfo, FuncCallContext *funcctx,
  const STBox *bounds, double xsize, double ysize, double zsize,
  const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin,
  bool border_inc)
{
  int ntiles;
  funcctx->user_fctx = stbox_space_time_tile_init(bounds, xsize, ysize, zsize,
    duration, sorigin, torigin, border_inc, &ntiles);
  get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
  BlessTupleDesc(funcctx->tuple_desc);
  return;
}

/**
 * @brief Return the next tile of a grid, as a row `(index, tile)` numbering
 * the tiles from 1
 */
static Datum
Stbox_tiles_next(FunctionCallInfo fcinfo)
{
  FuncCallContext *funcctx = SRF_PERCALL_SETUP();
  STboxGridState *state = funcctx->user_fctx;
  /* Stop when we've used up all the grid tiles */
  if (state->done)
  {
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }
  /* Get the current tile and advance the state. Every tile is generated,
   * since no bit matrix selects them, so a tile is always found */
  STBox *box = palloc(sizeof(STBox));
  stbox_tile_state_get(state, box);
  stbox_tile_state_next(state);
  /* The index was advanced by the call to the next function */
  Datum values[2];
  values[0] = Int32GetDatum(state->i - 1);
  values[1] = PointerGetDatum(box);
  bool isnull[2] = {0, 0};
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Stbox_space_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_space_tiles);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatial grid of a spatiotemporal box
 * @sqlfn spaceTiles()
 */
Datum
Stbox_space_tiles(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    STBox *bounds = PG_GETARG_STBOX_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = 0;
    double zsize = 0;
    int i = 2;
    if (PG_NARGS() > 4)
      ysize = PG_GETARG_FLOAT8(i++);
    if (PG_NARGS() > 5)
      zsize = PG_GETARG_FLOAT8(i++);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    bool border_inc = PG_GETARG_BOOL(i++);
    Stbox_tiles_start(fcinfo, funcctx, bounds, xsize, ysize, zsize, NULL,
      sorigin, 0, border_inc);
    MemoryContextSwitchTo(oldcontext);
  }
  return Stbox_tiles_next(fcinfo);
}

PGDLLEXPORT Datum Stbox_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_time_tiles);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the temporal grid of a spatiotemporal box
 * @sqlfn timeTiles()
 */
Datum
Stbox_time_tiles(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    STBox *bounds = PG_GETARG_STBOX_P(0);
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
    bool border_inc = PG_GETARG_BOOL(3);
    Stbox_tiles_start(fcinfo, funcctx, bounds, 0.0, 0.0, 0.0, duration, NULL,
      torigin, border_inc);
    MemoryContextSwitchTo(oldcontext);
  }
  return Stbox_tiles_next(fcinfo);
}

PGDLLEXPORT Datum Stbox_space_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_space_time_tiles);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal grid of a spatiotemporal box
 * @sqlfn spaceTimeTiles()
 */
Datum
Stbox_space_time_tiles(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    STBox *bounds = PG_GETARG_STBOX_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = 0;
    double zsize = 0;
    int i = 2;
    if (PG_NARGS() > 6)
      ysize = PG_GETARG_FLOAT8(i++);
    if (PG_NARGS() > 7)
      zsize = PG_GETARG_FLOAT8(i++);
    Interval *duration = PG_GETARG_INTERVAL_P(i++);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(i++);
    bool border_inc = PG_GETARG_BOOL(i++);
    Stbox_tiles_start(fcinfo, funcctx, bounds, xsize, ysize, zsize, duration,
      sorigin, torigin, border_inc);
    MemoryContextSwitchTo(oldcontext);
  }
  return Stbox_tiles_next(fcinfo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_get_space_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space_tile);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the spatial grid of a spatiotemporal box
 * @sqlfn getSpaceTile()
 */
Datum
Stbox_get_space_tile(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  double xsize = PG_GETARG_FLOAT8(1);
  double ysize = 0;
  double zsize = 0;
  int i = 2;
  if (PG_NARGS() > 3)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 4)
    zsize = PG_GETARG_FLOAT8(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  STBox *result = stbox_get_space_tile(point, xsize, ysize, zsize, sorigin);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_get_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_time_tile);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the temporal grid of a spatiotemporal box
 * @sqlfn getStboxTimeTile()
 */
Datum
Stbox_get_time_tile(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_STBOX_P(stbox_get_time_tile(t, duration, torigin));
}

PGDLLEXPORT Datum Stbox_get_space_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space_time_tile);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the spatiotemporal grid of a spatiotemporal box
 * @sqlfn getSpaceTimeTile()
 */
Datum
Stbox_get_space_time_tile(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double xsize = PG_GETARG_FLOAT8(2);
  double ysize = 0;
  double zsize = 0;
  int i = 3;
  if (PG_NARGS() > 6)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 7)
    zsize = PG_GETARG_FLOAT8(i++);
  Interval *duration = PG_GETARG_INTERVAL_P(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(i++);
  STBox *result = stbox_get_space_time_tile(point, t, xsize, ysize, zsize,
    duration, sorigin, torigin);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Return the boxes a grid function answered as an array
 */
static Datum
Stbox_boxes_array(STBox *boxes, int count)
{
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tgeo_space_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_boxes);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal boxes of a temporal geo split with respect
 * to a spatial grid
 * @sqlfn spaceBoxes()
 */
Datum
Tgeo_space_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double xsize = PG_GETARG_FLOAT8(1);
  double ysize = 0;
  double zsize = 0;
  int i = 2;
  if (PG_NARGS() > 5)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 6)
    zsize = PG_GETARG_FLOAT8(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);
  int count;
  STBox *boxes = tgeo_space_boxes(temp, xsize, ysize, zsize, sorigin,
    bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Stbox_boxes_array(boxes, count);
}

PGDLLEXPORT Datum Tgeo_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_time_boxes);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal boxes of a temporal geo split with respect
 * to time bins
 * @sqlfn timeBoxes()
 */
Datum
Tgeo_time_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  bool bitmatrix = PG_GETARG_BOOL(3);
  bool border_inc = PG_GETARG_BOOL(4);
  int count;
  STBox *boxes = tgeo_space_time_boxes(temp, 0.0, 0.0, 0.0, duration, NULL,
    torigin, bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Stbox_boxes_array(boxes, count);
}

PGDLLEXPORT Datum Tgeo_space_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_time_boxes);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal boxes of a temporal geo split with
 * respect to a spatiotemporal grid
 * @sqlfn spaceTimeBoxes()
 */
Datum
Tgeo_space_time_boxes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double xsize = PG_GETARG_FLOAT8(1);
  double ysize = 0;
  double zsize = 0;
  int i = 2;
  if (PG_NARGS() > 7)
    ysize = PG_GETARG_FLOAT8(i++);
  if (PG_NARGS() > 8)
    zsize = PG_GETARG_FLOAT8(i++);
  Interval *duration = PG_GETARG_INTERVAL_P(i++);
  GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(i++);
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);
  int count;
  STBox *boxes = tgeo_space_time_boxes(temp, xsize, ysize, zsize, duration,
    sorigin, torigin, bitmatrix, border_inc, &count);
  PG_FREE_IF_COPY(temp, 0);
  return Stbox_boxes_array(boxes, count);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Lay the grid of a temporal geo for the calls returning its
 * fragments
 * @details #tgeo_space_time_split_init validates the arguments and lays the
 * state #tgeo_space_time_split reads as well
 */
static void
Tgeo_split_start(FunctionCallInfo fcinfo, FuncCallContext *funcctx,
  const Temporal *temp, double xsize, double ysize, double zsize,
  const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin,
  bool bitmatrix, bool border_inc)
{
  int ntiles;
  funcctx->user_fctx = tgeo_space_time_split_init(temp, xsize, ysize, zsize,
    duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
  get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
  BlessTupleDesc(funcctx->tuple_desc);
  return;
}

/**
 * @brief Return the next fragment of a split, as a row `(point, fragment)`
 * or `(point, time, fragment)`
 */
static Datum
Tgeo_split_next(FunctionCallInfo fcinfo)
{
  FuncCallContext *funcctx = SRF_PERCALL_SETUP();
  STboxGridState *state = funcctx->user_fctx;
  bool timesplit = MEOS_FLAGS_GET_T(state->box.flags);
  bool hasz = MEOS_FLAGS_GET_Z(state->temp->flags);
  /* Loop since the restriction to a tile may be empty */
  while (true)
  {
    /* Stop when we have used up all the grid tiles. A tile may be missing
     * when the previous one was the last set in the bit matrix */
    STBox box;
    if (state->done || ! stbox_tile_state_get(state, &box))
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
    /* Restrict the value to the tile */
    Temporal *atstbox = tgeo_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (! atstbox)
      continue;
    Datum values[3];
    int i = 0;
    values[i++] = PointerGetDatum(geopoint_make(box.xmin, box.ymin, box.zmin,
      hasz, false, box.srid));
    if (timesplit)
      values[i++] = box.period.lower;
    values[i++] = PointerGetDatum(atstbox);
    bool isnull[3] = {0, 0, 0};
    HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
    SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
  }
}

PGDLLEXPORT Datum Tgeo_space_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_split);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a temporal geo split with respect to a spatial grid
 * @sqlfn spaceSplit()
 */
Datum
Tgeo_space_split(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = 0;
    double zsize = 0;
    int i = 2;
    if (PG_NARGS() > 5)
      ysize = PG_GETARG_FLOAT8(i++);
    if (PG_NARGS() > 6)
      zsize = PG_GETARG_FLOAT8(i++);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    bool bitmatrix = PG_GETARG_BOOL(i++);
    bool border_inc = PG_GETARG_BOOL(i++);
    Tgeo_split_start(fcinfo, funcctx, temp, xsize, ysize, zsize, NULL,
      sorigin, 0, bitmatrix, border_inc);
    MemoryContextSwitchTo(oldcontext);
  }
  return Tgeo_split_next(fcinfo);
}

PGDLLEXPORT Datum Tgeo_space_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_time_split);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a temporal geo split with respect to a spatiotemporal grid
 * @sqlfn spaceTimeSplit()
 */
Datum
Tgeo_space_time_split(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = 0;
    double zsize = 0;
    int i = 2;
    if (PG_NARGS() > 7)
      ysize = PG_GETARG_FLOAT8(i++);
    if (PG_NARGS() > 8)
      zsize = PG_GETARG_FLOAT8(i++);
    Interval *duration = PG_GETARG_INTERVAL_P(i++);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(i++);
    bool bitmatrix = PG_GETARG_BOOL(i++);
    bool border_inc = PG_GETARG_BOOL(i++);
    Tgeo_split_start(fcinfo, funcctx, temp, xsize, ysize, zsize, duration,
      sorigin, torigin, bitmatrix, border_inc);
    MemoryContextSwitchTo(oldcontext);
  }
  return Tgeo_split_next(fcinfo);
}

/*****************************************************************************/
