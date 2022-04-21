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
 * @file tnpoint_posops_meos.c
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

/* MobilityDB */
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************/
/* geom op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry is strictly to the left of the temporal
 * network point
 */
bool
left_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry does not extend to the right of the
 * temporal network point
 */
bool
overleft_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry is strictly to the right of the temporal
 * network point
 */
bool
right_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry does not extend to the left of the
 * temporal network point
 */
bool
overright_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry is strictly below the temporal network
 * point
 */
bool
below_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry does not extend above the temporal
 * network point
 */
bool
overbelow_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry is strictly above the temporal network
 * point
 */
bool
above_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the geometry does not extend below the temporal
 * network point
 */
bool
overabove_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overabove_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op geom */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly to the left of
 * the geometry
 */
bool
left_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend to the
 * right of the geometry
 */
bool
overleft_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly to the right
 * of the geometry
 */
bool
right_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend to the left
 * of the geometry
 */
bool
overright_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly below the
 * geometry
 */
bool
below_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend above the
 * geometry
 */
bool
overbelow_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly above the
 * geometry
 */
bool
above_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend below the
 * geometry
 */
bool
overabove_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overabove_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* stbox op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly to the left of the
 * temporal network point
 */
bool
left_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &left_stbox_stbox, true, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend to the right of
 * the temporal network point
 */
bool
overleft_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overleft_stbox_stbox, true,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly to the right of the
 * temporal network point
 */
bool
right_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &right_stbox_stbox, true, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend to the left of
 * the temporal network point
 */
bool
overright_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overright_stbox_stbox, true,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly below the temporal
 * network point
 */
bool
below_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &below_stbox_stbox, true, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend above the
 * temporal network point
 */
bool
overbelow_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overbelow_stbox_stbox, true,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly above the
 * temporal network point
 */
bool
above_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &above_stbox_stbox, true, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend below the
 * temporal network point
 */
bool
overabove_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overabove_stbox_stbox, true,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly before of the
 * temporal network point
 */
bool
before_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &before_stbox_stbox, false,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend after the
 * temporal network point
 */
bool
overbefore_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overbefore_stbox_stbox, false,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box is strictly after the
 * temporal network point
 */
bool
after_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &after_stbox_stbox, false, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the spatiotemporal box does not extend before the
 * temporal network point
 */
bool
overafter_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overafter_stbox_stbox, false,
    INVERT);
}

/*****************************************************************************/
/* Temporal op stbox */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly to the left of
 * the spatiotemporal box
 */
bool
left_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &left_stbox_stbox, true, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend to the
 * right of the spatiotemporal box
 */
bool
overleft_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overleft_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly to the right
 * of the spatiotemporal box
 */
bool
right_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &right_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend to the left
 * of the spatiotemporal box
 */
bool
overright_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overright_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly below the
 * spatiotemporal box
 */
bool
below_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &below_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend above the
 * spatiotemporal box
 */
bool
overbelow_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overbelow_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly above the
 * spatiotemporal box
 */
bool
above_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &above_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend below the
 * spatiotemporal box
 */
bool
overabove_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overabove_stbox_stbox, true,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly before the
 * spatiotemporal box
 */
bool
before_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &before_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend after the
 * spatiotemporal box
 */
bool
overbefore_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overbefore_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point is strictly after the
 * spatiotemporal box
 */
bool
after_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &after_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal network point does not extend before the
 * spatiotemporal box
 */
bool
overafter_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overafter_stbox_stbox, false,
    INVERT_NO);
}

/*****************************************************************************/
/* Npoint op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point is strictly to the left of the
 * temporal point
 */
bool
left_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point does not extend to the right of the
 * temporal point
 */
bool
overleft_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point is strictly to the right of the
 * temporal point
 */
bool
right_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point does not extend to the left of the
 * temporal point
 */
bool
overright_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point is strictly below the temporal point
 */
bool
below_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point does not extend above the temporal
 * point
 */
bool
overbelow_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point is strictly above the temporal point
 */
bool
above_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the network point does not extend below the temporal
 * point
 */
bool
overabove_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overabove_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op Npoint */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point is strictly to the left of the
 * network point
 */
bool
left_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point does not extend to the right of the
 * network point
 */
bool
overleft_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point is strictly to the right of the
 * network point
 */
bool
right_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point does not extend to the left of the
 * network point
 */
bool
overright_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point is strictly below the network point
 */
bool
below_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point does not extend above the network
 * point
 */
bool
overbelow_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point is strictly above the network point
 */
bool
above_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal point does not extend below the network
 * point
 */
bool
overabove_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overabove_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly to the
 * left of the second one
 */
bool
left_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &left_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend to
 * the right of the second one
 */
bool
overleft_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overleft_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly to the
 * right of the second one
 */
bool
right_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &right_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend to
 * the left of the second one
 */
bool
overright_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overright_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly below
 * the second one
 */
bool
below_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &below_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend above
 * the second one
 */
bool
overbelow_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overbelow_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly above
 * the second one
 */
bool
above_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &above_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend below
 * the second one
 */
bool
overabove_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overabove_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly before
 * the second one
 */
bool
before_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &before_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend after
 * the second one
 */
bool
overbefore_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overbefore_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point is strictly after
 * the second one
 */
bool
after_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &after_stbox_stbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal network point does not extend
 * before the second one
 */
bool
overafter_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overafter_stbox_stbox);
}

/*****************************************************************************/
