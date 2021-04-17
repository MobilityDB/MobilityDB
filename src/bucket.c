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
#include <utils/builtins.h>
#include <utils/datetime.h>

#include "bucket.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "rangetypes_ext.h"
#include "temporaltypes.h"
#include "temporal_util.h"

/**
 * Return the starting timestamp of the bucket in which a timestamp falls.
 *
 * @param[in] timestamp Input timestamp
 * @param[in] period Size of the buckets
 * @param[in] origin Origin of the buckets
 */
static TimestampTz
timestamptz_bucket_internal(TimestampTz timestamp, int64 period,
  TimestampTz origin)
{
  TimestampTz result;
  /* origin = origin % period, but use TMODULO */
  TMODULO(origin, result, period);

  if ((origin > 0 && timestamp < DT_NOBEGIN + origin) ||
    (origin < 0 && timestamp > DT_NOEND + origin))
    ereport(ERROR,
        (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
         errmsg("timestamp out of range")));
  timestamp -= origin;

  /* result = (timestamp / period) * period */
  TMODULO(timestamp, result, period);
  if (timestamp < 0)
  {
    /*
     * need to subtract another period if remainder < 0 this only happens
     * if timestamp is negative to begin with and there is a remainder
     * after division. Need to subtract another period since division
     * truncates toward 0 in C99.
     */
    result = (result * period) - period;
  }
  else
    result *= period;
  result += origin;
  return result;
}

PG_FUNCTION_INFO_V1(timestamptz_bucket);

PGDLLEXPORT Datum
timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz timestamp = PG_GETARG_TIMESTAMPTZ(0);
  if (TIMESTAMP_NOT_FINITE(timestamp))
    PG_RETURN_TIMESTAMPTZ(timestamp);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 period = get_interval_units(duration);
  TimestampTz origin = (PG_NARGS() > 2 ?
    PG_GETARG_TIMESTAMPTZ(2) : DEFAULT_TIME_ORIGIN);

  TimestampTz result = timestamptz_bucket_internal(timestamp, period, origin);
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of pieces according to period buckets.
 *
 * @param[in] ti Temporal value
 * @param[in] periods Array of period buckets
 * @param[in] count Number of periods in the array
 */
static ArrayType *
tinstantset_time_bucket(const TInstantSet *ti, Period *periods, int count)
{
  /* Singleton instant set */
  if (ti->count == 1)
    return temporalarr_to_array((const Temporal **) &ti, 1);

  TInstantSet **instsets = palloc(sizeof(TInstantSet *) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
  int i = 0,  /* counter for instants of temporal value */
      j = 0,  /* counter for period buckets */
      k = 0,  /* counter for instants of next split */
      l = 0;  /* counter for resulting splits */
  while (i < ti->count)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    if (contains_period_timestamp_internal(&periods[j], inst->t))
    {
      instants[k++] = inst;
      i++;
    }
    else
    {
      if (k > 0)
      {
        instsets[l++] = tinstantset_make(instants, k, MERGE_NO);
        k = 0;
      }
      j++;
      if (j == count)
       break;
    }
  }
  if (k > 0)
    instsets[l++] = tinstantset_make(instants, k, MERGE_NO);
  ArrayType *result = temporalarr_to_array((const Temporal **) instsets, l);
  pfree_array((void **) instsets, l);
  return result;
}

/**
 * Split a temporal value into an array of pieces according to period buckets.
 *
 * @param[in] seq Temporal value
 * @param[in] periods Array of period buckets
 * @param[in] count Number of periods in the array
 * @param[out] newcount Number of sequences in the resulting array
 * @note This function is called for each sequence of a temporal sequence set
 */
