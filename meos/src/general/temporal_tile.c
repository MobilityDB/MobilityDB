/***********************************************************************
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
  /* Ensure validity of the arguments */
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
  /* Ensure validity of the arguments */
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
  /* Ensure validity of the arguments */
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
int64
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) duration) ||
      ! ensure_valid_day_duration(duration))
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
     * time is at least the minimum time value value (DT_NOBEGIN) and when
     * negative that it is less than the maximum time value (DT_NOEND)
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) duration) ||
      ! ensure_valid_duration(duration))
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) nbins) ||
      ! ensure_not_null((void *) duration) ||
      (ss->basetype != T_DATE && ! ensure_valid_duration(duration)) ||
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) nbins) ||
      ! ensure_positive_datum(vsize, ss->basetype))
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) nbins) ||
      ! ensure_not_null((void *) duration) ||
      ! ensure_valid_duration(duration))
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) nbins) ||
      ! ensure_tnumber_type(temp->temptype) ||
      ! ensure_positive_datum(vsize, temptype_basetype(temp->temptype)))
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
 * Bins functions
 *****************************************************************************/

#if MEOS
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) count) ||
      ! ensure_not_null((void *) duration) ||
      ! ensure_valid_day_duration(duration) ||
      ! ensure_span_isof_type(s, T_DATESPAN))
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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) count) ||
      ! ensure_not_null((void *) duration) ||
      ! ensure_valid_duration(duration) ||
      ! ensure_span_isof_type(s, T_TSTZSPAN))
    return NULL;

  int64 tunits = interval_units(duration);
  return span_bins(s, Int64GetDatum(tunits), TimestampTzGetDatum(origin),
    count);
}
#endif /* MEOS */

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

#if MEOS
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
  return spanset_time_spans(ss, duration, TimestampTzGetDatum(torigin), count);
}
#endif /* MEOS */

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

#if MEOS
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
  return spanset_value_spans(ss, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), count);
}
#endif /* MEOS */

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

#if MEOS
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
  return tnumber_value_spans(temp, Float8GetDatum(vsize),
    Float8GetDatum(vorigin), count);
}
#endif /* MEOS */

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

