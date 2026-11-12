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
 * @details The restriction is exact for the body extent: a `trgeometry` is
 * "in" a
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
#include "rgeo/trgeo_distwalk.h"
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
 * @brief Return true if any polygon component of an LWGEOM carries an interior
 * ring (hole)
 * @details The M1 clip primitive ignores holes, so a holey target is rejected
 * rather than silently treated as solid.
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
 * segment-local parameter t in [0, 1]) for a pure-translation body segment
 * @details The body translates by @p dx, @p dy with fixed orientation. Each
 * exterior ring edge of the (already world-posed at t = 0) body @p body0 sweeps
 * a parallelogram; the clip primitive returns the times at which that edge
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
        int n = trgeometry_geom_clip_lwgeom(&a1, &b1, &a2, &b2, target, &iv);
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
  return (bbox.xmin >= box->xmax) || (bbox.ymin >= box->ymax) ||
    (MEOS_FLAGS_GET_Z(box->flags) && bbox.zmin >= box->zmax);
}

/**
 * @brief Return whether a vertex of a body lies in a closed box with Z, by
 * comparisons of its coordinates
 * @param[in] body Body with Z, already placed in the world
 * @param[in] box Box with Z
 */
static bool
body_vertex_in_box(const GSERIALIZED *body, const STBox *box)
{
  LWGEOM *geom = lwgeom_from_gserialized(body);
  LWPOINTITERATOR *it = lwpointiterator_create(geom);
  POINT4D p;
  bool found = false;
  while (! found && lwpointiterator_next(it, &p))
    found = p.x >= box->xmin && p.x <= box->xmax && p.y >= box->ymin &&
      p.y <= box->ymax && p.z >= box->zmin && p.z <= box->zmax;
  lwpointiterator_destroy(it);
  lwgeom_free(geom);
  return found;
}

/**
 * @brief Return whether a body meets a target, leaving out the upper borders
 * of @p excl where it is given
 * @details A body meets a closed box with Z where a vertex of it lies in the
 * box or where it is at distance zero of the faces of the box, which
 * #geom_dwithin3d answers exactly for faces lying in a plane: a body meeting
 * the box and holding no vertex in it crosses its boundary. A target with no
 * box with Z is met in 2D (#geom_intersects2d)
 * @param[in] body Body, already placed in the world
 * @param[in] target Target geometry, the faces of @p box3d where it is given
 * @param[in] excl Box whose upper borders are left out, NULL to meet the
 * target as a closed region @nullable
 * @param[in] box3d Box with Z the target bounds, NULL for a target met in
 * 2D @nullable
 */
