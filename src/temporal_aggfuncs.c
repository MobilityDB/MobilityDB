/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file temporal_aggfuncs.c
 * Temporal aggregate functions
 */

#include "temporal_aggfuncs.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <catalog/pg_collation.h>
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>

#include "skiplist.h"
#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "tempcache.h"
#include "temporal_util.h"
#include "tbool_boolops.h"
#include "temporal_boxops.h"
#include "doublen.h"
#include "time_aggfuncs.h"

TInstant **
tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2,
  int count2, datum_func2 func, int *newcount);
TSequence **
tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2,
  int count2, datum_func2 func, bool crossings, int *newcount);

/*****************************************************************************
 * Aggregate functions on datums
 *****************************************************************************/

/**
 * Returns the minimum value of the two arguments
 */
Datum
datum_min_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

/**
 * Returns the maximum value of the two arguments
 */
Datum
datum_max_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

/**
 * Returns the minimum value of the two arguments
 */
Datum
datum_min_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

/**
 * Returns the maximum value of the two arguments
 */
Datum
datum_max_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

/**
 * Returns the minimum value of the two arguments
 */
 Datum
datum_min_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

/**
 * Returns the maximum value of the two arguments
 */
Datum
datum_max_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/**
 * Returns the sum of the two arguments
 */
Datum
datum_sum_float8(Datum l, Datum r)
{
  return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

/**
 * Returns the sum of the two arguments
 */
Datum
datum_sum_double2(Datum l, Datum r)
{
  return PointerGetDatum(double2_add((double2 *)DatumGetPointer(l),
    (double2 *)DatumGetPointer(r)));
}

/**
 * Returns the sum of the two arguments
 */
Datum
datum_sum_double3(Datum l, Datum r)
{
  return PointerGetDatum(double3_add((double3 *)DatumGetPointer(l),
    (double3 *)DatumGetPointer(r)));
}

/**
 * Returns the sum of the two arguments
 */
Datum
datum_sum_double4(Datum l, Datum r)
{
  return PointerGetDatum(double4_add((double4 *)DatumGetPointer(l),
    (double4 *)DatumGetPointer(r)));
}

/*****************************************************************************
 * TInstant generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate function for temporal instants
 *
 * @param[in] instants1 Accumulated state
 * @param[in] instants2 instants of the input temporal instant set value
 * @note Returns new sequences that must be freed by the calling function.
 */
TInstant **
tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2,
  int count2, datum_func2 func, int *newcount)
{
  TInstant **result = palloc(sizeof(TInstant *) * (count1 + count2));
  int i = 0, j = 0, count = 0;
  while (i < count1 && j < count2)
  {
    TInstant *inst1 = instants1[i];
    TInstant *inst2 = instants2[j];
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      result[count++] = tinstant_make(
        func(tinstant_value(inst1), tinstant_value(inst2)),
        inst1->t, inst1->basetypid);
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      result[count++] = tinstant_copy(inst1);
      i++;
    }
    else
    {
      result[count++] = tinstant_copy(inst2);
      j++;
    }
  }
  /* Copy the instants from state1 or state2 that are after the end of the
     other state */
  while (i < count1)
    result[count++] = tinstant_copy(instants1[i++]);
  while (j < count2)
    result[count++] = tinstant_copy(instants2[j++]);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * TSequence generic aggregation functions
 *****************************************************************************/

/**
 * Generic aggregate function for temporal sequences
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * ranges are stored
 * @param[in] seq1,seq2 Temporal sequence values to be aggregated
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 * @note Returns new sequences that must be freed by the calling function
 */
