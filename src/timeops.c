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
 * @file timeops.c
 * Operators for time types.
 */

#include "timeops.h"

#include <assert.h>
#include <utils/timestamp.h>

#include "period.h"
#include "periodset.h"
#include "timestampset.h"
#include "tempcache.h"
#include "temporal_util.h"

typedef enum
{
  UNION,
  INTER,
  MINUS
} SetOper;

/*****************************************************************************/

/**
 * Ensure that the Oid corresponds to a time type
 */
void
ensure_time_type_oid(Oid timetypid)
{
  if (timetypid != type_oid(T_TIMESTAMPTZ) &&
    timetypid != type_oid(T_TIMESTAMPSET) &&
    timetypid != type_oid(T_PERIOD) &&
    timetypid != type_oid(T_PERIODSET))
    elog(ERROR, "unknown time type: %d", timetypid);
}

/**
 * Determine the relative position of the two timestamps
 */
RelativeTimePos
pos_timestamp_timestamp(TimestampTz t1, TimestampTz t2)
{
  int32 cmp = timestamp_cmp_internal(t1, t2);
  if (cmp > 0)
    return BEFORE;
  if (cmp < 0)
    return AFTER;
  return DURING;
}

/**
 * Determine the relative position of the period and the timestamp
 */
RelativeTimePos
pos_period_timestamp(const Period *p, TimestampTz t)
{
  int32 cmp = timestamp_cmp_internal(p->lower, t);
  if (cmp > 0)
    return BEFORE;
  if (cmp == 0 && !(p->lower_inc))
    return BEFORE;
  cmp = timestamp_cmp_internal(p->upper, t);
  if (cmp < 0)
    return AFTER;
  if (cmp == 0 && !(p->upper_inc))
    return AFTER;
  return DURING;
}

/*****************************************************************************
 * Generic operations
 *****************************************************************************/

/**
 * Returns the union, intersection or difference of the two time values
 */
TimestampSet *
setop_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2, SetOper setop)
{
  if (setop == INTER || setop == MINUS)
  {
    /* Bounding box test */
    const Period *p1 = timestampset_bbox_ptr(ts1);
    const Period *p2 = timestampset_bbox_ptr(ts2);
    if (!overlaps_period_period_internal(p1, p2))
      return setop == INTER ? NULL : timestampset_copy(ts1);
  }

  int count;
  if (setop == UNION)
    count = ts1->count + ts2->count;
  else if (setop == INTER)
    count = Min(ts1->count, ts2->count);
  else /* setop == MINUS */
    count = ts1->count;
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);
  int i = 0, j = 0, k = 0;
  TimestampTz t1 = timestampset_time_n(ts1, 0);
  TimestampTz t2 = timestampset_time_n(ts2, 0);
  while (i < ts1->count && j < ts2->count)
  {
    int cmp = timestamp_cmp_internal(t1, t2);
    if (cmp == 0)
    {
      if (setop == UNION || setop == INTER)
        times[k++] = t1;
      i++; j++;
      if (i == ts1->count || j == ts2->count)
        break;
      t1 = timestampset_time_n(ts1, i);
      t2 = timestampset_time_n(ts2, j);
    }
    else if (cmp < 0)
    {
      if (setop == UNION || setop == MINUS)
        times[k++] = t1;
      i++;
      if (i == ts1->count)
        break;
      else
        t1 = timestampset_time_n(ts1, i);
    }
    else
    {
      if (setop == UNION || setop == MINUS)
        times[k++] = t2;
      j++;
      if (j == ts2->count)
        break;
      else
        t2 = timestampset_time_n(ts2, j);
    }
  }
  if (setop == UNION)
  {
    while (i < ts1->count)
      times[k++] = timestampset_time_n(ts1, i++);
    while (j < ts2->count)
      times[k++] = timestampset_time_n(ts2, j++);
  }
  return timestampset_make_free(times, k);
}

/**
 * Returns the intersection or the difference of the two time values
 */
TimestampSet *
setop_timestampset_period(const TimestampSet *ts, const Period *p,
  SetOper setop)
{
  assert(setop == INTER || setop == MINUS);
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period_internal(p1, p))
    return (setop == INTER) ? NULL : timestampset_copy(ts);

  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    if (((setop == INTER) && contains_period_timestamp_internal(p, t)) ||
      ((setop == MINUS) && !contains_period_timestamp_internal(p, t)))
      times[k++] = t;
  }
  return timestampset_make_free(times, k);
}

/*
 * Returns the intersection or the difference of the two time values
 */
TimestampSet *
setop_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps,
  SetOper setop)
{
  assert(setop == INTER || setop == MINUS);
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts);
  const Period *p2 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period_internal(p1, p2))
    return (setop == INTER) ? NULL : timestampset_copy(ts);

  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  TimestampTz t = timestampset_time_n(ts, 0);
  const Period *p = periodset_per_n(ps, 0);
  int i = 0, j = 0, k = 0;
  while (i < ts->count && j < ps->count)
  {
    if (t < p->lower)
    {
      if (setop == MINUS)
        times[k++] = t;
      i++;
      if (i == ts->count)
        break;
      else
        t = timestampset_time_n(ts, i);
    }
    else if (t > p->upper)
    {
      j++;
      if (j == ps->count)
        break;
      else
        p = periodset_per_n(ps, j);
    }
    else
    {
      if ((setop == INTER && contains_period_timestamp_internal(p, t)) ||
        (setop == MINUS && !contains_period_timestamp_internal(p, t)))
        times[k++] = t;
      i++;
      if (i == ts->count)
        break;
      else
        t = timestampset_time_n(ts, i);
    }
  }
  if (setop == MINUS)
  {
    for (int l = i; l < ts->count; l++)
      times[k++] = timestampset_time_n(ts, l);
  }
  return timestampset_make_free(times, k);
}

/*****************************************************************************/
/* contains? */

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = timestampset_bbox_ptr(ts);
  if (!contains_period_timestamp_internal(p, t))
    return false;

  int loc;
  return timestampset_find_timestamp(ts, t, &loc);
}

PG_FUNCTION_INFO_V1(contains_timestampset_timestamp);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = contains_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts1);
  const Period *p2 = timestampset_bbox_ptr(ts2);
  if (!contains_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (j < ts2->count)
  {
    TimestampTz t1 = timestampset_time_n(ts1, i);
    TimestampTz t2 = timestampset_time_n(ts2, j);
    int cmp = timestamp_cmp_internal(t1, t2);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      return false;
  }
  return true;
}