static bool
body_meets_target(const GSERIALIZED *body, const GSERIALIZED *target,
  const STBox *excl, const STBox *box3d)
{
  bool meets = box3d ?
    body_vertex_in_box(body, box3d) || geom_dwithin3d(body, target, 0.0) :
    geom_intersects2d(body, target);
  if (! meets)
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
    bool ov = body_meets_target(bodym, target, excl, NULL);
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
/**
 * @brief Turn a rotating segment carries before it is read as one piece
 * @details A vertex of a rotating body travels an arc, and the swept-edge clip
 * the translating path solves reads straight travel alone, so a rotating
 * segment is read in pieces short enough that the overlap holds or fails
 * throughout each. The same turn bounds the pieces of the traversed area
 */
#define TRGEO_OVERLAP_ANGLE_TOL 0.05

/**
 * @brief Return whether a body placed by the pose at a ratio of a segment
 * meets a target
 */
static bool
body_meets_at(const GSERIALIZED *ref, const Pose *pose1, const Pose *pose2,
  double ratio, const GSERIALIZED *target, const STBox *excl)
{
  Pose *p = posesegm_interpolate(pose1, pose2, ratio);
  GSERIALIZED *body = pose_apply_geo(p, ref);
  bool ov = body_meets_target(body, target, excl, NULL);
  pfree(body); pfree(p);
  return ov;
}

/**
 * @brief Return the ratio at which the overlap of a rotating body with a
 * target changes between two ratios that answer differently
 * @details The two ratios answer differently, so a change stands between them
 * and halving the bracket closes on it. What the halving reads is the overlap
 * itself, so the ratio it returns is the one the predicate changes at
 */
static double
rotating_change_ratio(const GSERIALIZED *ref, const Pose *pose1,
  const Pose *pose2, double lo, double hi, bool ov_lo,
  const GSERIALIZED *target, const STBox *excl)
{
  for (int k = 0; k < 40 && hi - lo > MEOS_GEOM_TOLERANCE; k++)
  {
    double mid = 0.5 * (lo + hi);
    if (body_meets_at(ref, pose1, pose2, mid, target, excl) == ov_lo)
      lo = mid;
    else
      hi = mid;
  }
  return hi;
}

/**
 * @brief Compute the times at which a body meets a target over a segment
 * along which it also ROTATES
 * @details The translating path takes its events from a swept-edge clip,
 * which reads straight travel and has nothing to say about a vertex on an
 * arc. A rotating segment is read instead in pieces each carrying at most
 * `TRGEO_OVERLAP_ANGLE_TOL` of turn: the overlap is asked at the ends of every
 * piece, and where two consecutive readings differ the ratio the answer
 * changes at is found by halving that piece. The times so found are exact in
 * the overlap they read and carry the resolution of the halving, where the
 * translating path solves its events in closed form
 * @param[in] ref Reference geometry
 * @param[in] pose1,pose2 Poses at the ends of the segment
 * @param[in] t1,t2 Times at the ends of the segment
 * @param[in] target Target geometry
 * @param[in] excl Box whose upper borders are left out, NULL for none @nullable
 * @param[in,out] spans,nspans,spancap Spans to append to
 */
static void
segment_overlap_spans_rotating(const GSERIALIZED *ref, const Pose *pose1,
  const Pose *pose2, TimestampTz t1, TimestampTz t2,
  const GSERIALIZED *target, const STBox *excl, Span **spans, int *nspans,
  int *spancap)
{
  double dtheta = fabs(pose2->data[2] - pose1->data[2]);
  int npieces = (int) ceil(dtheta / TRGEO_OVERLAP_ANGLE_TOL);
  if (npieces < 1)
    npieces = 1;

  bool prev = body_meets_at(ref, pose1, pose2, 0.0, target, excl);
  double run_start = 0.0;
  for (int k = 1; k <= npieces; k++)
  {
    double f = (double) k / npieces;
    bool ov = body_meets_at(ref, pose1, pose2, f, target, excl);
    if (ov == prev)
      continue;
    double change = rotating_change_ratio(ref, pose1, pose2,
      (double) (k - 1) / npieces, f, prev, target, excl);
    if (ov)
      run_start = change;
    else
    {
      if (*nspans + 1 > *spancap)
      {
        *spancap = (*nspans + 1) * 2 + 8;
        *spans = repalloc(*spans, (size_t) *spancap * sizeof(Span));
      }
      TimestampTz lo = t1 +
        (TimestampTz) llround(run_start * (double) (t2 - t1));
      TimestampTz hi = t1 + (TimestampTz) llround(change * (double) (t2 - t1));
      span_set(TimestampTzGetDatum(lo), TimestampTzGetDatum(hi), true, true,
        T_TIMESTAMPTZ, T_TSTZSPAN, &(*spans)[*nspans]);
      (*nspans)++;
    }
    prev = ov;
  }
  if (prev)
  {
    if (*nspans + 1 > *spancap)
    {
      *spancap = (*nspans + 1) * 2 + 8;
      *spans = repalloc(*spans, (size_t) *spancap * sizeof(Span));
    }
    TimestampTz lo = t1 + (TimestampTz) llround(run_start * (double) (t2 - t1));
    span_set(TimestampTzGetDatum(lo), TimestampTzGetDatum(t2), true, true,
      T_TIMESTAMPTZ, T_TSTZSPAN, &(*spans)[*nspans]);
    (*nspans)++;
  }
  return;
}

static void
instant_overlap_span(const GSERIALIZED *ref, const Pose *pose, TimestampTz t,
  const GSERIALIZED *target, const STBox *excl, const STBox *box3d,
  Span **spans, int *nspans, int *spancap)
{
  GSERIALIZED *body = pose_apply_geo(pose, ref);
  bool ov = body_meets_target(body, target, excl, box3d);
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
 * @brief Return true if the walk of the distance reads a body and a target
 * @details The walk needs a body that is a convex polygon without holes
 * (#distrefpoly_set), and a target of points, lines and convex polygons
 * without holes, and of their collections of one type, the parts it measures
 * (#dist_geo_parts reads them so); each is tested here without an error
 * @param[in] ref Reference geometry of the body
 * @param[in] target Target geometry
 */
static bool
trgeometry_walk_reads(const GSERIALIZED *ref, const LWGEOM *target)
{
  LWGEOM *body = lwgeom_from_gserialized(ref);
  DistRefPoly rp;
  bool result = body->type == POLYGONTYPE &&
    distrefpoly_set(&rp, (const LWPOLY *) body);
  if (result)
    distrefpoly_free(&rp);
  lwgeom_free(body);
  if (! result || lwgeom_is_empty(target))
    return result;
  uint32_t type = target->type;
  if (type == POINTTYPE || type == LINETYPE || type == MULTIPOINTTYPE ||
      type == MULTILINETYPE)
    return true;
  if (type != POLYGONTYPE && type != MULTIPOLYGONTYPE)
    return false;
  const LWCOLLECTION *coll = (type == MULTIPOLYGONTYPE) ?
    (const LWCOLLECTION *) target : NULL;
  uint32_t n = coll ? coll->ngeoms : 1;
  for (uint32_t i = 0; i < n && result; i++)
  {
    const LWGEOM *part = coll ? coll->geoms[i] : target;
    if (lwgeom_is_empty(part))
      continue;
    double cx, cy;
    result = distrefpoly_set_static(&rp, (const LWPOLY *) part, &cx, &cy);
    if (result)
      distrefpoly_free(&rp);
  }
  return result;
}

/**
 * @brief Return true if a segment between two poses with Z translates
 * @details A pose with Z holds its orientation in a quaternion, the data after
 * its position, and the segment translates where the two quaternions are
 * equal or opposite
 */
static bool
pose3d_segment_translates(const Pose *pose1, const Pose *pose2)
{
  const double *q1 = &pose1->data[3], *q2 = &pose2->data[3];
  bool same = true, opposite = true;
  for (int k = 0; k < 4; k++)
  {
    same &= q1[k] == q2[k];
    opposite &= q1[k] == -q2[k];
  }
  return same || opposite;
}

/**
 * @brief Return true if every segment of a temporal rigid geometry with Z in
 * linear interpolation translates, see #pose3d_segment_translates()
 */
static bool
trgeometry_translates(const Temporal *temp)
{
  if (temp->subtype == TINSTANT)
    return true;
  const TSequenceSet *ss = (temp->subtype == TSEQUENCESET) ?
    (const TSequenceSet *) temp : NULL;
  int nseqs = ss ? ss->count : 1;
  for (int s = 0; s < nseqs; s++)
  {
    const TSequence *seq = ss ? TSEQUENCESET_SEQ_N(ss, s) :
      (const TSequence *) temp;
    for (int i = 0; i + 1 < seq->count; i++)
      if (! pose3d_segment_translates(
          DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq, i))),
          DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq, i + 1)))))
        return false;
  }
  return true;
}

/**
 * @brief Compute the timestamps at which a trgeometry body overlaps a target
 * geometry, returning a tstzspanset (or NULL if no overlap)
 * @details Iterates the trgeometry instants. Each linear segment is required to
 * be a pure translation; any rotating segment raises FEATURE_NOT_SUPPORTED. The
 * caller has already rejected holey targets.
 *
 * @return The overlap spanset, NULL when the body never overlaps the target
 * @note A rotating segment raises through the MEOS error handler and sets
 * @p *err, which is what tells an error apart from an absent overlap
 * @errval NULL
 */
