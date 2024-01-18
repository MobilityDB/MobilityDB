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

#include "npoint/tnpoint_routeops.h"

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

/*****************************************************************************
 * Generic route functions
 *****************************************************************************/

/**
 * @brief Return true if a temporal network point and a route satisfy the
 * function
 */
bool
contains_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(rid));
  pfree(routes);
  return result;
}

/**
 * @brief Return true if a temporal network point and a route satisfy the
 * function
 */
bool
contained_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  return contains_rid_tnpoint_bigint(temp, rid, INVERT);
}

/**
 * @brief Return true if a temporal network point and a route satisfy the
 * function
 */
bool
same_rid_tnpoint_bigint(const Temporal *temp, int64 rid,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = (routes->count == 1) &&
    (DatumGetInt64(SET_VAL_N(routes, 0)) == rid);
  pfree(routes);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if a temporal network point and a big integer set
 * satisfy the function
 */
bool
overlaps_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = overlaps_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @brief Return true if a temporal network point and a big integer set
 * satisfy the function
 */
bool
contains_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert)
{
  Set *routes = tnpoint_routes(temp);
  bool result = invert ? contains_set_set(s, routes) :
    contains_set_set(routes, s);
  pfree(routes);
  return result;
}

/**
 * @brief Return true if a temporal network point and a big integer set
 * satisfy the function
 */
bool
contained_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert)
{
  return contains_rid_tnpoint_bigintset(temp, s, ! invert);
}

/**
 * @brief Return true if a temporal network point and a big integer set
 * satisfy the function
 */
bool
same_rid_tnpoint_bigintset(const Temporal *temp, const Set *s,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = set_eq(routes, s);
  pfree(routes);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if a temporal network point and a network point
 * satisfy the function
 */
bool
contains_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = contains_set_value(routes, Int64GetDatum(np->rid));
  pfree(routes);
  return result;
}

/**
 * @brief Return true if a temporal network point and a network point
 * satisfy the function
 */
bool
contained_rid_npoint_tnpoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  return contains_rid_tnpoint_npoint(temp, np, invert);
}

/**
 * @brief Return true if a temporal network point and a network point
 * satisfy the function
 */
bool
same_rid_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool invert __attribute__((unused)))
{
  Set *routes = tnpoint_routes(temp);
  bool result = (routes->count == 1) &&
    (DatumGetInt64(SET_VAL_N(routes, 0)) == np->rid);
  pfree(routes);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return true if two temporal network points satisfy the function
 */
bool
overlaps_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = overlaps_set_set(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/**
 * @brief Return true if two temporal network points satisfy the function
 */
bool
contains_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = contains_set_set(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/**
 * @brief Return true if two temporal network points satisfy the function
 */
bool
contained_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = contains_set_set(routes2, routes1);
  pfree(routes1); pfree(routes2);
  return result;
}

/**
 * @brief Return true if two temporal network points satisfy the function
 */
bool
same_rid_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Set *routes1 = tnpoint_routes(temp1);
  Set *routes2 = tnpoint_routes(temp2);
  bool result = set_eq(routes1, routes2);
  pfree(routes1); pfree(routes2);
  return result;
}

/*****************************************************************************/
