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
 * @brief A program that tests that a pixel holding NaN reads as no value
 * @details No MEOS value holds a NaN. A raster marks a pixel without data by
 * its nodata value, which GDAL writes as NaN for a float band, so a pixel
 * holding NaN carries no value: a trajectory read over it answers no value
 * there, as it does over a pixel equal to the nodata value, rather than a
 * temporal float holding a NaN, which every MEOS constructor refuses.
 *
 * The program reads a trajectory over a one-pixel Raquet tile of 64-bit
 * floats, as #raster_tile_value reads it in raster_validity_test.c: a pixel
 * of 7.5 answers a value, and a NaN pixel, whether the band declares NaN as
 * its nodata value or declares none, answers no value and raises no error.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o raster_nan_pixel_test raster_nan_pixel_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_quadbin.h>
#include <meos_raster.h>

/* Write a double as the little-endian bytes raquet_make reads */
static void
pixel_set(uint8_t *pixels, double d)
{
  uint64_t bits;
  memcpy(&bits, &d, sizeof(bits));
  for (int i = 0; i < 8; i++)
    pixels[i] = (uint8_t) (bits >> (8 * i));
}

/* Read the trajectory over a one-pixel tile and report what it answers */
static Temporal *
read_pixel(const Temporal *traj, Quadbin tile, double pixel, double nodata,
  bool has_nodata, const char *what)
{
  uint8_t pixels[8];
  pixel_set(pixels, pixel);
  Raquet *rq = raquet_make(tile, 1, 1, MEOS_PT_FLOAT64, nodata, has_nodata,
    pixels, sizeof(pixels));
  assert(rq);
  meos_errno_reset();
  Temporal *res = raster_tile_value(traj, rq);
  printf("%s: %s, errno %d\n", what, res ? "a value" : "no value",
    meos_errno());
  free(rq);
  return res;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The trajectory and the tile it starts in, as raster_validity_test.c
   * reads them */
  Temporal *traj = tgeompoint_in(
    "SRID=4326;[Point(0.5 2.5)@2020-01-01, Point(2.5 0.5)@2020-01-02]");
  int ntiles = 0;
  Temporal *ttiles = tgeompoint_to_tquadbin(traj, 8, true);
  Quadbin *tiles = tquadbin_values(ttiles, &ntiles);
  assert(traj && tiles && ntiles > 0);

  /* A pixel holding a number answers a value */
  Temporal *res = read_pixel(traj, tiles[0], 7.5, 0.0, false,
    "pixel 7.5, no nodata");
  assert(res != NULL && meos_errno() == 0);
  free(res);

  /* A NaN pixel of a band whose nodata value is NaN answers no value */
  res = read_pixel(traj, tiles[0], NAN, NAN, true, "pixel NaN, nodata NaN");
  assert(res == NULL && meos_errno() == 0);

  /* A NaN pixel of a band declaring no nodata value answers no value */
  res = read_pixel(traj, tiles[0], NAN, 0.0, false, "pixel NaN, no nodata");
  assert(res == NULL && meos_errno() == 0);

  free(tiles); free(ttiles); free(traj);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
