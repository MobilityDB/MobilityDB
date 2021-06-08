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
 * @file lifting.c
 * Generic functions for lifting functions and operators on temporal types.
 * These functions are used for lifting arithmetic operators (`+`, `-`, `*`,
 * `/`), Boolean operators (`and`, `or`, `not`), comparisons (`<`, `<=`, `>`,
 * `>=`), distance (`<->`), spatial relationships (`tcontains`), etc.
 *
 * The lifting of functions and operators must take into account the following
 * characteristic of the function to be lifted
 * 1. The number of arguments of the function
 *  - binary functions, such as spatial relationships functions (e.g.,
 *    `tintersects`).
 *  - ternary functions, such as spatial relationships functions that need
 *    an additional parameter (e.g., `tdwithin`).
 *  - quaternary functions which apply binary operators (e.g., `+` or `<`) to
 *    temporal number types that can be of different base type (that is,
 *    integer and float), and thus the third and fourth arguments are the
 *    Oids of the first two arguments.
 * 2. The type of the arguments
 *   - a temporal type and a base type. In this case the non-lifted function
 *     is applied to each instant of the temporal type.
 *   - two temporal types. In this case the operands must be synchronized
 *     and the function is applied to each pair of synchronized instants.
 * 3. Whether the function has instantaneous discontinuities at the crossings.
 *    Examples of such functions are temporal comparisons for temporal floats
 *    or temporal spatial relationships since the value of the result may
 *    change immediately before, at, or immediately after a crossing.
 * 4. Whether intermediate points between synchronized instants must be added
 *    to take into account the crossings or the turning points (or local
  *   minimum/maximum) of the function. For example, `tfloat + tfloat`
 *    only needs to synchronize the arguments while `tfloat * tfloat` requires
 *    in addition to add the turning point, which is the timestamp between the
 *    two consecutive synchronized instants in which the linear functions
 *    defined by the two segments are equal.
 *
 * Examples
 *   - `tfloatseq * base => tfunc_tsequence_base`
 *     applies the `*` operator to each instant and results in a `tfloatseq`.
 *   - `tfloatseq < base => tfunc_tsequence_base_discont`
 *     applies the `<` operator to each instant, if the sequence is equal
 *     to the base value in the middle of two consecutive instants add an
 *     instantaneous sequence at the crossing. The result is a `tfloatseqset`.
 *   - `tfloatseq + tfloatseq => sync_tfunc_tsequence_tsequence`
 *     synchronizes the sequences and applies the `+` operator to each instant.
 *   - `tfloatseq * tfloatseq => sync_tfunc_tsequence_tsequence`
 *     synchronizes the sequences possibly adding the turning points between
 *     two consecutive instants and applies the `*` operator to each instant.
 *     The result is a `tfloatseq`.
 *   - `tfloatseq < tfloatseq => sync_tfunc_tsequence_tsequence1`
 *     synchronizes the sequences, applies the `<` operator to each instant,
 *     and if there is a crossing in the middle of two consecutive pairs of
 *     instants add an instant sequence at the crossing. The result is a
 *     `tfloatseqset`.
 *
 * A struct named `LiftedFunctionInfo` is used to describe the above
 * characteristics of the function to be lifted. Such struct is filled by
 * the calling function and is passed through the dispatch functions.
 * To avoid code redundancy when coping with functions with 2, 3, and 4
 * arguments, variadic function pointers are used. The idea is sketched next.
 * @code
 * typedef Datum (*varfunc)    (Datum, ...);
 *
 * TInstant *
 * tfunc_tinstant(const TInstant *inst, Datum param, LiftedFunctionInfo lfinfo)
 * {
 *   Datum resvalue;
 *   if (lfinfo.numparam == 1)
 *     resvalue = (*lfinfo.func)(temporalinst_value(inst));
 *   else if (lfinfo.numparam == 2)
 *     resvalue = (*lfinfo.func)(temporalinst_value(inst), param);
 *   else
 *     elog(ERROR, "Number of function parameters not supported: %u",
 *       lfinfo.numparam);
 *   TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
 *   DATUM_FREE(resvalue, lfinfo.restypid);
 *   return result;
 * }
 *
 * // Definitions for TInstantSet, TSequence, and TSequenceSet
 * [...]
 *
 * // Dispatch function
 * Temporal *
 * tfunc_temporal(const Temporal *temp, Datum param,
 *   LiftedFunctionInfo lfinfo)
 * {
 *   // Dispatch depending on the temporal type
 *   [...]
 * }
 * @endcode
 * An example of use of the lifting functions is given next.
 * @code
 * // Transform the geometry to a geography
 * PGDLLEXPORT Datum
 * tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
 * {
 *   Temporal *temp = PG_GETARG_TEMPORAL(0);
 *   // We only need to fill these parameters for tfunc_temporal
 *   LiftedFunctionInfo lfinfo;
 *   lfinfo.func = (varfunc) &geom_to_geog;
 *   lfinfo.numparam = 1;
 *   lfinfo.restypid = type_oid(T_GEOGRAPHY);
 *   Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
 *   PG_FREE_IF_COPY(temp, 0);
 *   PG_RETURN_POINTER(result);
 * }
 * @endcode
 */

