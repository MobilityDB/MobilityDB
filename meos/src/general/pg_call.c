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
 * @brief MobilityDB functions pg_func(...) corresponding to external
 * PostgreSQL functions func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 */

#include "general/pg_call.h"

/* C */
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <common/int128.h>
#include <utils/datetime.h>
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal_util.h"

/*****************************************************************************/

/* Definitions taken from miscadmin.h */

/* valid DateStyle values */
#define USE_POSTGRES_DATES 0
#define USE_ISO_DATES      1
#define USE_SQL_DATES      2
#define USE_GERMAN_DATES   3
#define USE_XSD_DATES      4

/* valid DateOrder values taken */
#define DATEORDER_YMD      0
#define DATEORDER_DMY      1
#define DATEORDER_MDY      2

/*
 * IntervalStyles
 *   INTSTYLE_POSTGRES         Like Postgres < 8.4 when DateStyle = 'iso'
 *   INTSTYLE_POSTGRES_VERBOSE     Like Postgres < 8.4 when DateStyle != 'iso'
 *   INTSTYLE_SQL_STANDARD       SQL standard interval literals
 *   INTSTYLE_ISO_8601         ISO-8601-basic formatted intervals
 */
#define INTSTYLE_POSTGRES      0
#define INTSTYLE_POSTGRES_VERBOSE  1
#define INTSTYLE_SQL_STANDARD    2
#define INTSTYLE_ISO_8601      3

/* Definitions from globals.c */

int DateStyle = USE_ISO_DATES;
int DateOrder = DATEORDER_MDY;
int IntervalStyle = INTSTYLE_POSTGRES;

/*****************************************************************************
 * Functions adapted from float.c
 *****************************************************************************/

/**
 * @brief Return the sine of arg1 (radians)
 * @note PostgreSQL function: Datum dsin(PG_FUNCTION_ARGS)
 */
float8
pg_dsin(float8 arg1)
{
  float8 result;

  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(arg1))
    return get_float8_nan();

  /* Be sure to throw an error if the input is infinite --- see dcos() */
  errno = 0;
  result = sin(arg1);
  if (errno != 0 || isinf(arg1))
    elog(ERROR, "input is out of range");
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/**
 * @brief Return the cosine of arg1 (radians)
 * @note PostgreSQL function: Datum dcos(PG_FUNCTION_ARGS)
 */
float8
pg_dcos(float8 arg1)
{
  float8 result;

  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(arg1))
    return get_float8_nan();

  /*
   * cos() is periodic and so theoretically can work for all finite inputs,
   * but some implementations may choose to throw error if the input is so
   * large that there are no significant digits in the result.  So we should
   * check for errors.  POSIX allows an error to be reported either via
   * errno or via fetestexcept(), but currently we only support checking
   * errno.  (fetestexcept() is rumored to report underflow unreasonably
   * early on some platforms, so it's not clear that believing it would be a
   * net improvement anyway.)
   *
   * For infinite inputs, POSIX specifies that the trigonometric functions
   * should return a domain error; but we won't notice that unless the
   * platform reports via errno, so also explicitly test for infinite
   * inputs.
   */
  errno = 0;
  result = cos(arg1);
  if (errno != 0 || isinf(arg1))
    elog(ERROR, "input is out of range");
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/**
 * @brief Return the arctan of arg1 (radians)
 * @note PostgreSQL function: Datum datan(PG_FUNCTION_ARGS)
 */
float8
pg_datan(float8 arg1)
{
  float8 result;

  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(arg1))
    return get_float8_nan();

  /*
   * The principal branch of the inverse tangent function maps all inputs to
   * values in the range [-Pi/2, Pi/2], so the result should always be
   * finite, even if the input is infinite.
   */
  result = atan(arg1);
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/**
 * @brief Return the arctan of arg1/arg2 (radians)
 * @note PostgreSQL function: Datum datan2d(PG_FUNCTION_ARGS)
 */