#if MEOS
/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Generate a multidimensional grid for temporal numbers
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the temporal size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 */
TBox *
tbox_value_time_tiles(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin, int *count)
{
  assert(box); assert(count); assert(duration);
  assert(positive_datum(vsize, box->span.basetype));
  assert(valid_duration(duration));

  TboxGridState *state = tbox_tile_state_make(NULL, box, vsize, duration,
    vorigin, torigin);
  int nrows = 1, ncols = 1;
  Datum start_bin, end_bin;
  /* Determine the number of value bins */
  if (datum_double(vsize, box->span.basetype))
    nrows = span_no_bins(&box->span, vsize, vorigin, &start_bin, &end_bin);
  /* Determine the number of time bins */
  int64 tunits = duration ? interval_units(duration) : 0;
  if (tunits)
    ncols = span_no_bins(&box->period, Int64GetDatum(tunits),
      TimestampTzGetDatum(torigin), &start_bin, &end_bin);
  /* Total number of tiles */
  int count1 = nrows * ncols;

  /* Compute the tiles */
  TBox *result = palloc0(sizeof(TBox) * count1);
  for (int i = 0; i < count1; i++)
  {
    tbox_tile_state_set(state->value, state->t, state->vsize, state->tunits,
      state->box.span.basetype, state->box.span.spantype, &result[i]);
    tbox_tile_state_next(state);
  }
  *count = count1;
  pfree(state);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile list of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tintbox_value_time_tiles(const TBox *box, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) count))
    return NULL;
  return tbox_value_time_tiles(box, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile list of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_value_time_tiles()
 */
TBox *
tfloatbox_value_time_tiles(const TBox *box, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) count))
    return NULL;
  return tbox_value_time_tiles(box, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
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

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @csqlfn #Tbox_get_value_tile()
 */
TBox *
tintbox_get_value_tile(int value, int vsize, int vorigin)
{
  return tbox_get_value_time_tile(Int32GetDatum(value), 0, Int32GetDatum(vsize),
    NULL, Int32GetDatum(vorigin), 0, T_INT4, T_INTSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] vsize Value size of the tiles
 * @param[in] vorigin Value origin of the tiles
 * @csqlfn #Tbox_get_value_tile()
 */
TBox *
tfloatbox_get_value_tile(double value, double vsize, double vorigin)
{
  return tbox_get_value_time_tile(Float8GetDatum(value), 0, Float8GetDatum(vsize),
    NULL, Float8GetDatum(vorigin), 0, T_FLOAT8, T_FLOATSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] t Timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_time_tile()
 */
TBox *
tintbox_get_time_tile(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  return tbox_get_value_time_tile((Datum) 0, t, (Datum) 0, duration, (Datum) 0,
    torigin, T_INT4, T_INTSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] t Timestamp
 * @param[in] duration Interval defining the size of the bins
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_time_tile()
 */
TBox *
tfloatbox_get_time_tile(TimestampTz t, const Interval *duration,
  TimestampTz torigin)
{
  return tbox_get_value_time_tile((Datum) 0, t, (Datum) 0, duration, (Datum) 0,
    torigin, T_FLOAT8, T_FLOATSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_value_time_tile()
 */
TBox *
tintbox_get_value_time_tile(int value, TimestampTz t, int vsize,
  const Interval *duration, int vorigin, TimestampTz torigin)
{
  return tbox_get_value_time_tile(Int32GetDatum(value), t, Int32GetDatum(vsize),
    duration, Int32GetDatum(vorigin), torigin, T_INT4, T_INTSPAN);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the bins
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_get_value_time_tile()
 */
TBox *
tfloatbox_get_value_time_tile(double value, TimestampTz t, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin)
{
  return tbox_get_value_time_tile(Float8GetDatum(value), t,
    Float8GetDatum(vsize), duration, Float8GetDatum(vorigin), torigin,
    T_FLOAT8, T_FLOATSPAN);
}
#endif /* MEOS */

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
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) ntiles) ||
      ! ensure_positive_datum(vsize, temptype_basetype(temp->temptype)) ||
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
 * @param[in] duration Size of the time dimension as an interval
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
  assert(temp);

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
 * @brief Return the temporal boxes of a temporal integer split with respect to
 * value bins
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] vorigin Origin for the value dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes
 */
TBox *
tint_value_boxes(const Temporal *temp, int vsize, int vorigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), NULL,
    Int32GetDatum(vorigin), 0, count);
}

/**
 * @brief Return the temporal boxes of a temporal float split with respect to
 * time bins
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] vorigin Origin for the value dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_boxes
 */
TBox *
tfloat_value_boxes(const Temporal *temp, double vsize, double vorigin,
  int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), NULL,
    Float8GetDatum(vorigin), 0, count);
}

/**
 * @brief Return the temporal boxes of a temporal integer split with respect to
 * time bins
 * @param[in] temp Temporal number
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] torigin Origin for the time dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes
 */
TBox *
tint_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(0), duration,
    Int32GetDatum(0), torigin, count);
}

/**
 * @brief Return the temporal boxes of a temporal integer split with respect to
 * a value and possibly a time grid
 * @param[in] temp Temporal number
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] torigin Origin for the time dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_time_boxes
 */
TBox *
tfloat_time_boxes(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(0), duration,
    Float8GetDatum(0), torigin, count);
}

/**
 * @brief Return the temporal boxes of a temporal integer split with respect to
 * a value and possibly a time grid
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_time_boxes
 */
