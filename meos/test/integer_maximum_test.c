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
 * @brief A program that tests that a temporal integer holds no value at the
 * maximum of its base type
 * @details Each operation is called once with a result at the maximum of its
 * base type, which must answer NULL and report an error through meos_errno,
 * and once with a result just below it, which must answer and report none:
 * the input of an instant, a sequence and a sequence set, the construction of
 * an instant, the addition to a sequence and to a sequence set, the shift of
 * an instant and of a sequence, the delta of a sequence and the temporal sum.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o integer_maximum_test integer_maximum_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

static int failures;

/* Report whether a call answered and reported an error, and count an
 * unexpected answer */
static void
intmaxcheck_report(const char *what, Temporal *result, bool expect_error)
{
  bool error = meos_errno() != 0;
  printf("%s: %s, %s\n", what, result ? "answer" : "no answer",
    error ? "error reported" : "no error");
  if (error != expect_error || (result == NULL) != expect_error)
  {
    printf("  expected %s\n", expect_error ? "no answer and an error" :
      "an answer and no error");
    failures++;
  }
  free(result);
  meos_errno_reset();
}

/* Return the temporal sum of two temporal integers */
static Temporal *
intmaxcheck_tsum(const char *str1, const char *str2)
{
  Temporal *temp1 = tint_in(str1), *temp2 = tint_in(str2);
  SkipList *state = tint_tsum_transfn(NULL, temp1);
  state = tint_tsum_transfn(state, temp2);
  free(temp1); free(temp2);
  return state ? temporal_tagg_finalfn(state) : NULL;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  meos_errno_reset();

  intmaxcheck_report("tint INT_MAX instant",
    tint_in("2147483647@2001-01-01"), true);
  intmaxcheck_report("tint INT_MAX - 1 instant",
    tint_in("2147483646@2001-01-01"), false);
  intmaxcheck_report("tbigint INT64_MAX instant",
    tbigint_in("9223372036854775807@2001-01-01"), true);
  intmaxcheck_report("tint INT_MAX in a sequence",
    tint_in("[1@2001-01-01, 2147483647@2001-01-02]"), true);
  intmaxcheck_report("tint INT_MAX in a sequence set",
    tint_in("{[2147483647@2001-01-01], [1@2001-01-02]}"), true);
  intmaxcheck_report("tintinst_make INT_MAX",
    (Temporal *) tintinst_make(INT_MAX, 0), true);
  intmaxcheck_report("tintinst_make INT_MAX - 1",
    (Temporal *) tintinst_make(INT_MAX - 1, 0), false);

  Temporal *seq = tint_in("[2147483646@2001-01-01, 1@2001-01-02]");
  Temporal *ss = tint_in("{[1@2001-01-01], [2147483646@2001-01-02]}");
  Temporal *low = tint_in("[1@2001-01-01, 2147483645@2001-01-02]");
  intmaxcheck_report("sequence + 1 reaching INT_MAX", add_tint_int(seq, 1),
    true);
  intmaxcheck_report("sequence set + 1 reaching INT_MAX", add_tint_int(ss, 1),
    true);
  intmaxcheck_report("sequence + 1 below INT_MAX", add_tint_int(low, 1),
    false);
  intmaxcheck_report("sequence shifted to INT_MAX", tint_shift_value(seq, 1),
    true);
  intmaxcheck_report("sequence set shifted to INT_MAX",
    tint_shift_value(ss, 1), true);
  intmaxcheck_report("sequence shifted below INT_MAX",
    tint_shift_value(low, 1), false);
  free(seq); free(ss); free(low);

  Temporal *inst = tint_in("2147483646@2001-01-01");
  intmaxcheck_report("instant shifted to INT_MAX", tint_shift_value(inst, 1),
    true);
  intmaxcheck_report("instant shifted below INT_MAX",
    tint_shift_value(inst, -1), false);
  free(inst);

  Temporal *delta = tint_in("[-1@2001-01-01, 2147483646@2001-01-02, 0@2001-01-03]");
  intmaxcheck_report("delta reaching INT_MAX", tnumber_delta_value(delta),
    true);
  free(delta);
  delta = tint_in("[0@2001-01-01, 2147483646@2001-01-02, 0@2001-01-03]");
  intmaxcheck_report("delta below INT_MAX", tnumber_delta_value(delta), false);
  free(delta);

  intmaxcheck_report("tsum reaching INT_MAX",
    intmaxcheck_tsum("[2147483646@2001-01-01, 0@2001-01-02]",
      "[1@2001-01-01, 0@2001-01-02]"), true);
  intmaxcheck_report("tsum below INT_MAX",
    intmaxcheck_tsum("[2147483645@2001-01-01, 0@2001-01-02]",
      "[1@2001-01-01, 0@2001-01-02]"), false);

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
