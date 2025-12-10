/*-------------------------------------------------------------------------
 *
 * timestamp.c
 *    Functions for the built-in SQL types "timestamp" and "interval".
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/timestamp.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>

#include "miscadmin.h"
#include "catalog/pg_type.h"
#include "common/int.h"
#include "common/int128.h"
#include "parser/scansup.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/float.h"
#include "utils/numeric.h"
#include "utils/jsonb.h"

#include "pgtypes.h"

extern Numeric int64_div_fast_to_numeric(int64 val1, int log10val2);

// #include "access/xact.h"
// #include "catalog/pg_type.h"
// #include "common/int.h"
// #include "common/int128.h"
// #include "funcapi.h"
// #include "libpq/pqformat.h"
// #include "miscadmin.h"
// #include "nodes/nodeFuncs.h"
// #include "nodes/supportnodes.h"
// #include "optimizer/optimizer.h"
// #include "parser/scansup.h"
// #include "utils/array.h"
// #include "utils/builtins.h"
// #include "utils/date.h"
// #include "utils/datetime.h"
// #include "utils/float.h"
// #include "utils/numeric.h"
// #include "utils/skipsupport.h"
// #include "utils/sortsupport.h"

/*
 * gcc's -ffast-math switch breaks routines that expect exact results from
 * expressions like timeval / SECS_PER_HOUR, where timeval is double.
 */
#ifdef __FAST_MATH__
#error -ffast-math is known to break this code
#endif

#define SAMESIGN(a,b)  (((a) < 0) == ((b) < 0))

/* Set at postmaster start */
TimestampTz PgStartTime;

/* Set at configuration reload */
TimestampTz PgReloadTime;

#define IA_TOTAL_COUNT(ia) \
  ((ia)->N + (ia)->pInfcount + (ia)->nInfcount)

static TimeOffset time2t(const int hour, const int min, const int sec,
  const fsec_t fsec);
static Timestamp dt2local(Timestamp dt, int timezone);
static bool AdjustIntervalForTypmod(Interval *interval, int32 typmod,
  Node *escontext);
static TimestampTz timestamp2timestamptz(Timestamp timestamp);
static Timestamp timestamptz2timestamp(TimestampTz timestamp);

static void EncodeSpecialInterval(const Interval *interval, char *str);
static void interval_um_internal(const Interval *interval, Interval *result);

/* common code for timestamptypmodout and timestamptztypmodout */
static char *
anytimestamp_typmodout(bool istz, int32 typmod)
{
  const char *tz = istz ? " with time zone" : " without time zone";
  if (typmod >= 0)
    return psprintf("(%d)%s", (int) typmod, tz);
  else
    return pstrdup(tz);
}

/*****************************************************************************
 *   USER I/O ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_timestamp
 * @brief Return a timestamp from its string representation
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_in()
 */
#if MEOS
Timestamp
timestamp_in(const char *str, int32 typmod)
{
  return pg_timestamp_in(str, typmod);
}
#endif
Timestamp
pg_timestamp_in(const char *str, int32 typmod)
{
  Timestamp result;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;
  int tz;
  int dtype;
  int nf;
  int dterr;
  char *field[MAXDATEFIELDS];
  int ftype[MAXDATEFIELDS];
  char workbuf[MAXDATELEN + MAXDATEFIELDS];
  DateTimeErrorExtra extra;

  dterr = pg_ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype,
    MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz, &extra);
  if (dterr != 0)
  {
    pg_DateTimeParseError(dterr, &extra, str, "timestamp", NULL);
    return DT_NOEND;
  }

  switch (dtype)
  {
    case DTK_DATE:
      if (tm2timestamp(tm, fsec, NULL, &result) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range: \"%s\"", str);
        return (Datum) 0;
      }
      break;

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
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unexpected dtype %d while parsing timestamp \"%s\"",
        dtype, str);
      TIMESTAMP_NOEND(result);
    }
  }

  AdjustTimestampForTypmod(&result, typmod, NULL);
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the string representation of a timestamp
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p timestamp_out()
 */
#if MEOS
char *
timestamp_out(Timestamp ts)
{
  return pg_timestamp_out(ts);
}
#endif
char *
pg_timestamp_out(Timestamp ts)
{
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  char buf[MAXDATELEN + 1];

  if (TIMESTAMP_NOT_FINITE(ts))
    EncodeSpecialTimestamp(ts, buf);
  else if (timestamp2tm(ts, NULL, tm, &fsec, NULL, NULL) == 0)
    EncodeDateTime(tm, fsec, false, 0, NULL, DateStyle, buf);
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return NULL;
  }

  return pstrdup(buf);
}

/* Output the typmod of a timestamp */
char *
timestamp_typmodout(int32 typmod)
{
  return anytimestamp_typmodout(false, typmod);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Adjust a timestamp for specified scale factor
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_scale()
 */
#if MEOS
Timestamp
timestamp_scale(Timestamp ts, int32 typmod)
{
  return pg_timestamp_scale(ts, typmod);
}
#endif
Timestamp
pg_timestamp_scale(Timestamp ts, int32 typmod)
{
  Timestamp result = ts;
  if (AdjustTimestampForTypmod(&result, typmod, NULL))
    return DT_NOEND;
  return result;
}

/*
 * AdjustTimestampForTypmod --- round off a timestamp to suit given typmod
 * Works for either timestamp or timestamptz.
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
bool
AdjustTimestampForTypmod(Timestamp *time, int32 typmod, Node *escontext UNUSED)
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
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp(%d) precision must be between %d and %d",
        typmod, 0, MAX_TIMESTAMP_PRECISION);
      return false;
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

/**
 * @ingroup meos_base_timestamp
 * @brief Return a timestamptz from its string representation
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_in()
 */
#if MEOS
TimestampTz
timestamptz_in(const char *str, int32 typmod)
{
  return pg_timestamptz_in(str, typmod);
}
#endif
TimestampTz
pg_timestamptz_in(const char *str, int32 typmod)
{
  TimestampTz result;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;
  int tz;
  int dtype;
  int nf;
  int dterr;
  char *field[MAXDATEFIELDS];
  int ftype[MAXDATEFIELDS];
  char workbuf[MAXDATELEN + MAXDATEFIELDS];
  DateTimeErrorExtra extra;

  dterr = pg_ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype,
    MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz, &extra);
  if (dterr != 0)
  {
    pg_DateTimeParseError(dterr, &extra, str, "timestamp with time zone", NULL);
    return DT_NOEND;
  }

  switch (dtype)
  {
    case DTK_DATE:
      if (tm2timestamp(tm, fsec, &tz, &result) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range: \"%s\"", str);
        return (Datum) 0;
      }
      break;

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
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unexpected dtype %d while parsing timestamptz \"%s\"",
        dtype, str);
      TIMESTAMP_NOEND(result);
  }

  AdjustTimestampForTypmod(&result, typmod, NULL);
  return result;
}

/*
 * Try to parse a timezone specification, and return its timezone offset value
 * if it's acceptable.  Otherwise, an error is thrown.
 *
 * Note: some code paths update tm->tm_isdst, and some don't; current callers
 * don't care, so we don't bother being consistent.
 */
static int
parse_sane_timezone(struct pg_tm *tm, text *zone)
{
  char tzname[TZ_STRLEN_MAX + 1];
  text_to_cstring_buffer(zone, tzname, sizeof(tzname));

  /*
   * Look up the requested timezone.  First we try to interpret it as a
   * numeric timezone specification; if DecodeTimezone decides it doesn't
   * like the format, we try timezone abbreviations and names.
   *
   * Note pg_tzset happily parses numeric input that DecodeTimezone would
   * reject.  To avoid having it accept input that would otherwise be seen
   * as invalid, it's enough to disallow having a digit in the first
   * position of our input string.
   */
  if (isdigit((unsigned char) *tzname))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "invalid input syntax for type %s: \"%s\"",
      "numeric time zone", tzname);
    return INT_MAX;
  }

  int tz;
  int dterr = DecodeTimezone(tzname, &tz);
  if (dterr != 0)
  {
    if (dterr == DTERR_TZDISP_OVERFLOW)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "numeric time zone \"%s\" out of range", tzname);
      return INT_MAX;
    }
    else if (dterr != DTERR_BAD_FORMAT)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "time zone \"%s\" not recognized", tzname);
      return INT_MAX;
    }

    int val;
    pg_tz *tzp;
    int type = DecodeTimezoneName(tzname, &val, &tzp);
    if (type == TZNAME_FIXED_OFFSET)
    {
      /* fixed-offset abbreviation */
      tz = -val;
    }
    else if (type == TZNAME_DYNTZ)
    {
      /* dynamic-offset abbreviation, resolve using specified time */
      tz = DetermineTimeZoneAbbrevOffset(tm, tzname, tzp);
    }
    else
    {
      /* full zone name */
      tz = DetermineTimeZoneOffset(tm, tzp);
    }
    pfree(tzp);
  }
  return tz;
}

/*
 * Look up the requested timezone, returning a pg_tz struct.
 *
 * This is the same as DecodeTimezoneNameToTz, but starting with a text Datum.
 */
static pg_tz *
lookup_timezone(text *zone)
{
  char tzname[TZ_STRLEN_MAX + 1];
  text_to_cstring_buffer(zone, tzname, sizeof(tzname));
  return DecodeTimezoneNameToTz(tzname);
}

/*
 * make_timestamp_internal
 *    workhorse for make_timestamp and make_timestamptz
 */
static Timestamp
make_timestamp_internal(int year, int month, int day, int hour, int min,
  double sec)
{
  Timestamp result;
  struct pg_tm tm;
  tm.tm_year = year;
  tm.tm_mon = month;
  tm.tm_mday = day;

  /* Handle negative years as BC */
  bool bc = false;
  if (tm.tm_year < 0)
  {
    bc = true;
    tm.tm_year = -tm.tm_year;
  }

  int dterr = ValidateDate(DTK_DATE_M, false, false, bc, &tm);
  if (dterr != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "date field value out of range: %d-%02d-%02d", year, month, day);
    return DT_NOEND;
  }

  if (!IS_VALID_JULIAN(tm.tm_year, tm.tm_mon, tm.tm_mday))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "date out of range: %d-%02d-%02d", year, month, day);
    return DT_NOEND;
  }

  TimeOffset date = date2j(tm.tm_year, tm.tm_mon, tm.tm_mday) - POSTGRES_EPOCH_JDATE;

  /* Check for time overflow */
  if (float_time_overflows(hour, min, sec))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "time field value out of range: %d:%02d:%02g", hour, min, sec);
    return DT_NOEND;
  }

  /* This should match tm2time */
  TimeOffset time = (((hour * MINS_PER_HOUR + min) * SECS_PER_MINUTE)
      * USECS_PER_SEC) + (int64) rint(sec * USECS_PER_SEC);

  if (unlikely(pg_mul_s64_overflow(date, USECS_PER_DAY, &result) ||
         pg_add_s64_overflow(result, time, &result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range: %d-%02d-%02d %d:%02d:%02g",
      year, month, day, hour, min, sec);
    return DT_NOEND;
  }

  /* final range check catches just-out-of-range timestamps */
  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range: %d-%02d-%02d %d:%02d:%02g",
      year, month, day, hour, min, sec);
    return DT_NOEND;
  }

  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Construct a timestamp from the arguments
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p make_timestamp()
 */
Timestamp
timestamp_make(int32 year, int32 month, int32 mday, int32 hour, int32 min,
  float8 sec)
{
  return make_timestamp_internal(year, month, mday, hour, min, sec);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Construct a timestamptz from the arguments
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p make_timestamptz()
 */
TimestampTz
timestamptz_make(int32 year, int32 month, int32 day, int32 hour, int32 min,
  float8 sec)
{
  Timestamp result = make_timestamp_internal(year, month, day, hour, min, sec);
  return timestamp2timestamptz(result);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Construct a timestamptz from the arguments
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p make_timestamptz_at_timezone()
 */
TimestampTz
timestamptz_make_at_timezone(int32 year, int32 month, int32 day, int32 hour,
  int32 min, float8 sec, const text *zone)
{
  struct pg_tm tt;
  fsec_t fsec;
  Timestamp timestamp = make_timestamp_internal(year, month, day, hour, min, sec);
  if (timestamp2tm(timestamp, NULL, &tt, &fsec, NULL, NULL) != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return DT_NOEND;
  }

  int tz = parse_sane_timezone(&tt, (text *) zone);
  TimestampTz result = dt2local(timestamp, -tz);
  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return DT_NOEND;
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Convert a UNIX epoch to a timestamptz
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p float8_timestamptz()
 */
TimestampTz
float8_to_timestamptz(float8 secs)
{
  TimestampTz result;

  /* Deal with NaN and infinite inputs ... */
  if (isnan(secs))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp cannot be NaN");
    return DT_NOEND;
  }

  if (isinf(secs))
  {
    if (secs < 0)
      TIMESTAMP_NOBEGIN(result);
    else
      TIMESTAMP_NOEND(result);
  }
  else
  {
    /* Out of range? */
    if (secs <
      (float8) SECS_PER_DAY * (DATETIME_MIN_JULIAN - UNIX_EPOCH_JDATE)
      || secs >=
      (float8) SECS_PER_DAY * (TIMESTAMP_END_JULIAN - UNIX_EPOCH_JDATE))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range: \"%g\"", secs);
      return DT_NOEND;
    }

    /* Convert UNIX epoch to Postgres epoch */
    secs -= ((POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY);

    secs = rint(secs * USECS_PER_SEC);
    result = (int64) secs;

    /* Recheck in case roundoff produces something just out of range */
    if (!IS_VALID_TIMESTAMP(result))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range: \"%g\"", secs);
      return DT_NOEND;
    }
  }

  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the string representation of a timestamptz
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p timestamptz_out()
 */
#if MEOS
char *
timestamptz_out(TimestampTz ts)
{
  return pg_timestamptz_out(ts);
}
#endif
char *
pg_timestamptz_out(TimestampTz ts)
{
  char *result;
  int tz;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  const char *tzn;
  char buf[MAXDATELEN + 1];

  if (TIMESTAMP_NOT_FINITE(ts))
    EncodeSpecialTimestamp(ts, buf);
  else if (timestamp2tm(ts, &tz, tm, &fsec, &tzn, NULL) == 0)
    EncodeDateTime(tm, fsec, true, tz, tzn, DateStyle, buf);
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return NULL;
  }

  result = pstrdup(buf);
  return result;
}

/* Output the typmod of a timestamptz */
char *
timestamptz_typmodout(int32 typmod)
{
  return anytimestamp_typmodout(true, typmod);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Adjust a time for a scale factor
 * @note Derived from PostgreSQL function @p timestamptz_scale()
 */
#if MEOS
TimestampTz
timestamptz_scale(TimestampTz ts, int32 typmod)
{
  return pg_timestamptz_scale(ts, typmod);
}
#endif
TimestampTz
pg_timestamptz_scale(TimestampTz ts, int32 typmod)
{
  TimestampTz result = ts;
  AdjustTimestampForTypmod(&result, typmod, NULL);
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Return an interval from its string representation
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_in()
 */
#if MEOS
Interval *
interval_in(const char *str, int32 typmod)
{
  return pg_interval_in(str, typmod);
}
#endif
Interval *
pg_interval_in(const char *str, int32 typmod)
{
  Interval *result;
  struct pg_itm_in tt, *itm_in = &tt;
  int dtype;
  int nf;
  int range;
  int dterr;
  char *field[MAXDATEFIELDS];
  int ftype[MAXDATEFIELDS];
  char workbuf[256];
  DateTimeErrorExtra extra;

  itm_in->tm_year = 0;
  itm_in->tm_mon = 0;
  itm_in->tm_mday = 0;
  itm_in->tm_usec = 0;

  if (typmod >= 0)
    range = INTERVAL_RANGE(typmod);
  else
    range = INTERVAL_FULL_RANGE;

  dterr = pg_ParseDateTime(str, workbuf, sizeof(workbuf), field, ftype,
    MAXDATEFIELDS, &nf);
  if (dterr == 0)
    dterr = DecodeInterval(field, ftype, nf, range, &dtype, itm_in);

  /* if those functions think it's a bad format, try ISO8601 style */
  if (dterr == DTERR_BAD_FORMAT)
    dterr = DecodeISO8601Interval((char *) str, &dtype, itm_in);

  if (dterr != 0)
  {
    if (dterr == DTERR_FIELD_OVERFLOW)
      dterr = DTERR_INTERVAL_OVERFLOW;
    pg_DateTimeParseError(dterr, &extra, str, "interval", NULL);
    return NULL;
  }

  result = (Interval *) palloc(sizeof(Interval));

  switch (dtype)
  {
    case DTK_DELTA:
      if (itmin2interval(itm_in, result) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "interval out of range");
        return NULL;
      }
      break;

    case DTK_LATE:
      INTERVAL_NOEND(result);
      break;

    case DTK_EARLY:
      INTERVAL_NOBEGIN(result);
      break;

    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unexpected dtype %d while parsing interval \"%s\"", dtype,
        str);
      return NULL;
  }

  AdjustIntervalForTypmod(result, typmod, NULL);
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Return the string representation of an interval
 * @note Derived from PostgreSQL function @p interval_out()
 */
#if MEOS
char *
interval_out(const Interval *interv)
{
  return pg_interval_out(interv);
}
#endif
char *
pg_interval_out(const Interval *interv)
{
  struct pg_itm tt, *itm = &tt;
  char buf[MAXDATELEN + 1];
  if (INTERVAL_NOT_FINITE(interv))
    EncodeSpecialInterval(interv, buf);
  else
  {
    interval2itm(*interv, itm);
    pg_EncodeInterval(itm, IntervalStyle, buf);
  }
  char *result = pstrdup(buf);
  return result;
}

/* Output the typmod of an interval */
char *
interval_typmodout(int32 typmod)
{
  char *result = (char *) palloc(64);
  int fields;
  int precision;
  const char *fieldstr;

  if (typmod < 0)
  {
    *result = '\0';
    return result;
  }

  fields = INTERVAL_RANGE(typmod);
  precision = INTERVAL_PRECISION(typmod);

  switch (fields)
  {
    case INTERVAL_MASK(YEAR):
      fieldstr = " year";
      break;
    case INTERVAL_MASK(MONTH):
      fieldstr = " month";
      break;
    case INTERVAL_MASK(DAY):
      fieldstr = " day";
      break;
    case INTERVAL_MASK(HOUR):
      fieldstr = " hour";
      break;
    case INTERVAL_MASK(MINUTE):
      fieldstr = " minute";
      break;
    case INTERVAL_MASK(SECOND):
      fieldstr = " second";
      break;
    case INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH):
      fieldstr = " year to month";
      break;
    case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR):
      fieldstr = " day to hour";
      break;
    case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
      fieldstr = " day to minute";
      break;
    case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      fieldstr = " day to second";
      break;
    case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
      fieldstr = " hour to minute";
      break;
    case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      fieldstr = " hour to second";
      break;
    case INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      fieldstr = " minute to second";
      break;
    case INTERVAL_FULL_RANGE:
      fieldstr = "";
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "invalid INTERVAL typmod: 0x%x", typmod);
      fieldstr = "";
      break;
  }

  if (precision != INTERVAL_FULL_PRECISION)
    snprintf(result, 64, "%s(%d)", fieldstr, precision);
  else
    snprintf(result, 64, "%s", fieldstr);

  return result;
}

