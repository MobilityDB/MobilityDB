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
 * @brief MEOS kernel for sampling Raquet raster chips along tgeompoint
 * trajectories.
 *
 * Each Raquet tile is identified by a CARTO QUADBIN cell (uint64) whose
 * Morton-encoded x/y/z coordinates define the Web-Mercator bounding box and
 * pixel grid without any external metadata.  The pixel-to-coordinate mapping
 * uses the standard slippy-tile Mercator transform:
 *
 *   col = floor((lon − xmin) / (xmax − xmin) × width)
 *   row = floor((top_merc − merc(lat)) / (top_merc − bot_merc) × height)
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

/* Highest zoom level accepted when covering a trajectory with QUADBIN cells.
 * The zoom bounds the shift building the tile grid, so it is validated before
 * use; the value is the documented range of #trajectory_quadbins() */
#define QB_TRAJECTORY_MAX_ZOOM  15

/** Latitude the Web-Mercator grid reaches, beyond which a tile has no extent */
#define QB_MAX_LATITUDE  85.051129

static const uint64_t QB_B[6] = {
  UINT64_C(0x5555555555555555), UINT64_C(0x3333333333333333),
  UINT64_C(0x0F0F0F0F0F0F0F0F), UINT64_C(0x00FF00FF00FF00FF),
  UINT64_C(0x0000FFFF0000FFFF), UINT64_C(0x00000000FFFFFFFF)
};

/**
 * @brief Morton-decode a QUADBIN cell into Web-Mercator tile coordinates.
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
 * @brief Morton-encode tile (x, y, z) into a QUADBIN cell.
 * @details Canonical spread_bits algorithm matching CARTO quadbin-js
 * quadbinTileToCell: scale to 2^26 grid, spread left (shifts 16→8→4→2→1),
 * interleave x/y.
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
  return QB_HEADER | QB_MODE | ((uint64_t)tz << 52) | (xx | (yy << 1));
}

/**
 * @brief Compute the WGS-84 bounding box and Mercator top/bottom of a tile.
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
 * EPSG:3857 georeferencing.
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
  /* A QUADBIN tile is square */
  if (fabs(ext_x - ext_y) > 1e-6 * ext_x)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Raster is not a square Web-Mercator tile; cannot derive its QUADBIN cell");
    return false;
  }
  /* Zoom z: the tile extent is 2*QB_MERC_MAX / 2^z metres */
  double world = 2.0 * QB_MERC_MAX;
  double zf = log2(world / ext_x);
  long z = lround(zf);
  if (fabs(zf - (double) z) > 1e-6 || z < 0 || z > 26)
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
  if (fabs(txf - (double) tx) > 1e-6 || fabs(tyf - (double) ty) > 1e-6)
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
 * trajectory: the pixel array, the layout its QUADBIN cell fixes, and whether
 * a pixel held a value the sampling surface cannot carry
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
  if (state->has_nodata && pixval == state->nodata)
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
  ops->cross = &raquet_cross;
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
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory.
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
  size_t pixels_size, int32 width, int32 height, uint64 quadbin,
  MeosPixType pixtype, double nodata, bool has_nodata)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pixels, NULL);
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
 * @brief Return the value of the pixel a position falls in, or false when
 * the position lies outside the pre-filter box, outside the pixel grid, or
 * on a nodata pixel
 */
