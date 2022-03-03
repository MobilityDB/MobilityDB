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
 * @file tnumber_mathfuncs.c
 * Temporal mathematical operators (+, -, *, /) and functions (round, degrees).
 */

#include "general/tnumber_mathfuncs.h"

#include <assert.h>
#include <math.h>
#include <utils/builtins.h>

#include "general/period.h"
#include "general/timeops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/lifting.h"

/*****************************************************************************
 * Miscellaneous functions on datums
 *****************************************************************************/

/**
 * Round the number to the number of decimal places
 */
Datum
datum_round_float(Datum value, Datum prec)
{
  Datum number = call_function1(float8_numeric, value);
  Datum round = call_function2(numeric_round, number, prec);
  return call_function1(numeric_float8, round);
}

/**
 * Convert the number from radians to degrees
 */
static Datum
datum_degrees(Datum value)
{
  return call_function1(degrees, value);
}

/**
 * Find the single timestamptz at which the operation of two temporal
 * number segments is at a local minimum/maximum. The function supposes that
 * the instants are synchronized, that is, start1->t = start2->t and
 * end1->t = end2->t. The function only return an intersection at the middle,
 * that is, it returns false if the timestamp found is not at a bound.
 *
 @note This function is called only when both sequences are linear.
 */
static bool
tnumber_arithop_tp_at_timestamp1(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1 = datum_double(tinstant_value(start1), start1->basetypid);
  double x2 = datum_double(tinstant_value(end1), start1->basetypid);
  double x3 = datum_double(tinstant_value(start2), start2->basetypid);
  double x4 = datum_double(tinstant_value(end2), start2->basetypid);
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
  long double fraction = min + (max - min)/2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= MOBDB_EPSILON || fraction >= (1.0 - MOBDB_EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (TimestampTz) (duration * fraction);
  return true;
}

/**
 * Find the single timestamptz at which the operation of two temporal
 * number segments is at a local minimum/maximum. 
 *
 @note This function is called only when both sequences are linear.
 */
static bool
tnumber_arithop_tp_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, char op, Datum *value,
  TimestampTz *t)
{
  if (! tnumber_arithop_tp_at_timestamp1(start1, end1, start2, end2, t))
    return false;
  Datum value1 = tsegment_value_at_timestamp(start1, end1, LINEAR, *t);
  Datum value2 = tsegment_value_at_timestamp(start2, end2, LINEAR, *t);
  assert (op == '*' || op == '/');
  *value = (op == '*') ?
    datum_mult(value1, value2, start1->basetypid, start2->basetypid) :
    datum_div(value1, value2, start1->basetypid, start2->basetypid);
  return true;
}

/**
 * Find the single timestamptz at which the multiplication of two temporal
 * number segments is at a local minimum/maximum. 
 *
 @note This function is called only when both sequences are linear.
 */
static bool
tnumber_mult_tp_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  return tnumber_arithop_tp_at_timestamp(start1, end1, start2, end2, '*',
    value, t);
}

/**
 * Find the single timestamptz at which the division of two temporal
 * number segments is at a local minimum/maximum. 
 *
 @note This function is called only when both sequences are linear.
 */
static bool
tnumber_div_tp_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, Datum *value, TimestampTz *t)
{
  return tnumber_arithop_tp_at_timestamp(start1, end1, start2, end2, '/',
    value, t);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Generic arithmetic operator on a temporal number and a number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] temp Temporal number
 * @param[in] value Number
 * @param[in] basetypid Oid of the base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */
Temporal *
arithop_tnumber_base1(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper,
  Temporal *temp, Datum value, Oid basetypid, bool invert)
{
  ensure_tnumber_base_type(basetypid);
  /* If division test whether the denominator is zero */
  if (oper == DIV)
  {
    if (invert)
    {
      if (temporal_ever_eq_internal(temp, Float8GetDatum(0.0)))
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Division by zero")));
    }
    else
    {
      double d = datum_double(value, basetypid);
      if (fabs(d) < MOBDB_EPSILON)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Division by zero")));
    }
  }

  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp->basetypid;
  lfinfo.argtypid[1] = basetypid;
  lfinfo.restypid = base_oid_from_temporal(temptypid);
  /* This parameter is not used for tnumber <op> base */
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  return tfunc_temporal_base(temp, value, &lfinfo);
}

