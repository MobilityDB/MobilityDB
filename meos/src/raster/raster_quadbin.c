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
 * @brief Raquet sampling callback reading one pixel of a tile
 * @details The column is linear in longitude and the row is linear in the
 * Mercator ordinate, which is what the tile's own georeferencing states
 */
static bool
raster_tile_value_sample(void *ctxp, double x, double y, double *value)
{
  RaquetSampleState *state = (RaquetSampleState *) ctxp;
  if (x < state->xmin || x > state->xmax || y < state->ymin ||
      y > state->ymax)
    return false;

  int col = (int) floor((x - state->xmin) / (state->xmax - state->xmin) *
    state->width);
  double merc_y = log(tan(M_PI / 4.0 + y * M_PI / 360.0));
  int row = (int) floor((state->top_merc - merc_y) /
    (state->top_merc - state->bot_merc) * state->height);
  /* The half-open pixel convention of the tile: a position on the far edge
   * belongs to the neighbouring tile */
  if (col < 0 || col >= state->width || row < 0 || row >= state->height)
    return false;

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
 * @brief Return the distance between two positions of a walk over a Raquet
 * tile, half the smaller pixel side in the units the trajectory states
 * @details The columns of a tile divide its longitude evenly, while its rows
 * divide the Mercator ordinate evenly and so cover less latitude the further
 * they lie from the equator. The step therefore reads the shorter of the two
 * row extents, so that a walk cannot step over a pixel anywhere in the tile.
 */
static double
raquet_gridops_step(const RaquetSampleState *state)
{
  double lon_side = (state->xmax - state->xmin) / (double) state->width;
  double merc_side = (state->top_merc - state->bot_merc) /
    (double) state->height;
  /* Latitude of the Mercator ordinate one row inside each edge */
  double top_in = 90.0 - 360.0 / M_PI *
    atan(exp(- (state->top_merc - merc_side)));
  double bot_in = 90.0 - 360.0 / M_PI *
    atan(exp(- (state->bot_merc + merc_side)));
  double lat_top = fabs(state->ymax - top_in);
  double lat_bot = fabs(bot_in - state->ymin);
  double lat_side = (lat_top < lat_bot) ? lat_top : lat_bot;
  double smaller = (lon_side < lat_side) ? lon_side : lat_side;
  return (smaller > 0.0) ? smaller / 2.0 : 0.0;
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
  ops->sample = &raster_tile_value_sample;
  ops->ctx = state;
  ops->step = raquet_gridops_step(state);
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
 * separate metadata. Instants outside the tile extent or on nodata pixels are
 * silently dropped; NULL is returned when no instants survive.
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
 * @return tfloat instant set, or NULL
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
 * @brief Return the values of a raster sampled at the instants of a
 * trajectory
 * @details The pixel access is delegated to the @p sample callback so that
 * the one algorithm serves any raster engine: a PostGIS raster is read
 * through the vendored raster core, a raster file through GDAL, a Raquet
 * tile from its own pixel array.
 * @param[in] traj Trajectory (temporal geometry point)
 * @param[in] ops Grid the values are read from
 * @return A temporal float, or @p NULL when no instant of @p traj falls
 * inside the raster or survives nodata filtering
 */
/**
 * @brief Return half the smaller side of a pixel, the distance between two
 * positions of a walk over the raster
 * @details The step mirrors #h3_sample_step_deg(), which walks a hexagon at
 * half its edge: half a cell cannot step over a cell. The geotransform states
 * the two pixel sides as vectors, so a rotated raster answers the length of
 * its own sides rather than of their projections.
 * @param[in] gt Geotransform of the raster
 */
double
raster_sample_step(const double *gt)
{
  double pixel_w = hypot(gt[1], gt[4]), pixel_h = hypot(gt[2], gt[5]);
  double smaller = (pixel_w < pixel_h) ? pixel_w : pixel_h;
  return (smaller > 0.0) ? smaller / 2.0 : 0.0;
}

/**
 * @brief Return the pixel value at a position, or false when the position
 * lies outside the pre-filter box, outside the pixel grid, or on a nodata
 * pixel
 */
static bool
raster_sample_at(const STBox *box, raster_sample_fn sample, void *ctx,
  double x, double y, double *value)
{
  if (box && (x < box->xmin || x > box->xmax || y < box->ymin ||
      y > box->ymax))
    return false;
  return sample(ctx, x, y, value);
}

/**
 * @brief Return the values a raster holds along a linearly interpolated
 * sequence, as the sequences of a step temporal float
 * @details A trajectory moving between two instants passes over the pixels
 * between them, and their values belong to the answer as much as the values
 * under the instants themselves. The segment is therefore walked in steps of
 * @p step, the way #tpointseq_densify_to_th3index() walks one for a hexagon,
 * and a value is emitted where it differs from the one before it: a pixel
 * value holds until the trip reaches a pixel holding another, which is step
 * interpolation. A position outside the raster or over a nodata pixel carries
 * no value and ends the run, so a trip leaving and re-entering the raster
 * answers one sequence per visit.
 * @param[in] seq Trajectory sequence with linear interpolation
 * @param[in] box Bounding box of the raster used as a pre-filter, may be NULL
 * @param[in] sample Callback returning the pixel value at a position
 * @param[in] ctx Opaque context passed through to the callback
 * @param[in] step Distance between two walk positions, in the units of the
 * trajectory
 * @param[out] result Sequences of the answer, appended from @p nseqs
 * @param[in,out] nseqs Number of sequences written
 */
static void
tpointseq_densify_to_raster_value(const TSequence *seq, const STBox *box,
  raster_sample_fn sample, void *ctx, double step, TSequence **result,
  int *nseqs)
{
  /* An instant sequence holds one position, which the walk below cannot
   * improve on */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    /* The position is read, never kept, and the instant that holds it
     * outlives this call, so it is borrowed rather than copied */
    const POINT2D *p = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst)));
    double value;
    if (raster_sample_at(box, sample, ctx, p->x, p->y, &value))
    {
      TInstant **insts = palloc(sizeof(TInstant *));
      insts[0] = tinstant_make(Float8GetDatum(value), T_TFLOAT, inst->t);
      result[(*nseqs)++] = tsequence_make_free(insts, 1, true, true, STEP,
        NORMALIZE);
    }
    return;
  }

  /* The run being built: its instants, and the value last emitted */
  #define PUSH_INSTANT(_value, _t)                                       \
    do {                                                                 \
      if (ninsts >= maxinsts)                                            \
      {                                                                  \
        maxinsts = (maxinsts == 0) ? 8 : maxinsts * 2;                   \
        insts = (insts == NULL) ?                                        \
          palloc(sizeof(TInstant *) * (size_t) maxinsts) :               \
          repalloc(insts, sizeof(TInstant *) * (size_t) maxinsts);       \
      }                                                                  \
      insts[ninsts++] = tinstant_make(Float8GetDatum(_value), T_TFLOAT,  \
        (_t));                                                           \
    } while (0)

  TInstant **insts = NULL;
  int ninsts = 0, maxinsts = 0;
  double last_value = 0.0;
  bool lower_inc = seq->period.lower_inc;

  for (int i = 0; i + 1 < seq->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst1)));
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(
      (const GSERIALIZED *) DatumGetPointer(tinstant_value_p(inst2)));
    double dx = p2->x - p1->x, dy = p2->y - p1->y;
    double length = sqrt(dx * dx + dy * dy);
    /* One walk position per step, and the last of them is the end of the
     * segment, which the next segment reads as its own start */
    int nsteps = (int) ceil(length / step);
    if (nsteps < 1)
      nsteps = 1;
    bool last_segment = (i + 2 == seq->count);

    for (int j = 0; j <= nsteps; j++)
    {
      /* The end of a segment is the start of the next one, so it is read
       * once, with the last segment reading its own end */
      if (j == nsteps && ! last_segment)
        break;
      double frac = (double) j / (double) nsteps;
      double x = p1->x + frac * dx, y = p1->y + frac * dy;
      TimestampTz t = (j == 0) ? inst1->t :
        (j == nsteps) ? inst2->t :
        inst1->t + (TimestampTz) (frac * (double) (inst2->t - inst1->t));

      double value;
      bool have = raster_sample_at(box, sample, ctx, x, y, &value);
      if (! have)
      {
        /* The trip leaves the raster or reaches a nodata pixel: the value it
         * carried holds until here, so the run closes on it and the position
         * itself carries none */
        if (ninsts > 0)
        {
          PUSH_INSTANT(last_value, t);
          result[(*nseqs)++] = tsequence_make_free(insts, ninsts, lower_inc,
            false, STEP, NORMALIZE);
          insts = NULL; ninsts = maxinsts = 0;
        }
        lower_inc = true;
        continue;
      }
      if (ninsts > 0 && value == last_value)
        continue;   /* the pixel value still holds */
      PUSH_INSTANT(value, t);
      last_value = value;
    }
  }
  /* The last value holds to the end of the trip, which the closing instant
   * states: a sequence reaches no further than its last instant */
  if (ninsts > 0)
  {
    const TInstant *last = TSEQUENCE_INST_N(seq, seq->count - 1);
    if (insts[ninsts - 1]->t < last->t)
      PUSH_INSTANT(last_value, last->t);
    result[(*nseqs)++] = tsequence_make_free(insts, ninsts, lower_inc,
      seq->period.upper_inc, STEP, NORMALIZE);
  }
  return;
}

