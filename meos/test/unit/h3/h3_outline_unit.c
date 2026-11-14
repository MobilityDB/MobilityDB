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
 * @brief Test of the canonical outline of an H3 cell of meos/src/h3/h3_geo.c
 * @details For every cell of the resolutions 0 to a maximum, the argument of
 * the program (2 by default), #h3_cell_outline
 * must answer, and every vertex a cell shares with a neighbour, a vertex of
 * the neighbour's outline within SHARED degrees, must be that vertex with the
 * same doubles, the extra vertices of the edges crossing a face of the
 * icosahedron included; the cell across each edge must be a neighbour whose
 * outline has an edge across to the cell and holds both ends of the edge,
 * which a cell across another edge does not. The extra vertices appear at the
 * odd resolutions only, and the test fails if none is met there, so that the
 * case they are read for is exercised.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <h3api.h>

#include <meos.h>
#include <meos_h3.h>
#include "h3/th3index_internal.h"

#define MAXRES 2
#define SHARED 1e-9

/* Index of a vertex of an outline within SHARED degrees of a position, or -1 */
static int
outline_near(const DggsOutline *o, double lon, double lat)
{
  for (int k = 0; k < o->count; k++)
    if (fabs(o->lon[k] - lon) < SHARED && fabs(o->lat[k] - lat) < SHARED)
      return k;
  return -1;
}

/* Whether a vertex of an outline is one the vertex index names */
static bool
outline_named(H3Index cell, double lon, double lat)
{
  H3Index verts[6] = { 0 };
  cellToVertexes(cell, verts);
  for (int k = 0; k < 6; k++)
  {
    LatLng v;
    if (verts[k] != 0 && vertexToLatLng(verts[k], &v) == E_SUCCESS &&
        radsToDegs(v.lng) == lon && radsToDegs(v.lat) == lat)
      return true;
  }
  return false;
}

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
    long ncells = 0, shared = 0, extra = 0;
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
        if (! h3_cell_outline(cells[c], &o))
        {
          printf("FAIL res %d: no outline for %llx\n", res,
            (unsigned long long) cells[c]);
          failures++;
          continue;
        }
        for (int k = 0; k < o.count; k++)
        {
          DggsOutline n;
          if (! h3_cell_outline(o.across[k], &n))
          {
            printf("FAIL res %d: no outline for %llx\n", res,
              (unsigned long long) o.across[k]);
            failures++;
            continue;
          }
          bool back = false;
          for (int m = 0; m < n.count; m++)
            back |= (n.across[m] == cells[c]);
          if (! back)
          {
            printf("FAIL res %d: %llx is across an edge of %llx, not "
              "the other way\n", res, (unsigned long long) o.across[k],
              (unsigned long long) cells[c]);
            failures++;
          }
          /* The two ends of the edge are vertices of the neighbour too */
          for (int e = k; e <= k + 1; e++)
          {
            int i = e % o.count;
            int j = outline_near(&n, o.lon[i], o.lat[i]);
            if (j < 0)
            {
              printf("FAIL res %d: %llx is across an edge of %llx whose "
                "end %d it does not hold\n", res,
                (unsigned long long) o.across[k],
                (unsigned long long) cells[c], i);
              failures++;
              continue;
            }
            shared++;
            if (! outline_named(cells[c], o.lon[i], o.lat[i]))
              extra++;
            if (n.lon[j] != o.lon[i] || n.lat[j] != o.lat[i])
            {
              printf("FAIL res %d: %llx and %llx state a shared vertex "
                "with different doubles\n", res,
                (unsigned long long) cells[c],
                (unsigned long long) o.across[k]);
              failures++;
            }
          }
        }
      }
      free(cells);
    }
    printf("res %d: %ld cells, %ld shared vertices read, %ld of them "
      "extra vertices\n", res, ncells, shared, extra);
    if (res % 2 == 1 && extra == 0)
    {
      printf("FAIL res %d: no extra vertex met\n", res);
      failures++;
    }
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
