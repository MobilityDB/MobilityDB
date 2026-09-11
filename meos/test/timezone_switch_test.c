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
 * @brief A program that tests reading a timezone abbreviation in one timezone
 * after another.
 *
 * An abbreviation the session timezone defines means what that timezone says
 * it means, so the same abbreviation read under each timezone
 * #meos_initialize_timezone sets takes the meaning of that timezone. LMT, the
 * local mean time a zone keeps before its first standard time, has a different
 * offset in every zone, which makes it the abbreviation that shows which
 * timezone answered.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o timezone_switch_test timezone_switch_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <pg_timestamp.h>

#define INPUT "1850-01-01 12:00:00 LMT"

static int failures = 0;

/* Read the input in a timezone and compare its output with the expected text */
static void
check(const char *zone, const char *expected)
{
  meos_initialize_timezone(zone);
  TimestampTz t = timestamptz_in(INPUT, -1);
  char *out = timestamptz_out(t);
  if (strcmp(out, expected) != 0)
  {
    printf("FAIL: %s in %s gives %s, expected %s\n", INPUT, zone, out,
      expected);
    failures++;
  }
  else
    printf("ok: %s in %s gives %s\n", INPUT, zone, out);
  free(out);
}

int
main(void)
{
  meos_initialize();
  /* The local mean time of each zone, read one zone after the other */
  check("Europe/Moscow", "1850-01-01 12:00:00+02:30:17");
  check("America/New_York", "1850-01-01 12:00:00-04:56:02");
  check("Europe/Moscow", "1850-01-01 12:00:00+02:30:17");
  meos_finalize();
  if (failures)
  {
    printf("%d failure(s)\n", failures);
    return EXIT_FAILURE;
  }
  printf("All timezone switch checks passed\n");
  return EXIT_SUCCESS;
}
