/***********************************************************************
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
 * @file bucket.c
 * Time bucket functions for temporal types.
 * These functions are inspired from the time bucket function from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include <postgres.h>
#include <assert.h>
#include <float.h>
#include <funcapi.h>
#include <utils/builtins.h>
#include <utils/datetime.h>

#include "bucket.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "rangetypes_ext.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "tnumber_mathfuncs.h"

/**
 * Return the initial timestamp of the bucket in which a timestamp falls.
 *
 * @param[in] timestamp Input timestamp
 * @param[in] tunits Size of the buckets
 * @param[in] origin Origin of the buckets
 */
static TimestampTz
timestamptz_bucket_internal(TimestampTz timestamp, int64 tunits,
  TimestampTz origin)
{
  TimestampTz result;
  /* origin = origin % tunits, but use TMODULO */
  TMODULO(origin, result, tunits);

  if ((origin > 0 && timestamp < DT_NOBEGIN + origin) ||
    (origin < 0 && timestamp > DT_NOEND + origin))
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
      errmsg("timestamp out of range")));
  timestamp -= origin;

  /* result = (timestamp / tunits) * tunits */
  TMODULO(timestamp, result, tunits);
  if (timestamp < 0)
  {
    /*
     * need to subtract another tunits if remainder < 0 this only happens
     * if timestamp is negative to begin with and there is a remainder
     * after division. Need to subtract another tunits since division
     * truncates toward 0 in C99.
     */
    result = (result * tunits) - tunits;
  }
  else
    result *= tunits;
  result += origin;
  return result;
}

PG_FUNCTION_INFO_V1(timestamptz_bucket);
/**
 * Return the initial timestamp of the bucket in which a timestamp falls.
 */
PGDLLEXPORT Datum
timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz timestamp = PG_GETARG_TIMESTAMPTZ(0);
  if (TIMESTAMP_NOT_FINITE(timestamp))
    PG_RETURN_TIMESTAMPTZ(timestamp);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);

  TimestampTz result = timestamptz_bucket_internal(timestamp, tunits, origin);
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************/

/**
 * Return the initial value of the bucket in which an integer value falls.
 *
 * @param[in] value Input value
 * @param[in] width Width of the buckets
 * @param[in] origin Origin of the buckets
 */
static double
int_bucket_internal(int value, int width, int origin)
{
  int result;
  origin = origin % width;

  if ((origin > 0 && value < PG_INT32_MIN + origin) ||
    (origin < 0 && value > PG_INT32_MAX + origin))
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
      errmsg("integer value out of range")));
  value -= origin;

  result = value / width;
  if (result < 0)
  {
    /*
     * need to subtract another width if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another width since division
     * truncates toward 0 in C99.
     */
    result = (result * width) - width;
  }
  else
    result *= width;
  result += origin;
  return result;
}

PG_FUNCTION_INFO_V1(int_bucket);
/**
 * Return the initial value of the bucket in which an integer value falls.
 */
PGDLLEXPORT Datum
int_bucket(PG_FUNCTION_ARGS)
{
  int value = PG_GETARG_INT32(0);
  if (value == PG_INT32_MIN || value == PG_INT32_MAX)
    PG_RETURN_INT32(value);
  int width = PG_GETARG_INT32(1);
  ensure_positive_int(width);
  int origin = PG_GETARG_INT32(2);

  int result = int_bucket_internal(value, width, origin);
  PG_RETURN_INT32(result);
}

/*****************************************************************************/

/**
 * Return the initial value of the bucket in which a float value falls.
 *
 * @param[in] value Input value
 * @param[in] width Width of the buckets
 * @param[in] origin Origin of the buckets
 */
double
float_bucket_internal(double value, double width, double origin)
{
  double result;
  origin = fmod(origin, width);

  if ((origin > 0 && value < DBL_MIN + origin) ||
    (origin < 0 && value > DBL_MAX + origin))
    ereport(ERROR,
        (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
         errmsg("value out of range")));
  value -= origin;

  /* result = (value / width) * width */
  FMODULO(value, result, width);
  if (result < 0)
  {
    /*
     * need to subtract another width if remainder < 0 this only happens
     * if value is negative to begin with and there is a remainder
     * after division. Need to subtract another width since division
     * truncates toward 0 in C99.
     */
    result = (result * width) - width;
  }
  else
    result *= width;
  result += origin;
  return result;
}

