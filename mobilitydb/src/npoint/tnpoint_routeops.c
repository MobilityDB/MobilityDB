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
 * @brief Route identfier operators for temporal network points.
 *
 * These operators test the set of routes temporal npoints, which are bigint
 * values. The following operators are defined:
 *    overlaps, contains, contained, same
 */

// #include "npoint/tnpoint_boxops.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "general/set.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_util.h"
// #include "pg_point/tpoint_boxops.h"
#include "pg_npoint/tnpoint.h"

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @brief Generic box function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_bigint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  int64 rid = PG_GETARG_INT64(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = routeop_tnpoint_bigint(temp, rid, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal network point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_route_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 rid = PG_GETARG_INT64(0);
  int result = routeop_tnpoint_bigint(temp, rid, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_bigintset_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = routeop_tnpoint_bigintset(temp, os, func, INVERT);
  PG_FREE_IF_COPY(os, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_bigintset_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(1);
  int result = routeop_tnpoint_bigintset(temp, os, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a network point and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = routeop_tnpoint_npoint(temp, np, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for a temporal network point and a network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool result = routeop_tnpoint_npoint(temp, np, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Generic box function for two temporal network points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
routeop_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBox *, const STBox *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = routeop_tnpoint_tnpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the route and
 * the temporal network point overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &overlaps_rid_bigint_tnpoint);
}

PG_FUNCTION_INFO_V1(Overlaps_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &overlaps_rid_bigintset_tnpoint);
}

PG_FUNCTION_INFO_V1(Overlaps_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the network point and the
 * temporal network point overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &overlaps_rid_npoint_tnpoint);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the route overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &overlaps_rid_tnpoint_bigint);
}

PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point and
 * the spatiotemporal box overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &overlaps_rid_tnpoint_bigintset);
}

PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the network point overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &overlaps_rid_tnpoint_npoint);
}

PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network points
 * overlap
 * @sqlfunc overlaps_rid()
 * @sqlop @p @@
 */
PGDLLEXPORT Datum
Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &overlaps_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the route contains the one of
 * the temporal network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &contains_rid_bigint_tnpoint);
}

PG_FUNCTION_INFO_V1(Contains_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box contains the one of the temporal
 * network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &contains_rid_bigintset_tnpoint);
}

PG_FUNCTION_INFO_V1(Contains_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the network point contains the one
 * of the temporal network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &contains_rid_npoint_tnpoint);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the route
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &contains_rid_tnpoint_bigint);
}

PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the spatiotemporal box
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &contains_rid_tnpoint_bigintset);
}

PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &contains_rid_tnpoint_npoint);
}

PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the first temporal network point
 * contain the one of the second temporal network point
 * @sqlfunc contains_rid()
 * @sqlop @p \@?
 */
PGDLLEXPORT Datum
Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &contains_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the route is contained by the
 * one of the temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &contained_rid_bigint_tnpoint);
}

PG_FUNCTION_INFO_V1(Contained_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box is contained by the one of the
 * temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &contained_rid_bigintset_tnpoint);
}

PG_FUNCTION_INFO_V1(Contained_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the network point is contained by
 * the one of the temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &contained_rid_npoint_tnpoint);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the route
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &contained_rid_tnpoint_bigint);
}

PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the spatiotemporal box
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &contained_rid_tnpoint_bigintset);
}

PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &contained_rid_tnpoint_npoint);
}

PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box of the first temporal network point
 * is contained by the one of the second temporal network point
 * @sqlfunc contained_rid()
 * @sqlop @p ?@
 */
PGDLLEXPORT Datum
Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &contained_rid_tnpoint_tnpoint);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Same_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the route and the temporal
 * network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigint_tnpoint_ext(fcinfo, &same_rid_bigint_tnpoint);
}

PG_FUNCTION_INFO_V1(Same_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of the
 * temporal network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_bigintset_tnpoint_ext(fcinfo, &same_rid_bigintset_tnpoint);
}

PG_FUNCTION_INFO_V1(Same_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the network point and the
 * temporal network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_npoint_tnpoint_ext(fcinfo, &same_rid_npoint_tnpoint);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the big integer are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigint_ext(fcinfo, &same_rid_tnpoint_bigint);
}

PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the big integer set are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_bigintset_ext(fcinfo, &same_rid_tnpoint_bigintset);
}

PG_FUNCTION_INFO_V1(Same_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network point and
 * the network point are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_npoint_ext(fcinfo, &same_rid_tnpoint_npoint);
}

PG_FUNCTION_INFO_V1(Same_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_routes
 * @brief Return true if the route identifiers of the temporal network points
 * are equal
 * @sqlfunc same_rid()
 * @sqlop @p @=
 */
PGDLLEXPORT Datum
Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return routeop_tnpoint_tnpoint_ext(fcinfo, &same_rid_tnpoint_tnpoint);
}

/*****************************************************************************/
