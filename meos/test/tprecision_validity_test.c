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
 * @brief A program that tests how the function setting the time precision of
 * a timestamptz set of the MEOS API reports an invalid set under the noexit
 * error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer or a value of another
 * type receives an error it can raise in its host language.
 *
 * The program verifies that #tstzset_tprecision reports a null set and a set
 * of integers by answering NULL and setting #meos_errno, as
 * #tstzspan_tprecision reports a null span, and that a timestamptz set
 * answers its values set to the precision with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tprecision_validity_test tprecision_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Report the answer of a call and check the error code it leaves */
static void
check(const char *call, const Set *result, bool answered, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "a set" : "NULL",
    meos_errno());
  assert((result != NULL) == answered);
  assert(meos_errno() == errcode);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Interval *day = interval_in("1 day", -1);
  Set *ints = intset_in("{1, 2}");
  Set *times = tstzset_in("{2001-01-01 08:00, 2001-01-02 08:00}");
  assert(day); assert(ints); assert(times);

  /* A null set and a set of another type are reported rather than read */
  check("tstzset_tprecision(NULL, day, 0)",
    tstzset_tprecision(NULL, day, 0), false, MEOS_ERR_INVALID_ARG);
  Set *wrong = tstzset_tprecision(ints, day, 0);
  printf("tstzset_tprecision(intset, day, 0): %s, errno %d\n",
    wrong ? "a set" : "NULL", meos_errno());
  assert(wrong == NULL); assert(meos_errno() != 0);
  meos_errno_reset();

  /* A timestamptz set answers its values set to the precision */
  Set *result = tstzset_tprecision(times, day, 0);
  check("tstzset_tprecision(tstzset, day, 0)", result, true, 0);
  Set *expected = tstzset_in("{2001-01-01, 2001-01-02}");
  assert(expected);
  bool equal = set_eq(result, expected);
  printf("the answer is {2001-01-01, 2001-01-02}: %s\n",
    equal ? "true" : "false");
  assert(equal);

  free(expected);
  free(result);
  free(times);
  free(ints);
  free(day);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
