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
 * @brief Internal declarations for the tquadbin type-inheritance
 * boilerplate.
 *
 * This header is the analogue of `meos/include/h3/th3index.h`.
 * It carries the extern decls for the static helpers in `tquadbin.c`
 * that do not belong in the public `meos_quadbin.h`. The validity
 * macro `VALIDATE_TQUADBIN(temp, ret)`, used in every lifted function
 * and public accessor, lives in the public `meos_quadbin.h` alongside
 * the other type-validation macros so bindings can use it directly.
 *
 * Quadbin is the square/Web-Mercator (planar) counterpart of the
 * hexagonal/geodetic H3 index, so its temporal point bridge uses
 * `tgeompoint` rather than `tgeogpoint`.
 */

#ifndef __TQUADBIN_H__
#define __TQUADBIN_H__

#include <stdbool.h>
#include <stdint.h>

#include <meos.h>
#include <meos_quadbin.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Validators (bodies in tquadbin.c)
 *****************************************************************************/

/**
 * @brief Ensure a (tquadbin, tquadbin) operand pair is safe to combine —
 * both are the right temptype and are synchronisable.
 */
extern bool ensure_valid_tquadbin_tquadbin(const Temporal *temp1,
  const Temporal *temp2);

/**
 * @brief Ensure a (tquadbin, quadbin) pair — the bare `Quadbin` is validated
 * only for non-zero (quadbin cells are non-zero by construction; zero is the
 * canonical invalid sentinel).
 */
extern bool ensure_valid_tquadbin_quadbin(const Temporal *temp, Quadbin cell);

/**
 * @brief Ensure a (tquadbin, tgeompoint) pair — used by the Web-Mercator
 * planar point bridges.
 */
extern bool ensure_valid_tquadbin_tgeompoint(const Temporal *temp1,
  const Temporal *temp2);

/*****************************************************************************
 * Low-level Datum-returning comparison primitives (used by tquadbin
 * comparison-operator wrappers)
 *****************************************************************************/

/**
 * @brief Compare two quadbin cell Datums for equality; used as the
 * per-instant comparison function by the ever/always dispatchers.
 * Delegates to bit equality at the int64 level.
 */
extern Datum datum2_quadbin_eq(Datum d1, Datum d2, MeosType type);
extern Datum datum2_quadbin_ne(Datum d1, Datum d2, MeosType type);

#endif /* __TQUADBIN_H__ */
