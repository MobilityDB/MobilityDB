/*-------------------------------------------------------------------------
 *
 * timestamp.c
 *    Functions for the built-in SQL types "timestamp" and "interval".
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/timestamp.c
 *
 *-------------------------------------------------------------------------
 */

#include <limits.h>

#include "postgres.h"

// MobilityDB
// #include "datatype/timestamp.h"
#include "utils/timestamp_def.h"
#include "utils/datetime.h"

#define SAMESIGN(a,b)	(((a) < 0) == ((b) < 0))

static TimeOffset time2t(const int hour, const int min, const int sec, const fsec_t fsec);
static Timestamp dt2local(Timestamp dt, int timezone);

/* Needed for gettimeofday */
#include <sys/time.h>

/* EncodeSpecialTimestamp()
 * Convert reserved timestamp data type to string.
 */
void
EncodeSpecialTimestamp(Timestamp dt, char *str)
{
  if (TIMESTAMP_IS_NOBEGIN(dt))
    strcpy(str, EARLY);
  else if (TIMESTAMP_IS_NOEND(dt))
    strcpy(str, LATE);
  else            /* shouldn't happen */
    elog(ERROR, "invalid argument for EncodeSpecialTimestamp");
}

/*
 * GetCurrentTimestamp -- get the current operating system time
 *
 * Result is in the form of a TimestampTz value, and is expressed to the
 * full precision of the gettimeofday() syscall
 */
