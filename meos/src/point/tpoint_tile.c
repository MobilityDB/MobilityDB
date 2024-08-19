/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
#include <float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/temporal_tile.h"
#include "point/stbox.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tile.h"

/*****************************************************************************
 * Bit Matrix implementation based on
 * https://www.pvladov.com/2012/05/bit-matrix-in-c-sharp.html
 *****************************************************************************/

/**
 * @brief Create a bit matrix
 */
BitMatrix *
bitmatrix_make(int *count, int ndims)
{
  /* Calculate the needed number of bits and bytes */
  int i, bitCount = 1;
  for (i = 0; i < ndims; i++)
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
  result->ndims = ndims;
  for (i = 0; i < ndims; i++)
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
  for (i = 0; i < bm->ndims; i++)
    assert(coords[i] <= bm->count[i]);
  int pos = 0;
  for (i = 0; i < bm->ndims; i++)
  {
    int offset = coords[i];
    for (j = i + 1; j < bm->ndims; j++)
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
  for (i = 0; i < bm->ndims; i++)
    assert(coords[i] <= bm->count[i]);
  for (i = 0; i < bm->ndims - 1; i++)
  {
    int offset = coords[i];
    for (j = i + 1; j < bm->ndims; j++)
      offset *= bm->count[j];
    pos += offset;
  }
  pos += coords[bm->ndims - 1];
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
  int dim = bm->ndims - 2;
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
 * @note This function is only used for debugging purposes
 */
void
bitmatrix_print(const BitMatrix *bm)
{
  int i, j, k, l, coords[MAXDIMS];
  memset(&coords, 0, sizeof(coords));
  int totalcount = 1;
  for (i = 0; i < bm->ndims; i++)
    totalcount *= bm->count[i];
  int count = 0;
  for (i = 0; i < bm->count[0]; i++)
  {
    coords[0] = i;
    for (j = 0; j < bm->count[1]; j++)
    {
      coords[1] = j;
      if (bm->ndims >= 3)
      {
        for (k = 0; k < bm->count[2]; k++)
        {
          coords[2] = k;
          if (bm->ndims >= 4)
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
  return;
}
#endif /* DEBUG_BUILD */

/*****************************************************************************
 * N-dimensional version of the fast voxel traversal algorithm
 * setting in the bit array all the tiles connecting the two given tiles.
 *
 * Amanatides, John, and Andrew Woo.
 * "A fast voxel traversal algorithm for ray tracing."
 * Eurographics. Vol. 87. No. 3. 1987.
 *****************************************************************************/

/**
 * @brief Set in the bit matrix the bits of the tiles connecting with a line
 * two input tiles
 * @param[in] coords1, coords2 Coordinates of the input tiles
 * @param[in] eps1, eps2 Fractional position of the points in the input tiles
 * @param[in] ndims Number of dimensions of the grid. It is either 2 (for 2D),
 * 3 (for 3D or 2D+T) or 4 (3D+T)
 * @param[out] bm Bit matrix
 * @return Number of tiles set
 */
static int
fastvoxel_bm(int *coords1, double *eps1, int *coords2, double *eps2,
  int ndims, BitMatrix *bm)
{
  int i, k, coords[MAXDIMS], next[MAXDIMS], result = 0;
  double length, tMax[MAXDIMS], tDelta[MAXDIMS];
  /* Compute Manhattan distance */
  k = 0;
  for (i = 0; i < ndims; ++i)
    k += abs(coords2[i] - coords1[i]);
  /* Shortcut function if the segment covers only 1 or 2 cells */
  if (k == 0)
  {
    bitmatrix_set_cell(bm, coords1, true);
    result++;
    return result;
  }
  else if (k == 1)
  {
    bitmatrix_set_cell(bm, coords1, true);
    bitmatrix_set_cell(bm, coords2, true);
    result += 2;
    return result;
  }
  /* Compute length of translation for normalization */
  length = 0;
  for (i = 0; i < ndims; ++i)
    length += pow((double) coords2[i] + eps2[i]
                - (double) coords1[i] - eps1[i], 2);
  length = sqrt(length);
  /* Initialize all vectors */
  for (i = 0; i < ndims; ++i)
  {
    /* Compute the (normalized) tDelta */
    tDelta[i] = length / fabs((double) coords2[i] + eps2[i]
                           - (double) coords1[i] - eps1[i]);
    /* Compute the direction of movement and tMax */
    if (coords2[i] > coords1[i])
    {
      next[i] = 1;
      tMax[i] = (1 - eps1[i]) * tDelta[i];
    }
    else if (coords2[i] < coords1[i])
    {
      next[i] = -1;
      tMax[i] = eps1[i] * tDelta[i];
    }
    else
    {
      next[i] = 0;
      tMax[i] = DBL_MAX;
    }
  }
  /* Set the starting bitmap cell */
  memcpy(coords, coords1, sizeof(int) * ndims);
  bitmatrix_set_cell(bm, coords, true);
  result++;
  for (i = 0; i < k; ++i)
  {
    /* Find dimension with smallest tMax */
    int idx = 0;
    for (int j = 1; j < ndims; ++j)
    {
      if (tMax[j] < tMax[idx])
        idx = j;
    }
    /* Progress to the next cell in that dimension */
    tMax[idx] += tDelta[idx];
    coords[idx] += next[idx];
    /* Set the bitmap cell */
    bitmatrix_set_cell(bm, coords, true);
    result++;
  }
  assert(memcmp(coords, coords2, sizeof(int) * ndims) == 0);
  return result;
}

/*****************************************************************************
 * Grid functions
 *****************************************************************************/

/**
 * @brief Generate a tile from the current state of the multidimensional grid
 * @param[in] x,y,z,t Lower coordinates of the tile to output
 * @param[in] xsize,ysize,zsize Tile sizes for the spatial dimensions in the
 * units of the SRID
 * @param[in] tunits Tile size for the temporal dimension in PostgreSQL time
 * units
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] srid SRID of the spatial coordinates
 * @param[out] result Box representing the tile
 */
void
stbox_tile_set(double x, double y, double z, TimestampTz t, double xsize,
  double ysize, double zsize, int64 tunits, bool hasz, bool hast, int32 srid,
  STBox *result)
{
  double xmin = x;
  double xmax = xmin + xsize;
  double ymin = y;
  double ymax = ymin + ysize;
  double zmin = 0, zmax = 0;
  Span p;
  if (hasz)
  {
    zmin = z;
    zmax = zmin + zsize;
  }
  if (hast)
  {
    span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t + tunits), true,
      false, T_TIMESTAMPTZ, T_TSTZSPAN, &p);
  }
  stbox_set(true, hasz, false, srid, xmin, xmax, ymin, ymax, zmin, zmax,
    hast ? &p : NULL, result);
  return;
}

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] temp Optional temporal point to split
 * @param[in] box Bounds for generating the multidimensional grid
 * @param[in] xsize,ysize,zsize Tile sizes for the spatial dimensions in the
 * units of the SRID
 * @param[in] duration Tile size for the temporal dimension as an interval.
 * When @p NULL, only the spatial dimension is tiled.
 * @param[in] sorigin Spatial origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 */
STboxGridState *
stbox_tile_state_make(const Temporal *temp, const STBox *box, double xsize,
  double ysize, double zsize, const Interval *duration, POINT3DZ sorigin,
  TimestampTz torigin, bool border_inc)
{
  assert(box);
  assert(MEOS_FLAGS_GET_X(box->flags) && xsize > 0 && ysize > 0);
  /* When zsize is greater than 0, the box must have Z dimension */
  assert(zsize <= 0 || MEOS_FLAGS_GET_Z(box->flags));
  /* When the interval is not NULL, the box must have T dimension */
  assert(! duration || MEOS_FLAGS_GET_T(box->flags));
  /* palloc0 to initialize the missing dimensions to 0 */
  STboxGridState *state = palloc0(sizeof(STboxGridState));
  /* Fill in state */
  state->i = 1;
  state->ntiles = 1;
  state->xsize = xsize;
  state->ysize = ysize;
  state->box.xmin = float_get_bin(box->xmin, xsize, sorigin.x);
  state->box.xmax = float_get_bin(box->xmax, xsize, sorigin.x);
  state->max_coords[0] = ceil((state->box.xmax - state->box.xmin) / xsize);
  if (border_inc)
    state->max_coords[0] += 1;
  state->ntiles *= (state->max_coords[0] + 1);
  state->box.ymin = float_get_bin(box->ymin, ysize, sorigin.y);
  state->box.ymax = float_get_bin(box->ymax, ysize, sorigin.y);
  state->max_coords[1] = ceil((state->box.ymax - state->box.ymin) / ysize);
  if (border_inc)
    state->max_coords[1] += 1;
  state->ntiles *= (state->max_coords[1] + 1);
  state->box.srid = box->srid;
  state->box.flags = box->flags;
  state->x = state->box.xmin;
  state->y = state->box.ymin;
  /* Notice that the dimensions are "compacted", that is, for both XYZ and XYT
   * the last dimension Z or T are in coords[2] */
  int dim = 2;
  if (MEOS_FLAGS_GET_Z(box->flags))
  {
    if (zsize > 0)
    {
      state->hasz = true;
      state->zsize = zsize;
      state->box.zmin = float_get_bin(box->zmin, zsize, sorigin.z);
      state->box.zmax = float_get_bin(box->zmax, zsize, sorigin.z);
      state->max_coords[dim] = ceil((state->box.zmax - state->box.zmin) / zsize);
      if (border_inc)
        state->max_coords[dim] += 1;
      state->ntiles *= (state->max_coords[dim] + 1);
      state->z = state->box.zmin;
      dim++;
    }
    else
      MEOS_FLAGS_SET_Z(state->box.flags, false);
  }
  if (MEOS_FLAGS_GET_T(box->flags))
  {
    if (duration)
    {
      state->hast = true;
      state->tunits = interval_units(duration);
      state->box.period.lower = TimestampTzGetDatum(timestamptz_get_bin1(
        DatumGetTimestampTz(box->period.lower), state->tunits, torigin));
      state->box.period.upper = TimestampTzGetDatum(timestamptz_get_bin1(
        DatumGetTimestampTz(box->period.upper), state->tunits, torigin));
      state->max_coords[dim] =
        ceil((state->box.period.upper - state->box.period.lower) / 
          state->tunits);
      if (border_inc)
        state->max_coords[dim] += 1;
      state->ntiles *= (state->max_coords[dim] + 1);
      state->t = DatumGetTimestampTz(state->box.period.lower);
    }
    else
      MEOS_FLAGS_SET_T(state->box.flags, false);
  }
  state->temp = temp;
  return state;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional
 * grid
 * @param[in] state State to increment
 */
void
stbox_tile_state_next(STboxGridState *state)
{
  if (! state || state->done)
    return;
  /* Move to the next cell. We need to take into account whether hasz and/or
   * hast and thus there are 4 possible cases.
   * Notice that the dimensions are "compacted", that is, for both XYZ and XYT
   * the last dimension Z or T are in coords[2] */
  state->i++;
  state->x += state->xsize;
  state->coords[0]++;
  if (state->coords[0] >= state->max_coords[0])
  {
    state->x = state->box.xmin;
    state->coords[0] = 0;
    state->y += state->ysize;
    state->coords[1]++;
    if (state->coords[1] >= state->max_coords[1])
    {
      if (state->hasz)
      {
        /* has Z */
        state->y = state->box.ymin;
        state->coords[1] = 0;
        state->z += state->zsize;
        state->coords[2]++;
        if (state->coords[2] >= state->max_coords[2])
        {
          if (state->hast)
          {
            /* has Z and has T */
            state->z = state->box.zmin;
            state->coords[2] = 0;
            state->t += state->tunits;
            state->coords[3]++;
            if (state->coords[3] >= state->max_coords[3])
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
        if (state->hast)
        {
          /* does not have Z and has T */
          state->y = state->box.ymin;
          state->coords[1] = 0;
          state->t += state->tunits;
          state->coords[2]++;
          if (state->coords[2] >= state->max_coords[2])
          {
            state->done = true;
            return;
          }
        }
        else
        {
          /* does not have Z and does not have T */
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
 * @param[in] state State to increment
 * @param[out] box Current tile
 */
bool
stbox_tile_state_get(STboxGridState *state, STBox *box)
{
  if (! state || state->done)
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
  stbox_tile_set(state->x, state->y, state->z, state->t, state->xsize,
    state->ysize, state->zsize, state->tunits, state->hasz, state->hast,
    state->box.srid, box);
  return true;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the multidimensional grid of a spatiotemporal box
 * @param[in] bounds Bounds
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] count Number of values in the output array
 * @csqlfn #Stbox_space_time_tiles()
 */
STBox *
stbox_space_time_tiles(const STBox *bounds, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin,
  TimestampTz torigin, bool border_inc, int *count)
{
  /* Ensure validity of the arguments
   * Since we pass by default Point(0 0 0) as origin independently of the input
   * STBox, we test the same spatial dimensionality only for STBox Z */
  if (! ensure_not_null((void *) bounds) || ! ensure_has_X_stbox(bounds) ||
      ! ensure_not_geodetic(bounds->flags) ||
      ! ensure_not_null((void *) count) ||
      ! ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8) ||
      ! ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8) ||
      ! ensure_not_empty(sorigin) || ! ensure_point_type(sorigin) ||
      (MEOS_FLAGS_GET_Z(bounds->flags) &&
        (! ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8) ||
         ! ensure_same_spatial_dimensionality_stbox_gs(bounds, sorigin))) ||
      (duration &&
        (! ensure_has_T_stbox(bounds) || ! ensure_valid_duration(duration))))
      return NULL;
  int32 srid = bounds->srid;
  int32 gs_srid = gserialized_get_srid(sorigin);
  if (gs_srid != SRID_UNKNOWN && ! ensure_same_srid(srid, gs_srid))
    return NULL;

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
    const POINT2D *p2d = GSERIALIZED_POINT2D_P(sorigin);
    pt.x = p2d->x;
    pt.y = p2d->y;
    /* Since when zsize is not given we pass by default xsize, if the box does
     * not have Z dimension we set zsize to 0 */
    zsize = 0;
  }

  STboxGridState *state = stbox_tile_state_make(NULL, bounds, xsize, ysize,
    zsize, duration, pt, torigin, border_inc);
  bool hasz = MEOS_FLAGS_GET_Z(state->box.flags);
  bool hast = MEOS_FLAGS_GET_T(state->box.flags);
  int cellcount[MAXDIMS];
  cellcount[0] = ceil((state->box.xmax - state->box.xmin) / state->xsize) + 1;
  cellcount[1] = ceil((state->box.ymax - state->box.ymin) / state->ysize) + 1;
  int count1 = cellcount[0] * cellcount[1];
  if (hasz)
  {
    cellcount[2] = ceil((state->box.zmax - state->box.zmin) / state->zsize) + 1;
    count1 *= cellcount[2];
  }
  if (hast)
  {
    TimestampTz duration1 = (DatumGetTimestampTz(state->box.period.upper) -
      DatumGetTimestampTz(state->box.period.lower));
    cellcount[3] = ceil((double) duration1 / state->tunits) + 1;
    count1 *= cellcount[3];
  }
  STBox *result = palloc0(sizeof(STBox) * count1);
  /* Stop when we've used up all the grid tiles */
  for (int i = 0; i < count1; i++)
  {
    stbox_tile_set(state->x, state->y, state->z, state->t, state->xsize,
      state->ysize, state->zsize, state->tunits, hasz, hast, state->box.srid,
      &result[i]);
    stbox_tile_state_next(state);
  }
  *count = count1;
  if (state->bm)
    pfree(state->bm);
  pfree(state);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the multidimensional grid of a spatiotemporal box
 * @param[in] bounds Bounds
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] sorigin Origin for the space dimension
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] count Number of values in the output array
 * @csqlfn #Stbox_space_tiles()
 */
STBox *
stbox_space_tiles(const STBox *bounds, double xsize, double ysize, double zsize,
  const GSERIALIZED *sorigin,  bool border_inc, int *count)
{
  return stbox_space_time_tiles(bounds, xsize, ysize, zsize, NULL, sorigin, 0,
    border_inc, count);
}
#endif /* MEOS */

/**
 * @brief Return a tile in the multidimensional grid of a spatiotemporal box
 * @param[in] point Point
 * @param[in] t Timestamp
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @param[out] hast True when spliting by time
 */
STBox *
stbox_space_time_tile_common(const GSERIALIZED *point, TimestampTz t,
  double xsize, double ysize, double zsize, const Interval *duration,
  const GSERIALIZED *sorigin, TimestampTz torigin, bool hast)
{
  /* Ensure parameter validity */
  if (! ensure_not_empty(point) || ! ensure_point_type(point) ||
      ! ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8) ||
      ! ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8) ||
      ! ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8) ||
      ! ensure_not_empty(sorigin) || ! ensure_point_type(sorigin) ||
      (hast && ! ensure_valid_duration(duration)))
    return NULL;

  int64 tunits = 0; /* make compiler quiet */
  if (hast)
    tunits = interval_units(duration);
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
  double xmin = float_get_bin(pt.x, xsize, ptorig.x);
  double ymin = float_get_bin(pt.y, ysize, ptorig.y);
  double zmin = float_get_bin(pt.z, zsize, ptorig.z);
  TimestampTz tmin = 0; /* make compiler quiet */
  if (hast)
    tmin = timestamptz_get_bin1(t, tunits, torigin);
  STBox *result = palloc0(sizeof(STBox));
  stbox_tile_set(xmin, ymin, zmin, tmin, xsize, ysize, zsize, tunits, hasz,
    hast, srid, result);
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return a tile in the multidimensional grid of a spatiotemporal box
 * @param[in] point Point
 * @param[in] t Timestamp
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @csqlfn Stbox_space_time_tile()
 */
STBox *
stbox_space_time_tile(const GSERIALIZED *point, TimestampTz t, double xsize,
  double ysize, double zsize, const Interval *duration,
  const GSERIALIZED *sorigin, TimestampTz torigin)
{
  return stbox_space_time_tile_common(point, t, xsize, ysize, zsize, duration,
    sorigin, torigin, true);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return a tile in the multidimensional grid of a spatiotemporal box
 * @param[in] point Point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] sorigin Origin for the space dimension
 * @csqlfn Stbox_space_tile()
 */
STBox *
stbox_space_tile(const GSERIALIZED *point, double xsize, double ysize,
  double zsize, const GSERIALIZED *sorigin)
{
  return stbox_space_time_tile_common(point, 0, xsize, ysize, zsize, NULL,
    sorigin, 0, false);
}
#endif /* MEOS */

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Return the spatiotemporal boxes of a temporal point split with
 * respect to a space and possibly a time grid
 * @param[in] temp Temporal point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tpoint_space_time_boxes_ext
 */
STBox *
tpoint_space_time_boxes(const Temporal *temp, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin, 
  TimestampTz torigin, bool bitmatrix, bool border_inc, int *count) 
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) || 
      ! ensure_not_null((void *) sorigin) || 
      ! ensure_positive_datum(xsize, T_FLOAT8) ||
      ! ensure_positive_datum(ysize, T_FLOAT8) ||
      (MEOS_FLAGS_GET_Z(temp->flags) && 
        ! ensure_positive_datum(zsize, T_FLOAT8)) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Initialize state */
  int ntiles;
  STboxGridState *state = tpoint_space_time_tile_init(temp, xsize, ysize, 
    zsize, duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
  if (! state)
    return NULL;

  /* We need to loop since atStbox may be NULL */
  int i = 0;
  STBox *result = palloc(sizeof(STBox) * ntiles);
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      break;
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
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

    /* Restrict the temporal point to the box and compute its bounding box */
    Temporal *atstbox = tpoint_restrict_stbox(state->temp, &box, BORDER_EXC, 
      REST_AT);
    if (atstbox == NULL)
      continue;
    tspatial_set_stbox(atstbox, &box);
    /* If only space grid */
    // if (! duration)
      // MEOS_FLAGS_SET_T(box.flags, false);
    pfree(atstbox);

    /* Copy the box to the result */
    memcpy(&result[i++], &box, sizeof(STBox));
  }
  *count = i;
  return result;
}

#if MEOS
/**
 * @brief Return the spatiotemporal boxes of a temporal point split with
 * respect to a space grid
 * @param[in] temp Temporal point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] sorigin Origin for the space dimension
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] count Number of elements in the output array
 */
STBox *
tpoint_space_boxes(const Temporal *temp, double xsize, double ysize, 
  double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc, 
  int *count)
{
  return tpoint_space_time_boxes(temp, xsize, ysize, zsize, NULL, sorigin, 0,
    bitmatrix, border_inc, count);
}
#endif /* MEOS */

/*****************************************************************************
 * Split functions
 *****************************************************************************/

/**
 * @brief Transform the minimum values of a tile into matrix coordinates
 * @param[in] x,y,z,t Minimum values of the tile
 * @param[in] state Grid information
 * @param[out] coords Matrix coordinates
 */
static void
tile_get_coords(double x, double y, double z, TimestampTz t,
  const STboxGridState *state, int *coords)
{
  /* Transform the minimum values of the tile into matrix coordinates */
  int ncoords = 0;
  coords[ncoords++] = (int) ((x - state->box.xmin) / state->xsize);
  coords[ncoords++] = (int) ((y - state->box.ymin) / state->ysize);
  if (MEOS_FLAGS_GET_Z(state->box.flags))
    coords[ncoords++] = (int) ((z - state->box.zmin) / state->zsize);
  if (MEOS_FLAGS_GET_T(state->box.flags))
    coords[ncoords++] = (int) ((t - DatumGetTimestampTz(state->box.period.lower)) /
      state->tunits);
  return;
}

/**
 * @brief Transform the values in a tile into relative positions in matrix cells
 * @param[in] dx,dy,dz,dt Values in a tile
 * @param[in] state Grid information
 * @param[out] fpos Fractional position of values in their matrix cells (between
 * 0 and 1)
 */
static void
tile_get_fpos(double dx, double dy, double dz, TimestampTz dt,
  const STboxGridState *state, double *fpos)
{
  /* Transform the values in a tile into relative positions in matrix cells */
  int k = 0;
  fpos[k++] = dx / state->xsize;
  fpos[k++] = dy / state->ysize;
  if (MEOS_FLAGS_GET_Z(state->box.flags))
    fpos[k++] = dz / state->zsize;
  if (MEOS_FLAGS_GET_T(state->box.flags))
    fpos[k++] = (double) dt / state->tunits;
  return;
}

/**
 * @brief Get the coordinates of the tile corresponding the temporal point
 * instant
 * @param[in] inst Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] coords Tile coordinates
 * @param[out] fpos Fractional position inside the tile
 */
static void
tpointinst_get_coords_fpos(const TInstant *inst, bool hasz, bool hast,
  const STboxGridState *state, int *coords, double *fpos)
{
  /* Read the point and compute the minimum values of the tile */
  POINT4D p;
  datum_point4d(tinstant_val(inst), &p);
  double x = float_get_bin(p.x, state->xsize, state->box.xmin);
  double y = float_get_bin(p.y, state->ysize, state->box.ymin);
  double z = 0;
  TimestampTz t = 0;
  if (hasz)
    z = float_get_bin(p.z, state->zsize, state->box.zmin);
  if (hast)
    t = timestamptz_get_bin1(inst->t, state->tunits,
      DatumGetTimestampTz(state->box.period.lower));
  /* Transform the minimum values of the tile into matrix coordinates */
  tile_get_coords(x, y, z, t, state, coords);
  /* Transform the values in a tile into relative positions in matrix cells */
  if (fpos != NULL) /* Some methods do not need this information */
    tile_get_fpos(p.x - x, p.y - y, p.z - z, inst->t - t, state, fpos);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting a temporal point
 * sequence
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static int
tpointseq_disc_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  /* Transform the point into tile coordinates */
  int coords[MAXDIMS], result = 0;
  memset(coords, 0, sizeof(coords));
  for (int i = 0; i < seq->count; i++)
  {
    tpointinst_get_coords_fpos(TSEQUENCE_INST_N(seq, i), hasz, hast, state,
      coords, NULL);
    bitmatrix_set_cell(bm, coords, true);
    result++;
  }
  return result;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal
 * point sequence
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static int
tpointseq_cont_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  int ndims = 2 + (hasz ? 1 : 0) + (hast ? 1 : 0);
  int coords1[MAXDIMS], coords2[MAXDIMS], result = 0;
  double eps1[MAXDIMS], eps2[MAXDIMS];
  memset(coords1, 0, sizeof(coords1));
  memset(coords2, 0, sizeof(coords2));
  tpointinst_get_coords_fpos(TSEQUENCE_INST_N(seq, 0), hasz, hast, state,
    coords1, eps1);
  for (int i = 1; i < seq->count; i++)
  {
    tpointinst_get_coords_fpos(TSEQUENCE_INST_N(seq, i), hasz, hast, state,
      coords2, eps2);
    result += fastvoxel_bm(coords1, eps1, coords2, eps2, ndims, bm);
    memcpy(coords1, coords2, sizeof(coords1));
    memcpy(eps1, eps2, sizeof(eps1));
  }
  return result;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal
 * point sequence
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static int
tpointseq_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  return MEOS_FLAGS_LINEAR_INTERP(seq->flags) ?
    tpointseq_cont_set_tiles((TSequence *) seq, hasz, hast, state, bm) :
    tpointseq_disc_set_tiles((TSequence *) seq, hasz, hast, state, bm);
}

/**
 * @brief Set the bit corresponding to the tiles intersecting a temporal point
 * sequence set
 * @param[in] ss Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static int
tpointseqset_set_tiles(const TSequenceSet *ss, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  int result = 0;
  for (int i = 0; i < ss->count; i++)
    result += tpointseq_set_tiles(TSEQUENCESET_SEQ_N(ss, i), hasz, hast, state,
      bm);
  return result;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting a temporal point
 * @param[in] temp Temporal point
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 * @return Number of tiles set
 */
int
tpoint_set_tiles(const Temporal *temp, const STboxGridState *state,
  BitMatrix *bm)
{
  /* The usage of bitmatrix is disallowed for instantaneous temporal values */
  assert(temporal_num_instants(temp) > 1);
  bool hasz = MEOS_FLAGS_GET_Z(state->box.flags);
  bool hast = (state->tunits > 0);
  assert(temp->subtype == TSEQUENCE || temp->subtype == TSEQUENCESET);
  if (temp->subtype == TSEQUENCE)
    return tpointseq_set_tiles((TSequence *) temp, hasz, hast, state, bm);
  else
    return tpointseqset_set_tiles((TSequenceSet *) temp, hasz, hast, state, bm);
}

/*****************************************************************************/

/**
 * @brief Set the state with a temporal point and a space and possibly time 
 * grid for splitting or obtaining a set of spatiotemporal boxes
 * @param[in] temp Temporal point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] ntiles Number of tiles
 */
STboxGridState *
tpoint_space_time_tile_init(const Temporal *temp, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin,
  TimestampTz torigin, bool bitmatrix, bool border_inc, int *ntiles)
{
  /* Ensure parameter validity */
  if (! ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8) ||
      ! ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8) ||
      ! ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8) ||
      ! ensure_not_empty(sorigin) || ! ensure_point_type(sorigin) ||
      ! ensure_same_geodetic(temp->flags, sorigin->gflags))
    return NULL;

  /* Set bounding box */
  STBox bounds;
  temporal_set_bbox(temp, &bounds);
  int32 srid = bounds.srid;
  int32 gs_srid = gserialized_get_srid(sorigin);
  if (gs_srid != SRID_UNKNOWN && ! ensure_same_srid(srid, gs_srid))
    return NULL;

  /* Disable the usage of bitmatrix for instantaneous temporal values */
  if (temporal_num_instants(temp) == 1)
      bitmatrix = false;

  POINT3DZ pt;
  memset(&pt, 0, sizeof(POINT3DZ));
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
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
    /* Since when zsize is not given we pass by default xsize, if temp does
     * not have Z dimension we set zsize to 0 */
    zsize = 0;
  }

  bool timesplit = (duration != NULL);
  if (timesplit)
    ensure_valid_duration(duration);
  else
    /* Disallow T dimension for generating a spatial only grid */
    MEOS_FLAGS_SET_T(bounds.flags, false);

  /* Create function state */
  STboxGridState *state = stbox_tile_state_make(temp, &bounds, xsize, ysize,
    zsize, duration, pt, torigin, border_inc);
  /* If a bit matrix is used to speed up the process */
  if (bitmatrix)
  {
    /* Create the bit matrix and set the tiles traversed by the temporal point */
    int ndims = 2 + (hasz ? 1 : 0) + (timesplit ? 1 : 0);
    state->bm = bitmatrix_make(state->max_coords, ndims);
    *ntiles = tpoint_set_tiles(temp, state, state->bm);
  }
  else
    *ntiles = state->ntiles;
  return state;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments a temporal point split according to a space and
 * possibly a time grid
 * @param[in] temp Temporal point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] sorigin Origin for the space dimension
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] space_bins Array of space bins
 * @param[out] count Number of elements in the output arrays
 */
