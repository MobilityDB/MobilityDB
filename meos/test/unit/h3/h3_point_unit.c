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
 * @brief Test of the planar cell holding a position of meos/src/h3/h3_geo.c
 * @details A planar cell is its straight-edged outline (#h3_cell_outline),
 * holding its interior and, by the rule of the cell walks, its north and west
 * sides (#h3_outline_holds_point), the cell around a pole the region between
 * its ring and the pole. First, the rows of h3_point_rows.inc, each a
 * position whose cell the oracle of the plan of the cell walks decides in
 * rationals and the library, latLngToCell, names another: the point function
 * #h3_point_cell names the oracle's cell. Then, for every cell
 * of the resolutions 0 to a maximum, the argument of the program (1 by
 * default): each vertex of its outline, and the positions one ulp east,
 * west, north and south of it, lie in exactly one of the cells meeting there,
 * which #h3_point_cell names; each vertex of the boundary the library gives
 * the cell, where the outline and the library part, lies in the cell
 * #h3_point_cell names and in no other cell beside it.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <h3api.h>

#include <meos.h>
#include <meos_h3.h>
#include "h3/th3index_internal.h"

#define MAXRES 1

typedef struct
{
  double lon, lat;    /* Position */
  int res;            /* Resolution */
  H3Index cell;       /* Cell holding it, by the oracle */
} PointRow;

static const PointRow rows[] =
{
#include "h3_point_rows.inc"
};

static int failures = 0;

/* Read the outlines of cells by #h3_cell_outline, false when one has none */
static bool
read_outlines(const H3Index *cells, int ncells, DggsOutline *outs, int res)
{
  for (int j = 0; j < ncells; j++)
    if (! h3_cell_outline(cells[j], &outs[j]))
    {
      printf("FAIL res %d: no outline for %llx\n", res,
        (unsigned long long) cells[j]);
      failures++;
      return false;
    }
  return true;
}

/* Check that exactly one of the cells, of the outlines given, holds a
 * position, and that the point function names it, as #check of
 * quadbin_point_unit.c checks a tile */
static void
check_meet(const H3Index *cells, const DggsOutline *outs, int ncells,
  double lon, double lat, int res, const char *what)
{
  H3Index holder = 0;
  int holders = 0;
  for (int j = 0; j < ncells; j++)
  {
    if (h3_outline_holds_point(&outs[j], lon, lat))
    {
      holder = cells[j];
      holders++;
    }
  }
  if (holders != 1)
  {
    printf("FAIL res %d: %s (%a, %a) held by %d cells\n", res, what, lon, lat,
      holders);
    failures++;
    return;
  }
  H3Index cell = h3_point_cell(lon, lat, false, res);
  if (cell != holder)
  {
    printf("FAIL res %d: %s (%a, %a) is in %llx, not %llx\n", res, what, lon,
      lat, (unsigned long long) cell, (unsigned long long) holder);
    failures++;
  }
  return;
}

int
main(int argc, char **argv)
{
  int maxres = argc > 1 ? atoi(argv[1]) : MAXRES;
  meos_initialize();

  /* The rows of the oracle */
  int nrows = sizeof(rows) / sizeof(rows[0]);
  for (int i = 0; i < nrows; i++)
  {
    const PointRow *r = &rows[i];
    H3Index cell = h3_point_cell(r->lon, r->lat, false, r->res);
    if (cell != r->cell)
    {
      printf("FAIL row %d: (%a, %a) at resolution %d is in %llx, not %llx\n",
        i, r->lon, r->lat, r->res, (unsigned long long) cell,
        (unsigned long long) r->cell);
      failures++;
    }
  }
  printf("%d rows of the oracle, %d failures\n", nrows, failures);

  H3Index res0[122];
  getRes0Cells(res0);
  for (int res = 0; res <= maxres; res++)
  {
    int before = failures;
    long npos = 0;
    for (int b = 0; b < 122; b++)
    {
      int64_t sz;
      cellToChildrenSize(res0[b], res, &sz);
      H3Index *cells = calloc((size_t) sz, sizeof(H3Index));
      if (! cells)
      {
        printf("FAIL: out of memory\n");
        return EXIT_FAILURE;
      }
      cellToChildren(res0[b], res, cells);
      for (int64_t c = 0; c < sz; c++)
      {
        if (cells[c] == 0)
          continue;
        DggsOutline o;
        if (! h3_cell_outline(cells[c], &o))
        {
          printf("FAIL res %d: no outline for %llx\n", res,
            (unsigned long long) cells[c]);
          failures++;
          continue;
        }
        /* Each vertex and the positions one ulp around it, in the cells
         * meeting there: the cell and the cells across its two edges */
        for (int k = 0; k < o.count; k++)
        {
          int kp = (k + o.count - 1) % o.count;
          H3Index meet[3] = { cells[c], o.across[kp], o.across[k] };
          int nmeet = meet[1] == meet[2] ? 2 : 3;
          DggsOutline outs[3];
          if (! read_outlines(meet, nmeet, outs, res))
            continue;
          double x = o.lon[k], y = o.lat[k];
          check_meet(meet, outs, nmeet, x, y, res, "a vertex");
          check_meet(meet, outs, nmeet, nextafter(x, INFINITY), y, res,
            "east of a vertex");
          check_meet(meet, outs, nmeet, nextafter(x, -INFINITY), y, res,
            "west of a vertex");
          check_meet(meet, outs, nmeet, x, nextafter(y, INFINITY), res,
            "north of a vertex");
          check_meet(meet, outs, nmeet, x, nextafter(y, -INFINITY), res,
            "south of a vertex");
          npos += 5;
        }
        /* Each vertex of the boundary of the library, in the cell the point
         * function names and in no other cell beside it */
        CellBoundary bnd;
        cellToBoundary(cells[c], &bnd);
        for (int k = 0; k < bnd.numVerts; k++)
        {
          double x = radsToDegs(bnd.verts[k].lng);
          double y = radsToDegs(bnd.verts[k].lat);
          H3Index cell = h3_point_cell(x, y, false, res);
          H3Index disk[7] = { 0 };
          if (cell == 0 || gridDisk(cell, 1, disk) != E_SUCCESS)
          {
            printf("FAIL res %d: no cell for (%a, %a)\n", res, x, y);
            failures++;
            continue;
          }
          int nd = 0;
          for (int j = 0; j < 7; j++)
            if (disk[j] != 0)
              disk[nd++] = disk[j];
          DggsOutline outs[7];
          if (read_outlines(disk, nd, outs, res))
            check_meet(disk, outs, nd, x, y, res, "a vertex of the library");
          npos++;
        }
      }
      free(cells);
    }
    printf("res %d: %ld positions, %d failures\n", res, npos,
      failures - before);
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
