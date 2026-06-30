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
 * @brief PG V1 wrappers for the static `h3index` SQL type.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
/* MEOS */
#include <h3api.h>
#include <meos.h>
#include <meos_h3.h>
#include "h3/h3index.h"
#include "h3/h3_generated.h"
#include "pg_temporal/temporal.h"

/* DatumGetH3Index / H3IndexGetDatum live in h3index.h.
 * PG_GETARG_H3INDEX / PG_RETURN_H3INDEX are the fmgr-layer
 * conveniences defined locally here because fmgr.h is a
 * MobilityDB-side dependency. */
#define PG_GETARG_H3INDEX(n) DatumGetH3Index(PG_GETARG_DATUM(n))
#define PG_RETURN_H3INDEX(x) PG_RETURN_DATUM(H3IndexGetDatum(x))

/*****************************************************************************
 * Input / output
 *****************************************************************************/

PGDLLEXPORT Datum H3index_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_in);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Parse an h3index value from its string representation
 * @sqlfn h3index_in()
 */
Datum
H3index_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_H3INDEX(h3index_in(str));
}

PGDLLEXPORT Datum H3index_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_out);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Render an h3index value as its canonical hex string
 * @sqlfn h3index_out()
 */
Datum
H3index_out(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  PG_RETURN_CSTRING(h3index_out(cell));
}

PGDLLEXPORT Datum H3index_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_recv);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Receive an h3index value over the binary wire protocol
 */
Datum
H3index_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_H3INDEX((H3Index) pq_getmsgint64(buf));
}

PGDLLEXPORT Datum H3index_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_send);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Send an h3index value over the binary wire protocol
 */
Datum
H3index_send(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, (int64) cell);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * WKB and HexWKB input/output
 *
 * An h3index is a geographic cell with the constant default SRID WGS84
 * (EPSG:4326). asBinary/asHexWKB are the SRID-less base WKB (the inherited
 * Temporal<T> surface, like geography ST_AsBinary — the 4326 is implicit);
 * they mirror th3index exactly. The SRID-bearing EWKB form is the
 * TSpatial<T> overload, not exposed here. The output side reuses the generic
 * Datum_as_wkb / Datum_as_hexwkb dispatch; the input side calls the MEOS
 * helpers (which accept an absent SRID as 4326, or a present 4326).
 *****************************************************************************/

PGDLLEXPORT Datum H3index_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_from_wkb);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return an h3index from its Well-Known Binary (WKB) representation
 * @sqlfn h3indexFromBinary()
 */
Datum
H3index_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  H3Index result = h3index_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_H3INDEX(result);
}

PGDLLEXPORT Datum H3index_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_from_hexwkb);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return an h3index from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn h3indexFromHexWKB()
 */
Datum
H3index_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text_to_cstring(hexwkb_text);
  H3Index result = h3index_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_H3INDEX(result);
}

PGDLLEXPORT Datum H3index_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_as_wkb);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of an h3index
 * @sqlfn asBinary()
 */
Datum
H3index_as_wkb(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, H3IndexGetDatum(cell), T_H3INDEX,
    false));
}

PGDLLEXPORT Datum H3index_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_as_hexwkb);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of an h3index
 * @sqlfn asHexWKB()
 */
Datum
H3index_as_hexwkb(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, H3IndexGetDatum(cell), T_H3INDEX,
    false));
}

/*****************************************************************************
 * Comparison operators
 *
 * Thin wrappers over the MEOS-layer `h3index_eq / _lt / …` helpers
 * declared in `h3/h3index.h`. h3 cell ordering on the int64 payload
 * has no geographic meaning, but the operators are required for
 * btree indexing, ORDER BY, GROUP BY, etc.
 *****************************************************************************/

PGDLLEXPORT Datum H3index_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_eq);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if two h3index values are equal
 * @sqlop @p =
 */
Datum
H3index_eq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_eq(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_ne);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if two h3index values are not equal
 * @sqlop @p <>
 */
Datum
H3index_ne(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_ne(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_lt);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index is less than the second
 * @sqlop @p <
 */
Datum
H3index_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_lt(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_le);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index is less than or equal to
 * the second
 * @sqlop @p <=
 */
Datum
H3index_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_le(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_gt);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index is greater than the second
 * @sqlop @p >
 */
Datum
H3index_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_gt(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_ge);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index is greater than or equal
 * to the second
 * @sqlop @p >=
 */
Datum
H3index_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3index_ge(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cmp);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return the btree-style comparison of two h3index values
 * (-1 / 0 / +1)
 */
Datum
H3index_cmp(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32(h3index_cmp(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_hash);
/**
 * @ingroup mobilitydb_h3_base_accessor
 * @brief Return the hash code of an h3index value
 */
Datum
H3index_hash(PG_FUNCTION_ARGS)
{
  PG_RETURN_UINT32(h3index_hash(PG_GETARG_H3INDEX(0)));
}

/*****************************************************************************
 * Validity predicates — thin wrappers over the MEOS-side
 * `h3_is_valid_*_meos` functions from `h3_generated.h`.
 *****************************************************************************/

PGDLLEXPORT Datum H3index_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_is_valid_cell);
/**
 * @ingroup mobilitydb_h3_base_accessor
 * @brief Return true if the value encodes a valid H3 cell
 * @sqlfn isValidCell()
 */
Datum
H3index_is_valid_cell(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3_is_valid_cell_meos(PG_GETARG_H3INDEX(0)));
}

PGDLLEXPORT Datum H3index_is_valid_directed_edge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_is_valid_directed_edge);
/**
 * @ingroup mobilitydb_h3_base_accessor
 * @brief Return true if the value encodes a valid H3 directed edge
 * @sqlfn isValidDirectedEdge()
 */
Datum
H3index_is_valid_directed_edge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3_is_valid_directed_edge_meos(PG_GETARG_H3INDEX(0)));
}

PGDLLEXPORT Datum H3index_is_valid_vertex(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_is_valid_vertex);
/**
 * @ingroup mobilitydb_h3_base_accessor
 * @brief Return true if the value encodes a valid H3 vertex
 * @sqlfn isValidVertex()
 */
Datum
H3index_is_valid_vertex(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3_is_valid_vertex_meos(PG_GETARG_H3INDEX(0)));
}

/*****************************************************************************/
