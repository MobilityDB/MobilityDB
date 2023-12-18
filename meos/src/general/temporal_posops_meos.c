/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_boxops.h"

/*****************************************************************************/
/* Span op Temporal */

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is before a temporal value
 * @csqlfn #Before_tstzspan_temporal()
 */
bool
before_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not after a temporal value
 * @csqlfn #Overbefore_tstzspan_temporal()
 */
bool
overbefore_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &overleft_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is after a temporal value
 * @csqlfn #After_tstzspan_temporal()
 */
bool
after_tstzspan_temporal(const Span *s, const Temporal *temp)
{
  return boxop_temporal_tstzspan(temp, s, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a timestamptz span is not before a temporal value
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal value is before a timestamptz span
 * @csqlfn #Before_temporal_tstzspan()
 */
bool
before_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal value is not after a timestamptz span
 * @csqlfn #Overbefore_temporal_tstzspan()
 */
bool
overbefore_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal value is after a timestamptz span
 * @csqlfn #After_temporal_tstzspan()
 */
bool
after_temporal_tstzspan(const Temporal *temp, const Span *s)
{
  return boxop_temporal_tstzspan(temp, s, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal value is not before a timestamptz span
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal value is before the second one
 * @csqlfn #Before_temporal_temporal()
 */
bool
before_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &left_span_span);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal value is not after the second one
 * @csqlfn #Overbefore_temporal_temporal()
 */
bool
overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overleft_span_span);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal value is after the second one
 * @csqlfn #After_temporal_temporal()
 */
bool
after_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &right_span_span);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal value is not before the second one
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a number span is to the left of a temporal number
 * @csqlfn #Left_numspan_tnumber()
 */
bool
left_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the right of a
 * temporal number
 * @csqlfn #Overleft_numspan_tnumber()
 */
bool
overleft_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &overleft_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a number span is to the right of a temporal number
 * @csqlfn #Right_numspan_tnumber()
 */
bool
right_numspan_tnumber(const Span *s, const Temporal *temp)
{
  return boxop_tnumber_numspan(temp, s, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a number span does not extend to the left of a temporal number
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a number span
 * @csqlfn #Left_tnumber_numspan()
 */
bool
left_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a number span
 * @csqlfn #Overleft_tnumber_numspan()
 */
bool
overleft_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a number span
 * @csqlfn #Right_tnumber_numspan()
 */
bool
right_tnumber_numspan(const Temporal *temp, const Span *s)
{
  return boxop_tnumber_numspan(temp, s, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a
 * number span
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is to the left of a temporal number
 * @csqlfn #Left_tbox_tnumber()
 */
bool
left_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the right of a temporal number
 * @csqlfn #Overleft_tbox_tnumber()
 */
bool
overleft_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is to the right of a temporal number
 * @csqlfn #Right_tbox_tnumber()
 */
bool
right_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box does not extend to the left of a temporal number
 * @csqlfn #Overright_tbox_tnumber()
 */
bool
overright_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overright_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is before a temporal number
 * @csqlfn #Before_tbox_tnumber()
 */
bool
before_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &before_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is not after a temporal number
 * @csqlfn #Overbefore_tbox_tnumber()
 */
bool
overbefore_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &overbefore_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is after a temporal number
 * @csqlfn #After_tbox_tnumber()
 */
bool
after_tbox_tnumber(const TBox *box, const Temporal *temp)
{
  return boxop_tnumber_tbox(temp, box, &after_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal box is not before a temporal number
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the left of a temporal box
 * @csqlfn #Left_tnumber_tbox()
 */
bool
left_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the right of a temporal box
 * @csqlfn #Overleft_tnumber_tbox()
 */
bool
overleft_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is to the right of a temporal box
 * @csqlfn #Right_tnumber_tbox()
 */
bool
right_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number does not extend to the left of a temporal box
 * @csqlfn #Overright_tnumber_tbox()
 */
bool
overright_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overright_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is before a temporal box
 * @csqlfn #Before_tnumber_tbox()
 */
bool
before_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &before_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is not after a temporal box
 * @csqlfn #Overbefore_tnumber_tbox()
 */
bool
overbefore_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &overbefore_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is after a temporal box
 * @csqlfn #After_tnumber_tbox()
 */
bool
after_tnumber_tbox(const Temporal *temp, const TBox *box)
{
  return boxop_tnumber_tbox(temp, box, &after_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if a temporal number is not before a temporal box
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
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the left of the
 * second one
 * @csqlfn #Left_tnumber_tnumber()
 */
bool
left_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &left_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the right of the
 * second one
 * @csqlfn #Overleft_tnumber_tnumber()
 */
bool
overleft_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overleft_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is to the right of the
 * second one
 * @csqlfn #Right_tnumber_tnumber()
 */
bool
right_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &right_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number does not extend to the left of the
 * second one
 * @csqlfn #Overright_tnumber_tnumber()
 */
bool
overright_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overright_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is before the second one
 * @csqlfn #Before_tnumber_tnumber()
 */
bool
before_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &before_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is not after the second one
 * @csqlfn #Overbefore_tnumber_tnumber()
 */
bool
overbefore_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overbefore_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is after the second one
 * @csqlfn #After_tnumber_tnumber()
 */
bool
after_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &after_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_bbox_pos
 * @brief Return true if the first temporal number is not before the second one
 * @csqlfn #Overafter_tnumber_tnumber()
 */
bool
overafter_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_tnumber_tnumber(temp1, temp2, &overafter_tbox_tbox);
}

/*****************************************************************************/
