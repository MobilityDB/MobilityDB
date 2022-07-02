/***********************************************************************
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
 * @brief Distance functions for temporal numbers.
 */

#include "general/tnumber_distance.h"

/* C */
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/lifting.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Distance_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a number and a temporal number
 * @sqlfunc tnumber_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_number_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Oid restypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = distance_tnumber_number(temp, value,
    oid_type(valuetypid), oid_type(restypid));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal number and a number
 * @sqlfunc tnumber_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_tnumber_number(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid restypid = get_fn_expr_rettype(fcinfo->flinfo);
  Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = distance_tnumber_number(temp, value,
    oid_type(valuetypid), oid_type(restypid));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between two temporal numbers
 * @sqlfunc tnumber_distance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Distance_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = distance_tnumber_tnumber1(temp1, temp2,
    oid_type(temptypid));
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a number and a temporal number
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_number_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnumber_number(temp, value,
    oid_type(basetypid));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal number and a number
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_tnumber_number(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  double result = nad_tnumber_number(temp, value,
    oid_type(basetypid));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tbox_tbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between the temporal boxes
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBOX *box1 = PG_GETARG_TBOX_P(0);
  TBOX *box2 = PG_GETARG_TBOX_P(1);
  double result = nad_tbox_tbox(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal box and a
 * temporal number
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_tbox_tnumber(PG_FUNCTION_ARGS)
{
  TBOX *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnumber_tbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal number and a
 * temporal box
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_tnumber_tbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBOX *box = PG_GETARG_TBOX_P(1);
  double result = nad_tnumber_tbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between the temporal numbers
 * @sqlfunc nearestApproachDistance()
 */
PGDLLEXPORT Datum
NAD_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  ensure_tnumber_type(temp1->temptype);
  ensure_tnumber_type(temp2->temptype);
  /* Result of the distance function is a tint iff both arguments are tint */
  mobdbType restype = (temp1->temptype == T_TINT && temp2->temptype == T_TINT) ?
    T_TINT : T_TFLOAT;
  Temporal *dist = distance_tnumber_tnumber1(temp1, temp2, restype);
  if (dist == NULL)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Datum result = temporal_min_value(dist);
  if (restype == T_TINT)
    result = Float8GetDatum((double) DatumGetInt32(result));
  pfree(dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
