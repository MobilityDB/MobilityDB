/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
#include "temporal/temporal.h"
#include "geo/tspatial_boxops.h"

/*****************************************************************************/
/* stbox op Temporal */

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is to the left of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Left_stbox_tspatial()
 */
inline bool
left_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &left_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the right of a
 * spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overleft_stbox_tspatial()
 */
inline bool
overleft_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overleft_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is to the right of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Right_stbox_tspatial()
 */
inline bool
right_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &right_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the left of a
 * spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overright_stbox_tspatial()
 */
inline bool
overright_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overright_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is below a spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Below_stbox_tspatial()
 */
inline bool
below_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &below_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend above a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overbelow_stbox_tspatial()
 */
inline bool
overbelow_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overbelow_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is above a spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Above_stbox_tspatial()
 */
inline bool
above_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &above_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend below a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overabove_stbox_tspatial()
 */
inline bool
overabove_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overabove_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is in front of a spatiotemporal
 * value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Front_stbox_tspatial()
 */
inline bool
front_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &front_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the back of a
 * spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overfront_stbox_tspatial()
 */
inline bool
overfront_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overfront_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is at the back of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Back_stbox_tspatial()
 */
inline bool
back_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &back_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box does not extend to the front of a
 * spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overback_stbox_tspatial()
 */
inline bool
overback_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overback_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is before a spatiotemporal
 * value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Before_stbox_tspatial()
 */
inline bool
before_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &before_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is not after a spatiotemporal
 * value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overbefore_stbox_tspatial()
 */
inline bool
overbefore_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overbefore_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is after a spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #After_stbox_tspatial()
 */
inline bool
after_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &after_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal box is not before a spatiotemporal
 * value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overafter_stbox_tspatial()
 */
inline bool
overafter_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overafter_stbox_stbox, INVERT);
}

/*****************************************************************************/
/* Temporal op stbox */

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is to the left of a
 * spatiotemporal
 * box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
box * @csqlfn #Left_tspatial_stbox()
 */
inline bool
left_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &left_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the right
 * of a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overleft_tspatial_stbox()
 */
inline bool
overleft_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overleft_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is to the right of a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Right_tspatial_stbox()
 */
inline bool
right_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &right_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the left
 * of a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overright_tspatial_stbox()
 */
inline bool
overright_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overright_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is below a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Below_tspatial_stbox()
 */
inline bool
below_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &below_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend above a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overbelow_tspatial_stbox()
 */
inline bool
overbelow_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overbelow_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is above a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Above_tspatial_stbox()
 */
inline bool
above_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &above_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend below a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overabove_tspatial_stbox()
 */
inline bool
overabove_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overabove_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is in front of a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Front_tspatial_stbox()
 */
inline bool
front_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &front_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the back
 * of a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overfront_tspatial_stbox()
 */
inline bool
overfront_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overfront_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is at the back of a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Back_tspatial_stbox()
 */
inline bool
back_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &back_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value does not extend to the front
 * of a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overback_tspatial_stbox()
 */
inline bool
overback_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overback_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is before a spatiotemporal
 * box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Before_tspatial_stbox()
 */
inline bool
before_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &before_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is not after a spatiotemporal
 * box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overbefore_tspatial_stbox()
 */
inline bool
overbefore_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overbefore_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is after a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #After_tspatial_stbox()
 */
inline bool
after_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &after_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if a spatiotemporal value is not before a
 * spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overafter_tspatial_stbox()
 */
inline bool
overafter_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overafter_stbox_stbox, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is to the left of the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Left_tspatial_tspatial()
 */
inline bool
left_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &left_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the right of the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overleft_tspatial_tspatial()
 */
inline bool
overleft_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overleft_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is to the right of
 * the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Right_tspatial_tspatial()
 */
inline bool
right_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &right_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the left of the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overright_tspatial_tspatial()
 */
inline bool
overright_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overright_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is below the second
 * one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Below_tspatial_tspatial()
 */
inline bool
below_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &below_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend above
 * the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overbelow_tspatial_tspatial()
 */
inline bool
overbelow_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overbelow_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is above the second
 * one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Above_tspatial_tspatial()
 */
inline bool
above_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &above_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend below
 * the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overabove_tspatial_tspatial()
 */
inline bool
overabove_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overabove_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is in front of the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Front_tspatial_tspatial()
 */
inline bool
front_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &front_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the back of the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overfront_tspatial_tspatial()
 */
inline bool
overfront_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overfront_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is at the back of the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Back_tspatial_tspatial()
 */
inline bool
back_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &back_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value does not extend to
 * the front of the second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overback_tspatial_tspatial()
 */
inline bool
overback_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overback_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is before the second
 * one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Before_tspatial_tspatial()
 */
inline bool
before_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &before_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is not after the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overbefore_tspatial_tspatial()
 */
inline bool
overbefore_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overbefore_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is after the second
 * one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #After_tspatial_tspatial()
 */
inline bool
after_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &after_stbox_stbox);
}

/**
 * @ingroup meos_geo_bbox_pos
 * @brief Return true if the first spatiotemporal value is not before the
 * second one
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overafter_tspatial_tspatial()
 */
inline bool
overafter_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overafter_stbox_stbox);
}

/*****************************************************************************/
