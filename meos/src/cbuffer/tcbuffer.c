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
 * @brief Basic functions for temporal circular buffers
 */

/* C */
#include <assert.h>
#include <limits.h>
#include <math.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_util.h"
#include "geo/geo_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tgeo_tempspatialrels.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_boxops.h"
#include "cbuffer/tcbuffer_tempspatialrels.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * valid for operations
 * @param[in] temp Temporal value
 * @param[in] cb Value
 */
bool
ensure_valid_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(cb, false);
  if (! ensure_same_srid(tspatial_srid(temp), cbuffer_srid(cb)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @note A circular buffer is 2D and planar by construction (#cbuffer_make
 * rejects geodetic, Z and M input), so the geometry must likewise be 2D and
 * planar. Centralizing these constraints here keeps every temporal
 * circular-buffer/geometry operation consistent and makes switching the
 * supported dimensionality a single-place change.
 */
bool
ensure_valid_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_not_geodetic_geo(gs) || ! ensure_has_not_Z_geo(gs))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal circular buffer and a
 * spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 */
bool
ensure_valid_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), box->srid))
    return false;
  return true;
}

/**
 * @brief Return true if a temporal circular buffer and a circular buffer are
 * valid for operations
 * @param[in] temp1,temp2 Temporal value
 */