static int
tsequence_time_bucket1(TSequence **result, const TSequence *seq, Period *periods,
  int count)
{
  assert(count > 0);

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result[0] = tsequence_copy(seq);
    return 1;
  }

  /* General case */
  /* This loop is needed for filtering unnecesary buckets for the sequences
   * composing a sequence set */
  int j = 0;  /* counter for period buckets */
  while (! overlaps_period_period_internal(&seq->period, &periods[j]))
    j++;
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  TInstant **tofree = palloc(sizeof(TInstant *) * (count - j) * 2);
  bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0,  /* counter for instants to free */
      m = 0;  /* counter for resulting splits */
  bool lower_inc1;
  while (i < seq->count)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    if (contains_period_timestamp_internal(&periods[j], inst->t) ||
      (inst->t == periods[j].upper && (linear || i == seq->count - 1)))
    {
      instants[k++] = inst;
      i++;
    }
    else
    {
      /* Compute the value at the end of the bucket */
      if (instants[k - 1]->t < periods[j].upper)
      {
        /* The last two values of sequences with step interpolation and
         * exclusive upper bound must be equal */
        if (linear)
          tofree[l] = tsequence_at_timestamp1(instants[k - 1], inst, linear,
            periods[j].upper);
        else
        {
          Datum value = tinstant_value(instants[k - 1]);
          tofree[l] = tinstant_make(value, periods[j].upper, seq->valuetypid);
        }
        instants[k++] = (TInstant *) tofree[l++];
      }
      lower_inc1 = (m == 0) ? seq->period.lower_inc : true;
      result[m++] = tsequence_make(instants, k, lower_inc1,
         (k > 1) ? false : true, linear, NORMALIZE);
      k = 0;
      j++;
      /* The second condition is needed for filtering unnecesary buckets for the
       * sequences composing a sequence set */
      if (j == count || ! overlaps_period_period_internal(&seq->period, &periods[j]))
        break;
      /* Reuse the end value of the previous bucket for the beginning of the bucket */
      if (periods[j].lower < inst->t)
        instants[k++] = tsequence_inst_n(result[m - 1], result[m - 1]->count - 1);
     }
  }
  if (k > 0)
  {
    lower_inc1 = (m == 0) ? seq->period.lower_inc : true;
    result[m++] = tsequence_make((const TInstant **) instants, k,
      lower_inc1, seq->period.upper_inc, linear, NORMALIZE);
  }
  pfree_array((void **) tofree, l);
  return m;
}

/**
 * Split a temporal value into an array of pieces according to period buckets.
 *
 * @param[in] seq Temporal value
 * @param[in] periods Array of period buckets
 * @param[in] count Number of periods in the array
 */
static ArrayType *
tsequence_time_bucket(const TSequence *seq, Period *periods, int count)
{
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  int newcount = tsequence_time_bucket1(sequences, seq, periods, count);
  ArrayType *result = temporalarr_to_array((const Temporal **) sequences, newcount);
  pfree_array((void **) sequences, newcount);
  return result;
}

/**
 * Split a temporal value into an array of non self-intersecting pieces
 *
 * @param[in] ts Temporal value
 * @param[in] periods Array of period buckets
 * @param[in] count Number of periods in the array
 */
static ArrayType *
tsequenceset_time_bucket(const TSequenceSet *ts, Period *periods, int count)
{
  /* Singleton sequence set */
  if (ts->count == 1)
    return tsequence_time_bucket(tsequenceset_seq_n(ts, 0), periods, count);

  /* General case */
  TSequence **sequences = palloc0(sizeof(TSequence *) * ts->count * count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    k += tsequence_time_bucket1(&sequences[k], seq, periods, count);
  }
  ArrayType *result = temporalarr_to_array((const Temporal **) sequences, k);
  pfree_array((void **) sequences, k);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_time_bucket);
/**
 * Split a temporal value into an array of pieces according to time buckets.
 */
