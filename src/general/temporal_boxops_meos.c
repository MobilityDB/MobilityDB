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
 * @file temporal_boxops.c
 * @brief Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are
 * - a `Period` for temporal Booleans
 * - a `TBOX` for temporal integers and floats, where the *x* coordinate is for
 *   the value dimension and the *t* coordinate is for the time dimension.
 * The following operators are defined: `overlaps`, `contains`, `contained`,
 * `same`, and `adjacent`.
 *
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 */

#include "general/temporal_boxops.h"

/* PostgreSQL */
#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/rangetypes_ext.h"
#include "general/tbox.h"
#include "point/tpoint.h"
#include "point/stbox.h"
#include "point/tpoint_boxops.h"
#include "npoint/tnpoint_boxops.h"

/*****************************************************************************
 * Bounding box operators for temporal types
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the timestamp contains the bounding period of the
 * temporal value
 */
bool
contains_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &contains_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value contains the
 * timestamp
 */
bool
contains_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &contains_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the timestampset contains the
 * bounding period of the temporal value
 */
bool
contains_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &contains_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value contains the
 * the bounding period of timestampset
 */
bool
contains_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &contains_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the period contains the bounding period of the
 * temporal value
 */
bool
contains_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &contains_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value contains
 * the period
 */
bool
contains_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &contains_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the periodset contains the
 * bounding period of the temporal value
 */
bool
contains_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &contains_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value contains
 * the bounding period of the periodset
 */
bool
contains_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &contains_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the first temporal value
 * contains the bounding period of the second one.
 */
bool
contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contains_period_period);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the timestamp is contained in the bounding period of
 * the temporal value
 */
bool
contained_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &contained_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value is contained
 * in the timestamp
 */
bool
contained_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &contained_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the timestampset is contained
 * in the bounding period of the temporal value
 */
bool
contained_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &contained_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value is contained
 * in the bouding period of timestampset
 */
bool
contained_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &contained_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the period is contained the bounding period of the
 * temporal value
 */
bool
contained_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &contained_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value is contained
 * in the period
 */
bool
contained_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &contained_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the the bounding period of the periodset is contained
 * in the bounding period of the temporal value
 */
bool
contained_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &contained_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value is contained
 * in the bounding period of the periodset
 */
bool
contained_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &contained_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the first temporal value is
 * contained in the bounding period of the second one.
 */
bool
contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contained_period_period);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the timestamp and the bounding period of the temporal
 * value overlap
 */
bool
overlaps_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overlaps_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * timestamp overlap
 */
bool
overlaps_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overlaps_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the the bounding period of timestampset and the
 * bounding period of the temporal value overlap
 */
bool
overlaps_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overlaps_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the timestampset overlap
 */
bool
overlaps_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overlaps_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the period and the bounding period of the temporal
 * value overlap
 */
bool
overlaps_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overlaps_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * period overlap
 */
bool
overlaps_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overlaps_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the periodset and the bounding
 * period of the temporal value overlap
 */
bool
overlaps_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overlaps_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the periodset overlap
 */
bool
overlaps_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overlaps_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values overlap
 */
bool
overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overlaps_period_period);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the timestamp and the bounding period of the temporal
 * value are equal
 */
bool
same_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &period_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * timestamp are equal
 */
bool
same_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &period_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the timestampset and the
 * bounding period of the temporal value are equal
 */
bool
same_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &period_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the timestampset are equal
 */
bool
same_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &period_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the period and the bounding period of the temporal
 * value are equal
 */
bool
same_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &period_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * period are equal
 */
bool
same_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &period_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the periodset and the bounding
 * period of the temporal value are equal
 */
bool
same_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &period_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the periodset are equal
 */
bool
same_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &period_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values are equal
 */
bool
same_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &period_eq);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the timestamp and the bounding period of the temporal
 * value are adjacent
 */
bool
adjacent_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &adjacent_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * timestamp are adjacent
 */
