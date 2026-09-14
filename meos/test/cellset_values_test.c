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
 * @brief A program that tests the typed values functions of the cell index
 * sets.
 *
 * Each set type gives the array of its values through a typed public function
 * that tests its argument, as intset_values does for an integer set. The
 * program verifies that quadbinset_values, s2cellset_values and
 * h3indexset_values answer the values of the set in its order with no error
 * left behind, and that each reports a null set with MEOS_ERR_INVALID_ARG and
 * an integer set with MEOS_ERR_INVALID_ARG_TYPE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cellset_values_test cellset_values_test.c -L/usr/local/lib -lmeos
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

/* The null set and the set of another type are refused, each with its own
 * error, and leave no result behind */
#define CHECK_ERRORS(fn, other) \
  do { \
    int n = -1; \
    void *res = (void *) fn(NULL, &n); \
    printf(#fn "(NULL): %s, errno %d\n", res ? "a value" : "NULL", \
      meos_errno()); \
    assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    res = (void *) fn(other, &n); \
    printf(#fn "(intset): %s, errno %d\n", res ? "a value" : "NULL", \
      meos_errno()); \
    assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE); \
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

  Set *intset = intset_in("{1, 2}");
  assert(intset);
  int count;

  /* QUADBIN: the values are the cells the set reads, in its order */
  Set *qs = quadbinset_in("{480fffffffffffff, 48427fffffffffff}");
  Quadbin *qv = quadbinset_values(qs, &count);
  printf("quadbinset_values: %d values, errno %d\n", count, meos_errno());
  assert(qv && count == 2 && meos_errno() == 0);
  assert(qv[0] == quadbin_in("480fffffffffffff"));
  assert(qv[1] == quadbin_in("48427fffffffffff"));
  free(qv); free(qs);
  CHECK_ERRORS(quadbinset_values, intset);

  /* S2: the values are the cells the set reads, in its order */
  Set *ss = s2cellset_in("{47c3c3, 54b5c9}");
  S2CellId *sv = s2cellset_values(ss, &count);
  printf("s2cellset_values: %d values, errno %d\n", count, meos_errno());
  assert(sv && count == 2 && meos_errno() == 0);
  assert(sv[0] == s2cell_in("47c3c3"));
  assert(sv[1] == s2cell_in("54b5c9"));
  free(sv); free(ss);
  CHECK_ERRORS(s2cellset_values, intset);

  /* H3: each value, turned back into a one-cell set, writes the cell the set
   * reads at that position */
  Set *hs = h3indexset_in("{880326b885fffff, 880326b88dfffff}");
  H3Index *hv = h3indexset_values(hs, &count);
  printf("h3indexset_values: %d values, errno %d\n", count, meos_errno());
  assert(hv && count == 2 && meos_errno() == 0);
  const char *expected[] = {"{\"880326b885fffff\"}", "{\"880326b88dfffff\"}"};
  for (int i = 0; i < count; i++)
  {
    Set *one = h3index_to_set(hv[i]);
    char *str = h3indexset_out(one);
    printf("  value %d: %s\n", i, str);
    assert(str && strcmp(str, expected[i]) == 0);
    free(str); free(one);
  }
  free(hv); free(hs);
  CHECK_ERRORS(h3indexset_values, intset);

  free(intset);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
