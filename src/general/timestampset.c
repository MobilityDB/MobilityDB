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
 * @file timestampset.c
 * @brief General functions for `timestampset` values composed of an ordered
 * list of distinct `timestamptz` values.
 */

#include "general/timestampset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/timestamp.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/pg_call.h"
#include "general/time_ops.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Return the n-th timestamp of a timestamp set
 * @pre The argument @p index is less than or equal to the number of timestamps
 * in the timestamp set
 */
TimestampTz
timestampset_time_n(const TimestampSet *ts, int index)
{
  return ts->elems[index];
}

/**
 * Return a pointer to the precomputed bounding box of a timestamp set
 */
const Period *
timestampset_period_ptr(const TimestampSet *ts)
{
  return (Period *)&ts->period;
}

/**
 * Return the location of the timestamp in a timestamp set
 * using binary search
 *
 * If the timestamp is found, the index of the timestamp is returned
 * in the output parameter. Otherwise, return a number encoding whether it
 * is before, between two timestamps, or after the timestamp set.
 * For example, given a value composed of 3 timestamps and a timestamp,
 * the result of the function is as follows:
 * @code
 *            0       1        2
 *            |       |        |
 * 1)    t^                            => loc = 0
 * 2)        t^                        => loc = 0
 * 3)            t^                    => loc = 1
 * 4)                    t^            => loc = 2
 * 5)                            t^    => loc = 3
 * @endcode
 *
 * @param[in] ts Timestamp set
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the timestamp set
 */
