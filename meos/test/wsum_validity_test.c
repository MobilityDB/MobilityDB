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
 * @brief A program that tests how the window sum of temporal floats reports
 * a value with continuous interpolation under the noexit error handler
 * @details The window sum of a temporal float is not supported over a
 * sequence or a sequence set with step or linear interpolation, and MEOS
 * refuses it, so a binding calling #tfloat_wsum_transfn receives the error
 * that PostgreSQL raises for `wSum` rather than an answer.
 *
 * The program verifies that #tfloat_wsum_transfn reports a sequence with
 * linear interpolation, a sequence with step interpolation and a sequence set
 * by returning NULL and setting #meos_errno, and that an instant and a
 * sequence with discrete interpolation still answer with no error left
 * behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o wsum_validity_test wsum_validity_test.c -L/usr/local/lib -lmeos
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

  Interval *interv = interval_in("1 day", -1);
  meos_errno_reset();

  /* A temporal float with continuous interpolation is reported rather than
   * answered */
  static const char *refused[] = {
    "[1@2001-01-01, 3@2001-01-03]",
    "Interp=Step;[1@2001-01-01, 3@2001-01-03]",
    "{[1@2001-01-01, 3@2001-01-03],[4@2001-01-04, 6@2001-01-06]}"};
  for (int i = 0; i < 3; i++)
  {
    Temporal *temp = tfloat_in(refused[i]);
    SkipList *state = tfloat_wsum_transfn(NULL, temp, interv);
    printf("tfloat_wsum_transfn(NULL, %s): %s, errno %d\n", refused[i],
      state ? "state" : "NULL", meos_errno());
    assert(state == NULL);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();
    free(temp);
  }

  /* An instant and a sequence with discrete interpolation still answer, and
   * the guard leaves no error behind */
  static const char *answered[] = {
    "1@2001-01-01",
    "{1@2001-01-01, 3@2001-01-03}"};
  for (int i = 0; i < 2; i++)
  {
    Temporal *temp = tfloat_in(answered[i]);
    SkipList *state = tfloat_wsum_transfn(NULL, temp, interv);
    Temporal *result = temporal_tagg_finalfn(state);
    char *result_out = result ? tfloat_out(result, 6) : NULL;
    printf("tfloat_wsum_transfn(NULL, %s): %s, errno %d\n", answered[i],
      result_out ? result_out : "NULL", meos_errno());
    assert(result != NULL);
    assert(meos_errno() == 0);
    free(temp); free(result); free(result_out);
  }

  free(interv);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
