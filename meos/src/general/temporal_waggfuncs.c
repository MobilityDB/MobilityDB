/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief General aggregate functions for temporal types
 */

#include "general/temporal_aggfuncs.h"

/* C */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/doublen.h"
#include "general/type_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Extend a temporal instant by a time interval
 * @param[in] inst Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tinstant_extend(const TInstant *inst, const Interval *interv,
  TSequence **result)
{
  TInstant *instants[2];
  TimestampTz upper = add_timestamptz_interval(inst->t, interv);
  instants[0] = (TInstant *) inst;
  instants[1] = tinstant_make(tinstant_val(inst), inst->temptype, upper);
  result[0] = tsequence_make((const TInstant **) instants, 2, true, true,
    MEOS_FLAGS_GET_CONTINUOUS(inst->flags) ? LINEAR : STEP, NORMALIZE_NO);
  pfree(instants[1]);
  return 1;
}

/**
 * @brief Extend a temporal discrete sequence by a time interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tdiscseq_extend(const TSequence *seq, const Interval *interv,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
    tinstant_extend(TSEQUENCE_INST_N(seq, i), interv, &result[i]);
  return seq->count;
}

/**
 * @brief Extend a temporal sequence by a time interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[in] min True if the calling function is min, max otherwise.
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored. This parameter is only used for linear interpolation.
 */
static int
tcontseq_extend(const TSequence *seq, const Interval *interv, bool min,
  TSequence **result)
{
  if (seq->count == 1)
    return tinstant_extend(TSEQUENCE_INST_N(seq, 0), interv, result);

  TInstant *instants[3];
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_val(inst1);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool lower_inc = seq->period.lower_inc;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count - 1; i++)
  {
    TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq, i + 1);
    Datum value2 = tinstant_val(inst2);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;

    /* Step interpolation or constant segment */
    if (interp != LINEAR || datum_eq(value1, value2, basetype))
    {
      TimestampTz upper = add_timestamptz_interval(inst2->t, interv);
      instants[0] = (TInstant *) inst1;
      instants[1] = tinstant_make(value1, inst1->temptype, upper);
      result[i] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, interp, NORMALIZE_NO);
      pfree(instants[1]);
    }
    else
    {
      /* Increasing period and minimum function or
       * decreasing period and maximum function */
      if ((datum_lt(value1, value2, basetype) && min) ||
        (datum_gt(value1, value2, basetype) && ! min))
      {
        /* Extend a start value for the duration of the window */
        TimestampTz lower = add_timestamptz_interval(inst1->t, interv);
        TimestampTz upper = add_timestamptz_interval(inst2->t, interv);
        instants[0] = inst1;
        instants[1] = tinstant_make(value1, inst1->temptype, lower);
        instants[2] = tinstant_make(value2, inst1->temptype, upper);
        result[i] = tsequence_make((const TInstant **) instants, 3,
          lower_inc, upper_inc, interp, NORMALIZE_NO);
        pfree(instants[1]); pfree(instants[2]);
      }
      else
      {
        /* Extend a end value for the duration of the window */
        TimestampTz upper = add_timestamptz_interval(seq->period.upper,
          interv);
        instants[0] = inst1;
        instants[1] = inst2;
        instants[2] = tinstant_make(value2, inst1->temptype, upper);
        result[i] = tsequence_make((const TInstant**) instants, 3,
          lower_inc, upper_inc, interp, NORMALIZE_NO);
        pfree(instants[2]);
      }
    }
    inst1 = inst2;
    value1 = value2;
    lower_inc = true;
  }
  return seq->count - 1;
}

/**
 * @brief Extend a temporal sequence set by a time interval
 * @param[in] ss Temporal value
 * @param[in] interv Interval
 * @param[in] min True if the calling function is min, max otherwise.
 * This parameter is only used for linear interpolation.
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tsequenceset_extend(const TSequenceSet *ss, const Interval *interv, bool min,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
    k += tcontseq_extend(TSEQUENCESET_SEQ_N(ss, i), interv, min, &result[k]);
  return k;
}

/**
 * @brief Extend a temporal value by a time interval
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @param[in] min True if the calling function is min, max otherwise
 * @param[out] count Number of elements in the output array
 */
TSequence **
temporal_extend(const Temporal *temp, const Interval *interv, bool min,
  int *count)
{
  TSequence **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      TInstant *inst = (TInstant *) temp;
      result = palloc(sizeof(TSequence *));
      *count = tinstant_extend(inst, interv, result);
      break;
    }
    case TSEQUENCE:
    {
      TSequence *seq = (TSequence *) temp;
      result = palloc(sizeof(TSequence *) * seq->count);
      *count = MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
        tdiscseq_extend(seq, interv, result) :
        tcontseq_extend(seq, interv, min, result);
      break;
    }
    default: /* TSEQUENCESET */
    {
      TSequenceSet *ss = (TSequenceSet *) temp;
      result = palloc(sizeof(TSequence *) * ss->totalcount);
      *count = tsequenceset_extend(ss, interv, min, result);
    }
  }
  return result;
}

