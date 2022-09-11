/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
/* MobilityDB */
#include "pg_general/temporal_tile.h"
#include "pg_point/postgis.h"
#include "pg_point/tpoint_tile.h"

/*****************************************************************************
 * Bit Matrix implementation based on
 * https://www.pvladov.com/2012/05/bit-matrix-in-c-sharp.html
 *****************************************************************************/

/**
 * @brief Create a bit matrix
 */
static BitMatrix *
bitmatrix_make(int *count, int numdims)
{
  /* Calculate the needed number of bits and bytes */
  int i, bitCount = 1;
  for (i = 0; i < numdims; i++)
    bitCount *= count[i];
  int byteCount = bitCount >> 3;
  if (bitCount % 8 != 0)
    byteCount++;
  /* Allocate the needed number of bytes taking into account that there is
   * already one byte allocated in the struct */
  size_t size = sizeof(BitMatrix) + byteCount - 1;
  /* palloc0 to set all bits to 0 */
  BitMatrix *result = palloc0(size);
  /* Fill the structure */
  result->numdims = numdims;
  for (i = 0; i < numdims; i++)
    result->count[i] = count[i];
  return result;
}

/**
 * @brief Get the value of the bit in the bit matrix
 */
static bool
bitmatrix_get(const BitMatrix *bm, int *coords)
{
  int i, j;
  for (i = 0; i < bm->numdims; i++)
    assert(coords[i] <= bm->count[i]);
  int pos = 0;
  for (i = 0; i < bm->numdims; i++)
  {
    int offset = coords[i];
    for (j = i + 1; j < bm->numdims; j++)
    {
      offset *= bm->count[j];
    }
    pos += offset;
  }
  int index = pos % 8;
  pos >>= 3;
  return (bm->byte[pos] & (((uint8_t)1) << index)) != 0;
}

/**
 * @brief Set the value of the bit in the bit matrix
 */
static void
bitmatrix_set_cell(BitMatrix *bm, int *coords, bool value)
{
  int i, j, pos = 0;
  for (i = 0; i < bm->numdims; i++)
    assert(coords[i] <= bm->count[i]);
  for (i = 0; i < bm->numdims - 1; i++)
  {
    int offset = coords[i];
    for (j = i + 1; j < bm->numdims; j++)
      offset *= bm->count[j];
    pos += offset;
  }
  pos += coords[bm->numdims - 1];
  int index = pos % 8;
  pos >>= 3;
  bm->byte[pos] &= (unsigned char)(~(1 << index));
  if (value)
    bm->byte[pos] |= (uint8_t)(1 << index);
  return;
}

#ifdef DEBUG_BUILD
/**
 * @brief Print a 2D bit matrix
 * @note This function is only used for debugging purposes
 */
void
bitmatrix_print2D(const BitMatrix *bm, int *coords)
{
  int i, j;
  int dim = bm->numdims - 2;
  /* Print the 2D matrix */
  if (bm->count[dim + 1] / 10 > 0)
  {
    printf("\n      ");
    for (j = 0; j < bm->count[dim + 1] / 10; j++)
    {
      printf("         %1d", j + 1);
    }
  }
  printf("\n     ");
  for (j = 0; j < bm->count[dim + 1]; j++)
  {
    printf("%1d", j % 10);
  }
  printf("\n-----");
  for (j = 0; j < bm->count[dim + 1]; j++)
    printf("-");
  printf("\n");
  for (i = 0; i < bm->count[dim + 0]; i++)
  {
    if (i / 10 > 0 && i % 10 == 0)
      printf("%1d", i / 10);
    else
      printf(" ");
    printf("%1d | ", i % 10);
    coords[dim + 0] = i;
    for (j = 0; j < bm->count[dim + 1]; j++)
    {
      coords[dim + 1] = j;
      if (bitmatrix_get(bm, coords))
        printf("1");
      else
        printf(".");
    }
    printf("\n");
  }
  printf("-----");
  for (j = 0; j < bm->count[dim + 1]; j++)
    printf("-");
  if (bm->count[dim + 1] / 10 > 0)
  {
    printf("\n      ");
    for (j = 0; j < bm->count[dim + 1] / 10; j++)
    {
      printf("         %1d", j + 1);
    }
  }
  printf("\n     ");
  for (j = 0; j < bm->count[dim + 1]; j++)
    printf("%1d", j % 10);
  printf("\n");
  return;
}

/**
 * @brief Print the bit matrix
 * @note This function is only used while debugging
 */
