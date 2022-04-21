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
 * @file lifting.c
 * @brief Generic functions for lifting functions and operators on temporal
 * types.
 *
 * These functions are used for lifting arithmetic operators (`+`, `-`, `*`,
 * `/`), Boolean operators (`and`, `or`, `not`), comparisons (`<`, `<=`, `>`,
 * `>=`), distance (`<->`), spatial relationships (`tcontains`), etc.
 *
 * The lifting of functions and operators must take into account the following
 * characteristic of the function to be lifted
 * 1. The number of arguments of the function
 *  - unary functions, such as `degrees` for temporal floats or `round`
 *    for temporal points.
 *  - binary functions and operators, such as arithmetic operators and comparisons (e.g.,
 *    `+` or `<`) or spatial relationships functions (e.g.,`tintersects`).
 * 2. The type of the arguments for binary functions
 *   - a temporal type and a base type. In this case the non-lifted function
 *     is applied to each instant of the temporal type.
 *   - two temporal types. In this case the operands must be synchronized
 *     and the function is applied to each pair of synchronized instants.
 * 4. Whether the type of the arguments may vary. For example, temporal
 *    numbers can be of different base type (that is, integer and float).
 *    Therefore, the Oids of the arguments must be taken into account when
 *    computing binary operators (e.g., `+` or `<`) for temporal numbers.
 * 5. The number of optional parameters of the function
 *  - no arguments, such as most spatial relationships functions (e.g.,
 *    `tintersects`).
 *  - one argument, such as spatial relationships functions that need
 *    an additional parameter (e.g., `tdwithin`).
 *  - two arguments, e.g., when assembling a temporal point from two temporal
 *    floats, the SRID and a boolean flag stating whether the resulting
 *    temporal point is geometric or geographic are needed.
 * 6. Whether the function has instantaneous discontinuities at the crossings.
 *    Examples of such functions are temporal comparisons for temporal floats
 *    or temporal spatial relationships since the value of the result may
 *    change immediately before, at, or immediately after a crossing.
 * 7. Whether intermediate points between synchronized instants must be added
 *    to take into account the crossings or the turning points (or local
 *    minimum/maximum) of the function. For example, `tfloat + tfloat`
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
 *   - `tfloatseq + tfloatseq => tfunc_tsequence_tsequence`
 *     synchronizes the sequences and applies the `+` operator to each instant.
 *   - `tfloatseq * tfloatseq => tfunc_tsequence_tsequence`
 *     synchronizes the sequences possibly adding the turning points between
 *     two consecutive instants and applies the `*` operator to each instant.
 *     The result is a `tfloatseq`.
 *   - `tfloatseq < tfloatseq => tfunc_tsequence_tsequence_discont`
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
 * tfunc_tinstant(const TInstant *inst, LiftedFunctionInfo *lfinfo)
 * {
 *   Datum resvalue;
 *   if (lfinfo->numparam == 1)
 *     resvalue = (*lfinfo->func)(temporalinst_value(inst));
 *   else if (lfinfo->numparam == 2)
 *     resvalue = (*lfinfo->func)(temporalinst_value(inst), lfinfo->param[0]);
 *     resvalue = (*lfinfo->func)(temporalinst_value(inst), lfinfo->param[0]);
 *   else
 *     elog(ERROR, "Number of function parameters not supported: %u",
 *       lfinfo->numparam);
 *   TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
 *   DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
 *   return result;
 * }
 *
 * // Definitions for TInstantSet, TSequence, and TSequenceSet
 * [...]
 *
 * // Dispatch function
 * Temporal *
 * tfunc_temporal(const Temporal *temp, LiftedFunctionInfo *lfinfo)
 * {
 *   // Dispatch depending on the temporal type
 *   [...]
 * }
 * @endcode
 * An example of use of the lifting functions is given next.
 * @code
 * // Transform the geometry to a geography
 * PGDLLEXPORT Datum
 * tgeompoint_tgeogpoint(PG_FUNCTION_ARGS)
 * {
 *   Temporal *temp = PG_GETARG_TEMPORAL_P(0);
 *   // We only need to fill these parameters for tfunc_temporal
 *   LiftedFunctionInfo *lfinfo;
 *   memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
 *   lfinfo->func = (varfunc) &geom_to_geog;
 *   lfinfo->numparam = 1;
 *   lfinfo->restype = T_TGEOGPOINT;
 *   lfinfo->tpfunc_base = NULL;
 *   lfinfo->tpfunc = NULL;
 *   Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
 *   PG_FREE_IF_COPY(temp, 0);
 *   PG_RETURN_POINTER(result);
 * }
 * @endcode
 */

#include "general/lifting.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/period.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Functions where the argument is a temporal type.
 * The function is applied to the composing instants.
 *****************************************************************************/

/**
 * Apply the variadic function with the optional arguments to the base value
 */
static Datum
tfunc_base(Datum value, LiftedFunctionInfo *lfinfo)
{
  /* Lifted functions may have from 0 to MAX_PARAMS parameters */
  assert(lfinfo->numparam >= 0 && lfinfo->numparam <= MAX_PARAMS);
  if (lfinfo->numparam == 0)
    return (*lfinfo->func)(value);
  else if (lfinfo->numparam == 1)
    return (*lfinfo->func)(value, lfinfo->param[0]);
  else if (lfinfo->numparam == 2)
    return (*lfinfo->func)(value, lfinfo->param[0], lfinfo->param[1]);
  else /* lfinfo->numparam == 3 */
    return (*lfinfo->func)(value, lfinfo->param[0], lfinfo->param[1],
      lfinfo->param[2]);
}

