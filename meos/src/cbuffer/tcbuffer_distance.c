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
#include "geo/geo_funcs.h"
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
  /* A static circular buffer is a segment that does not move, so the question
   * is the one the two-segment turning point already answers: pass the value
   * as both endpoints of the second segment. Reading it here in closed form
   * instead solved d/dt (dist^2 - r^2) = 0, whose minimiser is that of the
   * product (dist - r)(dist + r) rather than of the gap the distance reports,
   * and the two part company as soon as the radius varies. */
  return tcbuffersegm_distance_turnpt(start, end, value, value, (Datum) 0,
    lower, upper, t1, t2);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the temporal distance between a temporal circular buffer and
 * a circular buffer
 * @csqlfn #Tdistance_tcbuffer_cbuffer() #Tdistance_cbuffer_tcbuffer()
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

/*****************************************************************************
 * Temporal distance to a geometry: exact full-geometry engine
 *
 * The bounding-circle composition above (geom_to_cbuffer + tdistance_tcbuffer
 * _cbuffer) collapses a non-point, non-circle geometry to its minimum
 * bounding circle, which is a systematic over-estimate for every line or
 * polygon. This engine instead reuses the #dist_geom_build edge
 * decomposition already shared with #nad_tcbuffer_geo_analytic and
 * #nai_tcbuffer_geo_analytic: for every edge (straight or circular-arc) it
 * finds the exact stationary points, in closed form, of the radius-aware
 * signed gap
 *   g(t) = dist(centre(t), edge) - r(t)
 * mirroring #dist_minfun (tgeo_distance.c). The distance to the whole
 * geometry at every segment endpoint and every per-edge turning point is
 * emitted as an exact instant, with linear interpolation in between,
 * exactly as #tpointseq_distance_geom (tpoint_geom_clip.c) does for the r = 0
 * (moving point) case: the global minimum over the segment is the minimum,
 * over edges, of each edge's own minimum, so emitting every per-edge
 * extremum makes the temporal float's minValue exact, and every emitted
 * value is the exact clamped distance #cbuffer_distance would report at that
 * instant.
 *****************************************************************************/

/**
 * @brief Return the exact distance from a stationary disc to the whole
 * geometry, clamped at zero to match #cbuffer_distance
 */
static inline double
tcbuffer_geom_dist(double cx, double cy, double r, const DistGeom *g)
{
  double best = DBL_MAX;
  dist_segm_nad(cx, cy, r, cx, cy, r, g, &best);
  return fmax(best, 0.0);
}

/**
 * @brief Append a candidate parameter to the array if it lies in [0,1]
 * (clamping a tiny out-of-range value to the nearest endpoint)
 * @details A candidate outside its originating feature's true region is
 * harmless: the caller always re-evaluates the exact distance to the whole
 * geometry at the emitted parameter (#tcbuffer_geom_dist), so an extra
 * sample never changes correctness, only adds a redundant instant
 */
static void
tcbuffer_add_within01(double t, double *cand, int *nc)
{
  if (t > -1e-9 && t < 1.0 + 1e-9)
  {
    if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
    cand[(*nc)++] = t;
  }
}

/**
 * @brief Append the [0,1] stationary-point candidates of
 * sqrt(A t^2 + B t + C) - (a value increasing at rate DR)
 * @details Same closed form as #dist_minfun (tgeo_distance.c), which
 * minimises this quantity: the constant term of the subtracted value drops
 * out of its derivative, so only DR enters the (up to two) roots of the
 * stationarity equation squared, (A^2 - A DR^2) t^2 + (A B - B DR^2) t +
 * (B^2/4 - C DR^2) = 0, plus the quadratic vertex -B/(2A) explicitly, since
 * in the perpendicular regime that quadratic is a perfect square and the
 * discriminant can round to a spurious negative, silently dropping the true
 * double root -- the instant the moving centre crosses the edge's supporting
 * line or circle (an exact zero-distance overlap)
 */
static void
tcbuffer_add_dist_turnpts(double A, double B, double C, double DR,
  double *cand, int *nc)
{
  double DR_2 = DR * DR;
  double a2 = A * (A - DR_2);
  double a1 = B * (A - DR_2);
  double a0 = 0.25 * B * B - C * DR_2;
  if (fabs(a2) > 1e-18)
  {
    double disc = a1 * a1 - 4.0 * a2 * a0;
    if (disc >= 0.0)
    {
      double sd = sqrt(disc);
      tcbuffer_add_within01((-a1 + sd) / (2.0 * a2), cand, nc);
      tcbuffer_add_within01((-a1 - sd) / (2.0 * a2), cand, nc);
    }
  }
  else if (fabs(a1) > 1e-18)
    tcbuffer_add_within01(-a0 / a1, cand, nc);
  if (fabs(A) > 1e-18)
    tcbuffer_add_within01(-B / (2.0 * A), cand, nc);
}

/**
 * @brief Append the [0,1] candidate turning parameters of the moving disc
 * distance to a straight edge, mirroring the endpoint/perpendicular feature
 * split of #dist_segm_edge_mindist
 * @param[in] cx1,cy1,cx2,cy2 Centre of the disc at the segment bounds
 * @param[in] r1,r2 Radius of the disc at the segment bounds
 * @param[in] e Edge
 * @param[in,out] cand,nc Candidate array and count
 */
static void
tcbuffersegm_edge_dist_turnpts(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const DistEdge *e, double *cand, int *nc)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1, dr = r2 - r1;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay, l2 = ux * ux + uy * uy;

  /* Closest approach to the edge start */
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    tcbuffer_add_dist_turnpts(A, B, C, dr, cand, nc);
  }
  /* Degenerate edge (a point): only the point-distance branch applies */
  if (l2 <= 1e-24)
    return;

  /* Closest approach to the edge end */
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - bx) * dcx + (cy1 - by) * dcy);
    double C = (cx1 - bx) * (cx1 - bx) + (cy1 - by) * (cy1 - by);
    tcbuffer_add_dist_turnpts(A, B, C, dr, cand, nc);
  }
  /* Perpendicular-foot time (moving centre on the supporting line) */
  const double k0 = (cx1 - ax) * uy - (cy1 - ay) * ux;
  const double k1 = dcx * uy - dcy * ux;
  if (fabs(k1) > 1e-18)
  {
    double A = k1 * k1 / l2;
    double B = 2.0 * k0 * k1 / l2;
    double C = k0 * k0 / l2;
    tcbuffer_add_dist_turnpts(A, B, C, dr, cand, nc);
  }
  /* Foot-parameter region boundaries (s = 0 and s = 1): the nearest feature
   * changes there, a kink in the per-edge distance */
  const double s0 = (cx1 - ax) * ux + (cy1 - ay) * uy;
  const double s1 = dcx * ux + dcy * uy;
  if (fabs(s1) > 1e-18)
  {
    tcbuffer_add_within01(-s0 / s1, cand, nc);
    tcbuffer_add_within01((l2 - s0) / s1, cand, nc);
  }
}

