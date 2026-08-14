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
 * @brief A program that tests the input and output functions of the PostGIS
 * raster type, and the band count read through them.
 *
 * A @p Raster is the serialized form of the PostGIS @p raster type. A
 * PostgreSQL session reaches it by detoasting its own column, but every other
 * binding holds the interchange representation instead, so
 * #raster_num_bands() is reachable outside PostgreSQL only through
 * #raster_from_wkb() / #raster_from_hexwkb(). This program exercises that path
 * with no PostgreSQL involved, which is what makes the band count inheritable
 * by the language bindings rather than a PostgreSQL-only operator.
 *
 * The two rasters below are the ASCII hex-encoded WKB of the two rasters the
 * `numBands` case of mobilitydb/test/raster/queries/500_raster.test.sql builds
 * with `ST_AddBand(ST_MakeEmptyRaster(3, 3, 0.0, 3.0, 1.0, -1.0, 0.0, 0.0,
 * 4326), '32BF', 0.0, NULL)`, so that the two suites read the same values.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o raster_test raster_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_raster.h>

/* A 3x3 raster with a single 32BF band */
static const char *raster_one_band =
  "0100000100000000000000f03f000000000000f0bf000000000000000000000000000008"
  "4000000000000000000000000000000000e6100000030003000a00000000000000000000"
  "000000000000000000000000000000000000000000000000000000000000";

/* The same raster with a second 32BF band added */
static const char *raster_two_bands =
  "0100000200000000000000f03f000000000000f0bf000000000000000000000000000008"
  "4000000000000000000000000000000000e6100000030003000a00000000000000000000"
  "000000000000000000000000000000000000000000000000000000000000"
  "0a0000000000000000000000000000000000000000000000000000000000000000000000"
  "0000000000";

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The band count is read from the interchange representation alone, which is
   * all a binding holds */
  const char *hexwkbs[2] = {raster_one_band, raster_two_bands};
  for (int i = 0; i < 2; i++)
  {
    meos_errno_reset();
    Raster *rast = raster_from_hexwkb(hexwkbs[i]);
    assert(rast != NULL);
    assert(meos_errno() == 0);
    int nbands = raster_num_bands(rast);
    printf("raster_num_bands(raster_from_hexwkb(#%d)): %d\n", i + 1, nbands);
    assert(nbands == i + 1);
    assert(meos_errno() == 0);

    /* The HexWKB output is read back into a raster with the same bands */
    size_t hexwkb_size;
    char *hexwkb = raster_as_hexwkb(rast, &hexwkb_size);
    assert(hexwkb != NULL);
    assert(hexwkb_size == strlen(hexwkb));
    Raster *rast1 = raster_from_hexwkb(hexwkb);
    assert(rast1 != NULL);
    assert(raster_num_bands(rast1) == nbands);

    /* The binary output round trips likewise, and the two representations
     * encode the same bytes */
    size_t wkb_size;
    uint8_t *wkb = raster_as_wkb(rast, &wkb_size);
    assert(wkb != NULL);
    assert(wkb_size * 2 == hexwkb_size);
    Raster *rast2 = raster_from_wkb(wkb, wkb_size);
    assert(rast2 != NULL);
    assert(raster_num_bands(rast2) == nbands);
    assert(meos_errno() == 0);

    free(hexwkb); free(wkb); free(rast); free(rast1); free(rast2);
  }

  /* A null argument is rejected rather than dereferenced */
  size_t size;
  meos_errno_reset();
  assert(raster_from_wkb(NULL, 0) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_from_hexwkb(NULL) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_num_bands(NULL) == -1);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_as_wkb(NULL, &size) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_as_hexwkb(NULL, &size) == NULL);
  assert(meos_errno() != 0);

  /* A raster is read against the length it is given, so every truncation of a
   * valid representation is a parse failure and not a read past the end of the
   * buffer. The size the header inside the bytes claims is never trusted */
  size_t wkb_size;
  Raster *rast = raster_from_hexwkb(raster_one_band);
  assert(rast != NULL);
  uint8_t *wkb = raster_as_wkb(rast, &wkb_size);
  assert(wkb != NULL);
  for (size_t trunc = 0; trunc < wkb_size; trunc++)
  {
    meos_errno_reset();
    Raster *bad = raster_from_wkb(wkb, trunc);
    assert(bad == NULL);
    assert(meos_errno() != 0);
  }
  printf("raster_from_wkb(truncated to 0..%zu bytes): NULL\n", wkb_size - 1);
  free(wkb); free(rast);

  /* A string that is not hex-encoded WKB is a parse failure */
  meos_errno_reset();
  Raster *bad_hex = raster_from_hexwkb("this is not hex-encoded WKB");
  printf("raster_from_hexwkb(bad HexWKB): %s, errno %d\n",
    bad_hex ? "non-NULL" : "NULL", meos_errno());
  assert(bad_hex == NULL);
  assert(meos_errno() != 0);

  meos_finalize();
  printf("raster_test: all assertions passed\n");
  return 0;
}
