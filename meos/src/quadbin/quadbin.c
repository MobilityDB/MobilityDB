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
 * @brief First-party implementation of the CARTO quadbin cell algebra
 * @details Quadbin is a square-quadtree Discrete Global Grid System packing a
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
 * bit 62      : header bit (always set)            -> QUADBIN_HEADER
 * bits 59..61 : mode (0..6; data cells use mode 1)
 * bits 52..56 : resolution / zoom (0..26)
 * bits 0..51  : interleaved (Morton) quadkey, low unused bits set to 1
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
#include "temporal/tcellindex.h"

/*****************************************************************************
 * Input/output
 *****************************************************************************/

/**
 * @brief Parse a string into a quadbin cell
 * @details See header for the accepted input shapes.
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
 * @brief Return a quadbin from its string representation
 * @param[in] str String
 * @csqlfn #Quadbin_in()
 */
Quadbin
quadbin_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, (Quadbin) 0);
  return quadbin_parse(str);
}

/**
 * @ingroup meos_quadbin_conversion
 * @brief Return a QUADBIN index from a 64-bit integer, raising an error when
 * the integer encodes no well-formed index
 * @param[in] i Integer
 * @csqlfn #Bigint_to_quadbin()
 */
Quadbin
bigint_to_quadbin(int64 i)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cell(Int64GetDatum(i), T_TQUADBIN))
    return (Quadbin) 0;
  return (Quadbin) i;
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
 * @ingroup meos_quadbin_base_comp
 * @brief Return the 32-bit hash value of a quadbin — matches the result
 * `hashint8` would produce on the same bit pattern
 * @csqlfn #Quadbin_hash()
 */
uint32
quadbin_hash(Quadbin cell)
{
  return int64_hash((int64) cell);
}

/**
 * @ingroup meos_quadbin_base_comp
 * @brief Return the 64-bit hash value of a quadbin using a seed — matches the
 * result `hashint8extended` would produce on the same bit pattern
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
 * @errval false
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

/** Latitude of the Web-Mercator limit, beyond which the first and the last
 * rows of the grid extend */
#define QUADBIN_MAX_LATITUDE  85.051128779806604

/**
 * @brief Set the fractional tile column and row of a lon/lat position in the
 * grid of `n` tiles a side
 */
static void
quadbin_tile_coords(double longitude, double latitude, double n, double *xf,
  double *yf)
{
  double lat = latitude;
  if (lat > QUADBIN_MAX_LATITUDE) lat = QUADBIN_MAX_LATITUDE;
  if (lat < -QUADBIN_MAX_LATITUDE) lat = -QUADBIN_MAX_LATITUDE;
  double lat_rad = lat * M_PI / 180.0;
  *xf = n * ((longitude + 180.0) / 360.0);
  *yf = n * (1.0 - (log(tan(lat_rad) + 1.0 / cos(lat_rad)) / M_PI)) / 2.0;
  return;
}

/**
 * @brief Return the tile holding a fractional tile coordinate, in the grid of
 * `n` tiles a side
 */
static long
quadbin_tile_index(double f, double n)
{
  long t = (long) floor(f);
  long maxt = (long) n - 1;
  if (t < 0)
    t = 0;
  else if (t > maxt)
    t = maxt;
  return t;
}

/**
 * @brief Return the latitude of the row boundary `y` of the grid of `n` tiles
 * a side
 * @details The boundary is read as #quadbin_cell_bounding_box reads the
 * latitudes of a cell
 */
static double
quadbin_row_latitude(double y, double n)
{
  return 180.0 / M_PI * atan(sinh(M_PI * (1.0 - 2.0 * y / n)));
}

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
  double n = (double) (UINT64_C(1) << resolution);
  double xf, yf;
  quadbin_tile_coords(longitude, latitude, n, &xf, &yf);
  return quadbin_tile_to_cell((uint32_t) quadbin_tile_index(xf, n),
    (uint32_t) quadbin_tile_index(yf, n), resolution);
}


