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
 * @brief A trajectory's cell cover holds every cell the trajectory crosses
 *
 * @details The property under test is a SUPERSET one, and the truth it reads
 * against is the SAME entry point over a DISCRETE sequence of many positions
 * along the identical segment. A discrete sequence takes the
 * one-cell-per-instant path, so its gaps sit far below a cell and it cannot
 * step over one, while sharing the encoder with the value under test. A truth
 * built from a different entry compares two encodings and reports every cell
 * as missing, which measures the instrument rather than the cover.
 *
 * A cover holding a cell the dense walk does not reach is NOT a failure: the
 * dense walk is a lower bound on the cells a segment meets, so a traversal
 * legitimately finds a corner clip the walk steps over.
 *
 * A geodetic segment moves along its great circle, and its dense walk is
 * placed along that circle. Its cover is also held to the path: a cell the
 * walk does not reach is a corner the path clips between two positions, so it
 * borders a cell the walk reaches, and a cover cell bordering none lies off
 * the path.
 *
 * The segments are drawn here rather than read from a `tbl_` fixture because
 * the property lives in the geometry: a cell is lost exactly where a segment
 * clips its corner, which needs many short segments at a chosen latitude and
 * at the neighbourhood of a pentagon. The fixtures carry neither. The SQL
 * suite pins the concrete cases over the shared tables.
 *
 * @code
 * gcc -Wall -Werror=implicit-function-declaration -g -I/usr/local/include
 *   -o cellcover_test cellcover_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_raster.h>
#include <meos_s2cell.h>
#include <meos_internal.h>

/** Positions of the dense sequence the truth is read from */
#define DENSE_POSITIONS   2048
/** Segments drawn at each site */
#define NSEGMENTS         120
/** Length of the buffer a dense sequence is written into */
#define DENSE_BUFSZ       ((size_t) DENSE_POSITIONS * 64 + 64)

/**
 * @brief Return whether an array holds a value
 */
static bool
holds(const uint64 *arr, int n, uint64 v)
{
  for (int i = 0; i < n; i++)
    if (arr[i] == v)
      return true;
  return false;
}

/**
 * @brief Write a linear two-instant segment and the dense discrete sequence
 * covering the identical positions
 */
static void
segment_pair(char *seg, size_t segsz, char *dense, size_t densesz,
  double lon0, double lat0, double lon1, double lat1)
{
  snprintf(seg, segsz, "SRID=4326;[Point(%.9f %.9f)@2020-01-01 00:00:00, "
    "Point(%.9f %.9f)@2020-01-01 00:00:04]", lon0, lat0, lon1, lat1);
  int off = snprintf(dense, densesz, "SRID=4326;{");
  for (int s = 0; s <= DENSE_POSITIONS; s++)
  {
    double f = (double) s / (double) DENSE_POSITIONS;
    off += snprintf(dense + off, densesz - (size_t) off,
      "%sPoint(%.9f %.9f)@2020-01-01 00:00:%02d.%03d", (s ? ", " : ""),
      lon0 + f * (lon1 - lon0), lat0 + f * (lat1 - lat0),
      (s / 1000) % 60, s % 1000);
  }
  snprintf(dense + off, densesz - (size_t) off, "}");
}

/**
 * @brief Return how many cells of the dense walk the th3index cover of the
 * same segment does not hold, or -1 where a conversion answers nothing
 *
 * A conversion that fails answers NULL, and counting that as "no cell is
 * missing" reports a cover that was never built as a sound one, so it is
 * carried back as its own answer rather than folded into the count.
 */
static long
h3_missing(const char *seg_wkt, const char *dense_wkt, int32 resolution)
{
  Temporal *dense = tgeompoint_in(dense_wkt);
  Temporal *seg = tgeompoint_in(seg_wkt);
  if (dense == NULL || seg == NULL)
    return -1;
  Temporal *tcover = tgeompoint_to_th3index(seg, resolution);
  Temporal *ttruth = tgeompoint_to_th3index(dense, resolution);
  long missing = (tcover != NULL && ttruth != NULL) ? 0 : -1;
  if (tcover != NULL && ttruth != NULL)
  {
    int ncover = 0, ntruth = 0;
    H3Index *cover = th3index_values(tcover, &ncover);
    H3Index *truth = th3index_values(ttruth, &ntruth);
    for (int i = 0; i < ntruth; i++)
      if (! holds((const uint64 *) cover, ncover, (uint64) truth[i]))
        missing++;
    free(cover); free(truth);
  }
  free(tcover); free(ttruth); free(seg); free(dense);
  return missing;
}

