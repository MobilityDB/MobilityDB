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
 * @brief Reading a raster along a moving trajectory holds every pixel the
 * trajectory crosses, each from the instant the trajectory reaches it
 *
 * @details The truth is the SAME entry point over a DISCRETE sequence of many
 * positions along the identical segment. A discrete sequence is read at its
 * instants, one pixel per position, through the grid the value under test is
 * read through, so the two compare pixels rather than two encodings of them.
 * Two properties are held:
 * - every value the dense read meets is a value of the trajectory's answer;
 * - at every dense instant the answer holds the value of the pixel under the
 *   position, except within two microseconds of an instant of the answer,
 *   which is the rounding of a crossing time to whole microseconds.
 *
 * Every pixel carries a distinct value, so a pixel the answer passes over
 * shows as a missing value and a change read late as a wrong one. The
 * segments are drawn here because the property lives in the geometry: a pixel
 * is lost where a segment clips its corner, which needs many segments in
 * general position over an axis-aligned grid, a rotated one and a Mercator
 * tile. The SQL suite pins the concrete cases.
 *
 * @code
 * gcc -Wall -Werror=implicit-function-declaration -g -I/usr/local/include
 *   -o rasterwalk_test rasterwalk_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_raster.h>

/** Positions of the dense sequence the truth is read from */
#define DENSE_POSITIONS   4000
/** Segments drawn over each grid */
#define NSEGMENTS         200
/** Duration of every segment, one day in microseconds */
#define SEGMENT_USECS     ((TimestampTz) 86400000000LL)
/** Rounding of a crossing time to whole microseconds, with its margin */
#define ROUNDING_USECS    2

/**
 * @brief Grid a trajectory is read against: a PostGIS raster, or a Raquet
 * tile given by its pixels
 */
typedef struct
{
  const char *name;          /**< Name printed beside the counts */
  Raster *rast;              /**< PostGIS raster, or NULL for a tile */
  const uint8_t *pixels;     /**< Pixels of the tile */
  size_t size;               /**< Bytes of the tile */
  int width;                 /**< Columns of the tile */
  int height;                /**< Rows of the tile */
  uint64 quadbin;            /**< Cell of the tile */
  int32 srid;                /**< SRID of the trajectories */
  double x0, y0, span;       /**< Where the segments are drawn */
} Grid;

/**
 * @brief Return the values of a grid read along a trajectory
 */
static Temporal *
grid_read(const Grid *g, const Temporal *traj)
{
  if (g->rast)
    return raster_value(traj, g->rast, 1);
  return raster_tile_value_quadbin(traj, g->pixels, g->size, g->width,
    g->height, g->quadbin, MEOS_PT_UINT16, 0.0, false);
}

/**
 * @brief Append bytes to a buffer being written
 */
static void
put(uint8_t **p, const void *v, size_t n)
{
  memcpy(*p, v, n);
  *p += n;
}

/**
 * @brief Return a one-band 32BF PostGIS raster built from its Well-Known
 * Binary
 * @details The geotransform maps the pixel (col, row) to
 * (ipx + col * scalex + row * skewx, ipy + col * skewy + row * scaley). The
 * values are written in the byte order of the machine, which the first byte
 * of the Well-Known Binary states.
 */
static Raster *
raster_make32bf(uint16_t width, uint16_t height, double scalex,
  double scaley, double ipx, double ipy, double skewx, double skewy,
  const float *values, bool hasnodata, float nodata)
{
  size_t size = 61 + 1 + 4 + (size_t) width * height * 4;
  uint8_t *wkb = malloc(size), *p = wkb;
  uint16_t one = 1, version = 0, nbands = 1;
  uint8_t endian = *(uint8_t *) &one;
  int32_t srid = 0;
  put(&p, &endian, 1); put(&p, &version, 2); put(&p, &nbands, 2);
  put(&p, &scalex, 8); put(&p, &scaley, 8);
  put(&p, &ipx, 8); put(&p, &ipy, 8);
  put(&p, &skewx, 8); put(&p, &skewy, 8);
  put(&p, &srid, 4); put(&p, &width, 2); put(&p, &height, 2);
  /* Pixel type 32BF, with the flag stating a nodata value */
  uint8_t flags = (uint8_t) (10 | (hasnodata ? 0x40 : 0));
  put(&p, &flags, 1); put(&p, &nodata, 4);
  put(&p, values, (size_t) width * height * 4);
  Raster *rast = raster_from_wkb(wkb, size);
  free(wkb);
  return rast;
}

