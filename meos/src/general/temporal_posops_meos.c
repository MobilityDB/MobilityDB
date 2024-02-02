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
 * @brief Position operators for temporal types
 *
 * The following operators are defined:
 * - @p left, @p overleft, @p right, @p overright for the value dimension
 * - @p before, @p overbefore, @p after, @p overafter for the time dimension
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/temporal.h"
#include "general/temporal_boxops.h"

/*****************************************************************************/
/* Span op Temporal */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is before a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Before_tstzspan_temporal()
 */
bool
before_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &left_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not after a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overbefore_tstzspan_temporal()
 */
bool
overbefore_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &overleft_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is after a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #After_tstzspan_temporal()
 */
bool
after_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &right_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not before a temporal value
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overafter_tstzspan_temporal()
 */
bool
overafter_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &overright_span_span, INVERT);
}

/*****************************************************************************/
/* Temporal op Period */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal value is before a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Before_temporal_tstzspan()
 */
bool
before_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &left_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal value is not after a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overbefore_temporal_tstzspan()
 */
bool
overbefore_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal value is after a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #After_temporal_tstzspan()
 */
bool
after_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &right_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal value is not before a timestamptz span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overafter_temporal_tstzspan()
 */
bool
overafter_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &overright_span_span, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal value is before the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Before_temporal_temporal()
 */
bool
before_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &left_span_span);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal value is not after the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overbefore_temporal_temporal()
 */
bool
overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overleft_span_span);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal value is after the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #After_temporal_temporal()
 */
bool
after_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &right_span_span);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal value is not before the second one
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Overafter_temporal_temporal()
 */
bool
overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overright_span_span);
}

/*****************************************************************************/
/* Span op Tnumber */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a number span is to the left of a temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Left_numspan_tnumber()
 */
bool
left_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &left_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the right of a
 * temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overleft_numspan_tnumber()
 */
bool
overleft_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &overleft_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a number span is to the right of a temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Right_numspan_tnumber()
 */
bool
right_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &right_span_span, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the left of a
 * temporal number
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Overright_numspan_tnumber()
 */
bool
overright_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &overright_span_span, INVERT);
}

/*****************************************************************************/
/* Tnumber op Span */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a number span
 * @param[in] s Span
 * @param[in] temp Temporal value
 * @csqlfn #Left_tnumber_numspan()
 */
bool
left_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &left_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a
 * number span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overleft_tnumber_numspan()
 */
bool
overleft_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a number span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Right_tnumber_numspan()
 */
bool
right_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &right_span_span, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a
 * number span
 * @param[in] temp Temporal value
 * @param[in] s Span
 * @csqlfn #Overright_tnumber_numspan()
 */
bool
overright_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &overright_span_span, INVERT_NO);
}

/*****************************************************************************/
/* TBox op Temporal */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is to the left of a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Left_tbox_tnumber()
 */
bool
left_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the right of a
 * temporal number
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Overleft_tbox_tnumber()
 */
bool
overleft_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is to the right of a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Right_tbox_tnumber()
 */
bool
right_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the left of a
 * temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Overright_tbox_tnumber()
 */
bool
overright_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overright_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is before a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Before_tbox_tnumber()
 */
bool
before_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &before_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is not after a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Overbefore_tbox_tnumber()
 */
bool
overbefore_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overbefore_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is after a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #After_tbox_tnumber()
 */
bool
after_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &after_tbox_tbox, INVERT);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal box is not before a temporal number
 * @param[in] box Temporal box
 * @param[in] temp Temporal number
 * @csqlfn #Overafter_tbox_tnumber()
 */
bool
overafter_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overafter_tbox_tbox, INVERT);
}

/*****************************************************************************/
/* Temporal op TBox */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Left_tnumber_tbox()
 */
bool
left_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a
 * temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Overleft_tnumber_tbox()
 */
bool
overleft_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Right_tnumber_tbox()
 */
bool
right_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a
 * temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Overright_tnumber_tbox()
 */
bool
overright_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overright_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is before a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Before_tnumber_tbox()
 */
bool
before_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &before_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is not after a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Overbefore_tnumber_tbox()
 */
bool
overbefore_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overbefore_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is after a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #After_tnumber_tbox()
 */
bool
after_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &after_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if a temporal number is not before a temporal box
 * @param[in] temp Temporal number
 * @param[in] box Temporal box
 * @csqlfn #Overafter_tnumber_tbox()
 */
bool
overafter_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overafter_tbox_tbox, INVERT_NO);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the left of the
 * second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Left_tnumber_tnumber()
 */
bool
left_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &left_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the right
 * of the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Overleft_tnumber_tnumber()
 */
bool
overleft_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overleft_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the right of the
 * second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Right_tnumber_tnumber()
 */
bool
right_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &right_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the left
 * of the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Overright_tnumber_tnumber()
 */
bool
overright_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overright_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is before the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Before_tnumber_tnumber()
 */
bool
before_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &before_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is not after the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Overbefore_tnumber_tnumber()
 */
bool
overbefore_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overbefore_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is after the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #After_tnumber_tnumber()
 */
bool
after_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &after_tbox_tbox);
}

/**
 * @ingroup meos_temporal_bbox_pos
 * @brief Return true if the first temporal number is not before the second one
 * @param[in] temp1,temp2 Temporal numbers
 * @csqlfn #Overafter_tnumber_tnumber()
 */
bool
overafter_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overafter_tbox_tbox);
}

/*****************************************************************************/