/**
 * Generic arithmetic operator on a number an a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
Datum
arithop_base_tnumber(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = arithop_tnumber_base1(fcinfo, func, oper,
    temp, value, basetypid, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Generic arithmetic operator on a temporal number an a number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
Datum
arithop_tnumber_base(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = arithop_tnumber_base1(fcinfo, func, oper,
    temp, value, basetypid, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Generic arithmetic operator on a temporal numbers
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] tpfunc Function determining the turning point
 */
static Datum
arithop_tnumber_tnumber(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper,
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *, 
    const TInstant *, Datum *, TimestampTz *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool linear1 = MOBDB_FLAGS_GET_LINEAR(temp1->flags);
  bool linear2 = MOBDB_FLAGS_GET_LINEAR(temp2->flags);

  /* If division test whether the denominator will ever be zero during
   * the common timespan */
  if (oper == DIV)
  {
    PeriodSet *ps = temporal_get_time_internal(temp1);
    Temporal *projtemp2 = temporal_restrict_periodset_internal(temp2, ps, REST_AT);
    if (projtemp2 == NULL)
      PG_RETURN_NULL();
    if (temporal_ever_eq_internal(projtemp2, Float8GetDatum(0.0)))
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Division by zero")));
  }

  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp1->basetypid;
  lfinfo.argtypid[1] = temp2->basetypid;
  lfinfo.restypid = base_oid_from_temporal(temptypid);
  lfinfo.reslinear = linear1 || linear2;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = (oper == MULT || oper == DIV) && linear1 && linear2 ?
    tpfunc : NULL;
  Temporal *result = tfunc_temporal_temporal(temp1, temp2, &lfinfo);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

PG_FUNCTION_INFO_V1(add_base_tnumber);
/**
 * Returns the temporal addition of the number and the temporal number
 */
PGDLLEXPORT Datum
add_base_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_base_tnumber(fcinfo, &datum_add, ADD);
}

PG_FUNCTION_INFO_V1(add_tnumber_base);
/**
 * Returns the temporal addition of the temporal number and the number
 */
PGDLLEXPORT Datum
add_tnumber_base(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_base(fcinfo, &datum_add, ADD);
}

PG_FUNCTION_INFO_V1(add_tnumber_tnumber);
/**
 * Returns the temporal addition of the temporal numbers
 */
PGDLLEXPORT Datum
add_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber(fcinfo, &datum_add, ADD, NULL);
}

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

PG_FUNCTION_INFO_V1(sub_base_tnumber);
/**
 * Returns the temporal subtraction of the number and the temporal number
 */
PGDLLEXPORT Datum
sub_base_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_base_tnumber(fcinfo, &datum_sub, SUB);
}

PG_FUNCTION_INFO_V1(sub_tnumber_base);
/**
 * Returns the temporal subtraction of the temporal number and the number
 */
PGDLLEXPORT Datum
sub_tnumber_base(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_base(fcinfo, &datum_sub, SUB);
}

PG_FUNCTION_INFO_V1(sub_tnumber_tnumber);
/**
 * Returns the temporal subtraction of the temporal numbers
 */
PGDLLEXPORT Datum
sub_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber(fcinfo, &datum_sub, SUB, NULL);
}

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

PG_FUNCTION_INFO_V1(mult_base_tnumber);
/**
 * Returns the temporal multiplication of the number and the temporal number
 */
PGDLLEXPORT Datum
mult_base_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_base_tnumber(fcinfo, &datum_mult, MULT);
}

PG_FUNCTION_INFO_V1(mult_tnumber_base);
/**
 * Returns the temporal multiplication of the temporal number and the number
 */
PGDLLEXPORT Datum
mult_tnumber_base(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_base(fcinfo, &datum_mult, MULT);
}