/**
 * @brief Append the [0,1] candidate turning parameters of the moving disc
 * distance to a circular-arc edge, mirroring the on-span/off-span split of
 * #dist_segm_arc_mindist
 * @param[in] cx1,cy1,cx2,cy2 Centre of the disc at the segment bounds
 * @param[in] r1,r2 Radius of the disc at the segment bounds
 * @param[in] e Edge
 * @param[in,out] cand,nc Candidate array and count
 */
static void
tcbuffersegm_arc_dist_turnpts(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const DistEdge *e, double *cand, int *nc)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1, dr = r2 - r1;
  const double px = e->acx, py = e->acy, R = e->arad;
  const double A = dcx * dcx + dcy * dcy;
  const double B = 2.0 * ((cx1 - px) * dcx + (cy1 - py) * dcy);
  const double C = (cx1 - px) * (cx1 - px) + (cy1 - py) * (cy1 - py);

  /* Circle crossings sqrt(Q) = R: kinks of the absolute value defining the
   * distance to the arc's supporting circle */
  {
    double c0 = C - R * R;
    if (fabs(A) > 1e-18)
    {
      double disc = B * B - 4.0 * A * c0;
      if (disc >= 0.0)
      {
        double sd = sqrt(disc);
        tcbuffer_add_within01((-B + sd) / (2.0 * A), cand, nc);
        tcbuffer_add_within01((-B - sd) / (2.0 * A), cand, nc);
      }
    }
    else if (fabs(B) > 1e-18)
      tcbuffer_add_within01(-c0 / B, cand, nc);
  }
  /* Vertex of Q: closest/farthest approach to the arc's centre */
  if (fabs(A) > 1e-18)
    tcbuffer_add_within01(-B / (2.0 * A), cand, nc);
  /* Stationary points of | sqrt(Q) - R | - r(t): R is a constant offset so
   * it cancels in the derivative, giving the same closed form as the
   * straight-edge branches */
  tcbuffer_add_dist_turnpts(A, B, C, dr, cand, nc);
  /* Angular-sector boundary crossings: the moving centre crosses the ray
   * from the arc's centre through an endpoint. Purely positional, so the
   * same closed form applies regardless of the radius */
  for (int ep = 0; ep < 2; ep++)
  {
    double ex = ep == 0 ? e->x1 : e->x2, ey = ep == 0 ? e->y1 : e->y2;
    double ddx = ex - px, ddy = ey - py;
    double wx = cx1 - px, wy = cy1 - py;
    double den = dcx * ddy - dcy * ddx;
    if (fabs(den) > 1e-18)
      tcbuffer_add_within01(-(wx * ddy - wy * ddx) / den, cand, nc);
  }
  /* Off-span regions are nearest to an arc endpoint */
  for (int ep = 0; ep < 2; ep++)
  {
    double ex = ep == 0 ? e->x1 : e->x2, ey = ep == 0 ? e->y1 : e->y2;
    double Be = 2.0 * ((cx1 - ex) * dcx + (cy1 - ey) * dcy);
    double Ce = (cx1 - ex) * (cx1 - ex) + (cy1 - ey) * (cy1 - ey);
    tcbuffer_add_dist_turnpts(A, Be, Ce, dr, cand, nc);
  }
}

/**
 * @brief Comparator for sorting the per-segment distance turning-point
 * candidates
 */
static int
tcbufferdist_cand_cmp(const void *a, const void *b)
{
  double d = *(const double *) a - *(const double *) b;
  return (d < 0.0) ? -1 : (d > 0.0 ? 1 : 0);
}

/**
 * @brief Return the temporal distance of one temporal circular buffer
 * sequence to a geometry given as its edge decomposition
 */
static TSequence *
tcbufferseq_distance_geom(const TSequence *seq, const DistGeom *g)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    /* Step or discrete interpolation, or a single instant: every instant is
     * a stationary disc, no interior turning point. The result keeps the
     * input's own interpolation (discrete stays discrete, step stays step) */
    interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
      const POINT2D *p = cbuffer_point2d_p(c);
      double d = tcbuffer_geom_dist(p->x, p->y, c->radius, g);
      instants[i] = tinstant_make(Float8GetDatum(d), T_TFLOAT, inst->t);
    }
    return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
      seq->period.upper_inc, interp, NORMALIZE);
  }

  /* Linear interpolation, at least two instants: upper bound on the number
   * of result instants is the two endpoints of every segment plus, per edge,
   * up to eleven straight-edge or fourteen arc-edge turning points */
  int cap = g->n * 16 + 4;
  int maxinsts = 1 + (seq->count - 1) * cap;
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  int ninsts = 0;
  double *cand = palloc(sizeof(double) * cap);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const Cbuffer *c1 = DatumGetCbufferP(tinstant_value_p(inst1));
  const POINT2D *p1 = cbuffer_point2d_p(c1);
  instants[ninsts++] = tinstant_make(
    Float8GetDatum(tcbuffer_geom_dist(p1->x, p1->y, c1->radius, g)), T_TFLOAT,
    inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *c2 = DatumGetCbufferP(tinstant_value_p(inst2));
    const POINT2D *p2 = cbuffer_point2d_p(c2);

    int nc = 0;
    for (int j = 0; j < g->n; j++)
    {
      const DistEdge *e = &g->segs[j];
      if (e->is_arc)
        tcbuffersegm_arc_dist_turnpts(p1->x, p1->y, p2->x, p2->y, c1->radius,
          c2->radius, e, cand, &nc);
      else
        tcbuffersegm_edge_dist_turnpts(p1->x, p1->y, p2->x, p2->y, c1->radius,
          c2->radius, e, cand, &nc);
    }
    qsort(cand, nc, sizeof(double), tcbufferdist_cand_cmp);

    const double duration = (double) (inst2->t - inst1->t);
    TimestampTz prevt = inst1->t;
    for (int k = 0; k < nc; k++)
    {
      double frac = cand[k];
      if (frac <= MEOS_EPSILON || frac >= 1.0 - MEOS_EPSILON)
        continue;
      if (k > 0 && fabs(frac - cand[k - 1]) < MEOS_EPSILON)
        continue;
      TimestampTz t = inst1->t + (TimestampTz) (duration * frac);
      /* Keep the instants strictly increasing and off the segment bounds */
      if (t <= prevt || t >= inst2->t)
        continue;
      double cx = p1->x + frac * (p2->x - p1->x);
      double cy = p1->y + frac * (p2->y - p1->y);
      double r = c1->radius + frac * (c2->radius - c1->radius);
      instants[ninsts++] = tinstant_make(
        Float8GetDatum(tcbuffer_geom_dist(cx, cy, r, g)), T_TFLOAT, t);
      prevt = t;
    }
    instants[ninsts++] = tinstant_make(
      Float8GetDatum(tcbuffer_geom_dist(p2->x, p2->y, c2->radius, g)),
      T_TFLOAT, inst2->t);

    inst1 = inst2; c1 = c2; p1 = p2;
  }
  pfree(cand);

  return tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @brief Return the temporal distance of a temporal circular buffer to a
 * geometry given as its edge decomposition
 */
