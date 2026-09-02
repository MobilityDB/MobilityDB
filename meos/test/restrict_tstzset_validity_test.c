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
 * @brief A program that tests how the timestamptz-set restrictions of the MEOS
 * API report an invalid argument under the noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer or a set of the wrong type receives an
 * error it can raise in its host language. An assertion cannot carry that
 * contract: it is compiled out under NDEBUG, which leaves the release build a
 * binding links against with no check at all, and the internal
 * #temporal_restrict_tstzset the restrictions delegate to states its
 * preconditions that way.
 *
 * The program verifies that #temporal_at_tstzset and #temporal_minus_tstzset
 * report a null temporal value, a null set, and a set that is not a timestamptz
 * set, by returning NULL and setting #meos_errno — and that a valid restriction
 * still answers with no error left behind. Their siblings over a timestamptz
 * span already carry the contract and are asserted beside them, so the program
 * states the family's behaviour rather than two functions of it.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o restrict_tstzset_validity_test restrict_tstzset_validity_test.c -L/usr/local/lib -lmeos
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

  Temporal *temp = tint_in("[1@2001-01-01, 2@2001-01-03]");
  Set *tstzset = tstzset_in("{2001-01-01, 2001-01-02}");
  Set *intset = intset_in("{1, 2}");
  Span *tstzspan = tstzspan_in("[2001-01-01, 2001-01-02]");
  assert(temp); assert(tstzset); assert(intset); assert(tstzspan);

  /* A null temporal value is reported rather than dereferenced */
  meos_errno_reset();
  Temporal *res = temporal_at_tstzset(NULL, tstzset);
  printf("temporal_at_tstzset(NULL, {2001-01-01, 2001-01-02}): NULL, errno %d\n",
    meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  meos_errno_reset();
  res = temporal_minus_tstzset(NULL, tstzset);
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  /* A null set is reported alike */
  meos_errno_reset();
  res = temporal_at_tstzset(temp, NULL);
  printf("temporal_at_tstzset([1@2001-01-01, 2@2001-01-03], NULL): NULL, "
    "errno %d\n", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  meos_errno_reset();
  res = temporal_minus_tstzset(temp, NULL);
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  /* A set that is not a timestamptz set is reported as a type error: the
   * restriction reads the set's elements as timestamps, so accepting an
   * integer set would read one type's bytes as another's */
  meos_errno_reset();
  res = temporal_at_tstzset(temp, intset);
  printf("temporal_at_tstzset([1@2001-01-01, 2@2001-01-03], {1, 2}): NULL, "
    "errno %d\n", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);

  meos_errno_reset();
  res = temporal_minus_tstzset(temp, intset);
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);

  /* The siblings over a timestamptz span answer the same way */
  meos_errno_reset();
  res = temporal_at_tstzspan(NULL, tstzspan);
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  meos_errno_reset();
  res = temporal_minus_tstzspan(temp, NULL);
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  /* A valid restriction still answers, and the guards leave no error behind */
  meos_errno_reset();
  res = temporal_at_tstzset(temp, tstzset);
  assert(res != NULL);
  assert(meos_errno() == 0);
  char *out = tint_out(res);
  printf("temporal_at_tstzset([1@2001-01-01, 2@2001-01-03], "
    "{2001-01-01, 2001-01-02}): %s\n", out);
  free(out); free(res);

  meos_errno_reset();
  res = temporal_minus_tstzset(temp, tstzset);
  assert(res != NULL);
  assert(meos_errno() == 0);
  free(res);

  free(temp); free(tstzset); free(intset); free(tstzspan);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
