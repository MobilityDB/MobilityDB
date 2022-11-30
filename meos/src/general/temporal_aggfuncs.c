/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief General aggregate functions for temporal types.
 */

#include "general/temporal_aggfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
// #include <catalog/pg_collation.h>
// #include <libpq/pqformat.h>
// #include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporaltypes.h"
#include "general/temporal_tile.h"
#include "general/tbool_boolops.h"
#include "general/doublen.h"
#include "general/time_aggfuncs.h"

/*****************************************************************************
 * Aggregate functions on datums
 *****************************************************************************/

/**
 * Return the minimum value of the two arguments
 */
Datum
datum_min_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

/**
 * Return the maximum value of the two arguments
 */
Datum
datum_max_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

/**
 * Return the minimum value of the two arguments
 */
Datum
datum_min_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

/**
 * Return the maximum value of the two arguments
 */
Datum
datum_max_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

/**
 * Return the minimum value of the two arguments
 */
 Datum
datum_min_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

/**
 * Return the maximum value of the two arguments
 */
Datum
datum_max_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/**
 * Return the sum of the two arguments
 */
Datum
datum_sum_float8(Datum l, Datum r)
{
  return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

/**
 * Return the sum of the two arguments
 */
Datum
datum_sum_double2(Datum l, Datum r)
{
  return PointerGetDatum(double2_add((double2 *) DatumGetPointer(l),
    (double2 *) DatumGetPointer(r)));
}

/**
 * Return the sum of the two arguments
 */
Datum
datum_sum_double3(Datum l, Datum r)
{
  return PointerGetDatum(double3_add((double3 *) DatumGetPointer(l),
    (double3 *) DatumGetPointer(r)));
}

/**
 * Return the sum of the two arguments
 */
Datum
datum_sum_double4(Datum l, Datum r)
{
  return PointerGetDatum(double4_add((double4 *) DatumGetPointer(l),
    (double4 *) DatumGetPointer(r)));
}

/*****************************************************************************
 * Generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate function for temporal instants
 *
 * @param[in] instants1 Accumulated state
 * @param[in] instants2 Instants of the input temporal discrete sequence
 * @note Return new sequences that must be freed by the calling function.
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
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      result[count++] = tinstant_make(
        func(tinstant_value(inst1), tinstant_value(inst2)), inst1->temptype,
        inst1->t);
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
  /* We finished to aggregate state1 */
  assert (i == count1);
  /* Copy the instants from state2 that are after the end of state1 */
  while (j < count2)
    result[count++] = tinstant_copy(instants2[j++]);
  *newcount = count;
  return result;
}

/**
 * Generic aggregate function for temporal sequences
 *
 * @param[in] seq1,seq2 Temporal sequence values to be aggregated
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 * @param[out] result Array on which the pointers of the newly constructed
 * ranges are stored
 * @note Return new sequences that must be freed by the calling function
 */
