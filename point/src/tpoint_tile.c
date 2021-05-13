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
 * @file tpoint_tile.c
 * Functions for spatial and spatiotemporal tiles.
 */

#include <postgres.h>
#include <assert.h>
#include <funcapi.h>
#if MOBDB_PGSQL_VERSION < 120000
#include <access/htup_details.h>
#endif
#include <liblwgeom.h>

#include "tpoint_tile.h"
#include "period.h"
#include "timeops.h"
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
 *
 * @param[in] x,y,z,t Lower coordinates of the tile to output
 * @param[in] size Spatial size of the tiles
 * @param[in] tunits Temporal size of the tiles in PostgreSQL time units
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] srid SRID of the spatial coordinates
 *
 */
STBOX *
stbox_tile_get(double x, double y, double z, TimestampTz t, double size,
  int64 tunits, bool hasz, bool hast, int32 srid)
{
  double xmin = x;
  double xmax = xmin + size;
  double ymin = y;
  double ymax = ymin + size;
  double zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  if (hasz)
  {
    zmin = z;
    zmax = zmin + size;
  }
  if (hast)
  {
    tmin = t;
    tmax = tmin + tunits;
  }
  return stbox_make(true, hasz, hast, false, srid, xmin, xmax, ymin, ymax,
    zmin, zmax, tmin, tmax);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] temp Temporal point to split
 * @param[in] box Bounds for generating the multidimensional grid
 * @param[in] size Spatial size of the tiles
 * @param[in] tunits Temporal size of the tiles in PostgreSQL time units
 * @param[in] sorigin Spatial origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre The xsize argument must be greater to 0.
 * @note The tunits argument may be equal to 0 if it was not provided by the
 * user. In that case only the spatial dimension is tiled.
 */
static STboxGridState *
stbox_tile_state_make(Temporal *temp, STBOX *box, double size, int64 tunits,
  POINT3DZ sorigin, TimestampTz torigin)
{
  assert(size > 0);
  /* palloc0 to initialize the missing dimensions to 0 */
  STboxGridState *state = palloc0(sizeof(STboxGridState));
  /* fill in state */
  state->done = false;
  state->i = 1;
  state->size = size;
  state->tunits = tunits;
  state->box.xmin = float_bucket_internal(box->xmin, size, sorigin.x);
  state->box.xmax = float_bucket_internal(box->xmax, size, sorigin.x);
  state->box.ymin = float_bucket_internal(box->ymin, size, sorigin.y);
  state->box.ymax = float_bucket_internal(box->ymax, size, sorigin.y);
  state->box.zmin = float_bucket_internal(box->zmin, size, sorigin.z);
  state->box.zmax = float_bucket_internal(box->zmax, size, sorigin.z);
  state->box.tmin = timestamptz_bucket_internal(box->tmin, size, torigin);
  state->box.tmax = timestamptz_bucket_internal(box->tmax, size, torigin);
  state->box.srid = box->srid;
  state->box.flags = box->flags;
  MOBDB_FLAGS_SET_T(state->box.flags, MOBDB_FLAGS_GET_T(box->flags) && tunits > 0);
  state->x = state->box.xmin;
  state->y = state->box.ymin;
  state->z = state->box.zmin;
  state->t = state->box.tmin;
  state->temp = temp;
  return state;
}

/**
 * Increment the current state to the next tile of the multidimensional grid
 *
 * @param[in] state State to increment
 */
