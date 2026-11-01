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
 * @brief A program that tests how the scalings of the MEOS API by the factors
 * a point states report an erroneous argument under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it can
 * raise in its host language. An assertion cannot carry that contract: it is
 * compiled out under NDEBUG, which leaves the release build a binding links
 * against with no check at all.
 *
 * The program verifies that #tgeo_scale_geo, #tgeo_scale_geo_origin,
 * #geo_scale_geo and #geo_scale_geo_origin report a null factor point or
 * origin by returning NULL and setting #meos_errno to 10, and an empty point
 * and an origin of another SRID by setting it to 12, as #tgeo_rotate_geo
 * reports its center. It also verifies that each answers the scaled value
 * with no error left behind, a two-dimensional factor point answering as
 * #tgeo_scale with a factor 1 along z.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o scale_origin_validity_test scale_origin_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/**
 * @brief Check that a call answered NULL with the expected error, then clear
 * it
 */
static void
expect_error(void *res, int code, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == code);
  meos_errno_reset();
}

/**
 * @brief Check that a call answered the expected text and left no error
 * behind
 */
static void
expect_text(char *res, const char *expected, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? res : "NULL", meos_errno());
  assert(res != NULL);
  assert(strcmp(res, expected) == 0);
  assert(meos_errno() == 0);
  free(res);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *temp = tgeompoint_in(
    "SRID=3812;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]");
  GSERIALIZED *gs = geom_in("SRID=3812;Point(2 2)", -1);
  GSERIALIZED *factors = geom_in("Point(2 3)", -1);
  GSERIALIZED *origin = geom_in("SRID=3812;Point(1 1)", -1);
  assert(temp && gs && factors && origin);
  meos_errno_reset();

  /* A null factor point or origin is reported rather than read as none */
  int arg = MEOS_ERR_INVALID_ARG, value = MEOS_ERR_INVALID_ARG_VALUE;
  expect_error(tgeo_scale_geo(temp, NULL), arg, "tgeo_scale_geo(NULL)");
  expect_error(tgeo_scale_geo_origin(temp, factors, NULL), arg,
    "tgeo_scale_geo_origin(factors, NULL)");
  expect_error(tgeo_scale_geo_origin(temp, NULL, origin), arg,
    "tgeo_scale_geo_origin(NULL, origin)");
  expect_error(geo_scale_geo(gs, NULL), arg, "geo_scale_geo(NULL)");
  expect_error(geo_scale_geo_origin(gs, factors, NULL), arg,
    "geo_scale_geo_origin(factors, NULL)");
  expect_error(geo_scale_geo_origin(gs, NULL, origin), arg,
    "geo_scale_geo_origin(NULL, origin)");

  /* An empty point and an origin of another SRID are reported, as the
   * center of a rotation is */
  GSERIALIZED *empty = geom_in("Point EMPTY", -1);
  GSERIALIZED *other = geom_in("SRID=5676;Point(1 1)", -1);
  assert(empty && other);
  expect_error(tgeo_scale_geo(temp, empty), value, "tgeo_scale_geo(EMPTY)");
  expect_error(tgeo_scale_geo_origin(temp, factors, empty), value,
    "tgeo_scale_geo_origin(factors, EMPTY)");
  expect_error(tgeo_scale_geo_origin(temp, factors, other), value,
    "tgeo_scale_geo_origin(factors, SRID=5676)");
  expect_error(geo_scale_geo(gs, empty), value, "geo_scale_geo(EMPTY)");
  expect_error(geo_scale_geo_origin(gs, factors, empty), value,
    "geo_scale_geo_origin(factors, EMPTY)");
  expect_error(geo_scale_geo_origin(gs, factors, other), value,
    "geo_scale_geo_origin(factors, SRID=5676)");
  free(empty); free(other);

  /* The factor point answers the scaled value, about the origin when there
   * is one */
  Temporal *scaled = tgeo_scale(temp, 2, 3, 1);
  char *expected = tspatial_as_text(scaled, 6);
  Temporal *res = tgeo_scale_geo(temp, factors);
  expect_text(tspatial_as_text(res, 6), expected, "tgeo_scale_geo(2 3)");
  free(res); free(scaled); free(expected);
  Temporal *about = tgeompoint_in(
    "SRID=3812;[Point(1 1)@2001-01-01, Point(3 4)@2001-01-02]");
  expected = tspatial_as_text(about, 6);
  res = tgeo_scale_geo_origin(temp, factors, origin);
  expect_text(tspatial_as_text(res, 6), expected,
    "tgeo_scale_geo_origin(2 3, 1 1)");
  free(res); free(about); free(expected);
  GSERIALIZED *gres = geo_scale_geo(gs, factors);
  expect_text(geo_as_text(gres, 6), "POINT(4 6)", "geo_scale_geo(2 3)");
  free(gres);
  gres = geo_scale_geo_origin(gs, factors, origin);
  expect_text(geo_as_text(gres, 6), "POINT(3 4)",
    "geo_scale_geo_origin(2 3, 1 1)");
  free(gres);

  free(temp); free(gs); free(factors); free(origin);
  meos_finalize();
  return EXIT_SUCCESS;
}