static int
tsequence_tagg1(const TSequence *seq1, const TSequence *seq2,
  datum_func2 func, bool crossings, TSequence **result)
{
  Period inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
  {
    const TSequence *sequences[2];
    /* The two sequences do not intersect: copy the sequences in the right order */
    if (span_cmp(&seq1->period, &seq2->period) < 0)
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
    TSequence **normseqs = tseqarr_normalize(sequences, 2, &count);
    for (int i = 0; i < count; i++)
      result[i] = normseqs[i];
    pfree(normseqs);
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
  TimestampTz lower1 = DatumGetTimestampTz(seq1->period.lower);
  TimestampTz upper1 = DatumGetTimestampTz(seq1->period.upper);
  bool lower1_inc = seq1->period.lower_inc;
  bool upper1_inc = seq1->period.upper_inc;

  TimestampTz lower2 = DatumGetTimestampTz(seq2->period.lower);
  TimestampTz upper2 = DatumGetTimestampTz(seq2->period.upper);
  bool lower2_inc = seq2->period.lower_inc;
  bool upper2_inc = seq2->period.upper_inc;

  TimestampTz lower = DatumGetTimestampTz(inter.lower);
  TimestampTz upper = DatumGetTimestampTz(inter.upper);
  bool lower_inc = inter.lower_inc;
  bool upper_inc = inter.upper_inc;
  TSequence *sequences[3];
  int k = 0;

  /* Compute the aggregation on the period before the
   * intersection of the intervals */
  int cmp1 = timestamptz_cmp_internal(lower1, lower);
  int cmp2 = timestamptz_cmp_internal(lower2, lower);
  if (cmp1 < 0 || (lower1_inc && !lower_inc && cmp1 == 0))
  {
    span_set(TimestampTzGetDatum(lower1), TimestampTzGetDatum(lower),
      lower1_inc, ! lower_inc, T_TIMESTAMPTZ, &period);
    sequences[k++] = tcontseq_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (lower2_inc && !lower_inc && cmp2 == 0))
  {
    span_set(TimestampTzGetDatum(lower2), TimestampTzGetDatum(lower),
      lower2_inc, ! lower_inc, T_TIMESTAMPTZ, &period);
    sequences[k++] = tcontseq_at_period(seq2, &period);
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
      func(tinstant_value(inst1), tinstant_value(inst2)), seq1->temptype,
      inst1->t);
  }
  sequences[k++] = tsequence_make_free(instants, syncseq1->count,
    lower_inc, upper_inc, MOBDB_FLAGS_GET_INTERP(seq1->flags), NORMALIZE);
  pfree(syncseq1); pfree(syncseq2);

  /* Compute the aggregation on the period after the intersection
   * of the intervals */
  cmp1 = timestamptz_cmp_internal(upper, upper1);
  cmp2 = timestamptz_cmp_internal(upper, upper2);
  if (cmp1 < 0 || (!upper_inc && upper1_inc && cmp1 == 0))
  {
    span_set(TimestampTzGetDatum(upper), TimestampTzGetDatum(upper1),
      ! upper_inc, upper1_inc, T_TIMESTAMPTZ, &period);
    sequences[k++] = tcontseq_at_period(seq1, &period);
  }
  else if (cmp2 < 0 || (! upper_inc && upper2_inc && cmp2 == 0))
  {
    span_set(TimestampTzGetDatum(upper), TimestampTzGetDatum(upper2),
      ! upper_inc, upper2_inc, T_TIMESTAMPTZ, &period);
    sequences[k++] = tcontseq_at_period(seq2, &period);
  }

  /* Normalization */
  if (k == 1)
  {
    result[0] = sequences[0];
    return 1;
  }
  int count;
  TSequence **normseqs = tseqarr_normalize(
    (const TSequence **) sequences, k, &count);
  for (int i = 0; i < count; i++)
    result[i] = normseqs[i];
  pfree(normseqs);
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
 * @param[in] crossings True if turning points are added in the segments
 * @param[out] newcount Number of elements in the result
 * @note Return new sequences that must be freed by the calling function.
 */
TSequence **
tsequence_tagg(TSequence **sequences1, int count1, TSequence **sequences2,
  int count2, datum_func2 func, bool crossings, int *newcount)
{
  /*
   * Each sequence can be split 3 times, there may be count - 1 holes between
   * sequences for both sequences1 and sequences2, and there may be
   * 2 sequences before and after.
   */
  int seqcount = (count1 * 3) + count1 + count2 + 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * seqcount);
  int i = 0, j = 0, k = 0;
  TSequence *seq1 = sequences1[i];
  TSequence *seq2 = sequences2[j];
  while (i < count1 && j < count2)
  {
    int countstep = tsequence_tagg1(seq1, seq2, func, crossings, &sequences[k]);
    k += countstep - 1;
    /* If both upper bounds are equal */
    int cmp = timestamptz_cmp_internal(seq1->period.upper, seq2->period.upper);
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
  TSequence **result = tseqarr_normalize((const TSequence **) sequences,
    k, &count);
  pfree_array((void **) sequences, k);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

/**
 * Ensure that the subtype and the interpolation of the skiplist and temporal
 * point are the same
 */
void
ensure_same_tempsubtype_skiplist(SkipList *state, Temporal *temp)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (state->elemtype != TEMPORAL || head->subtype != temp->subtype)
    elog(ERROR, "Cannot aggregate temporal values of different type");
  if (MOBDB_FLAGS_GET_LINEAR(head->flags) !=
    MOBDB_FLAGS_GET_LINEAR(temp->flags))
    elog(ERROR, "Cannot aggregate temporal values of different interpolation");
  return;
}

/**
 * Generic transition function for aggregating temporal values
 * of instant subtype
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] inst Temporal value
 * @param[in] func Function
 */
SkipList *
tinstant_tagg_transfn(SkipList *state, const TInstant *inst, datum_func2 func)
{
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) &inst, 1, TEMPORAL);
  else
  {
    ensure_same_tempsubtype_skiplist(state, (Temporal *) inst);
    skiplist_splice(state, (void **) &inst, 1, func, false);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal discrete sequence values
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function
 */
SkipList *
tdiscseq_tagg_transfn(SkipList *state, const TSequence *seq, datum_func2 func)
{
  int count;
  const TInstant **instants = tsequence_instants(seq, &count);
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) instants, seq->count, TEMPORAL);
  else
  {
    ensure_same_tempsubtype_skiplist(state, (Temporal *) instants[0]);
    skiplist_splice(state, (void **) instants, seq->count, func, false);
    result = state;
  }
  pfree(instants);
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence subtype
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
tsequence_tagg_transfn(SkipList *state,
  TSequence *seq, datum_func2 func, bool crossings)
{
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) &seq, 1, TEMPORAL);
  else
  {
    ensure_same_tempsubtype_skiplist(state, (Temporal *) seq);
    skiplist_splice(state, (void **) &seq, 1, func, crossings);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence set subtype
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] ss Temporal value
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
tsequenceset_tagg_transfn(SkipList *state, const TSequenceSet *ss,
  datum_func2 func, bool crossings)
{
  const TSequence **sequences = tsequenceset_sequences_p(ss);
  SkipList *result;
  if (! state)
    result = skiplist_make((void **)sequences, ss->count, TEMPORAL);
  else
  {
    ensure_same_tempsubtype_skiplist(state, (Temporal *) sequences[0]);
    skiplist_splice(state, (void **) sequences, ss->count, func,
      crossings);
    result = state;
  }
  pfree(sequences);
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence set subtype
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] temp Temporal value
 * @param[in] func Function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
