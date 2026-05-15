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
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"

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

    /* Check if the turning points are truly internal */
    if (t_in > lower && t_out < upper)
    {
      *t1 = t_in;
      *t2 = t_out;
      return 2;
    }
    else if (t_in > lower && t_out >= upper)
    {
      *t1 = *t2 = t_in;
      return 1;
    }
    else if (t_in <= lower && t_out < upper)
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

    /* Check if the turning points are truly internal */
    if (t_in > lower && t_out < upper)
    {
      *t1 = t_in;
      *t2 = t_out;
      return 2;
    }
    else if (t_in > lower && t_out >= upper)
    {
      *t1 = *t2 = t_in;
      return 1;
    }
    else if (t_in <= lower && t_out < upper)
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
/**
 * @brief Return the 2D distance between two axis-aligned bounding boxes,
 * 0 when they overlap. This is a lower bound for the distance between any
 * geometries the boxes contain.
 */
static double
box2d_distance(double axmin, double aymin, double axmax, double aymax,
  double bxmin, double bymin, double bxmax, double bymax)
{
  double dx = fmax(fmax(axmin - bxmax, bxmin - axmax), 0.0);
  double dy = fmax(fmax(aymin - bymax, bymin - aymax), 0.0);
  return sqrt(dx * dx + dy * dy);
}

/**
 * @brief Set the 2D bounding box of the circular buffer value of a temporal
 * instant (the buffer centre expanded by its radius)
 */
static void
tcbufferinst_xybox(const TInstant *inst, double *xmin, double *ymin,
  double *xmax, double *ymax)
{
  const Cbuffer *cb = DatumGetCbufferP(tinstant_value_p(inst));
  const POINT2D *p = GSERIALIZED_POINT2D_P(cbuffer_point_p(cb));
  double r = cb->radius;
  *xmin = p->x - r; *xmax = p->x + r;
  *ymin = p->y - r; *ymax = p->y + r;
}

/**
 * @brief Accumulate one traversed-area unit into the running nearest
 * approach. Takes ownership of @p unit: keeps it as the best geometry when
 * @p line is requested, otherwise frees it.
 */
static void
tcbuffer_geo_nad_accum(GSERIALIZED *unit, const GSERIALIZED *gs,
  bool want_line, double *best, GSERIALIZED **best_geom)
{
  double d = geom_distance2d(unit, gs);
  if (d < *best)
  {
    *best = d;
    if (want_line)
    {
      if (*best_geom)
        pfree(*best_geom);
      *best_geom = unit;
      return;
    }
  }
  pfree(unit);
}

/**
 * @brief Update the running nearest approach between the segments of a
 * temporal circular buffer sequence and a geometry, skipping segments whose
 * bounding box is provably farther than the current best
 */
static void
tcbufferseq_geo_nad(const TSequence *seq, const GSERIALIZED *gs,
  double gxmin, double gymin, double gxmax, double gymax, bool want_line,
  double *best, GSERIALIZED **best_geom)
{
  /* Instantaneous sequence or non-linear interpolation: one circle per
   * instant (this is exactly the traversed area of those cases) */
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      double uxmin, uymin, uxmax, uymax;
      tcbufferinst_xybox(inst, &uxmin, &uymin, &uxmax, &uymax);
      if (box2d_distance(uxmin, uymin, uxmax, uymax, gxmin, gymin, gxmax,
          gymax) >= *best)
        continue;
      tcbuffer_geo_nad_accum(tcbufferinst_trav_area(inst), gs, want_line,
        best, best_geom);
      if (*best == 0.0)
        return;
    }
    return;
  }

  /* Linear interpolation: one swept capsule per segment */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  double b1xmin, b1ymin, b1xmax, b1ymax;
  tcbufferinst_xybox(inst1, &b1xmin, &b1ymin, &b1xmax, &b1ymax);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    double b2xmin, b2ymin, b2xmax, b2ymax;
    tcbufferinst_xybox(inst2, &b2xmin, &b2ymin, &b2xmax, &b2ymax);
    /* The swept capsule of a linearly moving, linearly resized disk is
     * contained in the union of the two endpoint disk boxes */
    double uxmin = fmin(b1xmin, b2xmin), uymin = fmin(b1ymin, b2ymin);
    double uxmax = fmax(b1xmax, b2xmax), uymax = fmax(b1ymax, b2ymax);
    if (box2d_distance(uxmin, uymin, uxmax, uymax, gxmin, gymin, gxmax,
        gymax) < *best)
    {
      tcbuffer_geo_nad_accum(tcbuffersegm_trav_area(inst1, inst2), gs,
        want_line, best, best_geom);
      if (*best == 0.0)
        return;
    }
    inst1 = inst2;
    b1xmin = b2xmin; b1ymin = b2ymin; b1xmax = b2xmax; b1ymax = b2ymax;
  }
}

/**
 * @brief Return the nearest approach distance between a temporal circular
 * buffer and a geometry and, when requested, the shortest line
 * @details The traversed area is a collection of per-segment swept capsules,
 * so the distance to it equals the minimum of the per-segment distances and
 * the shortest line is the one to the nearest segment. Segments whose
 * bounding box is farther than the running best are skipped without building
 * their geometry, which avoids materialising the whole traversed area.
 */
static double
tcbuffer_geo_nad(const Temporal *temp, const GSERIALIZED *gs,
  bool want_line, GSERIALIZED **line)
{
  STBox gbox;
  geo_set_stbox(gs, &gbox);
  double best = DBL_MAX;
  GSERIALIZED *best_geom = NULL;

  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    double uxmin, uymin, uxmax, uymax;
    tcbufferinst_xybox((TInstant *) temp, &uxmin, &uymin, &uxmax, &uymax);
    if (box2d_distance(uxmin, uymin, uxmax, uymax, gbox.xmin, gbox.ymin,
        gbox.xmax, gbox.ymax) < best)
      tcbuffer_geo_nad_accum(tcbufferinst_trav_area((TInstant *) temp), gs,
        want_line, &best, &best_geom);
  }
  else if (temp->subtype == TSEQUENCE)
    tcbufferseq_geo_nad((TSequence *) temp, gs, gbox.xmin, gbox.ymin,
      gbox.xmax, gbox.ymax, want_line, &best, &best_geom);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && best > 0.0; i++)
      tcbufferseq_geo_nad(TSEQUENCESET_SEQ_N(ss, i), gs, gbox.xmin,
        gbox.ymin, gbox.xmax, gbox.ymax, want_line, &best, &best_geom);
  }

  if (want_line)
  {
    *line = best_geom ? geom_shortestline2d(best_geom, gs) : NULL;
    if (best_geom)
      pfree(best_geom);
  }
  return best;
}

double
nad_tcbuffer_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tcbuffer_geo(temp, gs) || gserialized_is_empty(gs))
    return DBL_MAX;

  return tcbuffer_geo_nad(temp, gs, false, NULL);
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

  GSERIALIZED *geo = stbox_geo(box);
  double result = tcbuffer_geo_nad(temp, geo, false, NULL);
  pfree(geo);
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
  double result = tcbuffer_geo_nad(temp, geom, false, NULL);
  pfree(geom);
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

  GSERIALIZED *result = NULL;
  tcbuffer_geo_nad(temp, gs, true, &result);
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
  GSERIALIZED *result = NULL;
  tcbuffer_geo_nad(temp, geom, true, &result);
  pfree(geom);
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
