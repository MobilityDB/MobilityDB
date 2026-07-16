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
  double dist_start = sqrt(dx_start * dx_start + dy_start * dy_start) - ra1 - rb;

  double dx_end = xa2 - xb;
  double dy_end = ya2 - yb;
  double dist_end = sqrt(dx_end * dx_end + dy_end * dy_end) - ra2 - rb;

  if (dist > 0.0)
  {
    /* Check if the turning point is truly internal */
    if (t_turn <= lower || t_turn >= upper)
    {
      /* No true internal turning point */
      *t1 = *t2 = (TimestampTz) 0;
      return 0;
    }
    /* Single turning point: return t1 and value1 */
    *t1 = *t2 = t_turn;
    return 1;
  }
  else
  {
    /* Crossing zero: compute entrance and exit times */
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
 * @param[in] lower,upper Timestamps defining the segments
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
  double r1 = end1->radius + end2->radius;

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

  double dist0 = sqrt(dx0*dx0 + dy0*dy0) - r0;
  double dist1 = sqrt(dx1*dx1 + dy1*dy1) - r1;

  /* Single turning point */
  if (dist_turn > 0.0) {
    TimestampTz t_turn = lower + (TimestampTz)(t_rel); 
    *t1 = *t2 = t_turn;
    return 1;
  } else { /* Crossing: compute entrance and exit times */
    double alpha_in = (dist_turn - dist0 == 0.0) ? (0.0 - dist0) : (0.0 - dist0) / (dist_turn - dist0);
    double alpha_out = (dist1 - dist_turn == 0.0) ? (0.0 - dist_turn) : (0.0 - dist_turn) / (dist1 - dist_turn);

    TimestampTz t_in = lower + (TimestampTz)(t_rel * alpha_in);
    TimestampTz t_out = lower + (TimestampTz)((t_rel + (duration - t_rel) * alpha_out));

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
      /* No true internal turning point */
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

static bool nai_tcbuffer_geo_analytic(const Temporal *temp,
  const GSERIALIZED *gs, TimestampTz *result);

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of the temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #NAI_tcbuffer_geo()
 */
TInstant *
nai_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* The instant of minimum swept-capsule distance, so the value at the instant
   * is the nearest-approach distance. We do not build the instant from a
   * closest point to avoid roundoff errors: the closest point may be at an
   * exclusive bound. */
  TimestampTz t;
  if (nai_tcbuffer_geo_analytic(temp, gs, &t))
  {
    Datum value;
    temporal_value_at_timestamptz(temp, t, false, &value);
    return tinstant_make_free(value, temp->temptype, t);
  }

  /* Curved or otherwise unsupported geometry: fall back to the centreline */
  Temporal *tpoint = tcbuffer_to_tgeompoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, gs);
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
 * @brief Ray-casting test: true if (x,y) is inside any polygon ring among
 * the segments (even-odd rule over the polygon-ring segments only)
 */

/*****************************************************************************
 * Shortest line: same closed-form minimisation, but tracking the witness
 * (the parameter on the swept-capsule segment that attains the minimum and
 * the nearest point on the geometry edge), so the connecting line can be
 * built without GEOS. The nearest-approach value path above is left
 * untouched.
 *****************************************************************************/

/**
 * @brief Return true if a stationary disc (centre @p cx,@p cy, radius @p r) is
 * within @p dist of the geometry
 * @details Byte-identical to the boolean `nad(stationary disc) <= dist`: the
 * disc is within @p dist iff its centre lies inside a polygon ring or some
 * boundary edge is within @p dist. Unlike the running-minimum kernel this is a
 * bounded existence test — the reach is the fixed threshold @p dist, so an edge
 * whose box is farther than @p dist cannot qualify and is pruned. Because the
 * per-edge distance uses the same arc/edge kernels, "min over edges <= dist" and
 * "some edge <= dist" agree, so the visit order and pruning do not affect the
 * result.
 */
static bool
tcbuffer_disc_within_dist(double cx, double cy, double r, double dist,
  const GeoDistGeom *g)
{
  if (g->has_poly && geodist_geom_point_inside(cx, cy, g))
    return true;
  double sxmin = cx - r, sxmax = cx + r, symin = cy - r, symax = cy + r;
  double dist2 = dist * dist;
  STBox query;
  stbox_set(true, false, false, 0, sxmin - dist, sxmax + dist, symin - dist,
    symax + dist, 0, 0, NULL, &query);
  int nc = rtree_search(g->rtree, RTREE_OVERLAPS, &query, geodist_pip_results);
  for (int j = 0; j < nc; j++)
  {
    const GeoDistEdge *ed =
      &g->segs[*(int *) meos_array_get(geodist_pip_results, j)];
    double edx = fmax(fmax(ed->xmin - sxmax, sxmin - ed->xmax), 0.0);
    double edy = fmax(fmax(ed->ymin - symax, symin - ed->ymax), 0.0);
    if (edx * edx + edy * edy > dist2)
      continue;
    double m = ed->is_arc ?
      geodist_segm_arc_mindist(cx, cy, cx, cy, r, r, ed) :
      geodist_segm_edge_mindist(cx, cy, cx, cy, r, r, ed);
    if (m <= dist)
      return true;
  }
  return false;
}

/**
 * @brief Update the running minimum with one temporal circular buffer
 * sequence (linear interpolation walks consecutive segments; discrete or
 * step interpolation treats each instant as a stationary disk)
 */
static void
tcbufferseq_nad(const TSequence *seq, const GeoDistGeom *g, double *best)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && *best > 0.0; i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
      geodist_segm_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, g, best);
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
    geodist_segm_nad(p1->x, p1->y, c1->radius, p2->x, p2->y, c2->radius, g, best);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free nearest approach distance between a temporal circular
 * buffer and a geometry
 */
