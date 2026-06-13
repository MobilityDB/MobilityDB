/*-------------------------------------------------------------------------
 *
 * numeric.h
 *    Definitions for the exact numeric data type of Postgres
 *
 * Original coding 1998, Jan Wieck.  Heavily revised 2003, Tom Lane.
 *
 * Copyright (c) 1998-2025, PostgreSQL Global Development Group
 *
 * src/include/utils/numeric.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef __PG_NUMERIC_H__
#define __PG_NUMERIC_H__

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef float float4;
typedef double float8;

/*
 * Limits on the precision and scale specifiable in a NUMERIC typmod.  The
 * precision is strictly positive, but the scale may be positive or negative.
 * A negative scale implies rounding before the decimal point.
 *
 * Note that the minimum display scale defined below is zero --- we always
 * display all digits before the decimal point, even when the scale is
 * negative.
 *
 * Note that the implementation limits on the precision and display scale of a
 * numeric value are much larger --- beware of what you use these for!
 */
#define NUMERIC_MAX_PRECISION    1000

#define NUMERIC_MIN_SCALE      (-1000)
#define NUMERIC_MAX_SCALE      1000

/*
 * Internal limits on the scales chosen for calculation results
 */
#define NUMERIC_MAX_DISPLAY_SCALE  NUMERIC_MAX_PRECISION
#define NUMERIC_MIN_DISPLAY_SCALE  0

#define NUMERIC_MAX_RESULT_SCALE  (NUMERIC_MAX_PRECISION * 2)

/*
 * For inherently inexact calculations such as division and square root,
 * we try to get at least this many significant digits; the idea is to
 * deliver a result no worse than float8 would.
 */
#define NUMERIC_MIN_SIG_DIGITS    16

/* The actual contents of Numeric are private to numeric.c */
struct NumericData;
typedef struct NumericData *Numeric;

/* MEOS functions for numeric */

extern Numeric float4_to_numeric(float4 num);
extern Numeric float8_to_numeric(float8 num);
extern Numeric int16_to_numeric(int16 num);
extern Numeric int32_to_numeric(int32 num);
extern Numeric int64_to_numeric(int64 num);
extern Numeric numeric(Numeric num, int32 typmod);
extern Numeric numeric_abs(Numeric num);
extern Numeric numeric_add(Numeric num1, Numeric num2);
extern Numeric numeric_ceil(Numeric num);
extern int32 numeric_cmp(Numeric num1, Numeric num2);
extern Numeric numeric_copy(Numeric num);
extern Numeric numeric_div(Numeric num1, Numeric num2);
extern Numeric numeric_div_trunc(Numeric num1, Numeric num2);
extern bool numeric_eq(Numeric num1, Numeric num2);
extern Numeric numeric_exp(Numeric num);
extern Numeric numeric_fac(int64 num);
extern Numeric numeric_floor(Numeric num);
extern Numeric numeric_gcd(Numeric num1, Numeric num2);
extern bool numeric_ge(Numeric num1, Numeric num2);
extern bool numeric_gt(Numeric num1, Numeric num2);
extern uint32 numeric_hash(Numeric key);
extern uint64 numeric_hash_extended(Numeric num, uint64 seed);
extern Numeric numeric_in(const char *str, int32 typmod);
extern Numeric numeric_inc(Numeric num);
extern Numeric numeric_larger(Numeric num1, Numeric num2);
extern Numeric numeric_lcm(Numeric num1, Numeric num2);
extern bool numeric_le(Numeric num1, Numeric num2);
extern Numeric numeric_ln(Numeric num);
extern Numeric numeric_log(Numeric num1, Numeric num2);
extern bool numeric_lt(Numeric num1, Numeric num2);
extern uint32 numeric_min_scale(Numeric num);
extern Numeric numeric_minus(Numeric num1, Numeric num2);
extern Numeric numeric_mod(Numeric num1, Numeric num2);
extern Numeric numeric_mul(Numeric num1, Numeric num2);
extern bool numeric_ne(Numeric num1, Numeric num2);
extern char *numeric_out(Numeric num);
extern Numeric numeric_pow(Numeric num1, Numeric num2);
extern Numeric numeric_round(Numeric num, int32 scale);
extern int32 numeric_scale(Numeric num);
extern Numeric numeric_sign(Numeric num);
extern Numeric numeric_smaller(Numeric num1, Numeric num2);
extern Numeric numeric_sqrt(Numeric num);
extern float4 numeric_to_float4(Numeric num);
extern float8 numeric_to_float8(Numeric num);
extern int16 numeric_to_int16(Numeric num);
extern int32 numeric_to_int32(Numeric num);
extern int64 numeric_to_int64(Numeric num);
extern Numeric numeric_trim_scale(Numeric num);
extern Numeric numeric_trunc(Numeric num, int32 scale);
extern Numeric numeric_uminus(Numeric num);
extern Numeric numeric_uplus(Numeric num);
extern int32 numeric_width_bucket(Numeric operand, Numeric bound1, Numeric bound2, int32 count);

#endif /* __PG_NUMERIC_H__ */

/*****************************************************************************/
