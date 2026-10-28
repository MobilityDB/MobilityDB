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
 * @brief The relationships of two circular buffers whose squared distances
 * leave the range of a double, judged by the whole numbers
 * @details The relationships read the sign of the squared distance between
 * the centres less the square of a sum of the radii. Where the coordinates
 * are subnormal, those squares lie below the smallest double, and where they
 * exceed 2^512, beyond the largest; each case below states its sign on whole
 * multiples of a power of two.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>

static Cbuffer *
disc(double x, double y, double r)
{
  char buf[256];
  snprintf(buf, sizeof(buf), "Cbuffer(Point(%.17g %.17g),%.17g)", x, y, r);
  return cbuffer_in(buf);
}

/* The five relationships of two discs that do not meet */
static void
apart(Cbuffer *cb1, Cbuffer *cb2)
{
  assert(cb1 != NULL && cb2 != NULL);
  assert(intersects_cbuffer_cbuffer(cb1, cb2) == 0);
  assert(disjoint_cbuffer_cbuffer(cb1, cb2) == 1);
  assert(touches_cbuffer_cbuffer(cb1, cb2) == 0);
  assert(covers_cbuffer_cbuffer(cb1, cb2) == 0);
  assert(contains_cbuffer_cbuffer(cb1, cb2) == 0);
  free(cb1); free(cb2);
}

int
main(void)
{
  meos_initialize();

  /* In units of 2^-1074: the disc of radius 1 about the origin and the
   * point (1 1), whose squared distance 2 exceeds the squared radius 1 */
  apart(disc(0.0, 0.0, 0x1p-1074), disc(0x1p-1074, 0x1p-1074, 0.0));
  /* In units of 2^-1073: radii 1 and 1 against centres (0 0) and (2 1),
   * squared distance 5 against 4 */
  apart(disc(0.0, 0.0, 0x1p-1073), disc(0x1p-1072, 0x1p-1073, 0x1p-1073));
  /* The same in units of 2^-1074, centres touching: squared distance 4 */
  Cbuffer *a = disc(0.0, 0.0, 0x1p-1074), *b = disc(0x1p-1073, 0.0, 0x1p-1074);
  assert(a != NULL && b != NULL);
  assert(touches_cbuffer_cbuffer(a, b) == 1);
  assert(intersects_cbuffer_cbuffer(a, b) == 1);
  free(a); free(b);

  /* In units of 2^1016: centres (-96 0) and (96 0), radii 80 and 88, so the
   * centres stand 192 apart against a sum of 168 */
  apart(disc(-96 * 0x1p1016, 0.0, 80 * 0x1p1016),
    disc(96 * 0x1p1016, 0.0, 88 * 0x1p1016));
  /* The same radii at centres (-80 0) and (88 0), touching */
  a = disc(-80 * 0x1p1016, 0.0, 80 * 0x1p1016);
  b = disc(88 * 0x1p1016, 0.0, 88 * 0x1p1016);
  assert(a != NULL && b != NULL);
  assert(touches_cbuffer_cbuffer(a, b) == 1);
  assert(intersects_cbuffer_cbuffer(a, b) == 1);
  free(a); free(b);

  printf("cbuffer_magnitude_test: every relationship answered as the whole "
    "numbers decide it\n");
  meos_finalize();
  return 0;
}