static double
nad_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Curved / unsupported geometry, or no segments: fall back to the exact
   * traversed-area distance so the result is never wrong */
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
  {
    GSERIALIZED *trav = tcbuffer_traversed_area(temp, false);
    double result = geom_distance2d(trav, gs);
    pfree(trav);
    return result;
  }

  double best = DBL_MAX;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p((TInstant *) temp));
    const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
    geodist_segm_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, &g, &best);
  }
  else if (temp->subtype == TSEQUENCE)
    tcbufferseq_nad((TSequence *) temp, &g, &best);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && best > 0.0; i++)
      tcbufferseq_nad(TSEQUENCESET_SEQ_N(ss, i), &g, &best);
  }

  geodist_geom_free(&g);
  return best < 0.0 ? 0.0 : best;
}

/**
 * @brief Update the witness with one temporal circular buffer sequence
 */
static void
tcbufferseq_shortestline(const TSequence *seq, const GeoDistGeom *g,
  GeoDistShortLine *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
      geodist_segm_shortestline(p->x, p->y, c->radius, p->x, p->y, c->radius,
        g, w);
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
    geodist_segm_shortestline(p1->x, p1->y, c1->radius, p2->x, p2->y,
      c2->radius, g, w);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free shortest line between a temporal circular buffer and a
 * geometry, arc-exact for circular-arc input. Returns NULL when the analytic
 * path does not apply (an unsupported geometry type), so the caller can fall
 * back to the exact traversed-area shortest line.
 */
static GSERIALIZED *
shortestline_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
    return NULL;

  GeoDistShortLine w;
  w.d = DBL_MAX; w.set = false;
  w.px = w.py = w.qx = w.qy = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p((TInstant *) temp));
    const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
    geodist_segm_shortestline(p->x, p->y, c->radius, p->x, p->y, c->radius,
      &g, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    tcbufferseq_shortestline((TSequence *) temp, &g, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      tcbufferseq_shortestline(TSEQUENCESET_SEQ_N(ss, i), &g, &w);
  }
  geodist_geom_free(&g);
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

/*****************************************************************************
 * GEOS-free analytic nearest approach instant between a temporal circular
 * buffer and a geometry
 *
 * The nearest approach instant is the time attaining the minimum swept-capsule
 * distance, so it reuses the same per-segment analytic minimum as the
 * nearest-approach value: each moving-disc unit yields the parametric fraction
 * where it comes closest to the geometry, and the running witness keeps the
 * earliest timestamp attaining the overall minimum. Because it minimises the
 * same disc-to-geometry distance as #nad_tcbuffer_geo_analytic, the value at
 * the returned instant is exactly the nearest-approach distance (the centreline
 * delegation instead minimises the centre-to-geometry distance, which differs
 * when the radius varies).
 *****************************************************************************/

/**
 * @brief Update the witness with one temporal circular buffer sequence,
 * mirroring #tcbufferseq_shortestline
 */
static void
tcbufferseq_nai(const TSequence *seq, const GeoDistGeom *g, GeoDistNai *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
      const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
      geodist_segm_nai(p->x, p->y, c->radius, inst->t, p->x, p->y, c->radius,
        inst->t, g, w);
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
    geodist_segm_nai(p1->x, p1->y, c1->radius, i1->t, p2->x, p2->y, c2->radius,
      i2->t, g, w);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free nearest approach instant between a temporal circular buffer
 * and a geometry
 * @details Returns the timestamp attaining the minimum swept-capsule distance
 * in @p result. Returns false when the analytic path does not apply (an
 * unsupported geometry type) so the caller can fall back to the centreline
 * delegation.
 */
static bool
nai_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs,
  TimestampTz *result)
{
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
    return false;

  GeoDistNai w;
  w.d = DBL_MAX; w.t = 0; w.set = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (TInstant *) temp;
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
    const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(c));
    geodist_segm_nai(p->x, p->y, c->radius, inst->t, p->x, p->y, c->radius,
      inst->t, &g, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    tcbufferseq_nai((TSequence *) temp, &g, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      tcbufferseq_nai(TSEQUENCESET_SEQ_N(ss, i), &g, &w);
  }
  geodist_geom_free(&g);
  if (! w.set)
    return false;
  *result = w.t;
  return true;
}

/*****************************************************************************
 * Temporal within relationship (GEOS-free)
 *
 * The sub-periods during which a temporal circular buffer stays within a
 * distance of a non-curved geometry, from the same swept-capsule distance
 * kernel as the nearest-approach value. The candidate crossing instants are
 * the roots of dist(centre(t), edge) = radius(t) + dist per boundary edge;
 * each candidate sub-interval is classified with the exact interior-aware unit
 * distance, so the ever-projection of the result agrees with the
 * nearest-approach value.
 *****************************************************************************/

/**
 * @brief Reusable geometry context that owns the boundary segments and the
 * bucket hierarchy, so many discs and segments can be tested against one
 * geometry without reparsing it
 */
typedef struct
{
  GeoDistEdge *segs;
  GeoDistGeom g;
} TcbufferGeoCtx;

/**
 * @brief Build the reusable geometry context for the native within kernel, or
 * return NULL for curved or unsupported geometry (the caller then uses the
 * exact traversed-area path)
 */
void *
tcbuffer_geo_ctx_make(const GSERIALIZED *gs)
{
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  GeoDistEdge *segs = NULL;
  int cap = 0, n = 0;
  bool has_poly = false;
  bool ok = geodist_geom_edges(lw, true, &segs, &cap, &n, &has_poly);
  lwgeom_free(lw);
  if (! ok || n == 0)
  {
    if (segs) pfree(segs);
    return NULL;
  }
  double gxmin = DBL_MAX, gymin = DBL_MAX, gxmax = -DBL_MAX, gymax = -DBL_MAX;
  for (int k = 0; k < n; k++)
  {
    if (segs[k].xmin < gxmin) gxmin = segs[k].xmin;
    if (segs[k].ymin < gymin) gymin = segs[k].ymin;
    if (segs[k].xmax > gxmax) gxmax = segs[k].xmax;
    if (segs[k].ymax > gymax) gymax = segs[k].ymax;
  }
  TcbufferGeoCtx *ctx = palloc(sizeof(TcbufferGeoCtx));
  ctx->segs = segs;
  ctx->g = (GeoDistGeom) { segs, n, has_poly, gxmin, gymin, gxmax, gymax, NULL,
    0, geodist_geom_build_rtree(segs, n) };
  /* Scratch buffer for the R-tree candidate ids, created with the R-tree and
   * freed with it in #tcbuffer_geo_ctx_free (see geodist_pip_results). */
  geodist_pip_results = meos_array_create(sizeof(int));
  return ctx;
}

/**
 * @brief Free a geometry context built by #tcbuffer_geo_ctx_make
 */
void
tcbuffer_geo_ctx_free(void *ctx)
{
  if (! ctx)
    return;
  TcbufferGeoCtx *c = (TcbufferGeoCtx *) ctx;
  rtree_free(c->g.rtree);
  if (geodist_pip_results)
  {
    meos_array_destroy(geodist_pip_results);
    geodist_pip_results = NULL;
  }
  pfree(c->segs);
  pfree(c);
}

/**
 * @brief Return the number of boundary segments in a geometry context, used to
 * size the per-segment within-interval output
 */
int
tcbuffer_geo_ctx_nsegs(const void *ctxv)
{
  return ((const TcbufferGeoCtx *) ctxv)->g.n;
}

/**
 * @brief Return true if a static circular buffer is within @p dist of the
 * geometry, i.e. dist(centre, geometry) - radius <= dist
 */
bool
tcbuffer_disc_within_ctx(const Cbuffer *cb, double dist, const void *ctxv)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb));
  return tcbuffer_disc_within_dist(p->x, p->y, cb->radius, dist, &ctx->g);
}

