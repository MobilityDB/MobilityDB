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
#include "temporal/lifting.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_distance.h"
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
  double *xy;                /**< Vertices of the part as in the geometry,
                                  x, y pairs, the first not repeated */
  int npts;                  /**< Number of vertices */
} DistPart;

/**
 * @brief Set the vertices of a part to the @p n points of @p pa from @p i
 */
static void
dist_part_points(DistPart *part, const POINTARRAY *pa, uint32_t i, int n)
{
  part->xy = palloc(sizeof(double) * 2 * n);
  part->npts = n;
  for (int k = 0; k < n; k++)
  {
    const POINT2D *p = getPoint2d_cp(pa, i + (uint32_t) k);
    part->xy[2 * k] = p->x;
    part->xy[2 * k + 1] = p->y;
  }
  return;
}

/**
 * @brief Add a point part to an array of parts
 */
static void
dist_part_point(DistPart *parts, int *n, double x, double y)
{
  distrefpoly_set_point(&parts[*n].body);
  distmotion_set_static(&parts[*n].motion, x, y);
  parts[*n].xy = palloc(sizeof(double) * 2);
  parts[*n].xy[0] = x;
  parts[*n].xy[1] = y;
  parts[*n].npts = 1;
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
        dist_part_points(&parts[*n], pa, i, 2);
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
  const POINTARRAY *ring = ((LWPOLY *) geom)->rings[0];
  dist_part_points(&parts[*n], ring, 0, (int) ring->npoints - 1);
  (*n)++;
  return true;
}

/**
 * @brief Free an array of parts
 */
static void
dist_parts_free(DistPart *parts, int count)
{
  for (int i = 0; i < count; i++)
  {
    distrefpoly_free(&parts[i].body);
    pfree(parts[i].xy);
  }
  pfree(parts);
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
    dist_parts_free(parts, *count);
    return NULL;
  }
  return parts;
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
  const GSERIALIZED *gs;     /**< Static geometry @nullable */
  DistPart *parts;           /**< Parts of the static geometry */
  int nparts;                /**< Number of parts */
  double *refxy;             /**< Vertices of the reference geometry of the
                                  first operand, x, y pairs, the first not
                                  repeated, for a static geometry */
  int nref;                  /**< Number of vertices */
  const GSERIALIZED *ref2;   /**< Reference geometry of a temporal rigid
                                  geometry, or NULL for a temporal point */
  DistRefPoly body2;         /**< Body of a temporal point or temporal rigid
                                  geometry */
  double level;              /**< Distance whose crossings are events, or a
                                  negative value, see distwalk_segm() */
} DistOps;

/**
 * @brief Return the distance between the first operand at the pose @p pose
 * and the static geometry of @p ops
 * @details A body at the angle 0 is its reference geometry moved by the
 * position of the pose, and its distance to the geometry is the least of the
 * distances to the parts, each the double nearest its exact value
 * (#convex_motion_nad_exact) on the exact sums of the coordinates and the
 * position.  Where no such sum rounds (#translation_is_exact), the placed
 * body is that body, and its distance is the one of #geom_distance2d().
 * Otherwise, and for a body that turns, the distance is the one of the body
 * placed at the pose.
 */
static double
dist_pose_geo(const DistOps *ops, const Pose *pose)
{
  if (ops->refxy && pose->data[2] == 0.0 &&
      ! translation_is_exact(ops->refxy, ops->nref, pose->data))
  {
    double result = DBL_MAX;
    for (int i = 0; i < ops->nparts; i++)
      result = fmin(result, convex_motion_nad_exact(ops->refxy, ops->nref,
        pose->data, pose->data, ops->parts[i].xy, ops->parts[i].npts));
    return result;
  }
  GSERIALIZED *geo = pose_apply_geo(pose, ops->ref1);
  double result = geom_distance2d(geo, ops->gs);
  pfree(geo);
  return result;
}

/**
 * @brief Return the distance between the first operand at the pose @p pose1
 * and the second operand
 * @details The function places the bodies at their poses and returns the
 * double nearest the distance of the placed geometries, see
 * #geom_distance2d().  The distance to a static geometry is the one of
 * #dist_pose_geo().  The second operand is the static geometry of @p ops,
 * the point @p point2 of a temporal point, or the body of a temporal rigid
 * geometry at the pose @p pose2.
 * @param[in] pose2 Pose of a temporal rigid geometry, or NULL @nullable
 * @param[in] point2 Point of a temporal point, or NULL @nullable
 */
static double
dist_placed(const DistOps *ops, const Pose *pose1, const Pose *pose2,
  const GSERIALIZED *point2)
{
  if (ops->gs)
    return dist_pose_geo(ops, pose1);
  GSERIALIZED *geo1 = pose_apply_geo(pose1, ops->ref1);
  double result;
  if (! ops->ref2)
    result = geom_distance2d(geo1, point2);
  else
  {
    GSERIALIZED *geo2 = pose_apply_geo(pose2, ops->ref2);
    result = geom_distance2d(geo1, geo2);
    pfree(geo2);
  }
  pfree(geo1);
  return result;
}

