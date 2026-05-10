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
#include "temporal/lifting.h"
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "geo/tspatial_parser.h"
#include "cbuffer/cbuffer.h"
#include "cbuffer/tcbuffer_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

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

PGDLLEXPORT Datum Tcbuffer_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_typmod_in);
/**
 * @brief Input typmod information for temporal circular buffers
 */
Datum
Tcbuffer_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, true, false);
  PG_RETURN_INT32(typmod);
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
  Temporal *result = tcbuffer_to_tgeompoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_to_tfloat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_to_tfloat);
/**
 * @ingroup mobilitydb_cbuffer_conversion
 * @brief Convert a temporal circular buffer into a temporal float
 * @sqlfn tfloat()
 * @sqlop @p ::
 */
Datum
Tcbuffer_to_tfloat(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tcbuffer_to_tfloat(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeometry_to_tcbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeometry_to_tcbuffer);
/**
 * @ingroup mobilitydb_cbuffer_conversion
 * @brief Convert a temporal geometry into a temporal circular buffer
 * @details The function applies the PostGIS function ST_MinimumBoundingRadius
 * to each composing geometry
 * @sqlfn tcbuffer()
 * @sqlop @p ::
 */
Datum
Tgeometry_to_tcbuffer(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeometry_to_tcbuffer(temp);
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
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_expand(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_expand);
/**
 * @ingroup mobilitydb_cbuffer_transf
 * @brief Return a temporal circular buffer with the radius expanded by a
 * distance
 * @sqlfn expand()
 */
Datum
Tcbuffer_expand(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  Temporal *result = tcbuffer_expand(temp, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Traversed area
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
  bool unary_union = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    unary_union = PG_GETARG_BOOL(1);
  GSERIALIZED *result = tcbuffer_traversed_area(temp, unary_union);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Return a temporal circular buffer restricted to (the complement of)
 * a circular buffer
 * @note Only 2D is supported
 */
static Datum
Tcbuffer_restrict_cbuffer(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Cbuffer *cb = PG_GETARG_CBUFFER_P(1);
  Temporal *result = tcbuffer_restrict_cbuffer(temp, cb, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_at_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_at_cbuffer);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @note Only 2D is supported
 * @sqlfn atValue()
 */
inline Datum
Tcbuffer_at_cbuffer(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_cbuffer(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tcbuffer_minus_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_minus_cbuffer);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * circular buffer
 * @sqlfn minusValue()
 */
inline Datum
Tcbuffer_minus_cbuffer(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_cbuffer(fcinfo, REST_MINUS);
}

/*****************************************************************************/


/**
 * @brief Return a temporal circular buffer restricted to (the complement of)
 * a circular buffer
 * @note Only 2D is supported
 */
static Datum
Tcbuffer_restrict_stbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tcbuffer_restrict_stbox(temp, box, false, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_at_stbox);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to a circular buffer
 * @note Only 2D is supported
 * @sqlfn atValue()
 */
inline Datum
Tcbuffer_at_stbox(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_stbox(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tcbuffer_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_minus_stbox);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * circular buffer
 * @sqlfn minusValue()
 */
inline Datum
Tcbuffer_minus_stbox(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_stbox(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal circular buffer restricted to (the complement of)
 * a geometry
 * @note Only 2D is supported
 */
static Datum
Tcbuffer_restrict_geom(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tcbuffer_restrict_geom(temp, geo, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tcbuffer_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_at_geom);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to a geometry
 * @note Only 2D is supported
 * @sqlfn atGeometry()
 */
inline Datum
Tcbuffer_at_geom(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_geom(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Tcbuffer_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_minus_geom);
/**
 * @ingroup mobilitydb_geo_restrict
 * @brief Return a temporal circular buffer restricted to the complement of a
 * geometry
 * @note Only 2D is supported
 * @sqlfn minusGeometry()
 */
inline Datum
Tcbuffer_minus_geom(PG_FUNCTION_ARGS)
{
  return Tcbuffer_restrict_geom(fcinfo, REST_MINUS);
}

/*****************************************************************************/