/**
 * @brief Append to @p cand the normalized times in (lo,hi) at which the moving
 * disc distance to a region equals @p dist, i.e. the roots of
 * (A - DR^2) t^2 + (B - 2 R0 DR) t + (C - R0^2) = 0 with R0 = r1 + dist
 */
static void
tcbuffer_region_within_roots(double A, double B, double C, double R0,
  double DR, double lo, double hi, double *cand, int *nc)
{
  double a = A - DR * DR;
  double b = B - 2.0 * R0 * DR;
  double c = C - R0 * R0;
  if (fabs(a) < 1e-18)
  {
    if (fabs(b) > 1e-18)
    {
      double t = -c / b;
      if (t > lo && t < hi)
        cand[(*nc)++] = t;
    }
    return;
  }
  double disc = b * b - 4.0 * a * c;
  if (disc < 0.0)
    return;
  double sd = sqrt(disc);
  double t1 = (-b - sd) / (2.0 * a);
  double t2 = (-b + sd) / (2.0 * a);
  if (t1 > lo && t1 < hi)
    cand[(*nc)++] = t1;
  if (t2 > lo && t2 < hi)
    cand[(*nc)++] = t2;
}

/**
 * @brief Append the within-distance crossing times of one moving disc segment
 * against one geometry edge, mirroring the perpendicular/endpoint region split
 * of #geodist_segm_edge_mindist
 */
