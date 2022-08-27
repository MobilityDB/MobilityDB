/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief A simple program that uses the MEOS library for input and output
 * GSERIALIZED values.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o meos_gserialized_inout meos_gserialized_inout.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>  /* for printf */

/* Include the MEOS API header */
#include "meos.h"

int main()
{
  /* Initialize MEOS */
  meos_initialize();

  /* Input geometries in WKT format */
  char *point_wkt = "POINT(1 1)";
  char *linestring_wkt = "Linestring(1 1,2 2,1 1)";
  char *polygon_wkt = "Polygon((1 1,1 2,2 2,2 1,1 1))";

  /* Read WKT into geometries */
  GSERIALIZED *point = gserialized_in(point_wkt, -1);
  GSERIALIZED *linestring = gserialized_in(linestring_wkt, -1);
  GSERIALIZED *polygon = gserialized_in(polygon_wkt, -1);

  /* Convert geometries to WKT */
  char *point_text = gserialized_as_text(point, 6);
  char *linestring_text = gserialized_as_text(linestring, 6);
  char *polygon_text = gserialized_as_text(polygon, 6);

  /* Revert generated WKT strings into geometries */
  GSERIALIZED *point1 = gserialized_in(point_text, -1);
  GSERIALIZED *linestring1 = gserialized_in(linestring_text, -1);
  GSERIALIZED *polygon1 = gserialized_in(polygon_text, -1);

  /* Ensure that the reverted types are equal to the original ones */
  if (! gserialized_same(point, point1))
    printf("ERROR: Distinct input and output geometries in WKT\n%s\n%s",
      point_wkt, point_text);
  if (! gserialized_same(linestring, linestring1))
    printf("ERROR: Distinct input and output geometries in WKT\n%s\n%s",
      linestring_wkt, linestring_text);
  if (! gserialized_same(polygon, polygon1))
    printf("ERROR: Distinct input and output geometries in WKT\n%s\n%s",
      polygon_wkt, polygon_text);

  /* Convert geometries to GeoJSON */
  char *point_geojson = gserialized_as_geojson(point, 1, 6, NULL);
  char *linestring_geojson = gserialized_as_geojson(linestring, 1, 6, NULL);
  char *polygon_geojson = gserialized_as_geojson(polygon, 1, 6, NULL);

  /* Revert generated GeoJSON strings into geometries */
  GSERIALIZED *point2 = gserialized_from_geojson(point_geojson);
  GSERIALIZED *linestring2 = gserialized_from_geojson(linestring_geojson);
  GSERIALIZED *polygon2 = gserialized_from_geojson(polygon_geojson);

  /* Ensure that the reverted types are equal to the original ones */
  if (! gserialized_same(point, point2))
    printf("ERROR: Distinct input and output geometries in GeoJSON\n%s\n%s",
      point_wkt, point_geojson);
  if (! gserialized_same(linestring, linestring2))
    printf("ERROR: Distinct input and output geometries in GeoJSON\n%s\n%s",
      linestring_wkt, linestring_geojson);
  if (! gserialized_same(polygon, polygon2))
    printf("ERROR: Distinct input and output geometries in GeoJSON\n%s\n%s",
      polygon_wkt, polygon_geojson);

  /* Convert geometries to HexEWKB */
  char *point_hexwkb = gserialized_as_hexewkb(point, "XDR");
  char *linestring_hexwkb = gserialized_as_hexwkb(linestring, "XDR");
  char *polygon_hexwkb = gserialized_as_hexwkb(polygon, "XDR");

  /* Revert generated GeoJSON strings into geometries */
  GSERIALIZED *point3 = gserialized_from_hexewkb(point_hexwkb);
  GSERIALIZED *linestring3 = gserialized_from_hexewkb(linestring_hexwkb);
  GSERIALIZED *polygon3 = gserialized_from_hexewkb(polygon_hexwkb);

  /* Ensure that the reverted types are equal to the original ones */
  if (! gserialized_same(point, point3))
    printf("ERROR: Distinct input and output geometries in HexEWKB\n%s\n%s",
      point_wkt, point_text);
  if (! gserialized_same(linestring, linestring3))
    printf("ERROR: Distinct input and output geometries in HexEWKB\n%s\n%s",
      linestring_wkt, linestring_text);
  if (! gserialized_same(polygon, polygon3))
    printf("ERROR: Distinct input and output geometries in HexEWKB\n%s\n%s",
      polygon_wkt, polygon_text);

  /* Read WKT into temporal point object */
  printf("\n"
    "--------\n"
    "| Point |\n"
    "--------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "Text:\n"
    "-----\n%s\n\n"
    "GeoJSON:\n"
    "--------\n%s\n"
    "HexWKB:\n"
    "-------\n%s\n", point_wkt, point_text, point_geojson, point_hexwkb);
  printf("\n"
    "-------------\n"
    "| Linestring |\n"
    "-------------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "Text:\n"
    "-----\n%s\n\n"
    "GeoJSON:\n"
    "--------\n%s\n"
    "HexWKB:\n"
    "-------\n%s\n", linestring_wkt, linestring_text, linestring_geojson, linestring_hexwkb);
  printf("\n"
    "----------\n"
    "| Polygon |\n"
    "----------\n\n"
    "WKT:\n"
    "----\n%s\n\n"
    "Text:\n"
    "-----\n%s\n\n"
    "GeoJSON:\n"
    "--------\n%s\n"
    "HexWKB:\n"
    "-------\n%s\n", polygon_wkt, polygon_text, polygon_geojson, polygon_hexwkb);


  /* Clean up allocated objects */
  free(point); free(point1); free(point2); free(point3);
  free(point_text); free(point_geojson); free(point_hexwkb);
  free(linestring); free(linestring1); free(linestring2); free(linestring3);
  free(linestring_text); free(linestring_geojson); free(linestring_hexwkb);
  free(polygon); free(polygon1); free(polygon2); free(polygon3); 
  free(polygon_text); free(polygon_geojson); free(polygon_hexwkb);

  /* Finalize MEOS */
  meos_finish();

  /* Return */
  return 0;
}
