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
 * @brief The `h3_cellops` descriptor that plugs h3 into the shared temporal
 * cell-index machinery (meos/src/temporal/tcellindex.c).
 *
 * The Datum-convention wrappers of the h3 kernel live alongside the base type
 * in h3index.c, so this file holds the descriptor and the single adapter the
 * descriptor needs: the shared `cell_area` slot is a one-argument function
 * returning square metres, whereas the h3 kernel takes the unit as a second
 * argument.
 *
 * An h3 cell is geodetic, so the descriptor emits a tgeogpoint centroid and,
 * through it, a tgeography boundary. The static adapter behind the centroid
 * builds an SRID-4326 point, the coordinates the H3 specification carries on a
 * sphere of the WGS84/EPSG:4326 authalic radius
 * (https://h3geo.org/docs/core-library/overview/); geography and geometry are distinguished at the
 * lifting layer by `point_temptype`, exactly as the typed th3index conversions
 * distinguish them by their `restype`.
 */

#include "h3/th3index_internal.h"

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_h3.h>
#include "temporal/meos_catalog.h"
#include "temporal/tcellindex.h"

/*****************************************************************************
 * Datum-convention adapter
 *****************************************************************************/

/**
 * @brief Return the area of an h3 cell in square metres, the unit the shared
 * `cell_area` slot is defined in.
 */
static Datum
datum_h3_cell_area_m2(Datum cell_d)
{
  return datum_h3_cell_area(cell_d, Int32GetDatum((int32) H3_UNIT_M2));
}

/*****************************************************************************
 * Descriptor
 *****************************************************************************/

/**
 * @brief H3 operations descriptor consumed by `dggs_cellops()`.
 */
const DggsCellOps h3_cellops =
{
  .celltype        = T_H3INDEX,
  .settype         = T_H3INDEXSET,
  .temptype        = T_TH3INDEX,
  .min_resolution  = 0,
  .max_resolution  = 15,
  .point_temptype  = T_TGEOGPOINT,
  .point_srid      = 4326,
  .get_resolution  = &datum_h3_get_resolution,
  .is_valid_cell   = &datum_h3_is_valid_cell,
  .cell_to_parent  = &datum_h3_cell_to_parent,
  .cell_to_point   = &datum_h3_cell_to_latlng,
  .cell_to_boundary = &datum_h3_cell_to_boundary,
  .cell_area       = &datum_h3_cell_area_m2
};

/*****************************************************************************/
