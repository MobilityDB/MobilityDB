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
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two `Boolean` values stating whether the bounds are inclusive
 */

#include "temporal/span.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <utils/timestamp.h>
#include <common/hashfn.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
intspan_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return span_in(str, T_INTSPAN);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
bigintspan_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return span_in(str, T_BIGINTSPAN);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
floatspan_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return span_in(str, T_FLOATSPAN);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a date span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
datespan_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return span_in(str, T_DATESPAN);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a timestamptz span from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @return On error return @p NULL
 * @csqlfn #Span_in()
 */
Span *
tstzspan_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return span_in(str, T_TSTZSPAN);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of an integer span
 * @return On error return @p NULL
 * @param[in] s Span
 * @csqlfn #Span_out()
 */
char *
intspan_out(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, NULL);
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a big integer span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
bigintspan_out(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, NULL);
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a float span
 * @param[in] s Span
 * @param[in] maxdd Maximum number of decimal digits
 * @return On error return @p NULL
  * @csqlfn #Span_out()
*/
char *
floatspan_out(const Span *s, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, NULL);
  return span_out(s, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a date span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
datespan_out(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, NULL);
  return span_out(s, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a timestamtz span
 * @param[in] s Span
 * @return On error return @p NULL
 * @csqlfn #Span_out()
 */
char *
tstzspan_out(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  return span_out(s, 0);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_constructor
 * @brief Return an integer span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
intspan_make(int lower, int upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int32GetDatum(lower), Int32GetDatum(upper), lower_inc, upper_inc,
    T_INT4, T_INTSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a big integer span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
bigintspan_make(int64 lower, int64 upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Int64GetDatum(lower), Int64GetDatum(upper), lower_inc, upper_inc,
    T_INT8, T_BIGINTSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a float span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
floatspan_make(double lower, double upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(Float8GetDatum(lower), Float8GetDatum(upper), lower_inc, upper_inc,
    T_FLOAT8, T_FLOATSPAN, s);
  return s;
}

/**
 * @ingroup meos_setspan_constructor
 * @brief Return a date span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
datespan_make(DateADT lower, DateADT upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(DateADTGetDatum(lower), DateADTGetDatum(upper), lower_inc,
    upper_inc, T_DATE, T_DATESPAN, s);
  return s;
}
/**
 * @ingroup meos_setspan_constructor
 * @brief Return a timestamptz span from the bounds
 * @param[in] lower,upper Bounds
 * @param[in] lower_inc,upper_inc True when the bounds are inclusive
 * @csqlfn #Span_constructor()
 */
Span *
tstzspan_make(TimestampTz lower, TimestampTz upper, bool lower_inc,
  bool upper_inc)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = palloc(sizeof(Span));
  span_set(TimestampTzGetDatum(lower), TimestampTzGetDatum(upper), lower_inc,
    upper_inc, T_TIMESTAMPTZ, T_TSTZSPAN, s);
  return s;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert an integer into a span
 * @param[in] i Value
 * @csqlfn #Value_to_span()
 */
Span *
int_to_span(int i)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(Int32GetDatum(i), Int32GetDatum(i + 1), true, false, T_INT4,
    T_INTSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a big integer into a span
 * @param[in] i Value
 * @csqlfn #Value_to_span()
 */
Span *
bigint_to_span(int i)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(Int64GetDatum(i), Int64GetDatum(i + 1), true, false, T_INT8,
    T_BIGINTSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a float into a span
 * @param[in] d Value
 * @csqlfn #Value_to_span()
 */
Span *
float_to_span(double d)
{
  Span *result = palloc(sizeof(Span));
  span_set(Float8GetDatum(d), Float8GetDatum(d), true, true, T_FLOAT8,
    T_FLOATSPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a date into a span
 * @param[in] d Value
 * @csqlfn #Value_to_span()
 */
Span *
date_to_span(DateADT d)
{
  Span *result = palloc(sizeof(Span));
  /* Account for canonicalized spans */
  span_set(DateADTGetDatum(d), DateADTGetDatum(d + 1), true, false, T_DATE,
    T_DATESPAN, result);
  return result;
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a timestamptz into a span
 * @param[in] t Value
 * @csqlfn #Value_to_span()
 */
Span *
timestamptz_to_span(TimestampTz t)
{
  Span *result = palloc(sizeof(Span));
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, result);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @return On error return @p INT_MAX
 * @param[in] s Span
 * @csqlfn #Span_lower()
 */
int
intspan_lower(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, INT_MAX);
  return DatumGetInt32(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span
 * @param[in] s Span
 * @return On error return LONG_MAX
 * @csqlfn #Span_lower()
 */
int64
bigintspan_lower(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, LONG_MAX);
  return DatumGetInt64(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a float span
 * @param[in] s Span
 * @return On error return @p DBL_MAX
 * @csqlfn #Span_lower()
 */
double
floatspan_lower(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, DBL_MAX);
  return DatumGetFloat8(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a date span
 * @param[in] s Span
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Span_lower()
 */
DateADT
datespan_lower(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, DATEVAL_NOEND);
  return DateADTGetDatum(s->lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a timestamptz span
 * @param[in] s Span
 * @return On error return DT_NOEND
 * @csqlfn #Span_lower()
 */
TimestampTz
tstzspan_lower(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, DT_NOEND);
  return TimestampTzGetDatum(s->lower);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @param[in] s Span
 * @return On error return @p INT_MAX
 * @csqlfn #Span_upper()
 */
int
intspan_upper(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, INT_MAX);
  return Int32GetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span
 * @param[in] s Span
 * @return On error return LONG_MAX
 * @csqlfn #Span_upper()
 */
int64
bigintspan_upper(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, LONG_MAX);
  return Int64GetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a float span
 * @param[in] s Span
 * @return On error return @p DBL_MAX
 * @csqlfn #Span_upper()
 */
double
floatspan_upper(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, DBL_MAX);
  return DatumGetFloat8(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a date span
 * @param[in] s Span
 * @return On error return DATEVAL_NOEND
 * @csqlfn #Span_upper()
 */
DateADT
datespan_upper(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, DATEVAL_NOEND);
  return DateADTGetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a timestamptz span
 * @param[in] s Span
 * @return On error return DT_NOEND
 * @csqlfn #Span_upper()
 */
TimestampTz
tstzspan_upper(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, DT_NOEND);
  return TimestampTzGetDatum(s->upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the lower bound of a span is inclusive
 * @param[in] s Span
 * @csqlfn #Span_lower_inc()
 */
bool
span_lower_inc(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, false);
  return s->lower_inc;
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return true if the upper bound of a span is inclusive
 * @param[in] s Span
 * @csqlfn #Span_lower_inc()
 */
bool
span_upper_inc(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, false);
  return s->upper_inc;
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
int
intspan_width(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, -1);
  return Int32GetDatum(numspan_width(s));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a big integer span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
int64
bigintspan_width(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, -1);
  return Int64GetDatum(numspan_width(s));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a float span
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Numspan_width()
 */
double
floatspan_width(const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, -1.0);
  return DatumGetFloat8(numspan_width(s));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
intspan_shift_scale(const Span *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, NULL);
  return numspan_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
bigintspan_shift_scale(const Span *s, int64 shift, int64 width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, NULL);
  return numspan_shift_scale(s, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
floatspan_shift_scale(const Span *s, double shift, double width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, NULL);
  return numspan_shift_scale(s, Float8GetDatum(shift), Float8GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a date span shifted and/or scaled by the values
 * @param[in] s Span
 * @param[in] shift Value for shifting the bounds
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspan_shift(), #Numspan_scale(), #Numspan_shift_scale()
 */
Span *
datespan_shift_scale(const Span *s, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, NULL);
  return numspan_shift_scale(s, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/******************************************************************************/
