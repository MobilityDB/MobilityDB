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
 * @brief Helpers for the tests of meos/src/rgeo/trgeo_distwalk.c
 * @details The helpers compute the true distance between the placed bodies by
 * brute force, and compare the events of the walk with it.  The brute force
 * does not use the walk or its functions, so that it is an independent check.
 */

#ifndef __TRGEO_DISTWALK_TESTUTIL_H__
#define __TRGEO_DISTWALK_TESTUTIL_H__

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "rgeo/trgeo_distwalk.h"

/** Largest number of vertices of a body in the tests */
#define TU_MAXVERT  64

/**
 * @brief Return in (@p x, @p y) the vertices of a body at time @p t
 */
static inline void
tu_place(const DistMotion *m, const double *v, int n, double t, double *x,
  double *y)
{
  for (int i = 0; i < n; i++)
    distmotion_place(m, v[2 * i], v[2 * i + 1], t, &x[i], &y[i]);
}

/**
 * @brief Return the distance from (@p px, @p py) to the segment from
 * (@p ax, @p ay) to (@p bx, @p by)
 */
static inline double
tu_segdist(double px, double py, double ax, double ay, double bx, double by)
{
  double ux = bx - ax, uy = by - ay, l2 = ux * ux + uy * uy;
  double s = (l2 > 0.0) ? ((px - ax) * ux + (py - ay) * uy) / l2 : 0.0;
  s = (s < 0.0) ? 0.0 : ((s > 1.0) ? 1.0 : s);
  return hypot(px - (ax + s * ux), py - (ay + s * uy));
}

/**
 * @brief Return true if an edge of the counterclockwise polygon (ax, ay) has
 * all the points (bx, by) strictly outside its line
 */
static inline bool
tu_separates(const double *ax, const double *ay, int n, const double *bx,
  const double *by, int m)
{
  for (int i = 0; i < n; i++)
  {
    int j = (i + 1) % n;
    bool all = true;
    for (int k = 0; k < m && all; k++)
      if ((ax[j] - ax[i]) * (by[k] - ay[i]) -
          (ay[j] - ay[i]) * (bx[k] - ax[i]) >= 0.0)
        all = false;
    if (all)
      return true;
  }
  return false;
}

/**
 * @brief Return the true distance at @p t between the convex polygon @p va
 * with @p n vertices and the convex polygon, segment or point @p vb with
 * @p m vertices
 * @details The distance is zero if no edge separates the bodies.  Else it is
 * the smallest distance from a vertex of a body to an edge of the other.  A
 * segment is a polygon with two edges in opposite directions, which works
 * with the same tests.
 */
static inline double
tu_truedist(const DistMotion *ma, const double *va, int n,
  const DistMotion *mb, const double *vb, int m, double t)
{
  double ax[TU_MAXVERT], ay[TU_MAXVERT], bx[TU_MAXVERT], by[TU_MAXVERT];
  tu_place(ma, va, n, t, ax, ay);
  tu_place(mb, vb, m, t, bx, by);
  if (! tu_separates(ax, ay, n, bx, by, m) &&
      (m < 2 || ! tu_separates(bx, by, m, ax, ay, n)))
    return 0.0;
  double best = INFINITY;
  for (int i = 0; i < n; i++)
    for (int k = 0; k < m; k++)
    {
      double d = tu_segdist(bx[k], by[k], ax[i], ay[i], ax[(i + 1) % n],
        ay[(i + 1) % n]);
      if (d < best)
        best = d;
      if (m >= 2)
      {
        d = tu_segdist(ax[i], ay[i], bx[k], by[k], bx[(k + 1) % m],
          by[(k + 1) % m]);
        if (d < best)
          best = d;
      }
    }
  return best;
}

/**
 * @brief Set @p rp from the ring @p v with @p n vertices, to a segment if
 * @p n is 2, or to a point if @p n is 1
 * @details A segment is placed at its middle; the test gives segments that
 * have their middle at the origin, so that the vertices do not change.
 * @return False if the walk refuses the ring
 */
