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
 * @brief A program that tests how the functions reading a raster along a
 * trajectory report an erroneous trajectory under the noexit error handler
 * @details A raster, a raster file and a Raquet tile each carry a planar grid,
 * and the
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
  Temporal *ttiles = tgeompoint_to_tquadbin(geom, 8, true);
  Quadbin *tiles = tquadbin_values(ttiles, &ntiles);
  assert(geom && geog && rast && vspan && tiles && ntiles > 0);
  uint8_t pixels[1] = { 7 };
  Raquet *rq = raquet_make(tiles[0], 1, 1, MEOS_PT_UINT8, 0.0, false, pixels,
    1);
  assert(rq);
  const Raquet *rqarr[1] = { rq };
  /* A series of one raster and of one tile, valid over the whole trip */
  const Raster *rasts[1] = { rast };
  Span *period = tstzspan_in("[2020-01-01, 2020-01-02]");
  assert(period);
  Span spans[1] = { *period };
  meos_errno_reset();

  /* A null trajectory and a temporal geography point are reported rather than
   * read */
  const Temporal *bad[2] = { NULL, geog };
  const int errnum[2] = { MEOS_ERR_INVALID_ARG, MEOS_ERR_INVALID_ARG_TYPE };
  const char *what[2] = { "NULL", "tgeogpoint" };
  for (int i = 0; i < 2; i++)
  {
    char call[128];
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
    refused(CALL("raster_tile_at_value"),
      raster_tile_at_value(bad[i], rq, vspan) == NULL, errnum[i]);
    refused(CALL("raster_tile_at_value_array"),
      raster_tile_at_value_array(bad[i], rqarr, 1, vspan) == NULL, errnum[i]);
    refused(CALL("raster_tile_minus_value"),
      raster_tile_minus_value(bad[i], rq, vspan) == NULL, errnum[i]);
    refused(CALL("raster_tile_minus_value_array"),
      raster_tile_minus_value_array(bad[i], rqarr, 1, vspan) == NULL, errnum[i]);
    refused(CALL("eraster_tile_value"),
      eraster_tile_value(bad[i], rq, vspan) == -1, errnum[i]);
    refused(CALL("eraster_tile_value_array"),
      eraster_tile_value_array(bad[i], rqarr, 1, vspan) == -1, errnum[i]);
    refused(CALL("araster_tile_value"),
      araster_tile_value(bad[i], rq, vspan) == -1, errnum[i]);
    refused(CALL("araster_tile_value_array"),
      araster_tile_value_array(bad[i], rqarr, 1, vspan) == -1, errnum[i]);
    refused(CALL("raster_value_series"),
      raster_value_series(bad[i], rasts, spans, 1, 1) == NULL, errnum[i]);
    refused(CALL("raster_at_value_series"),
      raster_at_value_series(bad[i], rasts, spans, 1, 1, vspan) == NULL,
      errnum[i]);
    refused(CALL("raster_minus_value_series"),
      raster_minus_value_series(bad[i], rasts, spans, 1, 1, vspan) == NULL,
      errnum[i]);
    refused(CALL("eraster_value_series"),
      eraster_value_series(bad[i], rasts, spans, 1, 1, vspan) == -1,
      errnum[i]);
    refused(CALL("araster_value_series"),
      araster_value_series(bad[i], rasts, spans, 1, 1, vspan) == -1,
      errnum[i]);
    refused(CALL("raster_tile_value_series"),
      raster_tile_value_series(bad[i], rqarr, spans, 1) == NULL, errnum[i]);
    refused(CALL("raster_tile_at_value_series"),
      raster_tile_at_value_series(bad[i], rqarr, spans, 1, vspan) == NULL,
      errnum[i]);
    refused(CALL("raster_tile_minus_value_series"),
      raster_tile_minus_value_series(bad[i], rqarr, spans, 1, vspan) == NULL,
      errnum[i]);
    refused(CALL("eraster_tile_value_series"),
      eraster_tile_value_series(bad[i], rqarr, spans, 1, vspan) == -1,
      errnum[i]);
    refused(CALL("araster_tile_value_series"),
      araster_tile_value_series(bad[i], rqarr, spans, 1, vspan) == -1,
      errnum[i]);
#undef CALL
  }

  /* Two distinct periods that overlap are refused, by the reading and by the
   * ever and always forms, which answer -1 rather than false; two rasters
   * sharing one period are the tiles of one raster in space, and are read */
  Span *early = tstzspan_in("[2020-01-01, 2020-01-01 18:00]");
  Span *late = tstzspan_in("[2020-01-01 06:00, 2020-01-02]");
  assert(early && late);
  Span overlap[2] = { *early, *late };
  Span shared[2] = { *period, *period };
  const Raster *rasts2[2] = { rast, rast };
  refused("raster_value_series(overlapping periods)",
    raster_value_series(geom, rasts2, overlap, 2, 1) == NULL,
    MEOS_ERR_INVALID_ARG_VALUE);
  refused("eraster_value_series(overlapping periods)",
    eraster_value_series(geom, rasts2, overlap, 2, 1, vspan) == -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  refused("araster_value_series(overlapping periods)",
    araster_value_series(geom, rasts2, overlap, 2, 1, vspan) == -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  Temporal *shared_res = raster_value_series(geom, rasts2, shared, 2, 1);
  printf("raster_value_series(one period shared): %s, errno %d\n",
    shared_res ? "a value" : "NULL", meos_errno());
  assert(shared_res != NULL && meos_errno() == 0);
  free(shared_res); free(late); free(early);

  /* Two rasters of one footprint valid over one period are one tile given
   * twice: read together where they read one value, refused where they read
   * two, the ever and always forms answering -1 */
  uint8_t px7[1] = { 7 }, px9[1] = { 9 };
  Raster *r7 = raster_make(1, 1, 0.0, 3.0, 3.0, -3.0, 0.0, 0.0, 4326, "8BUI",
    0.0, false, px7, 1);
  Raster *r7b = raster_make(1, 1, 0.0, 3.0, 3.0, -3.0, 0.0, 0.0, 4326, "8BUI",
    0.0, false, px7, 1);
  Raster *r9 = raster_make(1, 1, 0.0, 3.0, 3.0, -3.0, 0.0, 0.0, 4326, "8BUI",
    0.0, false, px9, 1);
  assert(r7 && r7b && r9);
  const Raster *same[2] = { r7, r7b }, *differ[2] = { r7, r9 };
  Temporal *same_res = raster_value_series(geom, same, shared, 2, 1);
  printf("raster_value_series(one tile given twice, one value): %s, errno %d\n",
    same_res ? "a value" : "NULL", meos_errno());
  assert(same_res != NULL && meos_errno() == 0);
  free(same_res);
  refused("raster_value_series(one tile given twice, two values)",
    raster_value_series(geom, differ, shared, 2, 1) == NULL,
    MEOS_ERR_INVALID_ARG_VALUE);
  refused("eraster_value_series(one tile given twice, two values)",
    eraster_value_series(geom, differ, shared, 2, 1, vspan) == -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  refused("araster_value_series(one tile given twice, two values)",
    araster_value_series(geom, differ, shared, 2, 1, vspan) == -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  /* The same holds for two Raquet tiles of one cell */
  uint8_t px1[1] = { 1 };
  Raquet *rq1 = raquet_make(tiles[0], 1, 1, MEOS_PT_UINT8, 0.0, false, px1, 1);
  assert(rq1);
  const Raquet *rqdiffer[2] = { rq, rq1 }, *rqsame[2] = { rq, rq };
  Temporal *rqsame_res = raster_tile_value_series(geom, rqsame, shared, 2);
  printf("raster_tile_value_series(one tile given twice, one value): %s, "
    "errno %d\n", rqsame_res ? "a value" : "NULL", meos_errno());
  assert(rqsame_res != NULL && meos_errno() == 0);
  free(rqsame_res);
  refused("raster_tile_value_series(one tile given twice, two values)",
    raster_tile_value_series(geom, rqdiffer, shared, 2) == NULL,
    MEOS_ERR_INVALID_ARG_VALUE);
  refused("eraster_tile_value_series(one tile given twice, two values)",
    eraster_tile_value_series(geom, rqdiffer, shared, 2, vspan) == -1,
    MEOS_ERR_INVALID_ARG_VALUE);
  free(rq1); free(r9); free(r7b); free(r7);

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
  res = raster_value_series(geom, rasts, spans, 1, 1);
  printf("raster_value_series(tgeompoint): %s, errno %d\n",
    res ? "a value" : "NULL", meos_errno());
  assert(res != NULL && meos_errno() == 0);
  free(res);
  free(period);
  free(tiles); free(ttiles); free(rq); free(vspan); free(rast); free(geog); free(geom);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
