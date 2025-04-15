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
 * @brief Operators for span set types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/spanset.h"
#include "general/temporal.h"
#include "general/type_util.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Contains_spanset_value()
 */
bool
contains_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return contains_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Contains_spanset_value()
 */
bool
contains_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return contains_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Contains_spanset_value()
 */
bool
contains_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return contains_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Contains_spanset_value()
 */
bool
contains_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return contains_spanset_value(ss, DateADTGetDatum(d));
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if an integer is contained in a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Contained_value_spanset()
 */
bool
contained_int_spanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return contained_value_spanset(Int32GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a big integer is contained in a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Contained_value_spanset()
 */
bool
contained_bigint_spanset(int64 i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return contained_value_spanset(Int64GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a float is contained in a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Contained_value_spanset()
 */
bool
contained_float_spanset(double d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return contained_value_spanset(Float8GetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a date is contained in a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Contained_value_spanset()
 */
bool
contained_date_spanset(DateADT d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return contained_value_spanset(DateADTGetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a timestamptz is contained in a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Contained_value_spanset()
 */
bool
contained_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return contained_value_spanset(TimestampTzGetDatum(t), ss);
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and an integer are adjacent
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return adjacent_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a big integer are adjacent
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return adjacent_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a float are adjacent
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return adjacent_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a date are adjacent
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return adjacent_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a timestamptz are adjacent
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return adjacent_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Strictly left
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the left of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Left_value_spanset()
 */
bool
left_int_spanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return left_value_spanset(Int32GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the left of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Left_value_spanset()
 */
bool
left_bigint_spanset(int64 i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return left_value_spanset(Int64GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the left of a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Left_value_spanset()
 */
bool
left_float_spanset(double d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return left_value_spanset(Float8GetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is before a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Left_value_spanset()
 */
bool
before_date_spanset(DateADT d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return left_value_spanset(DateADTGetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is before a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Left_value_spanset()
 */
bool
before_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return left_value_spanset(TimestampTzGetDatum(t), ss);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the left of an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Left_spanset_value()
 */
bool
left_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return left_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the left of a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Left_spanset_value()
 */
bool
left_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return left_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the left of a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Left_spanset_value()
 */
bool
left_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return left_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is before a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Left_spanset_value()
 */
bool
before_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return left_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is before a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Left_spanset_value()
 */
bool
before_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return left_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer is to the right of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Right_value_spanset()
 */
bool
right_int_spanset(int i, const SpanSet *ss)
{
  return left_spanset_int(ss, i);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer is to the right of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Right_value_spanset()
 */
bool
right_bigint_spanset(int64 i, const SpanSet *ss)
{
  return left_spanset_bigint(ss, i);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float is to the right of a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Right_value_spanset()
 */
bool
right_float_spanset(double d, const SpanSet *ss)
{
  return left_spanset_float(ss, d);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is after a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Right_value_spanset()
 */
bool
after_date_spanset(DateADT d, const SpanSet *ss)
{
  return before_spanset_date(ss, d);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is after a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Right_value_spanset()
 */
bool
after_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  return before_spanset_timestamptz(ss, t);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the right of an integer.
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Right_spanset_value()
 */
bool
right_spanset_int(const SpanSet *ss, int i)
{
  return left_int_spanset(i, ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the right of a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Right_spanset_value()
 */
bool
right_spanset_bigint(const SpanSet *ss, int64 i)
{
  return left_bigint_spanset(i, ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the right of a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Right_spanset_value()
 */
bool
right_spanset_float(const SpanSet *ss, double d)
{
  return left_float_spanset(d, ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is after a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Right_spanset_value()
 */
bool
after_spanset_date(const SpanSet *ss, DateADT d)
{
  return before_date_spanset(d, ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is after a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Right_spanset_value()
 */
bool
after_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  return before_timestamptz_spanset(t, ss);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the right of an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Overleft_spanset_value()
 */
bool
overleft_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return overleft_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a big
 * integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Overleft_spanset_value()
 */
bool
overleft_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return overleft_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Overleft_spanset_value()
 */
bool
overleft_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return overleft_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is not after a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Overleft_spanset_value()
 */
bool
overbefore_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return overleft_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is not after a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Overleft_spanset_value()
 */
bool
overbefore_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return overleft_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the right of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Overleft_value_spanset()
 */
bool
overleft_int_spanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return overleft_value_spanset(Int32GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a span
 * set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Overleft_value_spanset()
 */
bool
overleft_bigint_spanset(int64 i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return overleft_value_spanset(Int64GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the right of a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Overleft_value_spanset()
 */
bool
overleft_float_spanset(double d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return overleft_value_spanset(Float8GetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not after a span set.
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Overleft_value_spanset()
 */
bool
overbefore_date_spanset(DateADT d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return overleft_value_spanset(DateADTGetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not after a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Overleft_value_spanset()
 */
bool
overbefore_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return overleft_value_spanset(TimestampTzGetDatum(t), ss);
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if an integer does not extend to the left of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Overright_value_spanset()
 */
bool
overright_int_spanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return overright_value_spanset(Int32GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a big integer does not extend to the left of a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Overright_value_spanset()
 */
bool
overright_bigint_spanset(int64 i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return overright_value_spanset(Int64GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a float does not extend to the left of a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Overright_value_spanset()
 */
bool
overright_float_spanset(double d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return overright_value_spanset(Float8GetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not before a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Overright_value_spanset()
 */
bool
overafter_date_spanset(DateADT d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return overright_value_spanset(DateADTGetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is not before a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Overright_value_spanset()
 */
bool
overafter_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return overright_value_spanset(TimestampTzGetDatum(t), ss);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the left of an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Overright_spanset_value()
 */
bool
overright_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, false);
  return overright_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Overright_spanset_value()
 */
bool
overright_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, false);
  return overright_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Overright_spanset_value()
 */
bool
overright_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, false);
  return overright_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is before a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Overright_spanset_value()
 */
bool
overafter_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, false);
  return overright_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is before a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Overright_spanset_value()
 */
bool
overafter_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, false);
  return overright_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_int_spanset(int i, SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return union_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_bigint_spanset(int64 i, SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return union_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_float_spanset(double d, SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return union_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_date_spanset(DateADT d, SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return union_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_timestamptz_spanset(TimestampTz t, SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return union_spanset_value(ss, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return union_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return union_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return union_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return union_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Union_spanset_value()
 */
SpanSet *
union_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return union_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Intersection_spanset_value()
 */
SpanSet *
intersection_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return intersection_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Intersection_spanset_value()
 */
SpanSet *
intersection_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return intersection_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Intersection_spanset_value()
 */
SpanSet *
intersection_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return intersection_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Intersection_spanset_value()
 */
SpanSet *
intersection_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return intersection_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Intersection_spanset_value()
 */
SpanSet *
intersection_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return intersection_spanset_value(ss, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of an integer and a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Minus_value_spanset()
 */
SpanSet *
minus_int_spanset(int i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return minus_value_spanset(Int32GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a big integer and a span set
 * @param[in] i Value
 * @param[in] ss Span set
 * @csqlfn #Minus_value_spanset()
 */
SpanSet *
minus_bigint_spanset(int64 i, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return minus_value_spanset(Int64GetDatum(i), ss);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a float and a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Minus_value_spanset()
 */
SpanSet *
minus_float_spanset(double d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return minus_value_spanset(Float8GetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a date and a span set
 * @param[in] d Value
 * @param[in] ss Span set
 * @csqlfn #Minus_value_spanset()
 */
SpanSet *
minus_date_spanset(DateADT d, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return minus_value_spanset(DateADTGetDatum(d), ss);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a timestamptz and a span set
 * @param[in] t Value
 * @param[in] ss Span set
 * @csqlfn #Minus_value_spanset()
 */
SpanSet *
minus_timestamptz_spanset(TimestampTz t, const SpanSet *ss)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return minus_value_spanset(TimestampTzGetDatum(t), ss);
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Minus_spanset_value()
 */
SpanSet *
minus_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL);
  return minus_spanset_value(ss, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @csqlfn #Minus_spanset_value()
 */
SpanSet *
minus_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL);
  return minus_spanset_value(ss, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Minus_spanset_value()
 */
SpanSet *
minus_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL);
  return minus_spanset_value(ss, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and a date
 * @param[in] ss Span set
 * @param[in] d Value
 * @csqlfn #Minus_spanset_value()
 */
SpanSet *
minus_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL);
  return minus_spanset_value(ss, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Minus_spanset_value()
 */
SpanSet *
minus_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL);
  return minus_spanset_value(ss, TimestampTzGetDatum(t));
}

/******************************************************************************
 * Distance functions
 ******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
int
distance_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, -1);
  return DatumGetInt32(distance_spanset_value(ss, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
int64
distance_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, -1);
  return DatumGetInt64(distance_spanset_value(ss, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
double
distance_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, -1.0);
  return DatumGetFloat8(distance_spanset_value(ss, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a span set and a date as a
 * double
 * @param[in] ss Span set
 * @param[in] d Value
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
int
distance_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, -1);
  return DatumGetInt32(distance_spanset_value(ss, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
double
distance_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, -1.0);
  return DatumGetFloat8(distance_spanset_value(ss, TimestampTzGetDatum(t)));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between an integer span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
int
distance_intspanset_intspan(const SpanSet *ss, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, -1); VALIDATE_INTSPAN(s, -1);
  return DatumGetInt32(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a big integer span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
int64
distance_bigintspanset_bigintspan(const SpanSet *ss, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, -1); VALIDATE_BIGINTSPAN(s, -1);
  return DatumGetInt64(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a float span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
double
distance_floatspanset_floatspan(const SpanSet *ss, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, -1.0); VALIDATE_INTSPAN(s, -1.0);
  return DatumGetFloat8(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a date span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
int
distance_datespanset_datespan(const SpanSet *ss, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, -1); VALIDATE_DATESPAN(s, -1);
  return DatumGetInt32(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a timestamptz span set and a
 * span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
double
distance_tstzspanset_tstzspan(const SpanSet *ss, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, -1.0); VALIDATE_TSTZSPAN(s, -1.0);
  return DatumGetFloat8(distance_spanset_span(ss, s));
}

/*****************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_spanset()
 */
int
distance_intspanset_intspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss1, -1); VALIDATE_INTSPANSET(ss2, -1);
  return DatumGetInt32(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_spanset()
 */
int64
distance_bigintspanset_bigintspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss1, -1); VALIDATE_BIGINTSPANSET(ss2, -1);
  return DatumGetInt64(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_spanset()
 */
double
distance_floatspanset_floatspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss1, -1.0); VALIDATE_FLOATSPANSET(ss2, -1.0);
  return DatumGetFloat8(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between two date span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_spanset()
 */
int
distance_datespanset_datespanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss1, -1); VALIDATE_DATESPANSET(ss2, -1);
  return DatumGetInt32(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between two timestamptz span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1.0
 * @csqlfn #Distance_spanset_spanset()
 */
double
distance_tstzspanset_tstzspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  VALIDATE_TSTZSPANSET(ss1, -1.0); VALIDATE_TSTZSPANSET(ss2, -1.0);
  return DatumGetFloat8(distance_spanset_spanset(ss1, ss2));
}

/******************************************************************************/