bool
timestampset_find_timestamp(const TimestampSet *ts, TimestampTz t, int *loc)
{
  int first = 0;
  int last = ts->count - 1;
  int middle = 0; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    TimestampTz t1 = timestampset_time_n(ts, middle);
    int cmp = timestamp_cmp_internal(t, t1);
    if (cmp == 0)
    {
      *loc = middle;
      return true;
    }
    if (cmp < 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (middle == ts->count)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions in string format
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a timestampt set from its string representation.
 */
TimestampSet *
timestampset_in(char *str)
{
  return timestampset_parse(&str);
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the string representation of a timestamp set.
 */
char *
timestampset_out(const TimestampSet *ts)
{
  char **strings = palloc(sizeof(char *) * ts->count);
  size_t outlen = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    strings[i] = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(t));
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ts->count, outlen, "", '{', '}');
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a timestamp set from an array of timestamps.
 *
 * For example, the memory structure of a timestamp set with 3
 * timestamps is as follows
 * @code
 * ---------------------------------------------------------------------------
 * ( TimestampSet )_X | ( bbox )_X | Timestamp_0 | Timestamp_1 | Timestamp_2 |
 * ---------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and bbox is the
 * bounding box which is a period.
 *
 * @param[in] times Array of timestamps
 * @param[in] count Number of elements in the array
 * @sqlfunc timestampset()
 */
TimestampSet *
timestampset_make(const TimestampTz *times, int count)
{
  /* Test the validity of the timestamps */
  for (int i = 0; i < count - 1; i++)
  {
    if (times[i] >= times[i + 1])
      elog(ERROR, "Invalid value for timestamp set");
  }
  /* Notice that the first timestamp is already declared in the struct */
  size_t memsize = double_pad(sizeof(TimestampSet)) +
    sizeof(TimestampTz) * (count - 1);
  /* Create the TimestampSet */
  TimestampSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = count;

  /* Compute the bounding box */
  span_set(times[0], times[count - 1], true, true, T_TIMESTAMPTZ,
    &result->period);
  /* Copy the timestamp array */
  for (int i = 0; i < count; i++)
    result->elems[i] = times[i];
  return result;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a timestamp set from the array of timestamps and free the
 * array after the creation.
 *
 * @param[in] times Array of timestamps
 * @param[in] count Number of elements in the array
 * @see timestampset_make
 * @sqlfunc timestampset()
 */
TimestampSet *
timestampset_make_free(TimestampTz *times, int count)
{
  if (count == 0)
  {
    pfree(times);
    return NULL;
  }
  TimestampSet *result = timestampset_make(times, count);
  pfree(times);
  return result;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Return a copy of a timestamp set.
 */
TimestampSet *
timestampset_copy(const TimestampSet *ts)
{
  TimestampSet *result = palloc(VARSIZE(ts));
  memcpy(result, ts, VARSIZE(ts));
  return result;
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a timestamp as a timestamp set
 * @sqlop @p ::
 */
TimestampSet *
timestamp_to_timestampset(TimestampTz t)
{
  TimestampSet *result = timestampset_make(&t, 1);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the size in bytes of a timestamp set.
 * @sqlfunc memSize()
 */
int
timestampset_mem_size(const TimestampSet *ts)
{
  return (int) VARSIZE(DatumGetPointer(ts));
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the timespan of a timestamp set.
 * @sqlfunc timespan()
 */
Interval *
timestampset_timespan(const TimestampSet *ts)
{
  TimestampTz start = timestampset_time_n(ts, 0);
  TimestampTz end = timestampset_time_n(ts, ts->count - 1);
  Interval *result = pg_timestamp_mi(end, start);
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Set a period to the bounding period of a timestamp set
 * @sqlfunc period()
 */
void
timestampset_set_period(const TimestampSet *ts, Period *p)
{
  const Period *p1 = &ts->period;
  span_set(p1->lower, p1->upper, p1->lower_inc, p1->upper_inc, T_TIMESTAMPTZ, p);
  return;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the number of timestamps of a timestamp set.
 * @sqlfunc numTimestamps()
 */
int
timestampset_num_timestamps(const TimestampSet *ts)
{
  return ts->count;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the start timestamp of a timestamp set.
 * @sqlfunc startTimestamp()
 */
TimestampTz
timestampset_start_timestamp(const TimestampSet *ts)
{
  TimestampTz result = timestampset_time_n(ts, 0);
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the end timestamp of a timestamp set.
 * @sqlfunc endTimestamp()
 */
TimestampTz
timestampset_end_timestamp(const TimestampSet *ts)
{
  TimestampTz result = timestampset_time_n(ts, ts->count - 1);
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the n-th timestamp of a timestamp set.
 *
 * @param[in] ts Timestamp set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 */
bool
timestampset_timestamp_n(const TimestampSet *ts, int n, TimestampTz *result)
{
  if (n < 1 || n > ts->count)
    return false;
  *result = timestampset_time_n(ts, n - 1);
  return true;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the array of timestamps of a timestamp set.
 * @sqlfunc timestamps()
 */
TimestampTz *
timestampset_timestamps(const TimestampSet *ts)
{
  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  for (int i = 0; i < ts->count; i++)
    times[i] = timestampset_time_n(ts, i);
  return times;
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_transf
 * @brief Return a timestamp set shifted and/or scaled by the intervals
 * @sqlfunc shiftTscale()
 */
TimestampSet *
timestampset_shift_tscale(const TimestampSet *ts, const Interval *start,
  const Interval *duration)
{
  assert(start != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  TimestampSet *result = timestampset_copy(ts);

  /* Shift and/or scale the bounding period */
  period_shift_tscale(start, duration, &result->period);

  /* Set the first instant */
  result->elems[0] = result->period.lower;
  if (ts->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz shift;
    if (start != NULL)
      shift = result->period.lower - ts->period.lower;
    double scale;
    if (duration != NULL)
      scale =
        (double) (result->period.upper - result->period.lower) /
        (double) (ts->period.upper - ts->period.lower);
    for (int i = 1; i < ts->count - 1; i++)
    {
      if (start != NULL)
        result->elems[i] += shift;
      if (duration != NULL)
        result->elems[i] = result->period.lower +
          (result->elems[i] - result->period.lower) * scale;
    }
    /* Set the last instant */
    result->elems[ts->count - 1] = result->period.upper;
  }
  return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 */
bool
timestampset_eq(const TimestampSet *ts1, const TimestampSet *ts2)
{
  if (ts1->count != ts2->count)
    return false;
  /* ts1 and ts2 have the same number of TimestampSet */
  for (int i = 0; i < ts1->count; i++)
  {
    TimestampTz t1 = timestampset_time_n(ts1, i);
    TimestampTz t2 = timestampset_time_n(ts2, i);
    if (t1 != t2)
      return false;
  }
  /* All timestamps of the two timestamp set are equal */
  return true;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is different from the
 * second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p <>
 */
bool
timestampset_ne(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return ! timestampset_eq(ts1, ts2);
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first timestamp set
 * value is less than, equal, or greater than the second temporal value.
 * @note Function used for B-tree comparison
 */
int
timestampset_cmp(const TimestampSet *ts1, const TimestampSet *ts2)
{
  int count = Min(ts1->count, ts2->count);
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    TimestampTz t1 = timestampset_time_n(ts1, i);
    TimestampTz t2 = timestampset_time_n(ts2, i);
    result = timestamp_cmp_internal(t1, t2);
    if (result)
      break;
  }
  /* The first count times of the two TimestampSet are equal */
  if (! result)
  {
    if (count < ts1->count) /* ts1 has more timestamps than ts2 */
      result = 1;
    else if (count < ts2->count) /* ts2 has more timestamps than ts1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is less than the second one
 * @sqlop @p <
 */
bool
timestampset_lt(const TimestampSet *ts1, const TimestampSet *ts2)
{
  int cmp = timestampset_cmp(ts1, ts2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is less than
 * or equal to the second one
 * @sqlop @p <=
 */
bool
timestampset_le(const TimestampSet *ts1, const TimestampSet *ts2)
{
  int cmp = timestampset_cmp(ts1, ts2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is greater than
 * or equal to the second one
 * @sqlop @p >=
 */
bool
timestampset_ge(const TimestampSet *ts1, const TimestampSet *ts2)
{
  int cmp = timestampset_cmp(ts1, ts2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first timestamp set is greater than the second one
 * @sqlop @p >
 */
bool
timestampset_gt(const TimestampSet *ts1, const TimestampSet *ts2)
{
  int cmp = timestampset_cmp(ts1, ts2);
  return cmp > 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 32-bit hash value of a timestamp set.
 */
uint32
timestampset_hash(const TimestampSet *ts)
{
  uint32 result = 1;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    uint32 time_hash = pg_hashint8(t);
    result = (result << 5) - result + time_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 64-bit hash value of a timestamp set using a seed.
 */
uint64
timestampset_hash_extended(const TimestampSet *ts, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    uint64 time_hash = pg_hashint8extended(t, seed);
    result = (result << 5) - result + time_hash;
  }
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_in);
/**
 * Input function for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  TimestampSet *result = timestampset_in(input);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_out);
/**
 * Output function for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_out(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  char *result = timestampset_out(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Timestampset_recv);
/**
 * Receive function for timestamp set
 */
PGDLLEXPORT Datum
Timestampset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  TimestampSet *result = timestampset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_send);
/**
 * Send function for timestamp set
 */
PGDLLEXPORT Datum
Timestampset_send(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(ts);
  uint8_t *wkb = timestampset_as_wkb(ts, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_constructor);
/**
 * Construct a timestamp set from an array of timestamps
 */
PGDLLEXPORT Datum
Timestampset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  TimestampTz *times = timestamparr_extract(array, &count);
  TimestampSet *result = timestampset_make_free(times, count);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_to_timestampset);
/**
 * Cast a timestamp as a timestamp set
 */
PGDLLEXPORT Datum
Timestamp_to_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *result = timestamp_to_timestampset(t);
  PG_RETURN_POINTER(result);
}

/**
 * Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
timestampset_period_slice(Datum tsdatum, Period *p)
{
  TimestampSet *ts = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) tsdatum))
    ts = (TimestampSet *) PG_DETOAST_DATUM_SLICE(tsdatum, 0,
      time_max_header_size());
  else
    ts = (TimestampSet *) tsdatum;
  timestampset_set_period(ts, p);
  PG_FREE_IF_COPY_P(ts, DatumGetPointer(tsdatum));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_to_period);
/**
 * Return the bounding period on which a timestamp set is defined
 */
PGDLLEXPORT Datum
Timestampset_to_period(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  Period *result = palloc(sizeof(Period));
  timestampset_period_slice(tsdatum, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_mem_size);
/**
 * Return the size in bytes of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_mem_size(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Datum result = timestampset_mem_size(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timespan);
/**
 * Return the timespan of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timespan(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *result = timestampset_timespan(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_num_timestamps);
/**
 * Return the number of timestamps of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_num_timestamps(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_INT32(timestampset_num_timestamps(ts));
}

PG_FUNCTION_INFO_V1(Timestampset_start_timestamp);
/**
 * Return the start timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_start_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = timestampset_start_timestamp(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_end_timestamp);
/**
 * Return the end timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_end_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = timestampset_end_timestamp(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timestamp_n);
/**
 * Return the n-th timestamp of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timestamp_n(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = timestampset_timestamp_n(ts, n, &result);
  PG_FREE_IF_COPY(ts, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timestamps);
/**
 * Return the timestamps of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_timestamps(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz *times = timestampset_timestamps(ts);
  ArrayType *result = timestamparr_to_array(times, ts->count);
  pfree(times);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_shift);
/**
 * Shift a timestamp set by an interval
 */
PGDLLEXPORT Datum
Timestampset_shift(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  TimestampSet *result = timestampset_shift_tscale(ts, start, NULL);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_tscale);
/**
 * Scale a timestamp set by an interval
 */
PGDLLEXPORT Datum
Timestampset_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, NULL, duration);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_shift_tscale);
/**
 * Shift and scale a timestamp set by the intervals
 */
PGDLLEXPORT Datum
Timestampset_shift_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, start, duration);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first timestamp set
 * is less than, equal, or greater than the second temporal value
 */
PGDLLEXPORT Datum
Timestampset_cmp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  int cmp = timestampset_cmp(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Timestampset_eq);
/**
 * Return true if the first timestamp set is equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_eq(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_eq(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_ne);
/**
 * Return true if the first timestamp set is different from the second one
 */
PGDLLEXPORT Datum
Timestampset_ne(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_ne(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_lt);
/**
 * Return true if the first timestamp set is less than the second one
 */
PGDLLEXPORT Datum
Timestampset_lt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_lt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_le);
/**
 * Return true if the first timestamp set is less than
 * or equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_le(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_le(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_ge);
/**
 * Return true if the first timestamp set is greater than
 * or equal to the second one
 */
PGDLLEXPORT Datum
Timestampset_ge(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_ge(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Timestampset_gt);
/**
 * Return true if the first timestamp set is greater than the second one
 */
PGDLLEXPORT Datum
Timestampset_gt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = timestampset_gt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_hash);
/**
 * Return the 32-bit hash value of a timestamp set
 */
PGDLLEXPORT Datum
Timestampset_hash(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint32 result = timestampset_hash(ts);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Timestampset_hash_extended);
/**
 * Return the 64-bit hash value of a timestamp set using a seed
 */
PGDLLEXPORT Datum
Timestampset_hash_extended(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = timestampset_hash_extended(ts, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
