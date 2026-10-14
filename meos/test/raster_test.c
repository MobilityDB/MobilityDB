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
 * @brief A program that tests the input and output functions of the PostGIS
 * raster type, and the band count read through them
 * @details A @p Raster is the serialized form of the PostGIS @p raster type. A
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
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
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

/* The 3x3 raster of the rasterValue cases: a 32BF band of 1 degree pixels
 * holding 10, nodata, 30 / 40, 50, 60 / 70, 80, 90, with -9999 declared as
 * its nodata value */
static const char *raster_values =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000e6100000030003004a003c1cc600002041"
  "003c1cc60000f04100002042000048420000704200008c420000a0420000b442";

/* A 3x3 raster on the grid of raster_values with an unsigned 16-bit band
 * holding 3000, 500, 2000 / 4000, 1000, 6000 / 100, 0, 65535, the near
 * infrared band of the arithmetic cases, written by PostGIS */
static const char *raster_nir =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000e610000003000300060000b80bf401d007"
  "a00fe803701764000000ffff";

/* The red band of the arithmetic cases, unsigned 16-bit, holding 1000, 1500,
 * 2000 / 1000, 3000, 2000 / 300, 1, 0 */
static const char *raster_red =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000e610000003000300060000e803dc05d007"
  "e803b80bd0072c0101000000";

/* A 32BF band of ones on the grid of raster_values one pixel to the east */
static const char *raster_east1 =
  "0100000100000000000000f03f000000000000f0bf000000000000f03f000000000000"
  "084000000000000000000000000000000000e6100000030003000a000000000000803f"
  "0000803f0000803f0000803f0000803f0000803f0000803f0000803f0000803f";

/* The same band half a pixel to the east, on a grid of its own */
static const char *raster_easthalf =
  "0100000100000000000000f03f000000000000f0bf000000000000e03f000000000000"
  "084000000000000000000000000000000000e6100000030003000a000000000000803f"
  "0000803f0000803f0000803f0000803f0000803f0000803f0000803f0000803f";

/* The same band ten pixels to the east, sharing no pixel with raster_values */
static const char *raster_east10 =
  "0100000100000000000000f03f000000000000f0bf0000000000002440000000000000"
  "084000000000000000000000000000000000e6100000030003000a000000000000803f"
  "0000803f0000803f0000803f0000803f0000803f0000803f0000803f0000803f";

/* The same band on the grid of raster_values in SRID 3857 */
static const char *raster_srid3857 =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000110f0000030003000a000000000000803f"
  "0000803f0000803f0000803f0000803f0000803f0000803f0000803f0000803f";

/* A 4x4 raster of elevations over 1 unit pixels in SRID 3857, a 32BF band
 * holding 10, 20, 30, 40 / 15, 25, nodata, 45 / 20, 30, 40, 50 / 25, 35, 45,
 * 55, with -9999 as its nodata value, written by PostGIS */
static const char *raster_dem =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "104000000000000000000000000000000000110f0000040004004a003c1cc600002041"
  "0000a0410000f04100002042000070410000c841003c1cc6000034420000a0410000f0"
  "4100002042000048420000c84100000c420000344200005c42";

/* The 2x2 extent from (1, 3) on the grid of raster_dem, with no band */
static const char *raster_dem_square =
  "0100000000000000000000f03f000000000000f0bf000000000000f03f000000000000"
  "084000000000000000000000000000000000110f000002000200";

/* A 3x3 plane rising by 1 for each pixel to the east, a 32BF band holding
 * 0, 1, 2 on every row */
static const char *raster_plane =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000e6100000030003000a0000000000000000"
  "0000803f00000040000000000000803f00000040000000000000803f00000040";

/* A 3x3 raster with a 32BF band holding 1.24, 1.26, nodata / 2, 2, 2 / 3,
 * 1.24, 2, with -9999 as its nodata value, the value count cases, written by
 * PostGIS */
static const char *raster_counts =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "084000000000000000000000000000000000e6100000030003004a003c1cc652b89e3f"
  "ae47a13f003c1cc60000004000000040000000400000404052b89e3f00000040";

