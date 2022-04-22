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

PG_FUNCTION_INFO_V1(Before_timestamp_temporal);
/**
 * Return true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
Before_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_timestamp_temporal);
/**
 * Return true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
Overbefore_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_timestamp_temporal);
/**
 * Return true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
After_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_timestamp_temporal);
/**
 * Return true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
Overafter_timestamp_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestamp_temporal_ext(fcinfo, &overafter_period_period);
}

PG_FUNCTION_INFO_V1(Before_timestampset_temporal);
/**
 * Return true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
Before_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_temporal);
/**
 * Return true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
Overbefore_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_timestampset_temporal);
/**
 * Return true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
After_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_temporal);
/**
 * Return true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
Overafter_timestampset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_timestampset_temporal_ext(fcinfo, &overafter_period_period);
}

PG_FUNCTION_INFO_V1(Before_period_temporal);
/**
 * Return true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
Before_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_period_temporal);
/**
 * Return true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
Overbefore_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_period_temporal);
/**
 * Return true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
After_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_period_temporal);
/**
 * Return true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
Overafter_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal_ext(fcinfo, &overafter_period_period);
}

PG_FUNCTION_INFO_V1(Before_periodset_temporal);
/**
 * Return true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
Before_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_temporal);
/**
 * Return true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
Overbefore_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_periodset_temporal);
/**
 * Return true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
After_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_temporal);
/**
 * Return true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
Overafter_periodset_temporal(PG_FUNCTION_ARGS)
{
  return boxop_periodset_temporal_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Temporal op Timestamptz */

PG_FUNCTION_INFO_V1(Before_temporal_timestamp);
/**
 * Return true if the temporal value is strictly before the timestamptz value
 */
PGDLLEXPORT Datum
Before_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_temporal_timestamp);
/**
 * Return true if the temporal value is not after the timestamptz value
 */
PGDLLEXPORT Datum
Overbefore_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_temporal_timestamp);
/**
 * Return true if the temporal value is strictly after the timestamptz value
 */
PGDLLEXPORT Datum
After_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_temporal_timestamp);
/**
 * Return true if the temporal value is not before the timestamptz value
 */
PGDLLEXPORT Datum
Overafter_temporal_timestamp(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestamp_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Temporal op Timestampset */

PG_FUNCTION_INFO_V1(Before_temporal_timestampset);
/**
 * Return true if the temporal value is strictly before the timestampset value
 */
PGDLLEXPORT Datum
Before_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_temporal_timestampset);
/**
 * Return true if the temporal value is not after the timestampset value
 */
PGDLLEXPORT Datum
Overbefore_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_temporal_timestampset);
/**
 * Return true if the temporal value is strictly after the timestampset value
 */
PGDLLEXPORT Datum
After_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_temporal_timestampset);
/**
 * Return true if the temporal value is not before the timestampset value
 */
PGDLLEXPORT Datum
Overafter_temporal_timestampset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_timestampset_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Temporal op Period */

PG_FUNCTION_INFO_V1(Before_temporal_period);
/**
 * Return true if the temporal value is strictly before the period value
 */
PGDLLEXPORT Datum
Before_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_temporal_period);
/**
 * Return true if the temporal value is not after the period value
 */
PGDLLEXPORT Datum
Overbefore_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_temporal_period);
/**
 * Return true if the temporal value is strictly after the period value
 */
PGDLLEXPORT Datum
After_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_temporal_period);
/**
 * Return true if the temporal value is not before the period value
 */
PGDLLEXPORT Datum
Overafter_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Temporal op PeriodSet */

PG_FUNCTION_INFO_V1(Before_temporal_periodset);
/**
 * Return true if the temporal value is strictly before the periodset value
 */
PGDLLEXPORT Datum
Before_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_temporal_periodset);
/**
 * Return true if the temporal value is not after the periodset value
 */
PGDLLEXPORT Datum
Overbefore_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_temporal_periodset);
/**
 * Return true if the temporal value is strictly after the periodset value
 */
PGDLLEXPORT Datum
After_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_temporal_periodset);
/**
 * Return true if the temporal value is not before the periodset value
 */
PGDLLEXPORT Datum
Overafter_temporal_periodset(PG_FUNCTION_ARGS)
{
  return boxop_temporal_periodset_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(Before_temporal_temporal);
/**
 * Return true if the first temporal value is strictly before the second one
 */
PGDLLEXPORT Datum
Before_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &before_period_period);
}

PG_FUNCTION_INFO_V1(Overbefore_temporal_temporal);
/**
 * Return true if the first temporal value is not after the second one
 */
PGDLLEXPORT Datum
Overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &overbefore_period_period);
}

PG_FUNCTION_INFO_V1(After_temporal_temporal);
/**
 * Return true if the first temporal value is strictly after the second one
 */
PGDLLEXPORT Datum
After_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &after_period_period);
}

PG_FUNCTION_INFO_V1(Overafter_temporal_temporal);
/**
 * Return true if the first temporal value is not before the second one
 */
PGDLLEXPORT Datum
Overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal_ext(fcinfo, &overafter_period_period);
}

/*****************************************************************************/
/* Number op Tnumber */

PG_FUNCTION_INFO_V1(Left_number_tnumber);
/**
 * Return true if the number value is strictly to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Left_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_number_tnumber);
/**
 * Return true if the number value is not to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Overleft_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_number_tnumber);
/**
 * Return true if the number value is strictly to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Right_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_number_tnumber);
/**
 * Return true if the number value is not to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Overright_number_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_number_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

/*****************************************************************************/
/* Range op Tnumber */