bool
ensure_valid_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TCBUFFER(temp1, false); VALIDATE_TCBUFFER(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return false;
  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Return the fractions of a segment at which the distance between the
 * centres of two circular buffer segments equals a length that varies linearly
 * over the segment
 * @details Over the fraction f in [0, 1] of the segment the centres stand apart
 * by the vector (dx0 + dx f, dy0 + dy f) and the length reads
 * L(f) = (1 - f) l0 + f l1. The fractions solve |c(f)| = L(f), which squares to
 * the quadratic a f^2 + b f + c = 0 below. Squaring also admits the fractions
 * at which |c(f)| = -L(f), which are the roots at which L is negative, so a
 * root is kept by its position in [0, 1] and by the sign of L there. It is
 * never kept by evaluating the distance at it again: at projected coordinates
 * the rounding of that evaluation exceeds any fixed band, and reads a crossing
 * the quadratic states exactly as a miss. The roots are computed as
 * #tpointsegm_tdwithin_turnpt computes them, one by the quadratic formula and
 * the other by Viète's, so that neither loses its digits to a cancellation.
 * @param[in] sv1,ev1 Circular buffers defining the first segment
 * @param[in] sv2,ev2 Circular buffers defining the second segment
 * @param[in] l0,l1 Length at the start and at the end of the segment
 * @param[out] roots Fractions found, in increasing order
 * @return Number of fractions found, from 0 to 2
 */
static int
tcbuffersegm_length_roots(const Cbuffer *sv1, const Cbuffer *ev1,
  const Cbuffer *sv2, const Cbuffer *ev2, double l0, double l1,
  long double roots[2])
{
  const POINT2D *spt1 = cbuffer_point2d_p(sv1);
  const POINT2D *ept1 = cbuffer_point2d_p(ev1);
  const POINT2D *spt2 = cbuffer_point2d_p(sv2);
  const POINT2D *ept2 = cbuffer_point2d_p(ev2);
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  double dx = (ept1->x - spt1->x) - (ept2->x - spt2->x);
  double dy = (ept1->y - spt1->y) - (ept2->y - spt2->y);
  double dl = l1 - l0;
  /* The products are summed and the roots taken in extended precision, as
   * #tpointsegm_tdwithin_turnpt takes them */
  long double a = (long double) (dx * dx) + dy * dy - dl * dl;
  long double b = (long double) (2 * dx0 * dx) + 2 * dy0 * dy - 2 * l0 * dl;
  long double c = (long double) (dx0 * dx0) + dy0 * dy0 - l0 * l0;

  long double cand[2];
  int ncand = 0;
  if (a == 0.0)
  {
    /* A distance changing at the rate of the length leaves a linear equation,
     * and a gap that does not change at all leaves no isolated root */
    if (b != 0.0)
      cand[ncand++] = -c / b;
  }
  else
  {
    long double delta = b * b - 4 * a * c;
    if (delta < 0.0)
      return 0;
    if (delta == 0.0)
      cand[ncand++] = -b / (2 * a);
    else
    {
      /* The root the quadratic formula would compute as a difference of close
       * values is taken from the product of the roots, c / a */
      long double sq = sqrtl(delta);
      long double q = (b >= 0.0) ? -0.5 * (b + sq) : -0.5 * (b - sq);
      cand[ncand++] = q / a;
      cand[ncand++] = c / q;
    }
  }

  int nroots = 0;
  for (int i = 0; i < ncand; i++)
  {
    long double f = cand[i];
    if (f < 0.0 || f > 1.0 || (1.0 - f) * l0 + f * l1 < 0.0)
      continue;
    roots[nroots++] = f;
  }
  if (nroots == 2)
  {
    if (roots[0] > roots[1])
    {
      long double tmp = roots[0]; roots[0] = roots[1]; roots[1] = tmp;
    }
    if (roots[0] == roots[1])
      nroots = 1;
  }
  return nroots;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments are within a
 * distance during the output period, return 0 otherwise
 * @details The period is defined by the output timestamps. These are the
 * turning points when computing the temporal distance.
 * @param[in] start1,end1 Circular buffers defining the first segment
 * @param[in] start2,end2 Circular buffers the second segment
 * @param[in] dist Distance
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not constant.
 */
int
tcbuffersegm_dwithin_turnpt(Datum start1, Datum end1, Datum start2, Datum end2,
  Datum dist, TimestampTz lower, TimestampTz upper, TimestampTz *t1,
  TimestampTz *t2)
{
  assert(t1); assert(t2); assert(lower < upper);
  const Cbuffer *sv1 = DatumGetCbufferP(start1);
  const Cbuffer *ev1 = DatumGetCbufferP(end1);
  const Cbuffer *sv2 = DatumGetCbufferP(start2);
  const Cbuffer *ev2 = DatumGetCbufferP(end2);
  double d = DatumGetFloat8(dist);

  /* The buffers are at the distance when their centres stand the sum of the
   * radii and the distance apart */
  long double roots[2];
  int nroots = tcbuffersegm_length_roots(sv1, ev1, sv2, ev2,
    sv1->radius + sv2->radius + d, ev1->radius + ev2->radius + d, roots);

  /* Keep the strictly internal instants: those at the bounds are no crossings
   * the caller splices, being handled by the surrounding sequence construction,
   * and returning them duplicates a timestamp */
  double duration = (double) (upper - lower);
  TimestampTz ts[2];
  int nts = 0;
  for (int i = 0; i < nroots; i++)
  {
    TimestampTz t = lower + (TimestampTz) (roots[i] * duration);
    if (t > lower && t < upper && (nts == 0 || t != ts[nts - 1]))
      ts[nts++] = t;
  }
  if (nts == 0)
  {
    *t1 = *t2 = (TimestampTz) 0;
    return 0;
  }
  *t1 = ts[0];
  *t2 = ts[nts - 1];
  return nts;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments are within a
 * distance during a sub-period of the segment, return 0 otherwise
 * @details Unlike #tcbuffersegm_dwithin_turnpt, which returns the instants at
 * which the distance equals @p dist (used to split the temporal distance), this
 * returns the sub-interval `[t1, t2]` of `[lower, upper]` during which the two
 * segments are within @p dist, so the temporal `tDwithin` boolean is true on a
 * continuous interval rather than only at the crossing instant. It is derived
 * from the crossings together with the within status at the two segment
 * endpoints, which is read by the predicate the instants themselves are read
 * with.
 * @param[in] start1,end1 Circular buffers defining the first segment
 * @param[in] start2,end2 Circular buffers defining the second segment
 * @param[in] dist Distance
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting within-distance period
 * @pre The segments are not constant.
 */
int
tcbuffersegm_tdwithin_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum dist, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(t1); assert(t2); assert(lower < upper);
  const Cbuffer *sv1 = DatumGetCbufferP(start1);
  const Cbuffer *ev1 = DatumGetCbufferP(end1);
  const Cbuffer *sv2 = DatumGetCbufferP(start2);
  const Cbuffer *ev2 = DatumGetCbufferP(end2);
  double d = DatumGetFloat8(dist);
  long double roots[2];
  int nroots = tcbuffersegm_length_roots(sv1, ev1, sv2, ev2,
    sv1->radius + sv2->radius + d, ev1->radius + ev2->radius + d, roots);
  /* Within status at the two segment endpoints */
  bool win_lower = cbuffer_dwithin(sv1, sv2, d);
  bool win_upper = cbuffer_dwithin(ev1, ev2, d);
  long double fstart, fend;
  if (nroots == 0)
  {
    /* No crossing: within throughout the segment, or never */
    if (! win_lower)
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
    fstart = 0.0; fend = 1.0;
  }
  else if (nroots == 1)
  {
    /* One crossing: within on the side whose endpoint is within */
    if (win_lower && ! win_upper)
    {
      fstart = 0.0; fend = roots[0];
    }
    else if (! win_lower && win_upper)
    {
      fstart = roots[0]; fend = 1.0;
    }
    else if (win_lower)
    {
      /* The gap between the discs, the distance of the centres less an affine
       * length, is convex over the segment, so a segment within at both ends
       * is within throughout: the crossing is a tangency at one of its ends */
      fstart = 0.0; fend = 1.0;
    }
    else
    {
      /* Tangent touch from outside: within only at the crossing instant */
      fstart = fend = roots[0];
    }
  }
  else
  {
    /* Two crossings: within between them */
    fstart = roots[0]; fend = roots[1];
  }
  double duration = (double) (upper - lower);
  *t1 = lower + (TimestampTz) (fstart * duration);
  *t2 = lower + (TimestampTz) (fend * duration);
  return (*t1 == *t2) ? 1 : 2;
}

/**
 * @brief Return 1 or 2 if a temporal circular buffer segment contains or
 * covers another one during a sub-period of the segment, 0 otherwise
 * @details The first segment contains the second one when @p strict is true
 * and covers it otherwise. A moving disk (P1, R1) contains a moving disk
 * (P2, R2) when dist(P1, P2) + R2 <= R1, that is when the clearance
 * g(t) = dist(P1(t), P2(t)) - (R1(t) - R2(t)) is non-positive; g is the
 * distance of the centres minus the radius DIFFERENCE, so this mirrors
 * #tcbuffersegm_tdwithin_turnpt with the difference of the radii in the place
 * of their sum and a zero distance. The difference may be negative, and the
 * roots at which it is are those the squaring introduces, which
 * #tcbuffersegm_length_roots discards by that sign. It returns the
 * sub-interval [t1, t2] of [lower, upper] during which the relation holds, so
 * the temporal contains and covers Boolean is true on a continuous interval
 * rather than only at the clearance minimum. As #cbuffer_contains states it,
 * contains is covers except for a point on the boundary of a disk of a
 * strictly positive radius, so with @p strict true the isolated tangency
 * (g = 0 at a single instant) of such a point does not count.
 * @param[in] start1,end1 Circular buffers defining the first segment
 * @param[in] start2,end2 Circular buffers defining the second segment
 * @param[in] strict Passed as a float, non-zero for contains, zero for covers
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period
 * @pre The segments are not constant.
 */
int
tcbuffersegm_contains_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum strict, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(t1); assert(t2); assert(lower < upper);
  const Cbuffer *sv1 = DatumGetCbufferP(start1);
  const Cbuffer *ev1 = DatumGetCbufferP(end1);
  const Cbuffer *sv2 = DatumGetCbufferP(start2);
  const Cbuffer *ev2 = DatumGetCbufferP(end2);
  bool contains = (DatumGetFloat8(strict) != 0);
  /* Contains differs from covers only for a disk of a zero radius, a point,
   * on the boundary of a disk of a strictly positive radius, as
   * #cbuffer_contains states it. The radius of the second disk is affine and
   * non-negative, so it is zero inside the segment only when it is zero at
   * both ends; otherwise contains is covers there */
  bool is_strict = contains && sv2->radius == 0.0 && ev2->radius == 0.0;
  /* Radius DIFFERENCE R1 - R2 (contains threshold), not the sum */
  long double roots[2];
  int nroots = tcbuffersegm_length_roots(sv1, ev1, sv2, ev2,
    sv1->radius - sv2->radius, ev1->radius - ev2->radius, roots);
  /* Status at the two segment endpoints, read by the base predicates the
   * instants of the result are read with */
  bool in_lower = contains ? cbuffer_contains(sv1, sv2) :
    cbuffer_covers(sv1, sv2);
  bool in_upper = contains ? cbuffer_contains(ev1, ev2) :
    cbuffer_covers(ev1, ev2);
  long double fstart, fend;
  if (nroots == 0)
  {
    if (! in_lower)
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
    fstart = 0.0; fend = 1.0;
  }
  else if (nroots == 1)
  {
    if (in_lower && ! in_upper)
    {
      fstart = 0.0; fend = roots[0];
    }
    else if (! in_lower && in_upper)
    {
      fstart = roots[0]; fend = 1.0;
    }
    else if (in_lower)
    {
      /* The clearance is convex over the segment, so a segment in at both
       * ends is in throughout: the crossing is a tangency at one of its ends,
       * as #tcbuffersegm_tdwithin_turnpt reads it */
      fstart = 0.0; fend = 1.0;
    }
    else
    {
      /* Isolated tangency: the disks graze from inside at a single instant.
       * Covers holds there; contains does not when a point touches the
       * boundary of a disk of a strictly positive radius */
      long double r1 = sv1->radius + roots[0] * (ev1->radius - sv1->radius);
      if (is_strict && r1 > 0)
      {
        *t1 = *t2 = (TimestampTz) 0;
        return 0;
      }
      fstart = fend = roots[0];
    }
  }
  else
  {
    fstart = roots[0]; fend = roots[1];
  }
  double duration = (double) (upper - lower);
  *t1 = lower + (TimestampTz) (fstart * duration);
  *t2 = lower + (TimestampTz) (fend * duration);
  return (*t1 == *t2) ? 1 : 2;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments are at a
 * minimum distance during the period defined by the output timestamps
 * @details The function returns 0 otherwise. These are the turning points
 * when computing the temporal distance.
 * @param[in] start1,end1 Circular buffers defining the first segment
 * @param[in] start2,end2 Circular buffers the second segment
 * @param[in] dist Distance, unused parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not constant.
 */
int
tcbuffersegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
 Datum end2, Datum dist UNUSED, TimestampTz lower, TimestampTz upper,
 TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  const Cbuffer *sv1 = DatumGetCbufferP(start1);
  const Cbuffer *ev1 = DatumGetCbufferP(end1);
  const Cbuffer *sv2 = DatumGetCbufferP(start2);
  const Cbuffer *ev2 = DatumGetCbufferP(end2);
  const POINT2D *spt1 = cbuffer_point2d_p(sv1);
  const POINT2D *ept1 = cbuffer_point2d_p(ev1);
  const POINT2D *spt2 = cbuffer_point2d_p(sv2);
  const POINT2D *ept2 = cbuffer_point2d_p(ev2);

  /* The gap read in the fraction f of the period: the centres separate by an
   * affine vector and the radii sum to an affine scalar */
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  double r0 = sv1->radius + sv2->radius;
  double dx = (ept1->x - spt1->x) - (ept2->x - spt2->x);
  double dy = (ept1->y - spt1->y) - (ept2->y - spt2->y);
  double dr = (ev1->radius - sv1->radius) + (ev2->radius - sv2->radius);

  /* Centres that keep their separation leave an affine gap, whose extrema are
   * the endpoints the caller already reads */
  double s = dx * dx + dy * dy;
  if (s == 0.0)
    return 0;
  double q = dx * dx0 + dy * dy0;
  double e = dx0 * dx0 + dy0 * dy0;

  /* g(f) = sqrt(e + 2qf + sf^2) - (r0 + dr f), and g'(f) = 0 reads
   * q + s f = dr sqrt(e + 2qf + sf^2), which squares to the quadratic below */
  double cand[2];
  int ncand = 0;
  if (dr == 0.0)
    /* A combined radius that does not change leaves the classic closest
     * approach of the two centres, which is what the temporal point family
     * reads and what the quadratic below degenerates to. Taking it directly
     * keeps the two families bit-compatible and keeps the fourth power s^2 out
     * of a degeneracy test that a segment of small relative motion trips */
    cand[ncand++] = -q / s;
  else
  {
    double a = s * (s - dr * dr);
    double b = 2 * q * (s - dr * dr);
    double c = q * q - dr * dr * e;
    /* The degeneracy test is relative to the coefficients it sits among, the
     * absolute one being meaningless against squared coordinates */
    double scale = fabs(b) + fabs(c);
    if (fabs(a) <= scale * DBL_EPSILON)
    {
      if (fabs(b) > 0.0)
        cand[ncand++] = -c / b;
    }
    else
    {
      double delta = b * b - 4 * a * c;
      if (delta < 0.0)
        return 0;
      double sq = sqrt(delta);
      cand[ncand++] = (-b - sq) / (2 * a);
      cand[ncand++] = (-b + sq) / (2 * a);
    }
  }

  /* The gap is the norm of an affine function minus an affine one, hence
   * convex, so it holds at most one interior extremum and that extremum is a
   * minimum lying strictly below both endpoints. Requiring that discards the
   * companion root the squaring introduces, on which g' has the wrong sign */
  double gs = sqrt(e) - r0;
  double ge = hypot(dx0 + dx, dy0 + dy) - (r0 + dr);
  double best = (gs < ge) ? gs : ge;
  double fbest = -1.0;
  for (int i = 0; i < ncand; i++)
  {
    double f = cand[i];
    if (f <= 0.0 || f >= 1.0)
      continue;
    double g = hypot(dx0 + dx * f, dy0 + dy * f) - (r0 + dr * f);
    if (g < best)
    {
      best = g;
      fbest = f;
    }
  }
  if (fbest < 0.0)
    return 0;

  /* A gap that dips below zero is clamped there by the distance itself, so the
   * value is flat between the two instants at which it reaches zero and the
   * minimum is no breakpoint of it. Those instants are where the two disks meet,
   * which is the crossing question, and the dwithin turning point at a zero
   * distance answers it */
  if (best < 0.0)
    return tcbuffersegm_dwithin_turnpt(start1, end1, start2, end2, (Datum) 0.0,
      lower, upper, t1, t2);

  *t1 = *t2 = lower + (TimestampTz) ((double) (upper - lower) * fbest);
  return 1;
}

/*****************************************************************************/

/**
 * @brief Return whether a temporal circular buffer segment and a circular
 * buffer intersect
 * @details Return 1 or 2 if they intersect during the period defined by the
 * output timestamps, return 0 otherwise.
 * @param[in] start,end Temporal instants defining the segment
 * @param[in] value Value to locate
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tcbuffersegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  int result = tcbuffersegm_dwithin_turnpt(start, end, value, value,
    (Datum) 0.0, lower, upper, t1, t2);
  return result;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments intersect
 * during the period defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tcbuffersegm_intersection(Datum start1, Datum end1, Datum start2, Datum end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  return tcbuffersegm_dwithin_turnpt(start1, end1, start2, end2, (Datum) 0.0,
    lower, upper, t1, t2);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_inout
 * @brief Return a temporal circular buffer from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Tcbuffer_in()
 */
Temporal *
tcbuffer_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TCBUFFER);
}