/**
 * @brief Return how many tiles of the dense walk the quadbin cover of the
 * same segment does not hold
 */
static long
quadbin_missing(const char *seg_wkt, const char *dense_wkt, uint32_t zoom)
{
  Temporal *dense = tgeompoint_in(dense_wkt);
  Temporal *seg = tgeompoint_in(seg_wkt);
  if (dense == NULL || seg == NULL)
    return 0;
  int ncover = 0, ntruth = 0;
  uint64 *cover = trajectory_quadbins(seg, zoom, &ncover);
  uint64 *truth = trajectory_quadbins(dense, zoom, &ntruth);
  long missing = 0;
  for (int i = 0; i < ntruth; i++)
    if (! holds(cover, ncover, truth[i]))
      missing++;
  free(cover); free(truth); free(seg); free(dense);
  return missing;
}

/**
 * @brief Set the last argument to the unit vector of a longitude and latitude
 * in degrees
 */
static void
unit_vector(double lon, double lat, double p[3])
{
  double lo = lon * M_PI / 180.0, la = lat * M_PI / 180.0;
  p[0] = cos(la) * cos(lo);
  p[1] = cos(la) * sin(lo);
  p[2] = sin(la);
}

/**
 * @brief Write a linear two-instant geodetic segment and the dense discrete
 * sequence of positions along its great circle
 * @details The positions are placed by spherical linear interpolation of the
 * unit vectors of the endpoints, which follows the great circle a temporal
 * geodetic point moves along between two instants.
 */
static void
geodetic_segment_pair(char *seg, size_t segsz, char *dense, size_t densesz,
  double lon0, double lat0, double lon1, double lat1)
{
  snprintf(seg, segsz, "[Point(%.9f %.9f)@2020-01-01 00:00:00, "
    "Point(%.9f %.9f)@2020-01-01 00:00:04]", lon0, lat0, lon1, lat1);
  double a[3], b[3];
  unit_vector(lon0, lat0, a);
  unit_vector(lon1, lat1, b);
  double c = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  double omega = acos(c < -1.0 ? -1.0 : (c > 1.0 ? 1.0 : c));
  int off = snprintf(dense, densesz, "{");
  for (int s = 0; s <= DENSE_POSITIONS; s++)
  {
    double f = (double) s / (double) DENSE_POSITIONS;
    double wa = sin((1.0 - f) * omega) / sin(omega);
    double wb = sin(f * omega) / sin(omega);
    double p[3] = { wa * a[0] + wb * b[0], wa * a[1] + wb * b[1],
      wa * a[2] + wb * b[2] };
    double lon = atan2(p[1], p[0]) * 180.0 / M_PI;
    double lat = atan2(p[2], sqrt(p[0] * p[0] + p[1] * p[1])) * 180.0 / M_PI;
    off += snprintf(dense + off, densesz - (size_t) off,
      "%sPoint(%.9f %.9f)@2020-01-01 00:00:%02d.%03d", (s ? ", " : ""),
      lon, lat, (s / 1000) % 60, s % 1000);
  }
  snprintf(dense + off, densesz - (size_t) off, "}");
}

/**
 * @brief Return whether two H3 cells are neighbours
 */
static bool
cells_adjacent(H3Index a, H3Index b)
{
  Temporal *ta = th3index_make(a, 0);
  Temporal *tb = th3index_make(b, 0);
  Temporal *adj = (ta != NULL && tb != NULL) ?
    th3index_are_neighbor_cells(ta, tb) : NULL;
  bool result = (adj != NULL) && tbool_start_value(adj);
  free(ta); free(tb); free(adj);
  return result;
}