PGDLLEXPORT Datum
temporal_time_bucket(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 period = get_interval_units(duration);
  TimestampTz origin = (PG_NARGS() > 2 ?
    PG_GETARG_TIMESTAMPTZ(2) : DEFAULT_TIME_ORIGIN);

  Period p;
  temporal_period(&p, temp);
  TimestampTz start_time = p.lower;
  TimestampTz end_time = p.upper;
  TimestampTz start_bucket = timestamptz_bucket_internal(start_time, period,
    origin);
  TimestampTz end_bucket = timestamptz_bucket_internal(end_time, period,
    origin);
  /* We need to add period to obtain the end timestamp of the last bucket */
  end_bucket = (TimestampTz) ((int64) end_bucket + period);
  int count = (int) (((int64) end_bucket - (int64) start_bucket) / period);
  if (count == 1)
    PG_RETURN_POINTER(temporalarr_to_array((const Temporal **) &temp, 1));

  Period *periods = palloc(sizeof(Period) * count);
  int k = 0;
  TimestampTz lower = start_bucket;
  while (lower < end_bucket)
  {
    TimestampTz upper = (TimestampTz) ((int64) lower + period);
    Period p1;
    period_set(&p1, lower, upper, true, false);
    if (overlaps_period_period_internal(&p, &p1))
      periods[k++] = p1;
    else
      break;
    lower = upper;
  }
  /* k may be one less than count in case p.upper is non inclusive and p.upper
   * is the start of the last bucket */

  ArrayType *result;
  if (temp->temptype == INSTANT)
    result = temporalarr_to_array((const Temporal **) &temp, 1);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_time_bucket((const TInstantSet *) temp, periods, k);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_time_bucket((const TSequence *) temp, periods, k);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_time_bucket((const TSequenceSet *) temp, periods, k);
  pfree(periods);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Return the starting value of the bucket in which a value falls.
 *
 * @param[in] value Input value
 * @param[in] width Width of the buckets
 * @param[in] origin Origin of the buckets
 */
static double
int_bucket(int value, int width, int origin)
{
  int result;
  origin = origin % width;

  if ((origin > 0 && value < PG_INT32_MIN + origin) ||
    (origin < 0 && value > PG_INT32_MAX + origin))
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

/**
 * Return the starting value of the bucket in which a value falls.
 *
 * @param[in] value Input value
 * @param[in] width Width of the buckets
 * @param[in] origin Origin of the buckets
 */
double
float_bucket(double value, double width, double origin)
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

PG_FUNCTION_INFO_V1(tint_range_bucket);
/**
 * Split a temporal number value into an array of pieces according to
 * range buckets.
 */
PGDLLEXPORT Datum
tint_range_bucket(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int width = PG_GETARG_INT32(1);
  if (width <= 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
       errmsg("bucket width must be greater then 0")));
  int origin = (PG_NARGS() > 2 ?
    PG_GETARG_FLOAT8(2) : DEFAULT_RANGE_ORIGIN);

  RangeType *r = tnumber_value_range_internal((const Temporal *) temp);
  int start_value, end_value;
  intrange_bounds(r, &start_value, &end_value);
  int start_bucket = int_bucket(start_value, width, origin);
  /* We need to add width to obtain the end value of the last bucket */
  int end_bucket = int_bucket(end_value, width, origin) + width;
  int count = (end_bucket - start_bucket) / width;
  if (count == 1)
    PG_RETURN_POINTER(temporalarr_to_array((const Temporal **) &temp, 1));

  Temporal **temporals = palloc(sizeof(Temporal *) * count);
  int k = 0;
  int lower = start_bucket;
  while (lower < end_bucket)
  {
    int upper = lower + width;
    RangeType *range = range_make(Int32GetDatum(lower), Int32GetDatum(upper),
      true, false, INT4OID);
    Temporal *atrange = tnumber_restrict_range_internal(temp, range, REST_AT);
    if (atrange != NULL)
      temporals[k++] = atrange;
    pfree(range);
    lower = upper;
  }
  ArrayType *result = temporalarr_to_array((const Temporal **) temporals, k);

  pfree(r);
  pfree_array((void **) temporals, k);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_range_bucket);
/**
 * Split a temporal number value into an array of pieces according to
 * range buckets.
 */
PGDLLEXPORT Datum
tfloat_range_bucket(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  double width = PG_GETARG_FLOAT8(1);
  if (width <= 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
       errmsg("range width must be greater then 0")));
  double origin = (PG_NARGS() > 2 ?
    PG_GETARG_FLOAT8(2) : DEFAULT_RANGE_ORIGIN);

  RangeType *r = tnumber_value_range_internal((const Temporal *) temp);
  double start_value, end_value;
  range_bounds(r, &start_value, &end_value);
  double start_bucket = float_bucket(start_value, width, origin);
  /* We need to add width to obtain the end value of the last bucket */
  double end_bucket = float_bucket(end_value, width, origin) + width;
  int count = (int) ((end_bucket - start_bucket) / width);
  if (count == 1)
    PG_RETURN_POINTER(temporalarr_to_array((const Temporal **) &temp, 1));

  Temporal **temporals = palloc(sizeof(Temporal *) * count);
  int k = 0;
  double lower = start_bucket;
  while (lower < end_bucket)
  {
    double upper = lower + width;
    RangeType *range = range_make(Float8GetDatum(lower), Float8GetDatum(upper),
      true, false, FLOAT8OID);
    Temporal *atrange = tnumber_restrict_range_internal(temp, range, REST_AT);
    if (atrange != NULL)
      temporals[k++] = atrange;
    pfree(range);
    lower = upper;
  }
  ArrayType *result = temporalarr_to_array((const Temporal **) temporals, k);

  pfree(r);
  pfree_array((void **) temporals, k);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
