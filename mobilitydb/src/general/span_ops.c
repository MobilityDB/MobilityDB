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
 * @brief Operators for span types
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_span_value);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a span contains a value
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(contains_span_value(s, value));
}

PGDLLEXPORT Datum Contains_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_span_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first span contains the second one
 * @sqlfn span_contains()
 * @sqlop @p \@>
 */
Datum
Contains_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contains_span_span(s1, s2));
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_value_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a value is contained in a span
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_value_span(value, s));
}

PGDLLEXPORT Datum Contained_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_span_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first span is contained in the second one
 * @sqlfn span_contained()
 * @sqlop @p <@
 */
Datum
Contained_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_span_span(s1, s2));
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_span_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if two spans overlap
 * @sqlfn span_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overlaps_span_span(s1, s2));
}

/*****************************************************************************
 * Adjacent to (but not overlapping)
 *****************************************************************************/

PGDLLEXPORT Datum Adjacent_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_value_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a value and a span are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_span_value(s, value));
}

PGDLLEXPORT Datum Adjacent_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_span_value);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a span and a value are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(adjacent_span_value(s, value));
}

PGDLLEXPORT Datum Adjacent_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_span_span);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if two spans are adjacent
 * @sqlfn span_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_span_span(s1, s2));
}

/*****************************************************************************
 * Left of
 *****************************************************************************/

PGDLLEXPORT Datum Left_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_value_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value is to the left of a span
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(left_value_span(value, s));
}

PGDLLEXPORT Datum Left_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_span_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a span is to the left of a value
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(left_span_value(s, value));
}

PGDLLEXPORT Datum Left_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_span_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first span is to the left of the second one
 * @sqlfn span_left()
 * @sqlop @p <<
 */
Datum
Left_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(left_span_span(s1, s2));
}

/*****************************************************************************
 * Right of
 *****************************************************************************/

PGDLLEXPORT Datum Right_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_value_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value is to the right of a span
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(right_value_span(value, s));
}

PGDLLEXPORT Datum Right_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_span_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a span is to the right of a value
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(right_span_value(s, value));
}

PGDLLEXPORT Datum Right_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_span_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first span is to the right of the second one
 * @sqlfn span_right()
 * @sqlop @p >>
 */
Datum
Right_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(right_span_span(s1, s2));
}

/*****************************************************************************
 * Does not extend to the right of
 *****************************************************************************/

PGDLLEXPORT Datum Overleft_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_value_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value does not extend to the right of a span
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overleft_value_span(value, s));
}

PGDLLEXPORT Datum Overleft_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_span_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a span does not extend to the right of a value
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(overleft_span_value(s, value));
}

PGDLLEXPORT Datum Overleft_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_span_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first span does not extend to the right of the
 * second one
 * @sqlfn span_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overleft_span_span(s1, s2));
}

/*****************************************************************************
 * Does not extend to the left of
 *****************************************************************************/

PGDLLEXPORT Datum Overright_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_value_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value does not extend to the left of a span
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overright_value_span(value, s));
}

PGDLLEXPORT Datum Overright_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_span_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a span does not extend to the left of a value
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(overright_span_value(s, value));
}

PGDLLEXPORT Datum Overright_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_span_span);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first span does not extend to the left of the
 * second one
 * @sqlfn span_overright()
 * @sqlop @p &>
 */
Datum
Overright_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overright_span_span(s1, s2));
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PGDLLEXPORT Datum Union_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_value_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a value and a span
 * @sqlfn time_union()
 * @sqlop @p +
 */
Datum
Union_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_SPANSET_P(union_value_span(value, s));
}

PGDLLEXPORT Datum Union_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_span_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a span and a value
 * @sqlfn time_union()
 * @sqlop @p +
 */
Datum
Union_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_SPANSET_P(union_span_value(s, value));
}

PGDLLEXPORT Datum Union_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_span_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of two spans
 * @sqlfn time_union()
 * @sqlop @p +
 */
Datum
Union_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_SPANSET_P(union_span_span(s1, s2));
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PGDLLEXPORT Datum Intersection_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_value_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a value and a span
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  Span *result = intersection_value_span(value, s);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Intersection_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_span_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a span and a value
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Intersection_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Span *result = intersection_span_value(s, value);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Intersection_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_span_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of two spans
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
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

PGDLLEXPORT Datum Minus_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_value_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a value and a span
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Minus_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  SpanSet *result = minus_value_span(value, s);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Minus_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_span_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a span and a value
 * @sqlfn span_intersection()
 * @sqlop @p *
 */
Datum
Minus_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  SpanSet *result = minus_span_value(s, value);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Minus_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_span_span);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of two spans
 * @sqlfn time_minus()
 * @sqlop @p -
 */
Datum
Minus_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  SpanSet *result = minus_span_span(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPANSET_P(result);
}


/******************************************************************************
 * Distance functions
 ******************************************************************************/

PGDLLEXPORT Datum Distance_value_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_value_value);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between two values
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_value_value(PG_FUNCTION_ARGS)
{
  Datum value1 = PG_GETARG_DATUM(0);
  Datum value2 = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_DATUM(distance_value_value(value1, value2, basetype));
}

PGDLLEXPORT Datum Distance_value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_value_span);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between a value and a span
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_DATUM(distance_span_value(s, value));
}

PGDLLEXPORT Datum Distance_span_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_span_value);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between a span and a value
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_span_value(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  PG_RETURN_DATUM(distance_span_value(s, value));
}

PGDLLEXPORT Datum Distance_span_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_span_span);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between two spans
 * @sqlfn span_distance()
 * @sqlop @p <->
 */
Datum
Distance_span_span(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_DATUM(distance_span_span(s1, s2));
}

/******************************************************************************/
