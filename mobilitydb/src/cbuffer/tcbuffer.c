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
 * @brief Basic functions for temporal network points
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "general/lifting.h"
#include "general/set.h"
#include "general/temporal.h"
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#define TCBUFFER_MAX_TYPMOD 2

/**
 * @brief Check the consistency of the metadata specified in the typmod: SRID,
 * type, and dimensionality. If things are inconsistent, shut down the query.
 */
static Temporal *
tcbuffer_valid_typmod(Temporal *temp, int32_t typmod)
{
  int32 srid = tspatial_srid(temp);
  uint8 subtype = temp->subtype;
  uint8 typmod_subtype = TYPMOD_GET_TEMPSUBTYPE(typmod);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);

  /* No typmod (-1) */
  if (typmod < 0 && typmod_subtype == ANYTEMPSUBTYPE)
    return temp;
  /* Typmod has a preference for SRID? Circular buffer SRID had better match */
  if (typmod_srid > 0 && typmod_srid != srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal circular buffer SRID (%d) does not match column SRID (%d)",
        srid, typmod_srid) ));
  /* Typmod has a preference for temporal subtype */
  if (typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal subtype (%s) does not match column type (%s)",
        tempsubtype_name(subtype), tempsubtype_name(typmod_subtype)) ));

  return temp;
}

PGDLLEXPORT Datum Tcbuffer_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_in);
/**
 * @ingroup mobilitydb_cbuffer_inout
 * @brief Return a temporal circular buffer from its Well-Known Text (WKT)
 * representation
 * @sqlfn tcbuffer_in()
 */
Datum
Tcbuffer_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Temporal *result = tspatial_parse(&input, T_TCBUFFER);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_enforce_typmod(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal circular buffers with 
 * respect to temporal subtype and SRID
 */
Datum
Tcbuffer_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if the typmod of the temporal circular buffer is consistent with the
   * supplied one */
  temp = tcbuffer_valid_typmod(temp, typmod);
  PG_RETURN_TEMPORAL_P(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_constructor);
/**
 * @ingroup mobilitydb_cbuffer_constructor
 * @brief Construct a temporal circular buffer from a temporal point and a
 * temporal float
 * @sqlfn tcbuffer_constructor()
 */
Datum
Tcbuffer_constructor(PG_FUNCTION_ARGS)
{
  Temporal *point = PG_GETARG_TEMPORAL_P(0);
  Temporal *radius = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tcbuffer_make(point, radius);
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
 * @ingroup mobilitydb_cbuffer_conversion
 * @brief Convert a temporal circular buffer into a temporal geometry point
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
 * @ingroup mobilitydb_cbuffer_conversion
 * @brief Convert a temporal circular buffer into a temporal float
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

PGDLLEXPORT Datum Tgeompoint_to_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_to_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_conversion
 * @brief Convert a temporal geometry point into a temporal circular buffer
 * with a zero radius
 * @sqlfn tgeompoint()
 * @sqlop @p ::
 */
Datum
Tgeompoint_to_tcbuffer(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_points);
/**
 * @ingroup mobilitydb_cbuffer_accessor
 * @brief Return the set of points of a temporal circular buffer
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

PGDLLEXPORT Datum Tcbuffer_radius(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_radius);
/**
 * @ingroup mobilitydb_cbuffer_accessor
 * @brief Return the set of radii of a temporal circular buffer
 * @sqlfn points()
 */
Datum
Tcbuffer_radius(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tcbuffer_radius(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Return the geometric positions covered by a temporal circular buffer
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_traversed_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_traversed_area);
/**
 * @ingroup mobilitydb_cbuffer_accessor
 * @brief Return the geometry covered by a temporal circular buffer
 * @sqlfn traversedArea()
 */
Datum
Tcbuffer_traversed_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = tcbuffer_traversed_area(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
