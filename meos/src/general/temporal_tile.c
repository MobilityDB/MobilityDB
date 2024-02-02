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
 * @brief Bucket and tile functions for temporal types
 * @note The time bucket functions are inspired from TimescaleDB
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
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/temporal_restrict.h"
#include "general/tsequence.h"
#include "general/type_util.h"

/*****************************************************************************
 * Span bucket functions
 *****************************************************************************/

/**
 * @brief Get the time buckets of a temporal value
 * @param[in] s Span to tile
 * @param[in] size Size of the buckets
 * @param[in] origin Time origin of the tiles
 * @param[out] start_bucket,end_bucket Values of the start and end buckets
 * @result Number of buckets
 */
int
span_no_buckets(const Span *s, Datum size, Datum origin, Datum *start_bucket,
  Datum *end_bucket)
{
  assert(start_bucket); assert(end_bucket);
  Datum start_value = s->lower;
  /* We need to add size to obtain the end value of the last bucket */
  Datum end_value = datum_add(s->upper, size, s->basetype);
  *start_bucket = datum_bucket(start_value, size, origin, s->basetype);
  *end_bucket = datum_bucket(end_value, size, origin, s->basetype);
  return (s->basetype == T_INT4) ? /** xx **/
      (DatumGetInt32(*end_bucket) - DatumGetInt32(*start_bucket)) /
        DatumGetInt32(size) :
      (int) floor((DatumGetFloat8(*end_bucket) -
        DatumGetFloat8(*start_bucket)) /  DatumGetFloat8(size));
}

/**
 * @brief Get the time buckets of a temporal value
 * @param[in] s Span to tile
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] torigin Time origin of the tiles
 * @param[out] start_bucket,end_bucket Start and end buckets
 * @result Number of buckets
 */
