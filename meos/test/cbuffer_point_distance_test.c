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
 * @brief The nearest approach distance of a circular buffer to a point,
 * judged by the closed form
 *
 * @details The distance of a disc of centre c and radius r to a point p is
 * `max(0, sqrt(S) - r)`, `S` the squared distance of c and p, and the answer
 * is the double nearest it: the double `d` for which `S` lies strictly between
 * `(r + d - g1)^2` and `(r + d + g2)^2`, `g1` and `g2` the half gaps to its
 * neighbours. Each expected value below is that double, found in exact
 * rational arithmetic from the input doubles and written in hexadecimal:
 * - a Pythagorean offset at projected coordinates answers its whole number;
 * - the offsets (1000, 1000), (1, 1) and (1, 2) answer `sqrt(S) - r` rounded
 *   once, at projected coordinates and near the origin;
 * - a point inside the disc answers 0.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_point_distance_test \
 *   cbuffer_point_distance_test.c -L/usr/local/lib -lmeos -lm
 * ./cbuffer_point_distance_test
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/* The nearest approach distance of the disc of centre (x1 y1) and radius r,
 * at rest at one instant, to the point (x2 y2), the disc built as #relate of
 * tcbuffer_contact_test.c builds it */
static double
disc_point_distance(double x1, double y1, double r, double x2, double y2)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-01",
    x1, y1, r);
  Temporal *temp = tcbuffer_in(buf);
  snprintf(buf, sizeof(buf), "Point(%.17g %.17g)", x2, y2);
  GSERIALIZED *gs = geom_in(buf, -1);
  assert(temp != NULL && gs != NULL);
  double d = nad_tcbuffer_geo(temp, gs);
  free(temp); free(gs);
  return d;
}

int
main(void)
{
  meos_initialize();

  /* (3, 4) apart at projected coordinates, radius 1 */
  assert(disc_point_distance(500000.0, 4000000.0, 1.0, 500003.0, 4000004.0) ==
    0x1.0000000000000p+2);
  /* (1000, 1000) apart, radius 250 */
  assert(disc_point_distance(612345.5, 5432101.25, 250.0, 613345.5,
    5433101.25) == 0x1.230dab0184066p+10);
  /* (1, 1) apart near the origin, radius 0.5 */
  assert(disc_point_distance(0.0, 0.0, 0.5, 1.0, 1.0) ==
    0x1.d413cccfe7799p-1);
  /* (1, 2) apart at projected coordinates, radius 0.1 */
  assert(disc_point_distance(700000.125, 6000000.0, 0.1, 700001.125,
    6000002.0) == 0x1.116aacecb27dbp+1);
  /* 0.5 apart, radius 1: inside */
  assert(disc_point_distance(1000.0, 1000.0, 1.0, 1000.0, 1000.5) == 0.0);

  printf("cbuffer_point_distance_test: every distance answered as its closed "
    "form\n");
  meos_finalize();
  return 0;
}
