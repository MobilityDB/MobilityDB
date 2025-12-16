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
 * @file
 * @brief General aggregate functions for temporal types
 */

#include "temporal/temporal_aggfuncs.h"

/* C */
#include <assert.h>
#include <string.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#include "utils/varlena.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/skiplist.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tbool_ops.h"
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/tsequenceset.h"
#include "temporal/type_util.h"

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

#if ! MEOS
  extern FunctionCallInfo fetch_fcinfo();
  extern void store_fcinfo(FunctionCallInfo fcinfo);
  extern MemoryContext set_aggregation_context(FunctionCallInfo fcinfo);
  extern void unset_aggregation_context(MemoryContext ctx);
#endif /* ! MEOS */

/*****************************************************************************
 * Aggregate functions on datums
 *****************************************************************************/

/**
 * @brief Return the minimum value of the two arguments
 */
Datum
datum_min_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

/**
 * @brief Return the maximum value of the two arguments
 */
Datum
datum_max_int32(Datum l, Datum r)
{
  return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

/**
 * @brief Return the minimum value of the two arguments
 */
Datum
datum_min_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

/**
 * @brief Return the maximum value of the two arguments
 */
Datum
datum_max_float8(Datum l, Datum r)
{
  return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

/**
 * @brief Return the minimum value of the two arguments
 */
 Datum
datum_min_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

/**
 * @brief Return the maximum value of the two arguments
 */
Datum
datum_max_text(Datum l, Datum r)
{
  return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_int32(Datum l, Datum r)
{
  return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_float8(Datum l, Datum r)
{
  return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_double2(Datum l, Datum r)
{
  return PointerGetDatum(double2_add((double2 *) DatumGetPointer(l),
    (double2 *) DatumGetPointer(r)));
}

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_double3(Datum l, Datum r)
{
  return PointerGetDatum(double3_add((double3 *) DatumGetPointer(l),
    (double3 *) DatumGetPointer(r)));
}

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_double4(Datum l, Datum r)
{
  return PointerGetDatum(double4_add((double4 *) DatumGetPointer(l),
    (double4 *) DatumGetPointer(r)));
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

bool
ensure_same_skiplist_subtype(SkipList *state, uint8 subtype)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (head->subtype != subtype)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot aggregate temporal values of different subtype");
    return false;
  }
  return true;
}

/*****************************************************************************
 * Specific skiplist functions for temporal aggregations
 *****************************************************************************/

/**
 * @brief Determine the relative position of a span and a timestamptz
 */
static int
span_timestamptz_cmp(const Span *s, TimestampTz t)
{
  if (left_span_value(s, TimestampTzGetDatum(t)))
    return -1;
  if (right_span_value(s, TimestampTzGetDatum(t)))
    return 1;
  return 0;
}

/**
 * @brief Determine the relative position of two periods
 */
static int
span_span_cmp(const Span *s1, const Span *s2)
{
  if (left_span_span(s1, s2))
    return -1;
  if (left_span_span(s2, s1))
    return 1;
  return 0;
}

/**
 * @brief Comparison function used for skiplists
 */
static int
temporal_skiplist_elempos(const SkipList *list, Span *s, int cur)
{
  if (cur == 0)
    return 1; /* Head is -inf */
  if (cur == -1 || cur == list->tail)
    return -1; /* Tail is +inf */

  Temporal *temp = (Temporal *) list->elems[cur].value;
  if (temp->subtype == TINSTANT)
    return span_timestamptz_cmp(s, ((TInstant *) temp)->t);
  else /* temp->subtype == TSEQUENCE */
    return span_span_cmp(s, &((TSequence *) temp)->period);
}

/**
 * @brief Determine the segment of the list that overlaps with the new set of
 * temporal values
 * @param[in] list Skiplist
 * @param[in] values Array of values
 * @param[in] count Number of elements in the array
 * @param[out] lower Array index of the start of the segment 
 * @param[out] upper Array index of the end of the segment 
 * @param[out] update Array of indices keeping the levels of the elements to
 * insert
 * @return Number of elements in the list that will be aggregated with the new
 * values, on error return -1
 */
int
temporal_skiplist_common(SkipList *list, void **values, int count,
  int *lower, int *upper, int update[SKIPLIST_MAXLEVEL])
{
  /* Temporal aggregation cannot mix instants and sequences */
  Temporal *temp1 = (Temporal *) skiplist_headval(list);
  Temporal *temp2 = (Temporal *) values[0];
  if (temp1->subtype != temp2->subtype)
  {
    meos_error(ERROR, MEOS_ERR_AGGREGATION_ERROR,
      "Cannot aggregate temporal values of different subtype");
    return -1;
  }
  if (MEOS_FLAGS_LINEAR_INTERP(temp1->flags) !=
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
  {
    meos_error(ERROR, MEOS_ERR_AGGREGATION_ERROR,
      "Cannot aggregate temporal values of different interpolation");
    return -1;
  }

  /* Compute the span of the new values */
  Span s;
  uint8 subtype = ((Temporal *) values[0])->subtype;
  if (subtype == TINSTANT)
  {
    TInstant *first = (TInstant *) values[0];
    TInstant *last = (TInstant *) values[count - 1];
    span_set(TimestampTzGetDatum(first->t), TimestampTzGetDatum(last->t),
      true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  }
  else /* subtype == TSEQUENCE */
  {
    TSequence *first = (TSequence *) values[0];
    TSequence *last = (TSequence *) values[count - 1];
    span_set(first->period.lower, last->period.upper, first->period.lower_inc,
      last->period.upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  }

  /* Find the list values that are strictly before the span of new values */
  memset(update, 0, sizeof(&update));
  int height = list->elems[0].height;
  SkipListElem *elem = &list->elems[0];
  int cur = 0;
  for (int level = height - 1; level >= 0; level--)
  {
    while (elem->next[level] != -1 &&
      temporal_skiplist_elempos(list, &s, elem->next[level]) == 1)
    {
      cur = elem->next[level];
      elem = &list->elems[cur];
    }
    update[level] = cur;
  }
  int lower1, upper1;
  cur = lower1 = elem->next[0];
  elem = &list->elems[cur];

  int result = 0;
  /* Count the number of elements that will be merged with the new values */
  while (temporal_skiplist_elempos(list, &s, cur) == 0)
  {
    cur = elem->next[0];
    elem = &list->elems[cur];
    result++;
  }
  upper1 = cur;
  /* Write output parameters and return */
  *lower = lower1;
  *upper = upper1;
  return result;
}

/**
 * @brief Return the new values obtained by merging the segment of the list
 * that overlaps with the new set of temporal values
 * @param[in] spliced Array of spliced values
 * @param[in] spliced_count Number of elements in the spliced array
 * @param[in] values Array of new values
 * @param[in] crossings True if turning points are added in the segments when
 * aggregating temporal value
 * @param[in] count Number of elements in the values array
 * @param[in] func Function used when aggregating temporal values, may be NULL
 * for the merge aggregate function
 * @param[out] newcount Number of elements in the output array
 * @param[out] tofree Array of values that must be freed
 * @param[out] nfree Number of values that must be freed
 */
void **
temporal_skiplist_merge(void **spliced, int spliced_count, void **values,
  int count, datum_func2 func, bool crossings, int *newcount, void ***tofree,
  int *nfree)
{
  *newcount = 0;
  void **newvalues;
  uint8 subtype = ((Temporal *) values[0])->subtype;
  if (subtype == TINSTANT)
    newvalues = (void **) tinstant_tagg((TInstant **) spliced, spliced_count,
      (TInstant **) values, count, func, newcount, tofree, nfree);
  else /* subtype == TSEQUENCE */
  {
    newvalues = (void **) tsequence_tagg((TSequence **) spliced, spliced_count,
      (TSequence **) values, count, func, crossings, newcount);
    *tofree = newvalues;
    *nfree = *newcount;
  }
  return newvalues;
}

/**
 * @brief Insert a new set of values to the skiplist while performing the 
 * aggregation between the new values that overlap with the values in the list
*/
void
temporal_skiplist_splice(SkipList *list, void **values, int count,
  datum_func2 func, bool crossings)
{
  return skiplist_splice(list, NULL, values, count, func, crossings, TEMPORAL);
}

/*****************************************************************************
 * Generic aggregation functions
 *****************************************************************************/

/**
 * @brief Generic aggregate function for temporal instants
 * @param[in] instants1 Instants of the accumulated state that overlap with
 * the new instants in instants2. At this point of the processing these
 * instants have been removed from the skiplist.
 * @param[in] instants2 Instants of the input temporal discrete sequence that
 * need to be merged into the skiplist.
 * @param[in] count1,count2 Number of values in the input arrays
 * @param[in] func Function, may be NULL for the merge aggregate function
 * @param[out] newcount Number of instants in the output array
 * @param[out] tofree Array of values that must be freed
 * @param[out] nfree Number of values that must be freed
 */
TInstant **
tinstant_tagg(TInstant **instants1, int count1, TInstant **instants2,
  int count2, datum_func2 func, int *newcount, void ***tofree, int *nfree)
{
  TInstant **result = palloc(sizeof(TInstant *) * (count1 + count2));
  void **tofree1 = palloc(sizeof(TInstant *) * Max(count1, count2));
  int i = 0, j = 0, count = 0, nfree1 = 0;
  while (i < count1 && j < count2)
  {
    const TInstant *inst1 = instants1[i];
    const TInstant *inst2 = instants2[j];
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      if (func)
      {
        result[count++] = tinstant_make(func(tinstant_value_p(inst1),
          tinstant_value_p(inst2)), inst1->temptype, inst1->t);
        if (tofree)
          tofree1[nfree1++] = result[count - 1];
      }
      else
      {
        if (tinstant_eq(inst1, inst2))
        {
          result[count++] = tinstant_copy(inst1);
        }
        else
        {
          char *t1 = pg_timestamptz_out(inst1->t);
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "The temporal values have different value at their common timestamp %s",
            t1);
          return NULL;
        }
      }
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
      if (tofree)
        tofree1[nfree1++] = result[count - 1];
      j++;
    }
  }
  /* We finished to aggregate state1 */
  assert (i == count1);
  /* Copy the instants from state2 that are after the end of state1 */
  while (j < count2)
  {
    result[count++] = tinstant_copy(instants2[j++]);
    if (tofree)
      tofree1[nfree1++] = result[count - 1];
  }
  /* Set the output parameters and return */
  *newcount = count;
  *tofree = tofree1;
  *nfree = nfree1;
  return result;
}

/**
 * @brief Generic aggregate function for temporal sequences (iterator function)
 * @param[in] seq1,seq2 Temporal sequence values to be aggregated
 * @param[in] func Function, may be NULL for the merge aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @param[out] result Array on which the pointers of the newly constructed
 * ranges are stored
 * @return On error return -1
 * @note Return new sequences that must be freed by the calling function
 */
static int
tsequence_tagg_iter(const TSequence *seq1, const TSequence *seq2,
  datum_func2 func, bool crossings, TSequence **result)
{
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
  {
    TSequence *sequences[2];
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
  Span period;
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
  int nseqs = 0;

  /* Compute the aggregation on the period before the intersection of the
   * intervals */
  int cmp1 = timestamptz_cmp_internal(lower1, lower);
  int cmp2 = timestamptz_cmp_internal(lower2, lower);
  if (cmp1 < 0 || (lower1_inc && !lower_inc && cmp1 == 0))
  {
    span_set(TimestampTzGetDatum(lower1), TimestampTzGetDatum(lower),
      lower1_inc, ! lower_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &period);
    sequences[nseqs++] = tcontseq_at_tstzspan(seq1, &period);
  }
  else if (cmp2 < 0 || (lower2_inc && !lower_inc && cmp2 == 0))
  {
    span_set(TimestampTzGetDatum(lower2), TimestampTzGetDatum(lower),
      lower2_inc, ! lower_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &period);
    sequences[nseqs++] = tcontseq_at_tstzspan(seq2, &period);
  }

  /*
   * Compute the aggregation on the intersection of intervals
   */
  TSequence *syncseq1, *syncseq2;
  synchronize_tsequence_tsequence(seq1, seq2, &syncseq1, &syncseq2, crossings);
  TInstant **instants = palloc(sizeof(TInstant *) * syncseq1->count);
  for (int i = 0; i < syncseq1->count; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(syncseq1, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(syncseq2, i);
    if (func)
    {
      Datum value = func(tinstant_value_p(inst1), tinstant_value_p(inst2));
      instants[i] = tinstant_make(value, seq1->temptype, inst1->t);
    }
    else
    {
      if (tinstant_eq(inst1, inst2))
        instants[i] = tinstant_copy(inst1);
      else
      {
        char *t1 = pg_timestamptz_out(inst1->t);
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "The temporal values have different value at their common timestamp %s",
          t1);
        for (int j = 0; j < i; j++)
          pfree(instants[i]);
        pfree(instants);
        return -1;
      }
    }
  }
  sequences[nseqs++] = tsequence_make_free(instants, syncseq1->count,
    lower_inc, upper_inc, MEOS_FLAGS_GET_INTERP(seq1->flags), NORMALIZE);
  pfree(syncseq1); pfree(syncseq2);

  /* Compute the aggregation on the period after the intersection of the
   * intervals */
  cmp1 = timestamptz_cmp_internal(upper, upper1);
  cmp2 = timestamptz_cmp_internal(upper, upper2);
  if (cmp1 < 0 || (!upper_inc && upper1_inc && cmp1 == 0))
  {
    span_set(TimestampTzGetDatum(upper), TimestampTzGetDatum(upper1),
      ! upper_inc, upper1_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &period);
    sequences[nseqs++] = tcontseq_at_tstzspan(seq1, &period);
  }
  else if (cmp2 < 0 || (! upper_inc && upper2_inc && cmp2 == 0))
  {
    span_set(TimestampTzGetDatum(upper), TimestampTzGetDatum(upper2),
      ! upper_inc, upper2_inc, T_TIMESTAMPTZ, T_TSTZSPAN, &period);
    sequences[nseqs++] = tcontseq_at_tstzspan(seq2, &period);
  }

  /* Normalization */
  if (nseqs == 1)
  {
    result[0] = sequences[0];
    return 1;
  }
  int count;
  TSequence **normseqs = tseqarr_normalize(sequences, nseqs, &count);
  for (int i = 0; i < count; i++)
    result[i] = normseqs[i];
  pfree(normseqs);
  for (int i = 0; i < nseqs; i++)
    pfree(sequences[i]);
  return count;
}

/**
 * @brief Generic aggregate function for temporal sequences
 * @param[in] sequences1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] sequences2 Sequences of a temporal sequence set
 * @param[in] count2 Number of elements in the temporal sequence set
 * @param[in] func Function, may be NULL for the merge aggregate function
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
  const TSequence *seq1 = sequences1[i];
  const TSequence *seq2 = sequences2[j];
  TSequence *tofree = NULL;
  while (i < count1 && j < count2)
  {
    int countstep = tsequence_tagg_iter(seq1, seq2, func, crossings,
      &sequences[k]);
    k += countstep - 1;

    /* Need to get all info from seq1 and seq2 since we might free one of them
     * right after */
    int cmp = timestamptz_cmp_internal(seq1->period.upper, seq2->period.upper);
    bool upper1_inc = seq1->period.upper_inc,
         upper2_inc = seq2->period.upper_inc;

    /* We just discarded sequences[k] from last iteration, so free it */
    if (tofree)
      pfree(tofree);

    /* If both upper bounds are equal */
    if (cmp == 0 && upper1_inc == upper2_inc)
    {
      k++; i++; j++;
      if (i == count1 || j == count2)
        break;
      seq1 = sequences1[i];
      seq2 = sequences2[j];
      /* Nothing to free in next iteration */
      tofree = NULL;
    }
    /* If the upper bound of seq1 is less than or equal to the upper bound of seq2 */
    else if (cmp < 0 ||
      (!upper1_inc && upper2_inc && cmp == 0))
    {
      i++;
      if (i == count1)
      {
        k++; j++;
        break;
      }
      /* sequences[k] will be overwritten in the next iteration so we need to
       * save and later free it */
      seq1 = sequences1[i];
      seq2 = tofree = sequences[k];
    }
    else
    {
      j++;
      if (j == count2)
      {
        k++; i++;
        break;
      }
      /* see above for tofree */
      seq1 = tofree = sequences[k];
      seq2 = sequences2[j];
    }
  }
  while (i < count1)
    sequences[k++] = tsequence_copy(sequences1[i++]);
  while (j < count2)
    sequences[k++] = tsequence_copy(sequences2[j++]);

  /* Normalization */
  TSequence **result;
  if (k == 1)
  {
    result = palloc(sizeof(TSequence *));
    result[0] = sequences[0];
    pfree(sequences);
    *newcount = 1;
    return result;
  }
  int count;
  result = tseqarr_normalize(sequences, k, &count);
  pfree_array((void **) sequences, k);
  *newcount = count;
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

/**
 * @brief Generic transition function for aggregating temporal values
 * of instant subtype
 * @param[in,out] state Current aggregate state
 * @param[in] inst Temporal value to aggregate
 * @param[in] func Function, may be NULL for the merge aggregate function
 */
SkipList *
tinstant_tagg_transfn(SkipList *state, const TInstant *inst, datum_func2 func)
{
  assert(inst);
  const TInstant **instants = palloc(sizeof(TInstant *));
  instants[0] = inst;
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) instants, 1, func, false);
  pfree(instants);
  return state;
}

/**
 * @brief Generic transition function for aggregating temporal discrete
 * sequence values
 * @param[in,out] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function, may be NULL for the merge aggregate function
 */
SkipList *
tdiscseq_tagg_transfn(SkipList *state, const TSequence *seq, datum_func2 func)
{
  assert(seq);
  const TInstant **instants = tsequence_insts_p(seq);
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) instants, seq->count, func, false);
  pfree(instants);
  return state;
}


/**
 * @brief Generic transition function for aggregating temporal values
 * of sequence subtype
 * @param[in,out] state Skiplist containing the state
 * @param[in] seq Temporal value
 * @param[in] func Function, may be NULL for the merge aggregate function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
tcontseq_tagg_transfn(SkipList *state, const TSequence *seq,
  datum_func2 func, bool crossings)
{
  assert(seq);
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) &seq, 1, func, crossings);
  return state;
}

/**
 * @brief Generic transition function for aggregating temporal values
 * of sequence set subtype
 * @param[in,out] state Skiplist containing the state
 * @param[in] ss Temporal value
 * @param[in] func Function, may be NULL for the merge aggregate function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
tsequenceset_tagg_transfn(SkipList *state, const TSequenceSet *ss,
  datum_func2 func, bool crossings)
{
  assert(ss);
  const TSequence **sequences = tsequenceset_sequences_p(ss);
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) sequences, ss->count, func,
    crossings);
  pfree(sequences);
  return state;
}

/**
 * @brief Generic transition function for aggregating temporal values
 * of sequence set subtype
 * @param[in,out] state Skiplist containing the state
 * @param[in] temp Temporal value
 * @param[in] func Function, may be NULL for the merge aggregate function
 * @param[in] crossings True if turning points are added in the segments
 */
SkipList *
temporal_tagg_transfn(SkipList *state, const Temporal *temp, datum_func2 func,
  bool crossings)
{
  assert(temp); assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return  tinstant_tagg_transfn(state, (TInstant *) temp, func);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        tdiscseq_tagg_transfn(state, (TSequence *) temp, func) :
        tcontseq_tagg_transfn(state, (TSequence *) temp, func, crossings);
    default: /* TSEQUENCESET */
      return tsequenceset_tagg_transfn(state, (TSequenceSet *) temp, func,
        crossings);
  }
}

