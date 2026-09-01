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
 * @brief Internal declarations for the ts2cell type-inheritance boilerplate.
 *
 * The analogue of `meos/include/quadbin/tquadbin.h`. It carries the extern
 * declarations for the helpers in `ts2cell.c` that do not belong in the public
 * `meos_s2cell.h`. The validity macro `VALIDATE_TS2CELL(temp, ret)`, used in
 * every lifted function and public accessor, lives in the public header
 * alongside the other type-validation macros so bindings reach it directly.
 *
 * An S2 cell is defined on the sphere, so the temporal point bridge answers a
 * `tgeogpoint`, where the Web-Mercator quadbin answers a `tgeompoint`.
 */

#ifndef __TS2CELL_H__
#define __TS2CELL_H__

#include <stdbool.h>
#include <stdint.h>

#include <meos.h>
#include <meos_s2cell.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Validators (bodies in ts2cell.c)
 *****************************************************************************/

/**
 * @brief Ensure a (ts2cell, ts2cell) operand pair is safe to combine — both
 * are the right temptype and are synchronisable.
 */
extern bool ensure_valid_ts2cell_ts2cell(const Temporal *temp1,
  const Temporal *temp2);

/**
 * @brief Ensure a (ts2cell, s2cell) pair — the bare `S2CellId` is validated
 * for encoding a cell, zero being the canonical invalid sentinel.
 */
extern bool ensure_valid_ts2cell_s2cell(const Temporal *temp, S2CellId cell);

/**
 * @brief Ensure a (ts2cell, tgeogpoint) pair — used by the geodetic point
 * bridges.
 */
extern bool ensure_valid_ts2cell_tgeogpoint(const Temporal *temp1,
  const Temporal *temp2);

#endif /* __TS2CELL_H__ */
