/*-------------------------------------------------------------------------
 *
 * float.c
 *    Functions for the built-in floating-point types.
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/float.c
 *
 *-------------------------------------------------------------------------
 */

/* C */
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>
/* PostgreSQL */
#include "postgres.h"
#include "common/int.h"
#include "common/shortest_dec.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/numeric.h"
#include "utils/jsonb.h"
#include "utils/float.h"

#include "pgtypes.h"

/*****************************************************************************
 * Definitions taken from the file liblwgeom_internal.h
 *****************************************************************************/

/* Any (absolute) values outside this range will be printed in scientific
 * notation */
#define OUT_MIN_DOUBLE 1E-8
#define OUT_MAX_DOUBLE 1E15
#define OUT_DEFAULT_DECIMAL_DIGITS 15

/* 17 digits are sufficient for round-tripping
 * Then we might add up to 8 (from OUT_MIN_DOUBLE) max leading zeroes (or
 * 2 digits for "e+") */
#define OUT_MAX_DIGITS 17 + 8

/* Limit for the max amount of characters that a double can use, including dot
 * and sign */
#define OUT_MAX_BYTES_DOUBLE (1 /* Sign */ + 2 /* 0.x */ + OUT_MAX_DIGITS)
#define OUT_DOUBLE_BUFFER_SIZE OUT_MAX_BYTES_DOUBLE + 1 /* +1 including NULL */

extern int lwprint_double(double d, int maxdd, char *buf);

/*****************************************************************************/

/*
 * Configurable GUC parameter
 *
 * If >0, use shortest-decimal format for output; this is both the default and
 * allows for compatibility with clients that explicitly set a value here to
 * get round-trip-accurate results. If 0 or less, then use the old, slow,
 * decimal rounding method.
 */
int extra_float_digits = 1;

/* Cached constants for degree-based trig functions */
static bool degree_consts_set = false;
static float8 sin_30 = 0;
static float8 one_minus_cos_60 = 0;
static float8 asin_0_5 = 0;
static float8 acos_0_5 = 0;
static float8 atan_1_0 = 0;
static float8 tan_45 = 0;
static float8 cot_45 = 0;

/*
 * These are intentionally not static; don't "fix" them.  They will never
 * be referenced by other files, much less changed; but we don't want the
 * compiler to know that, else it might try to precompute expressions
 * involving them.  See comments for init_degree_constants().
 *
 * The additional extern declarations are to silence
 * -Wmissing-variable-declarations.
 */
extern float8 degree_c_thirty;
extern float8 degree_c_forty_five;
extern float8 degree_c_sixty;
extern float8 degree_c_one_half;
extern float8 degree_c_one;
float8 degree_c_thirty = 30.0;
float8 degree_c_forty_five = 45.0;
float8 degree_c_sixty = 60.0;
float8 degree_c_one_half = 0.5;
float8 degree_c_one = 1.0;

/* Local function prototypes */
static double sind_q1(double x);
static double cosd_q1(double x);
static void init_degree_constants(void);

/*
 * Return -1 if 'val' represents negative infinity, 1 if 'val'
 * represents (positive) infinity, and 0 otherwise. On some platforms,
 * this is equivalent to the isinf() macro, but not everywhere: C99
 * does not specify that isinf() needs to distinguish between positive
 * and negative infinity.
 */
int
is_infinite(double val)
{
  int inf = isinf(val);
  if (inf == 0)
    return 0;
  else if (val > 0)
    return 1;
  else
    return -1;
}

/* ========== USER I/O ROUTINES ========== */


/*
 *    float4in    - Convert "num" to float4
 *
 * Note that this code now uses strtof(), where it used to use strtod().
 *
 * The motivation for using strtof() is to avoid a double-rounding problem:
 * for certain decimal inputs, if you round the input correctly to a double,
 * and then round the double to a float, the result is incorrect in that it
 * does not match the result of rounding the decimal value to float directly.
 *
 * One of the best examples is 7.038531e-26:
 *
 * 0xAE43FDp-107 = 7.03853069185120912085...e-26
 *      midpoint   7.03853100000000022281...e-26
 * 0xAE43FEp-107 = 7.03853130814879132477...e-26
 *
 * making 0xAE43FDp-107 the correct float result, but if you do the conversion
 * via a double, you get
 *
 * 0xAE43FD.7FFFFFF8p-107 = 7.03853099999999907487...e-26
 *               midpoint   7.03853099999999964884...e-26
 * 0xAE43FD.80000000p-107 = 7.03853100000000022281...e-26
 * 0xAE43FD.80000008p-107 = 7.03853100000000137076...e-26
 *
 * so the value rounds to the double exactly on the midpoint between the two
 * nearest floats, and then rounding again to a float gives the incorrect
 * result of 0xAE43FEp-107.
 *
 */

/*
 * float4inf - guts of float4in()
 *
 * This is exposed for use by functions that want a reasonably
 * platform-independent way of inputting floats. The behavior is
 * essentially like strtof + ereturn on error.
 *
 * Uses the same API as float8in_internal below, so most of its
 * comments also apply here, except regarding use in geometric types.
 */
static float4
pg_float4in_internal(char *num, char **endptr_p, const char *type_name,
  const char *orig_string)
{
  float val;
  char *endptr;

  /*
   * endptr points to the first character _after_ the sequence we recognized
   * as a valid floating point number. orig_string points to the original
   * input string.
   */

  /* skip leading whitespace */
  while (*num != '\0' && isspace((unsigned char) *num))
    num++;

  /*
   * Check for an empty-string input to begin with, to avoid the vagaries of
   * strtod() on different platforms.
   */
  if (*num == '\0')
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid input syntax for type %s: \"%s\"",
      type_name, orig_string);
    return FLT_MAX;
  }

  errno = 0;
  val = strtof(num, &endptr);

  /* did we not see anything that looks like a double? */
  if (endptr == num || errno != 0)
  {
    int save_errno = errno;

    /*
     * C99 requires that strtof() accept NaN, [+-]Infinity, and [+-]Inf,
     * but not all platforms support all of these (and some accept them
     * but set ERANGE anyway...)  Therefore, we check for these inputs
     * ourselves if strtof() fails.
     *
     * Note: C99 also requires hexadecimal input as well as some extended
     * forms of NaN, but we consider these forms unportable and don't try
     * to support them.  You can use 'em if your strtof() takes 'em.
     */
    if (pg_strncasecmp(num, "NaN", 3) == 0)
    {
      val = get_float4_nan();
      endptr = num + 3;
    }
    else if (pg_strncasecmp(num, "Infinity", 8) == 0)
    {
      val = get_float4_infinity();
      endptr = num + 8;
    }
    else if (pg_strncasecmp(num, "+Infinity", 9) == 0)
    {
      val = get_float4_infinity();
      endptr = num + 9;
    }
    else if (pg_strncasecmp(num, "-Infinity", 9) == 0)
    {
      val = -get_float4_infinity();
      endptr = num + 9;
    }
    else if (pg_strncasecmp(num, "inf", 3) == 0)
    {
      val = get_float4_infinity();
      endptr = num + 3;
    }
    else if (pg_strncasecmp(num, "+inf", 4) == 0)
    {
      val = get_float4_infinity();
      endptr = num + 4;
    }
    else if (pg_strncasecmp(num, "-inf", 4) == 0)
    {
      val = -get_float4_infinity();
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
       */
      if (val == 0.0 ||
#if !defined(HUGE_VALF)
        isinf(val)
#else
        (val >= HUGE_VALF || val <= -HUGE_VALF)
#endif
        )
      {
        /* see comments in float8in_internal for rationale */
        char *errnumber = pstrdup(num);
        errnumber[endptr - num] = '\0';
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "\"%s\" is out of range for type real", errnumber);
        return FLT_MAX;
      }
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "invalid input syntax for type %s: \"%s\"",
        type_name, orig_string);
      return FLT_MAX;
    }
  }

  /* skip trailing whitespace */
  while (*endptr != '\0' && isspace((unsigned char) *endptr))
    endptr++;

  /* report stopping point if wanted, else complain if not end of string */
  if (endptr_p)
    *endptr_p = endptr;
  else if (*endptr != '\0')
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid input syntax for type %s: \"%s\"",
      type_name, orig_string);
    return FLT_MAX;
  }
  return val;
}