Temporal *
raster_value_sampler(const Temporal *traj, const RasterGridOps *ops)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(traj, NULL); VALIDATE_NOT_NULL((void *) ops, NULL);
  VALIDATE_NOT_NULL((void *) ops->sample, NULL);
  const STBox *box = &ops->box;
  raster_sample_fn sample = ops->sample;
  void *ctx = ops->ctx;
  double step = ops->step;

  /* A trajectory that moves between its instants passes over the pixels
   * between them, so the walk reads those too; one that holds its position,
   * or that states nothing between its instants, is read at the instants */
  if (MEOS_FLAGS_GET_INTERP(traj->flags) == LINEAR && step > 0.0)
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
     * position the raster does not answer for */
    int maxseqs = 0;
    for (int i = 0; i < nseqs_in; i++)
      maxseqs += seqs[i]->count;
    TSequence **result_seqs = palloc(sizeof(TSequence *) *
      (size_t) (maxseqs + nseqs_in));
    int nseqs = 0;
    for (int i = 0; i < nseqs_in; i++)
      tpointseq_densify_to_raster_value(seqs[i], box, sample, ctx, step, result_seqs,
        &nseqs);
    pfree(seqs);
    if (nseqs == 0)
    {
      pfree(result_seqs);
      return NULL;
    }
    if (nseqs == 1)
    {
      Temporal *result = (Temporal *) result_seqs[0];
      pfree(result_seqs);
      return result;
    }
    return (Temporal *) tsequenceset_make_free(result_seqs, nseqs, NORMALIZE);
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
    if (! raster_sample_at(box, sample, ctx, p->x, p->y, &pixval))
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