/**
 * @brief Return the distance between the operands with the value @p value1
 * of the first operand and @p value2 of the second operand
 * @details @p value2 is a pose for a temporal rigid geometry, a point for a
 * temporal point, and is not read for a static geometry.
 */
static double
dist_values(const DistOps *ops, Datum value1, Datum value2)
{
  const Pose *pose1 = DatumGetPoseP(value1);
  if (ops->gs)
    return dist_placed(ops, pose1, NULL, NULL);
  if (! ops->ref2)
    return dist_placed(ops, pose1, NULL, DatumGetGserializedP(value2));
  return dist_placed(ops, pose1, DatumGetPoseP(value2), NULL);
}

/**
 * @brief Return the distance between the operands at the instant @p inst1 of
 * the first operand and @p inst2 of the second operand
 * @details @p inst2 is NULL for a static geometry.
 */
static double
dist_instant(const DistOps *ops, const TInstant *inst1, const TInstant *inst2)
{
  const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
  if (! inst2)
    return dist_placed(ops, pose1, NULL, NULL);
  return dist_values(ops, tinstant_value_p(inst1), tinstant_value_p(inst2));
}

/**
 * @brief Return a copy of the value of segment @p i of a sequence at @p t
 * @details With step interpolation, the value is the one of instant @p i
 * during the whole segment, its end included, since the segment gives the
 * piece [t_i, t_{i+1}) of the result.  With linear interpolation, the value
 * is the one #tsequence_value_at_timestamptz() gives.
 */
static Datum
dist_segvalue(const TSequence *seq, int i, TimestampTz t)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
  if (! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
    return tinstant_value(inst1);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
  return tsegment_value_at_timestamptz(tinstant_value_p(inst1),
    tinstant_value_p(inst2), seq->temptype, inst1->t, inst2->t, t);
}

/**
 * @brief Return the distance between the operands at the instant @p t of
 * segment @p i
 * @details @p seq2 is NULL for a static geometry.
 */
static double
dist_segm_at(const DistOps *ops, const TSequence *seq1, const TSequence *seq2,
  int i, TimestampTz t)
{
  Datum value1 = dist_segvalue(seq1, i, t);
  double result;
  if (! seq2)
    result = dist_placed(ops, DatumGetPoseP(value1), NULL, NULL);
  else
  {
    Datum value2 = dist_segvalue(seq2, i, t);
    result = dist_values(ops, value1, value2);
    pfree(DatumGetPointer(value2));
  }
  pfree(DatumGetPointer(value1));
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
 * The walk gives the instants of the result.  At an instant of the operands,
 * the value is the distance between the operands placed there, see
 * #dist_placed(), as #tsequence_value_at_timestamptz() places them.  An
 * event inside a segment is at a time that is not a microsecond, and its
 * value is the one of the walk at that time: zero where the bodies meet,
 * @p level where the distance crosses it, and the value of the function of
 * the feature at an extremum.  The instant of such an event is the nearest
 * microsecond, see #dist_event_time(), where the value of the bodies placed
 * differs from the value at the event.
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
  /* The distance to one part of a static geometry of several parts is not
   * the distance to the geometry, whose values the minimum over the parts
   * sets, see dist2d_seq_exact_values() */
  bool exact = ! (ops->gs && ops->nparts > 1);
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
    {
      double ratio = events.ev[j].t;
      TimestampTz t = dist_event_time(seq1, i, ratio);
      double dist = events.ev[j].dist;
      if (exact && (ratio <= 0.0 || ratio >= 1.0))
        dist = dist_segm_at(ops, seq1, seq2, i, t);
      dist_append(&insts, &count, &size, dist, t);
    }
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
 * between the rigid geometry @p seq and the static geometry of @p ops
 * @details The minimum of the distances to the parts has instants where two
 * linear interpolations cross.  The interpolated value there is not the
 * distance, thus the function computes the distance at each instant, see
 * #dist_pose_geo().  Then the result is exact at each instant, as for one
 * part, see #dist_walk_seq().
 *
 * An instant whose value is exactly the distance level of @p ops keeps it.
 * Such a value comes from the crossing of the level by the part that is
 * closest at that time, see distwalk_segm(), thus it is the exact distance.
 * A value from PostGIS has a rounding error, which can put the crossing in
 * the same microsecond as the instant, where the comparison with the level
 * does not see it.
 * @param[in] dist Minimum of the distances to the parts, freed
 */
static TSequence *
dist2d_seq_exact_values(const DistOps *ops, TSequence *dist,
  const TSequence *seq)
{
  double level = ops->level;
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
    instants[i] = tinstant_make(Float8GetDatum(dist_pose_geo(ops,
      DatumGetPoseP(pose))), T_TFLOAT, inst->t);
    pfree(DatumGetPointer(pose));
  }
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc, LINEAR, NORMALIZE_NO);
  pfree(dist);
  return result;
}

