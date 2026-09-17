/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief First-party implementation of the Google S2 cell algebra.
 *
 * S2 is a quadtree Discrete Global Grid System on the sphere. This file is the
 * MobilityDB-owned counterpart of libh3: it provides the static-cell kernel
 * that the temporal `ts2cell` layer lifts over time. It links only libm.
 *
 * The bit layout and the integer functions are transcribed from the public S2
 * cell-id specification, reference implementation `s2geometry`
 * (https://github.com/google/s2geometry), released under the Apache-2.0
 * license, Copyright Google Inc. This is a first-party re-implementation from
 * that public specification — not a vendored copy — so it carries no vendor
 * split and no .codacy.yml exclusion. In particular it reproduces the Hilbert
 * curve from its four-entry orientation tables rather than from the 1024-entry
 * lookup tables the C++ library precomputes.
 *
 * Cell layout (most-significant bit first):
 *   bits 61..63 : cube face (0..5)
 *   bits 1..60  : position along the Hilbert curve, two bits per level
 *   bit  0..    : a single trailing set bit at position `2 * (30 - level)`
 *                 that records the level of the cell
 *
 * Five coordinate systems connect a cell id to a geodetic position:
 *   (lon,lat) <-> (x,y,z) on the unit sphere <-> (face,u,v) on the cube
 *             <-> (face,s,t) in the unit square <-> (face,i,j) leaf indices
 *             <-> cell id along the Hilbert curve
 */

#include "s2cell/s2cell.h"

/* C */
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include <pg_int.h>
#include <port/pg_bitutils.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/tcellindex.h"

/*****************************************************************************
 * Hilbert curve orientation tables
 *
 * A cell is subdivided into four children laid out along a Hilbert curve whose
 * orientation is one of four states: the identity, the axes swapped, the bits
 * inverted, and both. `kPosToIJ` maps a position along the curve to the
 * `(i,j)` offset of the child, `kIJtoPos` is its inverse, and
 * `kPosToOrientation` gives the state to exclusive-or into the parent's state
 * to obtain the child's.
 *****************************************************************************/

#define S2_SWAP_MASK    1
#define S2_INVERT_MASK  2

/** @brief Position along the curve to the `(i << 1) | j` child offset */
static const int kPosToIJ[4][4] =
{
  { 0, 1, 3, 2 },   /* canonical order   : (0,0) (0,1) (1,1) (1,0) */
  { 0, 2, 3, 1 },   /* axes swapped      : (0,0) (1,0) (1,1) (0,1) */
  { 3, 2, 0, 1 },   /* bits inverted     : (1,1) (1,0) (0,0) (0,1) */
  { 3, 1, 0, 2 },   /* swapped & inverted: (1,1) (0,1) (0,0) (1,0) */
};

/** @brief The `(i << 1) | j` child offset to its position along the curve */
static const int kIJtoPos[4][4] =
{
  { 0, 1, 3, 2 },
  { 0, 3, 1, 2 },
  { 2, 3, 1, 0 },
  { 2, 1, 3, 0 },
};

/** @brief Orientation change contributed by each position along the curve */
static const int kPosToOrientation[4] =
{
  S2_SWAP_MASK, 0, 0, S2_INVERT_MASK + S2_SWAP_MASK
};

/*****************************************************************************
 * Bit-layout primitives
 *****************************************************************************/

/**
 * @brief Return the lowest set bit of a cell id, which encodes its level
 */
uint64
s2cell_lsb(S2CellId cell)
{
  return cell & (~cell + 1);
}

/**
 * @brief Return the lowest-set-bit value that a cell of @p level carries
 */
uint64
s2cell_lsb_for_level(uint32_t level)
{
  return UINT64_C(1) << (2 * (S2_MAX_LEVEL - level));
}

/**
 * @ingroup meos_s2cell
 * @brief Return true if @p cell is a structurally valid S2 cell identifier
 * @details The face must be one of the six, and the trailing sentinel bit must
 * sit at an even position no higher than `2 * 30`, so that it records a level
 * in `[0, 30]`.
 * @csqlfn #S2cell_is_valid_cell()
 */
bool
s2cell_is_valid_cell(S2CellId cell)
{
  return (cell >> S2_FACE_SHIFT) < S2_NUM_FACES &&
    (s2cell_lsb(cell) & UINT64_C(0x1555555555555555)) != 0;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the level in `[0, 30]` of an S2 cell
 * @csqlfn #S2cell_get_resolution()
 */
uint32_t
s2cell_get_resolution(S2CellId cell)
{
  uint32_t shift = 0;
  uint64 lsb = s2cell_lsb(cell);
  while ((lsb >> shift) != 1)
    shift++;
  return S2_MAX_LEVEL - (shift >> 1);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the cube face in `[0, 5]` containing an S2 cell
 * @csqlfn #S2cell_get_face()
 */
uint32_t
s2cell_get_face(S2CellId cell)
{
  return (uint32_t) (cell >> S2_FACE_SHIFT);
}

/*****************************************************************************
 * Hierarchy
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the smallest cell identifier among the descendants of @p cell
 * @csqlfn #S2cell_range_min()
 */
S2CellId
s2cell_range_min(S2CellId cell)
{
  return cell - (s2cell_lsb(cell) - 1);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the largest cell identifier among the descendants of @p cell
 * @csqlfn #S2cell_range_max()
 */
S2CellId
s2cell_range_max(S2CellId cell)
{
  return cell + (s2cell_lsb(cell) - 1);
}

/**
 * @ingroup meos_s2cell
 * @brief Return true if @p cell contains @p other in the S2 hierarchy
 * @details A cell contains exactly the identifiers of the contiguous Hilbert
 * interval `[range_min, range_max]`, which is what makes an S2 ancestor test
 * a pair of integer comparisons.
 * @csqlfn #S2cell_cell_contains()
 */
bool
s2cell_cell_contains(S2CellId cell, S2CellId other)
{
  return other >= s2cell_range_min(cell) && other <= s2cell_range_max(cell);
}

/**
 * @ingroup meos_s2cell
 * @brief Return the ancestor of @p cell at @p level
 * @param[in] cell S2 cell
 * @param[in] level Level of the result, no finer than the level of @p cell
 * @csqlfn #S2cell_cell_to_parent()
 */
S2CellId
s2cell_cell_to_parent(S2CellId cell, uint32_t level)
{
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return (S2CellId) 0;
  }
  if (level > s2cell_get_resolution(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The parent level %u must be coarser than the cell level %u", level,
      s2cell_get_resolution(cell));
    return (S2CellId) 0;
  }
  uint64 lsb = s2cell_lsb_for_level(level);
  return (cell & (~lsb + 1)) | lsb;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the child of @p cell at @p level and Hilbert @p position
 * @param[in] cell S2 cell
 * @param[in] level Level of the result, exactly one finer than @p cell
 * @param[in] position Position along the Hilbert curve, in `[0, 3]`
 * @csqlfn #S2cell_cell_to_child()
 */
S2CellId
s2cell_cell_to_child(S2CellId cell, uint32_t level, uint32_t position)
{
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return (S2CellId) 0;
  }
  uint32_t cell_level = s2cell_get_resolution(cell);
  if (level != cell_level + 1 || level > S2_MAX_LEVEL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The child level %u must be exactly one finer than the cell level %u",
      level, cell_level);
    return (S2CellId) 0;
  }
  if (position > 3)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The child position %u must be in [0, 3]", position);
    return (S2CellId) 0;
  }
  uint64 lsb = s2cell_lsb(cell) >> 2;
  return cell + (2 * (uint64) position + 1 - 4) * lsb;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the descendants of @p cell at @p level
 * @param[in] cell S2 cell
 * @param[in] level Level of the results, no finer than 30
 * @param[out] count Number of descendants returned
 * @return A palloc'd array of `4 ^ (level - cell level)` cells (caller frees)
 */
S2CellId *
s2cell_cell_to_children(S2CellId cell, uint32_t level, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(count, NULL);
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return NULL;
  }
  uint32_t cell_level = s2cell_get_resolution(cell);
  if (level <= cell_level || level > S2_MAX_LEVEL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The children level %u must be finer than the cell level %u and at "
      "most %u", level, cell_level, S2_MAX_LEVEL);
    return NULL;
  }
  uint32_t levels = level - cell_level;
  /* Guard the allocation: 4 ^ 11 cells already exceed four million */
  if (levels > 11)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The children level %u is more than 11 levels finer than the cell "
      "level %u", level, cell_level);
    return NULL;
  }
  int64 num = INT64_C(1) << (2 * levels);
  S2CellId *result = palloc(sizeof(S2CellId) * (size_t) num);
  uint64 lsb = s2cell_lsb_for_level(level);
  /* The first descendant is the one covering the smallest leaf of the cell,
   * and successive descendants are two sentinel bits apart */
  S2CellId child = s2cell_range_min(cell) + lsb - 1;
  for (int64 k = 0; k < num; k++)
  {
    result[k] = child;
    child += 2 * lsb;
  }
  *count = (int) num;
  return result;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the level of the lowest common ancestor of two S2 cells, or
 * -1 if they lie on different cube faces
 * @csqlfn #S2cell_common_ancestor_level()
 */
