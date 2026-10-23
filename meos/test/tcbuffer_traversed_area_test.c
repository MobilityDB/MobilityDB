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
 * @brief The traversed area of a temporal circular buffer as one geometry,
 * judged by the closed form at points whose answer it settles
 *
 * @details A circular buffer moving linearly from (c1, r1) to (c2, r2) covers
 * the point q at some instant exactly where |q - c(t)|^2 - r(t)^2 <= 0 for
 * some t in [0, 1], with c(t) and r(t) moving linearly. Every input below is
 * a dyadic rational, and every point asked about stands clear of the
 * boundary, so each expected answer follows from that inequality by hand.
 *
 * Two cases carry the defects an exact witness found:
 * - a buffer that does not move, whose answer is one whole circle: the box
 *   serialized with it holds the whole disc, so a predicate reading the box
 *   first finds its centre;
 * - a line a buffer of radius zero draws back over itself, through the point
 *   it started from, then a buffer growing from its end: the crossings of
 *   that line with the circle stand where they are, so the disc is covered
 *   once and its centre lies inside.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o tcbuffer_traversed_area_test \
 *   tcbuffer_traversed_area_test.c -L/usr/local/lib -lmeos -lm
 * ./tcbuffer_traversed_area_test
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* Whether the traversed area as one geometry holds the point */
static bool
holds(const GSERIALIZED *ta, double x, double y)
{
  char wkt[128];
  snprintf(wkt, sizeof(wkt), "POINT(%.17g %.17g)", x, y);
  GSERIALIZED *p = geom_in(wkt, -1);
  assert(p != NULL);
  bool result = geom_intersects2d(ta, p);
  free(p);
  return result;
}

/* The traversed area as one geometry, which the exact sweep answers */
static GSERIALIZED *
area(const char *tcbuffer)
{
  Temporal *temp = tcbuffer_in(tcbuffer);
  assert(temp != NULL);
  GSERIALIZED *ta = tcbuffer_traversed_area(temp, true);
  assert(ta != NULL);
  free(temp);
  return ta;
}

int
main(void)
{
  meos_initialize();

  /* A buffer that does not move: one whole circle, its centre inside, also
   * at projected coordinates */
  GSERIALIZED *ta = area("[Cbuffer(Point(1 1),0.5)@2001-01-01, "
    "Cbuffer(Point(1 1),0.5)@2001-01-02]");
  assert(holds(ta, 1.0, 1.0));
  assert(holds(ta, 1.0, 1.25));
  assert(! holds(ta, 1.625, 1.0));
  free(ta);
  ta = area("[Cbuffer(Point(6300000.25 6300000.5),1.5)@2001-01-01, "
    "Cbuffer(Point(6300000.25 6300000.5),1.5)@2001-01-02]");
  assert(holds(ta, 6300000.25, 6300000.5));
  assert(! holds(ta, 6300002.0, 6300000.5));
  free(ta);

  /* A buffer moving along x: the band |y| <= 1 between its two ends and the
   * two half discs at them */
  ta = area("[Cbuffer(Point(0 0),1)@2001-01-01, "
    "Cbuffer(Point(10 0),1)@2001-01-02]");
  assert(holds(ta, 5.0, 0.875));
  assert(! holds(ta, 5.0, 1.125));
  assert(holds(ta, 10.5, 0.0));
  assert(! holds(ta, -1.125, 0.0));
  free(ta);

  /* A buffer growing from a point: at x = 5 the half width is
   * 5 * 5 / sqrt(100 - 25) = 2.8867..., inside below and outside above */
  ta = area("[Cbuffer(Point(0 0),0)@2001-01-01, "
    "Cbuffer(Point(10 0),5)@2001-01-02]");
  assert(holds(ta, 5.0, 2.875));
  assert(! holds(ta, 5.0, 2.90625));
  free(ta);

  /* A buffer of radius zero draws a line, which holds its own points */
  ta = area("[Cbuffer(Point(0 0),0)@2001-01-01, "
    "Cbuffer(Point(2 2),0)@2001-01-02]");
  assert(holds(ta, 1.0, 1.0));
  assert(! holds(ta, 1.0, 1.125));
  free(ta);

  /* A line drawn back over itself through the point it started from, then a
   * buffer growing from its end to radius 0.75 at (0.5, 2.5): the disc holds
   * its centre, the line beyond the disc holds its point (2.125, 1.875), and
   * a point away from both lies outside. The sequence read with the instant
   * the normalization removes, the line drawn as two moves, answers alike */
  const char *seqs[2] = {
    "[Cbuffer(Point(1.25 2),0)@2001-01-01, Cbuffer(Point(3 1.75),0)@2001-01-02, "
    "Cbuffer(Point(-0.5 2.25),0)@2001-01-04, "
    "Cbuffer(Point(0.5 2.5),0.75)@2001-01-05]",
    "[Cbuffer(Point(1.25 2),0)@2001-01-01, Cbuffer(Point(3 1.75),0)@2001-01-02, "
    "Cbuffer(Point(1.25 2),0)@2001-01-03, Cbuffer(Point(-0.5 2.25),0)@2001-01-04, "
    "Cbuffer(Point(0.5 2.5),0.75)@2001-01-05]" };
  for (int k = 0; k < 2; k++)
  {
    ta = area(seqs[k]);
    assert(holds(ta, 0.5, 2.5));
    assert(holds(ta, 0.75, 2.75));
    assert(holds(ta, 2.125, 1.875));
    assert(! holds(ta, 2.0, 2.5));
    free(ta);
  }

  meos_finalize();
  printf("tcbuffer_traversed_area_test: every point answered as the closed form "
    "settles it\n");
  return 0;
}
