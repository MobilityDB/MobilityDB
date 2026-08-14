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
 * @brief A program that tests the OGC GeoPose encoding of poses and temporal
 * poses.
 *
 * MobilityDB implements four of the eight conformance classes of OGC GeoPose
 * v1.0 (OGC 21-056r11): Basic-YPR and Basic-Quaternion for a pose, and the
 * Regular and Irregular Composite Sequence Series for a temporal pose. The
 * program exercises each of them through the four functions the C API exposes,
 * and covers the error paths that a malformed document reaches.
 *
 * The conformance class of a temporal pose follows its value: a single instant
 * yields a Basic document, a sequence whose instants are equally spaced yields
 * a Regular Series, and any other sequence yields an Irregular Series.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geopose_test geopose_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>

/* Conformance classes of a Basic document, as the second argument of the
 * functions encoding a pose */
#define BASIC_QUATERNION 0
#define BASIC_YPR        1

/**
 * @brief Report a document and assert that it carries a member
 */
static void
ensure_member(const char *what, const char *json, const char *member)
{
  printf("%s: %s\n", what, json);
  assert(strstr(json, member) != NULL);
  return;
}

/**
 * @brief Report a document and assert that it carries no such member
 */
static void
ensure_no_member(const char *what, const char *json, const char *member)
{
  printf("%s: carries no `%s`\n", what, member);
  assert(strstr(json, member) == NULL);
  return;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  printf("****************************************************************\n");
  printf("OGC GeoPose 1.0 encoding\n");
  printf("****************************************************************\n");

  /*--------------------------------------------------------------------------
   * Basic-Quaternion, the class a pose takes by default
   *------------------------------------------------------------------------*/

  const char *quat_json = "{\"position\":{\"lat\":47,\"lon\":8,\"h\":1500},"
    "\"quaternion\":{\"x\":0,\"y\":0,\"z\":0.7071067811865476,"
    "\"w\":0.7071067811865476}}";

  meos_errno_reset();
  Pose *quat = pose_from_geopose(quat_json);
  assert(quat != NULL);
  assert(meos_errno() == 0);

  char *out = pose_as_geopose(quat, BASIC_QUATERNION, 6);
  assert(out != NULL);
  ensure_member("Basic-Quaternion", out, "\"quaternion\"");
  ensure_member("Basic-Quaternion", out, "\"position\"");
  /* The position of a Basic document is always three-dimensional */
  ensure_member("Basic-Quaternion", out, "\"h\"");
  free(out);

  /* The same pose seen as Basic-YPR carries the Euler angles instead */
  out = pose_as_geopose(quat, BASIC_YPR, 6);
  assert(out != NULL);
  ensure_member("Basic-YPR view", out, "\"angles\"");
  ensure_member("Basic-YPR view", out, "\"yaw\"");
  ensure_no_member("Basic-YPR view", out, "\"quaternion\"");
  free(out);

  /* The round trip preserves the pose */
  out = pose_as_geopose(quat, BASIC_QUATERNION, 15);
  Pose *quat_back = pose_from_geopose(out);
  assert(quat_back != NULL);
  char *s1 = pose_out(quat, 6);
  char *s2 = pose_out(quat_back, 6);
  printf("round trip: %s -> %s\n", s1, s2);
  assert(strcmp(s1, s2) == 0);
  free(s1); free(s2); free(out); free(quat_back); free(quat);

  /*--------------------------------------------------------------------------
   * Basic-YPR, and the two-dimensional pose it also encodes
   *------------------------------------------------------------------------*/

  meos_errno_reset();
  Pose *ypr = pose_from_geopose("{\"position\":{\"lat\":47,\"lon\":8,\"h\":0},"
    "\"angles\":{\"yaw\":90,\"pitch\":0,\"roll\":0}}");
  assert(ypr != NULL);
  assert(meos_errno() == 0);
  out = pose_as_geopose(ypr, BASIC_YPR, 6);
  ensure_member("Basic-YPR", out, "\"angles\"");
  free(out); free(ypr);

  /* A yaw-only document without a height is the two-dimensional pose of a
   * trajectory that tracks neither elevation nor tilt */
  meos_errno_reset();
  Pose *flat = pose_from_geopose("{\"position\":{\"lat\":0,\"lon\":0},"
    "\"angles\":{\"yaw\":90,\"pitch\":0,\"roll\":0}}");
  assert(flat != NULL);
  assert(meos_errno() == 0);
  s1 = pose_out(flat, 6);
  printf("two-dimensional pose: %s\n", s1);
  free(s1); free(flat);

  /* The identity quaternion is the pose of a body aligned with its frame */
  meos_errno_reset();
  Pose *ident = pose_from_geopose("{\"position\":{\"lat\":0,\"lon\":0,\"h\":0},"
    "\"quaternion\":{\"x\":0,\"y\":0,\"z\":0,\"w\":1}}");
  assert(ident != NULL);
  assert(meos_errno() == 0);
  out = pose_as_geopose(ident, BASIC_QUATERNION, 6);
  ensure_member("identity quaternion", out, "\"w\":1");
  free(out); free(ident);

  /*--------------------------------------------------------------------------
   * A single instant yields a Basic document
   *------------------------------------------------------------------------*/

  meos_errno_reset();
  Temporal *inst = tpose_in("Geodpose(Point(8 47), 0)@2026-01-01");
  assert(inst != NULL);
  assert(meos_errno() == 0);
  out = tpose_as_geopose(inst, BASIC_QUATERNION, 6);
  assert(out != NULL);
  ensure_member("single instant", out, "\"quaternion\"");
  ensure_no_member("single instant", out, "\"header\"");
  free(out); free(inst);

  /*--------------------------------------------------------------------------
   * Equally spaced instants yield a Regular Series
   *------------------------------------------------------------------------*/

  /* The poses turn a corner, so that none of them lies on the segment
   * between its neighbours and the normalisation of the sequence keeps all
   * three: a Series carries as many poses as the value holds */
  meos_errno_reset();
  Temporal *regular = tpose_in("[Geodpose(Point(0 0), 0)@2026-01-01, "
    "Geodpose(Point(1 0), 0)@2026-01-02, Geodpose(Point(1 1), 0)@2026-01-03]");
  assert(regular != NULL);
  assert(meos_errno() == 0);
  s1 = temporal_out(regular, 6);
  printf("Regular value: %s\n", s1);
  free(s1);

  out = tpose_as_geopose(regular, BASIC_QUATERNION, 6);
  assert(out != NULL);
  /* The four members the Series schema requires, and the duration that only
   * the Regular class carries */
  ensure_member("Regular Series", out, "\"header\"");
  ensure_member("Regular Series", out, "\"outerFrame\"");
  ensure_member("Regular Series", out, "\"innerFrameSeries\"");
  ensure_member("Regular Series", out, "\"trailer\"");
  ensure_member("Regular Series", out, "\"interPoseDuration\"");
  /* The header of a Series holds the count, the bounds and the model of the
   * transition between poses */
  ensure_member("Regular Series", out, "\"poseCount\":3");
  ensure_member("Regular Series", out, "\"startInstant\"");
  ensure_member("Regular Series", out, "\"stopInstant\"");
  ensure_member("Regular Series", out, "\"transitionModel\"");
  /* A linear interpolation is the `interpolate` literal of the standard's
   * TransitionModel enumeration */
  ensure_member("Regular Series", out, "\"interpolate\"");

  free(out);

  /* The inner frames of a Series hold a rotation against its outer frame, so
   * a two-dimensional pose comes back three-dimensional and the value read
   * back is not the value written. What the round trip does preserve is the
   * document: encoding the value read back reproduces it member for member,
   * so nothing of the Series is lost on the way through.
   *
   * The reproduction holds up to eleven digits. A Series spends its digits on
   * metres from the tangent point rather than on degrees, and the conversion
   * through the geocentric frame and back leaves about a nanometre of
   * floating-point round-off, which a twelfth digit exposes. */
  out = tpose_as_geopose(regular, BASIC_QUATERNION, 6);
  assert(out != NULL);
  Temporal *regular_back = tpose_from_geopose(out);
  assert(regular_back != NULL);
  assert(meos_errno() == 0);
  assert(temporal_num_instants(regular_back) == 3);
  char *again = tpose_as_geopose(regular_back, BASIC_QUATERNION, 6);
  assert(again != NULL);
  printf("Regular re-encoding is stable: %s\n",
    strcmp(out, again) == 0 ? "yes" : "no");
  assert(strcmp(out, again) == 0);
  free(again); free(out); free(regular_back); free(regular);

  /*--------------------------------------------------------------------------
   * Unequally spaced instants yield an Irregular Series
   *------------------------------------------------------------------------*/

  meos_errno_reset();
  Temporal *irregular = tpose_in("[Geodpose(Point(0 0), 0)@2026-01-01, "
    "Geodpose(Point(1 0), 0)@2026-01-02, Geodpose(Point(1 1), 0)@2026-01-05]");
  assert(irregular != NULL);
  assert(meos_errno() == 0);
  s1 = temporal_out(irregular, 6);
  printf("Irregular value: %s\n", s1);
  free(s1);

  out = tpose_as_geopose(irregular, BASIC_QUATERNION, 6);
  assert(out != NULL);
  ensure_member("Irregular Series", out, "\"header\"");
  ensure_member("Irregular Series", out, "\"outerFrame\"");
  ensure_member("Irregular Series", out, "\"innerFrameAndTimeSeries\"");
  ensure_member("Irregular Series", out, "\"trailer\"");
  /* Each element of an Irregular Series times its own frame */
  ensure_member("Irregular Series", out, "\"validTime\"");
  ensure_no_member("Irregular Series", out, "\"interPoseDuration\"");

  free(out);

  out = tpose_as_geopose(irregular, BASIC_QUATERNION, 6);
  assert(out != NULL);
  Temporal *irregular_back = tpose_from_geopose(out);
  assert(irregular_back != NULL);
  assert(meos_errno() == 0);
  assert(temporal_num_instants(irregular_back) == 3);
  again = tpose_as_geopose(irregular_back, BASIC_QUATERNION, 6);
  assert(again != NULL);
  printf("Irregular re-encoding is stable: %s\n",
    strcmp(out, again) == 0 ? "yes" : "no");
  assert(strcmp(out, again) == 0);
  /* The instants a Series times are the instants of the value it came from */
  s1 = temporal_out(irregular_back, 6);
  printf("Irregular read back: %s\n", s1);
  assert(strstr(s1, "2026-01-05") != NULL);
  free(s1); free(again); free(out); free(irregular_back); free(irregular);

  /*--------------------------------------------------------------------------
   * The error paths
   *------------------------------------------------------------------------*/

  printf("****************************************************************\n");
  printf("Error paths\n");
  printf("****************************************************************\n");

  /* A document that is not JSON at all */
  meos_errno_reset();
  Pose *bad = pose_from_geopose("this is not JSON");
  printf("pose_from_geopose(not JSON): %s, errno %d\n",
    bad ? "non-NULL" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);

  /* A document carrying neither a quaternion nor the Euler angles belongs to
   * no Basic conformance class */
  meos_errno_reset();
  bad = pose_from_geopose("{\"position\":{\"lat\":0,\"lon\":0,\"h\":0}}");
  printf("pose_from_geopose(no orientation): %s, errno %d\n",
    bad ? "non-NULL" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);

  /* A document without a position */
  meos_errno_reset();
  bad = pose_from_geopose("{\"quaternion\":{\"x\":0,\"y\":0,\"z\":0,\"w\":1}}");
  printf("pose_from_geopose(no position): %s, errno %d\n",
    bad ? "non-NULL" : "NULL", meos_errno());
  assert(bad == NULL);
  assert(meos_errno() != 0);

  /* Every class of the standard places its pose in a topocentric frame on
   * the surface of the Earth, which a planar pose does not have */
  meos_errno_reset();
  Pose *planar = pose_in("Pose(Point(1 1),0.5)");
  assert(planar != NULL);
  char *flat_json = pose_as_geopose(planar, BASIC_QUATERNION, 6);
  printf("pose_as_geopose(planar pose): %s, errno %d\n",
    flat_json ? "non-NULL" : "NULL", meos_errno());
  assert(flat_json == NULL);
  assert(meos_errno() != 0);
  free(planar);

  meos_errno_reset();
  Temporal *planar_t = tpose_in("Pose(Point(8 47), 0)@2026-01-01");
  assert(planar_t != NULL);
  flat_json = tpose_as_geopose(planar_t, BASIC_QUATERNION, 6);
  printf("tpose_as_geopose(planar pose): %s, errno %d\n",
    flat_json ? "non-NULL" : "NULL", meos_errno());
  assert(flat_json == NULL);
  assert(meos_errno() != 0);
  free(planar_t);

  /* An unknown conformance class */
  meos_errno_reset();
  Pose *good = pose_in("Geodpose(Point(1 1),0.5)");
  assert(good != NULL);
  char *none = pose_as_geopose(good, 7, 6);
  printf("pose_as_geopose(conformance 7): %s, errno %d\n",
    none ? "non-NULL" : "NULL", meos_errno());
  assert(none == NULL);
  assert(meos_errno() != 0);
  free(good);

  /* A Series whose header carries none of the members the schema requires */
  meos_errno_reset();
  Temporal *torn = tpose_from_geopose("{\"header\":{\"poseCount\":99}}");
  printf("tpose_from_geopose(incomplete header): %s, errno %d\n",
    torn ? "non-NULL" : "NULL", meos_errno());
  assert(torn == NULL);
  assert(meos_errno() != 0);

  printf("****************************************************************\n");
  printf("All GeoPose assertions hold\n");
  printf("****************************************************************\n");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
