/***********************************************************************
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
#include <funcapi.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Number bucket functions
 *****************************************************************************/

/**
 * Return the initial value of the bucket in which an integer value falls.
 *
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] offset Origin of the buckets
 */
static int
int_bucket(int value, int size, int offset)
{
  assert(size > 0);
  if (offset != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the offset is
     * applied: when the offset is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    offset = offset % size;
    if ((offset > 0 && value < PG_INT32_MIN + offset) ||
      (offset < 0 && value > PG_INT32_MAX + offset))
      elog(ERROR, "number out of span");
    value -= offset;
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
      elog(ERROR, "number out of span");
    else
      result -= size;
  }
  result += offset;
  return result;
}

/**
 * Return the initial value of the bucket in which a float value falls.
 *
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] offset Origin of the buckets
 */
double
float_bucket(double value, double size, double offset)
{
  assert(size > 0.0);
  if (offset != 0)
  {
    /*
     * We need to ensure that the value is in span _after_ the offset is
     * applied: when the offset is positive we need to make sure the resultant
     * value is at least the minimum integer value (PG_INT32_MIN) and when
     * negative that it is less than the maximum integer value (PG_INT32_MAX)
     */
    offset = fmod(offset, size);
    if ((offset > 0 && value < -1 * DBL_MAX + offset) ||
      (offset < 0 && value > DBL_MAX + offset))
      elog(ERROR, "number out of span");
    value -= offset;
  }
  float result = floor(value / size) * size;
  /* Notice that by using the floor function above we remove the need to
   * add the additional if needed for the integer case to take into account
   * that integer division truncates toward 0 in C99 */
  result += offset;
  return result;
}

/**
 * Return the initial value of the bucket in which a number value falls.
 *
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] offset Origin of the buckets
 * @param[in] basetype Data type of the arguments
 */
static Datum
number_bucket(Datum value, Datum size, Datum offset,
  mobdbType basetype)
{
  ensure_tnumber_basetype(basetype);
  if (basetype == T_INT4)
    return Int32GetDatum(int_bucket(DatumGetInt32(value),
      DatumGetInt32(size), DatumGetInt32(offset)));
  else /* basetype == T_FLOAT8 */
    return Float8GetDatum(float_bucket(DatumGetFloat8(value),
      DatumGetFloat8(size), DatumGetFloat8(offset)));
}

PG_FUNCTION_INFO_V1(Number_bucket);
/**
 * Return the initial value of the bucket in which an integer value falls.
 */
PGDLLEXPORT Datum
Number_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  ensure_positive_datum(size, basetype);
  Datum result = number_bucket(value, size, origin, basetype);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Timestamp bucket functions
 *****************************************************************************/

/**
 * Return the initial timestamp of the bucket in which a timestamp falls.
 *
 * @param[in] timestamp Input timestamp
 * @param[in] size Size of the time buckets in PostgreSQL time units
 * @param[in] offset Origin of the buckets
 */
TimestampTz
timestamptz_bucket(TimestampTz timestamp, int64 size,
  TimestampTz offset)
{
  assert(size > 0);
  if (offset != 0)
  {
    /*
     * We need to ensure that the timestamp is in span _after_ the offset is
     * applied: when the offset is positive we need to make sure the resultant
     * time is at least the minimum time value value (DT_NOBEGIN) and when
     * negative that it is less than the maximum time value (DT_NOEND)
     */
    offset = offset % size;
    if ((offset > 0 && timestamp < DT_NOBEGIN + offset) ||
      (offset < 0 && timestamp > DT_NOEND + offset))
      elog(ERROR, "timestamp out of span");
    timestamp -= offset;
  }
  TimestampTz result = (timestamp / size) * size;
  if (timestamp < 0 && timestamp % size)
  {
    /*
     * We need to subtract another size if remainder < 0 this only happens
     * if timestamp is negative to begin with and there is a remainder
     * after division. Need to subtract another size since division
     * truncates toward 0 in C99.
     */
    if (result < DT_NOBEGIN + size)
      elog(ERROR, "timestamp out of span");
    else
      result -= size;
  }
  result += offset;
  return result;
}

/**
 * Return the interval in the same representation as Postgres timestamps.
 */
