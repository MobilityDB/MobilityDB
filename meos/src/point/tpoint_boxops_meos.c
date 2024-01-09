/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @file
 * @brief Bounding box operators for temporal points
 *
 * These operators test the bounding boxes of temporal points, which are an
 * @p STBox, where the *x*, *y*, and optional *z* coordinates are for the space
 * (value) dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: overlaps, contains, contained,
 * same, and adjacent.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "point/tpoint_boxops.h"

/* MEOS */
#include <meos.h>
#include "general/temporal.h"

/*****************************************************************************
 * overlaps
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point overlap
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overlaps_stbox_tpoint()
 */
bool
overlaps_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box overlap
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overlaps_tpoint_stbox()
 */
bool
overlaps_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal points
 * overlap
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overlaps_tpoint_tpoint()
 */
bool
overlaps_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box contains the one of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Contains_stbox_tpoint()
 */
bool
contains_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal point contains a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Contains_tpoint_stbox()
 */
bool
contains_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal point
 * contains the one of the second temporal point
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Contains_tpoint_tpoint()
 */
bool
contains_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box is contained in the
 * spatiotemporal box of a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Contained_stbox_tpoint()
 */
bool
contained_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal point is
 * contained in the spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Contained_tpoint_stbox()
 */
bool
contained_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of the first temporal point is
 * contained in the one of the second temporal point
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Contained_tpoint_tpoint()
 */
bool
contained_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are equal in the common dimensions
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Same_stbox_tpoint()
 */
bool
same_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are equal in the common dimensions
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Same_tpoint_stbox()
 */
bool
same_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal points are
 * equal in the common dimensions
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Same_tpoint_tpoint()
 */
bool
same_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * temporal point are adjacent
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Adjacent_stbox_tpoint()
 */
bool
adjacent_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal box of a temporal point and a
 * spatiotemporal box are adjacent
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Adjacent_tpoint_stbox()
 */
bool
adjacent_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two temporal points are
 * adjacent
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Adjacent_tpoint_tpoint()
 */
bool
adjacent_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &adjacent_stbox_stbox);
}

/*****************************************************************************/
