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
 * @brief Test of the vertices of an S2 cell of meos/src/s2cell/s2cell.c
 * @details A vertex is the corner of the cube, exact: its coordinates on the
 * face are the rationals of the quadratic transform at a dyadic index, and
 * its longitude and latitude are the doubles nearest the degrees of that
 * exact corner (#s2cell_cell_vertices, #vector_lon_nearest,
 * #vector_lat_nearest; decision D7 of the cell walks). The rows are cells whose vertices a judge in MPFR and
 * CGAL::Gmpq, sharing no code with MEOS, states, each one whose vertices read
 * as the cube coordinates rounded in doubles and libm's atan2 differ, or one
 * touching the antimeridian or a pole. A second pass reads every other
 * vertex again from its corner as a vector of integers doubled, which lies
 * beyond the 2^62 the bounded pairs of doubles read
 * (#ddb_vector_lon_nearest), so the enclosure in fixed point answers it
 * (#degree_angle_enclose), and it must give the same degrees.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include "s2cell/s2cell.h"
#include "geo/geo_funcs.h"

typedef struct
{
  unsigned face, level;
  unsigned long i, j;
  double v[8];            /* lon, lat of the four vertices */
} CornerRow;

static const CornerRow rows[] =
{
#include "s2_corner_rows.inc"
};

/* The numerator over 3 4^30 of the cube coordinate at the leaf index I, the
 * rational of the quadratic transform at s = I / 2^30 */
static int64
corner_num(uint64_t I)
{
  const int64 q = INT64CONST(1) << 60;
  int64 i = (int64) I;
  if (I >= (UINT64CONST(1) << 29))
    return 4 * i * i - q;
  int64 r = (INT64CONST(1) << 30) - i;
  return q - 4 * r * r;
}

/* The corner of a face at the leaf indices I and J times 3 4^30, doubled */
static void
corner_twice(unsigned face, uint64_t I, uint64_t J, int64 *v)
{
  const int64 n = 2 * (INT64CONST(3) << 60);
  int64 u = 2 * corner_num(I), w = 2 * corner_num(J);
  switch (face)
  {
    case 0: v[0] =  n; v[1] =  u; v[2] =  w; break;
    case 1: v[0] = -u; v[1] =  n; v[2] =  w; break;
    case 2: v[0] = -u; v[1] = -w; v[2] =  n; break;
    case 3: v[0] = -n; v[1] = -w; v[2] = -u; break;
    case 4: v[0] =  w; v[1] = -n; v[2] = -u; break;
    default: v[0] = w; v[1] =  u; v[2] = -n; break;
  }
}

int
main(void)
{
  meos_initialize();
  int failures = 0, nrows = (int) (sizeof(rows) / sizeof(rows[0]));
  for (int r = 0; r < nrows; r++)
  {
    const CornerRow *row = &rows[r];
    S2CellId cell = 0;
    s2cell_from_face_ij(row->face, (uint32_t) (row->i << (30 - row->level)),
      (uint32_t) (row->j << (30 - row->level)), row->level, &cell);
    double lon[4], lat[4];
    s2cell_cell_vertices(cell, lon, lat);
    for (int k = 0; k < 4; k++)
      if (lon[k] != row->v[2 * k] || lat[k] != row->v[2 * k + 1] ||
          signbit(lon[k]) != signbit(row->v[2 * k]) ||
          signbit(lat[k]) != signbit(row->v[2 * k + 1]))
      {
        printf("FAIL row %d vertex %d: got %a %a, want %a %a\n", r, k,
          lon[k], lat[k], row->v[2 * k], row->v[2 * k + 1]);
        failures++;
      }
  }
  /* The enclosure alone, on the corners doubled */
  int exact = 0;
  for (int r = 0; r < nrows; r++)
  {
    const CornerRow *row = &rows[r];
    int shift = 30 - (int) row->level;
    const uint64_t is[4] = { row->i, row->i + 1, row->i + 1, row->i };
    const uint64_t js[4] = { row->j, row->j, row->j + 1, row->j + 1 };
    for (int k = 0; k < 4; k++)
    {
      double lon = row->v[2 * k], lat = row->v[2 * k + 1];
      if (fabs(lon) == 180.0 || fabs(lat) == 90.0)
        continue;
      int64 v[3];
      corner_twice(row->face, is[k] << shift, js[k] << shift, v);
      double elon = vector_lon_nearest(v), elat = vector_lat_nearest(v);
      if (elon != lon || elat != lat)
      {
        printf("FAIL exact row %d vertex %d: got %a %a, want %a %a\n", r,
          k, elon, elat, lon, lat);
        failures++;
      }
      exact++;
    }
  }
  meos_finalize();
  printf("%d rows, %d vertices by the enclosure, %d failures\n", nrows,
    exact, failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
