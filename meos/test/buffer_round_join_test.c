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
 * @brief The round-join buffer of polygons whose joins meet exactly
 * @details A round join is the arc of the circle of centre the vertex and
 * radius the distance. The polygons below make those arcs meet or tie
 * exactly: an axis-aligned square, whose joins on one side reach the same
 * greatest coordinate; a square with a hole that the distance shrinks to
 * exactly a point; a square at projected coordinates narrower than the
 * distance; and a rhombus whose coordinates span 2^1000, its offsets and
 * joins decided across that spread. Each buffer is a surface that covers the polygon it is taken of,
 * with no error, and the holed square's buffer also covers its hole. A band
 * whose ends lie near 1e200, buffered by about 1e183, returns: past the
 * range of the exact signs of its arcs they are declined. A line whose cap
 * is two arcs one unit in the last place apart is answered, its signs held
 * in numbers wider than expansions.
 *
 * The area of a buffer has a closed form where the polygon is a square, and
 * the buffers below answer it: the square at coordinates whose differences
 * with the distance no double holds, a hole within a circle smaller than
 * the distance, which the buffer covers whole, a hole the buffer leaves
 * open, the square its erosion is, and two squares apart, a buffer of two
 * surfaces.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o buffer_round_join_test \
 *   buffer_round_join_test.c -L/usr/local/lib -lmeos -lm
 * ./buffer_round_join_test
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>

/* The buffer of @p wkt by 1, asserted to be a surface covering @p cover */
static void
buffer_covers(const char *label, const char *wkt, const char *cover)
{
  GSERIALIZED *geo = geom_in(wkt, -1);
  GSERIALIZED *cov = geom_in(cover, -1);
  assert(geo != NULL && cov != NULL);
  meos_errno_reset();
  GSERIALIZED *buf = geom_buffer(geo, 1.0, "");
  printf("geom_buffer(%s): %d, errno %d\n", label, buf != NULL, meos_errno());
  assert(buf != NULL);
  assert(meos_errno() == 0);
  char covers_patt[10] = "T*****FF*";
  bool covers = geom_relate_pattern(buf, cov, covers_patt);
  printf("  it covers %s: %d\n", cover == wkt ? "the polygon" : "the hole",
    covers);
  assert(covers);
  free(geo); free(cov); free(buf);
}

/* The buffer of @p wkt by 1, as #buffer_covers takes it, asserted to be
 * answered with no error and to have a box holding the box of the polygon:
 * at coordinates whose squares no double holds, where a relate cannot read
 * the cover */
static void
buffer_box_holds(const char *label, const char *wkt)
{
  GSERIALIZED *geo = geom_in(wkt, -1);
  assert(geo != NULL);
  meos_errno_reset();
  GSERIALIZED *buf = geom_buffer(geo, 1.0, "");
  printf("geom_buffer(%s): %d, errno %d\n", label, buf != NULL, meos_errno());
  assert(buf != NULL);
  assert(meos_errno() == 0);
  STBox *box = geo_to_stbox(geo), *bbox = geo_to_stbox(buf);
  bool holds = contains_stbox_stbox(bbox, box);
  printf("  its box holds that of the polygon: %d\n", holds);
  assert(holds);
  free(geo); free(buf); free(box); free(bbox);
}

/* The buffer of @p wkt by 1, as #buffer_covers takes it, asserted to have
 * the area @p area to the rounding of the points of its arcs */
static void
buffer_area_is(const char *label, const char *wkt, double area)
{
  GSERIALIZED *geo = geom_in(wkt, -1);
  assert(geo != NULL);
  meos_errno_reset();
  GSERIALIZED *buf = geom_buffer(geo, 1.0, "");
  assert(buf != NULL);
  assert(meos_errno() == 0);
  double got = geom_area(buf);
  printf("area of geom_buffer(%s): %.15g, closed form %.15g\n", label, got,
    area);
  assert(fabs(got - area) <= 1e-12 * area);
  free(geo); free(buf);
}

/* The buffer of @p wkt by @p dist, as #buffer_box_holds takes it at
 * coordinates whose squares no double holds, asserted to return within a
 * few seconds of processor time: there the products of the exact signs of
 * an arc meeting an offset pass 2^1000, and the signs are declined rather
 * than carried on in expansions that only grow. What answers the buffer
 * then is not judged here */
