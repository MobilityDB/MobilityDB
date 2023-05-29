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
#include <float.h>
#include <postgres.h>
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

/*****************************************************************************
 * Bit Matrix implementation based on
 * https://www.pvladov.com/2012/05/bit-matrix-in-c-sharp.html
 *****************************************************************************/

/**
 * @brief Create a bit matrix
 */
BitMatrix *
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
 * @brief Set in the bit matrix the bits of the tiles connecting with a line the
 * two input tiles
 *
 * @param[in] coords1, coords2 Coordinates of the input tiles
 * @param[in] eps1, eps2 Relative position of the points in the input tiles
 * @param[in] numdims Number of dimensions of the grid. It is either 2 (for 2D),
 * 3 (for 3D or 2D+T) or 4 (3D+T)
 * @param[out] bm Bit matrix
 */
static void
fastvoxel_bm(int *coords1, double *eps1, int *coords2, double *eps2,
  int numdims, BitMatrix *bm)
{
  int i, k, coords[MAXDIMS], next[MAXDIMS];
  double length, tMax[MAXDIMS], tDelta[MAXDIMS];
  /* Compute manhattan distance */
  k = 0;
  for (i = 0; i < numdims; ++i)
    k += abs(coords2[i] - coords1[i]);
  /* Shortcut function if the segement covers only 1 or 2 cells */
  if (k == 0)
  {
    bitmatrix_set_cell(bm, coords1, true);
    return;
  }
  else if (k == 1)
  {
    bitmatrix_set_cell(bm, coords1, true);
    bitmatrix_set_cell(bm, coords2, true);
    return;
  }
  /* Compute length of translation for normalization */
  length = 0;
  for (i = 0; i < numdims; ++i)
    length += pow((double) coords2[i] + eps2[i]
                - (double) coords1[i] - eps1[i], 2);
  length = sqrt(length);
  /* Initialize all vectors */
  for (i = 0; i < numdims; ++i)
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
  memcpy(coords, coords1, sizeof(int)*numdims);
  bitmatrix_set_cell(bm, coords, true);
  for (i = 0; i < k; ++i)
  {
    /* Find dimension with smallest tMax */
    int idx = 0;
    for (int j = 1; j < numdims; ++j)
    {
      if (tMax[j] < tMax[idx])
        idx = j;
    }
    /* Progress to the next cell in that dimension */
    tMax[idx] += tDelta[idx];
    coords[idx] += next[idx];
    /* Set the bitmap cell */
    bitmatrix_set_cell(bm, coords, true);
  }
  assert(memcmp(coords, coords2, sizeof(int)*numdims) == 0);
  return;
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
      false, T_TIMESTAMPTZ, &p);
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
 * @param[in] tunits Tile size for the temporal dimension in PostgreSQL time
 * units
 * @param[in] sorigin Spatial origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre The size argument must be greater to 0.
 * @note The tunits argument may be equal to 0. In that case only the spatial
 * dimension is tiled.
 */
