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
 * @brief Operators for span set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_tile.h"
#include "general/type_util.h"

/*****************************************************************************
 * Time precision functions for time values
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_transf
 * @brief Set the precision of a timestamp according to time buckets.
 * @param[in] t Time value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
TimestampTz
timestamp_tprecision(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  ensure_valid_duration(duration);
  TimestampTz result = timestamptz_bucket(t, duration, torigin);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Set the precision of a period according to period buckets.
 * @param[in] s Time value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
Span *
period_tprecision(const Span *s, const Interval *duration, TimestampTz torigin)
{
  assert(s->basetype == T_TIMESTAMPTZ);
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(s->lower);
  TimestampTz upper = DatumGetTimestampTz(s->upper);
  TimestampTz lower_bucket = timestamptz_bucket(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz upper_bucket = timestamptz_bucket(upper, duration, torigin) +
    tunits;
  Span *result = span_make(TimestampTzGetDatum(lower_bucket),
    TimestampTzGetDatum(upper_bucket), true, false, T_TIMESTAMPTZ);
  return result;
}

/**
 * @ingroup libmeos_setspan_transf
 * @brief Set the precision of a period set according to period buckets.
 * @param[in] ss Time value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 */
SpanSet *
periodset_tprecision(const SpanSet *ss, const Interval *duration,
  TimestampTz torigin)
{
  assert(ss->basetype == T_TIMESTAMPTZ);
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  TimestampTz lower = DatumGetTimestampTz(ss->span.lower);
  TimestampTz upper = DatumGetTimestampTz(ss->span.upper);
  TimestampTz lower_bucket = timestamptz_bucket(lower, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz upper_bucket = timestamptz_bucket(upper, duration, torigin) +
    tunits;
  /* Number of buckets */
  int count = (int) (((int64) upper_bucket - (int64) lower_bucket) / tunits);
  Span *spans = palloc(sizeof(Span) * count);
  lower = lower_bucket;
  upper = lower_bucket + tunits;
  int nspans = 0;
  /* Loop for each bucket */
  for (int i = 0; i < count; i++)
  {
    Span s;
    span_set(TimestampTzGetDatum(lower),TimestampTzGetDatum(upper),
      true, false, T_TIMESTAMPTZ, &s);
    if (overlaps_spanset_span(ss, &s))
      spans[nspans++] = s;
    lower += tunits;
    upper += tunits;
  }
  SpanSet *result = NULL;
  if (nspans > 0)
    result = spanset_make(spans, nspans, NORMALIZE);
  pfree(spans);
  return result;
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a span set contains a value.
 */
bool
contains_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  /* Bounding box test */
  if (! contains_span_value(&ss->span, d, basetype))
    return false;

  int loc;
  if (! spanset_find_value(ss, d, &loc))
    return false;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer span set contains an integer.
 * @sqlop @p \@>
 */
bool
contains_intspanset_int(const SpanSet *ss, int i)
{
  return contains_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer span set contains a big integer.
 * @sqlop @p \@>
 */
bool
contains_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return contains_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float span set contains a float.
 * @sqlop @p \@>
 */
bool
contains_floatspanset_float(const SpanSet *ss, double d)
{
  return contains_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a period set contains a timestamp.
 * @sqlop @p \@>
 */
bool
contains_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return contains_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set contains a span.
 * @sqlop @p \@>
 */
bool
contains_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Bounding box test */
  if (! contains_span_span(&ss->span, s))
    return false;

  int loc;
  spanset_find_value(ss, s->lower, &loc);
  const Span *s1 = spanset_sp_n(ss, loc);
  return contains_span_span(s1, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span contains a span set.
 * @sqlop @p \@>
 */
bool
contains_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_span_span(s, &ss->span);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first span set contains the second one.
 * @sqlop @p \@>
 */
bool
contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Bounding box test */
  if (! contains_span_span(&ss1->span, &ss2->span))
    return false;

  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    if (left_span_span(s1, s2))
      i++;
    else if (left_span_span(s2, s1))
      return false;
    else
    {
      /* s1 and s2 overlap */
      if (contains_span_span(s1, s2))
      {
        if (s1->upper == s2->upper)
        {
          i++; j++;
        }
        else
          j++;
      }
      else
        return false;
    }
  }
  /* if j == ss2->count every span in s2 is contained in a span of s1
     but s1 may have additional spans */
  return (j == ss2->count);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a value is contained in a span
 */
bool
contained_value_spanset(Datum d, meosType basetype, const SpanSet *ss)
{
  return contains_spanset_value(ss, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer is contained in an integer span
 * @sqlop @p \@>
 */
bool
contained_int_intspanset(int i, const SpanSet *ss)
{
  return contained_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer is contained in a big integer span
 * @sqlop @p \@>
 */
bool
contained_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return contained_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float is contained in a float span
 * @sqlop @p \@>
 */
bool
contained_float_floatspanset(double d, const SpanSet *ss)
{
  return contained_value_spanset(Float8GetDatum(d), T_FLOAT8, ss);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span is contained in a span set
 * @sqlop @p <@
 */
bool
contained_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_spanset_span(ss, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set is contained in a span
 * @sqlop @p <@
 */
bool
contained_spanset_span(const SpanSet *ss, const Span *s)
{
  return contains_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first span set is contained in the second one
 * @sqlop @p <@
 */
bool
contained_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  return contains_spanset_spanset(ss2, ss1);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set and a span overlap.
 * @sqlop @p &&
 */
bool
overlaps_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Bounding box test */
  if (! overlaps_span_span(s, &ss->span))
    return false;

  /* Binary search of lower bound of span */
  int loc;
  spanset_find_value(ss, s->lower, &loc);
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    if (overlaps_span_span(s1, s))
      return true;
    if (s->upper < s1->upper)
      break;
  }
  return false;
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if two span sets overlap.
 * @sqlop @p &&
 */
bool
overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Bounding box test */
  if (! overlaps_span_span(&ss1->span, &ss2->span))
    return false;

  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    if (overlaps_span_span(s1, s2))
      return true;
    int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return false;
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a span set and a value are adjacent.
 */
bool
adjacent_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  /*
   * A span set and a value are adjacent if and only if the first or the last
   * span is adjacent to the value
   */
  const Span *s1 = spanset_sp_n(ss, 0);
  const Span *s2 = spanset_sp_n(ss, ss->count - 1);
  return adjacent_span_value(s1, d, basetype) ||
         adjacent_span_value(s2, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if an integer span set and an integer are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_intspanset_int(const SpanSet *ss, int i)
{
  return adjacent_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a big integer span set and a big integer are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return adjacent_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a float span set and a float are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_floatspanset_float(const SpanSet *ss, double d)
{
  return adjacent_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a period set a timestamp are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return adjacent_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set and a span are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  const Span *s2 = spanset_sp_n(ss, ss->count - 1);
  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (s2->upper == s->lower && s2->upper_inc != s->lower_inc) ||
       (s->upper == s1->lower && s->upper_inc != s1->lower_inc);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if two span sets are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  const Span *startps1 = spanset_sp_n(ss1, 0);
  const Span *endps1 = spanset_sp_n(ss1, ss1->count - 1);
  const Span *startps2 = spanset_sp_n(ss2, 0);
  const Span *endps2 = spanset_sp_n(ss2, ss2->count - 1);
  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (endps1->upper == startps2->lower && endps1->upper_inc != startps2->lower_inc) ||
    (endps2->upper == startps1->lower && endps2->upper_inc != startps1->lower_inc);
}

/*****************************************************************************
 * Strictly left
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 */
bool
left_value_spanset(Datum d, meosType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return left_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the left of an integer span set.
 * @sqlop @p <<
 */
bool
left_int_intspanset(int i, const SpanSet *ss)
{
  return left_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the left of a big integer
 * span set.
 * @sqlop @p <<
 */
bool
left_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return left_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the left of a float span set.
 * @sqlop @p <<
 */
bool
left_float_floatspanset(double d, const SpanSet *ss)
{
  return left_value_spanset(Float8GetDatum(d), T_FLOAT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly before a period set.
 * @sqlop @p <<#
 */
bool
before_timestamp_periodset(TimestampTz t, const SpanSet *ps)
{
  return left_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span is strictly before a span set.
 * @sqlop @p <<
 */
bool
left_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  return left_span_span(s, s1);
}

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span set is strictly to the left of a value.
 */
bool
left_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return left_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span set is strictly to the left of an
 * integer.
 * @sqlop @p <<
 */
bool
left_intspanset_int(const SpanSet *ss, int i)
{
  return left_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span set is strictly to the left of a
 * big integer.
 * @sqlop @p <<
 */
bool
left_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return left_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span set is strictly to the left of a float.
 * @sqlop @p <<
 */
bool
left_floatspanset_float(const SpanSet *ss, double d)
{
  return left_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period set is strictly before a timestamp.
 * @sqlop @p <<#
 */
bool
before_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return left_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the left a span.
 * @sqlop @p <<
 */
bool
left_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return left_span_span(s1, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span set is strictly to the left of the
 * second one.
 * @sqlop @p <<
 */
bool
left_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  const Span *s1 = spanset_sp_n(ss1, ss1->count - 1);
  const Span *s2 = spanset_sp_n(ss2, 0);
  return left_span_span(s1, s2);
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 */
bool
right_value_spanset(Datum d, meosType basetype, const SpanSet *ss)
{
  return left_spanset_value(ss, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer is strictly to the right of an integer span set.
 * @sqlop @p #&>
 */
bool
right_int_intspanset(int i, const SpanSet *ss)
{
  return left_intspanset_int(ss, i);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer is strictly to the right of a big
 * integer span set.
 * @sqlop @p #&>
 */
bool
right_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return left_bigintspanset_bigint(ss, i);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float is strictly to the right of a float span set.
 * @sqlop @p #&>
 */
bool
right_float_floatspanset(double d, const SpanSet *ss)
{
  return left_floatspanset_float(ss, d);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is strictly after a period set.
 * @sqlop @p #&>
 */
bool
after_timestamp_periodset(TimestampTz t, const SpanSet *ss)
{
  return before_periodset_timestamp(ss, t);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span is strictly to the right of a span set.
 * @sqlop @p >>
 */
bool
right_span_spanset(const Span *s, const SpanSet *ss)
{
  return left_spanset_span(ss, s);
}

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span set is strictly to the right of a value.
 */
bool
right_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  return left_value_spanset(d, basetype, ss);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span set is strictly to the right of an
 * integer.
 * @sqlop @p >>
 */
bool
right_intspanset_int(const SpanSet *ss, int i)
{
  return left_int_intspanset(i, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span set is strictly to the right of a
 * big integer.
 * @sqlop @p >>
 */
bool
right_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return left_bigint_bigintspanset(i, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span set is strictly to the right of a float.
 * @sqlop @p >>
 */
bool
right_floatspanset_float(const SpanSet *ss, double d)
{
  return left_float_floatspanset(d, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period set is strictly after a timestamp.
 * @sqlop @p >>
 */
bool
after_periodset_timestamp(const SpanSet *ss, TimestampTz t)
{
  return before_timestamp_periodset(t, ss);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the right of a span.
 * @sqlop @p >>
 */
bool
right_spanset_span(const SpanSet *ss, const Span *s)
{
  return left_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span set is strictly to the right of the
 * second one.
 * @sqlop @p >>
 */
bool
right_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  return left_spanset_spanset(ss2, ss1);
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span set does not extend to the right of a value.
 */
bool
overleft_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span set does not extend to the right of an
 * integer.
 * @sqlop @p &<
 */
bool
overleft_intspanset_int(const SpanSet *ss, int i)
{
  return overleft_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span set does not extend to the right of
 * a big integer.
 * @sqlop @p &<
 */
bool
overleft_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return overleft_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span set does not extend to the right of a
 * float.
 * @sqlop @p &<
 */
bool
overleft_floatspanset_float(const SpanSet *ss, double d)
{
  return overleft_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period set is not after a timestamp.
 * @sqlop @p &<#
 */
bool
overbefore_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return overleft_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set.
 */
bool
overleft_value_spanset(Datum d, meosType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the right of an integer
 * span set.
 * @sqlop @p &<
 */
bool
overleft_int_intspanset(int i, const SpanSet *ss)
{
  return overleft_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the right of a big
 * integer span set.
 * @sqlop @p &<
 */
bool
overleft_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return overleft_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the right of a float span set.
 * @sqlop @p &<
 */
bool
overleft_float_floatspanset(double d, const SpanSet *ss)
{
  return overleft_value_spanset(Float8GetDatum(d), T_FLOAT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not after a period set.
 * @sqlop @p &<#
 */
bool
overbefore_timestamp_periodset(TimestampTz t, const SpanSet *ps)
{
  return overleft_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span does not extend to the right of a span set.
 * @sqlop @p &<
 */
bool
overleft_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_span(s, s1);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a span.
 * @sqlop @p &<
 */
bool
overleft_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_span(s1, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span set does not extend to the right of the
 * second one.
 * @sqlop @p &<
 */
bool
overleft_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  const Span *s1 = spanset_sp_n(ss1, ss1->count - 1);
  const Span *s2 = spanset_sp_n(ss2, ss2->count - 1);
  return overleft_span_span(s1, s2);
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the left of a span set.
 */
bool
overright_value_spanset(Datum d, meosType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer does not extend to the left of an integer
 * span set.
 * @sqlop @p &>
 */
bool
overright_int_intspanset(int i, const SpanSet *ss)
{
  return overright_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer does not extend to the left of a big
 * integer span set.
 * @sqlop @p &>
 */
bool
overright_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return overright_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float does not extend to the left of a float span set.
 * @sqlop @p &>
 */
bool
overright_float_floatspanset(double d, const SpanSet *ss)
{
  return overright_value_spanset(Float8GetDatum(d), T_FLOAT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a timestamp is not before a period set.
 * @sqlop @p #&>
 */
bool
overafter_timestamp_periodset(TimestampTz t, const SpanSet *ps)
{
  return overright_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span does not extend to the left of a span set.
 * @sqlop @p &>
 */
bool
overright_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  return overright_span_span(s, s1);
}

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value.
 */
bool
overright_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an integer span set does not extend to the left of an
 * integer.
 * @sqlop @p &>
 */
bool
overright_intspanset_int(const SpanSet *ss, int i)
{
  return overright_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a big integer span set does not extend to the left of
 * a big integer.
 * @sqlop @p &>
 */
bool
overright_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return overright_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a float span set does not extend to the left of a float.
 * @sqlop @p &>
 */
bool
overright_floatspanset_float(const SpanSet *ss, double d)
{
  return overright_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period set is before a timestamp.
 * @sqlop @p &>
 */
bool
overafter_periodset_timestamp(const SpanSet *ss, TimestampTz t)
{
  return overright_spanset_value(ss, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a span.
 * @sqlop @p &>
 */
bool
overright_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  return overright_span_span(s1, s);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span set does not extend to the left of the
 * second one.
 * @sqlop @p &>
 */
bool
overright_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  const Span *s1 = spanset_sp_n(ss1, 0);
  const Span *s2 = spanset_sp_n(ss2, 0);
  return overright_span_span(s1, s2);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the union of a span set and a value.
 */
SpanSet *
union_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  Span s;
  span_set(d, d, true, true, basetype, &s);
  SpanSet *result = union_spanset_span(ss, &s);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of an integer span set and an integer.
 * @sqlop @p +
 */
SpanSet *
union_intspanset_int(const SpanSet *ss, int i)
{
  return union_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a big integer span set and a big integer.
 * @sqlop @p +
 */
SpanSet *
union_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return union_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a float span set and a float.
 * @sqlop @p +
 */
SpanSet *
union_floatspanset_float(const SpanSet *ss, double d)
{
  return union_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a period set and a timestamp.
 * @sqlop @p +
 */
SpanSet *
union_periodset_timestamp(SpanSet *ps, TimestampTz t)
{
  return union_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a span set and a span
 * @sqlop @p +
 */
SpanSet *
union_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Transform the span into a span set */
  SpanSet *ss1 = span_to_spanset(s);
  /* Call the function for the span set */
  SpanSet *result = union_spanset_spanset(ss1, ss);
  pfree(ss1);
  return result;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of two span sets.
 * @sqlop @p +
 */
SpanSet *
union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  Span *spans = palloc(sizeof(Span) * (ss1->count + ss2->count));
  int i = 0, j = 0, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    /* The spans do not overlap, copy the earliest span */
    if (! overlaps_span_span(s1, s2))
    {
      if (left_span_span(s1, s2))
      {
        spans[nspans++] = *s1;
        i++;
      }
      else
      {
        spans[nspans++] = *s2;
        j++;
      }
    }
    else
    {
      /* Find all spans in ss1 that overlap with spans in ss2
       *      i                    i
       *   |-----| |-| |-----|  |-----|
       *       |---------|  |-----|
       *            j          j
       */
      Span q;
      bbox_union_span_span(s1, s2, &q);
      while (i < ss1->count && j < ss2->count)
      {
        s1 = spanset_sp_n(ss1, i);
        s2 = spanset_sp_n(ss2, j);
        bool over_p1_q = overlaps_span_span(s1, &q);
        bool over_p2_q = overlaps_span_span(s2, &q);
        if (! over_p1_q && ! over_p2_q)
          break;
        if (over_p1_q)
        {
          span_expand(s1, &q);
          i++;
        }
        if (over_p2_q)
        {
          span_expand(s2, &q);
          j++;
        }
      }
      /* When one of the sets is finished we need to absorb overlapping
       * spans in the other set */
      while (i < ss1->count)
      {
        s1 = spanset_sp_n(ss1, i);
        if (overlaps_span_span(s1, &q))
        {
          span_expand(s1, &q);
          i++;
        }
        else
          break;
      }
      while (j < ss2->count)
      {
        s2 = spanset_sp_n(ss2, j);
        if (overlaps_span_span(s2, &q))
        {
          span_expand(s2, &q);
          j++;
        }
        else
          break;
      }
      spans[nspans++] = q;
    }
  }
  /* Only one of the following two while will be executed */
  while (i < ss1->count)
    spans[nspans++] = *spanset_sp_n(ss1, i++);
  while (j < ss2->count)
    spans[nspans++] = *spanset_sp_n(ss2, j++);
  /* nspans is never equal to 0 since the span sets are not empty */
  return spanset_make_free(spans, nspans, NORMALIZE);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the intersection of a span set and a value in the last
 * argument
 */
bool
intersection_spanset_value(const SpanSet *ss, Datum d, meosType basetype,
  Datum *result)
{
  if (! contains_spanset_value(ss, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of an integer span set and an integer in the
 * last argument
 * @sqlop @p *
 */
bool
intersection_intspanset_int(const SpanSet *ss, int i, int *result)
{
  if (! contains_spanset_value(ss, Int32GetDatum(i), T_INT4))
    return false;
  *result = i;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a big integer span set and a big integer
 * @sqlop @p *
 */
bool
intersection_bigintspanset_bigint(const SpanSet *ss, int64 i, int64 *result)
{
  if (! contains_spanset_value(ss, Int64GetDatum(i), T_INT8))
    return false;
  *result = i;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a float span set and a float in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_floatspanset_float(const SpanSet *ss, double d, double *result)
{
  if (! contains_spanset_value(ss, Float8GetDatum(d), T_FLOAT8))
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the intersection of a period set and a timestamp in the last
 * argument
 * @sqlop @p *
 */
bool
intersection_periodset_timestamp(const SpanSet *ps, TimestampTz t,
  TimestampTz *result)
{
  if (! contains_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ))
    return false;
  *result = t;
  return true;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of a span set and a span
 * @sqlop @p *
 */
SpanSet *
intersection_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Bounding box test */
  if (! overlaps_span_span(s, &ss->span))
    return NULL;

  /* Is the span set fully contained in the span? */
  if (contains_span_spanset(s, ss))
    return spanset_copy(ss);

  /* General case */
  int loc;
  spanset_find_value(ss, s->lower, &loc);
  int count = ss->count - loc;
  Span *spans = palloc(sizeof(Span) * count);
  int nspans = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    Span s2;
    if (inter_span_span(s1, s, &s2))
      spans[nspans++] = s2;
    if (s->upper < s1->upper)
      break;
  }
  SpanSet *result = NULL;
  if (nspans > 0)
    result = spanset_make_free(spans, nspans, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the intersection of two span sets.
 * @sqlop @p *
 */
SpanSet *
intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Bounding box test */
  Span s;
  if (! inter_span_span(&ss1->span, &ss2->span, &s))
    return NULL;

  int loc1, loc2;
  spanset_find_value(ss1, s.lower, &loc1);
  spanset_find_value(ss2, s.lower, &loc2);
  int count = ss1->count + ss2->count - loc1 - loc2;
  Span *spans = palloc(sizeof(Span) * count);
  int i = loc1, j = loc2, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    Span inter;
    if (inter_span_span(s1, s2, &inter))
      spans[nspans++] = inter;
    int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
    if (cmp == 0 && s1->upper_inc == s2->upper_inc)
    {
      i++; j++;
    }
    else if (cmp < 0 || (cmp == 0 && ! s1->upper_inc && s2->upper_inc))
      i++;
    else
      j++;
  }
  SpanSet *result = NULL;
  if (nspans > 0)
    result = spanset_make(spans, nspans, NORMALIZE);
  pfree(spans);
  return result;
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Compute the difference of a value and a span set
 */
bool
minus_value_spanset(Datum d, meosType basetype, const SpanSet *ss,
  Datum *result)
{
  if (contains_spanset_value(ss, d, basetype))
    return false;
  *result = d;
  return true;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of an integer and an integer span set in the
 * last argument
 * @sqlop @p -
 */
bool
minus_int_intspanset(int i, const SpanSet *ss, int *result)
{
  Datum v;
  bool found = minus_value_spanset(Int32GetDatum(i), T_INT4, ss, &v);
  *result = DatumGetInt32(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a big integer and a big integer span set in
 * the last argument
 * @sqlop @p -
 */
bool
minus_bigint_bigintspanset(int64 i, const SpanSet *ss, int64 *result)
{
  Datum v;
  bool found = minus_value_spanset(Int64GetDatum(i), T_INT8, ss, &v);
  *result = DatumGetInt64(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a float and a float span set in the last
 * argument
 * @sqlop @p -
 */
bool
minus_float_floatspanset(double d, const SpanSet *ss, double *result)
{
  Datum v;
  bool found = minus_value_spanset(Float8GetDatum(d), T_FLOAT8, ss, &v);
  *result = DatumGetFloat8(v);
  return found;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Compute the difference of a timestamp and a period set in the last
 * argument
 * @sqlop @p -
 */
bool
minus_timestamp_periodset(TimestampTz t, const SpanSet *ps,
  TimestampTz *result)
{
  Datum v;
  bool res = minus_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps,
    &v);
  *result = DatumGetTimestampTz(v);
  return res;
}
#endif /* MEOS */

/**
 * @brief Compute the difference of a span and a span set (iterator function).
 */
static int
minus_span_spanset_iter(const Span *s, const SpanSet *ss, int from, int to,
  Span *result)
{
  /* The span can be split at most into (to - from + 1) spans
   *   |----------------------|
   *       |---| |---| |---|
   */
  Span curr = *s;
  int nspans = 0;
  for (int i = from; i < to; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    /* If the remaining spans are to the left of the current span */
    int cmp = datum_cmp(curr.upper, s1->lower, curr.basetype);
    if (cmp < 0 || (cmp == 0 && curr.upper_inc && ! s1->lower_inc))
    {
      result[nspans++] = curr;
      break;
    }
    Span minus[2];
    int countminus = minus_span_span_iter(&curr, s1, minus);
    /* minus can have from 0 to 2 spans */
    if (countminus == 0)
      break;
    else if (countminus == 1)
      curr = minus[0];
    else /* countminus == 2 */
    {
      result[nspans++] = minus[0];
      curr = minus[1];
    }
    /* There are no more spans left */
    if (i == to - 1)
      result[nspans++] = curr;
  }
  return nspans;
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span and a span set.
 * @sqlop @p -
 */
SpanSet *
minus_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Bounding box test */
  if (! overlaps_span_span(s, &ss->span))
    return spanset_make((Span *) s, 1, false);

  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int count = minus_span_spanset_iter(s, ss, 0, ss->count, spans);
  if (count == 0)
  {
    pfree(spans);
    return NULL;
  }
  return spanset_make_free(spans, count, false);
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a span set and a value.
 */
SpanSet *
minus_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  /* Bounding box test */
  if (! contains_span_value(&ss->span, d, basetype))
    return spanset_copy(ss);

  /* At most one composing span can be split into two */
  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int nspans = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *p = spanset_sp_n(ss, i);
    nspans += minus_span_value_iter(p, d, basetype, &spans[nspans]);
  }
  if (nspans == 0)
  {
    pfree(spans);
    return NULL;
  }
  return spanset_make_free(spans, nspans, NORMALIZE_NO);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an integer span set and an integer.
 * @sqlop @p -
 */
SpanSet *
minus_intspanset_int(const SpanSet *ss, int i)
{
  return minus_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a big integer span set and a big integer.
 * @sqlop @p -
 */
SpanSet *
minus_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return minus_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a float span set and a float.
 * @sqlop @p -
 */
SpanSet *
minus_floatspanset_float(const SpanSet *ss, double d)
{
  return minus_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a period set and a timestamp.
 * @sqlop @p -
 */
SpanSet *
minus_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return minus_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span set and a span.
 * @sqlop @p -
 */
SpanSet *
minus_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Bounding box test */
  if (! overlaps_span_span(&ss->span, s))
    return spanset_copy(ss);

  /* At most one composing span can be split into two */
  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int nspans = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    nspans += minus_span_span_iter(s1, s, &spans[nspans]);
  }
  if (nspans == 0)
  {
    pfree(spans);
    return NULL;
  }
  return spanset_make_free(spans, nspans, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of two span sets.
 * @sqlop @p -
 */
SpanSet *
minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Bounding box test */
  if (! overlaps_span_span(&ss1->span, &ss2->span))
    return spanset_copy(ss1);

  Span *spans = palloc(sizeof(Span) * (ss1->count + ss2->count));
  int i = 0, j = 0, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    /* The spans do not overlap, copy the first span */
    if (! overlaps_span_span(s1, s2))
    {
      spans[nspans++] = *s1;
      i++;
    }
    else
    {
      /* Find all spans in ss2 that overlap with s1
       *                  i
       *    |------------------------|
       *      |-----|  |-----|          |---|
       *         j                        l
       */
      int l;
      for (l = j; l < ss2->count; l++)
      {
        const Span *p3 = spanset_sp_n(ss2, l);
        if (! overlaps_span_span(s1, p3))
          break;
      }
      int to = Min(l, ss2->count);
      /* Compute the difference of the overlapping spans */
      nspans += minus_span_spanset_iter(s1, ss2, j, to, &spans[nspans]);
      i++;
      j = l;
    }
  }
  /* Copy the sequences after the span set */
  while (i < ss1->count)
    spans[nspans++] = *spanset_sp_n(ss1, i++);
  if (nspans == 0)
  {
    pfree(spans);
    return NULL;
  }
  return spanset_make_free(spans, nspans, NORMALIZE_NO);
}

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a span set and a value as a double
 */
double
distance_spanset_value(const SpanSet *ss, Datum d, meosType basetype)
{
  return distance_span_value(&ss->span, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between an integer span set and an integer
 * as a double
 * @sqlop @p <->
 */
double
distance_intspanset_int(const SpanSet *ss, int i)
{
  return distance_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a big integer span set and a big integer
 * as a double
 * @sqlop @p <->
 */
double
distance_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return distance_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a float span set and a float
 * @sqlop @p <->
 */
double
distance_floatspanset_float(const SpanSet *ss, double d)
{
  return distance_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance in seconds between a period set and a timestamp
 * as a double
 * @sqlop @p <->
 */
double
distance_periodset_timestamp(const SpanSet *ps, TimestampTz t)
{
  return distance_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a span set and a span as a double
 * @sqlop @p <->
 */
double
distance_spanset_span(const SpanSet *ss, const Span *s)
{
  return distance_span_span(&ss->span, s);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two span sets as a double
 * @sqlop @p <->
 */
double
distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  return distance_span_span(&ss1->span, &ss2->span);
}

/******************************************************************************/
