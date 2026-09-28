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
 * @brief Distance functions for temporal rigid geometries
 * @details The temporal distance comes from the closest-feature walk of
 * trgeo_distwalk.c, which needs convex bodies.  See
 * meos/src/rgeo/trgeo_distance.txt for the algorithm.
 */

#include "rgeo/trgeo_distance.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "temporal/temporal_aggfuncs.h"
#include "temporal/temporal_compops.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_distwalk.h"

/*****************************************************************************
 * Bodies
 *****************************************************************************/

/**
 * @brief Set the body of the reference geometry of a temporal rigid geometry
 * @details The walk needs a convex body.  A polygon that is not convex or has
 * holes gives an error, also where the walk is not necessary, so that the
 * distance functions accept the same values in all cases.
 * @return False, with an error, if the reference geometry is not a convex
 * polygon without holes
 */
static bool
dist_ref_body(const GSERIALIZED *ref_gs, DistRefPoly *rp)
{
  LWGEOM *geom = lwgeom_from_gserialized(ref_gs);
  bool result = distrefpoly_set(rp, lwgeom_as_lwpoly(geom));
  lwgeom_free(geom);
  if (! result)
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The distance of a temporal rigid geometry needs a reference geometry "
      "that is a convex polygon without holes");
  return result;
}

/**
 * @brief A convex part of a static geometry
 */
typedef struct
{
  DistRefPoly body;          /**< Point, segment or convex polygon */
  DistMotion motion;         /**< Static motion at the center of the body */
} DistPart;

/**
 * @brief Add a point part to an array of parts
 */
static void
dist_part_point(DistPart *parts, int *n, double x, double y)
{
  distrefpoly_set_point(&parts[*n].body);
  distmotion_set_static(&parts[*n].motion, x, y);
  (*n)++;
}

/**
 * @brief Add the parts of a simple geometry to an array of parts
 * @details A point is one part, a line is one part for each segment, and a
 * polygon is one part.  A segment of length zero is a point.
 * @return False, with an error, if a polygon is not convex or has holes
 */
static bool
dist_parts_add(const LWGEOM *geom, DistPart *parts, int *n)
{
  if (lwgeom_is_empty(geom))
    return true;
  if (geom->type == POINTTYPE)
  {
    const POINT2D *p = getPoint2d_cp(((LWPOINT *) geom)->point, 0);
    dist_part_point(parts, n, p->x, p->y);
    return true;
  }
  if (geom->type == LINETYPE)
  {
    const POINTARRAY *pa = ((LWLINE *) geom)->points;
    for (uint32_t i = 0; i + 1 < pa->npoints; i++)
    {
      const POINT2D *p = getPoint2d_cp(pa, i);
      const POINT2D *q = getPoint2d_cp(pa, i + 1);
      if (p->x == q->x && p->y == q->y)
        dist_part_point(parts, n, p->x, p->y);
      else
      {
        double cx, cy;
        distrefpoly_set_segment(&parts[*n].body, p->x, p->y, q->x, q->y, &cx,
          &cy);
        distmotion_set_static(&parts[*n].motion, cx, cy);
        (*n)++;
      }
    }
    if (pa->npoints == 1)
    {
      const POINT2D *p = getPoint2d_cp(pa, 0);
      dist_part_point(parts, n, p->x, p->y);
    }
    return true;
  }
  /* POLYGONTYPE */
  double cx, cy;
  if (! distrefpoly_set_static(&parts[*n].body, (LWPOLY *) geom, &cx, &cy))
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The distance of a temporal rigid geometry to a polygon that is not "
      "convex or has holes is not supported");
    return false;
  }
  distmotion_set_static(&parts[*n].motion, cx, cy);
  (*n)++;
  return true;
}

/**
 * @brief Return the convex parts of a static geometry, and their number in
 * @p count
 * @details The walk finds the distance to a convex body.  A point, a
 * segment and a convex polygon are convex bodies, thus the distance to a
 * geometry is the minimum of the distances to its parts.
 * @return The parts, or NULL with an error if the geometry has a polygon that
 * is not convex or has holes, or if its type is not supported
 */