static int
tsequence_tagg1(TSequence **result, const TSequence *seq1, const TSequence *seq2,
  datum_func2 func, bool crossings)
{
  Period *intersect = intersection_period_period_internal(&seq1->period, &seq2->period);
  if (intersect == NULL)
  {
    const TSequence *sequences[2];
    /* The two sequences do not intersect: copy the sequences in the right order */
    if (period_cmp_internal(&seq1->period, &seq2->period) < 0)
    {
      sequences[0] = (TSequence *) seq1;
      sequences[1] = (TSequence *) seq2;
    }
    else
    {
      sequences[0] = (TSequence *) seq2;
      sequences[1] = (TSequence *) seq1;
    }
    /* Normalization */
    int count;
    TSequence **normsequences = tsequencearr_normalize(sequences, 2, &count);
    for (int i = 0; i < count; i++)
      result[i] = normsequences[i];
    pfree(normsequences);
    return count;
  }

  /*
   * If the two sequences intersect there will be at most 3 sequences in the
   * result: one before the intersection, one for the intersection, and one
   * after the intersection. This will be also the case for sequences with
   * step interpolation (e.g., tint) that has the last value different
   * from the previous one as tint '[1@2000-01-03, 2@2000-01-04]' and
   * tint '[3@2000-01-01, 4@2000-01-05]' whose result for sum would be the
   * following three sequences
   * [3@2000-01-01, 3@2000-01-03), [4@2000-01-03, 5@2000-01-04], and
   * (3@2000-01-04, 4@2000-01-05] which after normalization becomes
   * [3@2000-01-01, 4@2000-01-03, 5@2000-01-04], and
   * (3@2000-01-04, 4@2000-01-05]
   */
  Period period;
  TimestampTz lower1 = seq1->period.lower;
  TimestampTz upper1 = seq1->period.upper;
  bool lower1_inc = seq1->period.lower_inc;
  bool upper1_inc = seq1->period.upper_inc;

  TimestampTz lower2 = seq2->period.lower;
  TimestampTz upper2 = seq2->period.upper;
  bool lower2_inc = seq2->period.lower_inc;
  bool upper2_inc = seq2->period.upper_inc;

  TimestampTz lower = intersect->lower;
  TimestampTz upper = intersect->upper;
  bool lower_inc = intersect->lower_inc;
  bool upper_inc = intersect->upper_inc;
  TSequence *sequences[3];
  int k = 0;

  /* Compute the aggregation on the period before the
   * intersection of the intervals */
  int cmp1 = timestamp_cmp_internal(lower1, lower);
  int cmp2 = timestamp_cmp_internal(lower2, lower);
  if (cmp1 < 0 || (lower1_inc && !lower_inc && cmp1 == 0))
  {
    period_set(&period, lower1, lower, lower1_inc, !lower_inc);
    sequences[k++] = tsequence_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (lower2_inc && !lower_inc && cmp2 == 0))
  {
    period_set(&period, lower2, lower, lower2_inc, !lower_inc);
    sequences[k++] = tsequence_at_period(seq2, &period);
  }

  /*
   * Compute the aggregation on the intersection of intervals
   */
  TSequence *syncseq1, *syncseq2;
  synchronize_tsequence_tsequence(seq1, seq2, &syncseq1, &syncseq2, crossings);
  TInstant **instants = palloc(sizeof(TInstant *) * syncseq1->count);
  for (int i = 0; i < syncseq1->count; i++)
  {
    const TInstant *inst1 = tsequence_inst_n(syncseq1, i);
    const TInstant *inst2 = tsequence_inst_n(syncseq2, i);
    instants[i] = tinstant_make(
      func(tinstant_value(inst1), tinstant_value(inst2)),
      inst1->t, inst1->basetypid);
  }
  sequences[k++] = tsequence_make_free(instants, syncseq1->count,
    lower_inc, upper_inc, MOBDB_FLAGS_GET_LINEAR(seq1->flags), NORMALIZE);
  pfree(syncseq1); pfree(syncseq2);

  /* Compute the aggregation on the period after the intersection
   * of the intervals */
  cmp1 = timestamp_cmp_internal(upper, upper1);
  cmp2 = timestamp_cmp_internal(upper, upper2);
  if (cmp1 < 0 || (!upper_inc && upper1_inc && cmp1 == 0))
  {
    period_set(&period, upper, upper1, !upper_inc, upper1_inc);
    sequences[k++] = tsequence_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (!upper_inc && upper2_inc && cmp2 == 0))
  {
    period_set(&period, upper, upper2, !upper_inc, upper2_inc);
    sequences[k++] = tsequence_at_period(seq2, &period);
  }
  pfree(intersect);

  /* Normalization */
  if (k == 1)
  {
    result[0] = sequences[0];
    return 1;
  }
  int count;
  TSequence **normsequences = tsequencearr_normalize(
    (const TSequence **) sequences, k, &count);
  for (int i = 0; i < count; i++)
    result[i] = normsequences[i];
  pfree(normsequences);
  return count;
}

