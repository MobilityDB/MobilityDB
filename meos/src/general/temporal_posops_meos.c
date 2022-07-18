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
 * @brief Relative position operators for temporal types.
 *
 * The following operators are defined:
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include "general/temporal_boxops.h"

/*****************************************************************************/
/* <Time> op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly before a temporal value
 * @sqlop @p <<#
 */
bool
before_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not after a temporal value
 * @sqlop @p &<#
 */
bool
overbefore_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overleft_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly after a temporal value
 * @sqlop @p #>>
 */
bool
after_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not before a temporal value
 * @sqlop @p #&>
 */
bool
overafter_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overright_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly before a temporal value
 * @sqlop @p <<#
 */
bool
before_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not after a temporal value
 * @sqlop @p &<#
 */
bool
overbefore_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overleft_span_span,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly after a temporal value
 * @sqlop @p #>>
 */
bool
after_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not before a temporal value
 * @sqlop @p #&>
 */
bool
overafter_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overright_span_span,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly before a temporal value
 * @sqlop @p <<#
 */
bool
before_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not after a temporal value
 * @sqlop @p &<#
 */
bool
overbefore_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overleft_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly after a temporal value
 * @sqlop @p #>>
 */
bool
after_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not before a temporal value
 * @sqlop @p #&>
 */
bool
overafter_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overright_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly before a temporal value
 * @sqlop @p <<#
 */
bool
before_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &left_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not after a temporal value
 * @sqlop @p &<#
 */
bool
overbefore_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overleft_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is strictly after a temporal value
 * @sqlop @p #>>
 */
bool
after_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &right_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a period is not before a temporal value
 * @sqlop @p #&>
 */
bool
overafter_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overright_span_span, INVERT);
}

/*****************************************************************************/
/* Temporal op Timestamptz */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly before a timestamp
 * @sqlop @p <<#
 */
bool
before_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not after a timestamp
 * @sqlop @p &<#
 */
bool
overbefore_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly after a timestamp
 * @sqlop @p #>>
 */
bool
after_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not before a timestamp
 * @sqlop @p #&>
 */
bool
overafter_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overright_span_span, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Timestampset */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly before a timestamp set
 * @sqlop @p <<#
 */
bool
before_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not after a timestamp set
 * @sqlop @p &<#
 */
bool
overbefore_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overleft_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly after a timestamp set
 * @sqlop @p #>>
 */
bool
after_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not before a timestamp set
 * @sqlop @p #&>
 */
bool
overafter_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overright_span_span,
    INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Period */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly before a period
 * @sqlop @p <<#
 */
bool
before_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not after a period
 * @sqlop @p &<#
 */
bool
overbefore_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overleft_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly after a period
 * @sqlop @p #>>
 */
bool
after_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not before a period
 * @sqlop @p #&>
 */
bool
overafter_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overright_span_span, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op PeriodSet */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly before a period set
 * @sqlop @p <<#
 */
bool
before_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &left_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not after a period set
 * @sqlop @p &<#
 */
bool
overbefore_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overleft_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is strictly after a period set
 * @sqlop @p #>>
 */
bool
after_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &right_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal value is not before a period set
 * @sqlop @p #&>
 */
bool
overafter_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overright_span_span,
    INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is strictly before the
 * second one
 * @sqlop @p <<#
 */
bool
before_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &left_span_span);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is not after the second one
 * @sqlop @p &<#
 */
bool
overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overleft_span_span);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is strictly after the
 * second one
 * @sqlop @p #>>
 */
bool
after_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &right_span_span);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is not before the second one
 * @sqlop @p #&>
 */
bool
overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overright_span_span);
}

/*****************************************************************************/
/* Number op Tnumber */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is to the left of the bounding box of a
 * temporal integer
 * @sqlop @p <<
 */
bool
left_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is to the left of the bounding box of a
 * temporal float
 * @sqlop @p <<
 */
bool
left_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is to is not to the right of the bounding
 * box of a temporal integer
 * @sqlop @p &<
 */
bool
overleft_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is is not to the right of the bounding box of a
 * temporal float
 * @sqlop @p &<
 */
bool
overleft_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is to the right of the bounding box of a
 * temporal integer
 * @sqlop @p >>
 */
bool
right_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is to the right of the bounding box of a
 * temporal float
 * @sqlop @p >>
 */
bool
right_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is not to the left of the bounding box of a
 * temporal integer
 * @sqlop @p &>
 */
