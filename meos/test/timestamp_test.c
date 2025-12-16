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
 * @brief A simple program that tests the timestamp functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o timestamp_test timestamp_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
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
  float8 float8_in1 = 8;
  DateADT date1 = date_in("2025-03-01");
  char *date1_out = date_out(date1);
  Interval *interv = interval_in("8 hours", -1);
  char *interv_out = interval_out(interv);
  Timestamp ts1 = timestamp_in("2025-03-01 08:00", -1);
  Timestamp ts2 = timestamp_in("2025-05-01 08:00", -1);
  Timestamp ts3 = timestamp_in("2025-04-01 08:00", -1);
  Timestamp ts4 = timestamp_in("2025-06-01 08:00", -1);
  char *ts1_out = timestamp_out(ts1);
  char *ts2_out = timestamp_out(ts2);
  char *ts3_out = timestamp_out(ts3);
  char *ts4_out = timestamp_out(ts4);
  TimestampTz tstz1 = timestamptz_in("2025-03-01 08:00", -1);
  TimestampTz tstz2 = timestamptz_in("2025-05-01 08:00", -1);
  TimestampTz tstz3 = timestamptz_in("2025-04-01 08:00", -1);
  TimestampTz tstz4 = timestamptz_in("2025-06-01 08:00", -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstz_out2 = timestamptz_out(tstz2);
  char *tstz_out3 = timestamptz_out(tstz3);
  char *tstz_out4 = timestamptz_out(tstz4);

  /* Create the result types for the functions of the API */
  bool bool_result;
  int32 int32_result;
  uint32 uint32_result;
  uint64 uint64_result;
  float8 float8_result;
  char *char_result;
  text *text_result;
  Interval *interv_result;
  Numeric numeric_result;
  Timestamp ts_result;
  TimestampTz tstz_result;
  
  /* Execute and print the result for the bool functions of the API */

  printf("****************************************************************\n");
  printf("* Timestamp *\n");
  printf("****************************************************************\n");

  /* Timestamp add_timestamp_interval(Timestamp ts, const Interval *interv); */
  ts_result = add_timestamp_interval(ts1, interv);
  char_result = timestamp_out(ts_result);
  printf("add_timestamp_interval(%s, %s): %s\n", ts1_out, interv_out, char_result);
  free(char_result);

  /* Timestamp add_timestamptz_interval(TimestampTz tstz, const Interval *interv); */
  ts_result = add_timestamptz_interval(tstz1, interv);
  char_result = timestamp_out(ts_result);
  printf("add_timestamptz_interval(%s, %s): %s\n", tstz1_out, interv_out, char_result);
  free(char_result);

  /* Timestamp add_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone); */
  text *zone = text_in("Europe/Brussels");
  ts_result = add_timestamptz_interval_at_zone(tstz1, interv, zone);
  char_result = timestamp_out(ts_result);
  printf("add_timestamptz_interval_at_zone(%s, %s, \"Europe/Brussels\"): %s\n",tstz1_out, interv_out, char_result);
  free(zone); free(char_result);

  /* int32 cmp_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  int32_result = cmp_timestamp_timestamp(ts1, ts2);
  printf("cmp_timestamp_timestamp(%s, %s): %d\n", ts1_out, ts2_out, int32_result);

  /* int32 cmp_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  int32_result = cmp_timestamp_timestamptz(ts1, tstz1);
  printf("cmp_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, int32_result);

  /* int32 cmp_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  int32_result = cmp_timestamptz_timestamp(tstz1, ts1);
  printf("cmp_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, int32_result);

  /* bool eq_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = eq_timestamp_date(ts1, date1);
  printf("eq_timestamp_date(%s, %s): %c\n", ts1_out, date1_out, bool_result ? 't' : 'f');

  /* bool eq_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = eq_timestamp_timestamp(ts1, ts2);
  printf("eq_timestamp_timestamp(%s, %s): %c\n", ts1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool eq_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  bool_result = eq_timestamp_timestamptz(ts1, tstz1);
  printf("eq_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool eq_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = eq_timestamptz_date(tstz1, date1);
  printf("eq_timestamptz_date(%s, %s): %c\n", tstz1_out, date1_out, bool_result ? 't' : 'f');

  /* bool eq_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = eq_timestamptz_timestamp(tstz1, ts1);
  printf("eq_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* bool eq_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2); */
  bool_result = eq_timestamptz_timestamptz(tstz1, tstz2);
  printf("timestamptz_eq(%s, %s): %c\n", tstz1_out, tstz_out2, bool_result ? 't' : 'f');

  /* TimestampTz float8_to_timestamptz(float8 seconds); */
  tstz_result = float8_to_timestamptz(float8_in1);
  char_result = timestamptz_out(tstz_result);
  printf("float8_to_timestamptz(%lf): %s\n", float8_in1, char_result);
  free(char_result);

  /* bool gt_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = gt_timestamp_timestamp(ts1, ts2);
  printf("gt_timestamp_timestamp(%s, %s): %c\n", ts1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool gt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  bool_result = gt_timestamp_timestamptz(ts1, tstz1);
  printf("gt_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool gt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = gt_timestamptz_timestamp(tstz1, ts1);
  printf("gt_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* bool ge_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = ge_timestamp_timestamp(ts1, ts2);
  printf("ge_timestamp_timestamp(%s, %s): %c\n", ts1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool ge_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  bool_result = ge_timestamp_timestamptz(ts1, tstz1);
  printf("ge_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool ge_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = ge_timestamptz_timestamp(tstz1, ts1);
  printf("ge_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* bool le_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = le_timestamp_timestamp(ts1, ts2);
  printf("le_timestamp_timestamp(%s, %s): %c\n", ts1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool le_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  bool_result = le_timestamp_timestamptz(ts1, tstz1);
  printf("le_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool le_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = le_timestamptz_timestamp(tstz1, ts1);
  printf("le_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* bool lt_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = lt_timestamp_timestamp(ts1, ts2);
  printf("lt_timestamp_timestamp(%s, %s): %c\n", tstz1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool lt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz); */
  bool_result = lt_timestamp_timestamptz(ts1, tstz1);
  printf("lt_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool lt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = lt_timestamptz_timestamp(tstz1, ts1);
  printf("lt_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* Timestamp minus_timestamp_interval(Timestamp ts, const Interval *interv); */
  ts_result = minus_timestamp_interval(ts1, interv);
  char_result = timestamp_out(ts_result);
  printf("minus_timestamp_interval(%s, %s): %s\n", ts1_out, interv_out, char_result);
  free(char_result);

  /* Interval *minus_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  interv_result = minus_timestamp_timestamp(ts1, ts2);
  char_result = interval_out(interv_result);
  printf("minus_timestamp_timestamp(%s, %s): %s\n", ts1_out, ts2_out, char_result);
  free(interv_result); free(char_result);

  /* TimestampTz minus_timestamptz_interval(TimestampTz tstz, const Interval *interv); */
  tstz_result = minus_timestamptz_interval(tstz1, interv);
  char_result = timestamptz_out(tstz_result);
  printf("minus_timestamptz_interval(%s, %s): %s\n", tstz1_out, interv_out, char_result);
  free(char_result);

  /* TimestampTz minus_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone); */
  zone = text_in("Europe/Brussels");
  tstz_result = minus_timestamptz_interval_at_zone(tstz1, interv, zone);
  char_result = timestamptz_out(tstz_result);
  printf("minus_timestamptz_interval_at_zone(%s, %s, \"Europe/Brussels\"): %s\n", tstz1_out, interv_out, char_result);
  free(zone); free(char_result);

  /* Interval *minus_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2);  */
  interv_result = minus_timestamptz_timestamptz(tstz1, tstz2);
  char_result = interval_out(interv_result);
  printf("minus_timestamptz_timestamptz(%s, %s): %s\n", tstz1_out, tstz_out2, char_result);
  free(interv_result); free(char_result);

  /* bool ne_timestamp_date(Timestamp ts, DateADT date); */
  bool_result = ne_timestamp_date(ts1, date1);
  printf("ne_timestamp_date(%s, %s): %c\n", ts1_out, date1_out, bool_result ? 't' : 'f');

  /* bool ne_timestamptz_date(TimestampTz tstz, DateADT date); */
  bool_result = ne_timestamptz_date(tstz1, date1);
  printf("ne_timestamptz_date(%s, %s): %c\n", tstz1_out, date1_out, bool_result ? 't' : 'f');

  /* bool ne_timestamp_timestamp(Timestamp ts1, Timestamp ts2); */
  bool_result = ne_timestamp_timestamp(ts1, ts2);
  printf("ne_timestamp_timestamp(%s, %s): %c\n", ts1_out, ts2_out, bool_result ? 't' : 'f');

  /* bool ne_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);  */
  bool_result = ne_timestamp_timestamptz(ts1, tstz1);
  printf("ne_timestamp_timestamptz(%s, %s): %c\n", ts1_out, tstz1_out, bool_result ? 't' : 'f');

  /* bool ne_timestamptz_timestamp(TimestampTz tstz, Timestamp ts); */
  bool_result = ne_timestamptz_timestamp(tstz1, ts1);
  printf("ne_timestamptz_timestamp(%s, %s): %c\n", tstz1_out, ts1_out, bool_result ? 't' : 'f');

  /* Interval *timestamp_age(Timestamp ts1, Timestamp ts2); */
  interv_result = timestamp_age(ts1, ts2);
  char_result = interval_out(interv_result);
  printf("timestamp_age(%s, %s): %s\n", ts1_out, ts2_out, char_result);
  free(interv_result); free(char_result);

  /* TimestampTz timestamp_at_local(Timestamp ts); */
  tstz_result = timestamp_at_local(ts1);
  char_result = timestamptz_out(tstz_result);
  printf("timestamp_at_local(%s): %s\n", ts1_out, char_result);
  free(char_result);

  /* Timestamp timestamp_bin(Timestamp ts, const Interval *stride, Timestamp origin); */
  Interval *stride = interval_in("1 hour", -1);
  Timestamp origin = timestamp_in("2000-01-03", -1);
  ts_result = timestamp_bin(ts1, stride, origin);
  char_result = timestamp_out(ts_result);
  printf("timestamp_bin(%s, \"1 hour\", \"2000-01-03\"): %s\n", ts1_out, char_result);
  free(stride); free(char_result);

  /* Numeric timestamp_extract(Timestamp ts, const text *units); */
  text *units = text_in("seconds");
  numeric_result = timestamp_extract(ts1, units);
  char_result = numeric_out(numeric_result);
  printf("timestamp_extract(%s, \"seconds\"): %s\n", ts1_out, char_result);
  free(units); free(numeric_result); free(char_result);

  /* uint32 timestamp_hash(Timestamp ts); */
  uint32_result = timestamp_hash(ts1);
  printf("timestamp_hash(%s): %u\n", ts1_out, uint32_result);

  /* uint64 timestamp_hash_extended(TimestampTz tstz, uint64 seed); */
  uint64_result = timestamp_hash_extended(ts1, 1);
  printf("timestamp_hash_extended(%s, 1): %lu\n", ts1_out, uint64_result);

  /* Timestamp timestamp_in(const char *str, int32 typmod); */
  ts_result = timestamp_in("2025-03-01 08:00:00", -1);
  char_result = timestamp_out(ts_result);
  printf("timestamp_in(\"2025-03-01 08:00:00\", -1): %s\n", char_result);
  free(char_result);

  /* bool timestamp_is_finite(Timestamp ts); */
  bool_result = timestamp_is_finite(ts1);
  printf("timestamp_is_finite(%s): %c\n", ts1_out, bool_result ? 't' : 'f');

  /* TimestampTz timestamp_izone(Timestamp ts, const Interval *zone); */
  Interval *interv_zone = interval_in("2 hours", -1);
  tstz_result = timestamp_izone(ts1, interv_zone);
  char_result = timestamptz_out(tstz_result);
  printf("timestamp_izone(%s, \"Europe/Brussels\"): %s\n", ts1_out, char_result);
  free(interv_zone); free(char_result);

  /* text *time_of_day(void); */ 
  text_result = time_of_day();
  char_result = text_out(text_result);
  printf("time_of_day(): %s\n", char_result);
  free(text_result); free(char_result);

  /* Timestamp timestamp_larger(Timestamp ts1, Timestamp ts2); */
  ts_result = timestamp_larger(ts1, ts2);
  char_result = timestamp_out(ts_result);
  printf("timestamp_larger(%s, %s): %s\n", ts1_out, ts2_out, char_result);
  free(char_result);

  /* Timestamp timestamp_make(int32 year, int32 month, int32 mday, int32 hour, int32 min, float8 sec); */
  ts_result = timestamp_make(2025, 3, 1, 8, 0, 0);
  char_result = timestamp_out(ts_result);
  printf("timestamp_make(2025, 3, 1, 8, 0, 0): %s\n", char_result);
  free(char_result);

  /* char *timestamp_out(Timestamp ts); */
  char_result = timestamp_out(ts1);
  printf("timestamp_out(%s,): %s\n", ts1_out, char_result);
  free(char_result);

  /* bool timestamp_overlaps(Timestamp ts1, Timestamp te1, Timestamp ts2, Timestamp te2); */
  bool_result = timestamp_overlaps(ts1, ts2, ts3, ts4);
  printf("timestamp_overlaps(%s, %s, %s, %s): %c\n", ts1_out, ts2_out, ts3_out, ts4_out, bool_result ? 't' : 'f');

  /* float8 timestamp_part(Timestamp ts, const text *units); */
  units = text_in("seconds");
  float8_result = timestamp_part(ts1, units);
  printf("timestamp_part(%s, \"seconds\"): %lf\n", ts1_out, float8_result);
  free(units);

  /* Timestamp timestamp_scale(Timestamp ts, int32 typmod); */
  ts_result = timestamp_scale(ts1, -1);
  char_result = timestamp_out(ts_result);
  printf("timestamp_scale(%s, -1): %s\n", ts1_out, char_result);
  free(char_result);

  /* Timestamp timestamp_smaller(Timestamp ts1, Timestamp ts2); */
  ts_result = timestamp_smaller(ts1, ts2);
  char_result = timestamp_out(ts_result);
  printf("timestamp_smaller(%s, %s): %s\n", ts1_out, ts2_out, char_result);
  free(char_result);

  /* TimestampTz timestamp_to_timestamptz(Timestamp ts); */
  tstz_result = timestamp_to_timestamptz(ts1);
  char_result = timestamptz_out(tstz_result);
  printf("timestamp_to_timestamptz(%s): %s\n", ts1_out, char_result);
  free(char_result);

  /*  Timestamp timestamp_trunc(Timestamp ts, const text *units); */
  units = text_in("seconds");
  ts_result = timestamp_trunc(ts1, units);
  char_result = timestamp_out(ts_result);
  printf("timestamp_trunc(%s, \"seconds\"): %s\n", ts1_out, char_result);
  free(units); free(char_result);

  /* TimestampTz timestamp_zone(Timestamp ts, const text *zone); */
  zone = text_in("Europe/Brussels");
  tstz_result = timestamp_zone(ts1, zone);
  char_result = timestamptz_out(tstz_result);
  printf("timestamp_zone(%s, \"Europe/Brussels\"): %s\n", ts1_out, char_result);
  free(zone); free(char_result);

  /* Interval *timestamptz_age(TimestampTz tstz1, TimestampTz tstz2); */
  interv_result = timestamptz_age(tstz1, tstz2);
  char_result = interval_out(interv_result);
  printf("timestamptz_age(%s, %s): %s\n", tstz1_out, tstz_out2, char_result);
  free(interv_result); free(char_result);

  /* TimestampTz timestamptz_at_local(TimestampTz tstz); */
  tstz_result = timestamptz_at_local(tstz1);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_at_local(%s): %s\n", tstz1_out, char_result);
  free(char_result);

  /* TimestampTz timestamptz_bin(TimestampTz tstz, const Interval *stride, TimestampTz origin); */
  stride = interval_in("1 hour", -1);
  origin = timestamp_in("2000-01-03", -1);
  tstz_result = timestamptz_bin(tstz1, stride, origin);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_bin(%s, \"1 hour\", \"2000-01-03\"): %s\n", tstz1_out, char_result);
  free(stride); free(char_result);

  /* Numeric *timestamptz_extract(TimestampTz tstz, const text *units); */
  units = text_in("seconds");
  numeric_result = timestamptz_extract(tstz1, units);
  char_result = numeric_out(numeric_result);
  printf("timestamptz_extract(%s, \"seconds\"): %s\n", tstz1_out, char_result);
  free(numeric_result); free(units); free(char_result);

  /* int32 timestamptz_hash(TimestampTz tstz); */
  int32_result = timestamptz_hash(tstz1);
  printf("timestamptz_hash(%s): %d\n", tstz1_out, int32_result);

  /* uint64 timestamptz_hash_extended(TimestampTz tstz, uint64 seed); */
  uint64_result = timestamptz_hash_extended(tstz1, 1);
  printf("timestamptz_hash_extended(%s, 1): %ld\n", tstz1_out, uint64_result);

  /* TimestampTz timestamptz_in(const char *str, int32 typmod); */
  tstz_result = timestamptz_in("2025-03-01 08:00:00", -1);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_in(\"2025-03-01 08:00:00\", -1): %s\n", char_result);
  free(char_result);

  /* Timestamp timestamptz_izone(TimestampTz tstz, const Interval *zone); */
  interv_zone = interval_in("2 hours", -1);
  ts_result = timestamptz_izone(tstz1, interv_zone);
  char_result = timestamp_out(ts_result);
  printf("timestamptz_izone(%s, \"Europe/Brussels\"): %s\n", tstz1_out, char_result);
  free(interv_zone); free(char_result);

  /* TimestampTz timestamptz_make(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec); */
  tstz_result = timestamptz_make(2025, 3, 1, 8, 0, 0);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_make(2025, 03, 01, 08, 00, 00): %s\n", char_result);
  free(char_result);

  /* TimestampTz timestamptz_make_at_timezone(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec, const text *zone); */
  zone = text_in("Europe/Brussels");
  tstz_result = timestamptz_make_at_timezone(2025, 3, 1, 8, 0, 0, zone);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_make_at_timezone(2025, 3, 1, 8, 0, 0, \"Europe/Brussels\"): %s\n", char_result);
  free(zone); free(char_result);

  /* char *timestamptz_out(TimestampTz tstz); */
  char_result = timestamptz_out(tstz1);
  printf("timestamptz_out(%s): %s\n", tstz1_out, char_result);
  free(char_result);

  /* bool timestamptz_overlaps(TimestampTz ts1, TimestampTz te1, TimestampTz ts2, TimestampTz te2); */
  bool_result = timestamptz_overlaps(tstz1, tstz2, tstz3, tstz4);
  printf("timestamptz_overlaps(%s, %s, %s, %s): %c\n", tstz1_out, tstz_out2, tstz_out3, tstz_out4, bool_result ? 't' : 'f');

  /* float8 timestamptz_part(TimestampTz tstz, const text *units); */
  units = text_in("seconds");
  float8_result = timestamptz_part(tstz1, units);
  printf("timestamptz_part(%s, \"seconds\"): %lf\n", tstz1_out, float8_result);
  free(units);

  /* TimestampTz timestamptz_scale(TimestampTz tstz, int32 typmod); */
  tstz_result = timestamptz_scale(tstz1, -1);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_scale(%s, -1): %s\n", tstz1_out, char_result);
  free(char_result);

  /* TimestampTz timestamptz_shift(TimestampTz tstz, const Interval *interv); */
  tstz_result = timestamptz_shift(tstz1, interv);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_shift(%s, %s): %s\n", tstz1_out, interv_out, char_result);
  free(char_result);

  /* Timestamp timestamptz_to_timestamp(TimestampTz tstz); */
  ts_result = timestamptz_to_timestamp(tstz1);
  char_result = timestamp_out(ts_result);
  printf("timestamptz_to_timestamp(%s): %s\n", tstz1_out, char_result);
  free(char_result);

  /* TimestampTz timestamptz_trunc(TimestampTz tstz, const text *units); */
  units = text_in("seconds");
  tstz_result = timestamptz_trunc(tstz1, units);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_trunc(%s, \"seconds\"): %s\n", tstz1_out, char_result);
  free(units); free(char_result);

  /* TimestampTz timestamptz_trunc_zone(TimestampTz tstz, const text *units, const text *zone); */
  units = text_in("seconds");
  zone = text_in("Europe/Brussels");
  tstz_result = timestamptz_trunc_zone(tstz1, units, zone);
  char_result = timestamptz_out(tstz_result);
  printf("timestamptz_trunc_zone(%s, \"seconds\", \"Europe/Brussels\"): %s\n", tstz1_out, char_result);
  free(units); free(zone); free(char_result);

  /* Timestamp timestamptz_zone(TimestampTz tstz, const text *zone); */
  zone = text_in("Europe/Brussels");
  ts_result = timestamptz_zone(tstz1, zone);
  char_result = timestamp_out(ts_result);
  printf("timestamptz_zone(%s, \"Europe/Brussels\"): %s\n", tstz1_out, char_result);
  free(zone); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(date1_out);
  free(interv); free(interv_out);
  free(ts1_out); free(ts2_out); free(ts3_out); free(ts4_out);
  free(tstz1_out); free(tstz_out2); free(tstz_out3); free(tstz_out4);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
