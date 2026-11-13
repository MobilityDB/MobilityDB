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
 * @brief A program that tests the conversion of a timestamptz and of a
 * timestamp to and from the microseconds from the Unix epoch, and of a date to
 * and from the days from it
 * @details Each count is the one Python's datetime computes for the same
 * instant: a year start clear of the PostgreSQL epoch, the Unix epoch, the
 * microsecond before the Unix epoch, and an instant with a fraction of a
 * second written in two zones, each
 * with its round trip. An infinite timestamp, a count outside the range of
 * timestamps and the last timestamp of the range, whose count exceeds a 64-bit
 * integer, report #MEOS_ERR_DATETIME_OUT_OF_RANGE. The Unix seconds of the
 * Mapbox Vector Tile encoding round towards negative infinity, so the instant
 * half a second before the PostgreSQL epoch has the second before it.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o unix_epoch_test unix_epoch_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <pg_timestamp.h>

static int failures;

/* Verify the error the last operation left, then clear it */
static void
check_error(const char *what, int expected)
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

/* Verify the count of a timestamp and its round trip */
static void
check_count(const char *text, int64 expected)
{
  TimestampTz t = timestamptz_in(text, -1);
  int64 us = timestamptz_to_unix_us(t);
  TimestampTz back = unix_us_to_timestamptz(us);
  printf("%s: %lld\n", text, (long long) us);
  if (us != expected || back != t || meos_errno() != 0)
  {
    printf("  expected %lld and the same timestamp back\n",
      (long long) expected);
    failures++;
  }
  meos_errno_reset();
}

/* Verify the count of a timestamp without time zone and its round trip, as
 * #check_count does for a timestamptz */
static void
check_count_ts(const char *text, int64 expected)
{
  Timestamp t = timestamp_in(text, -1);
  int64 us = timestamp_to_unix_us(t);
  Timestamp back = unix_us_to_timestamp(us);
  printf("timestamp %s: %lld\n", text, (long long) us);
  if (us != expected || back != t || meos_errno() != 0)
  {
    printf("  expected %lld and the same timestamp back\n",
      (long long) expected);
    failures++;
  }
  meos_errno_reset();
}

/* Verify the days from the Unix epoch of a date and its round trip, as
 * #check_count does for a timestamptz */
static void
check_days(const char *text, int32 expected)
{
  DateADT d = date_in(text);
  int32 days = date_to_unix_days(d);
  DateADT back = unix_days_to_date(days);
  printf("date %s: %d\n", text, days);
  if (days != expected || back != d || meos_errno() != 0)
  {
    printf("  expected %d and the same date back\n", expected);
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
  const int DT = MEOS_ERR_DATETIME_OUT_OF_RANGE;

  check_count("2001-01-01 00:00:00+00", 978307200000000);
  check_count("1970-01-01 00:00:00+00", 0);
  check_count("1969-12-31 23:59:59.999999+00", -1);
  check_count("2025-03-01 08:00:00.123456+00", 1740816000123456);
  check_count("2025-03-01 09:00:00.123456+01", 1740816000123456);

  /* An infinite timestamp has no count, nor a count outside the range */
  timestamptz_to_unix_us(timestamptz_in("infinity", -1));
  check_error("timestamptz_to_unix_us(infinity)", DT);
  timestamptz_to_unix_us(timestamptz_in("-infinity", -1));
  check_error("timestamptz_to_unix_us(-infinity)", DT);
  unix_us_to_timestamptz(INT64_MIN);
  check_error("unix_us_to_timestamptz(INT64_MIN)", DT);
  /* The largest count is a timestamp before the end of the range, while the
   * last timestamp of the range has a count beyond the largest one */
  TimestampTz last = unix_us_to_timestamptz(INT64_MAX);
  check_error("unix_us_to_timestamptz(INT64_MAX)", 0);
  if (timestamptz_to_unix_us(last) != INT64_MAX)
  {
    printf("  expected the count of unix_us_to_timestamptz(INT64_MAX) back\n");
    failures++;
  }
  check_error("timestamptz_to_unix_us(unix_us_to_timestamptz(INT64_MAX))", 0);
  timestamptz_to_unix_us(timestamptz_in("294276-12-31 23:59:59.999999+00", -1));
  check_error("timestamptz_to_unix_us(294276-12-31 23:59:59.999999+00)", DT);

  /* A timestamp without time zone counts as a timestamptz in UTC does */
  check_count_ts("2001-01-01 00:00:00", 978307200000000);
  check_count_ts("1970-01-01 00:00:00", 0);
  check_count_ts("1969-12-31 23:59:59.999999", -1);
  check_count_ts("2025-03-01 08:00:00.123456", 1740816000123456);
  timestamp_to_unix_us(timestamp_in("infinity", -1));
  check_error("timestamp_to_unix_us(infinity)", DT);
  unix_us_to_timestamp(INT64_MIN);
  check_error("unix_us_to_timestamp(INT64_MIN)", DT);

  /* A date counts its days from the Unix epoch */
  check_days("2001-01-01", 11323);
  check_days("1970-01-01", 0);
  check_days("1969-12-31", -1);
  check_days("2025-03-01", 20148);
  date_to_unix_days(date_in("infinity"));
  check_error("date_to_unix_days(infinity)", DT);
  date_to_unix_days(date_in("-infinity"));
  check_error("date_to_unix_days(-infinity)", DT);
  unix_days_to_date(INT32_MIN);
  check_error("unix_days_to_date(INT32_MIN)", DT);
  unix_days_to_date(INT32_MAX);
  check_error("unix_days_to_date(INT32_MAX)", DT);

  /* The Unix seconds of the vector tile encoding */
  STBox *bounds = stbox_in("STBOX X((0,0),(1000,1000))");
  const char *points[] = {"Point(10 10)@1999-12-31 23:59:59.5+00",
    "Point(10 10)@2001-01-01 00:00:00+00"};
  const int64 seconds[] = {946684799, 978307200};
  for (int i = 0; i < 2; i++)
  {
    Temporal *temp = tgeompoint_in(points[i]);
    int64 *timesarr;
    int count;
    GSERIALIZED *gs = tpoint_as_mvtgeom(temp, bounds, 4096, 256, true,
      &timesarr, &count);
    printf("tpoint_as_mvtgeom(%s): %lld\n", points[i],
      gs && count == 1 ? (long long) timesarr[0] : -1LL);
    if (! gs || count != 1 || timesarr[0] != seconds[i])
    {
      printf("  expected %lld\n", (long long) seconds[i]);
      failures++;
    }
    if (gs)
    {
      free(gs); free(timesarr);
    }
    free(temp);
  }
  free(bounds);

  meos_finalize();
  printf("%d failure(s)\n", failures);
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
