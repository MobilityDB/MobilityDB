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
 * @brief Test of the points MEOS states for the boundary and the center of an
 * H3 cell, meos/src/h3/h3_geo.c
 * @details Each row of h3_vertex_rows.inc states the points of the boundary
 * of a cell in the order of `cellToBoundary` and its center, each the double
 * nearest the degrees of the value the formula of H3 4.6.0 gives it, every
 * constant read on its decimal literal (decision D8 of the cell walks), as an
 * oracle in exact arithmetic computes them. #h3_cell_boundary_lonlat and
 * #h3_cell_center_lonlat must answer the same doubles, on every machine:
 * they compute no trigonometry of the C library, whose doubles differ from
 * one machine to another.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <h3api.h>
#include <meos.h>
#include "h3/th3index_internal.h"

typedef struct
{
  H3Index cell;
  int n;
  double lon[10], lat[10];
  double clon, clat;
} VertexRow;

static const VertexRow rows[] =
{
#include "h3_vertex_rows.inc"
};

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = sizeof(rows) / sizeof(rows[0]);
  for (int i = 0; i < nrows; i++)
  {
    const VertexRow *r = &rows[i];
    double lon[H3_BOUNDARY_MAX], lat[H3_BOUNDARY_MAX], clon, clat;
    int n = h3_cell_boundary_lonlat(r->cell, lon, lat);
    if (n != r->n)
    {
      printf("FAIL row %d (%llx): %d points, expected %d\n", i,
        (unsigned long long) r->cell, n, r->n);
      failures++;
      continue;
    }
    for (int k = 0; k < n; k++)
      if (lon[k] != r->lon[k] || lat[k] != r->lat[k])
      {
        printf("FAIL row %d (%llx) point %d: %a %a, expected %a %a\n", i,
          (unsigned long long) r->cell, k, lon[k], lat[k], r->lon[k],
          r->lat[k]);
        failures++;
      }
    if (! h3_cell_center_lonlat(r->cell, &clon, &clat) ||
        clon != r->clon || clat != r->clat)
    {
      printf("FAIL row %d (%llx) center: %a %a, expected %a %a\n", i,
        (unsigned long long) r->cell, clon, clat, r->clon, r->clat);
      failures++;
    }
  }
  /* Every vertex the library names for a cell (`cellToVertexes`), stated by
   * decision D8 (#h3_vertex_point_lonlat), is a point of the outline MEOS
   * reads for the cell from its own vertex indices (#h3_cell_outline), on
   * every cell of the resolutions 0 to 3 */
  H3Index res0[122];
  getRes0Cells(res0);
  long ncells = 0;
  for (int res = 0; res <= 3; res++)
    for (int b = 0; b < 122; b++)
    {
      int64_t sz;
      cellToChildrenSize(res0[b], res, &sz);
      H3Index *cells = calloc((size_t) sz, sizeof(H3Index));
      if (! cells)
      {
        printf("FAIL no memory for the cells of resolution %d\n", res);
        failures++;
        continue;
      }
      cellToChildren(res0[b], res, cells);
      for (int64_t c = 0; c < sz; c++)
      {
        H3Index verts[6];
        DggsOutline o;
        if (! cells[c])
          continue;
        ncells++;
        if (cellToVertexes(cells[c], verts) != E_SUCCESS ||
            ! h3_cell_outline(cells[c], &o))
        {
          printf("FAIL cell %llx: no vertices or outline\n",
            (unsigned long long) cells[c]);
          failures++;
          continue;
        }
        /* Every cell across an edge of the outline is a neighbour */
        for (int j = 0; j < o.count; j++)
        {
          int adjacent = 0;
          if (areNeighborCells(cells[c], o.across[j], &adjacent) !=
              E_SUCCESS || ! adjacent)
          {
            printf("FAIL cell %llx edge %d: %llx is not a neighbour\n",
              (unsigned long long) cells[c], j,
              (unsigned long long) o.across[j]);
            failures++;
          }
        }
        for (int k = 0; k < 6; k++)
        {
          double vlon, vlat;
          bool held = false;
          if (! verts[k])
            continue;
          if (! h3_vertex_point_lonlat(verts[k], &vlon, &vlat))
          {
            failures++;
            continue;
          }
          for (int j = 0; j < o.count && ! held; j++)
            held = o.lon[j] == vlon && o.lat[j] == vlat;
          if (! held)
          {
            printf("FAIL cell %llx vertex %llx: %a %a not in its outline\n",
              (unsigned long long) cells[c], (unsigned long long) verts[k],
              vlon, vlat);
            failures++;
          }
        }
      }
      free(cells);
    }
  meos_finalize();
  printf("%d rows, %ld cells, %d failures\n", nrows, ncells, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
