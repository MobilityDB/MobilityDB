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
 * @file periodset.c
 * Basic functions for set of disjoint periods.
 */

#include "periodset.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "period.h"
#include "timeops.h"
#include "temporal_util.h"
#include "temporal_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Returns the n-th period of the period set value
 */
const Period *
periodset_per_n(const PeriodSet *ps, int index)
{
  return (Period *) &ps->elems[index];
}

/**
 * Returns a pointer to the precomputed bounding box of the period set value
 */
const Period *
periodset_bbox_ptr(const PeriodSet *ps)
{
  return (Period *) &ps->period;
}

/**
 * Copy in the first argument the bounding box of the timestamp set value
 */
void
periodset_bbox(Period *p, const PeriodSet *ps)
{
  const Period *p1 = (Period *)&ps->period;
  period_set(p, p1->lower, p1->upper, p1->lower_inc, p1->upper_inc);
  return;
}

/**
 * Construct a period set from an array of periods
 *
 * For example, the memory structure of a PeriodSet with 3 periods is as
 * follows
 * @code
 * ---------------------------------------------------------------------------------
 * ( PeriodSet | ( bbox )_X | ( Period_0 )_X | ( Period_1 )_X | ( Period_2 )_X )_X |
 * ---------------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and `bbox` is the
 * bounding box which is also period.
 *
 * @param[in] periods Array of periods
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized
 */
PeriodSet *
periodset_make(const Period **periods, int count, bool normalize)
{
  /* Test the validity of the periods */
  for (int i = 0; i < count - 1; i++)
  {
    int cmp = timestamp_cmp_internal(periods[i]->upper, periods[i + 1]->lower);
    if (cmp > 0 ||
      (cmp == 0 && periods[i]->upper_inc && periods[i + 1]->lower_inc))
      ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
        errmsg("Invalid value for period set")));
  }

  Period **newperiods = (Period **) periods;
  int newcount = count;
  if (normalize && count > 1)
    newperiods = periodarr_normalize((Period **) periods, count, &newcount);
  /* Notice that the first period is already declared in the struct */
  size_t memsize = double_pad(sizeof(PeriodSet) + sizeof(Period) * (newcount - 1));
  PeriodSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;

  /* Compute the bounding box */
  period_set(&result->period, newperiods[0]->lower, newperiods[newcount - 1]->upper,
    newperiods[0]->lower_inc, newperiods[newcount - 1]->upper_inc);
  /* Copy the period array */
  for (int i = 0; i < newcount; i++)
    period_set(&result->elems[i], newperiods[i]->lower, newperiods[i]->upper,
      newperiods[i]->lower_inc, newperiods[i]->upper_inc);
  /* Free after normalization */
  if (normalize && count > 1)
    pfree_array((void **) newperiods, newcount);
  return result;
}

/**
 * Construct a period set from an array of periods and free the array and the
 * periods after the creation
 *
 * @param[in] periods Array of periods
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 */
PeriodSet *
periodset_make_free(Period **periods, int count, bool normalize)
{
  if (count == 0)
  {
    pfree(periods);
    return NULL;
  }
  PeriodSet *result = periodset_make((const Period **) periods,
    count, normalize);
  pfree_array((void **) periods, count);
  return result;
}

/**
 * Construct a period set from a period (internal function)
 */
PeriodSet *
period_to_periodset_internal(const Period *period)
{
  return periodset_make((const Period **)&period, 1, NORMALIZE_NO);
}

/**
 * Returns a copy of the period set
 */
PeriodSet *
periodset_copy(const PeriodSet *ps)
{
  PeriodSet *result = palloc(VARSIZE(ps));
  memcpy(result, ps, VARSIZE(ps));
  return result;
}

/**
 * Returns the location of the timestamp in the temporal sequence set
 * value using binary search
 *
 * If the timestamp is found, the index of the period is returned
 * in the output parameter. Otherwise, return a number encoding whether the
 * timestamp is before, between two periods, or after the period set value.
 * For example, given a value composed of 3 periods and a timestamp, the
 * result of the function is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|
 * 1)    t^                                        => loc = 0
 * 2)            t^                                => loc = 0
 * 3)                 t^                           => loc = 1
 * 4)                            t^                => loc = 2
 * 5)                                        t^    => loc = 3
 * @endcode
 * @param[in] ps Period set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Returns true if the timestamp is contained in the period set value
 */