/**
 * @brief Give the instants inside each segment of a body that does not turn
 * the exact least distance to a static geometry
 * @details A body whose poses at the two ends of a segment have the angle 0
 * translates.  Its least distance over the segment to a part at rest is the
 * nearest approach of the two, the double nearest its exact value
 * (#convex_motion_nad_exact), and the least over the parts is the least
 * distance to the geometry.  No instant inside the segment is below it.
 * Where it is below the distances at the two ends, computed in the same way,
 * the instant of least value inside the segment, which the walk puts at the
 * least distance, takes it: the value at the event, whose time is not a
 * microsecond.  An instant whose value is exactly the distance @p level of
 * #DistOps keeps it, see #dist2d_seq_exact_values().
 * @param[in] dist Temporal distance of @p seq to the static geometry, freed
 */
static TSequence *
dist_seq_least(const DistOps *ops, const TSequence *seq, TSequence *dist)
{
  double *values = palloc(sizeof(double) * dist->count);
  for (int j = 0; j < dist->count; j++)
    values[j] = DatumGetFloat8(tinstant_value_p(TSEQUENCE_INST_N(dist, j)));
  bool changed = false;
  int k = 0;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    /* The instants first to last - 1 are inside the segment */
    while (k < dist->count && TSEQUENCE_INST_N(dist, k)->t <= inst1->t)
      k++;
    int first = k;
    while (k < dist->count && TSEQUENCE_INST_N(dist, k)->t < inst2->t)
      k++;
    int last = k;
    const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
    const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
    if (first == last || pose1->data[2] != 0.0 || pose2->data[2] != 0.0)
      continue;
    double least = DBL_MAX;
    for (int j = 0; j < ops->nparts; j++)
      least = fmin(least, convex_motion_nad_exact(ops->refxy, ops->nref,
        pose1->data, pose2->data, ops->parts[j].xy, ops->parts[j].npts));
    int min = -1;
    bool reached = false;
    for (int j = first; j < last; j++)
    {
      if (values[j] == ops->level)
      {
        /* The walk has put the least distance on a crossing of the level */
        reached |= (values[j] == least);
        continue;
      }
      if (values[j] < least)
      {
        values[j] = least;
        changed = true;
      }
      if (min < 0 || values[j] < values[min])
        min = j;
    }
    if (reached || min < 0 || values[min] == least)
      continue;
    double end1 = DBL_MAX, end2 = DBL_MAX;
    for (int j = 0; j < ops->nparts; j++)
    {
      end1 = fmin(end1, convex_motion_nad_exact(ops->refxy, ops->nref,
        pose1->data, pose1->data, ops->parts[j].xy, ops->parts[j].npts));
      end2 = fmin(end2, convex_motion_nad_exact(ops->refxy, ops->nref,
        pose2->data, pose2->data, ops->parts[j].xy, ops->parts[j].npts));
    }
    if (least < end1 && least < end2)
    {
      values[min] = least;
      changed = true;
    }
  }
  if (! changed)
  {
    pfree(values);
    return dist;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * dist->count);
  for (int j = 0; j < dist->count; j++)
    instants[j] = tinstant_make(Float8GetDatum(values[j]), T_TFLOAT,
      TSEQUENCE_INST_N(dist, j)->t);
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc, LINEAR, NORMALIZE_NO);
  pfree(values);
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
 * or NULL for a static geometry @nullable
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
    result = dist2d_seq_exact_values(ops, result, seq1);
  if (ops->refxy)
    result = dist_seq_least(ops, seq1, result);
  return (Temporal *) result;
}

/**
 * @brief Return the temporal distance between two operands
 * @param[in] temp1 Temporal rigid geometry
 * @param[in] temp2 Second operand with the same subtype and timestamps, or
 * NULL for a static geometry @nullable
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
/**
 * @brief Set the operands of the distance between a temporal rigid geometry
 * and a nonempty geometry
 * @return False, with an error, if the body or a part of the geometry is not
 * one the walk reads, see #dist_ref_body() and #dist_geo_parts()
 */
static bool
dist_geo_ops_set(DistOps *ops, const Temporal *temp, const GSERIALIZED *gs,
  double level)
{
  memset(ops, 0, sizeof(DistOps));
  ops->ref1 = trgeometry_geom_p(temp);
  ops->gs = gs;
  ops->level = level;
  if (! dist_ref_body(ops->ref1, &ops->body1))
    return false;
  ops->parts = dist_geo_parts(gs, &ops->nparts);
  if (! ops->parts)
  {
    distrefpoly_free(&ops->body1);
    return false;
  }
  /* The reference geometry is a polygon, see dist_ref_body() */
  LWGEOM *ref = lwgeom_from_gserialized(ops->ref1);
  const POINTARRAY *ring = lwgeom_as_lwpoly(ref)->rings[0];
  DistPart body;
  dist_part_points(&body, ring, 0, (int) ring->npoints - 1);
  lwgeom_free(ref);
  ops->refxy = body.xy;
  ops->nref = body.npts;
  return true;
}

