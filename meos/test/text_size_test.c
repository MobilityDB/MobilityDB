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
 * @brief A program that tests that every text output of MEOS states the size
 * of the string it returns
 * @details A function returning a string the caller frees writes its size,
 * the terminating null character included, through its `size_out` argument,
 * so that a binding copies the string without scanning it for the
 * terminator. For one value of each kind the program checks that the size
 * written is the length of the string plus one: the temporal values of each
 * subtype, the sets, spans and span sets, the boxes, the geometries in each
 * of their formats, the spatial temporal values and sets with and without
 * their SRID, MF-JSON compact and pretty, the circular buffers, the network
 * points, the poses and pose chains with their GeoPose documents, the rigid
 * geometries, the cells, the JSON values, the base values written by the
 * PostgreSQL output functions, and the version and style strings. It also
 * checks that a null `size_out` is refused with #meos_errno set, as the hex
 * WKB writers refuse it.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o text_size_test text_size_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_json.h>
#include <meos_npoint.h>
#include <meos_pose.h>
#include <meos_quadbin.h>
#include <meos_rgeo.h>
#include <meos_s2cell.h>
#include <pg_int.h>

/* The Basic-YPR class of a GeoPose document, as geopose_test.c names it */
#define BASIC_YPR 1

static int checked = 0;
static int failures = 0;

/*
 * Every value the program reads, freed before it ends. The test programs free
 * a value they read by name; this one reads most of its values inside the
 * CHECK calls, so it records them. No test program has such a routine; the
 * nearest is the counting allocator of trgeo_test.c (#counting_malloc).
 */
static void *kept[128];
static int nkept = 0;

static void *
keep(void *ptr)
{
  kept[nkept++] = ptr;
  return ptr;
}

/* The string is returned and its size is its length plus the terminator */
static void
check(const char *call, char *str, size_t size)
{
  checked++;
  if (! str)
  {
    printf("FAIL %s: NULL, errno %d\n", call, meos_errno());
    failures++;
    return;
  }
  size_t expected = strlen(str) + 1;
  if (size != expected)
  {
    printf("FAIL %s: size %zu, expected %zu for \"%s\"\n", call, size,
      expected, str);
    failures++;
  }
  free(str);
}

