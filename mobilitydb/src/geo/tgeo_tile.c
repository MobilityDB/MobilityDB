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
 * @brief Functions for spatial and spatiotemporal tiles
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
#include "general/temporal_tile.h"
#include "general/temporal_tile.h"
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tpoint_tile.h"
/* MobilityDB */
#include "pg_general/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************/

/**
 * @brief Return the spatial, temporal, or spatiotemporal grid of a
 * spatiotemporal box (external function)
 */
static Datum
Stbox_space_time_tiles_ext(FunctionCallInfo fcinfo, bool spacetiles,
  bool timetiles)
{
  assert(spacetiles || timetiles);

  FuncCallContext *funcctx;
  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize to 0 missing parameters */
    double xsize = 0, ysize = 0, zsize = 0;
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    bool border_inc = false;
    POINT3DZ pt;
    int i = 1;
    /* Get input parameters */
    STBox *bounds = PG_GETARG_STBOX_P(0);
    if (spacetiles)
    {
      ensure_has_X(T_STBOX, bounds->flags);
      ensure_not_geodetic(bounds->flags);
      xsize = PG_GETARG_FLOAT8(i++);
      ysize = PG_GETARG_FLOAT8(i++);
      zsize = PG_GETARG_FLOAT8(i++);
      ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
      ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8);
      ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8);
    }
    if (timetiles)
    {
      /* If time arguments are given */
      ensure_has_T(T_STBOX, bounds->flags);
      duration = PG_GETARG_INTERVAL_P(i++);
      ensure_valid_duration(duration);
    }
    if (spacetiles)
    {
      GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
      ensure_not_empty(sorigin);
      ensure_point_type(sorigin);
      /* Since we pass by default Point(0 0 0) as origin independently of the
       * input STBox, we test the same spatial dimensionality only for STBox Z.
       * Also, since when zsize is not given we pass by default xsize, if we
       * don't have an STBox Z we set zsize to 0 */
      if (MEOS_FLAGS_GET_Z(bounds->flags))
        ensure_same_spatial_dimensionality_stbox_geo(bounds, sorigin);
      else
        zsize = 0;
      int32_t srid = bounds->srid;
      int32_t gs_srid = gserialized_get_srid(sorigin);
      if (gs_srid != SRID_UNKNOWN)
        ensure_same_srid(srid, gs_srid);
      memset(&pt, 0, sizeof(POINT3DZ));
      if (FLAGS_GET_Z(sorigin->gflags))
      {
        const POINT3DZ *p3d = GSERIALIZED_POINT3DZ_P(sorigin);
        pt.x = p3d->x;
        pt.y = p3d->y;
        pt.z = p3d->z;
      }
      else
      {
        /* Initialize to 0 the Z dimension if it is missing */
        memset(&pt, 0, sizeof(POINT3DZ));
        const POINT2D *p2d = GSERIALIZED_POINT2D_P(sorigin);
        pt.x = p2d->x;
        pt.y = p2d->y;
        /* Since when zsize is not given we pass by default xsize, if the box does
         * not have Z dimension we set zsize to 0 */
        zsize = 0;
      }
    }
    if (timetiles)
    {
      torigin = PG_GETARG_TIMESTAMPTZ(i++);
    }
    border_inc = PG_GETARG_BOOL(i++);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = stbox_tile_state_make(NULL, bounds, xsize, ysize,
      zsize, duration, pt, torigin, border_inc);
    /* Build a tuple description for a multidim_grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  STboxGridState *state = funcctx->user_fctx;
  /* Stop when we've used up all the grid tiles */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Allocate box */
  STBox *box = palloc(sizeof(STBox));
  /* Get current tile and advance state
   * There is no need to test if the tile is found since all tiles should be
   * generated and thus there is no associated bit matrix */
  stbox_tile_state_get(state, box);
  stbox_tile_state_next(state);
  /* Form tuple and return
   * The i value was incremented with the previous _next function call */
  Datum values[2]; /* used to construct the composite return value */
  values[0] = Int32GetDatum(state->i - 1);
  values[1] = PointerGetDatum(box);
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PGDLLEXPORT Datum Stbox_space_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_space_tiles);
/**
 * @brief @ingroup mobilitydb_geo_tile
 * @brief Return the spatial grid of a spatiotemporal box
 * @sqlfn spaceTiles()
 */
inline Datum
Stbox_space_tiles(PG_FUNCTION_ARGS)
{
  return Stbox_space_time_tiles_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Stbox_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_time_tiles);
/**
 * @brief @ingroup mobilitydb_geo_tile
 * @brief Return the temporal grid of a spatiotemporal box
 * @sqlfn timeTiles()
 */
inline Datum
Stbox_time_tiles(PG_FUNCTION_ARGS)
{
  return Stbox_space_time_tiles_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Stbox_space_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_space_time_tiles);
/**
 * @brief @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal grid of a spatiotemporal box
 * @sqlfn spaceTimeTiles()
 */
inline Datum
Stbox_space_time_tiles(PG_FUNCTION_ARGS)
{
  return Stbox_space_time_tiles_ext(fcinfo, true, true);
}

/*****************************************************************************/

/**
 * @brief Return a tile in the spatiotemporal grid of a spatiotemporal box 
 * (external function)
 */