// /*
 // * Given an interval typmod value, return a code for the least-significant
 // * field that the typmod allows to be nonzero, for instance given
 // * INTERVAL DAY TO HOUR we want to identify "hour".
 // *
 // * The results should be ordered by field significance, which means
 // * we can't use the dt.h macros YEAR etc, because for some odd reason
 // * they aren't ordered that way.  Instead, arbitrarily represent
 // * SECOND = 0, MINUTE = 1, HOUR = 2, DAY = 3, MONTH = 4, YEAR = 5.
 // */
// static int
// intervaltypmodleastfield(int32 typmod)
// {
  // if (typmod < 0)
    // return 0;        /* SECOND */

  // switch (INTERVAL_RANGE(typmod))
  // {
    // case INTERVAL_MASK(YEAR):
      // return 5;      /* YEAR */
    // case INTERVAL_MASK(MONTH):
      // return 4;      /* MONTH */
    // case INTERVAL_MASK(DAY):
      // return 3;      /* DAY */
    // case INTERVAL_MASK(HOUR):
      // return 2;      /* HOUR */
    // case INTERVAL_MASK(MINUTE):
      // return 1;      /* MINUTE */
    // case INTERVAL_MASK(SECOND):
      // return 0;      /* SECOND */
    // case INTERVAL_MASK(YEAR) | INTERVAL_MASK(MONTH):
      // return 4;      /* MONTH */
    // case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR):
      // return 2;      /* HOUR */
    // case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
      // return 1;      /* MINUTE */
    // case INTERVAL_MASK(DAY) | INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      // return 0;      /* SECOND */
    // case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE):
      // return 1;      /* MINUTE */
    // case INTERVAL_MASK(HOUR) | INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      // return 0;      /* SECOND */
    // case INTERVAL_MASK(MINUTE) | INTERVAL_MASK(SECOND):
      // return 0;      /* SECOND */
    // case INTERVAL_FULL_RANGE:
      // return 0;      /* SECOND */
    // default:
      // meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        // "invalid INTERVAL typmod: 0x%x", typmod);
      // break;
  // }
  // return 0;          /* can't get here, but keep compiler quiet */
// }

/**
 * @ingroup meos_base_interval
 * @brief Return a copy of an interval
 * @return On error return NULL
 * @note MEOS function
 */
Interval *
interval_copy(const Interval *interv)
{
  Interval *result = palloc(sizeof(Interval));
  memcpy(result, interv, sizeof(Interval));
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Adjust an interval for specified fields
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_scale()
 */
#if MEOS
Interval *
interval_scale(const Interval *interv, int32 typmod)
{
  return pg_interval_scale(interv, typmod);
}
#endif
Interval *
pg_interval_scale(const Interval *interv, int32 typmod)
{
  Interval *result = palloc(sizeof(Interval));
  memcpy(result, interv, sizeof(Interval));
  if (AdjustIntervalForTypmod(result, typmod, NULL))
    return NULL;
  return result;
}

/*
 *  Adjust interval for specified precision, in both YEAR to SECOND
 *  range and sub-second precision.
 *
 * Returns true on success, false on failure (if escontext points to an
 * ErrorSaveContext; otherwise errors are thrown).
 */
static bool
AdjustIntervalForTypmod(Interval *interval, int32 typmod,
  Node *escontext UNUSED)
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

  /* Typmod has no effect on infinite intervals */
  if (INTERVAL_NOT_FINITE(interval))
    return true;

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
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized interval typmod: %d", typmod);

    /* Need to adjust sub-second precision? */
    if (precision != INTERVAL_FULL_PRECISION)
    {
      if (precision < 0 || precision > MAX_INTERVAL_PRECISION)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "interval(%d) precision must be between %d and %d",
          precision, 0, MAX_INTERVAL_PRECISION);
        return false;
      }

      if (interval->time >= INT64CONST(0))
      {
        if (pg_add_s64_overflow(interval->time, IntervalOffsets[precision],
              &interval->time))
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "interval out of range");
          return false;
        }

        interval->time -= interval->time % IntervalScales[precision];
      }
      else
      {
        if (pg_sub_s64_overflow(interval->time, IntervalOffsets[precision],
              &interval->time))
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "interval out of range");
          return false;
        }
        interval->time -= interval->time % IntervalScales[precision];
      }
    }
  }

  return true;
}

/**
 * @ingroup meos_base_interval
 * @brief Construct an interval from the arguments
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p make_interval()
 */
Interval *
interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours,
  int32 mins, float8 secs)
{
  /*
   * Reject out-of-range inputs.  We reject any input values that cause
   * integer overflow of the corresponding interval fields.
   */
  if (isinf(secs) || isnan(secs))
    goto out_of_range;

  Interval *result = (Interval *) palloc(sizeof(Interval));

  /* years and months -> months */
  if (pg_mul_s32_overflow(years, MONTHS_PER_YEAR, &result->month) ||
    pg_add_s32_overflow(result->month, months, &result->month))
    goto out_of_range;

  /* weeks and days -> days */
  if (pg_mul_s32_overflow(weeks, DAYS_PER_WEEK, &result->day) ||
    pg_add_s32_overflow(result->day, days, &result->day))
    goto out_of_range;

  /* hours and mins -> usecs (cannot overflow 64-bit) */
  result->time = hours * USECS_PER_HOUR + mins * USECS_PER_MINUTE;

  /* secs -> usecs */
  secs = rint(float8_mul(secs, USECS_PER_SEC));
  if (!FLOAT8_FITS_IN_INT64(secs) ||
    pg_add_s64_overflow(result->time, (int64) secs, &result->time))
    goto out_of_range;

  /* make sure that the result is finite */
  if (INTERVAL_NOT_FINITE(result))
    goto out_of_range;

  return result;

out_of_range:
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "interval out of range");
  return NULL;
}

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
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid argument for EncodeSpecialTimestamp");
}

static void
EncodeSpecialInterval(const Interval *interval, char *str)
{
  if (INTERVAL_IS_NOBEGIN(interval))
    strcpy(str, EARLY);
  else if (INTERVAL_IS_NOEND(interval))
    strcpy(str, LATE);
  else            /* shouldn't happen */
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid argument for EncodeSpecialInterval");
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

/**
 * @ingroup meos_base_timestamp
 * @brief Return the current time as a text
 * @note Derived from PostgreSQL function @p timeofday()
 */
text *
time_of_day(void)
{
  struct timeval tp;
  char templ[128];
  char buf[128];
  gettimeofday(&tp, NULL);
  pg_time_t tt = (pg_time_t) tp.tv_sec;
  pg_strftime(templ, sizeof(templ), "%a %b %d %H:%M:%S.%%06d %Y %Z",
    pg_localtime(&tt, session_timezone));
  snprintf(buf, sizeof(buf), templ, tp.tv_usec);
  return cstring_to_text(buf);
}

/*
 * TimestampDifference -- convert the difference between two timestamps
 *    into integer seconds and microseconds
 *
 * This is typically used to calculate a wait timeout for select(2),
 * which explains the otherwise-odd choice of output format.
 *
 * Both inputs must be ordinary finite timestamps (in current usage,
 * they'll be results from GetCurrentTimestamp()).
 *
 * We expect start_time <= stop_time.  If not, we return zeros,
 * since then we're already past the previously determined stop_time.
 */
void
TimestampDifference(TimestampTz start_time, TimestampTz stop_time,
  long *secs, int *microsecs)
{
  TimestampTz diff = stop_time - start_time;
  if (diff <= 0)
  {
    *secs = 0;
    *microsecs = 0;
  }
  else
  {
    *secs = (long) (diff / USECS_PER_SEC);
    *microsecs = (int) (diff % USECS_PER_SEC);
  }
}

/*
 * TimestampDifferenceMilliseconds -- convert the difference between two
 *     timestamps into integer milliseconds
 *
 * This is typically used to calculate a wait timeout for WaitLatch()
 * or a related function.  The choice of "long" as the result type
 * is to harmonize with that; furthermore, we clamp the result to at most
 * INT_MAX milliseconds, because that's all that WaitLatch() allows.
 *
 * We expect start_time <= stop_time.  If not, we return zero,
 * since then we're already past the previously determined stop_time.
 *
 * Subtracting finite and infinite timestamps works correctly, returning
 * zero or INT_MAX as appropriate.
 *
 * Note we round up any fractional millisecond, since waiting for just
 * less than the intended timeout is undesirable.
 */
long
TimestampDifferenceMilliseconds(TimestampTz start_time, TimestampTz stop_time)
{
  TimestampTz diff;
  /* Deal with zero or negative elapsed time quickly. */
  if (start_time >= stop_time)
    return 0;
  /* To not fail with timestamp infinities, we must detect overflow. */
  if (pg_sub_s64_overflow(stop_time, start_time, &diff))
    return (long) INT_MAX;
  if (diff >= (INT_MAX * INT64CONST(1000) - 999))
    return (long) INT_MAX;
  else
    return (long) ((diff + 999) / 1000);
}

/*
 * TimestampDifferenceExceeds -- report whether the difference between two
 *    timestamps is >= a threshold (expressed in milliseconds)
 *
 * Both inputs must be ordinary finite timestamps (in current usage,
 * they'll be results from GetCurrentTimestamp()).
 */
bool
TimestampDifferenceExceeds(TimestampTz start_time, TimestampTz stop_time,
  int msec)
{
  TimestampTz diff = stop_time - start_time;
  return (diff >= msec * INT64CONST(1000));
}

/*
 * Check if the difference between two timestamps is >= a given
 * threshold (expressed in seconds).
 */
bool
TimestampDifferenceExceedsSeconds(TimestampTz start_time,
  TimestampTz stop_time, int threshold_sec)
{
  /* Calculate the difference in seconds */
  long secs;
  int usecs;
  TimestampDifference(start_time, stop_time, &secs, &usecs);
  return (secs >= threshold_sec);
}

/*
 * Convert a time_t to TimestampTz.
 *
 * We do not use time_t internally in Postgres, but this is provided for use
 * by functions that need to interpret, say, a stat(2) result.
 *
 * To avoid having the function's ABI vary depending on the width of time_t,
 * we declare the argument as pg_time_t, which is cast-compatible with
 * time_t but always 64 bits wide (unless the platform has no 64-bit type).
 * This detail should be invisible to callers, at least at source code level.
 */
TimestampTz
time_t_to_timestamptz(pg_time_t tm)
{
  TimestampTz result = (TimestampTz) tm -
    ((POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY);
  result *= USECS_PER_SEC;
  return result;
}

/*
 * Convert a TimestampTz to time_t.
 *
 * This too is just marginally useful, but some places need it.
 *
 * To avoid having the function's ABI vary depending on the width of time_t,
 * we declare the result as pg_time_t, which is cast-compatible with
 * time_t but always 64 bits wide (unless the platform has no 64-bit type).
 * This detail should be invisible to callers, at least at source code level.
 */
pg_time_t
timestamptz_to_time_t(TimestampTz t)
{
  pg_time_t result = (pg_time_t) (t / USECS_PER_SEC +
    ((POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE) * SECS_PER_DAY));
  return result;
}

/*
 * Produce a C-string representation of a TimestampTz.
 *
 * This is mostly for use in emitting messages.  The primary difference
 * from timestamptz_out is that we force the output format to ISO.  Note
 * also that the result is in a static buffer, not pstrdup'd.
 *
 * See also pg_strftime.
 */
const char *
timestamptz_to_str(TimestampTz t)
{
  static char buf[MAXDATELEN + 1];
  int tz;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  const char *tzn;
  if (TIMESTAMP_NOT_FINITE(t))
    EncodeSpecialTimestamp(t, buf);
  else if (timestamp2tm(t, &tz, tm, &fsec, &tzn, NULL) == 0)
    EncodeDateTime(tm, fsec, true, tz, tzn, USE_ISO_DATES, buf);
  else
    strlcpy(buf, "(timestamp out of range)", sizeof(buf));
  return buf;
}

void
dt2time(Timestamp jd, int *hour, int *min, int *sec, fsec_t *fsec)
{
  TimeOffset time = jd;
  *hour = time / USECS_PER_HOUR;
  time -= (*hour) * USECS_PER_HOUR;
  *min = time / USECS_PER_MINUTE;
  time -= (*min) * USECS_PER_MINUTE;
  *sec = time / USECS_PER_SEC;
  *fsec = time - (*sec * USECS_PER_SEC);
  return;
}

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
timestamp2tm(Timestamp dt, int *tzp, struct pg_tm *tm, fsec_t *fsec,
  const char **tzn, pg_tz *attimezone)
{
  /* Use session timezone if caller asks for default */
  if (attimezone == NULL)
    attimezone = session_timezone;

  Timestamp time = dt;
  Timestamp date;
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
  pg_time_t utime = (pg_time_t) dt;
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
  /* Prevent overflow in Julian-day routines */
  if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
  {
    *result = 0;      /* keep compiler quiet */
    return -1;
  }

  TimeOffset date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;
  TimeOffset time = time2t(tm->tm_hour, tm->tm_min, tm->tm_sec, fsec);

  if (unlikely(pg_mul_s64_overflow(date, USECS_PER_DAY, result) ||
         pg_add_s64_overflow(*result, time, result)))
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


/* interval2itm()
 * Convert an Interval to a pg_itm structure.
 * Note: overflow is not possible, because the pg_itm fields are
 * wide enough for all possible conversion results.
 */
void
interval2itm(Interval interval, struct pg_itm *itm)
{
  itm->tm_year = interval.month / MONTHS_PER_YEAR;
  itm->tm_mon = interval.month % MONTHS_PER_YEAR;
  itm->tm_mday = interval.day;
  TimeOffset time = interval.time;
  TimeOffset tfrac = time / USECS_PER_HOUR;
  time -= tfrac * USECS_PER_HOUR;
  itm->tm_hour = tfrac;
  tfrac = time / USECS_PER_MINUTE;
  time -= tfrac * USECS_PER_MINUTE;
  itm->tm_min = (int) tfrac;
  tfrac = time / USECS_PER_SEC;
  time -= tfrac * USECS_PER_SEC;
  itm->tm_sec = (int) tfrac;
  itm->tm_usec = (int) time;
}

/* itm2interval()
 * Convert a pg_itm structure to an Interval.
 * Returns 0 if OK, -1 on overflow.
 *
 * This is for use in computations expected to produce finite results.  Any
 * inputs that lead to infinite results are treated as overflows.
 */
int
itm2interval(struct pg_itm *itm, Interval *interval)
{
  int64 total_months = (int64) itm->tm_year * MONTHS_PER_YEAR + itm->tm_mon;
  if (total_months > INT_MAX || total_months < INT_MIN)
    return -1;
  interval->month = (int32) total_months;
  interval->day = itm->tm_mday;
  if (pg_mul_s64_overflow(itm->tm_hour, USECS_PER_HOUR, &interval->time))
    return -1;
  /* tm_min, tm_sec are 32 bits, so intermediate products can't overflow */
  if (pg_add_s64_overflow(interval->time, itm->tm_min * USECS_PER_MINUTE,
        &interval->time))
    return -1;
  if (pg_add_s64_overflow(interval->time, itm->tm_sec * USECS_PER_SEC,
        &interval->time))
    return -1;
  if (pg_add_s64_overflow(interval->time, itm->tm_usec, &interval->time))
    return -1;
  if (INTERVAL_NOT_FINITE(interval))
    return -1;
  return 0;
}

/* itmin2interval()
 * Convert a pg_itm_in structure to an Interval.
 * Returns 0 if OK, -1 on overflow.
 *
 * Note: if the result is infinite, it is not treated as an overflow.  This
 * avoids any dump/reload hazards from pre-17 databases that do not support
 * infinite intervals, but do allow finite intervals with all fields set to
 * INT_MIN/INT_MAX (outside the documented range).  Such intervals will be
 * silently converted to +/-infinity.  This may not be ideal, but seems
 * preferable to failure, and ought to be pretty unlikely in practice.
 */
int
itmin2interval(struct pg_itm_in *itm_in, Interval *interval)
{
  int64 total_months = (int64) itm_in->tm_year * MONTHS_PER_YEAR + itm_in->tm_mon;
  if (total_months > INT_MAX || total_months < INT_MIN)
    return -1;
  interval->month = (int32) total_months;
  interval->day = itm_in->tm_mday;
  interval->time = itm_in->tm_usec;
  return 0;
}

static TimeOffset
time2t(const int hour, const int min, const int sec, const fsec_t fsec)
{
  return (((((hour * MINS_PER_HOUR) + min) * SECS_PER_MINUTE) + sec) *
    USECS_PER_SEC) + fsec;
}

static Timestamp
dt2local(Timestamp dt, int timezone)
{
  dt -= (timezone * USECS_PER_SEC);
  return dt;
}

/*****************************************************************************
 *   PUBLIC ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is finite
 * @note Derived from PostgreSQL function @p timestamp_finite()
 */
bool
timestamp_is_finite(Timestamp ts)
{
  return ! TIMESTAMP_NOT_FINITE(ts);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if an interval is finite
 * @note Derived from PostgreSQL function @p interval_finite()
 */
bool
interval_is_finite(const Interval *interv)
{
  return ! INTERVAL_NOT_FINITE(interv);
}

/*----------------------------------------------------------
 *  Relational operators for timestamp.
 *---------------------------------------------------------*/

void
GetEpochTime(struct pg_tm *tm)
{
  pg_time_t epoch = 0;
  struct pg_tm *t0 = pg_gmtime(&epoch);
  if (t0 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not convert epoch to timestamp: %m");
    return;
  }

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
  Timestamp dt;
  struct pg_tm tt, *tm = &tt;

  GetEpochTime(tm);
  /* we don't bother to test for failure ... */
  tm2timestamp(tm, 0, NULL, &dt);
  return dt;
}

/*
 * We are currently sharing some code between timestamp and timestamptz.
 * The comparison functions are among them. - thomas 2001-09-25
 *
 *    timestamp_relop - is timestamp1 relop timestamp2
 */
int
timestamp_cmp_internal(Timestamp ts1, Timestamp ts2)
{
  return (ts1 < ts2) ? -1 : ((ts1 > ts2) ? 1 : 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamps are equal
 * @note Derived from PostgreSQL function @p timestamp_eq()
 */
bool
eq_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) == 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamptz are equal
 * @note Derived from PostgreSQL function @p timestamp_eq()
 */
bool
eq_timestamptz_timestamptz(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) == 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamps are not equal
 * @note Derived from PostgreSQL function @p timestamp_ne()
 */
bool
ne_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) != 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamps are not equal
 * @note Derived from PostgreSQL function @p timestamp_ne()
 */
bool
ne_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) != 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is less than the second one
 * @note Derived from PostgreSQL function @p timestamp_lt()
 */
bool
lt_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) < 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is less than the second one
 * @note Derived from PostgreSQL function @p timestamp_lt()
 */
bool
lt_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) < 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is greater than the second one
 * @note Derived from PostgreSQL function @p timestamp_gt()
 */
bool
gt_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) > 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is greater than the second one
 * @note Derived from PostgreSQL function @p timestamp_gt()
 */
