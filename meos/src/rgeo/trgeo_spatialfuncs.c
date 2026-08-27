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
 * @brief Spatial restriction functions for temporal rigid geometries
 *
 * The restriction is exact for the body extent: a `trgeometry` is "in" a
 * geometry / STBox at time `t` iff its posed reference polygon (the whole
 * body, not merely its centre point) overlaps that geometry / STBox at `t`.
 * The overlap intervals are computed per trgeometry segment from the
 * swept-edge clip primitive (`trgeo_geom_clip_*`): every time at which a
 * moving body edge touches the target boundary is a candidate state-change,
 * and each inter-event sub-interval is classified by an exact GEOS
 * polygon-overlap midpoint test. This is exact for pure-translation
 * segments. Rotating segments and target geometries with interior rings
 * (holes) raise an honest `FEATURE_NOT_SUPPORTED` error instead of silently
 * reducing to the centre point.
 */

/* PostgreSQL */
#include <postgres.h>
#include <float.h>
#include <math.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_rgeo.h>
#include "geo/geo_funcs.h"
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_geom_clip.h"
#include "rgeo/trgeo_inst.h"
#include "rgeo/trgeo_spatialfuncs.h"
#include "rgeo/trgeo_utils.h"
#include "pose/pose.h"
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/temporal_analytics.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Exact body-overlap interval computation
 *
 * Given a trgeometry and a static target geometry, compute the set of
 * timestamps during which the posed reference polygon (the whole body)
 * overlaps the target. The computation is performed per consecutive-instant
 * segment and is exact for pure translation:
 *
 *   - Critical events: every time at which a moving body edge touches the
 *     target boundary. These are exactly the interval endpoints returned by
 *     the swept-edge clip primitive across all body edges (the clip captures
 *     moving-edge-endpoint-on-target-edge, target-vertex-on-moving-edge, and
 *     moving-endpoint-inside-target events). The overlap state
 *     `intersects(B(t), target)` is constant between consecutive events.
 *   - Classification: each inter-event sub-interval is classified by an
 *     exact GEOS polygon-overlap test (`geom_intersects2d`) at its midpoint,
 *     applied to the materialised world body. This correctly handles full
 *     containment in either direction, where no boundary touch occurs.
 *
 * A segment whose two endpoint poses differ in rotation by more than
 * FP tolerance, or a target geometry carrying interior rings (holes,
 * ignored by the M1 clip), raises an honest FEATURE_NOT_SUPPORTED error.
 *****************************************************************************/


/**
 * @brief Return true if any polygon component of an LWGEOM carries an
 * interior ring (hole). The M1 clip primitive ignores holes, so a holey
 * target is rejected rather than silently treated as solid.
 */
static bool
lwgeom_has_interior_ring(const LWGEOM *geom)
{
  if (! geom)
    return false;
  switch (geom->type)
  {
    case POLYGONTYPE:
      return ((const LWPOLY *) geom)->nrings > 1;
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        if (lwgeom_has_interior_ring(coll->geoms[i]))
          return true;
      return false;
    }
    default:
      return false;
  }
}

/**
 * @brief Append to @p events the per-body-edge clip-interval endpoints (in
 * segment-local parameter t in [0, 1]) for a pure-translation body segment.
 *
 * The body translates by @p dx, @p dy with fixed orientation. Each exterior
 * ring edge of the (already world-posed at t = 0) body @p body0 sweeps a
 * parallelogram; the clip primitive returns the times at which that edge
 * touches the target boundary.
 */
static void
collect_edge_events_m1(const LWGEOM *body0, double dx, double dy,
  const LWGEOM *target, double **events, int *nevents, int *cap)
{
  if (! body0)
    return;
  switch (body0->type)
  {
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) body0;
      /* Exterior ring only: the body interior is solid for overlap. */
      const POINTARRAY *pa = poly->rings[0];
      for (uint32_t k = 0; k + 1 < pa->npoints; k++)
      {
        const POINT2D *p0 = getPoint2d_cp(pa, k);
        const POINT2D *p1 = getPoint2d_cp(pa, k + 1);
        POINT2D a1 = *p0, b1 = *p1;
        POINT2D a2 = { p0->x + dx, p0->y + dy };
        POINT2D b2 = { p1->x + dx, p1->y + dy };
        Span *iv = NULL;
        int n = trgeo_geom_clip_lwgeom(&a1, &b1, &a2, &b2, target, &iv);
        for (int m = 0; m < n; m++)
        {
          if (*nevents + 2 > *cap)
          {
            *cap = (*nevents + 2) * 2 + 8;
            *events = repalloc(*events, (size_t) *cap * sizeof(double));
          }
          (*events)[(*nevents)++] = DatumGetFloat8(iv[m].lower);
          (*events)[(*nevents)++] = DatumGetFloat8(iv[m].upper);
        }
        if (iv)
          pfree(iv);
      }
      break;
    }
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) body0;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        collect_edge_events_m1(coll->geoms[i], dx, dy, target, events,
          nevents, cap);
      break;
    }
    default:
      break;
  }
}

static int
restrict_double_cmp(const void *a, const void *b)
{
  double da = *(const double *) a, db = *(const double *) b;
  return (da < db) ? -1 : (da > db) ? 1 : 0;
}