static void
tcbuffersegm_edge_within_roots(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e, double dist, double *cand, int *nc)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1, dr = r2 - r1, R0 = r1 + dist;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay, l2 = ux * ux + uy * uy;
  if (l2 <= 1e-24)
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    tcbuffer_region_within_roots(A, B, C, R0, dr, 0.0, 1.0, cand, nc);
    return;
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
  for (int k = 0; k + 1 < nb; k++)
  {
    double lo = bp[k], hi = bp[k + 1];
    if (hi - lo < 1e-15)
      continue;
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
    tcbuffer_region_within_roots(A, B, C, R0, dr, lo, hi, cand, nc);
  }
}

/**
 * @brief Append the within-distance crossing times of one moving disc segment
 * against one circular-arc edge, the temporal analogue of the on-span circle /
 * off-span endpoint split of #geodist_segm_arc_mindist
 * @details On the arc's angular span the distance to the disc is
 * | sqrt(Q(t)) - R | - r(t); setting it equal to @p dist gives
 * sqrt(Q) = R + r(t) + dist (disc outside the circle) or
 * sqrt(Q) = R - r(t) - dist (disc inside), each a region-crossing quadratic with
 * the arc radius folded into R0. Off the span the nearest arc point is an
 * endpoint, so the two endpoint region crossings are added as well. The result
 * is a superset of the true crossings; each candidate sub-interval is then
 * classified exactly by the arc-aware unit distance (#geodist_segm_nad), so
 * roots from the squared equation or the wrong angular regime are harmless.
 */
