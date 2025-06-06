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
 * @brief Temporal Boolean operators: and, or, not
 */

#include "temporal/tbool_ops.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include "temporal/span.h"

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

PGDLLEXPORT Datum Tand_bool_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tand_bool_tbool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean and of a boolean and a temporal boolean
 * @sqlfn temporal_and()
 * @sqlop @p &
 */
Datum
Tand_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tand_tbool_bool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tand_tbool_bool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean and of a temporal boolean and a boolean
 * @sqlfn temporal_and()
 * @sqlop @p &
 */
Datum
Tand_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_and, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tand_tbool_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tand_tbool_tbool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean and of two temporal booleans
 * @sqlfn temporal_and()
 * @sqlop @p &
 */
Datum
Tand_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_and);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

PGDLLEXPORT Datum Tor_bool_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tor_bool_tbool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean or of a boolean and the temporal boolean
 * @sqlfn temporal_or()
 * @sqlop @p |
 */
Datum
Tor_bool_tbool(PG_FUNCTION_ARGS)
{
  Datum b = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tor_tbool_bool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tor_tbool_bool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean or of a temporal boolean and a boolean
 * @sqlfn temporal_or()
 * @sqlop @p |
 */
Datum
Tor_tbool_bool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum b = PG_GETARG_DATUM(1);
  Temporal *result = boolop_tbool_bool(temp, b, &datum_or, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tor_tbool_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tor_tbool_tbool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean or of two temporal booleans
 * @sqlfn temporal_or()
 * @sqlop @p |
 */
Datum
Tor_tbool_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = boolop_tbool_tbool(temp1, temp2, &datum_or);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

PGDLLEXPORT Datum Tnot_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnot_tbool);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the boolean not of a temporal boolean
 * @sqlfn temporal_not()
 * @sqlop @p ~
 */
Datum
Tnot_tbool(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tnot_tbool(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal when
 *****************************************************************************/

PGDLLEXPORT Datum Tbool_when_true(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbool_when_true);
/**
 * @ingroup mobilitydb_temporal_bool
 * @brief Return the timestamptz span set in which a temporal boolean takes
 * value true
 * @sqlfn whenTrue()
 */
Datum
Tbool_when_true(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  SpanSet *result = tbool_when_true(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************/
