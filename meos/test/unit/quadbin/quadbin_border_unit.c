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
 * @brief Test of the border tile of the QUADBIN split of a temporal point
 * @details For every zoom from 1 to a maximum, the argument of the program
 * (10 by default), and every interior bound of the grid, a segment ends on
 * the bound or one ulp on each side of it. A tile holds its west and north
 * bounds, so the tile across the bound holds the end when the end lies on
 * its side or on the bound, and holds nothing but the border of the
 * trajectory when the end lies on the bound. The split of
 * #tgeompoint_quadbin_split holds that tile when it holds more than the
 * border, and when it holds the border alone only if borderInc is true, as
 * the split of a temporal point by a space grid lays its tiles. The fractional
 * tile coordinates, read in doubles, put a bound on the wrong side: the
 * longitude 2^-1074 at zoom 1 lies in column 1, which the split left out, and
 * the latitude of a row bound kept the row holding the border alone.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_quadbin.h>
#include "quadbin/quadbin.h"

#define MAXZOOM 10

static int failures = 0;

/* Return true if the split of the segment from (x0, y0) to (x1, y1) holds
 * the tile (x, y) */
static bool
split_holds(double x0, double y0, double x1, double y1, int32 resolution,
  bool border_inc, uint32_t x, uint32_t y)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "SRID=4326;[POINT(%.17g %.17g)@2001-01-01, "
    "POINT(%.17g %.17g)@2001-01-02]", x0, y0, x1, y1);
  Temporal *temp = tgeompoint_in(buf);
  Quadbin *cells = NULL;
  int count = 0;
  Temporal **parts = tgeompoint_quadbin_split(temp, resolution, border_inc,
    &cells, &count);
  Quadbin want = quadbin_tile_to_cell(x, y, (uint32_t) resolution);
  bool found = false;
  for (int i = 0; i < count; i++)
  {
    if (cells[i] == want)
      found = true;
    free(parts[i]);
  }
  free(parts);
  free(cells);
  free(temp);
  return found;
}

/* Check that the split holds the tile (x, y) exactly when it should */
static void
check(double x0, double y0, double x1, double y1, int32 resolution,
  bool border_inc, uint32_t x, uint32_t y, bool want, const char *what)
{
  if (split_holds(x0, y0, x1, y1, resolution, border_inc, x, y) != want)
  {
    printf("FAIL zoom %d borderInc %d: the split of the segment to %s "
      "(%a, %a) %s (%u, %u)\n", resolution, border_inc, what, x1, y1,
      want ? "does not hold" : "holds", x, y);
    failures++;
  }
  return;
}

int
main(int argc, char **argv)
{
  int32 maxzoom = argc > 1 ? atoi(argv[1]) : MAXZOOM;
  if (maxzoom > MAXZOOM)
    maxzoom = MAXZOOM;
  meos_initialize();

  /* The witness: at zoom 1 a segment running east to one ulp east of the
   * meridian 0 ends in column 1 */
  check(-45.0, 45.0, 0x1p-1074, 45.0, 1, false, 1, 0, true,
    "the witness longitude");

  for (int32 zoom = 1; zoom <= maxzoom; zoom++)
  {
    int before = failures;
    uint32_t n = UINT32_C(1) << zoom;
    for (uint32_t k = 1; k < n; k++)
    {
      double xmin, ymin, xmax, ymax;
      quadbin_cell_bounding_box(quadbin_tile_to_cell(k, k, (uint32_t) zoom),
        &xmin, &ymin, &xmax, &ymax);
      double lon = 0.5 * (xmin + xmax), lat = 0.5 * (ymin + ymax);
      double west = xmin - 0.75 * (xmax - xmin);
      double north = ymax + 0.75 * (ymax - ymin);
      for (int inc = 0; inc <= 1; inc++)
      {
        bool border_inc = (inc == 1);
        /* A segment running east to the west bound of column k: the column
         * holds the end east of the bound, and the end on the bound alone */
        check(west, lat, nextafter(xmin, INFINITY), lat, zoom, border_inc, k,
          k, true, "east of a bound");
        check(west, lat, xmin, lat, zoom, border_inc, k, k, border_inc,
          "the west bound");
        check(west, lat, nextafter(xmin, -INFINITY), lat, zoom, border_inc,
          k, k, false, "west of a bound");
        /* A segment running south to the north bound of row k: the row
         * holds the end south of the bound, and the end on the bound alone */
        check(lon, north, lon, nextafter(ymax, -INFINITY), zoom, border_inc,
          k, k, true, "south of a bound");
        check(lon, north, lon, ymax, zoom, border_inc, k, k, border_inc,
          "the north bound");
        check(lon, north, lon, nextafter(ymax, INFINITY), zoom, border_inc,
          k, k, false, "north of a bound");
      }
    }
    printf("zoom %d: %u tiles a side, %d failures\n", zoom, n,
      failures - before);
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
