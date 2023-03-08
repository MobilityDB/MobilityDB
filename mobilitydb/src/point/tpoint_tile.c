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

PG_FUNCTION_INFO_V1(Stbox_tile_list);
/**
 * @brief @ingroup mobilitydb_temporal_tile
 * @brief Generate a multidimensional grid for temporal points.
 * @sqlfunc multidimGrid()
 */
PGDLLEXPORT Datum
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
    double size = PG_GETARG_FLOAT8(1);
    ensure_positive_datum(Float8GetDatum(size), T_FLOAT8);
    GSERIALIZED *sorigin;
    int64 tunits = 0; /* make compiler quiet */
    TimestampTz torigin = 0; /* make compiler quiet */
    if (PG_NARGS() == 3)
      sorigin = PG_GETARG_GSERIALIZED_P(2);
    else /* PG_NARGS() == 5 */
    {
      /* If time arguments are given */
      ensure_has_T_stbox(bounds);
      Interval *duration = PG_GETARG_INTERVAL_P(2);
      ensure_valid_duration(duration);
      tunits = interval_units(duration);
      sorigin = PG_GETARG_GSERIALIZED_P(3);
      torigin = PG_GETARG_TIMESTAMPTZ(4);
    }
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    /* Since we pass by default Point(0 0 0) as origin independently of the input
     * STBox, we test the same spatial dimensionality only for STBox Z */
    if (MOBDB_FLAGS_GET_Z(bounds->flags))
      ensure_same_spatial_dimensionality_stbox_gs(bounds, sorigin);
    int32 srid = bounds->srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != SRID_UNKNOWN)
      ensure_same_srid(srid, gs_srid);
    POINT3DZ pt;
    if (FLAGS_GET_Z(sorigin->gflags))
      pt = datum_point3dz(PointerGetDatum(sorigin));
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
    funcctx->user_fctx = stbox_tile_state_make(NULL, bounds, size, tunits, pt,
      torigin);
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

PG_FUNCTION_INFO_V1(Stbox_tile);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Generate a tile in a multidimensional grid for temporal points.
 * @sqlfunc multidimTile()
 */