temporal_tagg_transfn(SkipList *state, Temporal *temp, datum_func2 func,
  bool crossings)
{
  ensure_valid_tempsubtype(temp->subtype);
  SkipList *result;
  if (temp->subtype == TINSTANT)
    result =  tinstant_tagg_transfn(state, (TInstant *) temp, func);
  else if (temp->subtype == TSEQUENCE)
    result = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      tdiscseq_tagg_transfn(state, (TSequence *) temp, func) :
      tsequence_tagg_transfn(state, (TSequence *) temp, func, crossings);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_tagg_transfn(state, (TSequenceSet *) temp,
      func, crossings);
  return result;
}

/**
 * Generic combine function for aggregating temporal values
 *
 * @param[in] state1, state2 State values
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @note This function is called for aggregating temporal points and thus
 * after checking the dimensionality and the SRID of the values
 */
SkipList *
temporal_tagg_combinefn(SkipList *state1, SkipList *state2,
  datum_func2 func, bool crossings)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  Temporal *head2 = (Temporal *) skiplist_headval(state2);
  ensure_same_tempsubtype_skiplist(state1, head2);
  int count2 = state2->length;
  void **values2 = skiplist_values(state2);
  skiplist_splice(state1, values2, count2, func, crossings);
  pfree_array(values2, count2);
  return state1;
}