/**
 * @brief Return a PostGIS raster whose pixels hold distinct values
 */
static Raster *
raster_distinct(uint16_t width, uint16_t height, double scalex,
  double scaley, double ipx, double ipy, double skewx, double skewy)
{
  float *values = malloc(sizeof(float) * width * height);
  for (int i = 0; i < width * height; i++)
    values[i] = (float) (i + 1);
  Raster *rast = raster_make32bf(width, height, scalex, scaley, ipx, ipy,
    skewx, skewy, values, false, 0.0f);
  free(values);
  return rast;
}

/**
 * @brief Return a linear sequence of two instants a day apart
 */
static Temporal *
segment(int32 srid, double x1, double y1, double x2, double y2)
{
  GSERIALIZED *g1 = geompoint_make2d(srid, x1, y1);
  GSERIALIZED *g2 = geompoint_make2d(srid, x2, y2);
  TInstant *insts[2];
  insts[0] = tpointinst_make(g1, 0);
  insts[1] = tpointinst_make(g2, SEGMENT_USECS);
  Temporal *result = (Temporal *) tsequence_make(insts, 2, true, true, LINEAR,
    false);
  free(insts[0]); free(insts[1]); free(g1); free(g2);
  return result;
}

/**
 * @brief Return the discrete sequence of the dense positions of a segment,
 * with the timestamps of its instants in the last argument
 */
static Temporal *
dense_sequence(int32 srid, double x1, double y1, double x2, double y2,
  TimestampTz *times)
{
  TInstant **insts = malloc(sizeof(TInstant *) * (DENSE_POSITIONS + 1));
  for (int k = 0; k <= DENSE_POSITIONS; k++)
  {
    double f = (double) k / (double) DENSE_POSITIONS;
    times[k] = (TimestampTz) (f * (double) SEGMENT_USECS);
    GSERIALIZED *gs = geompoint_make2d(srid, x1 + f * (x2 - x1),
      y1 + f * (y2 - y1));
    insts[k] = tpointinst_make(gs, times[k]);
    free(gs);
  }
  Temporal *result = (Temporal *) tsequence_make(insts, DENSE_POSITIONS + 1,
    true, true, DISCRETE, false);
  for (int k = 0; k <= DENSE_POSITIONS; k++)
    free(insts[k]);
  free(insts);
  return result;
}

/**
 * @brief Return whether an instant of the answer lies within the rounding
 * of a crossing time from a timestamp
 */
static bool
near_instant(const TimestampTz *times, int count, TimestampTz t)
{
  for (int i = 0; i < count; i++)
    if (llabs(times[i] - t) <= ROUNDING_USECS)
      return true;
  return false;
}

/**
 * @brief Compare the answer along a segment with the dense read of it,
 * adding the values it misses and the dense instants it answers wrongly
 */
static void
check_segment(const Grid *g, double x1, double y1, double x2, double y2,
  long *missing, long *wrong)
{
  TimestampTz *times = malloc(sizeof(TimestampTz) * (DENSE_POSITIONS + 1));
  Temporal *seg = segment(g->srid, x1, y1, x2, y2);
  Temporal *dense = dense_sequence(g->srid, x1, y1, x2, y2, times);
  Temporal *vseg = grid_read(g, seg);
  Temporal *vdense = grid_read(g, dense);

  int nd = 0, ns = 0, nt = 0;
  double *dvals = vdense ? tfloat_values(vdense, &nd) : NULL;
  double *svals = vseg ? tfloat_values(vseg, &ns) : NULL;
  TimestampTz *stimes = vseg ? temporal_timestamps(vseg, &nt) : NULL;
  for (int i = 0; i < nd; i++)
  {
    bool found = false;
    for (int j = 0; j < ns && ! found; j++)
      found = (svals[j] == dvals[i]);
    if (! found)
      (*missing)++;
  }
  for (int k = 0; k <= DENSE_POSITIONS; k++)
  {
    double d = 0.0, s = 0.0;
    bool hd = vdense && tfloat_value_at_timestamptz(vdense, times[k], false,
      &d);
    bool hs = vseg && tfloat_value_at_timestamptz(vseg, times[k], false, &s);
    if (hd == hs && (! hd || d == s))
      continue;
    if (! near_instant(stimes, nt, times[k]))
      (*wrong)++;
  }
  free(dvals); free(svals); free(stimes);
  free(vseg); free(vdense); free(seg); free(dense); free(times);
  return;
}