/**
 * Apply the function with the optional arguments to the temporal value
 *
 * @param[in] inst Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant(const TInstant *inst, LiftedFunctionInfo *lfinfo)
{
  Datum resvalue = tfunc_base(tinstant_value(inst), lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Apply the function to the temporal value
 *
 * @param[in] ti Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TInstantSet *
tfunc_tinstantset(const TInstantSet *ti, LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tfunc_tinstant(inst, lfinfo);
  }
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
}

/**
 * Apply the function to the temporal value
 *
 * @param[in] seq Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tfunc_tinstant(inst, lfinfo);
  }
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
    temptype_continuous(lfinfo->restype);
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
}

/**
 * Apply the function to the temporal value
 *
 * @param[in] ts Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset(const TSequenceSet *ts, LiftedFunctionInfo *lfinfo)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    sequences[i] = tfunc_tsequence(seq, lfinfo);
  }
  return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/**
 * Apply the function to the temporal value (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal(const Temporal *temp, LiftedFunctionInfo *lfinfo)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant((TInstant *) temp, lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset((TInstantSet *) temp, lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tfunc_tsequence((TSequence *) temp, lfinfo);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tfunc_tsequenceset((TSequenceSet *) temp, lfinfo);
  return result;
}

/*****************************************************************************
 * Functions where the arguments are a temporal type and a base type.
 * Notice that their base type may be different, for example, tfloat + int
 *****************************************************************************/

/*
 * Apply the variadic function with the optional arguments to the base values
 * taking into account that their type may be different
 */
static Datum
tfunc_base_base(Datum value1, Datum value2, LiftedFunctionInfo *lfinfo)
{
  /* Lifted functions may have from 0 to MAX_PARAMS parameters */
  assert(lfinfo->numparam >= 0 && lfinfo->numparam <= MAX_PARAMS);
  if (lfinfo->numparam == 0)
  {
    if (lfinfo->args)
      return lfinfo->invert ?
        (*lfinfo->func)(value2, value1, lfinfo->argtype[1], lfinfo->argtype[0]) :
        (*lfinfo->func)(value1, value2, lfinfo->argtype[0], lfinfo->argtype[1]);
    else
      return lfinfo->invert ?
        (*lfinfo->func)(value2, value1) : (*lfinfo->func)(value1, value2);
  }
  else if (lfinfo->numparam == 1)
    return lfinfo->invert ?
      (*lfinfo->func)(value2, value1, lfinfo->param[0]) :
      (*lfinfo->func)(value1, value2, lfinfo->param[0]);
  else if (lfinfo->numparam == 2)
    return lfinfo->invert ?
      (*lfinfo->func)(value2, value1, lfinfo->param[0], lfinfo->param[1]) :
      (*lfinfo->func)(value1, value2, lfinfo->param[0], lfinfo->param[1]);
  else /* lfinfo->numparam == 3 */
    return lfinfo->invert ?
      (*lfinfo->func)(value2, value1, lfinfo->param[0], lfinfo->param[1], lfinfo->param[2]) :
      (*lfinfo->func)(value1, value2, lfinfo->param[0], lfinfo->param[1], lfinfo->param[2]);
}

/**
 * Apply the function to the temporal value and the base value
 *
 * @param[in] inst Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Apply the function to the temporal value and the base value
 *
 * @param[in] ti Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TInstantSet *
tfunc_tinstantset_base(const TInstantSet *ti, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    instants[i] = tfunc_tinstant_base(inst, value, lfinfo);
  }
  return tinstantset_make_free(instants, ti->count, MERGE_NO);
}

/**
 * Apply the function to the temporal value and the base value when no
 * turning points should be added and when the function does not have
 * instantaneous discontinuities
 *
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tsequence_base_scan(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    instants[i] = tfunc_tinstant_base(inst, value, lfinfo);
  }
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
    temptype_continuous(lfinfo->restype);
  result[0] = tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
  return 1;
}

/**
 * Apply the function to the temporal value and the base value when turning
 * points should be added
 *
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tsequence_base_turnpt(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  int k = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds between one and two instants */
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    instants[k++] = tfunc_tinstant_base(inst1, value, lfinfo);
    /* If not constant segment and linear compute the function on the potential
       intermediate point before adding the new instant */
    Datum intervalue;
    TimestampTz intertime;
    if (lfinfo->tpfunc_base != NULL && linear &&
      ! datum_eq(value1, value2, temptype_basetype(seq->temptype)) &&
      lfinfo->tpfunc_base(inst1, inst2, value, lfinfo->argtype[1],
        &intervalue, &intertime))
    {
      instants[k++] = tinstant_make(intervalue, intertime, lfinfo->restype);
      DATUM_FREE(intervalue, resbasetype);
    }
    inst1 = inst2; value1 = value2;
  }
  instants[k++] = tfunc_tinstant_base(inst1, value, lfinfo);
  result[0] = tsequence_make_free(instants, k, seq->period.lower_inc,
    seq->period.upper_inc, linear, NORMALIZE);
  return 1;
}