/**
 * @brief Generic final function for aggregating temporal values
 * @ingroup libmeos_temporal_agg
 *
 * @param[in] state State values
 */
Temporal *
temporal_tagg_finalfn(SkipList *state)
{
  if (state == NULL || state->length == 0)
    return NULL;

  Temporal **values = (Temporal **) skiplist_values(state);
  Temporal *result;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tsequence_make((const TInstant **) values,
      state->length, true, true, DISCRETE, NORMALIZE_NO);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tsequenceset_make((const TSequence **) values,
      state->length, NORMALIZE);
  pfree(values);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal and of temporal booleans.
 * @sqlfunc tand()
 */
SkipList *
tbool_tand_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_and, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal booleans.
 * @sqlfunc tor()
 */
SkipList *
tbool_tor_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_or, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tmin()
 */
SkipList *
tint_tmin_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_min_int32, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tmin()
 */
SkipList *
tfloat_tmin_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_min_float8, CROSSINGS);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tmax()
 */
SkipList *
tint_tmax_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_max_int32, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tmax()
 */
SkipList *
tfloat_tmax_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_max_float8, CROSSINGS);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tsum()
 */
SkipList *
tint_tsum_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_sum_int32, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal count of temporal values.
 * @sqlfunc tsum()
 */
SkipList *
tfloat_tsum_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_sum_float8, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal average of temporal numbers.
 * @sqlfunc tavg()
 */
SkipList *
tnumber_tavg_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transform_transfn(state, temp, &datum_sum_double2,
    CROSSINGS_NO, &tnumberinst_transform_tavg);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal min of temporal text values.
 * @sqlfunc tmin()
 */
SkipList *
ttext_tmin_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_min_text, CROSSINGS_NO);
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Transition function for temporal max of temporal text values.
 * @sqlfunc tmax()
 */
SkipList *
ttext_tmax_transfn(SkipList *state, Temporal *temp)
{
  return temporal_tagg_transfn(state, temp, &datum_max_text, CROSSINGS_NO);
}

#endif /* MEOS */

/*****************************************************************************
 * Generic functions for aggregating temporal values that require a
 * transformation to be applied to each composing instant/sequence
 * such as average and centroid. The function passed as
 * argument is the function for transforming the temporal instant value
 *****************************************************************************/

/**
 * Transform a temporal discrete sequence value for aggregation
 */
TInstant **
tdiscseq_transform_tagg(const TSequence *seq,
  TInstant *(*func)(const TInstant *))
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    result[i] = func(inst);
  }
  return result;
}

/**
 * Transform a temporal sequence value for aggregation
 */
TSequence *
tcontseq_transform_tagg(const TSequence *seq,
  TInstant *(*func)(const TInstant *))
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = func(inst);
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * Transform a temporal sequence set value for aggregation
 */
TSequence **
tsequenceset_transform_tagg(const TSequenceSet *ss,
  TInstant *(*func)(const TInstant *))
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    result[i] = tcontseq_transform_tagg(seq, func);
  }
  return result;
}

/**
 * Transform a temporal value for aggregation
 */
Temporal **
temporal_transform_tagg(const Temporal *temp, int *count,
  TInstant *(*func)(const TInstant *))
{
  Temporal **result;
  if (temp->subtype == TINSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *)func((TInstant *) temp);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (MOBDB_FLAGS_GET_DISCRETE(temp->flags))
    {
      result = (Temporal **) tdiscseq_transform_tagg((TSequence *) temp,
        func);
      *count = ((TSequence *) temp)->count;
    }
    else
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tcontseq_transform_tagg((TSequence *) temp,
        func);
      *count = 1;
    }
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = (Temporal **) tsequenceset_transform_tagg((TSequenceSet *) temp,
      func);
    *count = ((TSequenceSet *) temp)->count;
  }
  assert(result != NULL);
  return result;
}

