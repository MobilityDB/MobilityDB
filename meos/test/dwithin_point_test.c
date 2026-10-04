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
 * @brief A program that tests that two temporal points answer the distance
 * they are within exactly
 * @details Whether two points are within a distance is a question about four
 * coordinates and that distance, each an exact rational, so it has one answer,
 * and #point_within_distance_sign gives it without constructing the distance
 * between them. A square root is a rounded value, and a rounded value that
 * decides an answer moves it for every pair lying within its own rounding of
 * the distance asked about.
 *
 * The first part asks only questions whose answer follows from the integers:
 * a 3-4-5 triangle scaled by a power of two has every coordinate and every
 * side exactly representable, and the squares satisfy 9 + 16 = 25 exactly, so
 * a pair five units apart IS within five, is NOT within the double below five,
 * and IS within the double above it. Sixty-one scales exercise the whole
 * exponent range those products stay exact over.
 *
 * The second part carries four pairs whose answer a double square root gets
 * WRONG, and the expected answers are CGAL's: its
 * `Exact_predicates_exact_constructions_kernel` reads the same doubles and
 * compares the squared distance against the square of the distance as exact
 * rationals. For each pair the distance asked about is the rounded square root
 * of its own squared distance, which rounds BELOW it, so the points are NOT
 * within that distance although `d >= sqrt(h*h + v*v)` reads true; and they ARE
 * within the `hypot` of the same coordinates, which rounds above. A kernel
 * that answers either of those by a square root fails here.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o dwithin_point_test dwithin_point_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/**
 * @brief Return a temporal point that stays where it is over the period every
 * case shares, so the distance of a pair is the distance of two points
 */
static Temporal *
still(double x, double y)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
    "[POINT(%.17g %.17g)@2001-01-01, POINT(%.17g %.17g)@2001-01-02]",
    x, y, x, y);
  return tgeompoint_in(buffer);
}

/**
 * @brief Assert that both quantifiers answer a pair of resting points as
 * expected, the two agreeing because the distance never changes
 */
static void
both(double px, double py, double qx, double qy, double d, int expected)
{
  Temporal *p = still(px, py), *q = still(qx, qy);
  assert(p != NULL && q != NULL);
  assert(edwithin_tgeo_tgeo(p, q, d) == expected);
  assert(adwithin_tgeo_tgeo(p, q, d) == expected);
  free(p);
  free(q);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* The 3-4-5 triangle at every scale its products stay exact over */
  int asked = 0;
  for (int exponent = -30; exponent <= 30; exponent++)
  {
    double s = ldexp(1.0, exponent);
    /* Exactly five units apart: within five, and within everything above it */
    both(0, 0, 3 * s, 4 * s, 5 * s, 1);
    both(0, 0, 3 * s, 4 * s, nextafter(5 * s, INFINITY), 1);
    both(0, 0, 3 * s, 4 * s, 10 * s, 1);
    /* And within nothing below it */
    both(0, 0, 3 * s, 4 * s, nextafter(5 * s, 0.0), 0);
    both(0, 0, 3 * s, 4 * s, 4 * s, 0);
    /* Two points in the same place are within no distance at all */
    both(3 * s, 4 * s, 3 * s, 4 * s, 0.0, 1);
    asked += 6;
  }

  /* The four pairs a double square root answers wrongly, judged by CGAL's
   * exact kernel. Each is asked twice: about the square root of its own
   * squared distance, which rounds below it, and about the `hypot` of the same
   * coordinates, which rounds above */
  static const double wrong[4][2] = {
    { 965086.86429126514,  781057.10902300524},
    { 577660.34620705084,   18646.751073490232},
    {-433916.44369527529,  950434.27122311399},
    { 877355.44791321992, -905655.76958733413}};
  for (int i = 0; i < 4; i++)
  {
    double x = wrong[i][0], y = wrong[i][1];
    double rounded_below = sqrt(x * x + y * y);
    double rounded_above = hypot(x, y);
    /* The two forms really do differ on these coordinates */
    assert(rounded_above > rounded_below);
    /* CGAL: not within the lower rounding, within the upper one */
    both(0, 0, x, y, rounded_below, 0);
    both(0, 0, x, y, rounded_above, 1);
    asked += 2;
  }

  printf("%d distances answered exactly, 61 scales and 4 pairs a double "
    "square root gets wrong\n", asked);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
