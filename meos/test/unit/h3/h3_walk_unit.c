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
 * @brief Test of the planar walk of meos/src/h3/h3_geo.c
 * @details A planar segment is straight in longitude and latitude, and so is
 * each edge of the outline of a cell (#h3_cell_outline), so the walk
 * #h3_segment_cells leaves each cell where the signs of the outline say it
 * crosses an edge or passes a vertex, the crossings ordered by
 * #segment_edge_cross_cmp, every sign exact on the input doubles. The rows
 * are trips whose cells, those each segment holds for a positive length, an
 * exact oracle in rational arithmetic sharing no code with MEOS states, each
 * a trip the base walked otherwise: on the curved boundaries of the library
 * it misses the cell a segment cuts across a corner of the outline, passes
 * a vertex into the wrong cell, or holds the cell along whose edge the
 * segment runs where the rule gives the edge to the cell across. A visit at
 * one parameter, a pass through a vertex, holds no length: the walk states
 * the crossing it enters each cell by, and the visit whose crossing and
 * next crossing lie at one parameter is left out.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <h3api.h>

#include <meos.h>
#include "geo/geo_funcs.h"
#include "h3/th3index_internal.h"

/* The most cells a segment of a row crosses */
#define MAXCELLS 4096

typedef struct
{
  int res;              /* Resolution */
  int n;                /* Number of positions */
  double x[4], y[4];    /* Positions */
  int ncells;           /* Number of cells held for a positive length */
  H3Index cells[8];     /* Those cells, in order */
} WalkRow;

static const WalkRow rows[] =
{
#include "h3_walk_rows.inc"
};

static H3Index cells[MAXCELLS];
static double enter[MAXCELLS];
static H3EdgeCross crossings[MAXCELLS];

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = (int) (sizeof(rows) / sizeof(rows[0]));
  for (int i = 0; i < nrows; i++)
  {
    const WalkRow *r = &rows[i];
    H3Index got[64];
    int ngot = 0;
    for (int s = 0; s + 1 < r->n; s++)
    {
      double x0 = r->x[s], y0 = r->y[s], x1 = r->x[s + 1], y1 = r->y[s + 1];
      int count = h3_segment_cells(x0, y0, x1, y1, false, r->res, cells,
        enter, crossings, MAXCELLS);
      for (int k = 0; k < count; k++)
      {
        /* A visit at one parameter holds no length */
        bool zero;
        if (k + 1 >= count)
          zero = false;
        else if (k == 0)
        {
          const H3EdgeCross *c = &crossings[1];
          zero = cross_product_sign_offset(c->ax, c->ay, c->bx, c->by, c->ax,
            c->ay, x0, y0, c->ob - c->oa, - c->oa) == 0;
        }
        else
        {
          const H3EdgeCross *c = &crossings[k], *d = &crossings[k + 1];
          zero = segment_edge_cross_cmp(x0, y0, x1, y1, c->ax, c->ay, c->oa,
            c->bx, c->by, c->ob, d->ax, d->ay, d->oa, d->bx, d->by,
            d->ob) == 0;
        }
        if (zero || (ngot > 0 && got[ngot - 1] == cells[k]))
          continue;
        if (ngot < 64)
          got[ngot++] = cells[k];
      }
    }
    bool same = ngot == r->ncells;
    for (int k = 0; same && k < ngot; k++)
      same = got[k] == r->cells[k];
    if (! same)
    {
      printf("FAIL row %d at resolution %d: %d cells, not %d:", i, r->res,
        ngot, r->ncells);
      for (int k = 0; k < ngot; k++)
        printf(" %llx", (unsigned long long) got[k]);
      printf("\n");
      failures++;
    }
  }
  meos_finalize();
  printf("%d rows, %d failures\n", nrows, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
