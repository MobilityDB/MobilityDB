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
#include "general/type_parser.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/stbox.h"
#include "geo/tpoint_restrfuncs.h"
#include "geo/tspatial.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h" /* For oid_type() */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input in (E)WKT representation
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_from_ewkt);
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
Spatialset_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  char *wkt = text2cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Set *result = set_parse(&wkt_ptr, oid_type(temptypid));
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************
 * Output in (E)WKT representation
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_as_text);
/**
 * @ingroup mobilitydb_geo_set_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatial set
 * @sqlfn asText()
 */
Datum
Spatialset_as_text(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = spatialset_as_text(s, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Spatialset_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_as_ewkt);
/**
 * @ingroup mobilitydb_geo_set_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * spatial set
 * @sqlfn asEWKT()
 */
Datum
Spatialset_as_ewkt(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = spatialset_as_ewkt(s, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * an array of spatial values (external function)
 */
Datum
Spatialarr_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);

  Datum *datumarr = datumarr_extract(array, &count);
  meosType basetype = oid_type(array->elemtype);
  char **strarr = spatialarr_wkt_out(datumarr, basetype, count, 
    dbl_dig_for_wkt, extended);
  /* We cannot use pfree_array */
  pfree(datumarr);
  ArrayType *result = strarr_to_textarray(strarr, count);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Spatialarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialarr_as_text);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * spatial values
 * @sqlfn asText()
 */
Datum
Spatialarr_as_text(PG_FUNCTION_ARGS)
{
  return Spatialarr_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Spatialarr_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialarr_as_ewkt);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation
 * of an array of spatial values
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Spatialarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return Spatialarr_as_text_ext(fcinfo, true);
}

/*****************************************************************************
 * Output in (E)WKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_as_ewkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_as_ewkb);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return the Extended Well-Known Binary (WKB) representation of a
 * spatial set
 * @note This will have 'SRID=#;' for spatial sets
 * @sqlfn asEWKB()
 */
Datum
Spatialset_as_ewkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Set *s = PG_GETARG_SET_P(0);
  bytea *result = Datum_as_wkb(fcinfo, PointerGetDatum(s), s->settype, true);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_to_stbox);
/**
 * @ingroup mobilitydb_geo_set_conversion
 * @brief Convert a spatial set into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Spatialset_to_stbox(PG_FUNCTION_ARGS)
{
  Set *set = PG_GETARG_SET_P(0);
  STBox *result = spatialset_stbox(set);
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Spatial reference system functions for spatial sets
 *****************************************************************************/

PGDLLEXPORT Datum Spatialset_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_srid);
/**
 * @ingroup mobilitydb_geo_set_srid
 * @brief Return the SRID of a spatial set
 * @sqlfn SRID()
 */
Datum
Spatialset_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int result = spatialset_srid(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Spatialset_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_set_srid);
/**
 * @ingroup mobilitydb_geo_set_srid
 * @brief Return a spatial set with the values set to an SRID
 * @sqlfn setSRID()
 */
Datum
Spatialset_set_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = spatialset_set_srid(s, srid);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Spatialset_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_transform);
/**
 * @ingroup mobilitydb_geo_set_srid
 * @brief Return a spatial set transformed to an SRID
 * @sqlfn transform()
 */
Datum
Spatialset_transform(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = spatialset_transform(s, srid);
  PG_FREE_IF_COPY(s, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Spatialset_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spatialset_transform_pipeline);
/**
 * @ingroup mobilitydb_geo_set_srid
 * @brief Return a spatial set transformed to an SRID using a transformation
 * pipeline
 * @sqlfn transformPipeline()
 */
Datum
Spatialset_transform_pipeline(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Set *result = spatialset_transform_pipeline(s, pipelinestr, srid, is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(s, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/
