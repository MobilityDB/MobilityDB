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
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Bucket functions
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
    {
      elog(ERROR, "number out of span");
    }
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
    {
      elog(ERROR, "number out of span");
    }
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
 * Return the initial timestamp of the bucket in which a timestamp falls.
 *
 * @param[in] timestamp Input timestamp
 * @param[in] size Size of the time buckets in PostgreSQL time units
 * @param[in] offset Origin of the buckets
 */
TimestampTz
timestamptz_bucket(TimestampTz timestamp, int64 size, TimestampTz offset)
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
    {
      elog(ERROR, "timestamp out of span");
    }
    else
      result -= size;
  }
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
Datum
datum_bucket(Datum value, Datum size, Datum offset, mobdbType basetype)
{
  ensure_span_basetype(basetype);
  if (basetype == T_INT4)
    return Int32GetDatum(int_bucket(DatumGetInt32(value),
      DatumGetInt32(size), DatumGetInt32(offset)));
  else if (basetype == T_FLOAT8)
    return Float8GetDatum(float_bucket(DatumGetFloat8(value),
      DatumGetFloat8(size), DatumGetFloat8(offset)));
  else /* basetype == T_TIMESTAMPTZ */
    return TimestampTzGetDatum(timestamptz_bucket(DatumGetTimestampTz(value),
      DatumGetInt64(size), DatumGetTimestampTz(offset)));
}

/*****************************************************************************
 * Time bucket functions
 *****************************************************************************/

/*****************************************************************************
 * Time split functions for temporal numbers
 *****************************************************************************/

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
 * @param[in] seq Temporal value
 * @param[in] start Start timestamp of the buckets
 * @param[in] tunits Size of the time buckets in PostgreSQL time units
 * @param[in] count Number of buckets
 * @param[out] buckets Start timestamp of the buckets containing a fragment
 * @param[out] newcount Number of values in the output array
 */