static DistPart *
dist_geo_parts(const GSERIALIZED *gs, int *count)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  uint32_t type = geom->type;
  if (type != POINTTYPE && type != LINETYPE && type != POLYGONTYPE &&
      type != MULTIPOINTTYPE && type != MULTILINETYPE &&
      type != MULTIPOLYGONTYPE)
  {
    lwgeom_free(geom);
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Unsupported geometry type: %s", lwtype_name(type));
    return NULL;
  }
  /* The number of parts is at most the number of points */
  int maxparts = (int) lwgeom_count_vertices(geom) + 1;
  DistPart *parts = palloc(sizeof(DistPart) * maxparts);
  *count = 0;
  bool ok = true;
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = lwgeom_as_lwcollection(geom);
    for (uint32_t i = 0; ok && i < coll->ngeoms; i++)
      ok = dist_parts_add(coll->geoms[i], parts, count);
  }
  else
    ok = dist_parts_add(geom, parts, count);
  lwgeom_free(geom);
  if (! ok)
  {
    for (int i = 0; i < *count; i++)
      distrefpoly_free(&parts[i].body);
    pfree(parts);
    return NULL;
  }
  return parts;
}

/**
 * @brief Free an array of parts
 */
static void
dist_parts_free(DistPart *parts, int count)
{
  for (int i = 0; i < count; i++)
    distrefpoly_free(&parts[i].body);
  pfree(parts);
}

/*****************************************************************************
 * Distance at an instant
 *****************************************************************************/

/**
 * @brief The operands of a temporal distance
 * @details The first operand is always a temporal rigid geometry.  The second
 * operand is a static geometry, a temporal point or a temporal rigid
 * geometry.
 */
typedef struct
{
  const GSERIALIZED *ref1;   /**< Reference geometry of the first operand */
  DistRefPoly body1;         /**< Body of the first operand */
  const GSERIALIZED *gs;     /**< Static geometry, or NULL */
  DistPart *parts;           /**< Parts of the static geometry */
  int nparts;                /**< Number of parts */
  const GSERIALIZED *ref2;   /**< Reference geometry of a temporal rigid
                                  geometry, or NULL for a temporal point */
  DistRefPoly body2;         /**< Body of a temporal point or temporal rigid
                                  geometry */
  double level;              /**< Distance whose crossings are events, or a
                                  negative value, see distwalk_segm() */
} DistOps;

/**
 * @brief Return the distance between the operands at the instant @p inst1 of
 * the first operand and @p inst2 of the second operand
 * @details The function places the bodies and uses PostGIS.  @p inst2 is
 * NULL for a static geometry.
 */
static double
dist_instant(const DistOps *ops, const TInstant *inst1, const TInstant *inst2)
{
  GSERIALIZED *geo1 = pose_apply_geo(DatumGetPoseP(tinstant_value_p(inst1)),
    ops->ref1);
  double result;
  if (ops->gs)
    result = geom_distance2d(geo1, ops->gs);
  else if (! ops->ref2)
    result = geom_distance2d(geo1,
      DatumGetGserializedP(tinstant_value_p(inst2)));
  else
  {
    GSERIALIZED *geo2 = pose_apply_geo(
      DatumGetPoseP(tinstant_value_p(inst2)), ops->ref2);
    result = geom_distance2d(geo1, geo2);
    pfree(geo2);
  }
  pfree(geo1);
  return result;
}

/*****************************************************************************
 * Distance of a sequence
 *****************************************************************************/

/**
 * @brief Set the motion in segment @p i of a sequence of poses or points
 * @details With step interpolation, the value keeps its value at instant
 * @p i during the segment.  A point is a body without rotation.
 */