int64
get_interval_units(Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

PG_FUNCTION_INFO_V1(Timestamptz_bucket);
/**
 * Return the initial timestamp of the bucket in which a timestamp falls.
 */
PGDLLEXPORT Datum
Timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz timestamp = PG_GETARG_TIMESTAMPTZ(0);
  if (TIMESTAMP_NOT_FINITE(timestamp))
    PG_RETURN_TIMESTAMPTZ(timestamp);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz result = timestamptz_bucket(timestamp, tunits, origin);
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Span bucket functions
 *****************************************************************************/

/**
 * Generate an integer or float span bucket from a bucket list
 *
 * @param[in] value Start value of the bucket
 * @param[in] size Size of the buckets
 * @param[in] basetype Type of the arguments
 */
static Span *
span_bucket_get(Datum value, Datum size, mobdbType basetype)
{
  Datum lower = value;
  Datum upper = datum_add(value, size, basetype, basetype);
  return span_make(lower, upper, true, false, basetype);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] temp Temporal number to split
 * @param[in] s Bounds for generating the bucket list
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 *
 * @pre The size argument must be greater to 0.
 * @note The first argument is NULL when generating the bucket list, otherwise
 * it is a temporal number to be split and in this case s is the value span
 * of the temporal number
 */
static SpanBucketState *
span_bucket_state_make(Temporal *temp, Span *s, Datum size, Datum origin)
{
  SpanBucketState *state = palloc0(sizeof(SpanBucketState));
  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->temp = temp;
  state->basetype = s->basetype;
  state->size = size;
  state->origin = origin;
  Datum upper = s->upper;
  /* intspans are in canonical form so their upper bound is exclusive */
  if (s->basetype == T_INT4)
    upper = Int32GetDatum(DatumGetInt32(s->upper) - 1);
  state->minvalue = number_bucket(s->lower, size, origin, state->basetype);
  state->maxvalue = number_bucket(upper, size, origin, state->basetype);
  state->value = state->minvalue;
  return state;
}

/**
 * Increment the current state to the next bucket of the bucket list
 *
 * @param[in] state State to increment
 */
static void
span_bucket_state_next(SpanBucketState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  state->value = datum_add(state->value, state->size, state->basetype,
    state->basetype);
  if (datum_gt(state->value, state->maxvalue, state->basetype))
    state->done = true;
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_bucket_list);
/**
 * Generate a span bucket list.
 */
PGDLLEXPORT Datum
Span_bucket_list(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  SpanBucketState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Span *bounds = PG_GETARG_SPAN_P(0);
    Datum size = PG_GETARG_DATUM(1);
    Datum origin = PG_GETARG_DATUM(2);

    /* Ensure parameter validity */
    mobdbType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
    ensure_positive_datum(size, type);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = span_bucket_state_make(NULL, bounds, size, origin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(bounds, 0);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all buckets */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store index */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate bucket */
  tuple_arr[1] = PointerGetDatum(span_bucket_get(state->value, state->size,
    state->basetype));
  /* Advance state */
  span_bucket_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_bucket);
/**
 * Generate an integer or float span bucket in a bucket list for spans.
*/
PGDLLEXPORT Datum
Span_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  mobdbType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  ensure_positive_datum(size, type);
  Datum origin = PG_GETARG_DATUM(2);
  Datum value_bucket = number_bucket(value, size, origin, type);
  Span *result = span_bucket_get(value_bucket, size, type);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Period bucket functions
 *****************************************************************************/

/**
 * Generate period bucket from a bucket list
 *
 * @param[in] t Start timestamp of the bucket
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 */
static Period *
period_bucket_get(TimestampTz t, int64 tunits)
{
  TimestampTz lower = t;
  TimestampTz upper = lower + tunits;
  return span_make(lower, upper, true, false, T_TIMESTAMPTZ);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] p Bounds for generating the bucket list
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] torigin Origin of the buckets
 *
 * @pre The tunits argument must be greater to 0.
 */
static PeriodBucketState *
period_bucket_state_make(Period *p, int64 tunits, TimestampTz torigin)
{
  assert(tunits > 0);
  PeriodBucketState *state = palloc0(sizeof(PeriodBucketState));

  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->tunits = tunits;
  state->torigin = torigin;
  state->mint = timestamptz_bucket(p->lower, tunits, torigin);
  state->maxt = timestamptz_bucket(p->upper, tunits, torigin);
  state->t = state->mint;
  return state;
}

/**
 * Increment the current state to the next bucket of the bucket list
 *
 * @param[in] state State to increment
 */
static void
period_bucket_state_next(PeriodBucketState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  state->t += state->tunits;
  if (state->t > state->maxt)
    state->done = true;
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Period_bucket_list);
/**
 * Generate a period bucket list.
 */
PGDLLEXPORT Datum
Period_bucket_list(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  PeriodBucketState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Period *bounds = PG_GETARG_SPAN_P(0);
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);

    /* Ensure parameter validity */
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = period_bucket_state_make(bounds, tunits, torigin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all buckets */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }
  /* Store bucket time */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate bucket */
  tuple_arr[1] = PointerGetDatum(period_bucket_get(state->t, state->tunits));
  /* Advance state */
  period_bucket_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Period_bucket);
/**
 * Generate a bucket in a bucket list for periods.
*/
PGDLLEXPORT Datum
Period_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bucket = timestamptz_bucket(t, tunits, torigin);
  Period *result = period_bucket_get(time_bucket, tunits);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time split functions for temporal numbers
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] buckets Initial timestamps of the buckets
 * @param[in] fragments Fragments of the input temporal value
 * @param[in] count Number of elements in the input arrays
 *
 * @pre count is greater than 0
 */