void
bitmatrix_print(const BitMatrix *bm)
{
  int i, j, k, l, coords[MAXDIMS];
  memset(&coords, 0, sizeof(coords));
  int totalcount = 1;
  for (i = 0; i < bm->numdims; i++)
    totalcount *= bm->count[i];
  int count = 0;
  for (i = 0; i < bm->count[0]; i++)
  {
    coords[0] = i;
    for (j = 0; j < bm->count[1]; j++)
    {
      coords[1] = j;
      if (bm->numdims >= 3)
      {
        for (k = 0; k < bm->count[2]; k++)
        {
          coords[2] = k;
          if (bm->numdims >= 4)
          {
            for (l = 0; l < bm->count[3]; l++)
            {
              coords[3] = l;
              if (bitmatrix_get(bm, coords))
              {
                printf("%d %d %d %d: %c\n", i, j, k, l, '1');
                count++;
              }
            }
          }
          else
          {
            if (bitmatrix_get(bm, coords))
            {
              printf("%d %d %d: %c\n", i, j, k, '1');
              count++;
            }
          }
        }
      }
      else
      {
        if (bitmatrix_get(bm, coords))
        {
          printf("%d %d: %c\n", i, j, '1');
          count++;
        }
      }
    }
  }
  printf("Total tiles: %d, Tiles set: %d\n", totalcount, count);
}
#endif

/*****************************************************************************
 * N-dimensional version of Bresenham's line algorithm
 * https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 * setting in the bit array all the tiles connecting the two given tiles
 *****************************************************************************/

#ifdef DEBUG_BUILD
/**
 * @brief Print the coordinates of a tile
 * @note This function is only used for debugging purposes
 */
void
coord_print(int *coords, int numdims)
{
  printf("(");
  for (int i = 0; i < numdims; i++)
  {
    if (i > 0) printf(", ");
    printf("%d", coords[i]);
  }
  printf(")\n");
}
#endif

/**
 * @brief Set in the bit matrix the bits of the tiles connecting with a line the
 * two input tiles
 *
 * @param[in] coords1, coords2 Coordinates of the input tiles
 * @param[in] numdims Number of dimensions of the grid
 * @param[out] bm Bit matrix
 */
static void
bresenham_bm(BitMatrix *bm, int *coords1, int *coords2, int numdims)
{
  int i, j, delta[MAXDIMS], next[MAXDIMS], p[MAXDIMS], coords[MAXDIMS],
    neighbors[MAXDIMS];
  memset(delta, 0, sizeof(delta));
  memset(next, 0, sizeof(next));
  memset(p, 0, sizeof(p));
  memset(coords, 0, sizeof(coords));
  memset(neighbors, 0, sizeof(neighbors));
  /* Compute the delta and next values for every dimension */
  for (i = 0; i < numdims; i++)
  {
    delta[i] = abs(coords2[i] - coords1[i]);
    if (coords2[i] > coords1[i])
      next[i] = 1;
    else
      next[i] = -1;
  }
  /* Compute the driving axis
   * At the end, the driving axis is the one in variable axis */
  int axis;
  for (i = 0; i < numdims; i++)
  {
    /* We bet on the current i axis */
    bool found = true;
    axis = i;
    for (j = 0; j < numdims; j++)
    {
      if (i == j) continue;
      if (delta[i] < delta[j])
      {
        found = false;
        break;
      }
    }
    if (found)
      break;
  }
  for (i = 0; i < numdims; i++)
  {
    if (i == axis) continue;
    p[i] = 2 * delta[i] - delta[axis];
  }
  /* Make a copy of the start coordinates */
  memcpy(coords, coords1, sizeof(neighbors));
  /* Loop from start to end tile
   * The end of the loop is after printing the last element */
  while (true)
  {
    /* Set the current Bresenham diagonal tile */
    // coord_print(coords, numdims);
    bitmatrix_set_cell(bm, coords, true);
    /* Exit the loop when finished */
    if (coords[axis] == coords2[axis])
      break;
    /* Find neighbors of the Bresenham diagonal tile */
    memcpy(neighbors, coords, sizeof(neighbors));
    // printf("Neighbors\n");
    for (i = 1; i < numdims; i++)
    {
      if (next[i] == 0) continue;
      /* Bottom of the Bresenham diagonal cell for 2D */
      neighbors[i] -= 1;
      int min = Min(coords1[i], coords2[i]);
      int max = Max(coords1[i], coords2[i]);
      if (neighbors[i] >= min)
      {
        // coord_print(neighbors, numdims);
        bitmatrix_set_cell(bm, coords, true);
      }
      /* Top of the Bresenham diagonal cell for 2D */
      neighbors[i] += 2;
      if (neighbors[i] <= max)
      {
        // coord_print(neighbors, numdims);
        bitmatrix_set_cell(bm, coords, true);
      }
    }
    // printf("-------\n");
    /* Advance state */
    coords[axis] += next[axis];
    for (i = 0; i < numdims; i++)
    {
      if (i == axis) continue;
      if (p[i] >= 0)
      {
        coords[i] += next[i];
        p[i] -= 2 * delta[axis];
      }
    }
    for (i = 0; i < numdims; i++)
    {
      if (i == axis) continue;
      p[i] += 2 * delta[i];
    }
  }
  return;
}

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

