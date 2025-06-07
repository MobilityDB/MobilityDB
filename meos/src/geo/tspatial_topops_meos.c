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
 * @brief Topological bounding box operators for spatiotemporal values
 * @details These operators test the bounding boxes of spatiotemporal values,  
 * which are an @p STBox, where the *x*, *y*, and optional *z* coordinates are  
 * for the space(value) dimension and the *t* coordinate is for the time
 * dimension.
 * 
 * The following operators are defined: overlaps, contains, contained,
 * same, and adjacent.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "geo/tspatial_boxops.h"

/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"

/*****************************************************************************
 * overlaps
 *****************************************************************************/

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value overlap
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Overlaps_stbox_tspatial()
 */
inline bool
overlaps_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &overlaps_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value and
 * a spatiotemporal box overlap
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Overlaps_tspatial_stbox()
 */
inline bool
overlaps_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &overlaps_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal
 * values overlap
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Overlaps_tspatial_tspatial()
 */
inline bool
overlaps_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &overlaps_stbox_stbox);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if a spatiotemporal box contains the one of a temporal
 * point
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Contains_stbox_tspatial()
 */
inline bool
contains_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &contains_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value
 * contains a spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Contains_tspatial_stbox()
 */
inline bool
contains_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &contains_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of the first spatiotemporal
 * value contains the one of the second spatiotemporal value
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Contains_tspatial_tspatial()
 */
inline bool
contains_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &contains_stbox_stbox);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if a spatiotemporal box is contained in the
 * spatiotemporal box of a spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Contained_stbox_tspatial()
 */
inline bool
contained_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &contained_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value is
 * contained in the spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Contained_tspatial_stbox()
 */
inline bool
contained_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &contained_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of the first spatiotemporal
 * value is contained in the one of the second spatiotemporal value
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Contained_tspatial_tspatial()
 */
inline bool
contained_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &contained_stbox_stbox);
}

/*****************************************************************************
 * same
 *****************************************************************************/

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value are equal in the common dimensions
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Same_stbox_tspatial()
 */
inline bool
same_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &same_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value and
 * a spatiotemporal box are equal in the common dimensions
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Same_tspatial_stbox()
 */
inline bool
same_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &same_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal
 * values are equal in the common dimensions
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Same_tspatial_tspatial()
 */
inline bool
same_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &same_stbox_stbox);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if a spatiotemporal box and the spatiotemporal box of a
 * spatiotemporal value are adjacent
 * @param[in] box Spatiotemporal box
 * @param[in] temp Spatiotemporal value
 * @csqlfn #Adjacent_stbox_tspatial()
 */
inline bool
adjacent_stbox_tspatial(const STBox *box, const Temporal *temp)
{
  return boxop_tspatial_stbox(temp, box, &adjacent_stbox_stbox, INVERT);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal box of a spatiotemporal value and
 * a spatiotemporal box are adjacent
 * @param[in] temp Spatiotemporal value
 * @param[in] box Spatiotemporal box
 * @csqlfn #Adjacent_tspatial_stbox()
 */
inline bool
adjacent_tspatial_stbox(const Temporal *temp, const STBox *box)
{
  return boxop_tspatial_stbox(temp, box, &adjacent_stbox_stbox, INVERT_NO);
}

/**
 * @ingroup meos_geo_bbox_topo
 * @brief Return true if the spatiotemporal boxes of two spatiotemporal values
 * are adjacent
 * @param[in] temp1,temp2 Spatiotemporal values
 * @csqlfn #Adjacent_tspatial_tspatial()
 */
inline bool
adjacent_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tspatial_tspatial(temp1, temp2, &adjacent_stbox_stbox);
}

/*****************************************************************************/