int
tstzspan_no_buckets(const Span *s, const Interval *duration, TimestampTz torigin,
  Datum *start_bucket, Datum *end_bucket)
{
  assert(start_bucket); assert(end_bucket);
  TimestampTz start_time = DatumGetTimestampTz(s->lower);
  TimestampTz end_time = DatumGetTimestampTz(s->upper);
  int64 tunits = interval_units(duration);
  TimestampTz start_time_bucket =
    timestamptz_bucket(start_time, duration, torigin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz end_time_bucket =
    timestamptz_bucket(end_time, duration, torigin) + tunits;
  *start_bucket = TimestampTzGetDatum(start_time_bucket);
  *end_bucket = TimestampTzGetDatum(end_time_bucket);
  return (int) (((int64) end_time_bucket - (int64) start_time_bucket) /
    tunits);
}

/*****************************************************************************/

/**
 * @brief Generate an integer or float span bucket from a bucket list
 * @param[in] lower Start value of the bucket
 * @param[in] size Size of the buckets
 * @param[in] basetype Type of the arguments
 * @param[out] span Output span
 */
void
span_bucket_set(Datum lower, Datum size, meosType basetype, Span *span)
{
  Datum upper = (basetype == T_TIMESTAMPTZ) ?
    TimestampTzGetDatum(DatumGetTimestampTz(lower) + DatumGetInt64(size)) :
    datum_add(lower, size, basetype);
  meosType spantype = basetype_spantype(basetype);
  span_set(lower, upper, true, false, basetype, spantype, span);
  return;
}

/**
 * @brief Generate an integer or float span bucket from a bucket list
 * @param[in] lower Start value of the bucket
 * @param[in] size Size of the buckets
 * @param[in] type Type of the arguments
 */
Span *
span_bucket_get(Datum lower, Datum size, meosType type)
{
  Span *result = palloc(sizeof(Span));
  span_bucket_set(lower, size, type, result);
  return result;
}

/**
 * @brief Create the initial state for tiling operations
 * @param[in] s Bounds for generating the bucket list
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @pre The size argument must be greater to 0.
 * @note The first argument is NULL when generating the bucket list, otherwise
 * it is a temporal number to be split and in this case is the value span
 * of the temporal number
 */
SpanBucketState *
span_bucket_state_make(const Span *s, Datum size, Datum origin)
{
  SpanBucketState *state = palloc0(sizeof(SpanBucketState));
  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->basetype = s->basetype;
  state->size = size;
  state->origin = origin;
  /* Account for canonicalized spans */
  Datum upper = span_decr_bound(s->upper, s->basetype);
  state->minvalue = state->value =
    datum_bucket(s->lower, size, origin, state->basetype);
  state->maxvalue = datum_bucket(upper, size, origin, state->basetype);
  return state;
}

/**
 * @brief Increment the current state to the next bucket of the bucket list
 * @param[in] state State to increment
 */
void
span_bucket_state_next(SpanBucketState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  state->value = (state->basetype == T_TIMESTAMPTZ) ?
    TimestampTzGetDatum(DatumGetTimestampTz(state->value) +
      DatumGetInt64(state->size)) :
    datum_add(state->value, state->size, state->basetype);
  if (datum_gt(state->value, state->maxvalue, state->basetype))
    state->done = true;
  return;
}

/*****************************************************************************
 * Bucket functions
 *****************************************************************************/

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the initial value of the bucket that contains an integer
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @return On error return @p INT_MAX
 * @csqlfn #Number_bucket()
 */
int
int_bucket(int value, int size, int origin)
{
  /* Ensure validity of the arguments */
  if (! ensure_positive(size))
    return INT_MAX;

  if (origin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    origin = origin % size;
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
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the initial value of the bucket that contains a float
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @return On error return @p DBL_MAX
 * @csqlfn #Number_bucket()
 */
double
float_bucket(double value, double size, double origin)
{
  /* Ensure validity of the arguments */
  if (! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return DBL_MAX;

  if (origin != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the origin is
     * applied: when the origin is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    origin = fmod(origin, size);
    if ((origin > 0 && value < -1 * DBL_MAX + origin) ||
        (origin < 0 && value > DBL_MAX + origin))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "number out of span");
      return DBL_MAX;
    }
    value -= origin;
  }
  double result = floor(value / size) * size;
  /* Notice that by using the floor function above we remove the need to
   * add the additional if needed for the integer case to take into account
   * that integer division truncates toward 0 in C99 */
  result += origin;
  return result;
}

/**
 * @brief Return the interval in the same representation as Postgres timestamps
 */
int64
interval_units(const Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

/**
 * @brief Return the initial timestamp of the bucket that contains a timestamp
 * @param[in] t Input timestamp
 * @param[in] size Size of the time buckets in PostgreSQL time units
 * @param[in] origin Origin of the buckets
 * @return On error return DT_NOEND
 */
TimestampTz
timestamptz_bucket1(TimestampTz t, int64 size, TimestampTz origin)
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
    origin = origin % size;
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
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the initial timestamp of the bucket that contains a
 * timestamptz
 * @param[in] t Input timestamp
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] origin Origin of the buckets
 * @csqlfn #Timestamptz_bucket()
 */
TimestampTz
timestamptz_bucket(TimestampTz t, const Interval *duration, TimestampTz origin)
{
  /* Ensure validity of the arguments */
  if (! ensure_valid_duration(duration))
    return DT_NOEND;
  int64 size = interval_units(duration);
  return timestamptz_bucket1(t, size, origin);
}

/**
 * @brief Return the initial value of the bucket that contains a number value
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[in] type Data type of the arguments
 */
Datum
datum_bucket(Datum value, Datum size, Datum origin, meosType type)
{
  /* This function is called directly by the MobilityDB APID */
  if (! ensure_positive_datum(size, type))
    return 0;

  assert(span_basetype(type));
  switch (type)
  {
    case T_INT4:
      return Int32GetDatum(int_bucket(DatumGetInt32(value),
        DatumGetInt32(size), DatumGetInt32(origin)));
    case T_FLOAT8:
      return Float8GetDatum(float_bucket(DatumGetFloat8(value),
        DatumGetFloat8(size), DatumGetFloat8(origin)));
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(timestamptz_bucket1(DatumGetTimestampTz(value),
        DatumGetInt64(size), DatumGetTimestampTz(origin)));
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Unknown bucket function for type: %d", type);
      return 0;
  }
}

/*****************************************************************************
 * Bucket list functions
 *****************************************************************************/

#if MEOS
/**
 * @brief Return the bucket list from a span
 * @param[in] s Input span to split
 * @param[in] size Bucket size
 * @param[in] origin Origin of the buckets
 * @param[out] count Number of elements in the output array
 * @csqlfn #Span_bucket_list()
 */
static Span *
span_bucket_list(const Span *s, Datum size, Datum origin, int count)
{
  SpanBucketState *state = span_bucket_state_make(s, size, origin);
  Span *buckets = palloc0(sizeof(Span) * count);
  for (int i = 0; i < count; i++)
  {
    span_bucket_set(state->value, state->size, state->basetype, &buckets[i]);
    span_bucket_state_next(state);
  }
  return buckets;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bucket list of an integer span
 * @param[in] s Input span to split
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] count Number of elements in the output array
 * @csqlfn #Numberspan_bucket_list()
 */
Span *
intspan_bucket_list(const Span *s, int size, int origin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_INTSPAN) ||
      ! ensure_not_null((void *) count) ||
      ! ensure_positive(size))
    return NULL;

  Datum start_value = s->lower;
  /* We need to add size to obtain the end value of the last bucket */
  Datum end_value = datum_add(s->upper, size, T_INT4);
  Datum start_bucket = datum_bucket(start_value, size, origin, T_INT4);
  Datum end_bucket = datum_bucket(end_value, size, origin, T_INT4);
  *count = (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) /
      DatumGetInt32(size);
  return span_bucket_list(s, Int32GetDatum(size), Int32GetDatum(origin),
    *count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bucket list of a float span
 * @param[in] s Input span to split
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] count Number of elements in the output array
 * @csqlfn #Numberspan_bucket_list()
 */
Span *
floatspan_bucket_list(const Span *s, double size, double origin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_FLOATSPAN) ||
      ! ensure_not_null((void *) count) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return NULL;

  Datum start_value = s->lower;
  /* We need to add size to obtain the end value of the last bucket */
  Datum end_value = datum_add(s->upper, size, T_FLOAT8);
  Datum start_bucket = datum_bucket(start_value, size, origin, T_FLOAT8);
  Datum end_bucket = datum_bucket(end_value, size, origin, T_FLOAT8);
  *count = (int) (floor((DatumGetFloat8(end_bucket) -
      DatumGetFloat8(start_bucket)) / DatumGetFloat8(size)));
  return span_bucket_list(s, Float8GetDatum(size), Float8GetDatum(origin),
    *count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the bucket list of a timestamptz span
 * @param[in] s Input span to split
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tstzspan_bucket_list()
 */
Span *
tstzspan_bucket_list(const Span *s, const Interval *duration, TimestampTz origin,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_span_isof_type(s, T_TSTZSPAN) ||
      ! ensure_not_null((void *) count) ||
      ! ensure_valid_duration(duration))
    return NULL;

  int64 tunits = interval_units(duration);
  TimestampTz start_time = DatumGetTimestampTz(s->lower);
  TimestampTz end_time = DatumGetTimestampTz(s->upper);
  TimestampTz start_time_bucket = timestamptz_bucket(start_time, duration,
    origin);
  /* We need to add tunits to obtain the end timestamp of the last bucket */
  TimestampTz end_time_bucket = timestamptz_bucket(end_time, duration,
    origin) + tunits;
  *count = (int) (((int64) end_time_bucket - (int64) start_time_bucket) /
    tunits);
  return span_bucket_list(s, Int64GetDatum(tunits), TimestampTzGetDatum(origin),
    *count);
}
#endif /* MEOS */

/*****************************************************************************
 * TBox tile functions
 *****************************************************************************/

/**
 * @brief Create the initial state for tiling operations
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the time size of the tile, may be NULL
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @pre At least one of vsize or tunits must be greater than 0.
 */
TboxGridState *
tbox_tile_state_make(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin)
{
  assert(box);
  int64 tunits = duration ? interval_units(duration) : 0;
  double size = datum_double(vsize, box->span.basetype);
  assert(size > 0 || tunits > 0);
  TboxGridState *state = palloc0(sizeof(TboxGridState));

  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->vsize = vsize;
  state->tunits = tunits;
  Span span, period;
  memset(&span, 0, sizeof(Span));
  memset(&period, 0, sizeof(Span));
  Datum start_bucket, end_bucket;
  if (size)
  {
    span_no_buckets(&box->span, vsize, vorigin, &start_bucket, &end_bucket);
    span_set(start_bucket, end_bucket, true, false, box->span.basetype,
      box->span.spantype, &span);
  }
  if (tunits)
  {
    tstzspan_no_buckets(&box->period, duration, torigin, &start_bucket,
      &end_bucket);
    span_set(start_bucket, end_bucket, true, false, T_TIMESTAMPTZ, T_TSTZSPAN,
      &period);
  }
  tbox_set(size ? &span : NULL, tunits ? &period : NULL, &state->box);
  state->value = state->box.span.lower;
  state->t = DatumGetTimestampTz(state->box.period.lower);
  return state;
}

/**
 * @brief Generate a tile from the a multidimensional grid
 * @param[in] value Start value of the tile to output
 * @param[in] basetype Type of the value
 * @param[in] t Start timestamp of the tile to output
 * @param[in] vsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[out] box Output box
 */
void
tbox_tile_get(Datum value, TimestampTz t, Datum vsize, int64 tunits,
  meosType basetype, TBox *box)
{
  assert(box);
  Datum xmin = value;
  Datum xmax = datum_add(value, vsize, basetype);
  Datum tmin = TimestampTzGetDatum(t);
  Datum tmax = TimestampTzGetDatum(t + tunits);
  Span span, period;
  memset(&span, 0, sizeof(Span));
  memset(&period, 0, sizeof(Span));
  double size = datum_double(vsize, basetype);
  meosType spantype = basetype_spantype(basetype);
  if (size)
    span_set(xmin, xmax, true, false, basetype, spantype, &span);
  if (tunits)
    span_set(tmin, tmax, true, false, T_TIMESTAMPTZ, T_TSTZSPAN, &period);
  tbox_set(size ? &span : NULL, tunits ? &period : NULL, box);
  return;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional
 * grid
 * @param[in] state State to increment
 */
void
tbox_tile_state_next(TboxGridState *state)
{
  if (!state || state->done)
      return;
  /* Move to the next tile */
  state->i++;
  state->value = datum_add(state->value, state->vsize, state->box.span.basetype);
  if (datum_gt(state->value, state->box.span.upper, state->box.span.basetype))
  {
    state->value = state->box.span.lower;
    state->t += state->tunits;
    if (state->t > DatumGetTimestampTz(state->box.period.upper))
    {
      state->done = true;
      return;
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
 * @param[in] torigin Time origin of the tile
 * @param[out] count Number of elements in the output array
 */
TBox *
tbox_tile_list(const TBox *box, Datum vsize, const Interval *duration,
  Datum vorigin, TimestampTz torigin, int *count)
{
  assert(box); assert(count);
  /* Ensure validity of the arguments */
  if (! ensure_positive_datum(vsize, box->span.basetype) ||
      (duration && ! ensure_valid_duration(duration)))
    return NULL;

  TboxGridState *state = tbox_tile_state_make(box, vsize, duration,
    vorigin, torigin);

  int nrows = 1, ncols = 1;
  Datum start_bucket, end_bucket;
  /* Determine the number of value buckets */
  double size = datum_double(vsize, box->span.basetype);
  if (size)
    nrows = span_no_buckets(&box->span, vsize, vorigin, &start_bucket,
      &end_bucket);
  /* Determine the number of time buckets */
  int64 tunits = duration ? interval_units(duration) : 0;
  if (tunits)
    ncols = tstzspan_no_buckets(&box->period, duration, torigin, &start_bucket,
      &end_bucket);
  /* Total number of tiles */
  int count1 = nrows * ncols;

  /* Compute the tiles */
  TBox *result = palloc0(sizeof(TBox) * count1);
  for (int i = 0; i < count1; i++)
  {
    tbox_tile_get(state->value, state->t, state->vsize, state->tunits,
      state->box.span.basetype, &result[i]);
    tbox_tile_state_next(state);
  }
  *count = count1;
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile list of a temporal integer box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_tile_list()
 */
TBox *
tintbox_tile_list(const TBox *box, int vsize, const Interval *duration,
  int vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) count))
    return NULL;
  return tbox_tile_list(box, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, count);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile list of a temporal float box
 * @param[in] box Input box to split
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] count Number of elements in the output array
 * @csqlfn #Tbox_tile_list()
 */
TBox *
tfloatbox_tile_list(const TBox *box, double vsize, const Interval *duration,
  double vorigin, TimestampTz torigin, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) count))
    return NULL;
  return tbox_tile_list(box, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, count);
}
#endif /* MEOS */

