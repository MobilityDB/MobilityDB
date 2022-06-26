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
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"

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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_BOOL(adjacent_span_elem(s, d, basetype));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  mobdbType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
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
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
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

/******************************************************************************/
