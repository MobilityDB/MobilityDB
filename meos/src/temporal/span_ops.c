/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <common/int.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include <pgtypes.h>

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the minimum value of two span base values
 */
Datum
span_min_value(Datum l, Datum r, MeosType type)
{
  assert(span_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(Min(DatumGetTimestampTz(l),
        DatumGetTimestampTz(r)));
    case T_DATE:
      return DateADTGetDatum(Min(DatumGetDateADT(l), DatumGetDateADT(r)));
    case T_INT4:
      return Int32GetDatum(Min(DatumGetInt32(l), DatumGetInt32(r)));
    case T_INT8:
      return Int64GetDatum(Min(DatumGetInt64(l), DatumGetInt64(r)));
    case T_FLOAT8:
      return Float8GetDatum(Min(DatumGetFloat8(l), DatumGetFloat8(r)));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown base type for minimum value function: %s",
        meostype_name(type));
    return 0;
  }
}

/**
 * @brief Return the maximum value of two span base values
 */
Datum
span_max_value(Datum l, Datum r, MeosType type)
{
  assert(span_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(Max(DatumGetTimestampTz(l),
        DatumGetTimestampTz(r)));
    case T_DATE:
      return DateADTGetDatum(Max(DatumGetDateADT(l), DatumGetDateADT(r)));
    case T_INT4:
      return Int32GetDatum(Max(DatumGetInt32(l), DatumGetInt32(r)));
    case T_INT8:
      return Int64GetDatum(Max(DatumGetInt64(l), DatumGetInt64(r)));
    case T_FLOAT8:
      return Float8GetDatum(Max(DatumGetFloat8(l), DatumGetFloat8(r)));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown base type for maximum value function: %s",
        meostype_name(type));
    return 0;
  }
}

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
  if (! ensure_not_nan(d))
    return false;
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
 * Contains
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a span contains a value
 * @param[in] s Span
 * @param[in] value Value
 */
bool
contains_span_value(const Span *s, Datum value)
{
  assert(s);
  int cmp = datum_cmp(s->lower, value, s->basetype);
  if (cmp > 0 || (cmp == 0 && ! s->lower_inc))
    return false;

  cmp = datum_cmp(s->upper, value, s->basetype);
  if (cmp < 0 || (cmp == 0 && ! s->upper_inc))
    return false;

  return true;
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #Contains_span_value()
 */
bool
contains_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false); 
  return contains_span_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if the first span contains the second one
 * @param[in] s1,s2 Spans
 */
bool
span_contains(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  int cmp1 = datum_cmp(s1->lower, s2->lower, s1->basetype);
  int cmp2 = datum_cmp(s1->upper, s2->upper, s1->basetype);
  if (
    (cmp1 < 0 || (cmp1 == 0 && (s1->lower_inc || ! s2->lower_inc))) &&
    (cmp2 > 0 || (cmp2 == 0 && (s1->upper_inc || ! s2->upper_inc))) )
    return true;
  return false;
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if the first span contains the second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Contains_span_span()
 */
bool
contains_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_contains(s1, s2);
}


/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a value is contained in a span
 * @param[in] value Value
 * @param[in] s Span
 */
inline bool
contained_value_span(Datum value, const Span *s)
{
  return contains_span_value(s, value);
}

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if the first span is contained in the second one
 * @param[in] s1,s2 Spans
 */
bool
span_contained(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  return span_contains(s2, s1);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if the first span is contained in the second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Contained_span_span()
 */
bool
contained_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_contained(s1, s2);
}

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
  if (! ensure_not_nan(d))
    return false;
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
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if two spans overlap
 * @param[in] s1,s2 Spans
 */
bool
span_overlaps(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  int cmp1 = datum_cmp(s1->lower, s2->upper, s1->basetype);
  int cmp2 = datum_cmp(s2->lower, s1->upper, s1->basetype);
  if (
    (cmp1 < 0 || (cmp1 == 0 && s1->lower_inc && s2->upper_inc)) &&
    (cmp2 < 0 || (cmp2 == 0 && s2->lower_inc && s1->upper_inc)) )
    return true;
  return false;
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if two spans overlap
 * @param[in] s1,s2 Spans
 * @csqlfn #Overlaps_span_span()
 */
bool
overlaps_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_overlaps(s1, s2);
}


/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if two spans overlap or are adjacent
 * @param[in] s1,s2 Spans
 * @note This function is used for avoiding normalization in span operations
 */
