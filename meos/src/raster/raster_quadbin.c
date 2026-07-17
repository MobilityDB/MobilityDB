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
#include <math.h>
#include <stdint.h>
#include <string.h>
/* liblwgeom (vendored) */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
/* Raster */
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

static const uint64_t QB_B[6] = {
  UINT64_C(0x5555555555555555), UINT64_C(0x3333333333333333),
  UINT64_C(0x0F0F0F0F0F0F0F0F), UINT64_C(0x00FF00FF00FF00FF),
  UINT64_C(0x0000FFFF0000FFFF), UINT64_C(0x00000000FFFFFFFF)
};

/**
 * @brief Morton-decode a QUADBIN cell into Web-Mercator tile coordinates.
 *
 * Canonical compact_bits algorithm matching CARTO quadbin-js quadbinCellToTile:
 * extract even/odd bits, compact right (shifts 1→2→4→8→16), descale from 2^26.
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
 *
 * Canonical spread_bits algorithm matching CARTO quadbin-js quadbinTileToCell:
 * scale to 2^26 grid, spread left (shifts 16→8→4→2→1), interleave x/y.
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
 *
 * The Mercator top/bot values are kept in the caller-visible representation
 * to avoid recomputing them in the hot per-instant loop.
 */
static void
qb_bbox(uint32_t tx, uint32_t ty, uint32_t tz,
        double *xmin, double *xmax,
        double *ymin, double *ymax,
        double *top_merc, double *bot_merc)
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
 * @brief Derive the QUADBIN cell of a Web-Mercator raster tile from its
 * EPSG:3857 georeferencing.
 *
 * A Raquet tile is a single QUADBIN cell of the Web-Mercator tile pyramid, so
 * its EPSG:3857 origin and pixel resolution determine the cell exactly. The
 * pixel extent gives the zoom (a tile of zoom @p z spans 2*QB_MERC_MAX / 2^z
 * metres); the top-left origin gives the tile column and row. The raster must
 * be a single axis-aligned Web-Mercator tile: a non-square extent, an extent
 * that is not a power-of-two fraction of the world, or an origin off the tile
 * grid are rejected (mixing georeferencing that is not a QUADBIN tile is an
 * error, not a value to coerce).
 *
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
 * @brief Read a single pixel value at (col, row) from a row-major byte array.
 *
 * Pixel stride = sizeof(pixtype); no alignment assumptions (uses memcpy).
 */
static double
read_pixel(const uint8_t *pixels, int col, int row, int width,
           MeosPixType pixtype)
{
  const uint8_t *p;
  switch (pixtype)
  {
    case MEOS_PT_UINT8:
      p = pixels + (row * width + col);
      { uint8_t v; memcpy(&v, p, 1); return (double) v; }
    case MEOS_PT_INT16:
      p = pixels + (row * width + col) * 2;
      { int16_t v; memcpy(&v, p, 2); return (double) v; }
    case MEOS_PT_INT32:
      p = pixels + (row * width + col) * 4;
      { int32_t v; memcpy(&v, p, 4); return (double) v; }
    case MEOS_PT_FLOAT32:
      p = pixels + (row * width + col) * 4;
      { float v; memcpy(&v, p, 4); return (double) v; }
    case MEOS_PT_FLOAT64:
      p = pixels + (row * width + col) * 8;
      { double v; memcpy(&v, p, 8); return v; }
    default:
      return 0.0;
  }
}

/*****************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory.
 *
 * The chip is identified by its QUADBIN cell, which encodes the Web-Mercator
 * tile coordinates and thus the full georeferencing without any separate
 * metadata.  Instants outside the tile extent or on nodata pixels are
 * silently dropped; NULL is returned when no instants survive.
 *
 * @param[in] pixels   Row-major pixel bytes (all bands interleaved or
 *                     single-band depending on the Raquet producer)
 * @param[in] width    Tile width in pixels (typically 256)
 * @param[in] height   Tile height in pixels (typically 256)
 * @param[in] quadbin  CARTO QUADBIN cell identifier (uint64)
 * @param[in] pixtype  Pixel data type
 * @param[in] nodata   Nodata sentinel value
 * @param[in] has_nodata  Whether nodata filtering is active
 * @param[in] traj     Input tgeompoint trajectory (SRID 4326)
 * @return tfloat instant set, or NULL
 */
