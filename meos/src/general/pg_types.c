/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @file
 * @brief Functions for base and time types borrowed from PostgreSQL
 * version 14.2 source code
 * @note The MobilityDB functions pg_func(...) correspond to external
 * PostgreSQL functions func(PG_FUNCTION_ARGS). This enables bypassing the
 * function manager @p fmgr.c.
 */

#include "general/pg_types.h"

/* C */
#include <float.h>
#include <math.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <common/int128.h>
#include <utils/datetime.h>
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom_internal.h> /* for OUT_DOUBLE_BUFFER_SIZE */

/* MEOS */
#include <meos.h>
#include "general/temporal.h"

#if ! MEOS
  extern Datum call_function1(PGFunction func, Datum arg1);
  extern Datum call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3);
  extern Datum date_in(PG_FUNCTION_ARGS);
  extern Datum timestamp_in(PG_FUNCTION_ARGS);
  extern Datum timestamptz_in(PG_FUNCTION_ARGS);
  extern Datum date_out(PG_FUNCTION_ARGS);
  extern Datum timestamp_out(PG_FUNCTION_ARGS);
  extern Datum timestamptz_out(PG_FUNCTION_ARGS);
  extern Datum interval_out(PG_FUNCTION_ARGS);
#endif /* ! MEOS */

#if POSTGRESQL_VERSION_NUMBER >= 150000 || MEOS
  extern int64 pg_strtoint64(const char *s);
#else
  extern bool scanint8(const char *str, bool errorOK, int64 *result);
#endif

/* Definition in numutils.c */
extern int32 pg_strtoint32(const char *s);
#if POSTGRESQL_VERSION_NUMBER >= 130000
  extern int pg_ultoa_n(uint32 value, char *a);
  extern int pg_ulltoa_n(uint64 l, char *a);
#endif /* POSTGRESQL_VERSION_NUMBER >= 130000 */

/* To avoid including varlena.h */
extern int varstr_cmp(const char *arg1, int len1, const char *arg2, int len2,
  Oid collid);

/*****************************************************************************
 * Functions adapted from bool.c
 *****************************************************************************/

