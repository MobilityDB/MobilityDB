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
 * @brief Test of the tile holding a position of meos/src/quadbin/quadbin.c
 * @details For every zoom from 1 to a maximum, the argument of the program
 * (10 by default), a position on each bound of the tiles, the meridians and
 * the parallels of the grid, and one ulp on each side of it, lies in the tile
 * whose box, as #quadbin_cell_bounding_box states it, holds it by the rule of
 * the cell walks: a tile holds its west and north bounds (#quadbin_cell_edge_held),
 * the first row every latitude above it and the last row every latitude below
 * it, the poles included (decision D5). The point function
 * #quadbin_point_to_cell and both ends of the planar walk
 * #quadbin_segment_cells name that tile. The fractional coordinates of the
 * Mercator projection, read in doubles, put the position beside a bound in
 * the tile across it: at zoom 1 the longitude -2^-1074 in column 1 and the
 * latitude 2^-1074 in row 1.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_quadbin.h>
#include "quadbin/quadbin.h"

#define MAXZOOM 10

/* The longest planar walk of a test segment, from the centre of a tile to a
 * bound of another tile of the same column or row */
#define MAXCELLS (2 * (1 << MAXZOOM) + 4)

static int failures = 0;

/* Check that the point function and both ends of a planar walk put a
 * position in the tile (x, y) */
static void
check(double lon, double lat, uint32_t zoom, uint32_t x, uint32_t y,
  const char *what)
{
  uint32_t cx, cy, cz;
  quadbin_cell_tile(quadbin_point_to_cell(lon, lat, zoom), &cx, &cy, &cz);
  if (cx != x || cy != y)
  {
    printf("FAIL zoom %u: %s (%a, %a) is in (%u, %u), not (%u, %u)\n", zoom,
      what, lon, lat, cx, cy, x, y);
    failures++;
  }
  /* A walk from the centre of the grid ends there, a walk from there starts
   * there */
  static Quadbin cells[MAXCELLS];
  static double enter[MAXCELLS];
  Quadbin want = quadbin_tile_to_cell(x, y, zoom);
  int count = quadbin_segment_cells(0.25, 0.25, lon, lat, false, zoom, cells,
    enter, MAXCELLS);
  if (count < 1 || count == MAXCELLS || cells[count - 1] != want)
  {
    printf("FAIL zoom %u: the walk to %s (%a, %a) does not end in (%u, %u)\n",
      zoom, what, lon, lat, x, y);
    failures++;
  }
  count = quadbin_segment_cells(lon, lat, 0.25, 0.25, false, zoom, cells,
    enter, MAXCELLS);
  if (count < 1 || cells[0] != want)
  {
    printf("FAIL zoom %u: the walk from %s (%a, %a) does not start in "
      "(%u, %u)\n", zoom, what, lon, lat, x, y);
    failures++;
  }
  return;
}

int
main(int argc, char **argv)
{
  int maxzoom = argc > 1 ? atoi(argv[1]) : MAXZOOM;
  if (maxzoom > MAXZOOM)
    maxzoom = MAXZOOM;
  meos_initialize();

  /* The two witnesses, in the tiles their boxes state */
  check(-0x1p-1074, 0.5, 1, 0, 0, "the witness longitude");
  check(0.5, 0x1p-1074, 1, 1, 0, "the witness latitude");

  for (int z = 1; z <= maxzoom; z++)
  {
    uint32_t zoom = (uint32_t) z, n = UINT32_C(1) << z;
    int before = failures;
    for (uint32_t k = 0; k < n; k++)
    {
      double xmin, ymin, xmax, ymax;
      quadbin_cell_bounding_box(quadbin_tile_to_cell(k, k, zoom), &xmin,
        &ymin, &xmax, &ymax);
      /* A latitude and a longitude inside the tile (k, k), where the
       * positions on the other coordinate are put */
      double lat = 0.5 * (ymin + ymax), lon = 0.5 * (xmin + xmax);
      /* The west bound of column k, held by it; west of it, column k - 1;
       * the west bound of column 0 is the antimeridian, west of which there
       * is no longitude */
      check(xmin, lat, zoom, k, k, "the west bound");
      check(nextafter(xmin, INFINITY), lat, zoom, k, k, "east of a bound");
      if (k > 0)
        check(nextafter(xmin, -INFINITY), lat, zoom, k - 1, k,
          "west of a bound");
      /* The north bound of row k, held by it; north of it, row k - 1, or row
       * 0 for every latitude above the limit of the grid */
      check(lon, ymax, zoom, k, k, "the north bound");
      check(lon, nextafter(ymax, -INFINITY), zoom, k, k, "south of a bound");
      check(lon, nextafter(ymax, INFINITY), zoom, k, k > 0 ? k - 1 : 0,
        "north of a bound");
      if (k == n - 1)
      {
        /* The last row holds its south bound and every latitude below it */
        check(lon, ymin, zoom, k, k, "the south bound of the last row");
        check(lon, nextafter(ymin, -INFINITY), zoom, k, k,
          "south of the last row");
        check(lon, -90.0, zoom, k, k, "the south pole");
      }
      if (k == 0)
        check(lon, 90.0, zoom, k, k, "the north pole");
    }
    printf("zoom %d: %u tiles a side, %d failures\n", z, n, failures - before);
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
