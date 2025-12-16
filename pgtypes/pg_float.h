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

#ifndef __PG_FLOAT_H__
#define __PG_FLOAT_H__

typedef float float4;
typedef double float8;

/*****************************************************************************/

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
extern int float4_cmp(float4 num1, float4 num2);
extern uint32 float4_hash(float4 num);
extern uint64 float4_hash_extended(float4 num, uint64 seed);
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
extern int float8_cmp(float8 num1, float8 num2);
extern float8 float8_cos(float8 num);
extern float8 float8_cosd(float8 num);
extern float8 float8_cosh(float8 num);
extern float8 float8_cot(float8 num);
extern float8 float8_cotd(float8 num);
extern float8 float8_degrees(float8 num);
extern float8 float8_exp(float8 num);
extern float8 float8_floor(float8 num);
extern float8 float8_gamma(float8 num);
extern uint32 float8_hash(float8 num);
extern uint64 float8_hash_extended(float8 num, uint64 seed);
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

/*****************************************************************************/

#endif /* __PG_FLOAT_H__ */
