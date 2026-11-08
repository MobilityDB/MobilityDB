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
 * @brief Test of the clip of a moving edge by a polygon of
 * meos/src/rgeo/trgeo_geom_clip.c
 * @details A triangle moves between three 2D poses, translating and
 * rotating, across a square.  For each segment between two poses and each
 * edge of the triangle, #trgeometry_geom_clip_polygon_posed answers the
 * intervals of the segment, as fractions in [0, 1], during which the edge
 * meets the square.  The test places the edge at NSAMPLES + 1 fractions of
 * the segment, with the poses interpolated by #posesegm_interpolate, and
 * compares whether the fraction lies in an answered interval with whether
 * the 2D distance of liblwgeom from the placed edge to the square is zero.
 * A fraction closer than TOL to an end of an interval is skipped.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <meos.h>
#include <meos_pose.h>
#include <liblwgeom.h>

#include "pose/pose.h"
#include "rgeo/trgeo_geom_clip.h"

#define NSAMPLES 1000
#define TOL 1e-6

/* Place a point of the frame of the body under a 2D pose */
static POINT4D
place(const Pose *pose, const POINT2D *p)
{
  double c = cos(pose->data[2]), s = sin(pose->data[2]);
  POINT4D r = {pose->data[0] + p->x * c - p->y * s,
    pose->data[1] + p->x * s + p->y * c, 0.0, 0.0};
  return r;
}

/* Whether the edge from a to b of the body placed under the pose meets the
 * area of the polygon */
static bool
placed_edge_meets(const Pose *pose, const POINT2D *a, const POINT2D *b,
  const LWPOLY *poly)
{
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
  POINT4D pa_a = place(pose, a), pa_b = place(pose, b);
  ptarray_append_point(pa, &pa_a, LW_TRUE);
  ptarray_append_point(pa, &pa_b, LW_TRUE);
  LWLINE *line = lwline_construct(0, NULL, pa);
  double d = lwgeom_mindistance2d(lwline_as_lwgeom(line),
    lwpoly_as_lwgeom(poly));
  lwline_free(line);
  return d == 0.0;
}

int
main(void)
{
  meos_initialize();

  /* The body, a unit triangle in its own frame */
  const POINT2D body[3] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}};
  /* The static square */
  const double corners[5][2] = {{2, 2}, {5, 2}, {5, 5}, {2, 5}, {2, 2}};
  POINTARRAY *ring = ptarray_construct_empty(0, 0, 5);
  for (int i = 0; i < 5; i++)
  {
    POINT4D pt = {corners[i][0], corners[i][1], 0.0, 0.0};
    ptarray_append_point(ring, &pt, LW_TRUE);
  }
  /* The polygon owns its rings and frees them */
  POINTARRAY **rings = lwalloc(sizeof(POINTARRAY *));
  rings[0] = ring;
  LWPOLY *square = lwpoly_construct(0, NULL, 1, rings);
  /* The poses at the three instants, 0, 30 and 60 degrees */
  Pose *poses[3] = {
    pose_make_2d(0.0, 0.0, 0.0, false, 0),
    pose_make_2d(3.0, 3.0, M_PI / 6.0, false, 0),
    pose_make_2d(6.0, 6.0, M_PI / 3.0, false, 0)};

  int nintervals = 0, nin = 0, nout = 0, nbad = 0;
  for (int s = 0; s < 2; s++)
  {
    for (int e = 0; e < 3; e++)
    {
      const POINT2D *a = &body[e], *b = &body[(e + 1) % 3];
      Span *iv = NULL;
      int n = trgeometry_geom_clip_polygon_posed(a, b, poses[s],
        poses[s + 1], ring, &iv);
      printf("segment %d, edge %d->%d:", s, e, (e + 1) % 3);
      if (n < 0)
      {
        printf(" input refused\n");
        nbad++;
        continue;
      }
      for (int i = 0; i < n; i++)
        printf(" [%.6f, %.6f]", floatspan_lower(&iv[i]),
          floatspan_upper(&iv[i]));
      printf("%s\n", n ? "" : " none");
      nintervals += n;
      for (int k = 0; k <= NSAMPLES; k++)
      {
        double t = (double) k / NSAMPLES;
        bool in = false, near = false;
        for (int i = 0; i < n; i++)
        {
          double lo = floatspan_lower(&iv[i]), hi = floatspan_upper(&iv[i]);
          if (t >= lo && t <= hi)
            in = true;
          if (fabs(t - lo) < TOL || fabs(t - hi) < TOL)
            near = true;
        }
        if (near)
          continue;
        Pose *pose = posesegm_interpolate(poses[s], poses[s + 1], t);
        bool meets = placed_edge_meets(pose, a, b, square);
        free(pose);
        if (in)
          nin++;
        else
          nout++;
        if (in != meets)
        {
          printf("  at %.6f the clip answers %s, the placed edge %s\n", t,
            in ? "inside" : "outside", meets ? "meets" : "misses");
          nbad++;
        }
      }
      free(iv);
    }
  }
  printf("%d intervals, %d placements inside and %d outside, %d disagree\n",
    nintervals, nin, nout, nbad);

  lwpoly_free(square);
  for (int i = 0; i < 3; i++)
    free(poses[i]);
  meos_finalize();
  /* Both sides must be sampled, or the comparison proves nothing */
  return (nbad == 0 && nin > 0 && nout > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
