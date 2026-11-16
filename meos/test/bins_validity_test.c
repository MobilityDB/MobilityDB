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
 * @brief A program that tests how the functions answering the bins of a span,
 * of a span set and of the time of a temporal value of the MEOS API report a
 * null out argument under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. The count and the bins a function answers
 * through its last arguments are such pointers.
 *
 * The program verifies that each of #intspan_bins, #bigintspan_bins,
 * #floatspan_bins, #datespan_bins, #tstzspan_bins, #intspanset_bins,
 * #bigintspanset_bins, #floatspanset_bins, #datespanset_bins and
 * #tstzspanset_bins reports a null count by answering NULL and setting
 * #meos_errno, that #temporal_time_split reports null bins the same way and
 * sets its count to 0, and that a valid call answers its bins with no error
 * left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o bins_validity_test bins_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Check that a bins function reports a null count and answers a valid call,
 * freeing the value it reads and the bins it answers */
#define CHECK_BINS(func, infunc, str, size, origin, nbins) \
  do { \
    void *value = (void *) infunc(str); \
    assert(value != NULL); \
    void *none = (void *) func(value, (size), (origin), true, NULL); \
    printf("%s(..., NULL): %s, errno %d\n", #func, none ? "bins" : "NULL", \
      meos_errno()); \
    assert(none == NULL); \
    assert(meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    int count = -1; \
    void *bins = (void *) func(value, (size), (origin), true, &count); \
    printf("%s(..., &count): %d bins, errno %d\n", #func, count, \
      meos_errno()); \
    assert(bins != NULL); \
    assert(count == (nbins)); \
    assert(meos_errno() == 0); \
    free(bins); \
    free(value); \
  } while (0)

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Interval *day = interval_in("1 day", -1);
  assert(day);

  CHECK_BINS(intspan_bins, intspan_in, "[0, 4)", 2, 0, 2);
  CHECK_BINS(bigintspan_bins, bigintspan_in, "[0, 4)", 2, 0, 2);
  CHECK_BINS(floatspan_bins, floatspan_in, "[0, 4)", 2.0, 0.0, 2);
  CHECK_BINS(datespan_bins, datespan_in, "[2001-01-01, 2001-01-03)", day, 0,
    2);
  CHECK_BINS(tstzspan_bins, tstzspan_in, "[2001-01-01, 2001-01-03)", day, 0,
    2);
  CHECK_BINS(intspanset_bins, intspanset_in, "{[0, 4)}", 2, 0, 2);
  CHECK_BINS(bigintspanset_bins, bigintspanset_in, "{[0, 4)}", 2, 0, 2);
  CHECK_BINS(floatspanset_bins, floatspanset_in, "{[0, 4)}", 2.0, 0.0, 2);
  CHECK_BINS(datespanset_bins, datespanset_in, "{[2001-01-01, 2001-01-03)}",
    day, 0, 2);
  CHECK_BINS(tstzspanset_bins, tstzspanset_in, "{[2001-01-01, 2001-01-03)}",
    day, 0, 2);

  /* The time split reports null bins and leaves its count defined */
  Temporal *temp = tint_in("[1@2001-01-01, 2@2001-01-02, 2@2001-01-03)");
  assert(temp);
  int count = -1;
  Temporal **none = temporal_time_split(temp, day, 0, true, NULL, &count);
  printf("temporal_time_split(..., NULL, &count): %s, count %d, errno %d\n",
    none ? "fragments" : "NULL", count, meos_errno());
  assert(none == NULL); assert(count == 0);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  TimestampTz *bins = NULL;
  Temporal **frags = temporal_time_split(temp, day, 0, true, &bins, &count);
  printf("temporal_time_split(..., &bins, &count): %d fragments, errno %d\n",
    count, meos_errno());
  assert(frags != NULL); assert(bins != NULL); assert(count == 2);
  assert(meos_errno() == 0);
  for (int i = 0; i < count; i++)
    free(frags[i]);
  free(frags);
  free(bins);
  free(temp);
  free(day);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