static bool
raster_sample_at(const RasterGridOps *ops, double x, double y,
  double *value)
{
  const STBox *box = &ops->box;
  if (x < box->xmin || x > box->xmax || y < box->ymin || y > box->ymax)
    return false;
  double gcol, grow;
  ops->grid(ops->ctx, x, y, &gcol, &grow);
  int col = raster_cell_index(gcol, ops->width);
  int row = raster_cell_index(grow, ops->height);
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
 * The second one is placed one microsecond after the first, as
 * #tpointseq_densify_to_th3index() places a cell: that is the smallest
 * separation the type can state, and it keeps both the pixel and the order
 * in which the trip reaches the pixels.
 */
static void
raster_run_push(RasterRun *run, double value, TimestampTz t)
{
  if (run->count > 0 && t <= run->insts[run->count - 1]->t)
    t = run->insts[run->count - 1]->t + 1;
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
 * @brief Close a run at a timestamp and append it to the answer
 * @details The last value of the run holds up to @p t. A run whose upper
 * bound is exclusive ends on the value it holds before @p t, so a value the
 * trip reaches at @p t itself, which it holds for no time before the run
 * ends, is not part of the run.
 */
static void
raster_run_close(RasterRun *run, TimestampTz t, bool upper_inc,
  RasterAnswer *answer)
{
  if (run->count == 0)
    return;
  bool lower_inc = run->lower_inc;
  if (! upper_inc)
    while (run->count > 1 && run->insts[run->count - 1]->t >= t)
      pfree(run->insts[--run->count]);
  const TInstant *last = run->insts[run->count - 1];
  if (last->t < t)
    raster_run_push(run, DatumGetFloat8(tinstant_value_p(last)), t);
  else if (run->count == 1)
    /* A run of one instant holds it */
    lower_inc = upper_inc = true;
  if (answer->count >= answer->size)
  {
    answer->size *= 2;
    answer->seqs = repalloc(answer->seqs,
      sizeof(TSequence *) * (size_t) answer->size);
  }
  answer->seqs[answer->count++] = tsequence_make_free(run->insts, run->count,
    lower_inc, upper_inc, STEP, NORMALIZE);
  run->insts = NULL;
  run->count = run->size = 0;
  return;
}

/**
 * @brief Enter in a run the pixel a trajectory reaches at a timestamp
 * @details A pixel holding the value the run already holds continues it,
 * and a pixel holding another value starts a new piece of the step function
 * at the instant the trip reaches it. A cell outside the grid or a nodata
 * pixel carries no value: the value the run carried holds until here and the
 * run closes on it, and the next run starts at the next pixel the trip
 * reaches that carries a value.
 */
static void
raster_run_enter(RasterRun *run, const RasterGridOps *ops, int col, int row,
  TimestampTz t, RasterAnswer *answer)
{
  double value;
  if (col >= 0 && col < ops->width && row >= 0 && row < ops->height &&
      ops->pixel(ops->ctx, col, row, &value))
  {
    if (run->count == 0 ||
        DatumGetFloat8(tinstant_value_p(run->insts[run->count - 1])) != value)
      raster_run_push(run, value, t);
    return;
  }
  raster_run_close(run, t, false, answer);
  run->lower_inc = true;
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
 * @brief Return the values a raster holds along a linearly interpolated
 * sequence, as the sequences of a step temporal float
 * @details A trajectory moving between two instants passes over the pixels
 * between them, and their values belong to the answer as much as the values
 * under the instants themselves. Each segment is therefore TRAVERSED pixel by
 * pixel. The grid coordinates of a position are monotonic along a straight
 * segment, affine in it for a PostGIS raster or a GDAL file and with the row
 * following the Mercator ordinate for a Raquet tile, so the parameter at
 * which the segment reaches the next column line and the next row line is
 * solved directly. Stepping to the nearer of the two moves to an adjacent
 * pixel every time, and to the diagonal one where the segment passes through
 * a pixel corner, so the walk cannot pass over a pixel, and each value is
 * read from the instant the trip reaches the pixel holding it, which is step
 * interpolation. A walk sampling the segment at a spacing has neither
 * property at any spacing: a segment clips a pixel corner over an
 * arbitrarily short chord.
 *
 * A position outside the raster or over a nodata pixel carries no value and
 * ends the run, so a trip leaving and re-entering the raster answers one
 * sequence per visit.
 * @param[in] seq Trajectory sequence with linear interpolation
 * @param[in] ops Grid the values are read from
 * @param[in,out] answer Sequences of the answer, appended to
 */
static void
tpointseq_raster_value_traverse(const TSequence *seq, const RasterGridOps *ops,
  RasterAnswer *answer)
{
  RasterRun run = {NULL, 0, 0, seq->period.lower_inc};
  /* The positions are read, never kept, and the instants that hold them
   * outlive this call, so they are borrowed rather than copied */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(
    (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst1)));
  double g1[2];
  ops->grid(ops->ctx, p1->x, p1->y, &g1[0], &g1[1]);
  int col = raster_cell_index(g1[0], ops->width);
  int row = raster_cell_index(g1[1], ops->height);
  /* The trip starts in the pixel holding its first position */
  raster_run_enter(&run, ops, col, row, inst1->t, answer);

  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst2)));
    double g2[2];
    ops->grid(ops->ctx, p2->x, p2->y, &g2[0], &g2[1]);
    int ecol = raster_cell_index(g2[0], ops->width);
    int erow = raster_cell_index(g2[1], ops->height);
    /* A segment whose two ends lie on the same side outside the grid along
     * one axis lies there along its whole length, since the grid coordinate
     * is monotonic along it, and meets no pixel */
    bool outside = (col == ecol && (col < 0 || col >= ops->width)) ||
      (row == erow && (row < 0 || row >= ops->height));
    if (! outside)
    {
      int stepc = (ecol > col) ? 1 : ((ecol < col) ? -1 : 0);
      int stepr = (erow > row) ? 1 : ((erow < row) ? -1 : 0);
      double dt = (double) (inst2->t - inst1->t);
      double s_prev = 0.0;
      /* One step for each column line and each row line the segment
       * crosses bounds the walk */
      int guard = abs(ecol - col) + abs(erow - row);
      while ((col != ecol || row != erow) && guard-- > 0)
      {
        double sc = (col == ecol) ? DBL_MAX :
          raster_cross_param(ops, p1, p2, g1, g2, 0,
            (double) ((stepc > 0) ? col + 1 : col));
        double sr = (row == erow) ? DBL_MAX :
          raster_cross_param(ops, p1, p2, g1, g2, 1,
            (double) ((stepr > 0) ? row + 1 : row));
        if (sc == DBL_MAX && sr == DBL_MAX)
          break;
        double s;
        if (sc < sr)
        {
          col += stepc;
          s = sc;
        }
        else if (sr < sc)
        {
          row += stepr;
          s = sr;
        }
        else
        {
          /* The segment passes through a pixel corner, touching the two
           * pixels beside it at that point alone */
          col += stepc;
          row += stepr;
          s = sc;
        }
        /* Rounding may place a crossing a hair before the previous one or
         * past the end of the segment */
        if (s < s_prev)
          s = s_prev;
        if (s > 1.0)
          s = 1.0;
        s_prev = s;
        raster_run_enter(&run, ops, col, row,
          inst1->t + (TimestampTz) (dt * s), answer);
      }
      /* The end of the segment lies in the pixel holding its position */
      if (col != ecol || row != erow)
        raster_run_enter(&run, ops, ecol, erow, inst2->t, answer);
    }
    /* The next segment starts where this one ends */
    col = ecol;
    row = erow;
    inst1 = inst2;
    p1 = p2;
    g1[0] = g2[0];
    g1[1] = g2[1];
  }
  /* The last value holds to the end of the trip, which the closing instant
   * states: a sequence reaches no further than its last instant */
  raster_run_close(&run, TSEQUENCE_INST_N(seq, seq->count - 1)->t,
    seq->period.upper_inc, answer);
  return;
}

