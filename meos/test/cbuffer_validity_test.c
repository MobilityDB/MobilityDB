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
 * @brief A program that tests how the ever/always relationships of a
 * geometry and a temporal circular buffer report an erroneous argument under
 * the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer or with operands in
 * two spatial reference systems receives an error it can raise in its host
 * language rather than a crash or an answer.
 *
 * The program verifies that #acontains_geo_tcbuffer, #ecovers_geo_tcbuffer
 * and #acovers_geo_tcbuffer report a null geometry and a geometry whose SRID
 * differs from the one of the temporal circular buffer by returning -1 and
 * setting #meos_errno, as #econtains_tcbuffer_geo does, and that a valid call
 * still answers with no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_validity_test cbuffer_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>

/* The relationships of a geometry and a temporal circular buffer tested */
typedef int (*geo_tcbuffer_fn)(const GSERIALIZED *, const Temporal *);

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp = tcbuffer_in("[Cbuffer(Point(1 1),0.5)@2001-01-01, "
    "Cbuffer(Point(2 2),0.5)@2001-01-02]");
  GSERIALIZED *geo = geom_in("Polygon((0 0,0 5,5 5,5 0,0 0))", -1);
  GSERIALIZED *other = geom_in("SRID=3857;Polygon((0 0,0 5,5 5,5 0,0 0))",
    -1);
  meos_errno_reset();

  static const char *names[] = {"acontains_geo_tcbuffer",
    "ecovers_geo_tcbuffer", "acovers_geo_tcbuffer"};
  geo_tcbuffer_fn fns[] = {&acontains_geo_tcbuffer, &ecovers_geo_tcbuffer,
    &acovers_geo_tcbuffer};
  for (int i = 0; i < 3; i++)
  {
    /* A null geometry is reported rather than read */
    int result = fns[i](NULL, temp);
    printf("%s(NULL, temp): %d, errno %d\n", names[i], result, meos_errno());
    assert(result == -1);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG);
    meos_errno_reset();

    /* Operands in two spatial reference systems are reported rather than
     * answered */
    result = fns[i](other, temp);
    printf("%s(SRID 3857, SRID 0): %d, errno %d\n", names[i], result,
      meos_errno());
    assert(result == -1);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();

    /* A valid call still answers, and the guard leaves no error behind */
    result = fns[i](geo, temp);
    printf("%s(geo, temp): %d, errno %d\n", names[i], result, meos_errno());
    assert(result == 0 || result == 1);
    assert(meos_errno() == 0);
  }

  /* The sibling taking its operands in the other order already reports a
   * null geometry, and stays as it is */
  int result = econtains_tcbuffer_geo(temp, NULL);
  printf("econtains_tcbuffer_geo(temp, NULL): %d, errno %d\n", result,
    meos_errno());
  assert(result == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);

  free(temp); free(geo); free(other);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
