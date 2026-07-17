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
 * @brief General functions for temporal pose objects.
 */

/* C */
#include <assert.h>
/* Postgres */
#include <postgres.h>
#include <common/hashfn.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure the validity of a temporal pose and a geometry
 */
bool
ensure_valid_tpose_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  VALIDATE_TPOSE(temp, false); VALIDATE_NOT_NULL(gs, false);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal pose and a pose
 */
bool
ensure_valid_tpose_pose(const Temporal *temp, const Pose *pose)
{
  VALIDATE_TPOSE(temp, false); VALIDATE_NOT_NULL(pose, false);
  if (! ensure_same_srid(tspatial_srid(temp), pose_srid(pose)))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a temporal pose and a spatiotemporal box
 */
bool
ensure_valid_tpose_stbox(const Temporal *temp, const STBox *box)
{
  VALIDATE_TPOSE(temp, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) || 
      ! ensure_same_srid(tspatial_srid(temp), box->srid))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two temporal poses
 */
bool
ensure_valid_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  VALIDATE_TPOSE(temp1, false); VALIDATE_TPOSE(temp2, false);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return false;
  return true;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * @brief Return 1 or 0 if a temporal pose segment intersects a pose during the
 * period defined by the output timestamps, return 0 otherwise
 * @param[in] start,end Base values defining the segment
 * @param[in] value Base value
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 */
int
tposesegm_intersection_value(Datum start, Datum end, Datum value,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  const Pose *pose1 = DatumGetPoseP(start);
  const Pose *pose2 = DatumGetPoseP(end);
  const Pose *pose = DatumGetPoseP(value);
  /* Locate the value in the segment accounting for both position and
   * rotation; this handles the case where pose1 and pose2 share the same
   * point but differ in rotation. */
  long double fraction = posesegm_locate(pose1, pose2, pose);
  if (fraction < 0.0)
    return 0;
  double duration = (double) (upper - lower);
  *t1 = lower + (TimestampTz) (duration * (double) fraction);
  if (t2)
    *t2 = *t1;
  return 1;
}

/**
 * @brief Return 1 if two temporal pose segments intersect during the period
 * defined by the output timestamps, return 0 otherwise
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note The candidate instant is derived from the position component by
 * delegating to #tgeompointsegm_intersection; the lifting infrastructure
 * verifies full pose equality (position + rotation) at the returned
 * timestamp.
 */
int
tposesegm_intersection(Datum start1, Datum end1, Datum start2, Datum end2,
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  GSERIALIZED *gs1 = pose_to_point(DatumGetPoseP(start1));
  GSERIALIZED *gs2 = pose_to_point(DatumGetPoseP(end1));
  GSERIALIZED *gs3 = pose_to_point(DatumGetPoseP(start2));
  GSERIALIZED *gs4 = pose_to_point(DatumGetPoseP(end2));
  int result = tgeompointsegm_intersection(PointerGetDatum(gs1),
    PointerGetDatum(gs2), PointerGetDatum(gs3), PointerGetDatum(gs4),
    lower, upper, t1, t2);
  pfree(gs1); pfree(gs2); pfree(gs3); pfree(gs4);
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_pose_inout
 * @brief Return a temporal pose from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Tpose_in()
 */
Temporal *
tpose_in(const char *str)
{
  VALIDATE_NOT_NULL(str, NULL);
  return tspatial_parse(&str, T_TPOSE);
}

/**
 * @ingroup meos_internal_pose_inout
 * @brief Return a temporal pose instant from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 */
TInstant *
tposeinst_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tpose_in(str);
  assert(temp->subtype == TINSTANT);
  return (TInstant *) temp;
}

/**
 * @ingroup meos_internal_pose_inout
 * @brief Return a temporal pose sequence from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @param[in] interp Interpolation
 */
TSequence *
tposeseq_in(const char *str, interpType interp UNUSED)
{
  /* Call the superclass function */
  Temporal *temp = tpose_in(str);
  assert (temp->subtype == TSEQUENCE);
  return (TSequence *) temp;
}

/**
 * @ingroup meos_internal_pose_inout
 * @brief Return a temporal pose sequence set from its Well-Known
 * Text (WKT) representation
 * @param[in] str String
 */
TSequenceSet *
tposeseqset_in(const char *str)
{
  /* Call the superclass function */
  Temporal *temp = tpose_in(str);
  assert(temp->subtype == TSEQUENCESET);
  return (TSequenceSet *) temp;
}

/**
 * @ingroup meos_pose_inout
 * @brief Return a temporal pose from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @return On error return @p NULL
 * @see #temporal_from_mfjson()
 */
Temporal *
tpose_from_mfjson(const char *mfjson)
{
  VALIDATE_NOT_NULL(mfjson, NULL);
  return temporal_from_mfjson(mfjson, T_TPOSE);
}
#endif /* MEOS */

/*****************************************************************************
 * Costructor functions
 *****************************************************************************/

/**
 * @brief Return a temporal pose instant from the temporal point and a temporal
 * float instants
 * @param[in] inst1 Temporal point
 * @param[in] inst2 Temporal float
 * @pre The temporal point and the temporal float are synchronized
 */
static TInstant *
tgeompoint_tfloat_to_tposeinst(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == T_TGEOMPOINT);
  assert(inst2->temptype == T_TFLOAT);
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst1));
  const POINT4D *p = (const POINT4D *) GS_POINT_PTR(gs);
  double radius = DatumGetFloat8(tinstant_value_p(inst2));
  Pose *pose = pose_make_2d(p->x, p->y, radius,
    FLAGS_GET_GEODETIC(gs->gflags), gserialized_get_srid(gs));
  TInstant *result = tinstant_make(PointerGetDatum(pose), T_TPOSE, inst1->t);
  pfree(pose);
  return result;
}