/**
 * @brief Return how many cells of the dense walk along a geodetic segment its
 * th3index cover does not hold, adding to the last argument the cover cells
 * that neither the walk reaches nor border a cell it reaches, or -1 where a
 * conversion answers nothing
 */
static long
h3_geodetic_missing(const char *seg_wkt, const char *dense_wkt,
  int32 resolution, long *offpath)
{
  Temporal *dense = tgeogpoint_in(dense_wkt);
  Temporal *seg = tgeogpoint_in(seg_wkt);
  if (dense == NULL || seg == NULL)
    return -1;
  Temporal *tcover = tgeogpoint_to_th3index(seg, resolution);
  Temporal *ttruth = tgeogpoint_to_th3index(dense, resolution);
  long missing = (tcover != NULL && ttruth != NULL) ? 0 : -1;
  if (tcover != NULL && ttruth != NULL)
  {
    int ncover = 0, ntruth = 0;
    H3Index *cover = th3index_values(tcover, &ncover);
    H3Index *truth = th3index_values(ttruth, &ntruth);
    for (int i = 0; i < ntruth; i++)
      if (! holds((const uint64 *) cover, ncover, (uint64) truth[i]))
        missing++;
    for (int i = 0; i < ncover; i++)
    {
      if (holds((const uint64 *) truth, ntruth, (uint64) cover[i]))
        continue;
      bool near = false;
      for (int j = 0; j < ntruth && ! near; j++)
        near = cells_adjacent(cover[i], truth[j]);
      if (! near)
        (*offpath)++;
    }
    free(cover); free(truth);
  }
  free(tcover); free(ttruth); free(seg); free(dense);
  return missing;
}

/**
 * @brief Count the timestamps at which the temporal quadbin cell of a linear
 * segment holds a cell other than the one holding the position of the segment
 * then, and the cells it holds that no such position reaches nor borders
 * @details The position at each timestamp is read from the temporal point
 * itself and its cell through the static adapter, which shares the encoder
 * with the value under test. An entry time is a crossing rounded to whole
 * microseconds, so the segment spans an hour: a sampled timestamp then falls
 * within a microsecond of a crossing with a chance far below one in a
 * million.
 */
static void
tquadbin_check(double lon0, double lat0, double lon1, double lat1,
  int32 resolution, long *wrong, long *offpath, long *none)
{
  char wkt[256];
  snprintf(wkt, sizeof(wkt), "SRID=4326;[Point(%.9f %.9f)@2020-01-01 00:00:00, "
    "Point(%.9f %.9f)@2020-01-01 01:00:00]", lon0, lat0, lon1, lat1);
  Temporal *seg = tgeompoint_in(wkt);
  Temporal *cover = (seg != NULL) ?
    tgeompoint_to_tquadbin(seg, resolution) : NULL;
  if (cover == NULL)
  {
    (*none)++;
    free(seg);
    return;
  }
  int ncover = 0;
  Quadbin *cells = tquadbin_values(cover, &ncover);
  Quadbin *truth = malloc(sizeof(Quadbin) * (DENSE_POSITIONS + 1));
  int ntruth = 0;
  TimestampTz t0 = temporal_start_timestamptz(seg);
  TimestampTz t1 = temporal_end_timestamptz(seg);
  for (int s = 0; s <= DENSE_POSITIONS; s++)
  {
    TimestampTz t = t0 + (t1 - t0) * s / DENSE_POSITIONS;
    GSERIALIZED *pos = NULL;
    Quadbin held = 0;
    if (! tgeo_value_at_timestamptz(seg, t, true, &pos) ||
        ! tquadbin_value_at_timestamptz(cover, t, true, &held))
    {
      (*wrong)++;
      free(pos);
      continue;
    }
    Quadbin cell = geo_to_quadbin_cell(pos, resolution);
    free(pos);
    if (held != cell)
      (*wrong)++;
    if (! holds(truth, ntruth, cell))
      truth[ntruth++] = cell;
  }
  /* A held cell the sampled positions do not reach is a corner the path clips
   * between two of them, so it borders a cell they reach */
  for (int i = 0; i < ncover; i++)
  {
    if (holds(truth, ntruth, cells[i]))
      continue;
    uint32_t x, y, z;
    quadbin_cell_to_tile(cells[i], &x, &y, &z);
    bool near = false;
    for (int j = 0; j < ntruth && ! near; j++)
    {
      uint32_t tx, ty, tz;
      quadbin_cell_to_tile(truth[j], &tx, &ty, &tz);
      near = (x + 1 >= tx && tx + 1 >= x && y + 1 >= ty && ty + 1 >= y);
    }
    if (! near)
      (*offpath)++;
  }
  free(truth); free(cells); free(cover); free(seg);
  return;
}

