
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
 * @brief Mathematical operators (+, -, *, /) and functions (round, degrees,
 * ...) for temporal numbers
 */

#include "general/tnumber_mathfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/type_util.h"

/*****************************************************************************
 * General functions on datums
 *****************************************************************************/

/**
 * @brief Find the single timestamptz at which the operation of two temporal
 * number segments is at a local minimum/maximum
 * @details The function supposes that the instants are synchronized, that is,
 * `start1->t = start2->t` and `end1->t = end2->t`.
 * The function only return an intersection at the middle, that is, it
 * it returns false if the timestamp found is not at a bound
 * @note This function is called only when both sequences are linear
 */
static bool
tnumber_arithop_tp_at_timestamp1(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1 = tnumberinst_double(start1);
  double x2 = tnumberinst_double(end1);
  double x3 = tnumberinst_double(start2);
  double x4 = tnumberinst_double(end2);
  /* Compute the instants t1 and t2 at which the linear functions of the two
   * segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
   * minimum/maximum exactly at the middle between t1 and t2.
   * To reduce problems related to floating point arithmetic, t1 and t2
   * are shifted, respectively, to 0 and 1 before the computation */
  if ((x2 - x1) == 0.0 || (x4 - x3) == 0.0)
    return false;

  long double d1 = (-1 * x1) / (x2 - x1);
  long double d2 = (-1 * x3) / (x4 - x3);
  long double min = Min(d1, d2);
  long double max = Max(d1, d2);
  long double fraction = min + (max - min) / 2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * @brief Find the single timestamptz at which the operation of two temporal
 * number segments is at a local minimum/maximum
 * @note This function is called only when both sequences are linear.
 */
static bool
tnumber_arithop_tp_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TArithmetic op, Datum *value,
  TimestampTz *t)
{
  if (! tnumber_arithop_tp_at_timestamp1(start1, end1, start2, end2, t))
    return false;
  Datum value1 = tsegment_value_at_timestamptz(start1, end1, LINEAR, *t);
  Datum value2 = tsegment_value_at_timestamptz(start2, end2, LINEAR, *t);
  assert (op == MULT || op == DIV);
  assert (start1->temptype == start2->temptype);
  meosType basetype = temptype_basetype(start1->temptype);
  *value = (op == '*') ?
    datum_mult(value1, value2, basetype) :
    datum_div(value1, value2, basetype);
  return true;
}

/**
 * @brief Find the single timestamptz at which the multiplication of two
 * temporal number segments is at a local minimum/maximum
 * @note This function is called only when both sequences are linear.
 */
bool
tnumber_mult_tp_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  return tnumber_arithop_tp_at_timestamptz(start1, end1, start2, end2, MULT,
    value, t);
}

/**
 * @brief Find the single timestamptz at which the division of two temporal
 * number segments is at a local minimum/maximum
 * @note This function is called only when both sequences are linear.
 */
bool
tnumber_div_tp_at_timestamptz(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  return tnumber_arithop_tp_at_timestamptz(start1, end1, start2, end2, DIV,
    value, t);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Generic arithmetic operator on a temporal number and a number
 * @param[in] temp Temporal number
 * @param[in] value Number
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] func Arithmetic function
 * @param[in] invert True if the base value is the first argument of the
 * function
 */
Temporal *
arithop_tnumber_number(const Temporal *temp, Datum value, TArithmetic oper,
  Datum (*func)(Datum, Datum, meosType), bool invert)
{
  assert(tnumber_type(temp->temptype));
  meosType basetype = temptype_basetype(temp->temptype);
  /* If division test whether the denominator is zero */
  if (oper == DIV)
  {
    if (invert)
    {
      if (ever_eq_temporal_base(temp, Float8GetDatum(0.0)))
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Division by zero");
        return NULL;
      }
    }
    else
    {
      double d = datum_double(value, basetype);
      if (fabs(d) < MEOS_EPSILON)
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Division by zero");
        return NULL;
      }
    }
  }

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = basetype;
  lfinfo.restype = temp->temptype;
  /* This parameter is not used for temp <op> base */
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * @brief Generic arithmetic operator on two temporal numbers
 * @param[in] temp1,temp2 Temporal numbers
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] func Arithmetic function
 * @param[in] tpfunc Function determining the turning point
 */