/**
 * @brief Return a temporal pose sequence from a temporal point and a temporal
 * float sequences
 * @param[in] seq1 Temporal point
 * @param[in] seq2 Temporal float
 * @pre The temporal point and the temporal float are synchronized
 */
static TSequence *
tposeseq_make(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == T_TGEOMPOINT);
  assert(seq2->temptype == T_TFLOAT);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, 0);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);

  /* Instantaneous sequence */
  if (seq1->count == 1)
  {
    TInstant *inst = tgeompoint_tfloat_to_tposeinst(inst1, inst2);
    TSequence *result = tinstant_as_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  for (int i = 0; i < seq1->count; i++)
  {
    inst1 = TSEQUENCE_INST_N(seq1, i);
    inst2 = TSEQUENCE_INST_N(seq2, i);
    instants[i] = tgeompoint_tfloat_to_tposeinst(inst1, inst2);
  }
  TSequence *result = tsequence_make(instants, seq1->count,
    seq1->period.lower_inc, seq1->period.upper_inc, interp, NORMALIZE);
  return result;
}

/**
 * @brief Return a temporal pose sequence set from a temporal point and a
 * temporal float sequence sets
 * @param[in] ss1 Temporal point
 * @param[in] ss2 Temporal float
 * @pre The temporal point and the temporal float are synchronized
 */
static TSequenceSet *
tposeseqset_make(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2); assert(ss1->temptype == T_TGEOMPOINT);
  assert(ss2->temptype == T_TFLOAT);

  const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, 0);
  const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, 0);

  /* Instantaneous sequence */
  if (ss1->count == 1)
  {
    TSequence *seq = tposeseq_make(seq1, seq2);
    TSequenceSet *result = tsequence_as_tsequenceset(seq);
    pfree(seq);
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss1->count);
  for (int i = 0; i < ss1->count; i++)
  {
    seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    seq2 = TSEQUENCESET_SEQ_N(ss2, i);
    sequences[i] = tposeseq_make(seq1, seq2);
  }
  TSequenceSet *result = tsequenceset_make(sequences, ss1->count, NORMALIZE);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_conversion
 * @brief Return a temporal pose from a temporal point and a temporal float
 * @param[in] tpoint Temporal point
 * @param[in] tradius Temporal float
 * @csqlfn #Tpose_make()
 */