/**
 * @ingroup meos_internal_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[in] basetype Type of the values
 * @csqlfn #Tbox_tile()
 */
TBox *
tbox_tile(Datum value, TimestampTz t, Datum vsize, Interval *duration,
  Datum vorigin, TimestampTz torigin, meosType basetype)
{
  ensure_positive_datum(vsize, basetype);
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  Datum value_bucket = datum_bucket(value, vsize, vorigin, basetype);
  TimestampTz time_bucket = timestamptz_bucket(t, duration, torigin);
  TBox *result = palloc(sizeof(TBox));
  tbox_tile_get(value_bucket, time_bucket, vsize, tunits, basetype, result);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_tile()
 */
TBox *
tintbox_tile(int value, TimestampTz t, int vsize, Interval *duration,
  int vorigin, TimestampTz torigin)
{
  return tbox_tile(Int32GetDatum(value), t, Int32GetDatum(vsize), duration,
    Int32GetDatum(vorigin), torigin, T_INT4);
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the tile from the arguments
 * @param[in] value Value
 * @param[in] t Timestamp
 * @param[in] vsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] vorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @csqlfn #Tbox_tile()
 */
TBox *
tfloatbox_tile(double value, TimestampTz t, double vsize, Interval *duration,
  double vorigin, TimestampTz torigin)
{
  return tbox_tile(Float8GetDatum(value), t, Float8GetDatum(vsize), duration,
    Float8GetDatum(vorigin), torigin, T_FLOAT8);
}

/*****************************************************************************
 * Time split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * buckets
 * @param[in] inst Temporal value
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] count Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, int64 tunits, TimestampTz torigin,
  TimestampTz **buckets, int *count)
{
  assert(inst);
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = tinstant_copy(inst);
  times[0] = timestamptz_bucket1(inst->t, tunits, torigin);
  *buckets = times;
  *count = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to time
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start Start timestamp of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tnumberdiscseq_time_split(const TSequence *seq, TimestampTz start,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  assert(seq);
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
  *buckets = times;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to period
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bucket lower bounds
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tsequence_time_split_iter(const TSequence *seq, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TSequence **result,
  TimestampTz *times)
{
  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* This loop is needed for filtering unnecesary time buckets for the
   * sequences composing a sequence set.
   * The upper bound for the bucket is exclusive => the test below is >= */
  while (lower < end &&
    (DatumGetTimestampTz(seq->period.lower) >= upper ||
     lower > DatumGetTimestampTz(seq->period.upper) ||
     (lower == DatumGetTimestampTz(seq->period.upper) && ! seq->period.upper_inc)))
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
      nfrags = 0;  /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && (interp == LINEAR || i == seq->count - 1)))
    {
      instants[ninsts++] = inst;
      i++;
    }
    else
    {
      assert(ninsts > 0);
      /* Compute the value at the end of the bucket */
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
      lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
      times[nfrags] = lower;
      result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
         (ninsts > 1) ? false : true, interp, NORMALIZE);
      ninsts = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary buckets for the
       * sequences composing a sequence set */
      if (lower >= end || ! contains_span_timestamptz(&seq->period, lower))
        break;
      /* Reuse the end value of the previous bucket for the beginning of the bucket */
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
 * @brief Split a temporal value into an array of fragments according to period
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tsequence_time_split(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  assert(seq); assert(buckets); ensure_not_null((void *) newcount);
  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tsequence_time_split_iter(seq, start, end, tunits, count, result,
    times);
  *buckets = times;
  return result;
}