/**
 * @ingroup meos_cbuffer_inout
 * @brief Return a temporal circular buffer from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @errval NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tcbuffer_from_mfjson(const char *mfjson)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(mfjson, NULL);
  return temporal_from_mfjson(mfjson, T_TCBUFFER);
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer instant from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 */
TInstant *
tcbufferinst_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer sequence from its Well-Known Text
 * (WKT) representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tcbufferseq_in(const char *str, interpType interp UNUSED)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_cbuffer_inout
 * @brief Return a temporal circular buffer sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tcbufferseqset_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tcbuffer_in(str);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a
 * temporal float
 * @note This function is called after the synchronization done in function
 * #tcbuffer_make
 */
TInstant *
tgeompoint_tfloat_to_tcbufferinst(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst1->temptype == T_TGEOMPOINT);
  assert(inst2); assert(inst2->temptype == T_TFLOAT);
  assert(inst1->t == inst2->t);
  Cbuffer *cb = cbuffer_make(DatumGetGserializedP(tinstant_value_p(inst1)),
    DatumGetFloat8(tinstant_value_p(inst2)));
  return tinstant_make_free(PointerGetDatum(cb), T_TCBUFFER, inst1->t);
}

/**
 * @ingroup meos_internal_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a
 * temporal float
 * @note This function is called after the synchronization done in function
 * #tcbuffer_make
 */
