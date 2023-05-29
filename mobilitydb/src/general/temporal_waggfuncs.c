/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Window aggregate functions for temporal types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/doublen.h"
#include "general/temporal_aggfuncs.h"
#include "general/time_aggfuncs.h"
#include "general/tsequence.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Helper macro to input the current aggregate state
 */
#define INPUT_AGG_TRANS_STATE_ARG(fcinfo, state)  \
  do {  \
    MemoryContext ctx = set_aggregation_context(fcinfo); \
    state = PG_ARGISNULL(0) ?  \
      NULL : (SkipList *) PG_GETARG_POINTER(0);  \
    if (PG_ARGISNULL(1) || PG_ARGISNULL(2))  \
    {  \
      if (state)  \
        PG_RETURN_POINTER(state);  \
      else  \
        PG_RETURN_NULL();  \
    }  \
    unset_aggregation_context(ctx); \
  } while (0)

/**
 * @brief Extend the temporal instant value by the time interval
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tinstant_extend(const TInstant *inst, const Interval *interval,
  TSequence **result)
{
  TInstant *instants[2];
  TimestampTz upper = pg_timestamp_pl_interval(inst->t, interval);
  instants[0] = (TInstant *) inst;
  instants[1] = tinstant_make(tinstant_value(inst), inst->temptype, upper);
  result[0] = tsequence_make((const TInstant **) instants, 2, true, true,
    MEOS_FLAGS_GET_CONTINUOUS(inst->flags) ? LINEAR : STEP, NORMALIZE_NO);
  pfree(instants[1]);
  return 1;
}

/**
 * @brief Extend the temporal discrete sequence value by the time interval
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tdiscseq_extend(const TSequence *seq, const Interval *interval,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    tinstant_extend(inst, interval, &result[i]);
  }
  return seq->count;
}

/**
 * @brief Extend the temporal sequence value by the time interval
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min, max otherwise.
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored. This parameter is only used for linear interpolation.
 */
static int
tcontseq_extend(const TSequence *seq, const Interval *interval, bool min,
  TSequence **result)
{
  if (seq->count == 1)
    return tinstant_extend(TSEQUENCE_INST_N(seq, 0), interval, result);

  TInstant *instants[3];
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  bool lower_inc = seq->period.lower_inc;
  meosType basetype = temptype_basetype(seq->temptype);
  for (int i = 0; i < seq->count - 1; i++)
  {
    TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq, i + 1);
    Datum value2 = tinstant_value(inst2);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;

    /* Step interpolation or constant segment */
    if (interp != LINEAR || datum_eq(value1, value2, basetype))
    {
      TimestampTz upper = pg_timestamp_pl_interval(inst2->t, interval);
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
        (datum_gt(value1, value2, basetype) && !min))
      {
        /* Extend the start value for the duration of the window */
        TimestampTz lower = pg_timestamp_pl_interval(inst1->t, interval);
        TimestampTz upper = pg_timestamp_pl_interval(inst2->t, interval);
        instants[0] = inst1;
        instants[1] = tinstant_make(value1, inst1->temptype, lower);
        instants[2] = tinstant_make(value2, inst1->temptype, upper);
        result[i] = tsequence_make((const TInstant **) instants, 3,
          lower_inc, upper_inc, interp, NORMALIZE_NO);
        pfree(instants[1]); pfree(instants[2]);
      }
      else
      {
        /* Extend the end value for the duration of the window */
        TimestampTz upper = pg_timestamp_pl_interval(seq->period.upper,
          interval);
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
 * @brief Extend the temporal sequence set value by the time interval
 * @param[in] ss Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min, max otherwise.
 * This parameter is only used for linear interpolation.
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tsequenceset_extend(const TSequenceSet *ss, const Interval *interval, bool min,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    k += tcontseq_extend(seq, interval, min, &result[k]);
  }
  return k;
}

/**
 * @brief Extend the temporal value by the time interval
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min, max otherwise
 * @param[out] count Number of elements in the output array
 */
static TSequence **
temporal_extend(Temporal *temp, Interval *interval, bool min, int *count)
{
  TSequence **result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    TInstant *inst = (TInstant *) temp;
    result = palloc(sizeof(TSequence *));
    *count = tinstant_extend(inst, interval, result);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    TSequence *seq = (TSequence *) temp;
    result = palloc(sizeof(TSequence *) * seq->count);
    *count = MEOS_FLAGS_GET_DISCRETE(seq->flags) ?
      tdiscseq_extend(seq, interval, result) :
      tcontseq_extend(seq, interval, min, result);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    TSequenceSet *ss = (TSequenceSet *) temp;
    result = palloc(sizeof(TSequence *) * ss->totalcount);
    *count = tsequenceset_extend(ss, interval, min, result);
  }
  return result;
}

