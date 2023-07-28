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
 * @brief Route identifier operators for temporal network points.
 *
 * These operators test the set of routes of temporal network points, which are
 * bigint values. The following operators are defined:
 *    overlaps, contains, contained, same
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_npoint/tnpoint.h"

/*****************************************************************************
 * Generic route functions
 *****************************************************************************/

/**
 * @brief Return true if the temporal network point and the route satisfy the
 * function
 */
bool
contains_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  return contains_set_value(routes, Int64GetDatum(rid), T_INT8);
}

/**
 * @brief Return true if the temporal network point and the route satisfy the
 * function
 */
bool
contained_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  return contains_rid_tnpoint_bigint(temp, rid, INVERT);
}

/**
 * @brief Return true if the temporal network point and the route satisfy the
 * function
 */
bool
same_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  return (routes->count == 1) &&
    (DatumGetInt64(SET_VAL_N(routes, 0)) == rid);
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal network point and the big integer set
 * satisfy the function
 */
bool
overlaps_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  return overlaps_set_set(routes, s);
}

/**
 * @brief Return true if the temporal network point and the big integer set
 * satisfy the function
 */
bool
contains_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert)
{
  Set *routes = tnpoint_routes(temp);
  return invert ? contains_set_set(s, routes) : contains_set_set(routes, s);
}

/**
 * @brief Return true if the temporal network point and the big integer set
 * satisfy the function
 */
bool
contained_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert)
{
  return contains_rid_tnpoint_bigintset(temp, s, ! invert);
}

/**
 * @brief Return true if the temporal network point and the big integer set
 * satisfy the function
 */
bool
same_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  return set_eq(routes, s);
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal network point and the network point
 * satisfy the function
 */
bool
contains_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(np->rid),
    T_INT8);
  return result;
}

/**
 * @brief Return true if the temporal network point and the network point
 * satisfy the function
 */
bool
contained_rid_npoint_tnpoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  return contains_rid_tnpoint_npoint(temp, np, invert);
}

/**
 * @brief Return true if the temporal network point and the network point
 * satisfy the function
 */
bool
same_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  return (routes->count == 1) &&
    (DatumGetInt64(SET_VAL_N(routes, 0)) == np->rid);
}

/*****************************************************************************/

/**
 * @brief Return true if the two temporal network points satisfy the function
 */
bool
overlaps_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  return overlaps_set_set(routes1, routes2);
}

/**
 * @brief Return true if the two temporal network points satisfy the function
 */
bool
contains_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  return contains_set_set(routes1, routes2);
}

/**
 * @brief Return true if the two temporal network points satisfy the function
 */
bool
contained_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  return contains_set_set(routes2, routes1);
}

/**
 * @brief Return true if the two temporal network points satisfy the function
 */
bool
same_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  return set_eq(routes1, routes2);
}

/*****************************************************************************/

/**
 * @brief Generic route function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_bigint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, int64, bool))
{
  int64 rid = PG_GETARG_INT64(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = func(temp, rid, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for a temporal network point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_bigint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, int64, bool))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 rid = PG_GETARG_INT64(1);
  bool result = func(temp, rid, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_bigintset_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Set *, bool))
{
  Set *s = PG_GETARG_SET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = func(temp, s, INVERT);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_bigintset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Set *, bool))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool result = func(temp, s, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for a network point and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Npoint *, bool))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = func(temp, np, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for a temporal network point and a network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Npoint *, bool))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np =  PG_GETARG_NPOINT_P(1);
  bool result = func(temp, np, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic route function for two temporal network points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Temporal *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = func(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes and the routes of
 * the temporal network point overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &overlaps_rid_tnpoint_bigintset);
}

/*****************************************************************************/

PGDLLEXPORT Datum Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the temporal network point and
 * the routes overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &overlaps_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the temporal network points
 * overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &overlaps_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if big integer set contains the one of the temporal
 * network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &contains_rid_tnpoint_bigintset);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the temporal network point
 * contain the one of the route
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &contains_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the temporal network point
 * contain the routes
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &contains_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the temporal network point
 * contain the one of the network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &contains_rid_tnpoint_npoint);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the first temporal network point
 * contain the one of the second temporal network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &contains_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes is contained in the one of the
 * temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &contained_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes is contained in the one of the
 * temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &contained_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the network point is contained in
 * the one of the temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &contained_rid_npoint_tnpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the temporal network point is
 * contained in the routes
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &contained_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes of the first temporal network point
 * is contained in the one of the second temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &contained_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PGDLLEXPORT Datum Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the route and the temporal
 * network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &same_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the routes and the routes of the
 * temporal network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &same_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the network point and the
 * temporal network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &same_rid_tnpoint_npoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the big integer are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &same_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the big integer set are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &same_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &same_rid_tnpoint_npoint);
}

PGDLLEXPORT Datum Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_tnpoint_routes
 * @brief Return true if the route identifiers of the temporal network points
 * are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &same_rid_tnpoint_tnpoint);
}

/*****************************************************************************/
