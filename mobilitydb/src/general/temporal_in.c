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
 * @file temporal_in.c
 * @brief Input of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB format.
 */

/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"

/*****************************************************************************
 * Input in WKT and in MF-JSON format
 *****************************************************************************/

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_mfjson);
/**
 * Return a temporal point from its MF-JSON representation
 */
PGDLLEXPORT Datum
Temporal_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_txt = PG_GETARG_TEXT_P(0);
  char *mfjson = text2cstring(mfjson_txt);
  Temporal *result = temporal_from_mfjson(mfjson);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Input in WKB and in HEXWKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_from_wkb);
/**
 * Return a span from its WKB representation
 */
PGDLLEXPORT Datum
Span_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Span *span = span_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(span);
}

PG_FUNCTION_INFO_V1(Span_from_hexwkb);
/**
 * Return a span from its HEXWKB representation
 */
PGDLLEXPORT Datum
Span_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Span *span = span_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(span);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_from_wkb);
/**
 * Return a timestamp set from its WKB representation
 */
PGDLLEXPORT Datum
Timestampset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TimestampSet *ts = timestampset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(ts);
}

PG_FUNCTION_INFO_V1(Timestampset_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Timestampset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TimestampSet *ts = timestampset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(ts);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_from_wkb);
/**
 * Return a timestamp set from its WKB representation
 */
PGDLLEXPORT Datum
Periodset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  PeriodSet *ps = periodset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(ps);
}

PG_FUNCTION_INFO_V1(Periodset_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Periodset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  PeriodSet *ps = periodset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(ps);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_from_wkb);
/**
 * Return a temporal box from its WKB representation
 */
PGDLLEXPORT Datum
Tbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TBOX *box = tbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PG_FUNCTION_INFO_V1(Tbox_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Tbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TBOX *box = tbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_from_wkb);
/**
 * Return a temporal box from its WKB representation
 */
PGDLLEXPORT Datum
Stbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  STBOX *box = stbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PG_FUNCTION_INFO_V1(Stbox_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Stbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  STBOX *box = stbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_wkb);
/**
 * Return a temporal type from its WKB representation
 */
PGDLLEXPORT Datum
Temporal_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *temp = temporal_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(temp);
}

PG_FUNCTION_INFO_V1(Temporal_from_hexwkb);
/**
 * Return a temporal type from its HEXWKB representation
 */
PGDLLEXPORT Datum
Temporal_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Temporal *temp = temporal_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************/
