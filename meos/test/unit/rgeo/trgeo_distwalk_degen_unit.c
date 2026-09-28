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
 * @brief Degenerate cases of the closest-feature walk of
 * meos/src/rgeo/trgeo_distwalk.c
 * @details Random segments almost never give these cases, but they are the
 * cases where a walk is most likely to fail: a point on a boundary between
 * two regions at the start or at the end of a segment, a point that moves
 * along such a boundary, edges that stay parallel, a vertex that meets a
 * vertex, a touch without overlap, and overlaps at the start, at the end or
 * during the whole segment.  The test compares each case with the true
 * distance, see #tu_check().
 *
 * The test also checks that the walk cleans the reference rings: it removes
 * repeated vertices and vertices on the line between their neighbors,
 * orients the ring counterclockwise, and refuses a ring that is not convex,
 * also one with a spike.
 *
 * Last, a polygon with 5000 vertices turns by almost half a turn.  The walk
 * takes about one step for each vertex that it passes, and it must stay
 * within its limit of steps.
 *
 * An optional argument runs only the case with that index.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "trgeo_distwalk_testutil.h"

extern void meos_initialize(void);
extern void meos_finalize(void);

typedef struct
{
  const char *name;
  int n; double va[16];      /* Body A */
  int m; double vb[16];      /* Body B, a segment if m = 2, a point if m = 1 */
  double a[6];               /* Poses of A: x1 y1 th1 x2 y2 th2 */
  double b[6];               /* Poses of B */
} Case;

#define R2 1.4142135623730951
#define SQ {-1,-1, 1,-1, 1,1, -1,1}
#define TRI {-1,-1, 1,-1, 0,1}
#define PT 1, {0,0}