Temporal *
tpose_make(const Temporal *tpoint, const Temporal *tradius)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEOMPOINT(tpoint, NULL); VALIDATE_TFLOAT(tradius, NULL);
  if (! ensure_has_not_Z(tpoint->temptype, tpoint->flags) ||
      ! ensure_not_geodetic(tpoint->flags))
    return NULL;

  Temporal *sync1, *sync2;
  /* Return false if the temporal values do not intersect in time
   * The operation is synchronization without adding crossings */
  if (! intersection_temporal_temporal(tpoint, tradius, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return NULL;

  Temporal *result;
  assert(temptype_subtype(sync1->subtype));
  switch (sync1->subtype)
  {
    case TINSTANT:
      result = (Temporal *) tgeompoint_tfloat_to_tposeinst((TInstant *) sync1,
        (TInstant *) sync2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tposeseq_make((TSequence *) sync1, 
        (TSequence *) sync2);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tposeseqset_make((TSequenceSet *) sync1,
        (TSequenceSet *) sync2);
  }
  pfree(sync1); pfree(sync2);
  return result;
}

/**
 * @ingroup meos_pose_constructor
 * @brief Return a temporal pose instant from a pose and a timestamptz
 * @param[in] pose Value
 * @param[in] t Timestamp
 * @csqlfn #Tinstant_constructor()
 */
TInstant *
tposeinst_make(const Pose *pose, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  return tinstant_make(PointerGetDatum(pose), T_TPOSE, t);
}

/**
 * @ingroup meos_pose_constructor
 * @brief Return a temporal pose from a pose and the time frame of another
 * temporal value
 * @param[in] pose Value
 * @param[in] temp Temporal value
 */
Temporal *
tpose_from_base_temp(const Pose *pose, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_NOT_NULL(temp, NULL);
  return temporal_from_base_temp(PointerGetDatum(pose), T_TPOSE, temp);
}

/**
 * @ingroup meos_pose_constructor
 * @brief Return a temporal pose discrete sequence from a pose and a
 * timestamptz set
 * @param[in] pose Value
 * @param[in] s Set
 */
TSequence *
tposeseq_from_base_tstzset(const Pose *pose, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_TSTZSET(s, NULL);
  return tsequence_from_base_tstzset(PointerGetDatum(pose), T_TPOSE, s);
}

/**
 * @ingroup meos_pose_constructor
 * @brief Return a temporal pose sequence from a pose and a timestamptz span
 * @param[in] pose Value
 * @param[in] s Span
 * @param[in] interp Interpolation
 */
TSequence *
tposeseq_from_base_tstzspan(const Pose *pose, const Span *s, interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_TSTZSPAN(s, NULL);
  return tsequence_from_base_tstzspan(PointerGetDatum(pose), T_TPOSE, s,
    interp);
}

/**
 * @ingroup meos_pose_constructor
 * @brief Return a temporal pose sequence set from a pose and a timestamptz
 * span set
 * @param[in] pose Value
 * @param[in] ss Span set
 * @param[in] interp Interpolation
 */
TSequenceSet *
tposeseqset_from_base_tstzspanset(const Pose *pose, const SpanSet *ss,
  interpType interp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_TSTZSPANSET(ss, NULL);
  return tsequenceset_from_base_tstzspanset(PointerGetDatum(pose), T_TPOSE,
    ss, interp);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_conversion
 * @brief Return a temporal point from a temporal pose
 * @details The result is a temporal geometry point for a planar pose and a
 * temporal geography point for a geodetic pose
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_to_tpoint()
 */
Temporal *
tpose_to_tpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_geopoint;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = MEOS_FLAGS_GET_GEODETIC(temp->flags) ?
    T_TGEOGPOINT : T_TGEOMPOINT;
  /* Position projection is affine: linear input -> linear output */
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_accessor
 * @brief Return a the rotation of a temporal pose as a temporal float
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_rotation()
 */
Temporal *
tpose_rotation(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_rotation;
  lfinfo.argtype[0] = T_TPOSE;
  lfinfo.restype = T_TFLOAT;
  /* Rotation projection is affine: linear input -> linear output */
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/**
 * @brief Lift a pose-to-double Datum function through a temporal pose,
 * returning a temporal float. Shared backend for the @p yaw / @p pitch /
 * @p roll accessors below.
 */
static Temporal *
tpose_lift_to_tfloat(const Temporal *temp, varfunc func)
{
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = func;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TPOSE;
  lfinfo.restype = T_TFLOAT;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the yaw angle of a temporal pose (radians) as a temporal
 * float, ZYX intrinsic Tait-Bryan convention
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_yaw()
 */
Temporal *
tpose_yaw(const Temporal *temp)
{
  VALIDATE_TPOSE(temp, NULL);
  return tpose_lift_to_tfloat(temp, (varfunc) &datum_pose_yaw);
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the pitch angle of a temporal pose (radians) as a temporal
 * float, ZYX intrinsic Tait-Bryan convention
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_pitch()
 */
Temporal *
tpose_pitch(const Temporal *temp)
{
  VALIDATE_TPOSE(temp, NULL);
  return tpose_lift_to_tfloat(temp, (varfunc) &datum_pose_pitch);
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the roll angle of a temporal pose (radians) as a temporal
 * float, ZYX intrinsic Tait-Bryan convention
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_roll()
 */
Temporal *
tpose_roll(const Temporal *temp)
{
  VALIDATE_TPOSE(temp, NULL);
  return tpose_lift_to_tfloat(temp, (varfunc) &datum_pose_roll);
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the speed of a temporal pose as a temporal float
 * @details The result is the magnitude of the position-component
 * velocity, i.e. distance travelled per unit time. The orientation
 * component is not part of this scalar — angular velocity has its own
 * accessor @p tpose_angular_speed.
 * @param[in] temp Temporal pose
 * @return On error return @p NULL
 * @csqlfn #Tpose_speed()
 */
Temporal *
tpose_speed(const Temporal *temp)
{
  VALIDATE_TPOSE(temp, NULL);
  Temporal *traj = tpose_to_tpoint(temp);
  if (! traj)
    return NULL;
  Temporal *result = tpoint_speed(traj);
  pfree(traj);
  return result;
}

/**
 * @brief Per-segment angular speed for a TSequence of tpose with linear
 * interpolation. Builds a step-interpolation TSequence of tfloat where
 * each instant carries the constant @p |omega| of the segment that
 * starts at it.
 */
static TSequence *
tposeseq_angular_speed(const TSequence *seq)
{
  assert(seq); assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Instantaneous sequence — no segment, no angular speed. */
  if (seq->count == 1)
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
  double angspeed = 0.0;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
    double dt = (double) (inst2->t - inst1->t) / 1000000.0;
    angspeed = (dt > 0.0) ?
      pose_angular_distance(pose1, pose2) / dt : 0.0;
    instants[i] = tinstant_make(Float8GetDatum(angspeed), T_TFLOAT, inst1->t);
    inst1 = inst2;
    pose1 = pose2;
  }
  /* The trailing instant carries the previous segment's value, so the
   * step-interpolated tfloat is well-defined on the closed interval. */
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(angspeed),
    T_TFLOAT, seq->period.upper);
  TSequence *result = tsequence_make(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  pfree_array((void **) instants, seq->count);
  return result;
}

/**
 * @brief Per-sequenceset angular speed.
 */
static TSequenceSet *
tposeseqset_angular_speed(const TSequenceSet *ss)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    TSequence *r = tposeseq_angular_speed(seq);
    if (r) sequences[nseqs++] = r;
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the angular speed of a temporal pose (radians per unit
 * time) as a step-interpolated temporal float
 * @details For 2D poses this is the per-segment shortest-arc theta delta
 * divided by the segment duration. For 3D poses this is the SLERP arc
 * angle @p 2 · acos(|q1 · q2|) divided by the segment duration; SLERP is
 * by construction constant-angular-velocity over a segment so the
 * resulting tfloat is step-interpolated.
 * @param[in] temp Temporal pose with linear interpolation
 * @return On error or for an instantaneous tpose return @p NULL
 * @csqlfn #Tpose_angular_speed()
 */
Temporal *
tpose_angular_speed(const Temporal *temp)
{
  VALIDATE_TPOSE(temp, NULL);
  if (! ensure_linear_interp(temp->flags))
    return NULL;
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tposeseq_angular_speed((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tposeseqset_angular_speed(
        (TSequenceSet *) temp);
  }
}

/**
 * @brief Per-sequence builder of @p applyPose: applies the rigid-body
 * transform of each instant's pose to the body geometry, producing a
 * tgeompoint sequence with the same temporal shape as the input.
 */
static TSequence *
tposeseq_apply_geo(const TSequence *seq, const GSERIALIZED *body)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
    GSERIALIZED *world = pose_apply_geo(pose, body);
    if (world == NULL)
    {
      for (int j = 0; j < i; j++) pfree(instants[j]);
      pfree(instants);
      return NULL;
    }
    instants[i] = tinstant_make(GserializedPGetDatum(world), T_TGEOMPOINT,
      inst->t);
  }
  TSequence *result = tsequence_make(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  pfree_array((void **) instants, seq->count);
  return result;
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the world-frame trajectory obtained by applying a
 * temporal pose to a body-frame geometry
 * @details Per instant the rigid-body transform @p R(q_i) · v + p_i is
 * applied to the body geometry; the result is a temporal point with the
 * same subtype and interpolation as the input @p tpose. v1 supports
 * point and multipoint body geometries — the output is a tgeompoint
 * carrying the per-instant transformed point. Linear interpolation of
 * the resulting trajectory is the chord on each segment, which
 * approximates the true rigid-body trajectory (a circular arc under
 * SLERP); the same linear-interpolation trade-off MobilityDB already
 * uses for spatial trajectories.
 * @param[in] temp Temporal pose
 * @param[in] body Body-frame geometry (POINT or MULTIPOINT)
 * @return On error return @p NULL
 * @csqlfn #Tpose_apply_geo()
 */
Temporal *
tpose_apply_geo(const Temporal *temp, const GSERIALIZED *body)
{
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(body, NULL);

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      const TInstant *inst = (const TInstant *) temp;
      const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
      GSERIALIZED *world = pose_apply_geo(pose, body);
      if (world == NULL) return NULL;
      return (Temporal *) tinstant_make(GserializedPGetDatum(world),
        T_TGEOMPOINT, inst->t);
    }
    case TSEQUENCE:
      return (Temporal *) tposeseq_apply_geo((const TSequence *) temp, body);
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
      for (int i = 0; i < ss->count; i++)
      {
        TSequence *r = tposeseq_apply_geo(TSEQUENCESET_SEQ_N(ss, i), body);
        if (! r)
        {
          for (int j = 0; j < i; j++) pfree(sequences[j]);
          pfree(sequences);
          return NULL;
        }
        sequences[i] = r;
      }
      return (Temporal *) tsequenceset_make_free(sequences, ss->count, NORMALIZE);
    }
  }
}

/*****************************************************************************/

#if MEOS
/**
 * @ingroup meos_pose_accessor
 * @brief Return a copy of the start value of a temporal pose
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_start_value()
 */
Pose *
tpose_start_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);
  return DatumGetPoseP(temporal_start_value(temp));
}

/**
 * @ingroup meos_pose_accessor
 * @brief Return a copy of the end value of a temporal pose
 * @param[in] temp Temporal value
 * @return On error return @p NULL
 * @csqlfn #Temporal_end_value()
 */
Pose *
tpose_end_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);
  return DatumGetPoseP(temporal_end_value(temp));
}

/**
 * @ingroup meos_pose_accessor
 * @brief Return a copy of the n-th value of a temporal pose
 * @param[in] temp Temporal value
 * @param[in] n Number
 * @param[out] result Value
 * @csqlfn #Temporal_value_n()
 */
bool
tpose_value_n(const Temporal *temp, int n, Pose **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, false); VALIDATE_NOT_NULL(result, false);
  Datum dresult;
  if (! temporal_value_n(temp, n, &dresult))
    return false;
  *result = DatumGetPoseP(dresult);
  return true;
}

