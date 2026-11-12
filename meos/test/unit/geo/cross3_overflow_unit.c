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
 * @brief Test of the signs in 3D whose cross products overflow the doubles,
 * #vector_triple_sign and #geom_dwithin3d
 * @details A face a = 0, b = (X, -X, 0), c = (X, X, 2X) and a segment from
 * (3X/4, X/4, X/4) to (X/4, -X/4, 3X/4), which crosses the face at
 * (X/2, 0, X/2), a point inside it at the weights (1/2, 1/4, 1/4), so the two
 * are within any distance at every scale X. From X = 2^512 on, every product
 * of two components of `(c - a) x (p - a)` is past the largest double, and the
 * two of each component are infinities of one sign whose difference is not
 * zero. Each scale is a power of two, so every coordinate is exact on doubles
 * and an answer depends on no magnitude. The orientations are rows whose
 * exact sign is known, read with the vectors scaled by 2^-600, exactly, where
 * no product overflows.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>
#include <meos_internal_geo.h>
#include "geo/geo_funcs.h"

static int failures = 0;
static int checks = 0;

static void
check(bool ok, const char *what, int e)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAIL %s at 2^%d\n", what, e);
  }
  return;
}

/* The face and the segment through its inside at the scale 2^e */
static void
face_segment_case(int e)
{
  double X = ldexp(1.0, e);
  char face[512], seg[256];
  snprintf(face, sizeof(face),
    "POLYGON Z((0 0 0,%.17g %.17g 0,%.17g %.17g %.17g,0 0 0))",
    X, - X, X, X, 2 * X);
  snprintf(seg, sizeof(seg), "LINESTRING Z(%.17g %.17g %.17g,%.17g %.17g %.17g)",
    3 * X / 4, X / 4, X / 4, X / 4, - X / 4, 3 * X / 4);
  GSERIALIZED *g1 = geom_in(face, -1), *g2 = geom_in(seg, -1);
  check(geom_dwithin3d(g1, g2, 0.0), "the segment crossing the face", e);
  check(geom_dwithin3d(g2, g1, 0.0), "the face crossed by the segment", e);
  free(g1); free(g2);
  return;
}

/* The orientation of the origin and three points whose products of two
 * coordinates overflow, against its sign on the same row scaled by 2^-600 */
static void
orientation_case(const double *p, const double *q, const double *r, int e)
{
  static const double origin[3] = {0.0, 0.0, 0.0};
  double ps[3], qs[3], rs[3];
  for (int k = 0; k < 3; k++)
  {
    ps[k] = ldexp(p[k], -600);
    qs[k] = ldexp(q[k], -600);
    rs[k] = ldexp(r[k], -600);
  }
  int scaled = vector_triple_sign(origin, ps, origin, qs, origin, rs);
  int s = vector_triple_sign(origin, p, origin, q, origin, r);
  check(scaled != 0 && s == scaled, "the orientation of an overflowing row",
    e);
  return;
}

int
main(void)
{
  meos_initialize();
  const int exps[] = {0, 300, 511, 512, 600, 1000};
  for (size_t i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
    face_segment_case(exps[i]);
  /* The face above, at 2^600: (b - a) . ((c - a) x (p - a)) is X^3 / 2 */
  const double X = 0x1p600;
  const double b[3] = {X, - X, 0.0}, c[3] = {X, X, 2 * X},
    p[3] = {3 * X / 4, X / 4, X / 4};
  orientation_case(b, c, p, 600);
  /* Three vectors near 2^600 in general position */
  const double u[3] = {-2.6267359220168972e+180, 9.9464125646531423e+179,
    -4.6456345885805699e+179};
  const double v[3] = {3.2837820861306462e+180, 1.4831059174175745e+180,
    3.0381484738968499e+180};
  const double w[3] = {-1.4922415380699889e+180, -3.3055674716404194e+180,
    -5.8242174528491677e+179};
  orientation_case(u, v, w, 600);
  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
