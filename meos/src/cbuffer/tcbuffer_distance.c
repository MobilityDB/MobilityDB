/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Temporal distance for temporal circular buffers
 */

/* C */
#include <float.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "geo/stbox.h"
#include "geo/tgeo.h"
#include "geo/tgeo_distance.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer.h"

/*****************************************************************************
 * Turning point functions
 *****************************************************************************/

/**
 * @brief Return 1 or 3 if a temporal circular buffer segment and a geometry
 * point are at the minimum distance during the period defined by the output
 * timestamps, return 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start,end Values defining the segment
 * @param[in] value Value to locate
 * @param[in] lower,upper Timestampts defining the segment
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segment is not constant.
 */
int
tcbuffer_cbuffer_distance_turnpt(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  /* Extract the two CBUFFER values */
  Cbuffer *ca1 = DatumGetCbufferP(start);
  const GSERIALIZED *gs1 = cbuffer_point_p(ca1);
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(gs1);
  Cbuffer *ca2 = DatumGetCbufferP(end);
  const GSERIALIZED *gs2 = cbuffer_point_p(ca2);
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(gs2);

  /* Extract the circular buffer value */
  Cbuffer *cb = DatumGetCbufferP(value);
  const GSERIALIZED *gs = cbuffer_point_p(cb);
  const POINT2D *p = GSERIALIZED_POINT2D_P(gs);

  /* Extract coordinates and radius at the two instants */
  double xa1 = p1->x;
  double ya1 = p1->y;
  double ra1 = ca1->radius;
  double xa2 = p2->x;
  double ya2 = p2->y;
  double ra2 = ca2->radius;

  /* Extract static cbuffer coordinates */
  double xb = p->x;
  double yb = p->y;
  double rb = cb->radius;

  /* Compute total duration in seconds */
  double total_duration = (double) (upper - lower) / USECS_PER_SEC;

  /* Initial relative position and radius at lower */
  double dx0 = xa1 - xb;
  double dy0 = ya1 - yb;
  double dr0 = ra1 + rb;

  /* Compute relative velocities */
  double vx = (xa2 - xa1) / total_duration;
  double vy = (ya2 - ya1) / total_duration;
  double vr = (ra2 - ra1) / total_duration;

  /* Coefficients of the derivative of (distance - radius)^2 */
  double a = vx * vx + vy * vy - vr * vr;
  double b = dx0 * vx + dy0 * vy - dr0 * vr;

  /* Compute relative time (in seconds) where derivative is zero */
  double t_rel;
  if (a == 0.0 || b == 0.0)
    t_rel = 0.0;
  else
    t_rel = -b / a;

  /* Clamp t_rel within [0, total_duration] */
  if (t_rel < 0.0)
    t_rel = 0.0;
  else if (t_rel > total_duration)
    t_rel = total_duration;

  /* Compute the timestamp at the turning point */
  TimestampTz t_turn = lower + (TimestampTz) (t_rel * USECS_PER_SEC);

  /* Check if the turning point is truly internal */
  if (t_turn <= lower || t_turn >= upper)
  {
    /* No true internal turning point */
    *t1 = *t2 = (TimestampTz) 0;
    return 0;
  }

  /* Interpolate position and radius at the turning point */
  double x_turn = xa1 + vx * t_rel;
  double y_turn = ya1 + vy * t_rel;
  double r_turn = ra1 + vr * t_rel;

  /* Compute the distance to the static point minus the radius */
  double dx = x_turn - xb;
  double dy = y_turn - yb;
  double dist = sqrt(dx * dx + dy * dy) - r_turn - rb;

  /* Interpolate the distances at start and end */
  double dx_start = xa1 - xb;
  double dy_start = ya1 - yb;

  double dx_end = xa2 - xb;
  double dy_end = ya2 - yb;

  if (dist > 0.0)
  {
    /* Single turning point: return t1 and value1 */
    *t1 = *t2 = t_turn;
    return 1;
  }
  else
  {
    /* Crossing zero: compute entrance and exit times */
    double dist_start = sqrt(dx_start * dx_start + dy_start * dy_start) - ra1 - rb;
    double dist_end = sqrt(dx_end * dx_end + dy_end * dy_end) - ra2 - rb;
    double alpha_in = (0.0 - dist_start) / (dist - dist_start);
    double alpha_out = (0.0 - dist) / (dist_end - dist);

    double t_in_secs = 0.0 + t_rel * alpha_in;
    double t_out_secs = t_rel + (total_duration - t_rel) * alpha_out;

    TimestampTz t_in = lower + (TimestampTz) (t_in_secs * USECS_PER_SEC);
    TimestampTz t_out = lower + (TimestampTz) (t_out_secs * USECS_PER_SEC);

    /* Order the turning points */
    if (t_in > t_out)
    {
      TimestampTz t = t_in;
      t_in = t_out;
      t_out = t;
    }

    /* Keep only turning points strictly inside (lower, upper) */
    bool in_internal  = (t_in  > lower && t_in  < upper);
    bool out_internal = (t_out > lower && t_out < upper);
    if (in_internal && out_internal)
    {
      *t1 = t_in;
      *t2 = t_out;
      return 2;
    }
    else if (in_internal)
    {
      *t1 = *t2 = t_in;
      return 1;
    }
    else if (out_internal)
    {
      *t1 = *t2 = t_out;
      return 1;
    }
    else
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
  }
}

/**
 * @brief Return 1 or 2 if two temporal circular buffers segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start1,end1 Circular buffers defining the first segment
 * @param[in] start2,end2 Circular buffers the second segment
 * @param[out] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not constant.
 */
int
cbuffersegm_distance_turnpt(const Cbuffer *start1, const Cbuffer *end1,
  const Cbuffer *start2, const Cbuffer *end2, TimestampTz lower,
  TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(start1); assert(end1); assert(start2); assert(end2); 
  assert(t1); assert(t2); assert(lower < upper);

  /* Extract the point values for the circular buffers */
  const POINT2D *spt1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(start1));
  const POINT2D *ept1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(end1));
  const POINT2D *spt2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(start2));
  const POINT2D *ept2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(end2));

  /* Compute the duration */
  double duration = (double) (upper - lower);

  /* Compute relative position and combined radius at lower */
  double dx0 = spt1->x - spt2->x;
  double dy0 = spt1->y - spt2->y;
  double r0 = start1->radius + start2->radius;

  double dx1 = ept1->x - ept2->x;
  double dy1 = ept1->y - ept2->y;

  /* Compute relative velocities */
  double vx = (ept1->x - spt1->x) / duration - (ept2->x - spt2->x) / duration;
  double vy = (ept1->y - spt1->y) / duration - (ept2->y - spt2->y) / duration;
  double vr = (end1->radius - start1->radius + end2->radius - start2->radius) / duration;

  /* Compute coefficients of the derivative of (distance - combined_radius)^2 */
  double a = vx*vx + vy*vy - vr*vr;
  double b = dx0*vx + dy0*vy - r0*vr;

  /* Compute relative time where derivative is zero */
  double t_rel = (a == 0.0 || b == 0.0) ? 0.0 : -b / a;
  if (t_rel < 0.0) t_rel = 0.0;
  if (t_rel > duration) t_rel = duration;

  /* Interpolate position and radius at the turning point */
  double cx1 = spt1->x + ((ept1->x - spt1->x) / duration) * t_rel;
  double cy1 = spt1->y + ((ept1->y - spt1->y) / duration) * t_rel;
  double rbuf1 = start1->radius + (end1->radius - start1->radius) * t_rel / duration;

  double cx2 = spt2->x + ((ept2->x - spt2->x) / duration) * t_rel;
  double cy2 = spt2->y + ((ept2->y - spt2->y) / duration) * t_rel;
  double rbuf2 = start2->radius + (end2->radius - start2->radius) * t_rel / duration;

  double dx_turn = cx1 - cx2;
  double dy_turn = cy1 - cy2;
  double dist_turn = sqrt(dx_turn*dx_turn + dy_turn*dy_turn) - rbuf1 - rbuf2;

  /* Single turning point */
  if (dist_turn > 0.0)
  {
    TimestampTz t_turn = lower + (TimestampTz)(t_rel); 
    *t1 = *t2 = t_turn;
    return 1;
  }
  else
  {
    /* Crossing: compute entrance and exit times */
    double r1 = end1->radius + end2->radius;
    double dist0 = sqrt(dx0 * dx0 + dy0 * dy0) - r0;
    double dist1 = sqrt(dx1 * dx1 + dy1 * dy1) - r1;
    double alpha_in = (dist_turn - dist0 == 0.0) ? 
      (0.0 - dist0) : (0.0 - dist0) / (dist_turn - dist0);
    double alpha_out = (dist1 - dist_turn == 0.0) ?
      (0.0 - dist_turn) : (0.0 - dist_turn) / (dist1 - dist_turn);

    TimestampTz t_in = lower + (TimestampTz)(t_rel * alpha_in);
    TimestampTz t_out = lower + (TimestampTz)((t_rel +
      (duration - t_rel) * alpha_out));

    /* Keep only turning points strictly inside (lower, upper) */
    bool in_internal  = (t_in  > lower && t_in  < upper);
    bool out_internal = (t_out > lower && t_out < upper);
    if (in_internal && out_internal)
    {
      *t1 = t_in;
      *t2 = t_out;
      return 2;
    }
    else if (in_internal)
    {
      *t1 = *t2 = t_in;
      return 1;
    }
    else if (out_internal)
    {
      *t1 = *t2 = t_out;
      return 1;
    }
    else
    {
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
  }
}