/**
 * @brief Compute, for one pure-translation segment, the absolute-time spans
 * during which the posed body overlaps the target, appending them to the
 * dynamic span buffer (@p spans, @p nspans, @p spancap).
 *
 * @param[in] ref Body reference geometry (body frame)
 * @param[in] pose1,pose2 Segment endpoint poses (same orientation)
 * @param[in] t1,t2 Segment endpoint timestamps
 * @param[in] target Static target geometry
 * @param[in] lwtarget LWGEOM view of @p target (for the clip primitive)
 * @return true on success, false if the error handler returned (error set)
 */
/**
 * @brief Return true where a body meets a box only along the borders the box
 * leaves out
 * @details A box of a grid is half-open, `[xmin,xmax) x [ymin,ymax)`, so the
 * tiles either side of a grid line claim a value lying on that line exactly
 * once. The overlap of two geometries is a closed test and answers true for a
 * body that merely touches, so a body every point of which stands at or beyond
 * one of the two upper borders meets the closed box while meeting no point of
 * the half-open one
 * @param[in] body Body, already placed in the world
 * @param[in] box Box whose upper borders are left out
 * @note A body reaching past an upper border and back is admitted, so this
 * decides exactly the case where the contact is confined to a border and never
 * withholds a box the body genuinely reaches
 */
static bool
body_on_excluded_border(const GSERIALIZED *body, const STBox *box)
{
  STBox bbox;
  if (! geo_set_stbox(body, &bbox))
    return false;
  return (bbox.xmin >= box->xmax) || (bbox.ymin >= box->ymax);
}

/**
 * @brief Return whether a body meets a target, leaving out the upper borders
 * of @p excl where it is given
 * @param[in] body Body, already placed in the world
 * @param[in] target Target geometry
 * @param[in] excl Box whose upper borders are left out, NULL to meet the
 * target as a closed region
 */
static bool
body_meets_target(const GSERIALIZED *body, const GSERIALIZED *target,
  const STBox *excl)
{
  if (! geom_intersects2d(body, target))
    return false;
  return excl ? ! body_on_excluded_border(body, excl) : true;
}

static bool
segment_overlap_spans(const GSERIALIZED *ref, const Pose *pose1,
  const Pose *pose2, TimestampTz t1, TimestampTz t2,
  const GSERIALIZED *target, const LWGEOM *lwtarget, const STBox *excl,
  Span **spans, int *nspans, int *spancap)
{
  /* Pure translation: orientation is fixed over the segment (the caller has
   * already verified theta1 == theta2 within tolerance). */
  double dx = pose2->data[0] - pose1->data[0];
  double dy = pose2->data[1] - pose1->data[1];

  /* World-posed body at the segment start. */
  GSERIALIZED *body0_gs = pose_apply_geo(pose1, ref);
  LWGEOM *body0 = lwgeom_from_gserialized(body0_gs);

  /* Critical events: clip-interval endpoints across all body edges. */
  int cap = 16;
  double *events = palloc((size_t) cap * sizeof(double));
  int nevents = 0;
  events[nevents++] = 0.0;
  events[nevents++] = 1.0;
  collect_edge_events_m1(body0, dx, dy, lwtarget, &events, &nevents, &cap);

  qsort(events, (size_t) nevents, sizeof(double), restrict_double_cmp);
  /* Deduplicate. */
  int nuniq = 0;
  for (int k = 0; k < nevents; k++)
    if (nuniq == 0 ||
        fabs(events[k] - events[nuniq - 1]) > MEOS_GEOM_TOLERANCE)
      events[nuniq++] = events[k];

  /* Classify each inter-event sub-interval by an exact GEOS overlap test at
   * its midpoint, applied to the materialised world body. Merge adjacent
   * overlapping sub-intervals. */
  bool prev_overlap = false;
  double run_start = 0.0;
  for (int k = 0; k < nuniq - 1; k++)
  {
    double ta = events[k], tb = events[k + 1];
    if (tb - ta <= MEOS_GEOM_TOLERANCE)
      continue;
    double tm = 0.5 * (ta + tb);
    Pose *posem = posesegm_interpolate(pose1, pose2, tm);
    GSERIALIZED *bodym = pose_apply_geo(posem, ref);
    bool ov = body_meets_target(bodym, target, excl);
    pfree(bodym);
    pfree(posem);
    if (ov && ! prev_overlap)
    {
      run_start = ta;
      prev_overlap = true;
    }
    else if (! ov && prev_overlap)
    {
      /* Emit [run_start, ta] in absolute time. */
      if (*nspans + 1 > *spancap)
      {
        *spancap = (*nspans + 1) * 2 + 8;
        *spans = repalloc(*spans, (size_t) *spancap * sizeof(Span));
      }
      TimestampTz lo = t1 + (TimestampTz) llround(run_start * (double) (t2 - t1));
      TimestampTz hi = t1 + (TimestampTz) llround(ta * (double) (t2 - t1));
      span_set(TimestampTzGetDatum(lo), TimestampTzGetDatum(hi),
        true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &(*spans)[*nspans]);
      (*nspans)++;
      prev_overlap = false;
    }
  }
  if (prev_overlap)
  {
    if (*nspans + 1 > *spancap)
    {
      *spancap = (*nspans + 1) * 2 + 8;
      *spans = repalloc(*spans, (size_t) *spancap * sizeof(Span));
    }
    TimestampTz lo = t1 + (TimestampTz) llround(run_start * (double) (t2 - t1));
    span_set(TimestampTzGetDatum(lo), TimestampTzGetDatum(t2),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &(*spans)[*nspans]);
    (*nspans)++;
  }

  pfree(events);
  lwgeom_free(body0);
  pfree(body0_gs);
  return true;
}

