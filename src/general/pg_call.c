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
 * @file pg_call.c
 * @brief MobilityDB functions pg_func(...) corresponding to external
 * PostgreSQL functions func(PG_FUNCTION_ARGS). This avoids bypassing the
 * function manager fmgr.c.
 * @note These functions are only available for PG version >= 14.
 */

#if POSTGRESQL_VERSION_NUMBER >= 140000

#include "general/pg_call.h"

/* C */
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <miscadmin.h>
#include <common/int128.h>
#include <common/hashfn.h>
#include <libpq/pqformat.h>
#include <utils/datetime.h>
#include <utils/float.h>
#include <utils/int8.h>

/* Definitions from builtins.h to avoid including it */

/* Sign + the most decimal digits an 8-byte number could have */
#define MAXINT8LEN 20
extern int32 pg_strtoint32(const char *s);
extern bool parse_bool_with_len(const char *value, size_t len, bool *result);
extern int pg_ltoa(int32 l, char *a);
extern int pg_lltoa(int64 ll, char *a);
extern int pg_ulltoa_n(uint64 l, char *a);
extern text *cstring_to_text_with_len(const char *s, int len);

/*****************************************************************************
 * Functions adapted from bool.c
 *****************************************************************************/

/**
 * @brief Convert "t" or "f" to 1 or 0
 *
 * Check explicitly for "true/false" and TRUE/FALSE, 1/0, YES/NO, ON/OFF.
 * Reject other values.
 *
 * In the switch statement, check the most-used possibilities first.
 * @note PostgreSQL function: Datum boolin(PG_FUNCTION_ARGS)
 */
bool
pg_boolin(const char *in_str)
{
  const char *str;
  size_t len;
  bool result;

  /*
   * Skip leading and trailing whitespace
   */
  str = in_str;
  while (isspace((unsigned char) *str))
    str++;

  len = strlen(str);
  while (len > 0 && isspace((unsigned char) str[len - 1]))
    len--;

  if (parse_bool_with_len(str, len, &result))
    return result;

  ereport(ERROR,(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
    errmsg("invalid input syntax for type %s: \"%s\"", "boolean", in_str)));

  /* not reached */
  return false;
}

/**
 * @brief Convert 1 or 0 to "t" or "f"
 * @note PostgreSQL function: Datum boolout(PG_FUNCTION_ARGS)
 */
char *
pg_boolout(bool b)
{
  char *result = (char *) palloc(2);
  result[0] = (b) ? 't' : 'f';
  result[1] = '\0';
  return result;
}

/**
 * @brief Convert external binary format to bool
 *
 * The external representation is one byte.  Any nonzero value is taken
 * as "true".
 * @note PostgreSQL function: Datum boolrecv(PG_FUNCTION_ARGS)
 */
bool
pg_boolrecv(StringInfo buf)
{
  int ext;
  ext = pq_getmsgbyte(buf);
  return ((ext != 0) ? true : false);
}

/**
 * @brief Convert bool to binary format
 * @note PostgreSQL function: Datum boolsend(PG_FUNCTION_ARGS)
 */
bytea *
pg_boolsend(bool arg1)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbyte(&buf, arg1 ? 1 : 0);
  return pq_endtypsend(&buf);
}

/*****************************************************************************
 * Functions adapted from int.c
 *****************************************************************************/

/**
 * @brief Return an int4 from a string
 * @note PostgreSQL function: Datum int4in(PG_FUNCTION_ARGS)
 */
int32
pg_int4in(char *str)
{
  return pg_strtoint32(str);
}

/**
 * @brief Return a string from an int4
 * @note PostgreSQL function: Datum int4out(PG_FUNCTION_ARGS)
 */
char *
pg_int4out(int32 val)
{
  char *result = (char *) palloc(12);  /* sign, 10 digits, '\0' */
  pg_ltoa(val, result);
  return result;
}

/**
 * @brief CConvert an int4 to binary format
 * @note PostgreSQL function: Datum int4send(PG_FUNCTION_ARGS)
 */
bytea *
pg_int4send(int32 arg1)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint32(&buf, arg1);
  return pq_endtypsend(&buf);
}

/**
 * @brief Convert external binary format to int4
 * @note PostgreSQL function: Datum int4recv(PG_FUNCTION_ARGS)
 */
