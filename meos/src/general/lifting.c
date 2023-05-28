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
 *    Therefore, the types of the arguments must be taken into account when
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
 *   - `tfloat_degrees => tfunc_temporal`
 *     applies the `degrees` function to each instant.
 *   - `tfloatseq + base => tfunc_tsequence_base`
 *     applies the `+` operator to each instant and results in a `tfloatseq`.
 *   - `tfloatseq < base => tfunc_tlinearseq_base_discfn`
 *     applies the `<` operator to each instant, if the sequence is equal
 *     to the base value in the middle of two consecutive instants add an
 *     instantaneous sequence at the crossing. The result is a `tfloatseqset`.
 *   - `tfloatseq + tfloatseq => tfunc_tcontseq_tcontseq`
 *     synchronizes the sequences and applies the `+` operator to each instant.
 *   - `tfloatseq * tfloatseq => tfunc_tcontseq_tcontseq`
 *     synchronizes the sequences possibly adding the turning points between
 *     two consecutive instants and applies the `*` operator to each instant.
 *     The result is a `tfloatseq`.
 *   - `tfloatseq < tfloatseq => tfunc_tcontseq_tcontseq_discfn`
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
 *   TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
 *   DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
 *   return result;
 * }
 *
 * // Definitions for TSequence, TSequence, and TSequenceSet
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

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/type_util.h"

/*****************************************************************************
 * Functions where the argument is a temporal type.
 * The function is applied to the composing instants.
 *****************************************************************************/

/**
 * @brief Apply the variadic function with the optional arguments to the base value
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
 * @brief Apply the function with the optional arguments to the temporal value
 * @param[in] inst Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TInstant *
tfunc_tinstant(const TInstant *inst, LiftedFunctionInfo *lfinfo)
{
  Datum resvalue = tfunc_base(tinstant_value(inst), lfinfo);
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Apply the function to the temporal value
 * @param[in] seq Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence(const TSequence *seq, LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    instants[i] = tfunc_tinstant(inst, lfinfo);
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Apply the function to the temporal value
 * @param[in] ss Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset(const TSequenceSet *ss, LiftedFunctionInfo *lfinfo)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = tfunc_tsequence(seq, lfinfo);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @brief Apply the function to the temporal value (dispatch function)
 * @param[in] temp Temporal value
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal(const Temporal *temp, LiftedFunctionInfo *lfinfo)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tfunc_tinstant((TInstant *) temp, lfinfo);
  else if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tfunc_tsequence((TSequence *) temp, lfinfo);
  else /* temp->subtype == TSEQUENCESET */
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
 * @brief Apply the function to the temporal value and the base value
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
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Apply the function to the temporal value and the base value
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TSequence *
tfunc_tsequence_base(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    instants[i] = tfunc_tinstant_base(inst, value, lfinfo);
  }
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    seq->period.upper_inc, MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
}

/**
 * @brief Apply the function to the temporal value and the base value when
 * turning points should be added
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tlinearseq_base_turnpt(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  int ninsts = 0;
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value(inst1);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  for (int i = 1; i < seq->count; i++)
  {
    /* Each iteration of the loop adds between one and two instants */
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value(inst2);
    instants[ninsts++] = tfunc_tinstant_base(inst1, value, lfinfo);
    /* If not constant segment and linear compute the function on the potential
       intermediate turning point before adding the new instant */
    Datum intervalue;
    TimestampTz intertime;
    if (lfinfo->tpfunc_base != NULL && interp == LINEAR &&
      ! datum_eq(value1, value2, temptype_basetype(seq->temptype)) &&
      lfinfo->tpfunc_base(inst1, inst2, value, lfinfo->argtype[1],
        &intervalue, &intertime))
    {
      instants[ninsts++] = tinstant_make(intervalue, lfinfo->restype, intertime);
      DATUM_FREE(intervalue, resbasetype);
    }
    inst1 = inst2;
    value1 = value2;
  }
  instants[ninsts++] = tfunc_tinstant_base(inst1, value, lfinfo);
  result[0] = tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, interp, NORMALIZE);
  return 1;
}

/**
 * @brief Apply the function to the temporal value and the base value when the
 * function has instantaneuous discontinuties
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @note The current version of the function supposes that the basetype
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly, it is not necessary to pfree the values resulting from
 * the function func.
 */