static bool
parse_bool_with_len(const char *value, size_t len, bool *result)
{
  switch (*value)
  {
    case 't':
    case 'T':
      if (pg_strncasecmp(value, "true", len) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case 'f':
    case 'F':
      if (pg_strncasecmp(value, "false", len) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case 'y':
    case 'Y':
      if (pg_strncasecmp(value, "yes", len) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case 'n':
    case 'N':
      if (pg_strncasecmp(value, "no", len) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case 'o':
    case 'O':
      /* 'o' is not unique enough */
      if (pg_strncasecmp(value, "on", (len > 2 ? len : 2)) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      else if (pg_strncasecmp(value, "off", (len > 2 ? len : 2)) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case '1':
      if (len == 1)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case '0':
      if (len == 1)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    default:
      break;
  }

  if (result)
    *result = false;    /* suppress compiler warning */
  return false;
}

/**
 * @ingroup meos_pg_types
 * @brief Return a boolean from its string representation
 * @details Convert @p "t" or @p "f" to 1 or 0.
 * Check explicitly for @p "true/false" and @p TRUE/FALSE, @p 1/0, @p YES/NO,
 * @p ON/OFF and reject other values. In the @p switch statement, check the
 * most-used possibilities first.
 * @note PostgreSQL function: @p boolin(PG_FUNCTION_ARGS)
 */
bool
bool_in(const char *in_str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) in_str))
    return false;

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

  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "invalid input syntax for type %s: \"%s\"", "boolean", in_str);

  /* not reached */
  return false;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a boolean
 * @details Convert 1 or 0 to @p "t" or @p "f"
 * @note PostgreSQL function: @p boolout(PG_FUNCTION_ARGS)
 */
char *
bool_out(bool b)
{
  char *result = palloc(2);
  result[0] = (b) ? 't' : 'f';
  result[1] = '\0';
  return result;
}

/*****************************************************************************
 * Functions adapted from int.c
 *****************************************************************************/

/**
 * @brief Return an int4 from a string
 * @note PostgreSQL function: @p int4in(PG_FUNCTION_ARGS)
 */
int32
int4_in(const char *str)
{
  return pg_strtoint32(str);
}

#if POSTGRESQL_VERSION_NUMBER >= 130000
/*
 * pg_ltoa: converts a signed 32-bit integer to its string representation and
 * returns strlen(a).
 *
 * It is the caller's responsibility to ensure that a is at least 12 bytes long,
 * which is enough room to hold a minus sign, a maximally long int32, and the
 * above terminating NUL.
 *
 * @note This function is copied here since it returned void in PostgreSQL
 * version < 14
 */
static int
mobdb_ltoa(int32 value, char *a)
{
  uint32 uvalue = (uint32) value;
  int len = 0;

  if (value < 0)
  {
    uvalue = (uint32) 0 - uvalue;
    a[len++] = '-';
  }
  len += pg_ultoa_n(uvalue, a + len);
  a[len] = '\0';
  return len;
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 130000 */

/**
 * @brief Return a string from an int4
 * @note PostgreSQL function: @p int4out(PG_FUNCTION_ARGS)
 */
char *
int4_out(int32 val)
{
  char *result = palloc(12);  /* sign, 10 digits, '\0' */
#if POSTGRESQL_VERSION_NUMBER >= 130000
  mobdb_ltoa(val, result);
#else
  sprintf(result, "%d", val);
#endif
  return result;
}

/*****************************************************************************
 * Functions adapted from int8.c
 *****************************************************************************/

/* Sign + the most decimal digits an 8-byte number could have */
#define MAXINT8LEN 20

/**
 * @brief Return an int8 from a string
 * @return On error return @p PG_INT64_MAX;
 * @note PostgreSQL function: @p int8in(PG_FUNCTION_ARGS)
 */
int64
int8_in(const char *str)
{
#if POSTGRESQL_VERSION_NUMBER >= 150000 || MEOS
  int64 result = pg_strtoint64(str);
#else
  int64 result;
  (void) scanint8(str, false, &result);
#endif
  return result;
}

#if POSTGRESQL_VERSION_NUMBER >= 130000
/*
 * pg_lltoa: converts a signed 64-bit integer to its string representation and
 * returns strlen(a).
 *
 * Caller must ensure that 'a' points to enough memory to hold the result
 * (at least MAXINT8LEN + 1 bytes, counting a leading sign and trailing NUL).
 *
 * @note This function is copied here since it returned void in PostgreSQL
 * version < 14
 */
int
mobdb_lltoa(int64 value, char *a)
{
  uint64    uvalue = value;
  int      len = 0;

  if (value < 0)
  {
    uvalue = (uint64) 0 - uvalue;
    a[len++] = '-';
  }

  len += pg_ulltoa_n(uvalue, a + len);
  a[len] = '\0';
  return len;
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 130000 */

/**
 * @brief Return a string from an @p int8
 * @note PostgreSQL function: @p int8out(PG_FUNCTION_ARGS)
 */
char *
int8_out(int64 val)
{
  char *result;
#if POSTGRESQL_VERSION_NUMBER >= 130000
  char buf[MAXINT8LEN + 1];
  int len = mobdb_lltoa(val, buf) + 1;
  /*
   * Since the length is already known, we do a manual palloc() and memcpy()
   * to avoid the strlen() call that would otherwise be done in pstrdup().
   */
  result = palloc(len);
  memcpy(result, buf, len);
#else
  result = palloc(MAXINT8LEN + 1);
  sprintf(result, "%ld", val);
#endif
  return result;
}

/*****************************************************************************
 * Functions adapted from float.c
 *****************************************************************************/

/**
 * float8in_internal_opt_error - guts of float8in()
 * @return On error return @p DBL_MAX
 *
 * This is exposed for use by functions that want a reasonably
 * platform-independent way of inputting doubles.  The behavior is
 * essentially like strtod + ereport on error, but note the following
 * differences:
 * 1. Both leading and trailing whitespace are skipped.
 * 2. If endptr_p is NULL, we throw error if there's trailing junk.
 * Otherwise, it's up to the caller to complain about trailing junk.
 * 3. In event of a syntax error, the report mentions the given type_name
 * and prints orig_string as the input; this is meant to support use of
 * this function with types such as "box" and "point", where what we are
 * parsing here is just a substring of orig_string.
 *
 * "num" could validly be declared "const char *", but that results in an
 * unreasonable amount of extra casting both here and in callers, so we don't.
 */
double
float8_in_opt_error(char *num, const char *type_name, const char *orig_string)
{
  double    val;
  char     *endptr;

  /* skip leading whitespace */
  while (*num != '\0' && isspace((unsigned char) *num))
    num++;

  /*
   * Check for an empty-string input to begin with, to avoid the vagaries of
   * strtod() on different platforms.
   */
  if (*num == '\0')
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "invalid input syntax for type %s: \"%s\"", type_name, orig_string);
    return DBL_MAX;
  }

  errno = 0;
  val = strtod(num, &endptr);

  /* did we not see anything that looks like a double? */
  if (endptr == num || errno != 0)
  {
    int      save_errno = errno;

    /*
     * C99 requires that strtod() accept NaN, [+-]Infinity, and [+-]Inf,
     * but not all platforms support all of these (and some accept them
     * but set ERANGE anyway...)  Therefore, we check for these inputs
     * ourselves if strtod() fails.
     *
     * Note: C99 also requires hexadecimal input as well as some extended
     * forms of NaN, but we consider these forms unportable and don't try
     * to support them.  You can use 'em if your strtod() takes 'em.
     */
    if (pg_strncasecmp(num, "NaN", 3) == 0)
    {
      val = get_float8_nan();
      endptr = num + 3;
    }
    else if (pg_strncasecmp(num, "Infinity", 8) == 0)
    {
      val = get_float8_infinity();
      endptr = num + 8;
    }
    else if (pg_strncasecmp(num, "+Infinity", 9) == 0)
    {
      val = get_float8_infinity();
      endptr = num + 9;
    }
    else if (pg_strncasecmp(num, "-Infinity", 9) == 0)
    {
      val = -get_float8_infinity();
      endptr = num + 9;
    }
    else if (pg_strncasecmp(num, "inf", 3) == 0)
    {
      val = get_float8_infinity();
      endptr = num + 3;
    }
    else if (pg_strncasecmp(num, "+inf", 4) == 0)
    {
      val = get_float8_infinity();
      endptr = num + 4;
    }
    else if (pg_strncasecmp(num, "-inf", 4) == 0)
    {
      val = -get_float8_infinity();
      endptr = num + 4;
    }
    else if (save_errno == ERANGE)
    {
      /*
       * Some platforms return ERANGE for denormalized numbers (those
       * that are not zero, but are too close to zero to have full
       * precision).  We'd prefer not to throw error for that, so try to
       * detect whether it's a "real" out-of-range condition by checking
       * to see if the result is zero or huge.
       *
       * On error, we intentionally complain about double precision not
       * the given type name, and we print only the part of the string
       * that is the current number.
       */
      if (val == 0.0 || val >= HUGE_VAL || val <= -HUGE_VAL)
      {
        char *errnumber = strdup(num);
        errnumber[endptr - num] = '\0';
        meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
          "\"%s\" is out of range for type double precision", errnumber);
        pfree(errnumber);
        return DBL_MAX;
      }
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "invalid input syntax for type %s: \"%s\"", type_name, orig_string);
      return DBL_MAX;
    }
  }

  /* skip trailing whitespace */
  while (*endptr != '\0' && isspace((unsigned char) *endptr))
    endptr++;

  return val;
}

/*
 * Interface to float8in_internal_opt_error().
 */
double
float8_in(const char *num, const char *type_name, const char *orig_string)
{
  return float8_in_opt_error((char *) num, type_name, orig_string);
}

/*
 * This function uses the PostGIS function lwprint_double to print an ordinate
 * value using at most **maxdd** number of decimal digits. The actual number
 * of printed decimal digits may be less than the requested ones if out of
 * significant digits.
 *
 * The function will write at most OUT_DOUBLE_BUFFER_SIZE bytes, including the
 * terminating NULL.
 */
char *
float8_out(double num, int maxdd)
{
  assert(maxdd >= 0);

  char *ascii = palloc(OUT_DOUBLE_BUFFER_SIZE);
  lwprint_double(num, maxdd, ascii);
  return ascii;
}

/**
 * @brief Return the sine of arg1 (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dsin(PG_FUNCTION_ARGS)
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
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "input is out of range");
    return DBL_MAX;
  }
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/**
 * @brief Return the cosine of arg1 (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dcos(PG_FUNCTION_ARGS)
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
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "input is out of range");
    return DBL_MAX;
  }
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/**
 * @brief Return the arctan of a double (radians)
 * @note PostgreSQL function: @p datan(PG_FUNCTION_ARGS)
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
 * @brief Return the arctan of two doubles (radians)
 * @note PostgreSQL function: @p datan2d(PG_FUNCTION_ARGS)
 */
float8
pg_datan2(float8 arg1, float8 arg2)
{
  /* Per the POSIX spec, return NaN if either input is NaN */
  if (isnan(arg1) || isnan(arg2))
    return get_float8_nan();

  /*
   * atan2 maps all inputs to values in the range [-Pi, Pi], so the result
   * should always be finite, even if the inputs are infinite.
   */
  float8 result = atan2(arg1, arg2);
  if (unlikely(isinf(result)))
    float_overflow_error();

  return result;
}

/*****************************************************************************
 * Functions adapted from date.c
 *****************************************************************************/

#if ! MEOS
/**
 * @ingroup meos_pg_types
 * @brief Return a date from its string representation
 * @return On error return @p DATEVAL_NOEND
 * @note PostgreSQL function: @p date_in(PG_FUNCTION_ARGS)
 */
DateADT
pg_date_in(const char *str)
{
  Datum arg = CStringGetDatum(str);
  return DatumGetTimestampTz(call_function1(date_in, arg));
}
#else
/**
 * @ingroup meos_pg_types
 * @brief Return a date from its string representation
 * @return On error return @p DATEVAL_NOEND
 * @note PostgreSQL function: @p date_in(PG_FUNCTION_ARGS)
 */
DateADT
pg_date_in(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return DATEVAL_NOEND;

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
#if POSTGRESQL_VERSION_NUMBER >= 160000
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tzp, NULL);
#else
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tzp);
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
  if (dterr != 0)
  {
#if POSTGRESQL_VERSION_NUMBER >= 160000
    DateTimeParseError(dterr, NULL, str, "date", NULL);
#else
    DateTimeParseError(dterr, str, "date");
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
    return DATEVAL_NOEND;
  }

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
#if POSTGRESQL_VERSION_NUMBER >= 160000
      DateTimeParseError(DTERR_BAD_FORMAT, NULL, str, "date", NULL);