static void
dist_segmotion(DistMotion *m, const TSequence *seq, int i, bool ispose)
{
  int j = MEOS_FLAGS_LINEAR_INTERP(seq->flags) ? i + 1 : i;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, j);
  if (ispose)
  {
    distmotion_from_pose(m, DatumGetPoseP(tinstant_value_p(inst1)),
      DatumGetPoseP(tinstant_value_p(inst2)));
    return;
  }
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(
    DatumGetGserializedP(tinstant_value_p(inst1)));
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(
    DatumGetGserializedP(tinstant_value_p(inst2)));
  distmotion_set(m, p1->x, p1->y, 0.0, p2->x, p2->y, 0.0);
}

/**
 * @brief Return the instants of the events of a walk in segment @p i
 * @details An event at the end of the segment is at the instant i + 1.
 */
static TimestampTz
dist_event_time(const TSequence *seq, int i, double ratio)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
  if (ratio <= 0.0)
    return inst1->t;
  if (ratio >= 1.0)
    return inst2->t;
  /* The nearest microsecond */
  return inst1->t + (TimestampTz) llround((double) (inst2->t - inst1->t) *
    ratio);
}

/**
 * @brief Add an instant to an array of instants
 * @details Two events at the same timestamp become one instant with the
 * smaller distance.  This happens at the end of a segment, which is also the
 * start of the next one, and for two events closer than one microsecond.
 */
static void
dist_append(TInstant ***insts, int *count, int *size, double dist,
  TimestampTz t)
{
  if (*count > 0 && (*insts)[*count - 1]->t >= t)
  {
    if (dist < DatumGetFloat8(tinstant_value_p((*insts)[*count - 1])))
    {
      pfree((*insts)[*count - 1]);
      (*insts)[*count - 1] = tinstant_make(Float8GetDatum(dist), T_TFLOAT,
        t);
    }
    return;
  }
  if (*count == *size)
  {
    *size *= 2;
    *insts = repalloc(*insts, sizeof(TInstant *) * *size);
  }
  (*insts)[(*count)++] = tinstant_make(Float8GetDatum(dist), T_TFLOAT, t);
}

/**
 * @brief Return the temporal distance between the first operand @p seq1 and
 * the body @p rb over the segments of @p seq1
 * @details If @p seq2 is NULL, @p rb is static with the motion @p mb.  Else
 * @p seq2 gives the motion of @p rb and has the same timestamps as
 * @p seq1.  A sequence with step interpolation keeps its value during each
 * segment.
 *
 * If one sequence has linear interpolation and the other has step
 * interpolation, the distance jumps at each instant.  Then the result is a
 * sequence set with one sequence [t_i, t_{i+1}) for each segment, and a last
 * sequence with the last instant, as for the other temporal types.  Else the
 * result is one sequence.
 *
 * The result is not normalized, see dist_normalize().
 *
 * A failure of the walk raises an error.  It is not a property of the data
 * but a defect.
 */