/**
 * Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 *
 * @param[in] state Current state
 * @param[in] temp New temporal value
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @param[in] transform Transform function
 */
SkipList *
temporal_tagg_transform_transfn(SkipList *state, Temporal *temp,
  datum_func2 func, bool crossings, TInstant *(*transform)(const TInstant *))
{
  int count;
  Temporal **temparr = temporal_transform_tagg(temp, &count, transform);
  if (state)
  {
    ensure_same_tempsubtype_skiplist(state, temparr[0]);
    skiplist_splice(state, (void **) temparr, count, func, crossings);
  }
  else
  {
    state = skiplist_make((void **) temparr, count, TEMPORAL);
  }

  pfree_array((void **) temparr, count);
  return state;
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

/**
 * Transform a temporal instant value into a temporal integer value for
 * performing temporal count aggregation
 */
TInstant *
tinstant_transform_tcount(const TInstant *inst, const Interval *interval,
  TimestampTz origin)
{
  TimestampTz t = inst->t;
  if (interval)
    t = timestamptz_bucket(t, interval, origin);
  return tinstant_make(Int32GetDatum(1), T_TINT, t);
}

/**
 * Transform a temporal discrete sequence value into a temporal integer value
 * for performing temporal count aggregation
 */
TInstant **
tdiscseq_transform_tcount(const TSequence *seq, const Interval *interval,
  TimestampTz origin)
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    TimestampTz t = inst->t;
    if (interval)
      t = timestamptz_bucket(t, interval, origin);
    result[i] = tinstant_make(datum_one, T_TINT, t);
  }
  return result;
}

/**
 * Transform a temporal sequence value into a temporal integer value for
 * performing temporal count aggregation
 */
TSequence *
tcontseq_transform_tcount(const TSequence *seq, const Interval *interval,
  TimestampTz origin)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TimestampTz t = seq->period.lower;
  if (interval)
    t = timestamptz_bucket(t, interval, origin);
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(datum_one, T_TINT, t);
    result = tinstant_to_tsequence(inst, STEPWISE);
    pfree(inst);
    return result;
  }

  TInstant *instants[2];
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  t = seq->period.upper;
  /* The upper timestamp must be gridded to the next bucket */
  if (interval)
  {
    int64 size = interval_units(interval);
    t = timestamptz_bucket(t, interval, origin) + size;
  }
  instants[1] = tinstant_make(datum_one, T_TINT, t);
  result = tsequence_make((const TInstant **) instants, 2,
    seq->period.lower_inc, seq->period.upper_inc, STEPWISE, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return result;
}

/**
 * Transform a temporal sequence set value into a temporal integer value for
 * performing temporal count aggregation
 */
TSequence **
tsequenceset_transform_tcount(const TSequenceSet *ss, const Interval *interval,
  TimestampTz origin)
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    result[i] = tcontseq_transform_tcount(seq, interval, origin);
  }
  return result;
}

/**
 * Transform a temporal value into a temporal integer value for
 * performing temporal count aggregation (dispatch function)
 */
Temporal **
temporal_transform_tcount(const Temporal *temp, Interval *interval,
  TimestampTz origin, int *count)
{
  Temporal **result;
  if (temp->subtype == TINSTANT)
  {
    result = palloc(sizeof(Temporal *));
    result[0] = (Temporal *) tinstant_transform_tcount((TInstant *) temp,
      interval, origin);
    *count = 1;
  }
  else if (temp->subtype == TSEQUENCE)
  {
    if (MOBDB_FLAGS_GET_DISCRETE(temp->flags))
    {
      result = (Temporal **) tdiscseq_transform_tcount((TSequence *) temp,
        interval, origin);
      *count = ((TSequence *) temp)->count;
    }
    else
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tcontseq_transform_tcount((TSequence *) temp,
        interval, origin);
      *count = 1;
    }
  }
  else /* temp->subtype == TSEQUENCESET */
  {
    result = (Temporal **) tsequenceset_transform_tcount((TSequenceSet *) temp,
      interval, origin);
    *count = ((TSequenceSet *) temp)->count;
  }
  assert(result != NULL);
  return result;
}