/**
 * @brief Generate a tile from the current state of the multidimensional grid
 *
 * @param[in] x,y,z,t Lower coordinates of the tile to output
 * @param[in] size Tile size for the spatial dimensions in the units of the SRID
 * @param[in] tunits Tile size for the temporal dimension in PostgreSQL time units
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] srid SRID of the spatial coordinates
 * @param[out] result Box representing the tile
 */
static void
stbox_tile_set(double x, double y, double z, TimestampTz t, double size,
  int64 tunits, bool hasz, bool hast, int32 srid, STBOX *result)
{
  double xmin = x;
  double xmax = xmin + size;
  double ymin = y;
  double ymax = ymin + size;
  double zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0;
  Period p;
  if (hasz)
  {
    zmin = z;
    zmax = zmin + size;
  }
  if (hast)
  {
    tmin = t;
    tmax = tmin + tunits;
    span_set(TimestampTzGetDatum(tmin), TimestampTzGetDatum(tmax), true, 
      (tmin == tmax), T_TIMESTAMPTZ, &p);
  }
  return stbox_set(hast ? &p : NULL, true, hasz, false, srid, xmin, xmax,
    ymin, ymax, zmin, zmax, result);
}

/**
 * @brief Create the initial state that persists across multiple calls of the function
 *
 * @param[in] temp Optional temporal point to split
 * @param[in] box Bounds for generating the multidimensional grid
 * @param[in] size Tile size for the spatial dimensions in the units of the SRID
 * @param[in] tunits Tile size for the temporal dimension in PostgreSQL time units
 * @param[in] sorigin Spatial origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre The size argument must be greater to 0.
 * @note The tunits argument may be equal to 0. In that case only the spatial
 * dimension is tiled.
 */
static STboxGridState *
stbox_tile_state_make(Temporal *temp, STBOX *box, double size, int64 tunits,
  POINT3DZ sorigin, TimestampTz torigin)
{
  assert(size > 0);
  /* palloc0 to initialize the missing dimensions to 0 */
  STboxGridState *state = palloc0(sizeof(STboxGridState));
  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->size = size;
  state->tunits = tunits;
  state->box.xmin = float_bucket(box->xmin, size, sorigin.x);
  state->box.xmax = float_bucket(box->xmax, size, sorigin.x);
  state->box.ymin = float_bucket(box->ymin, size, sorigin.y);
  state->box.ymax = float_bucket(box->ymax, size, sorigin.y);
  state->box.zmin = float_bucket(box->zmin, size, sorigin.z);
  state->box.zmax = float_bucket(box->zmax, size, sorigin.z);
  if (tunits)
  {
    state->box.period.lower = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.lower), tunits, torigin));
    state->box.period.upper = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.upper), tunits, torigin));
  }
  state->box.srid = box->srid;
  state->box.flags = box->flags;
  MOBDB_FLAGS_SET_T(state->box.flags,
    MOBDB_FLAGS_GET_T(box->flags) && tunits > 0);
  state->x = state->box.xmin;
  state->y = state->box.ymin;
  state->z = state->box.zmin;
  state->t = DatumGetTimestampTz(state->box.period.lower);
  state->temp = temp;
  return state;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional grid
 *
 * @param[in] state State to increment
 */
