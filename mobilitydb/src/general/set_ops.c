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

#include "general/set_ops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/periodset.h"
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************/

/**
 * @brief Return the size in bytes to read from toast to get the basic information
 * from a variable-length time type: Time struct (i.e., OrderedSet
 * or PeriodSet) and bounding box size
*/
uint32_t
time_max_header_size(void)
{
  return double_pad(Max(sizeof(OrderedSet), sizeof(PeriodSet)));
}

/*****************************************************************************/
/* contains? */

PG_FUNCTION_INFO_V1(Contains_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp set contains a timestamp
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first timestamp set contains the second one
 * @sqlfunc time_contains()
 * @sqlop @p @>
 */
PGDLLEXPORT Datum
Contains_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = contains_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(Contained_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if a timestamp is contained by a timestamp set
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the first timestamp set is contained by the second one
 * @sqlfunc time_contained()
 * @sqlop @p <@
 */
PGDLLEXPORT Datum
Contained_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = contained_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

PG_FUNCTION_INFO_V1(Overlaps_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_topo
 * @brief Return true if the timestamp sets overlap
 * @sqlfunc time_overlaps()
 * @sqlop @p &&
 */
PGDLLEXPORT Datum
Overlaps_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = overlaps_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly left of? */

PG_FUNCTION_INFO_V1(Left_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly left a timestamp set
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = left_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly left a timestamp
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = left_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Left_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first timestamp set is strictly left the second one
 * @sqlfunc time_left()
 * @sqlop @p <<#
 */
PGDLLEXPORT Datum
Left_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = left_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly right of? */

PG_FUNCTION_INFO_V1(Right_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is strictly right a timestamp set
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = right_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is strictly right a timestamp
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = right_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Right_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first timestamp set is strictly right the second one
 * @sqlfunc time_right()
 * @sqlop @p #>>
 */
PGDLLEXPORT Datum
Right_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = right_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

PG_FUNCTION_INFO_V1(Overleft_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not right a timestamp set
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overleft_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not right a timestamp
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overleft_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overleft_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first timestamp set is not right the second one
 * @sqlfunc time_overleft()
 * @sqlop @p &<#
 */
PGDLLEXPORT Datum
Overleft_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = overleft_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

PG_FUNCTION_INFO_V1(Overright_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp is not left a timestamp set
 * @sqlfunc time_overright()
 * @sqlop @p
 */
PGDLLEXPORT Datum
Overright_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overright_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if a timestamp set is not left a timestamp
 * @sqlfunc time_overright()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overright_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overright_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_pos
 * @brief Return true if the first timestamp set is not left the second one
 * @sqlfunc time_overright()
 * @sqlop @p #&>
 */
PGDLLEXPORT Datum
Overright_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  bool result = overright_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_value_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the timestamps
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  mobdbType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  mobdbType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  OrderedSet *result = union_value_value(d1, basetype1, d2, basetype2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp and a timestamp set
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  OrderedSet *result = union_value_orderedset(d, basetype, os);
  PG_FREE_IF_COPY(os, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of a timestamp set and a timestamp
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  OrderedSet *result = union_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the union of the timestamp sets
 * @sqlfunc time_union()
 * @sqlop @p +
 */
PGDLLEXPORT Datum
Union_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  OrderedSet *result = union_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_value_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of the timestamps
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_value(PG_FUNCTION_ARGS)
{
  Datum d1 = PG_GETARG_DATUM(0);
  Datum d2 = PG_GETARG_DATUM(1);
  mobdbType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  mobdbType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum result;
  bool found = intersection_value_value(d1, basetype1, d2, basetype2, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp and a timestamp set
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  Datum result;
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool found = intersection_value_orderedset(d, basetype, os, &result);
  PG_FREE_IF_COPY(os, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of a timestamp set and a timestamp
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum result;
  bool found = intersection_orderedset_value(os, d, basetype, &result);
  PG_FREE_IF_COPY(os, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Intersection_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the intersection of the timestamp sets
 * @sqlfunc time_intersection()
 * @sqlop @p *
 */
PGDLLEXPORT Datum
Intersection_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  OrderedSet *result = intersection_orderedset_orderedset(os1, os2);
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
 * @ingroup mobilitydb_spantime_set
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
  mobdbType basetype1 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  mobdbType basetype2 = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  if (! minus_value_value(d1, basetype1, d2, basetype2, &result))
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Minus_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp and a a timestamp set
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result;
  bool found = minus_value_orderedset(d, basetype, os, &result);
  PG_FREE_IF_COPY(os, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of a timestamp set and a timestamp
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_orderedset_value(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  OrderedSet *result = minus_orderedset_value(os, d, basetype);
  PG_FREE_IF_COPY(os, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_set
 * @brief Return the difference of the timestamp sets
 * @sqlfunc time_minus()
 * @sqlop @p -
 */
PGDLLEXPORT Datum
Minus_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  OrderedSet *os1 = PG_GETARG_ORDEREDSET_P(0);
  OrderedSet *os2 = PG_GETARG_ORDEREDSET_P(1);
  OrderedSet *result = minus_orderedset_orderedset(os1, os2);
  PG_FREE_IF_COPY(os1, 0);
  PG_FREE_IF_COPY(os2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_value_orderedset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a timestamp set
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_value_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Datum os = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Span s;
  orderedset_span_slice(os, &s);
  double result = distance_span_value(&s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_orderedset_value);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a timestamp
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_orderedset_value(PG_FUNCTION_ARGS)
{
  Datum os = PG_GETARG_DATUM(0);
  Datum d = PG_GETARG_DATUM(1);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Span s;
  orderedset_span_slice(os, &s);
  double result = distance_span_value(&s, d, basetype);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_orderedset_orderedset);
/**
 * @ingroup mobilitydb_spantime_dist
 * @brief Return the distance in seconds between the timestamp sets
 * @sqlfunc time_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_orderedset_orderedset(PG_FUNCTION_ARGS)
{
  Datum os1 = PG_GETARG_DATUM(0);
  Datum os2 = PG_GETARG_DATUM(1);
  Span s1, s2;
  orderedset_span_slice(os1, &s1);
  orderedset_span_slice(os2, &s2);
  double result = distance_span_span(&s1, &s2);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/