TimestampTz
GetCurrentTimestamp(void)
{
  TimestampTz result;
  struct timeval tp;

  gettimeofday(&tp, NULL);

  result = (TimestampTz) tp.tv_sec -
    ((POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY);
  result = (result * USECS_PER_SEC) + tp.tv_usec;

  return result;
}

void
dt2time(Timestamp jd, int *hour, int *min, int *sec, fsec_t *fsec)
{
  TimeOffset  time;

  time = jd;

  *hour = time / USECS_PER_HOUR;
  time -= (*hour) * USECS_PER_HOUR;
  *min = time / USECS_PER_MINUTE;
  time -= (*min) * USECS_PER_MINUTE;
  *sec = time / USECS_PER_SEC;
  *fsec = time - (*sec * USECS_PER_SEC);
}                /* dt2time() */

/*
 * timestamp2tm() - Convert timestamp data type to POSIX time structure.
 *
 * Note that year is _not_ 1900-based, but is an explicit full value.
 * Also, month is one-based, _not_ zero-based.
 * Returns:
 *   0 on success
 *  -1 on out of range
 *
 * If attimezone is NULL, the global timezone setting will be used.
 */
int
timestamp2tm(Timestamp dt, int *tzp, struct pg_tm *tm, fsec_t *fsec, const char **tzn, pg_tz *attimezone)
{
  Timestamp  date;
  Timestamp  time;
  pg_time_t  utime;

  /* Use session timezone if caller asks for default */
  if (attimezone == NULL)
    attimezone = session_timezone;

  time = dt;
  TMODULO(time, date, USECS_PER_DAY);

  if (time < INT64CONST(0))
  {
    time += USECS_PER_DAY;
    date -= 1;
  }

  /* add offset to go from J2000 back to standard Julian date */
  date += POSTGRES_EPOCH_JDATE;

  /* Julian day routine does not work for negative Julian days */
  if (date < 0 || date > (Timestamp) INT_MAX)
    return -1;

  j2date((int) date, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
  dt2time(time, &tm->tm_hour, &tm->tm_min, &tm->tm_sec, fsec);

  /* Done if no TZ conversion wanted */
  if (tzp == NULL)
  {
    tm->tm_isdst = -1;
    tm->tm_gmtoff = 0;
    tm->tm_zone = NULL;
    if (tzn != NULL)
      *tzn = NULL;
    return 0;
  }

  /*
   * If the time falls within the range of pg_time_t, use pg_localtime() to
   * rotate to the local time zone.
   *
   * First, convert to an integral timestamp, avoiding possibly
   * platform-specific roundoff-in-wrong-direction errors, and adjust to
   * Unix epoch.  Then see if we can convert to pg_time_t without loss. This
   * coding avoids hardwiring any assumptions about the width of pg_time_t,
   * so it should behave sanely on machines without int64.
   */
  dt = (dt - *fsec) / USECS_PER_SEC +
    (POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY;
  utime = (pg_time_t) dt;
  if ((Timestamp) utime == dt)
  {
    struct pg_tm *tx = pg_localtime(&utime, attimezone);

    tm->tm_year = tx->tm_year + 1900;
    tm->tm_mon = tx->tm_mon + 1;
    tm->tm_mday = tx->tm_mday;
    tm->tm_hour = tx->tm_hour;
    tm->tm_min = tx->tm_min;
    tm->tm_sec = tx->tm_sec;
    tm->tm_isdst = tx->tm_isdst;
    tm->tm_gmtoff = tx->tm_gmtoff;
    tm->tm_zone = tx->tm_zone;
    *tzp = -tm->tm_gmtoff;
    if (tzn != NULL)
      *tzn = tm->tm_zone;
  }
  else
  {
    /*
     * When out of range of pg_time_t, treat as GMT
     */
    *tzp = 0;
    /* Mark this as *no* time zone available */
    tm->tm_isdst = -1;
    tm->tm_gmtoff = 0;
    tm->tm_zone = NULL;
    if (tzn != NULL)
      *tzn = NULL;
  }

  return 0;
}

/* tm2timestamp()
 * Convert a tm structure to a timestamp data type.
 * Note that year is _not_ 1900-based, but is an explicit full value.
 * Also, month is one-based, _not_ zero-based.
 *
 * Returns -1 on failure (value out of range).
 */
int
tm2timestamp(struct pg_tm *tm, fsec_t fsec, int *tzp, Timestamp *result)
{
  TimeOffset  date;
  TimeOffset  time;

  /* Prevent overflow in Julian-day routines */
  if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
  {
    *result = 0;      /* keep compiler quiet */
    return -1;
  }

  date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;
  time = time2t(tm->tm_hour, tm->tm_min, tm->tm_sec, fsec);

  *result = date * USECS_PER_DAY + time;
  /* check for major overflow */
  if ((*result - time) / USECS_PER_DAY != date)
  {
    *result = 0;      /* keep compiler quiet */
    return -1;
  }
  /* check for just-barely overflow (okay except time-of-day wraps) */
  /* caution: we want to allow 1999-12-31 24:00:00 */
  if ((*result < 0 && date > 0) ||
    (*result > 0 && date < -1))
  {
    *result = 0;      /* keep compiler quiet */
    return -1;
  }
  if (tzp != NULL)
    *result = dt2local(*result, -(*tzp));

  /* final range check catches just-out-of-range timestamps */
  if (!IS_VALID_TIMESTAMP(*result))
  {
    *result = 0;      /* keep compiler quiet */
    return -1;
  }

  return 0;
}

/* interval2tm()
 * Convert an interval data type to a tm structure.
 */
int
interval2tm(Interval span, struct pg_tm *tm, fsec_t *fsec)
{
  TimeOffset  time;
  TimeOffset  tfrac;

  tm->tm_year = span.month / MONTHS_PER_YEAR;
  tm->tm_mon = span.month % MONTHS_PER_YEAR;
  tm->tm_mday = span.day;
  time = span.time;

  tfrac = time / USECS_PER_HOUR;
  time -= tfrac * USECS_PER_HOUR;
  tm->tm_hour = tfrac;
  if (!SAMESIGN(tm->tm_hour, tfrac))
    elog(ERROR, "interval out of range");
  tfrac = time / USECS_PER_MINUTE;
  time -= tfrac * USECS_PER_MINUTE;
  tm->tm_min = tfrac;
  tfrac = time / USECS_PER_SEC;
  *fsec = time - (tfrac * USECS_PER_SEC);
  tm->tm_sec = tfrac;

  return 0;
}

int
tm2interval(struct pg_tm *tm, fsec_t fsec, Interval *span)
{
  double    total_months = (double) tm->tm_year * MONTHS_PER_YEAR + tm->tm_mon;

  if (total_months > INT_MAX || total_months < INT_MIN)
    return -1;
  span->month = total_months;
  span->day = tm->tm_mday;
  span->time = (((((tm->tm_hour * INT64CONST(60)) +
           tm->tm_min) * INT64CONST(60)) +
           tm->tm_sec) * USECS_PER_SEC) + fsec;

  return 0;
}

static TimeOffset
time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
  return (((((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec) * USECS_PER_SEC) + fsec;
}

static Timestamp
dt2local(Timestamp dt, int tz)
{
  dt -= (tz * USECS_PER_SEC);
  return dt;
}

/*----------------------------------------------------------
 *	Relational operators for timestamp.
 *---------------------------------------------------------*/

void
GetEpochTime(struct pg_tm *tm)
{
  struct pg_tm *t0;
  pg_time_t  epoch = 0;

  t0 = pg_gmtime(&epoch);

  if (t0 == NULL)
    elog(ERROR, "could not convert epoch to timestamp: %m");

  tm->tm_year = t0->tm_year;
  tm->tm_mon = t0->tm_mon;
  tm->tm_mday = t0->tm_mday;
  tm->tm_hour = t0->tm_hour;
  tm->tm_min = t0->tm_min;
  tm->tm_sec = t0->tm_sec;

  tm->tm_year += 1900;
  tm->tm_mon++;
}

Timestamp
SetEpochTimestamp(void)
{
  Timestamp  dt;
  struct pg_tm tt,
         *tm = &tt;

  GetEpochTime(tm);
  /* we don't bother to test for failure ... */
  tm2timestamp(tm, 0, NULL, &dt);

  return dt;
}                /* SetEpochTimestamp() */

/*
 * We are currently sharing some code between timestamp and timestamptz.
 * The comparison functions are among them. - thomas 2001-09-25
 *
 *    timestamp_relop - is timestamp1 relop timestamp2
 */
int
timestamp_cmp_internal(Timestamp dt1, Timestamp dt2)
{
  return (dt1 < dt2) ? -1 : ((dt1 > dt2) ? 1 : 0);
}