/*****************************************************************************
 * Transform a temporal number type into a temporal integer type with value 1
 * extended by a time interval
 *****************************************************************************/

/**
 * @brief Return a sequence with a value 1 by extending the two bounds by
 * a time interval (iterator function)
 */
static TSequence *
tinstant_transform_wcount_iter(TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc, const Interval *interv)
{
  TInstant *instants[2];
  TimestampTz upper1 = add_timestamptz_interval(upper, interv);
  instants[0] = tinstant_make(Int32GetDatum(1), T_TINT, lower);
  instants[1] = tinstant_make(Int32GetDatum(1), T_TINT, upper1);
  TSequence *result = tsequence_make((const TInstant **) instants, 2,
    lower_inc, upper_inc, STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return result;
}

/**
 * @brief Transform a temporal number instant by a time interval
 * @param[in] inst Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tinstant_transform_wcount(const TInstant *inst, const Interval *interv,
  TSequence **result)
{
  result[0] = tinstant_transform_wcount_iter(inst->t, inst->t, true, true,
    interv);
  return 1;
}

/**
 * @brief Transform a temporal number discrete sequence by a time
 * interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tdiscseq_transform_wcount(const TSequence *seq, const Interval *interv,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
    tinstant_transform_wcount(TSEQUENCE_INST_N(seq, i), interv, &result[i]);
  return seq->count;
}

/**
 * @brief Transform a temporal number sequence by a time interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tcontseq_transform_wcount(const TSequence *seq, const Interval *interv,
  TSequence **result)
{
  if (seq->count == 1)
    return tinstant_transform_wcount(TSEQUENCE_INST_N(seq, 0), interv,
      result);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
    result[i] = tinstant_transform_wcount_iter(inst1->t, inst2->t, lower_inc,
      upper_inc, interv);
    inst1 = inst2;
    lower_inc = true;
  }
  return seq->count - 1;
}

/**
 * @brief Transform a temporal number sequence set by a time interval
 * @param[in] ss Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tsequenceset_transform_wcount(const TSequenceSet *ss, const Interval *interv,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
    k += tcontseq_transform_wcount(TSEQUENCESET_SEQ_N(ss, i), interv,
      &result[k]);
  return k;
}

/**
 * @brief Transform a temporal number by a time interval
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @param[out] count Number of elements in the output array
 */
TSequence **
temporal_transform_wcount(const Temporal *temp, const Interval *interv,
  int *count)
{
  assert(temptype_subtype(temp->subtype));
  TSequence **result;
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      TInstant *inst = (TInstant *) temp;
      result = palloc(sizeof(TSequence *));
      *count = tinstant_transform_wcount(inst, interv, result);
      break;
    }
    case TSEQUENCE:
    {
      TSequence *seq = (TSequence *) temp;
      result = palloc(sizeof(TSequence *) * seq->count);
      *count = MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        tdiscseq_transform_wcount(seq, interv, result) :
        tcontseq_transform_wcount(seq, interv, result);
      break;
    }
    default: /* TSEQUENCESET */
    {
      TSequenceSet *ss = (TSequenceSet *) temp;
      result = palloc(sizeof(TSequence *) * ss->totalcount);
      *count = tsequenceset_transform_wcount(ss, interv, result);
    }
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Transform a temporal number into a temporal double and extend it
 * by a time interval
 * @param[in] inst Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tnumberinst_transform_wavg(const TInstant *inst, const Interval *interv,
  TSequence **result)
{
  /* TODO: Should be an additional attribute */
  float8 value = 0.0;
  assert(tnumber_type(inst->temptype));
  if (inst->temptype == T_TINT)
    value = DatumGetInt32(tinstant_val(inst));
  else /* inst->temptype == T_TFLOAT */
    value = DatumGetFloat8(tinstant_val(inst));
  double2 dvalue;
  double2_set(value, 1, &dvalue);
  TimestampTz upper = add_timestamptz_interval(inst->t, interv);
  TInstant *instants[2];
  instants[0] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, inst->t);
  instants[1] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, upper);
  result[0] = tsequence_make((const TInstant**) instants, 2, true, true,
    MEOS_FLAGS_GET_CONTINUOUS(inst->flags) ? LINEAR : STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return 1;
}

/**
 * @brief Transform a temporal number into a temporal double and extend it
 * by a time interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tnumberdiscseq_transform_wavg(const TSequence *seq, const Interval *interv,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
    tnumberinst_transform_wavg(TSEQUENCE_INST_N(seq, i), interv, &result[i]);
  return seq->count;
}

/**
 * @brief Transform a temporal integer sequence into a temporal double
 * and extend it by a time interval
 * @param[in] seq Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 * @note There is no equivalent function for temporal float types
 */