static Temporal *
dist_walk_seq(const DistOps *ops, const TSequence *seq1,
  const TSequence *seq2, const DistRefPoly *rb, const DistMotion *mb)
{
  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool jumps = seq2 && (linear1 != MEOS_FLAGS_LINEAR_INTERP(seq2->flags));
  DistEvents events;
  distevents_init(&events);
  int size = seq1->count * 4, count = 0, npieces = 0;
  TInstant **insts = palloc(sizeof(TInstant *) * size);
  TSequence **pieces = jumps ?
    palloc(sizeof(TSequence *) * seq1->count) : NULL;
  DistPair cf;
  cf.kind = DISTPAIR_NONE;
  cf.i = cf.j = 0;

  for (int i = 0; i < seq1->count - 1; i++)
  {
    DistMotion ma, mbi;
    dist_segmotion(&ma, seq1, i, true);
    if (seq2)
      dist_segmotion(&mbi, seq2, i, ops->ref2 != NULL);
    else
      mbi = *mb;
    DistSegm dseg;
    distsegm_set(&dseg, &ma, &mbi);
    events.count = 0;
    int rc = distwalk_segm(&dseg, &ops->body1, rb,
      distwalk_ftol(&dseg, &ops->body1, rb), ops->level, &cf, &events);
    if (rc != DISTWALK_OK)
    {
      distevents_free(&events);
      pfree_array((void **) insts, count);
      if (pieces)
        pfree_array((void **) pieces, npieces);
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed at "
        "segment %d", i);
      return NULL;
    }
    for (int j = 0; j < events.count; j++)
      dist_append(&insts, &count, &size, events.ev[j].dist,
        dist_event_time(seq1, i, events.ev[j].t));
    if (jumps)
    {
      /* The segment gives the sequence [t_i, t_{i+1}) */
      pieces[npieces++] = tsequence_make(insts, count,
        (i == 0) ? seq1->period.lower_inc : true, false, LINEAR,
        NORMALIZE_NO);
      pfree_array((void **) insts, count);
      insts = palloc(sizeof(TInstant *) * size);
      count = 0;
    }
  }
  distevents_free(&events);

  if (! jumps)
    return (Temporal *) tsequence_make_free(insts, count,
      seq1->period.lower_inc, seq1->period.upper_inc, LINEAR, NORMALIZE_NO);

  pfree(insts);
  if (seq1->period.upper_inc)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, seq1->count - 1);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, seq2->count - 1);
    TInstant *last = tinstant_make(Float8GetDatum(dist_instant(ops, inst1,
      inst2)), T_TFLOAT, inst1->t);
    pieces[npieces++] = tinstant_to_tsequence_free(last, LINEAR);
  }
  return (Temporal *) tsequenceset_make_free(pieces, npieces, NORMALIZE_NO);
}

/**
 * @brief Return the value at @p t of the linear sequence @p seq, whose
 * instants @p i - 1 and @p i are before and after @p t
 */
static double
dist_seq_interp(const TSequence *seq, int i, TimestampTz t)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, i - 1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
  double v1 = DatumGetFloat8(tinstant_value_p(inst1));
  double v2 = DatumGetFloat8(tinstant_value_p(inst2));
  return v1 + (v2 - v1) * ((double) (t - inst1->t) /
    (double) (inst2->t - inst1->t));
}

/**
 * @brief Return the pointwise minimum of two temporal float sequences with
 * linear interpolation
 * @details The result has the instants of the two sequences, and an instant
 * where their linear interpolations cross, at the nearest microsecond.  It
 * is not normalized, so that it keeps the extrema and the crossings of a
 * threshold of each sequence, see dist_normalize().
 * @pre The two sequences have the same period
 */
static TSequence *
tfloatseq_min_tfloatseq(const TSequence *seq1, const TSequence *seq2)
{
  TInstant **insts = palloc(sizeof(TInstant *) *
    2 * (seq1->count + seq2->count));
  int i = 0, j = 0, n = 0;
  double prev1 = 0.0, prev2 = 0.0;
  TimestampTz prevt = 0;
  /* The sequences have the same first and last instants, thus they end
   * together, and a sequence without an instant at t has one before and one
   * after t */
  while (i < seq1->count && j < seq2->count)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
    /* The next timestamp is an instant of one sequence or of both */
    bool at1 = (inst1->t <= inst2->t), at2 = (inst2->t <= inst1->t);
    TimestampTz t = at1 ? inst1->t : inst2->t;
    double v1 = at1 ? DatumGetFloat8(tinstant_value_p(inst1)) :
      dist_seq_interp(seq1, i, t);
    double v2 = at2 ? DatumGetFloat8(tinstant_value_p(inst2)) :
      dist_seq_interp(seq2, j, t);
    if (at1)
      i++;
    if (at2)
      j++;
    /* The minimum has a corner where the two interpolations cross */
    double d0 = prev1 - prev2, d1 = v1 - v2;
    if (n > 0 && ((d0 < 0.0 && d1 > 0.0) || (d0 > 0.0 && d1 < 0.0)))
    {
      double ratio = d0 / (d0 - d1);
      TimestampTz tc = prevt + (TimestampTz) llround((double) (t - prevt) *
        ratio);
      if (tc > prevt && tc < t)
      {
        double vc = prev1 + (v1 - prev1) * ((double) (tc - prevt) /
          (double) (t - prevt));
        insts[n++] = tinstant_make(Float8GetDatum(vc), T_TFLOAT, tc);
      }
    }
    insts[n++] = tinstant_make(Float8GetDatum(fmin(v1, v2)), T_TFLOAT, t);
    prev1 = v1;
    prev2 = v2;
    prevt = t;
  }
  return tsequence_make_free(insts, n, seq1->period.lower_inc,
    seq1->period.upper_inc, LINEAR, NORMALIZE_NO);
}