/**
 * @brief Return the values of a raster read along a trajectory
 * @details The pixel access is delegated to the grid descriptor so that the
 * one algorithm serves any raster engine: a PostGIS raster is read through
 * the vendored raster core, a raster file through GDAL, a Raquet tile from
 * its own pixel array.
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
   * position, or that states nothing between its instants, is read at the
   * instants */
  if (MEOS_FLAGS_GET_INTERP(traj->flags) == LINEAR)
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
      tpointseq_raster_value_traverse(seqs[i], ops, &answer);
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
    MEOS_FLAGS_GET_INTERP(traj->flags) == STEP ? STEP : DISCRETE, NORMALIZE);
}

/*****************************************************************************
 * raster_at_value_sampler / raster_minus_value_sampler /
 * eraster_value_sampler / araster_value_sampler
 *****************************************************************************/

/**
 * @brief Return a trajectory restricted to the instants where the sampled
 * raster pixel value falls inside a float span
 * @details Equivalent to, with
 * @p v = #raster_value_sampler(traj, ops):
 * @code
 * atTime(traj, getTime(atSpan(v, vspan)))
 * @endcode
 * composed here in C on top of #tnumber_restrict_span, #temporal_time and
 * #temporal_restrict_tstzspanset, so that every caller supplying a @p sample
 * callback gets the restriction.
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

  Temporal *v = raster_value_sampler(traj, ops);
  if (! v)
    return NULL;
  Temporal *v1 = tnumber_restrict_span(v, vspan, REST_AT);
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
 * @brief Return a trajectory restricted to the instants where the sampled
 * raster pixel value falls outside a float span
 * @details Equivalent to, with
 * @p v = #raster_value_sampler(traj, ops):
 * @code
 * atTime(traj, getTime(minusSpan(v, vspan)))
 * @endcode
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

  Temporal *v = raster_value_sampler(traj, ops);
  if (! v)
    return NULL;
  Temporal *v1 = tnumber_restrict_span(v, vspan, REST_MINUS);
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

  Temporal *v = raster_value_sampler(traj, ops);
  if (! v)
    return 0;
  Temporal *v1 = tnumber_restrict_span(v, vspan, REST_AT);
  pfree(v);
  bool result = (v1 != NULL);
  if (v1)
    pfree(v1);
  return result ? 1 : 0;
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

  Temporal *v = raster_value_sampler(traj, ops);
  if (! v)
    return 0;
  Temporal *v1 = tnumber_restrict_span(v, vspan, REST_MINUS);
  pfree(v);
  bool result = (v1 == NULL);
  if (v1)
    pfree(v1);
  return result ? 1 : 0;
}

/*****************************************************************************
 * trajectory_quadbins
 *****************************************************************************/