static int
tfunc_tlinearseq_base_discfn(const TSequence *seq, Datum value,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == LINEAR);
  const TInstant *start = TSEQUENCE_INST_N(seq, 0);
  Datum startvalue = tinstant_value(start);
  Datum startresult = tfunc_base_base(startvalue, value, lfinfo);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    instants[0] = tinstant_make(startresult, lfinfo->restype, start->t);
    result[0] = tinstant_to_tsequence(instants[0], STEP);
    pfree(instants[0]);
    return 1;
  }

  /* General case */
  meosType basetype = temptype_basetype(seq->temptype);
  meosType restype = lfinfo->restype;
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  bool lower_inc = seq->period.lower_inc;
  int ninsts = 0, nseqs = 0;
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *end = TSEQUENCE_INST_N(seq, i);
    Datum endvalue = tinstant_value(end);
    Datum endresult = tfunc_base_base(endvalue, value, lfinfo);
    Datum intvalue, intresult;
    bool lower_eq;
    TimestampTz inttime;

    /* If the segment is constant continue the current sequence */
    if (datum_eq(startvalue, endvalue, basetype))
    {
      instants[ninsts++] = tinstant_make(startresult, restype, start->t);
      if (i == seq->count - 1)
        instants[ninsts++] = tinstant_make(startresult, restype, end->t);
    }
    /* If either the start or the end value is equal to the value compute the
     * function at the middle time between the start and end instants */
    else if (datum_eq2(startvalue, value, basetype, lfinfo->argtype[1]) ||
         datum_eq2(endvalue, value, basetype, lfinfo->argtype[1]))
    {
      inttime = start->t + ((end->t - start->t) / 2);
      intvalue = tsegment_value_at_timestamp(start, end, LINEAR, inttime);
      intresult = tfunc_base_base(intvalue, value, lfinfo);
      lower_eq = datum_eq(startresult, intresult, resbasetype);
      if (lower_eq)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      else /* ! lower_eq => upper_eq */
      {
        /* Close the current sequence at the start */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        result[nseqs++] = tsequence_make((const TInstant **) instants,
          ninsts, lower_inc, true, STEP, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
        lower_inc = false;
        /* Start a new sequence */
        instants[ninsts++] = tinstant_make(intresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      DATUM_FREE(intvalue, basetype);
      DATUM_FREE(intresult, resbasetype);
    }
    else
    {
      /* Determine whether there is a crossing and compute the value at the
       * crossing if there is one */
      bool hascross = tlinearsegm_intersection_value(start, end, value,
        lfinfo->argtype[1], &intvalue, &inttime);
      if (! hascross)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start->t);
        if (i == seq->count - 1)
          instants[ninsts++] = tinstant_make(endresult, restype, end->t);
      }
      else /* hascross */
      {
        intresult = tfunc_base_base(intvalue, value, lfinfo);
        lower_eq = datum_eq(startresult, intresult, resbasetype);
        bool upper_eq = datum_eq(intresult, endresult, resbasetype);
        /* If the value at the crossing is equal to the start or to the end
         * value close the current sequence at the start or end instant.
         * Notice that lower_eq and upper_eq can be both true */
        if (lower_eq || upper_eq)
        {
          instants[ninsts++] = tinstant_make(startresult, restype, start->t);
          instants[ninsts++] = tinstant_make(startresult, restype, inttime);
          /* The upper_inc bound of the closing sequence is true if lower_eq,
           * false if upper_eq */
          result[nseqs++] = tsequence_make((const TInstant **) instants,
            ninsts, lower_inc, lower_eq, STEP, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* The lower_inc of the new sequence is false if lower_eq is true */
          lower_inc = ! lower_eq;
          /* Start a new sequence */
          instants[ninsts++] = tinstant_make(endresult, restype, inttime);
          if (i == seq->count - 1)
            instants[ninsts++] = tinstant_make(endresult, restype, end->t);
        }
        else /* ! lower_eq && ! upper_eq */
        {
          /* The crossing is at the middle: close the current sequence */
          instants[ninsts++] = tinstant_make(startresult, restype, start->t);
          instants[ninsts++] = tinstant_make(startresult, restype, inttime);
          result[nseqs++] = tsequence_make((const TInstant **) instants,
            ninsts, lower_inc, false, STEP, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* Add a singleton sequence with the crossing */
          instants[0] = tinstant_make(intresult, restype, inttime);
          result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, STEP, NORMALIZE);
          pfree(instants[0]);
          /* Start a new sequence from the crossing to the end of the segment */
          lower_inc = false;
          instants[ninsts++] = tinstant_make(endresult, restype, inttime);
          if (i == seq->count - 1)
            instants[ninsts++] = tinstant_make(endresult, restype, end->t);
        }
        DATUM_FREE(intvalue, basetype);
        DATUM_FREE(intresult, resbasetype);
      }
    }
    start = end;
    startvalue = endvalue;
    startresult = endresult;
  }
  if (ninsts > 0)
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      (ninsts == 1) ? true : lower_inc,
      (ninsts == 1) ? true : seq->period.upper_inc, STEP, NORMALIZE);
  pfree(instants);
  return nseqs;
}

/**
 * @brief Apply the function to the temporal value and the base value.
 *
 * Dispatch function depending on whether the function has
 * instantaneous discontinuities.
 */
Temporal *
tfunc_tlinearseq_base(const TSequence *seq, Datum value,
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
    int nseqs = tfunc_tlinearseq_base_discfn(seq, value, lfinfo, sequences);
    /* We are sure that nseqs > 0 */
    return (Temporal *) tsequenceset_make_free(sequences, nseqs, NORMALIZE);
  }
  else
  {
    /* We are sure that the result is a single sequence */
    if (lfinfo->tpfunc_base != NULL)
      tfunc_tlinearseq_base_turnpt(seq, value, lfinfo, sequences);
    else
      sequences[0] = tfunc_tsequence_base(seq, value, lfinfo);
    return (Temporal *) sequences[0];
  }
}

