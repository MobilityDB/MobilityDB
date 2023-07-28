/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Bucket and tile functions for temporal types.
 *
 * @note The time bucket functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "general/temporal_tile.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/type_util.h"

/*****************************************************************************
 * Span bucket functions
 *****************************************************************************/

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
    datum_add(lower, size, basetype, basetype);
  span_set(lower, upper, true, false, basetype, span);
  return;
}

/**
 * @brief Generate an integer or float span bucket from a bucket list
 * @param[in] lower Start value of the bucket
 * @param[in] size Size of the buckets
 * @param[in] basetype Type of the arguments
 */
Span *
span_bucket_get(Datum lower, Datum size, meosType basetype)
{
  Span *result = palloc(sizeof(Span));
  span_bucket_set(lower, size, basetype, result);
  return result;
}


/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] s Bounds for generating the bucket list
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 *
 * @pre The size argument must be greater to 0.
 * @note The first argument is NULL when generating the bucket list, otherwise
 * it is a temporal number to be split and in this case s is the value span
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
  /* intspans are in canonical form so their upper bound is exclusive */
  Datum upper = (s->basetype == T_INT4) ? /** xx **/
    Int32GetDatum(DatumGetInt32(s->upper) - 1) : s->upper;
  state->minvalue = state->value =
    datum_bucket(s->lower, size, origin, state->basetype);
  state->maxvalue = datum_bucket(upper, size, origin, state->basetype);
  return state;
}

/**
 * @brief Increment the current state to the next bucket of the bucket list@brief
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
    datum_add(state->value, state->size, state->basetype, state->basetype);
  if (datum_gt(state->value, state->maxvalue, state->basetype))
    state->done = true;
  return;
}

/*****************************************************************************
 * Bucket functions
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the initial value of the bucket in which an integer value falls.
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 */
int
int_bucket(int value, int size, int origin)
{
  assert(size > 0);
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
      elog(ERROR, "number out of span");
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
      elog(ERROR, "number out of span");
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the initial value of the bucket in which a float value falls.
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 */
double
float_bucket(double value, double size, double origin)
{
  assert(size > 0.0);
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
      elog(ERROR, "number out of span");
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
 * @brief Return the initial timestamp of the bucket in which a timestamp falls
 * @param[in] t Input timestamp
 * @param[in] size Size of the time buckets in PostgreSQL time units
 * @param[in] origin Origin of the buckets
 */
TimestampTz
timestamptz_bucket1(TimestampTz t, int64 size, TimestampTz origin)
{
  if (TIMESTAMP_NOT_FINITE(t))
    elog(ERROR, "timestamp out of span");
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
      elog(ERROR, "timestamp out of span");
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
      elog(ERROR, "timestamp out of span");
    }
    else
      result -= size;
  }
  result += origin;
  return result;
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the initial timestamp of the bucket in which a timestamp falls.
 * @param[in] t Input timestamp
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] origin Origin of the buckets
 */
TimestampTz
timestamptz_bucket(TimestampTz t, const Interval *duration, TimestampTz origin)
{
  ensure_valid_duration(duration);
  int64 size = interval_units(duration);
  return timestamptz_bucket1(t, size, origin);
}

/**
 * @brief Return the initial value of the bucket in which a number value falls
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[in] basetype Data type of the arguments
 */
Datum
datum_bucket(Datum value, Datum size, Datum origin, meosType basetype)
{
  ensure_positive_datum(size, basetype);
  assert(span_basetype(basetype));
  if (basetype == T_INT4)
    return Int32GetDatum(int_bucket(DatumGetInt32(value),
      DatumGetInt32(size), DatumGetInt32(origin)));
  else if (basetype == T_FLOAT8)
    return Float8GetDatum(float_bucket(DatumGetFloat8(value),
      DatumGetFloat8(size), DatumGetFloat8(origin)));
  else /* basetype == T_TIMESTAMPTZ */
    return TimestampTzGetDatum(timestamptz_bucket1(DatumGetTimestampTz(value),
      DatumGetInt64(size), DatumGetTimestampTz(origin)));
}

/*****************************************************************************
 * Bucket list functions
 *****************************************************************************/

#if MEOS
/**
 * @brief Return the bucket list from a span.
 * @param[in] bounds Input span to split
 * @param[in] size Bucket size
 * @param[in] origin Origin of the buckets
 * @param[out] count Number of elements in the output array
 */