/**
 * @brief Free the operands that #dist_geo_ops_set() sets
 */
static void
dist_geo_ops_free(DistOps *ops)
{
  pfree(ops->refxy);
  dist_parts_free(ops->parts, ops->nparts);
  distrefpoly_free(&ops->body1);
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a geometry
 * @param[in] level Distance whose crossings are instants of the result, or a
 * negative value.  Then the result is on the same side of @p level as the
 * distance at each time, which the threshold functions such as
 * #tdwithin_trgeometry_geo() need.
 */
Temporal *
tdistance_trgeometry_geo_intl(const Temporal *temp, const GSERIALIZED *gs,
  double level)
{
  assert(temp); assert(gs); assert(ensure_valid_trgeometry_geo(temp, gs));
  assert(! MEOS_FLAGS_GET_Z(temp->flags));
  /* An empty geometry has no distance to a temporal value */
  if (gserialized_is_empty(gs))
    return NULL;
  DistOps ops;
  if (! dist_geo_ops_set(&ops, temp, gs, level))
    return NULL;
  /* An error raised for one of the instants makes the answer NULL, as the
   * entry functions of lifting.c answer it */
  int last_errno = meos_errno_reset();
  Temporal *result = (Temporal *) lifting_result(dist_temporal(&ops, temp,
    NULL), last_errno);
  dist_geo_ops_free(&ops);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point or another temporal rigid geometry
 * @param[in] level See #tdistance_trgeometry_geo_intl()
 * @pre The arguments are valid and planar, as the external entries test them
 */
static Temporal *
tdistance_trgeometry_temporal(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(! MEOS_FLAGS_GET_Z(temp1->flags));
  assert(! MEOS_FLAGS_GET_Z(temp2->flags));
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeometry_geom_p(temp1);
  ops.ref2 = (temp2->temptype == T_TRGEOMETRY) ? trgeometry_geom_p(temp2) : NULL;
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
    /* An error raised for one of the instants makes the answer NULL, as the
     * entry functions of lifting.c answer it */
    int last_errno = meos_errno_reset();
    result = (Temporal *) lifting_result(dist_temporal(&ops, sync1, sync2),
      last_errno);
    pfree(sync1); pfree(sync2);
  }
  distrefpoly_free(&ops.body1);
  distrefpoly_free(&ops.body2);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point, with the crossings of @p level as instants, see
 * #tdistance_trgeometry_geo_intl()
 */
Temporal *
tdistance_trgeometry_tpoint_intl(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(temp1); assert(temp2);
  assert(ensure_valid_trgeometry_tpoint(temp1, temp2));
  return tdistance_trgeometry_temporal(temp1, temp2, level);
}

/**
 * @brief Return the temporal distance between two temporal rigid geometries,
 * with the crossings of @p level as instants, see #tdistance_trgeometry_geo_intl()
 */
Temporal *
tdistance_trgeometry_trgeometry_intl(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(temp1); assert(temp2);
  assert(ensure_valid_trgeometry_trgeometry(temp1, temp2));
  return tdistance_trgeometry_temporal(temp1, temp2, level);
}

/*****************************************************************************
 * Ever within a distance
 *****************************************************************************/

/**
 * @brief Return true if two bodies are farther apart than @p dist during the
 * whole segment @p seg
 * @details Every point of a body lies within the distance @p rmax of its
 * rotation center, see #distmotion_place(), so the bodies are at least the
 * distance of their centers minus the two @p rmax apart.  The centers move
 * linearly in the segment, and their distance is smallest at the projection
 * of the origin on their relative motion, clamped to the segment.  The bound
 * is grown by #MEOS_EPSILON relative to the magnitude of the values it is
 * computed from, far above the rounding of the computation, so that no
 * segment in which the walk finds a distance at or below @p dist is skipped.
 */
static bool
dist_segm_apart(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb, double dist)
{
  double vv = seg->wx1 * seg->wx1 + seg->wy1 * seg->wy1;
  double t = 0.0;
  if (vv > 0.0)
    t = fmin(fmax(-(seg->wx0 * seg->wx1 + seg->wy0 * seg->wy1) / vv, 0.0),
      1.0);
  double nx = seg->wx0 + t * seg->wx1, ny = seg->wy0 + t * seg->wy1;
  double near = sqrt(nx * nx + ny * ny);
  double bound = dist + ra->rmax + rb->rmax;
  double scale = fabs(seg->ma.cx) + fabs(seg->ma.cy) + fabs(seg->mb.cx) +
    fabs(seg->mb.cy) + sqrt(vv) + bound;
  return near > bound + MEOS_EPSILON * scale;
}

/**
 * @brief Return 1 if the operands are ever within @p dist in two
 * synchronized sequences, 0 if not, -1 if the walk fails
 * @details The values compared with @p dist are those #dist_seq() gives the
 * temporal distance, read until the first one at or below @p dist.  A
 * segment the centers of the bodies keep apart, see #dist_segm_apart(), has
 * none of them and is not walked.
 */
static int
dist_ever_seq(const DistOps *ops, const TSequence *seq1,
  const TSequence *seq2, double dist)
{
  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  if (seq1->count == 1 || (! linear1 && ! linear2))
  {
    for (int i = 0; i < seq1->count; i++)
      if (dist_instant(ops, TSEQUENCE_INST_N(seq1, i),
          TSEQUENCE_INST_N(seq2, i)) <= dist)
        return 1;
    return 0;
  }

  DistEvents events;
  distevents_init(&events);
  DistPair cf;
  cf.kind = DISTPAIR_NONE;
  cf.i = cf.j = 0;
  int result = 0;
  for (int i = 0; i < seq1->count - 1 && result == 0; i++)
  {
    DistMotion ma, mb;
    dist_segmotion(&ma, seq1, i, true);
    dist_segmotion(&mb, seq2, i, true);
    DistSegm dseg;
    distsegm_set(&dseg, &ma, &mb);
    if (dist_segm_apart(&dseg, &ops->body1, &ops->body2, dist))
    {
      /* The closest feature at the start of the next segment is unknown */
      cf.kind = DISTPAIR_NONE;
      continue;
    }
    events.count = 0;
    if (distwalk_segm(&dseg, &ops->body1, &ops->body2,
        distwalk_ftol(&dseg, &ops->body1, &ops->body2), ops->level, &cf,
        &events) != DISTWALK_OK)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed at "
        "segment %d", i);
      result = -1;
      break;
    }
    for (int j = 0; j < events.count && result == 0; j++)
      if (events.ev[j].dist <= dist)
        result = 1;
  }
  distevents_free(&events);
  /* With a linear and a step operand, the last instant is a value of its own,
   * see #dist_walk_seq() */
  if (result == 0 && linear1 != linear2 && seq1->period.upper_inc &&
      dist_instant(ops, TSEQUENCE_INST_N(seq1, seq1->count - 1),
        TSEQUENCE_INST_N(seq2, seq2->count - 1)) <= dist)
    result = 1;
  return result;
}

