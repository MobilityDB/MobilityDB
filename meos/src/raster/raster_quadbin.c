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
 * @brief MEOS kernel for sampling Raquet raster chips along tgeompoint
 * trajectories
 * @details Each Raquet tile is identified by a CARTO QUADBIN cell (uint64)
 * whose
 * Morton-encoded x/y/z coordinates define the Web-Mercator bounding box and
 * pixel grid without any external metadata.  The pixel-to-coordinate mapping
 * uses the standard slippy-tile Mercator transform:
 *
 * col = floor((lon − xmin) / (xmax − xmin) × width)
 * row = floor((top_merc − merc(lat)) / (top_merc − bot_merc) × height)
 *
 * where merc(lat) = ln(tan(π/4 + lat·π/360)) in radians.
 *
 * The implementation is self-contained (no tquadbin PR dependency): the
 * QUADBIN Morton decode and bbox math are inlined from the same slippy-tile
 * formulas used in meos/src/quadbin/quadbin.c so both share identical
 * numerical behaviour.
 */

/* C */
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
/* liblwgeom (vendored) */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/tcellindex.h"
#include "geo/geo_funcs.h"
#include "raster/raquet.h"
#include "raster/raster_quadbin.h"

/*****************************************************************************
 * QUADBIN helpers (self-contained Morton decode + bbox, matching quadbin.c)
 *****************************************************************************/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Bit-layout constants — identical to meos/src/quadbin/quadbin.c */
#define QB_HEADER  UINT64_C(0x4000000000000000)
#define QB_FOOTER  UINT64_C(0x000FFFFFFFFFFFFF)
#define QB_MODE    (UINT64_C(1) << 59)   /* spatial mode bit */

/* Half-extent of the EPSG:3857 (Web-Mercator) plane in metres: pi * 6378137
 * (the WGS-84 semi-major axis). The world spans [-QB_MERC_MAX, +QB_MERC_MAX]
 * on both axes; QUADBIN tile (x, y, z) covers 2*QB_MERC_MAX / 2^z metres. */
#define QB_MERC_MAX  20037508.342789244

/* Highest zoom level of the QUADBIN grid: its identifier holds the zoom in five
 * bits and the tile in the Morton code of the 52 bits below them, two bits per
 * level. The zoom bounds the shift building the tile grid, so it is validated
 * before use */
#define QB_MAX_ZOOM  26

/** Latitude the Web-Mercator grid reaches, beyond which a tile has no extent */
#define QB_MAX_LATITUDE  85.051129

static const uint64_t QB_B[6] = {
  UINT64_C(0x5555555555555555), UINT64_C(0x3333333333333333),
  UINT64_C(0x0F0F0F0F0F0F0F0F), UINT64_C(0x00FF00FF00FF00FF),
  UINT64_C(0x0000FFFF0000FFFF), UINT64_C(0x00000000FFFFFFFF)
};

/**
 * @brief Morton-decode a QUADBIN cell into Web-Mercator tile coordinates
 * @details Canonical compact_bits algorithm matching CARTO quadbin-js
 * quadbinCellToTile: extract even/odd bits, compact right (shifts 1→2→4→8→16),
 * descale from 2^26.
 */
static void
qb_to_xyz(uint64_t cell, uint32_t *tx, uint32_t *ty, uint32_t *tz)
{
  uint32_t zz = (uint32_t)((cell >> 52) & 31);
  uint64_t q  = cell & QB_FOOTER;    /* 52-bit Morton code; no shift */
  uint64_t xx = q        & QB_B[0]; /* compact x: even bit positions */
  uint64_t yy = (q >> 1) & QB_B[0]; /* compact y: odd bit positions  */
  xx = (xx | (xx >>  1)) & QB_B[1];
  xx = (xx | (xx >>  2)) & QB_B[2];
  xx = (xx | (xx >>  4)) & QB_B[3];
  xx = (xx | (xx >>  8)) & QB_B[4];
  xx = (xx | (xx >> 16)) & QB_B[5];
  yy = (yy | (yy >>  1)) & QB_B[1];
  yy = (yy | (yy >>  2)) & QB_B[2];
  yy = (yy | (yy >>  4)) & QB_B[3];
  yy = (yy | (yy >>  8)) & QB_B[4];
  yy = (yy | (yy >> 16)) & QB_B[5];
  /* Descale from 2^26 grid to zoom-level tile coordinates */
  *tx = (uint32_t)(xx >> (26 - zz));
  *ty = (uint32_t)(yy >> (26 - zz));
  *tz = zz;
}

/**
 * @brief Morton-encode tile (x, y, z) into a QUADBIN cell
 * @details Canonical spread_bits algorithm matching CARTO quadbin-js
 * quadbinTileToCell: scale to 2^26 grid, spread left (shifts 16→8→4→2→1),
 * interleave x/y, and set to one the 52 - 2z bits of the Morton code below
 * the zoom, which a QUADBIN cell carries as its footer and which the QUADBIN
 * family's #quadbin_tile_to_cell() sets in the same way.
 */
static uint64_t
xyz_to_qb(uint32_t tx, uint32_t ty, uint32_t tz)
{
  /* Scale tile coords to the 2^26 (MAX_ZOOM) grid */
  uint64_t xx = (uint64_t)tx * (UINT64_C(1) << (26 - tz));
  uint64_t yy = (uint64_t)ty * (UINT64_C(1) << (26 - tz));
  /* spread_bits: expand each coord bit into alternating positions */
  xx = (xx | (xx << 16)) & QB_B[4];
  xx = (xx | (xx <<  8)) & QB_B[3];
  xx = (xx | (xx <<  4)) & QB_B[2];
  xx = (xx | (xx <<  2)) & QB_B[1];
  xx = (xx | (xx <<  1)) & QB_B[0];
  yy = (yy | (yy << 16)) & QB_B[4];
  yy = (yy | (yy <<  8)) & QB_B[3];
  yy = (yy | (yy <<  4)) & QB_B[2];
  yy = (yy | (yy <<  2)) & QB_B[1];
  yy = (yy | (yy <<  1)) & QB_B[0];
  return QB_HEADER | QB_MODE | ((uint64_t)tz << 52) | (xx | (yy << 1)) |
    (QB_FOOTER >> (tz * 2));
}

/**
 * @brief Return true if @p cell is a cell of the QUADBIN tile grid
 * @details A cell carries the header bit, the cell mode, a zoom of the grid,
 * and the bits of the Morton code below the zoom set to one, the rule
 * #quadbin_is_valid_index() of the QUADBIN family states for any index, here
 * restricted to the cell mode a tile key has
 * @param[in] cell QUADBIN identifier
 */
bool
raster_quadbin_is_cell(uint64 cell)
{
  if ((cell & QB_HEADER) != QB_HEADER || ((cell >> 59) & 7) != 1)
    return false;
  uint32_t zoom = (uint32_t) ((cell >> 52) & 0x1F);
  if (zoom > QB_MAX_ZOOM)
    return false;
  uint64_t footer = QB_FOOTER >> (zoom * 2);
  return (cell & footer) == footer;
}

/**
 * @brief Compute the WGS-84 bounding box and Mercator top/bottom of a tile
 * @details The Mercator top/bot values are kept in the caller-visible
 * representation to avoid recomputing them in the hot per-instant loop.
 */
static void
qb_bbox(uint32_t tx, uint32_t ty, uint32_t tz, double *xmin, double *xmax,
  double *ymin, double *ymax, double *top_merc, double *bot_merc)
{
  double n = (double)(UINT64_C(1) << tz);
  *xmin = (double) tx       / n * 360.0 - 180.0;
  *xmax = (double)(tx + 1)  / n * 360.0 - 180.0;
  *top_merc = M_PI * (1.0 - 2.0 * (double) ty       / n);
  *bot_merc = M_PI * (1.0 - 2.0 * (double)(ty + 1)  / n);
  *ymax = 180.0 / M_PI * atan(sinh(*top_merc));
  *ymin = 180.0 / M_PI * atan(sinh(*bot_merc));
}

/**
 * @brief Return the WGS-84 bounding box of the tile identified by a QUADBIN
 * cell
 * @param[in] cell QUADBIN cell
 * @param[out] xmin,ymin,xmax,ymax Longitude and latitude bounds in degrees
 * @note The raster family carries its own Morton decode and tile arithmetic, so
 * the tile footprint is available whether or not the QUADBIN family is built
 */
void
raster_quadbin_bounds(uint64 cell, double *xmin, double *ymin, double *xmax,
  double *ymax)
{
  assert(xmin); assert(ymin); assert(xmax); assert(ymax);
  uint32_t tx, ty, tz;
  qb_to_xyz(cell, &tx, &ty, &tz);
  double top_merc, bot_merc;
  qb_bbox(tx, ty, tz, xmin, xmax, ymin, ymax, &top_merc, &bot_merc);
}

/**
 * @brief Return the zoom level of the tile identified by a QUADBIN cell
 * @param[in] cell QUADBIN cell
 * @details A higher zoom covers less ground with the same pixel grid, so of two
 * tiles covering a point the one with the higher zoom carries the finer
 * resolution
 */
