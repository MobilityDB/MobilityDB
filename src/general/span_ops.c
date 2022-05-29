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
 * @file time_ops.c
 * @brief Operators for time types.
 */

#include "general/span_ops.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/temporal_util.h"

/*****************************************************************************/

/**
 * Return the minimum value of the two span base values
 */
Datum
span_elem_min(Datum l, Datum r, CachedType type)
{
  ensure_span_basetype(type);
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Min(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  if (type == T_INT4)
    return Int32GetDatum(Min(DatumGetInt32(l), DatumGetInt32(r)));
  if (type == T_FLOAT8)
    return Float8GetDatum(Min(DatumGetFloat8(l), DatumGetFloat8(r)));
  elog(ERROR, "Unknown Min function for span base type: %d", type);
}

/**
 * Return the minimum value of the two span base values
 */
Datum
span_elem_max(Datum l, Datum r, CachedType type)
{
  ensure_span_basetype(type);
  if (type == T_TIMESTAMPTZ)
    return TimestampTzGetDatum(Max(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r)));
  if (type == T_INT4)
    return Int32GetDatum(Max(DatumGetInt32(l), DatumGetInt32(r)));
  if (type == T_FLOAT8)
    return Float8GetDatum(Max(DatumGetFloat8(l), DatumGetFloat8(r)));
  elog(ERROR, "Unknown Max function for span base type: %d", type);
}

/*****************************************************************************/
/* contains? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span contains an element.
 */