/**
 * @brief Return 1 if a sequence of a temporal rigid geometry is ever within
 * @p dist of the static geometry of @p ops, 0 if not, -1 if the walk fails
 * @details The answer is that of the minimum of the distance #dist_seq()
 * gives.  Without linear interpolation, its values are the distances at the
 * instants.  With it, a segment that the center of the body keeps apart from
 * a part, see #dist_segm_apart(), has no value within @p dist for that part.
 * In a segment where the body does not turn, the least distance to a part is
 * their nearest approach, the double nearest its exact value
 * (#convex_motion_nad_exact), which #dist_seq_least() gives the distance, the
 * ends of the segment included, and at the distance 0 it is 0 where they
 * meet (#convex_motion_meet_exact).  A segment where the body turns needs the
 * walk, and then #dist_seq() gives the distance of the sequence.
 */
static int
dist_ever_seq_geo(const DistOps *ops, const TSequence *seq, double dist)
{
  if (seq->count == 1 || ! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
  {
    for (int i = 0; i < seq->count; i++)
      if (dist_instant(ops, TSEQUENCE_INST_N(seq, i), NULL) <= dist)
        return 1;
    return 0;
  }
  bool turns = false;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const Pose *pose1 = DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq,
      i)));
    const Pose *pose2 = DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq,
      i + 1)));
    bool still = (pose1->data[2] == 0.0 && pose2->data[2] == 0.0);
    DistMotion ma;
    dist_segmotion(&ma, seq, i, true);
    for (int k = 0; k < ops->nparts; k++)
    {
      DistSegm dseg;
      distsegm_set(&dseg, &ma, &ops->parts[k].motion);
      if (dist_segm_apart(&dseg, &ops->body1, &ops->parts[k].body, dist))
        continue;
      if (! still)
      {
        turns = true;
        break;
      }
      /* At the distance 0 the question is whether they meet */
      if (dist == 0.0 ?
          convex_motion_meet_exact(ops->refxy, ops->nref, pose1->data,
            pose2->data, ops->parts[k].xy, ops->parts[k].npts) :
          convex_motion_nad_exact(ops->refxy, ops->nref, pose1->data,
            pose2->data, ops->parts[k].xy, ops->parts[k].npts) <= dist)
        return 1;
    }
  }
  if (! turns)
    return 0;
  Temporal *tdist = dist_seq(ops, seq, NULL);
  if (! tdist)
    return -1;
  int result = (DatumGetFloat8(temporal_min_value(tdist)) <= dist) ? 1 : 0;
  pfree(tdist);
  return result;
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever within
 * a distance, 0 if not, -1 on error
 * @details The answer is that of the minimum of their temporal distance, as
 * #tdistance_trgeometry_geo_intl() gives it, read sequence by sequence until
 * the first one within @p dist, see #dist_ever_seq_geo().
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @pre The arguments are valid and planar, as the external entry tests
 * them
 */