static Case cases[] = {
  /* A square and a point */
  {"point: no rotation", 4, SQ, PT, {-6,0,0, 6,0,0}, {3,5,0, 3,5,0}},
  {"point: collinear vertex in the ring", 5, {-1,-1, 0,-1, 1,-1, 1,1, -1,1},
    PT, {0,0,0, 0,0,0}, {-3,-3,0, 3,-3,0}},
  {"point: almost collinear vertex", 5,
    {-1,-1, 0,-1.000000001, 1,-1, 1,1, -1,1}, PT,
    {0,0,0, 0,0,0}, {-3,-3,0, 3,-3,0}},
  {"point: starts on a region boundary", 4, SQ, PT,
    {0,0,0, 0,0,0}, {2,-1,0, 2,1,0}},
  {"point: ends on a region boundary", 4, SQ, PT,
    {0,0,0, 0,0,0}, {2,1,0, 2,-1,0}},
  {"point: almost tangent to a vertex", 4, SQ, PT,
    {0,0,0, 0,0,M_PI/2}, {R2+0.001,0,0, R2+0.001,0,0}},
  {"point: parallel to an edge", 4, SQ, PT,
    {0,0,0, 0,0,0}, {-3,-3,0, 3,-3,0}},
  {"point: rotation of exactly pi", 4, SQ, PT,
    {0,0,0, 0,0,M_PI}, {3,0,0, 3,0,0}},
  {"point: rotation of exactly pi/2", 4, SQ, PT,
    {0,0,0, 0,0,M_PI/2}, {3,0,0, 3,0,0}},
  {"point: extremum at t = 0", 4, SQ, PT, {0,0,0, 0,6,0}, {0,-3,0, 0,-3,0}},
  {"point: extremum at t = 1", 4, SQ, PT, {0,6,0, 0,0,0}, {0,-3,0, 0,-3,0}},
  {"point: no motion", 4, SQ, PT, {0,0,0, 0,0,0}, {3,0,0, 3,0,0}},
  {"point: touches the line of an edge", 4, SQ, PT,
    {0,0,-0.4, 0,0,0.4}, {0,R2,0, 0,R2,0}},
  {"point: change of feature at t = 1", 4, SQ, PT,
    {0,0,0, 0,0,0}, {2,-3,0, 2,-1,0}},
  {"point: touches a vertex", 4, SQ, PT,
    {0,0,0, 0,0,M_PI/2}, {R2,0,0, R2,0,0}},
  {"point: body passes over it", 4, SQ, PT, {-6,0,0, 6,0,0}, {3,0,0, 3,0,0}},
  {"point: starts inside", 4, SQ, PT, {0,0,0, 0,0,0}, {0,0,0, 3,0.5,0}},
  {"point: ends inside", 4, SQ, PT, {0,0,0, 0,0,0}, {3,0.5,0, 0,0,0}},
  {"point: inside for the whole segment", 4, SQ, PT,
    {0,0,0, 0,0,0.3}, {0.2,0.1,0, -0.3,0.4,0}},
  {"point: touches an edge from inside", 4, SQ, PT,
    {0,0,-0.4, 0,0,0.4}, {1,0,0, 1,0,0}},
  {"point: rotating body passes over it", 4, SQ, PT,
    {-6,0,0, 6,0,M_PI/2}, {0,0.5,0, 0,0.5,0}},
  {"point: through a vertex", 4, SQ, PT, {0,0,0, 0,0,0}, {3,-1,0, -1,3,0}},
  {"point: along the line of an edge", 4, SQ, PT,
    {0,0,0, 0,0,0}, {-3,-1,0, 3,-1,0}},
  {"point: along a diagonal", 4, SQ, PT, {0,0,0, 0,0,0}, {-3,-3,0, 3,3,0}},
  /* A square and a segment with its middle at the origin */
  {"segment: parallel to an edge", 4, SQ, 2, {-1,0, 1,0},
    {0,0,0, 0,0,0}, {-4,3,0, 4,3,0}},
  {"segment: end touches an edge", 4, SQ, 2, {0,-1, 0,1},
    {0,0,0, 0,0,0}, {-3,2,0, 3,2,0}},
  {"segment: crosses, no end inside", 4, SQ, 2, {-3,0, 3,0},
    {0,-4,0, 0,4,0}, {0,0,0, 0,0,0}},
  {"segment: on the line of an edge", 4, SQ, 2, {-1,0, 1,0},
    {0,0,0, 0,0,0}, {-4,-1,0, 4,-1,0}},
  {"segment: rotates about its middle", 4, SQ, 2, {-2,0, 2,0},
    {0,0,0, 0,0,0}, {0,3.5,0, 0,3.5,M_PI}},
  {"segment: end through a vertex", 4, SQ, 2, {0,0, 2,2},
    {0,0,0, 0,0,0}, {4,0,0, -4,0,0}},
  /* Two polygons */
  {"polygons: parallel faces slide", 4, SQ, 4, SQ,
    {-6,0,0, 6,0,0}, {0,3,0, 0,3,0}},
  {"polygons: parallel faces, same rotation", 4, SQ, 4, SQ,
    {-6,0,0.3, 6,0,1.3}, {0,4,0.3, 0,4,1.3}},
  {"polygons: parallel faces approach", 4, SQ, 4, SQ,
    {0,-6,0, 0,-3,0}, {0,3,0, 0,3,0}},
  {"polygons: parallel faces touch", 4, SQ, 4, SQ,
    {0,-4,0, 0,0,0}, {0,2,0, 0,2,0}},
  {"polygons: vertex meets vertex", 4, SQ, 4, SQ,
    {-4,-4,0, 0,0,0}, {2,2,0, 2,2,0}},
  {"polygons: vertex meets vertex, rotating", 3, TRI, 3, TRI,
    {-5,0,0, 5,0,M_PI/2}, {0,3,M_PI, 0,3,M_PI}},
  {"polygons: vertex touches a face", 4, SQ, 4, SQ,
    {0,-2-R2,M_PI/4, 0,-2-R2,M_PI/4}, {0,0,0, 0,0,0}},
  {"polygons: overlap for the whole segment", 4, SQ, 3, TRI,
    {0,0,0, 1,0,0.5}, {0.5,0.2,0, 0,0.1,-0.5}},
  {"polygons: start overlapping", 4, SQ, 4, SQ,
    {0,0,0, 8,0,0.4}, {1,1,0, 1,1,0}},
  {"polygons: end overlapping", 4, SQ, 4, SQ,
    {8,0,0.4, 0,0,0}, {1,1,0, 1,1,0}},
  {"polygons: cross, no vertex inside", 4, {-3,-0.5, 3,-0.5, 3,0.5, -3,0.5},
    4, {-0.5,-3, 0.5,-3, 0.5,3, -0.5,3}, {0,-8,0, 0,8,0}, {0,0,0, 0,0,0}},
  {"polygons: same body passes through", 4, SQ, 4, SQ,
    {-6,0,0, 6,0,0}, {0,0,0, 0,0,0}},
  {"polygons: no motion", 3, TRI, 4, SQ, {0,0,0, 0,0,0}, {5,1,0.2, 5,1,0.2}},
  {"polygons: both turn half a turn", 4, SQ, 3, TRI,
    {-4,0,0, -4,0,M_PI}, {4,0,0, 4,0,M_PI}},
};