bool
contains_span_elem(const Span *s, Datum d, CachedType basetype)
{
  int cmp = datum_cmp2(s->lower, d, s->basetype, basetype);
  if (cmp > 0 || (cmp == 0 && ! s->lower_inc))
    return false;

  cmp = datum_cmp2(s->upper, d, s->basetype, basetype);
  if (cmp < 0 || (cmp == 0 && ! s->upper_inc))
    return false;

  return true;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first span contains the second one.
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

/*****************************************************************************/
/* contained? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if an element is contained by a span
 */
bool
contained_elem_span(Datum d, CachedType basetype, const Span *s)
{
  return contains_span_elem(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first span is contained by the second one
 */
bool
contained_span_span(const Span *s1, const Span *s2)
{
  return contains_span_span(s2, s1);
}

/*****************************************************************************/
/* overlaps? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the spans overlap.
 */
bool
overlaps_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int c1 = datum_cmp(s1->lower, s2->upper, s1->basetype);
  int c2 = datum_cmp(s2->lower, s1->upper, s2->basetype);
  if (
    (c1 < 0 || (c1 == 0 && s1->lower_inc && s2->upper_inc)) &&
    (c2 < 0 || (c2 == 0 && s2->lower_inc && s1->upper_inc))
  )
    return true;
  return false;
}

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if an element and a span are adjacent.
 */
bool
adjacent_elem_span(Datum d, CachedType basetype, const Span *s)
{
  /*
   * A timestamp A and a span C..D are adjacent if and only if
   * A is adjacent to C, or D is adjacent to A.
   */
  return (datum_eq2(d, s->lower, basetype, s->basetype) && ! s->lower_inc) ||
    (datum_eq2(s->upper, d, s->basetype, basetype) && ! s->upper_inc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a span and an element are adjacent
 */
bool
adjacent_span_elem(const Span *s, Datum d, CachedType basetype)
{
  return adjacent_elem_span(d, basetype, s);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the spans are adjacent.
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

/*****************************************************************************/
/* strictly left of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an element is strictly to the left of a span.
 */
bool
left_elem_span(Datum d, CachedType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->lower, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is strictly to the left of an element.
 */
bool
left_span_elem(const Span *s, Datum d, CachedType basetype)
{

  int cmp = datum_cmp2(s->upper, d, s->basetype, basetype);
  return (cmp < 0 || (cmp == 0 && ! s->upper_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is strictly to the left of the second one.
 */
bool
left_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s1->upper, s2->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || ! s2->lower_inc)));
}

/*****************************************************************************/
/* strictly right of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an element is strictly to the right of a span.
 */
bool
right_elem_span(Datum d, CachedType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->upper, basetype, s->basetype);
  return (cmp > 0 || (cmp == 0 && ! s->upper_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is strictly to the right of an element
 */
bool
right_span_elem(const Span *s, Datum d, CachedType basetype)
{
  int cmp = datum_cmp2(d, s->lower, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && ! s->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is strictly to right the of the second one.
 */
bool
right_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s2->upper, s1->lower, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s2->upper_inc || ! s1->lower_inc)));
}

/*****************************************************************************/
/* does not extend to right of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an element is not to the right of a span.
 */
bool
overleft_elem_span(Datum d, CachedType basetype, const Span *s)
{
  int cmp = datum_cmp2(d, s->upper, basetype, s->basetype);
  return (cmp < 0 || (cmp == 0 && s->upper_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is not to the right of an element.
 */
bool
overleft_span_elem(const Span *s, Datum d, CachedType basetype)
{
  return datum_le2(s->upper, d, s->basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is not to the right of the second one.
 */
bool
overleft_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp = datum_cmp(s1->upper, s2->upper, s1->basetype);
  return (cmp < 0 || (cmp == 0 && (! s1->upper_inc || s2->upper_inc)));
}

/*****************************************************************************/
/* does not extend to left of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if an element is not the left of a span.
 */
bool
overright_elem_span(Datum d, CachedType basetype, const Span *s)
{
  int cmp = datum_cmp2(s->lower, d, s->basetype, basetype);
  return (cmp < 0 || (cmp == 0 && s->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a span is not to the left of an element.
 */
bool
overright_span_elem(const Span *s, Datum d, CachedType basetype)
{
  return datum_le2(d, s->lower, basetype, s->basetype);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first span is not to the left of the second one.
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
 */
Span *
union_span_span(const Span *s1, const Span *s2, bool strict)
{
  assert(s1->spantype == s2->spantype);
  /* If the spans do not overlap */
  if (strict && ! overlaps_span_span(s1, s2) && ! adjacent_span_span(s1, s2))
    elog(ERROR, "The union of the two spans would not be contiguous");

  /* Compute the union of the overlapping spans */
  Span *result = span_copy(s1);
  span_expand(s2, result);
  return result;
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Set a span with the result of the intersection of the first two spans
 *
 * @note This function equivalent is to intersection_span_span but avoids
 * memory allocation
 */
bool
inter_span_span(const Span *s1, const Span *s2, Span *result)
{
  assert(s1->spantype == s2->spantype);
  /* Bounding box test */
  if (! overlaps_span_span(s1, s2))
    return false;

  Datum lower = span_elem_max(s1->lower, s2->lower, s1->basetype);
  Datum upper = span_elem_min(s1->upper, s2->upper, s1->basetype);
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
 */
Span *
intersection_span_span(const Span *s1, const Span *s2)
{
  Span *result = palloc0(sizeof(Span));
  if (! inter_span_span(s1, s2, result))
  {
    pfree(result);
    return NULL;
  }
  return result;
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed right
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the spans.
 */
Span *
minus_span_span(const Span *s1, const Span *s2)
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
    elog(ERROR, "result of span difference would not be contiguous");

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
   * result      |----|
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
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    return span_make(s2->upper, s1->upper, !(s2->upper_inc), s1->upper_inc,
      s1->basetype);

  elog(ERROR, "unexpected case in span_minus");
  return NULL;
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between the elements
 */
double
distance_elem_elem(Datum l, Datum r, CachedType typel, CachedType typer)
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
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return (double) labs((DatumGetTimestampTz(l) - DatumGetTimestampTz(r))) /
      USECS_PER_SEC;
  elog(ERROR, "unknown distance_elem_elem function for span base type: %d",
    typel);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between an element and a span.
 */
double
distance_elem_span(Datum d, CachedType basetype, const Span *s)
{
  return distance_span_elem(s, d, basetype);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between a span and a element.
 */
double
distance_span_elem(const Span *s, Datum d, CachedType basetype)
{
  /* If the span contains the element return 0 */
  if (contains_span_elem(s, d, basetype))
    return 0.0;

  /* If the span is to the right of the element return the distance
   * between the element and the lower bound of the span
   *     d   [---- s ----] */
  if (right_span_elem(s, d, basetype))
    return distance_elem_elem(d, s->lower, basetype, s->basetype);

  /* If the span is to the left of the element return the distance
   * between the upper bound of the span and element
   *     [---- s ----]   d */
  return distance_elem_elem(s->upper, d, s->basetype, basetype);
}

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance between the spans.
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
    return distance_elem_elem(s1->upper, s2->lower, s1->basetype, s2->basetype);

  /* If the first span is to the right of the second one return the distance
   * between the upper bound of the second and the lower bound of the first
   *     [---- s2 ----]   [---- s1 ----] */
  return distance_elem_elem(s2->upper, s1->lower, s2->basetype, s1->basetype);
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************/
/* contains? */

PG_FUNCTION_INFO_V1(Contains_span_elem);
/**
 * Return true if a span contains an element
 */
PGDLLEXPORT Datum
Contains_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(contains_span_elem(s, d, basetype));
}


PG_FUNCTION_INFO_V1(Contains_span_span);
/**
 * Return true if the first span contains the second one
 */
PGDLLEXPORT Datum
Contains_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contains_span_span(s1, s2));
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(Contained_elem_span);
/**
 * Return true if an element is contained by a span
 */
PGDLLEXPORT Datum
Contained_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(contained_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Contained_span_span);
/**
 * Return true if the first span is contained by the second one
 */
PGDLLEXPORT Datum
Contained_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_span_span(s1, s2));
}

/*****************************************************************************/
/* overlaps? */

PG_FUNCTION_INFO_V1(Overlaps_span_span);
/**
 * Return true if the spans overlap
 */
PGDLLEXPORT Datum
Overlaps_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overlaps_span_span(s1, s2));
}

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

PG_FUNCTION_INFO_V1(Adjacent_elem_span);
/**
 * Return true if an element and a span are adjacent
 */
PGDLLEXPORT Datum
Adjacent_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(adjacent_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Adjacent_span_elem);
/**
 * Return true if a span and an element are adjacent
 */
PGDLLEXPORT Datum
Adjacent_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(adjacent_span_elem(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Adjacent_span_span);
/**
 * Return true if the spans are adjacent
 */
PGDLLEXPORT Datum
Adjacent_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_span_span(s1, s2));
}

/*****************************************************************************/
/* strictly left of? */

PG_FUNCTION_INFO_V1(Left_elem_span);
/**
 * Return true if an element is strictly to the left of a span
 */
PGDLLEXPORT Datum
Left_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(left_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Left_span_elem);
/**
 * Return true if a span is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(left_span_elem(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Left_span_span);
/**
 * Return true if the first span is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(left_span_span(s1, s2));
}

/*****************************************************************************/
/* strictly right of? */

PG_FUNCTION_INFO_V1(Right_elem_span);
/**
 * Return true if an element is strictly to the right of a span
 */
PGDLLEXPORT Datum
Right_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(right_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Right_span_elem);
/**
 * Return true if a span is strictly to the right of an element
 */
PGDLLEXPORT Datum
Right_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(right_span_elem(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Right_span_span);
/**
 * Return true if the first span is strictly to the right of the second one
 */
PGDLLEXPORT Datum
Right_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(right_span_span(s1, s2));
}

/*****************************************************************************/
/* does not extend to right of? */

PG_FUNCTION_INFO_V1(Overleft_elem_span);
/**
 * Return true if an element is not to right of a span
 */
PGDLLEXPORT Datum
Overleft_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(overleft_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Overleft_span_elem);
/**
 * Return true if a span is not to right of an element
 */
PGDLLEXPORT Datum
Overleft_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(overleft_span_elem(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Overleft_span_span);
/**
 * Return true if the first span is not to right of the second one
 */
PGDLLEXPORT Datum
Overleft_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overleft_span_span(s1, s2));
}

/*****************************************************************************/
/* does not extend to left of? */

PG_FUNCTION_INFO_V1(Overright_elem_span);
/**
 * Return true if an element is not to the left of a span
 */
PGDLLEXPORT Datum
Overright_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(overright_elem_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Overright_span_elem);
/**
 * Return true if a span is not to the left of an element
 */
PGDLLEXPORT Datum
Overright_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(overright_span_elem(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Overright_span_span);
/**
 * Return true if the first span is not to the left of the second one
 */
PGDLLEXPORT Datum
Overright_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overright_span_span(s1, s2));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_span_span);
/**
 * Return the union of the spans
 */
PGDLLEXPORT Datum
Union_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  /* Strict union */
  PG_RETURN_POINTER(union_span_span(s1, s2, true));
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_span_span);
/**
 * Return the intersection of the spans
 */
PGDLLEXPORT Datum
Intersection_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  Span *result = intersection_span_span(s1, s2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed right
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_span_span);
/**
 * @brief Return the difference of the spans.
 */
PGDLLEXPORT Datum
Minus_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  Span *result = minus_span_span(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_elem_elem);
/**
 * Return the distance in seconds between the elements
 */
PGDLLEXPORT Datum
Distance_elem_elem(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  CachedType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  CachedType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_elem_elem(d1, d2, basetype1, basetype2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_elem_span);
/**
 * Return the distance in seconds between an element and a span
 */
PGDLLEXPORT Datum
Distance_elem_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  double result = distance_span_elem(s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_elem);
/**
 * Return the distance in seconds between a span and an element
 */
PGDLLEXPORT Datum
Distance_span_elem(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_span_elem(s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_span);
/**
 * Return the distance in seconds between the spans
 */
PGDLLEXPORT Datum
Distance_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  double result = distance_span_span(s1, s2);
  PG_RETURN_FLOAT8(result);
}

#endif /* #if ! MEOS */

/******************************************************************************/
