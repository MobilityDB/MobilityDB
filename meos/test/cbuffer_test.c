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
 * @brief A simple program that tests the funtions for the temporal circular
 * buffer types in MEOS, that is, cbuffer, cbufferset, and tcbuffer.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o cbuffer_test cbuffer_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <pg_bool.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */

  double float8_in1 = 1;
  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);

  text *text_null = text_in("NULL");

  char *cbuffer1_in = "SRID=5676;Cbuffer(Point(1 1), 1)";
  Cbuffer *cbuffer1 = cbuffer_in(cbuffer1_in);
  char *cbuffer1_out = cbuffer_as_text(cbuffer1, 6);
  char *cbuffer2_in = "SRID=5676;Cbuffer(Point(2 2), 1)";
  Cbuffer *cbuffer2 = cbuffer_in(cbuffer2_in);
  char *cbuffer2_out = cbuffer_as_text(cbuffer2, 6);

  char *cbufferset1_in = "SRID=5676;{\"Cbuffer(Point(1 1), 1)\", \"Cbuffer(Point(2 2), 1)\"}";
  Set *cbufferset1 = cbufferset_in(cbufferset1_in);
  char *cbufferset1_out = spatialset_as_text(cbufferset1, 6);

  char *geom1_in = "SRID=5676;Point(1 1)";
  GSERIALIZED *geom1 = geom_in(geom1_in, -1);
  char *geom1_out = geo_as_text(geom1, 6);

  Cbuffer *cbufferarray[2];

  char *tfloat1_in = "[1@2001-01-01, 3@2001-01-03]";
  Temporal *tfloat1 = tfloat_in(tfloat1_in);
  char *tfloat1_out = tfloat_out(tfloat1, 6);

  char *stbox1_in = "SRID=5676;STBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])";
  STBox *stbox1 = stbox_in(stbox1_in);
  char *stbox1_out = stbox_out(stbox1, 6);

  char *tgeompt1_in = "SRID=5676;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]";
  Temporal *tgeompt1 = tgeompoint_in(tgeompt1_in);
  char *tgeompt1_out = tspatial_as_text(tgeompt1, 6);

  char *tcbuffer1_in = "SRID=5676;[Cbuffer(Point(1 1), 1)@2001-01-01, Cbuffer(Point(3 3), 1)@2001-01-03]";
  Temporal *tcbuffer1 = tcbuffer_in(tcbuffer1_in);
  char *tcbuffer1_out = tspatial_as_text(tcbuffer1, 6);
  char *tcbuffer2_in = "SRID=5676;[Cbuffer(Point(3 3), 1)@2001-01-01, Cbuffer(Point(1 1), 1)@2001-01-03]";
  Temporal *tcbuffer2 = tcbuffer_in(tcbuffer2_in);
  char *tcbuffer2_out = tspatial_as_text(tcbuffer2, 6);

  /* Create the result types for the functions of the API */

  bool bool_result;
  int32_t int32_result;
  uint32_t uint32_result;
  uint64_t uint64_result;
  double float8_result;
  char *char_result;
  size_t size;

  Set *floatset_result;
  GSERIALIZED *geom_result;
  Set *geomset_result;
  Cbuffer *cbuffer_result;
  Set *cbufferset_result;
  STBox *stbox_result;
  Temporal *tbool_result;
  Temporal *tfloat_result;
  Temporal *tgeompt_result;
  Temporal *tcbuffer_result;
  Cbuffer **cbufferarray_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Circular buffer types *\n");
  printf("****************************************************************\n");

  /* Input and output functions */

  /* char *cbuffer_as_ewkt(const Cbuffer *cb, int maxdd); */
  char_result = cbuffer_as_ewkt(cbuffer1, 6);
  printf("cbuffer_as_ewkt(%s, 6): %s\n", cbuffer1_out, char_result);
  free(char_result);

  /* char *cbuffer_as_hexwkb(const Cbuffer *cb, uint8_t variant, size_t *size); */
  char_result = cbuffer_as_hexwkb(cbuffer1, 1, &size);
  printf("cbuffer_as_hexwkb(%s, 1, %ld): %s\n", cbuffer1_out, size, char_result);
  free(char_result);

  /* char *cbuffer_as_text(const Cbuffer *cb, int maxdd); */
  char_result = cbuffer_as_text(cbuffer1, 6);
  printf("cbuffer_as_text(%s, 6): %s\n", cbuffer1_out, char_result);
  free(char_result);

  // /* uint8_t *cbuffer_as_wkb(const Cbuffer *cb, uint8_t variant, size_t *size_out); */
  // binchar_result = cbuffer_as_wkb(const Cbuffer *cb, uint8_t variant, size_t *size_out);

  // /* Cbuffer *cbuffer_from_hexwkb(const char *hexwkb); */
  // cbuffer_result = cbuffer_from_hexwkb(const char *hexwkb);
  // char_result = cbuffer_as_text(cbuffer_result, 6);
  // printf("cbuffer_from_hexwkb(%s): %s\n", xxx, char_result);
  // free(cbuffer_result); free(char_result);

  // /* Cbuffer *cbuffer_from_wkb(const uint8_t *wkb, size_t size); */
  // cbuffer_result = cbuffer_from_wkb(const uint8_t *wkb, size);
  // char_result = cbuffer_as_text(cbuffer_result, 6);
  // printf("cbuffer_from_wkb(%s): %s\n", xxx, char_result);
  // free(cbuffer_result); free(char_result);

  /* Cbuffer *cbuffer_in(const char *str); */
  cbuffer_result = cbuffer_in(cbuffer1_in);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbuffer_in(%s): %s\n", cbuffer1_in, char_result);
  free(cbuffer_result); free(char_result);

  /* char *cbuffer_as_text(const Cbuffer *cb, int maxdd); */
  char_result = cbuffer_as_text(cbuffer1, 6);
  printf("cbuffer_as_text(%s, 6): %s\n", cbuffer1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Cbuffer *cbuffer_copy(const Cbuffer *cb); */
  cbuffer_result = cbuffer_copy(cbuffer1);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbuffer_copy(%s): %s\n", cbuffer1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Cbuffer *cbuffer_make(const GSERIALIZED *point, double radius); */
  cbuffer_result = cbuffer_make(geom1, float8_in1);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbuffer_make(%s, %lf): %s\n", geom1_out, float8_in1, char_result);
  free(cbuffer_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* GSERIALIZED *cbuffer_to_geom(const Cbuffer *cb); */
  geom_result = cbuffer_to_geom(cbuffer1);
  char_result = geo_as_text(geom_result, 6);
  printf("cbuffer_to_geom(%s): %s\n", cbuffer1_out, char_result);
  free(geom_result); free(char_result);

  /* STBox *cbuffer_to_stbox(const Cbuffer *cb); */
  stbox_result = cbuffer_to_stbox(cbuffer1);
  char_result = stbox_out(stbox_result, 6);
  printf("cbuffer_to_stbox(%s): %s\n", cbuffer1_out, char_result);
  free(stbox_result); free(char_result);

  /* GSERIALIZED *cbufferarr_to_geom(const Cbuffer **cbarr, int count); */
  cbufferarray[0] = cbuffer1;
  cbufferarray[1] = cbuffer2;
  geom_result = cbufferarr_to_geom((const Cbuffer **) cbufferarray, 2);
  char_result = geo_as_text(geom_result, 6);
  printf("cbufferarr_to_geom({%s, %s}): %s\n", cbuffer1_out, cbuffer2_out, char_result);
  free(geom_result); free(char_result);

  /* Cbuffer *geom_to_cbuffer(const GSERIALIZED *gs); */
  cbuffer_result = geom_to_cbuffer(geom1);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("geom_to_cbuffer(%s): %s\n", geom1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* uint32 cbuffer_hash(const Cbuffer *cb); */
  uint32_result = cbuffer_hash(cbuffer1);
  printf("cbuffer_hash(%s, true): %u\n", cbuffer1_out, uint32_result);

  /* uint64 cbuffer_hash_extended(const Cbuffer *cb, uint64 seed); */
  uint64_result = cbuffer_hash_extended(cbuffer1, 1);
  printf("cbuffer_hash_extended(%s, 1): %lu\n", cbuffer1_out, uint64_result);

  /* GSERIALIZED *cbuffer_point(const Cbuffer *cb); */
  geom_result = cbuffer_point(cbuffer1);
  char_result = geo_as_text(geom_result, 6);
  printf("cbuffer_point(%s): %s\n", cbuffer1_out, char_result);
  free(geom_result); free(char_result);

  /* double cbuffer_radius(const Cbuffer *cb); */
  float8_result = cbuffer_radius(cbuffer1);
  printf("cbuffer_radius(%s, true): %lf\n", cbuffer1_out, float8_result);

  /* Transformation functions */
  printf("****************************************************************\n");

  /* Cbuffer *cbuffer_round(const Cbuffer *cb, int maxdd); */
  cbuffer_result = cbuffer_round(cbuffer1, 6);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbuffer_round(%s): %s\n", cbuffer1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Cbuffer **cbufferarr_round(const Cbuffer **cbarr, int count, int maxdd); */
  cbufferarray[0] = cbuffer1;
  cbufferarray[1] = cbuffer2;
  cbufferarray_result = cbufferarr_round((const Cbuffer**) cbufferarray, 2, 6);
  printf("cbufferarr_round({%s, %s}): {", cbuffer1_out, cbuffer2_out);
  for (int i = 0; i < cbufferset1->count; i++)
  {
    char_result = cbuffer_as_text(cbufferarray_result[i], 6);
    printf("%s", char_result);
    if (i < cbufferset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(cbufferarray_result[i]);
    free(char_result);
  }
  free(cbufferarray_result);

  /* Spatial reference system functions */
  printf("****************************************************************\n");

  // /* void cbuffer_set_srid(Cbuffer *cb, int32_t srid); */
  // void cbuffer_set_srid(cbuffer1, 3857); // TODO

  /* int32_t cbuffer_srid(const Cbuffer *cb); */
  int32_result = cbuffer_srid(cbuffer1);
  printf("cbuffer_srid(%s, true): %d\n", cbuffer1_out, int32_result);

  /* Cbuffer *cbuffer_transform(const Cbuffer *cb, int32_t srid); */
  cbuffer_result = cbuffer_transform(cbuffer1, 3857);
  char_result = cbuffer_as_text(cbuffer_result, 3857);
  printf("cbuffer_transform(%s): %s\n", cbuffer1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Cbuffer *cbuffer_transform_pipeline(const Cbuffer *cb, const char *pipelinestr, int32_t srid, bool is_forward); */
  cbuffer_result = cbuffer_transform_pipeline(cbuffer1, "urn:ogc:def:coordinateOperation:EPSG::16031", 16031, true);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbuffer_transform_pipeline(%s, \"urn:ogc:def:coordinateOperation:EPSG::16031\", 16031, true): %s\n", cbuffer1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Spatial relationship functions */
  printf("****************************************************************\n");

  /* int contains_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = contains_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("contains_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* int covers_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = covers_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("covers_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* int disjoint_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = disjoint_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("disjoint_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* int dwithin_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2, double dist); */
  int32_result = dwithin_cbuffer_cbuffer(cbuffer1, cbuffer2, float8_in1);
  printf("dwithin_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* int intersects_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = intersects_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("intersects_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* int touches_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = touches_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("touches_cbuffer_cbuffer(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* Bounding box functions */
  printf("****************************************************************\n");

  /* STBox *cbuffer_tstzspan_to_stbox(const Cbuffer *cb, const Span *s); */
  stbox_result = cbuffer_tstzspan_to_stbox(cbuffer1, tstzspan1);
  char_result = stbox_out(stbox_result, 6);
  printf("cbuffer_tstzspan_to_stbox(%s, %s): %s\n", cbuffer1_out, tstzspan1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *cbuffer_timestamptz_to_stbox(const Cbuffer *cb, TimestampTz t); */
  stbox_result = cbuffer_timestamptz_to_stbox(cbuffer1, tstz1);
  char_result = stbox_out(stbox_result, 6);
  printf("cbuffer_timestamptz_to_stbox(%s, %s): %s\n", cbuffer1_out, tstz1_out, char_result);
  free(stbox_result); free(char_result);

  /* Distance functions */
  printf("****************************************************************\n");

  /* double distance_cbuffer_cbuffer(const Cbuffer *cb1, const Cbuffer *cb2); */
  float8_result = distance_cbuffer_cbuffer(cbuffer1, cbuffer2);
  printf("distance_cbuffer_cbuffer(%s, %s): %lf\n", cbuffer1_out, cbuffer2_out, float8_result);

  /* double distance_cbuffer_geo(const Cbuffer *cb, const GSERIALIZED *gs); */
  float8_result = distance_cbuffer_geo(cbuffer1, geom1);
  printf("distance_cbuffer_geo(%s, %s): %lf\n", cbuffer1_out, geom1_out, float8_result);

  /* double distance_cbuffer_stbox(const Cbuffer *cb, const STBox *box); */
  float8_result = distance_cbuffer_stbox(cbuffer1, stbox1);
  printf("distance_cbuffer_stbox(%s, %s): %lf\n", cbuffer1_out, stbox1_out, float8_result);

  /* double nad_cbuffer_stbox(const Cbuffer *cb, const STBox *box); */
  float8_result = nad_cbuffer_stbox(cbuffer1, stbox1);
  printf("nad_cbuffer_stbox(%s, %s): %lf\n", cbuffer1_out, stbox1_out, float8_result);

  /* Comparison functions */
  printf("****************************************************************\n");

  /* int cbuffer_cmp(const Cbuffer *cb1, const Cbuffer *cb2); */
  int32_result = cbuffer_cmp(cbuffer1, cbuffer2);
  printf("cbuffer_cmp(%s, %s): %d\n", cbuffer1_out, cbuffer2_out, int32_result);

  /* bool cbuffer_eq(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_eq(cbuffer1, cbuffer2);
  printf("cbuffer_eq(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_ge(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_ge(cbuffer1, cbuffer2);
  printf("cbuffer_ge(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_gt(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_gt(cbuffer1, cbuffer2);
  printf("cbuffer_gt(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_le(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_le(cbuffer1, cbuffer2);
  printf("cbuffer_le(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_lt(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_lt(cbuffer1, cbuffer2);
  printf("cbuffer_lt(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_ne(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_ne(cbuffer1, cbuffer2);
  printf("cbuffer_ne(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_nsame(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_nsame(cbuffer1, cbuffer2);
  printf("cbuffer_nsame(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /* bool cbuffer_same(const Cbuffer *cb1, const Cbuffer *cb2); */
  bool_result = cbuffer_same(cbuffer1, cbuffer2);
  printf("cbuffer_same(%s, %s): %c\n", cbuffer1_out, cbuffer2_out, bool_result ? 't' : 'n');

  /******************************************************************************
   * Functions for circular buffer sets
   ******************************************************************************/

  /* Input and output functions */
  printf("****************************************************************\n");

  /* Set *cbufferset_in(const char *str); */
  cbufferset_result = cbufferset_in(cbufferset1_in);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("cbufferset_in(%s): %s\n", cbufferset1_in, char_result);
  free(cbufferset_result); free(char_result);

  /* char *spatialset_as_text(const Set *s, int maxdd); */
  char_result = spatialset_as_text(cbufferset1, 6);
  printf("spatialset_as_text(%s, 6): %s\n", cbufferset1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Set *cbufferset_make(Cbuffer **values, int count); */
  cbufferarray[0] = cbuffer1;
  cbufferarray[1] = cbuffer2;
  cbufferset_result = cbufferset_make(cbufferarray, 2);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("cbufferset_make({%s, %s}): %s\n", cbuffer1_out, cbuffer2_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* Set *cbuffer_to_set(const Cbuffer *cb); */
  cbufferset_result = cbuffer_to_set(cbuffer1);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("cbuffer_to_set(%s): %s\n", cbuffer1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* Cbuffer *cbufferset_end_value(const Set *s); */
  cbuffer_result = cbufferset_end_value(cbufferset1);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbufferset_end_value(%s): %s\n", cbufferset1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* Cbuffer *cbufferset_start_value(const Set *s); */
  cbuffer_result = cbufferset_start_value(cbufferset1);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbufferset_start_value(%s): %s\n", cbufferset1_out, char_result);
  free(cbuffer_result); free(char_result);

  /* bool cbufferset_value_n(const Set *s, int n, Cbuffer **result); */
  bool_result = cbufferset_value_n(cbufferset1, 1, &cbuffer_result);
  char_result = cbuffer_as_text(cbuffer_result, 6);
  printf("cbufferset_value_n(%s, 1, %s): %c\n", cbufferset1_out, cbuffer1_out, bool_result ? 't' : 'n');
  free(cbuffer_result); free(char_result);

  /* Cbuffer **cbufferset_values(const Set *s); */
  cbufferarray_result = cbufferset_values(cbufferset1);
  printf("cbufferset_values(%s): {", cbufferset1_out);
  for (int i = 0; i < cbufferset1->count; i++)
  {
    char_result = cbuffer_as_text(cbufferarray_result[i], 6);
    printf("%s", char_result);
    if (i < cbufferset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(cbufferarray_result[i]);
    free(char_result);
  }
  free(cbufferarray_result);

  /* Set operations */
  printf("****************************************************************\n");

  // /* Set *cbuffer_union_transfn(Set *state, const Cbuffer *cb); */
  // cbufferset_result = cbuffer_union_transfn(cbufferset1, cbuffer1);
  // char_result = spatialset_as_text(cbufferset_result, 6);
  // printf("cbuffer_union(%s, %s): %s\n", cbuffer_union_transfn, char_result);
  // free(cbufferset_result); free(char_result);

  /* bool contained_cbuffer_set(const Cbuffer *cb, const Set *s); */
  bool_result = contained_cbuffer_set(cbuffer1, cbufferset1);
  printf("contained_cbuffer_set(%s, %s): %c\n", cbuffer1_out, cbufferset1_out, bool_result ? 't' : 'n');

  /* bool contains_set_cbuffer(const Set *s, Cbuffer *cb); */
  bool_result = contains_set_cbuffer(cbufferset1, cbuffer1);
  printf("contains_set_cbuffer(%s, %s): %c\n", cbufferset1_out, cbuffer1_out, bool_result ? 't' : 'n');

  /* Set *intersection_cbuffer_set(const Cbuffer *cb, const Set *s); */
  cbufferset_result = intersection_cbuffer_set(cbuffer1, cbufferset1);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("intersection_cbuffer_set(%s, %s): %s\n", cbuffer1_out, cbufferset1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Set *intersection_set_cbuffer(const Set *s, const Cbuffer *cb); */
  cbufferset_result = intersection_set_cbuffer(cbufferset1, cbuffer1);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("intersection_set_cbuffer(%s, %s): %s\n", cbuffer1_out, cbufferset1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Set *minus_cbuffer_set(const Cbuffer *cb, const Set *s); */
  cbufferset_result = minus_cbuffer_set(cbuffer1, cbufferset1);
  char_result = cbufferset_result ? 
    spatialset_as_text(cbufferset_result, 6) : text_out(text_null);
  printf("minus_cbuffer_set(%s, %s): %s\n", cbuffer1_out, cbufferset1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Set *minus_set_cbuffer(const Set *s, const Cbuffer *cb); */
  cbufferset_result = minus_set_cbuffer(cbufferset1, cbuffer1);
  char_result = cbufferset_result ? 
    spatialset_as_text(cbufferset_result, 6) : text_out(text_null);
  printf("minus_set_cbuffer(%s, %s): %s\n", cbufferset1_out, cbuffer1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Set *union_cbuffer_set(const Cbuffer *cb, const Set *s); */
  cbufferset_result = union_cbuffer_set(cbuffer1, cbufferset1);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("union_cbuffer_set(%s, %s): %s\n", cbuffer1_out, cbufferset1_out, char_result);
  free(cbufferset_result); free(char_result);

  /* Set *union_set_cbuffer(const Set *s, const Cbuffer *cb); */
  cbufferset_result = union_set_cbuffer(cbufferset1, cbuffer1);
  char_result = spatialset_as_text(cbufferset_result, 6);
  printf("union_set_cbuffer(%s, %s): %s\n", cbufferset1_out, cbuffer1_out, char_result);
  free(cbufferset_result); free(char_result);

  /*===========================================================================*
   * Functions for temporal types
   *===========================================================================*/

  /*****************************************************************************
   * Input/output functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tcbuffer_in(const char *str); */
  tcbuffer_result = tcbuffer_in(tcbuffer1_in);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_in(%s): %s\n", tcbuffer1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /*****************************************************************************
   * Constructor functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tcbuffer_make(const Temporal *tpoint, const Temporal *tfloat); */
  tcbuffer_result = tcbuffer_make(tgeompt1, tfloat1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_make(%s, %s): %s\n", tgeompt1_out, tfloat1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /*****************************************************************************
   * Accessor functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Set *tcbuffer_points(const Temporal *temp); */
  geomset_result = tcbuffer_points(tcbuffer1);
  char_result = spatialset_as_text(geomset_result, 6);
  printf("tcbuffer_points(%s, 6): %s", tcbuffer1_out, char_result);
  free(geomset_result); free(char_result);

  /* Set *tcbuffer_radius(const Temporal *temp); */
  floatset_result = tcbuffer_radius(tcbuffer1);
  char_result = floatset_out(floatset_result, 6);
  printf("tcbuffer_points(%s, 6): %s", tcbuffer1_out, char_result);
  free(floatset_result); free(char_result);

  /* GSERIALIZED *tcbuffer_trav_area(const Temporal *temp, bool merge_union); */
  geom_result = tcbuffer_trav_area(tcbuffer1, true);
  char_result = geo_as_text(geom_result, 6);
  printf("tcbuffer_trav_area(%s, true): %s\n", tcbuffer1_out, char_result);
  free(geom_result); free(char_result);

  /*****************************************************************************
   * Conversion functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tcbuffer_to_tfloat(const Temporal *temp); */
  tfloat_result = tcbuffer_to_tfloat(tcbuffer1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tcbuffer_to_tfloat(%s, 6): %s\n", tcbuffer1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tcbuffer_to_tgeompoint(const Temporal *temp); */
  tgeompt_result = tcbuffer_to_tgeompoint(tcbuffer1);
  char_result = tspatial_as_text(tgeompt_result, 6);
  printf("tcbuffer_to_tgeompoint(%s, 6): %s\n", tcbuffer1_out, char_result);
  free(tgeompt_result); free(char_result);

  /* Temporal *tgeometry_to_tcbuffer(const Temporal *temp); */
  tcbuffer_result = tgeometry_to_tcbuffer(tgeompt1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tgeometry_to_tcbuffer(%s): %s\n", tgeompt1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /*****************************************************************************
   * Transformation functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tcbuffer_expand(const Temporal *temp, double dist); */
  tcbuffer_result = tcbuffer_expand(tcbuffer1, float8_in1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_expand(%s, %lf): %s\n", tcbuffer1_out, float8_in1, char_result);
  free(tcbuffer_result); free(char_result);

  /*****************************************************************************
   * Restriction functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tcbuffer_at_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tcbuffer_result = tcbuffer_at_cbuffer(tcbuffer1, cbuffer1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_at_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* Temporal *tcbuffer_at_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tcbuffer_result = tcbuffer_at_geom(tcbuffer1, geom1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_at_geom(%s, %s): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* Temporal *tcbuffer_at_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tcbuffer_result = tcbuffer_at_stbox(tcbuffer1, stbox1, true);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("tcbuffer_at_stbox(%s, %s, true): %s\n", tcbuffer1_out, stbox1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* Temporal *tcbuffer_minus_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tcbuffer_result = tcbuffer_minus_cbuffer(tcbuffer1, cbuffer1);
  char_result = tcbuffer_result ? tspatial_as_text(tcbuffer_result, 6) : text_out(text_null);
  printf("tcbuffer_minus_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  if (tcbuffer_result)
    free(tcbuffer_result);
  free(char_result);

  /* Temporal *tcbuffer_minus_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tcbuffer_result = tcbuffer_minus_geom(tcbuffer1, geom1);
  char_result = tcbuffer_result ? tspatial_as_text(tcbuffer_result, 6) : text_out(text_null);
  printf("tcbuffer_minus_geom(%s, %s): %s\n", tcbuffer1_out, geom1_out, char_result);
  if (tcbuffer_result)
    free(tcbuffer_result);
  free(char_result);

  /* Temporal *tcbuffer_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tcbuffer_result = tcbuffer_minus_stbox(tcbuffer1, stbox1, true);
  char_result = tcbuffer_result ? tspatial_as_text(tcbuffer_result, 6) : text_out(text_null);
  printf("tcbuffer_minus_stbox(%s, %s, true): %s\n", tcbuffer1_out, stbox1_out, char_result);
  if (tcbuffer_result)
    free(tcbuffer_result);
  free(char_result);

  /*****************************************************************************
   * Distance functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tdistance_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tfloat_result = tdistance_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tcbuffer_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tfloat_result = tdistance_tcbuffer_geo(tcbuffer1, geom1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tcbuffer_geo(%s, %s): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  tfloat_result = tdistance_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tcbuffer_tcbuffer(%s, %s): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tfloat_result); free(char_result);

  /* double nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  float8_result = nad_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("nad_tcbuffer_cbuffer(%s, %s): %lf\n", tcbuffer1_out, cbuffer1_out, float8_result);

  /* double nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  float8_result = nad_tcbuffer_geo(tcbuffer1, geom1);
  printf("nad_tcbuffer_geo(%s, %s): %lf\n", tcbuffer1_out, geom1_out, float8_result);

  /* double nad_tcbuffer_stbox(const Temporal *temp, const STBox *box); */
  float8_result = nad_tcbuffer_stbox(tcbuffer1, stbox1);
  printf("nad_tcbuffer_stbox(%s, %s): %lf\n", tcbuffer1_out, stbox1_out, float8_result);

  /* double nad_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  float8_result = nad_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("nad_tcbuffer_tcbuffer(%s, %s): %lf\n", tcbuffer1_out, tcbuffer2_out, float8_result);

  /* TInstant *nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tcbuffer_result = (Temporal *) nai_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("nai_tcbuffer_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* TInstant *nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tcbuffer_result = (Temporal *) nai_tcbuffer_geo(tcbuffer1, geom1);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("nai_tcbuffer_geo(%s, %s): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* TInstant *nai_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  tcbuffer_result = (Temporal *) nai_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  char_result = tspatial_as_text(tcbuffer_result, 6);
  printf("nai_tcbuffer_tcbuffer(%s, %s): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tcbuffer_result); free(char_result);

  /* GSERIALIZED *shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  geom_result = shortestline_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tcbuffer_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *shortestline_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  geom_result = shortestline_tcbuffer_geo(tcbuffer1, geom1);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tcbuffer_geo(%s, %s): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *shortestline_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  geom_result = shortestline_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tcbuffer_tcbuffer(%s, %s): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(geom_result); free(char_result);

  /*****************************************************************************
   * Comparison functions
   *****************************************************************************/

  /* Ever/always comparison functions */
  printf("****************************************************************\n");

  /* int always_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = always_eq_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("always_eq_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int always_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = always_eq_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("always_eq_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int always_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_eq_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("always_eq_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int always_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = always_ne_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("always_ne_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int always_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = always_ne_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("always_ne_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int always_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_ne_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("always_ne_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int ever_eq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = ever_eq_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("ever_eq_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int ever_eq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = ever_eq_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("ever_eq_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int ever_eq_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_eq_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("ever_eq_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int ever_ne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = ever_ne_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("ever_ne_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int ever_ne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = ever_ne_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("ever_ne_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int ever_ne_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_ne_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("ever_ne_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* Temporal comparison functions */
  printf("****************************************************************\n");

  /* Temporal *teq_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  tbool_result = teq_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  char_result = tbool_out(tbool_result);
  printf("teq_cbuffer_tcbuffer(%s, %s): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *teq_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tbool_result = teq_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  char_result = tbool_out(tbool_result);
  printf("teq_tcbuffer_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  tbool_result = tne_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  char_result = tbool_out(tbool_result);
  printf("tne_cbuffer_tcbuffer(%s, %s): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  tbool_result = tne_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  char_result = tbool_out(tbool_result);
  printf("tne_tcbuffer_cbuffer(%s, %s): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /*****************************************************************************
   * Spatial relationship functions
   *****************************************************************************/

  /* Ever and always spatial relationship functions */
  printf("****************************************************************\n");

  /* int acontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = acontains_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("acontains_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int acontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = acontains_geo_tcbuffer(geom1, tcbuffer1);
  printf("acontains_geo_tcbuffer(%s, %s): %d\n", geom1_out, tcbuffer1_out, int32_result);

  /* int acontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = acontains_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("acontains_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int acontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = acontains_tcbuffer_geo(tcbuffer1, geom1);
  printf("acontains_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int acovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = acovers_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("acovers_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int acovers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp); */
  int32_result = acovers_geo_tcbuffer(geom1, tcbuffer1);
  printf("acovers_geo_tcbuffer(%s, %s): %d\n", geom1_out, tcbuffer1_out, int32_result);

  /* int acovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = acovers_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("acovers_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int acovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = acovers_tcbuffer_geo(tcbuffer1, geom1);
  printf("acovers_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int adisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = adisjoint_tcbuffer_geo(tcbuffer1, geom1);
  printf("adisjoint_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int adisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = adisjoint_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("adisjoint_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int adisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = adisjoint_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("adisjoint_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int adwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist); */
  int32_result = adwithin_tcbuffer_geo(tcbuffer1, geom1, float8_in1);
  printf("adwithin_tcbuffer_geo(%s, %s, %lf): %d\n", tcbuffer1_out, geom1_out, float8_in1, int32_result);

  /* int adwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist); */
  int32_result = adwithin_tcbuffer_cbuffer(tcbuffer1, cbuffer1, float8_in1);
  printf("adwithin_tcbuffer_cbuffer(%s, %s, %lf): %d\n", tcbuffer1_out, cbuffer1_out, float8_in1, int32_result);

  /* int adwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist); */
  int32_result = adwithin_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, float8_in1);
  printf("adwithin_tcbuffer_tcbuffer(%s, %s, %lf): %d\n", tcbuffer1_out, tcbuffer2_out, float8_in1, int32_result);

  /* int aintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = aintersects_tcbuffer_geo(tcbuffer1, geom1);
  printf("aintersects_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int aintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = aintersects_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("aintersects_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int aintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = aintersects_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("aintersects_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int atouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = atouches_tcbuffer_geo(tcbuffer1, geom1);
  printf("atouches_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int atouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = atouches_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("atouches_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int atouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = atouches_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("atouches_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int econtains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = econtains_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("econtains_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int econtains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = econtains_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("econtains_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int econtains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = econtains_tcbuffer_geo(tcbuffer1, geom1);
  printf("econtains_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int ecovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp); */
  int32_result = ecovers_cbuffer_tcbuffer(cbuffer1, tcbuffer1);
  printf("ecovers_cbuffer_tcbuffer(%s, %s): %d\n", cbuffer1_out, tcbuffer1_out, int32_result);

  /* int ecovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = ecovers_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("ecovers_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int ecovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = ecovers_tcbuffer_geo(tcbuffer1, geom1);
  printf("ecovers_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int ecovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ecovers_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("ecovers_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int edisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = edisjoint_tcbuffer_geo(tcbuffer1, geom1);
  printf("edisjoint_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int edisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = edisjoint_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("edisjoint_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int edwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist); */
  int32_result = edwithin_tcbuffer_geo(tcbuffer1, geom1, float8_in1);
  printf("edwithin_tcbuffer_geo(%s, %s, %lf): %d\n", tcbuffer1_out, geom1_out, float8_in1, int32_result);

  /* int edwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist); */
  int32_result = edwithin_tcbuffer_cbuffer(tcbuffer1, cbuffer1, float8_in1);
  printf("edwithin_tcbuffer_cbuffer(%s, %s, %lf): %d\n", tcbuffer1_out, cbuffer1_out, float8_in1, int32_result);

  /* int edwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist); */
  int32_result = edwithin_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, float8_in1);
  printf("edwithin_tcbuffer_tcbuffer(%s, %s, %lf): %d\n", tcbuffer1_out, tcbuffer2_out, float8_in1, int32_result);

  /* int eintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = eintersects_tcbuffer_geo(tcbuffer1, geom1);
  printf("eintersects_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int eintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = eintersects_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("eintersects_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int eintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = eintersects_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("eintersects_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* int etouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs); */
  int32_result = etouches_tcbuffer_geo(tcbuffer1, geom1);
  printf("etouches_tcbuffer_geo(%s, %s): %d\n", tcbuffer1_out, geom1_out, int32_result);

  /* int etouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb); */
  int32_result = etouches_tcbuffer_cbuffer(tcbuffer1, cbuffer1);
  printf("etouches_tcbuffer_cbuffer(%s, %s): %d\n", tcbuffer1_out, cbuffer1_out, int32_result);

  /* int etouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2); */
  int32_result = etouches_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2);
  printf("etouches_tcbuffer_tcbuffer(%s, %s): %d\n", tcbuffer1_out, tcbuffer2_out, int32_result);

  /* Spatiotemporal relationship functions */
  printf("****************************************************************\n");

  /* Temporal *tcontains_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcontains_cbuffer_tcbuffer(cbuffer1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_cbuffer_tcbuffer(%s, %s, true, true): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcontains_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcontains_geo_tcbuffer(geom1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_geo_tcbuffer(%s, %s, true, true): %s\n", geom1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcontains_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tcontains_tcbuffer_geo(tcbuffer1, geom1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_tcbuffer_geo(%s, %s, true, true): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcontains_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue); */
  tbool_result = tcontains_tcbuffer_cbuffer(tcbuffer1, cbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcontains_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tcontains_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcontains_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcovers_cbuffer_tcbuffer(cbuffer1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcovers_cbuffer_tcbuffer(%s, %s, true, true): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = tcovers_geo_tcbuffer(geom1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcovers_geo_tcbuffer(%s, %s, true, true): %s\n", geom1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tcovers_tcbuffer_geo(tcbuffer1, geom1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcovers_tcbuffer_geo(%s, %s, true, true): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue); */
  tbool_result = tcovers_tcbuffer_cbuffer(tcbuffer1, cbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcovers_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tcovers_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tcovers_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tcovers_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_geo_tcbuffer(geom1, tcbuffer1, float8_in1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdwithin_geo_tcbuffer(%s, %s, %lf, true, true): %s\n", geom1_out, tcbuffer1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_tcbuffer_geo(tcbuffer1, geom1, float8_in1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdwithin_tcbuffer_geo(%s, %s, %lf, true, true): %s\n", tcbuffer1_out, geom1_out, float8_in1, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_tcbuffer_cbuffer(tcbuffer1, cbuffer1, float8_in1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdwithin_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdwithin_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue); */
  tbool_result = tdwithin_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, float8_in1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdwithin_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  // /* Temporal *tdisjoint_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,bool restr, bool atvalue); */
  // tbool_result = tdisjoint_cbuffer_tcbuffer(cbuffer1, tcbuffer1, true, true);
  // char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  // printf("tdisjoint_cbuffer_tcbuffer(%s, %s, true, true): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  // free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,bool restr, bool atvalue); */
  tbool_result = tdisjoint_geo_tcbuffer(geom1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdisjoint_geo_tcbuffer(%s, %s, true, true): %s\n", geom1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tdisjoint_tcbuffer_geo(tcbuffer1, geom1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdisjoint_tcbuffer_geo(%s, %s, true, true): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue); */
  tbool_result = tdisjoint_tcbuffer_cbuffer(tcbuffer1, cbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdisjoint_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tdisjoint_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tdisjoint_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tdisjoint_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp,bool restr, bool atvalue); */
  tbool_result = tintersects_cbuffer_tcbuffer(cbuffer1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tintersects_cbuffer_tcbuffer(%s, %s, true, true): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp,bool restr, bool atvalue); */
  tbool_result = tintersects_geo_tcbuffer(geom1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tintersects_geo_tcbuffer(%s, %s, true, true): %s\n", geom1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = tintersects_tcbuffer_geo(tcbuffer1, geom1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tintersects_tcbuffer_geo(%s, %s, true, true): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue); */
  tbool_result = tintersects_tcbuffer_cbuffer(tcbuffer1, cbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tintersects_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tintersects_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = tintersects_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("tintersects_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_geo_tcbuffer(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = ttouches_geo_tcbuffer(geom1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("ttouches_geo_tcbuffer(%s, %s, true, true): %s\n", geom1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue); */
  tbool_result = ttouches_tcbuffer_geo(tcbuffer1, geom1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("ttouches_tcbuffer_geo(%s, %s, true, true): %s\n", tcbuffer1_out, geom1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_cbuffer_tcbuffer(const Cbuffer *cb, const Temporal *temp, bool restr, bool atvalue); */
  tbool_result = ttouches_cbuffer_tcbuffer(cbuffer1, tcbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("ttouches_cbuffer_tcbuffer(%s, %s, true, true): %s\n", cbuffer1_out, tcbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb, bool restr, bool atvalue); */
  tbool_result = ttouches_tcbuffer_cbuffer(tcbuffer1, cbuffer1, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("ttouches_tcbuffer_cbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, cbuffer1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *ttouches_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue); */
  tbool_result = ttouches_tcbuffer_tcbuffer(tcbuffer1, tcbuffer2, true, true);
  char_result = tbool_result ? tbool_out(tbool_result) : text_out(text_null);
  printf("ttouches_tcbuffer_tcbuffer(%s, %s, true, true): %s\n", tcbuffer1_out, tcbuffer2_out, char_result);
  free(tbool_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */

  free(tstz1_out);
  free(tstzspan1); free(tstzspan1_out);
  free(text_null);
  free(cbuffer1); free(cbuffer1_out);
  free(cbuffer2); free(cbuffer2_out);
  free(cbufferset1);
  free(cbufferset1_out);
  free(geom1); free(geom1_out);
  free(tfloat1); free(tfloat1_out);
  free(stbox1); free(stbox1_out);
  free(tgeompt1); free(tgeompt1_out);
  free(tcbuffer1); free(tcbuffer1_out);
  free(tcbuffer2); free(tcbuffer2_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