/**
 * @brief Generic combine function for aggregating temporal values
 * @param[in] state1, state2 State values
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @note This function is called for aggregating temporal points and thus
 * after checking the dimensionality and the SRID of the values
 */
SkipList *
temporal_tagg_combinefn(SkipList *state1, SkipList *state2, datum_func2 func,
  bool crossings)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  if (state1->length == 0)
    return state2;
  if (state2->length == 0)
    return state1;

  int count2 = state2->length;
  void **values2 = skiplist_values(state2);
  temporal_skiplist_splice(state1, values2, count2, func, crossings);
  pfree(values2);
  return state1;
}

/**
 * @brief Return a copy of the temporal values contained in the skiplist
 */
Temporal **
skiplist_temporal_values(SkipList *list)
{
  Temporal **result = palloc(sizeof(Temporal *) * list->length);
  int cur = list->elems[0].next[0];
  int count = 0;
  while (cur != list->tail)
  {
    result[count++] = temporal_copy(list->elems[cur].value);
    cur = list->elems[cur].next[0];
  }
  return result;
}

/**
 * @ingroup meos_temporal_agg
 * @brief Generic final function for aggregating temporal values
 * @param[in] state Current aggregate state
 * @csqlfn #Temporal_tagg_finalfn()
 */
Temporal *
temporal_tagg_finalfn(SkipList *state)
{
  if (! state || state->length == 0)
    return NULL;
  /* A copy of the values is needed for switching from aggregate context,
   * for this reason the #skiplist_values cannot be used */
  Temporal **values = (Temporal **) skiplist_temporal_values(state);
  Temporal *result = NULL;
  assert(values[0]->subtype == TINSTANT || values[0]->subtype == TSEQUENCE);
  if (values[0]->subtype == TINSTANT)
    result = (Temporal *) tsequence_make_free((TInstant **) values,
      state->length, true, true, DISCRETE, NORMALIZE_NO);
  else /* values[0]->subtype == TSEQUENCE */
    result = (Temporal *) tsequenceset_make_free((TSequence **) values,
      state->length, NORMALIZE);
  skiplist_free(state);
  return result;
}

