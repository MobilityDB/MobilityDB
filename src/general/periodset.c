/*****************************************************************************
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
 * @file periodset.c
 * @brief General functions for set of disjoint periods.
 */

#include "general/periodset.h"

/* PostgreSQL */
#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/time_ops.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the n-th period of the period set value.
 */
const Period *
periodset_per_n(const PeriodSet *ps, int index)
{
  return (Period *) &ps->elems[index];
}

/**
 * Return a pointer to the precomputed bounding box of the period set value
 */
const Period *
periodset_bbox_ptr(const PeriodSet *ps)
{
  return (Period *) &ps->period;
}

/**
 * Copy in the second argument the bounding box of the timestamp set value
 */
void
periodset_bbox(const PeriodSet *ps, Period *p)
{
  const Period *p1 = (Period *) &ps->period;
  period_set(p1->lower, p1->upper, p1->lower_inc, p1->upper_inc, p);
  return;
}

/**
 * Peak into a period set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
periodset_bbox_slice(Datum psdatum, Period *p)
{
  PeriodSet *ps = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) psdatum))
    ps = (PeriodSet *) PG_DETOAST_DATUM_SLICE(psdatum, 0,
      time_max_header_size());
  else
    ps = (PeriodSet *) psdatum;
  periodset_bbox(ps, p);
  PG_FREE_IF_COPY_P(ps, DatumGetPointer(psdatum));
  return;
}

/**
 * @ingroup libmeos_time_constructor
 * @brief Construct a period set from an array of disjoint periods.
 *
 * For example, the memory structure of a PeriodSet with 3 periods is as
 * follows
 * @code
 * ---------------------------------------------------------------------------------
 * ( PeriodSet )_X | ( bbox )_X | ( Period_0 )_X | ( Period_1 )_X | ( Period_2 )_X |
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
  size_t memsize = double_pad(sizeof(PeriodSet)) + double_pad(sizeof(Period)) * (newcount - 1);
  PeriodSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;

  /* Compute the bounding box */
  period_set(newperiods[0]->lower, newperiods[newcount - 1]->upper,
    newperiods[0]->lower_inc, newperiods[newcount - 1]->upper_inc,
    &result->period);
  /* Copy the period array */
  for (int i = 0; i < newcount; i++)
    period_set(newperiods[i]->lower, newperiods[i]->upper,
      newperiods[i]->lower_inc, newperiods[i]->upper_inc, &result->elems[i]);
  /* Free after normalization */
  if (normalize && count > 1)
    pfree_array((void **) newperiods, newcount);
  return result;
}

/**
 * @ingroup libmeos_time_constructor
 * @brief Construct a period set from an array of periods and free the array
 * and the periods after the creation.
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
  PeriodSet *result = periodset_make((const Period **) periods, count,
    normalize);
  pfree_array((void **) periods, count);
  return result;
}

/**
 * @ingroup libmeos_time_constructor
 * @brief Return a copy of the period set.
 */
PeriodSet *
periodset_copy(const PeriodSet *ps)
{
  PeriodSet *result = palloc(VARSIZE(ps));
  memcpy(result, ps, VARSIZE(ps));
  return result;
}

/**
 * Return the location of the timestamp in the temporal sequence set
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
 * @result Return true if the timestamp is contained in the period set value
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
    if (contains_period_timestamp(p, t))
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

/**
 * @ingroup libmeos_time_input_output
 * @brief Return the string representation of the period set value.
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

/**
 * @ingroup libmeos_time_input_output
 * @brief Write the binary representation of the time value into the buffer.
 *
 * @param[in] ps Time value
 * @param[in] buf Buffer
 */
void
periodset_write(const PeriodSet *ps, StringInfo buf)
{
  pq_sendint32(buf, ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    period_write(p, buf);
  }
  return;
}

/**
 * @ingroup libmeos_time_input_output
 * @brief Return a new time value from its binary representation
 * read from the buffer.
 *
 * @param[in] buf Buffer
 */
PeriodSet *
periodset_read(StringInfo buf)
{
  int count = (int) pq_getmsgint(buf, 4);
  Period **periods = palloc(sizeof(Period *) * count);
  for (int i = 0; i < count; i++)
    periods[i] = period_read(buf);
  PeriodSet *result = periodset_make_free(periods, count, NORMALIZE_NO);
  return result;
}

/*****************************************************************************
 * Constructor function
 ****************************************************************************/

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * @ingroup libmeos_time_cast
 * @brief Cast the timestamp value as a period set value.
 */