uint32_t
raster_quadbin_zoom(uint64 cell)
{
  uint32_t tx, ty, tz;
  qb_to_xyz(cell, &tx, &ty, &tz);
  return tz;
}

/**
 * @brief Derive the QUADBIN cell of a Web-Mercator raster tile from its
 * EPSG:3857 georeferencing
 * @details A Raquet tile is a single QUADBIN cell of the Web-Mercator tile 
 * pyramid, so its EPSG:3857 origin and pixel resolution determine the cell 
 * exactly. Thepixel extent gives the zoom (a tile of zoom
 * @p z spans 2*QB_MERC_MAX / 2^z metres); the top-left origin gives the tile
 * column and row. The raster must be a single axis-aligned Web-Mercator tile:
 * a non-square extent, an extent that is not a power-of-two fraction of the
 * world, or an origin off the tile grid are rejected (mixing georeferencing
 * that is not a QUADBIN tile is an error, not a value to coerce).
 * @param[in] origin_x,origin_y Top-left corner of the raster in EPSG:3857 metres
 * @param[in] pixel_w Pixel width in metres (west-east resolution, > 0)
 * @param[in] pixel_h Pixel height in metres (north-south resolution, may be < 0)
 * @param[in] xsize,ysize Raster dimensions in pixels
 * @param[out] result Derived QUADBIN cell
 * @return true on success; on failure sets a MEOS error and returns false
 */
bool
raster_quadbin_from_bounds(double origin_x, double origin_y, double pixel_w,
  double pixel_h, int xsize, int ysize, uint64 *result)
{
  double ext_x = (double) xsize * pixel_w;
  double ext_y = (double) ysize * fabs(pixel_h);
  if (ext_x <= 0.0 || ext_y <= 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster has a non-positive extent; cannot derive its QUADBIN cell");
    return false;
  }
  /* A QUADBIN tile is square. The extent either IS the tile's or it is not:
   * a band around the difference accepts a raster that is not a tile and
   * answers a tile for it, which is the coercion this function refuses */
  if (ext_x != ext_y)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster is not a square Web-Mercator tile; cannot derive its QUADBIN cell");
    return false;
  }
  /* Zoom z: the tile extent is 2*QB_MERC_MAX / 2^z metres */
  double world = 2.0 * QB_MERC_MAX;
  double zf = log2(world / ext_x);
  long z = lround(zf);
  if ((double) z != zf || z < 0 || z > 26)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster extent does not match a QUADBIN zoom level");
    return false;
  }
  /* Exact tile side at this zoom, and the tile column/row from the origin */
  double side = world / (double) (UINT64_C(1) << z);
  double txf = (origin_x + QB_MERC_MAX) / side;
  double tyf = (QB_MERC_MAX - origin_y) / side;
  long tx = lround(txf);
  long ty = lround(tyf);
  if ((double) tx != txf || (double) ty != tyf)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster origin is not aligned to the QUADBIN tile grid");
    return false;
  }
  long ntiles = (long) (UINT64_C(1) << z);
  if (tx < 0 || tx >= ntiles || ty < 0 || ty >= ntiles)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster origin is outside the Web-Mercator tile grid");
    return false;
  }
  *result = xyz_to_qb((uint32_t) tx, (uint32_t) ty, (uint32_t) z);
  return true;
}

/*****************************************************************************
 * Pixel reader
 *****************************************************************************/

/**
 * @brief Return in the last argument the pixel value at (col, row) of a
 * row-major byte array
 * @return true on success; on failure sets a MEOS error and returns false
 */
static bool
read_pixel(const uint8_t *pixels, int col, int row, int width,
  MeosPixType pixtype, double *result)
{
  return raquet_pixel_value(pixels, (size_t) row * width + col, pixtype,
    result);
}

/*****************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/**
 * @brief State a Raquet tile sampling call keeps for the length of a
 * trajectory
 * @details The state holds the pixel array, the layout its QUADBIN cell fixes,
 * and whether a pixel held a value the sampling surface cannot carry.
 */
typedef struct
{
  const uint8_t *pixels;
  int32 width;
  int32 height;
  MeosPixType pixtype;
  double nodata;
  bool has_nodata;
  bool undomainable;
  double xmin, xmax, ymin, ymax;   /**< Tile bounds in lon/lat */
  double top_merc, bot_merc;       /**< Tile bounds in Mercator metres */
  bool exact;                      /**< Whether the pixels are QUADBIN tiles,
                                        the width and height powers of 2 */
  double ncols, nrows;             /**< Columns and rows of those tiles over
                                        the world */
  double col0, row0;               /**< Column and row of the first pixel */
} RaquetSampleState;

/**
 * @brief Raquet grid callback placing a position in the pixels of a tile
 * @details The column is linear in longitude and the row is linear in the
 * Mercator ordinate, which is what the tile's own georeferencing states
 */
static void
raquet_grid(const void *ctxp, double x, double y, double *col, double *row)
{
  const RaquetSampleState *state = (const RaquetSampleState *) ctxp;
  *col = (x - state->xmin) / (state->xmax - state->xmin) * state->width;
  double merc_y = log(tan(M_PI / 4.0 + y * M_PI / 360.0));
  *row = (state->top_merc - merc_y) / (state->top_merc - state->bot_merc) *
    state->height;
  return;
}

/**
 * @brief Raquet pixel callback reading one pixel of a tile
 */
static bool
raquet_pixel(void *ctxp, int col, int row, double *value)
{
  RaquetSampleState *state = (RaquetSampleState *) ctxp;
  double pixval;
  if (! read_pixel(state->pixels, col, row, state->width, state->pixtype,
      &pixval))
  {
    state->undomainable = true;
    return false;
  }
  /* A pixel holding NaN or an infinity carries no value, as a pixel equal
   * to the nodata value does: GDAL writes NaN as the nodata value of a float
   * band, which no comparison matches, and every MEOS value is finite */
  if (! isfinite(pixval) || (state->has_nodata && pixval == state->nodata))
    return false;
  *value = pixval;
  return true;
}

/**
 * @brief Raquet crossing callback returning the parameter at which a segment
 * reaches a column line or a row line of a tile
 * @details The column is linear in the longitude, so a column line sits at a
 * longitude. The row follows the Mercator ordinate, so a row line sits at
 * the latitude the Gudermannian gives for its ordinate, as
 * #qb_lat_at_tile_y() places a tile row. The segment is straight in lon/lat,
 * so it reaches either line at the parameter of that longitude or latitude.
 * The edges of the tile are its bounds themselves, which #qb_bbox() computes
 * from the integer tile coordinates, so a tile and its neighbour place the
 * edge they share at the same double and a trip leaves one at the instant it
 * enters the other.
 */
static double
raquet_cross(const void *ctxp, double x1, double y1, double x2, double y2,
  int axis, double k)
{
  const RaquetSampleState *state = (const RaquetSampleState *) ctxp;
  if (axis == 0)
  {
    double lon = (k <= 0.0) ? state->xmin :
      ((k >= (double) state->width) ? state->xmax :
        state->xmin + k / state->width * (state->xmax - state->xmin));
    return (lon - x1) / (x2 - x1);
  }
  double lat;
  if (k <= 0.0)
    lat = state->ymax;
  else if (k >= (double) state->height)
    lat = state->ymin;
  else
  {
    double merc = state->top_merc - k / state->height *
      (state->top_merc - state->bot_merc);
    lat = (atan(exp(merc)) - M_PI / 4.0) * 360.0 / M_PI;
  }
  return (lat - y1) / (y2 - y1);
}

/**
 * @brief Return the parameter at which a segment reaches a grid line of a
 * raster georeferenced by an affine geotransform
 * @details The line is placed in world coordinates and met there, as a
 * geometric restriction of the trajectory meets a boundary. The grid
 * coordinates of the endpoints would state the same parameter, but the
 * inverse geotransform reaches a grid coordinate near zero by subtracting two
 * magnitudes of the order of the world coordinates, and the error that leaves
 * behind is enough to place a crossing that falls on a microsecond one
 * microsecond before it. The column line @p k runs from
 * (gt[0] + k gt[1], gt[3] + k gt[4]) in the direction (gt[2], gt[5]), and the
 * row line @p k from (gt[0] + k gt[2], gt[3] + k gt[5]) in the direction
 * (gt[1], gt[4]). A raster without skew has vertical column lines and
 * horizontal row lines, and the parameter is then the quotient of the
 * distances along one axis. Two rasters tiled from one grid place the edge
 * they share at the same double, so a trip leaves one at the instant it
 * enters the other.
 * @param[in] gt Geotransform of the raster, in the GDAL order
 * @param[in] x1,y1,x2,y2 Endpoints of the segment, in the reference system of
 * the raster
 * @param[in] axis 0 for a column line, 1 for a row line
 * @param[in] k Grid line
 */