/* Call a text output with &size as its last argument and check the size */
#define CHECK(call) \
  do { \
    size_t size = 0; \
    char *str = call; \
    check(#call, str, size); \
  } while (0)

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Temporal values of each subtype, through the shared builders */
  Temporal *tinst = keep(tfloat_in("1.5@2001-01-01"));
  Temporal *tseq = keep(tfloat_in("[1.5@2001-01-01, 2.5@2001-01-02]"));
  Temporal *tss = keep(tfloat_in(
    "{[1.5@2001-01-01, 2.5@2001-01-02], [3.5@2001-01-03, 4.5@2001-01-04]}"));
  Temporal *tstep = keep(tfloat_in(
    "Interp=Step;[1.5@2001-01-01, 2.5@2001-01-02]"));
  CHECK(temporal_out(tinst, 6, &size));
  CHECK(temporal_out(tseq, 6, &size));
  CHECK(temporal_out(tss, 6, &size));
  CHECK(temporal_out(tstep, 6, &size));
  CHECK(temporal_out(keep(tint_in("{1@2001-01-01, 2@2001-01-02}")), 0, &size));
  CHECK(temporal_out(keep(tbool_in("[true@2001-01-01, false@2001-01-02]")), 0, &size));
  CHECK(temporal_out(keep(ttext_in("[\"a b\"@2001-01-01, \"c\"@2001-01-02]")), 0, &size));
  CHECK(temporal_out(keep(tbigint_in("1@2001-01-01")), 0, &size));
  CHECK(temporal_as_mfjson(tseq, 0, 0, 6, NULL, &size));
  /* The flag of a pretty document, JSON_C_TO_STRING_PRETTY */
  CHECK(temporal_as_mfjson(tss, 1, 2, 6, NULL, &size));

  /* Sets, spans and span sets */
  CHECK(set_out(keep(intset_in("{1, 2, 3}")), 0, &size));
  CHECK(set_out(keep(floatset_in("{1.5, 2.5}")), 6, &size));
  CHECK(set_out(keep(textset_in("{\"a b\", c}")), 0, &size));
  CHECK(set_out(keep(dateset_in("{2001-01-01, 2001-01-02}")), 0, &size));
  CHECK(set_out(keep(tstzset_in("{2001-01-01, 2001-01-02}")), 0, &size));
  CHECK(span_out(keep(intspan_in("[1, 3)")), 0, &size));
  CHECK(span_out(keep(floatspan_in("[1.5, 2.5]")), 6, &size));
  CHECK(span_out(keep(bigintspan_in("[1, 3)")), 0, &size));
  CHECK(span_out(keep(tstzspan_in("[2001-01-01, 2001-01-02]")), 0, &size));
  CHECK(spanset_out(keep(floatspanset_in("{[1.5, 2.5], [3.5, 4.5]}")), 6, &size));
  CHECK(spanset_out(keep(tstzspanset_in(
      "{[2001-01-01, 2001-01-02], [2001-01-03, 2001-01-04]}")), 0, &size));

  /* Boxes */
  CHECK(tbox_out(keep(tbox_in(
    "TBOXFLOAT XT([1.5, 2.5],[2001-01-01, 2001-01-02])")), 6, &size));
  STBox *box = keep(stbox_in(
    "SRID=4326;STBOX XT(((1,1),(2,2)),[2001-01-01, 2001-01-02])"));
  CHECK(stbox_out(box, 6, &size));

  /* Geometries in each format */
  GSERIALIZED *gs = keep(geom_in("SRID=4326;LINESTRING(1 1,2 2,3 1)", -1));
  GSERIALIZED *gs2 = keep(geom_in("SRID=4326;POINT(2 2)", -1));
  CHECK(geo_out(gs, &size));
  CHECK(geo_as_text(gs, 6, &size));
  CHECK(geo_as_ewkt(gs, 6, &size));
  CHECK(geo_as_geojson(gs, 0, 6, NULL, &size));
  CHECK(geo_as_hexewkb(gs, "NDR", &size));
  CHECK(geom_relate(gs, gs2, &size));

  /* Spatial temporal values and sets, with and without their SRID */
  Temporal *tpt = keep(tgeompoint_in(
    "SRID=4326;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]"));
  Temporal *tpt0 = keep(tgeompoint_in(
    "[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]"));
  Temporal *tptstep = keep(tgeompoint_in(
    "SRID=4326;Interp=Step;[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]"));
  CHECK(temporal_out(tpt, 6, &size));
  CHECK(tspatial_as_text(tpt, 6, &size));
  CHECK(tspatial_as_ewkt(tpt, 6, &size));
  CHECK(tspatial_as_ewkt(tpt0, 6, &size));
  CHECK(tspatial_as_ewkt(tptstep, 6, &size));
  Set *gset = keep(geomset_in("SRID=4326;{Point(1 1), Point(2 2)}"));
  Set *gset0 = keep(geomset_in("{Point(1 1), Point(2 2)}"));
  CHECK(set_out(gset, 6, &size));
  CHECK(spatialset_as_text(gset, 6, &size));
  CHECK(spatialset_as_ewkt(gset, 6, &size));
  CHECK(spatialset_as_ewkt(gset0, 6, &size));
  CHECK(temporal_as_mfjson(tpt, 1, 0, 6, "EPSG:4326", &size));

  /* Circular buffers */
  Cbuffer *cb = keep(cbuffer_in("SRID=4326;Cbuffer(Point(1 1), 0.5)"));
  CHECK(cbuffer_out(cb, 6, &size));
  CHECK(cbuffer_as_text(cb, 6, &size));
  CHECK(cbuffer_as_ewkt(cb, 6, &size));
  CHECK(set_out(keep(cbufferset_in("{\"Cbuffer(Point(1 1), 0.5)\"}")), 6, &size));
  CHECK(tspatial_as_ewkt(
    keep(tcbuffer_in("SRID=4326;[Cbuffer(Point(1 1), 0.5)@2001-01-01]")), 6,
    &size));

  /* Network points and segments */
  Npoint *np = keep(npoint_in("Npoint(1, 0.5)"));
  CHECK(npoint_out(np, 6, &size));
  CHECK(npoint_as_text(np, 6, &size));
  Nsegment *ns = keep(nsegment_in("Nsegment(1, 0.25, 0.75)"));
  CHECK(nsegment_out(ns, 6, &size));
  CHECK(nsegment_as_text(ns, 6, &size));

  /* Poses, pose chains and their GeoPose documents */
  Pose *pose = keep(pose_in("SRID=4326;Pose(Point(8 47), 0.5)"));
  CHECK(pose_out(pose, 6, &size));
  CHECK(pose_as_text(pose, 6, &size));
  CHECK(pose_as_ewkt(pose, 6, &size));
  /* A GeoPose document describes a geodetic pose */
  Pose *gpose = keep(pose_in("Geodpose(Point(8 47), 0.5)"));
  CHECK(pose_as_geopose(gpose, BASIC_YPR, 6, &size));
  CHECK(set_out(keep(poseset_in("{\"Pose(Point(1 1), 0.5)\"}")), 6, &size));
  PoseChain *pc = keep(posechain_in(
    "SRID=4326;PoseChain(Pose(Point(1 1), 0.5),Pose(Point(2 2), 1))"));
  CHECK(posechain_out(pc, 6, &size));
  CHECK(posechain_as_text(pc, 6, &size));
  CHECK(posechain_as_ewkt(pc, 6, &size));
  Temporal *tpose1 = keep(tpose_in("Geodpose(Point(8 47), 0.5)@2001-01-01"));
  Temporal *tposes = keep(tpose_in("[Geodpose(Point(8 47), 0.5)@2001-01-01, "
    "Geodpose(Point(9 48), 1)@2001-01-02]"));
  CHECK(tpose_as_geopose(tpose1, BASIC_YPR, 6, &size));
  CHECK(tpose_as_geopose(tposes, BASIC_YPR, 6, &size));
  CHECK(tpose_as_geopose_stream(tposes, 6, &size));
  CHECK(tpose_as_geopose_stream_header(tposes, 6, &size));
  CHECK(geopose_frame_name(1, &size));

  /* Rigid geometries */
  Temporal *trgeo = keep(trgeometry_in("Polygon((1 1,2 2,3 1,1 1));"
    "[Pose(Point(1 1), 0.5)@2001-01-01, Pose(Point(2 2), 1)@2001-01-02]"));
  CHECK(trgeometry_out(trgeo, &size));
  CHECK(trgeometry_as_text(trgeo, 6, &size));
  CHECK(trgeometry_as_ewkt(trgeo, 6, &size));

  /* Cells */
  CHECK(quadbin_out(quadbin_in("480fffffffffffff"), &size));
  CHECK(temporal_out(keep(th3index_in("8a2a1072b59ffff@2001-01-01")), 0, &size));

  /* JSON values */
  Jsonb *jb = keep(jsonb_in("{\"a\": [1, 2], \"b\": \"c\"}"));
  CHECK(jsonb_out(jb, &size));
  CHECK(jsonb_to_cstring(jb, &size));
  CHECK(set_out(keep(jsonbset_in("{\"{\\\"a\\\": 1}\"}")), 0, &size));
  CHECK(temporal_out(keep(tjsonb_in("\"{\\\"a\\\": 1}\"@2001-01-01")), 0, &size));

  /* Base values written by the PostgreSQL output functions */
  CHECK(bool_out(true, &size));
  CHECK(int32_out(-12345, &size));
  CHECK(int64_out(1234567890123LL, &size));
  CHECK(float8_out(1.5, 6, &size));
  CHECK(date_out(date_in("2001-01-01"), &size));
  CHECK(time_out(time_in("10:00:00", -1), &size));
  CHECK(timestamptz_out(timestamptz_in("2001-01-01 10:00:00", -1), &size));
  CHECK(interval_out(keep(interval_in("1 day 2 hours", -1)), &size));
  CHECK(text_out(keep(text_in("a text value")), &size));
  CHECK(numeric_out(keep(numeric_in("1234.5678", -1)), &size));

  /* Version and style strings */
  CHECK(meos_version(&size));
  CHECK(mobilitydb_version(&size));
  CHECK(meos_get_datestyle(&size));
  CHECK(meos_get_intervalstyle(&size));

  /* A null size_out is refused, as the hex WKB writers refuse it, by a MEOS
   * function and by a base type output of pgtypes */
  meos_errno_reset();
  char *str = temporal_out(tseq, 6, NULL);
  printf("temporal_out(..., NULL): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  if (str || meos_errno() != MEOS_ERR_INVALID_ARG)
    failures++;
  meos_errno_reset();
  str = text_out(keep(text_in("a text value")), NULL);
  printf("text_out(..., NULL): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  if (str || meos_errno() != MEOS_ERR_INVALID_ARG)
    failures++;
  meos_errno_reset();

  printf("%d text outputs checked, %d failures\n", checked, failures);
  for (int i = 0; i < nkept; i++)
    free(kept[i]);
  meos_finalize();
  return failures ? 1 : 0;
}
