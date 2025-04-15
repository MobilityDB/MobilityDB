/***********************************************************************
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
 * @brief Bin and tile functions for span types
 * @note The time bin functions are inspired from TimescaleDB
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "general/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/date.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/spanset.h"
#include "general/temporal_restrict.h"
#include "general/tsequence.h"
#include "general/type_util.h"

/*****************************************************************************
 * Bins functions
 *****************************************************************************/

/**
 * @brief Return the bins from a span
 * @param[in] s Input span to split
 * @param[in] size Bin size
 * @param[in] origin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
static Span *
span_bins(const Span *s, Datum size, Datum origin, int *count)
{
  assert(s); assert(count); assert(positive_datum(size, s->basetype));

  SpanBinState *state = span_bin_state_make(NULL, s, size, origin);
  Span *bins = palloc0(sizeof(Span) * state->nbins);
  for (int i = 0; i < state->nbins; i++)
  {
    span_bin_state_set(state->value, state->size, s->basetype, s->spantype,
      &bins[i]);
    span_bin_state_next(state);
  }
  *count = state->nbins;
  pfree(state);
  return bins;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of an integer span
 * @param[in] s Input span to split
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
intspan_bins(const Span *s, int size, int origin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPAN(s, NULL); VALIDATE_NOT_NULL(count, NULL);
  return span_bins(s, Int32GetDatum(size), Int32GetDatum(origin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a float span
 * @param[in] s Input span to split
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
floatspan_bins(const Span *s, double size, double origin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPAN(s, NULL); VALIDATE_NOT_NULL(count, NULL);
  return span_bins(s, Float8GetDatum(size), Float8GetDatum(origin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a date span
 * @param[in] s Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] origin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
datespan_bins(const Span *s, const Interval *duration, DateADT origin,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPAN(s, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_valid_day_duration(duration))
    return NULL;

  int32 days = (int32) (interval_units(duration) / USECS_PER_DAY);
  return span_bins(s, Int32GetDatum(days), DateADTGetDatum(origin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a timestamptz span
 * @param[in] s Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] origin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
tstzspan_bins(const Span *s, const Interval *duration, TimestampTz origin,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPAN(s, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  if (! ensure_valid_duration(duration))
    return NULL;

  int64 tunits = interval_units(duration);
  return span_bins(s, Int64GetDatum(tunits), TimestampTzGetDatum(origin),
    count);
}

/*****************************************************************************
 * Spans functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time bins of a span set
 * @param[in] ss Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #spanset_time_bin_init
 */
Span *
datespanset_time_spans(const SpanSet *ss, const Interval *duration,
  DateADT torigin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_DATESPANSET(ss, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  return spanset_time_spans(ss, duration, DateADTGetDatum(torigin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time bins of a span set
 * @param[in] ss Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #spanset_time_bin_init
 */
Span *
tstzspanset_time_spans(const SpanSet *ss, const Interval *duration,
  TimestampTz torigin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TSTZSPANSET(ss, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(count, NULL);
  return spanset_time_spans(ss, duration, TimestampTzGetDatum(torigin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of an integer span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
intspanset_value_spans(const SpanSet *ss, int vsize, int vorigin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_INTSPANSET(ss, NULL); VALIDATE_NOT_NULL(count, NULL);
  return spanset_value_spans(ss, Int32GetDatum(vsize), Int32GetDatum(vorigin),
    count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a bigint span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
bigintspanset_value_spans(const SpanSet *ss, int64 vsize, int64 vorigin,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_BIGINTSPANSET(ss, NULL); VALIDATE_NOT_NULL(count, NULL);
  return spanset_value_spans(ss, Int64GetDatum(vsize), Int64GetDatum(vorigin),
    count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a float span set
 * @param[in] ss SpanSet number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
floatspanset_value_spans(const SpanSet *ss, double vsize, double vorigin,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_FLOATSPANSET(ss, NULL); VALIDATE_NOT_NULL(count, NULL);
  return spanset_value_spans(ss, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of an integer span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
tint_value_spans(const Temporal *temp, int vsize, int vorigin, int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TINT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_spans(temp, Int32GetDatum(vsize),
    Int32GetDatum(vorigin), count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bins of a float span
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 */
Span *
tfloat_value_spans(const Temporal *temp, double vsize, double vorigin,
  int *count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TFLOAT(temp, NULL); VALIDATE_NOT_NULL(count, NULL);
  return tnumber_value_spans(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), count);
}

/*****************************************************************************/