PG_FUNCTION_INFO_V1(contains_timestampset_timestampset);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = contains_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_period_timestamp_internal(const Period *p, TimestampTz t)
{
  int cmp = timestamp_cmp_internal(p->lower, t);
  if (cmp > 0 || (cmp == 0 && ! p->lower_inc))
    return false;

  cmp = timestamp_cmp_internal(p->upper, t);
  if (cmp < 0 || (cmp == 0 && ! p->upper_inc))
    return false;

  return true;
}

PG_FUNCTION_INFO_V1(contains_period_timestamp);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(contains_period_timestamp_internal(p, t));
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  /* It is sufficient to do a bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts);
  if (!contains_period_period_internal(p, p1))
    return false;
  return true;
}

PG_FUNCTION_INFO_V1(contains_period_timestampset);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = contains_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_period_period_internal(const Period *p1, const Period *p2)
{
  int c1 = timestamp_cmp_internal(p1->lower, p2->lower);
  int c2 = timestamp_cmp_internal(p1->upper, p2->upper);
  if (
    (c1 < 0 || (c1 == 0 && (p1->lower_inc || ! p2->lower_inc))) &&
    (c2 > 0 || (c2 == 0 && (p1->upper_inc || ! p2->upper_inc)))
  )
    return true;
  return false;
}

PG_FUNCTION_INFO_V1(contains_period_period);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(contains_period_period_internal(p1, p2));
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_periodset_timestamp_internal(const PeriodSet *ps, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = periodset_bbox_ptr(ps);
  if (!contains_period_timestamp_internal(p, t))
    return false;

  int loc;
  if (!periodset_find_timestamp(ps, t, &loc))
    return false;
  return true;
}

PG_FUNCTION_INFO_V1(contains_periodset_timestamp);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = contains_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_periodset_timestampset_internal(const PeriodSet *ps, const TimestampSet *ts)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  const Period *p2 = timestampset_bbox_ptr(ts);
  if (!contains_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (j < ts->count)
  {
    const Period *p = periodset_per_n(ps, i);
    TimestampTz t = timestampset_time_n(ts, j);
    if (contains_period_timestamp_internal(p, t))
      j++;
    else
    {
      if (t > p->upper)
        i++;
      else
        return false;
    }
  }
  return true;
}

PG_FUNCTION_INFO_V1(contains_periodset_timestampset);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = contains_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  if (!contains_period_period_internal(p1, p))
    return false;

  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  p1 = periodset_per_n(ps, loc);
  return contains_period_period_internal(p1, p);
}

PG_FUNCTION_INFO_V1(contains_periodset_period);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = contains_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_bbox_ptr(ps);
  return contains_period_period_internal(p, p1);
}

PG_FUNCTION_INFO_V1(contains_period_periodset);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = contains_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value contains the second one (internal function)
 */
bool
contains_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps1);
  const Period *p2 = periodset_bbox_ptr(ps2);
  if (!contains_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    if (before_period_period_internal(p1, p2))
      i++;
    else if (before_period_period_internal(p2, p1))
      return false;
    else
    {
      /* p1 and p2 overlap */
      if (contains_period_period_internal(p1, p2))
      {
        if (p1->upper == p2->upper)
        {
          i++; j++;
        }
        else
          j++;
      }
      else
        return false;
    }
  }
  /* if j == ps2->count every period in p2 is contained in a period of p1
     but p1 may have additional periods */
  return (j == ps2->count);
}

PG_FUNCTION_INFO_V1(contains_periodset_periodset);
/**
 * Returns true if the first time value contains the second one
 */
PGDLLEXPORT Datum
contains_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = contains_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(contained_timestamp_timestampset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = contains_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_timestamp_period);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(contains_period_timestamp_internal(p, t));
}

PG_FUNCTION_INFO_V1(contained_timestamp_periodset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = contains_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_timestampset_timestampset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = contains_timestampset_timestampset_internal(ts2, ts1);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_timestampset_period);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = contains_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_timestampset_periodset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = contains_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is contained by the second one
 * (internal function)
 */
bool
contained_period_period_internal(const Period *p1, const Period *p2)
{
  return contains_period_period_internal(p2, p1);
}

PG_FUNCTION_INFO_V1(contained_period_period);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(contains_period_period_internal(p2, p1));
}

PG_FUNCTION_INFO_V1(contained_period_periodset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = contains_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_periodset_period);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = contains_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_periodset_periodset);
/**
 * Returns true if the first time value is contained by the second one
 */
PGDLLEXPORT Datum
contained_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = contains_periodset_periodset_internal(ps2, ps1);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts1);
  const Period *p2 = timestampset_bbox_ptr(ts2);
  if (!overlaps_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ts1->count && j < ts2->count)
  {
    TimestampTz t1 = timestampset_time_n(ts1, i);
    TimestampTz t2 = timestampset_time_n(ts2, j);
    int cmp = timestamp_cmp_internal(t1, t2);
    if (cmp == 0)
      return true;
    if (cmp < 0)
      i++;
    else
      j++;
  }
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_timestampset_timestampset);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = overlaps_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period_internal(p, p1))
    return false;

  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    if (contains_period_timestamp_internal(p, t))
      return true;
  }
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_timestampset_period);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overlaps_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  const Period *p2 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ts->count && j < ps->count)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    const Period *p = periodset_per_n(ps, j);
    if (contains_period_timestamp_internal(p, t))
      return true;
    else if (t > p->upper)
      j++;
    else
      i++;
  }
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_timestampset_periodset);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overlaps_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_period_timestampset);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overlaps_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_period_period_internal(const Period *p1, const Period *p2)
{
  int c1 = timestamp_cmp_internal(p1->lower, p2->upper);
  int c2 = timestamp_cmp_internal(p2->lower, p1->upper);
  if (
    (c1 < 0 || (c1 == 0 && p1->lower_inc && p2->upper_inc)) &&
    (c2 < 0 || (c2 == 0 && p2->lower_inc && p1->upper_inc))
  )
    return true;
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_period_period);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(overlaps_period_period_internal(p1, p2));
}

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period_internal(p, p1))
    return false;

  /* Binary search of lower bound of period */
  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  for (int i = loc; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    if (overlaps_period_period_internal(p1, p))
      return true;
    if (p->upper < p1->upper)
      break;
  }
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_period_periodset);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overlaps_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_periodset_timestampset);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overlaps_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_periodset_period);
/**
 * Returns true if the two time values overlap
 */
PGDLLEXPORT Datum
overlaps_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overlaps_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time values overlap (internal function)
 */
bool
overlaps_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps1);
  const Period *p2 = periodset_bbox_ptr(ps2);
  if (!overlaps_period_period_internal(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    if (overlaps_period_period_internal(p1, p2))
      return true;
    int cmp = timestamp_cmp_internal(p1->upper, p2->upper);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return false;
}

PG_FUNCTION_INFO_V1(overlaps_periodset_periodset);
/**
 * Returns true if the two time values overlap (internal function)
 */
PGDLLEXPORT Datum
overlaps_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = overlaps_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly before of? */

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestamp_timestampset_internal(const TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t < t1);
}