Temporal **
tpoint_space_split(const Temporal *temp, double xsize, double ysize, 
  double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc,
  GSERIALIZED ***space_bins, int *count)
{
  return tpoint_space_time_split(temp, xsize, ysize, zsize, NULL, sorigin, 0,
    bitmatrix, border_inc, space_bins, NULL, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments a temporal point split according to a space and
 * possibly a time grid
 * @param[in] temp Temporal point
 * @param[in] xsize,ysize,zsize Size of the corresponding dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] sorigin Origin for the space dimension
 * @param[in] torigin Origin for the time dimension
 * @param[in] bitmatrix True when using a bitmatrix to speed up the computation
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @param[out] space_bins Array of space bins
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of elements in the output arrays
 * @note This function in MEOS corresponds to the MobilityDB function
 * #Tpoint_space_time_split_ext. Note that the test for the validity of the 
 * arguments is done in #tpoint_space_time_tile_init
 */
Temporal **
tpoint_space_time_split(const Temporal *temp, double xsize, double ysize,
  double zsize, const Interval *duration, const GSERIALIZED *sorigin, 
  TimestampTz torigin, bool bitmatrix, bool border_inc, 
  GSERIALIZED ***space_bins, TimestampTz **time_bins, int *count)
{
  /* Initialize state */
  int ntiles;
  STboxGridState *state = tpoint_space_time_tile_init(temp, xsize, ysize,
    zsize, duration, sorigin, torigin, bitmatrix, border_inc, &ntiles);
  if (! state)
    return NULL;

  GSERIALIZED **spaces = palloc(sizeof(GSERIALIZED *) * ntiles);
  TimestampTz *times = NULL;
  if (duration)
    times = palloc(sizeof(TimestampTz) * ntiles);
  Temporal **result = palloc(sizeof(Temporal *) * ntiles);
  bool hasz = MEOS_FLAGS_GET_Z(state->temp->flags);
  int i = 0;
  /* We need to loop since atStbox may be NULL */
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      if (state->bm)
        pfree(state->bm);
      pfree(state);
      break;
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
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

    /* Restrict the temporal point to the box */
    Temporal *atstbox = tpoint_restrict_stbox(state->temp, &box, BORDER_EXC,
      REST_AT);
    if (atstbox == NULL)
      continue;

    /* Construct value of the result */
    spaces[i] = geopoint_make(box.xmin, box.ymin, box.zmin, hasz, false,
      box.srid);
    if (duration)
      times[i] = DatumGetTimestampTz(box.period.lower);
    result[i++] = atstbox;
  }
  *count = i;
  if (space_bins)
    *space_bins = spaces;
  if (time_bins)
    *time_bins = times;
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
