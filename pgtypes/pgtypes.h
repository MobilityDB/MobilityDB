/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Functions for base and time types corresponding to the external (SQL)
 * PostgreSQL functions
 */

#ifndef POSTGRES_TYPES_H
#define POSTGRES_TYPES_H

/* PostgreSQL */
#include <postgres.h>
#include "utils/numeric.h"
#include <utils/date.h>
#include <utils/json.h>
#include <utils/jsonb.h>
#include <utils/timestamp.h>

#if POSTGRESQL_VERSION_NUMBER < 170000
/*
 * Infinite intervals are represented by setting all fields to the minimum or
 * maximum integer values.
 */
#define INTERVAL_NOBEGIN(i)  \
  do {  \
    (i)->time = PG_INT64_MIN;  \
    (i)->day = PG_INT32_MIN;  \
    (i)->month = PG_INT32_MIN;  \
  } while (0)

#define INTERVAL_IS_NOBEGIN(i)  \
  ((i)->month == PG_INT32_MIN && (i)->day == PG_INT32_MIN && (i)->time == PG_INT64_MIN)

#define INTERVAL_NOEND(i)  \
  do {  \
    (i)->time = PG_INT64_MAX;  \
    (i)->day = PG_INT32_MAX;  \
    (i)->month = PG_INT32_MAX;  \
  } while (0)

#define INTERVAL_IS_NOEND(i)  \
  ((i)->month == PG_INT32_MAX && (i)->day == PG_INT32_MAX && (i)->time == PG_INT64_MAX)

#define INTERVAL_NOT_FINITE(i) (INTERVAL_IS_NOBEGIN(i) || INTERVAL_IS_NOEND(i))
#endif /* POSTGRESQL_VERSION_NUMBER < 170000 */

/*****************************************************************************/

/* Hash functions */

extern uint32 char_hash(char c);
extern uint64 char_hash_extended(char c, uint64 seed);
extern uint32 int16_hash(int16 val);
extern uint64 int16_hash_extended(int16 val, uint64 seed);
extern uint32 int32_hash(int32 val);
extern uint64 int32_hash_extended(int32 val, uint64 seed);
extern uint32 int64_hash(int64 val);
extern uint64 int64_hash_extended(int64 val, uint64 seed);
extern uint32 float4_hash(float4 num);
extern uint64 float4_hash_extended(float4 num, uint64 seed);
extern uint32 float8_hash(float8 num);
extern uint64 float8_hash_extended(float8 num, uint64 seed);
extern uint32 text_hash(const text *num, Oid collid);
extern uint64 text_hash_extended(const text *txt, uint64 seed, Oid collid);

/* Functions for booleans */

extern bool bool_eq(bool arg1, bool arg2);
extern bool bool_ge(bool arg1, bool arg2);
extern bool bool_gt(bool arg1, bool arg2);
extern uint32 bool_hash(bool arg);
extern uint64 bool_hash_extended(bool arg, int64 seed);
extern bool bool_in(const char *str);
extern bool bool_le(bool arg1, bool arg2);
extern bool bool_lt(bool arg1, bool arg2);
extern bool bool_ne(bool arg1, bool arg2);
extern char *bool_out(bool b);
extern text *bool_to_text(bool b);

/* Functions for integers */