bool
gt_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) > 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is less than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p timestamp_le()
 */
bool
le_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) <= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is less than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p timestamp_le()
 */
bool
le_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) <= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is greater than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p timestamp_ge()
 */
bool
ge_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) >= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if the first timestamp is greater than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p timestamp_ge()
 */
bool
ge_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return (timestamp_cmp_internal(ts1, ts2) >= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return -1, 0, or 1 depending on whether the first timestamp is less
 * than, equal to, or less than the second one
 * @note Derived from PostgreSQL function @p timestamp_cmp()
 */
int32
cmp_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  return timestamp_cmp_internal(ts1, ts2);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return -1, 0, or 1 depending on whether the first timestamptz is less
 * than, equal to, or less than the second one
 * @note Derived from PostgreSQL function @p timestamp_cmp()
 */
int32
cmp_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return timestamp_cmp_internal(ts1, ts2);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the 32-bit hash value of a timestamp
 * @note Derived from PostgreSQL function @p timestamp_hash()
 */
#if MEOS
uint32
timestamp_hash(Timestamp ts)
{
  return int64_hash(ts);
}
#endif
uint32
pg_timestamp_hash(Timestamp ts)
{
  return int64_hash(ts);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the 32-bit hash value of a timestamptz
 * @note Derived from PostgreSQL function @p timestamptz_hash()
 */
#if MEOS
uint32
timestamptz_hash(TimestampTz ts)
{
  return int64_hash(ts);
}
#endif
uint32
pg_timestamptz_hash(TimestampTz ts)
{
  return int64_hash(ts);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the 64-bit hash value of a timestamp using a seed
 * @note Derived from PostgreSQL function @p timestamp_hash_extended()
 */
#if MEOS
uint64
timestamp_hash_extended(TimestampTz ts, uint64 seed)
{
  return int64_hash_extended(ts, seed);
}
#endif
uint64
pg_timestamp_hash_extended(TimestampTz ts, uint64 seed)
{
  return int64_hash_extended(ts, seed);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the 64-bit hash value of a timestamptz using a seed
 * @note Derived from PostgreSQL function @p timestamptz_hash_extended()
 */
#if MEOS
uint64
timestamptz_hash_extended(TimestampTz ts, uint64 seed)
{
  return int64_hash_extended(ts, seed);
}
#endif
uint64
pg_timestamptz_hash_extended(TimestampTz ts, uint64 seed)
{
  return int64_hash_extended(ts, seed);
}

/*
 * Cross-type comparison functions for timestamp vs timestamptz
 */

int32
timestamp_cmp_timestamptz_internal(Timestamp ts1, TimestampTz ts2)
{
  int overflow;
  TimestampTz tstz = timestamp2timestamptz_opt_overflow(ts1, &overflow);
  if (overflow > 0)
  {
    /* tstz is larger than any finite timestamp, but less than infinity */
    return TIMESTAMP_IS_NOEND(ts2) ? -1 : +1;
  }
  if (overflow < 0)
  {
    /* tstz is less than any finite timestamp, but more than -infinity */
    return TIMESTAMP_IS_NOBEGIN(ts2) ? +1 : -1;
  }

  return timestamptz_cmp_internal(tstz, ts2);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is equal to a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_eq_timestamptz()
 */
bool
eq_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) == 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is not equal to a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_ne_timestamptz()
 */
bool
ne_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) != 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is less than a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_lt_timestamptz()
 */
bool
lt_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) < 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is greater than a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_gt_timestamptz()
 */
bool
gt_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) > 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is less than or equal to a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_le_timestamptz()
 */
bool
le_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) <= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamp is greater than or equal to a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_ge_timestamptz()
 */
bool
ge_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts1, ts2) >= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return -1, 0, or 1 depending on whether a timestamp is less than,
 * equal to, or greater than a timestamptz
 * @note Derived from PostgreSQL function @p timestamp_cmp_timestamptz()
 */
int32
cmp_timestamp_timestamptz(Timestamp ts1, TimestampTz ts2)
{
  return timestamp_cmp_timestamptz_internal(ts1, ts2);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is equal to a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_eq_timestamp()
 */
bool
eq_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) == 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is not equal to a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_ne_timestamp()
 */
bool
ne_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) != 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is less than a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_lt_timestamp()
 */
bool
lt_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) > 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is greater than a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_gt_timestamp()
 */
bool
gt_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) < 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is less than or equal to a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_le_timestamp()
 */
bool
le_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) >= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if a timestamptz is greater than or equal to a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_ge_timestamp()
 */
bool
ge_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return (timestamp_cmp_timestamptz_internal(ts2, ts1) <= 0);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return -1, 0, or 1 depending on whether a timestamptz is less than,
 * equal to, or greater than a timestamp
 * @note Derived from PostgreSQL function @p timestamptz_cmp_timestamp()
 */
int32
cmp_timestamptz_timestamp(TimestampTz ts1, Timestamp ts2)
{
  return -timestamp_cmp_timestamptz_internal(ts2, ts1);
}

/*
 *    interval_relop  - is interv1 relop interv2
 *
 * Interval comparison is based on converting interval values to a linear
 * representation expressed in the units of the time field (microseconds,
 * in the case of integer timestamps) with days assumed to be always 24 hours
 * and months assumed to be always 30 days.  To avoid overflow, we need a
 * wider-than-int64 datatype for the linear representation, so use INT128.
 */

static inline INT128
interval_cmp_value(const Interval *interv)
{
  /*
   * Combine the month and day fields into an integral number of days.
   * Because the inputs are int32, int64 arithmetic suffices here.
   */
  int64 days = interv->month * INT64CONST(30);
  days += interv->day;

  /* Widen time field to 128 bits */
  INT128 span = int64_to_int128(interv->time);

  /* Scale up days to microseconds, forming a 128-bit product */
  int128_add_int64_mul_int64(&span, days, USECS_PER_DAY);

  return span;
}

static int
interval_cmp_internal(const Interval *interv1, const Interval *interv2)
{
  INT128 span1 = interval_cmp_value(interv1);
  INT128 span2 = interval_cmp_value(interv2);
  return int128_compare(span1, span2);
}

static int
interval_sign(const Interval *interv)
{
  INT128 span = interval_cmp_value(interv);
  INT128 zero = int64_to_int128(0);
  return int128_compare(span, zero);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if two intervals are equal
 * @note Derived from PostgreSQL function @p interval_eq()
 */
#if MEOS
bool
interval_eq(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) == 0);
}
#endif
bool
pg_interval_eq(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) == 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if two intervals are not equal
 * @note Derived from PostgreSQL function @p interval_ne()
 */
#if MEOS
bool
interval_ne(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) != 0);
}
#endif
bool
pg_interval_ne(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) != 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if the first interval is less than the second one
 * @note Derived from PostgreSQL function @p interval_lt()
 */
#if MEOS
bool
interval_lt(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) < 0);
}
#endif
bool
pg_interval_lt(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) < 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if the first interval is greater than the second one
 * @note Derived from PostgreSQL function @p interval_gt()
 */
#if MEOS
bool
interval_gt(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) > 0);
}
#endif
bool
pg_interval_gt(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) > 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if the first interval is less than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p interval_le()
 */
#if MEOS
bool
interval_le(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) <= 0);
}
#endif
bool
pg_interval_le(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) <= 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return true if the first interval is greater than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p interval_ge()
 */
#if MEOS
bool
interval_ge(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) >= 0);
}
#endif
bool
pg_interval_ge(const Interval *interv1, const Interval *interv2)
{
  return (interval_cmp_internal(interv1, interv2) >= 0);
}

/**
 * @ingroup meos_base_interval
 * @brief Return -1, 0, or 1 depending on whether the first interval is less
 * than, equal to, or less than the second one
 * @note Derived from PostgreSQL function @p interval_cmp()
 */
#if MEOS
int32
interval_cmp(const Interval *interv1, const Interval *interv2)
{
  return interval_cmp_internal(interv1, interv2);
}
#endif
int32
pg_interval_cmp(const Interval *interv1, const Interval *interv2)
{
  return interval_cmp_internal(interv1, interv2);
}

/**
 * @ingroup meos_base_interval
 * @brief Return the 32-bit hash value of an interval
 * @details We must produce equal hashvals for values that
 * interval_cmp_internal() considers equal.  So, compute the net interval the
 * same way it does, and then hash that.
 * @note Derived from PostgreSQL function @p interval_hash()
 */
#if MEOS
uint32
interval_hash(const Interval *interv)
{
  return pg_interval_hash(interv);
}
#endif
uint32
pg_interval_hash(const Interval *interv)
{
  INT128 span = interval_cmp_value(interv);
  /*
   * Use only the least significant 64 bits for hashing.  The upper 64 bits
   * seldom add any useful information, and besides we must do it like this
   * for compatibility with hashes calculated before use of INT128 was
   * introduced.
   */
  int64 span64 = int128_to_int64(span);
  return int64_hash(span64);
}

/**
 * @ingroup meos_base_interval
 * @brief Return the 64-bit hash value of an interval using a seed
 * @note Derived from PostgreSQL function @p interval_hash_extended()
 */
#if MEOS
uint64
interval_hash_extended(const Interval *interv, uint64 seed)
{
  return pg_interval_hash_extended(interv, seed);
}
#endif
uint64
pg_interval_hash_extended(const Interval *interv, uint64 seed)
{
  INT128 span = interval_cmp_value(interv);
  /* Same approach as interval_hash */
  int64 span64 = int128_to_int64(span);
  return int64_hash_extended(span64, seed);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamps overlap
 * @details Implements the SQL OVERLAPS operator, although in this case none
 * of the inputs is null
 * @note Derived from PostgreSQL function @p overlaps_timestamp()
 */
bool
timestamp_overlaps(Timestamp ts1, Timestamp te1, Timestamp ts2, Timestamp te2)
{
  /*
   * We can consider three cases: ts1 > ts2, ts1 < ts2, ts1 = ts2
   */
  if (gt_timestamp_timestamp(ts1, ts2))
  {
    /*
     * This case is ts1 < te2 OR te1 < te2, which may look redundant but
     * in the presence of nulls it's not quite completely so.
     */
    if (lt_timestamp_timestamp(ts1, te2))
      return (true);
    /*
     * We had ts1 <= te1 above, and we just found ts1 >= te2, hence te1 >= te2
     */
    return (false);
  }
  else if (lt_timestamp_timestamp(ts1, ts2))
  {
    /* This case is ts2 < te1 OR te2 < te1 */
    if (lt_timestamp_timestamp(ts2, te1))
      return (true);

    /*
     * We had ts2 <= te2 above, and we just found ts2 >= te1,chence te2 >= te1
     */
    return (false);
  }
  else
  {
    /*
     * For ts1 = ts2 the spec says te1 <> te2 OR te1 = te2, which is a
     * rather silly way of saying "true if both are non-null, else null".
     */
    return (true);
  }
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return true if two timestamptz overlap
 * @details Implements the SQL OVERLAPS operator, although in this case none
 * of the inputs is null
 * @note Derived from PostgreSQL function @p overlaps_timestamp()
 */
bool
timestamptz_overlaps(TimestampTz ts1, TimestampTz te1, TimestampTz ts2,
  TimestampTz te2)
{
  return timestamp_overlaps((Timestamp) ts1, (Timestamp) te1, (Timestamp) ts2,
    (Timestamp) te2);
}

/*----------------------------------------------------------
 *  "Arithmetic" operators on date/times.
 *---------------------------------------------------------*/

/**
 * @ingroup meos_base_timestamp
 * @brief Return the smaller of two timestamps
 * @note Derived from PostgreSQL function @p timestamp_smaller()
 */
#if MEOS
Timestamp
timestamp_smaller(Timestamp ts1, Timestamp ts2)
{
  return pg_timestamp_smaller(ts1, ts2);
}
#endif
Timestamp
pg_timestamp_smaller(Timestamp ts1, Timestamp ts2)
{
  Timestamp result;
  /* use timestamp_cmp_internal to be sure this agrees with comparisons */
  if (timestamp_cmp_internal(ts1, ts2) < 0)
    result = ts1;
  else
    result = ts2;
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the larger of two timestamps
 * @note Derived from PostgreSQL function @p timestamp_larger()
 */
#if MEOS
Timestamp
timestamp_larger(Timestamp ts1, Timestamp ts2)
{
  return pg_timestamp_larger(ts1, ts2);
}
#endif
Timestamp
pg_timestamp_larger(Timestamp ts1, Timestamp ts2)
{
  Timestamp result;
  if (timestamp_cmp_internal(ts1, ts2) > 0)
    result = ts1;
  else
    result = ts2;
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the subraction of two timestamps
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p timestamp_mi()
 */
Interval *
minus_timestamp_timestamp(Timestamp ts1, Timestamp ts2)
{
  Interval *res = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the interval type has nothing equivalent to NaN.
   */
  if (TIMESTAMP_NOT_FINITE(ts1) || TIMESTAMP_NOT_FINITE(ts2))
  {
    if (TIMESTAMP_IS_NOBEGIN(ts1))
    {
      if (TIMESTAMP_IS_NOBEGIN(ts2))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "interval out of range");
        pfree(res);
        return NULL;
      }
      else
        INTERVAL_NOBEGIN(res);
    }
    else if (TIMESTAMP_IS_NOEND(ts1))
    {
      if (TIMESTAMP_IS_NOEND(ts2))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "interval out of range");
        pfree(res);
        return NULL;
      }
      else
        INTERVAL_NOEND(res);
    }
    else if (TIMESTAMP_IS_NOBEGIN(ts2))
      INTERVAL_NOEND(res);
    else          /* TIMESTAMP_IS_NOEND(ts2) */
      INTERVAL_NOBEGIN(res);

    return res;
  }

  if (unlikely(pg_sub_s64_overflow(ts1, ts2, &res->time)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    pfree(res);
    return NULL;
  }

  res->month = 0;
  res->day = 0;

  /*----------
   *  This is wrong, but removing it breaks a lot of regression tests.
   *  For example:
   *
   *  test=> SET timezone = 'EST5EDT';
   *  test=> SELECT
   *  test-> ('2005-10-30 13:22:00-05'::timestamptz -
   *  test(>  '2005-10-29 13:22:00-04'::timestamptz);
   *  ?column?
   *  ----------------
   *   1 day 01:00:00
   *   (1 row)
   *
   *  so adding that to the first timestamp gets:
   *
   *   test=> SELECT
   *   test-> ('2005-10-29 13:22:00-04'::timestamptz +
   *   test(> ('2005-10-30 13:22:00-05'::timestamptz -
   *   test(>  '2005-10-29 13:22:00-04'::timestamptz)) at time zone 'EST';
   *    timezone
   *  --------------------
   *  2005-10-30 14:22:00
   *  (1 row)
   *----------
   */
  Interval *result = pg_interval_justify_hours(res);
  pfree(res);
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the subraction of two timestamps
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p timestamp_mi()
 */
Interval *
minus_timestamptz_timestamptz(TimestampTz ts1, TimestampTz ts2)
{
  return minus_timestamp_timestamp((Timestamp) ts1, (Timestamp) ts2);
}


/**
 * @ingroup meos_base_interval
 * @brief Adjust interval so 'month', 'day', and 'time' portions are within
 * customary bounds
 * @details Specifically:
 * - 0 <= abs(time) < 24 hours
 * - 0 <= abs(day)  < 30 days
 *  Also, the sign bit on all three fields is made equal, so either
 *  all three fields are negative or all are positive.
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_justify_interval()
 */
#if MEOS
Interval *
interval_justify_interval(const Interval *interv)
{
  return pg_interval_justify_interval(interv);
}
#endif /* MEOS */
Interval *
pg_interval_justify_interval(const Interval *interv)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));
  result->month = interv->month;
  result->day = interv->day;
  result->time = interv->time;

  /* do nothing for infinite intervals */
  if (INTERVAL_NOT_FINITE(result))
    return result;

  /* pre-justify days if it might prevent overflow */
  int32 wholemonth;
  if ((result->day > 0 && result->time > 0) ||
      (result->day < 0 && result->time < 0))
  {
    wholemonth = result->day / DAYS_PER_MONTH;
    result->day -= wholemonth * DAYS_PER_MONTH;
    if (pg_add_s32_overflow(result->month, wholemonth, &result->month))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(result);
      return NULL;
    }
  }

  /*
   * Since TimeOffset is int64, abs(wholeday) can't exceed about 1.07e8.  If
   * we pre-justified then abs(result->day) is less than DAYS_PER_MONTH, so
   * this addition can't overflow.  If we didn't pre-justify, then day and
   * time are of different signs, so it still can't overflow.
   */
  TimeOffset wholeday;
  TMODULO(result->time, wholeday, USECS_PER_DAY);
  result->day += wholeday;

  wholemonth = result->day / DAYS_PER_MONTH;
  result->day -= wholemonth * DAYS_PER_MONTH;
  if (pg_add_s32_overflow(result->month, wholemonth, &result->month))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    pfree(result);
    return NULL;
  }

  if (result->month > 0 &&
    (result->day < 0 || (result->day == 0 && result->time < 0)))
  {
    result->day += DAYS_PER_MONTH;
    result->month--;
  }
  else if (result->month < 0 &&
       (result->day > 0 || (result->day == 0 && result->time > 0)))
  {
    result->day -= DAYS_PER_MONTH;
    result->month++;
  }

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
 * @ingroup meos_base_interval
 * @brief Adjust interval so 'time' contains less than a whole day, adding
 *  the excess to 'day'
 * @details This is useful for situations (such as non-TZ) where '1 day' =
 * '24 hours' is valid, e.g. interval subtraction and division
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_justify_hours()
 */