static SpanSet *
trgeometry_overlap_spanset(const Temporal *temp, const GSERIALIZED *gs,
  const LWGEOM *lwgs, const STBox *excl, const STBox *box3d, bool *err)
{
  *err = false;
  const GSERIALIZED *ref = trgeometry_geom_p(temp);
  /* The clip of a moving body reads its exterior ring alone (#trgeo_geom_clip),
   * so a body with holes in linear motion is refused rather than answered as
   * the region its exterior ring bounds */
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags))
  {
    LWGEOM *body = lwgeom_from_gserialized(ref);
    bool holes = lwgeom_has_interior_ring(body);
    lwgeom_free(body);
    if (holes)
    {
      *err = true;
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "The restriction of a temporal rigid geometry with holes in linear "
        "motion is not implemented");
      return NULL;
    }
  }
  /* A body in linear motion meets a closed target where its distance to it
   * is zero, which the walk of the distance answers exactly, turning
   * included (#tdwithin_trgeometry_geo); the clip below reads the bodies and
   * targets the walk does not, and the upper borders a tile leaves out.  A
   * body with Z in a plane of constant z that translates, against a target
   * without Z, is read on the dimensions they share, its projection to the
   * ground (#trgeometry_force2d), which moves rigidly with it */
  bool linear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  Temporal *ground = NULL;
  if (linear && ! excl && MEOS_FLAGS_GET_Z(temp->flags) &&
      ! FLAGS_GET_Z(gs->gflags) && trgeometry_ref_is_flat(ref) &&
      trgeometry_translates(temp))
  {
    ground = trgeometry_force2d(temp);
    if (! ground)
    {
      *err = true;
      return NULL;
    }
  }
  const Temporal *value = ground ? ground : temp;
  if (linear && ! excl && ! MEOS_FLAGS_GET_Z(value->flags) &&
      ! FLAGS_GET_Z(gs->gflags) &&
      trgeometry_walk_reads(trgeometry_geom_p(value), lwgs))
  {
    Temporal *within = tdwithin_trgeometry_geo(value, gs, 0.0);
    if (ground)
      pfree(ground);
    if (! within)
    {
      *err = true;
      return NULL;
    }
    SpanSet *result = tbool_when_true(within);
    pfree(within);
    return result;
  }
  if (ground)
    pfree(ground);
  int spancap = 16, nspans = 0;
  Span *spans = palloc((size_t) spancap * sizeof(Span));

  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (const TInstant *) temp;
    instant_overlap_span(ref, DatumGetPoseP(tinstant_value_p(inst)),
      inst->t, gs, excl, box3d, &spans, &nspans, &spancap);
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
    for (int s = 0; s < nseqs; s++)
    {
      const TSequence *seq = seqs[s];
      if (seq->count == 1)
      {
        const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
        instant_overlap_span(ref, DatumGetPoseP(tinstant_value_p(inst)),
          inst->t, gs, excl, box3d, &spans, &nspans, &spancap);
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
          bool ov = body_meets_target(body, gs, excl, box3d);
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
        /* A pose with Z holds its orientation in a quaternion, the data after
         * its position, and a segment translates where the two quaternions
         * are equal or opposite; a segment turning in 3D is refused, its
         * projection on the plane being no rigid motion of it */
        if (MEOS_FLAGS_GET_Z(temp->flags))
        {
          bool turns = ! pose3d_segment_translates(pose1, pose2);
          if (! turns && segment_overlap_spans(ref, pose1, pose2, inst1->t,
              inst2->t, gs, lwgs, excl, &spans, &nspans, &spancap))
            continue;
          pfree(spans);
          if (temp->subtype == TSEQUENCESET)
            pfree(seqs);
          *err = true;
          if (turns)
          {
            meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
              "The restriction of a temporal rigid geometry with Z turning in "
              "linear interpolation is not implemented");
            return NULL;
          }
          return NULL;
        }
        /* A rotating segment carries its vertices along arcs, which the
         * swept-edge clip of the translating path does not solve, so it is
         * read in pieces short enough to hold one answer each */
        if (fabs(pose2->data[2] - pose1->data[2]) > MEOS_GEOM_TOLERANCE)
        {
          segment_overlap_spans_rotating(ref, pose1, pose2, inst1->t,
            inst2->t, gs, excl, &spans, &nspans, &spancap);
          continue;
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
 * timestamps, preserving the appended reference geometry
 * @details The generic temporal restriction operates on the pose component
 * and drops the reference geometry; this re-appends it so the result is a
 * well-formed temporal rigid geometry.
 * @param[in] temp Temporal rigid geometry
 * @param[in] ss Set of timestamps (overlap intervals); when NULL, an empty
 * restriction is assumed
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
static Temporal *
trgeometry_restrict_overlap(const Temporal *temp, const SpanSet *ss, bool atfunc)
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
  Temporal *result = geometry_tpose_to_trgeometry(trgeometry_geom_p(temp), res);
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
 * trgeometries, and a value in step or discrete interpolation is restricted
 * by its placements, holes of the target included. A target with holes for a
 * value in linear interpolation raises a FEATURE_NOT_SUPPORTED error.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
trgeometry_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  LWGEOM *lwgs = lwgeom_from_gserialized(gs);
  /* A body that holds each pose is restricted by the static intersection of
   * each placement, which reads the holes of the target (#body_meets_target);
   * the clip of a body in linear motion reads the exterior ring of the target
   * alone, so a target with holes is refused for it */
  if (MEOS_FLAGS_LINEAR_INTERP(temp->flags) && lwgeom_has_interior_ring(lwgs))
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
  SpanSet *ss = trgeometry_overlap_spanset(temp, gs, lwgs, NULL, NULL, &err);
  lwgeom_free(lwgs);
  if (err)
    return NULL;

  Temporal *result = trgeometry_restrict_overlap(temp, ss, atfunc);
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
  return trgeometry_restrict_geom(temp, gs, REST_AT);
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
  return trgeometry_restrict_geom(temp, gs, REST_MINUS);
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
trgeometry_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments. A box of time alone restricts
   * the time domain, as #tgeo_restrict_stbox does */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(box, NULL);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  if (! hasx)
    return temporal_restrict_tstzspan(temp, &box->period, atfunc);
  /* The box is read on the dimensions the value and the box both have, as a
   * relationship of two spatial values reads them (#tgeo_different_dimensions) */
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(tspatial_srid(temp), stbox_srid(box)))
    return NULL;
  STBox box1 = *box;
  bool z = MEOS_FLAGS_GET_Z(temp->flags) && MEOS_FLAGS_GET_Z(box->flags);
  if (! z)
    MEOS_FLAGS_SET_Z(box1.flags, false);
  /* A body in linear motion against a box with Z sweeps a region in 3D,
   * which no kernel solves yet; one holding each pose is read placement by
   * placement */
  if (z && MEOS_FLAGS_LINEAR_INTERP(temp->flags) && temp->subtype != TINSTANT)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The restriction of a temporal rigid geometry with Z in linear "
      "interpolation by a box with Z is not implemented");
    return NULL;
  }
  /* The box period already carries its temporal-border inclusivity. On the
   * spatial side the box is closed where the upper border is asked for, and
   * half-open where it is not, so that the tiles either side of a grid line
   * claim a body lying on that line exactly once */
  const STBox *excl = border_inc ? NULL : &box1;

  /* Spatial overlap spans. A box with Z is met through its faces, read as a
   * surface rather than as the solid #stbox_geo flags, and its vertices
   * (#body_meets_target) */
  GSERIALIZED *geo = stbox_geo(&box1);
  LWGEOM *lwgeo = lwgeom_from_gserialized(geo);
  if (z)
  {
    FLAGS_SET_SOLID(lwgeo->flags, 0);
    pfree(geo);
    geo = geo_serialize(lwgeo);
  }
  bool err;
  SpanSet *spatial = trgeometry_overlap_spanset(temp, geo, lwgeo, excl,
    z ? &box1 : NULL, &err);
  lwgeom_free(lwgeo);
  pfree(geo);
  if (err)
    return NULL;
  if (! spatial)
    /* No spatial overlap. */
    return atfunc ? NULL : temporal_copy(temp);

  /* Intersect the spatial overlap with the box time span. */
  SpanSet *atspans;
  if (hast)
  {
    Span *period = span_copy(&box->period);
    SpanSet *periodset = spanset_make_free(period, 1, NORMALIZE_NO, ORDER_NO);
    atspans = intersection_spanset_spanset(spatial, periodset);
    pfree(periodset);
    pfree(spatial);
    if (! atspans)
      return atfunc ? NULL : temporal_copy(temp);
  }
  else
    atspans = spatial;

  Temporal *result = trgeometry_restrict_overlap(temp, atspans, atfunc);
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
  return trgeometry_restrict_stbox(temp, box, border_inc, REST_AT);
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
  return trgeometry_restrict_stbox(temp, box, border_inc, REST_MINUS);
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
 * @details The centroid is a point fixed on the body, so the result has the
 * interpolation of the temporal rigid geometry, as the result of
 * #trgeometry_body_point_trajectory
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
  lfinfo.reslinear = true;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(trgeometry_geom_p(temp)), &lfinfo);
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
  GSERIALIZED *places = trgeometry_placements(temp);
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
 * @details The pose is applied to the point at each instant, and the result
 * has the interpolation of the temporal rigid geometry. A linear result is
 * exact along a segment in which the body does not rotate. Along a segment in
 * which it rotates, the point follows a curve, which the result replaces by
 * the segment joining the positions at the two instants.
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
  lfinfo.reslinear = true;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}


