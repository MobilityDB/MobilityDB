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
 * Generic box function for a geometry and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_geom_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  geo_stbox(gs, &box1);
  temporal_bbox(temp, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for a temporal network point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_geom(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  ensure_has_not_Z_gs(gs);
  STBOX box1, box2;
  geo_stbox(gs, &box2);
  temporal_bbox(temp, &box1);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_stbox_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  ensure_not_geodetic(box->flags);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Generic temporal box function for an stbox and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
tposop_stbox_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    temporal_bbox(temp, &box1);
    result = func(box, &box1);
  }
  PG_FREE_IF_COPY(temp, 1);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  STBOX *box = PG_GETARG_STBOX_P(1);
  if (! MOBDB_FLAGS_GET_X(box->flags))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  ensure_not_geodetic(box->flags);
  ensure_same_srid_tnpoint_stbox(temp, box);
  STBOX box1;
  temporal_bbox(temp, &box1);
  bool result = func(&box1, box);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic temporal box function for a temporal network point and an stbox
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
tposop_tnpoint_stbox(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  bool hast = MOBDB_FLAGS_GET_T(box->flags);
  bool result = false;
  if (hast)
  {
    STBOX box1;
    temporal_bbox(temp, &box1);
    result = func(&box1, box);
  }
  PG_FREE_IF_COPY(temp, 0);
  if (!hast)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for a network point and a temporal network point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_npoint_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  STBOX box1, box2;
  /* Return an error if the geometry is not found, is null, or is empty */
  npoint_stbox(np, &box1);
  temporal_bbox(temp, &box2);
  bool result = func(&box1, &box2);
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
posop_tnpoint_npoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  STBOX box1, box2;
  /* Return an error if the geometry is not found, is null, or is empty */
  npoint_stbox(np, &box2);
  temporal_bbox(temp, &box1);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Generic box function for two temporal network points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
static Datum
posop_tnpoint_tnpoint(FunctionCallInfo fcinfo,
  bool (*func)(const STBOX *, const STBOX *))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  ensure_same_srid(tnpoint_srid(temp1), tnpoint_srid(temp2));
  STBOX box1, box2;
  temporal_bbox(temp1, &box1);
  temporal_bbox(temp2, &box2);
  bool result = func(&box1, &box2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(Left_geom_tnpoint);
/**
 * Return true if the geometry is strictly to the left of the temporal network point
 */
PGDLLEXPORT Datum
Left_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_geom_tnpoint);
/**
 * Return true if the geometry does not extend to the right of the temporal network point
 */
PGDLLEXPORT Datum
Overleft_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_geom_tnpoint);
/**
 * Return true if the geometry is strictly to the right of the temporal network point
 */
PGDLLEXPORT Datum
Right_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_geom_tnpoint);
/**
 * Return true if the geometry does not extend to the left of the temporal network point
 */
PGDLLEXPORT Datum
Overright_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_geom_tnpoint);
/**
 * Return true if the geometry is strictly below the temporal network point
 */
PGDLLEXPORT Datum
Below_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_geom_tnpoint);
/**
 * Return true if the geometry does not extend above the temporal network point
 */
PGDLLEXPORT Datum
Overbelow_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_geom_tnpoint);
/**
 * Return true if the geometry is strictly above the temporal network point
 */
PGDLLEXPORT Datum
Above_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_geom_tnpoint);
/**
 * Return true if the geometry does not extend below the temporal network point
 */
PGDLLEXPORT Datum
Overabove_geom_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_geom_tnpoint(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(Left_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly to the left of the geometry
 */
PGDLLEXPORT Datum
Left_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend to the right of the geometry
 */
PGDLLEXPORT Datum
Overleft_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly to the right of the geometry
 */
PGDLLEXPORT Datum
Right_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend to the left of the geometry
 */
PGDLLEXPORT Datum
Overright_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly below the geometry
 */
PGDLLEXPORT Datum
Below_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend above the geometry
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_geom);
/**
 * Return true if the temporal network point is strictly above the geometry
 */
PGDLLEXPORT Datum
Above_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_geom);
/**
 * Return true if the temporal network point does not extend below the geometry
 */
