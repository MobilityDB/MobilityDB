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
 * @brief Test how temporal restrictions, time overlaps, the timestamp of an
 * instant and the boxes of a temporal number report an invalid argument
 * @details A program that tests how the temporal restrictions to the instants
 * before or after a timestamptz, the test of whether the time of two temporal
 * values overlaps, the timestamp of a temporal instant, and the value and time
 * boxes of a temporal integer or float report an invalid argument under the
 * noexit error handler.
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
 * receive wrongly. The program also verifies that #tinstant_timestamptz
 * reports a null value and a temporal sequence, the subtype it does not read,
 * and answers the timestamp of an instant, and that #tint_value_boxes and
 * #tfloat_time_boxes report a null value, a null count and a value of the
 * other temporal number type, and answer the boxes of a value of their own.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_validity_test temporal_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_interval.h>

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

  /* The timestamp of an instant is read from an instant alone: a null value
   * and a sequence are reported with the error value DT_NOEND, the largest
   * 64-bit integer, and an instant answers its timestamp */
  TimestampTz ts = tinstant_timestamptz(NULL);
  printf("tinstant_timestamptz(NULL): errno %d\n", meos_errno());
  assert(ts == INT64_MAX);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  ts = tinstant_timestamptz((const TInstant *) temp);
  printf("tinstant_timestamptz([1@2001-01-01, 2@2001-01-03]): errno %d\n",
    meos_errno());
  assert(ts == INT64_MAX);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  Temporal *inst = tint_in("1@2001-01-02");
  assert(inst);
  ts = tinstant_timestamptz((const TInstant *) inst);
  printf("tinstant_timestamptz(1@2001-01-02): errno %d\n", meos_errno());
  assert(ts == temporal_start_timestamptz(inst));
  assert(meos_errno() == 0);
  free(inst);

  /* The value and time boxes of a temporal integer or float are computed from
   * a value of that type alone: a null value, a null count and a value of the
   * other type are reported, and a value of the type answers its boxes */
  int count;
  TBox *boxes = tint_value_boxes(NULL, 1, 0, true, &count);
  printf("tint_value_boxes(NULL, true): %s, errno %d\n",
    boxes ? "boxes" : "NULL", meos_errno());
  assert(boxes == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  boxes = tint_value_boxes(temp, 1, 0, true, NULL);
  printf("tint_value_boxes(count NULL, true): %s, errno %d\n",
    boxes ? "boxes" : "NULL", meos_errno());
  assert(boxes == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  Temporal *tfloat = tfloat_in("[1.5@2001-01-01, 2.5@2001-01-03]");
  assert(tfloat);
  boxes = tint_value_boxes(tfloat, 1, 0, true, &count);
  printf("tint_value_boxes(tfloat, true): %s, errno %d\n",
    boxes ? "boxes" : "NULL", meos_errno());
  assert(boxes == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  Interval *day = interval_in("1 day", -1);
  boxes = tfloat_time_boxes(temp, day, t, true, &count);
  printf("tfloat_time_boxes(tint, true): %s, errno %d\n",
    boxes ? "boxes" : "NULL", meos_errno());
  assert(boxes == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();
  boxes = tint_value_boxes(temp, 1, 0, true, &count);
  printf("tint_value_boxes([1@2001-01-01, 2@2001-01-03], 1, true): %d boxes, "
    "errno %d\n", boxes ? count : 0, meos_errno());
  assert(boxes != NULL && count == 2);
  assert(meos_errno() == 0);
  free(boxes);
  boxes = tfloat_time_boxes(tfloat, day, t, true, &count);
  printf("tfloat_time_boxes([1.5@2001-01-01, 2.5@2001-01-03], 1 day, true): "
    "%d boxes, errno %d\n", boxes ? count : 0, meos_errno());
  assert(boxes != NULL && count == 3);
  assert(meos_errno() == 0);
  free(boxes); free(day); free(tfloat);

  free(temp);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