int
s2cell_common_ancestor_level(S2CellId a, S2CellId b)
{
  if (! s2cell_is_valid_cell(a) || ! s2cell_is_valid_cell(b))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Both arguments must encode valid S2 cells");
    return -1;
  }
  /* The common ancestor is determined by the highest differing bit, floored
   * to the coarser of the two levels. */
  uint64 bits = a ^ b;
  uint64 lsb = s2cell_lsb(a) > s2cell_lsb(b) ? s2cell_lsb(a) : s2cell_lsb(b);
  if (bits < lsb)
    bits = lsb;
  int msb_pos = pg_leftmost_one_pos64(bits);
  /* Map the bit position to a level: {0} -> 30, {1,2} -> 29, ... ,
   * {59,60} -> 0, and the three face bits {61,62,63} -> -1 */
  int diff = 2 * (int) S2_MAX_LEVEL - msb_pos;
  return diff < 0 ? -1 : diff / 2;
}

/*****************************************************************************
 * Cell id <-> (face, i, j)
 *****************************************************************************/

/**
 * @brief Return the cell of @p level covering the leaf `(face, i, j)`
 * @param[in] face Cube face in `[0, 5]`
 * @param[in] i,j Leaf indices in `[0, 2^30)`
 * @param[in] level Level of the result
 * @param[out] result Cell identifier
 * @return The face of the result
 */
uint32_t
s2cell_from_face_ij(uint32_t face, uint32_t i, uint32_t j, uint32_t level,
  S2CellId *result)
{
  int orientation = (int) (face & S2_SWAP_MASK);
  uint64 pos = 0;
  for (int k = S2_MAX_LEVEL - 1; k >= 0; k--)
  {
    int ij = (int) ((((i >> k) & 1) << 1) | ((j >> k) & 1));
    int p = kIJtoPos[orientation][ij];
    pos = (pos << 2) | (uint64) p;
    orientation ^= kPosToOrientation[p];
  }
  S2CellId leaf = ((uint64) face << S2_FACE_SHIFT) | (pos << 1) | 1;
  *result = (level == S2_MAX_LEVEL) ? leaf : s2cell_cell_to_parent(leaf, level);
  return face;
}

/**
 * @brief Return the leaf `(i, j)` and the Hilbert orientation of a cell
 * @param[in] cell S2 cell
 * @param[out] i,j Leaf indices in `[0, 2^30)`
 * @param[out] orientation Hilbert orientation, ignored when NULL
 * @return The cube face of @p cell
 */
