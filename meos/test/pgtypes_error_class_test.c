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
 * @brief A program that tests that an error of the PostgreSQL code of MEOS
 * reports the error code of the class PostgreSQL gives it
 * @details Each operation is called once with an argument PostgreSQL refuses,
 * which must report through meos_errno the MEOS code of the class of the
 * errcode PostgreSQL raises with the same message, and once with an argument
 * it accepts, which must report none: a division by zero reports
 * #MEOS_ERR_DIVISION_BY_ZERO, the factorial of a negative number
 * #MEOS_ERR_VALUE_OUT_OF_RANGE, the difference of an infinite date and a bin
 * stride of zero #MEOS_ERR_DATETIME_OUT_OF_RANGE, a NaN converted to an
 * integer, a bin stride of months and a truncation unit a timestamp does not
 * have #MEOS_ERR_FEATURE_NOT_SUPPORTED, and a field name no timestamp has
 * #MEOS_ERR_INVALID_ARG_VALUE.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o pgtypes_error_class_test pgtypes_error_class_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_date.h>
#include <pg_float.h>
#include <pg_int.h>
#include <pg_interval.h>
#include <pg_numeric.h>
#include <pg_text.h>
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

  const int DIV = MEOS_ERR_DIVISION_BY_ZERO;
  const int NUM = MEOS_ERR_VALUE_OUT_OF_RANGE;
  const int DT = MEOS_ERR_DATETIME_OUT_OF_RANGE;
  const int FEAT = MEOS_ERR_FEATURE_NOT_SUPPORTED;
  const int ARG = MEOS_ERR_INVALID_ARG_VALUE;

  /* Division of integers, big integers and floats */
  div_int32_int32(1, 0); check("int32 1 / 0", DIV);
  div_int32_int32(4, 2); check("int32 4 / 2", 0);
  int32_mod(1, 0); check("int32 1 % 0", DIV);
  int32_mod(5, 2); check("int32 5 % 2", 0);
  div_int64_int64(1, 0); check("int64 1 / 0", DIV);
  div_int64_int64(4, 2); check("int64 4 / 2", 0);
  int64_mod(1, 0); check("int64 1 % 0", DIV);
  int64_mod(5, 2); check("int64 5 % 2", 0);
  div_float8_float8(1.0, 0.0); check("float8 1 / 0", DIV);
  div_float8_float8(1.0, 2.0); check("float8 1 / 2", 0);

  /* Division of numerics and intervals */
  Numeric one = numeric_in("1", -1);
  Numeric zero = numeric_in("0", -1);
  Numeric n = numeric_div(one, zero); check("numeric 1 / 0", DIV); free(n);
  n = numeric_div(one, one); check("numeric 1 / 1", 0); free(n);
  n = numeric_mod(one, zero); check("numeric 1 % 0", DIV); free(n);
  n = numeric_mod(one, one); check("numeric 1 % 1", 0); free(n);
  Interval *iday = interval_in("1 day", -1);
  Interval *i = div_interval_float8(iday, 0.0);
  check("interval 1 day / 0", DIV); free(i);
  i = div_interval_float8(iday, 2.0);
  check("interval 1 day / 2", 0); free(i);

  /* Factorial of a negative number */
  n = numeric_fac(-1); check("numeric_fac(-1)", NUM); free(n);
  n = numeric_fac(3); check("numeric_fac(3)", 0); free(n);

  /* Conversion of a NaN to an integer */
  Numeric nan = numeric_in("NaN", -1);
  numeric_to_int32(nan); check("numeric NaN to int32", FEAT);
  numeric_to_int32(one); check("numeric 1 to int32", 0);

  /* Difference of an infinite date */
  DateADT dinf = date_in("infinity");
  DateADT d = date_in("2001-01-01");
  minus_date_date(dinf, d); check("date infinity - 2001-01-01", DT);
  minus_date_date(d, d); check("date 2001-01-01 - 2001-01-01", 0);

  /* Binning of timestamps */
  Timestamp t = timestamp_in("2001-01-01 12:34:56", -1);
  Timestamp origin = timestamp_in("2001-01-01", -1);
  Interval *izero = interval_in("0 seconds", -1);
  Interval *imonth = interval_in("1 month", -1);
  Interval *iquarter = interval_in("15 minutes", -1);
  timestamp_bin(t, izero, origin); check("timestamp_bin stride 0", DT);
  timestamp_bin(t, imonth, origin); check("timestamp_bin stride 1 month", FEAT);
  timestamp_bin(t, iquarter, origin); check("timestamp_bin stride 15 minutes", 0);

  /* Truncation and field of a timestamp */
  text *ttz = cstring_to_text("timezone");
  text *tfoo = cstring_to_text("foo");
  text *tyear = cstring_to_text("year");
  timestamp_trunc(t, ttz); check("timestamp_trunc to timezone", FEAT);
  timestamp_trunc(t, tyear); check("timestamp_trunc to year", 0);
  timestamp_part(t, tfoo); check("timestamp_part foo", ARG);
  timestamp_part(t, tyear); check("timestamp_part year", 0);

  free(one); free(zero); free(nan); free(iday); free(izero); free(imonth);
  free(iquarter); free(ttz); free(tfoo); free(tyear);
  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