/*****************************************************************************
 * Transform a temporal number type into a temporal integer type with value 1
 * extended by a time interval.
 *****************************************************************************/

/**
 * @brief Construct a sequence with a value 1 by extending the two bounds by
 * the time interval (iterator function)
 */
static TSequence *
tinstant_transform_wcount_iter(TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc, const Interval *interval)
{
  TInstant *instants[2];
  TimestampTz upper1 = pg_timestamp_pl_interval(upper, interval);
  instants[0] = tinstant_make(Int32GetDatum(1), T_TINT, lower);
  instants[1] = tinstant_make(Int32GetDatum(1), T_TINT, upper1);
  TSequence *result = tsequence_make((const TInstant **) instants, 2,
    lower_inc, upper_inc, STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return result;
}

/**
 * @brief Transform the temporal number instant value by the time interval
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tinstant_transform_wcount(const TInstant *inst, const Interval *interval,
  TSequence **result)
{
  result[0] = tinstant_transform_wcount_iter(inst->t, inst->t, true, true,
    interval);
  return 1;
}

/**
 * @brief Transform the temporal number discrete sequence value by the time
 * interval
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tdiscseq_transform_wcount(const TSequence *seq, const Interval *interval,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    tinstant_transform_wcount(inst, interval, &result[i]);
  }
  return seq->count;
}

/**
 * @brief Transform the temporal number sequence value by the time interval
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tcontseq_transform_wcount(const TSequence *seq, const Interval *interval,
  TSequence **result)
{
  if (seq->count == 1)
    return tinstant_transform_wcount(TSEQUENCE_INST_N(seq, 0), interval,
      result);

  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
    result[i] = tinstant_transform_wcount_iter(inst1->t, inst2->t, lower_inc,
      upper_inc, interval);
    inst1 = inst2;
    lower_inc = true;
  }
  return seq->count - 1;
}

/**
 * @brief Transform the temporal number sequence set value by the time interval
 * @param[in] ss Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tsequenceset_transform_wcount(const TSequenceSet *ss, const Interval *interval,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    k += tcontseq_transform_wcount(seq, interval, &result[k]);
  }
  return k;
}

/**
 * @brief Transform the temporal number by the time interval
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[out] count Number of elements in the output array
 */
static TSequence **
temporal_transform_wcount(const Temporal *temp, const Interval *interval,
  int *count)
{
  assert(temptype_subtype(temp->subtype));
  TSequence **result;
  if (temp->subtype == TINSTANT)
  {
    TInstant *inst = (TInstant *) temp;
    result = palloc(sizeof(TSequence *));
    *count = tinstant_transform_wcount(inst, interval, result);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    TSequence *seq = (TSequence *) temp;
    result = palloc(sizeof(TSequence *) * seq->count);
    *count = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      tdiscseq_transform_wcount(seq, interval, result) :
      tcontseq_transform_wcount(seq, interval, result);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    TSequenceSet *ss = (TSequenceSet *) temp;
    result = palloc(sizeof(TSequence *) * ss->totalcount);
    *count = tsequenceset_transform_wcount(ss, interval, result);
  }
  return result;
}

/*****************************************************************************/

/**
 * @brief Transform the temporal number into a temporal double and extend it
 * by the time interval
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tnumberinst_transform_wavg(const TInstant *inst, const Interval *interval,
  TSequence **result)
{
  /* TODO: Should be an additional attribute */
  float8 value = 0.0;
  assert(tnumber_type(inst->temptype));
  if (inst->temptype == T_TINT)
    value = DatumGetInt32(tinstant_value(inst));
  else /* inst->temptype == T_TFLOAT */
    value = DatumGetFloat8(tinstant_value(inst));
  double2 dvalue;
  double2_set(value, 1, &dvalue);
  TimestampTz upper = pg_timestamp_pl_interval(inst->t, interval);
  TInstant *instants[2];
  instants[0] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, inst->t);
  instants[1] = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, upper);
  result[0] = tsequence_make((const TInstant**) instants, 2, true, true,
    MEOS_FLAGS_GET_CONTINUOUS(inst->flags) ? LINEAR : STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return 1;
}

