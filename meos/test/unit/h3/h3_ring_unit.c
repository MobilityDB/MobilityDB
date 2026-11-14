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
 * @brief Test of the microsecond at which a segment crosses the line of an
 * edge of an H3 cell of meos/src/h3/h3_geo.c
 * @details Each row of h3_ring_rows.inc states the microsecond, truncated, at
 * which a segment crosses the line of an edge of a cell's outline, and
 * whether it lies on the line exactly there, as CGAL::Gmpq computes them on
 * the outline the H3 library gives the cell. #dggs_outline_edge_offset must
 * answer both. On the rows of kind 1 the parameter computed in doubles,
 * `floor(D c0 / (c0 - c1))` on the two cross products, dates another
 * microsecond; on the rows of kind 2, across the antimeridian, reading the
 * edge without its offset of 360 does. The test checks that each of them
 * does, so that the rows keep witnessing what they witness.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <h3api.h>

#include <meos.h>
#include "geo/geo_funcs.h"
#include "h3/th3index_internal.h"

typedef struct
{
  H3Index cell;
  int k;
  double x0, y0, x1, y1;
  int64 D, n;
  int whole, kind;
} RingRow;

static const RingRow rows[] =
{
#include "h3_ring_rows.inc"
};

/* The longitude shifted by 360 nearest to a reference, added in doubles */
static double
near_lon(double ref, double lon)
{
  if (lon - ref > 180.0)
    return lon - 360.0;
  if (ref - lon > 180.0)
    return lon + 360.0;
  return lon;
}

/* The microsecond dated in doubles, and whether it reads as exact */
static int64
offset_in_doubles(const DggsOutline *o, const RingRow *r, int *whole)
{
  int k1 = (r->k + 1) % o->count;
  double ax = near_lon(r->x0, o->lon[r->k]), ay = o->lat[r->k];
  double bx = near_lon(ax, o->lon[k1]), by = o->lat[k1];
  double c0 = (bx - ax) * (r->y0 - ay) - (by - ay) * (r->x0 - ax);
  double c1 = (bx - ax) * (r->y1 - ay) - (by - ay) * (r->x1 - ax);
  if (c0 == 0.0)
  {
    *whole = 1;
    return 0;
  }
  if ((c0 > 0.0) == (c1 > 0.0) || c1 == 0.0)
  {
    *whole = (c1 == 0.0);
    return r->D;
  }
  double t = floor((double) r->D * (c0 / (c0 - c1)));
  *whole = ((double) r->D * (c0 / (c0 - c1)) == t);
  return llround(t);
}

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = sizeof(rows) / sizeof(rows[0]);
  for (int i = 0; i < nrows; i++)
  {
    const RingRow *r = &rows[i];
    DggsOutline o;
    if (! h3_cell_outline(r->cell, &o))
    {
      printf("FAIL row %d: no outline\n", i);
      failures++;
      continue;
    }
    bool whole;
    int64 n = dggs_outline_edge_offset(&o, r->k, r->x0, r->y0, r->x1, r->y1,
      r->D, &whole);
    if (n != r->n || whole != (r->whole != 0))
    {
      printf("FAIL row %d: " INT64_FORMAT " %d, expected " INT64_FORMAT
        " %d\n", i, n, whole, r->n, r->whole);
      failures++;
    }
    if (r->kind == 1)
    {
      int w;
      int64 m = offset_in_doubles(&o, r, &w);
      if (m == r->n && w == r->whole)
      {
        printf("FAIL row %d: the doubles answer the exact microsecond\n", i);
        failures++;
      }
    }
    else if (r->kind == 2)
    {
      int k1 = (r->k + 1) % o.count;
      bool w;
      int64 m = segment_edge_cross_offset(r->x0, r->y0, r->x1, r->y1, r->D,
        o.lon[r->k], o.lat[r->k], 0.0, o.lon[k1], o.lat[k1], 0.0, &w);
      if (m == r->n && w == (r->whole != 0))
      {
        printf("FAIL row %d: the edge without its offset answers the same\n",
          i);
        failures++;
      }
    }
  }
  meos_finalize();
  printf("%d rows, %d failures\n", nrows, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