Temporal *
arithop_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2,
  TArithmetic oper, Datum (*func)(Datum, Datum, meosType),
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *,
    const TInstant *, Datum *, TimestampTz *))
{
  assert(tnumber_type(temp1->temptype));
  assert(temp1->temptype == temp2->temptype);
  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(temp1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(temp2->flags);

  /* If division test whether the denominator will ever be zero during
   * the common timespan */
  if (oper == DIV)
  {
    SpanSet *ss = temporal_time(temp1);
    Temporal *projtemp2 = temporal_restrict_tstzspanset(temp2, ss, REST_AT);
    if (projtemp2 == NULL)
      return NULL;
    if (ever_eq_temporal_base(projtemp2, Float8GetDatum(0.0)))
    {
      pfree(projtemp2);
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "Division by zero");
      return NULL;
    }
  }

  /* Fill the lifted structure */
  meosType basetype = temptype_basetype(temp1->temptype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = basetype;
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = temp1->temptype;
  lfinfo.reslinear = linear1 || linear2;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = (oper == MULT || oper == DIV) && linear1 && linear2 ?
    tpfunc : NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Absolute value
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the absolute value of a temporal number instant
 * @param[in] inst Temporal instant
 * @csqlfn #Tnumber_abs()
 */
TInstant *
tnumberinst_abs(const TInstant *inst)
{
  assert(inst); assert(tnumber_type(inst->temptype));
  meosType basetype = temptype_basetype(inst->temptype);
  assert(tnumber_basetype(basetype));
  Datum value = tinstant_value_p(inst);
  Datum absvalue;
  if (basetype == T_INT4)
    absvalue = Int32GetDatum(abs(DatumGetInt32(value)));
  else /* basetype == T_FLOAT8 */
    absvalue = Float8GetDatum(fabs(DatumGetFloat8(value)));
  return tinstant_make(absvalue, inst->temptype, inst->t);
}

/**
 * @brief Return the absolute value of a temporal number sequence
 */
static TSequence *
tnumberseq_iter_abs(const TSequence *seq)
{
  assert(seq); assert(tnumber_type(seq->temptype));
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  for (int i = 0; i < seq->count; i++)
    instants[i] = tnumberinst_abs(TSEQUENCE_INST_N(seq, i));
  return tsequence_make_free(instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, interp, NORMALIZE);
}

/**
 * @brief Return the absolute value of a temporal number sequence
 */
static TSequence *
tnumberseq_linear_abs(const TSequence *seq)
{
  assert(seq);
  assert(tnumber_type(seq->temptype));
  const TInstant *inst1;
  meosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    inst1 = TSEQUENCE_INST_N(seq, 0);
    return tinstant_to_tsequence_free(tnumberinst_abs(inst1), LINEAR);
  }

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
  inst1 = TSEQUENCE_INST_N(seq, 0);
  instants[0] = tnumberinst_abs(inst1);
  Datum value1 = tinstant_value_p(inst1);
  double dvalue1 = datum_double(value1, basetype);
  int ninsts = 1;
  Datum dzero = (basetype == T_INT4) ? Int32GetDatum(0) : Float8GetDatum(0);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    double dvalue2 = datum_double(value2, basetype);
    /* Add the instant when the segment has value equal to zero */
    if ((dvalue1 < 0 && dvalue2 > 0) || (dvalue1 > 0 && dvalue2 < 0))
    {
      double ratio = fabs(dvalue1) / (fabs(dvalue1) + fabs(dvalue2));
      double duration = (double) (inst2->t - inst1->t);
      TimestampTz t = inst1->t + (TimestampTz) (duration * ratio);
      if (t != inst1->t && t != inst2->t)
        instants[ninsts++] = tinstant_make(dzero, seq->temptype, t);
    }
    instants[ninsts++] = tnumberinst_abs(inst2);
    inst1 = inst2;
    value1 = value2;
    dvalue1 = dvalue2;
  }
  /* We are sure that ninsts > 0 */
  return tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the absolute value of a temporal number sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tnumber_abs()
 */
TSequence *
tnumberseq_abs(const TSequence *seq)
{
  assert(seq); assert(tnumber_type(seq->temptype));
  return MEOS_FLAGS_LINEAR_INTERP(seq->flags) ?
    tnumberseq_linear_abs(seq) : tnumberseq_iter_abs(seq);
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the absolute value of a temporal number sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tnumber_abs()
 */
TSequenceSet *
tnumberseqset_abs(const TSequenceSet *ss)
{
  assert(ss); assert(tnumber_type(ss->temptype));
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    sequences[i] = linear ?
      tnumberseq_linear_abs(seq) : tnumberseq_iter_abs(seq);
  }
  return tsequenceset_make_free(sequences, ss->count, NORMALIZE);
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the absolute value of a temporal number
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_abs()
 */
Temporal *
tnumber_abs(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal *) tnumberinst_abs((TInstant *) temp);
    case TSEQUENCE:
      return (Temporal *) tnumberseq_abs((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_abs((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Delta value
 *****************************************************************************/

/**
 * @brief Return the delta value of two numbers
 */
static Datum
delta_value(Datum value1, Datum value2, meosType basetype)
{
  assert(basetype == T_INT4 || basetype == T_FLOAT8);
  if (basetype == T_INT4)
    return Int32GetDatum(DatumGetInt32(value2) - DatumGetInt32(value1));
  else /* basetype == T_FLOAT8 */
    return Float8GetDatum(DatumGetFloat8(value2) - DatumGetFloat8(value1));
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the delta value of a temporal number sequence
 */
TSequence *
tnumberseq_delta_value(const TSequence *seq)
{
  assert(seq); assert(tnumber_type(seq->temptype));
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  meosType basetype = temptype_basetype(seq->temptype);
  Datum delta = 0; /* make compiler quiet */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    delta = delta_value(value1, value2, basetype);
    instants[i - 1] = tinstant_make(delta, seq->temptype, inst1->t);
    inst1 = inst2;
    value1 = value2;
  }
  instants[seq->count - 1] = tinstant_make(delta, seq->temptype, inst1->t);
  /* Resulting sequence has discrete or step interpolation */
  interpType interp = MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ? DISCRETE : STEP;
  return tsequence_make_free(instants, seq->count, seq->period.lower_inc,
    interp == DISCRETE ? true : false, interp, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the delta value of a temporal number sequence set
 * @param[in] ss Temporal sequence set
 */
TSequenceSet *
tnumberseqset_delta_value(const TSequenceSet *ss)
{
  assert(ss); assert(tnumber_type(ss->temptype));
  TSequence *delta;
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    delta = tnumberseq_delta_value(TSEQUENCESET_SEQ_N(ss, 0));
    return tsequence_to_tsequenceset_free(delta);
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    delta = tnumberseq_delta_value(TSEQUENCESET_SEQ_N(ss, i));
    if (delta)
      sequences[nseqs++] = delta;
  }
  /* Resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the delta value of a temporal number
 * @param[in] temp Temporal value
 * @csqlfn #Tnumber_delta_value()
 */
Temporal *
tnumber_delta_value(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tnumberseq_delta_value((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_delta_value((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Angular difference
 *****************************************************************************/

/**
 * @brief Return the angular difference, i.e., the smaller angle between the
 * two degree values
 */
static Datum
angular_difference(Datum degrees1, Datum degrees2)
{
  double diff = fabs(DatumGetFloat8(degrees1) - DatumGetFloat8(degrees2));
  if (diff > 180)
   diff = fabs(diff - 360);
  return Float8GetDatum(diff);
}

/**
 * @brief Return the temporal angular difference of a temporal number
 */
static int
tnumberseq_angular_difference_iter(const TSequence *seq, TInstant **result)
{
  /* Instantaneous sequence */
  if (seq->count == 1)
    return 0;

  /* General case */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  Datum angdiff = Float8GetDatum(0);
  int ninsts = 0;
  if (seq->period.lower_inc)
    result[ninsts++] = tinstant_make(angdiff, seq->temptype, inst1->t);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value_p(inst2);
    angdiff = angular_difference(value1, value2);
    if (i != seq->count - 1 || seq->period.upper_inc)
      result[ninsts++] = tinstant_make(angdiff, seq->temptype, inst2->t);
    inst1 = inst2;
    value1 = value2;
  }
  return ninsts;
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the temporal angular difference of a temporal number sequence
 * @param[in] seq Temporal sequence
 */
TSequence *
tnumberseq_angular_difference(const TSequence *seq)
{
  assert(seq);
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  /* We are sure that there are at least 2 instants */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int ninsts = tnumberseq_angular_difference_iter(seq, instants);
  /* Resulting sequence has discrete interpolation */
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE, NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the angular difference of a temporal number sequence set
 * @param[in] ss Temporal sequence set
 */
TSequence *
tnumberseqset_angular_difference(const TSequenceSet *ss)
{
  assert(ss);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_angular_difference(TSEQUENCESET_SEQ_N(ss, 0));

  /* General case */
  TInstant **instants = palloc(sizeof(TSequence *) * ss->totalcount);
  int ninsts = 0;
  for (int i = 0; i < ss->count; i++)
    ninsts += tnumberseq_angular_difference_iter(TSEQUENCESET_SEQ_N(ss, i),
      &instants[ninsts]);
  /* Resulting sequence has discrete interpolation */
  return tsequence_make_free(instants, ninsts, true, true, DISCRETE,
    NORMALIZE);
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the angular difference of a temporal number
 * @param[in] temp Temporal value
 * @csqlfn #Tpoint_angular_difference()
 */
Temporal *
tnumber_angular_difference(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL);

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tnumberseq_angular_difference((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tnumberseqset_angular_difference((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Derivative functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the derivative of a temporal float sequence
 * @param[in] seq Temporal sequence
 * @csqlfn #Tfloat_derivative()
 */
TSequence *
tfloatseq_derivative(const TSequence *seq)
{
  assert(seq); assert(seq->temptype == T_TFLOAT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  meosType basetype = temptype_basetype(seq->temptype);
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  Datum value1 = tinstant_value_p(inst1);
  double dvalue1 = datum_double(value1, basetype);
  double derivative = 0.0; /* make compiler quiet */
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    Datum value2 = tinstant_value_p(inst2);
    double dvalue2 = datum_double(value2, basetype);
    derivative = datum_eq(value1, value2, basetype) ? 0.0 :
      (dvalue1 - dvalue2) / ((double)(inst2->t - inst1->t) / 1000000);
    instants[i] = tinstant_make(Float8GetDatum(derivative), T_TFLOAT, inst1->t);
    inst1 = inst2;
    value1 = value2;
    dvalue1 = dvalue2;
  }
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(derivative),
    T_TFLOAT, seq->period.upper);
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make((const TInstant **) instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  pfree_array((void **) instants, seq->count - 1);
  return result;
}

/**
 * @ingroup meos_internal_temporal_math
 * @brief Return the derivative of a temporal float sequence set
 * @param[in] ss Temporal sequence set
 * @csqlfn #Tfloat_derivative()
 */
TSequenceSet *
tfloatseqset_derivative(const TSequenceSet *ss)
{
  assert(ss); assert(ss->temptype == T_TFLOAT);
  TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
  int nseqs = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (seq->count > 1)
      sequences[nseqs++] = tfloatseq_derivative(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the derivative of a temporal number
 * @param[in] temp Temporal value
 * @see #tfloatseq_derivative()
 * @see #tfloatseqset_derivative()
 * @csqlfn #Tfloat_derivative()
 */
Temporal *
tfloat_derivative(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);

  if (! ensure_linear_interp(temp->flags))
    return NULL;

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return NULL;
    case TSEQUENCE:
      return (Temporal *) tfloatseq_derivative((TSequence *) temp);
    default: /* TSEQUENCESET */
      return (Temporal *) tfloatseqset_derivative((TSequenceSet *) temp);
  }
}

/*****************************************************************************
 * Exponential functions
 *****************************************************************************/

/**
 * @brief Return the exponential of a double
 * @param[in] d Value
 * @note PostgreSQL function: dexp(PG_FUNCTION_ARGS)
 */

double
pg_exp(double d)
{
  double result;

	/*
	 * Handle NaN and Inf cases explicitly.  This avoids needing to assume
	 * that the platform's exp() conforms to POSIX for these cases, and it
	 * removes some edge cases for the overflow checks below.
	 */
	if (isnan(d))
		result = d;
	else if (isinf(d))
	{
		/* Per POSIX, exp(-Inf) is 0 */
		result = (d > 0.0) ? d : 0;
	}
	else
	{
		/*
		 * On some platforms, exp() will not set errno but just return Inf or
		 * zero to report overflow/underflow; therefore, test both cases.
		 */
		errno = 0;
		result = exp(d);
		if (unlikely(errno == ERANGE))
		{
			if (result != 0.0)
				float_overflow_error();
			else
				float_underflow_error();
		}
		else if (unlikely(isinf(result)))
			float_overflow_error();
		else if (unlikely(result == 0.0))
			float_underflow_error();
	}

  return result;
}


/**
 * @brief Return the exponential of a double
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_exp(Datum d)
{
  return Float8GetDatum(pg_exp(DatumGetFloat8(d)));
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the exponential of a double
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_exp()
 */
Temporal *
tfloat_exp(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_exp;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Logarithm functions
 *****************************************************************************/

/**
 * @brief Return the natural logarithm of a double
 * @param[in] d Value
 * @note PostgreSQL function: dlog1(PG_FUNCTION_ARGS)
 */

double
pg_ln(double d)
{
  double result;

  /*
   * Emit particular SQLSTATE error codes for ln(). This is required by the
   * SQL standard.
   */
  if (d == 0.0)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot take logarithm of zero");
  if (d < 0)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot take logarithm of a negative number");

  result = log(d);
  if (unlikely(isinf(result)) && !isinf(d))
    float_overflow_error();
  if (unlikely(result == 0.0) && d != 1.0)
    float_underflow_error();

  return result;
}

/**
 * @brief Return the natural logarithm of a double
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_ln(Datum d)
{
  return Float8GetDatum(pg_ln(DatumGetFloat8(d)));
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the natural logarithm of a double
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_ln()
 */
Temporal *
tfloat_ln(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  /* Cannot compute logarithm of 0 or negative value */
  if (ever_le_temporal_base(temp, Float8GetDatum(0.0)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot take logarithm of zero or a negative number");
    return NULL;
  }
  
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_ln;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/

/**
 * @brief Return the logarithm base 10 of a double
 * @param[in] d Value
 * @note PostgreSQL function: dlog10(PG_FUNCTION_ARGS)
 */
double
pg_log10(double d)
{
  double result;

  /*
   * Emit particular SQLSTATE error codes for log(). The SQL spec doesn't
   * define log(), but it does define ln(), so it makes sense to emit the
   * same error code for an analogous error condition.
   */
  if (d == 0.0)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot take logarithm of zero");
  if (d < 0)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot take logarithm of a negative number");

  result = log10(d);
  if (unlikely(isinf(result)) && !isinf(d))
    float_overflow_error();
  if (unlikely(result == 0.0) && d != 1.0)
    float_underflow_error();

  return result;
}

/**
 * @brief Return the logarithm base 10 of a double
 * @param[in] d Value
 * @note Function used for lifting
 */
static Datum
datum_log10(Datum d)
{
  return Float8GetDatum(pg_log10(DatumGetFloat8(d)));
}

/**
 * @ingroup meos_temporal_math
 * @brief Return the natural logarithm of a double
 * @param[in] temp Temporal value
 * @csqlfn #Tfloat_log10()
 */
Temporal *
tfloat_log10(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL);
  /* Cannot compute logarithm of 0 or negative value */
  if (ever_le_temporal_base(temp, Float8GetDatum(0.0)))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Cannot take logarithm of zero or a negative number");
    return NULL;
  }
  
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_log10;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TFLOAT;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
