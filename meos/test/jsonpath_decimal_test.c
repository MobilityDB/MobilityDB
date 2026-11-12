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
 * @brief A program that tests the jsonpath item method .decimal() with a
 * precision and a scale
 * @details Each answer is the one the regression test jsonb_jsonpath of
 * PostgreSQL 18 expects for the same JSON value and jsonpath: the method
 * rounds the number to the precision and the scale, of a number as of a
 * string, and refuses a number the precision cannot hold. Run under valgrind,
 * the program also asks that the method reads no string it freed, frees none
 * twice and keeps none, and that the parse of a jsonpath keeps nothing.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o jsonpath_decimal_test jsonpath_decimal_test.c -L/usr/local/lib -lmeos -lm
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
 * its answer, NULL standing for an error and no answer */
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
  bool ok = expected ?
    (err == 0 && text && strcmp(text, expected) == 0) : (err != 0 && ! text);
  if (! ok)
  {
    printf("  expected %s\n", expected ? expected : "an error and no answer");
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

  /* The number rounded to the precision and the scale */
  check("12345.678", "$.decimal(6, 1)", "12345.7");
  check("1234.5678", "$.decimal(6, 2)", "1234.57");
  check("1234.5678", "$.decimal(+6, +2)", "1234.57");
  check("1234.5678", "$.decimal(+6, -2)", "1200");
  check("-1234.5678", "$.decimal(+6, -2)", "-1200");
  check("0.0123456", "$.decimal(1,2)", "0.01");
  check("0.0012345", "$.decimal(2,4)", "0.0012");
  /* The same of a string, whose text the method reads twice */
  check("\"1234.5678\"", "$.decimal(6, 2)", "1234.57");
  check("\"12345.678\"", "$.decimal(6, 1)", "12345.7");
  /* A number the precision cannot hold */
  check("12345.678", "$.decimal(6, 2)", NULL);
  check("12345.678", "$.decimal(4, 6)", NULL);
  check("\"12345.678\"", "$.decimal(6, 2)", NULL);
  /* A precision or a scale out of range */
  check("12345.678", "$.decimal(0, 6)", NULL);
  check("12345.678", "$.decimal(1001, 6)", NULL);
  check("1234.5678", "$.decimal(6, 1001)", NULL);
  /* No precision, which keeps the number */
  check("\"12.3\"", "$.decimal()", "12.3");
  check("12.3", "$.decimal()", "12.3");

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
