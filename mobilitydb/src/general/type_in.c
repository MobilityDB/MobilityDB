/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Input of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB format.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/type_util.h"

/*****************************************************************************
 * Input in WKT and in MF-JSON format
 *****************************************************************************/

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_from_mfjson(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_mfjson);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal value from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON(), tintFromMFJSON(), tfloatFromMFJSON(),
 * ttextFromMFJSON(), tgeompointFromMFJSON(), tgeogpointFromMFJSON()
 */
Datum
Temporal_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_txt = PG_GETARG_TEXT_P(0);
  char *mfjson = text2cstring(mfjson_txt);
  Temporal *result = temporal_from_mfjson(mfjson);
  pfree(mfjson);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Input in WKB and in HEXWKB format
 *****************************************************************************/

PGDLLEXPORT Datum Span_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a span from its WKB representation
 * @sqlfunc intspanFromBinary(), floatspanFromBinary(), periodFromBinary
 */
Datum
Span_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Span *span = span_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(span);
}

PGDLLEXPORT Datum Span_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a span from its HEXWKB representation
 * @sqlfunc intspanFromHexWKB(), floatspanFromHexWKB(), periodFromHexWKB
 */
Datum
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

PGDLLEXPORT Datum Set_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a timestamp set from its WKB representation
 * @sqlfunc tstzsetFromBinary()
 */
Datum
Set_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Set *s = set_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(s);
}

PGDLLEXPORT Datum Set_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a timestamp set from its HexWKB representation
 * @sqlfunc tstzsetFromHexWKB()
 */
Datum
Set_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Set *s = set_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(s);
}

/*****************************************************************************/

PGDLLEXPORT Datum Spanset_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a span set from its WKB representation
 * @sqlfunc instspansetFromBinary(), floatspansetFromBinary(),
 * periodsetFromBinary()
 */
Datum
Spanset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  SpanSet *ss = spanset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(ss);
}

PGDLLEXPORT Datum Spanset_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a period set from its HexWKB representation
 * @sqlfunc periodsetFromHexWKB()
 */
Datum
Spanset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  SpanSet *ss = spanset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(ss);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal box from its WKB representation
 * @sqlfunc tboxFromBinary()
 */
Datum
Tbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TBox *box = tbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PGDLLEXPORT Datum Tbox_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal box from its HexWKB representation
 * @sqlfunc tboxFromHexWKB()
 */
Datum
Tbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TBox *box = tbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal box from its WKB representation
 * @sqlfunc stboxFromBinary()
 */
Datum
Stbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  STBox *box = stbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PGDLLEXPORT Datum Stbox_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a spatiotemporal box from its HexWKB representation
 * @sqlfunc stboxFromHexWKB()
 */
Datum
Stbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  STBox *box = stbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal type from its WKB representation
 * @sqlfunc tboolFromBinary(), tintFromBinary(), tfloatFromBinary(),
 * ttextFromBinary(), tgeompointFromBinary(), tgeogpointFromBinary()
 */
Datum
Temporal_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *temp = temporal_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(temp);
}

PGDLLEXPORT Datum Temporal_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Input a temporal value from its HexWKB representation
 * @sqlfunc tboolFromHexWKB(), tintFromHexWKB(), tfloatFromHexWKB(),
 * ttextFromHexWKB(), tgeompointFromHexWKB(), tgeogpointFromHexWKB()
 */
Datum
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
