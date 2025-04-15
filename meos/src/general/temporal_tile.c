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
 * @brief Bin and tile functions for temporal types
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
 * Bin functions for the various span base types
 *****************************************************************************/

/**
 * @brief Return the initial value of the bin that contains an integer
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @return On error return @p INT_MAX
 */
int
int_get_bin(int value, int size, int origin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive(size))
    return INT_MAX;

  if (origin != 0)
  {
    /*
     * We need to ensure that the value is in span AFTER the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    if ((origin > 0 && value < PG_INT32_MIN + origin) ||
        (origin < 0 && value > PG_INT32_MAX + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT_MAX;
    }
    value -= origin;
  }
  int result = (value / size) * size;
  if (value < 0 && value % size)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < PG_INT32_MIN + size)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT_MAX;
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @brief Return the initial value of the bin that contains an integer
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @return On error return @p INT_MAX
 */
int64
bigint_get_bin(int64 value, int64 size, int64 origin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive_datum(size, T_INT8))
    return INT64_MAX;

  if (origin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    if ((origin > 0 && value < PG_INT64_MIN + origin) ||
        (origin < 0 && value > PG_INT64_MAX + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT64_MAX;
    }
    value -= origin;
  }
  int64 result = (value / size) * size;
  if (value < 0 && value % size)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < PG_INT64_MIN + size)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return INT64_MAX;
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @brief Return the initial value of the bin that contains a float
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @return On error return @p DBL_MAX
 */
double
float_get_bin(double value, double size, double origin)
{
  /* Ensure the validity of the arguments */
  if (! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return DBL_MAX;

  if (origin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum double value (-1 * DBL_MAX) and when
     * negative that it is less than the maximum double value (DBL_MAX)
     */
    if ((origin > 0 && value < -1 * DBL_MAX + origin) ||
        (origin < 0 && value > DBL_MAX + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return DBL_MAX;
    }
    value -= origin;
  }
  /* Notice that by using the floor function above we remove the need to
   * add the additional if needed for the integer case to take into account
   * that integer division truncates toward 0 in C99 */
  double result = floor(value / size) * size;
  result += origin;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the interval in the same representation as Postgres timestamps
 */
inline int64
interval_units(const Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

/**
 * @brief To bin by day we get the year and month of a date and convert
 * that to the nth month since origin. This allows us to treat month bining
 * similar to int bining. During this process we ignore the day component and
 * therefore only support bining by full months.
 */
static DateADT
date_get_bin_int(DateADT d, int32 ndays, DateADT origin)
{
  /* In PostgreSQL DateADT is defined as a typedef of int32 */
  return (DateADT) int_get_bin((int) d, (int) ndays, (int) origin);
}

/**
 * @brief Return the initial date of the bin that contains a date
 * @param[in] d Input date
 * @param[in] duration Interval defining the size of the bins
 * @param[in] origin Origin of the bins
 * @return On error return @p DATEVAL_NOEND
 */
DateADT
date_get_bin(DateADT d, const Interval *duration, DateADT origin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, DATEVAL_NOEND);
  if (! ensure_valid_day_duration(duration))
    return DATEVAL_NOEND;

  if (DATE_NOT_FINITE(d))
    return d;

  int32 ndays = interval_units(duration) / USECS_PER_DAY;
  return date_get_bin_int(d, ndays, origin);
}

/**
 * @brief Return the initial timestamp of the bin that contains a timestamptz
 * (internal function)
 * @param[in] t Input timestamp
 * @param[in] size Size of the time bins in PostgreSQL time units
 * @param[in] origin Origin of the bins
 * @return On error return DT_NOEND
 */
TimestampTz
timestamptz_get_bin_int(TimestampTz t, int64 size, TimestampTz origin)
{
  if (TIMESTAMP_NOT_FINITE(t))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
    return DT_NOEND;
  }
  if (origin != 0)
  {
    /*
     * We need to ensure that the timestamp is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * time is at least the minimum time value (DT_NOBEGIN) and when negative,
     * that it is less than the maximum time value (DT_NOEND)
     */
    if ((origin > 0 && t < DT_NOBEGIN + origin) ||
        (origin < 0 && t > DT_NOEND + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
      return DT_NOEND;
    }

    t -= origin;
  }
  TimestampTz result = (t / size) * size;
  if (t < 0 && t % size)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if timestamp is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < DT_NOBEGIN + size)
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of span");
      return DT_NOEND;
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @brief Return the initial timestamp of the bin that contains a timestamptz
 * @param[in] t Input timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] origin Origin of the bins
 */
TimestampTz
timestamptz_get_bin(TimestampTz t, const Interval *duration,
  TimestampTz origin)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(duration, DT_NOEND);
  if (! ensure_valid_duration(duration))
    return DT_NOEND;
  int64 size = interval_units(duration);
  return timestamptz_get_bin_int(t, size, origin);
}

/*****************************************************************************
 * Span bin functions
 *****************************************************************************/

/**
 * @brief Return the initial value of the bin that contains a number value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Data type of the arguments
 * @pre When called for dates, this function assumes that the duration interval
 * which the calling function translated into the argument `size` does NOT
 * have a month component
 */
Datum
datum_bin(Datum value, Datum size, Datum origin, meosType type)
{
  /* This function is called directly by the MobilityDB API */
  if (! ensure_positive_datum(size, type))
    return 0;

  assert(span_basetype(type));
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(int_get_bin(DatumGetInt32(value),
        DatumGetInt32(size), DatumGetInt32(origin)));
    case T_INT8:
      return Int64GetDatum(bigint_get_bin(DatumGetInt64(value),
        DatumGetInt64(size), DatumGetInt64(origin)));
    case T_FLOAT8:
      return Float8GetDatum(float_get_bin(DatumGetFloat8(value),
        DatumGetFloat8(size), DatumGetFloat8(origin)));
    case T_DATE:
      return DateADTGetDatum(date_get_bin_int(DatumGetDateADT(value),
          DatumGetInt32(size), DatumGetDateADT(origin)));
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(timestamptz_get_bin_int(
        DatumGetTimestampTz(value), DatumGetInt64(size),
        DatumGetTimestampTz(origin)));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unknown bin function for type: %s", meostype_name(type));
      return 0;
  }
}

/**
 * @brief Get the time bins of a temporal value
 * @param[in] s Span to tile
 * @param[in] size Size of the bins
 * @param[in] origin Time origin of the tiles
 * @param[out] start_bin,end_bin Values of the start and end bins
 * @return Number of bins
 * @pre When called for dates, this function assumes that the duration interval
 * which the calling function translated into the argument `size` does NOT
 * have a month component
 */
int
span_no_bins(const Span *s, Datum size, Datum origin, Datum *start_bin,
  Datum *end_bin)
{
  assert(s); assert(start_bin); assert(end_bin);

  Datum start_value = s->lower;
  /* We need to add size to obtain the end value of the last bin */
  Datum end_value = datum_add(s->upper, size, s->basetype);
  *start_bin = datum_bin(start_value, size, origin, s->basetype);
  *end_bin = datum_bin(end_value, size, origin, s->basetype);
  switch (s->basetype)
  {
    case T_INT4:
      return (DatumGetInt32(*end_bin) - DatumGetInt32(*start_bin)) /
        DatumGetInt32(size);
    case T_INT8:
      return (DatumGetInt64(*end_bin) - DatumGetInt64(*start_bin)) /
        DatumGetInt64(size);
    case T_FLOAT8:
      return (int) floor((DatumGetFloat8(*end_bin) -
        DatumGetFloat8(*start_bin)) / DatumGetFloat8(size));
    case T_DATE:
      return (DatumGetDateADT(*end_bin) - DatumGetDateADT(*start_bin)) /
          DatumGetInt32(size);
    case T_TIMESTAMPTZ:
      return (DatumGetInt64(*end_bin) - DatumGetInt64(*start_bin)) /
        DatumGetInt64(size);
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unknown number of bins function for type: %s",
        meostype_name(s->basetype));
      return 0;
  }
}

