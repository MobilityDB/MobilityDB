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
 * @brief The relationships between two circular buffers, judged by the closed
 * form on inputs whose answer is whole-number arithmetic
 *
 * @details A disc is bounded by a circular arc, and the witness for an arc is
 * the closed form. Every relationship between two discs is the sign of
 * `dx * dx + dy * dy - (sum of lengths) * (sum of lengths)`, so on integer
 * centres and radii the answer is settled in whole numbers and a reader checks
 * it without floating point and without an oracle: the discs at (0, 0) with
 * radius 2 and at (3, 4) with radius 3 touch because 9 + 16 = 25 = 5 * 5.
 *
 * The grids below ask every integer centre offset within a square of side 13
 * against several radius pairs, and each expected answer is that whole-number
 * comparison.
 *
 * ONE PAIR IS NOT INTEGRAL AND IS THE POINT OF THE EXERCISE: the discs at
 * (0, 0) with the radius nearest the square root of two and at (1, 1) with
 * radius zero. Their centres stand `dx * dx + dy * dy = 2` apart while the sum
 * of the radii squares to 2.0000000000000004, so the discs OVERLAP and do not
 * touch. A form that compares a rounded square root against the sum answers
 * that they touch, because the root it computes is that same double.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_exact_test cbuffer_exact_test.c \
 *   -L/usr/local/lib -lmeos -lm
 * ./cbuffer_exact_test
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>

/**
 * @brief The circular buffer at the integer centre with the integer radius
 */
static Cbuffer *
disc(int x, int y, int r)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "Cbuffer(Point(%d %d),%d)", x, y, r);
  return cbuffer_in(buffer);
}

int
main(void)
{
  meos_initialize();

  int cases = 0, tangent = 0, nested = 0;

  /* Every integer centre offset within a square of side 13, against radius
   * pairs that put the whole-number comparison on both sides of every
   * relationship */
  static const int radii[][2] = {{0, 0}, {1, 0}, {1, 1}, {2, 3}, {5, 2},
    {3, 3}, {4, 1}};
  for (int dx = -6; dx <= 6; dx++)
    for (int dy = -6; dy <= 6; dy++)
      for (size_t k = 0; k < sizeof(radii) / sizeof(radii[0]); k++)
      {
        int r1 = radii[k][0], r2 = radii[k][1];
        Cbuffer *cb1 = disc(0, 0, r1);
        Cbuffer *cb2 = disc(dx, dy, r2);
        assert(cb1 != NULL && cb2 != NULL);

        /* The three whole-number quantities every relationship reads */
        int gap = dx * dx + dy * dy;
        int sum = (r1 + r2) * (r1 + r2);
        int dif = (r1 - r2) * (r1 - r2);

        /* Touching is the centres standing exactly the sum of the radii apart */
        assert(touches_cbuffer_cbuffer(cb1, cb2) == (gap == sum ? 1 : 0));
        if (gap == sum) tangent++;

        /* Covering is the second disc lying inside or on the first, which asks
         * the centre distance against the difference of the radii, and only
         * where the first is the wider */
        bool covers = (r1 >= r2) && (gap <= dif);
        assert(covers_cbuffer_cbuffer(cb1, cb2) == (covers ? 1 : 0));
        if (covers) nested++;

        /* Intersecting is the centres standing no farther than the sum */
        assert(intersects_cbuffer_cbuffer(cb1, cb2) == (gap <= sum ? 1 : 0));
        assert(disjoint_cbuffer_cbuffer(cb1, cb2) == (gap <= sum ? 0 : 1));

        /* Within a distance adds that distance to the sum of the radii */
        for (int d = 0; d <= 3; d++)
          assert(dwithin_cbuffer_cbuffer(cb1, cb2, (double) d) ==
            (gap <= (r1 + r2 + d) * (r1 + r2 + d) ? 1 : 0));

        free(cb1);
        free(cb2);
        cases++;
      }

  /* The pair the whole numbers cannot carry: the sum of the radii squares to
   * more than the centre distance squared, so the discs overlap */
  Cbuffer *a = cbuffer_in("Cbuffer(Point(0 0),1.4142135623730951)");
  Cbuffer *b = cbuffer_in("Cbuffer(Point(1 1),0)");
  assert(a != NULL && b != NULL);
  assert(1.4142135623730951 * 1.4142135623730951 > 2.0);
  assert(touches_cbuffer_cbuffer(a, b) == 0);
  assert(intersects_cbuffer_cbuffer(a, b) == 1);
  assert(disjoint_cbuffer_cbuffer(a, b) == 0);
  free(a);
  free(b);

  printf("%d whole-number cases, %d of them tangent and %d nested, and the "
    "rounded-root pair answers overlap\n", cases, tangent, nested);

  meos_finalize();
  return 0;
}