static void
tcbuffersegm_arc_within_roots(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e, double dist, double *cand, int *nc)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1, dr = r2 - r1;
  const double px = e->acx, py = e->acy, R = e->arad;
  const double A = dcx * dcx + dcy * dcy;
  const double B = 2.0 * ((cx1 - px) * dcx + (cy1 - py) * dcy);
  const double C = (cx1 - px) * (cx1 - px) + (cy1 - py) * (cy1 - py);
  /* On-span circle crossings: sqrt(Q) = R + r(t) + dist and R - r(t) - dist */
  tcbuffer_region_within_roots(A, B, C, R + r1 + dist, dr, 0.0, 1.0, cand, nc);
  tcbuffer_region_within_roots(A, B, C, R - r1 - dist, -dr, 0.0, 1.0, cand, nc);
  /* Off-span regions are nearest to an arc endpoint: their region crossings */
  for (int ep = 0; ep < 2; ep++)
  {
    double ex = ep == 0 ? e->x1 : e->x2;
    double ey = ep == 0 ? e->y1 : e->y2;
    double Be = 2.0 * ((cx1 - ex) * dcx + (cy1 - ey) * dcy);
    double Ce = (cx1 - ex) * (cx1 - ex) + (cy1 - ey) * (cy1 - ey);
    tcbuffer_region_within_roots(A, Be, Ce, r1 + dist, dr, 0.0, 1.0, cand, nc);
  }
}

/**
 * @brief Comparator for sorting the candidate crossing times
 */
static int
tcbuffer_double_cmp(const void *a, const void *b)
{
  double d = *(const double *) a - *(const double *) b;
  return (d < 0.0) ? -1 : (d > 0.0 ? 1 : 0);
}

/**
 * @brief Return the within-distance sub-intervals of one linear moving disc
 * segment as normalized [0,1] time ranges in @p outlo / @p outhi, returning
 * their count. The crossing candidates come from the per-edge roots and each
 * sub-interval is classified with the exact interior-aware unit distance.
 */
int
tcbufferseg_within_ctx(const Cbuffer *cb1, const Cbuffer *cb2, double dist,
  const void *ctxv, double *outlo, double *outhi, int maxout)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb1));
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb2));
  double cx1 = p1->x, cy1 = p1->y, r1 = cb1->radius;
  double cx2 = p2->x, cy2 = p2->y, r2 = cb2->radius;
  int ncap = 2 + 8 * ctx->g.n;
  double *cand = palloc(sizeof(double) * ncap);
  int nc = 0;
  cand[nc++] = 0.0;
  cand[nc++] = 1.0;
  /* A within root needs dist(centre(t), edge) - radius(t) == dist at some t, and
   * the moving centre stays inside its box, so an edge whose box is farther than
   * dist + max(r1,r2) from that box keeps that quantity strictly above dist and
   * yields no root. Query the R-tree with the swept box grown by that reach so
   * the root search visits only the boundary near the swept disk; the per-edge
   * prune keeps the candidate set identical to the flat scan, and the roots are
   * sorted afterwards so the visit order does not matter. */
  double cxmin = fmin(cx1, cx2), cxmax = fmax(cx1, cx2);
  double cymin = fmin(cy1, cy2), cymax = fmax(cy1, cy2);
  double reach = dist + fmax(r1, r2);
  double reach2 = reach * reach;
  STBox query;
  stbox_set(true, false, false, 0, cxmin - reach, cxmax + reach, cymin - reach,
    cymax + reach, 0, 0, NULL, &query);
  int ncand = rtree_search(ctx->g.rtree, RTREE_OVERLAPS, &query,
    geodist_pip_results);
  for (int j = 0; j < ncand; j++)
  {
    const GeoDistEdge *ed =
      &ctx->g.segs[*(int *) meos_array_get(geodist_pip_results, j)];
    double edx = fmax(fmax(ed->xmin - cxmax, cxmin - ed->xmax), 0.0);
    double edy = fmax(fmax(ed->ymin - cymax, cymin - ed->ymax), 0.0);
    if (edx * edx + edy * edy > reach2)
      continue;
    if (ed->is_arc)
      tcbuffersegm_arc_within_roots(cx1, cy1, cx2, cy2, r1, r2, ed, dist, cand,
        &nc);
    else
      tcbuffersegm_edge_within_roots(cx1, cy1, cx2, cy2, r1, r2, ed, dist, cand,
        &nc);
  }
  qsort(cand, nc, sizeof(double), tcbuffer_double_cmp);
  int m = 0;
  for (int i = 0; i < nc; i++)
    if (i == 0 || cand[i] - cand[m - 1] > 1e-15)
      cand[m++] = cand[i];
  int nout = 0;
  int k = 0;
  while (k < m - 1 && nout < maxout)
  {
    double tm = 0.5 * (cand[k] + cand[k + 1]);
    double cx = cx1 + (cx2 - cx1) * tm, cy = cy1 + (cy2 - cy1) * tm;
    double r = r1 + (r2 - r1) * tm;
    if (tcbuffer_disc_within_dist(cx, cy, r, dist, &ctx->g))
    {
      int ks = k;
      k++;
      while (k < m - 1)
      {
        double tm2 = 0.5 * (cand[k] + cand[k + 1]);
        double cx_2 = cx1 + (cx2 - cx1) * tm2, cy_2 = cy1 + (cy2 - cy1) * tm2;
        double r_2 = r1 + (r2 - r1) * tm2;
        if (tcbuffer_disc_within_dist(cx_2, cy_2, r_2, dist, &ctx->g)) k++;
        else break;
      }
      outlo[nout] = cand[ks];
      outhi[nout] = cand[k];
      nout++;
    }
    else
      k++;
  }
  pfree(cand);
  return nout;
}