#else
      DateTimeParseError(DTERR_BAD_FORMAT, str, "date");
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
      return DATEVAL_NOEND;
  }

  /* Prevent overflow in Julian-day routines */
  if (!IS_VALID_JULIAN(tm->tm_year, tm->tm_mon, tm->tm_mday))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "date out of range: \"%s\"", str);
    return DATEVAL_NOEND;
  }

  date = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;

  /* Now check for just-out-of-range dates */
  if (!IS_VALID_DATE(date))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "date out of range: \"%s\"", str);
    return DATEVAL_NOEND;
  }

  return date;
}
#endif /* MEOS */

#if ! MEOS
/**
 * @brief Return a string converted to a date
 * @note PostgreSQL function: @p date_in(PG_FUNCTION_ARGS)
 */
char *
pg_date_out(DateADT date)
{
  Datum d = DateADTGetDatum(date);
  return DatumGetCString(call_function1(date_out, d));
}
#else
/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a date
 * @note PostgreSQL function: @p date_in(PG_FUNCTION_ARGS)
 */
char *
pg_date_out(DateADT date)
{
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

  return pstrdup(buf);
}
#endif /* MEOS */

#if MEOS || POSTGRESQL_VERSION_NUMBER < 130000
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
#endif /* MEOS || POSTGRESQL_VERSION_NUMBER < 130000 */

