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
 * @brief A program that tests the readers and the writers of a geometry and a
 * geography over every representation
 * @details Each reader is given the same point in every representation and
 * reads the representations its SQL names state, and only those: the text
 * and EWKT readers the WKT representation optionally prefixed with an SRID,
 * the binary readers the WKB and EWKB representations, the hexadecimal
 * readers their hex-encoded forms, the GeoJSON readers GeoJSON. A geography
 * reader refuses an SRID that is not a longitude and latitude system and
 * reads WGS84 where the value states none. Each writer writes the bytes of
 * the representation it names. The program prints a row per reader and
 * representation, and a row per writer, and exits with the number of rows
 * that differ from what they state.
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_io_test geo_io_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* The point (1 2) in every textual representation */
enum { WKT, EWKT4326, EWKT3857, HEXWKB, HEXEWKB4326, HEXEWKB3857, GEOJSON,
  GEOJSON3857, NFORMS };
static const char *FORMS[NFORMS][2] = {
  {"WKT", "Point(1 2)"},
  {"EWKT4326", "SRID=4326;Point(1 2)"},
  {"EWKT3857", "SRID=3857;Point(1 2)"},
  {"HexWKB", "0101000000000000000000F03F0000000000000040"},
  {"HexEWKB4326", "0101000020E6100000000000000000F03F0000000000000040"},
  {"HexEWKB3857", "0101000020110F0000000000000000F03F0000000000000040"},
  {"GeoJSON", "{\"type\":\"Point\",\"coordinates\":[1,2]}"},
  {"GeoJSON3857", "{\"type\":\"Point\",\"crs\":{\"type\":\"name\","
    "\"properties\":{\"name\":\"EPSG:3857\"}},\"coordinates\":[1,2]}"},
};

/* What a reader answers for a representation: the SRID of the value read,
 * or REFUSED */
#define REFUSED -1

static int failures = 0;

/**
 * @brief Return the bytes of a hexadecimal string
 */
static uint8_t *
hex_bytes(const char *hex, size_t *size)
{
  *size = strlen(hex) / 2;
  uint8_t *result = malloc(*size);
  for (size_t i = 0; i < *size; i++)
    sscanf(hex + 2 * i, "%2hhx", &result[i]);
  return result;
}

/**
 * @brief Print and check the row of a reader and a representation
 */
static void
row(const char *reader, int form, GSERIALIZED *gs, int srid, bool geodetic)
{
  int got_srid = REFUSED;
  bool got_geodetic = false;
  if (gs)
  {
    STBox *box = geo_to_stbox(gs);
    got_srid = geo_srid(gs);
    got_geodetic = stbox_isgeodetic(box);
    free(box); free(gs);
  }
  meos_errno_reset();
  bool ok = got_srid == srid && (srid == REFUSED || got_geodetic == geodetic);
  if (! ok)
    failures++;
  if (got_srid == REFUSED)
    printf("%-18s %-12s refused%s\n", reader, FORMS[form][0], ok ? "" :
      "  DIFFERS");
  else
    printf("%-18s %-12s read srid=%d %s%s\n", reader, FORMS[form][0],
      got_srid, got_geodetic ? "geography" : "geometry", ok ? "" :
      "  DIFFERS");
}

/**
 * @brief Print and check the row of a writer
 */
static void
written(const char *writer, const char *got, const char *expected)
{
  bool ok = got && strcmp(got, expected) == 0;
  if (! ok)
    failures++;
  printf("%-18s %s%s\n", writer, got ? got : "(null)", ok ? "" : "  DIFFERS");
}

/**
 * @brief Return the hexadecimal string of bytes
 */
