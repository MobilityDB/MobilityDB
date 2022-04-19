/*****************************************************************************
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
 * @file tpoint_in.c
 * @brief Input of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON format.
 */

#include "point/tpoint_in.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
/* JSON-C */
#include <json-c/json.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "point/postgis.h"
#include "point/tpoint.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_mfjson);
/**
 * Return a temporal point from its MF-JSON representation
 */
PGDLLEXPORT Datum
Tpoint_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_input = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = tpoint_from_mfjson_ext(fcinfo, mfjson_input,
    oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Input in EWKB format
 * Please refer to the file tpoint_out.c where the binary format is explained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_ewkb);
/**
 * Return a temporal point from its EWKB representation
 */
PGDLLEXPORT Datum
Tpoint_from_ewkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *temp = tpoint_from_ewkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Input in HEXEWKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_hexewkb);
/**
 * Return a temporal point from its HEXEWKB representation
 */
PGDLLEXPORT Datum
Tpoint_from_hexewkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  int hexwkb_len = strlen(hexwkb);
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, hexwkb_len);
  Temporal *temp = tpoint_from_ewkb(wkb, hexwkb_len/2);
  pfree(hexwkb);
  pfree(wkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_ewkt);
/**
 * This just does the same thing as the _in function, except it has to handle
 * a 'text' input. First, unwrap the text into a cstring, then do as tpoint_in
*/
PGDLLEXPORT Datum
Tpoint_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  char *wkt = text2cstring(wkt_text);
  /* Save the address of wkt since it is modified by the parse function */
  char *wkt_save = wkt;
  Temporal *result = tpoint_parse(&wkt, oid_type(temptypid));
  pfree(wkt_save);
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
