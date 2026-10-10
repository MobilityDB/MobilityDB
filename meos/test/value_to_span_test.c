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
 * @brief A program that tests that the conversion of a value into a set, a
 * span and a span set keeps the value, for every base type of the set and
 * span types embedded in MEOS.
 *
 * Each conversion answers the value it is given, so its text output is
 * compared with the text of the expected set, span or span set. The values
 * reach the parts of their type a narrower one cannot hold: a big integer
 * beyond 32 bits on both sides of zero, a float with a fractional part, a
 * negative integer, a date and a timestamp with a time of day.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o value_to_span_test value_to_span_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

static int failures;

/* The text output of a conversion must be the expected one */
static void
check(const char *what, void *result, char *out, const char *expected)
{
  if (! out || strcmp(out, expected) != 0)
  {
    printf("FAIL %s: answered %s, expected %s\n", what, out ? out : "NULL",
      expected);
    failures++;
  }
  free(result);
  free(out);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  Set *s;
  Span *sp;
  SpanSet *ss;

  /* Integer */
  int i = -7;
  s = int_to_set(i);
  check("int_to_set(-7)", s, intset_out(s), "{-7}");
  sp = int_to_span(i);
  check("int_to_span(-7)", sp, intspan_out(sp), "[-7, -6)");
  ss = int_to_spanset(i);
  check("int_to_spanset(-7)", ss, intspanset_out(ss), "{[-7, -6)}");

  /* Big integer beyond 32 bits, positive and negative */
  int64 big = 5000000000LL;
  s = bigint_to_set(big);
  check("bigint_to_set(5000000000)", s, bigintset_out(s), "{5000000000}");
  sp = bigint_to_span(big);
  check("bigint_to_span(5000000000)", sp, bigintspan_out(sp),
    "[5000000000, 5000000001)");
  ss = bigint_to_spanset(big);
  check("bigint_to_spanset(5000000000)", ss, bigintspanset_out(ss),
    "{[5000000000, 5000000001)}");
  sp = bigint_to_span(-big);
  check("bigint_to_span(-5000000000)", sp, bigintspan_out(sp),
    "[-5000000000, -4999999999)");
  ss = bigint_to_spanset(-big);
  check("bigint_to_spanset(-5000000000)", ss, bigintspanset_out(ss),
    "{[-5000000000, -4999999999)}");

  /* Float with a fractional part */
  double d = 3.7;
  s = float_to_set(d);
  check("float_to_set(3.7)", s, floatset_out(s, 15), "{3.7}");
  sp = float_to_span(d);
  check("float_to_span(3.7)", sp, floatspan_out(sp, 15), "[3.7, 3.7]");
  ss = float_to_spanset(d);
  check("float_to_spanset(3.7)", ss, floatspanset_out(ss, 15),
    "{[3.7, 3.7]}");

  /* Date */
  DateADT date = date_in("2001-01-02");
  s = date_to_set(date);
  check("date_to_set(2001-01-02)", s, dateset_out(s), "{2001-01-02}");
  sp = date_to_span(date);
  check("date_to_span(2001-01-02)", sp, datespan_out(sp),
    "[2001-01-02, 2001-01-03)");
  ss = date_to_spanset(date);
  check("date_to_spanset(2001-01-02)", ss, datespanset_out(ss),
    "{[2001-01-02, 2001-01-03)}");

  /* Timestamp with a time of day */
  TimestampTz t = timestamptz_in("2001-01-02 08:30:00", -1);
  s = timestamptz_to_set(t);
  check("timestamptz_to_set", s, tstzset_out(s),
    "{\"2001-01-02 08:30:00+00\"}");
  sp = timestamptz_to_span(t);
  check("timestamptz_to_span", sp, tstzspan_out(sp),
    "[2001-01-02 08:30:00+00, 2001-01-02 08:30:00+00]");
  ss = timestamptz_to_spanset(t);
  check("timestamptz_to_spanset", ss, tstzspanset_out(ss),
    "{[2001-01-02 08:30:00+00, 2001-01-02 08:30:00+00]}");

  if (failures == 0)
    printf("Value to span test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
