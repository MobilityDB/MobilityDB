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
 * @brief The shortest line of a point or a circular buffer at rest to a
 * geometry, each constructed end judged by the closed form
 *
 * @details The shortest line from a point c to a segment ab ends at the
 * nearest point Q of the segment, `a + t (b - a)` with `t` the rational
 * `(c - a).(b - a) / |b - a|^2` clamped to [0, 1], and starts at c itself.
 * The shortest line from a disc of centre c and radius r clear of a segment
 * ends at Q and starts at the point of its circle towards Q,
 * `c + r (Q - c) / |Q - c|`. Each constructed coordinate is the double
 * nearest its exact value, found in exact rational arithmetic from the input
 * doubles (for the circle, by exact signs of squares) and written in
 * hexadecimal:
 * - two points against a segment near 1e150, and one against a segment whose
 *   difference of ends overflows, the end the nearest double of the foot of
 *   the perpendicular;
 * - three discs clear of a segment at projected coordinates, the start the
 *   nearest double of the point of the circle.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o shortestline_nearest_test \
 *   shortestline_nearest_test.c -L/usr/local/lib -lmeos -lm
 * ./shortestline_nearest_test
 * @endcode
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* The four coordinates of the shortest line from the point (x y), r = 0, or
 * the disc of radius r > 0 centred there, at rest over two instants, to the
 * geometry wkt, read back from the little-endian EWKB of the line */
static void
shortest_line(double x, double y, double r, const char *wkt, double v[4])
{
  char buf[512];
  Temporal *temp;
  if (r == 0.0)
  {
    snprintf(buf, sizeof(buf),
      "[Point(%.17g %.17g)@2001-01-01, Point(%.17g %.17g)@2001-01-02]",
      x, y, x, y);
    temp = tgeompoint_in(buf);
  }
  else
  {
    snprintf(buf, sizeof(buf), "[Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-01, "
      "Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-02]", x, y, r, x, y, r);
    temp = tcbuffer_in(buf);
  }
  GSERIALIZED *gs = geom_in(wkt, -1);
  assert(temp != NULL && gs != NULL);
  GSERIALIZED *sl = (r == 0.0) ? shortestline_tgeo_geo(temp, gs, false) :
    shortestline_tcbuffer_geo(temp, gs);
  assert(sl != NULL);
  size_t size = 0;
  uint8_t *wkb = geo_as_ewkb(sl, "NDR", &size);
  /* Byte order, type, an SRID when the type flags one, the point count */
  uint32_t type;
  memcpy(&type, wkb + 1, 4);
  size_t off = 5 + ((type & 0x20000000) ? 4 : 0) + 4;
  assert(size >= off + 4 * sizeof(double));
  memcpy(v, wkb + off, 4 * sizeof(double));
  free(wkb); free(sl); free(gs); free(temp);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  double v[4];

  /* A point against a segment near 1e150: the end is the foot of the
   * perpendicular */
  shortest_line(-1.5285551538279573e+150, -2.0949832398599417e+149, 0.0,
    "Linestring(-1.8044225536862182e+150 3.656650249579651e+148,"
    "-2.5327694085907134e+150 -9.107457317795018e+149)", v);
  assert(v[2] == -0x1.1ccd33ff3f384p+499 && v[3] == 0x1.30c0919e2651ep+492);
  shortest_line(3.234555951276503e+150, -5.287640193575559e+149, 0.0,
    "Linestring(1.5004287092279825e+150 7.369195293504539e+148,"
    "2.003149172252982e+150 -1.0872237456866691e+150)", v);
  assert(v[2] == 0x1.37e142e0eb7f5p+499 && v[3] == -0x1.4d7820ff32949p+498);
  /* A point against a segment whose difference of ends overflows: the end is
   * the foot of the perpendicular all the same */
  shortest_line(8.595624732128586e+299, 794588.3504119772, 0.0,
    "Linestring(-1e+308 0,1e+308 0)", v);
  assert(v[2] == 0x1.489498e73ee42p+996 && v[3] == 0.0);

  /* A disc clear of a segment at projected coordinates: the start is the
   * point of its circle towards the nearest point of the segment */
  shortest_line(569726.1649257315, 5007484.1411976265, 272.79227160134485,
    "Linestring(570005.462 5007540.098,569853.7507768031 5007953.217379384)",
    v);
  assert(v[0] == 0x1.164fc78ee4b3bp+19 && v[1] == 0x1.31a368b7695e0p+22);
  assert(v[2] == 0x1.1650cc223b1e9p+19 && v[3] == 0x1.31a374ad8e819p+22);
  shortest_line(516002.8774789506, 5222225.186855064, 186.87333580055608,
    "Linestring(515705.04099999997 5222083.098,515529.749761516 "
    "5222575.5422642175)", v);
  assert(v[0] == 0x1.f7bcb4d0eb3f6p+18 && v[1] == 0x1.3ebc4a137b88fp+22);
  assert(v[2] == 0x1.f79b694b65429p+18 && v[3] == 0x1.3ebb8c72be42bp+22);
  shortest_line(480807.4590033934, 5231322.627839951, 574.2851713985305,
    "Linestring(480302.328 5231763.056,479910.863891441 5231268.2026332)", v);
  assert(v[0] == 0x1.d5194401022dcp+18 && v[1] == 0x1.3f50fbb1875dbp+22);
  assert(v[2] == 0x1.d5069d6586a2cp+18 && v[3] == 0x1.3f51e7c2609a2p+22);

  printf("shortestline_nearest_test: every constructed end answered as its "
    "closed form\n");
  meos_finalize();
  return 0;
}
