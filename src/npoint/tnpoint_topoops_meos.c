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
 * @file tnpoint_topoops_meos.c
 * @brief Topological operators for temporal network points.
 *
 * These operators test the bounding boxes of temporal npoints, which are
 * STBOX boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "npoint/tnpoint_boxops.h"

/* MobilityDB */
#include "point/tpoint_boxops.h"


/*****************************************************************************
 * overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the geometry and
 * the temporal network point overlap
 */
bool
overlaps_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tpoint_geo(tnpoint, geo, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point overlap
 */
bool
overlaps_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tpoint_stbox(tnpoint, stbox, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point overlap
 */
bool
overlaps_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overlaps_stbox_stbox, INVERT);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the geometry overlap
 */
bool
overlaps_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box overlap
 */
bool
overlaps_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &overlaps_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the network point overlap
 */
bool
overlaps_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * overlap
 */
bool
overlaps_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the geometry contains the one of
 * the temporal network point
 */
bool
contains_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tpoint_geo(tnpoint, geo, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box contains the one of the temporal
 * network point
 */
bool
contains_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tpoint_stbox(tnpoint, stbox, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the network point contains
 * the one of the temporal network point
 */
bool
contains_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &contains_stbox_stbox, INVERT);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the geometry
 */
bool
contains_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the spatiotemporal box
 */
bool
contains_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &contains_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * contain the one of the network point
 */
bool
contains_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point contain the one of the second temporal network point
 */
bool
contains_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the geometry is contained by the
 * one of the temporal network point
 */
bool
contained_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tpoint_geo(tnpoint, geo, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box is contained by the one of the
 * temporal network point
 */
bool
contained_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tpoint_stbox(tnpoint, stbox, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the network point is
 * contained by the one of the temporal network point
 */
bool
contained_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &contained_stbox_stbox, INVERT);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the geometry
 */
bool
contained_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the spatiotemporal box
 */
bool
contained_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &contained_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point is
 * contained by the one of the network point
 */
bool
contained_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal network
 * point is contained by the one of the second temporal network point
 */
bool
contained_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the geometry and the
 * temporal network point are equal in the common dimensions
 */
bool
same_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tpoint_geo(tnpoint, geo, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point are equal in the common dimensions
 */
bool
same_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tpoint_stbox(tnpoint, stbox, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are equal in the common dimensions
 */
bool
same_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &same_stbox_stbox, INVERT);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the geometry are equal in the common dimensions
 */
bool
same_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box are equal in the common dimensions
 */
bool
same_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &same_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the network point are equal in the common dimensions
 */
bool
same_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * are equal in the common dimensions
 */
bool
same_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &same_stbox_stbox);
}

/*****************************************************************************
 * Adjacent
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the geometry and the
 * temporal network point are adjacent
 */
bool
adjacent_geo_tnpoint(const GSERIALIZED *geo, const Temporal *tnpoint)
{
  return boxop_tpoint_geo(tnpoint, geo, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box and the spatiotemporal box of
 * the temporal network point are adjacent
 */
bool
adjacent_stbox_tnpoint(const STBOX *stbox, const Temporal *tnpoint)
{
  return boxop_tpoint_stbox(tnpoint, stbox, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the network point and the
 * temporal network point are adjacent
 */
bool
adjacent_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return boxop_tnpoint_npoint(tnpoint, np, &adjacent_stbox_stbox, INVERT);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the geometry are adjacent
 */
bool
adjacent_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *geo)
{
  return boxop_tnpoint_geo(tnpoint, geo, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the temporal network point
 * and the spatiotemporal box are adjacent
 */
bool
adjacent_tnpoint_stbox(const Temporal *tnpoint, const STBOX *stbox)
{
  return boxop_tnpoint_stbox(tnpoint, stbox, &adjacent_stbox_stbox, false,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network point
 * and the network point are adjacent
 */
bool
adjacent_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return boxop_tnpoint_npoint(tnpoint, np, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal network points
 * are adjacent
 */
bool
adjacent_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return boxop_tnpoint_tnpoint(tnpoint1, tnpoint2, &adjacent_stbox_stbox);
}

/*****************************************************************************/
