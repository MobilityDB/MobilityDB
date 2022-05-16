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
 * @file time_ops.c
 * @brief Operators for time types.
 */

#include "general/time_ops.h"

/* C */
#include <assert.h>
/* PostgreSQL */
// #include <utils/builtins.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/periodset.h"
#include "general/timestampset.h"
#include "general/temporal_util.h"

typedef enum
{
  UNION,
  INTER,
  MINUS
} SetOper;

/*****************************************************************************/

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
 * Determine the relative position of a period and a timestamp
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
 * Return the union, intersection or difference of two timestamp sets
 */
TimestampSet *
setop_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2, SetOper setop)
{
  if (setop == INTER || setop == MINUS)
  {
    /* Bounding box test */
    const Period *p1 = timestampset_period_ptr(ts1);
    const Period *p2 = timestampset_period_ptr(ts2);
    if (! overlaps_span_span(p1, p2))
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
 * Return the intersection or the difference of a timestamp set and a period
 */
TimestampSet *
setop_timestampset_period(const TimestampSet *ts, const Period *p,
  SetOper setop)
{
  assert(setop == INTER || setop == MINUS);
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts);
  if (! overlaps_span_span(p1, p))
    return (setop == INTER) ? NULL : timestampset_copy(ts);

  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  int k = 0;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    if (((setop == INTER) && contains_period_timestamp(p, t)) ||
      ((setop == MINUS) && !contains_period_timestamp(p, t)))
      times[k++] = t;
  }
  return timestampset_make_free(times, k);
}

/*
 * Return the intersection or the difference of a timestamp set and a period set
 */
