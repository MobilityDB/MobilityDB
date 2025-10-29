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
 * @brief A simple program that tests the numeric functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o numeric_test numeric_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_float.h>
#include <pg_numeric.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create input values to test the numeric functions of the API */
  Numeric num1_in = numeric_in("1.5", -1);
  Numeric num2_in = numeric_in("2.1", -1);
  char *num1_out = numeric_out(num1_in);
  char *num2_out = numeric_out(num2_in);
  int16 int16_in = 16;
  int32 int32_in = 32;
  int64 int64_in = 64;
  float4 float4_in = 1.5;
  float8 float8_in = 1.5;

  /* Create the result types for the functions of the API */
  Numeric numeric_result;
  bool bool_result;
  uint16 int16_result;
  uint32 int32_result;
  uint64 int64_result;
  uint32 uint32_result;
  uint64 uint64_result;
  float4 float4_result;
  float8 float8_result;
  char *char_result;
  
  /* Execute and print the result of functions */

  printf("****************************************************************\n");
  printf("* Numeric *\n");
  printf("****************************************************************\n");

  /* Numeric float4_to_numeric(float4 num); */
  numeric_result = float4_to_numeric(float4_in);
  char_result = numeric_out(numeric_result);
  printf("float4_to_numeric(%f): %s\n", float4_in, char_result);
  free(numeric_result); free(char_result);

  /* Numeric float8_to_numeric(float8 num); */
  numeric_result = float8_to_numeric(float8_in);
  char_result = numeric_out(numeric_result);
  printf("float8_to_numeric(%lf): %s\n", float8_in, char_result);
  free(numeric_result); free(char_result);

  /* Numeric int16_to_numeric(int16 num); */
  numeric_result = int16_to_numeric(int16_in);
  char_result = numeric_out(numeric_result);
  printf("int16_to_numeric(%d): %s\n", int16_in, char_result);
  free(numeric_result); free(char_result);

  /* Numeric int64_to_numeric(int64 num); */
  numeric_result = int64_to_numeric(int64_in);
  char_result = numeric_out(numeric_result);
  printf("int64_to_numeric(%ld): %s\n", int64_in, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric(Numeric num, int32 typmod); */
  numeric_result = numeric(num1_in, -1);
  char_result = numeric_out(numeric_result);
  printf("numeric(%s, -1): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_abs(Numeric num); */
  numeric_result = numeric_abs(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_abs(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_add(Numeric num1, Numeric num2); */
  numeric_result = numeric_add(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_add(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_ceil(Numeric num); */
  numeric_result = numeric_ceil(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_ceil(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* int32 numeric_cmp(Numeric num1, Numeric num2); */
  int32_result = numeric_cmp(num1_in, num2_in);
  printf("numeric_cmp(%s, %s): %d\n", num1_out, num2_out, int32_result);

  /* Numeric numeric_copy(Numeric num); */
  numeric_result = numeric_copy(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_copy(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_div(Numeric num1, Numeric num2); */
  numeric_result = numeric_div(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_div(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_div_trunc(Numeric num1, Numeric num2); */
  numeric_result = numeric_div_trunc(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_div_trunc(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* bool numeric_eq(Numeric num1, Numeric num2); */
  bool_result = numeric_eq(num1_in, num2_in);
  printf("numeric_eq(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* Numeric numeric_exp(Numeric num); */
  numeric_result = numeric_exp(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_exp(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_fac(int64 num); */
  numeric_result = numeric_fac(int64_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_fac(%ld): %s\n", int64_in, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_floor(Numeric num); */
  numeric_result = numeric_floor(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_floor(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_gcd(Numeric num1, Numeric num2); */
  numeric_result = numeric_gcd(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_gcd(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* bool numeric_ge(Numeric num1, Numeric num2); */
  bool_result = numeric_ge(num1_in, num2_in);
  printf("numeric_ge(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* bool numeric_gt(Numeric num1, Numeric num2); */
  bool_result = numeric_gt(num1_in, num2_in);
  printf("numeric_gt(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* uint32 numeric_hash(Numeric key); */
  uint32_result = numeric_hash(num1_in);
  printf("numeric_hash(%s): %u\n", num1_out, uint32_result);

  /* uint64 numeric_hash_extended(Numeric num, uint64 seed); */
  uint64_result = numeric_hash_extended(num1_in, 1);
  printf("numeric_hash_extended(%s, 1): %lu\n", num1_out, uint64_result);

  /* Numeric numeric_inc(Numeric num); */
  numeric_result = numeric_inc(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_inc(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_larger(Numeric num1, Numeric num2); */
  numeric_result = numeric_larger(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_larger(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_lcm(Numeric num1, Numeric num2); */
  numeric_result = numeric_lcm(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_lcm(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* bool numeric_le(Numeric num1, Numeric num2); */
  bool_result = numeric_le(num1_in, num2_in);
  printf("numeric_le(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* Numeric numeric_ln(Numeric num); */
  numeric_result = numeric_ln(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_ln(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_log(Numeric num1, Numeric num2); */
  numeric_result = numeric_log(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_log(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* bool numeric_lt(Numeric num1, Numeric num2); */
  bool_result = numeric_lt(num1_in, num2_in);
  printf("numeric_lt(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* uint32 numeric_min_scale(Numeric num); */
  int32_result = numeric_min_scale(num1_in);
  printf("numeric_min_scale(%s): %d\n", num1_out, int32_result);

  /* Numeric numeric_minus(Numeric num1, Numeric num2); */
  numeric_result = numeric_minus(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_minus(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_mod(Numeric num1, Numeric num2); */
  numeric_result = numeric_mod(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_mod(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_mul(Numeric num1, Numeric num2); */
  numeric_result = numeric_mul(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_mul(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);
  
  /* bool numeric_ne(Numeric num1, Numeric num2); */
  bool_result = numeric_ne(num1_in, num2_in);
  printf("numeric_ne(%s, %s): %c\n", num1_out, num2_out, bool_result ? 't' : 'f');

  /* Numeric numeric_pow(Numeric num1, Numeric num2); */
  numeric_result = numeric_pow(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_pow(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_round(Numeric num, int32 scale); */
  numeric_result = numeric_round(num1_in, 2);
  char_result = numeric_out(numeric_result);
  printf("numeric_round(%s, 2): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* int32 numeric_scale(Numeric num); */
  int32_result = numeric_scale(num1_in);
  printf("numeric_scale(%s): %d\n", num1_out, int32_result);

  /* Numeric numeric_sign(Numeric num); */
  numeric_result = numeric_sign(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_sign(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_smaller(Numeric num1, Numeric num2); */
  numeric_result = numeric_smaller(num1_in, num2_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_smaller(%s, %s): %s\n", num1_out, num2_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_sqrt(Numeric num); */
  numeric_result = numeric_sqrt(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_sqrt(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* float4 numeric_to_float4(Numeric num); */
  float4_result = numeric_to_float4(num1_in);
  printf("numeric_to_float4(%s): %f\n", num1_out, float4_result);

  /* float8 numeric_to_float8(Numeric num); */
  float8_result = numeric_to_float8(num1_in);
  printf("numeric_to_float8(%s): %lf\n", num1_out, float8_result);

  /* int16 numeric_to_int16(Numeric num); */
  int16_result = numeric_to_int16(num1_in);
  printf("numeric_to_int16(%s): %d\n", num1_out, int16_result);

  /* int32 numeric_to_int32(Numeric num); */
  int32_result = numeric_to_int32(num1_in);
  printf("numeric_to_int32(%s): %d\n", num1_out, int32_result);

  /* int64 numeric_to_int64(Numeric num); */
  int64_result = numeric_to_int64(num1_in);
  printf("numeric_to_int64(%s): %ld\n", num1_out, int64_result);

  /* Numeric numeric_trim_scale(Numeric num); */
  numeric_result = numeric_trim_scale(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_trim_scale(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_trunc(Numeric num, int32 scale); */
  numeric_result = numeric_trunc(num1_in, 2);
  char_result = numeric_out(numeric_result);
  printf("numeric_trunc(%s, 2): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_uminus(Numeric num); */
  numeric_result = numeric_uminus(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_uminus(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* Numeric numeric_uplus(Numeric num); */
  numeric_result = numeric_uplus(num1_in);
  char_result = numeric_out(numeric_result);
  printf("numeric_uplus(%s): %s\n", num1_out, char_result);
  free(numeric_result); free(char_result);

  /* int32 numeric_width_bucket(Numeric operand, Numeric bound1, Numeric bound2, int32 count); */
  Numeric bound1 = numeric_in("1", -1);
  Numeric bound2 = numeric_in("10", -1);
  char *bound1_out = numeric_out(bound1);
  char *bound2_out = numeric_out(bound2);
  int32_result = numeric_width_bucket(num1_in, bound1, bound2, int32_in);
  printf("numeric_width_bucket(%s, %s, %s, %d): %d\n", num1_out, bound1_out, bound2_out, int32_in, int32_result);
  free(bound1); free(bound2);
  free(bound1_out); free(bound2_out);

  printf("****************************************************************\n");

  /* Clean up */
  free(num1_in); free(num2_in);
  free(num1_out); free(num2_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
