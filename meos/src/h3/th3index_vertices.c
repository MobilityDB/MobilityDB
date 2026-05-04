/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief MEOS lifting for vertex functions, plus the static
 * `h3_vertex_to_gs_point` adapter that backs the
 * `th3index_vertex_to_latlng` entry.
 *
 * `cell_to_vertex` and `is_valid_vertex` lift directly on the
 * `_meos` helpers in `h3_generated.c`.
 */

#include <string.h>

#include <meos.h>
#include <meos_h3.h>
#include <h3api.h>

#include "geo/tgeo_spatialfuncs.h"
#include "temporal/temporal.h"
#include "temporal/meos_catalog.h"
#include "temporal/lifting.h"

#include "h3/th3index_internal.h"

/*****************************************************************************
 * Static adapter — vertex → geodetic point
 *****************************************************************************/

GSERIALIZED *
h3_vertex_to_gs_point(H3Index vertex)
{
  LatLng ll;
  if (vertexToLatLng(vertex, &ll) != E_SUCCESS)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR, "h3 library error");
    return NULL;
  }
  return geopoint_make(radsToDegs(ll.lng), radsToDegs(ll.lat), 0.0,
    false, true, SRID_DEFAULT);
}

/*****************************************************************************
 * h3_cell_to_vertex(th3index, integer) — lift_with_const
 *****************************************************************************/

/**
 * @ingroup meos_h3_vertex
 * @brief Return the temporal H3 vertex at the given vertex number of a
 * temporal H3 cell.
 */
Temporal *
th3index_cell_to_vertex(const Temporal *temp, int32 vertex_num)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_cell_to_vertex;
  lfinfo.numparam = 1;
  lfinfo.param[0] = Int32GetDatum(vertex_num);
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TH3INDEX;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_vertex_to_latlng — needs h3_adapter.c body
 *****************************************************************************/

/**
 * @ingroup meos_h3_vertex
 * @brief Return the temporal geodetic point at each instant of a temporal
 * H3 vertex.
 */
Temporal *
th3index_vertex_to_latlng(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_vertex_to_latlng;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TGEOGPOINT;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************
 * h3_is_valid_vertex
 *****************************************************************************/

/**
 * @ingroup meos_h3_vertex
 * @brief Return a temporal boolean stating at each instant whether the
 * value is a valid H3 vertex.
 */
Temporal *
th3index_is_valid_vertex(const Temporal *temp)
{
  assert(temp); assert(temp->temptype == T_TH3INDEX);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_h3_is_valid_vertex;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = T_TH3INDEX;
  lfinfo.restype = T_TBOOL;
  lfinfo.reslinear = false;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  return tfunc_temporal(temp, &lfinfo);
}

/*****************************************************************************/