Temporal *
raster_tile_value_quadbin(const uint8_t *pixels, uint16_t width,
  uint16_t height, uint64 quadbin, MeosPixType pixtype,
  double nodata, bool has_nodata, const Temporal *traj)
{
  /* Decode QUADBIN → tile x, y, z → WGS84 bbox + Mercator top/bot */
  uint32_t tx, ty, tz;
  qb_to_xyz(quadbin, &tx, &ty, &tz);

  double xmin, xmax, ymin, ymax, top_merc, bot_merc;
  qb_bbox(tx, ty, tz, &xmin, &xmax, &ymin, &ymax, &top_merc, &bot_merc);
  double merc_height = top_merc - bot_merc;  /* > 0 */

  /* Iterate over trajectory instants */
  int count;
  const TInstant **insts = temporal_insts_p(traj, &count);
  TInstant **result_insts = palloc(sizeof(TInstant *) * count);
  int ninsts = 0;

  for (int i = 0; i < count; i++)
  {
    /* Extract lon/lat from point geometry via GBOX (xmin==xmax for points) */
    GSERIALIZED *pt_gs = (GSERIALIZED *) DatumGetPointer(tinstant_value(insts[i]));
    GBOX pt_box;
    gserialized_get_gbox_p(pt_gs, &pt_box);
    double lon = pt_box.xmin;
    double lat = pt_box.ymin;

    /* Tile bounding-box pre-filter */
    if (lon < xmin || lon > xmax || lat < ymin || lat > ymax)
      continue;

    /* Column: linear in longitude */
    int col = (int) floor((lon - xmin) / (xmax - xmin) * width);

    /* Row: Mercator inverse (latitude is non-linear in pixel space) */
    double merc_y = log(tan(M_PI / 4.0 + lat * M_PI / 360.0));
    int row = (int) floor((top_merc - merc_y) / merc_height * height);

    /* Guard against floating-point edge cases at tile boundaries */
    if (col < 0 || col >= (int) width || row < 0 || row >= (int) height)
      continue;

    double pixval = read_pixel(pixels, col, row, width, pixtype);
    if (has_nodata && pixval == nodata)
      continue;

    result_insts[ninsts++] =
      tinstant_make(Float8GetDatum(pixval), T_TFLOAT, insts[i]->t);
  }

  pfree(insts);

  if (ninsts == 0)
  {
    pfree(result_insts);
    return NULL;
  }
  return (Temporal *) tsequence_make_free(result_insts, ninsts,
                                          true, true, DISCRETE, NORMALIZE);
}

/*****************************************************************************
 * trajectory_quadbins
 *****************************************************************************/

/**
 * @ingroup meos_raster_base_accessor
 * @brief Return the unique QUADBIN cells at @p zoom covered by a trajectory.
 *
 * Suitable for use as the WHERE-clause argument when joining against a Raquet
 * table:
 *
 * @code{.sql}
 *   SELECT raster_tile_value_quadbin(band_data, 256, 256, quadbin, ...)
 *   FROM   elevation_raquet
 *   WHERE  quadbin = ANY(trajectory_quadbins(traj, 8));
 * @endcode
 *
 * @param[in]  traj   Input tgeompoint trajectory (SRID 4326)
 * @param[in]  zoom   Raquet zoom level (0–15)
 * @param[out] count  Number of distinct cells returned
 * @return Palloc'd array of QUADBIN cell identifiers
 */
uint64 *
trajectory_quadbins(const Temporal *traj, uint32_t zoom, int *count)
{
  int ninsts;
  const TInstant **insts = temporal_insts_p(traj, &ninsts);

  /* Upper bound: one cell per instant (usually far fewer after dedup) */
  uint64 *cells = palloc(sizeof(uint64) * ninsts);
  int ncells = 0;

  double n = (double)(UINT64_C(1) << zoom);

  for (int i = 0; i < ninsts; i++)
  {
    GSERIALIZED *pt_gs = (GSERIALIZED *) DatumGetPointer(tinstant_value(insts[i]));
    GBOX pt_box;
    gserialized_get_gbox_p(pt_gs, &pt_box);
    double lon = pt_box.xmin;
    double lat = pt_box.ymin;

    /* Tile indices via slippy-tile Mercator */
    uint32_t tx = (uint32_t) floor((lon + 180.0) / 360.0 * n);
    double lat_rad = lat * M_PI / 180.0;
    double merc_y  = log(tan(M_PI / 4.0 + lat_rad / 2.0));
    uint32_t ty = (uint32_t) floor((1.0 - merc_y / M_PI) / 2.0 * n);

    /* Clamp to valid tile range at this zoom */
    uint32_t maxidx = (uint32_t) n - 1;
    if (tx > maxidx) tx = maxidx;
    if (ty > maxidx) ty = maxidx;

    uint64_t cell = xyz_to_qb(tx, ty, zoom);

    /* Linear dedup (instants in a single tile cluster) */
    bool found = false;
    for (int j = 0; j < ncells; j++)
      if (cells[j] == cell) { found = true; break; }
    if (!found)
      cells[ncells++] = cell;
  }

  pfree(insts);
  *count = ncells;
  return cells;
}