/**
 * @brief Return 1 or 2 if two temporal circular buffers segments are at a
 * minimum distance during the periods defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Minimum distances at turning points
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tcbuffer_tcbuffer_distance_turnpt(Datum start1, Datum end1, Datum start2,
   Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  /* Extract the circular buffer values */
  Cbuffer *sv1 = DatumGetCbufferP(start1);
  Cbuffer *ev1 = DatumGetCbufferP(end1);
  Cbuffer *sv2 = DatumGetCbufferP(start2);
  Cbuffer *ev2 = DatumGetCbufferP(end2);
  return cbuffersegm_distance_turnpt(sv1, ev1, sv2, ev2, lower, upper, t1, t2);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the temporal distance between a temporal circular buffer and
 * a circular buffer
 * @csqlfn #Tdistance_tcbuffer_cbuffer()
 */
Temporal *
tdistance_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_cbuffer_distance;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_base = lfinfo.reslinear ?
    &tcbuffer_cbuffer_distance_turnpt : NULL;
  return tfunc_temporal_base(temp, PointerGetDatum(cb), &lfinfo);
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the temporal distance between a temporal circular buffer and
 * a geometry
 * @csqlfn #Tdistance_tcbuffer_geo()
 */
Temporal *
tdistance_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Cbuffer *cb = geom_to_cbuffer(gs);
  Temporal *result = tdistance_tcbuffer_cbuffer(temp, cb);
  pfree(cb);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the temporal distance between two temporal circular buffers
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Tdistance_tcbuffer_tcbuffer()
 */
Temporal *
tdistance_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_cbuffer_distance;
  lfinfo.argtype[0] = temp1->temptype;
  lfinfo.argtype[1] = temp2->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_temp = lfinfo.reslinear ? &tcbuffersegm_distance_turnpt : NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of the temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #NAI_tcbuffer_geo()
 * @note This function needs to be implemented TODO
 */
TInstant *
nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, gs);
  /* We do not call the function tgeompointinst_tcbufferinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of the circular buffer and a
 * temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #NAI_tcbuffer_cbuffer()
 * @note This function needs to be implemented TODO
 */
TInstant *
nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  GSERIALIZED *geom = cbuffer_to_geom(cb);
  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, geom);
  /* We do not call the function tgeompointinst_tcbufferinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom); pfree(geom);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of two temporal circular buffers
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #NAI_tcbuffer_tcbuffer()
 * @note This function needs to be implemented TODO
 */
TInstant *
nai_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *dist = tdistance_tcbuffer_tcbuffer(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_inst_p((const Temporal *) dist);
  /* The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp1, min->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp1->temptype, min->t);
  pfree(dist);
  return result;
}

/*****************************************************************************
 * GEOS-free analytic nearest approach distance between a temporal circular
 * buffer and a geometry
 *
 * The traversed region of a temporal circular buffer is the union of the
 * swept capsules of its segments; the distance to a geometry is the minimum
 * of the per-segment distances. For one segment the centre c(t) and radius
 * r(t) vary linearly in t. The distance from the swept capsule to a
 * geometry edge is min over t of [ dist(c(t), edge) - r(t) ], clamped at 0.
 * On each of the three projection regimes (closest to the edge start, to
 * the edge end, or perpendicular) the squared point distance is a quadratic
 * in t, so f(t) = sqrt(quadratic) - linear is minimised in closed form by
 * evaluating the interval endpoints and the roots of the squared
 * stationarity equation. A polygon that contains a segment centre gives a
 * zero distance directly. No GEOS call is made.
 *****************************************************************************/

/**
 * @brief Minimise f(t) = sqrt(A t^2 + B t + C) - (R0 + DR t) over [lo,hi]
 * @details Candidates are the interval endpoints and the real roots of the
 * stationarity equation squared, (A^2 - A DR^2) t^2 + (A B - B DR^2) t +
 * (B^2/4 - C DR^2) = 0. Spurious roots introduced by squaring only add
 * larger values, so the minimum over the candidates is exact.
 */
static double
tcb_minfun(double A, double B, double C, double R0, double DR, double lo,
  double hi)
{
  double cand[6];
  int nc = 0;
  cand[nc++] = lo;
  cand[nc++] = hi;
  double a2 = A * (A - DR * DR);
  double a1 = B * (A - DR * DR);
  double a0 = 0.25 * B * B - C * DR * DR;
  if (fabs(a2) > 1e-18)
  {
    double disc = a1 * a1 - 4.0 * a2 * a0;
    if (disc >= 0.0)
    {
      double sd = sqrt(disc);
      cand[nc++] = (-a1 + sd) / (2.0 * a2);
      cand[nc++] = (-a1 - sd) / (2.0 * a2);
    }
  }
  else if (fabs(a1) > 1e-18)
    cand[nc++] = -a0 / a1;
  double best = DBL_MAX;
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < lo) t = lo;
    if (t > hi) t = hi;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double f = sqrt(q) - (R0 + DR * t);
    if (f < best) best = f;
  }
  return best;
}

/**
 * @brief A geometry boundary segment with its precomputed 2D bounding box;
 * @p is_poly marks segments that belong to a polygon ring (used by the
 * ray-casting interior test)
 */
typedef struct
{
  double x1, y1, x2, y2;
  double xmin, ymin, xmax, ymax;
  bool is_poly;
} TcbSeg;

/**
 * @brief Append the segments of a point array to the segment array, growing
 * it as needed. A single-point array contributes one degenerate segment.
 */
static void
tcb_segs_add_ptarray(const POINTARRAY *pa, bool is_poly, TcbSeg **arr,
  int *cap, int *cnt)
{
  if (! pa || pa->npoints == 0)
    return;
  uint32_t np = pa->npoints;
  uint32_t nseg = (np == 1) ? 1 : np - 1;
  for (uint32_t i = 0; i < nseg; i++)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = (np == 1) ? a : getPoint2d_cp(pa, i + 1);
    if (*cnt == *cap)
    {
      int newcap = (*cap == 0) ? 64 : *cap * 2;
      *arr = (*arr == NULL) ? palloc(sizeof(TcbSeg) * newcap) :
        repalloc(*arr, sizeof(TcbSeg) * newcap);
      *cap = newcap;
    }
    TcbSeg *s = &(*arr)[(*cnt)++];
    s->x1 = a->x; s->y1 = a->y; s->x2 = b->x; s->y2 = b->y;
    s->xmin = fmin(a->x, b->x); s->xmax = fmax(a->x, b->x);
    s->ymin = fmin(a->y, b->y); s->ymax = fmax(a->y, b->y);
    s->is_poly = is_poly;
  }
}