float8
pg_datan2(float8 arg1, float8 arg2)
{
  float8 result;

  /* Per the POSIX spec, return NaN if either input is NaN */
  if (isnan(arg1) || isnan(arg2))
    return get_float8_nan();

  /*
   * atan2 maps all inputs to values in the range [-Pi, Pi], so the result
   * should always be finite, even if the inputs are infinite.
   */
  result = atan2(arg1, arg2);
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/*****************************************************************************
 * Functions adapted from date.c
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_pg_types
 * @brief Convert a string to a date in internal date format.
 * @note PostgreSQL function: Datum date_in(PG_FUNCTION_ARGS)
 */
DateADT
pg_date_in(const char *str)
{
  DateADT date;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;
  int tzp;
  int dtype;
  int nf;
  int dterr;
  char *field[MAXDATEFIELDS];
  int ftype[MAXDATEFIELDS];
  char workbuf[MAXDATELEN + 1];

  dterr = ParseDateTime(str, workbuf, sizeof(workbuf),
              field, ftype, MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tzp);
  if (dterr != 0)
    DateTimeParseError(dterr, str, "date");

  switch (dtype)
  {
    case DTK_DATE:
      break;

    case DTK_EPOCH:
      GetEpochTime(tm);
      break;

    case DTK_LATE:
      DATE_NOEND(date);
      PG_RETURN_DATEADT(date);

    case DTK_EARLY:
      DATE_NOBEGIN(date);
      PG_RETURN_DATEADT(date);

    default:
      DateTimeParseError(DTERR_BAD_FORMAT, str, "date");
      break;
  }

  /* Prevent overflow in Julian-day routines */
  if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
    elog(ERROR, "date out of range: \"%s\"", str);

  date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;

  /* Now check for just-out-of-range dates */
  if (!IS_VALID_DATE(date))
    elog(ERROR, "date out of range: \"%s\"", str);

  return date;
}

/* date_out()
 * Given internal format date, convert to text string.
 */
/**
 * @ingroup libmeos_pg_types
 * @brief Convert a date in internal date format to a string.
 * @note PostgreSQL function: Datum date_in(PG_FUNCTION_ARGS)
 */
char *
pg_date_out(DateADT date)
{
  char *result;
  struct pg_tm tt, *tm = &tt;
  char buf[MAXDATELEN + 1];

  if (DATE_NOT_FINITE(date))
    EncodeSpecialDate(date, buf);
  else
  {
    j2date(date + POSTGRES_EPOCH_JDATE,
         &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));
    EncodeDateOnly(tm, DateStyle, buf);
  }

  result = pstrdup(buf);
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 *   Time ADT
 *****************************************************************************/

#if MEOS
/* AdjustTimeForTypmod()
 * Force the precision of the time value to a specified value.
 * Uses *exactly* the same code as in AdjustTimestampForTypmod()
 * but we make a separate copy because those types do not
 * have a fundamental tie together but rather a coincidence of
 * implementation. - thomas
 */
void
AdjustTimeForTypmod(TimeADT *time, int32 typmod)
{
  static const int64 TimeScales[MAX_TIME_PRECISION + 1] = {
    INT64CONST(1000000),
    INT64CONST(100000),
    INT64CONST(10000),
    INT64CONST(1000),
    INT64CONST(100),
    INT64CONST(10),
    INT64CONST(1)
  };

  static const int64 TimeOffsets[MAX_TIME_PRECISION + 1] = {
    INT64CONST(500000),
    INT64CONST(50000),
    INT64CONST(5000),
    INT64CONST(500),
    INT64CONST(50),
    INT64CONST(5),
    INT64CONST(0)
  };

  if (typmod >= 0 && typmod <= MAX_TIME_PRECISION)
  {
    if (*time >= INT64CONST(0))
      *time = ((*time + TimeOffsets[typmod]) / TimeScales[typmod]) *
        TimeScales[typmod];
    else
      *time = -((((-*time) + TimeOffsets[typmod]) / TimeScales[typmod]) *
            TimeScales[typmod]);
  }
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a string to a time.
 * @note PostgreSQL function: Datum time_in(PG_FUNCTION_ARGS)
 */
TimeADT
pg_time_in(const char *str, int32 typmod)
{
  TimeADT result;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;
  int tz;
  int nf;
  int dterr;
  char workbuf[MAXDATELEN + 1];
  char *field[MAXDATEFIELDS];
  int dtype;
  int ftype[MAXDATEFIELDS];

  dterr = ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype,
    MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeTimeOnly(field, ftype, nf, &dtype, tm, &fsec, &tz);
  if (dterr != 0)
    DateTimeParseError(dterr, str, "time");

  tm2time(tm, fsec, &result);
  AdjustTimeForTypmod(&result, typmod);

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a time to a string.
 * @note PostgreSQL function: Datum time_out(PG_FUNCTION_ARGS)
 */
char *
pg_time_out(TimeADT time)
{
  char *result;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  char buf[MAXDATELEN + 1];

  time2tm(time, tm, &fsec);
  EncodeTimeOnly(tm, fsec, false, 0, DateStyle, buf);

  result = pstrdup(buf);
  return result;
}
#endif /* MEOS */

/*****************************************************************************
 * Functions adapted from timestamp.c
 *****************************************************************************/

/*
 * AdjustTimestampForTypmodError --- round off a timestamp to suit given typmod
 * Works for either timestamp or timestamptz.
 */
bool
AdjustTimestampForTypmodError(Timestamp *time, int32 typmod, bool *error)
{
  static const int64 TimestampScales[MAX_TIMESTAMP_PRECISION + 1] = {
    INT64CONST(1000000),
    INT64CONST(100000),
    INT64CONST(10000),
    INT64CONST(1000),
    INT64CONST(100),
    INT64CONST(10),
    INT64CONST(1)
  };

  static const int64 TimestampOffsets[MAX_TIMESTAMP_PRECISION + 1] = {
    INT64CONST(500000),
    INT64CONST(50000),
    INT64CONST(5000),
    INT64CONST(500),
    INT64CONST(50),
    INT64CONST(5),
    INT64CONST(0)
  };

  if (!TIMESTAMP_NOT_FINITE(*time)
    && (typmod != -1) && (typmod != MAX_TIMESTAMP_PRECISION))
  {
    if (typmod < 0 || typmod > MAX_TIMESTAMP_PRECISION)
    {
      if (error)
      {
        *error = true;
        return false;
      }

      elog(ERROR, "timestamp(%d) precision must be between %d and %d",
              typmod, 0, MAX_TIMESTAMP_PRECISION);
    }

    if (*time >= INT64CONST(0))
    {
      *time = ((*time + TimestampOffsets[typmod]) / TimestampScales[typmod]) *
        TimestampScales[typmod];
    }
    else
    {
      *time = -((((-*time) + TimestampOffsets[typmod]) / TimestampScales[typmod])
            * TimestampScales[typmod]);
    }
  }

  return true;
}

void
AdjustTimestampForTypmod(Timestamp *time, int32 typmod)
{
  (void) AdjustTimestampForTypmodError(time, typmod, NULL);
}

/**
 * @brief Convert a string to a either timestamp or a timestamp with timezone.
 * @note The function returns a TimestampTz that must be cast to a Timestamp
 * when calling the function with the last argument to false
 */
TimestampTz
timestamp_in_common(const char *str, int32 typmod, bool withtz)
{
  TimestampTz result;
  fsec_t    fsec;
  struct pg_tm tt,
         *tm = &tt;
  int      tz;
  int      dtype;
  int      nf;
  int      dterr;
  char     *field[MAXDATEFIELDS];
  int      ftype[MAXDATEFIELDS];
  char    workbuf[MAXDATELEN + MAXDATEFIELDS];

  dterr = ParseDateTime(str, workbuf, sizeof(workbuf),
              field, ftype, MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz);
  if (dterr != 0)
  {
    if (withtz)
      DateTimeParseError(dterr, str, "timestamp with time zone");
    else
      DateTimeParseError(dterr, str, "timestamp");
  }

  switch (dtype)
  {
    case DTK_DATE:
    {
      int status = (withtz) ?
        tm2timestamp(tm, fsec, &tz, &result) :
        tm2timestamp(tm, fsec, NULL, &result);
      if (status != 0)
        elog(ERROR, "timestamp out of range: \"%s\"", str);
      break;
    }
    case DTK_EPOCH:
      result = SetEpochTimestamp();
      break;

    case DTK_LATE:
      TIMESTAMP_NOEND(result);
      break;

    case DTK_EARLY:
      TIMESTAMP_NOBEGIN(result);
      break;

    default:
      elog(ERROR, "unexpected dtype %d while parsing timestamp%s \"%s\"",
        dtype, (withtz) ? "tz" : "", str);
      TIMESTAMP_NOEND(result);
  }

  AdjustTimestampForTypmod(&result, typmod);

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a string to a timestamp with time zone.
 * @note PostgreSQL function: Datum timestamptz_in(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamptz_in(const char *str, int32 typmod)
{
  return timestamp_in_common(str, typmod, true);
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a string to a timestamp without time zone.
 * @note PostgreSQL function: Datum timestamp_in(PG_FUNCTION_ARGS)
 */
Timestamp
pg_timestamp_in(const char *str, int32 typmod)
{
  return (Timestamp) timestamp_in_common(str, typmod, false);
}

/**
 * @brief Convert either a timestamp or a timestamp to a string.
 */
char *
timestamp_out_common(TimestampTz dt, bool withtz)
{
  char *result;
  int tz;
  struct pg_tm tt,
         *tm = &tt;
  fsec_t fsec;
  const char *tzn;
  char buf[MAXDATELEN + 1];

  if (TIMESTAMP_NOT_FINITE(dt))
    EncodeSpecialTimestamp(dt, buf);
  else if (withtz && timestamp2tm(dt, &tz, tm, &fsec, &tzn, NULL) == 0)
    EncodeDateTime(tm, fsec, true, tz, tzn, DateStyle, buf);
  else if (! withtz && timestamp2tm(dt, NULL, tm, &fsec, NULL, NULL) == 0)
    EncodeDateTime(tm, fsec, false, 0, NULL, DateStyle, buf);
  else
    elog(ERROR, "timestamp out of range");

  result = pstrdup(buf);
  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a timestamp with timezone to a string.
 * @note PostgreSQL function: Datum timestamptz_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamptz_out(TimestampTz dt)
{
  return timestamp_out_common(dt, true);
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a timestamp without timezone to a string.
 * @note PostgreSQL function: Datum timestamp_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamp_out(Timestamp dt)
{
  return timestamp_out_common((Timestamp) dt, false);
}

/*****************************************************************************/

#if MEOS
/*
 *  Adjust interval for specified precision, in both YEAR to SECOND
 *  range and sub-second precision.
 */
static void
AdjustIntervalForTypmod(Interval *interval, int32 typmod)
{
  static const int64 IntervalScales[MAX_INTERVAL_PRECISION + 1] = {
    INT64CONST(1000000),
    INT64CONST(100000),
    INT64CONST(10000),
    INT64CONST(1000),
    INT64CONST(100),
    INT64CONST(10),
    INT64CONST(1)
  };

  static const int64 IntervalOffsets[MAX_INTERVAL_PRECISION + 1] = {
    INT64CONST(500000),
    INT64CONST(50000),
    INT64CONST(5000),
    INT64CONST(500),
    INT64CONST(50),
    INT64CONST(5),
    INT64CONST(0)
  };

  /*
   * Unspecified range and precision? Then not necessary to adjust. Setting
   * typmod to -1 is the convention for all data types.
   */
  if (typmod >= 0)
  {
    int      range = INTERVAL_RANGE(typmod);
    int      precision = INTERVAL_PRECISION(typmod);

    /*
     * Our interpretation of intervals with a limited set of fields is
     * that fields to the right of the last one specified are zeroed out,
     * but those to the left of it remain valid.  Thus for example there
     * is no operational difference between INTERVAL YEAR TO MONTH and
     * INTERVAL MONTH.  In some cases we could meaningfully enforce that
     * higher-order fields are zero; for example INTERVAL DAY could reject
     * nonzero "month" field.  However that seems a bit pointless when we
     * can't do it consistently.  (We cannot enforce a range limit on the
     * highest expected field, since we do not have any equivalent of
     * SQL's <interval leading field precision>.)  If we ever decide to
     * revisit this, interval_support will likely require adjusting.
     *
     * Note: before PG 8.4 we interpreted a limited set of fields as
     * actually causing a "modulo" operation on a given value, potentially
     * losing high-order as well as low-order information.  But there is
     * no support for such behavior in the standard, and it seems fairly
     * undesirable on data consistency grounds anyway.  Now we only
     * perform truncation or rounding of low-order fields.
     */
    if (range == INTERVAL_FULL_RANGE)
    {
      /* Do nothing... */
    }
    else if (range == INTERVAL_MASK(YEAR))
    {
      interval->month = (interval->month / MONTHS_PER_YEAR) * MONTHS_PER_YEAR;
      interval->day = 0;
      interval->time = 0;
    }
    else if (range == INTERVAL_MASK(MONTH))
    {
      interval->day = 0;
      interval->time = 0;
    }
    /* YEAR TO MONTH */
    else if (range == (INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH)))
    {
      interval->day = 0;
      interval->time = 0;
    }
    else if (range == INTERVAL_MASK(DAY))
    {
      interval->time = 0;
    }
    else if (range == INTERVAL_MASK(HOUR))
    {
      interval->time = (interval->time / USECS_PER_HOUR) *
        USECS_PER_HOUR;
    }
    else if (range == INTERVAL_MASK(MINUTE))
    {
      interval->time = (interval->time / USECS_PER_MINUTE) *
        USECS_PER_MINUTE;
    }
    else if (range == INTERVAL_MASK(SECOND))
    {
      /* fractional-second rounding will be dealt with below */
    }
    /* DAY TO HOUR */
    else if (range == (INTERVAL_MASK(DAY) |
               INTERVAL_MASK(HOUR)))
    {
      interval->time = (interval->time / USECS_PER_HOUR) *
        USECS_PER_HOUR;
    }
    /* DAY TO MINUTE */
    else if (range == (INTERVAL_MASK(DAY) |
               INTERVAL_MASK(HOUR) |
               INTERVAL_MASK(MINUTE)))
    {
      interval->time = (interval->time / USECS_PER_MINUTE) *
        USECS_PER_MINUTE;
    }
    /* DAY TO SECOND */
    else if (range == (INTERVAL_MASK(DAY) |
               INTERVAL_MASK(HOUR) |
               INTERVAL_MASK(MINUTE) |
               INTERVAL_MASK(SECOND)))
    {
      /* fractional-second rounding will be dealt with below */
    }
    /* HOUR TO MINUTE */
    else if (range == (INTERVAL_MASK(HOUR) |
               INTERVAL_MASK(MINUTE)))
    {
      interval->time = (interval->time / USECS_PER_MINUTE) *
        USECS_PER_MINUTE;
    }
    /* HOUR TO SECOND */
    else if (range == (INTERVAL_MASK(HOUR) |
               INTERVAL_MASK(MINUTE) |
               INTERVAL_MASK(SECOND)))
    {
      /* fractional-second rounding will be dealt with below */
    }
    /* MINUTE TO SECOND */
    else if (range == (INTERVAL_MASK(MINUTE) |
               INTERVAL_MASK(SECOND)))
    {
      /* fractional-second rounding will be dealt with below */
    }
    else
      elog(ERROR, "unrecognized interval typmod: %d", typmod);

    /* Need to adjust sub-second precision? */
    if (precision != INTERVAL_FULL_PRECISION)
    {
      if (precision < 0 || precision > MAX_INTERVAL_PRECISION)
        elog(ERROR, "interval(%d) precision must be between %d and %d",
                precision, 0, MAX_INTERVAL_PRECISION);

      if (interval->time >= INT64CONST(0))
      {
        interval->time = ((interval->time +
                   IntervalOffsets[precision]) /
                  IntervalScales[precision]) *
          IntervalScales[precision];
      }
      else
      {
        interval->time = -(((-interval->time +
                   IntervalOffsets[precision]) /
                  IntervalScales[precision]) *
                   IntervalScales[precision]);
      }
    }
  }
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a string to an interval.
 * @note PostgreSQL function: Datum interval_in(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_in(const char *str, int32 typmod)
{
  Interval *result;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;
  int dtype;
  int nf;
  int range;
  int dterr;
  char *field[MAXDATEFIELDS];
  int ftype[MAXDATEFIELDS];
  char workbuf[256];

  tm->tm_year = 0;
  tm->tm_mon = 0;
  tm->tm_mday = 0;
  tm->tm_hour = 0;
  tm->tm_min = 0;
  tm->tm_sec = 0;
  fsec = 0;

  if (typmod >= 0)
    range = INTERVAL_RANGE(typmod);
  else
    range = INTERVAL_FULL_RANGE;

  dterr = ParseDateTime(str, workbuf, sizeof(workbuf), field,
              ftype, MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeInterval(field, ftype, nf, range, &dtype, tm, &fsec);

  /* if those functions think it's a bad format, try ISO8601 style */
  if (dterr == DTERR_BAD_FORMAT)
    dterr = DecodeISO8601Interval((char *) str, &dtype, tm, &fsec);

  if (dterr != 0)
  {
    if (dterr == DTERR_FIELD_OVERFLOW)
      dterr = DTERR_INTERVAL_OVERFLOW;
    DateTimeParseError(dterr, str, "interval");
  }

  result = (Interval *) palloc(sizeof(Interval));

  switch (dtype)
  {
    case DTK_DELTA:
      if (tm2interval(tm, fsec, result) != 0)
        elog(ERROR, "interval out of range");
      break;

    default:
      elog(ERROR, "unexpected dtype %d while parsing interval \"%s\"",
         dtype, str);
  }

  AdjustIntervalForTypmod(result, typmod);

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Interval constructor
 * @note PostgreSQL function: Datum make_interval(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours,
  int32 mins, double secs)
{
  Interval *result;

  /*
   * Reject out-of-range inputs.  We really ought to check the integer
   * inputs as well, but it's not entirely clear what limits to apply.
   */
  if (isinf(secs) || isnan(secs))
    elog(ERROR, "interval out of range");

  result = (Interval *) palloc(sizeof(Interval));
  result->month = years * MONTHS_PER_YEAR + months;
  result->day = weeks * 7 + days;

  secs = rint(secs * USECS_PER_SEC);
  result->time = hours * ((int64) SECS_PER_HOUR * USECS_PER_SEC) +
    mins * ((int64) SECS_PER_MINUTE * USECS_PER_SEC) + (int64) secs;

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Convert a time span to external form.
 * @note PostgreSQL function: Datum interval_out(PG_FUNCTION_ARGS)
 */
char *
pg_interval_out(Interval *span)
{
  char *result;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  char buf[MAXDATELEN + 1];

  if (interval2tm(*span, tm, &fsec) != 0)
    elog(ERROR, "could not convert interval to tm");

  EncodeInterval(tm, fsec, IntervalStyle, buf);

  result = pstrdup(buf);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

#define SAMESIGN(a,b) (((a) < 0) == ((b) < 0))

/**
 * @ingroup libmeos_pg_types
 * @brief Add an interval to a timestamp data type.
 * @note PostgreSQL function: Datum interval_pl(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_pl(const Interval *span1, const Interval *span2)
{
  Interval *result = palloc(sizeof(Interval));

  result->month = span1->month + span2->month;
  /* overflow check copied from int4pl */
  if (SAMESIGN(span1->month, span2->month) &&
    ! SAMESIGN(result->month, span1->month))
    elog(ERROR, "interval out of range");

  result->day = span1->day + span2->day;
  if (SAMESIGN(span1->day, span2->day) &&
    ! SAMESIGN(result->day, span1->day))
    elog(ERROR, "interval out of range");

  result->time = span1->time + span2->time;
  if (SAMESIGN(span1->time, span2->time) &&
    ! SAMESIGN(result->time, span1->time))
    elog(ERROR, "interval out of range");

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Add an interval to a timestamp data type.
 *
 * Note that interval has provisions for qualitative year/month and day
 * units, so try to do the right thing with them.
 * To add a month, increment the month, and use the same day of month.
 * Then, if the next month has fewer days, set the day of month
 * to the last day of month.
 * To add a day, increment the mday, and use the same time of day.
 * Lastly, add in the "quantitative time".
 * @note PostgreSQL function: Datum timestamp_pl_interval(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamp_pl_interval(TimestampTz timestamp, const Interval *span)
{
  Timestamp result;

  if (TIMESTAMP_NOT_FINITE(timestamp))
    result = timestamp;
  else
  {
    if (span->month != 0)
    {
      struct pg_tm tt,
             *tm = &tt;
      fsec_t    fsec;

      if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) != 0)
        elog(ERROR, "timestamp out of range");

      tm->tm_mon += span->month;
      if (tm->tm_mon > MONTHS_PER_YEAR)
      {
        tm->tm_year += (tm->tm_mon - 1) / MONTHS_PER_YEAR;
        tm->tm_mon = ((tm->tm_mon - 1) % MONTHS_PER_YEAR) + 1;
      }
      else if (tm->tm_mon < 1)
      {
        tm->tm_year += tm->tm_mon / MONTHS_PER_YEAR - 1;
        tm->tm_mon = tm->tm_mon % MONTHS_PER_YEAR + MONTHS_PER_YEAR;
      }

      /* adjust for end of month boundary problems... */
      if (tm->tm_mday > day_tab[isleap(tm->tm_year)][tm->tm_mon - 1])
        tm->tm_mday = (day_tab[isleap(tm->tm_year)][tm->tm_mon - 1]);

      if (tm2timestamp(tm, fsec, NULL, &timestamp) != 0)
        elog(ERROR, "timestamp out of range");
    }

    if (span->day != 0)
    {
      struct pg_tm tt,
             *tm = &tt;
      fsec_t    fsec;
      int      julian;

      if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) != 0)
        elog(ERROR, "timestamp out of range");

      /* Add days by converting to and from Julian */
      julian = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) + span->day;
      j2date(julian, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);

      if (tm2timestamp(tm, fsec, NULL, &timestamp) != 0)
        elog(ERROR, "timestamp out of range");
    }

    timestamp += span->time;

    if (!IS_VALID_TIMESTAMP(timestamp))
      elog(ERROR, "timestamp out of range");

    result = timestamp;
  }

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Add an interval to a timestamp data type.
 * @note PostgreSQL function: Datum timestamp_pl_interval(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamp_mi_interval(TimestampTz timestamp, const Interval *span)
{
  Interval tspan;
  tspan.month = -span->month;
  tspan.day = -span->day;
  tspan.time = -span->time;
  return pg_timestamp_pl_interval(timestamp, &tspan);
}

/**
 * @brief Add an interval to a timestamp data type.
 *
 *  Adjust interval so 'time' contains less than a whole day, adding
 *  the excess to 'day'.  This is useful for
 *  situations (such as non-TZ) where '1 day' = '24 hours' is valid,
 *  e.g. interval subtraction and division.
 * @note PostgreSQL function: Datum interval_justify_hours(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_justify_hours(const Interval *span)
{
  Interval *result = palloc(sizeof(Interval));
  result->month = span->month;
  result->day = span->day;
  result->time = span->time;

  TimeOffset wholeday = 0; /* make compiler quiet */
  TMODULO(result->time, wholeday, USECS_PER_DAY);
  result->day += wholeday;  /* could overflow... */

  if (result->day > 0 && result->time < 0)
  {
    result->time += USECS_PER_DAY;
    result->day--;
  }
  else if (result->day < 0 && result->time > 0)
  {
    result->time -= USECS_PER_DAY;
    result->day++;
  }

  return result;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Compute the difference of two timestamps
 * @note PostgreSQL function: Datum timestamp_mi(PG_FUNCTION_ARGS)
 * The original code from PostgreSQL has `Timestamp` as arguments
 */
Interval *
pg_timestamp_mi(TimestampTz dt1, TimestampTz dt2)
{
  if (TIMESTAMP_NOT_FINITE(dt1) || TIMESTAMP_NOT_FINITE(dt2))
    elog(ERROR, "cannot subtract infinite timestamps");

  Interval interval;
  interval.time = dt1 - dt2;
  interval.month = 0;
  interval.day = 0;
  Interval *result = pg_interval_justify_hours(&interval);
  return result;
}

/*
 *    interval_relop  - is interval1 relop interval2
 *
 * Interval comparison is based on converting interval values to a linear
 * representation expressed in the units of the time field (microseconds,
 * in the case of integer timestamps) with days assumed to be always 24 hours
 * and months assumed to be always 30 days.  To avoid overflow, we need a
 * wider-than-int64 datatype for the linear representation, so use INT128.
 */

static inline INT128
interval_cmp_value(const Interval *interval)
{
  INT128 span;
  int64 dayfraction;
  int64 days;

  /*
   * Separate time field into days and dayfraction, then add the month and
   * day fields to the days part.  We cannot overflow int64 days here.
   */
  dayfraction = interval->time % USECS_PER_DAY;
  days = interval->time / USECS_PER_DAY;
  days += interval->month * INT64CONST(30);
  days += interval->day;

  /* Widen dayfraction to 128 bits */
  span = (INT128) dayfraction;

  /* Scale up days to microseconds, forming a 128-bit product */
  span += (int128) days * (int128) USECS_PER_DAY;

  return span;
}

/**
 * @ingroup libmeos_pg_types
 * @brief Compare the two intervals
 * @note PostgreSQL function: Datum interval_cmp(PG_FUNCTION_ARGS)
 */
int
pg_interval_cmp(const Interval *interval1, const Interval *interval2)
{
  INT128 span1 = interval_cmp_value(interval1);
  INT128 span2 = interval_cmp_value(interval2);
  return int128_compare(span1, span2);
}

/*****************************************************************************
 * Functions adapted from hashfn.h and hashfn.c
 *****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 130000

/* Rotate a uint32 value left by k bits - note multiple evaluation! */
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c, 4); \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

/*
 * hash_bytes_uint32() -- hash a 32-bit value to a 32-bit value
 *
 * This has the same result as
 *    hash_bytes(&k, sizeof(uint32))
 * but is faster and doesn't force the caller to store k into memory.
 */
uint32
hash_bytes_uint32(uint32 k)
{
  uint32    a,
        b,
        c;

  a = b = c = 0x9e3779b9 + (uint32) sizeof(uint32) + 3923095;
  a += k;

  final(a, b, c);

  /* report the result */
  return c;
}

/*
 * hash_bytes_uint32_extended() -- hash 32-bit value to 64-bit value, with seed
 *
 * Like hash_bytes_uint32, this is a convenience function.
 */
uint64
hash_bytes_uint32_extended(uint32 k, uint64 seed)
{
  uint32 a, b, c;
  a = b = c = 0x9e3779b9 + (uint32) sizeof(uint32) + 3923095;

  if (seed != 0)
  {
    a += (uint32) (seed >> 32);
    b += (uint32) seed;
    mix(a, b, c);
  }

  a += k;

  final(a, b, c);

  /* report the result */
  return ((uint64) b << 32) | c;
}

#endif /* POSTGRESQL_VERSION_NUMBER < 130000 */

/**
 * @brief Get the 32-bit hash value of an int64 value.
 * @note PostgreSQL function: Datum hashint8(PG_FUNCTION_ARGS)
 */
uint32
pg_hashint8(int64 val)
{
  /*
   * The idea here is to produce a hash value compatible with the values
   * produced by hashint4 and hashint2 for logically equal inputs; this is
   * necessary to support cross-type hash joins across these input types.
   * Since all three types are signed, we can xor the high half of the int8
   * value if the sign is positive, or the complement of the high half when
   * the sign is negative.
   */
  uint32 lohalf = (uint32) val;
  uint32 hihalf = (uint32) (val >> 32);
  lohalf ^= (val >= 0) ? hihalf : ~hihalf;
  return DatumGetUInt32(hash_uint32(lohalf));
}

/**
 * @brief Get the 64-bit hash value of an int64 value.
 * @note PostgreSQL function: Datum hashint8extended(PG_FUNCTION_ARGS)
 */
uint64
pg_hashint8extended(int64 val, uint64 seed)
{
  /* Same approach as hashint8 */
  uint32 lohalf = (uint32) val;
  uint32 hihalf = (uint32) (val >> 32);
  lohalf ^= (val >= 0) ? hihalf : ~hihalf;
  return hash_uint32_extended(lohalf, seed);
}

/**
 * @brief Get the 32-bit hash value of an float64 value.
 * @note PostgreSQL function: Datum hashfloat8(PG_FUNCTION_ARGS)
 */
uint32
pg_hashfloat8(float8 key)
{
  /*
   * On IEEE-float machines, minus zero and zero have different bit patterns
   * but should compare as equal.  We must ensure that they have the same
   * hash value, which is most reliably done this way:
   */
  if (key == (float8) 0)
    return((uint32) 0);
  /*
   * Similarly, NaNs can have different bit patterns but they should all
   * compare as equal.  For backwards-compatibility reasons we force them to
   * have the hash value of a standard NaN.
   */
  if (isnan(key))
    key = get_float8_nan();
  return DatumGetUInt32(hash_any((unsigned char *) &key, sizeof(key)));
}

#if 0 /* not used */
/**
 * @brief Get the 64-bit hash value of a float64 value.
 * @note PostgreSQL function: Datum hashfloat8extended(PG_FUNCTION_ARGS)
 */
uint64
pg_hashfloat8extended(float8 key, uint64 seed)
{
  /* Same approach as hashfloat8 */
  if (key == (float8) 0)
    return seed;
  if (isnan(key))
    key = get_float8_nan();
  return DatumGetUInt64(hash_any_extended((unsigned char *) &key, sizeof(key),
    seed));
}
#endif /* not used */

/**
 * @brief Get the 32-bit hash value of an text value.
 * @note PostgreSQL function: Datum hashtext(PG_FUNCTION_ARGS)
 * We simulate what would happen using DEFAULT_COLLATION_OID
 */
uint32
pg_hashtext(text *key)
{
  uint32 result = UInt32GetDatum(hash_any((unsigned char *) VARDATA_ANY(key),
    VARSIZE_ANY_EXHDR(key)));
  return result;
}
/*****************************************************************************/