/*****************************************************************************
 * Touches contact instants (GEOS-free)
 *
 * A disk touches a geometry when their boundaries meet while their interiors
 * stay disjoint (the DE-9IM touches predicate). For a disk of centre c and
 * radius r this is exactly
 *   sg(c, r) := min_edge [ dist(c, edge) - r ] == 0  AND  c is not inside a
 *   polygon of the geometry,
 * where the minimum runs over the geometry boundary edges with the SIGNED
 * per-edge distance (negative when the disk crosses that edge). The signed
 * minimum, unlike the within test's #geodist_segm_nad (which clamps interior
 * overlap to 0), separates a tangential contact (sg == 0) from an interior
 * penetration where a nearer edge drives the minimum negative (sg < 0); the
 * point-in-polygon guard rejects a boundary contact reached from inside a
 * polygon, whose interiors overlap. These are contact INSTANTS, so unlike the
 * within sub-periods an isolated tangency is preserved.
 *****************************************************************************/

/** @brief Tolerance on the signed boundary distance for a contact instant */
#define TCBUFFER_TOUCH_EPS 1e-9

/**
 * @brief Signed nearest boundary distance of a stationary disk: the minimum
 * over the geometry boundary edges of dist(centre, edge) - radius, without the
 * interior-overlap clamp of #geodist_segm_nad, and set @p inside to whether the
 * centre lies strictly inside a polygon of the geometry
 */
static double
tcbuffer_disc_signed_boundary(double cx, double cy, double r,
  const GeoDistGeom *g, bool *inside)
{
  *inside = g->has_poly && geodist_geom_point_inside(cx, cy, g);
  double best = DBL_MAX;
  /* Every caller only tests the signed boundary distance sg = dist(centre,edge)
   * - r against the +/-eps contact band, so only edges within r + eps of the
   * centre can change a decision. Bound the scan to that reach: when the true
   * minimum is <= eps the closest edge lies within reach so `best` equals the
   * global minimum exactly; otherwise `best` stays > eps (or DBL_MAX), which the
   * callers treat identically to any other value above the band. This is a fixed
   * reach box query, result-identical to the full running-minimum scan. */
  double reach = r + TCBUFFER_TOUCH_EPS;
  double reach2 = reach * reach;
  STBox query;
  stbox_set(true, false, false, 0, cx - reach, cx + reach, cy - reach,
    cy + reach, 0, 0, NULL, &query);
  int nc = rtree_search(g->rtree, RTREE_OVERLAPS, &query, geodist_pip_results);
  for (int j = 0; j < nc; j++)
  {
    const GeoDistEdge *ed =
      &g->segs[*(int *) meos_array_get(geodist_pip_results, j)];
    double edx = fmax(fmax(ed->xmin - cx, cx - ed->xmax), 0.0);
    double edy = fmax(fmax(ed->ymin - cy, cy - ed->ymax), 0.0);
    if (edx * edx + edy * edy > reach2)
      continue;
    double m = ed->is_arc ?
      geodist_segm_arc_mindist(cx, cy, cx, cy, r, r, ed) :
      geodist_segm_edge_mindist(cx, cy, cx, cy, r, r, ed);
    if (m < best) best = m;
  }
  return best;
}

