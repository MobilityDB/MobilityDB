/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief A simple program that tests the interval functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o interval_test interval_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <meos.h>
#include <pg_bool.h>
#include <pg_date.h>
#include <pg_interval.h>
#include <pg_numeric.h>
#include <pg_text.h>
#include <pg_timestamp.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */
  Interval *interv_in1 = interval_in("3 months", -1);
  Interval *interv_in2 = interval_in("3 weeks", -1);
  char *interv_out1 = interval_out(interv_in1);
  char *interv_out2 = interval_out(interv_in2);

  /* Create the result types for the bool functions of the API */
  bool bool_result;
  int32_t int32_result;
  uint32_t uint32_result;
  uint64_t uint64_result;
  double float8_result;
  char *char_result;
  Interval *interv_result;
  Numeric numeric_result;
  
  /* Execute and print the result for the bool functions of the API */

  printf("****************************************************************\n");
  printf("* Interval *\n");
  printf("****************************************************************\n");

  /* Interval *add_interval_interval(const Interval *interv1, const Interval *interv2); */
  interv_result = add_interval_interval(interv_in1, interv_in2);
  char_result = interval_out(interv_result);
  printf("add_interval_interval(%s, %s): %s\n", interv_out1, interv_out2, char_result);
  free(interv_result); free(char_result);

  /* Interval *div_interval_float8(const Interval *interv, double factor); */
  interv_result = div_interval_float8(interv_in1, 5);
  char_result = interval_out(interv_result);
  printf("div_interval_float8(%s, 5): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* int32_t interval_cmp(const Interval *interv1, const Interval *interv2); */
  int32_result = interval_cmp(interv_in1, interv_in2);
  printf("interval_cmp(%s, %s): %d\n", interv_out1, interv_out2, int32_result);

  /* Interval *interval_copy(const Interval *interv); */
  interv_result = interval_copy(interv_in1);
  char_result = interval_out(interv_result);
  printf("interval_copy(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* bool interval_eq(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_eq(interv_in1, interv_in2);
  printf("interval_eq(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* Numeric interval_extract(const Interval *interv, const text *units); */
  text *units = text_in("seconds");
  numeric_result = interval_extract(interv_in1, units);
  char_result = numeric_out(numeric_result);
  printf("interval_extract(%s, \"seconds\"): %s\n", interv_out1, char_result);
  free(units); free(numeric_result); free(char_result);

  /* bool interval_ge(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_ge(interv_in1, interv_in2);
  printf("interval_ge(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* bool interval_gt(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_gt(interv_in1, interv_in2);
  printf("interval_gt(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* uint32_t interval_hash(const Interval *interv); */
  uint32_result = interval_hash(interv_in1);
  printf("interval_hash(%s): %u\n", interv_out1, uint32_result);

  /* uint64_t interval_hash_extended(const Interval *interv, uint64_t seed); */
  uint64_result = interval_hash_extended(interv_in1, 1);
  printf("interval_hash_extended(%s): %lu\n", interv_out1, uint64_result);

  /* Interval *interval_in(const char *str, int32_t typmod); */
  interv_result = interval_in("3 hours", -1);
  char_result = interval_out(interv_result);
  printf("interval_in(\"3 hours\", -1): %s\n", char_result);
  free(interv_result); free(char_result);

  /* bool interval_is_finite(const Interval *interv); */
  bool_result = interval_is_finite(interv_in1);
  printf("interval_is_finite(%s): %c\n", interv_out1, bool_result ? 't' : 'f');

  /* Interval *interval_justify_days(const Interval *interv); */
  interv_result = interval_justify_days(interv_in1);
  char_result = interval_out(interv_result);
  printf("interval_justify_days(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* Interval *interval_justify_hours(const Interval *interv); */
  interv_result = interval_justify_hours(interv_in1);
  char_result = interval_out(interv_result);
  printf("interval_justify_hours(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* Interval *interval_justify_interval(const Interval *interv); */
  interv_result = interval_justify_interval(interv_in1);
  char_result = interval_out(interv_result);
  printf("interval_justify_interval(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* Interval *interval_larger(const Interval *interv1, const Interval *interv2); */
  interv_result = interval_larger(interv_in1, interv_in2);
  char_result = interval_out(interv_result);
  printf("interval_larger(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* bool interval_le(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_le(interv_in1, interv_in2);
  printf("interval_le(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* bool interval_lt(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_lt(interv_in1, interv_in2);
  printf("interval_lt(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* Interval *interval_make(int32_t years, int32_t months, int32_t weeks, int32_t days, int32_t hours, int32_t mins, double secs); */
  interv_result = interval_make(2025, 3, 1, 4, 8, 0, 0);
  char_result = interval_out(interv_result);
  printf("interval_make(2025, 3, 1, 4, 8, 0, 0): %s\n", char_result);
  free(interv_result); free(char_result);

  /* bool interval_ne(const Interval *interv1, const Interval *interv2); */
  bool_result = interval_ne(interv_in1, interv_in2);
  printf("interval_ne(%s, %s): %c\n", interv_out1, interv_out2, bool_result ? 't' : 'f');

  /* Interval *interval_negate(const Interval *interv); */
  interv_result = interval_negate(interv_in1);
  char_result = interval_out(interv_result);
  printf("interval_negate(%s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* char *interval_out(const Interval *interv); */
  char_result = interval_out(interv_in1);
  printf("interval_out(%s): %s\n", interv_out1, char_result);
  free(char_result);

  /* double interval_part(const Interval *interv, const text *units); */
  units = text_in("seconds");
  float8_result = interval_part(interv_in1, units);
  printf("interval_part(%s, \"seconds\"): %lf\n", interv_out1, float8_result);
  free(units);

  /* Interval *interval_scale(const Interval *interv, int32_t typmod); */
  /* The value below sets the precision to (0) */
  Interval *interv = interval_in("1 hour 1 minute 1.123456789 seconds", 2147418112);
  char_result = interval_out(interv);
  printf("interval_scale(\"1 hour 1 minute 1.123456789 seconds\", 2147418112): %s\n", char_result);
  free(interv); free(char_result);

  /* Interval *interval_smaller(const Interval *interv1, const Interval *interv2); */
  interv_result = interval_smaller(interv_in1, interv_in2);
  char_result = interval_out(interv_result);
  printf("interval_smaller(%s, %s): %s\n", interv_out1, interv_out2, char_result);
  free(interv_result); free(char_result);

  /* Interval *interval_trunc(const Interval *interv, const text *units); */
  units = text_in("seconds");
  interv_result = interval_trunc(interv_in1, units);
  char_result = interval_out(interv_result);
  printf("interval_trunc(%s, \"seconds\"): %s\n", interv_out1, char_result);
  free(units); free(interv_result); free(char_result);

  /* Interval *minus_interval_interval(const Interval *interv1, const Interval *interv2); */
  interv_result = minus_interval_interval(interv_in1, interv_in2);
  char_result = interval_out(interv_result);
  printf("minus_interval_interval(%s, %s): %s\n", interv_out1, interv_out2, char_result);
  free(interv_result); free(char_result);

  /* Interval *mul_float8_interval(double factor, const Interval *interv); */
  interv_result = mul_float8_interval(8, interv_in1);
  char_result = interval_out(interv_result);
  printf("mul_float8_interval(8, %s): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  /* Interval *mul_interval_float8(const Interval *interv, double factor); */
  interv_result = mul_interval_float8(interv_in1, 8);
  char_result = interval_out(interv_result);
  printf("mul_interval_float8(%s, 8): %s\n", interv_out1, char_result);
  free(interv_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(interv_in1); free(interv_in2);
  free(interv_out1); free(interv_out2);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