double
raster_affine_cross(const double *gt, double x1, double y1, double x2,
  double y2, int axis, double k)
{
  double bx, by, dx, dy;
  if (axis == 0)
  {
    bx = gt[0] + k * gt[1];
    by = gt[3] + k * gt[4];
    dx = gt[2];
    dy = gt[5];
    if (dx == 0.0)
      return (bx - x1) / (x2 - x1);
  }
  else
  {
    bx = gt[0] + k * gt[2];
    by = gt[3] + k * gt[5];
    dx = gt[1];
    dy = gt[4];
    if (dy == 0.0)
      return (by - y1) / (y2 - y1);
  }
  return ((bx - x1) * dy - (by - y1) * dx) /
    ((x2 - x1) * dy - (y2 - y1) * dx);
}

/**
 * @brief Return the side of a position of the grid line @p k of a grid that
 * is the affine map of its geotransform, exactly, see #grid_line_side()
 */
int
raster_affine_side(const double *gt, double x, double y, int axis, int k)
{
  return grid_line_side(x, y, gt, axis, (double) k);
}

/**
 * @brief Return the microsecond at which a segment crosses the grid line
 * @p k of a grid that is the affine map of its geotransform, exactly, see
 * #segment_grid_line_cross_offset()
 */
int64
raster_affine_offset(const double *gt, double x1, double y1, double x2,
  double y2, int64 D, int axis, double k, bool *whole)
{
  return segment_grid_line_cross_offset(x1, y1, x2, y2, D, gt, axis, k,
    whole);
}

/**
 * @brief Return the longitude of the column line @p k of a Raquet tile whose
 * pixels are QUADBIN tiles, or the latitude of its row line @p k
 * @details The pixels of a tile of 2^b columns of a QUADBIN cell of zoom z
 * are the QUADBIN tiles of zoom z + b, and their bounds are the doubles that
 * place a QUADBIN tile: a column line at the exact longitude x / n 360 - 180
 * and a row line at the double nearest the latitude of the Mercator ordinate
 * 1 - 2 y / n (#mercator_lat_nearest), n a power of 2
 */
static double
raquet_line(const RaquetSampleState *state, int axis, double k)
{
  if (axis == 0)
    return (state->col0 + k) / state->ncols * 360.0 - 180.0;
  return mercator_lat_nearest(1.0 - 2.0 * (state->row0 + k) / state->nrows);
}

/**
 * @brief Raquet side callback: the sign of the grid coordinate of a position
 * less the grid line @p k, from the bounds of #raquet_line compared as
 * doubles, a pixel holding its west and north bounds, as a QUADBIN tile does
 */
static int
raquet_side(const void *ctxp, double x, double y, int axis, int k)
{
  const RaquetSampleState *state = (const RaquetSampleState *) ctxp;
  double line = raquet_line(state, axis, k);
  /* The rows grow southward, so a row coordinate exceeds the line where the
   * latitude lies below it */
  if (axis == 0)
    return (x > line) - (x < line);
  return (line > y) - (line < y);
}

/**
 * @brief Raquet offset callback: the microsecond at which a segment crosses
 * the meridian or the parallel of the grid line @p k, dated by
 * #segment_edge_cross_offset on the bound of #raquet_line, as
 * #tquadbin_cross_offset dates the crossing of a QUADBIN tile
 */
static int64
raquet_offset(const void *ctxp, double x1, double y1, double x2, double y2,
  int64 D, int axis, double k, bool *whole)
{
  const RaquetSampleState *state = (const RaquetSampleState *) ctxp;
  double line = raquet_line(state, axis, k);
  if (axis == 0)
    return segment_edge_cross_offset(x1, y1, x2, y2, D, line, 0.0, 0.0, line,
      1.0, 0.0, whole);
  return segment_edge_cross_offset(x1, y1, x2, y2, D, 0.0, line, 0.0, 1.0,
    line, 0.0, whole);
}

/**
 * @brief Fill the grid descriptor of a Raquet tile
 * @details Mirrors #dggs_cellops(), which answers the descriptor of a DGGS:
 * an engine states its grid in one place, and every sampling entry point
 * reads it from there.
 * @param[in] state Sampling state of the tile, which the descriptor carries
 * @param[out] ops Descriptor of the tile grid
 */
static void
raquet_gridops(RaquetSampleState *state, RasterGridOps *ops)
{
  ops->grid = &raquet_grid;
  ops->pixel = &raquet_pixel;
  ops->point = NULL;
  ops->data = NULL;
  ops->cross = &raquet_cross;
  /* A tile whose sides are powers of 2 reads its pixels as QUADBIN tiles,
   * exactly; another reads them from its grid coordinates */
  ops->side = state->exact ? &raquet_side : NULL;
  ops->offset = state->exact ? &raquet_offset : NULL;
  ops->ctx = state;
  ops->width = state->width;
  ops->height = state->height;
  memset(&ops->box, 0, sizeof(STBox));
  ops->box.xmin = state->xmin; ops->box.xmax = state->xmax;
  ops->box.ymin = state->ymin; ops->box.ymax = state->ymax;
  return;
}

/**
 * @ingroup meos_raster_base_accessor
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory
 * @details The chip is identified by its QUADBIN cell, which encodes the
 * Web-Mercator tile coordinates and thus the full georeferencing without any
 * separate metadata. A position outside the tile extent or on a nodata pixel
 * carries no value, and NULL is returned when the trajectory never meets a
 * pixel carrying data.
 * @note The sampling surface is double-valued, whatever the pixel type of the
 * band: a pixel type belongs here when every value it can hold is exactly
 * representable in a double, which is what lets a band of any type be sampled
 * into one temporal float and tiles of different types be read by one query.
 * @param[in] pixels Row-major pixel bytes (all bands interleaved or
 * single-band depending on the Raquet producer), little-endian beyond one byte
 * a pixel
 * @param[in] traj Input tgeompoint trajectory (SRID 4326)
 * @param[in] pixels_size Number of bytes available at @p pixels
 * @param[in] width Tile width in pixels (typically 256)
 * @param[in] height Tile height in pixels (typically 256)
 * @param[in] quadbin CARTO QUADBIN cell identifier (uint64)
 * @param[in] pixtype Pixel data type
 * @param[in] nodata Nodata sentinel value
 * @param[in] has_nodata Whether nodata filtering is active
 * @return A temporal float, or NULL
 * @csqlfn #Raster_tile_value_quadbin()
 */
Temporal *
raster_tile_value_quadbin(const Temporal *traj, const uint8_t *pixels,
  size_t pixels_size, int32 width, int32 height, Quadbin quadbin,
  MeosPixType pixtype, double nodata, bool has_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(traj, NULL); VALIDATE_NOT_NULL(pixels, NULL);
  if (! raster_quadbin_is_cell(quadbin))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The QUADBIN value %016" PRIx64 " of a raquet tile is not a cell of the grid",
      quadbin);
    return NULL;
  }
  /* The dimensions are taken in the type the SQL surface uses and validated
   * before the narrowing to the tile's uint16 fields, so that a value outside
   * that range is rejected here instead of wrapping to a different tile than
   * the one asked for */
  if (width <= 0 || height <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raquet tile must be positive");
    return NULL;
  }
  if (width > UINT16_MAX || height > UINT16_MAX)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The width and height of a raquet tile must be at most %d: %d x %d",
      UINT16_MAX, width, height);
    return NULL;
  }
  size_t need = (size_t) width * height * raquet_pixtype_size(pixtype);
  if (pixels_size < need)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "The pixel array has %zu bytes but %zu are required for a %d x %d tile",
      pixels_size, need, width, height);
    return NULL;
  }

  /* Decode QUADBIN → tile x, y, z → WGS84 bbox + Mercator top/bot */
  uint32_t tx, ty, tz;
  qb_to_xyz(quadbin, &tx, &ty, &tz);

  RaquetSampleState state;
  qb_bbox(tx, ty, tz, &state.xmin, &state.xmax, &state.ymin, &state.ymax,
    &state.top_merc, &state.bot_merc);
  state.pixels = pixels;
  state.width = width;
  state.height = height;
  state.pixtype = pixtype;
  state.nodata = nodata;
  state.has_nodata = has_nodata;
  state.undomainable = false;
  /* The pixels of a tile whose sides are powers of 2 are the QUADBIN tiles
   * of that many more zoom levels */
  state.exact = (width & (width - 1)) == 0 && (height & (height - 1)) == 0;
  double n = (double) (UINT64_C(1) << tz);
  state.ncols = n * (double) width;
  state.nrows = n * (double) height;
  state.col0 = (double) tx * (double) width;
  state.row0 = (double) ty * (double) height;

  RasterGridOps ops;
  raquet_gridops(&state, &ops);
  Temporal *result = raster_value_sampler(traj, &ops);
  if (state.undomainable)
  {
    /* The band holds a value the sampling surface cannot carry, so the
     * result would stand for a number the band does not hold */
    if (result)
      pfree(result);
    return NULL;
  }
  return result;
}

