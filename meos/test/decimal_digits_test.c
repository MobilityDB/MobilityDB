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
 * @brief A program that tests how the functions taking a count of decimal
 * digits treat a count of zero and a negative count
 * @details A count of decimal digits says how many digits a value keeps after
 * the
 * decimal point, and zero keeps none; every output and rounding function of
 * MEOS accepts it. A negative count is an erroneous argument, which the
 * public function taking it reports at its entry, as it reports a null
 * value, so that every binding calling the function receives the error
 * instead of a value rounded to tens or an assertion failure in an internal
 * function.
 *
 * The program verifies that pcpoint_as_hexwkb and pcpatch_as_hexwkb, which
 * hand their hex output a count of zero, answer the hex their input reads
 * back with no error left behind; that tspatial_out, pose_round, geo_as_text
 * and float_round answer for a count of zero, and pose_as_geopose for the
 * negative precision GeoPose reads as its lossless form; that the text,
 * EWKT, GeoJSON and MF-JSON outputs and the rounding functions report a
 * negative count with MEOS_ERR_INVALID_ARG_VALUE, as do the internal outputs of a
 * temporal value, an array of them, a set, a span and a span set, which the
 * PostgreSQL wrappers of every type call with the count a user writes; and
 * that an MF-JSON precision above the default writes the default number of
 * decimal digits.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o decimal_digits_test decimal_digits_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_npoint.h>
#include <meos_pointcloud.h>
#include <meos_pose.h>

/* A pcpoint of pcid 1 holding X=1.0 Y=2.0 Z=3.0, and a pcpatch of pcid 1
 * holding the two points (1,1,1) and (2,2,2), in the hex WKB pgPointCloud
 * serializes, the form that carries no schema, as tpointcloud_test reads
 * them */
#define PCPOINT_HEX \
  "2300000001000000000000000000F03F0000000000000040000000000000084000" \
  "0000"