/**
 * @brief Recursively collect the boundary segments of a geometry. Returns
 * false for a curved type that cannot be decomposed exactly (the caller
 * then falls back to the exact traversed-area path).
 */
static bool
tcb_geo_segs(const LWGEOM *lw, TcbSeg **arr, int *cap, int *cnt,
  bool *has_poly)
{
  switch (lw->type)
  {
    case POINTTYPE:
      tcb_segs_add_ptarray(lwgeom_as_lwpoint(lw)->point, false, arr, cap,
        cnt);
      return true;
    case LINETYPE:
      tcb_segs_add_ptarray(lwgeom_as_lwline(lw)->points, false, arr, cap,
        cnt);
      return true;
    case TRIANGLETYPE:
      tcb_segs_add_ptarray(lwgeom_as_lwtriangle(lw)->points, true, arr,
        cap, cnt);
      *has_poly = true;
      return true;
    case POLYGONTYPE:
    {
      const LWPOLY *p = lwgeom_as_lwpoly(lw);
      for (uint32_t i = 0; i < p->nrings; i++)
        tcb_segs_add_ptarray(p->rings[i], true, arr, cap, cnt);
      if (p->nrings > 0)
        *has_poly = true;
      return true;
    }
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *c = lwgeom_as_lwcollection(lw);
      for (uint32_t i = 0; i < c->ngeoms; i++)
        if (! tcb_geo_segs(c->geoms[i], arr, cap, cnt, has_poly))
          return false;
      return true;
    }
    default:
      /* Curved or unsupported type: let the caller use the exact path */
      return false;
  }
}

/**
 * @brief Ray-casting test: true if (x,y) is inside any polygon ring among
 * the segments (even-odd rule over the polygon-ring segments only)
 */
static bool
tcb_pt_in_polys(double x, double y, const TcbSeg *segs, int n)
{
  bool inside = false;
  for (int i = 0; i < n; i++)
  {
    if (! segs[i].is_poly)
      continue;
    double y1 = segs[i].y1, y2 = segs[i].y2;
    if ((y1 > y) != (y2 > y))
    {
      double xint = segs[i].x1 +
        (y - y1) / (y2 - y1) * (segs[i].x2 - segs[i].x1);
      if (x < xint)
        inside = ! inside;
    }
  }
  return inside;
}

/**
 * @brief Minimum of [ dist(c(t), edge) - r(t) ] for t in [0,1], where the
 * centre moves from (cx1,cy1) to (cx2,cy2) and the radius from r1 to r2
 */
static double
tcb_seg_edge_mindist(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const TcbSeg *e)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay;
  const double l2 = ux * ux + uy * uy;

  /* Degenerate edge (a point): distance to that point over the whole t */
  if (l2 <= 1e-24)
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    return tcb_minfun(A, B, C, r1, dr, 0.0, 1.0);
  }

  /* Projection parameter s(t) = (s0 + s1 t) / l2, split [0,1] at s=0, s=1 */
  const double s0 = (cx1 - ax) * ux + (cy1 - ay) * uy;
  const double s1 = dcx * ux + dcy * uy;
  double bp[4];
  int nb = 0;
  bp[nb++] = 0.0;
  bp[nb++] = 1.0;
  if (fabs(s1) > 1e-18)
  {
    double ta = -s0 / s1, tb = (l2 - s0) / s1;
    if (ta > 0.0 && ta < 1.0) bp[nb++] = ta;
    if (tb > 0.0 && tb < 1.0) bp[nb++] = tb;
  }
  /* Sort the (at most 4) breakpoints */
  for (int i = 0; i < nb; i++)
    for (int j = i + 1; j < nb; j++)
      if (bp[j] < bp[i]) { double tmp = bp[i]; bp[i] = bp[j]; bp[j] = tmp; }

  double best = DBL_MAX;
  for (int k = 0; k + 1 < nb; k++)
  {
    double lo = bp[k], hi = bp[k + 1];
    if (hi - lo < 1e-15) continue;
    double mt = 0.5 * (lo + hi);
    double s = (s0 + s1 * mt) / l2;
    double A, B, C;
    if (s <= 0.0)
    {
      /* Closest to the edge start A */
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
      C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    }
    else if (s >= 1.0)
    {
      /* Closest to the edge end B */
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - bx) * dcx + (cy1 - by) * dcy);
      C = (cx1 - bx) * (cx1 - bx) + (cy1 - by) * (cy1 - by);
    }
    else
    {
      /* Perpendicular distance: cross(c(t)-A, u) / |u| */
      double k0 = (cx1 - ax) * uy - (cy1 - ay) * ux;
      double k1 = dcx * uy - dcy * ux;
      A = k1 * k1 / l2;
      B = 2.0 * k0 * k1 / l2;
      C = k0 * k0 / l2;
    }
    double m = tcb_minfun(A, B, C, r1, dr, lo, hi);
    if (m < best) best = m;
  }
  return best;
}

/*****************************************************************************
 * Shortest line: same closed-form minimisation, but tracking the witness
 * (the parameter on the swept-capsule segment that attains the minimum and
 * the nearest point on the geometry edge), so the connecting line can be
 * built without GEOS. The nearest-approach value path above is left
 * untouched.
 *****************************************************************************/

/**
 * @brief Like #tcb_minfun but also returns the argument that attains the
 * minimum (clamped into [lo,hi])
 */
static double
tcb_minfun_w(double A, double B, double C, double R0, double DR, double lo,
  double hi, double *argt)
{
  double cand[6];
  int nc = 0;
  cand[nc++] = lo;
  cand[nc++] = hi;
  double a2 = A * (A - DR * DR);
  double a1 = B * (A - DR * DR);
  double a0 = 0.25 * B * B - C * DR * DR;
  if (fabs(a2) > 1e-18)
  {
    double disc = a1 * a1 - 4.0 * a2 * a0;
    if (disc >= 0.0)
    {
      double sd = sqrt(disc);
      cand[nc++] = (-a1 + sd) / (2.0 * a2);
      cand[nc++] = (-a1 - sd) / (2.0 * a2);
    }
  }
  else if (fabs(a1) > 1e-18)
    cand[nc++] = -a0 / a1;
  double best = DBL_MAX, bt = lo;
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < lo) t = lo;
    if (t > hi) t = hi;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double f = sqrt(q) - (R0 + DR * t);
    if (f < best) { best = f; bt = t; }
  }
  *argt = bt;
  return best;
}

/**
 * @brief Minimum of [ dist(c(t), edge) - r(t) ] for t in [0,1] together
 * with the argument t that attains it (mirrors #tcb_seg_edge_mindist)
 */