TBox *
tint_value_time_boxes(const Temporal *temp, int vsize,
  const Interval *duration, int vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/**
 * @brief Return the temporal boxes of a temporal integer split with respect to
 * a value and possibly a time grid
 * @param[in] temp Temporal number
 * @param[in] vsize Size of the value dimension
 * @param[in] duration Size of the time dimension as an interval
 * @param[in] vorigin Origin for the value dimension
 * @param[in] torigin Origin for the time dimension
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tnumber_value_time_boxes
 */
TBox *
tfloat_value_time_boxes(const Temporal *temp, double vsize,
  const Interval *duration, double vorigin, TimestampTz torigin, int *count)
{
  return tnumber_value_time_boxes(temp, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}
#endif /* MEOS */

/*****************************************************************************
 * Time split functions for temporal numbers
 *****************************************************************************/

#if MEOS
/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] inst Temporal value
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] count Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, int64 tunits, TimestampTz torigin,
  TimestampTz **bins, int *count)
{
  assert(inst);
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = tinstant_copy(inst);
  times[0] = timestamptz_get_bin_int(inst->t, tunits, torigin);
  *bins = times;
  *count = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start Start timestamp of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tdiscseq_time_split(const TSequence *seq, TimestampTz start, int64 tunits,
  int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0,       /* counter for instants of temporal value */
      ninsts = 0,  /* counter for instants of next split */
      nfrags = 0;  /* counter for resulting fragments */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if (lower <= inst->t && inst->t < upper)
    {
      instants[ninsts++] = inst;
      i++;
    }
    else
    {
      if (ninsts > 0)
      {
        times[nfrags] = lower;
        result[nfrags++] = tsequence_make(instants, ninsts, true, true,
          DISCRETE, NORMALIZE_NO);
        ninsts = 0;
      }
      lower = upper;
      upper += tunits;
    }
  }
  if (ninsts > 0)
  {
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, true, true, DISCRETE,
      NORMALIZE_NO);
  }
  pfree(instants);
  *bins = times;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bin lower bounds
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tcontseq_time_split_iter(const TSequence *seq, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TSequence **result,
  TimestampTz *times)
{
  assert(seq); assert(result); assert(times);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* This loop is needed for filtering unnecesary time bins that are in the
   * time gaps between sequences composing a sequence set.
   * The upper bound for the bin is exclusive => the test below is >= */
  while (lower < end &&
    (DatumGetTimestampTz(seq->period.lower) >= upper ||
     lower > DatumGetTimestampTz(seq->period.upper) ||
     (lower == DatumGetTimestampTz(seq->period.upper) &&
        ! seq->period.upper_inc)))
  {
    lower = upper;
    upper += tunits;
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  int i = 0,      /* counter for instants of temporal value */
      ninsts = 0, /* counter for instants of next split */
      nfree = 0,  /* counter for instants to free */
      nfrags = 0; /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    /* If the instant is in the bin */
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && (interp == LINEAR || i == seq->count - 1)))
    {
      instants[ninsts++] = inst;
      i++;
    }
    else
    {
      assert(ninsts > 0);
      /* Compute the value at the end of the bin */
      if (instants[ninsts - 1]->t < upper)
      {
        if (interp == LINEAR)
          tofree[nfree] = tsegment_at_timestamptz(instants[ninsts - 1], inst,
            interp, upper);
        else
        {
          /* The last two values of sequences with step interpolation and
           * exclusive upper bound must be equal */
          Datum value = tinstant_val(instants[ninsts - 1]);
          tofree[nfree] = tinstant_make(value, seq->temptype, upper);
        }
        instants[ninsts++] = tofree[nfree++];
      }

      /* Compute the fragment */
      lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
      times[nfrags] = lower;
      result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
         (ninsts > 1) ? false : true, interp, NORMALIZE);

      /* Set up for the next bin */
      ninsts = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary time bins
       * that are in the gaps between sequences composing a sequence set */
      if (lower >= end || ! contains_span_timestamptz(&seq->period, lower))
        break;
      /* The end value of the previous bin is the start of the new bin */
      if (lower < inst->t)
        instants[ninsts++] = TSEQUENCE_INST_N(result[nfrags - 1],
          result[nfrags - 1]->count - 1);
     }
  }
  if (ninsts > 0)
  {
    lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
    times[nfrags] = lower;
    result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
      seq->period.upper_inc, interp, NORMALIZE);
  }
  pfree_array((void **) tofree, nfree);
  pfree(instants);
  return nfrags;
}