/**
 * @brief Test the posed body overlap at a single instant timestamp,
 * appending an instantaneous span to the buffer if it overlaps.
 */
static void
instant_overlap_span(const GSERIALIZED *ref, const Pose *pose, TimestampTz t,
  const GSERIALIZED *target, const STBox *excl, Span **spans, int *nspans,
  int *spancap)
{
  GSERIALIZED *body = pose_apply_geo(pose, ref);
  bool ov = body_meets_target(body, target, excl);
  pfree(body);
  if (! ov)
    return;
  if (*nspans + 1 > *spancap)
  {
    *spancap = (*nspans + 1) * 2 + 8;
    *spans = repalloc(*spans, (size_t) *spancap * sizeof(Span));
  }
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &(*spans)[*nspans]);
  (*nspans)++;
}

/**
 * @brief Compute the timestamps at which a trgeometry body overlaps a target
 * geometry, returning a tstzspanset (or NULL if no overlap).
 *
 * Iterates the trgeometry instants. Each linear segment is required to be a
 * pure translation; any rotating segment raises FEATURE_NOT_SUPPORTED. The
 * caller has already rejected holey targets.
 *
 * @return The overlap spanset on success. On error (rotation) the MEOS error
 * handler is invoked; @p *err is set to true and NULL is returned.
 */
static SpanSet *
trgeo_overlap_spanset(const Temporal *temp, const GSERIALIZED *gs,
  const LWGEOM *lwgs, const STBox *excl, bool *err)
{
  *err = false;
  const GSERIALIZED *ref = trgeo_geom_p(temp);
  int spancap = 16, nspans = 0;
  Span *spans = palloc((size_t) spancap * sizeof(Span));

  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    instant_overlap_span(ref, DatumGetPoseP(tinstant_value_p(inst)),
      inst->t, gs, excl, &spans, &nspans, &spancap);
  }
  else
  {
    /* Collect the sequences to iterate. */
    int nseqs;
    const TSequence **seqs;
    const TSequence *one;
    if (temp->subtype == TSEQUENCE)
    {
      one = (const TSequence *) temp;
      seqs = &one;
      nseqs = 1;
    }
    else
    {
      const TSequenceSet *sset = (const TSequenceSet *) temp;
      nseqs = sset->count;
      seqs = palloc((size_t) nseqs * sizeof(TSequence *));
      for (int i = 0; i < nseqs; i++)
        ((const TSequence **) seqs)[i] = TSEQUENCESET_SEQ_N(sset, i);
    }
    bool linear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
    for (int s = 0; s < nseqs; s++)
    {
      const TSequence *seq = seqs[s];
      if (seq->count == 1)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
        instant_overlap_span(ref, DatumGetPoseP(tinstant_value_p(inst)),
          inst->t, gs, excl, &spans, &nspans, &spancap);
        continue;
      }
      if (! linear)
      {
        /* Step interpolation: each instant holds its pose (the body is static)
         * over [t_i, t_{i+1}); if the held body overlaps the target, the whole
         * step interval overlaps. The last instant contributes an instant. */
        for (int i = 0; i < seq->count; i++)
        {
          const TInstant *inst = TSEQUENCE_INST_N(seq, i);
          const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
          GSERIALIZED *body = pose_apply_geo(pose, ref);
          bool ov = body_meets_target(body, gs, excl);
          pfree(body);
          if (! ov)
            continue;
          if (nspans + 1 > spancap)
          {
            spancap = (nspans + 1) * 2 + 8;
            spans = repalloc(spans, (size_t) spancap * sizeof(Span));
          }
          TimestampTz hi = (i + 1 < seq->count) ?
            TSEQUENCE_INST_N(seq, i + 1)->t : inst->t;
          bool upper_inc = (i + 1 < seq->count) ? false : true;
          span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(hi),
            true, upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &spans[nspans]);
          nspans++;
        }
        continue;
      }
      for (int i = 0; i + 1 < seq->count; i++)
      {
        const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
        const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
        const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
        const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
        /* Pure-translation guard: any rotation is honestly not implemented. */
        if (fabs(pose2->data[2] - pose1->data[2]) > MEOS_GEOM_TOLERANCE)
        {
          pfree(spans);
          if (temp->subtype == TSEQUENCESET)
            pfree(seqs);
          *err = true;
          meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
            "Exact spatial restriction of a temporal rigid geometry with a "
            "rotating segment is not implemented");
          return NULL;
        }
        if (! segment_overlap_spans(ref, pose1, pose2, inst1->t, inst2->t,
            gs, lwgs, excl, &spans, &nspans, &spancap))
        {
          pfree(spans);
          if (temp->subtype == TSEQUENCESET)
            pfree(seqs);
          *err = true;
          return NULL;
        }
      }
    }
    if (temp->subtype == TSEQUENCESET)
      pfree(seqs);
  }

  if (nspans == 0)
  {
    pfree(spans);
    return NULL;
  }
  /* spanset_make_free normalises (sorts + merges adjacent/overlapping spans)
   * and takes ownership of the spans array. */
  return spanset_make_free(spans, nspans, NORMALIZE, ORDER);
}

