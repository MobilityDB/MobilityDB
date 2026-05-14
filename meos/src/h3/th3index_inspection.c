/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief MEOS lifting for inspection functions.
 *
 * Every entry in this file has a ready static helper in
 * `meos/src/h3/h3_generated.c`; the lifting is a straight call to
 * `tfunc_temporal` with a `LiftedFunctionInfo` populated for a
 * `unary_scalar` shape.
 */

#include <string.h>

#include <meos.h>
#include <meos_h3.h>

#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * h3_get_resolution
 *****************************************************************************/

/**
 * @ingroup meos_h3_inspection
 * @brief Return the temporal resolution (int32) of a temporal H3 cell.
 */
Temporal *
th3index_get_resolution(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_get_resolution;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_get_base_cell_number
 *****************************************************************************/

/**
 * @ingroup meos_h3_inspection
 * @brief Return the temporal base-cell number (int32) of a temporal H3 cell.
 */
Temporal *
th3index_get_base_cell_number(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_get_base_cell_number;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_is_valid_cell
 *****************************************************************************/

/**
 * @ingroup meos_h3_inspection
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 cell.
 */
Temporal *
th3index_is_valid_cell(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_is_valid_cell;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_is_res_class_iii
 *****************************************************************************/

/**
 * @ingroup meos_h3_inspection
 * @brief Return a temporal boolean stating whether the cell has Class-III
 * orientation at each instant.
 */
Temporal *
th3index_is_res_class_iii(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_is_res_class_iii;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_is_pentagon
 *****************************************************************************/

/**
 * @ingroup meos_h3_inspection
 * @brief Return a temporal boolean stating whether the cell is a pentagon
 * at each instant.
 */
Temporal *
th3index_is_pentagon(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_is_pentagon;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