PG_FUNCTION_INFO_V1(before_timestamp_timestampset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = before_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestamp_period_internal(const TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->lower);
  return (cmp < 0 || (cmp == 0 && ! p->lower_inc));
}

PG_FUNCTION_INFO_V1(before_timestamp_period);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(before_timestamp_period_internal(t, p));
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestamp_periodset_internal(const TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return before_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(before_timestamp_periodset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = before_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t1 < t);
}

PG_FUNCTION_INFO_V1(before_timestampset_timestamp);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = before_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, ts1->count - 1);
  TimestampTz t2 = timestampset_time_n(ts2, 0);
  return (t1 < t2);
}

PG_FUNCTION_INFO_V1(before_timestampset_timestampset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = before_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return before_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(before_timestampset_period);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = before_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return before_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(before_timestampset_periodset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = before_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_period_timestamp_internal(const Period *p, TimestampTz t)
{

  int cmp = timestamp_cmp_internal(p->upper, t);
  return (cmp < 0 || (cmp == 0 && ! p->upper_inc));
}

PG_FUNCTION_INFO_V1(before_period_timestamp);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(before_period_timestamp_internal(p, t));
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return before_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(before_period_timestampset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = before_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_period_period_internal(const Period *p1, const Period *p2)
{
  int cmp = timestamp_cmp_internal(p1->upper, p2->lower);
  return (cmp < 0 || (cmp == 0 && (! p1->upper_inc || ! p2->lower_inc)));
}

PG_FUNCTION_INFO_V1(before_period_period);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(before_period_period_internal(p1, p2));
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return before_period_period_internal(p, p1);
}

PG_FUNCTION_INFO_V1(before_period_periodset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = before_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_periodset_timestamp_internal(const PeriodSet *ps, const TimestampTz t)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return before_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(before_periodset_timestamp);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = before_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_periodset_timestampset_internal(const PeriodSet *ps, const TimestampSet *ts)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz t = timestampset_time_n(ts, 0);
  return before_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(before_periodset_timestampset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = before_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return before_period_period_internal(p1, p);
}

PG_FUNCTION_INFO_V1(before_periodset_period);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = before_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly before the second one
 * (internal function)
 */
bool
before_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, ps1->count - 1);
  const Period *p2 = periodset_per_n(ps2, 0);
  return before_period_period_internal(p1, p2);
}

PG_FUNCTION_INFO_V1(before_periodset_periodset);
/**
 * Returns true if the first time value is strictly before the second one
 */
PGDLLEXPORT Datum
before_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = before_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly after of? */

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestamp_timestampset_internal(const TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t > t1);
}

PG_FUNCTION_INFO_V1(after_timestamp_timestampset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = after_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestamp_period_internal(const TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->upper);
  return (cmp > 0 || (cmp == 0 && ! p->upper_inc));
}

PG_FUNCTION_INFO_V1(after_timestamp_period);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(after_timestamp_period_internal(t, p));
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestamp_periodset_internal(const TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return after_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(after_timestamp_periodset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = after_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t1 > t);
}

PG_FUNCTION_INFO_V1(after_timestampset_timestamp);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = after_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, 0);
  TimestampTz t2 = timestampset_time_n(ts2, ts2->count - 1);
  return (t1 > t2);
}

PG_FUNCTION_INFO_V1(after_timestampset_timestampset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = after_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return after_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(after_timestampset_period);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = after_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz t = timestampset_time_n(ts, 0);
  return after_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(after_timestampset_periodset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = after_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_period_timestamp_internal(const Period *p, TimestampTz t)
{
  int cmp = timestamp_cmp_internal(t, p->lower);
  return (cmp < 0 || (cmp == 0 && ! p->lower_inc));
}

PG_FUNCTION_INFO_V1(after_period_timestamp);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(after_period_timestamp_internal(p, t));
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return after_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(after_period_timestampset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = after_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_period_period_internal(const Period *p1, const Period *p2)
{
  int cmp = timestamp_cmp_internal(p2->upper, p1->lower);
  return (cmp < 0 || (cmp == 0 && (! p2->upper_inc || ! p1->lower_inc)));
}

PG_FUNCTION_INFO_V1(after_period_period);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(after_period_period_internal(p1, p2));
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return after_period_period_internal(p, p1);
}

PG_FUNCTION_INFO_V1(after_period_periodset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = after_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_periodset_timestamp_internal(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, 0);
  return after_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(after_periodset_timestamp);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = after_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_periodset_timestampset_internal(const PeriodSet *ps, const TimestampSet *ts)
{
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return after_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(after_periodset_timestampset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = after_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return after_period_period_internal(p1, p);
}

PG_FUNCTION_INFO_V1(after_periodset_period);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = after_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is strictly after the second one
 * (internal function)
 */
bool
after_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, 0);
  const Period *p2 = periodset_per_n(ps2, ps2->count - 1);
  return after_period_period_internal(p1, p2);
}

PG_FUNCTION_INFO_V1(after_periodset_periodset);
/**
 * Returns true if the first time value is strictly after the second one
 */
PGDLLEXPORT Datum
after_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = after_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestamp_timestampset_internal(const TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t <= t1);
}

PG_FUNCTION_INFO_V1(overbefore_timestamp_timestampset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overbefore_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestamp_period_internal(const TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->upper);
  return (cmp < 0 || (cmp == 0 && p->upper_inc));
}

PG_FUNCTION_INFO_V1(overbefore_timestamp_period);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(overbefore_timestamp_period_internal(t, p));
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestamp_periodset_internal(const TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return overbefore_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(overbefore_timestamp_periodset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overbefore_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t1 <= t);
}

PG_FUNCTION_INFO_V1(overbefore_timestampset_timestamp);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overbefore_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, ts1->count - 1);
  TimestampTz t2 = timestampset_time_n(ts2, ts2->count - 1);
  return (t1 <= t2);
}

PG_FUNCTION_INFO_V1(overbefore_timestampset_timestampset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = overbefore_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return (overbefore_timestamp_period_internal(t, p));
}

PG_FUNCTION_INFO_V1(overbefore_timestampset_period);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overbefore_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return (!after_timestamp_period_internal(t, p));
}

PG_FUNCTION_INFO_V1(overbefore_timestampset_periodset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overbefore_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_period_timestamp_internal(const Period *p, TimestampTz t)
{
  return p->upper <= t;
}

PG_FUNCTION_INFO_V1(overbefore_period_timestamp);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overbefore_period_timestamp_internal(p, t));
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return (overbefore_period_timestamp_internal(p, t));
}

