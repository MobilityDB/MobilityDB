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
 * @brief A program that tests the typed value functions of the temporal
 * instants
 * @details Each temporal type answers the value of a temporal instant through
 * a typed public function, the one the SQL function getValue names. The
 * program verifies, for every temporal type, that the value of an instant is
 * the value its start value function answers, and that a sequence, a null
 * value and a value of another type are refused with the error codes 11, 10
 * and 11 and the error value of the function.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_value_test temporal_value_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_h3.h>
#include <meos_json.h>
#include <meos_npoint.h>
#include <meos_pointcloud.h>
#include <meos_pose.h>
#include <meos_quadbin.h>
#include <meos_rgeo.h>
#include <meos_s2cell.h>

/* A pcpoint of pcid 1 holding X=1.0 Y=2.0 Z=3.0 and a pcpatch of pcid 1
 * holding the points (1,1,1) and (2,2,2), in the hex WKB of pgPointCloud, as
 * tpointcloud_test reads them */
#define PCPOINT_HEX "010100000064000000C80000002C010000"
#define PCPATCH_HEX \
  "01010000000000000002000000640000006400000064000000C8000000C8000000" \
  "C8000000"

/* Check that a call answered its error value with the expected error code */
#define REFUSED(label, iserrval, code) \
  do { \
    bool errval = (iserrval); \
    printf("%s: %s, errno %d\n", (label), errval ? "error value" : "a value", \
      meos_errno()); \
    assert(errval && meos_errno() == (code)); \
    meos_errno_reset(); \
  } while (0)

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The pcpoint and the pcpatch name pcid 1, whose schema lays out their
   * data */
  PCDimensionSpec dims[3] = {
    { "X", NULL, 1, "int32_t", 0.01, 0, true },
    { "Y", NULL, 2, "int32_t", 0.01, 0, true },
    { "Z", NULL, 3, "int32_t", 0.01, 0, true }
  };
  assert(meos_pc_schema_register_dims(1, 0, "none", dims, 3));

  /* A temporal float instant serves as the value of another type for every
   * function but tfloat_value, which takes a temporal integer instant */
  Temporal *other = tfloat_in("1.5@2001-01-01");
  Temporal *otherint = tint_in("1@2001-01-01");
  assert(other && otherint);

  /* Booleans */
  Temporal *inst = tbool_in("true@2001-01-01");
  Temporal *seq = tbool_in("{true@2001-01-01, false@2001-01-02}");
  bool b = tbool_value(inst);
  printf("tbool_value: %s, errno %d\n", b ? "true" : "false", meos_errno());
  assert(b && b == tbool_start_value(inst) && meos_errno() == 0);
  b = tbool_value(seq);
  REFUSED("tbool_value(sequence)", ! b, MEOS_ERR_INVALID_ARG_TYPE);
  b = tbool_value(NULL);
  REFUSED("tbool_value(NULL)", ! b, MEOS_ERR_INVALID_ARG);
  b = tbool_value(other);
  REFUSED("tbool_value(tfloat)", ! b, MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Integers */
  inst = tint_in("5@2001-01-01");
  seq = tint_in("[5@2001-01-01, 7@2001-01-02]");
  int i = tint_value(inst);
  printf("tint_value: %d, errno %d\n", i, meos_errno());
  assert(i == 5 && i == tint_start_value(inst) && meos_errno() == 0);
  i = tint_value(seq);
  REFUSED("tint_value(sequence)", i == INT_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  i = tint_value(NULL);
  REFUSED("tint_value(NULL)", i == INT_MAX, MEOS_ERR_INVALID_ARG);
  i = tint_value(other);
  REFUSED("tint_value(tfloat)", i == INT_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Big integers, one beyond the range of an integer */
  inst = tbigint_in("5000000000@2001-01-01");
  seq = tbigint_in("[5000000000@2001-01-01, 7@2001-01-02]");
  int64 bi = tbigint_value(inst);
  printf("tbigint_value: %ld, errno %d\n", (long) bi, meos_errno());
  assert(bi == 5000000000 && bi == tbigint_start_value(inst) &&
    meos_errno() == 0);
  bi = tbigint_value(seq);
  REFUSED("tbigint_value(sequence)", bi == INT64_MAX,
    MEOS_ERR_INVALID_ARG_TYPE);
  bi = tbigint_value(NULL);
  REFUSED("tbigint_value(NULL)", bi == INT64_MAX, MEOS_ERR_INVALID_ARG);
  bi = tbigint_value(other);
  REFUSED("tbigint_value(tfloat)", bi == INT64_MAX,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Floats */
  inst = tfloat_in("2.5@2001-01-01");
  seq = tfloat_in("[2.5@2001-01-01, 3.5@2001-01-02]");
  double d = tfloat_value(inst);
  printf("tfloat_value: %g, errno %d\n", d, meos_errno());
  assert(d == 2.5 && d == tfloat_start_value(inst) && meos_errno() == 0);
  d = tfloat_value(seq);
  REFUSED("tfloat_value(sequence)", d == DBL_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  d = tfloat_value(NULL);
  REFUSED("tfloat_value(NULL)", d == DBL_MAX, MEOS_ERR_INVALID_ARG);
  d = tfloat_value(otherint);
  REFUSED("tfloat_value(tint)", d == DBL_MAX, MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Texts */
  inst = ttext_in("\"AAA\"@2001-01-01");
  seq = ttext_in("[\"AAA\"@2001-01-01, \"BBB\"@2001-01-02]");
  text *txt = ttext_value(inst);
  text *txt2 = ttext_start_value(inst);
  char *str = text_out(txt);
  char *str2 = text_out(txt2);
  printf("ttext_value: %s, errno %d\n", str, meos_errno());
  assert(strcmp(str, "AAA") == 0 && strcmp(str, str2) == 0 &&
    meos_errno() == 0);
  free(txt); free(txt2); free(str); free(str2);
  REFUSED("ttext_value(sequence)", ttext_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("ttext_value(NULL)", ttext_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("ttext_value(tfloat)", ttext_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Geometry points, as for the four temporal geo types */
  inst = tgeompoint_in("Point(1 2)@2001-01-01");
  seq = tgeompoint_in("[Point(1 2)@2001-01-01, Point(3 4)@2001-01-02]");
  GSERIALIZED *gs = tgeo_value(inst);
  GSERIALIZED *gs2 = tgeo_start_value(inst);
  str = geo_as_text(gs, 6);
  printf("tgeo_value: %s, errno %d\n", str, meos_errno());
  assert(strcmp(str, "POINT(1 2)") == 0 && geo_same(gs, gs2) &&
    meos_errno() == 0);
  free(gs); free(gs2); free(str);
  REFUSED("tgeo_value(sequence)", tgeo_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tgeo_value(NULL)", tgeo_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tgeo_value(tfloat)", tgeo_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Circular buffers */
  inst = tcbuffer_in("Cbuffer(Point(1 1),0.5)@2001-01-01");
  seq = tcbuffer_in("[Cbuffer(Point(1 1),0.5)@2001-01-01, "
    "Cbuffer(Point(2 2),0.5)@2001-01-02]");
  Cbuffer *cb = tcbuffer_value(inst);
  Cbuffer *cb2 = tcbuffer_start_value(inst);
  str = cbuffer_out(cb, 6);
  printf("tcbuffer_value: %s, errno %d\n", str, meos_errno());
  assert(cbuffer_radius(cb) == 0.5 && cbuffer_eq(cb, cb2) &&
    meos_errno() == 0);
  free(cb); free(cb2); free(str);
  REFUSED("tcbuffer_value(sequence)", tcbuffer_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tcbuffer_value(NULL)", tcbuffer_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tcbuffer_value(tfloat)", tcbuffer_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* H3 cells */
  inst = th3index_in("880326b885fffff@2001-01-01");
  seq = th3index_in("{880326b885fffff@2001-01-01, 880326b88dfffff@2001-01-02}");
  H3Index h3 = th3index_value(inst);
  printf("th3index_value: %lx, errno %d\n", (unsigned long) h3, meos_errno());
  assert(h3 == 0x880326b885fffff && h3 == th3index_start_value(inst) &&
    meos_errno() == 0);
  REFUSED("th3index_value(sequence)", th3index_value(seq) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("th3index_value(NULL)", th3index_value(NULL) == 0,
    MEOS_ERR_INVALID_ARG);
  REFUSED("th3index_value(tfloat)", th3index_value(other) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* JSONB values */
  inst = tjsonb_in("\"{\\\"a\\\": 1}\"@2001-01-01");
  seq = tjsonb_in("{{\"a\": 1}@2001-01-01, {\"a\": 2}@2001-01-02}");
  Jsonb *jb = tjsonb_value(inst);
  Jsonb *jb2 = tjsonb_start_value(inst);
  str = jsonb_out(jb);
  printf("tjsonb_value: %s, errno %d\n", str, meos_errno());
  assert(strcmp(str, "{\"a\": 1}") == 0 && jsonb_eq(jb, jb2) &&
    meos_errno() == 0);
  free(jb); free(jb2); free(str);
  REFUSED("tjsonb_value(sequence)", tjsonb_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tjsonb_value(NULL)", tjsonb_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tjsonb_value(tfloat)", tjsonb_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Network points */
  inst = tnpoint_in("Npoint(1, 0.5)@2001-01-01");
  seq = tnpoint_in("[Npoint(1, 0.5)@2001-01-01, Npoint(1, 0.7)@2001-01-02]");
  Npoint *np = tnpoint_value(inst);
  Npoint *np2 = tnpoint_start_value(inst);
  str = npoint_out(np, 6);
  printf("tnpoint_value: %s, errno %d\n", str, meos_errno());
  assert(npoint_route(np) == 1 && npoint_position(np) == 0.5 &&
    npoint_eq(np, np2) && meos_errno() == 0);
  free(np); free(np2); free(str);
  REFUSED("tnpoint_value(sequence)", tnpoint_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tnpoint_value(NULL)", tnpoint_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tnpoint_value(tfloat)", tnpoint_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Point cloud points */
  inst = tpcpoint_in(PCPOINT_HEX "@2001-01-01");
  seq = tpcpoint_in("{" PCPOINT_HEX "@2001-01-01, " PCPOINT_HEX
    "@2001-01-02}");
  assert(inst && seq);
  Pcpoint *pt = tpcpoint_value(inst);
  Pcpoint *pt2 = tpcpoint_start_value(inst);
  printf("tpcpoint_value: pcid %u, errno %d\n", pcpoint_get_pcid(pt),
    meos_errno());
  assert(pcpoint_get_pcid(pt) == 1 && pcpoint_eq(pt, pt2) &&
    meos_errno() == 0);
  free(pt); free(pt2);
  REFUSED("tpcpoint_value(sequence)", tpcpoint_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tpcpoint_value(NULL)", tpcpoint_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tpcpoint_value(tfloat)", tpcpoint_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Point cloud patches */
  inst = tpcpatch_in(PCPATCH_HEX "@2001-01-01");
  seq = tpcpatch_in("{" PCPATCH_HEX "@2001-01-01, " PCPATCH_HEX
    "@2001-01-02}");
  assert(inst && seq);
  Pcpatch *pa = tpcpatch_value(inst);
  Pcpatch *pa2 = tpcpatch_start_value(inst);
  printf("tpcpatch_value: %u points, errno %d\n", pcpatch_npoints(pa),
    meos_errno());
  assert(pcpatch_npoints(pa) == 2 && pcpatch_eq(pa, pa2) &&
    meos_errno() == 0);
  free(pa); free(pa2);
  REFUSED("tpcpatch_value(sequence)", tpcpatch_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tpcpatch_value(NULL)", tpcpatch_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tpcpatch_value(tfloat)", tpcpatch_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Poses */
  inst = tpose_in("Pose(Point(1 1), 0.5)@2001-01-01");
  seq = tpose_in("[Pose(Point(1 1), 0.5)@2001-01-01, "
    "Pose(Point(2 2), 0.5)@2001-01-02]");
  Pose *pose = tpose_value(inst);
  Pose *pose2 = tpose_start_value(inst);
  str = pose_out(pose, 6);
  printf("tpose_value: %s, errno %d\n", str, meos_errno());
  assert(pose_eq(pose, pose2) && meos_errno() == 0);
  free(pose); free(pose2); free(str);
  REFUSED("tpose_value(sequence)", tpose_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tpose_value(NULL)", tpose_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tpose_value(tfloat)", tpose_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Pose chains, whose start value is the one value of an instant */
  int count;
  inst = tposechain_in("PoseChain(Pose(Point(0 0), 0), "
    "Pose(Point(1 0), 0))@2001-01-01");
  seq = tposechain_in("{PoseChain(Pose(Point(0 0), 0))@2001-01-01, "
    "PoseChain(Pose(Point(1 0), 0))@2001-01-02}");
  PoseChain *pc = tposechain_value(inst);
  PoseChain **pcs = tposechain_values(inst, &count);
  str = posechain_out(pc, 6);
  printf("tposechain_value: %s, errno %d\n", str, meos_errno());
  assert(count == 1 && posechain_num_poses(pc) == 2 &&
    posechain_eq(pc, pcs[0]) && meos_errno() == 0);
  free(pc); free(pcs[0]); free(pcs); free(str);
  REFUSED("tposechain_value(sequence)", tposechain_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tposechain_value(NULL)", tposechain_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tposechain_value(tfloat)", tposechain_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Quadbin cells */
  inst = tquadbin_in("480fffffffffffff@2001-01-01");
  seq = tquadbin_in("{480fffffffffffff@2001-01-01, "
    "48427fffffffffff@2001-01-02}");
  Quadbin qb = tquadbin_value(inst);
  printf("tquadbin_value: %lx, errno %d\n", (unsigned long) qb, meos_errno());
  assert(qb == 0x480fffffffffffff && qb == tquadbin_start_value(inst) &&
    meos_errno() == 0);
  REFUSED("tquadbin_value(sequence)", tquadbin_value(seq) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("tquadbin_value(NULL)", tquadbin_value(NULL) == 0,
    MEOS_ERR_INVALID_ARG);
  REFUSED("tquadbin_value(tfloat)", tquadbin_value(other) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* Rigid geometries, whose value is the pose of the reference geometry, as
   * their values function answers */
  inst = trgeometry_in("Polygon((0 0,1 0,1 1,0 1,0 0));"
    "Pose(Point(4 0), 0.0)@2001-01-01");
  seq = trgeometry_in("Polygon((0 0,1 0,1 1,0 1,0 0));"
    "{Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-02}");
  pose = trgeometry_value(inst);
  Pose **poses = trgeometry_values(inst, &count);
  str = pose_out(pose, 6);
  printf("trgeometry_value: %s, errno %d\n", str, meos_errno());
  assert(count == 1 && pose_eq(pose, poses[0]) && meos_errno() == 0);
  free(pose); free(poses[0]); free(poses); free(str);
  REFUSED("trgeometry_value(sequence)", trgeometry_value(seq) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("trgeometry_value(NULL)", trgeometry_value(NULL) == NULL,
    MEOS_ERR_INVALID_ARG);
  REFUSED("trgeometry_value(tfloat)", trgeometry_value(other) == NULL,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  /* S2 cells */
  inst = ts2cell_in("47c3c3@2001-01-01");
  seq = ts2cell_in("{47c3c3@2001-01-01, 54b5c9@2001-01-02}");
  S2CellId s2 = ts2cell_value(inst);
  printf("ts2cell_value: %lx, errno %d\n", (unsigned long) s2, meos_errno());
  assert(s2 != 0 && s2 == ts2cell_start_value(inst) && meos_errno() == 0);
  REFUSED("ts2cell_value(sequence)", ts2cell_value(seq) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  REFUSED("ts2cell_value(NULL)", ts2cell_value(NULL) == 0,
    MEOS_ERR_INVALID_ARG);
  REFUSED("ts2cell_value(tfloat)", ts2cell_value(other) == 0,
    MEOS_ERR_INVALID_ARG_TYPE);
  free(inst); free(seq);

  free(other); free(otherint);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
