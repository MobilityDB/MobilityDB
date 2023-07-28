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
 * @brief Basic functions for temporal network points.
 */

#include "npoint/tnpoint.h"

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/lifting.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_parser.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_npoint/tnpoint_static.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

#if 0 /* not used */
/**
 * @brief Convert a C array of int64 values into a PostgreSQL array
 */
ArrayType *
int64arr_array(const int64 *int64arr, int count)
{
  return construct_array((Datum *)int64arr, count, INT8OID, 8, true, 'd');
}

/**
 * @brief Convert a C array of network point values into a PostgreSQL array
 */
ArrayType *
npointarr_array(Npoint **npointarr, int count)
{
  return construct_array((Datum *)npointarr, count, type_oid(T_NPOINT),
    sizeof(Npoint), false, 'd');
}
#endif

/**
 * @brief Convert a C array of network segment values into a PostgreSQL array
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
 * @brief Input function for temporal network points
 * @sqlfunc tnpoint_in()
 */
Datum
Tnpoint_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = temporal_parse(&input, oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Cast a temporal network point as a temporal geometric point
 * @sqlfunc tgeompoint()
 */
Datum
Tnpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnpoint_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tnpoint);
/**
 * @ingroup mobilitydb_temporal_cast
 * @brief Cast a temporal geometric point as a temporal network point
 * @sqlfunc tnpoint()
 */
Datum
Tgeompoint_to_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tnpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Set the precision of the fraction of the temporal network point to the
 * number of decimal places.
 */
Temporal *
tnpoint_round(const Temporal *temp, Datum size)
{
  /* We only need to fill these parameters for tfunc_temporal */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_npoint_round;
  lfinfo.numparam = 1;
  lfinfo.param[0] = size;
  lfinfo.restype = temp->temptype;
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  return result;
}

PGDLLEXPORT Datum Tnpoint_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Set the precision of the fraction of the temporal network point to the
 * number of decimal places.
 * @sqlfunc round()
 */
Datum
Tnpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tnpoint_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Npointset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Npointset_round);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Sets the precision of the coordinates of the geometry set
 * @sqlfunc round()
 */
Datum
Npointset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(npointset_round(s, size));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnpoint_positions(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_positions);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the network segments covered by the temporal network point
 * @sqlfunc positions()
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
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tnpoint_route(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnpoint_route);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the route of a temporal network point
 * @sqlfunc route()
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
 * @sqlfunc routes()
 */
Datum
Tnpoint_routes(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tnpoint_routes(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