/**
 * @brief Transform the temporal number into a temporal double and extend it
 * by the time interval
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 */
static int
tnumberdiscseq_transform_wavg(const TSequence *seq, const Interval *interval,
  TSequence **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    tnumberinst_transform_wavg(inst, interval, &result[i]);
  }
  return seq->count;
}

/**
 * @brief Transform the temporal integer sequence value into a temporal double
 * and extend it by a time interval
 *
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 * @note There is no equivalent function for temporal float types
 */
static int
tintseq_transform_wavg(const TSequence *seq, const Interval *interval,
  TSequence **result)
{
  const TInstant *inst1;
  TInstant *instants[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    tnumberinst_transform_wavg(inst1, interval, &result[0]);
    return 1;
  }

  /* General case */
  inst1 = TSEQUENCE_INST_N(seq, 0);
  bool lower_inc = seq->period.lower_inc;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
    double value = DatumGetInt32(tinstant_value(inst1));
    double2 dvalue;
    double2_set(value, 1, &dvalue);
    TimestampTz upper = pg_timestamp_pl_interval(inst2->t, interval);
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
* Transform the temporal integer sequence set value into a temporal double and extend
 * it by a time interval
 *
 * @param[in] ss Temporal value
 * @param[in] interval Interval
 * @param[out] result Array on which the pointers of the newly constructed
 * values are stored
 * @note There is no equivalent function for temporal float types
 */
static int
tintseqset_transform_wavg(const TSequenceSet *ss, const Interval *interval,
  TSequence **result)
{
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    k += tintseq_transform_wavg(seq, interval, &result[k]);
  }
  return k;
}

