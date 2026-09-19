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
 * temporal point and of a temporal circular buffer, whether the geometry
 * contains and covers a circular buffer, the temporal distance of a
 * circular buffer to a TIN and a polyhedral surface, and when a circular
 * buffer moving across a TIN, whose two faces share an edge, intersects it,
 * is within a distance of it and is ever disjoint from it, and whether the TIN
 * contains, covers and touches a buffer standing across the shared edge.
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
 * Compare the temporal distance of a temporal circular buffer standing still
 * at a place with the closed form, at its first instant
 */
static void
check_tdistance(double x, double y, double radius, const char *wkt,
  double expected)
{
  char text[256];
  snprintf(text, sizeof(text), "[Cbuffer(Point(%g %g),%g)@2001-01-01, "
    "Cbuffer(Point(%g %g),%g)@2001-01-02]", x, y, radius, x, y, radius);
  Temporal *temp = tcbuffer_in(text);
  GSERIALIZED *gs = geom_in(wkt, -1);
  Temporal *dist = tdistance_tcbuffer_geo(temp, gs);
  double d = dist ? tfloat_start_value(dist) : -1.0;
  bool ok = fabs(d - expected) <= TOLERANCE;
  printf("  (%g %g) r %-4g %-66s %.12g %s\n", x, y, radius, wkt, d,
    ok ? "OK" : "FAIL");
  if (! ok)
  {
    printf("    closed form %.12g\n", expected);
    failures++;
  }
  free(dist); free(temp); free(gs);
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

/*
 * Compare when a temporal circular buffer of radius @p radius moving along
 * y = @p y from x = -1 at 2001-01-01 to x = 3 at 2001-01-05, one unit a day,
 * is within @p dist of a geometry with the closed form: from @p lo to @p hi
 * days after its start
 */
static void
check_within(double y, double radius, double dist, const char *wkt,
  double lo, double hi)
{
  char text[256];
  snprintf(text, sizeof(text), "[Cbuffer(Point(-1 %g),%g)@2001-01-01, "
    "Cbuffer(Point(3 %g),%g)@2001-01-05]", y, radius, y, radius);
  Temporal *temp = tcbuffer_in(text);
  GSERIALIZED *gs = geom_in(wkt, -1);
  Temporal *tb = (dist > 0) ? tdwithin_tcbuffer_geo(temp, gs, dist) :
    tintersects_tcbuffer_geo(temp, gs);
  SpanSet *ss = tb ? tbool_when_true(tb) : NULL;
  /* The start of the value, 2001-01-01, in microseconds */
  TimestampTz start = timestamptz_in("2001-01-01", -1);
  double day = 86400e6;
  double l = ss ? (tstzspanset_lower(ss) - start) / day : -1.0;
  double u = ss ? (tstzspanset_upper(ss) - start) / day : -1.0;
  /* The answer is read to two microseconds */
  bool ok = ss && spanset_num_spans(ss) == 1 && fabs(l - lo) <= 2.0 / day &&
    fabs(u - hi) <= 2.0 / day;
  printf("  y %g r %-4g d %-4g %-66s [%.9g, %.9g] %s\n", y, radius, dist,
    wkt, l, u, ok ? "OK" : "FAIL");
  if (! ok)
  {
    printf("    closed form [%.9g, %.9g]\n", lo, hi);
    failures++;
  }
  free(ss); free(tb); free(temp); free(gs);
}

/*
 * Compare whether a temporal circular buffer moving from one place to another
 * is ever disjoint from a geometry with the expected answer
 */
static void
check_edisjoint(const char *text, const char *wkt, int expected)
{
  Temporal *temp = tcbuffer_in(text);
  GSERIALIZED *gs = geom_in(wkt, -1);
  int disjoint = edisjoint_tcbuffer_geo(temp, gs);
  bool ok = disjoint == expected;
  printf("  %-60s %-50s edisjoint %d %s\n", text, wkt, disjoint,
    ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
  free(temp); free(gs);
}

/*
 * Compare whether a temporal circular buffer standing still at a place ever
 * touches a geometry with the expected answer
 */
static void
check_touches(double x, double y, double radius, const char *wkt,
  int expected)
{
  char text[256];
  snprintf(text, sizeof(text), "[Cbuffer(Point(%g %g),%g)@2001-01-01, "
    "Cbuffer(Point(%g %g),%g)@2001-01-02]", x, y, radius, x, y, radius);
  Temporal *temp = tcbuffer_in(text);
  GSERIALIZED *gs = geom_in(wkt, -1);
  int touches = etouches_tcbuffer_geo(temp, gs);
  bool ok = touches == expected;
  printf("  (%g %g) r %-4g %-66s touches %d %s\n", x, y, radius, wkt,
    touches, ok ? "OK" : "FAIL");
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
  check_contains(0.5, 0.4, 0.1, tin, 1);
  check_contains(5, 0.4, 0.1, twice, 0);
  printf("Temporal distance of a circular buffer to a surface:\n");
  check_tdistance(3, 0.5, 0.1, apart, 1.9);
  check_tdistance(3, 0.5, 0.1,
    "POLYHEDRALSURFACE(((0 0,1 0,0 1,0 0)),((1 0,1 1,0 1,1 0)))", 1.9);
  /* The two faces of the TIN cover the unit square and share its diagonal
   * from (1 0) to (0 1), which lies inside the TIN: the moving circular buffer
   * crosses it while inside, and meets the TIN from x = -r - d to x = 1 + r + d,
   * that is x + 1 days after its start */
  printf("A circular buffer moving across a TIN:\n");
  check_within(0.5, 0.1, 0, apart, 0.9, 2.1);
  check_within(0.5, 0.1, 0.4, apart, 0.5, 2.5);
  check_within(0.25, 0.2, 0, apart, 0.8, 2.2);
  check_edisjoint("[Cbuffer(Point(-1 0.5),0.1)@2001-01-01, "
    "Cbuffer(Point(3 0.5),0.1)@2001-01-05]", apart, 1);
  check_edisjoint("[Cbuffer(Point(0.2 0.5),0.1)@2001-01-01, "
    "Cbuffer(Point(0.8 0.5),0.1)@2001-01-02]", apart, 0);
  /* The shared diagonal is inside the TIN and is none of its boundary: a
   * buffer centred on it is contained, covered and touches nothing, while one
   * tangent to a side of the square from outside touches it */
  check_contains(0.5, 0.5, 0.1, apart, 1);
  check_touches(0.5, 0.5, 0.1, apart, 0);
  check_touches(1.1, 0.5, 0.1, apart, 1);

  if (failures == 0)
    printf("Overlap distance test: all tests passed\n");
  else
    printf("Overlap distance test: %d test(s) FAILED\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
