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
 * @brief A program that checks the operators a span shares with a temporal
 * box: a span answers them as its own box does
 * @details The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o span_tbox_ops_test span_tbox_ops_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Report the answer of a predicate and check the error code it leaves */
static void
check(const char *call, bool result, bool expected, int errcode)
{
  printf("%s: %s, errno %d\n", call, result ? "true" : "false",
    meos_errno());
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
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  TBox *box = tbox_in("TBOXINT XT([3, 8),[2001-01-01, 2001-01-02])");
  TBox *bigintbox = tbox_in("TBOXBIGINT XT([3, 8),[2001-01-01, 2001-01-02])");
  TBox *timebox = tbox_in("TBOX T([2001-01-01, 2001-01-02])");
  Span *s15 = intspan_in("[1, 5)");
  Span *s38 = intspan_in("[3, 8)");
  Span *s89 = intspan_in("[8, 9)");
  Span *bs15 = bigintspan_in("[1, 5)");
  Span *fs15 = floatspan_in("[1, 5)");
  Span *ts13 = tstzspan_in("[2001-01-01, 2001-01-03]");
  assert(box && bigintbox && timebox && s15 && s38 && s89 && bs15 && fs15 &&
    ts13);
  meos_errno_reset();

  /* A number span answers on the value dimension of the box */
  check("overlaps_numspan_tbox([1,5), box)",
    overlaps_numspan_tbox(s15, box), true, 0);
  check("overlaps_tbox_numspan(box, [1,5))",
    overlaps_tbox_numspan(box, s15), true, 0);
  check("contains_numspan_tbox([1,5), box)",
    contains_numspan_tbox(s15, box), false, 0);
  check("contained_numspan_tbox([1,5), box)",
    contained_numspan_tbox(s15, box), false, 0);
  check("contains_tbox_numspan(box, [1,5))",
    contains_tbox_numspan(box, s15), false, 0);
  check("same_numspan_tbox([1,5), box)",
    same_numspan_tbox(s15, box), false, 0);
  check("adjacent_numspan_tbox([1,5), box)",
    adjacent_numspan_tbox(s15, box), false, 0);
  check("same_numspan_tbox([3,8), box)",
    same_numspan_tbox(s38, box), true, 0);
  check("contains_numspan_tbox([3,8), box)",
    contains_numspan_tbox(s38, box), true, 0);
  check("contained_tbox_numspan(box, [3,8))",
    contained_tbox_numspan(box, s38), true, 0);
  check("adjacent_numspan_tbox([8,9), box)",
    adjacent_numspan_tbox(s89, box), true, 0);
  check("adjacent_tbox_numspan(box, [8,9))",
    adjacent_tbox_numspan(box, s89), true, 0);

  /* ... and so do its positions */
  check("left_numspan_tbox([1,5), box)",
    left_numspan_tbox(s15, box), false, 0);
  check("overleft_numspan_tbox([1,5), box)",
    overleft_numspan_tbox(s15, box), true, 0);
  check("overright_numspan_tbox([1,5), box)",
    overright_numspan_tbox(s15, box), false, 0);
  check("overright_tbox_numspan(box, [1,5))",
    overright_tbox_numspan(box, s15), true, 0);
  check("right_numspan_tbox([8,9), box)",
    right_numspan_tbox(s89, box), true, 0);
  check("left_tbox_numspan(box, [8,9))",
    left_tbox_numspan(box, s89), true, 0);

  /* A big integer span against a big integer box */
  check("overlaps_numspan_tbox(bigint [1,5), bigint box)",
    overlaps_numspan_tbox(bs15, bigintbox), true, 0);

  /* A timestamptz span answers on the time dimension of the box */
  check("overlaps_tstzspan_tbox(ts, box)",
    overlaps_tstzspan_tbox(ts13, box), true, 0);
  check("contains_tstzspan_tbox(ts, box)",
    contains_tstzspan_tbox(ts13, box), true, 0);
  check("contained_tbox_tstzspan(box, ts)",
    contained_tbox_tstzspan(box, ts13), true, 0);
  check("contained_tstzspan_tbox(ts, box)",
    contained_tstzspan_tbox(ts13, box), false, 0);
  check("before_tstzspan_tbox(ts, box)",
    before_tstzspan_tbox(ts13, box), false, 0);
  check("overbefore_tstzspan_tbox(ts, box)",
    overbefore_tstzspan_tbox(ts13, box), false, 0);
  check("overafter_tstzspan_tbox(ts, box)",
    overafter_tstzspan_tbox(ts13, box), true, 0);
  check("overbefore_tbox_tstzspan(box, ts)",
    overbefore_tbox_tstzspan(box, ts13), true, 0);
  check("same_tstzspan_tbox(ts, time box)",
    same_tstzspan_tbox(ts13, timebox), false, 0);

  /* A span of another number than the box is refused, as two boxes of
   * different numbers are */
  check("overlaps_numspan_tbox(float [1,5), int box)",
    overlaps_numspan_tbox(fs15, box), false, MEOS_ERR_INVALID_ARG_TYPE);
  /* Each function takes the span of its name */
  check("overlaps_tstzspan_tbox([1,5), box)",
    overlaps_tstzspan_tbox(s15, box), false, MEOS_ERR_INVALID_ARG_TYPE);
  check("overlaps_numspan_tbox(ts, box)",
    overlaps_numspan_tbox(ts13, box), false, MEOS_ERR_INVALID_ARG_TYPE);
  /* A box without the dimension of the span shares nothing with it */
  check("overlaps_numspan_tbox([1,5), time box)",
    overlaps_numspan_tbox(s15, timebox), false, MEOS_ERR_INVALID_ARG_VALUE);
  check("left_numspan_tbox([1,5), time box)",
    left_numspan_tbox(s15, timebox), false, MEOS_ERR_INVALID_ARG_VALUE);
  /* A null operand is reported rather than read */
  check("overlaps_numspan_tbox(NULL, box)",
    overlaps_numspan_tbox(NULL, box), false, MEOS_ERR_INVALID_ARG);
  check("before_tbox_tstzspan(NULL, ts)",
    before_tbox_tstzspan(NULL, ts13), false, MEOS_ERR_INVALID_ARG);

  free(box); free(bigintbox); free(timebox);
  free(s15); free(s38); free(s89); free(bs15); free(fs15); free(ts13);

  /* Finalize MEOS */
  meos_finalize();
  printf("span_tbox_ops_test: all checks passed\n");
  return EXIT_SUCCESS;
}