/**
 * @brief Split a temporal value into an array of disjoint fragments
 * @param[in] ss Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequenceSet **
tsequenceset_time_split(const TSequenceSet *ss, TimestampTz start,
  TimestampTz end, int64 tunits, int count, TimestampTz **buckets,
  int *newcount)
{
  assert(ss); assert(buckets); assert(newcount);
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    TSequence **sequences = tsequence_time_split(TSEQUENCESET_SEQ_N(ss, 0),
      start, end, tunits, count, buckets, newcount);
    TSequenceSet **result = palloc(sizeof(TSequenceSet *) * *newcount);
    for (int i = 0; i < *newcount; i++)
      result[i] = tsequence_to_tsequenceset_free(sequences[i]);
    pfree(sequences);
    return result;
  }

  /* General case */
  /* Sequences obtained by spliting one composing sequence */
  TSequence **sequences = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Start timestamp of buckets obtained by spliting one composing sequence */
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ss->count + count));
  /* Sequences composing the currently constructed bucket of the sequence set */
  TSequence **fragments = palloc(sizeof(TSequence *) * (ss->count * count));
  /* Sequences for the buckets of the sequence set */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  /* Variable used to adjust the start timestamp passed to the
   * tsequence_time_split1 function in the loop */
  TimestampTz lower = start;
  int nfrags = 0, /* Number of accumulated fragments of the current time bucket */
      nbucks = 0; /* Number of time buckets already processed */
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz upper = lower + tunits;
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Output the accumulated fragments of the current time bucket (if any)
     * if the current sequence starts on the next time bucket */
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
    /* Number of time buckets of the current sequence */
    int l = tsequence_time_split_iter(seq, lower, end, tunits, count,
      sequences, &times[nbucks]);
    /* If the current sequence has produced more than two time buckets */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bucket (if any)  */
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
  /* Process the accumulated fragments of the last time bucket */
  if (nfrags > 0)
  {
    result[nbucks++] = tsequenceset_make((const TSequence **) fragments, nfrags,
      NORMALIZE);
    for (int j = 0; j < nfrags; j++)
      pfree(fragments[j]);
  }
  pfree(sequences); pfree(fragments);
  *buckets = times;
  *newcount = nbucks;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal value split according to
 * time buckets
 * @param[in] temp Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static Temporal **