static void
stbox_tile_state_next(STboxGridState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next cell. We need to take into account whether
   * hasz and/or hast and thus there are 4 possible cases */
  state->i++;
  state->x += state->size;
  state->coords[0]++;
  if (state->x > state->box.xmax)
  {
    state->x = state->box.xmin;
    state->coords[0] = 0;
    state->y += state->size;
    state->coords[1]++;
    if (state->y > state->box.ymax)
    {
      if (MOBDB_FLAGS_GET_Z(state->box.flags))
      {
        /* has Z */
        state->x = state->box.xmin;
        state->y = state->box.ymin;
        state->coords[0] = state->coords[1] = 0;
        state->z += state->size;
        state->coords[2]++;
        if (state->z > state->box.zmax)
        {
          if (MOBDB_FLAGS_GET_T(state->box.flags))
          {
            /* has Z and has T */
            state->x = state->box.xmin;
            state->y = state->box.ymin;
            state->z = state->box.zmin;
            state->coords[0] = state->coords[1] = state->coords[2] = 0;
            state->t += state->tunits;
            state->coords[3]++;
            if (state->t > DatumGetTimestampTz(state->box.period.upper))
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
          state->coords[0] = state->coords[1] = 0;
          state->t += state->tunits;
          state->coords[2]++;
          if (state->t > DatumGetTimestampTz(state->box.period.upper))
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

/**
 * @brief Get the current tile of the multidimensional grid
 *
 * @param[in] state State to increment
 * @param[out] box Current tile
 */
static bool
stbox_tile_state_get(STboxGridState *state, STBOX *box)
{
  if (!state || state->done)
    return false;
  /* Get the box of the current tile.
   * If there is a bit matrix for speeding up the computation, the while loop
     finds the next tile that is set in the matrix, if any */
  if (state->bm != NULL)
  {
    while (! bitmatrix_get(state->bm, state->coords))
    {
      stbox_tile_state_next(state);
      if (state->done)
        return false;
    }
  }
  bool hasz = MOBDB_FLAGS_GET_Z(state->box.flags);
  bool hast = MOBDB_FLAGS_GET_T(state->box.flags);
  stbox_tile_set(state->x, state->y, state->z, state->t, state->size,
    state->tunits, hasz, hast, state->box.srid, box);
  return true;
}

PG_FUNCTION_INFO_V1(Stbox_multidim_grid);
/**
 * @brief @ingroup mobilitydb_temporal_tile
 * @brief Generate a multidimensional grid for temporal points.
 * @sqlfunc multidimGrid()
 */
PGDLLEXPORT Datum
Stbox_multidim_grid(PG_FUNCTION_ARGS)
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
    STBOX *bounds = PG_GETARG_STBOX_P(0);
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
      tunits = get_interval_units(duration);
      sorigin = PG_GETARG_GSERIALIZED_P(3);
      torigin = PG_GETARG_TIMESTAMPTZ(4);
    }
    ensure_non_empty(sorigin);
    ensure_point_type(sorigin);
    /* Since we pass by default Point(0 0 0) as origin independently of the input
     * STBOX, we test the same spatial dimensionality only for STBOX Z */
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
      const POINT2D *p2d = gserialized_point2d_p(sorigin);
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
  STBOX *box = palloc(sizeof(STBOX));
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

PG_FUNCTION_INFO_V1(Stbox_multidim_tile);
/**
 * @ingroup mobilitydb_temporal_tile
 * @brief Generate a tile in a multidimensional grid for temporal points.
 * @sqlfunc multidimTile()
 */
PGDLLEXPORT Datum
Stbox_multidim_tile(PG_FUNCTION_ARGS)
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
    tunits = get_interval_units(duration);
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
    const POINT2D *p1 = gserialized_point2d_p(sorigin);
    pt.x = p1->x;
    pt.y = p1->y;
    memset(&ptorig, 0, sizeof(POINT3DZ));
    const POINT2D *p2 = gserialized_point2d_p(sorigin);
    ptorig.x = p2->x;
    ptorig.y = p2->y;
  }
  double xmin = float_bucket(pt.x, size, ptorig.x);
  double ymin = float_bucket(pt.y, size, ptorig.y);
  double zmin = float_bucket(pt.z, size, ptorig.z);
  TimestampTz tmin = 0; /* make compiler quiet */
  if (hast)
    tmin = timestamptz_bucket(t, tunits, torigin);
  STBOX *result = palloc0(sizeof(STBOX));
  stbox_tile_set(xmin, ymin, zmin, tmin, size, tunits, hasz, hast, srid,
    result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Transform the minimum values of a tile into matrix coordinates
 *
 * @param[in] x,y,z,t Minimum values of the tile
 * @param[in] state Grid information
 * @param[out] coords Matrix coordinates
 */
static void
tile_get_coords(int *coords, double x, double y, double z, TimestampTz t,
  const STboxGridState *state)
{
  /* Transform the minimum values of the tile into matrix coordinates */
  int k = 0;
  coords[k++] = (x - state->box.xmin) / state->size;
  coords[k++] = (y - state->box.ymin) / state->size;
  if (MOBDB_FLAGS_GET_Z(state->box.flags))
    coords[k++] = (z - state->box.zmin) / state->size;
  if (MOBDB_FLAGS_GET_T(state->box.flags))
    coords[k++] = (t - DatumGetTimestampTz(state->box.period.lower)) /
      state->tunits;
  return;
}

/**
 * @brief Get the coordinates of the tile corresponding the temporal instant point
 *
 * @param[in] inst Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] coords Tile coordinates
 */
static void
tpointinst_get_coords(int *coords, const TInstant *inst, bool hasz, bool hast,
  const STboxGridState *state)
{
  /* Read the point and compute the minimum values of the tile */
  POINT4D p;
  datum_point4d(tinstant_value(inst), &p);
  double x = float_bucket(p.x, state->size, state->box.xmin);
  double y = float_bucket(p.y, state->size, state->box.ymin);
  double z = 0;
  TimestampTz t = 0;
  if (hasz)
    z = float_bucket(p.z, state->size, state->box.zmin);
  if (hast)
    t = timestamptz_bucket(inst->t, state->tunits, state->box.ymin);
  /* Transform the minimum values of the tile into matrix coordinates */
  tile_get_coords(coords, x, y, z, t, state);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 *
 * @param[in] inst Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tpointinst_set_tiles(const TInstant *inst, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  /* Transform the point into tile coordinates */
  int coords[MAXDIMS];
  memset(coords, 0, sizeof(coords));
  tpointinst_get_coords(coords, inst, hasz, hast, state);
  /* Set the corresponding bit in the matix */
  bitmatrix_set_cell(bm, coords, true);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 *
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tdiscseq_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  /* Transform the point into tile coordinates */
  int coords[MAXDIMS];
  memset(coords, 0, sizeof(coords));
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    tpointinst_get_coords(coords, inst, hasz, hast, state);
    bitmatrix_set_cell(bm, coords, true);
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 *
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tcontseq_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  int numdims = 2 + (hasz ? 1 : 0) + (hast ? 1 : 0);
  int coords1[MAXDIMS], coords2[MAXDIMS];
  memset(coords1, 0, sizeof(coords1));
  memset(coords2, 0, sizeof(coords2));
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  tpointinst_get_coords(coords1, inst1, hasz, hast, state);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    tpointinst_get_coords(coords2, inst2, hasz, hast, state);
    bresenham_bm(bm, coords1, coords2, numdims);
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 *
 * @param[in] ss Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tpointseqset_set_tiles(const TSequenceSet *ss, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    tcontseq_set_tiles(seq, hasz, hast, state, bm);
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 *
 * @param[in] temp Temporal point
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tpoint_set_tiles(const Temporal *temp, const STboxGridState *state,
  BitMatrix *bm)
{
  bool hasz = MOBDB_FLAGS_GET_Z(state->box.flags);
  bool hast = (state->tunits > 0);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    tpointinst_set_tiles((TInstant *) temp, hasz, hast, state, bm);
  else if (temp->subtype == TSEQUENCE)
  {
    if (MOBDB_FLAGS_GET_DISCRETE(temp->flags))
      tdiscseq_set_tiles((TSequence *) temp, hasz, hast, state, bm);
    else
      tcontseq_set_tiles((TSequence *) temp, hasz, hast, state, bm);
  }
  else /* temp->subtype == TSEQUENCESET */
    tpointseqset_set_tiles((TSequenceSet *) temp, hasz, hast, state, bm);
  return;
}

/*****************************************************************************/

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
      tunits = get_interval_units(duration);
    }
    GSERIALIZED *sorigin = PG_GETARG_GSERIALIZED_P(i++);
    if (timesplit)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);
    bool bitmatrix = PG_GETARG_BOOL(i++);

    /* Set bounding box */
    STBOX bounds;
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
      const POINT2D *p2d = gserialized_point2d_p(sorigin);
      pt.x = p2d->x;
      pt.y = p2d->y;
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

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
    STBOX box;
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
    Temporal *atstbox = tpoint_at_stbox1(state->temp, &box, UPPER_EXC);
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