/**
 * Generic aggregate function for temporal sequences.
 *
 * @param[in] sequences1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] sequences2 Sequences of a temporal sequence set value
 * @param[in] count2 Number of elements in the temporal sequence set value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 * @param[in] newcount Number of elements in the result
 * @note Returns new sequences that must be freed by the calling function.
 */
TSequence **
tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2,
  int count2, datum_func2 func, bool crossings, int *newcount)
{
  /*
   * Each sequence can be split 3 times, there may be count - 1 holes between
   * sequences for both sequences1 and sequences2, and there may be
   * 2 sequences before and after.
   * TODO Verify this formula
   */
  int seqcount = (count1 * 3) + count1 + count2 + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * seqcount);
  int i = 0, j = 0, k = 0;
  TSequence *seq1 = sequences1[i];
  TSequence *seq2 = sequences2[j];
  while (i < count1 && j < count2)
  {
    int countstep = tsequence_tagg1(&sequences[k], seq1, seq2, func, crossings);
    k += countstep - 1;
    /* If both upper bounds are equal */
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0 && seq1->period.upper_inc == seq2->period.upper_inc)
    {
      k++; i++; j++;
      if (i == count1 || j == count2)
        break;
      seq1 = sequences1[i];
      seq2 = sequences2[j];
    }
    /* If upper bound of seq1 is less than or equal to the upper bound of seq2 */
    else if (cmp < 0 ||
      (!seq1->period.upper_inc && seq2->period.upper_inc && cmp == 0))
    {
      i++;
      if (i == count1)
      {
        k++; j++;
        break;
      }
      seq1 = sequences1[i];
      seq2 = sequences[k];
    }
    else
    {
      j++;
      if (j == count2)
      {
        k++; i++;
        break;
      }
      seq1 = sequences[k];
      seq2 = sequences2[j];
    }
  }
  while (i < count1)
    sequences[k++] = tsequence_copy(sequences1[i++]);
  while (j < count2)
    sequences[k++] = tsequence_copy(sequences2[j++]);

  /* Normalization */
  if (k == 1)
  {
    TSequence **result = palloc(sizeof(TSequence *));
    result[0] = sequences[0];
    pfree(sequences);
    *newcount = 1;
    return result;
  }
  int count;
  TSequence **result = tsequencearr_normalize((const TSequence **) sequences,
    k, &count);
  pfree_array((void **) sequences, k);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

void
ensure_same_temp_subtype_skiplist(SkipList *state, int16 subtype,
  Temporal *temp)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (state->elemtype != TEMPORAL || head->subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate temporal values of different type")));
  if (MOBDB_FLAGS_GET_LINEAR(head->flags) !=
    MOBDB_FLAGS_GET_LINEAR(temp->flags))
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate temporal values of different interpolation")));
  return;
}

/**
 * Generic transition function for aggregating temporal values
 * of instant subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] inst Temporal value
 * @param[in] func Function
 */