TimestampSet *
setop_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps,
  SetOper setop)
{
  assert(setop == INTER || setop == MINUS);
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts);
  const Period *p2 = periodset_period_ptr(ps);
  if (! overlaps_span_span(p1, p2))
    return (setop == INTER) ? NULL : timestampset_copy(ts);

  TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
  TimestampTz t = timestampset_time_n(ts, 0);
  const Period *p = periodset_per_n(ps, 0);
  int i = 0, j = 0, k = 0;
  while (i < ts->count && j < ps->count)
  {
    if (t < (TimestampTz) p->lower)
    {
      if (setop == MINUS)
        times[k++] = t;
      i++;
      if (i == ts->count)
        break;
      else
        t = timestampset_time_n(ts, i);
    }
    else if (t > (TimestampTz) p->upper)
    {
      j++;
      if (j == ps->count)
        break;
      else
        p = periodset_per_n(ps, j);
    }
    else
    {
      if ((setop == INTER && contains_period_timestamp(p, t)) ||
        (setop == MINUS && !contains_period_timestamp(p, t)))
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
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set contains a timestamp.
 */
bool
contains_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = timestampset_period_ptr(ts);
  if (! contains_period_timestamp(p, t))
    return false;

  int loc;
  return timestampset_find_timestamp(ts, t, &loc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first timestamp set contains the second one.
 */
bool
contains_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts1);
  const Period *p2 = timestampset_period_ptr(ts2);
  if (! contains_span_span(p1, p2))
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

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period contains a timestamp.
 */
bool
contains_period_timestamp(const Period *p, TimestampTz t)
{
  int cmp = timestamp_cmp_internal(p->lower, t);
  if (cmp > 0 || (cmp == 0 && ! p->lower_inc))
    return false;

  cmp = timestamp_cmp_internal(p->upper, t);
  if (cmp < 0 || (cmp == 0 && ! p->upper_inc))
    return false;

  return true;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period contains a timestamp set.
 */
bool
contains_period_timestampset(const Period *p, const TimestampSet *ts)
{
  /* It is sufficient to do a bounding box test */
  const Period *p1 = timestampset_period_ptr(ts);
  if (! contains_span_span(p, p1))
    return false;
  return true;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set contains a timestamp.
 */
bool
contains_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = periodset_period_ptr(ps);
  if (! contains_period_timestamp(p, t))
    return false;

  int loc;
  if (! periodset_find_timestamp(ps, t, &loc))
    return false;
  return true;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set contains a timestamp set.
 */
bool
contains_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  const Period *p2 = timestampset_period_ptr(ts);
  if (! contains_span_span(p1, p2))
    return false;

  int i = 0, j = 0;
  while (j < ts->count)
  {
    const Period *p = periodset_per_n(ps, i);
    TimestampTz t = timestampset_time_n(ts, j);
    if (contains_period_timestamp(p, t))
      j++;
    else
    {
      if (t > (TimestampTz) p->upper)
        i++;
      else
        return false;
    }
  }
  return true;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set contains a period.
 */
bool
contains_periodset_period(const PeriodSet *ps, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  if (! contains_span_span(p1, p))
    return false;

  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  p1 = periodset_per_n(ps, loc);
  return contains_span_span(p1, p);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period contains a period set.
 */
bool
contains_period_periodset(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_period_ptr(ps);
  return contains_span_span(p, p1);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first period set contains the second one.
 */
bool
contains_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps1);
  const Period *p2 = periodset_period_ptr(ps2);
  if (! contains_span_span(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    if (left_span_span(p1, p2))
      i++;
    else if (left_span_span(p2, p1))
      return false;
    else
    {
      /* p1 and p2 overlap */
      if (contains_span_span(p1, p2))
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

/*****************************************************************************/
/* contained? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp is contained by a timestamp set
 */
bool
contained_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  return contains_timestampset_timestamp(ts, t);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp is contained by a period
 */
bool
contained_timestamp_period(TimestampTz t, const Period *p)
{
  return contains_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp is contained by a period set
 */
bool
contained_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  return contains_periodset_timestamp(ps, t);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set is contained by the second one
 */
bool
contained_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return contains_timestampset_timestampset(ts2, ts1);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set is contained by a period
 */
bool
contained_timestampset_period(const TimestampSet *ts, const Period *p)
{
  return contains_period_timestampset(p, ts);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set is contained by a period set
 */
bool
contained_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  return contains_periodset_timestampset(ps, ts);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period is contained by a period
 */
bool
contained_period_periodset(const Period *p, const PeriodSet *ps)
{
  return contains_periodset_period(ps, p);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set is contained by a period
 */
bool
contained_periodset_period(const PeriodSet *ps, const Period *p)
{
  return contains_period_periodset(p, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the first period set is contained by the second one
 */
bool
contained_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  return contains_periodset_periodset(ps2, ps1);
}

/*****************************************************************************/
/* overlaps? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if the timestamp sets overlap.
 */
bool
overlaps_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts1);
  const Period *p2 = timestampset_period_ptr(ts2);
  if (! overlaps_span_span(p1, p2))
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

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set and a period overlap.
 */
bool
overlaps_timestampset_period(const TimestampSet *ts, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts);
  if (! overlaps_span_span(p, p1))
    return false;

  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    if (contains_period_timestamp(p, t))
      return true;
  }
  return false;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set and a period set overlap.
 */
bool
overlaps_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  const Period *p2 = timestampset_period_ptr(ts);
  if (! overlaps_span_span(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ts->count && j < ps->count)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    const Period *p = periodset_per_n(ps, j);
    if (contains_period_timestamp(p, t))
      return true;
    else if (t > (TimestampTz) p->upper)
      j++;
    else
      i++;
  }
  return false;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period and a timestamp set overlap
 */
bool
overlaps_period_timestampset(const Period *p, const TimestampSet *ts)
{
  return overlaps_timestampset_period(ts, p);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period and a period set overlap.
 */
bool
overlaps_period_periodset(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  if (! overlaps_span_span(p, p1))
    return false;

  /* Binary search of lower bound of period */
  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  for (int i = loc; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    if (overlaps_span_span(p1, p))
      return true;
    if (p->upper < p1->upper)
      break;
  }
  return false;
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set and a timestamp set overlap
 */
bool
overlaps_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  return overlaps_timestampset_periodset(ts, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a periodset and a period overlap
 */
bool
overlaps_periodset_period(const PeriodSet *ps, const Period *p)
{
  return overlaps_period_periodset(p, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if two period sets overlap.
 */
bool
overlaps_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps1);
  const Period *p2 = periodset_period_ptr(ps2);
  if (! overlaps_span_span(p1, p2))
    return false;

  int i = 0, j = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    if (overlaps_span_span(p1, p2))
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

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp and a period are adjacent.
 */
bool
adjacent_timestamp_period(TimestampTz t, const Period *p)
{
  /*
   * A timestamp A and a period C..D are adjacent if and only if
   * A is adjacent to C, or D is adjacent to A.
   */
  return (t == (TimestampTz) p->lower && ! p->lower_inc) ||
    ((TimestampTz) p->upper == t && ! p->upper_inc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp and a period set are adjacent.
 */
bool
adjacent_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  /*
   * Two periods A..B and C..D are adjacent if and only if
   * B is adjacent to C, or D is adjacent to A.
   */
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  return (t == (TimestampTz) p1->lower && ! p1->lower_inc) ||
       ((TimestampTz) p2->upper == t && ! p2->upper_inc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set and a period are adjacent.
 */
bool
adjacent_timestampset_period(const TimestampSet *ts, const Period *p)
{
  /*
   * A periods A..B and a timestamptz C are adjacent if and only if
   * B is adjacent to C, or C is adjacent to A.
   */
  TimestampTz t1 = timestampset_time_n(ts, 0);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  return (t2 == (TimestampTz) p->lower && ! p->lower_inc) ||
         ((TimestampTz) p->upper == t1 && ! p->upper_inc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a timestamp set and a period set are adjacent.
 */
bool
adjacent_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  /*
   * A periods A..B and a timestamptz C are adjacent if and only if
   * B is adjacent to C, or C is adjacent to A.
   */
  TimestampTz t1 = timestampset_time_n(ts, 0);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  return (t2 == (TimestampTz) p1->lower && ! p1->lower_inc) ||
       ((TimestampTz) p2->upper == t1 && ! p2->upper_inc);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period and a timestamp are adjacent
 */
bool
adjacent_period_timestamp(const Period *p, TimestampTz t)
{
  return adjacent_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period and a timestamp set are adjacent
 */
bool
adjacent_period_timestampset(const Period *p, const TimestampSet *ts)
{
  return adjacent_timestampset_period(ts, p);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period and a period set are adjacent.
 */
bool
adjacent_period_periodset(const Period *p, const PeriodSet *ps)
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

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set a timestamp are adjacent
 */
bool
adjacent_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return adjacent_timestamp_periodset(t, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set and a timestamp set are adjacent
 */
bool
adjacent_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  return adjacent_timestampset_periodset(ts, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set and a period are adjacent
 */
bool
adjacent_periodset_period(const PeriodSet *ps, const Period *p)
{
  return adjacent_period_periodset(p, ps);
}

/**
 * @ingroup libmeos_spantime_topo
 * @brief Return true if a period set and a period are adjacent.
 */
bool
adjacent_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
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

/*****************************************************************************/
/* strictly before of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly before the second
 * one.
 */
bool
before_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t < t1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly before a period.
 */
bool
before_timestamp_period(TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->lower);
  return (cmp < 0 || (cmp == 0 && ! p->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly before a period set.
 */
bool
before_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return before_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly before a timestamp set.
 */
bool
before_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t1 < t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly before the second one.
 */
bool
before_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, ts1->count - 1);
  TimestampTz t2 = timestampset_time_n(ts2, 0);
  return (t1 < t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly before a period.
 */
bool
before_timestampset_period(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return before_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly before a period set.
 */
bool
before_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return before_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is strictly before a timestamp.
 */
bool
before_period_timestamp(const Period *p, TimestampTz t)
{

  int cmp = timestamp_cmp_internal(p->upper, t);
  return (cmp < 0 || (cmp == 0 && ! p->upper_inc));
}

/**
 * Return true if a period is strictly before a timestamp set
 */
bool
before_period_timestampset(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return before_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is strictly before a period set.
 */
bool
before_period_periodset(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return left_span_span(p, p1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly before a timestamp.
 */
bool
before_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return before_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly before a timestamp set.
 */
bool
before_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz t = timestampset_time_n(ts, 0);
  return before_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly before a period.
 */
bool
before_periodset_period(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return left_span_span(p1, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first period set is strictly before the second one.
 */
bool
before_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, ps1->count - 1);
  const Period *p2 = periodset_per_n(ps2, 0);
  return left_span_span(p1, p2);
}

/*****************************************************************************/
/* strictly after of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly after a timestamp set.
 */
bool
after_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t > t1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly after a period.
 */
bool
after_timestamp_period(TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->upper);
  return (cmp > 0 || (cmp == 0 && ! p->upper_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is strictly after a period set.
 */
bool
after_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return after_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly after a timestamp.
 */
bool
after_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t1 > t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first timestamp set is strictly after the second one.
 */
bool
after_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, 0);
  TimestampTz t2 = timestampset_time_n(ts2, ts2->count - 1);
  return (t1 > t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly after a period.
 */
bool
after_timestampset_period(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return after_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is strictly after a period set.
 */
bool
after_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  TimestampTz t = timestampset_time_n(ts, 0);
  return after_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is strictly after a timestamp.
 */
bool
after_period_timestamp(const Period *p, TimestampTz t)
{
  int cmp = timestamp_cmp_internal(t, p->lower);
  return (cmp < 0 || (cmp == 0 && ! p->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is strictly after a timestamp set.
 */
bool
after_period_timestampset(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return after_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is strictly after a period set.
 */
bool
after_period_periodset(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return right_span_span(p, p1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly after a timestamp.
 */
bool
after_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, 0);
  return after_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly after a timestamp set.
 */
bool
after_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return after_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is strictly after a period.
 */
bool
after_periodset_period(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return right_span_span(p1, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first period set is strictly after the second one.
 */
bool
after_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, 0);
  const Period *p2 = periodset_per_n(ps2, ps2->count - 1);
  return right_span_span(p1, p2);
}

/*****************************************************************************/
/* does not extend to right of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not after a timestamp set.
 */
bool
overbefore_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t <= t1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not after a period.
 */
bool
overbefore_timestamp_period(TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(t, p->upper);
  return (cmp < 0 || (cmp == 0 && p->upper_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not after a period set.
 */
bool
overbefore_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return overbefore_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not after a timestamp.
 */
bool
overbefore_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, ts->count - 1);
  return (t1 <= t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first timestamp set is not after the second one.
 */
bool
overbefore_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, ts1->count - 1);
  TimestampTz t2 = timestampset_time_n(ts2, ts2->count - 1);
  return (t1 <= t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not after a period.
 */
bool
overbefore_timestampset_period(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return (overbefore_timestamp_period(t, p));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not after a period set.
 */
bool
overbefore_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return (!after_timestamp_period(t, p));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not after a timestamp.
 */
bool
overbefore_period_timestamp(const Period *p, TimestampTz t)
{
  return (TimestampTz) p->upper <= t;
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not after a timestamp set.
 */
bool
overbefore_period_timestampset(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, ts->count - 1);
  return (overbefore_period_timestamp(p, t));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not after a period set.
 */
bool
overbefore_period_periodset(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return overleft_span_span(p, p1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not after a timestamp.
 */
bool
overbefore_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return overbefore_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not after a timestamp set.
 */
bool
overbefore_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  TimestampTz t1 = periodset_end_timestamp(ps);
  TimestampTz t2 = timestampset_time_n(ts, ts->count - 1);
  return (t1 <= t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not after a period.
 */
bool
overbefore_periodset_period(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, ps->count - 1);
  return overleft_span_span(p1, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first a period set is not after the second one.
 */
bool
overbefore_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, ps1->count - 1);
  const Period *p2 = periodset_per_n(ps2, ps2->count - 1);
  return overleft_span_span(p1, p2);
}

/*****************************************************************************/
/* does not extend to left of? */

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not before a timestamp set.
 */
bool
overafter_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t >= t1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not before a period.
 */
bool
overafter_timestamp_period(TimestampTz t, const Period *p)
{
  int cmp = timestamp_cmp_internal(p->lower, t);
  return (cmp < 0 || (cmp == 0 && p->lower_inc));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp is not before a period set.
 */
bool
overafter_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return overafter_timestamp_period(t, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not before a timestamp.
 */
bool
overafter_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  TimestampTz t1 = timestampset_time_n(ts, 0);
  return (t1 >= t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first timestamp set is not before the second one.
 */
bool
overafter_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  TimestampTz t1 = timestampset_time_n(ts1, 0);
  TimestampTz t2 = timestampset_time_n(ts2, 0);
  return (t1 >= t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not before a period.
 */
bool
overafter_timestampset_period(const TimestampSet *ts, const Period *p)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return (overafter_timestamp_period(t, p));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a timestamp set is not before a period set.
 */
bool
overafter_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  const Period *p = periodset_per_n(ps, 0);
  return (overafter_timestamp_period(t, p));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not before a timestamp.
 */
bool
overafter_period_timestamp(const Period *p, TimestampTz t)
{
  return (t <= (TimestampTz) p->lower);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not before a timestamp set.
 */
bool
overafter_period_timestampset(const Period *p, const TimestampSet *ts)
{
  TimestampTz t = timestampset_time_n(ts, 0);
  return (overafter_period_timestamp(p, t));
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period is not before a period set.
 */
bool
overafter_period_periodset(const Period *p, const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return overright_span_span(p, p1);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not before a timestamp.
 */
bool
overafter_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  const Period *p = periodset_per_n(ps, 0);
  return overafter_period_timestamp(p, t);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not before a timestamp set.
 */
bool
overafter_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  TimestampTz t1 = periodset_start_timestamp(ps);
  TimestampTz t2 = timestampset_time_n(ts, 0);
  return (t1 >= t2);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if a period set is not before a period.
 */
bool
overafter_periodset_period(const PeriodSet *ps, const Period *p)
{
  const Period *p1 = periodset_per_n(ps, 0);
  return overright_span_span(p1, p);
}

/**
 * @ingroup libmeos_spantime_pos
 * @brief Return true if the first period set is not before the second one.
 */
bool
overafter_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  const Period *p1 = periodset_per_n(ps1, 0);
  const Period *p2 = periodset_per_n(ps2, 0);
  return overright_span_span(p1, p2);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the timestamps
 */
TimestampSet *
union_timestamp_timestamp(TimestampTz t1, TimestampTz t2)
{
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
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp and a timestamp set.
 */
TimestampSet *
union_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  TimestampTz *times = palloc(sizeof(TimestampTz) * (ts->count + 1));
  int k = 0;
  bool found = false;
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t1 = timestampset_time_n(ts, i);
    if (! found)
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
  if (! found)
    times[k++] = t;
  return timestampset_make_free(times, k);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp and a period
 */
PeriodSet *
union_timestamp_period(TimestampTz t, const Period *p)
{
  Period p1;
  span_set(t, t, true, true, T_TIMESTAMPTZ, &p1);
  PeriodSet *result = union_period_period(p, &p1);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp and a period set
 */
PeriodSet *
union_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  Period p;
  span_set(t, t, true, true, T_TIMESTAMPTZ, &p);
  PeriodSet *result = union_period_periodset(&p, ps);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp set and a timestamp
 */
TimestampSet *
union_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t)
{
  return union_timestamp_timestampset(t, ts);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the timestamp sets.
 */
TimestampSet *
union_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, UNION);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp set and a period
 */
PeriodSet *
union_timestampset_period(const TimestampSet *ts, const Period *p)
{
  PeriodSet *ps = timestampset_to_periodset(ts);
  PeriodSet *result = union_period_periodset(p, ps);
  pfree(ps);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a timestamp set and a period set
 */
PeriodSet *
union_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  PeriodSet *ps1 = timestampset_to_periodset(ts);
  PeriodSet *result = union_periodset_periodset(ps, ps1);
  pfree(ps1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period and a timestamp
 */
PeriodSet *
union_period_timestamp(const Period *p, TimestampTz t)
{
  Period p1;
  span_set(t, t, true, true, T_TIMESTAMPTZ, &p1);
  PeriodSet *result = union_period_period(p, &p1);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period and a timestamp set
 */
PeriodSet *
union_period_timestampset(const Period *p, const TimestampSet *ts)
{
  PeriodSet *ps = timestampset_to_periodset(ts);
  PeriodSet *result = union_period_periodset(p, ps);
  pfree(ps);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the periods.
 */
PeriodSet *
union_period_period(const Period *p1, const Period *p2)
{
  /* If the periods do not overlap */
  if (! overlaps_span_span(p1, p2) &&
    !adjacent_span_span(p1, p2))
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
  span_set(p1->lower, p1->upper, p1->lower_inc, p1->upper_inc, T_TIMESTAMPTZ, &p);
  span_expand(p2, &p);
  PeriodSet *result = period_to_periodset(&p);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period and a period set.
 */
PeriodSet *
union_period_periodset(const Period *p, const PeriodSet *ps)
{
  /* Transform the period into a period set */
  PeriodSet *ps1 = period_to_periodset(p);
  /* Call the function for the period set */
  PeriodSet *result = union_periodset_periodset(ps1, ps);
  pfree(ps1);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period set and a timestamp.
 */
PeriodSet *
union_periodset_timestamp(PeriodSet *ps, TimestampTz t)
{
  Period p;
  span_set(t, t, true, true, T_TIMESTAMPTZ, &p);
  PeriodSet *result = union_period_periodset(&p, ps);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period set and a timestamp set.
 */
PeriodSet *
union_periodset_timestampset(PeriodSet *ps, TimestampSet *ts)
{
  PeriodSet *ps1 = timestampset_to_periodset(ts);
  PeriodSet *result = union_periodset_periodset(ps, ps1);
  pfree(ps1);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of a period set and a period
 */
PeriodSet *
union_periodset_period(const PeriodSet *ps, const Period *p)
{
  return  union_period_periodset(p, ps);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the union of the period sets.
 */
PeriodSet *
union_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  Period **periods = palloc(sizeof(Period *) * (ps1->count + ps2->count));
  Period **mustfree = NULL;
  /* If the period sets overlap we will be intersecting composing periods */
  if (overlaps_periodset_periodset(ps1, ps2))
    mustfree = palloc(sizeof(Period *) * Max(ps1->count, ps2->count));

  int i = 0, j = 0, k = 0, l = 0;
  while (i < ps1->count && j < ps2->count)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, j);
    /* The periods do not overlap, copy the earliest period */
    if (! overlaps_span_span(p1, p2))
    {
      if (left_span_span(p1, p2))
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
      Period *q = union_span_span(p1, p2, false);
      while (i < ps1->count && j < ps2->count)
      {
        p1 = periodset_per_n(ps1, i);
        p2 = periodset_per_n(ps2, j);
        bool over_p1_q = overlaps_span_span(p1, q);
        bool over_p2_q = overlaps_span_span(p2, q);
        if (! over_p1_q && ! over_p2_q)
          break;
        if (over_p1_q)
        {
          span_expand(p1, q);
          i++;
        }
        if (over_p2_q)
        {
          span_expand(p2, q);
          j++;
        }
      }
      /* When one of the sets is finished we need to absorb overlapping
       * periods in the other set */
      while (i < ps1->count)
      {
        p1 = periodset_per_n(ps1, i);
        if (overlaps_span_span(p1, q))
        {
          span_expand(p1, q);
          i++;
        }
        else
          break;
      }
      while (j < ps2->count)
      {
        p2 = periodset_per_n(ps2, j);
        if (overlaps_span_span(p2, q))
        {
          span_expand(p2, q);
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

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the timestamps
 */
bool
intersection_timestamp_timestamp(TimestampTz t1, TimestampTz t2,
  TimestampTz *result)
{
  if (t1 != t2)
    return false;
  *result  = t1;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp and a timestamp set
 */
bool
intersection_timestamp_timestampset(TimestampTz t, const TimestampSet *ts,
  TimestampTz *result)
{
  if (! contains_timestampset_timestamp(ts, t))
    return false;
  *result  = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp and a period
 */
bool
intersection_timestamp_period(TimestampTz t, const Period *p,
  TimestampTz *result)
{
  if (! contains_period_timestamp(p, t))
    return false;
  *result  = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp and a period set
 */
bool
intersection_timestamp_periodset(TimestampTz t, const PeriodSet *ps,
  TimestampTz *result)
{
  if (! contains_periodset_timestamp(ps, t))
    return false;
  *result  = t;
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp set and a timestamp
 */
bool
intersection_timestampset_timestamp(const TimestampSet *ts, const TimestampTz t,
  TimestampTz *result)
{
  if (! contains_timestampset_timestamp(ts, t))
    return false;
  *result  = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the timestamp sets.
 */
TimestampSet *
intersection_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, INTER);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp set and a period.
 */
TimestampSet *
intersection_timestampset_period(const TimestampSet *ts, const Period *p)
{
  return setop_timestampset_period(ts, p, INTER);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a timestamp set and a period set.
 */
TimestampSet *
intersection_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  return setop_timestampset_periodset(ts, ps, INTER);
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period and a timestamp
 */
bool
intersection_period_timestamp(const Period *p, TimestampTz t,
  TimestampTz *result)
{
  if (! contains_period_timestamp(p, t))
    return false;
  *result  = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period and a timestamp set
 */
TimestampSet *
intersection_period_timestampset(const Period *ps, const TimestampSet *ts)
{
  return intersection_timestampset_period(ts, ps);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period and a period set.
 */
PeriodSet *
intersection_period_periodset(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  if (! overlaps_span_span(p, p1))
    return NULL;

  /* Is the period set fully contained in the period? */
  if (contains_period_periodset(p, ps))
    return periodset_copy(ps);

  /* General case */
  int loc;
  periodset_find_timestamp(ps, p->lower, &loc);
  Period **periods = palloc(sizeof(Period *) * (ps->count - loc));
  int k = 0;
  for (int i = loc; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    Period *p2 = intersection_span_span(p1, p);
    if (p2 != NULL)
      periods[k++] = p2;
    if (p->upper < p1->upper)
      break;
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period set and a timestamp
 */
bool
intersection_periodset_timestamp(const PeriodSet *ps, TimestampTz t,
  TimestampTz *result)
{
  if (! contains_periodset_timestamp(ps, t))
    return false;
  *result = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period set and a timestamp set
 */
TimestampSet *
intersection_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  return intersection_timestampset_periodset(ts, ps);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of a period set and a period
 */
PeriodSet *
intersection_periodset_period(const PeriodSet *ps, const Period *p)
{
  return intersection_period_periodset(p, ps);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the intersection of the period sets.
 */
PeriodSet *
intersection_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps1);
  const Period *p2 = periodset_period_ptr(ps2);
  Period p;
  if (! inter_span_span(p1, p2, &p))
    return NULL;

  int loc1, loc2;
  periodset_find_timestamp(ps1, p.lower, &loc1);
  periodset_find_timestamp(ps2, p.lower, &loc2);
  Period **periods = palloc(sizeof(Period *) *
    (ps1->count + ps2->count - loc1 - loc2));
  int i = loc1, j = loc2, k = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    Period *inter = intersection_span_span(p1, p2);
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

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the timestamps
 */
bool
minus_timestamp_timestamp(TimestampTz t1, TimestampTz t2, TimestampTz *result)
{
  if (t1 == t2)
    return false;
  *result = t1;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp and a timestamp set
 */
bool
minus_timestamp_timestampset(TimestampTz t, const TimestampSet *ts,
  TimestampTz *result)
{
  if (contains_timestampset_timestamp(ts, t))
    return false;
  *result = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp and a period
 */
bool
minus_timestamp_period(TimestampTz t, const Period *p, TimestampTz *result)
{
  if (contains_period_timestamp(p, t))
    return false;
  *result = t;
  return true;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp and a period set
 */
bool
minus_timestamp_periodset(TimestampTz t, const PeriodSet *ps,
  TimestampTz *result)
{
  if (contains_periodset_timestamp(ps, t))
    return false;
  *result = t;
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp set and a timestamp.
 */
TimestampSet *
minus_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = timestampset_period_ptr(ts);
  if (! contains_period_timestamp(p, t))
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

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the timestamp sets.
 */
TimestampSet *
minus_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  return setop_timestampset_timestampset(ts1, ts2, MINUS);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp set and a period.
 */
TimestampSet *
minus_timestampset_period(const TimestampSet *ts, const Period *p)
{
  return setop_timestampset_period(ts, p, MINUS);
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a timestamp set and a period set.
 */
TimestampSet *
minus_timestampset_periodset(const TimestampSet *ts, const PeriodSet *ps)
{
  return setop_timestampset_periodset(ts, ps, MINUS);
}

/*****************************************************************************/

/**
 * @brief Return the difference of a period and a timestamp.
 */
static int
minus_period_timestamp1(const Period *p, TimestampTz t, Period **result)
{
  if (! contains_period_timestamp(p, t))
  {
    result[0] = span_copy(p);
    return 1;
  }

  if ((TimestampTz) p->lower == t && (TimestampTz) p->upper == t)
    return 0;

  if ((TimestampTz) p->lower == t)
  {
    result[0] = span_make(p->lower, p->upper, false, p->upper_inc,
      T_TIMESTAMPTZ);
    return 1;
  }

  if ((TimestampTz) p->upper == t)
  {
    result[0] = span_make(p->lower, p->upper, p->lower_inc, false,
      T_TIMESTAMPTZ);
    return 1;
  }

  result[0] = span_make(p->lower, t, p->lower_inc, false, T_TIMESTAMPTZ);
  result[1] = span_make(t, p->upper, false, p->upper_inc, T_TIMESTAMPTZ);
  return 2;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period and a timestamp.
 */
PeriodSet *
minus_period_timestamp(const Period *p, TimestampTz t)
{
  Period *periods[2];
  int count = minus_period_timestamp1(p, t, periods);
  if (count == 0)
    return NULL;
  PeriodSet *result = periodset_make((const Period **) periods, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(periods[i]);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period and a timestamp set.
 */
PeriodSet *
minus_period_timestampset(const Period *p, const TimestampSet *ts)
{
  /* Transform the period into a period set */
  PeriodSet *ps = period_to_periodset(p);
  /* Bounding box test */
  const Period *p1 = timestampset_period_ptr(ts);
  if (! overlaps_span_span(p, p1))
    return ps;

  /* Call the function for the period set */
  PeriodSet *result = minus_periodset_timestampset(ps, ts);
  pfree(ps);
  return result;
}

/**
 * @brief Return the difference of the two periods.
 * @note This function generalizes the function minus_span_span by enabling
 * the result to be two periods
 */
static int
minus_period_period1(const Period *p1, const Period *p2, Period **result)
{
  SpanBound lower1, lower2, upper1, upper2;

  span_deserialize((const Span *) p1, &lower1, &upper1);
  span_deserialize((const Span *) p2, &lower2, &upper2);

  int cmp_l1l2 = span_bound_cmp(&lower1, &lower2);
  int cmp_l1u2 = span_bound_cmp(&lower1, &upper2);
  int cmp_u1l2 = span_bound_cmp(&upper1, &lower2);
  int cmp_u1u2 = span_bound_cmp(&upper1, &upper2);

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
    result[0] = span_make(p1->lower, p2->lower, p1->lower_inc,
      !(p2->lower_inc), T_TIMESTAMPTZ);
    result[1] = span_make(p2->upper, p1->upper, !(p2->upper_inc),
      p1->upper_inc, T_TIMESTAMPTZ);
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
    result[0] = span_copy(p1);

  /*
   * p1           |-----|
   * p2               |----|
   * result       |---|
   */
  else if (cmp_l1l2 <= 0 && cmp_u1u2 <= 0)
    result[0] = span_make(p1->lower, p2->lower, p1->lower_inc,
      !(p2->lower_inc), T_TIMESTAMPTZ);
  /*
   * p1         |-----|
   * p2      |----|
   * result       |---|
   */
  else if (cmp_l1l2 >= 0 && cmp_u1u2 >= 0)
    result[0] = span_make(p2->upper, p1->upper, !(p2->upper_inc),
      p1->upper_inc, T_TIMESTAMPTZ);
  return 1;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the periods.
 */
PeriodSet *
minus_period_period(const Period *p1, const Period *p2)
{
  Period *periods[2];
  int count = minus_period_period1(p1, p2, periods);
  if (count == 0)
    return NULL;
  PeriodSet *result = periodset_make((const Period **) periods, count,
    NORMALIZE_NO);
  for (int i = 0; i < count; i++)
    pfree(periods[i]);
  return result;
}

/**
 * Return the difference of a period and a period set.
 */
int
minus_period_periodset1(Period **result, const Period *p, const PeriodSet *ps,
  int from, int to)
{
  /* The period can be split at most into (to - from + 1) periods
   *   |----------------------|
   *       |---| |---| |---|
   */
  Period *curr = span_copy(p);
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
    int countminus = minus_period_period1(curr, p1, minus);
    pfree(curr);
    /* minus can have from 0 to 2 periods */
    if (countminus == 0)
      break;
    else if (countminus == 1)
      curr = minus[0];
    else /* countminus == 2 */
    {
      result[k++] = span_copy(minus[0]);
      curr = minus[1];
    }
    /* There are no more periods left */
    if (i == to - 1)
      result[k++] = curr;
  }
  return k;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period and a period set.
 */
PeriodSet *
minus_period_periodset(const Period *p, const PeriodSet *ps)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  if (! overlaps_span_span(p, p1))
    return periodset_make((const Period **) &p, 1, false);

  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int count = minus_period_periodset1(periods, p, ps, 0, ps->count);
  PeriodSet *result = periodset_make_free(periods, count, false);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period set and a timestamp.
 */
PeriodSet *
minus_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  /* Bounding box test */
  const Period *p = periodset_period_ptr(ps);
  if (! contains_period_timestamp(p, t))
    return periodset_copy(ps);

  /* At most one composing period can be split into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int k = 0;
  for (int i = 0; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    k += minus_period_timestamp1(p, t, &periods[k]);
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period set and a timestamp set.
 */
PeriodSet *
minus_periodset_timestampset(const PeriodSet *ps, const TimestampSet *ts)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  const Period *p2 = timestampset_period_ptr(ts);
  if (! overlaps_span_span(p1, p2))
    return periodset_copy(ps);

  /* Each timestamp will split at most one composing period into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + ts->count + 1));
  int i = 0, j = 0, k = 0;
  Period *curr = span_copy(periodset_per_n(ps, 0));
  TimestampTz t = timestampset_time_n(ts, 0);
  while (i < ps->count && j < ts->count)
  {
    if (t > (TimestampTz) curr->upper)
    {
      periods[k++] = curr;
      i++;
      if (i == ps->count)
        break;
      else
        curr = span_copy(periodset_per_n(ps, i));
    }
    else if (t < (TimestampTz) curr->lower)
    {
      j++;
      if (j == ts->count)
        break;
      else
        t = timestampset_time_n(ts, j);
    }
    else
    {
      if (contains_period_timestamp(curr, t))
      {
        if (curr->lower == curr->upper)
        {
          pfree(curr);
          i++;
          if (i == ps->count)
            break;
          else
            curr = span_copy(periodset_per_n(ps, i));
        }
        else if ((TimestampTz) curr->lower == t)
        {
          Period *curr1 = span_make(curr->lower, curr->upper, false,
            curr->upper_inc, T_TIMESTAMPTZ);
          pfree(curr);
          curr = curr1;
        }
        else if ((TimestampTz) curr->upper == t)
        {
          periods[k++] = span_make(curr->lower, curr->upper, curr->lower_inc,
            false, T_TIMESTAMPTZ);
          pfree(curr);
          i++;
          if (i == ps->count)
            break;
          else
            curr = span_copy(periodset_per_n(ps, i));
        }
        else
        {
          periods[k++] = span_make(curr->lower, t, curr->lower_inc, false,
            T_TIMESTAMPTZ);
          Period *curr1 = span_make(t, curr->upper, false, curr->upper_inc,
            T_TIMESTAMPTZ);
          pfree(curr);
          curr = curr1;
        }
      }
      else
      {
        if ((TimestampTz) curr->upper == t)
        {
          periods[k++] = curr;
          i++;
          if (i == ps->count)
            break;
          else
            curr = span_copy(periodset_per_n(ps, i));
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
  pfree_array((void **) periods, i);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of a period set and a period.
 */
PeriodSet *
minus_periodset_period(const PeriodSet *ps, const Period *p)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps);
  if (! overlaps_span_span(p1, p))
    return periodset_copy(ps);

  /* At most one composing period can be split into two */
  Period **periods = palloc(sizeof(Period *) * (ps->count + 1));
  int k = 0;
  for (int i = 0; i < ps->count; i++)
  {
    p1 = periodset_per_n(ps, i);
    k += minus_period_period1(p1, p, &periods[k]);
  }
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_spantime_set
 * @brief Return the difference of the period sets.
 */
PeriodSet *
minus_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  /* Bounding box test */
  const Period *p1 = periodset_period_ptr(ps1);
  const Period *p2 = periodset_period_ptr(ps2);
  if (! overlaps_span_span(p1, p2))
    return periodset_copy(ps1);

  Period **periods = palloc(sizeof(const Period *) * (ps1->count + ps2->count));
  int i = 0, j = 0, k = 0;
  while (i < ps1->count && j < ps2->count)
  {
    p1 = periodset_per_n(ps1, i);
    p2 = periodset_per_n(ps2, j);
    /* The periods do not overlap, copy the first period */
    if (! overlaps_span_span(p1, p2))
    {
      periods[k++] = span_copy(p1);
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
        if (! overlaps_span_span(p1, p3))
          break;
      }
      int to = Min(l, ps2->count);
      /* Compute the difference of the overlapping periods */
      k += minus_period_periodset1(&periods[k], p1,
        ps2, j, to);
      i++;
      j = l;
    }
  }
  /* Copy the sequences after the period set */
  while (i < ps1->count)
    periods[k++] = span_copy(periodset_per_n(ps1, i++));
  PeriodSet *result = periodset_make_free(periods, k, NORMALIZE_NO);
  return result;
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between the timestamps
 */
double
distance_timestamp_timestamp(TimestampTz t1, TimestampTz t2)
{
  double result;
  if (t1 < t2)
    result = ((float8) t2 - (float8) t1) / USECS_PER_SEC;
  else
    result = ((float8) t1 - (float8) t2) / USECS_PER_SEC;
  return result;
}

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a timestamp set.
 */
double
distance_timestamp_timestampset(TimestampTz t, const TimestampSet *ts)
{
  Period p;
  timestampset_set_period(ts, &p);
  double result = distance_period_timestamp(&p, t);
  return result;
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a period.
 */
double
distance_timestamp_period(TimestampTz t, const Period *p)
{
  return distance_period_timestamp(p, t);
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp and a period set
 */
double
distance_timestamp_periodset(TimestampTz t, const PeriodSet *ps)
{
  Period p;
  periodset_set_period(ps, &p);
  double result = distance_period_timestamp(&p, t);
  return result;
}
#endif

/******************************************************************************/

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a timestamp
 */
double
distance_timestampset_timestamp(const TimestampSet *ts, TimestampTz t)
{
  Period p;
  timestampset_set_period(ts, &p);
  double result = distance_period_timestamp(&p, t);
  return result;
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between the timestamp sets
 */
double
distance_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2)
{
  Period p1, p2;
  timestampset_set_period(ts1, &p1);
  timestampset_set_period(ts2, &p2);
  double result = distance_span_span(&p1, &p2);
  return result;
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a period.
 */
double
distance_timestampset_period(const TimestampSet *ts, const Period *p)
{
  Period p1;
  timestampset_set_period(ts, &p1);
  double result = distance_span_span(&p1, p);
  return result;
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a timestamp set and a period set
 */
double
distance_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps)
{
  Period p1, p2;
  timestampset_set_period(ts, &p1);
  periodset_set_period(ps, &p2);
  double result = distance_span_span(&p1, &p2);
  return result;
}
#endif

/******************************************************************************/

/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period and a timestamp.
 */
double
distance_period_timestamp(const Period *p, TimestampTz t)
{
  /* If the periods intersect return 0 */
  if (contains_period_timestamp(p, t))
    return 0.0;

  /* If the period is to the left of a timestamp return the distance
   * between the upper bound of the period and a timestamp */
  if ((TimestampTz) p->lower > t)
    return ((float8) p->lower - (float8) t) / USECS_PER_SEC;

  /* If the first period is to the right of the seconde return the distance
   * between the upper bound of the second and lower bound of the first */
    return ((float8) t - (float8) p->upper) / USECS_PER_SEC;
}

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period and a timestamp set
 */
double
distance_period_timestampset(const Period *p, const TimestampSet *ts)
{
  return distance_timestampset_period(ts, p);
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period and a period set
 */
double
distance_period_periodset(const Period *p, const PeriodSet *ps)
{
  Period p1;
  periodset_set_period(ps, &p1);
  double result = distance_span_span(&p1, p);
  return result;
}
#endif

/******************************************************************************/

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period set and a timestamp
 */
double
distance_periodset_timestamp(const PeriodSet *ps, TimestampTz t)
{
  return distance_timestamp_periodset(t, ps);
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period set and a timestamp set
 */
double
distance_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts)
{
  return distance_timestampset_periodset(ts, ps);
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between a period set and a period
 */
double
distance_periodset_period(const PeriodSet *ps, const Period *p)
{
  return distance_period_periodset(p, ps);
}
#endif

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_dist
 * @brief Return the distance in seconds between the period sets
 */
double
distance_periodset_periodset(const PeriodSet *ps1, const PeriodSet *ps2)
{
  Period p1, p2;
  periodset_set_period(ps1, &p1);
  periodset_set_period(ps2, &p2);
  double result = distance_span_span(&p1, &p2);
  return result;
}
#endif

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/**
 * Return the size in bytes to read from toast to get the basic information
 * from a variable-length time type: Time struct (i.e., TimestampSet
 * or PeriodSet) and bounding box size
*/
uint32_t
time_max_header_size(void)
{
  return double_pad(Max(sizeof(TimestampSet), sizeof(PeriodSet)));
}


/*****************************************************************************/
/* contains? */

PG_FUNCTION_INFO_V1(Contains_timestampset_timestamp);
/**
 * Return true if a timestamp set contains a timestamp
 */
PGDLLEXPORT Datum
Contains_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = contains_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_timestampset_timestampset);
/**
 * Return true if the first timestamp set contains the second one
 */
PGDLLEXPORT Datum
Contains_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contains_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_period_timestamp);
/**
 * Return true if a period contains a timestamp
 */
PGDLLEXPORT Datum
Contains_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(contains_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Contains_period_timestampset);
/**
 * Return true if a period contains a timestamp set
 */
PGDLLEXPORT Datum
Contains_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contains_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_period_period);
/**
 * Return true if a period contains a period
 */
PGDLLEXPORT Datum
Contains_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contains_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Contains_periodset_timestamp);
/**
 * Return true if a period set contains a timestamp
 */
PGDLLEXPORT Datum
Contains_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = contains_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_periodset_timestampset);
/**
 * Return true if a period set contains a timestamp set
 */
PGDLLEXPORT Datum
Contains_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contains_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_periodset_period);
/**
 * Return true if a period set contains a period set
 */
PGDLLEXPORT Datum
Contains_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = contains_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_period_periodset);
/**
 * Return true if a period contains a period set
 */
PGDLLEXPORT Datum
Contains_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contains_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contains_periodset_periodset);
/**
 * Return true if the first period set contains the second one
 */
PGDLLEXPORT Datum
Contains_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = contains_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* contained? */

PG_FUNCTION_INFO_V1(Contained_timestamp_timestampset);
/**
 * Return true if a timestamp is contained by a timestamp set
 */
PGDLLEXPORT Datum
Contained_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contained_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestamp_period);
/**
 * Return true if a timestamp is contained by a period
 */
PGDLLEXPORT Datum
Contained_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Contained_timestamp_periodset);
/**
 * Return true if a timestamp is contained by a period set
 */
PGDLLEXPORT Datum
Contained_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contained_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_timestampset);
/**
 * Return true if the first timestamp set is contained by the second one
 */
PGDLLEXPORT Datum
Contained_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = contained_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_period);
/**
 * Return true if a timestamp set is contained by a period
 */
PGDLLEXPORT Datum
Contained_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = contained_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_timestampset_periodset);
/**
 * Return true if a timestamp set is contained by a period set
 */
PGDLLEXPORT Datum
Contained_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contained_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_period_period);
/**
 * Return true if a period is contained by the second one
 */
PGDLLEXPORT Datum
Contained_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(contained_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Contained_period_periodset);
/**
 * Return true if a period is contained by a period set
 */
PGDLLEXPORT Datum
Contained_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = contained_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_periodset_period);
/**
 * Return true if a period set is contained by a period
 */
PGDLLEXPORT Datum
Contained_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = contained_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Contained_periodset_periodset);
/**
 * Return true if the first period set is contained by the second one
 */
PGDLLEXPORT Datum
Contained_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = contained_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* overlaps? */

PG_FUNCTION_INFO_V1(Overlaps_timestampset_timestampset);
/**
 * Return true if the timestamp sets overlap
 */
PGDLLEXPORT Datum
Overlaps_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overlaps_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_timestampset_period);
/**
 * Return true if a timestamp set and a period overlap
 */
PGDLLEXPORT Datum
Overlaps_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overlaps_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_timestampset_periodset);
/**
 * Return true if a timestamp set and a period set overlap
 */
PGDLLEXPORT Datum
Overlaps_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overlaps_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_period_timestampset);
/**
 * Return true if a period and a timestamp set overlap
 */
PGDLLEXPORT Datum
Overlaps_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overlaps_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_period_period);
/**
 * Return true if the periods overlap
 */
PGDLLEXPORT Datum
Overlaps_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overlaps_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Overlaps_period_periodset);
/**
 * Return true if a period and a period set overlap
 */
PGDLLEXPORT Datum
Overlaps_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overlaps_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_periodset_timestampset);
/**
 * Return true if a period set and a timestamp set overlap
 */
PGDLLEXPORT Datum
Overlaps_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overlaps_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_periodset_period);
/**
 * Return true if a period set and a period overlap
 */
PGDLLEXPORT Datum
Overlaps_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overlaps_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overlaps_periodset_periodset);
/**
 * Return true if the period sets overlap
 */
PGDLLEXPORT Datum
Overlaps_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = overlaps_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* adjacent to (but not overlapping)? */

PG_FUNCTION_INFO_V1(Adjacent_timestamp_period);
/**
 * Return true if a timestamp and a period are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Adjacent_timestamp_periodset);
/**
 * Return true if a timestamp and a period set are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_timestampset_period);
/**
 * Return true if a timestamp set and a period are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = adjacent_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_timestampset_periodset);
/**
 * Return true if a timestamp set and a period set are adjacent
 */
PGDLLEXPORT Datum
Adjacent_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_period_timestamp);
/**
 * Return true if a period and a timestamp are adjacent
 */
PGDLLEXPORT Datum
Adjacent_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(adjacent_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Adjacent_period_timestampset);
/**
 * Return true if a period and a timestamp set are adjacent
 */
PGDLLEXPORT Datum
Adjacent_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = adjacent_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_period_period);
/**
 * Return true if the periods are adjacent
 */
PGDLLEXPORT Datum
Adjacent_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(adjacent_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Adjacent_period_periodset);
/**
 * Return true if a period and a period set are adjacent
 */
PGDLLEXPORT Datum
Adjacent_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_timestamp);
/**
 * Return true if a period set and a timestamp are adjacent
 */
PGDLLEXPORT Datum
Adjacent_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = adjacent_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_timestampset);
/**
 * Return true if a period set and a timestamp set are adjacent
 */
PGDLLEXPORT Datum
Adjacent_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = adjacent_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_period);
/**
 * Return true if a period set and a period are adjacent
 */
PGDLLEXPORT Datum
Adjacent_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = adjacent_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Adjacent_periodset_periodset);
/**
 * Return true if the period sets are adjacent
 */
PGDLLEXPORT Datum
Adjacent_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = adjacent_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly before of? */

PG_FUNCTION_INFO_V1(Before_timestamp_timestampset);
/**
 * Return true if a timestamp is strictly before a timestamp set
 */
PGDLLEXPORT Datum
Before_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestamp_period);
/**
 * Return true if a timestamp is strictly before a period
 */
PGDLLEXPORT Datum
Before_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(before_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Before_timestamp_periodset);
/**
 * Return true if a timestamp is strictly before a period set
 */
PGDLLEXPORT Datum
Before_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = before_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_timestamp);
/**
 * Return true if a timestamp set is strictly before a timestamp
 */
PGDLLEXPORT Datum
Before_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = before_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_timestampset);
/**
 * Return true if the first timestamp set is strictly before the second one
 */
PGDLLEXPORT Datum
Before_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_period);
/**
 * Return true if a timestamp set is strictly before a period
 */
PGDLLEXPORT Datum
Before_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = before_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_timestampset_periodset);
/**
 * Return true if a timestamp set is strictly before a period set
 */
PGDLLEXPORT Datum
Before_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = before_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_period_timestamp);
/**
 * Return true if a period is strictly before a timestamp
 */
PGDLLEXPORT Datum
Before_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(before_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Before_period_timestampset);
/**
 * Return true if a period is strictly before a timestamp set
 */
PGDLLEXPORT Datum
Before_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_period_period);
/**
 * Return true if a period is strictly before the second one
 */
PGDLLEXPORT Datum
Before_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(left_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Before_period_periodset);
/**
 * Return true if a period is strictly before a period set
 */
PGDLLEXPORT Datum
Before_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = before_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_timestamp);
/**
 * Return true if a period set is strictly before a timestamp
 */
PGDLLEXPORT Datum
Before_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = before_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_timestampset);
/**
 * Return true if a period set is strictly before a timestamp set
 */
PGDLLEXPORT Datum
Before_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = before_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_period);
/**
 * Return true if a period set is strictly before a period
 */
PGDLLEXPORT Datum
Before_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = before_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Before_periodset_periodset);
/**
 * Return true if the first period set is strictly before the second one
 */
PGDLLEXPORT Datum
Before_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = before_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* strictly after of? */

PG_FUNCTION_INFO_V1(After_timestamp_timestampset);
/**
 * Return true if a timestamp is strictly after a timestamp set
 */
PGDLLEXPORT Datum
After_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestamp_period);
/**
 * Return true if a timestamp is strictly after a period
 */
PGDLLEXPORT Datum
After_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(after_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(After_timestamp_periodset);
/**
 * Return true if a timestamp is strictly after a period set
 */
PGDLLEXPORT Datum
After_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = after_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_timestamp);
/**
 * Return true if a timestamp set is strictly after a timestamp
 */
PGDLLEXPORT Datum
After_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = after_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_timestampset);
/**
 * Return true if the first timestamp set is strictly after the second one
 */
PGDLLEXPORT Datum
After_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_period);
/**
 * Return true if a timestamp set is strictly after a period
 */
PGDLLEXPORT Datum
After_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = after_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_timestampset_periodset);
/**
 * Return true if a timestamp set is strictly after a period set
 */
PGDLLEXPORT Datum
After_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = after_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_period_timestamp);
/**
 * Return true if a period is strictly after a timestamp
 */
PGDLLEXPORT Datum
After_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(after_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(After_period_timestampset);
/**
 * Return true if a period is strictly after a timestamp set
 */
PGDLLEXPORT Datum
After_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_period_period);
/**
 * Return true if the first period is strictly after the second one
 */
PGDLLEXPORT Datum
After_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(right_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(After_period_periodset);
/**
 * Return true if a period is strictly after a period set
 */
PGDLLEXPORT Datum
After_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = after_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_timestamp);
/**
 * Return true if a period set is strictly after a timestamp
 */
PGDLLEXPORT Datum
After_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = after_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_timestampset);
/**
 * Return true if a period set is strictly after a timestamp set
 */
PGDLLEXPORT Datum
After_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = after_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_period);
/**
 * Return true if a period set is strictly after a period
 */
PGDLLEXPORT Datum
After_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = after_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(After_periodset_periodset);
/**
 * Return true if the first period set is strictly after the second one
 */
PGDLLEXPORT Datum
After_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = after_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to right of? */

PG_FUNCTION_INFO_V1(Overbefore_timestamp_timestampset);
/**
 * Return true if a timestamp is not after a timestamp set
 */
PGDLLEXPORT Datum
Overbefore_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestamp_period);
/**
 * Return true if a timestamp is not after a period
 */
PGDLLEXPORT Datum
Overbefore_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overbefore_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Overbefore_timestamp_periodset);
/**
 * Return true if a timestamp is not after a period set
 */
PGDLLEXPORT Datum
Overbefore_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_timestamp);
/**
 * Return true if a timestamp is not after a timestamp
 */
PGDLLEXPORT Datum
Overbefore_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overbefore_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_timestampset);
/**
 * Return true if the first timestamp set is not after the second one
 */
PGDLLEXPORT Datum
Overbefore_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_period);
/**
 * Return true if a timestamp set is not after a period
 */
PGDLLEXPORT Datum
Overbefore_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overbefore_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_timestampset_periodset);
/**
 * Return true if a timestamp set is not after a period set
 */
PGDLLEXPORT Datum
Overbefore_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_period_timestamp);
/**
 * Return true if a period is not after a timestamp
 */
PGDLLEXPORT Datum
Overbefore_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overbefore_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Overbefore_period_timestampset);
/**
 * Return true if a period is not after a timestamp set
 */
PGDLLEXPORT Datum
Overbefore_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_period_period);
/**
 * Return true if the first period is not after the second one
 */
PGDLLEXPORT Datum
Overbefore_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overleft_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Overbefore_period_periodset);
/**
 * Return true if a period is not after a period set
 */
PGDLLEXPORT Datum
Overbefore_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_timestamp);
/**
 * Return true if a period set is not after a timestamp
 */
PGDLLEXPORT Datum
Overbefore_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overbefore_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_timestampset);
/**
 * Return true if a period set is not after a timestamp set
 */
PGDLLEXPORT Datum
Overbefore_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overbefore_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_period);
/**
 * Return true if a period set is not after a period set
 */
PGDLLEXPORT Datum
Overbefore_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overbefore_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overbefore_periodset_periodset);
/**
 * Return true if the first period set is not after the second one
 */
PGDLLEXPORT Datum
Overbefore_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = overbefore_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* does not extend to left of? */

PG_FUNCTION_INFO_V1(Overafter_timestamp_timestampset);
/**
 * Return true if a timestamp is not before a timestamp set
 */
PGDLLEXPORT Datum
Overafter_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestamp_period);
/**
 * Return true if a timestamp is not before a period
 */
PGDLLEXPORT Datum
Overafter_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overafter_timestamp_period(t, p));
}

PG_FUNCTION_INFO_V1(Overafter_timestamp_periodset);
/**
 * Return true if a timestamp is not before a period set
 */
PGDLLEXPORT Datum
Overafter_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_timestamp);
/**
 * Return true if a timestamp set is not before a timestamp
 */
PGDLLEXPORT Datum
Overafter_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overafter_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_timestampset);
/**
 * Return true if the first timestamp set is not before the second one
 */
PGDLLEXPORT Datum
Overafter_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_period);
/**
 * Return true if a timestamp set is not before a period
 */
PGDLLEXPORT Datum
Overafter_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overafter_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_timestampset_periodset);
/**
 * Return true if a timestamp set is not before a period set
 */
PGDLLEXPORT Datum
Overafter_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_period_timestamp);
/**
 * Return true if a period is not before a timestamp
 */
PGDLLEXPORT Datum
Overafter_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_BOOL(overafter_period_timestamp(p, t));
}

PG_FUNCTION_INFO_V1(Overafter_period_timestampset);
/**
 * Return true if a period is not before a timestamp set
 */
PGDLLEXPORT Datum
Overafter_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_period_period);
/**
 * Return true if the first period is not before the second one
 */
PGDLLEXPORT Datum
Overafter_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(overright_span_span(p1, p2));
}

PG_FUNCTION_INFO_V1(Overafter_period_periodset);
/**
 * Return true if a period is not before a period set
 */
PGDLLEXPORT Datum
Overafter_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_timestamp);
/**
 * Return true if a period set is not before a timestamp
 */
PGDLLEXPORT Datum
Overafter_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  bool result = overafter_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_timestampset);
/**
 * Return true if a period set is not before a timestamp set
 */
PGDLLEXPORT Datum
Overafter_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = overafter_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_period);
/**
 * Return true if a period set is not before a period
 */
PGDLLEXPORT Datum
Overafter_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  bool result = overafter_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Overafter_periodset_periodset);
/**
 * Return true if the first period set is not before the second one
 */
PGDLLEXPORT Datum
Overafter_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  bool result = overafter_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Set union
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Union_timestamp_timestamp);
/**
 * Return the union of the timestamps
 */
PGDLLEXPORT Datum
Union_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result = union_timestamp_timestamp(t1, t2);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestamp_timestampset);
/**
 * Return the union of a timestamp and a timestamp set
 */
PGDLLEXPORT Datum
Union_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = union_timestamp_timestampset(t, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestamp_period);
/**
 * Return the union of a timestamp and a period
 */
PGDLLEXPORT Datum
Union_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = union_timestamp_period(t, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestamp_periodset);
/**
 * Return the union of a timestamp and a period set
 */
PGDLLEXPORT Datum
Union_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_timestamp_periodset(t, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_timestampset_timestamp);
/**
 * Return the union of a timestamp set and a timestamp
 */
PGDLLEXPORT Datum
Union_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result = union_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestampset_timestampset);
/**
 * Return the union of the timestamp sets
 */
PGDLLEXPORT Datum
Union_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = union_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestampset_period);
/**
 * Return the union of a timestamp set and a period
 */
PGDLLEXPORT Datum
Union_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = union_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_timestampset_periodset);
/**
 * Return the union of a timestamp set and a period set
 */
PGDLLEXPORT Datum
Union_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_period_timestamp);
/**
 * Return the union of a period and a timestamp
 */
PGDLLEXPORT Datum
Union_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = union_period_timestamp(p, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_period_timestampset);
/**
 * Return the union of a period and a timestamp set
 */
PGDLLEXPORT Datum
Union_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = union_period_timestampset(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_period_period);
/**
 * Return the union of the periods
 */
PGDLLEXPORT Datum
Union_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_POINTER(union_period_period(p1, p2));
}

PG_FUNCTION_INFO_V1(Union_period_periodset);
/**
 * Return the union of a period and a period set
 */
PGDLLEXPORT Datum
Union_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Union_periodset_timestamp);
/**
 * Return the union of a period set and a timestamp
 */
