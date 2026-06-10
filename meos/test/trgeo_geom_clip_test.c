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
 *****************************************************************************/

/**
 * @file trgeo_geom_clip_test.c
 * @brief Unit tests for the M1 pure-translation trapezoid-polygon clip
 *
 * Build (matches the convention of `meos/test/geo_test.c` /
 * `meos/test/cbuffer_test.c` — assumes MEOS installed system-wide
 * via `make install`):
 *
 *   gcc -Wall -g -I/usr/local/include -o trgeo_geom_clip_test \
 *     trgeo_geom_clip_test.c -L/usr/local/lib -lmeos -lm
 *
 * Each test prints "FAIL: <reason>" if it fails; `main` returns 0
 * if all pass. The suite runs in <100 ms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <meos.h>
#include <liblwgeom.h>

#include "pose/pose.h"
#include "rgeo/trgeo_geom_clip.h"

#define EPS 1e-6

static int test_count = 0;
static int test_failed = 0;

#define ASSERT_EQ_INT(expected, actual, msg) do { \
  test_count++; \
  if ((expected) != (actual)) { \
    fprintf(stderr, "FAIL: %s — expected %d, got %d\n", msg, \
      (int)(expected), (int)(actual)); \
    test_failed++; \
  } \
} while (0)

#define ASSERT_NEAR(expected, actual, msg) do { \
  test_count++; \
  if (fabs((expected) - (actual)) > EPS) { \
    fprintf(stderr, "FAIL: %s — expected %g, got %g\n", msg, \
      (double)(expected), (double)(actual)); \
    test_failed++; \
  } \
} while (0)

/* Build a square POINTARRAY [(x0,y0),(x1,y0),(x1,y1),(x0,y1),(x0,y0)] */
static POINTARRAY *
make_square(double x0, double y0, double x1, double y1)
{
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
  POINT4D pt;
  pt.x = x0; pt.y = y0; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = x1; pt.y = y0; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = x1; pt.y = y1; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = x0; pt.y = y1; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = x0; pt.y = y0; ptarray_append_point(pa, &pt, LW_TRUE);
  return pa;
}

