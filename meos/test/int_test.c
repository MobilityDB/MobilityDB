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
 * @brief A simple program that tests the integer functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o int_test int_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_int.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create input values to test the integer functions of the API */
  int16 int16_in1 = 16;
  int16 int16_in2 = 17;
  int32 int32_in1 = 32;
  int32 int32_in2 = 33;
  int64 int64_in1 = 64;
  int64 int64_in2 = 65;
  bool bool_in1 = true;
  float4 float4_in1 = 1.5;
  float8 float8_in1 = 2.5;

  /* Create the result types */
  bool bool_result;
  int16 int16_result;
  int32 int32_result;
  int64 int64_result;
  uint32 uint32_result;
  uint64 uint64_result;
  float4 float4_result;
  float8 float8_result;
  char *char_result;
  
  /* Execute the functions and print the result */

  printf("****************************************************************\n");
  printf("* Integer *\n");
  printf("****************************************************************\n");

  /* int16 add_int16_int16(int16 num1, int16 num2); */
  int16_result = add_int16_int16(int16_in1, int16_in2);
  printf("add_int16_int16(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 add_int16_int32(int16 num1, int32 num2); */
  int32_result = add_int16_int32(int16_in1, int32_in2);
  printf("add_int16_int32(%d, %d): %d\n", int16_in1, int32_in2, int32_result);

  /* int64 add_int16_int64(int16 num1, int64 num2); */
  int64_result = add_int16_int64(int16_in1, int64_in2);
  printf("add_int16_int64(%d, %ld): %ld\n", int16_in1, int64_in2, int64_result);

  /* int32 add_int32_int16(int32 num1, int16 num2); */
  int32_result = add_int32_int16(int32_in1, int16_in2);
  printf("add_int32_int16(%d, %d): %d\n", int32_in1, int16_in2, int32_result);

  /* int32 add_int32_int32(int32 num1, int32 num2); */
  int32_result = add_int32_int32(int32_in1, int32_in2);
  printf("add_int32_int32(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int64 add_int32_int64(int32 num1, int64 num2); */
  int64_result = add_int32_int64(int32_in1, int64_in2);
  printf("add_int32_int64(%d, %ld): %ld\n", int32_in1, int64_in2, int64_result);

  /* int64 add_int64_int16(int64 num1, int16 num2); */
  int64_result = add_int64_int16(int64_in1, int16_in2);
  printf("add_int64_int16(%ld, %d): %ld\n", int64_in1, int16_in2, int64_result);

  /* int64 add_int64_int32(int64 num1, int32 num2); */
  int64_result = add_int64_int32(int64_in1, int32_in2);
  printf("add_int64_int32(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 add_int64_int64(int64 num1, int64 num2); */
  int64_result = add_int64_int64(int64_in1, int64_in2);
  printf("add_int64_int64(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* int32 bool_to_int32(bool b); */
  int32_result = bool_to_int32(bool_in1);
  printf("bool_to_int32(%c): %d\n", bool_in1 ? 't' : 'f', int32_result);

  /* int16 div_int16_int16(int16 num1, int16 num2); */
  int16_result = div_int16_int16(int16_in1, int16_in2);
  printf("div_int16_int16(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 div_int16_int32(int16 num1, int32 num2); */
  int32_result = div_int16_int32(int16_in1, int32_in2);
  printf("div_int16_int32(%d, %d): %d\n", int16_in1, int32_in2, int32_result);

  /* int64 div_int16_int64(int16 num1, int64 num2); */
  int64_result = div_int16_int64(int16_in1, int64_in2);
  printf("div_int16_int64(%d, %ld): %ld\n", int16_in1, int64_in2, int64_result);

  /* int32 div_int32_int16(int32 num1, int16 num2); */
  int32_result = div_int32_int16(int32_in1, int16_in2);
  printf("div_int32_int16(%d, %d): %d\n", int32_in1, int16_in2, int32_result);

  /* int32 div_int32_int32(int32 num1, int32 num2); */
  int32_result = div_int32_int32(int32_in1, int32_in2);
  printf("div_int32_int32(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int64 div_int32_int64(int32 num1, int64 num2); */
  int64_result = div_int32_int64(int32_in1, int64_in2);
  printf("div_int32_int64(%d, %ld): %ld\n", int32_in1, int64_in2, int64_result);

  /* int64 div_int64_int16(int64 num1, int16 num2); */
  int64_result = div_int64_int16(int64_in1, int16_in2);
  printf("div_int64_int16(%ld, %d); %ld\n", int64_in1, int16_in2, int64_result);

  /* int64 div_int64_int32(int64 num1, int32 num2); */
  int64_result = div_int64_int32(int64_in1, int32_in2);
  printf("div_int64_int32(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 div_int64_int64(int64 num1, int64 num2); */
  int64_result = div_int64_int64(int64_in1, int64_in2);
  printf("div_int64_int64(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* bool eq_int16_int16(int16 num1, int16 num2); */
  bool_result = eq_int16_int16(int16_in1, int16_in2);
  printf("eq_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool eq_int16_int32(int16 num1, int32 num2); */
  bool_result = eq_int16_int32(int16_in1, int32_in2);
  printf("eq_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool eq_int16_int64(int16 num1, int64 num2); */
  bool_result = eq_int16_int64(int16_in1, int64_in2);
  printf("eq_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool eq_int32_int16(int32 num1, int16 num2); */
  bool_result = eq_int32_int16(int32_in1, int16_in2);
  printf("eq_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool eq_int32_int32(int32 num1, int32 num2); */
  bool_result = eq_int32_int32(int32_in1, int32_in2);
  printf("eq_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool eq_int32_int64(int32 num1, int64 num2); */
  bool_result = eq_int32_int64(int32_in1, int64_in2);
  printf("eq_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool eq_int64_int16(int64 num1, int16 num2); */
  bool_result = eq_int64_int16(int64_in1, int16_in2);
  printf("eq_int64_int16(%ld, %d) %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool eq_int64_int32(int64 num1, int32 num2); */
  bool_result = eq_int64_int32(int64_in1, int32_in2);
  printf("eq_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool eq_int64_int64(int64 num1, int64 num2); */
  bool_result = eq_int64_int64(int64_in1, int64_in2);
  printf("eq_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  /* int64 float4_to_int64(float4 num); */
  int64_result = float4_to_int64(float4_in1);
  printf("float4_to_int64(%f): %ld\n", float4_in1, int64_result);

  /* int64 float8_to_int64(float8 num); */
  int64_result = float8_to_int64(float8_in1);
  printf("float8_to_int64(%lf): %ld\n", float8_in1, int64_result);

  /* bool ge_int16_int16(int16 num1, int16 num2); */
  bool_result = ge_int16_int16(int16_in1, int16_in2);
  printf("ge_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ge_int16_int32(int16 num1, int32 num2); */
  bool_result = ge_int16_int32(int16_in1, int32_in2);
  printf("ge_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ge_int16_int64(int16 num1, int64 num2); */
  bool_result = ge_int16_int64(int16_in1, int64_in2);
  printf("ge_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool ge_int32_int16(int32 num1, int16 num2); */
  bool_result = ge_int32_int16(int32_in1, int16_in2);
  printf("ge_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ge_int32_int32(int32 num1, int32 num2); */
  bool_result = ge_int32_int32(int32_in1, int32_in2);
  printf("ge_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ge_int32_int64(int32 num1, int64 num2); */
  bool_result = ge_int32_int64(int32_in1, int64_in2);
  printf("ge_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool ge_int64_int16(int64 num1, int16 num2); */
  bool_result = ge_int64_int16(int64_in1, int16_in2);
  printf("ge_int64_int16(%ld, %d): %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ge_int64_int32(int64 num1, int32 num2); */
  bool_result = ge_int64_int32(int64_in1, int32_in2);
  printf("ge_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ge_int64_int64(int64 num1, int64 num2); */
  bool_result = ge_int64_int64(int64_in1, int64_in2);
  printf("ge_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool gt_int16_int16(int16 num1, int16 num2); */
  bool_result = gt_int16_int16(int16_in1, int16_in2);
  printf("gt_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool gt_int16_int32(int16 num1, int32 num2); */
  bool_result = gt_int16_int32(int16_in1, int32_in2);
  printf("gt_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool gt_int16_int64(int16 num1, int64 num2); */
  bool_result = gt_int16_int64(int16_in1, int64_in2);
  printf("gt_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool gt_int32_int16(int32 num1, int16 num2); */
  bool_result = gt_int32_int16(int32_in1, int16_in2);
  printf("gt_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool gt_int32_int32(int32 num1, int32 num2); */
  bool_result = gt_int32_int32(int32_in1, int32_in2);
  printf("gt_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool gt_int32_int64(int32 num1, int64 num2); */
  bool_result = gt_int32_int64(int32_in1, int64_in2);
  printf("gt_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool gt_int64_int16(int64 num1, int16 num2); */
  bool_result = gt_int64_int16(int64_in1, int16_in2);
  printf("gt_int64_int16(%ld, %d): %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool gt_int64_int32(int64 num1, int32 num2); */
  bool_result = gt_int64_int32(int64_in1, int32_in2);
  printf("gt_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool gt_int64_int64(int64 num1, int64 num2); */
  bool_result = gt_int64_int64(int64_in1, int64_in2);
  printf("gt_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  /* int16 int16_abs(int16 num); */
  int16_result = int16_abs(int16_in1);
  printf("int16_abs(%d): %d\n", int16_in1, int16_result);

  /* int16 int16_and(int16 num1, int16 num2); */
  int16_result = int16_and(int16_in1, int16_in2);
  printf("int16_and(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* uint32 int16_hash(int16 val); */
  uint32_result = int16_hash(int16_in1);
  printf("int16_hash(%d): %u\n", int16_in1, uint32_result);

  /* uint64 int16_hash_extended(int16 val, uint64 seed); */
  uint64_result = int16_hash_extended(int16_in1, 1);
  printf("int16_hash_extended(%d, 1): %lu\n", int16_in1, uint64_result);

  /* int16 int16_in(int16 const char *str); */
  int16_result = int16_in("16");
  printf("int16_in(\"16\"): %d\n", int16_result);

  /* int16 int16_larger(int16 num1, int16 num2); */
  int16_result = int16_larger(int16_in1, int16_in2);
  printf("int16_larger(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int16 int16_mod(int16 num1, int16 num2); */
  int16_result = int16_mod(int16_in1, int16_in2);
  printf("int16_mod(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int16 int16_not(int16 num); */
  int16_result = int16_not(int16_in1);
  printf("int16_not(%d): %d\n", int16_in1, int16_result);

  /* int16 int16_or(int16 num1, int16 num2); */
  int16_result = int16_or(int16_in1, int16_in2);
  printf("int16_or(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* char *int16_out(int16 num); */
  char_result = int16_out(int16_in1);
  printf("int16_out(%d): %s\n", int16_in1, char_result);
  free(char_result);

  /* int16 int16_shl(int16 num1, int32 num2); */
  int16_result = int16_shl(int16_in1, int16_in2);
  printf("int16_shl(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int16 int16_shr(int16 num1, int32 num2); */
  int16_result = int16_shr(int16_in1, int16_in2);
  printf("int16_shr(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int16 int16_smaller(int16 num1, int16 num2); */
  int16_result = int16_smaller(int16_in1, int16_in2);
  printf("int16_smaller(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 int16_to_int32(int16 num); */
  int32_result = int16_to_int32(int16_in1);
  printf("int16_to_int32(%d): %d\n", int16_in1, int32_result);

  /* int64 int16_to_int64(int16 num); */
  int64_result = int16_to_int64(int16_in1);
  printf("int16_to_int64(%d): %ld\n", int16_in1, int64_result);

  /* int16 int16_uminus(int16 num); */
  int16_result = int16_uminus(int16_in1);
  printf("int16_uminus(%d): %d\n", int16_in1, int16_result);

  /* int16 int16_uplus(int16 num); */
  int16_result = int16_uplus(int16_in1);
  printf("int16_uplus(%d): %d\n", int16_in1, int16_result);

  /* int16 int16_xor(int16 num1, int16 num2); */
  int16_result = int16_xor(int16_in1, int16_in2);
  printf("int16_xor(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 int32_abs(int32 num); */
  int32_result = int32_abs(int32_in1);
  printf("int32_abs(%d): %d\n", int32_in1, int32_result);

  /* int32 int32_and(int32 num1, int32 num2); */
  int32_result = int32_and(int32_in1, int32_in2);
  printf("int32_and(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int int32_cmp(int32 l, int32 r); */
  int32_result = int32_cmp(int32_in1, int32_in2);
  printf("int32_cmp(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_gcd(int32 num1, int32 num2); */
  int32_result = int32_gcd(int32_in1, int32_in2);
  printf("int32_gcd(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* uint32 int32_hash(int32 val); */
  uint32_result = int32_hash(int32_in1);
  printf("int32_hash(%d): %u\n", int32_in1, uint32_result);

  /* uint64 int32_hash_extended(int32 val, uint64 seed); */
  uint64_result = int32_hash_extended(int32_in1, 1);
  printf("int32_hash_extended(%d, 1): %lu\n", int32_in1, uint64_result);

  /* int32 int32_in(int32 const char *str); */
  int32_result = int32_in("32");
  printf("int32_in(\"32\"): %d\n", int32_result);

  /* int32 int32_inc(int32 num); */
  int32_result = int32_inc(int32_in1);
  printf("int32_inc(%d): %d\n", int32_in1, int32_result);

  /* int32 int32_larger(int32 num1, int32 num2); */
  int32_result = int32_larger(int32_in1, int32_in2);
  printf("int32_larger(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_lcm(int32 num1, int32 num2); */
  int32_result = int32_lcm(int32_in1, int32_in2);
  printf("int32_lcm(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_mod(int32 num1, int32 num2); */
  int32_result = int32_mod(int32_in1, int32_in2);
  printf("int32_mod(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_not(int32 num); */
  int32_result = int32_not(int32_in1);
  printf("int32_not(%d): %d\n", int32_in1, int32_result);

  /* int32 int32_or(int32 num1, int32 num2); */
  int32_result = int32_or(int32_in1, int32_in2);
  printf("int32_or(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* char *int32_out(int32 num); */
  char_result = int32_out(int32_in1);
  printf("int32_out(%d): %s\n", int32_in1, char_result);
  free(char_result);

  /* int32 int32_shl(int32 num1, int32 num2); */
  int32_result = int32_shl(int32_in1, int32_in2);
  printf("int32_shl(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_shr(int32 num1, int32 num2); */
  int32_result = int32_shr(int32_in1, int32_in2);
  printf("int32_shr(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int32 int32_smaller(int32 num1, int32 num2); */
  int32_result = int32_smaller(int32_in1, int32_in2);
  printf("int32_smaller(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* bool int32_to_bool(int32 num); */
  bool_result = int32_to_bool(int32_in1);
  printf("int32_to_bool(%d): %c\n", int32_in1, bool_result ? 't' : 'f');

  /* int16 int32_to_int16(int32 num); */
  int16_result = int32_to_int16(int32_in1);
  printf("int32_to_int16(%d): %d\n", int32_in1, int16_result);

  /* int64 int32_to_int64(int32 num); */
  int64_result = int32_to_int64(int32_in1);
  printf("int32_to_int64(%d): %ld\n", int32_in1, int64_result);

  /* int32 int32_uminus(int32 num); */
  int32_result = int32_uminus(int32_in1);
  printf("int32_uminus(%d): %d\n", int32_in1, int32_result);

  /* int32 int32_uplus(int32 num); */
  int32_result = int32_uplus(int32_in1);
  printf("int32_uplus(%d): %d\n", int32_in1, int32_result);

  /* int32 int32_xor(int32 num1, int32 num2); */
  int32_result = int32_xor(int32_in1, int32_in2);
  printf("int32_xor(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int64 int64_abs(int64 num); */
  int64_result = int64_abs(int64_in1);
  printf("int64_abs(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_and(int64 num1, int64 num2); */
  int64_result = int64_and(int64_in1, int64_in2);
  printf("int64_and(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);
  
  /* int int64_cmp(int64 l, int64 r); */
  int32_result = int64_cmp(int64_in1, int64_in2);
  printf("int64_cmp(%ld, %ld): %d\n", int64_in1, int64_in2, int32_result);

  /* int64 int64_dec(int64 num); */
  int64_result = int64_dec(int64_in1);
  printf("int64_dec(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_gcd(int64 num1, int64 num2); */
  int64_result = int64_gcd(int64_in1, int64_in2);
  printf("int64_gcd(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* uint32 int64_hash(int64 num); */
  uint32_result = int64_hash(int64_in1);
  printf("int32_hash(%ld): %u\n", int64_in1, uint32_result);

  /* uint64 int64_hash_extended(int64 num, uint64 seed); */
  uint64_result = int64_hash_extended(int64_in1, 1);
  printf("int32_hash_extended(%ld, 1): %lu\n", int64_in1, uint64_result);

  /* int64 int64_in(int64 const char *str); */
  int64_result = int64_in("64");
  printf("int64_in(\"64\"): %ld\n", int64_result);

  /* int64 int64_inc(int64 num); */
  int64_result = int64_inc(int64_in1);
  printf("int64_inc(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_larger(int64 num1, int64 num2); */
  int64_result = int64_larger(int64_in1, int64_in2);
  printf("int64_larger(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* int64 int64_lcm(int64 num1, int64 num2); */
  int64_result = int64_lcm(int64_in1, int64_in2);
  printf("int64_lcm(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* int64 int64_mod(int64 num1, int64 num2); */
  int64_result = int64_mod(int64_in1, int64_in2);
  printf("int64_mod(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* int64 int64_not(int64 num); */
  int64_result = int64_not(int64_in1);
  printf("int64_not(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_or(int64 num1, int64 num2); */
  int64_result = int64_or(int64_in1, int64_in2);
  printf("int64_or(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* char *int64_out(int64 num); */
  char_result = int64_out(int64_in1);
  printf("int64_out(%ld): %s\n", int64_in1, char_result);
  free(char_result);

  /* int64 int64_shl(int64 num1, int32 num2); */
  int64_result = int64_shl(int64_in1, int32_in2);
  printf("int64_shl(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 int64_shr(int64 num1, int32 num2); */
  int64_result = int64_shr(int64_in1, int32_in2);
  printf("int64_shr(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 int64_smaller(int64 num1, int64 num2); */
  int64_result = int64_smaller(int64_in1, int64_in2);
  printf("int64_smaller(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* float4 int64_to_float4(int64 num); */
  float4_result = int64_to_float4(int64_in1);
  printf("int64_to_float4(%ld): %f\n", int64_in1, float4_result);

  /* float8 int64_to_float8(int64 num); */
  float8_result = int64_to_float8(int64_in1);
  printf("int64_to_float8(%ld): %lf\n", int64_in1, float8_result);

  /* int16 int64_to_int16(int64 num); */
  int16_result = int64_to_int16(int64_in1);
  printf("int64_to_int16(%ld): %d\n", int64_in1, int16_result);

  /* int32 int64_to_int32(int64 num); */
  int32_result = int64_to_int32(int64_in1);
  printf("int64_to_int32(%ld): %d\n", int64_in1, int32_result);

  /* int64 int64_uminus(int64 num); */
  int64_result = int64_uminus(int64_in1);
  printf("int64_uminus(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_uplus(int64 num); */
  int64_result = int64_uplus(int64_in1);
  printf("int64_uplus(%ld): %ld\n", int64_in1, int64_result);

  /* int64 int64_xor(int64 num1, int64 num2); */
  int64_result = int64_xor(int64_in1, int64_in2);
  printf("int64_xor(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* bool le_int16_int16(int16 num1, int16 num2); */
  bool_result = le_int16_int16(int16_in1, int16_in2);
  printf("le_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool le_int16_int32(int16 num1, int32 num2); */
  bool_result = le_int16_int32(int16_in1, int32_in2);
  printf("le_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool le_int16_int64(int16 num1, int64 num2); */
  bool_result = le_int16_int64(int16_in1, int64_in2);
  printf("le_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool le_int32_int16(int32 num1, int16 num2); */
  bool_result = le_int32_int16(int32_in1, int16_in2);
  printf("le_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool le_int32_int32(int32 num1, int32 num2); */
  bool_result = le_int32_int32(int32_in1, int32_in2);
  printf("le_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool le_int32_int64(int32 num1, int64 num2); */
  bool_result = le_int32_int64(int32_in1, int64_in2);
  printf("le_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool le_int64_int16(int64 num1, int16 num2); */
  bool_result = le_int64_int16(int64_in1, int16_in2);
  printf("le_int64_int16(%ld, %d): %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool le_int64_int32(int64 num1, int32 num2); */
  bool_result = le_int64_int32(int64_in1, int32_in2);
  printf("le_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool le_int64_int64(int64 num1, int64 num2); */
  bool_result = le_int64_int64(int64_in1, int64_in2);
  printf("le_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool lt_int16_int16(int16 num1, int16 num2); */
  bool_result = lt_int16_int16(int16_in1, int16_in2);
  printf("lt_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool lt_int16_int32(int16 num1, int32 num2); */
  bool_result = lt_int16_int32(int16_in1, int32_in2);
  printf("lt_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool lt_int16_int64(int16 num1, int64 num2); */
  bool_result = lt_int16_int64(int16_in1, int64_in2);
  printf("lt_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool lt_int32_int16(int32 num1, int16 num2); */
  bool_result = lt_int32_int16(int32_in1, int16_in2);
  printf("lt_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool lt_int32_int32(int32 num1, int32 num2); */
  bool_result = lt_int32_int32(int32_in1, int32_in2);
  printf("lt_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool lt_int32_int64(int32 num1, int64 num2); */
  bool_result = lt_int32_int64(int32_in1, int64_in2);
  printf("lt_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool lt_int64_int16(int64 num1, int16 num2); */
  bool_result = lt_int64_int16(int64_in1, int16_in2);
  printf("lt_int64_int16(%ld, %d): %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool lt_int64_int32(int64 num1, int32 num2); */
  bool_result = lt_int64_int32(int64_in1, int32_in2);
  printf("lt_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool lt_int64_int64(int64 num1, int64 num2); */
  bool_result = lt_int64_int64(int64_in1, int64_in2);
  printf("lt_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  /* int16 minus_int16_int16(int16 num1, int16 num2); */
  int16_result = minus_int16_int16(int16_in1, int16_in2);
  printf("minus_int16_int16(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 minus_int16_int32(int16 num1, int32 num2); */
  int32_result = minus_int16_int32(int16_in1, int32_in2);
  printf("minus_int16_int32(%d, %d): %d\n", int16_in1, int32_in2, int32_result);

  /* int64 minus_int16_int64(int16 num1, int64 num2); */
  int64_result = minus_int16_int64(int16_in1, int64_in2);
  printf("minus_int16_int64(%d, %ld): %ld\n", int16_in1, int64_in2, int64_result);

  /* int32 minus_int32_int16(int32 num1, int16 num2); */
  int32_result = minus_int32_int16(int32_in1, int16_in2);
  printf("minus_int32_int16(%d, %d): %d\n", int32_in1, int16_in2, int32_result);

  /* int32 minus_int32_int32(int32 num1, int32 num2); */
  int32_result = minus_int32_int32(int32_in1, int32_in2);
  printf("minus_int32_int32(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int64 minus_int32_int64(int32 num1, int64 num2); */
  int64_result = minus_int32_int64(int32_in1, int64_in2);
  printf("minus_int32_int64(%d, %ld): %ld\n", int32_in1, int64_in2, int64_result);

  /* int64 minus_int64_int16(int64 num1, int16 num2); */
  int64_result = minus_int64_int16(int64_in1, int16_in2);
  printf("minus_int64_int16(%ld, %d): %ld\n", int64_in1, int16_in2, int64_result);

  /* int64 minus_int64_int32(int64 num1, int32 num2); */
  int64_result = minus_int64_int32(int64_in1, int32_in2);
  printf("minus_int64_int32(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 minus_int64_int64(int64 num1, int64 num2); */
  int64_result = minus_int64_int64(int64_in1, int64_in2);
  printf("minus_int64_int64(%ld, %ld): %ld\n", int64_in1, int64_in2, int64_result);

  /* int16 mul_int16_int16(int16 num1, int16 num2); */
  int16_result = mul_int16_int16(int16_in1, int16_in2);
  printf("mul_int16_int16(%d, %d): %d\n", int16_in1, int16_in2, int16_result);

  /* int32 mul_int16_int32(int16 num1, int32 num2); */
  int32_result = mul_int16_int32(int16_in1, int32_in2);
  printf("mul_int16_int32(%d, %d): %d\n", int16_in1, int32_in2, int32_result);

  /* int64 mul_int16_int64(int16 num1, int64 num2); */
  int64_result = mul_int16_int64(int16_in1, int64_in2);
  printf("mul_int16_int64(%d, %ld): %ld\n", int16_in1, int64_in2, int64_result);

  /* int32 mul_int32_int16(int32 num1, int16 num2); */
  int32_result = mul_int32_int16(int32_in1, int16_in2);
  printf("mul_int32_int16(%d, %d): %d\n", int32_in1, int16_in2, int32_result);

  /* int32 mul_int32_int32(int32 num1, int32 num2); */
  int32_result = mul_int32_int32(int32_in1, int32_in2);
  printf("mul_int32_int32(%d, %d): %d\n", int32_in1, int32_in2, int32_result);

  /* int64 mul_int32_int64(int32 num1, int64 num2); */
  int64_result = mul_int32_int64(int32_in1, int64_in2);
  printf("mul_int32_int64(%d, %ld): %ld\n", int32_in1, int64_in2, int64_result);
  
  /* int64 mul_int64_int16(int64 num1, int16 num2); */
  int64_result = mul_int64_int16(int64_in1, int16_in2);
  printf("mul_int64_int16(%ld, %d): %ld\n", int64_in1, int16_in2, int64_result);

  /* int64 mul_int64_int32(int64 num1, int32 num2); */
  int64_result = mul_int64_int32(int64_in1, int32_in2);
  printf("mul_int64_int32(%ld, %d): %ld\n", int64_in1, int32_in2, int64_result);

  /* int64 mul_int64_int64(int64 num1, int64 num2); */
  int64_result = mul_int64_int64(int64_in1, int64_in2);
  printf("mul_int64_int64(%ld, %ld) %ld\n", int64_in1, int64_in2, int64_result);

  /* bool ne_int16_int16(int16 num1, int16 num2); */
  bool_result = ne_int16_int16(int16_in1, int16_in2);
  printf("ne_int16_int16(%d, %d): %c\n", int16_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ne_int16_int32(int16 num1, int32 num2); */
  bool_result = ne_int16_int32(int16_in1, int32_in2);
  printf("ne_int16_int32(%d, %d): %c\n", int16_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ne_int16_int64(int16 num1, int64 num2); */
  bool_result = ne_int16_int64(int16_in1, int64_in2);
  printf("ne_int16_int64(%d, %ld): %c\n", int16_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool ne_int32_int16(int32 num1, int16 num2); */
  bool_result = ne_int32_int16(int32_in1, int16_in2);
  printf("ne_int32_int16(%d, %d): %c\n", int32_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ne_int32_int32(int32 num1, int32 num2); */
  bool_result = ne_int32_int32(int32_in1, int32_in2);
  printf("ne_int32_int32(%d, %d): %c\n", int32_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ne_int32_int64(int32 num1, int64 num2); */
  bool_result = ne_int32_int64(int32_in1, int64_in2);
  printf("ne_int32_int64(%d, %ld): %c\n", int32_in1, int64_in2, bool_result ? 't' : 'f');

  /* bool ne_int64_int16(int64 num1, int16 num2); */
  bool_result = ne_int64_int16(int64_in1, int16_in2);
  printf("ne_int64_int16(%ld, %d): %c\n", int64_in1, int16_in2, bool_result ? 't' : 'f');

  /* bool ne_int64_int32(int64 num1, int32 num2); */
  bool_result = ne_int64_int32(int64_in1, int32_in2);
  printf("ne_int64_int32(%ld, %d): %c\n", int64_in1, int32_in2, bool_result ? 't' : 'f');

  /* bool ne_int64_int64(int64 num1, int64 num2); */
  bool_result = ne_int64_int64(int64_in1, int64_in2);
  printf("ne_int64_int64(%ld, %ld): %c\n", int64_in1, int64_in2, bool_result ? 't' : 'f');

  printf("****************************************************************\n");

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
