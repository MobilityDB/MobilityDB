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
 * @brief A program that tests the error status functions of the MEOS API.
 *
 * The error status is only observable once
 * #meos_initialize_noexit_error_handler is installed — the handler every
 * language binding uses, since a binding must return an exception to its host
 * rather than terminate it. The program verifies that a failing call sets
 * #meos_errno, that #meos_errno_reset clears it and returns its previous
 * value, and that #meos_errno_restore puts back a stored value, which is the
 * sequence documented for monitoring a section of code for new errors.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o error_test error_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
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

  /* A fresh error status is clear */
  assert(meos_errno() == 0);

  /* A failing call sets the error status */
  Temporal *bad = tfloat_in("this is not a temporal float");
  assert(bad == NULL);
  int failed_errno = meos_errno();
  printf("errno after a failed call: %d\n", failed_errno);
  assert(failed_errno != 0);

  /* Resetting returns the previous value and clears the status */
  int previous = meos_errno_reset();
  printf("meos_errno_reset() returned %d, errno is now %d\n", previous,
    meos_errno());
  assert(previous == failed_errno);
  assert(meos_errno() == 0);

  /* A succeeding call leaves the status clear */
  Temporal *good = tfloat_in("{77@2000-01-01}");
  assert(good != NULL);
  char *good_out = tfloat_out(good, 6);
  printf("tfloat_in(\"{77@2000-01-01}\"): %s, errno %d\n", good_out,
    meos_errno());
  assert(meos_errno() == 0);

  /* Restoring puts back a stored value, so that a function which reset the
   * status to monitor a section of code can hand the caller's error back */
  meos_errno_restore(failed_errno);
  printf("meos_errno_restore(%d): errno is now %d\n", failed_errno,
    meos_errno());
  assert(meos_errno() == failed_errno);

  /* And the status can be cleared again afterwards */
  assert(meos_errno_reset() == failed_errno);
  assert(meos_errno() == 0);

  free(good); free(good_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
