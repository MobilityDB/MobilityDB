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
 * @brief MEOS lifting for grid-traversal functions, plus the two
 * static local-IJ adapters that back them.
 *
 * `h3_grid_distance_meos` is auto-generated (see `h3_generated.h`);
 * the local-IJ pair is hand-written because h3-pg uses PG `Point`
 * varlenas that we trade for planar (SRID 0, non-geodetic)
 * GSERIALIZED carriers so the lifted `datum_h3_cell_to_local_ij`
 * can feed the result straight into a `tgeompoint`.
 */

#include <string.h>

#include <meos.h>
#include <meos_h3.h>
#include <h3api.h>

#include "geo/tgeo_spatialfuncs.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * Static adapters — local-IJ coordinates
 *****************************************************************************/

/**
 * @brief Emit a planar (SRID 0, non-geodetic) point carrying the H3
 * local-IJ coordinates as `x = I`, `y = J`. H3 local coordinates
 * have no geodesic interpretation, so a planar carrier is the right
 * fit for downstream `tgeompoint` lifting.
 */
GSERIALIZED *
h3_cell_to_local_ij_meos(H3Index origin, H3Index cell)
{
  CoordIJ coord;
  if (cellToLocalIj(origin, cell, 0, &coord) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return geopoint_make((double) coord.i, (double) coord.j, 0.0,
    false, false, SRID_UNKNOWN);
}

H3Index
h3_local_ij_to_cell_meos(H3Index origin, const GSERIALIZED *coord)
{
  const POINT2D *p = GSERIALIZED_POINT2D_P(coord);
  CoordIJ ij = { .i = (int) p->x, .j = (int) p->y };
  H3Index cell;
  if (localIjToCell(origin, &ij, 0, &cell) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return (H3Index) 0;
  }
  return cell;
}

/*****************************************************************************
 * h3_grid_distance — binary_synced (th3index, th3index) → tbigint
 *****************************************************************************/

/**
 * @ingroup meos_h3_traversal
 * @brief Return the temporal grid-hop distance between two temporal H3
 * cells.
 *
 * Shared with the `<->` operator on th3index (grid-hop distance,
 * not the arithmetic distance that tnumber's `<->` would give).
 */
Temporal *
th3index_grid_distance(const Temporal *origin, const Temporal *dest)
{
  assert(origin); assert(dest);
  assert(origin->temptype == T_TH3INDEX);
  assert(dest->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_grid_distance;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.argtype[1] = T_TH3INDEX;
  lfinfo.restype = T_TBIGINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, dest, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_local_ij — binary_synced (th3index, th3index) → tgeompoint
 *****************************************************************************/

/**
 * @ingroup meos_h3_traversal
 * @brief Return the temporal local (I, J) pair of a cell anchored on
 * another cell, carried as a temporal planar point.
 */
Temporal *
th3index_cell_to_local_ij(const Temporal *origin, const Temporal *cell)
{
  assert(origin); assert(cell);
  assert(origin->temptype == T_TH3INDEX);
  assert(cell->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_local_ij;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.argtype[1] = T_TH3INDEX;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, cell, &lfinfo);
}

/*****************************************************************************
 * h3_local_ij_to_cell — binary_synced (th3index, tgeompoint) → th3index
 *****************************************************************************/

/**
 * @ingroup meos_h3_traversal
 * @brief Return the temporal H3 cell at a temporal local (I, J) coord
 * anchored on a temporal origin cell.
 */
Temporal *
th3index_local_ij_to_cell(const Temporal *origin, const Temporal *coord)
{
  assert(origin); assert(coord);
  assert(origin->temptype == T_TH3INDEX);
  assert(coord->temptype == T_TGEOMPOINT);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_local_ij_to_cell;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.argtype[1] = T_TGEOMPOINT;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, coord, &lfinfo);
}

/*****************************************************************************/
