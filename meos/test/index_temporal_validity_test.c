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
 * @brief A program that tests that the functions indexing a temporal value in
 * an in-memory index test their arguments
 * @details Each function inserting a temporal value into an R-tree or an
 * SP-tree, or searching one with it, reports a null tree, a null value and a
 * value of another SRID than the stored boxes as an error and answers the
 * error return its documentation states, rather than reading the null
 * argument or answering as if nothing matched.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o index_temporal_validity_test index_temporal_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

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
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *tpt = tgeompoint_in("[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02, Point(3 1)@2001-01-03]");
  assert(tpt);
  Temporal *tpt3857 = tgeompoint_in("SRID=3857;[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02, Point(3 1)@2001-01-03]");
  assert(tpt3857);
  RTree *rtree = rtree_create_stbox();
  SPTree *sptree = sptree_create_stbox(SPTREE_QUADTREE);
  MeosArray *result = index_result_create();
  assert(rtree_insert_temporal_split(rtree, tpt, 1, 2));
  assert(sptree_insert_temporal_split(sptree, tpt, 1, 2));
  meos_errno_reset();

  /* A null tree or a null value is reported rather than read */
  check("rtree_insert_temporal(NULL, tpt, 2)",
    rtree_insert_temporal(NULL, tpt, 2), false, MEOS_ERR_INVALID_ARG);
  check("rtree_insert_temporal(rtree, NULL, 2)",
    rtree_insert_temporal(rtree, NULL, 2), false, MEOS_ERR_INVALID_ARG);
  check("rtree_insert_temporal_split(NULL, tpt, 2, 2)",
    rtree_insert_temporal_split(NULL, tpt, 2, 2), false, MEOS_ERR_INVALID_ARG);
  check("rtree_search_temporal(NULL, tpt)",
    rtree_search_temporal(NULL, INDEX_OVERLAPS, tpt, result), -1,
    MEOS_ERR_INVALID_ARG);
  check("rtree_search_temporal_dedup(rtree, NULL)",
    rtree_search_temporal_dedup(rtree, INDEX_OVERLAPS, NULL, 2, result), -1,
    MEOS_ERR_INVALID_ARG);
  check("sptree_insert_temporal(NULL, tpt, 2)",
    sptree_insert_temporal(NULL, tpt, 2), false, MEOS_ERR_INVALID_ARG);
  check("sptree_insert_temporal_split(sptree, NULL, 2, 2)",
    sptree_insert_temporal_split(sptree, NULL, 2, 2), false,
    MEOS_ERR_INVALID_ARG);
  check("sptree_search_temporal(sptree, NULL)",
    sptree_search_temporal(sptree, INDEX_OVERLAPS, NULL, result), -1,
    MEOS_ERR_INVALID_ARG);
  check("sptree_search_temporal_dedup(NULL, tpt)",
    sptree_search_temporal_dedup(NULL, INDEX_OVERLAPS, tpt, 2, result), -1,
    MEOS_ERR_INVALID_ARG);

  /* A value of another SRID than the stored boxes is reported once and
   * answers the error return, not an empty answer */
  check("rtree_search_temporal_dedup(rtree, tpt3857)",
    rtree_search_temporal_dedup(rtree, INDEX_OVERLAPS, tpt3857, 2, result), -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  check("sptree_search_temporal_dedup(sptree, tpt3857)",
    sptree_search_temporal_dedup(sptree, INDEX_OVERLAPS, tpt3857, 2, result),
    -1, MEOS_ERR_INVALID_ARG_VALUE);

  /* A valid search answers the stored value */
  check("rtree_search_temporal_dedup(rtree, tpt)",
    rtree_search_temporal_dedup(rtree, INDEX_OVERLAPS, tpt, 2, result), 1, 0);
  check("sptree_search_temporal_dedup(sptree, tpt)",
    sptree_search_temporal_dedup(sptree, INDEX_OVERLAPS, tpt, 2, result), 1, 0);

  meos_array_destroy(result);
  rtree_free(rtree);
  sptree_free(sptree);
  free(tpt); free(tpt3857);
  printf("Index temporal validity test: all tests passed\n");
  meos_finalize();
  return EXIT_SUCCESS;
}
