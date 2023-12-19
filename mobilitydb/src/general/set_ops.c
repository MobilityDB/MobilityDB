/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Operators for set types
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************/
/* contains? */

PGDLLEXPORT Datum Contains_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_set_value);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a set contains a value
 * @sqlfn set_contains()
 * @sqlop @p \@>
 */
Datum
Contains_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = contains_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first set contains the second one
 * @sqlfn set_contains()
 * @sqlop @p \@>
 */
Datum
Contains_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = contains_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PGDLLEXPORT Datum Contained_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_value_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if a value is contained in a set
 * @sqlfn set_contained()
 * @sqlop @p <@
 */
Datum
Contained_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = contained_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if the first set is contained in the second one
 * @sqlfn set_contained()
 * @sqlop @p <@
 */
Datum
Contained_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = contained_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

PGDLLEXPORT Datum Overlaps_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_set_set);
/**
 * @ingroup mobilitydb_setspan_topo
 * @brief Return true if two sets overlap
 * @sqlfn set_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = overlaps_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly left of? */

PGDLLEXPORT Datum Left_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value is to the left of a set
 * @sqlfn set_left()
 * @sqlop @p <<
 */
Datum
Left_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = left_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a set is to the left of a value
 * @sqlfn set_left()
 * @sqlop @p <<
 */
Datum
Left_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = left_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Left_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first set is to the left of the second one
 * @sqlfn set_left()
 * @sqlop @p <<
 */
Datum
Left_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = left_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly right of? */

PGDLLEXPORT Datum Right_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value is to the right of a set
 * @sqlfn set_right()
 * @sqlop @p >>
 */
Datum
Right_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = right_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a set is to the right of a value
 * @sqlfn set_right()
 * @sqlop @p >>
 */
Datum
Right_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = right_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Right_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first set is to the right of the second one
 * @sqlfn set_right()
 * @sqlop @p >>
 */
Datum
Right_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = right_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

PGDLLEXPORT Datum Overleft_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value does not extend to the right of a set
 * @sqlfn set_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overleft_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a set does not extend to the right of a value
 * @sqlfn set_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overleft_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overleft_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first set does not extend to the right of the
 * second one
 * @sqlfn set_overleft()
 * @sqlop @p &<
 */
Datum
Overleft_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = overleft_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

PGDLLEXPORT Datum Overright_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_value_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a value does not extend to the left of a set
 * @sqlfn set_overright()
 * @sqlop @p
 */
Datum
Overright_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  bool result = overright_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_set_value);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if a set does not extend to the left of a value
 * @sqlfn set_overright()
 * @sqlop @p &>
 */
Datum
Overright_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  bool result = overright_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overright_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_set_set);
/**
 * @ingroup mobilitydb_setspan_pos
 * @brief Return true if the first set does not extend to the left of the
 * second one
 * @sqlfn set_overright()
 * @sqlop @p &>
 */
Datum
Overright_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = overright_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PGDLLEXPORT Datum Union_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a value and a set
 * @sqlfn set_union()
 * @sqlop @p +
 */
Datum
Union_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Set *result = union_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Union_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of a set and a value
 * @sqlfn set_union()
 * @sqlop @p +
 */
Datum
Union_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Set *result = union_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Union_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the union of two sets
 * @sqlfn set_union()
 * @sqlop @p +
 */
Datum
Union_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  Set *result = union_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PGDLLEXPORT Datum Intersection_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a value and a set
 * @sqlfn set_intersection()
 * @sqlop @p *
 */
Datum
Intersection_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Set *result = intersection_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Intersection_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of a set and a value
 * @sqlfn set_intersection()
 * @sqlop @p *
 */
Datum
Intersection_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Set *result = intersection_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Intersection_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the intersection of two sets
 * @sqlfn set_intersection()
 * @sqlop @p *
 */
Datum
Intersection_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  Set *result = intersection_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed right
 *****************************************************************************/

PGDLLEXPORT Datum Minus_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_value_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a value and a set
 * @sqlfn set_minus()
 * @sqlop @p -
 */
Datum
Minus_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Set *result = minus_value_set(d, basetype, s);
  PG_FREE_IF_COPY(s, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Minus_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_set_value);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of a set and a value
 * @sqlfn set_minus()
 * @sqlop @p -
 */
Datum
Minus_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Set *result = minus_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Minus_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Minus_set_set);
/**
 * @ingroup mobilitydb_setspan_set
 * @brief Return the difference of two sets
 * @sqlfn set_minus()
 * @sqlop @p -
 */
Datum
Minus_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  Set *result = minus_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PGDLLEXPORT Datum Distance_value_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_value_set);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between a value and a set
 * @sqlfn set_distance()
 * @sqlop @p <->
 */
Datum
Distance_value_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Set *s = PG_GETARG_SET_P(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  double result = distance_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_set_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_set_value);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between a set and a value
 * @sqlfn set_distance()
 * @sqlop @p <->
 */
Datum
Distance_set_value(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  double result = distance_set_value(s, d, basetype);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Distance_set_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_set_set);
/**
 * @ingroup mobilitydb_setspan_dist
 * @brief Return the distance between two sets
 * @sqlfn set_distance()
 * @sqlop @p <->
 */
Datum
Distance_set_set(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  double result = distance_set_set(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_FLOAT8(result);
}

/******************************************************************************/