/**
 * @brief Apply the function to the temporal value and the base value
 * @param[in] ss Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ss, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = ss->totalcount * 3;
  else
    count = ss->count;
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (lfinfo->discont)
      nseqs += tfunc_tlinearseq_base_discfn(seq, value, lfinfo,
        &sequences[nseqs]);
    else if (lfinfo->tpfunc_base != NULL)
      nseqs += tfunc_tlinearseq_base_turnpt(seq, value, lfinfo,
        &sequences[nseqs]);
    else
      sequences[nseqs++] = tfunc_tsequence_base(seq, value, lfinfo);
  }
  /* We are sure that nseqs > 0 */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Apply the function to the temporal value and the base value
 * (dispatch function)
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value,
  LiftedFunctionInfo *lfinfo)
{
  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (Temporal *) tfunc_tinstant_base((TInstant *) temp, value, lfinfo);
  else if (temp->subtype == TSEQUENCE)
    result = ! MEOS_FLAGS_GET_LINEAR(temp->flags) ?
      (Temporal *) tfunc_tsequence_base((TSequence *) temp, value, lfinfo) :
      (Temporal *) tfunc_tlinearseq_base((TSequence *) temp, value, lfinfo);
  else /* temp->subtype == TSEQUENCESET */
    result = (Temporal *) tfunc_tsequenceset_base((TSequenceSet *) temp, value,
      lfinfo);
  return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass.
 *****************************************************************************/

/**
 * @brief Invert the arguments of the lfinfo struct
 */
static void
lfinfo_invert_args(LiftedFunctionInfo *lfinfo)
{
  lfinfo->invert = ! lfinfo->invert;
  meosType temp = lfinfo->argtype[0];
  lfinfo->argtype[0] = lfinfo->argtype[1];
  lfinfo->argtype[1] = temp;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
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
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst1->t);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] is,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tdiscseq_tinstant(const TSequence *is, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tdiscseq_value_at_timestamp(is, inst->t, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] inst,is Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tdiscseq(const TInstant *inst, const TSequence *is,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tdiscseq_tinstant(is, inst, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tcontseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamp(seq, inst->t, true, &value1);
  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
  DATUM_FREE(value1, temptype_basetype(seq->temptype));
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tcontseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tcontseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] ss,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tsequenceset_tinstant(const TSequenceSet *ss, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tsequenceset_value_at_timestamp(ss, inst->t, true, &value1))
    return NULL;

  Datum value2 = tinstant_value(inst);
  Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
  TInstant *result = tinstant_make(resvalue, lfinfo->restype, inst->t);
  DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
  return result;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] inst,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TInstant *
tfunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tinstant(ss, inst, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * with optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function tfunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
TSequence *
tfunc_tdiscseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) *
    Min(seq1->count, seq2->count));
  int i = 0, j = 0, ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum value1 = tinstant_value(inst1);
      Datum value2 = tinstant_value(inst2);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[ninsts++] = tinstant_make(resvalue, lfinfo->restype, inst1->t);
      DATUM_FREE(resvalue, resbasetype);
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
    }
    else if (cmp < 0)
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
    else
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
  }
  if (ninsts == 0)
  {
    pfree(instants);
    return NULL;
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE_NO);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tcontseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq2->count);
  int ninsts = 0;
  meosType basetype = temptype_basetype(seq1->temptype);
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  TimestampTz upper1 = DatumGetTimestampTz(seq1->period.upper);
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq2, i);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamp(seq1, inst->t, true, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[ninsts++] = tinstant_make(resvalue, lfinfo->restype, inst->t);
      DATUM_FREE(value1, basetype); DATUM_FREE(resvalue, resbasetype);
    }
    if (upper1 < inst->t)
      break;
  }
  if (ninsts == 0)
  {
    pfree(instants);
    return NULL;
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE, NORMALIZE_NO);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tdiscseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tcontseq_tdiscseq(seq2, seq1, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tsequenceset_tdiscseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0, j = 0, ninsts = 0;
  meosType basetype = temptype_basetype(ss->temptype);
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < ss->count && j < seq->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, j);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamp(ss, inst->t, true, &value1);
      Datum value2 = tinstant_value(inst);
      Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
      instants[ninsts++] = tinstant_make(resvalue, lfinfo->restype, inst->t);
      DATUM_FREE(value1, basetype); DATUM_FREE(resvalue, resbasetype);
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      inst->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  if (ninsts == 0)
  {
    pfree(instants);
    return NULL;
  }
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE, NORMALIZE_NO);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] is,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static TSequence *
tfunc_tdiscseq_tsequenceset(const TSequence *is, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tdiscseq(ss, is, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Synchronizes the temporal values and apply to them the function.
 *
 * This function is applied when the result is a single sequence and thus
 * it is used when
 * - the function to lift has NOT instantaneous discontinuities
 * - the temporal values have equal interpolation.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 * @pre The sequences are both linear or both step
 */
static int
tfunc_tcontseq_tcontseq_single(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  /*
   * General case
   * seq1 =  ...    *       *       *>
   * seq2 =    <*       *   *   * ...
   * result =  <S T S T S T * T S T S>
   * where S, T, and * are values computed, respectively, at the
   * Synchronization points, optional Turning points, and common points
   */
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  TInstant *prev1 = NULL, *prev2 = NULL; /* make compiler quiet */
  TimestampTz lower = DatumGetTimestampTz(inter->lower);
  TimestampTz upper = DatumGetTimestampTz(inter->upper);
  int i = 0, j = 0, ninsts = 0, nfree = 0;
  if (inst1->t < lower)
  {
    i = tcontseq_find_timestamp(seq1, inter->lower) + 1;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
  }
  else if (inst2->t < lower)
  {
    j = tcontseq_find_timestamp(seq2, inter->lower) + 1;
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  int count = (seq1->count - i + seq2->count - j) * 2;
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  Datum value;
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= upper || inst2->t <= upper))
  {
    /* Synchronize the start instant */
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      inst2 = tcontseq_at_timestamp(seq2, inst1->t);
      tofree[nfree++] = inst2;
    }
    else
    {
      j++;
      inst1 = tcontseq_at_timestamp(seq1, inst2->t);
      tofree[nfree++] = inst1;
    }
    /* If not the first instant compute the function on the potential
       turning point before adding the new instants */
    if (lfinfo->tpfunc != NULL && ninsts > 0)
    {
      TimestampTz tptime;
      bool found = lfinfo->tpfunc(prev1, inst1, prev2, inst2, &value, &tptime);
      /* Avoid adding a turning point at the same timestamp added next */
      if (found && tptime != prev1->t)
        instants[ninsts++] = tinstant_make(value, lfinfo->restype, tptime);
    }
    /* Compute the function on the synchronized instants */
    Datum value1 = tinstant_value(inst1);
    Datum value2 = tinstant_value(inst2);
    value = tfunc_base_base(value1, value2, lfinfo);
    instants[ninsts++] = tinstant_make(value, lfinfo->restype, inst1->t);
    DATUM_FREE(value, resbasetype);
    if (i == seq1->count || j == seq2->count)
    {
      break;
    }
    prev1 = inst1;
    prev2 = inst2;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  /* We are sure that ninsts != 0 due to the period intersection test above */
  /* The last two values of sequences with step interpolation and
     exclusive upper bound must be equal */
  if (! lfinfo->reslinear && ! inter->upper_inc && ninsts > 1)
  {
    tofree[nfree++] = instants[ninsts - 1];
    value = tinstant_value(instants[ninsts - 2]);
    instants[ninsts - 1] = tinstant_make(value, lfinfo->restype,
      instants[ninsts - 1]->t);
    /* We cannot DATUM_FREE(value, lfinfo->restype); */
  }
  pfree_array((void **) tofree, nfree);
  interpType interp = Min(MEOS_FLAGS_GET_INTERP(seq1->flags),
    MEOS_FLAGS_GET_INTERP(seq2->flags));
  result[0] = tsequence_make_free(instants, ninsts, inter->lower_inc,
    inter->upper_inc, interp, NORMALIZE);
  return 1;
}

/**
 * @brief Synchronize the temporal values and apply to them the function.
 *
 * This function is applied when the result is an array of sequences and thus
 * it is used when
 * - the function to lift has instantaneous discontinuities
 * - one temporal value has linear and the other has step interpolation.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 * @param[out] result Array on which the pointers of the newly constructed
 * sequences are stored
 */
static int
tfunc_tcontseq_tcontseq_discfn(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  int count = seq1->count + seq2->count;
  /* Array that keeps the new instants to be accumulated */
  TInstant **instants = palloc(sizeof(TInstant *) * count);
  /* Array that keeps the new instants added for the synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  bool linear1 = MEOS_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MEOS_FLAGS_GET_LINEAR(seq2->flags);
  meosType basetype1 = temptype_basetype(seq1->temptype);
  meosType basetype2 = temptype_basetype(seq2->temptype);
  meosType restype = lfinfo->restype;
  meosType resbasetype = temptype_basetype(lfinfo->restype);
  interpType interp = lfinfo->reslinear ? LINEAR : STEP;

  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nseqs = 0, ninsts = 0, nfree = 0;
  /* Synchronize the start instant */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamp(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamp(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamp(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamp(seq2, inter->lower) + 1;
  }
  Datum startvalue1, startvalue2, startresult;
  bool lower_inc = inter->lower_inc;
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant. Notice that we cannot reuse
     * the values from the previous iteration since one of the two sequences
     * may have step interpolation */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
    /* Synchronize the end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[nfree++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[nfree++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);
    Datum intvalue1, intvalue2, intresult;
    TimestampTz inttime = 0; /* make compiler quiet */
    bool lower_eq;

    /* If both segments are constant compute the function at the start and
     * end instants and continue the current sequence */
    if (datum_eq(startvalue1, endvalue1, basetype1) &&
        datum_eq(startvalue2, endvalue2, basetype2))
    {
      instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
    }
    /* If either the start values are equal or both have linear interpolation
     * and the end values are equal compute the function at the start
     * instant, at an intermediate point, and at the end instant */
    else if (datum_eq2(startvalue1, startvalue2, basetype1, basetype2) ||
      datum_eq2(endvalue1, endvalue2, basetype1, basetype2))
    {
      /* Compute the function at the middle time between the start and end
       * instants */
      inttime = start1->t + ((end1->t - start1->t) / 2);
      intvalue1 = tsegment_value_at_timestamp(start1, end1, linear1, inttime);
      intvalue2 = tsegment_value_at_timestamp(start2, end2, linear2, inttime);
      intresult = tfunc_base_base(intvalue1, intvalue2, lfinfo);
      lower_eq = datum_eq(startresult, intresult, resbasetype);
      if (lower_eq)
      {
        /* Continue the current sequence */
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
      }
      else /* ! lower_eq => upper_eq */
      {
        /* Close the current sequence at the start instant */
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
        result[nseqs++] = tsequence_make((const TInstant **) instants,
          ninsts, lower_inc, true, interp, NORMALIZE);
        for (int j = 0; j < ninsts; j++)
          pfree(instants[j]);
        ninsts = 0;
        lower_inc = false;
        /* Start a new sequence */
        instants[ninsts++] = tinstant_make(intresult, restype, start1->t);
      }
    }
    else
    {
      /* Determine whether there is a crossing and compute the value at the
       * crossing if there is one */
      bool hascross = tsegment_intersection(start1, end1, linear1,
        start2, end2, linear2, &intvalue1, &intvalue2, &inttime);
      if (! hascross)
      {
        instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
      }
      else /* hascross */
      {
        intresult = tfunc_base_base(intvalue1, intvalue2, lfinfo);
        lower_eq = datum_eq(startresult, intresult, resbasetype);
        bool upper_eq = datum_eq(intresult, endresult, resbasetype);
        /* If the value at the crossing is equal to the start or to the end
         * value close the current sequence at the start or end instant.
         * Notice that lower_eq and upper_eq can be both true */
        if (lower_eq || upper_eq)
        {
          instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
          instants[ninsts++] = tinstant_make(startresult, restype, inttime);
          /* The upper_inc bound of the closing sequence is true if lower_eq,
           * false if upper_eq */
          result[nseqs++] = tsequence_make((const TInstant **) instants,
            ninsts, lower_inc, lower_eq, interp, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* The lower_inc of the new sequence is false if lower_eq is true */
          lower_inc = ! lower_eq;
          /* Start a new sequence */
          instants[ninsts++] = tinstant_make(endresult, restype, inttime);
        }
        else /* ! lower_eq && ! upper_eq */
        {
          /* The crossing is at the middle: close the current sequence */
          instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
          instants[ninsts++] = tinstant_make(startresult, restype, inttime);
          result[nseqs++] = tsequence_make((const TInstant **) instants,
            ninsts, lower_inc, false, interp, NORMALIZE);
          for (int j = 0; j < ninsts; j++)
            pfree(instants[j]);
          ninsts = 0;
          /* Add a singleton sequence with the crossing */
          instants[0] = tinstant_make(intresult, restype, inttime);
          result[nseqs++] = tsequence_make((const TInstant **) instants, 1,
            true, true, interp, NORMALIZE);
          pfree(instants[0]);
          /* Start a new sequence from the crossing to the end of the segment */
          lower_inc = false;
          instants[ninsts++] = tinstant_make(endresult, restype, inttime);
        }
        DATUM_FREE(intvalue1, basetype1);
        DATUM_FREE(intvalue2, basetype2);
        DATUM_FREE(intresult, resbasetype);
      }
    }
    DATUM_FREE(startresult, resbasetype);
    DATUM_FREE(endresult, resbasetype);
    start1 = end1; start2 = end2;
  }
  /* Add the last instant */
  startvalue1 = tinstant_value(start1);
  startvalue2 = tinstant_value(start2);
  startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
  instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
  result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
    (ninsts == 1) ? true : lower_inc,
    (ninsts == 1) ? true : inter->upper_inc, interp, NORMALIZE);
  DATUM_FREE(startresult, resbasetype);
  pfree(instants);
  return nseqs;
}

/**
 * @brief Synchronizes the temporal values and apply to them the function
 * @note This function is called when one sequence has linear and the other
 * has step interpolation
 */
static int
tfunc_tlinearseq_tstepseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, Span *inter, TSequence **result)
{
  bool linear1 = MEOS_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MEOS_FLAGS_GET_LINEAR(seq2->flags);
  assert(linear1 != linear2);
  /* Array that keeps the new instants to be accumulated */
  TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
  /* Array that keeps the new instants added for synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nfree = 0, ninsts = 0, nseqs = 0;
  /* Synchronize the start instant */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamp(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamp(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamp(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamp(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  meosType restype = lfinfo->restype;
  meosType resbasetype = temptype_basetype(restype);
  /* Compute the function at the start instant */
  Datum startvalue1 = tinstant_value(start1);
  Datum startvalue2 = tinstant_value(start2);
  Datum startresult = tfunc_base_base(startvalue1, startvalue2, lfinfo);
  /* One sequence is produced for each instant of the step sequence */
  while (i < seq1->count && j < seq2->count)
  {
    /* Synchronize the end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    bool makeseq = false;
    if (cmp == 0)
    {
      i++; j++;
      makeseq = true;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[nfree++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[nfree++] = end1;
      makeseq = true;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = tinstant_value(end1);
    Datum endvalue2 = tinstant_value(end2);
    Datum endresult = tfunc_base_base(endvalue1, endvalue2, lfinfo);
    instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
    /* Close the current sequence if the step sequence changed value */
    if (makeseq)
    {
      Datum closeresult = linear1 ?
        tfunc_base_base(endvalue1, startvalue2, lfinfo) :
        tfunc_base_base(startvalue1, endvalue2, lfinfo);
      instants[ninsts++] = tinstant_make(closeresult, restype, end1->t);
      result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
        lower_inc, false, LINEAR, NORMALIZE_NO);
      for (int j = 0; j < ninsts; j++)
        pfree(instants[j]);
      ninsts = 0;
      DATUM_FREE(closeresult, resbasetype);
    }
    DATUM_FREE(startresult, resbasetype);
    start1 = end1; start2 = end2;
    startvalue1 = endvalue1; startvalue2 = endvalue2;
    startresult = endresult;
    lower_inc = true;
  }
  /* Add a final sequence if any */
  if (inter->upper_inc)
    instants[ninsts++] = tinstant_make(startresult, restype, start1->t);
  DATUM_FREE(startresult, resbasetype);
  if (ninsts > 0)
    result[nseqs++] = tsequence_make((const TInstant **) instants, ninsts,
      (ninsts == 1) ? true : lower_inc, (ninsts == 1) ? true : inter->upper_inc,
      LINEAR, NORMALIZE);
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nseqs;
}

/**
 * @brief Synchronizes the temporal values and apply to them the function
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
tfunc_tcontseq_tcontseq_dispatch(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo, TSequence **result)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return 0;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamp(seq1, inter.lower, true, &value1);
    tsequence_value_at_timestamp(seq2, inter.lower, true, &value2);
    Datum resvalue = tfunc_base_base(value1, value2, lfinfo);
    TInstant *inst = tinstant_make(resvalue, lfinfo->restype, inter.lower);
    interpType interp = lfinfo->reslinear ? LINEAR : STEP;
    result[0] = tinstant_to_tsequence(inst, interp);
    DATUM_FREE(value1, temptype_basetype(seq1->temptype));
    DATUM_FREE(value2, temptype_basetype(seq2->temptype));
    DATUM_FREE(resvalue, temptype_basetype(lfinfo->restype));
    pfree(inst);
    return 1;
  }

  if (lfinfo->discont)
    return tfunc_tcontseq_tcontseq_discfn(seq1, seq2, lfinfo, &inter, result);

  bool linear1 = MEOS_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MEOS_FLAGS_GET_LINEAR(seq2->flags);
  if (linear1 == linear2)
    return tfunc_tcontseq_tcontseq_single(seq1, seq2, lfinfo, &inter, result);
  else
    return tfunc_tlinearseq_tstepseq(seq1, seq2, lfinfo, &inter, result);
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 */
Temporal *
tfunc_tcontseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  int count;
  if (lfinfo->discont)
    count = (seq1->count + seq2->count) * 3;
  else
  {
    if (MEOS_FLAGS_GET_LINEAR(seq1->flags) == MEOS_FLAGS_GET_LINEAR(seq2->flags))
      count = 1;
    else
      count = (seq1->count + seq2->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int nseqs = tfunc_tcontseq_tcontseq_dispatch(seq1, seq2, lfinfo, sequences);
  /* The following is ensured by the period bound test in the dispatch
   * function */
  assert(nseqs > 0);
  if (nseqs == 1)
  {
    Temporal *result = (Temporal *) sequences[0];
    pfree(sequences);
    return result;
  }
  else
  {
    TSequenceSet *result = tsequenceset_make_free(sequences, nseqs, NORMALIZE);
    if (result->count == 1)
    {
      Temporal *resultseq = (Temporal *) tsequenceset_to_tsequence(result);
      pfree(result);
      return resultseq;
    }
    else
      return (Temporal *) result;
  }
}

/*****************************************************************************/

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static Temporal *
tfunc_tsequenceset_tcontseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  int loc;
  tsequenceset_find_timestamp(ss, seq->period.lower, &loc);
  /* We are sure that loc < ss->count due to the bounding period test made
   * in the dispatch function */
  int count;
  if (lfinfo->discont)
    count = (ss->totalcount + seq->count) * 3;
  else
  {
    if (MEOS_FLAGS_GET_LINEAR(ss->flags) == MEOS_FLAGS_GET_LINEAR(seq->flags))
      count = ss->count - loc;
    else
      count = (ss->totalcount + seq->count) * 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  int nseqs = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    nseqs += tfunc_tcontseq_tcontseq_dispatch(seq1, seq, lfinfo,
      &sequences[nseqs]);
    int cmp = timestamptz_cmp_internal(upper,
      DatumGetTimestampTz(seq1->period.upper));
    if (cmp < 0 ||
      (cmp == 0 && (! seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  /* We need to normalize when discont is true */
  return (Temporal *) tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static Temporal *
tfunc_tcontseq_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return tfunc_tsequenceset_tcontseq(ss, seq, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] ss1,ss2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
TSequenceSet *
tfunc_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, LiftedFunctionInfo *lfinfo)
{
  int count = ss1->totalcount + ss2->totalcount;
  if (lfinfo->discont)
    count *= 3;
  else
  {
    if (MEOS_FLAGS_GET_LINEAR(ss1->flags) != MEOS_FLAGS_GET_LINEAR(ss2->flags))
      count *= 2;
  }
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int i = 0, j = 0, nseqs = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    nseqs += tfunc_tcontseq_tcontseq_dispatch(seq1, seq2, lfinfo,
      &sequences[nseqs]);
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
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
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Synchronize the temporal values and apply to them the function

 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
Temporal *
tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  /* Bounding box test */
  Span p1, p2;
  temporal_set_period(temp1, &p1);
  temporal_set_period(temp2, &p2);
  if (! overlaps_span_span(&p1, &p2))
    return NULL;

  Temporal *result = NULL;
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  if (temp1->subtype == TINSTANT)
  {
    if (temp2->subtype == TINSTANT)
      result = (Temporal *) tfunc_tinstant_tinstant(
        (TInstant *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
      result = MEOS_FLAGS_GET_DISCRETE(temp2->flags) ?
        (Temporal *) tfunc_tinstant_tdiscseq(
          (TInstant *) temp1, (TSequence *) temp2, lfinfo) :
        (Temporal *) tfunc_tinstant_tcontseq(
          (TInstant *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == TSEQUENCESET */
      result = (Temporal *) tfunc_tinstant_tsequenceset(
        (TInstant *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == TSEQUENCE)
  {
    TSequence *seq1 = (TSequence *) temp1;
    interpType interp1 = MEOS_FLAGS_GET_INTERP(seq1->flags);
    if (temp2->subtype == TINSTANT)
      result = (interp1 == DISCRETE) ?
        (Temporal *) tfunc_tdiscseq_tinstant(
          (TSequence *) temp1, (TInstant *) temp2, lfinfo) :
        (Temporal *) tfunc_tcontseq_tinstant(
          (TSequence *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
    {
      TSequence *seq2 = (TSequence *) temp2;
      interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
      if (interp1 == DISCRETE && interp2 == DISCRETE)
        result = (Temporal *) tfunc_tdiscseq_tdiscseq(seq1, seq2, lfinfo);
      else if (interp1 == DISCRETE && interp2 != DISCRETE)
        result = (Temporal *) tfunc_tdiscseq_tcontseq(seq1, seq2, lfinfo);
      else if (interp1 != DISCRETE && interp2 == DISCRETE)
        result = (Temporal *) tfunc_tcontseq_tdiscseq(seq1, seq2, lfinfo);
      else /* interp1 != DISCRETE && interp2 = DISCRETE */
        result = (Temporal *) tfunc_tcontseq_tcontseq(seq1, seq2, lfinfo);
    }
    else /* temp2->subtype == TSEQUENCESET */
      result = MEOS_FLAGS_GET_DISCRETE(temp1->flags) ?
        (Temporal *) tfunc_tdiscseq_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, lfinfo) :
        (Temporal *) tfunc_tcontseq_tsequenceset(
          (TSequence *) temp1, (TSequenceSet *) temp2, lfinfo);
  }
  else /* temp1->subtype == TSEQUENCESET */
  {
    if (temp2->subtype == TINSTANT)
      result = (Temporal *) tfunc_tsequenceset_tinstant(
        (TSequenceSet *) temp1, (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
      result = MEOS_FLAGS_GET_DISCRETE(temp2->flags) ?
        (Temporal *) tfunc_tsequenceset_tdiscseq(
          (TSequenceSet *) temp1, (TSequence *) temp2, lfinfo) :
        (Temporal *) tfunc_tsequenceset_tcontseq(
          (TSequenceSet *) temp1, (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == TSEQUENCESET */
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
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] inst1,inst2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function #efunc_temporal_temporal and thus the overlapping test is
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
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] is,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tdiscseq_tinstant(const TSequence *is, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tdiscseq_value_at_timestamp(is, inst->t, &value1))
    return -1;

  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] inst,is Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tdiscseq(const TInstant *inst, const TSequence *is,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tdiscseq_tinstant(is, inst, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with the
 * optional argument
 * @param[in] seq,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tcontseq_tinstant(const TSequence *seq, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  /* The following call is ensured to return true due to the period bound test
   * in the dispatch function */
  tsequence_value_at_timestamp(seq, inst->t, true, &value1);
  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] inst,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tcontseq(const TInstant *inst, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tcontseq_tinstant(seq, inst, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] ss,inst Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tinstant(const TSequenceSet *ss, const TInstant *inst,
  LiftedFunctionInfo *lfinfo)
{
  Datum value1;
  if (! tsequenceset_value_at_timestamp(ss, inst->t, true, &value1))
    return -1;

  Datum value2 = tinstant_value(inst);
  bool result = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
  return result ? 1 : 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] inst,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tsequenceset_tinstant(ss, inst, lfinfo);
}

/*****************************************************************************/

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @note This function is called by other functions besides the dispatch
 * function efunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
static int
efunc_tdiscseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq2, j);
  while (i < seq1->count && j < seq2->count)
  {
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      Datum value1 = tinstant_value(inst1);
      Datum value2 = tinstant_value(inst2);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
      i++;
    }
    else if (cmp < 0)
      inst1 = TSEQUENCE_INST_N(seq1, ++i);
    else
      inst2 = TSEQUENCE_INST_N(seq2, ++j);
  }
  return 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tcontseq_tdiscseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  TimestampTz upper1 = DatumGetTimestampTz(seq1->period.upper);
  for (int i = 0; i < seq2->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq2, i);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      Datum value1;
      tsequence_value_at_timestamp(seq1, inst->t, true, &value1);
      Datum value2 = tinstant_value(inst);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
    }
    if ( upper1 < inst->t)
      break;
  }
  return 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tdiscseq_tcontseq(const TSequence *seq1, const TSequence *seq2,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tcontseq_tdiscseq(seq2, seq1, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tdiscseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  while (i < ss->count && j < seq->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    const TInstant *inst = TSEQUENCE_INST_N(seq, j);
    if (contains_period_timestamp(&seq1->period, inst->t))
    {
      Datum value1;
      tsequenceset_value_at_timestamp(ss, inst->t, true, &value1);
      Datum value2 = tinstant_value(inst);
      if (DatumGetBool(tfunc_base_base(value1, value2, lfinfo)))
        return 1;
    }
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      inst->t);
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
 * @brief Synchronize the temporal values and apply to them the function with
 * the optional argument
 * @param[in] is,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tdiscseq_tsequenceset(const TSequence *is, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tsequenceset_tdiscseq(ss, is, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function.
 * This function is applied for functions with instantaneous discontinuities
 * and thus the result is an array of sequences. This function is applied
 * when at least one temporal value has linear interpolation.
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 * @param[in] inter Overlapping period of the two sequences
 */
static int
efunc_tcontseq_tcontseq_discfn(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo, Span *inter)
{
  /* Array that keeps the new instants added for the synchronization */
  TInstant **tofree = palloc(sizeof(TInstant *) *
    (seq1->count + seq2->count) * 2);
  TInstant *start1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *start2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  int i = 1, j = 1, nfree = 0;
  /* Synchronize the start instant */
  if (start1->t < DatumGetTimestampTz(inter->lower))
  {
    start1 = tsequence_at_timestamp(seq1, inter->lower);
    tofree[nfree++] = start1;
    i = tcontseq_find_timestamp(seq1, inter->lower) + 1;
  }
  else if (start2->t < DatumGetTimestampTz(inter->lower))
  {
    start2 = tsequence_at_timestamp(seq2, inter->lower);
    tofree[nfree++] = start2;
    j = tcontseq_find_timestamp(seq2, inter->lower) + 1;
  }
  bool lower_inc = inter->lower_inc;
  bool linear1 = MEOS_FLAGS_GET_LINEAR(seq1->flags);
  bool linear2 = MEOS_FLAGS_GET_LINEAR(seq2->flags);
  Datum startvalue1, startvalue2;
  meosType basetype1 = temptype_basetype(seq1->temptype);
  meosType basetype2 = temptype_basetype(seq2->temptype);
  while (i < seq1->count && j < seq2->count)
  {
    /* Compute the function at the start instant */
    startvalue1 = tinstant_value(start1);
    startvalue2 = tinstant_value(start2);
    if (lower_inc &&
        DatumGetBool(tfunc_base_base(startvalue1, startvalue2, lfinfo)))
    {
      pfree_array((void **) tofree, nfree);
      return 1;
    }
    /* Synchronize the end instants */
    TInstant *end1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    TInstant *end2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
    int cmp = timestamptz_cmp_internal(end1->t, end2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      end2 = tsegment_at_timestamp(start2, end2, linear2, end1->t);
      tofree[nfree++] = end2;
    }
    else
    {
      j++;
      end1 = tsegment_at_timestamp(start1, end1, linear1, end2->t);
      tofree[nfree++] = end1;
    }
    /* Compute the function at the end instant */
    Datum endvalue1 = linear1 ? tinstant_value(end1) : startvalue1;
    Datum endvalue2 = linear2 ? tinstant_value(end2) : startvalue2;
    if (DatumGetBool(tfunc_base_base(endvalue1, endvalue2, lfinfo)))
    {
      pfree_array((void **) tofree, nfree);
      return 1;
    }

    Datum intvalue1, intvalue2;
    TimestampTz inttime;
    /* If either the start values or the end values are equal and both have
     * linear interpolation compute the function an intermediate point */
    if (datum_eq2(startvalue1, startvalue2, basetype1, basetype2) ||
         (linear1 && linear2 &&
          datum_eq2(endvalue1, endvalue2, basetype1, basetype2)))
    {
      /* Compute the function at the middle time between the start and end instants */
      inttime = start1->t + ((end1->t - start1->t) / 2);
      intvalue1 = tsegment_value_at_timestamp(start1, end1, linear1, inttime);
      intvalue2 = tsegment_value_at_timestamp(start2, end2, linear2, inttime);
      if (DatumGetBool(tfunc_base_base(intvalue1, intvalue2, lfinfo)))
      {
        pfree_array((void **) tofree, nfree);
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
        pfree_array((void **) tofree, nfree);
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
      pfree_array((void **) tofree, nfree);
      return 1;
    }
  }
  pfree_array((void **) tofree, nfree);
  return 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * (dispatch function)
 * @note This function is called for each composing sequence of a temporal
 * sequence set and therefore the bounding period test is repeated
 */
static int
efunc_tcontseq_tcontseq(const TSequence *seq1,
  const TSequence *seq2, LiftedFunctionInfo *lfinfo)
{
  /* Test whether the bounding period of the two temporal values overlap */
  Span inter;
  if (! inter_span_span(&seq1->period, &seq2->period, &inter))
    return -1;

  /* If the two sequences intersect at an instant */
  if (inter.lower == inter.upper)
  {
    Datum value1, value2;
    tsequence_value_at_timestamp(seq1, inter.lower, true, &value1);
    tsequence_value_at_timestamp(seq2, inter.lower, true, &value2);
    bool resvalue = DatumGetBool(tfunc_base_base(value1, value2, lfinfo));
    return resvalue ? 1 : 0;
  }
  /* Ever functions are always discontinuous */
  assert(lfinfo->discont);
  return efunc_tcontseq_tcontseq_discfn(seq1, seq2, lfinfo, &inter);
}

/*****************************************************************************/

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] ss,seq Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tcontseq(const TSequenceSet *ss, const TSequence *seq,
  LiftedFunctionInfo *lfinfo)
{
  TimestampTz upper = DatumGetTimestampTz(seq->period.upper);
  int loc;
  tsequenceset_find_timestamp(ss, seq->period.lower, &loc);
  /* We are sure that loc < ss->count due to the bounding period test made
   * in the dispatch function */
  for (int i = loc; i < ss->count; i++)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss, i);
    if (efunc_tcontseq_tcontseq(seq1, seq, lfinfo) == 1)
      return 1;
    int cmp = timestamptz_cmp_internal(upper,
      DatumGetTimestampTz(seq1->period.upper));
    if (cmp < 0 ||
      (cmp == 0 && (! seq->period.upper_inc || seq1->period.upper_inc)))
      break;
  }
  return 0;
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] seq,ss Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ss,
  LiftedFunctionInfo *lfinfo)
{
  lfinfo_invert_args(lfinfo);
  return efunc_tsequenceset_tcontseq(ss, seq, lfinfo);
}

/**
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] ss1,ss2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
static int
efunc_tsequenceset_tsequenceset(const TSequenceSet *ss1,
  const TSequenceSet *ss2, LiftedFunctionInfo *lfinfo)
{
  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    if (efunc_tcontseq_tcontseq(seq1, seq2, lfinfo) == 1)
      return 1;
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
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
 * @brief Synchronize the temporal values and apply to them the function
 * @param[in] temp1,temp2 Temporal values
 * @param[in] lfinfo Information about the lifted function
 */
int
efunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
  LiftedFunctionInfo *lfinfo)
{
  /* Bounding box test */
  Span p1, p2;
  temporal_set_period(temp1, &p1);
  temporal_set_period(temp2, &p2);
  if (! overlaps_span_span(&p1, &p2))
    return -1;

  int result;
  assert(temptype_subtype(temp1->subtype));
  assert(temptype_subtype(temp2->subtype));
  if (temp1->subtype == TINSTANT)
  {
    if (temp2->subtype == TINSTANT)
      result = efunc_tinstant_tinstant((TInstant *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
      result = MEOS_FLAGS_GET_DISCRETE(temp2->flags) ?
        efunc_tinstant_tdiscseq((TInstant *) temp1,
          (TSequence *) temp2, lfinfo) :
        efunc_tinstant_tcontseq((TInstant *) temp1,
          (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == TSEQUENCESET */
      result = efunc_tinstant_tsequenceset((TInstant *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  else if (temp1->subtype == TSEQUENCE)
  {
    if (temp2->subtype == TINSTANT)
      result =  MEOS_FLAGS_GET_DISCRETE(temp1->flags) ?
        efunc_tdiscseq_tinstant((TSequence *) temp1,
          (TInstant *) temp2, lfinfo) :
        efunc_tcontseq_tinstant((TSequence *) temp1,
          (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
    {
      interpType interp1 = MEOS_FLAGS_GET_INTERP(temp1->flags);
      interpType interp2 = MEOS_FLAGS_GET_INTERP(temp2->flags);
      if (interp1 == DISCRETE && interp2 == DISCRETE )
        result = efunc_tdiscseq_tdiscseq((TSequence *) temp1,
          (TSequence *) temp2, lfinfo);
      else if (interp1 == DISCRETE && interp2 != DISCRETE)
        result = efunc_tdiscseq_tcontseq((TSequence *) temp1,
          (TSequence *) temp2, lfinfo);
      else if (interp1 != DISCRETE && interp2 == DISCRETE)
        result = efunc_tcontseq_tdiscseq((TSequence *) temp1,
          (TSequence *) temp2, lfinfo);
      else /* interp1 != DISCRETE && interp2 != DISCRETE */
        result = efunc_tcontseq_tcontseq((TSequence *) temp1,
          (TSequence *) temp2, lfinfo);
    }
    else /* temp2->subtype == TSEQUENCESET */
      result = MEOS_FLAGS_GET_DISCRETE(temp1->flags) ?
        efunc_tdiscseq_tsequenceset((TSequence *) temp1,
          (TSequenceSet *) temp2, lfinfo) :
        efunc_tsequence_tsequenceset((TSequence *) temp1,
          (TSequenceSet *) temp2, lfinfo);
  }
  else /* temp1->subtype == TSEQUENCESET */
  {
    if (temp2->subtype == TINSTANT)
      result = efunc_tsequenceset_tinstant((TSequenceSet *) temp1,
        (TInstant *) temp2, lfinfo);
    else if (temp2->subtype == TSEQUENCE)
      result = MEOS_FLAGS_GET_DISCRETE(temp2->flags) ?
        efunc_tsequenceset_tdiscseq((TSequenceSet *) temp1,
          (TSequence *) temp2, lfinfo) :
        efunc_tsequenceset_tcontseq((TSequenceSet *) temp1,
          (TSequence *) temp2, lfinfo);
    else /* temp2->subtype == TSEQUENCESET */
      result = efunc_tsequenceset_tsequenceset((TSequenceSet *) temp1,
        (TSequenceSet *) temp2, lfinfo);
  }
  return result;
}

/*****************************************************************************/
