/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file tile.c
 * Functions for spatial and spatiotemporal tiles.
 */

#include <postgres.h>
#include <assert.h>
#include <funcapi.h>
#include <liblwgeom.h>

#include "tpoint_tile.h"
#include "temporal.h"
#include "temporal_util.h"
#include "temporal_tile.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

/**
 * Generate a tile from the current state of the multidimensional grid
 */
STBOX *
stbox_tile(bool hasz, bool hast, int32 srid, POINT3DZ sorigin,
  TimestampTz torigin, double xsize, int64 tsize, int *coords)
{
  double xmin = sorigin.x + (xsize * coords[0]);
  double xmax = sorigin.x + (xsize * (coords[0] + 1));
  double ymin = sorigin.y + (xsize * coords[1]);
  double ymax = sorigin.y + (xsize * (coords[1] + 1));
  double zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  int ndims = 2;
  if (hasz)
  {
    zmin = sorigin.z + (xsize * coords[ndims]);
    zmax = sorigin.z + (xsize * (coords[ndims++] + 1));
  }
  if (hast)
  {
    tmin = torigin + (TimestampTz) (tsize * coords[ndims]);
    tmax = torigin + (TimestampTz) (tsize * (coords[ndims] + 1));
  }
  return (STBOX *) stbox_make(true, hasz, hast, false, srid,
    xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax);
}

/**
 * Create the initial state that persists across the multiple calls generating
 * the multidimensional grid.
 * @pre The xsize argument must be greater to 0.
 * @note The tsize argument may be equal to 0 if it was not provided by the
 * user. In that case only the spatial dimension is tiled.
 */
STboxGridState *
stbox_tile_state_make(Temporal *temp, STBOX *box, double size, int64 tsize,
  POINT3DZ sorigin, TimestampTz torigin, int32 srid)
{
  assert(size > 0);
  /* palloc0 to initialize the missing dimensions to 0 */
  STboxGridState *state = palloc0(sizeof(STboxGridState));

  /* fill in state */
  state->done = false;
  state->hasz = MOBDB_FLAGS_GET_Z(box->flags);
  state->hast = MOBDB_FLAGS_GET_T(box->flags) && tsize > 0;
  state->srid = box->srid;
  state->size = size;
  state->tsize = tsize;
  state->srid = srid;
  state->temp = temp;
  state->sorigin = sorigin;
  state->torigin = torigin;
  state->min[0] = floor(box->xmin / size) - floor(sorigin.x / size);
  state->max[0] = floor(box->xmax / size) - floor(sorigin.x / size);
  state->min[1] = floor(box->ymin / size) - floor(sorigin.y / size);
  state->max[1] = floor(box->ymax / size) - floor(sorigin.y / size);
  int ndims = 2;
  if (state->hasz)
  {
    state->min[ndims] = floor(box->zmin / size) - floor(sorigin.z / size);
    state->max[ndims++] = floor(box->zmax / size) - floor(sorigin.z / size);
  }
  else if (state->hast)
  {
    state->min[ndims] = (box->tmin / tsize) - (torigin / tsize);
    state->max[ndims++] = (box->tmax / tsize) - (torigin / tsize);
  }
  for (int i = 0; i < ndims; i++)
    state->coords[i] = state->min[i];
  return state;
}

/**
 * Increment the current state to the next tile of the multidimensional grid
 */
void
stbox_tile_state_next(STboxGridState *state)
{
  if (!state || state->done)
      return;
  /* Move to the next cell. We need to to take into account whether
   * hasz and/or hast and thus there are 4 possible cases */
  state->coords[0]++;
  if (state->coords[0] > state->max[0])
  {
    state->coords[0] = state->min[0];
    state->coords[1]++;
    if (state->coords[1] > state->max[1])
    {
      if (state->hasz)
      {
        /* has Z */
        state->coords[0] = state->min[0];
        state->coords[1] = state->min[1];
        state->coords[2]++;
        if (state->coords[2] > state->max[2])
        {
          if (state->hast)
          {
            /* has Z and has T */
            state->coords[0] = state->min[0];
            state->coords[1] = state->min[1];
            state->coords[2] = state->min[2];
            state->coords[3]++;
            if (state->coords[3] > state->max[3])
            {
              state->done = true;
              return;
            }
          }
          else
          {
            /* has Z and does not have T */
            state->done = true;
            return;
          }
        }
      }
      else
      {
        /* does not have Z */
        if (state->hast)
        {
          state->coords[0] = state->min[0];
          state->coords[1] = state->min[1];
          state->coords[2]++;
          if (state->coords[2] > state->max[2])
          {
            state->done = true;
            return;
          }
        }
        else
        {
          /* does not have Z and does have T */
          state->done = true;
          return;
        }
      }
    }
  }
  return;
}

