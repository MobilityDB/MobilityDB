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
  /* We are sure that the trajectory is a line */
  Datum geom_start = datum_pose_point(start);
  Datum geom_end = datum_pose_point(end);
  Datum geom = datum_pose_point(value);
  double dist;
  /* Compute the value taking into account position only */
  double fraction = (double) pointsegm_locate(geom_start, geom_end, geom,
    &dist);
  pfree(DatumGetPointer(geom_start)); pfree(DatumGetPointer(geom_end));
  pfree(DatumGetPointer(geom));
  if (fraction < 0.0)
    return 0;
  /* Compare value with interpolated pose to take into account orientation as
   * well */
  const Pose *pose1 = DatumGetPoseP(start);
  const Pose *pose2 = DatumGetPoseP(end);
  Pose *pose_interp = posesegm_interpolate(pose1, pose2, fraction);
  Pose *pose = DatumGetPoseP(value);
  bool same = pose_same(pose, pose_interp);
  /* Temporal rigid geometries have poses as base values but are restricted
   * to geometries */
  // bool same;
  // if (inst1->temptype == T_TRGEOMETRY)
  // {
    // const GSERIALIZED *gs1 = DatumGetGserializedP(start);
    // const GSERIALIZED *gs2 = DatumGetGserializedP(value);
    // LWGEOM *geom1 = lwgeom_from_gserialized(gs1);
    // LWGEOM *geom2 = lwgeom_from_gserialized(gs2);
    // LWGEOM *geom_interp = lwgeom_clone_deep(geom2);
    // lwgeom_apply_pose(pose_interp, geom_interp);
    // if (geom_interp->bbox)
      // lwgeom_refresh_bbox(geom_interp);
    // same = lwgeom_same(geom1, geom_interp);
    // lwgeom_free(geom1); lwgeom_free(geom2); lwgeom_free(geom_interp);
  // }
  // else
  // {
    // Pose *pose = DatumGetPoseP(value);
    // same = pose_same(pose, pose_interp);
  // }
  pfree(pose_interp);
  if (! same)
    return 0;
  if (t1)
  {
    double duration = (double) (upper - lower);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t1 = lower + (TimestampTz) (duration * fraction);
    if (t2)
      *t2 = *t1;
  }
  return 1;
}

// /**
 // * @brief Return 1 if two temporal pose segments intersect during the period
 // * defined by the output timestamps, return 0 otherwise
 // * @param[in] start1,end1 Temporal instants defining the first segment
 // * @param[in] start2,end2 Temporal instants defining the second segment
 // * @param[in] lower,upper Timestamps defining the segments
 // * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 // */
// int
// tposesegm_intersection(Datum start1, Datum end1, Datum start2, Datum end2,
  // TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
// {
  // assert(lower < upper); assert(t1); assert(t2);
  // /* While waiting for this function we cheat and call the function below */
  // return posesegm_distance_turnpt(DatumGetPoseP(start1), DatumGetPoseP(end1),
    // DatumGetPoseP(start2), DatumGetPoseP(end2), lower, upper, t1, t2);
// }

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_pose_inout
 * @brief Return a temporal pose from its Well-Known Text (WKT) representation
 * @param[in] str String
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
tposeinst_make(const TInstant *inst1, const TInstant *inst2)
{
  assert(inst1); assert(inst2); assert(inst1->temptype == T_TGEOMPOINT);
  assert(inst2->temptype == T_TFLOAT);
  const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst1));
  POINT4D *p = (POINT4D *) GS_POINT_PTR(gs);
  double radius = DatumGetFloat8(tinstant_value_p(inst2));
  Pose *pose = pose_make_2d(p->x, p->y, radius, gserialized_get_srid(gs));
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
    TInstant *inst = tposeinst_make(inst1, inst2);
    TSequence *result = tinstant_to_tsequence(inst, interp);
    pfree(inst);
    return result;
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  for (int i = 0; i < seq1->count; i++)
  {
    inst1 = TSEQUENCE_INST_N(seq1, i);
    inst2 = TSEQUENCE_INST_N(seq2, i);
    instants[i] = tposeinst_make(inst1, inst2);
  }
  TSequence *result = tsequence_make((const TInstant **) instants, seq1->count,
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
    TSequenceSet *result = tsequence_to_tsequenceset(seq);
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
  TSequenceSet *result = tsequenceset_make((const TSequence **) sequences,
    ss1->count, NORMALIZE);
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
      result = (Temporal *) tposeinst_make((TInstant *) sync1,
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

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_conversion
 * @brief Return a geometry point from a temporal pose
 * @param[in] temp Temporal pose
 */
Temporal *
tpose_to_tpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_point;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
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
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TPOSE;
  lfinfo.restype = T_TFLOAT;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
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
  Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  Datum value = PointerGetDatum(pose_to_point(pose));
  return set_make_exp(&value, 1, 1, T_GEOMETRY, ORDER_NO);
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
  return set_make_free(values, count, T_GEOMETRY, ORDER_NO);
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
      Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
      values[nvalues++] = PointerGetDatum(pose_to_point(pose));
    }
  }
  datumarr_sort(values, ss->totalcount, T_GEOMETRY);
  int count = datumarr_remove_duplicates(values, ss->totalcount, T_GEOMETRY);
  return set_make_free(values, count, T_GEOMETRY, ORDER_NO);
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
 * @param[out] value Resulting value
 * @csqlfn #Temporal_value_at_timestamptz()
 */
bool
tpose_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,
  Pose **value)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(value, false); VALIDATE_TPOSE(temp, false);

  Datum res;
  bool result = temporal_value_at_timestamptz(temp, t, strict, &res);
  *value = DatumGetPoseP(res);
  return result;
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