/*****************************************************************************/

/**
 * @brief Create the initial state for tiling operations
 * @param[in] to_split Value to split, currently either a spanset or a temporal
 * value, may be @p NULL
 * @param[in] s Bounds for generating the bins
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @note The first argument is NULL when generating the bins, otherwise
 * it is a spanset or a temporal value to be split and in this case is the
 * bounding span of the value to split
 */
SpanBinState *
span_bin_state_make(const void *to_split, const Span *s, Datum size,
  Datum origin)
{
  assert(s); assert(positive_datum(size, s->basetype));

  /* Use palloc0 for initialization */
  SpanBinState *state = palloc0(sizeof(SpanBinState));
  /* Fill in state */
  state->done = false;
  state->basetype = s->basetype;
  state->i = 1;
  state->size = size;
  state->origin = origin;
  /* Get the span bounds of the state */
  Datum start_bin, end_bin;
  state->nbins = span_no_bins(s, size, origin, &start_bin, &end_bin);
  /* Set the span of the state */
  span_set(start_bin, end_bin, true, false, s->basetype, s->spantype,
    &state->span);
  state->value = start_bin;
  state->to_split = to_split;
  return state;
}

/**
 * @brief Generate an integer or float span bin from a bin list
 * @param[in] lower Start value of the bin
 * @param[in] size Size of the bins
 * @param[in] basetype Type of the arguments
 * @param[in] spantype Span type of the arguments
 * @param[out] span Output span
 */
