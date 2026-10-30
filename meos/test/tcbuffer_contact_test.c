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
 * @brief The contact of a circular buffer at rest with a polygon, judged by
 * the closed form
 *
 * @details A disc of centre c and radius r against a polygon with straight
 * edges, D the distance of c to the boundary: the polygon covers the disc
 * where c lies inside and D >= r, contains it as OGC defines it where it
 * covers it and r > 0, or where c lies inside and D > r for r = 0, and the
 * disc touches the polygon where c lies outside and D = r, all decided
 * exactly on the doubles. Every input below is a dyadic rational, so each
 * expected answer follows by hand:
 * - a disc tangent to the square from outside touches it, and one stopping
 *   2^-40 short of it or crossing it by 2^-40 does not, however small the
 *   gap or the overlap;
 * - a disc tangent to the square from inside is contained and covered, its
 *   open interior lying inside the square;
 * - a disc clear inside by 2^-40 is contained and covered, and one out of
 *   it by 2^-40 is neither;
 * - a disc of radius 0 on a side is covered and not contained;
 * - a disc tangent from inside to a static disc is contained and covered,
 *   as #cbuffer_contains answers for the two discs at rest.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o tcbuffer_contact_test \
 *   tcbuffer_contact_test.c -L/usr/local/lib -lmeos -lm
 * ./tcbuffer_contact_test
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

static const char *SQUARE = "POLYGON((0 0,1000 0,1000 1000,0 1000,0 0))";
/* The circle of centre (0 0) and radius 500 as two arcs of three points */
static const char *CIRCLE =
  "CURVEPOLYGON(CIRCULARSTRING(500 0,0 500,-500 0,0 -500,500 0))";

/* The value at its instant of a temporal boolean, freed */
static bool
at_instant(Temporal *t)
{
  assert(t != NULL);
  bool v = tbool_start_value(t);
  free(t);
  return v;
}

/* Contains, covers and touches of a disc at rest against the square */
static void
relate(double x, double y, double r, bool *contains, bool *covers,
  bool *touches)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-01",
    x, y, r);
  Temporal *temp = tcbuffer_in(buf);
  GSERIALIZED *gs = geom_in(SQUARE, -1);
  assert(temp != NULL && gs != NULL);
  *contains = at_instant(tcontains_geo_tcbuffer(gs, temp));
  *covers = at_instant(tcovers_geo_tcbuffer(gs, temp));
  *touches = at_instant(ttouches_tcbuffer_geo(temp, gs));
  free(gs); free(temp);
}

/* Contains, covers and touches of a disc at rest against the geometry of
 * @p wkt, as #relate reads them against the square */
static void
relate_geo(const char *wkt, double x, double y, double r, bool *contains,
  bool *covers, bool *touches)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-01",
    x, y, r);
  Temporal *temp = tcbuffer_in(buf);
  GSERIALIZED *gs = geom_in(wkt, -1);
  assert(temp != NULL && gs != NULL);
  *contains = at_instant(tcontains_geo_tcbuffer(gs, temp));
  *covers = at_instant(tcovers_geo_tcbuffer(gs, temp));
  *touches = at_instant(ttouches_tcbuffer_geo(temp, gs));
  free(gs); free(temp);
}

int
main(void)
{
  meos_initialize();
  bool c, v, t;

  /* Tangent from outside, at y = -300 with radius 300 */
  relate(500.0, -300.0, 300.0, &c, &v, &t);
  assert(! c && ! v && t);
  /* Short of the square by 2^-40 */
  relate(500.0, -300.0, 300.0 - 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);
  /* Across the boundary by 2^-40, so the interiors meet */
  relate(500.0, -300.0, 300.0 + 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);

  /* Tangent from inside, at y = 300 with radius 300 */
  relate(500.0, 300.0, 300.0, &c, &v, &t);
  assert(c && v && ! t);
  /* Clear inside by 2^-40 */
  relate(500.0, 300.0, 300.0 - 0x1p-40, &c, &v, &t);
  assert(c && v && ! t);
  /* Out of the square by 2^-40 */
  relate(500.0, 300.0, 300.0 + 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);
  /* Radius 0 on the side y = 0: covered, not contained */
  relate(500.0, 0.0, 0.0, &c, &v, &t);
  assert(! c && v);

  /* Against the circle of radius 500, tangent at (300 400), a point of an
   * arc between its input points: from outside, the centre (480 640) at
   * distance 800 from the centre of the circle */
  relate_geo(CIRCLE, 480.0, 640.0, 300.0, &c, &v, &t);
  assert(! c && ! v && t);
  relate_geo(CIRCLE, 480.0, 640.0, 300.0 - 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);
  relate_geo(CIRCLE, 480.0, 640.0, 300.0 + 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);
  /* From inside, the centre (120 160) at distance 200 */
  relate_geo(CIRCLE, 120.0, 160.0, 300.0, &c, &v, &t);
  assert(c && v && ! t);
  relate_geo(CIRCLE, 120.0, 160.0, 300.0 - 0x1p-40, &c, &v, &t);
  assert(c && v && ! t);
  relate_geo(CIRCLE, 120.0, 160.0, 300.0 + 0x1p-40, &c, &v, &t);
  assert(! c && ! v && ! t);

  /* Disc (2 0) of radius 3 in the static disc (0 0) of radius 5, tangent
   * from inside at (5 0) */
  Cbuffer *outer = cbuffer_in("Cbuffer(Point(0 0),5)");
  Temporal *inner = tcbuffer_in("Cbuffer(Point(2 0),3)@2001-01-01");
  assert(outer != NULL && inner != NULL);
  assert(at_instant(tcontains_cbuffer_tcbuffer(outer, inner)));
  assert(at_instant(tcovers_cbuffer_tcbuffer(outer, inner)));
  free(outer); free(inner);

  printf("tcbuffer_contact_test: every contact answered as its closed form\n");
  meos_finalize();
  return 0;
}