/**
 * @brief Count the timestamps at which the temporal S2 cell of a linear
 * geodetic segment holds a cell other than the one holding the position of the
 * segment then, and the cells it holds that no such position reaches nor
 * borders
 * @details As for #tquadbin_check, the position at each timestamp is read from
 * the temporal point itself and its cell through the static adapter, and the
 * segment spans an hour. A held cell the sampled positions do not reach is one
 * the path crosses between two of them, so it shares an edge with a cell they
 * reach.
 */
static void
ts2cell_check(double lon0, double lat0, double lon1, double lat1,
  int32 level, long *wrong, long *offpath, long *none)
{
  char wkt[256];
  snprintf(wkt, sizeof(wkt), "[Point(%.9f %.9f)@2020-01-01 00:00:00, "
    "Point(%.9f %.9f)@2020-01-01 01:00:00]", lon0, lat0, lon1, lat1);
  Temporal *seg = tgeogpoint_in(wkt);
  Temporal *cover = (seg != NULL) ? tgeogpoint_to_ts2cell(seg, level) : NULL;
  if (cover == NULL)
  {
    (*none)++;
    free(seg);
    return;
  }
  int ncover = 0;
  S2CellId *cells = ts2cell_values(cover, &ncover);
  S2CellId *truth = malloc(sizeof(S2CellId) * (DENSE_POSITIONS + 1));
  int ntruth = 0;
  TimestampTz t0 = temporal_start_timestamptz(seg);
  TimestampTz t1 = temporal_end_timestamptz(seg);
  for (int s = 0; s <= DENSE_POSITIONS; s++)
  {
    TimestampTz t = t0 + (t1 - t0) * s / DENSE_POSITIONS;
    GSERIALIZED *pos = NULL;
    S2CellId held = 0;
    if (! tgeo_value_at_timestamptz(seg, t, true, &pos) ||
        ! ts2cell_value_at_timestamptz(cover, t, true, &held))
    {
      (*wrong)++;
      free(pos);
      continue;
    }
    S2CellId cell = geo_to_s2cell_cell(pos, level);
    free(pos);
    if (held != cell)
      (*wrong)++;
    if (! holds(truth, ntruth, cell))
      truth[ntruth++] = cell;
  }
  for (int i = 0; i < ncover; i++)
  {
    if (holds(truth, ntruth, cells[i]))
      continue;
    int nnb = 0;
    S2CellId *nb = s2cell_edge_neighbors(cells[i], &nnb);
    bool near = false;
    for (int j = 0; j < nnb && ! near; j++)
      near = holds(truth, ntruth, nb[j]);
    free(nb);
    if (! near)
      (*offpath)++;
  }
  free(truth); free(cells); free(cover); free(seg);
  return;
}

