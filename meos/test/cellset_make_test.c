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
 * @brief A program that tests the typed constructors of the cell index sets.
 *
 * Each set type is built from an array of its values through a typed public
 * function that tests its arguments, as intset_make does for an integer set.
 * The program verifies that quadbinset_make, s2cellset_make and
 * h3indexset_make answer the set their text reads, ordered and with each cell
 * once, with no error left behind, and that each reports a null array with
 * MEOS_ERR_INVALID_ARG and an empty one with MEOS_ERR_INVALID_ARG_VALUE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cellset_make_test cellset_make_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/* The set built from the values writes what the set read from the text
 * writes */
#define CHECK_MAKE(fn, values, count, in, out, text) \
  do { \
    Set *made = fn(values, count); \
    Set *read = in(text); \
    char *s1 = made ? out(made) : NULL; \
    char *s2 = out(read); \
    printf(#fn ": %s, errno %d\n", s1 ? s1 : "NULL", meos_errno()); \
    assert(s1 && s2 && strcmp(s1, s2) == 0 && meos_errno() == 0); \
    free(s1); free(s2); free(made); free(read); \
  } while (0)

/* The null array and the empty one are refused, each with its own error, and
 * leave no result behind */
#define CHECK_ERRORS(fn, values) \
  do { \
    Set *res = fn(NULL, 2); \
    printf(#fn "(NULL, 2): %s, errno %d\n", res ? "a value" : "NULL", \
      meos_errno()); \
    assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    res = fn(values, 0); \
    printf(#fn "(values, 0): %s, errno %d\n", res ? "a value" : "NULL", \
      meos_errno()); \
    assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE); \
    meos_errno_reset(); \
  } while (0)

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Each array holds its cells out of order and one of them twice */

  /* QUADBIN */
  Quadbin qv[] = {quadbin_in("48427fffffffffff"),
    quadbin_in("480fffffffffffff"), quadbin_in("48427fffffffffff")};
  CHECK_MAKE(quadbinset_make, qv, 3, quadbinset_in, quadbinset_out,
    "{480fffffffffffff, 48427fffffffffff}");
  CHECK_ERRORS(quadbinset_make, qv);

  /* S2 */
  S2CellId sv[] = {s2cell_in("54b5c9"), s2cell_in("47c3c3"),
    s2cell_in("54b5c9")};
  CHECK_MAKE(s2cellset_make, sv, 3, s2cellset_in, s2cellset_out,
    "{47c3c3, 54b5c9}");
  CHECK_ERRORS(s2cellset_make, sv);

  /* H3 */
  H3Index hv[] = {h3index_in("880326b88dfffff"),
    h3index_in("880326b885fffff"), h3index_in("880326b88dfffff")};
  CHECK_MAKE(h3indexset_make, hv, 3, h3indexset_in, h3indexset_out,
    "{880326b885fffff, 880326b88dfffff}");
  CHECK_ERRORS(h3indexset_make, hv);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
