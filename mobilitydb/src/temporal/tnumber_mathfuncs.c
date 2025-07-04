
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

/* PostgreSQL */
#include <postgres.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Generic arithmetic operator on a number and a temporal number
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
static Datum
Arithop_number_tnumber(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, meosType))
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = arithop_tnumber_number(temp, value, oper, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic arithmetic operator on a temporal number an a number
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Arithmetic function
 * @param[in] oper Enumeration that states the arithmetic operator
 */
static Datum
Arithop_tnumber_number(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, meosType))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Temporal *result = arithop_tnumber_number(temp, value, oper, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Generic arithmetic operator on two temporal numbers
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] oper Enumeration that states the arithmetic operator
 * @param[in] func Arithmetic function
 */
static Datum
Arithop_tnumber_tnumber(FunctionCallInfo fcinfo, TArithmetic oper,
  Datum (*func)(Datum, Datum, meosType))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  meosType basetype = temptype_basetype(temp1->temptype);
  assert(basetype == T_INT4 || basetype == T_FLOAT8);
  Temporal *result;
  if (basetype == T_FLOAT8 && (oper == MULT || oper == DIV))
    result = arithop_tnumber_tnumber(temp1, temp2, oper, func,
      &tfloat_arithop_turnpt);
  else
    result = arithop_tnumber_tnumber(temp1, temp2, oper, func, NULL);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

PGDLLEXPORT Datum Add_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of a number and a temporal number
 * @sqlfn tnumber_add()
 * @sqlop @p +
 */
inline Datum
Add_number_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_number_tnumber(fcinfo, ADD, &datum_add);
}

PGDLLEXPORT Datum Add_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of a temporal number and a number
 * @sqlfn tnumber_add()
 * @sqlop @p +
 */
inline Datum
Add_tnumber_number(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_number(fcinfo, ADD, &datum_add);
}

PGDLLEXPORT Datum Add_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Add_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal addition of two temporal numbers
 * @sqlfn tnumber_add()
 * @sqlop @p +
 */
inline Datum
Add_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_tnumber(fcinfo, ADD, &datum_add);
}

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

PGDLLEXPORT Datum Sub_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of a number and a temporal number
 * @sqlfn tnumber_sub()
 * @sqlop @p -
 */
inline Datum
Sub_number_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_number_tnumber(fcinfo, SUB, &datum_sub);
}

PGDLLEXPORT Datum Sub_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of a temporal number and a number
 * @sqlfn tnumber_sub()
 * @sqlop @p -
 */
inline Datum
Sub_tnumber_number(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_number(fcinfo, SUB, &datum_sub);
}

PGDLLEXPORT Datum Sub_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Sub_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal subtraction of two temporal numbers
 * @sqlfn tnumber_sub()
 * @sqlop @p -
 */
inline Datum
Sub_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_tnumber(fcinfo, SUB, &datum_sub);
}

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

PGDLLEXPORT Datum Mult_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mult_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of a number and a temporal number
 * @sqlfn tnumber_mult()
 * @sqlop @p *
 */
inline Datum
Mult_number_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_number_tnumber(fcinfo, MULT, &datum_mult);
}

PGDLLEXPORT Datum Mult_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mult_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of a temporal number and a number
 * @sqlfn tnumber_mult()
 * @sqlop @p *
 */
inline Datum
Mult_tnumber_number(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_number(fcinfo, MULT, &datum_mult);
}

PGDLLEXPORT Datum Mult_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mult_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of two temporal numbers
 * @sqlfn tnumber_mult()
 * @sqlop @p *
 */
inline Datum
Mult_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_tnumber(fcinfo, MULT, &datum_mult);
}

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

PGDLLEXPORT Datum Div_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal division of a number and a temporal number
 * @sqlfn tnumber_div()
 * @sqlop @p /
 */
inline Datum
Div_number_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_number_tnumber(fcinfo, DIV, &datum_div);
}

PGDLLEXPORT Datum Div_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal division of a temporal number and a number
 * @sqlfn tnumber_div()
 * @sqlop @p /
 */
inline Datum
Div_tnumber_number(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_number(fcinfo, DIV, &datum_div);
}

PGDLLEXPORT Datum Div_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Div_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the temporal multiplication of two temporal numbers
 * @sqlfn tnumber_div()
 * @sqlop @p /
 */
inline Datum
Div_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return Arithop_tnumber_tnumber(fcinfo, DIV, &datum_div);
}

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_abs(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_abs);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the absolute value of a temporal number
 * @sqlfn abs()
 */
Datum
Tnumber_abs(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnumber_abs(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnumber_delta_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_delta_value);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the delta value of a temporal number
 * @sqlfn deltaValue()
 */
Datum
Tnumber_delta_value(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnumber_delta_value(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloat_floor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_floor);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return a temporal number rounded down to the nearest integer
 * @sqlfn floor()
 */
Datum
Tfloat_floor(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_floor(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloat_ceil(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_ceil);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return a temporal number rounded up to the nearest integer
 * @sqlfn ceil()
 */
Datum
Tfloat_ceil(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_ceil(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Float_degrees(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Float_degrees);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return a number transformed from radians to degrees
 * @sqlfn degrees()
 */
Datum
Float_degrees(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  bool normalize = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    normalize = PG_GETARG_BOOL(1);
  PG_RETURN_FLOAT8(float_degrees(value, normalize));
}

PGDLLEXPORT Datum Tfloat_degrees(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_degrees);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return a temporal number transformed from radians to degrees
 * @sqlfn degrees()
 */
Datum
Tfloat_degrees(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool normalize = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    normalize = PG_GETARG_BOOL(1);
  Temporal *result = tfloat_degrees(temp, normalize);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloat_radians(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_radians);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return a temporal number transformed from degrees to radians
 * @sqlfn radians()
 */
Datum
Tfloat_radians(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_radians(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Float_angular_difference(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Float_angular_difference);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the angular difference, i.e., the smaller angle between the
 * two degree values
 * @sqlfn angularDifference()
 */
Datum
Float_angular_difference(PG_FUNCTION_ARGS)
{
  double degrees1 = PG_GETARG_FLOAT8(0);
  double degrees2 = PG_GETARG_FLOAT8(1);
  PG_RETURN_FLOAT8(float_angular_difference(degrees1, degrees2));
}

/*****************************************************************************
 * Derivative functions
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_derivative(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_derivative);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the derivative of a temporal number
 * @sqlfn derivative()
 */
Datum
Temporal_derivative(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = temporal_derivative(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Exponential function
 *****************************************************************************/

PGDLLEXPORT Datum Tfloat_exp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_exp);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the natural logarithm of a temporal number
 * @sqlfn ln()
 */
Datum
Tfloat_exp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_exp(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Logarithm functions
 *****************************************************************************/

PGDLLEXPORT Datum Tfloat_ln(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_ln);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the natural logarithm of a temporal number
 * @sqlfn ln()
 */
Datum
Tfloat_ln(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_ln(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tfloat_log10(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tfloat_log10);
/**
 * @ingroup mobilitydb_temporal_math
 * @brief Return the natural logarithm of a temporal number
 * @sqlfn ln()
 */
Datum
Tfloat_log10(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tfloat_log10(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
