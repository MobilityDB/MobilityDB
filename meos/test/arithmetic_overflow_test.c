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
 * @brief A program that tests that a result outside the range of its type
 * reports the error code of its class
 * @details Each operation is called once with a result outside the range of
 * its type, which must report through meos_errno the code PostgreSQL gives the
 * same error, and once with a result inside it, which must report none: a
 * number out of range reports #MEOS_ERR_VALUE_OUT_OF_RANGE and a date, a
 * timestamp or an interval out of range reports
 * #MEOS_ERR_DATETIME_OUT_OF_RANGE. The operations are the arithmetic of
 * temporal integers, big integers and floats, the conversion of temporal
 * floats to temporal integers and big integers, the shift of integer, date
 * and timestamp spans, the input of integers, big integers, floats, dates,
 * timestamps and intervals, and the addition of days to a date, of an interval
 * to a timestamp and of two intervals.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o arithmetic_overflow_test arithmetic_overflow_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_date.h>
#include <pg_float.h>
#include <pg_int.h>
#include <pg_interval.h>
#include <pg_timestamp.h>

static int failures;

/* Verify the error the last operation left, then clear it */
static void
check(const char *what, int expected)
{
  int err = meos_errno();
  printf("%s: error %d\n", what, err);
  if (err != expected)
  {
    printf("  expected error %d\n", expected);
    failures++;
  }
  meos_errno_reset();
}

