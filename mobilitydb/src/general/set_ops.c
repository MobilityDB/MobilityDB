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
 * @brief Operators for set types.
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/mobdb_catalog.h"

/*****************************************************************************/
/* contains? */

PG_FUNCTION_INFO_V1(Contains_set_value);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if an ordered set contains a value
 * @sqlfunc time_contains()
 * @sqlop @p \@>
 */
PGDLLEXPORT Datum
Contains_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first timestamp set contains the second one
 * @sqlfunc time_contains()
 * @sqlop @p \@>
 */
PGDLLEXPORT Datum
Contains_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = contains_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(Contained_value_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a timestamp is contained by a timestamp set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_set(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first timestamp set is contained by the second one
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = contained_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

PG_FUNCTION_INFO_V1(Overlaps_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the timestamp sets overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = overlaps_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly left of? */

PG_FUNCTION_INFO_V1(Left_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp is strictly left a timestamp set
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = left_value_set(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp set is strictly left a timestamp
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = left_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first timestamp set is strictly left the second one
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = left_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly right of? */

PG_FUNCTION_INFO_V1(Right_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp is strictly right a timestamp set
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = right_value_set(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp set is strictly right a timestamp
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = right_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first timestamp set is strictly right the second one
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = right_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

PG_FUNCTION_INFO_V1(Overleft_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp is not right a timestamp set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overleft_value_set(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp is not right a timestamp
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overleft_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first timestamp set is not right the second one
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = overleft_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

PG_FUNCTION_INFO_V1(Overright_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp is not left a timestamp set
 * @sqlfunc time_overright()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Overright_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overright_value_set(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a timestamp set is not left a timestamp
 * @sqlfunc time_overright()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overright_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first timestamp set is not left the second one
 * @sqlfunc time_overright()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  bool result = overright_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_value_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of the timestamps
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Set *result = union_value_value(d1,d2, basetype);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a timestamp and a timestamp set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Set *result = union_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a timestamp set and a timestamp
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Set *result = union_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of the timestamp sets
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  Set *result = union_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_value_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of the timestamps
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = intersection_value_value(d1, d2, basetype, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a timestamp and a timestamp set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  Datum result;
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool found = intersection_set_value(os, d, basetype, &result);
  PG_FREE_IF_COPY(os, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a timestamp set and a timestamp
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum result;
  bool found = intersection_set_value(os, d, basetype, &result);
  PG_FREE_IF_COPY(os, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of the timestamp sets
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  Set *result = intersection_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed right
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_value_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of the timestamps
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  Datum result;
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  if (! minus_value_value(d1, d2, basetype, &result))
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Minus_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a timestamp and a a timestamp set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *os = PG_GETARG_SET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = minus_value_set(d, basetype, os, &result);
  PG_FREE_IF_COPY(os, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a timestamp set and a timestamp
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_set_value(PG_FUNCTION_ARGS)
{
  Set *os = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Set *result = minus_set_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of the timestamp sets
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_set_set(PG_FUNCTION_ARGS)
{
  Set *os1 = PG_GETARG_SET_P(0);
  Set *os2 = PG_GETARG_SET_P(1);
  Set *result = minus_set_set(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_value_set);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance in seconds between a timestamp and a timestamp set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Datum os = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Span s;
  set_span_slice(os, &s);
  double result = distance_span_value(&s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_set_value);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance in seconds between a timestamp set and a timestamp
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_set_value(PG_FUNCTION_ARGS)
{
  Datum os = PG_GETARG_DATUM(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Span s;
  set_span_slice(os, &s);
  double result = distance_span_value(&s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_set_set);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance in seconds between the timestamp sets
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_set_set(PG_FUNCTION_ARGS)
{
  Datum os1 = PG_GETARG_DATUM(0);
  Datum os2 = PG_GETARG_DATUM(1);
  Span s1, s2;
  set_span_slice(os1, &s1);
  set_span_slice(os2, &s2);
  double result = distance_span_span(&s1, &s2);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/
