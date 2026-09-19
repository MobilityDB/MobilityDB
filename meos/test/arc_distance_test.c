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
 * @brief A program that tests the distance of a temporal point to a geometry
 * bounded by circular arcs against its closed form
 * @details A temporal point standing still at one place is at the distance of
 * that place to the geometry, which a circle states in closed form: the
 * distance to the circle's centre less its radius. The program asks the
 * nearest approach distance of such a point to a full circle, which a circular
 * string returning to its start draws, to half of it and to its diameter, and
 * to an arc of radius one at the coordinates of a projected reference system,
 * where a circumcentre read from the squares of the coordinates carries an
 * error of the order of 1e-3, and to an arc far shorter than its radius, whose
 * distance read through the centre loses the digits the radius takes.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o arc_distance_test arc_distance_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Tolerance of the comparison with the closed form */
#define TOLERANCE 1e-9

static int failures = 0;

/*
 * Compare the nearest approach distance of a temporal point standing still at
 * a place with the closed form
 */
static void
check(const char *place, const char *wkt, double expected)
{
  char tpoint[128];
  snprintf(tpoint, sizeof(tpoint),
    "[%s@2001-01-01, %s@2001-01-02]", place, place);
  Temporal *temp = tgeompoint_in(tpoint);
  GSERIALIZED *gs = geom_in(wkt, -1);
  double d = nad_tgeo_geo(temp, gs);
  bool ok = fabs(d - expected) <= TOLERANCE;
  printf("  %-26s %-58s %.12g %s\n", place, wkt, d, ok ? "OK" : "FAIL");
  if (! ok)
  {
    printf("    closed form %.12g\n", expected);
    failures++;
  }
  free(temp); free(gs);
}

/*
 * Compare the nearest approach distance of a temporal point standing still at
 * a place with the exact distance, relative to it
 */
static void
check_rel(const char *place, const char *wkt, double expected)
{
  char tpoint[256];
  snprintf(tpoint, sizeof(tpoint),
    "[%s@2001-01-01, %s@2001-01-02]", place, place);
  Temporal *temp = tgeompoint_in(tpoint);
  GSERIALIZED *gs = geom_in(wkt, -1);
  double d = nad_tgeo_geo(temp, gs);
  bool ok = fabs(d - expected) <= 1e-9 * expected;
  printf("  %.17g %s\n", d, ok ? "OK" : "FAIL");
  if (! ok)
  {
    printf("    exact %.17g\n", expected);
    failures++;
  }
  free(temp); free(gs);
}

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* The circle centred at (1 0) with radius 1 passes through (1 1) */
  printf("Circle centred at (1 0) with radius 1:\n");
  check("Point(1 1)", "CIRCULARSTRING(0 0,2 0,0 0)", 0.0);
  check("Point(1 -1)", "CIRCULARSTRING(0 0,2 0,0 0)", 0.0);
  check("Point(1 0)", "CIRCULARSTRING(0 0,2 0,0 0)", 1.0);
  check("Point(1 3)", "CIRCULARSTRING(0 0,2 0,0 0)", 2.0);
  check("Point(1 1)", "CIRCULARSTRING(0 0,1 1,2 0)", 0.0);
  check("Point(1 -1)", "CIRCULARSTRING(0 0,1 1,2 0)", sqrt(2.0));
  check("Point(1 1)", "LINESTRING(0 0,2 0)", 1.0);

  /* The arc of radius 1 centred at (6100001 0) */
  printf("Arc of radius 1 centred at (6100001 0):\n");
  check("Point(6100001 1)",
    "CIRCULARSTRING(6100000 0,6100001 1,6100002 0)", 0.0);
  check("Point(6100001 0)",
    "CIRCULARSTRING(6100000 0,6100001 1,6100002 0)", 1.0);
  check("Point(6100001 4)",
    "CIRCULARSTRING(6100000 0,6100001 1,6100002 0)", 3.0);

  /* The arc through (-200000 9999999999), (0 10000000001) and
   * (200000 9999999999), on the circle of radius 10000000001 centred at the
   * origin: the Pythagorean triple (9999999999, 200000, 10000000001) puts the
   * three points exactly on it, and the arc is 25000 times shorter than its
   * radius */
  printf("Arc of radius 10000000001 centred at (0 0):\n");
  check("Point(0 10000000002)", "CIRCULARSTRING(-200000 9999999999,"
    "0 10000000001,200000 9999999999)", 1.0);
  check("Point(0 10000000000)", "CIRCULARSTRING(-200000 9999999999,"
    "0 10000000001,200000 9999999999)", 1.0);
  check("Point(0 10000000001.25)", "CIRCULARSTRING(-200000 9999999999,"
    "0 10000000001,200000 9999999999)", 0.25);

  /* Two nearly straight arcs, far shorter than the circle they lie on, and
   * the exact distance to each, computed in rational arithmetic: a point near
   * the first, and a point beyond the end of the second */
  printf("Nearly straight arcs:\n");
  check_rel("Point(-2.31273961515321e-08 -2.1153402775748315e-08)",
    "CIRCULARSTRING(-3.22566372349572e-09 -4.2240170882638686e-08,"
    "-2.3127496518755652e-08 -2.1153497503430836e-08,"
    "-5.8524987287719254e-08 1.63513561433461e-08)", 1.3801055539164903e-13);
  check_rel("Point(-1.7119576591494597e-09 3.305872965375566e-09)",
    "CIRCULARSTRING(1.5228162733504057e-10 -1.5379050301617932e-10,"
    "3.6466540003486635e-11 -1.5908996852759037e-10,"
    "-2.403045297136147e-10 -1.7175445613722083e-10)", 3.7761958921957409e-09);

  if (failures == 0)
    printf("Arc distance test: all tests passed\n");
  else
    printf("Arc distance test: %d test(s) FAILED\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
