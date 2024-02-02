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
 * @brief Bounding box functions for temporal types
 *
 * The bounding box of temporal values are
 * - a @p Span for temporal Boolean and temporal text values
 * - a @p TBox for temporal integers and floats, where the @p x coordinate is 
 *   for the value dimension and the @p t coordinate is for the time dimension.
 * The following functions are defined: @p overlaps, @p contains, @p contained,
 * @p same, and @p adjacent.
 *
 * The functions consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "general/temporal_boxops.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "general/temporal.h"

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span contains the time span of a
 * temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contains_tstzspan_temporal()
 */
bool
contains_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &contains_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value contains a
 * timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contains_temporal_tstzspan()
 */
bool
contains_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value
 * contains the one of the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contains_temporal_temporal()
 */
bool
contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contains_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span is contained the time span of
 * a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contained_tstzspan_temporal()
 */
bool
contained_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &contained_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value is contained
 * in a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contained_temporal_tstzspan()
 */
bool
contained_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of the first temporal value is
 * contained in the one of the second temporal value
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contained_temporal_temporal()
 */
bool
contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contained_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value overlap
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_tstzspan_temporal()
 */
bool
overlaps_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &overlaps_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a
 * timestamptz span overlap
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overlaps_temporal_tstzspan()
 */
bool
overlaps_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values overlap
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overlaps_temporal_temporal()
 */
bool
overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overlaps_span_span);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a
 * temporal value are equal
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Same_tstzspan_temporal()
 */
bool
same_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &span_eq, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a timestamptz
 * span are equal
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Same_temporal_tstzspan()
 */
bool
same_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &span_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Same_temporal_temporal()
 */
bool
same_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &span_eq);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a timestamptz span and the time span of a temporal
 * value are adjacent
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_tstzspan_temporal()
 */
bool
adjacent_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &adjacent_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time span of a temporal value and a timestamptz
 * span are adjacent
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Adjacent_temporal_tstzspan()
 */
bool
adjacent_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the time spans of two temporal values are adjacent
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Adjacent_temporal_temporal()
 */
bool
adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &adjacent_span_span);
}

/*****************************************************************************
 * Bounding box functions for temporal numbers
 *****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span contains the value span of a temporal
 * number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contains_numspan_tnumber()
 */
bool
contains_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &contains_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number contains a number
 * span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contains_tnumber_numspan()
 */
bool
contains_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box contains the bounding box of a
 * temporal number
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Contains_tbox_tnumber()
 */
bool
contains_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number contains a
 * temporal box
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Contains_tnumber_tbox()
 */
bool
contains_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number contains
 * the one of the second temporal number
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contains_tnumber_tnumber()
 */
bool
contains_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &contains_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span is contained in the value span of a
 * temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Contained_numspan_tnumber()
 */
bool
contained_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &contained_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number is contained in a
 * number span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Contained_tnumber_numspan()
 */
bool
contained_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box is contained in the bounding box of a
 * temporal number
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Contained_tbox_tnumber()
 */
bool
contained_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number is contained in
 * a temporal box
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Contained_tnumber_tbox()
 */
bool
contained_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of the first temporal number is
 * contained in the one of the second temporal number
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Contained_tnumber_tnumber()
 */
bool
contained_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &contained_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * overlap
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_numspan_tnumber()
 */
bool
overlaps_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &overlaps_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and the number
 * span overlap
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overlaps_tnumber_numspan()
 */
bool
overlaps_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number overlap
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Overlaps_tbox_tnumber()
 */
bool
overlaps_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box overlap
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Overlaps_tnumber_tbox()
 */
bool
overlaps_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers overlap
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overlaps_tnumber_tnumber()
 */
bool
overlaps_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overlaps_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * are equal
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Same_numspan_tnumber()
 */
bool
same_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &span_eq, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a number span
 * are equal
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Same_tnumber_numspan()
 */
bool
same_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &span_eq, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are equal in the common dimensions
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Same_tbox_tnumber()
 */
bool
same_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box are equal in the common dimensions
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Same_tnumber_tbox()
 */
bool
same_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are equal
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Same_tnumber_tnumber()
 */
bool
same_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &same_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a number span and the value span of a temporal number
 * are adjacent
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_numspan_tnumber()
 */
bool
adjacent_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &adjacent_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the value span of a temporal number and a number span
 * are adjacent
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Adjacent_tnumber_numspan()
 */
bool
adjacent_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if a temporal box and the bounding box of a temporal
 * number are adjacent
 * @param[in] box Bounding box
 * @param[in] temp Temporal value
 * @csqlfn #Adjacent_tbox_tnumber()
 */
bool
adjacent_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding box of a temporal number and a temporal
 * box are adjacent
 * @param[in] temp Temporal value
 * @param[in] box Bounding box
 * @csqlfn #Adjacent_tnumber_tbox()
 */
bool
adjacent_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_topo
 * @brief Return true if the bounding boxes of two temporal numbers are
 * adjacent
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Adjacent_tnumber_tnumber()
 */
bool
adjacent_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &adjacent_tbox_tbox);
}

/*****************************************************************************/