static TSequence **
tdiscseq_time_split(const TSequence *seq, TimestampTz start,
  int64 tunits, int count, TimestampTz **buckets, int *newcount)
{
  TSequence **result = palloc(sizeof(TSequence *) * count);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  const TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
  int i = 0,  /* counter for instants of temporal value */
      k = 0,  /* counter for instants of next split */
      l = 0;  /* counter for resulting fragments */
  TimestampTz lower = start;
  TimestampTz upper = start + tunits;
  while (i < seq->count)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
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
        result[l++] = tsequence_make(instants, k, k, true, true, DISCRETE,
          NORMALIZE_NO);
        k = 0;
      }
      lower = upper;
      upper += tunits;
    }
  }
  if (k > 0)
  {
    times[l] = lower;
    result[l++] = tsequence_make(instants, k, k, true, true, DISCRETE,
      NORMALIZE_NO);
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
  /* This loop is needed for filtering unnecesary time buckets for the
   * sequences composing a sequence set.
   * The upper bound for the bucket is exclusive => the test below is >= */
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
      result[m++] = tsequence_make(instants, k, k, lower_inc1,
         (k > 1) ? false : true, linear ? LINEAR : STEPWISE, NORMALIZE);
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
    result[m++] = tsequence_make(instants, k, k, lower_inc1,
      seq->period.upper_inc, linear ? LINEAR : STEPWISE, NORMALIZE);
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
temporal_time_split(const Temporal *temp, TimestampTz start, TimestampTz end,
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
  else if (temp->subtype == TSEQUENCE)
    fragments = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal **) tdiscseq_time_split((const TSequence *) temp,
        start, tunits, count, buckets, newcount) :
      (Temporal **) tsequence_time_split((const TSequence *) temp,
        start, end, tunits, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tsequenceset_time_split((const TSequenceSet *) temp,
      start, end, tunits, count, buckets, newcount);
  return fragments;
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
  values[0] = datum_bucket(value, size, start_bucket, basetype);
  *buckets = values;
  *newcount = 1;
  return result;
}

/*****************************************************************************/

/**
 * Split a temporal value into an array of fragments according to value buckets.
 *
 * @param[in] seq Temporal value
 * @param[in] size Size of the value buckets
 * @param[in] start_bucket Value of the start bucket
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing a fragment
 * @param[out] newcount Number of values in the output arrays
 */
static TSequence **
tdiscseq_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount)
{
  mobdbType basetype = temptype_basetype(seq->temptype);
  TSequence **result;
  Datum *values, value, bucket_value;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    result = palloc(sizeof(TSequence *));
    values = palloc(sizeof(Datum));
    result[0] = tsequence_copy(seq);
    value = tinstant_value(tsequence_inst_n(seq, 0));
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
    const TInstant *inst = tsequence_inst_n(seq, i);
    value = tinstant_value(inst);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    int bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    int inst_no = numinsts[bucket_no]++;
    instants[bucket_no * seq->count + inst_no] = inst;
  }
  /* Assemble the result for each value bucket */
  result = palloc(sizeof(TSequence *) * count);
  values = palloc(sizeof(Datum) * count);
  int k = 0;
  bucket_value = start_bucket;
  for (int i = 0; i < count; i++)
  {
    if (numinsts[i] > 0)
    {
      result[k] = tsequence_make(&instants[i * seq->count], numinsts[i],
        numinsts[i], true, true, DISCRETE, NORMALIZE_NO);
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
  assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum value, bucket_value;
  int bucket_no, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value = tinstant_value(tsequence_inst_n(seq, 0));
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tsequence_copy(seq);
    return;
  }

  /* General case */
  TInstant **tofree = palloc(sizeof(TInstant *) * count * seq->count);
  int l = 0;   /* counter for the instants to free */
  const TInstant *inst1 = tsequence_inst_n(seq, 0);
for (int i = 1; i < seq->count; i++)
  {
    value = tinstant_value(inst1);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
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
      k, k, lower_inc1, false, STEPWISE, NORMALIZE);
    bounds[0] = bounds[1];
    inst1 = inst2;
    lower_inc1 = true;
  }
  /* Last value if upper inclusive */
  if (seq->period.upper_inc)
  {
    inst1 = tsequence_inst_n(seq, seq->count - 1);
    value = tinstant_value(inst1);
    bucket_value = datum_bucket(value, size, start_bucket, basetype);
    bucket_no = bucket_position(bucket_value, size, start_bucket, basetype);
    seq_no = numseqs[bucket_no]++;
    result[bucket_no * numcols + seq_no] = tinstant_to_tsequence(inst1, STEPWISE);
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
 * @param[in,out] result Array containing the fragments of each bucket
 * @param[in,out] numseqs Number of fragments for each bucket
 * @param[in] numcols Number of columns in the 2D pointer array. It can be
 *    seq->count for sequences or ss->totalcount for sequence sets
 */
static void
tnumberseq_linear_value_split(const TSequence *seq, Datum start_bucket,
  Datum size, int count, TSequence **result, int *numseqs, int numcols)
{
  assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
  mobdbType basetype = temptype_basetype(seq->temptype);
  Datum value1, bucket_value1;
  int bucket_no1, seq_no;

  /* Instantaneous sequence */
  if (seq->count == 1)
  {
    value1 = tinstant_value(tsequence_inst_n(seq, 0));
    bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
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
  bucket_value1 = datum_bucket(value1, size, start_bucket, basetype);
  bucket_no1 = bucket_position(bucket_value1, size, start_bucket, basetype);
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = tsequence_inst_n(seq, i);
    Datum value2 = tinstant_value(inst2);
    Datum bucket_value2 = datum_bucket(value2, size, start_bucket, basetype);
    int bucket_no2 = bucket_position(bucket_value2, size, start_bucket, basetype);
    /* Take into account on whether the segment seq increasing or decreasing */
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
        Span bucketspan;
        span_set(bucket_lower, bucket_upper, true, false, basetype,
          &bucketspan);
        Span inter;
        bool found = inter_span_span(segspan, &bucketspan, &inter);
        if (found)
        {
          if (incr)
          {
            lower_inc1 = inter.lower_inc;
            upper_inc1 = inter.upper_inc;
          }
          else
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
      int k = (bounds[0]->t == bounds[1]->t) ? 1 : 2;
      /* We cannot add to last bucket if last instant has exclusive bound */
      if (k == 1 && ! upper_inc1)
        break;
      seq_no = numseqs[j]++;
      result[j * numcols + seq_no] = tsequence_make((const TInstant **) bounds,
        k, k, (k > 1) ? lower_inc1 : true, (k > 1) ? upper_inc1 : true,
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
    values[0] = datum_bucket(value, size, start_bucket, basetype);
    *buckets = values;
    *newcount = 1;
    return result;
  }

  /* General case */
  TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * count);
  /* palloc0 to initialize the counters to 0 */
  int *numseqs = palloc0(sizeof(int) * count);
  if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
    tnumberseq_linear_value_split(seq, start_bucket, size, count, sequences,
      numseqs, seq->count);
  else
    tnumberseq_step_value_split(seq, start_bucket, size, count, sequences,
      numseqs, seq->count);
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

/**
 * @ingroup libmeos_int_temporal_tile
 * @brief Split a temporal number into an array of fragments according to value
 * buckets.
 */
Temporal **
tnumber_value_split(const Temporal *temp, Datum start_bucket, Datum size,
  int count, Datum **buckets, int *newcount)
{
  assert(count > 0);
  /* Split the temporal value */
  Temporal **fragments;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == TINSTANT)
    fragments = (Temporal **) tnumberinst_value_split((const TInstant *) temp,
      start_bucket, size, buckets, newcount);
  else if (temp->subtype == TSEQUENCE)
    fragments = MOBDB_FLAGS_GET_DISCRETE(temp->flags) ?
      (Temporal **) tdiscseq_value_split((const TSequence *) temp,
        start_bucket, size, count, buckets, newcount) :
      (Temporal **) tnumberseq_value_split((const TSequence *) temp,
        start_bucket, size, count, buckets, newcount);
  else /* temp->subtype == TSEQUENCESET */
    fragments = (Temporal **) tnumberseqset_value_split((const TSequenceSet *) temp,
      start_bucket, size, count, buckets, newcount);
  return fragments;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal integer into an array of fragments according to
 * value buckets.
 *
 * @param[in] temp Temporal value
 * @param[in] start_bucket Start value of the first bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing the fragments
 * @param[out] newcount Number of values in the output arrays
 */
Temporal **
tint_value_split(const Temporal *temp, int start_bucket, int size,
  int count, int **buckets, int *newcount)
{
  Temporal **result = tnumber_value_split(temp, Int32GetDatum(start_bucket),
    Int32GetDatum(size), count, (Datum **) buckets, newcount);
  for (int i = 0; i < count; i++)
    *buckets[i] = DatumGetInt32(*buckets[i]);
  return result;
}

/**
 * @ingroup libmeos_temporal_tile
 * @brief Split a temporal float into an array of fragments according to value
 * buckets.
 *
 * @param[in] temp Temporal value
 * @param[in] start_bucket Start value of the first bucket
 * @param[in] size Size of the value buckets
 * @param[in] count Number of buckets
 * @param[out] buckets Start value of the buckets containing the fragments
 * @param[out] newcount Number of values in the output arrays
 */
Temporal **
tfloat_value_split(const Temporal *temp, double start_bucket, double size,
  int count, float **buckets, int *newcount)
{
  Temporal **result = tnumber_value_split(temp, Float8GetDatum(start_bucket),
    Float8GetDatum(size), count, (Datum **) buckets, newcount);
  for (int i = 0; i < count; i++)
    *buckets[i] = DatumGetFloat8(*buckets[i]);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
