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
 * @brief Basic functions for temporal network points.
 */

#include "npoint/tnpoint.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_parser.h"
#include "general/type_round.h"
#include "general/type_util.h"
#include "npoint/tnpoint_static.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

#if 0 /* not used */
/**
 * @brief Return a C array of network point values converted to a PostgreSQL
 * array
 */
ArrayType *
npointarr_array(Npoint **npointarr, int count)
{
  return construct_array((Datum *)npointarr, count, type_oid(T_NPOINT),
    sizeof(Npoint), false, 'd');
}
#endif

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
 * @ingroup mobilitydb_temporal_inout
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
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal network point converted to a temporal geometry point
 * @sqlfn tgeompoint()
 */
Datum
Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tnpoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal geometry point converted to a temporal network point
 * @sqlfn tnpoint()
 */
Datum
Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tnpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal network point with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Tnpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tnpoint_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Npointset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npointset_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a network point set with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Npointset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_SET_P(npointset_round(s, size));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_positions(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_positions);
/**
 * @ingroup mobilitydb_temporal_accessor
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
 * @ingroup mobilitydb_temporal_accessor
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
 * @ingroup mobilitydb_temporal_accessor
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

/*****************************************************************************/
