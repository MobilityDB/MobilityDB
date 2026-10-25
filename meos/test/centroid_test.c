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
 * @brief The centroid of a geometry, judged by its closed form
 *
 * @details The centroid is the mean of the highest-dimensional pieces of the
 * geometry weighted by their measure, and which dimension that is, is decided
 * exactly: a surface holds an area wherever a ring's shoelace sum is not zero,
 * however small that area is. The cases:
 * - a triangle of base 10 and height h holds an area for every h > 0, so its
 *   centroid is (5, h / 3), at heights far below any band on the area;
 * - a ring whose points lie on one line holds no area, so its centroid is
 *   that of its outline;
 * - a hole wound the same way as its shell is subtracted, being the second
 *   ring, never added for its winding: (16 * 2 - 1 * 1.5) / 15;
 * - a square, whose centroid is its centre.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o centroid_test centroid_test.c \
 *   -L/usr/local/lib -lmeos -lm
 * ./centroid_test
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Whether the centroid of the geometry stands within tol of (x, y) */
static bool
centroid_at(const char *wkt, double x, double y, double tol)
{
  GSERIALIZED *gs = geom_in(wkt, -1);
  assert(gs != NULL);
  GSERIALIZED *c = geom_centroid(gs);
  assert(c != NULL);
  char *text = geo_as_text(c, 17);
  double cx, cy;
  bool read = sscanf(text, "POINT(%lf %lf)", &cx, &cy) == 2;
  bool result = read && fabs(cx - x) <= tol && fabs(cy - y) <= tol;
  if (! result)
    printf("centroid of %s is %s, expected POINT(%.17g %.17g)\n", wkt, text,
      x, y);
  free(text); free(c); free(gs);
  return result;
}

int
main(void)
{
  meos_initialize();

  /* Slivers: the area centroid at every height, far below 1e-6 */
  assert(centroid_at("POLYGON((0 0,10 0,5 0.001,0 0))", 5.0, 0.001 / 3,
    1e-15));
  assert(centroid_at("POLYGON((0 0,10 0,5 1e-07,0 0))", 5.0, 1e-07 / 3,
    1e-15));
  assert(centroid_at("POLYGON((0 0,10 0,5 1e-09,0 0))", 5.0, 1e-09 / 3,
    1e-15));
  assert(centroid_at("POLYGON((1000 2000,1010 2000,1005 2000.0000001,"
    "1000 2000))", 1005.0, 2000.0 + 1e-07 / 3, 1e-10));

  /* A ring on one line holds no area: the centroid of its outline, the
   * middle of the segment traced back and forth */
  assert(centroid_at("POLYGON((0 0,2 3,4 6,2 3,0 0))", 2.0, 3.0, 1e-12));

  /* A hole wound the same way as its shell is subtracted */
  assert(centroid_at("POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 2,2 2,2 1,1 1))",
    (16.0 * 2 - 1.0 * 1.5) / 15, (16.0 * 2 - 1.0 * 1.5) / 15, 1e-12));

  /* A square: its centre */
  assert(centroid_at("POLYGON((0 0,4 0,4 4,0 4,0 0))", 2.0, 2.0, 1e-15));

  printf("centroid_test: every centroid answered as its closed form\n");
  meos_finalize();
  return 0;
}