int main(void)
{
  meos_initialize();
  int failures = 0;

  /* An axis-aligned grid of unit pixels over (0, 0)-(16, 16), and a rotated
   * one whose pixels are parallelograms */
  Raster *aligned = raster_distinct(16, 16, 1.0, -1.0, 0.0, 16.0, 0.0, 0.0);
  Raster *rotated = raster_distinct(16, 16, 0.9, -0.85, 0.0, 16.0, 0.25,
    0.35);

  /* A zoom-12 Raquet tile of 32 x 32 pixels, found through the cell a
   * trajectory at its position is covered by */
  const double tlon = 4.35, tlat = 50.85;
  GSERIALIZED *tpos = geompoint_make2d(4326, tlon, tlat);
  TInstant *tinst = tpointinst_make(tpos, 0);
  int ncells = 0;
  uint64 *cells = trajectory_quadbins((Temporal *) tinst, 12, &ncells);
  const int tw = 32, th = 32;
  uint8_t *tpix = malloc((size_t) tw * th * 2);
  for (int i = 0; i < tw * th; i++)
  {
    /* Little-endian, as a Raquet tile stores a pixel wider than a byte */
    tpix[2 * i] = (uint8_t) ((i + 1) & 0xff);
    tpix[2 * i + 1] = (uint8_t) ((i + 1) >> 8);
  }

  Grid grids[3] = {
    {"axis-aligned grid", aligned, NULL, 0, 0, 0, 0, 0, 8.0, 8.0, 9.0},
    {"rotated grid", rotated, NULL, 0, 0, 0, 0, 0, 9.2, 12.0, 9.0},
    {"mercator tile", NULL, tpix, (size_t) tw * th * 2, tw, th,
      ncells > 0 ? cells[0] : 0, 4326, tlon, tlat, 0.04},
  };

  for (int k = 0; k < 3; k++)
  {
    long missing = 0, wrong = 0;
    unsigned seed = 20260911u + (unsigned) k;
    for (int t = 0; t < NSEGMENTS; t++)
    {
      /* rand_r keeps the draw identical whatever else the process runs */
      double c[4];
      for (int i = 0; i < 4; i++)
        c[i] = ((rand_r(&seed) / (double) RAND_MAX) * 2.0 - 1.0) *
          grids[k].span;
      check_segment(&grids[k], grids[k].x0 + c[0], grids[k].y0 + c[1],
        grids[k].x0 + c[2], grids[k].y0 + c[3], &missing, &wrong);
    }
    printf("%-18s values missing %ld, dense instants answered wrongly %ld\n",
      grids[k].name, missing, wrong);
    if (missing > 0 || wrong > 0)
      failures++;
  }

  /* A trip over a row of pixels alternating between a value and nodata
   * answers one sequence per visit, six from a single segment */
  {
    float strip[12];
    for (int i = 0; i < 12; i++)
      strip[i] = (i % 2) ? -9999.0f : (float) (i / 2 + 1);
    Raster *rast = raster_make32bf(12, 1, 1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
      strip, true, -9999.0f);
    Temporal *seg = segment(0, 0.5, 0.5, 11.5, 0.5);
    Temporal *runs = raster_value(seg, rast, 1);
    int nruns = runs ? temporal_num_sequences(runs) : 0;
    printf("%-18s runs %d\n", "nodata strip", nruns);
    if (nruns != 6)
      failures++;
    free(runs); free(seg); free(rast);
  }

  free(aligned); free(rotated); free(cells); free(tinst); free(tpos);
  free(tpix);
  if (failures > 0)
  {
    printf("FAILED: a raster read along a trajectory misses or delays a "
      "pixel the trajectory crosses\n");
    meos_finalize();
    return 1;
  }
  printf("every raster read holds every pixel its trajectory crosses\n");
  meos_finalize();
  return 0;
}
