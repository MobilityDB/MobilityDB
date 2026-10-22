/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Route identifier operators for temporal network points
 * @details These operators test the set of routes of temporal network points,
 * which are
 * bigint values. The following operators are defined:
 * overlaps, contains, contained, same
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_npoint.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

PGDLLEXPORT Datum Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes in a set and the routes of
 * a temporal network point overlap
 * @sqlfn overlapsRid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = overlaps_rid_bigintset_tnpoint(s, temp);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point and
 * the routes in a set overlap
 * @sqlfn overlapsRid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool result = overlaps_rid_tnpoint_bigintset(temp, s);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of two temporal network points overlap
 * @sqlfn overlapsRid()
 * @sqlop @p @@
 */
Datum
Overlaps_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = overlaps_rid_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PGDLLEXPORT Datum Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if routes in a set contain the routes of a temporal
 * network point
 * @sqlfn containsRid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = contains_rid_bigintset_tnpoint(s, temp);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point
 * contain the routes in a set
 * @sqlfn containsRid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 rid = PG_GETARG_INT64(1);
  bool result = contains_rid_tnpoint_bigint(temp, rid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point
 * contain the routes in a set
 * @sqlfn containsRid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool result = contains_rid_tnpoint_bigintset(temp, s);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point
 * contain the route of a network point
 * @sqlfn containsRid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool result = contains_rid_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of the first temporal network point
 * contain the routes of the second one
 * @sqlfn containsRid()
 * @sqlop @p \@?
 */
Datum
Contains_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = contains_rid_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

PGDLLEXPORT Datum Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if a route is contained in the routes of a temporal
 * network point
 * @sqlfn containedRid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = contained_rid_bigint_tnpoint(rid, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes in the set are contained in the routes of
 * a temporal network point
 * @sqlfn containedRid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = contained_rid_bigintset_tnpoint(s, temp);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the route of a network point is contained in
 * the routes of a temporal network point
 * @sqlfn containedRid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = contained_rid_npoint_tnpoint(np, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of the temporal network point are
 * contained in the routes of a set
 * @sqlfn containedRid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool result = contained_rid_tnpoint_bigintset(temp, s);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of the first temporal network point
 * are contained in the routes of the second temporal network point
 * @sqlfn containedRid()
 * @sqlop @p ?@
 */
Datum
Contained_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = contained_rid_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Same
 *****************************************************************************/

PGDLLEXPORT Datum Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if a route and the routes of a temporal network point
 * are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigint_tnpoint(PG_FUNCTION_ARGS)
{
  int64 rid = PG_GETARG_INT64(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = same_rid_bigint_tnpoint(rid, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_bigintset_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a set and the routes of a temporal
 * network point are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_bigintset_tnpoint(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = same_rid_bigintset_tnpoint(s, temp);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_npoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the route of a network point and the routes of a
 * temporal network point are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = same_rid_npoint_tnpoint(np, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point and a route
 * are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 rid = PG_GETARG_INT64(1);
  bool result = same_rid_tnpoint_bigint(temp, rid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_bigintset);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point and the
 * routes of a set are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_bigintset(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  bool result = same_rid_tnpoint_bigintset(temp, s);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_npoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of a temporal network point and the route
 * of a network point are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool result = same_rid_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_rid_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_npoint_route
 * @brief Return true if the routes of two temporal network points are equal
 * @sqlfn sameRid()
 * @sqlop @p @=
 */
Datum
Same_rid_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = same_rid_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
