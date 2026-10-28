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
 * @brief The distance between two circular buffers, judged by the closed
 * form
 *
 * @details The distance between two discs is `max(0, D - r1 - r2)`, `D` the
 * distance between their centres, and the answer is the double nearest it.
 * The sum of the radii is no double in general, so each case below takes the
 * centres 5 apart, where `D` is a whole number, and radii whose exact sum lies
 * a few units in the last place below 5: the expected value is then
 * `(5 - r2) - r1`, each subtraction exact by Sterbenz's lemma since its
 * operands lie within a factor of two of each other, which the program checks
 * before it reads the answer. Subtracting the radii one at a time from 5
 * rounds the first difference, so the answer is off by units in its last
 * place, and in the first case it is 0 for two discs that do not meet.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>

static Cbuffer *
disc(double x, double y, double r)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "Cbuffer(Point(%.17g %.17g),%.17g)", x, y, r);
  return cbuffer_in(buf);
}

/* The distance between the discs of centres (3 4) and the origin, of radii
 * r1 and r2, asserted to be (5 - r2) - r1, which each case below makes exact */
static void
check(double r1, double r2, double expected)
{
  assert(r2 >= 2.5 && r2 <= 10.0 && (5.0 - r2) >= 0.5 * r1 &&
    (5.0 - r2) <= 2.0 * r1);
  assert((5.0 - r2) - r1 == expected);
  Cbuffer *cb1 = disc(3.0, 4.0, r1), *cb2 = disc(0.0, 0.0, r2);
  assert(cb1 != NULL && cb2 != NULL);
  assert(distance_cbuffer_cbuffer(cb1, cb2) == expected);
  /* Apart by a positive distance is disjoint */
  assert(disjoint_cbuffer_cbuffer(cb1, cb2) == (expected > 0.0 ? 1 : 0));
  free(cb1); free(cb2);
}

int
main(void)
{
  meos_initialize();

  /* The radii sum to 5 - 2^-52: the discs stand 2^-52 apart */
  check(0x1.8f01be9b021bfp+0, 0x1.b87f20b27ef20p+1, 0x1p-52);
  /* The radii sum to 5 - 7 * 2^-53 */
  check(0x1.9bfe163841f01p-1, 0x1.0c803d38f7c1fp+2, 0x1.cp-51);
  /* Radii summing exactly to the distance: the discs touch */
  check(1.5, 3.5, 0.0);

  printf("cbuffer_distance_test: every distance answered as its closed "
    "form\n");
  meos_finalize();
  return 0;
}
