/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Functions for spatial and spatiotemporal tiles.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/temporal_tile.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tile.h"
/* MobilityDB */
#include "pg_point/postgis.h"

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_tile_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tile_list);
/**
 * @brief @ingroup mobilitydb_temporal_tile
 * @brief Generate a multidimensional grid for temporal points.
 * @sqlfunc multidimGrid()
 */
Datum
Stbox_tile_list(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  STboxGridState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    STBox *bounds = PG_GETARG_STBOX_P(0);
    ensure_has_X_stbox(bounds);
    ensure_not_geodetic(bounds->flags);
    double xsize = PG_GETARG_FLOAT8(1);
    double ysize = PG_GETARG_FLOAT8(2);
    double zsize = PG_GETARG_FLOAT8(3);
    ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8);
    ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8);
    GSERIALIZED *sorigin;
    int64 tunits = 0; /* make compiler quiet */
    TimestampTz torigin = 0; /* make compiler quiet */
    if (PG_NARGS() == 5)
      sorigin = PG_GETARG_GSERIALIZED_P(4);
    else /* PG_NARGS() == 7 */
    {
      /* If time arguments are given */
      ensure_has_T_stbox(bounds);
      Interval *duration = PG_GETARG_INTERVAL_P(4);
      ensure_valid_duration(duration);
      tunits = interval_units(duration);
      sorigin = PG_GETARG_GSERIALIZED_P(5);
      torigin = PG_GETARG_TIMESTAMPTZ(6);
    }
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    /* Since we pass by default Point(0 0 0) as origin independently of the input
     * STBox, we test the same spatial dimensionality only for STBox Z */
    if (MEOS_FLAGS_GET_Z(bounds->flags))
      ensure_same_spatial_dimensionality_stbox_gs(bounds, sorigin);
    int32 srid = bounds->srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != SRID_UNKNOWN)
      ensure_same_srid(srid, gs_srid);
    POINT3DZ pt;
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
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = stbox_tile_state_make(NULL, bounds, xsize, ysize,
      zsize, tunits, pt, torigin);
    /* Build a tuple description for a multidim_grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
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
  tuple_arr[0] = Int32GetDatum(state->i - 1);
  tuple_arr[1] = PointerGetDatum(box);
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PGDLLEXPORT Datum Stbox_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tile);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Generate a tile in a multidimensional grid for temporal points.
 * @sqlfunc multidimTile()
 */