PGDLLEXPORT Datum
Overabove_tnpoint_geom(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_geom(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(Left_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly to the left of the temporal network point
 */
PGDLLEXPORT Datum
Left_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend to the right of the temporal network point
 */
PGDLLEXPORT Datum
Overleft_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly to the right of the temporal network point
 */
PGDLLEXPORT Datum
Right_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend to the left of the temporal network point
 */
PGDLLEXPORT Datum
Overright_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly below the temporal network point
 */
PGDLLEXPORT Datum
Below_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend above the temporal network point
 */
PGDLLEXPORT Datum
Overbelow_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly above the temporal network point
 */
PGDLLEXPORT Datum
Above_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend below the temporal network point
 */
PGDLLEXPORT Datum
Overabove_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_stbox_tnpoint(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly before of the temporal network point
 */
PGDLLEXPORT Datum
Before_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return tposop_stbox_tnpoint(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend after the temporal network point
 */
PGDLLEXPORT Datum
Overbefore_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return tposop_stbox_tnpoint(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box is strictly after the temporal network point
 */
PGDLLEXPORT Datum
After_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return tposop_stbox_tnpoint(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_stbox_tnpoint);
/**
 * Return true if the spatiotemporal box does not extend before the temporal network point
 */
PGDLLEXPORT Datum
Overafter_stbox_tnpoint(PG_FUNCTION_ARGS)
{
  return tposop_stbox_tnpoint(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(Left_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
Left_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overleft_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
Right_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overright_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly below the spatiotemporal box
 */
PGDLLEXPORT Datum
Below_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend above the spatiotemporal box
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly above the spatiotemporal box
 */
PGDLLEXPORT Datum
Above_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend below the spatiotemporal box
 */
PGDLLEXPORT Datum
Overabove_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_stbox(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly before the spatiotemporal box
 */
PGDLLEXPORT Datum
Before_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return tposop_tnpoint_stbox(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend after the spatiotemporal box
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return tposop_tnpoint_stbox(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tnpoint_stbox);
/**
 * Return true if the temporal network point is strictly after the spatiotemporal box
 */
PGDLLEXPORT Datum
After_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return tposop_tnpoint_stbox(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_stbox);
/**
 * Return true if the temporal network point does not extend before the spatiotemporal box
 */
PGDLLEXPORT Datum
Overafter_tnpoint_stbox(PG_FUNCTION_ARGS)
{
  return tposop_tnpoint_stbox(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* npoint op Temporal */

PG_FUNCTION_INFO_V1(Left_npoint_tnpoint);
/**
 * Return true if the network point is strictly to the left of the temporal point
 */
PGDLLEXPORT Datum
Left_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_npoint_tnpoint);
/**
 * Return true if the network point does not extend to the right of the temporal point
 */
PGDLLEXPORT Datum
Overleft_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_npoint_tnpoint);
/**
 * Return true if the network point is strictly to the right of the temporal point
 */
PGDLLEXPORT Datum
Right_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_npoint_tnpoint);
/**
 * Return true if the network point does not extend to the left of the temporal point
 */
PGDLLEXPORT Datum
Overright_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_npoint_tnpoint);
/**
 * Return true if the network point is strictly below the temporal point
 */
PGDLLEXPORT Datum
Below_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_npoint_tnpoint);
/**
 * Return true if the network point does not extend above the temporal point
 */
PGDLLEXPORT Datum
Overbelow_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_npoint_tnpoint);
/**
 * Return true if the network point is strictly above the temporal point
 */
PGDLLEXPORT Datum
Above_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_npoint_tnpoint);
/**
 * Return true if the network point does not extend below the temporal point
 */
PGDLLEXPORT Datum
Overabove_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_npoint_tnpoint(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op npoint */

PG_FUNCTION_INFO_V1(Left_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly to the left of the network point
 */
PGDLLEXPORT Datum
Left_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend to the right of the network point
 */
PGDLLEXPORT Datum
Overleft_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly to the right of the network point
 */
PGDLLEXPORT Datum
Right_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend to the left of the network point
 */
PGDLLEXPORT Datum
Overright_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly below the network point
 */
PGDLLEXPORT Datum
Below_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend above the network point
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_npoint);
/**
 * Return true if the temporal point is strictly above the network point
 */
PGDLLEXPORT Datum
Above_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_npoint);
/**
 * Return true if the temporal point does not extend below the network point
 */
PGDLLEXPORT Datum
Overabove_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_npoint(fcinfo, &overabove_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(Left_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend to the right of the second one
 */
PGDLLEXPORT Datum
Overleft_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly to the right of the second one
 */
PGDLLEXPORT Datum
Right_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend to the left of the second one
 */
PGDLLEXPORT Datum
Overright_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly below the second one
 */
PGDLLEXPORT Datum
Below_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend above the second one
 */
PGDLLEXPORT Datum
Overbelow_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly above the second one
 */
PGDLLEXPORT Datum
Above_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend below the second one
 */
PGDLLEXPORT Datum
Overabove_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly before the second one
 */
PGDLLEXPORT Datum
Before_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend after the second one
 */
PGDLLEXPORT Datum
Overbefore_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point is strictly after the second one
 */
PGDLLEXPORT Datum
After_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnpoint_tnpoint);
/**
 * Return true if the first temporal network point does not extend before the second one
 */
PGDLLEXPORT Datum
Overafter_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return posop_tnpoint_tnpoint(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