static int
tintseq_transform_wavg(const TSequence *seq, const Interval *interv,
  TSequence **result)
{
  const TInstant *inst1;
  TInstant *instants[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    tnumberinst_transform_wavg(inst1, interv, &result[0]);
    return 1;
  }

  /* General case */
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
    double value = DatumGetInt32(tinstant_val(inst1));
    double2 dvalue;
    double2_set(value, 1, &dvalue);
    TimestampTz upper = add_timestamptz_interval(inst2->t, interv);
    instants[0] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, inst1->t);
    instants[1] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, upper);
    result[i] = tsequence_make((const TInstant **) instants, 2, lower_inc,
      upper_inc, STEP, NORMALIZE_NO);
    pfree(instants[0]); pfree(instants[1]);
    inst1 = inst2;
    lower_inc = true;
  }
  return seq->count - 1;
}

/**
 * @brief Transform a temporal integer sequence set into a temporal double and
 * extend it by a time interval
 * @param[in] ss Temporal value
 * @param[in] interv Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 * @note There is no equivalent function for temporal float types
 */
static int
tintseqset_transform_wavg(const TSequenceSet *ss, const Interval *interv,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
    k += tintseq_transform_wavg(TSEQUENCESET_SEQ_N(ss, i), interv, &result[k]);
  return k;
}

/**
 * @brief Transform a temporal integer sequence set into a temporal
 * double and extend it by a time interval
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @param[out] count Number of elements in the output array
 * @note There is no equivalent function for temporal float types
*/
TSequence **
tnumber_transform_wavg(const Temporal *temp, const Interval *interv,
  int *count)
{
  TSequence **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      TInstant *inst = (TInstant *) temp;
      result = palloc(sizeof(TSequence *));
      *count = tnumberinst_transform_wavg(inst, interv, result);
      break;
    }
    case TSEQUENCE:
    {
      TSequence *seq = (TSequence *) temp;
      result = palloc(sizeof(TSequence *) * seq->count);
      *count = MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ?
        tnumberdiscseq_transform_wavg(seq, interv, result) :
        tintseq_transform_wavg(seq, interv, result);
      break;
    }
    default: /* TSEQUENCESET */
    {
      TSequenceSet *ss = (TSequenceSet *) temp;
      result = palloc(sizeof(TSequence *) * ss->totalcount);
      *count = tintseqset_transform_wavg(ss, interv, result);
    }
  }
  return result;
}

/*****************************************************************************
 * Generic moving window transition functions
 *****************************************************************************/

/**
 * @brief Generic moving window transition function for min, max, and sum
 * aggregation
 * @param[in,out] state Skiplist containing the state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @param[in] func Function
 * @param[in] min True if the calling function is min, max otherwise
 * @param[in] crossings True if turning points are added in the segments
 * @note This function is directly called by the window sum aggregation for
 * temporal floats after verifying since the operation is not supported for
 * sequence (set) type
 */
SkipList *
temporal_wagg_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv,
  datum_func2 func, bool min, bool crossings)
{
  int count;
  TSequence **sequences = temporal_extend(temp, interv, min, &count);
  SkipList *result = tcontseq_tagg_transfn(state, sequences[0], func,
    crossings);
  for (int i = 1; i < count; i++)
    result = tcontseq_tagg_transfn(result, sequences[i], func, crossings);
  pfree_array((void **) sequences, count);
  return result;
}

/**
 * @brief Transition function for moving window count and average aggregation
 * for temporal values
 */
SkipList *
temporal_wagg_transform_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv, datum_func2 func,
  TSequence ** (*transform)(const Temporal *, const Interval *, int *))
{
  int count;
  TSequence **sequences = transform(temp, interv, &count);
  SkipList *result = tcontseq_tagg_transfn(state, sequences[0], func, false);
  for (int i = 1; i < count; i++)
    result = tcontseq_tagg_transfn(result, sequences[i], func, false);
  pfree_array((void **) sequences, count);
  return result;
}

/*****************************************************************************
 * MEOS window aggregate transition functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wmin_transfn()
 */
SkipList *
tint_wmin_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_min_int32,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal minimum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmin_transfn()
 */
SkipList *
tfloat_wmin_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_min_float8,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wmax_transfn()
 */
SkipList *
tint_wmax_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_max_int32,
    GET_MAX, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal maximum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wmax_transfn()
 */
SkipList *
tfloat_wmax_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_max_float8,
    GET_MAX, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tint_wsum_transfn()
 */
SkipList *
tint_wsum_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TINT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_sum_int32,
    GET_MIN, CROSSINGS_NO);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal sum of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tfloat_wsum_transfn()
 */
SkipList *
tfloat_wsum_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_temporal_isof_type(temp, T_TFLOAT))
    return NULL;
  return temporal_wagg_transfn(state, temp, interv, &datum_sum_float8,
    GET_MIN, CROSSINGS);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal average of temporal numbers
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value
 * @param[in] interv Interval
 * @csqlfn #Tnumber_wavg_transfn()
 */
SkipList *
tnumber_wavg_transfn(SkipList *state, const Temporal *temp,
  const Interval *interv)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  /* Ensure validity of the arguments */
  if (! ensure_tnumber_type(temp->temptype))
    return NULL;
  return
  temporal_wagg_transform_transfn(state, temp, interv, &datum_sum_double2,
    &tnumber_transform_wavg);
}
#endif /* MEOS */

/*****************************************************************************/
