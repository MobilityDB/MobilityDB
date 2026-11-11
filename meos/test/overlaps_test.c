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
 * @brief A program that tests that the overlaps functions of the time types
 * answer as the SQL OVERLAPS operator of PostgreSQL
 * @details Each case gives two intervals by their endpoints, in either order,
 * as microseconds from the PostgreSQL epoch, and the answer PostgreSQL 18
 * gives for `(s1, e1) OVERLAPS (s2, e2)` over timestamptz, timestamp, time and
 * timetz values at these instants: an interval given with its endpoints
 * reversed, intervals that touch, equal instants, and an instant inside or at
 * the end of an interval.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o overlaps_test overlaps_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <meos.h>
#include <pg_time.h>
#include <pg_timestamp.h>

static int failures;

/* Verify one answer against the answer of PostgreSQL; the twin of #check of
 * jsonpath_error_path_test.c */
static void
check(const char *what, int n, bool answer, bool expected)
{
  printf("%s case %d: %c\n", what, n, answer ? 't' : 'f');
  if (answer != expected)
  {
    printf("  expected %c\n", expected ? 't' : 'f');
    failures++;
  }
}

int main(void)
{
  meos_initialize();

  /* s1, e1, s2, e2, and the answer of PostgreSQL */
  static const struct { int s1, e1, s2, e2; bool expected; } cases[] = {
    {3, 1, 0, 2, true},   /* the first interval reversed */
    {0, 2, 3, 1, true},   /* the second interval reversed */
    {1, 3, 0, 2, true},
    {1, 2, 2, 3, false},  /* intervals that touch */
    {5, 5, 5, 5, true},   /* equal instants */
    {2, 2, 1, 3, true},   /* an instant inside an interval */
    {3, 3, 1, 3, false},  /* an instant at the end of an interval */
    {6, 5, 3, 0, false},  /* both reversed, apart */
    {3, 1, 3, 1, true},   /* both reversed, equal */
  };
  for (int i = 0; i < (int) (sizeof(cases) / sizeof(cases[0])); i++)
  {
    int s1 = cases[i].s1, e1 = cases[i].e1, s2 = cases[i].s2,
      e2 = cases[i].e2;
    bool expected = cases[i].expected;
    check("timestamptz_overlaps", i + 1,
      timestamptz_overlaps(s1, e1, s2, e2), expected);
    check("timestamp_overlaps", i + 1,
      timestamp_overlaps(s1, e1, s2, e2), expected);
    check("time_overlaps", i + 1, time_overlaps(s1, e1, s2, e2), expected);
    TimeTzADT ts1 = {s1, 0}, te1 = {e1, 0}, ts2 = {s2, 0}, te2 = {e2, 0};
    check("timetz_overlaps", i + 1,
      timetz_overlaps(&ts1, &te1, &ts2, &te2), expected);
  }

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? 1 : 0;
}