#if MEOS
Interval *
interval_justify_hours(const Interval *interv)
{
  return pg_interval_justify_hours(interv);
}
#endif /* MEOS */
Interval *
pg_interval_justify_hours(const Interval *interv)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));
  result->month = interv->month;
  result->day = interv->day;
  result->time = interv->time;

  /* do nothing for infinite intervals */
  if (INTERVAL_NOT_FINITE(result))
    return result;

  TimeOffset wholeday;
  TMODULO(result->time, wholeday, USECS_PER_DAY);
  if (pg_add_s32_overflow(result->day, wholeday, &result->day))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return NULL;
  }

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
 * @ingroup meos_base_interval
 * @brief Adjust interval so 'day' contains less than 30 days, adding
 * the excess to 'month'
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_justify_days()
 */
#if MEOS
Interval *
interval_justify_days(const Interval *interv)
{
  return pg_interval_justify_days(interv);
}
#endif /* MEOS */
Interval *
pg_interval_justify_days(const Interval *interv)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));
  result->month = interv->month;
  result->day = interv->day;
  result->time = interv->time;

  /* do nothing for infinite intervals */
  if (INTERVAL_NOT_FINITE(result))
    return result;

  int32 wholemonth = result->day / DAYS_PER_MONTH;
  result->day -= wholemonth * DAYS_PER_MONTH;
  if (pg_add_s32_overflow(result->month, wholemonth, &result->month))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return NULL;
  }

  if (result->month > 0 && result->day < 0)
  {
    result->day += DAYS_PER_MONTH;
    result->month--;
  }
  else if (result->month < 0 && result->day > 0)
  {
    result->day -= DAYS_PER_MONTH;
    result->month++;
  }

  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Add an interval to a timestamp
 * @details Note that interval has provisions for qualitative year/month and
 * day units, so try to do the right thing with them.
 * To add a month, increment the month, and use the same day of month.
 * Then, if the next month has fewer days, set the day of month
 *  to the last day of month.
 * To add a day, increment the mday, and use the same time of day.
 * Lastly, add in the "quantitative time".
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_pl_interval()
 */