static Temporal *
tdistance_tcbuffer_geo_analytic(const Temporal *temp, const DistGeom *g)
{
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
    const POINT2D *p = cbuffer_point2d_p(c);
    double d = tcbuffer_geom_dist(p->x, p->y, c->radius, g);
    return (Temporal *) tinstant_make(Float8GetDatum(d), T_TFLOAT, inst->t);
  }
  if (temp->subtype == TSEQUENCE)
    return (Temporal *) tcbufferseq_distance_geom((TSequence *) temp, g);
  /* TSEQUENCESET */
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    sequences[i] = tcbufferseq_distance_geom(TSEQUENCESET_SEQ_N(ss, i), g);
  return (Temporal *) tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the temporal distance between a temporal circular buffer and
 * a geometry
 * @details The geometry is decomposed into its boundary edges (straight and
 * circular-arc), the same decomposition #nad_tcbuffer_geo and
 * #nearestApproachInstant use, and the distance to the full geometry is
 * computed exactly rather than to its minimum bounding circle. A geometry
 * with no exact edge decomposition (a TIN or a polyhedral surface) falls
 * back to the bounding-circle approximation, mirroring the other analytic
 * distance kernels of this file.
 * @csqlfn #Tdistance_tcbuffer_geo() #Tdistance_geo_tcbuffer()
 */
Temporal *
tdistance_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  DistGeom g;
  if (dist_geom_build(gs, &g))
  {
    Temporal *result = tdistance_tcbuffer_geo_analytic(temp, &g);
    dist_geom_free(&g);
    return result;
  }

  /* No exact edge decomposition (TIN / polyhedral surface): fall back to the
   * bounding-circle approximation */
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
 *
 * Analytic nearest approach instant between a temporal circular buffer and a
 * geometry
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
nai_tcbufferseq(const TSequence *seq, const DistGeom *g, DistNai *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
      const POINT2D *p = cbuffer_point2d_p(c);
      dist_segm_nai(p->x, p->y, c->radius, inst->t, p->x, p->y, c->radius,
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
    const POINT2D *p1 = cbuffer_point2d_p(c1);
    const POINT2D *p2 = cbuffer_point2d_p(c2);
    dist_segm_nai(p1->x, p1->y, c1->radius, i1->t, p2->x, p2->y, c2->radius,
      i2->t, g, w);
    i1 = i2;
  }
}

/**
 * @brief Nearest approach instant between a temporal circular buffer and a
 * geometry
 * @details Returns the timestamp attaining the minimum swept-capsule distance
 * in @p result. Returns false when the analytic path does not apply (an
 * unsupported geometry type) so the caller can fall back to the centreline
 * delegation.
 */
static bool
nai_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs,
  TimestampTz *result)
{
  DistGeom g;
  if (! dist_geom_build(gs, &g))
    return false;

  DistNai w;
  w.d = DBL_MAX; w.t = 0; w.set = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (TInstant *) temp;
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p(inst));
    const POINT2D *p = cbuffer_point2d_p(c);
    dist_segm_nai(p->x, p->y, c->radius, inst->t, p->x, p->y, c->radius,
      inst->t, &g, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    nai_tcbufferseq((TSequence *) temp, &g, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      nai_tcbufferseq(TSEQUENCESET_SEQ_N(ss, i), &g, &w);
  }
  dist_geom_free(&g);
  if (! w.set)
    return false;
  *result = w.t;
  return true;
}

/*****************************************************************************/

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of the temporal circular buffer
 * and a geometry
 * @param[in] temp Temporal circular buffer
 * @param[in] gs Geometry
 * @csqlfn #NAI_tcbuffer_geo() #NAI_geo_tcbuffer()
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

  /* A geometry that has no edge decomposition: take the instant from the exact
   * temporal distance, which honours the radius through the traversed area, as
   * the nearest approach distance does on the same geometries. Reading the
   * centreline instead minimises the distance of the centre, a quantity that
   * parts from the one the nearest approach reports as soon as the radius
   * varies, so the instant it returns is one whose distance is not the nearest
   * approach. */
  Temporal *dist = tdistance_tcbuffer_geo(temp, gs);
  if (! dist)
    return NULL;
  const TInstant *min = temporal_min_inst_p((const Temporal *) dist);
  TimestampTz tmin = min->t;
  pfree(dist);
  Datum value;
  if (! temporal_value_at_timestamptz(temp, tmin, false, &value))
    return NULL;
  return tinstant_make_free(value, temp->temptype, tmin);
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach instant of the circular buffer and a
 * temporal circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #NAI_tcbuffer_cbuffer() #NAI_cbuffer_tcbuffer()
 */
TInstant *
nai_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  /* A static disc is a constant temporal circular buffer, so the nearest
   * approach instant is the one the two-temporal kernel already computes.
   * Converting the disc to a geometry instead would measure to the circle's
   * defining vertices, and dropping the temporal radius would minimise a
   * different quantity than the nearest approach distance reports. */
  Temporal *ctemp = tcbuffer_from_base_temp(cb, temp);
  TInstant *result = nai_tcbuffer_tcbuffer(temp, ctemp);
  pfree(ctemp);
  return result;
}

/* Defined below, beside the nearest approach distance that shares it */
static double tcbufferseg_distance_lb(Datum start1, Datum end1,
  Datum start2, Datum end2);

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

  /* Fast path: the time-synchronous running minimum reports the instant that
   * attains it, so the nearest approach instant reads off the same walk the
   * nearest approach distance takes, without materialising the temporal
   * distance. A finite result is exact; the infinity sentinel (empty or
   * degenerate overlap) defers to the temporal distance path */
  if (nad_tcont_tcont_sync_applies(temp1, temp2))
  {
    TimestampTz t;
    double d = nad_tcont_tcont_sync(temp1, temp2, &datum_cbuffer_distance,
      &tcbuffersegm_distance_turnpt, &tcbufferseg_distance_lb, &t);
    if (d != DBL_MAX)
    {
      /* The closest point may be at an exclusive bound. */
      Datum value;
      if (temporal_value_at_timestamptz(temp1, t, false, &value))
        return tinstant_make_free(value, temp1->temptype, t);
    }
  }

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
 * Analytic nearest approach distance between a temporal circular buffer and a
 * geometry
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
 * built natively. The nearest-approach value path above is left
 * untouched.
 *****************************************************************************/