/**
 * @brief Set the value of each instant of @p dist to the exact distance
 * between the rigid geometry @p seq and the geometry @p gs
 * @details The minimum of the distances to the parts has instants where two
 * linear interpolations cross.  The interpolated value there is not the
 * distance, thus the function computes the distance at each instant with
 * PostGIS.  Then the result is exact at each instant, as for one part.
 *
 * An instant whose value is exactly @p level keeps it.  Such a value comes
 * from the crossing of @p level by the part that is closest at that time, see
 * distwalk_segm(), thus it is the exact distance.  A value from PostGIS has a
 * rounding error, which can put the crossing in the same microsecond as the
 * instant, where the comparison with @p level does not see it.
 * @param[in] dist Minimum of the distances to the parts, freed
 */
static TSequence *
dist2d_seq_exact_values(TSequence *dist, const TSequence *seq,
  const GSERIALIZED *gs, const GSERIALIZED *ref_gs, double level)
{
  TInstant **instants = palloc(sizeof(TInstant *) * dist->count);
  for (int i = 0; i < dist->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(dist, i);
    if (level >= 0.0 && DatumGetFloat8(tinstant_value_p(inst)) == level)
    {
      instants[i] = tinstant_copy(inst);
      continue;
    }
    Datum pose;
    /* The instants of the minimum are in the period of the rigid geometry */
    if (! tsequence_value_at_timestamptz(seq, inst->t, false, &pose))
    {
      instants[i] = tinstant_copy(inst);
      continue;
    }
    GSERIALIZED *body = pose_apply_geo(DatumGetPoseP(pose), ref_gs);
    instants[i] = tinstant_make(Float8GetDatum(geom_distance2d(body, gs)),
      T_TFLOAT, inst->t);
    pfree(body); pfree(DatumGetPointer(pose));
  }
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc, LINEAR, NORMALIZE_NO);
  pfree(dist);
  return result;
}

/**
 * @brief Return the temporal distance between two operands over one
 * sequence
 * @details If no operand has linear interpolation, the distance is the
 * distance at each instant.  Else the walk gives it.  For a static geometry,
 * the result is the minimum of the distances to its parts.
 * @param[in] seq1 Sequence of the rigid geometry
 * @param[in] seq2 Sequence of the second operand with the same timestamps,
 * or NULL for a static geometry
 */
static Temporal *
dist_seq(const DistOps *ops, const TSequence *seq1, const TSequence *seq2)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq1->flags) ||
    (seq2 && MEOS_FLAGS_LINEAR_INTERP(seq2->flags));
  if (seq1->count == 1 || ! linear)
  {
    /* Discrete interpolation if one operand has it, else step */
    interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);
    if (seq2 && MEOS_FLAGS_GET_INTERP(seq2->flags) == DISCRETE)
      interp = DISCRETE;
    TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
    for (int i = 0; i < seq1->count; i++)
    {
      const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
      instants[i] = tinstant_make(Float8GetDatum(dist_instant(ops, inst1,
        seq2 ? TSEQUENCE_INST_N(seq2, i) : NULL)), T_TFLOAT, inst1->t);
    }
    return (Temporal *) tsequence_make_free(instants, seq1->count,
      seq1->period.lower_inc, seq1->period.upper_inc, interp, NORMALIZE);
  }
  if (seq2)
    return dist_walk_seq(ops, seq1, seq2, &ops->body2, NULL);

  TSequence *result = NULL;
  for (int k = 0; k < ops->nparts; k++)
  {
    TSequence *dist = (TSequence *) dist_walk_seq(ops, seq1, NULL,
      &ops->parts[k].body, &ops->parts[k].motion);
    if (! dist)
    {
      if (result)
        pfree(result);
      return NULL;
    }
    if (! result)
      result = dist;
    else
    {
      TSequence *min = tfloatseq_min_tfloatseq(result, dist);
      pfree(result); pfree(dist);
      result = min;
    }
  }
  if (ops->nparts > 1)
    result = dist2d_seq_exact_values(result, seq1, ops->gs, ops->ref1,
      ops->level);
  return (Temporal *) result;
}

