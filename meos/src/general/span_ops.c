/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Operators for span types.
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
#include "general/type_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the minimum value of two span base values
 */
Datum
span_min_value(Datum l, Datum r, meosType type)
{
  assert(span_basetype(type));
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Min(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  else if (type == T_INT4)
    return Int32GetDatum(Min(DatumGetInt32(l), DatumGetInt32(r)));
  else if (type == T_INT8)
    return Int64GetDatum(Min(DatumGetInt64(l), DatumGetInt64(r)));
  else /* type == T_FLOAT8 */
    return Float8GetDatum(Min(DatumGetFloat8(l), DatumGetFloat8(r)));
}

/**
 * @brief Return the maximum value of two span base values
 */
Datum
span_max_value(Datum l, Datum r, meosType type)
{
  assert(span_basetype(type));
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Max(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  else if (type == T_INT4) /** xx **/
    return Int32GetDatum(Max(DatumGetInt32(l), DatumGetInt32(r)));
  else if (type == T_INT8)
    return Int64GetDatum(Max(DatumGetInt64(l), DatumGetInt64(r)));
  else /* type == T_FLOAT8 */
    return Float8GetDatum(Max(DatumGetFloat8(l), DatumGetFloat8(r)));
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a span contains a value.
 */
bool
contains_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s);
  assert(s->basetype == basetype);
  int cmp = datum_cmp(s->lower, d, basetype);
  if (cmp > 0 || (cmp == 0 && ! s->lower_inc))
    return false;

  cmp = datum_cmp(s->upper, d, basetype);
  if (cmp < 0 || (cmp == 0 && ! s->upper_inc))
    return false;

  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer span contains an integer.
 * @sqlop @p \@>
 */
bool
contains_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return contains_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer span contains a big integer.
 * @sqlop @p \@>
 */
bool
contains_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return contains_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float span contains a float.
 * @sqlop @p \@>
 */
bool
contains_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return contains_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a period contains a timestamp.
 * @sqlop @p \@>
 */
bool
contains_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first span contains the second one.
 * @sqlop @p \@>
 */
bool
contains_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

  int c1 = datum_cmp(s1->lower, s2->lower, s1->basetype);
  int c2 = datum_cmp(s1->upper, s2->upper, s1->basetype);
  if (
    (c1 < 0 || (c1 == 0 && (s1->lower_inc || ! s2->lower_inc))) &&
    (c2 > 0 || (c2 == 0 && (s1->upper_inc || ! s2->upper_inc)))
  )
    return true;
  return false;
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a value is contained in a span
 */
bool
contained_value_span(Datum d, meosType basetype, const Span *s)
{
  return contains_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer is contained in an integer span
 * @sqlop @p <@
 */
bool
contained_int_intspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return contains_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer is contained in a big integer span
 * @sqlop @p <@
 */
bool
contained_bigint_bigintspan(int64 i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return contains_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float is contained in a float span
 * @sqlop @p <@
 */
bool
contained_float_floatspan(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return contains_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a timestamp is contained in a period
 * @sqlop @p <@
 */
bool
contained_timestamp_period(TimestampTz t, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return contains_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first span is contained in the second one
 * @sqlop @p <@
 */
bool
contained_span_span(const Span *s1, const Span *s2)
{
  return contains_span_span(s2, s1);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if two spans overlap.
 * @sqlop @p &&
 */
bool
overlaps_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

  int cmp1 = datum_cmp(s1->lower, s2->upper, s1->basetype);
  int cmp2 = datum_cmp(s2->lower, s1->upper, s1->basetype);
  if (
    (cmp1 < 0 || (cmp1 == 0 && s1->lower_inc && s2->upper_inc)) &&
    (cmp2 < 0 || (cmp2 == 0 && s2->lower_inc && s1->upper_inc))
  )
    return true;
  return false;
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a span and a value are adjacent
 */
bool
adjacent_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  return adjacent_span_span(s, &s1);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer span and an integer are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return adjacent_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer span and a big integer are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return adjacent_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float span and a float are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return adjacent_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a period and a timestamp are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return adjacent_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if two spans are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (
    (datum_eq(s1->upper, s2->lower, s1->basetype) &&
      s1->upper_inc != s2->lower_inc) ||
    (datum_eq(s2->upper, s1->lower, s1->basetype) &&
      s2->upper_inc != s1->lower_inc) );
}

/*****************************************************************************
 * Strictly left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is strictly to the left of a span.
 */
bool
left_value_span(Datum d, meosType basetype, const Span *s)
{
  assert(s->basetype == basetype);
  int cmp = datum_cmp(d, s->lower, basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the left of an integer span.
 * @sqlop @p <<
 */
bool
left_int_intspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return left_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the left of a big integer
 * span.
 * @sqlop @p <<
 */
bool
left_bigint_bigintspan(int64 i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return left_value_span(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the left of a float span.
 * @sqlop @p <<
 */
bool
left_float_floatspan(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return left_value_span(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly before a period.
 * @sqlop @p <<
 */
bool
before_timestamp_period(TimestampTz t, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_value_span(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span is strictly to the left of a value.
 */
bool
left_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  int cmp = datum_cmp(s->upper, d, basetype);
  return (cmp < 0 || (cmp == 0 && ! s->upper_inc));
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span is strictly to the left of an integer.
 * @sqlop @p <<
 */
bool
left_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return left_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span is strictly to the left of a big integer.
 * @sqlop @p <<
 */
bool
left_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return left_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span is strictly to the left of a float.
 * @sqlop @p <<
 */
bool
left_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return left_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period is strictly before a timestamp.
 * @sqlop @p <<
 */
bool
before_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span is strictly to the left of the second one.
 * @sqlop @p <<
 */
bool
left_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

  int cmp = datum_cmp(s1->upper, s2->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || ! s2->lower_inc)));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is strictly to the right of a span.
 */
bool
right_value_span(Datum d, meosType basetype, const Span *s)
{
  return left_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the right of an integer span.
 * @sqlop @p >>
 */
bool
right_int_intspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return left_span_value(s, DatumGetInt32(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the right of a big
 * integer span.
 * @sqlop @p >>
 */
bool
right_bigint_bigintspan(int64 i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return left_span_value(s, DatumGetInt64(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the right of a float span.
 * @sqlop @p >>
 */
bool
right_float_floatspan(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return left_span_value(s, DatumGetFloat8(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly after a period.
 * @sqlop @p #>>
 */
bool
after_timestamp_period(TimestampTz t, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_span_value(s, DatumGetTimestampTz(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span is strictly to the right of a value
 */
bool
right_span_value(const Span *s, Datum d, meosType basetype)
{
  return left_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span is strictly to the right of an integer
 * @sqlop @p >>
 */
bool
right_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return left_value_span(DatumGetInt32(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span is strictly to the right of a big
 * integer
 * @sqlop @p >>
 */
bool
right_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return left_value_span(DatumGetInt64(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span is strictly to the right of a float.
 * @sqlop @p >>
 */
bool
right_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return left_value_span(DatumGetFloat8(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period is strictly after a timestamp.
 * @sqlop @p #>>
 */
bool
after_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return left_value_span(DatumGetTimestampTz(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span is strictly to right the of the second one.
 * @sqlop @p >>
 */
bool
right_span_span(const Span *s1, const Span *s2)
{
  return left_span_span(s2, s1);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a span.
 */
bool
overleft_value_span(Datum d, meosType basetype, const Span *s)
{
  assert(s->basetype == basetype);
  int cmp = datum_cmp(d, s->upper, basetype);
  return (cmp < 0 || (cmp == 0 && s->upper_inc));
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the right of an integer
 * span.
 * @sqlop @p &<
 */
bool
overleft_int_intspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return overleft_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a big
 * integer span.
 * @sqlop @p &<
 */
bool
overleft_bigint_bigintspan(int64 i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return overleft_value_span(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the right of a float span.
 * @sqlop @p &<
 */
bool
overleft_float_floatspan(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return overleft_value_span(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not after a period.
 * @sqlop @p &<#
 */
bool
overbefore_timestamp_period(TimestampTz t, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_value_span(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span does not extend to the right of a value.
 */
bool
overleft_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  /* Integer spans are canonicalized and thus their upper bound is exclusive.
   * Therefore, we cannot simply check that s->upper <= d */
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  return overleft_span_span(s, &s1);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span does not extend to the right of an integer.
 * @sqlop @p &<
 */
bool
overleft_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return overleft_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span does not extend to the right of a
 * big integer.
 * @sqlop @p &<
 */
bool
overleft_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return overleft_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span does not extend to the right of a float.
 * @sqlop @p &<
 */
bool
overleft_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return overleft_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period is not after a timestamp.
 * @sqlop @p &<#
 */
bool
overbefore_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overleft_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span does not extend to the right of the
 * second one.
 * @sqlop @p &<
 */
bool
overleft_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;
  int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || s2->upper_inc)));
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the left of a span.
 */
bool
overright_value_span(Datum d, meosType basetype, const Span *s)
{
  assert(s->basetype == basetype);
  int cmp = datum_cmp(s->lower, d, basetype);
  return (cmp < 0 || (cmp == 0 && s->lower_inc));
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the left of an integer
 * span.
 * @sqlop @p &>
 */
bool
overright_int_intspan(int i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return overright_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the left of a big integer span.
 * @sqlop @p &>
 */
bool
overright_bigint_bigintspan(int64 i, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;
  return overright_value_span(Int64GetDatum(i), T_INT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the left of a float span.
 * @sqlop @p &>
 */
bool
overright_float_floatspan(double d, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return overright_value_span(Float8GetDatum(d), T_FLOAT8, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not before a period.
 * @sqlop @p #&>
 */
bool
overafter_timestamp_period(TimestampTz t, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_value_span(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span does not extend to the left of a value.
 */
bool
overright_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  return datum_le(d, s->lower, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span does not extend to the left of an integer.
 * @sqlop @p &>
 */
bool
overright_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;
  return overright_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span does not extend to the left of a big integer.
 * @sqlop @p &>
 */
bool
overright_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||! ensure_same_span_basetype(s, T_INT8))
    return false;
  return overright_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span does not extend to the left of a float.
 * @sqlop @p &>
 */
bool
overright_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;
  return overright_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period is not before a timestamp.
 * @sqlop @p #&>
 */
bool
overafter_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;
  return overright_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span does not extend to the left of the second one.
 * @sqlop @p &>
 */
bool
overright_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return false;

  int cmp = datum_cmp(s2->lower, s1->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->lower_inc || s2->lower_inc)));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the bouding box union of two spans
 * @note The result of the function is always a span even if the spans do not
 * overlap
 * @sqlop @p +
 */
void
bbox_union_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  memset(result, 0, sizeof(Span));
  memcpy(result, s1, sizeof(Span));
  span_expand(s2, result);
  return;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of a span and a value
 * @sqlop @p +
 */
SpanSet *
union_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  SpanSet *result = union_span_span(s, &s1);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of an integer span and an integer
 * @sqlop @p +
 */
SpanSet *
union_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return NULL;
  return union_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a big integer span and a big integer
 * @sqlop @p +
 */
SpanSet *
union_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return NULL;
  return union_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a float span and a float
 * @sqlop @p +
 */
SpanSet *
union_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return NULL;
  return union_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a period and a timestamp
 * @sqlop @p +
 */
SpanSet *
union_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return union_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two spans
 * @sqlop @p +
 */
SpanSet *
union_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return NULL;

  /* If the spans do not overlap */
  if (! overlaps_span_span(s1, s2) &&
    !adjacent_span_span(s1, s2))
  {
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
    return spanset_make(spans, 2, NORMALIZE_NO);
  }

  /* Compute the union of the overlapping spans */
  Span s;
  memcpy(&s, s1, sizeof(Span));
  span_expand(s2, &s);
  SpanSet *result = span_to_spanset(&s);
  return result;
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the intersection of a span and a value
 */
bool
intersection_span_value(const Span *s, Datum d, meosType basetype,
  Datum *result)
{
  assert(s->basetype == basetype);
  if (! contains_span_value(s, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of an integer span and an integer in the
 * last argument
 * @sqlop @p *
 */
bool
intersection_intspan_int(const Span *s, int i, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return false;

  if (! contains_span_value(s, Int32GetDatum(i), T_INT4))
    return false;
  *result = i;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a big integer span and a big integer in
 * the last argument
 * @sqlop @p *
 */
bool
intersection_bigintspan_bigint(const Span *s, int64 i, int64 *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return false;

  if (! contains_span_value(s, Int64GetDatum(i), T_INT8))
    return false;
  *result = i;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a float span and a float in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_floatspan_float(const Span *s, double d, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;

  if (! contains_span_value(s, Float8GetDatum(d), T_FLOAT8))
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a period and a timestamp in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_period_timestamp(const Span *s, TimestampTz t,
  TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;

  if (! contains_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ))
    return false;
  *result = t;
  return true;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the intersection of two spans
 * @note This function is equivalent to @ref intersection_span_span without
 * memory allocation
 */
bool
inter_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1); assert(s2); assert(s1->spantype == s2->spantype);
  /* Bounding box test */
  if (! overlaps_span_span(s1, s2))
    return false;

  memset(result, 0, sizeof(Span));
  Datum lower = span_max_value(s1->lower, s2->lower, s1->basetype);
  Datum upper = span_min_value(s1->upper, s2->upper, s1->basetype);
  bool lower_inc = s1->lower == s2->lower ? s1->lower_inc && s2->lower_inc :
    ( lower == s1->lower ? s1->lower_inc : s2->lower_inc );
  bool upper_inc = s1->upper == s2->upper ? s1->upper_inc && s2->upper_inc :
    ( upper == s1->upper ? s1->upper_inc : s2->upper_inc );
  span_set(lower, upper, lower_inc, upper_inc, s1->basetype, result);
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two spans.
 * @sqlop @p *
 */
Span *
intersection_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return NULL;

  Span result;
  if (! inter_span_span(s1, s2, &result))
    return NULL;
  return span_copy(&result);
}

/*****************************************************************************
 * Set difference
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the difference of a value and a span
 */
bool
minus_value_span(Datum d, meosType basetype, const Span *s,
  Datum *result)
{
  assert(s->basetype == basetype);
  if (contains_span_value(s, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of an integer and an integer span in the last
 * argument
 * @sqlop @p -
 */
bool
minus_int_intspan(int i, const Span *s, int *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_span_basetype(s, T_INT4))
    return false;

  Datum v;
  bool found = minus_value_span(Int32GetDatum(i), T_INT4, s, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a big integer and a big integer span in the
 * last argument
 * @sqlop @p -
 */
bool
minus_bigint_bigintspan(int64 i, const Span *s, int64 *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_span_basetype(s, T_INT8))
    return false;

  Datum v;
  bool found = minus_value_span(Int64GetDatum(i), T_INT8, s, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a float and a float span
 * @sqlop @p -
 */
bool
minus_float_floatspan(double d, const Span *s, double *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_span_basetype(s, T_FLOAT8))
    return false;

  Datum v;
  bool found = minus_value_span(Float8GetDatum(d), T_FLOAT8, s, &v);
  *result = DatumGetFloat8(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a timestamp and a period
 * @sqlop @p -
 */
bool
minus_timestamp_period(TimestampTz t, const Span *s, TimestampTz *result)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) result) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return false;

  Datum v;
  bool res = minus_value_span(TimestampTzGetDatum(t), T_TIMESTAMPTZ, s, &v);
  *result = DatumGetTimestampTz(v);
  return res;
}
#endif /* MEOS */

/**
 * @brief Compute the difference of a span and a value (iterator function).
 */
int
minus_span_value_iter(const Span *s, Datum d, meosType basetype, Span *result)
{
  assert(s->basetype == basetype);
  if (! contains_span_value(s, d, basetype))
  {
    result[0] = *s;
    return 1;
  }

  /* Account for canonicalized spans */
  Datum upper = span_canon_upper(s);

  bool lowereq = datum_eq(s->lower, d, basetype);
  bool uppereq = datum_eq(upper, d, basetype);
  if (lowereq && uppereq)
    return 0;

  if (lowereq)
  {
    span_set(s->lower, s->upper, false, s->upper_inc, basetype, &result[0]);
    return 1;
  }

  if (uppereq)
  {
    span_set(s->lower, upper, s->lower_inc, false, basetype, &result[0]);
    return 1;
  }

  span_set(s->lower, d, s->lower_inc, false, basetype, &result[0]);
  span_set(d, s->upper, false, s->upper_inc, basetype, &result[1]);
  return 2;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a span and a value.
 */
SpanSet *
minus_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  Span spans[2];
  int count = minus_span_value_iter(s, d, basetype, spans);
  if (count == 0)
    return NULL;
  return spanset_make(spans, count, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an integer span and an integer
 * @sqlop @p -
 */
SpanSet *
minus_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return NULL;
  return minus_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a big integer span and a big integer
 * @sqlop @p -
 */
SpanSet *
minus_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return NULL;
  return minus_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a float span and a float
 * @sqlop @p -
 */
SpanSet *
minus_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return NULL;
  return minus_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a period and a timestamp
 * @sqlop @p -
 */
SpanSet *
minus_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return NULL;
  return minus_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @brief Compute the difference of two spans (iterator function).
 */
int
minus_span_span_iter(const Span *s1, const Span *s2, Span *result)
{
  SpanBound lower1, lower2, upper1, upper2;
  span_deserialize((const Span *) s1, &lower1, &upper1);
  span_deserialize((const Span *) s2, &lower2, &upper2);

  int cmp_l1l2 = span_bound_cmp(&lower1, &lower2);
  int cmp_l1u2 = span_bound_cmp(&lower1, &upper2);
  int cmp_u1l2 = span_bound_cmp(&upper1, &lower2);
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
      s1->basetype, &result[0]);
    span_set(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype, &result[1]);
    return 2;
  }

  /* Result is a span */
  /*
   * s1         |----|
   * s2  |----|
   * s2                 |----|
   * result      |----|
   */
  if (cmp_l1u2 > 0 || cmp_u1l2 < 0)
    result[0] = *s1;

  /*
   * s1           |-----|
   * s2               |----|
   * result       |---|
   */
  else if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    span_set(s1->lower, s2->lower, s1->lower_inc, !(s2->lower_inc),
      s1->basetype, &result[0]);
  /*
   * s1         |-----|
   * s2      |----|
   * result       |---|
   */
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    span_set(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype, &result[0]);
  return 1;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two spans.
 * @sqlop @p -
 */
SpanSet *
minus_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return NULL;

  Span spans[2];
  int count = minus_span_span_iter(s1, s2, spans);
  if (count == 0)
    return NULL;
  return spanset_make(spans, count, NORMALIZE_NO);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between two values as a double
 * @return On error return DBL_MAX
 */
double
distance_value_value(Datum l, Datum r, meosType type)
{
  assert(span_basetype(type));
  if (type == T_INT4)
    return (double) abs(DatumGetInt32(l) - DatumGetInt32(r));
  if (type == T_INT8)
    return (double) llabs(DatumGetInt64(l) - DatumGetInt64(r));
  if (type == T_FLOAT8)
    return fabs(DatumGetFloat8(l) - DatumGetFloat8(r));
  if (type == T_TIMESTAMPTZ)
    /* Distance in seconds if the base type is TimestampTz */
    return (double) (llabs((DatumGetTimestampTz(l) -
      DatumGetTimestampTz(r)))) / USECS_PER_SEC;
  meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
    "Unknown types for distance between values of type: %d", type);
  return DBL_MAX;
}

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a span and a value as a double
 */
double
distance_span_value(const Span *s, Datum d, meosType basetype)
{
  assert(s->basetype == basetype);
  /* If the span contains the value return 0 */
  if (contains_span_value(s, d, basetype))
    return 0.0;

  /* If the span is to the right of the value return the distance
   * between the value and the lower bound of the span
   *     d   [---- s ----] */
  if (right_span_value(s, d, basetype))
    return distance_value_value(d, s->lower, basetype);

  /* Account for canonicalized spans */
  Datum upper = span_canon_upper(s);

  /* If the span is to the left of the value return the distance
   * between the upper bound of the span and value
   *     [---- s ----]   d */
  return distance_value_value(upper, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an integer span and an integer
 * as a double
 * @return On error return -1.0
 * @sqlop @p <->
 */
double
distance_intspan_int(const Span *s, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT4))
    return -1.0;
  return distance_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a big integer span and a big integer
 * as a double
 * @return On error return -1.0
 * @sqlop @p <->
 */
double
distance_bigintspan_bigint(const Span *s, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_INT8))
    return -1.0;
  return distance_span_value(s, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a float span and a float.
 * @return On error return -1.0
 * @sqlop @p <->
 */
double
distance_floatspan_float(const Span *s, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_same_span_basetype(s, T_FLOAT8))
    return -1.0;
  return distance_span_value(s, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance in seconds between a period and a timestamp
 * as a double
 * @return On error return -1.0
 * @sqlop @p <->
 */
double
distance_period_timestamp(const Span *s, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) ||
      ! ensure_same_span_basetype(s, T_TIMESTAMPTZ))
    return -1.0;
  return distance_span_value(s, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two spans as a double
 * @return On error return -1.0
 * @sqlop @p <->
 */
double
distance_span_span(const Span *s1, const Span *s2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s1) || ! ensure_not_null((void *) s2) ||
      ! ensure_same_span_type(s1, s2))
    return -1.0;

  /* If the spans intersect return 0 */
  if (overlaps_span_span(s1, s2))
    return 0.0;

  /* Account for canonicalized spans */
  Datum upper1 = span_canon_upper(s1);
  Datum upper2 = span_canon_upper(s2);

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

/******************************************************************************/
