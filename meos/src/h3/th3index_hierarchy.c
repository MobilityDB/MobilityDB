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
 * @brief MEOS lifting for hierarchy functions, plus the two
 * next-resolution conveniences that back them.
 *
 * The `cell_to_child_pos` / `child_pos_to_cell` pair and the
 * required-resolution `cell_to_parent_meos` /
 * `cell_to_center_child_meos` forms are auto-generated from h3-pg
 * (see `h3_generated.h`). The hand-written
 * `h3_cell_to_parent_next_meos` / `_center_child_next_meos`
 * helpers below pick the adjacent resolution and delegate.
 */

#include <string.h>

#include <meos.h>
#include <meos_h3.h>
#include <h3api.h>

#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/h3_generated.h"
#include "h3/th3index_internal.h"

/*****************************************************************************
 * Next-resolution conveniences (hand-written)
 *****************************************************************************/

H3Index
h3_cell_to_parent_next_meos(H3Index cell)
{
  int32_t res = getResolution(cell);
  if (res <= 0)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "h3 cell is already at the coarsest resolution (0)");
    return (H3Index) 0;
  }
  return h3_cell_to_parent_meos(cell, res - 1);
}

H3Index
h3_cell_to_center_child_next_meos(H3Index cell)
{
  int32_t res = getResolution(cell);
  if (res >= 15)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "h3 cell is already at the finest resolution (15)");
    return (H3Index) 0;
  }
  return h3_cell_to_center_child_meos(cell, res + 1);
}

/*****************************************************************************
 * h3_cell_to_parent(th3index, integer)
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal parent cell at the given resolution.
 */
Temporal *
th3index_cell_to_parent(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_parent;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(resolution);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_parent(th3index)
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal parent cell at the next-coarser resolution.
 */
Temporal *
th3index_cell_to_parent_next(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_parent_next;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_center_child(th3index, integer)
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal center-child cell at the given resolution.
 */
Temporal *
th3index_cell_to_center_child(const Temporal *temp, int32 resolution)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_center_child;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(resolution);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_center_child(th3index)
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal center-child cell at the next-finer resolution.
 */
Temporal *
th3index_cell_to_center_child_next(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_center_child_next;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_cell_to_child_pos(th3index, integer)
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal position of a child cell among its parent's
 * children at the given parent resolution.
 */
Temporal *
th3index_cell_to_child_pos(const Temporal *temp, int32 parent_res)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_child_pos;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(parent_res);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBIGINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_child_pos_to_cell(tbigint, th3index, integer)
 *
 * Two temporal operands are synchronised over their shared time axis;
 * the integer child resolution is constant.
 *****************************************************************************/

/**
 * @ingroup meos_h3_hierarchy
 * @brief Return the temporal child cell of a parent at a given ordinal
 * position among siblings, at a given child resolution.
 */
Temporal *
th3index_child_pos_to_cell(const Temporal *child_pos, const Temporal *parent,
  int32 child_res)
{
  assert(child_pos); assert(parent);
  assert(child_pos->temptype == T_TBIGINT);
  assert(parent->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_child_pos_to_cell;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(child_res);
  lfinfo.argtype[0] = T_TBIGINT;
  lfinfo.argtype[1] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal_temporal(child_pos, parent, &lfinfo);
}

/*****************************************************************************/
