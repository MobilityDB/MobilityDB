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
 * @brief Test of the incircle sign over the whole range of the doubles,
 * #incircle_sign
 * @details The circle through A = (1, 0), B = (0, 1) and C = (-1, 0), taken
 * counterclockwise, is the unit circle, so the sign is 1 for D = (0, 1/2)
 * inside it, -1 for D = (0, 2) outside, 0 for D = (0, -1) on it, 1 for the
 * double above -1 and -1 for the double below -1, one unit in the last place
 * inside and outside. A second circle through (3, 1), (1, 3) and (-1, 1), of
 * centre (1, 1) and radius 2, holds (1, -1) on it. The determinant is
 * homogeneous of degree 4 in the coordinate differences, so scaling the four
 * points by 2^e leaves its sign unchanged; every scale from 2^-1020 to 2^1020
 * keeps each coordinate an exact double, and the rows of a scale whose
 * products of four differences overflow or underflow the doubles answer as
 * those of 2^0.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>
#include "geo/geo_funcs.h"

static int failures = 0;
static int checks = 0;

/* The rows: the four points and the exact sign */
typedef struct
{
  double p[8];
  int sign;
  const char *what;
} IncircleRow;

static void
check_row(const IncircleRow *r, int e)
{
  double q[8];
  for (int i = 0; i < 8; i++)
    q[i] = ldexp(r->p[i], e);
  int s = incircle_sign(q[0], q[1], q[2], q[3], q[4], q[5], q[6], q[7]);
  checks++;
  if (s != r->sign)
  {
    failures++;
    if (failures <= 20)
      printf("FAIL %s at 2^%d: %d, expected %d\n", r->what, e, s, r->sign);
  }
  return;
}

int
main(void)
{
  meos_initialize();
  const IncircleRow rows[] = {
    {{1, 0, 0, 1, -1, 0, 0, 0.5}, 1, "unit circle, D inside"},
    {{1, 0, 0, 1, -1, 0, 0, 2}, -1, "unit circle, D outside"},
    {{1, 0, 0, 1, -1, 0, 0, -1}, 0, "unit circle, D on it"},
    {{1, 0, 0, 1, -1, 0, 0, -0x1.fffffffffffffp-1}, 1,
      "unit circle, D one ulp inside"},
    {{1, 0, 0, 1, -1, 0, 0, -0x1.0000000000001p+0}, -1,
      "unit circle, D one ulp outside"},
    {{3, 1, 1, 3, -1, 1, 1, -1}, 0, "circle of radius 2, D on it"},
    {{3, 1, 1, 3, -1, 1, 1, 0}, 1, "circle of radius 2, D inside"},
    {{3, 1, 1, 3, -1, 1, 4, 4}, -1, "circle of radius 2, D outside"},
  };
  int nrows = (int) (sizeof(rows) / sizeof(rows[0]));
  for (int e = -1020; e <= 1020; e += 4)
    for (int i = 0; i < nrows; i++)
      check_row(&rows[i], e);
  printf("%d of %d checks passed\n", checks - failures, checks);
  meos_finalize();
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
