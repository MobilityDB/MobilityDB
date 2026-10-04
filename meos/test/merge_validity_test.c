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
 * @brief A program that tests how the merge of temporal geometries and
 * geographies reports a union of the values sharing a timestamp that is
 * refused, under the noexit error handler
 * @details Merging two temporal values whose instants share a timestamp takes
 * the union of their values at that timestamp. Where the union is refused --
 * a temporal geography answers it for positions only -- the merge reports
 * the refusal as an error and answers no value, as it does for any other pair
 * of instants it cannot merge.
 *
 * The program verifies that #temporal_merge of two temporal geographies
 * holding lines at one timestamp returns NULL with #meos_errno set to
 * MEOS_ERR_FEATURE_NOT_SUPPORTED, and that the merges the union answers --
 * two geography points, two geometry polygons -- still answer with no error
 * left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o merge_validity_test merge_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Two geography lines at one timestamp: the union is refused, and so is
   * the merge */
  Temporal *line1 = tgeography_in("Linestring(0 0,1 1)@2001-01-01");
  Temporal *line2 = tgeography_in("Linestring(1 0,0 1)@2001-01-01");
  meos_errno_reset();
  Temporal *result = temporal_merge(line1, line2);
  printf("merge of two geography lines at one timestamp: %s, errno %d\n",
    result ? "a value" : "NULL", meos_errno());
  assert(result == NULL);
  assert(meos_errno() == MEOS_ERR_FEATURE_NOT_SUPPORTED);
  meos_errno_reset();

  /* Two geography points at one timestamp: the union answers */
  Temporal *point1 = tgeography_in("Point(1 1)@2001-01-01");
  Temporal *point2 = tgeography_in("Point(2 2)@2001-01-01");
  result = temporal_merge(point1, point2);
  printf("merge of two geography points at one timestamp: %s, errno %d\n",
    result ? "a value" : "NULL", meos_errno());
  assert(result != NULL);
  assert(meos_errno() == 0);
  free(result);

  /* Two geometry polygons at one timestamp: the union answers */
  Temporal *poly1 = tgeometry_in("Polygon((0 0,1 0,1 1,0 1,0 0))@2001-01-01");
  Temporal *poly2 = tgeometry_in("Polygon((5 5,6 5,6 6,5 6,5 5))@2001-01-01");
  result = temporal_merge(poly1, poly2);
  printf("merge of two geometry polygons at one timestamp: %s, errno %d\n",
    result ? "a value" : "NULL", meos_errno());
  assert(result != NULL);
  assert(meos_errno() == 0);
  free(result);

  free(line1); free(line2); free(point1); free(point2);
  free(poly1); free(poly2);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