/**
 * @brief Return where a geodetic path leaves the tile holding it, in the grid
 * of `n` tiles a side
 * @details A tile spans at most half the meridians, so it lies in the two
 * hemispheres bounded by the planes of its west and east meridians, and
 * between the planes of constant height of its north and south parallels. A
 * path inside every one of those half-spaces leaves the tile where it first
 * reaches any of their planes. The top and bottom rows reach the poles, since
 * a position beyond the latitude limit of the grid lies in them.
 * @return The path parameter of the exit, or a value above 1 when the path
 * ends inside the tile
 */
static double
quadbin_tile_exit_param_geodetic(const DggsArc *arc, uint32_t x, uint32_t y,
  double n, double tmin)
{
  double best = 2.0, t;
  if (n > 1.0)
  {
    for (int k = 0; k < 2; k++)
    {
      double lon = ((double) x + k) / n * 2.0 * M_PI - M_PI;
      const double m[3] = { -sin(lon), cos(lon), 0.0 };
      t = dggs_arc_plane_param(arc, m, 0.0, tmin);
      if (t < best)
        best = t;
    }
  }
  const double pole[3] = { 0.0, 0.0, 1.0 };
  if (y > 0)
  {
    t = dggs_arc_plane_param(arc, pole,
      sin(quadbin_row_latitude((double) y, n) * M_PI / 180.0), tmin);
    if (t < best)
      best = t;
  }
  if ((double) y + 1.0 < n)
  {
    t = dggs_arc_plane_param(arc, pole,
      sin(quadbin_row_latitude((double) y + 1.0, n) * M_PI / 180.0), tmin);
    if (t < best)
      best = t;
  }
  return best;
}

/**
 * @brief Return the cell holding the position a geodetic path reaches at a
 * parameter, or 0 when the position cannot be projected
 */
static Quadbin
quadbin_arc_cell(const DggsArc *arc, double t, uint32_t resolution)
{
  double lon, lat;
  if (! dggs_arc_point(arc, t, &lon, &lat))
    return (Quadbin) 0;
  return quadbin_point_to_cell(lon * 180.0 / M_PI, lat * 180.0 / M_PI,
    resolution);
}

/** @brief A geodetic path and the resolution its tiles are read at, the state
 * #dggs_crossing_param() reads a tile of the path from */
typedef struct
{
  const DggsArc *arc;        /**< Path of a geodetic segment */
  uint32_t resolution;       /**< Resolution of the grid */
} QuadbinArcAt;

/**
 * @brief Return the tile the path of a geodetic segment holds at a parameter,
 * or 0 when the position cannot be projected
 */
static uint64
quadbin_arc_cell_at(void *state, double t)
{
  const QuadbinArcAt *path = (const QuadbinArcAt *) state;
  return (uint64) quadbin_arc_cell(path->arc, t, path->resolution);
}

/**
 * @brief Return the angle subtended by the shortest side of a tile, in the
 * grid of `n` tiles a side
 */
static double
quadbin_tile_shortest_side(uint32_t y, double n)
{
  double north = quadbin_row_latitude((double) y, n) * M_PI / 180.0;
  double south = quadbin_row_latitude((double) y + 1.0, n) * M_PI / 180.0;
  double width = 2.0 * M_PI / n *
    cos(fabs(north) > fabs(south) ? north : south);
  double height = north - south;
  return (width < height) ? width : height;
}

/**
 * @brief Fill `cells` with every cell a geodetic segment crosses, and `enter`
 * with the segment parameter at which it reaches each
 * @details The walk of #s2cell_segment_cells: from the tile in hand the walk
 * leaves through its boundary, and the tile just beyond that crossing is a
 * neighbour of it, so no tile between the two is passed over. The crossing is
 * found on the sphere, where a path across the antimeridian or near a pole is
 * an arc like any other.
 */