uint32_t
s2cell_to_face_ij(S2CellId cell, uint32_t *i, uint32_t *j,
  uint32_t *orientation)
{
  uint32_t face = s2cell_get_face(cell);
  int orient = (int) (face & S2_SWAP_MASK);
  uint32_t ii = 0, jj = 0;
  for (int k = S2_MAX_LEVEL - 1; k >= 0; k--)
  {
    int p = (int) ((cell >> (2 * k + 1)) & 3);
    int ij = kPosToIJ[orient][p];
    ii = (ii << 1) | (uint32_t) (ij >> 1);
    jj = (jj << 1) | (uint32_t) (ij & 1);
    orient ^= kPosToOrientation[p];
  }
  *i = ii;
  *j = jj;
  if (orientation)
    *orientation = (uint32_t) orient;
  return face;
}

/*****************************************************************************
 * Coordinate systems
 *
 * The default S2 projection is quadratic: it trades a little arithmetic for
 * cells whose areas vary by a factor of about 2.08 across a face, against 5.2
 * for the linear projection.
 *****************************************************************************/

/**
 * @brief Convert a cube coordinate in `[-1, 1]` to a face coordinate in `[0, 1]`
 */
static double
s2cell_uv_to_st(double u)
{
  if (u >= 0)
    return 0.5 * sqrt(1.0 + 3.0 * u);
  return 1.0 - 0.5 * sqrt(1.0 - 3.0 * u);
}

/**
 * @brief Convert a face coordinate in `[0, 1]` to a cube coordinate in `[-1, 1]`
 */
static double
s2cell_st_to_uv(double s)
{
  if (s >= 0.5)
    return (1.0 / 3.0) * (4.0 * s * s - 1.0);
  return (1.0 / 3.0) * (1.0 - 4.0 * (1.0 - s) * (1.0 - s));
}

/**
 * @brief Convert a face coordinate in `[0, 1]` to a leaf index in `[0, 2^30)`
 */
static uint32_t
s2cell_st_to_ij(double s)
{
  double v = floor(S2_MAX_SIZE * s);
  if (v < 0.0)
    return 0;
  if (v > (double) (S2_MAX_SIZE - 1))
    return S2_MAX_SIZE - 1;
  return (uint32_t) v;
}

/**
 * @brief Return the cube face and the `(u, v)` coordinates of a unit vector
 */
static uint32_t
s2cell_xyz_to_face_uv(const double xyz[3], double *u, double *v)
{
  uint32_t face = 0;
  double best = fabs(xyz[0]);
  for (uint32_t k = 1; k < 3; k++)
  {
    if (fabs(xyz[k]) > best)
    {
      best = fabs(xyz[k]);
      face = k;
    }
  }
  if (xyz[face] < 0)
    face += 3;
  switch (face)
  {
    case 0: *u =  xyz[1] / xyz[0]; *v =  xyz[2] / xyz[0]; break;
    case 1: *u = -xyz[0] / xyz[1]; *v =  xyz[2] / xyz[1]; break;
    case 2: *u = -xyz[0] / xyz[2]; *v = -xyz[1] / xyz[2]; break;
    case 3: *u =  xyz[2] / xyz[0]; *v =  xyz[1] / xyz[0]; break;
    case 4: *u =  xyz[2] / xyz[1]; *v = -xyz[0] / xyz[1]; break;
    default: *u = -xyz[1] / xyz[2]; *v = -xyz[0] / xyz[2]; break;
  }
  return face;
}

/**
 * @brief Return the unit vector of a `(face, u, v)` cube coordinate
 */
static void
s2cell_face_uv_to_xyz(uint32_t face, double u, double v, double xyz[3])
{
  switch (face)
  {
    case 0: xyz[0] =  1.0; xyz[1] =    u; xyz[2] =    v; break;
    case 1: xyz[0] =   -u; xyz[1] =  1.0; xyz[2] =    v; break;
    case 2: xyz[0] =   -u; xyz[1] =   -v; xyz[2] =  1.0; break;
    case 3: xyz[0] = -1.0; xyz[1] =   -v; xyz[2] =   -u; break;
    case 4: xyz[0] =    v; xyz[1] = -1.0; xyz[2] =   -u; break;
    default: xyz[0] =   v; xyz[1] =    u; xyz[2] = -1.0; break;
  }
  double norm = sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
  xyz[0] /= norm;
  xyz[1] /= norm;
  xyz[2] /= norm;
}

/**
 * @brief Return the unit vector of a geodetic position in degrees
 */
static void
s2cell_lonlat_to_xyz(double longitude, double latitude, double xyz[3])
{
  double lon = longitude * M_PI / 180.0;
  double lat = latitude * M_PI / 180.0;
  double c = cos(lat);
  xyz[0] = c * cos(lon);
  xyz[1] = c * sin(lon);
  xyz[2] = sin(lat);
}

/**
 * @brief Return the geodetic position in degrees of a unit vector
 */
static void
s2cell_xyz_to_lonlat(const double xyz[3], double *longitude, double *latitude)
{
  *longitude = atan2(xyz[1], xyz[0]) * 180.0 / M_PI;
  *latitude = atan2(xyz[2], sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1])) *
    180.0 / M_PI;
}

/*****************************************************************************
 * Geometry conversion
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the S2 cell of @p level containing a geodetic position
 * @param[in] longitude,latitude Position in degrees
 * @param[in] level Level of the result, in `[0, 30]`
 */
