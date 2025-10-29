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

#ifndef PG_INT_H
#define PG_INT_H

typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef float float4;
typedef double float8;

/*****************************************************************************/

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
extern bool eq_int16_int16(int16 num1, int16 num2);
extern bool eq_int16_int32(int16 num1, int32 num2);
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
extern uint32 int16_hash(int16 val);
extern uint64 int16_hash_extended(int16 val, uint64 seed);
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
extern uint32 int32_hash(int32 val);
extern uint64 int32_hash_extended(int32 val, uint64 seed);
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
extern uint32 int64_hash(int64 num);
extern uint64 int64_hash_extended(int64 num, uint64 seed);
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

/*****************************************************************************/

#endif /* PG_INT_H */
