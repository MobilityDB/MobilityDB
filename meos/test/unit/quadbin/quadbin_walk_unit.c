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
 * @brief Test of the order of the crossings of the planar walk of
 * meos/src/quadbin/quadbin.c
 * @details A planar segment crossing a column bound and a row bound near the
 * corner where they meet reaches one of them first, and passes through the
 * tile beside the corner on that side, or passes through the corner itself.
 * #quadbin_segment_cells decides it by the side of the segment the corner
 * lies on, a sign of a cross product on the input doubles. The rows are
 * segments through the corners of the tiles of the zooms 2 to 8 and one ulp
 * of an end beside them, each the walk the base answered wrongly, with the
 * tiles an exact oracle in rational arithmetic, sharing no code with MEOS,
 * states: the parameters of the two crossings round to the same double or to
 * a reversed order, and the base passed through the corner where the segment
 * passes beside it, or the reverse. The witness: at zoom 2, the segment from
 * (45, 33.25...) to (-45 - 2^-47, -33.25...) reaches the meridian 0 before
 * the equator, and walks through (1, 1), not through (2, 2).
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_quadbin.h>
#include "quadbin/quadbin.h"

/* The longest walk of a row */
#define MAXCELLS 8

typedef struct
{
  uint32_t zoom;
  double x0, y0, x1, y1;
  int count;
  uint32_t tiles[MAXCELLS][2];
} WalkRow;

static const WalkRow rows[] =
{
#include "quadbin_walk_rows.inc"
};

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = (int) (sizeof(rows) / sizeof(rows[0]));
  for (int i = 0; i < nrows; i++)
  {
    const WalkRow *r = &rows[i];
    Quadbin cells[MAXCELLS + 1];
    double enter[MAXCELLS + 1];
    int count = quadbin_segment_cells(r->x0, r->y0, r->x1, r->y1, false,
      r->zoom, cells, enter, MAXCELLS + 1);
    bool ok = (count == r->count);
    for (int k = 0; ok && k < count; k++)
      ok = cells[k] == quadbin_tile_to_cell(r->tiles[k][0], r->tiles[k][1],
        r->zoom);
    if (! ok)
    {
      printf("FAIL zoom %u: the walk from (%a, %a) to (%a, %a) is not the "
        "exact one\n", r->zoom, r->x0, r->y0, r->x1, r->y1);
      failures++;
    }
  }
  meos_finalize();
  printf("%d of %d walks wrong\n", failures, nrows);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
