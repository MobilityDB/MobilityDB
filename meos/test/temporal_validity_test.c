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
 * @brief A program that tests how the temporal restrictions to the instants
 * before or after a timestamptz, and the test of whether the time of two
 * temporal values overlaps, report a null argument under the noexit error
 * handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer receives an error it can raise in its
 * host language. An assertion cannot carry that contract: it is compiled out
 * under NDEBUG, which leaves the release build a binding links against with no
 * check at all.
 *
 * The program verifies that #temporal_before_timestamptz,
 * #temporal_after_timestamptz and #temporal_time_overlaps report a null
 * temporal value by returning their error value and setting #meos_errno, and
 * that a valid call still answers with no error left behind. Each accepts a
 * value of every temporal type, so a null pointer is the argument they can
 * receive wrongly.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_validity_test temporal_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp = tint_in("[1@2001-01-01, 2@2001-01-03]");
  assert(temp);
  TimestampTz t = temporal_start_timestamptz(temp);
  meos_errno_reset();

  /* A null temporal value is reported rather than dereferenced */
  Temporal *res = temporal_before_timestamptz(NULL, t, false);
  printf("temporal_before_timestamptz(NULL, 2001-01-01, false): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  res = temporal_after_timestamptz(NULL, t, true);
  printf("temporal_after_timestamptz(NULL, 2001-01-01, true): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  bool overlaps = temporal_time_overlaps(NULL, temp);
  printf("temporal_time_overlaps(NULL, [1@2001-01-01, 2@2001-01-03]): %s, "
    "errno %d\n", overlaps ? "true" : "false", meos_errno());
  assert(! overlaps);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  overlaps = temporal_time_overlaps(temp, NULL);
  printf("temporal_time_overlaps([1@2001-01-01, 2@2001-01-03], NULL): %s, "
    "errno %d\n", overlaps ? "true" : "false", meos_errno());
  assert(! overlaps);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A valid call still answers, and the guards leave no error behind */
  res = temporal_before_timestamptz(temp, t, false);
  printf("temporal_before_timestamptz([1@2001-01-01, 2@2001-01-03], "
    "2001-01-01, false): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res != NULL);
  assert(meos_errno() == 0);
  free(res);

  res = temporal_after_timestamptz(temp, t, false);
  printf("temporal_after_timestamptz([1@2001-01-01, 2@2001-01-03], "
    "2001-01-01, false): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res != NULL);
  assert(meos_errno() == 0);
  free(res);

  overlaps = temporal_time_overlaps(temp, temp);
  printf("temporal_time_overlaps([1@2001-01-01, 2@2001-01-03], "
    "[1@2001-01-01, 2@2001-01-03]): %s, errno %d\n",
    overlaps ? "true" : "false", meos_errno());
  assert(overlaps);
  assert(meos_errno() == 0);

  free(temp);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
