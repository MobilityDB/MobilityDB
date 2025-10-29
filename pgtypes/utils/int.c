/*-------------------------------------------------------------------------
 *
 * int.c
 *    Functions for the built-in integer types (except int8).
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/int.c
 *
 *-------------------------------------------------------------------------
 */
/*
 * OLD COMMENTS
 *    I/O routines:
 *     int2in, int2out, int2recv, int2send
 *     int4in, int4out, int4recv, int4send
 *     int2vectorin, int2vectorout, int2vectorrecv, int2vectorsend
 *    Boolean operators:
 *     inteq, intne, intlt, intle, intgt, intge
 *    Arithmetic operators:
 *     intpl, intmi, int4mul, intdiv
 *
 *    Arithmetic operators:
 *     intmod
 */

/* C */
#include <assert.h>
#include <limits.h>
/* PostgreSQL */
#include "postgres.h"
#include "common/int.h"
#include "utils/builtins.h"

/*****************************************************************************/

// /* Functions migrated from int.h since they are declared static inline */

// /*
 // * INT32
 // */
// bool
// pg_add_s32_overflow(int32 a, int32 b, int32 *result)
// {
// #if defined(HAVE__BUILTIN_OP_OVERFLOW)
	// return __builtin_add_overflow(a, b, result);
// #else
	// int64		res = (int64) a + (int64) b;

	// if (res > PG_INT32_MAX || res < PG_INT32_MIN)
	// {
		// *result = 0x5EED;		/* to avoid spurious warnings */
		// return true;
	// }
	// *result = (int32) res;
	// return false;
// #endif
// }

// bool
// pg_mul_s32_overflow(int32 a, int32 b, int32 *result)
// {
// #if defined(HAVE__BUILTIN_OP_OVERFLOW)
	// return __builtin_mul_overflow(a, b, result);
// #else
	// int64		res = (int64) a * (int64) b;

	// if (res > PG_INT32_MAX || res < PG_INT32_MIN)
	// {
		// *result = 0x5EED;		/* to avoid spurious warnings */
		// return true;
	// }
	// *result = (int32) res;
	// return false;
// #endif
// }

/*****************************************************************************
 *   USER I/O ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_int
 * @brief Return an int16 number from its string representation
 * @note Derived from PostgreSQL function @p int2in()
 */
int16
int16_in(const char *str)
{
  return (pg_strtoint16_safe((char *) str, NULL));
}

/**
 * @ingroup meos_base_int
 * @brief Return the string representation of an int16 number
 * @note Derived from PostgreSQL function @p int2out()
 */
char *
int16_out(int16 num)
{
  char *result = (char *) palloc(7);  /* sign, 5 digits, '\0' */
  pg_itoa(num, result);
  return result;
}

/*****************************************************************************
 *   PUBLIC ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_int
 * @brief Return an int32 number from its string representation
 * @note Derived from PostgreSQL function @p int4in()
 */
int32
int32_in(const char *str)
{
  return pg_strtoint32_safe((char *) str, NULL);
}

/**
 * @ingroup meos_base_int
 * @brief Return the string representation of an int32 number
 * @note Derived from PostgreSQL function @p int4out()
 */
char *
int32_out(int32 num)
{
  char *result = (char *) palloc(12);  /* sign, 10 digits, '\0' */
  pg_ltoa(num, result);
  return result;
}

/*
 *    ===================
 *    CONVERSION ROUTINES
 *    ===================
 */

/**
 * @ingroup meos_base_int
 * @brief Convert an int16 number into an int32 number
 * @note Derived from PostgreSQL function @p i2toi4()
 */
int32
int16_to_int32(int16 num)
{
  return ((int32) num);
}

/**
 * @ingroup meos_base_int
 * @brief Convert an int32 number into an int16 number
 * @note Derived from PostgreSQL function @p i4toi2()
 */
int16
int32_to_int16(int32 num)
{
  if (unlikely(num < SHRT_MIN) || unlikely(num > SHRT_MAX))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }
  return ((int16) num);
}

/**
 * @ingroup meos_base_int
 * @brief Convert an int32 number into a boolean value
 * @note Derived from PostgreSQL function @p int4bool()
 */
