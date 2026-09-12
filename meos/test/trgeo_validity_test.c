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
 * @brief A program that tests how the spatiotemporal boxes of a temporal
 * rigid geometry report an erroneous argument under the noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer or a value of another type receives
 * an error it can raise in its host language. An assertion cannot carry that
 * contract: it is compiled out under NDEBUG, which leaves the release build a
 * binding links against with no check at all.
 *
 * The program verifies that #trgeometry_stboxes reports a null temporal value,
 * a null count and a temporal value of another type by returning NULL, setting
 * #meos_errno and leaving the count at zero, and that a valid call still
 * answers with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o trgeo_validity_test trgeo_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_rgeo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *trgeo = trgeometry_in("Polygon((1 1,2 2,3 1,1 1));"
    "[Pose(Point(1 2),0.5)@2001-01-01, Pose(Point(2 3),0.5)@2001-01-02]");
  assert(trgeo);
  Temporal *tint = tint_in("[1@2001-01-01, 2@2001-01-03]");
  assert(tint);
  meos_errno_reset();

  /* A null temporal value is reported rather than dereferenced, and the
   * count is left at zero */
  int count = -1;
  STBox *boxes = trgeometry_stboxes(NULL, &count);
  printf("trgeometry_stboxes(NULL, &count): %s, count %d, errno %d\n",
    boxes ? "a value" : "NULL", count, meos_errno());
  assert(boxes == NULL);
  assert(count == 0);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A null count is reported rather than written through */
  boxes = trgeometry_stboxes(trgeo, NULL);
  printf("trgeometry_stboxes(trgeo, NULL): %s, errno %d\n",
    boxes ? "a value" : "NULL", meos_errno());
  assert(boxes == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A temporal value of another type is reported rather than read as a
   * temporal rigid geometry */
  count = -1;
  boxes = trgeometry_stboxes(tint, &count);
  printf("trgeometry_stboxes([1@2001-01-01, 2@2001-01-03], &count): %s, "
    "count %d, errno %d\n", boxes ? "a value" : "NULL", count, meos_errno());
  assert(boxes == NULL);
  assert(count == 0);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  /* A valid call still answers one box for the one segment of the sequence,
   * and the guards leave no error behind */
  boxes = trgeometry_stboxes(trgeo, &count);
  printf("trgeometry_stboxes(trgeo, &count): %s, count %d, errno %d\n",
    boxes ? "a value" : "NULL", count, meos_errno());
  assert(boxes != NULL);
  assert(count == 1);
  assert(meos_errno() == 0);
  free(boxes);

  free(trgeo);
  free(tint);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
