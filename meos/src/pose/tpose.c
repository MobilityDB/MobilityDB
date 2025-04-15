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

#include "pose/tpose.h"

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
#include "general/lifting.h"
#include "general/set.h"
#include "general/span.h"
#include "general/spanset.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"

/*****************************************************************************
 * Input/output in WKT, EWKT, and MFJSON format
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
tposeseq_in(const char *str, interpType interp __attribute__((unused)))
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
tpoint_tfloat_inst_to_tpose(const TInstant *inst1, const TInstant *inst2)
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
tpoint_tfloat_seq_to_tpose(const TSequence *seq1, const TSequence *seq2)
{
  assert(seq1); assert(seq2); assert(seq1->temptype == T_TGEOMPOINT);
  assert(seq2->temptype == T_TFLOAT);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, 0);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, 0);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);

  /* Instantaneous sequence */
  if (seq1->count == 1)
  {
    TInstant *inst = tpoint_tfloat_inst_to_tpose(inst1, inst2);
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
    instants[i] = tpoint_tfloat_inst_to_tpose(inst1, inst2);
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
tpoint_tfloat_seqset_to_tpose(const TSequenceSet *ss1, const TSequenceSet *ss2)
{
  assert(ss1); assert(ss2); assert(ss1->temptype == T_TGEOMPOINT);
  assert(ss2->temptype == T_TFLOAT);

  const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, 0);
  const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, 0);

  /* Instantaneous sequence */
  if (ss1->count == 1)
  {
    TSequence *seq = tpoint_tfloat_seq_to_tpose(seq1, seq2);
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
    sequences[i] = tpoint_tfloat_seq_to_tpose(seq1, seq2);
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
 * @csqlfn #Tpoint_tfloat_to_tpose()
 */
Temporal *
tpoint_tfloat_to_tpose(const Temporal *tpoint, const Temporal *tradius)
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
      result = (Temporal *) tpoint_tfloat_inst_to_tpose((TInstant *) sync1,
        (TInstant *) sync2);
      break;
    case TSEQUENCE:
      result = (Temporal *) tpoint_tfloat_seq_to_tpose(
        (TSequence *) sync1, (TSequence *) sync2);
      break;
    default: /* TSEQUENCESET */
      result = (Temporal *) tpoint_tfloat_seqset_to_tpose(
        (TSequenceSet *) sync1, (TSequenceSet *) sync2);
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
tpose_tpoint(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_point;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_conversion
 * @brief Return a geometry point from a temporal pose
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
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
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
  Datum value = PointerGetDatum(pose_point(pose));
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
    values[i] = PointerGetDatum(pose_point(pose));
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
      values[nvalues++] = PointerGetDatum(pose_point(pose));
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
tpose_at_value(const Temporal *temp, Pose *pose)
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
tpose_minus_value(const Temporal *temp, Pose *pose)
{
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(pose, NULL);
  return temporal_restrict_value(temp, PointerGetDatum(pose), REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/
