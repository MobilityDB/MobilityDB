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
 * @brief A simple program that tests the meeting of two temporal geography
 * points
 * @details Two temporal geography points meet on a segment where the two
 * points they construct along their great circles at the instant lie within
 * the rounding of that construction. The program reads `teq` and `ever_eq` on
 * the pair of the tests of PyMEOS, the same arc in opposite directions, on
 * mirror images across the equator, which meet at 12:00, and on that mirror
 * pair with one end moved by 1e-6 degrees, which never meets.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geodetic_meeting_test geodetic_meeting_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static const struct
{
  const char *temp1;
  const char *temp2;
  const char *expected;
} PAIRS[] = {
  /* The same arc in opposite directions meets at its middle */
  {"[Point(1 1)@2019-09-01, Point(2 2)@2019-09-02]",
   "[Point(2 2)@2019-09-01, Point(1 1)@2019-09-02, Point(1 1)@2019-09-03]",
   "{[f@2019-09-01 00:00:00+00, t@2019-09-01 12:00:00+00], "
     "(f@2019-09-01 12:00:00+00, f@2019-09-02 00:00:00+00]}"},
  /* Mirror images across the equator meet on it at 12:00 */
  {"[Point(10 3)@2019-09-01, Point(12 -3)@2019-09-02]",
   "[Point(10 -3)@2019-09-01, Point(12 3)@2019-09-02]",
   "{[f@2019-09-01 00:00:00+00, t@2019-09-01 12:00:00+00], "
     "(f@2019-09-01 12:00:00+00, f@2019-09-02 00:00:00+00]}"},
  /* The mirror pair with one end moved by 1e-6 degrees never meets */
  {"[Point(10 3)@2019-09-01, Point(12 -3)@2019-09-02]",
   "[Point(10 -3)@2019-09-01, Point(12.000001 3)@2019-09-02]",
   "[f@2019-09-01 00:00:00+00, f@2019-09-02 00:00:00+00]"},
};

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int failures = 0, count = 0;
  for (size_t i = 0; i < sizeof(PAIRS) / sizeof(PAIRS[0]); i++)
  {
    Temporal *t1 = tgeogpoint_in(PAIRS[i].temp1);
    Temporal *t2 = tgeogpoint_in(PAIRS[i].temp2);
    Temporal *res = teq_temporal_temporal(t1, t2);
    size_t size;
    char *out = res ? temporal_out(res, 6, &size) : NULL;
    int ever = ever_eq_temporal_temporal(t1, t2);
    bool meet = strstr(PAIRS[i].expected, "t@") != NULL;
    if (! out || strcmp(out, PAIRS[i].expected) != 0 || ever != (meet ? 1 : 0))
    {
      printf("FAILED: pair %zu: teq %s, ever_eq %d, expected %s\n", i + 1,
        out ? out : "NULL", ever, PAIRS[i].expected);
      failures++;
    }
    else
      printf("OK: pair %zu: teq %s, ever_eq %d\n", i + 1, out, ever);
    count++;
    free(out); free(res); free(t1); free(t2);
  }

  meos_finalize();
  printf("geodetic_meeting_test: %d cases, %d failed\n", count, failures);
  return failures ? 1 : 0;
}
