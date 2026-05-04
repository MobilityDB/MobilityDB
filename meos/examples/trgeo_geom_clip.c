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
 * @file trgeo_geom_clip.c
 * @brief Worked MEOS example: swept-edge-polygon clip on a moving rigid body.
 *
 * Illustrates how to invoke the trgeo_geom_clip primitive from C-side
 * MEOS code. A small triangular body translates and rotates between
 * three poses; the program reports the time intervals during which any
 * edge of the body crosses a static target polygon.
 *
 * Build (matches the convention of other examples in this folder):
 *
 *   gcc -Wall -g -I/usr/local/include -o trgeo_geom_clip \
 *     trgeo_geom_clip.c -L/usr/local/lib -lmeos -lm
 *
 * The composition pattern shown here is what a future
 * `trgeo_at_geom` / `trgeo_traversed_area` migration would use
 * internally: walk segments, walk body edges, accumulate parameter
 * intervals, map back to time. It also serves as the template for the
 * cbuffer counterpart (`tcbuffer_geom_clip`) when that lands — same
 * outer loop, body shape replaced.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <meos.h>
#include <liblwgeom.h>

#include "pose/pose.h"
#include "rgeo/trgeo_geom_clip.h"

/* Small helper: build a 2D Pose. */
static Pose *
make_pose2d(double x, double y, double theta)
{
  size_t size = sizeof(Pose) + 3 * sizeof(double);
  Pose *p = (Pose *) palloc0(size);
  SET_VARSIZE(p, size);
  p->flags = 0;
  p->data[0] = x;
  p->data[1] = y;
  p->data[2] = theta;
  return p;
}

/* Print intervals to stdout, mapping [0, 1] back to a real time window. */
static void
print_intervals(const char *label, Span *intervals, int n,
  double t_real_a, double t_real_b)
{
  printf("  %s: ", label);
  if (n <= 0) { printf("(none)\n"); return; }
  for (int i = 0; i < n; i++)
  {
    double u_a = DatumGetFloat8(intervals[i].lower);
    double u_b = DatumGetFloat8(intervals[i].upper);
    double t_a = t_real_a + u_a * (t_real_b - t_real_a);
    double t_b = t_real_a + u_b * (t_real_b - t_real_a);
    printf("[%.3f, %.3f] (real time [%.2f, %.2f])%s",
      u_a, u_b, t_a, t_b, (i + 1 == n) ? "\n" : ", ");
  }
}

int
main(void)
{
  meos_initialize();

  /* Body polygon (in body-local frame): a unit triangle */
  POINT2D body[3] = {
    {0.0, 0.0},
    {1.0, 0.0},
    {0.0, 1.0}
  };
  const int nverts = 3;

  /* Target static polygon (a square offset from origin) */
  POINTARRAY *target = ptarray_construct_empty(0, 0, 5);
  POINT4D pt;
  pt.z = 0; pt.m = 0;
  pt.x = 2; pt.y = 2; ptarray_append_point(target, &pt, LW_TRUE);
  pt.x = 5; pt.y = 2; ptarray_append_point(target, &pt, LW_TRUE);
  pt.x = 5; pt.y = 5; ptarray_append_point(target, &pt, LW_TRUE);
  pt.x = 2; pt.y = 5; ptarray_append_point(target, &pt, LW_TRUE);
  pt.x = 2; pt.y = 2; ptarray_append_point(target, &pt, LW_TRUE);

  /* Three poses: at t=0, t=1, t=2 (real-time scale) */
  Pose *p0 = make_pose2d(0.0, 0.0, 0.0);                    /* origin */
  Pose *p1 = make_pose2d(3.0, 3.0, M_PI / 6.0);             /* mid-traversal, 30° */
  Pose *p2 = make_pose2d(6.0, 6.0, M_PI / 3.0);             /* exit, 60° */

  printf("Swept-edge-polygon clip — moving triangle vs. static square\n");
  printf("=========================================================\n");
  printf("Body: triangle [(0,0), (1,0), (0,1)] in body-local frame\n");
  printf("Target polygon: square [(2,2), (5,2), (5,5), (2,5)]\n");
  printf("Pose at t=0.0: (0,0,0°)\n");
  printf("Pose at t=1.0: (3,3,30°)\n");
  printf("Pose at t=2.0: (6,6,60°)\n\n");

  /* For each segment (pair of consecutive poses), each body edge,
   * call the kernel and print results. */
  Pose *segs[3] = {p0, p1, p2};
  double seg_times[3] = {0.0, 1.0, 2.0};

  for (int s = 0; s < 2; s++)
  {
    printf("Segment t=[%.1f, %.1f]:\n", seg_times[s], seg_times[s + 1]);
    for (int e = 0; e < nverts; e++)
    {
      int next_e = (e + 1) % nverts;
      Span *intervals = NULL;
      int n = trgeo_geom_clip_polygon_posed(
        &body[e], &body[next_e],
        segs[s], segs[s + 1],
        target, &intervals);

      char label[32];
      snprintf(label, sizeof(label), "edge %d->%d", e, next_e);
      print_intervals(label, intervals, n, seg_times[s], seg_times[s + 1]);
      if (intervals) free(intervals);
    }
    printf("\n");
  }

  ptarray_free(target);
  pfree(p0); pfree(p1); pfree(p2);
  meos_finalize();
  return 0;
}
