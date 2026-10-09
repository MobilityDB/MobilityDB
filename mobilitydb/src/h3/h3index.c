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
 * @brief PG V1 wrappers for the static `h3index` SQL type
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

/* The h3index type plumbing (in/out/recv/send), the comparison operators,
 * and the btree/hash operator classes are provided by the h3-pg extension,
 * which the mobilitydb extension now requires when built with H3. The
 * corresponding PG wrappers were removed together with their SQL. */

/*****************************************************************************
 * Text, WKB and HexWKB input/output
 *
 * The text representation is the one the type input and output functions of
 * h3-pg read and write. An h3index is a geographic cell with the constant default SRID WGS84
 * (EPSG:4326). asBinary/asHexWKB are the SRID-less base WKB (the inherited
 * Temporal<T> surface, like geography ST_AsBinary — the 4326 is implicit);
 * they mirror th3index exactly. The SRID-bearing EWKB form is the
 * TSpatial<T> overload, not exposed here. The output side reuses the generic
 * Datum_as_wkb / Datum_as_hexwkb dispatch; the input side calls the MEOS
 * helpers (which accept an absent SRID as 4326, or a present 4326).
 *****************************************************************************/

PGDLLEXPORT Datum H3index_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_as_text);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return the text representation of an h3index
 * @details Written as #Quadbin_as_text writes a quadbin
 * @sqlfn asText()
 */
Datum
H3index_as_text(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  char *str = meos_h3index_out(cell);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum H3index_from_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_from_text);
/**
 * @ingroup mobilitydb_h3_base_inout
 * @brief Return an h3index from its text representation
 * @details Read as #Quadbin_from_text reads a quadbin
 * @sqlfn h3indexFromText()
 */
Datum
H3index_from_text(PG_FUNCTION_ARGS)
{
  text *txt = PG_GETARG_TEXT_P(0);
  char *str = text_to_cstring(txt);
  H3Index result = meos_h3index_in(str);
  pfree(str);
  PG_FREE_IF_COPY(txt, 0);
  PG_RETURN_H3INDEX(result);
}

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


/*****************************************************************************
 * Comparison functions
 *
 * The h3 extension provides the comparison operators and the operator
 * classes of h3index; these functions are the names a query uses in every
 * engine, as #Quadbin_eq and its siblings are for quadbin.
 *****************************************************************************/

PGDLLEXPORT Datum H3index_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_eq);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if two h3index values are equal
 * @sqlfn eq()
 */
Datum
H3index_eq(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_eq(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_ne);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if two h3index values are not equal
 * @sqlfn ne()
 */
Datum
H3index_ne(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_ne(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_lt);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index value is less than the second one
 * @sqlfn lt()
 */
Datum
H3index_lt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_lt(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_le);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index value is less than or equal to the
 * second one
 * @sqlfn le()
 */
Datum
H3index_le(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_le(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_gt);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index value is greater than the second
 * one
 * @sqlfn gt()
 */
Datum
H3index_gt(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_gt(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_ge);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return true if the first h3index value is greater than or equal to
 * the second one
 * @sqlfn ge()
 */
Datum
H3index_ge(PG_FUNCTION_ARGS)
{
  PG_RETURN_BOOL(meos_h3index_ge(PG_GETARG_H3INDEX(0), PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_cmp);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return -1, 0 or 1 depending on whether the first h3index value is
 * less than, equal to or greater than the second one
 * @sqlfn cmp()
 */
Datum
H3index_cmp(PG_FUNCTION_ARGS)
{
  PG_RETURN_INT32(meos_h3index_cmp(PG_GETARG_H3INDEX(0),
    PG_GETARG_H3INDEX(1)));
}

PGDLLEXPORT Datum H3index_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_hash);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return the 32-bit hash value of an h3index value
 * @sqlfn hash()
 * @altsqlfn h3indexHash()
 */
Datum
H3index_hash(PG_FUNCTION_ARGS)
{
  PG_RETURN_UINT32(meos_h3index_hash(PG_GETARG_H3INDEX(0)));
}

PGDLLEXPORT Datum H3index_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(H3index_hash_extended);
/**
 * @ingroup mobilitydb_h3_base_comp
 * @brief Return the 64-bit hash value of an h3index value using a seed
 * @sqlfn hashExtended()
 * @altsqlfn h3indexHashExtended()
 */
Datum
H3index_hash_extended(PG_FUNCTION_ARGS)
{
  H3Index cell = PG_GETARG_H3INDEX(0);
  uint64 seed = PG_GETARG_INT64(1);
  PG_RETURN_UINT64(meos_h3index_hash_extended(cell, seed));
}

/*****************************************************************************/