static double
tcb_seg_edge_dt(double cx1, double cy1, double cx2, double cy2, double r1,
  double r2, const TcbSeg *e, double *out_t)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay;
  const double l2 = ux * ux + uy * uy;
  if (l2 <= 1e-24)
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    return tcb_minfun_w(A, B, C, r1, dr, 0.0, 1.0, out_t);
  }
  const double s0 = (cx1 - ax) * ux + (cy1 - ay) * uy;
  const double s1 = dcx * ux + dcy * uy;
  double bp[4];
  int nb = 0;
  bp[nb++] = 0.0;
  bp[nb++] = 1.0;
  if (fabs(s1) > 1e-18)
  {
    double ta = -s0 / s1, tb = (l2 - s0) / s1;
    if (ta > 0.0 && ta < 1.0) bp[nb++] = ta;
    if (tb > 0.0 && tb < 1.0) bp[nb++] = tb;
  }
  for (int i = 0; i < nb; i++)
    for (int j = i + 1; j < nb; j++)
      if (bp[j] < bp[i]) { double tmp = bp[i]; bp[i] = bp[j]; bp[j] = tmp; }
  double best = DBL_MAX, bt = 0.0;
  for (int k = 0; k + 1 < nb; k++)
  {
    double lo = bp[k], hi = bp[k + 1];
    if (hi - lo < 1e-15) continue;
    double mt = 0.5 * (lo + hi);
    double s = (s0 + s1 * mt) / l2;
    double A, B, C;
    if (s <= 0.0)
    {
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
      C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    }
    else if (s >= 1.0)
    {
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - bx) * dcx + (cy1 - by) * dcy);
      C = (cx1 - bx) * (cx1 - bx) + (cy1 - by) * (cy1 - by);
    }
    else
    {
      double k0 = (cx1 - ax) * uy - (cy1 - ay) * ux;
      double k1 = dcx * uy - dcy * ux;
      A = k1 * k1 / l2;
      B = 2.0 * k0 * k1 / l2;
      C = k0 * k0 / l2;
    }
    double rt;
    double m = tcb_minfun_w(A, B, C, r1, dr, lo, hi, &rt);
    if (m < best) { best = m; bt = rt; }
  }
  *out_t = bt;
  return best;
}

/**
 * @brief Witness of the nearest approach: the point @p (px,py) on the
 * swept-capsule boundary and @p (qx,qy) on the geometry
 */
typedef struct
{
  double d;
  double px, py, qx, qy;
  bool set;
} TcbWitness;

/**
 * @brief Closest point on edge @p e to (px,py)
 */
static void
tcb_closest_on_edge(double px, double py, const TcbSeg *e, double *qx,
  double *qy)
{
  double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
  double l2 = ux * ux + uy * uy;
  if (l2 <= 1e-24) { *qx = e->x1; *qy = e->y1; return; }
  double s = ((px - e->x1) * ux + (py - e->y1) * uy) / l2;
  if (s < 0.0) s = 0.0;
  if (s > 1.0) s = 1.0;
  *qx = e->x1 + s * ux;
  *qy = e->y1 + s * uy;
}

/**
 * @brief Update the witness with one swept-capsule unit
 */
static void
tcb_sl_unit(double cx1, double cy1, double r1, double cx2, double cy2,
  double r2, const TcbSeg *segs, int n, bool has_poly, TcbWitness *w)
{
  if (w->set && w->d <= 0.0)
    return;
  /* A centre inside a polygon means the swept region overlaps it: the
   * shortest line is a zero-length line at that interior point (mirrors
   * the nearest-approach interior short-circuit) */
  if (has_poly)
  {
    bool in1 = tcb_pt_in_polys(cx1, cy1, segs, n);
    bool in2 = (cx2 == cx1 && cy2 == cy1) ? in1 :
      tcb_pt_in_polys(cx2, cy2, segs, n);
    if (in1 || in2)
    {
      double ix = in1 ? cx1 : cx2, iy = in1 ? cy1 : cy2;
      w->d = 0.0; w->px = ix; w->py = iy; w->qx = ix; w->qy = iy;
      w->set = true;
      return;
    }
  }
  double sxmin = fmin(cx1 - r1, cx2 - r2);
  double sxmax = fmax(cx1 + r1, cx2 + r2);
  double symin = fmin(cy1 - r1, cy2 - r2);
  double symax = fmax(cy1 + r1, cy2 + r2);
  for (int k = 0; k < n; k++)
  {
    const TcbSeg *e = &segs[k];
    if (w->set)
    {
      double dx = fmax(fmax(e->xmin - sxmax, sxmin - e->xmax), 0.0);
      double dy = fmax(fmax(e->ymin - symax, symin - e->ymax), 0.0);
      if (dx * dx + dy * dy >= w->d * w->d)
        continue;
    }
    double t;
    double m = tcb_seg_edge_dt(cx1, cy1, cx2, cy2, r1, r2, e, &t);
    if (! w->set || m < w->d)
    {
      double ccx = cx1 + (cx2 - cx1) * t;
      double ccy = cy1 + (cy2 - cy1) * t;
      double rr = r1 + (r2 - r1) * t;
      double qx, qy;
      tcb_closest_on_edge(ccx, ccy, e, &qx, &qy);
      double vx = qx - ccx, vy = qy - ccy;
      double vl = sqrt(vx * vx + vy * vy);
      double pxp, pyp;
      if (vl <= 1e-12 || m <= 0.0)
      {
        /* Overlap or centre on the edge: degenerate line at the contact */
        pxp = qx; pyp = qy;
      }
      else
      {
        pxp = ccx + vx * (rr / vl);
        pyp = ccy + vy * (rr / vl);
      }
      w->d = m; w->px = pxp; w->py = pyp; w->qx = qx; w->qy = qy;
      w->set = true;
    }
  }
}

/**
 * @brief Update the witness with one temporal circular buffer sequence
 */
static void
tcb_sl_seq(const TSequence *seq, const TcbSeg *segs, int n, bool has_poly,
  TcbWitness *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
      tcb_sl_unit(p->x, p->y, c->radius, p->x, p->y, c->radius, segs, n,
        has_poly, w);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && ! (w->set && w->d <= 0.0); i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *c1 = DatumGetCbufferP(tinstant_value_p(i1));
    const Cbuffer *c2 = DatumGetCbufferP(tinstant_value_p(i2));
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(c1));
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(c2));
    tcb_sl_unit(p1->x, p1->y, c1->radius, p2->x, p2->y, c2->radius, segs,
      n, has_poly, w);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free shortest line between a temporal circular buffer and a
 * geometry. Returns NULL when the analytic path does not apply (curved or
 * unsupported geometry, or a polygon-contained centre), so the caller can
 * fall back to the exact traversed-area shortest line.
 */
static GSERIALIZED *
tcbuffer_geo_shortestline_analytic(const Temporal *temp,
  const GSERIALIZED *gs)
{
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  TcbSeg *segs = NULL;
  int cap = 0, n = 0;
  bool has_poly = false;
  bool ok = tcb_geo_segs(lw, &segs, &cap, &n, &has_poly);
  lwgeom_free(lw);
  if (! ok || n == 0)
  {
    if (segs) pfree(segs);
    return NULL;
  }

  TcbWitness w;
  w.d = DBL_MAX; w.set = false;
  w.px = w.py = w.qx = w.qy = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p((TInstant *) temp));
    const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
    tcb_sl_unit(p->x, p->y, c->radius, p->x, p->y, c->radius, segs, n,
      has_poly, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    tcb_sl_seq((TSequence *) temp, segs, n, has_poly, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      tcb_sl_seq(TSEQUENCESET_SEQ_N(ss, i), segs, n, has_poly, &w);
  }
  pfree(segs);
  if (! w.set)
    return NULL;

  int32_t srid = gserialized_get_srid(gs);
  POINTARRAY *pa = ptarray_construct(0, 0, 2);
  POINT4D p4;
  p4.z = 0.0; p4.m = 0.0;
  p4.x = w.px; p4.y = w.py;
  ptarray_set_point4d(pa, 0, &p4);
  p4.x = w.qx; p4.y = w.qy;
  ptarray_set_point4d(pa, 1, &p4);
  LWLINE *ln = lwline_construct(srid, NULL, pa);
  GSERIALIZED *line = geo_serialize(lwline_as_lwgeom(ln));
  lwline_free(ln);
  return line;
}

/**
 * @brief A spatially-local group of consecutive segments with its bounding box
 */
typedef struct
{
  int start, n;                  /**< Range [start, start+n) in the segments */
  double xmin, ymin, xmax, ymax; /**< Bounding box of the bucket */
} TcbBucket;

/**
 * @brief Geometry context for the nearest-approach kernel: the boundary
 * segments (Morton-ordered), a bucket bounding-volume hierarchy over them, the
 * overall bounding box, and whether any polygon ring is present
 */
typedef struct
{
  const TcbSeg *segs;
  int n;
  bool has_poly;
  double xmin, ymin, xmax, ymax; /**< Overall geometry bounding box */
  const TcbBucket *bks;
  int nbk;
} TcbGeom;