void
stbox_tile_state_next(STboxGridState *state)
{
  if (!state || state->done)
      return;
  /* Move to the next cell. We need to to take into account whether
   * hasz and/or hast and thus there are 4 possible cases */
  state->i++;
  state->x += state->size;
  if (state->x > state->box.xmax)
  {
    state->x = state->box.xmin;
    state->y += state->size;
    if (state->y > state->box.ymax)
    {
      if (MOBDB_FLAGS_GET_Z(state->box.flags))
      {
        /* has Z */
        state->x = state->box.xmin;
        state->y = state->box.ymin;
        state->z += state->size;
        if (state->z > state->box.zmax)
        {
          if (MOBDB_FLAGS_GET_T(state->box.flags))
          {
            /* has Z and has T */
            state->x = state->box.xmin;
            state->y = state->box.ymin;
            state->z = state->box.zmin;
            state->t += state->tunits;
            if (state->t > state->box.tmax)
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
        if (MOBDB_FLAGS_GET_T(state->box.flags))
        {
          /* does not have Z and has T */
          state->x = state->box.xmin;
          state->y = state->box.ymin;
          state->t += state->tunits;
          if (state->t > state->box.tmax)
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
    double size = PG_GETARG_FLOAT8(1);
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
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
      tunits = get_interval_units(duration);
      sorigin = PG_GETARG_GSERIALIZED_P(3);
      torigin = PG_GETARG_TIMESTAMPTZ(4);
    }
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    // ensure_same_spatial_dimensionality_stbox_gs(bounds, sorigin);
    int32 srid = bounds->srid;
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

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
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
    SRF_RETURN_DONE(funcctx);

  /* Store index */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate box */
  tuple_arr[1] = PointerGetDatum(stbox_tile_get(state->x, state->y, state->z,
    state->t, state->size, state->tunits, MOBDB_FLAGS_GET_Z(state->box.flags), 
    MOBDB_FLAGS_GET_T(state->box.flags), state->box.srid));
  /* Advance state */
  stbox_tile_state_next(state);
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
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
    sorigin = PG_GETARG_GSERIALIZED_P(2);
  }
  else /* PG_NARGS() == 6 */
  {
    /* If time arguments are given */
    t = PG_GETARG_TIMESTAMPTZ(1);
    size = PG_GETARG_FLOAT8(2);
    ensure_positive_datum(Float8GetDatum(size), FLOAT8OID);
    Interval *duration = PG_GETARG_INTERVAL_P(3);
    ensure_valid_duration(duration);
    tunits = get_interval_units(duration);
    sorigin = PG_GETARG_GSERIALIZED_P(4);
    torigin = PG_GETARG_TIMESTAMPTZ(5);
    hast = true;
  }
  ensure_non_empty(sorigin);
  ensure_point_type(sorigin);
  int32 srid = gserialized_get_srid(point);
  int32 gs_srid = gserialized_get_srid(sorigin);
  if (gs_srid != 0)
    ensure_same_srid_gs(point, sorigin);
  POINT3DZ pt, ptorig;
  bool hasz = FLAGS_GET_Z(point->flags);
  if (hasz)
  {
    ensure_has_Z_gs(sorigin);
    pt = datum_get_point3dz(PointerGetDatum(point));
    ptorig = datum_get_point3dz(PointerGetDatum(sorigin));
  }
  else
  {
    /* Initialize to 0 the Z dimension if it is missing */
    memset(&pt, 0, sizeof(POINT3DZ));
    const POINT2D *p1 = gs_get_point2d_p(sorigin);
    pt.x = p1->x;
    pt.y = p1->y;
    memset(&ptorig, 0, sizeof(POINT3DZ));
    const POINT2D *p2 = gs_get_point2d_p(sorigin);
    ptorig.x = p2->x;
    ptorig.y = p2->y;
  }
  double xmin = float_bucket_internal(pt.x, size, ptorig.x);
  double ymin = float_bucket_internal(pt.y, size, ptorig.y);
  double zmin = float_bucket_internal(pt.z, size, ptorig.z);
  TimestampTz tmin = 0; /* make compiler quiet */
  if (hast)
    tmin = timestamptz_bucket_internal(t, tunits, torigin);
  STBOX *result = stbox_tile_get(xmin, ymin, zmin, tmin, size, tunits, hasz,
    hast, srid);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

static Datum
stbox_upper_bound(const STBOX *box)
{
  /* Compute the 2D upper bound of the box */
  LWGEOM *points[3];
  /* Top left */
  points[0] = (LWGEOM *) lwpoint_make2d(box->srid, box->xmin, box->ymax); 
  /* Top right */
  points[1] = (LWGEOM *) lwpoint_make2d(box->srid, box->xmax, box->ymax); 
  /* Bottom left */
  points[2] = (LWGEOM *) lwpoint_make2d(box->srid, box->xmax, box->ymin); 
  FLAGS_SET_GEODETIC(points[0]->flags, false);
  FLAGS_SET_GEODETIC(points[1]->flags, false);
  FLAGS_SET_GEODETIC(points[2]->flags, false);
  LWGEOM *lwgeom = (LWGEOM *) lwline_from_lwgeom_array(box->srid, 3, points);
  FLAGS_SET_Z(lwgeom->flags, false);
  FLAGS_SET_GEODETIC(lwgeom->flags, false);
  Datum result = PointerGetDatum(geo_serialize(lwgeom));
  lwgeom_free(lwgeom);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_space_split);
/**
 * Split a temporal number with respect to space tiles.
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
      pt, 0);
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
    /* Stop when we've used up all the grid squares */
    if (state->done)
      SRF_RETURN_DONE(funcctx);

    /* Generate the tile 
     * We must generate a 2D/3D geometry for keeping the bounds and after we
     * set the box to 2D so that we can project a 3D point to a 2D geometry */
    bool hasz = MOBDB_FLAGS_GET_Z(state->temp->flags);
    STBOX *box = stbox_tile_get(state->x, state->y, state->z, state->t,
      state->size, state->tunits, hasz, false, state->box.srid);
    /* Advance state */
    stbox_tile_state_next(state);
    /* Restrict the temporal point to the box projected to 2D */
    MOBDB_FLAGS_SET_Z(box->flags, false);
    Temporal *atstbox = tpoint_at_stbox_internal(state->temp, box);
    if (atstbox == NULL)
      continue;
    if (hasz)
    {
      /* Filter the boxes that do not intersect in 3D with the restriction */
      STBOX box1;
      memset(&box1, 0, sizeof(STBOX));
      temporal_bbox(&box1, atstbox);
      if (box->zmin > box1.zmin || box1.zmin >= box->zmax)
      {
        pfree(atstbox);
        continue;
      }
    }
    Datum upper_bound = stbox_upper_bound(box);
    Temporal *minus_upper = tpoint_restrict_geometry_internal(atstbox,
      upper_bound, REST_MINUS);
    pfree(DatumGetPointer(upper_bound));
    if (minus_upper != NULL)
    {
      /* Form tuple and return */
      tuple_arr[0] = point_make(box->xmin, box->ymin, box->zmin,
        MOBDB_FLAGS_GET_Z(state->temp->flags), false, box->srid);
      tuple_arr[1] = PointerGetDatum(minus_upper);
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
      pt, torigin);
    /* Build a tuple description for a multidimensional grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* We need to loop since atRange may be NULL */
  while (true)
  {
    /* Stop when we've used up all the grid squares */
    if (state->done)
      SRF_RETURN_DONE(funcctx);

    /* Generate the tile 
     * We must generate a 2D/3D geometry for keeping the bounds and after we
     * set the box to 2D so that we can project a 3D point to a 2D geometry */
    bool hasz = MOBDB_FLAGS_GET_Z(state->temp->flags);
    STBOX *box = stbox_tile_get(state->x, state->y, state->z, state->t,
      state->size, state->tunits, hasz, true, state->box.srid);
    /* Advance state */
    stbox_tile_state_next(state);
    /* Restrict the temporal point to the box projected to 2D */
    MOBDB_FLAGS_SET_Z(box->flags, false);
    Temporal *atstbox = tpoint_at_stbox_internal(state->temp, box);
    if (atstbox == NULL)
      continue;
    if (hasz)
    {
      /* Filter the boxes that do not intersect in 3D with the restriction */
      STBOX box1;
      memset(&box1, 0, sizeof(STBOX));
      temporal_bbox(&box1, atstbox);
      if (box1.zmin >= box->zmax || box->zmin >= box1.zmax)
        continue;
    }
    Datum upper_bound = stbox_upper_bound(box);
    Temporal *minus_upper = tpoint_restrict_geometry_internal(atstbox,
      upper_bound, REST_MINUS);
    pfree(DatumGetPointer(upper_bound));
    if (minus_upper != NULL)
    {
      /* Form tuple and return */
      tuple_arr[0] = point_make(box->xmin, box->ymin, box->zmin,
        hasz, false, box->srid);
      tuple_arr[1] = TimestampTzGetDatum(box->tmin);
      tuple_arr[2] = PointerGetDatum(minus_upper);
      tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
      result = HeapTupleGetDatum(tuple);
      SRF_RETURN_NEXT(funcctx, result);
    }
  }
}

/*****************************************************************************/