/*****************************************************************************
 * Motion metrics
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the length traversed by the centroid of a temporal rigid
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @errval DBL_MAX
 * @csqlfn #Trgeometry_length()
 */
double
trgeometry_length(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, DBL_MAX);

  Temporal *tpoint = trgeometry_to_tgeompoint(temp);
  double result = tpoint_length(tpoint, true);
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
  Temporal *result = tpoint_cumulative_length(tpoint, true);
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
 * @errval NULL
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
 * @brief The placements a temporal rigid geometry passes through, in time
 * order, each with its timestamp and whether the body turns on its way to it
 */
typedef struct
{
  GSERIALIZED **geoms;  /**< The placements */
  TimestampTz *times;   /**< The timestamp of each placement */
  bool *turns;          /**< True at k if the body turns from placement k - 1 */
  int count;            /**< Number of placements */
  int cap;              /**< Capacity of the arrays */
  double radius;        /**< The greatest distance of a vertex of the body
                             from the point its pose places */
} TrgeoPlacements;

/**
 * @brief Append the materialised polygon at timestamp @p t to the running
 * placements, growing them as needed
 * @param[in] temp Temporal rigid geometry
 * @param[in] t Timestamp
 * @param[in] turn True if the body turns on its way to this placement
 * @param[in,out] pl Placements
 */
static void
trgeometry_trav_emit_at(const Temporal *temp, TimestampTz t, bool turn,
  TrgeoPlacements *pl)
{
  Datum value;
  if (! trgeometry_value_at_timestamptz_intl(temp, t, false, &value))
    return;
  if (pl->count == pl->cap)
  {
    pl->cap *= 2;
    pl->geoms = repalloc(pl->geoms, sizeof(GSERIALIZED *) * pl->cap);
    pl->times = repalloc(pl->times, sizeof(TimestampTz) * pl->cap);
    pl->turns = repalloc(pl->turns, sizeof(bool) * pl->cap);
  }
  pl->geoms[pl->count] = DatumGetGserializedP(value);
  pl->times[pl->count] = t;
  pl->turns[pl->count++] = turn;
}

/**
 * @brief Append the placements a body passes through on one segment
 * @details The rotation from one pose to the next decides how many
 * placements the segment carries: enough that no interval spans more than
 * `TRGEO_TRAVERSED_AREA_ANGLE_TOL` of turn, which is
 * `ceil(rotation / tolerance) - 1` intermediate ones. A segment the body
 * only translates along asks for none, its two placements bounding
 * everything it covers. The placement at @p inst_a is appended by the
 * caller, as the previous segment's end or as the first instant. Each
 * placement it appends records whether the body turns on the segment, which
 * #trgeometry_traversed_area reads to sweep the pair that ends there.
 */
static void
trgeometry_trav_segment(const Temporal *temp, const TInstant *inst_a,
  const TInstant *inst_b, TrgeoPlacements *pl)
{
  Pose *pose_a = DatumGetPoseP(tinstant_value_p((TInstant *) inst_a));
  Pose *pose_b = DatumGetPoseP(tinstant_value_p((TInstant *) inst_b));
  double dtheta, travel;
  bool turn;
  if (MEOS_FLAGS_GET_Z(pose_a->flags))
  {
    /* A pose with Z holds its position in its first three values and its
     * orientation in the quaternion after them, which turns the body by twice
     * the angle between the two quaternions, the shortest way round */
    turn = ! pose3d_segment_translates(pose_a, pose_b);
    double dot = 0.0;
    for (int k = 3; k < 7; k++)
      dot += pose_a->data[k] * pose_b->data[k];
    dtheta = turn ? 2.0 * acos(fmin(fabs(dot), 1.0)) : 0.0;
    double dx = pose_b->data[0] - pose_a->data[0];
    double dy = pose_b->data[1] - pose_a->data[1];
    double dz = pose_b->data[2] - pose_a->data[2];
    travel = sqrt(dx * dx + dy * dy + dz * dz);
  }
  else
  {
    /* The rotation is the shortest way round, so it never exceeds pi */
    dtheta = pose_b->data[2] - pose_a->data[2];
    const double TWOPI = 2.0 * M_PI;
    while (dtheta > M_PI)  dtheta -= TWOPI;
    while (dtheta < -M_PI) dtheta += TWOPI;
    turn = (dtheta != 0.0);
    travel = hypot(pose_b->data[0] - pose_a->data[0],
      pose_b->data[1] - pose_a->data[1]);
  }
  int n = (int) ceil(fabs(dtheta) / TRGEO_TRAVERSED_AREA_ANGLE_TOL) - 1;
  /* An edge turning while it travels bulges past its straight sweep by
   * about the travel times the turn of an interval over 8, which the
   * intervals keep within the sagitta of a turn of the step at the radius of
   * the body, the radius times the step squared over 8 */
  if (turn && pl->radius > 0.0)
  {
    int m = (int) ceil(sqrt(travel * fabs(dtheta) / pl->radius) /
      TRGEO_TRAVERSED_AREA_ANGLE_TOL) - 1;
    if (m > n)
      n = m;
  }
  if (n > 0)
  {
    /* An interval shorter than the timestamp resolution carries no
     * placement of its own */
    TimestampTz span = inst_b->t - inst_a->t;
    if (span > n)
      for (int i = 1; i <= n; i++)
        trgeometry_trav_emit_at(temp,
          inst_a->t + (TimestampTz) (span * i / (n + 1)), turn, pl);
  }
  trgeometry_trav_emit_at(temp, inst_b->t, turn, pl);
}

/**
 * @brief Return true if the reference geometry of a temporal rigid geometry is
 * a convex body
 * @details Convexity is a property of the reference geometry, which is one
 * geometry for the whole value, so it is read once rather than per segment.
 * A body carrying an inner ring is not convex.
 */
static bool
trgeometry_ref_is_convex(const GSERIALIZED *gs)
{
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  bool result = false;
  if (lwgeom->type == POLYGONTYPE && ((LWPOLY *) lwgeom)->nrings == 1)
    result = geom_ring_is_convex(((LWPOLY *) lwgeom)->rings[0]);
  lwgeom_free(lwgeom);
  return result;
}

