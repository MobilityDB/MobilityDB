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

/* MobilityDB */
#include <meos.h>
#include "general/temporal_boxops.h"

/*****************************************************************************
 * Bounding box operators for temporal types
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a timestamp contains the bounding period of a
 * temporal value
 * @sqlop @p \@>
 */
bool
contains_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &contains_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value contains a
 * timestamp
 * @sqlop @p \@>
 */
bool
contains_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a timestamp set contains the
 * one of a temporal value
 * @sqlop @p \@>
 */
bool
contains_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &contains_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value contains the
 * the one of a timestamp set
 * @sqlop @p \@>
 */
bool
contains_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &contains_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a period contains the bounding period of a
 * temporal value
 * @sqlop @p \@>
 */
bool
contains_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &contains_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value contains
 * a period
 * @sqlop @p \@>
 */
bool
contains_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a period set contains the
 * one of a temporal value
 * @sqlop @p \@>
 */
bool
contains_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &contains_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value contains
 * the one of a period set
 * @sqlop @p \@>
 */
bool
contains_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &contains_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the first temporal value
 * contains the one of the second temporal value.
 * @sqlop @p \@>
 */
bool
contains_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contains_span_span);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a timestamp is contained in the bounding period of
 * a temporal value
 * @sqlop @p <@
 */
bool
contained_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &contained_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value is contained
 * in a timestamp
 * @sqlop @p <@
 */
bool
contained_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a timestamp set is contained
 * in the one of a temporal value
 * @sqlop @p <@
 */
bool
contained_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &contained_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value is contained
 * in the one of a timestamp set
 * @sqlop @p <@
 */
bool
contained_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &contained_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a period is contained the bounding period of a
 * temporal value
 * @sqlop @p <@
 */
bool
contained_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &contained_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value is contained
 * in a period
 * @sqlop @p <@
 */
bool
contained_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a period set is contained
 * in the one of a temporal value
 * @sqlop @p <@
 */
bool
contained_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &contained_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value is contained
 * in the one of a period set
 * @sqlop @p <@
 */
bool
contained_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &contained_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of the first temporal value is
 * contained in the one of the second temporal value.
 * @sqlop @p <@
 */
bool
contained_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &contained_span_span);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a timestamp and the bounding period of a temporal
 * value overlap
 * @sqlop @p &&
 */
bool
overlaps_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &overlaps_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and a
 * timestamp overlap
 * @sqlop @p &&
 */
bool
overlaps_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a timestamp set and of
 * a temporal value overlap
 * @sqlop @p &&
 */
bool
overlaps_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &overlaps_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a temporal value and of
 * a timestamp set overlap
 * @sqlop @p &&
 */
bool
overlaps_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &overlaps_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a period and the bounding period of a temporal
 * value overlap
 * @sqlop @p &&
 */
bool
overlaps_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &overlaps_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and the
 * period overlap
 * @sqlop @p &&
 */
bool
overlaps_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a period set and of
 * a temporal value overlap
 * @sqlop @p &&
 */
bool
overlaps_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &overlaps_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a temporal value and of
 * a period set overlap
 * @sqlop @p &&
 */
bool
overlaps_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &overlaps_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values overlap
 * @sqlop @p &&
 */
bool
overlaps_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &overlaps_span_span);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a timestamp and the bounding period of a temporal
 * value are equal
 * @sqlop @p ~=
 */
bool
same_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &span_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and a
 * timestamp are equal
 * @sqlop @p ~=
 */
bool
same_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &span_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a timestamp set and of the
 * a temporal value are equal
 * @sqlop @p ~=
 */
bool
same_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &span_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a temporal value and of
 * a timestamp set are equal
 * @sqlop @p ~=
 */
bool
same_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &span_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a period and the bounding period of a temporal
 * value are equal
 * @sqlop @p ~=
 */
bool
same_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &span_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and the
 * period are equal
 * @sqlop @p ~=
 */
