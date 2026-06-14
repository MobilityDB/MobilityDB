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
 * @brief PG V1 wrappers for the static `quadbin` SQL type.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
/* MEOS */
#include <meos.h>
#include <meos_quadbin.h>
#include "quadbin/quadbin_meos.h"

/* DatumGetQuadbin / QuadbinGetDatum live in quadbin/quadbin_meos.h.
 * PG_GETARG_QUADBIN / PG_RETURN_QUADBIN are the fmgr-layer
 * conveniences defined locally here because fmgr.h is a
 * MobilityDB-side dependency. */
#define PG_GETARG_QUADBIN(n) DatumGetQuadbin(PG_GETARG_DATUM(n))
#define PG_RETURN_QUADBIN(x) PG_RETURN_DATUM(QuadbinGetDatum(x))

/*****************************************************************************
 * Input / output
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_in);
/**
 * @ingroup mobilitydb_quadbin_base_inout
 * @brief Parse a quadbin value from its string representation
 * @sqlfn quadbin_in()
 */
Datum
Quadbin_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_QUADBIN(quadbin_parse(str));
}

PGDLLEXPORT Datum Quadbin_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_out);
/**
 * @ingroup mobilitydb_quadbin_base_inout
 * @brief Render a quadbin value as its canonical hex string
 * @sqlfn quadbin_out()
 */
Datum
Quadbin_out(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  PG_RETURN_CSTRING(quadbin_index_to_string(cell));
}

PGDLLEXPORT Datum Quadbin_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_recv);
/**
 * @ingroup mobilitydb_quadbin_base_inout
 * @brief Receive a quadbin value over the binary wire protocol
 */
Datum
Quadbin_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_QUADBIN((Quadbin) pq_getmsgint64(buf));
}

PGDLLEXPORT Datum Quadbin_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_send);
/**
 * @ingroup mobilitydb_quadbin_base_inout
 * @brief Send a quadbin value over the binary wire protocol
 */
Datum
Quadbin_send(PG_FUNCTION_ARGS)
{
  Quadbin cell = PG_GETARG_QUADBIN(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, (int64) cell);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Comparison operators
 *
 * Thin wrappers over the MEOS-layer `quadbin_eq / _lt / …` helpers
 * declared in `meos_quadbin.h`. quadbin cell ordering on the uint64
 * payload has no geographic meaning, but the operators are required
 * for btree indexing, ORDER BY, GROUP BY, etc.
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_eq);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if two quadbin values are equal
 * @sqlop @p =
 */
Datum
Quadbin_eq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_eq(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_ne);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if two quadbin values are not equal
 * @sqlop @p <>
 */
Datum
Quadbin_ne(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_ne(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_lt);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if the first quadbin is less than the second
 * @sqlop @p <
 */
Datum
Quadbin_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_lt(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_le);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if the first quadbin is less than or equal to
 * the second
 * @sqlop @p <=
 */
Datum
Quadbin_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_le(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_gt);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than the second
 * @sqlop @p >
 */
Datum
Quadbin_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_gt(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_ge);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return true if the first quadbin is greater than or equal
 * to the second
 * @sqlop @p >=
 */
Datum
Quadbin_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_ge(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_cmp);
/**
 * @ingroup mobilitydb_quadbin_base_comp
 * @brief Return the btree-style comparison of two quadbin values
 * (-1 / 0 / +1)
 */
Datum
Quadbin_cmp(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32(quadbin_cmp(PG_GETARG_QUADBIN(0), PG_GETARG_QUADBIN(1)));
}

PGDLLEXPORT Datum Quadbin_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_hash);
/**
 * @ingroup mobilitydb_quadbin_base_accessor
 * @brief Return the hash code of a quadbin value
 */
Datum
Quadbin_hash(PG_FUNCTION_ARGS)
{
  PG_RETURN_UINT32(quadbin_hash(PG_GETARG_QUADBIN(0)));
}

/*****************************************************************************
 * Validity predicates — thin wrappers over the first-party kernel
 * `quadbin_is_valid_*` checks declared in `meos_quadbin.h`.
 *****************************************************************************/

PGDLLEXPORT Datum Quadbin_is_valid_index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_is_valid_index);
/**
 * @ingroup mobilitydb_quadbin_base_accessor
 * @brief Return true if the value encodes a valid CARTO quadbin index
 * @sqlfn quadbin_is_valid_index()
 */
Datum
Quadbin_is_valid_index(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_is_valid_index(PG_GETARG_QUADBIN(0)));
}

PGDLLEXPORT Datum Quadbin_is_valid_cell(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Quadbin_is_valid_cell);
/**
 * @ingroup mobilitydb_quadbin_base_accessor
 * @brief Return true if the value encodes a valid quadbin cell at a
 * concrete resolution
 * @sqlfn quadbin_is_valid_cell()
 */
Datum
Quadbin_is_valid_cell(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(quadbin_is_valid_cell(PG_GETARG_QUADBIN(0)));
}

/*****************************************************************************/
