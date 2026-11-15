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
 * @brief A program that tests how the functions adding to and counting an
 * expandable array of the MEOS API report an invalid argument under the
 * noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. An assertion cannot carry that contract: it
 * is compiled out under NDEBUG, which leaves the release build a binding links
 * against with no check at all.
 *
 * The program verifies that #meos_array_count reports a null array by
 * answering -1 and setting #meos_errno, that #meos_array_add reports a null
 * array, and a null value given to a fixed-size array, by setting #meos_errno
 * and adding nothing, that a varlength array stores a null pointer as any
 * other, and that a valid call still answers with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o array_validity_test array_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Report the answer of a call and check the error code it leaves */
static void
check(const char *call, int result, int expected, int errcode)
{
  printf("%s: %d, errno %d\n", call, result, meos_errno());
  assert(result == expected);
  assert(meos_errno() == errcode);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  MeosArray *ints = meos_array_create(sizeof(int));
  MeosArray *ptrs = meos_array_create(-1);
  assert(ints); assert(ptrs);
  int one = 1, two = 2;

  /* A null array is reported rather than read */
  check("meos_array_count(NULL)", meos_array_count(NULL), -1,
    MEOS_ERR_INVALID_ARG);
  meos_array_add(NULL, &one);
  check("meos_array_add(NULL, 1)", 0, 0, MEOS_ERR_INVALID_ARG);

  /* A fixed-size array copies the value, so a null value is reported and
   * nothing is added */
  meos_array_add(ints, NULL);
  printf("meos_array_add(ints, NULL): errno %d\n", meos_errno());
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  check("meos_array_count(ints)", meos_array_count(ints), 0, 0);

  /* A varlength array stores the pointer it is given, a null one included */
  meos_array_add(ptrs, NULL);
  meos_array_add(ptrs, &two);
  check("meos_array_count(ptrs)", meos_array_count(ptrs), 2, 0);
  void *first = meos_array_get(ptrs, 0);
  int *second = meos_array_get(ptrs, 1);
  printf("meos_array_get(ptrs, 0..1): %s %d, errno %d\n",
    first ? "a pointer" : "NULL", second ? *second : -1, meos_errno());
  assert(first == NULL); assert(second == &two); assert(meos_errno() == 0);

  /* A valid call adds and counts with no error left behind */
  meos_array_add(ints, &one);
  meos_array_add(ints, &two);
  check("meos_array_count(ints)", meos_array_count(ints), 2, 0);
  int *got = meos_array_get(ints, 1);
  printf("meos_array_get(ints, 1): %d, errno %d\n", got ? *got : -1,
    meos_errno());
  assert(got && *got == 2); assert(meos_errno() == 0);

  meos_array_destroy(ints);
  meos_array_destroy(ptrs);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
