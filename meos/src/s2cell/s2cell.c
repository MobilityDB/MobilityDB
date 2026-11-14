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
 * @brief First-party implementation of the Google S2 cell algebra
 * @details S2 is a quadtree Discrete Global Grid System on the sphere. This
 * file is the
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
 * bits 61..63 : cube face (0..5)
 * bits 1..60  : position along the Hilbert curve, two bits per level
 * bit  0..    : a single trailing set bit at position `2 * (30 - level)`
 * that records the level of the cell
 *
 * Five coordinate systems connect a cell id to a geodetic position:
 * (lon,lat) <-> (x,y,z) on the unit sphere <-> (face,u,v) on the cube
 * <-> (face,s,t) in the unit square <-> (face,i,j) leaf indices
 * <-> cell id along the Hilbert curve
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
#include "geo/geo_funcs.h"
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/tcellindex.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"

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
 * @param[out] orientation Hilbert orientation, ignored when NULL @nullable
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
 * @brief Return the point of the cube `[-1, 1]^3` at a `(face, u, v)` cube
 * coordinate
 * @details The inverse of #s2cell_xyz_to_face_uv on the cube, which
 * #s2cell_face_uv_to_xyz projects onto the sphere
 */
static void
s2cell_face_uv_to_cube(uint32_t face, double u, double v, double xyz[3])
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
  return;
}

/**
 * @brief Return the unit vector of a `(face, u, v)` cube coordinate
 * @details The point #s2cell_face_uv_to_cube places on the cube, normalised
 */