/**
 * @brief Return the index of the grid cell holding a grid coordinate, a
 * coordinate outside the grid naming the cell just outside it
 * @details A position may lie arbitrarily far outside the grid, where every
 * cell carries no value. Folding those cells into the one beside the grid
 * keeps the index in the range of an integer, and lets a traversal cross the
 * whole outside in one step that lands exactly on the edge of the grid.
 * @param[in] g Grid coordinate
 * @param[in] n Number of cells along the axis
 */
static int
raster_cell_index(double g, int n)
{
  /* The negated test also sends a NaN outside */
  if (! (g >= 0.0))
    return -1;
  if (g >= (double) n)
    return n;
  return (int) floor(g);
}

/**
 * @brief Return the index along an axis of the pixel holding a position, a
 * position outside the grid naming the index just outside it, see
 * #raster_cell_index()
 * @details On a grid whose engine decides the side of a grid line exactly
 * (the affine map of a geotransform, #raster_affine_side, or the QUADBIN
 * tiles of a Raquet tile) the index read from the grid coordinate in doubles
 * is an estimate, corrected against the exact side of the position of each
 * grid line, so a position on a line or an ulp beside it lies in the pixel
 * that holds it: a pixel holds the lines of its lower column and row.
 * Another grid reads the index from the grid coordinate
 * @param[in] ops Grid
 * @param[in] x,y Position
 * @param[in] g Grid coordinate of the position along the axis
 * @param[in] axis 0 for the columns, 1 for the rows
 * @param[in] n Number of pixels along the axis
 */
static int
raster_axis_index(const RasterGridOps *ops, double x, double y, double g,
  int axis, int n)
{
  int c = raster_cell_index(g, n);
  if (! ops->side)
    return c;
  while (c >= 0 && ops->side(ops->ctx, x, y, axis, c) < 0)
    c--;
  while (c < n && ops->side(ops->ctx, x, y, axis, c + 1) >= 0)
    c++;
  return c;
}

/**
 * @brief Return the value of the pixel a position falls in
 * @details The function returns false when the position lies outside the
 * pre-filter box, outside the pixel grid, or on a nodata pixel.
 */
static bool
raster_sample_at(const RasterGridOps *ops, double x, double y,
  double *value)
{
  /* The box pre-filters a grid whose pixel is read from its coordinates;
   * on a grid decided exactly, whose box is rounded, the exact index decides */
  const STBox *box = &ops->box;
  if (! ops->side &&
      (x < box->xmin || x > box->xmax || y < box->ymin || y > box->ymax))
    return false;
  double gcol, grow;
  ops->grid(ops->ctx, x, y, &gcol, &grow);
  int col = raster_axis_index(ops, x, y, gcol, 0, ops->width);
  int row = raster_axis_index(ops, x, y, grow, 1, ops->height);
  /* The half-open pixel convention: a position on the far edge of the grid
   * belongs to the cell beyond it */
  if (col < 0 || col >= ops->width || row < 0 || row >= ops->height)
    return false;
  /* A value varying within its pixel is read at the position itself */
  if (ops->point)
    return ops->point(ops->ctx, gcol, grow, value);
  return ops->pixel(ops->ctx, col, row, value);
}

/**
 * @brief Sequences of a step temporal float answered along a trajectory
 */
typedef struct
{
  TSequence **seqs;   /**< Sequences of the answer */
  int count;          /**< Number of sequences */
  int size;           /**< Capacity of the array */
} RasterAnswer;

/**
 * @brief Run of a step temporal float being built along a trajectory: its
 * instants and whether it holds its first one
 */
typedef struct
{
  TInstant **insts;   /**< Instants of the run */
  int count;          /**< Number of instants */
  int size;           /**< Capacity of the array */
  bool lower_inc;     /**< Whether the run holds its first instant */
} RasterRun;

/**
 * @brief Append a value to a run at a timestamp
 * @details A crossing time is interpolated from the parameter at which the
 * trip reaches a pixel, while a timestamp holds whole microseconds, so two
 * crossings closer together than one microsecond round to the same instant.
 * The value of a crossing landing on or before the instant already stated is
 * held for no time, so it is not part of the run, as a crossing within
 * #MEOS_EPSILON of the end of a segment is no crossing for
 * #tgeogpointsegm_distance_turnpt().
 */
static void
raster_run_push(RasterRun *run, double value, TimestampTz t)
{
  if (run->count > 0 && t <= run->insts[run->count - 1]->t)
  {
    /* The value already stated at this instant is left for this one before it
     * holds any time, so this one is the value the trip reads there */
    TimestampTz tlast = run->insts[run->count - 1]->t;
    pfree(run->insts[--run->count]);
    t = tlast;
  }
  if (run->count >= run->size)
  {
    run->size = (run->size == 0) ? 8 : run->size * 2;
    run->insts = (run->insts == NULL) ?
      palloc(sizeof(TInstant *) * (size_t) run->size) :
      repalloc(run->insts, sizeof(TInstant *) * (size_t) run->size);
  }
  run->insts[run->count++] = tinstant_make(Float8GetDatum(value), T_TFLOAT,
    t);
  return;
}

/**
 * @brief Return the parameter at which a segment reaches the grid line @p k
 * of an axis, or DBL_MAX when no parameter answers it
 * @param[in] ops Grid the segment crosses
 * @param[in] p1,p2 Endpoints of the segment
 * @param[in] g1,g2 Grid coordinates of the endpoints, which answer the
 * parameter where the grid coordinates are affine in the position
 * @param[in] axis 0 for a column line, 1 for a row line
 * @param[in] k Grid line
 */
static double
raster_cross_param(const RasterGridOps *ops, const POINT2D *p1,
  const POINT2D *p2, const double *g1, const double *g2, int axis, double k)
{
  double s = ops->cross ?
    ops->cross(ops->ctx, p1->x, p1->y, p2->x, p2->y, axis, k) :
    (k - g1[axis]) / (g2[axis] - g1[axis]);
  return isfinite(s) ? s : DBL_MAX;
}

/**
 * @brief Return the key of a pixel named by its column and its row, each
 * from -1 to the number of pixels along its axis, as the visits of a walk
 * carry it
 */
static Datum
raster_pixel_key(int col, int row)
{
  return Int64GetDatum(((int64) (col + 1) << 32) | (int64) (uint32) (row + 1));
}

/**
 * @brief Append to the visits of a trajectory the pixel it enters at a
 * timestamp, the previous visit ending there
 * @details The visits of a raster walk as #tquadbin_visit_append states those
 * of the QUADBIN walk: a pixel holds exactly the microseconds whose exact
 * position lies in it, and the timestamp is the microsecond of the crossing,
 * truncated, which the pixel entered holds when @p entered and the pixel
 * left holds otherwise. A crossing landing before the previous one, which the
 * truncation of two crossings within one microsecond gives, lands on it
 */
static void
raster_visit_append(DggsVisit **visits, int *count, int *size, Datum key,
  TimestampTz t, bool entered)
{
  DggsVisit *prev = &(*visits)[*count - 1];
  if (t < prev->lower)
    t = prev->lower;
  prev->upper = t;
  prev->upper_inc = ! entered;
  if (*count == *size)
  {
    *size *= 2;
    *visits = repalloc(*visits, sizeof(DggsVisit) * (size_t) *size);
  }
  DggsVisit *v = &(*visits)[(*count)++];
  v->cell = key;
  v->lower = t;
  v->lower_inc = entered;
  return;
}

/**
 * @brief Return the microsecond at which a segment crosses the grid line
 * @p k of an axis, as the offset from the start of the segment, truncated
 * @details On a grid whose engine dates a crossing exactly (the affine map
 * of a geotransform, #raster_affine_offset, or the QUADBIN tiles of a Raquet
 * tile) the crossing is dated by the truncation of its exact parameter.
 * Another grid reads the parameter in doubles (#raster_cross_param)
 * @param[in] ops Grid the segment crosses
 * @param[in] p1,p2 Endpoints of the segment
 * @param[in] g1,g2 Grid coordinates of the endpoints
 * @param[in] D Duration of the segment in microseconds
 * @param[in] axis 0 for a column line, 1 for a row line
 * @param[in] k Grid line
 * @param[out] whole True when the segment lies on the line exactly at the
 * microsecond returned
 */
static int64
raster_cross_offset(const RasterGridOps *ops, const POINT2D *p1,
  const POINT2D *p2, const double *g1, const double *g2, int64 D, int axis,
  int k, bool *whole)
{
  if (ops->offset)
    return ops->offset(ops->ctx, p1->x, p1->y, p2->x, p2->y, D, axis, k,
      whole);
  double s = raster_cross_param(ops, p1, p2, g1, g2, axis, (double) k);
  *whole = false;
  if (s == DBL_MAX)
    return D;
  double ds = (double) D * fmin(fmax(s, 0.0), 1.0);
  int64 off = (int64) ds;
  if (off > D)
    off = D;
  *whole = ((double) off == ds);
  return off;
}

