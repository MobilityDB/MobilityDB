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
 * @brief A simple program that tests the time functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o time_test time_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_date.h>
#include <pg_time.h>
#include <pg_text.h>
#include <pg_numeric.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */
  DateADT date1 = date_in("2025-03-01");
  char *date1_out = date_out(date1);
  TimeADT time1 = time_in("08:00:00", -1);
  TimeADT time2 = time_in("10:00:00", -1);
  TimeADT time3 = time_in("09:00:00", -1);
  TimeADT time4 = time_in("11:00:00", -1);
  char *time1_out = time_out(time1);
  char *time2_out = time_out(time2);
  char *time_out3 = time_out(time3);
  char *time_out4 = time_out(time4);
  // TimeTzADT *timetz1 = timetz_in("08:00:00", -1);
  // TimeTzADT *timetz2 = timetz_in("10:00:00", -1);
  // TimeTzADT *timetz3 = timetz_in("09:00:00", -1);
  // TimeTzADT *timetz4 = timetz_in("11:00:00", -1);
  // char *timetz1_out = timetz_out(timetz1);
  // char *timetz2_out = timetz_out(timetz2);
  // char *timetz3_out = timetz_out(timetz3);
  // char *timetz4_out = timetz_out(timetz4);
  Timestamp ts_in = timestamp_in("2025-03-01 08:00:00", -1);
  char *ts_out = timestamp_out(ts_in);
  TimestampTz tstz_in = timestamptz_in("2025-03-01 08:00:00", -1);
  char *tstz_out = timestamptz_out(tstz_in);
  Interval *interv_in = interval_in("3 hours", -1);
  char *interv_out = interval_out(interv_in);

  /* Create the result types for the functions of the API */
  bool bool_result;
  int32_t int32_result;
  uint32_t uint32_result;
  uint64_t uint64_result;
  double float8_result;
  char *char_result;
  Numeric numeric_result;
  TimeADT time_result;
  // TimeTzADT timetz_result;
  Interval *interv_result;
  // TimestampTz tstz_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Time *\n");
  printf("****************************************************************\n");

  // /* TimestampTz date_timetz_to_timestamptz(DateADT date, const TimeTzADT *timetz); */
  // tstz_result = date_timetz_to_timestamptz(date1, timetz1);
  // char_result = timestamptz_out(tstz_result);
  // printf("timetz1_out(%s, %s): %s\n", date1_out, timetz1_out, char_result);
  // free(char_result);

  /* TimeADT interval_to_time(const Interval *interv); */
  time_result = interval_to_time(interv_in);
  char_result = time_out(time_result);
  printf("interval_to_time(%s): %s\n", interv_out, char_result);
  free(char_result);

  /* TimeADT minus_time_interval(TimeADT time, const Interval *interv); */
  time_result = minus_time_interval(time1, interv_in);
  char_result = time_out(time_result);
  printf("minus_time_interval(%s, %s): %s\n", time1_out, interv_out, char_result);
  free(char_result);

  /* Interval *minus_time_time(TimeADT time1, TimeADT time2); */
  interv_result = minus_time_time(time1, time2);
  char_result = time_out(time_result);
  printf("minus_time_time(%s, %s): %s\n", time1_out, time2_out, char_result);
  free(interv_result); free(char_result);

  // TimeTzADT *minus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
  // timetz_result = minus_timetz_interval(timetz1, interv_in);
  // char_result = timetz_out(timetz_result);
  // printf("minus_timetz_interval(%s, %s): %s\n", timetz1_out, interv_out, bool_result ? "t" : "f");
  // free(timetz_result); free(char_result);

  /* TimeADT plus_time_interval(TimeADT time, Interval *interv); */
  time_result = plus_time_interval(time1, interv_in);
  char_result = time_out(time_result);
  printf("plus_time_interval(%s, %s): %s\n", time1_out, interv_out, char_result);
  free(char_result);

  // TimeTzADT *plus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
  // timetz_result = plus_timetz_interval(timetz1, interv_in);
  // char_result = timetz_out(timetz_result);
  // printf("plus_timetz_interval(%s, %s): %s\n", timetz1_out, interv_out, char_result);
  // free(timetz_result); free(char_result);

  /* int time_cmp(TimeADT time1, TimeADT time2); */
  int32_result = time_cmp(time1, time2);
  printf("time_cmp(%s, %s): %d\n", time1_out, time2_out, int32_result);

  /* bool time_eq(TimeADT time1, TimeADT time2); */
  bool_result = time_eq(time1, time2);
  printf("time_eq(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* Numeric time_extract(TimeADT time, const text *units); */
  text *units = text_in("seconds");
  numeric_result = time_extract(time1, units);
  char_result = numeric_out(numeric_result);
  printf("time_extract(%s, \"seconds\"): %s\n", time1_out, char_result);
  free(units); free(numeric_result); free(char_result);

  /* bool time_ge(TimeADT time1, TimeADT time2); */
  bool_result = time_ge(time1, time2);
  printf("time_ge(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* bool time_gt(TimeADT time1, TimeADT time2); */
  bool_result = time_gt(time1, time2);
  printf("time_gt(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* uint32_t time_hash(TimeADT time); */
  uint32_result = time_hash(time1);
  printf("time_hash(%s): %u\n", time1_out, uint32_result);

  /* uint64_t time_hash_extended(TimeADT time, int32_t seed); */
  uint64_result = time_hash_extended(time1, 1);
  printf("time_hash_extended(%s, 1): %lu\n", time1_out, uint64_result);

  /* TimeADT time_in(const char *str, int32_t typmod); */
  time_result = time_in("08:00:00", -1);
  char_result = time_out(time_result);
  printf("time_in(\"08:00:00\"): %s\n", char_result);
  free(char_result);

  /* TimeADT time_larger(TimeADT time1, TimeADT time2); */
  time_result = time_larger(time1, time2);
  char_result = time_out(time_result);
  printf("time_larger(%s, %s): %s\n", time1_out, time2_out, char_result);
  free(char_result);

  /* bool time_le(TimeADT time1, TimeADT time2); */
  bool_result = time_le(time1, time2);
  printf("time_le(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* bool time_lt(TimeADT time1, TimeADT time2); */
  bool_result = time_lt(time1, time2);
  printf("time_lt(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* TimeADT time_make(int tm_hour, int tm_min, double sec); */
  time_result = time_make(3, 3, 3);
  char_result = time_out(time_result);
  printf("time_make(3, 3, 3): %s\n", char_result);
  free(char_result);

  /* bool time_ne(TimeADT time1, TimeADT time2); */
  bool_result = time_ne(time1, time2);
  printf("time_ne(%s, %s): %s\n", time1_out, time2_out, bool_result ? "t" : "f");

  /* char *time_out(TimeADT time); */
  char_result = time_out(time1);
  printf("time_out(%s): %s\n", time1_out, char_result);
  free(char_result);

  /* bool time_overlaps(TimeADT ts1, TimeADT te1, TimeADT ts2, TimeADT te2); */
  bool_result = time_overlaps(time1, time2, time3, time4);
  printf("time_overlaps(%s, %s, %s, %s): %s\n", time1_out, time2_out, time_out3, time_out4, bool_result ? "t" : "f");

  /* double time_part(TimeADT time, const text *units); */
  units = text_in("seconds");
  float8_result = time_part(time1, units);
  printf("time_part(%s, \"seconds\"): %lf\n", time1_out, float8_result);
  free(units);

  /* TimeADT time_scale(TimeADT date, int32_t typmod); */
  time_result = time_scale(date1, -1);
  char_result = time_out(time_result);
  printf("time_scale(%s, -1): %s\n", date1_out, char_result);
  free(char_result);

  /* TimeADT time_smaller(TimeADT time1, TimeADT time2); */
  time_result = time_smaller(time1, time2);
  char_result = time_out(time_result);
  printf("time_smaller(%s, %s): %s\n", time1_out, time2_out, char_result);
  free(char_result);

  /* Interval *time_to_interval(TimeADT time); */
  interv_result = time_to_interval(time1);
  char_result = interval_out(interv_result);
  printf("time_to_interval(%s): %s\n", time1_out, char_result);
  free(interv_result); free(char_result);

  // /* TimeTzADT *time_to_timetz(TimeADT time); */
  // timetz_result = time_to_timetz(time1);
  // char_result = timetz_out(timetz_result);
  // printf("time_to_timetz(%s): %s\n", time1_out, char_result);
  // free(timetz_result); free(char_result);

  /* TimeADT timestamp_to_time(Timestamp ts); */
  time_result = timestamp_to_time(ts_in);
  char_result = time_out(time_result);
  printf("timestamp_to_time(%s): %s\n", ts_out, char_result);
  free(char_result);

  /* TimeADT timestamptz_to_time(TimestampTz tztz); */
  time_result = timestamptz_to_time(tstz_in);
  char_result = time_out(time_result);
  printf("timestamptz_to_time(%s): %s\n", tstz_out, char_result);
  free(char_result);

  // /* extern TimeTzADT *timestamptz_to_timetz(TimestampTz tztz); */
  // timetz_result = timestamptz_to_timetz(tstz_in);
  // char_result = timetz_out(timetz_result);
  // printf("timestamptz_to_timetz(%s): %s\n", tstz_out, char_result);
  // free(timetz_result); free(char_result);

  // /* extern TimeTzADT *timetz_at_local(const TimeTzADT *timetz); */
  // timetz_result = timetz_at_local(timetz1);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_at_local(%s): %s\n", timetz1_out, char_result);
  // free(timetz_result); free(char_result);

  // /* int32_t timetz_cmp(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // int32_result = timetz_cmp(timetz1, timetz2);
  // printf("timetz_cmp(%s, %s): %d\n", timetz1_out, timetz2_out, int32_result);

  // /* bool timetz_eq(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_eq(timetz1, timetz2);
  // printf("timetz_eq(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* Numeric timetz_extract(const TimeTzADT *timetz, const text *units); */
  // units = text_in("seconds");
  // numeric_result = timetz_extract(timetz1, units);
  // char_result = numeric_out(numeric_result);
  // printf("timetz_extract(%s, \"seconds\"): %s\n", timetz1_out, char_result);
  // free(units); free(numeric_result); free(char_result);

  // /* bool timetz_ge(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_ge(timetz1, timetz2);
  // printf("timetz_ge(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* bool timetz_gt(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_gt(timetz1, timetz2);
  // printf("timetz_gt(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* uint32_t timetz_hash(const TimeTzADT *timetz); */
  // uint32_result = timetz_hash(timetz1);
  // printf("timetz_hash(%s): %u\n", timetz1_out, uint32_result);

  // /* uint64_t timetz_hash_extended(const TimeTzADT *timetz, uint64_t seed); */
  // uint64_result = timetz_hash_extended(timetz1, 1);
  // printf("timetz_hash_extended(%s, 1): %lu\n", timetz1_out, uint64_result);

  // /* TimeTzADT *timetz_in(const char *str, int32_t typmod); */
  // timetz_result = timetz_in("2025-03-01", -1);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_in(\"2025-03-01\", -1): %s\n", char_result);
  // free(timetz_result); free(char_result);

  // /* TimeTzADT *timetz_izone(const TimeTzADT *timetz, const Interval *zone); */
  // timetz_result = timetz_izone(timetz1, interv_in);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_izone(%s, %s): %s\n", timetz1_out, interv_out, char_result);
  // free(timetz_result); free(char_result);

  // /* TimeTzADT *timetz_larger(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // timetz_result = timetz_larger(timetz1, timetz2);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_larger(%s, %s): %s\n", timetz1_out, timetz2_out, char_result);
  // free(timetz_result); free(char_result);

  // /* bool timetz_le(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_le(timetz1, timetz2);
  // printf("timetz_le(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* bool timetz_lt(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_lt(timetz1, timetz2);
  // printf("timetz_lt(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* bool timetz_ne(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // bool_result = timetz_ne(timetz1, timetz2);
  // printf("timetz_ne(%s, %s): %s\n", timetz1_out, timetz2_out, bool_result ? "t" : "f");

  // /* char *timetz_out(const TimeTzADT *timetz); */
  // char_result = timetz_out(timetz1);
  // printf("timetz_out(%s): %s\n", timetz1_out, char_result);
  // free(char_result);

  // /* bool timetz_overlaps(const TimeTzADT *ts1, const TimeTzADT *te1, const TimeTzADT *ts2, const TimeTzADT *te2); */
  // bool_result = timetz_overlaps(timetz1, timetz2, timetz3, timetz4);
  // printf("timetz_overlaps(%s, %s, %s, %s): %s\n", timetz1_out, timetz2_out, timetz3_out, timetz4_out, bool_result ? "t" : "f");

  // /* double timetz_part(const TimeTzADT *timetz, const text *units); */
  // units = text_in("seconds");
  // float8_result = timetz_part(timetz1, units);
  // printf("timetz_part(%s, \"seconds\"): %lf\n", timetz1_out, float8_result);
  // free(units);

  // /* TimeTzADT *timetz_scale(const TimeTzADT *timetz, int32_t typmod); */
  // timetz_result = timetz_scale(timetz1, -1);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_scale(%s, -1): %s\n", timetz1_out, char_result);
  // free(timetz_result); free(char_result);

  // /* TimeTzADT *timetz_smaller(const TimeTzADT *timetz1, const TimeTzADT *timetz2); */
  // timetz_result = timetz_smaller(timetz1, timetz2);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_smaller(%s, %s): %s\n", timetz1_out, timetz2_out, char_result);
  // free(timetz_result); free(char_result);

  // /* TimeADT timetz_to_time(const TimeTzADT *timetz); */
  // time_result = timetz_to_time(timetz1);
  // char_result = time_out(time_result);
  // printf("timetz_to_time(%s): %s\n", timetz1_out, char_result);
  // free(char_result);

  // /* TimeTzADT *timetz_zone(const TimeTzADT *timetz, const text *zone); */
  // text *zone = text_in("Europe/Brussels");
  // timetz_result = timetz_zone(timetz1, zone);
  // char_result = timetz_out(timetz_result);
  // printf("timetz_zone(%s, \"Europe/Brussels\"): %s\n", timetz1_out, char_result);
  // free(zone); free(timetz_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(time1_out); free(time2_out); free(time_out3); free(time_out4);
  // free(timetz1); free(timetz2); free(timetz3); free(timetz4); 
  // free(timetz1_out); free(timetz2_out); free(timetz3_out); free(timetz4_out); 
  free(date1_out); free(ts_out); free(tstz_out);
  free(interv_in); free(interv_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
