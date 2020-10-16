/*****************************************************************************
 *
 * tnumber_mathfuncs.c
 *  Temporal mathematical operators (+, -, *, /) and functions (round,
 *  degrees).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tnumber_mathfuncs.h"

#include <math.h>
#include <utils/builtins.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "lifting.h"

/*****************************************************************************
 * Mathematical functions on datums
 * As functions are static, there is no need to verify the validity of the
 * Oids passed as arguments as this has been done in the calling function.
 *****************************************************************************/

/**
 * Returns the addition of the two numbers
 */
static Datum
datum_add(Datum l, Datum r, Oid typel, Oid typer)
{
  Datum result = 0;
  if (typel == INT4OID && typer == INT4OID)
    result = Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = Float8GetDatum(DatumGetFloat8(l) + DatumGetInt32(r));
  else if (typel == FLOAT8OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
  return result;
}

/**
 * Returns the subtraction of the two numbers
 */
static Datum
datum_sub(Datum l, Datum r, Oid typel, Oid typer)
{
  Datum result = 0;
  if (typel == INT4OID && typer == INT4OID)
    result = Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetInt32(l) - DatumGetFloat8(r));
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = Float8GetDatum(DatumGetFloat8(l) - DatumGetInt32(r));
  else if (typel == FLOAT8OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
  return result;
}

/**
 * Returns the multiplication of the two numbers
 */
static Datum
datum_mult(Datum l, Datum r, Oid typel, Oid typer)
{
  Datum result = 0;
  if (typel == INT4OID && typer == INT4OID)
    result = Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetInt32(l) * DatumGetFloat8(r));
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = Float8GetDatum(DatumGetFloat8(l) * DatumGetInt32(r));
  else if (typel == FLOAT8OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
  return result;
}

/**
 * Returns the division of the two numbers
 */
static Datum
datum_div(Datum l, Datum r, Oid typel, Oid typer)
{
  Datum result = 0;
  if (typel == INT4OID && typer == INT4OID)
    result = Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetInt32(l) / DatumGetFloat8(r));
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = Float8GetDatum(DatumGetFloat8(l) / DatumGetInt32(r));
  else if (typel == FLOAT8OID && typer == FLOAT8OID)
    result = Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
  return result;
}

/**
 * Round the number to the number of decimal places
 */
Datum
datum_round(Datum value, Datum prec)
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
 * Find the single timestamptz at which the multiplication of two temporal
 * number segments is at a local minimum/maximum. The function supposes that
 * the instants are synchronized, that is  start1->t = start2->t and
 * end1->t = end2->t. The function only return an intersection at the middle,
 * that is, it returns false if the timestamp found is not at a bound.
 */
static bool
tnumberseq_mult_maxmin_at_timestamp(const TInstant *start1, const TInstant *end1,
  const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
  double x1 = datum_double(tinstant_value(start1), start1->valuetypid);
  double x2 = datum_double(tinstant_value(end1), start1->valuetypid);
  double x3 = datum_double(tinstant_value(start2), start2->valuetypid);
  double x4 = datum_double(tinstant_value(end2), start2->valuetypid);
  /* Compute the instants t1 and t2 at which the linear functions of the two
     segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
     minimum/maximum exactly at the middle between t1 and t2.
     To reduce problems related to floating point arithmetic, t1 and t2
     are shifted, respectively, to 0 and 1 before the computation */
  if ((x2 - x1) == 0.0 || (x4 - x3) == 0.0)
    return false;

  long double d1 = (-1 * x1) / (x2 - x1);
  long double d2 = (-1 * x3) / (x4 - x3);
  long double min = Min(d1, d2);
  long double max = Max(d1, d2);
  long double fraction = min + (max - min)/2;
  long double duration = (long double) (end1->t - start1->t);
  if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
    /* Minimum/maximum occurs out of the period */
    return false;

  *t = start1->t + (long) (duration * fraction);
  return true;
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
 * @param[in] valuetypid Oid of the base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */
Temporal *
arithop_tnumber_base1(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper,
  Temporal *temp, Datum value, Oid valuetypid, bool invert)
{
  ensure_tnumber_base_type(valuetypid);
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
      double d = datum_double(value, valuetypid);
      if (fabs(d) < EPSILON)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
          errmsg("Division by zero")));
    }
  }

  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = base_oid_from_temporal(temptypid);
  /* This parameter is not used for tnumber <op> base */
  lfinfo.reslinear = false;
  lfinfo.invert = invert;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_base(temp, value, valuetypid, (Datum) NULL, lfinfo);
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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = arithop_tnumber_base1(fcinfo, func, oper,
    temp, value, valuetypid, INVERT);
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = arithop_tnumber_base1(fcinfo, func, oper,
    temp, value, valuetypid, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Generic arithmetic operator on a temporal number an a number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] tpfunc Function determining the turning point
 */
static Datum
arithop_tnumber_tnumber(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid), TArithmetic oper,
  bool (*tpfunc)(const TInstant *, const TInstant *,
    const TInstant *, const TInstant *, TimestampTz *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
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
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 4;
  lfinfo.restypid = base_oid_from_temporal(temptypid);
  lfinfo.reslinear = linear1 || linear2;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfunc = (oper == MULT || oper == DIV) && linear1 && linear2 ?
    tpfunc : NULL;
  Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
    lfinfo);
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
    &tnumberseq_mult_maxmin_at_timestamp);
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
    &tnumberseq_mult_maxmin_at_timestamp);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tnumber_round);
/**
 * Round the temporal number to the number of decimal places
 */
PGDLLEXPORT Datum
tnumber_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum digits = PG_GETARG_DATUM(1);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &datum_round;
  lfinfo.numparam = 2;
  lfinfo.restypid = FLOAT8OID;
  Temporal *result = tfunc_temporal(temp, digits, lfinfo);
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  lfinfo.func = (varfunc) &datum_degrees;
  lfinfo.numparam = 1;
  lfinfo.restypid = FLOAT8OID;
  Temporal *result = tfunc_temporal(temp, (Datum) NULL, lfinfo);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