PG_FUNCTION_INFO_V1(overbefore_period_timestampset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overbefore_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_period_period_internal(const Period *p1, const Period *p2)
{
  int cmp = timestamp_cmp_internal(p1->upper, p2->upper);
  return (cmp < 0 || (cmp == 0 && (! p1->upper_inc || p2->upper_inc)));
}

PG_FUNCTION_INFO_V1(overbefore_period_period);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(overbefore_period_period_internal(p1, p2));
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return overbefore_period_period_internal(p, p1);
}

PG_FUNCTION_INFO_V1(overbefore_period_periodset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overbefore_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_periodset_timestamp_internal(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return overbefore_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(overbefore_periodset_timestamp);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overbefore_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_periodset_timestampset_internal(const PeriodSet *ps, const TimestampSet *ts)
{
  TimestampTz t1 = periodset_end_timestamp_internal(ps);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  return (t1 <= t2);
}

PG_FUNCTION_INFO_V1(overbefore_periodset_timestampset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overbefore_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return overbefore_period_period_internal(p1, p);
}

PG_FUNCTION_INFO_V1(overbefore_periodset_period);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overbefore_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not after the second one
 * (internal function)
 */
bool
overbefore_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, ps1->count - 1);
  const Period *p2 = periodset_per_n(ps2, ps2->count - 1);
  return overbefore_period_period_internal(p1, p2);
}

PG_FUNCTION_INFO_V1(overbefore_periodset_periodset);
/**
 * Returns true if the first time value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = overbefore_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestamp_timestampset_internal(const TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t >= t1);
}

PG_FUNCTION_INFO_V1(overafter_timestamp_timestampset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overafter_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestamp_period_internal(const TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(p->lower, t);
  return (cmp < 0 || (cmp == 0 && p->lower_inc));
}

PG_FUNCTION_INFO_V1(overafter_timestamp_period);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(overafter_timestamp_period_internal(t, p));
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestamp_periodset_internal(const TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return overafter_timestamp_period_internal(t, p);
}

PG_FUNCTION_INFO_V1(overafter_timestamp_periodset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overafter_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t1 >= t);
}

PG_FUNCTION_INFO_V1(overafter_timestampset_timestamp);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overafter_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, 0);
  TimestampTz t2 = timestampset_time_n(ts2, 0);
  return (t1 >= t2);
}

PG_FUNCTION_INFO_V1(overafter_timestampset_timestampset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  bool result = overafter_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return (overafter_timestamp_period_internal(t, p));
}

PG_FUNCTION_INFO_V1(overafter_timestampset_period);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overafter_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  const Period *p = periodset_per_n(ps, 0);
  return (overafter_timestamp_period_internal(t, p));
}

PG_FUNCTION_INFO_V1(overafter_timestampset_periodset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overafter_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_period_timestamp_internal(const Period *p, TimestampTz t)
{
  return (t <= p->lower);
}

PG_FUNCTION_INFO_V1(overafter_period_timestamp);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overafter_period_timestamp_internal(p, t));
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return (overafter_period_timestamp_internal(p, t));
}

PG_FUNCTION_INFO_V1(overafter_period_timestampset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overafter_period_timestampset_internal(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_period_period_internal(const Period *p1, const Period *p2)
{
  int cmp = timestamp_cmp_internal(p2->lower, p1->lower);
  return (cmp < 0 || (cmp == 0 && (! p1->lower_inc || p2->lower_inc)));
}

PG_FUNCTION_INFO_V1(overafter_period_period);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(overafter_period_period_internal(p1, p2));
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return overafter_period_period_internal(p, p1);
}

PG_FUNCTION_INFO_V1(overafter_period_periodset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = overafter_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_periodset_timestamp_internal(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, 0);
  return overafter_period_timestamp_internal(p, t);
}

PG_FUNCTION_INFO_V1(overafter_periodset_timestamp);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overafter_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_periodset_timestampset_internal(const PeriodSet *ps, const TimestampSet *ts)
{
  TimestampTz t1 = periodset_start_timestamp_internal(ps);
  TimestampTz t2 = timestampset_time_n(ts, 0);
  return (t1 >= t2);
}

PG_FUNCTION_INFO_V1(overafter_periodset_timestampset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = overafter_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return overafter_period_period_internal(p1, p);
}

PG_FUNCTION_INFO_V1(overafter_periodset_period);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = overafter_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first time value is not before the second one
 * (internal function)
 */
bool
overafter_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, 0);
  const Period *p2 = periodset_per_n(ps2, 0);
  return overafter_period_period_internal(p1, p2);
}

PG_FUNCTION_INFO_V1(overafter_periodset_periodset);
/**
 * Returns true if the first time value is not before the second one
 */
PGDLLEXPORT Datum
overafter_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = overafter_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_timestamp_period_internal(const TimestampTz t, const Period *p)
{
  /*
   * A timestamp A and a period C..D are adjacent if and only if
   * A is adjacent to C, or D is adjacent to A.
   */
  return (t == p->lower && ! p->lower_inc) ||
    (p->upper == t && ! p->upper_inc);
}

PG_FUNCTION_INFO_V1(adjacent_timestamp_period);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(adjacent_timestamp_period_internal(t, p));
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_timestamp_periodset_internal(const TimestampTz t, const PeriodSet *ps)
{
  /*
   * Two periods A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  return (t == p1->lower && ! p1->lower_inc) ||
       (p2->upper == t && ! p2->upper_inc);
}

PG_FUNCTION_INFO_V1(adjacent_timestamp_periodset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = adjacent_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  /*
   * A periods A..B and a timestamptz C are adjacent if and only if
   * B is adjacent to C, or C is adjacent to A.
   */
  TimestampTz t1 = timestampset_time_n(ts, 0);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  return (t2 == p->lower && ! p->lower_inc) ||
       (p->upper == t1 && ! p->upper_inc);
}

PG_FUNCTION_INFO_V1(adjacent_timestampset_period);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = adjacent_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  /*
   * A periods A..B and a timestamptz C are adjacent if and only if
   * B is adjacent to C, or C is adjacent to A.
   */
  TimestampTz t1 = timestampset_time_n(ts, 0);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  return (t2 == p1->lower && ! p1->lower_inc) ||
       (p2->upper == t1 && ! p2->upper_inc);
}

PG_FUNCTION_INFO_V1(adjacent_timestampset_periodset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = adjacent_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_period_timestamp);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(adjacent_timestamp_period_internal(t, p));
}

