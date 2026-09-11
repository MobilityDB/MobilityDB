/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief A program that tests the time zone database compiled into MEOS.
 *
 * Standalone MEOS reads its zones from the database compiled into the
 * library, so the program answers the same on a host without a zone
 * directory, Windows included. It checks a zone found by name ignoring case,
 * a zone name that is a link to another zone, the standard and daylight
 * saving offsets of a zone, an offset that is not a whole number of hours,
 * a zone named inside the text of a timestamp, and abbreviations of
 * PostgreSQL's Default set that the session zone does not define.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tzdata_test tzdata_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_timestamp.h>

static int failures = 0;

/* Output an instant in a session zone and compare it with the expected text */
static void
check(const char *zone, const char *input, const char *expected)
{
  meos_initialize_timezone(zone);
  TimestampTz t = timestamptz_in(input, -1);
  char *out = timestamptz_out(t);
  if (strcmp(out, expected) != 0)
  {
    printf("FAIL: %s in %s gives %s, expected %s\n", input, zone, out,
      expected);
    failures++;
  }
  else
    printf("ok: %s in %s gives %s\n", input, zone, out);
  free(out);
}

int
main(void)
{
  meos_initialize();

  /* Standard and daylight saving time of one zone */
  check("Europe/Brussels", "2001-01-15 00:00:00+00", "2001-01-15 01:00:00+01");
  check("Europe/Brussels", "2001-07-15 00:00:00+00", "2001-07-15 02:00:00+02");
  /* A zone name spelt in another case */
  check("america/new_york", "2001-01-15 00:00:00+00", "2001-01-14 19:00:00-05");
  /* A zone name that is a link to another zone */
  check("US/Eastern", "2001-07-15 00:00:00+00", "2001-07-14 20:00:00-04");
  /* A zone whose offset is not a whole number of hours */
  check("Asia/Kolkata", "2001-01-15 00:00:00+00", "2001-01-15 05:30:00+05:30");
  /* A zone named inside the text of a timestamp */
  check("Europe/Brussels", "2001-01-15 12:00:00 America/New_York",
    "2001-01-15 18:00:00+01");
  /* Abbreviations of PostgreSQL's Default set that the session zone does not
   * define, answered as PostgreSQL answers them: CET is a fixed offset of one
   * hour, also in summer and although a zone of that name exists, PDT is a
   * daylight saving time, and MSK takes the offset of Europe/Moscow at the
   * instant */
  check("America/New_York", "2001-07-15 12:00:00 CET",
    "2001-07-15 07:00:00-04");
  check("America/New_York", "2001-07-15 12:00:00 PDT",
    "2001-07-15 15:00:00-04");
  check("America/New_York", "2001-01-15 12:00:00 MSK",
    "2001-01-15 04:00:00-05");

  meos_finalize();
  if (failures)
  {
    printf("%d failure(s)\n", failures);
    return EXIT_FAILURE;
  }
  printf("All time zone checks passed\n");
  return EXIT_SUCCESS;
}
