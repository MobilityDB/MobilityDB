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
 * @brief A program that tests the coordinate reference system the GeoJSON
 * and MF-JSON representations name for the SRID of a value
 * @details Where the caller names no system, #geo_as_geojson names the one
 * its options ask for as PostGIS function @p ST_AsGeoJSON does, and
 * #temporal_as_mfjson names the one of a spatial value of known SRID as the
 * PostgreSQL function @p asMFJSON does, the long name where its option asks
 * for it alone and the short one otherwise, writing the bounding box where it
 * asks for it, both reading the name from `spatial_ref_sys.csv`. An SRID absent from the file is an error, reached
 * under #meos_initialize_noexit_error_handler, the handler every language
 * binding installs. Reading back, #geo_from_geojson, #geom_in and
 * #tgeompoint_from_mfjson take the SRID of the system the value names, the
 * short name, the OGC URN or an OGC name of WGS 84, WGS 84 where a GeoJSON
 * value names none, and refuse a name in no such form.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o crs_name_test crs_name_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

#define SHORT_3857 "\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"EPSG:3857\"}}"
#define LONG_3857 \
  "\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"urn:ogc:def:crs:EPSG::3857\"}}"
#define SHORT_4326 "\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"EPSG:4326\"}}"

/**
 * @brief Return the GeoJSON representation of a geometry given in EWKT
 */
static char *
geojson(const char *ewkt, int option, const char *srs)
{
  size_t size_out;
  GSERIALIZED *gs = geom_in(ewkt, -1);
  assert(gs);
  char *result = geo_as_geojson(gs, option, 9, srs, &size_out);
  free(gs);
  return result;
}

/**
 * @brief Return the MF-JSON representation of a temporal value
 */
static char *
mfjson(Temporal *temp, int option)
{
  size_t size_out;
  assert(temp);
  char *result = temporal_as_mfjson(temp, option, 0, 6, NULL, &size_out);
  free(temp);
  return result;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();
  meos_initialize_timezone("UTC");

  /* The guess of the default option names every system but WGS 84 */
  char *s = geojson("SRID=3857;Point(1 1)", 8, NULL);
  assert(s && strstr(s, SHORT_3857));
  free(s);
  s = geojson("SRID=4326;Point(1 1)", 8, NULL);
  assert(s && ! strstr(s, "\"crs\""));
  free(s);

  /* The short and the long names */
  s = geojson("SRID=4326;Point(1 1)", 2, NULL);
  assert(s && strstr(s, SHORT_4326));
  free(s);
  s = geojson("SRID=3857;Point(1 1)", 4, NULL);
  assert(s && strstr(s, LONG_3857));
  free(s);

  /* No option and no SRID state no system, and a name given is stated */
  s = geojson("SRID=3857;Point(1 1)", 0, NULL);
  assert(s && ! strstr(s, "\"crs\""));
  free(s);
  s = geojson("Point(1 1)", 2, NULL);
  assert(s && ! strstr(s, "\"crs\""));
  free(s);
  s = geojson("SRID=3857;Point(1 1)", 2, "EPSG:2154");
  assert(s && strstr(s, "\"name\":\"EPSG:2154\"") && ! strstr(s, "3857"));
  free(s);

  /* A second call reads the name from the PROJ cache */
  s = geojson("SRID=3857;Point(1 1)", 8, NULL);
  assert(s && strstr(s, SHORT_3857));
  free(s);

  /* An SRID absent from spatial_ref_sys.csv is an error */
  s = geojson("SRID=123456;Point(1 1)", 8, NULL);
  assert(s == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* A temporal point of known SRID names its system, one of unknown SRID
   * and a temporal float name none */
  s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), 0);
  assert(s && strstr(s, "\"properties\":{\"name\":\"EPSG:3857\"}"));
  free(s);
  s = mfjson(tgeompoint_in("Point(1 1)@2001-01-01"), 0);
  assert(s && ! strstr(s, "\"crs\""));
  free(s);
  s = mfjson(tfloat_in("1.5@2001-01-01"), 0);
  assert(s && ! strstr(s, "\"crs\""));
  free(s);
  s = mfjson(tgeompoint_in("SRID=123456;Point(1 1)@2001-01-01"), 0);
  assert(s == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* The option asks for the long name alone with 4, the short name wins
   * with 2 and 4, and 1 writes the bounding box */
  s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), 4);
  assert(s && strstr(s,
    "\"properties\":{\"name\":\"urn:ogc:def:crs:EPSG::3857\"}"));
  free(s);
  s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), 6);
  assert(s && strstr(s, "\"properties\":{\"name\":\"EPSG:3857\"}"));
  free(s);
  s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), 1);
  assert(s && strstr(s, "\"bbox\""));
  free(s);
  s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), 0);
  assert(s && ! strstr(s, "\"bbox\""));
  free(s);

  /* A reader takes the SRID of the system a GeoJSON value names, the short
   * name, the OGC URN without and with a version, and an OGC name of WGS 84,
   * and WGS 84 where it names none */
  const char *names[][2] = {
    {"EPSG:3857", "3857"},
    {"urn:ogc:def:crs:EPSG::3857", "3857"},
    {"urn:ogc:def:crs:EPSG:9.9.1:3857", "3857"},
    {"urn:ogc:def:crs:OGC:1.3:CRS84", "4326"}};
  for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
  {
    char json[256];
    snprintf(json, sizeof(json), "{\"type\":\"Point\",\"coordinates\":[1,1],"
      "\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"%s\"}}}",
      names[i][0]);
    GSERIALIZED *gs = geo_from_geojson(json);
    assert(gs && geo_srid(gs) == atoi(names[i][1]));
    free(gs);
    gs = geom_in(json, -1);
    assert(gs && geo_srid(gs) == atoi(names[i][1]));
    free(gs);
  }
  GSERIALIZED *gs = geo_from_geojson("{\"type\":\"Point\",\"coordinates\":[1,1]}");
  assert(gs && geo_srid(gs) == 4326);
  free(gs);

  /* A system named in no form a reader knows is an error */
  gs = geo_from_geojson("{\"type\":\"Point\",\"coordinates\":[1,1],"
    "\"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"EPSG:x\"}}}");
  assert(gs == NULL && meos_errno() != 0);
  meos_errno_reset();

  /* The GeoJSON and MF-JSON of a value, the short and the long names, read
   * back with its SRID */
  s = geojson("SRID=3857;Point(1 1)", 4, NULL);
  gs = geo_from_geojson(s);
  assert(gs && geo_srid(gs) == 3857);
  free(gs); free(s);
  for (int option = 0; option <= 4; option += 4)
  {
    s = mfjson(tgeompoint_in("SRID=3857;Point(1 1)@2001-01-01"), option);
    Temporal *temp = tgeompoint_from_mfjson(s);
    assert(temp && tspatial_srid(temp) == 3857);
    free(temp); free(s);
  }

  meos_finalize();
  printf("crs_name_test: OK\n");
  return EXIT_SUCCESS;
}