PGDLLEXPORT Datum
Union_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = union_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_periodset_timestampset);
/**
 * Return the union of a period set and a timestamp set
 */
PGDLLEXPORT Datum
Union_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = union_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_periodset_period);
/**
 * Return the union of a period set and a period
 */
PGDLLEXPORT Datum
Union_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = union_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Union_periodset_periodset);
/**
 * Return the union of the period sets
 */
PGDLLEXPORT Datum
Union_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = union_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set intersection
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_timestamp_timestamp);
/**
 * Return the intersection of the timestamps
 */
PGDLLEXPORT Datum
Intersection_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_timestamp_timestamp(t1, t2, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestamp_timestampset);
/**
 * Return the intersection of a timestamp and a timestamp set
 */
PGDLLEXPORT Datum
Intersection_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampTz result;
  bool found = intersection_timestamp_timestampset(t, ts, &result);
  PG_FREE_IF_COPY(ts, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestamp_period);
/**
 * Return the intersection of a timestamp and a period
 */
PGDLLEXPORT Datum
Intersection_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampTz result;
  bool found = intersection_timestamp_period(t, p, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestamp_periodset);
/**
 * Return the intersection of a timestamp and a period set
 */
PGDLLEXPORT Datum
Intersection_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampTz result;
  bool found = intersection_timestamp_periodset(t, ps, &result);
  PG_FREE_IF_COPY(ps, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_timestamp);