PG_FUNCTION_INFO_V1(mult_tnumber_tnumber);
/**
 * Returns the temporal multiplication of the temporal numbers
 */
PGDLLEXPORT Datum
mult_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber(fcinfo, &datum_mult, MULT,
    &tnumber_mult_tp_at_timestamp);
}

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

PG_FUNCTION_INFO_V1(div_base_tnumber);
/**
 * Returns the temporal division of the number and the temporal number
 */
PGDLLEXPORT Datum
div_base_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_base_tnumber(fcinfo, &datum_div, DIV);
}

PG_FUNCTION_INFO_V1(div_tnumber_base);
/**
 * Returns the temporal division of the temporal number and the number
 */
PGDLLEXPORT Datum
div_tnumber_base(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_base(fcinfo, &datum_div, DIV);
}

PG_FUNCTION_INFO_V1(div_tnumber_tnumber);
/**
 * Returns the temporal multiplication of the temporal numbers
 */
PGDLLEXPORT Datum
div_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber(fcinfo, &datum_div, DIV,
    &tnumber_div_tp_at_timestamp);
}

/*****************************************************************************
 * Miscellaneous temporal functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_round);
/**
 * Round the temporal number to the number of decimal places
 */
PGDLLEXPORT Datum
tnumber_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum digits = PG_GETARG_DATUM(1);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_float;
  lfinfo.numparam = 1;
  lfinfo.param[0] = digits;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp->basetypid;
  lfinfo.argtypid[1] = INT4OID;
  lfinfo.restypid = FLOAT8OID;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_degrees);
/**
 * Convert the temporal number from radians to degrees
 */
PGDLLEXPORT Datum
tnumber_degrees(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_degrees;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = temp->basetypid;
  lfinfo.restypid = FLOAT8OID;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Derivative functions
 *****************************************************************************/

/**
 * Returns the derivative of the temporal number
 * @pre The temporal number has linear interpolation
 */
static TSequence *
tnumberseq_derivative(const TSequence *seq)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));

  /* Instantaneous sequence */
  if (seq->count == 1)
    return NULL;

  /* General case */
  TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  Datum value1 = tinstant_value(inst1);
  double derivative;
  Oid basetypid = seq->basetypid;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i + 1);
    Datum value2 = tinstant_value(inst2);
    derivative = datum_eq(value1, value2, basetypid) ? 0.0 :
      (datum_double(value1, basetypid) - datum_double(value2, basetypid)) /
        ((double)(inst2->t - inst1->t) / 1000000);
    instants[i] = tinstant_make(Float8GetDatum(derivative), inst1->t,
      FLOAT8OID);
    inst1 = inst2;
    value1 = value2;
  }
  instants[seq->count - 1] = tinstant_make(Float8GetDatum(derivative),
    seq->period.upper, FLOAT8OID);
  /* The resulting sequence has step interpolation */
  TSequence *result = tsequence_make((const TInstant **) instants, seq->count,
    seq->period.lower_inc, seq->period.upper_inc, STEP, NORMALIZE);
  pfree_array((void **) instants, seq->count - 1);
  return result;
}

/**
 * Returns the derivative of the temporal number
 */
static TSequenceSet *
tnumberseqset_derivative(const TSequenceSet *ts)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (seq->count > 1)
      sequences[k++] = tnumberseq_derivative(seq);
  }
  /* The resulting sequence set has step interpolation */
  return tsequenceset_make_free(sequences, k, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tnumber_derivative);
/**
 * Returns the derivative of the temporal number
 */
PGDLLEXPORT Datum
tnumber_derivative(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = NULL;
  ensure_linear_interpolation(temp->flags);
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT || temp->subtype == INSTANTSET)
    ;
  else if (temp->subtype == SEQUENCE)
    result = (Temporal *)tnumberseq_derivative((TSequence *)temp);
  else /* temp->subtype == SEQUENCESET */
    result = (Temporal *)tnumberseqset_derivative((TSequenceSet *)temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