/**
 * @brief Return the temporal distance between two operands
 * @param[in] temp1 Temporal rigid geometry
 * @param[in] temp2 Second operand with the same subtype and timestamps, or
 * NULL for a static geometry
 */
static Temporal *
dist_temporal(const DistOps *ops, const Temporal *temp1, const Temporal *temp2)
{
  assert(temptype_subtype(temp1->subtype));
  if (temp1->subtype == TINSTANT)
  {
    const TInstant *inst1 = (const TInstant *) temp1;
    return (Temporal *) tinstant_make(Float8GetDatum(dist_instant(ops, inst1,
      (const TInstant *) temp2)), T_TFLOAT, inst1->t);
  }
  if (temp1->subtype == TSEQUENCE)
    return dist_seq(ops, (const TSequence *) temp1,
      (const TSequence *) temp2);
  /* TSEQUENCESET */
  const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
  const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
  int size = ss1->totalcount, count = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * size);
  for (int i = 0; i < ss1->count; i++)
  {
    Temporal *dist = dist_seq(ops, TSEQUENCESET_SEQ_N(ss1, i),
      ss2 ? TSEQUENCESET_SEQ_N(ss2, i) : NULL);
    if (! dist)
    {
      pfree_array((void **) sequences, count);
      return NULL;
    }
    /* The distance of a sequence is a sequence or a sequence set */
    if (dist->subtype == TSEQUENCE)
      sequences[count++] = (TSequence *) dist;
    else
    {
      const TSequenceSet *ss = (const TSequenceSet *) dist;
      if (count + ss->count > size)
      {
        size = 2 * (count + ss->count);
        sequences = repalloc(sequences, sizeof(TSequence *) * size);
      }
      for (int j = 0; j < ss->count; j++)
        sequences[count++] = tsequence_copy(TSEQUENCESET_SEQ_N(ss, j));
      pfree(dist);
    }
  }
  return (Temporal *) tsequenceset_make_free(sequences, count, NORMALIZE_NO);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a geometry
 * @param[in] level Distance whose crossings are instants of the result, or a
 * negative value.  Then the result is on the same side of @p level as the
 * distance at each time, which the threshold functions such as
 * #tdwithin_trgeometry_geo() need.
 */