TSequence *
tgeompoint_tfloat_to_tcbufferseq(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq1->temptype == T_TGEOMPOINT);
  assert(seq2); assert(seq2->temptype == T_TFLOAT);
  assert(seq1->count == seq2->count);
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  for (int i = 0; i < seq1->count; i++)
    instants[i] = tgeompoint_tfloat_to_tcbufferinst(TSEQUENCE_INST_N(seq1, i),
      TSEQUENCE_INST_N(seq2, i));
  return tsequence_make_free(instants, seq1->count, seq1->period.lower_inc,
    seq1->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE_NO);
}

/**
 * @ingroup meos_internal_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a
 * temporal float
 * @note This function is called after the synchronization done in function
 * #tcbuffer_make
 */
TSequenceSet *
tgeompoint_tfloat_to_tcbufferseqset(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss1->temptype == T_TGEOMPOINT);
  assert(ss2); assert(ss2->temptype == T_TFLOAT);
  assert(ss1->count == ss2->count);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
  for (int i = 0; i < ss1->count; i++)
    sequences[i] = tgeompoint_tfloat_to_tcbufferseq(TSEQUENCESET_SEQ_N(ss1, i),
      TSEQUENCESET_SEQ_N(ss2, i));
  return tsequenceset_make_free(sequences, ss1->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer from a temporal point and a
 * temporal float
 * @csqlfn #Tcbuffer_constructor()
 */
Temporal *
tcbuffer_make(const Temporal *tpoint, const Temporal *tfloat)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(tpoint, NULL); VALIDATE_TFLOAT(tfloat, NULL);

  Temporal *sync1, *sync2;
  /* Return NULL if the temporal values do not intersect in time
   * The operation performed is synchronization without adding crossings */
  if (! intersection_temporal_temporal(tpoint, tfloat, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  assert(temptype_subtype(sync1->subtype));
  Temporal *result;
  switch (sync1->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tgeompoint_tfloat_to_tcbufferinst((TInstant *) sync1,
        (TInstant *) sync2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tgeompoint_tfloat_to_tcbufferseq((TSequence *) sync1,
        (TSequence *) sync2);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tgeompoint_tfloat_to_tcbufferseqset((TSequenceSet *) sync1,
        (TSequenceSet *) sync2);
  }
  pfree(sync1); pfree(sync2);
  return result;
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer instant from a circular buffer and
 * a timestamptz
 * @param[in] cb Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tcbufferinst_make(const Cbuffer *cb, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL);
  return tinstant_make(PointerGetDatum(cb), T_TCBUFFER, t);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer from a circular buffer and the time
 * frame of another temporal value
 * @param[in] cb Value
 * @param[in] temp Temporal value
 */
Temporal *
tcbuffer_from_base_temp(const Cbuffer *cb, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(cb), T_TCBUFFER, temp);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer discrete sequence from a circular
 * buffer and a timestamptz set
 * @param[in] cb Value
 * @param[in] s Set
 */
TSequence *
tcbufferseq_from_base_tstzset(const Cbuffer *cb, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(cb), T_TCBUFFER, s);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer sequence from a circular buffer and
 * a timestamptz span
 * @param[in] cb Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tcbufferseq_from_base_tstzspan(const Cbuffer *cb, const Span *s,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_TSTZSPAN(s, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(cb), T_TCBUFFER, s,
    interp);
}

/**
 * @ingroup meos_cbuffer_constructor
 * @brief Return a temporal circular buffer sequence set from a circular buffer
 * and a timestamptz span set
 * @param[in] cb Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tcbufferseqset_from_base_tstzspanset(const Cbuffer *cb, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(cb, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(cb), T_TCBUFFER,
    ss, interp);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @brief Return a temporal geometry point constructed from the points of a
 * temporal circular buffer
 */
TInstant *
tcbufferinst_tgeompointinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  GSERIALIZED *point = cbuffer_point(DatumGetCbufferP(tinstant_value_p(inst)));
  TInstant *result = tinstant_make(PointerGetDatum(point), T_TGEOMPOINT,
    inst->t);
  pfree(point);
  return result;
}

/**
 * @brief Return a temporal geometry point constructed from the points of a
 * temporal circular buffer
 */
TSequence *
tcbufferseq_tgeompointseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TCBUFFER);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tcbufferinst_tgeompointinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal geometry point constructed from the points of a
 * temporal circular buffer
 */
TSequenceSet *
tcbufferseqset_tgeompointseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TCBUFFER);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_tgeompointseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Return a temporal geometry point constructed from the points of a
 * temporal circular buffer
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tgeompoint()
 */
Temporal *
tcbuffer_to_tgeompoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_tgeompointinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_tgeompointseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_tgeompointseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TInstant *
tcbufferinst_tfloatinst(const TInstant *inst)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  double radius = cbuffer_radius(DatumGetCbufferP(tinstant_value_p(inst)));
  return tinstant_make(Float8GetDatum(radius), T_TFLOAT, inst->t);
}

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TSequence *
tcbufferseq_tfloatseq(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TCBUFFER);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tcbufferinst_tfloatinst(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 */
TSequenceSet *
tcbufferseqset_tfloatseqset(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TCBUFFER);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_tfloatseq(TSEQUENCESET_SEQ_N(ss, i));
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Return a temporal float constructed from the radius of a temporal
 * circular buffer
 * @param[in] temp Temporal point
 * @csqlfn #Tcbuffer_to_tfloat()
 */
Temporal *
tcbuffer_to_tfloat(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_tfloatinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_tfloatseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_tfloatseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************/

/**
 * @brief Convert a temporal geometry into a temporal circular buffer
 */
TInstant *
tgeominst_tcbufferinst(const TInstant *inst)
{
  assert(inst); assert(tgeo_type_all(inst->temptype));
  GSERIALIZED *value = (GSERIALIZED *) DatumGetGserializedP(
    tinstant_value_p(inst));
  double radius = 0.0;
  uint32_t geotype = gserialized_get_type(value);
  if (geotype != POINTTYPE)
  {
    value = geom_minimum_bounding_radius(value, &radius);
  }
  Cbuffer *cb = cbuffer_make(value, radius);
  if (geotype != POINTTYPE)
    pfree(value);
  if (cb == NULL)
    return NULL;
  return tinstant_make_free(PointerGetDatum(cb), T_TCBUFFER, inst->t);
}

/**
 * @brief Convert a temporal geometry into a temporal circular buffer
 */
TSequence *
tgeomseq_tcbufferseq(const TSequence *seq)
{
  assert(seq); assert(tgeo_type_all(seq->temptype));
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tgeominst_tcbufferinst(TSEQUENCE_INST_N(seq, i));
    if (inst == NULL)
    {
      pfree_array((void **) instants, i);
      return NULL;
    }
    instants[i] = inst;
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Convert a temporal geometry into a temporal circular buffer
 */
TSequenceSet *
tgeomseqset_tcbufferseqset(const TSequenceSet *ss)
{
  assert(ss); assert(tgeo_type_all(ss->temptype));
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    TSequence *seq = tgeomseq_tcbufferseq(TSEQUENCESET_SEQ_N(ss, i));
    if (seq == NULL)
    {
      pfree_array((void **) sequences, i);
      return NULL;
    }
    sequences[i] = seq;
  }
  return tsequenceset_make_free(sequences, ss->count, true);
}

/**
 * @ingroup meos_cbuffer_conversion
 * @brief Convert a temporal geometry into a temporal circular buffer
 * @param[in] temp Temporal point
 * @csqlfn #Tgeometry_to_tcbuffer()
 */
Temporal *
tgeometry_to_tcbuffer(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tgeominst_tcbufferinst((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tgeomseq_tcbufferseq((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tgeomseqset_tcbufferseqset((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the start value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Temporal_start_value()
 */
Cbuffer *
tcbuffer_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  return DatumGetCbufferP(temporal_start_value(temp));
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the end value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Temporal_end_value()
 */
Cbuffer *
tcbuffer_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  return DatumGetCbufferP(temporal_end_value(temp));
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return a copy of the n-th value of a temporal circular buffer
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tcbuffer_value_n(const Temporal *temp, int n, Cbuffer **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetCbufferP(dresult);
  return true;
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of copies of base values of a temporal circular buffer
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Cbuffer **
tcbuffer_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(count, NULL);

  Datum *datumarr = temporal_values_p(temp, count);
  Cbuffer **result = palloc(sizeof(Cbuffer *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = cbuffer_copy(DatumGetCbufferP(datumarr[i]));
  pfree(datumarr);
  return result;
}

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return the distinct values of a temporal circular buffer, each with the span set on
 * which it is taken
 * @param[in] temp Temporal value
 * @param[out] values Array of the distinct values
 * @param[out] count Number of values in the output arrays
 * @return Array of span sets, the i-th one the time on which @p temp takes
 * the i-th value
 * @csqlfn #Temporal_unnest()
 */
SpanSet **
tcbuffer_unnest(const Temporal *temp, Cbuffer ***values, int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL);
  *count = 0;
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_nonlinear_interp(temp->flags))
    return NULL;

  Datum *datums;
  SpanSet **result = temporal_unnest(temp, &datums, count);
  /* The datums are copies, so the values take them over */
  Cbuffer **vals = palloc(sizeof(Cbuffer *) * *count);
  for (int i = 0; i < *count; i++)
    vals[i] = DatumGetCbufferP(datums[i]);
  *values = vals;
  pfree(datums);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
static Set *
tcbufferinst_members(const TInstant *inst, bool point)
{
  Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  if (point)
  {
    GSERIALIZED *gs = cbuffer_point(cb);
    Datum value = PointerGetDatum(gs);
    Set *result = set_make_exp(&value, 1, 1, T_GEOMETRY, ORDER_NO);
    pfree(gs);
    return result;
  }
  Datum value = Float8GetDatum(cb->radius);
  return set_make_exp(&value, 1, 1, T_FLOAT8, ORDER_NO);
}

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
static Set *
tcbufferseq_members(const TSequence *seq, bool point)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Cbuffer *cb = DatumGetCbufferP(
      tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    values[i] = point ?
      PointerGetDatum(cbuffer_point(cb)) : Float8GetDatum(cb->radius);
  }
  MeosType basetype = point ? T_GEOMETRY : T_FLOAT8;
  datumarr_sort(values, seq->count, basetype);
  int count = datumarr_remove_duplicates(values, seq->count, basetype);
  Set *result = set_make_exp(values, count, count, basetype, ORDER_NO);
  if (point)
  {
    for (int i = 0; i < seq->count; i++)
      pfree(DatumGetPointer(values[i]));
  }
  pfree(values);
  return result;
}

/**
 * @brief Return the points or radii of a temporal circular buffer
 */
static Set *
tcbufferseqset_members(const TSequenceSet *ss, bool point)
{
  Datum *values = palloc(sizeof(Datum) * ss->totalcount);
  int nvalues = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      const Cbuffer *cb = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, j)));
      values[nvalues++] = point ?
        PointerGetDatum(cbuffer_point(cb)) : Float8GetDatum(cb->radius);
    }
  }
  MeosType basetype = point ? T_GEOMETRY : T_FLOAT8;
  datumarr_sort(values, ss->totalcount, basetype);
  int count = datumarr_remove_duplicates(values, ss->totalcount, basetype);
  Set *result = set_make_exp(values, count, count, basetype, ORDER_NO);
  if (point)
  {
    for (int i = 0; i < ss->totalcount; i++)
      pfree(DatumGetPointer(values[i]));
  }
  pfree(values);
  return result;
}

/**
 * @ingroup meos_internal_cbuffer_accessor
 * @brief Return the points or radii or radius of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
static Set *
tcbuffer_members(const Temporal *temp, bool point)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tcbufferinst_members((TInstant *) temp, point);
    case TSEQUENCE:
      return tcbufferseq_members((TSequence *) temp, point);
    default: /* TSEQUENCESET */
      return tcbufferseqset_members((TSequenceSet *) temp, point);
  }
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of points or radius of a temporal circular buffer
 * @csqlfn #Tcbuffer_points()
 */
Set *
tcbuffer_points(const Temporal *temp)
{
  return tcbuffer_members(temp, true);
}

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the array of radii of a temporal circular buffer
 * @csqlfn #Tcbuffer_radius()
 */
Set *
tcbuffer_radius(const Temporal *temp)
{
  return tcbuffer_members(temp, false);
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_accessor
 * @brief Return the value of a temporal circular buffer at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tcbuffer_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Cbuffer **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, false); VALIDATE_NOT_NULL(value, false);
  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetCbufferP(res);
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Return a temporal circular buffer instant with the radius expanded by
 * a distance
 */
static TInstant *
tcbufferinst_expand(const TInstant *inst, double dist)
{
  assert(inst); assert(inst->temptype == T_TCBUFFER);
  const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  const POINT2D *p = cbuffer_point2d_p(cb);
  Cbuffer *result = cbuffer_make_coords(cbuffer_srid(cb), p->x, p->y,
    cbuffer_radius(cb) + dist);
  return tinstant_make_free(PointerGetDatum(result), T_TCBUFFER, inst->t);
}

/**
 * @brief Return a temporal circular buffer sequence with the radius expanded by
 * a distance
 */
static TSequence *
tcbufferseq_expand(const TSequence *seq, double dist)
{
  assert(seq); assert(seq->temptype == T_TCBUFFER);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tcbufferinst_expand(TSEQUENCE_INST_N(seq, i), dist);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Return a temporal circular buffer sequence set with the radius
 * expanded by a distance
 */
static TSequenceSet *
tcbufferseqset_expand(const TSequenceSet *ss, double dist)
{
  assert(ss); assert(ss->temptype == T_TCBUFFER);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_expand(TSEQUENCESET_SEQ_N(ss, i), dist);
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE_NO);
}

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return a temporal circular buffer with the radius expanded by a
 * distance
 * @param[in] temp Temporal value
 * @param[in] dist Distance
 * @csqlfn #Tcbuffer_expand()
 * @note The radius of each instant is expanded in a single pass, sourcing the
 * point and radius directly from each circular buffer, which is equivalent to
 * decomposing the value into its temporal point and temporal float radius,
 * adding the distance to the radius, and recomposing the circular buffer
 */
Temporal *
tcbuffer_expand(const Temporal *temp, double dist)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tcbufferinst_expand((TInstant *) temp, dist);
    case TSEQUENCE:
      return (Temporal *) tcbufferseq_expand((TSequence *) temp, dist);
    default: /* TSEQUENCESET */
      return (Temporal *) tcbufferseqset_expand((TSequenceSet *) temp, dist);
  }
}

/**
 * @brief Return a temporal circular buffer moved by a planar rigid motion
 * @details The motion is lifted as #tpose_compose_pose lifts a composition
 * with a fixed frame. It moves every centre and keeps every radius, so a
 * linear segment of circular buffers moves to the segment between the moved
 * ones and the result keeps the interpolation of the input
 * @param[in] temp Temporal circular buffer
 * @param[in] frame Offsets and angle of the motion, as #cbuffer_motion reads
 * them
 */
static Temporal *
tcbuffer_motion(const Temporal *temp, const double *frame)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_cbuffer_motion;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TCBUFFER;
  lfinfo.argtype[1] = T_TCBUFFER;
  lfinfo.restype = T_TCBUFFER;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  return tfunc_temporal_base(temp, PointerGetDatum(frame), &lfinfo);
}

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return a temporal circular buffer translated by offsets, as
 * #tgeo_translate translates a temporal geo
 * @details A circular buffer is planar, so the translation takes no vertical
 * offset
 * @param[in] temp Temporal circular buffer
 * @param[in] deltax,deltay Offsets
 * @csqlfn #Tcbuffer_translate()
 */
