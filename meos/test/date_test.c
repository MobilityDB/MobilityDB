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
 * @brief A simple program that tests the date functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o date_test date_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_date.h>
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
  DateADT date1 = date_in("2025-03-01");
  DateADT date2 = date_in("2025-04-01");
  char *date1_out = date_out(date1);
  char *date2_out = date_out(date2);
  TimeADT time1 = time_in("08:00:00", -1);
  char *time1_out = time_out(time1);
  Timestamp ts_in = timestamp_in("2025-03-01 08:00:00", -1);
  char *ts_out = timestamp_out(ts_in);
  Timestamp tstz_in = timestamptz_in("2025-03-01 08:00:00", -1);
  char *tstz_out = timestamptz_out(tstz_in);
  Interval *interv_in = interval_in("8 hours", -1);
  char *interv_out = interval_out(interv_in);

  /* Create the result types for the functions of the API */
  bool bool_result;
  int32_t int32_result;
  uint32_t uint32_result;
  uint64_t uint64_result;
  Numeric numeric_result;
  DateADT date_result;
  char *char_result;
  Timestamp ts_result;
  TimestampTz tstz_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Date *\n");
  printf("****************************************************************\n");

  /* DateADT add_date_int(DateADT date, int32_t days); */
  date_result = add_date_int(date1, 15);
  char_result = date_out(date_result);
  printf("add_date_int(%s, 15): %s\n", date1_out, char_result);
  free(char_result);

  /* DateADT add_date_interval(DateADT date, Interval *interv); */
  date_result = add_date_interval(date1, interv_in);
  char_result = date_out(date_result);
  printf("add_date_interval(%s, %s): %s\n", date1_out, interv_out, char_result);
  free(char_result);

  /* int32_t cmp_date_timestamp(DateADT date, Timestamp ts); */
  int32_result = cmp_date_timestamp(date1, tstz_in);
  printf("cmp_date_timestamp(%s, %s): %d\n", date1_out, tstz_out, int32_result);

  /* int cmp_date_date(DateADT date1, DateADT date2); */
  int32_result = cmp_date_date(date1, date2);
  printf("cmp_date_date(%s, %s): %d\n", date1_out, date2_out, int32_result);

  /* int32_t cmp_date_timestamptz(DateADT date, TimestampTz tstz); */
  int32_result = cmp_date_timestamptz(date1, tstz_in);
  printf("cmp_date_timestamptz(%s, %s): %d\n", date1_out, tstz_out, int32_result);

  /* int32_t cmp_timestamp_date(Timestamp ts, DateADT date); */
  int32_result = cmp_timestamp_date(ts_in, date1);
  printf("cmp_timestamp_date(%s, %s): %d\n", ts_out, date1_out, int32_result);

  /* int32_t cmp_timestamptz_date(TimestampTz tstz, DateADT date); */
  int32_result = cmp_timestamptz_date(tstz_in, date1);
  printf("cmp_timestamptz_date(%s, %s): %d\n", tstz_out, date1_out, int32_result);

  /* Numeric date_extract(DateADT date, const text *units); */
  text *units = text_in("days");
  numeric_result = date_extract(date1, units);
  char_result = numeric_out(numeric_result);
  printf("date_extract(%s, \"days\"): %s\n", date1_out, char_result);
  free(units); free(numeric_result); free(char_result);

  /* uint32_t date_hash(DateADT date); */
  uint32_result = date_hash(date1);
  printf("date_hash(%s): %ud\n", date1_out, uint32_result);

  /* uint64_t date_hash_extended(DateADT date, uint64_t seed); */
  uint64_result = date_hash_extended(date1, 1);
  printf("date_hash_extended(%s, 1): %lu\n", date1_out, uint64_result);

  /* DateADT date_in(const char *str); */
  date_result = date_in("2025-03-01");
  char_result = date_out(date_result);
  printf("date_in(\"2025-03-01\"): %s\n", char_result);
  free(char_result);

  /* bool date_is_finite(DateADT date); */
  bool_result = date_is_finite(date1);
  printf("date_is_finite(%s): %c\n", date1_out, bool_result ? 't' : 'f');

  /* DateADT date_larger(DateADT date1, DateADT date2); */
  date_result = date_larger(date1, date2);
  char_result = date_out(date_result);
  printf("date_larger(%s, %s): %s\n", date1_out, date2_out, char_result);
  free(char_result);

  /* DateADT date_make(int year, int mon, int mday); */
  date_result = date_make(2025, 3, 1);
  char_result = date_out(date_result);
  printf("date_make(2025, 3, 1): %s\n", char_result);
  free(char_result);

  /* char *date_out(DateADT date); */
  char_result = date_out(date1);
  printf("date_out(%s): %s\n", date1_out, char_result);
  free(char_result);

  /* DateADT date_smaller(DateADT date1, DateADT date2); */
  date_result = date_smaller(date1, date2);
  char_result = date_out(date_result);
  printf("date_smaller(%s, %s): %s\n", date1_out, date2_out, char_result);
  free(char_result);

  /* Timestamp date_time_to_timestamp(DateADT date, TimeADT time); */
  ts_result = date_time_to_timestamp(date1, time1);
  char_result = timestamp_out(ts_result);
  printf("date_time_to_timestamp(%s, %s): %s\n", date1_out, time1_out, char_result);
  free(char_result);

  /* Timestamp date_to_timestamp(DateADT date); */
  ts_result = date_to_timestamp(date1);
  char_result = timestamp_out(ts_result);
  printf("date_to_timestamp(%s): %s\n", date1_out, char_result);
  free(char_result);

  /* TimestampTz date_to_timestamptz(DateADT date); */
  tstz_result = date_to_timestamptz(date1);
  char_result = timestamptz_out(tstz_result);
  printf("date_to_timestamptz(%s): %s\n", date1_out, char_result);
  free(char_result);

  /* bool eq_date_date(DateADT date1, DateADT date2); */
  bool_result = eq_date_date(date1, date2);
  printf("eq_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool eq_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = eq_date_timestamp(date1, tstz_in);
  printf("eq_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool eq_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = eq_date_timestamptz(date1, tstz_in);
  printf("eq_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool eq_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = eq_timestamp_date(ts_in, date1);
  printf("eq_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool eq_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = eq_timestamptz_date(tstz_in, date1);
  printf("eq_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* bool ge_date_date(DateADT date1, DateADT date2); */
  bool_result = ge_date_date(date1, date2);
  printf("ge_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool ge_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = ge_date_timestamp(date1, tstz_in);
  printf("ge_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool ge_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = ge_date_timestamptz(date1, tstz_in);
  printf("ge_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool ge_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = ge_timestamp_date(ts_in, date1);
  printf("ge_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool ge_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = ge_timestamptz_date(tstz_in, date1);
  printf("ge_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* bool gt_date_date(DateADT date1, DateADT date2); */
  bool_result = gt_date_date(date1, date2);
  printf("gt_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool gt_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = gt_date_timestamp(date1, tstz_in);
  printf("gt_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool gt_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = gt_date_timestamptz(date1, tstz_in);
  printf("gt_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool gt_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = gt_timestamp_date(ts_in, date1);
  printf("gt_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool gt_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = gt_timestamptz_date(tstz_in, date1);
  printf("gt_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* bool le_date_date(DateADT date1, DateADT date2); */
  bool_result = le_date_date(date1, date2);
  printf("le_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool le_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = le_date_timestamp(date1, tstz_in);
  printf("le_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool le_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = le_date_timestamptz(date1, tstz_in);
  printf("le_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool le_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = le_timestamp_date(ts_in, date1);
  printf("le_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool le_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = le_timestamptz_date(tstz_in, date1);
  printf("le_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* bool lt_date_date(DateADT date1, DateADT date2); */
  bool_result = lt_date_date(date1, date2);
  printf("lt_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool lt_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = lt_date_timestamp(date1, tstz_in);
  printf("lt_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool lt_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = lt_date_timestamptz(date1, tstz_in);
  printf("lt_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool lt_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = lt_timestamp_date(ts_in, date1);
  printf("lt_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool lt_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = lt_timestamptz_date(tstz_in, date1);
  printf("lt_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* int32_t minus_date_date(DateADT date1, DateADT date2); */
  int32_result = minus_date_date(date1, date2);
  printf("minus_date_date(%s, %s): %d\n", date1_out, date2_out, int32_result);

  /* DateADT minus_date_int(DateADT date, int32_t days); */
  date_result = minus_date_int(date1, 3);
  char_result = date_out(date_result);
  printf("minus_date_int(%s, 3): %s\n", date1_out, char_result);
  free(char_result);

  /* DateADT minus_date_interval(DateADT date, Interval *span); */
  date_result = minus_date_interval(date1, interv_in);
  char_result = date_out(date_result);
  printf("minus_date_interval(%s, %s): %s\n", date1_out, interv_out, char_result);
  free(char_result);

  /* bool ne_date_date(DateADT date1, DateADT date2); */
  bool_result = ne_date_date(date1, date2);
  printf("ne_date_date(%s, %s): %c\n", date1_out, date2_out, bool_result ? 't' : 'f');

  /* bool ne_date_timestamp(DateADT date, Timestamp ts); */
  bool_result = ne_date_timestamp(date1, tstz_in);
  printf("ne_date_timestamp(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool ne_date_timestamptz(DateADT date, TimestampTz tstz); */
  bool_result = ne_date_timestamptz(date1, tstz_in);
  printf("ne_date_timestamptz(%s, %s): %c\n", date1_out, tstz_out, bool_result ? 't' : 'f');

  /* bool ne_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = ne_timestamp_date(ts_in, date1);
  printf("ne_timestamp_date(%s, %s): %c\n", ts_out, date1_out, bool_result ? 't' : 'f');

  /* bool ne_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = ne_timestamptz_date(tstz_in, date1);
  printf("ne_timestamptz_date(%s, %s): %c\n", tstz_out, date1_out, bool_result ? 't' : 'f');

  /* DateADT timestamp_to_date(Timestamp ts); */
  date_result = timestamp_to_date(ts_in);
  char_result = date_out(date_result);
  printf("timestamp_to_date(%s): %s\n", ts_out, char_result);
  free(char_result);

  /* DateADT timestamptz_to_date(TimestampTz tstz); */
  date_result = timestamptz_to_date(tstz_in);
  char_result = date_out(date_result);
  printf("timestamptz_to_date(%s): %s\n", tstz_out, char_result);
  free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(date1_out); free(date2_out);
  free(time1_out);
  free(ts_out); free(tstz_out);
  free(interv_in); free(interv_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
