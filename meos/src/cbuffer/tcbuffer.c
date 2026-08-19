/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Basic functions for temporal circular buffers
 */

/* C */
#include <assert.h>
#include <limits.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_util.h"
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
 * @brief Compute the temporal distance between two circular buffers at time t
 * @param[in] dx0,dy0 Initial spatial offset between the buffer centres.
 * @param[in] vx,vy Relative velocities in the x and y directions.
 * @param[in] r0 Initial sum of the radii.
 * @param[in] vr Rate of change of the sum of the radii.
 * @param[in] t Time offset from the lower bound of the segment.
 * @return The temporal distance at time t, i.e., the Euclidean distance between
 * the centres minus the sum of the radii.
 */
static inline double
tcbuffersegm_distance_at_time(double dx0, double dy0, double vx, double vy,
  double r0, double vr, double t)
{
  double dx = dx0 + vx * t;
  double dy = dy0 + vy * t;
  double sum_r = r0 + vr * t;
  return sqrt(dx * dx + dy * dy) - sum_r;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments are within a
 * distance during the period defined by the output timestampos, return 0
 * otherwise
 * @details These are the turning points when computing the temporal distance.
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
  /* Extract the circular buffers and the distance */
  Cbuffer *sv1 = DatumGetCbufferP(start1);
  Cbuffer *ev1 = DatumGetCbufferP(end1);
  Cbuffer *sv2 = DatumGetCbufferP(start2);
  Cbuffer *ev2 = DatumGetCbufferP(end2);
  double d = DatumGetFloat8(dist);

  /* Extract the points */
  const POINT2D *spt1 = cbuffer_point2d_p(sv1);
  const POINT2D *ept1 = cbuffer_point2d_p(ev1);
  const POINT2D *spt2 = cbuffer_point2d_p(sv2);
  const POINT2D *ept2 = cbuffer_point2d_p(ev2);

  double duration = (double)(upper - lower);

  /* Tolerance threshold for floating-point comparison */
  if (duration <= FP_TOLERANCE)
  {
    *t1 = *t2 = 0;
    return 0;
  }

  /* Initial relative positions and combined radii */
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  double r0 = sv1->radius + sv2->radius;

  /* Relative velocities */
  double vx = (ept1->x - spt1->x - (ept2->x - spt2->x)) / duration;
  double vy = (ept1->y - spt1->y - (ept2->y - spt2->y)) / duration;
  double vr = (ev1->radius - sv1->radius + ev2->radius - sv2->radius) /
    duration;

  /* Quadratic derivative coefficients of f(t) = (distance - d)^2 */
  double a = vx * vx + vy * vy - vr * vr;
  double b = 2*(dx0 * vx + dy0 * vy - (r0 + d) * vr);
  double c = dx0 * dx0 + dy0 * dy0 - (r0 + d) * (r0+d);
  double delta = b * b - 4 * a * c;

  double roots[2];
  int nroots = 0;

  /* Linear case */
  double d1;
  if (delta >= -FP_TOLERANCE)
  {
    double t_cand1, t_cand2;
    if (a == 0 && fabs(b) >= FP_TOLERANCE)
    {
      t_cand1 = -c / b;
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
    }
    /* Quadratic case */
    else
    {
      double sqrt_delta = sqrt(fmax(0.0, delta));
      t_cand1 = (-b - sqrt_delta) / (2*a);
      t_cand2 = (-b + sqrt_delta) / (2*a);
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
      if (fabs(t_cand2 - t_cand1) > FP_TOLERANCE && t_cand2 >= -FP_TOLERANCE &&
          t_cand2 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand2);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand2;
      }
    }
  }
  /* Filter to only strictly internal timestamps: t ∈ (lower, upper).
   * Boundary roots (t == lower or t == upper) are not "crossings" the
   * caller needs to splice; they are handled by the surrounding sequence
   * construction logic. Returning them causes duplicate-timestamp errors. */
  double valid[2];
  int nvalid = 0;
  for (int i = 0; i < nroots; i++)
  {
    TimestampTz t = lower + (TimestampTz) roots[i];
    if (t > lower && t < upper)
    {
      if (nvalid == 0 || fabs(roots[i] - valid[nvalid - 1]) > FP_TOLERANCE)
        valid[nvalid++] = roots[i];
    }
  }
  if (nvalid == 0)
  {
    *t1 = *t2 = (TimestampTz) 0;
    return 0;
  }
  else if (nvalid == 1)
  {
    *t1 = *t2 = lower + (TimestampTz) valid[0];
    return 1;
  }
  else
  {
    *t1 = lower + (TimestampTz) valid[0];
    *t2 = lower + (TimestampTz) valid[1];
    return 2;
  }
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
 * endpoints.
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
  Cbuffer *sv1 = DatumGetCbufferP(start1);
  Cbuffer *ev1 = DatumGetCbufferP(end1);
  Cbuffer *sv2 = DatumGetCbufferP(start2);
  Cbuffer *ev2 = DatumGetCbufferP(end2);
  double d = DatumGetFloat8(dist);
  const POINT2D *spt1 = cbuffer_point2d_p(sv1);
  const POINT2D *ept1 = cbuffer_point2d_p(ev1);
  const POINT2D *spt2 = cbuffer_point2d_p(sv2);
  const POINT2D *ept2 = cbuffer_point2d_p(ev2);
  double duration = (double) (upper - lower);
  if (duration <= FP_TOLERANCE)
  {
    *t1 = *t2 = 0;
    return 0;
  }
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  double r0 = sv1->radius + sv2->radius;
  double vx = (ept1->x - spt1->x - (ept2->x - spt2->x)) / duration;
  double vy = (ept1->y - spt1->y - (ept2->y - spt2->y)) / duration;
  double vr = (ev1->radius - sv1->radius + ev2->radius - sv2->radius) /
    duration;
  double a = vx * vx + vy * vy - vr * vr;
  double b = 2 * (dx0 * vx + dy0 * vy - (r0 + d) * vr);
  double c = dx0 * dx0 + dy0 * dy0 - (r0 + d) * (r0 + d);
  double delta = b * b - 4 * a * c;
  double roots[2];
  int nroots = 0;
  if (delta >= -FP_TOLERANCE)
  {
    double t_cand1, d1;
    if (a == 0 && fabs(b) >= FP_TOLERANCE)
    {
      t_cand1 = -c / b;
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
    }
    else
    {
      double sqrt_delta = sqrt(fmax(0.0, delta));
      t_cand1 = (-b - sqrt_delta) / (2 * a);
      double t_cand2 = (-b + sqrt_delta) / (2 * a);
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
      if (fabs(t_cand2 - t_cand1) > FP_TOLERANCE && t_cand2 >= -FP_TOLERANCE &&
          t_cand2 <= duration + FP_TOLERANCE)
      {
        d1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand2);
        if (fabs(d1 - d) < FP_TOLERANCE) roots[nroots++] = t_cand2;
      }
    }
  }
  if (nroots == 2 && roots[0] > roots[1])
  {
    double tmp = roots[0]; roots[0] = roots[1]; roots[1] = tmp;
  }
  /* Within status at the two segment endpoints */
  bool win_lower = (tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr,
    0.0) <= d + FP_TOLERANCE);
  bool win_upper = (tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr,
    duration) <= d + FP_TOLERANCE);
  double tstart, tend;
  if (nroots == 0)
  {
    /* No crossing: within throughout the segment, or never */
    if (! win_lower)
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
    tstart = 0.0; tend = duration;
  }
  else if (nroots == 1)
  {
    /* One crossing: within on the side whose endpoint is within */
    if (win_lower && ! win_upper)
    {
      tstart = 0.0; tend = roots[0];
    }
    else if (! win_lower && win_upper)
    {
      tstart = roots[0]; tend = duration;
    }
    else
    {
      /* Tangent touch: within only at the crossing instant */
      *t1 = *t2 = lower + (TimestampTz) roots[0];
      return 1;
    }
  }
  else
  {
    /* Two crossings: within between them */
    tstart = roots[0]; tend = roots[1];
  }
  if (tend - tstart < FP_TOLERANCE)
  {
    *t1 = *t2 = lower + (TimestampTz) tstart;
    return 1;
  }
  *t1 = lower + (TimestampTz) tstart;
  *t2 = lower + (TimestampTz) tend;
  return 2;
}

