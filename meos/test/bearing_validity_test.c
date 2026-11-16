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
 * @brief A program that tests how the function answering the bearing of two
 * points of the MEOS API reports a null result under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. The bearing a function answers through its
 * last argument is such a pointer, as it is for #tpoint_direction.
 *
 * The program verifies that #bearing_point_point reports a null result by
 * answering false and setting #meos_errno, and that two points answer the
 * bearing from the first to the second with no error left behind.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o bearing_validity_test bearing_validity_test.c -L/usr/local/lib -lmeos
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
  meos_initialize_noexit_error_handler();

  GSERIALIZED *origin = geom_in("Point(0 0)", -1);
  GSERIALIZED *north = geom_in("Point(0 1)", -1);
  assert(origin); assert(north);

  /* A null result is reported rather than written */
  bool found = bearing_point_point(origin, north, NULL);
  printf("bearing_point_point(origin, north, NULL): %s, errno %d\n",
    found ? "true" : "false", meos_errno());
  assert(! found); assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* Two points answer the bearing from the first to the second */
  double bearing = -1.0;
  found = bearing_point_point(origin, north, &bearing);
  printf("bearing_point_point(origin, north, &bearing): %s, %g, errno %d\n",
    found ? "true" : "false", bearing, meos_errno());
  assert(found); assert(bearing == 0.0); assert(meos_errno() == 0);

  free(north);
  free(origin);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