static SkipList *
tinstant_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TInstant *inst, datum_func2 func)
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) &inst, 1, TEMPORAL);
  else
  {
    ensure_same_temp_subtype_skiplist(state, INSTANT, (Temporal *) inst);
    skiplist_splice(fcinfo, state, (void **) &inst, 1, func, false);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of instant set subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ti Temporal value
 * @param[in] func Function
 */
static SkipList *
tinstantset_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TInstantSet *ti, datum_func2 func)
{
  const TInstant **instants = tinstantset_instants(ti);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) instants, ti->count, TEMPORAL);
  else
  {
    ensure_same_temp_subtype_skiplist(state, INSTANT, (Temporal *) ti);
    skiplist_splice(fcinfo, state, (void **) instants, ti->count, func, false);
    result = state;
  }
  pfree(instants);
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
SkipList *
tsequence_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  TSequence *seq, datum_func2 func, bool crossings)
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) &seq, 1, TEMPORAL);
  else
  {
    ensure_same_temp_subtype_skiplist(state, SEQUENCE, (Temporal *) seq);
    skiplist_splice(fcinfo, state, (void **) &seq, 1, func, crossings);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence set subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ts Temporal value
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
static SkipList *
tsequenceset_tagg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TSequenceSet *ts, datum_func2 func, bool crossings)
{
  const TSequence **sequences = tsequenceset_sequences(ts);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **)sequences, ts->count, TEMPORAL);
  else
  {
    ensure_same_temp_subtype_skiplist(state, SEQUENCE, (Temporal *) ts);
    skiplist_splice(fcinfo, state, (void **) sequences, ts->count, func, crossings);
    result = state;
  }
  pfree(sequences);
  return result;
}

/*****************************************************************************
 * Generic aggregate functions for TInstant and TSequence
 *****************************************************************************/

/**
 * Generic transition function for aggregating temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 */
static Datum
temporal_tagg_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_valid_tempsubtype(temp->subtype);
  SkipList *result;
  if (temp->subtype == INSTANT)
    result =  tinstant_tagg_transfn(fcinfo, state, (TInstant *) temp,
      func);
  else if (temp->subtype == INSTANTSET)
    result =  tinstantset_tagg_transfn(fcinfo, state, (TInstantSet *) temp,
      func);
  else if (temp->subtype == SEQUENCE)
    result =  tsequence_tagg_transfn(fcinfo, state, (TSequence *) temp,
      func, crossings);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_tagg_transfn(fcinfo, state, (TSequenceSet *) temp,
      func, crossings);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Generic combine function for aggregating temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] state1, state2 State values
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 * @note This function is called for aggregating temporal points and thus
 * after checking the dimensionality and the SRID of the values
 */
SkipList *
temporal_tagg_combinefn1(FunctionCallInfo fcinfo, SkipList *state1,
  SkipList *state2, datum_func2 func, bool crossings)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  Temporal *head2 = (Temporal *) skiplist_headval(state2);
  ensure_same_temp_subtype_skiplist(state1, head2->subtype, head2);
  int count2 = state2->length;
  void **values2 = skiplist_values(state2);
  skiplist_splice(fcinfo, state1, values2, count2, func, crossings);
  pfree_array(values2, count2);
  return state1;
}

/**
 * Generic combine function for aggregating temporal alphanumeric values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] crossings State whether turning points are added in the segments
 */
Datum
temporal_tagg_combinefn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings)
{
  SkipList *state1 = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  SkipList *state2 = PG_ARGISNULL(1) ? NULL :
    (SkipList *) PG_GETARG_POINTER(1);
  if (state1 == NULL && state2 == NULL)
    PG_RETURN_NULL();

  SkipList *result = temporal_tagg_combinefn1(fcinfo, state1, state2, func,
    crossings);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tagg_finalfn);
/**
 * Generic final function for temporal aggregation
 */
