/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief A simple program that shows the connection of MEOS and PROJ
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_proj meos_proj.c -L/usr/lib -lproj -L/usr/local/lib -lmeos
 * @endcode
 */

#include <proj.h>
#include <stdio.h>
#include <meos.h>

int main(void)
{
  PJ_CONTEXT *C;
  PJ *P;
  PJ *norm;
  PJ_COORD a, b;

  /* or you may set C=PJ_DEFAULT_CTX if you are sure you will     */
  /* use PJ objects from only one thread                          */
  C = proj_context_create();

  P = proj_create_crs_to_crs(
      C, "EPSG:4326", "+proj=utm +zone=32 +datum=WGS84", /* or EPSG:32632 */
      NULL);

  if (0 == P) {
      fprintf(stderr, "Failed to create transformation object.\n");
      return 1;
  }

  /* This will ensure that the order of coordinates for the input CRS */
  /* will be longitude, latitude, whereas EPSG:4326 mandates latitude, */
  /* longitude */
  norm = proj_normalize_for_visualization(C, P);
  if (0 == norm) {
      fprintf(stderr, "Failed to normalize transformation object.\n");
      return 1;
  }
  proj_destroy(P);
  P = norm;

  /* a coordinate union representing Copenhagen: 55d N, 12d E */
  /* Given that we have used proj_normalize_for_visualization(), the order */
  /* of coordinates is longitude, latitude, and values are expressed in */
  /* degrees. */
  a = proj_coord(12, 55, 0, 0);

  /* transform to UTM zone 32, then back to geographical */
  b = proj_trans(P, PJ_FWD, a);
  printf("easting: %.3f, northing: %.3f\n", b.enu.e, b.enu.n);

  b = proj_trans(P, PJ_INV, b);
  printf("longitude: %g, latitude: %g\n", b.lp.lam, b.lp.phi);

  /* Clean up */
  proj_destroy(P);
  proj_context_destroy(C); /* may be omitted in the single threaded case */
  return 0;
}