PGDLLEXPORT Datum
Stbox_tile(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  ensure_non_empty(point);
  ensure_point_type(point);
  TimestampTz t;
  double size;
  int64 tunits = 0; /* make compiler quiet */
  GSERIALIZED *sorigin;
  TimestampTz torigin;
  bool hast = false;
  if (PG_NARGS() == 3)
  {
    size = PG_GETARG_FLOAT8(1);
    sorigin = PG_GETARG_GSERIALIZED_P(2);
  }
  else /* PG_NARGS() == 6 */
  {
    /* If time arguments are given */
    t = PG_GETARG_TIMESTAMPTZ(1);
    size = PG_GETARG_FLOAT8(2);
    Interval *duration = PG_GETARG_INTERVAL_P(3);
    ensure_valid_duration(duration);
    tunits = interval_units(duration);
    sorigin = PG_GETARG_GSERIALIZED_P(4);
    torigin = PG_GETARG_TIMESTAMPTZ(5);
    hast = true;
  }
  /* Ensure parameter validity */
  ensure_positive_datum(Float8GetDatum(size), T_FLOAT8);
  ensure_non_empty(sorigin);
  ensure_point_type(sorigin);
  int32 srid = gserialized_get_srid(point);
  int32 gs_srid = gserialized_get_srid(sorigin);
  if (gs_srid != SRID_UNKNOWN)
    ensure_same_srid(srid, gs_srid);
  POINT3DZ pt, ptorig;
  bool hasz = (bool) FLAGS_GET_Z(point->gflags);
  if (hasz)
  {
    ensure_has_Z_gs(sorigin);
    pt = datum_point3dz(PointerGetDatum(point));
    ptorig = datum_point3dz(PointerGetDatum(sorigin));
  }
  else
  {
    /* Initialize to 0 the Z dimension if it is missing */
    memset(&pt, 0, sizeof(POINT3DZ));
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(sorigin);
    pt.x = p1->x;
    pt.y = p1->y;
    memset(&ptorig, 0, sizeof(POINT3DZ));
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(sorigin);
    ptorig.x = p2->x;
    ptorig.y = p2->y;
  }
  double xmin = float_bucket(pt.x, size, ptorig.x);
  double ymin = float_bucket(pt.y, size, ptorig.y);
  double zmin = float_bucket(pt.z, size, ptorig.z);
  TimestampTz tmin = 0; /* make compiler quiet */
  if (hast)
    tmin = timestamptz_bucket1(t, tunits, torigin);
  STBox *result = palloc0(sizeof(STBox));
  stbox_tile_set(xmin, ymin, zmin, tmin, size, tunits, hasz, hast, srid,
    result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Get the tile border that must be removed from a spatiotemporal box
 * during the tiling process as a PostGIS geometry
 *
 * The following figure shows the borders that are removed (represented by x)
 * @code
 * xxxxxxxxxxxxxxxxx
 * |               x
 * |               x
 * |---------------x
 * @code
 */
static GSERIALIZED *
stbox_to_tile_border(const STBox *box)
{
  ensure_has_X_stbox(box);
  assert(! MOBDB_FLAGS_GET_Z(box->flags));
  assert(! MOBDB_FLAGS_GET_GEODETIC(box->flags));
  /* Since there is no M value a 0 value is passed */
  POINTARRAY *pa = ptarray_construct_empty(false, 0, 3);
  /* Initialize the 3 vertices of the line */
  POINT4D pt;
  pt = (POINT4D) { box->xmin, box->ymax, 0.0, 0.0 };
  ptarray_append_point(pa, &pt, LW_TRUE);
  pt = (POINT4D) { box->xmax, box->ymax, 0.0, 0.0 };
  ptarray_append_point(pa, &pt, LW_TRUE);
  pt = (POINT4D) { box->xmax, box->ymin, 0.0, 0.0 };
  ptarray_append_point(pa, &pt, LW_TRUE);
  /* No bbox is passed as second argument */
  LWLINE *line = lwline_construct(box->srid, NULL, pa);
  FLAGS_SET_Z(line->flags, false);
  FLAGS_SET_GEODETIC(line->flags, false);
  LWGEOM *geo = lwline_as_lwgeom(line);
  GSERIALIZED *result = geo_serialize(geo);
  lwgeom_free(geo);
  return result;
}

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
    double size = PG_GETARG_FLOAT8(1);
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    int64 tunits = 0;
    int i = 2;
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
      MOBDB_FLAGS_SET_T(bounds.flags, false);

    /* Ensure parameter validity */
    ensure_positive_datum(Float8GetDatum(size), T_FLOAT8);
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    ensure_same_geodetic(temp->flags, sorigin->gflags);
    int32 srid = bounds.srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != SRID_UNKNOWN)
      ensure_same_srid(srid, gs_srid);
    POINT3DZ pt;
    bool hasz = (bool) MOBDB_FLAGS_GET_Z(temp->flags);
    if (hasz)
    {
      ensure_has_Z_gs(sorigin);
      pt = datum_point3dz(PointerGetDatum(sorigin));
    }
    else
    {
      /* Initialize to 0 the Z dimension if it is missing */
      memset(&pt, 0, sizeof(POINT3DZ));
      const POINT2D *p2d = GSERIALIZED_POINT2D_P(sorigin);
      pt.x = p2d->x;
      pt.y = p2d->y;
    }

    /* Create function state */
    STboxGridState *state = stbox_tile_state_make(temp, &bounds, size, tunits,
      pt, torigin);
    /* If a bit matrix is used to speed up the process */
    if (bitmatrix)
    {
      /* Create the bit matrix and set the tiles traversed by the temporal point */
      int count[MAXDIMS];
      memset(&count, 0, sizeof(count));
      int numdims = 2;
      /* We need to add 1 to take into account the last bucket for each dimension */
      count[0] = ( (state->box.xmax - state->box.xmin) / state->size ) + 1;
      count[1] = ( (state->box.ymax - state->box.ymin) / state->size ) + 1;
      if (MOBDB_FLAGS_GET_Z(state->box.flags))
        count[numdims++] = ( (state->box.zmax - state->box.zmin) / state->size ) + 1;
      if (state->tunits)
        count[numdims++] = ( (DatumGetTimestampTz(state->box.period.upper) -
          DatumGetTimestampTz(state->box.period.lower)) / state->tunits ) + 1;
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
      if (state->bm) pfree(state->bm);
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
    Temporal *atstbox = tpoint_at_stbox1(state->temp, &box);
    if (atstbox == NULL)
      continue;
    /* Remove the right and lower bound of the tile */
    STBox box2d;
    memcpy(&box2d, &box, sizeof(STBox));
    MOBDB_FLAGS_SET_Z(box2d.flags, false);
    GSERIALIZED *geo = stbox_to_tile_border(&box2d);
    Temporal *atstbox1 = tpoint_restrict_geometry(atstbox, geo, REST_MINUS);
    pfree(geo); pfree(atstbox);
    atstbox = atstbox1;
    if (atstbox == NULL)
      continue;
    /* Form tuple and return */
    hasz = MOBDB_FLAGS_GET_Z(state->temp->flags);
    int i = 0;
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

PG_FUNCTION_INFO_V1(Tpoint_space_split);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Split a temporal point with respect to a spatial grid.
 * @sqlfunc spaceSplit()
 */
PGDLLEXPORT Datum
Tpoint_space_split(PG_FUNCTION_ARGS)
{
  return Tpoint_space_time_split_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tpoint_space_time_split);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Split a temporal point with respect to a spatiotemporal grid.
 * @sqlfunc spaceTimeSplit()
 */
PGDLLEXPORT Datum
Tpoint_space_time_split(PG_FUNCTION_ARGS)
{
  return Tpoint_space_time_split_ext(fcinfo, true);
}

/*****************************************************************************/
