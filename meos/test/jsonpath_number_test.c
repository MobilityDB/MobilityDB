/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief A program that tests the numbers the jsonpath item methods
 * .double(), .bigint() and .integer() answer
 * @details Each answer is the one the regression test jsonb_jsonpath of
 * PostgreSQL 18 expects for the same JSON value and jsonpath, of a number as
 * of a string. Run under valgrind, the program also asks that the number each
 * method builds is freed once the answer is written.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o jsonpath_number_test jsonpath_number_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>

static int failures;

/* Apply a jsonpath to a JSON value through #jsonb_path_query_first and verify
 * that it answers the expected text with no error; the twin of #check of
 * jsonpath_decimal_test.c */
static void
check(const char *json, const char *path, const char *expected)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *vars = jsonb_in("{}");
  Jsonb *res = jsonb_path_query_first(jb, jp, vars, false, false);
  int err = meos_errno();
  size_t size;
  char *text = res ? jsonb_out(res, &size) : NULL;
  printf("%s on %s: %s, error %d\n", path, json, text ? text : "no answer",
    err);
  if (err != 0 || ! text || strcmp(text, expected) != 0)
  {
    printf("  expected %s\n", expected);
    failures++;
  }
  meos_errno_reset();
  free(text); free(res); free(vars); free(jp); free(jb);
}

int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  check("1.23", "$.double()", "1.23");
  check("\"1.23\"", "$.double()", "1.23");
  check("123", "$.bigint()", "123");
  check("\"123\"", "$.bigint()", "123");
  check("1.23", "$.bigint()", "1");
  check("1.83", "$.bigint()", "2");
  check("1234567890123", "$.bigint()", "1234567890123");
  check("123", "$.integer()", "123");
  check("\"123\"", "$.integer()", "123");
  check("1.23", "$.integer()", "1");
  check("1.83", "$.integer()", "2");
  check("\"+123\"", "$.integer()", "123");

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