/*****************************************************************************
 * Generic functions for aggregating temporal values that require a
 * transformation to be applied to each composing instant/sequence
 * such as average and centroid. The function passed as
 * argument is the function for transforming the temporal instant value
 *****************************************************************************/

/**
 * @brief Transform a temporal discrete sequence value for aggregation
 */
TInstant **
tdiscseq_transform_tagg(const TSequence *seq,
  TInstant *(*func)(const TInstant *))
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    result[i] = func(TSEQUENCE_INST_N(seq, i));
  return result;
}

/**
 * @brief Transform a temporal sequence value for aggregation
 */
TSequence *
tcontseq_transform_tagg(const TSequence *seq,
  TInstant *(*func)(const TInstant *))
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = func(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE_NO);
}

/**
 * @brief Transform a temporal sequence set for aggregation
 */
TSequence **
tsequenceset_transform_tagg(const TSequenceSet *ss,
  TInstant *(*func)(const TInstant *))
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    result[i] = tcontseq_transform_tagg(TSEQUENCESET_SEQ_N(ss, i), func);
  return result;
}

/**
 * @brief Transform a temporal value for aggregation
 */
Temporal **
temporal_transform_tagg(const Temporal *temp, int *count,
  TInstant *(*func)(const TInstant *))
{
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *)func((TInstant *) temp);
      *count = 1;
      return result;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
      {
        *count = ((TSequence *) temp)->count;
        return (Temporal **) tdiscseq_transform_tagg((TSequence *) temp,
          func);
      }
      else
      {
        result = palloc(sizeof(Temporal *));
        result[0] = (Temporal *) tcontseq_transform_tagg((TSequence *) temp,
          func);
        *count = 1;
        return result;
      }
    }
    default: /* TSEQUENCESET */
    {
      *count = ((TSequenceSet *) temp)->count;
      return (Temporal **) tsequenceset_transform_tagg((TSequenceSet *) temp,
        func);
    }
  }
}

