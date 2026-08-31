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
 * @brief First-party implementation of the CARTO quadbin cell algebra.
 *
 * Quadbin is a square-quadtree Discrete Global Grid System packing a
 * Web-Mercator slippy-tile (z, x, y) into a 64-bit integer. This file is the
 * MobilityDB-owned counterpart of libh3: it provides the static-cell kernel
 * that the temporal `tquadbin` layer lifts over time. It links only libm.
 *
 * The bit layout and the integer functions are transcribed from the public
 * CARTO quadbin specification, reference implementation `quadbin-py`
 * (https://github.com/CartoDB/quadbin-py), released under the BSD-3-Clause
 * license, Copyright (c) 2022 CARTO. This is a first-party re-implementation
 * from that public specification — not a vendored copy — so it carries no
 * vendor split and no .codacy.yml exclusion.
 *
 * Cell layout (most-significant bit first):
 *   bit 62      : header bit (always set)            -> QUADBIN_HEADER
 *   bits 59..61 : mode (0..6; data cells use mode 1)
 *   bits 52..56 : resolution / zoom (0..26)
 *   bits 0..51  : interleaved (Morton) quadkey, low unused bits set to 1
 */

#include "quadbin/quadbin.h"

/* C */
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
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Input/output
 *****************************************************************************/

/**
 * @brief Parse a string into a quadbin cell. See header for the accepted
 * input shapes.
 */
Quadbin
quadbin_parse(const char *str)
{
  assert(str);

  /* Strip leading whitespace. */
  while (*str && isspace((unsigned char) *str))
    str++;

  /* Skip an optional "0x" / "0X" hex prefix; the canonical quadbin
   * output is unprefixed lowercase hex. */
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    str += 2;

  Quadbin cell = quadbin_string_to_index(str);

  /* Reject anything that does not encode a well-formed quadbin index. The
   * mode a well-formed index carries is a property of the value rather than
   * of the type: a quadbin holds an index of any mode, as an h3index holds a
   * cell, a vertex or a directed edge, and #quadbin_is_valid_cell is what
   * answers whether the value at hand is a data cell. Validating the mode
   * here makes that question unaskable for this grid alone. */
  if (! quadbin_is_valid_index(cell))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "quadbin value \"%s\" does not encode a valid quadbin index", str);
    return (Quadbin) 0;
  }

  return cell;
}

/**
 * @ingroup meos_quadbin_base_inout
 * @brief Return a quadbin from its string representation.
 * @param[in] str String
 * @csqlfn #Quadbin_in()
 */
Quadbin
quadbin_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, (Quadbin) 0);
  // return quadbin_parse(&str, true); // TODO
  return quadbin_parse(str);
}

/*****************************************************************************
 * Comparison / ordering
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if two quadbin values are equal
 * @csqlfn #Quadbin_eq()
 */
