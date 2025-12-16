/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief A simple program that tests the funtions for the temporal geometry
 * types in MEOS, that is, geometry, geography, stbox, tgeometry, tgeography,
 * tgeompoint, tgeogpoint.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_test geo_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <meos.h>
#include <meos_geo.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */

  int32 int32_in1 = 1;
  int32 int32_in2 = 2;
  double float8_in1 = 1;
  double float8array1[3];
  double float8array2[3];
  double float8array3[3];
  char *interv1_in = "3 days";
  Interval *interv1 = interval_in(interv1_in, -1);
  char *interv1_out = interval_out(interv1);
  char *interv2_in = "3 hours";
  Interval *interv2 = interval_in(interv2_in, -1);
  char *interv2_out = interval_out(interv2);
  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstz2_in = "2001-01-03";
  TimestampTz tstz2 = timestamptz_in(tstz2_in, -1);
  char *tstz2_out = timestamptz_out(tstz2);
  TimestampTz tstzarray[2];
  size_t size, geom_size_wkb, geog_size_wkb, stbox_size_wkb;
  
  text *text_null = text_in("NULL");

  Span *fspan1 = floatspan_in("[1,3]");
  char *fspan1_out = floatspan_out(fspan1, 6);
  
  char *tstzset1_in = "{2001-01-01, 2001-01-03}";
  Set *tstzset1 = tstzset_in(tstzset1_in);
  char *tstzset1_out = tstzset_out(tstzset1);

  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);

  char *tstzspanset1_in = "{[2001-01-01, 2001-01-03],[2001-01-04, 2001-01-05]}";
  SpanSet *tstzspanset1 = tstzspanset_in(tstzspanset1_in);
  char *tstzspanset1_out = tstzspanset_out(tstzspanset1);

  char *geomset1_in = "SRID=5676;{Point(1 1), \"Linestring(2 2,3 3)\"}";
  Set *geomset1 = geomset_in(geomset1_in);
  char *geomset1_out = spatialset_as_text(geomset1, 6);

  char *geogset1_in = "SRID=5676;{Point(1 1), \"Linestring(2 2,3 3)\"}";

  GBOX *gbox1 = gbox_make(true, true, false, 1, 1, 1, 1, 3, 3, 3, 3);
  char *gbox1_out = gbox_out(gbox1, 6);

  BOX3D *box3d1 = box3d_make(1, 5, 1, 5, 1, 5, 5676);
  char *box3d1_out = box3d_out(box3d1, 6);

  char *geom1_in = "SRID=5676;Point(1 1)";
  GSERIALIZED *geom1 = geom_in(geom1_in, -1);
  char *geom1_out = geo_as_ewkt(geom1, 6);
  char *geom1_hexwkb = geo_as_hexewkb(geom1, NULL);
  uint8_t *geom1_wkb = geo_as_ewkb(geom1, NULL, &geom_size_wkb);
  char *geom1_geojson = geo_as_geojson(geom1, 0, 6, "EPSG:4326");
  char *geom2_in = "SRID=5676;Linestring(1 1,2 2)";
  GSERIALIZED *geom2 = geom_in(geom2_in, -1);
  char *geom2_out = geo_as_ewkt(geom2, 6);
  char *geom3_in = "SRID=5676;Polygon((0 0,0 5,5 5,5 0,0 0))";
  GSERIALIZED *geom3 = geom_in(geom3_in, -1);
  char *geom3_out = geo_as_ewkt(geom3, 6);

  char *geompt1_in = "SRID=5676;Point(1 1)";
  GSERIALIZED *geompt1 = geom_in(geompt1_in, -1);
  char *geompt1_out = geo_as_ewkt(geompt1, 6);
  char *geompt2_in = "SRID=5676;Point(2 2)";
  GSERIALIZED *geompt2 = geom_in(geompt2_in, -1);
  char *geompt2_out = geo_as_ewkt(geompt2, 6);

  char *point_wkt_in = "Point(1 1)";
  GSERIALIZED *geompt_wkt;
  GSERIALIZED *geogpt_wkt;
 
  GSERIALIZED *geomarray[2];

  GSERIALIZED *geom3d1 = geom_in("SRID=5676;Linestring(1 1 1,2 2 2)", -1);
  char *geom3d1_out = geo_as_ewkt(geom3d1, 6);
  GSERIALIZED *geom3d2 = geom_in("SRID=5676;Linestring(3 3 3,1 1 1)", -1);
  char *geom3d2_out = geo_as_ewkt(geom3d2, 6);

  char *geog1_in = "SRID=4326;Linestring(1 1,2 2)";
  GSERIALIZED *geog1 = geog_in(geog1_in, -1);
  char *geog1_out = geo_as_ewkt(geog1, 6);
  char *geog1_hexwkb = geo_as_hexewkb(geog1, NULL);
  uint8_t *geog1_wkb = geo_as_ewkb(geog1, NULL, &geog_size_wkb);
  char *geog2_in = "SRID=4326;Polygon((1 1,1 2,2 2,2 1,1 1))";
  GSERIALIZED *geog2 = geog_in(geog2_in, -1);
  char *geog2_out = geo_as_ewkt(geog2, 6);

  GSERIALIZED *line1 = geom_in("SRID=5676;Linestring(1 1,5 5)", -1);
  char *line1_out = geo_as_ewkt(line1, 6);

  char *tfloat1_in = "[1@2001-01-01, 3@2001-01-03]";
  Temporal *tfloat1 = tfloat_in(tfloat1_in);
  char *tfloat1_out = tfloat_out(tfloat1, 6);

  char *stbox1_in = "SRID=5676;STBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])";
  STBox *stbox1 = stbox_in(stbox1_in);
  char *stbox1_out = stbox_out(stbox1, 6);
  char *stbox1_hexwkb = stbox_as_hexwkb(stbox1, 1, &size);
  uint8_t *stbox1_wkb = stbox_as_wkb(stbox1, 1, &stbox_size_wkb);
  char *stbox2_in = "SRID=5676;STBOX XT(((2,2),(4,4)),[2001-01-02, 2001-01-04])";
  STBox *stbox2 = stbox_in(stbox2_in);
  char *stbox2_out = stbox_out(stbox2, 6);

  char *stbox3d1_in = "SRID=5676;STBOX ZT(((1,1,1),(3,3,3)),[2001-01-01, 2001-01-03])";
  STBox *stbox3d1 = stbox_in(stbox3d1_in);
  char *stbox3d1_out = stbox_out(stbox3d1, 6);
  char *stbox3d2_in = "SRID=5676;STBOX ZT(((2,2,2),(4,4,4)),[2001-01-02, 2001-01-04])";
  STBox *stbox3d2 = stbox_in(stbox3d2_in);
  char *stbox3d2_out = stbox_out(stbox3d2, 6);

  STBox *stbox_vector;

  /* Temporal points */
  
  char *tgeompt1_in = "SRID=5676;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]";
  Temporal *tgeompt1 = tgeompoint_in(tgeompt1_in);
  char *tgeompt1_out = tspatial_as_ewkt(tgeompt1, 6);
  char *tgeompt1_mfjson = temporal_as_mfjson(tgeompt1, true, 1, 6, NULL);
  char *tgeompt2_in = "SRID=5676;[Point(3 3)@2001-01-01, Point(1 1)@2001-01-03]";
  Temporal *tgeompt2 = tgeompoint_in(tgeompt2_in);
  char *tgeompt2_out = tspatial_as_ewkt(tgeompt2, 6);

  char *tgeompt3d1_in = "SRID=5676;[Point(1 1 1)@2001-01-01, Point(3 3 3)@2001-01-03]";
  Temporal *tgeompt3d1 = tgeompoint_in(tgeompt3d1_in);
  char *tgeompt3d1_out = tspatial_as_ewkt(tgeompt3d1, 6);
  char *tgeompt3d2_in = "SRID=5676;[Point(2 2 2)@2001-01-02, Point(4 4 4)@2001-01-04]";
  Temporal *tgeompt3d2 = tgeompoint_in(tgeompt3d2_in);
  char *tgeompt3d2_out = tspatial_as_ewkt(tgeompt3d2, 6);

  char *tgeompt1_step_in = "SRID=5676,Interp=Step;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]";
  Temporal *tgeompt1_step = tgeompoint_in(tgeompt1_step_in);
  char *tgeompt1_step_out = tspatial_as_ewkt(tgeompt1_step, 6);

  char *tgeogpt1_step_in = "SRID=4326,Interp=Step;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]";
  Temporal *tgeogpt1_step = tgeogpoint_in(tgeogpt1_step_in);
  char *tgeogpt1_step_out = tspatial_as_ewkt(tgeogpt1_step, 6);

  char *tgeogpt1_in = "SRID=4326;[Point(3 3)@2001-01-01, Point(1 1)@2001-01-03]";
  Temporal *tgeogpt1 = tgeogpoint_in(tgeogpt1_in);
  char *tgeogpt1_out = tspatial_as_ewkt(tgeogpt1, 6);
  char *tgeogpt1_mfjson = temporal_as_mfjson(tgeogpt1, true, 1, 6, NULL);
  char *tgeogpt2_in = "SRID=4326;[Point(3 3)@2001-01-01, Point(1 1)@2001-01-03]";
  Temporal *tgeogpt2 = tgeogpoint_in(tgeogpt2_in);
  char *tgeogpt2_out = tspatial_as_ewkt(tgeogpt2, 6);

  /* Temporal geos */

  char *tgeom1_in = "SRID=5676;[Point(1 1)@2001-01-01, \"Linestring(2 2,3 3)\"@2001-01-03]";
  Temporal *tgeom1 = tgeometry_in(tgeom1_in);
  char *tgeom1_out = tspatial_as_ewkt(tgeom1, 6);
  char *tgeom1_mfjson = temporal_as_mfjson(tgeom1, true, 1, 6, NULL);
  char *tgeom2_in = "SRID=5676;[\"Linestring(1 1,2 2)\"@2001-01-01, \"Polygon((2 2,2 3,3 3,3 2,2 2))\"@2001-01-03]";
  Temporal *tgeom2 = tgeometry_in(tgeom2_in);
  char *tgeom2_out = tspatial_as_ewkt(tgeom2, 6);

  char *tgeog1_in = "SRID=4326;[Point(1 1)@2001-01-01, \"Linestring(1 1,2 2)\"@2001-01-03]";
  Temporal *tgeog1 = tgeography_in(tgeog1_in);
  char *tgeog1_out = tspatial_as_ewkt(tgeog1, 6);
  char *tgeog1_mfjson = temporal_as_mfjson(tgeog1, true, 1, 6, NULL);
  char *tgeog2_in = "SRID=4326;[Point(3 3)@2001-01-01, \"Linestring(2 2,3 3)\"@2001-01-03]";
  Temporal *tgeog2 = tgeography_in(tgeog2_in);
  char *tgeog2_out = tspatial_as_ewkt(tgeog2, 6);

  char *tgeom1_pt_in = "SRID=5676;[Point(1 1)@2001-01-01, Point(5 5)@2001-01-03]";
  Temporal *tgeom1_pt = tgeometry_in(tgeom1_pt_in);
  char *tgeom1_pt_out = tspatial_as_ewkt(tgeom1_pt, 6);

  char *tgeog1_pt_in = "SRID=4326;[Point(1 1)@2001-01-01, Point(5 5)@2001-01-03]";
  Temporal *tgeog1_pt = tgeography_in(tgeog1_pt_in);
  char *tgeog1_pt_out = tspatial_as_ewkt(tgeog1_pt, 6);

  char *tpoint_wkt_in = "[Point(1 1)@2001-01-01, Point(5 5)@2001-01-05]";
  
  /* Create the result types for the functions of the API */

  bool bool_result;
  int32_t int32_result;
  int64 *int64array_result;
  double float8_result;
  char *char_result;
  char *char1_result;
  char *char2_result;
  int32 count;

  uint8_t *binchar_result;

  TimestampTz tstz_result;
  TimestampTz *tstzarray_result;
  Span *tstzspan_result;

  GSERIALIZED *geom_result;
  GSERIALIZED *geog_result;
  GBOX *gbox_result;
  BOX3D *box3d_result;

  int32_t *int32array_result;
  uint32_t *uint32array_result;
  STBox *stboxarray_result;
  GSERIALIZED **geomarray_result;
  Temporal **tgeomptarray_result;

  Set *geomset_result ;
  Set *geogset_result;
  STBox *stbox_result;
  Temporal *tbool_result;
  Temporal *tfloat_result;
  Temporal *tgeom_result;
  Temporal *tgeog_result;
  Temporal *tgeompt_result;
  Temporal *tgeogpt_result;

  /* For aggregates */
  SkipList *sklist;

  /* For similarity */
  Match *matches;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Geometry types *\n");
  printf("****************************************************************\n");

  /* Input and output functions */

  /* uint8_t *geo_as_ewkb(const GSERIALIZED *gs, const char *endian, size_t *size); */
  binchar_result = geo_as_ewkb(geom1, "XDR", &size);
  printf("geo_as_ewkb(%s, \"XDR\", %ld): ", tfloat1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* char *geo_as_ewkt(const GSERIALIZED *gs, int precision); */
  char_result = geo_as_ewkt(geom1, 0.1);
  printf("geo_as_ewkt(%s): %s\n", geom1_out, char_result);
  free(char_result);

  /* char *geo_as_geojson(const GSERIALIZED *gs, int option, int precision, const char *srs); */
  char_result = geo_as_geojson(geom1, 6, 5, "EPSG:4326");
  printf("geo_as_geojson(%s): %s\n", geom1_out, char_result);
  free(char_result);

  /* char *geo_as_hexewkb(const GSERIALIZED *gs, const char *endian); */
  char_result = geo_as_hexewkb(geom1, "XDR");
  printf("geo_as_hexewkb(%s, \"XDR\"): %s\n", geom1_out, char_result);
  free(char_result);

  /* char *geo_as_text(const GSERIALIZED *gs, int precision); */
  char_result = geo_as_text(geom1, 0.1);
  printf("geo_as_text(%s): %s\n", geom1_out, char_result);
  free(char_result);

  /* GSERIALIZED *geo_from_ewkb(const uint8_t *wkb, size_t wkb_size, int32_t srid); */
  geom_result = geo_from_ewkb(geom1_wkb, geom_size_wkb, 5676);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_from_ewkb(");
  fwrite(geom1_wkb, geom_size_wkb, 1, stdout);
  printf(", %ld): %s\n", size, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geo_from_geojson(const char *geojson); */
  geom_result = geo_from_geojson(geom1_geojson);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_from_geojson(%s): %s\n", geom1_geojson, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geo_from_text(const char *wkt, int32_t srid); */
  geom_result = geo_from_text(point_wkt_in, 5676);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_from_text(%s): %s\n", point_wkt_in, char_result);
  free(geom_result); free(char_result);

  /* char *geo_out(const GSERIALIZED *gs); */
  char_result = geo_out(geom1);
  printf("geo_out(%s): %s\n", geom1_out, char_result);
  free(char_result);

  /* GSERIALIZED *geog_from_binary(const char *wkb_bytea); */
  geom_result = geo_from_ewkb(geog1_wkb, geog_size_wkb, 4326);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geog_from_binary(%s, %ld): %s\n", geog1_wkb, geog_size_wkb, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geog_from_hexewkb(const char *wkt); */
  geom_result = geog_from_hexewkb(geog1_hexwkb);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geog_from_hexewkb(%s): %s\n", geog1_hexwkb, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geog_in(const char *str, int32 typmod); */
  geog_result = geog_in(geog1_in, -1);
  char_result = geo_as_ewkt(geog_result, 6);
  printf("geog_in(%s): %s\n", geog1_in, char_result);
  free(geog_result); free(char_result);

  /* GSERIALIZED *geom_from_hexewkb(const char *wkt); */
  geom_result = geom_from_hexewkb(geom1_hexwkb);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_from_hexewkb(%s): %s\n", geom1_hexwkb, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_in(const char *str, int32 typmod); */
  geom_result = geom_in(geom1_in, -1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_in(%s): %s\n", geom1_in, char_result);
  free(geom_result); free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* GSERIALIZED *geo_copy(const GSERIALIZED *g); */
  geom_result = geo_copy(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_copy(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geogpoint_make2d(int32_t srid, double x, double y); */
  geom_result = geogpoint_make2d(4326, 1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geogpoint_make2d(4326, 1, 1): %s\n", char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geogpoint_make3dz(int32_t srid, double x, double y, double z); */
  geom_result = geogpoint_make3dz(4326, 1, 1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geogpoint_make3dz(4326, 1, 1, 1): %s\n", char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geompoint_make2d(int32_t srid, double x, double y); */
  geom_result = geompoint_make2d(4326, 1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geompoint_make2d(4326, 1, 1): %s\n", char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geompoint_make3dz(int32_t srid, double x, double y, double z); */
  geom_result = geompoint_make3dz(4326, 1, 1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geompoint_make3dz(4326, 1, 1): %s\n", char_result);
  free(geom_result); free(char_result);

  /* Conversion functions */

  /* GSERIALIZED *geom_to_geog(const GSERIALIZED *geom); */
  geompt_wkt = geom_in(point_wkt_in, -1);
  geog_result = geom_to_geog(geompt_wkt);
  char_result = geo_as_ewkt(geog_result, 6);
  printf("geom_to_geog(%s): %s\n", point_wkt_in, char_result);
  free(geompt_wkt); free(geog_result); free(char_result);

  /* GSERIALIZED *geog_to_geom(const GSERIALIZED *geog); */
  geogpt_wkt = geog_in(point_wkt_in, -1);
  geom_result = geog_to_geom(geogpt_wkt);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geog_to_geom(%s): %s\n", point_wkt_in, char_result);
  free(geogpt_wkt); free(geom_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* bool geo_is_empty(const GSERIALIZED *g); */
  bool_result = geo_is_empty(geom1);
  printf("geo_is_empty(%s): %c\n", geom1_out, bool_result ? 't' : 'n');

  /* bool geo_is_unitary(const GSERIALIZED *gs); */
  bool_result = geo_is_unitary(geom1);
  printf("geo_is_unitary(%s): %c\n", geom1_out, bool_result ? 't' : 'n');

  /* const char *geo_typename(int type); */
  char_result = strdup(geo_typename(1));
  printf("geo_typename(1): %s\n", char_result);
  free(char_result);

  /* double geog_area(const GSERIALIZED *gs, bool use_spheroid); */
  float8_result = geog_area(geog1, true);
  printf("geog_area(%s, true): %lf\n", geog1_out, float8_result);

  /* GSERIALIZED *geog_centroid(const GSERIALIZED *g, bool use_spheroid); */
  geog_result = geog_centroid(geog1, true);
  char_result = geo_as_ewkt(geog_result, 6);
  printf("geog_centroid(%s, true): %s\n", geog1_out, char_result);
  free(geog_result); free(char_result);

  /* double geog_length(const GSERIALIZED *gs, bool use_spheroid); */
  float8_result = geog_length(geog1, true);
  printf("geog_length(%s, true): %lf\n", geog1_out, float8_result);

  /* double geog_perimeter(const GSERIALIZED *gs, bool use_spheroid); */
  float8_result = geog_perimeter(geog1, true);
  printf("geog_perimeter(%s, true): %lf\n", geog1_out, float8_result);

  /* bool geom_azimuth(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result); */
  bool_result = geom_azimuth(geompt1, geompt2, &float8_result);
  printf("geom_azimuth(%s, %s, %lf): %c\n", geompt1_out, geompt2_out, float8_result, bool_result ? 't' : 'n');

  /* double geom_length(const GSERIALIZED *gs); */
  float8_result = geom_length(geom1);
  printf("geom_length(%s): %lf\n", geom1_out, float8_result);

  /* double geom_perimeter(const GSERIALIZED *gs); */
  float8_result = geom_perimeter(geom1);
  printf("geom_perimeter(%s): %lf\n", geom1_out, float8_result);

  /* int line_numpoints(const GSERIALIZED *gs); */
  int32_result = line_numpoints(line1);
  printf("line_numpoints(%s): %d\n", line1_out, int32_result);

  /* GSERIALIZED *line_point_n(const GSERIALIZED *geom, int n); */
  geom_result = line_point_n(line1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("line_point_n(%s): %s\n", line1_out, char_result);
  free(geom_result); free(char_result);

  /* Transformation functions */
  printf("****************************************************************\n");

  /* GSERIALIZED *geo_reverse(const GSERIALIZED *gs); */
  geom_result = geo_reverse(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_reverse(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geo_round(const GSERIALIZED *gs, int maxdd); */
  geom_result = geo_round(geom1, 6);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_round(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* Spatial reference system functions */
  printf("****************************************************************\n");

  /* GSERIALIZED *geo_set_srid(const GSERIALIZED *gs, int32_t srid); */
  geom_result = geo_set_srid(geom1, 4326);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_set_srid(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* int32_t geo_srid(const GSERIALIZED *gs); */
  int32_result = geo_srid(geom1);
  printf("geo_srid(%s): %d\n", geom1_out, int32_result);

  /* GSERIALIZED *geo_transform(GSERIALIZED *geom, int32_t srid_to); */
  geom_result = geo_transform(geom1, 3857);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_transform(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geo_transform_pipeline(const GSERIALIZED *gs, char *pipeline, int32_t srid_to, bool is_forward); */
  geom_result = geo_transform_pipeline(geom1, "urn:ogc:def:coordinateOperation:EPSG::1671", 1671, true);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_transform_pipeline(%s, \"urn:ogc:def:coordinateOperation:EPSG::1671\", 1671): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* Spatial processing functions */
  printf("****************************************************************\n");

  /* geom_result = geo_collect_garray(GSERIALIZED **gsarr, int count); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  geom_result = geo_collect_garray(geomarray, 2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_collect_garray({%s, %s}): %s\n", geom1_out, geom2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geo_makeline_garray(GSERIALIZED **gsarr, int count); */
  geomarray[0] = geompt1;
  geomarray[1] = geompt2;
  geom_result = geo_makeline_garray(geomarray, 2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_makeline_garray({%s, %s}): %s\n", geompt1_out, geompt2_out, char_result);
  free(geom_result); free(char_result);

  /* int geo_num_points(const GSERIALIZED *gs); */
  int32_result = geo_num_points(geom1);
  printf("geo_num_points(%s): %d\n", geom1_out, int32_result);

  /* int geo_num_geos(const GSERIALIZED *gs); */
  int32_result = geo_num_geos(geom1);
  printf("geo_num_geos(%s): %d\n", geom1_out, int32_result);

  /* GSERIALIZED *geo_geo_n(const GSERIALIZED *geom, int n); */
  geom_result = geo_geo_n(geom1, 1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_geo_n(%s, 1): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED **geo_pointarr(const GSERIALIZED *gs, int *count); */
  geomarray_result = geo_pointarr(geom1, &count);
  printf("geo_pointarr(%s, %d): {", geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(geomarray_result[i]);
    free(char_result);
  }
  free(geomarray_result);

  /* GSERIALIZED *geo_points(const GSERIALIZED *gs); */
  geom_result = geo_points(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geo_points(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_array_union(GSERIALIZED **gsarr, int count); */
  geomarray[0] = geompt1;
  geomarray[1] = geompt2;
  geom_result = geom_array_union(geomarray, 2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_array_union({%s, %s}, 2): %s\n", geompt1_out, geompt2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_boundary(const GSERIALIZED *gs); */
  geom_result = geom_boundary(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_boundary(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_buffer(const GSERIALIZED *gs, double size, char *params); */
  geom_result = geom_buffer(geom1, 1, "quad_segs=8");
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_buffer(%s, \"quad_segs=8\"): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_centroid(const GSERIALIZED *gs); */
  geom_result = geom_centroid(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_centroid(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_convex_hull(const GSERIALIZED *gs); */
  geom_result = geom_convex_hull(geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_centroid(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_difference2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  geom_result = geom_difference2d(geom1, geom2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_difference2d(%s, %s): %s\n", geom1_out, geom2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_intersection2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  geom_result = geom_intersection2d(geom1, geom2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_intersection2d(%s, %s): %s\n", geom1_out, geom2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_intersection2d_coll(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  geom_result = geom_intersection2d_coll(geom1, geom2);
  char_result = geom_result ? geo_as_ewkt(geom_result, 6) : text_out(text_null);
  printf("geom_intersection2d_coll(%s, %s): %s\n", geom1_out, geom2_out, char_result);
  if (geom_result)
    free(geom_result);
  free(char_result);

  /* GSERIALIZED *geom_min_bounding_radius(const GSERIALIZED *geom, double *radius); */
  geom_result = geom_min_bounding_radius(geom1, &float8_result);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_min_bounding_radius(%s, %lf): %s\n", geom1_out, float8_result, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_shortestline2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  geom_result = geom_shortestline2d(geom1, geom2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_shortestline2d(%s, %s): %s\n", geom1_out, geom2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_shortestline3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  geom_result = geom_shortestline3d(geom1, geom2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_shortestline3d(%s, %s): %s\n", geom1_out, geom2_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geom_unary_union(const GSERIALIZED *gs, double prec); */
  geom_result = geom_unary_union(geom1, 0.1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geom_unary_union(%s, 0.1): %s\n", geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *line_interpolate_point(const GSERIALIZED *gs, double distance_fraction, bool repeat); */
  geom_result = line_interpolate_point(line1, 0.2, true);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("line_interpolate_point(%s): %s\n", line1_out, char_result);
  free(geom_result); free(char_result);

  /* double line_locate_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  float8_result = line_locate_point(line1, geompt1);
  printf("line_locate_point(%s, %s): %lf\n", line1_out, geompt1_out, float8_result);

  /* GSERIALIZED *line_substring(const GSERIALIZED *gs, double from, double to); */
  geom_result = line_substring(line1, 0.3, 0.7);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("line_substring(%s): %s\n", line1_out, char_result);
  free(geom_result); free(char_result);

  /* Spatial relationship functions */
  printf("****************************************************************\n");

  /* bool geog_dwithin(const GSERIALIZED *g1, const GSERIALIZED *g2, double tolerance, bool use_spheroid); */
  bool_result = geog_dwithin(geog1, geog2, float8_in1, true);
  printf("geog_dwithin(%s, %s): %c\n", geog1_out, geog2_out, bool_result ? 't' : 'n');

  /* bool geog_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2, bool use_spheroid); */
  bool_result = geog_intersects(geog1, geog2, true);
  printf("geog_intersects(%s, %s): %c\n", geog1_out, geog2_out, bool_result ? 't' : 'n');

  /* bool geom_contains(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_contains(geom1, geom2);
  printf("geom_contains(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_covers(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_covers(geom1, geom2);
  printf("geom_covers(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_disjoint2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_disjoint2d(geom1, geom2);
  printf("geom_disjoint2d(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_dwithin2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance); */
  bool_result = geom_dwithin2d(geom1, geom2, 0.1);
  printf("geom_dwithin2d(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_dwithin3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance); */
  bool_result = geom_dwithin3d(geom1, geom2, 0.1);
  printf("geom_dwithin3d(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_intersects2d(geom1, geom2);
  printf("geom_intersects2d(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_intersects3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_intersects3d(geom3d1, geom3d2);
  printf("geom_intersects3d(%s, %s): %c\n", geom3d1_out, geom3d2_out, bool_result ? 't' : 'n');

  /* bool geom_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2, char *patt); */
  bool_result = geom_relate_pattern(geom1, geom2, "TTTFFFFFF");
  printf("geom_relate_pattern(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* bool geom_touches(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geom_touches(geom1, geom2);
  printf("geom_touches(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /* Bounding box functions */
  printf("****************************************************************\n");

  /* STBox *geo_stboxes(const GSERIALIZED *gs, int *count); */
  stboxarray_result = geo_stboxes(line1, &count);
  printf("geo_stboxes(%s, %d): {", line1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *geo_split_each_n_stboxes(const GSERIALIZED *gs, int elem_count, int *count); */
  stboxarray_result = geo_split_each_n_stboxes(line1, int32_in2, &count);
  printf("geo_split_each_n_stboxes(%s, %d): {", line1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *geo_split_n_stboxes(const GSERIALIZED *gs, int box_count, int *count); */
  stboxarray_result = geo_split_n_stboxes(line1, 2, &count);
  printf("geo_split_n_stboxes(%s, 1, %d): {", line1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* Distance functions */
  printf("****************************************************************\n");

  /* double geog_distance(const GSERIALIZED *g1, const GSERIALIZED *g2); */
  float8_result = geog_distance(geog1, geog2);
  printf("geog_distance(%s, %s): %lf\n", geog1_out, geog2_out, float8_result);

  /* double geom_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  float8_result = geom_distance2d(geom1, geom2);
  printf("geom_distance2d(%s, %s): %lf\n", geom1_out, geom2_out, float8_result);

  /* double geom_distance3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  float8_result = geom_distance3d(geom3d1, geom3d2);
  printf("geom_distance3d(%s, %s): %lf\n", geom3d1_out, geom3d2_out, float8_result);

  /* Comparison functions */
  printf("****************************************************************\n");

  /* int geo_equals(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  int32_result = geo_equals(geom1, geom2);
  printf("geo_equals(%s, %s): %d\n", geom1_out, geom2_out, int32_result);

  /* bool geo_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2); */
  bool_result = geo_same(geom1, geom2);
  printf("geo_same(%s, %s): %c\n", geom1_out, geom2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Functions for spatial sets
   *****************************************************************************/

  /* Input and output functions */
  printf("****************************************************************\n");

  /* Set *geogset_in(const char *str); */
  geogset_result = geogset_in(geogset1_in);
  char_result = spatialset_as_text(geogset_result, 6);
  printf("geogset_in(%s): %s\n", geogset1_in, char_result);
  free(geogset_result); free(char_result);

  /* Set *geomset_in(const char *str); */
  geomset_result = geomset_in(geomset1_in);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("geomset_in(%s): %s\n", geomset1_in, char_result);
  free(geomset_result); free(char_result);

  /* char *spatialset_as_text(const Set *set, int maxdd); */
  char_result = spatialset_as_text(geomset1, 6);
  printf("spatialset_as_text(%s): %s\n", geomset1_out, char_result);
  free(char_result);

  /* char *spatialset_as_ewkt(const Set *set, int maxdd); */
  char_result = spatialset_as_ewkt(geomset1, 6);
  printf("spatialset_as_ewkt(%s): %s\n", geomset1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Set *geoset_make(GSERIALIZED **values, int count); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  geomset_result = geoset_make(geomarray, 2);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("geoset_make({%s, %s}): %s\n", geom1_out, geom2_out, char_result);
  free(geomset_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* Set *geo_to_set(const GSERIALIZED *gs); */
  geomset_result = geo_to_set(geom1);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("geo_to_set(%s): %s\n", geom1_out, char_result);
  free(geomset_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* GSERIALIZED *geoset_end_value(const Set *s); */
  geom_result = geoset_end_value(geomset1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geoset_end_value(%s): %s\n", geomset1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *geoset_start_value(const Set *s); */
  geom_result = geoset_start_value(geomset1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geoset_start_value(%s): %s\n", geomset1_out, char_result);
  free(geom_result); free(char_result);

  /* bool geoset_value_n(const Set *s, int n, GSERIALIZED **result); */
  bool_result = geoset_value_n(geomset1, 1, &geom_result);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("geoset_value_n(%s, 1, %s): %c\n", geomset1_out, char_result, bool_result ? 't' : 'n');
  free(geom_result); free(char_result);

  /* GSERIALIZED **geoset_values(const Set *s); */
  geomarray_result = geoset_values(geomset1);
  printf("geoset_values(%s): {", geom1_out);
  for (int i = 0; i < geomset1->count; i++)
  {
    char_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s", char_result);
    if (i < geomset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(geomarray_result[i]);
    free(char_result);
  }
  free(geomarray_result);

  /* Set operations */

  /* bool contained_geo_set(const GSERIALIZED *gs, const Set *s); */
  bool_result = contained_geo_set(geom1, geomset1);
  printf("contained_geo_set(%s, %s): %c\n", geom1_out, geomset1_out, bool_result ? 't' : 'n');

  /* bool contains_set_geo(const Set *s, GSERIALIZED *gs); */
  bool_result = contains_set_geo(geomset1, geom1);
  printf("contains_set_geo(%s, %s): %c\n", geomset1_out, geom1_out, bool_result ? 't' : 'n');

  /* Set *geo_union_transfn(Set *state, const GSERIALIZED *gs); */
  geomset_result = geo_union_transfn(NULL, geom1);
  geomset_result = geo_union_transfn(geomset_result, geom2);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("geo_union({%s, %s}): %s\n", geom1_out, geom2_out, char_result);
  free(geomset_result); free(char_result);

  /* Set *intersection_geo_set(const GSERIALIZED *gs, const Set *s); */
  geomset_result = intersection_geo_set(geom1, geomset1);
  char_result = geomset_result ? spatialset_as_text(geomset_result, 6) : text_out(text_null);
  printf("intersection_geo_set(%s, %s): %s\n", geom1_out, geomset1_out, char_result);
  if (geomset_result)
    free(geomset_result);
  free(char_result);

  /* Set *intersection_set_geo(const Set *s, const GSERIALIZED *gs); */
  geomset_result = intersection_set_geo(geomset1, geom1);
  char_result = geomset_result ? spatialset_as_text(geomset_result, 6) : text_out(text_null);
  printf("intersection_set_geo(%s, %s): %s\n", geomset1_out, geom1_out, char_result);
  if (geomset_result)
    free(geomset_result);
  free(char_result);

  /* Set *minus_geo_set(const GSERIALIZED *gs, const Set *s); */
  geomset_result = minus_geo_set(geom1, geomset1);
  char_result = geomset_result ? spatialset_as_text(geomset_result, 6) : text_out(text_null);
  printf("minus_geo_set(%s, %s): %s\n", geom1_out, geomset1_out, char_result);
  if (geomset_result)
    free(geomset_result);
  free(char_result);

  /* Set *minus_set_geo(const Set *s, const GSERIALIZED *gs); */
  geomset_result = minus_set_geo(geomset1, geom1);
  char_result = geomset_result ? spatialset_as_text(geomset_result, 6) : text_out(text_null);
  printf("minus_set_geo(%s, %s): %s\n", geomset1_out, geom1_out, char_result);
  if (geomset_result)
    free(geomset_result);
  free(char_result);

  /* Set *union_geo_set(const GSERIALIZED *gs, const Set *s); */
  geomset_result = union_geo_set(geom1, geomset1);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("union_geo_set(%s, %s): %s\n", geom1_out, geomset1_out, char_result);
  free(geomset_result); free(char_result);

  /* Set *union_set_geo(const Set *s, const GSERIALIZED *gs); */
  geomset_result = union_set_geo(geomset1, geom1);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("union_set_geo(%s, %s): %s\n", geomset1_out, geom1_out, char_result);
  free(geomset_result); free(char_result);

  /* SRID functions */
  printf("****************************************************************\n");

  /* Set *spatialset_set_srid(const Set *s, int32_t srid); */
  geomset_result = spatialset_set_srid(geomset1, 4326);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("spatialset_set_srid(%s): %s\n", geomset1_out, char_result);
  free(geomset_result); free(char_result);

  /* int32_t spatialset_srid(const Set *s); */
  int32_result = spatialset_srid(geomset1);
  printf("spatialset_srid(%s): %d\n", geomset1_out, int32_result);

  /* Set *spatialset_transform(const Set *s, int32_t srid); */
  geomset_result = spatialset_transform(geomset1, 4326);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("spatialset_transform(%s, 4326): %s\n", geomset1_out, char_result);
  free(geomset_result); free(char_result);

  /* Set *spatialset_transform_pipeline(const Set *s, const char *pipelinestr, int32_t srid, bool is_forward); */
  geomset_result = spatialset_transform_pipeline(geomset1, "urn:ogc:def:coordinateOperation:EPSG::1671", 1671, true);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("spatialset_transform_pipeline(%s, \"urn:ogc:def:coordinateOperation:EPSG::1671\", 1671, true): %s\n", geomset1_out, char_result);
  free(geomset_result); free(char_result);

  /*****************************************************************************
   * Functions for spatiotemporal boxes
   *****************************************************************************/

  /* Input/output functions */
  printf("****************************************************************\n");

  /* char *stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size); */
  char_result = stbox_as_hexwkb(stbox1, 1, &size);
  printf("stbox_as_hexwkb(%s): %s\n", stbox1_out, char_result);
  free(char_result);

  /* uint8_t *stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out); */
  binchar_result = stbox_as_wkb(stbox1, 1, &size);
  printf("tbox_as_wkb(%s, 1, %ld): ", stbox1_out, size);
  fwrite(binchar_result, size, 1, stdout);
  printf("\n");
  free(binchar_result);

  /* STBox *stbox_from_hexwkb(const char *hexwkb); */
  stbox_result = stbox_from_hexwkb(stbox1_hexwkb);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_from_hexwkb(%s): %s\n", stbox1_hexwkb, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_from_wkb(const uint8_t *wkb, size_t size); */
  stbox_result = stbox_from_wkb(stbox1_wkb, stbox_size_wkb);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_from_wkb(%s, %ld): %s\n", stbox1_wkb, stbox_size_wkb, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_in(const char *str); */
  stbox_result = stbox_in(stbox1_in);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_in(%s): %s\n", stbox1_in, char_result);
  free(stbox_result); free(char_result);

  /* char *stbox_out(const STBox *box, int maxdd); */
  char_result = stbox_out(stbox1, 6);
  printf("stbox_out(%s): %s\n", stbox1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* STBox *geo_timestamptz_to_stbox(const GSERIALIZED *gs, TimestampTz t); */
  stbox_result = geo_timestamptz_to_stbox(geom1, tstz1);
  char_result = stbox_out(stbox_result, 6);
  printf("geo_timestamptz_to_stbox(%s, %s): %s\n", geom1_out, tstz1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *geo_tstzspan_to_stbox(const GSERIALIZED *gs, const Span *s); */
  stbox_result = geo_tstzspan_to_stbox(geom1, tstzspan1);
  char_result = stbox_out(stbox_result, 6);
  printf("geo_tstzspan_to_stbox(%s, %s): %s\n", geom1_out, tstzspan1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_copy(const STBox *box); */
  stbox_result = stbox_copy(stbox1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_copy(%s): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *s); */
  stbox_result = stbox_make(true, true, false, 4326, 1, 3, 1, 3, 1, 3, tstzspan1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_make(true, true, false, 4326, 1, 3, 1, 3, 1, 3, %s): %s\n", tstzspan1_out, char_result);
  free(stbox_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* STBox *geo_to_stbox(const GSERIALIZED *gs); */
  stbox_result = geo_to_stbox(geom1);
  char_result = stbox_out(stbox_result, 6);
  printf("geo_to_stbox(%s): %s\n", geom1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *spatialset_to_stbox(const Set *s); */
  stbox_result = spatialset_to_stbox(geomset1);
  char_result = stbox_out(stbox_result, 6);
  printf("spatialset_to_stbox(%s): %s\n", geomset1_out, char_result);
  free(stbox_result); free(char_result);

  /* BOX3D *stbox_to_box3d(const STBox *box); */
  box3d_result = stbox_to_box3d(stbox1);
  char_result = box3d_out(box3d_result, 6);
  printf("stbox_to_box3d(%s): %s\n", stbox1_out, char_result);
  free(box3d_result); free(char_result);

  /* GBOX *stbox_to_gbox(const STBox *box); */
  gbox_result = stbox_to_gbox(stbox1);
  char_result = gbox_out(gbox_result, 6);
  printf("stbox_to_gbox(%s): %s\n", stbox1_out, char_result);
  free(gbox_result); free(char_result);

  /* GSERIALIZED *stbox_to_geo(const STBox *box); */
  geom_result = stbox_to_geo(stbox1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("stbox_to_geo(%s): %s\n", stbox1_out, char_result);
  free(geom_result); free(char_result);

  /* Span *stbox_to_tstzspan(const STBox *box); */
  tstzspan_result = stbox_to_tstzspan(stbox1);
  char_result = tstzspan_out(tstzspan_result);
  printf("stbox_to_tstzspan(%s): %s\n", stbox1_out, char_result);
  free(tstzspan_result); free(char_result);

  /* STBox *timestamptz_to_stbox(TimestampTz t); */
  stbox_result = timestamptz_to_stbox(tstz1);
  char_result = stbox_out(stbox_result, 6);
  printf("timestamptz_to_stbox(%s): %s\n", tstz1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *tstzset_to_stbox(const Set *s); */
  stbox_result = tstzset_to_stbox(tstzset1);
  char_result = stbox_out(stbox_result, 6);
  printf("tstzset_to_stbox(%s): %s\n", tstzset1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *tstzspan_to_stbox(const Span *s); */
  stbox_result = tstzspan_to_stbox(tstzspan1);
  char_result = stbox_out(stbox_result, 6);
  printf("tstzspan_to_stbox(%s): %s\n", tstzspan1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *tstzspanset_to_stbox(const SpanSet *ss); */
  stbox_result = tstzspanset_to_stbox(tstzspanset1);
  char_result = stbox_out(stbox_result, 6);
  printf("tstzspanset_to_stbox(%s): %s\n", tstzspanset1_out, char_result);
  free(stbox_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* double stbox_area(const STBox *box, bool spheroid); */
  float8_result = stbox_area(stbox1, true);
  printf("stbox_area(%s, true): %lf\n", stbox1_out, float8_result);

  /* bool stbox_hast(const STBox *box); */
  bool_result = stbox_hast(stbox1);
  printf("stbox_hast(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* bool stbox_hasx(const STBox *box); */
  bool_result = stbox_hasx(stbox1);
  printf("stbox_hasx(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* bool stbox_hasz(const STBox *box); */
  bool_result = stbox_hasz(stbox1);
  printf("stbox_hasz(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* bool stbox_isgeodetic(const STBox *box); */
  bool_result = stbox_isgeodetic(stbox1);
  printf("stbox_isgeodetic(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* double stbox_perimeter(const STBox *box, bool spheroid); */
  float8_result = stbox_perimeter(stbox1, true);
  printf("stbox_perimeter(%s, true): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* bool stbox_tmax(const STBox *box, TimestampTz *result); */
  bool_result = stbox_tmax(stbox1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("stbox_tmax(%s, %s): %c\n", stbox1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* bool stbox_tmax_inc(const STBox *box, bool *result); */
  bool_result = stbox_tmax_inc(stbox1, &bool_result);
  printf("stbox_tmax_inc(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* bool stbox_tmin(const STBox *box, TimestampTz *result); */
  bool_result = stbox_tmin(stbox1, &tstz_result);
  char_result = timestamptz_out(tstz_result);
  printf("stbox_tmin(%s, %s): %c\n", stbox1_out, char_result, bool_result ? 't' : 'n');
  free(char_result);

  /* bool stbox_tmin_inc(const STBox *box, bool *result); */
  bool_result = stbox_tmin_inc(stbox1, &bool_result);
  printf("stbox_tmin_inc(%s): %c\n", stbox1_out, bool_result ? 't' : 'n');

  /* double stbox_volume(const STBox *box); */
  float8_result = stbox_volume(stbox3d1);
  printf("stbox_volume(%s): %lf\n", stbox3d1_out, float8_result);

  /* bool stbox_xmax(const STBox *box, double *result); */
  bool_result = stbox_xmax(stbox1, &float8_result);
  printf("stbox_xmax(%s, %lf): %c\n", stbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool stbox_xmin(const STBox *box, double *result); */
  bool_result = stbox_xmin(stbox1, &float8_result);
  printf("stbox_xmin(%s, %lf): %c\n", stbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool stbox_ymax(const STBox *box, double *result); */
  bool_result = stbox_ymax(stbox1, &float8_result);
  printf("stbox_ymax(%s, %lf): %c\n", stbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool stbox_ymin(const STBox *box, double *result); */
  bool_result = stbox_ymin(stbox1, &float8_result);
  printf("stbox_ymin(%s, %lf): %c\n", stbox1_out, float8_result, bool_result ? 't' : 'n');

  /* bool stbox_zmax(const STBox *box, double *result); */
  bool_result = stbox_zmax(stbox3d1, &float8_result);
  printf("stbox_zmax(%s, %lf): %c\n", stbox3d1_out, float8_result, bool_result ? 't' : 'n');

  /* bool stbox_zmin(const STBox *box, double *result); */
  bool_result = stbox_zmin(stbox3d1, &float8_result);
  printf("stbox_zmin(%s, %lf): %c\n", stbox3d1_out, float8_result, bool_result ? 't' : 'n');

  /* Transformation functions */
  printf("****************************************************************\n");

  /* STBox *stbox_expand_space(const STBox *box, double d); */
  stbox_result = stbox_expand_space(stbox1, float8_in1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_expand_space(%s, %lf): %s\n", stbox1_out, float8_in1, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_expand_time(const STBox *box, const Interval *interv); */
  stbox_result = stbox_expand_time(stbox1, interv1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_expand_time(%s, %s): %s\n", stbox1_out, interv1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_get_space(const STBox *box); */
  stbox_result = stbox_get_space(stbox1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_get_space(%s): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_quad_split(const STBox *box, int *count); */
  stbox_result = stbox_quad_split(stbox1, &count);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_quad_split(%s): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_round(const STBox *box, int maxdd); */
  stbox_result = stbox_round(stbox1, 6);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_round(%s): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_shift_scale_time(const STBox *box, const Interval *shift, const Interval *duration); */
  stbox_result = stbox_shift_scale_time(stbox1, interv1, interv2);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_shift_scale_time(%s, %s, %s): %s\n", stbox1_out, interv1_out, interv2_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stboxarr_round(const STBox *boxarr, int count, int maxdd); */
  stbox_vector = malloc(sizeof(STBox) * 2);
  stbox_vector[0] = *stbox1;
  stbox_vector[1] = *stbox2;
  stbox_result = stboxarr_round((const STBox *) stbox_vector, 2, 6);
  char_result = stbox_out(stbox_result, 6);
  printf("stboxarr_round({%s, %s}): %s\n", stbox1_out, stbox2_out, char_result);
  free(stbox_vector); free(stbox_result); free(char_result);

  /* SRID functions */
  printf("****************************************************************\n");

  /* STBox *stbox_set_srid(const STBox *box, int32_t srid); */
  stbox_result = stbox_set_srid(stbox1, 4326);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_set_srid(%s, 4326): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* int32_t stbox_srid(const STBox *box); */
  int32_result = stbox_srid(stbox1);
  printf("stbox_srid(%s): %d\n", stbox1_out, int32_result);

  /* STBox *stbox_transform(const STBox *box, int32_t srid); */
  stbox_result = stbox_transform(stbox1, 4326);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_transform(%s): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_transform_pipeline(const STBox *box, const char *pipelinestr, int32_t srid, bool is_forward); */
  stbox_result = stbox_transform_pipeline(stbox1, "urn:ogc:def:coordinateOperation:EPSG::1671", 1671, true);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_transform_pipeline(%s, \"urn:ogc:def:coordinateOperation:EPSG::1671\", 1671): %s\n", stbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* Topological functions */
  printf("****************************************************************\n");

  /* bool adjacent_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = adjacent_stbox_stbox(stbox1, stbox2);
  printf("adjacent_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool contained_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = contained_stbox_stbox(stbox1, stbox2);
  printf("contained_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool contains_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = contains_stbox_stbox(stbox1, stbox2);
  printf("contains_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overlaps_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overlaps_stbox_stbox(stbox1, stbox2);
  printf("overlaps_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool same_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = same_stbox_stbox(stbox1, stbox2);
  printf("same_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* Position functions */
  printf("****************************************************************\n");

  /* bool above_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = above_stbox_stbox(stbox1, stbox2);
  printf("above_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool after_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = after_stbox_stbox(stbox1, stbox2);
  printf("after_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool back_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = back_stbox_stbox(stbox3d1, stbox3d2);
  printf("back_stbox_stbox(%s, %s): %c\n", stbox3d1_out, stbox3d2_out, bool_result ? 't' : 'n');

  /* bool before_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = before_stbox_stbox(stbox1, stbox2);
  printf("before_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool below_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = below_stbox_stbox(stbox1, stbox2);
  printf("below_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool front_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = front_stbox_stbox(stbox3d1, stbox3d2);
  printf("front_stbox_stbox(%s, %s): %c\n", stbox3d1_out, stbox3d2_out, bool_result ? 't' : 'n');

  /* bool left_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = left_stbox_stbox(stbox1, stbox2);
  printf("left_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overabove_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overabove_stbox_stbox(stbox1, stbox2);
  printf("overabove_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overafter_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overafter_stbox_stbox(stbox1, stbox2);
  printf("overafter_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overback_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overback_stbox_stbox(stbox3d1, stbox3d2);
  printf("overback_stbox_stbox(%s, %s): %c\n", stbox3d1_out, stbox3d2_out, bool_result ? 't' : 'n');

  /* bool overbefore_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overbefore_stbox_stbox(stbox1, stbox2);
  printf("overbefore_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overbelow_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overbelow_stbox_stbox(stbox1, stbox2);
  printf("overbelow_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overfront_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overfront_stbox_stbox(stbox3d1, stbox3d2);
  printf("overfront_stbox_stbox(%s, %s): %c\n", stbox3d1_out, stbox3d2_out, bool_result ? 't' : 'n');

  /* bool overleft_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overleft_stbox_stbox(stbox1, stbox2);
  printf("overleft_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool overright_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = overright_stbox_stbox(stbox1, stbox2);
  printf("overright_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool right_stbox_stbox(const STBox *box1, const STBox *box2); */
  bool_result = right_stbox_stbox(stbox1, stbox2);
  printf("right_stbox_stbox(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* Set functions */
  printf("****************************************************************\n");

  /* STBox *union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict); */
  stbox_result = union_stbox_stbox(stbox1, stbox2, true);
  char_result = stbox_out(stbox_result, 6);
  printf("union_stbox_stbox(%s, %s): %s\n", stbox1_out, stbox2_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *intersection_stbox_stbox(const STBox *box1, const STBox *box2); */
  stbox_result = intersection_stbox_stbox(stbox1, stbox2);
  char_result = stbox_out(stbox_result, 6);
  printf("intersection_stbox_stbox(%s, %s): %s\n", stbox1_out, stbox2_out, char_result);
  free(stbox_result); free(char_result);

  /* Comparisons */
  printf("****************************************************************\n");

  /* int stbox_cmp(const STBox *box1, const STBox *box2); */
  int32_result = stbox_cmp(stbox1, stbox2);
  printf("stbox_cmp(%s, %s): %d\n", stbox1_out, stbox2_out, int32_result);

  /* bool stbox_eq(const STBox *box1, const STBox *box2); */
  bool_result = stbox_eq(stbox1, stbox2);
  printf("stbox_eq(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool stbox_ge(const STBox *box1, const STBox *box2); */
  bool_result = stbox_ge(stbox1, stbox2);
  printf("stbox_ge(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool stbox_gt(const STBox *box1, const STBox *box2); */
  bool_result = stbox_gt(stbox1, stbox2);
  printf("stbox_gt(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool stbox_le(const STBox *box1, const STBox *box2); */
  bool_result = stbox_le(stbox1, stbox2);
  printf("stbox_le(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool stbox_lt(const STBox *box1, const STBox *box2); */
  bool_result = stbox_lt(stbox1, stbox2);
  printf("stbox_lt(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /* bool stbox_ne(const STBox *box1, const STBox *box2); */
  bool_result = stbox_ne(stbox1, stbox2);
  printf("stbox_ne(%s, %s): %c\n", stbox1_out, stbox2_out, bool_result ? 't' : 'n');

  /*****************************************************************************
   * Functions for temporal geometries/geographies
   *****************************************************************************/

  /* Input and output functions */
  printf("****************************************************************\n");

  /* char *tspatial_as_text(const Temporal *temp, int maxdd); */
  char_result = tspatial_as_text(tgeom1, 6);
  printf("tspatial_as_text(%s): %s\n", tgeom1_out, char_result);
  free(char_result);

  /* Temporal *tgeogpoint_from_mfjson(const char *str); */
  tgeogpt_result = tgeogpoint_from_mfjson(tgeogpt1_mfjson);
  char_result = tspatial_as_ewkt(tgeogpt_result, 6);
  printf("tgeogpoint_from_mfjson(%s): %s\n", tgeogpt1_mfjson, char_result);
  free(tgeogpt_result); free(char_result);

  /* Temporal *tgeogpoint_in(const char *str); */
  tgeogpt_result = tgeogpoint_in(tgeogpt1_in);
  char_result = tspatial_as_ewkt(tgeogpt_result, 6);
  printf("tgeogpoint_in(%s): %s\n", tgeogpt1_in, char_result);
  free(tgeogpt_result); free(char_result);

  /* Temporal *tgeography_from_mfjson(const char *mfjson); */
  tgeog_result = tgeography_from_mfjson(tgeog1_mfjson);
  char_result = tspatial_as_ewkt(tgeog_result, 6);
  printf("tgeography_from_mfjson(%s): %s\n", tgeog1_mfjson, char_result);
  free(tgeog_result); free(char_result);

  /* Temporal *tgeography_in(const char *str); */
  tgeog_result = tgeography_in(tgeog1_in);
  char_result = tspatial_as_ewkt(tgeog_result, 6);
  printf("tgeography_in(%s): %s\n", tgeog1_in, char_result);
  free(tgeog_result); free(char_result);

  /* Temporal *tgeometry_from_mfjson(const char *str); */
  tgeom_result = tgeometry_from_mfjson(tgeom1_mfjson);
  char_result = tspatial_as_ewkt(tgeom_result, 6);
  printf("tgeometry_from_mfjson(%s): %s\n", tgeom1_mfjson, char_result);
  free(tgeom_result); free(char_result);

  /* Temporal *tgeometry_in(const char *str); */
  tgeom_result = tgeometry_in(tgeom1_in);
  char_result = tspatial_as_ewkt(tgeom_result, 6);
  printf("tgeometry_in(%s): %s\n", tgeom1_in, char_result);
  free(tgeom_result); free(char_result);

  /* Temporal *tgeompoint_from_mfjson(const char *str); */
  tgeompt_result = tgeompoint_from_mfjson(tgeompt1_mfjson);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeompoint_from_mfjson(%s): %s\n", tgeompt1_mfjson, char_result);
  free(tgeompt_result); free(char_result);

  /* Temporal *tgeompoint_in(const char *str); */
  tgeompt_result = tgeompoint_in(tgeompt1_in);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeompoint_in(%s): %s\n", tgeompt1_in, char_result);
  free(tgeompt_result); free(char_result);

  /* char *tspatial_as_ewkt(const Temporal *temp, int maxdd); */
  char_result = tspatial_as_ewkt(tgeom1, 6);
  printf("tspatial_as_ewkt(%s): %s\n", tgeom1_out, char_result);
  free(char_result);

  /* char *tspatial_as_text(const Temporal *temp, int maxdd); */
  char_result = tspatial_as_text(tgeom1, 6);
  printf("tspatial_as_text(%s): %s\n", tgeom1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Temporal *tgeo_from_base_temp(const GSERIALIZED *gs, const Temporal *temp); */
  tgeompt_result = tgeo_from_base_temp(geompt1, tgeom2);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeo_from_base_temp(%s, %s): %s\n", geompt1_out, tgeom2_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TInstant *tgeoinst_make(const GSERIALIZED *gs, TimestampTz t); */
  tgeompt_result = (Temporal *) tgeoinst_make(geompt1, tstz1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeoinst_make(%s, %s): %s\n", geompt1_out, tstz1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequence *tgeoseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s); */
  tgeompt_result = (Temporal *) tgeoseq_from_base_tstzset(geompt1, tstzset1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeoseq_from_base_tstzset(%s, %s): %s\n", geompt1_out, tstzset1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequence *tgeoseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s, interpType interp); */
  tgeompt_result = (Temporal *) tgeoseq_from_base_tstzspan(geompt1, tstzspan1, STEP);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeoseq_from_base_tstzspan(%s, %s, STEP): %s\n", geompt1_out, tstzspan1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequenceSet *tgeoseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss, interpType interp); */
  tgeompt_result = (Temporal *) tgeoseqset_from_base_tstzspanset(geompt1, tstzspanset1, STEP);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeoseqset_from_base_tstzspanset(%s, %s, STEP): %s\n", geompt1_out, tstzspanset1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Temporal *tpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp); */
  tgeompt_result = tpoint_from_base_temp(geompt1, tgeom1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpoint_from_base_temp(%s, %s): %s\n", geompt1_out, tgeom1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TInstant *tpointinst_make(const GSERIALIZED *gs, TimestampTz t); */
  tgeompt_result = (Temporal *) tpointinst_make(geompt1, tstz1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpointinst_make(%s, %s): %s\n", geompt1_out, tstz1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequence *tpointseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s); */
  tgeompt_result = (Temporal *) tpointseq_from_base_tstzset(geompt1, tstzset1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpointseq_from_base_tstzset(%s, %s): %s\n", geompt1_out, tstzset1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequence *tpointseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s, interpType interp); */
  tgeompt_result = (Temporal *) tpointseq_from_base_tstzspan(geompt1, tstzspan1, LINEAR);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpointseq_from_base_tstzspan(%s, %s, LINEAR): %s\n", geompt1_out, tstzspan1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords, const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize); */
  float8array1[0] = float8array2[0] = float8array3[0] = 1;
  float8array1[1] = float8array2[1] = float8array3[1] = 1;
  tstzarray[0] = tstz1;
  tstzarray[1] = tstz2;
  tgeompt_result = (Temporal *) tpointseq_make_coords(float8array1, float8array2, float8array3, tstzarray, 2, 4326, true, true, true, LINEAR, true);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpointseq_make_coords({1,1,1},{1,1,1},{1,1,1},{%s,%s},2,4326,true,true,true,LINEAR,true): %s\n", tstz1_out, tstz2_out,char_result);
  free(tgeompt_result); free(char_result);

  /* TSequenceSet *tpointseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss, interpType interp); */
  tgeompt_result = (Temporal *) tpointseqset_from_base_tstzspanset(geom1, tstzspanset1, LINEAR);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpointseqset_from_base_tstzspanset(%s, %s, LINEAR): %s\n", geom1_out, tstzspanset1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* STBox *box3d_to_stbox(const BOX3D *box); */
  stbox_result = box3d_to_stbox(box3d1);
  char_result = stbox_out(stbox_result, 6);
  printf("box3d_to_stbox(%s): %s\n", box3d1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *gbox_to_stbox(const GBOX *box); */
  stbox_result = gbox_to_stbox(gbox1);
  char_result = stbox_out(stbox_result, 6);
  printf("gbox_to_stbox(%s): %s\n", gbox1_out, char_result);
  free(stbox_result); free(char_result);

  /* Temporal *geomeas_to_tpoint(const GSERIALIZED *gs); */
  bool_result = tpoint_tfloat_to_geomeas(tgeompt1_step, tfloat1, true, &geom_result);
  tgeompt_result = geomeas_to_tpoint(geom_result);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("geomeas_to_tpoint(%s): %s\n", geom1_out, char_result);
  free(geom_result); free(tgeompt_result); free(char_result);

  /* Temporal *tgeogpoint_to_tgeography(const Temporal *temp); */
  tgeogpt_result = tgeogpoint_to_tgeography(tgeogpt1_step);
  char_result = tspatial_as_ewkt(tgeogpt_result, 6);
  printf("tgeogpoint_to_tgeography(%s): %s\n", tgeompt1_step_out, char_result);
  free(tgeogpt_result); free(char_result);

  /* Temporal *tgeography_to_tgeogpoint(const Temporal *temp); */
  tgeogpt_result = tgeography_to_tgeogpoint(tgeog1_pt);
  char_result = tspatial_as_ewkt(tgeogpt_result, 6);
  printf("tgeography_to_tgeogpoint(%s): %s\n", tgeog1_pt_out, char_result);
  free(tgeogpt_result); free(char_result);

  /* Temporal *tgeography_to_tgeometry(const Temporal *temp); */
  tgeog_result = tgeography_in(tpoint_wkt_in);
  tgeom_result = tgeography_to_tgeometry(tgeog_result);
  char_result = tspatial_as_ewkt(tgeom_result, 6);
  printf("tgeography_to_tgeometry(%s): %s\n", tgeog1_out, char_result);
  free(tgeog_result); free(tgeom_result); free(char_result);

  /* Temporal *tgeometry_to_tgeography(const Temporal *temp); */
  tgeom_result = tgeometry_in(tpoint_wkt_in);
  tgeog_result = tgeometry_to_tgeography(tgeom_result);
  char_result = tspatial_as_ewkt(tgeog_result, 6);
  printf("tgeometry_to_tgeography(%s): %s\n", tgeom1_out, char_result);
  free(tgeom_result); free(tgeog_result); free(char_result);

  /* Temporal *tgeometry_to_tgeompoint(const Temporal *temp); */
  tgeogpt_result = tgeometry_to_tgeompoint(tgeom1_pt);
  char_result = tspatial_as_ewkt(tgeogpt_result, 6);
  printf("tgeometry_to_tgeompoint(%s): %s\n", tgeom1_pt_out, char_result);
  free(tgeogpt_result); free(char_result);

  /* Temporal *tgeompoint_to_tgeometry(const Temporal *temp); */
  tgeompt_result = tgeompoint_to_tgeometry(tgeompt1_step);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeompoint_to_tgeometry(%s): %s\n", tgeompt1_step_out, char_result);
  free(tgeompt_result); free(char_result);

  /* bool tpoint_as_mvtgeom(const Temporal *temp, const STBox *bounds, int32_t extent, int32_t buffer, bool clip_geom, GSERIALIZED **gsarr, int64 **timesarr, int *count); */
  bool_result = tpoint_as_mvtgeom(tgeompt1, stbox1, int32_in1, int32_in2, true, &geom_result, &int64array_result, &count);
  printf("tpoint_as_mvtgeom(%s, %s, %d, %d, true): %c\n", tgeompt1_out, stbox1_out, int32_in1, int32_in2, bool_result ? 't' : 'n');
  char_result = geo_as_ewkt(geom_result, 6);
  printf("%s: {", char_result);
  free(char_result);
  for (int i = 0; i < count; i++)
  {
    printf("%ld", int64array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(geom_result);
  free(int64array_result);

  /* bool tpoint_tfloat_to_geomeas(const Temporal *tpoint, const Temporal *measure, bool segmentize, GSERIALIZED **result); */
  bool_result = tpoint_tfloat_to_geomeas(tgeompt1, tfloat1, true, &geom_result);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tpoint_tfloat_to_geomeas(%s, %s, true, %s): %c\n", tgeompt1_out, tfloat1_out, char_result, bool_result ? 't' : 'n');
  free(geom_result); free(char_result);

  /* STBox *tspatial_to_stbox(const Temporal *temp); */
  stbox_result = tspatial_to_stbox(tgeompt1);
  char_result = stbox_out(stbox_result, 6);
  printf("tspatial_to_stbox(%s): %s\n", tgeompt1_out, char_result);
  free(stbox_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* bool bearing_point_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result); */
  bool_result = bearing_point_point(geompt1, geompt2, &float8_result);
  printf("bearing_point_point(%s, %s): %c\n", geompt1_out, geompt2_out, bool_result ? 't' : 'n');

  /* Temporal *bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert); */
  tfloat_result = bearing_tpoint_point(tgeompt1, geompt1, true);
  char_result = tfloat_out(tfloat_result, 6);
  printf("bearing_tpoint_point(%s, %s): %s\n", tgeompt1_out, geompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2); */
  tfloat_result = bearing_tpoint_tpoint(tgeompt1, tgeompt2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("bearing_tpoint_tpoint(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tgeo_centroid(const Temporal *temp); */
  tgeompt_result = tgeo_centroid(tgeompt1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeo_centroid(%s): %s\n", tgeompt1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* GSERIALIZED *tgeo_convex_hull(const Temporal *temp); */
  geom_result = tgeo_convex_hull(tgeompt1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_convex_hull(%s): %s\n", tgeompt1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *tgeo_end_value(const Temporal *temp); */
  geom_result = tgeo_end_value(tgeompt1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_end_value(%s): %s\n", tgeompt1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *tgeo_start_value(const Temporal *temp); */
  geom_result = tgeo_start_value(tgeompt1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_start_value(%s): %s\n", tgeompt1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *tgeo_traversed_area(const Temporal *temp, bool unary_union); */
  geom_result = tgeo_traversed_area(tgeom1, false);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_traversed_area(%s): %s\n", tgeom1_out, char_result);
  free(geom_result); free(char_result);

  /* bool tgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value); */
  bool_result = tgeo_value_at_timestamptz(tgeompt1, tstz1, true, &geom_result);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_value_at_timestamptz(%s, %s, true, %s): %c\n", tgeompt1_out, tstz1_out, char_result, bool_result ? 't' : 'n');
  free(geom_result); free(char_result);

  /* bool tgeo_value_n(const Temporal *temp, int n, GSERIALIZED **result); */
  bool_result = tgeo_value_n(tgeompt1, 1, &geom_result);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tgeo_value_n(%s, 1, %s): %c\n", tgeompt1_out, char_result, bool_result ? 't' : 'n');
  free(geom_result); free(char_result);

  /* GSERIALIZED **tgeo_values(const Temporal *temp, int *count); */
  geomarray_result = tgeo_values(tgeompt1, &count);
  printf("tgeo_values(%s, %d): {", tgeompt1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(geomarray_result[i]);
    free(char_result);
  }
  free(geomarray_result);

  /* Temporal *tpoint_angular_difference(const Temporal *temp); */
  tfloat_result = tpoint_angular_difference(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_angular_difference(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tpoint_azimuth(const Temporal *temp); */
  tfloat_result = tpoint_azimuth(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_azimuth(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tpoint_cumulative_length(const Temporal *temp); */
  tfloat_result = tpoint_cumulative_length(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_cumulative_length(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* bool tpoint_direction(const Temporal *temp, double *result); */
  bool_result = tpoint_direction(tgeompt1, &float8_result);
  printf("tpoint_direction(%s, %lf): %c\n", tgeompt1_out, float8_result, bool_result ? 't' : 'n');

  /* Temporal *tpoint_get_x(const Temporal *temp); */
  tfloat_result = tpoint_get_x(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_get_x(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tpoint_get_y(const Temporal *temp); */
  tfloat_result = tpoint_get_y(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_get_y(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tpoint_get_z(const Temporal *temp); */
  tfloat_result = tpoint_get_z(tgeompt3d1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_get_z(%s): %s\n", tgeompt3d1_out, char_result);
  free(tfloat_result); free(char_result);

  /* bool tpoint_is_simple(const Temporal *temp); */
  bool_result = tpoint_is_simple(tgeompt1);
  printf("tpoint_is_simple(%s): %c\n", tgeompt1_out, bool_result ? 't' : 'n');

  /* double tpoint_length(const Temporal *temp); */
  float8_result = tpoint_length(tgeompt1);
  printf("tpoint_length(%s): %lf\n", tgeompt1_out, float8_result);

  /* Temporal *tpoint_speed(const Temporal *temp); */
  tfloat_result = tpoint_speed(tgeompt1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tpoint_speed(%s): %s\n", tgeompt1_out, char_result);
  free(tfloat_result); free(char_result);

  /* GSERIALIZED *tpoint_trajectory(const Temporal *temp, bool unary_union); */
  geom_result = tpoint_trajectory(tgeompt1, false);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tpoint_trajectory(%s): %s\n", tgeompt1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *tpoint_twcentroid(const Temporal *temp); */
  geom_result = tpoint_twcentroid(tgeompt1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("tpoint_twcentroid(%s): %s\n", tgeompt1_out, char_result);
  free(geom_result); free(char_result);

  /* Transformation functions */
  printf("****************************************************************\n");

  /* Temporal *tgeo_affine(const Temporal *temp, const AFFINE *a); */
  /* Rotate a 3D temporal point 180 degrees about the z axis */
  AFFINE affine = {cos(M_PI), -sin(M_PI), 0, sin(M_PI), cos(M_PI), 0, 0, 0, 1, 0, 0, 0};
  tgeompt_result = tgeo_affine(tgeompt3d1, &affine);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeo_affine(%s, affine): %s\n", tgeompt3d1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Temporal *tgeo_scale(const Temporal *temp, const GSERIALIZED *scale, const GSERIALIZED *sorigin); */
  tgeompt_result = tgeo_scale(tgeompt1, geompt2, geompt1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tgeo_scale(%s, %s, %s): %s\n", tgeompt1_out, geompt2_out, geompt1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Temporal **tpoint_make_simple(const Temporal *temp, int *count); */
  tgeomptarray_result = tpoint_make_simple(tgeompt1, &count);
  printf("tpoint_make_simple(%s, %d): {", tgeompt1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tspatial_as_ewkt(tgeomptarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tgeomptarray_result[i]);
    free(char_result);
  }
  free(tgeomptarray_result);

  /* SRID functions */
  printf("****************************************************************\n");

  /* int32_t tspatial_srid(const Temporal *temp); */
  int32_result = tspatial_srid(tgeompt1);
  printf("tspatial_srid(%s): %d\n", tgeompt1_out, int32_result);

  /* Temporal *tspatial_set_srid(const Temporal *temp, int32_t srid); */
  // TODO Remove the need to create a BRAND NEW temporal geometry
  Temporal *tgeom1_test = tgeometry_in("[Point(1 1)@2001-01-01, Point(5 5)@2001-01-05]");
  char *tgeom1_test_out = tspatial_as_ewkt(tgeom1_test, 6);
  tgeompt_result = tspatial_set_srid(tgeom1_test, 4326);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tspatial_set_srid(%s, 4326): %s\n", tgeom1_test_out, char_result);
  free(tgeom1_test); free(tgeom1_test_out); free(tgeompt_result); free(char_result);

  /* Temporal *tspatial_transform(const Temporal *temp, int32_t srid); */
  tgeom_result = tspatial_transform(tgeom1, 4326);
  char_result = tspatial_as_ewkt(tgeom_result, 6);
  printf("tspatial_transform(%s, 4326): %s\n", tgeom1_out, char_result);
  free(tgeom_result); free(char_result);

  /* Temporal *tspatial_transform_pipeline(const Temporal *temp, const char *pipelinestr, int32_t srid, bool is_forward); */
  tgeompt_result = tspatial_transform_pipeline(tgeompt1, "urn:ogc:def:coordinateOperation:EPSG::1671", 1671, true);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tspatial_transform_pipeline(%s, \"urn:ogc:def:coordinateOperation:EPSG::1671\", 1671, true): %s\n", tgeompt1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Restriction functions */
  printf("****************************************************************\n");

  /* Temporal *tgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tgeompt_result = tgeo_at_geom(tgeompt1, geom3);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_at_geom(%s, %s): %s\n", tgeompt1_out, geom3_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tgeompt_result = tgeo_at_stbox(tgeompt1, stbox1, true);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_at_stbox(%s, %s, true): %s\n", tgeompt1_out, stbox1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tgeo_at_value(const Temporal *temp, GSERIALIZED *gs); */
  tgeompt_result = tgeo_at_value(tgeompt1, geom1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_at_value(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tgeompt_result = tgeo_minus_geom(tgeompt1, geom1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_minus_geom(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tgeompt_result = tgeo_minus_stbox(tgeompt1, stbox1, true);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_minus_stbox(%s, %s, true): %s\n", tgeompt1_out, stbox1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tgeo_minus_value(const Temporal *temp, GSERIALIZED *gs); */
  tgeompt_result = tgeo_minus_value(tgeompt1, geom1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tgeo_minus_value(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan); */
  tgeompt_result = tpoint_at_geom(tgeompt3d1, geom1, fspan1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tpoint_at_geom(%s, %s, %s): %s\n", tgeompt3d1_out, geom1_out, fspan1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tpoint_at_value(const Temporal *temp, GSERIALIZED *gs); */
  tgeompt_result = tpoint_at_value(tgeompt1, geom1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tpoint_at_value(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan); */
  tgeompt_result = tpoint_minus_geom(tgeompt3d1, geom1, fspan1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tpoint_minus_geom(%s, %s, %s): %s\n", tgeompt3d1_out, geom1_out, fspan1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Temporal *tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs); */
  tgeompt_result = tpoint_minus_value(tgeompt1, geom1);
  char_result = tgeompt_result ? tspatial_as_ewkt(tgeompt_result, 6) : text_out(text_null);
  printf("tpoint_minus_value(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  if (tgeompt_result)
    free(tgeompt_result);
  free(char_result);

  /* Ever and always comparisons */
  printf("****************************************************************\n");

  /* int always_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = always_eq_geo_tgeo(geompt1, tgeom1);
  printf("always_eq_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int always_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = always_eq_tgeo_geo(tgeompt1, geom1);
  printf("always_eq_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int always_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_eq_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("always_eq_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int always_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = always_ne_geo_tgeo(geompt1, tgeom1);
  printf("always_ne_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int always_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = always_ne_tgeo_geo(tgeompt1, geom1);
  printf("always_ne_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int always_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_ne_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("always_ne_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int ever_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = ever_eq_geo_tgeo(geompt1, tgeom1);
  printf("ever_eq_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int ever_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = ever_eq_tgeo_geo(tgeompt1, geom1);
  printf("ever_eq_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int ever_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_eq_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("ever_eq_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int ever_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = ever_ne_geo_tgeo(geompt1, tgeom1);
  printf("ever_ne_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int ever_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = ever_ne_tgeo_geo(tgeompt1, geom1);
  printf("ever_ne_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int ever_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_ne_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("ever_ne_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* Temporal comparisons */
  printf("****************************************************************\n");

  /* Temporal *teq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  tbool_result = teq_geo_tgeo(geompt1, tgeom1);
  char_result = tbool_out(tbool_result);
  printf("teq_geo_tgeo(%s, %s): %s\n", geompt1_out, tgeom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tbool_result = teq_tgeo_geo(tgeompt1, geom1);
  char_result = tbool_out(tbool_result);
  printf("teq_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  tbool_result = tne_geo_tgeo(geompt1, tgeom1);
  char_result = tbool_out(tbool_result);
  printf("tne_geo_tgeo(%s, %s): %s\n", geompt1_out, tgeom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tbool_result = tne_tgeo_geo(tgeompt1, geom1);
  char_result = tbool_out(tbool_result);
  printf("tne_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Bounding box functions */
  printf("****************************************************************\n");

  /* STBox *tgeo_stboxes(const Temporal *temp, int *count); */
  stboxarray_result = tgeo_stboxes(tgeompt1, &count);
  printf("tgeo_stboxes(%s, %d): {", geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *tgeo_space_boxes(const Temporal *temp, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc, int *count); */
  stboxarray_result = tgeo_space_boxes(tgeompt1, 1, 1, 1, geompt1, false, true, &count); // TODO true, true
  printf("tgeo_space_boxes(%s, 1, 1, 1, %s, false, true, %d): {", tgeompt1_out, geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *tgeo_space_time_boxes(const Temporal *temp, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool bitmatrix, bool border_inc, int *count); */
  stboxarray_result = tgeo_space_time_boxes(tgeompt1, 1, 1, 1, interv1, geompt1, tstz1, false, true, &count); // TODO true, true
  printf("tgeo_space_time_boxes(%s, 1, 1, 1, %s, %s, %s, %d): {", tgeompt1_out, interv1_out, geompt1_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *tgeo_split_each_n_stboxes(const Temporal *temp, int elem_count, int *count); */
  stboxarray_result = tgeo_split_each_n_stboxes(tgeompt1, 2, &count);
  printf("tgeo_split_each_n_stboxes(%s, 2, %d): {", geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *tgeo_split_n_stboxes(const Temporal *temp, int box_count, int *count); */
  stboxarray_result = tgeo_split_n_stboxes(tgeompt1, 2, &count);
  printf("tgeo_split_n_stboxes(%s, 2, %d): {", geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* Topological functions */
  printf("****************************************************************\n");

  /* bool adjacent_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = adjacent_stbox_tspatial(stbox1, tgeompt1);
  printf("adjacent_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = adjacent_tspatial_stbox(tgeompt1, stbox1);
  printf("adjacent_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool adjacent_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = adjacent_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("adjacent_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool contained_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = contained_stbox_tspatial(stbox1, tgeompt1);
  printf("contained_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool contained_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = contained_tspatial_stbox(tgeompt1, stbox1);
  printf("contained_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool contained_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contained_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("contained_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool contains_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = contains_stbox_tspatial(stbox1, tgeompt1);
  printf("contains_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool contains_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = contains_tspatial_stbox(tgeompt1, stbox1);
  printf("contains_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool contains_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = contains_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("contains_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overlaps_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overlaps_stbox_tspatial(stbox1, tgeompt1);
  printf("overlaps_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overlaps_tspatial_stbox(tgeompt1, stbox1);
  printf("overlaps_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overlaps_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overlaps_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overlaps_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool same_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = same_stbox_tspatial(stbox1, tgeompt1);
  printf("same_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool same_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = same_tspatial_stbox(tgeompt1, stbox1);
  printf("same_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool same_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = same_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("same_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* Position functions */
  printf("****************************************************************\n");

  /* bool above_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = above_stbox_tspatial(stbox1, tgeompt1);
  printf("above_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool above_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = above_tspatial_stbox(tgeompt1, stbox1);
  printf("above_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool above_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = above_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("above_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool after_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = after_stbox_tspatial(stbox1, tgeompt1);
  printf("after_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool after_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = after_tspatial_stbox(tgeompt1, stbox1);
  printf("after_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool after_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = after_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("after_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool back_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = back_stbox_tspatial(stbox3d1, tgeompt3d1);
  printf("back_stbox_tspatial(%s, %s): %c\n", stbox3d1_out, tgeompt3d1_out, bool_result ? 't' : 'n');

  /* bool back_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = back_tspatial_stbox(tgeompt3d1, stbox3d1);
  printf("back_tspatial_stbox(%s, %s): %c\n", tgeompt3d1_out, stbox3d1_out, bool_result ? 't' : 'n');

  /* bool back_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = back_tspatial_tspatial(tgeompt3d1, tgeompt3d2);
  printf("back_tspatial_tspatial(%s, %s): %c\n", tgeompt3d1_out, tgeompt3d2_out, bool_result ? 't' : 'n');

  /* bool before_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = before_stbox_tspatial(stbox1, tgeompt1);
  printf("before_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool before_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = before_tspatial_stbox(tgeompt1, stbox1);
  printf("before_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool before_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = before_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("before_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool below_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = below_stbox_tspatial(stbox1, tgeompt1);
  printf("below_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool below_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = below_tspatial_stbox(tgeompt1, stbox1);
  printf("below_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool below_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = below_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("below_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool front_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = front_stbox_tspatial(stbox3d1, tgeompt3d1);
  printf("front_stbox_tspatial(%s, %s): %c\n", stbox3d1_out, tgeompt3d1_out, bool_result ? 't' : 'n');

  /* bool front_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = front_tspatial_stbox(tgeompt3d1, stbox3d1);
  printf("front_tspatial_stbox(%s, %s): %c\n", tgeompt3d1_out, stbox3d1_out, bool_result ? 't' : 'n');

  /* bool front_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = front_tspatial_tspatial(tgeompt3d1, tgeompt3d2);
  printf("front_tspatial_tspatial(%s, %s): %c\n", tgeompt3d1_out, tgeompt3d2_out, bool_result ? 't' : 'n');

  /* bool left_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = left_stbox_tspatial(stbox1, tgeompt1);
  printf("left_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool left_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = left_tspatial_stbox(tgeompt1, stbox1);
  printf("left_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool left_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = left_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("left_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overabove_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overabove_stbox_tspatial(stbox1, tgeompt1);
  printf("overabove_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overabove_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overabove_tspatial_stbox(tgeompt1, stbox1);
  printf("overabove_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overabove_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overabove_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overabove_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overafter_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overafter_stbox_tspatial(stbox1, tgeompt1);
  printf("overafter_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overafter_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overafter_tspatial_stbox(tgeompt1, stbox1);
  printf("overafter_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overafter_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overafter_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overafter_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overback_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overback_stbox_tspatial(stbox3d1, tgeompt3d1);
  printf("overback_stbox_tspatial(%s, %s): %c\n", stbox3d1_out, tgeompt3d1_out, bool_result ? 't' : 'n');

  /* bool overback_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overback_tspatial_stbox(tgeompt3d1, stbox3d1);
  printf("overback_tspatial_stbox(%s, %s): %c\n", tgeompt3d1_out, stbox3d1_out, bool_result ? 't' : 'n');

  /* bool overback_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overback_tspatial_tspatial(tgeompt3d1, tgeompt3d2);
  printf("overback_tspatial_tspatial(%s, %s): %c\n", tgeompt3d1_out, tgeompt3d2_out, bool_result ? 't' : 'n');

  /* bool overbefore_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overbefore_stbox_tspatial(stbox1, tgeompt1);
  printf("overbefore_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overbefore_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overbefore_tspatial_stbox(tgeompt1, stbox1);
  printf("overbefore_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overbefore_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overbefore_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overbefore_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overbelow_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overbelow_stbox_tspatial(stbox1, tgeompt1);
  printf("overbelow_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overbelow_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overbelow_tspatial_stbox(tgeompt1, stbox1);
  printf("overbelow_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overbelow_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overbelow_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overbelow_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overfront_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overfront_stbox_tspatial(stbox3d1, tgeompt3d1);
  printf("overfront_stbox_tspatial(%s, %s): %c\n", stbox3d1_out, tgeompt3d1_out, bool_result ? 't' : 'n');

  /* bool overfront_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overfront_tspatial_stbox(tgeompt3d1, stbox3d1);
  printf("overfront_tspatial_stbox(%s, %s): %c\n", tgeompt3d1_out, stbox3d1_out, bool_result ? 't' : 'n');

  /* bool overfront_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overfront_tspatial_tspatial(tgeompt3d1, tgeompt3d2);
  printf("overfront_tspatial_tspatial(%s, %s): %c\n", tgeompt3d1_out, tgeompt3d2_out, bool_result ? 't' : 'n');

  /* bool overleft_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overleft_stbox_tspatial(stbox1, tgeompt1);
  printf("overleft_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overleft_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overleft_tspatial_stbox(tgeompt1, stbox1);
  printf("overleft_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overleft_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overleft_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overleft_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool overright_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = overright_stbox_tspatial(stbox1, tgeompt1);
  printf("overright_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool overright_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = overright_tspatial_stbox(tgeompt1, stbox1);
  printf("overright_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool overright_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = overright_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("overright_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* bool right_stbox_tspatial(const STBox *box, const Temporal *temp); */
  bool_result = right_stbox_tspatial(stbox1, tgeompt1);
  printf("right_stbox_tspatial(%s, %s): %c\n", stbox1_out, tgeompt1_out, bool_result ? 't' : 'n');

  /* bool right_tspatial_stbox(const Temporal *temp, const STBox *box); */
  bool_result = right_tspatial_stbox(tgeompt1, stbox1);
  printf("right_tspatial_stbox(%s, %s): %c\n", tgeompt1_out, stbox1_out, bool_result ? 't' : 'n');

  /* bool right_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2); */
  bool_result = right_tspatial_tspatial(tgeompt1, tgeompt2);
  printf("right_tspatial_tspatial(%s, %s): %c\n", tgeompt1_out, tgeompt2_out, bool_result ? 't' : 'n');

  /* Ever and always spatial relationships */

  /* int acontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = acontains_geo_tgeo(geompt1, tgeom1);
  printf("acontains_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int acontains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = acontains_tgeo_geo(tgeompt1, geom1);
  printf("acontains_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int acontains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = acontains_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("acontains_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int adisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = adisjoint_tgeo_geo(tgeompt1, geom1);
  printf("adisjoint_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int adisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = adisjoint_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("adisjoint_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int adwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist); */
  int32_result = adwithin_tgeo_geo(tgeompt1, geom1, float8_in1);
  printf("adwithin_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int adwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist); */
  int32_result = adwithin_tgeo_tgeo(tgeompt1, tgeompt2, float8_in1);
  printf("adwithin_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int aintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = aintersects_tgeo_geo(tgeompt1, geom1);
  printf("aintersects_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int aintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = aintersects_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("aintersects_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int atouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = atouches_tgeo_geo(tgeompt1, geom1);
  printf("atouches_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int atouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = atouches_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("atouches_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int atouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = atouches_tpoint_geo(tgeompt1, geom1);
  printf("atouches_tpoint_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int econtains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = econtains_geo_tgeo(geompt1, tgeom1);
  printf("econtains_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int econtains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = econtains_tgeo_geo(tgeompt1, geom1);
  printf("econtains_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int econtains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = econtains_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("econtains_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int ecovers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = ecovers_geo_tgeo(geompt1, tgeom1);
  printf("ecovers_geo_tgeo(%s, %s): %d\n", geompt1_out, tgeom1_out, int32_result);

  /* int ecovers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = ecovers_tgeo_geo(tgeompt1, geom1);
  printf("ecovers_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int ecovers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ecovers_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("ecovers_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int edisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = edisjoint_tgeo_geo(tgeompt1, geom1);
  printf("edisjoint_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int edisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = edisjoint_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("edisjoint_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int edwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist); */
  int32_result = edwithin_tgeo_geo(tgeompt1, geom1, float8_in1);
  printf("edwithin_tgeo_geo(%s, %s, %lf): %d\n", tgeompt1_out, geom1_out, float8_in1, int32_result);

  /* int edwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist); */
  int32_result = edwithin_tgeo_tgeo(tgeompt1, tgeompt2, float8_in1);
  printf("edwithin_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int eintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = eintersects_tgeo_geo(tgeompt1, geom1);
  printf("eintersects_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int eintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = eintersects_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("eintersects_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int etouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = etouches_tgeo_geo(tgeompt1, geom1);
  printf("etouches_tgeo_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* int etouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  int32_result = etouches_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("etouches_tgeo_tgeo(%s, %s): %d\n", tgeompt1_out, tgeompt2_out, int32_result);

  /* int etouches_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = etouches_tpoint_geo(tgeompt1, geom1);
  printf("etouches_tpoint_geo(%s, %s): %d\n", tgeompt1_out, geom1_out, int32_result);

  /* Spatiotemporal relationships */
  printf("****************************************************************\n");

  /* Temporal *tcontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcontains_geo_tgeo(geom3, tgeompt1, false, false);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_geo_tgeo(%s, %s): %s\n", geom3_out, tgeompt1_out, char_result);
  if (tbool_result)
    free(tbool_result);
  free(char_result);

  /* Temporal *tcontains_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tcontains_tgeo_geo(tgeompt1, geom3, false, false);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom3_out, char_result);
  if (tbool_result)
    free(tbool_result);
  free(char_result);

  /* Temporal *tcontains_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tcontains_tgeo_tgeo(tgeompt1, tgeompt2, false, false);
  char_result = tbool_out(tbool_result);
  printf("tcontains_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcovers_geo_tgeo(geom1, tgeompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tcovers_geo_tgeo(%s, %s): %s\n", geom1_out, tgeompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tcovers_tgeo_geo(tgeompt1, geompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tcovers_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tcovers_tgeo_tgeo(tgeompt1, tgeompt2, false, false);
  char_result = tbool_out(tbool_result);
  printf("tcovers_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tdisjoint_geo_tgeo(geom1, tgeompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdisjoint_geo_tgeo(%s, %s): %s\n", geom1_out, tgeompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tdisjoint_tgeo_geo(tgeompt1, geompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdisjoint_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tdisjoint_tgeo_tgeo(tgeompt1, tgeompt2, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdisjoint_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_geo_tgeo(geom1, tgeompt1, float8_in1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdwithin_geo_tgeo(%s, %s): %s\n", geom1_out, tgeompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_tgeo_geo(tgeompt1, geom1, float8_in1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdwithin_tgeo_geo(%s, %s, %lf): %s\n", tgeompt1_out, geom1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_tgeo_tgeo(tgeompt1, tgeompt2, float8_in1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tdwithin_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tintersects_geo_tgeo(geom1, tgeompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tintersects_geo_tgeo(%s, %s): %s\n", geom1_out, tgeompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tintersects_tgeo_geo(tgeompt1, geompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("tintersects_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tintersects_tgeo_tgeo(tgeompt1, tgeompt2, false, false);
  char_result = tbool_out(tbool_result);
  printf("tintersects_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = ttouches_geo_tgeo(geom1, tgeompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("ttouches_geo_tgeo(%s, %s): %s\n", geom1_out, tgeompt1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = ttouches_tgeo_geo(tgeompt1, geompt1, false, false);
  char_result = tbool_out(tbool_result);
  printf("ttouches_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = ttouches_tgeo_tgeo(tgeompt1, tgeompt2, false, false);
  char_result = tbool_out(tbool_result);
  printf("ttouches_tgeo_tgeo(%s, %s): %s\n", tgeom1_out, tgeom2_out, char_result);
  free(tbool_result); free(char_result);

  /* Distance */
  printf("****************************************************************\n");

  /* Temporal *tdistance_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tfloat_result = tdistance_tgeo_geo(tgeompt1, geom1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  tfloat_result = tdistance_tgeo_tgeo(tgeompt1, tgeompt2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tfloat_result); free(char_result);

  /* double nad_stbox_geo(const STBox *box, const GSERIALIZED *gs); */
  float8_result = nad_stbox_geo(stbox1, geom1);
  printf("nad_stbox_geo(%s, %s): %lf\n", stbox1_out, geom1_out, float8_result);

  /* double nad_stbox_stbox(const STBox *box1, const STBox *box2); */
  float8_result = nad_stbox_stbox(stbox1, stbox2);
  printf("nad_stbox_stbox(%s, %s): %lf\n", stbox1_out, stbox2_out, float8_result);

  /* double nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  float8_result = nad_tgeo_geo(tgeompt1, geom1);
  printf("nad_tgeo_geo(%s, %s): %lf\n", tgeompt1_out, geom1_out, float8_result);

  /* double nad_tgeo_stbox(const Temporal *temp, const STBox *box); */
  float8_result = nad_tgeo_stbox(tgeompt1, stbox1);
  printf("nad_tgeo_stbox(%s, %s): %lf\n", tgeompt1_out, stbox1_out, float8_result);

  /* double nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  float8_result = nad_tgeo_tgeo(tgeompt1, tgeompt2);
  printf("nad_tgeo_tgeo(%s, %s): %lf\n", tgeompt1_out, tgeompt2_out, float8_result);

  /* TInstant *nai_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tgeompt_result = (Temporal *) nai_tgeo_geo(tgeompt1, geom1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("nai_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* TInstant *nai_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  tgeompt_result = (Temporal *) nai_tgeo_tgeo(tgeompt1, tgeompt2);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("nai_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(tgeompt_result); free(char_result);

  /* GSERIALIZED *shortestline_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs); */
  geom_result = shortestline_tgeo_geo(tgeompt1, geom1);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("shortestline_tgeo_geo(%s, %s): %s\n", tgeompt1_out, geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *shortestline_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2); */
  geom_result = shortestline_tgeo_tgeo(tgeompt1, tgeompt2);
  char_result = geo_as_ewkt(geom_result, 6);
  printf("shortestline_tgeo_tgeo(%s, %s): %s\n", tgeompt1_out, tgeompt2_out, char_result);
  free(geom_result); free(char_result);

  /* Aggregates */
  printf("****************************************************************\n");

  /* SkipList *tpoint_tcentroid_transfn(SkipList *state, Temporal *temp); */
  sklist = tpoint_tcentroid_transfn(NULL, tgeompt1);
  sklist = tpoint_tcentroid_transfn(sklist, tgeompt2);
  tgeompt_result = tpoint_tcentroid_finalfn(sklist);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tpoint_tcentroid aggregate\n");
  printf("%s\n", tgeompt1_out);
  printf("%s\n", tgeompt2_out);
  printf("tpoint_tcentroid result\n");
  printf("%s\n", char_result);
  free(tgeompt_result); free(char_result);

  /* STBox *tspatial_extent_transfn(STBox *box, const Temporal *temp); */
  STBox *stbox_agg = tspatial_extent_transfn(NULL, tgeom1);
  stbox_agg = tspatial_extent_transfn(stbox_agg, tgeom2);
  char_result = stbox_out(stbox_agg, 6);
  printf("tspatial_extent_transfn aggregate\n");
  printf("%s\n", tgeom1_out);
  printf("%s\n", tgeom2_out);
  printf("tspatial_extent_transfn result\n");
  printf("%s\n", char_result);
  free(stbox_agg); free(char_result);

  /* Tile functions */
  printf("****************************************************************\n");

  /* STBox *stbox_get_space_tile(const GSERIALIZED *point, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin); */
  stbox_result = stbox_get_space_tile(geompt1, 1, 1, 1, geompt2);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_get_space_tile(%s, 1, 1, 1, %s): %s\n", geompt1_out, geompt2_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_get_space_time_tile(const GSERIALIZED *point, TimestampTz t, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin); */
  stbox_result = stbox_get_space_time_tile(geompt1, tstz1, 1, 1, 1, interv1, geompt2, tstz1);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_get_space_time_tile(%s, %s, 1, 1, 1, %s, %s, %s): %s\n", geompt1_out, tstz1_out, interv1_out, geompt2_out, tstz1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_get_time_tile(TimestampTz t, const Interval *duration, TimestampTz torigin); */
  stbox_result = stbox_get_time_tile(tstz1, interv1, tstz2);
  char_result = stbox_out(stbox_result, 6);
  printf("stbox_get_time_tile(%s, %s, %s): %s\n", tstz1_out, interv1_out, tstz2_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *stbox_space_tiles(const STBox *bounds, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool border_inc, int *count); */
  stboxarray_result = stbox_space_tiles(stbox1, 1, 1, 1, geompt1, true, &count);
  printf("stbox_space_tiles(%s, 1, 1, 1, %s, true, %d): {", stbox1_out, geompt1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);
  
  /* STBox *stbox_space_time_tiles(const STBox *bounds, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool border_inc, int *count); */
  stboxarray_result = stbox_space_time_tiles(stbox1, 1, 1, 1, interv1, geompt1, tstz1, true, &count);
  printf("stbox_space_time_tiles(%s, 1, 1, 1, %s, %s, %s, true, %d): {", stbox1_out, interv1_out, geompt1_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* STBox *stbox_time_tiles(const STBox *bounds, const Interval *duration, TimestampTz torigin, bool border_inc, int *count); */
  stboxarray_result = stbox_time_tiles(stbox1, interv1, tstz1, true, &count);
  printf("stbox_time_tiles(%s, %s, %s, true, %d): {", stbox1_out, interv1_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = stbox_out(&(stboxarray_result[i]), 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(char_result);
  }
  free(stboxarray_result);

  /* Temporal **tgeo_space_split(const Temporal *temp, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc, GSERIALIZED ***space_bins, int *count); */
  tgeomptarray_result = tgeo_space_split(tgeompt1, 1, 1, 1, geompt1, false, true, &geomarray_result, &count);
  printf("tgeo_space_split(%s, 1, 1, 1, %s, false, true, &geomarray_result, %d): {", tgeompt1_out, geompt1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tspatial_as_ewkt(tgeomptarray_result[i], 6);
    char1_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s: %s", char_result, char1_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tgeomptarray_result[i]);
    free(geomarray_result[i]);
    free(char_result);
    free(char1_result);
  }
  free(tgeomptarray_result);
  free(geomarray_result);

  /* Temporal **tgeo_space_time_split(const Temporal *temp, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool bitmatrix, bool border_inc, GSERIALIZED ***space_bins, TimestampTz **time_bins, int *count); */
  tgeomptarray_result = tgeo_space_time_split(tgeompt1, 1, 1, 1, interv1, geompt1, tstz1, false, true, &geomarray_result, &tstzarray_result, &count);
  printf("tgeo_space_time_split(%s, 1, 1, 1, %s, %s, %s, false, true, &geomarray_result, &tstzarray_result, %d): {", tgeompt1_out, interv1_out, geompt1_out, tstz1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = tspatial_as_ewkt(tgeomptarray_result[i], 6);
    char1_result = geo_as_ewkt(geomarray_result[i], 6);
    char2_result = timestamptz_out(tstzarray_result[i]);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(tgeomptarray_result[i]);
    free(geomarray_result[i]);
    free(char_result);
    free(char1_result);
    free(char2_result);
  }
  free(tgeomptarray_result);
  free(geomarray_result);
  free(tstzarray_result);

  /* Clustering functions */
  printf("****************************************************************\n");

  /* int *geo_cluster_kmeans(const GSERIALIZED **geoms, uint32_t ngeoms, uint32_t k); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  int32array_result = geo_cluster_kmeans((const GSERIALIZED **) geomarray, 2, 2);
  printf("geo_cluster_kmeans(%s, 2, 2): {", geom1_out);
  for (int i = 0; i < 2; i++)
  {
    printf("%u", int32array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(int32array_result);

  /* uint32_t *geo_cluster_dbscan(const GSERIALIZED **geoms, uint32_t ngeoms, double tolerance, int minpoints, int *count); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  uint32array_result = geo_cluster_dbscan((const GSERIALIZED **) geomarray, 2, 0.1, 2, &count);
  printf("geo_cluster_dbscan({%s, %s}, 2, 0.1, 2): {", geom1_out, geom2_out);
  for (int i = 0; i < count; i++)
  {
    printf("%u", uint32array_result[i]);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(uint32array_result);

  /* GSERIALIZED **geo_cluster_intersecting(const GSERIALIZED **geoms, uint32_t ngeoms, int *count); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  geomarray_result = geo_cluster_intersecting((const GSERIALIZED **) geomarray, 2, &count);
  printf("geo_cluster_intersecting({%s, %s}, 2, %d): {", geom1_out, geom2_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(geomarray_result[i]);
    free(char_result);
  }
  free(geomarray_result);

  /* GSERIALIZED **geo_cluster_within(const GSERIALIZED **geoms, uint32_t ngeoms, double tolerance, int *count); */
  geomarray[0] = geom1;
  geomarray[1] = geom2;
  geomarray_result = geo_cluster_within((const GSERIALIZED **) geomarray, 2, 0.1, &count);
  printf("geo_cluster_within(%s, 2, 0.1, %d): {", geom1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = geo_as_ewkt(geomarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(geomarray_result[i]);
    free(char_result);
  }
  free(geomarray_result);

  /*****************************************************************************/
  /* Similarity functions for temporal geotypes */
  printf("****************************************************************\n");

  /* double temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_dyntimewarp_distance(tgeompt1, tgeompt2);
  printf("temporal_dyntimewarp_distance(%s, %s): %lf\n", tgeompt1_out, tgeompt2_out, float8_result);

  /* Match *temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count); */
  matches = temporal_dyntimewarp_path(tgeompt1, tgeompt2, &count);
  printf("temporal_dyntimewarp_path(%s, %s): {", tgeompt1_out, tgeompt2_out);
  for (int i = 0; i < count; i++)
  {
    printf("(%d, %d)", matches[i].i, matches[i].j);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(matches);

  /* double temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_frechet_distance(tgeompt1, tgeompt2);
  printf("temporal_frechet_distance(%s, %s): %lf\n", tgeompt1_out, tgeompt2_out, float8_result);

  /* Match *temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count); */
  matches = temporal_frechet_path(tgeompt1, tgeompt2, &count);
  printf("temporal_frechet_path(%s, %s): {", tgeompt1_out, tgeompt2_out);
  for (int i = 0; i < count; i++)
  {
    printf("(%d, %d)", matches[i].i, matches[i].j);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
  }
  free(matches);

  /* double temporal_hausdorff_distance(const Temporal *temp1, const Temporal *temp2); */
  float8_result = temporal_hausdorff_distance(tgeompt1, tgeompt2);
  printf("temporal_hausdorff_distance(%s, %s): %lf\n", tgeompt1_out, tgeompt2_out, float8_result);

  printf("****************************************************************\n");

  /* Clean up */
  free(interv1); free(interv1_out);
  free(interv2); free(interv2_out);
  free(tstz1_out); free(tstz2_out);
  free(text_null);
  free(fspan1); free(fspan1_out);
  free(tstzset1); free(tstzset1_out);
  free(tstzspan1); free(tstzspan1_out);
  free(tstzspanset1); free(tstzspanset1_out);
  free(geomset1); free(geomset1_out);
  free(box3d1); free(box3d1_out);
  free(gbox1); free(gbox1_out);
  free(geom1); free(geom1_out); 
    free(geom1_hexwkb); free(geom1_wkb); free(geom1_geojson);
  free(geom2); free(geom2_out);
  free(geom3); free(geom3_out);
  free(geompt1); free(geompt1_out);
  free(geompt2); free(geompt2_out);
  free(geom3d1); free(geom3d1_out);
  free(geom3d2); free(geom3d2_out);
  free(geog1); free(geog1_out); free(geog1_hexwkb); free(geog1_wkb);
  free(geog2); free(geog2_out);
  free(line1); free(line1_out);
  free(tfloat1); free(tfloat1_out);
  free(stbox1); free(stbox1_out); free(stbox1_hexwkb); free(stbox1_wkb);
  free(stbox2); free(stbox2_out);
  free(stbox3d1); free(stbox3d1_out);
  free(stbox3d2); free(stbox3d2_out);
  free(tgeompt1); free(tgeompt1_out); free(tgeompt1_mfjson);
  free(tgeompt2); free(tgeompt2_out);
  free(tgeompt3d1); free(tgeompt3d1_out);
  free(tgeompt3d2); free(tgeompt3d2_out);
  free(tgeompt1_step); free(tgeompt1_step_out);
  free(tgeogpt1_step); free(tgeogpt1_step_out);
  free(tgeogpt1); free(tgeogpt1_out); free(tgeogpt1_mfjson);
  free(tgeogpt2); free(tgeogpt2_out);
  free(tgeom1); free(tgeom1_out); free(tgeom1_mfjson);
  free(tgeom2); free(tgeom2_out);
  free(tgeog1); free(tgeog1_out); free(tgeog1_mfjson);
  free(tgeog2); free(tgeog2_out);
  free(tgeom1_pt); free(tgeom1_pt_out);
  free(tgeog1_pt); free(tgeog1_pt_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}


