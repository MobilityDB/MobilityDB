/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tnpoint_posops.c
 * @brief Relative position operators for temporal network points.
 *
 * The following operators are defined for the spatial dimension:
 * - left, overleft, right, overright, below, overbelow, above, overabove,
 *   front, overfront, back, overback
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 * The following operators for the temporal dimension:
 * - before, overbefore, after, overafter
 * for both temporal geometry and geography points are "inherited" from the
 * basic temporal types. In this file they are defined when one of the
 * arguments is a stbox.
 */

#include "npoint/tnpoint_posops.h"

/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
#include "point/postgis.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_posops.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_boxops.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic box functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_box
 * @brief Generic box function for a temporal network point and a geometry.
 *
 * @param[in] temp Temporal network point
 * @param[in] gs Geometry
 * @param[in] func Function
 * @param[in] invert True when the geometry is the first argument of the
 * function
 */
int
posop_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  if (gserialized_is_empty(gs))
    return -1;
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  geo_stbox(gs, &box2);
  temporal_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return(result ? 1 : 0);
}

/**
 * Generic box function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_geo_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = posop_tnpoint_geo(temp, gs, func, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * Generic box function for a temporal network point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_geo_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = posop_tnpoint_geo(temp, gs, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_box
 * @brief Generic box function for a temporal network point and an stbox.
 *
 * @param[in] temp Temporal network point
 * @param[in] box Bounding box
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 * @param[in] invert True when the bounding box is the first argument of the
 * function
 */
int
posop_tnpoint_stbox(const Temporal *temp, const STBOX *box,
  bool (*func)(const STBOX *, const STBOX *), bool spatial, bool invert)
{
  if ((spatial && ! MOBDB_FLAGS_GET_X(box->flags)) ||
     (! spatial && ! MOBDB_FLAGS_GET_T(box->flags)))
    return -1;
  ensure_not_geodetic(box->flags);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = invert ? func(box, &box1) : func(&box1, box);
  return result ? 1 : 0;
}

/**
 * Generic box function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 */
static Datum
posop_stbox_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *), bool spatial)
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = posop_tnpoint_stbox(temp, box, func, spatial, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * Generic box function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] spatial True when the function considers the spatial dimension,
 * false when it considers the temporal dimension
 */
static Datum
posop_tnpoint_stbox_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *), bool spatial)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  int result = posop_tnpoint_stbox(temp, box, func, spatial, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_box
 * @brief Generic box function for a temporal network point and a network point.
 *
 * @param[in] temp Temporal network point
 * @param[in] np network point
 * @param[in] func Function
 * @param[in] invert True when the network point is the first argument of the
 * function
 */
bool
posop_tnpoint_npoint(const Temporal *temp, const Npoint *np,
  bool (*func)(const STBOX *, const STBOX *), bool invert)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  STBOX box1, box2;
  /* Return an error if the geometry is not found, is null, or is empty */
  npoint_stbox(np, &box2);
  temporal_bbox(temp, &box1);
  bool result = invert ? func(&box2, &box1) : func(&box1, &box2);
  return result;
}

/**
 * Generic box function for a network point and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool result = posop_tnpoint_npoint(temp, np, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for a temporal network point and a network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool result = posop_tnpoint_npoint(temp, np, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_oper_box
 * @brief Generic box function for two temporal network points
 *
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] func Function
 */
bool
posop_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  bool (*func)(const STBOX *, const STBOX *))
{
  ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2));
  STBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  return result;
}

/**
 * Generic box function for two temporal network points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool result = posop_tnpoint_tnpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* geo op Temporal */

PG_FUNCTION_INFO_V1(Left_geom_tnpoint);
/**
 * Return true if the geometry is strictly to the left of the temporal network point
 */
PGDLLEXPORT Datum
Left_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_geom_tnpoint);
/**
 * Return true if the geometry does not extend to the right of the temporal network point
 */
PGDLLEXPORT Datum
Overleft_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_geom_tnpoint);
/**
 * Return true if the geometry is strictly to the right of the temporal network point
 */
PGDLLEXPORT Datum
Right_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_geom_tnpoint);
/**
 * Return true if the geometry does not extend to the left of the temporal network point
 */
PGDLLEXPORT Datum
Overright_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_geom_tnpoint);
/**
 * Return true if the geometry is strictly below the temporal network point
 */
PGDLLEXPORT Datum
Below_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_geom_tnpoint);
/**
 * Return true if the geometry does not extend above the temporal network point
 */
PGDLLEXPORT Datum
Overbelow_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_geom_tnpoint);
/**
 * Return true if the geometry is strictly above the temporal network point
 */
PGDLLEXPORT Datum
Above_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_geom_tnpoint);
/**
 * Return true if the geometry does not extend below the temporal network point
 */
PGDLLEXPORT Datum
Overabove_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geo_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op geo */