/**
 * @brief Return the lon/lat a temporal point instant holds
 * @details The position is read, never kept, and the instant that holds it
 * outlives the call, so it is borrowed rather than copied
 */
static void
tinstant_point_coords(const TInstant *inst, double *lon, double *lat)
{
  const GSERIALIZED *gs =
    (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst));
  GBOX box;
  gserialized_get_gbox_p((GSERIALIZED *) gs, &box);
  *lon = box.xmin;
  *lat = box.ymin;
  return;
}

/**
 * @brief Return the QUADBIN cell of a position at a zoom, through the
 * slippy-tile Mercator the family decodes its tiles with
 */
static uint64
quadbin_cell_at(double lon, double lat, uint32_t zoom)
{
  double n = (double) (UINT64_C(1) << zoom);
  uint32_t tx = (uint32_t) floor((lon + 180.0) / 360.0 * n);
  double merc_y = log(tan(M_PI / 4.0 + lat * M_PI / 360.0));
  uint32_t ty = (uint32_t) floor((1.0 - merc_y / M_PI) / 2.0 * n);
  /* Clamp to the valid tile range at this zoom */
  uint32_t maxidx = (uint32_t) n - 1;
  if (tx > maxidx) tx = maxidx;
  if (ty > maxidx) ty = maxidx;
  return xyz_to_qb(tx, ty, zoom);
}


/**
 * @brief Add a cell to the answer unless it is already there
 * @details The answer is a set, and a trip stays in one tile across many
 * positions, so the walk repeats a cell far more often than it changes one
 */
static void
quadbin_cells_add(uint64 *cells, int *ncells, uint64 cell)
{
  for (int i = 0; i < *ncells; i++)
    if (cells[i] == cell)
      return;
  cells[(*ncells)++] = cell;
  return;
}

/**
 * @brief Return the tile column holding a longitude at a zoom
 */
static double
qb_tile_x_at(double lon, double n)
{
  return (lon + 180.0) / 360.0 * n;
}

/**
 * @brief Return the tile row holding a latitude at a zoom
 * @details The row grows SOUTHWARD: the Mercator ordinate falls as the
 * latitude rises, so a rising latitude walks towards row zero.
 */