extern int16 add_int16_int16(int16 num1, int16 num2);
extern int32 add_int16_int32(int16 num1, int32 num2);
extern int64 add_int16_int64(int16 num1, int64 num2);
extern int32 add_int32_int16(int32 num1, int16 num2);
extern int32 add_int32_int32(int32 num1, int32 num2);
extern int64 add_int32_int64(int32 num1, int64 num2);
extern int64 add_int64_int16(int64 num1, int16 num2);
extern int64 add_int64_int32(int64 num1, int32 num2);
extern int64 add_int64_int64(int64 num1, int64 num2);
extern int32 bool_to_int32(bool b);
extern int16 div_int16_int16(int16 num1, int16 num2);
extern int32 div_int16_int32(int16 num1, int32 num2);
extern int64 div_int16_int64(int16 num1, int64 num2);
extern int32 div_int32_int16(int32 num1, int16 num2);
extern int32 div_int32_int32(int32 num1, int32 num2);
extern int64 div_int32_int64(int32 num1, int64 num2);
extern int64 div_int64_int16(int64 num1, int16 num2);
extern int64 div_int64_int32(int64 num1, int32 num2);
extern int64 div_int64_int64(int64 num1, int64 num2);
extern bool eq_int16_int32(int16 num1, int32 num2);
extern bool eq_int16_int16(int16 num1, int16 num2);
extern bool eq_int16_int64(int16 num1, int64 num2);
extern bool eq_int32_int16(int32 num1, int16 num2);
extern bool eq_int32_int32(int32 num1, int32 num2);
extern bool eq_int32_int64(int32 num1, int64 num2);
extern bool eq_int64_int16(int64 num1, int16 num2);
extern bool eq_int64_int32(int64 num1, int32 num2);
extern bool eq_int64_int64(int64 num1, int64 num2);
extern int64 float4_to_int64(float4 num);
extern int64 float8_to_int64(float8 num);
extern bool ge_int16_int16(int16 num1, int16 num2);
extern bool ge_int16_int32(int16 num1, int32 num2);
extern bool ge_int16_int64(int16 num1, int64 num2);
extern bool ge_int32_int16(int32 num1, int16 num2);
extern bool ge_int32_int32(int32 num1, int32 num2);
extern bool ge_int32_int64(int32 num1, int64 num2);
extern bool ge_int64_int16(int64 num1, int16 num2);
extern bool ge_int64_int32(int64 num1, int32 num2);
extern bool ge_int64_int64(int64 num1, int64 num2);
extern bool gt_int16_int16(int16 num1, int16 num2);
extern bool gt_int16_int32(int16 num1, int32 num2);
extern bool gt_int16_int64(int16 num1, int64 num2);
extern bool gt_int32_int16(int32 num1, int16 num2);
extern bool gt_int32_int32(int32 num1, int32 num2);
extern bool gt_int32_int64(int32 num1, int64 num2);
extern bool gt_int64_int16(int64 num1, int16 num2);
extern bool gt_int64_int32(int64 num1, int32 num2);
extern bool gt_int64_int64(int64 num1, int64 num2);
extern int16 int16_abs(int16 num);
extern int16 int16_and(int16 num1, int16 num2);
extern int16 int16_in(const char *str);
extern int16 int16_larger(int16 num1, int16 num2);
extern int16 int16_mod(int16 num1, int16 num2);
extern int16 int16_not(int16 num);
extern int16 int16_or(int16 num1, int16 num2);
extern char *int16_out(int16 num);
extern int16 int16_shl(int16 num1, int32 num2);
extern int16 int16_shr(int16 num1, int32 num2);
extern int16 int16_smaller(int16 num1, int16 num2);
extern int32 int16_to_int32(int16 num);
extern int64 int16_to_int64(int16 num);
extern int16 int16_uminus(int16 num);
extern int16 int16_uplus(int16 num);
extern int16 int16_xor(int16 num1, int16 num2);
extern int32 int32_abs(int32 num);
extern int32 int32_and(int32 num1, int32 num2);
extern int int32_cmp(int32 l, int32 r);
extern int32 int32_gcd(int32 num1, int32 num2);
extern int32 int32_in(const char *str);
extern int32 int32_inc(int32 num);
extern int32 int32_larger(int32 num1, int32 num2);
extern int32 int32_lcm(int32 num1, int32 num2);
extern int32 int32_mod(int32 num1, int32 num2);
extern int32 int32_not(int32 num);
extern int32 int32_or(int32 num1, int32 num2);
extern char *int32_out(int32 num);
extern int32 int32_shl(int32 num1, int32 num2);
extern int32 int32_shr(int32 num1, int32 num2);
extern int32 int32_smaller(int32 num1, int32 num2);
extern bool int32_to_bool(int32 num);
extern int16 int32_to_int16(int32 num);
extern int64 int32_to_int64(int32 num);
extern int32 int32_uminus(int32 num);
extern int32 int32_uplus(int32 num);
extern int32 int32_xor(int32 num1, int32 num2);
extern int64 int64_abs(int64 num);
extern int64 int64_and(int64 num1, int64 num2);
extern int int64_cmp(int64 l, int64 r);
extern int64 int64_dec(int64 num);
extern int64 int64_gcd(int64 num1, int64 num2);
extern int64 int64_in(const char *str);
extern int64 int64_inc(int64 num);
extern int64 int64_larger(int64 num1, int64 num2);
extern int64 int64_lcm(int64 num1, int64 num2);
extern int64 int64_mod(int64 num1, int64 num2);
extern int64 int64_not(int64 num);
extern int64 int64_or(int64 num1, int64 num2);
extern char *int64_out(int64 num);
extern int64 int64_shl(int64 num1, int32 num2);
extern int64 int64_shr(int64 num1, int32 num2);
extern int64 int64_smaller(int64 num1, int64 num2);
extern float4 int64_to_float4(int64 num);
extern float8 int64_to_float8(int64 num);
extern int16 int64_to_int16(int64 num);
extern int32 int64_to_int32(int64 num);
extern int64 int64_uminus(int64 num);
extern int64 int64_uplus(int64 num);
extern int64 int64_xor(int64 num1, int64 num2);
extern bool le_int16_int16(int16 num1, int16 num2);
extern bool le_int16_int32(int16 num1, int32 num2);
extern bool le_int16_int64(int16 num1, int64 num2);
extern bool le_int32_int16(int32 num1, int16 num2);
extern bool le_int32_int32(int32 num1, int32 num2);
extern bool le_int32_int64(int32 num1, int64 num2);
extern bool le_int64_int16(int64 num1, int16 num2);
extern bool le_int64_int32(int64 num1, int32 num2);
extern bool le_int64_int64(int64 num1, int64 num2);
extern bool lt_int16_int16(int16 num1, int16 num2);
extern bool lt_int16_int32(int16 num1, int32 num2);
extern bool lt_int16_int64(int16 num1, int64 num2);
extern bool lt_int32_int16(int32 num1, int16 num2);
extern bool lt_int32_int32(int32 num1, int32 num2);
extern bool lt_int32_int64(int32 num1, int64 num2);
extern bool lt_int64_int16(int64 num1, int16 num2);
extern bool lt_int64_int32(int64 num1, int32 num2);
extern bool lt_int64_int64(int64 num1, int64 num2);
extern int16 minus_int16_int16(int16 num1, int16 num2);
extern int32 minus_int16_int32(int16 num1, int32 num2);
extern int64 minus_int16_int64(int16 num1, int64 num2);
extern int32 minus_int32_int16(int32 num1, int16 num2);
extern int32 minus_int32_int32(int32 num1, int32 num2);
extern int64 minus_int32_int64(int32 num1, int64 num2);
extern int64 minus_int64_int16(int64 num1, int16 num2);
extern int64 minus_int64_int32(int64 num1, int32 num2);
extern int64 minus_int64_int64(int64 num1, int64 num2);
extern int16 mul_int16_int16(int16 num1, int16 num2);
extern int32 mul_int16_int32(int16 num1, int32 num2);
extern int64 mul_int16_int64(int16 num1, int64 num2);
extern int32 mul_int32_int16(int32 num1, int16 num2);
extern int32 mul_int32_int32(int32 num1, int32 num2);
extern int64 mul_int32_int64(int32 num1, int64 num2);
extern int64 mul_int64_int16(int64 num1, int16 num2);
extern int64 mul_int64_int32(int64 num1, int32 num2);
extern int64 mul_int64_int64(int64 num1, int64 num2);
extern bool ne_int16_int16(int16 num1, int16 num2);
extern bool ne_int16_int32(int16 num1, int32 num2);
extern bool ne_int16_int64(int16 num1, int64 num2);
extern bool ne_int32_int16(int32 num1, int16 num2);
extern bool ne_int32_int32(int32 num1, int32 num2);
extern bool ne_int32_int64(int32 num1, int64 num2);
extern bool ne_int64_int16(int64 num1, int16 num2);
extern bool ne_int64_int32(int64 num1, int32 num2);
extern bool ne_int64_int64(int64 num1, int64 num2);