void
span_bin_state_set(Datum lower, Datum size, meosType basetype,
  meosType spantype, Span *span)
{
  assert(span);

  Datum upper = datum_add(lower, size, basetype);
  span_set(lower, upper, true, false, basetype, spantype, span);
  return;
}

/**
 * @brief Get the current bin of the bins
 * @param[in] state State to increment
 * @param[out] span Current bin
 */
bool
span_bin_state_get(SpanBinState *state, Span *span)
{
  if (! state || state->done)
    return false;
  /* Get the box of the current tile */
  span_bin_state_set(state->value, state->size, state->span.basetype,
    state->span.spantype, span);
  return true;
}

/**
 * @brief Increment the current state to the next bin of the bins
 * @param[in] state State to increment
 */
void
span_bin_state_next(SpanBinState *state)
{
  if (! state || state->done)
    return;
  /* Move to the next bin */
  state->i++;
  state->value = datum_add(state->value, state->size, state->basetype);
  if (state->i > state->nbins)
    state->done = true;
  return;
}

/*****************************************************************************/

/**
 * @brief Set the state with a spanset and a time bin for splitting
 * or obtaining a set of spans
 * @param[in] ss SpanSet value
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] torigin Origin for the time dimension, may be a Date or a
 * TimestampTz
 * @param[out] nbins Number of bins
 */
SpanBinState *
spanset_time_bin_init(const SpanSet *ss, const Interval *duration,
  Datum torigin, int *nbins)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(nbins, NULL);
  if ((ss->basetype != T_DATE && ! ensure_valid_duration(duration)) ||
      (ss->basetype == T_DATE && ! ensure_valid_day_duration(duration)))
    return NULL;

  /* Create function state */
  SpanBinState *state;
  if (ss->basetype == T_DATE)
  {
    int32 days = (int32) (interval_units(duration) / USECS_PER_DAY);
    state = span_bin_state_make((const void *) ss, &ss->span,
      Int32GetDatum(days), torigin);
  }
  else
  {
    int64 tunits = interval_units(duration);
    state = span_bin_state_make((const void *) ss, &ss->span,
      Int64GetDatum(tunits), torigin);
  }
  *nbins = state->nbins;
  return state;
}