/** @brief A segment paired with its Morton (Z-order) key, for sorting */
typedef struct
{
  uint32_t key;
  TcbSeg seg;
} TcbSortItem;

/** @brief Spread the low 16 bits of @p v with one zero bit between each */
static uint32_t
tcb_morton_part(uint32_t v)
{
  v &= 0x0000ffff;
  v = (v | (v << 8)) & 0x00ff00ff;
  v = (v | (v << 4)) & 0x0f0f0f0f;
  v = (v | (v << 2)) & 0x33333333;
  v = (v | (v << 1)) & 0x55555555;
  return v;
}

/** @brief Order TcbSortItem by Morton key */
static int
tcb_morton_cmp(const void *a, const void *b)
{
  uint32_t ka = ((const TcbSortItem *) a)->key;
  uint32_t kb = ((const TcbSortItem *) b)->key;
  return (ka > kb) - (ka < kb);
}

/**
 * @brief Reorder the segments along a Morton (Z-order) curve and group them
 * into ~sqrt(n) spatially-local buckets, each with its bounding box. The
 * buckets let a swept-capsule unit skip whole groups of edges that are farther
 * than the running minimum, turning the per-unit edge scan from O(edges) into
 * roughly O(sqrt(edges) + matches) — the geometry's overall bounding box is too
 * coarse for large coastal polygons, but the bucket boxes are tight.
 */
static TcbBucket *
tcb_build_buckets(TcbSeg *segs, int n, double gxmin, double gymin,
  double gxmax, double gymax, int *nbk_out)
{
  double sx = (gxmax > gxmin) ? 65535.0 / (gxmax - gxmin) : 0.0;
  double sy = (gymax > gymin) ? 65535.0 / (gymax - gymin) : 0.0;
  TcbSortItem *items = palloc(sizeof(TcbSortItem) * n);
  for (int i = 0; i < n; i++)
  {
    double cx = 0.5 * (segs[i].xmin + segs[i].xmax);
    double cy = 0.5 * (segs[i].ymin + segs[i].ymax);
    uint32_t ix = (uint32_t) ((cx - gxmin) * sx);
    uint32_t iy = (uint32_t) ((cy - gymin) * sy);
    items[i].key = tcb_morton_part(ix) | (tcb_morton_part(iy) << 1);
    items[i].seg = segs[i];
  }
  qsort(items, n, sizeof(TcbSortItem), tcb_morton_cmp);
  for (int i = 0; i < n; i++)
    segs[i] = items[i].seg;
  pfree(items);

  int bsize = (int) ceil(sqrt((double) n));
  if (bsize < 1) bsize = 1;
  int nbk = (n + bsize - 1) / bsize;
  TcbBucket *bks = palloc(sizeof(TcbBucket) * nbk);
  for (int b = 0; b < nbk; b++)
  {
    int s = b * bsize, e = s + bsize;
    if (e > n) e = n;
    double xmn = DBL_MAX, ymn = DBL_MAX, xmx = -DBL_MAX, ymx = -DBL_MAX;
    for (int k = s; k < e; k++)
    {
      if (segs[k].xmin < xmn) xmn = segs[k].xmin;
      if (segs[k].ymin < ymn) ymn = segs[k].ymin;
      if (segs[k].xmax > xmx) xmx = segs[k].xmax;
      if (segs[k].ymax > ymx) ymx = segs[k].ymax;
    }
    bks[b].start = s; bks[b].n = e - s;
    bks[b].xmin = xmn; bks[b].ymin = ymn; bks[b].xmax = xmx; bks[b].ymax = ymx;
  }
  *nbk_out = nbk;
  return bks;
}

/**
 * @brief Ray-casting interior test over the bucket hierarchy: a bucket can hold
 * an edge crossing the rightward horizontal ray from (x,y) only if y is inside
 * its y-range and its xmax reaches x, so far buckets are skipped wholesale
 */
static bool
tcb_pt_in_polys_g(double x, double y, const TcbGeom *g)
{
  bool inside = false;
  for (int b = 0; b < g->nbk; b++)
  {
    const TcbBucket *bk = &g->bks[b];
    if (y < bk->ymin || y > bk->ymax || bk->xmax < x)
      continue;
    int e = bk->start + bk->n;
    for (int i = bk->start; i < e; i++)
    {
      const TcbSeg *s = &g->segs[i];
      if (! s->is_poly)
        continue;
      double y1 = s->y1, y2 = s->y2;
      if ((y1 > y) != (y2 > y))
      {
        double xint = s->x1 + (y - y1) / (y2 - y1) * (s->x2 - s->x1);
        if (x < xint)
          inside = ! inside;
      }
    }
  }
  return inside;
}

/**
 * @brief Update the running minimum with one swept-capsule unit (the centre
 * moves from c1 to c2 with radius r1 to r2; a stationary disk has c1 == c2)
 */
static void
tcb_unit_nad(double cx1, double cy1, double r1, double cx2, double cy2,
  double r2, const TcbGeom *g, double *best)
{
  double sxmin = fmin(cx1 - r1, cx2 - r2);
  double sxmax = fmax(cx1 + r1, cx2 + r2);
  double symin = fmin(cy1 - r1, cy2 - r2);
  double symax = fmax(cy1 + r1, cy2 + r2);
  /* Coarse branch-and-bound prune: the distance between this unit's swept
   * bounding box and the geometry's overall bounding box is a lower bound on
   * the unit's nearest approach (a box contains its geometry, so the box-to-box
   * distance cannot exceed the geometry-to-capsule distance). If that lower
   * bound already reaches the running minimum, no edge can improve it, so the
   * point-in-polygon test and the whole per-edge loop are skipped. A segment
   * whose centre is inside a polygon overlaps the geometry bounding box, giving
   * a zero lower bound, so it is never wrongly pruned. */
  if (*best != DBL_MAX)
  {
    double dgx = fmax(fmax(g->xmin - sxmax, sxmin - g->xmax), 0.0);
    double dgy = fmax(fmax(g->ymin - symax, symin - g->ymax), 0.0);
    if (dgx * dgx + dgy * dgy >= (*best) * (*best))
      return;
  }
  if (g->has_poly &&
      (tcb_pt_in_polys_g(cx1, cy1, g) || tcb_pt_in_polys_g(cx2, cy2, g)))
  {
    *best = 0.0;
    return;
  }
  /* Bucket bounding-volume hierarchy: skip whole buckets of edges that are
   * farther than the running minimum, then the per-edge prune within a bucket */
  for (int b = 0; b < g->nbk && *best > 0.0; b++)
  {
    const TcbBucket *bk = &g->bks[b];
    if (*best != DBL_MAX)
    {
      double dx = fmax(fmax(bk->xmin - sxmax, sxmin - bk->xmax), 0.0);
      double dy = fmax(fmax(bk->ymin - symax, symin - bk->ymax), 0.0);
      if (dx * dx + dy * dy >= (*best) * (*best))
        continue;
    }
    int e = bk->start + bk->n;
    for (int k = bk->start; k < e && *best > 0.0; k++)
    {
      const TcbSeg *ed = &g->segs[k];
      if (*best != DBL_MAX)
      {
        double dx = fmax(fmax(ed->xmin - sxmax, sxmin - ed->xmax), 0.0);
        double dy = fmax(fmax(ed->ymin - symax, symin - ed->ymax), 0.0);
        if (dx * dx + dy * dy >= (*best) * (*best))
          continue;
      }
      double m = tcb_seg_edge_mindist(cx1, cy1, cx2, cy2, r1, r2, ed);
      if (m < *best) *best = m;
    }
  }
}

/**
 * @brief Update the running minimum with one temporal circular buffer
 * sequence (linear interpolation walks consecutive segments; discrete or
 * step interpolation treats each instant as a stationary disk)
 */