/**
 * Apply the function to the temporal value and the base value when the
 * function has instantaneuous discontinuties
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @note The current version of the function supposes that the basetype
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly, it is not necessary to pfree the values resulting from
 * the function func.
 */
static int
tfunc_tsequence_base_discont(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  const TInstant *start = tsequence_inst_n(seq, 0);
  Datum startvalue = tinstant_value(start);
  Datum startresult = tfunc_base_base(startvalue, value, lfinfo);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  TInstant *instants[2];

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    instants[0] = tinstant_make(startresult, start->t, lfinfo->restype);
    result[0] = tinstant_tsequence(instants[0], STEP);
    pfree(instants[0]);
    return 1;
  }

  /* General case */
  int k = 0;
  bool lower_inc = seq->period.lower_inc;
  /* We create two temporal instants with arbitrary values to avoid
   * in the for loop creating and freeing the instants each time a
   * segment of the result is computed */
  instants[0] = tinstant_make(startresult, start->t, lfinfo->restype);
  instants[1] = tinstant_make(startresult, start->t, lfinfo->restype);
  CachedType basetype = temptype_basetype(seq->temptype);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds between one and three sequences */
    const TInstant *end = tsequence_inst_n(seq, i);
    Datum endvalue = tinstant_value(end);
    Datum endresult = tfunc_base_base(endvalue, value, lfinfo);
    bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
    Datum intvalue, intresult;
    bool lower_eq = false, upper_eq = false; /* make Codacy quiet */
    TimestampTz inttime;

    /* If the segment is constant compute the function at the start and
     * end instants */
    if (datum_eq(startvalue, endvalue, basetype))
    {
      tinstant_set(instants[0], startresult, start->t);
      tinstant_set(instants[1], startresult, end->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, upper_inc, STEP, NORMALIZE_NO);
    }
    /* If either the start or the end value is equal to the value compute
     * the function at the start, at the middle, and at the end instants */
    else if (datum_eq2(startvalue, value, basetype, lfinfo->argtype[1]) ||
         datum_eq2(endvalue, value, basetype, lfinfo->argtype[1]))
    {
      /* Compute the function at the middle time between start and the end instants */
      inttime = start->t + ((end->t - start->t)/2);
      intvalue = tsegment_value_at_timestamp(start, end, linear, inttime);
      intresult = tfunc_base_base(intvalue, value, lfinfo);
      lower_eq = lower_inc && datum_eq(startresult, intresult, resbasetype);
      upper_eq = upper_inc && datum_eq(intresult, endresult, resbasetype);
      if (lower_inc && ! lower_eq)
      {
        tinstant_set(instants[0], startresult, start->t);
        result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
      }
      tinstant_set(instants[0], intresult, start->t);
      tinstant_set(instants[1], intresult, end->t);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_eq, upper_eq, lfinfo->reslinear, NORMALIZE_NO);
      if (upper_inc && ! upper_eq)
      {
        tinstant_set(instants[0], endresult, end->t);
        result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
      }
      DATUM_FREE(intvalue, basetype);
      DATUM_FREE(intresult, resbasetype);
    }
    else
    {
      /* Determine whether there is a crossing and compute the value
       * at the crossing if there is one */
      bool hascross = tlinearsegm_intersection_value(start, end, value,
        lfinfo->argtype[1], &intvalue, &inttime);
      if (hascross)
      {
        intresult = tfunc_base_base(intvalue, value, lfinfo);
        lower_eq = datum_eq(startresult, intresult, resbasetype);
        upper_eq = upper_inc && datum_eq(intresult, endresult, resbasetype);
      }
      /* If there is no crossing or the value at the crossing is equal to the
       * start value compute the function at the start and end instants */
      if (! hascross || (lower_eq && upper_eq))
      {
        /* Compute the function at the start and end instants */
        tinstant_set(instants[0], startresult, start->t);
        tinstant_set(instants[1], startresult, end->t);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, hascross ? upper_eq : false, lfinfo->reslinear, NORMALIZE_NO);
        if (! hascross && upper_inc)
        {
          tinstant_set(instants[0], endresult, end->t);
          result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
          DATUM_FREE(endresult, resbasetype);
        }
      }
      else
      {
        /* Since there is a crossing compute the function at the start instant,
         * at the crossing, and at the end instant */
        tinstant_set(instants[0], startresult, start->t);
        tinstant_set(instants[1], startresult, inttime);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, lower_eq, lfinfo->reslinear, NORMALIZE_NO);
        /* Second sequence if any */
        if (! lower_eq && ! upper_eq)
        {
          tinstant_set(instants[0], intresult, inttime);
          result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
        }
        /* Third sequence */
        tinstant_set(instants[0], endresult, inttime);
        tinstant_set(instants[1], endresult, end->t);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          upper_eq, upper_inc, lfinfo->reslinear, NORMALIZE_NO);
        DATUM_FREE(intvalue, basetype);
        DATUM_FREE(intresult, resbasetype);
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
 * Apply the function to the temporal value and the base value.
 * Dispatch function depending on whether the function has
 * instantaneous discontinuities.
 */