/**
 * @brief Set the state with a spanset and a time bin for splitting
 * or obtaining a set of spans
 * @param[in] ss SpanSet value
 * @param[in] vsize Size of the value dimension
 * @param[in] vorigin Origin for the value dimension
 * @param[out] nbins Number of bins
 */
SpanBinState *
spanset_value_bin_init(const SpanSet *ss, Datum vsize, Datum vorigin,
  int *nbins)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(nbins, NULL);
  if (! ensure_positive_datum(vsize, ss->basetype))
    return NULL;

  /* Create function state */
  SpanBinState *state = span_bin_state_make((const void *) ss, &ss->span,
    vsize, vorigin);
  *nbins = state->nbins;
  return state;
}

/*****************************************************************************/

/**
 * @brief Set the state with a temporal value and a time bin for splitting
 * or obtaining a set of spans
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] torigin Origin for the time dimension
 * @param[out] nbins Number of bins
 */
SpanBinState *
temporal_time_bin_init(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, int *nbins)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(duration, NULL);
  VALIDATE_NOT_NULL(nbins, NULL);
  if (! ensure_valid_duration(duration))
    return NULL;

  /* Set bounding box */
  Span bounds;
  temporal_set_tstzspan(temp, &bounds);
  /* Create function state */
  int64 tunits = interval_units(duration);
  SpanBinState *state = span_bin_state_make((const void *) temp, &bounds,
    tunits, torigin);
  *nbins = state->nbins;
  return state;
}

/**
 * @brief Set the state with a temporal value and a time bin for splitting
 * or obtaining a set of spans
 * @param[in] temp Temporal value
 * @param[in] vsize Size of the value dimension
 * @param[in] vorigin Origin for the value dimension
 * @param[out] nbins Number of bins
 */
SpanBinState *
tnumber_value_bin_init(const Temporal *temp, Datum vsize, Datum vorigin,
  int *nbins)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL); VALIDATE_NOT_NULL(nbins, NULL);
  if (! ensure_positive_datum(vsize, temptype_basetype(temp->temptype)))
    return NULL;

  /* Set bounding box */
  Span bounds;
  tnumber_set_span(temp, &bounds);
  /* Create function state */
  SpanBinState *state = span_bin_state_make((const void *) temp, &bounds,
    vsize, vorigin);
  *nbins = state->nbins;
  return state;
}

/*****************************************************************************
 * Spans functions
 *****************************************************************************/

/**
 * @brief Function with common functionality for functions
 * #spanset_value_spans and #spanset_time_spans
 * @param[in] ss Input span to split
 * @param[in] state State
 * @param[out] count Number of elements in the output array
 */
static Span *
spanset_spans_common(const SpanSet *ss, SpanBinState *state, int *count)
{
  assert(ss); assert(state); assert(count);

  Span *result = palloc(sizeof(Span) * state->nbins);
  /* We need to loop since atSpan may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid bins */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current bin (if any) and advance state */
    Span span;
    if (! span_bin_state_get(state, &span))
    {
      pfree(state);
      break;
    }
    span_bin_state_next(state);

    /* Restrict the temporal number to the span and compute its bounding span */
    SpanSet *atspan = intersection_spanset_span(ss, &span);
    if (atspan == NULL)
      continue;
    memcpy(&span, &atspan->span, sizeof(Span));
    pfree(atspan);

    /* Copy the span to the result */
    memcpy(&result[i++], &span, sizeof(Span));
  }
  *count = i;
  return result;
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return the time bins of a span set
 * @param[in] ss Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #spanset_time_bin_init
 */