/**
 * @brief Return the microsecond nearest to the instant at which a segment
 * crosses the grid line @p k of an axis, as the offset from the start of the
 * segment, a tie going to the even timestamp
 * @details On a grid whose engine dates a crossing exactly, the crossing over
 * the 2D half microseconds of the segment, truncated (#raster_cross_offset),
 * is m2, so the crossing lies at m2 / 2 where the segment lies on the line
 * then and between m2 / 2 and (m2 + 1) / 2 otherwise: the nearest microsecond
 * is (m2 + 1) / 2 rounded down, and a crossing at m2 / 2 with m2 odd lies
 * halfway between two microseconds, a tie going to the even timestamp, the
 * parity that of @p lower plus the offset, as #floatsegm_crossing_search
 * dates the crossing of two float segments. Another grid, or a segment of
 * 2^52 microseconds or more, reads it from the parameter @p s in doubles, as
 * #raster_cross_offset does
 * @param[in] ops Grid the segment crosses
 * @param[in] p1,p2 Endpoints of the segment
 * @param[in] lower Timestamp of the start of the segment
 * @param[in] D Duration of the segment in microseconds
 * @param[in] axis 0 for a column line, 1 for a row line
 * @param[in] k Grid line, a multiple of 1/2
 * @param[in] s Parameter of the crossing in doubles
 */
static int64
raster_cross_nearest(const RasterGridOps *ops, const POINT2D *p1,
  const POINT2D *p2, TimestampTz lower, int64 D, int axis, double k, double s)
{
  if (ops->offset && D < (INT64CONST(1) << 52))
  {
    bool whole;
    int64 m2 = ops->offset(ops->ctx, p1->x, p1->y, p2->x, p2->y, 2 * D, axis,
      k, &whole);
    int64 m = (m2 + 1) / 2;
    if (whole && (m2 % 2) != 0 && ((lower + m) & 1) != 0)
      m--;
    return m;
  }
  return llrint((double) D * fmin(fmax(s, 0.0), 1.0));
}

/**
 * @brief Append a temporal float to the sequences of an answer, which takes
 * its sequences over
 */
static void
raster_answer_add(RasterAnswer *answer, Temporal *temp)
{
  if (! temp)
    return;
  int count = (temp->subtype == TSEQUENCE) ? 1 :
    ((TSequenceSet *) temp)->count;
  while (answer->count + count > answer->size)
  {
    answer->size *= 2;
    answer->seqs = repalloc(answer->seqs,
      sizeof(TSequence *) * (size_t) answer->size);
  }
  if (temp->subtype == TSEQUENCE)
  {
    answer->seqs[answer->count++] = (TSequence *) temp;
    return;
  }
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  for (int i = 0; i < ss->count; i++)
    answer->seqs[answer->count++] = tsequence_copy(TSEQUENCESET_SEQ_N(ss, i));
  pfree(temp);
  return;
}

/**
 * @brief Append to an answer the values the pixels of the visits of a walk
 * hold, as the sequences of a step temporal float
 * @details Each pixel holds its value over its visit, and the visits of one
 * stretch over pixels carrying a value become the step function
 * #dggs_visits_to_temporal makes of the visits of a cell walk, the value of
 * each pixel standing for its cell: two pixels of one value meeting are one
 * piece, and a crossing whose instant the pixel left holds starts a sequence
 * after it. A visit outside the grid or over a pixel carrying no value ends
 * the stretch where it holds a microsecond, and a visit holding none, which
 * the walk gives a pixel crossed between two microseconds, is no part of it
 * @param[in] ops Grid the values are read from
 * @param[in] visits,count Visits of the walk, their cells the keys of
 * #raster_pixel_key
 * @param[in,out] answer Sequences of the answer, appended to
 */
static void
raster_visits_answer(const RasterGridOps *ops, const DggsVisit *visits,
  int count, RasterAnswer *answer)
{
  DggsVisit *run = palloc(sizeof(DggsVisit) * (size_t) count);
  int n = 0;
  for (int i = 0; i < count; i++)
  {
    DggsVisit v = visits[i];
    int64 key = DatumGetInt64(v.cell);
    int col = (int) (key >> 32) - 1, row = (int) (key & 0xFFFFFFFF) - 1;
    double value;
    if (col >= 0 && col < ops->width && row >= 0 && row < ops->height &&
        ops->pixel(ops->ctx, col, row, &value))
    {
      v.cell = Float8GetDatum(value);
      run[n++] = v;
      continue;
    }
    bool holds = ! ((v.lower == v.upper && ! (v.lower_inc && v.upper_inc)) ||
      (v.upper - v.lower == 1 && ! v.lower_inc && ! v.upper_inc));
    if (holds && n > 0)
    {
      raster_answer_add(answer, dggs_visits_to_temporal(run, n, T_TFLOAT));
      n = 0;
    }
  }
  if (n > 0)
    raster_answer_add(answer, dggs_visits_to_temporal(run, n, T_TFLOAT));
  pfree(run);
  return;
}

/**
 * @brief Return the values a raster holds along a linearly interpolated
 * sequence, as the sequences of a step temporal float
 * @details A trajectory moving between two instants passes over the pixels
 * between them, and their values belong to the answer as much as the values
 * under the instants themselves. Each segment is therefore TRAVERSED pixel by
 * pixel, as the QUADBIN walk traverses its tiles: a pixel holds exactly the
 * microseconds whose exact position lies in it, and the lines of its lower
 * column and row. The grid coordinates of a position are monotonic along a
 * straight segment, so the segment crosses the next column line and the next
 * row line once each, and each crossing is dated by the truncation of its
 * exact parameter (#raster_cross_offset); a crossing at a microsecond comes
 * before one after it, and stepping to the earlier of the two moves to an
 * adjacent pixel every time, to the diagonal one where the segment passes
 * through a pixel corner, so the walk cannot pass over a pixel. The pixel
 * entered holds the instant of a crossing exactly at a microsecond when it
 * holds the line crossed, and the pixel left holds it otherwise, so a
 * crossing between two microseconds leaves the earlier one to the pixel
 * left, the pixel entered starting after it; a corner held by a third pixel
 * holds that instant alone. A walk sampling the segment at a spacing has
 * neither property at any spacing: a segment clips a pixel corner over an
 * arbitrarily short chord.
 *
 * A position outside the raster or over a nodata pixel carries no value and
 * ends the sequence, so a trip leaving and re-entering the raster answers one
 * sequence per visit.
 * @param[in] seq Trajectory sequence with linear interpolation
 * @param[in] ops Grid the values are read from
 * @param[in,out] answer Sequences of the answer, appended to
 */
static void
tpointseq_raster_value_traverse(const TSequence *seq, const RasterGridOps *ops,
  RasterAnswer *answer)
{
  /* The positions are read, never kept, and the instants that hold them
   * outlive this call, so they are borrowed rather than copied */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(
    (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst1)));
  double g1[2];
  ops->grid(ops->ctx, p1->x, p1->y, &g1[0], &g1[1]);
  int col = raster_axis_index(ops, p1->x, p1->y, g1[0], 0, ops->width);
  int row = raster_axis_index(ops, p1->x, p1->y, g1[1], 1, ops->height);
  /* The trip starts in the pixel holding its first position */
  int vsize = seq->count + 1, nvisits = 1;
  DggsVisit *visits = palloc(sizeof(DggsVisit) * (size_t) vsize);
  visits[0].cell = raster_pixel_key(col, row);
  visits[0].lower = inst1->t;
  visits[0].lower_inc = seq->period.lower_inc;

  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst2)));
    double g2[2];
    ops->grid(ops->ctx, p2->x, p2->y, &g2[0], &g2[1]);
    int ecol = raster_axis_index(ops, p2->x, p2->y, g2[0], 0, ops->width);
    int erow = raster_axis_index(ops, p2->x, p2->y, g2[1], 1, ops->height);
    int64 D = inst2->t - inst1->t;
    int stepc = (ecol > col) ? 1 : ((ecol < col) ? -1 : 0);
    int stepr = (erow > row) ? 1 : ((erow < row) ? -1 : 0);
    /* One step for each column line and each row line the segment crosses
     * bounds the walk */
    int guard = abs(ecol - col) + abs(erow - row);
    while ((col != ecol || row != erow) && guard-- > 0)
    {
      /* The next column line and row line, each crossing keyed so that one
       * exactly at a microsecond precedes one after it */
      int kc = (stepc > 0) ? col + 1 : col, kr = (stepr > 0) ? row + 1 : row;
      bool wc = false, wr = false;
      int64 oc = 0, orr = 0, keyc = INT64_MAX, keyr = INT64_MAX;
      if (col != ecol)
      {
        oc = raster_cross_offset(ops, p1, p2, g1, g2, D, 0, kc, &wc);
        keyc = 2 * oc + (wc ? 0 : 1);
      }
      if (row != erow)
      {
        orr = raster_cross_offset(ops, p1, p2, g1, g2, D, 1, kr, &wr);
        keyr = 2 * orr + (wr ? 0 : 1);
      }
      bool bc = (keyc <= keyr), br = (keyr <= keyc);
      int ncol = bc ? col + stepc : col, nrow = br ? row + stepr : row;
      bool whole = bc ? wc : wr;
      TimestampTz t = inst1->t + (bc ? oc : orr);
      Datum next = raster_pixel_key(ncol, nrow);
      if (bc && br && whole)
      {
        /* Through a corner at a microsecond: the pixel holding the corner,
         * the one of its lower column and row, holds that instant */
        int ocol = (stepc > 0) ? ncol : col, orow = (stepr > 0) ? nrow : row;
        if (ocol == ncol && orow == nrow)
          raster_visit_append(&visits, &nvisits, &vsize, next, t, true);
        else if (ocol == col && orow == row)
          raster_visit_append(&visits, &nvisits, &vsize, next, t, false);
        else
        {
          raster_visit_append(&visits, &nvisits, &vsize,
            raster_pixel_key(ocol, orow), t, true);
          raster_visit_append(&visits, &nvisits, &vsize, next, t, false);
        }
      }
      else
        raster_visit_append(&visits, &nvisits, &vsize, next, t,
          whole && ((bc && stepc > 0) || (br && stepr > 0)));
      col = ncol;
      row = nrow;
    }
    /* The end of the segment lies in the pixel holding its position */
    if (col != ecol || row != erow)
    {
      raster_visit_append(&visits, &nvisits, &vsize,
        raster_pixel_key(ecol, erow), inst2->t, true);
      col = ecol;
      row = erow;
    }
    /* The next segment starts where this one ends */
    inst1 = inst2;
    p1 = p2;
    g1[0] = g2[0];
    g1[1] = g2[1];
  }

  /* The last pixel holds to the end of the trip. A crossing dated at the
   * first or the last instant of the trip holds that instant only where the
   * trip does, as the QUADBIN walk states it */
  TimestampTz tstart = TSEQUENCE_INST_N(seq, 0)->t;
  TimestampTz tend = TSEQUENCE_INST_N(seq, seq->count - 1)->t;
  visits[nvisits - 1].upper = tend;
  visits[nvisits - 1].upper_inc = seq->period.upper_inc;
  for (int i = 0; i < nvisits; i++)
  {
    if (visits[i].lower == tstart)
      visits[i].lower_inc &= seq->period.lower_inc;
    if (visits[i].upper == tend)
      visits[i].upper_inc &= seq->period.upper_inc;
  }
  raster_visits_answer(ops, visits, nvisits, answer);
  pfree(visits);
  return;
}