/**
 * @brief Return the hull of two placements of a convex body
 * @details A convex body that translates moves every point along a straight
 * segment, so the hull of the two placements is exactly what it covers
 * between them. A body that turns covers the hull where none of its edges
 * crosses itself between the two, see #trgeometry_turn_sweep. The hull
 * contains the two placements it spans, so it replaces them rather than
 * joining them.
 */
static GSERIALIZED *
trgeometry_convex_sweep(const GSERIALIZED *a, const GSERIALIZED *b)
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
 * @brief Append a geometry to an array of geometries, growing it as needed,
 * as #trgeometry_trav_emit_at grows the placements
 */
static void
trgeometry_pieces_append(GSERIALIZED *gs, GSERIALIZED ***buf, int *count,
  int *cap)
{
  if (! gs)
    return;
  if (*count == *cap)
  {
    *cap *= 2;
    *buf = repalloc(*buf, sizeof(GSERIALIZED *) * (*cap));
  }
  (*buf)[(*count)++] = gs;
}

/**
 * @brief Return true if the segments @p p1 @p p2 and @p p3 @p p4 cross at a
 * point interior to both
 * @details The crossing is decided by the exact signs of the four
 * orientations, as #relate_segments_intersect decides a proper crossing; a
 * shared endpoint, a touch or a collinear overlap is no crossing
 */
static bool
trgeometry_segm_cross(const POINT2D *p1, const POINT2D *p2, const POINT2D *p3,
  const POINT2D *p4)
{
  int s1 = cross_product_sign(p3->x, p3->y, p4->x, p4->y, p3->x, p3->y,
    p1->x, p1->y);
  int s2 = cross_product_sign(p3->x, p3->y, p4->x, p4->y, p3->x, p3->y,
    p2->x, p2->y);
  if (s1 * s2 >= 0)
    return false;
  int s3 = cross_product_sign(p1->x, p1->y, p2->x, p2->y, p1->x, p1->y,
    p3->x, p3->y);
  int s4 = cross_product_sign(p1->x, p1->y, p2->x, p2->y, p1->x, p1->y,
    p4->x, p4->y);
  return s3 * s4 < 0;
}

/**
 * @brief Append the triangle @p a @p b @p c as a counter-clockwise polygon,
 * nothing for three collinear points
 * @details A triangle goes into the union as the polygon it draws, as
 * #buffer_triangle_as_poly writes one, the readers of a collection taking no
 * other surface type alike
 */
static void
trgeometry_triangle_append(int32_t srid, const POINT2D *a, const POINT2D *b,
  const POINT2D *c, GSERIALIZED ***buf, int *count, int *cap)
{
  int o = cross_product_sign(a->x, a->y, b->x, b->y, a->x, a->y, c->x, c->y);
  if (o == 0)
    return;
  const POINT2D *corner[4] = {a, o > 0 ? b : c, o > 0 ? c : b, a};
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 4);
  POINT4D p = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++)
  {
    p.x = corner[i]->x; p.y = corner[i]->y;
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  POINTARRAY **rings = lwalloc(sizeof(POINTARRAY *));
  rings[0] = pa;
  LWGEOM *poly = lwpoly_as_lwgeom(lwpoly_construct(srid, NULL, 1, rings));
  GSERIALIZED *gs = geo_serialize(poly);
  lwgeom_free(poly);
  trgeometry_pieces_append(gs, buf, count, cap);
}

/**
 * @brief Append the region the edge @p a0 @p b0 sweeps to @p a1 @p b1 when
 * its two ends move straight and it does not cross itself, as two triangles
 * @details The region is the quadrilateral the two positions of the edge and
 * the paths of its ends bound, the four corners of #trgeometry_convex_sweep
 * read in order rather than hulled, split along the diagonal lying inside
 * it, the one that separates the two other corners. Every corner is a vertex
 * of a placement, so no point is constructed
 */
static void
trgeometry_edge_sweep(int32_t srid, const POINT2D *a0, const POINT2D *b0,
  const POINT2D *a1, const POINT2D *b1, GSERIALIZED ***buf, int *count,
  int *cap)
{
  int s1 = cross_product_sign(b0->x, b0->y, a1->x, a1->y, b0->x, b0->y,
    a0->x, a0->y);
  int s2 = cross_product_sign(b0->x, b0->y, a1->x, a1->y, b0->x, b0->y,
    b1->x, b1->y);
  if (s1 * s2 < 0)
  {
    trgeometry_triangle_append(srid, a0, b0, a1, buf, count, cap);
    trgeometry_triangle_append(srid, b0, b1, a1, buf, count, cap);
  }
  else
  {
    trgeometry_triangle_append(srid, a0, b0, b1, buf, count, cap);
    trgeometry_triangle_append(srid, a0, b1, a1, buf, count, cap);
  }
}

/**
 * @brief Return the face @p i of a body: the polygon itself, or a patch of a
 * polyhedral surface
 */
static const LWPOLY *
trgeometry_face_n(const LWGEOM *lwgeom, int i)
{
  return (lwgeom->type == POLYGONTYPE) ? (const LWPOLY *) lwgeom :
    (const LWPOLY *) ((const LWCOLLECTION *) lwgeom)->geoms[i];
}

/**
 * @brief Return true if a face covers a convex region of the ground
 * @details The face has no inner ring and its ring, read on the ground, turns
 * the same way at every vertex where it turns at all, by the exact signs of
 * #cross_product_sign, so that a face standing upright, whose ground is a
 * segment, covers a convex region as well
 */
static bool
trgeometry_face_ground_convex(const LWPOLY *face)
{
  if (face->nrings != 1)
    return false;
  const POINTARRAY *ring = face->rings[0];
  uint32_t n = ring->npoints - 1;
  int side = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    const POINT2D *a = getPoint2d_cp(ring, i);
    const POINT2D *b = getPoint2d_cp(ring, (i + 1) % n);
    const POINT2D *c = getPoint2d_cp(ring, (i + 2) % n);
    int s = cross_product_sign(a->x, a->y, b->x, b->y, b->x, b->y, c->x, c->y);
    if (s == 0)
      continue;
    if (side == 0)
      side = s;
    else if (s != side)
      return false;
  }
  return true;
}

/**
 * @brief Append a face as the polygon it draws on the ground
 */
static void
trgeometry_face_ground_append(const LWPOLY *face, GSERIALIZED ***buf,
  int *count, int *cap)
{
  LWGEOM *ground = lwgeom_force_2d((const LWGEOM *) face);
  trgeometry_pieces_append(geo_serialize(ground), buf, count, cap);
  lwgeom_free(ground);
}

