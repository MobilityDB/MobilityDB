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
 * @brief General functions for `timestampset` values composed of an ordered
 * list of distinct `timestamptz` values.
 */

#include "general/timestampset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/time_ops.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_accessor
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
    int cmp = timestamptz_cmp_internal(t, t1);
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
 * @ingroup libmeos_spantime_in_out
 * @brief Return a timestampt set from its Well-Known Text (WKT) representation.
 */
TimestampSet *
timestampset_in(const char *str)
{
  return timestampset_parse(&str);
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a timestamp set.
 */
char *
timestampset_out(const TimestampSet *ts)
{
  char **strings = palloc(sizeof(char *) * ts->count);
  size_t outlen = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    strings[i] = pg_timestamptz_out(t);
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
 * @pymeosfunc TimestampSet()
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

  /* Compute the bounding period */
  span_set(TimestampTzGetDatum(times[0]), TimestampTzGetDatum(times[count - 1]),
    true, true, T_TIMESTAMPTZ, &result->period);
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
 * @pymeosfunc TimestampSet()
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
 * @pymeosfunc timespan()
 */
Interval *
timestampset_timespan(const TimestampSet *ts)
{
  TimestampTz start = timestampset_time_n(ts, 0);
  TimestampTz end = timestampset_time_n(ts, ts->count - 1);
  Interval *result = pg_timestamp_mi(end, start);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of a timestamp set.
 * @sqlfunc period()
 * @sqlop @p ::
 * @pymeosfunc period()
 */
Period *
timestampset_to_period(const TimestampSet *ts)
{
  Period *result = palloc(sizeof(Period));
  memcpy(result, &ts->period, sizeof(Period));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the number of timestamps of a timestamp set.
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
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
 * @pymeosfunc startTimestamp()
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
 * @pymeosfunc endTimestamp()
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
 * @pymeosfunc timestampN()
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
 * @pymeosfunc timestamps()
 */
TimestampTz *
timestampset_timestamps(const TimestampSet *ts)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * ts->count);
  for (int i = 0; i < ts->count; i++)
    result[i] = timestampset_time_n(ts, i);
  return result;
}

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_transf
 * @brief Return a timestamp set shifted and/or scaled by the intervals
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shift()
 */
TimestampSet *
timestampset_shift_tscale(const TimestampSet *ts, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  TimestampSet *result = timestampset_copy(ts);

  /* Shift and/or scale the bounding period */
  period_shift_tscale(shift, duration, &result->period);

  /* Set the first instant */
  result->elems[0] = result->period.lower;
  if (ts->count > 1)
  {
    /* Shift and/or scale from the second to the penultimate instant */
    TimestampTz delta;
    if (shift != NULL)
      delta = result->period.lower - ts->period.lower;
    double scale;
    if (duration != NULL)
      scale = (double) (result->period.upper - result->period.lower) /
        (double) (ts->period.upper - ts->period.lower);
    for (int i = 1; i < ts->count - 1; i++)
    {
      if (shift != NULL)
        result->elems[i] += delta;
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
 * @pymeosfunc __eq__()
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
 * @sqlfunc timestampset_cmp()
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
    result = timestamptz_cmp_internal(t1, t2);
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
 * @sqlfunc timestampset_hash()
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
 * @sqlfunc timestampset_hash_extended()
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