int32
pg_int4recv(StringInfo buf)
{
 return (int32) pq_getmsgint(buf, sizeof(int32));
}

/*****************************************************************************
 * Functions adapted from int8.c
 *****************************************************************************/

/**
 * @brief Return an int8 from a string
 * @note PostgreSQL function: Datum int8in(PG_FUNCTION_ARGS)
 */
int64
pg_int8in(char *str)
{
  int64 result;
  (void) scanint8(str, false, &result);
  return result;
}

/**
 * @brief convert a signed 64-bit integer to its string representation
 *
 * Caller must ensure that 'a' points to enough memory to hold the result
 * (at least MAXINT8LEN + 1 bytes, counting a leading sign and trailing NUL).
 * @note We need to copy this function since this function has been fixed in
 * PG version 14, previouly the function returned void
 */
static int
mobdb_lltoa(int64 value, char *a)
{
  uint64 uvalue = value;
  int len = 0;

  if (value < 0)
  {
    uvalue = (uint64) 0 - uvalue;
    a[len++] = '-';
  }

  len += pg_ulltoa_n(uvalue, a + len);
  a[len] = '\0';
  return len;
}

/**
 * @brief Return a string from an int8
 * @note PostgreSQL function: Datum int8out(PG_FUNCTION_ARGS)
 */
char *
pg_int8out(int64 val)
{
  char buf[MAXINT8LEN + 1];
  char *result;
  int len;

  len = mobdb_lltoa(val, buf) + 1;
  /*
   * Since the length is already known, we do a manual palloc() and memcpy()
   * to avoid the strlen() call that would otherwise be done in pstrdup().
   */
  result = palloc(len);
  memcpy(result, buf, len);
  return result;
}

#if 0 /* not used */
/**
 * @brief Convert an int8 to binary format
 * @note PostgreSQL function: Datum int8send(PG_FUNCTION_ARGS)
 */
bytea *
pg_int8send(int64 arg1)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, arg1);
  return pq_endtypsend(&buf);
}

/**
 * @brief Convert external binary format to int8
 * @note PostgreSQL function: Datum int8recv(PG_FUNCTION_ARGS)
 */
int64
pg_int8recv(StringInfo buf)
{
  return pq_getmsgint64(buf);
}
#endif /* not used */

/*****************************************************************************
 * Functions adapted from float.c
 *****************************************************************************/

/**
 * @brief Convert external binary format to float8
 * @note PostgreSQL function: Datum float8recv(PG_FUNCTION_ARGS)
 */
float8
pg_float8recv(StringInfo buf)
{
  return pq_getmsgfloat8(buf);
}

/**
 * @brief Convert float8 to binary format
 * @note PostgreSQL function: Datum float8send(PG_FUNCTION_ARGS)
 */
bytea *
pg_float8send(float8 num)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendfloat8(&buf, num);
  return pq_endtypsend(&buf);
}

/*****************************************************************************/

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
    ereport(ERROR, (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
      errmsg("input is out of range")));
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
    ereport(ERROR,
        (errcode(ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE),
         errmsg("input is out of range")));
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
 * Functions adapted from varlena.c
 *****************************************************************************/

/**
 * @brief Convert external binary format to text
 * @note PostgreSQL function: Datum textrecv(PG_FUNCTION_ARGS)
 */
text *
pg_textrecv(StringInfo buf)
{
  text *result;
  char *str;
  int nbytes;
  str = pq_getmsgtext(buf, buf->len - buf->cursor, &nbytes);
  result = cstring_to_text_with_len(str, nbytes);
  pfree(str);
  return result;
}

/**
 * @brief Convert text to binary format
 * @note PostgreSQL function: Datum textsend(PG_FUNCTION_ARGS)
 */
bytea *
pg_textsend(text *t)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendtext(&buf, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
  return pq_endtypsend(&buf);
}

/*****************************************************************************
 * Functions adapted from timestamp.c
 *****************************************************************************/