static inline bool
tu_refpoly(DistRefPoly *rp, const double *v, int n)
{
  if (n == 1)
  {
    distrefpoly_set_point(rp);
    return true;
  }
  if (n == 2)
  {
    double cx, cy;
    distrefpoly_set_segment(rp, v[0], v[1], v[2], v[3], &cx, &cy);
    return true;
  }
  POINTARRAY *pa = ptarray_construct_empty(0, 0, (uint32_t) (n + 1));
  for (int i = 0; i <= n; i++)
  {
    POINT4D p = {v[2 * (i % n)], v[2 * (i % n) + 1], 0, 0};
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  POINTARRAY **ringarr = lwalloc(sizeof(POINTARRAY *));
  ringarr[0] = pa;
  LWPOLY *poly = lwpoly_construct(0, NULL, 1, ringarr);
  bool ok = distrefpoly_set(rp, poly);
  lwpoly_free(poly);
  return ok;
}

/**
 * @brief Copy the vertices of @p rp to @p v and return their number
 * @details The tests compute the true distance with these vertices, which
 * are the ones that the walk uses.
 */
static inline int
tu_vertices(const DistRefPoly *rp, double *v)
{
  for (int i = 0; i < rp->nvert; i++)
  {
    v[2 * i] = rp->verts[i].x;
    v[2 * i + 1] = rp->verts[i].y;
  }
  return rp->nvert;
}

/**
 * @brief Result of the comparison of the events of a walk with the true
 * distance
 */
typedef struct
{
  double maxerr;         /**< Largest relative error of an event */
  int order;             /**< Events that are not in increasing time order */
  int below;             /**< Events below the true distance */
  double nadgap;         /**< Minimum of the events minus the true minimum,
                              relative */
  bool overlap;          /**< The bodies overlap at some time of the scan */
  int nonzero_overlap;   /**< Scan times with overlap but a result that is
                              not zero */
  int zero_apart;        /**< Result intervals of zero with bodies apart */
  int level_side;        /**< Scan times at which the result and the true
                              distance are on different sides of the level */
} TuCheck;

/**
 * @brief Compare the events @p ev of a walk with the true distance, at the
 * events and at @p scan times of a regular scan
 * @details The checks are:
 * - each event has the true distance at its time;
 * - the events are in increasing time order;
 * - no event is below the true distance, which would show a walk that
 *   measures to the line of an edge where the origin does not project on
 *   the edge;
 * - the minimum of the events is the true minimum, which makes the nearest
 *   approach distance correct;
 * - where the bodies overlap, the linear interpolation of the events is zero;
 * - where two consecutive events are zero, the bodies overlap or touch;
 * - if @p level is not negative, the linear interpolation of the events is
 *   on the same side of @p level as the true distance, which is what the
 *   threshold functions such as tDwithin need.
 */
static inline void
tu_check(const DistMotion *ma, const double *va, int n, const DistMotion *mb,
  const double *vb, int m, const DistEvents *ev, int scan, double level,
  TuCheck *res)
{
  memset(res, 0, sizeof(TuCheck));
  const DistEvent *e = ev->ev;
  int ne = ev->count;
  double emin = INFINITY, bmin = INFINITY;
  for (int i = 0; i < ne; i++)
  {
    double td = tu_truedist(ma, va, n, mb, vb, m, e[i].t);
    double err = fabs(e[i].dist - td) / (1.0 + td);
    if (err > res->maxerr)
      res->maxerr = err;
    if (e[i].dist < td - 1e-9 * (1.0 + td))
      res->below++;
    if (i > 0 && e[i].t <= e[i - 1].t)
      res->order++;
    if (e[i].dist < emin)
      emin = e[i].dist;
  }
  int k = 0;
  for (int j = 0; j <= scan; j++)
  {
    double t = (double) j / scan;
    double td = tu_truedist(ma, va, n, mb, vb, m, t);
    if (td < bmin)
      bmin = td;
    while (k + 2 < ne && e[k + 1].t <= t)
      k++;
    double span = e[k + 1].t - e[k].t;
    double r = (span > 0.0) ? (t - e[k].t) / span : 0.0;
    double interp = e[k].dist + r * (e[k + 1].dist - e[k].dist);
    /* Near the level, the true distance cannot decide the side */
    if (level >= 0.0 && fabs(td - level) > 1e-9 * (1.0 + level) &&
        (interp <= level) != (td <= level))
      res->level_side++;
    if (td > 0.0)
      continue;
    res->overlap = true;
    if (interp > 1e-7)
      res->nonzero_overlap++;
  }
  res->nadgap = (emin - bmin) / (1.0 + bmin);
  for (int i = 0; i + 1 < ne; i++)
  {
    if (e[i].dist != 0.0 || e[i + 1].dist != 0.0)
      continue;
    for (int j = 1; j < 16; j++)
    {
      double t = e[i].t + (e[i + 1].t - e[i].t) * j / 16.0;
      if (tu_truedist(ma, va, n, mb, vb, m, t) > 1e-7)
      {
        res->zero_apart++;
        break;
      }
    }
  }
}

#endif /* __TRGEO_DISTWALK_TESTUTIL_H__ */