static char *
bytes_hex(const uint8_t *bytes, size_t size)
{
  if (! bytes)
    return NULL;
  char *result = malloc(2 * size + 1);
  for (size_t i = 0; i < size; i++)
    sprintf(result + 2 * i, "%02X", bytes[i]);
  return result;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* The text and EWKT readers read the WKT representation optionally
   * prefixed with an SRID, and nothing else */
  for (int f = 0; f < NFORMS; f++)
  {
    const char *str = FORMS[f][1];
    bool text = f == WKT || f == EWKT4326 || f == EWKT3857;
    row("geom_from_ewkt", f, geom_from_ewkt(str),
      ! text ? REFUSED : f == WKT ? 0 : f == EWKT4326 ? 4326 : 3857, false);
    row("geog_from_ewkt", f, geog_from_ewkt(str),
      ! text || f == EWKT3857 ? REFUSED : 4326, true);
  }
  /* The hexadecimal readers read the hex-encoded WKB and EWKB, and nothing
   * else */
  for (int f = 0; f < NFORMS; f++)
  {
    const char *str = FORMS[f][1];
    bool hex = f == HEXWKB || f == HEXEWKB4326 || f == HEXEWKB3857;
    row("geom_from_hexewkb", f, geom_from_hexewkb(str),
      ! hex ? REFUSED : f == HEXWKB ? 0 : f == HEXEWKB4326 ? 4326 : 3857,
      false);
    row("geog_from_hexewkb", f, geog_from_hexewkb(str),
      ! hex || f == HEXEWKB3857 ? REFUSED : 4326, true);
  }
  /* The GeoJSON readers read GeoJSON, WGS84 where it names no system */
  for (int f = 0; f < NFORMS; f++)
  {
    const char *str = FORMS[f][1];
    bool json = f == GEOJSON || f == GEOJSON3857;
    row("geo_from_geojson", f, geo_from_geojson(str),
      ! json ? REFUSED : f == GEOJSON ? 4326 : 3857, false);
    row("geog_from_geojson", f, geog_from_geojson(str),
      ! json || f == GEOJSON3857 ? REFUSED : 4326, true);
  }
  /* The binary readers read the WKB and EWKB bytes */
  for (int f = HEXWKB; f <= HEXEWKB3857; f++)
  {
    size_t size;
    uint8_t *wkb = hex_bytes(FORMS[f][1], &size);
    row("geo_from_ewkb", f, geo_from_ewkb(wkb, size, 0),
      f == HEXWKB ? 0 : f == HEXEWKB4326 ? 4326 : 3857, false);
    row("geog_from_ewkb", f, geog_from_ewkb(wkb, size),
      f == HEXEWKB3857 ? REFUSED : 4326, true);
    free(wkb);
  }

  /* Each writer writes the representation it names, the WKB ones in the
   * little-endian order of the corpus */
  GSERIALIZED *geom = geom_from_ewkt(FORMS[EWKT4326][1]);
  GSERIALIZED *geog = geog_from_ewkt(FORMS[EWKT4326][1]);
  GSERIALIZED *values[2] = {geom, geog};
  const char *names[2] = {"geometry", "geography"};
  for (int i = 0; i < 2; i++)
  {
    size_t size;
    char *s;
    printf("%s\n", names[i]);
    s = geo_as_text(values[i], 15, &size);
    written("  geo_as_text", s, "POINT(1 2)"); free(s);
    s = geo_as_ewkt(values[i], 15, &size);
    written("  geo_as_ewkt", s, "SRID=4326;POINT(1 2)"); free(s);
    uint8_t *wkb = geo_as_wkb(values[i], "NDR", &size);
    s = bytes_hex(wkb, size);
    written("  geo_as_wkb", s, FORMS[HEXWKB][1]); free(s); free(wkb);
    wkb = geo_as_ewkb(values[i], "NDR", &size);
    s = bytes_hex(wkb, size);
    written("  geo_as_ewkb", s, FORMS[HEXEWKB4326][1]); free(s); free(wkb);
    s = geo_as_hexwkb(values[i], "NDR", &size);
    written("  geo_as_hexwkb", s, FORMS[HEXWKB][1]); free(s);
    s = geo_as_hexewkb(values[i], "NDR", &size);
    written("  geo_as_hexewkb", s, FORMS[HEXEWKB4326][1]); free(s);
    s = geo_as_geojson(values[i], 0, 9, NULL, &size);
    written("  geo_as_geojson", s, FORMS[GEOJSON][1]); free(s);
  }
  free(geom); free(geog);

  printf("%d row(s) differ\n", failures);
  meos_finalize();
  return failures;
}
