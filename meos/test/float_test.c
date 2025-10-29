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
 * @brief A simple program that tests the float functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o float_test float_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_float.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create two boolean values to test the bool functions of the API */
  int16 int16_in1 = 16;
  int32 int32_in1 = 32;
  float4 float4_in1 = float4_in("4.5");
  float4 float4_in2 = float4_in("3.5");
  float8 float8_in1 = float8_in("8.5");
  float8 float8_in2 = float8_in("7.5");

  /* Create the result types for the bool functions of the API */
  bool bool_result;
  int16 int16_result;
  int32 int32_result;
  uint32 uint32_result;
  uint64 uint64_result;
  float4 float4_result;
  float8 float8_result;
  char *char_result;
  
  /* Execute and print the result for the bool functions of the API */

  printf("****************************************************************\n");
  printf("* Float *\n");
  printf("****************************************************************\n");

  /* float4 add_float4_float4(float4 num1, float4 num2); */
  float4_result = add_float4_float4(float4_in1, float4_in2);
  printf("add_float4_float4(%f, %f): %f\n", float4_in1, float4_in2, float4_result);

  /* float8 add_float4_float8(float4 num1, float8 num2); */
  float8_result = add_float4_float8(float4_in1, float8_in2);
  printf("add_float4_float8(%f, %lf): %lf\n", float4_in1, float8_in2, float8_result);

  /* float8 add_float8_float4(float8 num1, float4 num2); */
  float8_result = add_float8_float4(float8_in1, float4_in2);
  printf("add_float8_float4(%lf, %f): %lf\n", float8_in1, float4_in2, float8_result);

  /* float8 add_float8_float8(float8 num1, float8 num2); */
  float8_result = add_float8_float8(float8_in1, float8_in2);
  printf("add_float8_float8(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float4 div_float4_float4(float4 num1, float4 num2); */
  float4_result = div_float4_float4(float4_in1, float4_in2);
  printf("div_float4_float4(%f, %f): %f\n", float4_in1, float4_in2, float4_result);

  /* float8 div_float4_float8(float4 num1, float8 num2); */
  float8_result = div_float4_float8(float4_in1, float8_in2);
  printf("div_float4_float8(%f, %lf): %lf\n", float4_in1, float8_in2, float8_result);

  /* float8 div_float8_float4(float8 num1, float4 num2); */
  float8_result = div_float8_float4(float8_in1, float4_in2);
  printf("div_float8_float4(%lf, %f): %lf\n", float8_in1, float4_in2, float8_result);

  /* float8 div_float8_float8(float8 num1, float8 num2); */
  float8_result = div_float8_float8(float8_in1, float8_in2);
  printf("div_float8_float8(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* bool eq_float4_float4(float4 num1, float4 num2); */
  bool_result = eq_float4_float4(float4_in1, float4_in2);
  printf("eq_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool eq_float4_float8(float4 num1, float8 num2); */
  bool_result = eq_float4_float8(float4_in1, float8_in2);
  printf("eq_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool eq_float8_float4(float8 num1, float4 num2); */
  bool_result = eq_float8_float4(float8_in1, float4_in2);
  printf("eq_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool eq_float8_float8(float8 num1, float8 num2); */
  bool_result = eq_float8_float8(float8_in1, float8_in2);
  printf("eq_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  /* float4 float4_abs(float4 num); */
  float4_result = float4_abs(float4_in1);
  printf("float4_abs(%f): %f\n", float4_in1, float4_result);

  /* int float4_cmp(float4 num1, float4 num2); */
  int32_result = float4_cmp(float4_in1, float4_in2);
  printf("float4_cmp(%f, %f): %d\n", float4_in1, float4_in2, int32_result);

  /* float4 float4_hash(float4 num); */
  uint32_result = float4_hash(float4_in1);
  printf("float4_hash(%f): %u\n", float4_in1, uint32_result);

  /* float4 float4_hash_extended(float4 num, uint64 seed); */
  uint64_result = float4_hash_extended(float4_in1, 1);
  printf("float4_hash_extended(%f, 1): %lu\n", float4_in1, uint64_result);

  /* float4 float4_in(const char *num); */
  float4_result = float4_in("1.23456789");
  printf("float4_in(1.23456789): %f\n", float4_result);

  /* float4 float4_larger(float4 num1, float4 num2); */
  float4_result = float4_larger(float4_in1, float4_in2);
  printf("float4_larger(%f, %f): %f\n", float4_in1, float4_in2, float4_result);

  /* char *float4_out(float4 num); */
  char_result = float4_out(float4_in1);
  printf("float4_out(%f): %s\n", float4_in1, char_result);
  free(char_result);

  /* float4 float4_smaller(float4 num1, float4 num2); */
  float4_result = float4_smaller(float4_in1, float4_in2);
  printf("float4_smaller(%f, %f): %f\n", float4_in1, float4_in2, float4_result);

  /* float8 float4_to_float8(float4 num); */
  float8_result = float4_to_float8(float4_in1);
  printf("float4_to_float8(%f): %lf\n", float4_in1, float8_result);

  /* int16 float4_to_int16(float4 num); */
  int16_result = float4_to_int16(float4_in1);
  printf("float4_to_int16(%f): %d\n", float4_in1, int16_result);

  /* int32 float4_to_int32(float4 num); */
  int32_result = float4_to_int32(float4_in1);
  printf("float4_to_int32(%f): %d\n", float4_in1, int32_result);

  /* float4 float4_um(float4 num); */
  float4_result = float4_um(float4_in1);
  printf("float4_um(%f): %f\n", float4_in1, float4_result);

  /* float4 float4_up(float4 num); */
  float4_result = float4_up(float4_in1);
  printf("float4_up(%f): %f\n", float4_in1, float4_result);

  /* float8 float8_abs(float8 num); */
  float8_result = float8_abs(float8_in1);
  printf("float8_abs(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_acos(float8 num); */
  float8_result = float8_acos(0.5);
  printf("float8_acos(0.5): %lf\n", float8_result);

  /* float8 float8_acosd(float8 num);  */
  float8_result = float8_acosd(0.5);
  printf("float8_acosd(0.5): %lf\n", float8_result);

  /* float8 float8_acosh(float8 num);  */
  float8_result = float8_acosh(float8_in1);
  printf("float8_acosh(%lf): %lf\n", float8_in1, float8_result);

  /* double float8_angular_difference(float8 degrees1, float8 degrees2); */
  float8_result = float8_angular_difference(float8_in1, float8_in2);
  printf("float8_angular_difference(%f, %f): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_asin(float8 num); */
  float8_result = float8_asin(0.5);
  printf("float8_asin(0.5): %lf\n", float8_result);

  /* float8 float8_asind(float8 num); */
  float8_result = float8_asind(0.5);
  printf("float8_asind(0.5): %lf\n", float8_result);

  /* float8 float8_asinh(float8 num); */
  float8_result = float8_asinh(float8_in1);
  printf("float8_asinh(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_atan(float8 num); */
  float8_result = float8_atan(float8_in1);
  printf("float8_atan(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_atan2(float8 num1, float8 num2); */
  float8_result = float8_atan2(float8_in1, float8_in2);
  printf("float8_atan2(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_atan2d(float8 num1, float8 num2); */
  float8_result = float8_atan2d(float8_in1, float8_in2);
  printf("float8_atan2d(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_atand(float8 num);  */
  float8_result = float8_atand(float8_in1);
  printf("float8_atand(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_atanh(float8 num); */
  float8_result = float8_atanh(0.5);
  printf("float8_atanh(0.5): %lf\n", float8_result);

  /* float8 float8_cbrt(float8 num); */
  float8_result = float8_cbrt(float8_in1);
  printf("float8_cbrt(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_ceil(float8 num); */
  float8_result = float8_ceil(float8_in1);
  printf("float8_ceil(%lf): %lf\n", float8_in1, float8_result);

  /* int float8_cmp(float8 num1, float8 num2); */
  int32_result = float8_cmp(float8_in1, float8_in2);
  printf("float8_cmp(%lf, %lf): %d\n", float8_in1, float8_in2, int32_result);

  /* float8 float8_cos(float8 num); */
  float8_result = float8_cos(float8_in1);
  printf("float8_cos(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_cosd(float8 num); */
  float8_result = float8_cosd(float8_in1);
  printf("float8_cosd(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_cosh(float8 num); */
  float8_result = float8_cosh(float8_in1);
  printf("float8_cosh(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_cot(float8 num); */
  float8_result = float8_cot(float8_in1);
  printf("float8_cot(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_cotd(float8 num); */
  float8_result = float8_cotd(float8_in1);
  printf("float8_cotd(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_degrees(float8 num); */
  float8_result = float8_degrees(float8_in1);
  printf("float8_degrees(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_exp(float8 num); */
  float8_result = float8_exp(float8_in1);
  printf("float8_exp(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_floor(float8 num); */
  float8_result = float8_floor(float8_in1);
  printf("float8_floor(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_gamma(float8 num); */
  float8_result = float8_gamma(float8_in1);
  printf("float8_gamma(%lf): %lf\n", float8_in1, float8_result);

  /* uint32 float8_hash(float8 num); */
  uint32_result = float8_hash(float8_in1);
  printf("float8_hash(%lf): %u\n", float8_in1, uint32_result);

  /* uint64 float8_hash_extended(float8 num, uint64 seed); */
  uint64_result = float8_hash_extended(float8_in1, 1);
  printf("float8_hash_extended(%lf, 1): %lu\n", float8_in1, uint64_result);

  /* float8 float8_in(const char *str); */
  float8_result = float8_in("1.23456789");
  printf("float8_in(1.23456789): %lf\n", float8_result);

  /* float8 float8_larger(float8 num1, float8 num2); */
  float8_result = float8_larger(float8_in1, float8_in2);
  printf("float8_larger(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_lgamma(float8 num); */
  float8_result = float8_lgamma(float8_in1);
  printf("float8_lgamma(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_ln(float8 num); */
  float8_result = float8_ln(float8_in1);
  printf("float8_ln(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_log10(float8 num); */
  float8_result = float8_log10(float8_in1);
  printf("float8_log10(%lf): %lf\n", float8_in1, float8_result);

  /* char *float8_out(float8 num, int maxdd); */
  char_result = float8_out(float8_in1, 6);
  printf("float8_out(%lf, 6): %s\n", float8_in1, char_result);
  free(char_result);

  /* float8 float8_pi(void); */
  float8_result = float8_pi();
  printf("float8_pi(): %lf\n", float8_result);

  /* float8 float8_pow(float8 num1, float8 num2); */
  float8_result = float8_pow(float8_in1, float8_in2);
  printf("float8_pow(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_radians(float8 num); */
  float8_result = float8_radians(float8_in1);
  printf("float8_radians(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_rint(float8 num); */
  float8_result = float8_rint(float8_in1);
  printf("float8_rint(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_round(float8 num, int maxdd); */
  float8_result = float8_round(float8_in1, 6);
  printf("float8_round(%lf, 6): %lf\n", float8_in1, float8_result);

  /* float8 float8_sign(float8 num); */
  float8_result = float8_sign(float8_in1);
  printf("float8_sign(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_sin(float8 num); */
  float8_result = float8_sin(float8_in1);
  printf("float8_sin(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_sind(float8 num); */
  float8_result = float8_sind(float8_in1);
  printf("float8_sind(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_sinh(float8 num); */
  float8_result = float8_sinh(float8_in1);
  printf("float8_sinh(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_smaller(float8 num1, float8 num2); */
  float8_result = float8_smaller(float8_in1, float8_in2);
  printf("float8_smaller(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float8 float8_sqrt(float8 num); */
  float8_result = float8_sqrt(float8_in1);
  printf("float8_sqrt(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_tan(float8 num); */
  float8_result = float8_tan(float8_in1);
  printf("float8_tan(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_tand(float8 num); */
  float8_result = float8_tand(float8_in1);
  printf("float8_tand(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_tanh(float8 num); */
  float8_result = float8_tanh(float8_in1);
  printf("float8_tanh(%lf): %lf\n", float8_in1, float8_result);

  /* float4 float8_to_float4(float8 num); */
  float4_result = float8_to_float4(float8_in1);
  printf("float8_to_float4(%lf): %f\n", float8_in1, float4_result);

  /* int16 float8_to_int16(float8 num); */
  int16_result = float8_to_int16(float8_in1);
  printf("float8_to_int16(%lf): %d\n", float8_in1, int16_result);

  /* int32 float8_to_int32(float8 num); */
  int32_result = float8_to_int32(float8_in1);
  printf("float8_to_int32(%lf): %d\n", float8_in1, int32_result);

  /* float8 float8_trunc(float8 num); */
  float8_result = float8_trunc(float8_in1);
  printf("float8_trunc(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_um(float8 num); */
  float8_result = float8_um(float8_in1);
  printf("float8_um(%lf): %lf\n", float8_in1, float8_result);

  /* float8 float8_up(float8 num); */
  float8_result = float8_up(float8_in1);
  printf("float8_up(%lf): %lf\n", float8_in1, float8_result);

  /* int32 float8_width_bucket(float8 num, float8 bound1, float8 bound2, int32 count); */
  int32_result = float8_width_bucket(float8_in1, 1, 10, 5);
  printf("float8_width_bucket(%lf, 1, 10, 5): %d\n", float8_in1, int32_result);

  /* bool ge_float4_float4(float4 num1, float4 num2); */
  bool_result = ge_float4_float4(float4_in1, float4_in2);
  printf("ge_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool ge_float4_float8(float4 num1, float8 num2); */
  bool_result = ge_float4_float8(float4_in1, float8_in2);
  printf("ge_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool ge_float8_float4(float8 num1, float4 num2); */
  bool_result = ge_float8_float4(float8_in1, float4_in2);
  printf("ge_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool ge_float8_float8(float8 num1, float8 num2); */
  bool_result = ge_float8_float8(float8_in1, float8_in2);
  printf("ge_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool gt_float4_float4(float4 num1, float4 num2); */
  bool_result = gt_float4_float4(float4_in1, float4_in2);
  printf("gt_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool gt_float4_float8(float4 num1, float8 num2); */
  bool_result = gt_float4_float8(float4_in1, float8_in2);
  printf("gt_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool gt_float8_float4(float8 num1, float4 num2); */
  bool_result = gt_float8_float4(float8_in1, float4_in2);
  printf("gt_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool gt_float8_float8(float8 num1, float8 num2); */
  bool_result = gt_float8_float8(float8_in1, float8_in2);
  printf("gt_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  /* float4 int16_to_float4(int16 num); */
  float4_result = int16_to_float4(int16_in1);
  printf("int16_to_float4(%d): %f\n", int16_in1, float4_result);

  /* float8 int16_to_float8(int16 num); */
  float8_result = int16_to_float8(int16_in1);
  printf("int16_to_float8(%d): %lf\n", int16_in1, float8_result);

  /* float4 int32_to_float4(int32 num); */
  float4_result = int32_to_float4(int32_in1);
  printf("int32_to_float4(%d): %f\n", int32_in1, float4_result);

  /* float8 int32_to_float8(int32 num); */
  float8_result = int32_to_float8(int32_in1);
  printf("int32_to_float8(%d): %lf\n", int32_in1, float8_result);

  /* bool le_float4_float4(float4 num1, float4 num2); */
  bool_result = le_float4_float4(float4_in1, float4_in2);
  printf("le_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool le_float4_float8(float4 num1, float8 num2); */
  bool_result = le_float4_float8(float4_in1, float8_in2);
  printf("le_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool le_float8_float4(float8 num1, float4 num2); */
  bool_result = le_float8_float4(float8_in1, float4_in2);
  printf("le_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool le_float8_float8(float8 num1, float8 num2); */
  bool_result = le_float8_float8(float8_in1, float8_in2);
  printf("le_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool lt_float4_float4(float4 num1, float4 num2); */
  bool_result = lt_float4_float4(float4_in1, float4_in2);
  printf("lt_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool lt_float4_float8(float4 num1, float8 num2); */
  bool_result = lt_float4_float8(float4_in1, float8_in2);
  printf("lt_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool lt_float8_float4(float8 num1, float4 num2); */
  bool_result = lt_float8_float4(float8_in1, float4_in2);
  printf("lt_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool lt_float8_float8(float8 num1, float8 num2); */
  bool_result = lt_float8_float8(float8_in1, float8_in2);
  printf("lt_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  /* float4 minus_float4_float4(float4 num1, float4 num2); */
  float4_result = minus_float4_float4(float4_in1, float4_in2);
  printf("minus_float4_float4(%f, %f): %f\n", float4_in1, float4_in2, float4_result);

  /* float8 minus_float4_float8(float4 num1, float8 num2); */
  float8_result = minus_float4_float8(float4_in1, float8_in2);
  printf("minus_float4_float8(%f, %lf): %lf\n", float4_in1, float8_in2, float8_result);

  /* float8 minus_float8_float4(float8 num1, float4 num2); */
  float8_result = minus_float8_float4(float8_in1, float4_in2);
  printf("minus_float8_float4(%lf, %f): %lf\n", float8_in1, float4_in2, float8_result);

  /* float8 minus_float8_float8(float8 num1, float8 num2); */
  float8_result = minus_float8_float8(float8_in1, float8_in2);
  printf("minus_float8_float8(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* float4 mul_float4_float4(float4 num1, float4 num2); */
  float4_result = mul_float4_float4(float4_in1, float4_in2);
  printf("mul_float4_float4(%f, %f): %lf\n", float4_in1, float4_in2, float8_result);

  /* float8 mul_float4_float8(float4 num1, float8 num2); */
  float8_result = mul_float4_float8(float4_in1, float8_in2);
  printf("mul_float4_float8(%f, %lf): %lf\n", float4_in1, float8_in2, float8_result);

  /* float8 mul_float8_float4(float8 num1, float4 num2); */
  float8_result = mul_float8_float4(float8_in1, float4_in2);
  printf("mul_float8_float4(%lf, %f): %lf\n", float8_in1, float4_in2, float8_result);

  /* float8 mul_float8_float8(float8 num1, float8 num2); */
  float8_result = mul_float8_float8(float8_in1, float8_in2);
  printf("mul_float8_float8(%lf, %lf): %lf\n", float8_in1, float8_in2, float8_result);

  /* bool ne_float4_float4(float4 num1, float4 num2); */
  bool_result = ne_float4_float4(float4_in1, float4_in2);
  printf("ne_float4_float4(%f, %f): %c\n", float4_in1, float4_in2, bool_result ? 't' : 'f');

  /* bool ne_float4_float8(float4 num1, float8 num2); */
  bool_result = ne_float4_float8(float4_in1, float8_in2);
  printf("ne_float4_float8(%f, %lf): %c\n", float4_in1, float8_in2, bool_result ? 't' : 'f');

  /* bool ne_float8_float4(float8 num1, float4 num2); */
  bool_result = ne_float8_float4(float8_in1, float4_in2);
  printf("ne_float8_float4(%lf, %f): %c\n", float8_in1, float4_in2, bool_result ? 't' : 'f');

  /*  bool ne_float8_float8(float8 num1, float8 num2); */
  bool_result = ne_float8_float8(float8_in1, float8_in2);
  printf("ne_float8_float8(%lf, %lf): %c\n", float8_in1, float8_in2, bool_result ? 't' : 'f');

  printf("****************************************************************\n");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
