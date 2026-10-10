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
 * @brief A program that tests the DateStyle of MEOS, which states how a
 * timestamp is written and in which order the day, the month and the year of
 * an ambiguous date are read
 * @details #meos_get_datestyle states the output style and the field order
 * that #meos_set_datestyle sets, as the PostgreSQL parameter @p DateStyle
 * does, and #timestamptz_in reads the fields of an ambiguous date in that
 * order.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o datestyle_test datestyle_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/**
 * @brief Assert that the DateStyle reads as expected
 */
static void
assert_datestyle(const char *expected)
{
  char *s = meos_get_datestyle();
  assert(s && strcmp(s, expected) == 0);
  free(s);
}

/**
 * @brief Assert that a timestamp is read as the one written in ISO format
 */
static void
assert_reads_as(const char *input, const char *iso)
{
  assert(timestamptz_in(input, -1) == timestamptz_in(iso, -1));
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* The default reads the month before the day */
  assert_datestyle("ISO, MDY");
  assert_reads_as("01/02/2026 10:00:00+00", "2026-01-02 10:00:00+00");

  /* Each order is stated as it is set and reads the fields in that order */
  assert(meos_set_datestyle("ISO, DMY", NULL));
  assert_datestyle("ISO, DMY");
  assert_reads_as("01/02/2026 10:00:00+00", "2026-02-01 10:00:00+00");
  assert_reads_as("20/01/2026 10:00:00+00", "2026-01-20 10:00:00+00");
  assert(meos_set_datestyle("ISO, YMD", NULL));
  assert_datestyle("ISO, YMD");

  /* An output style alone keeps the order, and German sets the day first */
  assert(meos_set_datestyle("SQL", NULL));
  assert_datestyle("SQL, YMD");
  assert(meos_set_datestyle("German", NULL));
  assert_datestyle("German, DMY");

  assert(meos_set_datestyle("Postgres, MDY", NULL));
  assert_datestyle("Postgres, MDY");
  assert_reads_as("01/02/2026 10:00:00+00", "2026-01-02 10:00:00+00");

  meos_finalize();
  printf("datestyle_test: OK\n");
  return EXIT_SUCCESS;
}