/**
 * @brief Transition function for aggregating temporal values that require a
 * transformation to each composing instant/sequence
 * @param[in] state Current state
 * @param[in] temp New temporal value
 * @param[in] func Aggregate function
 * @param[in] crossings True if turning points are added in the segments
 * @param[in] transform Transform function
 */
SkipList *
temporal_tagg_transform_transfn(SkipList *state, const Temporal *temp,
  datum_func2 func, bool crossings, TInstant *(*transform)(const TInstant *))
{
  assert(temp);
  int count;
  Temporal **temparr = temporal_transform_tagg(temp, &count, transform);
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) temparr, count, func, crossings);
  pfree_array((void **) temparr, count);
  return state;
}

/*****************************************************************************
 * Temporal count
 *****************************************************************************/

/**
 * @brief Transform a timestamp value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
timestamp_transform_tcount(TimestampTz t)
{
  TInstant **result = palloc(sizeof(TInstant *));
  result[0] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  return result;
}

/**
 * @brief Transform a timestamp set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
tstzset_transform_tcount(const Set *s)
{
  TInstant **result = palloc(sizeof(TInstant *) * s->count);
  for (int i = 0; i < s->count; i++)
  {
    TimestampTz t = DatumGetTimestampTz(SET_VAL_N(s, i));
    result[i] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  }
  return result;
}

/**
 * @brief Transform a a timestamptz span into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
tstzspan_transform_tcount(const Span *s)
{
  Datum datum_one = Int32GetDatum(1);
  TInstant *instants[2];
  TimestampTz t = s->lower;
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  TSequence *result;
  if (s->lower == s->upper)
  {
    result = tsequence_make(instants, 1, s->lower_inc, s->upper_inc, STEP,
      NORMALIZE_NO);
  }
  else
  {
    t = s->upper;
    instants[1] = tinstant_make(datum_one, T_TINT, t);
    result = tsequence_make(instants, 2, s->lower_inc, s->upper_inc, STEP,
      NORMALIZE_NO);
    pfree(instants[1]);
  }
  pfree(instants[0]);
  return result;
}

/**
 * @brief Transform a timestamptz span set value into a temporal integer value
 * for performing temporal count aggregation
 */