/**
 * @brief Split a temporal value into an array of fragments according to time
 * bins
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tcontseq_time_split(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) != DISCRETE);

  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tcontseq_time_split_iter(seq, start, end, tunits, count, result,
    times);
  *bins = times;
  return result;
}

/**
 * @brief Split a temporal value into an array of disjoint fragments
 * @param[in] ss Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequenceSet **
tsequenceset_time_split(const TSequenceSet *ss, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TimestampTz **bins,
  int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence **sequences = tcontseq_time_split(TSEQUENCESET_SEQ_N(ss, 0),
      start, end, tunits, count, bins, newcount);
    TSequenceSet **result = palloc(sizeof(TSequenceSet *) * *newcount);
    for (int i = 0; i < *newcount; i++)
      result[i] = tsequence_to_tsequenceset_free(sequences[i]);
    pfree(sequences);
    return result;
  }

  /* General case */
  /* Sequences obtained by spliting one composing sequence */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Start timestamp of bins obtained by spliting one composing sequence */
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ss->count + count));
  /* Sequences composing the currently constructed bin of the sequence set */
  TSequence **fragments = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Sequences for the bins of the sequence set */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  /* Variable used to adjust the start timestamp passed to the
   * tcontseq_time_split1 function in the loop */
  TimestampTz lower = start;
  int nfrags = 0, /* Number of accumulated fragments of the current time bin */
      nbucks = 0; /* Number of time bins already processed */
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz upper = lower + tunits;
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Output the accumulated fragments of the current time bin (if any)
     * if the current sequence starts on the next time bin */
    if (nfrags > 0 && DatumGetTimestampTz(seq->period.lower) >= upper)
    {
      result[nbucks++] = tsequenceset_make((const TSequence **) fragments, nfrags,
        NORMALIZE);
      for (int j = 0; j < nfrags; j++)
        pfree(fragments[j]);
      nfrags = 0;
      lower += tunits;
      upper += tunits;
    }
    /* Number of time bins of the current sequence */
    int l = tcontseq_time_split_iter(seq, lower, end, tunits, count,
      sequences, &times[nbucks]);
    /* If the current sequence has produced more than two time bins */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bin (if any)  */
      if (nfrags == 0)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[0]);
      else
      {
        fragments[nfrags++] = sequences[0];
        result[nbucks++] = tsequenceset_make((const TSequence **) fragments,
          nfrags, NORMALIZE);
        for (int j = 0; j < nfrags; j++)
          pfree(fragments[j]);
        nfrags = 0;
      }
      for (int j = 1; j < l - 1; j++)
        result[nbucks++] = tsequence_to_tsequenceset_free(sequences[j]);
    }
    /* Save the last fragment in case it is necessary to assemble with the
     * first one of the next sequence */
    fragments[nfrags++] = sequences[l - 1];
    lower = times[nbucks];
  }
  /* Process the accumulated fragments of the last time bin */
  if (nfrags > 0)
  {
    result[nbucks++] = tsequenceset_make((const TSequence **) fragments, nfrags,
      NORMALIZE);
    for (int j = 0; j < nfrags; j++)
      pfree(fragments[j]);
  }
  pfree(sequences); pfree(fragments);
  *bins = times;
  *newcount = nbucks;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] start,end Start and end timestamps of the bins
 * @param[in] tunits Size of the time bins in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[in] count Number of bins
 * @param[out] bins Start timestamp of the bins containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static Temporal **
temporal_time_split_int(const Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, TimestampTz torigin, int count, TimestampTz **bins,
  int *newcount)
{
  assert(temp); assert(bins); assert(newcount); assert(start < end);
  assert(count > 0);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tinstant_time_split((const TInstant *) temp,
        tunits, torigin, bins, newcount);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tdiscseq_time_split((const TSequence *) temp,
          start, tunits, count, bins, newcount) :
        (Temporal **) tcontseq_time_split((const TSequence *) temp,
          start, end, tunits, count, bins, newcount);
    default: /* TSEQUENCESET */
      return (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
        start, end, tunits, count, bins, newcount);
  }
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time bins
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time bins
 * @param[in] torigin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_time_split()
 */
Temporal **
temporal_time_split(const Temporal *temp, const Interval *duration,
  TimestampTz torigin, TimestampTz **bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_valid_duration(duration))
    return NULL;

  Span s;
  temporal_set_tstzspan(temp, &s);
  Datum start_bin, end_bin;
  int64 tunits = interval_units(duration);
  int nbins = span_no_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), &start_bin, &end_bin);
  return temporal_time_split_int(temp, DatumGetTimestampTz(start_bin),
    DatumGetTimestampTz(end_bin), tunits, torigin, nbins, bins, count);
}
#endif /* MEOS */

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

#if MEOS
/**
 * @brief Get the bin number in the bin space that contains the value
 * @param[in] value Input value
 * @param[in] size Size of the bins
 * @param[in] origin Origin of the bins
 * @param[in] type Type of the arguments
 */
