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
 * @brief Test of the exact crossings of float segments, #tfloatsegm_side,
 * #tnumbersegm_side, #tfloatsegm_crossing and #tnumbersegm_crossing
 * @details Each row states a segment whose exact crossing is known as a
 * rational: a crossing on a whole microsecond, a tie half-way between two
 * microseconds in both directions and from an odd and an even start, a
 * crossing whose nearest microsecond is a bound, two segments crossing at a
 * seventh of a day, a segment longer than 2^60 microseconds, and values
 * whose differences overflow the doubles.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <meos_internal.h>
#include "temporal/tsequence.h"

/* A day in microseconds */
#define DAY INT64CONST(86400000000)

static int failures = 0;
static int checks = 0;

static void
check(bool ok, const char *what)
{
  checks++;
  if (! ok)
  {
    failures++;
    printf("FAIL %s\n", what);
  }
  return;
}

/* A crossing of a segment with a value, expected at @p expected, or none
 * where @p expected is -1 */
static void
value_case(double start, double end, double value, TimestampTz lower,
  TimestampTz upper, TimestampTz expected, const char *what)
{
  TimestampTz t = -1;
  bool found = tfloatsegm_crossing(start, end, value, lower, upper, &t);
  check(expected < 0 ? ! found : (found && t == expected), what);
  if (found && expected >= 0)
  {
    /* The two neighbours of the instant lie on either side of the value */
    int below = tfloatsegm_side(start, end, value, lower, upper, t - 1);
    int above = tfloatsegm_side(start, end, value, lower, upper, t + 1);
    check(below != 0 && below == - above, what);
  }
  return;
}

/* A crossing of two segments, expected at @p expected, or none where
 * @p expected is -1 */
static void
segm_case(double start1, double end1, double start2, double end2,
  TimestampTz lower, TimestampTz upper, TimestampTz expected,
  const char *what)
{
  TimestampTz t = -1;
  bool found = tnumbersegm_crossing(start1, end1, start2, end2, lower, upper,
    &t);
  check(expected < 0 ? ! found : (found && t == expected), what);
  /* The order of the operands does not change the instant */
  TimestampTz t2 = -1;
  bool found2 = tnumbersegm_crossing(start2, end2, start1, end1, lower, upper,
    &t2);
  check(found2 == found && (! found || t2 == t), what);
  return;
}

int
main(void)
{
  meos_initialize();

  /* 0.7 of a day, 60479999999.99999 us when computed from the fraction */
  value_case(0.0, 10.0, 7.0, 0, DAY, INT64CONST(60480000000),
    "a crossing on a whole microsecond");
  check(tfloatsegm_side(0.0, 10.0, 7.0, 0, DAY, INT64CONST(60480000000)) == 0,
    "the side at a crossing on a whole microsecond");
  /* 1.5 us from the start: a tie, which goes to the even timestamp 2 */
  value_case(0.0, 1.0, 0.5, 0, 3, 2, "a rising tie from an even start");
  value_case(1.0, 0.0, 0.5, 0, 3, 2, "a falling tie from an even start");
  value_case(0.0, 1.0, 0.5, 1, 4, 2, "a rising tie from an odd start");
  value_case(1.0, 0.0, 0.5, 1, 4, 2, "a falling tie from an odd start");
  /* A tie whose even timestamp is a bound belongs to the bound */
  value_case(0.0, 1.0, 0.25, 0, 2, -1, "a tie at the lower bound");
  /* 0.0000096 us before the end */
  value_case(2.0, 1.0, 1.0000000000000002, 0, DAY, -1,
    "a crossing nearest to the upper bound");
  /* 0.0864 us and 0.864 us after the start */
  value_case(0.0, 1.0, 1e-12, 0, DAY, -1,
    "a crossing nearest to the lower bound");
  value_case(0.0, 1.0, 1e-11, 0, DAY, 1,
    "a crossing nearest to the first microsecond");
  value_case(0.0, 1.0, 0.0, 0, DAY, -1, "a value at a bound");
  value_case(0.0, 1.0, 2.0, 0, DAY, -1, "a value outside the segment");
  /* Differences of the values past the largest double */
  value_case(-1e308, 1e308, 0.0, 0, 3, 2, "a tie of overflowing values");
  /* 2^59 + 1/2 us from the start of a segment of 2^60 + 1 us */
  TimestampTz big = INT64CONST(1) << 60;
  value_case(0.0, 1.0, 0.5, 0, big + 1, big / 2, "a tie on a long segment");
  value_case(0.0, 1.0, 0.5, 1, big + 2, big / 2 + 2,
    "a tie on a long segment from an odd start");

  /* d / 7 = 12342857142.857... us from the start */
  segm_case(0.0, 6.0, 1.0, 0.0, 0, DAY, INT64CONST(12342857143),
    "two segments crossing");
  check(tnumbersegm_side(0.0, 6.0, 1.0, 0.0, 0, DAY,
      INT64CONST(12342857142)) == -1 &&
    tnumbersegm_side(0.0, 6.0, 1.0, 0.0, 0, DAY,
      INT64CONST(12342857143)) == 1, "the sides of two segments crossing");
  /* The same segment reversed meets it at the middle of the day */
  segm_case(78.098185047553017, 77.460573568223325, 77.460573568223325,
    78.098185047553017, 0, DAY, DAY / 2, "a segment and its reverse");
  segm_case(0.0, 1.0, 1.0, 2.0, 0, DAY, -1, "two parallel segments");
  segm_case(0.0, 1.0, 0.0, 2.0, 0, DAY, -1, "two segments equal at a bound");

  printf("%d of %d checks pass\n", checks - failures, checks);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