Span *
spanset_time_spans(const SpanSet *ss, const Interval *duration,
  Datum torigin, int *count)
{
  /* Initialize state */
  int nbins;
  SpanBinState *state = spanset_time_bin_init(ss, duration, torigin,
    &nbins);
  if (! state)
    return NULL;
  return spanset_spans_common(ss, state, count);
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return the bins of a temporal number
 * @param[in] ss Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #spanset_value_bin_init
 */
Span *
spanset_value_spans(const SpanSet *ss, Datum vsize, Datum vorigin, int *count)
{
  /* Initialize state */
  int nbins;
  SpanBinState *state = spanset_value_bin_init(ss, vsize, vorigin, &nbins);
  if (! state)
    return NULL;
  return spanset_spans_common(ss, state, count);
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the time bins of a temporal value
 * @param[in] temp Input span to split
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #temporal_time_bin_init
 */
Span *
temporal_time_spans(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, int *count)
{
  /* Initialize state */
  int nbins;
  SpanBinState *state = temporal_time_bin_init(temp, duration, torigin,
    &nbins);
  if (! state)
    return NULL;

  Span *result = palloc(sizeof(Span) * nbins);
  /* We need to loop since atSpan may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid bins */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current bin (if any) and advance state */
    Span span;
    if (! span_bin_state_get(state, &span))
    {
      pfree(state);
      break;
    }
    span_bin_state_next(state);

    /* Restrict the temporal value to the timestamptz span and compute
     * its bounding span */
    Temporal *atspan = temporal_restrict_tstzspan(temp, &span, REST_AT);
    if (atspan == NULL)
      continue;
    temporal_set_tstzspan(atspan, &span);
    pfree(atspan);

    /* Copy the span to the result */
    memcpy(&result[i++], &span, sizeof(Span));
  }
  *count = i;
  return result;
}

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return the bins of a temporal number
 * @param[in] temp Input span to split
 * @param[in] vsize Size of the bins
 * @param[in] vorigin Origin of the bins
 * @param[out] count Number of elements in the output array
 * @note The tests for the validity of the arguments is done in function
 * #tnumber_value_bin_init
 */
Span *
tnumber_value_spans(const Temporal *temp, Datum vsize, Datum vorigin,
  int *count)
{
  /* Initialize state */
  int nbins;
  SpanBinState *state = tnumber_value_bin_init(temp, vsize, vorigin, &nbins);
  if (! state)
    return NULL;

  Span *result = palloc(sizeof(Span) * nbins);
  /* We need to loop since atSpan may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid bins */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current bin (if any) and advance state */
    Span span;
    if (! span_bin_state_get(state, &span))
    {
      pfree(state);
      break;
    }
    span_bin_state_next(state);

    /* Restrict the temporal number to the span and compute its bounding span */
    Temporal *atspan = tnumber_restrict_span((Temporal *) state->to_split,
      &span, REST_AT);
    if (atspan == NULL)
      continue;
    tnumber_set_span(atspan, &span);
    pfree(atspan);

    /* Copy the span to the result */
    memcpy(&result[i++], &span, sizeof(Span));
  }
  *count = i;
  return result;
}

/*****************************************************************************
 * TBox tile functions
 *****************************************************************************/

/**
 * @brief Create the initial state for tiling operations
 * @param[in] temp Temporal number, may be @p NULL
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] vsize Value size of the tiles, may be 0 for time boxes
 * @param[in] duration Interval defining the time size of the tile, may be
 * @p NULL for value boxes
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 */
TboxGridState *
tbox_tile_state_make(const Temporal *temp, const TBox *box, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin)
{
  assert(box); assert(duration || datum_gt(vsize, 0, box->span.basetype));

  /* Create the state, use palloc0 to initialize missing dimensions */
  TboxGridState *state = palloc0(sizeof(TboxGridState));
  /* Fill in state */
  state->i = 1;
  state->ntiles = 1;
  Datum start_bin, end_bin;
  /* Set the value dimension of the state box*/
  int j = 0;
  if (datum_double(vsize, box->span.basetype))
  {
    state->vsize = vsize;
    state->max_coords[j] = span_no_bins(&box->span, vsize, vorigin, &start_bin,
      &end_bin) - 1;
    state->ntiles *= (state->max_coords[j] + 1);
    span_set(start_bin, end_bin, true, false, box->span.basetype,
      box->span.spantype, &state->box.span);
    MEOS_FLAGS_SET_X(state->box.flags, true);
    /* Advance max_coords */
    j++;
  }
  /* Set the time dimension of the state box */
  if (duration)
  {
    state->tunits = interval_units(duration);
    state->max_coords[j] = span_no_bins(&box->period,
      Int64GetDatum(state->tunits), TimestampTzGetDatum(torigin),
      &start_bin, &end_bin) - 1;
    state->ntiles *= (state->max_coords[j] + 1);
    span_set(start_bin, end_bin, true, false, T_TIMESTAMPTZ, T_TSTZSPAN,
      &state->box.period);
    MEOS_FLAGS_SET_T(state->box.flags, true);
  }
  /* Set the temporal value */
  state->temp = temp;
  /* Set the state start values */
  state->value = state->box.span.lower;
  state->t = DatumGetTimestampTz(state->box.period.lower);
  return state;
}

