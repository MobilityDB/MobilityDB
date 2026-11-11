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
 * @brief Test of the owner of a boundary point of an H3 cell of
 * meos/src/h3/h3_geo.c
 * @details For every cell of the resolutions 0 to a maximum, the argument of
 * the program (2 by default): each edge of its outline is held by exactly one
 * of the two cells sharing it (#h3_outline_edge_held), and each vertex,
 * the extra vertices of the edges crossing a face of the icosahedron
 * included, by exactly one of the cells meeting there
 * (#h3_outline_vertex_held). One owner holds whatever the sense of the
 * outline, so the test also checks the sense the rule reads: away from the
 * poles and the antimeridian, the centre of the cell lies left of every
 * edge, so the edge a cell holds runs west, and is its north side.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <h3api.h>

#include <meos.h>
#include <meos_h3.h>
#include "geo/geo_funcs.h"
#include "h3/th3index_internal.h"

#define MAXRES 2

/* Index of the vertex of an outline with these doubles, or -1 */
static int
outline_vertex(const H3Outline *o, double lon, double lat)
{
  for (int k = 0; k < o->count; k++)
    if (o->lon[k] == lon && o->lat[k] == lat)
      return k;
  return -1;
}

/* Whether the outline lies away from the poles and the antimeridian */
static bool
outline_plain(const H3Outline *o)
{
  for (int k = 0; k < o->count; k++)
    if (o->lat[k] > 60.0 || o->lat[k] < -60.0 || o->lon[k] > 170.0 ||
        o->lon[k] < -170.0)
      return false;
  return true;
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
    long nedges = 0, nverts = 0, nextra = 0, nsense = 0, nheld = 0;
    long shape[7] = { 0 };
    /* The cells around the poles, by #h3_latlng_deg_to_cell */
    H3Index north = h3_latlng_deg_to_cell(90.0, 0.0, res);
    H3Index south = h3_latlng_deg_to_cell(-90.0, 0.0, res);
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
        H3Outline o;
        if (! h3_cell_outline(cells[c], &o))
        {
          printf("FAIL res %d: no outline for %llx\n", res,
            (unsigned long long) cells[c]);
          failures++;
          continue;
        }
        LatLng centre;
        cellToLatLng(cells[c], &centre);
        double cx = radsToDegs(centre.lng), cy = radsToDegs(centre.lat);
        bool plain = outline_plain(&o);
        int cellheld = 0, cellverts = 0, rawheld = 0;
        for (int k = 0; k < o.count; k++)
          cellverts += h3_outline_vertex_held(&o, k);
        for (int k = 0; k < o.count; k++)
        {
          int k1 = (k + 1) % o.count;
          bool held = h3_outline_edge_held(&o, k);
          nheld += held;
          rawheld += held;
          /* The edges of a cell, an edge with an extra vertex counted once */
          if (held && (k == 0 || o.across[k] != o.across[k - 1]))
            cellheld++;
          /* The sense of the outline */
          if (plain)
          {
            nsense++;
            if (cross_product_sign_exact(o.lon[k], o.lat[k], o.lon[k1],
                o.lat[k1], o.lon[k], o.lat[k], cx, cy) <= 0)
            {
              printf("FAIL res %d: the centre of %llx is not left of its "
                "edge %d\n", res, (unsigned long long) cells[c], k);
              failures++;
            }
          }
          /* The edge, read by the cell across it in the other sense */
          H3Outline n;
          if (! h3_cell_outline(o.across[k], &n))
          {
            printf("FAIL res %d: no outline for %llx\n", res,
              (unsigned long long) o.across[k]);
            failures++;
            continue;
          }
          int m = outline_vertex(&n, o.lon[k1], o.lat[k1]);
          if (m < 0 || n.lon[(m + 1) % n.count] != o.lon[k] ||
              n.lat[(m + 1) % n.count] != o.lat[k])
          {
            printf("FAIL res %d: %llx does not read edge %d of %llx\n", res,
              (unsigned long long) o.across[k], k,
              (unsigned long long) cells[c]);
            failures++;
            continue;
          }
          nedges++;
          if (held == h3_outline_edge_held(&n, m))
          {
            printf("FAIL res %d: edge %d of %llx held by %d cells\n", res, k,
              (unsigned long long) cells[c], held ? 2 : 0);
            failures++;
          }
        }
        /* The rule holds the north side, not the south one: the cell
         * around the north pole holds nothing, the one around the south
         * pole its whole ring */
        if ((cells[c] == north && (cellheld != 0 || cellverts != 0)) ||
            (cells[c] == south && (rawheld != o.count ||
              cellverts != o.count)))
        {
          printf("FAIL res %d: the polar cell %llx holds %d edges and %d of "
            "its %d vertices\n", res, (unsigned long long) cells[c],
            cellheld, cellverts, o.count);
          failures++;
        }
        if (! isPentagon(cells[c]) && cells[c] != north && cells[c] != south)
          shape[cellheld]++;
        /* Each vertex: the cell and the cells across its two edges */
        for (int k = 0; k < o.count; k++)
        {
          int kp = (k + o.count - 1) % o.count;
          H3Index meet[3] = { cells[c], o.across[kp], o.across[k] };
          int nmeet = meet[1] == meet[2] ? 2 : 3;
          nextra += (nmeet == 2);
          int holders = h3_outline_vertex_held(&o, k);
          for (int j = 1; j < nmeet; j++)
          {
            H3Outline n;
            int m;
            if (! h3_cell_outline(meet[j], &n) ||
                (m = outline_vertex(&n, o.lon[k], o.lat[k])) < 0)
            {
              printf("FAIL res %d: %llx does not read vertex %d of %llx\n",
                res, (unsigned long long) meet[j], k,
                (unsigned long long) cells[c]);
              failures++;
              continue;
            }
            holders += h3_outline_vertex_held(&n, m);
          }
          nverts++;
          if (holders != 1)
          {
            printf("FAIL res %d: vertex %d of %llx held by %d cells\n", res, k,
              (unsigned long long) cells[c], holders);
            failures++;
          }
        }
      }
      free(cells);
    }
    printf("res %d: %ld edges and %ld vertices read from each side, %ld of "
      "them extra vertices; %ld edges held; sense checked on %ld edges\n",
      res, nedges, nverts, nextra, nheld, nsense);
    printf("res %d: hexagons holding 0 to 6 edges: %ld %ld %ld %ld %ld %ld "
      "%ld\n", res, shape[0], shape[1], shape[2], shape[3], shape[4],
      shape[5], shape[6]);
    if (res % 2 == 1 && nextra == 0)
    {
      printf("FAIL res %d: no extra vertex met\n", res);
      failures++;
    }
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