S2CellId
s2cell_point_to_cell(double longitude, double latitude, uint32_t level)
{
  if (level > S2_MAX_LEVEL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The S2 level %u must be in [0, %u]", level, S2_MAX_LEVEL);
    return (S2CellId) 0;
  }
  if (latitude < -90.0 || latitude > 90.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The latitude %f must be in [-90, 90]", latitude);
    return (S2CellId) 0;
  }
  double xyz[3], u, v;
  s2cell_lonlat_to_xyz(longitude, latitude, xyz);
  uint32_t face = s2cell_xyz_to_face_uv(xyz, &u, &v);
  uint32_t i = s2cell_st_to_ij(s2cell_uv_to_st(u));
  uint32_t j = s2cell_st_to_ij(s2cell_uv_to_st(v));
  S2CellId result;
  s2cell_from_face_ij(face, i, j, level, &result);
  return result;
}

/**
 * @brief Return the geodetic centre of an S2 cell
 * @param[in] cell S2 cell
 * @param[out] longitude,latitude Position in degrees
 */
void
s2cell_cell_point(S2CellId cell, double *longitude, double *latitude)
{
  assert(longitude); assert(latitude);
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return;
  }
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  /* The centre of a cell is the corner of the leaf grid shifted by half a
   * leaf for a leaf cell, and by a whole leaf for every coarser cell whose
   * ancestry places its centre on a leaf boundary. */
  uint32_t delta = (s2cell_lsb(cell) == 1) ? 1 :
    (((i ^ (uint32_t) (cell >> 2)) & 1) ? 2 : 0);
  double s = (2.0 * (double) i + delta) / (2.0 * (double) S2_MAX_SIZE);
  double t = (2.0 * (double) j + delta) / (2.0 * (double) S2_MAX_SIZE);
  double xyz[3];
  s2cell_face_uv_to_xyz(face, s2cell_st_to_uv(s), s2cell_st_to_uv(t), xyz);
  s2cell_xyz_to_lonlat(xyz, longitude, latitude);
  return;
}

/**
 * @brief Return the `(u, v)` extent of an S2 cell on its cube face
 */
static uint32_t
s2cell_cell_uv(S2CellId cell, double *umin, double *vmin, double *umax,
  double *vmax)
{
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  uint32_t size = 1u << (S2_MAX_LEVEL - s2cell_get_resolution(cell));
  uint32_t ilo = i & ~(size - 1);
  uint32_t jlo = j & ~(size - 1);
  *umin = s2cell_st_to_uv((double) ilo / (double) S2_MAX_SIZE);
  *umax = s2cell_st_to_uv((double) (ilo + size) / (double) S2_MAX_SIZE);
  *vmin = s2cell_st_to_uv((double) jlo / (double) S2_MAX_SIZE);
  *vmax = s2cell_st_to_uv((double) (jlo + size) / (double) S2_MAX_SIZE);
  return face;
}

/**
 * @brief Return the four geodetic vertices of an S2 cell, counterclockwise
 * @param[in] cell S2 cell
 * @param[out] longitudes,latitudes Arrays of four positions in degrees
 */
void
s2cell_cell_vertices(S2CellId cell, double *longitudes, double *latitudes)
{
  assert(longitudes); assert(latitudes);
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return;
  }
  double umin, vmin, umax, vmax;
  uint32_t face = s2cell_cell_uv(cell, &umin, &vmin, &umax, &vmax);
  const double us[4] = { umin, umax, umax, umin };
  const double vs[4] = { vmin, vmin, vmax, vmax };
  for (int k = 0; k < 4; k++)
  {
    double xyz[3];
    s2cell_face_uv_to_xyz(face, us[k], vs[k], xyz);
    s2cell_xyz_to_lonlat(xyz, &longitudes[k], &latitudes[k]);
  }
  return;
}

/*****************************************************************************
 * Cells crossed by a segment
 *****************************************************************************/

/**
 * @brief Set the last argument to the four vertices of an S2 cell as unit
 * vectors, three coordinates each, in the order #s2cell_cell_vertices states
 * them
 */
static void
s2cell_cell_xyz_vertices(S2CellId cell, double *verts)
{
  double umin, vmin, umax, vmax;
  uint32_t face = s2cell_cell_uv(cell, &umin, &vmin, &umax, &vmax);
  const double us[4] = { umin, umax, umax, umin };
  const double vs[4] = { vmin, vmin, vmax, vmax };
  for (int k = 0; k < 4; k++)
    s2cell_face_uv_to_xyz(face, us[k], vs[k], &verts[3 * k]);
  return;
}

/**
 * @brief Set the last two arguments to the longitudes and latitudes in
 * radians of four cell vertices given by their `(x, y, z)` coordinates, as
 * #s2cell_xyz_to_lonlat reads them in degrees
 */
static void
s2cell_xyz_vertices_to_lonlat(const double *verts, double *lons,
  double *lats)
{
  for (int k = 0; k < 4; k++)
  {
    const double *p = &verts[3 * k];
    lons[k] = atan2(p[1], p[0]);
    lats[k] = atan2(p[2], sqrt(p[0] * p[0] + p[1] * p[1]));
  }
  return;
}

/**
 * @brief Return the angle subtended by the shortest edge of a cell given by
 * its four vertices as unit vectors
 */
static double
s2cell_shortest_edge(const double *verts)
{
  double best = DBL_MAX;
  for (int k = 0; k < 4; k++)
  {
    const double *p = &verts[3 * k];
    const double *q = &verts[3 * ((k + 1) % 4)];
    const double c[3] = { p[1] * q[2] - p[2] * q[1],
      p[2] * q[0] - p[0] * q[2], p[0] * q[1] - p[1] * q[0] };
    double angle = atan2(sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]),
      p[0] * q[0] + p[1] * q[1] + p[2] * q[2]);
    if (angle < best)
      best = angle;
  }
  return best;
}

/**
 * @brief Return the S2 cell of a level holding the position a geodetic path
 * reaches at a parameter, or 0 when the position cannot be projected
 */