/** @brief Rings to clean, and their expected number of vertices */
static struct
{
  const char *name;
  int n;
  double v[24];
  int want;
} rings[] = {
  {"square", 4, {-1,-1, 1,-1, 1,1, -1,1}, 4},
  {"clockwise square", 4, {-1,-1, -1,1, 1,1, 1,-1}, 4},
  {"repeated vertex", 5, {-1,-1, 1,-1, 1,-1, 1,1, -1,1}, 4},
  {"two repeated vertices", 6, {-1,-1, -1,-1, 1,-1, 1,1, -1,1, -1,1}, 4},
  {"collinear vertex", 5, {-1,-1, 0,-1, 1,-1, 1,1, -1,1}, 4},
  {"three collinear vertices", 7,
    {-1,-1, -0.5,-1, 0,-1, 0.5,-1, 1,-1, 1,1, -1,1}, 4},
  {"collinear vertex at the start", 5, {0,-1, 1,-1, 1,1, -1,1, -1,-1}, 4},
  {"not convex (refused)", 5, {0,0, 2,0, 2,2, 1,0.5, 0,2}, 0},
  {"spike (refused)", 5, {0,0, 3,0, 2,0, 2,2, 0,2}, 0},
  {"a line (refused)", 4, {0,0, 1,0, 2,0, 3,0}, 0},
  {"one point (refused)", 4, {1,1, 1,1, 1,1, 1,1}, 0},
};