TimeSplitState *
time_split_state_make(int64 tunits, TimestampTz *buckets, Temporal **fragments,
  int count)
{
  assert(count > 0);
  TimeSplitState *state = palloc0(sizeof(TimeSplitState));
  /* Fill in state */
  state->done = false;
  state->tunits = tunits;
  state->buckets = buckets;
  state->fragments = fragments;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to output the next split
 *
 * @param[in] state State to increment
 */
void
time_split_state_next(TimeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to time buckets.
 *
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
  times[0] = timestamptz_bucket(inst->t, tunits, torigin);
  *buckets = times;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to time buckets.
 *
 * @param[in] is Temporal value
 * @param[in] start Start timestamp of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TInstantSet **
tinstantset_time_split(const TInstantSet *is, TimestampTz start,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  TInstantSet **result = palloc(sizeof(TInstantSet *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0;  /* counter for resulting fragments */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  while (i < is->count)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    if (lower <= inst->t && inst->t < upper)
    {
      instants[k++] = inst;
      i++;
    }
    else
    {
      if (k > 0)
      {
        times[l] = lower;
        result[l++] = tinstantset_make(instants, k, MERGE_NO);
        k = 0;
      }
      lower = upper;
      upper += tunits;
    }
  }
  if (k > 0)
  {
    times[l] = lower;
    result[l++] = tinstantset_make(instants, k, MERGE_NO);
  }
  pfree(instants);
  *buckets = times;
  *newcount = l;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to period buckets.
 *
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bucket lower bounds
 *
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tsequence_time_split1(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TSequence **result, TimestampTz *times)
{
  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* This loop is needed for filtering unnecesary time buckets for the sequences
   * composing a sequence set */
  /* The upper bound for the bucket is exclusive => the test below is >= */
  while (lower < end &&
    ((TimestampTz) seq->period.lower >= upper ||
     lower > (TimestampTz) seq->period.upper ||
     (lower == (TimestampTz) seq->period.upper && ! seq->period.upper_inc)))
  {
    lower = upper;
    upper += tunits;
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count * count);
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0,  /* counter for instants to free */
      m = 0;  /* counter for resulting fragments */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if ((lower <= inst->t && inst->t < upper) ||
      (inst->t == upper && (linear || i == seq->count - 1)))
    {
      instants[k++] = inst;
      i++;
    }
    else
    {
      assert(k > 0);
      /* Compute the value at the end of the bucket */
      if (instants[k - 1]->t < upper)
      {
        if (linear)
          tofree[l] = tsegment_at_timestamp(instants[k - 1], inst, linear,
            upper);
        else
        {
          /* The last two values of sequences with step interpolation and
           * exclusive upper bound must be equal */
          Datum value = tinstant_value(instants[k - 1]);
          tofree[l] = tinstant_make(value, seq->temptype, upper);
        }
        instants[k++] = tofree[l++];
      }
      lower_inc1 = (m == 0) ? seq->period.lower_inc : true;
      times[m] = lower;
      result[m++] = tsequence_make(instants, k, lower_inc1,
         (k > 1) ? false : true, linear, NORMALIZE);
      k = 0;
      lower = upper;
      upper += tunits;
      /* The second condition is needed for filtering unnecesary buckets for the
       * sequences composing a sequence set */
      if (lower >= end || ! contains_period_timestamp(&seq->period, lower))
        break;
      /* Reuse the end value of the previous bucket for the beginning of the bucket */
      if (lower < inst->t)
        instants[k++] = tsequence_inst_n(result[m - 1], result[m - 1]->count - 1);
     }
  }
  if (k > 0)
  {
    lower_inc1 = (m == 0) ? seq->period.lower_inc : true;
    times[m] = lower;
    result[m++] = tsequence_make(instants, k, lower_inc1,
      seq->period.upper_inc, linear, NORMALIZE);
  }
  pfree_array((void **) tofree, l);
  pfree(instants);
  return m;
}

/**
 * Split a temporal value into an array of fragments according to period buckets.
 *
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
 * Split a temporal value into an array of disjoint fragments
 *
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
    const TSequence *seq = tsequenceset_seq_n(ss, 0);
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
  int k = 0, /* Number of accumulated fragments of the current time bucket */
      m = 0; /* Number of time buckets already processed */
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz upper = lower + tunits;
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    /* Output the accumulated fragments of the current time bucket (if any)
     * if the current sequence starts on the next time bucket */
    if (k > 0 && (TimestampTz) seq->period.lower >= upper)
    {
      result[m++] = tsequenceset_make((const TSequence **) fragments, k,
        NORMALIZE);
      for (int j = 0; j < k; j++)
        pfree(fragments[j]);
      k = 0;
      lower += tunits;
      upper += tunits;
    }
    /* Number of time buckets of the current sequence */
    int l = tsequence_time_split1(seq, lower, end, tunits, count,
      sequences, &times[m]);
    /* If the current sequence has produced more than two time buckets */
    if (l > 1)
    {
      /* Assemble the accumulated fragments of the first time bucket (if any)  */
      if (k > 0)
      {
        fragments[k++] = sequences[0];
        result[m++] = tsequenceset_make((const TSequence **) fragments, k,
          NORMALIZE);
        for (int j = 0; j < k; j++)
          pfree(fragments[j]);
        k = 0;
      }
      else
      {
        result[m++] = tsequence_to_tsequenceset(sequences[0]);
        pfree(sequences[0]);
      }
      for (int j = 1; j < l - 1; j++)
      {
        result[m++] = tsequence_to_tsequenceset(sequences[j]);
        pfree(sequences[j]);
      }
    }
    /* Save the last fragment in case it is necessary to assemble with the
     * first one of the next sequence */
    fragments[k++] = sequences[l - 1];
    lower = times[m];
  }
  /* Process the accumulated fragments of the last time bucket */
  if (k > 0)
  {
    result[m++] = tsequenceset_make((const TSequence **) fragments, k,
      NORMALIZE);
    for (int j = 0; j < k; j++)
      pfree(fragments[j]);
  }
  pfree(sequences); pfree(fragments);
  *buckets = times;
  *newcount = m;
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_tiling
 * @brief Split a temporal value into fragments with respect to period buckets
 *
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
temporal_time_split(Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, TimestampTz torigin, int count, TimestampTz **buckets,
  int *newcount)
{
  assert(start < end);
  assert(count > 0);
  /* Split the temporal value */
  Temporal **fragments;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    fragments = (Temporal **) tinstant_time_split((const TInstant *) temp,
      tunits, torigin, buckets, newcount);
  else if (temp->subtype == TINSTANTSET)
    fragments = (Temporal **) tinstantset_time_split((const TInstantSet *) temp,
      start, tunits, count, buckets, newcount);
  else if (temp->subtype == TSEQUENCE)
    fragments = (Temporal **) tsequence_time_split((const TSequence *) temp,
      start, end, tunits, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
      start, end, tunits, count, buckets, newcount);
  return fragments;
}

PG_FUNCTION_INFO_V1(Temporal_time_split);
/**
 * Split a temporal value into fragments with respect to period buckets.
 */
PGDLLEXPORT Datum
Temporal_time_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  TimeSplitState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);

    /* Ensure parameter validity */
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the bounds */
    Period p;
    temporal_set_period(temp, &p);
    TimestampTz start_time = p.lower;
    TimestampTz end_time = p.upper;
    TimestampTz start_bucket = timestamptz_bucket(start_time, tunits,
      torigin);
    /* We need to add tunits to obtain the end timestamp of the last bucket */
    TimestampTz end_bucket = timestamptz_bucket(end_time, tunits,
      torigin) + tunits;
    int count = (int) (((int64) end_bucket - (int64) start_bucket) / tunits);
    /* Split the temporal value */
    TimestampTz *buckets;
    int newcount;
    Temporal **fragments = temporal_time_split(temp, start_bucket,
      end_bucket, tunits, torigin, count, &buckets, &newcount);
    assert(newcount > 0);

    /* Create function state */
    funcctx->user_fctx = time_split_state_make(tunits, buckets, fragments, newcount);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp, 0);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the fragments */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    for (int i = 0; i < state->count; i++)
      pfree(state->fragments[i]);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store timestamp and split */
  tuple_arr[0] = TimestampTzGetDatum(state->buckets[state->i]);
  tuple_arr[1] = PointerGetDatum(state->fragments[state->i]);
  /* Advance state */
  time_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************
 * TBOX tile functions
 *****************************************************************************/

