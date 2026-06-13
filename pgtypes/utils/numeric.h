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
#ifndef _PG_NUMERIC_H_
#define _PG_NUMERIC_H_

#include <postgres.h>

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

/*
 * fmgr interface macros
 */

static inline Numeric
DatumGetNumeric(Datum X)
{
  return (Numeric) X;
}

static inline Datum
NumericGetDatum(Numeric X)
{
  return PointerGetDatum(X);
}

#define PG_GETARG_NUMERIC(n)    DatumGetNumeric(PG_GETARG_DATUM(n))
#define PG_RETURN_NUMERIC(x)    return NumericGetDatum(x)

/*
 * Utility functions in numeric.c
 */
extern bool numeric_is_nan(Numeric num);
extern bool numeric_is_inf(Numeric num);
extern int32 numeric_maximum_size(int32 typmod);
extern char *numeric_out_sci(Numeric num, int scale);
extern char *numeric_normalize(Numeric num);

extern Numeric int64_to_numeric(int64 val);
extern Numeric int64_div_fast_to_numeric(int64 val1, int log10val2);

extern Numeric numeric_add_opt_error(Numeric num1, Numeric num2,
  bool *have_error);
extern Numeric numeric_sub_opt_error(Numeric num1, Numeric num2,
  bool *have_error);
extern Numeric numeric_mul_opt_error(Numeric num1, Numeric num2,
  bool *have_error);
extern Numeric numeric_div_opt_error(Numeric num1, Numeric num2,
  bool *have_error);
extern Numeric numeric_mod_opt_error(Numeric num1, Numeric num2,
  bool *have_error);
extern int32 pg_numeric_int4_opt_error(Numeric num, bool *have_error);
extern int64 pg_numeric_int8_opt_error(Numeric num, bool *have_error);

/* MEOS functions for numeric */

extern Numeric pg_numeric_copy(Numeric num);
extern Numeric pg_numeric_in(const char *str, int32 typmod);
extern char *pg_numeric_out(Numeric num);
extern Numeric pg_numeric(Numeric num, int32 typmod);
extern Numeric pg_numeric_abs(Numeric num);
extern Numeric pg_numeric_uplus(Numeric num);
extern Numeric pg_numeric_uminus(Numeric num);
extern Numeric pg_numeric_sign(Numeric num);
extern Numeric pg_numeric_round(Numeric num, int32 scale);
extern Numeric pg_numeric_trunc(Numeric num, int32 scale);
extern Numeric pg_numeric_ceil(Numeric num);
extern Numeric pg_numeric_floor(Numeric num);
extern int32 numeric_width_bucket(Numeric operand, Numeric bound1,
  Numeric bound2, int32 count);
extern int32 pg_numeric_cmp(Numeric num1, Numeric num2);
extern bool pg_numeric_eq(Numeric num1, Numeric num2);
extern bool pg_numeric_ne(Numeric num1, Numeric num2);
extern bool pg_numeric_gt(Numeric num1, Numeric num2);
extern bool pg_numeric_ge(Numeric num1, Numeric num2);
extern bool pg_numeric_lt(Numeric num1, Numeric num2);
extern bool pg_numeric_le(Numeric num1, Numeric num2);
extern uint32 numeric_hash(Numeric key);
extern uint64 numeric_hash_extended(Numeric key, uint64 seed);
extern Numeric pg_numeric_add(Numeric num1, Numeric num2);
extern Numeric pg_numeric_sub(Numeric num1, Numeric num2);
extern Numeric pg_numeric_mul(Numeric num1, Numeric num2);
extern Numeric pg_numeric_div(Numeric num1, Numeric num2);
extern Numeric pg_numeric_div_trunc(Numeric num1, Numeric num2);
extern Numeric pg_numeric_mod(Numeric num1, Numeric num2);
extern Numeric pg_numeric_inc(Numeric num);
extern Numeric pg_numeric_smaller(Numeric num1, Numeric num2);
extern Numeric pg_numeric_larger(Numeric num1, Numeric num2);
extern Numeric pg_numeric_gcd(Numeric num1, Numeric num2);
extern Numeric pg_numeric_lcm(Numeric num1, Numeric num2);
extern Numeric pg_numeric_fac(int64 num);
extern Numeric pg_numeric_sqrt(Numeric num);
extern Numeric pg_numeric_exp(Numeric num);
extern Numeric pg_numeric_ln(Numeric num);
extern Numeric pg_numeric_log(Numeric num1, Numeric num2);
extern char *pg_numeric_out(Numeric num);
extern Numeric numeric_pow(Numeric num1, Numeric num2);
extern int32 pg_numeric_scale(Numeric num);
extern int32 pg_numeric_min_scale(Numeric num);
extern Numeric pg_numeric_trim_scale(Numeric num);

extern Numeric int16_to_numeric(int16 num);
extern int16 numeric_to_int16(Numeric num);
extern Numeric int32_to_numeric(int32 num);
extern int32 numeric_to_int32(Numeric num);
extern Numeric int64_to_numeric(int64 num);
extern int64 numeric_to_int64(Numeric num);
extern Numeric float4_to_numeric(float4 num);
extern float4 numeric_to_float4(Numeric num);
extern Numeric float8_to_numeric(float8 num);
extern float8 numeric_to_float8(Numeric num);

#endif              /* _PG_NUMERIC_H_ */