/**
 * @brief Append the ground a body with Z covers between two placements it
 * translates between, or at one placement where @p b is @p a
 * @details Every point of the body moves along one straight segment, and so
 * does its shadow on the ground, so the ground the body covers is the union
 * over its faces of the ground each face covers. Where the ground of a face
 * is convex (#trgeometry_face_ground_convex) it covers the hull of its ground
 * at the two placements; a face standing upright and moving along its own
 * direction covers a segment or a point, the hull being no polygon, and joins
 * the union as such. Any other face covers its ground at the two placements
 * and what the ground of each edge of its rings sweeps between them, the
 * parallelogram the two positions of the edge bound (#trgeometry_edge_sweep):
 * a point the face reaches in neither placement is reached by its boundary on
 * the way. A ground that is not convex is a polygon, the shadow of a planar
 * face that does not stand upright. No point is constructed but the corners
 * of the hulls, every other one being a vertex of a placement.
 */
static void
trgeometry_ground_sweep(const GSERIALIZED *a, const GSERIALIZED *b,
  GSERIALIZED ***buf, int *count, int *cap)
{
  LWGEOM *lw_a = lwgeom_from_gserialized(a);
  LWGEOM *lw_b = (b == a) ? lw_a : lwgeom_from_gserialized(b);
  int32_t srid = gserialized_get_srid(a);
  int nfaces = (lw_a->type == POLYGONTYPE) ? 1 :
    (int) ((const LWCOLLECTION *) lw_a)->ngeoms;
  for (int f = 0; f < nfaces; f++)
  {
    const LWPOLY *fa = trgeometry_face_n(lw_a, f);
    const LWPOLY *fb = trgeometry_face_n(lw_b, f);
    if (trgeometry_face_ground_convex(fa))
    {
      const POINTARRAY *ra = fa->rings[0], *rb = fb->rings[0];
      POINTARRAY *pa = ptarray_construct_empty(0, 0,
        ra->npoints + rb->npoints);
      POINT4D p = {0, 0, 0, 0};
      for (int k = 0; k < 2; k++)
      {
        const POINTARRAY *r = k ? rb : ra;
        for (uint32_t i = 0; i < r->npoints; i++)
        {
          const POINT2D *q = getPoint2d_cp(r, i);
          p.x = q->x; p.y = q->y;
          ptarray_append_point(pa, &p, LW_TRUE);
        }
      }
      LWGEOM *line = lwline_as_lwgeom(lwline_construct(srid, NULL, pa));
      GSERIALIZED *gs = geo_serialize(line);
      lwgeom_free(line);
      trgeometry_pieces_append(geom_convex_hull(gs), buf, count, cap);
      pfree(gs);
      continue;
    }
    trgeometry_face_ground_append(fa, buf, count, cap);
    if (b == a)
      continue;
    trgeometry_face_ground_append(fb, buf, count, cap);
    for (uint32_t r = 0; r < fa->nrings; r++)
      for (uint32_t i = 0; i + 1 < fa->rings[r]->npoints; i++)
        trgeometry_edge_sweep(srid, getPoint2d_cp(fa->rings[r], i),
          getPoint2d_cp(fa->rings[r], i + 1), getPoint2d_cp(fb->rings[r], i),
          getPoint2d_cp(fb->rings[r], i + 1), buf, count, cap);
  }
  if (lw_b != lw_a)
    lwgeom_free(lw_b);
  lwgeom_free(lw_a);
}

/* The times an interval holding an edge pivoting between two placements is
 * read in halves, the positions of the edge bulging past the two triangles
 * of its sweep (#trgeometry_turn_sweep_halves) */
#define TRGEO_TURN_SPLITS 1
/* The times an interval holding an edge the sweep cannot read is halved */
#define TRGEO_TURN_DEPTH 8

/**
 * @brief Return true if a point lies strictly inside a convex ring, decided
 * by the exact signs of its side of every edge (#cross_product_sign)
 */
static bool
trgeometry_strictly_inside(const POINTARRAY *ring, double x, double y)
{
  uint32_t n = ring->npoints - 1;
  int side = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    const POINT2D *a = getPoint2d_cp(ring, i), *b = getPoint2d_cp(ring, i + 1);
    int s = cross_product_sign(a->x, a->y, b->x, b->y, a->x, a->y, x, y);
    if (s == 0)
      return false;
    if (side == 0)
      side = s;
    else if (s != side)
      return false;
  }
  return true;
}

/**
 * @brief Set @p anchor to a point strictly inside two placements of a convex
 * body: the mean of the vertices of the first, of the second, or the point
 * between the two; false where none of them is
 */
static bool
trgeometry_turn_anchor(const POINTARRAY *ra, const POINTARRAY *rb,
  POINT2D *anchor)
{
  uint32_t n = ra->npoints - 1;
  POINT2D c[3] = {{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}};
  for (uint32_t i = 0; i < n; i++)
  {
    const POINT2D *a = getPoint2d_cp(ra, i), *b = getPoint2d_cp(rb, i);
    c[0].x += a->x / n; c[0].y += a->y / n;
    c[1].x += b->x / n; c[1].y += b->y / n;
  }
  c[2].x = (c[0].x + c[1].x) / 2.0; c[2].y = (c[0].y + c[1].y) / 2.0;
  for (int k = 0; k < 3; k++)
    if (trgeometry_strictly_inside(ra, c[k].x, c[k].y) &&
        trgeometry_strictly_inside(rb, c[k].x, c[k].y))
    {
      *anchor = c[k];
      return true;
    }
  return false;
}

/**
 * @brief Append the region a convex body covers between two placements it
 * takes while it turns, halving the interval where it asks for it
 * @details The region the body sweeps between two placements is the two
 * placements together with what each edge sweeps, every vertex moving along
 * the chord of its arc. An edge whose two positions and the paths of its ends
 * keep apart sweeps the quadrilateral they bound (#trgeometry_edge_sweep).
 * An edge whose two positions cross pivots about their crossing Y, and sweeps
 * the two triangles meeting there, which no double holds. They are read from
 * a point Y' strictly inside both placements instead: the triangle from the
 * path of one end to Y' lies within that end's triangle and the two
 * placements, Y lying on an edge of each, and the two placements and the two
 * triangles from Y' hold the same ground as the two placements and the two
 * triangles from Y. Y' lies inside both placements, so no piece puts a point
 * near an edge it is not on. An interval holding such an edge is read in
 * halves #TRGEO_TURN_SPLITS times, since the positions of a pivoting edge
 * bulge past its two triangles; one holding an edge whose end paths cross,
 * or two placements with no point inside both, is halved until it does not,
 * #TRGEO_TURN_DEPTH times at most, each half at the placement the body takes
 * there. Every piece is ground the body covers.
 * @param[in] temp Temporal rigid geometry
 * @param[in] a,b The two placements
 * @param[in] ta,tb Their timestamps
 * @param[in] depth,split The halvings left
 * @param[in,out] buf,count,cap Pieces
 */