static TSequence **
tstzspanset_transform_tcount(const SpanSet *ss)
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    result[i] = tstzspan_transform_tcount(SPANSET_SP_N(ss, i));
  return result;
}

/*****************************************************************************/

/**
 * @brief Transform a temporal instant value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant *
tinstant_transform_tcount(const TInstant *inst)
{
  return tinstant_make(Int32GetDatum(1), T_TINT, inst->t);
}

/**
 * @brief Transform a temporal discrete sequence value into a temporal integer
 * value for performing temporal count aggregation
 */
static TInstant **
tdiscseq_transform_tcount(const TSequence *seq)
{
  TInstant **result = palloc(sizeof(TInstant *) * seq->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < seq->count; i++)
  {
    TimestampTz t = TSEQUENCE_INST_N(seq, i)->t;
    result[i] = tinstant_make(datum_one, T_TINT, t);
  }
  return result;
}

/**
 * @brief Transform a temporal sequence value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
tcontseq_transform_tcount(const TSequence *seq)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TimestampTz t = seq->period.lower;
  if (seq->count == 1)
  {
    TInstant *inst = tinstant_make(datum_one, T_TINT, t);
    return tinstant_to_tsequence_free(inst, STEP);
  }

  TInstant *instants[2];
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  t = seq->period.upper;
  instants[1] = tinstant_make(datum_one, T_TINT, t);
  result = tsequence_make(instants, 2, seq->period.lower_inc,
    seq->period.upper_inc, STEP, NORMALIZE_NO);
  pfree(instants[0]); pfree(instants[1]);
  return result;
}

/**
 * @brief Transform a temporal sequence set value into a temporal integer value
 * for performing temporal count aggregation
 */
