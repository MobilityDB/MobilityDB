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
 * @brief A program that tests how the compaction and the uncompaction of a
 * set of H3 cells report an erroneous argument under the noexit error
 * handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null or mistyped set receives an error it can
 * raise in its host language, and receives the same error its sibling
 * functions report for the same condition.
 *
 * The program verifies that #h3_compact_cells and #h3_uncompact_cells report
 * a null set with MEOS_ERR_INVALID_ARG and a set of integers with
 * MEOS_ERR_INVALID_ARG_TYPE by returning NULL, and that the seven children of
 * a cell compact to that one cell and uncompact back to seven, with no error
 * left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o h3_validity_test h3_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_h3.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* A resolution 10 cell, its seven resolution 11 children and a set of
   * integers that is not a set of cells */
  H3Index parent = 0x8a1fb46622dffffULL;
  Set *children = h3_cell_to_children(parent, 11);
  Set *one = h3index_to_set(parent);
  Set *ints = intset_in("{1, 2}");
  assert(children && one && ints);
  assert(set_num_values(children) == 7);
  meos_errno_reset();

  /* A null set is reported rather than read */
  Set *res = h3_compact_cells(NULL);
  printf("h3_compact_cells(NULL): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  res = h3_uncompact_cells(NULL, 11);
  printf("h3_uncompact_cells(NULL, 11): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A set of integers is reported rather than read as a set of cells */
  res = h3_compact_cells(ints);
  printf("h3_compact_cells(intset): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  res = h3_uncompact_cells(ints, 11);
  printf("h3_uncompact_cells(intset, 11): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  /* The seven children compact to their parent and the parent uncompacts
   * back to its seven children, with no error left behind */
  res = h3_compact_cells(children);
  printf("h3_compact_cells(children): %d values, errno %d\n",
    res ? set_num_values(res) : -1, meos_errno());
  assert(res != NULL && set_num_values(res) == 1);
  assert(meos_errno() == 0);
  free(res);

  res = h3_uncompact_cells(one, 11);
  printf("h3_uncompact_cells(parent, 11): %d values, errno %d\n",
    res ? set_num_values(res) : -1, meos_errno());
  assert(res != NULL && set_num_values(res) == 7);
  assert(meos_errno() == 0);
  free(res);

  free(children); free(one); free(ints);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
