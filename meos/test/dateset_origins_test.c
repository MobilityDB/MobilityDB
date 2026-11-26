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
 * @brief A program that tests the origins of the days of a date set in a
 * time zone
 * @details The origin of a day is its local noon minus twelve hours. The
 * program reads the origins of days around both daylight-saving changes of
 * a year in Europe/Zurich (+01 in winter, +02 in summer) and in
 * America/New_York (-05 and -04), in UTC, and at a fixed offset, where the
 * origin of a change day is not the local midnight; it checks that a day
 * beyond the timestamps and a time zone that is not known answer NULL with
 * one error under a handler that returns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_text.h>

/* A handler of a host: it counts the errors */
static int errors = 0;
static void
host_handler(int level, int code, const char *msg)
{
  (void) level; (void) code; (void) msg;
  errors++;
}

static int failures = 0, cases = 0;

static void
check(const char *dates, const char *zone, const char *expected)
{
  cases++;
  Set *s = dateset_in(dates);
  text *z = cstring_to_text(zone);
  Set *res = dateset_origins(s, z);
  size_t size;
  char *out = res ? set_out(res, 15, &size) : NULL;
  bool ok = expected ?
    (res && errors == 0 && strcmp(out, expected) == 0) :
    (! res && errors == 1 && meos_errno() != 0);
  if (! ok)
  {
    printf("  dateset_origins(%s, %s): %s, %d error(s)\n  expected %s\n",
      dates, zone, out ? out : "NULL", errors, expected ? expected : "NULL");
    failures++;
  }
  free(s); free(z); free(res); free(out);
  errors = 0;
  meos_errno_reset();
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(host_handler);

  /* Spring change on 2026-03-29: the origin is 23:00 of the day before in
   * local time, as noon is in summer time */
  check("{2026-01-15, 2026-03-28, 2026-03-29, 2026-03-30}", "Europe/Zurich",
    "{\"2026-01-14 23:00:00+00\", \"2026-03-27 23:00:00+00\", "
    "\"2026-03-28 22:00:00+00\", \"2026-03-29 22:00:00+00\"}");
  /* Autumn change on 2026-10-25: the origin is 01:00 of the day in local
   * time, as noon is in winter time */
  check("{2026-07-15, 2026-10-24, 2026-10-25, 2026-10-26}", "Europe/Zurich",
    "{\"2026-07-14 22:00:00+00\", \"2026-10-23 22:00:00+00\", "
    "\"2026-10-24 23:00:00+00\", \"2026-10-25 23:00:00+00\"}");
  check("{2026-03-08, 2026-11-01}", "America/New_York",
    "{\"2026-03-08 04:00:00+00\", \"2026-11-01 05:00:00+00\"}");
  check("{2026-07-15}", "UTC", "{\"2026-07-15 00:00:00+00\"}");
  /* A numeric zone follows the POSIX sign convention of AT TIME ZONE */
  check("{2026-07-15}", "+05", "{\"2026-07-15 05:00:00+00\"}");
  /* A day beyond the timestamps and a time zone that is not known */
  check("{2026-07-15, 300000-01-01}", "Europe/Zurich", NULL);
  check("{2026-07-15}", "Not/AZone", NULL);

  printf("%d of %d case(s) failed\n", failures, cases);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
