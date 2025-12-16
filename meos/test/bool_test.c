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
 * @brief A simple program that tests the boolean functions exposed by the
 * PostgreSQL types embedded in MEOS.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o bool_test bool_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_text.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* Create values to test the functions of the API */
  bool b1 = bool_in("true");
  bool b2 = bool_in("false");
  char *b1_out = bool_out(b1);
  char *b2_out = bool_out(b2);

  /* Create the result types for the functions of the API */
  bool bool_result;
  uint32 uint32_result;
  uint64 uint64_result;
  char *char_result;
  text *text_result;
  
  /* Execute and print the result for the functions of the API */

  printf("****************************************************************\n");
  printf("* Boolean *\n");
  printf("****************************************************************\n");

  /* bool bool_eq(bool arg1, bool arg2); */
  bool_result = bool_eq(b1, b2);
  printf("bool_eq(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* bool bool_ge(bool arg1, bool arg2); */
  bool_result = bool_ge(b1, b2);
  printf("bool_ge(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* bool bool_gt(bool arg1, bool arg2); */
  bool_result = bool_gt(b1, b2);
  printf("bool_gt(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* uint32 bool_hash(bool arg); */
  uint32_result = bool_hash(b1);
  printf("bool_hash(%s): %u\n", b1_out, uint32_result);

  /* uint64 bool_hash_extended(bool arg, int64 seed); */
  uint64_result = bool_hash_extended(b1, 1);
  printf("bool_hash_extended(%s, 1): %lu\n", b1_out, uint64_result);

  /* bool bool_le(bool arg1, bool arg2); */
  bool_result = bool_le(b1, b2);
  printf("bool_le(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* bool bool_lt(bool arg1, bool arg2); */
  bool_result = bool_lt(b1, b2);
  printf("bool_lt(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* bool bool_ne(bool arg1, bool arg2); */
  bool_result = bool_ne(b1, b2);
  printf("bool_ne(%s, %s): %c\n", b1_out, b2_out, bool_result ? 't' : 'f');

  /* char *bool_out(bool b); */
  char_result = bool_out(b1);
  printf("bool_out(%s): %s\n", b1_out, char_result);
  free(char_result);

  /* text *bool_to_text(bool b); */
  text_result = bool_to_text(b1);
  char_result = text_out(text_result);
  printf("bool_to_text(%s): %s\n", b1_out, char_result);
  free(text_result); free(char_result);

  printf("****************************************************************\n");

  /* Finalize MEOS */
  free(b1_out);
  free(b2_out);
  
  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
