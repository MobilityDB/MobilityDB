/*****************************************************************************
 *
 * Verification harness for the OGC GeoPose JSON I/O on `pose`.
 * Exercises both Basic-Quaternion and Basic-YPR conformance classes,
 * plus the 2D-pose path and the documented error cases.
 *
 * Build:
 *   gcc -Wall -g -O2 -I<prefix>/include -o geopose_test geopose_test.c \
 *     -L<prefix>/lib -lmeos
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_pose.h>

static int
roundtrip(const char *name, const char *json_in, int conformance,
  int precision, const char *expected_substring)
{
  Pose *p = pose_from_geopose(json_in);
  if (! p)
  {
    printf("[FAIL] %s — pose_from_geopose returned NULL\n  in: %s\n",
      name, json_in);
    return 1;
  }
  char *out = pose_as_geopose(p, conformance, precision);
  if (! out)
  {
    free(p);
    printf("[FAIL] %s — pose_as_geopose returned NULL\n", name);
    return 1;
  }
  int pass = expected_substring ?
    (strstr(out, expected_substring) != NULL) : 1;
  printf("%s %s\n  in:       %s\n  out (cls=%d, p=%d): %s\n  expects: %s\n",
    pass ? "[PASS]" : "[FAIL]", name, json_in, conformance, precision, out,
    expected_substring ? expected_substring : "(any)");
  free(out);
  free(p);
  return pass ? 0 : 1;
}

/* Error paths are exercised by the SQL test fixture
 * (mobilitydb/test/pose/queries/103_pose_geopose.test.sql) where
 * PostgreSQL's exception handler can trap each ereport(ERROR, …).
 * The standalone MEOS harness can't run them without a custom error
 * callback because the default handler exits the process. */

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int fails = 0;

  /* OGC canonical Basic-Quaternion: 90° yaw at lat=47, lon=8, h=1500. */
  fails += roundtrip("Basic-Quaternion 90° yaw",
    "{\"position\":{\"lat\":47,\"lon\":8,\"h\":1500},"
    "\"quaternion\":{\"x\":0,\"y\":0,\"z\":0.7071067811865476,"
    "\"w\":0.7071067811865476}}",
    0, 6, "\"z\":0.707107");

  /* Identity quaternion. */
  fails += roundtrip("Identity quaternion",
    "{\"position\":{\"lat\":0,\"lon\":0,\"h\":0},"
    "\"quaternion\":{\"x\":0,\"y\":0,\"z\":0,\"w\":1}}",
    0, 6, "\"w\":1");

  /* Round-trip Basic-Quaternion → Basic-YPR. */
  fails += roundtrip("Basic-Quaternion → YPR view",
    "{\"position\":{\"lat\":47,\"lon\":8,\"h\":1500},"
    "\"quaternion\":{\"x\":0,\"y\":0,\"z\":0.7071067811865476,"
    "\"w\":0.7071067811865476}}",
    1, 6, "\"yaw\":90");

  /* Basic-YPR (yaw-only). */
  fails += roundtrip("Basic-YPR yaw=90°",
    "{\"position\":{\"lat\":47,\"lon\":8,\"h\":1500},"
    "\"angles\":{\"yaw\":90,\"pitch\":0,\"roll\":0}}",
    0, 6, "\"z\":0.707107");

  /* Basic-YPR (all three angles). */
  fails += roundtrip("Basic-YPR yaw=30 pitch=45 roll=60",
    "{\"position\":{\"lat\":47,\"lon\":8,\"h\":1500},"
    "\"angles\":{\"yaw\":30,\"pitch\":45,\"roll\":60}}",
    1, 6, "\"yaw\":30");

  /* 2D pose: missing h, missing pitch+roll → stored as 2D. The output
   * fills in h:0 and pitch=roll=0 to keep a complete Basic-class JSON. */
  fails += roundtrip("2D pose (yaw-only, no h)",
    "{\"position\":{\"lat\":50.85,\"lon\":4.35},"
    "\"angles\":{\"yaw\":45}}",
    1, 6, "\"yaw\":45");

  meos_finalize();
  printf("\n%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