/**
 * @brief Convert a string to a timestamp.
 * @note PostgreSQL function: Datum timestamptz_in(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamptz_in(char *str, int32 typmod)
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
    DateTimeParseError(dterr, str, "timestamp with time zone");

  switch (dtype)
  {
    case DTK_DATE:
      if (tm2timestamp(tm, fsec, &tz, &result) != 0)
        ereport(ERROR,
            (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
             errmsg("timestamp out of range: \"%s\"", str)));
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
      elog(ERROR, "unexpected dtype %d while parsing timestamptz \"%s\"",
         dtype, str);
      TIMESTAMP_NOEND(result);
  }

  AdjustTimestampForTypmod(&result, typmod);

  return result;
}

/**
 * @brief Convert a timestamp to a string.
 * @note PostgreSQL function: Datum timestamptz_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamptz_out(TimestampTz dt)
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
  else if (timestamp2tm(dt, &tz, tm, &fsec, &tzn, NULL) == 0)
    EncodeDateTime(tm, fsec, true, tz, tzn, DateStyle, buf);
  else
    ereport(ERROR,
        (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("timestamp out of range")));

  result = pstrdup(buf);
  return result;
}

/**
 * @brief Convert timestamptz to binary format
 * @note PostgreSQL function: Datum timestamptz_send(PG_FUNCTION_ARGS)
 */
bytea *
pg_timestamptz_send(TimestampTz timestamp)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendint64(&buf, timestamp);
  return pq_endtypsend(&buf);
}

/**
 * @brief Convert external binary format to timestamptz
 * @note PostgreSQL function: Datum timestamptz_recv(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamptz_recv(StringInfo buf)
{
  // We do not use typmod
  int32 typmod = -1;
  TimestampTz timestamp;
  int tz;
  struct pg_tm tt,
         *tm = &tt;
  fsec_t fsec;

  timestamp = (TimestampTz) pq_getmsgint64(buf);

  /* range check: see if timestamptz_out would like it */
  if (TIMESTAMP_NOT_FINITE(timestamp))
     /* ok */ ;
  else if (timestamp2tm(timestamp, &tz, tm, &fsec, NULL, NULL) != 0 ||
       ! IS_VALID_TIMESTAMP(timestamp))
    ereport(ERROR, (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("timestamp out of range")));

  AdjustTimestampForTypmod(&timestamp, typmod);

  return timestamp;
}

/*****************************************************************************/

#define SAMESIGN(a,b) (((a) < 0) == ((b) < 0))

/**
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
    ereport(ERROR,
        (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("interval out of range")));

  result->day = span1->day + span2->day;
  if (SAMESIGN(span1->day, span2->day) &&
    ! SAMESIGN(result->day, span1->day))
    ereport(ERROR,
        (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("interval out of range")));

  result->time = span1->time + span2->time;
  if (SAMESIGN(span1->time, span2->time) &&
    ! SAMESIGN(result->time, span1->time))
    ereport(ERROR,
        (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("interval out of range")));

  return result;
}

/**
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
        ereport(ERROR,
            (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
             errmsg("timestamp out of range")));

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
        ereport(ERROR,
            (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
             errmsg("timestamp out of range")));
    }

    if (span->day != 0)
    {
      struct pg_tm tt,
             *tm = &tt;
      fsec_t    fsec;
      int      julian;

      if (timestamp2tm(timestamp, NULL, tm, &fsec, NULL, NULL) != 0)
        ereport(ERROR,
            (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
             errmsg("timestamp out of range")));

      /* Add days by converting to and from Julian */
      julian = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) + span->day;
      j2date(julian, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);

      if (tm2timestamp(tm, fsec, NULL, &timestamp) != 0)
        ereport(ERROR,
            (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
             errmsg("timestamp out of range")));
    }

    timestamp += span->time;

    if (!IS_VALID_TIMESTAMP(timestamp))
      ereport(ERROR,
          (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
           errmsg("timestamp out of range")));

    result = timestamp;
  }

  return result;
}

/**
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

/*
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

/*
 * @brief Compute the difference of two timestamps
 * @note PostgreSQL function: Datum timestamp_mi(PG_FUNCTION_ARGS)
 * The original code from PostgreSQL has `Timestamp` as arguments
 */
Interval *
pg_timestamp_mi(TimestampTz dt1, TimestampTz dt2)
{
  if (TIMESTAMP_NOT_FINITE(dt1) || TIMESTAMP_NOT_FINITE(dt2))
    ereport(ERROR,
        (errcode(ERRCODE_DATETIME_VALUE_OUT_OF_RANGE),
         errmsg("cannot subtract infinite timestamps")));

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

/*
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

/*
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

/*
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

/*
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
/*
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

/*
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

#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */
