/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Basic functions for temporal network points.
 */

#include "npoint/tnpoint.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/lifting.h"
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "npoint/tnpoint.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return a C array of network segment values with M measure to a
 * PostgreSQL array
 */
ArrayType *
nsegmentarr_array(Nsegment **nsegmentarr, int count)
{
  return construct_array((Datum *)nsegmentarr, count, type_oid(T_NSEGMENT),
    sizeof(Nsegment), false, 'd');
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_in);
/**
 * @ingroup mobilitydb_npoint_inout
 * @brief Return a network point from its Well-Known Text (WKT) representation
 * @sqlfn tnpoint_in()
 */
Datum
Tnpoint_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_parse(&input, oid_type(temptypid));
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_npoint_conversion
 * @brief Convert a temporal network point into a temporal geometry point
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_to_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tnpoint);
/**
 * @ingroup mobilitydb_npoint_conversion
 * @brief Convert a temporal geometry point into a temporal network point
 * @sqlfn tnpoint()
 * @sqlop @p ::
 */
Datum
Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_to_tnpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_positions(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_positions);
/**
 * @ingroup mobilitydb_npoint_accessor
 * @brief Return the network segments covered by a temporal network point
 * @sqlfn positions()
 */
Datum
Tnpoint_positions(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Nsegment **segments = tnpoint_positions(temp, &count);
  ArrayType *result = nsegmentarr_array(segments, count);
  pfree_array((void **) segments, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tnpoint_route(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_route);
/**
 * @ingroup mobilitydb_npoint_accessor
 * @brief Return the route of a temporal network point
 * @sqlfn route()
 */
Datum
Tnpoint_route(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int64 result = tnpoint_route(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT64(result);
}

PGDLLEXPORT Datum Tnpoint_routes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_routes);
/**
 * @ingroup mobilitydb_npoint_accessor
 * @brief Return the array of routes of a temporal network point
 * @sqlfn routes()
 */
Datum
Tnpoint_routes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tnpoint_routes(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * @brief Return a temporal network point restricted to (the complement of) a
 * network point
 */
static Datum
Tnpoint_restrict_npoint(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Temporal *result = tnpoint_restrict_npoint(temp, np, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnpoint_at_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_npoint);
/**
 * @ingroup mobilitydb_npoint_restrict
 * @brief Return a temporal value restricted to a base value
 * @sqlfn atValues()
 */
Datum
Tnpoint_at_npoint(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_npoint(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnpoint_minus_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_npoint);
/**
 * @ingroup mobilitydb_npoint_restrict
 * @brief Return a temporal value restricted to the complement of a base value
 * @sqlfn minusValues()
 */
Datum
Tnpoint_minus_npoint(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_npoint(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal value restricted to (the complement of) a set of
 * network points
 */
Datum
Tnpoint_restrict_npointset(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *s = PG_GETARG_SET_P(1);
  Temporal *result = tnpoint_restrict_npointset(temp, s, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(s, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tnpoint_at_npointset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_at_npointset);
/**
 * @ingroup mobilitydb_npoint_restrict
 * @brief Return a temporal network point restricted to a set of network points
 * @sqlfn atValues()
 */
inline Datum
Tnpoint_at_npointset(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_npointset(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tnpoint_minus_npointset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_minus_npointset);
/**
 * @ingroup mobilitydb_npoint_restrict
 * @brief Return a temporal network point restricted to the complement of a set
 * of network points
 * @sqlfn minusValues()
 */
inline Datum
Tnpoint_minus_npointset(PG_FUNCTION_ARGS)
{
  return Tnpoint_restrict_npointset(fcinfo, REST_MINUS);
}

/*****************************************************************************/