Span *
span_bucket_list(const Span *bounds, Datum size, Datum origin, int count)
{
  SpanBucketState *state = span_bucket_state_make(bounds, size, origin);
  Span *buckets = palloc0(sizeof(Span) * count);
  for (int i = 0; i < count; i++)
  {
    span_bucket_set(state->value, state->size, state->basetype, &buckets[i]);
    span_bucket_state_next(state);
  }
  return buckets;
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the bucket list from an integer span.
 * @param[in] bounds Input span to split
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] newcount Number of elements in the output array
 */
Span *
intspan_bucket_list(const Span *bounds, int size, int origin, int *newcount)
{
  *newcount = ceil((DatumGetInt32(bounds->upper) -
    DatumGetInt32(bounds->lower)) / size);
  return span_bucket_list(bounds, Int32GetDatum(size), Int32GetDatum(origin),
    *newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the bucket list from an integer span
 * @param[in] bounds Input span to split
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] newcount Number of elements in the output array
 */
Span *
floatspan_bucket_list(const Span *bounds, double size, double origin,
  int *newcount)
{
  *newcount = ceil((DatumGetFloat8(bounds->upper) -
     DatumGetFloat8(bounds->lower)) / size);
  return span_bucket_list(bounds, Float8GetDatum(size),
    Float8GetDatum(origin), *newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Return the bucket list from a period
 * @param[in] bounds Input span to split
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[out] newcount Number of elements in the output array
 */
Span *
period_bucket_list(const Span *bounds, const Interval *duration,
  TimestampTz origin, int *newcount)
{
  ensure_valid_duration(duration);
  int64 size = interval_units(duration);
  *newcount = ceil((DatumGetTimestampTz(bounds->upper) -
    DatumGetTimestampTz(bounds->lower)) / size);
  return span_bucket_list(bounds, Int64GetDatum(size),
    TimestampTzGetDatum(origin), *newcount);
}
#endif /* MEOS */

/*****************************************************************************
 * TBox tile functions
 *****************************************************************************/

/**
 * @brief Generate a tile from the a multidimensional grid
 * @param[in] value Start value of the tile to output
 * @param[in] t Start timestamp of the tile to output
 * @param[in] xsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[out] box Output box
 */
void
tbox_tile_get(double value, TimestampTz t, double xsize, int64 tunits,
  TBox *box)
{
  Datum xmin = Float8GetDatum(value);
  Datum xmax = Float8GetDatum(value + xsize);
  Datum tmin = TimestampTzGetDatum(t);
  Datum tmax = TimestampTzGetDatum(t + tunits);
  Span period;
  Span span;
  span_set(tmin, tmax, true, false, T_TIMESTAMPTZ, &period);
  span_set(xmin, xmax, true, false, T_FLOAT8, &span);
  tbox_set(&span, &period, box);
  return;
}

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] xsize Value size of the tiles
 * @param[in] duration Interval defining the time size of the tile
 * @param[in] xorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre Both xsize and tunits must be greater than 0.
 */
TboxGridState *
tbox_tile_state_make(const TBox *box, double xsize, const Interval *duration,
  double xorigin, TimestampTz torigin)
{
  int64 tunits = interval_units(duration);
  assert(xsize > 0 || tunits > 0);
  TboxGridState *state = palloc0(sizeof(TboxGridState));

  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->xsize = xsize;
  state->tunits = tunits;
  if (xsize)
  {
    state->box.span.lower = Float8GetDatum(float_bucket(
      DatumGetFloat8(box->span.lower), xsize, xorigin));
    double upper = DatumGetFloat8(box->span.upper);
    double upper_bucket = float_bucket(upper, xsize, xorigin);
    if (upper == upper_bucket && ! box->span.upper_inc)
      upper_bucket -= xsize;
    state->box.span.upper = Float8GetDatum(upper_bucket);
  }
  if (tunits)
  {
    state->box.period.lower = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.lower), duration, torigin));
    state->box.period.upper = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.upper), duration, torigin));
  }
  state->value = DatumGetFloat8(state->box.span.lower);
  state->t = DatumGetTimestampTz(state->box.period.lower);
  return state;
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
  state->value += state->xsize;
  if (state->value > DatumGetFloat8(state->box.span.upper))
  {
    state->value = DatumGetFloat8(state->box.span.lower);
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
 * @ingroup libmeos_temporal_tile
 * @brief Generate a multidimensional grid for temporal numbers.
 * @param[in] bounds Input span to split
 * @param[in] xsize Value size of the tiles
 * @param[in] duration Interval defining the temporal size of the tiles
 * @param[in] xorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tile
 * @param[out] rows,columns Number of rows and columns in the output array
 */

TBox *
tbox_tile_list(const TBox *bounds, double xsize, const Interval *duration,
  double xorigin, TimestampTz torigin, int *rows, int *columns)
{
  // TODO: generalize for intspan
  ensure_positive_datum(Float8GetDatum(xsize), bounds->span.basetype);
  ensure_valid_duration(duration);
  int64 tsize = interval_units(duration);
  TboxGridState *state = tbox_tile_state_make(bounds, xsize, duration,
    xorigin, torigin);
  int no_rows = ceil((DatumGetFloat8(bounds->span.upper) -
    DatumGetFloat8(bounds->span.lower)) / xsize);
  int no_cols = ceil((DatumGetTimestampTz(bounds->period.upper) -
    DatumGetTimestampTz(bounds->period.lower)) / tsize);
  TBox *result = palloc0(sizeof(TBox) * no_rows * no_cols);
  for (int i = 0; i < no_rows * no_cols; i++)
  {
    tbox_tile_get(state->value, state->t, state->xsize, state->tunits,
      &result[i]);
    tbox_tile_state_next(state);
  }
  *rows = no_rows;
  *columns = no_cols;
  return result;
}

/**
 * @brief Return the grid list from a span and a period.
 * @param[in] bounds Input value span to split
 * @param[in] xsize Value size of the tiles
 * @param[in] duration Interval defining the size of the buckets
 * @param[in] xorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 * @param[out] rows,columns Number of rows and columns in the output array
 */
TBox *
floatspan_period_tile_list(const TBox *bounds, double xsize,
  const Interval *duration, double xorigin, TimestampTz torigin, int *rows,
  int *columns)
{
  return tbox_tile_list(bounds, Float8GetDatum(xsize), duration,
    Float8GetDatum(xorigin), torigin, rows, columns);
}
#endif /* MEOS */

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
 * @param[out] newcount Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, int64 tunits, TimestampTz torigin,
  TimestampTz **buckets, int *newcount)
{
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = tinstant_copy(inst);
  times[0] = timestamptz_bucket1(inst->t, tunits, torigin);
  *buckets = times;
  *newcount = 1;
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
tnumberseq_disc_time_split(const TSequence *seq, TimestampTz start,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
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
tsequence_time_split1(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TSequence **result, TimestampTz *times)
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
  bool linear = MEOS_FLAGS_GET_LINEAR(seq->flags);
  int i = 0,      /* counter for instants of temporal value */
      ninsts = 0, /* counter for instants of next split */
      nfree = 0,  /* counter for instants to free */
      nfrags = 0;  /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && (linear || i == seq->count - 1)))
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
        if (linear)
          tofree[nfree] = tsegment_at_timestamp(instants[ninsts - 1], inst,
            linear, upper);
        else
        {
          /* The last two values of sequences with step interpolation and
           * exclusive upper bound must be equal */
          Datum value = tinstant_value(instants[ninsts - 1]);
          tofree[nfree] = tinstant_make(value, seq->temptype, upper);
        }
        instants[ninsts++] = tofree[nfree++];
      }
      lower_inc1 = (nfrags == 0) ? seq->period.lower_inc : true;
      times[nfrags] = lower;
      result[nfrags++] = tsequence_make(instants, ninsts, lower_inc1,
         (ninsts > 1) ? false : true, linear ? LINEAR : STEP, NORMALIZE);
      ninsts = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary buckets for the
       * sequences composing a sequence set */
      if (lower >= end || ! contains_period_timestamp(&seq->period, lower))
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
      seq->period.upper_inc, linear ? LINEAR : STEP, NORMALIZE);
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
  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tsequence_time_split1(seq, start, end, tunits, count,
    result, times);
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
  /* Singleton sequence set */
  if (ss->count == 1)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, 0);
    TSequence **sequences = tsequence_time_split(seq, start, end, tunits,
      count, buckets, newcount);
    TSequenceSet **result = palloc(sizeof(TSequenceSet *) * *newcount);
    for (int i = 0; i < *newcount; i++)
      result[i] = tsequence_to_tsequenceset(sequences[i]);
    pfree_array((void **) sequences, *newcount);
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
    int l = tsequence_time_split1(seq, lower, end, tunits, count,
      sequences, &times[nbucks]);
    /* If the current sequence has produced more than two time buckets */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bucket (if any)  */
      if (nfrags > 0)
      {
        fragments[nfrags++] = sequences[0];
        result[nbucks++] = tsequenceset_make((const TSequence **) fragments,
          nfrags, NORMALIZE);
        for (int j = 0; j < nfrags; j++)
          pfree(fragments[j]);
        nfrags = 0;
      }
      else
      {
        result[nbucks++] = tsequence_to_tsequenceset(sequences[0]);
        pfree(sequences[0]);
      }
      for (int j = 1; j < l - 1; j++)
      {
        result[nbucks++] = tsequence_to_tsequenceset(sequences[j]);
        pfree(sequences[j]);
      }
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
 * @brief Split a temporal value into fragments with respect to period buckets
 * @param[in] temp Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] torigin Time origin of the tiles
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 * @sqlfunc timeSplit()
 */
