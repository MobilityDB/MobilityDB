/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * @file tpoint_posops.c
 * Relative position operators for temporal geometry points.
 *
 * The following operators are defined for the spatial dimension:
 * `left`, `overleft`, `right`, `overright`, `below`, `overbelow`, `above`,
 * `overabove`, `front`, `overfront`, `back`, `overback`. 
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 *
 * The following operators are defined for for the time dimension: `before`, 
 * `overbefore`,  `after`, `overafter`. For both temporal geometry and 
 * geography points the same operators are derived from the basic temporal 
 * types. In this file they are defined when one of the arguments is an 
 * `STBOX`.
 */
 
#include "point/tpoint_posops.h"

#include <assert.h>

#include "point/postgis.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_boxops.h"

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(left_geom_tpoint);
/**
 * Returns true if the geometry is strictly to the left of the temporal point
 */
PGDLLEXPORT Datum
left_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_geom_tpoint);
/**
 * Returns true if the geometry does not extend to the right of the temporal point
 */
PGDLLEXPORT Datum
overleft_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_geom_tpoint);
/**
 * Returns true if the geometry is strictly to the right of the temporal point
 */
PGDLLEXPORT Datum
right_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_geom_tpoint);
/**
 * Returns true if the geometry does not extend to the left of the temporal point
 */
PGDLLEXPORT Datum
overright_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_geom_tpoint);
/**
 * Returns true if the geometry is strictly below the temporal point
 */
PGDLLEXPORT Datum
below_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_geom_tpoint);
/**
 * Returns true if the geometry does not extend above the temporal point
 */
PGDLLEXPORT Datum
overbelow_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_geom_tpoint);
/**
 * Returns true if the geometry is strictly above the temporal point
 */
PGDLLEXPORT Datum
above_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_geom_tpoint);
/**
 * Returns true if the geometry does not extend below the temporal point
 */
PGDLLEXPORT Datum
overabove_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_geom_tpoint);
/**
 * Returns true if the geometry is strictly in front of the temporal point
 */
PGDLLEXPORT Datum
front_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_geom_tpoint);
/**
 * Returns true if the geometry does not extend to the back of the temporal point
 */
PGDLLEXPORT Datum
overfront_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_geom_tpoint);
/**
 * Returns true if the geometry is strictly back of the temporal point
 */
PGDLLEXPORT Datum
back_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_geom_tpoint);
/**
 * Returns true if the geometry does not extend to the front of the temporal point
 */
PGDLLEXPORT Datum
overback_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint(fcinfo, &overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(left_tpoint_geom);
/**
 * Returns true if the temporal point is strictly to the left of the geometry
 */
PGDLLEXPORT Datum
left_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_geom);
/**
 * Returns true if the temporal point does not extend to the right of the geometry
 */
PGDLLEXPORT Datum
overleft_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_geom);
/**
 * Returns true if the temporal point is strictly to the right of the geometry
 */
PGDLLEXPORT Datum
right_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_geom);
/**
 * Returns true if the temporal point does not extend to the left of the geometry
 */
PGDLLEXPORT Datum
overright_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_geom);
/**
 * Returns true if the temporal point is strictly below the geometry
 */
PGDLLEXPORT Datum
below_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_geom);
/**
 * Returns true if the temporal point does not extend above the geometry
 */
PGDLLEXPORT Datum
overbelow_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_geom);
/**
 * Returns true if the temporal point is strictly above the geometry
 */
PGDLLEXPORT Datum
above_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_geom);
/**
 * Returns true if the temporal point does not extend below the geometry
 */
PGDLLEXPORT Datum
overabove_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_geom);
/**
 * Returns true if the temporal point is strictly in front of the geometry
 */
PGDLLEXPORT Datum
front_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_geom);
/**
 * Returns true if the temporal point does not extend to the back of the geometry
 */
PGDLLEXPORT Datum
overfront_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_geom);
/**
 * Returns true if the temporal point is strictly back of the geometry
 */
PGDLLEXPORT Datum
back_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_geom);
/**
 * Returns true if the temporal point does not extend to the front of the geometry
 */
