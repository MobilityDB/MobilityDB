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
 * @brief Operators for span types
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/temporal.h"
#include "general/type_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Contains_span_value()
 */
bool
contains_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return contains_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Contains_span_value()
 */
bool
contains_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return contains_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Contains_span_value()
 */
bool
contains_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return contains_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Contains_span_value()
 */
bool
contains_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return contains_span_value(s, DateADTGetDatum(d));
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if an integer is contained in a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Contained_value_span()
 */
bool
contained_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return contains_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a big integer is contained in a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Contained_value_span()
 */
bool
contained_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return contains_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a float is contained in a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Contained_value_span()
 */
bool
contained_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return contains_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a date is contained in a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Contained_value_span()
 */
bool
contained_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return contains_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a timestamp is contained in a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Contained_value_span()
 */
bool
contained_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return contains_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and an integer are adjacent
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Adjacent_span_value()
 */
bool
adjacent_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return adjacent_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a big integer are adjacent
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Adjacent_span_value()
 */
bool
adjacent_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return adjacent_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a float are adjacent
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Adjacent_span_value()
 */
bool
adjacent_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return adjacent_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a date are adjacent
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Adjacent_span_value()
 */
bool
adjacent_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return adjacent_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a timestamptz are adjacent
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Adjacent_span_value()
 */
bool
adjacent_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return adjacent_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Left of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the left of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Left_value_span()
 */