PG_FUNCTION_INFO_V1(adjacent_period_timestampset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = adjacent_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_period_period_internal(const Period *p1, const Period *p2)
{
  /*
   * Two periods A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (p1->upper == p2->lower && p1->upper_inc != p2->lower_inc) ||
       (p2->upper == p1->lower && p2->upper_inc != p1->lower_inc);
}

PG_FUNCTION_INFO_V1(adjacent_period_period);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_BOOL(adjacent_period_period_internal(p1, p2));
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  /*
   * Two periods A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (p2->upper == p->lower && p2->upper_inc != p->lower_inc) ||
       (p->upper == p1->lower && p->upper_inc != p1->lower_inc);
}

PG_FUNCTION_INFO_V1(adjacent_period_periodset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool result = adjacent_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_periodset_timestamp);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = adjacent_timestamp_periodset_internal(t, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_periodset_timestampset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool result = adjacent_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_periodset_period);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool result = adjacent_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

/**
 * Returns true if the two time value are adjacent (internal function)
 */
bool
adjacent_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *startps1 = periodset_per_n(ps1, 0);
  const Period *endps1 = periodset_per_n(ps1, ps1->count - 1);
  const Period *startps2 = periodset_per_n(ps2, 0);
  const Period *endps2 = periodset_per_n(ps2, ps2->count - 1);
  /*
   * Two periods A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  return (endps1->upper == startps2->lower && endps1->upper_inc != startps2->lower_inc) ||
    (endps2->upper == startps1 ->lower && endps2->upper_inc != startps1 ->lower_inc);
}

PG_FUNCTION_INFO_V1(adjacent_periodset_periodset);
/**
 * Returns true if the two time value are adjacent
 */
PGDLLEXPORT Datum
adjacent_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  bool result = adjacent_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(union_timestamp_timestamp);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result;
  int cmp = timestamp_cmp_internal(t1, t2);
  if (cmp == 0)
    result = timestampset_make(&t1, 1);
  else
  {
    TimestampTz times[2];
    if (cmp < 0)
    {
      times[0] = t1;
      times[1] = t2;
    }
    else
    {
      times[0] = t2;
      times[1] = t1;
    }
    result = timestampset_make(times, 2);
  }
  PG_RETURN_POINTER(result);
}

/**
 * Returns the union of the two time values (internal function)
 */
TimestampSet *
union_timestamp_timestampset_internal(const TimestampTz t, const TimestampSet *ts)
{
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ts->count + 1));
  int k = 0;
  bool found = false;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t1 = timestampset_time_n(ts, i);
    if (!found)
    {
      int cmp = timestamp_cmp_internal(t, t1);
      if (cmp < 0)
      {
        times[k++] = t;
        found = true;
      }
      else if (cmp == 0)
        found = true;
    }
    times[k++] = t1;
  }
  if (!found)
    times[k++] = t;
  return timestampset_make_free(times, k);
}

PG_FUNCTION_INFO_V1(union_timestamp_timestampset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = union_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_timestamp_period);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  Period *p1 = period_make(t, t, true, true);
  PeriodSet *result = union_period_period_internal(p, p1);
  pfree(p1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_timestamp_periodset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  Period *p = period_make(t, t, true, true);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  pfree(p);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(union_timestampset_timestamp);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result = union_timestamp_timestampset_internal(t, ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the union of the two time values (internal function)
 */
TimestampSet *
union_timestampset_timestampset_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, UNION);
}

PG_FUNCTION_INFO_V1(union_timestampset_timestampset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = union_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_timestampset_period);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  PeriodSet *ps = timestampset_to_periodset_internal(ts);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  pfree(ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_timestampset_periodset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  PeriodSet *ps1 = timestampset_to_periodset_internal(ts);
  PeriodSet *result = union_periodset_periodset_internal(ps, ps1);
  pfree(ps1);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the union of the two time values (internal function)
 */
PeriodSet *
union_period_period_internal(const Period *p1, const Period *p2)
{
  /* If the periods do not overlap */
  if (!overlaps_period_period_internal(p1, p2) &&
    !adjacent_period_period_internal(p1, p2))
  {
    const Period *periods[2];
    if (p1->lower < p2->lower)
    {
      periods[0] = p1;
      periods[1] = p2;
    }
    else
    {
      periods[0] = p2;
      periods[1] = p1;
    }
    PeriodSet *result = periodset_make((const Period **)periods, 2, NORMALIZE_NO);
    return result;
  }

  /* Compute the union of the overlapping periods */
  Period p;
  period_set(&p, p1->lower, p1->upper, p1->lower_inc, p1->upper_inc);
  period_expand(&p, p2);
  PeriodSet *result = period_to_periodset_internal(&p);
  return result;
}

PG_FUNCTION_INFO_V1(union_period_timestamp);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period *p1 = period_make(t, t, true, true);
  PeriodSet *result = union_period_period_internal(p, p1);
  pfree(p1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_period_timestampset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  PeriodSet *ps = timestampset_to_periodset_internal(ts);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  pfree(ps);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_period_period);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PG_RETURN_POINTER(union_period_period_internal(p1, p2));
}

/**
 * Returns the union of the two time values (internal function)
 */
PeriodSet *
union_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  /* Transform the period into a period set */
  PeriodSet *ps1 = period_to_periodset_internal(p);
  /* Call the function for the period set */
  PeriodSet *result = union_periodset_periodset_internal(ps1, ps);
  pfree(ps1);
  return result;
}