/**
 * @ingroup meos_pg_types
 * @brief Return a timestamptz converted to a date
 */
TimestampTz
date_to_timestamptz(DateADT d)
{
  return date2timestamptz_opt_overflow(d, NULL);
}

/**
 * @ingroup meos_pg_types
 * @brief Return the addition of a date and a number of days
 * @details Must handle both positive and negative numbers of days.
 * @note PostgreSQL function: @p date_pli(PG_FUNCTION_ARGS)
 */
DateADT
add_date_int(DateADT d, int32 days)
{
  DateADT result;

  if (DATE_NOT_FINITE(d))
    return d; /* can't change infinity */

  result = d + days;

  /* Check for integer overflow and out-of-allowed-range */
  if ((days >= 0 ? (result < d) : (result > d)) || !IS_VALID_DATE(result))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "date out of range");
    return DATEVAL_NOEND;
  }

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the subtraction of a date and a number of days
 */
DateADT
minus_date_int(DateADT d, int32 days)
{
  DateADT result;

  if (DATE_NOT_FINITE(d))
    return d; /* can't change infinity */

  result = d - days;

  /* Check for integer overflow and out-of-allowed-range */
  if ((days >= 0 ? (result > d) : (result < d)) || !IS_VALID_DATE(result))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "date out of range");
    return DATEVAL_NOEND;
  }

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the subtraction of two dates
 * @note PostgreSQL function: @p date_mi(PG_FUNCTION_ARGS)
 */
Interval *
minus_date_date(DateADT d1, DateADT d2)
{
  if (DATE_NOT_FINITE(d1) || DATE_NOT_FINITE(d2))
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "cannot subtract infinite dates");

  Interval *result = palloc0(sizeof(Interval));
  result->day = (int32) (d1 - d2);
  return result;
}

/*****************************************************************************
 *   Time ADT
 *****************************************************************************/

#if MEOS
/* MEOSAdjustTimeForTypmod()
 * Force the precision of the time value to a specified value.
 * Uses *exactly* the same code as in MEOSAdjustTimestampForTypmod()
 * but we make a separate copy because those types do not
 * have a fundamental tie together but rather a coincidence of
 * implementation. - thomas
 */
void
MEOSAdjustTimeForTypmod(TimeADT *time, int32 typmod)
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
  return;
}

/**
 * @ingroup meos_pg_types
 * @brief Return a time from its string representation
 * @note PostgreSQL function: @p time_in(PG_FUNCTION_ARGS)
 */
TimeADT
pg_time_in(const char *str, int32 typmod)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return DT_NOEND;

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
  {
#if POSTGRESQL_VERSION_NUMBER >= 160000
    DateTimeParseError(dterr, NULL, str, "time", NULL);
#else
    DateTimeParseError(dterr, str, "time");
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
    return DT_NOEND;
  }

  tm2time(tm, fsec, &result);
  MEOSAdjustTimeForTypmod(&result, typmod);

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a time
 * @note PostgreSQL function: @p time_out(PG_FUNCTION_ARGS)
 */
char *
pg_time_out(TimeADT time)
{
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  char buf[MAXDATELEN + 1];

  time2tm(time, tm, &fsec);
  EncodeTimeOnly(tm, fsec, false, 0, DateStyle, buf);

  return pstrdup(buf);
}
#endif /* MEOS */

/*****************************************************************************
 * Functions adapted from timestamp.c
 *****************************************************************************/

#if ! MEOS
/**
 * @brief Return a string converted to a timestamp with timezone.
 * @note PostgreSQL function: @p timestamptz_in(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamptz_in(const char *str, int32 typmod)
{
  Datum arg1 = CStringGetDatum(str);
  Datum arg3 = Int32GetDatum(typmod);
  TimestampTz result = DatumGetTimestampTz(call_function3(timestamptz_in, arg1,
    (Datum) 0, arg3));
  return result;
}
#else
/*
 * MEOSAdjustTimestampForTypmodError --- round off a timestamp to suit given typmod
 * Works for either timestamp or timestamptz.
 */
bool
MEOSAdjustTimestampForTypmodError(Timestamp *time, int32 typmod, bool *error)
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

      meos_error(ERROR, MEOS_ERR_INVALID_ARG,
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

void
MEOSAdjustTimestampForTypmod(Timestamp *time, int32 typmod)
{
  (void) MEOSAdjustTimestampForTypmodError(time, typmod, NULL);
  return;
}

/**
 * @brief Return a string converted to a either timestamp or a timestamp with
 * timezone
 * @result On error return DT_NOEND
 * @note The function returns a TimestampTz that must be cast to a Timestamp
 * when calling the function with the last argument to false
 */
TimestampTz
timestamp_in_common(const char *str, int32 typmod, bool withtz)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return DT_NOEND;

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
  if (dterr != 0)
    return DT_NOEND;

#if POSTGRESQL_VERSION_NUMBER >= 160000
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz, NULL);
#else
    dterr = DecodeDateTime(field, ftype, nf, &dtype, tm, &fsec, &tz);
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */

  if (dterr != 0)
  {
    char *type_str = withtz ? "timestamp with time zone" : "time";
#if POSTGRESQL_VERSION_NUMBER >= 160000
    DateTimeParseError(dterr, NULL, str, type_str, NULL);
#else
    DateTimeParseError(dterr, str, type_str);
#endif
    return DT_NOEND;
  }

  switch (dtype)
  {
    case DTK_DATE:
    {
      int status = (withtz) ?
        tm2timestamp(tm, fsec, &tz, &result) :
        tm2timestamp(tm, fsec, NULL, &result);
      if (status != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range: \"%s\"", str);
        return DT_NOEND;
      }
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
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "unexpected dtype %d while parsing timestamp%s \"%s\"",
        dtype, (withtz) ? "tz" : "", str);
      TIMESTAMP_NOEND(result);
  }

  MEOSAdjustTimestampForTypmod(&result, typmod);

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return a timestamp without time zone from its string representation
 * @return On error return @p DT_NOEND
 * @note PostgreSQL function: @p timestamp_in(PG_FUNCTION_ARGS)
 */
Timestamp
pg_timestamp_in(const char *str, int32 typmod)
{
  return (Timestamp) timestamp_in_common(str, typmod, false);
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a timestamp with time zone
 * @return On error return @p DT_NOEND
 * @note PostgreSQL function: @p timestamptz_in(PG_FUNCTION_ARGS)
 */
TimestampTz
pg_timestamptz_in(const char *str, int32 typmod)
{
  return timestamp_in_common(str, typmod, true);
}
#endif /* MEOS */

#if ! MEOS
/**
 * @ingroup meos_pg_types
 * @brief Return a timestamp with timezone converted to a string
 * @return On error return @p NULL
 * @note PostgreSQL function: @p timestamptz_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamptz_out(TimestampTz t)
{
  Datum d = TimestampTzGetDatum(t);
  return DatumGetCString(call_function1(timestamptz_out, d));
}
#else
/**
 * @brief Return either a timestamp or a timestamptz converted to a string
 */
char *
timestamp_out_common(TimestampTz t, bool withtz)
{
  int tz;
  struct pg_tm tt,
         *tm = &tt;
  fsec_t fsec;
  const char *tzn;
  char buf[MAXDATELEN + 1];

  if (TIMESTAMP_NOT_FINITE(t))
    EncodeSpecialTimestamp(t, buf);
  else if (withtz && timestamp2tm(t, &tz, tm, &fsec, &tzn, NULL) == 0)
    EncodeDateTime(tm, fsec, true, tz, tzn, DateStyle, buf);
  else if (! withtz && timestamp2tm(t, NULL, tm, &fsec, NULL, NULL) == 0)
    EncodeDateTime(tm, fsec, false, 0, NULL, DateStyle, buf);
  else
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "timestamp out of range");
    return NULL;
  }

  return pstrdup(buf);
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a timestamp with timezone
 * @note PostgreSQL function: @p timestamptz_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamptz_out(TimestampTz t)
{
  return timestamp_out_common(t, true);
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of a timestamp without timezone
 * @note PostgreSQL function: @p timestamp_out(PG_FUNCTION_ARGS)
 */
char *
pg_timestamp_out(Timestamp t)
{
  return timestamp_out_common((Timestamp) t, false);
}
#endif /* MEOS */

/**
 * @ingroup meos_pg_types
 * @brief Return a timestamp with time zone converted to a date
 * @note PostgreSQL function timestamptz_date()
 */
DateADT
timestamptz_to_date(TimestampTz t)
{
  DateADT result;
  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  int tz;

  if (TIMESTAMP_IS_NOBEGIN(t))
    DATE_NOBEGIN(result);
  else if (TIMESTAMP_IS_NOEND(t))
    DATE_NOEND(result);
  else
  {
    if (timestamp2tm(t, &tz, tm, &fsec, NULL, NULL) != 0)
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range");

    result = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) - POSTGRES_EPOCH_JDATE;
  }

  return result;
}

/*****************************************************************************/

#if ! MEOS
/**
 * @brief Return the string representation of an interval
 * @return On error return @p NULL
 * @note PostgreSQL function: @p interval_out(PG_FUNCTION_ARGS)
 */
char *
pg_interval_out(const Interval *interval)
{
  Datum d = PointerGetDatum(interval);
  return DatumGetCString(call_function1(interval_out, d));
}
#else /*if MEOS */
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
    {
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "unrecognized interval typmod: %d", typmod);
      return;
    }

    /* Need to adjust sub-second precision? */
    if (precision != INTERVAL_FULL_PRECISION)
    {
      if (precision < 0 || precision > MAX_INTERVAL_PRECISION)
      {
        meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
          "interval(%d) precision must be between %d and %d",
          precision, 0, MAX_INTERVAL_PRECISION);
        return;
      }

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
  return;
}

/**
 * @ingroup meos_pg_types
 * @brief Return an interval from its string representation
 * @note PostgreSQL function: @p interval_in(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_in(const char *str, int32 typmod)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;

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
#if POSTGRESQL_VERSION_NUMBER >= 160000
    DateTimeParseError(dterr, NULL, str, "interval", NULL);
#else
    DateTimeParseError(dterr, str, "interval");
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
    return NULL;
  }

  result = (Interval *) palloc(sizeof(Interval));

  switch (dtype)
  {
    case DTK_DELTA:
      if (tm2interval(tm, fsec, result) != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "interval out of range");
        pfree(result);
        return NULL;
      }
      break;

    default:
      meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
        "unexpected dtype %d while parsing interval \"%s\"", dtype, str);
      pfree(result);
      return NULL;
  }

  AdjustIntervalForTypmod(result, typmod);

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return an interval constructed from its arguments
 * @note PostgreSQL function: @p make_interval(PG_FUNCTION_ARGS)
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
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    return NULL;
  }

  result = (Interval *) palloc(sizeof(Interval));
  result->month = years * MONTHS_PER_YEAR + months;
  result->day = weeks * 7 + days;

  secs = rint(secs * USECS_PER_SEC);
  result->time = hours * ((int64) SECS_PER_HOUR * USECS_PER_SEC) +
    mins * ((int64) SECS_PER_MINUTE * USECS_PER_SEC) + (int64) secs;

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the string representation of an interval
 * @note PostgreSQL function: @p interval_out(PG_FUNCTION_ARGS)
 */
char *
pg_interval_out(const Interval *interv)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interv))
    return NULL;

  struct pg_tm tt, *tm = &tt;
  fsec_t fsec;
  char buf[MAXDATELEN + 1];

  if (interval2tm(*interv, tm, &fsec) != 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not convert interval to tm");
    return NULL;
  }

  EncodeInterval(tm, fsec, IntervalStyle, buf);

  return pstrdup(buf);
}

/**
 * @ingroup meos_pg_types
 * @brief Return the multiplication of an interval and a factor
 * @note PostgreSQL function: @p interval_mul(PG_FUNCTION_ARGS)
 */
Interval *
mult_interval_double(const Interval *interv, double factor)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interv))
    return NULL;

  double month_remainder_days, sec_remainder, result_double;
  int32 orig_month = interv->month,
    orig_day = interv->day;
  Interval *result;

  result = (Interval *) palloc(sizeof(Interval));

  result_double = interv->month * factor;
  if (isnan(result_double) ||
    result_double > INT_MAX || result_double < INT_MIN)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    return NULL;
  }
  result->month = (int32) result_double;

  result_double = interv->day * factor;
  if (isnan(result_double) ||
    result_double > INT_MAX || result_double < INT_MIN)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    return NULL;
  }
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
  sec_remainder = (orig_day * factor - result->day +
           month_remainder_days - (int) month_remainder_days) * SECS_PER_DAY;
  sec_remainder = TSROUND(sec_remainder);

  /*
   * Might have 24:00:00 hours due to rounding, or >24 hours because of time
   * cascade from months and days.  It might still be >24 if the combination
   * of cascade and the seconds factor operation itself.
   */
  if (Abs(sec_remainder) >= SECS_PER_DAY)
  {
    result->day += (int) (sec_remainder / SECS_PER_DAY);
    sec_remainder -= (int) (sec_remainder / SECS_PER_DAY) * SECS_PER_DAY;
  }

  /* cascade units down */
  result->day += (int32) month_remainder_days;
  result_double = rint(interv->time * factor + sec_remainder * USECS_PER_SEC);
  if (isnan(result_double) || !FLOAT8_FITS_IN_INT64(result_double))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    return NULL;
  }
  result->time = (int64) result_double;

  return result;
}
#endif /* MEOS */

/*****************************************************************************/

#define SAMESIGN(a,b) (((a) < 0) == ((b) < 0))

/**
 * @ingroup meos_pg_types
 * @brief Return the addition of two intervals
 * @note PostgreSQL function: @p interval_pl(PG_FUNCTION_ARGS)
 */
