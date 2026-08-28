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
 * @brief Internal header for the first-party S2 cell kernel.
 *
 * Re-exports the public API in meos_s2cell.h to the implementation files in
 * meos/src/s2cell/, mirroring the meos/include/quadbin/ internal-header layout.
 */

#ifndef __S2CELL_H__
#define __S2CELL_H__

/* MEOS */
#include <meos.h>
#include <meos_s2cell.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetS2Cell(X)   ((S2CellId) DatumGetInt64(X))
#define S2CellGetDatum(X)   Int64GetDatum((int64) (X))
#define PG_GETARG_S2CELL(n) DatumGetS2Cell(PG_GETARG_DATUM(n))
#define PG_RETURN_S2CELL(x) PG_RETURN_DATUM(S2CellGetDatum(x))

/*****************************************************************************
 * Cell-id bit layout
 *****************************************************************************/

/** @brief Number of position bits below the three face bits */
#define S2_POS_BITS      (2 * S2_MAX_LEVEL + 1)
/** @brief Number of leaf cells along one side of a cube face */
#define S2_MAX_SIZE      (1u << S2_MAX_LEVEL)
/** @brief Bit at which the face field starts */
#define S2_FACE_SHIFT    S2_POS_BITS

/*****************************************************************************/

extern S2CellId s2cell_parse(const char *str);

extern uint64 s2cell_lsb(S2CellId cell);
extern uint64 s2cell_lsb_for_level(uint32_t level);
extern uint32_t s2cell_from_face_ij(uint32_t face, uint32_t i, uint32_t j,
  uint32_t level, S2CellId *result);
extern uint32_t s2cell_to_face_ij(S2CellId cell, uint32_t *i, uint32_t *j,
  uint32_t *orientation);

/* The raw lon/lat forms, internal as the quadbin twins are: they assert their
 * out-parameters where the public geometry forms in s2cell_geo.c test theirs */
extern void s2cell_cell_point(S2CellId cell, double *longitude,
  double *latitude);
extern void s2cell_cell_vertices(S2CellId cell, double *longitudes,
  double *latitudes);
extern void s2cell_cell_bounding_box(S2CellId cell, double *xmin, double *ymin,
  double *xmax, double *ymax);

/*****************************************************************************/

#endif /* __S2CELL_H__ */