#include "lifting.h"

#include <assert.h>
#include <utils/timestamp.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "temporal_util.h"

/*****************************************************************************
 * Functions where the argument is a temporal type.
 * The function is applied to the composing instants.
 *****************************************************************************/

/**
 * Applies the function with the optional argument to the temporal value
 *
 * @param[in] inst Temporal value
 * @param[in] param Optional argument for the function
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant(const TInstant *inst, Datum param, LiftedFunctionInfo lfinfo)
{
  /* MobilityDB only supports lifting functions with 1 or 2 parameters */
  assert(lfinfo.numparam == 1 || lfinfo.numparam == 2);
  Datum resvalue;
  if (lfinfo.numparam == 1)
    resvalue = (*lfinfo.func)(tinstant_value(inst));
  else /* lfinfo.numparam == 2 */
    resvalue = (*lfinfo.func)(tinstant_value(inst), param);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Applies the function with the optional argument to the temporal value
 *
 * @param[in] ti Temporal value
 * @param[in] param Optional argument for the function
 * @param[in] lfinfo Information about the lifted function
 */
TInstantSet *
tfunc_tinstantset(const TInstantSet *ti, Datum param,
  LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tfunc_tinstant(inst, param, lfinfo);
  }
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
}

/**
 * Applies the function with the optional argument to the temporal value
 *
 * @param[in] seq Temporal value
 * @param[in] param Optional argument for the function
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence(const TSequence *seq, Datum param,
  LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tfunc_tinstant(inst, param, lfinfo);
  }
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
    base_type_continuous(lfinfo.restypid);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
}

/**
 * Applies the function with the optional argument to the temporal value
 *
 * @param[in] ts Temporal value
 * @param[in] param Optional argument for the function
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset(const TSequenceSet *ts, Datum param,
  LiftedFunctionInfo lfinfo)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tfunc_tsequence(seq, param, lfinfo);
  }
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/**
 * Applies the function with the optional argument to the temporal value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] param Optional argument for the function
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal(const Temporal *temp, Datum param,
  LiftedFunctionInfo lfinfo)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant((TInstant *) temp, param, lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset((TInstantSet *) temp, param, lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tfunc_tsequence((TSequence *) temp, param, lfinfo);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tfunc_tsequenceset((TSequenceSet *) temp, param, lfinfo);
  return result;
}

/*****************************************************************************
 * Functions where the arguments are a temporal type and a base type.
 * Notice that their base type may be different, for example, tfloat + int
 *****************************************************************************/

/*
 * Apply the variadic function with the optional argument to the base values
 * taking into account that their type may be different
 */