STboxGridState *
stbox_tile_state_make(const Temporal *temp, const STBox *box, double xsize,
  double ysize, double zsize, int64 tunits, POINT3DZ sorigin,
  TimestampTz torigin)
{
  assert(MEOS_FLAGS_GET_X(box->flags) && xsize > 0 && ysize > 0);
  /* When tunits is greater than 0, verify that the box has T dimension */
  assert(tunits <= 0 || MEOS_FLAGS_GET_T(box->flags));
  /* palloc0 to initialize the missing dimensions to 0 */
  STboxGridState *state = palloc0(sizeof(STboxGridState));
  /* Fill in state */
  state->done = false;
  state->hasz = zsize && MEOS_FLAGS_GET_Z(box->flags);
  state->hast = tunits && MEOS_FLAGS_GET_T(box->flags);
  state->i = 1;
  state->xsize = xsize;
  state->ysize = ysize;
  state->zsize = zsize;
  state->tunits = tunits;
  state->box.xmin = float_bucket(box->xmin, xsize, sorigin.x);
  state->box.xmax = float_bucket(box->xmax, xsize, sorigin.x);
  state->box.ymin = float_bucket(box->ymin, ysize, sorigin.y);
  state->box.ymax = float_bucket(box->ymax, ysize, sorigin.y);
  if (state->hasz)
  {
    state->box.zmin = float_bucket(box->zmin, zsize, sorigin.z);
    state->box.zmax = float_bucket(box->zmax, zsize, sorigin.z);
  }
  if (state->hast)
  {
    state->box.period.lower = TimestampTzGetDatum(timestamptz_bucket1(
      DatumGetTimestampTz(box->period.lower), tunits, torigin));
    state->box.period.upper = TimestampTzGetDatum(timestamptz_bucket1(
      DatumGetTimestampTz(box->period.upper), tunits, torigin));
  }
  state->box.srid = box->srid;
  state->box.flags = box->flags;
  MEOS_FLAGS_SET_T(state->box.flags,
    MEOS_FLAGS_GET_T(box->flags) && tunits > 0);
  state->x = state->box.xmin;
  state->y = state->box.ymin;
  if (state->hasz)
    state->z = state->box.zmin;
  if (state->hast)
    state->t = DatumGetTimestampTz(state->box.period.lower);
  state->temp = temp;
  return state;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional grid
 * @param[in] state State to increment
 */
void
stbox_tile_state_next(STboxGridState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next cell. We need to take into account whether
   * hasz and/or hast and thus there are 4 possible cases */
  state->i++;
  state->x += state->xsize;
  state->coords[0]++;
  if (state->x > state->box.xmax)
  {
    state->x = state->box.xmin;
    state->coords[0] = 0;
    state->y += state->ysize;
    state->coords[1]++;
    if (state->y > state->box.ymax)
    {
      if (state->hasz)
      {
        /* has Z */
        state->y = state->box.ymin;
        state->coords[1] = 0;
        state->z += state->zsize;
        state->coords[2]++;
        if (state->z > state->box.zmax)
        {
          if (state->hast)
          {
            /* has Z and has T */
            state->z = state->box.zmin;
            state->coords[2] = 0;
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
        if (state->hast)
        {
          /* does not have Z and has T */
          state->y = state->box.ymin;
          state->coords[1] = 0;
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
  bool hasz = MEOS_FLAGS_GET_Z(state->box.flags);
  bool hast = MEOS_FLAGS_GET_T(state->box.flags);
  stbox_tile_set(state->x, state->y, state->z, state->t, state->xsize,
    state->ysize, state->zsize, state->tunits, hasz, hast, state->box.srid,
    box);
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_tile
 * @brief Generate a multidimensional grid for temporal points.
 * @sqlfunc multidimGrid()
 */
STBox *
stbox_tile_list(const STBox *bounds, double xsize, double ysize, double zsize,
  const Interval *duration, GSERIALIZED *sorigin, TimestampTz torigin,
  int **no_cells)
{
  /* Get input parameters */
  ensure_has_X_stbox(bounds);
  ensure_not_geodetic(bounds->flags);
  ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
  ensure_positive_datum(Float8GetDatum(ysize), T_FLOAT8);
  if (MEOS_FLAGS_GET_Z(bounds->flags))
    ensure_positive_datum(Float8GetDatum(zsize), T_FLOAT8);
  int64 tunits = 0; /* make compiler quiet */
  /* If time arguments are given */
  if (duration)
  {
    ensure_has_T_stbox(bounds);
    ensure_valid_duration(duration);
    tunits = interval_units(duration);
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
    const POINT2D *p2d = GSERIALIZED_POINT2D_P(sorigin);
    pt.x = p2d->x;
    pt.y = p2d->y;
  }

  STboxGridState *state = stbox_tile_state_make(NULL, bounds, xsize, ysize,
    zsize, tunits, pt, torigin);
  bool hasz = MEOS_FLAGS_GET_Z(state->box.flags);
  bool hast = MEOS_FLAGS_GET_T(state->box.flags);
  int *cellcount = palloc0(sizeof(int) * MAXDIMS);
  cellcount[0] = ceil((state->box.xmax - state->box.xmin) / state->xsize) + 1;
  cellcount[1] = ceil((state->box.ymax - state->box.ymin) / state->ysize) + 1;
  int count = cellcount[0] * cellcount[1];
  if (hasz)
  {
    cellcount[2] = ceil((state->box.zmax - state->box.zmin) / state->zsize) + 1;
    count *= cellcount[2];
  }
  if (hast)
  {
    cellcount[3] = ceil((DatumGetTimestampTz(state->box.period.upper) -
      DatumGetTimestampTz(state->box.period.lower)) / state->tunits) + 1;
    count *= cellcount[3];
  }
  STBox *result = palloc0(sizeof(STBox) * count);
  /* Stop when we've used up all the grid tiles */
  for (int i = 0; i < count; i++)
  {
    stbox_tile_set(state->x, state->y, state->z, state->t, state->xsize,
      state->ysize, state->zsize, state->tunits, hasz, hast, state->box.srid,
      &result[i]);
    stbox_tile_state_next(state);
  }
  *no_cells = cellcount;
  return result;
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
 * @param[out] eps Relative position of values in their matrix cells (between
 * 0 and 1)
 */
static void
tile_get_eps(double dx, double dy, double dz, TimestampTz dt,
  const STboxGridState *state, double *eps)
{
  /* Transform the values in a tile into relative positions in matrix cells */
  int k = 0;
  eps[k++] = dx / state->xsize;
  eps[k++] = dy / state->ysize;
  if (MEOS_FLAGS_GET_Z(state->box.flags))
    eps[k++] = dz / state->zsize;
  if (MEOS_FLAGS_GET_T(state->box.flags))
    eps[k++] = (double) dt / state->tunits;
  return;
}

/**
 * @brief Get the coordinates of the tile corresponding the temporal instant point
 * @param[in] inst Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] coords Tile coordinates
 * @param[out] eps    Relative position inside the tile
 */
static void
tpointinst_get_coords_eps(const TInstant *inst, bool hasz, bool hast,
  const STboxGridState *state, int *coords, double *eps)
{
  /* Read the point and compute the minimum values of the tile */
  POINT4D p;
  datum_point4d(tinstant_value(inst), &p);
  double x = float_bucket(p.x, state->xsize, state->box.xmin);
  double y = float_bucket(p.y, state->ysize, state->box.ymin);
  double z = 0;
  TimestampTz t = 0;
  if (hasz)
    z = float_bucket(p.z, state->zsize, state->box.zmin);
  if (hast)
    t = timestamptz_bucket1(inst->t, state->tunits,
      DatumGetTimestampTz(state->box.period.lower));
  /* Transform the minimum values of the tile into matrix coordinates */
  tile_get_coords(x, y, z, t, state, coords);
  /* Transform the values in a tile into relative positions in matrix cells */
  if (eps != NULL) /* Some methods do not need this information */
    tile_get_eps(p.x - x, p.y - y, p.z - z, inst->t - t, state, eps);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
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
  tpointinst_get_coords_eps(inst, hasz, hast, state, coords, NULL);
  /* Set the corresponding bit in the matix */
  bitmatrix_set_cell(bm, coords, true);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
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
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    tpointinst_get_coords_eps(inst, hasz, hast, state, coords, NULL);
    bitmatrix_set_cell(bm, coords, true);
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
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
  double eps1[MAXDIMS], eps2[MAXDIMS];
  memset(coords1, 0, sizeof(coords1));
  memset(coords2, 0, sizeof(coords2));
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  tpointinst_get_coords_eps(inst1, hasz, hast, state, coords1, eps1);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    tpointinst_get_coords_eps(inst2, hasz, hast, state, coords2, eps2);
    fastvoxel_bm(coords1, eps1, coords2, eps2, numdims, bm);
    memcpy(coords1, coords2, sizeof(coords1));
    memcpy(eps1, eps2, sizeof(eps1));
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 * @param[in] seq Temporal point
 * @param[in] hasz Whether the tile has Z dimension
 * @param[in] hast Whether the tile has T dimension
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
static void
tpointseq_set_tiles(const TSequence *seq, bool hasz, bool hast,
  const STboxGridState *state, BitMatrix *bm)
{
  if (MEOS_FLAGS_GET_LINEAR(seq->flags))
    tcontseq_set_tiles((TSequence *) seq, hasz, hast, state, bm);
  else
    tdiscseq_set_tiles((TSequence *) seq, hasz, hast, state, bm);
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    tpointseq_set_tiles(seq, hasz, hast, state, bm);
  }
  return;
}

/**
 * @brief Set the bit corresponding to the tiles intersecting the temporal point
 * @param[in] temp Temporal point
 * @param[in] state Grid definition
 * @param[out] bm Bit matrix
 */
void
tpoint_set_tiles(const Temporal *temp, const STboxGridState *state,
  BitMatrix *bm)
{
  bool hasz = MEOS_FLAGS_GET_Z(state->box.flags);
  bool hast = (state->tunits > 0);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    tpointinst_set_tiles((TInstant *) temp, hasz, hast, state, bm);
  else if (temp->subtype == TSEQUENCE)
    tpointseq_set_tiles((TSequence *) temp, hasz, hast, state, bm);
  else /* temp->subtype == TSEQUENCESET */
    tpointseqset_set_tiles((TSequenceSet *) temp, hasz, hast, state, bm);
  return;
}

/*****************************************************************************/
