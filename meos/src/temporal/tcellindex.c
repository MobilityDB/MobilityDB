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
 * @brief Shared temporal lifting for the DGGS cell-index family.
 *
 * The generic entry points dispatch on the temporal type via
 * `dggs_cellops()` and lift the descriptor's Datum-convention static kernel
 * with `tfunc_temporal`. Adding a DGGS requires only a new descriptor and one
 * line in `dggs_cellops()` — see meos/include/temporal/tcellindex.h.
 */

#include "temporal/tcellindex.h"

/* C */
#include <string.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include "temporal/lifting.h"

/* Per-DGGS descriptors, defined in each family and referenced here under the
 * same build-flag guard that compiles the family. */
#if QUADBIN
extern const DggsCellOps quadbin_cellops;
#endif

/*****************************************************************************
 * Catalog predicate + descriptor registry
 *****************************************************************************/

/**
 * @brief Return true if @p type is a temporal DGGS cell-index type.
 */
bool
tcellindex_type(MeosType type)
{
  return
#if QUADBIN
    type == T_TQUADBIN ||
#endif
    false;
}

/**
 * @brief Return the operations descriptor for a temporal cell-index type.
 */
const DggsCellOps *
dggs_cellops(MeosType temptype)
{
  switch (temptype)
  {
#if QUADBIN
    case T_TQUADBIN:
      return &quadbin_cellops;
#endif
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE,
        "Type %d is not a temporal DGGS cell-index type", temptype);
      return NULL;
  }
}

/*****************************************************************************
 * Generic lifting helpers
 *****************************************************************************/

/**
 * @brief Lift a unary Datum-convention cell function over a temporal value.
 */
static Temporal *
tcellindex_lift_unary(const Temporal *temp, Datum (*func)(Datum),
  const char *opname, MeosType restype)
{
  if (! func)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation \"%s\" is not available for this DGGS cell type", opname);
    return NULL;
  }
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = restype;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/**
 * @brief Lift a one-parameter Datum-convention cell function over a temporal
 * value.
 */
static Temporal *
tcellindex_lift_param1(const Temporal *temp, Datum (*func)(Datum, Datum),
  const char *opname, Datum param, MeosType restype)
{
  if (! func)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation \"%s\" is not available for this DGGS cell type", opname);
    return NULL;
  }
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 1;
  lfinfo.param[0] = param;
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.restype = restype;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * Generic temporal entry points
 *****************************************************************************/

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal resolution (tint) of a temporal cell index.
 */
Temporal *
tcellindex_get_resolution(const Temporal *temp)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  return tcellindex_lift_unary(temp, ops->get_resolution, "getResolution",
    T_TINT);
}

/**
 * @ingroup meos_cellindex
 * @brief Return a tbool stating at each instant whether the value is a valid
 * cell.
 */
Temporal *
tcellindex_is_valid_cell(const Temporal *temp)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  return tcellindex_lift_unary(temp, ops->is_valid_cell, "isValidCell",
    T_TBOOL);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal parent cell at the given resolution.
 */
Temporal *
tcellindex_cell_to_parent(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  return tcellindex_lift_param1(temp, ops->cell_to_parent, "cellToParent",
    Int32GetDatum(resolution), temp->temptype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell centroid as a temporal point (geodetic for
 * H3/S2, Web-Mercator for quadbin).
 */
Temporal *
tcellindex_cell_to_point(const Temporal *temp)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  return tcellindex_lift_unary(temp, ops->cell_to_point, "cellToPoint",
    ops->point_temptype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell boundary as a temporal (multi)polygon.
 */
Temporal *
tcellindex_cell_to_boundary(const Temporal *temp)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  MeosType restype = (ops->point_temptype == T_TGEOGPOINT) ?
    T_TGEOGRAPHY : T_TGEOMETRY;
  return tcellindex_lift_unary(temp, ops->cell_to_boundary, "cellToBoundary",
    restype);
}

/**
 * @ingroup meos_cellindex
 * @brief Return the temporal cell area in square meters (tfloat).
 */
Temporal *
tcellindex_cell_area(const Temporal *temp)
{
  assert(temp); assert(tcellindex_type(temp->temptype));
  const DggsCellOps *ops = dggs_cellops(temp->temptype);
  return tcellindex_lift_unary(temp, ops->cell_area, "cellArea", T_TFLOAT);
}

/*****************************************************************************/