temporal_time_split1(const Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, TimestampTz torigin, int count, TimestampTz **buckets,
  int *newcount)
{
  assert(temp); assert(buckets); assert(newcount);
  assert(start < end);
  assert(count > 0);
  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tinstant_time_split((const TInstant *) temp,
        tunits, torigin, buckets, newcount);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberdiscseq_time_split((const TSequence *) temp,
          start, tunits, count, buckets, newcount) :
        (Temporal **) tsequence_time_split((const TSequence *) temp,
          start, end, tunits, count, buckets, newcount);
    default: /* TSEQUENCESET */
      return (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
        start, end, tunits, count, buckets, newcount);
  }
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time buckets
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] buckets Array of buckets
 * @param[out] count Number of values in the output array
 * @csqlfn #Temporal_time_split()
 */
Temporal **
temporal_time_split(Temporal *temp, Interval *duration, TimestampTz torigin,
  TimestampTz **buckets, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_valid_duration(duration))
    return NULL;

  Datum start_bucket, end_bucket;
  Span s;
  temporal_set_tstzspan(temp, &s);
  int nbuckets = tstzspan_no_buckets(&s, duration, torigin, &start_bucket,
    &end_bucket);
  int64 tunits = interval_units(duration);
  return temporal_time_split1(temp, DatumGetTimestampTz(start_bucket),
    DatumGetTimestampTz(end_bucket), tunits, torigin, nbuckets, buckets,
    count);
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Get the bucket number in the bucket space that contains the value
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[in] type Type of the arguments
 */