static int
quadbin_arc_cells(double lon1, double lat1, double lon2, double lat2,
  uint32_t resolution, Quadbin *cells, double *enter, int maxout)
{
  Quadbin cur = quadbin_point_to_cell(lon1, lat1, resolution);
  cells[0] = cur; enter[0] = 0.0;
  int count = 1;
  /* A path ending in the tile it starts in may still leave it on the way: an
   * arc bulges toward the pole, across the parallel bounding a tile there, so
   * the walk runs whatever tile the far endpoint lies in */
  DggsArc arc;
  if (! dggs_arc_init(lon1, lat1, lon2, lat2, &arc))
    return count;
  double n = (double) (UINT64_C(1) << resolution);
  uint32_t x, y, z;

  /* A position on a tile boundary belongs to the one tile the grid assigns
   * it, and a path starting there moves into the neighbouring tile at once,
   * through a crossing no search strictly ahead of the start states: the walk
   * leaves from the tile just past the start, entered where the halving of
   * #dggs_crossing_param() places the crossing */
  QuadbinArcAt path = { .arc = &arc, .resolution = resolution };
  double t = 0.0;
  quadbin_cell_tile(cur, &x, &y, &z);
  double t0 = quadbin_tile_shortest_side(y, n) * 1e-4 / arc.dist;
  if (t0 < 1.0)
  {
    Quadbin first = quadbin_arc_cell(&arc, t0, resolution);
    if (first != (Quadbin) 0 && first != cur && count < maxout)
    {
      cells[count] = first;
      enter[count++] = dggs_crossing_param(0.0, t0, cur, &quadbin_arc_cell_at,
        &path);
      cur = first;
      t = t0;
    }
  }
  while (count < maxout)
  {
    quadbin_cell_tile(cur, &x, &y, &z);
    double texit = quadbin_tile_exit_param_geodetic(&arc, x, y, n, t);
    if (texit > 1.0)
      break;                 /* the segment ends inside this tile */
    /* A nudge past the crossing lands inside the next tile without reaching
     * the one after it: a ten-thousandth of the shortest side of the tile is
     * far below the width of a neighbouring tile and far above the rounding
     * of the crossing itself */
    double nudge = quadbin_tile_shortest_side(y, n) * 1e-4 / arc.dist;
    double tn = texit + nudge, tin = texit;
    Quadbin next = (Quadbin) 0;
    /* A nudge that lands back in the tile just left says the crossing sits
     * within its own rounding, so widen it rather than stall. A crossing
     * nearer the end of the segment than the nudge reads the tile of the end,
     * which is the tile the path enters there */
    for (int k = 0; k < 8; k++)
    {
      if (tn > 1.0)
        tn = 1.0;
      next = quadbin_arc_cell(&arc, tn, resolution);
      if (next == (Quadbin) 0 || next != cur || tn >= 1.0)
        break;
      tin = tn;
      tn += nudge * (double) (1 << k);
    }
    if (next == (Quadbin) 0)
      break;                 /* the position cannot be projected */
    if (next == cur)
    {
      /* A path past a crossing still in the tile is one the rounding places
       * on a boundary plane the path runs along or touches, so the walk goes
       * on from that crossing */
      t = texit;
      continue;
    }
    /* A crossing the path is still in the tile past is one the rounding of a
     * boundary the path runs along places where the path is: the probes
     * bracket the crossing, and halving the bracket closes on it */
    cells[count] = next;
    enter[count++] = (tin > texit) ?
      dggs_crossing_param(tin, tn, cur, &quadbin_arc_cell_at, &path) : texit;
    cur = next;
    t = tn;
  }
  return count;
}

/**
 * @brief Fill `cells` with every cell a segment crosses, and `enter` with the
 * segment parameter at which it reaches each
 * @details A traversal, not a sampling walk. The segment is the straight line
 * in longitude and latitude a planar point moves along between two instants.
 * The tile column is linear in the longitude and the tile row monotonic in
 * the latitude, so the parameter at which the path leaves its tile through a
 * column or a row boundary follows in closed form, and the walk steps to the
 * nearer of the two. Each step moves to an adjacent tile, so no tile the path
 * crosses is passed over. A tile holds its west and north boundaries, so the
 * point where four tiles meet belongs to the tile east of the meridian and
 * south of the parallel meeting there; a path through that point passes
 * through that tile when it lies beside the corner, and then enters the tile
 * diagonally across. The walk ends in the tile holding the second endpoint.
 *
 * A geodetic segment follows the great circle through its endpoints, and is
 * traversed tile by tile on the sphere as #quadbin_arc_cells states.
 * @param[in] lon1,lat1,lon2,lat2 Segment endpoints in degrees
 * @param[in] geodetic True when the segment is geodetic
 * @param[in] resolution Quadbin resolution
 * @param[out] cells,enter Arrays of at least `maxout` entries; `enter[0]` is
 *   always 0, the parameter of the first endpoint
 * @param[in] maxout Capacity of both arrays. A planar walk needs one more
 *   than the column and the row distances between the endpoint tiles
 * @return Number of cells written, which is `maxout` when the arrays fill
 * before the segment ends
 */