static void
tcb_seq_nad(const TSequence *seq, const TcbGeom *g, double *best)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && *best > 0.0; i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
      tcb_unit_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, g, best);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && *best > 0.0; i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *c1 = DatumGetCbufferP(tinstant_value_p(i1));
    const Cbuffer *c2 = DatumGetCbufferP(tinstant_value_p(i2));
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(c1));
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(c2));
    tcb_unit_nad(p1->x, p1->y, c1->radius, p2->x, p2->y, c2->radius, g, best);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free nearest approach distance between a temporal circular
 * buffer and a geometry
 */
static double
tcbuffer_geo_nad_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  TcbSeg *segs = NULL;
  int cap = 0, n = 0;
  bool has_poly = false;
  bool ok = tcb_geo_segs(lw, &segs, &cap, &n, &has_poly);
  lwgeom_free(lw);

  /* Curved / unsupported geometry, or no segments: fall back to the exact
   * traversed-area distance so the result is never wrong */
  if (! ok || n == 0)
  {
    if (segs) pfree(segs);
    GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
    double result = geom_distance2d(trav, gs);
    pfree(trav);
    return result;
  }

  /* Overall bounding box of the geometry (union of the edge bounding boxes) */
  double gxmin = DBL_MAX, gymin = DBL_MAX, gxmax = -DBL_MAX, gymax = -DBL_MAX;
  for (int k = 0; k < n; k++)
  {
    if (segs[k].xmin < gxmin) gxmin = segs[k].xmin;
    if (segs[k].ymin < gymin) gymin = segs[k].ymin;
    if (segs[k].xmax > gxmax) gxmax = segs[k].xmax;
    if (segs[k].ymax > gymax) gymax = segs[k].ymax;
  }

  /* Build the bucket bounding-volume hierarchy over the segments (reorders
   * segs along a Morton curve) and assemble the geometry context */
  int nbk = 0;
  TcbBucket *bks = tcb_build_buckets(segs, n, gxmin, gymin, gxmax, gymax, &nbk);
  TcbGeom g = { segs, n, has_poly, gxmin, gymin, gxmax, gymax, bks, nbk };

  double best = DBL_MAX;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p((TInstant *) temp));
    const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
    tcb_unit_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, &g, &best);
  }
  else if (temp->subtype == TSEQUENCE)
    tcb_seq_nad((TSequence *) temp, &g, &best);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && best > 0.0; i++)
      tcb_seq_nad(TSEQUENCESET_SEQ_N(ss, i), &g, &best);
  }

  pfree(bks);
  pfree(segs);
  return best < 0.0 ? 0.0 : best;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_base_dist
 * @brief Return the nearest approach distance between a circular buffer
 * and a spatiotemporal box
 * @param[in] cb Circular buffer
 * @param[in] box Spatiotemporal box
 * @csqlfn #NAD_cbuffer_stbox()
 */
double
nad_cbuffer_stbox(const Cbuffer *cb, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_cbuffer_stbox(cb, box))
    return DBL_MAX;

  Datum geocbuf = PointerGetDatum(cbuffer_to_geom(cb));
  Datum geobox = PointerGetDatum(stbox_geo(box));
  double result = DatumGetFloat8(datum_geom_distance2d(geocbuf, geobox));
  pfree(DatumGetPointer(geocbuf)); pfree(DatumGetPointer(geobox)); 
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #NAD_tcbuffer_geo()
 */
double
nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return DBL_MAX;

  return tcbuffer_geo_nad_analytic(temp, gs);
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a spatiotemporal box
 * @param[in] temp Temporal circular buffer
 * @param[in] box Spatiotemporal box
 * @csqlfn #NAD_tcbuffer_geo()
 */
double
nad_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_stbox(temp, box))
    return DBL_MAX;

  GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
  GSERIALIZED *geo = stbox_geo(box);
  double result = geom_distance2d(trav, geo);
  pfree(trav); pfree(geo);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #NAD_tcbuffer_cbuffer()
 */
double
nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return DBL_MAX;

  GSERIALIZED *geom = cbuffer_to_geom(cb);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
  double result = geom_distance2d(trav, geom);
  pfree(trav); pfree(geom);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach distance of two temporal circular buffers
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #NAD_tcbuffer_tcbuffer()
 */
double
nad_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return DBL_MAX;

  Temporal *dist = tdistance_tcbuffer_tcbuffer(temp1, temp2);
  if (dist == NULL)
    return DBL_MAX;
  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/*****************************************************************************
 * Threshold-aware plane-sweep spatial-min kernel for tcbuffer
 *
 * Computes the exact minimum spatial distance between two temporal circular
 * buffers without materialising either traversed area.  Walks segment pairs
 * directly: for each pair of cbuffer-segments (A,r_A)->(B,r_B) and
 * (C,r_C)->(D,r_D), the closed-form min over (s,t) in [0,1]^2 of
 *   f(s,t) = |c1(s) - c2(t)| - r1(s) - r2(t)
 * is the global min of nine candidates: 4 corners (s,t in {0,1}^2), 4 edge
 * critical points (one per edge, root of a 1D quadratic), and 1 interior
 * critical point (root of a quadratic in D=sqrt(h)).  Each candidate is O(1)
 * arithmetic.  The plane-sweep sorts T2's expanded (radius-aware) segment
 * boxes by minx once per pair and tightens the running threshold as it
 * walks.  Returns the same value as
 * `ST_Distance(traversedArea(temp1), traversedArea(temp2))` to liblwgeom
 * numerical tolerance.
 *****************************************************************************/

typedef struct
{
  int idx;
  float minx;
  float maxx;
  float miny;
  float maxy;
} CbufSegBox;

static int
cbufsegbox_cmp_minx(const void *a, const void *b)
{
  float da = ((const CbufSegBox *) a)->minx;
  float db = ((const CbufSegBox *) b)->minx;
  return (da < db) ? -1 : (da > db ? 1 : 0);
}

/* Append candidate value of f(s,t) on edge s=0 (in t) given:
 *   e = |E|^2 with E = A - C
 *   q = E . V
 *   v = |V|^2
 *   dr2 = r_D - r_C
 *   r_sum = r_A + r_C  (radius offset at t=0)
 * Computes the critical point of g(t) = sqrt(e - 2tq + t^2 v) - r_sum - t dr2
 * via the quadratic v(v - dr2^2) t^2 - 2 q (v - dr2^2) t + (q^2 - dr2^2 e) = 0.
 * Only v > dr2^2 yields a local minimum; the v == dr2^2 inflection case is
 * skipped (covered by corners). */
static void
cbufsegm_edge_crit_in_t(double e, double q, double v, double dr2, double r_sum,
  double *best)
{
  double dr2sq = dr2 * dr2;
  if (v <= dr2sq)
    return;
  double diff = v - dr2sq;
  double aa = v * diff;
  double bb = -2.0 * q * diff;
  double cc = q * q - dr2sq * e;
  double disc = bb * bb - 4.0 * aa * cc;
  if (disc < 0.0)
    return;
  double sq = sqrt(disc);
  for (int sign = -1; sign <= 1; sign += 2)
  {
    double t = (-bb + sign * sq) / (2.0 * aa);
    if (t <= 0.0 || t >= 1.0)
      continue;
    double h = e - 2.0 * t * q + t * t * v;
    /* h is the squared centerline distance at t.  h <= 0 means the centers
     * coincide there, so the swept discs overlap and f is negative; do not
     * skip it (skipping misses the true minimum), and skip the sign-
     * consistency check which only qualifies a strictly-positive root. */
    if (h > 0.0 && dr2 * (v * t - q) < 0.0)
      continue;
    double f = sqrt(h > 0.0 ? h : 0.0) - r_sum - t * dr2;
    if (f < *best)
      *best = f;
  }
}

/* Append candidate value of f(s,t) on edge t=0 (in s) given:
 *   e = |E|^2 with E = A - C
 *   p = E . U
 *   u = |U|^2
 *   dr1 = r_B - r_A
 *   r_sum = r_A + r_C
 * Critical point of g(s) = sqrt(e + 2sp + s^2 u) - r_sum - s dr1, derivative
 * (us + p)/sqrt(...) - dr1 = 0; analogous quadratic. */