PGDLLEXPORT Datum
temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = (Temporal **) skiplist_values(state);
  Temporal *result = NULL;
  assert(values[0]->subtype == INSTANT ||
    values[0]->subtype == SEQUENCE);
  if (values[0]->subtype == INSTANT)
    result = (Temporal *) tinstantset_make((const TInstant **)values,
      state->length, MERGE_NO);
  else /* values[0]->subtype == SEQUENCE */
    result = (Temporal *) tsequenceset_make((const TSequence **)values,
      state->length, NORMALIZE);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Generic functions for aggregating temporal values that require a
 * transformation to be applied to each composing instant/sequence
 * such as average and centroid. The function passed as
 * argument is the function for transforming the temporal instant value
 *****************************************************************************/

/**
 * Transform a temporal instant set value for aggregation
 */
TInstant **
tinstantset_transform_tagg(const TInstantSet *ti,
  TInstant *(*func)(const TInstant *))
{
  TInstant **result = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    result[i] = func(inst);
  }
  return result;
}

/**
 * Transform a temporal sequence value for aggregation
 */
TSequence *
tsequence_transform_tagg(const TSequence *seq,
  TInstant *(*func)(const TInstant *))
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = func(inst);
  }
  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc,
    MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE_NO);
}

/**
 * Transform a temporal sequence set value for aggregation
 */
TSequence **
tsequenceset_transform_tagg(const TSequenceSet *ts,
  TInstant *(*func)(const TInstant *))
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result[i] = tsequence_transform_tagg(seq, func);
  }
  return result;
}

/**
 * Transform a temporal value for aggregation (dispatch function)
 */
Temporal **
temporal_transform_tagg(const Temporal *temp, int *count,
  TInstant *(*func)(const TInstant *))
{
  Temporal **result;
  if (temp->subtype == INSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)func((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = (Temporal **) tinstantset_transform_tagg((
      TInstantSet *) temp, func);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tsequence_transform_tagg(
      (TSequence *) temp, func);
    *count = 1;
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = (Temporal **) tsequenceset_transform_tagg(
      (TSequenceSet *) temp, func);
    *count = ((TSequenceSet *) temp)->count;
  }
  assert(result != NULL);
  return result;
}

/*****************************************************************************/

/**
 * Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Aggregate function
 * @param[in] crossings State whether turning points are added in the segments
 * @param[in] transform Transform function
 */
Datum
temporal_tagg_transform_transfn(FunctionCallInfo fcinfo, datum_func2 func,
  bool crossings, TInstant *(*transform)(const TInstant *))
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Temporal *temp = PG_GETARG_TEMPORAL(1);
  int count;
  Temporal **temparr = temporal_transform_tagg(temp, &count, transform);
  if (state)
  {
    ensure_same_temp_subtype_skiplist(state, temparr[0]->subtype, temparr[0]);
    skiplist_splice(fcinfo, state, (void **) temparr, count, func, crossings);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) temparr, count, TEMPORAL);
  }

  pfree_array((void **) temparr, count);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

/**
 * Transform a temporal instant value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant *
tinstant_transform_tcount(const TInstant *inst)
{
  return tinstant_make(Int32GetDatum(1), inst->t, INT4OID);
}

/**
 * Transform a temporal instant set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
tinstantset_transform_tcount(const TInstantSet *ti)
{
  TInstant **result = palloc(sizeof(TInstant *) * ti->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    result[i] = tinstant_make(datum_one, inst->t, INT4OID);
  }
  return result;
}

/**
 * Transform a temporal sequence value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
tsequence_transform_tcount(const TSequence *seq)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(datum_one, seq->period.lower, INT4OID);
    result = tinstant_to_tsequence(inst, STEP);
    pfree(inst);
    return result;
  }

  TInstant *instants[2];
  instants[0] = tinstant_make(datum_one, seq->period.lower, INT4OID);
  instants[1] = tinstant_make(datum_one, seq->period.upper, INT4OID);
  result = tsequence_make((const TInstant **) instants, 2,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return result;
}

/**
 * Transform a temporal sequence set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence **
tsequenceset_transform_tcount(const TSequenceSet *ts)
{
  TSequence **result = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    result[i] = tsequence_transform_tcount(seq);
  }
  return result;
}

/**
 * Transform a temporal value into a temporal integer value for
 * performing temporal count aggregation (dispatch function)
 */