static S2CellId
s2cell_arc_cell(const DggsArc *arc, double t, uint32_t level)
{
  double lon, lat;
  if (! dggs_arc_point(arc, t, &lon, &lat))
    return (S2CellId) 0;
  return s2cell_point_to_cell(lon * 180.0 / M_PI, lat * 180.0 / M_PI, level);
}

/** @brief A path of a segment and the level its cells are read at, the state
 * #dggs_crossing_param() reads a cell of the path from */
typedef struct
{
  const DggsArc *arc;        /**< Path of a geodetic segment, or NULL */
  const DggsLine *line;      /**< Path of a planar segment, or NULL */
  uint32_t level;            /**< S2 level */
} S2PathAt;

/**
 * @brief Return the S2 cell the path of a segment holds at a parameter, or 0
 * when the position cannot be projected
 */
static uint64
s2cell_path_cell_at(void *state, double t)
{
  const S2PathAt *path = (const S2PathAt *) state;
  if (path->arc)
    return (uint64) s2cell_arc_cell(path->arc, t, path->level);
  double lon, lat;
  dggs_line_point(path->line, t, &lon, &lat);
  return (uint64) s2cell_point_to_cell(lon, lat, path->level);
}

/**
 * @brief Fill `cells` with every S2 cell a planar segment crosses, and `enter`
 * with the segment parameter at which it reaches each
 * @details The walk of the geodetic segment, along the straight line in
 * longitude and latitude a planar point moves along: from the cell in hand the
 * walk leaves through its boundary, found by #dggs_line_exit_param, and the
 * cell just beyond that crossing is a neighbour of it. That line is no great
 * circle, so it may leave a cell and come back to it, and the walk runs for
 * every segment, whatever cell its far endpoint lies in.
 */