int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const int NUM = MEOS_ERR_VALUE_OUT_OF_RANGE;
  const int DT = MEOS_ERR_DATETIME_OUT_OF_RANGE;

  Temporal *imax = tint_in("2147483646@2001-01-01");
  Temporal *imin = tint_in("-2147483648@2001-01-01");
  Temporal *bmax = tbigint_in("9223372036854775806@2001-01-01");
  Temporal *fbig = tfloat_in("1e300@2001-01-01");
  meos_errno_reset();
  Temporal *r;

  /* Arithmetic of temporal numbers */
  r = add_tint_int(imax, 2); check("tint (INT_MAX - 1) + 2", NUM); free(r);
  r = add_tint_int(imax, -1); check("tint (INT_MAX - 1) + -1", 0); free(r);
  r = sub_tint_int(imin, 1); check("tint INT_MIN - 1", NUM); free(r);
  r = sub_tint_int(imin, -1); check("tint INT_MIN - -1", 0); free(r);
  r = mul_tint_int(imax, 2); check("tint (INT_MAX - 1) * 2", NUM); free(r);
  r = mul_tint_int(imax, 1); check("tint (INT_MAX - 1) * 1", 0); free(r);
  r = div_tint_int(imin, -1); check("tint INT_MIN / -1", NUM); free(r);
  r = div_tint_int(imin, 1); check("tint INT_MIN / 1", 0); free(r);
  r = add_tbigint_bigint(bmax, 2); check("tbigint (INT64_MAX - 1) + 2", NUM); free(r);
  r = add_tbigint_bigint(bmax, -1); check("tbigint (INT64_MAX - 1) + -1", 0); free(r);
  r = mul_tbigint_bigint(bmax, 2); check("tbigint (INT64_MAX - 1) * 2", NUM); free(r);
  r = mul_tfloat_float(fbig, 1e300); check("tfloat 1e300 * 1e300", NUM); free(r);
  r = mul_tfloat_float(fbig, 2.0); check("tfloat 1e300 * 2", 0); free(r);

  /* Conversion of temporal floats */
  Temporal *fout = tfloat_in("3e9@2001-01-01");
  Temporal *fin = tfloat_in("-2147483648.5@2001-01-01");
  r = tfloat_to_tint(fout); check("tfloat 3e9 to tint", NUM); free(r);
  r = tfloat_to_tint(fin); check("tfloat INT_MIN - 0.5 to tint", 0); free(r);
  r = tfloat_to_tbigint(fbig); check("tfloat 1e300 to tbigint", NUM); free(r);
  r = tfloat_to_tbigint(fout); check("tfloat 3e9 to tbigint", 0); free(r);
  free(fout); free(fin);

  /* Shift of spans */
  Span *ispan = intspan_make(1, INT_MAX - 1000, true, false);
  Span *s = intspan_shift_scale(ispan, 2000, 0, true, false);
  check("intspan [1, INT_MAX - 1000) shifted by 2000", NUM); free(s);
  s = intspan_shift_scale(ispan, 10, 0, true, false);
  check("intspan [1, INT_MAX - 1000) shifted by 10", 0); free(s);
  Span *dspan = datespan_make(0, 10, true, false);
  s = datespan_shift_scale(dspan, INT_MAX - 5, 0, true, false);
  check("datespan shifted by INT_MAX - 5 days", DT); free(s);
  s = datespan_shift_scale(dspan, 10, 0, true, false);
  check("datespan shifted by 10 days", 0); free(s);
  Span *tspan = tstzspan_in("[2001-01-01, 2001-01-02]");
  Interval *ilong = interval_in("300000 years", -1);
  Interval *ishort = interval_in("1 year", -1);
  Interval *iday = interval_in("1 day", -1);
  s = tstzspan_shift_scale(tspan, ilong, iday);
  check("tstzspan shifted by 300000 years", DT); free(s);
  s = tstzspan_shift_scale(tspan, ishort, iday);
  check("tstzspan shifted by 1 year", 0); free(s);

  /* Input of numbers */
  int32_in("2147483648"); check("int32_in(\"2147483648\")", NUM);
  int32_in("2147483647"); check("int32_in(\"2147483647\")", 0);
  int64_in("9223372036854775808");
  check("int64_in(\"9223372036854775808\")", NUM);
  int64_in("9223372036854775807");
  check("int64_in(\"9223372036854775807\")", 0);
  float8_in("1e400"); check("float8_in(\"1e400\")", NUM);
  float8_in("1e300"); check("float8_in(\"1e300\")", 0);

  /* Input of dates, timestamps and intervals */
  date_in("5874898-01-01"); check("date_in(\"5874898-01-01\")", DT);
  date_in("5874897-12-31"); check("date_in(\"5874897-12-31\")", 0);
  timestamptz_in("294277-01-01 00:00:00+00", -1);
  check("timestamptz_in(\"294277-01-01 00:00:00+00\")", DT);
  timestamptz_in("294276-12-31 00:00:00+00", -1);
  check("timestamptz_in(\"294276-12-31 00:00:00+00\")", 0);
  Interval *i = interval_in("2147483648 days", -1);
  check("interval_in(\"2147483648 days\")", DT); free(i);
  i = interval_in("2147483647 days", -1);
  check("interval_in(\"2147483647 days\")", 0); free(i);

  /* Arithmetic of dates, timestamps and intervals */
  DateADT d = date_in("2001-01-01");
  add_date_int(d, INT_MAX - 5); check("date 2001-01-01 + (INT_MAX - 5) days", DT);
  add_date_int(d, 10); check("date 2001-01-01 + 10 days", 0);
  TimestampTz t = timestamptz_in("2001-01-01 00:00:00+00", -1);
  add_timestamptz_interval(t, ilong);
  check("timestamptz 2001-01-01 + 300000 years", DT);
  add_timestamptz_interval(t, ishort);
  check("timestamptz 2001-01-01 + 1 year", 0);
  Interval *ihuge = interval_in("178000000 years", -1);
  i = add_interval_interval(ihuge, ihuge);
  check("interval 178000000 years + 178000000 years", DT); free(i);
  i = add_interval_interval(ihuge, ishort);
  check("interval 178000000 years + 1 year", 0); free(i);

  free(imax); free(imin); free(bmax); free(fbig); free(ispan); free(dspan);
  free(tspan); free(ilong); free(ishort); free(iday); free(ihuge);
  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
