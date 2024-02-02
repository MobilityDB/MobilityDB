/***********************************************************************
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
 * @brief Distance functions for temporal numbers
 */

/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tbox.h"
#include "general/temporal.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a number and a temporal number
 * @sqlfn tnumber_distance()
 * @sqlop @p <->
 */
Datum
Distance_number_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnumber_number(temp, value);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal number and a number
 * @sqlfn tnumber_distance()
 * @sqlop @p <->
 */
Datum
Distance_tnumber_number(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Temporal *result = distance_tnumber_number(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between two temporal numbers
 * @sqlfn tnumber_distance()
 * @sqlop @p <->
 */
Datum
Distance_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnumber_tnumber(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

PGDLLEXPORT Datum NAD_number_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_number_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a number and a temporal
 * number
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_number_tnumber(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = nad_tnumber_number(temp, value);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum NAD_tnumber_number(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnumber_number);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal number and a
 * number
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_tnumber_number(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Datum result = nad_tnumber_number(temp, value);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum NAD_tbox_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tbox_tbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between two temporal boxes
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_tbox_tbox(PG_FUNCTION_ARGS)
{
  TBox *box1 = PG_GETARG_TBOX_P(0);
  TBox *box2 = PG_GETARG_TBOX_P(1);
  Datum result = nad_tbox_tbox(box1, box2);
  double dresult = datum_double(result, box1->span.basetype);
  if (dresult < 0.0)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum NAD_tbox_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tbox_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal box and a
 * temporal number
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_tbox_tnumber(PG_FUNCTION_ARGS)
{
  TBox *box = PG_GETARG_TBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = nad_tnumber_tbox(temp, box);
  double dresult = datum_double(result, box->span.basetype);
  PG_FREE_IF_COPY(temp, 1);
  if (dresult < 0.0)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum NAD_tnumber_tbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnumber_tbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between a temporal number and a
 * temporal box
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_tnumber_tbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TBox *box = PG_GETARG_TBOX_P(1);
  Datum result = nad_tnumber_tbox(temp, box);
  double dresult = datum_double(result, box->span.basetype);
  PG_FREE_IF_COPY(temp, 0);
  if (dresult < 0.0)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum NAD_tnumber_tnumber(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tnumber_tnumber);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between two temporal numbers
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Datum result = nad_tnumber_tnumber(temp1, temp2);
  meosType basetype = temptype_basetype(temp1->temptype);
  double dresult = datum_double(result, basetype);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (dresult < 0.0)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