/**
 * Return the intersection of a timestamp set and a timestamp
 */
PGDLLEXPORT Datum
Intersection_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_timestampset_timestamp(ts, t, &result);
  PG_FREE_IF_COPY(ts, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_timestampset);
/**
 * Return the intersection of the timestamp sets
 */
PGDLLEXPORT Datum
Intersection_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = intersection_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_period);
/**
 * @brief Return the intersection of a timestamp set and a period.
 */
PGDLLEXPORT Datum
Intersection_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampSet *result = intersection_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_timestampset_periodset);
/**
 * Return the intersection of a timestamp set and a period set
 */
PGDLLEXPORT Datum
Intersection_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampSet *result = intersection_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_period_timestamp);
/**
 * Return the intersection of a period and a timestamp
 */
PGDLLEXPORT Datum
Intersection_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_period_timestamp(p, t, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_period_timestampset);
/**
 * Return the intersection of a period and a timestamp set
 */
PGDLLEXPORT Datum
Intersection_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = intersection_period_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_period_period);
/**
 * Return the intersection of the periods
 */
PGDLLEXPORT Datum
Intersection_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  Period *result = intersection_span_span(p1, p2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PG_FUNCTION_INFO_V1(Intersection_period_periodset);
/**
 * Return the intersection of a period and a period set
 */
PGDLLEXPORT Datum
Intersection_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = intersection_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Intersection_periodset_timestamp);
/**
 * Return the intersection of a period set and a timestamp
 */