Timestamp
add_timestamp_interval(Timestamp ts, const Interval *interv)
{
  Timestamp result;

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the timestamp type has nothing equivalent to NaN.
   */
  if (INTERVAL_IS_NOBEGIN(interv))
  {
    if (TIMESTAMP_IS_NOEND(ts))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    else
      TIMESTAMP_NOBEGIN(result);
  }
  else if (INTERVAL_IS_NOEND(interv))
  {
    if (TIMESTAMP_IS_NOBEGIN(ts))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    else
      TIMESTAMP_NOEND(result);
  }
  else if (TIMESTAMP_NOT_FINITE(ts))
    result = ts;
  else
  {
    if (interv->month != 0)
    {
      struct pg_tm tt, *tm = &tt;
      fsec_t fsec;
      if (timestamp2tm(ts, NULL, tm, &fsec, NULL, NULL) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
      if (pg_add_s32_overflow(tm->tm_mon, interv->month, &tm->tm_mon))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
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

      if (tm2timestamp(tm, fsec, NULL, &ts) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    if (interv->day != 0)
    {
      struct pg_tm tt, *tm = &tt;
      fsec_t fsec;
      int julian;
      if (timestamp2tm(ts, NULL, tm, &fsec, NULL, NULL) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }

      /*
       * Add days by converting to and from Julian.  We need an overflow
       * check here since j2date expects a non-negative integer input.
       */
      julian = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
      if (pg_add_s32_overflow(julian, interv->day, &julian) ||
        julian < 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
      j2date(julian, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
      if (tm2timestamp(tm, fsec, NULL, &ts) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    if (pg_add_s64_overflow(ts, interv->time, &ts))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    if (! IS_VALID_TIMESTAMP(ts))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    result = ts;
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Subtract an interval to a timestamp
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_mi_interval()
 */
Timestamp
minus_timestamp_interval(Timestamp ts, const Interval *interv)
{
  Interval tspan;
  interval_um_internal(interv, &tspan);
  return add_timestamp_interval(ts, &tspan);
}


/* timestamptz_pl_interval_internal()
 * Add an interval to a timestamptz, in the given (or session) timezone.
 *
 * Note that interval has provisions for qualitative year/month and day
 *  units, so try to do the right thing with them.
 * To add a month, increment the month, and use the same day of month.
 * Then, if the next month has fewer days, set the day of month
 *  to the last day of month.
 * To add a day, increment the mday, and use the same time of day.
 * Lastly, add in the "quantitative time".
 */
static TimestampTz
timestamptz_pl_interval_internal(TimestampTz timestamp,
  Interval *interval, pg_tz *attimezone)
{
  TimestampTz result;
  int tz;

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the timestamptz type has nothing equivalent to NaN.
   */
  if (INTERVAL_IS_NOBEGIN(interval))
  {
    if (TIMESTAMP_IS_NOEND(timestamp))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    else
      TIMESTAMP_NOBEGIN(result);
  }
  else if (INTERVAL_IS_NOEND(interval))
  {
    if (TIMESTAMP_IS_NOBEGIN(timestamp))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    else
      TIMESTAMP_NOEND(result);
  }
  else if (TIMESTAMP_NOT_FINITE(timestamp))
    result = timestamp;
  else
  {
    /* Use session timezone if caller asks for default */
    if (attimezone == NULL)
      attimezone = session_timezone;

    if (interval->month != 0)
    {
      struct pg_tm tt, *tm = &tt;
      fsec_t fsec;
      if (timestamp2tm(timestamp, &tz, tm, &fsec, NULL, attimezone) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
      if (pg_add_s32_overflow(tm->tm_mon, interval->month, &tm->tm_mon))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
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
      tz = DetermineTimeZoneOffset(tm, attimezone);
      if (tm2timestamp(tm, fsec, &tz, &timestamp) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    if (interval->day != 0)
    {
      struct pg_tm tt, *tm = &tt;
      fsec_t fsec;
      int julian;
      if (timestamp2tm(timestamp, &tz, tm, &fsec, NULL, attimezone) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }

      /*
       * Add days by converting to and from Julian.  We need an overflow
       * check here since j2date expects a non-negative integer input.
       * In practice though, it will give correct answers for small
       * negative Julian dates; we should allow -1 to avoid
       * timezone-dependent failures, as discussed in timestamp.h.
       */
      julian = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday);
      if (pg_add_s32_overflow(julian, interval->day, &julian) ||
        julian < -1)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
      j2date(julian, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);
      tz = DetermineTimeZoneOffset(tm, attimezone);
      if (tm2timestamp(tm, fsec, &tz, &timestamp) != 0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    if (pg_add_s64_overflow(timestamp, interval->time, &timestamp))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    if (!IS_VALID_TIMESTAMP(timestamp))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    result = timestamp;
  }

  return result;
}

/* timestamptz_mi_interval_internal()
 * As above, but subtract the interval.
 */
static TimestampTz
timestamptz_mi_interval_internal(TimestampTz timestamp, Interval *interval,
  pg_tz *attimezone)
{
  Interval tspan;
  interval_um_internal(interval, &tspan);
  return timestamptz_pl_interval_internal(timestamp, &tspan, attimezone);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Add an interval to a timestamptz, in the current timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_pl_interval()
 */
TimestampTz
add_timestamptz_interval(TimestampTz ts, const Interval *interv)
{
  return timestamptz_pl_interval_internal(ts, (Interval *) interv, NULL);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Subtract an interval to a timestamptz, in the current timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_mi_interval()
 */
TimestampTz
minus_timestamptz_interval(TimestampTz ts, const Interval *interv)
{
  return timestamptz_mi_interval_internal(ts, (Interval *) interv, NULL);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Add an interval to a timestamptz, in the specified timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_pl_interval_at_zone()
 */
Timestamp
add_timestamptz_interval_at_zone(TimestampTz ts, const Interval *interv,
  const text *zone)
{
  pg_tz *attimezone = lookup_timezone((text *) zone);
  Timestamp result = timestamptz_pl_interval_internal(ts, (Interval *) interv,
    attimezone);
  pfree(attimezone);
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Subract an interval to a timestamptz, in the specified timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_mi_interval_at_zone()
 */
Timestamp
minus_timestamptz_interval_at_zone(TimestampTz ts, const Interval *interv,
  const text *zone)
{
  pg_tz *attimezone = lookup_timezone((text *) zone);
  Timestamp result = timestamptz_mi_interval_internal(ts, (Interval *) interv,
    attimezone);
  pfree(attimezone);
  return result;
}

/* interval_um_internal()
 * Negate an interval.
 */
static void
interval_um_internal(const Interval *interval, Interval *result)
{
  if (INTERVAL_IS_NOBEGIN(interval))
    INTERVAL_NOEND(result);
  else if (INTERVAL_IS_NOEND(interval))
    INTERVAL_NOBEGIN(result);
  else
  {
    /* Negate each field, guarding against overflow */
    if (pg_sub_s64_overflow(INT64CONST(0), interval->time, &result->time) ||
      pg_sub_s32_overflow(0, interval->day, &result->day) ||
      pg_sub_s32_overflow(0, interval->month, &result->month) ||
      INTERVAL_NOT_FINITE(result))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
    }
  }
}

/**
 * @ingroup meos_base_interval
 * @brief Return the unary minus of an interval
 * @note Derived from PostgreSQL function @p interval_um()
 */
Interval *
interval_negate(const Interval *interv)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));
  interval_um_internal(interv, result);
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Return the smaller of two intervals
 * @note Derived from PostgreSQL function @p interval_smaller()
 */
#if MEOS
Interval *
interval_smaller(const Interval *interv1, const Interval *interv2)
{
  return pg_interval_smaller(interv1, interv2);
}
#endif
Interval *
pg_interval_smaller(const Interval *interv1, const Interval *interv2)
{
  Interval *result;
  /* use interval_cmp_internal to be sure this agrees with comparisons */
  if (interval_cmp_internal(interv1, interv2) < 0)
    result = interval_copy(interv1);
  else
    result = interval_copy(interv2);
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Return the larger of two intervals
 * @note Derived from PostgreSQL function @p interval_larger()
 */
 #if MEOS
Interval *
interval_larger(const Interval *interv1, const Interval *interv2)
{
  return pg_interval_larger(interv1, interv2);
}
#endif

Interval *
pg_interval_larger(const Interval *interv1, const Interval *interv2)
{
  Interval *result;
  if (interval_cmp_internal(interv1, interv2) > 0)
    result = interval_copy(interv1);
  else
    result = interval_copy(interv2);
  return result;
}

static void
finite_interval_pl(const Interval *interv1, const Interval *interv2, Interval *result)
{
  Assert(!INTERVAL_NOT_FINITE(interv1));
  Assert(!INTERVAL_NOT_FINITE(interv2));

  if (pg_add_s32_overflow(interv1->month, interv2->month, &result->month) ||
    pg_add_s32_overflow(interv1->day, interv2->day, &result->day) ||
    pg_add_s64_overflow(interv1->time, interv2->time, &result->time) ||
    INTERVAL_NOT_FINITE(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
  }
}

/**
 * @ingroup meos_base_interval
 * @brief Return the addition of two intervals
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_pl()
 */
Interval *
add_interval_interval(const Interval *interv1, const Interval *interv2)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the interval type has nothing equivalent to NaN.
   */
  if (INTERVAL_IS_NOBEGIN(interv1))
  {
    if (INTERVAL_IS_NOEND(interv2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(result);
      return NULL;
    }
    else
      INTERVAL_NOBEGIN(result);
  }
  else if (INTERVAL_IS_NOEND(interv1))
  {
    if (INTERVAL_IS_NOBEGIN(interv2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(result);
      return NULL;
    }
    else
      INTERVAL_NOEND(result);
  }
  else if (INTERVAL_NOT_FINITE(interv2))
    memcpy(result, interv2, sizeof(Interval));
  else
    finite_interval_pl(interv1, interv2, result);

  return result;
}

static void
finite_interval_mi(const Interval *interv1, const Interval *interv2,
  Interval *result)
{
  Assert(!INTERVAL_NOT_FINITE(interv1));
  Assert(!INTERVAL_NOT_FINITE(interv2));

  if (pg_sub_s32_overflow(interv1->month, interv2->month, &result->month) ||
    pg_sub_s32_overflow(interv1->day, interv2->day, &result->day) ||
    pg_sub_s64_overflow(interv1->time, interv2->time, &result->time) ||
    INTERVAL_NOT_FINITE(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
  }
  return;
}

/**
 * @ingroup meos_base_interval
 * @brief Return the suctraction of two intervals
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_mi()
 */
Interval *
minus_interval_interval(const Interval *interv1, const Interval *interv2)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the interval type has nothing equivalent to NaN.
   */
  if (INTERVAL_IS_NOBEGIN(interv1))
  {
    if (INTERVAL_IS_NOBEGIN(interv2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(result);
      return NULL;
    }
    else
      INTERVAL_NOBEGIN(result);
  }
  else if (INTERVAL_IS_NOEND(interv1))
  {
    if (INTERVAL_IS_NOEND(interv2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(result);
      return NULL;
    }
    else
      INTERVAL_NOEND(result);
  }
  else if (INTERVAL_IS_NOBEGIN(interv2))
    INTERVAL_NOEND(result);
  else if (INTERVAL_IS_NOEND(interv2))
    INTERVAL_NOBEGIN(result);
  else
    finite_interval_mi(interv1, interv2, result);

  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Multiply an interval by a float8
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_mul()
 */
Interval *
mul_interval_float8(const Interval *interv, float8 factor)
{
  double month_remainder_days, sec_remainder, result_double;
  int32 orig_month = interv->month;
  int32 orig_day = interv->day;
  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle NaN and infinities.
   *
   * We treat "0 * infinity" and "infinity * 0" as errors, since the
   * interval type has nothing equivalent to NaN.
   */
  if (isnan(factor))
    goto out_of_range;
  if (INTERVAL_NOT_FINITE(interv))
  {
    if (factor == 0.0)
      goto out_of_range;
    if (factor < 0.0)
      interval_um_internal(interv, result);
    else
      memcpy(result, interv, sizeof(Interval));
    return result;
  }
  if (isinf(factor))
  {
    int isign = interval_sign(interv);
    if (isign == 0)
      goto out_of_range;
    if (factor * isign < 0)
      INTERVAL_NOBEGIN(result);
    else
      INTERVAL_NOEND(result);
    return result;
  }

  result_double = interv->month * factor;
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT32(result_double))
    goto out_of_range;
  result->month = (int32) result_double;

  result_double = interv->day * factor;
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT32(result_double))
    goto out_of_range;
  result->day = (int32) result_double;

  /*
   * The above correctly handles the whole-number part of the month and day
   * products, but we have to do something with any fractional part
   * resulting when the factor is non-integral.  We cascade the fractions
   * down to lower units using the conversion factors DAYS_PER_MONTH and
   * SECS_PER_DAY.  Note we do NOT cascade up, since we are not forced to do
   * so by the representation.  The user can choose to cascade up later,
   * using justify_hours and/or justify_days.
   */

  /*
   * Fractional months full days into days.
   *
   * Floating point calculation are inherently imprecise, so these
   * calculations are crafted to produce the most reliable result possible.
   * TSROUND() is needed to more accurately produce whole numbers where
   * appropriate.
   */
  month_remainder_days = (orig_month * factor - result->month) * DAYS_PER_MONTH;
  month_remainder_days = TSROUND(month_remainder_days);
  sec_remainder = (orig_day * factor - result->day + month_remainder_days -
    (int) month_remainder_days) * SECS_PER_DAY;
  sec_remainder = TSROUND(sec_remainder);

  /*
   * Might have 24:00:00 hours due to rounding, or >24 hours because of time
   * cascade from months and days.  It might still be >24 if the combination
   * of cascade and the seconds factor operation itself.
   */
  if (fabs(sec_remainder) >= SECS_PER_DAY)
  {
    if (pg_add_s32_overflow(result->day, (int) (sec_remainder / SECS_PER_DAY),
        &result->day))
      goto out_of_range;
    sec_remainder -= (int) (sec_remainder / SECS_PER_DAY) * SECS_PER_DAY;
  }

  /* cascade units down */
  if (pg_add_s32_overflow(result->day, (int32) month_remainder_days,
        &result->day))
    goto out_of_range;
  result_double = rint(interv->time * factor + sec_remainder * USECS_PER_SEC);
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT64(result_double))
    goto out_of_range;
  result->time = (int64) result_double;

  if (INTERVAL_NOT_FINITE(result))
    goto out_of_range;

  return result;

out_of_range:
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "interval out of range");
  pfree(result);
  return NULL;
}

/**
 * @ingroup meos_base_interval
 * @brief Multiply a float8 by an interval
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p mul_d_interval()
 */
Interval *
mul_float8_interval(float8 factor, const Interval *interv)
{
  return mul_interval_float8(interv, factor);
}

/**
 * @ingroup meos_base_interval
 * @brief Divide an interval by a float8
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p interval_div()
 */
Interval *
div_interval_float8(const Interval *interv, float8 factor)
{
  if (factor == 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "division by zero");
    return NULL;
  }

  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle NaN and infinities.
   *
   * We treat "infinity / infinity" as an error, since the interval type has
   * nothing equivalent to NaN.  Otherwise, dividing by infinity is handled
   * by the regular division code, causing all fields to be set to zero.
   */
  if (isnan(factor))
    goto out_of_range;
  if (INTERVAL_NOT_FINITE(interv))
  {
    if (isinf(factor))
      goto out_of_range;
    if (factor < 0.0)
      interval_um_internal(interv, result);
    else
      memcpy(result, interv, sizeof(Interval));
    return result;
  }

  double result_double = interv->month / factor;
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT32(result_double))
    goto out_of_range;
  result->month = (int32) result_double;

  result_double = interv->day / factor;
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT32(result_double))
    goto out_of_range;
  result->day = (int32) result_double;

  /*
   * Fractional months full days into days.  See comment in interval_mul().
   */
  int32 orig_month = interv->month, orig_day = interv->day;
  double month_remainder_days = (orig_month / factor - result->month) *
    DAYS_PER_MONTH;
  month_remainder_days = TSROUND(month_remainder_days);
  double sec_remainder = (orig_day / factor - result->day +
    month_remainder_days - (int) month_remainder_days) * SECS_PER_DAY;
  sec_remainder = TSROUND(sec_remainder);
  if (fabs(sec_remainder) >= SECS_PER_DAY)
  {
    if (pg_add_s32_overflow(result->day,
        (int) (sec_remainder / SECS_PER_DAY), &result->day))
      goto out_of_range;
    sec_remainder -= (int) (sec_remainder / SECS_PER_DAY) * SECS_PER_DAY;
  }

  /* cascade units down */
  if (pg_add_s32_overflow(result->day, (int32) month_remainder_days,
        &result->day))
    goto out_of_range;
  result_double = rint(interv->time / factor + sec_remainder * USECS_PER_SEC);
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT64(result_double))
    goto out_of_range;
  result->time = (int64) result_double;

  if (INTERVAL_NOT_FINITE(result))
    goto out_of_range;

  return result;

out_of_range:
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "interval out of range");
  pfree(result);
  return NULL;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return the time difference between two timestamps while retaining
 * the year/month fields
 * @details Note that this does not result in an accurate absolute time interval
 * since year and month are out of context once the arithmetic is done
 * @return On error return NULL
 * @note Derived from PostgreSQL function @p timestamp_age()
 */
#if MEOS
Interval *
timestamp_age(Timestamp ts1, Timestamp ts2)
{
  return pg_timestamp_age(ts1, ts2);
}
#endif
Interval *
pg_timestamp_age(Timestamp ts1, Timestamp ts2)
{
  fsec_t fsec1, fsec2;
  struct pg_itm tt, *tm = &tt;
  struct pg_tm tt1, *tm1 = &tt1;
  struct pg_tm tt2, *tm2 = &tt2;

  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the interval type has nothing equivalent to NaN.
   */
  if (TIMESTAMP_IS_NOBEGIN(ts1))
  {
    if (TIMESTAMP_IS_NOBEGIN(ts2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
    else
      INTERVAL_NOBEGIN(result);
  }
  else if (TIMESTAMP_IS_NOEND(ts1))
  {
    if (TIMESTAMP_IS_NOEND(ts2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
    else
      INTERVAL_NOEND(result);
  }
  else if (TIMESTAMP_IS_NOBEGIN(ts2))
    INTERVAL_NOEND(result);
  else if (TIMESTAMP_IS_NOEND(ts2))
    INTERVAL_NOBEGIN(result);
  else if (timestamp2tm(ts1, NULL, tm1, &fsec1, NULL, NULL) == 0 &&
       timestamp2tm(ts2, NULL, tm2, &fsec2, NULL, NULL) == 0)
  {
    /* form the symbolic difference */
    tm->tm_usec = fsec1 - fsec2;
    tm->tm_sec = tm1->tm_sec - tm2->tm_sec;
    tm->tm_min = tm1->tm_min - tm2->tm_min;
    tm->tm_hour = tm1->tm_hour - tm2->tm_hour;
    tm->tm_mday = tm1->tm_mday - tm2->tm_mday;
    tm->tm_mon = tm1->tm_mon - tm2->tm_mon;
    tm->tm_year = tm1->tm_year - tm2->tm_year;

    /* flip sign if necessary... */
    if (ts1 < ts2)
    {
      tm->tm_usec = -tm->tm_usec;
      tm->tm_sec = -tm->tm_sec;
      tm->tm_min = -tm->tm_min;
      tm->tm_hour = -tm->tm_hour;
      tm->tm_mday = -tm->tm_mday;
      tm->tm_mon = -tm->tm_mon;
      tm->tm_year = -tm->tm_year;
    }

    /* propagate any negative fields into the next higher field */
    while (tm->tm_usec < 0)
    {
      tm->tm_usec += USECS_PER_SEC;
      tm->tm_sec--;
    }

    while (tm->tm_sec < 0)
    {
      tm->tm_sec += SECS_PER_MINUTE;
      tm->tm_min--;
    }

    while (tm->tm_min < 0)
    {
      tm->tm_min += MINS_PER_HOUR;
      tm->tm_hour--;
    }

    while (tm->tm_hour < 0)
    {
      tm->tm_hour += HOURS_PER_DAY;
      tm->tm_mday--;
    }

    while (tm->tm_mday < 0)
    {
      if (ts1 < ts2)
      {
        tm->tm_mday += day_tab[isleap(tm1->tm_year)][tm1->tm_mon - 1];
        tm->tm_mon--;
      }
      else
      {
        tm->tm_mday += day_tab[isleap(tm2->tm_year)][tm2->tm_mon - 1];
        tm->tm_mon--;
      }
    }

    while (tm->tm_mon < 0)
    {
      tm->tm_mon += MONTHS_PER_YEAR;
      tm->tm_year--;
    }

    /* recover sign if necessary... */
    if (ts1 < ts2)
    {
      tm->tm_usec = -tm->tm_usec;
      tm->tm_sec = -tm->tm_sec;
      tm->tm_min = -tm->tm_min;
      tm->tm_hour = -tm->tm_hour;
      tm->tm_mday = -tm->tm_mday;
      tm->tm_mon = -tm->tm_mon;
      tm->tm_year = -tm->tm_year;
    }

    if (itm2interval(tm, result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return NULL;
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Calculate time difference while retaining year/month fields
 * @details Note that this does not result in an accurate absolute time
 * interval since year and month are out of context once the arithmetic is done
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p timestamptz_age()
 */
#if MEOS
Interval *
timestamptz_age(TimestampTz ts1, TimestampTz ts2)
{
  return pg_timestamptz_age(ts1, ts2);
}
#endif
Interval *
pg_timestamptz_age(TimestampTz ts1, TimestampTz ts2)
{
  fsec_t fsec1, fsec2;
  struct pg_itm tt, *tm = &tt;
  struct pg_tm tt1, *tm1 = &tt1;
  struct pg_tm tt2, *tm2 = &tt2;
  int tz1;
  int tz2;
  Interval *result = (Interval *) palloc(sizeof(Interval));

  /*
   * Handle infinities.
   *
   * We treat anything that amounts to "infinity - infinity" as an error,
   * since the interval type has nothing equivalent to NaN.
   */
  if (TIMESTAMP_IS_NOBEGIN(ts1))
  {
    if (TIMESTAMP_IS_NOBEGIN(ts2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
    else
      INTERVAL_NOBEGIN(result);
  }
  else if (TIMESTAMP_IS_NOEND(ts1))
  {
    if (TIMESTAMP_IS_NOEND(ts2))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
    else
      INTERVAL_NOEND(result);
  }
  else if (TIMESTAMP_IS_NOBEGIN(ts2))
    INTERVAL_NOEND(result);
  else if (TIMESTAMP_IS_NOEND(ts2))
    INTERVAL_NOBEGIN(result);
  else if (timestamp2tm(ts1, &tz1, tm1, &fsec1, NULL, NULL) == 0 &&
           timestamp2tm(ts2, &tz2, tm2, &fsec2, NULL, NULL) == 0)
  {
    /* form the symbolic difference */
    tm->tm_usec = fsec1 - fsec2;
    tm->tm_sec = tm1->tm_sec - tm2->tm_sec;
    tm->tm_min = tm1->tm_min - tm2->tm_min;
    tm->tm_hour = tm1->tm_hour - tm2->tm_hour;
    tm->tm_mday = tm1->tm_mday - tm2->tm_mday;
    tm->tm_mon = tm1->tm_mon - tm2->tm_mon;
    tm->tm_year = tm1->tm_year - tm2->tm_year;

    /* flip sign if necessary... */
    if (ts1 < ts2)
    {
      tm->tm_usec = -tm->tm_usec;
      tm->tm_sec = -tm->tm_sec;
      tm->tm_min = -tm->tm_min;
      tm->tm_hour = -tm->tm_hour;
      tm->tm_mday = -tm->tm_mday;
      tm->tm_mon = -tm->tm_mon;
      tm->tm_year = -tm->tm_year;
    }

    /* propagate any negative fields into the next higher field */
    while (tm->tm_usec < 0)
    {
      tm->tm_usec += USECS_PER_SEC;
      tm->tm_sec--;
    }

    while (tm->tm_sec < 0)
    {
      tm->tm_sec += SECS_PER_MINUTE;
      tm->tm_min--;
    }

    while (tm->tm_min < 0)
    {
      tm->tm_min += MINS_PER_HOUR;
      tm->tm_hour--;
    }

    while (tm->tm_hour < 0)
    {
      tm->tm_hour += HOURS_PER_DAY;
      tm->tm_mday--;
    }

    while (tm->tm_mday < 0)
    {
      if (ts1 < ts2)
      {
        tm->tm_mday += day_tab[isleap(tm1->tm_year)][tm1->tm_mon - 1];
        tm->tm_mon--;
      }
      else
      {
        tm->tm_mday += day_tab[isleap(tm2->tm_year)][tm2->tm_mon - 1];
        tm->tm_mon--;
      }
    }

    while (tm->tm_mon < 0)
    {
      tm->tm_mon += MONTHS_PER_YEAR;
      tm->tm_year--;
    }

    /*
     * Note: we deliberately ignore any difference between tz1 and tz2.
     */

    /* recover sign if necessary... */
    if (ts1 < ts2)
    {
      tm->tm_usec = -tm->tm_usec;
      tm->tm_sec = -tm->tm_sec;
      tm->tm_min = -tm->tm_min;
      tm->tm_hour = -tm->tm_hour;
      tm->tm_mday = -tm->tm_mday;
      tm->tm_mon = -tm->tm_mon;
      tm->tm_year = -tm->tm_year;
    }

    if (itm2interval(tm, result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      return NULL;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return NULL;
  }
  return result;
}

/*----------------------------------------------------------
 *  Conversion operators.
 *---------------------------------------------------------*/

/**
 * @ingroup meos_base_timestamp
 * @brief Bin a timestamp into specified interval
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_bin()
 */
#if MEOS
Timestamp
timestamp_bin(Timestamp ts, const Interval *stride, Timestamp origin)
{
  return pg_timestamp_bin(ts, stride, origin);
}
#endif
Timestamp
pg_timestamp_bin(Timestamp ts, const Interval *stride, Timestamp origin)
{
  Timestamp result, stride_usecs, tm_diff, tm_modulo, tm_delta;

  if (TIMESTAMP_NOT_FINITE(ts))
    return ts;

  if (TIMESTAMP_NOT_FINITE(origin))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "origin out of range");
    return DT_NOEND;
  }

  if (INTERVAL_NOT_FINITE(stride))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamps cannot be binned into infinite intervals");
    return DT_NOEND;
  }

  if (stride->month != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamps cannot be binned into intervals containing months or years");
    return DT_NOEND;
  }
  if (unlikely(pg_mul_s64_overflow(stride->day, USECS_PER_DAY, &stride_usecs)) ||
    unlikely(pg_add_s64_overflow(stride_usecs, stride->time, &stride_usecs)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return DT_NOEND;
  }
  if (stride_usecs <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "stride must be greater than zero");
    return DT_NOEND;
  }
  if (unlikely(pg_sub_s64_overflow(ts, origin, &tm_diff)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return DT_NOEND;
  }

  /* These calculations cannot overflow */
  tm_modulo = tm_diff % stride_usecs;
  tm_delta = tm_diff - tm_modulo;
  result = origin + tm_delta;

  /*
   * We want to round towards -infinity, not 0, when tm_diff is negative and
   * not a multiple of stride_usecs.  This adjustment *can* cause overflow,
   * since the result might now be out of the range origin .. timestamp.
   */
  if (tm_modulo < 0)
  {
    if (unlikely(pg_sub_s64_overflow(result, stride_usecs, &result)) ||
      !IS_VALID_TIMESTAMP(result))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Truncate a timestamp to specified units
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_trunc()
 */
#if MEOS
Timestamp
timestamp_trunc(Timestamp ts, const text *units)
{
  return pg_timestamp_trunc(ts, units);
}
#endif
Timestamp
pg_timestamp_trunc(Timestamp ts, const text *units)
{
  Timestamp result;
  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);

  int val;
  int type = DecodeUnits(0, lowunits, &val);
  if (type == UNITS)
  {
    if (TIMESTAMP_NOT_FINITE(ts))
    {
      /*
       * Errors thrown here for invalid units should exactly match those
       * below, else there will be unexpected discrepancies between
       * finite- and infinite-input cases.
       */
      switch (val)
      {
        case DTK_WEEK:
        case DTK_MILLENNIUM:
        case DTK_CENTURY:
        case DTK_DECADE:
        case DTK_YEAR:
        case DTK_QUARTER:
        case DTK_MONTH:
        case DTK_DAY:
        case DTK_HOUR:
        case DTK_MINUTE:
        case DTK_SECOND:
        case DTK_MILLISEC:
        case DTK_MICROSEC:
          pfree(lowunits);
          return ts;
          break;
        default:
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "unit \"%s\" not supported for type %s",
            // lowunits, format_type_be(TIMESTAMPOID));
            lowunits, "timestamp without time zone");
          result = 0;
      }
    }

    fsec_t fsec;
    struct pg_tm tt, *tm = &tt;
    if (timestamp2tm(ts, NULL, tm, &fsec, NULL, NULL) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      pfree(lowunits);
      return DT_NOEND;
    }

    switch (val)
    {
      case DTK_WEEK:
        {
          int woy = date2isoweek(tm->tm_year, tm->tm_mon, tm->tm_mday);

          /*
           * If it is week 52/53 and the month is January, then the
           * week must belong to the previous year. Also, some
           * December dates belong to the next year.
           */
          if (woy >= 52 && tm->tm_mon == 1)
            --tm->tm_year;
          if (woy <= 1 && tm->tm_mon == MONTHS_PER_YEAR)
            ++tm->tm_year;
          isoweek2date(woy, &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));
          tm->tm_hour = 0;
          tm->tm_min = 0;
          tm->tm_sec = 0;
          fsec = 0;
          break;
        }
      case DTK_MILLENNIUM:
        /* see comments in timestamptz_trunc */
        if (tm->tm_year > 0)
          tm->tm_year = ((tm->tm_year + 999) / 1000) * 1000 - 999;
        else
          tm->tm_year = -((999 - (tm->tm_year - 1)) / 1000) * 1000 + 1;
        /* FALL THRU */
      case DTK_CENTURY:
        /* see comments in timestamptz_trunc */
        if (tm->tm_year > 0)
          tm->tm_year = ((tm->tm_year + 99) / 100) * 100 - 99;
        else
          tm->tm_year = -((99 - (tm->tm_year - 1)) / 100) * 100 + 1;
        /* FALL THRU */
      case DTK_DECADE:
        /* see comments in timestamptz_trunc */
        if (val != DTK_MILLENNIUM && val != DTK_CENTURY)
        {
          if (tm->tm_year > 0)
            tm->tm_year = (tm->tm_year / 10) * 10;
          else
            tm->tm_year = -((8 - (tm->tm_year - 1)) / 10) * 10;
        }
        /* FALL THRU */
      case DTK_YEAR:
        tm->tm_mon = 1;
        /* FALL THRU */
      case DTK_QUARTER:
        tm->tm_mon = (3 * ((tm->tm_mon - 1) / 3)) + 1;
        /* FALL THRU */
      case DTK_MONTH:
        tm->tm_mday = 1;
        /* FALL THRU */
      case DTK_DAY:
        tm->tm_hour = 0;
        /* FALL THRU */
      case DTK_HOUR:
        tm->tm_min = 0;
        /* FALL THRU */
      case DTK_MINUTE:
        tm->tm_sec = 0;
        /* FALL THRU */
      case DTK_SECOND:
        fsec = 0;
        break;

      case DTK_MILLISEC:
        fsec = (fsec / 1000) * 1000;
        break;

      case DTK_MICROSEC:
        break;

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPOID));
          lowunits, "timestamp without time zone");
        result = 0;
    }

    if (tm2timestamp(tm, fsec, NULL, &result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      pfree(lowunits);
      return DT_NOEND;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(TIMESTAMPOID));
      lowunits, "timestamp without time zone");
    result = 0;
  }

  pfree(lowunits);
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Return a timestamptz binned into an interval using an origin
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p timestamptz_bin()
 */
#if MEOS
TimestampTz
timestamptz_bin(TimestampTz ts, const Interval *stride, TimestampTz origin)
{
  return pg_timestamptz_bin(ts, stride, origin);
}
#endif
TimestampTz
pg_timestamptz_bin(TimestampTz ts, const Interval *stride, TimestampTz origin)
{
  TimestampTz result, stride_usecs, tm_diff, tm_modulo, tm_delta;

  if (TIMESTAMP_NOT_FINITE(ts))
    return ts;

  if (TIMESTAMP_NOT_FINITE(origin))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "origin out of range");
    return DT_NOEND;
  }
  if (INTERVAL_NOT_FINITE(stride))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamps cannot be binned into infinite intervals");
    return DT_NOEND;
  }
  if (stride->month != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamps cannot be binned into intervals containing months or years");
    return DT_NOEND;
  }
  if (unlikely(pg_mul_s64_overflow(stride->day, USECS_PER_DAY, &stride_usecs)) ||
    unlikely(pg_add_s64_overflow(stride_usecs, stride->time, &stride_usecs)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return DT_NOEND;
  }
  if (stride_usecs <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "stride must be greater than zero");
    return DT_NOEND;
  }
  if (unlikely(pg_sub_s64_overflow(ts, origin, &tm_diff)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval out of range");
    return DT_NOEND;
  }

  /* These calculations cannot overflow */
  tm_modulo = tm_diff % stride_usecs;
  tm_delta = tm_diff - tm_modulo;
  result = origin + tm_delta;

  /*
   * We want to round towards -infinity, not 0, when tm_diff is negative and
   * not a multiple of stride_usecs.  This adjustment *can* cause overflow,
   * since the result might now be out of the range origin .. timestamp.
   */
  if (tm_modulo < 0)
  {
    if (unlikely(pg_sub_s64_overflow(result, stride_usecs, &result)) ||
      !IS_VALID_TIMESTAMP(result))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
  }
  return result;
}

/*
 * Common code for timestamptz_trunc() and timestamptz_trunc_zone().
 *
 * tzp identifies the zone to truncate with respect to.  We assume
 * infinite timestamps have already been rejected.
 */
static TimestampTz
timestamptz_trunc_internal(TimestampTz ts, const text *units, pg_tz *tzp)
{
  TimestampTz result;
  int tz;
  bool redotz = false;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;

  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);

  int val;
  int type = DecodeUnits(0, lowunits, &val);

  if (type == UNITS)
  {
    if (TIMESTAMP_NOT_FINITE(ts))
    {
      /*
       * Errors thrown here for invalid units should exactly match those
       * below, else there will be unexpected discrepancies between
       * finite- and infinite-input cases.
       */
      switch (val)
      {
        case DTK_WEEK:
        case DTK_MILLENNIUM:
        case DTK_CENTURY:
        case DTK_DECADE:
        case DTK_YEAR:
        case DTK_QUARTER:
        case DTK_MONTH:
        case DTK_DAY:
        case DTK_HOUR:
        case DTK_MINUTE:
        case DTK_SECOND:
        case DTK_MILLISEC:
        case DTK_MICROSEC:
          pfree(lowunits);
          return ts;
          break;

        default:
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "unit \"%s\" not supported for type %s",
            // lowunits, format_type_be(TIMESTAMPTZOID));
            lowunits, "timestamp with time zone");
          result = 0;
      }
    }

    if (timestamp2tm(ts, &tz, tm, &fsec, NULL, tzp) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      pfree(lowunits);
      return DT_NOEND;
    }

    switch (val)
    {
      case DTK_WEEK:
        {
          int woy = date2isoweek(tm->tm_year, tm->tm_mon, tm->tm_mday);

          /*
           * If it is week 52/53 and the month is January, then the
           * week must belong to the previous year. Also, some
           * December dates belong to the next year.
           */
          if (woy >= 52 && tm->tm_mon == 1)
            --tm->tm_year;
          if (woy <= 1 && tm->tm_mon == MONTHS_PER_YEAR)
            ++tm->tm_year;
          isoweek2date(woy, &(tm->tm_year), &(tm->tm_mon), &(tm->tm_mday));
          tm->tm_hour = 0;
          tm->tm_min = 0;
          tm->tm_sec = 0;
          fsec = 0;
          redotz = true;
          break;
        }
        /* one may consider DTK_THOUSAND and DTK_HUNDRED... */
      case DTK_MILLENNIUM:

        /*
         * truncating to the millennium? what is this supposed to
         * mean? let us put the first year of the millennium... i.e.
         * -1000, 1, 1001, 2001...
         */
        if (tm->tm_year > 0)
          tm->tm_year = ((tm->tm_year + 999) / 1000) * 1000 - 999;
        else
          tm->tm_year = -((999 - (tm->tm_year - 1)) / 1000) * 1000 + 1;
        /* FALL THRU */
      case DTK_CENTURY:
        /* truncating to the century? as above: -100, 1, 101... */
        if (tm->tm_year > 0)
          tm->tm_year = ((tm->tm_year + 99) / 100) * 100 - 99;
        else
          tm->tm_year = -((99 - (tm->tm_year - 1)) / 100) * 100 + 1;
        /* FALL THRU */
      case DTK_DECADE:

        /*
         * truncating to the decade? first year of the decade. must
         * not be applied if year was truncated before!
         */
        if (val != DTK_MILLENNIUM && val != DTK_CENTURY)
        {
          if (tm->tm_year > 0)
            tm->tm_year = (tm->tm_year / 10) * 10;
          else
            tm->tm_year = -((8 - (tm->tm_year - 1)) / 10) * 10;
        }
        /* FALL THRU */
      case DTK_YEAR:
        tm->tm_mon = 1;
        /* FALL THRU */
      case DTK_QUARTER:
        tm->tm_mon = (3 * ((tm->tm_mon - 1) / 3)) + 1;
        /* FALL THRU */
      case DTK_MONTH:
        tm->tm_mday = 1;
        /* FALL THRU */
      case DTK_DAY:
        tm->tm_hour = 0;
        redotz = true;  /* for all cases >= DAY */
        /* FALL THRU */
      case DTK_HOUR:
        tm->tm_min = 0;
        /* FALL THRU */
      case DTK_MINUTE:
        tm->tm_sec = 0;
        /* FALL THRU */
      case DTK_SECOND:
        fsec = 0;
        break;
      case DTK_MILLISEC:
        fsec = (fsec / 1000) * 1000;
        break;
      case DTK_MICROSEC:
        break;

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPTZOID));
          lowunits, "timestamp with time zone");
        result = 0;
    }

    if (redotz)
      tz = DetermineTimeZoneOffset(tm, tzp);

    if (tm2timestamp(tm, fsec, &tz, &result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      result = 0;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(TIMESTAMPTZOID));
      lowunits, "timestamp with time zone");
    result = 0;
  }

  pfree(lowunits);
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Truncate timestamptz to specified units in current timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_trunc()
 */
#if MEOS
TimestampTz
timestamptz_trunc(TimestampTz ts, const text *units)
{
  return timestamptz_trunc_internal(ts, units, session_timezone);
}
#endif
TimestampTz
pg_timestamptz_trunc(TimestampTz ts, const text *units)
{
  return timestamptz_trunc_internal(ts, (text *) units, session_timezone);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Truncate timestamptz to specified units in specified timezone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_trunc_zone()
 */
#if MEOS
TimestampTz
timestamptz_trunc_zone(TimestampTz ts, const text *units,
  const text *zone)
{
  return pg_timestamptz_trunc_zone(ts, (text *) units, zone);
}
#endif
TimestampTz
pg_timestamptz_trunc_zone(TimestampTz ts, const text *units,
  const text *zone)
{
  /*
   * Look up the requested timezone.
   */
  pg_tz *tzp = lookup_timezone((text *) zone);
  TimestampTz result = timestamptz_trunc_internal(ts, (text *) units, tzp);
  pfree(tzp);
  return result;
}

/**
 * @ingroup meos_base_interval
 * @brief Extract the specified field from an interval
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p interval_trunc()
 */
#if MEOS
Interval *
interval_trunc(const Interval *interv, const text *units)
{
  return pg_interval_trunc(interv, units);
}
#endif
Interval *
pg_interval_trunc(const Interval *interv, const text *units)
{
  Interval *result = (Interval *) palloc(sizeof(Interval));
  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);

  int val;
  int type = DecodeUnits(0, lowunits, &val);
  if (type == UNITS)
  {
    if (INTERVAL_NOT_FINITE(interv))
    {
      /*
       * Errors thrown here for invalid units should exactly match those
       * below, else there will be unexpected discrepancies between
       * finite- and infinite-input cases.
       */
      switch (val)
      {
        case DTK_MILLENNIUM:
        case DTK_CENTURY:
        case DTK_DECADE:
        case DTK_YEAR:
        case DTK_QUARTER:
        case DTK_MONTH:
        case DTK_DAY:
        case DTK_HOUR:
        case DTK_MINUTE:
        case DTK_SECOND:
        case DTK_MILLISEC:
        case DTK_MICROSEC:
          memcpy(result, interv, sizeof(Interval));
          pfree(lowunits);
          return result;
          break;

        default:
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "unit \"%s\" not supported for type %s",
            // lowunits, format_type_be(INTERVALOID));
            lowunits, "interval");
          result = 0;
      }
    }

    struct pg_itm tt, *tm = &tt;
    interval2itm(*interv, tm);
    switch (val)
    {
      case DTK_MILLENNIUM:
        /* caution: C division may have negative remainder */
        tm->tm_year = (tm->tm_year / 1000) * 1000;
        /* FALL THRU */
      case DTK_CENTURY:
        /* caution: C division may have negative remainder */
        tm->tm_year = (tm->tm_year / 100) * 100;
        /* FALL THRU */
      case DTK_DECADE:
        /* caution: C division may have negative remainder */
        tm->tm_year = (tm->tm_year / 10) * 10;
        /* FALL THRU */
      case DTK_YEAR:
        tm->tm_mon = 0;
        /* FALL THRU */
      case DTK_QUARTER:
        tm->tm_mon = 3 * (tm->tm_mon / 3);
        /* FALL THRU */
      case DTK_MONTH:
        tm->tm_mday = 0;
        /* FALL THRU */
      case DTK_DAY:
        tm->tm_hour = 0;
        /* FALL THRU */
      case DTK_HOUR:
        tm->tm_min = 0;
        /* FALL THRU */
      case DTK_MINUTE:
        tm->tm_sec = 0;
        /* FALL THRU */
      case DTK_SECOND:
        tm->tm_usec = 0;
        break;
      case DTK_MILLISEC:
        tm->tm_usec = (tm->tm_usec / 1000) * 1000;
        break;
      case DTK_MICROSEC:
        break;

      default:
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(INTERVALOID));
          lowunits, "interval");
        pfree(lowunits);
        return NULL;
      }
    }

    if (itm2interval(tm, result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "interval out of range");
      pfree(lowunits);
      return NULL;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(INTERVALOID));
      lowunits, "interval");
    pfree(lowunits);
    return NULL;
  }

  pfree(lowunits);
  return result;
}

/* isoweek2j()
 *
 *  Return the Julian day which corresponds to the first day (Monday) of the given ISO 8601 year and week.
 *  Julian days are used to convert between ISO week dates and Gregorian dates.
 *
 *  XXX: This function has integer overflow hazards, but restructuring it to
 *  work with the soft-error handling that its callers do is likely more
 *  trouble than it's worth.
 */
int
isoweek2j(int year, int week)
{
  /* fourth day of current year */
  int day4 = date2j(year, 1, 4);

  /* day0 == offset to first day of week (Monday) */
  int day0 = j2day(day4 - 1);
  return ((week - 1) * 7) + (day4 - day0);
}

/* isoweek2date()
 * Convert ISO week of year number to date.
 * The year field must be specified with the ISO year!
 * karel 2000/08/07
 */
void
isoweek2date(int woy, int *year, int *mon, int *mday)
{
  j2date(isoweek2j(*year, woy), year, mon, mday);
}

/* isoweekdate2date()
 *
 *  Convert an ISO 8601 week date (ISO year, ISO week) into a Gregorian date.
 *  Gregorian day of week sent so weekday strings can be supplied.
 *  Populates year, mon, and mday with the correct Gregorian values.
 *  year must be passed in as the ISO year.
 */
void
isoweekdate2date(int isoweek, int wday, int *year, int *mon, int *mday)
{
  int jday = isoweek2j(*year, isoweek);
  /* convert Gregorian week start (Sunday=1) to ISO week start (Monday=1) */
  if (wday > 1)
    jday += wday - 2;
  else
    jday += 6;
  j2date(jday, year, mon, mday);
}

/* date2isoweek()
 *
 *  Returns ISO week number of year.
 */
int
date2isoweek(int year, int mon, int mday)
{
  /* current day */
  int dayn = date2j(year, mon, mday);

  /* fourth day of current year */
  int day4 = date2j(year, 1, 4);

  /* day0 == offset to first day of week (Monday) */
  int day0 = j2day(day4 - 1);

  /*
   * We need the first week containing a Thursday, otherwise this day falls
   * into the previous year for purposes of counting weeks
   */
  if (dayn < day4 - day0)
  {
    day4 = date2j(year - 1, 1, 4);

    /* day0 == offset to first day of week (Monday) */
    day0 = j2day(day4 - 1);
  }

  float8 result = (dayn - (day4 - day0)) / 7 + 1;

  /*
   * Sometimes the last few days in a year will fall into the first week of
   * the next year, so check for this.
   */
  if (result >= 52)
  {
    day4 = date2j(year + 1, 1, 4);

    /* day0 == offset to first day of week (Monday) */
    day0 = j2day(day4 - 1);

    if (dayn >= day4 - day0)
      result = (dayn - (day4 - day0)) / 7 + 1;
  }

  return (int) result;
}


/* date2isoyear()
 *
 *  Returns ISO 8601 year number.
 *  Note: zero or negative results follow the year-zero-exists convention.
 */
int
date2isoyear(int year, int mon, int mday)
{
  /* current day */
  int dayn = date2j(year, mon, mday);

  /* fourth day of current year */
  int day4 = date2j(year, 1, 4);

  /* day0 == offset to first day of week (Monday) */
  int day0 = j2day(day4 - 1);

  /*
   * We need the first week containing a Thursday, otherwise this day falls
   * into the previous year for purposes of counting weeks
   */
  if (dayn < day4 - day0)
  {
    day4 = date2j(year - 1, 1, 4);
    /* day0 == offset to first day of week (Monday) */
    day0 = j2day(day4 - 1);
    year--;
  }

  float8 result = (dayn - (day4 - day0)) / 7 + 1;

  /*
   * Sometimes the last few days in a year will fall into the first week of
   * the next year, so check for this.
   */
  if (result >= 52)
  {
    day4 = date2j(year + 1, 1, 4);
    /* day0 == offset to first day of week (Monday) */
    day0 = j2day(day4 - 1);
    if (dayn >= day4 - day0)
      year++;
  }
  return year;
}


/* date2isoyearday()
 *
 *  Returns the ISO 8601 day-of-year, given a Gregorian year, month and day.
 *  Possible return values are 1 through 371 (364 in non-leap years).
 */
int
date2isoyearday(int year, int mon, int mday)
{
  return date2j(year, mon, mday) - isoweek2j(date2isoyear(year, mon, mday), 1) + 1;
}

/*
 * NonFiniteTimestampTzPart
 *
 *  Used by timestamp_part and timestamptz_part when extracting from infinite
 *  timestamp[tz].  Returns +/-Infinity if that is the appropriate result,
 *  otherwise returns zero (which should be taken as meaning to return NULL).
 *
 *  Errors thrown here for invalid units should exactly match those that
 *  would be thrown in the calling functions, else there will be unexpected
 *  discrepancies between finite- and infinite-input cases.
 */
static float8
NonFiniteTimestampTzPart(int type, int unit, char *lowunits, bool isNegative,
  bool isTz)
{
  if ((type != UNITS) && (type != RESERV))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s", lowunits,
      // format_type_be(isTz ? TIMESTAMPTZOID : TIMESTAMPOID));
      isTz ? "timestamp with time zone" : "timestamp without time zone");
    return get_float8_infinity();
  }

  switch (unit)
  {
      /* Oscillating units */
    case DTK_MICROSEC:
    case DTK_MILLISEC:
    case DTK_SECOND:
    case DTK_MINUTE:
    case DTK_HOUR:
    case DTK_DAY:
    case DTK_MONTH:
    case DTK_QUARTER:
    case DTK_WEEK:
    case DTK_DOW:
    case DTK_ISODOW:
    case DTK_DOY:
    case DTK_TZ:
    case DTK_TZ_MINUTE:
    case DTK_TZ_HOUR:
      return 0.0;

      /* Monotonically-increasing units */
    case DTK_YEAR:
    case DTK_DECADE:
    case DTK_CENTURY:
    case DTK_MILLENNIUM:
    case DTK_JULIAN:
    case DTK_ISOYEAR:
    case DTK_EPOCH:
      if (isNegative)
        return -get_float8_infinity();
      else
        return get_float8_infinity();

    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unit \"%s\" not supported for type %s", lowunits,
        // format_type_be(isTz ? TIMESTAMPTZOID : TIMESTAMPOID));
        isTz ? "timestamp with time zone" : "timestamp without time zone");
      return get_float8_infinity();
  }
}

/* timestamp_part() and extract_timestamp()
 * Extract specified field from timestamp.
 */
static Datum
timestamp_part_common(text *units, Timestamp timestamp, bool retnumeric)
{
  int64 intresult;

  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);

  int val;
  int type = DecodeUnits(0, lowunits, &val);
  if (type == UNKNOWN_FIELD)
    type = DecodeSpecial(0, lowunits, &val);

  if (TIMESTAMP_NOT_FINITE(timestamp))
  {
    double r = NonFiniteTimestampTzPart(type, val, lowunits,
       TIMESTAMP_IS_NOBEGIN(timestamp), false);

    if (r != 0.0)
    {
      pfree(lowunits);
      if (retnumeric)
      {
        if (r < 0)
          return NumericGetDatum(pg_numeric_in("-Infinity", -1));
        else if (r > 0)
          return NumericGetDatum(pg_numeric_in("Infinity", -1));
      }
      else
        return Float8GetDatum(r);
    }
    else
      return (Datum) 0;
  }

  if (type == UNITS)
  {
    struct pg_tm tt, *tm = &tt;
    fsec_t fsec;
    if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      pfree(lowunits);
      return (Datum) 0;
    }

    switch (val)
    {
      case DTK_MICROSEC:
        intresult = tm->tm_sec * INT64CONST(1000000) + fsec;
        break;

      case DTK_MILLISEC:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec * 1000 + fsec / 1000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(tm->tm_sec * INT64CONST(1000000) + fsec, 3));
        else
          return Float8GetDatum(tm->tm_sec * 1000.0 + fsec / 1000.0);

      case DTK_SECOND:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec + fsec / 1'000'000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1'000'000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(tm->tm_sec * INT64CONST(1000000) + fsec, 6));
        else
          return Float8GetDatum(tm->tm_sec + fsec / 1000000.0);

      case DTK_MINUTE:
        intresult = tm->tm_min;
        break;

      case DTK_HOUR:
        intresult = tm->tm_hour;
        break;

      case DTK_DAY:
        intresult = tm->tm_mday;
        break;

      case DTK_MONTH:
        intresult = tm->tm_mon;
        break;

      case DTK_QUARTER:
        intresult = (tm->tm_mon - 1) / 3 + 1;
        break;

      case DTK_WEEK:
        intresult = date2isoweek(tm->tm_year, tm->tm_mon, tm->tm_mday);
        break;

      case DTK_YEAR:
        if (tm->tm_year > 0)
          intresult = tm->tm_year;
        else
          /* there is no year 0, just 1 BC and 1 AD */
          intresult = tm->tm_year - 1;
        break;

      case DTK_DECADE:

        /*
         * what is a decade wrt dates? let us assume that decade 199
         * is 1990 thru 1999... decade 0 starts on year 1 BC, and -1
         * is 11 BC thru 2 BC...
         */
        if (tm->tm_year >= 0)
          intresult = tm->tm_year / 10;
        else
          intresult = -((8 - (tm->tm_year - 1)) / 10);
        break;

      case DTK_CENTURY:

        /* ----
         * centuries AD, c>0: year in [ (c-1)* 100 + 1 : c*100 ]
         * centuries BC, c<0: year in [ c*100 : (c+1) * 100 - 1]
         * there is no number 0 century.
         * ----
         */
        if (tm->tm_year > 0)
          intresult = (tm->tm_year + 99) / 100;
        else
          /* caution: C division may have negative remainder */
          intresult = -((99 - (tm->tm_year - 1)) / 100);
        break;

      case DTK_MILLENNIUM:
        /* see comments above. */
        if (tm->tm_year > 0)
          intresult = (tm->tm_year + 999) / 1000;
        else
          intresult = -((999 - (tm->tm_year - 1)) / 1000);
        break;

      case DTK_JULIAN:
        pfree(lowunits);
        if (retnumeric)
          return NumericGetDatum(numeric_add_opt_error(
            int64_to_numeric(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday)),
            numeric_div_opt_error(int64_to_numeric(
              ((((tm->tm_hour * MINS_PER_HOUR) + tm->tm_min) * 
              SECS_PER_MINUTE) + tm->tm_sec) * INT64CONST(1000000) + fsec),
              int64_to_numeric(SECS_PER_DAY * INT64CONST(1000000)), NULL),
            NULL));
        else
          return Float8GetDatum(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) +
            ((((tm->tm_hour * MINS_PER_HOUR) + tm->tm_min) * SECS_PER_MINUTE) +
            tm->tm_sec + (fsec / 1000000.0)) / (double) SECS_PER_DAY);

      case DTK_ISOYEAR:
        intresult = date2isoyear(tm->tm_year, tm->tm_mon, tm->tm_mday);
        /* Adjust BC years */
        if (intresult <= 0)
          intresult -= 1;
        break;

      case DTK_DOW:
      case DTK_ISODOW:
        intresult = j2day(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday));
        if (val == DTK_ISODOW && intresult == 0)
          intresult = 7;
        break;

      case DTK_DOY:
        intresult = (date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) -
          date2j(tm->tm_year, 1, 1) + 1);
        break;

      case DTK_TZ:
      case DTK_TZ_MINUTE:
      case DTK_TZ_HOUR:
      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPOID));
          lowunits, "timestamp without time zone");
        intresult = 0;
    }
  }
  else if (type == RESERV)
  {
    Timestamp epoch;
    switch (val)
    {
      case DTK_EPOCH:
        epoch = SetEpochTimestamp();
        /* (timestamp - epoch) / 1000000 */
        pfree(lowunits);
        if (retnumeric)
        {
          Numeric result;
          if (timestamp < (PG_INT64_MAX + epoch))
            result = int64_div_fast_to_numeric(timestamp - epoch, 6);
          else
          {
            result = numeric_div_opt_error(numeric_sub_opt_error(
              int64_to_numeric(timestamp), int64_to_numeric(epoch), NULL), 
              int64_to_numeric(1000000), NULL);
            result = pg_numeric_round(result, 6);
          }
          return NumericGetDatum(result);
        }
        else
        {
          float8 result;
          /* try to avoid precision loss in subtraction */
          if (timestamp < (PG_INT64_MAX + epoch))
            result = (timestamp - epoch) / 1000000.0;
          else
            result = ((float8) timestamp - epoch) / 1000000.0;
          return Float8GetDatum(result);
        }

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPOID));
          lowunits, "timestamp without time zone");
        intresult = 0;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(TIMESTAMPOID));
      lowunits, "timestamp without time zone");
    intresult = 0;
  }

  pfree(lowunits);
  if (retnumeric)
    return NumericGetDatum(int64_to_numeric(intresult));
  else
    return Float8GetDatum(intresult);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Extract from a field from a timestamp
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_part()
 */