/**
 * @brief Return true if a stationary circular buffer touches the geometry, i.e.
 * its boundary meets the geometry boundary with disjoint interiors (the signed
 * boundary distance vanishes and the centre is not inside a polygon)
 */
bool
tcbuffer_disc_touch_ctx(const Cbuffer *cb, const void *ctxv)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb));
  bool inside;
  double sg = tcbuffer_disc_signed_boundary(p->x, p->y, cb->radius, &ctx->g,
    &inside);
  return (! inside) && fabs(sg) <= TCBUFFER_TOUCH_EPS;
}

/**
 * @brief Return true if the geometry contains (@p strict) or covers
 * (not @p strict) a stationary circular buffer
 * @details A geometry contains a disk when the disk lies in the open interior,
 * and covers it when the disk lies in the closed region (tangency to the
 * boundary allowed). For a disk of centre @p c and radius @p r this is exactly
 *   c is strictly inside a polygon of the geometry  AND  sg(c, r) > 0 (contains)
 *   resp.  sg(c, r) >= 0   (covers),
 * where @p sg is the signed nearest boundary distance min_edge[dist(c, edge)-r]:
 * strictly positive means the whole disk clears the boundary, zero means it is
 * tangent to it, negative means it crosses to the exterior. The point-in-polygon
 * guard makes the relation false for a disk in a hole, outside the geometry, or
 * against a geometry with no polygonal (2D) component, which cannot contain a
 * positive-radius disk. This is the moving-disk analogue of the temporal point
 * containment rule generalized to a positive radius.
 */
bool
tcbuffer_disc_contains_ctx(const Cbuffer *cb, const void *ctxv, bool strict)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb));
  bool inside;
  double sg = tcbuffer_disc_signed_boundary(p->x, p->y, cb->radius, &ctx->g,
    &inside);
  return inside &&
    (strict ? sg > TCBUFFER_TOUCH_EPS : sg >= - TCBUFFER_TOUCH_EPS);
}

/**
 * @brief Append to @p outt the normalized times in (0,1) at which a linearly
 * moving disk touches the geometry
 * @details The candidate crossing times are the same region roots the within
 * kernel uses (#tcbuffersegm_edge_within_roots / #tcbuffersegm_arc_within_roots
 * at distance 0, where dist(centre, edge) == radius). Each is kept only when the
 * exact signed boundary distance vanishes there — a genuine tangential contact,
 * not an interior penetration where a nearer edge makes the signed minimum
 * negative, nor a spurious root of the squared equation where it stays
 * positive — and the centre is not inside a polygon. Returns the number of
 * contact times written (at most @p maxout)
 */
