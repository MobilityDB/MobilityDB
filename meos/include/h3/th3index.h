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
 * @brief Internal declarations for the th3index type-inheritance
 * boilerplate.
 *
 * This header is the analogue of `meos/include/cbuffer/tcbuffer.h`.
 * It carries the extern decls for the static helpers in `th3index.c`
 * that do not belong in the public `meos_h3.h`. The validity macro
 * `VALIDATE_TH3INDEX(temp, ret)`, used in every lifted function and
 * public accessor, lives in the public `meos_h3.h` alongside the
 * other type-validation macros so bindings can use it directly.
 */

#ifndef __TH3INDEX_H__
#define __TH3INDEX_H__

#include <stdbool.h>
#include <stdint.h>
#include <h3api.h>

#include <meos.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Validators (bodies in th3index.c)
 *****************************************************************************/

/**
 * @brief Ensure a (th3index, th3index) operand pair is safe to combine —
 * both are the right temptype and are synchronisable.
 */
extern bool ensure_valid_th3index_th3index(const Temporal *temp1,
  const Temporal *temp2);

/**
 * @brief Ensure a (th3index, h3index) pair — the bare `H3Index` is validated
 * only for non-zero (h3 cells are non-zero by construction; zero is the
 * canonical invalid sentinel).
 */
extern bool ensure_valid_th3index_h3index(const Temporal *temp, H3Index cell);

/**
 * @brief Ensure a (th3index, tgeogpoint) pair — used by
 * `tgeogpoint_great_circle_distance` and future geodetic bridges.
 */
extern bool ensure_valid_th3index_tgeogpoint(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Low-level Datum-returning accessors (used by th3index_compops.c)
 *****************************************************************************/

/**
 * @brief Compare two H3 cell Datums for equality; used as the per-instant
 * comparison function by the ever/always dispatchers. Delegates to
 * `datum2_eq` since h3 cell equality is bit equality at the int64 level.
 */
extern Datum datum2_h3index_eq(Datum d1, Datum d2, MeosType type);
extern Datum datum2_h3index_ne(Datum d1, Datum d2, MeosType type);

#endif /* __TH3INDEX_H__ */
