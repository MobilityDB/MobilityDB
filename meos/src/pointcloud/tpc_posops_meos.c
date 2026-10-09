/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Position bounding box operators for temporal point clouds
 * @details These operators test the relative position of the bounding boxes
 * of temporal point clouds, which are a @p TPCBox, where the *x*, *y*, and
 * optional *z* coordinates are for the space dimension and the *t* coordinate
 * is for the time dimension.
 *
 * The following operators are defined for the space dimension: left,
 * overleft, right, overright, below, overbelow, above, overabove, front,
 * overfront, back, and overback, and for the time dimension: before,
 * overbefore, after, and overafter.
 */

#include "pointcloud/tpc_boxops.h"

/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * left
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is to the left of the point cloud
 * box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Left_tpcbox_tpointcloud()
 */
bool
left_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &left_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is to
 * the left of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Left_tpointcloud_tpcbox()
 */
bool
left_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &left_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is to
 * the left of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Left_tpointcloud_tpointcloud()
 */
bool
left_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &left_tpcbox_tpcbox);
}

/*****************************************************************************
 * overleft
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend to the right of the
 * point cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overleft_tpcbox_tpointcloud()
 */
bool
overleft_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overleft_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the right of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overleft_tpointcloud_tpcbox()
 */
bool
overleft_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overleft_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the right of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overleft_tpointcloud_tpointcloud()
 */
bool
overleft_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overleft_tpcbox_tpcbox);
}

/*****************************************************************************
 * right
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is to the right of the point cloud
 * box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Right_tpcbox_tpointcloud()
 */
bool
right_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &right_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is to
 * the right of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Right_tpointcloud_tpcbox()
 */
bool
right_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &right_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is to
 * the right of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Right_tpointcloud_tpointcloud()
 */
bool
right_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &right_tpcbox_tpcbox);
}

/*****************************************************************************
 * overright
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend to the left of the
 * point cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overright_tpcbox_tpointcloud()
 */
bool
overright_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overright_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the left of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overright_tpointcloud_tpcbox()
 */
bool
overright_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overright_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the left of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overright_tpointcloud_tpointcloud()
 */
bool
overright_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overright_tpcbox_tpcbox);
}

/*****************************************************************************
 * below
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is below the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Below_tpcbox_tpointcloud()
 */
bool
below_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &below_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is below
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Below_tpointcloud_tpcbox()
 */
bool
below_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &below_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is below
 * the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Below_tpointcloud_tpointcloud()
 */
bool
below_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &below_tpcbox_tpcbox);
}

/*****************************************************************************
 * overbelow
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend above the point
 * cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overbelow_tpcbox_tpointcloud()
 */
bool
overbelow_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overbelow_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend above a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overbelow_tpointcloud_tpcbox()
 */
bool
overbelow_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overbelow_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend above the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overbelow_tpointcloud_tpointcloud()
 */
bool
overbelow_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overbelow_tpcbox_tpcbox);
}

/*****************************************************************************
 * above
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is above the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Above_tpcbox_tpointcloud()
 */
bool
above_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &above_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is above
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Above_tpointcloud_tpcbox()
 */
bool
above_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &above_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is above
 * the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Above_tpointcloud_tpointcloud()
 */
bool
above_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &above_tpcbox_tpcbox);
}

/*****************************************************************************
 * overabove
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend below the point
 * cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overabove_tpcbox_tpointcloud()
 */
bool
overabove_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overabove_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend below a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overabove_tpointcloud_tpcbox()
 */
bool
overabove_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overabove_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend below the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overabove_tpointcloud_tpointcloud()
 */
bool
overabove_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overabove_tpcbox_tpcbox);
}

/*****************************************************************************
 * front
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is in front of the point cloud box
 * of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Front_tpcbox_tpointcloud()
 */
bool
front_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &front_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is in
 * front of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Front_tpointcloud_tpcbox()
 */
