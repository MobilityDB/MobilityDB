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
 * @file basetype_inout.c
 * @brief Functions for string input/output base types taken from PostgreSQL
 * version 14.2 source code.
 */

// #include "general/basetype_parser.h"


/* PostgreSQL */
#include <postgres.h>
#include <common/int.h>
#include <common/shortest_dec.h>
#include <port/pg_bitutils.h>
#include <utils/float.h>

/* MobilityDB */
// #include <libmeos.h>
// #include "general/temporal_util.h"
// #include "general/temporal_parser.h"

/* Definitions from builtins.h to avoid including it */
#if POSTGRESQL_VERSION_NUMBER >= 150000
  extern int64 pg_strtoint64(const char *s);
#else
  extern bool scanint8(const char *str, bool errorOK, int64 *result);
#endif

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
 * @brief Convert "t" or "f" to 1 or 0
 *
 * Check explicitly for "true/false" and TRUE/FALSE, 1/0, YES/NO, ON/OFF.
 * Reject other values.
 *
 * In the switch statement, check the most-used possibilities first.
 * @note PostgreSQL function: Datum boolin(PG_FUNCTION_ARGS)
 */
bool
bool_in(const char *in_str)
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

  elog(ERROR, "invalid input syntax for type %s: \"%s\"", "boolean", in_str);

  /* not reached */
  return false;
}

/**
 * @brief Convert 1 or 0 to "t" or "f"
 * @note PostgreSQL function: Datum boolout(PG_FUNCTION_ARGS)
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
 * Functions adapted from numutils.c
 *****************************************************************************/

/*
 * A table of all two-digit numbers. This is used to speed up decimal digit
 * generation by copying pairs of digits into the final output.
 */
static const char DIGIT_TABLE[200] =
"00" "01" "02" "03" "04" "05" "06" "07" "08" "09"
"10" "11" "12" "13" "14" "15" "16" "17" "18" "19"
"20" "21" "22" "23" "24" "25" "26" "27" "28" "29"
"30" "31" "32" "33" "34" "35" "36" "37" "38" "39"
"40" "41" "42" "43" "44" "45" "46" "47" "48" "49"
"50" "51" "52" "53" "54" "55" "56" "57" "58" "59"
"60" "61" "62" "63" "64" "65" "66" "67" "68" "69"
"70" "71" "72" "73" "74" "75" "76" "77" "78" "79"
"80" "81" "82" "83" "84" "85" "86" "87" "88" "89"
"90" "91" "92" "93" "94" "95" "96" "97" "98" "99";

/*
 * Adapted from http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
 */
static inline int
decimalLength32(const uint32 v)
{
  int      t;
  static const uint32 PowersOfTen[] = {
    1, 10, 100,
    1000, 10000, 100000,
    1000000, 10000000, 100000000,
    1000000000
  };

  /*
   * Compute base-10 logarithm by dividing the base-2 logarithm by a
   * good-enough approximation of the base-2 logarithm of 10
   */
  t = (pg_leftmost_one_pos32(v) + 1) * 1233 / 4096;
  return t + (v >= PowersOfTen[t]);
}

static inline int
decimalLength64(const uint64 v)
{
  int      t;
  static const uint64 PowersOfTen[] = {
    UINT64CONST(1), UINT64CONST(10),
    UINT64CONST(100), UINT64CONST(1000),
    UINT64CONST(10000), UINT64CONST(100000),
    UINT64CONST(1000000), UINT64CONST(10000000),
    UINT64CONST(100000000), UINT64CONST(1000000000),
    UINT64CONST(10000000000), UINT64CONST(100000000000),
    UINT64CONST(1000000000000), UINT64CONST(10000000000000),
    UINT64CONST(100000000000000), UINT64CONST(1000000000000000),
    UINT64CONST(10000000000000000), UINT64CONST(100000000000000000),
    UINT64CONST(1000000000000000000), UINT64CONST(10000000000000000000)
  };

  /*
   * Compute base-10 logarithm by dividing the base-2 logarithm by a
   * good-enough approximation of the base-2 logarithm of 10
   */
  t = (pg_leftmost_one_pos64(v) + 1) * 1233 / 4096;
  return t + (v >= PowersOfTen[t]);
}

/*
 * Convert input string to a signed 32 bit integer.
 *
 * Allows any number of leading or trailing whitespace characters. Will throw
 * ereport() upon bad input format or overflow.
 *
 * NB: Accumulate input as a negative number, to deal with two's complement
 * representation of the most negative number, which can't be represented as a
 * positive number.
 */