static int
bucket_position(Datum value, Datum size, Datum origin, meosType type)
{
  assert(tnumber_basetype(type));
  if (type == T_INT4) /** xx **/
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] inst Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] start_bucket Value of the start bucket
 * @param[out] buckets Start value of the buckets containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstant **
tnumberinst_value_split(const TInstant *inst, Datum start_bucket, Datum size,
  Datum **buckets, int *newcount)
{
  assert(inst); assert(buckets); assert(newcount);
  Datum value = tinstant_val(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = datum_bucket(value, size, start_bucket, basetype);
  *buckets = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] seq Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] start_bucket Value of the start bucket
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tnumberdiscseq_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  assert(seq); assert(buckets); assert(newcount);
  meosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bucket_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bucket(value, size, start_bucket, basetype);
    *buckets = values;
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
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    int bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    int inst_no = ninsts[bucket_no]++;
    instants[bucket_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bucket */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bucket_value = start_bucket;
  for (int i = 0; i < count; i++)
  {
    if (ninsts[i] > 0)
    {
      result[nfrags] = tsequence_make(&instants[i * seq->count], ninsts[i],
        true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype);
  }
  pfree(instants);
  pfree(ninsts);
  *buckets = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start_bucket Value of the start bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[in,out] result Array containing the fragments of each bucket
 * @param[in,out] nseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(! MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  meosType basetype = temptype_basetype(seq->temptype);
  Datum value, bucket_value;
  int bucket_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = nseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * count * seq->count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_val(inst1);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = nseqs[bucket_no]++;
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    int nfrags = 1;
    if (i < seq->count)
    {
      tofree[nfree++] = bounds[1] = tinstant_make(value, seq->temptype, inst2->t);
      nfrags++;
    }
    result[bucket_no * numcols + seq_no] = tsequence_make(
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
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = nseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tinstant_to_tsequence(inst1, STEP);
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start_bucket Value of the start bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[in,out] result Array containing the fragments of each bucket
 * @param[in,out] nseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 * @p seq->count for sequences or @p ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, TSequence **result, int *nseqs, int numcols)
{
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));
  meosType basetype = temptype_basetype(seq->temptype);
  meosType spantype = basetype_spantype(basetype);
  Datum value1, bucket_value1;
  int bucket_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
    bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
    seq_no = nseqs[bucket_no1]++;
    result[bucket_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_val(inst1);
  bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
  bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_val(inst2);
    Datum bucket_value2 = datum_bucket(value2, size, start_bucket, basetype);
    int bucket_no2 = bucket_position(bucket_value2, size, start_bucket, basetype);

    /* Set variables depending on whether the segment is constant, increasing,
     * or decreasing */
    Datum min_value, max_value;
    int first_bucket, last_bucket, first, last;
    bool lower_inc1, upper_inc1; /* Lower/upper bound inclusion of the segment */
    bool lower_inc_def, upper_inc_def; /* Default lower/upper bound inclusion */
    int cmp = datum_cmp(value1, value2, basetype);
    if (cmp <= 0)
    {
      /* Both for constant and increasing segments */
      min_value = value1;
      max_value = value2;
      first_bucket = bucket_no1;
      last_bucket = bucket_no2;
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
      first_bucket = bucket_no2;
      last_bucket = bucket_no1;
      first = 1;
      last = 0;
      lower_inc_def = false;
      upper_inc_def = true;
      lower_inc1 = (i == seq->count - 1) ? seq->period.upper_inc : false;
      upper_inc1 = (i == 1) ? seq->period.lower_inc : true;
    }

    /* Split the segment into buckets */
    span_set(min_value, max_value, lower_inc1, (cmp != 0) ? upper_inc1 : true,
      basetype, spantype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bucket_lower = (cmp <= 0) ? bucket_value1 : bucket_value2;
    Datum bucket_upper = datum_add(bucket_lower, size, basetype);
    for (int j = first_bucket; j <= last_bucket; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bucket_upper, basetype) &&
        datum_lt(bucket_upper, max_value, basetype))
      {
        TimestampTz t;
        Datum projvalue;
        tlinearsegm_intersection_value(inst1, inst2, bucket_upper, basetype,
          &projvalue, &t);
        /* To reduce the roundoff errors we take the value projected to the
         * timestamp instead of the bound value */
        tofree[nfree++] = bounds[last] =
          tinstant_make(projvalue, seq->temptype, t);
      }
      else
        bounds[last] = (cmp <= 0) ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bucket || j == last_bucket)
      {
        Span bucketspan;
        span_set(bucket_lower, bucket_upper, true, false, basetype, spantype,
          &bucketspan);
        Span inter;
        bool found = inter_span_span(&segspan, &bucketspan, &inter);
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
        /* Sequence bounds are the bucket bounds */
        lower_inc1 = lower_inc_def;
        upper_inc1 = upper_inc_def;
      }
      /* If last bucket contains a single instant */
      int nfrags = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bucket if last instant has exclusive bound */
      if (nfrags == 1 && ! upper_inc1)
        break;
      seq_no = nseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        nfrags, (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bucket_lower = bucket_upper;
      bucket_upper = datum_add(bucket_upper, size, basetype);
    }
    inst1 = inst2;
    value1 = value2;
    bucket_value1 = bucket_value2;
    bucket_no1 = bucket_no2;
  }
  pfree_array((void **) tofree, nfree);
  return;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] seq Temporal value
 * @param[in] start_bucket Value of the start bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing the fragments
 * @param[out] newcount Number of elements in output arrays
 */