Datum
Stbox_tile(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  ensure_non_empty(point);
  ensure_point_type(point);
  TimestampTz t = 0; /* make compiler quiet */
  double xsize, ysize, zsize;
  int64 tunits = 0; /* make compiler quiet */
  GSERIALIZED *sorigin;
  TimestampTz torigin = 0; /* make compiler quiet */
  bool hast = false;
  if (PG_NARGS() == 5)
  {
    xsize = PG_GETARG_FLOAT8(1);
    ysize = PG_GETARG_FLOAT8(2);
    zsize = PG_GETARG_FLOAT8(3);
    sorigin = PG_GETARG_GSERIALIZED_P(4);
  }
  else /* PG_NARGS() == 8 */
  {
    /* If time arguments are given */
    t = PG_GETARG_TIMESTAMPTZ(1);
    xsize = PG_GETARG_FLOAT8(2);
    ysize = PG_GETARG_FLOAT8(3);
    zsize = PG_GETARG_FLOAT8(4);
    Interval *duration = PG_GETARG_INTERVAL_P(5);
    ensure_valid_duration(duration);
    tunits = interval_units(duration);
    sorigin = PG_GETARG_GSERIALIZED_P(6);
    torigin = PG_GETARG_TIMESTAMPTZ(7);
    hast = true;
  }
  /* Ensure parameter validity */
  ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
  ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8);
  ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8);
  ensure_non_empty(sorigin);
  ensure_point_type(sorigin);
  int32 srid = gserialized_get_srid(point);
  int32 gs_srid = gserialized_get_srid(sorigin);
  if (gs_srid != SRID_UNKNOWN)
    ensure_same_srid(srid, gs_srid);
  POINT3DZ pt, ptorig;
  memset(&pt, 0, sizeof(POINT3DZ));
  memset(&ptorig, 0, sizeof(POINT3DZ));
  bool hasz = (bool) FLAGS_GET_Z(point->gflags);
  if (hasz)
  {
    ensure_has_Z_gs(sorigin);
    const POINT3DZ *p1 = GSERIALIZED_POINT3DZ_P(point);
    pt.x = p1->x;
    pt.y = p1->y;
    pt.z = p1->z;
    const POINT3DZ *p2 = GSERIALIZED_POINT3DZ_P(sorigin);
    ptorig.x = p2->x;
    ptorig.y = p2->y;
    ptorig.z = p2->z;
  }
  else
  {
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(point);
    pt.x = p1->x;
    pt.y = p1->y;
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(sorigin);
    ptorig.x = p2->x;
    ptorig.y = p2->y;
  }
  double xmin = float_bucket(pt.x, xsize, ptorig.x);
  double ymin = float_bucket(pt.y, ysize, ptorig.y);
  double zmin = float_bucket(pt.z, zsize, ptorig.z);
  TimestampTz tmin = 0; /* make compiler quiet */
  if (hast)
    tmin = timestamptz_bucket1(t, tunits, torigin);
  STBox *result = palloc0(sizeof(STBox));
  stbox_tile_set(xmin, ymin, zmin, tmin, xsize, ysize, zsize, tunits, hasz,
    hast, srid, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Split a temporal point with respect to a spatial and possibly a
 * temporal grid.
 */
Datum
Tpoint_space_time_split_ext(FunctionCallInfo fcinfo, bool timesplit)
{
  FuncCallContext *funcctx;
  STboxGridState *state;
  bool hasz;
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */
  Datum tuple_arr[3]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

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
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    int64 tunits = 0;
    int i = 4;
    if (timesplit)
    {
      duration = PG_GETARG_INTERVAL_P(i++);
      ensure_valid_duration(duration);
      tunits = interval_units(duration);
    }
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    if (timesplit)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);
    bool bitmatrix = PG_GETARG_BOOL(i++);

    /* Set bounding box */
    STBox bounds;
    temporal_set_bbox(temp, &bounds);
    if (! timesplit)
      /* Disallow T dimension for generating a spatial only grid */
      MEOS_FLAGS_SET_T(bounds.flags, false);

    /* Ensure parameter validity */
    ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8);
    ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8);
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    ensure_same_geodetic(temp->flags, sorigin->gflags);
    int32 srid = bounds.srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != SRID_UNKNOWN)
      ensure_same_srid(srid, gs_srid);
    POINT3DZ pt;
    memset(&pt, 0, sizeof(POINT3DZ));
    hasz = MEOS_FLAGS_GET_Z(temp->flags);
    if (hasz)
    {
      ensure_has_Z_gs(sorigin);
      const POINT3DZ *p3d = GSERIALIZED_POINT3DZ_P(sorigin);
      pt.x = p3d->x;
      pt.y = p3d->y;
      pt.z = p3d->z;
    }
    else
    {
      const POINT2D *p2d = GSERIALIZED_POINT2D_P(sorigin);
      pt.x = p2d->x;
      pt.y = p2d->y;
    }

    /* Create function state */
    state = stbox_tile_state_make(temp, &bounds, xsize, ysize, zsize, tunits,
      pt, torigin);
    /* If a bit matrix is used to speed up the process */
    if (bitmatrix)
    {
      /* Create the bit matrix and set the tiles traversed by the temporal point */
      int count[MAXDIMS];
      memset(&count, 0, sizeof(count));
      int numdims = 2;
      /* We need to add 1 to take into account the last bucket for each dimension */
      count[0] = (int) ((state->box.xmax - state->box.xmin) / state->xsize) + 1;
      count[1] = (int) ((state->box.ymax - state->box.ymin) / state->ysize) + 1;
      if (MEOS_FLAGS_GET_Z(state->box.flags))
        count[numdims++] = (int) ((state->box.zmax - state->box.zmin) /
          state->zsize) + 1;
      if (state->tunits)
        count[numdims++] = (int) ((DatumGetTimestampTz(state->box.period.upper) -
          DatumGetTimestampTz(state->box.period.lower)) / state->tunits) + 1;
      state->bm = bitmatrix_make(count, numdims);
      tpoint_set_tiles(temp, state, state->bm);
    }
    funcctx->user_fctx = state;

    /* Build a tuple description for a multidimensional grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
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
      if (state->bm) pfree(state->bm);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }
    stbox_tile_state_next(state);

    /* Restrict the temporal point to the box */
    Temporal *atstbox = tpoint_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (atstbox == NULL)
      continue;

    /* Form tuple and return */
    int i = 0;
    hasz = MEOS_FLAGS_GET_Z(state->temp->flags);
    tuple_arr[i++] = PointerGetDatum(gspoint_make(box.xmin, box.ymin, box.zmin,
      hasz, false, box.srid));
    if (timesplit)
      tuple_arr[i++] = box.period.lower;
    tuple_arr[i++] = PointerGetDatum(atstbox);
    tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
    result = HeapTupleGetDatum(tuple);
    SRF_RETURN_NEXT(funcctx, result);
  }
}

PGDLLEXPORT Datum Tpoint_space_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_space_split);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Split a temporal point with respect to a spatial grid.
 * @sqlfunc spaceSplit()
 */
Datum
Tpoint_space_split(PG_FUNCTION_ARGS)
{
  return Tpoint_space_time_split_ext(fcinfo, false);
}

PGDLLEXPORT Datum Tpoint_space_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_space_time_split);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Split a temporal point with respect to a spatiotemporal grid.
 * @sqlfunc spaceTimeSplit()
 */
Datum
Tpoint_space_time_split(PG_FUNCTION_ARGS)
{
  return Tpoint_space_time_split_ext(fcinfo, true);
}

/*****************************************************************************/