static Temporal **
temporal_transform_tcount(const Temporal *temp, int *count)
{
  Temporal **result;
  if (temp->subtype == INSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tinstant_transform_tcount((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == INSTANTSET)
  {
    result = (Temporal **) tinstantset_transform_tcount((TInstantSet *) temp);
    *count = ((TInstantSet *) temp)->count;
  }
  else if (temp->subtype == SEQUENCE)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tsequence_transform_tcount((TSequence *) temp);
    *count = 1;
  }
  else /* temp->subtype == SEQUENCESET */
  {
    result = (Temporal **) tsequenceset_transform_tcount((TSequenceSet *) temp);
    *count = ((TSequenceSet *) temp)->count;
  }
  assert(result != NULL);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_tcount_transfn);
/**
 * Generic transition function for temporal aggregation
 */
PGDLLEXPORT Datum
temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Temporal *temp = PG_GETARG_TEMPORAL(1);
  int count;
  Temporal **temparr = temporal_transform_tcount(temp, &count);
  if (state)
  {
    ensure_same_temp_subtype_skiplist(state, temparr[0]->subtype, temparr[0]);
    skiplist_splice(fcinfo, state, (void **) temparr, count, &datum_sum_int32, false);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) temparr, count, TEMPORAL);
  }

  pfree_array((void **) temparr, count);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(temporal_tcount_combinefn);
/**
 * Generic combine function for temporal aggregation
 */
PGDLLEXPORT Datum
temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, false);
}

/*****************************************************************************
 * Temporal extent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_extent_transfn);
/**
 * Transition function for temporal extent aggregation of temporal values
 * with period bounding box
 */
PGDLLEXPORT Datum
temporal_extent_transfn(PG_FUNCTION_ARGS)
{
  Period *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_PERIOD(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);
  Period *result;

  /* Can't do anything with null inputs */
  if (!p && !temp)
    PG_RETURN_NULL();
  /* Null period and non-null temporal, return the bbox of the temporal */
  if (!p)
  {
    result = palloc0(sizeof(Period));
    temporal_bbox(result, temp);
    PG_RETURN_POINTER(result);
  }
  /* Non-null period and null temporal, return the period */
  if (!temp)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    PG_RETURN_POINTER(result);
  }

  Period p1;
  temporal_bbox(&p1, temp);
  result = period_super_union(p, &p1);

  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_extent_combinefn);
/**
 * Combine function for temporal extent aggregation
 */
PGDLLEXPORT Datum
temporal_extent_combinefn(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_PERIOD(0);
  Period *p2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_PERIOD(1);

  if (!p2 && !p1)
    PG_RETURN_NULL();
  if (p1 && !p2)
    PG_RETURN_POINTER(p1);
  if (p2 && !p1)
    PG_RETURN_POINTER(p2);

  Period *result = period_super_union(p1, p2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_extent_transfn);
/**
 * Transition function for temporal extent aggregation for temporal numbers
 */
PGDLLEXPORT Datum
tnumber_extent_transfn(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);

  /* Can't do anything with null inputs */
  if (!box && !temp)
    PG_RETURN_NULL();
  TBOX *result = palloc0(sizeof(TBOX));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (!box)
  {
    temporal_bbox(result, temp);
    PG_RETURN_POINTER(result);
  }
  /* Non-null box and null temporal, return the box */
  if (!temp)
  {
    memcpy(result, box, sizeof(TBOX));
    PG_RETURN_POINTER(result);
  }

  /* Both box and temporal are not null */
  temporal_bbox(result, temp);
  tbox_expand(result, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_extent_combinefn);
/**
 * Combine function for temporal extent aggregation for temporal numbers
 */
PGDLLEXPORT Datum
tnumber_extent_combinefn(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_TBOX_P(1);

  if (!box2 && !box1)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_POINTER(box1);
  if (box2 && !box1)
    PG_RETURN_POINTER(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality_tbox(box1, box2);
  TBOX *result = tbox_copy(box1);
  tbox_expand(result, box2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal boolean functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbool_tand_transfn);
/**
 * Transition function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tand_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tand_combinefn);
/**
 * Combine function for temporal and aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_and, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tor_transfn);
/**
 * Transition function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tor_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_or, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tbool_tor_combinefn);
/**
 * Combine function for temporal or aggregation of temporal boolean values
 */
PGDLLEXPORT Datum
tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_or, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tint_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tint_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_float8, CROSSINGS);
}

PG_FUNCTION_INFO_V1(tint_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tint_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal integer values
 */
PGDLLEXPORT Datum
tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_int32, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_transfn);
/**
 * Transition function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_combinefn);
/**
 * Combine function for temporal sum aggregation of temporal float values
 */
PGDLLEXPORT Datum
tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_float8, CROSSINGS_NO);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_tmin_transfn);
/**
 * Transition function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmin_combinefn);
/**
 * Combine function for temporal minimum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_min_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmax_transfn);
/**
 * Transition function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transfn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

PG_FUNCTION_INFO_V1(ttext_tmax_combinefn);
/**
 * Combine function for temporal maximum aggregation of temporal text values
 */
