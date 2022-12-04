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

#include "general/time_ops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/periodset.h"
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span set contains a value.
 * @sqlop @p \@>
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

/**
 * @ingroup libmeos_spantime_topo
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
  periodset_find_timestamp(ss, s->lower, &loc);
  const Span *s1 = spanset_sp_n(ss, loc);
  return contains_span_span(s1, s);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span contains a span set.
 * @sqlop @p \@>
 */
bool
contains_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_span_span(s, &ss->span);
}

/**
 * @ingroup libmeos_spantime_topo
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
 * @ingroup libmeos_int_spantime_topo
 * @brief Return true if an element is contained by a span
 */
bool
contained_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  return contains_spanset_value(ss, d, basetype);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span is contained by a span
 * @sqlop @p <@
 */
bool
contained_span_spanset(const Span *s, const SpanSet *ss)
{
  return contains_spanset_span(ss, s);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span set is contained by a span
 * @sqlop @p <@
 */
bool
contained_spanset_span(const SpanSet *ss, const Span *s)
{
  return contains_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_spantime_topo
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span and a span set overlap.
 * @sqlop @p &&
 */
bool
overlaps_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Bounding box test */
  if (! overlaps_span_span(s, &ss->span))
    return false;

  /* Binary search of lower bound of span */
  int loc;
  periodset_find_timestamp(ss, s->lower, &loc);
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span set and a span overlap
 * @sqlop @p &&
 */
bool
overlaps_spanset_span(const SpanSet *ss, const Span *s)
{
  return overlaps_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_spantime_topo
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
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(s1->upper),
      DatumGetTimestampTz(s2->upper));
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a value and a span set are adjacent.
 * @sqlop @p -|-
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

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a value and a span set are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  return adjacent_spanset_value(ss, d, basetype);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span and a span set are adjacent.
 * @sqlop @p -|-
 */
bool
adjacent_span_spanset(const Span *s, const SpanSet *ss)
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span set and a span are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_spanset_span(const SpanSet *ss, const Span *s)
{
  return adjacent_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_spantime_topo
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
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span set is strictly to the left of a value.
 * @sqlop @p <<
 */
bool
left_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return left_span_value(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is strictly to the left of a span set.
 * @sqlop @p <<
 */
bool
left_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return left_value_span(d, basetype, s);
}

/**
 * @ingroup libmeos_spantime_pos
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
 * @ingroup libmeos_spantime_pos
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
 * @ingroup libmeos_spantime_pos
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
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span set is strictly to the right of a value.
 * @sqlop @p >>
 */
bool
right_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, 0);
  return right_span_value(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is strictly to the right of a span set.
 * @sqlop @p #&>
 */
bool
right_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return right_value_span(d, basetype, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is strictly to the right of a span set.
 * @sqlop @p >>
 */
bool
right_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return right_span_span(s, s1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span set is strictly after a span.
 * @sqlop @p >>
 */
bool
right_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return right_span_span(s1, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span set is strictly after the second one.
 * @sqlop @p >>
 */
bool
right_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  const Span *s1 = spanset_sp_n(ss1, 0);
  const Span *s2 = spanset_sp_n(ss2, ss2->count - 1);
  return right_span_span(s1, s2);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a value does not extend to the right of a span set.
 * @sqlop @p &<
 */
bool
overleft_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_value(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value does not extend to the right of a span set.
 * @sqlop @p &<
 */
bool
overleft_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, ss->count - 1);
  return overleft_value_span(d, basetype, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is not after a span set.
 * @sqlop @p &<
 */
bool
overleft_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_span(s, s1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span set is not after a span.
 * @sqlop @p &<
 */
bool
overleft_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, ss->count - 1);
  return overleft_span_span(s1, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first a span set is not after the second one.
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
 * Does not extend to left of
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_pos
 * @brief Return true if a span set is not to the left of an element.
 * @sqlop @p &>
 */
bool
overright_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_span_value(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a value is not to the left of a span set.
 * @sqlop @p &>
 */
bool
overright_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  const Span *s = spanset_sp_n(ss, 0);
  return overright_value_span(d, basetype, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is not before a span set.
 * @sqlop @p &>
 */
bool
overright_span_spanset(const Span *s, const SpanSet *ss)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  return overright_span_span(s, s1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span set is not before a span.
 * @sqlop @p &>
 */
bool
overright_spanset_span(const SpanSet *ss, const Span *s)
{
  const Span *s1 = spanset_sp_n(ss, 0);
  return overright_span_span(s1, s);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span set is not before the second one.
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
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a value and a span set
 * @sqlop @p +
 */
SpanSet *
union_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss)
{
  Span s;
  span_set(d, d, true, true, basetype, &s);
  SpanSet *result = union_span_spanset(&s, ss);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @sqlop @p +
 * @brief Return the union of a span and a span set.
 */
SpanSet *
union_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Transform the span into a span set */
  SpanSet *ss1 = span_to_spanset(s);
  /* Call the function for the span set */
  SpanSet *result = union_spanset_spanset(ss1, ss);
  pfree(ss1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a span set and a span
 * @sqlop @p +
 */
SpanSet *
union_spanset_span(const SpanSet *ss, const Span *s)
{
  return union_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the span sets.
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
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a span set and a value
 * @sqlop @p *
 */
bool
intersection_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype,
  Datum *result)
{
  if (! contains_spanset_value(ss, d, basetype))
    return false;
  *result  = d;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a value and a span set
 * @sqlop @p *
 */
bool
intersection_value_spanset(Datum d, mobdbType basetype, const SpanSet *ss,
  Datum *result)
{
  return intersection_spanset_value(ss, d, basetype, result);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a span and a span set.
 * @sqlop @p *
 */
SpanSet *
intersection_span_spanset(const Span *s, const SpanSet *ss)
{
  /* Bounding box test */
  if (! overlaps_span_span(s, &ss->span))
    return NULL;

  /* Is the span set fully contained in the span? */
  if (contains_span_spanset(s, ss))
    return spanset_copy(ss);

  /* General case */
  int loc;
  periodset_find_timestamp(ss, s->lower, &loc);
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
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a span set and a span
 * @sqlop @p *
 */
SpanSet *
intersection_spanset_span(const SpanSet *ss, const Span *s)
{
  return intersection_span_spanset(s, ss);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the span sets.
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
  periodset_find_timestamp(ss1, s.lower, &loc1);
  periodset_find_timestamp(ss2, s.lower, &loc2);
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
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(s1->upper),
      DatumGetTimestampTz(s2->upper));
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
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a span set and a value.
 * @sqlop @p -
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp and a period set
 * @sqlop @p -
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

/**
 * @brief Return the difference of the two spans.
 * @note This function generalizes the function minus_span_span by enabling
 * the result to be two spans
 */
static int
minus_span_span1(const Span *s1, const Span *s2, Span **result)
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
    result[0] = span_make(s1->lower, s2->lower, s1->lower_inc,
      !(s2->lower_inc), s1->basetype);
    result[1] = span_make(s2->upper, s1->upper, !(s2->upper_inc),
      s1->upper_inc, s1->basetype);
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
    result[0] = span_copy(s1);

  /*
   * s1           |-----|
   * s2               |----|
   * result       |---|
   */
  else if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    result[0] = span_make(s1->lower, s2->lower, s1->lower_inc,
      !(s2->lower_inc), s1->basetype);
  /*
   * s1         |-----|
   * s2      |----|
   * result       |---|
   */
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    result[0] = span_make(s2->upper, s1->upper, !(s2->upper_inc),
      s1->upper_inc, s1->basetype);
  return 1;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the spans.
 * @sqlop @p -
 */
SpanSet *
minus_span_span(const Span *s1, const Span *s2)
{
  Span *spans[2];
  int count = minus_span_span1(s1, s2, spans);
  if (count == 0)
    return NULL;
  SpanSet *result = spanset_make((const Span **) spans, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(spans[i]);
  return result;
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
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(curr->upper),
      DatumGetTimestampTz(s1->lower));
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
 * @ingroup libmeos_spantime_set
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
 * @ingroup libmeos_spantime_set
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
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the span sets.
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
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a timestamp and a span set
 * @sqlop @p <->
 */
double
distance_spanset_value(const SpanSet *ss, Datum d, mobdbType basetype)
{
  return distance_span_value(&ss->span, d, basetype);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a span and a span set
 * @sqlop @p <->
 */
double
distance_span_spanset(const Span *s, const SpanSet *ss)
{
  return distance_span_span(&ss->span, s);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a span set and a span
 * @sqlop @p <->
 */
double
distance_spanset_span(const SpanSet *ss, const Span *s)
{
  return distance_span_span(&ss->span, s);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between two span sets
 * @sqlop @p <->
 */
double
distance_spanset_spanset(const SpanSet *ss1, const SpanSet *ss2)
{
  return distance_span_span(&ss1->span, &ss2->span);
}

/******************************************************************************/