/**
 * @brief Transform the temporal integer sequence set value into a temporal
 * double and extend it by a time interval
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[out] count Number of elements in the output array
 * @note There is no equivalent function for temporal float types
*/
static TSequence **
tnumber_transform_wavg(const Temporal *temp, const Interval *interval,
  int *count)
{
  TSequence **result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    TInstant *inst = (TInstant *) temp;
    result = palloc(sizeof(TSequence *));
    *count = tnumberinst_transform_wavg(inst, interval, result);
  }
  else if (temp->subtype == TSEQUENCE)
  {
    TSequence *seq = (TSequence *) temp;
    result = palloc(sizeof(TSequence *) * seq->count);
    *count = MEOS_FLAGS_GET_DISCRETE(seq->flags) ?
      tnumberdiscseq_transform_wavg(seq, interval, result) :
      tintseq_transform_wavg(seq, interval, result);
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    TSequenceSet *ss = (TSequenceSet *) temp;
    result = palloc(sizeof(TSequence *) * ss->totalcount);
    *count = tintseqset_transform_wavg(ss, interval, result);
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
 * @param[in] interval Interval
 * @param[in] func Function
 * @param[in] min True if the calling function is min, max otherwise
 * @param[in] crossings True if turning points are added in the segments
 * @note This function is directly called by the window sum aggregation for
 * temporal floats after verifying since the operation is not supported for
 * sequence (set) type
 */
static SkipList *
temporal_wagg_transfn1(SkipList *state, Temporal *temp, Interval *interval,
  datum_func2 func, bool min, bool crossings)
{
  int count;
  TSequence **sequences = temporal_extend(temp, interval, min, &count);
  SkipList *result = tcontseq_tagg_transfn(state, sequences[0],
    func, crossings);
  for (int i = 1; i < count; i++)
    result = tcontseq_tagg_transfn(result, sequences[i],
      func, crossings);
  pfree_array((void **) sequences, count);
  return result;
}

/**
 * @brief Generic moving window transition function for min, max, and sum
 * aggregation
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] min True if the calling function is min, max otherwise
 * @param[in] crossings True if turning points are added in the segments
 */
Datum
temporal_wagg_transfn(FunctionCallInfo fcinfo, datum_func2 func, bool min,
  bool crossings)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE_ARG(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Interval *interval = PG_GETARG_INTERVAL_P(2);
  if ( temp->subtype != TINSTANT && ! MEOS_FLAGS_GET_DISCRETE(temp->flags) &&
      temp->temptype == T_TFLOAT && func == &datum_sum_float8)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Operation not supported for temporal continuous float sequences")));

  store_fcinfo(fcinfo);
  SkipList *result = temporal_wagg_transfn1(state, temp, interval, func, min,
    crossings);

  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Transition function for moving window count and average aggregation
 * for temporal values
 */
Datum
temporal_wagg_transform_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  TSequence ** (*transform)(const Temporal *, const Interval *, int *))
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE_ARG(fcinfo, state);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Interval *interval = PG_GETARG_INTERVAL_P(2);
  store_fcinfo(fcinfo);
  int count;
  TSequence **sequences = transform(temp, interval, &count);
  SkipList *result = tcontseq_tagg_transfn(state, sequences[0], func, false);
  for (int i = 1; i < count; i++)
    result = tcontseq_tagg_transfn(result, sequences[i], func, false);
  pfree_array((void **) sequences, count);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(interval, 2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tint_wmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wmin_transfn);
/**
 * @brief Transition function for moving window minimun aggregation for
 * temporal integer values
 */
Datum
Tint_wmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_min_int32, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_wmin_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wmin_transfn);
/**
 * @brief Transition function for moving window minimun
 */
Datum
Tfloat_wmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_min_float8, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Tint_wmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wmax_transfn);
/**
 * @brief Transition function for moving window maximun aggregation for
 * temporal integer values
 */
Datum
Tint_wmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_max_int32, GET_MAX, CROSSINGS);
}

PGDLLEXPORT Datum Tfloat_wmax_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wmax_transfn);
/**
 * @brief Transition function for moving window maximun aggregation for
 * temporal float values
 */
Datum
Tfloat_wmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_max_float8, GET_MAX, CROSSINGS);
}

PGDLLEXPORT Datum Tint_wsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tint_wsum_transfn);
/**
 * @brief Transition function for moving window sum aggregation for temporal
 * integer values
 */
Datum
Tint_wsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_sum_int32, GET_MIN, CROSSINGS_NO);
}

PGDLLEXPORT Datum Tfloat_wsum_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_wsum_transfn);
/**
 * @brief Transition function for moving window sum aggregation for temporal
 * float values
 */
Datum
Tfloat_wsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transfn(fcinfo, &datum_sum_float8, GET_MIN, CROSSINGS);
}

PGDLLEXPORT Datum Temporal_wcount_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_wcount_transfn);
/**
 * @brief Transition function for moving window count aggregation for temporal
 * values
 */
Datum
Temporal_wcount_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transform_transfn(fcinfo, &datum_sum_int32,
    &temporal_transform_wcount);
}

PGDLLEXPORT Datum Tnumber_wavg_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_wavg_transfn);
/**
 * @brief Transition function for moving window average aggregation for
 * temporal values
 */
Datum
Tnumber_wavg_transfn(PG_FUNCTION_ARGS)
{
  return temporal_wagg_transform_transfn(fcinfo, &datum_sum_double2,
    &tnumber_transform_wavg);
}

/*****************************************************************************/
