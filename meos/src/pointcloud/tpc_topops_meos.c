/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Topological bounding box operators for temporal point clouds
 * @details These operators test the bounding boxes of temporal point clouds,
 * which are a @p TPCBox, where the *x*, *y*, and optional *z* coordinates are
 * for the space dimension and the *t* coordinate is for the time dimension.
 *
 * The following operators are defined: overlaps, contains, contained, same,
 * and adjacent.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both the
 * space and the time dimensions.
 */

#include "pointcloud/tpc_boxops.h"

/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * overlaps
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box overlaps the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overlaps_tpcbox_tpointcloud()
 */
bool
overlaps_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overlaps_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud overlaps
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overlaps_tpointcloud_tpcbox()
 */
bool
overlaps_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overlaps_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud overlaps
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overlaps_tpointcloud_tpointcloud()
 */
bool
overlaps_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overlaps_tpcbox_tpcbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box contains the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Contains_tpcbox_tpointcloud()
 */
bool
contains_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contains_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud contains
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Contains_tpointcloud_tpcbox()
 */
bool
contains_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contains_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud contains
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Contains_tpointcloud_tpointcloud()
 */
bool
contains_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &contains_tpcbox_tpcbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is contained in the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Contained_tpcbox_tpointcloud()
 */
bool
contained_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contained_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is contained in
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Contained_tpointcloud_tpcbox()
 */
bool
contained_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &contained_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is contained in
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Contained_tpointcloud_tpointcloud()
 */
bool
contained_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &contained_tpcbox_tpcbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is equal in the common dimensions to the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Same_tpcbox_tpointcloud()
 */
bool
same_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &same_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is equal in the common dimensions to
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Same_tpointcloud_tpcbox()
 */
bool
same_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &same_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is equal in the common dimensions to
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Same_tpointcloud_tpointcloud()
 */
bool
same_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &same_tpcbox_tpcbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if a point cloud box is adjacent to the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Adjacent_tpcbox_tpointcloud()
 */
bool
adjacent_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &adjacent_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is adjacent to
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Adjacent_tpointcloud_tpcbox()
 */
bool
adjacent_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &adjacent_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_topo
 * @brief Return true if the point cloud box of a temporal point cloud is adjacent to
 * the point cloud box of another one
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Adjacent_tpointcloud_tpointcloud()
 */
bool
adjacent_tpointcloud_tpointcloud(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &adjacent_tpcbox_tpcbox);
}

/*****************************************************************************/