PG_FUNCTION_INFO_V1(stbox_multidim_grid);
/**
 * Generate a multidimensional grid for temporal points.
 */
Datum stbox_multidim_grid(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  STboxGridState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    STBOX *bounds = PG_GETARG_STBOX_P(0);
    ensure_not_geodetic_stbox(bounds);
    ensure_has_X_stbox(bounds);
    int32 srid = bounds->srid;
    double size = PG_GETARG_FLOAT8(1);
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
    GSERIALIZED *sorigin;
    int64 tsize = 0;
    TimestampTz torigin = 0;
    if (PG_NARGS() == 3)
      sorigin = PG_GETARG_GSERIALIZED_P(2);
    else /* PG_NARGS() == 5 */
    {
      /* If time arguments are given */
      ensure_has_T_stbox(bounds);
      Interval *duration = PG_GETARG_INTERVAL_P(2);
      ensure_valid_duration(duration);
      tsize = get_interval_units(duration);
      sorigin = PG_GETARG_GSERIALIZED_P(3);
      torigin = PG_GETARG_TIMESTAMPTZ(4);
    }
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != 0)
      error_if_srid_mismatch(srid, gs_srid);
    POINT3DZ p;
    if (FLAGS_GET_Z(sorigin->flags))
      p = datum_get_point3dz(PointerGetDatum(sorigin));
    else
    {
      /* Initialize to 0 the Z dimension if it is missing */
      memset(&p, 0, sizeof(POINT3DZ));
      const POINT2D *p2d = gs_get_point2d_p(sorigin);
      p.x = p2d->x;
      p.y = p2d->y;
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = stbox_tile_state_make(NULL, bounds, size, tsize, p,
      torigin, srid);
    /* Build a tuple description for a multidim_grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all the grid tiles */
  if (state->done)
    SRF_RETURN_DONE(funcctx);

  /* Store tile coordinates */
  Datum coords[4];
  coords[0] = Int32GetDatum(state->coords[0]);
  coords[1] = Int32GetDatum(state->coords[1]);
  int ndims = 2;
  if (state->hasz)
    coords[ndims++] = Int32GetDatum(state->coords[2]);
  if (state->hast && state->tsize > 0)
    coords[ndims++] = Int32GetDatum(state->coords[3]);
  ArrayType *coordarr = intarr_to_array(coords, ndims);
  tuple_arr[0] = PointerGetDatum(coordarr);

  /* Generate box */
  STBOX *box = stbox_tile(state->hasz, state->hast, state->srid, 
    state->sorigin, state->torigin, state->size, state->tsize,
    state->coords);
  stbox_tile_state_next(state);
  tuple_arr[1] = PointerGetDatum(box);

  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PG_FUNCTION_INFO_V1(stbox_multidim_tile);
/**
 * Generate a tile in a multidimensional grid for temporal points.
*/
Datum stbox_multidim_tile(PG_FUNCTION_ARGS)
{
  ArrayType *coordarr = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(coordarr);
  int ndims;
  int *coords = intarr_extract(coordarr, &ndims);
  if (ndims < 2 || ndims > 4)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The number of coordinates must be between 2 and 4")));
  double size = PG_GETARG_FLOAT8(1);
  ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
  GSERIALIZED *sorigin;
  int64 tsize = 0;
  bool hast = false;
  TimestampTz torigin = 0;

  if (PG_NARGS() == 3)
    sorigin = PG_GETARG_GSERIALIZED_P(2);
  else /* PG_NARGS() == 5 */
  {
    /* If time arguments are given */
    if (ndims == 2)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("The number of coordinates must be at least 3 for the temporal dimension")));
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    ensure_valid_duration(duration);
    tsize = get_interval_units(duration);
    sorigin = PG_GETARG_GSERIALIZED_P(3);
    torigin = PG_GETARG_TIMESTAMPTZ(4);
    hast = true;
  }

  ensure_non_empty(sorigin);
  ensure_point_type(sorigin);
  int32 srid = gserialized_get_srid(sorigin);
  bool hasz = (ndims == 4 || (ndims == 3 && ! hast));
  POINT3DZ p;
  if (FLAGS_GET_Z(sorigin->flags))
    p = datum_get_point3dz(PointerGetDatum(sorigin));
  else
  {
    /* Initialize to 0 the Z dimension if it is missing */
    memset(&p, 0, sizeof(POINT3DZ));
    const POINT2D *p2d = gs_get_point2d_p(sorigin);
    p.x = p2d->x;
    p.y = p2d->y;
  }
  STBOX *result = stbox_tile(hasz, hast, srid, p, torigin, size, tsize,
    coords);
  PG_FREE_IF_COPY(coordarr, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_space_split);