bool
periodset_find_timestamp(const PeriodSet *ps, TimestampTz t, int *loc)
{
  int first = 0;
  int last = ps->count - 1;
  int middle = 0; /* make compiler quiet */
  const Period *p = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    p = periodset_per_n(ps, middle);
    if (contains_period_timestamp_internal(p, t))
    {
      *loc = middle;
      return true;
    }
    if (t <= p->lower)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (t >= p->upper)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(periodset_in);
/**
 * Input function for period set values
 */
PGDLLEXPORT Datum
periodset_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  PeriodSet *result = periodset_parse(&input);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the string representation of the period set value
 */
char *
periodset_to_string(const PeriodSet *ps)
{
  char **strings = palloc(sizeof(char *) * ps->count);
  size_t outlen = 0;

  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    strings[i] = period_to_string(p);
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ps->count, outlen, "", '{', '}');
}

PG_FUNCTION_INFO_V1(periodset_out);
/**
 * Output function for period set values
 */
PGDLLEXPORT Datum
periodset_out(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  char *result = periodset_to_string(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(periodset_send);
/**
 * Send function for period set values
 */
PGDLLEXPORT Datum
periodset_send(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
#if MOBDB_PGSQL_VERSION < 110000
  pq_sendint(&buf, (uint32) ps->count, 4);
#else
  pq_sendint32(&buf, ps->count);
#endif
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    period_write(p, &buf);
  }
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(periodset_recv);
/**
 * Receive function for period set values
 */
PGDLLEXPORT Datum
periodset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  int count = (int) pq_getmsgint(buf, 4);
  Period **periods = palloc(sizeof(Period *) * count);
  for (int i = 0; i < count; i++)
    periods[i] = period_read(buf);
  PeriodSet *result = periodset_make_free(periods, count, NORMALIZE_NO);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor function
 ****************************************************************************/

PG_FUNCTION_INFO_V1(periodset_constructor);
/**
 * Construct a period set from an array of period values
 */
PGDLLEXPORT Datum
periodset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Period **periods = periodarr_extract(array, &count);
  PeriodSet *result = periodset_make((const Period **) periods, count, NORMALIZE);

  pfree(periods);
  PG_FREE_IF_COPY(array, 0);

  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * Cast the timestamp value as a period set value (internal function)
 */
PeriodSet *
timestamp_to_periodset_internal(TimestampTz t)
{
  Period p;
  period_set(&p, t, t, true, true);
  PeriodSet *result = period_to_periodset_internal(&p);
  return result;
}

PG_FUNCTION_INFO_V1(timestamp_to_periodset);
/**
 * Cast the timestamp value as a period set value
 */
PGDLLEXPORT Datum
timestamp_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *result = timestamp_to_periodset_internal(t);
  PG_RETURN_POINTER(result);
}

/**
 * Cast the timestamp set value as a period set value (internal function)
 */
PeriodSet *
timestampset_to_periodset_internal(const TimestampSet *ts)
{
  Period **periods = palloc(sizeof(Period *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    periods[i] = period_make(t, t, true, true);
  }
  PeriodSet *result = periodset_make_free(periods, ts->count, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(timestampset_to_periodset);
/**
 * Cast the timestamp set value as a period set value
 */
PGDLLEXPORT Datum
timestampset_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *result = timestampset_to_periodset_internal(ts);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(period_to_periodset);
/**
 * Cast the period value as a period set value
 */
PGDLLEXPORT Datum
period_to_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *result = period_to_periodset_internal(p);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the bounding period on which the period set value is defined
 * (internal function)
 */
void
periodset_to_period_internal(Period *p, const PeriodSet *ps)
{
  const Period *start = periodset_per_n(ps, 0);
  const Period *end = periodset_per_n(ps, ps->count - 1);
  period_set(p, start->lower, end->upper,
    start->lower_inc, end->upper_inc);
}

PG_FUNCTION_INFO_V1(periodset_to_period);
/**
 * Returns the bounding period on which the period set value is defined
 */
PGDLLEXPORT Datum
periodset_to_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *result = (Period *) palloc(sizeof(Period));
  periodset_to_period_internal(result, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(periodset_mem_size);
/**
 * Returns the size in bytes of the period set value
 */
PGDLLEXPORT Datum
periodset_mem_size(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Datum result = Int32GetDatum((int)VARSIZE(DatumGetPointer(ps)));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(periodset_timespan);
/**
 * Returns the timespan of the period set value
 */
PGDLLEXPORT Datum
periodset_timespan(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  Datum result = call_function2(timestamp_mi, TimestampTzGetDatum(p2->upper),
    TimestampTzGetDatum(p1->lower));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(periodset_duration);
/**
 * Returns the timespan of the period set value
 */
PGDLLEXPORT Datum
periodset_duration(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period *p = periodset_per_n(ps, 0);
  Datum result = call_function2(timestamp_mi, TimestampTzGetDatum(p->upper),
    TimestampTzGetDatum(p->lower));
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    Datum interval1 = call_function2(timestamp_mi,
      TimestampTzGetDatum(p->upper), TimestampTzGetDatum(p->lower));
    Datum interval2 = call_function2(interval_pl, result, interval1);
    pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
    result = interval2;
  }
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(periodset_num_periods);
/**
 * Returns the number of periods of the period set value
 */
PGDLLEXPORT Datum
periodset_num_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  int result = ps->count;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(periodset_start_period);
/**
 * Returns the start period of the period set value
 */
PGDLLEXPORT Datum
periodset_start_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *result = period_copy(periodset_per_n(ps, 0));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(periodset_end_period);
/**
 * Returns the end period of the period set value
 */
PGDLLEXPORT Datum
periodset_end_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *result = period_copy(periodset_per_n(ps, ps->count - 1));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(periodset_period_n);
/**
 * Returns the n-th period of the period set value
 */
PGDLLEXPORT Datum
periodset_period_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  Period *result = NULL;
  if (i >= 1 && i <= ps->count)
    result = period_copy(periodset_per_n(ps, i - 1));
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the periods of the period set value (internal function)
 */
const Period **
periodset_periods_internal(const PeriodSet *ps)
{
  const Period **periods = palloc(sizeof(Period *) * ps->count);
  for (int i = 0; i < ps->count; i++)
    periods[i] = periodset_per_n(ps, i);
  return periods;
}

PG_FUNCTION_INFO_V1(periodset_periods);
/**
 * Returns the periods of the period set value
 */
PGDLLEXPORT Datum
periodset_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period **periods = periodset_periods_internal(ps);
  ArrayType *result = periodarr_to_array(periods, ps->count);
  pfree(periods);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(periodset_num_timestamps);
/**
 * Returns the number of timestamps of the period set value
 */
PGDLLEXPORT Datum
periodset_num_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz prev = p->lower;
  bool start = false;
  int result = 1;
  TimestampTz d;
  int i = 1;
  while (i < ps->count || !start)
  {
    if (start)
    {
      p = periodset_per_n(ps, i++);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      result++;
      prev = d;
    }
  }
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

/**
 * Returns the start timestamp of the period set value (internal function)
 */
TimestampTz
periodset_start_timestamp_internal(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return p->lower;
}

PG_FUNCTION_INFO_V1(periodset_start_timestamp);
/**
 * Returns the start timestamp of the period set value
 */
PGDLLEXPORT Datum
periodset_start_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz result = p->lower;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

/**
 * Returns the end timestamp of the period set value (internal function)
 */
TimestampTz
periodset_end_timestamp_internal(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return p->upper;
}

PG_FUNCTION_INFO_V1(periodset_end_timestamp);
/**
 * Returns the end timestamp of the period set value
 */
PGDLLEXPORT Datum
periodset_end_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz result = p->upper;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(periodset_timestamp_n);
/**
 * Returns the n-th timestamp of the period set value
 */
PGDLLEXPORT Datum
periodset_timestamp_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  int pernum = 0;
  const Period *p = periodset_per_n(ps, pernum);
  TimestampTz d = p->lower;
  if (n == 1)
  {
    PG_FREE_IF_COPY(ps, 0);
    PG_RETURN_TIMESTAMPTZ(d);
  }

  bool start = false;
  int i = 1;
  TimestampTz prev = d;
  while (i < n)
  {
    if (start)
    {
      pernum++;
      if (pernum == ps->count)
        break;

      p = periodset_per_n(ps, pernum);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      i++;
      prev = d;
    }
  }
  PG_FREE_IF_COPY(ps, 0);
  if (i != n)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(d);
}

PG_FUNCTION_INFO_V1(periodset_timestamps);
/**
 * Returns the timestamps of the period set value
 */
PGDLLEXPORT Datum
periodset_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz *times = palloc(sizeof(TimestampTz) * 2 * ps->count);
  const Period *p = periodset_per_n(ps, 0);
  times[0] = p->lower;
  int k = 1;
  if (p->lower != p->upper)
    times[k++] = p->upper;
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    if (times[k - 1] != p->lower)
      times[k++] = p->lower;
    if (times[k - 1] != p->upper)
      times[k++] = p->upper;
  }
  ArrayType *result = timestamparr_to_array(times, k);
  pfree(times);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/**
 * Shift the period set value by the interval (internal function)
 */
PeriodSet *
periodset_shift_internal(const PeriodSet *ps, const Interval *interval)
{
  Period **periods = palloc(sizeof(Period *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    periods[i] = period_shift_internal(p, interval);
  }
  PeriodSet *result = periodset_make_free(periods, ps->count, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(periodset_shift);
/**
 * Shift the period set value by the interval
 */
PGDLLEXPORT Datum
periodset_shift(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  PeriodSet *result = periodset_shift_internal(ps, interval);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first period set value
 * is less than, equal, or greater than the second temporal value
 * (internal function)
 *
 * @note Function used for B-tree comparison
 */
int
periodset_cmp_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int count1 = ps1->count;
  int count2 = ps2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    result = period_cmp_internal(p1, p2);
    if (result)
      break;
  }
  /* The first count periods of the two PeriodSet are equal */
  if (!result)
  {
    if (count < count1) /* ps1 has more PeriodSet than ps2 */
      result = 1;
    else if (count < count2) /* ps2 has more PeriodSet than ps1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

PG_FUNCTION_INFO_V1(periodset_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first period set value
 * is less than, equal, or greater than the second temporal value
 */
PGDLLEXPORT Datum
periodset_cmp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  int cmp = periodset_cmp_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_INT32(cmp);
}

/**
 * Returns true if the first period set value is equal to the second one
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
periodset_eq_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  if (ps1->count != ps2->count)
    return false;
  /* ps1 and ps2 have the same number of PeriodSet */
  for (int i = 0; i < ps1->count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    if (period_ne_internal(p1, p2))
      return false;
  }
  /* All periods of the two PeriodSet are equal */
  return true;
}

PG_FUNCTION_INFO_V1(periodset_eq);
/**
 * Returns true if the first period set value is equal to the second one
 */
PGDLLEXPORT Datum
periodset_eq(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = periodset_eq_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first period set value is different from the second one
 * (internal function)
 */
bool
periodset_ne_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  return !periodset_eq_internal(ps1, ps2);
}

PG_FUNCTION_INFO_V1(periodset_ne);
/**
 * Returns true if the first period set value is different from the second one
 */
PGDLLEXPORT Datum
periodset_ne(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = periodset_ne_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(periodset_lt);
/**
 * Returns true if the first period set value is less than the second one
 */
PGDLLEXPORT Datum
periodset_lt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  int cmp = periodset_cmp_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(periodset_le);
/**
 * Returns true if the first period set value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
periodset_le(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  int cmp = periodset_cmp_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(periodset_ge);
/**
 * Returns true if the first period set value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
periodset_ge(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  int cmp = periodset_cmp_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(periodset_gt);
/**
 * Returns true if the first period set value is greater than the second one
 */
PGDLLEXPORT Datum
periodset_gt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  int cmp = periodset_cmp_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(cmp > 0);
}

/*****************************************************************************/
