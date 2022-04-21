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
 * @file tpoint_posops.c
 * @brief Relative position operators for temporal geometry points.
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

/* MobilityDB */
#include "point/tpoint_boxops.h"

/*****************************************************************************/
/* geom op Temporal */

PG_FUNCTION_INFO_V1(Left_geom_tpoint);
/**
 * Return true if the geometry is strictly to the left of the temporal point
 */
PGDLLEXPORT Datum
Left_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_geom_tpoint);
/**
 * Return true if the geometry does not extend to the right of the temporal point
 */
PGDLLEXPORT Datum
Overleft_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_geom_tpoint);
/**
 * Return true if the geometry is strictly to the right of the temporal point
 */
PGDLLEXPORT Datum
Right_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_geom_tpoint);
/**
 * Return true if the geometry does not extend to the left of the temporal point
 */
PGDLLEXPORT Datum
Overright_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_geom_tpoint);
/**
 * Return true if the geometry is strictly below the temporal point
 */
PGDLLEXPORT Datum
Below_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_geom_tpoint);
/**
 * Return true if the geometry does not extend above the temporal point
 */
PGDLLEXPORT Datum
Overbelow_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_geom_tpoint);
/**
 * Return true if the geometry is strictly above the temporal point
 */
PGDLLEXPORT Datum
Above_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_geom_tpoint);
/**
 * Return true if the geometry does not extend below the temporal point
 */
PGDLLEXPORT Datum
Overabove_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Front_geom_tpoint);
/**
 * Return true if the geometry is strictly in front of the temporal point
 */
PGDLLEXPORT Datum
Front_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &front_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overfront_geom_tpoint);
/**
 * Return true if the geometry does not extend to the back of the temporal point
 */
PGDLLEXPORT Datum
Overfront_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overfront_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Back_geom_tpoint);
/**
 * Return true if the geometry is strictly back of the temporal point
 */
PGDLLEXPORT Datum
Back_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &back_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overback_geom_tpoint);
/**
 * Return true if the geometry does not extend to the front of the temporal point
 */
PGDLLEXPORT Datum
Overback_geom_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_geo_tpoint_ext(fcinfo, &overback_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op geom */

PG_FUNCTION_INFO_V1(Left_tpoint_geom);
/**
 * Return true if the temporal point is strictly to the left of the geometry
 */
PGDLLEXPORT Datum
Left_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tpoint_geom);
/**
 * Return true if the temporal point does not extend to the right of the geometry
 */
PGDLLEXPORT Datum
Overleft_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tpoint_geom);
/**
 * Return true if the temporal point is strictly to the right of the geometry
 */
PGDLLEXPORT Datum
Right_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tpoint_geom);
/**
 * Return true if the temporal point does not extend to the left of the geometry
 */
PGDLLEXPORT Datum
Overright_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tpoint_geom);
/**
 * Return true if the temporal point is strictly below the geometry
 */
PGDLLEXPORT Datum
Below_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tpoint_geom);
/**
 * Return true if the temporal point does not extend above the geometry
 */
PGDLLEXPORT Datum
Overbelow_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tpoint_geom);
/**
 * Return true if the temporal point is strictly above the geometry
 */
PGDLLEXPORT Datum
Above_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tpoint_geom);
/**
 * Return true if the temporal point does not extend below the geometry
 */
PGDLLEXPORT Datum
Overabove_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Front_tpoint_geom);
/**
 * Return true if the temporal point is strictly in front of the geometry
 */
PGDLLEXPORT Datum
Front_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &front_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overfront_tpoint_geom);
/**
 * Return true if the temporal point does not extend to the back of the geometry
 */
PGDLLEXPORT Datum
Overfront_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overfront_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Back_tpoint_geom);
/**
 * Return true if the temporal point is strictly back of the geometry
 */
PGDLLEXPORT Datum
Back_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &back_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overback_tpoint_geom);
/**
 * Return true if the temporal point does not extend to the front of the geometry
 */
PGDLLEXPORT Datum
Overback_tpoint_geom(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_geo_ext(fcinfo, &overback_stbox_stbox);
}

/*****************************************************************************/
/* stbox op Temporal */

PG_FUNCTION_INFO_V1(Left_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly to the left of the temporal point
 */
PGDLLEXPORT Datum
Left_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend to the right of the temporal point
 */
PGDLLEXPORT Datum
Overleft_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly to the right of the temporal point
 */
PGDLLEXPORT Datum
Right_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend to the left of the temporal point
 */
PGDLLEXPORT Datum
Overright_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly below the temporal point
 */
PGDLLEXPORT Datum
Below_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend above the temporal point
 */
PGDLLEXPORT Datum
Overbelow_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly above the temporal point
 */
PGDLLEXPORT Datum
Above_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend below the temporal point
 */
PGDLLEXPORT Datum
Overabove_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Front_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly in front of the temporal point
 */
PGDLLEXPORT Datum
Front_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &front_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overfront_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend to the back of the temporal point
 */
PGDLLEXPORT Datum
Overfront_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overfront_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Back_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly back of the temporal point
 */
PGDLLEXPORT Datum
Back_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &back_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overback_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend to the front of the temporal point
 */
PGDLLEXPORT Datum
Overback_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overback_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly before the temporal point
 */
PGDLLEXPORT Datum
Before_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend after the temporal point
 */
PGDLLEXPORT Datum
Overbefore_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_stbox_tpoint);
/**
 * Return true if the spatiotemporal box is strictly after the temporal point
 */
