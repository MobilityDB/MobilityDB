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
 * @brief Test of the owner of a boundary point of a QUADBIN tile of
 * meos/src/quadbin/quadbin.c
 * @details For every tile of the zooms 0 to a maximum, the argument of the
 * program (8 by default), each edge of its ring is held by exactly one of the
 * tiles sharing it (#quadbin_cell_edge_held), and each corner by exactly one
 * of the tiles meeting there (#quadbin_cell_vertex_held), the antimeridian,
 * where the columns wrap, and the poles included. A corner is named by its
 * column, modulo the columns of the zoom, and its row; an edge by its two
 * corners. Each edge a tile holds faces its lower index coordinates, its own
 * column or row, but the south edge of the last row, which extends to the
 * south pole (decision D5 of the cell walks); the corner a tile holds is its
 * own column and row, or on the south pole its south-west corner; and the
 * vertex k of the ring of
 * #quadbin_cell_to_geom is the corner the two functions read as k.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_quadbin.h>
#include "geo/geo_funcs.h"
#include "quadbin/quadbin.h"

#define MAXZOOM 8

int
main(int argc, char **argv)
{
  int maxzoom = argc > 1 ? atoi(argv[1]) : MAXZOOM;
  meos_initialize();
  int failures = 0;
  /* The corners of the ring of #quadbin_cell_to_geom: south-west, south-east,
   * north-east, north-west, as column and row offsets, rows counting
   * southward */
  const int dx[4] = {0, 1, 1, 0}, dy[4] = {1, 1, 0, 0};
  for (int zoom = 0; zoom <= maxzoom; zoom++)
  {
    int n = 1 << zoom;
    /* Owners of the corners (column, row), of the vertical edges west of
     * (column, row) and of the horizontal edges north of (column, row) */
    int *corner = calloc((size_t) n * (n + 1), sizeof(int));
    int *vedge = calloc((size_t) n * n, sizeof(int));
    int *hedge = calloc((size_t) n * (n + 1), sizeof(int));
    if (! corner || ! vedge || ! hedge)
    {
      printf("FAIL: out of memory\n");
      return EXIT_FAILURE;
    }
    for (int x = 0; x < n; x++)
      for (int y = 0; y < n; y++)
      {
        Quadbin cell = quadbin_tile_to_cell((uint32_t) x, (uint32_t) y,
          (uint32_t) zoom);
        /* The ring as #quadbin_cell_to_geom states it */
        double xmin, ymin, xmax, ymax;
        quadbin_cell_bounding_box(cell, &xmin, &ymin, &xmax, &ymax);
        GSERIALIZED *gs = quadbin_cell_to_geom(cell);
        LWGEOM *geom = lwgeom_from_gserialized(gs);
        const LWPOLY *poly = (const LWPOLY *) geom;
        for (int k = 0; k < 4; k++)
        {
          POINT2D p;
          getPoint2d_p(poly->rings[0], (uint32_t) k, &p);
          if (p.x != (dx[k] ? xmax : xmin) || p.y != (dy[k] ? ymin : ymax))
          {
            printf("FAIL zoom %d: vertex %d of (%d, %d) is not its corner "
              "%d\n", zoom, k, x, y, k);
            failures++;
          }
        }
        lwgeom_free(geom);
        free(gs);
        for (int k = 0; k < 4; k++)
        {
          int k1 = (k + 1) % 4;
          int cx = (x + dx[k]) % n, cy = y + dy[k];
          bool vheld = quadbin_cell_vertex_held(cell, k);
          corner[cx * (n + 1) + cy] += vheld;
          /* The corner a tile holds is its own column and row, or on the
           * south pole its south-west corner */
          if (vheld != ((dx[k] == 0 && dy[k] == 0) ||
              (dx[k] == 0 && dy[k] == 1 && y == n - 1)))
          {
            printf("FAIL zoom %d: vertex %d of (%d, %d) is %sheld\n", zoom,
              k, x, y, vheld ? "" : "not ");
            failures++;
          }
          bool eheld = quadbin_cell_edge_held(cell, k);
          if (dx[k] == dx[k1])
            /* A meridian: the west edge of the column x + dx[k] */
            vedge[((x + dx[k]) % n) * n + y] += eheld;
          else
            /* A parallel: the north edge of the row y + dy[k] */
            hedge[x * (n + 1) + y + dy[k]] += eheld;
          /* Facing the lower index coordinates: the tile's own column or
           * row, or the south pole */
          bool low = (dx[k] == 0 && dx[k1] == 0) ||
            (dy[k] == 0 && dy[k1] == 0) ||
            (dy[k] == 1 && dy[k1] == 1 && y == n - 1);
          if (eheld != low)
          {
            printf("FAIL zoom %d: edge %d of (%d, %d) is %sheld\n", zoom, k,
              x, y, eheld ? "" : "not ");
            failures++;
          }
        }
      }
    int bad_edges = 0, bad_corners = 0;
    for (int i = 0; i < n * n; i++)
      bad_edges += (vedge[i] != 1);
    for (int i = 0; i < n * (n + 1); i++)
    {
      bad_edges += (hedge[i] != 1);
      bad_corners += (corner[i] != 1);
    }
    printf("zoom %d: %d edges and %d corners, %d edges and %d corners without "
      "exactly one owner\n", zoom, n * n + n * (n + 1), n * (n + 1),
      bad_edges, bad_corners);
    failures += bad_edges + bad_corners;
    free(corner);
    free(vedge);
    free(hedge);
  }
  meos_finalize();
  printf("%d failures\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