static void
trgeometry_turn_sweep_halves(const Temporal *temp, const GSERIALIZED *a,
  const GSERIALIZED *b, TimestampTz ta, TimestampTz tb, int depth, int split,
  GSERIALIZED ***buf, int *count, int *cap)
{
  LWGEOM *lw_a = lwgeom_from_gserialized(a);
  LWGEOM *lw_b = lwgeom_from_gserialized(b);
  POINTARRAY *ra = ((LWPOLY *) lw_a)->rings[0];
  POINTARRAY *rb = ((LWPOLY *) lw_b)->rings[0];
  uint32_t n = ra->npoints - 1;
  POINT2D anchor;
  bool anchored = trgeometry_turn_anchor(ra, rb, &anchor);
  /* What each edge sweeps: 0 its quadrilateral, 1 its two triangles from the
   * anchor, 2 nothing this interval can read */
  int *kind = palloc(sizeof(int) * n);
  bool pivot = false, unread = false;
  for (uint32_t i = 0; i < n; i++)
  {
    const POINT2D *a0 = getPoint2d_cp(ra, i), *b0 = getPoint2d_cp(ra, i + 1);
    const POINT2D *a1 = getPoint2d_cp(rb, i), *b1 = getPoint2d_cp(rb, i + 1);
    bool paths = trgeometry_segm_cross(a0, a1, b0, b1);
    bool positions = trgeometry_segm_cross(a0, b0, a1, b1);
    if (paths)
      kind[i] = 2;
    else if (! positions)
      kind[i] = 0;
    else
      kind[i] = anchored ? 1 : 2;
    pivot |= (kind[i] == 1);
    unread |= (kind[i] == 2);
  }
  /* The interval in halves, at the placement the body takes halfway */
  bool halve = (pivot && split > 0) || (unread && depth > 0);
  Datum value;
  if (halve && tb - ta > 1 &&
      trgeometry_value_at_timestamptz_intl(temp, ta + (tb - ta) / 2, false,
        &value))
  {
    GSERIALIZED *mid = DatumGetGserializedP(value);
    TimestampTz tm = ta + (tb - ta) / 2;
    int d = (pivot && split > 0) ? depth : depth - 1;
    int s = (pivot && split > 0) ? split - 1 : split;
    trgeometry_turn_sweep_halves(temp, a, mid, ta, tm, d, s, buf, count, cap);
    trgeometry_turn_sweep_halves(temp, mid, b, tm, tb, d, s, buf, count, cap);
    pfree(mid);
  }
  else
  {
    int32_t srid = gserialized_get_srid(a);
    trgeometry_pieces_append(geo_copy(b), buf, count, cap);
    for (uint32_t i = 0; i < n; i++)
    {
      const POINT2D *a0 = getPoint2d_cp(ra, i), *b0 = getPoint2d_cp(ra, i + 1);
      const POINT2D *a1 = getPoint2d_cp(rb, i), *b1 = getPoint2d_cp(rb, i + 1);
      if (kind[i] == 0)
        trgeometry_edge_sweep(srid, a0, b0, a1, b1, buf, count, cap);
      else if (kind[i] == 1)
      {
        trgeometry_triangle_append(srid, a0, a1, &anchor, buf, count, cap);
        trgeometry_triangle_append(srid, b0, b1, &anchor, buf, count, cap);
      }
    }
  }
  pfree(kind);
  lwgeom_free(lw_a); lwgeom_free(lw_b);
}

/**
 * @brief Append the region a convex body covers between two placements it
 * takes while it turns (#trgeometry_turn_sweep_halves)
 * @details Every vertex is read as moving along the chord of its arc, and
 * every piece is ground the body covers: the area holds no point the body
 * never reaches. What the chords leave out is the bulge of the arcs past
 * them, which the step of the placements bounds
 * (#trgeometry_trav_segment)
 * @param[in] temp Temporal rigid geometry
 * @param[in] a,b The two placements
 * @param[in] ta,tb Their timestamps
 * @param[in] with_a True to append the placement @p a as well, which no
 * earlier piece holds
 * @param[in,out] buf,count,cap Pieces
 */
static void
trgeometry_turn_sweep(const Temporal *temp, const GSERIALIZED *a,
  const GSERIALIZED *b, TimestampTz ta, TimestampTz tb, bool with_a,
  GSERIALIZED ***buf, int *count, int *cap)
{
  if (with_a)
    trgeometry_pieces_append(geo_copy(a), buf, count, cap);
  trgeometry_turn_sweep_halves(temp, a, b, ta, tb, TRGEO_TURN_DEPTH,
    TRGEO_TURN_SPLITS, buf, count, cap);
}

/**
 * @brief Return the placements a temporal rigid geometry passes through
 * @details Every input instant contributes its placement, and a segment
 * carries as many placements between two of them as its rotation asks for at
 * `TRGEO_TRAVERSED_AREA_ANGLE_TOL` of turn each. The trgeometry-specific step
 * is materialising the reference geometry at every emitted sample via
 * `pose_apply_geo`
 * @param[in] temp Temporal rigid geometry
 * @param[out] pl Placements, each with its timestamp and turn
 * @return False if the value has no instant
 */