int
main(int argc, char **argv)
{
  meos_initialize();
  int ncase = (int) (sizeof cases / sizeof cases[0]), bad = 0;
  int only = (argc > 1) ? atoi(argv[1]) : -1;
  DistEvents events;
  distevents_init(&events);
  printf("%-42s %-6s %4s %9s %9s %s\n", "case", "result", "ev", "max err",
    "nad gap", "zero and level checks");
  for (int c = 0; c < ncase; c++)
  {
    if (only >= 0 && c != only)
      continue;
    const Case *k = &cases[c];
    DistMotion ma, mb;
    distmotion_set(&ma, k->a[0], k->a[1], k->a[2], k->a[3], k->a[4], k->a[5]);
    distmotion_set(&mb, k->b[0], k->b[1], k->b[2], k->b[3], k->b[4], k->b[5]);
    DistSegm seg;
    distsegm_set(&seg, &ma, &mb);
    DistRefPoly ra, rb;
    if (! tu_refpoly(&ra, k->va, k->n) || ! tu_refpoly(&rb, k->vb, k->m))
    {
      printf("%-42s ring refused\n", k->name);
      bad++;
      continue;
    }
    DistPair cf;
    cf.kind = DISTPAIR_NONE;
    cf.i = cf.j = 0;
    events.count = 0;
    /* The level 1.5 also checks the crossings of a distance level */
    int rc = distwalk_segm(&seg, &ra, &rb, distwalk_ftol(&seg, &ra, &rb), 1.5,
      &cf, &events);
    if (rc != DISTWALK_OK)
    {
      printf("%-42s %s\n", k->name, rc == DISTWALK_CYCLE ? "CYCLE" : "SOLVE");
      bad++;
    }
    else
    {
      double ca[32], cb[32];
      int n = tu_vertices(&ra, ca), m = tu_vertices(&rb, cb);
      TuCheck res;
      tu_check(&ma, ca, n, &mb, cb, m, &events, 100000, 1.5, &res);
      bool ok = res.maxerr <= 1e-9 && ! res.order && ! res.below &&
        res.nadgap <= 1e-9 && ! res.nonzero_overlap && ! res.zero_apart &&
        ! res.level_side;
      printf("%-42s %-6s %4d %9.2e %+9.2e %d %d %d\n", k->name,
        ok ? "OK" : "WRONG", events.count, res.maxerr, res.nadgap,
        res.nonzero_overlap, res.zero_apart, res.level_side);
      if (! ok)
        bad++;
    }
    distrefpoly_free(&ra);
    distrefpoly_free(&rb);
  }
  distevents_free(&events);

  if (only < 0)
  {
    printf("\ncleaning of the reference rings\n");
    int nr = (int) (sizeof rings / sizeof rings[0]);
    for (int r = 0; r < nr; r++)
    {
      DistRefPoly rp;
      bool ok = tu_refpoly(&rp, rings[r].v, rings[r].n);
      int got = ok ? rp.nvert : 0;
      /* A counterclockwise ring has a positive area */
      double a2 = 0.0;
      for (int i = 0; ok && i < rp.nvert; i++)
      {
        int j = (i + 1) % rp.nvert;
        a2 += rp.verts[i].x * rp.verts[j].y - rp.verts[j].x * rp.verts[i].y;
      }
      bool good = (got == rings[r].want) && (! ok || a2 > 0.0);
      printf("  %-32s %d vertices  %s\n", rings[r].name, got,
        good ? "ok" : "FAIL");
      if (! good)
        bad++;
      if (ok)
        distrefpoly_free(&rp);
    }

    /* A regular polygon of radius 1 turns near a static point.  A vertex
     * points to the point during the turn, thus the nearest approach
     * distance is the distance of the point to the center minus 1. */
    int nv = 5000;
    double *v = malloc(sizeof(double) * 2 * nv);
    if (! v)
    {
      printf("out of memory\n");
      return 1;
    }
    for (int i = 0; i < nv; i++)
    {
      v[2 * i] = cos(2.0 * M_PI * i / nv);
      v[2 * i + 1] = sin(2.0 * M_PI * i / nv);
    }
    DistRefPoly ra, rb;
    bool ok = tu_refpoly(&ra, v, nv);
    free(v);
    distrefpoly_set_point(&rb);
    DistMotion ma, mb;
    distmotion_set(&ma, 0, 0, 0, 0, 0, 3.1);
    distmotion_set(&mb, 1.5, 0.2, 0, 1.5, 0.2, 0);
    DistSegm seg;
    distsegm_set(&seg, &ma, &mb);
    DistPair cf;
    cf.kind = DISTPAIR_NONE;
    cf.i = cf.j = 0;
    distevents_init(&events);
    int rc = ok ? distwalk_segm(&seg, &ra, &rb, distwalk_ftol(&seg, &ra, &rb),
      -1.0, &cf, &events) : DISTWALK_SOLVE;
    double nad = INFINITY;
    for (int i = 0; rc == DISTWALK_OK && i < events.count; i++)
      nad = fmin(nad, events.ev[i].dist);
    double want = hypot(1.5, 0.2) - 1.0;
    bool good = (rc == DISTWALK_OK) && fabs(nad - want) <= 1e-9;
    printf("\n%-42s %s, %d events, nad %.12f, expected %.12f\n",
      "polygon with 5000 vertices turns", good ? "ok" : "FAIL", events.count,
      nad, want);
    if (! good)
      bad++;
    distevents_free(&events);
    if (ok)
      distrefpoly_free(&ra);
    distrefpoly_free(&rb);
  }
  printf("\n%d problems\n", bad);
  meos_finalize();
  return bad ? 1 : 0;
}
