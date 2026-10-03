/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief PG V1 wrappers for the static `s2cell` SQL type
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
/* MEOS */
#include <meos.h>
#include <meos_s2cell.h>
#include "s2cell/s2cell.h"
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Input and output
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_in);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return an S2 cell from its string representation
 * @sqlfn s2cell_in()
 */
Datum
S2cell_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_S2CELL(s2cell_in(str));
}

PGDLLEXPORT Datum Bigint_to_s2cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Bigint_to_s2cell);
/**
 * @ingroup mobilitydb_s2cell_conversion
 * @brief Return an S2 cell from a big integer encoding one
 * @sqlfn s2cell()
 * @sqlop @p ::
 */
Datum
Bigint_to_s2cell(PG_FUNCTION_ARGS)
{
  int64 i = PG_GETARG_INT64(0);
  PG_RETURN_S2CELL(bigint_to_s2cell(i));
}

PGDLLEXPORT Datum S2cell_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_out);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return an S2 cell as its canonical hexadecimal string
 * @sqlfn s2cell_out()
 */
Datum
S2cell_out(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_CSTRING(s2cell_out(cell));
}

PGDLLEXPORT Datum S2cell_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_as_text);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return the text representation of an S2 cell
 * @sqlfn asText()
 */
Datum
S2cell_as_text(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  char *str = s2cell_out(cell);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum S2cell_from_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_from_text);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return an S2 cell from its text representation
 * @sqlfn s2cellFromText()
 */
Datum
S2cell_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = text_to_cstring(txt);
  S2CellId result = s2cell_in(str);
  pfree(str);
  PG_FREE_IF_COPY(txt, 0);
  PG_RETURN_S2CELL(result);
}

PGDLLEXPORT Datum S2cell_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_recv);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return an S2 cell received over the binary wire protocol
 */
Datum
S2cell_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_S2CELL(bigint_to_s2cell(pq_getmsgint64(buf)));
}

PGDLLEXPORT Datum S2cell_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_send);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Send an S2 cell over the binary wire protocol
 */
Datum
S2cell_send(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, (int64) cell);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * WKB and HexWKB input/output
 *
 * A S2 cell is a cell of a grid over WGS84 (EPSG:4326), which the grid fixes:
 * asBinary/asHexWKB are the SRID-less base WKB, as for an h3index. The output
 * side reuses the generic Datum_as_wkb / Datum_as_hexwkb dispatch; the input
 * side calls the MEOS functions (which accept an absent SRID as 4326, or a
 * present 4326).
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_from_wkb);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return a S2 cell from its Well-Known Binary (WKB) representation
 * @sqlfn s2cellFromBinary()
 */
Datum
S2cell_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  S2CellId result = s2cell_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_S2CELL(result);
}

PGDLLEXPORT Datum S2cell_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_from_hexwkb);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return a S2 cell from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn s2cellFromHexWKB()
 */
Datum
S2cell_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  S2CellId result = s2cell_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_S2CELL(result);
}

PGDLLEXPORT Datum S2cell_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_as_wkb);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a S2 cell
 * @sqlfn asBinary()
 */
Datum
S2cell_as_wkb(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, S2CellGetDatum(cell), T_S2CELL,
    false));
}

PGDLLEXPORT Datum S2cell_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_as_hexwkb);
/**
 * @ingroup mobilitydb_s2cell_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a S2 cell
 * @sqlfn asHexWKB()
 */
Datum
S2cell_as_hexwkb(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, S2CellGetDatum(cell), T_S2CELL,
    false));
}

/*****************************************************************************
 * Comparison operators
 *
 * Thin wrappers over the MEOS-layer `s2cell_eq / _lt / …` helpers declared in
 * `meos_s2cell.h`. The order of S2 cells on the uint64 payload follows the
 * Hilbert curve, so it carries locality where the quadbin order carries none,
 * and the operators are what btree indexing, ORDER BY and GROUP BY need.
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_eq);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is equal the second
 * @sqlop @p =
 * @sqlfn eq()
 */
Datum
S2cell_eq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_eq(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_ne);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is different the second
 * @sqlop @p <>
 * @sqlfn ne()
 */
Datum
S2cell_ne(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_ne(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_lt);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is less than the second
 * @sqlop @p <
 * @sqlfn lt()
 */
Datum
S2cell_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_lt(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_le);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is less than or equal to the second
 * @sqlop @p <=
 * @sqlfn le()
 */
Datum
S2cell_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_le(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_gt);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is greater than the second
 * @sqlop @p >
 * @sqlfn gt()
 */
Datum
S2cell_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_gt(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_ge);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return true if the first S2 cell is greater than or equal to the second
 * @sqlop @p >=
 * @sqlfn ge()
 */
Datum
S2cell_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_ge(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

PGDLLEXPORT Datum S2cell_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_cmp);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return -1, 0 or 1 as the first S2 cell is less than, equal to or
 * greater than the second
 * @sqlfn cmp()
 */
Datum
S2cell_cmp(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32(s2cell_cmp(PG_GETARG_S2CELL(0), PG_GETARG_S2CELL(1)));
}

/*****************************************************************************
 * Hash
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_hash);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return the 32-bit hash of an S2 cell
 * @sqlfn hash()
 * @altsqlfn s2cellHash()
 */
Datum
S2cell_hash(PG_FUNCTION_ARGS)
{
  PG_RETURN_UINT32(s2cell_hash(PG_GETARG_S2CELL(0)));
}

PGDLLEXPORT Datum S2cell_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_hash_extended);
/**
 * @ingroup mobilitydb_s2cell_base_comp
 * @brief Return the 64-bit hash of an S2 cell using a seed
 * @sqlfn hashExtended()
 * @altsqlfn s2cellHashExtended()
 */
Datum
S2cell_hash_extended(PG_FUNCTION_ARGS)
{
  S2CellId cell = PG_GETARG_S2CELL(0);
  uint64 seed = PG_GETARG_INT64(1);
  PG_RETURN_UINT64(s2cell_hash_extended(cell, seed));
}

/*****************************************************************************
 * Validity
 *****************************************************************************/

PGDLLEXPORT Datum S2cell_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_is_valid_cell);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return true if a value encodes a valid S2 cell
 * @sqlfn isValidCell()
 */
Datum
S2cell_is_valid_cell(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(s2cell_is_valid_cell(PG_GETARG_S2CELL(0)));
}

/*****************************************************************************/
