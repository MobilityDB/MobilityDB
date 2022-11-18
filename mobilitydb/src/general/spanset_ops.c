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

#include "general/time_ops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/periodset.h"
#include "general/spanset.h"
#include "general/timestampset.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_spanset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains an element
 * @sqlfunc span_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_spanset_value(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_spanset_value(ss, d, basetype);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set contains a span set
 * @sqlfunc time_contains()
 * @sqlop @s @>
 */
PGDLLEXPORT Datum
Contains_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contains_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a span set
 * @sqlfunc time_contains()
 * @sqlop @s @>
 */
PGDLLEXPORT Datum
Contains_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contains_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set contains the second one
 * @sqlfunc time_contains()
 * @sqlop @s @>
 */
PGDLLEXPORT Datum
Contains_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = contains_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_value_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an element is contained by a span
 * @sqlfunc span_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_value_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_spanset(d, basetype, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span is contained by a span set
 * @sqlfunc time_contained()
 * @sqlop @s <@
 */
PGDLLEXPORT Datum
Contained_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = contained_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set is contained by a span
 * @sqlfunc time_contained()
 * @sqlop @s <@
 */
PGDLLEXPORT Datum
Contained_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contained_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span set is contained by the second one
 * @sqlfunc time_contained()
 * @sqlop @s <@
 */
PGDLLEXPORT Datum
Contained_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = contained_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set overlap
 * @sqlfunc time_overlaps()
 * @sqlop @s &&
 */
PGDLLEXPORT Datum
Overlaps_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span overlap
 * @sqlfunc time_overlaps()
 * @sqlop @s &&
 */
PGDLLEXPORT Datum
Overlaps_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overlaps_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the span sets overlap
 * @sqlfunc time_overlaps()
 * @sqlop @s &&
 */
PGDLLEXPORT Datum
Overlaps_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overlaps_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_span_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a span set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @s -|-
 */
PGDLLEXPORT Datum
Adjacent_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span set and a span are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @s -|-
 */
PGDLLEXPORT Datum
Adjacent_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = adjacent_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the span sets are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @s -|-
 */
PGDLLEXPORT Datum
Adjacent_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = adjacent_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Strictly before of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Left_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly before a span set
 * @sqlfunc time_before()
 * @sqlop @s <<#
 */
PGDLLEXPORT Datum
Left_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = left_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly before a span
 * @sqlfunc time_before()
 * @sqlop @s <<#
 */
PGDLLEXPORT Datum
Left_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = left_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is strictly before the second one
 * @sqlfunc time_before()
 * @sqlop @s <<#
 */
PGDLLEXPORT Datum
Left_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = left_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Right_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly after a span set
 * @sqlfunc time_after()
 * @sqlop @s #>>
 */
PGDLLEXPORT Datum
Right_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = right_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is strictly after a span
 * @sqlfunc time_after()
 * @sqlop @s #>>
 */
PGDLLEXPORT Datum
Right_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = right_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is strictly after the second one
 * @sqlfunc time_after()
 * @sqlop @s #>>
 */
PGDLLEXPORT Datum
Right_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = right_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overleft_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not after a span set
 * @sqlfunc time_overbefore()
 * @sqlop @s &<#
 */
PGDLLEXPORT Datum
Overleft_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overleft_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not after a span set
 * @sqlfunc time_overbefore()
 * @sqlop @s &<#
 */
PGDLLEXPORT Datum
Overleft_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overleft_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is not after the second one
 * @sqlfunc time_overbefore()
 * @sqlop @s &<#
 */
PGDLLEXPORT Datum
Overleft_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overleft_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overright_span_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not before a span set
 * @sqlfunc time_overafter()
 * @sqlop @s #&>
 */
PGDLLEXPORT Datum
Overright_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  bool result = overright_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span set is not before a span
 * @sqlfunc time_overafter()
 * @sqlop @s #&>
 */
PGDLLEXPORT Datum
Overright_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overright_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span set is not before the second one
 * @sqlfunc time_overafter()
 * @sqlop @s #&>
 */
PGDLLEXPORT Datum
Overright_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = overright_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_span_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the periods
 * @sqlfunc time_union()
 * @sqlop @s +
 */
PGDLLEXPORT Datum
Union_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_POINTER(union_span_span(s1, s2));
}

PG_FUNCTION_INFO_V1(Union_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span and a span set
 * @sqlfunc time_union()
 * @sqlop @s +
 */
PGDLLEXPORT Datum
Union_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span set and a span
 * @sqlfunc time_union()
 * @sqlop @s +
 */
PGDLLEXPORT Datum
Union_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = union_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the span sets
 * @sqlfunc time_union()
 * @sqlop @s +
 */
PGDLLEXPORT Datum
Union_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = union_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span and a span set
 * @sqlfunc time_intersection()
 * @sqlop @s *
 */
PGDLLEXPORT Datum
Intersection_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = intersection_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span set and a span
 * @sqlfunc time_intersection()
 * @sqlop @s *
 */
PGDLLEXPORT Datum
Intersection_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = intersection_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of the span sets
 * @sqlfunc time_intersection()
 * @sqlop @s *
 */
PGDLLEXPORT Datum
Intersection_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = intersection_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_span_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of the periods.
 * @sqlfunc time_minus()
 * @sqlop @s -
 */
PGDLLEXPORT Datum
Minus_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  SpanSet *result = minus_span_span(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_span_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span and a span set
 * @sqlfunc time_minus()
 * @sqlop @s -
 */
PGDLLEXPORT Datum
Minus_span_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_GETARG_SPANSET_P(1);
  SpanSet *result = minus_span_spanset(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_spanset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span set and a span
 * @sqlfunc time_minus()
 * @sqlop @s -
 */
PGDLLEXPORT Datum
Minus_spanset_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = minus_spanset_span(ss, s);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_spanset_spanset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of the span sets
 * @sqlfunc time_minus()
 * @sqlop @s -
 */
PGDLLEXPORT Datum
Minus_spanset_spanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  SpanSet *result = minus_spanset_spanset(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************/