/**
 * @brief Return 1 or 2 if the first temporal circular buffer segment contains
 * (@p strict) or covers the second one during a sub-period of the segment,
 * return 0 otherwise
 * @details A moving disk (P1, R1) contains a moving disk (P2, R2) when
 * dist(P1, P2) + R2 <= R1, that is when the clearance
 * g(t) = dist(P1(t), P2(t)) - (R1(t) - R2(t)) is non-positive; g is the distance
 * of the centres minus the radius DIFFERENCE, so this mirrors
 * #tcbuffersegm_tdwithin_turnpt with the radius sum replaced by the difference
 * and the distance set to zero. It returns the sub-interval [t1, t2] of
 * [lower, upper] during which the relation holds, so the temporal contains and
 * covers Boolean is true on a continuous interval rather than only at the
 * clearance minimum. With @p strict true an isolated tangency (g = 0 at a single
 * instant) does not count, matching the strict interior of contains.
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
  bool is_strict = (DatumGetFloat8(strict) != 0);
  const POINT2D *spt1 = cbuffer_point2d_p(sv1);
  const POINT2D *ept1 = cbuffer_point2d_p(ev1);
  const POINT2D *spt2 = cbuffer_point2d_p(sv2);
  const POINT2D *ept2 = cbuffer_point2d_p(ev2);
  double duration = (double) (upper - lower);
  if (duration <= FP_TOLERANCE)
  {
    *t1 = *t2 = 0;
    return 0;
  }
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  /* Radius DIFFERENCE R1 - R2 (contains threshold), not the sum */
  double r0 = sv1->radius - sv2->radius;
  double vx = (ept1->x - spt1->x - (ept2->x - spt2->x)) / duration;
  double vy = (ept1->y - spt1->y - (ept2->y - spt2->y)) / duration;
  double vr = (ev1->radius - sv1->radius - (ev2->radius - sv2->radius)) /
    duration;
  /* Clearance g(t) = dist(centres) - (r0 + vr t); crossings solve g = 0, i.e.
   * dist^2 = (r0 + vr t)^2, valid only where r0 + vr t >= 0 */
  double a = vx * vx + vy * vy - vr * vr;
  double b = 2 * (dx0 * vx + dy0 * vy - r0 * vr);
  double c = dx0 * dx0 + dy0 * dy0 - r0 * r0;
  double delta = b * b - 4 * a * c;
  double roots[2];
  int nroots = 0;
  if (delta >= -FP_TOLERANCE)
  {
    double t_cand1, g1;
    if (a == 0 && fabs(b) >= FP_TOLERANCE)
    {
      t_cand1 = -c / b;
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        g1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(g1) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
    }
    else
    {
      double sqrt_delta = sqrt(fmax(0.0, delta));
      t_cand1 = (-b - sqrt_delta) / (2 * a);
      double t_cand2 = (-b + sqrt_delta) / (2 * a);
      if (t_cand1 >= -FP_TOLERANCE && t_cand1 <= duration + FP_TOLERANCE)
      {
        g1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand1);
        if (fabs(g1) < FP_TOLERANCE) roots[nroots++] = t_cand1;
      }
      if (fabs(t_cand2 - t_cand1) > FP_TOLERANCE && t_cand2 >= -FP_TOLERANCE &&
          t_cand2 <= duration + FP_TOLERANCE)
      {
        g1 = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, t_cand2);
        if (fabs(g1) < FP_TOLERANCE) roots[nroots++] = t_cand2;
      }
    }
  }
  if (nroots == 2 && roots[0] > roots[1])
  {
    double tmp = roots[0]; roots[0] = roots[1]; roots[1] = tmp;
  }
  /* Contains/covers status at the two segment endpoints: covers is g <= 0,
   * strict contains is g < 0, so identical or internally tangent disks
   * (g == 0 throughout) covers but never strictly contains */
  double g_lower = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr, 0.0);
  double g_upper = tcbuffersegm_distance_at_time(dx0, dy0, vx, vy, r0, vr,
    duration);
  bool in_lower = is_strict ? (g_lower < - FP_TOLERANCE) :
    (g_lower <= FP_TOLERANCE);
  bool in_upper = is_strict ? (g_upper < - FP_TOLERANCE) :
    (g_upper <= FP_TOLERANCE);
  double tstart, tend;
  if (nroots == 0)
  {
    if (! in_lower)
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
    tstart = 0.0; tend = duration;
  }
  else if (nroots == 1)
  {
    if (in_lower && ! in_upper)
    {
      tstart = 0.0; tend = roots[0];
    }
    else if (! in_lower && in_upper)
    {
      tstart = roots[0]; tend = duration;
    }
    else
    {
      /* Isolated tangency: the disks graze from inside at a single instant.
       * Covers holds there, strict contains does not. */
      if (is_strict)
      {
        *t1 = *t2 = (TimestampTz) 0;
        return 0;
      }
      *t1 = *t2 = lower + (TimestampTz) roots[0];
      return 1;
    }
  }
  else
  {
    tstart = roots[0]; tend = roots[1];
  }
  if (tend - tstart < FP_TOLERANCE)
  {
    *t1 = *t2 = lower + (TimestampTz) tstart;
    return 1;
  }
  *t1 = lower + (TimestampTz) tstart;
  *t2 = lower + (TimestampTz) tend;
  return 2;
}

/**
 * @brief Return 1 or 2 if two temporal circular buffer segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
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
  if (s <= FP_TOLERANCE)
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
    if (f <= MEOS_EPSILON || f >= 1.0 - MEOS_EPSILON)
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
 * @brief Return 1 or 2 if a temporal circular buffer segment and a circular
 * buffer intersect during the period defined by the output timestamps, return
 * 0 otherwise
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
 * @return On error return @p NULL
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
    value = geom_min_bounding_radius(value, &radius);
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
 * @return On error return @p NULL
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
 * @return On error return @p NULL
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
 * @csqlfn #Tcbuffer_points()
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