bool
ovadj_span_span(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  int cmp1 = datum_cmp(s1->lower, s2->upper, s1->basetype);
  int cmp2 = datum_cmp(s2->lower, s1->upper, s1->basetype);
  if (
    (cmp1 < 0 || (cmp1 == 0 && (s1->lower_inc || s2->upper_inc))) &&
    (cmp2 < 0 || (cmp2 == 0 && (s2->lower_inc || s1->upper_inc))) )
    return true;
  return false;
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a span and a value are adjacent
 * @param[in] s Span
 * @param[in] value Value
 */
bool
adjacent_span_value(const Span *s, Datum value)
{
  assert(s);
  Span s1;
  span_set(value, value, true, true, s->basetype, s->spantype, &s1);
  return adjacent_span_span(s, &s1);
}

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if two spans are adjacent
 * @param[in] s1,s2 Spans
 */
bool
span_adjacent(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  /*
   * Canonical span-adjacency rule: set-theoretic share-a-boundary.
   * Two spans are adjacent iff their closures meet at a single boundary value
   * (upper1 == lower2 or upper2 == lower1); the inclusive/exclusive bits are
   * NOT consulted. This is the geometric/topological adjacency -- a shared
   * boundary value is a measure-zero touch, not an overlap -- and the only
   * notion definable uniformly across the bounding-box families: TBox/STBox
   * spatial dimensions are closed continuous ranges with no inc/exc to carry,
   * so every span + bbox path shares this one definition. Discrete base types
   * (int, bigint, date) stay correct because they are canonicalized to
   * half-open [lower, upper+1) at construction (see span_set), so consecutive
   * values test as adjacent, e.g. intspan '[1,5]' -|- '[6,10]' = true; for a
   * continuous type a shared boundary is adjacency, e.g. floatspan
   * '[1,5]' -|- '[5,9]' = true.
   */
  return (datum_eq(s1->upper, s2->lower, s1->basetype) ||
          datum_eq(s2->upper, s1->lower, s1->basetype));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if two spans are adjacent
 * @param[in] s1,s2 Spans
 * @csqlfn #Adjacent_span_span()
 */
bool
adjacent_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_adjacent(s1, s2);
}

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
  if (! ensure_not_nan(d))
    return false;
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

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if an integer is adjacent to a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Adjacent_value_span()
 */
bool
adjacent_int_span(int i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, false);
  return adjacent_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a big integer is adjacent to a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Adjacent_value_span()
 */
bool
adjacent_bigint_span(int64 i, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, false);
  return adjacent_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a float is adjacent to a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Adjacent_value_span()
 */
bool
adjacent_float_span(double d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, false);
  if (! ensure_not_nan(d))
    return false;
  return adjacent_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a date is adjacent to a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Adjacent_value_span()
 */
bool
adjacent_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return adjacent_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a timestamp is adjacent to a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Adjacent_value_span()
 */
bool
adjacent_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return adjacent_span_value(s, TimestampTzGetDatum(t));
}

/*****************************************************************************
 * Same
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if two spans are equal in the common dimensions
 * @param[in] s1,s2 Spans
 */
bool
span_same(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  /* A span is a single dimension, so `same` (`~=`) reduces to span equality.
   * This is the base case of the bounding-box `same` family (#same_tbox_tbox(),
   * #same_stbox_stbox()), letting the temporal/span bounding-box wrappers
   * dispatch on `~=` uniformly instead of borrowing the equality (`=`)
   * function #span_eq(). */
  return span_eq(s1, s2);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if two spans are equal in the common dimensions
 * @param[in] s1,s2 Spans
 */
bool
same_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_same(s1, s2);
}


/*****************************************************************************
 * Left of
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the left of a span
 * @param[in] value Value
 * @param[in] s Span
 */
bool
left_value_span(Datum value, const Span *s)
{
  assert(s);
  int cmp = datum_cmp(value, s->lower, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span is to the left of a value
 * @param[in] s Span
 * @param[in] value Value
 */
bool
left_span_value(const Span *s, Datum value)
{
  assert(s);
  int cmp = datum_cmp(s->upper, value, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->upper_inc));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if the first span is to the left of the second one
 * @param[in] s1,s2 Spans
 */
bool
span_left(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  int cmp = datum_cmp(s1->upper, s2->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || ! s2->lower_inc)));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span is to the left of the second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Left_span_span(), #Before_span_span()
 */
bool
left_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_left(s1, s2);
}


/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if the first span is to the left and not adjacent to the
 * second one
 * @param[in] s1,s2 Spans
 * @note This function is used for avoiding normalization in span operations
 */
bool
lfnadj_span_span(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s1->upper, s2->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && ! s1->upper_inc && ! s2->lower_inc));
}

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
  if (! ensure_not_nan(d))
    return false;
  return left_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is before a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Before_value_span()
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
 * @csqlfn #Before_value_span()
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
  if (! ensure_not_nan(d))
    return false;
  return left_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is before a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Before_span_value()
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
 * @csqlfn #Before_span_value()
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the right of a span
 * @param[in] value Value
 * @param[in] s Span
 */
