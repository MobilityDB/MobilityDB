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
#include <limits.h>
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
  assert(raster_num_bands(NULL) == INT_MAX);
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
      5193776270265024512ULL, MEOS_PT_UINT8, 0.0, false);
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
  uint64 *zero_quadbin = trajectory_quadbins(traj, 0, &zero_count);
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
  free(zero_quadbin);
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
  uint64 *crossed = trajectory_quadbins(traj_across, 3, &ncrossed);
  uint64 *sampled = trajectory_quadbins(traj_sampled, 3, &nsampled);
  printf("trajectory_quadbins(linear trip, 3): %d cell(s), sampled: %d\n",
    ncrossed, nsampled);
  assert(ncrossed == 4);
  assert(nsampled == 4);
  free(crossed); free(sampled); free(traj_across); free(traj_sampled);

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
  Temporal *values = raster_value(traj_values, rast_values, 1);
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
  Temporal *along = raster_value(traj_linear, rast_values, 1);
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
  Temporal *visits = raster_value(traj_gap, rast_values, 1);
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
      bad_bands[i]);
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
  assert(raster_value(traj_3857, rast_values, 1) == NULL);
  assert(meos_errno() != 0);

  /* A null argument is rejected rather than dereferenced */
  meos_errno_reset();
  assert(raster_value(NULL, rast_values, 1) == NULL);
  assert(raster_value(traj_values, NULL, 1) == NULL);
  assert(raster_at_value(traj_values, rast_values, 1, NULL) == NULL);
  assert(raster_minus_value(traj_values, rast_values, 1, NULL) == NULL);
  assert(eraster_value(traj_values, rast_values, 1, NULL) == -1);
  assert(araster_value(traj_values, rast_values, 1, NULL) == -1);
  assert(meos_errno() != 0);

  free(vspan); free(traj_3857); free(traj_values); free(rast_values);

  meos_finalize();
  printf("raster_test: all assertions passed\n");
  return 0;
}