bool
left_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return left_value_span(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the left of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Left_value_span()
 */
bool
left_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return left_value_span(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the left of a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Left_value_span()
 */
bool
left_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return left_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is before a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Left_value_span()
 */
bool
before_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_value_span(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is before a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Left_value_span()
 */
bool
before_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_value_span(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the left of an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Left_span_value()
 */
bool
left_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return left_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the left of a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Left_span_value()
 */
bool
left_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return left_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the left of a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Left_span_value()
 */
bool
left_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return left_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is before a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Left_span_value()
 */
bool
before_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_span_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is before a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Left_span_value()
 */
bool
before_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the right of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Right_value_span()
 */
bool
right_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return left_span_value(s, DatumGetInt32(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the right of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Right_value_span()
 */
bool
right_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return left_span_value(s, DatumGetInt64(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the right of a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Right_value_span()
 */
bool
right_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return left_span_value(s, DatumGetFloat8(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is after a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Right_value_span()
 */
bool
after_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_span_value(s, DatumGetDateADT(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is after a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Right_value_span()
 */
bool
after_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_span_value(s, DatumGetTimestampTz(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the right of an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Right_span_value()
 */
bool
right_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return left_value_span(DatumGetInt32(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the right of a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Right_span_value()
 */
bool
right_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return left_value_span(DatumGetInt64(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the right of a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Right_span_value()
 */
bool
right_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return left_value_span(DatumGetFloat8(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is after a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Right_span_value()
 */
bool
after_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_value_span(DatumGetDateADT(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is after a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Right_span_value()
 */
bool
after_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_value_span(DatumGetTimestampTz(t), s);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the right of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Overleft_value_span()
 */
bool
overleft_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return overleft_value_span(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Overleft_value_span()
 */
bool
overleft_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return overleft_value_span(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the right of a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overleft_value_span()
 */
bool
overleft_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return overleft_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not after a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overleft_value_span()
 */
bool
overbefore_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return overleft_value_span(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not after a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Overleft_value_span()
 */
bool
overbefore_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return overleft_value_span(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the right of an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Overleft_span_value()
 */
bool
overleft_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return overleft_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the right of a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Overleft_span_value()
 */
bool
overleft_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return overleft_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the right of a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overleft_span_value()
 */
bool
overleft_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return overleft_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not after a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overleft_span_value()
 */
bool
overbefore_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return overleft_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not after a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Overleft_span_value()
 */
bool
overbefore_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return overleft_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the left of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Overright_value_span()
 */
bool
overright_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return overright_value_span(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the left of a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Overright_value_span()
 */
bool
overright_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return overright_value_span(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the left of a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overright_value_span()
 */
bool
overright_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return overright_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not before a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overright_value_span()
 */
bool
overafter_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return overright_value_span(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not before a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Overright_value_span()
 */
bool
overafter_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return overright_value_span(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the left of an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Overright_span_value()
 */
bool
overright_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return overright_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the left of a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Overright_span_value()
 */
bool
overright_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return overright_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the left of a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overright_span_value()
 */
bool
overright_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return overright_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not before a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overright_span_value()
 */
bool
overafter_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return overright_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not before a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Overright_span_value()
 */
bool
overafter_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return overright_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Union_span_value()
 */
SpanSet *
union_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return union_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of an integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Union_value_span()
 */
SpanSet *
union_int_span(int i, const Span *s)
{
  return union_span_int(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Union_span_value()
 */
SpanSet *
union_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return union_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a big integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Union_value_span()
 */
SpanSet *
union_bigint_span(const Span *s, int64 i)
{
  return union_span_bigint(s, i);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Union_span_value()
 */
SpanSet *
union_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return union_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a float and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Union_value_span()
 */
SpanSet *
union_float_span(const Span *s, double d)
{
  return union_span_float(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Union_span_value()
 */
SpanSet *
union_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return union_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a date and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Union_value_span()
 */
SpanSet *
union_date_span(const Span *s, DateADT d)
{
  return union_span_date(s, d);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Union_span_value()
 */
SpanSet *
union_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return union_span_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a timestamptz and a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Union_value_span()
 */
SpanSet *
union_timestamptz_span(TimestampTz t, const Span *s)
{
  return union_span_timestamptz(s, t);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Intersection_span_value()
 */
Span *
intersection_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return intersection_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Intersection_span_value()
 */
Span *
intersection_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return intersection_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Intersection_span_value()
 */
Span *
intersection_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return intersection_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Intersection_span_value()
 */
Span *
intersection_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return intersection_span_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Intersection_span_value()
 */
Span *
intersection_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return intersection_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set difference
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of an integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Minus_value_span()
 */
SpanSet *
minus_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return minus_value_span(Int32GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a big integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Minus_value_span()
 */
SpanSet *
minus_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return minus_value_span(Int64GetDatum(i), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a float and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Minus_value_span()
 */
SpanSet *
minus_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, NULL);
  return minus_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a date and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Minus_value_span()
 */
SpanSet *
minus_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, NULL);
  return minus_value_span(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a timestamptz and a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Minus_value_span()
 */
SpanSet *
minus_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return minus_value_span(TimestampTzGetDatum(t), s);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and an integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Minus_span_value()
 */
SpanSet *
minus_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return minus_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and a big integer
 * @param[in] s Span
 * @param[in] i Value
 * @csqlfn #Minus_span_value()
 */
SpanSet *
minus_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return minus_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and a float
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Minus_span_value()
 */
SpanSet *
minus_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  return minus_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Minus_span_value()
 */
SpanSet *
minus_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return minus_span_value(s, DateADTGetDatum(d));
}
/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Minus_span_value()
 */
SpanSet *
minus_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return minus_span_value(s, TimestampTzGetDatum(t));
}

/******************************************************************************
 * Distance functions
 ******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and an integer as a double
 * @param[in] s Span
 * @param[in] i Value
 * @return On error return -1
 * @csqlfn #Distance_span_value()
 */
int
distance_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, -1);
  return distance_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and a big integer as a double
 * @param[in] s Span
 * @param[in] i Value
 * @return On error return -1
 * @csqlfn #Distance_span_value()
 */
int64
distance_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, -1);
  return distance_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and a float
 * @param[in] s Span
 * @param[in] d Value
 * @return On error return -1.0
 * @csqlfn #Distance_span_value()
 */
double
distance_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, -1.0);
  return distance_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a span and a date as a double
 * @param[in] s Span
 * @param[in] d Value
 * @return On error return -1
 * @csqlfn #Distance_span_value()
 */
int
distance_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, -1);
  return distance_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a span and a timestamptz as a
 * double
 * @param[in] s Span
 * @param[in] t Value
 * @return On error return -1.0
 * @csqlfn #Distance_span_value()
 */
double
distance_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, -1.0);
  return distance_span_value(s, TimestampTzGetDatum(t));
}

/******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer spans
 * @param[in] s1,s2 Spans
 * @return On error return -1
 * @csqlfn #Distance_span_span()
 */
int
distance_intspan_intspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s1, -1); VALIDATE_INTSPAN(s2, -1);
  return DatumGetInt32(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer spans
 * @param[in] s1,s2 Spans
 * @return On error return -1
 * @csqlfn #Distance_span_span()
 */
int64
distance_bigintspan_bigintspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s1, -1); VALIDATE_BIGINTSPAN(s2, -1);
  return DatumGetInt64(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float spans
 * @param[in] s1,s2 Spans
 * @return On error return -1.0
 * @csqlfn #Distance_span_span()
 */
double
distance_floatspan_floatspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s1, -1.0); VALIDATE_DATESPAN(s2, -1.0);
  return DatumGetFloat8(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two date spans
 * @param[in] s1,s2 Spans
 * @return On error return -1
 * @csqlfn #Distance_span_span()
 */
int
distance_datespan_datespan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s1, -1); VALIDATE_DATESPAN(s2, -1);
  return DatumGetInt32(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between two timestamptz spans
 * @param[in] s1,s2 Spans
 * @return On error return -1.0
 * @csqlfn #Distance_span_span()
 */
double
distance_tstzspan_tstzspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s1, -1.0); VALIDATE_TSTZSPAN(s2, -1.0);
  return DatumGetFloat8(distance_span_span(s1, s2));
}

/******************************************************************************/