int
tcbufferseg_touch_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const void *ctxv, double *outt, int maxout)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb1));
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb2));
  double cx1 = p1->x, cy1 = p1->y, r1 = cb1->radius;
  double cx2 = p2->x, cy2 = p2->y, r2 = cb2->radius;
  int ncap = 8 * ctx->g.n + 2;
  double *cand = palloc(sizeof(double) * ncap);
  int nc = 0;
  /* The moving centre stays inside its box, so an edge farther than the larger
   * radius from that box is farther than the disk radius at every time and can
   * contribute no contact time. Query the R-tree with the swept box grown by
   * that reach so the root search visits only the boundary near the swept disk;
   * the per-edge prune keeps the candidate set identical to the flat scan. */
  double cxmin = fmin(cx1, cx2), cxmax = fmax(cx1, cx2);
  double cymin = fmin(cy1, cy2), cymax = fmax(cy1, cy2);
  double rmax = fmax(r1, r2);
  double rmax2 = rmax * rmax;
  STBox query;
  stbox_set(true, false, false, 0, cxmin - rmax, cxmax + rmax, cymin - rmax,
    cymax + rmax, 0, 0, NULL, &query);
  int ncand = rtree_search(ctx->g.rtree, RTREE_OVERLAPS, &query,
    geodist_pip_results);
  for (int j = 0; j < ncand; j++)
  {
    const GeoDistEdge *ed =
      &ctx->g.segs[*(int *) meos_array_get(geodist_pip_results, j)];
    double edx = fmax(fmax(ed->xmin - cxmax, cxmin - ed->xmax), 0.0);
    double edy = fmax(fmax(ed->ymin - cymax, cymin - ed->ymax), 0.0);
    if (edx * edx + edy * edy > rmax2)
      continue;
    if (ed->is_arc)
      tcbuffersegm_arc_within_roots(cx1, cy1, cx2, cy2, r1, r2, ed, 0.0, cand,
        &nc);
    else
      tcbuffersegm_edge_within_roots(cx1, cy1, cx2, cy2, r1, r2, ed, 0.0, cand,
        &nc);
  }
  qsort(cand, nc, sizeof(double), tcbuffer_double_cmp);
  int nout = 0;
  double last = -1.0;
  for (int i = 0; i < nc && nout < maxout; i++)
  {
    double t = cand[i];
    if (t <= 0.0 || t >= 1.0 || (nout > 0 && t - last <= 1e-12))
      continue;
    double cx = cx1 + (cx2 - cx1) * t, cy = cy1 + (cy2 - cy1) * t;
    double r = r1 + (r2 - r1) * t;
    bool inside;
    double sg = tcbuffer_disc_signed_boundary(cx, cy, r, &ctx->g, &inside);
    if (! inside && fabs(sg) <= TCBUFFER_TOUCH_EPS)
    {
      outt[nout++] = t;
      last = t;
    }
  }
  pfree(cand);
  return nout;
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

  return nad_tcbuffer_geo_analytic(temp, gs);
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
} TcbufferSegBox;

static int
tcbuffersegbox_cmp_minx(const void *a, const void *b)
{
  float da = ((const TcbufferSegBox *) a)->minx;
  float db = ((const TcbufferSegBox *) b)->minx;
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
cbuffersegm_edge_crit_in_t(double e, double q, double v, double dr2, double r_sum,
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
cbuffersegm_edge_crit_in_s(double e, double p, double u, double dr1, double r_sum,
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
    cbuffersegm_edge_crit_in_t(e, q, v, dr2, rA + rC, &best);
    /* Edge s=1: substitute (E + U) for E.  |E+U|^2 = e + u + 2p,
     * (E+U).V = q + w, radius at s=1 is rB. */
    cbuffersegm_edge_crit_in_t(e + u + 2.0 * p, q + w, v, dr2, rB + rC, &best);
  }
  if (u > 0.0)
  {
    /* Edge t=0: f(s,0) = sqrt(e + 2sp + s^2 u) - rA - rC - s dr1 */
    cbuffersegm_edge_crit_in_s(e, p, u, dr1, rA + rC, &best);
    /* Edge t=1: substitute (E - V) for E.  |E-V|^2 = e + v - 2q,
     * (E-V).U = p - w, radius at t=1 is rD. */
    cbuffersegm_edge_crit_in_s(e + v - 2.0 * q, p - w, u, dr1, rA + rD, &best);
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
  TcbufferSegBox *boxes2 = palloc(n2_segs * sizeof(TcbufferSegBox));
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
  qsort(boxes2, n2_segs, sizeof(TcbufferSegBox), tcbuffersegbox_cmp_minx);

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

  GSERIALIZED *result = shortestline_tcbuffer_geo_analytic(temp, gs);
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
