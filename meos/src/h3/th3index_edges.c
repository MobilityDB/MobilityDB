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
 * @brief MEOS lifting for directed-edge functions, plus the static
 * `h3_directed_edge_to_gs_boundary` adapter that backs the one
 * boundary-returning entry.
 *
 * Five of the six lifted entries call `_meos` helpers in
 * `h3_generated.c` directly. The sixth
 * (`th3index_directed_edge_to_boundary`) needs a GSERIALIZED hand
 * body — defined below and shared with `th3index_latlng.c` via the
 * `cell_boundary_to_gs` helper declared in `th3index_internal.h`.
 */

#include <string.h>

#include <meos.h>
#include <meos_h3.h>
#include <h3api.h>

#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * Static adapter — directed edge → polygon boundary
 *****************************************************************************/

GSERIALIZED *
h3_directed_edge_to_gs_boundary(H3Index edge)
{
  CellBoundary bnd;
  if (directedEdgeToBoundary(edge, &bnd) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  /* h3-pg emits a closed POLYGON for directed edges (same container
   * as cells), swapping lat/lng in the process — a known upstream
   * quirk. We emit a closed POLYGON too so consumers see a uniform
   * shape, but keep x = lng, y = lat as in
   * `h3_cell_to_gs_boundary`. */
  return cell_boundary_to_gs(&bnd);
}

/*****************************************************************************
 * h3_are_neighbor_cells — binary_synced
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return a temporal boolean stating whether two temporal cells are
 * grid neighbours at each instant.
 */
Temporal *
th3index_are_neighbor_cells(const Temporal *origin, const Temporal *dest)
{
  assert(origin); assert(dest);
  assert(origin->temptype == T_TH3INDEX);
  assert(dest->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_are_neighbor_cells;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.argtype[1] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, dest, &lfinfo);
}

/*****************************************************************************
 * h3_cells_to_directed_edge — binary_synced
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return a temporal directed-edge index from two temporal cells.
 */
Temporal *
th3index_cells_to_directed_edge(const Temporal *origin, const Temporal *dest)
{
  assert(origin); assert(dest);
  assert(origin->temptype == T_TH3INDEX);
  assert(dest->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cells_to_directed_edge;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.argtype[1] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(origin, dest, &lfinfo);
}

/*****************************************************************************
 * h3_is_valid_directed_edge
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 directed edge.
 */
Temporal *
th3index_is_valid_directed_edge(const Temporal *edge)
{
  assert(edge); assert(edge->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_is_valid_directed_edge;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(edge, &lfinfo);
}

/*****************************************************************************
 * h3_get_directed_edge_origin
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return the temporal origin cell of a temporal directed edge.
 */
Temporal *
th3index_get_directed_edge_origin(const Temporal *edge)
{
  assert(edge); assert(edge->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_get_directed_edge_origin;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(edge, &lfinfo);
}

/*****************************************************************************
 * h3_get_directed_edge_destination
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return the temporal destination cell of a temporal directed edge.
 */
Temporal *
th3index_get_directed_edge_destination(const Temporal *edge)
{
  assert(edge); assert(edge->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_get_directed_edge_destination;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(edge, &lfinfo);
}

/*****************************************************************************
 * h3_directed_edge_to_boundary — needs h3_adapter.c body
 *****************************************************************************/

/**
 * @ingroup meos_h3_edges
 * @brief Return the per-instant polygon boundary of a temporal directed
 * edge as a temporal geography.
 *
 * Depends on the `h3_directed_edge_to_gs_boundary` adapter (opt-out list).
 */
Temporal *
th3index_directed_edge_to_boundary(const Temporal *edge)
{
  assert(edge); assert(edge->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_directed_edge_to_boundary;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGRAPHY;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(edge, &lfinfo);
}

/*****************************************************************************/