static TSequenceSet **
tnumbercontseq_value_split(const TSequence *seq, Datum start_bucket, Datum size,
  int count, Datum **buckets, int *newcount)
{
  assert(seq); assert(buckets); assert(newcount);
  meosType basetype = temptype_basetype(seq->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_to_tsequenceset(seq);
    Datum value = tinstant_val(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bucket, size, count, sequences,
      nseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bucket, size, count, sequences,
      nseqs, seq->count);
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)(&sequences[seq->count * i]),
        nseqs[i], NORMALIZE);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype);
  }
  pfree(sequences);
  pfree(nseqs);
  *buckets = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] ss Temporal value
 * @param[in] start_bucket Start value of the first bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[out] buckets Array of start values of the buckets containing the
 * fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  assert(ss); assert(buckets); assert(newcount);
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumbercontseq_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bucket,
      size, count, buckets, newcount);

  /* General case */
  meosType basetype = temptype_basetype(ss->temptype);
  TSequence **bucketseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *nseqs = palloc0(sizeof(int) * count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (MEOS_FLAGS_LINEAR_INTERP(ss->flags))
      tnumberseq_linear_value_split(seq, start_bucket, size, count, bucketseqs,
        nseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bucket, size, count, bucketseqs,
        nseqs, ss->totalcount);
  }
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (nseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)(&bucketseqs[i * ss->totalcount]),
        nseqs[i], NORMALIZE);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype);
  }
  pfree(bucketseqs);
  pfree(nseqs);
  *buckets = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] vorigin Origin of the value buckets
 * @param[out] buckets Array of start values of the buckets containing the
 * fragments
 * @param[out] count Number of values in the output arrays
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum size, Datum vorigin,
  Datum **buckets, int *count)
{
  assert(temp); assert(buckets); assert(count);
  /* Compute the value bounds */
  Span s;
  Datum start_bucket, end_bucket;
  tnumber_set_span(temp, &s);
  int nbuckets = span_no_buckets(&s, size, vorigin, &start_bucket,
      &end_bucket);

  /* Split the temporal value */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return (Temporal **) tnumberinst_value_split((const TInstant *) temp,
        start_bucket, size, buckets, count);
    case TSEQUENCE:
      return MEOS_FLAGS_DISCRETE_INTERP(temp->flags) ?
        (Temporal **) tnumberdiscseq_value_split((const TSequence *) temp,
          start_bucket, size, nbuckets, buckets, count) :
        (Temporal **) tnumbercontseq_value_split((const TSequence *) temp,
          start_bucket, size, nbuckets, buckets, count);
    default: /* TSEQUENCESET */
      return (Temporal **) tnumberseqset_value_split((const TSequenceSet *) temp,
        start_bucket, size, nbuckets, buckets, count);
  }
}

/*****************************************************************************/

/**
 * @brief Return a temporal value split according to a base value and possibly
 * a temporal grid
 */
