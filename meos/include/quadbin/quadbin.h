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
 * @brief Internal header for the first-party quadbin cell kernel.
 *
 * Re-exports the public API in meos_quadbin.h to the implementation files in
 * meos/src/quadbin/, mirroring the meos/include/h3/ internal-header layout.
 */

#ifndef __QUADBIN_H__
#define __QUADBIN_H__

/* MEOS */
#include <meos.h>
#include <meos_quadbin.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetQuadbin(X)   ((Quadbin) DatumGetInt64(X))
#define QuadbinGetDatum(X)   Int64GetDatum((int64) (X))
#define PG_GETARG_QUADBIN(n) DatumGetQuadbin(PG_GETARG_DATUM(n))
#define PG_RETURN_QUADBIN(x) PG_RETURN_DATUM(QuadbinGetDatum(x))

/*****************************************************************************/

extern Quadbin quadbin_parse(const char *str);

extern void quadbin_cell_tile(Quadbin cell, uint32_t *x, uint32_t *y,
  uint32_t *z);
extern void quadbin_cell_point(Quadbin cell, double *longitude,
  double *latitude);
extern void quadbin_cell_bounding_box(Quadbin cell, double *xmin,
  double *ymin, double *xmax, double *ymax);

/*****************************************************************************/

#endif /* __QUADBIN_H__ */