/**
 * @ingroup meos_base_float
 * @brief Return a float4 number from its string representation
 * @note Derived from PostgreSQL function @p float4in()
 */
float4
float4_in(const char *num)
{
  return pg_float4in_internal((char *) num, NULL, "real", num);
}

/**
 * @ingroup meos_base_float
 * @brief Return the string representation of a float4 number
 * @note Derived from PostgreSQL function @p float4out()
 */
char *
float4_out(float4 num)
{
  char *ascii = (char *) palloc(32);
  int ndig = FLT_DIG + extra_float_digits;

  if (extra_float_digits > 0)
  {
    float_to_shortest_decimal_buf(num, ascii);
    return ascii;
  }

  (void) pg_strfromd(ascii, 32, ndig, num);
  return ascii;
}

/*
 * float8in_internal - guts of float8in()
 *
 * This is exposed for use by functions that want a reasonably
 * platform-independent way of inputting doubles.  The behavior is
 * essentially like strtod + ereturn on error, but note the following
 * differences:
 * 1. Both leading and trailing whitespace are skipped.
 * 2. If endptr_p is NULL, we report error if there's trailing junk.
 * Otherwise, it's up to the caller to complain about trailing junk.
 * 3. In event of a syntax error, the report mentions the given type_name
 * and prints orig_string as the input; this is meant to support use of
 * this function with types such as "box" and "point", where what we are
 * parsing here is just a substring of orig_string.
 *
 * "num" could validly be declared "const char *", but that results in an
 * unreasonable amount of extra casting both here and in callers, so we don't.
 */
static float8
pg_float8in_internal(char *num, char **endptr_p, const char *type_name,
  const char *orig_string)
{
  float8 val;
  char *endptr;

  /* skip leading whitespace */
  while (*num != '\0' && isspace((unsigned char) *num))
    num++;

  /*
   * Check for an empty-string input to begin with, to avoid the vagaries of
   * strtod() on different platforms.
   */
  if (*num == '\0')
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid input syntax for type %s: \"%s\"",
      type_name, orig_string);
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
        char     *errnumber = pstrdup(num);

        errnumber[endptr - num] = '\0';
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "\"%s\" is out of range for type double precision",
          errnumber);
        return DBL_MAX;
      }
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "invalid input syntax for type %s: \"%s\"",
        type_name, orig_string);
      return DBL_MAX;
    }
  }

  /* skip trailing whitespace */
  while (*endptr != '\0' && isspace((unsigned char) *endptr))
    endptr++;

  /* report stopping point if wanted, else complain if not end of string */
  if (endptr_p)
    *endptr_p = endptr;
  else if (*endptr != '\0')
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid input syntax for type %s: \"%s\"",
      type_name, orig_string);
    return DBL_MAX;
  }

  return val;
}

/**
 * @ingroup meos_base_float
 * @brief Return a float8 number from its string representation
 * @return On error return `DBL_MAX`
 * @note Derived from PostgreSQL function @p float8in()
 */
float8
float8_in(const char *str)
{
  return pg_float8in_internal((char *) str, NULL, "double precision", str);
}

/*
 * float8out_internal - guts of float8out()
 *
 * This is exposed for use by functions that want a reasonably
 * platform-independent way of outputting doubles.
 * The result is always palloc'd.
 */
char *
float8out_internal(double num)
{
  char *ascii = (char *) palloc(32);
  int ndig = DBL_DIG + extra_float_digits;

  if (extra_float_digits > 0)
  {
    double_to_shortest_decimal_buf(num, ascii);
    return ascii;
  }

  (void) pg_strfromd(ascii, 32, ndig, num);
  return ascii;
}

/**
 * @ingroup meos_base_float
 * @brief Return the string representation of a float8 number
 * @details This function uses the PostGIS function lwprint_double to print an
 * ordinate value using at most **maxdd** number of decimal digits. The actual 
 * number of printed decimal digits may be less than the requested ones if out 
 * of significant digits.
 *
 * The function will write at most OUT_DOUBLE_BUFFER_SIZE bytes, including the
 * terminating NULL.
 */
char *
float8_out(float8 num, int maxdd)
{
  assert(maxdd >= 0);
  char *ascii = palloc(OUT_DOUBLE_BUFFER_SIZE);
  lwprint_double(num, maxdd, ascii);
  return ascii;
}

/* ========== PUBLIC ROUTINES ========== */

/*
 *    ======================
 *    FLOAT4 BASE OPERATIONS
 *    ======================
 */

/**
 * @ingroup meos_base_float
 * @brief Return the absolute value of a float4 number
 * @note Derived from PostgreSQL function @p float8abs()
 */
float4
float4_abs(float4 num)
{
  return fabsf(num);
}

/**
 * @ingroup meos_base_float
 * @brief Return the unary minus of a float4 number
 * @note Derived from PostgreSQL function @p float8um()
 */
float4
float4_um(float4 num)
{
  return -num;
}

/**
 * @ingroup meos_base_float
 * @brief Return the unary plus of a float4 number
 * @note Derived from PostgreSQL function @p float8up()
 */
float4
float4_up(float4 num)
{
  return num;
}

/**
 * @ingroup meos_base_float
 * @brief Return the larger of two float4 numbers
 * @note Derived from PostgreSQL function @p float8larger()
 */