#if MEOS
float8
timestamp_part(Timestamp ts, const text *units)
{
  return DatumGetFloat8(timestamp_part_common((text *) units, ts, false));
}
#endif
float8
pg_timestamp_part(Timestamp ts, const text *units)
{
  return DatumGetFloat8(timestamp_part_common((text *) units, ts, false));
}

/**
 * @ingroup meos_base_timestamp
 * @brief Extract from a field from a timestamp
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p extract_timestamp()
 */
Numeric
timestamp_extract(Timestamp ts, const text *units)
{
  return DatumGetNumeric(timestamp_part_common((text *) units, ts, true));
}

/* timestamptz_part() and extract_timestamptz()
 * Extract specified field from timestamp with time zone.
 */
static Datum
timestamptz_part_common(TimestampTz timestamp, text *units, bool retnumeric)
{
  int64 intresult;
  Timestamp epoch;
  int tz;
  int type, val;
  fsec_t fsec;
  struct pg_tm tt, *tm = &tt;

  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);
  type = DecodeUnits(0, lowunits, &val);
  if (type == UNKNOWN_FIELD)
    type = DecodeSpecial(0, lowunits, &val);

  if (TIMESTAMP_NOT_FINITE(timestamp))
  {
    double r = NonFiniteTimestampTzPart(type, val, lowunits,
      TIMESTAMP_IS_NOBEGIN(timestamp), true);
    if (r != 0.0)
    {
      pfree(lowunits);
      if (retnumeric)
      {
        if (r < 0)
          return NumericGetDatum(pg_numeric_in("-Infinity", -1));
        else if (r > 0)
          return NumericGetDatum(pg_numeric_in("Infinity", -1));
      }
      else
        return Float8GetDatum(r);
    }
    else
      return (Datum) 0;
  }

  if (type == UNITS)
  {
    if (timestamp2tm(timestamp, &tz, tm, &fsec, NULL, NULL) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      pfree(lowunits);
      return (Datum) 0;
    }

    switch (val)
    {
      case DTK_TZ:
        intresult = -tz;
        break;

      case DTK_TZ_MINUTE:
        intresult = (-tz / SECS_PER_MINUTE) % MINS_PER_HOUR;
        break;

      case DTK_TZ_HOUR:
        intresult = -tz / SECS_PER_HOUR;
        break;

      case DTK_MICROSEC:
        intresult = tm->tm_sec * INT64CONST(1000000) + fsec;
        break;

      case DTK_MILLISEC:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec * 1000 + fsec / 1000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(tm->tm_sec *
            INT64CONST(1000000) + fsec, 3));
        else
          return Float8GetDatum(tm->tm_sec * 1000.0 + fsec / 1000.0);

      case DTK_SECOND:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec + fsec / 1'000'000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1'000'000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(tm->tm_sec *
            INT64CONST(1000000) + fsec, 6));
        else
          return Float8GetDatum(tm->tm_sec + fsec / 1000000.0);

      case DTK_MINUTE:
        intresult = tm->tm_min;
        break;

      case DTK_HOUR:
        intresult = tm->tm_hour;
        break;

      case DTK_DAY:
        intresult = tm->tm_mday;
        break;

      case DTK_MONTH:
        intresult = tm->tm_mon;
        break;

      case DTK_QUARTER:
        intresult = (tm->tm_mon - 1) / 3 + 1;
        break;

      case DTK_WEEK:
        intresult = date2isoweek(tm->tm_year, tm->tm_mon, tm->tm_mday);
        break;

      case DTK_YEAR:
        if (tm->tm_year > 0)
          intresult = tm->tm_year;
        else
          /* there is no year 0, just 1 BC and 1 AD */
          intresult = tm->tm_year - 1;
        break;

      case DTK_DECADE:
        /* see comments in timestamp_part */
        if (tm->tm_year > 0)
          intresult = tm->tm_year / 10;
        else
          intresult = -((8 - (tm->tm_year - 1)) / 10);
        break;

      case DTK_CENTURY:
        /* see comments in timestamp_part */
        if (tm->tm_year > 0)
          intresult = (tm->tm_year + 99) / 100;
        else
          intresult = -((99 - (tm->tm_year - 1)) / 100);
        break;

      case DTK_MILLENNIUM:
        /* see comments in timestamp_part */
        if (tm->tm_year > 0)
          intresult = (tm->tm_year + 999) / 1000;
        else
          intresult = -((999 - (tm->tm_year - 1)) / 1000);
        break;

      case DTK_JULIAN:
        pfree(lowunits);
        if (retnumeric)
          return NumericGetDatum(numeric_add_opt_error(
            int64_to_numeric(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday)),
            numeric_div_opt_error(
              int64_to_numeric(((((tm->tm_hour * MINS_PER_HOUR) + tm->tm_min) * 
                SECS_PER_MINUTE) + tm->tm_sec) * INT64CONST(1000000) + fsec),
              int64_to_numeric(SECS_PER_DAY * INT64CONST(1000000)), 
              NULL), NULL));
        else
          return Float8GetDatum(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) +
            ((((tm->tm_hour * MINS_PER_HOUR) + tm->tm_min) * SECS_PER_MINUTE) +
            tm->tm_sec + (fsec / 1000000.0)) / (double) SECS_PER_DAY);

      case DTK_ISOYEAR:
        intresult = date2isoyear(tm->tm_year, tm->tm_mon, tm->tm_mday);
        /* Adjust BC years */
        if (intresult <= 0)
          intresult -= 1;
        break;

      case DTK_DOW:
      case DTK_ISODOW:
        intresult = j2day(date2j(tm->tm_year, tm->tm_mon, tm->tm_mday));
        if (val == DTK_ISODOW && intresult == 0)
          intresult = 7;
        break;

      case DTK_DOY:
        intresult = (date2j(tm->tm_year, tm->tm_mon, tm->tm_mday)
               - date2j(tm->tm_year, 1, 1) + 1);
        break;

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPTZOID));
          lowunits, "timestamp with time zone");
        intresult = 0;
    }
  }
  else if (type == RESERV)
  {
    switch (val)
    {
      case DTK_EPOCH:
        pfree(lowunits);
        epoch = SetEpochTimestamp();
        /* (timestamp - epoch) / 1000000 */
        if (retnumeric)
        {
          Numeric result;
          if (timestamp < (PG_INT64_MAX + epoch))
            result = int64_div_fast_to_numeric(timestamp - epoch, 6);
          else
          {
            result = numeric_div_opt_error(numeric_sub_opt_error(
              int64_to_numeric(timestamp), int64_to_numeric(epoch), NULL),
              int64_to_numeric(1000000), NULL);
            result = pg_numeric_round(result, 6);
          }
          return NumericGetDatum(result);
        }
        else
        {
          float8 result;
          /* try to avoid precision loss in subtraction */
          if (timestamp < (PG_INT64_MAX + epoch))
            result = (timestamp - epoch) / 1000000.0;
          else
            result = ((float8) timestamp - epoch) / 1000000.0;
          return Float8GetDatum(result);
        }

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(TIMESTAMPTZOID));
          lowunits, "timestamp with time zone");
        intresult = 0;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(TIMESTAMPTZOID));
      lowunits, "timestamp with time zone");
    intresult = 0;
  }

  pfree(lowunits);
  if (retnumeric)
    return NumericGetDatum(int64_to_numeric(intresult));
  else
    return Float8GetDatum(intresult);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Extract a field from a timestamptz
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_part()
 */
