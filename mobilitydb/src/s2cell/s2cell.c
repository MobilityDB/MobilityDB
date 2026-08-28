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
 * @brief PG V1 wrappers for the static `s2cell` SQL type.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <libpq/pqformat.h>
/* MEOS */
#include <meos.h>
#include <meos_s2cell.h>
#include "s2cell/s2cell.h"

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
  PG_RETURN_S2CELL((S2CellId) pq_getmsgint64(buf));
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
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the 32-bit hash of an S2 cell
 * @sqlfn hash()
 */
Datum
S2cell_hash(PG_FUNCTION_ARGS)
{
  PG_RETURN_UINT32(s2cell_hash(PG_GETARG_S2CELL(0)));
}

PGDLLEXPORT Datum S2cell_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(S2cell_hash_extended);
/**
 * @ingroup mobilitydb_s2cell_base_accessor
 * @brief Return the 64-bit hash of an S2 cell using a seed
 * @sqlfn hashExtended()
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