/* Functions for numeric */

extern Numeric numeric_copy(Numeric num);
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
extern Numeric numeric_pow(Numeric num1, Numeric num2);
extern int32 pg_numeric_scale(Numeric num);
extern int32 pg_numeric_min_scale(Numeric num);
extern Numeric pg_numeric_trim_scale(Numeric num);
extern Numeric int32_to_numeric(int32 num);
extern int32 numeric_to_int32(Numeric num);
extern Numeric int64_to_numeric(int64 num);
extern int64 numeric_to_int64(Numeric num);
extern Numeric int16_to_numeric(int16 num);
extern int16 numeric_to_int16(Numeric num);
extern float4 numeric_to_float4(Numeric num);
extern float8 numeric_to_float8(Numeric num);
extern Numeric float4_to_numeric(float4 num);
extern Numeric float8_to_numeric(float8 num);

/* Functions for floats */

extern float4 add_float4_float4(float4 num1, float4 num2);
extern float8 add_float4_float8(float4 num1, float8 num2);
extern float8 add_float8_float4(float8 num1, float4 num2);
extern float8 add_float8_float8(float8 num1, float8 num2);
extern float4 div_float4_float4(float4 num1, float4 num2);
extern float8 div_float4_float8(float4 num1, float8 num2);
extern float8 div_float8_float4(float8 num1, float4 num2);
extern float8 div_float8_float8(float8 num1, float8 num2);
extern bool eq_float4_float4(float4 num1, float4 num2);
extern bool eq_float4_float8(float4 num1, float8 num2);
extern bool eq_float8_float4(float8 num1, float4 num2);
extern bool eq_float8_float8(float8 num1, float8 num2);
extern float4 float4_abs(float4 num);
extern uint32 float4_hash(float4 key);
extern uint64 float4_hash_extended(float4 key, uint64 seed);
extern float4 float4_in(const char *num);
extern float4 float4_larger(float4 num1, float4 num2);
extern char *float4_out(float4 num);
extern float4 float4_smaller(float4 num1, float4 num2);
extern float8 float4_to_float8(float4 num);
extern int16 float4_to_int16(float4 num);
extern int32 float4_to_int32(float4 num);
extern float4 float4_um(float4 num);
extern float4 float4_up(float4 num);
extern float8 float8_abs(float8 num);
extern float8 float8_acos(float8 num);
extern float8 float8_acosd(float8 num);
extern float8 float8_acosh(float8 num);
extern double float8_angular_difference(float8 degrees1, float8 degrees2);
extern float8 float8_asin(float8 num);
extern float8 float8_asind(float8 num);
extern float8 float8_asinh(float8 num);
extern float8 float8_atan(float8 num);
extern float8 float8_atan2(float8 num1, float8 num2);
extern float8 float8_atan2d(float8 num1, float8 num2);
extern float8 float8_atand(float8 num);
extern float8 float8_atanh(float8 num);
extern float8 float8_cbrt(float8 num);
extern float8 float8_ceil(float8 num);
extern float8 float8_cos(float8 num);
extern float8 float8_cosd(float8 num);
extern float8 float8_cosh(float8 num);
extern float8 float8_cot(float8 num);
extern float8 float8_cotd(float8 num);
extern float8 float8_degrees(float8 num);
extern float8 float8_exp(float8 num);
extern float8 float8_floor(float8 num);
extern float8 float8_gamma(float8 num);
extern uint32 float8_hash(float8 key);
extern uint64 float8_hash_extended(float8 key, uint64 seed);
extern float8 float8_in(const char *str);
extern float8 float8_larger(float8 num1, float8 num2);
extern float8 float8_lgamma(float8 num);
extern float8 float8_ln(float8 num);
extern float8 float8_log10(float8 num);
extern char *float8_out(float8 num, int maxdd);
extern float8 float8_pi(void);
extern float8 float8_pow(float8 num1, float8 num2);
extern float8 float8_radians(float8 num);
extern float8 float8_rint(float8 num);
extern float8 float8_round(float8 num, int maxdd);
extern float8 float8_sign(float8 num);
extern float8 float8_sin(float8 num);
extern float8 float8_sind(float8 num);
extern float8 float8_sinh(float8 num);
extern float8 float8_smaller(float8 num1, float8 num2);
extern float8 float8_sqrt(float8 num);
extern float8 float8_tan(float8 num);
extern float8 float8_tand(float8 num);
extern float8 float8_tanh(float8 num);
extern float4 float8_to_float4(float8 num);
extern int16 float8_to_int16(float8 num);
extern int32 float8_to_int32(float8 num);
extern float8 float8_trunc(float8 num);
extern float8 float8_um(float8 num);
extern float8 float8_up(float8 num);
extern int32 float8_width_bucket(float8 num, float8 bound1, float8 bound2, int32 count);
extern bool ge_float4_float4(float4 num1, float4 num2);
extern bool ge_float4_float8(float4 num1, float8 num2);
extern bool ge_float8_float4(float8 num1, float4 num2);
extern bool ge_float8_float8(float8 num1, float8 num2);
extern bool gt_float4_float4(float4 num1, float4 num2);
extern bool gt_float4_float8(float4 num1, float8 num2);
extern bool gt_float8_float4(float8 num1, float4 num2);
extern bool gt_float8_float8(float8 num1, float8 num2);
extern float4 int16_to_float4(int16 num);
extern float8 int16_to_float8(int16 num);
extern float4 int32_to_float4(int32 num);
extern float8 int32_to_float8(int32 num);
extern bool le_float4_float4(float4 num1, float4 num2);
extern bool le_float4_float8(float4 num1, float8 num2);
extern bool le_float8_float4(float8 num1, float4 num2);
extern bool le_float8_float8(float8 num1, float8 num2);
extern bool lt_float4_float4(float4 num1, float4 num2);
extern bool lt_float4_float8(float4 num1, float8 num2);
extern bool lt_float8_float4(float8 num1, float4 num2);
extern bool lt_float8_float8(float8 num1, float8 num2);
extern float4 minus_float4_float4(float4 num1, float4 num2);
extern float8 minus_float4_float8(float4 num1, float8 num2);
extern float8 minus_float8_float4(float8 num1, float4 num2);
extern float8 minus_float8_float8(float8 num1, float8 num2);
extern float4 mul_float4_float4(float4 num1, float4 num2);
extern float8 mul_float4_float8(float4 num1, float8 num2);
extern float8 mul_float8_float4(float8 num1, float4 num2);
extern float8 mul_float8_float8(float8 num1, float8 num2);
extern bool ne_float4_float4(float4 num1, float4 num2);
extern bool ne_float4_float8(float4 num1, float8 num2);
extern bool ne_float8_float4(float8 num1, float4 num2);
extern bool ne_float8_float8(float8 num1, float8 num2);
extern int pg_float4_cmp(float4 a, float4 b);
extern int pg_float8_cmp(float8 a, float8 b);
 
