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
 * @brief Test of the proper crossings of a segment and an arc and of two arcs
 * that relate reads, #arcsegm_cross and #arcarc_cross of
 * meos/include/geo/geo_funcs.h
 * @details Each case is built so that its answer is known in closed form: the
 * arcs lie on circles of radius 5 through points whose coordinates are
 * doubles (the 3-4-5 triangle), and every offset is a power of two, so each
 * crossing lies where the construction puts it. A crossing is proper where
 * the two edges pass through each other at a point interior to both; a
 * tangency, a meeting at an end of either edge and a meeting outside the span
 * of an arc are not. Each case runs at the scales 2^-40, 1 and 2^40, which are
 * exact on doubles, so an answer depends on no magnitude. The edges are the
 * ones relate reads, from #geom_extract_edges.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <meos.h>
#include <meos_internal.h>
#include <liblwgeom.h>

#include "geo/geo_funcs.h"

/* An offset below the bands of the predicates, a double at every scale */
#define TINY 0x1p-44

static int failures = 0;
static int checks = 0;

/* The first edge of a circular string of three points, scaled by s */
static Edge
arc_edge(double s, double x1, double y1, double x2, double y2, double x3,
  double y3)
{
  char wkt[256];
  snprintf(wkt, sizeof(wkt), "CIRCULARSTRING(%.17g %.17g,%.17g %.17g,"
    "%.17g %.17g)", s * x1, s * y1, s * x2, s * y2, s * x3, s * y3);
  LWGEOM *geom = lwgeom_from_wkt(wkt, LW_PARSER_CHECK_NONE);
  MeosArray *edges = geom_extract_edges(geom);
  if (! edges || edges->count != 1)
  {
    fprintf(stderr, "arc_edge: %s does not give one edge\n", wkt);
    exit(EXIT_FAILURE);
  }
  Edge e = ((Edge *) edges->elems)[0];
  meos_array_destroy(edges);
  lwgeom_free(geom);
  return e;
}

static void
check(const char *name, double s, bool got, bool expected)
{
  checks++;
  if (got != expected)
  {
    failures++;
    printf("FAIL %-44s scale %-8g got %s, expected %s\n", name, s,
      got ? "crossing" : "no crossing", expected ? "crossing" : "no crossing");
  }
  return;
}

/* The segment from s (ax, ay) along s (rx, ry) against the arc from (5, 0)
 * through (3, 4) to (-3, 4), counterclockwise from angle 0 to about 127 deg */
static void
segment_cases(double s)
{
  Edge arc = arc_edge(s, 5, 0, 3, 4, -3, 4);
  struct
  {
    const char *name;
    double ax, ay, rx, ry;
    bool expected;
  } cases[] = {
    {"crosses at (0, 5)", 0, 0, 0, 10, true},
    {"tangent at (0, 5)", -10, 5, 20, 0, false},
    {"secant 2^-44 below the tangent", -10, 5 - TINY, 20, 0, true},
    {"meets at the start of the segment", 0, 5, 0, 5, false},
    {"crosses 2^-44 into the segment", 0, 5 - 10 * TINY, 0, 10, true},
    {"meets at the start of the arc", 4, 0, 2, 0, false},
    {"crosses just past the start of the arc", 4, 0x1p-40, 2, 0, true},
    {"crosses the circle outside the arc", 0, 0, 0, -10, false},
    {"misses the circle", 10, 10, 1, 0, false},
  };
  /* The end of each segment is a + r, exact on these values */
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
    check(cases[i].name, s, arcsegm_cross(s * cases[i].ax, s * cases[i].ay,
      s * (cases[i].ax + cases[i].rx), s * (cases[i].ay + cases[i].ry), &arc),
      cases[i].expected);
  return;
}

/* The right half of the circle of radius 5 about the origin, or its stretch
 * from (4, -3) to (4, 3), against the left half of the circle of radius 5
 * about (c, 0) */
static void
arc_cases(double s)
{
  Edge right = arc_edge(s, 0, -5, 5, 0, 0, 5);
  Edge corner = arc_edge(s, 4, -3, 5, 0, 4, 3);
  struct
  {
    const char *name;
    const Edge *a;
    double c;
    bool expected;
  } cases[] = {
    {"circles 8 apart cross at (4, +-3)", &right, 8, true},
    {"circles 10 apart touch at (5, 0)", &right, 10, false},
    {"circles 2^-43 nearer than tangent", &right, 10 - 2 * TINY, true},
    {"circles 8 apart meet at the ends of the arc", &corner, 8, false},
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
  {
    double c = cases[i].c;
    Edge left = arc_edge(s, c, 5, c - 5, 0, c, -5);
    check(cases[i].name, s, arcarc_cross(cases[i].a, &left),
      cases[i].expected);
  }
  return;
}

/* A segment from the centre of the whole circle of radius 5 about the origin,
 * the closed circular string of three points #geocircle_make writes for a
 * circular buffer, against each edge relate reads from it, as #segment_cases
 * does for one arc: the segment crosses exactly one of them, whichever way it
 * points */
static void
circle_cases(double s)
{
  char wkt[256];
  snprintf(wkt, sizeof(wkt), "CIRCULARSTRING(%.17g 0,%.17g 0,%.17g 0)",
    -5 * s, 5 * s, -5 * s);
  LWGEOM *geom = lwgeom_from_wkt(wkt, LW_PARSER_CHECK_NONE);
  MeosArray *edges = geom_extract_edges(geom);
  struct
  {
    const char *name;
    double rx, ry;
  } cases[] = {
    {"whole circle, from the centre up", 0, 10},
    {"whole circle, from the centre down", 0, -10},
    {"whole circle, from the centre at (3, 4)", 6, 8},
    {"whole circle, from the centre at (-3, -4)", -6, -8},
  };
  for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
  {
    int ncross = 0;
    for (size_t j = 0; j < edges->count; j++)
      ncross += arcsegm_cross(0, 0, s * cases[i].rx, s * cases[i].ry,
        &((Edge *) edges->elems)[j]);
    check(cases[i].name, s, ncross == 1, true);
  }
  meos_array_destroy(edges);
  lwgeom_free(geom);
  return;
}

int
main(void)
{
  meos_initialize();
  const double scales[] = {0x1p-40, 1.0, 0x1p40};
  for (size_t k = 0; k < sizeof(scales) / sizeof(scales[0]); k++)
  {
    segment_cases(scales[k]);
    arc_cases(scales[k]);
    circle_cases(scales[k]);
  }
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
