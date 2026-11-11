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
 * @brief Test of the location of a point in an area that the ray casts of the
 * geometry engine answer: the inside test of the nearest approach distance,
 * #dist_geom_point_inside, and the point in polygon test of the spatial
 * relationships, #meos_point_in_polygon
 * @details Each case is built so that its answer is known in closed form, as
 * #segment_cases of arc_cross_unit.c builds its own. A point at the centre of
 * a disc, or 3/5 of the way to its circle, is inside it, so its nearest
 * approach distance is 0, and a point 10 from the centre of a disc of radius
 * 5 is 5 from it; each runs at the scales 2^-40, 1 and 2^40, which are exact
 * on doubles, so an answer depends on no magnitude. The triangles share an
 * edge and a point a few units in the last place off its midpoint, the third
 * vertex on either side, so the point is inside one and outside the other;
 * its side of the edge is the sign of a determinant of degree two in the
 * coordinates, read exactly on rationals.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>

static int failures = 0;
static int checks = 0;

static void
check(const char *name, double s, bool got, bool expected)
{
  checks++;
  if (got != expected)
  {
    failures++;
    printf("FAIL %-48s scale %-8g got %s, expected %s\n", name, s,
      got ? "true" : "false", expected ? "true" : "false");
  }
  return;
}

/* A temporal point at rest at s (x, y) against the disc of radius 5 s about
 * the origin, the whole circle of a CURVEPOLYGON */
static void
disc_cases(double s)
{
  char wkt[256];
  snprintf(wkt, sizeof(wkt),
    "CURVEPOLYGON(CIRCULARSTRING(%.17g 0,%.17g 0,%.17g 0))", -5 * s, 5 * s,
    -5 * s);
  GSERIALIZED *disc = geom_in(wkt, -1);
  struct
  {
    const char *name;
    double x, y, nad;
  } cases[] = {
    {"disc, the point at its centre", 0, 0, 0},
    {"disc, the point at (3, 0)", 3, 0, 0},
    {"disc, the point at (0, -3)", 0, -3, 0},
    {"disc, the point 10 from its centre", 10, 0, 5},
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
  {
    snprintf(wkt, sizeof(wkt), "POINT(%.17g %.17g)@2001-01-01",
      s * cases[i].x, s * cases[i].y);
    Temporal *temp = tgeompoint_in(wkt);
    check(cases[i].name, s,
      nad_tgeo_geo(temp, disc, false) == s * cases[i].nad, true);
    free(temp);
  }
  free(disc);
  return;
}

/* The point P against the triangles A B C1, which holds it, and A B C2, which
 * does not: P lies a few units in the last place off the midpoint of A B, on
 * the side of C1 */
#define TRI_A "13.436424411240122 84.74337369372327"
#define TRI_B "76.3774618976614 25.50690257394217"
#define TRI_C1 "72.32100289862751 84.25363497700519"
#define TRI_C2 "17.492883410274025 25.996641290660254"
#define TRI_P "POINT(44.90694315445077 55.12513813383271)"

static void
triangle_cases(void)
{
  GSERIALIZED *in = geom_in("POLYGON((" TRI_A "," TRI_B "," TRI_C1 ","
    TRI_A "))", -1);
  GSERIALIZED *out = geom_in("POLYGON((" TRI_A "," TRI_B "," TRI_C2 ","
    TRI_A "))", -1);
  GSERIALIZED *p = geom_in(TRI_P, -1);
  GSERIALIZED *a = geom_in("POINT(" TRI_A ")", -1);
  check("triangle holding the point contains it", 1, geom_contains(in, p),
    true);
  check("triangle holding the point does not touch it", 1,
    geom_touches(in, p), false);
  check("triangle not holding the point does not cover it", 1,
    geom_covers(out, p), false);
  check("triangle not holding the point does not touch it", 1,
    geom_touches(out, p), false);
  /* Controls: a vertex is on the boundary of both */
  check("triangle touches its vertex", 1, geom_touches(in, a), true);
  check("triangle does not contain its vertex", 1, geom_contains(out, a),
    false);
  free(in); free(out); free(p); free(a);
  return;
}

int
main(void)
{
  meos_initialize();
  const double scales[] = {0x1p-40, 1.0, 0x1p40};
  for (size_t k = 0; k < sizeof(scales) / sizeof(scales[0]); k++)
    disc_cases(scales[k]);
  triangle_cases();
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