static Datum
tfunc_base_base(Datum value1, Datum value2, Oid basetypid1, Oid basetypid2,
  Datum param, LiftedFunctionInfo lfinfo)
{
  /* MobilityDB only supports lifting functions with 2, 3, or 4 parameters */
  assert(lfinfo.numparam >= 2 && lfinfo.numparam <= 4);
  if (lfinfo.numparam == 2)
    return lfinfo.invert ?
      (*lfinfo.func)(value2, value1) : (*lfinfo.func)(value1, value2);
  else if (lfinfo.numparam == 3)
    return lfinfo.invert ?
      (*lfinfo.func)(value2, value1, param) :
      (*lfinfo.func)(value1, value2, param);
  else /* lfinfo.numparam == 4 */
    return lfinfo.invert ?
      (*lfinfo.func)(value2, value1, basetypid2, basetypid1) :
      (*lfinfo.func)(value1, value2, basetypid1, basetypid2);
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value
 *
 * @param[in] inst Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value, Oid basetypid,
  Datum param, LiftedFunctionInfo lfinfo)
{
  Datum value1 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value, inst->basetypid, basetypid,
    param, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value
 *
 * @param[in] ti Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
TInstantSet *
tfunc_tinstantset_base(const TInstantSet *ti, Datum value, Oid basetypid,
  Datum param, LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tfunc_tinstant_base(inst, value, basetypid, param, lfinfo);
  }
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value when the function does not have instantaneous discontinuities
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static int
tfunc_tsequence_base1(TSequence **result, const TSequence *seq, Datum value,
  Oid basetypid, Datum param, LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tfunc_tinstant_base(inst, value, basetypid, param, lfinfo);
  }
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
    base_type_continuous(lfinfo.restypid);
  result[0] = tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
  return 1;
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value when the function has instantaneuous discontinuties
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @note The current version of the function supposes that the basetypid
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly, it is not necessary to pfree the values resulting from
 * the function func.
 */
static int
tfunc_tsequence_base_discont1(TSequence **result, const TSequence *seq,
  Datum value, Oid basetypid, Datum param, LiftedFunctionInfo lfinfo)
{
  const TInstant *start = tsequence_inst_n(seq, 0);
  Datum startvalue = tinstant_value(start);
  Datum startresult = tfunc_base_base(startvalue, value, seq->basetypid,
    basetypid, param, lfinfo);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  TInstant *instants[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    instants[0] = tinstant_make(startresult, start->t, lfinfo.restypid);
    result[0] = tinstant_to_tsequence(instants[0], STEP);
    pfree(instants[0]);
    return 1;
  }

  /* General case */
  int k = 0;
  bool lower_inc = seq->period.lower_inc;
  /* We create two temporal instants with arbitrary values to avoid
   * in the for loop creating and freeing the instants each time a
   * segment of the result is computed */
  instants[0] = tinstant_make(startresult, start->t, lfinfo.restypid);
  instants[1] = tinstant_make(startresult, start->t, lfinfo.restypid);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds between one and three sequences */
    const TInstant *end = tsequence_inst_n(seq, i);
    Datum endvalue = tinstant_value(end);
    Datum endresult = tfunc_base_base(endvalue, value, seq->basetypid,
      basetypid, param, lfinfo);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    Datum intvalue, intresult;
    bool lower_eq = false, upper_eq = false; /* make Codacy quiet */
    TimestampTz inttime;

    /* If the segment is constant compute the function at the start and
     * end instants */
    if (datum_eq(startvalue, endvalue, seq->basetypid))
    {
      tinstant_set(instants[0], startresult, start->t);
      tinstant_set(instants[1], startresult, end->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
      /* If either the start or the end value is equal to the value compute
       * the function at the start, at the middle, and at the end instants */
    else if (datum_eq2(startvalue, value, seq->basetypid, basetypid) ||
         datum_eq2(endvalue, value, seq->basetypid, basetypid))
    {
      /* Compute the function at the middle time between start and the end instants */
      inttime = start->t + ((end->t - start->t)/2);
      intvalue = tsequence_value_at_timestamp1(start, end, linear, inttime);
      intresult = tfunc_base_base(intvalue, value, seq->basetypid, basetypid,
        param, lfinfo);
      lower_eq = lower_inc && datum_eq(startresult, intresult, lfinfo.restypid);
      upper_eq = upper_inc && datum_eq(intresult, endresult, lfinfo.restypid);
      if (lower_inc && ! lower_eq)
      {
        tinstant_set(instants[0], startresult, start->t);
        result[k++] = tinstant_to_tsequence(instants[0], STEP);
      }
      tinstant_set(instants[0], intresult, start->t);
      tinstant_set(instants[1], intresult, end->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_eq, upper_eq, STEP, NORMALIZE_NO);
      if (upper_inc && ! upper_eq)
      {
        tinstant_set(instants[0], endresult, end->t);
        result[k++] = tinstant_to_tsequence(instants[0], STEP);
      }
      DATUM_FREE(intvalue, seq->basetypid);
      DATUM_FREE(intresult, lfinfo.restypid);
    }
    else
    {
      /* Determine whether there is a crossing and compute the value
       * at the crossing if there is one */
      bool hascross = tlinearseq_intersection_value(start, end, value,
        basetypid, &intvalue, &inttime);
      if (hascross)
      {
        intresult = tfunc_base_base(intvalue, value, seq->basetypid,
          basetypid, param, lfinfo);
        lower_eq = datum_eq(startresult, intresult, lfinfo.restypid);
        upper_eq = upper_inc && datum_eq(intresult, endresult, lfinfo.restypid);
      }
      /* If there is no crossing or the value at the crossing is equal to the
       * start value compute the function at the start and end instants */
      if (! hascross || (lower_eq && upper_eq))
      {
        /* Compute the function at the start and end instants */
        tinstant_set(instants[0], startresult, start->t);
        tinstant_set(instants[1], startresult, end->t);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, hascross ? upper_eq : false, STEP, NORMALIZE_NO);
        if (! hascross && upper_inc)
        {
          tinstant_set(instants[0], endresult, end->t);
          result[k++] = tinstant_to_tsequence(instants[0], STEP);
          DATUM_FREE(endresult, lfinfo.restypid);
        }
      }
      else
      {
        /* Since there is a crossing compute the function at the start instant,
         * at the crossing, and at the end instant */
        tinstant_set(instants[0], startresult, start->t);
        tinstant_set(instants[1], startresult, inttime);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, lower_eq, STEP, NORMALIZE_NO);
        /* Second sequence if any */
        if (! lower_eq && ! upper_eq)
        {
          tinstant_set(instants[0], intresult, inttime);
          result[k++] = tinstant_to_tsequence(instants[0], STEP);
        }
        /* Third sequence */
        tinstant_set(instants[0], endresult, inttime);
        tinstant_set(instants[1], endresult, end->t);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          upper_eq, upper_inc, STEP, NORMALIZE_NO);
        DATUM_FREE(intvalue, seq->basetypid);
        DATUM_FREE(intresult, lfinfo.restypid);
      }
    }
    start = end;
    startvalue = endvalue;
    startresult = endresult;
    lower_inc = true;
  }
  pfree(instants[0]); pfree(instants[1]);
  return k;
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value. Dispatch function depending on whether the function has
 * instantaneous discontinuities.
 */
