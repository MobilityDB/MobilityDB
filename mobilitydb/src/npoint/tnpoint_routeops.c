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
 * @brief Route identifier operators for temporal network points.
 *
 * These operators test the set of routes of temporal network points, which are
 * bigint values. The following operators are defined:
 *    overlaps, contains, contained, same
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/temporal.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_routeops.h"

/*****************************************************************************
 * Generic route functions
 *****************************************************************************/

/**
 * @brief Generic route function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
Routeop_bigint_tnpoint(FunctionCallInfo fcinfo,
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
Routeop_tnpoint_bigint(FunctionCallInfo fcinfo,
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
Routeop_bigintset_tnpoint(FunctionCallInfo fcinfo,
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
Routeop_tnpoint_bigintset(FunctionCallInfo fcinfo,
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
Routeop_npoint_tnpoint(FunctionCallInfo fcinfo,
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
Routeop_tnpoint_npoint(FunctionCallInfo fcinfo,
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
Routeop_tnpoint_tnpoint(FunctionCallInfo fcinfo,
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
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes in a set and the routes of
 * a temporal network point overlap
 * @sqlfn overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigintset_tnpoint(fcinfo, &overlaps_rid_tnpoint_bigintset);
}

/*****************************************************************************/

PGDLLEXPORT Datum Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point and
 * the routes in a set overlap
 * @sqlfn overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigintset(fcinfo, &overlaps_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of two temporal network points overlap
 * @sqlfn overlaps_rid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_tnpoint(fcinfo, &overlaps_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if routes in a set contain the routes of a temporal
 * network point
 * @sqlfn contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigintset_tnpoint(fcinfo, &contains_rid_tnpoint_bigintset);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point
 * contain the routes in a set
 * @sqlfn contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigint(fcinfo, &contains_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point
 * contain the routes in a set
 * @sqlfn contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigintset(fcinfo, &contains_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point
 * contain the route of a network point
 * @sqlfn contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_npoint(fcinfo, &contains_rid_tnpoint_npoint);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of the first temporal network point
 * contain the routes of the second one
 * @sqlfn contains_rid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_tnpoint(fcinfo, &contains_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if a route is contained in the routes of a temporal
 * network point
 * @sqlfn contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigint_tnpoint(fcinfo, &contained_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes in the set are contained in the routes of
 * a temporal network point
 * @sqlfn contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigintset_tnpoint(fcinfo, &contained_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the route of a network point is contained in
 * the routes of a temporal network point
 * @sqlfn contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_npoint_tnpoint(fcinfo, &contained_rid_npoint_tnpoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of the temporal network point are
 * contained in the routes of a set
 * @sqlfn contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigintset(fcinfo, &contained_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of the first temporal network point
 * are contained in the routes of the second temporal network point
 * @sqlfn contained_rid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_tnpoint(fcinfo, &contained_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PGDLLEXPORT Datum Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if a route and the routes of a temporal network point
 * are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigint_tnpoint(fcinfo, &same_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a set and the routes of a temporal
 * network point are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_bigintset_tnpoint(fcinfo, &same_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the route of a network point and the routes of a
 * temporal network point are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_npoint_tnpoint(fcinfo, &same_rid_tnpoint_npoint);
}

/*****************************************************************************/

PGDLLEXPORT Datum Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point and a route
 * are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigint(fcinfo, &same_rid_tnpoint_bigint);
}

PGDLLEXPORT Datum Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point and the
 * routes of a set are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_bigintset(fcinfo, &same_rid_tnpoint_bigintset);
}

PGDLLEXPORT Datum Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of a temporal network point and the route
 * of a network point are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_npoint(fcinfo, &same_rid_tnpoint_npoint);
}

PGDLLEXPORT Datum Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_route
 * @brief Return true if the routes of two temporal network points are equal
 * @sqlfn same_rid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Routeop_tnpoint_tnpoint(fcinfo, &same_rid_tnpoint_tnpoint);
}

/*****************************************************************************/