int
edwithin_trgeometry_geo_intl(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  assert(temp); assert(gs); assert(ensure_valid_trgeometry_geo(temp, gs));
  assert(dist >= 0.0);
  assert(! MEOS_FLAGS_GET_Z(temp->flags));
  /* An empty geometry has no distance to a temporal value */
  if (gserialized_is_empty(gs))
    return -1;
  DistOps ops;
  if (! dist_geo_ops_set(&ops, temp, gs, -1.0))
    return -1;
  /* An error raised for one of the instants makes the answer -1, as the
   * temporal distance is then NULL */
  int last_errno = meos_errno_reset();
  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (dist_instant(&ops, (const TInstant *) temp, NULL) <= dist) ?
      1 : 0;
  else if (temp->subtype == TSEQUENCE)
    result = dist_ever_seq_geo(&ops, (const TSequence *) temp, dist);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    result = 0;
    for (int i = 0; i < ss->count && result == 0; i++)
      result = dist_ever_seq_geo(&ops, TSEQUENCESET_SEQ_N(ss, i), dist);
  }
  dist_geo_ops_free(&ops);
  return lifting_result_int(result, last_errno);
}

/**
 * @brief Return the pose at @p t of a temporal rigid geometry sequence in its
 * segment starting at instant @p k
 * @details The pose is the one #tsegment_value_at_timestamptz gives, so that
 * the motion of an interval of #tcont_tcont_merge_walk is that of the
 * synchronized sequences of #tdistance_trgeometry_temporal: the start pose of a
 * constant segment and at the start of the segment, the end pose at its end,
 * and the interpolated pose inside it
 * @param[out] alloc True if the pose is allocated
 */
static const Pose *
dist_seq_pose(const TSequence *seq, int k, TimestampTz t, bool *alloc)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, k);
  const Pose *start = DatumGetPoseP(tinstant_value_p(inst1));
  *alloc = false;
  if (t == inst1->t || seq->count == 1)
    return start;
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, k + 1);
  const Pose *end = DatumGetPoseP(tinstant_value_p(inst2));
  if (pose_eq(start, end))
    return start;
  if (t == inst2->t)
    return end;
  long double ratio = (long double) (t - inst1->t) /
    (long double) (inst2->t - inst1->t);
  *alloc = true;
  return posesegm_interpolate(start, end, (double) ratio);
}

/**
 * @brief The pose of one operand at the end of the last interval walked,
 * which is its pose at the start of the next interval in the same segment
 */
typedef struct
{
  const TSequence *seq;        /**< Sequence */
  int k;                       /**< Instant starting the segment */
  TimestampTz t;               /**< Time of the pose */
  const Pose *pose;            /**< Pose @nullable */
  bool alloc;                  /**< True if the pose is allocated */
} DistPoseCache;

/**
 * @brief Return the pose of #dist_seq_pose, taken from the cache where it
 * holds it, the cache then being emptied
 */
static const Pose *
dist_seq_pose_cached(DistPoseCache *cache, const TSequence *seq, int k,
  TimestampTz t, bool *alloc)
{
  if (cache->pose && cache->seq == seq && cache->k == k && cache->t == t)
  {
    const Pose *result = cache->pose;
    *alloc = cache->alloc;
    cache->pose = NULL;
    return result;
  }
  return dist_seq_pose(seq, k, t, alloc);
}

/**
 * @brief Keep a pose in the cache, freeing the one it held
 */
static void
dist_pose_cache_set(DistPoseCache *cache, const TSequence *seq, int k,
  TimestampTz t, const Pose *pose, bool alloc)
{
  if (cache->pose && cache->alloc)
    pfree((Pose *) cache->pose);
  cache->seq = seq;
  cache->k = k;
  cache->t = t;
  cache->pose = pose;
  cache->alloc = alloc;
}

/**
 * @brief State of #trgeometry_dwithin_interval
 */
typedef struct
{
  const DistOps *ops;          /**< Operands */
  double dist;                 /**< Distance */
  DistPair cf;                 /**< Closest feature at the instant @p cft */
  TimestampTz cft;             /**< Instant of @p cf */
  const TSequence *seq1;       /**< Sequences of @p cf */
  const TSequence *seq2;
  DistPoseCache cache1;        /**< Pose of each operand at the end of the */
  DistPoseCache cache2;        /**< last interval walked */
  DistEvents events;           /**< Events of the last interval walked */
  bool failed;                 /**< True if a walk failed */
} DistEverWalk;

/**
 * @brief Return true if two temporal rigid geometries are within a distance
 * at some instant of an interval of #tcont_tcont_merge_walk
 * @details The values compared with the distance are those #dist_ever_seq()
 * reads on the same interval of the synchronized sequences: the walk of the
 * segment from the poses at the bounds of the interval, see #dist_seq_pose(),
 * or the distance of the placed bodies where the two share one instant. An
 * interval the centers of the bodies keep apart, see #dist_segm_apart(), is
 * not walked.
 */