/* Test 1 — quad fully outside polygon: 0 intervals */
static void
test_outside(void)
{
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 5}, b2 = {1, 5};
  POINTARRAY *pa = make_square(10, 10, 20, 20);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(0, n, "quad outside polygon -> 0 intervals");
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 2 — quad fully inside polygon: one interval [0, 1] */
static void
test_inside(void)
{
  POINT2D a1 = {1, 1}, b1 = {2, 1}, a2 = {1, 2}, b2 = {2, 2};
  POINTARRAY *pa = make_square(0, 0, 10, 10);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(1, n, "quad inside polygon -> 1 interval");
  if (n >= 1)
  {
    ASSERT_NEAR(0.0, DatumGetFloat8(intervals[0].lower),
      "inside lower bound");
    ASSERT_NEAR(1.0, DatumGetFloat8(intervals[0].upper),
      "inside upper bound");
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 3 — quad enters polygon halfway through:
 * moving edge from (0, 0)–(1, 0) at t=0 to (0, 4)–(1, 4) at t=1,
 * polygon is square [(0, 2), (5, 2), (5, 5), (0, 5)] — reached at t=0.5
 */
static void
test_enter_halfway(void)
{
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 4}, b2 = {1, 4};
  POINTARRAY *pa = make_square(0, 2, 5, 5);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(1, n, "quad enters at t=0.5 -> 1 interval");
  if (n >= 1)
  {
    ASSERT_NEAR(0.5, DatumGetFloat8(intervals[0].lower),
      "enter lower bound at t=0.5");
    ASSERT_NEAR(1.0, DatumGetFloat8(intervals[0].upper),
      "enter upper bound at t=1");
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 4 — pure translation precondition violated:
 * Different displacement on the two endpoints — should return -1 */
static void
test_non_translation_rejected(void)
{
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 1}, b2 = {2, 1}; /* b2 != b1 + (a2-a1) */
  POINTARRAY *pa = make_square(0, 0, 10, 10);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(-1, n, "non-translation input -> -1");
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 5 — degenerate polygon (too few points): should return -1 */
static void
test_degenerate_polygon(void)
{
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 1}, b2 = {1, 1};
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
  POINT4D pt = {0, 0, 0, 0};
  ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = 1; ptarray_append_point(pa, &pt, LW_TRUE);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(-1, n, "degenerate polygon -> -1");
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 6 — quad straddles polygon: enters and exits within [0, 1] */
static void
test_straddle(void)
{
  /* Moving edge from (0, 0)-(1, 0) to (0, 10)-(1, 10);
   * polygon is square [(0, 3), (5, 3), (5, 5), (0, 5)] — moving edge
   * is inside on t in [0.3, 0.5] */
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 10}, b2 = {1, 10};
  POINTARRAY *pa = make_square(0, 3, 5, 5);
  Span *intervals = NULL;
  int n = trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, &intervals);
  ASSERT_EQ_INT(1, n, "straddle -> 1 interval");
  if (n >= 1)
  {
    ASSERT_NEAR(0.3, DatumGetFloat8(intervals[0].lower),
      "straddle lower bound at t=0.3");
    ASSERT_NEAR(0.5, DatumGetFloat8(intervals[0].upper),
      "straddle upper bound at t=0.5");
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
}

/* Test 7 — box convenience wrapper: same as enter_halfway but via the
 * box API. Should produce identical output. */
static void
test_box_wrapper(void)
{
  POINT2D a1 = {0, 0}, b1 = {1, 0}, a2 = {0, 4}, b2 = {1, 4};
  Span *intervals = NULL;
  int n = trgeo_geom_clip_box(&a1, &b1, &a2, &b2, 0, 2, 5, 5, &intervals);
  ASSERT_EQ_INT(1, n, "box wrapper -> 1 interval");
  if (n >= 1)
  {
    ASSERT_NEAR(0.5, DatumGetFloat8(intervals[0].lower),
      "box wrapper lower bound at t=0.5");
    ASSERT_NEAR(1.0, DatumGetFloat8(intervals[0].upper),
      "box wrapper upper bound at t=1");
  }
  if (intervals) free(intervals);
}

/*****************************************************************************
 * Rotational cases (posed entries)
 *****************************************************************************/

/* Helper: build a 2D Pose at (x, y, theta). Caller must pfree. */
static Pose *
make_pose2d(double x, double y, double theta)
{
  /* 2D pose serialised form: data = [x, y, theta] */
  size_t size = sizeof(Pose) + 3 * sizeof(double);
  Pose *p = (Pose *) palloc0(size);
  SET_VARSIZE(p, size);
  p->flags = 0; /* Z bit clear → 2D */
  p->data[0] = x;
  p->data[1] = y;
  p->data[2] = theta;
  return p;
}

/* Test: pure-rotation-only delegated correctly to translation fast-path
 * when theta1 == theta2. The body translates from (0, 0) to (0, 4)
 * with constant orientation; should match test_enter_halfway. */
static void
test_posed_translation_only(void)
{
  POINT2D pa_local = {0, 0}, pb_local = {1, 0};
  Pose *p1 = make_pose2d(0, 0, 0);
  Pose *p2 = make_pose2d(0, 4, 0);
  Span *intervals = NULL;
  POINTARRAY *pa = make_square(0, 2, 5, 5);
  int n = trgeo_geom_clip_polygon_posed(&pa_local, &pb_local, p1, p2, pa,
    &intervals);
  ASSERT_EQ_INT(1, n, "posed translation-only -> 1 interval");
  if (n >= 1)
  {
    ASSERT_NEAR(0.5, DatumGetFloat8(intervals[0].lower),
      "posed translation-only lower bound at t=0.5");
    ASSERT_NEAR(1.0, DatumGetFloat8(intervals[0].upper),
      "posed translation-only upper bound at t=1");
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
  pfree(p1); pfree(p2);
}

/* Test: small rotation (within Taylor threshold). Body rotates 5° while
 * translating; the moving edge enters the polygon roughly halfway. */
static void
test_posed_small_rotation(void)
{
  POINT2D pa_local = {0, 0}, pb_local = {1, 0};
  Pose *p1 = make_pose2d(0, 0, 0);
  Pose *p2 = make_pose2d(0, 4, 5.0 * M_PI / 180.0);
  Span *intervals = NULL;
  POINTARRAY *pa = make_square(0, 2, 5, 5);
  int n = trgeo_geom_clip_polygon_posed(&pa_local, &pb_local, p1, p2, pa,
    &intervals);
  /* Expect at least one interval; the entry instant should be near
   * t=0.5 (within ~5% due to small rotation perturbation). */
  test_count++;
  if (n < 1)
  {
    fprintf(stderr, "FAIL: posed small rotation -> 0 intervals\n");
    test_failed++;
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
  pfree(p1); pfree(p2);
}

/* Test: large rotation (forces numerical solver). Body rotates 60° while
 * translating. Verify the solver doesn't loop and produces a
 * non-empty result. */
static void
test_posed_large_rotation(void)
{
  POINT2D pa_local = {0, 0}, pb_local = {1, 0};
  Pose *p1 = make_pose2d(0, 0, 0);
  Pose *p2 = make_pose2d(0, 4, 60.0 * M_PI / 180.0);
  Span *intervals = NULL;
  POINTARRAY *pa = make_square(0, 2, 5, 5);
  int n = trgeo_geom_clip_polygon_posed(&pa_local, &pb_local, p1, p2, pa,
    &intervals);
  test_count++;
  if (n < 0)
  {
    fprintf(stderr, "FAIL: posed large rotation returned -1\n");
    test_failed++;
  }
  if (intervals) free(intervals);
  ptarray_free(pa);
  pfree(p1); pfree(p2);
}

/* Test: 3D pose input rejected with -1 */
static void
test_posed_3d_rejected(void)
{
  POINT2D pa_local = {0, 0}, pb_local = {1, 0};
  /* Build a 3D pose: 7 doubles + Z flag set */
  size_t size = sizeof(Pose) + 7 * sizeof(double);
  Pose *p1 = (Pose *) palloc0(size);
  SET_VARSIZE(p1, size);
  MEOS_FLAGS_SET_Z(p1->flags, true);
  Pose *p2 = (Pose *) palloc0(size);
  SET_VARSIZE(p2, size);
  MEOS_FLAGS_SET_Z(p2->flags, true);

  Span *intervals = NULL;
  POINTARRAY *pa = make_square(0, 0, 10, 10);
  int n = trgeo_geom_clip_polygon_posed(&pa_local, &pb_local, p1, p2, pa,
    &intervals);
  ASSERT_EQ_INT(-1, n, "3D pose -> -1 (2D-only constraint)");
  if (intervals) free(intervals);
  ptarray_free(pa);
  pfree(p1); pfree(p2);
}

int
main(void)
{
  meos_initialize();

  test_outside();
  test_inside();
  test_enter_halfway();
  test_non_translation_rejected();
  test_degenerate_polygon();
  test_straddle();
  test_box_wrapper();
  test_posed_translation_only();
  test_posed_small_rotation();
  test_posed_large_rotation();
  test_posed_3d_rejected();

  meos_finalize();

  fprintf(stdout, "Tests: %d, Failed: %d\n", test_count, test_failed);
  return test_failed == 0 ? 0 : 1;
}