bool
same_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &span_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a period set and of
 * a temporal value are equal
 * @sqlop @p ~=
 */
bool
same_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &span_eq, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and
 * a period set are equal
 * @sqlop @p ~=
 */
bool
same_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &span_eq, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values are equal
 * @sqlop @p ~=
 */
bool
same_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &span_eq);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a timestamp and the bounding period of a temporal
 * value are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_timestamp_temporal(TimestampTz t, const Temporal *temp)
{
  return boxop_temporal_timestamp(temp, t, &adjacent_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and a
 * timestamp are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_temporal_timestamp(const Temporal *temp, TimestampTz t)
{
  return boxop_temporal_timestamp(temp, t, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a timestamp set and
 * a temporal value are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_timestampset_temporal(const TimestampSet *ts, const Temporal *temp)
{
  return boxop_temporal_timestampset(temp, ts, &adjacent_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of a temporal value and of
 * a timestamp set are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_temporal_timestampset(const Temporal *temp, const TimestampSet *ts)
{
  return boxop_temporal_timestampset(temp, ts, &adjacent_span_span,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a period and the bounding period of a temporal
 * value are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_period_temporal(const Period *p, const Temporal *temp)
{
  return boxop_temporal_period(temp, p, &adjacent_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and the
 * period are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_temporal_period(const Temporal *temp, const Period *p)
{
  return boxop_temporal_period(temp, p, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a period set and
 * a temporal value are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_periodset_temporal(const PeriodSet *ps, const Temporal *temp)
{
  return boxop_temporal_periodset(temp, ps, &adjacent_span_span, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding period of a temporal value and
 * a period set are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_temporal_periodset(const Temporal *temp, const PeriodSet *ps)
{
  return boxop_temporal_periodset(temp, ps, &adjacent_span_span, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding periods of the temporal values are
 * adjacent
 * @sqlop @p -|-
 */
bool
adjacent_temporal_temporal(const Temporal *temp1, const Temporal *temp2)
{
  return boxop_temporal_temporal(temp1, temp2, &adjacent_span_span);
}

/*****************************************************************************
 * Bounding box operators for temporal numbers
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer contains the bounding box of a temporal
 * integer
 * @sqlop @p \@>
 */
bool
contains_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float contains the bounding box of a temporal
 * float
 * @sqlop @p \@>
 */
bool
contains_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer contains an
 * integer
 * @sqlop @p \@>
 */
bool
contains_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float contains a
 * float
 * @sqlop @p \@>
 */
bool
contains_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a span contains the bounding box of a temporal
 * number
 * @sqlop @p \@>
 */
bool
contains_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number contains a
 * span
 * @sqlop @p \@>
 */
bool
contains_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &contains_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a temporal box contains the bounding box of a
 * temporal number
 * @sqlop @p \@>
 */
bool
contains_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contains_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number contains a
 * temporal box
 * @sqlop @p \@>
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
 * @sqlop @p \@>
 */
bool
contains_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &contains_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is contained in the bounding box of a
 * temporal integer
 * @sqlop @p <@
 */
bool
contained_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is contained in the bounding box of a temporal
 * float
 * @sqlop @p <@
 */
bool
contained_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is contained in
 * an integer
 * @sqlop @p <@
 */
bool
contained_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is contained in a
 * float
 * @sqlop @p <@
 */
bool
contained_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a span is contained in the bounding box of a
 * temporal number
 * @sqlop @p <@
 */
bool
contained_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number is contained
 * in a span
 * @sqlop @p <@
 */
bool
contained_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &contained_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a temporal box is contained in the bounding box of
 * a temporal number
 * @sqlop @p <@
 */
bool
contained_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &contained_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number is contained
 * in a temporal box
 * @sqlop @p <@
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
 * @sqlop @p <@
 */
bool
contained_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &contained_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer overlaps the bounding box of a
 * temporal integer
 * @sqlop @p &&
 */
bool
overlaps_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float overlaps the bounding box of a temporal
 * float
 * @sqlop @p &&
 */
bool
overlaps_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer overlaps
 * an integer
 * @sqlop @p &&
 */
bool
overlaps_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float overlaps a float
 * @sqlop @p &&
 */
bool
overlaps_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a span and the bounding box of a temporal number
 * overlap
 * @sqlop @p &&
 */
bool
overlaps_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and a span
 * overlap
 * @sqlop @p &&
 */
bool
overlaps_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a temporal box and the bounding box
 * of a temporal number overlap
 * @sqlop @p &&
 */
bool
overlaps_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overlaps_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and a
 * temporal box overlap
 * @sqlop @p &&
 */
bool
overlaps_tnumber_tbox(const Temporal *tnumber, const TBOX *tbox)
{
  return boxop_tnumber_tbox(tnumber, tbox, &overlaps_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding boxes of the temporal numbers overlap
 * @sqlop @p &&
 */
bool
overlaps_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &overlaps_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is the same as the bounding box of a
 * temporal integer
 * @sqlop @p ~=
 */
bool
same_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is the same as the bounding box of a temporal
 * float
 * @sqlop @p ~=
 */
bool
same_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is the same as
 * an integer
 * @sqlop @p ~=
 */
bool
same_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is the same as a
 * float
 * @sqlop @p ~=
 */
bool
same_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a span and the bounding box of a temporal number
 * are equal on the common dimensions
 * @sqlop @p ~=
 */
bool
same_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and a span
 * are equal on the common dimensions
 * @sqlop @p ~=
 */
bool
same_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &same_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a temporal box and the bounding box
 * of a temporal number are equal in the common dimensions
 * @sqlop @p ~=
 */
bool
same_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &same_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and a
 * temporal box are equal in the common dimensions
 * @sqlop @p ~=
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
 * @sqlop @p ~=
 */
bool
same_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &same_tbox_tbox);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if an integer is adjacent to the bounding box of a
 * temporal integer
 * @sqlop @p -|-
 */
bool
adjacent_int_tint(int i, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a float is adjacent to the bounding box of a temporal
 * float
 * @sqlop @p -|-
 */
bool
adjacent_float_tfloat(double d, const Temporal *tnumber)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal integer is adjacent to
 * an integer
 * @sqlop @p -|-
 */
bool
adjacent_tint_int(const Temporal *tnumber, int i)
{
  return boxop_tnumber_number(tnumber, Int32GetDatum(i), T_INT4,
    &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal float is adjacent to a
 * float
 * @sqlop @p -|-
 */
bool
adjacent_tfloat_float(const Temporal *tnumber, double d)
{
  return boxop_tnumber_number(tnumber, Float8GetDatum(d), T_FLOAT8,
    &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a span and the bounding box of a temporal number
 * are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_span_tnumber(const Span *span, const Temporal *tnumber)
{
  return boxop_tnumber_span(tnumber, span, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and the
 * a span are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_tnumber_span(const Temporal *tnumber, const Span *span)
{
  return boxop_tnumber_span(tnumber, span, &adjacent_tbox_tbox, INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if a temporal box and the bounding box of a
 * temporal number are adjacent
 * @sqlop @p -|-
 */
bool
adjacent_tbox_tnumber(const TBOX *tbox, const Temporal *tnumber)
{
  return boxop_tnumber_tbox(tnumber, tbox, &adjacent_tbox_tbox, INVERT);
}

/**
 * @ingroup libmeos_temporal_topo
 * @brief Return true if the bounding box of a temporal number and a
 * temporal box are adjacent
 * @sqlop @p -|-
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
 * @sqlop @p -|-
 */
bool
adjacent_tnumber_tnumber(const Temporal *tnumber1, const Temporal *tnumber2)
{
  return boxop_tnumber_tnumber(tnumber1, tnumber2, &adjacent_tbox_tbox);
}
/*****************************************************************************/