/**
 * @ingroup meos_pose_accessor
 * @brief Return the array of copies of base values of a temporal pose
 * @param[in] temp Temporal value
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_valueset()
 */
Pose **
tpose_values(const Temporal *temp, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  Datum *datumarr = temporal_values_p(temp, count);
  Pose **result = palloc(sizeof(Pose *) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = pose_copy(DatumGetPoseP(datumarr[i]));
  pfree(datumarr);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return the points of a temporal pose
 */
Set *
tposeinst_points(const TInstant *inst)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  GSERIALIZED *gs = pose_to_point(pose);
  Datum value = PointerGetDatum(gs);
  Set *result = set_make_exp(&value, 1, 1, T_GEOMETRY, ORDER_NO);
  pfree(gs);
  return result;
}

/**
 * @brief Return the points of a temporal pose
 */
Set *
tposeseq_points(const TSequence *seq)
{
  Datum *values = palloc(sizeof(Datum) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const Pose *pose = DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq, i)));
    values[i] = PointerGetDatum(pose_to_point(pose));
  }
  datumarr_sort(values, seq->count, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, seq->count, T_GEOMETRY);
  Set *result = set_make_exp(values, count, count, T_GEOMETRY, ORDER_NO);
  for (int i = 0; i < seq->count; i++)
    pfree(DatumGetPointer(values[i]));
  pfree(values);
  return result;
}