Temporal *
tcbuffer_translate(const Temporal *temp, double deltax, double deltay)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  if (! ensure_not_geodetic(temp->flags))
    return NULL;
  const double frame[3] = {deltax, deltay, 0.0};
  return tcbuffer_motion(temp, frame);
}

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return a temporal circular buffer rotated counter-clockwise about
 * the vertical through a point, as #tgeo_rotate rotates a temporal geo
 * @details The rotation carries the point @p (x0, y0) to itself, so its
 * offsets are that point minus its image by the rotation about the origin
 * @param[in] temp Temporal circular buffer
 * @param[in] angle Angle in radians
 * @param[in] x0,y0 Coordinates of the centre of the rotation
 * @csqlfn #Tcbuffer_rotate()
 */
Temporal *
tcbuffer_rotate(const Temporal *temp, double angle, double x0, double y0)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL);
  if (! ensure_not_geodetic(temp->flags))
    return NULL;
  double s = sin(angle), c = cos(angle);
  const double frame[3] = {x0 - (c * x0 - s * y0), y0 - (s * x0 + c * y0),
    angle};
  return tcbuffer_motion(temp, frame);
}

/**
 * @ingroup meos_cbuffer_transf
 * @brief Return a temporal circular buffer rotated counter-clockwise about
 * the z axis, as #tgeo_rotate_z rotates a temporal geo
 * @param[in] temp Temporal circular buffer
 * @param[in] angle Angle in radians
 * @csqlfn #Tcbuffer_rotate_z()
 */