Temporal **
temporal_time_split1(const Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, TimestampTz torigin, int count, TimestampTz **buckets,
  int *newcount)
{
  assert(start < end);
  assert(count > 0);
  /* Split the temporal value */
  Temporal **fragments;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    fragments = (Temporal **) tinstant_time_split((const TInstant *) temp,
      tunits, torigin, buckets, newcount);
  else if (temp->subtype == TSEQUENCE)
    fragments = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal **) tnumberseq_disc_time_split((const TSequence *) temp,
        start, tunits, count, buckets, newcount) :
      (Temporal **) tsequence_time_split((const TSequence *) temp,
        start, end, tunits, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
      start, end, tunits, count, buckets, newcount);
  return fragments;
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/*****************************************************************************/

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
  Datum value = tinstant_value(inst);
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
tnumberseq_disc_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  meosType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bucket_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *numinsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    value = tinstant_value(inst);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    int bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    int inst_no = numinsts[bucket_no]++;
    instants[bucket_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bucket */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int nfrags = 0;
  bucket_value = start_bucket;
  for (int i = 0; i < count; i++)
  {
    if (numinsts[i] > 0)
    {
      result[nfrags] = tsequence_make(&instants[i * seq->count], numinsts[i],
        true, true, DISCRETE, NORMALIZE_NO);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(instants);
  pfree(numinsts);
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
 * @param[in,out] numseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 *    seq->count for sequences or ss->totalcount for sequence sets
 */
static void
tnumberseq_step_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, TSequence **result, int *numseqs, int numcols)
{
  assert(! MEOS_FLAGS_GET_LINEAR(seq->flags));
  meosType basetype = temptype_basetype(seq->temptype);
  Datum value, bucket_value;
  int bucket_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * count * seq->count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value(inst1);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
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
    value = tinstant_value(inst1);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
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
 * @param[in,out] numseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 *    seq->count for sequences or ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, TSequence **result, int *numseqs, int numcols)
{
  assert(MEOS_FLAGS_GET_LINEAR(seq->flags));
  meosType basetype = temptype_basetype(seq->temptype);
  Datum value1, bucket_value1;
  int bucket_no1, seq_no;
  Span segspan;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
    bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no1]++;
    result[bucket_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int nfree = 0;   /* counter for the instants to free */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  value1 = tinstant_value(inst1);
  bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
  bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
  /* For each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    Datum value2 = tinstant_value(inst2);
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
      basetype, &segspan);
    TInstant *bounds[2];
    bounds[first] = (cmp <= 0) ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bucket_lower = (cmp <= 0) ? bucket_value1 : bucket_value2;
    Datum bucket_upper = datum_add(bucket_lower, size, basetype, basetype);
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
        span_set(bucket_lower, bucket_upper, true, false, basetype,
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
      seq_no = numseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        nfrags, (nfrags > 1) ? lower_inc1 : true, (nfrags > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bucket_lower = bucket_upper;
      bucket_upper = datum_add(bucket_upper, size, basetype, basetype);
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
tnumberseq_value_split(const TSequence *seq, Datum start_bucket, Datum size,
  int count, Datum **buckets, int *newcount)
{
  meosType basetype = temptype_basetype(seq->temptype);
  interpType interp = MEOS_FLAGS_GET_INTERP(seq->flags);
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_to_tsequenceset(seq);
    Datum value = tinstant_value(TSEQUENCE_INST_N(seq, 0));
    values[0] = datum_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *numseqs = palloc0(sizeof(int) * count);
  if (interp == LINEAR)
    tnumberseq_linear_value_split(seq, start_bucket, size, count, sequences,
      numseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bucket, size, count, sequences,
      numseqs, seq->count);
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (numseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)(&sequences[seq->count * i]),
        numseqs[i], NORMALIZE);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(sequences);
  pfree(numseqs);
  *buckets = values;
  *newcount = nfrags;
  return result;}

/*****************************************************************************/

/**
 * @brief Split a temporal value into an array of fragments according to value
 * buckets
 * @param[in] ss Temporal value
 * @param[in] start_bucket Start value of the first bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing the fragments
 * @param[out] newcount Number of values in the output arrays
 */
static TSequenceSet **
tnumberseqset_value_split(const TSequenceSet *ss, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  /* Singleton sequence set */
  if (ss->count == 1)
    return tnumberseq_value_split(TSEQUENCESET_SEQ_N(ss, 0), start_bucket,
      size, count, buckets, newcount);

  /* General case */
  meosType basetype = temptype_basetype(ss->temptype);
  TSequence **bucketseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *numseqs = palloc0(sizeof(int) * count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (MEOS_FLAGS_GET_LINEAR(ss->flags))
      tnumberseq_linear_value_split(seq, start_bucket, size, count, bucketseqs,
        numseqs, ss->totalcount);
    else
      tnumberseq_step_value_split(seq, start_bucket, size, count, bucketseqs,
        numseqs, ss->totalcount);
  }
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int nfrags = 0;
  for (int i = 0; i < count; i++)
  {
    if (numseqs[i] > 0)
    {
      result[nfrags] = tsequenceset_make((const TSequence **)(&bucketseqs[i * ss->totalcount]),
        numseqs[i], NORMALIZE);
      values[nfrags++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(bucketseqs);
  pfree(numseqs);
  *buckets = values;
  *newcount = nfrags;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * buckets
 */
Temporal **
tnumber_value_split1(const Temporal *temp, Datum start_bucket, Datum size,
  int count, Datum **buckets, int *newcount)
{
  assert(count > 0);
  /* Split the temporal value */
  Temporal **fragments;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    fragments = (Temporal **) tnumberinst_value_split((const TInstant *) temp,
      start_bucket, size, buckets, newcount);
  else if (temp->subtype == TSEQUENCE)
    fragments = MEOS_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal **) tnumberseq_disc_value_split((const TSequence *) temp,
        start_bucket, size, count, buckets, newcount) :
      (Temporal **) tnumberseq_value_split((const TSequence *) temp,
        start_bucket, size, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tnumberseqset_value_split((const TSequenceSet *) temp,
      start_bucket, size, count, buckets, newcount);
  return fragments;
}

/*****************************************************************************/

/**
 * @brief Split a temporal value with respect to a base value and possibly a
 * temporal grid
 */
Temporal **
temporal_value_time_split1(Temporal *temp, Datum size, Interval *duration,
  Datum vorigin, TimestampTz torigin, bool valuesplit, bool timesplit,
  Datum **value_buckets, TimestampTz **time_buckets, int *newcount)
{
  meosType basetype = temptype_basetype(temp->temptype);
  int64 tunits = 0;
  if (valuesplit)
    ensure_positive_datum(size, basetype);
  if (timesplit)
  {
    ensure_valid_duration(duration);
    tunits = interval_units(duration);
  }

  /* Compute the value bounds, if any */
  Datum start_bucket = Float8GetDatum(0);
  Datum end_bucket = Float8GetDatum(0);
  int value_count = 1;
  if (valuesplit)
  {
    Span *span = tnumber_to_span((const Temporal *) temp);
    Datum start_value = span->lower;
    /* We need to add size to obtain the end value of the last bucket */
    Datum end_value = datum_add(span->upper, size, basetype, basetype);
    start_bucket = datum_bucket(start_value, size, vorigin, basetype);
    end_bucket = datum_bucket(end_value, size, vorigin, basetype);
    value_count = (basetype == T_INT4) ? /** xx **/
      (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) /
        DatumGetInt32(size) :
      (int) (floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) /
        DatumGetFloat8(size)));
  }

  /* Compute the time bounds, if any */
  TimestampTz start_time_bucket = 0, end_time_bucket = 0;
  int time_count = 1;
  if (timesplit)
  {
    Span p;
    temporal_set_period(temp, &p);
    TimestampTz start_time = p.lower;
    TimestampTz end_time = p.upper;
    start_time_bucket = timestamptz_bucket(start_time, duration, torigin);
    /* We need to add tunits to obtain the end timestamp of the last bucket */
    end_time_bucket = timestamptz_bucket(end_time, duration, torigin) + tunits;
    time_count =
      (int) (((int64) end_time_bucket - (int64) start_time_bucket) / tunits);
  }

  /* Adjust the number of tiles */
  int tilecount = value_count * time_count;

  /* Split the temporal value */
  Datum *v_buckets = NULL;
  TimestampTz *t_buckets = NULL;
  Temporal **fragments;
  int count = 0;
  if (valuesplit && ! timesplit)
  {
    fragments = tnumber_value_split1(temp, start_bucket, size, tilecount,
      &v_buckets, &count);
  }
  else if (! valuesplit && timesplit)
  {
    fragments = temporal_time_split1(temp, start_time_bucket, end_time_bucket,
      tunits, torigin, tilecount, &t_buckets, &count);
  }
  else /* valuesplit && timesplit */
  {
    v_buckets = palloc(sizeof(Datum) * tilecount);
    t_buckets = palloc(sizeof(TimestampTz) * tilecount);
    fragments = palloc(sizeof(Temporal *) * tilecount);
    int nfrags = 0;
    Datum lower_value = start_bucket;
    while (datum_lt(lower_value, end_bucket, basetype))
    {
      Datum upper_value = datum_add(lower_value, size, basetype, basetype);
      Span s;
      span_set(lower_value, upper_value, true, false, basetype, &s);
      Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
      if (atspan != NULL)
      {
        int num_time_splits;
        TimestampTz *times;
        Temporal **time_splits = temporal_time_split1(atspan,
          start_time_bucket, end_time_bucket, tunits, torigin, time_count,
          &times, &num_time_splits);
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
    count = nfrags;
  }
  *newcount = count;
  if (value_buckets)
    *value_buckets = v_buckets;
  if (time_buckets)
    *time_buckets = t_buckets;
  return fragments;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal integer into fragments with respect to value buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] origin Time origin of the buckets
 * @param[out] newcount Number of values in the output array
 * @sqlfunc valueSplit()
 */
Temporal **
tint_value_split(Temporal *temp, int size, int origin, int *newcount)
{
  Datum *value_buckets;
  return temporal_value_time_split1(temp, Int32GetDatum(size), NULL,
    Int32GetDatum(origin), 0, true, false, &value_buckets, NULL, newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal float into fragments with respect to value buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] origin Time origin of the buckets
 * @param[out] newcount Number of values in the output array
 * @sqlfunc valueSplit()
 */
Temporal **
tfloat_value_split(Temporal *temp, double size, double origin, int *newcount)
{
  Datum *value_buckets;
  return temporal_value_time_split1(temp, Float8GetDatum(size), NULL,
    Float8GetDatum(origin), 0, true, false, &value_buckets, NULL, newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal value into fragments with respect to period buckets
 * @param[in] temp Temporal value
 * @param[in] duration Size of the time buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] newcount Number of values in the output array
 * @sqlfunc timeSplit()
 */
Temporal **
temporal_time_split(Temporal *temp, Interval *duration, TimestampTz torigin,
  int *newcount)
{
  TimestampTz *time_buckets;
  return temporal_value_time_split1(temp, Float8GetDatum(0), duration,
    Float8GetDatum(0), torigin, false, true, NULL, &time_buckets, newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal integer into fragments with respect to value and
 * period buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] duration Size of the time buckets
 * @param[in] vorigin Time origin of the buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] newcount Number of values in the output array
 * @sqlfunc timeSplit()
 */
Temporal **
tint_value_time_split(Temporal *temp, int size, int vorigin,
  Interval *duration, TimestampTz torigin, int *newcount)
{
  Datum *value_buckets;
  TimestampTz *time_buckets;
  return temporal_value_time_split1(temp, Int32GetDatum(size), duration,
    Int32GetDatum(vorigin), torigin, true, true, &value_buckets, &time_buckets,
    newcount);
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal integer into fragments with respect to value and
 * period buckets
 * @param[in] temp Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] duration Size of the time buckets
 * @param[in] vorigin Time origin of the buckets
 * @param[in] torigin Time origin of the buckets
 * @param[out] newcount Number of values in the output array
 * @sqlfunc timeSplit()
 */
Temporal **
tfloat_value_time_split(Temporal *temp, double size, double vorigin,
  Interval *duration, TimestampTz torigin, int *newcount)
{
  Datum *value_buckets;
  TimestampTz *time_buckets;
  return temporal_value_time_split1(temp, Float8GetDatum(size), duration,
    Float8GetDatum(vorigin), torigin, true, true, &value_buckets, &time_buckets,
    newcount);
}
#endif /* MEOS */

/*****************************************************************************/