Temporal *
tfunc_tsequence_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = seq->count * 3;
  else
    count = 1;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  if (lfinfo->discont)
  {
    int k = tfunc_tsequence_base_discont(seq, value, lfinfo, sequences);
    return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE);
  }
  else
  {
    /* We are sure that the result is a single sequence */
    if (lfinfo->tpfunc_base != NULL)
      tfunc_tsequence_base_turnpt(seq, value, lfinfo, sequences);
    else
      tfunc_tsequence_base_scan(seq, value, lfinfo, sequences);
    return (Temporal *) sequences[0];
  }
}

/**
 * Apply the function to the temporal value and the base value
 *
 * @param[in] ts Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = ts->totalcount * 3;
  else
    count = ts->count;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (lfinfo->discont)
      k += tfunc_tsequence_base_discont(seq, value, lfinfo, &sequences[k]);
    else if (lfinfo->tpfunc_base != NULL)
      k += tfunc_tsequence_base_turnpt(seq, value, lfinfo, &sequences[k]);
    else
      k += tfunc_tsequence_base_scan(seq, value, lfinfo, &sequences[k]);
  }
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Apply the function to the temporal value and the base value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  Temporal *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = (Temporal *) tfunc_tinstant_base((TInstant *) temp, value, lfinfo);
  else if (temp->subtype == INSTANTSET)
    result = (Temporal *) tfunc_tinstantset_base((TInstantSet *) temp, value,
      lfinfo);
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *) tfunc_tsequence_base((TSequence *) temp, value,
      lfinfo);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *) tfunc_tsequenceset_base((TSequenceSet *) temp, value,
      lfinfo);
  return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass.
 *****************************************************************************/

/**
 * Invert the arguments of the lfinfo struct
 */