/**
 * Split a temporal number with respect to value buckets.
 */
Datum tpoint_space_split(PG_FUNCTION_ARGS)
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
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    double size = PG_GETARG_FLOAT8(1);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(2);

    /* Ensure parameter validity */
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    ensure_same_geodetic(temp->flags, sorigin->flags);
    STBOX bounds;
    memset(&bounds, 0, sizeof(STBOX));
    temporal_bbox(&bounds, temp);
    int32 srid = bounds.srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != 0)
      error_if_srid_mismatch(srid, gs_srid);
    /* Disallow T dimension for generating a spatial only grid */
    MOBDB_FLAGS_SET_T(bounds.flags, false);
    POINT3DZ pt;
    if (FLAGS_GET_Z(sorigin->flags))
      pt = datum_get_point3dz(PointerGetDatum(sorigin));
    else
    {
      /* Initialize to 0 the Z dimension if it is missing */
      memset(&pt, 0, sizeof(POINT3DZ));
      const POINT2D *p2d = gs_get_point2d_p(sorigin);
      pt.x = p2d->x;
      pt.y = p2d->y;
    }

    /* Create function state */
    funcctx->user_fctx = stbox_tile_state_make(temp, &bounds, size, 0,
      pt, 0, srid);
    /* Build a tuple description for a multidimensial grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  
  /* We need to loop since atRange may be NULL */
  while (true)
  {
    /* Stop when we've used up all the grid squares */
    if (state->done)
      SRF_RETURN_DONE(funcctx);

    /* Generate tile 
     * We must generate a 2D geometry so that we can project a 3D point 
     * to 2D geometry */
    STBOX *box = stbox_tile(false, state->hast, state->srid, 
      state->sorigin, state->torigin, state->size, state->tsize,
      state->coords);
    stbox_tile_state_next(state);
    Temporal *atstbox = tpoint_at_stbox_internal(state->temp, box);
    if (atstbox != NULL)
    {
      /* Form tuple and return */
      tuple_arr[0] = point_make(box->xmin, box->ymin, box->zmin, state->hasz,
        false, state->srid);
      tuple_arr[1] = PointerGetDatum(atstbox);
      tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
      result = HeapTupleGetDatum(tuple);
      SRF_RETURN_NEXT(funcctx, result);
    }
  }
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_space_time_split);
/**
 * Split a temporal number with respect to value buckets.
 */
Datum tpoint_space_time_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  STboxGridState *state;
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
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    double size = PG_GETARG_FLOAT8(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    /* Ensure parameter validity */
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    ensure_same_geodetic(temp->flags, sorigin->flags);
    STBOX bounds;
    memset(&bounds, 0, sizeof(STBOX));
    temporal_bbox(&bounds, temp);
    int32 srid = bounds.srid;
    int32 gs_srid = gserialized_get_srid(sorigin);
    if (gs_srid != 0)
      error_if_srid_mismatch(srid, gs_srid);
    POINT3DZ pt;
    if (FLAGS_GET_Z(sorigin->flags))
      pt = datum_get_point3dz(PointerGetDatum(sorigin));
    else
    {
      /* Initialize to 0 the Z dimension if it is missing */
      memset(&pt, 0, sizeof(POINT3DZ));
      const POINT2D *p2d = gs_get_point2d_p(sorigin);
      pt.x = p2d->x;
      pt.y = p2d->y;
    }

    /* Create function state */
    funcctx->user_fctx = stbox_tile_state_make(temp, &bounds, size, tunits,
      pt, torigin, srid);
    /* Build a tuple description for a multidimensial grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  
  /* We need to loop since atRange may be NULL */
  while (true)
  {
    /* Stop when we've used up all the grid squares */
    if (state->done)
      SRF_RETURN_DONE(funcctx);

    /* Generate tile 
     * We must generate a 2D geometry so that we can project a 3D point 
     * to 2D geometry */
    STBOX *box = stbox_tile(false, state->hast, state->srid, 
      state->sorigin, state->torigin, state->size, state->tsize,
      state->coords);
    stbox_tile_state_next(state);
    Temporal *atstbox = tpoint_at_stbox_internal(state->temp, box);
    if (atstbox != NULL)
    {
      /* Form tuple and return */
      tuple_arr[0] = point_make(box->xmin, box->ymin, box->zmin, state->hasz,
        false, state->srid);
      tuple_arr[1] = TimestampTzGetDatum(box->tmin);
      tuple_arr[2] = PointerGetDatum(atstbox);
      tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
      result = HeapTupleGetDatum(tuple);
      SRF_RETURN_NEXT(funcctx, result);
    }
  }
}

/*****************************************************************************/
