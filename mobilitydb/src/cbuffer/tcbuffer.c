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

#include "cbuffer/tcbuffer.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_parser.h"
#include "general/type_round.h"
#include "general/type_util.h"
#include "geo/tgeo_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_in);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from its Well-Known Text (WKT) representation
 * @sqlfn tcbuffer_in()
 */
Datum
Tcbuffer_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Temporal *result = tcbuffer_parse(&input);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_constructor);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a circular buffer from a temporal point and a temporal float
 * @sqlfn tcbuffer_constructor()
 */
Datum
Tcbuffer_constructor(PG_FUNCTION_ARGS)
{
  Temporal *point = PG_GETARG_TEMPORAL_P(0);
  Temporal *radius = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tcbuffer_constructor(point, radius);
  PG_FREE_IF_COPY(point, 0);
  PG_FREE_IF_COPY(radius, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_to_tgeompoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_tgeompoint);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal circular buffer converted to a temporal geometry point
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_to_tfloat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_tfloat);
/**
 * @ingroup mobilitydb_temporal_conversion
 * @brief Return a temporal circular buffer converted to a temporal geometry point
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a temporal circular buffer with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Tcbuffer_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Temporal *result = tcbuffer_round(temp, size);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Cbufferset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferset_round);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return a buffer set with the precision of the positions
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Cbufferset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Set *result = cbufferset_round(s, size);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_points);
/**
 * @ingroup mobilitydb_temporal_accessor
 * @brief Return the array of points of a temporal circular buffer
 * @sqlfn points()
 */
Datum
Tcbuffer_points(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tcbuffer_points(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

// PGDLLEXPORT Datum Tcbuffer_route(Tcbuffer_line);
// PG_FUNCTION_INFO_V1(Tcbuffer_line);
// /**
 // * @ingroup mobilitydb_temporal_accessor
 // * @brief Return the central line of a temporal circular buffer
 // * @sqlfn point()
 // */
// Datum
// Tcbuffer_line(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // GSERIALIZED *result = tcbuffer_line(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_GSERIALIZED_P(result);
// }

/*****************************************************************************/