/**
 * @brief Restrict a temporal rigid geometry to (the complement of) a set of
 * timestamps, preserving the appended reference geometry.
 * @details The generic temporal restriction operates on the pose component
 * and drops the reference geometry; this re-appends it so the result is a
 * well-formed temporal rigid geometry.
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Set of timestamps (overlap intervals); when NULL, an empty
 * restriction is assumed
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
static Temporal *
trgeo_restrict_overlap(const Temporal *temp, const SpanSet *ss, bool atfunc)
{
  if (! ss)
    /* No overlap: at -> empty, minus -> whole. */
    return atfunc ? NULL : temporal_copy(temp);
  /* Restrict the pose component, then re-append the reference geometry: the
   * generic temporal restriction operates on the pose and drops the geometry. */
  Temporal *tpose = trgeometry_to_tpose(temp);
  Temporal *res = temporal_restrict_tstzspanset(tpose, ss, atfunc);
  pfree(tpose);
  if (! res)
    return NULL;
  Temporal *result = geometry_tpose_to_trgeometry(trgeo_geom_p(temp), res);
  pfree(res);
  return result;
}

/*****************************************************************************
 * Restriction by geometry
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to (the complement
 * of) a geometry
 * @details The restriction is exact on the body extent for pure-translation
 * trgeometries. Rotating segments and holey targets raise an honest
 * FEATURE_NOT_SUPPORTED error.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
trgeo_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  LWGEOM *lwgs = lwgeom_from_gserialized(gs);
  /* The M1 clip ignores polygon holes; reject holey targets honestly. */
  if (lwgeom_has_interior_ring(lwgs))
  {
    lwgeom_free(lwgs);
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Exact spatial restriction of a temporal rigid geometry against a "
      "geometry with interior rings (holes) is not implemented");
    return NULL;
  }

  bool err;
  /* A geometry is met as a closed region: only a box of a grid leaves its
   * upper borders out */
  SpanSet *ss = trgeo_overlap_spanset(temp, gs, lwgs, NULL, &err);
  lwgeom_free(lwgs);
  if (err)
    return NULL;

  Temporal *result = trgeo_restrict_overlap(temp, ss, atfunc);
  if (ss)
    pfree(ss);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Trgeometry_at_geom()
 */
Temporal *
trgeometry_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Trgeometry_minus_geom()
 */
Temporal *
trgeometry_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_geom(temp, gs, REST_MINUS);
}

/*****************************************************************************
 * Restriction by spatiotemporal box
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to (the complement
 * of) a spatiotemporal box
 * @details The spatial footprint of the box is treated as a rectangular
 * polygon and the exact body-overlap restriction is applied; the result is
 * additionally restricted to the box's time span. Exact on the body extent
 * for pure-translation trgeometries; rotating segments raise an honest
 * FEATURE_NOT_SUPPORTED error.
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 * @note @p border_inc governs the upper border of BOTH dimensions. The time
 * span carries it directly. On the spatial side the box is closed where the
 * upper border is asked for and half-open where it is not, so a body meeting
 * the box only along an upper border stands outside it -- which is what lets
 * the tiles either side of a grid line claim a body lying on that line once
 * rather than twice.
 */
Temporal *
trgeo_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_stbox(temp, box))
    return NULL;

  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  /* The box period already carries its temporal-border inclusivity. On the
   * spatial side the box is closed where the upper border is asked for, and
   * half-open where it is not, so that the tiles either side of a grid line
   * claim a body lying on that line exactly once */
  const STBox *excl = border_inc ? NULL : box;

  /* Spatial overlap spans (whole time domain if the box has no X bounds). */
  SpanSet *spatial = NULL;
  if (hasx)
  {
    GSERIALIZED *geo = stbox_geo(box);
    LWGEOM *lwgeo = lwgeom_from_gserialized(geo);
    bool err;
    spatial = trgeo_overlap_spanset(temp, geo, lwgeo, excl, &err);
    lwgeom_free(lwgeo);
    pfree(geo);
    if (err)
      return NULL;
    if (! spatial)
      /* No spatial overlap. */
      return atfunc ? NULL : temporal_copy(temp);
  }

  /* Intersect the spatial overlap with the box time span. */
  SpanSet *atspans;
  if (hast)
  {
    Span *period = span_copy(&box->period);
    SpanSet *periodset = spanset_make_free(period, 1, NORMALIZE_NO, ORDER_NO);
    if (spatial)
    {
      atspans = intersection_spanset_spanset(spatial, periodset);
      pfree(periodset);
      pfree(spatial);
      if (! atspans)
        return atfunc ? NULL : temporal_copy(temp);
    }
    else
      atspans = periodset;
  }
  else
    atspans = spatial;

  Temporal *result = trgeo_restrict_overlap(temp, atspans, atfunc);
  if (atspans)
    pfree(atspans);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Trgeometry_at_stbox()
 */
Temporal *
trgeometry_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return trgeo_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * spatiotemporal box
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Trgeometry_minus_stbox()
 */
Temporal *
trgeometry_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return trgeo_restrict_stbox(temp, box, border_inc, REST_MINUS);
}

