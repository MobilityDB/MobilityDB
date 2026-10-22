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
#include <meos_npoint.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "npoint/tnpoint.h"

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes in a set and the routes of a temporal
 * network point overlap
 * @param[in] s Big integer set
 * @param[in] temp Temporal network point
 * @csqlfn #Overlaps_rid_bigintset_tnpoint()
 */
bool
overlaps_rid_bigintset_tnpoint(const Set *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = overlaps_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point and the routes
 * in a set overlap
 * @param[in] temp Temporal network point
 * @param[in] s Big integer set
 * @csqlfn #Overlaps_rid_tnpoint_bigintset()
 */
bool
overlaps_rid_tnpoint_bigintset(const Temporal *temp, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = overlaps_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of two temporal network points overlap
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Overlaps_rid_tnpoint_tnpoint()
 */
bool
overlaps_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, false); VALIDATE_TNPOINT(temp2, false);
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = overlaps_set_set(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/*****************************************************************************
 * Contains
 *****************************************************************************/

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes in a set contain the routes of a temporal
 * network point
 * @param[in] s Big integer set
 * @param[in] temp Temporal network point
 * @csqlfn #Contains_rid_bigintset_tnpoint()
 */
bool
contains_rid_bigintset_tnpoint(const Set *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_set(s, routes);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point contain a
 * route
 * @param[in] temp Temporal network point
 * @param[in] rid Route identifier
 * @csqlfn #Contains_rid_tnpoint_bigint()
 */
bool
contains_rid_tnpoint_bigint(const Temporal *temp, int64 rid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(rid));
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point contain the
 * routes in a set
 * @param[in] temp Temporal network point
 * @param[in] s Big integer set
 * @csqlfn #Contains_rid_tnpoint_bigintset()
 */
bool
contains_rid_tnpoint_bigintset(const Temporal *temp, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point contain the
 * route of a network point
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Contains_rid_tnpoint_npoint()
 */
bool
contains_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(np, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(np->rid));
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of the first temporal network point
 * contain the routes of the second one
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Contains_rid_tnpoint_tnpoint()
 */
bool
contains_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, false); VALIDATE_TNPOINT(temp2, false);
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = contains_set_set(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/*****************************************************************************
 * Contained
 *****************************************************************************/

/**
 * @ingroup meos_npoint_route
 * @brief Return true if a route is contained in the routes of a temporal
 * network point
 * @param[in] rid Route identifier
 * @param[in] temp Temporal network point
 * @csqlfn #Contained_rid_bigint_tnpoint()
 */
bool
contained_rid_bigint_tnpoint(int64 rid, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(rid));
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes in a set are contained in the routes of a
 * temporal network point
 * @param[in] s Big integer set
 * @param[in] temp Temporal network point
 * @csqlfn #Contained_rid_bigintset_tnpoint()
 */
bool
contained_rid_bigintset_tnpoint(const Set *s, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the route of a network point is contained in the
 * routes of a temporal network point
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Contained_rid_npoint_tnpoint()
 */
bool
contained_rid_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(np, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(np->rid));
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point are contained
 * in the routes in a set
 * @param[in] temp Temporal network point
 * @param[in] s Big integer set
 * @csqlfn #Contained_rid_tnpoint_bigintset()
 */
bool
contained_rid_tnpoint_bigintset(const Temporal *temp, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_set(s, routes);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of the first temporal network point are
 * contained in the routes of the second one
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Contained_rid_tnpoint_tnpoint()
 */
bool
contained_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, false); VALIDATE_TNPOINT(temp2, false);
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = contains_set_set(routes2, routes1);
  pfree(routes1); pfree(routes2);
  return result;
}

/*****************************************************************************
 * Same
 *****************************************************************************/

/**
 * @ingroup meos_npoint_route
 * @brief Return true if a route and the routes of a temporal network point
 * are equal
 * @param[in] rid Route identifier
 * @param[in] temp Temporal network point
 * @csqlfn #Same_rid_bigint_tnpoint()
 */
bool
same_rid_bigint_tnpoint(int64 rid, const Temporal *temp)
{
  return same_rid_tnpoint_bigint(temp, rid);
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes in a set and the routes of a temporal
 * network point are equal
 * @param[in] s Big integer set
 * @param[in] temp Temporal network point
 * @csqlfn #Same_rid_bigintset_tnpoint()
 */
bool
same_rid_bigintset_tnpoint(const Set *s, const Temporal *temp)
{
  return same_rid_tnpoint_bigintset(temp, s);
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the route of a network point and the routes of a
 * temporal network point are equal
 * @param[in] np Network point
 * @param[in] temp Temporal network point
 * @csqlfn #Same_rid_npoint_tnpoint()
 */
bool
same_rid_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return same_rid_tnpoint_npoint(temp, np);
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point and a route
 * are equal
 * @param[in] temp Temporal network point
 * @param[in] rid Route identifier
 * @csqlfn #Same_rid_tnpoint_bigint()
 */
bool
same_rid_tnpoint_bigint(const Temporal *temp, int64 rid)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false);
  Set *routes = tnpoint_routes(temp);
  bool result = (routes->count == 1) &&
    (DatumGetInt64(SET_VAL_N(routes, 0)) == rid);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point and the routes
 * in a set are equal
 * @param[in] temp Temporal network point
 * @param[in] s Big integer set
 * @csqlfn #Same_rid_tnpoint_bigintset()
 */
bool
same_rid_tnpoint_bigintset(const Temporal *temp, const Set *s)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_BIGINTSET(s, false);
  Set *routes = tnpoint_routes(temp);
  bool result = set_eq(routes, s);
  pfree(routes);
  return result;
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of a temporal network point and the route
 * of a network point are equal
 * @param[in] temp Temporal network point
 * @param[in] np Network point
 * @csqlfn #Same_rid_tnpoint_npoint()
 */
bool
same_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp, false); VALIDATE_NOT_NULL(np, false);
  return same_rid_tnpoint_bigint(temp, np->rid);
}

/**
 * @ingroup meos_npoint_route
 * @brief Return true if the routes of two temporal network points are equal
 * @param[in] temp1,temp2 Temporal network points
 * @csqlfn #Same_rid_tnpoint_tnpoint()
 */
bool
same_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNPOINT(temp1, false); VALIDATE_TNPOINT(temp2, false);
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = set_eq(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/*****************************************************************************/
