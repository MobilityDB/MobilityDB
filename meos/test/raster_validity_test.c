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
 * @brief A program that tests how the functions reading a raster along a
 * trajectory report an erroneous trajectory under the noexit error handler.
 *
 * A raster, a raster file and a Raquet tile each carry a planar grid, and the
 * functions reading them along a trajectory, like the QUADBIN cover of a
 * trajectory, take a temporal geometry point, as their SQL signatures do. A
 * binding calling one of them with a null trajectory or with a temporal
 * geography point receives an error it can raise in its host language.
 *
 * The program verifies that each of these functions reports a null trajectory
 * with MEOS_ERR_INVALID_ARG and a temporal geography point with
 * MEOS_ERR_INVALID_ARG_TYPE, and that a temporal geometry point is read with
 * no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o raster_validity_test raster_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_raster.h>

/* A 3 x 3 raster with one band in SRID 4326, covering [0, 3] x [0, 3] */
static const char *raster_one_band =
  "0100000100000000000000f03f000000000000f0bf000000000000000000000000000008"
  "4000000000000000000000000000000000e6100000030003000a00000000000000000000"
  "000000000000000000000000000000000000000000000000000000000000";

/**
 * @brief Assert that a call refused its trajectory with the error expected
 */
static void
refused(const char *call, bool noanswer, int errnum)
{
  printf("%s: %s, errno %d\n", call, noanswer ? "refused" : "answered",
    meos_errno());
  assert(noanswer);
  assert(meos_errno() == errnum);
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* A planar trajectory over the raster, the geodetic trajectory between the
   * same positions, and the raster, file, tile and value range they are read
   * against */
  Temporal *geom = tgeompoint_in(
    "SRID=4326;[Point(0.5 2.5)@2020-01-01, Point(2.5 0.5)@2020-01-02]");
  Temporal *geog = tgeogpoint_in(
    "[Point(0.5 2.5)@2020-01-01, Point(2.5 0.5)@2020-01-02]");
  Raster *rast = raster_from_hexwkb(raster_one_band);
  Span *vspan = floatspan_in("[0, 10]");
  const char *path = "no_such_raster.tif";
  int ntiles = 0;
  uint64 *tiles = trajectory_quadbins(geom, 8, &ntiles);
  assert(geom && geog && rast && vspan && tiles && ntiles > 0);
  uint8_t pixels[1] = { 7 };
  Raquet *rq = raquet_make(tiles[0], 1, 1, MEOS_PT_UINT8, 0.0, false, pixels,
    1);
  assert(rq);
  const Raquet *rqarr[1] = { rq };
  meos_errno_reset();

  /* A null trajectory and a temporal geography point are reported rather than
   * read */
  const Temporal *bad[2] = { NULL, geog };
  const int errnum[2] = { MEOS_ERR_INVALID_ARG, MEOS_ERR_INVALID_ARG_TYPE };
  const char *what[2] = { "NULL", "tgeogpoint" };
  for (int i = 0; i < 2; i++)
  {
    char call[128];
    int count = -1;
#define CALL(name) (snprintf(call, sizeof(call), "%s(%s)", name, what[i]), call)
    refused(CALL("raster_value"),
      raster_value(bad[i], rast, 1, true, "nearest") == NULL, errnum[i]);
    refused(CALL("raster_at_value"),
      raster_at_value(bad[i], rast, 1, vspan) == NULL, errnum[i]);
    refused(CALL("raster_minus_value"),
      raster_minus_value(bad[i], rast, 1, vspan) == NULL, errnum[i]);
    refused(CALL("eraster_value"),
      eraster_value(bad[i], rast, 1, vspan) == -1, errnum[i]);
    refused(CALL("araster_value"),
      araster_value(bad[i], rast, 1, vspan) == -1, errnum[i]);
    refused(CALL("raster_value_gdal"),
      raster_value_gdal(bad[i], path, 1) == NULL, errnum[i]);
    refused(CALL("raster_at_value_gdal"),
      raster_at_value_gdal(bad[i], path, 1, vspan) == NULL, errnum[i]);
    refused(CALL("raster_minus_value_gdal"),
      raster_minus_value_gdal(bad[i], path, 1, vspan) == NULL, errnum[i]);
    refused(CALL("eraster_value_gdal"),
      eraster_value_gdal(bad[i], path, 1, vspan) == -1, errnum[i]);
    refused(CALL("araster_value_gdal"),
      araster_value_gdal(bad[i], path, 1, vspan) == -1, errnum[i]);
    refused(CALL("raster_tile_value_quadbin"),
      raster_tile_value_quadbin(bad[i], pixels, 1, 1, 1, tiles[0],
        MEOS_PT_UINT8, 0.0, false) == NULL, errnum[i]);
    refused(CALL("raster_tile_value"),
      raster_tile_value(bad[i], rq) == NULL, errnum[i]);
    refused(CALL("raster_tile_value_array"),
      raster_tile_value_array(bad[i], rqarr, 1) == NULL, errnum[i]);
    refused(CALL("trajectory_quadbins"),
      trajectory_quadbins(bad[i], 8, &count) == NULL, errnum[i]);
#undef CALL
  }

  /* A temporal geometry point is read with no error left behind */
  Temporal *res = raster_value(geom, rast, 1, false, "nearest");
  printf("raster_value(tgeompoint): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(meos_errno() == 0);
  free(res);
  res = raster_tile_value(geom, rq);
  printf("raster_tile_value(tgeompoint): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res != NULL && meos_errno() == 0);
  free(res);
  printf("trajectory_quadbins(tgeompoint): %d tiles, errno %d\n", ntiles,
    meos_errno());
  assert(meos_errno() == 0);

  free(tiles); free(rq); free(vspan); free(rast); free(geog); free(geom);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