/**
 * @brief Return the squared 2D distance between two axis-aligned bounding
 * boxes, 0 when they overlap
 * @note The square is returned so that the callers compare it with a squared
 * distance, as the geometry distance prune does, the square root being
 * monotonic and both compared values non-negative
 */
static double
box2d_distance_sqr(double axmin, double aymin, double axmax, double aymax,
  double bxmin, double bymin, double bxmax, double bymax)
{
  double dx = fmax(fmax(axmin - bxmax, bxmin - axmax), 0.0);
  double dy = fmax(fmax(aymin - bymax, bymin - aymax), 0.0);
  return dx * dx + dy * dy;
}

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
  const DistGeom *g)
{
  if (g->has_poly && dist_geom_point_inside(cx, cy, g))
    return true;
  double sxmin = cx - r, sxmax = cx + r, symin = cy - r, symax = cy + r;
  double dist2 = dist * dist;
  STBox query;
  stbox_set(true, false, false, 0, sxmin - dist, sxmax + dist, symin - dist,
    symax + dist, 0, 0, NULL, &query);
  int nc = rtree_search(g->rtree, INDEX_OVERLAPS, &query, dist_pip_results);
  for (int j = 0; j < nc; j++)
  {
    const DistEdge *ed =
      &g->segs[*(int64 *) meos_array_get(dist_pip_results, j)];
    if (box2d_distance_sqr(ed->xmin, ed->ymin, ed->xmax, ed->ymax, sxmin,
        symin, sxmax, symax) > dist2)
      continue;
    double m = ed->is_arc ?
      dist_segm_arc_mindist(cx, cy, cx, cy, r, r, ed) :
      dist_segm_edge_mindist(cx, cy, cx, cy, r, r, ed);
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
tcbufferseq_nad(const TSequence *seq, const DistGeom *g, double *best)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && *best > 0.0; i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = cbuffer_point2d_p(c);
      dist_segm_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, g, best);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && *best > 0.0; i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const Cbuffer *c1 = DatumGetCbufferP(tinstant_value_p(i1));
    const Cbuffer *c2 = DatumGetCbufferP(tinstant_value_p(i2));
    const POINT2D *p1 = cbuffer_point2d_p(c1);
    const POINT2D *p2 = cbuffer_point2d_p(c2);
    dist_segm_nad(p1->x, p1->y, c1->radius, p2->x, p2->y, c2->radius, g, best);
    i1 = i2;
  }
}

/**
 * @brief Nearest approach distance between a temporal circular buffer and a
 * geometry
 */
static double
nad_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  /* A geometry that has no edge decomposition, or no segments: fall back to
   * the exact traversed-area distance so the result is never wrong */
  DistGeom g;
  if (! dist_geom_build(gs, &g))
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
    const POINT2D *p = cbuffer_point2d_p(c);
    dist_segm_nad(p->x, p->y, c->radius, p->x, p->y, c->radius, &g, &best);
  }
  else if (temp->subtype == TSEQUENCE)
    tcbufferseq_nad((TSequence *) temp, &g, &best);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && best > 0.0; i++)
      tcbufferseq_nad(TSEQUENCESET_SEQ_N(ss, i), &g, &best);
  }

  dist_geom_free(&g);
  return best < 0.0 ? 0.0 : best;
}

/**
 * @brief Update the witness with one temporal circular buffer sequence
 */