bool
adjacent_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &adjacent_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the timestampset and the
 * bounding period of the temporal value are adjacent
 */
bool
adjacent_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &adjacent_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the timestampset are adjacent
 */
bool
adjacent_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &adjacent_period_period,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the period and the bounding period of the temporal
 * value are adjacent
 */
bool
adjacent_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &adjacent_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * period are adjacent
 */
bool
adjacent_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &adjacent_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the periodset and the bounding
 * period of the temporal value are adjacent
 */
bool
adjacent_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &adjacent_period_period, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the temporal value and the
 * bounding period of the periodset are adjacent
 */
bool
adjacent_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &adjacent_period_period, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values are
 * adjacent
 */
bool
adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &adjacent_period_period);
}

/*****************************************************************************
 * Bounding box operators for temporal numbers
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the number contains the bounding box of the temporal
 * number
 */
bool
contains_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number contains the
 * the number
 */
bool
contains_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range contains the bounding box of the temporal
 * number
 */
bool
contains_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number contains the
 * range
 */
bool
contains_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal box contains the bounding box of the
 * temporal number
 */
bool
contains_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number contains the
 * temporal box
 */
bool
contains_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the first temporal number contains
 * the one of the second temporal number
 */
bool
contains_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &contains_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the number is contained in the bounding box of the
 * temporal number
 */
bool
contained_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number is contained
 * in the number
 */
bool
contained_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range is contained in the bounding box of the
 * temporal number
 */
bool
contained_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number is contained
 * in the range
 */
bool
contained_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal box is contained in the bounding box of
 * the temporal number
 */
bool
contained_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number is contained
 * in the temporal box
 */
bool
contained_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the first temporal number is
 * contained in the one of the second temporal number
 */
bool
contained_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &contained_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * overlap
 */
bool
overlaps_number_tnumber(Datum number, CachedType basetype,
  const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range overlap
 */
bool
overlaps_tnumber_number(const Temporal *tnumber, Datum number,
  CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * overlap
 */
bool
overlaps_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range overlap
 */
bool
overlaps_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal box and the bounding box
 * of the temporal number overlap
 */
bool
overlaps_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * temporal box overlap
 */
bool
overlaps_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding boxes of the temporal numbers overlap
 */
bool
overlaps_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overlaps_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * are equal on the common dimensions
 */
bool
same_number_tnumber(Datum number, CachedType basetype, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range are equal on the common dimensions
 */
bool
same_tnumber_number(const Temporal *tnumber, Datum number, CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * are equal on the common dimensions
 */
bool
same_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range are equal on the common dimensions
 */
bool
same_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal box and the bounding box
 * of the temporal number are equal in the common dimensions
 */
bool
same_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * temporal box are equal in the common dimensions
 */
bool
same_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding boxes of the temporal numbers are equal
 * in the common dimensions
 */
bool
same_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &same_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * are adjacent
 */
bool
adjacent_number_tnumber(Datum number, CachedType basetype,
  const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, number, basetype, &adjacent_tbox_tbox,
    INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range are adjacent
 */
bool
adjacent_tnumber_number(const Temporal *tnumber, Datum number,
  CachedType basetype)
{
  return boxop_tnumber_number(tnumber, number, basetype, &adjacent_tbox_tbox,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the range and the bounding box of the temporal number
 * are adjacent
 */
bool
adjacent_range_tnumber(const RangeType *range, const Temporal *tnumber)
{
  return boxop_tnumber_range(tnumber, range, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * the range are adjacent
 */
bool
adjacent_tnumber_range(const Temporal *tnumber, const RangeType *range)
{
  return boxop_tnumber_range(tnumber, range, &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the temporal box and the bounding box of the
 * temporal number are adjacent
 */
bool
adjacent_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of the temporal number and the
 * temporal box are adjacent
 */
bool
adjacent_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding boxes of the temporal numbers are
 * adjacent
 */
bool
adjacent_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &adjacent_tbox_tbox);
}
/*****************************************************************************/