/* Functions for text */

extern uint32 char_hash(char c);
extern uint64 char_hash_extended(char c, uint64 seed);
extern text *cstring_to_text(const char *str);
extern text *int32_to_bin(int32 num);
extern text *int32_to_hex(int32 num);
extern text *int32_to_oct(int32 num);
extern text *int64_to_bin(int64 num);
extern text *int64_to_hex(int64 num);
extern text *int64_to_oct(int64 num);
extern text *pg_icu_unicode_version(void);
extern text *pg_text_concat(text **textarr, int count);
extern text *pg_text_concat_ws(text **textarr, int count, const text *sep);
extern bool pg_text_ge(const text *txt1, const text *txt2);
extern bool pg_text_gt(const text *txt1, const text *txt2);
extern text *pg_text_larger(const text *txt1, const text *txt2);
extern bool pg_text_le(const text *txt1, const text *txt2);
extern text *pg_text_left(const text *txt, int n);
extern bool pg_text_lt(const text *txt1, const text *txt2);
extern bool pg_text_pattern_ge(const text *txt1, const text *txt2);
extern bool pg_text_pattern_gt(const text *txt1, const text *txt2);
extern bool pg_text_pattern_le(const text *txt1, const text *txt2);
extern bool pg_text_pattern_lt(const text *txt1, const text *txt2);
extern text *pg_text_reverse(const text *txt);
extern text *pg_text_right(const text *txt, int n);
extern text *pg_text_smaller(const text *txt1, const text *txt2);
extern bool pg_text_starts_with(const text *txt1, const text *txt2);
extern text *pg_text_substr(const text *txt, int32 start, int32 length);
extern text *pg_text_substr_no_len(const text *txt, int32 start);
extern bool pg_unicode_assigned(const text *txt);
extern bool pg_unicode_is_normalized(const text *txt, const text *fmt);
extern text *pg_unicode_normalize_func(const text *txt, const text *fmt);
extern text *pg_unicode_version(void);
extern text *pg_unistr(const text *txt);
extern char *text_to_cstring(const text *txt);
extern text *text_cat(const text *txt1, const text *txt2);
extern int text_cmp(const text *txt1, const text *txt2, Oid collid);
extern text *text_copy(const text *txt);
extern bool text_eq(const text *txt1, const text *txt2);
extern uint32 text_hash(const text *txt, Oid collid);
extern uint64 text_hash_extended(const text *txt, uint64 seed, Oid collid);
extern text *text_in(const char *str);
extern text *text_initcap(const text *txt);
extern int32 text_len(const text *txt);
extern text *text_lower(const text *txt);
extern bool text_ne(const text *txt1, const text *txt2);
extern int32 text_octetlen(const text *txt);
extern char *text_out(const text *txt);
extern text *text_overlay(const text *txt1, const text *txt2, int from, int count);
extern text *text_overlay_no_len(const text *txt1, const text *txt2, int from);
extern int32 text_pos(const text *txt, const text *search);
extern text *text_upper(const text *txt);
extern text *textcat_text_text(const text *txt1, const text *txt2);

