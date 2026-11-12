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
 * @brief Test of the relationships of two areas bounded by arcs that meet
 * near an end of an arc, #geom_disjoint2d, #geom_touches, #geom_contains and
 * #geom_covers
 * @details Each case is built so that its answer is known, as #arc_cases of
 * arc_cross_unit.c builds its own: the lens of the arc of the circle of
 * radius 5 about the origin from (4, -3) through (5, 0) to (4, 3), closed by
 * its chord, against a lens or a half disc of the circle of radius 5 about
 * (c, 0). With c = 8 - 2^-44 the lens from (c - 4, 3) through (c - 5, 0) to
 * (c - 4, -3) lies 2^-44 left of the first, the two chords parallel, and the
 * two are disjoint. With c = 8 + 2^-44 the half disc left of the diameter
 * from (c, 5) to (c, -5) crosses the first lens 2^-44 inside the ends of its
 * arc: their interiors overlap and neither contains the other. Two squares
 * 2^-44 apart are the straight control. Each runs at the scales 2^-40, 1 and
 * 2^40, which are exact on doubles, so an answer depends on no magnitude.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>

#define GAP 0x1p-44

static int failures = 0;
static int checks = 0;

static void
check(const char *name, const char *pred, double s, bool got, bool expected)
{
  checks++;
  if (got != expected)
  {
    failures++;
    printf("FAIL %-36s %-9s scale %-8g got %d, expected %d\n", name, pred, s,
      got, expected);
  }
  return;
}

/* The lens of the arc from (s x1, s y1) through (s x2, s y2) to (s x3, s y3)
 * closed by its chord */
static GSERIALIZED *
lens(double s, double x1, double y1, double x2, double y2, double x3,
  double y3)
{
  char wkt[512];
  snprintf(wkt, sizeof(wkt), "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING("
    "%.17g %.17g,%.17g %.17g,%.17g %.17g),(%.17g %.17g,%.17g %.17g)))",
    s * x1, s * y1, s * x2, s * y2, s * x3, s * y3, s * x3, s * y3, s * x1,
    s * y1);
  return geom_in(wkt, -1);
}

/* Disjoint, touches, contains and covers, both ways round */
static void
relate_case(const char *name, double s, const GSERIALIZED *a,
  const GSERIALIZED *b, bool disjoint, bool touches, bool contains_ab,
  bool contains_ba)
{
  check(name, "disjoint", s, geom_disjoint2d(a, b), disjoint);
  check(name, "touches", s, geom_touches(a, b), touches);
  check(name, "contains", s, geom_contains(a, b), contains_ab);
  check(name, "covers", s, geom_covers(a, b), contains_ab);
  check(name, "contains'", s, geom_contains(b, a), contains_ba);
  check(name, "covers'", s, geom_covers(b, a), contains_ba);
  return;
}

static void
arc_end_cases(double s)
{
  GSERIALIZED *corner = lens(s, 4, -3, 5, 0, 4, 3);

  double c = 8 - GAP;
  GSERIALIZED *left = lens(s, c - 4, 3, c - 5, 0, c - 4, -3);
  relate_case("lens 2^-44 beside the lens", s, corner, left, true, false,
    false, false);

  c = 8 + GAP;
  GSERIALIZED *half = lens(s, c, 5, c - 5, 0, c, -5);
  relate_case("half disc crossing 2^-44 inside", s, corner, half, false,
    false, false, false);

  char wkt[256];
  snprintf(wkt, sizeof(wkt), "POLYGON((0 0,%.17g 0,%.17g %.17g,0 %.17g,0 0))",
    s, s, s, s);
  GSERIALIZED *sq1 = geom_in(wkt, -1);
  snprintf(wkt, sizeof(wkt), "POLYGON((%.17g 0,%.17g 0,%.17g %.17g,%.17g "
    "%.17g,%.17g 0))", s * (1 + GAP), s * 2, s * 2, s, s * (1 + GAP), s,
    s * (1 + GAP));
  GSERIALIZED *sq2 = geom_in(wkt, -1);
  relate_case("square 2^-44 beside the square", s, sq1, sq2, true, false,
    false, false);

  free(corner); free(left); free(half); free(sq1); free(sq2);
  return;
}

int
main(void)
{
  meos_initialize();
  const double scales[] = {0x1p-40, 1.0, 0x1p40};
  for (size_t k = 0; k < sizeof(scales) / sizeof(scales[0]); k++)
    arc_end_cases(scales[k]);
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
