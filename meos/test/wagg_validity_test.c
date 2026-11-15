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
 * @brief A program that tests how the transition functions of the moving
 * aggregates of the MEOS API report an invalid interval under the noexit
 * error handler
 * @details The interval of a moving aggregate moves every timestamp forward:
 * none of its fields is negative and they are not all zero (#positive_window).
 * A public MEOS function tests its arguments at entry, so a binding calling
 * one of these functions with a null interval, or with one that moves a
 * timestamp back or leaves it in place, receives an error rather than a crash.
 *
 * The program verifies that #tint_wmin_transfn, #tbigint_wmin_transfn,
 * #tfloat_wmin_transfn, #tint_wmax_transfn, #tbigint_wmax_transfn,
 * #tfloat_wmax_transfn, #tint_wsum_transfn, #tbigint_wsum_transfn,
 * #tfloat_wsum_transfn, #tnumber_wavg_transfn and #temporal_wcount_transfn
 * report a null interval by returning NULL with #MEOS_ERR_INVALID_ARG, and a
 * zero interval, a negative one and one of mixed signs by returning NULL with
 * #MEOS_ERR_INVALID_ARG_VALUE, as #ensure_positive_duration refuses the
 * duration of a tile, and that an interval of a day and one of a month still
 * answer with no error left behind, as wsum_validity_test.c reads an answer.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o wagg_validity_test wagg_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

/* The transition function of a moving aggregate */
typedef SkipList *(*wagg_transfn)(SkipList *, const Temporal *,
  const Interval *);

/* Main program */
int main(void)
{
  size_t size_out;
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tint = tint_in("{1@2001-01-01, 2@2001-01-03}");
  Temporal *tbigint = tbigint_in("{1@2001-01-01, 2@2001-01-03}");
  Temporal *tfloat = tfloat_in("{1.5@2001-01-01, 2.5@2001-01-03}");
  assert(tint); assert(tbigint); assert(tfloat);

  static const char *names[] = { "tint_wmin_transfn", "tbigint_wmin_transfn",
    "tfloat_wmin_transfn", "tint_wmax_transfn", "tbigint_wmax_transfn",
    "tfloat_wmax_transfn", "tint_wsum_transfn", "tbigint_wsum_transfn",
    "tfloat_wsum_transfn", "tnumber_wavg_transfn",
    "temporal_wcount_transfn" };
  wagg_transfn fns[] = { &tint_wmin_transfn, &tbigint_wmin_transfn,
    &tfloat_wmin_transfn, &tint_wmax_transfn, &tbigint_wmax_transfn,
    &tfloat_wmax_transfn, &tint_wsum_transfn, &tbigint_wsum_transfn,
    &tfloat_wsum_transfn, &tnumber_wavg_transfn, &temporal_wcount_transfn };
  const Temporal *temps[] = { tint, tbigint, tfloat, tint, tbigint, tfloat,
    tint, tbigint, tfloat, tint, tint };
  /* An interval that moves a timestamp back or leaves it in place */
  static const char *refused[] = { "0 days", "-1 day", "1 month -40 days" };
  Interval *ivs[3];
  for (int j = 0; j < 3; j++)
  {
    ivs[j] = interval_in(refused[j], -1);
    assert(ivs[j]);
  }
  meos_errno_reset();

  for (int i = 0; i < 11; i++)
  {
    /* A null interval is reported rather than added to a timestamp */
    SkipList *state = fns[i](NULL, temps[i], NULL);
    printf("%s(NULL, temp, NULL): %s, errno %d\n", names[i],
      state ? "a state" : "NULL", meos_errno());
    assert(state == NULL); assert(meos_errno() == MEOS_ERR_INVALID_ARG);
    meos_errno_reset();
    for (int j = 0; j < 3; j++)
    {
      state = fns[i](NULL, temps[i], ivs[j]);
      printf("%s(NULL, temp, %s): %s, errno %d\n", names[i], refused[j],
        state ? "a state" : "NULL", meos_errno());
      assert(state == NULL);
      assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
      meos_errno_reset();
    }
  }

  /* An interval of a day and one of a month answer with no error left
   * behind */
  static const char *answered[] = { "1 day", "1 month" };
  static const char *expected[] = {
    "{[1@2001-01-01 00:00:00+00, 1@2001-01-02 00:00:00+00], "
      "[2@2001-01-03 00:00:00+00, 2@2001-01-04 00:00:00+00]}",
    "{[1@2001-01-01 00:00:00+00, 2@2001-01-03 00:00:00+00, "
      "2@2001-02-03 00:00:00+00]}" };
  for (int j = 0; j < 2; j++)
  {
    Interval *iv = interval_in(answered[j], -1);
    SkipList *state = tint_wmax_transfn(NULL, tint, iv);
    Temporal *res = state ? temporal_tagg_finalfn(state) : NULL;
    char *out = res ? temporal_out(res, 0, &size_out) : NULL;
    printf("tint_wmax_transfn(NULL, tint, %s): %s, errno %d\n", answered[j],
      out ? out : "NULL", meos_errno());
    assert(out && strcmp(out, expected[j]) == 0); assert(meos_errno() == 0);
    free(out); free(res); free(iv);
  }

  for (int j = 0; j < 3; j++)
    free(ivs[j]);
  free(tint); free(tbigint); free(tfloat);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