PG_FUNCTION_INFO_V1(union_period_periodset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(union_periodset_timestamp);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period *p = period_make(t, t, true, true);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  pfree(p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_periodset_timestampset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  PeriodSet *ps1 = timestampset_to_periodset_internal(ts);
  PeriodSet *result = union_periodset_periodset_internal(ps, ps1);
  pfree(ps1);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(union_periodset_period);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  PeriodSet *result = union_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the union of the two time values (internal function)
 */
PeriodSet *
union_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  Period **periods = palloc(sizeof(Period *) * (ps1->count + ps2->count));
  Period **mustfree = NULL;
  /* If the period sets overlap we will be intersecting composing periods */
  if (overlaps_periodset_periodset_internal(ps1, ps2))
    mustfree = palloc(sizeof(Period *) * Max(ps1->count, ps2->count));

  int i = 0, j = 0, k = 0, l = 0;
  while (i < ps1->count && j < ps2->count)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, j);
    /* The periods do not overlap, copy the earliest period */
    if (!overlaps_period_period_internal(p1, p2))
    {
      if (before_period_period_internal(p1, p2))
      {
        periods[k++] = (Period *) p1;
        i++;
      }
      else
      {
        periods[k++] = (Period *) p2;
        j++;
      }
    }
    else
    {
      /* Find all periods in ps1 that overlap with periods in ps2
       *      i                    i
       *   |-----| |-| |-----|  |-----|
       *       |---------|  |-----|
       *            j          j
       */
      Period *q = period_super_union(p1, p2);
      while (i < ps1->count && j < ps2->count)
      {
        p1 = periodset_per_n(ps1, i);
        p2 = periodset_per_n(ps2, j);
        if (!overlaps_period_period_internal(p1, q) &&
          !overlaps_period_period_internal(p2, q))
          break;
        if (overlaps_period_period_internal(p1, q))
        {
          period_expand(q, p1);
          i++;
        }
        if (overlaps_period_period_internal(p2, q))
        {
          period_expand(q, p2);
          j++;
        }
      }
      /* When one of the sets is finished we need to absorb overlapping
       * periods in the other set */
      while (i < ps1->count)
      {
        p1 = periodset_per_n(ps1, i);
        if (overlaps_period_period_internal(p1, q))
        {
          period_expand(q, p1);
          i++;
        }
        else
          break;
      }
      while (j < ps2->count)
      {
        p2 = periodset_per_n(ps2, j);
        if (overlaps_period_period_internal(p2, q))
        {
          period_expand(q, p2);
          j++;
        }
        else
          break;
      }
      periods[k++] = mustfree[l++] = q;
    }
  }
  /* Only one of the following two while will be executed */
  while (i < ps1->count)
    periods[k++] = (Period *) periodset_per_n(ps1, i++);
  while (j < ps2->count)
    periods[k++] = (Period *) periodset_per_n(ps2, j++);
  /* k is never equal to 0 since the periodsets are not empty */
  PeriodSet *result = periodset_make((const Period **) periods, k, NORMALIZE);
  pfree(periods);

  if (mustfree)
    pfree_array((void **) mustfree, l);

  return result;
}

PG_FUNCTION_INFO_V1(union_periodset_periodset);
/**
 * Returns the union of the two time values
 */
PGDLLEXPORT Datum
union_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  PeriodSet *result = union_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intersection_timestamp_timestamp);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  if (t1 != t2)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t1);
}

PG_FUNCTION_INFO_V1(intersection_timestamp_timestampset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool contains = contains_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 1);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(intersection_timestamp_period);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool contains = contains_period_timestamp_internal(p, t);
  PG_FREE_IF_COPY(p, 1);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(intersection_timestamp_periodset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool contains = contains_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 1);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(intersection_timestampset_timestamp);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool contains = contains_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
TimestampSet *
intersection_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, INTER);
}

PG_FUNCTION_INFO_V1(intersection_timestampset_timestampset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = intersection_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
TimestampSet *
intersection_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  return setop_timestampset_period(ts, p, INTER);
}

PG_FUNCTION_INFO_V1(intersection_timestampset_period);
/**
 * Returns the intersection of the two time values (internal function)
 */
