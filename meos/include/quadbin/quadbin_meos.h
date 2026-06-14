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
 * @brief Internal helpers for the static `quadbin` SQL type.
 *
 * The static quadbin type is the analogue of the static `h3index`
 * type — it provides the base value that the temporal `tquadbin`
 * type carries through time. Like h3index, quadbin has no compound
 * payload (it is a 64-bit integer cell identifier), so the helpers
 * here are minimal:
 *
 *   * an input parser that validates the quadbin cell structure,
 *   * an output formatter (canonical form is lowercase hex),
 *   * comparison / ordering / hashing helpers — exposed at the MEOS
 *     layer so MobilityDuck and other consumers can reuse them
 *     without re-implementing the int64 bit-compare logic.
 *
 * The pure bit/value kernel (parse without validation, formatting,
 * tile/hierarchy/lat-lng math) lives in meos/src/quadbin/quadbin.c
 * and is not duplicated here.
 */

#ifndef __QUADBIN_MEOS_H__
#define __QUADBIN_MEOS_H__

#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL — for Datum / Int64GetDatum / DatumGetInt64 */
#include <postgres.h>
/* MEOS */
#include <meos_quadbin.h>

/*****************************************************************************
 * Datum packing
 *
 * Quadbin is a uint64, binary-identical to int8 in PG's Datum
 * representation on 64-bit platforms. These macros hide the int64
 * round-trip so call sites read as Quadbin-typed.
 *****************************************************************************/

#define DatumGetQuadbin(X)   ((Quadbin) DatumGetInt64(X))
#define QuadbinGetDatum(X)   Int64GetDatum((int64) (X))

/*****************************************************************************
 * Parsing / formatting
 *
 * The output formatter `quadbin_index_to_string` is provided by the
 * kernel (meos/src/quadbin/quadbin.c). The validating parser below
 * wraps the kernel `quadbin_string_to_index`.
 *****************************************************************************/

/**
 * Parse a string into a quadbin cell. Accepts a lowercase or
 * uppercase hexadecimal string of the 64-bit index, with an optional
 * "0x" prefix.
 *
 * Returns the parsed cell on success. Raises `meos_error(ERROR, …)`
 * on malformed input or on a value that does not encode a valid
 * quadbin cell.
 */
extern Quadbin quadbin_parse(const char *str);

/*****************************************************************************
 * Comparison / ordering / hashing
 *
 * Quadbin cell identifiers are uint64; ordering and equality fall
 * through to plain int64 bit-compare — they carry no geographic
 * meaning but are required for btree indexing, ORDER BY, GROUP BY,
 * DISTINCT, etc.
 *****************************************************************************/

extern bool quadbin_eq(Quadbin a, Quadbin b);
extern bool quadbin_ne(Quadbin a, Quadbin b);
extern bool quadbin_lt(Quadbin a, Quadbin b);
extern bool quadbin_le(Quadbin a, Quadbin b);
extern bool quadbin_gt(Quadbin a, Quadbin b);
extern bool quadbin_ge(Quadbin a, Quadbin b);
extern int quadbin_cmp(Quadbin a, Quadbin b);
extern uint32 quadbin_hash(Quadbin cell);

#endif /* __QUADBIN_MEOS_H__ */