static bool
trgeometry_dwithin_interval(const TSequence *seq1, int k1, const TSequence *seq2,
  int k2, TimestampTz lower, TimestampTz upper, void *state)
{
  DistEverWalk *dw = (DistEverWalk *) state;
  bool alloc1, alloc2, alloc3, alloc4;
  /* The poses at the start of the interval are those at the end of the last
   * one where the segment is the same */
  const Pose *p1 = dist_seq_pose_cached(&dw->cache1, seq1, k1, lower, &alloc1);
  const Pose *p2 = dist_seq_pose_cached(&dw->cache2, seq2, k2, lower, &alloc2);
  bool result = false;
  if (lower == upper)
  {
    /* The two values share one instant: the distance of the placed bodies */
    GSERIALIZED *geo1 = pose_apply_geo(p1, dw->ops->ref1);
    GSERIALIZED *geo2 = pose_apply_geo(p2, dw->ops->ref2);
    /* A distance raising an error, -1.0, places the bodies within no
     * distance */
    double d = geom_distance2d(geo1, geo2);
    result = d >= 0.0 && d <= dw->dist;
    pfree(geo1); pfree(geo2);
  }
  else
  {
    const Pose *q1 = dist_seq_pose(seq1, k1, upper, &alloc3);
    const Pose *q2 = dist_seq_pose(seq2, k2, upper, &alloc4);
    DistMotion ma, mb;
    distmotion_from_pose(&ma, p1, q1);
    distmotion_from_pose(&mb, p2, q2);
    DistSegm dseg;
    distsegm_set(&dseg, &ma, &mb);
    if (dist_segm_apart(&dseg, &dw->ops->body1, &dw->ops->body2, dw->dist))
      /* The closest feature at the start of the next interval is unknown */
      dw->cf.kind = DISTPAIR_NONE;
    else
    {
      /* The closest feature at the end of the last interval walked starts the
       * walk of the interval that follows it in the same two sequences */
      if (dw->cft != lower || dw->seq1 != seq1 || dw->seq2 != seq2)
        dw->cf.kind = DISTPAIR_NONE;
      dw->events.count = 0;
      if (distwalk_segm(&dseg, &dw->ops->body1, &dw->ops->body2,
          distwalk_ftol(&dseg, &dw->ops->body1, &dw->ops->body2),
          dw->ops->level, &dw->cf, &dw->events) != DISTWALK_OK)
      {
        dw->failed = true;
        result = true;
      }
      else
      {
        dw->cft = upper;
        dw->seq1 = seq1;
        dw->seq2 = seq2;
        for (int i = 0; i < dw->events.count && ! result; i++)
          result = dw->events.ev[i].dist <= dw->dist;
      }
    }
    /* The poses at the end of the interval start the next one */
    dist_pose_cache_set(&dw->cache1, seq1, k1, upper, q1, alloc3);
    dist_pose_cache_set(&dw->cache2, seq2, k2, upper, q2, alloc4);
  }
  if (alloc1) pfree((Pose *) p1);
  if (alloc2) pfree((Pose *) p2);
  return result;
}

/**
 * @brief Return 1 if two temporal rigid geometries are ever within a distance,
 * 0 if not, -1 if they do not intersect in time
 * @details The answer is that of the minimum of their temporal distance, as
 * #tdistance_trgeometry_trgeometry_intl() gives it: the values of that distance are read
 * until the first one at or below @p dist, and a segment that the centers of
 * the bodies keep apart is not walked, see #dist_ever_seq(). Two sequences
 * with linear interpolation are walked over their merged instants without
 * being synchronized, see #trgeometry_dwithin_interval().
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @pre The arguments are valid and planar, as the external entry tests
 * them
 */
