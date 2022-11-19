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
 * @brief Operators for time types.
 */

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"

/*****************************************************************************/

/**
 * Return the minimum value of the two span base values
 */
Datum
span_value_min(Datum l, Datum r, mobdbType type)
{
  ensure_span_basetype(type);
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Min(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  else if (type == T_INT4)
    return Int32GetDatum(Min(DatumGetInt32(l), DatumGetInt32(r)));
  else /* type == T_FLOAT8 */
    return Float8GetDatum(Min(DatumGetFloat8(l), DatumGetFloat8(r)));
}

/**
 * Return the minimum value of the two span base values
 */
Datum
span_value_max(Datum l, Datum r, mobdbType type)
{
  ensure_span_basetype(type);
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Max(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  else if (type == T_INT4)
    return Int32GetDatum(Max(DatumGetInt32(l), DatumGetInt32(r)));
  else /* type == T_FLOAT8 */
    return Float8GetDatum(Max(DatumGetFloat8(l), DatumGetFloat8(r)));
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_topo
 * @brief Return true if a span contains a value.
 */
bool
contains_span_value(const Span *s, Datum d, mobdbType basetype)
{
  int cmp = datum_cmp2(s->lower, d, s->basetype, basetype);
  if (cmp > 0 || (cmp == 0 && ! s->lower_inc))
    return false;

  cmp = datum_cmp2(s->upper, d, s->basetype, basetype);
  if (cmp < 0 || (cmp == 0 && ! s->upper_inc))
    return false;

  return true;
}

#if MEOS
/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if an integer span contains an integer.
 * @sqlop @p \@>
 */
bool
contains_intspan_int(const Span *s, int i)
{
  return contains_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a float span contains a float.
 * @sqlop @p \@>
 */
bool
contains_floatspan_float(const Span *s, double d)
{
  return contains_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first span contains the second one.
 * @sqlop @p \@>
 */
bool
contains_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
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
 * @ingroup libmeos_int_spantime_topo
 * @brief Return true if a value is contained by a span
 */
bool
contained_value_span(Datum d, mobdbType basetype, const Span *s)
{
  return contains_span_value(s, d, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if an integer is contained by an integer span
 * @sqlop @p <@
 */
bool
contained_int_intspan(int i, const Span *s)
{
  return contains_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a float is contained by a float span
 * @sqlop @p <@
 */
bool
contained_float_floatspan(double d, const Span *s)
{
  return contains_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first span is contained by the second one
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the spans overlap.
 * @sqlop @p &&
 * @pymeosfunc overlap()
 */
bool
overlaps_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
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
 * @ingroup libmeos_int_spantime_topo
 * @brief Return true if a span and a value are adjacent
 */
bool
adjacent_span_value(const Span *s, Datum d, mobdbType basetype)
{
  /*
   * A timestamp A and a span C..D are adjacent if and only if
   * A is adjacent to C, or D is adjacent to A.
   */
  return (datum_eq2(d, s->lower, basetype, s->basetype) && ! s->lower_inc) ||
    (datum_eq2(s->upper, d, s->basetype, basetype) && ! s->upper_inc);
}

/**
 * @ingroup libmeos_int_spantime_topo
 * @brief Return true if a span and a value are adjacent
 */
bool
adjacent_value_span(Datum d, mobdbType basetype, const Span *s)
{
  return adjacent_span_value(s, d, basetype);
}


#if MEOS
/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if an integer span and an integer are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_intspan_int(const Span *s, int i)
{
  return adjacent_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a float span and a float are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_floatspan_float(const Span *s, double d)
{
  return adjacent_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the spans are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_span_span(const Span *s1, const Span *s2)
{
  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  assert(s1->spantype == s2->spantype);
  return (
    (datum_eq2(s1->upper, s2->lower, s1->basetype, s2->basetype) &&
      s1->upper_inc != s2->lower_inc) ||
    (datum_eq2(s2->upper, s1->lower, s2->basetype, s1->basetype) &&
      s2->upper_inc != s1->lower_inc) );
}

/*****************************************************************************
 * Strictly left of
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a value is strictly to the left of a span.
 */
bool
left_value_span(Datum d, mobdbType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->lower, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer is strictly to the left of an integer span.
 * @sqlop @p <<
 */
bool
left_int_intspan(int i, const Span *s)
{
  return left_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float is strictly to the left of a float span.
 * @sqlop @p <<
 */
bool
left_float_floatspan(double d, const Span *s)
{
  return left_value_span(Float8GetDatum(d), T_FLOAT8, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a span is strictly to the left of a value.
 */
bool
left_span_value(const Span *s, Datum d, mobdbType basetype)
{

  int cmp = datum_cmp2(s->upper, d, s->basetype, basetype);
  return (cmp < 0 || (cmp == 0 && ! s->upper_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer span is strictly to the left of an integer.
 * @sqlop @p <<
 */
bool
left_intspan_int(const Span *s, int i)
{
  return left_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float span is strictly to the left of a float.
 * @sqlop @p <<
 */
bool
left_floatspan_float(const Span *s, double d)
{
  return left_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is strictly to the left of the second one.
 * @sqlop @p <<
 */
bool
left_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s1->upper, s2->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || ! s2->lower_inc)));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a value is strictly to the right of a span.
 */
bool
right_value_span(Datum d, mobdbType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->upper, basetype, s->basetype);
  return (cmp > 0 || (cmp == 0 && ! s->upper_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer is strictly to the right of an integer span.
 * @sqlop @p >>
 */
bool
right_int_intspan(int i, const Span *s)
{
  return right_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float is strictly to the right of a float span.
 * @sqlop @p >>
 */
bool
right_float_floatspan(double d, const Span *s)
{
  return right_value_span(Float8GetDatum(d), T_FLOAT8, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a span is strictly to the right of a value
 */
bool
right_span_value(const Span *s, Datum d, mobdbType basetype)
{
  int cmp = datum_cmp2(d, s->lower, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer span is strictly to the right of an integer
 * @sqlop @p >>
 */
bool
right_intspan_int(const Span *s, int i)
{
  return right_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float span is strictly to the right of a float
 * @sqlop @p >>
 */
bool
right_floatspan_float(const Span *s, double d)
{
  return right_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_pos
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
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a value is not to the right of a span.
 */
bool
overleft_value_span(Datum d, mobdbType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->upper, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && s->upper_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer is not to the right of an integer span.
 * @sqlop @p &<
 */
bool
overleft_int_intspan(int i, const Span *s)
{
  return overleft_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float is not to the right of a float span.
 * @sqlop @p &<
 */
bool
overleft_float_floatspan(double d, const Span *s)
{
  return overleft_value_span(Float8GetDatum(d), T_INT4, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a span is not to the right of a value.
 */
bool
overleft_span_value(const Span *s, Datum d, mobdbType basetype)
{
  /* Integer spans are canonicalized and thus their upper bound is exclusive.
   * Therefore, we cannot simply check that s->upper <= d */
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  return overleft_span_span(s, &s1);
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer span is not to the right of an integer.
 * @sqlop @p &<
 */
bool
overleft_intspan_int(const Span *s, int i)
{
  return overleft_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float span is not to the right of a float.
 * @sqlop @p &<
 */
bool
overleft_floatspan_float(const Span *s, double d)
{
  return overleft_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is not to the right of the second one.
 * @sqlop @p &<
 */
bool
overleft_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || s2->upper_inc)));
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a value is not the left of a span.
 */
bool
overright_value_span(Datum d, mobdbType basetype, const Span *s)
{
  int cmp = datum_cmp2(s->lower, d, s->basetype, basetype);
  return (cmp < 0 || (cmp == 0 && s->lower_inc));
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer is not the left of an integer span.
 * @sqlop @p &>
 */
bool
overright_int_intspan(int i, const Span *s)
{
  return overright_value_span(Int32GetDatum(i), T_INT4, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float is not the left of a float span.
 * @sqlop @p &>
 */
bool
overright_float_floatspan(double d, const Span *s)
{
  return overright_value_span(Float8GetDatum(d), T_FLOAT8, s);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a span is not to the left of a value.
 */
bool
overright_span_value(const Span *s, Datum d, mobdbType basetype)
{
  return datum_le2(d, s->lower, basetype, s->basetype);
}

#if MEOS
/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an integer span is not to the left of an integer.
 * @sqlop @p &>
 */
bool
overright_intspan_int(const Span *s, int i)
{
  return overright_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a float span is not to the left of a float.
 * @sqlop @p &>
 */
bool
overright_floatspan_float(const Span *s, double d)
{
  return overright_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is not to the left of the second one.
 * @sqlop @p &>
 */
bool
overright_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s2->lower, s1->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->lower_inc || s2->lower_inc)));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the spans.
 * @sqlop @p +
 */
Span *
bbox_union_span_span(const Span *s1, const Span *s2, bool strict)
{
  assert(s1->spantype == s2->spantype);
  /* If the spans do not overlap */
  if (strict && ! overlaps_span_span(s1, s2) && ! adjacent_span_span(s1, s2))
    elog(ERROR, "The result of span union would not be contiguous");

  /* Compute the union of the overlapping spans */
  Span *result = span_copy(s1);
  span_expand(s2, result);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period and a timestamp
 * @sqlop @p +
 */
SpanSet *
union_span_value(const Span *s, Datum d, mobdbType basetype)
{
  Span s1;
  span_set(d, d, true, true, basetype, &s1);
  SpanSet *result = union_span_span(s, &s1);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp and a period
 * @sqlop @p +
 */
SpanSet *
union_value_span(Datum d, mobdbType basetype, const Span *s)
{
  return union_span_value(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the spans.
 * @sqlop @s +
 */
SpanSet *
union_span_span(const Span *s1, const Span *s2)
{
  /* If the spans do not overlap */
  if (! overlaps_span_span(s1, s2) &&
    !adjacent_span_span(s1, s2))
  {
    const Span *spans[2];
    if (datum_lt(s1->lower, s2->lower, s1->basetype))
    {
      spans[0] = s1;
      spans[1] = s2;
    }
    else
    {
      spans[0] = s2;
      spans[1] = s1;
    }
    SpanSet *result = spanset_make((const Span **) spans, 2, NORMALIZE_NO);
    return result;
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
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period and a timestamp
 * @sqlop @p *
 */
bool
intersection_span_value(const Span *s, Datum d, mobdbType basetype,
  Datum *result)
{
  if (! contains_span_value(s, d, basetype))
    return false;
  *result  = d;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a value and a span
 * @sqlop @p *
 */
bool
intersection_value_span(Datum d, mobdbType basetype, const Span *s,
  Datum *result)
{
  return intersection_span_value(s, d, basetype, result);
}

/**
 * @ingroup libmeos_int_spantime_set
 * @brief Set a span with the result of the intersection of the first two spans
 * @note This function is equivalent to @ref intersection_span_span without
 * memory allocation
 */
bool
inter_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1->spantype == s2->spantype);
  /* Bounding box test */
  if (! overlaps_span_span(s1, s2))
    return false;

  memset(result, 0, sizeof(Span));
  Datum lower = span_value_max(s1->lower, s2->lower, s1->basetype);
  Datum upper = span_value_min(s1->upper, s2->upper, s1->basetype);
  bool lower_inc = s1->lower == s2->lower ? s1->lower_inc && s2->lower_inc :
    ( lower == s1->lower ? s1->lower_inc : s2->lower_inc );
  bool upper_inc = s1->upper == s2->upper ? s1->upper_inc && s2->upper_inc :
    ( upper == s1->upper ? s1->upper_inc : s2->upper_inc );
  span_set(lower, upper, lower_inc, upper_inc, s1->basetype, result);
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the spans.
 * @sqlop @p *
 */
Span *
intersection_span_span(const Span *s1, const Span *s2)
{
  Span *result = palloc(sizeof(Span));
  if (! inter_span_span(s1, s2, result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************
 * Set difference
 *****************************************************************************/

/**
 * @brief Return the difference of a span and a value.
 */
int
minus_span_value1(const Span *s, Datum d, mobdbType basetype, Span **result)
{
  if (! contains_span_value(s, d, basetype))
  {
    result[0] = span_copy(s);
    return 1;
  }

  bool eqlower = datum_eq(s->lower, d, basetype);
  bool equpper = datum_eq(s->upper, d, basetype);
  if (eqlower && equpper)
    return 0;

  if (eqlower)
  {
    result[0] = span_make(s->lower, s->upper, false, s->upper_inc, basetype);
    return 1;
  }

  if (equpper)
  {
    result[0] = span_make(s->lower, s->upper, s->lower_inc, false, basetype);
    return 1;
  }

  result[0] = span_make(s->lower, d, s->lower_inc, false, basetype);
  result[1] = span_make(d, s->upper, false, s->upper_inc, basetype);
  return 2;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a span and a value.
 * @sqlop @p -
 */
SpanSet *
minus_span_value(const Span *s, Datum d, mobdbType basetype)
{
  Span *spans[2];
  int count = minus_span_value1(s, d, basetype, spans);
  if (count == 0)
    return NULL;
  SpanSet *result = spanset_make((const Span **) spans, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(spans[i]);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a value and a span
 * @sqlop @p -
 */
bool
minus_value_span(Datum d, mobdbType basetype, const Span *s,
  Datum *result)
{
  if (contains_span_value(s, d, basetype))
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the spans.
 * @sqlop @p -
 */
Span *
bbox_minus_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  /* Deserialize the spans */
  SpanBound lower1, lower2, upper1, upper2;
  span_deserialize(s1, &lower1, &upper1);
  span_deserialize(s2, &lower2, &upper2);

  /* Compare all bounds */
  int cmp_l1l2 = span_bound_cmp(&lower1, &lower2);
  int cmp_l1u2 = span_bound_cmp(&lower1, &upper2);
  int cmp_u1l2 = span_bound_cmp(&upper1, &lower2);
  int cmp_u1u2 = span_bound_cmp(&upper1, &upper2);

  if (cmp_l1l2 < 0 && cmp_u1u2 > 0)
    elog(ERROR, "The result of span difference would not be contiguous");

  /* Result is empty
   * s1         |----|
   * s2      |----------|
   */
  if (cmp_l1l2 >= 0 && cmp_u1u2 <= 0)
    return NULL;

  /* Result is a span */
  /*
   * s1         |----|
   * s2  |----|
   * s2                 |----|
   * result     |----|
   */
  if (cmp_l1u2 > 0 || cmp_u1l2 < 0)
    return span_copy(s1);

  /*
   * s1           |-----|
   * s2               |----|
   * result       |---|
   */
  else if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    return span_make(s1->lower, s2->lower, s1->lower_inc, !(s2->lower_inc),
      s1->basetype);
  /*
   * s1         |-----|
   * s2      |----|
   * result       |---|
   */
  else /* cmp_l1l2 >= 0 && cmp_u1u2 >= 0 */
    return span_make(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

/**
 * @ingroup libmeos_int_spantime_dist
 * @brief Return the distance between the values
 */
double
distance_value_value(Datum l, Datum r, mobdbType typel, mobdbType typer)
{
  ensure_span_basetype(typel);
  if (typel != typer)
    ensure_span_basetype(typer);
  if (typel == T_INT4 && typer == T_INT4)
    return fabs((double) DatumGetInt32(l) - (double) DatumGetInt32(r));
  if (typel == T_INT4 && typer == T_FLOAT8)
    return fabs((double) DatumGetInt32(l) - DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return fabs(DatumGetFloat8(l) - (double) DatumGetInt32(r));
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return fabs(DatumGetFloat8(l) - DatumGetFloat8(r));
  /* Distance in seconds if the base type is TimestampTz */
  else /* typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ */
    return (double) labs((DatumGetTimestampTz(l) - DatumGetTimestampTz(r))) /
      USECS_PER_SEC;
}

/**
 * @ingroup libmeos_int_spantime_dist
 * @brief Return the distance between a span and a value.
 */
double
distance_span_value(const Span *s, Datum d, mobdbType basetype)
{
  /* If the span contains the value return 0 */
  if (contains_span_value(s, d, basetype))
    return 0.0;

  /* If the span is to the right of the value return the distance
   * between the value and the lower bound of the span
   *     d   [---- s ----] */
  if (right_span_value(s, d, basetype))
    return distance_value_value(d, s->lower, basetype, s->basetype);

  /* If the span is to the left of the value return the distance
   * between the upper bound of the span and value
   *     [---- s ----]   d */
  return distance_value_value(s->upper, d, s->basetype, basetype);
}

#if MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between an integer span and an integer.
 * @sqlop @p <->
 */
double
distance_intspan_int(const Span *s, int i)
{
  return distance_span_value(s, Int32GetDatum(i), T_INT4);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a float span and a float.
 * @sqlop @p <->
 */
double
distance_floatspan_float(const Span *s, double d)
{
  return distance_span_value(s, Float8GetDatum(d), T_FLOAT8);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between the spans.
 * @sqlop @p <->
 */
double
distance_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);

  /* If the spans intersect return 0 */
  if (overlaps_span_span(s1, s2))
    return 0.0;

  /* If the first span is to the left of the second one return the distance
   * between the upper bound of the first and lower bound of the second
   *     [---- s1 ----]   [---- s2 ----] */
  if (left_span_span(s1, s2))
    return distance_value_value(s1->upper, s2->lower, s1->basetype, s2->basetype);

  /* If the first span is to the right of the second one return the distance
   * between the upper bound of the second and the lower bound of the first
   *     [---- s2 ----]   [---- s1 ----] */
  return distance_value_value(s2->upper, s1->lower, s2->basetype, s1->basetype);
}

/******************************************************************************/