Interval *
add_interval_interval(const Interval *interv1, const Interval *interv2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interv1) || ! ensure_not_null((void *) interv2))
    return NULL;

  Interval *result = palloc(sizeof(Interval));

  result->month = interv1->month + interv2->month;
  /* overflow check copied from int4pl */
  if (SAMESIGN(interv1->month, interv2->month) &&
    ! SAMESIGN(result->month, interv1->month))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    pfree(result);
    return NULL;
  }

  result->day = interv1->day + interv2->day;
  if (SAMESIGN(interv1->day, interv2->day) &&
    ! SAMESIGN(result->day, interv1->day))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    pfree(result);
    return NULL;
  }

  result->time = interv1->time + interv2->time;
  if (SAMESIGN(interv1->time, interv2->time) &&
    ! SAMESIGN(result->time, interv1->time))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE, "interval out of range");
    pfree(result);
    return NULL;
  }

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the addition of a timestamp and an interval
 * @details Note that interval has provisions for qualitative year/month and
 * day units, so try to do the right thing with them.
 * To add a month, increment the month, and use the same day of month.
 * Then, if the next month has fewer days, set the day of month
 * to the last day of month.
 * To add a day, increment the mday, and use the same time of day.
 * Lastly, add in the "quantitative time".
 * @return On error return DT_NOEND
 * @note PostgreSQL function: @p timestamp_pl_interval(PG_FUNCTION_ARGS)
 */
TimestampTz
add_timestamptz_interval(TimestampTz t, const Interval *interv)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interv))
    return DT_NOEND;

  Timestamp result;

  if (TIMESTAMP_NOT_FINITE(t))
    result = t;
  else
  {
    if (interv->month != 0)
    {
      struct pg_tm tt,
             *tm = &tt;
      fsec_t    fsec;

      if (timestamp2tm(t, NULL, tm, &fsec, NULL, NULL) != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range");
        return DT_NOEND;
      }

      tm->tm_mon += interv->month;
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

      if (tm2timestamp(tm, fsec, NULL, &t) != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    if (interv->day != 0)
    {
      struct pg_tm tt,
             *tm = &tt;
      fsec_t    fsec;
      int      julian;

      if (timestamp2tm(t, NULL, tm, &fsec, NULL, NULL) != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range");
        return DT_NOEND;
      }

      /* Add days by converting to and from Julian */
      julian = date2j(tm->tm_year, tm->tm_mon, tm->tm_mday) + interv->day;
      j2date(julian, &tm->tm_year, &tm->tm_mon, &tm->tm_mday);

      if (tm2timestamp(tm, fsec, NULL, &t) != 0)
      {
        meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
          "timestamp out of range");
        return DT_NOEND;
      }
    }

    t += interv->time;

    if (!IS_VALID_TIMESTAMP(t))
    {
      meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
        "timestamp out of range");
      return DT_NOEND;
    }

    result = t;
  }

  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the subtraction of a timestamptz and an interval
 * @note PostgreSQL function: @p timestamp_mi_interval(PG_FUNCTION_ARGS)
 */
TimestampTz
minus_timestamptz_interval(TimestampTz t, const Interval *interv)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interv))
    return DT_NOEND;

  Interval tinterv;
  tinterv.month = -interv->month;
  tinterv.day = -interv->day;
  tinterv.time = -interv->time;
  return add_timestamptz_interval(t, &tinterv);
}

/**
 * @brief Add an interval to a timestamp data type.
 * @details Adjust interval so 'time' contains less than a whole day, adding
 *  the excess to 'day'.  This is useful for  situations (such as non-TZ) where
 * '1 day' = '24 hours' is valid, e.g. interval subtraction and division.
 * @note PostgreSQL function: @p interval_justify_hours(PG_FUNCTION_ARGS)
 */
Interval *
pg_interval_justify_hours(const Interval *interv)
{
  Interval *result = palloc(sizeof(Interval));
  result->month = interv->month;
  result->day = interv->day;
  result->time = interv->time;

  TimeOffset wholeday = 0; /* make compiler quiet */
  TMODULO(result->time, wholeday, USECS_PER_DAY);
  result->day += (int32) wholeday;  /* could overflow... */

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
 * @ingroup meos_pg_types
 * @brief Return the subtraction of two timestamptz values
 * @note PostgreSQL function: @p timestamp_mi(PG_FUNCTION_ARGS). Notice that
 * the original code from PostgreSQL has @p Timestamp as arguments
 */
Interval *
minus_timestamptz_timestamptz(TimestampTz t1, TimestampTz t2)
{
  /* Ensure validity of the arguments */
  if (TIMESTAMP_NOT_FINITE(t1) || TIMESTAMP_NOT_FINITE(t2))
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "cannot subtract infinite timestamps");
    return NULL;
  }

  Interval interv;
  interv.time = t1 - t2;
  interv.month = 0;
  interv.day = 0;
  return pg_interval_justify_hours(&interv);
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
  int64 ndays;

  /*
   * Combine the month and day fields into an integral number of days.
   * Because the inputs are int32, int64 arithmetic suffices here.
   */
  ndays = interval->month * INT64CONST(30);
  ndays += interval->day;

  /* Widen time field to 128 bits */
  span = int64_to_int128(interval->time);

  /* Scale up days to microseconds, forming a 128-bit product */
  int128_add_int64_mul_int64(&span, ndays, USECS_PER_DAY);

  return span;
}