PGDLLEXPORT Datum
After_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_stbox_tpoint);
/**
 * Return true if the spatiotemporal box does not extend before the temporal point
 */
PGDLLEXPORT Datum
Overafter_stbox_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_stbox_tpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op stbox */

PG_FUNCTION_INFO_V1(Left_tpoint_stbox);
/**
 * Return true if the temporal point is strictly to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
Left_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tpoint_stbox);
/**
 * Return true if the temporal point does not extend to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overleft_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tpoint_stbox);
/**
 * Return true if the temporal point is strictly to the right of the spatiotemporal box
 */
PGDLLEXPORT Datum
Right_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tpoint_stbox);
/**
 * Return true if the temporal point does not extend to the left of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overright_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tpoint_stbox);
/**
 * Return true if the temporal point is strictly below the spatiotemporal box
 */
PGDLLEXPORT Datum
Below_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tpoint_stbox);
/**
 * Return true if the temporal point does not extend above the spatiotemporal box
 */
PGDLLEXPORT Datum
Overbelow_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tpoint_stbox);
/**
 * Return true if the temporal point is strictly above the spatiotemporal box
 */
PGDLLEXPORT Datum
Above_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tpoint_stbox);
/**
 * Return true if the temporal point does not extend below the spatiotemporal box
 */
PGDLLEXPORT Datum
Overabove_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Front_tpoint_stbox);
/**
 * Return true if the temporal point is strictly in front of the spatiotemporal box
 */
PGDLLEXPORT Datum
Front_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &front_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overfront_tpoint_stbox);
/**
 * Return true if the temporal point does not extend to the back of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overfront_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overfront_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Back_tpoint_stbox);
/**
 * Return true if the temporal point is strictly back of the spatiotemporal box
 */
PGDLLEXPORT Datum
Back_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &back_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overback_tpoint_stbox);
/**
 * Return true if the temporal point does not extend to the front of the spatiotemporal box
 */
PGDLLEXPORT Datum
Overback_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overback_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tpoint_stbox);
/**
 * Return true if the temporal point is strictly before the spatiotemporal box
 */
PGDLLEXPORT Datum
Before_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tpoint_stbox);
/**
 * Return true if the temporal point does not extend after the spatiotemporal box
 */
PGDLLEXPORT Datum
Overbefore_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tpoint_stbox);
/**
 * Return true if the temporal point is strictly after the spatiotemporal box
 */
PGDLLEXPORT Datum
After_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tpoint_stbox);
/**
 * Return true if the temporal point does not extend before the spatiotemporal box
 */
PGDLLEXPORT Datum
Overafter_tpoint_stbox(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_stbox_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(Left_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly to the left of the second one
 */
PGDLLEXPORT Datum
Left_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &left_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overleft_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend to the right of the second one
 */
PGDLLEXPORT Datum
Overleft_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overleft_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Right_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly to the right of the second one
 */
PGDLLEXPORT Datum
Right_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &right_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overright_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend to the left of the second one
 */
PGDLLEXPORT Datum
Overright_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overright_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Below_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly below the second one
 */
PGDLLEXPORT Datum
Below_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &below_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbelow_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend above the second one
 */
PGDLLEXPORT Datum
Overbelow_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overbelow_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Above_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly above the second one
 */
PGDLLEXPORT Datum
Above_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &above_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overabove_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend below the second one
 */
PGDLLEXPORT Datum
Overabove_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overabove_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Front_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly in front of the second one
 */
PGDLLEXPORT Datum
Front_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &front_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overfront_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend to the back of the second one
 */
PGDLLEXPORT Datum
Overfront_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overfront_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Back_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly back of the second one
 */
PGDLLEXPORT Datum
Back_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &back_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overback_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend to the front of the second one
 */
PGDLLEXPORT Datum
Overback_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overback_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Before_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly before the second one
 */
PGDLLEXPORT Datum
Before_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &before_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend after the second one
 */
PGDLLEXPORT Datum
Overbefore_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overbefore_stbox_stbox);
}

PG_FUNCTION_INFO_V1(After_tpoint_tpoint);
/**
 * Return true if the first temporal point is strictly after the second one
 */
PGDLLEXPORT Datum
After_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &after_stbox_stbox);
}

PG_FUNCTION_INFO_V1(Overafter_tpoint_tpoint);
/**
 * Return true if the first temporal point does not extend before the second one
 */
PGDLLEXPORT Datum
Overafter_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return boxop_tpoint_tpoint_ext(fcinfo, &overafter_stbox_stbox);
}

/*****************************************************************************/