bool
quadbin_eq(Quadbin a, Quadbin b)
{
  return a == b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if two quadbin values are not equal
 * @csqlfn #Quadbin_ne()
 */
bool
quadbin_ne(Quadbin a, Quadbin b)
{
  return a != b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is less than the second
 * @csqlfn #Quadbin_lt()
 */
bool
quadbin_lt(Quadbin a, Quadbin b)
{
  return a < b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is less than or equal to
 * the second
 * @csqlfn #Quadbin_le()
 */
bool
quadbin_le(Quadbin a, Quadbin b)
{
  return a <= b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than the second
 * @csqlfn #Quadbin_gt()
 */
bool
quadbin_gt(Quadbin a, Quadbin b)
{
  return a > b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than or equal
 * to the second
 * @csqlfn #Quadbin_ge()
 */
bool
quadbin_ge(Quadbin a, Quadbin b)
{
  return a >= b;
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return -1 / 0 / 1 depending on whether the first quadbin is
 * less than, equal to, or greater than the second
 * @csqlfn #Quadbin_cmp()
 */
int
quadbin_cmp(Quadbin a, Quadbin b)
{
  return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/*****************************************************************************
 * Hashing
 *****************************************************************************/

/**
 * @ingroup meos_quadbin_base_accessor
 * @brief Return the 32-bit hash value of a quadbin — matches the result
 * `hashint8` would produce on the same bit pattern.
 * @csqlfn #Quadbin_hash()
 */
uint32
quadbin_hash(Quadbin cell)
{
  return int64_hash((int64) cell);
}

/**
 * @ingroup meos_quadbin_base_accessor
 * @brief Return the 64-bit hash value of a quadbin using a seed — matches
 * the result `hashint8extended` would produce on the same bit pattern.
 * @param[in] cell Quadbin cell
 * @param[in] seed Seed
 * @csqlfn #Quadbin_hash_extended()
 */
uint64
quadbin_hash_extended(Quadbin cell, uint64 seed)
{
  return int64_hash_extended((int64) cell, seed);
}

/*****************************************************************************
 * Bit-layout constants (verbatim from quadbin-py)
 *****************************************************************************/

#define QUADBIN_HEADER  UINT64_C(0x4000000000000000)
#define QUADBIN_FOOTER  UINT64_C(0x000FFFFFFFFFFFFF)  /* 52 low bits set */
#define QUADBIN_MAX_RESOLUTION  26

/* Morton interleave masks B[0..5] and shift amounts S[0..4] */
static const uint64_t QB_B[6] = {
  UINT64_C(0x5555555555555555),
  UINT64_C(0x3333333333333333),
  UINT64_C(0x0F0F0F0F0F0F0F0F),
  UINT64_C(0x00FF00FF00FF00FF),
  UINT64_C(0x0000FFFF0000FFFF),
  UINT64_C(0x00000000FFFFFFFF)
};
static const uint32_t QB_S[5] = { 1, 2, 4, 8, 16 };

/*****************************************************************************
 * Tile (z/x/y) <-> cell conversion
 *
 * Transcribed bit-for-bit from quadbin-py main.py tile_to_cell / cell_to_tile.
 * No H3 analogue: H3 has no Web-Mercator tile address.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the quadbin cell of a Web-Mercator tile
 * @param[in] x,y Tile column and row at zoom @p z
 * @param[in] z Zoom / resolution (0..26)
 * @csqlfn #Quadbin_tile_to_cell()
 */
Quadbin
quadbin_tile_to_cell(uint32_t x, uint32_t y, uint32_t z)
{
  uint64_t xx = (uint64_t) x << (32 - z);
  uint64_t yy = (uint64_t) y << (32 - z);

  xx = (xx | (xx << QB_S[4])) & QB_B[4];
  yy = (yy | (yy << QB_S[4])) & QB_B[4];
  xx = (xx | (xx << QB_S[3])) & QB_B[3];
  yy = (yy | (yy << QB_S[3])) & QB_B[3];
  xx = (xx | (xx << QB_S[2])) & QB_B[2];
  yy = (yy | (yy << QB_S[2])) & QB_B[2];
  xx = (xx | (xx << QB_S[1])) & QB_B[1];
  yy = (yy | (yy << QB_S[1])) & QB_B[1];
  xx = (xx | (xx << QB_S[0])) & QB_B[0];
  yy = (yy | (yy << QB_S[0])) & QB_B[0];

  return QUADBIN_HEADER | (UINT64_C(1) << 59) | ((uint64_t) z << 52) |
    ((xx | (yy << 1)) >> 12) | (QUADBIN_FOOTER >> (z * 2));
}

/**
 * @brief Return the Web-Mercator tile of a quadbin cell
 * @param[in] cell Quadbin cell
 * @param[out] x,y,z Tile column, row, and zoom
 */
void
quadbin_cell_tile(Quadbin cell, uint32_t *x, uint32_t *y, uint32_t *z)
{
  assert(x); assert(y); assert(z);

  uint32_t zz = (cell >> 52) & 31;
  uint64_t q = (cell & QUADBIN_FOOTER) << 12;
  uint64_t xx = q;
  uint64_t yy = q >> 1;

  xx = xx & QB_B[0];
  yy = yy & QB_B[0];
  xx = (xx | (xx >> QB_S[0])) & QB_B[1];
  yy = (yy | (yy >> QB_S[0])) & QB_B[1];
  xx = (xx | (xx >> QB_S[1])) & QB_B[2];
  yy = (yy | (yy >> QB_S[1])) & QB_B[2];
  xx = (xx | (xx >> QB_S[2])) & QB_B[3];
  yy = (yy | (yy >> QB_S[2])) & QB_B[3];
  xx = (xx | (xx >> QB_S[3])) & QB_B[4];
  yy = (yy | (yy >> QB_S[3])) & QB_B[4];
  xx = (xx | (xx >> QB_S[4])) & QB_B[5];
  yy = (yy | (yy >> QB_S[4])) & QB_B[5];

  xx = xx >> (32 - zz);
  yy = yy >> (32 - zz);

  *x = (uint32_t) xx;
  *y = (uint32_t) yy;
  *z = zz;
}

/**
 * @ingroup meos_quadbin
 * @brief Return the Web-Mercator tile of a quadbin cell
 * @param[in] cell Quadbin cell
 * @param[out] x,y,z Tile column, row, and zoom
 * @return On error return @p false
 * @csqlfn #Quadbin_cell_to_tile()
 */
bool
quadbin_cell_to_tile(Quadbin cell, uint32_t *x, uint32_t *y, uint32_t *z)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(x, false); VALIDATE_NOT_NULL(y, false);
  VALIDATE_NOT_NULL(z, false);
  quadbin_cell_tile(cell, x, y, z);
  return true;
}

/**
 * @ingroup meos_quadbin
 * @brief Return the canonical base-4 quadkey string of a quadbin cell
 * @details The quadkey is the slippy-tile identifier: one base-4 digit per
 * zoom level, from coarsest to finest. Digit `d` at a level packs the tile
 * column bit in `d & 1` and the tile row bit in `d & 2`. A zoom-0 cell has the
 * empty quadkey.
 * @param[in] cell Quadbin cell
 * @return A palloc'd, null-terminated string of `z` characters (caller frees)
 * @csqlfn #Quadbin_cell_to_quadkey()
 */
char *
quadbin_cell_to_quadkey(Quadbin cell)
{
  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  char *result = palloc(z + 1);
  for (uint32_t i = 0; i < z; i++)
  {
    /* Most-significant tile bit first (coarsest zoom level first). */
    uint32_t shift = z - 1 - i;
    char digit = '0';
    if ((x >> shift) & 1U)
      digit += 1;
    if ((y >> shift) & 1U)
      digit += 2;
    result[i] = digit;
  }
  result[z] = '\0';
  return result;
}

/*****************************************************************************
 * Inspection
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the resolution (zoom) of a quadbin cell
 * @csqlfn #Quadbin_get_resolution()
 */
uint32_t
quadbin_get_resolution(Quadbin cell)
{
  return (cell >> 52) & 0x1F;
}

/*****************************************************************************
 * Hierarchy
 *
 * cell_to_parent / cell_to_children transcribed bit-for-bit from quadbin-py.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the parent of a quadbin cell at a coarser resolution
 * @param[in] cell Quadbin cell
 * @param[in] parent_resolution Target resolution (<= resolution of @p cell)
 * @return The parent cell, or 0 if @p parent_resolution is invalid
 * @csqlfn #Quadbin_cell_to_parent()
 */
Quadbin
quadbin_cell_to_parent(Quadbin cell, uint32_t parent_resolution)
{
  uint32_t resolution = quadbin_get_resolution(cell);
  if (parent_resolution > resolution)
    return 0;
  return (cell & ~(UINT64_C(0x1F) << 52)) |
    ((uint64_t) parent_resolution << 52) |
    (QUADBIN_FOOTER >> (parent_resolution << 1));
}

/**
 * @ingroup meos_quadbin
 * @brief Return the (exactly four-per-level) children of a quadbin cell
 * @param[in] cell Quadbin cell
 * @param[in] children_resolution Target resolution (> resolution of @p cell)
 * @param[out] count Number of children returned
 * @return A palloc'd array of children cells, or NULL on invalid resolution
 * @csqlfn #Quadbin_cell_to_children()
 */
Quadbin *
quadbin_cell_to_children(Quadbin cell, uint32_t children_resolution,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(count, NULL);

  uint32_t resolution = (cell >> 52) & 0x1F;
  if (children_resolution > QUADBIN_MAX_RESOLUTION ||
      children_resolution <= resolution)
  {
    *count = 0;
    return NULL;
  }

  uint32_t resolution_diff = children_resolution - resolution;
  uint64_t block_range = UINT64_C(1) << (resolution_diff << 1);
  uint32_t block_shift = 52 - (children_resolution << 1);

  uint64_t child_base = (cell & ~(UINT64_C(0x1F) << 52)) |
    ((uint64_t) children_resolution << 52);
  child_base = child_base & ~((block_range - 1) << block_shift);

  Quadbin *children = palloc(sizeof(Quadbin) * block_range);
  for (uint64_t i = 0; i < block_range; i++)
    children[i] = child_base | (i << block_shift);
  *count = (int) block_range;
  return children;
}

/**
 * @ingroup meos_quadbin
 * @brief Return the sibling cell in a cardinal direction
 * @param[in] cell Quadbin cell
 * @param[in] direction One of "up", "down", "left", "right"
 * @return The sibling cell, or 0 on an unknown direction
 * @csqlfn #Quadbin_cell_sibling()
 */
Quadbin
quadbin_cell_sibling(Quadbin cell, const char *direction)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(direction, (Quadbin) 0);

  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  if (strcmp(direction, "up") == 0)
    y -= 1;
  else if (strcmp(direction, "down") == 0)
    y += 1;
  else if (strcmp(direction, "left") == 0)
    x -= 1;
  else if (strcmp(direction, "right") == 0)
    x += 1;
  else
    return 0;
  return quadbin_tile_to_cell(x, y, z);
}

/*****************************************************************************
 * Grid traversal
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the cells within grid distance @p k of @p cell (square ring)
 * @param[in] cell Quadbin cell
 * @param[in] k Ring radius (Chebyshev distance, >= 0)
 * @param[out] count Number of cells returned ((2k+1)^2)
 * @return A palloc'd array of cells
 */
Quadbin *
quadbin_k_ring(Quadbin cell, int k, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(count, NULL);

  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  int side = 2 * k + 1;
  int n = side * side;
  Quadbin *result = palloc(sizeof(Quadbin) * n);
  int idx = 0;
  for (int dy = -k; dy <= k; dy++)
    for (int dx = -k; dx <= k; dx++)
      result[idx++] = quadbin_tile_to_cell((uint32_t) (x + dx),
        (uint32_t) (y + dy), z);
  *count = n;
  return result;
}

/*****************************************************************************
 * Lat/Lng (Web-Mercator slippy-tile math)
 *
 * Implements the standard Web-Mercator slippy-tile transforms (OSM
 * "Slippy map tilenames"); the CARTO quadbin-py point_to_cell /
 * cell_to_point / cell_to_bounding_box use the same transforms.
 *****************************************************************************/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @ingroup meos_quadbin
 * @brief Return the quadbin cell containing a lon/lat point at a resolution
 * @csqlfn #Quadbin_point_to_cell()
 */
Quadbin
quadbin_point_to_cell(double longitude, double latitude, uint32_t resolution)
{
  if (resolution == 0)
    return quadbin_tile_to_cell(0, 0, 0);
  /* Clamp latitude to the Web-Mercator limit */
  double lat = latitude;
  if (lat > 85.051128779806604) lat = 85.051128779806604;
  if (lat < -85.051128779806604) lat = -85.051128779806604;

  double n = (double) (UINT64_C(1) << resolution);
  double lat_rad = lat * M_PI / 180.0;
  double xf = n * ((longitude + 180.0) / 360.0);
  double yf = n * (1.0 - (log(tan(lat_rad) + 1.0 / cos(lat_rad)) / M_PI)) / 2.0;

  long xt = (long) floor(xf);
  long yt = (long) floor(yf);
  long maxt = (long) n - 1;
  if (xt < 0) xt = 0; else if (xt > maxt) xt = maxt;
  if (yt < 0) yt = 0; else if (yt > maxt) yt = maxt;
  return quadbin_tile_to_cell((uint32_t) xt, (uint32_t) yt, resolution);
}

/**
 * @brief Return the lon/lat centroid of a quadbin cell
 * @details Out-parameter kernel; the geometry projection cellToPoint
 * is backed by quadbin_cell_to_geompoint in quadbin_geo.c.
 * @param[in] cell Quadbin cell
 * @param[out] longitude Longitude of the centroid
 * @param[out] latitude Latitude of the centroid
 */
void
quadbin_cell_point(Quadbin cell, double *longitude, double *latitude)
{
  assert(longitude); assert(latitude);

  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  double n = (double) (UINT64_C(1) << z);
  *longitude = (x + 0.5) / n * 360.0 - 180.0;
  double yr = M_PI * (1.0 - 2.0 * (y + 0.5) / n);
  *latitude = 180.0 / M_PI * atan(sinh(yr));
}

/**
 * @brief Return the lon/lat bounding box (xmin, ymin, xmax, ymax) of a cell
 * @details Out-parameter kernel shared by the geometry boundary projection
 * cellToBoundary (via quadbin_cell_to_geom in quadbin_geo.c) and the
 * stbox(quadbin) cast (via quadbin_set_stbox).
 * @param[in] cell Quadbin cell
 * @param[out] xmin Minimum X coordinate
 * @param[out] ymin Minimum Y coordinate
 * @param[out] xmax Maximum X coordinate
 * @param[out] ymax Maximum Y coordinate
 */
void
quadbin_cell_bounding_box(Quadbin cell, double *xmin, double *ymin,
  double *xmax, double *ymax)
{
  assert(xmin); assert(ymin); assert(xmax); assert(ymax);

  uint32_t x, y, z;
  quadbin_cell_tile(cell, &x, &y, &z);
  double n = (double) (UINT64_C(1) << z);
  *xmin = (double) x / n * 360.0 - 180.0;
  *xmax = (double) (x + 1) / n * 360.0 - 180.0;
  /* y grows southward: tile y -> ymax, tile y+1 -> ymin */
  double yr_top = M_PI * (1.0 - 2.0 * (double) y / n);
  double yr_bot = M_PI * (1.0 - 2.0 * (double) (y + 1) / n);
  *ymax = 180.0 / M_PI * atan(sinh(yr_top));
  *ymin = 180.0 / M_PI * atan(sinh(yr_bot));
}

/**
 * @ingroup meos_quadbin
 * @brief Return the area in square meters of a quadbin cell (WGS84 sphere)
 * @csqlfn #Quadbin_cell_area()
 */
double
quadbin_cell_area(Quadbin cell)
{
  double xmin, ymin, xmax, ymax;
  quadbin_cell_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
  /* Spherical quadrangle area: R^2 * |lon2-lon1| * |sin(lat2)-sin(lat1)| */
  const double R = 6371007.180918475; /* authalic radius (m) */
  double dlon = (xmax - xmin) * M_PI / 180.0;
  double s1 = sin(ymin * M_PI / 180.0);
  double s2 = sin(ymax * M_PI / 180.0);
  double a = R * R * dlon * fabs(s2 - s1);
  return a;
}

/*****************************************************************************
 * Validity
 *
 * Structural checks derived from the bit layout: header bits, mode, and
 * resolution/footer consistency.
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return true if @p index is a structurally valid quadbin index
 * @csqlfn #Quadbin_is_valid_index()
 */
bool
quadbin_is_valid_index(Quadbin index)
{
  if ((index & QUADBIN_HEADER) != QUADBIN_HEADER)
    return false;
  uint32_t mode = (index >> 59) & 7;
  if (mode > 6)
    return false;
  uint32_t resolution = (index >> 52) & 0x1F;
  if (resolution > QUADBIN_MAX_RESOLUTION)
    return false;
  /* The low (52 - 2*resolution) bits must all be set to 1 (the footer) */
  uint64_t filler = QUADBIN_FOOTER >> (resolution << 1);
  return (index & filler) == filler;
}

/**
 * @ingroup meos_quadbin
 * @brief Return true if @p cell is a valid quadbin data cell (mode 1)
 * @csqlfn #Quadbin_is_valid_cell()
 */
bool
quadbin_is_valid_cell(Quadbin cell)
{
  return quadbin_is_valid_index(cell) && (((cell >> 59) & 7) == 1);
}

/*****************************************************************************
 * Serialization — lowercase hexadecimal of the 64-bit index
 *****************************************************************************/

/**
 * @ingroup meos_quadbin
 * @brief Return the lowercase hexadecimal string of a quadbin index
 * @return A palloc'd, null-terminated string (caller frees)
 * @csqlfn #Quadbin_out()
 */
char *
quadbin_index_to_string(Quadbin index)
{
  char *result = palloc(17);
  snprintf(result, 17, "%016" PRIx64, (uint64_t) index);
  return result;
}

/**
 * @ingroup meos_quadbin
 * @brief Parse a hexadecimal string into a quadbin index
 */
Quadbin
quadbin_string_to_index(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, (Quadbin) 0);
  return (Quadbin) strtoull(str, NULL, 16);
}

/*****************************************************************************/