Temporal **
tnumber_value_time_split(Temporal *temp, Datum size, Interval *duration,
  Datum vorigin, TimestampTz torigin, Datum **value_buckets,
  TimestampTz **time_buckets, int *count)
{
  meosType basetype = temptype_basetype(temp->temptype);
  meosType spantype = basetype_spantype(basetype);
  ensure_positive_datum(size, basetype);
  ensure_valid_duration(duration);

  Span s;
  Datum start_bucket, end_bucket, start_time_bucket, end_time_bucket;
  /* Compute the value bounds */
  tnumber_set_span(temp, &s);
  int value_count = span_no_buckets(&s, size, vorigin, &start_bucket,
      &end_bucket);
  /* Compute the time bounds */
  temporal_set_tstzspan(temp, &s);
  int time_count = tstzspan_no_buckets(&s, duration, torigin, &start_time_bucket,
      &end_time_bucket);
  TimestampTz start_time = DatumGetTimestampTz(start_time_bucket);
  TimestampTz end_time = DatumGetTimestampTz(end_time_bucket);
  int64 tunits = interval_units(duration);

  /* Total number of tiles */
  int ntiles = value_count * time_count;

  /* Split the temporal value */
  Datum *v_buckets = NULL;
  TimestampTz *t_buckets = NULL;
  Temporal **fragments;
  v_buckets = palloc(sizeof(Datum) * ntiles);
  t_buckets = palloc(sizeof(TimestampTz) * ntiles);
  fragments = palloc(sizeof(Temporal *) * ntiles);
  int nfrags = 0;
  Datum lower_value = start_bucket;
  while (datum_lt(lower_value, end_bucket, basetype))
  {
    Datum upper_value = datum_add(lower_value, size, basetype);
    Span s;
    span_set(lower_value, upper_value, true, false, basetype, spantype, &s);
    Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
    if (atspan != NULL)
    {
      int num_time_splits;
      TimestampTz *times;
      Temporal **time_splits = temporal_time_split1(atspan, start_time,
        end_time, tunits, torigin, time_count, &times, &num_time_splits);
      for (int i = 0; i < num_time_splits; i++)
      {
        v_buckets[i + nfrags] = lower_value;
        t_buckets[i + nfrags] = times[i];
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
  if (value_buckets)
    *value_buckets = v_buckets;
  if (time_buckets)
    *time_buckets = t_buckets;
  return fragments;
}

#if MEOS
/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] origin Time origin of the buckets
 * @param[out] buckets Array of buckets
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tint_value_split(Temporal *temp, int size, int origin, int **buckets,
  int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size))
    return NULL;

  Datum *datum_buckets;
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(size),
    Int32GetDatum(origin), &datum_buckets, count);
  /* Transform the datum buckets into float buckets and return */
  int *values = palloc(sizeof(int) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_buckets[i]);
  if (buckets)
    *buckets = values;
  pfree(datum_buckets);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal float split according to value
 * buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] origin Time origin of the buckets
 * @param[out] buckets Array of buckets
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_split()
 */
Temporal **
tfloat_value_split(Temporal *temp, double size, double origin,
  double **buckets, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8))
    return NULL;

  Datum *datum_buckets;
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(size),
    Float8GetDatum(origin), &datum_buckets, count);
  /* Transform the datum buckets into float buckets and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_buckets[i]);
  if (buckets)
    *buckets = values;
  pfree(datum_buckets);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] duration Size of the time buckets
 * @param[in] vorigin Time origin of the buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] value_buckets Array of value buckets
 * @param[out] time_buckets Array of time buckets
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tint_value_time_split(Temporal *temp, int size, Interval *duration,
  int vorigin, TimestampTz torigin, int **value_buckets,
  TimestampTz **time_buckets, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TINT) || ! ensure_positive(size) ||
      ! ensure_valid_duration(duration))
    return NULL;
  Datum *datum_buckets;
  Temporal **result = tnumber_value_time_split(temp, Int32GetDatum(size),
    duration, Int32GetDatum(vorigin), torigin, &datum_buckets, time_buckets,
    count);

  /* Transform the datum buckets into float buckets and return */
  int *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetInt32(datum_buckets[i]);
  if (value_buckets)
    *value_buckets = values;
  pfree(datum_buckets);
  return result;
}

/**
 * @ingroup meos_temporal_analytics_tile
 * @brief Return the fragments of a temporal integer split according to value
 * and time buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] duration Size of the time buckets
 * @param[in] vorigin Time origin of the buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] value_buckets Array of value buckets
 * @param[out] time_buckets Array of time buckets
 * @param[out] count Number of values in the output array
 * @csqlfn #Tnumber_value_time_split()
 */
Temporal **
tfloat_value_time_split(Temporal *temp, double size, Interval *duration,
  double vorigin, TimestampTz torigin, double **value_buckets,
  TimestampTz **time_buckets, int *count)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) count) ||
      ! ensure_temporal_isof_type(temp, T_TFLOAT) ||
      ! ensure_positive_datum(Float8GetDatum(size), T_FLOAT8) ||
      ! ensure_valid_duration(duration))
    return NULL;

  Datum *datum_buckets;
  Temporal **result = tnumber_value_time_split(temp, Float8GetDatum(size),
    duration, Float8GetDatum(vorigin), torigin, &datum_buckets, time_buckets,
    count);

  /* Transform the datum buckets into float buckets and return */
  double *values = palloc(sizeof(double) * *count);
  for (int i = 0; i < *count; i++)
    values[i] = DatumGetFloat8(datum_buckets[i]);
  if (value_buckets)
    *value_buckets = values;
  pfree(datum_buckets);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
