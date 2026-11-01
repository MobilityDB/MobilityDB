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
 * @brief A program that tests that every MEOS value is a finite number
 * @details NaN and the infinities have no position among the finite numbers,
 * so MEOS refuses them where a number enters it (#ensure_finite,
 * #ensure_finite_geo), and an operation of finite operands whose result
 * leaves the doubles raises an error (#ensure_finite_result). The program
 * reads NaN, +Infinity and -Infinity through each kind of entry point, and
 * calls operations over finite operands near the largest double, and
 * verifies that each case ends in an error: never a value holding a
 * non-finite number, never a non-finite result, never a crash, nor a value
 * returned together with an error. A result lying between finite values is
 * answered: a case expecting one checks its value. Each case runs in a child
 * process, so a case that crashes is reported as such and the others still
 * run. The cases are drawn from the census of
 * `doc/contributing/exactness/NAN-POLICY.md`:
 * - the text readers of the pose and circular buffer families, of boxes,
 *   spans, sets, temporal floats and geometries;
 * - their binary readers, and those of float boxes and span sets, a value
 *   holding the sentinel 1234.5 being written as hex WKB and its eight bytes
 *   overwritten with a non-finite double;
 * - their MF-JSON readers, the sentinel overwritten with a bare non-finite
 *   token;
 * - public functions taking a non-finite double;
 * - operations over finite operands whose result leaves the doubles, and
 *   operations whose intermediate leaves them while the result does not.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o nan_refusal_test nan_refusal_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <float.h>
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
#include <pg_interval.h>

static int failures = 0, cases = 0;

/* The outcome of a case: 0 an error, 1 a value, 2 a non-finite result, 3 a
 * setup that produced no sentinel to overwrite, so that the case would test
 * nothing, 4 a value returned with an error, 5 the expected finite value */
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
    /* An outcome without an error is a value or a non-finite result */
    _exit(err ? 0 : (r == 2 ? 2 : 1));
  }
  int st;
  waitpid(pid, &st, 0);
  const char *verdict = NULL;
  if (WIFSIGNALED(st))
    verdict = "crashes";
  else if (WEXITSTATUS(st) == 1)
    verdict = "accepts the value";
  else if (WEXITSTATUS(st) == 2)
    verdict = "returns a non-finite value";
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

/* The little-endian hex WKB of NaN, +Infinity and -Infinity */
static const char *HEX_NAN = "000000000000F87F";
static const char *HEX_PINF = "000000000000F07F";
static const char *HEX_NINF = "000000000000F0FF";

/* Replace the bytes of the sentinel 1234.5 in a hex WKB string by those of a
 * non-finite double */
static bool
patch_hex(char *hex, const char *bytes)
{
  char *p = hex ? strstr(hex, "00000000004A9340") : NULL;
  if (p)
    memcpy(p, bytes, 16);
  return p != NULL;
}

/* Replace the sentinel 1234.5 in an MF-JSON document by a token, the
 * document growing where the token is longer than the sentinel */
static bool
patch_json(char **doc, const char *token)
{
  char *p = *doc ? strstr(*doc, "1234.5") : NULL;
  if (! p)
    return false;
  size_t n = strlen(token), off = (size_t) (p - *doc);
  if (n > 6)
  {
    *doc = realloc(*doc, strlen(*doc) + n - 6 + 1);
    p = *doc + off;
  }
  memmove(p + n, p + 6, strlen(p + 6) + 1);
  memcpy(p, token, n);
  return true;
}

/* A case returning a value fails whatever the error number says: a reader
 * that raises an error and still returns a value hands its caller the
 * non-finite number */
static int
result(void *v)
{
  free(v);
  return v ? 4 : 0;
}

/* The outcome of a double result that only an error may accompany */
static int
dresult(double d)
{
  return isfinite(d) ? 1 : 2;
}

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

static int pose_inf_text(void) { return result(pose_in("Pose(Point(Infinity 1), 0.5)")); }
static int cbuffer_inf_text(void) { return result(cbuffer_in("Cbuffer(Point(1 1), Infinity)")); }
static int tcbuffer_inf_text(void) { return result(tcbuffer_in("Cbuffer(Point(1 -Infinity), 0.5)@2001-01-01")); }
static int box3d_inf_text(void) { return result(box3d_in("BOX3D(1 1 1,5 5 Infinity)")); }
static int geom_inf_text(void) { return result(geom_in("Point(-Infinity 1)", -1)); }
static int geom_big_text(void) { return result(geom_in("Linestring(1 1, 1e400 2)", -1)); }
static int geog_inf_text(void) { return result(geog_in("Point(4 Infinity)", -1)); }
static int stbox_inf_text(void) { return result(stbox_in("STBOX X((1,1),(Infinity,2))")); }
static int tbox_inf_text(void) { return result(tbox_in("TBOXFLOAT XT([-Infinity, Infinity],[2001-01-01, 2001-01-02])")); }
static int floatspan_inf_text(void) { return result(floatspan_in("[-Infinity, Infinity]")); }
static int floatspan_big_text(void) { return result(floatspan_in("[1, 1e400]")); }
static int floatset_inf_text(void) { return result(floatset_in("{0, Infinity}")); }
static int tfloat_inf_text(void) { return result(tfloat_in("[1@2001-01-01, Infinity@2001-01-02]")); }
static int tfloat_step_inf_text(void) { return result(tfloat_in("Interp=Step;[Infinity@2001-01-01, 1@2001-01-02]")); }
static int tfloat_big_text(void) { return result(tfloat_in("-1e400@2001-01-01")); }

/* Binary readers, the sentinel overwritten by each non-finite double */
#define WKB_CASES(name, setup, writer, reader, freefn) \
  static int name##_wkb(const char *bytes) \
  { \
    __typeof__(setup) v = (setup); \
    size_t size; \
    char *hex = writer; \
    freefn(v); \
    int r = patch_hex(hex, bytes) ? result(reader(hex)) : 3; \
    free(hex); \
    return r; \
  } \
  static int name##_wkb_nan(void) { return name##_wkb(HEX_NAN); } \
  static int name##_wkb_pinf(void) { return name##_wkb(HEX_PINF); } \
  static int name##_wkb_ninf(void) { return name##_wkb(HEX_NINF); }

static int
geom_wkb(const char *bytes)
{
  GSERIALIZED *gs = geom_in("Linestring(1 1, 1234.5 2)", -1);
  char *hex = geo_as_hexewkb(gs, "NDR");
  free(gs);
  int r = patch_hex(hex, bytes) ? result(geom_from_hexewkb(hex)) : 3;
  free(hex);
  return r;
}
static int geom_wkb_nan(void) { return geom_wkb(HEX_NAN); }
static int geom_wkb_pinf(void) { return geom_wkb(HEX_PINF); }
static int geom_wkb_ninf(void) { return geom_wkb(HEX_NINF); }

WKB_CASES(cbuffer, cbuffer_in("Cbuffer(Point(1234.5 1), 0.5)"),
  cbuffer_as_hexwkb(v, 0x08 | 0x04, &size), cbuffer_from_hexwkb, free)
WKB_CASES(pose, pose_in("Pose(Point(1234.5 1), 0.5)"),
  pose_as_hexwkb(v, 0x08 | 0x04, &size), pose_from_hexwkb, free)
WKB_CASES(tbox, tbox_in("TBOXFLOAT XT([1234.5, 2000],[2001-01-01, 2001-01-02])"),
  tbox_as_hexwkb(v, 0x08 | 0x04, &size), tbox_from_hexwkb, free)
WKB_CASES(span, floatspan_in("(1234.5, 2000)"),
  span_as_hexwkb(v, 0x08 | 0x04, &size), span_from_hexwkb, free)
WKB_CASES(spanset, floatspanset_in("{(1234.5, 2000)}"),
  spanset_as_hexwkb(v, 0x08 | 0x04, &size), spanset_from_hexwkb, free)
WKB_CASES(trgeo, trgeometry_in("Polygon((1 1,1234.5 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01"),
  temporal_as_hexwkb(v, 0x08 | 0x04, &size), temporal_from_hexwkb, free)
WKB_CASES(tfloat, tfloat_in("[1234.5@2001-01-01, 2@2001-01-02]"),
  temporal_as_hexwkb(v, 0x08 | 0x04, &size), temporal_from_hexwkb, free)

/* MF-JSON readers, the sentinel overwritten by each non-finite token */
#define MFJSON_CASES(name, literal, reader) \
  static int name##_mfjson(const char *token) \
  { \
    Temporal *t = reader##_in(literal); \
    char *doc = temporal_as_mfjson(t, 0, 0, 15, NULL); \
    free(t); \
    int r = patch_json(&doc, token) ? result(reader##_from_mfjson(doc)) : 3; \
    free(doc); \
    return r; \
  } \
  static int name##_mfjson_nan(void) { return name##_mfjson("NaN"); } \
  static int name##_mfjson_pinf(void) { return name##_mfjson("Infinity"); } \
  static int name##_mfjson_ninf(void) { return name##_mfjson("-Infinity"); }

MFJSON_CASES(tpose, "Pose(Point(1234.5 1), 0.5)@2001-01-01", tpose)
MFJSON_CASES(tcbuffer, "Cbuffer(Point(1234.5 1), 0.5)@2001-01-01", tcbuffer)
MFJSON_CASES(trgeo, "Polygon((1 1,1234.5 2,3 1,1 1));Pose(Point(1 1), 0.5)@2001-01-01", trgeometry)
MFJSON_CASES(tfloat, "[1234.5@2001-01-01, 2@2001-01-02]", tfloat)
MFJSON_CASES(tgeompoint, "[Point(1234.5 0)@2001-01-01, Point(2 0)@2001-01-02]", tgeompoint)

/* Public functions taking a non-finite double */
static int
teq_double(double d)
{
  Temporal *t = tfloat_in("[1@2001-01-01, 3@2001-01-02]");
  int r = result(teq_tfloat_float(t, d));
  free(t);
  return r;
}
static int teq_nan(void) { return teq_double(NAN); }
static int teq_inf(void) { return teq_double(INFINITY); }

static int
tdwithin_double(double d)
{
  Temporal *t = tgeompoint_in("[Point(1 1)@2001-01-01, Point(2 2)@2001-01-02]");
  GSERIALIZED *gs = geom_in("Point(1 1)", -1);
  int r = result(tdwithin_tgeo_geo(t, gs, d, false));
  free(t); free(gs);
  return r;
}
static int tdwithin_nan(void) { return tdwithin_double(NAN); }
static int tdwithin_inf(void) { return tdwithin_double(INFINITY); }

static int
rotate_double(double d)
{
  Temporal *t = tpose_in("[Pose(Point Z(1 1 1), 1, 0, 0, 0)@2001-01-01, "
    "Pose(Point Z(2 2 2), 1, 0, 0, 0)@2001-01-02]");
  int r = result(tpose_rotate_x(t, d));
  free(t);
  return r;
}
static int rotate_nan(void) { return rotate_double(NAN); }
static int rotate_inf(void) { return rotate_double(-INFINITY); }

static int
line_double(double d)
{
  GSERIALIZED *gs = geom_in("Linestring(0 0, 3 3)", -1);
  int r = result(line_interpolate_point(gs, d, false));
  free(gs);
  return r;
}
static int line_nan(void) { return line_double(NAN); }
static int line_inf(void) { return line_double(INFINITY); }

static int gbox_nan(void) { return result(gbox_make(false, false, false, NAN, 1, 0, 1, 0, 0, 0, 0)); }
static int gbox_inf(void) { return result(gbox_make(false, false, false, 0, INFINITY, 0, 1, 0, 0, 0, 0)); }

static int
shift_scale_double(double shift, double width)
{
  Span *s = floatspan_in("[1, 3]");
  int r = result(floatspan_shift_scale(s, shift, width, true, true));
  free(s);
  return r;
}
static int shift_scale_nan(void) { return shift_scale_double(NAN, 1); }
static int shift_scale_inf(void) { return shift_scale_double(1, INFINITY); }

static int
expand_double(double d)
{
  Span *s = floatspan_in("[1, 3]");
  int r = result(floatspan_expand(s, d));
  free(s);
  return r;
}
static int expand_inf(void) { return expand_double(-INFINITY); }

static int
box_shift_scale_double(double d)
{
  TBox *box = tbox_in("TBOXFLOAT XT([1, 3],[2001-01-01, 2001-01-02])");
  int r = result(tfloatbox_shift_scale(box, d, d, true, true));
  free(box);
  return r;
}
static int box_shift_scale_inf(void) { return box_shift_scale_double(INFINITY); }

static int
interval_mul_double(double d)
{
  Interval *i = interval_in("1 day", -1);
  int r = result(mul_interval_float8(i, d));
  free(i);
  return r;
}
static int interval_mul_nan(void) { return interval_mul_double(NAN); }
static int interval_mul_inf(void) { return interval_mul_double(INFINITY); }

static int
interval_div_double(double d)
{
  Interval *i = interval_in("1 day", -1);
  int r = result(div_interval_float8(i, d));
  free(i);
  return r;
}
static int interval_div_inf(void) { return interval_div_double(-INFINITY); }

static int degrees_inf(void) { return dresult(float_degrees(INFINITY, true)); }
static int bin_inf(void) { return dresult(float_get_bin(INFINITY, 1, 0)); }
static int angdiff_inf(void) { return dresult(float_angular_difference(INFINITY, 1)); }
static int round_inf(void) { return dresult(float_round(-INFINITY, 2)); }

/* Results of finite operands that leave the doubles */
static int
integral_far(void)
{
  Temporal *t = tfloat_in("[1e308@2001-01-01, 1e308@2001-01-02]");
  int r = dresult(tnumber_integral(t));
  free(t);
  return r;
}

static int
box_expand_far(void)
{
  TBox *box = tbox_in("TBOXFLOAT XT([-1.7e308, 1.7e308],[2001-01-01, 2001-01-02])");
  int r = result(tfloatbox_expand(box, DBL_MAX));
  free(box);
  return r;
}

static int value_tile_far(void) { return result(tbox_get_value_tile(DBL_MAX, DBL_MAX, DBL_MAX)); }

static int
exp_far(void)
{
  Temporal *t = tfloat_in("[-1.7e308@2001-01-01, 1.7e308@2001-01-02]");
  int r = result(tfloat_exp(t));
  free(t);
  return r;
}

static int
distance_far(void)
{
  GSERIALIZED *g1 = geom_in("Point(-1e308 0)", -1);
  GSERIALIZED *g2 = geom_in("Linestring(1e308 -1,1e308 1)", -1);
  int r = dresult(geom_distance2d(g1, g2));
  free(g1); free(g2);
  return r;
}

static int
cbuffer_distance_far(void)
{
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(-1e308 0), 1)");
  GSERIALIZED *gs = geom_in("Point(1e308 0)", -1);
  int r = dresult(distance_cbuffer_geo(cb, gs));
  free(cb); free(gs);
  return r;
}

/* Results within the doubles whose intermediate leaves them */
static int round_far(void) { return float_round(DBL_MAX, 15) == DBL_MAX ? 5 : 1; }
static int round_many_digits(void) { return float_round(1.5, 400) == 1.5 ? 5 : 1; }

static int
cbuffer_offset_far(void)
{
  Cbuffer *cb = cbuffer_in("Cbuffer(Point(-1e308 0), 1.5e308)");
  GSERIALIZED *gs = geom_in("Point(1e308 0)", -1);
  double d = distance_cbuffer_geo(cb, gs);
  free(cb); free(gs);
  /* The exact distance 2e308 - 1.5e308 of the input doubles */
  return d == 0x1.1ccf385ebc8ap+1022 ? 5 : 1;
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
  check("pose_in, infinite", pose_inf_text);
  check("cbuffer_in, infinite radius", cbuffer_inf_text);
  check("tcbuffer_in, infinite", tcbuffer_inf_text);
  check("box3d_in, infinite", box3d_inf_text);
  check("geom_in, infinite", geom_inf_text);
  check("geom_in, 1e400", geom_big_text);
  check("geog_in, infinite", geog_inf_text);
  check("stbox_in, infinite", stbox_inf_text);
  check("tbox_in, infinite", tbox_inf_text);
  check("floatspan_in, infinite", floatspan_inf_text);
  check("floatspan_in, 1e400", floatspan_big_text);
  check("floatset_in, infinite", floatset_inf_text);
  check("tfloat_in, linear, infinite", tfloat_inf_text);
  check("tfloat_in, step, infinite", tfloat_step_inf_text);
  check("tfloat_in, -1e400", tfloat_big_text);
  printf("Binary readers:\n");
  check("geom_from_hexewkb, NaN", geom_wkb_nan);
  check("geom_from_hexewkb, +Infinity", geom_wkb_pinf);
  check("geom_from_hexewkb, -Infinity", geom_wkb_ninf);
  check("cbuffer_from_hexwkb, NaN", cbuffer_wkb_nan);
  check("cbuffer_from_hexwkb, +Infinity", cbuffer_wkb_pinf);
  check("cbuffer_from_hexwkb, -Infinity", cbuffer_wkb_ninf);
  check("pose_from_hexwkb, NaN", pose_wkb_nan);
  check("pose_from_hexwkb, +Infinity", pose_wkb_pinf);
  check("pose_from_hexwkb, -Infinity", pose_wkb_ninf);
  check("tbox_from_hexwkb, NaN", tbox_wkb_nan);
  check("tbox_from_hexwkb, +Infinity", tbox_wkb_pinf);
  check("tbox_from_hexwkb, -Infinity", tbox_wkb_ninf);
  check("span_from_hexwkb, NaN", span_wkb_nan);
  check("span_from_hexwkb, +Infinity", span_wkb_pinf);
  check("span_from_hexwkb, -Infinity", span_wkb_ninf);
  check("spanset_from_hexwkb, NaN", spanset_wkb_nan);
  check("spanset_from_hexwkb, +Infinity", spanset_wkb_pinf);
  check("spanset_from_hexwkb, -Infinity", spanset_wkb_ninf);
  check("temporal_from_hexwkb of a trgeometry, NaN", trgeo_wkb_nan);
  check("temporal_from_hexwkb of a trgeometry, +Infinity", trgeo_wkb_pinf);
  check("temporal_from_hexwkb of a trgeometry, -Infinity", trgeo_wkb_ninf);
  check("temporal_from_hexwkb of a tfloat, NaN", tfloat_wkb_nan);
  check("temporal_from_hexwkb of a tfloat, +Infinity", tfloat_wkb_pinf);
  check("temporal_from_hexwkb of a tfloat, -Infinity", tfloat_wkb_ninf);
  printf("MF-JSON readers:\n");
  check("tpose_from_mfjson, NaN", tpose_mfjson_nan);
  check("tpose_from_mfjson, Infinity", tpose_mfjson_pinf);
  check("tpose_from_mfjson, -Infinity", tpose_mfjson_ninf);
  check("tcbuffer_from_mfjson, NaN", tcbuffer_mfjson_nan);
  check("tcbuffer_from_mfjson, Infinity", tcbuffer_mfjson_pinf);
  check("tcbuffer_from_mfjson, -Infinity", tcbuffer_mfjson_ninf);
  check("trgeometry_from_mfjson, NaN", trgeo_mfjson_nan);
  check("trgeometry_from_mfjson, Infinity", trgeo_mfjson_pinf);
  check("trgeometry_from_mfjson, -Infinity", trgeo_mfjson_ninf);
  check("tfloat_from_mfjson, NaN", tfloat_mfjson_nan);
  check("tfloat_from_mfjson, Infinity", tfloat_mfjson_pinf);
  check("tfloat_from_mfjson, -Infinity", tfloat_mfjson_ninf);
  check("tgeompoint_from_mfjson, NaN", tgeompoint_mfjson_nan);
  check("tgeompoint_from_mfjson, Infinity", tgeompoint_mfjson_pinf);
  check("tgeompoint_from_mfjson, -Infinity", tgeompoint_mfjson_ninf);
  printf("Functions taking a non-finite double:\n");
  check("teq_tfloat_float, NaN", teq_nan);
  check("teq_tfloat_float, +Infinity", teq_inf);
  check("tdwithin_tgeo_geo, NaN", tdwithin_nan);
  check("tdwithin_tgeo_geo, +Infinity", tdwithin_inf);
  check("tpose_rotate_x, NaN", rotate_nan);
  check("tpose_rotate_x, -Infinity", rotate_inf);
  check("line_interpolate_point, NaN", line_nan);
  check("line_interpolate_point, +Infinity", line_inf);
  check("gbox_make, NaN", gbox_nan);
  check("gbox_make, +Infinity", gbox_inf);
  check("floatspan_shift_scale, NaN", shift_scale_nan);
  check("floatspan_shift_scale, +Infinity", shift_scale_inf);
  check("floatspan_expand, -Infinity", expand_inf);
  check("tfloatbox_shift_scale, +Infinity", box_shift_scale_inf);
  check("mul_interval_float8, NaN", interval_mul_nan);
  check("mul_interval_float8, +Infinity", interval_mul_inf);
  check("div_interval_float8, -Infinity", interval_div_inf);
  check("float_degrees, +Infinity", degrees_inf);
  check("float_get_bin, +Infinity", bin_inf);
  check("float_angular_difference, +Infinity", angdiff_inf);
  check("float_round, -Infinity", round_inf);
  printf("Results of finite operands beyond the doubles:\n");
  check("tnumber_integral", integral_far);
  check("tfloatbox_expand", box_expand_far);
  check("tbox_get_value_tile", value_tile_far);
  check("tfloat_exp", exp_far);
  check("geom_distance2d", distance_far);
  check("distance_cbuffer_geo", cbuffer_distance_far);
  printf("Results within the doubles from an intermediate beyond them:\n");
  check("float_round of the largest double", round_far);
  check("float_round to 400 decimal places", round_many_digits);
  check("distance_cbuffer_geo less a radius", cbuffer_offset_far);

  printf("%d of %d case(s) failed\n", failures, cases);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