/**
 * @brief Return the points of a temporal pose
 */
Set *
tposeseqset_points(const TSequenceSet *ss)
{
  Datum *values = palloc(sizeof(int64) * ss->totalcount);
  int nvalues = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
      values[nvalues++] = PointerGetDatum(pose_to_point(pose));
    }
  }
  datumarr_sort(values, ss->totalcount, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, ss->totalcount, T_GEOMETRY);
  Set *result = set_make_exp(values, count, count, T_GEOMETRY, ORDER_NO);
  for (int i = 0; i < ss->totalcount; i++)
    pfree(DatumGetPointer(values[i]));
  pfree(values);
  return result;
}

/**
 * @ingroup meos_pose_accessor
 * @brief Return the array of points of a temporal pose
 * @csqlfn #Tpose_points()
 */
Set *
tpose_points(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tposeinst_points((TInstant *) temp);
    case TSEQUENCE:
      return tposeseq_points((TSequence *) temp);
    default: /* TSEQUENCESET */
      return tposeseqset_points((TSequenceSet *) temp);
  }
}

#if MEOS
/**
 * @ingroup meos_pose_accessor
 * @brief Return the value of a temporal pose at a timestamptz
 * @param[in] temp Temporal value
 * @param[in] t Timestamp
 * @param[in] strict True if the timestamp must belong to the temporal value,
 * false when it may be at an exclusive bound
 * @param[out] result Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tpose_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Pose **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(result, false); VALIDATE_TPOSE(temp, false);

  Datum res;
  bool found = temporal_value_at_timestamptz(temp, t, strict, &res);
  *result = DatumGetPoseP(res);
  return found;
}
#endif /* MEOS */

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal pose restricted to a pose
 * @param[in] temp Temporal value
 * @param[in] pose Value
 * @csqlfn #Temporal_at_value()
 */
Temporal *
tpose_at_pose(const Temporal *temp, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(pose, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(pose), REST_AT);
}

/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal pose restricted to the complement of a 
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Value
 * @csqlfn #Temporal_minus_value()
 */
Temporal *
tpose_minus_pose(const Temporal *temp, const Pose *pose)
{
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(pose, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(pose), REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/