/* Functions for bytea */

extern bytea *bytea_copy(const bytea *ba);

/* Functions for date */

extern DateADT add_date_int(DateADT date, int32 days);
extern DateADT add_date_interval(DateADT date, Interval *interv);
extern int32 cmp_date_timestamp(DateADT date, Timestamp ts2);
extern int cmp_date_date(DateADT date1, DateADT date2);
extern int32 cmp_date_timestamptz(DateADT date, TimestampTz ts2);
extern int32 cmp_timestamp_date(Timestamp ts1, DateADT date);
extern int32 cmp_timestamptz_date(TimestampTz ts1, DateADT date);
extern Numeric date_extract(DateADT date, const text *units);
extern uint32 date_hash(DateADT date);
extern uint64 date_hash_extended(DateADT date, int64 seed);
extern bool date_is_finite(DateADT date);
extern Timestamp date_time_to_timestamp(DateADT date, TimeADT time);
extern Timestamp date_to_timestamp(DateADT date);
extern Timestamp date_to_timestamptz(DateADT date);
extern bool eq_date_date(DateADT date1, DateADT date2);
extern bool eq_date_timestamp(DateADT date, Timestamp ts2);
extern bool eq_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool eq_timestamp_date(Timestamp ts1, DateADT date);
extern bool eq_timestamptz_date(TimestampTz ts1, DateADT date);
extern bool ge_date_date(DateADT date1, DateADT date2);
extern bool ge_date_timestamp(DateADT date, Timestamp ts);
extern bool ge_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool ge_timestamp_date(Timestamp ts1, DateADT date);
extern bool ge_timestamptz_date(TimestampTz ts1, DateADT date);
extern bool gt_date_date(DateADT date1, DateADT date2);
extern bool gt_date_timestamp(DateADT date, Timestamp ts);
extern bool gt_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool gt_timestamp_date(Timestamp ts1, DateADT date);
extern bool gt_timestamptz_date(TimestampTz ts1, DateADT date);
extern bool le_date_date(DateADT date1, DateADT date2);
extern bool le_date_timestamp(DateADT date, Timestamp ts);
extern bool le_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool le_timestamp_date(Timestamp ts1, DateADT date);
extern bool le_timestamptz_date(TimestampTz ts1, DateADT date);
extern bool lt_date_date(DateADT date1, DateADT date2);
extern bool lt_date_timestamp(DateADT date, Timestamp ts);
extern bool lt_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool lt_timestamp_date(Timestamp ts1, DateADT date);
extern bool lt_timestamptz_date(TimestampTz ts1, DateADT date);
extern int32 minus_date_date(DateADT date1, DateADT date2);
extern DateADT minus_date_int(DateADT date, int32 days);
extern DateADT minus_date_interval(DateADT date, Interval *span);
extern bool ne_date_date(DateADT date1, DateADT date2);
extern bool ne_date_timestamp(DateADT date, Timestamp ts);
extern bool ne_date_timestamptz(DateADT date, TimestampTz ts2);
extern bool ne_timestamp_date(Timestamp ts1, DateADT date);
extern bool ne_timestamptz_date(TimestampTz ts1, DateADT date);
extern DateADT pg_date_in(const char *str);
extern DateADT pg_date_larger(DateADT date1, DateADT date2);
extern DateADT pg_date_make(int year, int mon, int mday);
extern DateADT pg_date_smaller(DateADT date1, DateADT date2);
extern char *pg_date_out(DateADT date);
extern bool time_overlaps(TimeADT ts1, TimeADT te1, TimeADT ts2, TimeADT te2);
extern DateADT timestamp_to_date(Timestamp timestamp);
extern DateADT timestamptz_to_date(TimestampTz timestamp);
 
