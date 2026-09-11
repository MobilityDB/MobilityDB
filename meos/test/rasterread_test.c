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
 * @brief A raster band whose pixels cannot be read raises an error rather
 * than reading as a band without values
 *
 * @details Two rasters are read along a trip over them. Each opens, and each
 * fails at the first pixel it is asked for:
 * - a PostGIS raster whose band is stored outside the database in a file
 *   that does not exist, read through the vendored raster core;
 * - a GDAL virtual raster whose band takes its pixels from a file that does
 *   not exist, read through GDAL.
 *
 * The values read along the trip, the restrictions and the ever and always
 * predicates must each raise an error: no value, never and always would each
 * state something about a band nobody read. The SQL suite pins the raster core
 * cases; the GDAL readers are reached from C, so this program holds them.
 *
 * @code
 * gcc -Wall -Werror=implicit-function-declaration -g -I/usr/local/include
 *   -o rasterread_test rasterread_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_raster.h>

/** Virtual raster the GDAL case reads, written beside the program */
#define VRT_PATH "rasterread_test.vrt"

/** Errors raised since the last check */
static int nerrors = 0;

/**
 * @brief Error handler counting the errors instead of ending the program
 */
static void
count_error(int errlevel, int errcode, const char *errmsg)
{
  (void) errlevel;
  (void) errcode;
  printf("  raised: %s\n", errmsg);
  nerrors++;
  return;
}

/**
 * @brief Return whether a call raised one error and answered the error
 * value, printing the verdict beside the name of the call
 */
static bool
raised(const char *what, bool errval)
{
  bool ok = nerrors == 1 && errval;
  printf("%-34s %s\n", what, ok ? "raises" : "ANSWERS WITHOUT RAISING");
  nerrors = 0;
  return ok;
}

/**
 * @brief Write a virtual raster of 3 x 3 unit pixels over (0, 0)-(3, 3)
 * whose band takes its pixels from a file that does not exist
 */
static bool
write_vrt(void)
{
  FILE *f = fopen(VRT_PATH, "w");
  if (! f)
    return false;
  fputs("<VRTDataset rasterXSize=\"3\" rasterYSize=\"3\">\n"
    "  <SRS>EPSG:4326</SRS>\n"
    "  <GeoTransform>0.0, 1.0, 0.0, 3.0, 0.0, -1.0</GeoTransform>\n"
    "  <VRTRasterBand dataType=\"Float32\" band=\"1\">\n"
    "    <SimpleSource>\n"
    "      <SourceFilename relativeToVRT=\"1\">rasterread_no_such_file.tif"
    "</SourceFilename>\n"
    "      <SourceBand>1</SourceBand>\n"
    "    </SimpleSource>\n"
    "  </VRTRasterBand>\n"
    "</VRTDataset>\n", f);
  fclose(f);
  return true;
}

int main(void)
{
  meos_initialize();
  meos_initialize_error_handler(&count_error);
  int failures = 0;

  /* The two rasters share one grid, 3 x 3 unit pixels over (0, 0)-(3, 3) in
   * EPSG:4326, read at its centre and along its middle row */
  Temporal *inst = tgeompoint_in("SRID=4326;{POINT(1.5 1.5)@2001-01-01}");
  Temporal *trip = tgeompoint_in("SRID=4326;[POINT(0.5 1.5)@2001-01-01, "
    "POINT(2.5 1.5)@2001-01-02]");
  Span *span = floatspan_in("[0, 100]");
  Temporal *v;

  /* The PostGIS raster: its band, of pixel type 32BF, is stored outside the
   * database in no_such_raster.tif */
  Raster *rast = raster_from_hexwkb(
    "0100000100000000000000f03f000000000000f0bf0000000000000000"
    "000000000000084000000000000000000000000000000000e6100000030003008a"
    "00000000006e6f5f737563685f7261737465722e74696600");
  nerrors = 0;
  v = raster_value(inst, rast, 1);
  failures += ! raised("raster core, value at an instant", v == NULL);
  free(v);
  v = raster_value(trip, rast, 1);
  failures += ! raised("raster core, value along a trip", v == NULL);
  free(v);
  v = raster_at_value(trip, rast, 1, span);
  failures += ! raised("raster core, at a value span", v == NULL);
  free(v);
  v = raster_minus_value(trip, rast, 1, span);
  failures += ! raised("raster core, minus a value span", v == NULL);
  free(v);
  failures += ! raised("raster core, ever",
    eraster_value(trip, rast, 1, span) == -1);
  failures += ! raised("raster core, always",
    araster_value(trip, rast, 1, span) == -1);

  /* The GDAL virtual raster */
  if (! write_vrt())
  {
    printf("FAILED: cannot write %s\n", VRT_PATH);
    meos_finalize();
    return 1;
  }
  nerrors = 0;
  v = raster_value_gdal(inst, VRT_PATH, 1);
  failures += ! raised("GDAL, value at an instant", v == NULL);
  free(v);
  v = raster_value_gdal(trip, VRT_PATH, 1);
  failures += ! raised("GDAL, value along a trip", v == NULL);
  free(v);
  v = raster_at_value_gdal(trip, VRT_PATH, 1, span);
  failures += ! raised("GDAL, at a value span", v == NULL);
  free(v);
  v = raster_minus_value_gdal(trip, VRT_PATH, 1, span);
  failures += ! raised("GDAL, minus a value span", v == NULL);
  free(v);
  failures += ! raised("GDAL, ever",
    eraster_value_gdal(trip, VRT_PATH, 1, span) == -1);
  failures += ! raised("GDAL, always",
    araster_value_gdal(trip, VRT_PATH, 1, span) == -1);
  remove(VRT_PATH);

  free(rast); free(span); free(trip); free(inst);
  if (failures > 0)
  {
    printf("FAILED: %d reads of a band whose pixels cannot be read answer "
      "without raising an error\n", failures);
    meos_finalize();
    return 1;
  }
  printf("every read of a band whose pixels cannot be read raises an error\n");
  meos_finalize();
  return 0;
}