static void
cbufsegm_edge_crit_in_s(double e, double p, double u, double dr1, double r_sum,
  double *best)
{
  double dr1sq = dr1 * dr1;
  if (u <= dr1sq)
    return;
  double diff = u - dr1sq;
  double aa = u * diff;
  double bb = 2.0 * p * diff;
  double cc = p * p - dr1sq * e;
  double disc = bb * bb - 4.0 * aa * cc;
  if (disc < 0.0)
    return;
  double sq = sqrt(disc);
  for (int sign = -1; sign <= 1; sign += 2)
  {
    double s = (-bb + sign * sq) / (2.0 * aa);
    if (s <= 0.0 || s >= 1.0)
      continue;
    double h = e + 2.0 * s * p + s * s * u;
    /* h <= 0 means centers coincide at s, swept discs overlap, f negative;
     * keep it (skipping misses the minimum) and bypass the sign check. */
    if (h > 0.0 && dr1 * (u * s + p) < 0.0)
      continue;
    double f = sqrt(h > 0.0 ? h : 0.0) - r_sum - s * dr1;
    if (f < *best)
      *best = f;
  }
}

/* Exact min spatial distance between two cbuffer segments,
 *   c1(s) = A + s(B-A), r1(s) = r_A + s(r_B - r_A), s in [0,1]
 *   c2(t) = C + t(D-C), r2(t) = r_C + t(r_D - r_C), t in [0,1]
 * Returns max(0, min over [0,1]^2 of |c1(s) - c2(t)| - r1(s) - r2(t)),
 * capped above at @p best_so_far (caller's running threshold).  The
 * function never raises @p best_so_far above its input value.
 */
static double
cbuffersegm_segm_mindist(const POINT2D *A, double rA, const POINT2D *B,
  double rB, const POINT2D *C, double rC, const POINT2D *D, double rD,
  double best_so_far)
{
  double Ux = B->x - A->x, Uy = B->y - A->y;
  double Vx = D->x - C->x, Vy = D->y - C->y;
  double Ex = A->x - C->x, Ey = A->y - C->y;
  double u = Ux * Ux + Uy * Uy;
  double v = Vx * Vx + Vy * Vy;
  double w = Ux * Vx + Uy * Vy;
  double e = Ex * Ex + Ey * Ey;
  double p = Ex * Ux + Ey * Uy;
  double q = Ex * Vx + Ey * Vy;
  double dr1 = rB - rA;
  double dr2 = rD - rC;
  double r0 = rA + rC;
  double best = best_so_far;

  /* 4 corners.  |B-C|^2 = e + u + 2p; |A-D|^2 = e - 2q + v;
   *             |B-D|^2 = e + u + v + 2p - 2q - 2w. */
  double d;
  d = sqrt(e) - rA - rC;
  if (d < best) best = d;
  d = sqrt(e + u + 2.0 * p) - rB - rC;
  if (d < best) best = d;
  {
    double h01 = e - 2.0 * q + v;
    if (h01 > 0.0) { d = sqrt(h01) - rA - rD; if (d < best) best = d; }
    else           { d = -rA - rD;            if (d < best) best = d; }
  }
  {
    double h11 = e + u + v + 2.0 * p - 2.0 * q - 2.0 * w;
    if (h11 > 0.0) { d = sqrt(h11) - rB - rD; if (d < best) best = d; }
    else           { d = -rB - rD;            if (d < best) best = d; }
  }
  if (best <= 0.0)
    return 0.0;

  /* 4 edges. */
  if (v > 0.0)
  {
    /* Edge s=0: f(0,t) = sqrt(e - 2tq + t^2 v) - rA - rC - t dr2 */
    cbufsegm_edge_crit_in_t(e, q, v, dr2, rA + rC, &best);
    /* Edge s=1: substitute (E + U) for E.  |E+U|^2 = e + u + 2p,
     * (E+U).V = q + w, radius at s=1 is rB. */
    cbufsegm_edge_crit_in_t(e + u + 2.0 * p, q + w, v, dr2, rB + rC, &best);
  }
  if (u > 0.0)
  {
    /* Edge t=0: f(s,0) = sqrt(e + 2sp + s^2 u) - rA - rC - s dr1 */
    cbufsegm_edge_crit_in_s(e, p, u, dr1, rA + rC, &best);
    /* Edge t=1: substitute (E - V) for E.  |E-V|^2 = e + v - 2q,
     * (E-V).U = p - w, radius at t=1 is rD. */
    cbufsegm_edge_crit_in_s(e + v - 2.0 * q, p - w, u, dr1, rA + rD, &best);
  }
  if (best <= 0.0)
    return 0.0;

  /* Interior critical point: solve
   *   [u  -w] [s]   [dr1 D - p]                 D = sqrt(h(s,t))
   *   [-w  v] [t] = [dr2 D + q]
   * giving s(D) = alpha + beta D, t(D) = gamma + delta D.  Substitute back
   * into h(s,t) = D^2 to get a quadratic A D^2 + B D + C0 = 0.  Skip when
   * the linear system is rank-deficient (parallel segments, det = 0);
   * boundary candidates already cover that case. */
  double det = u * v - w * w;
  if (det > 0.0)
  {
    double alpha = (-v * p + w * q) / det;
    double beta  = (v * dr1 + w * dr2) / det;
    double gamma = (u * q - w * p) / det;
    double delta = (u * dr2 + w * dr1) / det;

    double Acoef = u * beta * beta + v * delta * delta
                 - 2.0 * w * beta * delta - 1.0;
    double Bcoef = 2.0 * (u * alpha * beta + v * gamma * delta
                        + beta * p - delta * q
                        - w * (alpha * delta + beta * gamma));
    double Ccoef = e + u * alpha * alpha + v * gamma * gamma
                 + 2.0 * alpha * p - 2.0 * gamma * q
                 - 2.0 * w * alpha * gamma;

    double Ds[2];
    int nD = 0;
    if (fabs(Acoef) < 1e-18)
    {
      if (fabs(Bcoef) > 1e-18)
        Ds[nD++] = -Ccoef / Bcoef;
    }
    else
    {
      double disc = Bcoef * Bcoef - 4.0 * Acoef * Ccoef;
      if (disc >= 0.0)
      {
        double sq = sqrt(disc);
        Ds[nD++] = (-Bcoef + sq) / (2.0 * Acoef);
        Ds[nD++] = (-Bcoef - sq) / (2.0 * Acoef);
      }
    }
    for (int k = 0; k < nD; k++)
    {
      double Dval = Ds[k];
      if (Dval < 0.0)
        continue;
      double s = alpha + beta * Dval;
      double t = gamma + delta * Dval;
      if (s <= 0.0 || s >= 1.0 || t <= 0.0 || t >= 1.0)
        continue;
      double f = Dval - r0 - s * dr1 - t * dr2;
      if (f < best)
        best = f;
    }
  }

  return best > 0.0 ? best : 0.0;
}

