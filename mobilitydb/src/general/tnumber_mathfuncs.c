
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
 * @brief Mathematical operators (+, -, *, /) and functions (round, degrees)
 * for temporal number.
 */

#include "pg_general/tnumber_mathfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include "general/lifting.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Generic arithmetic operator on a number an a temporal number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
static Datum
arithop_number_tnumber_ext(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType))
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Temporal *result = arithop_tnumber_number(temp, value, basetype, oper,
    func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Generic arithmetic operator on a temporal number an a number
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
static Datum
arithop_tnumber_number_ext(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Temporal *result = arithop_tnumber_number(temp, value, basetype, oper,
    func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Generic arithmetic operator on a temporal numbers
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] func Arithmetic function
 * @param[in] tpfunc Function determining the turning point
 */
static Datum
arithop_tnumber_tnumber_ext(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, mobdbType, mobdbType),
  bool (*tpfunc)(const TInstant *, const TInstant *, const TInstant *,
    const TInstant *, Datum *, TimestampTz *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = arithop_tnumber_tnumber(temp1, temp2, oper, func, tpfunc);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Add_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of a number and a temporal number
 * @sqlfunc tnumber_add()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Add_number_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_number_tnumber_ext(fcinfo, ADD, &datum_add);
}

PG_FUNCTION_INFO_V1(Add_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of a temporal number and a number
 * @sqlfunc tnumber_add()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Add_tnumber_number(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_number_ext(fcinfo, ADD, &datum_add);
}

PG_FUNCTION_INFO_V1(Add_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of the temporal numbers
 * @sqlfunc tnumber_add()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Add_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber_ext(fcinfo, ADD, &datum_add, NULL);
}

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Sub_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of a number and a temporal number
 * @sqlfunc tnumber_sub()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Sub_number_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_number_tnumber_ext(fcinfo, SUB, &datum_sub);
}

PG_FUNCTION_INFO_V1(Sub_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of a temporal number and a number
 * @sqlfunc tnumber_sub()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Sub_tnumber_number(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_number_ext(fcinfo, SUB, &datum_sub);
}

PG_FUNCTION_INFO_V1(Sub_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of the temporal numbers
 * @sqlfunc tnumber_sub()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Sub_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber_ext(fcinfo, SUB, &datum_sub, NULL);
}

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Mult_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of a number and a temporal number
 * @sqlfunc tnumber_mult()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Mult_number_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_number_tnumber_ext(fcinfo, MULT, &datum_mult);
}

PG_FUNCTION_INFO_V1(Mult_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of a temporal number and a number
 * @sqlfunc tnumber_mult()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Mult_tnumber_number(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_number_ext(fcinfo, MULT, &datum_mult);
}

PG_FUNCTION_INFO_V1(Mult_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of the temporal numbers
 * @sqlfunc tnumber_mult()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Mult_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber_ext(fcinfo, MULT, &datum_mult,
    &tnumber_mult_tp_at_timestamp);
}

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Div_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal division of a number and a temporal number
 * @sqlfunc tnumber_div()
 * @sqlop @p /
 */
PGDLLEXPORT Datum
Div_number_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_number_tnumber_ext(fcinfo, DIV, &datum_div);
}

PG_FUNCTION_INFO_V1(Div_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal division of a temporal number and a number
 * @sqlfunc tnumber_div()
 * @sqlop @p /
 */
PGDLLEXPORT Datum
Div_tnumber_number(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_number_ext(fcinfo, DIV, &datum_div);
}

PG_FUNCTION_INFO_V1(Div_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of the temporal numbers
 * @sqlfunc tnumber_div()
 * @sqlop @p /
 */
PGDLLEXPORT Datum
Div_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return arithop_tnumber_tnumber_ext(fcinfo, DIV, &datum_div,
    &tnumber_div_tp_at_timestamp);
}

/*****************************************************************************
 * Miscellaneous temporal functions
 *****************************************************************************/

/**
 * @brief Round a number to a given number of decimal places
 */
Datum
datum_round_float(Datum value, Datum prec)
{
  Datum result = value;
  double d = DatumGetFloat8(value);
  double inf = get_float8_infinity();
  if (d != -1 * inf && d != inf)
  {
    Datum number = call_function1(float8_numeric, value);
    Datum roundnumber = call_function2(numeric_round, number, prec);
    result = call_function1(numeric_float8, roundnumber);
  }
  return result;
}

/**
 * @brief Round a temporal number to a given number of decimal places
 */
Temporal *
tnumber_round(const Temporal *temp, Datum digits)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_round_float;
  lfinfo.numparam = 1;
  lfinfo.param[0] = digits;
  lfinfo.args = true;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_INT4;
  lfinfo.restype = T_TFLOAT;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

PG_FUNCTION_INFO_V1(Tnumber_round);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Round a temporal number to a given number of decimal places
 * @sqlfunc round()
 */
PGDLLEXPORT Datum
Tnumber_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum digits = PG_GETARG_DATUM(1);
  Temporal *result = tnumber_round(temp, digits);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tnumber_degrees);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Convert a temporal number from radians to degrees
 * @sqlfunc degrees()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Tnumber_degrees(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnumber_degrees(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Derivative functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_derivative);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the derivative of a temporal number
 * @sqlfunc derivative()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Tnumber_derivative(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnumber_derivative(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
