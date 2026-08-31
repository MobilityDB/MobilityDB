/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 *   * an input parser that reads the canonical hexadecimal cell literal,
 *     with an optional "0x" prefix and at most 16 significant digits, and
 *     that requires the value to denote a cell, a directed edge or a vertex,
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
/* MEOS — public h3index declarations (I/O, comparison, hashing) and the
 * `VALIDATE_H3INDEX_*` macros */
#include <meos_h3.h>

/*****************************************************************************
 * Internal twins of the public h3index functions
 *
 * The bare names collide with the C symbols of the h3-pg PostgreSQL
 * extension; the MobilityDB extension build must not export them, so
 * internal call sites use the meos_-prefixed twins and the bare names
 * are compiled only under MEOS (see h3index.c).
 *****************************************************************************/

extern H3Index meos_h3index_in(const char *str);
extern char *meos_h3index_out(H3Index cell);
extern bool meos_h3index_eq(H3Index a, H3Index b);
extern bool meos_h3index_ne(H3Index a, H3Index b);
extern bool meos_h3index_lt(H3Index a, H3Index b);
extern bool meos_h3index_le(H3Index a, H3Index b);
extern bool meos_h3index_gt(H3Index a, H3Index b);
extern bool meos_h3index_ge(H3Index a, H3Index b);
extern int meos_h3index_cmp(H3Index a, H3Index b);
extern uint32 meos_h3index_hash(H3Index cell);

/*****************************************************************************
 * Validators (bodies in h3index.c)
 *
 * The per-mode checks behind the `VALIDATE_H3INDEX_*` macros of the public
 * `meos_h3.h`. Each reports the offending value with the mode the operation
 * required, since the three modes share the same 64-bit representation and
 * the error is otherwise indistinguishable from a plain typo.
 *****************************************************************************/

extern bool ensure_h3index_cell(H3Index cell);
extern bool ensure_h3index_directed_edge(H3Index edge);
extern bool ensure_h3index_vertex(H3Index vertex);

/*****************************************************************************
 * Datum packing
 *
 * H3Index is a uint64, binary-identical to int8 in PG's Datum
 * representation on 64-bit platforms. These macros hide the int64
 * round-trip so call sites read as H3Index-typed.
 *****************************************************************************/

#define DatumGetH3Index(X)   ((H3Index) DatumGetInt64(X))
#define H3IndexGetDatum(X)   Int64GetDatum((int64) (X))

#endif /* __H3INDEX_H__ */