static void
s2cell_face_uv_to_xyz(uint32_t face, double u, double v, double xyz[3])
{
  s2cell_face_uv_to_cube(face, u, v, xyz);
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
  if (! ensure_finite(longitude) || ! ensure_finite(latitude))
    return (S2CellId) 0;
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

/** @brief The denominator of the cube coordinate of a corner, 3 4^30 */
#define S2_CORNER_DEN (INT64CONST(3) << 60)

/**
 * @brief Return the numerator of the cube coordinate of the corner at the
 * leaf index @p I in `[0, 2^30]`, the coordinate being that numerator over
 * #S2_CORNER_DEN
 * @details The exact twin of #s2cell_st_to_uv: the quadratic transform at
 * the dyadic s = I / 2^30 is the rational (4 s^2 - 1) / 3 from s = 1/2 up
 * and (1 - 4 (1 - s)^2) / 3 below, so its numerator over
 * 3 4^30 is 4 I^2 - 4^30 or 4^30 - 4 (2^30 - I)^2, at most 3 4^30 in
 * magnitude (decision D7 of the cell walks)
 */
static int64
s2cell_corner_num(uint32_t I)
{
  const int64 q = INT64CONST(1) << 60;
  int64 i = I;
  if (I >= (1u << 29))
    return 4 * i * i - q;
  int64 r = (INT64CONST(1) << 30) - i;
  return q - 4 * r * r;
}

/**
 * @brief Return in @p v the corner of a cube face at the leaf indices
 * @p I and @p J, scaled by #S2_CORNER_DEN to a vector of integers, as
 * #s2cell_face_uv_to_cube places it
 */
static void
s2cell_corner_int(uint32_t face, uint32_t I, uint32_t J, int64 *v)
{
  const int64 n = S2_CORNER_DEN;
  int64 u = s2cell_corner_num(I), w = s2cell_corner_num(J);
  switch (face)
  {
    case 0: v[0] =  n; v[1] =  u; v[2] =  w; break;
    case 1: v[0] = -u; v[1] =  n; v[2] =  w; break;
    case 2: v[0] = -u; v[1] = -w; v[2] =  n; break;
    case 3: v[0] = -n; v[1] = -w; v[2] = -u; break;
    case 4: v[0] =  w; v[1] = -n; v[2] = -u; break;
    default: v[0] = w; v[1] =  u; v[2] = -n; break;
  }
}

/** @brief Number of entries of #s2_uv_cache and #s2_vertex_cache, a power
 * of 2 */
#define S2_CORNER_CACHE 1024

/** @brief The cube coordinates of corners read last, per thread, by their
 * leaf index */
static MEOS_TLS struct
{
  uint32_t I;
  double u;
  bool used;
} s2_uv_cache[S2_CORNER_CACHE];

/** @brief The degrees of corners read last, per thread, by their face and
 * leaf indices */
static MEOS_TLS struct
{
  uint32_t face, I, J;
  double lon, lat;
  bool used;
} s2_vertex_cache[S2_CORNER_CACHE];

/** @brief The longitudes of corners read last, per thread, by their face
 * and the one leaf index they depend on */
static MEOS_TLS struct
{
  uint32_t face, index;
  double lon;
  bool used;
} s2_lon_cache[S2_CORNER_CACHE];

/**
 * @brief Return the double nearest the cube coordinate of the corner at the
 * leaf index @p I (#s2cell_corner_num, #rational_nearest)
 * @details The coordinate is a function of the index alone, and a walk reads
 * the corners neighbouring cells share again and again, so the ones read
 * last are kept by their index and answer again, as #h3_lonlat_vector keeps
 * the geodetic positions of the H3 vertices
 */
static double
s2cell_corner_uv(uint32_t I)
{
  int k = (int) ((I * UINT64CONST(0x9E3779B97F4A7C15)) >> 54) &
    (S2_CORNER_CACHE - 1);
  if (! s2_uv_cache[k].used || s2_uv_cache[k].I != I)
  {
    s2_uv_cache[k].u = rational_nearest(s2cell_corner_num(I),
      S2_CORNER_DEN);
    s2_uv_cache[k].I = I;
    s2_uv_cache[k].used = true;
  }
  return s2_uv_cache[k].u;
}

/**
 * @brief Return the longitude of the corner of a face at the leaf indices
 * @p I and @p J, the double nearest that of the exact corner @p v
 * (#vector_lon_nearest)
 * @details On the faces 0 and 1 the longitude is atan2(u, 1) and
 * atan2(1, -u), and on the faces 3 and 4 atan2(-w, -1) and atan2(-1, w),
 * functions of one index, so a walk along a row of cells reads it again
 * from the cell before; those are kept by the face and that index, as
 * #s2cell_corner_uv keeps the coordinates
 */
static double
s2cell_corner_lon(uint32_t face, uint32_t I, uint32_t J, const int64 *v)
{
  if (face == 2 || face == 5)
    return vector_lon_nearest(v);
  uint32_t index = face < 2 ? I : J;
  int k = (int) (((index * UINT64CONST(0x9E3779B97F4A7C15)) ^ face) *
    UINT64CONST(0xBF58476D1CE4E5B9) >> 54) & (S2_CORNER_CACHE - 1);
  if (! s2_lon_cache[k].used || s2_lon_cache[k].face != face ||
      s2_lon_cache[k].index != index)
  {
    s2_lon_cache[k].lon = vector_lon_nearest(v);
    s2_lon_cache[k].face = face;
    s2_lon_cache[k].index = index;
    s2_lon_cache[k].used = true;
  }
  return s2_lon_cache[k].lon;
}

/**
 * @brief Return in @p lon and @p lat the degrees of the corner of a face at
 * the leaf indices @p I and @p J, the doubles nearest those of the exact
 * corner (#s2cell_corner_int, #s2cell_corner_lon, #vector_lat_nearest),
 * kept as #s2cell_corner_uv keeps the coordinates
 */
static void
s2cell_corner_lonlat(uint32_t face, uint32_t I, uint32_t J, double *lon,
  double *lat)
{
  /* The face, I and J take 3 + 31 + 31 bits, more than one word holds, so
   * an entry keeps the three and the hash only places it */
  uint64 h = ((I * UINT64CONST(0x9E3779B97F4A7C15)) ^ J) *
    UINT64CONST(0xBF58476D1CE4E5B9) + face;
  int k = (int) ((h * UINT64CONST(0x9E3779B97F4A7C15)) >> 54) &
    (S2_CORNER_CACHE - 1);
  if (! s2_vertex_cache[k].used || s2_vertex_cache[k].face != face ||
      s2_vertex_cache[k].I != I || s2_vertex_cache[k].J != J)
  {
    int64 v[3];
    s2cell_corner_int(face, I, J, v);
    s2_vertex_cache[k].lon = s2cell_corner_lon(face, I, J, v);
    s2_vertex_cache[k].lat = vector_lat_nearest(v);
    s2_vertex_cache[k].face = face;
    s2_vertex_cache[k].I = I;
    s2_vertex_cache[k].J = J;
    s2_vertex_cache[k].used = true;
  }
  *lon = s2_vertex_cache[k].lon;
  *lat = s2_vertex_cache[k].lat;
}

/**
 * @brief Return the leaf indices of the low corner of an S2 cell and its
 * size in leaves
 */
static uint32_t
s2cell_cell_leaf(S2CellId cell, uint32_t *ilo, uint32_t *jlo,
  uint32_t *size)
{
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  *size = 1u << (S2_MAX_LEVEL - s2cell_get_resolution(cell));
  *ilo = i & ~(*size - 1);
  *jlo = j & ~(*size - 1);
  return face;
}

/**
 * @brief Return the `(u, v)` extent of an S2 cell on its cube face
 * @details Each bound is the double nearest the exact coordinate of the
 * corner, read through the cache of #s2cell_corner_uv, the twin of
 * #h3_lonlat_vector, so every platform reads the same cell (decision D7 of
 * the cell walks)
 */
static uint32_t
s2cell_cell_uv(S2CellId cell, double *umin, double *vmin, double *umax,
  double *vmax)
{
  uint32_t ilo, jlo, size;
  uint32_t face = s2cell_cell_leaf(cell, &ilo, &jlo, &size);
  *umin = s2cell_corner_uv(ilo);
  *umax = s2cell_corner_uv(ilo + size);
  *vmin = s2cell_corner_uv(jlo);
  *vmax = s2cell_corner_uv(jlo + size);
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
  /* Each vertex is the exact corner of the cube, its degrees the doubles
   * nearest those of that corner (decision D7 of the cell walks) */
  uint32_t ilo, jlo, size;
  uint32_t face = s2cell_cell_leaf(cell, &ilo, &jlo, &size);
  const uint32_t is[4] = { ilo, ilo + size, ilo + size, ilo };
  const uint32_t js[4] = { jlo, jlo, jlo + size, jlo + size };
  bool west = false;
  for (int k = 0; k < 4; k++)
  {
    s2cell_corner_lonlat(face, is[k], js[k], &longitudes[k], &latitudes[k]);
    west = west || longitudes[k] < 0.0;
  }
  /* A corner on the antimeridian lies at 180 and at -180, and
   * #vector_lon_nearest reads it as 180: it takes the side of its cell, -180
   * where another vertex lies west of the antimeridian, so the longitudes of
   * a cell touching it are contiguous and its box states the cell alone */
  if (west)
    for (int k = 0; k < 4; k++)
      if (longitudes[k] == 180.0)
        longitudes[k] = -180.0;
  /* A corner at a pole has every longitude: its two edges run along the
   * meridians of the vertices beside it, so it takes that of the vertex
   * before it, and the longitudes of the cell are those of its other
   * vertices */
  for (int k = 0; k < 4; k++)
    if (latitudes[k] == 90.0 || latitudes[k] == -90.0)
      longitudes[k] = longitudes[(k + 3) % 4];
  return;
}

/**
 * @brief Return true if an S2 cell holds the edge of its ring from the
 * vertex k to the vertex k + 1
 * @details A cell holds the part of its ring facing the lower index
 * coordinates of its face: its low-i edge, `u = umin`, and its low-j edge,
 * `v = vmin`. #s2cell_cell_vertices orders the vertices `(umin,vmin)`,
 * `(umax,vmin)`, `(umax,vmax)`, `(umin,vmax)`, so these are the edges from
 * the vertices 3 and 0. An edge on a cube edge, where the axes of the two
 * faces point in different directions, still has one owner: it is the low
 * side of exactly one of the two faces. The decision reads the index alone.
 * @param[in] k Index of the first vertex of the edge, 0 to 3
 */
bool
s2cell_cell_edge_held(int k)
{
  return k == 0 || k == 3;
}

/**
 * @brief Return true if an S2 cell holds the vertex k of its ring
 * @details A cell holds the corner between its low-i and low-j edges, the
 * vertex 0 of #s2cell_cell_vertices. Of the eight corners of the cube, two
 * are the low corner of no cell, (1, -1, 1) and (-1, 1, -1), and each
 * belongs to the cell of the lowest face meeting there: the cell of face 0
 * at `i = 0` and the highest `j`, whose vertex 3 is (1, -1, 1), and the cell
 * of face 1 at the highest `i` and `j = 0`, whose vertex 1 is (-1, 1, -1).
 * The decision reads the index alone.
 * @param[in] cell S2 cell
 * @param[in] k Index of the vertex, 0 to 3
 */
bool
s2cell_cell_vertex_held(S2CellId cell, int k)
{
  if (k == 0)
    return true;
  if (k != 1 && k != 3)
    return false;
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  uint32_t size = 1u << (S2_MAX_LEVEL - s2cell_get_resolution(cell));
  uint32_t ilo = i & ~(size - 1), jlo = j & ~(size - 1);
  if (k == 3)
    return face == 0 && ilo == 0 && jlo + size == S2_MAX_SIZE;
  return face == 1 && ilo + size == S2_MAX_SIZE && jlo == 0;
}

/*****************************************************************************
 * The planar cells: their straight-edged outlines
 *****************************************************************************/

/**
 * @brief Return the cell of a level holding a pole, the north one when
 * @p north
 * @details At level 0 the cell of face 2 or 5 holds the pole inside it;
 * from level 1 the pole is a corner of the cells meeting there, the centre
 * `u = v = 0` of the face, and the cell holding it is the one whose low
 * corner it is, the leaf index 2^29 on both axes (#s2cell_cell_vertex_held)
 */
static S2CellId
s2cell_pole_cell(bool north, uint32_t level)
{
  uint32_t half = level == 0 ? 0 : (1u << (S2_MAX_LEVEL - 1));
  S2CellId result;
  s2cell_from_face_ij(north ? 2 : 5, half, half, level, &result);
  return result;
}

/**
 * @brief Return in the last argument the planar outline of an S2 cell,
 * false when its neighbours are not read
 * @details The vertices are those of #s2cell_cell_vertices, the exact
 * corners of the cube (decision D7), joined by straight edges in longitude
 * and latitude, and the cell across each edge is the one
 * #s2cell_edge_neighbors states in the order of the edges. A corner at a
 * pole, where both edges meeting there run along meridians, reads as two
 * vertices on the pole latitude at the longitudes of those meridians, the
 * vertices before and after it, and the edge between them, along the pole
 * latitude, lies across from no cell. The cell of level 0 of the faces 2
 * and 5 holds a pole, and is the region between its ring and that pole.
 * @param[in] cell S2 cell
 * @param[out] out Outline
 */
bool
s2cell_cell_outline(S2CellId cell, DggsOutline *out)
{
  int count;
  S2CellId *nb = s2cell_edge_neighbors(cell, &count);
  if (! nb)
    return false;
  double lon[4], lat[4];
  s2cell_cell_vertices(cell, lon, lat);
  uint32_t face = s2cell_get_face(cell);
  out->cell = cell;
  out->pole = (s2cell_get_resolution(cell) != 0 || (face != 2 && face != 5)) ?
    0 : (face == 2 ? 1 : -1);
  int n = 0;
  for (int k = 0; k < 4; k++)
  {
    if (lat[k] == 90.0 || lat[k] == -90.0)
    {
      out->lon[n] = lon[(k + 3) % 4];
      out->lat[n] = lat[k];
      out->across[n++] = 0;
      out->lon[n] = lon[(k + 1) % 4];
    }
    else
      out->lon[n] = lon[k];
    out->lat[n] = lat[k];
    out->across[n++] = nb[k];
  }
  out->count = n;
  pfree(nb);
  return true;
}

/**
 * @brief Return the index of the first of the two vertices on the pole
 * latitude of an outline, or -1 when it has no corner at a pole
 */
static int
s2cell_outline_pole_vertex(const DggsOutline *o)
{
  if (o->count != 5)
    return -1;
  for (int k = 0; k < 4; k++)
    if ((o->lat[k] == 90.0 || o->lat[k] == -90.0) && o->lat[k + 1] == o->lat[k])
      return k;
  return -1;
}

/**
 * @brief Return true if an S2 cell holds the edge of its planar outline from
 * the vertex k to the vertex k + 1
 * @details The edge of the ring it reads (#s2cell_cell_edge_held); the edge
 * along the pole latitude is the pole, held as the corner there is
 * (#s2cell_cell_vertex_held)
 */
bool
s2cell_outline_edge_held(const DggsOutline *o, int k)
{
  int p = s2cell_outline_pole_vertex(o);
  if (p < 0 || k < p)
    return s2cell_cell_edge_held(k);
  if (k == p)
    return s2cell_cell_vertex_held(o->cell, p);
  return s2cell_cell_edge_held(k - 1);
}

/**
 * @brief Return true if an S2 cell holds the vertex k of its planar outline
 * @details The corner of the ring it reads (#s2cell_cell_vertex_held); both
 * vertices on the pole latitude read the corner at the pole
 */
bool
s2cell_outline_vertex_held(const DggsOutline *o, int k)
{
  int p = s2cell_outline_pole_vertex(o);
  if (p < 0 || k < p)
    return s2cell_cell_vertex_held(o->cell, k);
  if (k <= p + 1)
    return s2cell_cell_vertex_held(o->cell, p);
  return s2cell_cell_vertex_held(o->cell, k - 1);
}

/** @brief Outline of the last cell #s2cell_cell_outline_last read, per
 * thread */
static MEOS_TLS DggsOutline s2_last_outline;
/** @brief Cell of #s2_last_outline, or 0 when none */
static MEOS_TLS S2CellId s2_last_outline_cell = 0;

/**
 * @brief Return the planar outline of a cell, or NULL when it is not read
 * @details As #h3_cell_outline_last keeps the outline of the last H3 cell:
 * consecutive positions of a trajectory mostly lie in one cell
 */
static const DggsOutline *
s2cell_cell_outline_last(uint64 cell)
{
  if (s2_last_outline_cell != cell)
  {
    s2_last_outline_cell = 0;
    if (! s2cell_cell_outline(cell, &s2_last_outline))
      return NULL;
    s2_last_outline_cell = cell;
  }
  return &s2_last_outline;
}

/** @brief The planar cell of a position for the planar walk,
 * #s2cell_point_cell_planar */
static uint64
s2cell_planar_point_cell(double lon, double lat, int32 level)
{
  return s2cell_point_cell_planar(lon, lat, level);
}

/**
 * @brief Return the cell a planar segment starting at a pole lies in just
 * after its start, or 0 when it starts at no pole or runs along its latitude
 * @details A position on a pole latitude is the pole, held by one cell
 * (#s2cell_pole_cell), while a segment leaving the pole lies first in the
 * cell whose edge along the pole latitude holds the longitude of its start:
 * from level 1 four cells meet at a pole, each spanning 90 degrees of
 * longitude there between two meridians. A start on one of those meridians
 * lies in the cell on the side the segment heads to in longitude, and a
 * segment running down the meridian lies in the cell holding that edge
 * (#s2cell_outline_edge_held). Each comparison is exact, the longitude of the
 * start read against those of the vertices through exact offsets of 360.
 * The outline turns counter-clockwise, so its edge along the pole latitude
 * runs west at the north pole and east at the south one.
 */
static uint64
s2cell_planar_start_cell(double lon1, double lat1, double lon2, double lat2,
  int32 level)
{
  if ((lat1 != 90.0 && lat1 != -90.0) || level == 0 || lat2 == lat1)
    return 0;
  bool north = lat1 > 0;
  uint32_t size = 1u << (S2_MAX_LEVEL - level);
  uint32_t half = 1u << (S2_MAX_LEVEL - 1);
  int dx = (lon2 > lon1) - (lon2 < lon1);
  for (int a = 0; a < 4; a++)
  {
    S2CellId cell;
    s2cell_from_face_ij(north ? 2 : 5, (a & 1) ? half - size : half,
      (a & 2) ? half - size : half, (uint32_t) level, &cell);
    const DggsOutline *o = s2cell_cell_outline_last(cell);
    int p = o ? s2cell_outline_pole_vertex(o) : -1;
    if (p < 0)
      continue;
    int n = o->count, before = (p + n - 1) % n, after = (p + 1) % n;
    /* The west and east ends of the edge along the pole latitude, and the
     * edges down the meridians there */
    double w = north ? o->lon[p + 1] : o->lon[p];
    double e = north ? o->lon[p] : o->lon[p + 1];
    int mw = north ? after : before, me = north ? before : after;
    double oe = dggs_lon_offset(w, e);
    /* The offset putting the start in [w, w + 360) */
    double os = 360.0 * floor((lon1 - w) / 360.0);
    while (offset_x_cmp(w, os, lon1) > 0)
      os -= 360.0;
    while (offset_x_cmp(w, os + 360.0, lon1) <= 0)
      os += 360.0;
    int cw = offset_x_cmp(w, os, lon1), ce = offset_x_cmp(e, oe + os, lon1);
    if (cw < 0 && ce > 0)
      return cell;
    if (cw == 0 && (dx > 0 || (dx == 0 && s2cell_outline_edge_held(o, mw))))
      return cell;
    if (ce == 0 && (dx < 0 || (dx == 0 && s2cell_outline_edge_held(o, me))))
      return cell;
  }
  return 0;
}

/** @brief The planar walk of S2: its outlines and its owner rule, a cell
 * holding its low-i and low-j edges and the corner between them
 * (#s2cell_outline_edge_held, #s2cell_outline_vertex_held), and the cell a
 * segment leaving a pole lies in (#s2cell_planar_start_cell) */
static const DggsPlanarOps s2cell_planar_ops =
{
  s2cell_cell_outline_last,
  s2cell_planar_point_cell,
  s2cell_outline_edge_held,
  s2cell_outline_vertex_held,
  s2cell_planar_start_cell
};

/**
 * @brief Return true if an S2 cell holds a position of the plane of
 * longitude and latitude
 * @details The region its planar outline bounds, with the boundary points it
 * owns (#dggs_outline_holds_point); a position on a pole latitude is the
 * pole, held by the cell whose held corner it is (#s2cell_pole_cell)
 * @param[in] o Outline of the cell
 * @param[in] x,y Longitude and latitude of the position, in degrees
 */
bool
s2cell_outline_holds_point(const DggsOutline *o, double x, double y)
{
  if (y == 90.0 || y == -90.0)
    return o->cell == s2cell_pole_cell(y > 0, s2cell_get_resolution(o->cell));
  return dggs_outline_holds_point(&s2cell_planar_ops, o, x, y);
}

/** @brief Number of rings of edge neighbours around the estimate the owner
 * of a position is looked for in, as #H3_POINT_CELL_RINGS for H3 */
#define S2_POINT_CELL_RINGS 16

/**
 * @brief Return the planar S2 cell of a level holding a position, or 0 when
 * none is found
 * @details A planar cell is its straight-edged outline
 * (#s2cell_outline_holds_point), which departs from the cell of S2, whose
 * edges are great-circle arcs and no straight lines in longitude and
 * latitude. The cell of S2 holding the position (#s2cell_point_to_cell) is
 * the estimate, and the cells around it, ring by ring of edge neighbours,
 * are asked in turn until one holds the position, as #h3_point_cell looks
 * for the planar H3 cell. A position on a pole latitude is the pole.
 * @param[in] lon,lat Longitude and latitude of the position, in degrees
 * @param[in] level S2 level
 */
S2CellId
s2cell_point_cell_planar(double lon, double lat, uint32_t level)
{
  if (lat == 90.0 || lat == -90.0)
    return s2cell_pole_cell(lat > 0, level);
  S2CellId c0 = s2cell_point_to_cell(lon, lat, level);
  if (c0 == 0)
    return c0;
  const DggsOutline *o = s2cell_cell_outline_last(c0);
  if (o && s2cell_outline_holds_point(o, lon, lat))
    return c0;
  /* The rings, each the edge neighbours of the one before not seen yet */
  int size = 64, nseen = 1, ring_lo = 0;
  S2CellId *seen = palloc(sizeof(S2CellId) * (size_t) size);
  seen[0] = c0;
  S2CellId result = 0;
  for (int r = 1; r <= S2_POINT_CELL_RINGS && ! result; r++)
  {
    int ring_hi = nseen;
    for (int a = ring_lo; a < ring_hi && ! result; a++)
    {
      int count;
      S2CellId *nb = s2cell_edge_neighbors(seen[a], &count);
      if (! nb)
        continue;
      for (int b = 0; b < count && ! result; b++)
      {
        bool known = false;
        for (int s = 0; s < nseen && ! known; s++)
          known = seen[s] == nb[b];
        if (known)
          continue;
        if (nseen == size)
        {
          size *= 2;
          seen = repalloc(seen, sizeof(S2CellId) * (size_t) size);
        }
        seen[nseen++] = nb[b];
        o = s2cell_cell_outline_last(nb[b]);
        if (o && s2cell_outline_holds_point(o, lon, lat))
          result = nb[b];
      }
      pfree(nb);
    }
    ring_lo = ring_hi;
  }
  pfree(seen);
  return result;
}

/**
 * @brief Fill `cells` with every planar S2 cell a planar segment crosses,
 * `enter` with the segment parameter at which it reaches each, and `cross`
 * with each crossing, by the walk on the straight-edged outlines
 * (#dggs_segment_cells_planar)
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] level S2 level
 * @param[out] cells,enter,cross Arrays of at least `maxout` entries, the
 * last one possibly NULL
 * @param[in] maxout Capacity of the arrays, at least 1
 * @return Number of cells written, or 0 when the first lookup fails
 */
int
s2cell_segment_cells_planar(double lon1, double lat1, double lon2,
  double lat2, uint32_t level, S2CellId *cells, double *enter,
  DggsEdgeCross *cross, int maxout)
{
  return dggs_segment_cells_planar(&s2cell_planar_ops, lon1, lat1, lon2,
    lat2, level, cells, enter, cross, maxout);
}

/*****************************************************************************
 * Cells crossed by a segment
 *****************************************************************************/

/**
 * @brief Set the last argument to the inward unit normal of the plane of each
 * edge of a cell, three coordinates each, the edge from vertex `k` at
 * `normals[3 * k]`
 * @details A cube face states a position as `W + u U + v V`, with `W`, `U`
 * and `V` the three axes of the face, each a signed unit axis vector. An edge
 * of a cell holds `u` or `v` fixed, so the plane through the centre of the
 * sphere containing that edge is spanned by the other axis and by the
 * position the fixed coordinate gives, and its normal is their cross product.
 * The plane states its own normal that way, exactly but for the single
 * multiplication by the fixed coordinate, whereas a normal read from the
 * rounded vertices of the cell states a nearby plane instead: a path running
 * along the true edge then reads a height above that nearby plane which is
 * not zero, and the walk leaves the cell through an edge it never crosses.
 *
 * #s2cell_cell_vertices orders the vertices `(umin,vmin)`, `(umax,vmin)`,
 * `(umax,vmax)`, `(umin,vmax)`, so the edges hold `v=vmin`, `u=umax`,
 * `v=vmax` and `u=umin` in turn. Each normal is taken on the side the centre
 * of the cell lies on, which is the side of its interior.
 */
static void
s2cell_cell_edge_normals(S2CellId cell, double *normals)
{
  double umin, vmin, umax, vmax;
  uint32_t face = s2cell_cell_uv(cell, &umin, &vmin, &umax, &vmax);
  /* The three axes of the face, as #s2cell_face_uv_to_xyz combines them */
  double w[3], du[3], dv[3];
  switch (face)
  {
    case 0:
      w[0] =  1.0; w[1] =  0.0; w[2] =  0.0;
      du[0] =  0.0; du[1] =  1.0; du[2] =  0.0;
      dv[0] =  0.0; dv[1] =  0.0; dv[2] =  1.0; break;
    case 1:
      w[0] =  0.0; w[1] =  1.0; w[2] =  0.0;
      du[0] = -1.0; du[1] =  0.0; du[2] =  0.0;
      dv[0] =  0.0; dv[1] =  0.0; dv[2] =  1.0; break;
    case 2:
      w[0] =  0.0; w[1] =  0.0; w[2] =  1.0;
      du[0] = -1.0; du[1] =  0.0; du[2] =  0.0;
      dv[0] =  0.0; dv[1] = -1.0; dv[2] =  0.0; break;
    case 3:
      w[0] = -1.0; w[1] =  0.0; w[2] =  0.0;
      du[0] =  0.0; du[1] =  0.0; du[2] = -1.0;
      dv[0] =  0.0; dv[1] = -1.0; dv[2] =  0.0; break;
    case 4:
      w[0] =  0.0; w[1] = -1.0; w[2] =  0.0;
      du[0] =  0.0; du[1] =  0.0; du[2] = -1.0;
      dv[0] =  1.0; dv[1] =  0.0; dv[2] =  0.0; break;
    default:
      w[0] =  0.0; w[1] =  0.0; w[2] = -1.0;
      du[0] =  0.0; du[1] =  1.0; du[2] =  0.0;
      dv[0] =  1.0; dv[1] =  0.0; dv[2] =  0.0; break;
  }
  /* The direction of the centre of the cell, which its interior lies on */
  double uc = 0.5 * (umin + umax), vc = 0.5 * (vmin + vmax), centre[3];
  for (int i = 0; i < 3; i++)
    centre[i] = w[i] + uc * du[i] + vc * dv[i];
  /* Each edge by its fixed coordinate and the axis it runs along */
  const double fixed[4] = { vmin, umax, vmax, umin };
  for (int k = 0; k < 4; k++)
  {
    /* an edge of even index holds `v` and runs along `u` */
    const double *along = (k % 2 == 0) ? du : dv;
    const double *axis = (k % 2 == 0) ? dv : du;
    double p[3];
    for (int i = 0; i < 3; i++)
      p[i] = w[i] + fixed[k] * axis[i];
    double *m = &normals[3 * k];
    m[0] = p[1] * along[2] - p[2] * along[1];
    m[1] = p[2] * along[0] - p[0] * along[2];
    m[2] = p[0] * along[1] - p[1] * along[0];
    if (m[0] * centre[0] + m[1] * centre[1] + m[2] * centre[2] < 0.0)
    {
      m[0] = -m[0]; m[1] = -m[1]; m[2] = -m[2];
    }
    double n = sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
    if (n > 0.0)
    {
      m[0] /= n; m[1] /= n; m[2] /= n;
    }
  }
  return;
}
/**
 * @brief Return where the great circle of a geodetic segment leaves a cell,
 * and in the last argument the edge of the cell boundary it crosses there
 * @details An S2 cell is convex: each of its edges lies on a line of constant
 * `u` or `v` of its cube face, which the gnomonic projection of the face maps
 * to a great circle through the origin, so the cell is the intersection of
 * the four hemispheres those circles bound. The path therefore leaves where
 * its own great circle first leaves one of them, found in closed form by
 * #dggs_arc_normals_exit_param. The exit lies strictly ahead of `tmin`, or
 * at `tmin` for a path on the boundary there and heading out, and never on
 * an edge the path entered the cell through.
 * @param[in] cell Cell
 * @param[in] entry Mask of the edges of the cell the path entered it through
 * @param[in] arc Great circle of the segment
 * @param[in] tmin Parameter at which the path entered the cell
 * @param[out] edge Edge of the cell boundary crossed at the exit
 * @return the path parameter of the exit, or a value above 1 when the path
 * ends inside the cell
 */
static double
s2cell_cell_exit_param(S2CellId cell, uint32 entry, const DggsArc *arc,
  double tmin, int *edge)
{
  double normals[12];
  s2cell_cell_edge_normals(cell, normals);
  return dggs_arc_normals_exit_param(arc, normals, 4, tmin, entry, edge);
}

/**
 * @brief Fill `cells` with every S2 cell a segment crosses, and `enter` with
 * the segment parameter at which it reaches each
 * @details The traversal of a grid by a line, as the split of a temporal
 * point by a space grid walks its tiles: from the cell in hand the path
 * leaves through the edge it crosses first, and the cell across that edge is
 * the next one, so each cell of the walk is a neighbour of the one before it
 * and no cell between the two is passed over, however short the chord the
 * path clips from it. A path never leaves a cell through the edge it entered
 * it by, as a line never crosses back over the tile boundary it has just
 * crossed.
 *
 * #s2cell_cell_vertices states the vertices of a cell in the order
 * `(umin,vmin)`, `(umax,vmin)`, `(umax,vmax)`, `(umin,vmax)`, so the edge
 * from vertex `k` to the next one is the `v=vmin`, `u=umax`, `v=vmax` and
 * `u=umin` side in turn; those four sides face the cells
 * #s2cell_edge_neighbors states in that same order, which is how the crossed
 * edge names the cell entered through it.
 *
 * A crossing is the parameter at which the path meets the edge, and a cell is
 * entered there. A path passing through a vertex, where the cells of a corner
 * meet, leaves through one of the two edges meeting there and reaches the
 * next cell from the second at the same parameter, as a line through the
 * corner of four tiles passes one of the two side tiles for no length.
 *
 * The walk follows the path the point moves along: the straight line in
 * longitude and latitude of a planar point, walked on the straight-edged
 * outlines of the cells by exact signs (#s2cell_segment_cells_planar), the
 * great circle of a geodetic one. A geodetic crossing is found on the
 * sphere, where a path across the antimeridian, over a pole or from one cube
 * face to the next is an arc like any other.
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
    return s2cell_segment_cells_planar(lon1, lat1, lon2, lat2, level, cells,
      enter, NULL, maxout);
  S2CellId cur = s2cell_point_to_cell(lon1, lat1, level);
  if (cur == (S2CellId) 0)
    return 0;
  cells[0] = cur; enter[0] = 0.0;
  int n = 1;

  DggsArc arc;
  if (! dggs_arc_init(lon1, lat1, lon2, lat2, &arc))
    return n;
  /* A cell is convex on the sphere, so a geodetic path whose far endpoint
   * lies in the cell of its near one never leaves it and there is no boundary
   * to find. That is the common case wherever the positions are closer
   * together than a cell is wide */
  if (s2cell_point_to_cell(lon2, lat2, level) == cur)
    return n;

  double t = 0.0;
  S2CellId prev = (S2CellId) 0;
  while (n < maxout)
  {
    int count = 0;
    S2CellId *nb = s2cell_edge_neighbors(cur, &count);
    if (! nb || count != 4)
    {
      if (nb)
        pfree(nb);
      break;
    }
    /* A great circle leaves a convex cell through the edge it came in by
     * only at the parameter it came in at, so the edge is held out of the
     * search */
    uint32 entry = 0;
    if (prev != (S2CellId) 0)
      for (int k = 0; k < 4; k++)
        if (nb[k] == prev)
        {
          entry = 1u << k;
          break;
        }
    int edge = -1;
    double texit = s2cell_cell_exit_param(cur, entry, &arc, t, &edge);
    if (texit > 1.0 || edge < 0)
    {
      pfree(nb);
      break;                 /* the segment ends inside this cell */
    }
    S2CellId next = nb[edge];
    pfree(nb);
    if (next == (S2CellId) 0 || next == cur)
      break;
    cells[n] = next;
    enter[n] = texit;
    n++;
    prev = cur;
    cur = next;
    t = texit;
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

/**
 * @brief Set the centre of a cell of @p level on the cube `[-n, n]^3`, where
 * `n` is the number of cells of the level along an edge of a face
 * @details The point #s2cell_face_uv_to_cube places, scaled by `n`. The
 * coordinates are integers: the one across the face is `n` or `-n` and those
 * along the face are `2 * c + 1 - n` for the column or the row `c` of the
 * cell, so the centres of two cells sharing an edge are 2 apart in the sum of
 * the absolute differences of their coordinates, on one face as across an
 * edge of the cube
 */
static void
s2cell_cube_centre(S2CellId cell, uint32_t level, int64 centre[3])
{
  uint32_t i, j;
  uint32_t face = s2cell_to_face_ij(cell, &i, &j, NULL);
  int64 n = INT64_C(1) << level;
  int64 col = i >> (S2_MAX_LEVEL - level), row = j >> (S2_MAX_LEVEL - level);
  /* Dividing by a power of 2 and multiplying back by it is exact */
  double xyz[3];
  s2cell_face_uv_to_cube(face, (double) (2 * col + 1 - n) / (double) n,
    (double) (2 * row + 1 - n) / (double) n, xyz);
  for (int k = 0; k < 3; k++)
    centre[k] = llround(xyz[k] * (double) n);
  return;
}

/**
 * @brief Return the grid distance of two S2 cells of one level, the number of
 * steps between edge neighbours from one to the other
 * @details A step to one of the edge neighbours of #s2cell_edge_neighbors
 * moves the centre of #s2cell_cube_centre by 2 in the sum of the absolute
 * differences of its coordinates, so half that sum bounds the distance from
 * below, and a path following the cube reaches the bound unless the two cells
 * lie on opposite faces. A path between opposite faces crosses one of the
 * four faces between them, going around the cube along one axis of the faces,
 * so the distance is the shortest of those four paths.
 * @param[in] origin,dest S2 cells
 * @return The distance
 * @errval -1
 * @note A distance is never negative, so -1 states an error
 */
int64
s2cell_grid_distance(S2CellId origin, S2CellId dest)
{
  uint32_t level1 = s2cell_get_resolution(origin);
  uint32_t level2 = s2cell_get_resolution(dest);
  if (level1 != level2)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The S2 cells must have the same level: %u, %u", level1, level2);
    return -1;
  }
  int64 p[3], q[3];
  s2cell_cube_centre(origin, level1, p);
  s2cell_cube_centre(dest, level1, q);
  uint32_t face1 = s2cell_get_face(origin), face2 = s2cell_get_face(dest);
  if (face1 == face2 || face1 % 3 != face2 % 3)
    return (llabs(p[0] - q[0]) + llabs(p[1] - q[1]) +
      llabs(p[2] - q[2])) / 2;
  /* The faces are opposite along the axis a, and the path around the cube
   * along the axis b goes by the face where b is -n or that where it is n */
  int64 n = INT64_C(1) << level1;
  uint32_t a = face1 % 3, b = (a + 1) % 3, c = (a + 2) % 3;
  int64 around_b = llabs(p[c] - q[c]) +
    Min(2 * n + p[b] + q[b], 2 * n - p[b] - q[b]);
  int64 around_c = llabs(p[b] - q[b]) +
    Min(2 * n + p[c] + q[c], 2 * n - p[c] - q[c]);
  return (2 * n + Min(around_b, around_c)) / 2;
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
 * @param[out] size_out Size of the result, the terminating null character
 * included
 * @return A palloc'd, null-terminated string (caller frees)
 * @csqlfn #S2cell_cell_to_token()
 */
char *
s2cell_cell_to_token(S2CellId cell, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  char buf[S2_TOKEN_MAXLEN + 1];
  snprintf(buf, sizeof(buf), "%016" PRIx64, (uint64) cell);
  size_t len = S2_TOKEN_MAXLEN;
  while (len > 0 && buf[len - 1] == '0')
    len--;
  char *result = palloc(len + 1);
  memcpy(result, buf, len);
  result[len] = '\0';
  return cstring_size(result, size_out);
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
 * @brief Parse a string into an S2 cell
 * @details See header for the accepted input shapes.
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
 * @csqlfn #S2cell_in(), #S2cell_from_text()
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
 * @param[out] size_out Size of the result, the terminating null character
 * included
 * @return A palloc'd, null-terminated string (caller frees)
 * @csqlfn #S2cell_out(), #S2cell_as_text()
 */
char *
s2cell_out(S2CellId cell, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  return s2cell_cell_to_token(cell, size_out);
}


/*****************************************************************************
 * WKB and HexWKB input/output
 *
 * A S2 cell is a cell of a grid over WGS84 (EPSG:4326), which the grid fixes,
 * so its Well-Known Binary (WKB) is the one of an h3index: the endian flag,
 * the SRID flag, the SRID for the extended (EWKB) variant only, then the cell
 * id. The functions mirror #h3index_from_wkb, #h3index_from_hexwkb,
 * #h3index_as_wkb and #h3index_as_hexwkb.
 *****************************************************************************/

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return a S2 cell from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #S2cell_from_wkb(), #S2cell_recv()
 */
S2CellId
s2cell_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, (S2CellId) 0);
  return (S2CellId) DatumGetInt64(type_from_wkb(wkb, size, T_S2CELL));
}

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return a S2 cell from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #S2cell_from_hexwkb()
 */
S2CellId
s2cell_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, (S2CellId) 0);
  size_t size = strlen(hexwkb);
  return (S2CellId) DatumGetInt64(type_from_hexwkb(hexwkb, size, T_S2CELL));
}

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a S2 cell
 * @param[in] cell Cell
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #S2cell_as_wkb(), #S2cell_send()
 */
uint8_t *
s2cell_as_wkb(S2CellId cell, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(Int64GetDatum(cell), T_S2CELL, variant, size_out);
}

/**
 * @ingroup meos_s2cell_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a S2 cell
 * @param[in] cell Cell
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #S2cell_as_hexwkb()
 */
char *
s2cell_as_hexwkb(S2CellId cell, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(Int64GetDatum(cell), T_S2CELL,
    variant | (uint8_t) WKB_HEX, size_out);
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