static int
bin_position(Datum value, Datum size, Datum origin, meosType type)
{
  assert(tnumber_basetype(type));
  if (type == T_INT4)
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] inst Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstant **
tnumberinst_value_split(const TInstant *inst, Datum start_bin, Datum size,
  Datum **bins, int *newcount)
{
  assert(inst); assert(bins); assert(newcount);

  Datum value = tinstant_val(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = datum_bin(value, size, start_bin, basetype);
  *bins = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] size Size of the value bins
 * @param[in] start_bin Value of the start bin
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tnumberseq_disc_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  meosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bin_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *ninsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    value = tinstant_val(inst);
    bin_value = datum_bin(value, size, start_bin, basetype);
    int bin_no = bin_position(bin_value, size, start_bin, basetype);
    int inst_no = ninsts[bin_no]++;
    instants[bin_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bin */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bin_value = start_bin;
  for (int i = 0; i < count; i++)
  {
    if (ninsts[i] > 0)
    {
      result[nfrags] = tsequence_make(&instants[i * seq->count], ninsts[i],
        true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(instants);
  pfree(ninsts);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_GET_INTERP(seq->flags) == STEP);

  meosType basetype = temptype_basetype(seq->temptype);
  Datum value, bin_value;
  int bin_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_val(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    int nfrags = 1;
    if (i < seq->count)
    {
      tofree[nfree++] = bounds[1] = tinstant_make(value, seq->temptype,
        inst2->t);
      nfrags++;
    }
    result[bin_no * numcols + seq_no] = tsequence_make(
      (const TInstant **) bounds, nfrags, lower_inc1, false, STEP, NORMALIZE);
    bounds[0] = bounds[1];
    inst1 = inst2;
    lower_inc1 = true;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = TSEQUENCE_INST_N(seq, seq->count - 1);
    value = tinstant_val(inst1);
    bin_value = datum_bin(value, size, start_bin, basetype);
    bin_no = bin_position(bin_value, size, start_bin, basetype);
    seq_no = nseqs[bin_no]++;
    result[bin_no * numcols + seq_no] = tinstant_to_tsequence(inst1, STEP);
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[in,out] result Array containing the fragments of each bin
 * @param[in,out] nseqs Number of fragments for each bin
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bin,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(seq); assert(result); assert(nseqs);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  meosType basetype = temptype_basetype(seq->temptype);
  meosType spantype = basetype_spantype(basetype);
  Datum value1, bin_value1;
  int bin_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    bin_value1 = datum_bin(value1, size, start_bin, basetype);
    bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
    seq_no = nseqs[bin_no1]++;
    result[bin_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_val(inst1);
  bin_value1 = datum_bin(value1, size, start_bin, basetype);
  bin_no1 = bin_position(bin_value1, size, start_bin, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_val(inst2);
    Datum bin_value2 = datum_bin(value2, size, start_bin, basetype);
    int bin_no2 = bin_position(bin_value2, size, start_bin, basetype);

    /* Set variables depending on whether the segment is constant, increasing,
     * or decreasing */
    Datum min_value, max_value;
    int first_bin, last_bin, first, last;
    bool lower_inc1, upper_inc1; /* Lower/upper bound inclusion of the segment */
    bool lower_inc_def, upper_inc_def; /* Default lower/upper bound inclusion */
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp <= 0)
    {
      /* Both for constant and increasing segments */
      min_value = value1;
      max_value = value2;
      first_bin = bin_no1;
      last_bin = bin_no2;
      first = 0;
      last = 1;
      lower_inc_def = true;
      upper_inc_def = false;
      lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
      upper_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
    }
    else
    {
      min_value = value2;
      max_value = value1;
      first_bin = bin_no2;
      last_bin = bin_no1;
      first = 1;
      last = 0;
      lower_inc_def = false;
      upper_inc_def = true;
      lower_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
      upper_inc1 = (i == 1) ? seq->period.lower_inc : true;
    }

    /* Split the segment into bins */
    span_set(min_value, max_value, lower_inc1, (cmp != 0) ? upper_inc1 : true,
      basetype, spantype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bin_lower = (cmp <= 0) ? bin_value1 : bin_value2;
    Datum bin_upper = datum_add(bin_lower, size, basetype);
    for (int j = first_bin; j <= last_bin; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bin_upper, basetype) &&
        datum_lt(bin_upper, max_value, basetype))
      {
        TimestampTz t;
        Datum projvalue;
        tlinearsegm_intersection_value(inst1, inst2, bin_upper, basetype,
          &projvalue, &t);
        /* To reduce the roundoff errors we take the value projected to the
         * timestamp instead of the bound value */
        tofree[nfree++] = bounds[last] =
          tinstant_make(projvalue, seq->temptype, t);
      }
      else
        bounds[last] = (cmp <= 0) ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bin || j == last_bin)
      {
        Span binspan;
        span_set(bin_lower, bin_upper, true, false, basetype, spantype,
          &binspan);
        Span inter;
        bool found = inter_span_span(&segspan, &binspan, &inter);
        if (found)
        {
          /* Do nothing for constant segments */
          if (cmp < 0)
          {
            lower_inc1 = inter.lower_inc;
            upper_inc1 = inter.upper_inc;
          }
          else if (cmp > 0)
          {
            lower_inc1 = inter.upper_inc;
            upper_inc1 = inter.lower_inc;
          }
        }
        else
        {
          lower_inc1 = upper_inc1 = false;
        }
      }
      else
      {
        /* Sequence bounds are the bin bounds */
        lower_inc1 = lower_inc_def;
        upper_inc1 = upper_inc_def;
      }
      /* If last bin contains a single instant */
      int nfrags = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bin if last instant has exclusive bound */
      if (nfrags == 1 && ! upper_inc1)
        break;
      seq_no = nseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        nfrags, (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bin_lower = bin_upper;
      bin_upper = datum_add(bin_upper, size, basetype);
    }
    inst1 = inst2;
    value1 = value2;
    bin_value1 = bin_value2;
    bin_no1 = bin_no2;
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] seq Temporal value
 * @param[in] start_bin Value of the start bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Start value of the bins containing the fragments
 * @param[out] newcount Number of elements in output arrays
 */
static TSequenceSet **
tnumberseq_cont_value_split(const TSequence *seq, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(seq); assert(bins); assert(newcount);

  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  assert(interp != DISCRETE);
  meosType basetype = temptype_basetype(seq->temptype);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_to_tsequenceset(seq);
    Datum value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bin(value, size, start_bin, basetype);
    *bins = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bin, size, count, sequences,
      nseqs, seq->count);
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make(
        (const TSequence **)(&sequences[seq->count * i]), nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(sequences);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * bins
 * @param[in] ss Temporal value
 * @param[in] start_bin Start value of the first bin
 * @param[in] size Size of the value bins
 * @param[in] count Number of bins
 * @param[out] bins Array of start values of the bins containing the
 * fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bin, Datum size,
  int count, Datum **bins, int *newcount)
{
  assert(ss); assert(bins); assert(newcount);

  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_cont_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bin,
      size, count, bins, newcount);

  /* General case */
  meosType basetype = temptype_basetype(ss->temptype);
  TSequence **binseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  bool linear = MEOS_FLAGS_LINEAR_INTERP(ss->flags);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (linear)
      tnumberseq_linear_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bin, size, count, binseqs,
        nseqs, ss->totalcount);
  }
  /* Assemble the result for each value bin */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bin_value = start_bin;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)
        (&binseqs[i * ss->totalcount]), nseqs[i], NORMALIZE);
      values[nfrags++] = bin_value;
    }
    bin_value = datum_add(bin_value, size, basetype);
  }
  pfree(binseqs);
  pfree(nseqs);
  *bins = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] vorigin Origin of the value bins
 * @param[out] bins Array of start values of the bins containing the fragments
 * @param[out] count Number of values in the output arrays
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum size, Datum vorigin,
  Datum **bins, int *count)
{
  assert(temp); assert(bins); assert(count);
  assert(tnumber_type(temp->temptype));

  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  Datum start_bin, end_bin;
  int nbins = span_no_bins(&s, size, vorigin, &start_bin, &end_bin);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tnumberinst_value_split((const TInstant *) temp,
        start_bin, size, bins, count);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberseq_disc_value_split((const TSequence *) temp,
          start_bin, size, nbins, bins, count) :
        (Temporal **) tnumberseq_cont_value_split((const TSequence *) temp,
          start_bin, size, nbins, bins, count);
    default: /* TSEQUENCESET */
      return (Temporal **) tnumberseqset_value_split(
        (const TSequenceSet *) temp, start_bin, size, nbins, bins, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal value split according to a base value and possibly
 * a temporal grid
 */
Temporal **
tnumber_value_time_split(const Temporal *temp, Datum size,
  const Interval *duration, Datum vorigin, TimestampTz torigin,
  Datum **value_bins, TimestampTz **time_bins, int *count)
{
  meosType basetype = temptype_basetype(temp->temptype);
  ensure_positive_datum(size, basetype);
  ensure_valid_duration(duration);

  Datum start_bin, end_bin, start_time_bin, end_time_bin;
  /* Compute the value bounds */
  Span s;
  tnumber_set_span(temp, &s);
  int value_count = span_no_bins(&s, size, vorigin, &start_bin,
    &end_bin);
  /* Compute the time bounds */
  temporal_set_tstzspan(temp, &s);
  int64 tunits = interval_units(duration);
  int time_count = span_no_bins(&s, Int64GetDatum(tunits),
    TimestampTzGetDatum(torigin), &start_time_bin, &end_time_bin);
  TimestampTz start_time = DatumGetTimestampTz(start_time_bin);
  TimestampTz end_time = DatumGetTimestampTz(end_time_bin);
  /* Total number of tiles */
  int ntiles = value_count * time_count;

  /* Split the temporal value */
  Datum *v_bins = NULL;
  TimestampTz *t_bins = NULL;
  Temporal **fragments;
  v_bins = palloc(sizeof(Datum) * ntiles);
  t_bins = palloc(sizeof(TimestampTz) * ntiles);
  fragments = palloc(sizeof(Temporal *) * ntiles);
  int nfrags = 0;
  Datum lower_value = start_bin;
  meosType spantype = basetype_spantype(basetype);
  while (datum_lt(lower_value, end_bin, basetype))
  {
    Datum upper_value = datum_add(lower_value, size, basetype);
    span_set(lower_value, upper_value, true, false, basetype, spantype, &s);
    Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
    if (atspan != NULL)
    {
      int num_time_splits;
      TimestampTz *times;
      Temporal **time_splits = temporal_time_split_int(atspan, start_time,
        end_time, tunits, torigin, time_count, &times, &num_time_splits);
      for (int i = 0; i < num_time_splits; i++)
      {
        v_bins[i + nfrags] = lower_value;
        t_bins[i + nfrags] = times[i];
        fragments[i + nfrags] = time_splits[i];
      }
      nfrags += num_time_splits;
      pfree(time_splits);
      pfree(times);
      pfree(atspan);
    }
    lower_value = upper_value;
  }
  *count = nfrags;
  if (value_bins)
    *value_bins = v_bins;
  else
    pfree(v_bins);
  if (time_bins)
    *time_bins = t_bins;
  else
    pfree(t_bins);
  return fragments;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] origin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tint_value_split(const Temporal *temp, int size, int origin, int **bins,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(size),
    Int32GetDatum(origin), &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal float split according to value
 * bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] origin Time origin of the bins
 * @param[out] bins Array of bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tfloat_value_split(const Temporal *temp, double size, double origin,
  double **bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(size),
    Float8GetDatum(origin), &datum_bins, count);
  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (bins)
    *bins = values;
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Time origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[out] value_bins Array of value bins
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tint_value_time_split(const Temporal *temp, int size, const Interval *duration,
  int vorigin, TimestampTz torigin, int **value_bins,
  TimestampTz **time_bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size) ||
      ! ensure_valid_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Int32GetDatum(size),
    duration, Int32GetDatum(vorigin), torigin, &datum_bins, time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  int *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time bins
 * @param[in] temp Temporal value
 * @param[in] size Size of the value bins
 * @param[in] duration Size of the time bins
 * @param[in] vorigin Time origin of the bins
 * @param[in] torigin Time origin of the bins
 * @param[out] value_bins Array of value bins
 * @param[out] time_bins Array of time bins
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tfloat_value_time_split(const Temporal *temp, double size,
  const Interval *duration, double vorigin, TimestampTz torigin,
  double **value_bins, TimestampTz **time_bins, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8) ||
      ! ensure_valid_duration(duration))
    return NULL;

  Datum *datum_bins;
  Temporal **result = tnumber_value_time_split(temp, Float8GetDatum(size),
    duration, Float8GetDatum(vorigin), torigin, &datum_bins, time_bins,
    count);

  /* Transform the datum bins into float bins and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_bins[i]);
  if (value_bins)
    *value_bins = values;
  else
    pfree(values);
  pfree(datum_bins);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
