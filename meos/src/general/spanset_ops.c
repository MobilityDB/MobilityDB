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
 * @brief Operators for span set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/*
 * Return the intersection or the difference of an ordered set and a span set
 */
OrderedSet *
setop_orderedset_spanset(const OrderedSet *os, const SpanSet *ss, SetOper setop)
{
  assert(setop == INTER || setop == MINUS);
  /* Bounding box test */
  if (! overlaps_span_span(&os->span, &ss->span))
    return (setop == INTER) ? NULL : orderedset_copy(os);

  Datum *values = palloc(sizeof(Datum) * os->count);
  Datum v = orderedset_val_n(os, 0);
  const Span *s = spanset_sp_n(ss, 0);
  int i = 0, j = 0, k = 0;
  while (i < os->count && j < ss->count)
  {
    if (datum_lt(v, s->lower, os->span.basetype))
    {
      if (setop == MINUS)
        values[k++] = v;
      i++;
      if (i == os->count)
        break;
      else
        v = orderedset_val_n(os, i);
    }
    else if (datum_gt(v, s->upper, os->span.basetype))
    {
      j++;
      if (j == ss->count)
        break;
      else
        s = spanset_sp_n(ss, j);
    }
    else
    {
      if ((setop == INTER && contains_span_value(s, v, os->span.basetype)) ||
        (setop == MINUS && ! contains_span_value(s, v, os->span.basetype)))
        values[k++] = v;
      i++;
      if (i == os->count)
        break;
      else
        v = orderedset_val_n(os, i);
    }
  }
  if (setop == MINUS)
  {
    for (int l = i; l < os->count; l++)
      values[k++] = orderedset_val_n(os, l);
  }
  return orderedset_make_free(values, k, os->span.basetype);
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_topo
 * @brief Return true if a span set contains a value.
 */
bool
contains_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
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
 * @brief Return true if a span set contains a value.
 * @sqlop @p \@>
 */
bool
contains_intspanset_int(const SpanSet *ss, int i)
{
  return contains_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set contains a value.
 * @sqlop @p \@>
 */
bool
contains_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return contains_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set contains a value.
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
contains_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return contains_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set contains an ordered set.
 * @sqlop @p \@>
 */
bool
contains_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  /* Bounding box test */
  if (! contains_span_span(&ss->span, &os->span))
    return false;

  int i = 0, j = 0;
  while (j < os->count)
  {
    const Span *s = spanset_sp_n(ss, i);
    Datum v = orderedset_val_n(os, j);
    if (contains_span_value(s, v, ss->span.basetype))
      j++;
    else
    {
      if (datum_gt(v, s->upper, ss->span.basetype))
        i++;
      else
        return false;
    }
  }
  return true;
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
 * @brief Return true if a value is contained by a span
 */
bool
contained_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  return contains_spanset_value(ss, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by a span
 * @sqlop @p \@>
 */
bool
contained_int_intspanset(int i, const SpanSet *ss)
{
  return contained_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by a span
 * @sqlop @p \@>
 */
bool
contained_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return contained_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a value is contained by a span
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
 * @brief Return true if an ordered set is contained by a span set
 * @sqlop @p <@
 */
bool
contained_orderedset_spanset(const OrderedSet *os, const SpanSet *ss)
{
  return contains_spanset_orderedset(ss, os);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span is contained by a span
 * @sqlop @p <@
 */
bool
contained_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_spanset_span(ss, s);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set is contained by a span
 * @sqlop @p <@
 */
bool
contained_spanset_span(const SpanSet *ss, const Span *s)
{
  return contains_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if the first span set is contained by the second one
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
 * @brief Return true if an ordered set and a span set overlap.
 * @sqlop @p &&
 */
bool
overlaps_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  /* Bounding box test */
  if (! overlaps_span_span(&ss->span, &os->span))
    return false;

  int i = 0, j = 0;
  while (i < os->count && j < ss->count)
  {
    Datum d = orderedset_val_n(os, i);
    const Span *s = spanset_sp_n(ss, j);
    if (contains_span_value(s, d, os->span.basetype))
      return true;
    else if (datum_gt(d, s->upper, os->span.basetype))
      j++;
    else
      i++;
  }
  return false;
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span and a span set overlap.
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
adjacent_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
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
 * @brief Return true if a span set and a value are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_intspanset_int(const SpanSet *ss, int i)
{
  return adjacent_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set and a value are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return adjacent_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set and a value are adjacent.
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
adjacent_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return adjacent_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_topo
 * @brief Return true if a span set and an ordered set are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  /*
   * A spanset A..B and a span C are adjacent if and only if
   * B is adjacent to C, or C is adjacent to A.
   */
  Datum d1 = orderedset_val_n(os, 0);
  Datum d2 = orderedset_val_n(os, os->count - 1);
  const Span *s1 = spanset_sp_n(ss, 0);
  const Span *s2 = spanset_sp_n(ss, ss->count - 1);
  return (datum_eq(d2, s1->lower, os->span.basetype) && ! s1->lower_inc) ||
         (datum_eq(s2->upper, d1, os->span.basetype) && ! s2->upper_inc);
}

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
 * @brief Return true if a span set and a span are adjacent.
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
left_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return left_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<
 */
bool
left_int_intspanset(int i, const SpanSet *ss)
{
  return left_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<
 */
bool
left_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return left_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<
 */
bool
left_float_floatspanset(double d, const SpanSet *ss)
{
  return left_value_spanset(Float8GetDatum(d), T_FLOAT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a period set is strictly before a timestamp.
 * @sqlop @p <<#
 */
bool
before_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  return left_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set is strictly left a span set.
 * @sqlop @p <<#
 */
bool
left_orderedset_spanset(const OrderedSet *os, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  Datum v = orderedset_val_n(os, os->count - 1);
  return left_value_span(v, os->span.basetype, s);
}

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
left_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return left_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the left of a value.
 * @sqlop @p <<
 */
bool
left_intspanset_int(const SpanSet *ss, int i)
{
  return left_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the left of a value.
 * @sqlop @p <<
 */
bool
left_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return left_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the left of a value.
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
before_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return left_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the left of an ordered set.
 * @sqlop @p <<#
 */
bool
left_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  Datum v = orderedset_val_n(os, 0);
  return left_span_value(s, v, os->span.basetype);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly before a span.
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
 * @brief Return true if the first span set is strictly before the second one.
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
right_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  return left_spanset_value(ss, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p #&>
 */
bool
right_int_intspanset(int i, const SpanSet *ss)
{
  return left_intspanset_int(ss, i);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p #&>
 */
bool
right_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return left_bigintspanset_bigint(ss, i);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p #&>
 */
bool
right_float_floatspanset(double d, const SpanSet *ss)
{
  return left_floatspanset_float(ss, d);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value is strictly to the right of a span set.
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
 * @brief Return true if an ordered set is strictly right a span set.
 * @sqlop @p #>>
 */
bool
right_orderedset_spanset(const OrderedSet *os, const SpanSet *ss)
{
  return left_spanset_orderedset(ss, os);
}

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
right_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  return left_value_spanset(d, basetype, ss);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the right of a value.
 * @sqlop @p >>
 */
bool
right_intspanset_int(const SpanSet *ss, int i)
{
  return left_int_intspanset(i, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the right of a value.
 * @sqlop @p >>
 */
bool
right_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return left_bigint_bigintspanset(i, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the right of a value.
 * @sqlop @p >>
 */
bool
right_floatspanset_float(const SpanSet *ss, double d)
{
  return left_float_floatspanset(d, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly to the right of a value.
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
 * @brief Return true if a span set is strictly right of an ordered set.
 * @sqlop @p #>>
 */
bool
right_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  return left_orderedset_spanset(os, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set is strictly after a span.
 * @sqlop @p >>
 */
bool
right_spanset_span(const SpanSet *ss, const Span *s)
{
  return left_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if the first span set is strictly after the second one.
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
overleft_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a value.
 * @sqlop @p &<
 */
bool
overleft_intspanset_int(const SpanSet *ss, int i)
{
  return overleft_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a value.
 * @sqlop @p &<
 */
bool
overleft_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return overleft_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a value.
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
overbefore_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return overleft_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set.
 */
bool
overleft_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set.
 * @sqlop @p &<
 */
bool
overleft_int_intspanset(int i, const SpanSet *ss)
{
  return overleft_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set.
 * @sqlop @p &<
 */
bool
overleft_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return overleft_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set.
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
overbefore_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  return overleft_value_spanset(TimestampTzGetDatum(t), T_TIMESTAMPTZ, ps);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if an ordered set does not extend to the right of a
 * span set.
 * @sqlop @p &<#
 */
bool
overleft_orderedset_spanset(const OrderedSet *os, const SpanSet *ss)
{
  Datum v = orderedset_val_n(os, os->count - 1);
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_value_span(v, os->span.basetype, s);
}

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
 * @brief Return true if a span set does not extend to the right of an ordered set.
 * @sqlop @p &<#
 */
bool
overleft_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  Datum v = orderedset_val_n(os, os->count - 1);
  return overleft_span_value(s, v, os->span.basetype);
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
overright_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_value_span(d, basetype, s);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the left of a span set.
 * @sqlop @p &>
 */
bool
overright_int_intspanset(int i, const SpanSet *ss)
{
  return overright_value_spanset(Int32GetDatum(i), T_INT4, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the left of a span set.
 * @sqlop @p &>
 */
bool
overright_bigint_bigintspanset(int64 i, const SpanSet *ss)
{
  return overright_value_spanset(Int64GetDatum(i), T_INT8, ss);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a value does not extend to the left of a span set.
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
overafter_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
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
overright_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value.
 * @sqlop @p &>
 */
bool
overright_intspanset_int(const SpanSet *ss, int i)
{
  return overright_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value.
 * @sqlop @p &>
 */
bool
overright_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return overright_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value.
 * @sqlop @p &>
 */
bool
overright_floatspanset_float(const SpanSet *ss, double d)
{
  return overright_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value.
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
 * @brief Return true if a span set is not to the left of an ordered set.
 * @sqlop @p #&>
 */
bool
overright_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  const Span *s = spanset_sp_n(ss, 0);
  Datum v = orderedset_val_n(os, 0);
  return overright_span_value(s, v, ss->span.basetype);
}

/**
 * @ingroup libmeos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a span.
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
 * @brief Return true if the first span set does not extend to the right of the
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
union_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  Span s;
  span_set(d, d, true, true, basetype, &s);
  SpanSet *result = union_spanset_span(ss, &s);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a span set and a value.
 * @sqlop @p +
 */
bool
union_intspanset_int(const SpanSet *ss, int i)
{
  return union_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a span set and a value.
 * @sqlop @p +
 */
bool
union_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return union_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a span set and a value.
 * @sqlop @p +
 */
bool
union_floatspanset_float(const SpanSet *ss, double d)
{
  return union_spanset_value(ss, Float8GetDatum(d), T_FLOAT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a period set and a timestamp.
 * @sqlop @p +
 */
PeriodSet *
union_periodset_timestamp(PeriodSet *ps, TimestampTz t)
{
  return union_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the union of a span set and an ordered set.
 * @sqlop @p +
 */
SpanSet *
union_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  SpanSet *ss1 = orderedset_to_spanset(os);
  SpanSet *result = union_spanset_spanset(ss, ss1);
  pfree(ss1);
  return result;
}

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
  Span **spans = palloc(sizeof(Span *) * (ss1->count + ss2->count));
  Span **mustfree = NULL;
  /* If the span sets overlap we will be intersecting composing spans */
  if (overlaps_spanset_spanset(ss1, ss2))
    mustfree = palloc(sizeof(Span *) * Max(ss1->count, ss2->count));

  int i = 0, j = 0, k = 0, l = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    /* The spans do not overlap, copy the earliest span */
    if (! overlaps_span_span(s1, s2))
    {
      if (left_span_span(s1, s2))
      {
        spans[k++] = (Span *) s1;
        i++;
      }
      else
      {
        spans[k++] = (Span *) s2;
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
      Span *q = bbox_union_span_span(s1, s2, false);
      while (i < ss1->count && j < ss2->count)
      {
        s1 = spanset_sp_n(ss1, i);
        s2 = spanset_sp_n(ss2, j);
        bool over_p1_q = overlaps_span_span(s1, q);
        bool over_p2_q = overlaps_span_span(s2, q);
        if (! over_p1_q && ! over_p2_q)
          break;
        if (over_p1_q)
        {
          span_expand(s1, q);
          i++;
        }
        if (over_p2_q)
        {
          span_expand(s2, q);
          j++;
        }
      }
      /* When one of the sets is finished we need to absorb overlapping
       * spans in the other set */
      while (i < ss1->count)
      {
        s1 = spanset_sp_n(ss1, i);
        if (overlaps_span_span(s1, q))
        {
          span_expand(s1, q);
          i++;
        }
        else
          break;
      }
      while (j < ss2->count)
      {
        s2 = spanset_sp_n(ss2, j);
        if (overlaps_span_span(s2, q))
        {
          span_expand(s2, q);
          j++;
        }
        else
          break;
      }
      spans[k++] = mustfree[l++] = q;
    }
  }
  /* Only one of the following two while will be executed */
  while (i < ss1->count)
    spans[k++] = (Span *) spanset_sp_n(ss1, i++);
  while (j < ss2->count)
    spans[k++] = (Span *) spanset_sp_n(ss2, j++);
  /* k is never equal to 0 since the span sets are not empty */
  SpanSet *result = spanset_make((const Span **) spans, k, NORMALIZE);
  pfree(spans);

  if (mustfree)
    pfree_array((void **) mustfree, l);

  return result;
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the intersection of a span set and a value
 */
bool
intersection_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype,
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
 * @brief Return the intersection of a span set and a value
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
 * @brief Return the intersection of a span set and a value
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
 * @brief Return the intersection of a span set and a value
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
 * @brief Return the intersection of a period set and a timestamp
 * @sqlop @p *
 */
bool
intersection_periodset_timestamp(const PeriodSet *ps, TimestampTz t,
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
 * @brief Return the intersection of a span set and an ordered set
 * @sqlop @p *
 */
OrderedSet *
intersection_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  return setop_orderedset_spanset(os, ss, INTER);
}

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
  Span **spans = palloc(sizeof(Span *) * (ss->count - loc));
  int k = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    Span *s2 = intersection_span_span(s1, s);
    if (s2 != NULL)
      spans[k++] = s2;
    if (s->upper < s1->upper)
      break;
  }
  SpanSet *result = spanset_make_free(spans, k, NORMALIZE_NO);
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
  Span **spans = palloc(sizeof(Span *) *
    (ss1->count + ss2->count - loc1 - loc2));
  int i = loc1, j = loc2, k = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    Span *inter = intersection_span_span(s1, s2);
    if (inter != NULL)
      spans[k++] = inter;
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
  SpanSet *result = spanset_make_free(spans, k, NORMALIZE);
  return result;
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a value and a span set
 */
bool
minus_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss,
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
 * @brief Return the difference of a value and a span set
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
 * @brief Return the difference of a value and a span set
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
 * @brief Return the difference of a value and a span set
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
 * @brief Return the difference of a timestamp and a period set
 * @sqlop @p -
 */
bool
minus_timestamp_periodset(TimestampTz t, const PeriodSet *ps,
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
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of an ordered set and a span set.
 * @sqlop @p -
 */
OrderedSet *
minus_orderedset_spanset(const OrderedSet *os, const SpanSet *ss)
{
  return setop_orderedset_spanset(os, ss, MINUS);
}

/**
 * Return the difference of a span and a span set.
 */
int
minus_span_spanset1(Span **result, const Span *s, const SpanSet *ss,
  int from, int to)
{
  /* The span can be split at most into (to - from + 1) spans
   *   |----------------------|
   *       |---| |---| |---|
   */
  Span *curr = span_copy(s);
  int k = 0;
  for (int i = from; i < to; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    /* If the remaining spans are to the left of the current span */
    int cmp = datum_cmp(curr->upper, s1->lower, curr->basetype);
    if (cmp < 0 || (cmp == 0 && curr->upper_inc && ! s1->lower_inc))
    {
      result[k++] = curr;
      break;
    }
    Span *minus[2];
    int countminus = minus_span_span1(curr, s1, minus);
    pfree(curr);
    /* minus can have from 0 to 2 spans */
    if (countminus == 0)
      break;
    else if (countminus == 1)
      curr = minus[0];
    else /* countminus == 2 */
    {
      result[k++] = span_copy(minus[0]);
      curr = minus[1];
    }
    /* There are no more spans left */
    if (i == to - 1)
      result[k++] = curr;
  }
  return k;
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
    return spanset_make((const Span **) &s, 1, false);

  Span **spans = palloc(sizeof(Span *) * (ss->count + 1));
  int count = minus_span_spanset1(spans, s, ss, 0, ss->count);
  SpanSet *result = spanset_make_free(spans, count, false);
  return result;
}

/**
 * @ingroup libmeos_internal_setspan_set
 * @brief Return the difference of a span set and a value.
 */
SpanSet *
minus_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  /* Bounding box test */
  if (! contains_span_value(&ss->span, d, basetype))
    return spanset_copy(ss);

  /* At most one composing span can be split into two */
  Span **spans = palloc(sizeof(Span *) * (ss->count + 1));
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *p = spanset_sp_n(ss, i);
    k += minus_span_value1(p, d, basetype, &spans[k]);
  }
  SpanSet *result = spanset_make_free(spans, k, NORMALIZE_NO);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span set and a value.
 * @sqlop @p -
 */
SpanSet *
minus_intspanset_int(const SpanSet *ss, int i)
{
  return minus_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span set and a value.
 * @sqlop @p -
 */
SpanSet *
minus_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return minus_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span set and a value.
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
minus_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return minus_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_set
 * @brief Return the difference of a span set and an ordered set.
 * @sqlop @p -
 */
SpanSet *
minus_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  /* Bounding box test */
  if (! overlaps_span_span(&ss->span, &os->span))
    return spanset_copy(ss);

  /* Each value will split at most one composing span into two */
  Span **spans = palloc(sizeof(Span *) * (ss->count + os->count + 1));
  int i = 0, j = 0, k = 0;
  Span *curr = span_copy(spanset_sp_n(ss, 0));
  Datum d = orderedset_val_n(os, 0);
  while (i < ss->count && j < os->count)
  {
    if (datum_gt(d, curr->upper, ss->span.basetype))
    {
      spans[k++] = curr;
      i++;
      if (i == ss->count)
        break;
      else
        curr = span_copy(spanset_sp_n(ss, i));
    }
    else if (datum_lt(d, curr->lower, ss->span.basetype))
    {
      j++;
      if (j == os->count)
        break;
      else
        d = orderedset_val_n(os, j);
    }
    else
    {
      if (contains_span_value(curr, d, ss->span.basetype))
      {
        if (curr->lower == curr->upper)
        {
          pfree(curr);
          i++;
          if (i == ss->count)
            break;
          else
            curr = span_copy(spanset_sp_n(ss, i));
        }
        else if (datum_eq(curr->lower, d, ss->span.basetype))
        {
          Span *curr1 = span_make(curr->lower, curr->upper, false,
            curr->upper_inc, ss->span.basetype);
          pfree(curr);
          curr = curr1;
        }
        else if (datum_eq(curr->upper, d, ss->span.basetype))
        {
          spans[k++] = span_make(curr->lower, curr->upper, curr->lower_inc,
            false, ss->span.basetype);
          pfree(curr);
          i++;
          if (i == ss->count)
            break;
          else
            curr = span_copy(spanset_sp_n(ss, i));
        }
        else
        {
          spans[k++] = span_make(curr->lower, d, curr->lower_inc, false,
            ss->span.basetype);
          Span *curr1 = span_make(d, curr->upper, false, curr->upper_inc,
            ss->span.basetype);
          pfree(curr);
          curr = curr1;
        }
      }
      else
      {
        if (datum_eq(curr->upper, d, ss->span.basetype))
        {
          spans[k++] = curr;
          i++;
          if (i == ss->count)
            break;
          else
            curr = span_copy(spanset_sp_n(ss, i));
        }
      }
      j++;
      if (j == os->count)
        break;
      else
        d = orderedset_val_n(os, j);
    }
  }
  /* If we ran through all the instants */
  if (j == os->count)
    spans[k++] = curr;
  for (int l = i + 1; l < ss->count; l++)
    spans[k++] = (Span *) spanset_sp_n(ss, l);

  if (k == 0)
  {
    pfree(spans);
    return NULL;
  }
  SpanSet *result = spanset_make((const Span **) spans, k, NORMALIZE_NO);
  pfree_array((void **) spans, i);
  return result;
}

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
  Span **spans = palloc(sizeof(Span *) * (ss->count + 1));
  int k = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss, i);
    k += minus_span_span1(s1, s, &spans[k]);
  }
  SpanSet *result = spanset_make_free(spans, k, NORMALIZE_NO);
  return result;
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

  Span **spans = palloc(sizeof(const Span *) * (ss1->count + ss2->count));
  int i = 0, j = 0, k = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, j);
    /* The spans do not overlap, copy the first span */
    if (! overlaps_span_span(s1, s2))
    {
      spans[k++] = span_copy(s1);
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
      k += minus_span_spanset1(&spans[k], s1, ss2, j, to);
      i++;
      j = l;
    }
  }
  /* Copy the sequences after the span set */
  while (i < ss1->count)
    spans[k++] = span_copy(spanset_sp_n(ss1, i++));
  SpanSet *result = spanset_make_free(spans, k, NORMALIZE_NO);
  return result;
}

/******************************************************************************
 * Distance functions returning a double
 ******************************************************************************/

/**
 * @ingroup libmeos_internal_setspan_dist
 * @brief Return the distance between a timestamp and a span set
 */
double
distance_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  return distance_span_value(&ss->span, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a timestamp and a span set
 * @sqlop @p <->
 */
double
distance_intspanset_int(const SpanSet *ss, int i)
{
  return distance_spanset_value(ss, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a timestamp and a span set
 * @sqlop @p <->
 */
double
distance_bigintspanset_bigint(const SpanSet *ss, int64 i)
{
  return distance_spanset_value(ss, Int64GetDatum(i), T_INT8);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a timestamp and a span set
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
 * @sqlop @p <->
 */
double
distance_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return distance_spanset_value(ps, TimestampTzGetDatum(t), T_TIMESTAMPTZ);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a span set and an ordered set
 * @sqlop @p <->
 */
double
distance_spanset_orderedset(const SpanSet *ss, const OrderedSet *os)
{
  return distance_span_span(&ss->span, &os->span);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between a span set and a span
 * @sqlop @p <->
 */
double
distance_spanset_span(const SpanSet *ss, const Span *s)
{
  return distance_span_span(&ss->span, s);
}

/**
 * @ingroup libmeos_setspan_dist
 * @brief Return the distance between two span sets
 * @sqlop @p <->
 */
double
distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  return distance_span_span(&ss1->span, &ss2->span);
}

/******************************************************************************/