bool
int32_to_bool(int32 num)
{
  if (num == 0)
    return (false);
  else
    return (true);
}

/**
 * @ingroup meos_base_int
 * @brief Convert a boolean value into an int32 number
 * @note Derived from PostgreSQL function @p boolint4()
 */
int32
bool_to_int32(bool b)
{
  if (b == false)
    return 0;
  else
    return 1;
}

/*
 *    ============================
 *    COMPARISON OPERATOR ROUTINES
 *    ============================
 */

/*
 *    inteq      - returns 1 iff num1 == num2
 *    intne      - returns 1 iff num1 != num2
 *    intlt      - returns 1 iff num1 < num2
 *    intle      - returns 1 iff num1 <= num2
 *    intgt      - returns 1 iff num1 > num2
 *    intge      - returns 1 iff num1 >= num2
 */

/**
 * @ingroup meos_base_int
 * @brief Return true if two int32 numbers are equal
 * @note Derived from PostgreSQL function @p int4eq()
 */
bool
eq_int32_int32(int32 num1, int32 num2)
{
  return (num1 == num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if two int32 numbers are not equal
 * @note Derived from PostgreSQL function @p int4ne()
 */
bool
ne_int32_int32(int32 num1, int32 num2)
{
  return (num1 != num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int32 number is less than the second one
 * @note Derived from PostgreSQL function @p int4lt()
 */
bool
lt_int32_int32(int32 num1, int32 num2)
{
  return (num1 < num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int32 number is less than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p int4le()
 */
bool
le_int32_int32(int32 num1, int32 num2)
{
  return (num1 <= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int32 number is greater than the second one
 * @note Derived from PostgreSQL function @p int4gt()
 */
bool
gt_int32_int32(int32 num1, int32 num2)
{
  return (num1 > num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int32 number is greater than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p int4ge()
 */
bool
ge_int32_int32(int32 num1, int32 num2)
{
  return (num1 >= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if two int16 numbers are equal
 * @note Derived from PostgreSQL function @p int2eq()
 */
bool
eq_int16_int16(int16 num1, int16 num2)
{
  return (num1 == num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if two int16 numbers are not equal
 * @note Derived from PostgreSQL function @p int2ne()
 */
bool
ne_int16_int16(int16 num1, int16 num2)
{
  return (num1 != num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int16 number is less than the second one
 * @note Derived from PostgreSQL function @p int2lt()
 */
bool
lt_int16_int16(int16 num1, int16 num2)
{
  return (num1 < num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int16 number is less than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p int2le()
 */
bool
le_int16_int16(int16 num1, int16 num2)
{
  return (num1 <= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int16 number is greater than the second one
 * @note Derived from PostgreSQL function @p int2gt()
 */
bool
gt_int16_int16(int16 num1, int16 num2)
{
  return (num1 > num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if the first int16 number is greater than or equal to the
 * second one
 * @note Derived from PostgreSQL function @p int2ge()
 */
bool
ge_int16_int16(int16 num1, int16 num2)
{
  return (num1 >= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number and an int32 number are equal
 * @note Derived from PostgreSQL function @p int24eq()
 */
bool
eq_int16_int32(int16 num1, int32 num2)
{
  return (num1 == num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number and an int32 number are not equal
 * @note Derived from PostgreSQL function @p int24ne()
 */
bool
ne_int16_int32(int16 num1, int32 num2)
{
  return (num1 != num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number is less than an int32 number
 * @note Derived from PostgreSQL function @p int24lt()
 */
bool
lt_int16_int32(int16 num1, int32 num2)
{
  return (num1 < num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number is less than or equal to an int32
 * number
 * @note Derived from PostgreSQL function @p int24le()
 */
bool
le_int16_int32(int16 num1, int32 num2)
{
  return (num1 <= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number is greater than an int32 number
 * @note Derived from PostgreSQL function @p int24gt()
 */
bool
gt_int16_int32(int16 num1, int32 num2)
{
  return (num1 > num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int16 number is greater than or equal to an int32
 * number
 * @note Derived from PostgreSQL function @p int24ge()
 */
bool
ge_int16_int32(int16 num1, int32 num2)
{
  return (num1 >= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number and an int16 number are equal
 * @note Derived from PostgreSQL function @p int42eq()
 */
bool
eq_int32_int16(int32 num1, int16 num2)
{
  return (num1 == num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number and an int16 number are not equal
 * @note Derived from PostgreSQL function @p int42ne()
 */
bool
ne_int32_int16(int32 num1, int16 num2)
{
  return (num1 != num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number is less than an int16 number
 * @note Derived from PostgreSQL function @p int42lt()
 */
bool
lt_int32_int16(int32 num1, int16 num2)
{
  return (num1 < num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number is less than or equal to an int16
 * number
 * @note Derived from PostgreSQL function @p int42le()
 */
bool
le_int32_int16(int32 num1, int16 num2)
{
  return (num1 <= num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number is greater than an int16 number
 * @note Derived from PostgreSQL function @p int42gt()
 */
bool
gt_int32_int16(int32 num1, int16 num2)
{
  return (num1 > num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return true if an int32 number is less than or equal to an int16
 * number
 * @note Derived from PostgreSQL function @p int42ge()
 */
bool
ge_int32_int16(int32 num1, int16 num2)
{
  return (num1 >= num2);
}

/*----------------------------------------------------------*/


/*
 *    int[24]pl    - returns num1 + num2
 *    int[24]mi    - returns num1 - num2
 *    int[24]mul    - returns num1 * num2
 *    int[24]div    - returns num1 / num2
 */
 
/**
 * @ingroup meos_base_int
 * @brief Return the unary minus of an int32 number
 * @note Derived from PostgreSQL function @p int4um()
 */
int32
int32_uminus(int32 num)
{
  if (unlikely(num == PG_INT32_MIN))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return (-num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the unary plus of an int32 number
 * @note Derived from PostgreSQL function @p int4up()
 */
int32
int32_uplus(int32 num)
{
  return (num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the adition of two int32 number
 * @note Derived from PostgreSQL function @p int4pl()
 */
int32
add_int32_int32(int32 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_add_s32_overflow(num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the subraction of two int32 number
 * @note Derived from PostgreSQL function @p int4mi()
 */
int32
minus_int32_int32(int32 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_sub_s32_overflow(num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the multiplication of two int32 number
 * @note Derived from PostgreSQL function @p int4mul()
 */
int32
mul_int32_int32(int32 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_mul_s32_overflow(num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the division of two int32 number
 * @note Derived from PostgreSQL function @p int4div()
 */
int32
div_int32_int32(int32 num1, int32 num2)
{
  int32 result;

  if (num2 == 0)
  {
    elog(ERROR, "division by zero");
    return INT_MAX;
  }

  /*
   * INT_MIN / -1 is problematic, since the result can't be represented on a
   * two's-complement machine.  Some machines produce INT_MIN, some produce
   * zero, some throw an exception.  We can dodge the problem by recognizing
   * that division by -1 is the same as negation.
   */
  if (num2 == -1)
  {
    if (unlikely(num1 == PG_INT32_MIN))
    {
      elog(ERROR, "integer out of range");
      return INT_MAX;
    }
    result = -num1;
    return result;
  }

  /* No overflow is possible */

  result = num1 / num2;

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return an int32 number incremented by one
 * @note Derived from PostgreSQL function @p int4inc()
 */
int32
int32_inc(int32 num)
{
  int32 result;
  if (unlikely(pg_add_s32_overflow(num, 1, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the unary minus of an int16 number
 * @note Derived from PostgreSQL function @p int2um()
 */
int16
int16_uminus(int16 num)
{
  if (unlikely(num == PG_INT16_MIN))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }
  return (-num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the unary plus of an int16 number
 * @note Derived from PostgreSQL function @p int2up()
 */
int16
int16_uplus(int16 num)
{
  return (num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the addition of two int16 numbers
 * @note Derived from PostgreSQL function @p int2pl()
 */
int16
add_int16_int16(int16 num1, int16 num2)
{
  int16    result;

  if (unlikely(pg_add_s16_overflow(num1, num2, &result)))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the subtraction of two int16 numbers
 * @note Derived from PostgreSQL function @p int2mi()
 */
int16
minus_int16_int16(int16 num1, int16 num2)
{
  int16    result;

  if (unlikely(pg_sub_s16_overflow(num1, num2, &result)))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the multiplication of two int16 numbers
 * @note Derived from PostgreSQL function @p int2mul()
 */
int16
mul_int16_int16(int16 num1, int16 num2)
{
  int16    result;

  if (unlikely(pg_mul_s16_overflow(num1, num2, &result)))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the division of two int16 numbers
 * @note Derived from PostgreSQL function @p int2div()
 */
int16
div_int16_int16(int16 num1, int16 num2)
{
  int16    result;

  if (num2 == 0)
  {
    elog(ERROR, "division by zero");
    return SHRT_MAX;
  }

  /*
   * SHRT_MIN / -1 is problematic, since the result can't be represented on
   * a two's-complement machine.  Some machines produce SHRT_MIN, some
   * produce zero, some throw an exception.  We can dodge the problem by
   * recognizing that division by -1 is the same as negation.
   */
  if (num2 == -1)
  {
    if (unlikely(num1 == PG_INT16_MIN))
    {
      elog(ERROR, "smallint out of range");
      return SHRT_MAX;
    }
    result = -num1;
    return result;
  }

  /* No overflow is possible */

  result = num1 / num2;

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the addition of an int16 and an int32 numbers
 * @note Derived from PostgreSQL function @p int24pl()
 */
int32
add_int16_int32(int16 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_add_s32_overflow((int32) num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the subtraction of an int16 and an int32 numbers
 * @note Derived from PostgreSQL function @p int24mi()
 */
int32
minus_int16_int32(int16 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_sub_s32_overflow((int32) num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the multiplication of an int16 and an int32 numbers
 * @note Derived from PostgreSQL function @p int24mul()
 */
int32
mul_int16_int32(int16 num1, int32 num2)
{
  int32 result;

  if (unlikely(pg_mul_s32_overflow((int32) num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the division of an int16 and an int32 numbers
 * @note Derived from PostgreSQL function @p int24div()
 */
int32
div_int16_int32(int16 num1, int32 num2)
{
  if (unlikely(num2 == 0))
  {
    elog(ERROR, "division by zero");
    return INT_MAX;
  }

  /* No overflow is possible */
  return ((int32) num1 / num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the addition of an int32 and an int16 numbers
 * @note Derived from PostgreSQL function @p int42pl()
 */
int32
add_int32_int16(int32 num1, int16 num2)
{
  int32 result;

  if (unlikely(pg_add_s32_overflow(num1, (int32) num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the subtraction of an int32 and an int16 numbers
 * @note Derived from PostgreSQL function @p int42mi()
 */
int32
minus_int32_int16(int32 num1, int16 num2)
{
  int32 result;

  if (unlikely(pg_sub_s32_overflow(num1, (int32) num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the multiplication of an int32 and an int16 numbers
 * @note Derived from PostgreSQL function @p int42mul()
 */
int32
mul_int32_int16(int32 num1, int16 num2)
{
  int32 result;

  if (unlikely(pg_mul_s32_overflow(num1, (int32) num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the division of an int32 and an int16 numbers
 * @note Derived from PostgreSQL function @p int42div()
 */
int32
div_int32_int16(int32 num1, int16 num2)
{
  int32 result;

  if (unlikely(num2 == 0))
  {
    elog(ERROR, "division by zero");
    return INT_MAX;
  }

  /*
   * INT_MIN / -1 is problematic, since the result can't be represented on a
   * two's-complement machine.  Some machines produce INT_MIN, some produce
   * zero, some throw an exception.  We can dodge the problem by recognizing
   * that division by -1 is the same as negation.
   */
  if (num2 == -1)
  {
    if (unlikely(num1 == PG_INT32_MIN))
    {
      elog(ERROR, "integer out of range");
      return INT_MAX;
    }
    result = -num1;
    return result;
  }

  /* No overflow is possible */

  result = num1 / num2;

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the modulo of two int32 numbers
 * @note Derived from PostgreSQL function @p int4mod()
 */
int32
int32_mod(int32 num1, int32 num2)
{
  if (unlikely(num2 == 0))
  {
    elog(ERROR, "division by zero");
    return INT_MAX;
  }

  /*
   * Some machines throw a floating-point exception for INT_MIN % -1, which
   * is a bit silly since the correct answer is perfectly well-defined,
   * namely zero.
   */
  if (num2 == -1)
    return (0);

  /* No overflow is possible */

  return (num1 % num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the modulo of two int16 numbers
 * @note Derived from PostgreSQL function @p int2mod()
 */
int16
int16_mod(int16 num1, int16 num2)
{
  if (unlikely(num2 == 0))
  {
    elog(ERROR, "division by zero");
    return SHRT_MAX;
  }

  /*
   * Some machines throw a floating-point exception for INT_MIN % -1, which
   * is a bit silly since the correct answer is perfectly well-defined,
   * namely zero.  (It's not clear this ever happens when dealing with
   * int16, but we might as well have the test for safety.)
   */
  if (num2 == -1)
    return (0);

  /* No overflow is possible */

  return (num1 % num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the absolute value of an int32 number
 * @note Derived from PostgreSQL function @p int4abs()
 */
int32
int32_abs(int32 num)
{
  int32 result;

  if (unlikely(num == PG_INT32_MIN))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }
  result = (num < 0) ? -num : num;
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the absolute value of an int16 number
 * @note Derived from PostgreSQL function @p int2abs()
 */
int16
int16_abs(int16 num)
{
  int16    result;

  if (unlikely(num == PG_INT16_MIN))
  {
    elog(ERROR, "smallint out of range");
    return SHRT_MAX;
  }
  result = (num < 0) ? -num : num;
  return result;
}

/*
 * Greatest Common Divisor
 *
 * Returns the largest positive integer that exactly divides both inputs.
 * Special cases:
 *   - gcd(x, 0) = gcd(0, x) = abs(x)
 *       because 0 is divisible by anything
 *   - gcd(0, 0) = 0
 *       complies with the previous definition and is a common convention
 *
 * Special care must be taken if either input is INT_MIN --- gcd(0, INT_MIN),
 * gcd(INT_MIN, 0) and gcd(INT_MIN, INT_MIN) are all equal to abs(INT_MIN),
 * which cannot be represented as a 32-bit signed integer.
 */
static int32
int4gcd_internal(int32 num1, int32 num2)
{
  int32    swap;
  int32    a1,
        a2;

  /*
   * Put the greater absolute value in num1.
   *
   * This would happen automatically in the loop below, but avoids an
   * expensive modulo operation, and simplifies the special-case handling
   * for INT_MIN below.
   *
   * We do this in negative space in order to handle INT_MIN.
   */
  a1 = (num1 < 0) ? num1 : -num1;
  a2 = (num2 < 0) ? num2 : -num2;
  if (a1 > a2)
  {
    swap = num1;
    num1 = num2;
    num2 = swap;
  }

  /* Special care needs to be taken with INT_MIN.  See comments above. */
  if (num1 == PG_INT32_MIN)
  {
    if (num2 == 0 || num2 == PG_INT32_MIN)
    {
      elog(ERROR, "integer out of range");
    }

    /*
     * Some machines throw a floating-point exception for INT_MIN % -1,
     * which is a bit silly since the correct answer is perfectly
     * well-defined, namely zero.  Guard against this and just return the
     * result, gcd(INT_MIN, -1) = 1.
     */
    if (num2 == -1)
      return 1;
  }

  /* Use the Euclidean algorithm to find the GCD */
  while (num2 != 0)
  {
    swap = num2;
    num2 = num1 % num2;
    num1 = swap;
  }

  /*
   * Make sure the result is positive. (We know we don't have INT_MIN
   * anymore).
   */
  if (num1 < 0)
    num1 = -num1;

  return num1;
}

/**
 * @ingroup meos_base_int
 * @brief Return the greatest common denominator of two int32 numbers
 * @note Derived from PostgreSQL function @p int4gcd()
 */
int32
int32_gcd(int32 num1, int32 num2)
{
  int32 result = int4gcd_internal(num1, num2);
  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the least common multiple of two int32 numbers
 * @note Derived from PostgreSQL function @p int4lcm()
 */
int32
int32_lcm(int32 num1, int32 num2)
{
  int32    gcd;
  int32 result;

  /*
   * Handle lcm(x, 0) = lcm(0, x) = 0 as a special case.  This prevents a
   * division-by-zero error below when x is zero, and an overflow error from
   * the GCD computation when x = INT_MIN.
   */
  if (num1 == 0 || num2 == 0)
    return (0);

  /* lcm(x, y) = abs(x / gcd(x, y) * y) */
  gcd = int4gcd_internal(num1, num2);
  num1 = num1 / gcd;

  if (unlikely(pg_mul_s32_overflow(num1, num2, &result)))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }

  /* If the result is INT_MIN, it cannot be represented. */
  if (unlikely(result == PG_INT32_MIN))
  {
    elog(ERROR, "integer out of range");
    return INT_MAX;
  }

  if (result < 0)
    result = -result;

  return result;
}

/**
 * @ingroup meos_base_int
 * @brief Return the larger of two int16 numbers
 * @note Derived from PostgreSQL function @p int2larger()
 */
int16
int16_larger(int16 num1, int16 num2)
{
  return ((num1 > num2) ? num1 : num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the smaller of two int16 numbers
 * @note Derived from PostgreSQL function @p int2smaller()
 */
int16
int16_smaller(int16 num1, int16 num2)
{
  return ((num1 < num2) ? num1 : num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the larger of two int32 numbers
 * @note Derived from PostgreSQL function @p int4larger()
 */
int32
int32_larger(int32 num1, int32 num2)
{
  return ((num1 > num2) ? num1 : num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the smaller of two int32 numbers
 * @note Derived from PostgreSQL function @p int4smaller()
 */
int32
int32_smaller(int32 num1, int32 num2)
{
  return ((num1 < num2) ? num1 : num2);
}

/*
 * Bit-pushing operators
 *
 *    int[24]and    - returns num1 & num2
 *    int[24]or    - returns num1 | num2
 *    int[24]xor    - returns num1 # num2
 *    int[24]not    - returns ~num1
 *    int[24]shl    - returns num1 << num2
 *    int[24]shr    - returns num1 >> num2
 */

/**
 * @ingroup meos_base_int
 * @brief Return the binary and of two int32 numbers
 * @note Derived from PostgreSQL function @p int4and()
 */
int32
int32_and(int32 num1, int32 num2)
{
  return (num1 & num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary or of two int32 numbers
 * @note Derived from PostgreSQL function @p int4or()
 */
int32
int32_or(int32 num1, int32 num2)
{
  return (num1 | num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary xor of two int32 numbers
 * @note Derived from PostgreSQL function @p int4xor()
 */
int32
int32_xor(int32 num1, int32 num2)
{
  return (num1 ^ num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the first int32 number shifted to the left by the second one
 * @note Derived from PostgreSQL function @p int4shl()
 */
int32
int32_shl(int32 num1, int32 num2)
{
  return (num1 << num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the first int32 number shifted to the right by the second one
 * @note Derived from PostgreSQL function @p int4shr()
 */
int32
int32_shr(int32 num1, int32 num2)
{
  return (num1 >> num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary not of a int32 number
 * @note Derived from PostgreSQL function @p int4not()
 */
int32
int32_not(int32 num)
{
  return (~num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary and of two int16 numbers
 * @note Derived from PostgreSQL function @p int2and()
 */
int16
int16_and(int16 num1, int16 num2)
{
  return (num1 & num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary or of two int16 numbers
 * @note Derived from PostgreSQL function @p int2or()
 */
int16
int16_or(int16 num1, int16 num2)
{
  return (num1 | num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary xor of two int16 numbers
 * @note Derived from PostgreSQL function @p int2xor()
 */
int16
int16_xor(int16 num1, int16 num2)
{
  return (num1 ^ num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the binary not of an int16 number
 * @note Derived from PostgreSQL function @p int2not()
 */
int16
int16_not(int16 num)
{
  return (~num);
}

/**
 * @ingroup meos_base_int
 * @brief Return the first int16 number shifted to the left by the second one
 * @note Derived from PostgreSQL function @p int2shl()
 */
int16
int16_shl(int16 num1, int32 num2)
{
  return (num1 << num2);
}

/**
 * @ingroup meos_base_int
 * @brief Return the first int16 number shifted to the right by the second one
 * @note Derived from PostgreSQL function @p int2shr()
 */
int16
int16_shr(int16 num1, int32 num2)
{
  return (num1 >> num2);
}

/*****************************************************************************/