static TSequence **
tsequenceset_transform_tcount(const TSequenceSet *ss)
{
  TSequence **result = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
    result[i] = tcontseq_transform_tcount(TSEQUENCESET_SEQ_N(ss, i));
  return result;
}

/**
 * @brief Transform a temporal value into a temporal integer value for
 * performing temporal count aggregation (dispatch function)
 */
Temporal **
temporal_transform_tcount(const Temporal *temp, int *count)
{
  Temporal **result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
    {
      result = palloc(sizeof(Temporal *));
      result[0] = (Temporal *) tinstant_transform_tcount((TInstant *) temp);
      *count = 1;
      return result;
    }
    case TSEQUENCE:
    {
      if (MEOS_FLAGS_DISCRETE_INTERP(temp->flags))
      {
        result = (Temporal **) tdiscseq_transform_tcount((TSequence *) temp);
        *count = ((TSequence *) temp)->count;
      }
      else
      {
        result = palloc(sizeof(Temporal *));
        result[0] = (Temporal *) tcontseq_transform_tcount((TSequence *) temp);
        *count = 1;
      }
      return result;
    }
    default: /* TSEQUENCESET */
    {
      *count = ((TSequenceSet *) temp)->count;
      return (Temporal **) tsequenceset_transform_tcount((TSequenceSet *) temp);
    }
  }

}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count aggregate of timestamps
 * @param[in,out] state Current aggregate state
 * @param[in] t Timestamp to aggregate
 * @csqlfn #Timestamptz_tcount_transfn()
 */