PGDLLEXPORT Datum
ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_max_text, CROSSINGS_NO);
}

/*****************************************************************************
 * Temporal average
 *****************************************************************************/

/**
 * Transform a temporal number into a temporal double2 value for
 * performing temporal average aggregation
 */
TInstant *
tnumberinst_transform_tavg(const TInstant *inst)
{
  double value = datum_double(tinstant_value(inst), inst->basetypid);
  double2 dvalue;
  double2_set(&dvalue, value, 1);
  TInstant *result = tinstant_make(PointerGetDatum(&dvalue), inst->t,
    type_oid(T_DOUBLE2));
  return result;
}

PG_FUNCTION_INFO_V1(tnumber_tavg_transfn);
/**
 * Transition function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_transfn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_transform_transfn(fcinfo, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

PG_FUNCTION_INFO_V1(tnumber_tavg_combinefn);
/**
 * Combine function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_combinefn(PG_FUNCTION_ARGS)
{
  return temporal_tagg_combinefn(fcinfo, &datum_sum_double2, false);
}

/* Final function for tavg */

/**
 * Final function for temporal average aggregation of temporal instat values
 */
static TInstantSet *
tinstant_tavg_finalfn(TInstant **instants, int count)
{
  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    TInstant *inst = instants[i];
    double2 *value = (double2 *)DatumGetPointer(tinstant_value_ptr(inst));
    double tavg = value->a / value->b;
    newinstants[i] = tinstant_make(Float8GetDatum(tavg), inst->t,
      FLOAT8OID);
  }
  return tinstantset_make_free(newinstants, count, MERGE_NO);
}

/**
 * Final function for temporal average aggregation of temporal sequence values
 */
static TSequenceSet *
tsequence_tavg_finalfn(TSequence **sequences, int count)
{
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    TSequence *seq = sequences[i];
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      double2 *value2 = (double2 *)DatumGetPointer(tinstant_value_ptr(inst));
      double value = value2->a / value2->b;
      instants[j] = tinstant_make(Float8GetDatum(value), inst->t,
        FLOAT8OID);
    }
    newsequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tnumber_tavg_finalfn);
/**
 * Final function for temporal average aggregation
 */
PGDLLEXPORT Datum
tnumber_tavg_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  Temporal **values = (Temporal **) skiplist_values(state);
  assert(values[0]->subtype == INSTANT || values[0]->subtype == SEQUENCE);
  Temporal *result = (values[0]->subtype == INSTANT) ?
    (Temporal *) tinstant_tavg_finalfn((TInstant **)values, state->length) :
    (Temporal *) tsequence_tavg_finalfn((TSequence **)values, state->length);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