Temporal *
trgeo_tdistance_geo(const Temporal *temp, const GSERIALIZED *gs,
  double level)
{
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;
  if (MEOS_FLAGS_GET_Z(temp->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Distance computation in 3D is not currently supported");
    return NULL;
  }
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeo_geom_p(temp);
  ops.gs = gs;
  ops.level = level;
  if (! dist_ref_body(ops.ref1, &ops.body1))
    return NULL;
  ops.parts = dist_geo_parts(gs, &ops.nparts);
  Temporal *result = NULL;
  if (ops.parts)
  {
    result = dist_temporal(&ops, temp, NULL);
    dist_parts_free(ops.parts, ops.nparts);
  }
  distrefpoly_free(&ops.body1);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point or another temporal rigid geometry
 * @param[in] level See #trgeo_tdistance_geo()
 */
static Temporal *
trgeo_tdistance_temporal(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  if (MEOS_FLAGS_GET_Z(temp1->flags) || MEOS_FLAGS_GET_Z(temp2->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Distance computation in 3D is not currently supported");
    return NULL;
  }
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeo_geom_p(temp1);
  ops.ref2 = (temp2->temptype == T_TRGEOMETRY) ? trgeo_geom_p(temp2) : NULL;
  ops.level = level;
  if (! dist_ref_body(ops.ref1, &ops.body1))
    return NULL;
  if (ops.ref2)
  {
    if (! dist_ref_body(ops.ref2, &ops.body2))
    {
      distrefpoly_free(&ops.body1);
      return NULL;
    }
  }
  else
    distrefpoly_set_point(&ops.body2);

  /* The two values must have the same timestamps.  The synchronization does
   * not add the times at which they cross, which the walk finds. */
  Temporal *sync1, *sync2;
  Temporal *result = NULL;
  if (intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
  {
    result = dist_temporal(&ops, sync1, sync2);
    pfree(sync1); pfree(sync2);
  }
  distrefpoly_free(&ops.body1);
  distrefpoly_free(&ops.body2);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point, with the crossings of @p level as instants, see
 * #trgeo_tdistance_geo()
 */
Temporal *
trgeo_tdistance_tpoint(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;
  return trgeo_tdistance_temporal(temp1, temp2, level);
}

/**
 * @brief Return the temporal distance between two temporal rigid geometries,
 * with the crossings of @p level as instants, see #trgeo_tdistance_geo()
 */
Temporal *
trgeo_tdistance_trgeo(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;
  return trgeo_tdistance_temporal(temp1, temp2, level);
}

/**
 * @brief Return a temporal distance with its sequences normalized
 * @details The internal functions keep each instant of the walk.  The
 * normalization of MEOS removes an instant whose value is within
 * #MEOS_EPSILON of the line through its neighbors, and such an instant can
 * be the minimum or a crossing of a threshold.  Thus the nearest approach
 * and the dwithin functions use the internal functions, and only the result
 * of the temporal distance functions is normalized, as are the other
 * temporal distances of MEOS.
 * @param[in] temp Temporal distance, freed
 */
static Temporal *
dist_normalize(Temporal *temp)
{
  if (! temp || temp->subtype == TINSTANT)
    return temp;
  Temporal *result;
  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    int count;
    const TInstant **instants = tsequence_insts_p(seq, &count);
    result = (Temporal *) tsequence_make((TInstant **) instants, count,
      seq->period.lower_inc, seq->period.upper_inc,
      MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
    pfree(instants);
  }
  else
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int count;
      const TInstant **instants = tsequence_insts_p(seq, &count);
      sequences[i] = tsequence_make((TInstant **) instants, count,
        seq->period.lower_inc, seq->period.upper_inc,
        MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
      pfree(instants);
    }
    result = (Temporal *) tsequenceset_make_free(sequences, ss->count,
      NORMALIZE);
  }
  pfree(temp);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_geo() #Tdistance_geo_trgeometry()
 */
Temporal *
tdistance_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  return dist_normalize(trgeo_tdistance_geo(temp, gs, -1.0));
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and a
 * temporal geometry point
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_tpoint() #Tdistance_tpoint_trgeometry()
 */
Temporal *
tdistance_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return dist_normalize(trgeo_tdistance_tpoint(temp1, temp2, -1.0));
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_trgeometry()
 */
Temporal *
tdistance_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  return dist_normalize(trgeo_tdistance_trgeo(temp1, temp2, -1.0));
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal rigid
 * geometry and a geometry are within a distance
 * @details The temporal distance has an instant at each time at which it
 * crosses @p dist, thus the comparison of its linear interpolation with
 * @p dist is exact.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_trgeometry_geo()
 */
Temporal *
tdwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;
  Temporal *tdist = trgeo_tdistance_geo(temp, gs, dist);
  if (! tdist)
    return NULL;
  Temporal *result = tcomp_temporal_base(tdist, Float8GetDatum(dist), &datum2_le);
  pfree(tdist);
  return result;
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal rigid geometry are within a distance
 * @csqlfn #Tdwithin_geo_trgeometry()
 */
Temporal *
tdwithin_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp,
  double dist)
{
  return tdwithin_trgeometry_geo(temp, gs, dist);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether two temporal rigid
 * geometries are within a distance
 * @details See #tdwithin_trgeometry_geo()
 * @csqlfn #Tdwithin_trgeometry_trgeometry()
 */
Temporal *
tdwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8))
    return NULL;
  Temporal *tdist = trgeo_tdistance_trgeo(temp1, temp2, dist);
  if (! tdist)
    return NULL;
  Temporal *result = tcomp_temporal_base(tdist, Float8GetDatum(dist), &datum2_le);
  pfree(tdist);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid geometry
 * and a geometry
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_geo() #NAI_geo_trgeometry()
 */
TInstant *
nai_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  /* Also an instant goes through the temporal distance, which refuses the
   * same reference geometries for all subtypes */
  TInstant *result = NULL;
  Temporal *dist = trgeo_tdistance_geo(temp, gs, -1.0);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp, min->t, false, &value);
    result = trgeometryinst_make(trgeo_geom_p(temp), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid
 * geometry and a temporal point
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_tpoint() #NAI_tpoint_trgeometry()
 */
TInstant *
nai_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = trgeo_tdistance_tpoint(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeo_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between two temporal rigid
 * geometries
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_trgeometry()
 */
TInstant *
nai_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = trgeo_tdistance_trgeo(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound. */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeo_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a geometry
 * @csqlfn #NAD_trgeometry_geo() #NAD_geo_trgeometry()
 */
double
nad_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return DBL_MAX;

  Temporal *dist = trgeo_tdistance_geo(temp, gs, -1.0);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a spatiotemporal box
 * @csqlfn #NAD_trgeometry_stbox() #NAD_stbox_trgeometry()
 */
double
nad_trgeometry_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_stbox(temp, box))
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
    /* The generic temporal restriction drops the reference geometry, so the
     * rigid geometry restriction is the one that answers a trgeometry */
    temp1 = trgeometry_restrict_tstzspan(temp, &inter, REST_AT);
    /* The two spans meet while no value lies inside the intersection, which a
     * discrete or a step value can do */
    if (! temp1)
      return DBL_MAX;
  }
  /* Convert the stbox to a geometry */
  GSERIALIZED *geo = stbox_geo(box);
  /* Compute the result */
  Temporal *dist = trgeo_tdistance_geo(temp1, geo, -1.0);
  if (dist == NULL)
  {
    pfree(geo);
    if (hast)
      pfree(temp1);
    return DBL_MAX;
  }

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist); pfree(geo);
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal rigid geometry
 */
double
nad_stbox_trgeometry(const STBox *box, const Temporal *temp)
{
  return nad_trgeometry_stbox(temp, box);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a temporal point
 * @csqlfn #NAD_trgeometry_tpoint() #NAD_tpoint_trgeometry()
 */
double
nad_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return DBL_MAX;

  Temporal *dist = trgeo_tdistance_tpoint(temp1, temp2, -1.0);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @csqlfn #NAD_trgeometry_trgeometry()
 */
double
nad_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return DBL_MAX;

  Temporal *dist = trgeo_tdistance_trgeo(temp1, temp2, -1.0);
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
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a geometry
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_geo() #Shortestline_geo_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *dist = trgeo_tdistance_geo(temp, gs, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value;
  trgeo_value_at_timestamptz(temp, inst->t, false, &value);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value), gs);
  pfree(DatumGetPointer(value)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a temporal geometry point
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_tpoint() #Shortestline_tpoint_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_tpoint(temp1, temp2))
    return NULL;

  Temporal *dist = trgeo_tdistance_tpoint(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeo_value_at_timestamptz(temp1, inst->t, false, &value1);
  temporal_value_at_timestamptz(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_trgeo(temp1, temp2))
    return NULL;

  Temporal *dist = trgeo_tdistance_trgeo(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeo_value_at_timestamptz(temp1, inst->t, false, &value1);
  trgeo_value_at_timestamptz(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/*****************************************************************************/