int
edwithin_trgeometry_trgeometry_intl(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == T_TRGEOMETRY);
  assert(temp2->temptype == T_TRGEOMETRY);
  assert(dist >= 0.0);
  assert(! MEOS_FLAGS_GET_Z(temp1->flags));
  assert(! MEOS_FLAGS_GET_Z(temp2->flags));
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeometry_geom_p(temp1);
  ops.ref2 = trgeometry_geom_p(temp2);
  ops.level = -1.0;
  if (! dist_ref_body(ops.ref1, &ops.body1))
    return -1;
  if (! dist_ref_body(ops.ref2, &ops.body2))
  {
    distrefpoly_free(&ops.body1);
    return -1;
  }

  int result = -1;
  if (temp1->subtype != TINSTANT && temp2->subtype != TINSTANT &&
      MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
  {
    DistEverWalk dw;
    memset(&dw, 0, sizeof(DistEverWalk));
    dw.ops = &ops;
    dw.dist = dist;
    dw.cf.kind = DISTPAIR_NONE;
    dw.cft = DT_NOBEGIN;
    distevents_init(&dw.events);
    bool shared;
    bool found = tcont_tcont_merge_walk(temp1, temp2, &trgeometry_dwithin_interval,
      &dw, &shared);
    dist_pose_cache_set(&dw.cache1, NULL, 0, 0, NULL, false);
    dist_pose_cache_set(&dw.cache2, NULL, 0, 0, NULL, false);
    distevents_free(&dw.events);
    distrefpoly_free(&ops.body1);
    distrefpoly_free(&ops.body2);
    if (dw.failed)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed");
      return -1;
    }
    return shared ? (found ? 1 : 0) : -1;
  }

  /* The synchronization of #tdistance_trgeometry_temporal() */
  Temporal *sync1, *sync2;
  if (intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
  {
    assert(temptype_subtype(sync1->subtype));
    if (sync1->subtype == TINSTANT)
      result = (dist_instant(&ops, (const TInstant *) sync1,
        (const TInstant *) sync2) <= dist) ? 1 : 0;
    else if (sync1->subtype == TSEQUENCE)
      result = dist_ever_seq(&ops, (const TSequence *) sync1,
        (const TSequence *) sync2, dist);
    else /* TSEQUENCESET */
    {
      const TSequenceSet *ss1 = (const TSequenceSet *) sync1;
      const TSequenceSet *ss2 = (const TSequenceSet *) sync2;
      result = 0;
      for (int i = 0; i < ss1->count && result == 0; i++)
        result = dist_ever_seq(&ops, TSEQUENCESET_SEQ_N(ss1, i),
          TSEQUENCESET_SEQ_N(ss2, i), dist);
    }
    pfree(sync1); pfree(sync2);
  }
  distrefpoly_free(&ops.body1);
  distrefpoly_free(&ops.body2);
  return result;
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
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    Temporal * result = tdistance_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_geo_intl(temp, gs, -1.0));
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
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    Temporal * result = tdistance_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0));
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
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0));
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
  if (! ensure_finite(dist))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D, as
   * #tdistance_trgeometry_geo does */
  GSERIALIZED *flat = trgeometry_geo_common_dims(temp, gs);
  if (flat)
  {
    Temporal *result = tdwithin_trgeometry_geo(temp, flat, dist);
    pfree(flat);
    return result;
  }
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8) ||
      ! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* An empty geometry has no point, so it meets no value at any instant */
  if (gserialized_is_empty(gs))
    return temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
  Temporal *tdist = tdistance_trgeometry_geo_intl(temp, gs, dist);
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
  if (! ensure_finite(dist))
    return NULL;
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
  if (! ensure_finite(dist))
    return NULL;
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8) ||
      ! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  Temporal *tdist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, dist);
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
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    TInstant * result = nai_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      gserialized_is_empty(gs))
    return NULL;

  /* Also an instant goes through the temporal distance, which refuses the
   * same reference geometries for all subtypes */
  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp), DatumGetPoseP(value),
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
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    TInstant * result = nai_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp1), DatumGetPoseP(value),
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
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound. */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp1), DatumGetPoseP(value),
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
 * @errval -1.0
 */
double
nad_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    double result = nad_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;
  /* An empty geometry has no point to measure from, so the operands have no
   * distance, which is an answer */
  if (gserialized_is_empty(gs))
    return DBL_MAX;

  /* A geometry is defined at every instant of the temporal value, so the
   * temporal distance is NULL only where it raises an error */
  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (! dist)
    return -1.0;
  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a spatiotemporal box
 * @csqlfn #NAD_trgeometry_stbox() #NAD_stbox_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_stbox(temp, box) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;

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
  /* Compute the result. The time the operands share is decided above, so the
   * temporal distance is NULL only where it raises an error */
  Temporal *dist = tdistance_trgeometry_geo_intl(temp1, geo, -1.0);
  double result = -1.0;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  pfree(geo);
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal rigid geometry
 * @errval -1.0
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
 * @errval -1.0
 */
double
nad_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, -1.0); VALIDATE_NOT_NULL(temp2, -1.0);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return -1.0;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    double result = nad_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return -1.0;

  int last_errno = meos_errno_reset();
  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  /* The temporal distance is NULL where the operands share no time, which
   * is an answer, and where it raises an error */
  double result = DBL_MAX;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  return distance_result(result, last_errno);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @csqlfn #NAD_trgeometry_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return -1.0;

  int last_errno = meos_errno_reset();
  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  /* The temporal distance is NULL where the operands share no time, which
   * is an answer, and where it raises an error */
  double result = DBL_MAX;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  return distance_result(result, last_errno);
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
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    GSERIALIZED * result = shortestline_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      gserialized_is_empty(gs))
    return NULL;

  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value;
  trgeometry_value_at_timestamptz_intl(temp, inst->t, false, &value);
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
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    GSERIALIZED * result = shortestline_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeometry_value_at_timestamptz_intl(temp1, inst->t, false, &value1);
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
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeometry_value_at_timestamptz_intl(temp1, inst->t, false, &value1);
  trgeometry_value_at_timestamptz_intl(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/*****************************************************************************/