static int32
pg_strtoint32(const char *s)
{
  const char *ptr = s;
  int32    tmp = 0;
  bool    neg = false;

  /* skip leading spaces */
  while (likely(*ptr) && isspace((unsigned char) *ptr))
    ptr++;

  /* handle sign */
  if (*ptr == '-')
  {
    ptr++;
    neg = true;
  }
  else if (*ptr == '+')
    ptr++;

  /* require at least one digit */
  if (unlikely(!isdigit((unsigned char) *ptr)))
    goto invalid_syntax;

  /* process digits */
  while (*ptr && isdigit((unsigned char) *ptr))
  {
    int8    digit = (*ptr++ - '0');

    if (unlikely(pg_mul_s32_overflow(tmp, 10, &tmp)) ||
      unlikely(pg_sub_s32_overflow(tmp, digit, &tmp)))
      goto out_of_range;
  }

  /* allow trailing whitespace, but not other trailing chars */
  while (*ptr != '\0' && isspace((unsigned char) *ptr))
    ptr++;

  if (unlikely(*ptr != '\0'))
    goto invalid_syntax;

  if (!neg)
  {
    /* could fail if input is most negative number */
    if (unlikely(tmp == PG_INT32_MIN))
      goto out_of_range;
    tmp = -tmp;
  }

  return tmp;

out_of_range:
  elog(ERROR, "value \"%s\" is out of range for type %s", s, "integer");

invalid_syntax:
  elog(ERROR, "invalid input syntax for type %s: \"%s\"", "integer", s);

  return 0;          /* keep compiler quiet */
}

/*
 * pg_ultoa_n: converts an unsigned 32-bit integer to its string representation,
 * not NUL-terminated, and returns the length of that string representation
 *
 * Caller must ensure that 'a' points to enough memory to hold the result (at
 * least 10 bytes)
 */
