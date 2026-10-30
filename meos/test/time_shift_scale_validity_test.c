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
 * @brief A program that tests how the time shifts and scales of the MEOS API
 * report a null interval under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it can
 * raise in its host language. An assertion cannot carry that contract: it is
 * compiled out under NDEBUG, which leaves the release build a binding links
 * against with no check at all.
 *
 * The program verifies that #tstzset_shift, #tstzset_scale and
 * #tstzset_shift_scale, and #tstzspan_shift, #tstzspan_scale and
 * #tstzspan_shift_scale, report each null interval they take by returning NULL
 * and setting #meos_errno, and that each answers the shifted and scaled value
 * with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o time_shift_scale_validity_test time_shift_scale_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Interval *shift = interval_in("1 day", -1);
  Interval *duration = interval_in("4 days", -1);
  Set *s = tstzset_in("{2001-01-01, 2001-01-02, 2001-01-03}");
  assert(shift && duration && s);
  meos_errno_reset();

  /* A null interval is reported rather than read as no shift or no scale */
  Set *res = tstzset_shift(s, NULL);
  printf("tstzset_shift(NULL): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  res = tstzset_scale(s, NULL);
  printf("tstzset_scale(NULL): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  res = tstzset_shift_scale(s, shift, NULL);
  printf("tstzset_shift_scale(1 day, NULL): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  res = tstzset_shift_scale(s, NULL, duration);
  printf("tstzset_shift_scale(NULL, 4 days): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* The intervals answer the shifted and scaled set */
  res = tstzset_shift(s, shift);
  char *out = tstzset_out(res);
  printf("tstzset_shift(1 day): %s, errno %d\n", out, meos_errno());
  assert(res && meos_errno() == 0);
  free(out); free(res);
  res = tstzset_scale(s, duration);
  out = tstzset_out(res);
  printf("tstzset_scale(4 days): %s, errno %d\n", out, meos_errno());
  assert(res && meos_errno() == 0);
  free(out); free(res);
  res = tstzset_shift_scale(s, shift, duration);
  out = tstzset_out(res);
  printf("tstzset_shift_scale(1 day, 4 days): %s, errno %d\n", out,
    meos_errno());
  assert(res && meos_errno() == 0);
  free(out); free(res);

  /* A null interval given to a timestamptz span is reported */
  Span *sp = tstzspan_in("[2001-01-01, 2001-01-03]");
  assert(sp);
  Span *spres = tstzspan_shift(sp, NULL);
  printf("tstzspan_shift(NULL): %s, errno %d\n", spres ? "a value" : "NULL",
    meos_errno());
  assert(spres == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  spres = tstzspan_scale(sp, NULL);
  printf("tstzspan_scale(NULL): %s, errno %d\n", spres ? "a value" : "NULL",
    meos_errno());
  assert(spres == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  spres = tstzspan_shift_scale(sp, shift, NULL);
  printf("tstzspan_shift_scale(1 day, NULL): %s, errno %d\n",
    spres ? "a value" : "NULL", meos_errno());
  assert(spres == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  spres = tstzspan_shift_scale(sp, NULL, duration);
  printf("tstzspan_shift_scale(NULL, 4 days): %s, errno %d\n",
    spres ? "a value" : "NULL", meos_errno());
  assert(spres == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* The intervals answer the shifted and scaled span */
  spres = tstzspan_shift(sp, shift);
  out = tstzspan_out(spres);
  printf("tstzspan_shift(1 day): %s, errno %d\n", out, meos_errno());
  assert(spres && meos_errno() == 0);
  free(out); free(spres);
  spres = tstzspan_scale(sp, duration);
  out = tstzspan_out(spres);
  printf("tstzspan_scale(4 days): %s, errno %d\n", out, meos_errno());
  assert(spres && meos_errno() == 0);
  free(out); free(spres);
  spres = tstzspan_shift_scale(sp, shift, duration);
  out = tstzspan_out(spres);
  printf("tstzspan_shift_scale(1 day, 4 days): %s, errno %d\n", out,
    meos_errno());
  assert(spres && meos_errno() == 0);
  free(out); free(spres);

  free(sp); free(s); free(shift); free(duration);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