/**
 * Generate a tile from the a multidimensional grid
 *
 * @param[in] value Start value of the tile to output
 * @param[in] t Start timestamp of the tile to output
 * @param[in] xsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 */
static TBOX *
tbox_tile_get(double value, TimestampTz t, double xsize, int64 tunits)
{
  double xmin = value;
  double xmax = value + xsize;
  TimestampTz tmin = t;
  TimestampTz tmax = t + tunits;
  return (TBOX *) tbox_make(true, true, xmin, xmax, tmin, tmax);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] xsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[in] xorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre Both xsize and tunits must be greater than 0.
 */
static TboxGridState *
tbox_tile_state_make(TBOX *box, double xsize, int64 tunits, double xorigin,
  TimestampTz torigin)
{
  assert(xsize > 0 || tunits > 0);
  TboxGridState *state = palloc0(sizeof(TboxGridState));

  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->xsize = xsize;
  state->tunits = tunits;
  if (xsize)
  {
    state->box.xmin = float_bucket(box->xmin, xsize, xorigin);
    state->box.xmax = float_bucket(box->xmax, xsize, xorigin);
  }
  if (tunits)
  {
    state->box.tmin = timestamptz_bucket(box->tmin, tunits, torigin);
    state->box.tmax = timestamptz_bucket(box->tmax, tunits, torigin);
  }
  state->value = state->box.xmin;
  state->t = state->box.tmin;
  return state;
}