PeriodSet *
timestamp_periodset(TimestampTz t)
{
  Period p;
  period_set(t, t, true, true, &p);
  PeriodSet *result = period_periodset(&p);
  return result;
}

/**
 * @ingroup libmeos_time_cast
 * @brief Cast the timestamp set value as a period set value.
 */
PeriodSet *
timestampset_periodset(const TimestampSet *ts)
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

/**
 * @ingroup libmeos_time_cast
 * @brief Construct a period set from a period.
 */
PeriodSet *
period_periodset(const Period *period)
{
  return periodset_make((const Period **) &period, 1, NORMALIZE_NO);
}

/**
 * @ingroup libmeos_time_cast
 * @brief Return the bounding period of the period set value.
 */
void
periodset_period(const PeriodSet *ps, Period *p)
{
  const Period *start = periodset_per_n(ps, 0);
  const Period *end = periodset_per_n(ps, ps->count - 1);
  /* Note: zero-fill is required here, just as in heap tuples */
  memset(p, 0, sizeof(Period));
  period_set(start->lower, end->upper, start->lower_inc, end->upper_inc, p);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the size in bytes of the period set value
 */
int
periodset_mem_size(const PeriodSet *ps)
{
  return (int) VARSIZE(DatumGetPointer(ps));
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the timespan of the period set value
 */
Interval *
periodset_timespan(const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  Interval *result = (Interval *) DatumGetPointer(call_function2(timestamp_mi,
    TimestampTzGetDatum(p2->upper), TimestampTzGetDatum(p1->lower)));
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the duration of the period set value
 */
Interval *
periodset_duration(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  Datum result = call_function2(timestamp_mi,
    TimestampTzGetDatum(p->upper),  TimestampTzGetDatum(p->lower));
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    Datum interval1 = call_function2(timestamp_mi,
      TimestampTzGetDatum(p->upper), TimestampTzGetDatum(p->lower));
    Datum interval2 = call_function2(interval_pl, result, interval1);
    pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
    result = interval2;
  }
  return (Interval *) DatumGetPointer(result);
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the number of periods of the period set value
 */
int
periodset_num_periods(const PeriodSet *ps)
{
  return ps->count;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the end period of the period set value
 */
Period *
periodset_start_period(const PeriodSet *ps)
{
  Period *result = period_copy(periodset_per_n(ps, 0));
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the end period of the period set value
 */
Period *
periodset_end_period(const PeriodSet *ps)
{
  Period *result = period_copy(periodset_per_n(ps, ps->count - 1));
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the n-th period of the period set value
 */
Period *
periodset_period_n(const PeriodSet *ps, int i)
{
  Period *result = NULL;
  if (i >= 1 && i <= ps->count)
    result = period_copy(periodset_per_n(ps, i - 1));
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the periods of the period set value.
 */
const Period **
periodset_periods(const PeriodSet *ps)
{
  const Period **periods = palloc(sizeof(Period *) * ps->count);
  for (int i = 0; i < ps->count; i++)
    periods[i] = periodset_per_n(ps, i);
  return periods;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the number of timestamps of the period set value
 */
int
periodset_num_timestamps(const PeriodSet *ps)
{
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
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the start timestamp of the period set value.
 */
TimestampTz
periodset_start_timestamp(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return p->lower;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the end timestamp of the period set value.
 */
TimestampTz
periodset_end_timestamp(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return p->upper;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the n-th timestamp of the period set value.
 *
 * @param[in] ps Period set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 */
bool
periodset_timestamp_n(const PeriodSet *ps, int n, TimestampTz *result)
{
  int pernum = 0;
  const Period *p = periodset_per_n(ps, pernum);
  TimestampTz d = p->lower;
  if (n == 1)
  {
    *result = d;
    return true;
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
  if (i != n)
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the timestamps of the period set value
 */
TimestampTz *
periodset_timestamps(const PeriodSet *ps, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * 2 * ps->count);
  const Period *p = periodset_per_n(ps, 0);
  result[0] = p->lower;
  int k = 1;
  if (p->lower != p->upper)
    result[k++] = p->upper;
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    if (result[k - 1] != p->lower)
      result[k++] = p->lower;
    if (result[k - 1] != p->upper)
      result[k++] = p->upper;
  }
  *count = k;
  return result;
}

/*****************************************************************************
 * Modifications functions
 *****************************************************************************/

/**
 * @ingroup libmeos_time_transf
 * @brief Shift and/or scale the period set value by the two intervals.
 */
PeriodSet *
periodset_shift_tscale(const PeriodSet *ps, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  bool instant = (ps->period.lower == ps->period.upper);

  /* Copy the input period set to the output period set */
  PeriodSet *result = periodset_copy(ps);
  /* Shift and/or scale the bounding period */
  period_shift_tscale(start, duration, &result->period);
  /* Shift and/or scale the periods of the period set */
  TimestampTz shift;
  if (start != NULL)
    shift = result->period.lower - ps->period.lower;
  /* If the periodset is instantaneous we cannot scale */
  double scale;
  if (duration != NULL && ! instant)
    scale =
      (double) (result->period.upper - result->period.lower) /
      (double) (ps->period.upper - ps->period.lower) ;
  for (int i = 0; i < ps->count; i++)
  {
    if (start != NULL)
    {
      result->elems[i].lower += shift;
      result->elems[i].upper += shift;
    }
    if (duration != NULL && ! instant)
    {
      result->elems[i].lower = result->period.lower +
        (result->elems[i].lower - result->period.lower) * scale;
      result->elems[i].upper = result->period.lower +
        (result->elems[i].upper - result->period.lower) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

/**
 * @ingroup libmeos_time_comp
 * @brief Return -1, 0, or 1 depending on whether the first period set value
 * is less than, equal, or greater than the second temporal value.
 *
 * @note Function used for B-tree comparison
 */
int
periodset_cmp(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int count1 = ps1->count;
  int count2 = ps2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    result = period_cmp(p1, p2);
    if (result)
      break;
  }
  /* The first count periods of the two PeriodSet are equal */
  if (! result)
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

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is equal to the second one.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
periodset_eq(const PeriodSet *ps1, const PeriodSet *ps2)
{
  if (ps1->count != ps2->count)
    return false;
  /* ps1 and ps2 have the same number of PeriodSet */
  for (int i = 0; i < ps1->count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    if (period_ne(p1, p2))
      return false;
  }
  /* All periods of the two PeriodSet are equal */
  return true;
}

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is different from the
 * second one.
 */
bool
periodset_ne(const PeriodSet *ps1, const PeriodSet *ps2)
{
  return ! periodset_eq(ps1, ps2);
}

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is less than the second one
 */
bool
periodset_lt(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is less than or equal to
 * the second one
 */
bool
periodset_le(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is greater than or equal to
 * the second one
 */
bool
periodset_ge(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period set value is greater than the second
 * one
 */
bool
periodset_gt(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp > 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the 32-bit hash value of a period set value.
 */
uint32
periodset_hash(const PeriodSet *ps)
{
  uint32 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    uint32 per_hash = period_hash(p);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the 64-bit hash value of a period set using a seed
 */
uint64
periodset_hash_extended(const PeriodSet *ps, Datum seed)
{
  uint64 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    uint64 per_hash = period_hash_extended(p, seed);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_in);
/**
 * Input function for period set values
 */
PGDLLEXPORT Datum
Periodset_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  PeriodSet *result = periodset_parse(&input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_out);
/**
 * Output function for period set values
 */
PGDLLEXPORT Datum
Periodset_out(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  char *result = periodset_to_string(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Periodset_send);
/**
 * Send function for period set values
 */
PGDLLEXPORT Datum
Periodset_send(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  periodset_write(ps, &buf) ;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Periodset_recv);
/**
 * Receive function for period set values
 */
PGDLLEXPORT Datum
Periodset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  PeriodSet *result = periodset_read(buf);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor function
 ****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_constructor);
/**
 * Construct a period set from an array of period values
 */
PGDLLEXPORT Datum
Periodset_constructor(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(Timestamp_to_periodset);
/**
 * Cast the timestamp value as a period set value
 */
PGDLLEXPORT Datum
Timestamp_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *result = timestamp_periodset(t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_to_periodset);
/**
 * Cast the timestamp set value as a period set value
 */
PGDLLEXPORT Datum
Timestampset_to_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *result = timestampset_periodset(ts);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_periodset);
/**
 * Cast the period value as a period set value
 */
PGDLLEXPORT Datum
Period_to_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PeriodSet *result = period_periodset(p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_to_period);
/**
 * Return the bounding period on which the period set value is defined
 */
PGDLLEXPORT Datum
Periodset_to_period(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  Period *result = palloc(sizeof(Period));
  periodset_bbox_slice(psdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_mem_size);
/**
 * Return the size in bytes of the period set value
 */
PGDLLEXPORT Datum
Periodset_mem_size(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Datum result = periodset_mem_size(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Periodset_timespan);
/**
 * Return the timespan of the period set value
 */
PGDLLEXPORT Datum
Periodset_timespan(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *result = periodset_timespan(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_duration);
/**
 * Return the timespan of the period set value
 */
PGDLLEXPORT Datum
Periodset_duration(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *result = periodset_duration(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_num_periods);
/**
 * Return the number of periods of the period set value
 */
PGDLLEXPORT Datum
Periodset_num_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int result = periodset_num_periods(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_start_period);
/**
 * Return the start period of the period set value
 */
PGDLLEXPORT Datum
Periodset_start_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *result = periodset_start_period(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_end_period);
/**
 * Return the end period of the period set value
 */
PGDLLEXPORT Datum
Periodset_end_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *result = periodset_end_period(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_period_n);
/**
 * Return the n-th period of the period set value
 */
PGDLLEXPORT Datum
Periodset_period_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  Period *result = periodset_period_n(ps, i);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_periods);
/**
 * Return the periods of the period set value
 */
PGDLLEXPORT Datum
Periodset_periods(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  const Period **periods = periodset_periods(ps);
  ArrayType *result = periodarr_to_array(periods, ps->count);
  pfree(periods);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Periodset_num_timestamps);
/**
 * Return the number of timestamps of the period set value
 */
PGDLLEXPORT Datum
Periodset_num_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int result = periodset_num_timestamps(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_start_timestamp);
/**
 * Return the start timestamp of the period set value
 */
PGDLLEXPORT Datum
Periodset_start_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz result = p->lower;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_end_timestamp);
/**
 * Return the end timestamp of the period set value
 */
PGDLLEXPORT Datum
Periodset_end_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz result = p->upper;
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_timestamp_n);
/**
 * Return the n-th timestamp of the period set value
 */
PGDLLEXPORT Datum
Periodset_timestamp_n(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = periodset_timestamp_n(ps, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Periodset_timestamps);
/**
 * Return the timestamps of the period set value
 */
PGDLLEXPORT Datum
Periodset_timestamps(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  int count;
  TimestampTz *times = periodset_timestamps(ps, &count);
  ArrayType *result = timestamparr_to_array(times, count);
  pfree(times);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Modifications functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_shift);
/**
 * Shift the period set value by the interval
 */
PGDLLEXPORT Datum
Periodset_shift(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  PeriodSet *result = periodset_shift_tscale(ps, start, NULL);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_tscale);
/**
 * Shift the period set value by the interval
 */
PGDLLEXPORT Datum
Periodset_tscale(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  PeriodSet *result = periodset_shift_tscale(ps, NULL, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_shift_tscale);
/**
 * Shift the period set value by the interval
 */
PGDLLEXPORT Datum
Periodset_shift_tscale(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  PeriodSet *result = periodset_shift_tscale(ps, start, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first period set value
 * is less than, equal, or greater than the second temporal value
 */
PGDLLEXPORT Datum
Periodset_cmp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  int cmp = periodset_cmp(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Periodset_eq);
/**
 * Return true if the first period set value is equal to the second one
 */
PGDLLEXPORT Datum
Periodset_eq(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_eq(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_ne);
/**
 * Return true if the first period set value is different from the second one
 */
PGDLLEXPORT Datum
Periodset_ne(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_ne(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(Periodset_lt);
/**
 * Return true if the first period set value is less than the second one
 */
PGDLLEXPORT Datum
Periodset_lt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_lt(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_le);
/**
 * Return true if the first period set value is less than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Periodset_le(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_le(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_ge);
/**
 * Return true if the first period set value is greater than or equal to
 * the second one
 */
PGDLLEXPORT Datum
Periodset_ge(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_ge(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Periodset_gt);
/**
 * Return true if the first period set value is greater than the second one
 */
PGDLLEXPORT Datum
Periodset_gt(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = periodset_gt(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_hash);
/**
 * Return the 32-bit hash value of a period set
 */
PGDLLEXPORT Datum
Periodset_hash(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  uint32 result = periodset_hash(ps);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Periodset_hash_extended);
/**
 * Return the 64-bit hash value of a period set using a seed
 */
PGDLLEXPORT Datum
Periodset_hash_extended(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Datum seed = PG_GETARG_DATUM(1);
  uint64 result = periodset_hash_extended(ps, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