static int
s2cell_line_cells(double lon1, double lat1, double lon2, double lat2,
  uint32_t level, S2CellId *cells, double *enter, int maxout)
{
  S2CellId cur = s2cell_point_to_cell(lon1, lat1, level);
  if (cur == (S2CellId) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;
  DggsLine line;
  if (! dggs_line_init(lon1, lat1, lon2, lat2, &line))
    return n;

  /* A position on a cell boundary belongs to the one cell its level assigns
   * it, and a path starting there moves into the neighbouring cell at once,
   * through a crossing no search strictly ahead of the start states: the walk
   * leaves from the cell just past the start, entered where the halving of
   * #dggs_crossing_param() places the crossing */
  S2PathAt path = { .arc = NULL, .line = &line, .level = level };
  double t = 0.0, verts[12], lon, lat;
  s2cell_cell_xyz_vertices(cur, verts);
  double t0 = s2cell_shortest_edge(verts) * 1e-4 / line.length;
  if (t0 < 1.0)
  {
    dggs_line_point(&line, t0, &lon, &lat);
    S2CellId first = s2cell_point_to_cell(lon, lat, level);
    if (first != (S2CellId) 0 && first != cur && n < maxout)
    {
      cells[n] = first;
      enter[n] = dggs_crossing_param(0.0, t0, cur, &s2cell_path_cell_at,
        &path);
      n++;
      cur = first;
      t = t0;
    }
  }
  while (n < maxout)
  {
    s2cell_cell_xyz_vertices(cur, verts);
    double lons[4], lats[4];
    s2cell_xyz_vertices_to_lonlat(verts, lons, lats);
    double texit = dggs_line_exit_param(&line, lons, lats, 4, t, true);
    if (texit > 1.0)
      break;                 /* the segment ends inside this cell */
    /* The nudge of the geodetic walk, measured along the line */
    double nudge = s2cell_shortest_edge(verts) * 1e-4 / line.length;
    double tn = texit + nudge, tin = texit;
    S2CellId next = (S2CellId) 0;
    for (int k = 0; k < 8; k++)
    {
      if (tn > 1.0)
        tn = 1.0;
      dggs_line_point(&line, tn, &lon, &lat);
      next = s2cell_point_to_cell(lon, lat, level);
      if (next == (S2CellId) 0 || next != cur || tn >= 1.0)
        break;
      tin = tn;
      tn += nudge * (double) (1 << k);
    }
    if (next == (S2CellId) 0)
      break;                 /* the position cannot be projected */
    if (next == cur)
    {
      /* A path still in the cell past the crossing touches the edge circle
       * there, or meets it within the rounding, so the walk goes on from
       * just past that crossing */
      t = texit + nudge;
      continue;
    }
    /* A crossing the path is still in the cell past is one the rounding of an
     * edge the path runs along places where the path is: the probes bracket
     * the crossing, and halving the bracket closes on it */
    cells[n] = next;
    enter[n] = (tin > texit) ?
      dggs_crossing_param(tin, tn, cur, &s2cell_path_cell_at, &path) : texit;
    n++;
    cur = next;
    t = tn;
  }
  return n;
}

/**
 * @brief Fill `cells` with every S2 cell a segment crosses, and `enter` with
 * the segment parameter at which it reaches each
 * @details A traversal, not a sampling walk: from the cell in hand the walk
 * leaves through its boundary, and the cell just beyond that crossing is a
 * neighbour of it, so no cell between the two is passed over. An S2 cell edge
 * lies on a line of constant `u` or `v` of its cube face, which the gnomonic
 * projection of the face maps to a great circle, the circle a geodetic point
 * moves along. Every crossing is therefore found on the sphere, where a path
 * across the antimeridian, over a pole or from one cube face to the next is
 * an arc like any other. A planar segment follows the straight line in
 * longitude and latitude, traversed as #s2cell_line_cells states.
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] geodetic True when the segment is geodetic
 * @param[in] level S2 level
 * @param[out] cells,enter Arrays of at least `maxout` entries; `enter[0]` is
 * always 0, the parameter of the first endpoint
 * @param[in] maxout Capacity of both arrays
 * @return Number of cells written, which is `maxout` when the arrays fill
 * before the segment ends, or 0 when the first lookup fails
 */
int
s2cell_segment_cells(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, uint32_t level, S2CellId *cells, double *enter, int maxout)
{
  assert(cells); assert(enter);
  if (maxout < 1)
    return 0;
  if (! geodetic)
    return s2cell_line_cells(lon1, lat1, lon2, lat2, level, cells, enter,
      maxout);
  S2CellId cur = s2cell_point_to_cell(lon1, lat1, level);
  if (cur == (S2CellId) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;
  /* A cell is convex on the sphere, so a path whose far endpoint lies in the
   * cell of its near one never leaves it */
  if (s2cell_point_to_cell(lon2, lat2, level) == cur)
    return n;
  DggsArc arc;
  if (! dggs_arc_init(lon1, lat1, lon2, lat2, &arc))
    return n;

  /* A position on a cell boundary belongs to the one cell its level assigns
   * it, and a path starting there moves into the neighbouring cell at once,
   * through a crossing no search strictly ahead of the start states: the walk
   * leaves from the cell just past the start, entered where the halving of
   * #dggs_crossing_param() places the crossing */
  S2PathAt path = { .arc = &arc, .line = NULL, .level = level };
  double t = 0.0, verts[12];
  s2cell_cell_xyz_vertices(cur, verts);
  double t0 = s2cell_shortest_edge(verts) * 1e-4 / arc.dist;
  if (t0 < 1.0)
  {
    S2CellId first = s2cell_arc_cell(&arc, t0, level);
    if (first != (S2CellId) 0 && first != cur && n < maxout)
    {
      cells[n] = first;
      enter[n] = dggs_crossing_param(0.0, t0, cur, &s2cell_path_cell_at,
        &path);
      n++;
      cur = first;
      t = t0;
    }
  }
  while (n < maxout)
  {
    s2cell_cell_xyz_vertices(cur, verts);
    double lons[4], lats[4];
    s2cell_xyz_vertices_to_lonlat(verts, lons, lats);
    /* An S2 cell is convex, bounded by four arcs of great circles */
    double texit = dggs_arc_exit_param(&arc, lons, lats, 4, t, true);
    if (texit > 1.0)
      break;                 /* the segment ends inside this cell */
    /* A nudge past the crossing lands inside the next cell without reaching
     * the one after it: a ten-thousandth of the shortest edge of the cell is
     * far below the width of a neighbouring cell of the level and far above
     * the rounding of the crossing itself */
    double nudge = s2cell_shortest_edge(verts) * 1e-4 / arc.dist;
    double tn = texit + nudge, tin = texit;
    S2CellId next = (S2CellId) 0;
    /* A nudge that lands back in the cell just left says the crossing sits
     * within its own rounding, so widen it rather than stall. A crossing
     * nearer the end of the segment than the nudge reads the cell of the end,
     * which is the cell the path enters there */
    for (int k = 0; k < 8; k++)
    {
      if (tn > 1.0)
        tn = 1.0;
      next = s2cell_arc_cell(&arc, tn, level);
      if (next == (S2CellId) 0 || next != cur || tn >= 1.0)
        break;
      tin = tn;
      tn += nudge * (double) (1 << k);
    }
    if (next == (S2CellId) 0)
      break;                 /* the position cannot be projected */
    if (next == cur)
    {
      /* An arc leaving a convex cell never returns to it, so a crossing past
       * which the path is still in the cell is one the rounding places on the
       * circle of an edge the path runs along. The walk goes on from that
       * crossing and not from the farthest probe, which can lie past the
       * crossing through which the path does leave */
      t = texit;
      continue;
    }
    /* A crossing the path is still in the cell past is one the rounding of an
     * edge the path runs along places where the path is: the probes bracket
     * the crossing, and halving the bracket closes on it */
    cells[n] = next;
    enter[n] = (tin > texit) ?
      dggs_crossing_param(tin, tn, cur, &s2cell_path_cell_at, &path) : texit;
    n++;
    cur = next;
    t = tn;
  }
  return n;
}

/**
 * @brief Return the geodetic bounding box of an S2 cell
 * @details The box CONTAINS the cell. The shared derivation in
 * `dggs_lonlat_boundary_set_box` adds to the vertex extent what the vertices
 * alone do not state: the latitude a geodesic edge reaches beyond both of its
 * endpoints, the pole a cell holding one reaches, and the full longitude range
 * a cell holding a pole or crossing the antimeridian spans.
 */
void
s2cell_cell_bounding_box(S2CellId cell, double *xmin, double *ymin,
  double *xmax, double *ymax)
{
  double lons[4], lats[4];
  s2cell_cell_vertices(cell, lons, lats);
  bool north = s2cell_cell_contains(cell,
    s2cell_point_to_cell(0.0, 90.0, S2_MAX_LEVEL));
  bool south = s2cell_cell_contains(cell,
    s2cell_point_to_cell(0.0, -90.0, S2_MAX_LEVEL));
  dggs_lonlat_boundary_set_box(lons, lats, 4, north, south, xmin, ymin, xmax,
    ymax);
  return;
}

/*****************************************************************************
 * Metrics
 *****************************************************************************/

/** @brief Authalic radius of the WGS84 sphere, in metres */
#define S2_EARTH_RADIUS  6371007.180918475

/**
 * @brief Return the angle in radians between two unit vectors
 */
static double
s2cell_angle(const double a[3], const double b[3])
{
  const double cross[3] =
  {
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0]
  };
  double norm = sqrt(cross[0] * cross[0] + cross[1] * cross[1] +
    cross[2] * cross[2]);
  double dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  return atan2(norm, dot);
}

/**
 * @brief Return the area in steradians of a spherical triangle of unit
 * vectors, by l'Huilier's formula
 */
static double
s2cell_triangle_area(const double a[3], const double b[3], const double c[3])
{
  double sa = s2cell_angle(b, c);
  double sb = s2cell_angle(c, a);
  double sc = s2cell_angle(a, b);
  double s = 0.5 * (sa + sb + sc);
  double prod = tan(0.5 * s) * tan(0.5 * (s - sa)) * tan(0.5 * (s - sb)) *
    tan(0.5 * (s - sc));
  if (prod <= 0.0)
    return 0.0;
  return 4.0 * atan(sqrt(prod));
}

/**
 * @ingroup meos_s2cell
 * @brief Return the area in square metres of an S2 cell on the WGS84 sphere
 * @details The cell is a spherical quadrilateral bounded by four geodesic
 * edges; its area is the sum of the areas of the two spherical triangles that
 * a diagonal splits it into.
 * @csqlfn #S2cell_cell_area()
 */
double
s2cell_cell_area(S2CellId cell)
{
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return -1.0;
  }
  double umin, vmin, umax, vmax;
  uint32_t face = s2cell_cell_uv(cell, &umin, &vmin, &umax, &vmax);
  const double us[4] = { umin, umax, umax, umin };
  const double vs[4] = { vmin, vmin, vmax, vmax };
  double p[4][3];
  for (int k = 0; k < 4; k++)
    s2cell_face_uv_to_xyz(face, us[k], vs[k], p[k]);
  double steradians = s2cell_triangle_area(p[0], p[1], p[2]) +
    s2cell_triangle_area(p[0], p[2], p[3]);
  return steradians * S2_EARTH_RADIUS * S2_EARTH_RADIUS;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the length in metres of an edge of an S2 cell on the WGS84
 * sphere
 * @param[in] cell S2 cell
 * @param[in] edge Edge in `[0, 3]`, from vertex @p edge to vertex `edge + 1`
 * @csqlfn #S2cell_edge_length()
 */
double
s2cell_edge_length(S2CellId cell, uint32_t edge)
{
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return -1.0;
  }
  if (edge > 3)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The S2 cell edge %u must be in [0, 3]", edge);
    return -1.0;
  }
  double umin, vmin, umax, vmax;
  uint32_t face = s2cell_cell_uv(cell, &umin, &vmin, &umax, &vmax);
  const double us[4] = { umin, umax, umax, umin };
  const double vs[4] = { vmin, vmin, vmax, vmax };
  uint32_t next = (edge + 1) % 4;
  double a[3], b[3];
  s2cell_face_uv_to_xyz(face, us[edge], vs[edge], a);
  s2cell_face_uv_to_xyz(face, us[next], vs[next], b);
  return s2cell_angle(a, b) * S2_EARTH_RADIUS;
}