static void
tcbufferseq_shortestline(const TSequence *seq, const DistGeom *g,
  DistShortLine *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const Cbuffer *c = DatumGetCbufferP(
        tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
      const POINT2D *p = cbuffer_point2d_p(c);
      dist_segm_shortestline(p->x, p->y, c->radius, p->x, p->y, c->radius,
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
    const POINT2D *p1 = cbuffer_point2d_p(c1);
    const POINT2D *p2 = cbuffer_point2d_p(c2);
    dist_segm_shortestline(p1->x, p1->y, c1->radius, p2->x, p2->y,
      c2->radius, g, w);
    i1 = i2;
  }
}

/**
 * @brief Shortest line between a temporal circular buffer and a geometry,
 * arc-exact for circular-arc input. Returns NULL when the analytic path does
 * not apply (an unsupported geometry type), so the caller can fall back to the
 * traversed-area shortest line.
 */
static GSERIALIZED *
shortestline_tcbuffer_geo_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  DistGeom g;
  if (! dist_geom_build(gs, &g))
    return NULL;

  DistShortLine w;
  w.d = DBL_MAX; w.set = false;
  w.px = w.py = w.qx = w.qy = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const Cbuffer *c = DatumGetCbufferP(tinstant_value_p((TInstant *) temp));
    const POINT2D *p = cbuffer_point2d_p(c);
    dist_segm_shortestline(p->x, p->y, c->radius, p->x, p->y, c->radius,
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
  dist_geom_free(&g);
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
 * Temporal within relationship
 *
 * The sub-periods during which a temporal circular buffer stays within a
 * distance of a geometry, from the same swept-capsule distance
 * kernel as the nearest-approach value. The candidate crossing instants are
 * the roots of dist(centre(t), edge) = radius(t) + dist per boundary edge;
 * each candidate sub-interval is classified with the exact interior-aware unit
 * distance, so the ever-projection of the result agrees with the
 * nearest-approach value.
 *****************************************************************************/

/* Context kind discriminator shared as the first member of every context
 * struct, so the ever/always/spanset scan kernels can be reused verbatim for a
 * geometry boundary (#TcbufferGeoCtx) or for a single static circular buffer
 * (#TcbufferDiscCtx) */
#define TCBUF_CTX_GEO 0
#define TCBUF_CTX_DISC 1

/**
 * @brief Reusable geometry context that owns the boundary segments and the
 * bucket hierarchy, so many discs and segments can be tested against one
 * geometry without reparsing it
 */
typedef struct
{
  int kind;             /**< Always #TCBUF_CTX_GEO */
  DistEdge *segs;
  DistGeom g;
} TcbufferGeoCtx;

/**
 * @brief Reusable disc context that owns a single static circular buffer, so
 * the contains/covers scan kernels test a moving disk against it exactly (disc
 * in disc, no polygon approximation of the boundary)
 */
typedef struct
{
  int kind;             /**< Always #TCBUF_CTX_DISC */
  Cbuffer cb;           /**< The static circular buffer */
  bool container_is_temporal; /**< True if the temporal disk is the container
                                   (temp contains cb), false if @p cb contains
                                   the temporal disk */
} TcbufferDiscCtx;

/**
 * @brief Build the reusable geometry context for the native within kernel from
 * the straight and circular-arc edges of the boundary, or return NULL for a
 * geometry that has no edge decomposition, that is, a TIN or a polyhedral
 * surface (the caller then uses the traversed-area path)
 */
void *
tcbuffer_geo_ctx_make(const GSERIALIZED *gs)
{
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  DistEdge *segs = NULL;
  int cap = 0, n = 0;
  bool has_poly = false;
  bool ok = dist_geom_edges(lw, true, &segs, &cap, &n, &has_poly);
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
  ctx->kind = TCBUF_CTX_GEO;
  ctx->segs = segs;
  ctx->g = (DistGeom) { segs, n, has_poly, gxmin, gymin, gxmax, gymax, NULL,
    0, dist_geom_build_rtree(segs, n) };
  /* Scratch buffer for the R-tree candidate ids, created with the R-tree and
   * freed with it in #tcbuffer_geo_ctx_free (see dist_pip_results). */
  dist_pip_results = meos_array_create(sizeof(int64));
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
  if (dist_pip_results)
  {
    meos_array_destroy(dist_pip_results);
    dist_pip_results = NULL;
  }
  pfree(c->segs);
  pfree(c);
}

/**
 * @brief Build a disc context wrapping a single static circular buffer for the
 * exact disc-in-disc contains/covers scan kernels
 * @param[in] cb Static circular buffer
 * @param[in] container_is_temporal True if the temporal disk is the container
 * (temp contains @p cb), false if @p cb is the container (@p cb contains temp)
 */
void *
tcbuffer_disc_ctx_make(const Cbuffer *cb, bool container_is_temporal)
{
  TcbufferDiscCtx *ctx = palloc(sizeof(TcbufferDiscCtx));
  ctx->kind = TCBUF_CTX_DISC;
  ctx->cb = *cb;
  ctx->container_is_temporal = container_is_temporal;
  return ctx;
}

/**
 * @brief Free a disc context built by #tcbuffer_disc_ctx_make
 */
void
tcbuffer_disc_ctx_free(void *ctx)
{
  if (ctx)
    pfree(ctx);
}

/**
 * @brief Return the number of boundary segments in a context, used to size the
 * per-segment root output (a disc boundary yields at most the two roots of the
 * quadratic clearance equation)
 */
int
tcbuffer_geo_ctx_nsegs(const void *ctxv)
{
  if (*(const int *) ctxv == TCBUF_CTX_DISC)
    return 1;
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
  const POINT2D *p = cbuffer_point2d_p(cb);
  return tcbuffer_disc_within_dist(p->x, p->y, cb->radius, dist, &ctx->g);
}

/**
 * @brief Append to @p cand the normalized times in [lo,hi] at which the moving
 * disc distance to a region equals @p dist, i.e. the roots of
 * (A - DR^2) t^2 + (B - 2 R0 DR) t + (C - R0^2) = 0 with R0 = r1 + dist
 * @note The bounds are inclusive. The caller splits a segment into
 * perpendicular/endpoint sub-regions at breakpoints (#tcbuffersegm_edge_within_roots);
 * when the moving centre crosses the geometry exactly at a polygon vertex, the
 * crossing time coincides exactly with the breakpoint shared by the two
 * adjacent sub-regions, and is a genuine root of BOTH regions' equations
 * there. A strict `> lo && < hi` test drops it from both sides, so the true
 * crossing silently vanishes from @p cand and the within-distance test
 * over-extends the sub-period it reports
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
      if (t >= lo && t <= hi)
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
  if (t1 >= lo && t1 <= hi)
    cand[(*nc)++] = t1;
  if (t2 >= lo && t2 <= hi)
    cand[(*nc)++] = t2;
}

/**
 * @brief Append the within-distance crossing times of one moving disc segment
 * against one geometry edge, mirroring the perpendicular/endpoint region split
 * of #dist_segm_edge_mindist
 */
static void
tcbuffersegm_edge_within_roots(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const DistEdge *e, double dist, double *cand, int *nc)
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
 * off-span endpoint split of #dist_segm_arc_mindist
 * @details On the arc's angular span the distance to the disc is
 * | sqrt(Q(t)) - R | - r(t); setting it equal to @p dist gives
 * sqrt(Q) = R + r(t) + dist (disc outside the circle) or
 * sqrt(Q) = R - r(t) - dist (disc inside), each a region-crossing quadratic with
 * the arc radius folded into R0. Off the span the nearest arc point is an
 * endpoint, so the two endpoint region crossings are added as well. The result
 * is a superset of the true crossings; each candidate sub-interval is then
 * classified exactly by the arc-aware unit distance (#dist_segm_nad), so
 * roots from the squared equation or the wrong angular regime are harmless.
 */
static void
tcbuffersegm_arc_within_roots(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const DistEdge *e, double dist, double *cand, int *nc)
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
  const POINT2D *p1 = cbuffer_point2d_p(cb1);
  const POINT2D *p2 = cbuffer_point2d_p(cb2);
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
  int ncand = rtree_search(ctx->g.rtree, INDEX_OVERLAPS, &query,
    dist_pip_results);
  for (int j = 0; j < ncand; j++)
  {
    const DistEdge *ed =
      &ctx->g.segs[*(int64 *) meos_array_get(dist_pip_results, j)];
    if (box2d_distance_sqr(ed->xmin, ed->ymin, ed->xmax, ed->ymax, cxmin,
        cymin, cxmax, cymax) > reach2)
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
 * Touches contact instants
 *
 * A disk touches a geometry when their boundaries meet while their interiors
 * stay disjoint (the DE-9IM touches predicate). For a disk of centre c and
 * radius r this is exactly
 *   sg(c, r) := min_edge [ dist(c, edge) - r ] == 0  AND  c is not inside a
 *   polygon of the geometry,
 * where the minimum runs over the geometry boundary edges with the SIGNED
 * per-edge distance (negative when the disk crosses that edge). The signed
 * minimum, unlike the within test's #dist_segm_nad (which clamps interior
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
 * interior-overlap clamp of #dist_segm_nad, and set @p inside to whether
 * the centre lies strictly inside a polygon of the geometry
 */
static double
tcbuffer_disc_signed_boundary(double cx, double cy, double r,
  const DistGeom *g, bool *inside)
{
  *inside = g->has_poly && dist_geom_point_inside(cx, cy, g);
  double best = DBL_MAX;
  /* Every caller only tests the signed boundary distance sg = dist(centre,edge)
   * - r against the +/-eps contact band, so only edges within r + eps of the
   * centre can change a decision. Bound the scan to that reach: when the true
   * minimum is <= eps the closest edge lies within reach so `best` equals the
   * global minimum exactly; otherwise `best` stays > eps (or DBL_MAX), which 
   * the callers treat identically to any other value above the band. This is a
   * fixed reach box query, result-identical to the full running-minimum scan. */
  double reach = r + TCBUFFER_TOUCH_EPS;
  double reach2 = reach * reach;
  STBox query;
  stbox_set(true, false, false, 0, cx - reach, cx + reach, cy - reach,
    cy + reach, 0, 0, NULL, &query);
  int nc = rtree_search(g->rtree, INDEX_OVERLAPS, &query, dist_pip_results);
  for (int j = 0; j < nc; j++)
  {
    const DistEdge *ed =
      &g->segs[*(int64 *) meos_array_get(dist_pip_results, j)];
    if (box2d_distance_sqr(ed->xmin, ed->ymin, ed->xmax, ed->ymax, cx, cy,
        cx, cy) > reach2)
      continue;
    double m = ed->is_arc ?
      dist_segm_arc_mindist(cx, cy, cx, cy, r, r, ed) :
      dist_segm_edge_mindist(cx, cy, cx, cy, r, r, ed);
    if (m < best) best = m;
  }
  return best;
}

/**
 * @brief Return true if a stationary circular buffer touches the geometry, 
 * i.e., its boundary meets the geometry boundary with disjoint interiors (the
 * signed boundary distance vanishes and the centre is not inside a polygon)
 */
bool
tcbuffer_disc_touch_ctx(const Cbuffer *cb, const void *ctxv)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p = cbuffer_point2d_p(cb);
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
  if (*(const int *) ctxv == TCBUF_CTX_DISC)
  {
    /* Exact disc-in-disc test: a disk (C, rc) is contained in a disk (P, R)
     * iff dist(P, C) + rc <= R (covers) / < R (contains). The moving disk is
     * @p cb; the static disk and the containment direction come from the
     * context */
    const TcbufferDiscCtx *d = (const TcbufferDiscCtx *) ctxv;
    const Cbuffer *outer = d->container_is_temporal ? cb : &d->cb;
    const Cbuffer *inner = d->container_is_temporal ? &d->cb : cb;
    double gap = hypot(inner->x - outer->x, inner->y - outer->y) +
      inner->radius - outer->radius;
    return strict ? (gap < - TCBUFFER_TOUCH_EPS) : (gap <= TCBUFFER_TOUCH_EPS);
  }
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p = cbuffer_point2d_p(cb);
  bool inside;
  double sg = tcbuffer_disc_signed_boundary(p->x, p->y, cb->radius, &ctx->g,
    &inside);
  return inside &&
    (strict ? sg > TCBUFFER_TOUCH_EPS : sg >= - TCBUFFER_TOUCH_EPS);
}

/**
 * @brief Append to @p outt the normalized times in (0,1) at which the signed
 * boundary distance of a linearly moving disk vanishes, keeping only the
 * contacts made from outside the geometry when @p outside_only is true
 * @details The candidate crossing times are the same region roots the within
 * kernel uses (#tcbuffersegm_edge_within_roots / #tcbuffersegm_arc_within_roots
 * at distance 0, where dist(centre, edge) == radius). Each is kept only when the
 * exact signed boundary distance vanishes there — not an interior penetration
 * where a nearer edge makes the signed minimum negative, nor a spurious root of
 * the squared equation where it stays positive. Returns the number of times
 * written (at most @p maxout)
 */
static int
tcbufferseg_sg_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const void *ctxv, double *outt, int maxout, bool outside_only)
{
  const TcbufferGeoCtx *ctx = (const TcbufferGeoCtx *) ctxv;
  const POINT2D *p1 = cbuffer_point2d_p(cb1);
  const POINT2D *p2 = cbuffer_point2d_p(cb2);
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
  int ncand = rtree_search(ctx->g.rtree, INDEX_OVERLAPS, &query,
    dist_pip_results);
  for (int j = 0; j < ncand; j++)
  {
    const DistEdge *ed =
      &ctx->g.segs[*(int64 *) meos_array_get(dist_pip_results, j)];
    if (box2d_distance_sqr(ed->xmin, ed->ymin, ed->xmax, ed->ymax, cxmin,
        cymin, cxmax, cymax) > rmax2)
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
    if (t <= 0.0 || t >= 1.0 || (nout > 0 && t - last <= MEOS_GEOM_TOLERANCE))
      continue;
    double cx = cx1 + (cx2 - cx1) * t, cy = cy1 + (cy2 - cy1) * t;
    double r = r1 + (r2 - r1) * t;
    bool inside;
    double sg = tcbuffer_disc_signed_boundary(cx, cy, r, &ctx->g, &inside);
    if ((! outside_only || ! inside) && fabs(sg) <= TCBUFFER_TOUCH_EPS)
    {
      outt[nout++] = t;
      last = t;
    }
  }
  pfree(cand);
  return nout;
}

/**
 * @brief Append to @p outt the normalized times in (0,1) at which a linearly
 * moving disk touches the geometry
 * @details A touch requires the interiors to be disjoint, so a vanishing signed
 * boundary distance reached with the centre inside a polygon — the disk grazing
 * the boundary from within — is not a contact and is left out. Returns the
 * number of contact times written (at most @p maxout)
 */
int
tcbufferseg_touch_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const void *ctxv, double *outt, int maxout)
{
  return tcbufferseg_sg_roots(cb1, cb2, ctxv, outt, maxout, true);
}

/**
 * @brief Append to @p outt the normalized times in (0,1) at which the disk
 * boundary and the geometry boundary of a linearly moving disk are tangent,
 * from either side
 * @details These are the instants at which the geometry can start or stop
 * containing or covering the disk, so they are the sub-interval breakpoints of
 * the contains/covers kernels. Unlike #tcbufferseg_touch_roots this keeps the
 * internal tangency, where the disk grazes the boundary from within and
 * containment changes but no touch occurs. Returns the number of times written
 * (at most @p maxout)
 */
/**
 * @brief Append to @p outt the interior times in (0,1) at which a linearly
 * moving disk starts or stops containing/covering (or being contained/covered
 * by) the static disk of a disc context
 * @details Over the segment the moving disk is P(t)=(x0+t dx, y0+t dy),
 * R(t)=r0+t dr. Containment flips where the clearance g(t)=||P(t)-C||-thr(t)
 * vanishes, thr(t)=m+s t being R(t)-rc (temporal container) or rc-R(t) (static
 * container). Squaring the non-negative ||P(t)-C|| gives the quadratic
 * Q(t)=(a-s^2)t^2+(b-2ms)t+(c-m^2); a root is a genuine clearance zero only
 * where thr>=0, which discards the spurious branch introduced by squaring.
 */
static int
tcbuffer_disc_seg_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const TcbufferDiscCtx *d, double *outt, int maxout)
{
  double x0 = cb1->x, y0 = cb1->y, r0 = cb1->radius;
  double dx = cb2->x - cb1->x, dy = cb2->y - cb1->y, dr = cb2->radius - r0;
  double cx = d->cb.x, cy = d->cb.y, rc = d->cb.radius;
  double m, s;
  if (d->container_is_temporal) { m = r0 - rc; s = dr; }
  else { m = rc - r0; s = -dr; }
  double ex = x0 - cx, ey = y0 - cy;
  double a = dx * dx + dy * dy, b = 2 * (ex * dx + ey * dy), c = ex * ex + ey * ey;
  double A = a - s * s, B = b - 2 * m * s, C = c - m * m;
  double cand[2];
  int ncand = 0;
  if (fabs(A) > MEOS_GEOM_TOLERANCE)
  {
    double disc = B * B - 4 * A * C;
    if (disc >= 0)
    {
      double sq = sqrt(disc);
      cand[ncand++] = (-B - sq) / (2 * A);
      cand[ncand++] = (-B + sq) / (2 * A);
    }
  }
  else if (fabs(B) > MEOS_GEOM_TOLERANCE)
    cand[ncand++] = -C / B;
  int n = 0;
  for (int i = 0; i < ncand && n < maxout; i++)
  {
    double t = cand[i];
    if (t > 0.0 && t < 1.0 && (m + s * t) >= - TCBUFFER_TOUCH_EPS)
      outt[n++] = t;
  }
  return n;
}

