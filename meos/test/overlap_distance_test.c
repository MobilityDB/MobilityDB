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
 * @brief A program that tests the distance of a temporal point to geometries
 * whose surfaces overlap in the plane against its closed form
 * @details A temporal point standing still at one place is at the distance of
 * that place to the geometry. A place covered by a surface of the geometry is
 * at distance 0, whatever other surfaces of the geometry cover it too: a
 * collection of polygons, a TIN and a polyhedral surface are the union of
 * their surfaces, which may overlap in the plane. The program asks the nearest
 * approach distance of points covered by one, by two and by no surface, of a
 * temporal point and of a temporal circular buffer, and whether the geometry
 * contains and covers a circular buffer.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o overlap_distance_test overlap_distance_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* Tolerance of the comparison with the closed form */
#define TOLERANCE 1e-9

static int failures = 0;

/*
 * Compare the nearest approach distance of a temporal point, or of a temporal
 * circular buffer of radius @p radius when it is positive, standing still at
 * a place with the closed form
 */
static void
check(double x, double y, double radius, const char *wkt, double expected)
{
  char text[256];
  Temporal *temp;
  if (radius > 0)
  {
    snprintf(text, sizeof(text), "[Cbuffer(Point(%g %g),%g)@2001-01-01, "
      "Cbuffer(Point(%g %g),%g)@2001-01-02]", x, y, radius, x, y, radius);
    temp = tcbuffer_in(text);
  }
  else
  {
    snprintf(text, sizeof(text),
      "[Point(%g %g)@2001-01-01, Point(%g %g)@2001-01-02]", x, y, x, y);
    temp = tgeompoint_in(text);
  }
  GSERIALIZED *gs = geom_in(wkt, -1);
  double d = (radius > 0) ? nad_tcbuffer_geo(temp, gs) : nad_tgeo_geo(temp, gs);
  bool ok = fabs(d - expected) <= TOLERANCE;
  printf("  (%g %g) r %-4g %-66s %.12g %s\n", x, y, radius, wkt, d,
    ok ? "OK" : "FAIL");
  if (! ok)
  {
    printf("    closed form %.12g\n", expected);
    failures++;
  }
  free(temp); free(gs);
}

/*
 * Compare whether a geometry contains and covers a temporal circular buffer
 * standing still at a place, always and ever, with the expected answer
 */
static void
check_contains(double x, double y, double radius, const char *wkt,
  int expected)
{
  char text[256];
  snprintf(text, sizeof(text), "[Cbuffer(Point(%g %g),%g)@2001-01-01, "
    "Cbuffer(Point(%g %g),%g)@2001-01-02]", x, y, radius, x, y, radius);
  Temporal *temp = tcbuffer_in(text);
  GSERIALIZED *gs = geom_in(wkt, -1);
  int contains = acontains_geo_tcbuffer(gs, temp);
  int covers = ecovers_geo_tcbuffer(gs, temp);
  bool ok = contains == expected && covers == expected;
  printf("  (%g %g) r %-4g %-66s contains %d covers %d %s\n", x, y, radius,
    wkt, contains, covers, ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
  free(temp); free(gs);
}

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  const char *twice = "GEOMETRYCOLLECTION(POLYGON((0 0,1 0,1 1,0 1,0 0)),"
    "POLYGON((0 0,1 0,1 1,0 1,0 0)))";
  const char *folded = "GEOMETRYCOLLECTION(POLYGON((0 0,2 0,0 2,0 0)),"
    "POLYGON((0 0,2 0,2 2,0 0)))";
  const char *tin = "TIN(((0 0,2 0,0 2,0 0)),((0 0,2 0,2 2,0 0)))";
  const char *solid = "POLYHEDRALSURFACE(((0 0,1 0,1 1,0 1,0 0)),"
    "((0 0,1 0,1 1,0 1,0 0)))";
  const char *apart = "TIN(((0 0,1 0,0 1,0 0)),((1 0,1 1,0 1,1 0)))";

  printf("Places covered by two surfaces:\n");
  check(0.5, 0.4, 0, twice, 0.0);
  check(0.5, 0.4, 0, folded, 0.0);
  check(0.5, 0.4, 0, tin, 0.0);
  check(0.5, 0.4, 0, solid, 0.0);
  check(0.5, 0.4, 0.1, twice, 0.0);
  printf("Places covered by one surface:\n");
  check(0.3, 1.2, 0, folded, 0.0);
  check(0.2, 0.2, 0, apart, 0.0);
  printf("Places covered by none:\n");
  check(5, 0.4, 0, twice, 4.0);
  check(3, 0.4, 0, tin, 1.0);
  check(0.5, -3, 0, solid, 3.0);
  check(2, 2, 0, apart, sqrt(2.0));
  printf("Surfaces containing a circular buffer:\n");
  check_contains(0.5, 0.4, 0.1, twice, 1);
  check_contains(0.5, 0.4, 0.1, folded, 1);
  check_contains(5, 0.4, 0.1, twice, 0);

  if (failures == 0)
    printf("Overlap distance test: all tests passed\n");
  else
    printf("Overlap distance test: %d test(s) FAILED\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
