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
 * @brief General functions for set of disjoint spans
 */

#include "temporal/spanset.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_parser.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return an integer span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
intspanset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return spanset_parse(&str, T_INTSPANSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a big integer span from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
bigintspanset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return spanset_parse(&str, T_BIGINTSPANSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a float span from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
floatspanset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return spanset_parse(&str, T_FLOATSPANSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a date set from its Well-Known Text (WKT) representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
datespanset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return spanset_parse(&str, T_DATESPANSET);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a timestamptz set from its Well-Known Text (WKT)
 * representation
 * @param[in] str String
 * @csqlfn #Spanset_in()
 */
SpanSet *
tstzspanset_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return spanset_parse(&str, T_TSTZSPANSET);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of an integer span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
intspanset_out(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a big integer span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
bigintspanset_out(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a float span set
 * @param[in] ss Span set
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Spanset_out()
 */
char *
floatspanset_out(const SpanSet *ss, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return spanset_out(ss, maxdd);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a date span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
datespanset_out(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return spanset_out(ss, 0);
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a timpespantz span set
 * @param[in] ss Span set
 * @csqlfn #Spanset_out()
 */
char *
tstzspanset_out(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return spanset_out(ss, 0);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert an integer into a span set
 * @param[in] i Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
int_to_spanset(int i)
{
  return value_spanset(i, T_INT4);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a big integer into a span set
 * @param[in] i Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
bigint_to_spanset(int i)
{
  return value_spanset(i, T_INT8);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a float into a span set
 * @param[in] d Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
float_to_spanset(double d)
{
  return value_spanset(d, T_FLOAT8);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a date into a span set
 * @param[in] d Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
date_to_spanset(DateADT d)
{
  return value_spanset(d, T_DATE);
}

/**
 * @ingroup meos_setspan_conversion
 * @brief Convert a timestamptz into a span set
 * @param[in] t Value
 * @csqlfn #Value_to_spanset()
 */
SpanSet *
timestamptz_to_spanset(TimestampTz t)
{
  return value_spanset(t, T_TIMESTAMPTZ);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_transf
 * @brief Return an integer span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
intspanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return numspanset_shift_scale(ss, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a big integer span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
bigintspanset_shift_scale(const SpanSet *ss, int64 shift, int64 width,
  bool hasshift, bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return numspanset_shift_scale(ss, Int64GetDatum(shift), Int64GetDatum(width),
    hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a float span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
floatspanset_shift_scale(const SpanSet *ss, double shift, double width,
  bool hasshift, bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return numspanset_shift_scale(ss, Float8GetDatum(shift),
    Float8GetDatum(width), hasshift, haswidth);
}

/**
 * @ingroup meos_setspan_transf
 * @brief Return a date span shifted and/or scaled by two values
 * @param[in] ss Span set
 * @param[in] shift Value for shifting the span set
 * @param[in] width Width of the result
 * @param[in] hasshift True when the shift argument is given
 * @param[in] haswidth True when the width argument is given
 * @csqlfn #Numspanset_shift(), #Numspanset_scale(), #Numspanset_shift_scale()
 */
SpanSet *
datespanset_shift_scale(const SpanSet *ss, int shift, int width, bool hasshift,
  bool haswidth)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return numspanset_shift_scale(ss, Int32GetDatum(shift), Int32GetDatum(width),
    hasshift, haswidth);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_lower()
 */
int
intspanset_lower(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, INT_MAX);
  return DatumGetInt32(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of an integer span set
 * @param[in] ss Span set
 * @return On error return LONG_MAX
 * @csqlfn #Spanset_lower()
 */
int64
bigintspanset_lower(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, LONG_MAX);
  return DatumGetInt64(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a float span set
 * @param[in] ss Span set
 * @return On error return @p DBL_MAX
 * @csqlfn #Spanset_lower()
 */
double
floatspanset_lower(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, DBL_MAX);
  return Float8GetDatum(ss->elems[0].lower);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the lower bound of a timestamptz span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Spanset_lower()
 */
TimestampTz
tstzspanset_lower(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, DT_NOEND);
  return TimestampTzGetDatum(ss->elems[0].lower);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @param[in] ss Span set
 * @return On error return @p INT_MAX
 * @csqlfn #Spanset_upper()
 */
int
intspanset_upper(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, INT_MAX);
  return Int32GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of an integer span set
 * @param[in] ss Span set
 * @return On error return LONG_MAX
 * @csqlfn #Spanset_upper()
 */
int64
bigintspanset_upper(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, LONG_MAX);
  return Int64GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a float span set
 * @param[in] ss Span set
 * @return On error return @p DBL_MAX
 * @csqlfn #Spanset_upper()
 */
double
floatspanset_upper(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, DBL_MAX);
  return Float8GetDatum(ss->elems[ss->count - 1].upper);
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the upper bound of a timestamptz span set
 * @param[in] ss Span set
 * @return On error return DT_NOEND
 * @csqlfn #Spanset_upper()
 */
TimestampTz
tstzspanset_upper(const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, DT_NOEND);
  return TimestampTzGetDatum(ss->elems[ss->count - 1].upper);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
int
intspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, -1);
  return Int32GetDatum(numspanset_width(ss, boundspan));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of an integer span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
int64
bigintspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, -1);
  return Int64GetDatum(numspanset_width(ss, boundspan));
}

/**
 * @ingroup meos_setspan_accessor
 * @brief Return the width of a float span set
 * @param[in] ss Span
 * @param[in] boundspan True when the potential time gaps are ignored
 * @return On error return -1
 * @csqlfn #Numspanset_width(()
 */
double
floatspanset_width(const SpanSet *ss, bool boundspan)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, -1.0);
  return DatumGetFloat8(numspanset_width(ss, boundspan));
}

/*****************************************************************************/

