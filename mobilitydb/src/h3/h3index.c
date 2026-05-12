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
/* MEOS */
#include <h3api.h>
#include <meos.h>
#include "h3/h3index.h"
#include "h3/h3_generated.h"

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
  PG_RETURN_H3INDEX(h3index_parse(str));
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
  PG_RETURN_CSTRING(h3index_to_string(cell));
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
 * @sqlfn h3_is_valid_cell()
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
 * @sqlfn h3_is_valid_directed_edge()
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
 * @sqlfn h3_is_valid_vertex()
 */
Datum
H3index_is_valid_vertex(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(h3_is_valid_vertex_meos(PG_GETARG_H3INDEX(0)));
}

/*****************************************************************************/