int main(void)
{
  meos_initialize();

  /* The sites carry the three regimes a cover meets: an ordinary hexagon
   * field, the neighbourhood of an H3 pentagon, whose cells are the smallest
   * a resolution has, and the equator, where a degree of longitude spans its
   * greatest ground distance */
  const double site[3][2] = { { 55.5, 11.3 }, { 39.1, 122.3 }, { 0.5, 20.0 } };
  const char *name[3] = { "hexagon field", "beside a pentagon", "equator" };
  const int32 resolution = 12;
  const uint32_t zoom = 15;
  int failures = 0;

  for (int k = 0; k < 3; k++)
  {
    long h3_miss = 0, qb_miss = 0, h3_none = 0;
    unsigned seed = 20260906u + (unsigned) k;
    /* A segment that stays inside one cell crosses no boundary and asks the
     * question of nothing, so each family draws at ITS own scale: a few cell
     * widths for H3 at this resolution, a few tile widths for quadbin at this
     * zoom. The two differ by three orders of magnitude here */
    double h3_span = 0.00030;
    double qb_span = 360.0 / (double) (1ULL << zoom);
    for (int t = 0; t < NSEGMENTS; t++)
    {
      /* rand_r keeps the draw identical whatever else the process runs */
      for (int fam = 0; fam < 2; fam++)
      {
        double span = fam ? qb_span : h3_span;
        double j1 = (rand_r(&seed) / (double) RAND_MAX - 0.5) * span * 4.0;
        double j2 = (rand_r(&seed) / (double) RAND_MAX - 0.5) * span * 4.0;
        double ang = (rand_r(&seed) / (double) RAND_MAX) * 2.0 * M_PI;
        double len = span * (1.0 + (rand_r(&seed) / (double) RAND_MAX) * 4.0);
        double lat0 = site[k][0] + j1, lon0 = site[k][1] + j2;
        double lat1 = lat0 + len * sin(ang), lon1 = lon0 + len * cos(ang);

        char seg[256];
        char *dense = malloc(DENSE_BUFSZ);
        segment_pair(seg, sizeof(seg), dense, DENSE_BUFSZ, lon0, lat0, lon1,
          lat1);
        if (fam)
          qb_miss += quadbin_missing(seg, dense, zoom);
        else
        {
          long m = h3_missing(seg, dense, resolution);
          if (m < 0)
            h3_none++;
          else
            h3_miss += m;
        }
        free(dense);
      }
    }
    printf("%-20s th3index cells missing %ld, quadbin tiles missing %ld\n",
      name[k], h3_miss, qb_miss);
    if (h3_none > 0)
      printf("  %ld th3index cover(s) were not built at all\n", h3_none);
    if (h3_miss > 0 || qb_miss > 0 || h3_none > 0)
      failures++;
  }

  /* A cover states the time the path enters each cell, and a timestamp holds
   * whole microseconds, so a segment crossing more cells than its span holds
   * microseconds reaches two of them within one. The value must still be
   * built: the entry ORDER is what a step sequence carries, and the cells are
   * what a cover is for, so the two entries are separated by the smallest
   * step the type can state rather than one of them being dropped.
   *
   * The case is drawn here directly, because the same coincidence arises over
   * an ordinary span wherever a path passes near a vertex, where three cells
   * meet -- and a draw that waits for that is a test that usually does not
   * run. Crossing a kilometre of resolution-12 cells in five microseconds
   * forces it every time. */
  {
    const char *fast = "SRID=4326;"
      "[Point(11.300000000 55.500000000)@2020-01-01 00:00:00, "
      "Point(11.315000000 55.500000000)@2020-01-01 00:00:00.000005]";
    Temporal *seg = tgeompoint_in(fast);
    Temporal *cover = (seg != NULL) ?
      tgeompoint_to_th3index(seg, resolution) : NULL;
    int ncells = 0;
    H3Index *cells = (cover != NULL) ? th3index_values(cover, &ncells) : NULL;
    printf("%-20s cells %d\n", "crossings in one us", ncells);
    if (cover == NULL || ncells < 2)
    {
      printf("FAILED: a segment crossing cells faster than a microsecond "
        "builds no cover\n");
      failures++;
    }
    if (cells != NULL)
      free(cells);
    free(cover); free(seg);
  }

  /* A geodetic segment moves along its great circle. The regimes are the ones
   * a line in longitude and latitude departs from it most: a segment crossing
   * the antimeridian, at the equator and at 60N, one passing beside the pole,
   * and arcs of tens of degrees anywhere on the sphere at a coarse resolution,
   * where a dense walk still steps far below a cell */
  {
    const char *gname[4] = { "antimeridian at 0N", "antimeridian at 60N",
      "beside the pole", "long arcs" };
    for (int k = 0; k < 4; k++)
    {
      long miss = 0, offpath = 0, none = 0;
      unsigned seed = 20260913u + (unsigned) k;
      int32 res = (k == 3) ? 3 : resolution;
      for (int t = 0; t < NSEGMENTS; t++)
      {
        double r1 = rand_r(&seed) / (double) RAND_MAX;
        double r2 = rand_r(&seed) / (double) RAND_MAX;
        double r3 = rand_r(&seed) / (double) RAND_MAX;
        double r4 = rand_r(&seed) / (double) RAND_MAX;
        double lon0, lat0, lon1, lat1;
        if (k < 2)
        {
          double lat = (k == 0) ? 0.5 : 60.0;
          lon0 = 180.0 - 0.0010 * r1;
          lon1 = -180.0 + 0.0010 * r2;
          lat0 = lat + 0.0006 * (r3 - 0.5);
          lat1 = lat + 0.0006 * (r4 - 0.5);
        }
        else if (k == 2)
        {
          lon0 = 360.0 * r1 - 180.0;
          lon1 = 360.0 * r2 - 180.0;
          lat0 = 89.9990 + 0.0009 * r3;
          lat1 = 89.9990 + 0.0009 * r4;
        }
        else
        {
          lon0 = 360.0 * r1 - 180.0;
          lat0 = 160.0 * r2 - 80.0;
          lon1 = lon0 + 80.0 * (r3 - 0.5);
          if (lon1 > 180.0)
            lon1 -= 360.0;
          if (lon1 < -180.0)
            lon1 += 360.0;
          lat1 = lat0 + 80.0 * (r4 - 0.5);
          if (lat1 > 85.0)
            lat1 = 85.0;
          if (lat1 < -85.0)
            lat1 = -85.0;
        }
        char seg[256];
        char *dense = malloc(DENSE_BUFSZ);
        geodetic_segment_pair(seg, sizeof(seg), dense, DENSE_BUFSZ, lon0, lat0,
          lon1, lat1);
        long m = h3_geodetic_missing(seg, dense, res, &offpath);
        if (m < 0)
          none++;
        else
          miss += m;
        free(dense);
      }
      printf("%-20s th3index cells missing %ld, off the path %ld\n", gname[k],
        miss, offpath);
      if (none > 0)
        printf("  %ld th3index cover(s) were not built at all\n", none);
      if (miss > 0 || offpath > 0 || none > 0)
        failures++;
    }
  }

  /* A temporal quadbin cell holds at every timestamp the cell of the position
   * of its trajectory then, and no cell away from the path, over the sites
   * and at the scale of the quadbin covers above */
  for (int k = 0; k < 3; k++)
  {
    long wrong = 0, offpath = 0, none = 0;
    unsigned seed = 20260914u + (unsigned) k;
    double span = 360.0 / (double) (1ULL << zoom);
    for (int t = 0; t < NSEGMENTS; t++)
    {
      double j1 = (rand_r(&seed) / (double) RAND_MAX - 0.5) * span * 4.0;
      double j2 = (rand_r(&seed) / (double) RAND_MAX - 0.5) * span * 4.0;
      double ang = (rand_r(&seed) / (double) RAND_MAX) * 2.0 * M_PI;
      double len = span * (1.0 + (rand_r(&seed) / (double) RAND_MAX) * 4.0);
      double lat0 = site[k][0] + j1, lon0 = site[k][1] + j2;
      double lat1 = lat0 + len * sin(ang), lon1 = lon0 + len * cos(ang);
      tquadbin_check(lon0, lat0, lon1, lat1, zoom, &wrong, &offpath, &none);
    }
    printf("%-20s tquadbin positions in another cell %ld, off the path %ld\n",
      name[k], wrong, offpath);
    if (none > 0)
      printf("  %ld tquadbin value(s) were not built at all\n", none);
    if (wrong > 0 || offpath > 0 || none > 0)
      failures++;
  }

  /* A temporal S2 cell holds at every timestamp the cell of the position of
   * its trajectory then, and no cell away from the path. The regimes are the
   * ones a geodetic path meets on the cube: a crossing of the antimeridian at
   * the equator and at 60N, a path beside the pole, a path over the corner
   * where three cube faces meet, arcs of tens of degrees at a coarse level,
   * which pass from one face to the next, a path along the meridian 0 or 90,
   * which cube face 2 maps to a line between cells at every level, and a path
   * along one of those meridians over the pole and down the opposite one */
  {
    const char *sname[7] = { "antimeridian at 0N", "antimeridian at 60N",
      "beside the pole", "cube corner", "long arcs", "along a cell edge",
      "over the pole" };
    /* The corner of faces 0, 1 and 2 lies where x = y = z */
    const double clat = atan(1.0 / sqrt(2.0)) * 180.0 / M_PI;
    for (int k = 0; k < 7; k++)
    {
      long wrong = 0, offpath = 0, none = 0;
      unsigned seed = 20260915u + (unsigned) k;
      int32 level = (k == 4) ? 4 : 16;
      for (int t = 0; t < NSEGMENTS; t++)
      {
        double r1 = rand_r(&seed) / (double) RAND_MAX;
        double r2 = rand_r(&seed) / (double) RAND_MAX;
        double r3 = rand_r(&seed) / (double) RAND_MAX;
        double r4 = rand_r(&seed) / (double) RAND_MAX;
        double lon0, lat0, lon1, lat1;
        if (k < 2)
        {
          double lat = (k == 0) ? 0.5 : 60.0;
          lon0 = 180.0 - 0.02 * r1;
          lon1 = -180.0 + 0.02 * r2;
          lat0 = lat + 0.01 * (r3 - 0.5);
          lat1 = lat + 0.01 * (r4 - 0.5);
        }
        else if (k == 2)
        {
          lon0 = 360.0 * r1 - 180.0;
          lon1 = 360.0 * r2 - 180.0;
          lat0 = 89.990 + 0.009 * r3;
          lat1 = 89.990 + 0.009 * r4;
        }
        else if (k == 3)
        {
          lon0 = 45.0 + 0.02 * (r1 - 0.5);
          lon1 = 45.0 + 0.02 * (r2 - 0.5);
          lat0 = clat + 0.02 * (r3 - 0.5);
          lat1 = clat + 0.02 * (r4 - 0.5);
        }
        else if (k == 5)
        {
          lon0 = lon1 = (r1 < 0.5) ? 0.0 : 90.0;
          lat0 = 50.0 + 39.0 * r2;
          lat1 = lat0 + 0.02 * (r3 - 0.5);
        }
        else if (k == 6)
        {
          lon0 = (r1 < 0.5) ? 0.0 : 90.0;
          lon1 = lon0 - 180.0;
          lat0 = 89.990 + 0.009 * r2;
          lat1 = 89.990 + 0.009 * r3;
        }
        else
        {
          lon0 = 360.0 * r1 - 180.0;
          lat0 = 160.0 * r2 - 80.0;
          lon1 = lon0 + 80.0 * (r3 - 0.5);
          if (lon1 > 180.0)
            lon1 -= 360.0;
          if (lon1 < -180.0)
            lon1 += 360.0;
          lat1 = lat0 + 80.0 * (r4 - 0.5);
          if (lat1 > 85.0)
            lat1 = 85.0;
          if (lat1 < -85.0)
            lat1 = -85.0;
        }
        ts2cell_check(lon0, lat0, lon1, lat1, level, &wrong, &offpath, &none);
      }
      printf("%-20s ts2cell positions in another cell %ld, off the path %ld\n",
        sname[k], wrong, offpath);
      if (none > 0)
        printf("  %ld ts2cell value(s) were not built at all\n", none);
      if (wrong > 0 || offpath > 0 || none > 0)
        failures++;
    }
  }

  if (failures > 0)
  {
    printf("FAILED: a cover omits a cell its own dense walk reaches\n");
    meos_finalize();
    return 1;
  }
  printf("every cover holds every cell its dense walk reaches\n");
  meos_finalize();
  return 0;
}
