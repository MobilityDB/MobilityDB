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
 * @brief A program that tests that no MEOS value holds a NaN
 * @details A NaN has no position among the numbers, so MEOS refuses it where
 * a number enters it (#ensure_not_nan, #ensure_not_nan_geo), and an operation
 * whose result would be NaN raises an error. The program reads a NaN through
 * each kind of entry point and calls the operations that make one from
 * infinite values, the inputs a float accepts, and verifies that each case
 * ends in an error: never a value holding a NaN, never a NaN result, never a
 * crash, nor a value returned together with an error. Each case runs in a child process, so a case that crashes is
 * reported as such and the others still run. The cases are drawn from the
 * census of `doc/contributing/open_questions/NAN-POLICY.md`:
 * - the text readers of the pose and circular buffer families, of box3d and
 *   of geometries;
 * - their binary readers, and those of float boxes and span sets, a value
 *   holding the sentinel 1234.5 being written as hex WKB and its eight bytes
 *   overwritten with a NaN;
 * - their MF-JSON readers, the sentinel overwritten with the bare token NaN;
 * - public functions taking a NaN double;
 * - operations over infinite floats whose result would be NaN.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o nan_refusal_test nan_refusal_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_rgeo.h>

static int failures = 0, cases = 0;

/* The outcome of a case: 0 an error, 1 a value, 2 a NaN, 3 a setup that
 * produced no sentinel to overwrite, so that the case would test nothing, 4 a
 * value returned with an error, 5 a result holding no NaN, which a case over
 * infinite values accepts, since infinity is a value of the floats */
typedef int (*case_fn)(void);

static void
check(const char *what, case_fn fn)
{
  cases++;
  fflush(stdout);
  pid_t pid = fork();
  if (pid == 0)
  {
    meos_errno_reset();
    int r = fn();
    int err = meos_errno();
    if (r == 3)
      _exit(3);
    if (r == 4)
      _exit(err ? 4 : 1);
    if (r == 5)
      _exit(0);
    /* An outcome without an error is a value or a NaN */
    _exit(err ? 0 : (r == 2 ? 2 : 1));
  }
  int st;
  waitpid(pid, &st, 0);
  const char *verdict = NULL;
  if (WIFSIGNALED(st))
    verdict = "crashes";
  else if (WEXITSTATUS(st) == 1)
    verdict = "accepts the NaN";
  else if (WEXITSTATUS(st) == 2)
    verdict = "returns NaN";
  else if (WEXITSTATUS(st) == 3)
    verdict = "could not be set up";
  else if (WEXITSTATUS(st) == 4)
    verdict = "raises an error and still returns a value";
  else if (WEXITSTATUS(st) != 0)
    verdict = "exits";
  if (verdict)
  {
    printf("  %s: %s\n", what, verdict);
    failures++;
  }
}

/* Replace the bytes of the sentinel 1234.5 in a hex WKB string by a NaN */
static bool
patch_hex(char *hex)
{
  char *p = hex ? strstr(hex, "00000000004A9340") : NULL;
  if (p)
    memcpy(p, "000000000000F87F", 16);
  return p != NULL;
}

/* Replace the sentinel 1234.5 in an MF-JSON document by the token NaN */
static bool
patch_json(char *doc)
{
  char *p = doc ? strstr(doc, "1234.5") : NULL;
  if (p)
  {
    memcpy(p, "NaN", 3);
    memmove(p + 3, p + 6, strlen(p + 6) + 1);
  }
  return p != NULL;
}

/* A case returning a value fails whatever the error number says: a reader
 * that raises an error and still returns a value hands its caller the NaN */
static int
result(void *v)
{
  free(v);
  return v ? 4 : 0;
}

static int
dresult(double d)
{
  return isnan(d) ? 2 : 5;
}

/* The outcome of a result case read in the text MEOS writes of the value */
static int
tresult(void *v, char *str)
{
  int r = ! v ? 0 : (str && strstr(str, "NaN")) ? 2 : 5;
  free(v); free(str);
  return r;
}
#define TEXT_RESULT(r, v, out) \
  do { __typeof__(v) _v = (v); \
    r = tresult(_v, _v ? out(_v, 15) : NULL); } while (0)

/* Text readers */
static int pose_text(void) { return result(pose_in("Pose(Point(NaN 1), 0.5)")); }
static int pose3d_text(void) { return result(pose_in("Pose(Point Z(1 1 NaN), 1, 0, 0, 0)")); }
static int posechain_text(void) { return result(posechain_in("PoseChain(Pose(Point(NaN 0), 0))")); }
static int tpose_text(void) { return result(tpose_in("Pose(Point(NaN 1), 0.5)@2001-01-01")); }
static int trgeo_text(void) { return result(trgeometry_in("Polygon((1 1,2 2,3 1,1 1));Pose(Point(NaN 1), 0.5)@2001-01-01")); }
static int cbuffer_text(void) { return result(cbuffer_in("Cbuffer(Point(NaN 1), 0.5)")); }
static int tcbuffer_text(void) { return result(tcbuffer_in("Cbuffer(Point(1 NaN), 0.5)@2001-01-01")); }
static int box3d_text(void) { return result(box3d_in("BOX3D(NaN 1 1,5 5 5)")); }

static int geom_text(void) { return result(geom_in("Linestring(1 1, NaN 2)", -1)); }
static int geog_text(void) { return result(geog_in("Point(4 NaN)", -1)); }
static int geo_wkt(void) { return result(geo_from_text("Point(NaN 1)", 0)); }

/* Binary readers */
static int
geom_wkb(void)
{
  GSERIALIZED *gs = geom_in("Linestring(1 1, 1234.5 2)", -1);
  char *hex = geo_as_hexewkb(gs, "NDR");
  free(gs);
  int r = patch_hex(hex) ? result(geom_from_hexewkb(hex)) : 3;
  free(hex);
  return r;
}

static int
cbuffer_wkb(void)
{
  Cbuffer *v = cbuffer_in("Cbuffer(Point(1234.5 1), 0.5)");
  size_t size;
  char *hex = cbuffer_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(cbuffer_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

static int
pose_wkb(void)
{
  Pose *v = pose_in("Pose(Point(1234.5 1), 0.5)");
  size_t size;
  char *hex = pose_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(pose_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

static int
tbox_wkb(void)
{
  TBox *v = tbox_in("TBOXFLOAT XT([1234.5, 2000],[2001-01-01, 2001-01-02])");
  size_t size;
  char *hex = tbox_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(tbox_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

static int
span_wkb(void)
{
  Span *v = floatspan_in("(1234.5, 2000)");
  size_t size;
  char *hex = span_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(span_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

static int
spanset_wkb(void)
{
  SpanSet *v = floatspanset_in("{(1234.5, 2000)}");
  size_t size;
  char *hex = spanset_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(spanset_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

static int
trgeo_wkb(void)
{
  Temporal *v = trgeometry_in("Polygon((1 1,1234.5 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01");
  size_t size;
  char *hex = temporal_as_hexwkb(v, 0x08 | 0x04, &size);
  free(v);
  int r = patch_hex(hex) ? result(temporal_from_hexwkb(hex)) : 3;
  free(hex);
  return r;
}

/* MF-JSON readers */
static int
tpose_mfjson(void)
{
  Temporal *t = tpose_in("Pose(Point(1234.5 1), 0.5)@2001-01-01");
  char *doc = temporal_as_mfjson(t, 0, 0, 15, NULL);
  free(t);
  int r = patch_json(doc) ? result(tpose_from_mfjson(doc)) : 3;
  free(doc);
  return r;
}

static int
tcbuffer_mfjson(void)
{
  Temporal *t = tcbuffer_in("Cbuffer(Point(1234.5 1), 0.5)@2001-01-01");
  char *doc = temporal_as_mfjson(t, 0, 0, 15, NULL);
  free(t);
  int r = patch_json(doc) ? result(tcbuffer_from_mfjson(doc)) : 3;
  free(doc);
  return r;
}

static int
trgeo_mfjson(void)
{
  Temporal *t = trgeometry_in("Polygon((1 1,1234.5 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01");
  char *doc = temporal_as_mfjson(t, 0, 0, 15, NULL);
  free(t);
  int r = patch_json(doc) ? result(trgeometry_from_mfjson(doc)) : 3;
  free(doc);
  return r;
}

/* Public functions taking a NaN double */
static int
teq_nan(void)
{
  Temporal *t = tfloat_in("[1@2001-01-01, 3@2001-01-02]");
  int r = result(teq_tfloat_float(t, NAN));
  free(t);
  return r;
}

static int
tdwithin_nan(void)
{
  Temporal *t = tgeompoint_in("[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]");
  GSERIALIZED *gs = geom_in("Point(1 1)", -1);
  int r = result(tdwithin_tgeo_geo(t, gs, NAN, false));
  free(t); free(gs);
  return r;
}

static int
rotate_nan(void)
{
  Temporal *t = tpose_in("[Pose(Point Z(1 1 1), 1, 0, 0, 0)@2001-01-01, "
    "Pose(Point Z(2 2 2), 1, 0, 0, 0)@2001-01-02]");
  int r = result(tpose_rotate_x(t, NAN));
  free(t);
  return r;
}

static int
line_nan(void)
{
  GSERIALIZED *gs = geom_in("Linestring(0 0, 3 3)", -1);
  int r = result(line_interpolate_point(gs, NAN, false));
  free(gs);
  return r;
}

static int
gbox_nan(void)
{
  return result(gbox_make(false, false, false, NAN, 1, 0, 1, 0, 0, 0, 0));
}

static int
shift_scale_nan(void)
{
  Span *s = floatspan_in("[1, 3]");
  int r = result(floatspan_shift_scale(s, NAN, 1, true, true));
  free(s);
  return r;
}

/* Operations over infinite floats whose result would be NaN */
static int
integral_inf(void)
{
  Temporal *t = tfloat_in("[-Infinity@2001-01-01, Infinity@2001-01-02]");
  int r = dresult(tnumber_integral(t));
  free(t);
  return r;
}

static int
frechet_inf(void)
{
  Temporal *t = tfloat_in("[-Infinity@2001-01-01, Infinity@2001-01-02]");
  int r = dresult(temporal_frechet_distance(t, t, false));
  free(t);
  return r;
}

static int
shift_scale_inf(void)
{
  Span *s = floatspan_in("[-Infinity, Infinity]");
  int r;
  TEXT_RESULT(r, floatspan_shift_scale(s, INFINITY, INFINITY, true, true),
    span_out);
  free(s);
  return r;
}

static int
linear_inf(void)
{
  return result(tfloat_in("[1@2001-01-01, Infinity@2001-01-02]"));
}

static int
step_degrees_inf(void)
{
  Temporal *t = tfloat_in("Interp=Step;[Infinity@2001-01-01, 1@2001-01-02]");
  int r;
  TEXT_RESULT(r, tfloat_degrees(t, true), temporal_out);
  free(t);
  return r;
}

static int
span_degrees_inf(void)
{
  Span *s = floatspan_in("[-Infinity, Infinity]");
  int r;
  TEXT_RESULT(r, floatspan_degrees(s, true), span_out);
  free(s);
  return r;
}

static int
set_degrees_inf(void)
{
  Set *s = floatset_in("{0, Infinity}");
  int r;
  TEXT_RESULT(r, floatset_degrees(s, true), set_out);
  free(s);
  return r;
}

static int degrees_inf(void) { return dresult(float_degrees(INFINITY, true)); }
static int bin_inf(void) { return dresult(float_get_bin(INFINITY, INFINITY, INFINITY)); }
static int angdiff_inf(void) { return dresult(float_angular_difference(INFINITY, INFINITY)); }

static int
expand_inf(void)
{
  Span *s = floatspan_in("[-Infinity, Infinity]");
  int r;
  TEXT_RESULT(r, floatspan_expand(s, -INFINITY), span_out);
  free(s);
  return r;
}

static int
box_shift_scale_inf(void)
{
  TBox *box = tbox_in("TBOXFLOAT XT([-Infinity, Infinity],[2001-01-01, 2001-01-02])");
  int r;
  TEXT_RESULT(r, tfloatbox_shift_scale(box, INFINITY, INFINITY, true, true),
    tbox_out);
  free(box);
  return r;
}

/* Main program */
int main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  printf("Text readers:\n");
  check("pose_in", pose_text);
  check("pose_in, spatial", pose3d_text);
  check("posechain_in", posechain_text);
  check("tpose_in", tpose_text);
  check("trgeometry_in", trgeo_text);
  check("cbuffer_in", cbuffer_text);
  check("tcbuffer_in", tcbuffer_text);
  check("box3d_in", box3d_text);
  check("geom_in", geom_text);
  check("geog_in", geog_text);
  check("geo_from_text", geo_wkt);
  printf("Binary readers:\n");
  check("geom_from_hexewkb", geom_wkb);
  check("cbuffer_from_hexwkb", cbuffer_wkb);
  check("pose_from_hexwkb", pose_wkb);
  check("tbox_from_hexwkb", tbox_wkb);
  check("span_from_hexwkb", span_wkb);
  check("spanset_from_hexwkb", spanset_wkb);
  check("temporal_from_hexwkb of a trgeometry", trgeo_wkb);
  printf("MF-JSON readers:\n");
  check("tpose_from_mfjson", tpose_mfjson);
  check("tcbuffer_from_mfjson", tcbuffer_mfjson);
  check("trgeometry_from_mfjson", trgeo_mfjson);
  printf("Functions taking a NaN:\n");
  check("teq_tfloat_float", teq_nan);
  check("tdwithin_tgeo_geo", tdwithin_nan);
  check("tpose_rotate_x", rotate_nan);
  check("line_interpolate_point", line_nan);
  check("gbox_make", gbox_nan);
  check("floatspan_shift_scale", shift_scale_nan);
  printf("Results over infinite floats:\n");
  check("tnumber_integral", integral_inf);
  check("temporal_frechet_distance", frechet_inf);
  check("floatspan_shift_scale", shift_scale_inf);
  check("tfloat_in, linear, infinite", linear_inf);
  check("tfloat_degrees, step, infinite", step_degrees_inf);
  check("floatspan_degrees", span_degrees_inf);
  check("floatset_degrees", set_degrees_inf);
  check("float_degrees", degrees_inf);
  check("float_get_bin", bin_inf);
  check("float_angular_difference", angdiff_inf);
  check("floatspan_expand", expand_inf);
  check("tfloatbox_shift_scale", box_shift_scale_inf);

  printf("%d of %d case(s) failed\n", failures, cases);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