/* Plane-sweep over two cbuffer sequences. */
static double
mindist_tcbufferseq_tcbufferseq_threshold(const TSequence *seq1,
  const TSequence *seq2, double threshold)
{
  double best = threshold;

  /* Build expanded (radius-aware) segment boxes for seq2 once, then plane-
   * sweep seq1's segments against them.  Single-instant subsequences degrade
   * to single points carrying the instant's radius, handled via the same
   * kernel with degenerate segment (B == A, rB == rA). */
  int n2_segs = seq2->count > 1 ? seq2->count - 1 : 1;
  CbufSegBox *boxes2 = palloc(n2_segs * sizeof(CbufSegBox));
  for (int j = 0; j < n2_segs; j++)
  {
    const Cbuffer *cb_a = DatumGetCbufferP(
      tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
    const Cbuffer *cb_b = (seq2->count > 1) ?
      DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1))) :
      cb_a;
    const POINT2D *pa = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_a));
    const POINT2D *pb = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_b));
    double r_max = fmax(cb_a->radius, cb_b->radius);
    boxes2[j].idx = j;
    boxes2[j].minx = (float) (fmin(pa->x, pb->x) - r_max);
    boxes2[j].maxx = (float) (fmax(pa->x, pb->x) + r_max);
    boxes2[j].miny = (float) (fmin(pa->y, pb->y) - r_max);
    boxes2[j].maxy = (float) (fmax(pa->y, pb->y) + r_max);
  }
  qsort(boxes2, n2_segs, sizeof(CbufSegBox), cbufsegbox_cmp_minx);

  int n1_segs = seq1->count > 1 ? seq1->count - 1 : 1;
  for (int i = 0; i < n1_segs; i++)
  {
    const Cbuffer *cb_a1 = DatumGetCbufferP(
      tinstant_value_p(TSEQUENCE_INST_N(seq1, i)));
    const Cbuffer *cb_b1 = (seq1->count > 1) ?
      DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq1, i + 1))) :
      cb_a1;
    const POINT2D *pa1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_a1));
    const POINT2D *pb1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_b1));
    double r_max1 = fmax(cb_a1->radius, cb_b1->radius);
    double s1_minx = fmin(pa1->x, pb1->x) - r_max1;
    double s1_maxx = fmax(pa1->x, pb1->x) + r_max1;
    double s1_miny = fmin(pa1->y, pb1->y) - r_max1;
    double s1_maxy = fmax(pa1->y, pb1->y) + r_max1;
    double thresh = best;
    double hi_x = s1_maxx + thresh;
    double lo_x = s1_minx - thresh;
    int hi_idx;
    {
      int lo = 0, hi = n2_segs;
      while (lo < hi)
      {
        int mid = (lo + hi) / 2;
        if ((double) boxes2[mid].minx > hi_x) hi = mid;
        else lo = mid + 1;
      }
      hi_idx = lo;
    }
    for (int k = 0; k < hi_idx; k++)
    {
      if ((double) boxes2[k].maxx < lo_x)
        continue;
      if ((double) boxes2[k].maxy < s1_miny - best)
        continue;
      if ((double) boxes2[k].miny > s1_maxy + best)
        continue;
      int j = boxes2[k].idx;
      const Cbuffer *cb_a2 = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
      const Cbuffer *cb_b2 = (seq2->count > 1) ?
        DatumGetCbufferP(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1))) :
        cb_a2;
      const POINT2D *pa2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_a2));
      const POINT2D *pb2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb_b2));
      double d = cbuffersegm_segm_mindist(pa1, cb_a1->radius, pb1, cb_b1->radius,
        pa2, cb_a2->radius, pb2, cb_b2->radius, best);
      if (d < best) best = d;
      if (best == 0.0) { pfree(boxes2); return 0.0; }
    }
  }
  pfree(boxes2);
  return best;
}

/* Subtype dispatch. */
static double
mindist_tcbuffer_tcbuffer_threshold(const Temporal *temp1,
  const Temporal *temp2, double threshold)
{
  if (temp1->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
    for (int i = 0; i < ss1->count; i++)
    {
      double d = mindist_tcbuffer_tcbuffer_threshold(
        (const Temporal *) TSEQUENCESET_SEQ_N(ss1, i), temp2, threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp2->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
    for (int i = 0; i < ss2->count; i++)
    {
      double d = mindist_tcbuffer_tcbuffer_threshold(temp1,
        (const Temporal *) TSEQUENCESET_SEQ_N(ss2, i), threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp1->subtype == TINSTANT)
  {
    TInstant *one[1] = { (TInstant *) temp1 };
    TSequence *singleton = tsequence_make(one, 1, true, true, LINEAR,
      NORMALIZE_NO);
    double d;
    if (temp2->subtype == TINSTANT)
    {
      TInstant *two[1] = { (TInstant *) temp2 };
      TSequence *single2 = tsequence_make(two, 1, true, true, LINEAR,
        NORMALIZE_NO);
      d = mindist_tcbufferseq_tcbufferseq_threshold(singleton, single2,
        threshold);
      pfree(single2);
    }
    else
    {
      d = mindist_tcbufferseq_tcbufferseq_threshold(singleton,
        (const TSequence *) temp2, threshold);
    }
    pfree(singleton);
    return d;
  }
  if (temp2->subtype == TINSTANT)
  {
    TInstant *one[1] = { (TInstant *) temp2 };
    TSequence *singleton = tsequence_make(one, 1, true, true, LINEAR,
      NORMALIZE_NO);
    double d = mindist_tcbufferseq_tcbufferseq_threshold(
      (const TSequence *) temp1, singleton, threshold);
    pfree(singleton);
    return d;
  }
  return mindist_tcbufferseq_tcbufferseq_threshold(
    (const TSequence *) temp1, (const TSequence *) temp2, threshold);
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the minimum spatial distance between two temporal circular
 * buffers, capped at @p threshold
 * @details Time-agnostic spatial minimum, equivalent to
 * `ST_Distance(traversedArea(temp1), traversedArea(temp2))` and to the
 * BerlinMOD Q5 semantics on the tgeompoint side.  Walks segment pairs
 * via a closed-form per-pair kernel (corners + edge critical points +
 * interior critical point of the unconstrained 2D minimisation), with
 * STBox-pair pruning on the outer pair and radius-expanded segment-bbox
 * pruning inside a plane sweep.  Neither traversed area is materialised.
 * @param[in] temp1,temp2 Temporal circular buffers
 * @param[in] threshold Running minimum from a calling aggregate; pass
 *   @c DBL_MAX for unconditional evaluation
 */
double
mindistance_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2,
  double threshold)
{
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return DBL_MAX;

  /* Outer STBox prune.  The STBox of a tcbuffer encloses every point any
   * disc visits, so the minimum spatial distance is bounded below by the
   * spatial distance between the two STBoxes.  TINSTANT subtypes carry no
   * precomputed bbox (temporal_bbox_ptr returns NULL) so the prune skips
   * them; the per-pair kernel handles the instant case directly. */
  if (! MEOS_FLAGS_GET_GEODETIC(temp1->flags) &&
      temp1->subtype != TINSTANT && temp2->subtype != TINSTANT)
  {
    const STBox *bbox1 = (const STBox *) temporal_bbox_ptr(temp1);
    const STBox *bbox2 = (const STBox *) temporal_bbox_ptr(temp2);
    double bbox_dist = stbox_spatial_distance(bbox1, bbox2);
    if (bbox_dist >= threshold)
      return threshold;
  }
  return mindist_tcbuffer_tcbuffer_threshold(temp1, temp2, threshold);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #Shortestline_tcbuffer_geo()
 */
GSERIALIZED *
shortestline_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  GSERIALIZED *result = tcbuffer_geo_shortestline_analytic(temp, gs);
  if (result)
    return result;
  /* Curved or unsupported geometry: exact traversed-area shortest line */
  GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
  result = geom_shortestline2d(trav, gs);
  pfree(trav);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the line connecting the nearest approach point between a
 * circular buffer and a temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #Shortestline_tcbuffer_cbuffer()
 */
GSERIALIZED *
shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  GSERIALIZED *geom = cbuffer_to_geom(cb);
  GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
  GSERIALIZED *result = geom_shortestline2d(trav, geom);
  pfree(geom); pfree(trav);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal circular buffers
 * @param[in] temp1,temp2 Temporal circular buffers
 * @csqlfn #Shortestline_tcbuffer_tcbuffer()
 * @note This function needs to be implemented TODO
 */
GSERIALIZED *
shortestline_tcbuffer_tcbuffer(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_tcbuffer(temp1, temp2))
    return NULL;

  Temporal *tpoint1 = tcbuffer_to_tgeompoint(temp1);
  Temporal *tpoint2 = tcbuffer_to_tgeompoint(temp2);
  GSERIALIZED *result = shortestline_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/