/*****************************************************************************
 * Traversal
 *****************************************************************************/

/**
 * @brief Return the cell of @p level at `(face, i, j)`, wrapping across the
 * cube edge when an index leaves the face
 */
static S2CellId
s2cell_from_face_ij_wrap(uint32_t face, int64 i, int64 j, uint32_t level)
{
  const int64 limit = (int64) S2_MAX_SIZE;
  if (i >= 0 && j >= 0 && i < limit && j < limit)
  {
    S2CellId result;
    s2cell_from_face_ij(face, (uint32_t) i, (uint32_t) j, level, &result);
    return result;
  }
  /* Clamp to a leaf just beyond the boundary of the face, project it onto
   * the sphere, and re-derive the cell from the resulting position, which
   * lands on the neighbouring face. The projection used here is the linear
   * one, `u = 2 * s - 1`, applied in both directions: any projection wraps
   * correctly as long as it is its own inverse, and the linear one keeps the
   * clamped point barely outside the face rectangle, where the quadratic one
   * would move it far enough to reach the wrong leaf. */
  i = (i < -1) ? -1 : (i > limit ? limit : i);
  j = (j < -1) ? -1 : (j > limit ? limit : j);
  const double scale = 1.0 / (double) limit;
  const double bound = 1.0 + DBL_EPSILON;
  double u = scale * (double) ((i << 1) + 1 - limit);
  double v = scale * (double) ((j << 1) + 1 - limit);
  u = (u < -bound) ? -bound : (u > bound ? bound : u);
  v = (v < -bound) ? -bound : (v > bound ? bound : v);
  double xyz[3], nu, nv;
  s2cell_face_uv_to_xyz(face, u, v, xyz);
  uint32_t newface = s2cell_xyz_to_face_uv(xyz, &nu, &nv);
  uint32_t ni = s2cell_st_to_ij(0.5 * (nu + 1.0));
  uint32_t nj = s2cell_st_to_ij(0.5 * (nv + 1.0));
  S2CellId result;
  s2cell_from_face_ij(newface, ni, nj, level, &result);
  return result;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the four cells sharing an edge with @p cell
 * @param[in] cell S2 cell
 * @param[out] count Number of neighbours returned, always four
 * @return A palloc'd array of four cells (caller frees)
 */
S2CellId *
s2cell_edge_neighbors(S2CellId cell, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(count, NULL);
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Value %" PRIu64 " does not encode a valid S2 cell", (uint64) cell);
    return NULL;
  }
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  uint32_t level = s2cell_get_resolution(cell);
  int64 size = INT64_C(1) << (S2_MAX_LEVEL - level);
  S2CellId *result = palloc(sizeof(S2CellId) * 4);
  /* Edges 0 to 3 run down, right, up, and left */
  result[0] = s2cell_from_face_ij_wrap(face, (int64) i, (int64) j - size,
    level);
  result[1] = s2cell_from_face_ij_wrap(face, (int64) i + size, (int64) j,
    level);
  result[2] = s2cell_from_face_ij_wrap(face, (int64) i, (int64) j + size,
    level);
  result[3] = s2cell_from_face_ij_wrap(face, (int64) i - size, (int64) j,
    level);
  *count = 4;
  return result;
}