#define PCPATCH_HEX \
  "4F000000010000000000000002000000000000000000F03F000000000000F03F00" \
  "0000000000F03F0000000000000040000000000000004000000000000000400000" \
  "00000000000000000000000000"

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* A pcpoint writes the hex its input reads back */
  Pcpoint *pt = pcpoint_from_hexwkb(PCPOINT_HEX);
  assert(pt && meos_errno() == 0);
  char *hex1 = pcpoint_as_hexwkb(pt);
  printf("pcpoint_as_hexwkb: %s, errno %d\n", hex1 ? hex1 : "NULL",
    meos_errno());
  assert(hex1 && meos_errno() == 0);
  Pcpoint *pt2 = pcpoint_from_hexwkb(hex1);
  char *hex2 = pt2 ? pcpoint_as_hexwkb(pt2) : NULL;
  assert(hex2 && strcmp(hex1, hex2) == 0);
  free(pt); free(pt2); free(hex1); free(hex2);

  /* So does a pcpatch */
  Pcpatch *pa = pcpatch_from_hexwkb(PCPATCH_HEX);
  assert(pa && meos_errno() == 0);
  hex1 = pcpatch_as_hexwkb(pa);
  printf("pcpatch_as_hexwkb: %s, errno %d\n", hex1 ? hex1 : "NULL",
    meos_errno());
  assert(hex1 && meos_errno() == 0);
  Pcpatch *pa2 = pcpatch_from_hexwkb(hex1);
  hex2 = pa2 ? pcpatch_as_hexwkb(pa2) : NULL;
  assert(hex2 && strcmp(hex1, hex2) == 0);
  free(pa); free(pa2); free(hex1); free(hex2);

  /* A spatiotemporal value is written with no decimal digits */
  Temporal *temp = tgeompoint_in(
    "[Point(1.25 2.75)@2001-01-01, Point(2 2)@2001-01-02]");
  assert(temp);
  char *str = tspatial_out(temp, 0);
  printf("tspatial_out(temp, 0): %s, errno %d\n", str ? str : "NULL",
    meos_errno());
  assert(str && meos_errno() == 0);
  free(str);
  str = tspatial_out(temp, -1);
  printf("tspatial_out(temp, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(temp);

  /* A pose is rounded to no decimal digits */
  Pose *pose = pose_in("Pose(Point(1.25 2.75),0.5)");
  assert(pose);
  Pose *rounded = pose_round(pose, 0);
  str = rounded ? pose_out(rounded, 15) : NULL;
  printf("pose_round(pose, 0): %s, errno %d\n", str ? str : "NULL",
    meos_errno());
  assert(rounded && meos_errno() == 0);
  free(str); free(rounded);
  rounded = pose_round(pose, -1);
  printf("pose_round(pose, -1): %s, errno %d\n", rounded ? "a value" : "NULL",
    meos_errno());
  assert(rounded == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();

  /* A pose is written with a negative count refused */
  str = pose_as_text(pose, -1);
  printf("pose_as_text(pose, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = pose_as_ewkt(pose, -1);
  printf("pose_as_ewkt(pose, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(pose);

  /* GeoPose, which writes a geodetic pose, reads a negative precision as its
   * lossless form, the default of every asGeoPose declaration */
  Pose *gpose = pose_in("Geodpose(Point(1 1),0.5)");
  assert(gpose);
  str = pose_as_geopose(gpose, 0, -1);
  printf("pose_as_geopose(gpose, 0, -1): %s, errno %d\n",
    str ? str : "NULL", meos_errno());
  assert(str && meos_errno() == 0);
  free(str); free(gpose);

  /* So is a circular buffer */
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(1 1),0.5)");
  assert(cb);
  str = cbuffer_as_ewkt(cb, -1);
  printf("cbuffer_as_ewkt(cb, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(cb);

  /* A geometry is written with no decimal digits, and a negative count is
   * refused */
  GSERIALIZED *gs = geom_in("Point(1.25 2.75)", -1);
  assert(gs);
  str = geo_as_text(gs, 0);
  printf("geo_as_text(gs, 0): %s, errno %d\n", str ? str : "NULL",
    meos_errno());
  assert(str && meos_errno() == 0);
  free(str);
  str = geo_as_text(gs, -1);
  printf("geo_as_text(gs, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = geo_as_geojson(gs, 0, -1, NULL);
  printf("geo_as_geojson(gs, 0, -1, NULL): %s, errno %d\n",
    str ? "a value" : "NULL", meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(gs);

  /* The float set, span, span set and temporal float outputs refuse a
   * negative count */
  Set *fset = floatset_in("{1.25, 2.5}");
  Span *fspan = floatspan_in("[1.25, 2.5]");
  SpanSet *fspanset = floatspanset_in("{[1.25, 2.5]}");
  Temporal *tfloat = tfloat_in("1.26@2001-01-01");
  assert(fset && fspan && fspanset && tfloat);
  str = floatset_out(fset, -1);
  printf("floatset_out(s, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = floatspan_out(fspan, -1);
  printf("floatspan_out(s, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = floatspanset_out(fspanset, -1);
  printf("floatspanset_out(ss, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = tfloat_out(tfloat, -1);
  printf("tfloat_out(temp, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = temporal_as_mfjson(tfloat, false, 0, -1, NULL);
  printf("temporal_as_mfjson(temp, false, 0, -1, NULL): %s, errno %d\n",
    str ? "a value" : "NULL", meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();

  /* The rounding functions refuse a negative count */
  Temporal *trounded = temporal_round(tfloat, -1);
  printf("temporal_round(temp, -1): %s, errno %d\n",
    trounded ? "a value" : "NULL", meos_errno());
  assert(trounded == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  double d = float_round(1.26, 0);
  printf("float_round(1.26, 0): %g, errno %d\n", d, meos_errno());
  assert(d == 1.0 && meos_errno() == 0);
  d = float_round(1.26, -1);
  printf("float_round(1.26, -1): %s, errno %d\n",
    d == DBL_MAX ? "DBL_MAX" : "a value", meos_errno());
  assert(d == DBL_MAX && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  Npoint *np = npoint_make(1, 0.26);
  Nsegment *ns = nsegment_make(1, 0.26, 0.74);
  assert(np && ns);
  Npoint *nprounded = npoint_round(np, -1);
  printf("npoint_round(np, -1): %s, errno %d\n",
    nprounded ? "a value" : "NULL", meos_errno());
  assert(nprounded == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  Nsegment *nsrounded = nsegment_round(ns, -1);
  printf("nsegment_round(ns, -1): %s, errno %d\n",
    nsrounded ? "a value" : "NULL", meos_errno());
  assert(nsrounded == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  /* The internal outputs of a temporal value, an array of them, a set, a
   * span and a span set, which the PostgreSQL wrappers call with the count a
   * user writes, report a negative count */
  str = temporal_out(tfloat, -1);
  printf("temporal_out(temp, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  Temporal *temparr[1] = {tfloat};
  char **strarr = temparr_out(temparr, 1, -1);
  printf("temparr_out(temparr, 1, -1): %s, errno %d\n",
    strarr ? "a value" : "NULL", meos_errno());
  assert(strarr == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = set_out(fset, -1);
  printf("set_out(s, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = span_out(fspan, -1);
  printf("span_out(s, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  str = spanset_out(fspanset, -1);
  printf("spanset_out(ss, -1): %s, errno %d\n", str ? "a value" : "NULL",
    meos_errno());
  assert(str == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();

  /* An MF-JSON precision above the default writes the default number of
   * decimal digits */
  str = temporal_as_mfjson(tfloat, false, 0, 20, NULL);
  char *str15 = temporal_as_mfjson(tfloat, false, 0, 15, NULL);
  printf("temporal_as_mfjson(temp, false, 0, 20, NULL): %s, errno %d\n",
    str ? str : "NULL", meos_errno());
  assert(str && str15 && strcmp(str, str15) == 0 && meos_errno() == 0);
  free(str); free(str15);
  free(fset); free(fspan); free(fspanset); free(tfloat); free(np); free(ns);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
