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
 * @brief Bounding box operators for temporal points.
 */

#include "point/tpoint_boxops.h"

/* PostgreSQL */
/* MobilityDB */
#include <meos.h>

/*****************************************************************************
 * overlaps
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point overlap
 * @sqlop @p &&
 */
bool
overlaps_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of
 * a temporal point overlap
 * @sqlop @p &&
 */
bool
overlaps_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and a
 * geometry/geography overlap
 * @sqlop @p &&
 */
bool
overlaps_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box overlap
 * @sqlop @p &&
 */
bool
overlaps_tpoint_stbox(const Temporal* tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points overlap
 * @sqlop @p &&
 */
bool
overlaps_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a geometry/geography contains
 * the one of a temporal point
 * @sqlop @p \@>
 */
bool
contains_bbox_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a spatiotemporal box contains the one
 * of a temporal point
 * @sqlop @p \@>
 */
bool
contains_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains the
 * one of a geometry/geography
 * @sqlop @p \@>
 */
bool
contains_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains
 * the spatiotemporal box
 * @sqlop @p \@>
 */
bool
contains_tpoint_stbox(const Temporal* tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point
 * contains the one of the second temporal point
 * @sqlop @p \@>
 */
bool
contains_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a geometry/geography is
 * contained in the one of a temporal point
 * @sqlop @p <@
 */
bool
contained_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a spatiotemporal box is contained in the
 * one of a temporal point
 * @sqlop @p <@
 */
bool
contained_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point is
 * contained in the one of a geometry/geography
 * @sqlop @p <@
 */
bool
contained_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point is
 * contained in the spatiotemporal box
 * @sqlop @p <@
 */
bool
contained_tpoint_stbox(const Temporal* tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of the first temporal point is
 * contained in the one of the second temporal point
 * @sqlop @p <@
 */
bool
contained_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point are equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of
 * a temporal point are equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and
 * a geometry/geography are equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_tpoint_stbox(const Temporal* tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are
 * equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a geometry/geography and
 * a temporal point are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_geo_tpoint(const GSERIALIZED *geo, const Temporal *tpoint)
{
  return boxop_tpoint_geo(tpoint, geo, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of
 * a temporal point are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_stbox_tpoint(const STBOX *stbox, const Temporal *tpoint)
{
  return boxop_tpoint_stbox(tpoint, stbox, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of a temporal point and
 * a geometry/geography are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_tpoint_geo(const Temporal *tpoint, const GSERIALIZED *geo)
{
  return boxop_tpoint_geo(tpoint, geo, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_tpoint_stbox(const Temporal* tpoint, const STBOX *stbox)
{
  return boxop_tpoint_stbox(tpoint, stbox, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the spatiotemporal boxes of the temporal points are
 * adjacent
 * @sqlop @p -|-
 */
bool
adjacent_tpoint_tpoint(const Temporal* tpoint1, const Temporal *tpoint2)
{
  return boxop_tpoint_tpoint(tpoint1, tpoint2, &adjacent_stbox_stbox);
}

/*****************************************************************************/