static void
buffer_returns(const char *label, const char *wkt, double dist)
{
  GSERIALIZED *geo = geom_in(wkt, -1);
  assert(geo != NULL);
  meos_errno_reset();
  clock_t start = clock();
  GSERIALIZED *buf = geom_buffer(geo, dist, "");
  double secs = (double) (clock() - start) / CLOCKS_PER_SEC;
  printf("geom_buffer(%s): returned in %s\n", label,
    secs < 5.0 ? "under 5 s" : "5 s or more");
  assert(secs < 5.0);
  free(geo);
  if (buf)
    free(buf);
}

/* The buffer of @p wkt by @p dist, as #buffer_box_holds takes it, asserted
 * to be answered with no error and to contain the first vertex of the line,
 * which stands the whole distance inside it */
static void
buffer_holds_start(const char *label, const char *wkt, const char *start,
  double dist)
{
  GSERIALIZED *geo = geom_in(wkt, -1);
  GSERIALIZED *pt = geom_in(start, -1);
  assert(geo != NULL && pt != NULL);
  meos_errno_reset();
  GSERIALIZED *buf = geom_buffer(geo, dist, "");
  printf("geom_buffer(%s): %d, errno %d\n", label, buf != NULL, meos_errno());
  assert(buf != NULL);
  assert(meos_errno() == 0);
  bool holds = geom_contains(buf, pt);
  printf("  it contains the start of the line: %d\n", holds);
  assert(holds);
  free(geo); free(pt); free(buf);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const char *square = "Polygon((0 0,10 0,10 10,0 10,0 0))";
  buffer_covers("an axis-aligned square", square, square);

  const char *holed = "Polygon((0 0,10 0,10 10,0 10,0 0),"
    "(4 4,4 6,6 6,6 4,4 4))";
  buffer_covers("a square whose hole shrinks to a point", holed, holed);
  buffer_covers("a square whose hole shrinks to a point", holed,
    "Polygon((4 4,6 4,6 6,4 6,4 4))");

  const char *narrow = "Polygon((6400000 6400000,6400000.02 6400000,"
    "6400000.02 6400000.02,6400000 6400000.02,6400000 6400000))";
  buffer_covers("a square narrower than the distance", narrow, narrow);

  const char *spread = "Polygon((-1.0715086071862673e+301 1,0 0,"
    "1.0715086071862673e+301 1,0 2,-1.0715086071862673e+301 1))";
  buffer_box_holds("a rhombus whose coordinates span 2^1000", spread);

  buffer_returns("a band 2e200 wide with a notch, by 2e183",
    "Polygon((-1e200 -1,1e200 -1,1e200 1,0.1 1,0 0.5,-0.1 1,-1e200 1,"
    "-1e200 -1))", 2.1245519712670684e+183);

  /* The cap at its start is two arcs of one circle whose centres the
   * offsets round one unit in the last place apart: the sign of their
   * meeting is held only by numbers wider than expansions */
  buffer_holds_start("a line whose cap is two arcs one ulp apart",
    "LineString(979403.0128193274 814667.1888555789,"
    "977964.507578686 815985.468285189,978638.7202668741 815940.6542353164,"
    "979693.8684792126 814189.2127569565,"
    "978005.5565088153 815192.5834514729,"
    "979027.7235275264 815974.5523308413)",
    "Point(979403.0128193274 814667.1888555789)", 0.054522116184764186);

  /* A square of side 10 by 1: its area, its perimeter by 1 and the disc */
  const double pi = 3.14159265358979323846;
  buffer_area_is("an axis-aligned square", square, 140.0 + pi);
  buffer_area_is("a square off the binary grid",
    "Polygon((0.1 0.1,10.1 0.1,10.1 10.1,0.1 10.1,0.1 0.1))", 140.0 + pi);
  buffer_area_is("a square whose hole shrinks to a point", holed,
    140.0 + pi);
  buffer_area_is("a square with a hole a smaller circle encloses",
    "Polygon((0 0,10 0,10 10,0 10,0 0),(4 4,4 5,5 5,5 4,4 4))", 140.0 + pi);
  /* The hole of side 4 eroded by 1 is the square of side 2 */
  buffer_area_is("a square with a hole the buffer leaves open",
    "Polygon((0 0,10 0,10 10,0 10,0 0),(3 3,3 7,7 7,7 3,3 3))",
    136.0 + pi);
  buffer_area_is("two squares apart",
    "MultiPolygon(((0 0,1 0,1 1,0 1,0 0)),((10 0,11 0,11 1,10 1,10 0)))",
    10.0 + 2.0 * pi);

  printf("buffer_round_join_test: every buffer a surface covering its "
    "polygon, of the area of its closed form\n");
  meos_finalize();
  return 0;
}