/*****************************************************************************
 * Restriction by elevation
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to (the complement of) an
 * elevation span
 * @details The elevation of a rigid geometry is the elevation of the position
 * of its pose, so the restriction is taken on the temporal point of the
 * positions and the result is the temporal rigid geometry restricted to the
 * times it keeps. The body geometry is not taken into account: a body extends
 * above and below its position, and the elevation span is tested against the
 * position alone
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Elevation span
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
trgeo_restrict_elevation(const Temporal *temp, const Span *s, bool atfunc)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(s, NULL);
  if (! ensure_has_Z(temp->temptype, temp->flags))
    return NULL;

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  Temporal *res = tgeo_restrict_elevation(tpoint, s, atfunc);
  pfree(tpoint);
  if (! res)
    return NULL;
  /* We restrict the temporal rigid geometry to the times the restricted point
   * keeps rather than converting the point back, so that the poses and the
   * reference geometry are preserved exactly */
  SpanSet *ss = temporal_time(res);
  pfree(res);
  Temporal *result = trgeo_restrict_overlap(temp, ss, REST_AT);
  pfree(ss);
  return result;
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to an elevation span
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Elevation span
 * @csqlfn #Trgeometry_at_elevation()
 */
Temporal *
trgeometry_at_elevation(const Temporal *temp, const Span *s)
{
  return trgeo_restrict_elevation(temp, s, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of an
 * elevation span
 * @param[in] temp Temporal rigid geometry
 * @param[in] s Elevation span
 * @csqlfn #Trgeometry_minus_elevation()
 */
Temporal *
trgeometry_minus_elevation(const Temporal *temp, const Span *s)
{
  return trgeo_restrict_elevation(temp, s, REST_MINUS);
}

/**
 * @brief Apply a pose to a body-frame geometry and return the centroid of the
 * world-frame result as a Datum
 */
static Datum
datum_pose_geom_centroid(Datum pose_datum, Datum geom_datum)
{
  GSERIALIZED *world_geom = pose_apply_geo(DatumGetPoseP(pose_datum),
    DatumGetGserializedP(geom_datum));
  GSERIALIZED *centroid = geom_centroid(world_geom);
  pfree(world_geom);
  return GserializedPGetDatum(centroid);
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the centroid of a temporal rigid geometry as a temporal point
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_centroid()
 */
Temporal *
trgeometry_centroid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_geom_centroid;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_GEOMETRY;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(trgeo_geom_p(temp)), &lfinfo);
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the convex hull of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_convex_hull()
 */
GSERIALIZED *
trgeometry_convex_hull(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  /* The hull of the placements is the hull of everything between them */
  GSERIALIZED *places = trgeo_placements(temp);
  if (! places)
    return NULL;
  GSERIALIZED *result = geom_convex_hull(places);
  pfree(places);
  return result;
}

/*****************************************************************************
 * Body-frame trajectory functions
 *****************************************************************************/

/**
 * @brief Apply a pose to a body-frame geometry and return the world-frame
 * result as a Datum
 */
static Datum
datum_pose_apply_to_geom(Datum pose_datum, Datum geom_datum)
{
  const Pose *pose = DatumGetPoseP(pose_datum);
  const GSERIALIZED *gs = DatumGetGserializedP(geom_datum);
  return GserializedPGetDatum(pose_apply_geo(pose, gs));
}

/**
 * @ingroup meos_rgeo_spatialfuncs
 * @brief Return the world-frame trajectory of a body-frame point on a moving
 * rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Body-frame geometry
 * @csqlfn #Trgeometry_body_point_trajectory()
 */
Temporal *
trgeometry_body_point_trajectory(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  VALIDATE_NOT_NULL(gs, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_apply_to_geom;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_GEOMETRY;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}


/*****************************************************************************
 * Similarity distance functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_analytics_similarity
 * @brief Return the Hausdorff distance between two temporal rigid geometries
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Trgeometry_hausdorff_distance()
 */
double
trgeometry_hausdorff_distance(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return DBL_MAX;
  Temporal *tp1 = trgeometry_to_tgeompoint(temp1);
  Temporal *tp2 = trgeometry_to_tgeompoint(temp2);
  double result = temporal_hausdorff_distance(tp1, tp2);
  pfree(tp1); pfree(tp2);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics_similarity
 * @brief Return the discrete Frechet distance between two temporal rigid
 * geometries
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Trgeometry_frechet_distance()
 */
double
trgeometry_frechet_distance(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return DBL_MAX;
  Temporal *tp1 = trgeometry_to_tgeompoint(temp1);
  Temporal *tp2 = trgeometry_to_tgeompoint(temp2);
  double result = temporal_similarity(tp1, tp2, FRECHET);
  pfree(tp1); pfree(tp2);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics_similarity
 * @brief Return the Dynamic Time Warp distance between two temporal rigid
 * geometries
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @csqlfn #Trgeometry_dyntimewarp_distance()
 */
double
trgeometry_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return DBL_MAX;
  Temporal *tp1 = trgeometry_to_tgeompoint(temp1);
  Temporal *tp2 = trgeometry_to_tgeompoint(temp2);
  double result = temporal_similarity(tp1, tp2, DYNTIMEWARP);
  pfree(tp1); pfree(tp2);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics_similarity
 * @brief Return the discrete Frechet path between two temporal rigid
 * geometries
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[out] count Number of elements in the resulting array
 * @csqlfn #Trgeometry_frechet_path()
 */
Match *
trgeometry_frechet_path(const Temporal *temp1, const Temporal *temp2,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL); 
  *count = 0;
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;
  Temporal *tp1 = trgeometry_to_tgeompoint(temp1);
  Temporal *tp2 = trgeometry_to_tgeompoint(temp2);
  Match *result = temporal_similarity_path(tp1, tp2, count, FRECHET);
  pfree(tp1); pfree(tp2);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics_similarity
 * @brief Return the Dynamic Time Warp path between two temporal rigid
 * geometries
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[out] count Number of elements in the resulting array
 * @csqlfn #Trgeometry_dyntimewarp_path()
 */
Match *
trgeometry_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2,
  int *count)
{
  /* The out parameter is defined even when a later check fails */
  VALIDATE_NOT_NULL(count, NULL); 
  *count = 0;
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;
  Temporal *tp1 = trgeometry_to_tgeompoint(temp1);
  Temporal *tp2 = trgeometry_to_tgeompoint(temp2);
  Match *result = temporal_similarity_path(tp1, tp2, count, DYNTIMEWARP);
  pfree(tp1); pfree(tp2);
  return result;
}

/*****************************************************************************
 * Motion metrics
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the length traversed by the centroid of a temporal rigid
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p DBL_MAX
 * @csqlfn #Trgeometry_length()
 */
double
trgeometry_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, DBL_MAX);

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  double result = tpoint_length(tpoint);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the cumulative length traversed by the centroid of a temporal
 * rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_cumulative_length()
 */
Temporal *
trgeometry_cumulative_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  Temporal *result = tpoint_cumulative_length(tpoint);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the speed of the centroid of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_speed()
 */
Temporal *
trgeometry_speed(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  Temporal *result = temporal_derivative(tpoint);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the angular speed of a temporal rigid geometry (radians per
 * unit time) as a step-interpolated temporal float
 * @details The shape of a rigid geometry never changes, so its angular speed
 * is the angular speed of the pose that carries it.
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 * @csqlfn #Trgeometry_angular_speed()
 */
Temporal *
trgeometry_angular_speed(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpose = trgeometry_to_tpose(temp);
  Temporal *result = tpose_angular_speed(tpose);
  pfree(tpose);
  return result;
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the time-weighted centroid of the centroid trajectory of a
 * temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_twcentroid()
 */
GSERIALIZED *
trgeometry_twcentroid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  GSERIALIZED *result = tpoint_twcentroid(tpoint);
  pfree(tpoint);
  return result;
}

/*****************************************************************************
 * Traversed area
 *****************************************************************************/

/*
 * @brief The rotation a single sample interval of trgeometry_traversed_area
 * may span.
 *
 * A vertex of a turning body travels an arc, and a sample interval spans the
 * chord of that arc, so the answer follows the body the more closely the less
 * it turns across one interval. At 0.05 rad (~2.86 deg) a quarter turn is
 * carried by 31 intermediate placements and a half turn by 62, which is the
 * most any interval asks for: a rotation is read as the shortest way round,
 * so it never exceeds pi.
 */
#define TRGEO_TRAVERSED_AREA_ANGLE_TOL    0.05

/**
 * @brief Append the materialised polygon at timestamp @p t to the running
 * geom array, growing it as needed.
 */
static void
trgeo_trav_emit_at(const Temporal *temp, TimestampTz t,
  GSERIALIZED ***buf, int *buf_n, int *buf_cap)
{
  Datum value;
  if (! trgeo_value_at_timestamptz(temp, t, false, &value))
    return;
  if (*buf_n == *buf_cap)
  {
    *buf_cap *= 2;
    *buf = repalloc(*buf, sizeof(GSERIALIZED *) * (*buf_cap));
  }
  (*buf)[(*buf_n)++] = DatumGetGserializedP(value);
}

/**
 * @brief Append the placements a body passes through on one segment
 * @details The rotation from one pose to the next decides how many
 * placements the segment carries: enough that no interval spans more than
 * `TRGEO_TRAVERSED_AREA_ANGLE_TOL` of turn, which is
 * `ceil(rotation / tolerance) - 1` intermediate ones. A segment the body
 * only translates along asks for none, its two placements bounding
 * everything it covers. The placement at @p inst_a is appended by the
 * caller, as the previous segment's end or as the first instant.
 */
static void
trgeo_trav_segment(const Temporal *temp, const TInstant *inst_a,
  const TInstant *inst_b, GSERIALIZED ***buf, int *buf_n, int *buf_cap)
{
  Pose *pose_a = DatumGetPoseP(tinstant_value_p((TInstant *) inst_a));
  Pose *pose_b = DatumGetPoseP(tinstant_value_p((TInstant *) inst_b));
  /* The rotation is the shortest way round, so it never exceeds pi */
  double dtheta = pose_b->data[2] - pose_a->data[2];
  const double TWOPI = 2.0 * M_PI;
  while (dtheta > M_PI)  dtheta -= TWOPI;
  while (dtheta < -M_PI) dtheta += TWOPI;
  int n = (int) ceil(fabs(dtheta) / TRGEO_TRAVERSED_AREA_ANGLE_TOL) - 1;
  if (n > 0)
  {
    /* An interval shorter than the timestamp resolution carries no
     * placement of its own */
    TimestampTz span = inst_b->t - inst_a->t;
    if (span > n)
      for (int i = 1; i <= n; i++)
        trgeo_trav_emit_at(temp, inst_a->t + (TimestampTz) (span * i / (n + 1)),
          buf, buf_n, buf_cap);
  }
  trgeo_trav_emit_at(temp, inst_b->t, buf, buf_n, buf_cap);
}

/**
 * @brief Return true if the reference geometry of a temporal rigid geometry is
 * a convex body
 * @details Convexity is a property of the reference geometry, which is one
 * geometry for the whole value, so it is read once rather than per segment.
 * A body carrying an inner ring is not convex.
 */
static bool
trgeo_ref_is_convex(const GSERIALIZED *gs)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  bool result = false;
  if (lwgeom->type == POLYGONTYPE && ((LWPOLY *) lwgeom)->nrings == 1)
    result = geom_ring_is_convex(((LWPOLY *) lwgeom)->rings[0]);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Return the region a convex body covers between two of its placements
 * @details Every point of a convex body travels within the hull of the two
 * placements, each vertex moving along a straight segment, so that hull is
 * what the body covers between them. The hull also contains the two
 * placements it spans, so a sweep replaces them rather than joining them.
 */
static GSERIALIZED *
trgeo_convex_sweep(const GSERIALIZED *a, const GSERIALIZED *b)
{
  LWGEOM *lw_a = lwgeom_from_gserialized(a);
  LWGEOM *lw_b = lwgeom_from_gserialized(b);
  POINTARRAY *ra = ((LWPOLY *) lw_a)->rings[0];
  POINTARRAY *rb = ((LWPOLY *) lw_b)->rings[0];
  POINTARRAY *pa = ptarray_construct_empty(0, 0, ra->npoints + rb->npoints);
  POINT4D p = {0, 0, 0, 0};
  for (uint32_t i = 0; i < ra->npoints; i++)
  {
    const POINT2D *q = getPoint2d_cp(ra, i);
    p.x = q->x; p.y = q->y;
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  for (uint32_t i = 0; i < rb->npoints; i++)
  {
    const POINT2D *q = getPoint2d_cp(rb, i);
    p.x = q->x; p.y = q->y;
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  LWGEOM *line = lwline_as_lwgeom(lwline_construct(gserialized_get_srid(a),
    NULL, pa));
  GSERIALIZED *gs = geo_serialize(line);
  lwgeom_free(line);
  lwgeom_free(lw_a); lwgeom_free(lw_b);
  GSERIALIZED *result = geom_convex_hull(gs);
  pfree(gs);
  return result;
}

/**
 * @brief Return the placements a temporal rigid geometry passes through
 * @details Every input instant contributes its placement, and a segment
 * carries as many placements between two of them as its rotation asks for at
 * `TRGEO_TRAVERSED_AREA_ANGLE_TOL` of turn each. The trgeometry-specific step
 * is materialising the reference geometry at every emitted sample via
 * `pose_apply_geo`
 * @param[in] temp Temporal rigid geometry
 * @param[out] count Number of placements returned
 * @return On error return @p NULL
 */
static GSERIALIZED **
trgeo_placement_array(const Temporal *temp, int *count)
{
  int n_insts = 0;
  /* temporal_insts_p returns a pointer-to-array of in-place instants;
   * neither the elements nor the array itself need element-freeing —
   * we only pfree the array wrapper at the end. */
  const TInstant **insts = temporal_insts_p(temp, &n_insts);
  if (! insts || n_insts <= 0)
    return NULL;

  /* Initial capacity is generous — bisection rarely goes deep. */
  int cap = (n_insts > 1) ? n_insts * 4 : n_insts;
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * cap);
  int n_geoms = 0;

  /* Emit the first instant. */
  trgeo_trav_emit_at(temp, insts[0]->t, &geoms, &n_geoms, &cap);
  /* Per inter-instant segment, recursively bisect. The recursion
   * appends samples on (t_m, t_b], reusing the t_a sample emitted by
   * the previous iteration. */
  for (int i = 0; i + 1 < n_insts; i++)
    trgeo_trav_segment(temp, insts[i], insts[i + 1],
      &geoms, &n_geoms, &cap);

  pfree(insts);
  *count = n_geoms;
  return geoms;
}

/**
 * @brief Return the geometries of an array as a single value, consuming the
 * array
 * @details A single geometry answers for itself; several are gathered into a
 * collection, which @p unary_union then dissolves into one geometry
 * @param[in] geoms Array of geometries
 * @param[in] n_geoms Number of geometries
 * @param[in] unary_union True to dissolve the collection
 */
static GSERIALIZED *
trgeo_geoms_merge(GSERIALIZED **geoms, int n_geoms, bool unary_union)
{
  GSERIALIZED *result;
  if (n_geoms == 1)
  {
    result = geoms[0];
  }
  else
  {
    GSERIALIZED *coll = geo_collect_garray(geoms, n_geoms);
    for (int i = 0; i < n_geoms; i++)
      pfree(geoms[i]);
    if (unary_union)
    {
      result = geom_unary_union(coll, -1);
      pfree(coll);
    }
    else
      result = coll;
  }
  pfree(geoms);
  return result;
}

/**
 * @brief Return the placements a temporal rigid geometry passes through as a
 * geometry collection
 * @details The ever/always spatial relationships read the placements one by
 * one to tell their two quantifiers apart: ever holds where one placement
 * satisfies the relationship, always where every one does. The area the body
 * traverses spans several placements at once, so it answers neither
 * @param[in] temp Temporal rigid geometry
 * @return On error return @p NULL
 */
GSERIALIZED *
trgeo_placements(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  int n_geoms = 0;
  GSERIALIZED **geoms = trgeo_placement_array(temp, &n_geoms);
  if (! geoms)
    return NULL;
  return trgeo_geoms_merge(geoms, n_geoms, UNARY_UNION_NO);
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the union of every materialised polygon of a temporal rigid
 * geometry over its time domain
 * @param[in] temp Temporal rigid geometry
 * @param[in] unary_union True to apply a unary spatial union to the per-
 * instant polygons; false to return the raw GeometryCollection
 * @csqlfn #Trgeometry_traversed_area()
 *
 * @note Mirrors the collect-then-union pattern of `tgeo_traversed_area`
 * for general temporal geometries. The placements the body passes through
 * come from #trgeo_placements(); a convex body then covers the hull of every
 * consecutive pair, and the unary union dissolves the overlap the hulls share
 */
GSERIALIZED *
trgeometry_traversed_area(const Temporal *temp, bool unary_union)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  int n_geoms = 0;
  GSERIALIZED **geoms = trgeo_placement_array(temp, &n_geoms);
  if (! geoms)
    return NULL;

  /* The samples are the placements the body passes through; the area it
   * traverses is what it covers BETWEEN them as well. A convex body covers
   * the hull of each consecutive pair, which is the case the traversed area
   * is defined for; a body with a concavity keeps the placements alone, as
   * its hull would close a concavity the body never reaches.
   * A body that does not interpolate covers nothing between its placements:
   * it holds each of them for the whole of its own interval and then stands
   * at the next, so the placements ARE the area it traverses and a hull
   * spanning two of them answers for ground the body never crosses.
   * Only the united form sweeps; the collection form answers the placements
   * themselves, which #trgeo_placements() gives a caller directly. */
  if (unary_union && n_geoms > 1 && MEOS_FLAGS_LINEAR_INTERP(temp->flags) &&
      trgeo_ref_is_convex(trgeo_geom_p(temp)))
  {
    GSERIALIZED **swept = palloc(sizeof(GSERIALIZED *) * (n_geoms - 1));
    int n_swept = 0;
    for (int i = 0; i + 1 < n_geoms; i++)
    {
      GSERIALIZED *hull = trgeo_convex_sweep(geoms[i], geoms[i + 1]);
      if (hull)
        swept[n_swept++] = hull;
    }
    if (n_swept > 0)
    {
      for (int i = 0; i < n_geoms; i++)
        pfree(geoms[i]);
      pfree(geoms);
      geoms = swept;
      n_geoms = n_swept;
    }
    else
      pfree(swept);
  }

  return trgeo_geoms_merge(geoms, n_geoms, unary_union);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_conversion
 * @brief Materialise the moving polygon of a temporal rigid geometry as a
 * temporal geometry (one rotated/translated polygon per instant).
 * @details For each instant, applies the instant's pose to the trgeo's
 * reference geometry and emits the resulting polygon. The returned
 * `tgeometry` has the same temporal structure as the input and the same
 * SRID as the reference geometry.
 * @note Unlike `trgeometry_to_tgeompoint` (which gives the antenna point trajectory),
 * this preserves the body's footprint at each instant — the natural input
 * for compositional spatial-rel queries against `tgeometry`'s full surface.
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_to_tgeometry()
 */
Temporal *
trgeometry_to_tgeometry(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_has_geom(temp->flags))
    return NULL;
  const GSERIALIZED *ref = trgeo_geom_p(temp);
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    GSERIALIZED *poly = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst)),
      ref);
    TInstant *res = tinstant_make(PointerGetDatum(poly), T_TGEOMETRY,
      inst->t);
    pfree(poly);
    return (Temporal *) res;
  }
  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      GSERIALIZED *poly = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst)),
        ref);
      insts[i] = tinstant_make(PointerGetDatum(poly), T_TGEOMETRY,
        inst->t);
      pfree(poly);
    }
    return (Temporal *) tsequence_make_free(insts, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      STEP, NORMALIZE);
  }
  /* TSEQUENCESET */
  const TSequenceSet *ss = (const TSequenceSet *) temp;
  TSequence **seqs = palloc(sizeof(TSequence *) * ss->count);
  for (int s = 0; s < ss->count; s++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, s);
    TInstant **insts = palloc(sizeof(TInstant *) * seq->count);
    for (int i = 0; i < seq->count; i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      GSERIALIZED *poly = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst)),
        ref);
      insts[i] = tinstant_make(PointerGetDatum(poly), T_TGEOMETRY,
        inst->t);
      pfree(poly);
    }
    seqs[s] = tsequence_make_free(insts, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      STEP, NORMALIZE);
  }
  return (Temporal *) tsequenceset_make_free(seqs, ss->count, NORMALIZE);
}

/*****************************************************************************/