int
quadbin_segment_cells(double lon1, double lat1, double lon2, double lat2,
  bool geodetic, uint32_t resolution, Quadbin *cells, double *enter,
  int maxout)
{
  assert(cells); assert(enter); assert(resolution <= QUADBIN_MAX_RESOLUTION);
  if (maxout < 1)
    return 0;
  if (geodetic)
    return quadbin_arc_cells(lon1, lat1, lon2, lat2, resolution, cells, enter,
      maxout);
  double n = (double) (UINT64_C(1) << resolution);
  double x0, y0, x1, y1;
  quadbin_tile_coords(lon1, lat1, n, &x0, &y0);
  quadbin_tile_coords(lon2, lat2, n, &x1, &y1);
  long tx = quadbin_tile_index(x0, n), ty = quadbin_tile_index(y0, n);
  long ex = quadbin_tile_index(x1, n), ey = quadbin_tile_index(y1, n);
  cells[0] = quadbin_tile_to_cell((uint32_t) tx, (uint32_t) ty, resolution);
  enter[0] = 0.0;
  int count = 1;

  /* The row grows southward, as the Mercator ordinate falls */
  long stepx = (ex > tx) ? 1 : -1, stepy = (ey > ty) ? 1 : -1;
  double dlon = lon2 - lon1, dlat = lat2 - lat1;
  double t = 0.0;
  while ((tx != ex || ty != ey) && count < maxout)
  {
    /* The parameters at which the path reaches the next column boundary and
     * the next row boundary in the direction it travels */
    double tcol = HUGE_VAL, trow = HUGE_VAL;
    if (tx != ex && dlon != 0.0)
    {
      double bx = (double) ((stepx > 0) ? tx + 1 : tx);
      tcol = (bx / n * 360.0 - 180.0 - lon1) / dlon;
    }
    if (ty != ey && dlat != 0.0)
    {
      double by = (double) ((stepy > 0) ? ty + 1 : ty);
      trow = (quadbin_row_latitude(by, n) - lat1) / dlat;
    }
    if (tcol == HUGE_VAL && trow == HUGE_VAL)
      break;
    /* The crossings follow one another along the path, an order the
     * rounding of the two boundary forms does not reverse */
    double tnext = (tcol < trow) ? tcol : trow;
    if (tnext < t)
      tnext = t;
    if (tnext > 1.0)
      tnext = 1.0;
    t = tnext;
    if (tcol < trow)
      tx += stepx;
    else if (trow < tcol)
      ty += stepy;
    else
    {
      /* The path passes through the point where four tiles meet, which
       * belongs to the tile east of the meridian and south of the parallel
       * meeting there. Beside the corner, that tile holds the path at the
       * crossing before the tile diagonally across does */
      long ox = (stepx > 0) ? tx + 1 : tx, oy = (stepy > 0) ? ty + 1 : ty;
      if ((ox != tx || oy != ty) && (ox != tx + stepx || oy != ty + stepy))
      {
        cells[count] = quadbin_tile_to_cell((uint32_t) ox, (uint32_t) oy,
          resolution);
        enter[count++] = t;
        if (count >= maxout)
          break;
      }
      tx += stepx;
      ty += stepy;
    }
    cells[count] = quadbin_tile_to_cell((uint32_t) tx, (uint32_t) ty,
      resolution);
    enter[count++] = t;
  }
  return count;
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
