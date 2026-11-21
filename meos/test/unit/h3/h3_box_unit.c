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
 * @brief Test of the box of an H3 cell of meos/src/h3/th3index_boxops.c
 * @details For every cell of the resolutions 0 to a maximum, the argument of
 * the program (2 by default), the box #h3index_set_stbox states must hold
 * every vertex of the canonical outline of the cell (#h3_cell_outline), the
 * region that decides the cell holding a point and whether the cell meets a
 * box, and the box of a cell around a pole must reach the pole and span
 * every longitude.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <h3api.h>

#include <meos.h>
#include <meos_h3.h>
#include "h3/th3index_boxops.h"
#include "h3/th3index_internal.h"

#define MAXRES 2

int
main(int argc, char **argv)
{
  int maxres = argc > 1 ? atoi(argv[1]) : MAXRES;
  meos_initialize();
  int failures = 0;
  H3Index res0[122];
  getRes0Cells(res0);
  for (int res = 0; res <= maxres; res++)
  {
    long ncells = 0, poles = 0;
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
        ncells++;
        DggsOutline o;
        STBox box;
        memset(&box, 0, sizeof(STBox));
        if (! h3_cell_outline(cells[c], &o) ||
            ! h3index_set_stbox(cells[c], &box))
        {
          printf("FAIL: no outline or box for the cell %llx\n",
            (unsigned long long) cells[c]);
          failures++;
          continue;
        }
        for (int k = 0; k < o.count; k++)
          if (o.lon[k] < box.xmin || o.lon[k] > box.xmax ||
              o.lat[k] < box.ymin || o.lat[k] > box.ymax)
          {
            if (failures < 10)
              printf("FAIL: the vertex %d (%.17g, %.17g) of the cell %llx "
                "lies outside its box [%.17g, %.17g] x [%.17g, %.17g]\n", k,
                o.lon[k], o.lat[k], (unsigned long long) cells[c], box.xmin,
                box.xmax, box.ymin, box.ymax);
            failures++;
          }
        if (o.pole != 0)
        {
          poles++;
          if (box.xmin != -180.0 || box.xmax != 180.0 ||
              (o.pole == 1 && box.ymax != 90.0) ||
              (o.pole == -1 && box.ymin != -90.0))
          {
            printf("FAIL: the box of the cell %llx around a pole does not "
              "reach it across every longitude\n",
              (unsigned long long) cells[c]);
            failures++;
          }
        }
      }
      free(cells);
    }
    if (poles != 2)
    {
      printf("FAIL: %ld cells around a pole at the resolution %d\n", poles,
        res);
      failures++;
    }
    printf("resolution %d: %ld cells\n", res, ncells);
  }
  meos_finalize();
  if (failures)
  {
    printf("%d failures\n", failures);
    return EXIT_FAILURE;
  }
  printf("PASS\n");
  return EXIT_SUCCESS;
}