PG_FUNCTION_INFO_V1(Left_range_tnumber);
/**
 * Return true if the number range value is strictly to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Left_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_range_tnumber);
/**
 * Return true if the number range value is not to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Overleft_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_range_tnumber);
/**
 * Return true if the number range value is strictly to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Right_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_range_tnumber);
/**
 * Return true if the number range value is not to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Overright_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

/*****************************************************************************/
/* Tnumber op Number */

PG_FUNCTION_INFO_V1(Left_tnumber_number);
/**
 * Return true if the temporal number is strictly to the left of the
 * number value
 */
PGDLLEXPORT Datum
Left_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnumber_number);
/**
 * Return true if the temporal number is not to the right of the
 * number value
 */
PGDLLEXPORT Datum
Overleft_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_tnumber_number);
/**
 * Return true if the temporal number is strictly to the right of the
 * number value
 */
PGDLLEXPORT Datum
Right_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_tnumber_number);
/**
 * Return true if the temporal number is not to the left of the
 * number value
 */
PGDLLEXPORT Datum
Overright_tnumber_number(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_number_ext(fcinfo, &overright_tbox_tbox);
}

/*****************************************************************************/
/* Tnumber op Range */

PG_FUNCTION_INFO_V1(Left_tnumber_range);
/**
 * Return true if the temporal number is strictly to the left of the
 * number range value
 */
PGDLLEXPORT Datum
Left_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnumber_range);
/**
 * Return true if the temporal number is not to the right of the
 * number range value
 */
PGDLLEXPORT Datum
Overleft_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_tnumber_range);
/**
 * Return true if the temporal number is strictly to the right of the
 * number range value
 */
PGDLLEXPORT Datum
Right_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_tnumber_range);
/**
 * Return true if the temporal number is not to the left of the
 * number range value
 */
PGDLLEXPORT Datum
Overright_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range_ext(fcinfo, &overright_tbox_tbox);
}

/*****************************************************************************/
/* TBOX op Temporal */

PG_FUNCTION_INFO_V1(Left_tbox_tnumber);
/**
 * Return true if the temporal box value is strictly to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Left_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_tbox_tnumber);
/**
 * Return true if the temporal box value is not to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_tbox_tnumber);
/**
 * Return true if the temporal box value is strictly to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
Right_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_tbox_tnumber);
/**
 * Return true if the temporal box value is not to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
Overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Before_tbox_tnumber);
/**
 * Return true if the temporal box value is strictly before the
 * temporal number
 */
PGDLLEXPORT Datum
Before_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &before_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tbox_tnumber);
/**
 * Return true if the temporal box value is not after the
 * temporal number
 */
PGDLLEXPORT Datum
Overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overbefore_tbox_tbox);
}

PG_FUNCTION_INFO_V1(After_tbox_tnumber);
/**
 * Return true if the temporal box value is strictly after the
 * temporal number
 */
PGDLLEXPORT Datum
After_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &after_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overafter_tbox_tnumber);
/**
 * Return true if the temporal box value is not before the
 * temporal number
 */
PGDLLEXPORT Datum
Overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Temporal op TBOX */

PG_FUNCTION_INFO_V1(Left_tnumber_tbox);
/**
 * Return true if the temporal number is strictly to the left of
 * the temporal box value
 */
PGDLLEXPORT Datum
Left_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnumber_tbox);
/**
 * Return true if the temporal number is not to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
Overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_tnumber_tbox);
/**
 * Return true if the temporal number is strictly to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
Right_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_tnumber_tbox);
/**
 * Return true if the temporal number is not to the left of the
 * temporal box value
 */
PGDLLEXPORT Datum
Overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overright_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Before_tnumber_tbox);
/**
 * Return true if the temporal number is strictly before the
 * temporal box value
 */
PGDLLEXPORT Datum
Before_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &before_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnumber_tbox);
/**
 * Return true if the temporal number is not after the
 * temporal box value
 */
PGDLLEXPORT Datum
Overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overbefore_tbox_tbox);
}

PG_FUNCTION_INFO_V1(After_tnumber_tbox);
/**
 * Return true if the temporal number is strictly after the
 * temporal box value
 */
PGDLLEXPORT Datum
After_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &after_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnumber_tbox);
/**
 * Return true if the temporal number is not before the
 * temporal box value
 */
PGDLLEXPORT Datum
Overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

PG_FUNCTION_INFO_V1(Left_tnumber_tnumber);
/**
 * Return true if the first temporal number is strictly to the left of
 * the second one
 */
PGDLLEXPORT Datum
Left_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &left_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overleft_tnumber_tnumber);
/**
 * Return true if the first temporal number is not to the right of
 * the second one
 */
PGDLLEXPORT Datum
Overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overleft_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Right_tnumber_tnumber);
/**
 * Return true if the first temporal number is strictly to the right of
 * the second one
 */
PGDLLEXPORT Datum
Right_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &right_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overright_tnumber_tnumber);
/**
 * Return true if the first temporal number is not to the left of
 * the second one
 */
PGDLLEXPORT Datum
Overright_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overright_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Before_tnumber_tnumber);
/**
 * Return true if the first temporal number is strictly before
 * the second one
 */
PGDLLEXPORT Datum
Before_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &before_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overbefore_tnumber_tnumber);
/**
 * Return true if the first temporal number is not after
 * the second one
 */
PGDLLEXPORT Datum
Overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overbefore_tbox_tbox);
}

PG_FUNCTION_INFO_V1(After_tnumber_tnumber);
/**
 * Return true if the first temporal number is strictly after
 * the second one
 */
PGDLLEXPORT Datum
After_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &after_tbox_tbox);
}

PG_FUNCTION_INFO_V1(Overafter_tnumber_tnumber);
/**
 * Return true if the first temporal number is not before
 * the second one
 */
PGDLLEXPORT Datum
Overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber_ext(fcinfo, &overafter_tbox_tbox);
}

/*****************************************************************************/