PGDLLEXPORT Datum
Intersection_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  bool found = intersection_periodset_timestamp(ps, t, &result);
  PG_FREE_IF_COPY(ps, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Intersection_periodset_timestampset);
/**
 * Return the intersection of a period set and a timestamp set
 */
PGDLLEXPORT Datum
Intersection_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = intersection_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_periodset_period);
/**
 * Return the intersection of a period set and a period
 */
PGDLLEXPORT Datum
Intersection_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = intersection_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Intersection_periodset_periodset);
/**
 * Return the intersection of the period sets
 */
PGDLLEXPORT Datum
Intersection_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = intersection_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set difference
 * The functions produce new results that must be freed after
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_timestamp_timestamp);
/**
 * @brief Return the difference of the timestamps
 */
PGDLLEXPORT Datum
Minus_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  TimestampTz result;
  if (! minus_timestamp_timestamp(t1, t2, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Minus_timestamp_timestampset);
/**
 * Return the difference of a timestamp and a a timestamp set
 */
PGDLLEXPORT Datum
Minus_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampTz result;
  bool found = minus_timestamp_timestampset(t, ts, &result);
  PG_FREE_IF_COPY(ts, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Minus_timestamp_period);
/**
 * Return the difference of a timestamp and a period
 */
PGDLLEXPORT Datum
Minus_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampTz result;
  bool found = minus_timestamp_period(t, p, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Minus_timestamp_periodset);
/**
 * Return the difference of a timestamp and a period set
 */
PGDLLEXPORT Datum
Minus_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampTz result;
  bool found = minus_timestamp_periodset(t, ps, &result);
  PG_FREE_IF_COPY(ps, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_timestampset_timestamp);
/**
 * Return the difference of a timestamp set and a timestamp
 */
PGDLLEXPORT Datum
Minus_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  TimestampSet *result = minus_timestampset_timestamp(ts, t);
  PG_FREE_IF_COPY(ts, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_timestampset_timestampset);
/**
 * Return the difference of the timestamp sets
 */
PGDLLEXPORT Datum
Minus_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  TimestampSet *result = minus_timestampset_timestampset(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  if (result == NULL)
    PG_RETURN_NULL() ;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_timestampset_period);
/**
 * Return the difference of a timestamp set and period
 */
PGDLLEXPORT Datum
Minus_timestampset_period(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  TimestampSet *result = minus_timestampset_period(ts, p);
  PG_FREE_IF_COPY(ts, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_timestampset_periodset);
/**
 * Return the difference of a timestamp set and a period set
 */
PGDLLEXPORT Datum
Minus_timestampset_periodset(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TimestampSet *result = minus_timestampset_periodset(ts, ps);
  PG_FREE_IF_COPY(ts, 0);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_period_timestamp);
/**
 * Return the difference of a period and a timestamp
 */
PGDLLEXPORT Datum
Minus_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_period_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_period_timestampset);
/**
 * Return the difference of a period and a timestamp set
 */
PGDLLEXPORT Datum
Minus_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *ps = PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = minus_period_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_period_period);
/**
 * @brief Return the difference of the periods.
 */