bool
front_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &front_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is in
 * front of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Front_tpointcloud_tpointcloud()
 */
bool
front_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &front_tpcbox_tpcbox);
}

/*****************************************************************************
 * overfront
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend to the back of the
 * point cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overfront_tpcbox_tpointcloud()
 */
bool
overfront_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overfront_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the back of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overfront_tpointcloud_tpcbox()
 */
bool
overfront_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overfront_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the back of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overfront_tpointcloud_tpointcloud()
 */
bool
overfront_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overfront_tpcbox_tpcbox);
}

/*****************************************************************************
 * back
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is at the back of the point cloud
 * box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Back_tpcbox_tpointcloud()
 */
bool
back_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &back_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is at
 * the back of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Back_tpointcloud_tpcbox()
 */
bool
back_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &back_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is at
 * the back of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Back_tpointcloud_tpointcloud()
 */
bool
back_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &back_tpcbox_tpcbox);
}

/*****************************************************************************
 * overback
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box does not extend to the front of the
 * point cloud box of a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overback_tpcbox_tpointcloud()
 */
bool
overback_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overback_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the front of a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overback_tpointcloud_tpcbox()
 */
bool
overback_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overback_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud does not
 * extend to the front of the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overback_tpointcloud_tpointcloud()
 */
bool
overback_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overback_tpcbox_tpcbox);
}

/*****************************************************************************
 * before
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is before the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Before_tpcbox_tpointcloud()
 */
bool
before_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &before_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is
 * before a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Before_tpointcloud_tpcbox()
 */
bool
before_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &before_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is
 * before the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Before_tpointcloud_tpointcloud()
 */
bool
before_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &before_tpcbox_tpcbox);
}

/*****************************************************************************
 * overbefore
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is not after the point cloud box of
 * a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overbefore_tpcbox_tpointcloud()
 */
bool
overbefore_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overbefore_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is not
 * after a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overbefore_tpointcloud_tpcbox()
 */
bool
overbefore_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overbefore_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is not
 * after the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overbefore_tpointcloud_tpointcloud()
 */
bool
overbefore_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overbefore_tpcbox_tpcbox);
}

/*****************************************************************************
 * after
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is after the point cloud box of a
 * temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #After_tpcbox_tpointcloud()
 */
bool
after_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &after_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is after
 * a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #After_tpointcloud_tpcbox()
 */
bool
after_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &after_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is after
 * the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #After_tpointcloud_tpointcloud()
 */
bool
after_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &after_tpcbox_tpcbox);
}

/*****************************************************************************
 * overafter
 *****************************************************************************/

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if a point cloud box is not before the point cloud box of
 * a temporal point cloud
 * @param[in] box Point cloud box
 * @param[in] temp Temporal point cloud
 * @csqlfn #Overafter_tpcbox_tpointcloud()
 */
bool
overafter_tpcbox_tpointcloud(const TPCBox *box, const Temporal *temp)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overafter_tpcbox_tpcbox, INVERT);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is not
 * before a point cloud box
 * @param[in] temp Temporal point cloud
 * @param[in] box Point cloud box
 * @csqlfn #Overafter_tpointcloud_tpcbox()
 */
bool
overafter_tpointcloud_tpcbox(const Temporal *temp, const TPCBox *box)
{
  return boxop_tpointcloud_tpcbox(temp, box, &overafter_tpcbox_tpcbox, INVERT_NO);
}

/**
 * @ingroup meos_pointcloud_bbox_pos
 * @brief Return true if the point cloud box of a temporal point cloud is not
 * before the one of another temporal point cloud
 * @param[in] temp1,temp2 Temporal point clouds
 * @csqlfn #Overafter_tpointcloud_tpointcloud()
 */
bool
overafter_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2)
{
  return boxop_tpointcloud_tpointcloud(temp1, temp2, &overafter_tpcbox_tpcbox);
}

/*****************************************************************************/