Temporal *
tcbuffer_rotate_z(const Temporal *temp, double angle)
{
  return tcbuffer_rotate(temp, angle, 0.0, 0.0);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tcbuffer_restrict_cbuffer(const Temporal *temp, const Cbuffer *cb, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  /* Bounding box test: a temporal circular buffer equals a circular buffer
   * value only at the instants where their positions and radii coincide, so a
   * non-overlap of the (radius-aware) bounding boxes leaves no matching
   * instant. Mirrors the geometry and box restrictions. */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  cbuffer_set_stbox(cb, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  return temporal_restrict_value(temp, PointerGetDatum(cb), atfunc);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @csqlfn #Tcbuffer_at_cbuffer()
 */
Temporal *
tcbuffer_at_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcbuffer_restrict_cbuffer(temp, cb, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * circular buffer
 * @param[in] temp Temporal value
 * @param[in] cb Value
 * @csqlfn #Tcbuffer_minus_cbuffer()
 */
Temporal *
tcbuffer_minus_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  return tcbuffer_restrict_cbuffer(temp, cb, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_at_stbox()
 */
/**
 * @brief Return true when some unit of a temporal circular buffer can reach the
 * space of the box @p box
 * @details A disk meets a box exactly when its centre lies within the radius of
 * it, so a unit whose centre box stands farther than the larger of its two
 * radii from the box reaches it at no instant. The box of the value cannot
 * express this: it is widened by the radius and spans the whole trajectory, so
 * it overlaps wherever the disks reach anywhere, and on a spatial join it
 * passes nearly every pair into the running relationship below.
 *
 * Reading the centre against a radius-widened threshold, rather than a
 * radius-widened box against the box, keeps the radius from being removed on
 * both axes at once. The centre box and the radius maximum both
 * over-approximate the unit, so a value that does reach the box is never
 * rejected, and the scan is linear in the instants and touches no geometry.
 */
static bool
tcbuffer_may_reach_box(const Temporal *temp, const STBox *box)
{
  assert(temp); assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p((const TInstant *) temp));
    const POINT2D *p = cbuffer_point2d_p(cb);
    double dx = Max(Max(box->xmin - p->x, p->x - box->xmax), 0.0);
    double dy = Max(Max(box->ymin - p->y, p->y - box->ymax), 0.0);
    return dx * dx + dy * dy <= cb->radius * cb->radius;
  }
  int nseqs = (temp->subtype == TSEQUENCE) ? 1 :
    ((const TSequenceSet *) temp)->count;
  for (int s = 0; s < nseqs; s++)
  {
    const TSequence *seq = (temp->subtype == TSEQUENCE) ?
      (const TSequence *) temp :
      TSEQUENCESET_SEQ_N((const TSequenceSet *) temp, s);
    const Cbuffer *cb1 = DatumGetCbufferP(
      tinstant_value_p(TSEQUENCE_INST_N(seq, 0)));
    const POINT2D *p1 = cbuffer_point2d_p(cb1);
    for (int i = 1; i <= seq->count - 1; i++)
    {
      const Cbuffer *cb2 = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p2 = cbuffer_point2d_p(cb2);
      double r = fmax(cb1->radius, cb2->radius);
      double cxmin = fmin(p1->x, p2->x), cxmax = fmax(p1->x, p2->x);
      double cymin = fmin(p1->y, p2->y), cymax = fmax(p1->y, p2->y);
      double dx = Max(Max(box->xmin - cxmax, cxmin - box->xmax), 0.0);
      double dy = Max(Max(box->ymin - cymax, cymin - box->ymax), 0.0);
      if (dx * dx + dy * dy <= r * r)
        return true;
      cb1 = cb2; p1 = p2;
    }
    if (seq->count == 1)
    {
      double r = cb1->radius;
      double dx = Max(Max(box->xmin - p1->x, p1->x - box->xmax), 0.0);
      double dy = Max(Max(box->ymin - p1->y, p1->y - box->ymax), 0.0);
      if (dx * dx + dy * dy <= r * r)
        return true;
    }
  }
  return false;
}

Temporal *
tcbuffer_restrict_stbox(const Temporal *temp, const STBox *box,
  bool border_inc UNUSED, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TCBUFFER(temp, NULL); VALIDATE_NOT_NULL(box, NULL);
  if (! ensure_valid_tcbuffer_stbox(temp, box))
    return NULL;

  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  assert(hasx || hast);

  /* A box carrying only the time dimension restricts on the period alone */
  if (! hasx)
    return temporal_restrict_tstzspan(temp, &box->period, atfunc);

  /* Bounding box test. The box of the value is radius-aware, so it encloses
   * the swept disks and a miss proves that no disk ever reaches the box */
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  if (! overlaps_stbox_stbox(&box1, box))
    return atfunc ? NULL : temporal_copy(temp);

  /* Per-unit reach test on the centre, which the widened box above cannot
   * express: a value no unit of which brings a disk within reach of the box
   * meets it nowhere, and the restriction is settled without the running
   * relationship below and the decomposition it builds */
  if (! tcbuffer_may_reach_box(temp, box))
    return atfunc ? NULL : temporal_copy(temp);

  /* Restrict by the circular disk footprint, not the centre trajectory: the
   * buffer meets the box exactly when the moving disk intersects it, which is
   * the same reading the geometry restriction takes, and the space of a box is
   * a geometry like any other. A disk overlapping the box while its centre
   * stays outside belongs to the result, and the centre reading drops it. That
   * is the true time of the (arc-exact) temporal intersects relationship, so
   * the box restriction reduces to restricting the buffer to that time (`at`)
   * or to its complement (`minus`), intersected with the period the box
   * carries. Slicing the buffer by time keeps the radius without rebuilding the
   * value from its point and radius projections, and interpolates the same
   * point and radius at any crossing. */
  GSERIALIZED *geo = stbox_to_geo(box);
  if (! geo)
    return atfunc ? NULL : temporal_copy(temp);
  Temporal *tinter = tinterrel_tcbuffer_geo(temp, geo, TINTERSECTS);
  pfree(geo);
  if (! tinter)
    return atfunc ? NULL : temporal_copy(temp);
  SpanSet *ss = tbool_when_true(tinter);
  pfree(tinter);
  if (! ss)
    return atfunc ? NULL : temporal_copy(temp);
  if (hast)
  {
    SpanSet *ss1 = intersection_spanset_span(ss, &box->period);
    pfree(ss);
    if (! ss1)
      return atfunc ? NULL : temporal_copy(temp);
    ss = ss1;
  }
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  pfree(ss);
  return result;
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a spatiotemporal box
 * @param[in] temp Temporal value
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_at_stbox()
 */
Temporal *
tcbuffer_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tcbuffer_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] box Value
 * @param[in] border_inc True when the box contains the upper border, otherwise
 * the upper border is assumed as outside of the box.
 * @csqlfn #Tcbuffer_minus_stbox()
 */
Temporal *
tcbuffer_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tcbuffer_restrict_stbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @csqlfn #Tcbuffer_at_geom()
 */
Temporal *
tcbuffer_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, bool
  atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return atfunc ? NULL : temporal_copy(temp);

  /* Restrict by the circular disk footprint, not the centre trajectory: the
   * buffer meets the geometry exactly when the moving disk intersects it. That
   * is the true time of the (arc-exact) temporal intersects relationship,
   * so the geometry restriction reduces to restricting the buffer to that time
   * (`at`) or its complement (`minus`) */
  Temporal *tinter = tinterrel_tcbuffer_geo(temp, gs, TINTERSECTS);
  if (! tinter)
    return atfunc ? NULL : temporal_copy(temp);
  SpanSet *ss = tbool_when_true(tinter);
  pfree(tinter);
  if (! ss)
    return atfunc ? NULL : temporal_copy(temp);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  pfree(ss);
  return result;
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to a geometry
 * @param[in] temp Temporal value
 * @param[in] gs Geometry
 * @csqlfn #Tcbuffer_at_geom()
 */
Temporal *
tcbuffer_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs))
    return NULL;
  return tcbuffer_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_cbuffer_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * geometry
 * @param[in] temp Temporal value
 * @param[in] gs Value
 * @csqlfn #Tcbuffer_minus_geom()
 */
Temporal *
tcbuffer_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs))
    return NULL;
  return tcbuffer_restrict_geom(temp, gs, REST_MINUS);
}

/*****************************************************************************/

