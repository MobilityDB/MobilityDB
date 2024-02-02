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
 * @brief Position operators for temporal geometry points
 *
 * The following operators are defined for the spatial dimension:
 * left, overleft, right, overright, below, overbelow, above,
 * overabove, front, overfront, back, overback.
 * There are no equivalent operators for the temporal geography points since
 * PostGIS does not currently provide such functionality for geography.
 *
 * The following operators are defined for for the time dimension: before,
 * overbefore,  after, overafter. For both temporal geometry and
 * geography points the same operators are derived from the basic temporal
 * types. In this file they are defined when one of the arguments is an
 * @p STBox.
 */

/* MEOS */
#include <meos.h>
#include "general/temporal.h"
#include "point/tpoint_boxops.h"

/*****************************************************************************/
/* stbox op Temporal */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is to the left of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Left_stbox_tpoint()
 */
bool
left_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the right of a
 * temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overleft_stbox_tpoint()
 */
bool
overleft_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is to the right of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Right_stbox_tpoint()
 */
bool
right_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the left of a
 * temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overright_stbox_tpoint()
 */
bool
overright_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is below a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Below_stbox_tpoint()
 */
bool
below_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend above a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overbelow_stbox_tpoint()
 */
bool
overbelow_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is above a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Above_stbox_tpoint()
 */
bool
above_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend below a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overabove_stbox_tpoint()
 */
bool
overabove_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overabove_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is in front of a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Front_stbox_tpoint()
 */
bool
front_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &front_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the back of a
 * temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overfront_stbox_tpoint()
 */
bool
overfront_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overfront_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is at the back of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Back_stbox_tpoint()
 */
bool
back_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &back_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the front of a
 * temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overback_stbox_tpoint()
 */
bool
overback_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overback_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is before a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Before_stbox_tpoint()
 */
bool
before_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &before_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is not after a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overbefore_stbox_tpoint()
 */
bool
overbefore_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overbefore_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is after a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #After_stbox_tpoint()
 */
bool
after_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &after_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a spatiotemporal box is not before a temporal point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal point
 * @csqlfn #Overafter_stbox_tpoint()
 */
bool
overafter_stbox_tpoint(const STBox *box, const Temporal *temp)
{
  return boxop_tpoint_stbox(temp, box, &overafter_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op stbox */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is to the left of a spatiotemporal
 * box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
box * @csqlfn #Left_tpoint_stbox()
 */
bool
left_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend to the right of a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overleft_tpoint_stbox()
 */
bool
overleft_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is to the right of a spatiotemporal
 * box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Right_tpoint_stbox()
 */
bool
right_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend to the left of a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overright_tpoint_stbox()
 */
bool
overright_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is below a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Below_tpoint_stbox()
 */
bool
below_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend above a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overbelow_tpoint_stbox()
 */
bool
overbelow_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is above a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Above_tpoint_stbox()
 */
bool
above_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend below a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overabove_tpoint_stbox()
 */
bool
overabove_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overabove_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is in front of a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Front_tpoint_stbox()
 */
bool
front_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &front_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend to the back of a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overfront_tpoint_stbox()
 */
bool
overfront_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overfront_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is at the back of a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Back_tpoint_stbox()
 */
bool
back_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &back_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point does not extend to the front of a
 * spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overback_tpoint_stbox()
 */
bool
overback_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overback_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is before a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Before_tpoint_stbox()
 */
bool
before_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &before_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is not after a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overbefore_tpoint_stbox()
 */
bool
overbefore_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overbefore_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is after a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #After_tpoint_stbox()
 */
bool
after_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &after_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal point is not before a spatiotemporal box
 * @param[in] temp Temporal point
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overafter_tpoint_stbox()
 */
bool
overafter_tpoint_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tpoint_stbox(temp, box, &overafter_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is to the left of the second
 * one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Left_tpoint_tpoint()
 */
bool
left_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &left_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend to the right
 * of the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overleft_tpoint_tpoint()
 */
bool
overleft_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overleft_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is to the right of the second
 * one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Right_tpoint_tpoint()
 */
bool
right_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &right_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend to the left
 * of the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overright_tpoint_tpoint()
 */
bool
overright_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overright_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is below the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Below_tpoint_tpoint()
 */
bool
below_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &below_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend above the
 * second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overbelow_tpoint_tpoint()
 */
bool
overbelow_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overbelow_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is above the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Above_tpoint_tpoint()
 */
bool
above_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &above_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend below the
 * second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overabove_tpoint_tpoint()
 */
bool
overabove_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overabove_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is in front of the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Front_tpoint_tpoint()
 */
bool
front_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &front_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend to the back
 * of the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overfront_tpoint_tpoint()
 */
bool
overfront_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overfront_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is at the back of the second
 * one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Back_tpoint_tpoint()
 */
bool
back_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &back_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point does not extend to the front
 * of the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overback_tpoint_tpoint()
 */
bool
overback_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overback_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is before the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Before_tpoint_tpoint()
 */
bool
before_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &before_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is not after the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overbefore_tpoint_tpoint()
 */
bool
overbefore_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overbefore_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is after the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #After_tpoint_tpoint()
 */
bool
after_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &after_stbox_stbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal point is not before the second one
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Overafter_tpoint_tpoint()
 */
bool
overafter_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tpoint_tpoint(temp1, temp2, &overafter_stbox_stbox);
}

/*****************************************************************************/