#if MEOS
float8
timestamptz_part(TimestampTz ts, const text *units)
{
  return DatumGetFloat8(timestamptz_part_common(ts, (text *) units, false));
}
#endif
float8
pg_timestamptz_part(TimestampTz ts, const text *units)
{
  return DatumGetFloat8(timestamptz_part_common(ts, (text *) units, false));
}

/**
 * @ingroup meos_base_timestamp
 * @brief Extract a field from a timestamptz
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p extract_timestamptz()
 */
Numeric
timestamptz_extract(TimestampTz ts, const text *units)
{
  return DatumGetNumeric(timestamptz_part_common(ts, (text *) units, true));
}

/*
 * NonFiniteIntervalPart
 *
 *  Used by interval_part when extracting from infinite interval.  Returns
 *  +/-Infinity if that is the appropriate result, otherwise returns zero
 *  (which should be taken as meaning to return NULL).
 *
 *  Errors thrown here for invalid units should exactly match those that
 *  would be thrown in the calling functions, else there will be unexpected
 *  discrepancies between finite- and infinite-input cases.
 */
static float8
NonFiniteIntervalPart(int type, int unit, char *lowunits, bool isNegative)
{
  if ((type != UNITS) && (type != RESERV))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(INTERVALOID));
      lowunits, "interval");
    return get_float8_infinity();
  }

  switch (unit)
  {
      /* Oscillating units */
    case DTK_MICROSEC:
    case DTK_MILLISEC:
    case DTK_SECOND:
    case DTK_MINUTE:
    case DTK_WEEK:
    case DTK_MONTH:
    case DTK_QUARTER:
      return 0.0;

      /* Monotonically-increasing units */
    case DTK_HOUR:
    case DTK_DAY:
    case DTK_YEAR:
    case DTK_DECADE:
    case DTK_CENTURY:
    case DTK_MILLENNIUM:
    case DTK_EPOCH:
      if (isNegative)
        return -get_float8_infinity();
      else
        return get_float8_infinity();

    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unit \"%s\" not supported for type %s",
        // lowunits, format_type_be(INTERVALOID));
        lowunits, "interval");
      return get_float8_infinity();
  }
}