static void
lfinfo_invert(LiftedFunctionInfo *lfinfo)
{
  lfinfo->invert = ! lfinfo->invert;
  CachedType temp = lfinfo->argtype[0];
  lfinfo->argtype[0] = lfinfo->argtype[1];
  lfinfo->argtype[1] = temp;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function tfunc_temporal_temporal and thus the overlappint test is
 * repeated
 */
TInstant *
tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  LiftedFunctionInfo *lfinfo)
{
  /* The following is ensured by the period bound test in the dispatch function
   * or by the synchronizaton performed before this call */
  assert(inst1->t == inst2->t);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst1->t, lfinfo->restype);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tinstantset_value_at_timestamp(ti, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ti Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tinstantset_tinstant(ti, inst, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tsequence_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamp(seq, inst->t, &value1);
  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
  DATUM_FREE(value1, temptype_basetype(seq->temptype));
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tsequence(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tsequence_tinstant(seq, inst, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tsequenceset_value_at_timestamp(ts, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, inst->t, lfinfo->restype);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,ts Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tsequenceset_tinstant(ts, inst, lfinfo);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti1,ti2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function tfunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
TInstantSet *
tfunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) *
    Min(ti1->count, ti2->count));
  int i = 0, j = 0, k = 0;
  const TInstant *inst1 = tinstantset_inst_n(ti1, i);
  const TInstant *inst2 = tinstantset_inst_n(ti2, j);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < ti1->count && j < ti2->count)
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum value1 = tinstant_value(inst1);
      Datum value2 = tinstant_value(inst2);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst1->t, lfinfo->restype);
      DATUM_FREE(resvalue, resbasetype);
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
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
tfunc_tsequence_tinstantset(const TSequence *seq, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int k = 0;
  CachedType basetype = temptype_basetype(seq->temptype);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamp(seq, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst->t, lfinfo->restype);
      DATUM_FREE(value1, basetype); DATUM_FREE(resvalue, resbasetype);
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
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
tfunc_tinstantset_tsequence(const TInstantSet *ti, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tsequence_tinstantset(seq, ti, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,ti Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
tfunc_tsequenceset_tinstantset(const TSequenceSet *ts, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int i = 0, j = 0, k = 0;
  CachedType basetype = temptype_basetype(ts->temptype);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < ts->count && j < ti->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tinstantset_inst_n(ti, j);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamp(ts, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[k++] = tinstant_make(resvalue, inst->t, lfinfo->restype);
      DATUM_FREE(value1, basetype); DATUM_FREE(resvalue, resbasetype);
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
 * @param[in] lfinfo Information about the lifted function
 */
static TInstantSet *
tfunc_tinstantset_tsequenceset(const TInstantSet *ti, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tsequenceset_tinstantset(ts, ti, lfinfo);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function.
 * This function is applied when the result is a single sequence.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @pre The sequences are both linear or both stepwise
 */
static int
tfunc_tsequence_tsequence_lineareq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Period *inter, TSequence **result)
{
  /*
   * General case
   * seq1 =  ...    *       *       *>
   * seq2 =    <*       *   *   * ...
   * result =  <S T S T S T * T S T S>
   * where S, T, and * are values computed, respectively, at
   * Synchronization points, optional Turning points, and common points
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
  Datum value;
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= inter->upper || inst2->t <= inter->upper))
  {
    /* Synchronize the start instant */
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
       turning point before adding the new instants */
    TInstant *prev1, *prev2;
    Datum value1, value2;
    TimestampTz tptime;
    if (lfinfo->tpfunc != NULL && k > 0 &&
      lfinfo->tpfunc(prev1, inst1, prev2, inst2, &value, &tptime))
    {
      instants[k++] = tinstant_make(value, tptime, lfinfo->restype);
    }
    /* Compute the function on the synchronized instants */
    value1 = tinstant_value(inst1);
    value2 = tinstant_value(inst2);
    value = tfunc_base_base(value1, value2, lfinfo);
    instants[k++] = tinstant_make(value, inst1->t, lfinfo->restype);
    DATUM_FREE(value, resbasetype);
    if (i == seq1->count || j == seq2->count)
      break;
    prev1 = inst1; prev2 = inst2;
    inst1 = (TInstant *) tsequence_inst_n(seq1, i);
    inst2 = (TInstant *) tsequence_inst_n(seq2, j);
  }
  /* We are sure that k != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and
     exclusive upper bound must be equal */
  if (! lfinfo->reslinear && !inter->upper_inc && k > 1)
  {
    tofree[l++] = instants[k - 1];
    value = tinstant_value(instants[k - 2]);
    instants[k - 1] = tinstant_make(value, instants[k - 1]->t, lfinfo->restype);
    /* We cannot DATUM_FREE(value, lfinfo->restype); */
  }
  pfree_array((void **) tofree, l);
  result[0] = tsequence_make_free(instants, k, inter->lower_inc,
    inter->upper_inc, lfinfo->reslinear, NORMALIZE);
  return 1;
}

/**
 * Synchronizes the temporal values and applies to them the function.
 * This function is applied when one sequence has linear interpolation and
 * the other step interpolation and the function does not have instantaneous
 * discontinuities. The result is an array of sequences.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tsequence_tsequence_linearstep(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, Period *inter,
  TSequence **result)
{
  /* Array that keeps the new instants added for synchronization */
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
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    /* Synchronize the end instant */
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
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[l++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[l++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);
    instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
    instants[1] = tinstant_make(endresult, end1->t, lfinfo->restype);
    result[k++] = tsequence_make((const TInstant **) instants, 2, lower_inc, false,
      lfinfo->reslinear, NORMALIZE_NO);
    pfree(instants[0]); pfree(instants[1]);
    DATUM_FREE(startresult, resbasetype);
    DATUM_FREE(endresult, resbasetype);
    start1 = end1; start2 = end2;
    lower_inc = true;
  }
  /* Add a final instant if any */
  if (inter->upper_inc)
  {
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
    result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
    pfree(instants[0]);
    DATUM_FREE(startresult, resbasetype);
  }
  pfree_array((void **) tofree, l);
  return k;
}

/**
 * Synchronizes the temporal values and applies to them the function.
 * This function is applied for functions with instantaneous discontinuities
 * and thus the result is an array of sequences. This function is applied
 * when at least one temporal value has linear interpolation.
 *
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
tfunc_tsequence_tsequence_discont(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Period *inter, TSequence **result)
{
  /* Array that keeps the new instants added for the synchronization */
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
  Datum startvalue1, startvalue2, startresult;
  TInstant *instants[2];
  CachedType basetype1 = temptype_basetype(seq1->temptype);
  CachedType basetype2 = temptype_basetype(seq2->temptype);
  CachedType resbasetype = temptype_basetype(lfinfo->restype);
  /* Each iteration of the loop adds between one and three sequences */
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    /* Synchronize the end instants */
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
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[l++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[l++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);

    Datum intvalue1, intvalue2, intresult;
    TimestampTz inttime;
    bool lower_eq = false, upper_eq = false;
    /* If both segments are constant compute the function at the start and
     * end instants */
    if (datum_eq(startvalue1, endvalue1, basetype1) &&
      datum_eq(startvalue2, endvalue2, basetype2))
    {
      instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
      instants[1] = tinstant_make(startresult, end1->t, lfinfo->restype);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_inc, false, lfinfo->reslinear, NORMALIZE_NO);
      pfree(instants[0]); pfree(instants[1]);
    }
    /* If either the start values or the end values are equal and both have
     * linear interpolation compute the function at the start instant,
     * at an intermediate point, and at the end instant */
    else if (datum_eq2(startvalue1, startvalue2, basetype1, basetype2) ||
         (linear1 && linear2 &&
          datum_eq2(endvalue1, endvalue2, basetype1, basetype2)))
    {
      /* Compute the function at the middle time between start and the end instants */
      inttime = start1->t + ((end1->t - start1->t) / 2);
      intvalue1 = tsegment_value_at_timestamp(start1, end1, linear1, inttime);
      intvalue2 = tsegment_value_at_timestamp(start2, end2, linear2, inttime);
      intresult = tfunc_base_base(intvalue1, intvalue2, lfinfo);
      lower_eq = lower_inc && datum_eq(startresult, intresult, resbasetype);
      upper_eq = datum_eq(intresult, endresult, resbasetype);
      if (lower_inc && ! lower_eq)
      {
        instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
        result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
        pfree(instants[0]);
      }
      instants[0] = tinstant_make(intresult, start1->t, lfinfo->restype);
      instants[1] = tinstant_make(intresult, end1->t, lfinfo->restype);
      result[k++] = tsequence_make((const TInstant **) instants, 2,
        lower_eq, false, lfinfo->reslinear, NORMALIZE_NO);
      pfree(instants[0]); pfree(instants[1]);
      DATUM_FREE(intvalue1, basetype1);
      DATUM_FREE(intvalue2, basetype2);
      DATUM_FREE(intresult, resbasetype);
    }
    else
    {
      /* Determine whether there is a crossing and if there is one compute the
       * value at the crossing */
      bool hascross = tsegment_intersection(start1, end1, linear1,
        start2, end2, linear2, &intvalue1, &intvalue2, &inttime);
      if (hascross)
      {
        intresult = tfunc_base_base(intvalue1, intvalue2, lfinfo);
        lower_eq = datum_eq(startresult, intresult, resbasetype);
        upper_eq = datum_eq(intresult, endresult, resbasetype);
      }
      /* If there is no crossing or the value at the crossing is equal to the
       * start value compute the function at the start and end instants */
      if (! hascross || (lower_eq && upper_eq))
      {
        instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
        instants[1] = tinstant_make(startresult, end1->t, lfinfo->restype);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, false, lfinfo->reslinear, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
      }
      else
      {
        /* First sequence */
        instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
        instants[1] = tinstant_make(startresult, inttime, lfinfo->restype);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          lower_inc, lower_eq, lfinfo->reslinear, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
        /* Second sequence if any */
        if (! lower_eq && ! upper_eq)
        {
          instants[0] = tinstant_make(intresult, inttime, lfinfo->restype);
          result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
          pfree(instants[0]);
        }
        /* Third sequence */
        instants[0] = tinstant_make(endresult, inttime, lfinfo->restype);
        instants[1] = tinstant_make(endresult, end1->t, lfinfo->restype);
        result[k++] = tsequence_make((const TInstant **) instants, 2,
          upper_eq, false, lfinfo->reslinear, NORMALIZE_NO);
        pfree(instants[0]); pfree(instants[1]);
        DATUM_FREE(intvalue1, basetype1);
        DATUM_FREE(intvalue2, basetype2);
        DATUM_FREE(intresult, resbasetype);
      }
    }
    DATUM_FREE(startresult, resbasetype);
    DATUM_FREE(endresult, resbasetype);
    start1 = end1; start2 = end2;
    lower_inc = true;
  }
  /* Add a final instant if any */
  if (inter->upper_inc)
  {
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    instants[0] = tinstant_make(startresult, start1->t, lfinfo->restype);
    result[k++] = tinstant_tsequence(instants[0], lfinfo->reslinear);
    pfree(instants[0]);
    DATUM_FREE(startresult, resbasetype);
  }
  pfree_array((void **) tofree, l);
  return k;
}

/**
 * Synchronizes the temporal values and applies to them the function
 * (dispatch function)
 *
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
tfunc_tsequence_tsequence_dispatch(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, TSequence **result)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period inter;
  if (! inter_period_period(&seq1->period, &seq2->period, &inter))
    return 0;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamp(seq1, inter.lower, &value1);
    tsequence_value_at_timestamp(seq2, inter.lower, &value2);
    Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
    TInstant *inst = tinstant_make(resvalue, inter.lower, lfinfo->restype);
    result[0] = tinstant_tsequence(inst, lfinfo->reslinear);
    DATUM_FREE(value1, temptype_basetype(seq1->temptype));
    DATUM_FREE(value2, temptype_basetype(seq2->temptype));
    DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
    pfree(inst);
    return 1;
  }

  if (lfinfo->discont)
    return tfunc_tsequence_tsequence_discont(seq1, seq2, lfinfo, &inter, result);
  if (MOBDB_FLAGS_GET_LINEAR(seq1->flags) == MOBDB_FLAGS_GET_LINEAR(seq2->flags))
    return tfunc_tsequence_tsequence_lineareq(seq1, seq2, lfinfo, &inter, result);
  else
    return tfunc_tsequence_tsequence_linearstep(seq1, seq2, lfinfo, &inter, result);
}

/**
 * Synchronizes the temporal values and applies to them the function
 */
static Temporal *
tfunc_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = (seq1->count + seq2->count) * 3;
  else
  {
    if (MOBDB_FLAGS_GET_LINEAR(seq1->flags) == MOBDB_FLAGS_GET_LINEAR(seq2->flags))
      count = 1;
    else
      count = (seq1->count + seq2->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int k = tfunc_tsequence_tsequence_dispatch(seq1, seq2, lfinfo, sequences);
  /* The following is ensured by the period bound test in the dispatch
   * function */
  assert(k > 0);
  if (k == 1)
  {
    Temporal *result = (Temporal *) sequences[0];
    pfree(sequences);
    return result;
  }
  else
  {
    TSequenceSet *result = tsequenceset_make_free(sequences, k, NORMALIZE);
    if (result->count == 1)
    {
      Temporal *resultseq = (Temporal *) tsequenceset_tsequence(result);
      pfree(result);
      return resultseq;
    }
    else
      return (Temporal *) result;
  }
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
tfunc_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  int loc;
  tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
  /* We are sure that loc < ts->count due to the bounding period test made
   * in the dispatch function */
  int count;
  if (lfinfo->discont)
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
    k += tfunc_tsequence_tsequence_dispatch(seq1, seq, lfinfo, &sequences[k]);
    int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
    if (cmp < 0 ||
      (cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  /* We need to normalize when discont is true */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ts Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
tfunc_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return tfunc_tsequenceset_tsequence(ts, seq, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequenceSet *
tfunc_tsequenceset_tsequenceset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, LiftedFunctionInfo *lfinfo)
{
  int count = ts1->totalcount + ts2->totalcount;
  if (lfinfo->discont)
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
    k += tfunc_tsequence_tsequence_dispatch(seq1, seq2, lfinfo, &sequences[k]);
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0)
    {
      if (! seq1->period.upper_inc && seq2->period.upper_inc)
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
 * @ingroup libmeos_temporal_transf
 * Synchronizes the temporal values and applies to them the function
  *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  /* Bounding box test */
  Period p1, p2;
  temporal_period(temp1, &p1);
  temporal_period(temp2, &p2);
  if (! overlaps_period_period(&p1, &p2))
    return NULL;

  Temporal *result = NULL;
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);
  if (temp1->subtype == INSTANT)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) tfunc_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) tfunc_tinstant_tinstantset(
        (TInstant *) temp1, (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) tfunc_tinstant_tsequence(
        (TInstant *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) tfunc_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == INSTANTSET)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) tfunc_tinstantset_tinstant(
        (TInstantSet *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) tfunc_tinstantset_tinstantset(
        (TInstantSet *) temp1, (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) tfunc_tinstantset_tsequence(
        (TInstantSet *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) tfunc_tinstantset_tsequenceset(
        (TInstantSet *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == SEQUENCE)
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) tfunc_tsequence_tinstant(
        (TSequence *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) tfunc_tsequence_tinstantset(
        (TSequence *) temp1, (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) tfunc_tsequence_tsequence(
          (TSequence *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) tfunc_tsequence_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  else /* temp1->subtype == SEQUENCESET */
  {
    if (temp2->subtype == INSTANT)
      result = (Temporal *) tfunc_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = (Temporal *) tfunc_tsequenceset_tinstantset(
        (TSequenceSet *) temp1, (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = (Temporal *) tfunc_tsequenceset_tsequence(
          (TSequenceSet *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = (Temporal *) tfunc_tsequenceset_tsequenceset(
          (TSequenceSet *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a Boolean function
 * in a single pass using the ever semantics, that is, it stops when a true
 * value is found.
 *****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function efunc_temporal_temporal and thus the overlapping test is
 * repeated
 */
static int
efunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
  LiftedFunctionInfo *lfinfo)
{
  /* The following is ensured by the period bound test in the dispatch
   * function */
  assert(inst1->t == inst2->t);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tinstantset_value_at_timestamp(ti, inst->t, &value1))
    return -1;

  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ti Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tinstantset_tinstant(ti, inst, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequence_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamp(seq, inst->t, &value1);
  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tsequence(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tsequence_tinstant(seq, inst, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tsequenceset_value_at_timestamp(ts, inst->t, &value1))
    return -1;

  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] inst,ts Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tsequenceset_tinstant(ts, inst, lfinfo);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti1,ti2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function efunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
static int
efunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
  LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  const TInstant *inst1 = tinstantset_inst_n(ti1, i);
  const TInstant *inst2 = tinstantset_inst_n(ti2, j);
  while (i < ti1->count && j < ti2->count)
  {
    int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum value1 = tinstant_value(inst1);
      Datum value2 = tinstant_value(inst2);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
      i++;
    }
    else if (cmp < 0)
      inst1 = tinstantset_inst_n(ti1, ++i);
    else
      inst2 = tinstantset_inst_n(ti2, ++j);
  }
  return 0;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] seq,ti Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequence_tinstantset(const TSequence *seq, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamp(seq, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
    }
    if (seq->period.upper < inst->t)
      break;
  }
  return 0;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstantset_tsequence(const TInstantSet *ti, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tsequence_tinstantset(seq, ti, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ts,ti Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tinstantset(const TSequenceSet *ts, const TInstantSet *ti,
  LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  while (i < ts->count && j < ti->count)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    const TInstant *inst = tinstantset_inst_n(ti, j);
    if (contains_period_timestamp(&seq->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamp(ts, inst->t, &value1);
      Datum value2 = tinstant_value(inst);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
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
  return 0;
}

/**
 * Synchronizes the temporal values and applies to them the function with the
 * optional argument
 *
 * @param[in] ti,ts Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstantset_tsequenceset(const TInstantSet *ti, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tsequenceset_tinstantset(ts, ti, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function.
 * This function is applied for functions with instantaneous discontinuities
 * and thus the result is an array of sequences. This function is applied
 * when at least one temporal value has linear interpolation.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
efunc_tsequence_tsequence_discont(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, Period *inter)
{
  /* Array that keeps the new instants added for the synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) tsequence_inst_n(seq1, 0);
  TInstant *start2 = (TInstant *) tsequence_inst_n(seq2, 0);
  int i = 1, j = 1, l = 0;
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
  Datum startvalue1, startvalue2;
  CachedType basetype1 = temptype_basetype(seq1->temptype);
  CachedType basetype2 = temptype_basetype(seq2->temptype);
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    if (lower_inc && DatumGetBool(tfunc_base_base(startvalue1, startvalue2, lfinfo)))
    {
      pfree_array((void **) tofree, l);
      return 1;
    }
    /* Synchronize the end instants */
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
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[l++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[l++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    if (DatumGetBool(tfunc_base_base(endvalue1, endvalue2, lfinfo)))
    {
      pfree_array((void **) tofree, l);
      return 1;
    }

    Datum intvalue1, intvalue2;
    TimestampTz inttime;
    /* If either the start values or the end values are equal and both have
     * linear interpolation compute the function at the start instant,
     * at an intermediate point, and at the end instant */
    if (datum_eq2(startvalue1, startvalue2, basetype1, basetype2) ||
         (linear1 && linear2 &&
          datum_eq2(endvalue1, endvalue2, basetype1, basetype2)))
    {
      /* Compute the function at the middle time between start and the end instants */
      inttime = start1->t + ((end1->t - start1->t) / 2);
      intvalue1 = tsegment_value_at_timestamp(start1, end1, linear1, inttime);
      intvalue2 = tsegment_value_at_timestamp(start2, end2, linear2, inttime);
      if (DatumGetBool(tfunc_base_base(intvalue1, intvalue2, lfinfo)))
      {
        pfree_array((void **) tofree, l);
        return 1;
      }
    }
    else
    {
      /* Determine whether there is a crossing and if there is one compute the
       * value at the crossing */
      bool hascross = tsegment_intersection(start1, end1, linear1,
        start2, end2, linear2, &intvalue1, &intvalue2, &inttime);
      if (hascross && DatumGetBool(tfunc_base_base(intvalue1, intvalue2, lfinfo)))
      {
        pfree_array((void **) tofree, l);
        return 1;
      }
    }
    start1 = end1; start2 = end2;
    lower_inc = true;
  }
  /* Add a final instant if any */
  if (inter->upper_inc)
  {
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    if (DatumGetBool(tfunc_base_base(startvalue1, startvalue2, lfinfo)))
    {
      pfree_array((void **) tofree, l);
      return 1;
    }
  }
  pfree_array((void **) tofree, l);
  return 0;
}

/**
 * Synchronizes the temporal values and applies to them the function
 * (dispatch function)
 *
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
efunc_tsequence_tsequence(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Period *inter = intersection_period_period(&seq1->period,
    &seq2->period);
  if (inter == NULL)
    return -1;

  /* If the two sequences intersect at an instant */
  if (inter->lower == inter->upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamp(seq1, inter->lower, &value1);
    tsequence_value_at_timestamp(seq2, inter->lower, &value2);
    bool resvalue = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
    return resvalue ? 1 : 0;
  }
  /* Ever functions are always discontinuous */
  assert(lfinfo->discont);
  return efunc_tsequence_tsequence_discont(seq1, seq2, lfinfo, inter);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  int loc;
  tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
  /* We are sure that loc < ts->count due to the bounding period test made
   * in the dispatch function */
  for (int i = loc; i < ts->count; i++)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts, i);
    if (efunc_tsequence_tsequence(seq1, seq, lfinfo) == 1)
      return 1;
    int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
    if (cmp < 0 ||
      (cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  return 0;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ts Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert(lfinfo);
  return efunc_tsequenceset_tsequence(ts, seq, lfinfo);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tsequenceset(const TSequenceSet *ts1,
  const TSequenceSet *ts2, LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  while (i < ts1->count && j < ts2->count)
  {
    const TSequence *seq1 = tsequenceset_seq_n(ts1, i);
    const TSequence *seq2 = tsequenceset_seq_n(ts2, j);
    if (efunc_tsequence_tsequence(seq1, seq2, lfinfo) == 1)
      return 1;
    int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
    if (cmp == 0)
    {
      if (! seq1->period.upper_inc && seq2->period.upper_inc)
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
  return 0;
}

/*****************************************************************************/

/**
 * @brief Synchronizes the temporal values and applies to them the function
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
int
efunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  /* Bounding box test */
  Period p1, p2;
  temporal_period(temp1, &p1);
  temporal_period(temp2, &p2);
  if (! overlaps_period_period(&p1, &p2))
    return -1;

  int result;
  ensure_valid_tempsubtype(temp1->subtype);
  ensure_valid_tempsubtype(temp2->subtype);
  if (temp1->subtype == INSTANT)
  {
    if (temp2->subtype == INSTANT)
      result = efunc_tinstant_tinstant((TInstant *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = efunc_tinstant_tinstantset((TInstant *) temp1,
        (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = efunc_tinstant_tsequence((TInstant *) temp1,
        (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = efunc_tinstant_tsequenceset((TInstant *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == INSTANTSET)
  {
    if (temp2->subtype == INSTANT)
      result = efunc_tinstantset_tinstant((TInstantSet *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = efunc_tinstantset_tinstantset((TInstantSet *) temp1,
        (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = efunc_tinstantset_tsequence((TInstantSet *) temp1,
        (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = efunc_tinstantset_tsequenceset((TInstantSet *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == SEQUENCE)
  {
    if (temp2->subtype == INSTANT)
      result = efunc_tsequence_tinstant((TSequence *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = efunc_tsequence_tinstantset((TSequence *) temp1,
        (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = efunc_tsequence_tsequence((TSequence *) temp1,
        (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = efunc_tsequence_tsequenceset((TSequence *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  else /* temp1->subtype == SEQUENCESET */
  {
    if (temp2->subtype == INSTANT)
      result = efunc_tsequenceset_tinstant((TSequenceSet *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == INSTANTSET)
      result = efunc_tsequenceset_tinstantset((TSequenceSet *) temp1,
        (TInstantSet *) temp2, lfinfo);
    else if (temp2->subtype == SEQUENCE)
      result = efunc_tsequenceset_tsequence((TSequenceSet *) temp1,
        (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == SEQUENCESET */
      result = efunc_tsequenceset_tsequenceset((TSequenceSet *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  return result;
}

/*****************************************************************************/