PGDLLEXPORT Datum
Minus_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  PeriodSet *result = minus_period_period(p1, p2);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_period_periodset);
/**
 * Return the difference of a period and a period set
 */
PGDLLEXPORT Datum
Minus_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = minus_period_periodset(p, ps);
  PG_FREE_IF_COPY(ps, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Minus_periodset_timestamp);
/**
 * Return the difference of a period set and a timestamp
 */
PGDLLEXPORT Datum
Minus_periodset_timestamp(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PeriodSet *result = minus_periodset_timestamp(ps, t);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_periodset_timestampset);
/**
 * Return the difference of a period set and a timestamp set
 */
PGDLLEXPORT Datum
Minus_periodset_timestampset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  PeriodSet *result = minus_periodset_timestampset(ps, ts);
  PG_FREE_IF_COPY(ps, 0);
  PG_FREE_IF_COPY(ts, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_periodset_period);
/**
 * Return the difference of a period set and a period
 */
PGDLLEXPORT Datum
Minus_periodset_period(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  Period *p = PG_GETARG_SPAN_P(1);
  PeriodSet *result = minus_periodset_period(ps, p);
  PG_FREE_IF_COPY(ps, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Minus_periodset_periodset);
/**
 * Return the difference of the period sets
 */
PGDLLEXPORT Datum
Minus_periodset_periodset(PG_FUNCTION_ARGS)
{
  PeriodSet *ps1 = PG_GETARG_PERIODSET_P(0);
  PeriodSet *ps2 = PG_GETARG_PERIODSET_P(1);
  PeriodSet *result = minus_periodset_periodset(ps1, ps2);
  PG_FREE_IF_COPY(ps1, 0);
  PG_FREE_IF_COPY(ps2, 1);
  if (! result)
    PG_RETURN_NULL() ;
  PG_RETURN_POINTER(result);
}

/******************************************************************************
 * Distance functions returning a double representing the number of seconds
 ******************************************************************************/

PG_FUNCTION_INFO_V1(Distance_timestamp_timestamp);
/**
 * Return the distance in seconds between the timestamps
 */
PGDLLEXPORT Datum
Distance_timestamp_timestamp(PG_FUNCTION_ARGS)
{
  TimestampTz t1 = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz t2 = PG_GETARG_TIMESTAMPTZ(1);
  double result = distance_timestamp_timestamp(t1, t2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_timestampset);
/**
 * Return the distance in seconds between a timestamp and a timestamp set
 */
PGDLLEXPORT Datum
Distance_timestamp_timestampset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p;
  timestampset_period_slice(ts, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_period);
/**
 * Return the distance in seconds between a timestamp and a period
 */
PGDLLEXPORT Datum
Distance_timestamp_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *p = PG_GETARG_SPAN_P(1);
  double result = distance_period_timestamp(p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestamp_periodset);
/**
 * Return the distance in seconds between a timestamp and a period set
 */
PGDLLEXPORT Datum
Distance_timestamp_periodset(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p;
  periodset_period_slice(ps, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_timestamp);
/**
 * Return the distance in seconds between a timestamp set and a timestamp
 */
PGDLLEXPORT Datum
Distance_timestampset_timestamp(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period p;
  timestampset_period_slice(ts, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_timestampset);
/**
 * Return the distance in seconds between the timestamp sets
 */
PGDLLEXPORT Datum
Distance_timestampset_timestampset(PG_FUNCTION_ARGS)
{
  Datum ts1 = PG_GETARG_DATUM(0);
  Datum ts2 = PG_GETARG_DATUM(1);
  Period p1, p2;
  timestampset_period_slice(ts1, &p1);
  timestampset_period_slice(ts2, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_period);
/**
 * Return the distance in seconds between a timestamp set and a period
 */
PGDLLEXPORT Datum
Distance_timestampset_period(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  Period *p = PG_GETARG_SPAN_P(1);
  Period p1;
  timestampset_period_slice(ts, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_timestampset_periodset);
/**
 * Return the distance in seconds between a timestamp set and a period set
 */
PGDLLEXPORT Datum
Distance_timestampset_periodset(PG_FUNCTION_ARGS)
{
  Datum ts = PG_GETARG_DATUM(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p1, p2;
  timestampset_period_slice(ts, &p1);
  periodset_period_slice(ps, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_timestamp);
/**
 * Return the distance in seconds between a period and a timestamp
 */
PGDLLEXPORT Datum
Distance_period_timestamp(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double result = distance_period_timestamp(p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_timestampset);
/**
 * Return the distance in seconds between a period and a timestamp set
 */
PGDLLEXPORT Datum
Distance_period_timestampset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p1;
  timestampset_period_slice(ts, &p1);
  double result = distance_span_span(p, &p1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_period);
/**
 * Return the distance in seconds between the periods
 */
PGDLLEXPORT Datum
Distance_period_period(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_SPAN_P(0);
  Period *p2 = PG_GETARG_SPAN_P(1);
  double result = distance_span_span(p1, p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_period_periodset);
/**
 * Return the distance in seconds between a period and a period set
 */
PGDLLEXPORT Datum
Distance_period_periodset(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Datum ps = PG_GETARG_DATUM(1);
  Period p1;
  periodset_period_slice(ps, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_timestamp);
/**
 * Return the distance in seconds between a period set and a timestamp
 */
PGDLLEXPORT Datum
Distance_periodset_timestamp(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Period p;
  periodset_period_slice(ps, &p);
  double result = distance_period_timestamp(&p, t);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_timestampset);
/**
 * Return the distance in seconds between a period set and a timestamp set
 */
PGDLLEXPORT Datum
Distance_periodset_timestampset(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  Datum ts = PG_GETARG_DATUM(1);
  Period p1, p2;
  periodset_period_slice(ps, &p1);
  timestampset_period_slice(ts, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_period);
/**
 * Return the distance in seconds between a period set and a period
 */
PGDLLEXPORT Datum
Distance_periodset_period(PG_FUNCTION_ARGS)
{
  Datum ps = PG_GETARG_DATUM(0);
  Period *p = PG_GETARG_SPAN_P(1);
  Period p1;
  periodset_period_slice(ps, &p1);
  double result = distance_span_span(&p1, p);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Distance_periodset_periodset);
/**
 * Return the distance in seconds between the period sets
 */
PGDLLEXPORT Datum
Distance_periodset_periodset(PG_FUNCTION_ARGS)
{
  Datum ps1 = PG_GETARG_DATUM(0);
  Datum ps2 = PG_GETARG_DATUM(1);
  Period p1, p2;
  periodset_period_slice(ps1, &p1);
  periodset_period_slice(ps2, &p2);
  double result = distance_span_span(&p1, &p2);
  PG_RETURN_FLOAT8(result);
}

#endif /* #ifndef MEOS */

/******************************************************************************/
