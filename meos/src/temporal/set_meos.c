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
 * @brief General functions for set values composed of an ordered list of
 * distinct values
 */

#include "temporal/set.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/varlena.h"
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/type_parser.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
intset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_INTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
bigintset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_BIGINTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
floatset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_FLOATSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
textset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_TEXTSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
dateset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_DATESET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Set_in()
 */
Set *
tstzset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return set_parse(&str, T_TSTZSET);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of an integer set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
intset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a big integer set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
bigintset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a float set
 * @param[in] s Set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Set_out()
 */
char *
floatset_out(const Set *s, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return set_out(s, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a text set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
textset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL);
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a date set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
dateset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return set_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the string representation of a timestamptz set
 * @param[in] s Set
 * @csqlfn #Set_out()
 */
char *
tstzset_out(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  return set_out(s, 0);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_constructor
 * @brief Return an integer set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
intset_make(const int *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Int32GetDatum(values[i]);
  return set_make_free(datums, count, T_INT4, ORDER);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a big integer set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
bigintset_make(const int64 *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Int64GetDatum(values[i]);
  return set_make_free(datums, count, T_INT8, ORDER);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a float set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
floatset_make(const double *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = Float8GetDatum(values[i]);
  return set_make_free(datums, count, T_FLOAT8, ORDER);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a text set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
textset_make(text **values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = PointerGetDatum(values[i]);
  return set_make_free(datums, count, T_TEXT, ORDER);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a date set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
dateset_make(const DateADT *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = DateADTGetDatum(values[i]);
  return set_make_free(datums, count, T_DATE, ORDER);
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a timestamptz set from an array of values
 * @param[in] values Array of values
 * @param[in] count Number of elements of the array
 * @csqlfn #Set_constructor()
 */
Set *
tstzset_make(const TimestampTz *values, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(values, NULL);
  if (! ensure_positive(count))
    return NULL;

  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; ++i)
    datums[i] = TimestampTzGetDatum(values[i]);
  return set_make_free(datums, count, T_TIMESTAMPTZ, ORDER);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert an integer into a set
 * @param[in] i Value
 * @csqlfn #Value_to_set()
 */
Set *
int_to_set(int i)
{
  Datum v = Int32GetDatum(i);
  return set_make_exp(&v, 1, 1, T_INT4, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a big integer into a set
 * @param[in] i Value
 * @csqlfn #Value_to_set()
 */
Set *
bigint_to_set(int64 i)
{
  Datum v = Int64GetDatum(i);
  return set_make_exp(&v, 1, 1, T_INT8, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a float into a set
 * @param[in] d Value
 * @csqlfn #Value_to_set()
 */
Set *
float_to_set(double d)
{
  Datum v = Float8GetDatum(d);
  return set_make_exp(&v, 1, 1, T_FLOAT8, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a text into a set
 * @param[in] txt Value
 * @csqlfn #Value_to_set()
 */
Set *
text_to_set(const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(txt, NULL);
  Datum v = PointerGetDatum(txt);
  return set_make_exp(&v, 1, 1, T_TEXT, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a date into a set
 * @param[in] d Value
 * @csqlfn #Value_to_set()
 */
Set *
date_to_set(DateADT d)
{
  Datum v = DateADTGetDatum(d);
  return set_make_exp(&v, 1, 1, T_DATE, ORDER_NO);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a timestamptz into a set
 * @param[in] t Value
 * @csqlfn #Value_to_set()
 */
Set *
timestamptz_to_set(TimestampTz t)
{
  Datum v = TimestampTzGetDatum(t);
  return set_make_exp(&v, 1, 1, T_TIMESTAMPTZ, ORDER_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of an integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_start_value()
 */
int
intset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, INT_MAX);
  return DatumGetInt32(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a big integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_start_value()
 */
int64
bigintset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, LONG_MAX);
  return DatumGetInt64(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a float set
 * @param[in] s Set
 * @return On error return @p DBL_MAX
 * @csqlfn #Set_start_value()
 */
double
floatset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, DBL_MAX);
  return DatumGetFloat8(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the start value of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_start_value()
 */
text *
textset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL);
  return DatumGetTextP(datum_copy(SET_VAL_N(s, 0), s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a date set
 * @param[in] s Set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Set_start_value()
 */
DateADT
dateset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, DATEVAL_NOEND);
  return DatumGetDateADT(SET_VAL_N(s, 0));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the start value of a timestamptz set
 * @param[in] s Set
 * @return On error return DT_NOEND
 * @csqlfn #Set_start_value()
 */
TimestampTz
tstzset_start_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, DT_NOEND);
  return DatumGetTimestampTz(SET_VAL_N(s, 0));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of an integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_end_value()
 */
int
intset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, INT_MAX);
  return DatumGetInt32(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a big integer set
 * @param[in] s Set
 * @return On error return @p INT_MAX
 * @csqlfn #Set_end_value()
 */
int64
bigintset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, LONG_MAX);
  return DatumGetInt64(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a float set
 * @param[in] s Set
 * @return On error return @p DBL_MAX
 * @csqlfn #Set_end_value()
 */
double
floatset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, DBL_MAX);
  return DatumGetFloat8(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return a copy of the end value of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_end_value()
 */
text *
textset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL);
  return DatumGetTextP(datum_copy(SET_VAL_N(s, s->count - 1), s->basetype));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a date set
 * @param[in] s Set
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Set_end_value()
 */
DateADT
dateset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, DATEVAL_NOEND);
  return DatumGetDateADT(SET_VAL_N(s, s->count - 1));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the end value of a timestamptz set
 * @param[in] s Set
 * @return On error return DT_NOEND
 * @csqlfn #Set_end_value()
 */
TimestampTz
tstzset_end_value(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, DT_NOEND);
  return DatumGetTimestampTz(SET_VAL_N(s, s->count - 1));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th value of an integer set
 * @param[in] s Integer set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
intset_value_n(const Set *s, int n, int *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetInt32(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th value of a big integer set
 * @param[in] s Integer set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
bigintset_value_n(const Set *s, int n, int64 *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetInt64(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th value of a float set
 * @param[in] s Float set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
floatset_value_n(const Set *s, int n, double *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetFloat8(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument a copy of the n-th value of a text set
 * @param[in] s Text set
 * @param[in] n Number (1-based)
 * @param[out] result Value
 * @return Return true if the value is found
 * @csqlfn #Set_value_n()
 */
bool
textset_value_n(const Set *s, int n, text **result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetTextP(datum_copy(SET_VAL_N(s, n - 1), s->basetype));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th value of a date set
 * @param[in] s Date set
 * @param[in] n Number (1-based)
 * @param[out] result Date
 * @return Return true if the date is found
 * @csqlfn #Set_value_n()
 */
bool
dateset_value_n(const Set *s, int n, DateADT *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetDateADT(SET_VAL_N(s, n - 1));
  return true;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return in the last argument the n-th value of a timestamptz set
 * @param[in] s Timestamptz set
 * @param[in] n Number (1-based)
 * @param[out] result Timestamptz
 * @return Return true if the timestamptz is found
 * @csqlfn #Set_value_n()
 */
bool
tstzset_value_n(const Set *s, int n, TimestampTz *result)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, false); VALIDATE_NOT_NULL(result, false);
  if (n < 1 || n > s->count)
    return false;
  *result = DatumGetTimestampTz(SET_VAL_N(s, n - 1));
  return true;
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of an integer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
int *
intset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  int *result = palloc(sizeof(int) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt32(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a big integer set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
int64 *
bigintset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  int64 *result = palloc(sizeof(int64) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetInt64(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a float set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
double *
floatset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  double *result = palloc(sizeof(double) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetFloat8(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of copies of the values of a text set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
text **
textset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL);
  text **result = palloc(sizeof(text *) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetTextP(datum_copy(SET_VAL_N(s, i), s->basetype));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a date set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
DateADT *
dateset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  DateADT *result = palloc(sizeof(DateADT) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetDateADT(SET_VAL_N(s, i));
  return result;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the array of values of a timestamptz set
 * @param[in] s Set
 * @return On error return @p NULL
 * @csqlfn #Set_values()
 */
TimestampTz *
tstzset_values(const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSET(s, NULL);
  TimestampTz *result = palloc(sizeof(TimestampTz) * s->count);
  for (int i = 0; i < s->count; i++)
    result[i] = DatumGetTimestampTz(SET_VAL_N(s, i));
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return the concatenation of a text and a text set
 * @param[in] txt Text
 * @param[in] s Set
 * @csqlfn #Textcat_text_textset()
 */
Set *
textcat_text_textset(const text *txt, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return textcat_textset_text_common(s, txt, INVERT);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return the concatenation of a text set and a text
 * @param[in] s Set
 * @param[in] txt Text
 * @csqlfn #Textcat_textset_text()
 */
Set *
textcat_textset_text(const Set *s, const text *txt)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TEXTSET(s, NULL); VALIDATE_NOT_NULL(txt, NULL);
  return textcat_textset_text_common(s, txt, INVERT_NO);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
intset_shift_scale(const Set *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSET(s, NULL);
  return numset_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
bigintset_shift_scale(const Set *s, int64 shift, int64 width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSET(s, NULL);
  return numset_shift_scale(s, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
floatset_shift_scale(const Set *s, double shift, double width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSET(s, NULL);
  return numset_shift_scale(s, Float8GetDatum(shift), Float8GetDatum(width),
    hasshift, haswidth);
}
/**
 * @ingroup meos_setspan_transf
 * @brief Return a date set shifted and/or scaled by two values
 * @param[in] s Set
 * @param[in] shift Value for shifting the elements
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numset_shift(), #Numset_scale(), #Numset_shift_scale(),
 */
Set *
dateset_shift_scale(const Set *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESET(s, NULL);
  return numset_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/*****************************************************************************/