/**
 * @brief Generate a tile from the a multidimensional grid
 * @param[in] value Start value of the tile to output
 * @param[in] t Start timestamp of the tile to output
 * @param[in] vsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[in] basetype Type of the value
 * @param[in] spantype Span type of the value
 * @param[out] box Output box
 */
void
tbox_tile_state_set(Datum value, TimestampTz t, Datum vsize, int64 tunits,
  meosType basetype, meosType spantype, TBox *box)
{
  assert(box);

  /* Initialize the box for missing dimensions */
  memset(box, 0, sizeof(TBox));
  /* Set value span */
  if (numspan_type(spantype))
  {
    Datum xmin = value;
    Datum xmax = datum_add(value, vsize, basetype);
    span_set(xmin, xmax, true, false, basetype, spantype, &box->span);
    MEOS_FLAGS_SET_X(box->flags, true);
  }
  /* Set value span */
  if (tunits)
  {
    Datum tmin = TimestampTzGetDatum(t);
    Datum tmax = TimestampTzGetDatum(t + tunits);
    MEOS_FLAGS_SET_T(box->flags, true);
    span_set(tmin, tmax, true, false, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  }
  return;
}

/**
 * @brief Get the current tile of the multidimensional grid
 * @param[in] state State to increment
 * @param[out] box Current tile
 */
bool
tbox_tile_state_get(TboxGridState *state, TBox *box)
{
  if (! state || state->done)
    return false;
  /* Get the box of the current tile */
  tbox_tile_state_set(state->value, state->t, state->vsize, state->tunits,
    state->box.span.basetype, state->box.span.spantype, box);
  return true;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional
 * grid
 * @param[in] state State to increment
 */
void
tbox_tile_state_next(TboxGridState *state)
{
  assert(state);
  assert(MEOS_FLAGS_GET_X(state->box.flags) ||
    MEOS_FLAGS_GET_T(state->box.flags));

  if (! state || state->done)
    return;
  /* Move to the next tile */
  state->i++;
  /* Advance the current values for the available dimensions */
  if (MEOS_FLAGS_GET_X(state->box.flags))
  {
    /* X dimension */
    state->value = datum_add(state->value, state->vsize,
      state->box.span.basetype);
    state->coords[0]++;
    if (state->coords[0] > state->max_coords[0])
    {
      if (MEOS_FLAGS_GET_T(state->box.flags))
      {
        /* X and T dimensions */
        state->value = state->box.span.lower;
        state->coords[0] = 0;
        state->t += state->tunits;
        state->coords[1]++;
        if (state->coords[1] > state->max_coords[1])
        {
          state->done = true;
          return;
        }
      }
      else
      {
        /* Only X dimension */
        state->done = true;
        return;
      }
    }
  }
  else
  {
    /* Only T dimension */
    if (MEOS_FLAGS_GET_T(state->box.flags))
    {
      state->t += state->tunits;
      state->coords[0]++;
      if (state->coords[0] > state->max_coords[0])
      {
        state->done = true;
        return;
      }
    }
  }
  return;
}

/*****************************************************************************
 * Multidimensional tile list functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] basetype Type of the value
 * @param[in] spantype Type of the value span
 * @csqlfn #Tbox_get_value_time_tile()
 */
TBox *
tbox_get_value_time_tile(Datum value, TimestampTz t, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  meosType basetype, meosType spantype)
{
  /* Ensure the validity of the arguments */
  if (duration && ! ensure_valid_duration(duration))
    return NULL;

  /* Determine whether there is a value tile */
  bool valuetile = datum_gt(vsize, (Datum) 0, basetype);
  assert(valuetile || duration);
  /* Initialize to 0 missing arguments */
  Datum value_bin = (Datum) 0;
  int64 tunits = 0;
  TimestampTz time_bin = 0;
  /* Determine the tile */
  if (valuetile)
    value_bin = datum_bin(value, vsize, vorigin, basetype);
  if (duration)
  {
    tunits = interval_units(duration);
    time_bin = timestamptz_get_bin(t, duration, torigin);
  }
  TBox *result = palloc(sizeof(TBox));
  tbox_tile_state_set(value_bin, time_bin, vsize, tunits, basetype, spantype,
    result);
  return result;
}

/*****************************************************************************
 * TBoxes functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Set the state with a temporal number and a value and possibly time
 * grid for splitting or obtaining a set of temporal boxes
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[out] ntiles Number of tiles
 * @note The function can be used for obtaining value boxes, time boxes, and
 * value and time boxes
 */
TboxGridState *
tnumber_value_time_tile_init(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin, int *ntiles)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TNUMBER(temp, NULL); VALIDATE_NOT_NULL(ntiles, NULL);
  if (! ensure_positive_datum(vsize, temptype_basetype(temp->temptype)) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  /* Set bounding box */
  TBox bounds;
  tnumber_set_tbox(temp, &bounds);
  /* Create function state */
  TboxGridState *state = tbox_tile_state_make(temp, &bounds, vsize, duration,
    vorigin, torigin);
  *ntiles = state->ntiles;
  return state;
}

/**
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] duration Size of the time dimension as an interval, may be `NULL`
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[out] count Number of elements in the output array
 * @note The check for parameter validity is done in function
 * #tnumber_value_time_tile_init to be shared for both MEOS and MobilityDB
 * @csqlfn #Tnumber_value_time_boxes
 */
TBox *
tnumber_value_time_boxes(const Temporal *temp, Datum vsize,
  const Interval *duration, Datum vorigin, TimestampTz torigin, int *count)
{
  VALIDATE_TNUMBER(temp, NULL); 

  /* Initialize state */
  int ntiles;
  TboxGridState *state = tnumber_value_time_tile_init(temp, vsize, duration,
    vorigin, torigin, &ntiles);
  if (! state)
    return NULL;

  TBox *result = palloc(sizeof(TBox) * ntiles);
  /* We need to loop since atTbox may be NULL */
  int i = 0;
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      pfree(state);
      break;
    }

    /* Get current tile (if any) and advance state
     * It is necessary to test if we found a tile since the previous tile
     * may be the last one set in the associated bit matrix */
    TBox box;
    if (! tbox_tile_state_get(state, &box))
    {
      pfree(state);
      break;
    }
    tbox_tile_state_next(state);

    /* Restrict the temporal number to the box and compute its bounding box */
    Temporal *attbox = tnumber_at_tbox((Temporal *) state->temp, &box);
    if (attbox == NULL)
      continue;
    tnumber_set_tbox(attbox, &box);
    pfree(attbox);

    /* Copy the box to the result */
    memcpy(&result[i++], &box, sizeof(TBox));
  }
  *count = i;
  return result;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 */
TBox *
tint_value_time_boxes(const Temporal *temp, int vsize,
  const Interval *duration, int vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and possibly a time grid
 */
TBox *
tfloat_value_time_boxes(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}
#endif /* MEOS */

/*****************************************************************************/