/**
 * @ingroup libmeos_temporal_agg
 * @brief Generic transition function for temporal aggregation
 */
SkipList *
temporal_tcount_transfn(SkipList *state, Temporal *temp, Interval *interval,
  TimestampTz origin)
{
  int count;
  Temporal **temparr = temporal_transform_tcount(temp, interval, origin, &count);
  if (state)
  {
    ensure_same_tempsubtype_skiplist(state, temparr[0]);
    skiplist_splice(state, (void **) temparr, count, &datum_sum_int32,
      false);
  }
  else
  {
    state = skiplist_make((void **) temparr, count, TEMPORAL);
  }
  pfree_array((void **) temparr, count);
  return state;
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
  double value = tnumberinst_double(inst);
  double2 dvalue;
  double2_set(value, 1, &dvalue);
  TInstant *result = tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2,
    inst->t);
  return result;
}

/**
 * Final function for temporal average aggregation of temporal instat values
 */
TSequence *
tinstant_tavg_finalfn(TInstant **instants, int count)
{
  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    TInstant *inst = instants[i];
    double2 *value = (double2 * )DatumGetPointer(&inst->value);
    double tavg = value->a / value->b;
    newinstants[i] = tinstant_make(Float8GetDatum(tavg), T_TFLOAT, inst->t);
  }
  return tsequence_make_free(newinstants, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * Final function for temporal average aggregation of temporal sequence values
 */
TSequenceSet *
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
      double2 *value2 = (double2 *) DatumGetPointer(&inst->value);
      double value = value2->a / value2->b;
      instants[j] = tinstant_make(Float8GetDatum(value), T_TFLOAT, inst->t);
    }
    newsequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MOBDB_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

/**
 * @brief Final function for temporal average aggregation
 */
Temporal *
tnumber_tavg_finalfn(SkipList *state)
{
  if (state->length == 0)
    return NULL;

  Temporal **values = (Temporal **) skiplist_values(state);
  Temporal *result;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tinstant_tavg_finalfn((TInstant **) values,
      state->length);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tsequence_tavg_finalfn((TSequence **) values,
      state->length);
  pfree(values);
  return result;
}

/*****************************************************************************/

/**
 * Transition function for temporal extent aggregation of temporal values
 */
Period *
temporal_extent_transfn(Period *p, Temporal *temp)
{
  Period *result;

  /* Can't do anything with null inputs */
  if (! p && ! temp)
    return NULL;
  /* Null period and non-null temporal, return the bbox of the temporal */
  if (! p)
  {
    result = palloc0(sizeof(Period));
    temporal_set_period(temp, result);
    return result;
  }
  /* Non-null period and null temporal, return the period */
  if (! temp)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    return result;
  }

  Period p1;
  temporal_set_period(temp, &p1);
  result = bbox_union_span_span(p, &p1, false);
  return result;
}

/**
 * Transition function for temporal extent aggregation for temporal numbers
 */
TBox *
tnumber_extent_transfn(TBox *box, Temporal *temp)
{
  /* Can't do anything with null inputs */
  if (!box && !temp)
    return NULL;
  TBox *result = palloc0(sizeof(TBox));
  /* Null box and non-null temporal, return the bbox of the temporal */
  if (! box)
  {
    temporal_set_bbox(temp, result);
    return result;
  }
  /* Non-null box and null temporal, return the box */
  if (! temp)
  {
    memcpy(result, box, sizeof(TBox));
    return result;
  }
  /* Both box and temporal are not null */
  temporal_set_bbox(temp, result);
  tbox_expand(box, result);
  return result;
}

/*****************************************************************************/