float4
float4_larger(float4 num1, float4 num2)
{
  float4 result;
  if (float4_gt(num1, num2))
    result = num1;
  else
    result = num2;
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the smaller of two float4 numbers
 * @note Derived from PostgreSQL function @p float8smaller()
 */
float4
float4_smaller(float4 num1, float4 num2)
{
  float4 result;
  if (float4_lt(num1, num2))
    result = num1;
  else
    result = num2;
  return result;
}

/*
 *    ======================
 *    FLOAT8 BASE OPERATIONS
 *    ======================
 */

/**
 * @ingroup meos_base_float
 * @brief Return the absolute value of a float8 number
 * @note Derived from PostgreSQL function @p float8abs()
 */
float8
float8_abs(float8 num)
{
  return fabs(num);
}

/**
 * @ingroup meos_base_float
 * @brief Return the unary minus of a float8 number
 * @note Derived from PostgreSQL function @p float8um()
 */
float8
float8_um(float8 num)
{
  return -num;
}

/**
 * @ingroup meos_base_float
 * @brief Return the unary plus a float8 number
 * @note Derived from PostgreSQL function @p float8up()
 */
float8
float8_up(float8 num)
{
  return num;
}

/**
 * @ingroup meos_base_float
 * @brief Return the larger of two float8 numbers
 * @note Derived from PostgreSQL function @p float8larger()
 */
float8
float8_larger(float8 num1, float8 num2)
{
  float8 result;
  if (float8_gt(num1, num2))
    result = num1;
  else
    result = num2;
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the smaller of two float8 numbers
 * @note Derived from PostgreSQL function @p float8smaller()
 */
float8
float8_smaller(float8 num1, float8 num2)
{
  float8 result;
  if (float8_lt(num1, num2))
    result = num1;
  else
    result = num2;
  return result;
}

/*
 *    ====================
 *    ARITHMETIC OPERATORS
 *    ====================
 */

/*
 *    float4pl    - Return num1 + num2
 *    float4mi    - Return num1 - num2
 *    float4mul    - Return num1 * num2
 *    float4div    - Return num1 / num2
 */

/**
 * @ingroup meos_base_float
 * @brief Return the addition of two float4 numbers
 * @note Derived from PostgreSQL function @p float4pl()
 */
float4
add_float4_float4(float4 num1, float4 num2)
{
  return float4_pl(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the subtraction of two float4 numbers
 * @note Derived from PostgreSQL function @p float4mi()
 */
float4
minus_float4_float4(float4 num1, float4 num2)
{
  return float4_mi(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the multiplication of two float4 numbers
 * @note Derived from PostgreSQL function @p float4mul()
 */
float4
mul_float4_float4(float4 num1, float4 num2)
{
  return float4_mul(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the division of two float4 numbers
 * @note Derived from PostgreSQL function @p float4div()
 */
float4
div_float4_float4(float4 num1, float4 num2)
{
  return float4_div(num1, num2);
}

/*
 *    float8pl    - Return num1 + num2
 *    float8mi    - Return num1 - num2
 *    float8mul    - Return num1 * num2
 *    float8div    - Return num1 / num2
 */

/**
 * @ingroup meos_base_float
 * @brief Return the addition of two float8 numbers
 * @note Derived from PostgreSQL function @p float8pl()
 */
float8
add_float8_float8(float8 num1, float8 num2)
{
  return float8_pl(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the subtraction of two float8 numbers
 * @note Derived from PostgreSQL function @p float8mi()
 */
float8
minus_float8_float8(float8 num1, float8 num2)
{
  return float8_mi(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the multiplication of two float8 numbers
 * @note Derived from PostgreSQL function @p float8mul()
 */
float8
mul_float8_float8(float8 num1, float8 num2)
{
  return float8_mul(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the division of two float8 numbers
 * @note Derived from PostgreSQL function @p float8div()
 */
float8
div_float8_float8(float8 num1, float8 num2)
{
  return float8_div(num1, num2);
}

/*
 *    ====================
 *    COMPARISON OPERATORS
 *    ====================
 */

/*
 *    float4{eq,ne,lt,le,gt,ge}    - float4/float4 comparison operations
 */

/**
 * @ingroup meos_base_float
 * @brief Return -1, 0, or 1 depending on whether the first float4 number is
 * less than, equal, or greater than the second one
 * @note Derived from PostgreSQL function @p float4_cmp_internal
 */
#if MEOS
int
float4_cmp(float4 num1, float4 num2)
{
  return pg_float4_cmp(num1, num2);
}
#endif
int
pg_float4_cmp(float4 num1, float4 num2)
{
  if (float4_gt(num1, num2))
    return 1;
  if (float4_lt(num1, num2))
    return -1;
  return 0;
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is equal to another one
 * @note Derived from PostgreSQL function @p float4eq()
 */
bool
eq_float4_float4(float4 num1, float4 num2)
{
  return float4_eq(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is not equal to another one
 * @note Derived from PostgreSQL function @p float4ne()
 */
bool
ne_float4_float4(float4 num1, float4 num2)
{
  return float4_ne(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is less than another one
 * @note Derived from PostgreSQL function @p float4lt()
 */
bool
lt_float4_float4(float4 num1, float4 num2)
{
  return float4_lt(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is less than or equal to another one
 * @note Derived from PostgreSQL function @p float4le()
 */
bool
le_float4_float4(float4 num1, float4 num2)
{
  return float4_le(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is greater than another one
 * @note Derived from PostgreSQL function @p float4gt()
 */
bool
gt_float4_float4(float4 num1, float4 num2)
{
  return float4_gt(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is greater than or equal to another one
 * @note Derived from PostgreSQL function @p float4ge()
 */
bool
ge_float4_float4(float4 num1, float4 num2)
{
  return float4_ge(num1, num2);
}

/*
 *    float8{eq,ne,lt,le,gt,ge}    - float8/float8 comparison operations
 */

/**
 * @ingroup meos_base_float
 * @brief Return -1, 0, or 1 depending on whether the first float8 number is
 * less than, equal, or greater than the second one
 * @note Existing PostgreSQL function
 */
#if MEOS
int
float8_cmp(float8 num1, float8 num2)
{
  return pg_float8_cmp(num1, num2);
}
#endif
int
pg_float8_cmp(float8 num1, float8 num2)
{
  if (float8_gt(num1, num2))
    return 1;
  if (float8_lt(num1, num2))
    return -1;
  return 0;
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is equal to another one
 * @note Derived from PostgreSQL function @p float8eq()
 */
bool
eq_float8_float8(float8 num1, float8 num2)
{
  return float8_eq(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is not equal to another one
 * @note Derived from PostgreSQL function @p float8ne()
 */
bool
ne_float8_float8(float8 num1, float8 num2)
{
  return float8_ne(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is less than another one
 * @note Derived from PostgreSQL function @p float8lt()
 */
bool
lt_float8_float8(float8 num1, float8 num2)
{
  return float8_lt(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is less than or equal to another one
 * @note Derived from PostgreSQL function @p float8le()
 */
bool
le_float8_float8(float8 num1, float8 num2)
{
  return float8_le(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is greater than another one
 * @note Derived from PostgreSQL function @p float8gt()
 */
bool
gt_float8_float8(float8 num1, float8 num2)
{
  return float8_gt(num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is greater than or equal to another one
 * @note Derived from PostgreSQL function @p float8ge()
 */
bool
ge_float8_float8(float8 num1, float8 num2)
{
  return float8_ge(num1, num2);
}

/*
 *    ===================
 *    CONVERSION ROUTINES
 *    ===================
 */

/**
 * @ingroup meos_base_float
 * @brief Convert a float4 number into a float8 number
 * @note Derived from PostgreSQL function @p ftod()
 */
float8
float4_to_float8(float4 num)
{
  return (float8) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert a float8 number into a float4 number
 * @note Derived from PostgreSQL function @p dtof()
 */
float4
float8_to_float4(float8 num)
{
  float4 result = (float4) num;
  if (unlikely(isinf(result)) && !isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return FLT_MAX;
  }
  if (unlikely(result == 0.0f) && num != 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return FLT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Convert a float8 number into an int32 number
 * @note Derived from PostgreSQL function @p dtoi4()
 */
int32
float8_to_int32(float8 num)
{
  /*
   * Get rid of any fractional part in the input.  This is so we don't fail
   * on just-out-of-range values that would round into range.  Note
   * assumption that rint() will pass through a NaN or Inf unchanged.
   */
  num = rint(num);

  /* Range check */
  if (unlikely(isnan(num) || !FLOAT8_FITS_IN_INT32(num)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "integer out of range");
    return INT_MAX;
  }

  return (int32) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert a float8 number into an int16 number
 * @note Derived from PostgreSQL function @p dtoi2()
 */
int16
float8_to_int16(float8 num)
{
  /*
   * Get rid of any fractional part in the input.  This is so we don't fail
   * on just-out-of-range values that would round into range.  Note
   * assumption that rint() will pass through a NaN or Inf unchanged.
   */
  num = rint(num);

  /* Range check */
  if (unlikely(isnan(num) || !FLOAT8_FITS_IN_INT16(num)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "smallint out of range");
    return INT16_MAX;
  }

  return (int16) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert an int32 number into a float8 number
 * @note Derived from PostgreSQL function @p i4tod()
 */
float8
int32_to_float8(int32 num)
{
  return (float8) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert an int16 number into a float8 number
 * @note Derived from PostgreSQL function @p i2tod()
 */
float8
int16_to_float8(int16 num)
{
  return (float8) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert a float4 number into an int32 number
 * @note Derived from PostgreSQL function @p ftoi4()
 */
int32
float4_to_int32(float4 num)
{
  /*
   * Get rid of any fractional part in the input.  This is so we don't fail
   * on just-out-of-range values that would round into range.  Note
   * assumption that rint() will pass through a NaN or Inf unchanged.
   */
  num = rint(num);

  /* Range check */
  if (unlikely(isnan(num) || !FLOAT4_FITS_IN_INT32(num)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "integer out of range");
    return INT_MAX;
  }

  return (int32) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert a float4 number into an int16 number
 * @note Derived from PostgreSQL function @p ftoi2()
 */
int16
float4_to_int16(float4 num)
{
  /*
   * Get rid of any fractional part in the input.  This is so we don't fail
   * on just-out-of-range values that would round into range.  Note
   * assumption that rint() will pass through a NaN or Inf unchanged.
   */
  num = rint(num);

  /* Range check */
  if (unlikely(isnan(num) || !FLOAT4_FITS_IN_INT16(num)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "smallint out of range");
    return INT16_MAX;
  }

  return (int16) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert an int32 number into a float4 number
 * @note Derived from PostgreSQL function @p i4tof()
 */
float4
int32_to_float4(int32 num)
{
  return (float4) num;
}

/**
 * @ingroup meos_base_float
 * @brief Convert an int16 number into a float4 number
 * @note Derived from PostgreSQL function @p i2tof()
 */
float4
int16_to_float4(int16 num)
{
  return (float4) num;
}


/*
 *    =======================
 *    RANDOM FLOAT8 OPERATORS
 *    =======================
 */

/**
 * @ingroup meos_base_float
 * @brief Round a float8 number to the nearest integer
 * @note PostgreSQL function: @p dround()
 */
float8
float8_rint(float8 num)
{
  return rint(num);
}

/**
 * @ingroup meos_base_float
 * @brief Return a float number rounded to a given number of decimal places
 * @note MEOS function
 */
float8
float8_round(float8 num, int maxdd)
{
  assert(maxdd >= 0);
  float8 inf = get_float8_infinity();
  float8 result = num;
  if (num != -1 * inf && num != inf)
  {
    if (maxdd == 0)
      result = round(num);
    else
    {
      float8 power10 = pow(10.0, maxdd);
      result = round(num * power10) / power10;
    }
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the smallest integer greater than or equal to a float8 number
 * @note PostgreSQL function: @p dceil()
 */
float8
float8_ceil(float8 num)
{
  return ceil(num);
}

/**
 * @ingroup meos_base_float
 * @brief Return the largest integer lesser than or equal to a float8 number
 * @note PostgreSQL function: @p dfloor()
 */
float8
float8_floor(float8 num)
{
  return floor(num);
}

/**
 * @ingroup meos_base_float
 * @brief Return -1 if a float8 number is less than 0, 0 if it is equal to 0,
 * and 1 if it is greater than zero
 * @note PostgreSQL function: @p dsign()
 */
float8
float8_sign(float8 num)
{
  float8 result;
  if (num > 0)
    result = 1.0;
  else if (num < 0)
    result = -1.0;
  else
    result = 0.0;
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the truncation-towards-zero of a float8 number
 * @details If num1 >= 0 return the greatest integer less than or equal to num1,
 * if num1 < 0  return the least integer greater than or equal to num1
 * @note PostgreSQL function: @p dtrunc()
 */
float8
float8_trunc(float8 num)
{
  float8 result;
  if (num >= 0)
    result = floor(num);
  else
    result = -floor(-num);
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the square root of a float8 number
 * @note PostgreSQL function: @p dsqrt()
 */
float8
float8_sqrt(float8 num)
{
  if (num < 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "cannot take square root of a negative number");
    return DBL_MAX;
  }

  float8 result = sqrt(num);
  if (unlikely(isinf(result)) && !isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  if (unlikely(result == 0.0) && num != 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the cube root of a float8 number
 * @note PostgreSQL function: @p dcbrt()
 */
float8
float8_cbrt(float8 num)
{
  float8 result = cbrt(num);
  if (unlikely(isinf(result)) && !isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  if (unlikely(result == 0.0) && num != 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the first float8 number powered to the second one
 * @note PostgreSQL function: @p dpow()
 */
float8
float8_pow(float8 num1, float8 num2)
{
  /*
   * The POSIX spec says that NaN ^ 0 = 1, and 1 ^ NaN = 1, while all other
   * cases with NaN inputs yield NaN (with no error).  Many older platforms
   * get one or more of these cases wrong, so deal with them via explicit
   * logic rather than trusting pow(3).
   */
  if (isnan(num1))
  {
    if (isnan(num2) || num2 != 0.0)
      return get_float8_nan();
    return 1.0;
  }
  if (isnan(num2))
  {
    if (num1 != 1.0)
      return get_float8_nan();
    return 1.0;
  }

  /*
   * The SQL spec requires that we emit a particular SQLSTATE error code for
   * certain error conditions.  Specifically, we don't return a
   * divide-by-zero error code for 0 ^ -1.
   */
  if (num1 == 0 && num2 < 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "zero raised to a negative power is undefined");
    return DBL_MAX;
  }
  if (num1 < 0 && floor(num2) != num2)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "a negative number raised to a non-integer power yields a complex result");
    return DBL_MAX;
  }

  /*
   * We don't trust the platform's pow() to handle infinity cases per POSIX
   * spec either, so deal with those explicitly too.  It's easier to handle
   * infinite y first, so that it doesn't matter if x is also infinite.
   */
  float8 result;
  if (isinf(num2))
  {
    float8 absx = fabs(num1);
    if (absx == 1.0)
      result = 1.0;
    else if (num2 > 0.0)  /* y = +Inf */
    {
      if (absx > 1.0)
        result = num2;
      else
        result = 0.0;
    }
    else          /* y = -Inf */
    {
      if (absx > 1.0)
        result = 0.0;
      else
        result = -num2;
    }
  }
  else if (isinf(num1))
  {
    if (num2 == 0.0)
      result = 1.0;
    else if (num1 > 0.0)  /* x = +Inf */
    {
      if (num2 > 0.0)
        result = num1;
      else
        result = 0.0;
    }
    else          /* x = -Inf */
    {
      /*
       * Per POSIX, the sign of the result depends on whether y is an
       * odd integer.  Since x < 0, we already know from the previous
       * domain check that y is an integer.  It is odd if y/2 is not
       * also an integer.
       */
      float8    halfy = num2 / 2;  /* should be computed exactly */
      bool    yisoddinteger = (floor(halfy) != halfy);

      if (num2 > 0.0)
        result = yisoddinteger ? num1 : -num1;
      else
        result = yisoddinteger ? -0.0 : 0.0;
    }
  }
  else
  {
    /*
     * pow() sets errno on only some platforms, depending on whether it
     * follows _IEEE_, _POSIX_, _XOPEN_, or _SVID_, so we must check both
     * errno and invalid output values.  (We can't rely on just the
     * latter, either; some old platforms return a large-but-finite
     * HUGE_VAL when reporting overflow.)
     */
    errno = 0;
    result = pow(num1, num2);
    if (errno == EDOM || isnan(result))
    {
      /*
       * We handled all possible domain errors above, so this should be
       * impossible.  However, old glibc versions on x86 have a bug that
       * causes them to fail this way for abs(y) greater than 2^63:
       *
       * https://sourceware.org/bugzilla/show_bug.cgi?id=3866
       *
       * Hence, if we get here, assume y is finite but large (large
       * enough to be certainly even). The result should be 0 if x == 0,
       * 1.0 if abs(x) == 1.0, otherwise an overflow or underflow error.
       */
      if (num1 == 0.0)
        result = 0.0;  /* we already verified y is positive */
      else
      {
        float8 absx = fabs(num1);
        if (absx == 1.0)
          result = 1.0;
        else if (num2 >= 0.0 ? (absx > 1.0) : (absx < 1.0))
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "value out of range: overflow");
          return DBL_MAX;
        }
        else
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "value out of range: underflow");
          return DBL_MAX;
        }
      }
    }
    else if (errno == ERANGE)
    {
      if (result != 0.0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: overflow");
        return DBL_MAX;
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: underflow");
        return DBL_MAX;
     }
    }
    else
    {
      if (unlikely(isinf(result)))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: overflow");
        return DBL_MAX;
      }
      if (unlikely(result == 0.0) && num1 != 0.0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: underflow");
        return DBL_MAX;
      }
    }
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the exponential of a float8 number
 * @note PostgreSQL function: @p dexp()
 */
float8
float8_exp(float8 num)
{
  /*
   * Handle NaN and Inf cases explicitly.  This avoids needing to assume
   * that the platform's exp() conforms to POSIX for these cases, and it
   * removes some edge cases for the overflow checks below.
   */
  float8 result;
  if (isnan(num))
    result = num;
  else if (isinf(num))
  {
    /* Per POSIX, exp(-Inf) is 0 */
    result = (num > 0.0) ? num : 0;
  }
  else
  {
    /*
     * On some platforms, exp() will not set errno but just return Inf or
     * zero to report overflow/underflow; therefore, test both cases.
     */
    errno = 0;
    result = exp(num);
    if (unlikely(errno == ERANGE))
    {
      if (result != 0.0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: overflow");
        return DBL_MAX;
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: underflow");
        return DBL_MAX;
     }
    }
    else if (unlikely(isinf(result)))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: overflow");
        return DBL_MAX;
      }
    else if (unlikely(result == 0.0))
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "value out of range: underflow");
      return DBL_MAX;
    }
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the natural logarithm of a float8 number
 * @note PostgreSQL function: @p dlog1()
 */
float8
float8_ln(float8 num)
{
  /*
   * Emit particular SQLSTATE error codes for ln(). This is required by the
   * SQL standard.
   */
  if (num == 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "cannot take logarithm of zero");
    return DBL_MAX;
  }
  if (num < 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "cannot take logarithm of a negative number");
    return DBL_MAX;
  }

  float8 result = log(num);
  if (unlikely(isinf(result)) && !isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  if (unlikely(result == 0.0) && num != 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return DBL_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the base 10 logarithm of a float8 number
 * @note PostgreSQL function: @p dlog10()
 */
float8
float8_log10(float8 num)
{
  /*
   * Emit particular SQLSTATE error codes for log(). The SQL spec doesn't
   * define log(), but it does define ln(), so it makes sense to emit the
   * same error code for an analogous error condition.
   */
  if (num == 0.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "cannot take logarithm of zero");
    return DBL_MAX;
  }
  if (num < 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "cannot take logarithm of a negative number");
    return DBL_MAX;
  }

  float8 result = log10(num);
  if (unlikely(isinf(result)) && !isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  if (unlikely(result == 0.0) && num != 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arccos of a float8 number (radians)
 * @note PostgreSQL function: @p dacos()
 */
float8
float8_acos(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();
  /*
   * The principal branch of the inverse cosine function maps values in the
   * range [-1, 1] to values in the range [0, Pi], so we should reject any
   * inputs outside that range and the result will always be finite.
   */
  if (num < -1.0 || num > 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  float8 result = acos(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arcsin of a float8 number (radians)
 * @note PostgreSQL function: @p dasin()
 */
float8
float8_asin(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  /*
   * The principal branch of the inverse sine function maps values in the
   * range [-1, 1] to values in the range [-Pi/2, Pi/2], so we should reject
   * any inputs outside that range and the result will always be finite.
   */
  if (num < -1.0 || num > 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  float8 result = asin(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arctan of a double (radians)
 * @note PostgreSQL function: @p datan()
 */
float8
float8_atan(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  /*
   * The principal branch of the inverse tangent function maps all inputs to
   * values in the range [-Pi/2, Pi/2], so the result should always be
   * finite, even if the input is infinite.
   */
  float8 result = atan(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arctan of two float8 numbers (radians)
 * @note PostgreSQL function: @p datan2d()
 */
float8
float8_atan2(float8 num1, float8 num2)
{
  /* Per the POSIX spec, return NaN if either input is NaN */
  if (isnan(num1) || isnan(num2))
    return get_float8_nan();

  /*
   * atan2 maps all inputs to values in the range [-Pi, Pi], so the result
   * should always be finite, even if the inputs are infinite.
   */
  float8 result = atan2(num1, num2);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the cosine of a float8 number (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dcos()
 */
float8
float8_cos(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
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
  float8 result = cos(num);
  if (errno != 0 || isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the cotangent of a float8 number (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dcot()
 */
float8
float8_cot(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  /* Be sure to throw an error if the input is infinite --- see dcos() */
  errno = 0;
  float8 result = tan(num);
  if (errno != 0 || isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  result = 1.0 / result;
  /* Not checking for overflow because cot(0) == Inf */
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the sine of a float8 number (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dsin()
 */
float8
float8_sin(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  /* Be sure to throw an error if the input is infinite --- see dcos() */
  errno = 0;
  float8 result = sin(num);
  if (errno != 0 || isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the tangent of a float8 number (radians)
 * @return On error return @p DBL_MAX
 * @note PostgreSQL function: @p dtan()
 */
float8
float8_tan(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  /* Be sure to throw an error if the input is infinite --- see dcos() */
  errno = 0;
  float8 result = tan(num);
  if (errno != 0 || isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }
  /* Not checking for overflow because tan(pi/2) == Inf */
  return result;
}

/* ========== DEGREE-BASED TRIGONOMETRIC FUNCTIONS ========== */

/*
 * Initialize the cached constants declared at the head of this file
 * (sin_30 etc).  The fact that we need those at all, let alone need this
 * Rube-Goldberg-worthy method of initializing them, is because there are
 * compilers out there that will precompute expressions such as sin(constant)
 * using a sin() function different from what will be used at runtime.  If we
 * want exact results, we must ensure that none of the scaling constants used
 * in the degree-based trig functions are computed that way.  To do so, we
 * compute them from the variables degree_c_thirty etc, which are also really
 * constants, but the compiler cannot assume that.
 *
 * Other hazards we are trying to forestall with this kluge include the
 * possibility that compilers will rearrange the expressions, or compute
 * some intermediate results in registers wider than a standard double.
 *
 * In the places where we use these constants, the typical pattern is like
 *    volatile float8 sin_x = sin(x * RADIANS_PER_DEGREE);
 *    return (sin_x / sin_30);
 * where we hope to get a value of exactly 1.0 from the division when x = 30.
 * The volatile temporary variable is needed on machines with wide float
 * registers, to ensure that the result of sin(x) is rounded to double width
 * the same as the value of sin_30 has been.  Experimentation with gcc shows
 * that marking the temp variable volatile is necessary to make the store and
 * reload actually happen; hopefully the same trick works for other compilers.
 * (gcc's documentation suggests using the -ffloat-store compiler switch to
 * ensure this, but that is compiler-specific and it also pessimizes code in
 * many places where we don't care about this.)
 */
static void
init_degree_constants(void)
{
  sin_30 = sin(degree_c_thirty * RADIANS_PER_DEGREE);
  one_minus_cos_60 = 1.0 - cos(degree_c_sixty * RADIANS_PER_DEGREE);
  asin_0_5 = asin(degree_c_one_half);
  acos_0_5 = acos(degree_c_one_half);
  atan_1_0 = atan(degree_c_one);
  tan_45 = sind_q1(degree_c_forty_five) / cosd_q1(degree_c_forty_five);
  cot_45 = cosd_q1(degree_c_forty_five) / sind_q1(degree_c_forty_five);
  degree_consts_set = true;
}

#define INIT_DEGREE_CONSTANTS() \
do { \
  if (!degree_consts_set) \
    init_degree_constants(); \
} while(0)

/*
 *    asind_q1    - Return the inverse sine of x in degrees, for x in
 *              the range [0, 1].  The result is an angle in the
 *              first quadrant --- [0, 90] degrees.
 *
 *              For the 3 special case inputs (0, 0.5 and 1), this
 *              function will return exact values (0, 30 and 90
 *              degrees respectively).
 */
static double
asind_q1(double x)
{
  /*
   * Stitch together inverse sine and cosine functions for the ranges [0,
   * 0.5] and (0.5, 1].  Each expression below is guaranteed to return
   * exactly 30 for x=0.5, so the result is a continuous monotonic function
   * over the full range.
   */
  if (x <= 0.5)
  {
    volatile float8 asin_x = asin(x);
    return (asin_x / asin_0_5) * 30.0;
  }
  else
  {
    volatile float8 acos_x = acos(x);
    return 90.0 - (acos_x / acos_0_5) * 60.0;
  }
}

/*
 *    acosd_q1    - Return the inverse cosine of x in degrees, for x in
 *              the range [0, 1].  The result is an angle in the
 *              first quadrant --- [0, 90] degrees.
 *
 *              For the 3 special case inputs (0, 0.5 and 1), this
 *              function will return exact values (0, 60 and 90
 *              degrees respectively).
 */
static double
acosd_q1(double x)
{
  /*
   * Stitch together inverse sine and cosine functions for the ranges [0,
   * 0.5] and (0.5, 1].  Each expression below is guaranteed to return
   * exactly 60 for x=0.5, so the result is a continuous monotonic function
   * over the full range.
   */
  if (x <= 0.5)
  {
    volatile float8 asin_x = asin(x);
    return 90.0 - (asin_x / asin_0_5) * 30.0;
  }
  else
  {
    volatile float8 acos_x = acos(x);
    return (acos_x / acos_0_5) * 60.0;
  }
}

/**
 * @ingroup meos_base_float
 * @brief Return the arccos of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dacosd()
 */
float8
float8_acosd(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  INIT_DEGREE_CONSTANTS();

  /*
   * The principal branch of the inverse cosine function maps values in the
   * range [-1, 1] to values in the range [0, 180], so we should reject any
   * inputs outside that range and the result will always be finite.
   */
  if (num < -1.0 || num > 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  float8 result;
  if (num >= 0.0)
    result = acosd_q1(num);
  else
    result = 90.0 + asind_q1(-num);

  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arcsin of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dasind()
 */
float8
float8_asind(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  INIT_DEGREE_CONSTANTS();

  /*
   * The principal branch of the inverse sine function maps values in the
   * range [-1, 1] to values in the range [-90, 90], so we should reject any
   * inputs outside that range and the result will always be finite.
   */
  if (num < -1.0 || num > 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  float8 result;
  if (num >= 0.0)
    result = asind_q1(num);
  else
    result = -asind_q1(-num);

  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arctan of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p datand()
 */
float8
float8_atand(float8 num)
{
  /* Per the POSIX spec, return NaN if the input is NaN */
  if (isnan(num))
    return get_float8_nan();

  INIT_DEGREE_CONSTANTS();

  /*
   * The principal branch of the inverse tangent function maps all inputs to
   * values in the range [-90, 90], so the result should always be finite,
   * even if the input is infinite.  Additionally, we take care to ensure
   * than when num1 is 1, the result is exactly 45.
   */
  volatile float8 atan_num = atan(num);
  float8 result = (atan_num / atan_1_0) * 45.0;
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the arctan of two float8 numbers (degrees)
 * @note Derived from PostgreSQL function @p atan2d()
 */
float8
float8_atan2d(float8 num1, float8 num2)
{
  /* Per the POSIX spec, return NaN if either input is NaN */
  if (isnan(num1) || isnan(num2))
    return get_float8_nan();

  INIT_DEGREE_CONSTANTS();

  /*
   * atan2d maps all inputs to values in the range [-180, 180], so the
   * result should always be finite, even if the inputs are infinite.
   *
   * Note: this coding assumes that atan(1.0) is a suitable scaling constant
   * to get an exact result from atan2().  This might well fail on us at
   * some point, requiring us to decide exactly what inputs we think we're
   * going to guarantee an exact result for.
   */
  volatile float8 atan2_num1_num2 = atan2(num1, num2);
  float8 result = (atan2_num1_num2 / atan_1_0) * 45.0;
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/*
 *    sind_0_to_30  - Return the sine of an angle that lies between 0 and
 *              30 degrees.  This will return exactly 0 when x is 0,
 *              and exactly 0.5 when x is 30 degrees.
 */
static double
sind_0_to_30(double x)
{
  volatile float8 sin_x = sin(x * RADIANS_PER_DEGREE);
  return (sin_x / sin_30) / 2.0;
}

/*
 *    cosd_0_to_60  - Return the cosine of an angle that lies between 0
 *              and 60 degrees.  This will return exactly 1 when x
 *              is 0, and exactly 0.5 when x is 60 degrees.
 */
static double
cosd_0_to_60(double x)
{
  volatile float8 one_minus_cos_x = 1.0 - cos(x * RADIANS_PER_DEGREE);
  return 1.0 - (one_minus_cos_x / one_minus_cos_60) / 2.0;
}

/*
 *    sind_q1      - Return the sine of an angle in the first quadrant
 *              (0 to 90 degrees).
 */
static double
sind_q1(double x)
{
  /*
   * Stitch together the sine and cosine functions for the ranges [0, 30]
   * and (30, 90].  These guarantee to return exact answers at their
   * endpoints, so the overall result is a continuous monotonic function
   * that gives exact results when x = 0, 30 and 90 degrees.
   */
  if (x <= 30.0)
    return sind_0_to_30(x);
  else
    return cosd_0_to_60(90.0 - x);
}

/*
 *    cosd_q1      - Return the cosine of an angle in the first quadrant
 *              (0 to 90 degrees).
 */
static double
cosd_q1(double x)
{
  /*
   * Stitch together the sine and cosine functions for the ranges [0, 60]
   * and (60, 90].  These guarantee to return exact answers at their
   * endpoints, so the overall result is a continuous monotonic function
   * that gives exact results when x = 0, 60 and 90 degrees.
   */
  if (x <= 60.0)
    return cosd_0_to_60(x);
  else
    return sind_0_to_30(90.0 - x);
}

/**
 * @ingroup meos_base_float
 * @brief Return the cosine of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dcosd()
 */
float8
float8_cosd(float8 num)
{
  /*
   * Per the POSIX spec, return NaN if the input is NaN and throw an error
   * if the input is infinite.
   */
  if (isnan(num))
    return get_float8_nan();

  if (isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  INIT_DEGREE_CONSTANTS();

  /* Reduce the range of the input to [0,90] degrees */
  num = fmod(num, 360.0);

  if (num < 0.0)
  {
    /* cosd(-x) = cosd(x) */
    num = -num;
  }

  if (num > 180.0)
  {
    /* cosd(360-x) = cosd(x) */
    num = 360.0 - num;
  }

  int sign = 1;
  if (num > 90.0)
  {
    /* cosd(180-x) = -cosd(x) */
    num = 180.0 - num;
    sign = -sign;
  }

  float8 result = sign * cosd_q1(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the cotangent of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dcotd()
 */
float8
float8_cotd(float8 num)
{
  /*
   * Per the POSIX spec, return NaN if the input is NaN and throw an error
   * if the input is infinite.
   */
  if (isnan(num))
    return get_float8_nan();

  if (isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  INIT_DEGREE_CONSTANTS();

  /* Reduce the range of the input to [0,90] degrees */
  num = fmod(num, 360.0);

  int sign = 1;
  if (num < 0.0)
  {
    /* cotd(-x) = -cotd(x) */
    num = -num;
    sign = -sign;
  }

  if (num > 180.0)
  {
    /* cotd(360-x) = -cotd(x) */
    num = 360.0 - num;
    sign = -sign;
  }

  if (num > 90.0)
  {
    /* cotd(180-x) = -cotd(x) */
    num = 180.0 - num;
    sign = -sign;
  }

  volatile float8 cot_num = cosd_q1(num) / sind_q1(num);
  float8 result = sign * (cot_num / cot_45);

  /*
   * On some machines we get cotd(270) = minus zero, but this isn't always
   * true.  For portability, and because the user constituency for this
   * function probably doesn't want minus zero, force it to plain zero.
   */
  if (result == 0.0)
    result = 0.0;
  /* Not checking for overflow because cotd(0) == Inf */
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the sine of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dsind()
 */
float8
float8_sind(float8 num)
{
  /*
   * Per the POSIX spec, return NaN if the input is NaN and throw an error
   * if the input is infinite.
   */
  if (isnan(num))
    return get_float8_nan();
  if (isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  INIT_DEGREE_CONSTANTS();

  /* Reduce the range of the input to [0,90] degrees */
  num = fmod(num, 360.0);
  int sign = 1;
  if (num < 0.0)
  {
    /* sind(-x) = -sind(x) */
    num = -num;
    sign = -sign;
  }

  if (num > 180.0)
  {
    /* sind(360-x) = -sind(x) */
    num = 360.0 - num;
    sign = -sign;
  }

  if (num > 90.0)
  {
    /* sind(180-x) = sind(x) */
    num = 180.0 - num;
  }

  float8 result = sign * sind_q1(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the tangent of a float8 number (degrees)
 * @note Derived from PostgreSQL function @p dtand()
 */
float8
float8_tand(float8 num)
{
  /*
   * Per the POSIX spec, return NaN if the input is NaN and throw an error
   * if the input is infinite.
   */
  if (isnan(num))
    return get_float8_nan();
  if (isinf(num))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  INIT_DEGREE_CONSTANTS();

  /* Reduce the range of the input to [0,90] degrees */
  num = fmod(num, 360.0);
  int sign = 1;
  if (num < 0.0)
  {
    /* tand(-x) = -tand(x) */
    num = -num;
    sign = -sign;
  }
  if (num > 180.0)
  {
    /* tand(360-x) = -tand(x) */
    num = 360.0 - num;
    sign = -sign;
  }
  if (num > 90.0)
  {
    /* tand(180-x) = -tand(x) */
    num = 180.0 - num;
    sign = -sign;
  }

  volatile float8 tan_num = sind_q1(num) / cosd_q1(num);
  float8 result = sign * (tan_num / tan_45);
  /*
   * On some machines we get tand(180) = minus zero, but this isn't always
   * true.  For portability, and because the user constituency for this
   * function probably doesn't want minus zero, force it to plain zero.
   */
  if (result == 0.0)
    result = 0.0;
  /* Not checking for overflow because tand(90) == Inf */
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return degrees converted from radians
 * @note Derived from PostgreSQL function @p degrees()
 */
float8
float8_degrees(float8 num)
{
  return float8_div(num, RADIANS_PER_DEGREE);
}

/**
 * @ingroup meos_base_float
 * @brief Return the constant PI
 * @note Derived from PostgreSQL function @p dpi()
 */
float8
float8_pi(void)
{
  return M_PI;
}

/**
 * @ingroup meos_base_float
 * @brief Return radians converted from degrees
 * @note Derived from PostgreSQL function @p radians()
 */
float8
float8_radians(float8 num)
{
  return float8_mul(num, RADIANS_PER_DEGREE);
}

/* ========== HYPERBOLIC FUNCTIONS ========== */

/**
 * @ingroup meos_base_float
 * @brief Return the hyperbolic sine of a float8 number
 * @note Derived from PostgreSQL function @p dsinh()
 */
float8
float8_sinh(float8 num)
{
  errno = 0;
  float8 result = sinh(num);
  /*
   * if an ERANGE error occurs, it means there is an overflow.  For sinh,
   * the result should be either -infinity or infinity, depending on the
   * sign of num.
   */
  if (errno == ERANGE)
  {
    if (num < 0)
      result = -get_float8_infinity();
    else
      result = get_float8_infinity();
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the hyperbolic cosine of a float8 number
 * @note Derived from PostgreSQL function @p dcosh()
 */
float8
float8_cosh(float8 num)
{
  errno = 0;
  float8 result = cosh(num);
  /*
   * if an ERANGE error occurs, it means there is an overflow.  As cosh is
   * always positive, it always means the result is positive infinity.
   */
  if (errno == ERANGE)
    result = get_float8_infinity();
  if (unlikely(result == 0.0))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: underflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the hyperbolic tangent of a float8 number
 * @note Derived from PostgreSQL function @p dtanh()
 */
float8
float8_tanh(float8 num)
{
  /*
   * For tanh, we don't need an errno check because it never overflows.
   */
  float8  result = tanh(num);
  if (unlikely(isinf(result)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the inverse hyperbolic sine of a float8 number
 * @note Derived from PostgreSQL function @p dasinh()
 */
float8
float8_asinh(float8 num)
{
  /*
   * For asinh, we don't need an errno check because it never overflows.
   */
  float8 result = asinh(num);
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the inverse hyperbolic cosine of a float8 number
 * @note Derived from PostgreSQL function @p dacosh()
 */
float8
float8_acosh(float8 num)
{
  /*
   * acosh is only defined for inputs >= 1.0.  By checking this ourselves,
   * we need not worry about checking for an EDOM error, which is a good
   * thing because some implementations will report that for NaN. Otherwise,
   * no error is possible.
   */
  if (num < 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }
  float8 result = acosh(num);
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the inverse hyperbolic tangent of a float8 number
 * @note Derived from PostgreSQL function @p datanh()
 */
float8
float8_atanh(float8 num)
{
  /*
   * atanh is only defined for inputs between -1 and 1.  By checking this
   * ourselves, we need not worry about checking for an EDOM error, which is
   * a good thing because some implementations will report that for NaN.
   */
  if (num < -1.0 || num > 1.0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "input is out of range");
    return DBL_MAX;
  }

  /*
   * Also handle the infinity cases ourselves; this is helpful because old
   * glibc versions may produce the wrong errno for this.  All other inputs
   * cannot produce an error.
   */
  float8 result;
  if (num == -1.0)
    result = -get_float8_infinity();
  else if (num == 1.0)
    result = get_float8_infinity();
  else
    result = atanh(num);
  return result;
}

/* ========== GAMMA FUNCTIONS ========== */

/**
 * @ingroup meos_base_float
 * @brief Return the gamma function of a float8 number
 * @note Derived from PostgreSQL function @p dgamma()
 */
float8
float8_gamma(float8 num)
{
  /*
   * Handle NaN and Inf cases explicitly.  This simplifies the overflow
   * checks on platforms that do not set errno.
   */
  float8 result;
  if (isnan(num))
    result = num;
  else if (isinf(num))
  {
    /* Per POSIX, an input of -Inf causes a domain error */
    if (num < 0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "value out of range: overflow");
      return DBL_MAX;
    }
    else
      result = num;
  }
  else
  {
    /*
     * Note: the POSIX/C99 gamma function is called "tgamma", not "gamma".
     *
     * On some platforms, tgamma() will not set errno but just return Inf,
     * NaN, or zero to report overflow/underflow; therefore, test those
     * cases explicitly (note that, like the exponential function, the
     * gamma function has no zeros).
     */
    errno = 0;
    result = tgamma(num);
    if (errno != 0 || isinf(result) || isnan(result))
    {
      if (result != 0.0)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: overflow");
        return DBL_MAX;
      }
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "value out of range: underflow");
        return DBL_MAX;
      }
    }
    else if (result == 0.0)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "value out of range: underflow");
      return DBL_MAX;
    }
  }
  return result;
}

/**
 * @ingroup meos_base_float
 * @brief Return the natural logarithm of absolute value of gamma of a float8
 * number
 * @note Derived from PostgreSQL function @p dlgamma()
 */
float8
float8_lgamma(float8 num)
{
  /*
   * Note: lgamma may not be thread-safe because it may write to a global
   * variable signgam, which may not be thread-local. However, this doesn't
   * matter to us, since we don't use signgam.
   */
  errno = 0;
  float8 result = lgamma(num);
  /*
   * If an ERANGE error occurs, it means there was an overflow or a pole
   * error (which happens for zero and negative integer inputs).
   *
   * On some platforms, lgamma() will not set errno but just return infinity
   * to report overflow, but it should never underflow.
   */
  if (errno == ERANGE || (isinf(result) && !isinf(num)))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "value out of range: overflow");
    return DBL_MAX;
  }
  return result;
}

/*
 *    ====================================
 *    MIXED-PRECISION ARITHMETIC OPERATORS
 *    ====================================
 */

/*
 *    float48pl    - Return num1 + num2
 *    float48mi    - Return num1 - num2
 *    float48mul    - Return num1 * num2
 *    float48div    - Return num1 / num2
 */

/**
 * @ingroup meos_base_float
 * @brief Return the addition of a float4 number and a float8 number
 * @note Derived from PostgreSQL function @p float48pl()
 */
float8
add_float4_float8(float4 num1, float8 num2)
{
  return float8_pl((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the subtraction of a float4 number and a float8 number
 * @note Derived from PostgreSQL function @p float48mi()
 */
float8
minus_float4_float8(float4 num1, float8 num2)
{
  return float8_mi((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the multiplication of a float4 number and a float8 number
 * @note Derived from PostgreSQL function @p float48mul()
 */
float8
mul_float4_float8(float4 num1, float8 num2)
{
  return float8_mul((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the division of a float4 number and a float8 number
 * @note Derived from PostgreSQL function @p float48div()
 */
float8
div_float4_float8(float4 num1, float8 num2)
{
  return float8_div((float8) num1, num2);
}

/*
 *    float84pl    - Return num1 + num2
 *    float84mi    - Return num1 - num2
 *    float84mul    - Return num1 * num2
 *    float84div    - Return num1 / num2
 */

/**
 * @ingroup meos_base_float
 * @brief Return the addition of a float8 number and a float4 number
 * @note Derived from PostgreSQL function @p float84pl()
 */
float8
add_float8_float4(float8 num1, float4 num2)
{
  return float8_pl(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the subtraction of a float8 number and a float4 number
 * @note Derived from PostgreSQL function @p float84mi()
 */
float8
minus_float8_float4(float8 num1, float4 num2)
{
  return float8_mi(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the multiplication of a float8 number and a float4 number
 * @note Derived from PostgreSQL function @p float84mul()
 */
float8
mul_float8_float4(float8 num1, float4 num2)
{
  return float8_mul(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the division of a float8 number and a float4 number
 * @note Derived from PostgreSQL function @p float84div()
 */
float8
div_float8_float4(float8 num1, float4 num2)
{
  return float8_div(num1, (float8) num2);
}

/*
 *    ====================
 *    COMPARISON OPERATORS
 *    ====================
 */

/*
 *    float48{eq,ne,lt,le,gt,ge}    - float4/float8 comparison operations
 */

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is equal to a float8 number
 * @note Derived from PostgreSQL function @p float48eq()
 */
bool
eq_float4_float8(float4 num1, float8 num2)
{
  return float8_eq((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is not equal to a float8 number
 * @note Derived from PostgreSQL function @p float48ne()
 */
bool
ne_float4_float8(float4 num1, float8 num2)
{
  return float8_ne((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is less than a float8 number
 * @note Derived from PostgreSQL function @p float48lt()
 */
bool
lt_float4_float8(float4 num1, float8 num2)
{
  return float8_lt((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is less than or equal to a float8
 * number
 * @note Derived from PostgreSQL function @p float48le()
 */
bool
le_float4_float8(float4 num1, float8 num2)
{
  return float8_le((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is greater than a float8 number
 * @note Derived from PostgreSQL function @p float48gt()
 */
bool
gt_float4_float8(float4 num1, float8 num2)
{
  return float8_gt((float8) num1, num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float4 number is greater than or equal to a float8
 * number
 * @note Derived from PostgreSQL function @p float48ge()
 */
bool
ge_float4_float8(float4 num1, float8 num2)
{
  return float8_ge((float8) num1, num2);
}

/*
 *    float84{eq,ne,lt,le,gt,ge}    - float8/float4 comparison operations
 */

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is equal to a float4 number
 * @note Derived from PostgreSQL function @p float84eq()
 */
bool
eq_float8_float4(float8 num1, float4 num2)
{
  return float8_eq(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is not equal to a float4 number
 * @note Derived from PostgreSQL function @p float84ne()
 */
bool
ne_float8_float4(float8 num1, float4 num2)
{
  return float8_ne(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is less than a float4 number
 * @note Derived from PostgreSQL function @p float84lt()
 */
bool
lt_float8_float4(float8 num1, float4 num2)
{
  return float8_lt(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is less than or equal to a float4
 * number
 * @note Derived from PostgreSQL function @p float84le()
 */
bool
le_float8_float4(float8 num1, float4 num2)
{
  return float8_le(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is greater than a float4 number
 * @note Derived from PostgreSQL function @p float84gt()
 */
bool
gt_float8_float4(float8 num1, float4 num2)
{
  return float8_gt(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return true if a float8 number is greater than or equal to a float4
 * number
 * @note Derived from PostgreSQL function @p float84ge()
 */
bool
ge_float8_float4(float8 num1, float4 num2)
{
  return float8_ge(num1, (float8) num2);
}

/**
 * @ingroup meos_base_float
 * @brief Return the number of the bucket in which a float8 value falls in a
 * histogram having count equal-width buckets spanning the range low to high
 * @details Implements the float8 version of the width_bucket() function
 * defined by SQL2003. See also width_bucket_numeric().
 *
 * 'bound1' and 'bound2' are the lower and upper bounds of the
 * histogram's range, respectively. 'count' is the number of buckets
 * in the histogram. width_bucket() Return an integer indicating the
 * bucket number that 'num' belongs to in an equiwidth histogram
 * with the specified characteristics. An num smaller than the
 * lower bound is assigned to bucket 0. An operand greater than or equal
 * to the upper bound is assigned to an additional bucket (with number
 * count+1). We don't allow "NaN" for any of the float8 inputs, and we
 * don't allow either of the histogram bounds to be +/- infinity.
 * @note Derived from PostgreSQL function @p width_bucket_float8()
 */
int32
float8_width_bucket(float8 operand, float8 bound1, float8 bound2, int32 count)
{
  if (count <= 0)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "count must be greater than zero");
    return INT_MAX;
  }

  if (isnan(operand) || isnan(bound1) || isnan(bound2))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "operand, lower bound, and upper bound cannot be NaN");
    return INT_MAX;
  }
  /* Note that we allow "operand" to be infinite */
  if (isinf(bound1) || isinf(bound2))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "lower and upper bounds must be finite");
    return INT_MAX;
  }

  int32 result;
  if (bound1 < bound2)
  {
    if (operand < bound1)
      result = 0;
    else if (operand >= bound2)
    {
      if (pg_add_s32_overflow(count, 1, &result))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "integer out of range");
        return INT_MAX;
      }
    }
    else
    {
      if (!isinf(bound2 - bound1))
      {
        /* The quotient is surely in [0,1], so this can't overflow */
        result = count * ((operand - bound1) / (bound2 - bound1));
      }
      else
      {
        /*
         * We get here if bound2 - bound1 overflows DBL_MAX.  Since
         * both bounds are finite, their difference can't exceed twice
         * DBL_MAX; so we can perform the computation without overflow
         * by dividing all the inputs by 2.  That should be exact too,
         * except in the case where a very small operand underflows to
         * zero, which would have negligible impact on the result
         * given such large bounds.
         */
        result = count * ((operand / 2 - bound1 / 2) / (bound2 / 2 - bound1 / 2));
      }
      /* The quotient could round to 1.0, which would be a lie */
      if (result >= count)
        result = count - 1;
      /* Having done that, we can add 1 without fear of overflow */
      result++;
    }
  }
  else if (bound1 > bound2)
  {
    if (operand > bound1)
      result = 0;
    else if (operand <= bound2)
    {
      if (pg_add_s32_overflow(count, 1, &result))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "integer out of range");
        return INT_MAX;
      }
    }
    else
    {
      if (!isinf(bound1 - bound2))
        result = count * ((bound1 - operand) / (bound1 - bound2));
      else
        result = count * ((bound1 / 2 - operand / 2) / (bound1 / 2 - bound2 / 2));
      if (result >= count)
        result = count - 1;
      result++;
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "lower bound cannot equal upper bound");
    return INT_MAX;
  }
  return result;
}

/*****************************************************************************/