/*****************************************************************************
 * Token serialization
 *
 * The canonical S2 token is the lowercase hexadecimal of the identifier with
 * its trailing zeroes removed, so that a coarse cell reads as a short prefix.
 *****************************************************************************/

/**
 * @ingroup meos_s2cell
 * @brief Return the canonical token of an S2 cell
 * @return A palloc'd, null-terminated string (caller frees)
 * @csqlfn #S2cell_cell_to_token()
 */
char *
s2cell_cell_to_token(S2CellId cell)
{
  char buf[S2_TOKEN_MAXLEN + 1];
  snprintf(buf, sizeof(buf), "%016" PRIx64, (uint64) cell);
  size_t len = S2_TOKEN_MAXLEN;
  while (len > 0 && buf[len - 1] == '0')
    len--;
  char *result = palloc(len + 1);
  memcpy(result, buf, len);
  result[len] = '\0';
  return result;
}

/**
 * @ingroup meos_s2cell
 * @brief Return the S2 cell of a canonical token
 * @csqlfn #S2cell_token_to_cell()
 */
S2CellId
s2cell_token_to_cell(const char *token)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(token, (S2CellId) 0);
  size_t len = strlen(token);
  if (len == 0 || len > S2_TOKEN_MAXLEN)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The S2 token \"%s\" must hold between 1 and %d hexadecimal digits",
      token, S2_TOKEN_MAXLEN);
    return (S2CellId) 0;
  }
  uint64 result = 0;
  for (size_t k = 0; k < S2_TOKEN_MAXLEN; k++)
  {
    int digit = 0;
    if (k < len)
    {
      char c = token[k];
      if (c >= '0' && c <= '9')
        digit = c - '0';
      else if (c >= 'a' && c <= 'f')
        digit = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        digit = c - 'A' + 10;
      else
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "The S2 token \"%s\" holds the non-hexadecimal character '%c'",
          token, c);
        return (S2CellId) 0;
      }
    }
    result = (result << 4) | (uint64) digit;
  }
  return (S2CellId) result;
}

/*****************************************************************************
 * Input/output
 *****************************************************************************/

/**
 * @brief Parse a string into an S2 cell. See header for the accepted input
 * shapes.
 */
S2CellId
s2cell_parse(const char *str)
{
  assert(str);

  /* Strip leading whitespace. */
  while (*str && isspace((unsigned char) *str))
    str++;

  /* Skip an optional "0x" / "0X" hex prefix; the canonical S2 output is an
   * unprefixed lowercase token. */
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    str += 2;

  S2CellId cell = s2cell_token_to_cell(str);

  /* Reject anything that does not encode a valid S2 cell. The zero
   * identifier, whose S2 token is "X", is among those. */
  if (! s2cell_is_valid_cell(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "s2cell value \"%s\" does not encode a valid S2 cell", str);
    return (S2CellId) 0;
  }

  return cell;
}

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return an S2 cell from its string representation
 * @param[in] str String
 * @csqlfn #S2cell_in()
 */
S2CellId
s2cell_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, (S2CellId) 0);
  return s2cell_parse(str);
}

/**
 * @ingroup meos_s2cell_conversion
 * @brief Return an S2 cell from a 64-bit integer, raising an error when the
 * integer encodes no cell
 * @param[in] i Integer
 * @csqlfn #Bigint_to_s2cell()
 */
S2CellId
bigint_to_s2cell(int64 i)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cell(Int64GetDatum(i), T_TS2CELL))
    return (S2CellId) 0;
  return (S2CellId) i;
}

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return the string representation of an S2 cell
 * @param[in] cell S2 cell
 * @return A palloc'd, null-terminated string (caller frees)
 * @csqlfn #S2cell_out()
 */
char *
s2cell_out(S2CellId cell)
{
  return s2cell_cell_to_token(cell);
}

/*****************************************************************************
 * Comparison / ordering
 *
 * S2 cell identifiers order along the Hilbert curve, so their numerical order
 * is also a spatial order.
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if two S2 cells are equal
 * @csqlfn #S2cell_eq()
 */
bool
s2cell_eq(S2CellId a, S2CellId b)
{
  return a == b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if two S2 cells are different
 * @csqlfn #S2cell_ne()
 */
bool
s2cell_ne(S2CellId a, S2CellId b)
{
  return a != b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if the first S2 cell is less than the second one
 * @csqlfn #S2cell_lt()
 */
bool
s2cell_lt(S2CellId a, S2CellId b)
{
  return a < b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if the first S2 cell is less than or equal to the second
 * one
 * @csqlfn #S2cell_le()
 */
bool
s2cell_le(S2CellId a, S2CellId b)
{
  return a <= b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if the first S2 cell is greater than the second one
 * @csqlfn #S2cell_gt()
 */
bool
s2cell_gt(S2CellId a, S2CellId b)
{
  return a > b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return true if the first S2 cell is greater than or equal to the
 * second one
 * @csqlfn #S2cell_ge()
 */
bool
s2cell_ge(S2CellId a, S2CellId b)
{
  return a >= b;
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first S2 cell is less
 * than, equal to, or greater than the second one
 * @csqlfn #S2cell_cmp()
 */
int
s2cell_cmp(S2CellId a, S2CellId b)
{
  if (a < b)
    return -1;
  if (a > b)
    return 1;
  return 0;
}

/*****************************************************************************
 * Hash
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return the 32-bit hash of an S2 cell
 * @csqlfn #S2cell_hash()
 */
uint32
s2cell_hash(S2CellId cell)
{
  return int64_hash((int64) cell);
}

/**
 * @ingroup meos_s2cell_base_comp
 * @brief Return the 64-bit hash of an S2 cell using a seed
 * @csqlfn #S2cell_hash_extended()
 */
uint64
s2cell_hash_extended(S2CellId cell, uint64 seed)
{
  return int64_hash_extended((int64) cell, seed);
}

/*****************************************************************************/