Temporal *
tfunc_tsequence_base(const TSequence *seq, Datum value, Oid basetypid,
  Datum param, LiftedFunctionInfo lfinfo)
{
  int count;
  if (lfinfo.discont)
    count = seq->count * 3;
  else
    count = 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  if (lfinfo.discont)
  {
    int k = tfunc_tsequence_base_discont1(sequences, seq, value, basetypid,
      param, lfinfo);
    return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE);
  }
  else
  {
    /* We are sure that the result is a single sequence */
    tfunc_tsequence_base1(sequences, seq, value, basetypid, param, lfinfo);
    return (Temporal *) sequences[0];
  }
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value
 *
 * @param[in] ts Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */

TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value, Oid basetypid,
  Datum param, LiftedFunctionInfo lfinfo)
{
  int count;
  if (lfinfo.discont)
    count = ts->totalcount * 3;
  else
    count = ts->count;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += lfinfo.discont ?
      tfunc_tsequence_base_discont1(&sequences[k], seq, value, basetypid,
        param, lfinfo) :
      tfunc_tsequence_base1(&sequences[k], seq, value, basetypid, param,
        lfinfo);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Applies the function with the optional argument to the temporal value and
 * the base value (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] basetypid Type of the base value
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid basetypid,
  Datum param, LiftedFunctionInfo lfinfo)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant_base((TInstant *) temp,
      value, basetypid, param, lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset_base((TInstantSet *) temp,
      value, basetypid, param, lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tfunc_tsequence_base((TSequence *) temp,
      value, basetypid, param, lfinfo);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tfunc_tsequenceset_base((TSequenceSet *) temp,
      value, basetypid, param, lfinfo);
  return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass.
 *****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function sync_tfunc_temporal_temporal and thus the overlappint test is
 * repeated
 */
TInstant *
sync_tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  Datum param, LiftedFunctionInfo lfinfo)
{
  /* Test whether the two temporal values overlap on time */
  if (inst1->t != inst2->t)
    return NULL;

  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  Datum resvalue = tfunc_base_base(value1, value2, inst1->basetypid,
    inst2->basetypid, param, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst1->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti,inst Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
  Datum param, LiftedFunctionInfo lfinfo)
{
  Datum value1;
  if (!tinstantset_value_at_timestamp(ti, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, ti->basetypid,
    inst->basetypid, param, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,ti Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
  Datum param, LiftedFunctionInfo lfinfo)
{
  // Only works for commutative functions
  return sync_tfunc_tinstantset_tinstant(ti, inst, param, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,inst Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tsequence_tinstant(const TSequence *seq, const TInstant *inst,
  Datum param, LiftedFunctionInfo lfinfo)
{
  Datum value1;
  if (!tsequence_value_at_timestamp(seq, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, seq->basetypid,
    inst->basetypid, param, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,seq Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tinstant_tsequence(const TInstant *inst, const TSequence *seq,
  Datum param, LiftedFunctionInfo lfinfo)
{
  // Only works for commutative functions
  return sync_tfunc_tsequence_tinstant(seq, inst, param, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,inst Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
  Datum param, LiftedFunctionInfo lfinfo)
{
  Datum value1;
  if (!tsequenceset_value_at_timestamp(ts, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, ts->basetypid,
    inst->basetypid, param, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo.restypid);
  DATUM_FREE(resvalue, lfinfo.restypid);
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,ts Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
sync_tfunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
  Datum param, LiftedFunctionInfo lfinfo)
{
  // Only works for commutative functions
  return sync_tfunc_tsequenceset_tinstant(ts, inst, param, lfinfo);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti1,ti2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function sync_tfunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
TInstantSet *
sync_tfunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
  Datum param, LiftedFunctionInfo lfinfo)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period p1, p2;
  tinstantset_period(&p1, ti1);
  tinstantset_period(&p2, ti2);
  if (!overlaps_period_period_internal(&p1, &p2))
    return NULL;

  TInstant **instants = palloc(sizeof(TInstant *) *
    Min(ti1->count, ti2->count));
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tinstantset_inst_n(ti1, i);
  const TInstant *inst2 = tinstantset_inst_n(ti2, j);
  while (i < ti1->count && j < ti2->count)
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum value1 = tinstant_value(inst1);
      Datum value2 = tinstant_value(inst2);
      Datum resvalue = tfunc_base_base(value1, value2, ti1->basetypid,
        ti2->basetypid, param, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst1->t, lfinfo.restypid);
      DATUM_FREE(resvalue, lfinfo.restypid);
      inst1 = tinstantset_inst_n(ti1, ++i);
      inst2 = tinstantset_inst_n(ti2, ++j);
    }
    else if (cmp < 0)
      inst1 = tinstantset_inst_n(ti1, ++i);
    else
      inst2 = tinstantset_inst_n(ti2, ++j);
  }
  return tinstantset_make_free(instants, k, MERGE_NO);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,ti Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
sync_tfunc_tsequence_tinstantset(const TSequence *seq, const TInstantSet *ti,
  Datum param, LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp_internal(&seq->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamp(seq, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, seq->basetypid,
        ti->basetypid, param, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst->t, lfinfo.restypid);
      DATUM_FREE(value1, seq->basetypid); DATUM_FREE(resvalue, lfinfo.restypid);
    }
    if (seq->period.upper < inst->t)
      break;
  }
  return tinstantset_make_free(instants, k, MERGE_NO);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti,seq Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
sync_tfunc_tinstantset_tsequence(const TInstantSet *ti, const TSequence *seq,
  Datum param, LiftedFunctionInfo lfinfo)
{
  // Only works for commutative functions
  return sync_tfunc_tsequence_tinstantset(seq, ti, param, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,ti Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
sync_tfunc_tsequenceset_tinstantset(const TSequenceSet *ts, const TInstantSet *ti,
  Datum param, LiftedFunctionInfo lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int i = 0, j = 0, k = 0;
  while (i < ts->count && j < ti->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tinstantset_inst_n(ti, j);
    if (contains_period_timestamp_internal(&seq->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamp(ts, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, ts->basetypid,
        ti->basetypid, param, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst->t, lfinfo.restypid);
      DATUM_FREE(value1, ts->basetypid); DATUM_FREE(resvalue, lfinfo.restypid);
    }
    int cmp = timestamp_cmp_internal(seq->period.upper, inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return tinstantset_make_free(instants, k, MERGE_NO);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti,ts Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
sync_tfunc_tinstantset_tsequenceset(const TInstantSet *ti, const TSequenceSet *ts,
  Datum param, LiftedFunctionInfo lfinfo)
{
  // Only works for commutative functions
  return sync_tfunc_tsequenceset_tinstantset(ts, ti, param, lfinfo);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument. This function is applied for functions with
 * instantaneous discontinuities and thus the result is an array of sequences.
 * This function is applied when at least one temporal value has linear
 * interpolation.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq1,seq2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
sync_tfunc_tsequence_tsequence1(TSequence **result, const TSequence *seq1,
  const TSequence *seq2, Datum param, LiftedFunctionInfo lfinfo, Period *inter)
{
  /* This array keeps the new instants added for the synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) tsequence_inst_n(seq1, 0);
  TInstant *start2 = (TInstant *) tsequence_inst_n(seq2, 0);
  int i = 1, j = 1, k = 0, l = 0;
  /* Synchronize the start instant */
  if (start1->t < inter->lower)
  {
    start1 = tsequence_at_timestamp(seq1, inter->lower);
    tofree[l++] = start1;
    i = tsequence_find_timestamp(seq1, inter->lower) + 1;
  }
  else if (start2->t < inter->lower)
  {
    start2 = tsequence_at_timestamp(seq2, inter->lower);
    tofree[l++] = start2;
    j = tsequence_find_timestamp(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  /* Compute the function at the start instant */
  Datum startvalue1 = tinstant_value(start1);
  Datum startvalue2 = tinstant_value(start2);
  Datum startresult = tfunc_base_base(startvalue1, startvalue2,
    seq1->basetypid, seq2->basetypid, param, lfinfo);
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  TInstant *instants[2];
  while (i < seq1->count && j < seq2->count)
  {
    /* Each iteration of the loop adds between one and three sequences */
    TInstant *end1 = (TInstant *) tsequence_inst_n(seq1, i);
    TInstant *end2 = (TInstant *) tsequence_inst_n(seq2, j);
    /* Synchronize the instants */
    int cmp = timestamp_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsequence_at_timestamp1(start2, end2, linear2, end1->t);
      tofree[l++] = end2;
    }
    else
    {
      j++;
      end1 = tsequence_at_timestamp1(start1, end1, linear1, end2->t);
      tofree[l++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = tinstant_value(end1);
    Datum endvalue2 = tinstant_value(end2);
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, seq1->basetypid,
        seq2->basetypid, param, lfinfo);
    bool upper_inc = (end1->t == inter->upper) ? inter->upper_inc : false;
    Datum intvalue1, intvalue2, intresult;
    bool lower_eq = false, upper_eq = false; /* make Codacy quiet */
    TimestampTz inttime;

    /* If both segments are constant compute the function at the start and
     * end instants */
    if (datum_eq(startvalue1, endvalue1, start1->basetypid) &&
      datum_eq(startvalue2, endvalue2, start2->basetypid))
    {
      instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
      instants[1] = tinstant_make(startresult, end1->t, lfinfo.restypid);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
      pfree(instants[0]); pfree(instants[1]);
    }
    /* If either the start values are equal or the end values are equal and
     * both have linear interpolation compute the function at the start
     * instant, at an intermediate point, and at the end instant */
    else if (datum_eq2(startvalue1, startvalue2, start1->basetypid, start2->basetypid) ||
         (linear1 && linear2 &&
          datum_eq2(endvalue1, endvalue2, start1->basetypid, start2->basetypid)))
    {
      /* Compute the function at the middle time between start and the end instants */
      inttime = start1->t + ((end1->t - start1->t) / 2);
      intvalue1 = tsequence_value_at_timestamp1(start1, end1, linear1, inttime);
      intvalue2 = tsequence_value_at_timestamp1(start2, end2, linear2, inttime);
      intresult = tfunc_base_base(intvalue1, intvalue2, seq1->basetypid,
        seq2->basetypid, param, lfinfo);
      lower_eq = lower_inc && datum_eq(startresult, intresult, lfinfo.restypid);
      upper_eq = upper_inc && datum_eq(intresult, endresult, lfinfo.restypid);
      if (lower_inc && ! lower_eq)
      {
        instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
        result[k++] = tinstant_to_tsequence(instants[0], STEP);
        pfree(instants[0]);
      }
      instants[0] = tinstant_make(intresult, start1->t, lfinfo.restypid);
      instants[1] = tinstant_make(intresult, end1->t, lfinfo.restypid);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_eq, upper_eq, STEP, NORMALIZE_NO);
      pfree(instants[0]); pfree(instants[1]);
      if (upper_inc && ! upper_eq)
      {
        instants[0] = tinstant_make(endresult, end1->t, lfinfo.restypid);
        result[k++] = tinstant_to_tsequence(instants[0], STEP);;
        pfree(instants[0]);
      }
      DATUM_FREE(intvalue1, start1->basetypid);
      DATUM_FREE(intvalue2, start2->basetypid);
      DATUM_FREE(intresult, lfinfo.restypid);
    }
    else
    {
      /* Determine whether there is a crossing and compute the value
       * at the crossing if there is one */
      bool hascross = tsequence_intersection(start1, end1, linear1,
        start2, end2, linear2, &intvalue1, &intvalue2, &inttime);
      if (hascross)
      {
        intresult = tfunc_base_base(intvalue1, intvalue2, seq1->basetypid,
          seq2->basetypid, param, lfinfo);
        lower_eq = datum_eq(startresult, intresult, lfinfo.restypid);
        upper_eq = upper_inc && datum_eq(intresult, endresult, lfinfo.restypid);
      }
      /* If there is no crossing or the value at the crossing is equal to the
       * start value compute the function at the start and end instants */
      if (! hascross || (lower_eq && upper_eq))
      {
        instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
        instants[1] = tinstant_make(startresult, end1->t, lfinfo.restypid);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, hascross ? upper_eq : false, STEP, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
        if (! hascross && upper_inc)
        {
          instants[0] = tinstant_make(endresult, end1->t, lfinfo.restypid);
          result[k++] = tinstant_to_tsequence(instants[0], STEP);
          pfree(instants[0]);
        }
      }
      else
      {
        /* First sequence */
        instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
        instants[1] = tinstant_make(startresult, inttime, lfinfo.restypid);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, lower_eq, STEP, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
        /* Second sequence if any */
        if (! lower_eq && ! upper_eq)
        {
          instants[0] = tinstant_make(intresult, inttime, lfinfo.restypid);
          result[k++] = tinstant_to_tsequence(instants[0], STEP);
          pfree(instants[0]);
        }
        /* Third sequence */
        instants[0] = tinstant_make(endresult, inttime, lfinfo.restypid);
        instants[1] = tinstant_make(endresult, end1->t, lfinfo.restypid);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          upper_eq, upper_inc, STEP, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
        DATUM_FREE(intvalue1, start1->basetypid);
        DATUM_FREE(intvalue2, start2->basetypid);
        DATUM_FREE(intresult, lfinfo.restypid);
      }
    }
    DATUM_FREE(startresult, lfinfo.restypid);
    start1 = end1; start2 = end2;
    startvalue1 = endvalue1; startvalue2 = endvalue2;
    startresult = endresult;
    lower_inc = true;
  }
  pfree(inter);
  pfree_array((void **) tofree, l);
  return k;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument. This function is applied when the result is a single
 * sequence.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq1,seq2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
sync_tfunc_tsequence_tsequence2(TSequence **result, const TSequence *seq1,
  const TSequence *seq2, Datum param, LiftedFunctionInfo lfinfo, Period *inter)
{
  /*
   * General case
   * seq1 =  ...    *       *       *>
   * seq2 =    <*       *   *   * ...
   * result =  <X I X I X I * I X I X>
   * where X, I, and * are values computed, respectively at synchronization
   * points, intermediate points depending on the turning point function,
   * and common points
   */
  TInstant *inst1 = (TInstant *) tsequence_inst_n(seq1, 0);
  TInstant *inst2 = (TInstant *) tsequence_inst_n(seq2, 0);
  int i = 0, j = 0, k = 0, l = 0;
  if (inst1->t < inter->lower)
  {
    i = tsequence_find_timestamp(seq1, inter->lower) + 1;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
  }
  else if (inst2->t < inter->lower)
  {
    j = tsequence_find_timestamp(seq2, inter->lower) + 1;
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  TInstant *prev1, *prev2;
  Datum value;
  TimestampTz intertime;
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= inter->upper || inst2->t <= inter->upper))
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      inst2 = tsequence_at_timestamp(seq2, inst1->t);
      tofree[l++] = inst2;
    }
    else
    {
      j++;
      inst1 = tsequence_at_timestamp(seq1, inst2->t);
      tofree[l++] = inst1;
    }
    /* If not the first instant compute the function on the potential
       intermediate point before adding the new instants */
    if (lfinfo.tpfunc != NULL && k > 0 &&
      lfinfo.tpfunc(prev1, inst1, linear1, prev2, inst2, linear1, &intertime))
    {
      Datum inter1 = tsequence_value_at_timestamp1(prev1, inst1,
        linear1, intertime);
      Datum inter2 = tsequence_value_at_timestamp1(prev2, inst2,
        linear2, intertime);
      value = tfunc_base_base(inter1, inter2, seq1->basetypid,
        seq2->basetypid, param, lfinfo);
      instants[k++] = tinstant_make(value, intertime, lfinfo.restypid);
      DATUM_FREE(inter1, seq1->basetypid);
      DATUM_FREE(inter2, seq2->basetypid);
      DATUM_FREE(value, lfinfo.restypid);
    }
    Datum value1 = tinstant_value(inst1);
    Datum value2 = tinstant_value(inst2);
    value = tfunc_base_base(value1, value2, seq1->basetypid,
      seq2->basetypid, param, lfinfo);
    instants[k++] = tinstant_make(value, inst1->t, lfinfo.restypid);
    DATUM_FREE(value, lfinfo.restypid);
    if (i == seq1->count || j == seq2->count)
      break;
    prev1 = inst1; prev2 = inst2;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  /* We are sure that k != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and
     exclusive upper bound must be equal */
  if (!lfinfo.reslinear && !inter->upper_inc && k > 1)
  {
    tofree[l++] = instants[k - 1];
    value = tinstant_value(instants[k - 2]);
    instants[k - 1] = tinstant_make(value, instants[k - 1]->t, lfinfo.restypid);
  }

  pfree_array((void **) tofree, l); pfree(inter);

  result[0] = tsequence_make_free(instants, k, inter->lower_inc,
    inter->upper_inc, lfinfo.reslinear, NORMALIZE);
  return 1;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument. This function is applied when one sequence has linear
 * interpolation and the other step interpolation and thus the result is an
 * array of sequences.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq1,seq2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
sync_tfunc_tsequence_tsequence3(TSequence **result, const TSequence *seq1,
  const TSequence *seq2, Datum param, LiftedFunctionInfo lfinfo, Period *inter)
{
  /* This array keeps the new instants added for the synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) tsequence_inst_n(seq1, 0);
  TInstant *start2 = (TInstant *) tsequence_inst_n(seq2, 0);
  int i = 1, j = 1, k = 0, l = 0;
  /* Synchronize the start instant */
  if (start1->t < inter->lower)
  {
    start1 = tsequence_at_timestamp(seq1, inter->lower);
    tofree[l++] = start1;
    i = tsequence_find_timestamp(seq1, inter->lower) + 1;
  }
  else if (start2->t < inter->lower)
  {
    start2 = tsequence_at_timestamp(seq2, inter->lower);
    tofree[l++] = start2;
    j = tsequence_find_timestamp(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
  TInstant *instants[2];
  Datum startvalue1, startvalue2, startresult;
  /* Each iteration of the loop adds one sequence */
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2,
      seq1->basetypid, seq2->basetypid, param, lfinfo);
    /* Synchronize the instants */
    TInstant *end1 = (TInstant *) tsequence_inst_n(seq1, i);
    TInstant *end2 = (TInstant *) tsequence_inst_n(seq2, j);
    int cmp = timestamp_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsequence_at_timestamp1(start2, end2, linear2, end1->t);
      tofree[l++] = end2;
    }
    else
    {
      j++;
      end1 = tsequence_at_timestamp1(start1, end1, linear1, end2->t);
      tofree[l++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, seq1->basetypid,
      seq2->basetypid, param, lfinfo);
    instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
    instants[1] = tinstant_make(endresult, end1->t, lfinfo.restypid);
    result[k++] = tsequence_make((const TInstant **) instants, 2, lower_inc, false,
      lfinfo.reslinear, NORMALIZE_NO);
    pfree(instants[0]); pfree(instants[1]);
    DATUM_FREE(startresult, lfinfo.restypid);
    DATUM_FREE(endresult, lfinfo.restypid);
    start1 = end1; start2 = end2;
    lower_inc = true;
  }

  /* Add extra final point if any */
  if (inter->upper_inc)
  {
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2,
      seq1->basetypid, seq2->basetypid, param, lfinfo);
    instants[0] = tinstant_make(startresult, start1->t, lfinfo.restypid);
    result[k++] = tinstant_to_tsequence(instants[0], lfinfo.reslinear);
    pfree(instants[0]);
    DATUM_FREE(startresult, lfinfo.restypid);
  }
  pfree(inter);
  pfree_array((void **) tofree, l);
  return k;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument (dispatch function)
 *
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
sync_tfunc_tsequence_tsequence4(TSequence **result, const TSequence *seq1,
  const TSequence *seq2, Datum param, LiftedFunctionInfo lfinfo)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period *inter = intersection_period_period_internal(&seq1->period,
    &seq2->period);
  if (inter == NULL)
    return 0;

  /* If the two sequences intersect at an instant */
  if (inter->lower == inter->upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamp(seq1, inter->lower, &value1);
    tsequence_value_at_timestamp(seq2, inter->lower, &value2);
    Datum resvalue = tfunc_base_base(value1, value2, seq1->basetypid,
      seq2->basetypid, param, lfinfo);
    TInstant *inst = tinstant_make(resvalue, inter->lower, lfinfo.restypid);
    result[0] = tinstant_to_tsequence(inst, lfinfo.reslinear);
    DATUM_FREE(value1, seq1->basetypid);
    DATUM_FREE(value2, seq2->basetypid);
    DATUM_FREE(resvalue, lfinfo.restypid);
    pfree(inst); pfree(inter);
    return 1;
  }

  if (lfinfo.discont)
    return sync_tfunc_tsequence_tsequence1(result, seq1, seq2, param, lfinfo, inter);
  if (MOBDB_FLAGS_GET_LINEAR(seq1->flags) == MOBDB_FLAGS_GET_LINEAR(seq2->flags))
    return sync_tfunc_tsequence_tsequence2(result, seq1, seq2, param, lfinfo, inter);
  else
    return sync_tfunc_tsequence_tsequence3(result, seq1, seq2, param, lfinfo, inter);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 */
static Temporal *
sync_tfunc_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
  Datum param, LiftedFunctionInfo lfinfo)
{
  int count;
  if (lfinfo.discont)
    count = (seq1->count + seq2->count) * 3;
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(seq1->flags) == MOBDB_FLAGS_GET_LINEAR(seq2->flags))
      count = 1;
    else
      count = (seq1->count + seq2->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = sync_tfunc_tsequence_tsequence4(sequences, seq1, seq2, param, lfinfo);
  if (k == 0)
    return NULL;
  if (k == 1)
    return (Temporal *) sequences[0];
  else
    return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,seq Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
sync_tfunc_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
  Datum param, LiftedFunctionInfo lfinfo)
{
  int loc;
  tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
  /* We are sure that loc < ts->count due to the bounding period test made
   * in the dispatch function */
  int count;
  if (lfinfo.discont)
    count = (ts->totalcount + seq->count) * 3;
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(ts->flags) == MOBDB_FLAGS_GET_LINEAR(seq->flags))
      count = ts->count - loc;
    else
      count = (ts->totalcount + seq->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = 0;
  for (int i = loc; i < ts->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts, i);
    k += sync_tfunc_tsequence_tsequence4(&sequences[k], seq1, seq,
        param, lfinfo);
    int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
    if (cmp < 0 ||
      (cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  /* We need to normalize when discont is true */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,ts Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
sync_tfunc_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
  Datum param, LiftedFunctionInfo lfinfo)
{
  return sync_tfunc_tsequenceset_tsequence(ts, seq, param, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
sync_tfunc_tsequenceset_tsequenceset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, Datum param, LiftedFunctionInfo lfinfo)
{
  int count = ts1->totalcount + ts2->totalcount;
  if (lfinfo.discont)
    count *= 3;
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(ts1->flags) != MOBDB_FLAGS_GET_LINEAR(ts2->flags))
      count *= 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int i = 0, j = 0, k = 0;
  while (i < ts1->count && j < ts2->count)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, j);
    k += sync_tfunc_tsequence_tsequence4(&sequences[k], seq1, seq2,
        param, lfinfo);
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0)
    {
      if (!seq1->period.upper_inc && seq2->period.upper_inc)
        cmp = -1;
      else if (seq1->period.upper_inc && !seq2->period.upper_inc)
        cmp = 1;
    }
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  /* We need to normalize if the function has instantaneous discontinuities */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 * (dispatch function)
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] param Optional argument for ternary functions
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  Datum param, LiftedFunctionInfo lfinfo)
{
  /* Bounding box test */
  Period p1, p2;
  temporal_period(&p1, temp1);
  temporal_period(&p2, temp2);
  if (! overlaps_period_period_internal(&p1, &p2))
    return NULL;

  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);
  if (temp1->subtype == INSTANT)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) sync_tfunc_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2, param, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) sync_tfunc_tinstant_tinstantset(
        (TInstant *) temp1, (TInstantSet *) temp2, param, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) sync_tfunc_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2, param, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) sync_tfunc_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2, param, lfinfo);
  }
  else if (temp1->subtype == INSTANTSET)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) sync_tfunc_tinstantset_tinstant(
        (TInstantSet *) temp1, (TInstant *) temp2, param, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) sync_tfunc_tinstantset_tinstantset(
        (TInstantSet *) temp1, (TInstantSet *) temp2, param, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) sync_tfunc_tinstantset_tsequence(
        (TInstantSet *) temp1, (TSequence *) temp2, param, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) sync_tfunc_tinstantset_tsequenceset(
        (TInstantSet *) temp1, (TSequenceSet *) temp2, param, lfinfo);
  }
  else if (temp1->subtype == SEQUENCE)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) sync_tfunc_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2, param, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) sync_tfunc_tsequence_tinstantset(
        (TSequence *) temp1, (TInstantSet *) temp2, param, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) sync_tfunc_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2, param, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) sync_tfunc_tsequence_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, param, lfinfo);
  }
  else /* temp1->subtype == SEQUENCESET */
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) sync_tfunc_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2, param, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) sync_tfunc_tsequenceset_tinstantset(
        (TSequenceSet *) temp1, (TInstantSet *) temp2, param, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) sync_tfunc_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, param, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) sync_tfunc_tsequenceset_tsequenceset(
          (TSequenceSet *) temp1, (TSequenceSet *) temp2, param, lfinfo);
  }
  return result;
}

/*****************************************************************************/