PGDLLEXPORT Datum
intersection_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  TimestampSet *result = intersection_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(p, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
TimestampSet *
intersection_timestampset_periodset_internal(const TimestampSet *ts, const PeriodSet *ps)
{
  return setop_timestampset_periodset(ts, ps, INTER);
}

PG_FUNCTION_INFO_V1(intersection_timestampset_periodset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  TimestampSet *result = intersection_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(intersection_period_timestamp);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool contains = contains_period_timestamp_internal(p, t);
  PG_FREE_IF_COPY(p, 0);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(intersection_period_timestampset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = intersection_timestampset_period_internal(ts, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
Period *
intersection_period_period_internal(const Period *p1, const Period *p2)
{
  /* Bounding box test */
  if (!overlaps_period_period_internal(p1, p2))
    return NULL;

  TimestampTz lower = Max(p1->lower, p2->lower);
  TimestampTz upper = Min(p1->upper, p2->upper);
  bool lower_inc = p1->lower == p2->lower ? p1->lower_inc && p2->lower_inc :
    ( lower == p1->lower ? p1->lower_inc : p2->lower_inc );
  bool upper_inc = p1->upper == p2->upper ? p1->upper_inc && p2->upper_inc :
    ( upper == p1->upper ? p1->upper_inc : p2->upper_inc );
  return period_make(lower, upper, lower_inc, upper_inc);
}

PG_FUNCTION_INFO_V1(intersection_period_period);
/**
 * Returns the intersection of the two time values (internal function)
 */
PGDLLEXPORT Datum
intersection_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  Period *result = intersection_period_period_internal(p1, p2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_PERIOD(result);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
PeriodSet *
intersection_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period_internal(p, p1))
    return NULL;

  /* Is the period set fully contained in the period? */
  if (contains_period_periodset_internal(p, ps))
    return periodset_copy(ps);

  /* General case */
  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  Period **periods = palloc(sizeof(Period *) * (ps->count - loc));
  int k = 0;
  for (int i = loc; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    Period *p2 = intersection_period_period_internal(p1, p);
    if (p2 != NULL)
      periods[k++] = p2;
    if (p->upper < p1->upper)
      break;
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(intersection_period_periodset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  PeriodSet *result = intersection_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(intersection_periodset_timestamp);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool contains = contains_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (!contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(intersection_periodset_timestampset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = intersection_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(intersection_periodset_period);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  PeriodSet *result = intersection_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the intersection of the two time values (internal function)
 */
PeriodSet *
intersection_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps1);
  const Period *p2 = periodset_bbox_ptr(ps2);
  if (!overlaps_period_period_internal(p1, p2))
    return NULL;

  Period *inter = intersection_period_period_internal(p1, p2);
  int loc1, loc2;
  periodset_find_timestamp(ps1, inter->lower, &loc1);
  periodset_find_timestamp(ps2, inter->lower, &loc2);
  pfree(inter);
  Period **periods = palloc(sizeof(Period *) * (ps1->count + ps2->count - loc1 - loc2));
  int i = loc1, j = loc2, k = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    inter = intersection_period_period_internal(p1, p2);
    if (inter != NULL)
      periods[k++] = inter;
    int cmp = timestamp_cmp_internal(p1->upper, p2->upper);
    if (cmp == 0 && p1->upper_inc == p2->upper_inc)
    {
      i++; j++;
    }
    else if (cmp < 0 || (cmp == 0 && ! p1->upper_inc && p2->upper_inc))
      i++;
    else
      j++;
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE);
  return result;
}

PG_FUNCTION_INFO_V1(intersection_periodset_periodset);
/**
 * Returns the intersection of the two time values
 */
PGDLLEXPORT Datum
intersection_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  PeriodSet *result = intersection_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * All internal functions produce new results that must be freed after
 *****************************************************************************/

PG_FUNCTION_INFO_V1(minus_timestamp_timestamp);
/**
 * Returns the difference of the two time values (internal function)
 */
PGDLLEXPORT Datum
minus_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  if (t1 == t2)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t1);
}

PG_FUNCTION_INFO_V1(minus_timestamp_timestampset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  bool contains = contains_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 1);
  if (contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(minus_timestamp_period);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_PERIOD(1);
  bool contains = contains_period_timestamp_internal(p, t);
  PG_FREE_IF_COPY(p, 1);
  if (contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

PG_FUNCTION_INFO_V1(minus_timestamp_periodset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  bool contains = contains_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 1);
  if (contains)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(t);
}

/*****************************************************************************/

/**
 * Returns the difference of the two time values (internal function)
 */
TimestampSet *
minus_timestampset_timestamp_internal(const TimestampSet *ts, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = timestampset_bbox_ptr(ts);
  if (!contains_period_timestamp_internal(p, t))
    return timestampset_copy(ts);

  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t1 = timestampset_time_n(ts, i);
    if (t != t1)
      times[k++] = t1;
  }
  return timestampset_make_free(times, k);
}

PG_FUNCTION_INFO_V1(minus_timestampset_timestamp);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result = minus_timestampset_timestamp_internal(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
TimestampSet *
minus_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, MINUS);
}

PG_FUNCTION_INFO_V1(minus_timestampset_timestampset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
  TimestampSet *result = minus_timestampset_timestampset_internal(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  if (result == NULL)
    PG_RETURN_NULL() ;
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
TimestampSet *
minus_timestampset_period_internal(const TimestampSet *ts, const Period *p)
{
  return setop_timestampset_period(ts, p, MINUS);
}

PG_FUNCTION_INFO_V1(minus_timestampset_period);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  TimestampSet *result = minus_timestampset_period_internal(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(p, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
TimestampSet *
minus_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps)
{
  return setop_timestampset_periodset(ts, ps, MINUS);
}

PG_FUNCTION_INFO_V1(minus_timestampset_periodset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  TimestampSet *result = minus_timestampset_periodset_internal(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the difference of the two time values (internal function)
 */
static int
minus_period_timestamp_internal1(Period **result, const Period *p,
  TimestampTz t)
{
  if (!contains_period_timestamp_internal(p, t))
  {
    result[0] = period_copy(p);
    return 1;
  }

  if (p->lower == t && p->upper == t)
    return 0;

  if (p->lower == t)
  {
    result[0] = period_make(p->lower, p->upper, false, p->upper_inc);
    return 1;
  }

  if (p->upper == t)
  {
    result[0] = period_make(p->lower, p->upper, p->lower_inc, false);
    return 1;
  }

  result[0] = period_make(p->lower, t, p->lower_inc, false);
  result[1] = period_make(t, p->upper, false, p->upper_inc);
  return 2;
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_period_timestamp_internal(const Period *p, TimestampTz t)
{
  Period *periods[2];
  int count = minus_period_timestamp_internal1(periods, p, t);
  if (count == 0)
    return NULL;
  PeriodSet *result = periodset_make((const Period **) periods, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(periods[i]);
  return result;
}

PG_FUNCTION_INFO_V1(minus_period_timestamp);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_PERIOD(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_period_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_period_timestampset_internal(const Period *p, const TimestampSet *ts)
{
  /* Transform the period into a period set */
  PeriodSet *ps = period_to_periodset_internal(p);
  /* Bounding box test */
  const Period *p1 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period_internal(p, p1))
    return ps;

  /* Call the function for the period set */
  PeriodSet *result = minus_periodset_timestampset_internal(ps, ts);
  pfree(ps);
  return result;
}

PG_FUNCTION_INFO_V1(minus_period_timestampset);
/**
 * Returns the difference of the two time valuess
 */
PGDLLEXPORT Datum
minus_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_PERIOD(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  PeriodSet *result = minus_period_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
static int
minus_period_period_internal1(Period **result, const Period *p1,
  const Period *p2)
{
  PeriodBound lower1, lower2, upper1, upper2;

  period_deserialize(p1, &lower1, &upper1);
  period_deserialize(p2, &lower2, &upper2);

  int cmp_l1l2 = period_cmp_bounds(&lower1, &lower2);
  int cmp_l1u2 = period_cmp_bounds(&lower1, &upper2);
  int cmp_u1l2 = period_cmp_bounds(&upper1, &lower2);
  int cmp_u1u2 = period_cmp_bounds(&upper1, &upper2);

  /* Result is empty
   * p1         |----|
   * p2      |----------|
   */
  if (cmp_l1l2 >= 0 && cmp_u1u2 <= 0)
    return 0;

  /* Result is a periodset
   * p1      |----------|
   * p2         |----|
   * result  |--|    |--|
   */
  if (cmp_l1l2 < 0 && cmp_u1u2 > 0)
  {
    result[0] = period_make(p1->lower, p2->lower,
      p1->lower_inc, !(p2->lower_inc));
    result[1] = period_make(p2->upper, p1->upper,
      !(p2->upper_inc), p1->upper_inc);
    return 2;
  }

  /* Result is a period */
  /*
   * p1         |----|
   * p2  |----|
   * p2                 |----|
   * result      |----|
   */
  if (cmp_l1u2 > 0 || cmp_u1l2 < 0)
    result[0] = period_copy(p1);

  /*
   * p1           |-----|
   * p2               |----|
   * result       |---|
   */
  else if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    result[0] = period_make(p1->lower, p2->lower, p1->lower_inc, !(p2->lower_inc));
  /*
   * p1         |-----|
   * p2      |----|
   * result       |---|
   */
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    result[0] = period_make(p2->upper, p1->upper, !(p2->upper_inc), p1->upper_inc);
  return 1;
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_period_period_internal(const Period *p1, const Period *p2)
{
  Period *periods[2];
  int count = minus_period_period_internal1(periods, p1, p2);
  if (count == 0)
    return NULL;
  PeriodSet *result = periodset_make((const Period **) periods, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(periods[i]);
  return result;
}

PG_FUNCTION_INFO_V1(minus_period_period);
/**
 * Returns the difference of the two time values (internal function)
 */
PGDLLEXPORT Datum
minus_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD(0);
  Period *p2 = PG_GETARG_PERIOD(1);
  PeriodSet *result = minus_period_period_internal(p1, p2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
static int
minus_period_periodset_internal1(Period **result, const Period *p, const PeriodSet *ps,
  int from, int to)
{
  /* The period can be split at most into (to - from + 1) periods
   *   |----------------------|
   *       |---| |---| |---|
   */
  Period *curr = period_copy(p);
  int k = 0;
  for (int i = from; i < to; i++)
  {
    const Period *p1 = periodset_per_n(ps, i);
    /* If the remaining periods are to the left of the current period */
    int cmp = timestamp_cmp_internal(curr->upper, p1->lower);
    if (cmp < 0 || (cmp == 0 && curr->upper_inc && ! p1->lower_inc))
    {
      result[k++] = curr;
      break;
    }
    Period *minus[2];
    int countminus = minus_period_period_internal1(minus, curr, p1);
    pfree(curr);
    /* minus can have from 0 to 2 periods */
    if (countminus == 0)
      break;
    else if (countminus == 1)
      curr = minus[0];
    else /* countminus == 2 */
    {
      result[k++] = period_copy(minus[0]);
      curr = minus[1];
    }
    /* There are no more periods left */
    if (i == to - 1)
      result[k++] = curr;
  }
  return k;
}

PeriodSet *
minus_period_periodset_internal(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period_internal(p, p1))
    return periodset_make((const Period **) &p, 1, false);

  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int count = minus_period_periodset_internal1(periods, p, ps, 0, ps->count);
  PeriodSet *result = periodset_make_free(periods, count, false);
  return result;
}

PG_FUNCTION_INFO_V1(minus_period_periodset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD(0);
  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  PeriodSet *result = minus_period_periodset_internal(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_periodset_timestamp_internal(const PeriodSet *ps, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = periodset_bbox_ptr(ps);
  if (!contains_period_timestamp_internal(p, t))
    return periodset_copy(ps);

  /* At most one composing period can be split into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int k = 0;
  for (int i = 0; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    k += minus_period_timestamp_internal1(&periods[k], p, t);
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(minus_periodset_timestamp);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_periodset_timestamp_internal(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  const Period *p2 = timestampset_bbox_ptr(ts);
  if (!overlaps_period_period_internal(p1, p2))
    return periodset_copy(ps);

  /* Each timestamp will split at most one composing period into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + ts->count + 1));
  int i = 0, j = 0, k = 0;
  Period *curr = period_copy(periodset_per_n(ps, 0));
  TimestampTz t = timestampset_time_n(ts, 0);
  while (i < ps->count && j < ts->count)
  {
    if (t > curr->upper)
    {
      periods[k++] = curr;
      i++;
      if (i == ps->count)
        break;
      else
        curr = period_copy(periodset_per_n(ps, i));
    }
    else if (t < curr->lower)
    {
      j++;
      if (j == ts->count)
        break;
      else
        t = timestampset_time_n(ts, j);
    }
    else
    {
      if (contains_period_timestamp_internal(curr, t))
      {
        if (curr->lower == curr->upper)
        {
          pfree(curr);
          i++;
          if (i == ps->count)
            break;
          else
            curr = period_copy(periodset_per_n(ps, i));
        }
        else if (curr->lower == t)
        {
          Period *curr1 = period_make(curr->lower, curr->upper, false, curr->upper_inc);
          pfree(curr);
          curr = curr1;
        }
        else if (curr->upper == t)
        {
          periods[k++] = period_make(curr->lower, curr->upper, curr->lower_inc, false);
          pfree(curr);
          i++;
          if (i == ps->count)
            break;
          else
            curr = period_copy(periodset_per_n(ps, i));
        }
        else
        {
          periods[k++] = period_make(curr->lower, t, curr->lower_inc, false);
          Period *curr1 = period_make(t, curr->upper, false, curr->upper_inc);
          pfree(curr);
          curr = curr1;
        }
      }
      else
      {
        if (curr->upper == t)
        {
          periods[k++] = curr;
          i++;
          if (i == ps->count)
            break;
          else
            curr = period_copy(periodset_per_n(ps, i));
        }
      }
      j++;
      if (j == ts->count)
        break;
      else
        t = timestampset_time_n(ts, j);
    }
  }
  /* If we ran through all the instants */
  if (j == ts->count)
    periods[k++] = curr;
  for (int l = i + 1; l < ps->count; l++)
    periods[k++] = (Period *) periodset_per_n(ps, l);

  if (k == 0)
  {
    pfree(periods);
    return NULL;
  }
  PeriodSet *result = periodset_make((const Period **)periods, k, NORMALIZE_NO);
  for (int l = 0; l < i; l++)
    pfree(periods[l]);
  pfree(periods);
  return result;
}

PG_FUNCTION_INFO_V1(minus_periodset_timestampset);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  PeriodSet *result = minus_periodset_timestampset_internal(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_periodset_period_internal(const PeriodSet *ps, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps);
  if (!overlaps_period_period_internal(p1, p))
    return periodset_copy(ps);

  /* At most one composing period can be split into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int k = 0;
  for (int i = 0; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    k += minus_period_period_internal1(&periods[k], p1, p);
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(minus_periodset_period);
/**
 * Returns the difference of the two time values
 */
PGDLLEXPORT Datum
minus_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *p = PG_GETARG_PERIOD(1);
  PeriodSet *result = minus_periodset_period_internal(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Returns the difference of the two time values (internal function)
 */
PeriodSet *
minus_periodset_periodset_internal(const PeriodSet *ps1, const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_bbox_ptr(ps1);
  const Period *p2 = periodset_bbox_ptr(ps2);
  if (!overlaps_period_period_internal(p1, p2))
    return periodset_copy(ps1);

  Period **periods = palloc(sizeof(const Period *) * (ps1->count + ps2->count));
  int i = 0, j = 0, k = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    /* The periods do not overlap, copy the first period */
    if (!overlaps_period_period_internal(p1, p2))
    {
      periods[k++] = period_copy(p1);
      i++;
    }
    else
    {
      /* Find all periods in ps2 that overlap with p1
       *                  i
       *    |------------------------|
       *      |-----|  |-----|          |---|
       *         j                        l
       */
      int l;
      for (l = j; l < ps2->count; l++)
      {
        const Period *p3 = periodset_per_n(ps2, l);
        if (!overlaps_period_period_internal(p1, p3))
          break;
      }
      int to = Min(l, ps2->count);
      /* Compute the difference of the overlapping periods */
      k += minus_period_periodset_internal1(&periods[k], p1,
        ps2, j, to);
      i++;
      j = l;
    }
  }
  /* Copy the sequences after the period set */
  while (i < ps1->count)
    periods[k++] = period_copy(periodset_per_n(ps1, i++));
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

PG_FUNCTION_INFO_V1(minus_periodset_periodset);
/**
 * Returns the difference of the two time values (internal function)
 */
PGDLLEXPORT Datum
minus_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
  PeriodSet *result = minus_periodset_periodset_internal(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  if (! result)
    PG_RETURN_NULL() ;
  PG_RETURN_POINTER(result);
}

/******************************************************************************/