static double
qb_tile_y_at(double lat, double n)
{
  double merc = log(tan(M_PI / 4.0 + lat * M_PI / 360.0));
  return (1.0 - merc / M_PI) / 2.0 * n;
}

/**
 * @brief Return the latitude at which a tile row begins
 * @details The inverse of #qb_tile_y_at(): the row boundary `k` sits at the
 * Mercator ordinate `pi (1 - 2k/n)`, and the latitude follows from the
 * Gudermannian. Having it in closed form is what lets the walk below jump to
 * the crossing instead of hunting for it.
 */
static double
qb_lat_at_tile_y(double y, double n)
{
  double merc = M_PI * (1.0 - 2.0 * y / n);
  return (atan(exp(merc)) - M_PI / 4.0) * 360.0 / M_PI;
}

/**
 * @brief Add every tile the segment between two positions crosses
 * @details A grid traversal, not a sampling walk. The segment is straight in
 * lon/lat, the tile column is linear in the longitude and the tile row is
 * monotonic in the latitude, so the parameter at which the path leaves its
 * current tile through either boundary is available in closed form. Stepping
 * to the nearer of the two crossings moves to the ADJACENT tile every time,
 * and a walk that only ever moves to a neighbour cannot pass over a tile.
 *
 * That is the property a sampling walk cannot have at any step: a segment
 * clips a tile corner over an arbitrarily short chord, so for every spacing
 * there is a chord shorter than it, and the tile holding that chord is absent
 * from the answer. The cost is one step per tile crossed, which is the size
 * of the answer rather than a multiple of it.
 */
static void
quadbin_segment_cells_add(uint64 *cells, int *ncells, double lon1, double lat1,
  double lon2, double lat2, uint32_t zoom)
{
  double n = (double) (UINT64_C(1) << zoom);
  long maxidx = (long) n - 1;
  /* The row is undefined beyond the Mercator limit, which is where
   * #quadbin_cell_at() clamps too */
  double la1 = lat1, la2 = lat2;
  if (la1 >  QB_MAX_LATITUDE) la1 =  QB_MAX_LATITUDE;
  if (la1 < -QB_MAX_LATITUDE) la1 = -QB_MAX_LATITUDE;
  if (la2 >  QB_MAX_LATITUDE) la2 =  QB_MAX_LATITUDE;
  if (la2 < -QB_MAX_LATITUDE) la2 = -QB_MAX_LATITUDE;

  double x0 = qb_tile_x_at(lon1, n), y0 = qb_tile_y_at(la1, n);
  double x1 = qb_tile_x_at(lon2, n), y1 = qb_tile_y_at(la2, n);
  long tx = (long) floor(x0), ty = (long) floor(y0);
  long ex = (long) floor(x1), ey = (long) floor(y1);
  if (tx < 0) tx = 0; else if (tx > maxidx) tx = maxidx;
  if (ty < 0) ty = 0; else if (ty > maxidx) ty = maxidx;
  if (ex < 0) ex = 0; else if (ex > maxidx) ex = maxidx;
  if (ey < 0) ey = 0; else if (ey > maxidx) ey = maxidx;
  quadbin_cells_add(cells, ncells, xyz_to_qb((uint32_t) tx, (uint32_t) ty,
    zoom));

  int stepx = (ex > tx) ? 1 : ((ex < tx) ? -1 : 0);
  int stepy = (ey > ty) ? 1 : ((ey < ty) ? -1 : 0);
  double dlon = lon2 - lon1, dlat = la2 - la1;
  /* The traversal visits one tile per column step and one per row step, so
   * the bound is exact and the guard only catches a coordinate no boundary
   * can be solved for */
  long guard = labs(ex - tx) + labs(ey - ty) + 1;

  while ((tx != ex || ty != ey) && guard-- > 0)
  {
    double tX = DBL_MAX, tY = DBL_MAX;
    if (stepx != 0 && dlon != 0.0)
    {
      double bx = (double) ((stepx > 0) ? tx + 1 : tx);
      tX = (bx / n * 360.0 - 180.0 - lon1) / dlon;
    }
    if (stepy != 0 && dlat != 0.0)
    {
      double by = (double) ((stepy > 0) ? ty + 1 : ty);
      tY = (qb_lat_at_tile_y(by, n) - la1) / dlat;
    }
    if (tX == DBL_MAX && tY == DBL_MAX)
      break;
    if (tX <= tY)
      tx += stepx;
    else
      ty += stepy;
    if (tx < 0) tx = 0; else if (tx > maxidx) tx = maxidx;
    if (ty < 0) ty = 0; else if (ty > maxidx) ty = maxidx;
    quadbin_cells_add(cells, ncells, xyz_to_qb((uint32_t) tx, (uint32_t) ty,
      zoom));
  }
}




