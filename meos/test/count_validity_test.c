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
 * @brief A program that tests that a function answering an array through a
 * count leaves the count defined whichever argument it refuses, under the
 * noexit error handler
 * @details A public MEOS function answering an array writes the number of its
 * elements through its last argument. It tests that argument first and sets
 * it to zero, so that a caller reading the count of a refused call sees no
 * elements rather than whatever its variable held.
 *
 * The program verifies, for a function of each way the count was tested
 * after another argument, that a refused value leaves the count at zero and
 * a null count is reported: #set_spans and #spanset_spans, which tested their
 * value first; #tint_values, which tested its count on the line of its value;
 * and #stbox_space_tiles, whose callee tests the count after the function
 * tested its sizes. A valid call still answers with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o count_validity_test count_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* The value a count holds before a call, which no call answers */
#define UNSET -1

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Set *set = intset_in("{1, 3, 5}");
  SpanSet *ss = intspanset_in("{[1, 2], [4, 5]}");
  Temporal *tint = tint_in("[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]");
  STBox *box = stbox_in("STBOX X((0,0),(10,10))");
  assert(set && ss && tint && box);
  meos_errno_reset();

  /* A refused value leaves the count at zero */
  int count = UNSET;
  Span *spans = set_spans(NULL, &count);
  printf("set_spans(NULL, &count): %s, count %d, errno %d\n",
    spans ? "a value" : "NULL", count, meos_errno());
  assert(spans == NULL && count == 0 && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  count = UNSET;
  spans = spanset_spans(NULL, &count);
  printf("spanset_spans(NULL, &count): %s, count %d, errno %d\n",
    spans ? "a value" : "NULL", count, meos_errno());
  assert(spans == NULL && count == 0 && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  count = UNSET;
  int *values = tint_values(NULL, &count);
  printf("tint_values(NULL, &count): %s, count %d, errno %d\n",
    values ? "a value" : "NULL", count, meos_errno());
  assert(values == NULL && count == 0 &&
    meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  count = UNSET;
  STBox *tiles = stbox_space_tiles(box, NAN, 5.0, 5.0, NULL, true, &count);
  printf("stbox_space_tiles(box, NaN, ...): %s, count %d, errno %d\n",
    tiles ? "a value" : "NULL", count, meos_errno());
  assert(tiles == NULL && count == 0 && meos_errno() != 0);
  meos_errno_reset();

  /* A null count is reported rather than written through */
  spans = set_spans(set, NULL);
  printf("set_spans(set, NULL): %s, errno %d\n", spans ? "a value" : "NULL",
    meos_errno());
  assert(spans == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  values = tint_values(tint, NULL);
  printf("tint_values(tint, NULL): %s, errno %d\n",
    values ? "a value" : "NULL", meos_errno());
  assert(values == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  tiles = stbox_space_tiles(box, 5.0, 5.0, 5.0, NULL, true, NULL);
  printf("stbox_space_tiles(box, 5, ..., NULL): %s, errno %d\n",
    tiles ? "a value" : "NULL", meos_errno());
  assert(tiles == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A valid call answers its elements, and the guards leave no error */
  spans = set_spans(set, &count);
  assert(spans && count == 3 && meos_errno() == 0);
  free(spans);
  spans = spanset_spans(ss, &count);
  assert(spans && count == 2 && meos_errno() == 0);
  free(spans);
  values = tint_values(tint, &count);
  assert(values && count == 2 && meos_errno() == 0);
  free(values);
  /* Tiles of 5 over [0, 10] with the upper border included: 10 starts a
   * third tile along each axis */
  tiles = stbox_space_tiles(box, 5.0, 5.0, 5.0, NULL, true, &count);
  printf("stbox_space_tiles(box, 5, ...): %d tiles, errno %d\n", count,
    meos_errno());
  assert(tiles && count == 9 && meos_errno() == 0);
  free(tiles);

  free(set); free(ss); free(tint); free(box);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