bool
overright_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overright_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is not to the left of the bounding box of a
 * temporal float
 * @sqlop @p &>
 */
bool
overright_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overright_tbox_tbox, INVERT);
}

/*****************************************************************************/
/* Span op Tnumber */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a number span is strictly to the left of a
 * temporal number
 * @sqlop @p <<
 */
bool
left_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a number span is not to the right of a
 * temporal number
 * @sqlop @p &<
 */
bool
overleft_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a number span is strictly to the right of a
 * temporal number
 * @sqlop @p >>
 */
bool
right_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a number span is not to the left of a
 * temporal number
 * @sqlop @p &>
 */
bool
overright_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &overright_tbox_tbox, INVERT);
}

/*****************************************************************************/
/* Tnumber op Number */

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is to the left
 * of an integer
 * @sqlop @p <<
 */
bool
left_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is to the left of
 * a float
 * @sqlop @p <<
 */
bool
left_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is not to the
 * right of an integer
 * @sqlop @p &<
 */
bool
overleft_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is not to the
 * right of a float
 * @sqlop @p &<
 */
bool
overleft_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is to the
 * right of an integer
 * @sqlop @p >>
 */
bool
right_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is to the
 * right of a float
 * @sqlop @p >>
 */
bool
right_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is not to the
 * left of an integer
 * @sqlop @p &>
 */
bool
overright_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overright_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is not to the
 * left of a float
 * @sqlop @p &>
 */
bool
overright_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overright_tbox_tbox, INVERT_NO);
}

/*****************************************************************************/
/* Tnumber op Span */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly to the left of a
 * number span
 * @sqlop @p <<
 */
bool
left_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not to the right of a
 * number span
 * @sqlop @p &<
 */
bool
overleft_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly to the right of a
 * number span
 * @sqlop @p >>
 */
bool
right_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not to the left of a
 * number span
 * @sqlop @p &>
 */
bool
overright_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &overright_tbox_tbox, INVERT_NO);
}

/*****************************************************************************/
/* TBOX op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is strictly to the left of a
 * temporal number
 * @sqlop @p <<
 */
bool
left_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is not to the right of a
 * temporal number
 * @sqlop @p &<
 */
bool
overleft_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is strictly to the right of a
 * temporal number
 * @sqlop @p >>
 */
bool
right_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is not to the left of a
 * temporal number
 * @sqlop @p &>
 */
bool
overright_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overright_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is strictly before a
 * temporal number
 * @sqlop @p <<#
 */
bool
before_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &before_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is not after a temporal number
 * @sqlop @p &<#
 */
bool
overbefore_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overbefore_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is strictly after a temporal number
 * @sqlop @p #>>
 */
bool
after_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &after_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal box is not before a temporal number
 * @sqlop @p #&>
 */
bool
overafter_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overafter_tbox_tbox, INVERT);
}

/*****************************************************************************/
/* Temporal op TBOX */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly to the left of
 * a temporal box
 * @sqlop @p <<
 */
bool
left_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not to the right of
 * a temporal box
 * @sqlop @p &<
 */
bool
overleft_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly to the right of
 * a temporal box
 * @sqlop @p >>
 */
bool
right_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not to the left of a temporal box
 * @sqlop @p &>
 */
bool
overright_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overright_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly before a temporal box
 * @sqlop @p <<#
 */
bool
before_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &before_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not after a temporal box
 * @sqlop @p &<#
 */
bool
overbefore_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overbefore_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is strictly after a temporal box
 * @sqlop @p #>>
 */
bool
after_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &after_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if a temporal number is not before a temporal box
 * @sqlop @p #&>
 */
bool
overafter_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overafter_tbox_tbox, INVERT_NO);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is strictly to the left of
 * the second one
 * @sqlop @p <<
 */
bool
left_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &left_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is not to the right of
 * the second one
 * @sqlop @p &<
 */
bool
overleft_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overleft_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is strictly to the right of
 * the second one
 * @sqlop @p >>
 */
bool
right_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &right_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is not to the left of
 * the second one
 * @sqlop @p &>
 */
bool
overright_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overright_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is strictly before
 * the second one
 * @sqlop @p <<#
 */
bool
before_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &before_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is not after
 * the second one
 * @sqlop @p &<#
 */
bool
overbefore_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overbefore_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is strictly after
 * the second one
 * @sqlop @p #>>
 */
bool
after_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &after_tbox_tbox);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal number is not before
 * the second one
 * @sqlop @p #&>
 */
bool
overafter_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overafter_tbox_tbox);
}

/*****************************************************************************/
