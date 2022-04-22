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
 * @file tpoint_posops_meos.c
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
#include "point/postgis.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_boxops.h"

/*****************************************************************************/
/* geo op Temporal */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly to the left of the
 * temporal point
 */
bool
left_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend to the right of the
 * temporal point
 */
bool
overleft_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly to the right of the
 * temporal point
 */
bool
right_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend to the left of the
 * temporal point
 */
bool
overright_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly below the temporal point
 */
bool
below_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend above the temporal point
 */
bool
overbelow_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly above the temporal point
 */
bool
above_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend below the temporal point
 */
bool
overabove_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overabove_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly in front of the temporal point
 */
bool
front_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &front_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend to the back of the
 * temporal point
 */
bool
overfront_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overfront_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry is strictly back of the temporal point
 */
bool
back_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &back_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the geometry does not extend to the front of the
 * temporal point
 */
bool
overback_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overback_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op geom */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly to the left of
 * the geometry
 */
bool
left_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the right of
 * the geometry
 */
bool
overleft_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly to the right of
 * the geometry
 */
bool
right_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the left of
 * the geometry
 */
bool
overright_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly below the geometry
 */
bool
below_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend above the geometry
 */
bool
overbelow_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly above the geometry
 */
bool
above_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend below the geometry
 */
bool
overabove_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overabove_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly in front of the geometry
 */
bool
front_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &front_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the back of
 * the geometry
 */
bool
overfront_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overfront_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly back of the geometry
 */
bool
back_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &back_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the front of
 * the geometry
 */
bool
overback_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overback_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* stbox op Temporal */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly to the left of the
 * temporal point
 */
bool
left_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend to the right of
 * the temporal point
 */
bool
overleft_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly to the right of the
 * temporal point
 */
bool
right_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend to the left of
 * the temporal point
 */
bool
overright_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly below the
 * temporal point
 */
bool
below_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend above the
 * temporal point
 */
bool
overbelow_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly above the temporal
 * point
 */
bool
above_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend below the
 * temporal point
 */
bool
overabove_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overabove_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly in front of the
 * temporal point
 */
bool
front_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &front_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend to the back of
 * the temporal point
 */
bool
overfront_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overfront_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly back of the
 * temporal point
 */
bool
back_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &back_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend to the front of
 * the temporal point
 */
bool
overback_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overback_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly before the temporal
 * point
 */
bool
before_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &before_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend after the
 * temporal point
 */
bool
overbefore_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overbefore_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is strictly after the temporal
 * point
 */
bool
after_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &after_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box does not extend before the
 * temporal point
 */
bool
overafter_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overafter_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op stbox */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly to the left of the
 * spatiotemporal box
 */
bool
left_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the right of the
 * spatiotemporal box
 */
bool
overleft_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly to the right of the
 * spatiotemporal box
 */
bool
right_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the left of the
 * spatiotemporal box
 */
bool
overright_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly below the
 * spatiotemporal box
 */
bool
below_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend above the
 * spatiotemporal box
 */
bool
overbelow_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly above the
 * spatiotemporal box
 */
bool
above_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend below the
 * spatiotemporal box
 */
bool
overabove_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overabove_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly in front of the
 * spatiotemporal box
 */
bool
front_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &front_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the back of the
 * spatiotemporal box
 */
bool
overfront_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overfront_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly back of the
 * spatiotemporal box
 */
bool
back_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &back_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend to the front of the
 * spatiotemporal box
 */
bool
overback_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overback_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly before the
 * spatiotemporal box
 */
bool
before_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &before_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend after the
 * spatiotemporal box
 */
bool
overbefore_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overbefore_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point is strictly after the
 * spatiotemporal box
 */
bool
after_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &after_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal point does not extend before the
 * spatiotemporal box
 */
bool
overafter_tpoint_stbox(const Temporal *tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overafter_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly to the left of
 * the second one
 */
bool
left_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &left_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend to the right
 * of the second one
 */
bool
overleft_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overleft_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly to the right of
 * the second one
 */
bool
right_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &right_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend to the left
 * of the second one
 */
bool
overright_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overright_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly below the
 * second one
 */
bool
below_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &below_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend above the
 * second one
 */
bool
overbelow_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overbelow_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly above the
 * second one
 */
bool
above_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &above_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend below the
 * second one
 */
bool
overabove_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overabove_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly in front of the
 * second one
 */
bool
front_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &front_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend to the back
 * of the second one
 */
bool
overfront_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overfront_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly back of the
 * second one
 */
bool
back_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &back_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend to the front
 * of the second one
 */
bool
overback_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overback_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly before the
 * second one
 */
bool
before_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &before_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend after the
 * second one
 */
bool
overbefore_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overbefore_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point is strictly after the
 * second one
 */
bool
after_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &after_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the first temporal point does not extend before the
 * second one
 */
bool
overafter_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overafter_stbox_stbox);
}

/*****************************************************************************/