static int
pg_ultoa_n(uint32 value, char *a)
{
  int      olength,
        i = 0;

  /* Degenerate case */
  if (value == 0)
  {
    *a = '0';
    return 1;
  }

  olength = decimalLength32(value);

  /* Compute the result string. */
  while (value >= 10000)
  {
    const uint32 c = value - 10000 * (value / 10000);
    const uint32 c0 = (c % 100) << 1;
    const uint32 c1 = (c / 100) << 1;

    char     *pos = a + olength - i;

    value /= 10000;

    memcpy(pos - 2, DIGIT_TABLE + c0, 2);
    memcpy(pos - 4, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (value >= 100)
  {
    const uint32 c = (value % 100) << 1;

    char     *pos = a + olength - i;

    value /= 100;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (value >= 10)
  {
    const uint32 c = value << 1;

    char     *pos = a + olength - i;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
  }
  else
  {
    *a = (char) ('0' + value);
  }

  return olength;
}

/*
 * pg_ltoa: converts a signed 32-bit integer to its string representation and
 * returns strlen(a).
 *
 * It is the caller's responsibility to ensure that a is at least 12 bytes long,
 * which is enough room to hold a minus sign, a maximally long int32, and the
 * above terminating NUL.
 */
int
pg_ltoa(int32 value, char *a)
{
  uint32    uvalue = (uint32) value;
  int      len = 0;

  if (value < 0)
  {
    uvalue = (uint32) 0 - uvalue;
    a[len++] = '-';
  }
  len += pg_ultoa_n(uvalue, a + len);
  a[len] = '\0';
  return len;
}

/*
 * pg_ultostr_zeropad
 *    Converts 'value' into a decimal string representation stored at 'str'.
 *    'minwidth' specifies the minimum width of the result; any extra space
 *    is filled up by prefixing the number with zeros.
 *    MobilityDB: Function copied from numutils.c
 *
 * Returns the ending address of the string result (the last character written
 * plus 1).  Note that no NUL terminator is written.
 *
 * The intended use-case for this function is to build strings that contain
 * multiple individual numbers, for example:
 *
 *  str = pg_ultostr_zeropad(str, hours, 2);
 *  *str++ = ':';
 *  str = pg_ultostr_zeropad(str, mins, 2);
 *  *str++ = ':';
 *  str = pg_ultostr_zeropad(str, secs, 2);
 *  *str = '\0';
 *
 * Note: Caller must ensure that 'str' points to enough memory to hold the
 * result.
 */
char *
pg_ultostr_zeropad(char *str, uint32 value, int32 minwidth)
{
  int      len;

  Assert(minwidth > 0);

  if (value < 100 && minwidth == 2)  /* Short cut for common case */
  {
    memcpy(str, DIGIT_TABLE + value * 2, 2);
    return str + 2;
  }

  len = pg_ultoa_n(value, str);
  if (len >= minwidth)
    return str + len;

  memmove(str + minwidth - len, str, len);
  memset(str, '0', minwidth - len);
  return str + minwidth;
}

/*
 * pg_ultostr
 *    Converts 'value' into a decimal string representation stored at 'str'.
 *    MobilityDB: Function copied from numutils.c
 *
 * Returns the ending address of the string result (the last character written
 * plus 1).  Note that no NUL terminator is written.
 *
 * The intended use-case for this function is to build strings that contain
 * multiple individual numbers, for example:
 *
 *  str = pg_ultostr(str, a);
 *  *str++ = ' ';
 *  str = pg_ultostr(str, b);
 *  *str = '\0';
 *
 * Note: Caller must ensure that 'str' points to enough memory to hold the
 * result.
 */
char *
pg_ultostr(char *str, uint32 value)
{
  int      len = pg_ultoa_n(value, str);

  return str + len;
}

/*****************************************************************************
 * Functions adapted from int.c
 *****************************************************************************/

/**
 * @brief Return an int4 from a string
 * @note PostgreSQL function: Datum int4in(PG_FUNCTION_ARGS)
 */
int32
int4_in(char *str)
{
  return pg_strtoint32(str);
}

/**
 * @brief Return a string from an int4
 * @note PostgreSQL function: Datum int4out(PG_FUNCTION_ARGS)
 */
char *
int4_out(int32 val)
{
  char *result = palloc(12);  /* sign, 10 digits, '\0' */
  pg_ltoa(val, result);
  return result;
}

/*****************************************************************************
 * Functions adapted from int8.c
 *****************************************************************************/

/* Sign + the most decimal digits an 8-byte number could have */
#define MAXINT8LEN 20

/*
 * scanint8 --- try to parse a string into an int8.
 *
 * If errorOK is false, ereport a useful error message if the string is bad.
 * If errorOK is true, just return "false" for bad input.
 */
bool
scanint8(const char *str, bool errorOK, int64 *result)
{
	const char *ptr = str;
	int64		tmp = 0;
	bool		neg = false;

	/*
	 * Do our own scan, rather than relying on sscanf which might be broken
	 * for long long.
	 *
	 * As INT64_MIN can't be stored as a positive 64 bit integer, accumulate
	 * value as a negative number.
	 */

	/* skip leading spaces */
	while (*ptr && isspace((unsigned char) *ptr))
		ptr++;

	/* handle sign */
	if (*ptr == '-')
	{
		ptr++;
		neg = true;
	}
	else if (*ptr == '+')
		ptr++;

	/* require at least one digit */
	if (unlikely(!isdigit((unsigned char) *ptr)))
		goto invalid_syntax;

	/* process digits */
	while (*ptr && isdigit((unsigned char) *ptr))
	{
		int8		digit = (*ptr++ - '0');

		if (unlikely(pg_mul_s64_overflow(tmp, 10, &tmp)) ||
			unlikely(pg_sub_s64_overflow(tmp, digit, &tmp)))
			goto out_of_range;
	}

	/* allow trailing whitespace, but not other trailing chars */
	while (*ptr != '\0' && isspace((unsigned char) *ptr))
		ptr++;

	if (unlikely(*ptr != '\0'))
		goto invalid_syntax;

	if (!neg)
	{
		/* could fail if input is most negative number */
		if (unlikely(tmp == PG_INT64_MIN))
			goto out_of_range;
		tmp = -tmp;
	}

	*result = tmp;
	return true;

out_of_range:
	if (!errorOK)
		elog(ERROR, "value \"%s\" is out of range for type %s", str, "bigint");
	return false;

invalid_syntax:
	if (!errorOK)
		elog(ERROR, "invalid input syntax for type %s: \"%s\"", "bigint", str);
	return false;
}

/**
 * @brief Return an int8 from a string
 * @note PostgreSQL function: Datum int8in(PG_FUNCTION_ARGS)
 */
int64
int8_in(char *str)
{
#if POSTGRESQL_VERSION_NUMBER >= 150000
  int64 result = pg_strtoint64(str);
#else
  int64 result;
  (void) scanint8(str, false, &result);
#endif
  return result;
}

/*
 * Get the decimal representation, not NUL-terminated, and return the length of
 * same.  Caller must ensure that a points to at least MAXINT8LEN bytes.
 */
static int
pg_ulltoa_n(uint64 value, char *a)
{
  int      olength,
        i = 0;
  uint32    value2;

  /* Degenerate case */
  if (value == 0)
  {
    *a = '0';
    return 1;
  }

  olength = decimalLength64(value);

  /* Compute the result string. */
  while (value >= 100000000)
  {
    const uint64 q = value / 100000000;
    uint32    value2 = (uint32) (value - 100000000 * q);

    const uint32 c = value2 % 10000;
    const uint32 d = value2 / 10000;
    const uint32 c0 = (c % 100) << 1;
    const uint32 c1 = (c / 100) << 1;
    const uint32 d0 = (d % 100) << 1;
    const uint32 d1 = (d / 100) << 1;

    char     *pos = a + olength - i;

    value = q;

    memcpy(pos - 2, DIGIT_TABLE + c0, 2);
    memcpy(pos - 4, DIGIT_TABLE + c1, 2);
    memcpy(pos - 6, DIGIT_TABLE + d0, 2);
    memcpy(pos - 8, DIGIT_TABLE + d1, 2);
    i += 8;
  }

  /* Switch to 32-bit for speed */
  value2 = (uint32) value;

  if (value2 >= 10000)
  {
    const uint32 c = value2 - 10000 * (value2 / 10000);
    const uint32 c0 = (c % 100) << 1;
    const uint32 c1 = (c / 100) << 1;

    char     *pos = a + olength - i;

    value2 /= 10000;

    memcpy(pos - 2, DIGIT_TABLE + c0, 2);
    memcpy(pos - 4, DIGIT_TABLE + c1, 2);
    i += 4;
  }
  if (value2 >= 100)
  {
    const uint32 c = (value2 % 100) << 1;
    char     *pos = a + olength - i;

    value2 /= 100;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
    i += 2;
  }
  if (value2 >= 10)
  {
    const uint32 c = value2 << 1;
    char     *pos = a + olength - i;

    memcpy(pos - 2, DIGIT_TABLE + c, 2);
  }
  else
    *a = (char) ('0' + value2);

  return olength;
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
pg_lltoa(int64 value, char *a)
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
int8_out(int64 val)
{
  char buf[MAXINT8LEN + 1];
  char *result;
  int len;

  len = pg_lltoa(val, buf) + 1;
  /*
   * Since the length is already known, we do a manual palloc() and memcpy()
   * to avoid the strlen() call that would otherwise be done in pstrdup().
   */
  result = palloc(len);
  memcpy(result, buf, len);
  return result;
}

/*****************************************************************************
 * Functions adapted from float.c
 *****************************************************************************/

/*
 * float8in_internal_opt_error - guts of float8in()
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
	double		val;
	char	   *endptr;

	/* skip leading whitespace */
	while (*num != '\0' && isspace((unsigned char) *num))
		num++;

	/*
	 * Check for an empty-string input to begin with, to avoid the vagaries of
	 * strtod() on different platforms.
	 */
	if (*num == '\0')
		elog(ERROR, "invalid input syntax for type %s: \"%s\"", type_name,
      orig_string);

	errno = 0;
	val = strtod(num, &endptr);

	/* did we not see anything that looks like a double? */
	if (endptr == num || errno != 0)
	{
		int			save_errno = errno;

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
				char	   *errnumber = strdup(num);
				errnumber[endptr - num] = '\0';
				elog(ERROR, "\"%s\" is out of range for type double precision",
					errnumber);
				pfree(errnumber);
			}
		}
		else
			elog(ERROR, "invalid input syntax for type %s: \"%s\"",
				type_name, orig_string);
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
float8_in(char *num, const char *type_name, const char *orig_string)
{
  double result = float8_in_opt_error(num, type_name, orig_string);
	return result;
}

/* Definition taken from GCC's float.h */
#define DBL_DIG                __DBL_DIG__

/*
 * float8out_internal - guts of float8out()
 *
 * This is exposed for use by functions that want a reasonably
 * platform-independent way of outputting doubles.
 * The result is always palloc'd.
 */
char *
float8_out(double num)
{
	char	   *ascii = palloc(32);
	int			ndig = DBL_DIG + extra_float_digits;

	if (extra_float_digits > 0)
	{
		double_to_shortest_decimal_buf(num, ascii);
		return ascii;
	}

	(void) pg_strfromd(ascii, 32, ndig, num);
	return ascii;
}

/*****************************************************************************
 * Functions adapted from bool.c
 *****************************************************************************/




/*****************************************************************************/