PGDLLEXPORT Datum
overback_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo(fcinfo, &overback_stbox_stbox_internal);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(left_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly to the left of the temporal point
 */
PGDLLEXPORT Datum
left_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend to the right of the temporal point
 */
PGDLLEXPORT Datum
overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly to the right of the temporal point
 */
PGDLLEXPORT Datum
right_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend to the left of the temporal point
 */
PGDLLEXPORT Datum
overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly below the temporal point
 */
PGDLLEXPORT Datum
below_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend above the temporal point
 */
PGDLLEXPORT Datum
overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly above the temporal point
 */
PGDLLEXPORT Datum
above_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend below the temporal point
 */
PGDLLEXPORT Datum
overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly in front of the temporal point
 */
PGDLLEXPORT Datum
front_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend to the back of the temporal point
 */
PGDLLEXPORT Datum
overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly back of the temporal point
 */
PGDLLEXPORT Datum
back_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend to the front of the temporal point
 */
PGDLLEXPORT Datum
overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly before the temporal point
 */
PGDLLEXPORT Datum
before_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend after the temporal point
 */
PGDLLEXPORT Datum
overbefore_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is strictly after the temporal point
 */
PGDLLEXPORT Datum
after_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box does not extend before the temporal point
 */
PGDLLEXPORT Datum
overafter_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(left_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
left_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
right_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly below the spatiotemporal box
 */
PGDLLEXPORT Datum
below_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend above the spatiotemporal box
 */
PGDLLEXPORT Datum
overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly above the spatiotemporal box
 */
PGDLLEXPORT Datum
above_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend below the spatiotemporal box
 */
PGDLLEXPORT Datum
overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly in front of the spatiotemporal box
 */
PGDLLEXPORT Datum
front_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend to the back of the spatiotemporal box
 */
PGDLLEXPORT Datum
overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly back of the spatiotemporal box
 */
PGDLLEXPORT Datum
back_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend to the front of the spatiotemporal box
 */
PGDLLEXPORT Datum
overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly before the spatiotemporal box
 */
PGDLLEXPORT Datum
before_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend after the spatiotemporal box
 */
PGDLLEXPORT Datum
overbefore_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_tpoint_stbox);
/**
 * Returns true if the temporal point is strictly after the spatiotemporal box
 */
PGDLLEXPORT Datum
after_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_stbox);
/**
 * Returns true if the temporal point does not extend before the spatiotemporal box
 */
PGDLLEXPORT Datum
overafter_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(left_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly to the left of the second one
 */
PGDLLEXPORT Datum
left_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &left_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend to the right of the second one
 */
PGDLLEXPORT Datum
overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overleft_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(right_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly to the right of the second one
 */
PGDLLEXPORT Datum
right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &right_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend to the left of the second one
 */
PGDLLEXPORT Datum
overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overright_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(below_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly below the second one
 */
PGDLLEXPORT Datum
below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &below_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbelow_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend above the second one
 */
PGDLLEXPORT Datum
overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overbelow_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(above_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly above the second one
 */
PGDLLEXPORT Datum
above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &above_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overabove_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend below the second one
 */
PGDLLEXPORT Datum
overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overabove_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(front_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly in front of the second one
 */
PGDLLEXPORT Datum
front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &front_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overfront_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend to the back of the second one
 */
PGDLLEXPORT Datum
overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overfront_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(back_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly back of the second one
 */
PGDLLEXPORT Datum
back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &back_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overback_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend to the front of the second one
 */
PGDLLEXPORT Datum
overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overback_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(before_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly before the second one
 */
PGDLLEXPORT Datum
before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &before_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend after the second one
 */
PGDLLEXPORT Datum
overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overbefore_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(after_tpoint_tpoint);
/**
 * Returns true if the first temporal point is strictly after the second one
 */
PGDLLEXPORT Datum
after_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &after_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tpoint_tpoint);
/**
 * Returns true if the first temporal point does not extend before the second one
 */
PGDLLEXPORT Datum
overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint(fcinfo, &overafter_stbox_stbox_internal);
}

/*****************************************************************************/
