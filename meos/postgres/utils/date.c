/*-------------------------------------------------------------------------
 *
 * date.c
 *    implements DATE and TIME data types specified in SQL standard
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/date.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <limits.h>

#include "utils/timestamp_def.h"
#include "utils/date.h"
#include "utils/datetime.h"

#include "../../include/meos.h"

/*
 * gcc's -ffast-math switch breaks routines that expect exact results from
 * expressions like timeval / SECS_PER_HOUR, where timeval is double.
 */
#ifdef __FAST_MATH__
#error -ffast-math is known to break this code
#endif

/* time_overflows()
 * Check to see if a broken-down time-of-day is out of range.
 */
bool
time_overflows(int hour, int min, int sec, fsec_t fsec)
{
  /* Range-check the fields individually. */
  if (hour < 0 || hour > HOURS_PER_DAY ||
    min < 0 || min >= MINS_PER_HOUR ||
    sec < 0 || sec > SECS_PER_MINUTE ||
    fsec < 0 || fsec > USECS_PER_SEC)
    return true;

  /*
   * Because we allow, eg, hour = 24 or sec = 60, we must check separately
   * that the total time value doesn't exceed 24:00:00.
   */
  if ((((((hour * MINS_PER_HOUR + min) * SECS_PER_MINUTE)
       + sec) * USECS_PER_SEC) + fsec) > USECS_PER_DAY)
    return true;

  return false;
}

/* tm2time()
 * Convert a tm structure to a time data type.
 */
int
tm2time(struct pg_tm *tm, fsec_t fsec, TimeADT *result)
{
  *result = ((((tm->tm_hour * MINS_PER_HOUR + tm->tm_min) * SECS_PER_MINUTE) + tm->tm_sec)
         * USECS_PER_SEC) + fsec;
  return 0;
}

/* time2tm()
 * Convert time data type to POSIX time structure.
 *
 * For dates within the range of pg_time_t, convert to the local time zone.
 * If out of this range, leave as UTC (in practice that could only happen
 * if pg_time_t is just 32 bits) - thomas 97/05/27
 */
int
time2tm(TimeADT time, struct pg_tm *tm, fsec_t *fsec)
{
  tm->tm_hour = time / USECS_PER_HOUR;
  time -= tm->tm_hour * USECS_PER_HOUR;
  tm->tm_min = time / USECS_PER_MINUTE;
  time -= tm->tm_min * USECS_PER_MINUTE;
  tm->tm_sec = time / USECS_PER_SEC;
  time -= tm->tm_sec * USECS_PER_SEC;
  *fsec = time;
  return 0;
}

/*
 * Promote date to timestamp with time zone.
 *
 * On successful conversion, *overflow is set to zero if it's not NULL.
 *
 * If the date is finite but out of the valid range for timestamptz, then:
 * if overflow is NULL, we throw an out-of-range error.
 * if overflow is not NULL, we store +1 or -1 there to indicate the sign
 * of the overflow, and return the appropriate timestamptz infinity.
 */
TimestampTz
date2timestamptz_opt_overflow(DateADT dateVal, int *overflow)
{
  TimestampTz result;
  struct pg_tm tt, *tm = &tt;
  int tz;

  if (overflow)
    *overflow = 0;

  if (DATE_IS_NOBEGIN(dateVal))
    TIMESTAMP_NOBEGIN(result);
  else if (DATE_IS_NOEND(dateVal))
    TIMESTAMP_NOEND(result);
  else
  {
    /*
     * Since dates have the same minimum values as timestamps, only upper
     * boundary need be checked for overflow.
     */
    if (dateVal >= (TIMESTAMP_END_JULIAN - POSTGRES_EPOCH_JDATE))
    {
      if (overflow)
      {
        *overflow = 1;
        TIMESTAMP_NOEND(result);
        return result;
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "date out of range for timestamp");
        return 0;
      }
    }

    j2date(dateVal + POSTGRES_EPOCH_JDATE,
         &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    tz = DetermineTimeZoneOffset(tm, session_timezone);

    result = dateVal * USECS_PER_DAY + tz * USECS_PER_SEC;

    /*
     * Since it is possible to go beyond allowed timestamptz range because
     * of time zone, check for allowed timestamp range after adding tz.
     */
    if (!IS_VALID_TIMESTAMP(result))
    {
      if (overflow)
      {
        if (result < MIN_TIMESTAMP)
        {
          *overflow = -1;
          TIMESTAMP_NOBEGIN(result);
        }
        else
        {
          *overflow = 1;
          TIMESTAMP_NOEND(result);
        }
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "date out of range for timestamp");
        return 0;
      }
    }
  }
  return result;
}

/*
 * Promote date to timestamptz, throwing error for overflow.
 */
TimestampTz
date2timestamptz(DateADT dateVal)
{
  return date2timestamptz_opt_overflow(dateVal, NULL);
}