PG_FUNCTION_INFO_V1(float_bucket);
/**
 * Return the initial value of the bucket in which a float value falls.
 */
PGDLLEXPORT Datum
float_bucket(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  if (value == DBL_MIN || value == DBL_MAX)
    PG_RETURN_INT32(value);
  double width = PG_GETARG_FLOAT8(1);
  ensure_positive_double(width);
  double origin = PG_GETARG_FLOAT8(2);

  double result = float_bucket_internal(value, width, origin);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of splits according to time buckets.
 *
 * @param[in] inst Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Bucket width
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a split
 * @param[out] newcount Number of values in the output array
 */
static TInstant **
tinstant_time_split(const TInstant *inst, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  assert(start < end);
  TInstant **result = palloc(sizeof(TInstant *));
  TimestampTz *times = palloc(sizeof(TimestampTz));
  result[0] = (TInstant *) inst;
  TimestampTz lower = start;
  while (lower < end)
  {
    TimestampTz upper = lower + tunits;
    if (lower <= inst->t && inst->t < upper)
    {
      times[0] = lower;
      break;
    }
    lower = upper;
  }
  *buckets = times;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of splits according to time buckets.
 *
 * @param[in] ti Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Bucket width
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a split
 * @param[out] newcount Number of values in the output array
 */
static TInstantSet **
tinstantset_time_split(const TInstantSet *ti, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  assert(start < end);
  TInstantSet **result = palloc(sizeof(TInstantSet *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0;  /* counter for resulting splits */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  while (i < ti->count)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
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
      if (upper >= end)
       break;
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
 * Split a temporal value into an array of splits according to period buckets.
 *
 * @param[out] result Output array of fragments of the temporal value
 * @param[out] times Output array of bucket lower bounds
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Bucket width
 * @param[in] count Number of buckets
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tsequence_time_split1(TSequence **result, TimestampTz *times, const TSequence *seq,
  TimestampTz start, TimestampTz end, int64 tunits, int count)
{
  assert(start < end);
  assert(count > 0);

  /* This loop is needed for filtering unnecesary buckets for the sequences
   * composing a sequence set */
  TimestampTz lower = start;
  TimestampTz upper = lower + tunits;
  /* The upper bound for the bucket is exclusive => the test below is >= */
  while (lower < end && (seq->period.lower >= upper || lower > seq->period.upper ||
    (lower == seq->period.upper && ! seq->period.upper_inc)))
  {
    lower = upper;
    upper += tunits;
  }

  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant **tofree = palloc(sizeof(TInstant *) * count * 2);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0,  /* counter for instants to free */
      m = 0;  /* counter for resulting splits */
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
      /* Compute the value at the end of the bucket */
      if (instants[k - 1]->t < upper)
      {
        /* The last two values of sequences with step interpolation and
         * exclusive upper bound must be equal */
        if (linear)
          tofree[l] = tsequence_at_timestamp1(instants[k - 1], inst, linear,
            upper);
        else
        {
          Datum value = tinstant_value(instants[k - 1]);
          tofree[l] = tinstant_make(value, upper, seq->valuetypid);
        }
        instants[k++] = tofree[l++];
      }
      lower_inc1 = (m == 0) ? seq->period.lower_inc : true;
      times[m] = lower;
      result[m++] = tsequence_make(instants, k, lower_inc1,
         (k > 1) ? false : true, linear, NORMALIZE);
      k = 0;
      lower = upper;
      upper  += tunits;
      /* The second condition is needed for filtering unnecesary buckets for the
       * sequences composing a sequence set */
      if (lower >= end || ! contains_period_timestamp_internal(&seq->period, lower))
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
  pfree(instants);
  pfree_array((void **) tofree, l);
  return m;
}

/**
 * Split a temporal value into an array of splits according to period buckets.
 *
 * @param[in] seq Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Bucket width
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a split
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tsequence_time_split(const TSequence *seq, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  *newcount = tsequence_time_split1(result, times, seq, start, end,
    tunits, count);
  *buckets = times;
  return result;
}

/**
 * Split a temporal value into an array of non self-intersecting splits
 *
 * @param[in] ts Temporal value
 * @param[in] start,end Start and end timestamps of the buckets
 * @param[in] tunits Bucket width
 * @param[out] buckets Start timestamp of the buckets containing a split
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tsequenceset_time_split(const TSequenceSet *ts, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_time_split(tsequenceset_seq_n(ts, 0), start, end, tunits,
      count, buckets, newcount);

  /* General case */
  TSequence **result = palloc0(sizeof(TSequence *) * ts->count * count);
  TimestampTz *times = palloc0(sizeof(int) * ts->count * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_time_split1(&result[k], &times[k], seq, start, end,
      tunits, count);
  }
  *buckets = times;
  *newcount = k;
  return result;
}

/*****************************************************************************
 * Bucket functions
 *****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal splits
 */
typedef struct TimeSplitState
{
  bool done;
  int64 tunits;
  TimestampTz *buckets;
  Temporal **splits;
  int i;
  int count;
} TimeSplitState;

/**
 * Create the initial state that persists across multiple calls to output
 * the temporal splits
 */
static TimeSplitState *
time_split_state_new(int64 tunits, TimestampTz *buckets, Temporal **splits,
  int count)
{
  TimeSplitState *state = palloc0(sizeof(TimeSplitState));

  /* fill in state */
  state->done = false;
  state->tunits = tunits;
  state->buckets = buckets;
  state->splits = splits;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to output the next split
 */
static void
time_split_state_next(TimeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

static Temporal **
temporal_time_split_internal(Temporal *temp, TimestampTz start, TimestampTz end,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  /* Split the temporal value */
  Temporal **splits;
  if (temp->temptype == INSTANT)
    splits = (Temporal **) tinstant_time_split((const TInstant *) temp,
      start, end, tunits, count, buckets, newcount);
  else if (temp->temptype == INSTANTSET)
    splits = (Temporal **) tinstantset_time_split((const TInstantSet *) temp,
      start, end, tunits, count, buckets, newcount);
  else if (temp->temptype == SEQUENCE)
    splits = (Temporal **) tsequence_time_split((const TSequence *) temp,
      start, end, tunits, count, buckets, newcount);
  else /* temp->temptype == SEQUENCESET */
    splits = (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
      start, end, tunits, count, buckets, newcount);
  return splits;
}

PG_FUNCTION_INFO_V1(temporal_time_split);
/**
 * Split a temporal value into splits with respect to period buckets.
 */
Datum temporal_time_split(PG_FUNCTION_ARGS)
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
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the bounds */
    Period p;
    temporal_period(&p, temp);
    TimestampTz start_time = p.lower;
    TimestampTz end_time = p.upper;
    TimestampTz start_bucket = timestamptz_bucket_internal(start_time, tunits,
      torigin);
    /* We need to add tunits to obtain the end timestamp of the last bucket */
    TimestampTz end_bucket = timestamptz_bucket_internal(end_time, tunits,
      torigin) + tunits;
    int count = (int) (((int64) end_bucket - (int64) start_bucket) / tunits);
    /* Split the temporal value */
    TimestampTz *buckets;
    int newcount;
    Temporal **splits = temporal_time_split_internal(temp, start_bucket, end_bucket,
      tunits, count, &buckets, &newcount);
    assert(newcount > 0);

    /* Create function state */
    funcctx->user_fctx = time_split_state_new(tunits, buckets, splits, newcount);
    /* Build a tuple description for a multidimensial grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the splits */
  if (state->done)
    SRF_RETURN_DONE(funcctx);

  /* Store timestamp and split */
  tuple_arr[0] = TimestampTzGetDatum(state->buckets[state->i]);
  tuple_arr[1] = PointerGetDatum(state->splits[state->i]);
  time_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal splits
 */
typedef struct RangeSplitState
{
  bool done;
  Datum *buckets;
  Temporal **splits;
  int i;
  int count;
} RangeSplitState;

/**
 * Create the initial state that persists across multiple calls to output
 * the temporal splits
 */
static RangeSplitState *
range_split_state_new(Datum *buckets, Temporal **splits, int count)
{
  RangeSplitState *state = palloc0(sizeof(RangeSplitState));

  /* fill in state */
  state->done = false;
  state->buckets = buckets;
  state->splits = splits;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to output the next split
 */
static void
range_split_state_next(RangeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

PG_FUNCTION_INFO_V1(tnumber_range_split);
/**
 * Split a temporal value into splits with respect to period buckets.
 */
Datum tnumber_range_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  RangeSplitState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    Datum width = PG_GETARG_DATUM(1);
    Datum origin = PG_GETARG_DATUM(2);

    Oid valuetypid = temp->valuetypid;
    bool tint = (valuetypid == INT4OID);
    int iwidth, iorigin;
    double dwidth, dorigin;
    if (tint)
    {
      iwidth = DatumGetInt32(width);
      iorigin = DatumGetInt32(origin);
      ensure_positive_int(iwidth);
    }
    else
    {
      dwidth = DatumGetFloat8(width);
      dorigin = DatumGetFloat8(origin);
      ensure_positive_double(dwidth);
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the bounds */
    RangeType *range = tnumber_value_range_internal((const Temporal *) temp);
    Datum start_value = lower_datum(range);
    Datum end_value = upper_datum(range);
    Datum start_bucket = tint ?
      Int32GetDatum(int_bucket_internal(DatumGetInt32(start_value), iwidth, iorigin)) :
      Float8GetDatum(float_bucket_internal(DatumGetFloat8(start_value), dwidth, dorigin));
    /* We need to add width to obtain the end value of the last bucket */
    Datum end_bucket = tint ?
      Int32GetDatum(int_bucket_internal(DatumGetInt32(end_value), iwidth, iorigin) + iwidth) :
      Float8GetDatum(float_bucket_internal(DatumGetFloat8(end_value), dwidth, dorigin) + dwidth);
    int count = tint ?
      (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) / iwidth :
      floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) / dwidth);

    /* Split the temporal value */
    Datum *buckets = palloc(sizeof(int) * count);
    Temporal **splits = palloc(sizeof(Temporal *) * count);
    int k = 0;
    Datum lower = start_bucket;
    while (datum_lt(lower, end_bucket, valuetypid))
    {
      Datum upper = datum_add(lower, width, valuetypid, valuetypid);
      range = range_make(lower, upper, true, false, valuetypid);
      Temporal *atrange = tnumber_restrict_range_internal(temp, range, REST_AT);
      if (atrange != NULL)
      {
        buckets[k] = lower;
        splits[k++] = atrange;
      }
      pfree(range);
      lower = upper;
    }

    assert(k > 0);
    /* Create function state */
    funcctx->user_fctx = range_split_state_new(buckets, splits, k);
    /* Build a tuple description for a multidimensial grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the splits */
  if (state->done)
    SRF_RETURN_DONE(funcctx);

  /* Store value and split */
  tuple_arr[0] = state->buckets[state->i];
  tuple_arr[1] = PointerGetDatum(state->splits[state->i]);
  range_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

/**
 * Struct for storing the state that persists across multiple calls to output
 * the temporal splits
 */
typedef struct RangeTimeSplitState
{
  bool done;
  Datum *value_buckets;
  TimestampTz *time_buckets;
  Temporal **splits;
  int i;
  int count;
} RangeTimeSplitState;

/**
 * Create the initial state that persists across multiple calls to output
 * the temporal splits
 */
static RangeTimeSplitState *
range_time_split_state_new(Datum *value_buckets, TimestampTz *time_buckets,
  Temporal **splits, int count)
{
  RangeTimeSplitState *state = palloc0(sizeof(RangeTimeSplitState));

  /* fill in state */
  state->done = false;
  state->value_buckets = value_buckets;
  state->time_buckets = time_buckets;
  state->splits = splits;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * Increment the current state to output the next split
 */
static void
range_time_split_state_next(RangeTimeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

PG_FUNCTION_INFO_V1(tnumber_range_time_split);
/**
 * Split a temporal value into splits with respect to period tiles.
 */
Datum tnumber_range_time_split(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  RangeTimeSplitState *state;
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */
  Datum tuple_arr[3]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL(0);
    Datum width = PG_GETARG_DATUM(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    Datum origin = PG_GETARG_DATUM(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    Oid valuetypid = temp->valuetypid;
    bool tint = (valuetypid == INT4OID);
    int iwidth, iorigin;
    double dwidth, dorigin;
    if (tint)
    {
      iwidth = DatumGetInt32(width);
      iorigin = DatumGetInt32(origin);
      ensure_positive_int(iwidth);
    }
    else
    {
      dwidth = DatumGetFloat8(width);
      dorigin = DatumGetFloat8(origin);
      ensure_positive_double(dwidth);
    }
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the value bounds */
    RangeType *range = tnumber_value_range_internal((const Temporal *) temp);
    Datum start_value = lower_datum(range);
    Datum end_value = upper_datum(range);
    Datum start_bucket = tint ?
      Int32GetDatum(int_bucket_internal(DatumGetInt32(start_value), iwidth, iorigin)) :
      Float8GetDatum(float_bucket_internal(DatumGetFloat8(start_value), dwidth, dorigin));
    /* We need to add width to obtain the end value of the last bucket */
    Datum end_bucket = tint ?
      Int32GetDatum(int_bucket_internal(DatumGetInt32(end_value), iwidth, iorigin) + iwidth) :
      Float8GetDatum(float_bucket_internal(DatumGetFloat8(end_value), dwidth, dorigin) + dwidth);
    int value_count = tint ?
      (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) / iwidth :
      floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) / dwidth);

    /* Compute the time bounds */
    Period p;
    temporal_period(&p, temp);
    TimestampTz start_time = p.lower;
    TimestampTz end_time = p.upper;
    TimestampTz start_time_bucket = timestamptz_bucket_internal(start_time, tunits,
      torigin);
    /* We need to add tunits to obtain the end timestamp of the last bucket */
    TimestampTz end_time_bucket = timestamptz_bucket_internal(end_time, tunits,
      torigin) + tunits;
    int time_count = (int) (((int64) end_time_bucket - (int64) start_time_bucket) / tunits);

    /* Compute the number of tiles */
    int count = value_count * time_count;

    /* Split the temporal value */
    Datum *value_buckets = palloc(sizeof(Datum) * count);
    TimestampTz *time_buckets = palloc(sizeof(TimestampTz) * count);
    Temporal **splits = palloc(sizeof(Temporal *) * count);

    int k = 0;
    Datum lower_value = start_bucket;
    while (datum_lt(lower_value, end_bucket, valuetypid))
    {
      Datum upper_value = datum_add(lower_value, width, valuetypid, valuetypid);
      range = range_make(lower_value, upper_value, true, false, valuetypid);
      Temporal *atrange = tnumber_restrict_range_internal(temp, range, REST_AT);
      if (atrange != NULL)
      {
        int num_time_splits;
        TimestampTz *times;
        Temporal **time_splits = temporal_time_split_internal(atrange, start_time_bucket,
          end_time_bucket, tunits, time_count, &times, &num_time_splits);
        for (int i = 0; i < num_time_splits; i++)
        {
          value_buckets[i + k] = lower_value;
          time_buckets[i + k] = times[i];
          splits[i + k] = time_splits[i];
        }
        k += num_time_splits;
        pfree(time_splits);
        pfree(times);
      }
      pfree(range);
      lower_value = upper_value;
    }

    assert(k > 0);
    /* Create function state */
    funcctx->user_fctx = range_time_split_state_new(value_buckets, time_buckets, splits, k);
    /* Build a tuple description for a multidimensial grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the splits */
  if (state->done)
    SRF_RETURN_DONE(funcctx);

  /* Store integer, timestamp, and split */
  tuple_arr[0] = Int32GetDatum(state->value_buckets[state->i]);
  tuple_arr[1] = TimestampTzGetDatum(state->time_buckets[state->i]);
  tuple_arr[2] = PointerGetDatum(state->splits[state->i]);
  range_time_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/