inline bool
right_value_span(Datum value, const Span *s)
{
  return left_span_value(s, value);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span is to the right of a value
 * @param[in] s Span
 * @param[in] value Value
 */
inline bool
right_span_value(const Span *s, Datum value)
{
  return left_value_span(value, s);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if the first span is to right the of the second one
 * @param[in] s1,s2 Spans
 */
bool
span_right(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  return span_left(s2, s1);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span is to right the of the second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Right_span_span(), #After_span_span()
 */
bool
right_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_right(s1, s2);
}

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
  return left_span_value(s, Int32GetDatum(i));
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
  return left_span_value(s, Int64GetDatum(i));
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
  if (! ensure_not_nan(d))
    return false;
  return left_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is after a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #After_value_span()
 */
bool
after_date_span(DateADT d, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a timestamptz is after a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #After_value_span()
 */
bool
after_timestamptz_span(TimestampTz t, const Span *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_span_value(s, TimestampTzGetDatum(t));
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
  return left_value_span(Int32GetDatum(i), s);
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
  return left_value_span(Int64GetDatum(i), s);
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
  if (! ensure_not_nan(d))
    return false;
  return left_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is after a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #After_span_value()
 */
bool
after_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, false);
  return left_value_span(DateADTGetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is after a timestamptz
 * @param[in] s Span
 * @param[in] t Value
 * @csqlfn #After_span_value()
 */
bool
after_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, false);
  return left_value_span(TimestampTzGetDatum(t), s);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a span
 * @param[in] value Value
 * @param[in] s Span
 */
bool
overleft_value_span(Datum value, const Span *s)
{
  assert(s);
  int cmp = datum_cmp(value, s->upper, s->basetype);
  return (cmp < 0 || (cmp == 0 && s->upper_inc));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span does not extend to the right of a value
 * @param[in] s Span
 * @param[in] value Value
 */
bool
overleft_span_value(const Span *s, Datum value)
{
  assert(s);
  /* Integer spans are canonicalized and thus their upper bound is exclusive.
   * Therefore, we cannot simply check that s->upper <= value */
  Span s1;
  span_set(value, value, true, true, s->basetype, s->spantype, &s1);
  return overleft_span_span(s, &s1);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if the first span does not extend to the right of the
 * second one
 * @param[in] s1,s2 Spans
 */
bool
span_overleft(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || s2->upper_inc)));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span does not extend to the right of the
 * second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Overleft_span_span(), #Overbefore_span_span()
 */
bool
overleft_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_overleft(s1, s2);
}

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
  if (! ensure_not_nan(d))
    return false;
  return overleft_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not after a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overbefore_value_span()
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
 * @csqlfn #Overbefore_value_span()
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
  if (! ensure_not_nan(d))
    return false;
  return overleft_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not after a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overbefore_span_value()
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
 * @csqlfn #Overbefore_span_value()
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the left of a span
 * @param[in] value Value
 * @param[in] s Span
 */
bool
overright_value_span(Datum value, const Span *s)
{
  assert(s);
  int cmp = datum_cmp(s->lower, value, s->basetype);
  return (cmp < 0 || (cmp == 0 && s->lower_inc));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span does not extend to the left of a value
 * @param[in] s Span
 * @param[in] value Value
 */
bool
overright_span_value(const Span *s, Datum value)
{
  assert(s);
  return datum_le(value, s->lower, s->basetype);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if the first span does not extend to the left of the
 * second one
 * @param[in] s1,s2 Spans
 */
bool
span_overright(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2);
  assert(ensure_valid_span_span(s1, s2));
  int cmp = datum_cmp(s2->lower, s1->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->lower_inc || s2->lower_inc)));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span does not extend to the left of the
 * second one
 * @param[in] s1,s2 Spans
 * @csqlfn #Overright_span_span(), #Overafter_span_span()
 */
bool
overright_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return false;
  return span_overright(s1, s2);
}

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
  if (! ensure_not_nan(d))
    return false;
  return overright_value_span(Float8GetDatum(d), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a date is not before a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Overafter_value_span()
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
 * @csqlfn #Overafter_value_span()
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
  if (! ensure_not_nan(d))
    return false;
  return overright_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is not before a date
 * @param[in] s Span
 * @param[in] d Value
 * @csqlfn #Overafter_span_value()
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
 * @csqlfn #Overafter_span_value()
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
 * @ingroup meos_internal_setspan_set
 * @brief Return in the last argument the bounding box union of two spans
 * @param[in] s1,s2 Spans
 * @param[out] result Resulting span
 * @note The result of the function is always a span even if the spans do not
 * overlap
 */
void
bbox_union_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  memcpy(result, s1, sizeof(Span));
  span_expand(s2, result);
  return;
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the bounding union of two spans
 * @param[in] s1,s2 Spans
 * @note The result of the function is always a span even if the spans do not
 * overlap
 * @note This function is similar to #bbox_union_span_span **with** memory
 * allocation
 */
Span *
super_union_span_span(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  Span *result = span_copy(s1);
  span_expand(s2, result);
  return result;
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a span and a value
 * @param[in] s Span
 * @param[in] value Value
 */
SpanSet *
union_span_value(const Span *s, Datum value)
{
  assert(s);
  Span s1;
  span_set(value, value, true, true, s->basetype, s->spantype, &s1);
  return union_span_span(s, &s1);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a span
 * @param[in] s Span
 * @param[in] value Value
 */
SpanSet *
union_value_span(Datum value, const Span *s)
{
  return union_span_value(s, value);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of two spans
 * @param[in] s1,s2 Spans
 * @csqlfn #Union_span_span()
 */
SpanSet *
union_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return NULL;

  /* If the spans overlap */
  if (ovadj_span_span(s1, s2))
  {
    /* Compute the union of the overlapping spans */
    Span s;
    memcpy(&s, s1, sizeof(Span));
    span_expand(s2, &s);
    return span_to_spanset(&s);
  }

  Span spans[2];
  if (datum_lt(s1->lower, s2->lower, s1->basetype))
  {
    spans[0] = *s1;
    spans[1] = *s2;
  }
  else
  {
    spans[0] = *s2;
    spans[1] = *s1;
  }
  return spanset_make_exp(spans, 2, 2, NORMALIZE_NO, ORDER_NO);
}

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
  VALIDATE_INTSPAN(s, NULL);
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
  VALIDATE_BIGINTSPAN(s, NULL);
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
union_bigint_span(int64 i, const Span *s)
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
  VALIDATE_FLOATSPAN(s, NULL);
  if (! ensure_not_nan(d))
    return NULL;
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
union_float_span(double d, const Span *s)
{
  if (! ensure_not_nan(d))
    return NULL;
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
  VALIDATE_DATESPAN(s, NULL);
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
union_date_span(DateADT d, const Span *s)
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
  VALIDATE_TSTZSPAN(s, NULL);
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
 * @ingroup meos_internal_setspan_set
 * @brief Return the intersection of a span and a value
 * @param[in] s Span
 * @param[in] value Value
 */
Span *
intersection_span_value(const Span *s, Datum value)
{
  assert(s);
  if (! contains_span_value(s, value))
    return NULL;
  return value_span(value, s->basetype);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a span
 * @param[in] value Value
 * @param[in] s Span
 */
Span *
intersection_value_span(Datum value, const Span *s)
{
  return intersection_span_value(s, value);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return in the last argument the intersection of two spans
 * @param[in] s1,s2 Spans
 * @param[out] result Resulting span
 * @note This function is equivalent to #intersection_span_span without
 * memory allocation
 */
bool
inter_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  /* Bounding box test */
  if (! overlaps_span_span(s1, s2))
    return false;

  Datum lower = span_max_value(s1->lower, s2->lower, s1->basetype);
  Datum upper = span_min_value(s1->upper, s2->upper, s1->basetype);
  bool lower_inc = s1->lower == s2->lower ? s1->lower_inc && s2->lower_inc :
    ( lower == s1->lower ? s1->lower_inc : s2->lower_inc );
  bool upper_inc = s1->upper == s2->upper ? s1->upper_inc && s2->upper_inc :
    ( upper == s1->upper ? s1->upper_inc : s2->upper_inc );
  span_set(lower, upper, lower_inc, upper_inc, s1->basetype, s1->spantype,
    result);
  return true;
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of two spans
 * @param[in] s1,s2 Spans
 * @csqlfn #Intersection_span_span()
 */
Span *
intersection_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return NULL;
  Span result;
  if (! inter_span_span(s1, s2, &result))
    return NULL;
  return span_copy(&result);
}

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
  VALIDATE_INTSPAN(s, NULL);
  return intersection_span_value(s, Int32GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of an integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Intersection_value_span()
 */
Span *
intersection_int_span(int i, const Span *s)
{
  return intersection_span_int(s, i);
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
  VALIDATE_BIGINTSPAN(s, NULL);
  return intersection_span_value(s, Int64GetDatum(i));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a big integer and a span
 * @param[in] i Value
 * @param[in] s Span
 * @csqlfn #Intersection_value_span()
 */
Span *
intersection_bigint_span(int64 i, const Span *s)
{
  return intersection_span_bigint(s, i);
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
  VALIDATE_FLOATSPAN(s, NULL);
  if (! ensure_not_nan(d))
    return NULL;
  return intersection_span_value(s, Float8GetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a float and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Intersection_value_span()
 */
Span *
intersection_float_span(double d, const Span *s)
{
  if (! ensure_not_nan(d))
    return NULL;
  return intersection_span_float(s, d);
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
  VALIDATE_DATESPAN(s, NULL);
  return intersection_span_value(s, DateADTGetDatum(d));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a date and a span
 * @param[in] d Value
 * @param[in] s Span
 * @csqlfn #Intersection_value_span()
 */
Span *
intersection_date_span(DateADT d, const Span *s)
{
  return intersection_span_date(s, d);
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
  VALIDATE_TSTZSPAN(s, NULL);
  return intersection_span_value(s, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a timestamptz and a span
 * @param[in] t Value
 * @param[in] s Span
 * @csqlfn #Intersection_value_span()
 */
Span *
intersection_timestamptz_span(TimestampTz t, const Span *s)
{
  return intersection_span_timestamptz(s, t);
}

/*****************************************************************************
 * Set difference
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a value and a span
 * @param[in] value Value
 * @param[in] s Span
 */
SpanSet *
minus_value_span(Datum value, const Span *s)
{
  assert(s);
  if (contains_span_value(s, value))
    return NULL;
  return value_spanset(value, s->basetype);
}

/**
 * @brief Return in the last argument the difference of a span and a value
 * @param[in] s Span
 * @param[in] value Value
 * @param[out] result Resulting span
 */
int
mi_span_value(const Span *s, Datum value, Span *result)
{
  assert(s);
  /* The span does not contain the value */
  if (! contains_span_value(s, value))
  {
    result[0] = *s;
    return 1;
  }

  /* Account for canonicalized spans */
  Datum upper = span_decr_bound(s->upper, s->basetype);
  bool lowereq = datum_eq(s->lower, value, s->basetype);
  bool uppereq = datum_eq(upper, value, s->basetype);
  /* The span is equal to the value */
  if (lowereq && uppereq)
    return 0;
  /* The value is equal to a bound */
  if (lowereq)
  {
    span_set(s->lower, s->upper, false, s->upper_inc, s->basetype, s->spantype,
      &result[0]);
    return 1;
  }
  if (uppereq)
  {
    span_set(s->lower, upper, s->lower_inc, false, s->basetype, s->spantype,
      &result[0]);
    return 1;
  }
  /* The span is split into two */
  span_set(s->lower, value, s->lower_inc, false, s->basetype, s->spantype,
    &result[0]);
  span_set(value, s->upper, false, s->upper_inc, s->basetype, s->spantype,
    &result[1]);
  return 2;
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a span and a value
 * @param[in] s Span
 * @param[in] value Value
 */
SpanSet *
minus_span_value(const Span *s, Datum value)
{
  assert(s);
  Span spans[2];
  int count = mi_span_value(s, value, spans);
  if (count == 0)
    return NULL;
  return spanset_make_exp(spans, count, count, NORMALIZE_NO, ORDER_NO);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return in the last argument the difference of two spans
 * @param[in] s1,s2 Spans
 * @param[out] result Result
 * @return Number of the elements in the output array
 */
int
mi_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1); assert(s2); assert(result);
  /* Result is a span
   * s1         |----|
   * s2  |----|   or    |----|
   * result     |----|
   */
  if (left_span_span(s1, s2) || left_span_span(s2, s1))
  {
    result[0] = *s1;
    return 1;
  }

  /* Deserialize the spans to minimize the number of comparisons */
  SpanBound lower1, lower2, upper1, upper2;
  span_deserialize((const Span *) s1, &lower1, &upper1);
  span_deserialize((const Span *) s2, &lower2, &upper2);
  int cmp_l1l2 = span_bound_cmp(&lower1, &lower2);
  int cmp_u1u2 = span_bound_cmp(&upper1, &upper2);

  /* Result is empty
   * s1         |----|
   * s2      |----------|
   */
  if (cmp_l1l2 >= 0 && cmp_u1u2 <= 0)
    return 0;

  /* Result is a span set
   * s1      |----------|
   * s2         |----|
   * result  |--|    |--|
   */
  if (cmp_l1l2 < 0 && cmp_u1u2 > 0)
  {
    span_set(s1->lower, s2->lower, s1->lower_inc, !(s2->lower_inc),
      s1->basetype, s1->spantype, &result[0]);
    span_set(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype, s1->spantype, &result[1]);
    return 2;
  }

  /* Result is a span
   * s1           |-----|
   * s2               |----|
   * result       |---|
   */
  if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    span_set(s1->lower, s2->lower, s1->lower_inc, !(s2->lower_inc),
      s1->basetype, s1->spantype, &result[0]);

  /* Result is a span
   * s1         |-----|
   * s2      |----|
   * result       |---|
   */
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    span_set(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype, s1->spantype, &result[0]);

  return 1;
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of two spans
 * @param[in] s1,s2 Spans
 * @csqlfn #Minus_span_span()
 */
SpanSet *
minus_span_span(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_span_span(s1, s2))
    return NULL;

  Span spans[2];
  int count = mi_span_span(s1, s2, spans);
  if (count == 0)
    return NULL;
  return spanset_make_exp(spans, count, count, NORMALIZE_NO, ORDER_NO);
}

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
  VALIDATE_INTSPAN(s, NULL);
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
  VALIDATE_BIGINTSPAN(s, NULL);
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
  if (! ensure_not_nan(d))
    return NULL;
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
  VALIDATE_TSTZSPAN(s, NULL);
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
  VALIDATE_INTSPAN(s, NULL);
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
  VALIDATE_BIGINTSPAN(s, NULL);
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
  VALIDATE_FLOATSPAN(s, NULL);
  if (! ensure_not_nan(d))
    return NULL;
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
  VALIDATE_DATESPAN(s, NULL);
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
  VALIDATE_TSTZSPAN(s, NULL);
  return minus_span_value(s, TimestampTzGetDatum(t));
}

/******************************************************************************
 * Distance functions
 ******************************************************************************/

/**
 * @brief Return the sentinel the distance functions of a base type answer
 * where they compute no distance
 * @param[in] type Type of the values
 * @details The sentinel is the maximum value of the type in which the distance
 * is expressed, so that it sorts after every distance that could be computed
 * in the nearest-neighbor searches performed with the indexes
 */
Datum
distance_sentinel(MeosType type)
{
  switch (type)
  {
    case T_INT4:
    case T_DATE:
      /* The distance between dates is expressed in days */
      return Int32GetDatum(INT_MAX);
    case T_INT8:
      return Int64GetDatum(INT64_MAX);
    case T_FLOAT8:
      return Float8GetDatum(DBL_MAX);
    case T_TIMESTAMPTZ:
      /* The distance between timestamptz values is an interval, and a missing
       * interval is the sentinel of a function answering one */
      return PointerGetDatum(NULL);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown base type for the distance sentinel: %s", meostype_name(type));
      return Float8GetDatum(DBL_MAX);
  }
}

/**
 * @brief Return a distance of a base type as a double for the indexes
 * @param[in] dist Distance between two values of the base type
 * @param[in] type Base type of the values the distance was computed on
 * @errval DBL_MAX
 * @details The distance functions answer in the type of the distance, which
 * is the base type for the numbers, the number of days for the dates, and the
 * number of seconds for the timestamptz values. The nearest neighbor searches
 * rank their candidates with a double whatever that type is, so the value is
 * converted here instead of being computed a second time
 */
double
distance_double(Datum dist, MeosType type)
{
  switch (type)
  {
    case T_INT4:
    case T_DATE:
      /* The distance between dates is expressed in days */
      return (double) DatumGetInt32(dist);
    case T_INT8:
      return (double) DatumGetInt64(dist);
    case T_FLOAT8:
      return DatumGetFloat8(dist);
    case T_TIMESTAMPTZ:
    {
      /* The distance between timestamptz values is an interval, read here in
       * seconds */
      const Interval *interv = DatumGetIntervalP(dist);
      return ((double) interv->month * DAYS_PER_MONTH * USECS_PER_DAY +
        (double) interv->day * USECS_PER_DAY + (double) interv->time) /
        USECS_PER_SEC;
    }
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown distance type for conversion to double: %s",
        meostype_name(type));
      return DBL_MAX;
  }
}

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two values
 * @param[in] l,r Values
 * @param[in] type Type of the values
 * @errval #distance_sentinel()
 */
Datum
distance_value_value(Datum l, Datum r, MeosType type)
{
  assert(span_basetype(type));
  switch (type)
  {
    case T_INT4:
    case T_DATE:
    {
      /* The distance between dates is expressed in days. The difference of
       * two integers is checked as PostgreSQL's int4mi checks it, since an
       * integer cannot hold every distance between two integers */
      int32 v1 = DatumGetInt32(l), v2 = DatumGetInt32(r), result;
      if (pg_sub_s32_overflow(Max(v1, v2), Min(v1, v2), &result))
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "integer out of range");
        return distance_sentinel(type);
      }
      return Int32GetDatum(result);
    }
    case T_INT8:
    {
      int64 v1 = DatumGetInt64(l), v2 = DatumGetInt64(r), result;
      if (pg_sub_s64_overflow(Max(v1, v2), Min(v1, v2), &result))
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "bigint out of range");
        return distance_sentinel(type);
      }
      return Int64GetDatum(result);
    }
    case T_FLOAT8:
      return Float8GetDatum(distance_value_value_double(l, r, type));
    case T_TIMESTAMPTZ:
    {
      /* The distance between timestamptz values is the interval PostgreSQL's
       * timestamptz_mi answers for the later one minus the earlier one */
      TimestampTz t1 = DatumGetTimestampTz(l), t2 = DatumGetTimestampTz(r);
      Interval *result = minus_timestamptz_timestamptz(Max(t1, t2),
        Min(t1, t2));
      return result ? IntervalPGetDatum(result) : distance_sentinel(type);
    }
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown types for distance between values: %s",
        meostype_name(type));
      return distance_sentinel(type);
  }
}

/**
 * @brief Return the distance between two values as a double
 * @param[in] l,r Values
 * @param[in] type Type of the values
 * @errval DBL_MAX
 * @details The distance is the one #distance_value_value returns, read as
 * #distance_double reads it. It is computed without overflow, so that the
 * indexes and the statistics, which rank and histogram the distances as
 * doubles, can ask for the distance between any two values, while an integer
 * of the base type cannot hold every distance between two of its values
 */
double
distance_value_value_double(Datum l, Datum r, MeosType type)
{
  assert(span_basetype(type));
  switch (type)
  {
    case T_INT4:
    case T_DATE:
      /* The distance between dates is expressed in days */
      return fabs((double) DatumGetInt32(l) - (double) DatumGetInt32(r));
    case T_INT8:
    case T_TIMESTAMPTZ:
    {
      /* The unsigned difference of two 64-bit integers does not overflow */
      int64 v1 = DatumGetInt64(l), v2 = DatumGetInt64(r);
      double result = (double) (v1 > v2 ? (uint64) v1 - (uint64) v2 :
        (uint64) v2 - (uint64) v1);
      /* The distance between timestamptz values is expressed in seconds */
      return (type == T_INT8) ? result : result / USECS_PER_SEC;
    }
    case T_FLOAT8:
      return fabs(DatumGetFloat8(l) - DatumGetFloat8(r));
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown types for distance between values: %s",
        meostype_name(type));
      return DBL_MAX;
  }
}

/**
 * @brief Return the zero distance between two values of a span base type
 * @details The zero of the integers, the dates and the floats is the null
 * Datum, while the zero of the timestamptz values is an interval
 * @param[in] type Type of the values
 */
static Datum
distance_zero(MeosType type)
{
  if (type == T_TIMESTAMPTZ)
    return IntervalPGetDatum(palloc0(sizeof(Interval)));
  return (Datum) 0;
}

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between a span and a value
 * @param[in] s Span
 * @param[in] value Value
 */
Datum
distance_span_value(const Span *s, Datum value)
{
  assert(s);
  /* If the span contains the value return 0 */
  if (contains_span_value(s, value))
    return distance_zero(s->basetype);

  /* If the span is to the right of the value return the distance
   * between the value and the lower bound of the span
   *     value   [---- s ----] */
  if (right_span_value(s, value))
    return distance_value_value(value, s->lower, s->basetype);

  /* Account for canonicalized spans */
  Datum upper = span_decr_bound(s->upper, s->basetype);

  /* If the span is to the left of the value return the distance
   * between the upper bound of the span and value
   *     [---- s ----]   value */
  return distance_value_value(upper, value, s->basetype);
}

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two spans
 * @param[in] s1,s2 Spans
 * @errval #distance_sentinel()
 * @csqlfn #Distance_span_span()
 */
Datum
distance_span_span(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  /* If the spans intersect return 0 */
  if (overlaps_span_span(s1, s2))
    return distance_zero(s1->basetype);

  /* Account for canonicalized spans */
  Datum upper1 = span_decr_bound(s1->upper, s1->basetype);
  Datum upper2 = span_decr_bound(s2->upper, s2->basetype);

  /* If the first span is to the left of the second one return the distance
   * between the upper bound of the first and lower bound of the second
   *     [---- s1 ----]   [---- s2 ----] */
  if (left_span_span(s1, s2))
    return distance_value_value(upper1, s2->lower, s1->basetype);

  /* If the first span is to the right of the second one return the distance
   * between the upper bound of the second and the lower bound of the first
   *     [---- s2 ----]   [---- s1 ----] */
  return distance_value_value(upper2, s1->lower, s1->basetype);
}

/**
 * @brief Return the distance between two spans as a double
 * @details The distance is the one #distance_span_span returns, read as
 * #distance_double reads it, for the indexes that rank their candidates with
 * a double, and is computed by #distance_value_value_double without overflow
 * @param[in] s1,s2 Spans
 */
double
distance_span_span_double(const Span *s1, const Span *s2)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  /* If the spans intersect return 0 */
  if (overlaps_span_span(s1, s2))
    return 0.0;

  /* Account for canonicalized spans */
  Datum upper1 = span_decr_bound(s1->upper, s1->basetype);
  Datum upper2 = span_decr_bound(s2->upper, s2->basetype);

  /* If the first span is to the left of the second one return the distance
   * between the upper bound of the first and lower bound of the second
   *     [---- s1 ----]   [---- s2 ----] */
  if (left_span_span(s1, s2))
    return distance_value_value_double(upper1, s2->lower, s1->basetype);

  /* If the first span is to the right of the second one return the distance
   * between the upper bound of the second and the lower bound of the first
   *     [---- s2 ----]   [---- s1 ----] */
  return distance_value_value_double(upper2, s1->lower, s1->basetype);
}

/******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and an integer as a double
 * @param[in] s Span
 * @param[in] i Value
 * @errval INT_MAX
 * @csqlfn #Distance_span_value() #Distance_value_span()
 */
int
distance_span_int(const Span *s, int i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, INT_MAX);
  return DatumGetInt32(distance_span_value(s, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and a big integer as a double
 * @param[in] s Span
 * @param[in] i Value
 * @errval INT64_MAX
 * @csqlfn #Distance_span_value() #Distance_value_span()
 */
int64
distance_span_bigint(const Span *s, int64 i)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s, INT64_MAX);
  return DatumGetInt64(distance_span_value(s, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and a float
 * @param[in] s Span
 * @param[in] d Value
 * @errval DBL_MAX
 * @csqlfn #Distance_span_value() #Distance_value_span()
 */
double
distance_span_float(const Span *s, double d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, DBL_MAX);
  if (! ensure_not_nan(d))
    return DBL_MAX;
  return DatumGetFloat8(distance_span_value(s, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a span and a date as a double
 * @param[in] s Span
 * @param[in] d Value
 * @errval INT_MAX
 * @csqlfn #Distance_span_value() #Distance_value_span()
 */
int
distance_span_date(const Span *s, DateADT d)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, INT_MAX);
  return DatumGetInt32(distance_span_value(s, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span and a timestamptz as an
 * interval
 * @param[in] s Span
 * @param[in] t Value
 * @errval NULL
 * @csqlfn #Distance_span_value() #Distance_value_span()
 */
Interval *
distance_span_timestamptz(const Span *s, TimestampTz t)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL);
  return DatumGetIntervalP(distance_span_value(s, TimestampTzGetDatum(t)));
}

/******************************************************************************/

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer spans
 * @param[in] s1,s2 Spans
 * @errval INT_MAX
 * @csqlfn #Distance_span_span()
 */
int
distance_intspan_intspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s1, INT_MAX); VALIDATE_INTSPAN(s2, INT_MAX);
  return DatumGetInt32(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer spans
 * @param[in] s1,s2 Spans
 * @errval INT64_MAX
 * @csqlfn #Distance_span_span()
 */
int64
distance_bigintspan_bigintspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPAN(s1, INT64_MAX); VALIDATE_BIGINTSPAN(s2, INT64_MAX);
  return DatumGetInt64(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two float spans
 * @param[in] s1,s2 Spans
 * @errval DBL_MAX
 * @csqlfn #Distance_span_span()
 */
double
distance_floatspan_floatspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s1, DBL_MAX); VALIDATE_FLOATSPAN(s2, DBL_MAX);
  return DatumGetFloat8(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two date spans
 * @param[in] s1,s2 Spans
 * @errval INT_MAX
 * @csqlfn #Distance_span_span()
 */
int
distance_datespan_datespan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s1, INT_MAX); VALIDATE_DATESPAN(s2, INT_MAX);
  return DatumGetInt32(distance_span_span(s1, s2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two timestamptz spans
 * @param[in] s1,s2 Spans
 * @errval NULL
 * @csqlfn #Distance_span_span()
 */
Interval *
distance_tstzspan_tstzspan(const Span *s1, const Span *s2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s1, NULL); VALIDATE_TSTZSPAN(s2, NULL);
  return DatumGetIntervalP(distance_span_span(s1, s2));
}

