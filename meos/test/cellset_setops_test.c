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
 * @brief A program that tests the set operations of a cell index set and a
 * cell
 * @details Each cell set type answers the containment, union, intersection
 * and difference of a set and a cell, in both operand orders, through typed
 * public functions that test their set, as contains_set_bigint and
 * union_bigint_set do for a big integer set. The program verifies the answers
 * of contains_set_<cell>, contained_<cell>_set, union_set_<cell>,
 * union_<cell>_set, intersection_set_<cell>, intersection_<cell>_set,
 * minus_set_<cell> and minus_<cell>_set for quadbin, s2cell and h3index, and
 * that each family reports a null set with MEOS_ERR_INVALID_ARG and an
 * integer set with MEOS_ERR_INVALID_ARG_TYPE.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cellset_setops_test cellset_setops_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_quadbin.h>
#include <meos_s2cell.h>

/* The number of values of a set answer, the answer freed; an empty answer is
 * a NULL set with no error */
static int
count_free(Set *s)
{
  assert(meos_errno() == 0);
  if (! s)
    return 0;
  int n = set_num_values(s);
  free(s);
  return n;
}

/* Over the one-cell set {a} and the two-cell set {a, b}, every operation
 * answers as the set of its cells does, and a null set and an integer set
 * are refused, each with its own error */
#define CHECK_SETOPS(cell, one, two, a, b, other) \
  do { \
    assert(contains_set_##cell(one, a) && ! contains_set_##cell(one, b)); \
    assert(contained_##cell##_set(a, one) && ! contained_##cell##_set(b, one)); \
    assert(count_free(union_set_##cell(one, b)) == 2); \
    assert(count_free(union_##cell##_set(a, one)) == 1); \
    assert(count_free(intersection_set_##cell(two, a)) == 1); \
    assert(count_free(intersection_##cell##_set(b, one)) == 0); \
    assert(count_free(minus_set_##cell(two, a)) == 1); \
    assert(count_free(minus_##cell##_set(a, one)) == 0); \
    assert(count_free(minus_##cell##_set(b, one)) == 1); \
    printf(#cell ": the eight set operations answer, errno %d\n", \
      meos_errno()); \
    assert(! contains_set_##cell(NULL, a) && \
      meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    assert(! union_##cell##_set(a, other) && \
      meos_errno() == MEOS_ERR_INVALID_ARG_TYPE); \
    meos_errno_reset(); \
    assert(! minus_##cell##_set(a, NULL) && \
      meos_errno() == MEOS_ERR_INVALID_ARG); \
    meos_errno_reset(); \
    printf(#cell ": a null set and an integer set are refused\n"); \
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

  /* QUADBIN */
  Quadbin qa = quadbin_in("480fffffffffffff");
  Quadbin qb = quadbin_in("48427fffffffffff");
  Set *qone = quadbin_to_set(qa);
  Set *qtwo = quadbinset_in("{480fffffffffffff, 48427fffffffffff}");
  CHECK_SETOPS(quadbin, qone, qtwo, qa, qb, intset);
  /* The union aggregate of the two cells is the set of the two cells */
  Set *qagg = quadbin_union_transfn(NULL, qa);
  qagg = quadbin_union_transfn(qagg, qb);
  qagg = set_union_finalfn(qagg);
  assert(qagg && set_eq(qagg, qtwo));
  free(qagg);
  free(qone); free(qtwo);

  /* S2 */
  S2CellId sa = s2cell_in("47c3c3");
  S2CellId sb = s2cell_in("54b5c9");
  Set *sone = s2cell_to_set(sa);
  Set *stwo = s2cellset_in("{47c3c3, 54b5c9}");
  CHECK_SETOPS(s2cell, sone, stwo, sa, sb, intset);
  /* The union aggregate of the two cells is the set of the two cells */
  Set *sagg = s2cell_union_transfn(NULL, sa);
  sagg = s2cell_union_transfn(sagg, sb);
  sagg = set_union_finalfn(sagg);
  assert(sagg && set_eq(sagg, stwo));
  free(sagg);
  free(sone); free(stwo);

  /* H3: the cells are the values of the set that reads them */
  Set *htwo = h3indexset_in("{880326b885fffff, 880326b88dfffff}");
  H3Index *hv = h3indexset_values(htwo, &count);
  assert(hv && count == 2);
  Set *hone = h3index_to_set(hv[0]);
  CHECK_SETOPS(h3index, hone, htwo, hv[0], hv[1], intset);
  /* The union aggregate of the two cells is the set of the two cells */
  Set *hagg = h3index_union_transfn(NULL, hv[1]);
  hagg = h3index_union_transfn(hagg, hv[0]);
  hagg = set_union_finalfn(hagg);
  assert(hagg && set_eq(hagg, htwo));
  free(hagg);
  printf("the union aggregates of quadbin, S2 and H3 cells answer their sets\n");
  free(hone); free(htwo); free(hv);

  free(intset);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