/**
 * @brief Return the index of the half-pixel square holding a grid coordinate,
 * a coordinate outside the grid naming the square just outside it, see
 * #raster_cell_index()
 * @param[in] g Grid coordinate
 * @param[in] n Number of cells along the axis
 */
static int
raster_half_index(double g, int n)
{
  /* The negated test also sends a NaN outside */
  if (! (g >= 0.0))
    return -1;
  if (g >= (double) n)
    return 2 * n;
  return (int) floor(2.0 * g);
}

/**
 * @brief Return the first of the two pixels along an axis that the read of a
 * position interpolates over a half-pixel square
 * @details The first half of pixel c reads pixels c - 1 and c, its second
 * half pixels c and c + 1
 * @param[in] h Index of the half-pixel square along the axis
 */
static int
raster_half_stencil(int h)
{
  return (h + 1) / 2 - 1;
}

/**
 * @brief Return whether the pixel holding a half-pixel square carries a
 * value
 */
static bool
raster_half_value(const RasterGridOps *ops, int hc, int hr)
{
  if (hc < 0 || hc >= 2 * ops->width || hr < 0 || hr >= 2 * ops->height)
    return false;
  double value;
  return ops->pixel(ops->ctx, hc / 2, hr / 2, &value);
}

/**
 * @brief Return whether the read of a position is continuous where a segment
 * passes from one half-pixel square into another, both carrying a value
 * @details Each square reads one polynomial from the four pixels around it,
 * a pixel outside the grid or holding nodata taking the value of the pixel
 * holding the square, see ::raster_point_fn. A square passes into the other
 * half of the same pixel across the line through the centre of the pixel,
 * where the pixels the two polynomials do not share weigh nothing, so the
 * read is continuous there. Across the edge between two pixels the two
 * squares read the same four pixels, but a pixel holding no data takes the
 * value of a different pixel on each side, so the read is continuous there
 * when every pixel read holds data, and may jump otherwise.
 */
static bool
raster_half_joins(const RasterGridOps *ops, int hc0, int hr0, int hc1,
  int hr1)
{
  /* The line between squares h and h + 1 passes through the centre of a
   * pixel when h is even */
  bool edge = (hc0 != hc1 && Min(hc0, hc1) % 2 != 0) ||
    (hr0 != hr1 && Min(hr0, hr1) % 2 != 0);
  if (! edge)
    return true;
  int c0 = raster_half_stencil(Min(hc0, hc1));
  int c1 = raster_half_stencil(Max(hc0, hc1)) + 1;
  int r0 = raster_half_stencil(Min(hr0, hr1));
  int r1 = raster_half_stencil(Max(hr0, hr1)) + 1;
  for (int col = c0; col <= c1; col++)
    for (int row = r0; row <= r1; row++)
      if (col < 0 || col >= ops->width || row < 0 || row >= ops->height ||
          ! ops->data(ops->ctx, col, row))
        return false;
  return true;
}

/**
 * @brief Read the value at the grid coordinates @p g with the polynomial of
 * a half-pixel square, @p g clamped into the square
 * @details The square holds its lower lines and not its upper ones, so a
 * position on an upper line is read a hair inside, at the largest double
 * below it, as a read at a parameter is (#raster_half_read)
 */
static bool
raster_half_read_at(const RasterGridOps *ops, double *g, int hc, int hr,
  double *value)
{
  const int h[2] = {hc, hr};
  for (int axis = 0; axis < 2; axis++)
  {
    double lo = 0.5 * h[axis];
    double hi = nextafter(0.5 * (h[axis] + 1), lo);
    if (g[axis] < lo)
      g[axis] = lo;
    else if (g[axis] > hi)
      g[axis] = hi;
  }
  return ops->point(ops->ctx, g[0], g[1], value);
}

/**
 * @brief Return the value a half-pixel square reads at the parameter @p s of
 * a segment
 * @details The position is held inside the square, so that at an end of the
 * piece of the segment crossing the square the value is the one the
 * polynomial of the square takes there, whichever square a position on its
 * edge belongs to
 * @param[in] ops Grid the value is read from
 * @param[in] g1,g2 Grid coordinates of the endpoints of the segment
 * @param[in] s Parameter along the segment
 * @param[in] hc,hr Half-pixel square
 * @param[out] value Value read
 */
static bool
raster_half_read(const RasterGridOps *ops, const double *g1, const double *g2,
  double s, int hc, int hr, double *value)
{
  double g[2] = {g1[0] + s * (g2[0] - g1[0]), g1[1] + s * (g2[1] - g1[1])};
  return raster_half_read_at(ops, g, hc, hr, value);
}

/**
 * @brief Append to a run the extremum of the value read along the piece of a
 * segment crossing a half-pixel square
 * @details Over the square the value is a + b col + c row + d col row, and
 * along the piece the grid coordinates are affine in time, so the value is a
 * polynomial of degree two in time: linear where the segment runs along a
 * grid axis or where d is zero, and otherwise reaching one extremum. The
 * values at the ends and at the middle of the piece state the polynomial,
 * and the extremum, when it falls inside the piece, is read there and dated
 * at the microsecond nearest to it, as #tfloat_arithop_turnpt() places the
 * extremum of the product of two temporal floats.
 * @param[in] tb Instant dating the end of the piece
 */
static void
raster_half_turnpt(const RasterGridOps *ops, const double *g1,
  const double *g2, double sa, double sb, int hc, int hr, TimestampTz t1,
  double dt, TimestampTz tb, RasterRun *run)
{
  double fa, fm, fb;
  if (! (sb > sa) ||
      ! raster_half_read(ops, g1, g2, sa, hc, hr, &fa) ||
      ! raster_half_read(ops, g1, g2, 0.5 * (sa + sb), hc, hr, &fm) ||
      ! raster_half_read(ops, g1, g2, sb, hc, hr, &fb))
    return;
  /* The value along the piece is fa + b u + d u^2 for u in [0, 1] */
  double d = 2.0 * (fa + fb - 2.0 * fm);
  if (d == 0.0)
    return;
  double b = fb - fa - d;
  double u = -b / (2.0 * d);
  if (! (u > 0.0 && u < 1.0))
    /* The extremum lies outside the piece */
    return;
  double s = sa + u * (sb - sa);
  TimestampTz t = t1 + llrint(dt * s);
  /* An extremum dated at an instant already stated or at the end of the
   * piece holds no time of its own */
  if ((run->count > 0 && t <= run->insts[run->count - 1]->t) || t >= tb)
    return;
  double value;
  if (raster_half_read(ops, g1, g2, s, hc, hr, &value))
    raster_run_push(run, value, t);
  return;
}

/**
 * @brief Close a run of a linear temporal float and append it to the answer
 * @details The last instant of the run is its end, which the run holds when
 * @p upper_inc is true. A run of one instant holds it only when both of its
 * bounds are inclusive, and holds no time otherwise.
 */
