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
 * gcc -Wall -g -I/usr/local/include -o npoint_test npoint_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <pg_bool.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */

  int64_t int64_in1 = 1;
  double float8_in1 = 1;
  double float8_in2 = 1;
  int count;
  size_t size;

  text *text_null = text_in("NULL");

  char *tstz1_in = "2001-01-01";
  TimestampTz tstz1 = timestamptz_in(tstz1_in, -1);
  char *tstz1_out = timestamptz_out(tstz1);
  char *tstz2_in = "2001-01-03";
  TimestampTz tstz2 = timestamptz_in(tstz2_in, -1);
  char *tstz2_out = timestamptz_out(tstz2);
  
  char *tstzset1_in = "{2001-01-01, 2001-01-03}";
  Set *tstzset1 = tstzset_in(tstzset1_in);
  char *tstzset1_out = tstzset_out(tstzset1);

  char *tstzspan1_in = "[2001-01-01, 2001-01-03]";
  Span *tstzspan1 = tstzspan_in(tstzspan1_in);
  char *tstzspan1_out = tstzspan_out(tstzspan1);

  char *tstzspanset1_in = "{[2001-01-01, 2001-01-03],[2001-01-04, 2001-01-05]}";
  SpanSet *tstzspanset1 = tstzspanset_in(tstzspanset1_in);
  char *tstzspanset1_out = tstzspanset_out(tstzspanset1);

  char *tgeompt1_in = "SRID=5676;[Point(1 1)@2001-01-01, Point(3 3)@2001-01-03]";
  Temporal *tgeompt1 = tgeompoint_in(tgeompt1_in);
  char *tgeompt1_out = tspatial_as_ewkt(tgeompt1, 6);

  char *npoint1_in = "Npoint(1, 0.5)";
  Npoint *npoint1 = npoint_in(npoint1_in);
  char *npoint1_out = npoint_out(npoint1, 6);
  uint8_t *npoint1_wkb = npoint_as_wkb(npoint1, 1, &size);
  char *npoint1_hexwkb = npoint_as_hexwkb(npoint1, 1, &size);

  char *npoint2_in = "Npoint(2, 0.5)";
  Npoint *npoint2 = npoint_in(npoint2_in);
  char *npoint2_out = npoint_out(npoint2, 6);

  Npoint *npointarray[2];

  char *npointset1_in = "{\"Npoint(2, 0.5)\", \"Npoint(2, 0.5)\"}";
  Set *npointset1 = npointset_in(npointset1_in);
  char *npointset1_out = npointset_out(npointset1, 6);

  char *nsegment1_in = "Nsegment(1, 0.3, 0.5)";
  Nsegment *nsegment1 = nsegment_in(nsegment1_in);
  char *nsegment1_out = nsegment_out(nsegment1, 6);

  char *nsegment2_in = "Nsegment(2, 0.4, 0.6)";
  Nsegment *nsegment2 = nsegment_in(nsegment2_in);
  char *nsegment2_out = nsegment_out(nsegment2, 6);

  char *geom1_in = "SRID=5676;Point(1 1)";
  GSERIALIZED *geom1 = geom_in(geom1_in, -1);
  char *geom1_out = geo_as_text(geom1, 6);
  char *geom2_in = "SRID=5676;Polygon((1 1,1 20,20 20,20 1,1 1))";
  GSERIALIZED *geom2 = geom_in(geom2_in, -1);
  char *geom2_out = geo_as_text(geom2, 6);

  char *stbox1_in = "SRID=5676;STBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])";
  STBox *stbox1 = stbox_in(stbox1_in);
  char *stbox1_out = stbox_out(stbox1, 6);

  char *tnpoint1_in = "[NPoint(1, 0.3)@2001-01-01, NPoint(1, 0.5)@2001-01-03]";
  Temporal *tnpoint1 = tnpoint_in(tnpoint1_in);
  char *tnpoint1_out = tnpoint_out(tnpoint1, 6);

  char *tnpoint2_in = "[NPoint(2, 0.3)@2001-01-01, NPoint(2, 0.5)@2001-01-03]";
  Temporal *tnpoint2 = tnpoint_in(tnpoint2_in);
  char *tnpoint2_out = tnpoint_out(tnpoint2, 6);

  /* Create the result types for the functions of the API */

  bool bool_result;
  int32_t int32_result;
  double float8_result;
  int64_t int64_result;
  uint32_t uint32_result;
  uint64_t uint64_result;
  char *char_result;

  GSERIALIZED *geom_result;
  STBox *stbox_result;
  Npoint *npoint_result;
  Nsegment *nsegment_result;
  Set *bigintset_result;
  Set *npointset_result;
  Temporal *tbool_result;
  Temporal *tfloat_result;
  Temporal *tgeompt_result;
  Temporal *tnpoint_result;
  Npoint **npointarray_result;
  Nsegment **nsegmentarray_result;

  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Network point types *\n");
  printf("****************************************************************\n");

  /* Input and output functions */
  printf("****************************************************************\n");

  /* char *npoint_as_ewkt(const Npoint *np, int maxdd); */
  char_result = npoint_as_ewkt(npoint1, 6);
  printf("npoint_as_ewkt(%s, 6): %s\n", npoint1_out, char_result);
  free(char_result);

  /* char *npoint_as_hexwkb(const Npoint *np, uint8_t variant, size_t *size_out); */
  char_result = npoint_as_hexwkb(npoint1, 1, &size);
  printf("npoint_as_hexwkb(%s, 1, %ld): %s\n", npoint1_out, size, char_result);
  free(char_result);

  /* char *npoint_as_text(const Npoint *np, int maxdd); */
  char_result = npoint_as_text(npoint1, 6);
  printf("npoint_as_text(%s, 6): %s\n", npoint1_out, char_result);
  free(char_result);

  /* uint8_t *npoint_as_wkb(const Npoint *np, uint8_t variant, size_t *size_out); */
  // binchar_result = npoint_as_wkb(npoint1, 1, size_t *size_out);

  /* Npoint *npoint_from_hexwkb(const char *hexwkb); */
  npoint_result = npoint_from_hexwkb(npoint1_hexwkb);
  char_result = npoint_out(npoint_result, 6);
  printf("npoint_from_hexwkb(%s): %s\n", npoint1_hexwkb, char_result);
  free(npoint_result); free(char_result);

  // /* Npoint *npoint_from_wkb(const uint8_t *wkb, size_t size); */
  // npoint_result = npoint_from_wkb(npoint1_wkb, size_t size);
  // char_result = npoint_out(npoint_result, 6);
  // printf("npoint_from_wkb(%s, %s): %s\n", npoint1_wkb, char_result);
  // free(npoint_result); free(char_result);

  /* Npoint *npoint_in(const char *str); */
  npoint_result = npoint_in(npoint1_in);
  char_result = npoint_out(npoint_result, 6);
  printf("npoint_in(%s): %s\n", npoint1_in, char_result);
  free(npoint_result); free(char_result);

  /* char *npoint_out(const Npoint *np, int maxdd); */
  char_result = npoint_out(npoint1, 6);
  printf("npoint_out(%s, 6): %s\n", npoint1_out, char_result);
  free(char_result);

  /* Nsegment *nsegment_in(const char *str); */
  nsegment_result = nsegment_in(nsegment1_in);
  char_result = nsegment_out(nsegment_result, 6);
  printf("nsegment_in(%s): %s\n", nsegment1_in, char_result);
  free(nsegment_result); free(char_result);

  /* char *nsegment_out(const Nsegment *ns, int maxdd); */
  char_result = nsegment_out(nsegment1, 6);
  printf("nsegment_out(%s, 6): %s\n", nsegment1_out, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Npoint *npoint_make(int64 rid, double pos); */
  npoint_result = npoint_make(int64_in1, float8_in1);
  char_result = npoint_out(npoint_result, 6);
  printf("npoint_make(%ld, %lf): %s\n", int64_in1, float8_in1, char_result);
  free(npoint_result); free(char_result);

  /* Nsegment *nsegment_make(int64 rid, double pos1, double pos2); */
  nsegment_result = nsegment_make(int64_in1, float8_in1, float8_in2);
  char_result = nsegment_out(nsegment_result, 6);
  printf("nsegment_make(%ld, %lf, %lf): %s\n", int64_in1, float8_in1, float8_in2, char_result);
  free(nsegment_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* Npoint *geompoint_to_npoint(const GSERIALIZED *gs); */
  npoint_result = geompoint_to_npoint(geom1);
  char_result = npoint_out(npoint_result, 6);
  printf("geompoint_to_npoint(%s): %s\n", geom1_out, char_result);
  free(npoint_result); free(char_result);

  /* Nsegment *geom_to_nsegment(const GSERIALIZED *gs); */
  nsegment_result = geom_to_nsegment(geom1);
  char_result = nsegment_out(nsegment_result, 6);
  printf("geom_to_nsegment(%s): %s\n", geom1_out, char_result);
  free(nsegment_result); free(char_result);

  /* GSERIALIZED *npoint_to_geompoint(const Npoint *np); */
  geom_result = npoint_to_geompoint(npoint1);
  char_result = geo_as_text(geom_result, 6);
  printf("npoint_to_geompoint(%s): %s\n", npoint1_out, char_result);
  free(geom_result); free(char_result);

  /* Nsegment *npoint_to_nsegment(const Npoint *np); */
  nsegment_result = npoint_to_nsegment(npoint1);
  char_result = nsegment_out(nsegment_result, 6);
  printf("npoint_to_nsegment(%s): %s\n", npoint1_out, char_result);
  free(nsegment_result); free(char_result);

  /* STBox *npoint_to_stbox(const Npoint *np); */
  stbox_result = npoint_to_stbox(npoint1);
  char_result = stbox_out(stbox_result, 6);
  printf("npoint_to_stbox(%s): %s\n", npoint1_out, char_result);
  free(stbox_result); free(char_result);

  /* GSERIALIZED *nsegment_to_geom(const Nsegment *ns); */
  geom_result = nsegment_to_geom(nsegment1);
  char_result = geo_as_text(geom_result, 6);
  printf("nsegment_to_geom(%s): %s\n", nsegment1_out, char_result);
  free(geom_result); free(char_result);

  /* STBox *nsegment_to_stbox(const Nsegment *np); */
  stbox_result = nsegment_to_stbox(nsegment1);
  char_result = stbox_out(stbox_result, 6);
  printf("nsegment_to_stbox(%s, 6): %s\n", nsegment1_out, char_result);
  free(stbox_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* uint32 npoint_hash(const Npoint *np); */
  uint32_result = npoint_hash(npoint1);
  printf("npoint_hash(%s): %u\n", npoint1_out, uint32_result);

  /* uint64 npoint_hash_extended(const Npoint *np, uint64 seed); */
  uint64_result = npoint_hash_extended(npoint1, 1);
  printf("npoint_hash_extended(%s, 1): %lu\n", npoint1_out, uint64_result);

  /* double npoint_position(const Npoint *np); */
  float8_result = npoint_position(npoint1);
  printf("npoint_position(%s): %lf\n", npoint1_out, float8_result);

  /* int64 npoint_route(const Npoint *np); */
  int64_result = npoint_route(npoint1);
  printf("npoint_route(%s): %lu\n", npoint1_out, int64_result);

  /* double nsegment_end_position(const Nsegment *ns); */
  float8_result = nsegment_end_position(nsegment1);
  printf("nsegment_end_position(%s): %lf\n", nsegment1_out, float8_result);

  /* int64 nsegment_route(const Nsegment *ns); */
  int64_result = nsegment_route(nsegment1);
  printf("nsegment_route(%s): %lu\n", nsegment1_out, int64_result);

  /* double nsegment_start_position(const Nsegment *ns); */
  float8_result = nsegment_start_position(nsegment1);
  printf("nsegment_start_position(%s): %lf\n", nsegment1_out, float8_result);

  /* Route functions */
  printf("****************************************************************\n");

  /* bool route_exists(int64 rid); */
  bool_result = route_exists(int64_in1);
  printf("route_exists(%lu): %c\n", int64_in1, bool_result ? 't' : 'n');

  /* const GSERIALIZED *route_geom(int64 rid); */
  const GSERIALIZED *route = route_geom(int64_in1);
  char_result = geo_as_text(route, 6);
  printf("route_geom(%lu): %s\n", int64_in1, char_result);
  free(char_result);

  /* double route_length(int64 rid); */
  float8_result = route_length(int64_in1);
  printf("route_length(%lu): %lf\n", int64_in1, float8_result);

  /* Transformation functions */
  printf("****************************************************************\n");

  /* Npoint *npoint_round(const Npoint *np, int maxdd); */
  npoint_result = npoint_round(npoint1, 6);
  char_result = npoint_out(npoint_result, 6);
  printf("npoint_round(%s, 6): %s\n", npoint1_out, char_result);
  free(npoint_result); free(char_result);

  /* Nsegment *nsegment_round(const Nsegment *ns, int maxdd); */
  nsegment_result = nsegment_round(nsegment1, 6);
  char_result = nsegment_out(nsegment_result, 6);
  printf("nsegment_round(%s, 6): %s\n", nsegment1_out, char_result);
  free(nsegment_result); free(char_result);

  /* Spatial reference system functions */
  printf("****************************************************************\n");

  /* int32_t get_srid_ways(void); */
  int32_result = get_srid_ways();
  printf("get_srid_ways(): %d\n", int32_result);

  /* int32_t npoint_srid(const Npoint *np); */
  int32_result = npoint_srid(npoint1);
  printf("npoint_srid(%s): %d\n", npoint1_out, int32_result);

  /* int32_t nsegment_srid(const Nsegment *ns); */
  int32_result = nsegment_srid(nsegment1);
  printf("nsegment_srid(%s): %d\n", nsegment1_out, int32_result);

  /* Bounding box functions */
  printf("****************************************************************\n");

  /* STBox *npoint_timestamptz_to_stbox(const Npoint *np, TimestampTz t); */
  stbox_result = npoint_timestamptz_to_stbox(npoint1, tstz1);
  char_result = stbox_out(stbox_result, 6);
  printf("npoint_timestamptz_to_stbox(%s, %s): %s\n", npoint1_out, tstz1_out, char_result);
  free(stbox_result); free(char_result);

  /* STBox *npoint_tstzspan_to_stbox(const Npoint *np, const Span *s); */
  stbox_result = npoint_tstzspan_to_stbox(npoint1, tstzspan1);
  char_result = stbox_out(stbox_result, 6);
  printf("npoint_tstzspan_to_stbox(%s, %s): %s\n", npoint1_out, tstzspan1_out, char_result);
  free(stbox_result); free(char_result);

  /* Comparison functions */
  printf("****************************************************************\n");

  /* int npoint_cmp(const Npoint *np1, const Npoint *np2); */
  int32_result = npoint_cmp(npoint1, npoint2);
  printf("npoint_cmp(%s, %s): %d\n", npoint1_out, npoint2_out, int32_result);

  /* bool npoint_eq(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_eq(npoint1, npoint2);
  printf("npoint_eq(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_ge(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_ge(npoint1, npoint2);
  printf("npoint_ge(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_gt(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_gt(npoint1, npoint2);
  printf("npoint_gt(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_le(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_le(npoint1, npoint2);
  printf("npoint_le(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_lt(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_lt(npoint1, npoint2);
  printf("npoint_lt(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_ne(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_ne(npoint1, npoint2);
  printf("npoint_ne(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* bool npoint_same(const Npoint *np1, const Npoint *np2); */
  bool_result = npoint_same(npoint1, npoint2);
  printf("npoint_same(%s, %s): %c\n", npoint1_out, npoint2_out, bool_result ? 't' : 'n');

  /* int nsegment_cmp(const Nsegment *ns1, const Nsegment *ns2); */
  int32_result = nsegment_cmp(nsegment1, nsegment2);
  printf("nsegment_cmp(%s, %s): %d\n", nsegment1_out, nsegment2_out, int32_result);

  /* bool nsegment_eq(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_eq(nsegment1, nsegment2);
  printf("nsegment_eq(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /* bool nsegment_ge(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_ge(nsegment1, nsegment2);
  printf("nsegment_ge(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /* bool nsegment_gt(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_gt(nsegment1, nsegment2);
  printf("nsegment_gt(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /* bool nsegment_le(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_le(nsegment1, nsegment2);
  printf("nsegment_le(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /* bool nsegment_lt(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_lt(nsegment1, nsegment2);
  printf("nsegment_lt(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /* bool nsegment_ne(const Nsegment *ns1, const Nsegment *ns2); */
  bool_result = nsegment_ne(nsegment1, nsegment2);
  printf("nsegment_ne(%s, %s): %c\n", nsegment1_out, nsegment2_out, bool_result ? 't' : 'n');

  /******************************************************************************
   * Functions for network point sets
   ******************************************************************************/

  /* Input and output functions */
  printf("****************************************************************\n");

  /* Set *npointset_in(const char *str); */
  npointset_result = npointset_in(npointset1_in);
  char_result = spatialset_as_text(npointset_result, 6);
  printf("npointset_in(%s): %s\n", npointset1_in, char_result);
  free(npointset_result); free(char_result);

  /* char *npointset_out(const Set *s, int maxdd); */
  char_result = npointset_out(npointset1, 6);
  printf("npointset_out(%s, 6): %s\n", npointset1_in, char_result);
  free(char_result);

  /* Constructor functions */
  printf("****************************************************************\n");

  /* Set *npointset_make(Npoint **values, int count); */
  npointarray[0] = npoint1;
  npointarray[1] = npoint2;
  npointset_result = npointset_make(npointarray, 2);
  char_result = spatialset_as_text(npointset_result, 6);
  printf("npointset_make({%s, %s}, 2): %s\n", npoint1_out, npoint2_out, char_result);
  free(npointset_result); free(char_result);

  /* Conversion functions */
  printf("****************************************************************\n");

  /* Set *npoint_to_set(const Npoint *np); */
  npointset_result = npoint_to_set(npoint1);
  char_result = spatialset_as_text(npointset_result, 6);
  printf("npoint_to_set(%s): %s\n", npoint1_out, char_result);
  free(npointset_result); free(char_result);

  /* Accessor functions */
  printf("****************************************************************\n");

  /* Npoint *npointset_end_value(const Set *s); */
  npoint_result = npointset_end_value(npointset1);
  char_result = npoint_out(npoint_result, 6);
  printf("npointset_end_value(%s): %s\n", npointset1_out, char_result);
  free(npoint_result); free(char_result);

  /* Set *npointset_routes(const Set *s); */
  bigintset_result = npointset_routes(npointset1);
  char_result = bigintset_out(bigintset_result);
  printf("npointset_routes(%s): %s\n", npointset1_out, char_result);
  free(bigintset_result); free(char_result);

  /* Npoint *npointset_start_value(const Set *s); */
  npoint_result = npointset_start_value(npointset1);
  char_result = npoint_out(npoint_result, 6);
  printf("npointset_start_value(%s): %s\n", npointset1_out, char_result);
  free(npoint_result); free(char_result);

  /* bool npointset_value_n(const Set *s, int n, Npoint **result); */
  bool_result = npointset_value_n(npointset1, 1, &npoint_result);
  char_result = npoint_out(npoint_result, 6);
  printf("npointset_value_n(%s, %s): %c\n", npointset1_out, char_result, bool_result ? 't' : 'n');
  free(npoint_result); free(char_result);

  /* Npoint **npointset_values(const Set *s); */
  npointarray_result = npointset_values(npointset1);
  printf("npointset_values(%s): {", npointset1_out);
  for (int i = 0; i < npointset1->count; i++)
  {
    char_result = npoint_as_text(npointarray_result[i], 6);
    printf("%s", char_result);
    if (i < npointset1->count - 1)
      printf(", ");
    else
      printf("}\n");
    free(npointarray_result[i]);
    free(char_result);
  }
  free(npointarray_result);

  /* Set operations */
  printf("****************************************************************\n");

  /* bool contained_npoint_set(const Npoint *np, const Set *s); */
  bool_result = contained_npoint_set(npoint1, npointset1);
  printf("contained_npoint_set(%s, %s): %c\n", npoint1_out, npointset1_out, bool_result ? 't' : 'n');

  /* bool contains_set_npoint(const Set *s, Npoint *np); */
  bool_result = contains_set_npoint(npointset1, npoint1);
  printf("contains_set_npoint(%s, %s): %c\n", npointset1_out, npoint1_out, bool_result ? 't' : 'n');

  /* Set *intersection_npoint_set(const Npoint *np, const Set *s); */
  npointset_result = intersection_npoint_set(npoint1, npointset1);
  char_result = npointset_result ? spatialset_as_text(npointset_result, 6) : text_out(text_null);
  printf("intersection_npoint_set(%s, %s): %s\n", npoint1_out, npointset1_out, char_result);
  if (npointset_result)
    free(npointset_result);
  free(char_result);

  /* Set *intersection_set_npoint(const Set *s, const Npoint *np); */
  npointset_result = intersection_set_npoint(npointset1, npoint1);
  char_result = npointset_result ? spatialset_as_text(npointset_result, 6) : text_out(text_null);
  printf("intersection_set_npoint(%s, %s): %s\n", npointset1_out, npoint1_out, char_result);
  if (npointset_result)
    free(npointset_result);
  free(char_result);

  /* Set *minus_npoint_set(const Npoint *np, const Set *s); */
  npointset_result = minus_npoint_set(npoint1, npointset1);
  char_result = npointset_result ? spatialset_as_text(npointset_result, 6) : text_out(text_null);
  printf("minus_npoint_set(%s, %s): %s\n", npoint1_out, npointset1_out, char_result);
  if (npointset_result)
    free(npointset_result);
  free(char_result);

  /* Set *minus_set_npoint(const Set *s, const Npoint *np); */
  npointset_result = minus_set_npoint(npointset1, npoint1);
  char_result = npointset_result ? spatialset_as_text(npointset_result, 6) : text_out(text_null);
  printf("minus_set_npoint(%s, %s): %s\n", npointset1_out, npoint1_out, char_result);
  if (npointset_result)
    free(npointset_result);
  free(char_result);

  // /* Set *npoint_union_transfn(Set *state, const Npoint *np); */
  // npointset_result = npoint_union_transfn(Set *state, npoint1);
  // char_result = spatialset_as_text(npointset_result, 6);
  // printf("minus_set_npoint(%s, 6): %s\n", npoint1_out, char_result);
  // free(npointset_result); free(char_result);

  /* Set *union_npoint_set(const Npoint *np, const Set *s); */
  npointset_result = union_npoint_set(npoint1, npointset1);
  char_result = spatialset_as_text(npointset_result, 6);
  printf("union_npoint_set(%s, %s): %s\n", npoint1_out, npointset1_out, char_result);
  free(npointset_result); free(char_result);

  /* Set *union_set_npoint(const Set *s, const Npoint *np); */
  npointset_result = union_set_npoint(npointset1, npoint1);
  char_result = spatialset_as_text(npointset_result, 6);
  printf("union_set_npoint(%s, %s): %s\n", npointset1_out, npoint1_out, char_result);
  free(npointset_result); free(char_result);

  /*===========================================================================*
   * Functions for temporal network points
   *===========================================================================*/

  /*****************************************************************************
   * Input/output functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tnpoint_in(const char *str); */
  tnpoint_result = tnpoint_in(tnpoint1_in);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("tnpoint_in(%s): %s\n", tnpoint1_in, char_result);
  free(tnpoint_result); free(char_result);

  /* char *tnpoint_out(const Temporal *temp, int maxdd); */
  char_result = tnpoint_out(tnpoint1, 6);
  printf("tnpoint_out(%s, 6): %s\n", tnpoint1_in, char_result);
  free(char_result);

  /*****************************************************************************
   * Constructor functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* TInstant *tnpointinst_make(const Npoint *np, TimestampTz t); */
  tnpoint_result = (Temporal *) tnpointinst_make(npoint1, tstz1);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("tnpointinst_make(%s, %s): %s\n", npoint1_out, tstz1_out, char_result);
  free(tnpoint_result); free(char_result);

  /*****************************************************************************
   * Conversion functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tgeompoint_to_tnpoint(const Temporal *temp); */
  tnpoint_result = tgeompoint_to_tnpoint(tgeompt1);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("tgeompoint_to_tnpoint(%s): %s\n", tgeompt1_out, char_result);
  free(tnpoint_result); free(char_result);

  /* Temporal *tnpoint_to_tgeompoint(const Temporal *temp); */
  tgeompt_result = tnpoint_to_tgeompoint(tnpoint1);
  char_result = tspatial_as_ewkt(tgeompt_result, 6);
  printf("tnpoint_to_tgeompoint(%s): %s\n", tnpoint1_out, char_result);
  free(tgeompt_result); free(char_result);

  /*****************************************************************************
   * Accessor functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tnpoint_cumulative_length(const Temporal *temp); */
  tfloat_result = tnpoint_cumulative_length(tnpoint1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnpoint_cumulative_length(%s, 6): %s\n", tnpoint1_out, char_result);
  free(tfloat_result); free(char_result);

  /* double tnpoint_length(const Temporal *temp); */
  float8_result = tnpoint_length(tnpoint1);
  printf("tnpoint_length(%s): %lf\n", tnpoint1_out, float8_result);

  /* Nsegment **tnpoint_positions(const Temporal *temp, int *count); */
  nsegmentarray_result = tnpoint_positions(tnpoint1, &count);
  printf("tnpoint_positions(%s, %d): {", tnpoint1_out, count);
  for (int i = 0; i < count; i++)
  {
    char_result = nsegment_out(nsegmentarray_result[i], 6);
    printf("%s", char_result);
    if (i < count - 1)
      printf(", ");
    else
      printf("}\n");
    free(nsegmentarray_result[i]);
    free(char_result);
  }
  free(nsegmentarray_result);

  /* int64 tnpoint_route(const Temporal *temp); */
  int64_result = tnpoint_route(tnpoint1);
  printf("tnpoint_route(%s): %lu\n", tnpoint1_out, int64_result);

  /* Set *tnpoint_routes(const Temporal *temp); */
  bigintset_result = tnpoint_routes(tnpoint1);
  char_result = bigintset_out(bigintset_result);
  printf("tnpoint_routes(%s, 6): %s\n", tnpoint1_out, char_result);
  free(bigintset_result); free(char_result);

  /* Temporal *tnpoint_speed(const Temporal *temp); */
  tfloat_result = tnpoint_speed(tnpoint1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tnpoint_speed(%s, 6): %s\n", tnpoint1_out, char_result);
  free(tfloat_result); free(char_result);

  /* GSERIALIZED *tnpoint_trajectory(const Temporal *temp); */
  geom_result = tnpoint_trajectory(tnpoint1);
  char_result = geo_as_text(geom_result, 6);
  printf("tnpoint_trajectory(%s): %s\n", tnpoint1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *tnpoint_twcentroid(const Temporal *temp); */
  geom_result = tnpoint_twcentroid(tnpoint1);
  char_result = geo_as_text(geom_result, 6);
  printf("tnpoint_twcentroid(%s): %s\n", tnpoint1_out, char_result);
  free(geom_result); free(char_result);

  /*****************************************************************************
   * Restriction functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tnpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tnpoint_result = tnpoint_at_geom(tnpoint1, geom2);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_at_geom(%s, %s): %s\n", tnpoint1_out, geom2_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_at_npoint(const Temporal *temp, const Npoint *np); */
  tnpoint_result = tnpoint_at_npoint(tnpoint1, npoint1);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_at_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_at_npointset(const Temporal *temp, const Set *s); */
  tnpoint_result = tnpoint_at_npointset(tnpoint1, npointset1);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_at_npointset(%s, %s): %s\n", tnpoint1_out, npointset1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tnpoint_result = tnpoint_at_stbox(tnpoint1, stbox1, true);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_at_stbox(%s, %s, true): %s\n", tnpoint1_out, stbox1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs); */
  tnpoint_result = tnpoint_minus_geom(tnpoint1, geom2);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_minus_geom(%s, %s): %s\n", tnpoint1_out, geom2_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_minus_npoint(const Temporal *temp, const Npoint *np); */
  tnpoint_result = tnpoint_minus_npoint(tnpoint1, npoint1);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_minus_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_minus_npointset(const Temporal *temp, const Set *s); */
  tnpoint_result = tnpoint_minus_npointset(tnpoint1, npointset1);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_minus_npointset(%s, %s): %s\n", tnpoint1_out, npointset1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /* Temporal *tnpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc); */
  tnpoint_result = tnpoint_minus_stbox(tnpoint1, stbox1, true);
  char_result = tnpoint_result ? tnpoint_out(tnpoint_result, 6) : text_out(text_null);
  printf("tnpoint_minus_stbox(%s, %s, true): %s\n", tnpoint1_out, stbox1_out, char_result);
  if (tnpoint_result)
    free(tnpoint_result);
  free(char_result);

  /*****************************************************************************
   * Distance functions
   *****************************************************************************/
  printf("****************************************************************\n");

  /* Temporal *tdistance_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  tfloat_result = tdistance_tnpoint_npoint(tnpoint1, npoint1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tnpoint_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tnpoint_point(const Temporal *temp, const GSERIALIZED *gs); */
  tfloat_result = tdistance_tnpoint_point(tnpoint1, geom1);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tnpoint_point(%s, %s): %s\n", tnpoint1_out, geom1_out, char_result);
  free(tfloat_result); free(char_result);

  /* Temporal *tdistance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  tfloat_result = tdistance_tnpoint_tnpoint(tnpoint1, tnpoint2);
  char_result = tfloat_out(tfloat_result, 6);
  printf("tdistance_tnpoint_tnpoint(%s, %s): %s\n", tnpoint1_out, tnpoint2_out, char_result);
  free(tfloat_result); free(char_result);

  /* double nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs); */
  float8_result = nad_tnpoint_geo(tnpoint1, geom1);
  printf("nad_tnpoint_geo(%s, %s): %lf\n", tnpoint1_out, geom1_out, float8_result);

  /* double nad_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  float8_result = nad_tnpoint_npoint(tnpoint1, npoint1);
  printf("nad_tnpoint_npoint(%s, %s): %lf\n", tnpoint1_out, npoint1_out, float8_result);

  /* double nad_tnpoint_stbox(const Temporal *temp, const STBox *box); */
  float8_result = nad_tnpoint_stbox(tnpoint1, stbox1);
  printf("nad_tnpoint_stbox(%s, %s): %lf\n", tnpoint1_out, stbox1_out, float8_result);

  /* double nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  float8_result = nad_tnpoint_tnpoint(tnpoint1, tnpoint2);
  printf("nad_tnpoint_tnpoint(%s, %s): %lf\n", tnpoint1_out, tnpoint2_out, float8_result);

  /* TInstant *nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs); */
  tnpoint_result = (Temporal *) nai_tnpoint_geo(tnpoint1, geom1);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("nai_tnpoint_geo(%s, %s): %s\n", tnpoint1_out, geom1_out, char_result);
  free(tnpoint_result); free(char_result);

  /* TInstant *nai_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  tnpoint_result = (Temporal *) nai_tnpoint_npoint(tnpoint1, npoint1);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("nai_tnpoint_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  free(tnpoint_result); free(char_result);

  /* TInstant *nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  tnpoint_result = (Temporal *) nai_tnpoint_tnpoint(tnpoint1, tnpoint2);
  char_result = tnpoint_out(tnpoint_result, 6);
  printf("nai_tnpoint_tnpoint(%s, %s): %s\n", tnpoint1_out, tnpoint2_out, char_result);
  free(tnpoint_result); free(char_result);

  /* GSERIALIZED *shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs); */
  geom_result = shortestline_tnpoint_geo(tnpoint1, geom1);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tnpoint_geo(%s, %s): %s\n", tnpoint1_out, geom1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  geom_result = shortestline_tnpoint_npoint(tnpoint1, npoint1);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tnpoint_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  free(geom_result); free(char_result);

  /* GSERIALIZED *shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  geom_result = shortestline_tnpoint_tnpoint(tnpoint1, tnpoint2);
  char_result = geo_as_text(geom_result, 6);
  printf("shortestline_tnpoint_tnpoint(%s, %s): %s\n", tnpoint1_out, tnpoint2_out, char_result);
  free(geom_result); free(char_result);

  /*****************************************************************************
   * Aggregate functions
   *****************************************************************************/
  printf("****************************************************************\n");

  // /* SkipList *tnpoint_tcentroid_transfn(SkipList *state, Temporal *temp); */
  // SkipList *tnpoint_tcentroid_transfn(SkipList *state, Temporal *temp);

  /*****************************************************************************
   * Comparison functions
   *****************************************************************************/

  /* Ever/always comparisons */
  printf("****************************************************************\n");

  /* int always_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp); */
  int32_result = always_eq_npoint_tnpoint(npoint1, tnpoint1);
  printf("always_eq_npoint_tnpoint(%s, %s): %d\n", npoint1_out, tnpoint1_out, int32_result);

  /* int always_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  int32_result = always_eq_tnpoint_npoint(tnpoint1, npoint1);
  printf("always_eq_tnpoint_npoint(%s, %s): %d\n", tnpoint1_out, npoint1_out, int32_result);

  /* int always_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_eq_tnpoint_tnpoint(tnpoint1, tnpoint2);
  printf("always_eq_tnpoint_tnpoint(%s, %s): %d\n", tnpoint1_out, tnpoint2_out, int32_result);

  /* int always_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp); */
  int32_result = always_ne_npoint_tnpoint(npoint1, tnpoint1);
  printf("always_ne_npoint_tnpoint(%s, %s): %d\n", npoint1_out, tnpoint1_out, int32_result);

  /* int always_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  int32_result = always_ne_tnpoint_npoint(tnpoint1, npoint1);
  printf("always_ne_tnpoint_npoint(%s, %s): %d\n", tnpoint1_out, npoint1_out, int32_result);

  /* int always_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  int32_result = always_ne_tnpoint_tnpoint(tnpoint1, tnpoint2);
  printf("always_ne_tnpoint_tnpoint(%s, %s): %d\n", tnpoint1_out, tnpoint2_out, int32_result);

  /* int ever_eq_npoint_tnpoint(const Npoint *np, const Temporal *temp); */
  int32_result = ever_eq_npoint_tnpoint(npoint1, tnpoint1);
  printf("ever_eq_npoint_tnpoint(%s, %s): %d\n", npoint1_out, tnpoint1_out, int32_result);

  /* int ever_eq_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  int32_result = ever_eq_tnpoint_npoint(tnpoint1, npoint1);
  printf("ever_eq_tnpoint_npoint(%s, %s): %d\n", tnpoint1_out, npoint1_out, int32_result);

  /* int ever_eq_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_eq_tnpoint_tnpoint(tnpoint1, tnpoint2);
  printf("ever_eq_tnpoint_tnpoint(%s, %s): %d\n", tnpoint1_out, tnpoint2_out, int32_result);

  /* int ever_ne_npoint_tnpoint(const Npoint *np, const Temporal *temp); */
  int32_result = ever_ne_npoint_tnpoint(npoint1, tnpoint1);
  printf("ever_ne_npoint_tnpoint(%s, %s): %d\n", npoint1_out, tnpoint1_out, int32_result);

  /* int ever_ne_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  int32_result = ever_ne_tnpoint_npoint(tnpoint1, npoint1);
  printf("ever_ne_tnpoint_npoint(%s, %s): %d\n", tnpoint1_out, npoint1_out, int32_result);

  /* int ever_ne_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2); */
  int32_result = ever_ne_tnpoint_tnpoint(tnpoint1, tnpoint2);
  printf("ever_ne_tnpoint_tnpoint(%s, %s): %d\n", tnpoint1_out, tnpoint2_out, int32_result);

  /* Temporal comparisons */
  printf("****************************************************************\n");

  /* Temporal *teq_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  tbool_result = teq_tnpoint_npoint(tnpoint1, npoint1);
  char_result = tbool_out(tbool_result);
  printf("teq_tnpoint_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  free(tbool_result); free(char_result);

  /* Temporal *tne_tnpoint_npoint(const Temporal *temp, const Npoint *np); */
  tbool_result = tne_tnpoint_npoint(tnpoint1, npoint1);
  char_result = tbool_out(tbool_result);
  printf("tne_tnpoint_npoint(%s, %s): %s\n", tnpoint1_out, npoint1_out, char_result);
  free(tbool_result); free(char_result);

  printf("****************************************************************\n");

  /* Clean up */

  free(text_null);
  free(tstz1_out);
  free(tstz2_out);
  free(tstzset1); free(tstzset1_out);
  free(tstzspan1); free(tstzspan1_out);
  free(tstzspanset1); free(tstzspanset1_out);
  free(tgeompt1); free(tgeompt1_out);
  free(npoint1); free(npoint1_out); free(npoint1_wkb); free(npoint1_hexwkb);
  free(npoint2); free(npoint2_out);
  free(npointset1); free(npointset1_out);
  free(nsegment1); free(nsegment1_out);
  free(nsegment2); free(nsegment2_out);
  free(geom1); free(geom1_out);
  free(geom2); free(geom2_out);
  free(stbox1); free(stbox1_out);
  free(tnpoint1); free(tnpoint1_out);
  free(tnpoint2); free(tnpoint2_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