int
tcbufferseg_boundary_roots(const Cbuffer *cb1, const Cbuffer *cb2,
  const void *ctxv, double *outt, int maxout)
{
  if (*(const int *) ctxv == TCBUF_CTX_DISC)
    return tcbuffer_disc_seg_roots(cb1, cb2, (const TcbufferDiscCtx *) ctxv,
      outt, maxout);
  return tcbufferseg_sg_roots(cb1, cb2, ctxv, outt, maxout, false);
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
 * @csqlfn #NAD_cbuffer_stbox() #NAD_stbox_cbuffer()
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
 * @csqlfn #NAD_tcbuffer_geo() #NAD_geo_tcbuffer()
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
 * @csqlfn #NAD_tcbuffer_stbox() #NAD_stbox_tcbuffer()
 */
double
nad_tcbuffer_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_stbox(temp, box))
    return DBL_MAX;

  /* Project the temporal value to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  Temporal *temp1 = (Temporal *) temp;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
    temp1 = temporal_restrict_tstzspan(temp, &inter, REST_AT);
    /* The two spans meet while no value lies inside the intersection, which a
     * discrete or a step value can do */
    if (! temp1)
      return DBL_MAX;
  }

  GSERIALIZED *trav = tcbuffer_traversed_area(temp1, false);
  GSERIALIZED *geo = stbox_geo(box);
  double result = geom_distance2d(trav, geo);
  pfree(trav); pfree(geo);
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_cbuffer_dist
 * @brief Return the nearest approach distance of a temporal circular buffer
 * and a circular buffer
 * @param[in] temp Temporal circular buffer
 * @param[in] cb Circular buffer
 * @csqlfn #NAD_tcbuffer_cbuffer() #NAD_cbuffer_tcbuffer()
 */
