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
 * @brief Test that the MEOS build reads a spatial reference system from the
 * spatial_ref_sys.csv file #meos_set_spatial_ref_sys_csv names last
 * @details A program that writes two files byte for byte in the current
 * directory, as ways_index_test.c writes its networks, and removes them at
 * the end. They state SRID 4326 alike, SRID 3857 under the authorities EPSG
 * and MYAUTH, and SRID 990001, under an authority PROJ does not know so that
 * its proj4text defines it, as a Mercator projection and as longitude and
 * latitude. Under each file in turn, the name #geo_as_geojson states for
 * SRID 3857, as crs_name_test.c reads it, and the point of SRID 4326 at
 * (10, 10) transformed to SRID 990001 by #geo_transform, are the ones of
 * that file, though the systems of the first one were read before the
 * second one is named.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o srs_file_test srs_file_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* The two spatial_ref_sys.csv files the program writes */
static const char *SRS_A = "srs_file_test_a.csv";
static const char *SRS_B = "srs_file_test_b.csv";

#define SRS_HEADER "auth_name,auth_srid,proj4text,srtext\n"
#define SRS_4326 "EPSG,4326,+proj=longlat +datum=WGS84 +no_defs,\n"
#define SRS_MERC \
  "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 " \
  "+y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs"
static const char *TEXT_A = SRS_HEADER SRS_4326
  "EPSG,3857," SRS_MERC ",\n"
  "TEST,990001," SRS_MERC ",\n";
static const char *TEXT_B = SRS_HEADER SRS_4326
  "MYAUTH,3857," SRS_MERC ",\n"
  "TEST,990001,+proj=longlat +datum=WGS84 +no_defs,\n";

/* The x of the point (10, 10) of SRID 4326 in the spherical Mercator
 * projection of radius 6378137: the radius times 10 degrees in radians */
#define MERC_X_10 (6378137.0 * 10.0 * M_PI / 180.0)

static int failures = 0;

static bool
write_file(const char *path, const char *text)
{
  /* Binary mode writes the line ends of the text as they are, as
   * ways_index_test.c writes its files */
  FILE *file = fopen(path, "wb");
  if (! file)
    return false;
  fputs(text, file);
  fclose(file);
  return true;
}

/* Check that the GeoJSON of a point of SRID 3857 names the system name, as
 * #geojson of crs_name_test.c reads it with the short name option */
static void
check_name(const char *what, const char *name)
{
  size_t size_out;
  GSERIALIZED *gs = geom_in("SRID=3857;Point(1 1)", -1);
  char *s = gs ? geo_as_geojson(gs, 2, 9, NULL, &size_out) : NULL;
  bool ok = s && strstr(s, name);
  if (! ok)
    failures++;
  printf("%s %s: %s\n", ok ? "ok  " : "FAIL", what, s ? s : "(none)");
  free(s);
  free(gs);
}

/* Check the x of the point (10, 10) of SRID 4326 transformed to SRID
 * 990001, read from its text #geo_as_text writes, to within a micrometer */
static void
check_x(const char *what, double want)
{
  GSERIALIZED *gs = geom_in("SRID=4326;Point(10 10)", -1);
  GSERIALIZED *res = gs ? geo_transform(gs, 990001) : NULL;
  size_t size_out;
  char *text = res ? geo_as_text(res, 9, &size_out) : NULL;
  double x = NAN;
  if (! text || sscanf(text, "POINT(%lf", &x) != 1)
    x = NAN;
  bool ok = ! isnan(x) && fabs(x - want) < 1e-6;
  if (! ok)
    failures++;
  printf("%s %s: %s\n", ok ? "ok  " : "FAIL", what, text ? text : "(none)");
  free(text);
  free(res);
  free(gs);
}

int
main(void)
{
  if (! write_file(SRS_A, TEXT_A) || ! write_file(SRS_B, TEXT_B))
    return 1;

  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_set_spatial_ref_sys_csv(SRS_A);
  check_name("SRID 3857 of the first file", "\"name\":\"EPSG:3857\"");
  check_x("SRID 990001 of the first file, a Mercator projection", MERC_X_10);
  meos_set_spatial_ref_sys_csv(SRS_B);
  check_name("SRID 3857 of the second file, named after the first",
    "\"name\":\"MYAUTH:3857\"");
  check_x("SRID 990001 of the second file, longitude and latitude", 10.0);
  meos_set_spatial_ref_sys_csv(SRS_A);
  check_name("SRID 3857 of the first file, named again",
    "\"name\":\"EPSG:3857\"");
  meos_finalize();

  remove(SRS_A);
  remove(SRS_B);
  printf("%d failure(s)\n", failures);
  return failures ? 1 : 0;
}