/* interval_part() and extract_interval()
 * Extract specified field from interval.
 */
static Datum
interval_part_common(const Interval *interval, const text *units,
  bool retnumeric)
{
  char *lowunits = downcase_truncate_identifier(VARDATA_ANY(units),
    VARSIZE_ANY_EXHDR(units), false);

  int val;
  int type = DecodeUnits(0, lowunits, &val);
  if (type == UNKNOWN_FIELD)
    type = DecodeSpecial(0, lowunits, &val);

  if (INTERVAL_NOT_FINITE(interval))
  {
    double r = NonFiniteIntervalPart(type, val, lowunits,
      INTERVAL_IS_NOBEGIN(interval));
    pfree(lowunits);
    if (r != 0.0)
    {
      if (retnumeric)
      {
        if (r < 0)
          return NumericGetDatum(pg_numeric_in("-Infinity", -1));
        else if (r > 0)
          return NumericGetDatum(pg_numeric_in("Infinity", -1));
      }
      else
        return Float8GetDatum(r);
    }
    else
      return (Datum) 0;
  }

  int64 intresult;
  if (type == UNITS)
  {
    struct pg_itm tt, *tm = &tt;
    interval2itm(*interval, tm);
    switch (val)
    {
      case DTK_MICROSEC:
        intresult = tm->tm_sec * INT64CONST(1000000) + tm->tm_usec;
        break;

      case DTK_MILLISEC:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec * 1000 + fsec / 1000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(
            tm->tm_sec * INT64CONST(1000000) + tm->tm_usec, 3));
        else
          return Float8GetDatum(tm->tm_sec * 1000.0 + tm->tm_usec / 1000.0);
        break;

      case DTK_SECOND:
        pfree(lowunits);
        if (retnumeric)
          /*---
           * tm->tm_sec + fsec / 1'000'000
           * = (tm->tm_sec * 1'000'000 + fsec) / 1'000'000
           */
          return NumericGetDatum(int64_div_fast_to_numeric(
            tm->tm_sec * INT64CONST(1000000) + tm->tm_usec, 6));
        else
          return Float8GetDatum(tm->tm_sec + tm->tm_usec / 1000000.0);
        break;

      case DTK_MINUTE:
        intresult = tm->tm_min;
        break;

      case DTK_HOUR:
        intresult = tm->tm_hour;
        break;

      case DTK_DAY:
        intresult = tm->tm_mday;
        break;

      case DTK_WEEK:
        intresult = tm->tm_mday / 7;
        break;

      case DTK_MONTH:
        intresult = tm->tm_mon;
        break;

      case DTK_QUARTER:

        /*
         * We want to maintain the rule that a field extracted from a
         * negative interval is the negative of the field's value for
         * the sign-reversed interval.  The broken-down tm_year and
         * tm_mon aren't very helpful for that, so work from
         * interval->month.
         */
        if (interval->month >= 0)
          intresult = (tm->tm_mon / 3) + 1;
        else
          intresult = -(((-interval->month % MONTHS_PER_YEAR) / 3) + 1);
        break;

      case DTK_YEAR:
        intresult = tm->tm_year;
        break;

      case DTK_DECADE:
        /* caution: C division may have negative remainder */
        intresult = tm->tm_year / 10;
        break;

      case DTK_CENTURY:
        /* caution: C division may have negative remainder */
        intresult = tm->tm_year / 100;
        break;

      case DTK_MILLENNIUM:
        /* caution: C division may have negative remainder */
        intresult = tm->tm_year / 1000;
        break;

      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unit \"%s\" not supported for type %s",
          // lowunits, format_type_be(INTERVALOID));
          lowunits, "interval");
        intresult = 0;
    }
  }
  else if (type == RESERV && val == DTK_EPOCH)
  {
    pfree(lowunits);
    if (retnumeric)
    {
      Numeric result;
      int64 secs_from_day_month;
      int64 val;

      /*
       * To do this calculation in integer arithmetic even though
       * DAYS_PER_YEAR is fractional, multiply everything by 4 and then
       * divide by 4 again at the end.  This relies on DAYS_PER_YEAR
       * being a multiple of 0.25 and on SECS_PER_DAY being a multiple
       * of 4.
       */
      secs_from_day_month = ((int64) (4 * DAYS_PER_YEAR) * (interval->month / MONTHS_PER_YEAR) +
        (int64) (4 * DAYS_PER_MONTH) * (interval->month % MONTHS_PER_YEAR) +
        (int64) 4 * interval->day) * (SECS_PER_DAY / 4);

      /*---
       * result = secs_from_day_month + interval->time / 1'000'000
       * = (secs_from_day_month * 1'000'000 + interval->time) / 1'000'000
       */

      /*
       * Try the computation inside int64; if it overflows, do it in
       * numeric (slower).  This overflow happens around 10^9 days, so
       * not common in practice.
       */
      if (!pg_mul_s64_overflow(secs_from_day_month, 1000000, &val) &&
        !pg_add_s64_overflow(val, interval->time, &val))
        result = int64_div_fast_to_numeric(val, 6);
      else
        result =
          numeric_add_opt_error(int64_div_fast_to_numeric(interval->time, 6),
            int64_to_numeric(secs_from_day_month), NULL);

      return NumericGetDatum(result);
    }
    else
    {
      float8 result = interval->time / 1000000.0;
      result += ((double) DAYS_PER_YEAR * SECS_PER_DAY) * (interval->month /
        MONTHS_PER_YEAR);
      result += ((double) DAYS_PER_MONTH * SECS_PER_DAY) * (interval->month %
        MONTHS_PER_YEAR);
      result += ((double) SECS_PER_DAY) * interval->day;
      return Float8GetDatum(result);
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unit \"%s\" not recognized for type %s",
      // lowunits, format_type_be(INTERVALOID));
      lowunits, "interval");
    intresult = 0;
  }

  pfree(lowunits);
  if (retnumeric)
    return NumericGetDatum(int64_to_numeric(intresult));
  else
    return Float8GetDatum(intresult);
}

/**
 * @ingroup meos_base_interval
 * @brief Extract a field from an interval
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p interval_part()
 */
#if MEOS
float8
interval_part(const Interval *interv, const text *units)
{
  return interval_part_common(interv, units, false);
}
#endif
float8
pg_interval_part(const Interval *interv, const text *units)
{
  return DatumGetFloat8(interval_part_common((Interval *) interv,
    (text *) units, false));
}

/**
 * @ingroup meos_base_interval
 * @brief Extract a field from an interval
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p extract_interval()
 */
Numeric
interval_extract(const Interval *interv, const text *units)
{
  return DatumGetNumeric(interval_part_common((Interval *) interv,
    (text *) units, true));
}

/**
 * @ingroup meos_base_timestamp
 * @brief Encode timestamp type with specified time zone
 * @details This function is just timestamp2timestamptz() except instead of
 * shifting to the global timezone, we shift to the specified timezone.
 * This is different from the other AT TIME ZONE cases because instead
 * of shifting _to_ a new time zone, it sets the time to _be_ the
 * specified timezone.
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_zone()
 */
#if MEOS
TimestampTz
timestamp_zone(Timestamp ts, const text *zone)
{
  return pg_timestamp_zone(ts, zone);
}
#endif
TimestampTz
pg_timestamp_zone(Timestamp ts, const text *zone)
{
  TimestampTz result;
  char tzname[TZ_STRLEN_MAX + 1];
  struct pg_tm tm;
  fsec_t fsec;

  if (TIMESTAMP_NOT_FINITE(ts))
    return ts;

  /*
   * Look up the requested timezone.
   */
  text_to_cstring_buffer(zone, tzname, sizeof(tzname));
  int val, tz;
  pg_tz *tzp;
  int type = DecodeTimezoneName(tzname, &val, &tzp);
  if (type == TZNAME_FIXED_OFFSET)
  {
    /* fixed-offset abbreviation */
    tz = val;
    result = dt2local(ts, tz);
  }
  else if (type == TZNAME_DYNTZ)
  {
    /* dynamic-offset abbreviation, resolve using specified time */
    if (timestamp2tm(ts, NULL, &tm, &fsec, NULL, tzp) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    tz = -DetermineTimeZoneAbbrevOffset(&tm, tzname, tzp);
    result = dt2local(ts, tz);
  }
  else
  {
    /* full zone name, rotate to that zone */
    if (timestamp2tm(ts, NULL, &tm, &fsec, NULL, tzp) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    tz = DetermineTimeZoneOffset(&tm, tzp);
    if (tm2timestamp(&tm, fsec, &tz, &result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
  }
  pfree(tzp);

  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return DT_NOEND;
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Encode timestamp type with specified time interval as time zone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamp_izone()
 */
#if MEOS
TimestampTz
timestamp_izone(Timestamp ts, const Interval *zone)
{
  return pg_timestamp_izone(ts, zone);
}
#endif
TimestampTz
pg_timestamp_izone(Timestamp ts, const Interval *zone)
{
  if (TIMESTAMP_NOT_FINITE(ts))
    return ts;
  if (INTERVAL_NOT_FINITE(zone))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval time zone \"%s\" must be finite",
      pg_interval_out(zone));
    return DT_NOEND;
  }
  if (zone->month != 0 || zone->day != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval time zone \"%s\" must not include months or days",
      pg_interval_out(zone));
    return DT_NOEND;
  }

  int tz = zone->time / USECS_PER_SEC;
  TimestampTz result = dt2local(ts, tz);
  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return DT_NOEND;
  }
  return result;
}

/* TimestampTimestampTzRequiresRewrite()
 *
 * Returns false if the TimeZone GUC setting causes timestamp_timestamptz and
 * timestamptz_timestamp to be no-ops, where the return value has the same
 * bits as the argument.  Since project convention is to assume a GUC changes
 * no more often than STABLE functions change, the answer is valid that long.
 */
bool
TimestampTimestampTzRequiresRewrite(void)
{
  long offset;
  if (pg_get_timezone_offset(session_timezone, &offset) && offset == 0)
    return false;
  return true;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Convert local timestamp to timestamp at GMT
 * @note Derived from PostgreSQL function @p timestamp_timestamptz()
 */
TimestampTz
timestamp_to_timestamptz(Timestamp ts)
{
  return timestamp2timestamptz(ts);
}

/*
 * Convert timestamp to timestamp with time zone.
 *
 * On successful conversion, *overflow is set to zero if it's not NULL.
 *
 * If the timestamp is finite but out of the valid range for timestamptz, then:
 * if overflow is NULL, we throw an out-of-range error.
 * if overflow is not NULL, we store +1 or -1 there to indicate the sign
 * of the overflow, and return the appropriate timestamptz infinity.
 */
TimestampTz
timestamp2timestamptz_opt_overflow(Timestamp ts, int *overflow)
{
  if (overflow)
    *overflow = 0;

  if (TIMESTAMP_NOT_FINITE(ts))
    return ts;

  int tz;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  /* We don't expect this to fail, but check it pro forma */
  if (timestamp2tm(ts, NULL, tm, &fsec, NULL, NULL) == 0)
  {
    tz = DetermineTimeZoneOffset(tm, session_timezone);
    TimestampTz result = dt2local(ts, -tz);

    if (IS_VALID_TIMESTAMP(result))
    {
      return result;
    }
    else if (overflow)
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
      return result;
    }
  }
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "timestamp out of range");
  return DT_NOEND;
}

/*
 * Promote timestamp to timestamptz, throwing error for overflow.
 */
static TimestampTz
timestamp2timestamptz(Timestamp ts)
{
  return timestamp2timestamptz_opt_overflow(ts, NULL);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Convert timestamp at GMT to local timestamp
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_timestamp()
 */
Timestamp
timestamptz_to_timestamp(TimestampTz tstz)
{
  return timestamptz2timestamp(tstz);
}

static Timestamp
timestamptz2timestamp(TimestampTz tstz)
{
  Timestamp result;
  if (TIMESTAMP_NOT_FINITE(tstz))
    result = tstz;
  else
  {
    struct pg_tm tt, *tm = &tt;
    fsec_t fsec;
    int tz;
    if (timestamp2tm(tstz, &tz, tm, &fsec, NULL, NULL) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    if (tm2timestamp(tm, fsec, NULL, &result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Evaluate timestamp with time zone type at the specified time zone
 * @return On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_zone()
 */
#if MEOS
Timestamp
timestamptz_zone(TimestampTz tstz, const text *zone)
{
  return pg_timestamptz_zone(tstz, zone);
}
#endif
Timestamp
pg_timestamptz_zone(TimestampTz tstz, const text *zone)
{
  if (TIMESTAMP_NOT_FINITE(tstz))
    return tstz;

  /* Look up the requested timezone */
  char tzname[TZ_STRLEN_MAX + 1];
  text_to_cstring_buffer(zone, tzname, sizeof(tzname));

  Timestamp result;
  int val;
  pg_tz *tzp;
  int tz;
  int type = DecodeTimezoneName(tzname, &val, &tzp);
  if (type == TZNAME_FIXED_OFFSET)
  {
    /* fixed-offset abbreviation */
    tz = -val;
    result = dt2local(tstz, tz);
  }
  else if (type == TZNAME_DYNTZ)
  {
    /* dynamic-offset abbreviation, resolve using specified time */
    int isdst;
    tz = DetermineTimeZoneAbbrevOffsetTS(tstz, tzname, tzp, &isdst);
    result = dt2local(tstz, tz);
  }
  else
  {
    /* full zone name, rotate from that zone */
    struct pg_tm tm;
    fsec_t fsec;
    if (timestamp2tm(tstz, &tz, &tm, &fsec, NULL, tzp) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
    if (tm2timestamp(&tm, fsec, NULL, &result) != 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "timestamp out of range");
      return DT_NOEND;
    }
  }
  pfree(tzp);
  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
    return DT_NOEND;
  }
  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Encode timestamp with time zone type with specified time interval as time zone
 * @return Returns a timestamp without time zone. On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_izone()
 */
#if MEOS
Timestamp
timestamptz_izone(TimestampTz tstz, const Interval *zone)
{
  return pg_timestamptz_izone(tstz, zone);
}
#endif
Timestamp
pg_timestamptz_izone(TimestampTz tstz, const Interval *zone)
{
  if (TIMESTAMP_NOT_FINITE(tstz))
    return tstz;

  if (INTERVAL_NOT_FINITE(zone))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval time zone \"%s\" must be finite",
      pg_interval_out(zone));
    return DT_NOEND;
  }

  if (zone->month != 0 || zone->day != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "interval time zone \"%s\" must not include months or days",
      pg_interval_out(zone));
    return DT_NOEND;
  }

  int tz = -(zone->time / USECS_PER_SEC);
  Timestamp result = dt2local(tstz, tz);
  if (!IS_VALID_TIMESTAMP(result))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "timestamp out of range");
     return DT_NOEND;
 }

  return result;
}

/**
 * @ingroup meos_base_timestamp
 * @brief Encode timestamp with time zone type with specified time interval as
 * time zone
 * @return On error return INT_MAX
 * @note Derived from PostgreSQL function @p timestamp_at_local()
 */
#if MEOS
TimestampTz
timestamp_at_local(Timestamp ts)
{
  return timestamp_to_timestamptz(ts);
}
#endif
TimestampTz
pg_timestamp_at_local(Timestamp ts)
{
  return timestamp_to_timestamptz(ts);
}

/**
 * @ingroup meos_base_timestamp
 * @brief Encode timestamp with time zone type with specified time interval as time zone
 * @return Returns a timestamp without time zone. On error return DT_NOEND
 * @note Derived from PostgreSQL function @p timestamptz_at_local()
 */
TimestampTz
timestamptz_at_local(TimestampTz tstz)
{
  return timestamptz_to_timestamp(tstz);
}

/*****************************************************************************/