double
nad_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return DBL_MAX;

  /* A static disc is a constant temporal circular buffer, so the nearest
   * approach distance is the exact plane-sweep spatial minimum the two-temporal
   * kernel already computes, which avoids materialising the traversed area for
   * every pair. */
  Temporal *ctemp = tcbuffer_from_base_temp(cb, temp);
  double result = mindistance_tcbuffer_tcbuffer(temp, ctemp, DBL_MAX);
  pfree(ctemp);
  return result;
}

/**
 * @brief Return a lower bound on the distance between two synchronized linear
 * temporal circular buffer segments
 * @details A moving disc stays within the bounding box of its two endpoint
 * centres, so the box-to-box distance of the centre boxes minus the larger
 * endpoint radius of each disc bounds the signed gap below. The bound is
 * signed (negative when the discs may overlap), matching #cbuffer_distance
 */
static double
tcbufferseg_distance_lb(Datum start1, Datum end1, Datum start2, Datum end2)
{
  const Cbuffer *cs1 = DatumGetCbufferP(start1);
  const Cbuffer *ce1 = DatumGetCbufferP(end1);
  const Cbuffer *cs2 = DatumGetCbufferP(start2);
  const Cbuffer *ce2 = DatumGetCbufferP(end2);
  const POINT2D *s1 = cbuffer_point2d_p(cs1);
  const POINT2D *e1 = cbuffer_point2d_p(ce1);
  const POINT2D *s2 = cbuffer_point2d_p(cs2);
  const POINT2D *e2 = cbuffer_point2d_p(ce2);
  double r1 = fmax(cs1->radius, ce1->radius);
  double r2 = fmax(cs2->radius, ce2->radius);
  return sqrt(box2d_distance_sqr(
    fmin(s1->x, e1->x), fmin(s1->y, e1->y),
    fmax(s1->x, e1->x), fmax(s1->y, e1->y),
    fmin(s2->x, e2->x), fmin(s2->y, e2->y),
    fmax(s2->x, e2->x), fmax(s2->y, e2->y))) - r1 - r2;
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

  /* Fast path: linear temporal circular buffers via the time-synchronous
   * running minimum, avoiding the temporal distance materialization. A finite
   * result is exact; the infinity sentinel (empty or degenerate overlap)
   * defers to the temporal distance path */
  if (nad_tcont_tcont_sync_applies(temp1, temp2))
  {
    TimestampTz t;
    double d = nad_tcont_tcont_sync(temp1, temp2, &datum_cbuffer_distance,
      &tcbuffersegm_distance_turnpt, &tcbufferseg_distance_lb, &t);
    if (d != DBL_MAX)
      return d;
  }

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

/**
 * @brief
 */
typedef struct
{
  int idx;
  float minx;
  float maxx;
  float miny;
  float maxy;
} TcbufferSegBox;

/**
 * @brief
 */
static int
tcbuffersegbox_cmp_minx(const void *a, const void *b)
{
  float da = ((const TcbufferSegBox *) a)->minx;
  float db = ((const TcbufferSegBox *) b)->minx;
  return (da < db) ? -1 : (da > db ? 1 : 0);
}

/**
 * @brief Append candidate value of f(s,t) on edge s=0 (in t) given:
 *   e = |E|^2 with E = A - C
 *   q = E . V
 *   v = |V|^2
 *   dr2 = r_D - r_C
 *   r_sum = r_A + r_C  (radius offset at t=0)
 * Computes the critical point of g(t) = sqrt(e - 2tq + t^2 v) - r_sum - t dr2
 * via the quadratic v(v - dr2^2) t^2 - 2 q (v - dr2^2) t + (q^2 - dr2^2 e) = 0.
 * Only v > dr2^2 yields a local minimum; the v == dr2^2 inflection case is
 * skipped (covered by corners).
 */
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
  /* Same reduction as the sibling in s: 4 diff dr2^2 (v e - q^2), which the
   * Cauchy-Schwarz slack of E and V keeps non-negative */
  double disc = 4.0 * diff * dr2sq * (v * e - q * q);
  if (disc < 0.0)
    disc = 0.0;
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

/**
 * @brief Append candidate value of f(s,t) on edge t=0 (in s) given:
 *   e = |E|^2 with E = A - C
 *   p = E . U
 *   u = |U|^2
 *   dr1 = r_B - r_A
 *   r_sum = r_A + r_C
 * Critical point of g(s) = sqrt(e + 2sp + s^2 u) - r_sum - s dr1, derivative
 * (us + p)/sqrt(...) - dr1 = 0; analogous quadratic.
 */
static void
cbuffersegm_edge_crit_in_s(double e, double p, double u, double dr1,
  double r_sum, double *best)
{
  double dr1sq = dr1 * dr1;
  if (u <= dr1sq)
    return;
  double diff = u - dr1sq;
  double aa = u * diff;
  double bb = 2.0 * p * diff;
  /* The discriminant reduces to 4 diff dr1^2 (u e - p^2), which is never
   * negative: diff is positive above, and u e - p^2 is the Cauchy-Schwarz
   * slack of E and U.  Taking it in that form rather than as bb^2 - 4 aa cc
   * keeps the two products from cancelling: with a radius that holds over the
   * segment, dr1 is zero and the subtraction of two equal quantities rounds
   * to a small negative, which reads as no critical point and leaves the
   * minimum of the segment at whichever endpoint is nearer */
  double disc = 4.0 * diff * dr1sq * (u * e - p * p);
  if (disc < 0.0)
    disc = 0.0;
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

/**
 * @brief Exact min spatial distance between two cbuffer segments,
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

/**
 * @brief Plane-sweep over two cbuffer sequences.
 */
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
    const POINT2D *pa = cbuffer_point2d_p(cb_a);
    const POINT2D *pb = cbuffer_point2d_p(cb_b);
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
    const POINT2D *pa1 = cbuffer_point2d_p(cb_a1);
    const POINT2D *pb1 = cbuffer_point2d_p(cb_b1);
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
      const POINT2D *pa2 = cbuffer_point2d_p(cb_a2);
      const POINT2D *pb2 = cbuffer_point2d_p(cb_b2);
      double d = cbuffersegm_segm_mindist(pa1, cb_a1->radius, pb1, cb_b1->radius,
        pa2, cb_a2->radius, pb2, cb_b2->radius, best);
      if (d < best) best = d;
      if (best == 0.0) { pfree(boxes2); return 0.0; }
    }
  }
  pfree(boxes2);
  return best;
}