SkipList *
timestamptz_tcount_transfn(SkipList *state, TimestampTz t)
{
  TInstant **instants = timestamp_transform_tcount(t);
  if (! state)
    state = temporal_skiplist_make();
  else
  {
    if (! ensure_same_skiplist_subtype(state, TINSTANT))
      return NULL;
  }
  temporal_skiplist_splice(state, (void **) instants, 1, &datum_sum_int32,
    CROSSINGS_NO);
  pfree_array((void **) instants, 1);
  return state;
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count aggregate of timestamp sets
 * @param[in,out] state Current aggregate state
 * @param[in] s Timestamp set to aggregate
 * @csqlfn #Tstzset_tcount_transfn()
 */
SkipList *
tstzset_tcount_transfn(SkipList *state, const Set *s)
{
  /* Null set: return state */
  if (! s)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_set_isof_type(s, T_TSTZSET))
    return NULL;

  TInstant **instants = tstzset_transform_tcount(s);
  if (! state)
    state = temporal_skiplist_make();
  else
  {
    if (! ensure_same_skiplist_subtype(state, TINSTANT))
      return NULL;
  }
  temporal_skiplist_splice(state, (void **) instants, s->count, &datum_sum_int32,
    CROSSINGS_NO);
  pfree_array((void **) instants, s->count);
  return state;
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count aggregate of timestamptz spans
 * @param[in,out] state Current aggregate state
 * @param[in] s Timestamp span to aggregate
 * @csqlfn #Tstzspan_tcount_transfn()
 */
SkipList *
tstzspan_tcount_transfn(SkipList *state, const Span *s)
{
  /* Null span: return state */
  if (! s)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;

  TSequence *seq = tstzspan_transform_tcount(s);
  if (! state)
    state = temporal_skiplist_make();
  else
  {
    if (! ensure_same_skiplist_subtype(state, TSEQUENCE))
      return NULL;
  }
  temporal_skiplist_splice(state, (void **) &seq, 1, &datum_sum_int32,
    CROSSINGS_NO);
  pfree(seq);
  return state;
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count aggregate of timestamptz span
 * sets
 * @param[in,out] state Current aggregate state
 * @param[in] ss Timestamp span set to aggregate
 * @csqlfn #Tstzspanset_tcount_transfn()
 */
SkipList *
tstzspanset_tcount_transfn(SkipList *state, const SpanSet *ss)
{
  /* Null span set: return state */
  if (! ss)
    return state;
  /* Ensure the validity of the arguments */
  if (! ensure_spanset_isof_type(ss, T_TSTZSPANSET))
    return NULL;

  TSequence **sequences = tstzspanset_transform_tcount(ss);
  if (! state)
    state = temporal_skiplist_make();
  else
  {
    if (! ensure_same_skiplist_subtype(state, TSEQUENCE))
      return NULL;
  }
  for (int i = 0; i < ss->count; i++)
  {
    temporal_skiplist_splice(state, (void **) &sequences[i], 1,
      &datum_sum_int32, CROSSINGS_NO);
  }
  pfree_array((void **) sequences, ss->count);
  return state;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal count aggregation
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Temporal_tcount_transfn()
 */
SkipList *
temporal_tcount_transfn(SkipList *state, const Temporal *temp)
{
  /* Null temporal: return state */
  if (! temp)
    return state;
  int count;
  Temporal **temparr = temporal_transform_tcount(temp, &count);
  /* Null state: create a new state */
  if (! state)
    state = temporal_skiplist_make();
  temporal_skiplist_splice(state, (void **) temparr, count, &datum_sum_int32,
    false);
  pfree_array((void **) temparr, count);
  return state;
}

/*****************************************************************************
 * Temporal average
 *****************************************************************************/

/**
 * @brief Transform a temporal number into a temporal double2 value for
 * performing temporal average aggregation
 */
TInstant *
tnumberinst_transform_tavg(const TInstant *inst)
{
  double value = tnumberinst_double(inst);
  double2 dvalue;
  double2_set(value, 1, &dvalue);
  return tinstant_make(PointerGetDatum(&dvalue), T_TDOUBLE2, inst->t);
}

/**
 * @brief Final function for temporal average aggregation of temporal instant
 * values
 */
TSequence *
tinstant_tavg_finalfn(TInstant **instants, int count)
{
  TInstant **newinstants = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    const TInstant *inst = instants[i];
    double2 *value = (double2 *) DatumGetPointer(tinstant_value_p(inst));
    double tavg = value->a / value->b;
    newinstants[i] = tinstant_make(Float8GetDatum(tavg), T_TFLOAT, inst->t);
  }
  return tsequence_make_free(newinstants, count, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Final function for temporal average aggregation of temporal sequence
 * values
 */
TSequenceSet *
tsequence_tavg_finalfn(TSequence **sequences, int count)
{
  TSequence **newsequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    const TSequence *seq = sequences[i];
    TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      double2 *value2 = (double2 *) DatumGetPointer(tinstant_value_p(inst));
      double value = value2->a / value2->b;
      instants[j] = tinstant_make(Float8GetDatum(value), T_TFLOAT, inst->t);
    }
    newsequences[i] = tsequence_make_free(instants, seq->count,
      seq->period.lower_inc, seq->period.upper_inc,
      MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
  }
  return tsequenceset_make_free(newsequences, count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_agg
 * @brief Final function for temporal average aggregation
 * @param[in] state Current aggregate state
 * @csqlfn #Tnumber_tavg_finalfn()
 */
Temporal *
tnumber_tavg_finalfn(SkipList *state)
{
  if (! state || state->length == 0)
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
  skiplist_free(state);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal extent aggregate of temporal values
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Temporal_extent_transfn()
 */
Span *
temporal_extent_transfn(Span *state, const Temporal *temp)
{
  /* Can't do anything with null inputs */
  if (! state && ! temp)
    return NULL;
  /* Non-null state and null temporal, return the state */
  if (! temp)
    return state;
  /* Null state and non-null temporal, return the bbox of the temporal */
  if (! state)
  {
    Span *result = palloc0(sizeof(Span));
    temporal_set_tstzspan(temp, result);
    return result;
  }

  Span s;
  temporal_set_tstzspan(temp, &s);
  span_expand(&s, state);
  return state;
}

/**
 * @ingroup meos_temporal_agg
 * @brief Transition function for temporal extent aggregate of temporal numbers
 * @param[in,out] state Current aggregate state
 * @param[in] temp Temporal value to aggregate
 * @csqlfn #Tnumber_extent_transfn()
 */
TBox *
tnumber_extent_transfn(TBox *state, const Temporal *temp)
{
  /* Can't do anything with null inputs */
  if (! state && ! temp)
    return NULL;
  /* Null state and non-null temporal, return the bbox of the temporal */
  if (! state)
  {
    TBox *result = palloc0(sizeof(TBox));
    tnumber_set_tbox(temp, result);
    return result;
  }
  /* Non-null state and null temporal, return the state */
  if (! temp)
    return state;

  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnumber_tbox(temp, state))
    return NULL;

  /* Both state and temporal are not null */
  TBox b;
  tnumber_set_tbox(temp, &b);
  tbox_expand(&b, state);
  return state;
}

/*****************************************************************************
 * Append aggregate functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_agg
 * @brief Transition function for append temporal instant aggregate
 * @param[in,out] state Current aggregate state
 * @param[in] inst Temporal value to aggregate
 * @param[in] interp Interpolation
 * @param[in] maxdist Maximum distance
 * @param[in] maxt Maximum duration
 * @csqlfn #Temporal_app_tinst_transfn()
 */
Temporal *
temporal_app_tinst_transfn(Temporal *state, const TInstant *inst,
  interpType interp, double maxdist, const Interval *maxt)
{
  /* Null state: create a new temporal sequence with the instant */
  if (! state)
  {
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    /* Arbitrary initialization to 64 elements */
    Temporal *result = (Temporal *) tsequence_make_exp((TInstant **) &inst,
      1, 64, true, true, interp, NORMALIZE_NO);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
    return result;
  }

  return temporal_append_tinstant(state, inst, interp, maxdist, maxt, true);
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_agg
 * @brief Transition function for append temporal sequence aggregate
 * @param[in,out] state Current aggregate state
 * @param[in] seq Temporal value to aggregate
 * @csqlfn #Temporal_app_tseq_transfn()
 */
Temporal *
temporal_app_tseq_transfn(Temporal *state, const TSequence *seq)
{
  /* Null state: create a new temporal sequence with the sequence */
  if (! state)
  {
#if ! MEOS
    MemoryContext ctx = set_aggregation_context(fetch_fcinfo());
#endif /* ! MEOS */
    /* Arbitrary initialization to 64 elements */
    Temporal *result = (Temporal *) tsequenceset_make_exp((TSequence **) &seq,
      1, 64, NORMALIZE_NO);
#if ! MEOS
    unset_aggregation_context(ctx);
#endif /* ! MEOS */
    return result;
  }

  return temporal_append_tsequence(state, seq, true);
}

/*****************************************************************************/
