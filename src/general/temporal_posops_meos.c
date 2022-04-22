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
 * @file temporal_posops.c
 * @brief Relative position operators for temporal types.
 *
 * The following operators are defined:
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

#include "general/temporal_posops.h"

/* PostgreSQL */
#include <assert.h>
/* MobilityDB */
#include "general/time_ops.h"
#include "general/temporal.h"
#include "general/temporal_boxops.h"

/*****************************************************************************/
/* <Time> op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly before the temporal value
 */
bool
before_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &before_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not after the temporal value
 */
bool
overbefore_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overbefore_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly after the temporal value
 */
bool
after_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &after_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not before the temporal value
 */
bool
overafter_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overafter_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly before the temporal value
 */
bool
before_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &before_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not after the temporal value
 */
bool
overbefore_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overbefore_period_period,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly after the temporal value
 */
bool
after_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &after_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not before the temporal value
 */
bool
overafter_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overafter_period_period,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly before the temporal value
 */
bool
before_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &before_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not after the temporal value
 */
bool
overbefore_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overbefore_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly after the temporal value
 */
bool
after_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &after_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not before the temporal value
 */
bool
overafter_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overafter_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly before the temporal value
 */
bool
before_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &before_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not after the temporal value
 */
bool
overbefore_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overbefore_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is strictly after the temporal value
 */
bool
after_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &after_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the period value is not before the temporal value
 */
bool
overafter_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overafter_period_period, INVERT);
}

/*****************************************************************************/
/* Temporal op Timestamptz */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly before the timestamptz
 * value
 */
bool
before_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &before_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not after the timestamptz value
 */
bool
overbefore_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overbefore_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly after the timestamptz
 * value
 */
bool
after_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &after_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not before the timestamptz value
 */
bool
overafter_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overafter_period_period, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Timestampset */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly before the timestampset
 * value
 */
bool
before_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &before_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not after the timestampset value
 */
bool
overbefore_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overbefore_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly after the timestampset
 * value
 */
bool
after_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &after_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not before the timestampset value
 */
bool
overafter_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overafter_period_period,
    INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Period */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly before the period value
 */
bool
before_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &before_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not after the period value
 */
bool
overbefore_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overbefore_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly after the period value
 */
bool
after_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &after_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not before the period value
 */
bool
overafter_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overafter_period_period, INVERT_NO);
}

/*****************************************************************************/
/* Temporal op PeriodSet */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly before the periodset
 * value
 */
bool
before_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &before_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not after the periodset value
 */
bool
overbefore_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overbefore_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is strictly after the periodset
 * value
 */
bool
after_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &after_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal value is not before the periodset value
 */
bool
overafter_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overafter_period_period,
    INVERT_NO);
}

/*****************************************************************************/
/* Temporal op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is strictly before the
 * second one
 */
bool
before_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &before_period_period);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is not after the second one
 */
bool
overbefore_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overbefore_period_period);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is strictly after the
 * second one
 */
bool
after_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &after_period_period);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the first temporal value is not before the second one
 */
bool
overafter_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overafter_period_period);
}

/*****************************************************************************/
/* Number op Tnumber */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number value is strictly to the left of the
 * temporal number
 */
bool
left_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &left_tbox_tbox,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number value is not to the right of the
 * temporal number
 */
bool
overleft_number_tnumber(Datum number, CachedType basetype,
  const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overleft_tbox_tbox,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number value is strictly to the right of the
 * temporal number
 */
bool
right_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &right_tbox_tbox,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number value is not to the left of the
 * temporal number
 */
bool
overright_number_tnumber(Datum number, CachedType basetype,
  const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overright_tbox_tbox,
    INVERT);
}

/*****************************************************************************/
/* Range op Tnumber */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number range value is strictly to the left of the
 * temporal number
 */
bool
left_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number range value is not to the right of the
 * temporal number
 */
bool
overleft_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number range value is strictly to the right of the
 * temporal number
 */
bool
right_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the number range value is not to the left of the
 * temporal number
 */
bool
overright_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &overright_tbox_tbox, INVERT);
}

/*****************************************************************************/
/* Tnumber op Number */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly to the left of the
 * number value
 */
bool
left_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &left_tbox_tbox,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the right of the
 * number value
 */
bool
overleft_tnumber_number(const Temporal *tnumber, Datum number,
  CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overleft_tbox_tbox,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly to the right of the
 * number value
 */
bool
right_tnumber_number(const Temporal *tnumber, Datum number,
  CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &right_tbox_tbox,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the left of the
 * number value
 */
bool
overright_tnumber_number(const Temporal *tnumber, Datum number,
  CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overright_tbox_tbox,
    INVERT_NO);
}

/*****************************************************************************/
/* Tnumber op Range */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly to the left of the
 * number range value
 */
bool
left_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the right of the
 * number range value
 */
bool
overleft_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly to the right of the
 * number range value
 */
bool
right_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the left of the
 * number range value
 */
bool
overright_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &overright_tbox_tbox, INVERT_NO);
}

/*****************************************************************************/
/* TBOX op Temporal */

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is strictly to the left of the
 * temporal number
 */
bool
left_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &left_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is not to the right of the
 * temporal number
 */
bool
overleft_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overleft_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is strictly to the right of the
 * temporal number
 */
bool
right_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &right_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is not to the left of the
 * temporal number
 */
bool
overright_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overright_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is strictly before the
 * temporal number
 */
bool
before_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &before_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is not after the
 * temporal number
 */
bool
overbefore_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overbefore_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is strictly after the
 * temporal number
 */
bool
after_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &after_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal box value is not before the
 * temporal number
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
 * @brief Return true if the temporal number is strictly to the left of
 * the temporal box value
 */
bool
left_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &left_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the right of
 * the temporal box value
 */
bool
overleft_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overleft_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly to the right of
 * the temporal box value
 */
bool
right_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &right_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not to the left of the
 * temporal box value
 */
bool
overright_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overright_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly before the
 * temporal box value
 */
bool
before_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &before_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not after the
 * temporal box value
 */
bool
overbefore_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overbefore_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is strictly after the
 * temporal box value
 */
bool
after_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &after_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_pos
 * @brief Return true if the temporal number is not before the
 * temporal box value
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
 */
bool
overafter_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overafter_tbox_tbox);
}

/*****************************************************************************/