static bool
trgeometry_placement_array(const Temporal *temp, TrgeoPlacements *pl)
{
  int n_insts = 0;
  /* temporal_insts_p returns a pointer-to-array of in-place instants;
   * neither the elements nor the array itself need element-freeing —
   * we only pfree the array wrapper at the end. */
  const TInstant **insts = temporal_insts_p(temp, &n_insts);
  if (! insts || n_insts <= 0)
    return false;

  /* Initial capacity is generous — bisection rarely goes deep. */
  pl->cap = (n_insts > 1) ? n_insts * 4 : n_insts;
  pl->geoms = palloc(sizeof(GSERIALIZED *) * pl->cap);
  pl->times = palloc(sizeof(TimestampTz) * pl->cap);
  pl->turns = palloc(sizeof(bool) * pl->cap);
  pl->count = 0;
  /* The reference geometry is the body about the point its pose places, so
   * the radius is the greatest distance of a vertex from the origin */
  pl->radius = 0.0;
  LWGEOM *ref = lwgeom_from_gserialized(trgeometry_geom_p(temp));
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
  LWPOINTITERATOR *it = lwpointiterator_create(ref);
  POINT4D p;
  while (lwpointiterator_next(it, &p) == LW_SUCCESS)
    pl->radius = fmax(pl->radius, hasz ?
      sqrt(p.x * p.x + p.y * p.y + p.z * p.z) : hypot(p.x, p.y));
  lwpointiterator_destroy(it);
  lwgeom_free(ref);

  /* Emit the first instant. */
  trgeometry_trav_emit_at(temp, insts[0]->t, false, pl);
  /* Per inter-instant segment, recursively bisect. The recursion
   * appends samples on (t_m, t_b], reusing the t_a sample emitted by
   * the previous iteration. */
  for (int i = 0; i + 1 < n_insts; i++)
    trgeometry_trav_segment(temp, insts[i], insts[i + 1], pl);

  pfree(insts);
  return true;
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
trgeometry_geoms_merge(GSERIALIZED **geoms, int n_geoms, bool unary_union)
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
      result = geom_unary_union(coll);
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
 * @errval NULL
 */
GSERIALIZED *
trgeometry_placements(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  TrgeoPlacements pl;
  if (! trgeometry_placement_array(temp, &pl))
    return NULL;
  pfree(pl.times); pfree(pl.turns);
  return trgeometry_geoms_merge(pl.geoms, pl.count, UNARY_UNION_NO);
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
 * come from #trgeometry_placements(); a convex body then covers what it
 * sweeps between every consecutive pair, and the unary union dissolves the
 * overlap the pieces share. The united form of a body that translates is
 * exact. Where the body turns, every vertex is read as moving along the chord
 * of its arc between two placements a turn of
 * `TRGEO_TRAVERSED_AREA_ANGLE_TOL` apart, and nearer where the body travels
 * far while it turns (#trgeometry_trav_segment), so the area holds only
 * ground the body covers and leaves out the bulge of the arcs past their
 * chords, at most 5.8e-4 of the diameter of the body over the 400 turning
 * rows of the corpus of the exact rigid distance (#trgeometry_turn_sweep).
 * The united form of a value with Z is the ground it covers, with no Z
 * (#trgeometry_ground_sweep), and is refused for a body turning in linear
 * motion
 */
GSERIALIZED *
trgeometry_traversed_area(const Temporal *temp, bool unary_union)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  TrgeoPlacements pl;
  if (! trgeometry_placement_array(temp, &pl))
    return NULL;
  GSERIALIZED **geoms = pl.geoms;
  int n_geoms = pl.count;

  /* The samples are the placements the body passes through; the area it
   * traverses is what it covers BETWEEN them as well. A convex body covers
   * what it sweeps between each consecutive pair, the hull of the pair where
   * it translates and #trgeometry_turn_sweep where it turns, which is the
   * case the traversed area is defined for; a body with a concavity keeps the
   * placements alone, as its hull would close a concavity the body never
   * reaches.
   * A body that does not interpolate covers nothing between its placements:
   * it holds each of them for the whole of its own interval and then stands
   * at the next, so the placements ARE the area it traverses and a hull
   * spanning two of them answers for ground the body never crosses.
   * Only the united form sweeps; the collection form answers the placements
   * themselves, which #trgeometry_placements() gives a caller directly. */
  /* A polygon that is not convex, concave or holed, sweeps more than the
   * hull of two placements and less than their union reports, so its united
   * form in linear motion is refused rather than answered by its placements.
   * A value with Z covers the ground its shadow sweeps, and the shadow of a
   * body that turns in linear motion is refused as well */
  bool linear = unary_union && n_geoms > 1 &&
    MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);
  const char *refused = NULL;
  if (linear && hasz && ! trgeometry_translates(temp))
    refused = "The united traversed area of a temporal rigid geometry with Z "
      "in linear motion that turns is not implemented";
  else if (linear && ! hasz &&
      gserialized_get_type(trgeometry_geom_p(temp)) == POLYGONTYPE &&
      ! trgeometry_ref_is_convex(trgeometry_geom_p(temp)))
    refused = "The united traversed area of a temporal rigid geometry in "
      "linear motion whose body is not convex is not implemented";
  if (refused)
  {
    for (int i = 0; i < n_geoms; i++)
      pfree(geoms[i]);
    pfree(geoms); pfree(pl.times); pfree(pl.turns);
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED, "%s", refused);
    return NULL;
  }
  if (unary_union && hasz)
  {
    /* The united form of a value with Z is the ground it covers: what each
     * face sweeps between every pair of placements the body translates
     * between, or what each face covers at every placement of a value that
     * does not interpolate */
    int cap = n_geoms * 2, n_swept = 0;
    GSERIALIZED **swept = palloc(sizeof(GSERIALIZED *) * cap);
    if (linear)
      for (int i = 0; i + 1 < n_geoms; i++)
        trgeometry_ground_sweep(geoms[i], geoms[i + 1], &swept, &n_swept,
          &cap);
    else
      for (int i = 0; i < n_geoms; i++)
        trgeometry_ground_sweep(geoms[i], geoms[i], &swept, &n_swept, &cap);
    for (int i = 0; i < n_geoms; i++)
      pfree(geoms[i]);
    pfree(geoms);
    geoms = swept;
    n_geoms = n_swept;
  }
  else if (linear && trgeometry_ref_is_convex(trgeometry_geom_p(temp)))
  {
    int cap = n_geoms * 2, n_swept = 0;
    GSERIALIZED **swept = palloc(sizeof(GSERIALIZED *) * cap);
    for (int i = 0; i + 1 < n_geoms; i++)
    {
      /* A pair the body turns between; the first placement is held by no
       * earlier piece only at the start, each later one ending the pair
       * before it */
      if (pl.turns[i + 1])
        trgeometry_turn_sweep(temp, geoms[i], geoms[i + 1], pl.times[i],
          pl.times[i + 1], i == 0, &swept, &n_swept, &cap);
      else
        trgeometry_pieces_append(trgeometry_convex_sweep(geoms[i],
          geoms[i + 1]), &swept, &n_swept, &cap);
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

  pfree(pl.times); pfree(pl.turns);
  return trgeometry_geoms_merge(geoms, n_geoms, unary_union);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_conversion
 * @brief Materialise the moving polygon of a temporal rigid geometry as a
 * temporal geometry (one rotated/translated polygon per instant)
 * @details For each instant, applies the instant's pose to the trgeo's
 * reference geometry and emits the resulting polygon. The returned
 * `tgeometry` has the same temporal structure as the input and the same
 * SRID as the reference geometry.
 *
 * A temporal geometry keeps its value from one instant to the next, so it
 * states a rigid geometry with step or discrete interpolation. A rigid
 * geometry with linear interpolation moves and turns between its instants,
 * which no temporal geometry states, and the function returns an error for
 * it.
 * @note Unlike `trgeometry_to_tgeompoint` (which gives the antenna point trajectory),
 * this preserves the body's footprint at each instant — the natural input
 * for compositional spatial-rel queries against `tgeometry`'s full surface.
 * @param[in] temp Temporal rigid geometry
 * @errval NULL
 * @csqlfn #Trgeometry_to_tgeometry()
 */
Temporal *
trgeometry_to_tgeometry(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_has_geom(temp->flags) ||
      ! ensure_nonlinear_interp(temp->flags))
    return NULL;
  const GSERIALIZED *ref = trgeometry_geom_p(temp);
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

/**
 * @ingroup meos_rgeo_conversion
 * @brief Convert a temporal rigid geometry into a temporal geography
 * @details The value converts through #trgeometry_to_tgeometry and #tgeometry_to_tgeography, as every spatial type
 * does
 * @param[in] temp Temporal value
 * @errval NULL
 * @csqlfn #Trgeometry_to_tgeography()
 */
Temporal *
trgeometry_to_tgeography(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *value = trgeometry_to_tgeometry(temp);
  if (! value)
    return NULL;
  Temporal *result = tgeometry_to_tgeography(value);
  pfree(value);
  return result;
}

/*****************************************************************************/