static void
raster_run_close_linear(RasterRun *run, bool upper_inc, RasterAnswer *answer)
{
  if (run->count == 0)
    return;
  if (run->count == 1 && ! (run->lower_inc && upper_inc))
  {
    pfree(run->insts[0]);
    pfree(run->insts);
    run->insts = NULL;
    run->count = run->size = 0;
    return;
  }
  if (answer->count >= answer->size)
  {
    answer->size *= 2;
    answer->seqs = repalloc(answer->seqs,
      sizeof(TSequence *) * (size_t) answer->size);
  }
  answer->seqs[answer->count++] = tsequence_make_free(run->insts, run->count,
    run->lower_inc, upper_inc, LINEAR, NORMALIZE);
  run->insts = NULL;
  run->count = run->size = 0;
  return;
}

/**
 * @brief Return the values a raster read at a position holds along a
 * linearly interpolated sequence, as the sequences of a linear temporal float
 * @details The read of a position interpolates the four pixels around it,
 * and over each half-pixel square of the grid it is one polynomial, see
 * ::raster_point_fn. Each segment is therefore traversed square by square,
 * as #tpointseq_raster_value_traverse() traverses it pixel by pixel, with the
 * lines through the pixel centres beside the pixel edges. Along the piece of
 * a segment crossing a square the value is a polynomial of degree two in
 * time, whose extremum #raster_half_turnpt() adds. The answer thus holds the
 * value at the instants of the trip, at every line it crosses and at every
 * extremum between them, and is linear between those. It is the value itself
 * wherever the trip runs along a grid axis, where the polynomial is linear.
 * The value at a line is the one the trip reads on the line, at its exact
 * crossing, and the crossing is dated at its nearest microsecond
 * (#raster_cross_nearest), so the values of a row or a column in arithmetic
 * progression lie on one segment and the answer holds none of their
 * crossings.
 *
 * The value is continuous across the lines through the pixel centres, and
 * across a pixel edge unless a pixel read there holds no data. There it may
 * jump, so a new sequence starts at the instant the trip crosses the edge,
 * holding from that instant on the value of the square the trip enters, as a
 * step read holds the value of the pixel the trip enters.
 * @param[in] seq Trajectory sequence with linear interpolation
 * @param[in] ops Grid the values are read from
 * @param[in,out] answer Sequences of the answer, appended to
 */
static void
tpointseq_raster_point_traverse(const TSequence *seq, const RasterGridOps *ops,
  RasterAnswer *answer)
{
  RasterRun run = {NULL, 0, 0, seq->period.lower_inc};
  /* The positions are borrowed, as in #tpointseq_raster_value_traverse() */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(
    (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst1)));
  double g1[2];
  ops->grid(ops->ctx, p1->x, p1->y, &g1[0], &g1[1]);
  int hc = raster_half_index(g1[0], ops->width);
  int hr = raster_half_index(g1[1], ops->height);
  bool in = raster_half_value(ops, hc, hr);
  double value;
  /* The trip starts with the value at its first position */
  if (in && ops->point(ops->ctx, g1[0], g1[1], &value))
    raster_run_push(&run, value, inst1->t);

  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst2)));
    double g2[2];
    ops->grid(ops->ctx, p2->x, p2->y, &g2[0], &g2[1]);
    int hce = raster_half_index(g2[0], ops->width);
    int hre = raster_half_index(g2[1], ops->height);
    int64 D = inst2->t - inst1->t;
    double dt = (double) D;
    double s_prev = 0.0;
    /* A segment lying on one side outside the grid along its whole length
     * meets no square, see #tpointseq_raster_value_traverse() */
    bool outside = (hc == hce && (hc < 0 || hc >= 2 * ops->width)) ||
      (hr == hre && (hr < 0 || hr >= 2 * ops->height));
    if (! outside)
    {
      int stepc = (hce > hc) ? 1 : ((hce < hc) ? -1 : 0);
      int stepr = (hre > hr) ? 1 : ((hre < hr) ? -1 : 0);
      /* One step for each line the segment crosses bounds the walk */
      int guard = abs(hce - hc) + abs(hre - hr);
      while ((hc != hce || hr != hre) && guard-- > 0)
      {
        /* The next column line and row line, of pixel edges or of pixel
         * centres */
        double kc = 0.5 * ((stepc > 0) ? hc + 1 : hc);
        double kr = 0.5 * ((stepr > 0) ? hr + 1 : hr);
        double sc = (hc == hce) ? DBL_MAX :
          raster_cross_param(ops, p1, p2, g1, g2, 0, kc);
        double sr = (hr == hre) ? DBL_MAX :
          raster_cross_param(ops, p1, p2, g1, g2, 1, kr);
        if (sc == DBL_MAX && sr == DBL_MAX)
          break;
        /* A segment passing through a corner of the squares crosses both
         * lines at once */
        int hc1 = (sc <= sr) ? hc + stepc : hc;
        int hr1 = (sr <= sc) ? hr + stepr : hr;
        double s = Min(sc, sr);
        /* Rounding may place a crossing a hair before the previous one or
         * past the end of the segment */
        if (s < s_prev)
          s = s_prev;
        if (s > 1.0)
          s = 1.0;
        /* The crossing is dated at its nearest microsecond and read on the
         * line it crosses, the value of the exact crossing */
        TimestampTz t = inst1->t + ((sc <= sr) ?
          raster_cross_nearest(ops, p1, p2, inst1->t, D, 0, kc, s) :
          raster_cross_nearest(ops, p1, p2, inst1->t, D, 1, kr, s));
        double gx[2] = {g1[0] + s * (g2[0] - g1[0]),
          g1[1] + s * (g2[1] - g1[1])};
        if (sc <= sr)
          gx[0] = kc;
        if (sr <= sc)
          gx[1] = kr;
        if (in)
          raster_half_turnpt(ops, g1, g2, s_prev, s, hc, hr, inst1->t, dt, t,
            &run);
        bool in1 = raster_half_value(ops, hc1, hr1);
        if (in && in1 && raster_half_joins(ops, hc, hr, hc1, hr1))
        {
          if (raster_half_read_at(ops, gx, hc1, hr1, &value))
            raster_run_push(&run, value, t);
        }
        else
        {
          /* The value the trip leaves holds up to the crossing, and the one
           * it enters from the crossing on */
          if (in)
          {
            double gl[2] = {gx[0], gx[1]};
            if (raster_half_read_at(ops, gl, hc, hr, &value))
              raster_run_push(&run, value, t);
            raster_run_close_linear(&run, false, answer);
          }
          run.lower_inc = true;
          if (in1 && raster_half_read_at(ops, gx, hc1, hr1, &value))
            raster_run_push(&run, value, t);
        }
        hc = hc1;
        hr = hr1;
        in = in1;
        s_prev = s;
      }
    }
    if (hc != hce || hr != hre)
    {
      /* The walk stops short of the square holding the end of the segment
       * only where rounding defeats it, and the run ends there */
      raster_run_close_linear(&run, false, answer);
      run.lower_inc = true;
      hc = hce;
      hr = hre;
      in = raster_half_value(ops, hc, hr);
      s_prev = 1.0;
    }
    /* The segment ends with the value at its last position */
    if (in)
    {
      raster_half_turnpt(ops, g1, g2, s_prev, 1.0, hc, hr, inst1->t, dt,
        inst2->t, &run);
      if (ops->point(ops->ctx, g2[0], g2[1], &value))
        raster_run_push(&run, value, inst2->t);
    }
    /* The next segment starts where this one ends */
    inst1 = inst2;
    p1 = p2;
    g1[0] = g2[0];
    g1[1] = g2[1];
  }
  raster_run_close_linear(&run, seq->period.upper_inc, answer);
  return;
}

/**
 * @brief Add to the answer the values a step sequence reads from a raster
 * grid
 * @details A step sequence holds its position from one instant up to the
 * next, so the value it reads at an instant holds up to the next instant:
 * it is read at its instants, as #tpointseq_raster_value_traverse reads a
 * linear one along its segments, and as there a run ends where the grid
 * answers no value, here at an instant on a nodata pixel or outside the
 * grid, the last value of the run holding up to that instant, excluded.
 * The first run takes the lower bound of the sequence and the last run its
 * upper bound
 * @param[in] seq Step sequence of a temporal geometry point
 * @param[in] ops Grid the values are read from
 * @param[in,out] answer Sequences of the answer, which the runs are added to
 */
