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
 * @brief Input and output of temporal circular buffers in WKT and EWKT
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/array.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "point/tpoint_parser.h"
#include "cbuffer/tcbuffer.h"
#include "cbuffer/tcbuffer_parser.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h" /* For oid_type */
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

PGDLLEXPORT Datum Tcbuffer_from_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_from_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal circular buffer from its Extended Well-Known Text
 * (EWKT) representation
 * @note This just does the same thing as the _in function, except it has to handle
 * a 'text' input. First, unwrap the text into a cstring, then do as tpoint_in
 * @sqlfn tcbufferFromText(), tgeompointFromEWKT(), tcbufferFromEWKT()
 */
Datum
Tcbuffer_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  char *wkt = text2cstring(wkt_text);
  /* Copy the pointer since it will be advanced during parsing */
  const char *wkt_ptr = wkt;
  Temporal *result = tcbuffer_parse(&wkt_ptr);
  pfree(wkt);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Output in WKT and EWKT representation
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a circular buffer
 * @sqlfn asText()
 */
static Datum
Cbuffer_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  Cbuffer *cbuf = PG_GETARG_CBUFFER_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? cbuffer_as_ewkt(cbuf, dbl_dig_for_wkt) : 
    cbuffer_as_text(cbuf, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(cbuf, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Cbuffer_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @sqlfn asText()
 */
Datum
Cbuffer_as_text(PG_FUNCTION_ARGS)
{
  return Cbuffer_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Cbuffer_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * circular buffer
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Cbuffer_as_ewkt(PG_FUNCTION_ARGS)
{
  return Cbuffer_as_text_ext(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a temporal circular buffer
 * @sqlfn asText()
 */
static Datum
Tcbuffer_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? tcbuffer_as_ewkt(temp, dbl_dig_for_wkt) :
    tcbuffer_as_text(temp, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Tcbuffer_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a temporal circular buffer
 * @sqlfn asText()
 */
Datum
Tcbuffer_as_text(PG_FUNCTION_ARGS)
{
  return Tcbuffer_as_text_ext(fcinfo, false);
}

PGDLLEXPORT Datum Tcbuffer_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbuffer_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * temporal circular buffer
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Tcbuffer_as_ewkt(PG_FUNCTION_ARGS)
{
  return Tcbuffer_as_text_ext(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * geometry/geography
 */
static Datum
Cbufferarr_as_text_ext(FunctionCallInfo fcinfo, bool temporal, bool extended)
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

  char **strarr;
  if (temporal)
  {
    Temporal **temparr = temparr_extract(array, &count);
    strarr = tcbufferarr_as_text((const Temporal **) temparr, count,
      dbl_dig_for_wkt, extended);
    /* We cannot use pfree_array */
    pfree(temparr);
  }
  else
  {
    Datum *cbufarr = datumarr_extract(array, &count);
    strarr = cbufferarr_as_text(cbufarr, count, dbl_dig_for_wkt, extended);
    /* We cannot use pfree_array */
    pfree(cbufarr);
  }
  ArrayType *result = strarr_to_textarray(strarr, count);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Cbufferarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferarr_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * geometry/geography
 * @sqlfn asText()
 */
Datum
Cbufferarr_as_text(PG_FUNCTION_ARGS)
{
  return Cbufferarr_as_text_ext(fcinfo, false, false);
}

PGDLLEXPORT Datum Cbufferarr_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferarr_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation
 * of a geometry/geography array
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Cbufferarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return Cbufferarr_as_text_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tcbufferarr_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbufferarr_as_text);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of a
 * geometry/geography array
 * @sqlfn asText()
 */
Datum
Tcbufferarr_as_text(PG_FUNCTION_ARGS)
{
  return Cbufferarr_as_text_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tcbufferarr_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcbufferarr_as_ewkt);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation an array of
 * temporal circular buffers
 * @note The output is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Tcbufferarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return Cbufferarr_as_text_ext(fcinfo, true, true);
}

/*****************************************************************************/
