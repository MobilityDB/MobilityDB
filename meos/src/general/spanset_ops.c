/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a span set contains a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
bool
contains_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  /* Bounding box test */
  if (! contains_span_value(&ss->span, value))
    return false;

  int loc;
  if (! spanset_find_value(ss, value, &loc))
    return false;
  return true;
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
  return contains_spanset_value(ss, DateADTGetDatum(d));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @csqlfn #Contains_spanset_value()
 */
bool
contains_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return contains_spanset_value(ss, TimestampTzGetDatum(t));
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set contains a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Contains_spanset_span()
 */
bool
contains_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
    return contains_span_span(SPANSET_SP_N(ss, 0), s);

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;

  /* Bounding box test */
  if (! cont_span_span(&ss->span, s))
    return false;

  int loc;
  spanset_find_value(ss, s->lower, &loc);
  return cont_span_span(SPANSET_SP_N(ss, loc), s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span contains a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Contains_span_spanset()
 */
bool
contains_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_span_span(s, &ss->span);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if the first span set contains the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Contains_spanset_spanset()
 */
bool
contains_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return contains_span_spanset(SPANSET_SP_N(ss1, 0), ss2);
  if (ss2->count == 1)
    return contains_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;

  /* Bounding box test */
  if (! cont_span_span(&ss1->span, &ss2->span))
    return false;

  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = SPANSET_SP_N(ss1, i);
    const Span *s2 = SPANSET_SP_N(ss2, j);
    if (lf_span_span(s1, s2))
      i++;
    else if (lf_span_span(s2, s1))
      return false;
    else
    {
      /* s1 and s2 overlap */
      if (cont_span_span(s1, s2))
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
  /* If j == ss2->count every span in s2 is contained in a span of s1
     but s1 may have additional spans */
  return (j == ss2->count);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a value is contained in a span set
 * @param[in] value Value
 * @param[in] ss Span set
 */
bool
contained_value_spanset(Datum value, const SpanSet *ss)
{
  return contains_spanset_value(ss, value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return contained_value_spanset(TimestampTzGetDatum(t), ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span is contained in a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Contained_span_spanset()
 */
bool
contained_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_spanset_span(ss, s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set is contained in a span set
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Contained_spanset_span()
 */
bool
contained_spanset_span(const SpanSet *ss, const Span *s)
{
  return contains_span_spanset(s, ss);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if the first span set is contained in the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Contained_spanset_spanset()
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
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a span overlap
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Overlaps_spanset_span()
 */
bool
overlaps_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
    return overlaps_span_span(SPANSET_SP_N(ss, 0), s);

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;

  /* Bounding box test */
  if (! over_span_span(s, &ss->span))
    return false;

  /* Binary search of lower bound of span */
  int loc;
  spanset_find_value(ss, s->lower, &loc);
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s1 = SPANSET_SP_N(ss, i);
    if (over_span_span(s1, s))
      return true;
    if (s->upper < s1->upper)
      break;
  }
  return false;
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a span set overlap
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Overlaps_span_spanset()
 */
bool
overlaps_span_spanset(const Span *s, const SpanSet *ss)
{
  return overlaps_spanset_span(ss, s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if two span sets overlap
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Overlaps_spanset_spanset()
 */
bool
overlaps_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return overlaps_spanset_span(ss2, SPANSET_SP_N(ss1, 0));
  if (ss2->count == 1)
    return overlaps_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;

  /* Bounding box test */
  if (! over_span_span(&ss1->span, &ss2->span))
    return false;

  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = SPANSET_SP_N(ss1, i);
    const Span *s2 = SPANSET_SP_N(ss2, j);
    if (over_span_span(s1, s2))
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
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a span set and a value are adjacent
 * @param[in] ss Span set
 * @param[in] value Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  /*
   * A span set and a value are adjacent if and only if the first or the last
   * span is adjacent to the value
   */
  return adjacent_span_value(SPANSET_SP_N(ss, 0), value) ||
         adjacent_span_value(SPANSET_SP_N(ss, ss->count - 1), value);
}

/**
 * @ingroup meos_internal_setspan_topo
 * @brief Return true if a span set and a value are adjacent
 * @param[in] ss Span set
 * @param[in] value Value
 * @csqlfn #Adjacent_spanset_value()
 */
bool
adjacent_value_spanset(Datum value, const SpanSet *ss)
{
  return adjacent_spanset_value(ss, value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return adjacent_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span set and a span are adjacent
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Adjacent_spanset_span()
 */
bool
adjacent_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
    return adjacent_span_span(SPANSET_SP_N(ss, 0), s);

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;

  const Span *s1 = SPANSET_SP_N(ss, 0);
  const Span *s2 = SPANSET_SP_N(ss, ss->count - 1);
  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (s2->upper == s->lower && s2->upper_inc != s->lower_inc) ||
         (s->upper == s1->lower && s->upper_inc != s1->lower_inc);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if a span and a span set are adjacent
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Adjacent_span_spanset()
 */
bool
adjacent_span_spanset(const Span *s, const SpanSet *ss)
{
  return adjacent_spanset_span(ss, s);
}

/**
 * @ingroup meos_setspan_topo
 * @brief Return true if two span sets are adjacent
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Adjacent_spanset_spanset()
 */
bool
adjacent_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return adjacent_spanset_span(ss2, SPANSET_SP_N(ss1, 0));
  if (ss2->count == 1)
    return adjacent_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;

  const Span *starts1 = SPANSET_SP_N(ss1, 0);
  const Span *ends1 = SPANSET_SP_N(ss1, ss1->count - 1);
  const Span *starts2 = SPANSET_SP_N(ss2, 0);
  const Span *ends2 = SPANSET_SP_N(ss2, ss2->count - 1);
  /*
   * Two spans A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (ends1->upper == starts2->lower && ends1->upper_inc != starts2->lower_inc) ||
    (ends2->upper == starts1->lower && ends2->upper_inc != starts1->lower_inc);
}

/*****************************************************************************
 * Strictly left
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the left of a span set
 * @param[in] value Value
 * @param[in] ss Span set
 */
bool
left_value_spanset(Datum value, const SpanSet *ss)
{
  assert(ss);
  return left_value_span(value, SPANSET_SP_N(ss, 0));
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return left_value_spanset(TimestampTzGetDatum(t), ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is before a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Left_span_spanset()
 */
bool
left_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return lf_span_span(s, SPANSET_SP_N(ss, 0));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span set is to the left of a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
bool
left_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  return left_span_value(SPANSET_SP_N(ss, ss->count - 1), value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return left_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the left a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Left_spanset_span()
 */
bool
left_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return lf_span_span(SPANSET_SP_N(ss, ss->count - 1), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span set is to the left of the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Left_spanset_spanset()
 */
bool
left_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;
  return lf_span_span(SPANSET_SP_N(ss1, ss1->count - 1), SPANSET_SP_N(ss2, 0));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value is to the right of a span set
 * @param[in] value Value
 * @param[in] ss Span set
 */
bool
right_value_spanset(Datum value, const SpanSet *ss)
{
  return left_spanset_value(ss, value);
}

#if MEOS
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
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span is to the right of a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Right_span_spanset()
 */
bool
right_span_spanset(const Span *s, const SpanSet *ss)
{
  return left_spanset_span(ss, s);
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span set is to the right of a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
bool
right_spanset_value(const SpanSet *ss, Datum value)
{
  return left_value_spanset(value, ss);
}

#if MEOS
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
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set is to the right of a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Right_spanset_span()
 */
bool
right_spanset_span(const SpanSet *ss, const Span *s)
{
  return left_span_spanset(s, ss);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span set is to the right of the second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Right_spanset_spanset()
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
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span set does not extend to the right of a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
bool
overleft_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  return overleft_span_value(SPANSET_SP_N(ss, ss->count - 1), value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return overleft_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the right of a span set
 * @param[in] value Value
 * @param[in] ss Span set
 */
bool
overleft_value_spanset(Datum value, const SpanSet *ss)
{
  assert(ss);
  return overleft_value_span(value, SPANSET_SP_N(ss, ss->count - 1));
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return overleft_value_spanset(TimestampTzGetDatum(t), ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the right of a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Overleft_span_spanset()
 */
bool
overleft_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return ovlf_span_span(s, SPANSET_SP_N(ss, ss->count - 1));
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the right of a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Overleft_spanset_span()
 */
bool
overleft_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return ovlf_span_span(SPANSET_SP_N(ss, ss->count - 1), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span set does not extend to the right of the
 * second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Overleft_spanset_spanset()
 */
bool
overleft_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;
  return ovlf_span_span(SPANSET_SP_N(ss1, ss1->count - 1),
    SPANSET_SP_N(ss2, ss2->count - 1));
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a value does not extend to the left of a span set
 * @param[in] value Value
 * @param[in] ss Span set
" */
bool
overright_value_spanset(Datum value, const SpanSet *ss)
{
  assert(ss);
  return overright_value_span(value, SPANSET_SP_N(ss, 0));
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return overright_value_spanset(TimestampTzGetDatum(t), ss);
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span does not extend to the left of a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Overright_span_spanset()
 */
bool
overright_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return ovri_span_span(s, SPANSET_SP_N(ss, 0));
}

/**
 * @ingroup meos_internal_setspan_pos
 * @brief Return true if a span set does not extend to the left of a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
bool
overright_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  return overright_span_value(SPANSET_SP_N(ss, 0), value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return false;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return false;
  return overright_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if a span set does not extend to the left of a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Overright_spanset_span()
 */
bool
overright_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return false;
  return ovri_span_span(SPANSET_SP_N(ss, 0), s);
}

/**
 * @ingroup meos_setspan_pos
 * @brief Return true if the first span set does not extend to the left of the
 * second one
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Overright_spanset_spanset()
 */
bool
overright_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return false;
  return ovri_span_span(SPANSET_SP_N(ss1, 0), SPANSET_SP_N(ss2, 0));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a span set and a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
SpanSet *
union_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  Span s;
  span_set(value, value, true, true, ss->basetype, ss->spantype, &s);
  return union_spanset_span(ss, &s);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the union of a value and a span set
 * @param[in] ss Span set
 * @param[in] value Value
 */
SpanSet *
union_value_spanset(Datum value, const SpanSet *ss)
{
  return union_spanset_value(ss, value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return NULL;
  return union_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span set and a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Union_spanset_span()
 */
SpanSet *
union_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
    return union_span_span(SPANSET_SP_N(ss, 0), s);

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_span_type(SPANSET_SP_N(ss, 0), s))
    return NULL;

  /* Is the span set fully contained in the span? */
  if (cont_span_span(s, &ss->span))
    return span_spanset(s);

  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int i = 0, j = 0, nspans = 0;
  while (i < ss->count)
  {
    const Span *s1 = SPANSET_SP_N(ss, i);
    /* If the i-th component span is to the left of the argument span */
    if (lfnadj_span_span(s1, s))
    {
      spans[nspans++] = *s1;
      i++;
    }
    /* If the i-th component span is to the right of the argument span */
    else if (lfnadj_span_span(s, s1))
    {
      spans[nspans++] = *s;
      j++;
      break;
    }
    /* The two spans overlap */
    else
    {
      /* Find all spans in ss that overlap with s
       *      i           i
       *   |-----| |-| |-----|
       *       |---------|
       *            s
       */
      Span s2;
      bbox_union_span_span(s1, s, &s2);
      i++;
      while (i < ss->count)
      {
        s1 = SPANSET_SP_N(ss, i);
        if (ovadj_span_span(s1, &s2))
        {
          span_expand(s1, &s2);
          i++;
        }
        else
          break;
      }
      spans[nspans++] = s2;
      j++;
      break;
    }
  }
  /* Add the argument span if it is to rigth of the spanset */
  if (j == 0)
    spans[nspans++] = *s;
  /* Add the remaining component spans if any are left */
  while (i < ss->count)
    spans[nspans++] = *SPANSET_SP_N(ss, i++);
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of a span and a span set
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Union_span_spanset()
 */
SpanSet *
union_span_spanset(const Span *s, const SpanSet *ss)
{
  return union_spanset_span(ss, s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the union of two span sets
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Union_spanset_spanset()
 */
SpanSet *
union_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return union_spanset_span(ss2, SPANSET_SP_N(ss1, 0));
  if (ss2->count == 1)
    return union_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return NULL;

  Span *spans = palloc(sizeof(Span) * (ss1->count + ss2->count));
  int i = 0, j = 0, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = SPANSET_SP_N(ss1, i);
    const Span *s2 = SPANSET_SP_N(ss2, j);
    /* The spans do not overlap, copy the earliest span */
    if (lfnadj_span_span(s1, s2))
    {
      spans[nspans++] = *s1;
      i++;
    }
    else if (lfnadj_span_span(s2, s1))
    {
      spans[nspans++] = *s2;
      j++;
    }
    /* The spans overlap */
    else
    {
      /* Find all spans in ss1 that overlap with spans in ss2
       *      i                    i
       *   |-----| |-| |-----|  |-----|
       *       |---------|  |-----|
       *            j          j
       */
      Span s;
      bbox_union_span_span(s1, s2, &s);
      i++; j++;
      while (i < ss1->count || j < ss2->count)
      {
        /* First verify whether there is overlapping */
        int k = 0;
        if (i < ss1->count)
        {
          s1 = SPANSET_SP_N(ss1, i);
          if (ovadj_span_span(s1, &s))
          {
            span_expand(s1, &s);
            i++; k++;
          }
        }
        if (j < ss2->count)
        {
          s2 = SPANSET_SP_N(ss2, j);
          if (ovadj_span_span(s2, &s))
          {
            span_expand(s2, &s);
            j++; k++;
          }
        }
        /* If no overlapping have been found */
        if (k == 0)
          break;
      }
      spans[nspans++] = s;
    }
  }
  /* Only one of the following two while will be executed */
  while (i < ss1->count)
    spans[nspans++] = *SPANSET_SP_N(ss1, i++);
  while (j < ss2->count)
    spans[nspans++] = *SPANSET_SP_N(ss2, j++);
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the intersection of a span set and a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
SpanSet *
intersection_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  if (! contains_spanset_value(ss, value))
    return NULL;
  return value_to_spanset(value, ss->basetype);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the intersection of a value and a span set
 * @param[in] ss Span set
 * @param[in] value Value
 */
SpanSet *
intersection_value_spanset(Datum value, const SpanSet *ss)
{
  return intersection_spanset_value(ss, value);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return NULL;
  return intersection_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span set and a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Intersection_spanset_span()
 */
SpanSet *
intersection_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
  {
    Span s1;
    if (! inter_span_span(SPANSET_SP_N(ss, 0), s, &s1))
      return NULL;
    return spanset_make_exp((Span *) &s1, 1, 1, NORMALIZE_NO, ORDERED);
  }

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return NULL;

  /* Bounding box test */
  if (! over_span_span(s, &ss->span))
    return NULL;

  /* Is the span set fully contained in the span? */
  if (cont_span_span(s, &ss->span))
    return spanset_cp(ss);

  /* General case */
  int loc;
  spanset_find_value(ss, s->lower, &loc);
  Span *spans = palloc(sizeof(Span) * (ss->count - loc));
  int nspans = 0;
  for (int i = loc; i < ss->count; i++)
  {
    const Span *s1 = SPANSET_SP_N(ss, i);
    Span s2;
    if (inter_span_span(s1, s, &s2))
      spans[nspans++] = s2;
    if (s->upper < s1->upper)
      break;
  }
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of a span and a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Intersection_span_spanset()
 */
SpanSet *
intersection_span_spanset(const Span *s, const SpanSet *ss)
{
  return intersection_spanset_span(ss, s);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the intersection of two span sets
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Intersection_spanset_spanset()
 */
SpanSet *
intersection_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return intersection_spanset_span(ss2, SPANSET_SP_N(ss1, 0));
  if (ss2->count == 1)
    return intersection_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return NULL;

  /* Bounding box test */
  Span s;
  if (! inter_span_span(&ss1->span, &ss2->span, &s))
    return NULL;

  int loc1, loc2;
  spanset_find_value(ss1, s.lower, &loc1);
  spanset_find_value(ss2, s.lower, &loc2);
  Span *spans = palloc(sizeof(Span) * (ss1->count + ss2->count - loc1 - loc2));
  int i = loc1, j = loc2, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = SPANSET_SP_N(ss1, i);
    const Span *s2 = SPANSET_SP_N(ss2, j);
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
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a value and a span set
 * @param[in] value Value
 * @param[in] ss Span set
 */
SpanSet *
minus_value_spanset(Datum value, const SpanSet *ss)
{
  assert(ss);
  if (contains_spanset_value(ss, value))
    return NULL;
  return value_to_spanset(value, ss->basetype);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return NULL;
  return minus_value_spanset(TimestampTzGetDatum(t), ss);
}
#endif /* MEOS */

/**
 * @brief Return the last argument initialized with the difference of a span
 * and a span set
 */
static int
mi_span_spanset(const Span *s, const SpanSet *ss, int from, int to,
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
    const Span *s1 = SPANSET_SP_N(ss, i);
    /* If the remaining spans are to the left of the current span */
    if (lfnadj_span_span(&curr, s1))
    {
      result[nspans++] = curr;
      break;
    }
    Span minus[2];
    int nminus = mi_span_span(&curr, s1, minus);
    /* minus can have from 0 to 2 spans */
    if (nminus == 0)
      break;
    else if (nminus == 1)
      curr = minus[0];
    else /* nminus == 2 */
    {
      result[nspans++] = minus[0];
      curr = minus[1];
    }
    /* If there are no more spans left */
    if (i == to - 1)
      result[nspans++] = curr;
  }
  return nspans;
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span and a span set
 * @param[in] s Span
 * @param[in] ss Span set
 * @csqlfn #Minus_span_spanset()
 */
SpanSet *
minus_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Singleton span set */
  if (ss->count == 1)
    return minus_span_span(s, SPANSET_SP_N(ss, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return NULL;

  /* Bounding box test */
  if (! over_span_span(s, &ss->span))
    return span_spanset(s);

  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int count = mi_span_spanset(s, ss, 0, ss->count, spans);
  return spanset_make_free(spans, count, NORMALIZE_NO, ORDERED);
}

/**
 * @ingroup meos_internal_setspan_set
 * @brief Return the difference of a span set and a value
 * @param[in] ss Span set
 * @param[in] value Value
 */
SpanSet *
minus_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  /* Bounding box test */
  if (! contains_span_value(&ss->span, value))
    return spanset_cp(ss);

  /* At most one composing span can be split into two */
  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int nspans = 0;
  for (int i = 0; i < ss->count; i++)
    nspans += mi_span_value(SPANSET_SP_N(ss, i), value, &spans[nspans]);
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return NULL;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return NULL;
  return minus_spanset_value(ss, TimestampTzGetDatum(t));
}
#endif /* MEOS */

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of a span set and a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @csqlfn #Minus_spanset_span()
 */
SpanSet *
minus_spanset_span(const SpanSet *ss, const Span *s)
{
  /* Singleton span set */
  if (ss->count == 1)
    return minus_span_span(SPANSET_SP_N(ss, 0), s);

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_same_spanset_span_type(ss, s))
    return NULL;

  /* Bounding box test */
  if (! over_span_span(&ss->span, s))
    return spanset_cp(ss);

  /* At most one composing span can be split into two */
  Span *spans = palloc(sizeof(Span) * (ss->count + 1));
  int nspans = 0;
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s1 = SPANSET_SP_N(ss, i);
    nspans += mi_span_span(s1, s, &spans[nspans]);
  }
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/**
 * @ingroup meos_setspan_set
 * @brief Return the difference of two span sets
 * @param[in] ss1,ss2 Span sets
 * @csqlfn #Minus_spanset_spanset()
 */
SpanSet *
minus_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Singleton span set */
  if (ss1->count == 1)
    return minus_span_spanset(SPANSET_SP_N(ss1, 0), ss2);
  if (ss2->count == 1)
    return minus_spanset_span(ss1, SPANSET_SP_N(ss2, 0));

  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_same_spanset_type(ss1, ss2))
    return NULL;

  /* Bounding box test */
  if (! over_span_span(&ss1->span, &ss2->span))
    return spanset_cp(ss1);

  Span *spans = palloc(sizeof(Span) * (ss1->count + ss2->count));
  int i = 0, j = 0, nspans = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const Span *s1 = SPANSET_SP_N(ss1, i);
    const Span *s2 = SPANSET_SP_N(ss2, j);
    /* The spans do not overlap, copy the first span */
    if (! over_span_span(s1, s2))
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
       *         j                        k
       */
      int k;
      for (k = j; k < ss2->count; k++)
      {
        const Span *s3 = SPANSET_SP_N(ss2, k);
        if (! over_span_span(s1, s3))
          break;
      }
      int to = Min(k, ss2->count);
      /* Compute the difference of the overlapping spans */
      nspans += mi_span_spanset(s1, ss2, j, to, &spans[nspans]);
      i++;
      j = k;
    }
  }
  /* Copy the sequences after the span set */
  while (i < ss1->count)
    spans[nspans++] = *SPANSET_SP_N(ss1, i++);
  return spanset_make_free(spans, nspans, NORMALIZE_NO, ORDERED);
}

/******************************************************************************
 * Distance functions
 ******************************************************************************/

/**
 * @ingroup meos_internal_setspan_dist
 * @param[in] ss Span set
 * @param[in] value Value
 * @brief Return the distance between a span set and a value
 */
Datum
distance_spanset_value(const SpanSet *ss, Datum value)
{
  assert(ss);
  return distance_span_value(&ss->span, value);
}

#if MEOS
/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and an integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @result On error return -1
 * @csqlfn #Distance_spanset_value()
 */
int
distance_spanset_int(const SpanSet *ss, int i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4))
    return -1;
  return DatumGetInt32(distance_spanset_value(ss, Int32GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and a big integer
 * @param[in] ss Span set
 * @param[in] i Value
 * @result On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
int64
distance_spanset_bigint(const SpanSet *ss, int64 i)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8))
    return -1;
  return DatumGetInt64(distance_spanset_value(ss, Int64GetDatum(i)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a span set and a float
 * @param[in] ss Span set
 * @param[in] d Value
 * @result On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
double
distance_spanset_float(const SpanSet *ss, double d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8))
    return -1.0;
  return DatumGetFloat8(distance_spanset_value(ss, Float8GetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a span set and a date as a
 * double
 * @param[in] ss Span set
 * @param[in] d Value
 * @result On error return -1
 * @csqlfn #Distance_spanset_value()
 */
int
distance_spanset_date(const SpanSet *ss, DateADT d)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_DATE))
    return -1;
  return DatumGetInt32(distance_spanset_value(ss, DateADTGetDatum(d)));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in seconds between a span set and a timestamptz
 * @param[in] ss Span set
 * @param[in] t Value
 * @result On error return -1.0
 * @csqlfn #Distance_spanset_value()
 */
double
distance_spanset_timestamptz(const SpanSet *ss, TimestampTz t)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ))
    return -1.0;
  return DatumGetFloat8(distance_spanset_value(ss, TimestampTzGetDatum(t)));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between a span set and a span
 * @param[in] ss Span set
 * @param[in] s Span
 * @result On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
Datum
distance_spanset_span(const SpanSet *ss, const Span *s)
{
  assert(ss); assert(ss->spantype == s->spantype);
  return dist_span_span(&ss->span, s);
}

#if MEOS
/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between an integer span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Distance_spanset_span()
 */
int
distance_intspanset_intspan(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_spanset_isof_basetype(ss, T_INT4) ||
      ! ensure_span_isof_basetype(s, T_INT4))
    return -1;
  return DatumGetInt32(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between a big integer span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Distance_spanset_span()
 */
int64
distance_bigintspanset_bigintspan(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_spanset_isof_basetype(ss, T_INT8) ||
      ! ensure_span_isof_basetype(s, T_INT8))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_spanset_isof_basetype(ss, T_FLOAT8) ||
      ! ensure_span_isof_basetype(s, T_FLOAT8))
    return -1.0;
  return DatumGetFloat8(distance_spanset_span(ss, s));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between a date span set and a span
 * @param[in] ss Spanset
 * @param[in] s Span
 * @return On error return -1
 * @csqlfn #Distance_spanset_span()
 */
int
distance_datespanset_datespan(const SpanSet *ss, const Span *s)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
     ! ensure_spanset_isof_basetype(ss, T_DATE) ||
     ! ensure_span_isof_basetype(s, T_DATE))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) s) ||
      ! ensure_spanset_isof_basetype(ss, T_TIMESTAMPTZ) ||
      ! ensure_span_isof_basetype(s, T_TIMESTAMPTZ))
    return -1.0;
  return DatumGetFloat8(distance_spanset_span(ss, s));
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_setspan_dist
 * @brief Return the distance between two span sets
 * @param[in] ss1,ss2 Span sets
 * @result On error return -1.0
 * @csqlfn #Distance_spanset_span()
 */
Datum
distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  assert(ss1); assert(ss2); assert(ss1->spansettype == ss2->spansettype);
  return dist_span_span(&ss1->span, &ss2->span);
}

#if MEOS
/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two integer span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1
 * @csqlfn #Distance_spanset_spanset()
 */
int
distance_intspanset_intspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_spanset_isof_basetype(ss1, T_INT4) ||
      ! ensure_spanset_isof_basetype(ss2, T_INT4))
    return -1;
  return DatumGetInt32(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance between two big integer span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1
 * @csqlfn #Distance_spanset_spanset()
 */
int64
distance_bigintspanset_bigintspanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_spanset_isof_basetype(ss1, T_INT8) ||
      ! ensure_spanset_isof_basetype(ss2, T_INT8))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_spanset_isof_basetype(ss1, T_FLOAT8) ||
      ! ensure_spanset_isof_basetype(ss2, T_FLOAT8))
    return -1.0;
  return DatumGetFloat8(distance_spanset_spanset(ss1, ss2));
}

/**
 * @ingroup meos_setspan_dist
 * @brief Return the distance in days between two date span sets
 * @param[in] ss1,ss2 Spanset
 * @return On error return -1
 * @csqlfn #Distance_spanset_spanset()
 */
int
distance_datespanset_datespanset(const SpanSet *ss1, const SpanSet *ss2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
     ! ensure_spanset_isof_basetype(ss1, T_DATE) ||
     ! ensure_spanset_isof_basetype(ss2, T_DATE))
    return -1;
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss1) || ! ensure_not_null((void *) ss2) ||
      ! ensure_spanset_isof_basetype(ss1, T_TIMESTAMPTZ) ||
      ! ensure_spanset_isof_basetype(ss2, T_TIMESTAMPTZ))
    return -1.0;
  return DatumGetFloat8(distance_spanset_spanset(ss1, ss2));
}
#endif /* MEOS */

/******************************************************************************/