static void
tpointseq_step_raster_value(const TSequence *seq, const RasterGridOps *ops,
  RasterAnswer *answer)
{
  int n = seq->count;
  TInstant **insts = palloc(sizeof(TInstant *) * (size_t) (n + 1));
  int count = 0, first = 0;
  double last = 0.0;
  for (int i = 0; i <= n; i++)
  {
    const TInstant *inst = i < n ? TSEQUENCE_INST_N(seq, i) : NULL;
    double pixval;
    bool has = false;
    if (inst)
    {
      /* Borrowed: the sampling reads the point and keeps nothing */
      const POINT2D *p = GSERIALIZED_POINT2D_P(
        (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst)));
      has = raster_sample_at(ops, p->x, p->y, &pixval);
    }
    if (has)
    {
      if (count == 0)
        first = i;
      insts[count++] = tinstant_make(Float8GetDatum(pixval), T_TFLOAT,
        inst->t);
      last = pixval;
      continue;
    }
    if (count == 0)
      continue;
    /* The run ends here: at an instant with no value, up to which its last
     * value holds, or at the end of the sequence */
    bool lower_inc = (first == 0) ? seq->period.lower_inc : true;
    bool upper_inc;
    if (inst)
    {
      insts[count++] = tinstant_make(Float8GetDatum(last), T_TFLOAT, inst->t);
      upper_inc = false;
    }
    else
      upper_inc = seq->period.upper_inc;
    if (count == 1 && ! (lower_inc && upper_inc))
    {
      /* One instant excluded at either end holds no time */
      pfree(insts[0]);
      pfree(insts);
    }
    else
      raster_answer_add(answer, (Temporal *) tsequence_make_free(insts, count,
        lower_inc, upper_inc, STEP, NORMALIZE));
    insts = palloc(sizeof(TInstant *) * (size_t) (n + 1));
    count = 0;
  }
  pfree(insts);
  return;
}

/**
 * @brief Return the values of a raster read along a trajectory
 * @details The pixel access is delegated to the grid descriptor so that the
 * one algorithm serves any raster engine: a PostGIS raster is read through
 * the vendored raster core, a raster file through GDAL, a Raquet tile from
 * its own pixel array. A linear sequence is read along its segments by
 * #tpointseq_raster_value_traverse, a step sequence at its instants by
 * #tpointseq_step_raster_value, each keeping the bounds and the gaps of the
 * sequence, and a discrete trajectory at its instants.
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @return A temporal float, or @p NULL when no instant of @p traj falls
 * inside the raster or survives nodata filtering
 */
Temporal *
raster_value_sampler(const Temporal *traj, const RasterGridOps *ops)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL((void *) ops, NULL);
  VALIDATE_NOT_NULL((void *) ops->grid, NULL);
  VALIDATE_NOT_NULL((void *) ops->pixel, NULL);

  /* A trajectory that moves between its instants passes over the pixels
   * between them, so the traversal reads those too; one that holds its
   * position from one instant to the next is read at its instants, each
   * value holding up to the next; both are read sequence by sequence, so
   * their bounds and their gaps are kept. One that states nothing between
   * its instants is read at the instants */
  interpType interp = MEOS_FLAGS_GET_INTERP(traj->flags);
  if (interp == LINEAR || interp == STEP)
  {
    const TSequence **seqs;
    int nseqs_in;
    if (traj->subtype == TSEQUENCE)
    {
      seqs = palloc(sizeof(TSequence *));
      seqs[0] = (const TSequence *) traj;
      nseqs_in = 1;
    }
    else
      seqs = temporal_sequences_p(traj, &nseqs_in);

    /* Each visit to the raster answers one sequence, and a visit ends at a
     * position the raster does not answer for, of which a single segment
     * may cross many */
    RasterAnswer answer;
    answer.size = nseqs_in;
    answer.count = 0;
    answer.seqs = palloc(sizeof(TSequence *) * (size_t) answer.size);
    for (int i = 0; i < nseqs_in; i++)
    {
      if (interp == STEP)
        tpointseq_step_raster_value(seqs[i], ops, &answer);
      else if (ops->point)
        tpointseq_raster_point_traverse(seqs[i], ops, &answer);
      else
        tpointseq_raster_value_traverse(seqs[i], ops, &answer);
    }
    pfree(seqs);
    if (answer.count == 0)
    {
      pfree(answer.seqs);
      return NULL;
    }
    if (answer.count == 1)
    {
      Temporal *result = (Temporal *) answer.seqs[0];
      pfree(answer.seqs);
      return result;
    }
    return (Temporal *) tsequenceset_make_free(answer.seqs, answer.count,
      NORMALIZE);
  }

  /* Iterate over trajectory instants */
  int count;
  const TInstant **insts = temporal_insts_p(traj, &count);
  TInstant **result_insts = palloc(sizeof(TInstant *) * count);
  int ninsts = 0;

  for (int i = 0; i < count; i++)
  {
    /* Borrowed, as above: the sampling reads the point and keeps nothing */
    const POINT2D *p = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(insts[i])));

    double pixval;
    if (! raster_sample_at(ops, p->x, p->y, &pixval))
      continue;   /* nodata pixel or position outside the pixel grid */

    result_insts[ninsts++] =
      tinstant_make(Float8GetDatum(pixval), T_TFLOAT, insts[i]->t);
  }

  pfree(insts);

  if (ninsts == 0)
  {
    pfree(result_insts);
    return NULL;
  }
  return (Temporal *) tsequence_make_free(result_insts, ninsts, true, true,
    DISCRETE, NORMALIZE);
}

/*****************************************************************************
 * raster_at_value_sampler / raster_minus_value_sampler /
 * eraster_value_sampler / araster_value_sampler
 *****************************************************************************/

/**
 * @brief Return a trajectory restricted to the times where the values it
 * reads from a raster fall inside or outside a float span
 * @details Equivalent to, with @p v the values read along @p traj:
 * @code
 * atTime(traj, getTime(atSpan(v, vspan)))
 * atTime(traj, getTime(minusSpan(v, vspan)))
 * @endcode
 * the restrictions of every raster form, one raster, one Raquet tile or an
 * array of tiles, read from their values this way
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] v Values read along @p traj, freed here, or @p NULL when the
 * trajectory reads none
 * @param[in] vspan Float value range
 * @param[in] atfunc True for the times inside @p vspan, false for those
 * outside it
 * @return A trajectory restricted to the qualifying times, or @p NULL when
 * none qualify
 */
Temporal *
raster_values_restrict(const Temporal *traj, Temporal *v, const Span *vspan,
  bool atfunc)
{
  if (! v)
    return NULL;
  Temporal *v1 = tnumber_restrict_span(v, vspan, atfunc ? REST_AT :
    REST_MINUS);
  pfree(v);
  if (! v1)
    return NULL;
  SpanSet *ss = temporal_time(v1);
  pfree(v1);
  Temporal *result = temporal_restrict_tstzspanset(traj, ss, REST_AT);
  pfree(ss);
  return result;
}

/**
 * @brief Return true if the values a trajectory reads from a raster ever, or
 * always, fall inside a float span
 * @details A trajectory reading no value answers false for both
 * @param[in] v Values read along a trajectory, freed here, or @p NULL when
 * the trajectory reads none
 * @param[in] vspan Float value range
 * @param[in] ever True for ever, false for always
 * @return 1 if the values ever, or always, fall inside @p vspan, 0 if not
 */
int
raster_values_ever(Temporal *v, const Span *vspan, bool ever)
{
  if (! v)
    return 0;
  Temporal *v1 = tnumber_restrict_span(v, vspan, ever ? REST_AT : REST_MINUS);
  pfree(v);
  bool result = ever ? (v1 != NULL) : (v1 == NULL);
  if (v1)
    pfree(v1);
  return result ? 1 : 0;
}

/**
 * @brief Return a trajectory restricted to the instants where the sampled
 * raster pixel value falls inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @param[in] vspan Float value range (inclusive bounds)
 * @return A trajectory restricted to the qualifying instants, or @p NULL
 * when none qualify
 */
Temporal *
raster_at_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL((void *) ops, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj, raster_value_sampler(traj, ops), vspan,
    true);
}

/**
 * @brief Return a trajectory restricted to the instants where the sampled
 * raster pixel value falls outside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @param[in] vspan Float value range to exclude
 * @return A trajectory restricted to the qualifying instants, or @p NULL
 * when none qualify
 */
Temporal *
raster_minus_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL((void *) ops, NULL);
  VALIDATE_NOT_NULL(vspan, NULL);
  return raster_values_restrict(traj, raster_value_sampler(traj, ops), vspan,
    false);
}

/**
 * @brief Return true if a trajectory ever samples a raster pixel value
 * inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @param[in] vspan Float value range
 * @return 1 if the trajectory ever samples a value inside @p vspan, 0 if
 * not, and -1 on error
 */
int
eraster_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, -1); VALIDATE_NOT_NULL((void *) ops, -1);
  VALIDATE_NOT_NULL(vspan, -1);
  return raster_values_ever(raster_value_sampler(traj, ops), vspan, true);
}

/**
 * @brief Return true if every in-raster-extent instant of a trajectory
 * samples a pixel value inside a float span
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @param[in] vspan Float value range
 * @return 1 if every sampled value falls inside @p vspan, 0 if not, and -1
 * on error
 */
int
araster_value_sampler(const Temporal *traj,
  const RasterGridOps *ops, const Span *vspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, -1); VALIDATE_NOT_NULL((void *) ops, -1);
  VALIDATE_NOT_NULL(vspan, -1);
  return raster_values_ever(raster_value_sampler(traj, ops), vspan, false);
}

/*****************************************************************************/
