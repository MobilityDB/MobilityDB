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
 * @brief Input of types in WKB, HexWKB, and MF-JSON representation
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
#include "general/set.h"
#include "general/span.h"
#include "general/tbox.h"
#include "general/temporal.h"
#include "point/stbox.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Input in WKB and HexWKB representation for sets, spans, and span sets types
 *****************************************************************************/

PGDLLEXPORT Datum Set_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_from_wkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a set from its Well-Known Binary (WKB) representation
 * @sqlfn intsetFromBinary(), floatsetFromWKB(), ...
 */
Datum
Set_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Set *result = set_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Set_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_from_hexwkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a set from its hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation
 * @sqlfn intsetFromHexWKB(), floatsetFromHexWKB(), ...
 */
Datum
Set_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Set *result = set_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Span_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_wkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its Well-Known Binary (WKB) representation
 * @sqlfn intspanFromBinary(), floatspanFromBinary(), ...
 */
Datum
Span_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Span *result = span_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Span_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_hexwkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation
 * @sqlfn intspanFromHexWKB(), floatspanFromHexWKB(), ...
 */
Datum
Span_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Span *result = span_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Spanset_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_from_wkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span set from its Well-Known Binary (WKB) representation
 * @sqlfn instspansetFromBinary(), floatspansetFromBinary(), ...
 */
Datum
Spanset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  SpanSet *result = spanset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Spanset_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_from_hexwkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span set from its hex-encoded ASCII Well-Known Binary
 * (HexWKB) representation
 * @sqlfn intspansetFromHexWKB(), floatspansetFromHexWKB(), ...
 */
Datum
Spanset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  SpanSet *result = spanset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************
 * Input in WKB and HexWKB representation for bounding box types
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_from_wkb);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a temporal box from its Well-Known Binary (WKB) representation
 * @sqlfn tboxFromBinary()
 */
Datum
Tbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TBox *result = tbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_TBOX_P(result);
}

PGDLLEXPORT Datum Tbox_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_from_hexwkb);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a temporal box from its hex-encoded ASCII Well-Known Binary
 * (HexWKB) representation
 * @sqlfn tboxFromHexWKB()
 */
Datum
Tbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TBox *result = tbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_TBOX_P(result);
}

PGDLLEXPORT Datum Stbox_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_wkb);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation
 * @sqlfn stboxFromBinary()
 */
Datum
Stbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  STBox *result = stbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_hexwkb);
/**
 * @ingroup mobilitydb_box_inout
 * @brief Return a spatiotemporal box from its hex-encoded ASCII Well-Known
 * Binary (HexWKB) representation
 * @sqlfn stboxFromHexWKB()
 */
Datum
Stbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  STBox *result = stbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Input functions in WKB, HexWKB, and MF-JSON for temporal types
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_wkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal value from its Well-Known Binary (WKB)
 * representation
 * @sqlfn tintFromBinary(), tfloatFromBinary(), ...
 */
Datum
Temporal_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *result = temporal_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_hexwkb);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal value from its hex-encoded ASCII Well-Known Binary
 * (HexWKB) representation
 * @sqlfn tintFromHexWKB(), tfloatFromHexWKB(), ...
 */
Datum
Temporal_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Temporal *result = temporal_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_from_mfjson(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_from_mfjson);
/**
 * @ingroup mobilitydb_temporal_inout
 * @brief Return a temporal value from its Moving-Features JSON (MF-JSON)
 * representation
 * @sqlfn tintFromMFJSON(), tfloatFromMFJSON(), ...
 */
Datum
Temporal_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_txt = PG_GETARG_TEXT_P(0);
  char *mfjson = text2cstring(mfjson_txt);
  meosType temptype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  Temporal *result = temporal_from_mfjson(mfjson, temptype);
  pfree(mfjson);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
