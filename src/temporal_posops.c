/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Relative position operators for temporal types.
 *
 * The following operators are defined: 
 * - `left`, `overleft`, `right`, `overright` for the value dimension
 * - `before`, `overbefore`, `after`, `overafter`for the time dimension
 */

#include "temporal_posops.h"

#include <assert.h>

#include "timeops.h"
#include "temporal.h"
#include "temporal_boxops.h"

/*****************************************************************************/
/* Period op Temporal */

PG_FUNCTION_INFO_V1(before_period_temporal);
/**
 * Returns true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
before_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_period_temporal);
/**
 * Returns true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
overbefore_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_period_temporal);
/**
 * Returns true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
after_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_period_temporal);
/**
 * Returns true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
overafter_period_temporal(PG_FUNCTION_ARGS)
{
  return boxop_period_temporal(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************/
/* Temporal op Period */

PG_FUNCTION_INFO_V1(before_temporal_period);
/**
 * Returns true if the temporal value is strictly before the period value
 */
PGDLLEXPORT Datum
before_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_period);
/**
 * Returns true if the temporal value is not after the period value
 */
PGDLLEXPORT Datum
overbefore_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_temporal_period);
/**
 * Returns true if the temporal value is strictly after the period value
 */
PGDLLEXPORT Datum
after_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_temporal_period);
/**
 * Returns true if the temporal value is not before the period value
 */
PGDLLEXPORT Datum
overafter_temporal_period(PG_FUNCTION_ARGS)
{
  return boxop_temporal_period(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(before_temporal_temporal);
/**
 * Returns true if the first temporal value is strictly before the second one
 */
PGDLLEXPORT Datum
before_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_temporal);
/**
 * Returns true if the first temporal value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_temporal_temporal);
/**
 * Returns true if the first temporal value is strictly after the second one
 */
PGDLLEXPORT Datum
after_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_temporal_temporal);
/**
 * Returns true if the first temporal value is not before the second one
 */
PGDLLEXPORT Datum
overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
  return boxop_temporal_temporal(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************/
/* Range op Tnumber */

PG_FUNCTION_INFO_V1(left_range_tnumber);
/**
 * Returns true if the number range value is strictly to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
left_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_range_tnumber);
/**
 * Returns true if the number range value is not to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
overleft_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_range_tnumber);
/**
 * Returns true if the number range value is strictly to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
right_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_range_tnumber);
/**
 * Returns true if the number range value is not to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
overright_range_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_range_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

/*****************************************************************************/
/* Tnumber op Range */

PG_FUNCTION_INFO_V1(left_tnumber_range);
/**
 * Returns true if the temporal number is strictly to the left of the
 * number range value
 */
PGDLLEXPORT Datum
left_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_range);
/**
 * Returns true if the temporal number is not to the right of the
 * number range value
 */
PGDLLEXPORT Datum
overleft_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_range);
/**
 * Returns true if the temporal number is strictly to the right of the
 * number range value
 */
PGDLLEXPORT Datum
right_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_range);
/**
 * Returns true if the temporal number is not to the left of the
 * number range value
 */
PGDLLEXPORT Datum
overright_tnumber_range(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_range(fcinfo, &overright_tbox_tbox_internal);
}

/*****************************************************************************/
/* TBOX op Temporal */

PG_FUNCTION_INFO_V1(left_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
left_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tbox_tnumber);
/**
 * Returns true if the temporal box value is not to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly to the right of the
 * temporal number
 */
PGDLLEXPORT Datum
right_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tbox_tnumber);
/**
 * Returns true if the temporal box value is not to the left of the
 * temporal number
 */
PGDLLEXPORT Datum
overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly before the
 * temporal number
 */
PGDLLEXPORT Datum
before_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tbox_tnumber);
/**
 * Returns true if the temporal box value is not after the
 * temporal number
 */
PGDLLEXPORT Datum
overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly after the
 * temporal number
 */
PGDLLEXPORT Datum
after_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tbox_tnumber);
/**
 * Returns true if the temporal box value is not before the
 * temporal number
 */
PGDLLEXPORT Datum
overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tbox_tnumber(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
/* Temporal op TBOX */

PG_FUNCTION_INFO_V1(left_tnumber_tbox);
/**
 * Returns true if the temporal number is strictly to the left of
 * the temporal box value
 */
PGDLLEXPORT Datum
left_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_tbox);
/**
 * Returns true if the temporal number is not to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_tbox);
/**
 * Returns true if the temporal number is strictly to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
right_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_tbox);
/**
 * Returns true if the temporal number is not to the left of the
 * temporal box value
 */
PGDLLEXPORT Datum
overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tnumber_tbox);
/**
 * Returns true if the temporal number is strictly before the
 * temporal box value
 */
PGDLLEXPORT Datum
before_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_tbox);
/**
 * Returns true if the temporal number is not after the
 * temporal box value
 */
PGDLLEXPORT Datum
overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tnumber_tbox);
/**
 * Returns true if the temporal number is strictly after the
 * temporal box value
 */
PGDLLEXPORT Datum
after_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_tbox);
/**
 * Returns true if the temporal number is not before the
 * temporal box value
 */
PGDLLEXPORT Datum
overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tbox(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

PG_FUNCTION_INFO_V1(left_tnumber_tnumber);
/**
 * Returns true if the first temporal number is strictly to the left of
 * the second one
 */
PGDLLEXPORT Datum
left_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_tnumber);
/**
 * Returns true if the first temporal number is not to the right of
 * the second one
 */
PGDLLEXPORT Datum
overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_tnumber);
/**
 * Returns true if the first temporal number is strictly to the right of
 * the second one
 */
PGDLLEXPORT Datum
right_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_tnumber);
/**
 * Returns true if the first temporal number is not to the left of
 * the second one
 */
PGDLLEXPORT Datum
overright_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tnumber_tnumber);
/**
 * Returns true if the first temporal number is strictly before
 * the second one
 */
PGDLLEXPORT Datum
before_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_tnumber);
/**
 * Returns true if the first temporal number is not after
 * the second one
 */
PGDLLEXPORT Datum
overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tnumber_tnumber);
/**
 * Returns true if the first temporal number is strictly after
 * the second one
 */
PGDLLEXPORT Datum
after_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_tnumber);
/**
 * Returns true if the first temporal number is not before
 * the second one
 */
PGDLLEXPORT Datum
overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
{
  return boxop_tnumber_tnumber(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