/* A 2x2 raster whose 32BF band states that every pixel is nodata, -9999 */
static const char *raster_all_nodata =
  "0100000100000000000000f03f000000000000f0bf0000000000000000000000000000"
  "004000000000000000000000000000000000e6100000020002006a003c1cc6003c1cc6"
  "003c1cc6003c1cc6003c1cc6";

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
    char *hexwkb = raster_as_hexwkb(rast, 0, &hexwkb_size);
    assert(hexwkb != NULL);
    assert(hexwkb_size == strlen(hexwkb));
    Raster *rast1 = raster_from_hexwkb(hexwkb);
    assert(rast1 != NULL);
    assert(raster_num_bands(rast1) == nbands);

    /* The binary output round trips likewise, and the two representations
     * encode the same bytes */
    size_t wkb_size;
    uint8_t *wkb = raster_as_wkb(rast, 0, &wkb_size);
    assert(wkb != NULL);
    assert(wkb_size * 2 == hexwkb_size);
    Raster *rast2 = raster_from_wkb(wkb, wkb_size);
    assert(rast2 != NULL);
    assert(raster_num_bands(rast2) == nbands);
    assert(meos_errno() == 0);

    /* Either byte order round trips, and the first byte states the one asked
     * for. The other order rewrites every field, so the two representations
     * differ beyond their first byte while reading back the same raster */
    size_t ndr_size, xdr_size, back_size;
    uint8_t *ndr = raster_as_wkb(rast, WKB_NDR, &ndr_size);
    uint8_t *xdr = raster_as_wkb(rast, WKB_XDR, &xdr_size);
    assert(ndr != NULL && xdr != NULL && ndr_size == xdr_size);
    assert(ndr[0] == 1 && xdr[0] == 0);
    assert(memcmp(ndr + 1, xdr + 1, ndr_size - 1) != 0);
    Raster *rast3 = raster_from_wkb(xdr, xdr_size);
    assert(rast3 != NULL && raster_num_bands(rast3) == nbands);
    uint8_t *back = raster_as_wkb(rast3, WKB_NDR, &back_size);
    assert(back != NULL && back_size == ndr_size);
    assert(memcmp(back, ndr, ndr_size) == 0);
    char *xdrhex = raster_as_hexwkb(rast, WKB_XDR, &hexwkb_size);
    assert(xdrhex != NULL && strncmp(xdrhex, "00", 2) == 0);
    assert(meos_errno() == 0);
    printf("raster_as_wkb(#%d): %zu bytes in either byte order\n", i + 1,
      ndr_size);
    free(ndr); free(xdr); free(back); free(xdrhex); free(rast3);

    free(hexwkb); free(wkb); free(rast); free(rast1); free(rast2);
  }

  /* The shape of a raster is read from the interchange representation too, so
   * a binding states the grid without PostGIS. The values are the ones the
   * header above carries: a 3x3 grid of 1 degree pixels whose rows run north
   * to south from the upper left corner (0, 3), unskewed, in EPSG:4326 */
  meos_errno_reset();
  Raster *shape = raster_from_hexwkb(raster_one_band);
  assert(shape != NULL);
  assert(raster_width(shape) == 3);
  assert(raster_height(shape) == 3);
  assert(raster_srid(shape) == 4326);
  assert(raster_upper_left_x(shape) == 0.0);
  assert(raster_upper_left_y(shape) == 3.0);
  assert(raster_scale_x(shape) == 1.0);
  assert(raster_scale_y(shape) == -1.0);
  assert(raster_skew_x(shape) == 0.0);
  assert(raster_skew_y(shape) == 0.0);
  assert(meos_errno() == 0);
  printf("raster shape: %dx%d at (%g, %g) scale (%g, %g) skew (%g, %g) "
    "srid %d\n", raster_width(shape), raster_height(shape),
    raster_upper_left_x(shape), raster_upper_left_y(shape),
    raster_scale_x(shape), raster_scale_y(shape), raster_skew_x(shape),
    raster_skew_y(shape), raster_srid(shape));

  /* The band reports the pixel type under the name the RaQuet specification
   * writes, which is the name a raquet tile of the same type reports, and it
   * states no nodata value */
  char *pixtype = raster_band_pixel_type(shape, 1);
  assert(pixtype != NULL);
  assert(strcmp(pixtype, "float32") == 0);
  assert(! raster_band_has_nodata_value(shape, 1));
  printf("raster_band_pixel_type(#1, 1): %s\n", pixtype);
  free(pixtype);

  /* The extent of an unskewed raster runs from its origin over its scaled
   * size, and carries the reference system the raster states */
  STBox *box = raster_to_stbox(shape);
  assert(box != NULL);
  double xmin, xmax;
  assert(stbox_xmin(box, &xmin) && stbox_xmax(box, &xmax));
  assert(xmin == 0.0 && xmax == 3.0);
  assert(stbox_srid(box) == 4326);
  char *box_str = stbox_out(box, 6);
  printf("raster_to_stbox(#1): %s\n", box_str);
  free(box_str); free(box);

  /* A band declaring a nodata value reports it, which the band above does not,
   * so the two answers cannot both come from a constant */
  Raster *withnodata = raster_from_hexwkb(raster_values);
  assert(withnodata != NULL);
  assert(raster_band_has_nodata_value(withnodata, 1));
  double nodata = 0.0;
  assert(raster_band_nodata_value(withnodata, 1, &nodata));
  assert(nodata == -9999.0);
  printf("raster_band_nodata_value(values, 1): %g\n", nodata);
  assert(meos_errno() == 0);

  /* A band number outside the bands the raster holds is an error, on either
   * side, and the band numbering starts at one */
  meos_errno_reset();
  assert(raster_band_pixel_type(shape, 0) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_band_pixel_type(shape, 2) == NULL);
  assert(meos_errno() != 0);
  /* A band stating no nodata value answers false with no error and leaves the
   * result where it stands, as a box without X answers its xmin */
  meos_errno_reset();
  nodata = 0.0;
  assert(! raster_band_nodata_value(shape, 1, &nodata));
  assert(nodata == 0.0);
  assert(meos_errno() == 0);
  meos_errno_reset();

  free(shape); free(withnodata);

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
  assert(raster_width(NULL) == INT_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_height(NULL) == INT_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  /* The sentinel here is SRID_INVALID, which the public headers do not name,
   * so the rejection is read from meos_errno() rather than from the value */
  (void) raster_srid(NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_upper_left_x(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_upper_left_y(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_scale_x(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_scale_y(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_skew_x(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_skew_y(NULL) == DBL_MAX);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_band_pixel_type(NULL, 1) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(! raster_band_has_nodata_value(NULL, 1));
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(! raster_band_nodata_value(NULL, 1, &nodata));
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_to_stbox(NULL) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_as_wkb(NULL, 0, &size) == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_as_hexwkb(NULL, 0, &size) == NULL);
  assert(meos_errno() != 0);

  /* A raster is read against the length it is given, so every truncation of a
   * valid representation is a parse failure and not a read past the end of the
   * buffer. The size the header inside the bytes claims is never trusted */
  size_t wkb_size;
  Raster *rast = raster_from_hexwkb(raster_one_band);
  assert(rast != NULL);
  uint8_t *wkb = raster_as_wkb(rast, 0, &wkb_size);
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

  /* The tile dimensions are taken in the type the SQL surface uses and the
   * range is rejected by the MEOS function, so a caller outside PostgreSQL
   * gets the same answer as a PostgreSQL one. Passing a value that does not
   * fit the tile's unsigned 16-bit fields would otherwise sample a tile of a
   * different size: 65538 a two pixel wide one, -1 a 65535 pixel wide one */
  Temporal *traj = tgeompoint_in("SRID=4326;{Point(45.0 10.0)@2024-01-01}");
  assert(traj != NULL);
  const uint8_t tile_pixels[4] = {1, 2, 3, 4};
  const int32 bad_dims[][2] = {{65538, 2}, {2, -1}, {0, 2}, {2, 65536}};
  for (size_t i = 0; i < sizeof(bad_dims) / sizeof(bad_dims[0]); i++)
  {
    meos_errno_reset();
    Temporal *tile = raster_tile_value_quadbin(traj, tile_pixels,
      sizeof(tile_pixels), bad_dims[i][0], bad_dims[i][1],
      5194902170171867135ULL, MEOS_PT_UINT8, 0.0, false);
    printf("raster_tile_value_quadbin(%d x %d): %s, errno %d\n",
      bad_dims[i][0], bad_dims[i][1], tile ? "non-NULL" : "NULL",
      meos_errno());
    assert(tile == NULL);
    assert(meos_errno() != 0);
  }

  /* A band is a little-endian byte stream whatever machine wrote it, so the
   * same bytes name the same value everywhere a binding runs. The tile below
   * is a single pixel, so the trajectory samples it wherever in the cell it
   * falls, and the bytes of each type are the ones the specification gives for
   * the value asserted beside them, which is what ties the decoding to the
   * byte order of the specification rather than to that of the machine */
  int zero_count;
  Temporal *zero_tquadbin = tgeompoint_to_tquadbin(traj, 0, true);
  Quadbin *zero_quadbin = tquadbin_values(zero_tquadbin, &zero_count);
  assert(zero_quadbin != NULL && zero_count >= 1);
  const uint8_t pixel_int8[1] = {0xff};
  const uint8_t pixel_uint16[2] = {0xff, 0xff};
  const uint8_t pixel_uint32[4] = {0xff, 0xff, 0xff, 0xff};
  /* 2^53, the largest integer the sampling surface carries exactly */
  const uint8_t pixel_int64[8] = {0, 0, 0, 0, 0, 0, 0x20, 0x00};
  const uint8_t pixel_uint64[8] = {0, 0, 0, 0, 0, 0, 0x20, 0x00};
  /* 1.0 and -2.0 as 16-bit halves */
  const uint8_t pixel_float16[2] = {0x00, 0x3c};
  const uint8_t pixel_int16[2] = {0x00, 0x80};
  const uint8_t pixel_int32[4] = {0x04, 0x03, 0x02, 0x01};
  const uint8_t pixel_float32[4] = {0x00, 0x00, 0xc0, 0x3f};
  const uint8_t pixel_float64[8] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f};
  const struct
  {
    const uint8_t *pixels;
    size_t size;
    MeosPixType pixtype;
    double value;
  } pixel_cases[] = {
    {pixel_int16,   sizeof(pixel_int16),   MEOS_PT_INT16,   -32768.0},
    {pixel_int32,   sizeof(pixel_int32),   MEOS_PT_INT32,   16909060.0},
    {pixel_float32, sizeof(pixel_float32), MEOS_PT_FLOAT32, 1.5},
    {pixel_float64, sizeof(pixel_float64), MEOS_PT_FLOAT64, 1.5},
    {pixel_int8,    sizeof(pixel_int8),    MEOS_PT_INT8,    -1.0},
    {pixel_uint16,  sizeof(pixel_uint16),  MEOS_PT_UINT16,  65535.0},
    {pixel_uint32,  sizeof(pixel_uint32),  MEOS_PT_UINT32,  4294967295.0},
    {pixel_int64,   sizeof(pixel_int64),   MEOS_PT_INT64,   9007199254740992.0},
    {pixel_uint64,  sizeof(pixel_uint64),  MEOS_PT_UINT64,  9007199254740992.0},
    {pixel_float16, sizeof(pixel_float16), MEOS_PT_FLOAT16, 1.0}
  };
  for (size_t i = 0; i < sizeof(pixel_cases) / sizeof(pixel_cases[0]); i++)
  {
    meos_errno_reset();
    Temporal *tile = raster_tile_value_quadbin(traj, pixel_cases[i].pixels,
      pixel_cases[i].size, 1, 1, zero_quadbin[0], pixel_cases[i].pixtype,
      0.0, false);
    assert(tile != NULL);
    assert(meos_errno() == 0);
    double value = tfloat_start_value(tile);
    printf("raster_tile_value_quadbin(pixel type %d): %f\n",
      (int) pixel_cases[i].pixtype, value);
    assert(value == pixel_cases[i].value);
    free(tile);
  }
  /* A 64-bit integer band holds values that no double names. Such a pixel is
   * reported rather than rounded to a neighbour, so that a sampled value is
   * never a number the band does not hold. The two below are 2^53 + 1, the
   * first integer a double skips, and 2^63 */
  const uint8_t over_int64[8] = {0x01, 0, 0, 0, 0, 0, 0x20, 0x00};
  const uint8_t over_uint64[8] = {0, 0, 0, 0, 0, 0, 0, 0x80};
  const struct
  {
    const uint8_t *pixels;
    MeosPixType pixtype;
    const char *label;
  } domain_cases[] = {
    {over_int64,  MEOS_PT_INT64,  "INT64 2^53+1"},
    {over_uint64, MEOS_PT_UINT64, "UINT64 2^63"}
  };
  for (size_t i = 0; i < sizeof(domain_cases) / sizeof(domain_cases[0]); i++)
  {
    meos_errno_reset();
    Temporal *tile = raster_tile_value_quadbin(traj, domain_cases[i].pixels, 8,
      1, 1, zero_quadbin[0], domain_cases[i].pixtype, 0.0, false);
    printf("raster_tile_value_quadbin(%s): %s, errno %d\n",
      domain_cases[i].label, tile ? "non-NULL" : "NULL", meos_errno());
    assert(tile == NULL);
    assert(meos_errno() != 0);
  }
  free(zero_quadbin); free(zero_tquadbin);
  free(traj);

  /* A trajectory that moves between its instants covers the tiles it crosses:
   * a tile spans 45 degrees of longitude at zoom 3, so a trip from 10E to
   * 170E crosses four of them, and the same path sampled every 20 degrees
   * answers the same four. A join filtered on a set that omits a crossed tile
   * never reads that tile */
  Temporal *traj_across = tgeompoint_in("SRID=4326;[Point(10.0 10.0)@2024-01-01,"
    " Point(170.0 10.0)@2024-01-02]");
  Temporal *traj_sampled = tgeompoint_in("SRID=4326;{Point(10.0 10.0)@2024-01-01,"
    " Point(30.0 10.0)@2024-01-02, Point(50.0 10.0)@2024-01-03,"
    " Point(70.0 10.0)@2024-01-04, Point(90.0 10.0)@2024-01-05,"
    " Point(110.0 10.0)@2024-01-06, Point(130.0 10.0)@2024-01-07,"
    " Point(150.0 10.0)@2024-01-08, Point(170.0 10.0)@2024-01-09}");
  assert(traj_across != NULL && traj_sampled != NULL);
  int ncrossed, nsampled;
  Temporal *tcrossed = tgeompoint_to_tquadbin(traj_across, 3, true);
  Temporal *tsampled = tgeompoint_to_tquadbin(traj_sampled, 3, true);
  Quadbin *crossed = tquadbin_values(tcrossed, &ncrossed);
  Quadbin *sampled = tquadbin_values(tsampled, &nsampled);
  printf("tquadbin_values(linear trip, 3): %d cell(s), sampled: %d\n",
    ncrossed, nsampled);
  assert(ncrossed == 4);
  assert(nsampled == 4);
  free(crossed); free(sampled); free(tcrossed); free(tsampled);
  free(traj_across); free(traj_sampled);

  /* The sampling of a PostGIS raster is answered by MEOS, so a program using
   * the library reads the values a PostgreSQL session reads. The trajectory
   * below visits pixel(1,1) = 10, the nodata pixel(1,2), a position outside
   * the raster, and pixel(3,1) = 70, so the two positions carrying data are
   * the two the sampling answers */
  Raster *rast_values = raster_from_hexwkb(raster_values);
  assert(rast_values != NULL);
  Temporal *traj_values = tgeompoint_in("SRID=4326;{POINT(0.5 2.5)@2001-01-01,"
    " POINT(1.5 2.5)@2001-01-02, POINT(5.5 5.5)@2001-01-03,"
    " POINT(0.5 0.5)@2001-01-04}");
  assert(traj_values != NULL);
  meos_errno_reset();
  Temporal *values = raster_value(traj_values, rast_values, 1, true, NULL);
  assert(values != NULL);
  assert(meos_errno() == 0);
  char *values_str = tfloat_out(values, 0);
  printf("raster_value(traj, raster, 1): %s\n", values_str);
  assert(temporal_num_instants(values) == 2);
  assert(tfloat_start_value(values) == 10.0);
  assert(tfloat_end_value(values) == 70.0);
  free(values_str); free(values);

  /* A trajectory that moves between its instants passes over the pixels
   * between them: the diagonal below crosses pixel(2,2) = 50, which the
   * answer holds until the trip reaches the pixel holding 90, and the walk
   * that finds it is the one #tpointseq_densify_to_th3index() walks a
   * hexagon with */
  Temporal *traj_linear = tgeompoint_in("SRID=4326;[POINT(0.5 2.5)@2001-01-01,"
    " POINT(2.5 0.5)@2001-01-03]");
  assert(traj_linear != NULL);
  meos_errno_reset();
  Temporal *along = raster_value(traj_linear, rast_values, 1, true, NULL);
  assert(along != NULL);
  assert(meos_errno() == 0);
  char *along_str = tfloat_out(along, 0);
  printf("raster_value(linear trip, raster, 1): %s\n", along_str);
  assert(strcmp(temporal_interp(along), "Step") == 0);
  assert(temporal_num_instants(along) == 4);
  assert(tfloat_start_value(along) == 10.0);
  assert(tfloat_end_value(along) == 90.0);
  /* The crossed pixel is in the answer, which sampling the instants alone
   * cannot state */
  int ndistinct;
  double *distinct = tfloat_values(along, &ndistinct);
  assert(distinct != NULL && ndistinct == 3);
  free(distinct); free(along_str); free(along); free(traj_linear);

  /* A trip crossing the nodata pixel answers one sequence per visit */
  Temporal *traj_gap = tgeompoint_in("SRID=4326;[POINT(0.5 2.5)@2001-01-01,"
    " POINT(2.5 2.5)@2001-01-03]");
  assert(traj_gap != NULL);
  Temporal *visits = raster_value(traj_gap, rast_values, 1, true, NULL);
  assert(visits != NULL);
  char *visits_str = tfloat_out(visits, 0);
  printf("raster_value(trip across nodata): %s\n", visits_str);
  assert(temporal_num_sequences(visits) == 2);
  free(visits_str); free(visits); free(traj_gap);

  /* The restrictions and the predicates read the same values: only the
   * position sampling 70 falls inside [40, 90] */
  Span *vspan = floatspan_in("[40, 90]");
  assert(vspan != NULL);
  Temporal *at = raster_at_value(traj_values, rast_values, 1, vspan);
  assert(at != NULL && temporal_num_instants(at) == 1);
  Temporal *minus = raster_minus_value(traj_values, rast_values, 1,
    vspan);
  assert(minus != NULL && temporal_num_instants(minus) == 1);
  int ever = eraster_value(traj_values, rast_values, 1, vspan);
  int always = araster_value(traj_values, rast_values, 1, vspan);
  printf("eraster_value: %d, araster_value: %d\n", ever,
    always);
  assert(ever == 1 && always == 0);
  assert(meos_errno() == 0);
  free(at); free(minus);

  /* A band the raster does not have is refused, in either direction */
  const int bad_bands[] = {0, -1, 2};
  for (size_t i = 0; i < sizeof(bad_bands) / sizeof(bad_bands[0]); i++)
  {
    meos_errno_reset();
    Temporal *none = raster_value(traj_values, rast_values,
      bad_bands[i], true, NULL);
    printf("raster_value(band %d): %s, errno %d\n", bad_bands[i],
      none ? "non-NULL" : "NULL", meos_errno());
    assert(none == NULL);
    assert(meos_errno() != 0);
  }

  /* A raster and a trajectory in different reference systems state their
   * positions in different units, which is an error and not an empty answer */
  Temporal *traj_3857 = tgeompoint_in("SRID=3857;{POINT(0.5 2.5)@2001-01-01}");
  assert(traj_3857 != NULL);
  meos_errno_reset();
  assert(raster_value(traj_3857, rast_values, 1, true, NULL) == NULL);
  assert(meos_errno() != 0);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_value(NULL, rast_values, 1, true, NULL) == NULL);
  assert(raster_value(traj_values, NULL, 1, true, NULL) == NULL);
  assert(raster_at_value(traj_values, rast_values, 1, NULL) == NULL);
  assert(raster_minus_value(traj_values, rast_values, 1, NULL) == NULL);
  assert(eraster_value(traj_values, rast_values, 1, NULL) == -1);
  assert(araster_value(traj_values, rast_values, 1, NULL) == -1);
  assert(meos_errno() != 0);

  /* A position over a nodata pixel answers the nodata value when the caller
   * keeps it rather than leaving it out */
  Temporal *traj_nodata = tgeompoint_in(
    "SRID=4326;{POINT(1.5 2.5)@2001-01-01}");
  assert(traj_nodata != NULL);
  meos_errno_reset();
  Temporal *kept_nodata = raster_value(traj_nodata, rast_values, 1, false,
    NULL);
  assert(kept_nodata != NULL);
  assert(meos_errno() == 0);
  assert(tfloat_start_value(kept_nodata) == -9999.0);
  free(kept_nodata); free(traj_nodata);

  /* A bilinear read interpolates the four pixels around a position, a nodata
   * one taking the value of the pixel the position falls in: POINT(0.75 2.25)
   * weighs 10, the nodata pixel read as 10, 40 and 50 by 9, 3, 3 and 1
   * sixteenths, which is 18.125 */
  Temporal *traj_bilinear = tgeompoint_in(
    "SRID=4326;{POINT(0.75 2.25)@2001-01-01}");
  assert(traj_bilinear != NULL);
  meos_errno_reset();
  Temporal *bilin = raster_value(traj_bilinear, rast_values, 1, true,
    "Bilinear");
  assert(bilin != NULL);
  assert(meos_errno() == 0);
  printf("raster_value(raster, 1, bilinear) at POINT(0.75 2.25): %f\n",
    tfloat_start_value(bilin));
  assert(tfloat_start_value(bilin) == 18.125);
  free(bilin); free(traj_bilinear);

  /* Along a moving trajectory a bilinear read answers a linear temporal
   * float starting with the value read at its first position, and a read the
   * raster does not know is refused rather than taken for another */
  Temporal *traj_moving = tgeompoint_in("SRID=4326;[POINT(0.75 2.25)@2001-01-01,"
    " POINT(0.75 1.75)@2001-01-03]");
  assert(traj_moving != NULL);
  meos_errno_reset();
  Temporal *bilin_moving = raster_value(traj_moving, rast_values, 1, true,
    "bilinear");
  assert(bilin_moving != NULL);
  assert(meos_errno() == 0);
  printf("raster_value(raster, 1, bilinear) along a trajectory: %f\n",
    tfloat_start_value(bilin_moving));
  assert(strcmp(temporal_interp(bilin_moving), "Linear") == 0);
  assert(tfloat_start_value(bilin_moving) == 18.125);
  free(bilin_moving);
  meos_errno_reset();
  assert(raster_value(traj_values, rast_values, 1, true, "cubic") == NULL);
  assert(meos_errno() != 0);
  free(traj_moving);

  /* Clipping keeps the pixels a geometry covers and no others. The region
   * below spans x in [0,2] and y in [1,3], so it covers the two left columns
   * of the two upper rows: pixel(1,1) = 10 and pixel(2,2) = 50 stand inside
   * it, while pixel(3,1) = 70 and the right column stand outside */
  GSERIALIZED *region = geom_in("SRID=4326;POLYGON((0 1,2 1,2 3,0 3,0 1))",
    -1);
  assert(region != NULL);

  /* Cropping reduces the result to the extent the two share, which is two
   * pixels by two on the grid the subject states */
  meos_errno_reset();
  Raster *cropped = raster_clip(rast_values, NULL, 0, region, NULL, 0, true,
    false);
  assert(cropped != NULL);
  assert(meos_errno() == 0);
  assert(raster_width(cropped) == 2);
  assert(raster_height(cropped) == 2);
  assert(raster_srid(cropped) == 4326);
  assert(raster_upper_left_x(cropped) == 0.0);
  assert(raster_upper_left_y(cropped) == 3.0);
  assert(raster_num_bands(cropped) == 1);

  /* The pixels the region covers answer what they answered before the clip */
  Temporal *traj_kept = tgeompoint_in("SRID=4326;{POINT(0.5 2.5)@2001-01-01,"
    " POINT(1.5 1.5)@2001-01-02}");
  assert(traj_kept != NULL);
  meos_errno_reset();
  Temporal *kept = raster_value(traj_kept, cropped, 1, true, NULL);
  assert(kept != NULL);
  assert(meos_errno() == 0);
  assert(temporal_num_instants(kept) == 2);
  assert(tfloat_start_value(kept) == 10.0);
  assert(tfloat_end_value(kept) == 50.0);
  free(kept);

  /* Without cropping the result keeps the extent of the subject, so the clip
   * shows in the pixels rather than in the grid. This is the case a mask that
   * covered everything would pass and a mask that covered nothing would fail:
   * pixel(1,1) = 10 stands inside the region and answers, while
   * pixel(3,1) = 70 stands outside it and answers nothing */
  meos_errno_reset();
  Raster *whole = raster_clip(rast_values, NULL, 0, region, NULL, 0, false,
    false);
  assert(whole != NULL);
  assert(meos_errno() == 0);
  assert(raster_width(whole) == 3);
  assert(raster_height(whole) == 3);
  assert(raster_upper_left_x(whole) == 0.0);
  assert(raster_upper_left_y(whole) == 3.0);

  meos_errno_reset();
  Temporal *masked = raster_value(traj_values, whole, 1, true, NULL);
  assert(masked != NULL);
  assert(meos_errno() == 0);
  char *masked_str = tfloat_out(masked, 0);
  printf("raster_clip(raster, region, false) sampled: %s\n", masked_str);
  /* The unclipped raster answers this trajectory at two instants, 10 and 70;
   * the clip removes the second, so a single instant remains */
  assert(temporal_num_instants(masked) == 1);
  assert(tfloat_start_value(masked) == 10.0);
  free(masked_str); free(masked);

  /* A geometry in another reference system states its positions in different
   * units, which is an error and not an empty answer */
  GSERIALIZED *region_3857 = geom_in("SRID=3857;POLYGON((0 1,2 1,2 3,0 3,0 1))",
    -1);
  assert(region_3857 != NULL);
  meos_errno_reset();
  assert(raster_clip(rast_values, NULL, 0, region_3857, NULL, 0, true,
    false) == NULL);
  assert(meos_errno() != 0);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_clip(NULL, NULL, 0, region, NULL, 0, true, false) == NULL);
  assert(raster_clip(rast_values, NULL, 0, NULL, NULL, 0, true,
    false) == NULL);
  assert(meos_errno() != 0);

  /* A pixel the geometry touches is kept where the caller asks for it, where
   * otherwise only a pixel whose centre the geometry covers is. The triangle
   * below covers the centre of pixel(1,1) = 10 alone and touches the nodata
   * pixel(1,2) and pixel(2,1) = 40, so the pixels carrying a value count one
   * without touched and two with it. It spans two pixels each way, since the
   * mask it is burnt into rounds its extent to whole pixels */
  GSERIALIZED *corner = geom_in("SRID=4326;POLYGON((0 3,1.8 3,0 1.2,0 3))",
    -1);
  assert(corner != NULL);
  meos_errno_reset();
  Raster *centres = raster_clip(rast_values, NULL, 0, corner, NULL, 0, false,
    false);
  Raster *touching = raster_clip(rast_values, NULL, 0, corner, NULL, 0, false,
    true);
  assert(centres != NULL && touching != NULL);
  assert(meos_errno() == 0);
  BandStats *cst = raster_summary_stats(centres, 1, true);
  BandStats *tst = raster_summary_stats(touching, 1, true);
  assert(cst != NULL && tst != NULL);
  printf("raster_clip(corner) without and with touched: %u and %u pixel(s)\n",
    cst->count, tst->count);
  assert(cst->count == 1 && cst->sum == 10.0);
  assert(tst->count == 2 && tst->sum == 10.0 + 40.0);
  free(cst); free(tst); free(centres); free(touching); free(corner);

  /* The bands kept are the ones named, and the pixels the geometry does not
   * cover answer the nodata value the caller states; a band the raster does
   * not have is refused */
  int keep[] = {1};
  double nodata_one[] = {-1.0};
  meos_errno_reset();
  Raster *kept1 = raster_clip(rast_values, keep, 1, region, nodata_one, 1,
    true, false);
  assert(kept1 != NULL);
  assert(meos_errno() == 0);
  double kept1_nodata = 0.0;
  assert(raster_band_nodata_value(kept1, 1, &kept1_nodata));
  assert(kept1_nodata == -1.0);
  free(kept1);
  int missing[] = {2};
  meos_errno_reset();
  assert(raster_clip(rast_values, missing, 1, region, NULL, 0, true,
    false) == NULL);
  assert(meos_errno() != 0);

  free(region_3857); free(region); free(whole); free(cropped);
  free(traj_kept);

  /* Reprojection carries the coverage into another reference system rather
   * than relabelling the same pixels. Web Mercator states a position in
   * metres, so the pixel of one degree becomes a pixel of some 10^5 metres,
   * and the meridian of longitude 0 stands at x = 0 in both systems. */
  meos_errno_reset();
  Raster *merc = raster_transform(rast_values, 3857, NULL, 0.0, 0.0, 0.0);
  assert(merc != NULL);
  assert(meos_errno() == 0);
  printf("raster_transform(raster, 3857): srid %d, %dx%d, scale (%f, %f), "
    "upper left (%f, %f)\n", raster_srid(merc), raster_width(merc),
    raster_height(merc), raster_scale_x(merc), raster_scale_y(merc),
    raster_upper_left_x(merc), raster_upper_left_y(merc));
  assert(raster_srid(merc) == 3857);
  assert(raster_num_bands(merc) == 1);
  /* The units are those of the target system, not of the source */
  assert(raster_scale_x(merc) > 1000.0);
  assert(raster_upper_left_x(merc) > -1e-6 &&
    raster_upper_left_x(merc) < 1e-6);
  assert(raster_upper_left_y(merc) > 0.0);

  /* The DATA moves with the grid, which the header alone cannot show. The
   * oracle is the result's own geotransform rather than a projection formula:
   * both systems are north up, so the upper left pixel of the result covers
   * the ground the upper left pixel of the subject covers, and reading the
   * centre of that pixel must answer what that ground answered before */
  char traj_merc_str[256];
  snprintf(traj_merc_str, sizeof(traj_merc_str),
    "SRID=3857;{POINT(%.6f %.6f)@2001-01-01}",
    raster_upper_left_x(merc) + raster_scale_x(merc) / 2.0,
    raster_upper_left_y(merc) + raster_scale_y(merc) / 2.0);
  Temporal *traj_merc = tgeompoint_in(traj_merc_str);
  assert(traj_merc != NULL);
  meos_errno_reset();
  Temporal *merc_val = raster_value(traj_merc, merc, 1, true, NULL);
  assert(merc_val != NULL);
  assert(meos_errno() == 0);
  char *merc_val_str = tfloat_out(merc_val, 0);
  printf("raster_transform(raster, 3857) sampled at the centre of its upper "
    "left pixel: %s\n", merc_val_str);
  assert(temporal_num_instants(merc_val) == 1);
  assert(tfloat_start_value(merc_val) == 10.0);
  free(merc_val_str); free(merc_val);
  free(traj_merc);

  /* A coordinate operation GDAL would derive answers as the reprojection to
   * the same system does: the Web Mercator projection on the WGS84 ellipsoid is
   * the operation from EPSG:4326 to EPSG:3857, read longitude first and in
   * degrees as the geometry pipeline reads it, and its inverse carries the
   * result back to EPSG:4326 */
  meos_errno_reset();
  Raster *merc_pipe = raster_transform_pipeline(rast_values,
    "+proj=webmerc +ellps=WGS84", 3857, true, NULL, 0.0, 0.0, 0.0);
  assert(merc_pipe != NULL);
  assert(meos_errno() == 0);
  printf("raster_transform_pipeline(raster, webmerc, 3857): srid %d, %dx%d, "
    "scale (%f, %f), upper left (%f, %f)\n", raster_srid(merc_pipe),
    raster_width(merc_pipe), raster_height(merc_pipe),
    raster_scale_x(merc_pipe), raster_scale_y(merc_pipe),
    raster_upper_left_x(merc_pipe), raster_upper_left_y(merc_pipe));
  assert(raster_srid(merc_pipe) == 3857);
  assert(raster_width(merc_pipe) == raster_width(merc));
  assert(raster_height(merc_pipe) == raster_height(merc));
  assert(raster_scale_x(merc_pipe) == raster_scale_x(merc));
  assert(raster_scale_y(merc_pipe) == raster_scale_y(merc));
  assert(raster_upper_left_x(merc_pipe) == raster_upper_left_x(merc));
  assert(raster_upper_left_y(merc_pipe) == raster_upper_left_y(merc));
  Raster *back = raster_transform_pipeline(merc_pipe,
    "+proj=webmerc +ellps=WGS84", 4326, false, NULL, 0.0, 0.0, 0.0);
  assert(back != NULL);
  assert(meos_errno() == 0);
  assert(raster_srid(back) == 4326);
  free(back); free(merc_pipe);
  /* A null argument is rejected rather than dereferenced */
  assert(raster_transform_pipeline(NULL, "+proj=webmerc +ellps=WGS84", 3857,
    true, NULL, 0.0, 0.0, 0.0) == NULL);
  assert(raster_transform_pipeline(rast_values, NULL, 3857, true, NULL, 0.0,
    0.0, 0.0) == NULL);
  assert(meos_errno() != 0);
  free(merc);

  /* A pixel size stated for the result fixes its grid, and a raster to align
   * to hands over its reference system, its pixel size and its grid origin, so
   * the upper left corner of the result lies a whole number of pixels from the
   * corner of the grid it aligns to */
  meos_errno_reset();
  Raster *scaled = raster_transform(rast_values, 3857, NULL, 0.0, 150000.0,
    -150000.0);
  assert(scaled != NULL);
  assert(meos_errno() == 0);
  assert(raster_srid(scaled) == 3857);
  assert(raster_scale_x(scaled) == 150000.0);
  assert(raster_scale_y(scaled) == -150000.0);
  Raster *aligned = raster_transform_raster(rast_values, scaled, NULL, 0.0);
  assert(aligned != NULL);
  assert(meos_errno() == 0);
  assert(raster_srid(aligned) == 3857);
  assert(raster_scale_x(aligned) == 150000.0);
  double shift = (raster_upper_left_x(aligned) -
    raster_upper_left_x(scaled)) / 150000.0;
  printf("raster_transform_raster(raster, grid): upper left %f pixel(s) from "
    "the corner of the grid\n", shift);
  assert(shift == (double) (long) shift);
  free(aligned);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_transform_raster(NULL, scaled, NULL, 0.0) == NULL);
  assert(raster_transform_raster(rast_values, NULL, NULL, 0.0) == NULL);
  assert(meos_errno() != 0);
  free(scaled);

  /* Rescaling states the same coverage on a grid of the pixel size asked for,
   * keeping the reference system and the upper left corner. Halving the pixel
   * splits each of the nine into four, so the 3x3 of one degree becomes a 6x6
   * of half a degree over the same ground */
  meos_errno_reset();
  Raster *finer = raster_rescale(rast_values, 0.5, -0.5, NULL, 0.0);
  assert(finer != NULL);
  assert(meos_errno() == 0);
  printf("raster_rescale(raster, 0.5, -0.5): %dx%d, scale (%f, %f), "
    "upper left (%f, %f)\n", raster_width(finer), raster_height(finer),
    raster_scale_x(finer), raster_scale_y(finer),
    raster_upper_left_x(finer), raster_upper_left_y(finer));
  assert(raster_srid(finer) == 4326);
  assert(raster_num_bands(finer) == 1);
  assert(raster_width(finer) == 6);
  assert(raster_height(finer) == 6);
  assert(raster_scale_x(finer) == 0.5);
  assert(raster_scale_y(finer) == -0.5);
  assert(raster_upper_left_x(finer) == 0.0);
  assert(raster_upper_left_y(finer) == 3.0);

  /* Each quarter of the pixel that held 10 holds 10, which is what a nearest
   * neighbour resampling onto a finer grid means */
  Temporal *traj_finer = tgeompoint_in("SRID=4326;{POINT(0.25 2.75)@2001-01-01,"
    " POINT(0.75 2.75)@2001-01-02, POINT(0.25 2.25)@2001-01-03,"
    " POINT(0.75 2.25)@2001-01-04}");
  assert(traj_finer != NULL);
  meos_errno_reset();
  Temporal *finer_val = raster_value(traj_finer, finer, 1, true, NULL);
  assert(finer_val != NULL);
  assert(meos_errno() == 0);
  char *finer_str = tfloat_out(finer_val, 0);
  printf("raster_rescale(raster, 0.5, -0.5) sampled over the quarters of the "
    "pixel holding 10: %s\n", finer_str);
  assert(temporal_num_instants(finer_val) == 4);
  assert(tfloat_start_value(finer_val) == 10.0);
  assert(tfloat_end_value(finer_val) == 10.0);
  free(finer_str); free(finer_val); free(traj_finer); free(finer);

  /* The algorithm is read without regard to case, and a name the set does not
   * hold raises rather than resampling by another algorithm than the one
   * asked for */
  meos_errno_reset();
  Raster *bilinear = raster_rescale(rast_values, 0.5, -0.5, "BiLiNeAr", 0.0);
  assert(bilinear != NULL);
  assert(meos_errno() == 0);
  assert(raster_width(bilinear) == 6);
  free(bilinear);

  meos_errno_reset();
  assert(raster_rescale(rast_values, 0.5, -0.5, "Quadratic", 0.0) == NULL);
  assert(meos_errno() != 0);

  /* A reprojection states where it goes, a rescaling states a pixel that
   * covers ground, and a warp commits no negative error */
  meos_errno_reset();
  /* 0 is the unknown SRID; the public surface publishes no name for it */
  assert(raster_transform(rast_values, 0, NULL, 0.0, 0.0, 0.0) == NULL);
  assert(raster_rescale(rast_values, 0.0, -0.5, NULL, 0.0) == NULL);
  assert(raster_rescale(rast_values, 0.5, 0.0, NULL, 0.0) == NULL);
  assert(raster_transform(rast_values, 3857, NULL, -1.0, 0.0, 0.0) == NULL);
  assert(meos_errno() != 0);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_transform(NULL, 3857, NULL, 0.0, 0.0, 0.0) == NULL);
  assert(raster_rescale(NULL, 0.5, -0.5, NULL, 0.0) == NULL);
  assert(meos_errno() != 0);

  /* Reading a band as polygons states the same band as the regions its values
   * cover. The 3x3 band holds eight distinct values and one nodata pixel, and
   * no two neighbouring pixels share a value, so every pixel is a region of
   * its own: eight polygons excluding the nodata pixel, nine including it */
  int npolys = 0;
  meos_errno_reset();
  GeomVal *polys = raster_dump_as_polygons(rast_values, 1, true, &npolys);
  assert(polys != NULL);
  assert(meos_errno() == 0);
  printf("raster_dump_as_polygons(raster, 1, true): %d polygon(s)\n", npolys);
  assert(npolys == 8);

  /* Each polygon carries the reference system of the raster, so it can be
   * compared with the trajectories the coverage is read along, and each
   * covers exactly the one pixel whose value it states */
  double total = 0.0;
  bool seen_10 = false;
  for (int i = 0; i < npolys; i++)
  {
    assert(polys[i].geom != NULL);
    assert(geo_srid(polys[i].geom) == 4326);
    /* A pixel of one degree by one degree covers an area of 1 */
    assert(geom_area(polys[i].geom) > 0.999 &&
      geom_area(polys[i].geom) < 1.001);
    total += polys[i].val;
    if (polys[i].val == 10.0)
      seen_10 = true;
  }
  /* The eight values the band states, the nodata pixel excluded */
  printf("raster_dump_as_polygons(raster, 1, true) values sum to %f\n", total);
  assert(total == 10.0 + 30.0 + 40.0 + 50.0 + 60.0 + 70.0 + 80.0 + 90.0);
  assert(seen_10);
  geomval_arr_free(polys, npolys);

  /* Keeping the nodata pixel gives it a region of its own, so the count rises
   * by exactly one */
  meos_errno_reset();
  int npolys_all = 0;
  GeomVal *polys_all = raster_dump_as_polygons(rast_values, 1, false,
    &npolys_all);
  assert(polys_all != NULL);
  assert(meos_errno() == 0);
  printf("raster_dump_as_polygons(raster, 1, false): %d polygon(s)\n",
    npolys_all);
  assert(npolys_all == npolys + 1);
  geomval_arr_free(polys_all, npolys_all);

  /* A band the raster does not have is an error, as it is for every other
   * accessor of this family, and a null argument is rejected */
  meos_errno_reset();
  int nbad = -1;
  assert(raster_dump_as_polygons(rast_values, 0, true, &nbad) == NULL);
  assert(nbad == 0);
  assert(raster_dump_as_polygons(rast_values, 2, true, &nbad) == NULL);
  assert(raster_dump_as_polygons(NULL, 1, true, &nbad) == NULL);
  assert(raster_dump_as_polygons(rast_values, 1, true, NULL) == NULL);
  assert(meos_errno() != 0);

  /* Releasing an empty answer is not an error */
  geomval_arr_free(NULL, 0);

  /* What the band amounts to, read in one pass. Excluding nodata the eight
   * values are 10, 30, 40, 50, 60, 70, 80 and 90, so every statistic follows
   * in closed form: they sum to 430 and average 53.75, the smallest is 10 and
   * the largest 90 */
  meos_errno_reset();
  BandStats *st = raster_summary_stats(rast_values, 1, true);
  assert(st != NULL);
  assert(meos_errno() == 0);
  printf("raster_summary_stats(raster, 1, true): count %u, sum %f, mean %f, "
    "stddev %f, min %f, max %f\n", st->count, st->sum, st->mean, st->stddev,
    st->min, st->max);
  assert(st->count == 8);
  assert(st->sum == 430.0);
  assert(st->mean == 53.75);
  assert(st->min == 10.0);
  assert(st->max == 90.0);
  /* The deviations square to 4987.5, so the population standard deviation is
   * sqrt(4987.5/8) = 24.96873, and the sample one would be sqrt(4987.5/7) =
   * 26.69270. The band is read whole rather than sampled, so it is the first,
   * and the bound below separates the two conventions rather than merely
   * bracketing a number */
  assert(st->stddev > 24.96 && st->stddev < 24.98);
  free(st);

  /* Counting the nodata pixel counts the value it holds, -9999, so the count
   * rises by one and the sum and the minimum move to it */
  meos_errno_reset();
  BandStats *st_all = raster_summary_stats(rast_values, 1, false);
  assert(st_all != NULL);
  assert(meos_errno() == 0);
  printf("raster_summary_stats(raster, 1, false): count %u, sum %f, min %f, "
    "max %f\n", st_all->count, st_all->sum, st_all->min, st_all->max);
  assert(st_all->count == 9);
  assert(st_all->sum == 430.0 - 9999.0);
  assert(st_all->min == -9999.0);
  assert(st_all->max == 90.0);
  free(st_all);

  /* A band the raster does not have is an error, and a null argument is
   * rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_summary_stats(rast_values, 0, true) == NULL);
  assert(raster_summary_stats(rast_values, 2, true) == NULL);
  assert(raster_summary_stats(NULL, 1, true) == NULL);
  assert(meos_errno() != 0);

  /* Reclassifying maps the values of a band onto the classes an expression
   * names. The band holds 10, 30, 40, 50, 60, 70, 80 and 90 outside its
   * nodata pixel.
   * A RANGE WRITTEN PLAINLY IS HALF OPEN AT THE TOP: 0-50 states 0 included
   * and 50 EXCLUDED, since the high end is inclusive only where a closing
   * bracket says so. So 10, 30 and 40 take class 1, 60, 70, 80 and 90 take
   * class 2, and 50 falls in NEITHER range and is left unmapped. The summary
   * of the result is the oracle: seven pixels carrying three 1s and four 2s,
   * which sum to 11 */
  meos_errno_reset();
  Raster *rc = raster_reclass(rast_values, 1, "0-50:1, 51-100:2", "32BF",
    true, -9999.0);
  assert(rc != NULL);
  assert(meos_errno() == 0);
  BandStats *rcst = raster_summary_stats(rc, 1, true);
  assert(rcst != NULL);
  printf("raster_reclass(raster, 1, \"0-50:1, 51-100:2\"): count %u, sum %f, "
    "min %f, max %f\n", rcst->count, rcst->sum, rcst->min, rcst->max);
  assert(rcst->count == 7);
  assert(rcst->sum == 3.0 * 1.0 + 4.0 * 2.0);
  assert(rcst->min == 1.0);
  assert(rcst->max == 2.0);
  free(rcst);

  /* The grid and the reference system are those of the subject: only the
   * values of the band move */
  assert(raster_width(rc) == 3);
  assert(raster_height(rc) == 3);
  assert(raster_srid(rc) == 4326);
  assert(raster_upper_left_x(rc) == 0.0);
  assert(raster_upper_left_y(rc) == 3.0);

  /* The pixel holding 10 is in the first class and the one holding 90 in the
   * second, read where they stand rather than in aggregate */
  Temporal *traj_rc = tgeompoint_in("SRID=4326;{POINT(0.5 2.5)@2001-01-01,"
    " POINT(2.5 0.5)@2001-01-02}");
  assert(traj_rc != NULL);
  Temporal *rc_val = raster_value(traj_rc, rc, 1, true, NULL);
  assert(rc_val != NULL);
  char *rc_str = tfloat_out(rc_val, 0);
  printf("raster_reclass sampled at the 10 pixel and the 90 pixel: %s\n",
    rc_str);
  assert(tfloat_start_value(rc_val) == 1.0);
  assert(tfloat_end_value(rc_val) == 2.0);
  free(rc_str); free(rc_val); free(traj_rc); free(rc);

  /* A CLOSING BRACKET IS WHAT INCLUDES THE HIGH BOUND, and writing one brings
   * the value the plain form dropped back in: [50-100] takes 50, so every one
   * of the eight values is mapped and the pixel holding 50 reads class 2.
   * Three 1s and five 2s sum to 13 over eight pixels, against the seven and
   * the 11 above -- the difference IS the bracket */
  meos_errno_reset();
  Raster *rb = raster_reclass(rast_values, 1, "[0-50):1, [50-100]:2", "32BF",
    true, -9999.0);
  assert(rb != NULL);
  assert(meos_errno() == 0);
  BandStats *rbst = raster_summary_stats(rb, 1, true);
  assert(rbst != NULL);
  printf("raster_reclass(\"[0-50):1, [50-100]:2\"): count %u, sum %f\n",
    rbst->count, rbst->sum);
  assert(rbst->count == 8);
  assert(rbst->sum == 3.0 * 1.0 + 5.0 * 2.0);
  free(rbst);
  Temporal *traj_50 = tgeompoint_in("SRID=4326;{POINT(1.5 1.5)@2001-01-01}");
  assert(traj_50 != NULL);
  Temporal *v50 = raster_value(traj_50, rb, 1, true, NULL);
  assert(v50 != NULL);
  printf("raster_reclass(\"[0-50):1, [50-100]:2\") at the 50 pixel: %f\n",
    tfloat_start_value(v50));
  assert(tfloat_start_value(v50) == 2.0);
  free(v50); free(traj_50);

  /* Its neighbour holding 40 stands inside [0-50) and takes class 1, which is
   * what says the first range is read at all rather than everything falling
   * through to the second */
  Temporal *traj_40 = tgeompoint_in("SRID=4326;{POINT(0.5 1.5)@2001-01-01}");
  assert(traj_40 != NULL);
  Temporal *v40 = raster_value(traj_40, rb, 1, true, NULL);
  assert(v40 != NULL);
  assert(tfloat_start_value(v40) == 1.0);
  free(v40); free(traj_40); free(rb);

  /* A RANGE OF NEGATIVE BOUNDS PARSES, which is the case the splitting turns
   * on: -9999--1 states two bounds, not four empty pieces. Counting the nodata
   * pixel as the value it holds puts it in that range, so the nine pixels read
   * one 7 and eight 8s, summing to 71 */
  meos_errno_reset();
  Raster *rn = raster_reclass(rast_values, 1, "-9999--1:7, 0-1000:8", "32BF",
    false, 0.0);
  assert(rn != NULL);
  assert(meos_errno() == 0);
  BandStats *rnst = raster_summary_stats(rn, 1, false);
  assert(rnst != NULL);
  printf("raster_reclass(\"-9999--1:7, 0-1000:8\"): count %u, sum %f, min %f, "
    "max %f\n", rnst->count, rnst->sum, rnst->min, rnst->max);
  assert(rnst->count == 9);
  assert(rnst->sum == 7.0 + 8.0 * 8.0);
  assert(rnst->min == 7.0);
  assert(rnst->max == 8.0);
  free(rnst); free(rn);

  /* A malformed expression raises rather than answering the subject unchanged,
   * which is what PostGIS does: a mapping needs its colon, a range needs
   * numbers for bounds, and the pixel type has to be one the catalog names */
  meos_errno_reset();
  assert(raster_reclass(rast_values, 1, "0-50", "32BF", true, 0.0) == NULL);
  assert(raster_reclass(rast_values, 1, "0-50:x", "32BF", true, 0.0) == NULL);
  assert(raster_reclass(rast_values, 1, "0-50:1", "99XX", true, 0.0) == NULL);
  assert(raster_reclass(rast_values, 0, "0-50:1", "32BF", true, 0.0) == NULL);
  assert(raster_reclass(NULL, 1, "0-50:1", "32BF", true, 0.0) == NULL);
  assert(raster_reclass(rast_values, 1, NULL, "32BF", true, 0.0) == NULL);
  assert(raster_reclass(rast_values, 1, "0-50:1", NULL, true, 0.0) == NULL);
  assert(meos_errno() != 0);

  /* The arithmetic of rasters is computed pixel by pixel into 64-bit float
   * bands, so the normalized difference of two unsigned 16-bit bands is the
   * ratio itself. The near infrared and the red bands hold 3000, 500, 2000 /
   * 4000, 1000, 6000 / 100, 0, 65535 and 1000, 1500, 2000 / 1000, 3000, 2000
   * / 300, 1, 0, so (nir - red) / (nir + red) is 0.5, -0.5, 0 / 0.6, -0.5,
   * 0.5 / -0.5, -1, 1. PostGIS's ST_MapAlgebra over the same two
   * rasters with the '64BF' pixel type answers these values, but for the 0,
   * which it reads as nodata since it takes the minimum of a band stating no
   * nodata value, 0 for 16BUI, as the nodata value of the result. The bands
   * here state none, so the result states none either and the 0 is a value:
   * nine pixels summing to 0.1 */
  meos_errno_reset();
  Raster *nir = raster_from_hexwkb(raster_nir);
  Raster *red = raster_from_hexwkb(raster_red);
  assert(nir != NULL && red != NULL);
  Raster *nir_minus_red = sub_raster_raster(nir, red);
  Raster *nir_plus_red = add_raster_raster(nir, red);
  assert(nir_minus_red != NULL && nir_plus_red != NULL);
  Raster *ndvi = div_raster_raster(nir_minus_red, nir_plus_red);
  assert(ndvi != NULL);
  assert(meos_errno() == 0);
  char *ndvi_pixtype = raster_band_pixel_type(ndvi, 1);
  assert(ndvi_pixtype != NULL);
  assert(strcmp(ndvi_pixtype, "float64") == 0);
  assert(! raster_band_has_nodata_value(ndvi, 1));
  BandStats *ndvist = raster_summary_stats(ndvi, 1, true);
  assert(ndvist != NULL);
  printf("div_raster_raster(nir - red, nir + red): %s, count %u, sum %.17g, "
    "min %g, max %g\n", ndvi_pixtype, ndvist->count, ndvist->sum,
    ndvist->min, ndvist->max);
  assert(ndvist->count == 9);
  assert(fabs(ndvist->sum - 0.1) < 1e-12);
  assert(ndvist->min == -1.0);
  assert(ndvist->max == 1.0);
  free(ndvist); free(ndvi_pixtype); free(ndvi);

  /* The difference keeps its sign and the product its magnitude, which an
   * unsigned 16-bit band would clamp to 0 and to 65535: nir - red runs from
   * -2000 to 65535, and nir * red reaches 6000 * 2000 */
  BandStats *diffst = raster_summary_stats(nir_minus_red, 1, true);
  assert(diffst != NULL);
  printf("sub_raster_raster(nir, red): min %g, max %g\n", diffst->min,
    diffst->max);
  assert(diffst->min == -2000.0);
  assert(diffst->max == 65535.0);
  free(diffst);
  Raster *nir_times_red = mul_raster_raster(nir, red);
  assert(nir_times_red != NULL);
  BandStats *mulst = raster_summary_stats(nir_times_red, 1, true);
  assert(mulst != NULL);
  printf("mul_raster_raster(nir, red): max %g\n", mulst->max);
  assert(mulst->max == 12000000.0);
  free(mulst); free(nir_times_red); free(nir_minus_red); free(nir_plus_red);

  /* A raster and a number: the band of raster_values holds 10, 30, 40, 50,
   * 60, 70, 80 and 90 outside its nodata pixel, which stays nodata in the
   * result, with the nodata value of the operand */
  meos_errno_reset();
  Raster *rv_sub = sub_raster_float(rast_values, 10.0);
  assert(rv_sub != NULL);
  assert(meos_errno() == 0);
  double rv_nodata;
  assert(raster_band_nodata_value(rv_sub, 1, &rv_nodata));
  assert(rv_nodata == -9999.0);
  BandStats *rvst = raster_summary_stats(rv_sub, 1, true);
  assert(rvst != NULL);
  printf("sub_raster_float(raster, 10): count %u, sum %g, min %g, max %g\n",
    rvst->count, rvst->sum, rvst->min, rvst->max);
  assert(rvst->count == 8);
  assert(rvst->sum == 430.0 - 80.0);
  assert(rvst->min == 0.0);
  assert(rvst->max == 80.0);
  free(rvst); free(rv_sub);

  /* The number first is the number the operation starts from */
  Raster *rv_fsub = sub_float_raster(100.0, rast_values);
  assert(rv_fsub != NULL);
  rvst = raster_summary_stats(rv_fsub, 1, true);
  assert(rvst != NULL);
  printf("sub_float_raster(100, raster): count %u, sum %g, min %g, max %g\n",
    rvst->count, rvst->sum, rvst->min, rvst->max);
  assert(rvst->count == 8);
  assert(rvst->sum == 800.0 - 430.0);
  assert(rvst->min == 10.0);
  assert(rvst->max == 90.0);
  free(rvst); free(rv_fsub);

  Raster *rv_add = add_float_raster(1.0, rast_values);
  Raster *rv_mul = mul_raster_float(rast_values, 2.0);
  Raster *rv_div = div_raster_float(rast_values, 10.0);
  Raster *rv_fdiv = div_float_raster(360.0, rast_values);
  assert(rv_add != NULL && rv_mul != NULL && rv_div != NULL &&
    rv_fdiv != NULL);
  BandStats *addst = raster_summary_stats(rv_add, 1, true);
  BandStats *mulrst = raster_summary_stats(rv_mul, 1, true);
  BandStats *divst = raster_summary_stats(rv_div, 1, true);
  BandStats *fdivst = raster_summary_stats(rv_fdiv, 1, true);
  printf("add_float_raster(1, raster): sum %g; mul_raster_float(raster, 2): "
    "sum %g; div_raster_float(raster, 10): sum %g; div_float_raster(360, "
    "raster): min %g, max %g\n", addst->sum, mulrst->sum, divst->sum,
    fdivst->min, fdivst->max);
  assert(addst->count == 8 && addst->sum == 438.0);
  assert(mulrst->count == 8 && mulrst->sum == 860.0);
  assert(divst->count == 8 && divst->sum == 43.0);
  assert(fdivst->count == 8 && fdivst->min == 4.0 && fdivst->max == 36.0);
  free(addst); free(mulrst); free(divst); free(fdivst);
  free(rv_add); free(rv_mul); free(rv_div); free(rv_fdiv);

  /* Two rasters: a pixel nodata in either operand is nodata in the result */
  Raster *rv_twice = add_raster_raster(rast_values, rast_values);
  assert(rv_twice != NULL);
  BandStats *twicest = raster_summary_stats(rv_twice, 1, true);
  assert(twicest != NULL);
  printf("add_raster_raster(raster, raster): count %u, sum %g\n",
    twicest->count, twicest->sum);
  assert(twicest->count == 8);
  assert(twicest->sum == 860.0);
  free(twicest); free(rv_twice);

  /* A division by zero is an error, as it is for a temporal number, whether
   * the divisor is a number or a pixel: the raster minus itself is a band of
   * zeros outside its nodata pixel */
  meos_errno_reset();
  assert(div_raster_float(rast_values, 0.0) == NULL);
  assert(meos_errno() == MEOS_ERR_DIVISION_BY_ZERO);
  Raster *zeros = sub_raster_raster(rast_values, rast_values);
  assert(zeros != NULL);
  meos_errno_reset();
  assert(div_float_raster(1.0, zeros) == NULL);
  assert(meos_errno() == MEOS_ERR_DIVISION_BY_ZERO);
  meos_errno_reset();
  assert(div_raster_raster(rast_values, zeros) == NULL);
  assert(meos_errno() == MEOS_ERR_DIVISION_BY_ZERO);
  /* Zero divided by a number is not */
  meos_errno_reset();
  Raster *zeros_div = div_raster_float(zeros, 2.0);
  assert(zeros_div != NULL);
  assert(meos_errno() == 0);
  free(zeros_div); free(zeros);

  /* Two rasters are operated over the pixels they share, as two temporal
   * numbers are over the time they share: the grid one pixel to the east
   * shares the last two columns of raster_values, nodata, 30 / 50, 60 / 80,
   * 90, and a band of ones adds 1 to each value */
  Raster *east1 = raster_from_hexwkb(raster_east1);
  assert(east1 != NULL);
  meos_errno_reset();
  Raster *shared = add_raster_raster(rast_values, east1);
  assert(shared != NULL);
  assert(meos_errno() == 0);
  BandStats *sharedst = raster_summary_stats(shared, 1, true);
  assert(sharedst != NULL);
  printf("add_raster_raster(raster, raster one pixel east): %dx%d from "
    "(%g, %g), count %u, sum %g\n", raster_width(shared),
    raster_height(shared), raster_upper_left_x(shared),
    raster_upper_left_y(shared), sharedst->count, sharedst->sum);
  assert(raster_width(shared) == 2);
  assert(raster_height(shared) == 3);
  assert(raster_upper_left_x(shared) == 1.0);
  assert(raster_upper_left_y(shared) == 3.0);
  assert(raster_srid(shared) == 4326);
  assert(sharedst->count == 5);
  assert(sharedst->sum == 30.0 + 50.0 + 60.0 + 80.0 + 90.0 + 5.0);
  free(sharedst); free(shared); free(east1);

  /* Rasters sharing no pixel have no result, and no error */
  Raster *east10 = raster_from_hexwkb(raster_east10);
  assert(east10 != NULL);
  meos_errno_reset();
  assert(add_raster_raster(rast_values, east10) == NULL);
  assert(meos_errno() == 0);
  free(east10);

  /* Rasters on different grids, in different reference systems, or with
   * different numbers of bands are not operated */
  Raster *easthalf = raster_from_hexwkb(raster_easthalf);
  Raster *rast_3857 = raster_from_hexwkb(raster_srid3857);
  Raster *two_bands = raster_from_hexwkb(raster_two_bands);
  assert(easthalf != NULL && rast_3857 != NULL && two_bands != NULL);
  meos_errno_reset();
  Raster *bad = add_raster_raster(rast_values, easthalf);
  printf("add_raster_raster(raster, raster half a pixel east): %s, errno %d\n",
    bad ? "a raster" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  bad = add_raster_raster(rast_values, rast_3857);
  printf("add_raster_raster(raster SRID 4326, raster SRID 3857): %s, "
    "errno %d\n", bad ? "a raster" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);
  meos_errno_reset();
  bad = add_raster_raster(rast_values, two_bands);
  printf("add_raster_raster(raster of 1 band, raster of 2 bands): %s, "
    "errno %d\n", bad ? "a raster" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);

  /* Band by band: two rasters of two bands answer two bands */
  meos_errno_reset();
  Raster *two_twice = add_raster_raster(two_bands, two_bands);
  assert(two_twice != NULL);
  assert(meos_errno() == 0);
  assert(raster_num_bands(two_twice) == 2);
  char *band2_pixtype = raster_band_pixel_type(two_twice, 2);
  assert(band2_pixtype != NULL);
  assert(strcmp(band2_pixtype, "float64") == 0);
  free(band2_pixtype); free(two_twice);
  free(easthalf); free(rast_3857); free(two_bands);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(add_raster_raster(NULL, rast_values) == NULL);
  assert(add_raster_raster(rast_values, NULL) == NULL);
  assert(add_raster_float(NULL, 1.0) == NULL);
  assert(add_float_raster(1.0, NULL) == NULL);
  assert(meos_errno() != 0);
  free(nir); free(red);

  /* The terrain functions read each pixel with its eight neighbours. The
   * elevations of raster_dem are 10, 20, 30, 40 / 15, 25, nodata, 45 / 20,
   * 30, 40, 50 / 25, 35, 45, 55 over 1 unit pixels, and every statistic below
   * is the one PostGIS 3.6.4 answers for ST_Slope, ST_TRI and ST_TPI over the
   * same raster with the 64BF pixel type: the nodata pixel stays nodata, and a
   * neighbour that is nodata or lies outside the raster takes the value of the
   * pixel it surrounds */
  Raster *dem = raster_from_hexwkb(raster_dem);
  assert(dem != NULL);
  meos_errno_reset();
  Raster *slope_deg = raster_slope(dem, 1, NULL, "64BF", "DEGREES", 1.0, false);
  assert(slope_deg != NULL);
  assert(meos_errno() == 0);
  BandStats *terst = raster_summary_stats(slope_deg, 1, true);
  assert(terst != NULL);
  printf("raster_slope(dem, DEGREES): count %u, sum %.6f, min %.6f, max %.6f\n",
    terst->count, terst->sum, terst->min, terst->max);
  assert(terst->count == 15);
  assert(fabs(terst->sum - 1196.94882782578) < 1e-9);
  assert(fabs(terst->min - 70.31426989758563) < 1e-9);
  assert(fabs(terst->max - 84.77393068530789) < 1e-9);
  double tnodata;
  assert(raster_band_nodata_value(slope_deg, 1, &tnodata));
  assert(tnodata == -9999.0);
  char *tpixtype = raster_band_pixel_type(slope_deg, 1);
  assert(strcmp(tpixtype, "float64") == 0);
  free(tpixtype); free(terst); free(slope_deg);

  Raster *slope_per = raster_slope(dem, 1, NULL, "64BF", "percent", 1.0, false);
  assert(slope_per != NULL);
  terst = raster_summary_stats(slope_per, 1, true);
  printf("raster_slope(dem, percent): sum %.6f, max %.6f\n", terst->sum,
    terst->max);
  assert(fabs(terst->sum - 9708.753762847418) < 1e-9);
  assert(fabs(terst->max - 1093.3034802834939) < 1e-9);
  free(terst); free(slope_per);

  /* The scale divides the gradient, as PostGIS's does */
  Raster *slope_scale = raster_slope(dem, 1, NULL, "64BF", "DEG", 2.0, false);
  assert(slope_scale != NULL);
  terst = raster_summary_stats(slope_scale, 1, true);
  printf("raster_slope(dem, DEG, scale 2): sum %.6f\n", terst->sum);
  assert(fabs(terst->sum - 1057.240068859269) < 1e-9);
  free(terst); free(slope_scale);

  /* Interpolating the nodata pixel first gives it the mean of its eight
   * neighbours weighted by the inverse of their distances, 35, so every pixel
   * has a slope */
  Raster *slope_interp = raster_slope(dem, 1, NULL, "64BF", "DEGREES", 1.0,
    true);
  assert(slope_interp != NULL);
  terst = raster_summary_stats(slope_interp, 1, true);
  printf("raster_slope(dem, interpolated): count %u, sum %.6f, max %.6f\n",
    terst->count, terst->sum, terst->max);
  assert(terst->count == 16);
  assert(fabs(terst->sum - 1298.4430188481429) < 1e-9);
  assert(fabs(terst->max - 84.88891030471129) < 1e-9);
  free(terst); free(slope_interp);

  Raster *tri = raster_tri(dem, 1, NULL, "64BF", false);
  Raster *tpi = raster_tpi(dem, 1, NULL, "64BF", false);
  Raster *tri_interp = raster_tri(dem, 1, NULL, "64BF", true);
  Raster *tpi_interp = raster_tpi(dem, 1, NULL, "64BF", true);
  assert(tri && tpi && tri_interp && tpi_interp);
  BandStats *tristat = raster_summary_stats(tri, 1, true);
  BandStats *tpistat = raster_summary_stats(tpi, 1, true);
  BandStats *triistat = raster_summary_stats(tri_interp, 1, true);
  BandStats *tpiistat = raster_summary_stats(tpi_interp, 1, true);
  printf("raster_tri(dem): count %u, sum %g, min %g, max %g; raster_tpi(dem): "
    "sum %g, min %g, max %g\n", tristat->count, tristat->sum, tristat->min,
    tristat->max, tpistat->sum, tpistat->min, tpistat->max);
  printf("raster_tri(dem, interpolated): count %u, sum %g, max %g; "
    "raster_tpi(dem, interpolated): count %u, sum %g\n", triistat->count,
    triistat->sum, triistat->max, tpiistat->count, tpiistat->sum);
  assert(tristat->count == 15 && tristat->sum == 72.5);
  assert(tristat->min == 1.875 && tristat->max == 8.125);
  assert(tpistat->count == 15 && tpistat->sum == 0.0);
  assert(tpistat->min == -3.75 && tpistat->max == 3.75);
  assert(triistat->count == 16 && triistat->sum == 90.0);
  assert(triistat->max == 8.75);
  assert(tpiistat->count == 16 && fabs(tpiistat->sum) < 1e-12);
  free(tristat); free(tpistat); free(triistat); free(tpiistat);
  free(tri); free(tpi); free(tri_interp); free(tpi_interp);

  /* A custom extent decides the grid of the result: the 2x2 square from
   * (1, 3) reads the four pixels it covers, the nodata one among them */
  Raster *square = raster_from_hexwkb(raster_dem_square);
  assert(square != NULL);
  meos_errno_reset();
  Raster *slope_square = raster_slope(dem, 1, square, "64BF", "DEGREES", 1.0,
    false);
  assert(slope_square != NULL);
  assert(meos_errno() == 0);
  terst = raster_summary_stats(slope_square, 1, true);
  printf("raster_slope(dem, 2x2 extent): %dx%d from (%g, %g), count %u, "
    "sum %.6f\n", raster_width(slope_square), raster_height(slope_square),
    raster_upper_left_x(slope_square), raster_upper_left_y(slope_square),
    terst->count, terst->sum);
  assert(raster_width(slope_square) == 2 && raster_height(slope_square) == 2);
  assert(raster_upper_left_x(slope_square) == 1.0);
  assert(raster_upper_left_y(slope_square) == 3.0);
  assert(terst->count == 3);
  assert(fabs(terst->sum - (83.66949059883729 + 84.77393068530789 +
    84.6508249017144)) < 1e-9);
  free(terst); free(slope_square); free(square);

  /* A plane rising by 1 for each pixel to the east has a gradient of 1: 45
   * degrees, 100 percent, and pi/4 radians, where PostGIS 3.6.4 answers 45,
   * comparing the units in upper case to 'rad' */
  Raster *plane = raster_from_hexwkb(raster_plane);
  assert(plane != NULL);
  Raster *plane_rad = raster_slope(plane, 1, NULL, "64BF", "RADIANS", 1.0,
    false);
  Raster *plane_deg = raster_slope(plane, 1, NULL, "64BF", "DEGREES", 1.0,
    false);
  assert(plane_rad != NULL && plane_deg != NULL);
  BandStats *radst = raster_summary_stats(plane_rad, 1, true);
  BandStats *degst = raster_summary_stats(plane_deg, 1, true);
  printf("raster_slope(plane): %.6f radians, %.6f degrees at the center\n",
    radst->max, degst->max);
  assert(fabs(radst->max - M_PI / 4.0) < 1e-12);
  assert(fabs(degst->max - 45.0) < 1e-12);
  free(radst); free(degst); free(plane_rad); free(plane_deg);

  /* Units, a pixel type or a band the raster does not have, a zero scale, a
   * custom extent on another grid and a null argument are refused */
  meos_errno_reset();
  assert(raster_slope(plane, 1, NULL, "64BF", "furlongs", 1.0, false) == NULL);
  assert(raster_slope(plane, 1, NULL, "99XX", "DEGREES", 1.0, false) == NULL);
  assert(raster_slope(plane, 2, NULL, "64BF", "DEGREES", 1.0, false) == NULL);
  assert(raster_slope(plane, 1, NULL, "64BF", "DEGREES", 0.0, false) == NULL);
  assert(raster_tri(plane, 1, dem, "64BF", false) == NULL);
  assert(raster_tpi(NULL, 1, NULL, "64BF", false) == NULL);
  assert(raster_slope(plane, 1, NULL, NULL, "DEGREES", 1.0, false) == NULL);
  assert(meos_errno() != 0);
  free(plane); free(dem);

  /* The histogram of a band: raster_counts holds 1.24, 1.26, nodata / 2, 2,
   * 2 / 3, 1.24, 2 in a 32BF band, so its values are the float images of
   * those numbers. Every row below is the one PostGIS 3.6.4 answers for
   * ST_ValueCount and ST_ValuePercent over the same raster */
  Raster *counts = raster_from_hexwkb(raster_counts);
  assert(counts != NULL);
  int nvc = 0;
  meos_errno_reset();
  ValueCount *vc = raster_value_count(counts, 1, true, NULL, 0, 0.0, &nvc);
  assert(vc != NULL);
  assert(meos_errno() == 0);
  printf("raster_value_count(counts):");
  for (int i = 0; i < nvc; i++)
    printf(" (%g, %u, %g)", vc[i].value, vc[i].count, vc[i].percent);
  printf("\n");
  assert(nvc == 4);
  double vcsum = 0.0;
  for (int i = 0; i < nvc; i++)
  {
    if (vc[i].value == (double) 1.24f)
      assert(vc[i].count == 2 && vc[i].percent == 0.25);
    else if (vc[i].value == (double) 1.26f)
      assert(vc[i].count == 1 && vc[i].percent == 0.125);
    else if (vc[i].value == 2.0)
      assert(vc[i].count == 4 && vc[i].percent == 0.5);
    else
      assert(vc[i].value == 3.0 && vc[i].count == 1);
    vcsum += vc[i].percent;
  }
  assert(vcsum == 1.0);
  free(vc);

  /* The nodata pixel is a value where nodata is not excluded */
  vc = raster_value_count(counts, 1, false, NULL, 0, 0.0, &nvc);
  assert(vc != NULL && nvc == 5);
  bool nodata_row = false;
  for (int i = 0; i < nvc; i++)
    if (vc[i].value == -9999.0)
      nodata_row = (vc[i].count == 1);
  assert(nodata_row);
  free(vc);

  /* Rounded to tenths, 1.24 and 1.26 are 1.2 and 1.3 */
  vc = raster_value_count(counts, 1, true, NULL, 0, 0.1, &nvc);
  assert(vc != NULL && nvc == 4);
  printf("raster_value_count(counts, roundto 0.1):");
  for (int i = 0; i < nvc; i++)
    printf(" (%g, %u)", vc[i].value, vc[i].count);
  printf("\n");
  bool tenths = false;
  for (int i = 0; i < nvc; i++)
    if (fabs(vc[i].value - 1.3) < 1e-9)
      tenths = (vc[i].count == 1);
  assert(tenths);
  free(vc);

  /* Search values answer one row each, in their order, a value no pixel
   * holds counting 0 */
  double search[2] = {2.0, 7.0};
  vc = raster_value_count(counts, 1, true, search, 2, 0.0, &nvc);
  assert(vc != NULL && nvc == 2);
  printf("raster_value_count(counts, {2, 7}): (%g, %u, %g) (%g, %u, %g)\n",
    vc[0].value, vc[0].count, vc[0].percent, vc[1].value, vc[1].count,
    vc[1].percent);
  assert(vc[0].value == 2.0 && vc[0].count == 4 && vc[0].percent == 0.5);
  assert(vc[1].value == 7.0 && vc[1].count == 0 && vc[1].percent == 0.0);
  free(vc);

  /* A band stating that every pixel is nodata answers no row where nodata is
   * excluded, and the nodata value over every pixel otherwise */
  Raster *allnodata = raster_from_hexwkb(raster_all_nodata);
  assert(allnodata != NULL);
  meos_errno_reset();
  vc = raster_value_count(allnodata, 1, true, NULL, 0, 0.0, &nvc);
  assert(vc == NULL && nvc == 0);
  assert(meos_errno() == 0);
  vc = raster_value_count(allnodata, 1, false, NULL, 0, 0.0, &nvc);
  assert(vc != NULL && nvc == 1);
  printf("raster_value_count(all nodata, nodata kept): (%g, %u, %g)\n",
    vc[0].value, vc[0].count, vc[0].percent);
  assert(vc[0].value == -9999.0 && vc[0].count == 4 && vc[0].percent == 1.0);
  free(vc); free(allnodata);

  /* A band the raster does not have, search values left unstated and a null
   * argument are refused */
  meos_errno_reset();
  assert(raster_value_count(counts, 2, true, NULL, 0, 0.0, &nvc) == NULL);
  assert(raster_value_count(counts, 1, true, NULL, 2, 0.0, &nvc) == NULL);
  assert(raster_value_count(NULL, 1, true, NULL, 0, 0.0, &nvc) == NULL);
  assert(raster_value_count(counts, 1, true, NULL, 0, 0.0, NULL) == NULL);
  assert(meos_errno() != 0);
  free(counts);

  /* Two rasters lie on the same grid where one is a whole number of pixels
   * away from the other, as PostGIS's ST_SameAlignment answers for the grid
   * of raster_values one pixel and half a pixel to the east */
  Raster *al_base = raster_from_hexwkb(raster_values);
  Raster *al_east1 = raster_from_hexwkb(raster_east1);
  Raster *al_easthalf = raster_from_hexwkb(raster_easthalf);
  Raster *al_3857 = raster_from_hexwkb(raster_srid3857);
  assert(al_base && al_east1 && al_easthalf && al_3857);
  meos_errno_reset();
  assert(raster_same_alignment(al_base, al_east1));
  assert(! raster_same_alignment(al_base, al_easthalf));
  assert(meos_errno() == 0);
  char *al_reason = raster_not_same_alignment_reason(al_base, al_east1);
  char *al_reason2 = raster_not_same_alignment_reason(al_base, al_easthalf);
  printf("raster_not_same_alignment_reason: \"%s\", \"%s\"\n", al_reason,
    al_reason2);
  assert(strcmp(al_reason, "The rasters are aligned") == 0);
  assert(strcmp(al_reason2,
    "The rasters (pixel corner coordinates) are not aligned") == 0);
  free(al_reason); free(al_reason2);

  /* Two reference systems raise, where PostGIS answers false */
  meos_errno_reset();
  assert(! raster_same_alignment(al_base, al_3857));
  assert(meos_errno() != 0);
  meos_errno_reset();
  assert(raster_not_same_alignment_reason(al_base, al_3857) == NULL);
  assert(meos_errno() != 0);

  /* The aggregate keeps the grid of the first raster and whether every later
   * one is aligned with it; a NULL raster leaves the state as it is */
  meos_errno_reset();
  RasterAlignState *al_state = raster_same_alignment_transfn(NULL, al_base);
  al_state = raster_same_alignment_transfn(al_state, NULL);
  al_state = raster_same_alignment_transfn(al_state, al_east1);
  assert(al_state != NULL && al_state->aligned);
  assert(meos_errno() == 0);

  /* The state crosses as bytes and is read back as it was */
  size_t al_size;
  uint8_t *al_bytes = rasteralignstate_serialize(al_state, &al_size);
  assert(al_bytes != NULL && al_size > 1);
  RasterAlignState *al_copy = rasteralignstate_deserialize(al_bytes,
    al_size);
  assert(al_copy != NULL && al_copy->aligned);
  assert(raster_same_alignment(al_copy->grid, al_base));
  free(al_bytes);

  /* A misaligned raster makes the state false, and combining it with an
   * aligned state keeps it false */
  RasterAlignState *al_bad = raster_same_alignment_transfn(NULL, al_base);
  al_bad = raster_same_alignment_transfn(al_bad, al_easthalf);
  assert(al_bad != NULL && ! al_bad->aligned);
  al_copy = raster_same_alignment_combinefn(al_copy, al_bad);
  assert(! al_copy->aligned);
  printf("raster_same_alignment_finalfn: aligned %d, misaligned %d, "
    "none %d\n", raster_same_alignment_finalfn(al_state),
    raster_same_alignment_finalfn(al_copy),
    raster_same_alignment_finalfn(NULL));
  assert(! raster_same_alignment_finalfn(al_bad));

  /* A raster in another reference system raises inside the aggregate too */
  meos_errno_reset();
  RasterAlignState *al_mixed = raster_same_alignment_transfn(NULL, al_base);
  assert(raster_same_alignment_transfn(al_mixed, al_3857) == NULL);
  assert(meos_errno() != 0);
  assert(raster_same_alignment_finalfn(al_mixed));

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(! raster_same_alignment(NULL, al_base));
  assert(raster_not_same_alignment_reason(al_base, NULL) == NULL);
  assert(rasteralignstate_serialize(NULL, &al_size) == NULL);
  assert(rasteralignstate_deserialize(NULL, 4) == NULL);
  assert(meos_errno() != 0);
  free(al_base); free(al_east1); free(al_easthalf); free(al_3857);

  free(vspan); free(traj_3857); free(traj_values); free(rast_values);

  meos_finalize();
  printf("raster_test: all assertions passed\n");
  return 0;
}