PG_FUNCTION_INFO_V1(Left_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly to the left of the
 * geometry
 */
PGDLLEXPORT Datum
Left_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend to the right of
 * the geometry
 */
PGDLLEXPORT Datum
Overleft_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly to the right of the
 * geometry
 */
PGDLLEXPORT Datum
Right_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend to the left of
 * the geometry
 */
PGDLLEXPORT Datum
Overright_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly below the geometry
 */
PGDLLEXPORT Datum
Below_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend above the geometry
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly above the geometry
 */
PGDLLEXPORT Datum
Above_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend below the geometry
 */
PGDLLEXPORT Datum
Overabove_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geo_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(Left_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly to the left of the
 * temporal network point
 */
PGDLLEXPORT Datum
Left_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &left_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overleft_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend to the right of the
 * temporal network point
 */
PGDLLEXPORT Datum
Overleft_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overleft_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Right_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly to the right of the
 * temporal network point
 */
PGDLLEXPORT Datum
Right_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &right_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overright_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend to the left of the
 * temporal network point
 */
PGDLLEXPORT Datum
Overright_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overright_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Below_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly below the temporal
 * network point
 */
PGDLLEXPORT Datum
Below_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &below_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overbelow_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend above the temporal
 * network point
 */
PGDLLEXPORT Datum
Overbelow_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overbelow_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Above_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly above the temporal
 * network point
 */
PGDLLEXPORT Datum
Above_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &above_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overabove_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend below the temporal
 * network point
 */
PGDLLEXPORT Datum
Overabove_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overabove_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Before_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly before of the temporal
 * network point
 */
PGDLLEXPORT Datum
Before_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &before_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overbefore_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend after the temporal
 * network point
 */
PGDLLEXPORT Datum
Overbefore_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overbefore_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(After_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly after the temporal
 * network point
 */
PGDLLEXPORT Datum
After_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &after_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overafter_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend before the temporal
 * network point
 */
PGDLLEXPORT Datum
Overafter_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint_ext(fcinfo, &overafter_stbox_stbox, false);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(Left_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly to the left of the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Left_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &left_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend to the right of
 * the spatiotemporal box
 */
PGDLLEXPORT Datum
Overleft_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overleft_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly to the right of the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Right_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &right_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend to the left of
 * the spatiotemporal box
 */
PGDLLEXPORT Datum
Overright_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overright_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly below the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Below_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &below_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend above the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overbelow_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly above the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Above_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &above_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend below the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Overabove_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overabove_stbox_stbox, true);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly before the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Before_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &before_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend after the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overbefore_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(After_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly after the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
After_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &after_stbox_stbox, false);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend before the
 * spatiotemporal box
 */
PGDLLEXPORT Datum
Overafter_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox_ext(fcinfo, &overafter_stbox_stbox, false);
}

/*****************************************************************************/
/* Npoint op Temporal */

PG_FUNCTION_INFO_V1(Left_npoint_tnpoint);
/**
 * Return true if the network point is strictly to the left of the
 * temporal point
 */
PGDLLEXPORT Datum
Left_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_npoint_tnpoint);
/**
 * Return true if the network point does not extend to the right of the
 * temporal point
 */
PGDLLEXPORT Datum
Overleft_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_npoint_tnpoint);
/**
 * Return true if the network point is strictly to the right of the
 * temporal point
 */
PGDLLEXPORT Datum
Right_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_npoint_tnpoint);
/**
 * Return true if the network point does not extend to the left of the
 * temporal point
 */
PGDLLEXPORT Datum
Overright_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_npoint_tnpoint);
/**
 * Return true if the network point is strictly below the temporal point
 */
PGDLLEXPORT Datum
Below_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_npoint_tnpoint);
/**
 * Return true if the network point does not extend above the temporal point
 */
PGDLLEXPORT Datum
Overbelow_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_npoint_tnpoint);
/**
 * Return true if the network point is strictly above the temporal point
 */
PGDLLEXPORT Datum
Above_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_npoint_tnpoint);
/**
 * Return true if the network point does not extend below the temporal point
 */
PGDLLEXPORT Datum
Overabove_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Npoint */

PG_FUNCTION_INFO_V1(Left_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly to the left of the
 * network point
 */
PGDLLEXPORT Datum
Left_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend to the right of the
 * network point
 */
PGDLLEXPORT Datum
Overleft_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly to the right of the
 * network point
 */
PGDLLEXPORT Datum
Right_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend to the left of the
 * network point
 */
PGDLLEXPORT Datum
Overright_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly below the network point
 */
PGDLLEXPORT Datum
Below_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend above the network point
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly above the network point
 */
PGDLLEXPORT Datum
Above_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend below the network point
 */
PGDLLEXPORT Datum
Overabove_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint_ext(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(Left_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly to the left of
 * the second one
 */
PGDLLEXPORT Datum
Left_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend to the right
 * of the second one
 */
PGDLLEXPORT Datum
Overleft_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly to the right of
 * the second one
 */
PGDLLEXPORT Datum
Right_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend to the left
 * of the second one
 */
PGDLLEXPORT Datum
Overright_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly below the
 * second one
 */
PGDLLEXPORT Datum
Below_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend above the
 * second one
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly above the
 * second one
 */
PGDLLEXPORT Datum
Above_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend below the
 * second one
 */
PGDLLEXPORT Datum
Overabove_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly before the
 * second one
 */
PGDLLEXPORT Datum
Before_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend after the
 * second one
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly after the
 * second one
 */
PGDLLEXPORT Datum
After_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend before the
 * second one
 */
PGDLLEXPORT Datum
Overafter_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
