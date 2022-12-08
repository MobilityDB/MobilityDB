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
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_span_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains a value
 * @sqlfunc span_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(contains_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Contains_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span contains an ordered set
 * @sqlfunc span_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = contains_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_span_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span contains the second one
 * @sqlfunc span_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contains_span_span(s1, s2));
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_value_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value is contained by a span
 * @sqlfunc span_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(contained_value_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Contained_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set is contained by a span
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = contained_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_span_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first span is contained by the second one
 * @sqlfunc span_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_span_span(s1, s2));
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_span_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the spans overlap
 * @sqlfunc span_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overlaps_span_span(s1, s2));
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Adjacent_value_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a value and a span are adjacent
 * @sqlfunc span_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(adjacent_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Adjacent_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if an ordered set and a span are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = adjacent_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_span_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and a value are adjacent
 * @sqlfunc span_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(adjacent_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Adjacent_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a span and an ordered set are adjacent
 * @sqlfunc time_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = adjacent_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_span_span);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the spans are adjacent
 * @sqlfunc span_adjacent()
 * @sqlop @p -|-
 */
PGDLLEXPORT Datum
Adjacent_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_span_span(s1, s2));
}

/*****************************************************************************
 * Strictly left of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Left_value_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a value is strictly to the left of a span
 * @sqlfunc span_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(left_value_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Left_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is strictly before a span
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = left_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_span_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly to the left of the second one
 * @sqlfunc span_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(left_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Left_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly before an ordered set
 * @sqlfunc time_before()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = left_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_span_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span is strictly to the left of the second one
 * @sqlfunc span_left()
 * @sqlop @p <<
 */
PGDLLEXPORT Datum
Left_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(left_span_span(s1, s2));
}

/*****************************************************************************
 * Strictly right of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Right_value_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a value is strictly to the right of a span
 * @sqlfunc span_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(right_value_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Right_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is strictly to the right of a span
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = right_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}
PG_FUNCTION_INFO_V1(Right_span_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly to the right of a value
 * @sqlfunc span_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(right_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Right_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is strictly after an ordered set
 * @sqlfunc time_after()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = right_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_span_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span is strictly to the right of the second one
 * @sqlfunc span_right()
 * @sqlop @p >>
 */
PGDLLEXPORT Datum
Right_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(right_span_span(s1, s2));
}

/*****************************************************************************
 * Does not extend to right of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overleft_value_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a value is not to right of a span
 * @sqlfunc span_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(overleft_value_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Overleft_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is not after a span
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overleft_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_span_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not to right of a value
 * @sqlfunc span_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(overleft_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Overleft_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not after an ordered set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overleft_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_span_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span is not to right of the second one
 * @sqlfunc span_overleft()
 * @sqlop @p &<
 */
PGDLLEXPORT Datum
Overleft_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overleft_span_span(s1, s2));
}

/*****************************************************************************
 * Does not extend to left of
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overright_value_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a value is not to the left of a span
 * @sqlfunc span_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(overright_value_span(d, basetype, s));
}

PG_FUNCTION_INFO_V1(Overright_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if an ordered set is not left a span
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  bool result = overright_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_span_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not to the left of a value
 * @sqlfunc span_overright()
 * @sqlop @p &>
 */
PGDLLEXPORT Datum
Overright_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_BOOL(overright_span_value(s, d, basetype));
}

PG_FUNCTION_INFO_V1(Overright_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a span is not left an ordered set
 * @sqlfunc time_overafter()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  bool result = overright_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_span_span);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first span is not to the left of the second one
 * @sqlfunc span_overright()
 * @sqlop @p &>
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

PG_FUNCTION_INFO_V1(Union_value_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a value and a span
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  SpanSet *result = union_value_span(d, basetype, s);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of an ordered set and a span
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = union_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_span_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a value and a span
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  SpanSet *result = union_value_span(d, basetype, s);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a span and an ordered set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  PeriodSet *result = union_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_span_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the spans
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_POINTER(union_span_span(s1, s2));
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_value_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a value and a span
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = intersection_value_span(d, basetype, s, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of an ordered set and a span.
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  TimestampSet *result = intersection_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_span_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a value and a span
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum result;
  bool found = intersection_value_span(d, basetype, s, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a span and an ordered set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  OrderedSet *result = intersection_span_orderedset(s, os);
  PG_FREE_IF_COPY(os, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_span_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of the spans
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  Span *result = intersection_span_span(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************
 * Set difference
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_value_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a value and a span
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Minus_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = minus_value_span(d, basetype, s, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Minus_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of an ordered set and period
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_orderedset_span(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  TimestampSet *result = minus_orderedset_span(os, s);
  PG_FREE_IF_COPY(os, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_span_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span and a value
 * @sqlfunc span_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Minus_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  SpanSet *result = minus_span_value(s, d, basetype);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a span and an ordered set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *ss = PG_GETARG_SPAN_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  PeriodSet *result = minus_span_orderedset(ss, os);
  PG_FREE_IF_COPY(ss, 0);
  PG_FREE_IF_COPY(os, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_span_span);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of the spans.
 * @sqlfunc time_minus()
 * @sqlop @p -
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


/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_value_value);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the values
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  mobdbType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  mobdbType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_value_value(d1, d2, basetype1, basetype2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_value_span);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a value and a span
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_value_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  double result = distance_span_value(s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_orderedset_span);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between an ordered set and a span
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_orderedset_span(PG_FUNCTION_ARGS)
{
  Datum os = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  Period p1;
  orderedset_span_slice(os, &p1);
  double result = distance_span_span(&p1, s);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_value);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a span and a value
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_span_value(s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_orderedset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a span and an ordered set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_span_orderedset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum os = PG_GETARG_DATUM(1);
  Period p1;
  orderedset_span_slice(os, &p1);
  double result = distance_span_span(s, &p1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_span_span);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the spans
 * @sqlfunc span_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  double result = distance_span_span(s1, s2);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/