static Datum
Stbox_get_space_time_tile_ext(FunctionCallInfo fcinfo, bool spacetile,
  bool timetile)
{
  assert(spacetile || timetile);

  /* Initialize to 0 missing parameters */
  GSERIALIZED *point = NULL;
  double xsize = 0, ysize = 0, zsize = 0;
  GSERIALIZED *sorigin = NULL;
  TimestampTz t = 0, torigin = 0; /* make compiler quiet */
  Interval *duration = NULL; /* make compiler quiet */
  bool hasx = false, hast = false;
  int i = 0;
  if (spacetile)
  {
    point = PG_GETARG_GSERIALIZED_P(i++);
    hasx = true;
  }
  if (timetile)
  {
    t = PG_GETARG_TIMESTAMPTZ(i++);
    hast = true;
  }
  if (spacetile)
  {
    xsize = PG_GETARG_FLOAT8(i++);
    ysize = PG_GETARG_FLOAT8(i++);
    zsize = PG_GETARG_FLOAT8(i++);
  }
  if (timetile)
  {
    /* If time arguments are given */
    duration = PG_GETARG_INTERVAL_P(i++);
  }
  if (spacetile)
  {
    sorigin = PG_GETARG_GSERIALIZED_P(i++);
  }
  if (timetile)
  {
    torigin = PG_GETARG_TIMESTAMPTZ(i++);
  }
  PG_RETURN_STBOX_P(stbox_space_time_tile(point, t, xsize, ysize, zsize,
    duration, sorigin, torigin, hasx, hast));
}

PGDLLEXPORT Datum Stbox_get_space_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space_tile);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the spatial grid of a spatiotemporal box
 * @sqlfn getSpaceTile()
 */
inline Datum
Stbox_get_space_tile(PG_FUNCTION_ARGS)
{
  return Stbox_get_space_time_tile_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Stbox_get_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_time_tile);
/**
 * @brief @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the temporal grid of a spatiotemporal box
 * @sqlfn getTimeTile()
 */
inline Datum
Stbox_get_time_tile(PG_FUNCTION_ARGS)
{
  return Stbox_get_space_time_tile_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Stbox_get_space_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space_time_tile);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a tile in the spatiotemporal grid of a spatiotemporal box
 * @sqlfn getSpaceTimeTile()
 */
inline Datum
Stbox_get_space_time_tile(PG_FUNCTION_ARGS)
{
  return Stbox_get_space_time_tile_ext(fcinfo, true, true);
}

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Compute the spatiotemporal boxes of a temporal geo split with
 * respect to a spatial or spatiotemporal grid
 */
static Datum
Tgeo_space_time_boxes_ext(FunctionCallInfo fcinfo, bool spacetiles,
  bool timetiles)
{
  /* Get input parameters */
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
  TimestampTz torigin = timetiles ? torigin = PG_GETARG_TIMESTAMPTZ(i++) : 0;
  bool bitmatrix = PG_GETARG_BOOL(i++);
  bool border_inc = PG_GETARG_BOOL(i++);

  /* Get the tiles */
  if (temporal_num_instants(temp) == 1)
    bitmatrix = false;
  int count;
  STBox *boxes = tgeo_space_time_boxes(temp, xsize, ysize, zsize, duration, 
    sorigin, torigin, bitmatrix, border_inc, &count);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_FREE_IF_COPY(temp, 0);
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
inline Datum
Tgeo_space_boxes(PG_FUNCTION_ARGS)
{
  return Tgeo_space_time_boxes_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tgeo_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_time_boxes);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal boxes of a temporal geo split with respect
 * to time bins
 * @sqlfn timeBoxes()
 */
inline Datum
Tgeo_time_boxes(PG_FUNCTION_ARGS)
{
  return Tgeo_space_time_boxes_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tgeo_space_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_time_boxes);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return the spatiotemporal boxes of a temporal geo split with
 * respect to a spatiotemporal grid
 * @sqlfn spaceTimeBoxes()
 */
inline Datum
Tgeo_space_time_boxes(PG_FUNCTION_ARGS)
{
  return Tgeo_space_time_boxes_ext(fcinfo, true, true);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Return a temporal geo split with respect to a spatial or a 
 * spatiotemporal grid
 */
static Datum
Tgeo_space_time_split_ext(FunctionCallInfo fcinfo, bool timesplit)
{
  FuncCallContext *funcctx;

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Get input parameters */
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

    /* Initialize state and verify parameter validity */
    int ntiles;
    STboxGridState *state = tgeo_space_time_tile_init(temp, xsize, ysize,
      zsize, duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
    assert(state);

    /* Create function state */
    funcctx->user_fctx = state;

    /* Build a tuple description for a multidimensional grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  STboxGridState *state = funcctx->user_fctx;
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */
  /* We need to loop since atStbox may be NULL */
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      if (state->bm)
         pfree(state->bm);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
    STBox box;
    bool found = stbox_tile_state_get(state, &box);
    if (! found)
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }
    stbox_tile_state_next(state);

    /* Restrict the temporal point to the box */
    Temporal *atstbox = tgeo_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (atstbox == NULL)
      continue;

    /* Form tuple and return */
    bool hasz = MEOS_FLAGS_GET_Z(state->temp->flags);
    Datum values[3]; /* used to construct the composite return value */
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

PGDLLEXPORT Datum Tgeo_space_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_split);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a temporal geo split with respect to a spatial grid
 * @sqlfn spaceSplit()
 */
inline Datum
Tgeo_space_split(PG_FUNCTION_ARGS)
{
  return Tgeo_space_time_split_ext(fcinfo, false);
}

PGDLLEXPORT Datum Tgeo_space_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_space_time_split);
/**
 * @ingroup mobilitydb_geo_tile
 * @brief Return a temporal geo split with respect to a spatiotemporal grid
 * @sqlfn spaceTimeSplit()
 */
inline Datum
Tgeo_space_time_split(PG_FUNCTION_ARGS)
{
  return Tgeo_space_time_split_ext(fcinfo, true);
}

/*****************************************************************************/
