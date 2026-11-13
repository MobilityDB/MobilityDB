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
 * @brief Two planar temporal points that meet between two microseconds
 * @details A runs from (-h 0) to (h 0) and B from (0 -h) to (0 h) over the
 * same D microseconds, so both lie at (0 0) at D/2 and they meet for every D;
 * an odd D puts the meeting at a half microsecond. In 3D B also rises from
 * -h to h, and still meets A at the origin. The controls move B to x = 1e-6,
 * where the two never meet. #ever_eq_tgeo_tgeo and #eintersects_tgeo_tgeo
 * read time as continuous, so they answer true on every meeting and false on
 * every control.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_geo.h>
#include <pg_timestamp.h>

static int failures = 0;
static int checks = 0;

static void
check(bool ok, const char *what)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAIL: %s\n", what);
  }
}

/* The pair over D microseconds, B moved to x = off, in 3D where z is true */
static void
check_pair(double h, long long D, double off, bool z)
{
  char a[512], b[512], msg[256];
  size_t sz;
  TimestampTz t0 = timestamptz_in("2001-01-01 00:00:00+00", -1);
  char *s0 = timestamptz_out(t0, &sz), *s1 = timestamptz_out(t0 + D, &sz);
  if (z)
  {
    snprintf(a, sizeof(a), "[Point Z(%.17g 0 0)@%s, Point Z(%.17g 0 0)@%s]",
      -h, s0, h, s1);
    snprintf(b, sizeof(b), "[Point Z(%.17g %.17g %.17g)@%s, "
      "Point Z(%.17g %.17g %.17g)@%s]", off, -h, -h, s0, off, h, h, s1);
  }
  else
  {
    snprintf(a, sizeof(a), "[Point(%.17g 0)@%s, Point(%.17g 0)@%s]", -h, s0,
      h, s1);
    snprintf(b, sizeof(b), "[Point(%.17g %.17g)@%s, Point(%.17g %.17g)@%s]",
      off, -h, s0, off, h, s1);
  }
  Temporal *A = tgeompoint_in(a), *B = tgeompoint_in(b);
  int expect = (off == 0.0) ? 1 : 0;
  int eq = ever_eq_tgeo_tgeo(A, B), in = eintersects_tgeo_tgeo(A, B);
  snprintf(msg, sizeof(msg), "%s h=%g D=%lld %s: ever_eq %d, eintersects %d",
    z ? "3D" : "2D", h, D, off == 0.0 ? "meeting" : "control", eq, in);
  check(eq == expect && in == expect, msg);
  free(A); free(B); free(s0); free(s1);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  const double hs[3] = {0.1, 1, 10};
  const long long Ds[6] = {3, 1000001, 60000001, 3600000001LL,
    3600000003LL, 3600000000LL};
  for (int z = 0; z < 2; z++)
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 6; j++)
      {
        check_pair(hs[i], Ds[j], 0.0, z);
        check_pair(hs[i], Ds[j], 1e-6, z);
      }
  printf("%d/%d checks passed\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
