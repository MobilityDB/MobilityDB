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
 * @brief Internal helpers for the static `h3index` SQL type.
 *
 * The static h3index type is the analogue of the static `cbuffer`
 * type — it provides the base value that the temporal `th3index`
 * type carries through time. Unlike `cbuffer`, h3index has no
 * compound payload (it is a 64-bit integer cell identifier), so
 * the helpers here are minimal:
 *
 *   * an input parser that accepts both decimal and hex strings,
 *   * an output formatter (canonical form is hex, matching h3-pg),
 *   * comparison / ordering / hashing helpers — exposed at the MEOS
 *     layer so MobilityDuck and other consumers can reuse them
 *     without re-implementing the int64 bit-compare logic.
 */

#ifndef __H3INDEX_H__
#define __H3INDEX_H__

#include <stdbool.h>
#include <stdint.h>
/* PostgreSQL — for Datum / Int64GetDatum / DatumGetInt64 */
#include <postgres.h>
#include <h3api.h>

/*****************************************************************************
 * Datum packing
 *
 * H3Index is a uint64, binary-identical to int8 in PG's Datum
 * representation on 64-bit platforms. These macros hide the int64
 * round-trip so call sites read as H3Index-typed.
 *****************************************************************************/

#define DatumGetH3Index(X)   ((H3Index) DatumGetInt64(X))
#define H3IndexGetDatum(X)   Int64GetDatum((int64) (X))

/*****************************************************************************
 * Parsing / formatting
 *****************************************************************************/

/**
 * Parse a string into an H3Index. Accepts:
 *   * a decimal integer literal (e.g. "590464338553208831"),
 *   * a hex string with no prefix (e.g. "8a2a1072b59ffff"), the
 *     canonical h3-pg form,
 *   * a hex string with the "0x" prefix (e.g. "0x8a2a1072b59ffff").
 *
 * Returns the parsed cell on success. Raises `meos_error(ERROR, …)`
 * on malformed input or on a value that does not encode a valid H3
 * cell (libh3's `isValidCell`).
 */
extern H3Index h3index_parse(const char *str);

/**
 * Format an H3Index into its canonical hex string (lowercase, no
 * "0x" prefix, no leading zeros — matches h3-pg's output). The
 * caller owns the returned `palloc`'d C string.
 */
extern char *h3index_to_string(H3Index cell);

/*****************************************************************************
 * Comparison / ordering / hashing
 *
 * H3 cell identifiers are uint64; ordering and equality fall through
 * to plain int64 bit-compare — they carry no geographic meaning but
 * are required for btree indexing, ORDER BY, GROUP BY, DISTINCT, etc.
 *****************************************************************************/

extern bool h3index_eq(H3Index a, H3Index b);
extern bool h3index_ne(H3Index a, H3Index b);
extern bool h3index_lt(H3Index a, H3Index b);
extern bool h3index_le(H3Index a, H3Index b);
extern bool h3index_gt(H3Index a, H3Index b);
extern bool h3index_ge(H3Index a, H3Index b);
extern int h3index_cmp(H3Index a, H3Index b);
extern uint32 h3index_hash(H3Index cell);

#endif /* __H3INDEX_H__ */
