/***********************************************************************
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
 * @brief Functions for spatial reference systems for spatial values, that is,
 * spatial sets, spatiotemporal boxes, temporal points, and temporal geos
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/stbox.h"
#include "geo/tpoint_restrfuncs.h"
#include "geo/tspatial_parser.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h" /* For oid_type() */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input in (E)WKT representation
 *****************************************************************************/

PGDLLEXPORT Datum Tpoint_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_from_ewkt);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return a temporal point from its Extended Well-Known Text (EWKT)
 * representation
 * @note This just does the same thing as the tpoint_in function, except it has 
 * to handle a 'text' input. First, unwrap the text into a cstring, then do as 
 * tpoint_in
 * @sqlfn tgeompointFromText(), tgeogpointFromText(), tgeompointFromEWKT(),
 * tgeogpointFromEWKT()
 */
Datum
Tpoint_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  char *wkt = text2cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Temporal *result = tpoint_parse(&wkt_ptr, oid_type(temptypid));
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tspatial_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_from_ewkt);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return a temporal geo from its Extended Well-Known Text (EWKT)
 * representation
 * @note This just does the same thing as the SQL function tgeo_in, except it
 * has to handle a 'text' input. First, unwrap the text into a cstring, then
 * do as tgeo_in
 * @sqlfn tgeometryFromEWKT(), tgeographyFromEWKT()
 */
Datum
Tspatial_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  char *wkt = text2cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = tspatial_parse(&wkt_ptr, oid_type(temptypid));
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Output in (E)WKT representation
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal spatial value
 * @sqlfn asText()
 */
static Datum
Tspatial_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? tspatial_as_ewkt(temp, dbl_dig_for_wkt) : 
    tspatial_as_text(temp, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Tspatial_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_as_text);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal spatial
 * value
 * @sqlfn asText()
 */
Datum
Tspatial_as_text(PG_FUNCTION_ARGS)
{
  return Tspatial_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Tspatial_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_as_ewkt);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal spatial value
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Tspatial_as_ewkt(PG_FUNCTION_ARGS)
{
  return Tspatial_as_text_ext(fcinfo, true);
}

/*****************************************************************************
 * Output in (E)WKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Tspatial_as_ewkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_as_ewkb);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Extended Well-Known Binary (WKB) representation of a
 * temporal spatial value
 * @note This will have 'SRID=#;' for temporal spatial values
 * @sqlfn asEWKB()
 */
Datum
Tspatial_as_ewkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = Datum_as_wkb(fcinfo, PointerGetDatum(temp), temp->temptype,
    true);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tspatial_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_to_stbox);
/**
 * @ingroup mobilitydb_geo_conversion
 * @brief Convert a temporal spatial value into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tspatial_to_stbox(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Temporal *temp = temporal_slice(tempdatum);
  STBox *result = palloc(sizeof(STBox));
  tspatial_set_stbox(temp, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Geo_expand_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_expand_space);
/**
 * @ingroup mobilitydb_temporal_transf
 * @brief Return the bounding box of a geometry/geography expanded on the
 * spatial dimension by a value
 * @sqlfn expandSpace()
 */
Datum
Geo_expand_space(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double d = PG_GETARG_FLOAT8(1);
  STBox *result = geo_expand_space(gs, d);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Tspatial_expand_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_expand_space);
/**
 * @ingroup mobilitydb_geo_transf
 * @brief Return the bounding box of a temporal spatial value expanded on the
 * spatial dimension by a value
 * @sqlfn expandSpace()
 */
Datum
Tspatial_expand_space(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double d = PG_GETARG_FLOAT8(1);
  STBox *result = tspatial_expand_space(temp, d);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Spatial reference system functions for temporal spatial types
 *****************************************************************************/

PGDLLEXPORT Datum Tspatial_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_srid);
/**
 * @ingroup mobilitydb_geo_srid
 * @brief Return the SRID of a temporal spatial value
 * @sqlfn SRID()
 */
Datum
Tspatial_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tspatial_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tspatial_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_set_srid);
/**
 * @ingroup mobilitydb_geo_srid
 * @brief Return a temporal spatial value with the coordinates set to an SRID
 * @sqlfn setSRID()
 */
Datum
Tspatial_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tspatial_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tspatial_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_transform);
/**
 * @ingroup mobilitydb_geo_srid
 * @brief Return a temporal spatial value transformed to an SRID
 * @sqlfn transform()
 */
Datum
Tspatial_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Temporal *result = tspatial_transform(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tspatial_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_transform_pipeline);
/**
 * @ingroup mobilitydb_geo_srid
 * @brief Return a temporal spatial value transformed to an SRID using a
 * pipeline
 * @sqlfn transformPipeline()
 */
Datum
Tspatial_transform_pipeline(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Temporal *result = tspatial_transform_pipeline(temp, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
