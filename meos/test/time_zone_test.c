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
 * @brief A program that tests every function taking the name of a time zone
 * with a zone it knows and with a zone it does not know
 * @details Under an error handler that returns, as the handlers of the
 * bindings do, a function given a time zone it does not know raises one error
 * and returns its error value: an infinite timestamp, @p NULL for a
 * time with time zone. Given a time zone it knows, it raises no error and
 * returns the value PostgreSQL returns. The known zone is Europe/Zurich in
 * summer, two hours ahead of UTC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_text.h>
#include <pg_time.h>
#include <pg_timestamp.h>

/* A handler of a host: it counts the errors */
static int errors = 0;
static void
host_handler(int level, int code, const char *msg)
{
  (void) level; (void) code; (void) msg;
  errors++;
}

static int failures = 0, cases = 0;

/* A timestamp answer: the error value and one error for an unknown zone,
 * the expected value and no error for a known zone */
static void
check_ts(const char *what, bool known, TimestampTz value, const char *expected)
{
  cases++;
  int err = meos_errno();
  size_t size;
  char *out = known ? timestamptz_out(value, &size) : NULL;
  bool ok = known ?
    (errors == 0 && err == 0 && strcmp(out, expected) == 0) :
    (errors == 1 && err != 0 && ! timestamp_is_finite(value));
  if (! ok)
  {
    printf("  %s, %s zone: %s, %d error(s), error number %d\n", what,
      known ? "known" : "unknown", known ? out : "", errors, err);
    failures++;
  }
  free(out);
  errors = 0;
  meos_errno_reset();
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(host_handler);

  text *zones[2] = { cstring_to_text("Not/AZone"),
    cstring_to_text("Europe/Zurich") };
  text *units = cstring_to_text("day");
  TimestampTz tstz = timestamptz_in("2026-07-15 08:00:00+00", -1);
  Timestamp ts = timestamp_in("2026-07-15 08:00:00", -1);
  Interval *day = interval_in("1 day", -1);
  TimeTzADT *ttz = timetz_in("08:00:00+00", -1);

  for (int known = 0; known < 2; known++)
  {
    text *z = zones[known];
    check_ts("timestamp_zone", known, timestamp_zone(ts, z),
      "2026-07-15 06:00:00+00");
    check_ts("timestamptz_make_at_timezone", known,
      timestamptz_make_at_timezone(2026, 7, 15, 8, 0, 0.0, z),
      "2026-07-15 06:00:00+00");
    check_ts("timestamptz_trunc_zone", known,
      timestamptz_trunc_zone(tstz, units, z), "2026-07-14 22:00:00+00");
    check_ts("add_timestamptz_interval_at_zone", known,
      add_timestamptz_interval_at_zone(tstz, day, z),
      "2026-07-16 08:00:00+00");
    check_ts("minus_timestamptz_interval_at_zone", known,
      minus_timestamptz_interval_at_zone(tstz, day, z),
      "2026-07-14 08:00:00+00");

    /* A timestamp without time zone, read in UTC, is the local time */
    cases++;
    Timestamp local = timestamptz_zone(tstz, z);
    int err = meos_errno();
    size_t size;
    char *out = known ? timestamp_out(local, &size) : NULL;
    if (known ? (errors || err || strcmp(out, "2026-07-15 10:00:00")) :
        (errors != 1 || ! err || timestamp_is_finite(local)))
    {
      printf("  timestamptz_zone, %s zone: %d error(s), error number %d\n",
        known ? "known" : "unknown", errors, err);
      failures++;
    }
    free(out);
    errors = 0;
    meos_errno_reset();

    cases++;
    TimeTzADT *ttz_res = timetz_zone(ttz, z);
    err = meos_errno();
    if (known ? (errors || err || ! ttz_res) :
        (errors != 1 || ! err || ttz_res))
    {
      printf("  timetz_zone, %s zone: %s, %d error(s), error number %d\n",
        known ? "known" : "unknown", ttz_res ? "a value" : "NULL", errors,
        err);
      failures++;
    }
    free(ttz_res);
    errors = 0;
    meos_errno_reset();
  }

  printf("%d of %d case(s) failed\n", failures, cases);
  free(zones[0]); free(zones[1]); free(units); free(day); free(ttz);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