/**
 * @ingroup meos_pg_types
 * @brief Return the comparison of two intervals
 * @note PostgreSQL function: @p interval_cmp(PG_FUNCTION_ARGS)
 */
int
pg_interval_cmp(const Interval *interval1, const Interval *interval2)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) interval1) ||
      ! ensure_not_null((void *) interval2))
    return INT_MAX;

  INT128 span1 = interval_cmp_value(interval1);
  INT128 span2 = interval_cmp_value(interval2);
  return int128_compare(span1, span2);
}

/*****************************************************************************
 * Text and binary string functions
 *****************************************************************************/

/**
 * @brief Return a C binary string converted to a bytea
 */
bytea *
bstring2bytea(const uint8_t *wkb, size_t size)
{
  bytea *result = palloc(size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, size);
  SET_VARSIZE(result, size + VARHDRSZ);
  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return a C string converted to a text
 * @param[in] str String
 * @note We don't include `<utils/builtins.h>` to avoid collisions with
 * `json-c/json.h`
 * @note Function taken from PostGIS file `lwgeom_in_geojson.c`
 */
text *
cstring2text(const char *str)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) str))
    return NULL;

  size_t len = strlen(str);
  text *result = palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy(VARDATA(result), str, len);
  return result;
}

/**
 * @ingroup meos_pg_types
 * @brief Return a text converted to a C string
 * @param[in] txt Text
 * @note We don't include @p <utils/builtins.h> to avoid collisions with
 * @p json-c/json.h
 * @note Function taken from PostGIS file @p lwgeom_in_geojson.c
 */
char *
text2cstring(const text *txt)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) txt))
    return NULL;

  size_t size = VARSIZE_ANY_EXHDR(txt);
  char *str = palloc(size + 1);
  memcpy(str, VARDATA(txt), size);
  str[size] = '\0';
  return str;
}

#if MEOS
/* Simplified version of the function in varlena.c where LC_COLLATE is C */
int
varstr_cmp(const char *arg1, int len1, const char *arg2, int len2,
  Oid collid __attribute__((unused)))
{
  int result = memcmp(arg1, arg2, Min(len1, len2));
  if ((result == 0) && (len1 != len2))
    result = (len1 < len2) ? -1 : 1;
  return result;
}
#endif /* MEOS */

/**
 * @ingroup meos_pg_types
 * @brief Comparison function for text values
 * @param[in] txt1,txt2 Text values
 * @note Function derived from PostgreSQL since it is declared static. Notice 
 * that the third attribute collid of the original function has been removed
 * while waiting for a proper localization management
 */
int
text_cmp(const text *txt1, const text *txt2)
{
  char *t1p = VARDATA_ANY(txt1);
  char *t2p = VARDATA_ANY(txt2);
  int len1 = (int) VARSIZE_ANY_EXHDR(txt1);
  int len2 = (int) VARSIZE_ANY_EXHDR(txt2);
  return varstr_cmp(t1p, len1, t2p, len2, DEFAULT_COLLATION_OID);
}

/**
 * @ingroup meos_pg_types
 * @brief Copy a text value
 * @param[in] txt Text
 */
text *
text_copy(const text *txt)
{
  assert(txt);
  text *result = palloc(VARSIZE(txt));
  memcpy(result, txt, VARSIZE(txt));
  return result;
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
 * @note PostgreSQL function: @p hashint8(PG_FUNCTION_ARGS)
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
 * @note PostgreSQL function: @p hashint8extended(PG_FUNCTION_ARGS)
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
 * @note PostgreSQL function: @p hashfloat8(PG_FUNCTION_ARGS)
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

/**
 * @brief Get the 64-bit hash value of a @p float64 value
 * @note PostgreSQL function: @p hashfloat8extended(PG_FUNCTION_ARGS)
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

/**
 * @brief Get the 32-bit hash value of an text value.
 * @note PostgreSQL function: @p hashtext(PG_FUNCTION_ARGS).
 * We simulate what would happen using @p DEFAULT_COLLATION_OID
 */
uint32
pg_hashtext(text *key)
{
  return DatumGetUInt32(hash_any((unsigned char *) VARDATA_ANY(key),
    VARSIZE_ANY_EXHDR(key)));
}

/**
 * @brief Get the 32-bit hash value of an text value.
 * @note PostgreSQL function: @p hashtext(PG_FUNCTION_ARGS).
 * We simulate what would happen using @p DEFAULT_COLLATION_OID
 */
uint64
pg_hashtextextended(text *key, uint64 seed)
{
  return DatumGetUInt64(hash_any_extended(
    (unsigned char *) VARDATA_ANY(key), VARSIZE_ANY_EXHDR(key), seed));
}

/*****************************************************************************/