/**
 * @brief Subtype dispatch.
 */
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
  /* Ensure the validity of the arguments */
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
 * @csqlfn #Shortestline_tcbuffer_geo() #Shortestline_geo_tcbuffer()
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
  /* A geometry that has no edge decomposition: exact traversed-area
   * shortest line */
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
 * @csqlfn #Shortestline_tcbuffer_cbuffer() #Shortestline_cbuffer_tcbuffer()
 */
GSERIALIZED *
shortestline_tcbuffer_cbuffer(const Temporal *temp, const Cbuffer *cb)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_cbuffer(temp, cb))
    return NULL;

  /* A static disc is a constant temporal circular buffer, so the line is the
   * one the two-temporal kernel computes in closed form from the two disks.
   * Rendering the disc and the traversed area as polygons instead measures
   * between two polygonal approximations, and the line it returns is not the
   * one realising the nearest approach distance of the same pair. */
  Temporal *ctemp = tcbuffer_from_base_temp(cb, temp);
  GSERIALIZED *result = shortestline_tcbuffer_tcbuffer(temp, ctemp);
  pfree(ctemp);
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

  /* The nearest approach of two moving disks is attained at the instant where
   * the temporal distance is least, that distance honouring both radii and the
   * turning points. Read the two disks there and the line follows in closed
   * form: it runs along the line of centres, leaving the boundary of one disk
   * and reaching the boundary of the other, so its length is the nearest
   * approach distance. Reading the centres instead answers a different
   * question, one whose line is longer than the distance it is supposed to
   * realise by exactly the two radii. */
  TimestampTz t;
  bool found = false;
  if (nad_tcont_tcont_sync_applies(temp1, temp2))
    found = (nad_tcont_tcont_sync(temp1, temp2, &datum_cbuffer_distance,
      &tcbuffersegm_distance_turnpt, &tcbufferseg_distance_lb, &t) != DBL_MAX);
  if (! found)
  {
    Temporal *dist = tdistance_tcbuffer_tcbuffer(temp1, temp2);
    if (! dist)
      return NULL;
    const TInstant *min = temporal_min_inst_p((const Temporal *) dist);
    t = min->t;
    pfree(dist);
  }

  /* The closest point may be at an exclusive bound */
  Datum value1, value2;
  if (! temporal_value_at_timestamptz(temp1, t, false, &value1) ||
      ! temporal_value_at_timestamptz(temp2, t, false, &value2))
    return NULL;
  const Cbuffer *cb1 = DatumGetCbufferP(value1);
  const Cbuffer *cb2 = DatumGetCbufferP(value2);
  const POINT2D *c1 = cbuffer_point2d_p(cb1);
  const POINT2D *c2 = cbuffer_point2d_p(cb2);
  double vx = c2->x - c1->x, vy = c2->y - c1->y;
  double vl = sqrt(vx * vx + vy * vy);
  double px, py, qx, qy;
  if (vl <= MEOS_EPSILON || vl <= cb1->radius + cb2->radius)
  {
    /* Concentric or overlapping disks meet, and the line degenerates to the
     * point where the boundary of the first reaches the second, clamped to the
     * centre when there is no direction to take */
    double f = (vl <= MEOS_EPSILON) ? 0.0 : cb1->radius / vl;
    if (f > 1.0) f = 1.0;
    px = qx = c1->x + vx * f;
    py = qy = c1->y + vy * f;
  }
  else
  {
    px = c1->x + vx * (cb1->radius / vl);
    py = c1->y + vy * (cb1->radius / vl);
    qx = c2->x - vx * (cb2->radius / vl);
    qy = c2->y - vy * (cb2->radius / vl);
  }
  int32_t srid = tspatial_srid(temp1);
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2));

  POINTARRAY *pa = ptarray_construct(0, 0, 2);
  POINT4D p4;
  p4.z = 0.0; p4.m = 0.0;
  p4.x = px; p4.y = py;
  ptarray_set_point4d(pa, 0, &p4);
  p4.x = qx; p4.y = qy;
  ptarray_set_point4d(pa, 1, &p4);
  LWLINE *ln = lwline_construct(srid, NULL, pa);
  GSERIALIZED *line = geo_serialize(lwline_as_lwgeom(ln));
  lwline_free(ln);
  return line;
}

/*****************************************************************************/