/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the unique QUADBIN cells at @p zoom covered by a trajectory.
 * @details Suitable for use as the WHERE-clause argument when joining against
 * a Raquet table:
 * @code{.sql}
 *   SELECT raster_tile_value_quadbin(band_data, 256, 256, quadbin, ...)
 *   FROM   elevation_raquet
 *   WHERE  quadbin = ANY(trajectory_quadbins(traj, 8));
 * @endcode
 * @param[in] traj Input tgeompoint trajectory (SRID 4326)
 * @param[in] zoom Raquet zoom level (0–15)
 * @param[out] count Number of distinct cells returned
 * @return Palloc'd array of QUADBIN cell identifiers
 * @csqlfn #Trajectory_quadbins()
 */
uint64 *
trajectory_quadbins(const Temporal *traj, uint32_t zoom, int *count)
{
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL(count, NULL);
  /* The zoom bounds the shift building the tile grid below. A caller passing a
   * negative zoom reaches this as a large unsigned value, so the upper test
   * covers both ends of the documented range */
  if (zoom > QB_TRAJECTORY_MAX_ZOOM)
  {
    *count = 0;
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "zoom level must be between 0 and %d", QB_TRAJECTORY_MAX_ZOOM);
    return NULL;
  }

  int ninsts;
  const TInstant **insts = temporal_insts_p(traj, &ninsts);
  bool densify = (MEOS_FLAGS_GET_INTERP(traj->flags) == LINEAR);

  /* One cell per instant, and one per tile the traversal steps through when
   * the trajectory moves between them. A traversal crosses at most one tile
   * per column step plus one per row step, so the bound is the grid distance
   * between the two endpoints */
  double nn = (double) (UINT64_C(1) << zoom);
  int maxcells = ninsts;
  if (densify)
    for (int i = 0; i + 1 < ninsts; i++)
    {
      double lo1, la1, lo2, la2;
      tinstant_point_coords(insts[i], &lo1, &la1);
      tinstant_point_coords(insts[i + 1], &lo2, &la2);
      maxcells += (int) (fabs(qb_tile_x_at(lo2, nn) - qb_tile_x_at(lo1, nn)) +
        fabs(qb_tile_y_at(la2, nn) - qb_tile_y_at(la1, nn))) + 3;
    }
  uint64 *cells = palloc(sizeof(uint64) * (size_t) maxcells);
  int ncells = 0;

  for (int i = 0; i < ninsts; i++)
  {
    double lon, lat;
    tinstant_point_coords(insts[i], &lon, &lat);
    quadbin_cells_add(cells, &ncells, quadbin_cell_at(lon, lat, zoom));

    /* A trajectory that moves between its instants passes over the tiles
     * between them, and a join filtered on the cells it answers loses every
     * tile the trip crosses but the list omits. The segment is therefore
     * TRAVERSED tile by tile, which holds every one of them */
    if (! densify || i + 1 >= ninsts)
      continue;
    double lon2, lat2;
    tinstant_point_coords(insts[i + 1], &lon2, &lat2);
    quadbin_segment_cells_add(cells, &ncells, lon, lat, lon2, lat2, zoom);
  }

  pfree(insts);
  *count = ncells;
  return cells;
}

/*****************************************************************************/