/* Functions for time */

extern TimestampTz date_timetz_to_timestamptz(DateADT date, const TimeTzADT *timetz);
extern TimeADT interval_to_time(const Interval *interv);
extern TimeADT minus_time_interval(TimeADT time, const Interval *interv);
extern Interval *minus_time_time(TimeADT time1, TimeADT time2);
extern TimeTzADT *minus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
extern int pg_time_cmp(TimeADT time1, TimeADT time2);
extern bool pg_time_eq(TimeADT time1, TimeADT time2);
extern bool pg_time_ge(TimeADT time1, TimeADT time2);
extern bool pg_time_gt(TimeADT time1, TimeADT time2);
extern uint32 pg_time_hash(TimeADT time);
extern uint64 pg_time_hash_extended(TimeADT time, int32 seed);
extern TimeADT pg_time_in(const char *str, int32 typmod);
extern TimeADT pg_time_larger(TimeADT time1, TimeADT time2);
extern bool pg_time_le(TimeADT time1, TimeADT time2);
extern bool pg_time_lt(TimeADT time1, TimeADT time2);
extern bool pg_time_ne(TimeADT time1, TimeADT time2);
extern char *pg_time_out(TimeADT time);
extern float8 pg_time_part(TimeADT time, const text *units);
extern TimeADT pg_time_scale(TimeADT date, int32 typmod);
extern TimeADT pg_time_smaller(TimeADT time1, TimeADT time2);
extern TimeTzADT *pg_timetz_at_local(const TimeTzADT *timetz);
extern int32 pg_timetz_cmp(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern bool pg_timetz_eq(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern bool pg_timetz_ge(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern bool pg_timetz_gt(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern uint32 pg_timetz_hash(const TimeTzADT *timetz);
extern uint64 pg_timetz_hash_extended(const TimeTzADT *timetz, int64 seed);
extern TimeTzADT *pg_timetz_in(const char *str, int32 typmod);
extern TimeTzADT *pg_timetz_izone(const TimeTzADT *timetz, const Interval *zone);
extern bool pg_timetz_le(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern bool pg_timetz_lt(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern bool pg_timetz_ne(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern char *pg_timetz_out(const TimeTzADT *timetz);
extern TimeTzADT *pg_timetz_larger(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern float8 pg_timetz_part(const TimeTzADT *timetz, const text *units);
extern TimeTzADT *pg_timetz_scale(const TimeTzADT *timetz, int32 typmod);
extern TimeTzADT *pg_timetz_smaller(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
extern TimeTzADT *pg_timetz_zone(const TimeTzADT *timetz, const text *zone);
extern TimeADT plus_time_interval(TimeADT time, Interval *interv);
extern TimeTzADT *plus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
extern Numeric time_extract(TimeADT time, const text *units);
extern TimeADT time_make(int tm_hour, int tm_min, double sec);
extern Interval *time_to_interval(TimeADT time);
extern TimeTzADT *time_to_timetz(TimeADT time);
extern TimeADT timestamp_to_time(Timestamp ts);
extern TimeADT timestamptz_to_time(TimestampTz tztz);
extern TimeTzADT *timestamptz_to_timetz(TimestampTz tztz);
extern bool timetz_overlaps(const TimeTzADT *ts1, const TimeTzADT *te1, const TimeTzADT *ts2, const TimeTzADT *te2);
extern TimeADT timetz_to_time(const TimeTzADT *timetz);
extern Numeric timetz_extract(const TimeTzADT *timetz, const text *units);
 
/* Functions for timestamps */

extern Timestamp add_timestamp_interval(Timestamp ts, const Interval *interv);
extern Timestamp add_timestamptz_interval(TimestampTz tstz, const Interval *interv);
extern Timestamp add_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone);
extern int32 cmp_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern int32 cmp_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern int32 cmp_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool eq_timestamp_date(Timestamp ts, DateADT date);
extern bool eq_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool eq_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool eq_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool eq_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern TimestampTz float8_to_timestamptz(float8 seconds);
extern bool gt_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool gt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool gt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool ge_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool ge_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool ge_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool le_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool le_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool le_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool lt_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool lt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool lt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern Timestamp minus_timestamp_interval(Timestamp ts, const Interval *interv);
extern Interval *minus_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern TimestampTz minus_timestamptz_interval(TimestampTz tstz, const Interval *interv);
extern TimestampTz minus_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone);
extern Interval *minus_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2);
extern bool ne_timestamp_date(Timestamp ts, DateADT date);
extern bool ne_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool ne_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool ne_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool ne_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern Interval *pg_interval_justify_days(const Interval *interv);
extern Interval *pg_interval_justify_hours(const Interval *interv);
extern Interval *pg_interval_justify_interval(const Interval *interv);
extern Interval *pg_timestamp_age(Timestamp ts1, Timestamp ts2);
extern TimestampTz pg_timestamp_at_local(Timestamp ts);
extern Timestamp pg_timestamp_bin(Timestamp ts, const Interval *stride, Timestamp origin);
extern uint32 pg_timestamp_hash(Timestamp ts);
extern uint64 pg_timestamp_hash_extended(TimestampTz tstz, uint64 seed);
extern Timestamp pg_timestamp_in(const char *str, int32 typmod);
extern TimestampTz pg_timestamp_izone(Timestamp ts, const Interval *zone);
extern text *time_of_day(void);
extern Timestamp pg_timestamp_larger(Timestamp ts1, Timestamp ts2);
extern char *pg_timestamp_out(Timestamp ts);
extern float8 pg_timestamp_part(Timestamp ts, const text *units);
extern Timestamp pg_timestamp_scale(Timestamp ts, int32 typmod);
extern Timestamp pg_timestamp_smaller(Timestamp ts1, Timestamp ts2);
extern Timestamp pg_timestamp_trunc(Timestamp ts, const text *units);
extern TimestampTz pg_timestamp_zone(Timestamp ts, const text *zone);
extern Interval *pg_timestamptz_age(TimestampTz tstz1, TimestampTz tstz2);
extern TimestampTz pg_timestamptz_bin(TimestampTz tstz, const Interval *stride, TimestampTz origin);
extern uint32 pg_timestamptz_hash(TimestampTz tstz);
extern uint64 pg_timestamptz_hash_extended(TimestampTz tstz, uint64 seed);
extern TimestampTz pg_timestamptz_in(const char *str, int32 typmod);
extern Timestamp pg_timestamptz_izone(TimestampTz tstz, const Interval *zone);
extern char *pg_timestamptz_out(TimestampTz tstz);
extern float8 pg_timestamptz_part(TimestampTz tstz, const text *units);
extern TimestampTz pg_timestamptz_scale(TimestampTz tstz, int32 typmod);
extern TimestampTz pg_timestamptz_trunc(TimestampTz tstz, const text *units);
extern TimestampTz pg_timestamptz_trunc_zone(TimestampTz tstz, const text *units, const text *zone);
extern Timestamp pg_timestamptz_zone(TimestampTz tstz, const text *zone);
extern Numeric timestamp_extract(Timestamp ts, const text *units);
extern bool timestamp_is_finite(Timestamp ts);
extern Timestamp timestamp_make(int32 year, int32 month, int32 mday, int32 hour, int32 min, float8 sec);
extern bool timestamp_overlaps(Timestamp ts1, Timestamp te1, Timestamp ts2, Timestamp te2);
extern TimestampTz timestamp_to_timestamptz(Timestamp ts);
extern TimestampTz timestamptz_at_local(TimestampTz tstz);
extern Numeric timestamptz_extract(TimestampTz tstz, const text *units);
extern TimestampTz timestamptz_make(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec);
extern TimestampTz timestamptz_make_at_timezone(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec, const text *zone);
extern TimestampTz timestamptz_shift(TimestampTz tstz, const Interval *interv);
extern Timestamp timestamptz_to_timestamp(TimestampTz tstz);

/* Functions for intervals */

extern Interval *add_interval_interval(const Interval *interv1, const Interval *interv2);
extern Interval *div_interval_float8(const Interval *interv, float8 factor);
extern Numeric interval_extract(const Interval *interv, const text *units);
extern bool interval_is_finite(const Interval *interv);
extern Interval *interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, float8 secs);
extern Interval *interval_negate(const Interval *interv);
extern Interval *minus_interval_interval(const Interval *interv1, const Interval *interv2);
extern Interval *mul_float8_interval(float8 factor, const Interval *interv);
extern Interval *mul_interval_float8(const Interval *interv, float8 factor);
extern int32 pg_interval_cmp(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_eq(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_ge(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_gt(const Interval *interv1, const Interval *interv2);
extern uint32 pg_interval_hash(const Interval *interv);
extern uint64 pg_interval_hash_extended(const Interval *interv, uint64 seed);
extern Interval *pg_interval_in(const char *str, int32 typmod);
extern Interval *pg_interval_larger(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_le(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_lt(const Interval *interv1, const Interval *interv2);
extern bool pg_interval_ne(const Interval *interv1, const Interval *interv2);
extern char *pg_interval_out(const Interval *interv);
extern float8 pg_interval_part(const Interval *interv, const text *units);
extern Interval *pg_interval_scale(const Interval *interv, int32 typmod);
extern Interval *pg_interval_smaller(const Interval *interv1, const Interval *interv2);
extern Interval *pg_interval_trunc(const Interval *interv, const text *units);

/* Functions for JSON types */

extern Jsonb *jsonb_copy(const Jsonb *jb);

extern text **pg_json_array_elements(const text *json, int *count);
extern text **pg_json_array_elements_text(const text *json, int *count);
extern int pg_json_array_length(const text *json);
extern text **pg_json_each(const text *json, text **values, int *count);
extern text **pg_json_each_text(const text *json, text **values, int *count);
extern text *pg_json_in(const char *str);
extern text *pg_json_object(text **keyvalarr, int count);
extern text **pg_json_object_keys(const text *json, int *count);
extern text *pg_json_object_two_arg(text **keys, text **values, int count);
extern char *pg_json_out(const text *json);
extern text *pg_json_strip_nulls(const text *json, bool strip_in_arrays);
extern text *pg_json_typeof(const text *json);

extern Jsonb **pg_jsonb_array_elements(const Jsonb *jb, int *count);
extern text **pg_jsonb_array_elements_text(const Jsonb *jb, int *count);
extern int pg_jsonb_cmp(const Jsonb *jb1, const Jsonb *jb2);
extern Jsonb *pg_jsonb_concat(const Jsonb *jb1, const Jsonb *jb2);
extern bool pg_jsonb_contains(const Jsonb *jb1, const Jsonb *jb2);
extern Jsonb *pg_jsonb_delete(const Jsonb *jb, const text *key);
extern Jsonb *pg_jsonb_delete_array(const Jsonb *jb, const text **keys_elems, int keys_len);
extern Jsonb *pg_jsonb_delete_idx(const Jsonb *in, int idx);
extern Jsonb *pg_jsonb_delete_path(const Jsonb *jb, text **path_elems, int path_len);
extern text **pg_jsonb_each(const Jsonb *jb, Jsonb **values, int *count);
extern text **pg_jsonb_each_text(const Jsonb *jb, text **values, int *count);
extern bool pg_jsonb_eq(const Jsonb *jb1, const Jsonb *jb2);
extern bool pg_jsonb_exists(const Jsonb *jb, const text *key);
extern Jsonb *pg_jsonb_extract_path(const Jsonb *jb, text **path_elems, int path_len);
extern text *pg_jsonb_extract_path_text(const Jsonb *jb, text **path_elems, int path_len);
extern Jsonb *pg_jsonb_from_text(text *txt, bool unique_keys);
extern bool pg_jsonb_ge(const Jsonb *jb1, const Jsonb *jb2);
extern bool pg_jsonb_gt(const Jsonb *jb1, const Jsonb *jb2);
extern uint32 pg_jsonb_hash(const Jsonb *jb);
extern uint64 pg_jsonb_hash_extended(const Jsonb *jb, uint64 seed);
extern Jsonb * pg_jsonb_in(char *str);
extern Jsonb *pg_jsonb_insert(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool after);
extern bool pg_jsonb_le(const Jsonb *jb1, const Jsonb *jb2);
extern bool pg_jsonb_lt(const Jsonb *jb1, const Jsonb *jb2);
extern bool pg_jsonb_ne(const Jsonb *jb1, const Jsonb *jb2);
extern Jsonb *pg_jsonb_object(text **keys_vals, int count);
extern Jsonb *pg_jsonb_object_field(const Jsonb *jb, const text *key);
extern text *pg_jsonb_object_field_text(const Jsonb *jb, const text *key);
extern text **pg_jsonb_object_keys(const Jsonb *jb, int *count);
extern Jsonb *pg_jsonb_object_two_arg(text **keys, text **values, int count);
extern char *pg_jsonb_out(Jsonb *jb);
extern text *pg_jsonb_pretty(const Jsonb *jb);
extern Jsonb *pg_jsonb_set(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool create);
extern Jsonb *pg_jsonb_set_lax(const Jsonb *jb, text **path_elems, int path_len, Jsonb *newjb, bool create, const text *handle_null);
extern Jsonb *pg_jsonb_strip_nulls(const Jsonb *jb, bool strip_in_arrays);

/*****************************************************************************/

#endif /* POSTGRES_TYPES_H */
