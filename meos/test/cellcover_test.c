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
#include <meos_raster.h>
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