/**
 * Increment the current state to the next tile of the multidimensional grid
 *
 * @param[in] state State to increment
 */
static void
tbox_tile_state_next(TboxGridState *state)
{
  if (!state || state->done)
      return;
  /* Move to the next tile */
  state->i++;
  state->value += state->xsize;
  if (state->value > state->box.xmax)
  {
    state->value = state->box.xmin;
    state->t += state->tunits;
    if (state->t > state->box.tmax)
    {
      state->done = true;
      return;
    }
  }
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_multidim_grid);
/**
 * Generate a multidimensional grid for temporal numbers.
 */
PGDLLEXPORT Datum
Tbox_multidim_grid(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  TboxGridState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    TBOX *bounds = PG_GETARG_TBOX_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    double xorigin = PG_GETARG_FLOAT8(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    /* Ensure parameter validity */
    ensure_has_X_tbox(bounds);
    ensure_has_T_tbox(bounds);
    ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = tbox_tile_state_make(bounds, xsize, tunits, xorigin, torigin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all tiles */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store tile value and time */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate box */
  tuple_arr[1] = PointerGetDatum(tbox_tile_get(state->value, state->t,
    state->xsize, state->tunits));
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_multidim_tile);
/**
 * Generate a tile in a multidimensional grid for temporal numbers.
*/
PGDLLEXPORT Datum
Tbox_multidim_tile(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double xsize = PG_GETARG_FLOAT8(2);
  ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
  Interval *duration = PG_GETARG_INTERVAL_P(3);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  double xorigin = PG_GETARG_FLOAT8(4);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(5);
  double value_bucket = float_bucket(value, xsize, xorigin);
  TimestampTz time_bucket = timestamptz_bucket(t, tunits, torigin);
  TBOX *result = tbox_tile_get(value_bucket, time_bucket, xsize, tunits);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Value split functions for temporal numbers
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] size Size of the value buckets
 * @param[in] buckets Initial values of the buckets
 * @param[in] fragments Fragments of the input temporal value
 * @param[in] count Number of elements in the input arrays
 *
 * @pre count is greater than 0
 */
ValueSplitState *
value_split_state_make(Datum size, Datum *buckets, Temporal **fragments,
  int count)
{
  assert(count > 0);
  ValueSplitState *state = palloc0(sizeof(ValueSplitState));
  /* Fill in state */
  state->done = false;
  state->size = size;
  state->buckets = buckets;
  state->fragments = fragments;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to output the next split
 *
 * @param[in] state State to increment
 */
void
value_split_state_next(ValueSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************/

/**
 * Get the bucket number in the bucket space that contains the value
 *
 * @param[in] value Input value
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 * @param[in] type Type of the arguments
 */
static int
bucket_position(Datum value, Datum size, Datum origin, mobdbType type)
{
  ensure_tnumber_basetype(type);
  if (type == T_INT4)
    return (DatumGetInt32(value) - DatumGetInt32(origin)) /
      DatumGetInt32(size);
  else /* type == T_FLOAT8 */
    return (int) floor( (DatumGetFloat8(value) - DatumGetFloat8(origin)) /
      DatumGetFloat8(size) );
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
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
  mobdbType basetype = temptype_basetype(inst->temptype);
  TInstant **result = palloc(sizeof(TInstant *));
  Datum *values = palloc(sizeof(Datum));
  result[0] = tinstant_copy(inst);
  values[0] = number_bucket(value, size, start_bucket, basetype);
  *buckets = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
 * @param[in] is Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] start_bucket Value of the start bucket
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TInstantSet **
tnumberinstset_value_split(const TInstantSet *is, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  mobdbType basetype = temptype_basetype(is->temptype);
  TInstantSet **result;
  Datum *values, value, bucket_value;

  /* Singleton instant set */
  if (is->count == 1)
  {
    result = palloc(sizeof(TInstantSet *));
    values = palloc(sizeof(Datum));
    result[0] = tinstantset_copy(is);
    value = tinstant_value(tinstantset_inst_n(is, 0));
    values[0] = number_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  const TInstant **instants = palloc(sizeof(TInstant *) * is->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *numinsts = palloc0(sizeof(int) * count);
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    value = tinstant_value(inst);
    bucket_value = number_bucket(value, size, start_bucket, basetype);
    int bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    int inst_no = numinsts[bucket_no]++;
    instants[bucket_no * is->count + inst_no] = inst;
  }
  /* Assemble the result for each value bucket */
  result = palloc(sizeof(TInstantSet *) * count);
  values = palloc(sizeof(Datum) * count);
  int k = 0;
  bucket_value = start_bucket;
  for (int i = 0; i < count; i++)
  {
    if (numinsts[i] > 0)
    {
      result[k] = tinstantset_make(&instants[i * is->count], numinsts[i], MERGE_NO);
      values[k++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(instants);
  pfree(numinsts);
  *buckets = values;
  *newcount = k;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
 * @param[inout] result Array containing the fragments of each bucket
 * @param[inout] numseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 *    seq->count for sequences or ss->totalcount for sequence sets
 * @param[in] seq Temporal value
 * @param[in] start_bucket Value of the start bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 */
static void
tnumberseq_step_value_split(TSequence **result, int *numseqs, int numcols,
  const TSequence *seq, Datum start_bucket, Datum size, int count)
{
  assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum value, bucket_value;
  int bucket_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value(tsequence_inst_n(seq, 0));
    bucket_value = number_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * count * seq->count);
  int l = 0;   /* counter for the instants to free */
  const TInstant *inst1;
  for (int i = 1; i < seq->count; i++)
  {
    inst1 = tsequence_inst_n(seq, i - 1);
    value = tinstant_value(inst1);
    bucket_value = number_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    bool lower_inc1 = (i == 1) ? seq->period.lower_inc : true;
    TInstant *bounds[2];
    bounds[0] = (TInstant *) inst1;
    int k = 1;
    if (i < seq->count)
    {
      tofree[l++] = bounds[1] = tinstant_make(value, seq->temptype, inst2->t);
      k++;
    }
    result[bucket_no * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
      k, lower_inc1, false, STEP, NORMALIZE);
    bounds[0] = bounds[1];
    inst1 = inst2;
    lower_inc1 = true;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = tsequence_inst_n(seq, seq->count - 1);
    value = tinstant_value(inst1);
    bucket_value = number_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tsequence_make(&inst1, 1,
      true, true, STEP, NORMALIZE);
  }
  pfree_array((void **) tofree, l);
  return;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
 * @param[inout] result Array containing the fragments of each bucket
 * @param[inout] numseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 *    seq->count for sequences or ss->totalcount for sequence sets
 * @param[in] seq Temporal value
 * @param[in] start_bucket Value of the start bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 */
static void
tnumberseq_linear_value_split(TSequence **result, int *numseqs, int numcols,
  const TSequence *seq, Datum start_bucket, Datum size, int count)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum value1, bucket_value1;
  int bucket_no1, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value(tsequence_inst_n(seq, 0));
    bucket_value1 = number_bucket(value1, size, start_bucket, basetype);
    bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no1]++;
    result[bucket_no1 * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * seq->count * count);
  int l = 0;   /* counter for the instants to free */
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
  value1 = tinstant_value(inst1);
  bucket_value1 = number_bucket(value1, size, start_bucket, basetype);
  bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    Datum bucket_value2 = number_bucket(value2, size, start_bucket, basetype);
    int bucket_no2 = bucket_position(bucket_value2, size, start_bucket, basetype);
    /* Take into account on whether the segment is increasing or decreasing */
    Datum min_value, max_value;
    int first_bucket, last_bucket, first, last;
    bool lower_inc1, upper_inc1, lower_inc_def, upper_inc_def;
    bool incr = datum_lt(value1, value2, basetype);
    if (incr)
    {
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
    if (datum_eq(min_value, max_value, basetype))
    {
      lower_inc1 = upper_inc1 = true;
    }
    Span *segspan = span_make(min_value, max_value, lower_inc1, upper_inc1,
      basetype);
    TInstant *bounds[2];
    bounds[first] = incr ? (TInstant *) inst1 : (TInstant *) inst2;
    Datum bucket_lower = incr ? bucket_value1 : bucket_value2;
    Datum bucket_upper = datum_add(bucket_lower, size, basetype, basetype);
    for (int j = first_bucket; j <= last_bucket; j++)
    {
      /* Choose between interpolate or take one of the segment ends */
      if (datum_lt(min_value, bucket_upper, basetype) &&
        datum_lt(bucket_upper, max_value, basetype))
      {
        TimestampTz t;
        /* To reduce the roundoff errors we may take the bound instead of
         * projecting the value to the timestamp */
        Datum projvalue;
        tlinearsegm_intersection_value(inst1, inst2, bucket_upper, basetype,
          &projvalue, &t);
        tofree[l++] = bounds[last] =  SPAN_ROUNDOFF ?
          tinstant_make(bucket_upper, seq->temptype, t) :
          tinstant_make(projvalue, seq->temptype, t);
      }
      else
        bounds[last] = incr ? (TInstant *) inst2 : (TInstant *) inst1;
      /* Determine the bounds of the resulting sequence */
      if (j == first_bucket || j == last_bucket)
      {
        Span *bucketspan = span_make(bucket_lower, bucket_upper, true, false,
          basetype);
        Span *inter = intersection_span_span(segspan, bucketspan);
        if (inter)
        {
          if (incr)
          {
            lower_inc1 = inter->lower_inc;
            upper_inc1 = inter->upper_inc;
          }
          else
          {
            lower_inc1 = inter->upper_inc;
            upper_inc1 = inter->lower_inc;
          }
          pfree(inter); pfree(bucketspan);
        }
        else
        {
          // elog(ERROR, "There is a problem");
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
      int k = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bucket if last instant has exclusive bound */
      if (k == 1 && ! upper_inc1)
        break;
      seq_no = numseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        k, (k > 1) ? lower_inc1 : true, (k > 1) ? upper_inc1 : true,
        LINEAR, NORMALIZE_NO);
      bounds[first] = bounds[last];
      bucket_lower = bucket_upper;
      bucket_upper = datum_add(bucket_upper, size, basetype, basetype);
    }
    pfree(segspan);
    inst1 = inst2;
    value1 = value2;
    bucket_value1 = bucket_value2;
    bucket_no1 = bucket_no2;
  }
  pfree_array((void **) tofree, l);
  return;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
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
  mobdbType basetype = temptype_basetype(seq->temptype);
  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    TSequenceSet **result = palloc(sizeof(TSequenceSet *));
    Datum *values = palloc(sizeof(Datum));
    result[0] = tsequence_to_tsequenceset(seq);
    Datum value = tinstant_value(tsequence_inst_n(seq, 0));
    values[0] = number_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *numseqs = palloc0(sizeof(int) * count);
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
    tnumberseq_linear_value_split(sequences, numseqs, seq->count, seq,
      start_bucket, size, count);
  else
    tnumberseq_step_value_split(sequences, numseqs, seq->count, seq,
      start_bucket, size, count);
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    if (numseqs[i] > 0)
    {
      result[k] = tsequenceset_make((const TSequence **)(&sequences[seq->count * i]),
        numseqs[i], NORMALIZE);
      values[k++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(sequences);
  pfree(numseqs);
  *buckets = values;
  *newcount = k;
  return result;}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
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
    return tnumberseq_value_split(tsequenceset_seq_n(ss, 0), start_bucket,
      size, count, buckets, newcount);

  /* General case */
  mobdbType basetype = temptype_basetype(ss->temptype);
  TSequence **bucketseqs = palloc(sizeof(TSequence *) * ss->totalcount * count);
  /* palloc0 to initialize the counters to 0 */
  int *numseqs = palloc0(sizeof(int) * count);
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    if (MOBDB_FLAGS_GET_LINEAR(ss->flags))
      tnumberseq_linear_value_split(bucketseqs, numseqs, ss->totalcount, seq,
        start_bucket, size, count);
    else
      tnumberseq_step_value_split(bucketseqs, numseqs, ss->totalcount, seq,
        start_bucket, size, count);
  }
  /* Assemble the result for each value bucket */
  TSequenceSet **result = palloc(sizeof(TSequenceSet *) * count);
  Datum *values = palloc(sizeof(Datum) * count);
  Datum bucket_value = start_bucket;
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    if (numseqs[i] > 0)
    {
      result[k] = tsequenceset_make((const TSequence **)(&bucketseqs[i * ss->totalcount]),
        numseqs[i], NORMALIZE);
      values[k++] = bucket_value;
    }
    bucket_value = datum_add(bucket_value, size, basetype, basetype);
  }
  pfree(bucketseqs);
  pfree(numseqs);
  *buckets = values;
  *newcount = k;
  return result;
}

/*****************************************************************************/

Temporal **
tnumber_value_split(Temporal *temp, Datum start_bucket, Datum size,
  int count, Datum **buckets, int *newcount)
{
  assert(count > 0);
  /* Split the temporal value */
  Temporal **fragments;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    fragments = (Temporal **) tnumberinst_value_split((const TInstant *) temp,
      start_bucket, size, buckets, newcount);
  else if (temp->subtype == TINSTANTSET)
    fragments = (Temporal **) tnumberinstset_value_split((const TInstantSet *) temp,
      start_bucket, size, count, buckets, newcount);
  else if (temp->subtype == TSEQUENCE)
    fragments = (Temporal **) tnumberseq_value_split((const TSequence *) temp,
      start_bucket, size, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tnumberseqset_value_split((const TSequenceSet *) temp,
      start_bucket, size, count, buckets, newcount);
  return fragments;
}

PG_FUNCTION_INFO_V1(Tnumber_value_split);
/**
 * Split a temporal value into fragments with respect to period buckets.
 */
PGDLLEXPORT Datum
Tnumber_value_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  ValueSplitState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    Datum size = PG_GETARG_DATUM(1);
    Datum origin = PG_GETARG_DATUM(2);

    /* Ensure parameter validity */
    mobdbType basetype = temptype_basetype(temp->temptype);
    ensure_positive_datum(size, basetype);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the value bounds */
    Span *span = tnumber_to_span((const Temporal *) temp);
    Datum start_value = span->lower;
    /* We need to add size to obtain the end value of the last bucket */
    Datum end_value = datum_add(span->upper, size, basetype, basetype);
    Datum start_bucket = number_bucket(start_value, size, origin, basetype);
    Datum end_bucket = number_bucket(end_value, size, origin, basetype);
    int count = (basetype == T_INT4) ?
      (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) /
        DatumGetInt32(size) :
      floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) /
        DatumGetFloat8(size));

    /* Split the temporal value */
    Datum *buckets;
    int newcount;
    Temporal **fragments = tnumber_value_split(temp, start_bucket, size,
      count, &buckets, &newcount);
    assert(newcount > 0);

    /* Create function state */
    funcctx->user_fctx = value_split_state_make(size, buckets, fragments, newcount);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp, 0);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the fragments */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    for (int i = 0; i < state->count; i++)
      pfree(state->fragments[i]);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store timestamp and split */
  tuple_arr[0] = state->buckets[state->i];
  tuple_arr[1] = PointerGetDatum(state->fragments[state->i]);
  /* Advance state */
  value_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************
 * Value and time split functions for temporal numbers
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] value_buckets Initial values of the tiles
 * @param[in] time_buckets Initial timestamps of the tiles
 * @param[in] fragments Fragments of the input temporal value
 * @param[in] count Number of elements in the input arrays
 *
 * @pre count is greater than 0
 */
ValueTimeSplitState *
value_time_split_state_make(Datum *value_buckets, TimestampTz *time_buckets,
  Temporal **fragments, int count)
{
  assert(count > 0);
  ValueTimeSplitState *state = palloc0(sizeof(ValueTimeSplitState));
  /* Fill in state */
  state->done = false;
  state->value_buckets = value_buckets;
  state->time_buckets = time_buckets;
  state->fragments = fragments;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to the next tile of the multidimensional grid
 *
 * @param[in] state State to increment
 */
void
value_time_split_state_next(ValueTimeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_value_time_split);
/**
 * Split a temporal value into fragments with respect to period tiles.
 */
PGDLLEXPORT Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  ValueTimeSplitState *state;
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */
  Datum tuple_arr[3]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    Datum size = PG_GETARG_DATUM(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    Datum origin = PG_GETARG_DATUM(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    /* Ensure parameter validity */
    mobdbType basetype = temptype_basetype(temp->temptype);
    ensure_positive_datum(size, basetype);
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the value bounds */
    Span *span = tnumber_to_span((const Temporal *) temp);
    Datum start_value = span->lower;
    /* We need to add size to obtain the end value of the last bucket */
    Datum end_value = datum_add(span->upper, size, basetype, basetype);
    Datum start_bucket = number_bucket(start_value, size, origin, basetype);
    Datum end_bucket = number_bucket(end_value, size, origin, basetype);
    int value_count = (basetype == T_INT4) ?
      (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) / DatumGetInt32(size) :
      floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) / DatumGetFloat8(size));

    /* Compute the time bounds */
    Period p;
    temporal_set_period(temp, &p);
    TimestampTz start_time = p.lower;
    TimestampTz end_time = p.upper;
    TimestampTz start_time_bucket = timestamptz_bucket(start_time,
      tunits, torigin);
    /* We need to add tunits to obtain the end timestamp of the last bucket */
    TimestampTz end_time_bucket = timestamptz_bucket(end_time, tunits,
      torigin) + tunits;
    int time_count = (int) (((int64) end_time_bucket - (int64) start_time_bucket) / tunits);

    /* Compute the number of tiles */
    int count = value_count * time_count;

    /* Split the temporal value */
    Datum *value_buckets = palloc(sizeof(Datum) * count);
    TimestampTz *time_buckets = palloc(sizeof(TimestampTz) * count);
    Temporal **fragments = palloc(sizeof(Temporal *) * count);
    int k = 0;
    Datum lower_value = start_bucket;
    while (datum_lt(lower_value, end_bucket, basetype))
    {
      Datum upper_value = datum_add(lower_value, size, basetype, basetype);
      span = span_make(lower_value, upper_value, true, false, basetype);
      Temporal *atspan = tnumber_restrict_span(temp, span, REST_AT);
      if (atspan != NULL)
      {
        int num_time_splits;
        TimestampTz *times;
        Temporal **time_splits = temporal_time_split(atspan, start_time_bucket,
          end_time_bucket, tunits, torigin, time_count, &times,
          &num_time_splits);
        for (int i = 0; i < num_time_splits; i++)
        {
          value_buckets[i + k] = lower_value;
          time_buckets[i + k] = times[i];
          fragments[i + k] = time_splits[i];
        }
        k += num_time_splits;
        pfree(time_splits);
        pfree(times);
      }
      pfree(span);
      lower_value = upper_value;
    }

    assert(k > 0);
    /* Create function state */
    funcctx->user_fctx = value_time_split_state_make(value_buckets,
      time_buckets, fragments, k);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp, 0);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the fragments */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    for (int i = 0; i < state->count; i++)
      pfree(state->fragments[i]);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store value, timestamp, and split */
  tuple_arr[0] = state->value_buckets[state->i];
  tuple_arr[1] = TimestampTzGetDatum(state->time_buckets[state->i]);
  tuple_arr[2] = PointerGetDatum(state->fragments[state->i]);
  /* Advance state */
  value_time_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/