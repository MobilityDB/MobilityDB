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
 * @brief A program that tests that a jsonpath raising an error answers
 * nothing, through the error handler that returns
 * @details Each jsonpath raises an error once the handler returns, as it does
 * in every binding: an item method on an argument it cannot convert
 * (`.double()`, `.bigint()` and `.integer()` of a string or of a number beyond
 * the type), a member the
 * object does not hold, and a variable the variables do not hold, the last
 * asked of each of the five functions applying a jsonpath. Each must report an
 * error and answer nothing. Run under valgrind, which the MEOS workflow does,
 * the program also asks that no error path reads a string it freed, frees one
 * twice or keeps one.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o jsonpath_error_path_test jsonpath_error_path_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>

static int failures;

/* Verify that the last operation reported an error and answered nothing,
 * then clear the error; the twin of #check of pgtypes_error_class_test.c */
static void
check(const char *what, bool answered)
{
  int err = meos_errno();
  printf("%s: error %d, %s\n", what, err, answered ? "answered" : "no answer");
  if (err == 0 || answered)
  {
    printf("  expected an error and no answer\n");
    failures++;
  }
  meos_errno_reset();
}

/* Apply a jsonpath to a JSON value through #jsonb_path_query_first, raising
 * its errors, and verify that it reports one and answers nothing */
static void
first_entry(const char *json, const char *path)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *vars = jsonb_in("{}");
  Jsonb *res = jsonb_path_query_first(jb, jp, vars, false, false);
  char what[128];
  snprintf(what, sizeof(what), "%s on %s", path, json);
  check(what, res != NULL);
  free(res); free(vars); free(jp); free(jb);
}

/* Apply a jsonpath to a JSON value through #jsonb_path_query_first, with its
 * errors suppressed or raised, and verify that it reports no error and that
 * it answers the expected text, NULL standing for no answer */
static void
first_entry_answers(const char *json, const char *path, bool silent,
  const char *expected)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *vars = jsonb_in("{}");
  Jsonb *res = jsonb_path_query_first(jb, jp, vars, silent, false);
  int err = meos_errno();
  size_t size;
  char *text = res ? jsonb_out(res, &size) : NULL;
  printf("%s on %s%s: error %d, %s\n", path, json, silent ? " (silent)" : "",
    err, text ? text : "no answer");
  bool same = expected ? (text && strcmp(text, expected) == 0) : ! text;
  if (err != 0 || ! same)
  {
    printf("  expected no error and %s\n", expected ? expected : "no answer");
    failures++;
  }
  meos_errno_reset();
  free(text); free(res); free(vars); free(jp); free(jb);
}

/* Apply a jsonpath to a JSON value through #jsonb_path_query_first, raising
 * its errors, and verify that it reports the error of a non-numeric SQL/JSON
 * item, the code PostgreSQL raises as 22036, and answers nothing */
static void
first_entry_non_numeric(const char *json, const char *path)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *vars = jsonb_in("{}");
  Jsonb *res = jsonb_path_query_first(jb, jp, vars, false, false);
  int err = meos_errno();
  printf("%s on %s: error %d, %s\n", path, json, err,
    res ? "answered" : "no answer");
  if (err != MEOS_ERR_NON_NUMERIC_SQL_JSON_ITEM || res)
  {
    printf("  expected error %d and no answer\n",
      MEOS_ERR_NON_NUMERIC_SQL_JSON_ITEM);
    failures++;
  }
  meos_errno_reset();
  free(res); free(vars); free(jp); free(jb);
}

int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* Item methods on an argument they cannot convert */
  first_entry("\"x\"", "$.double()");
  first_entry("\"x\"", "$.bigint()");
  first_entry("1e30", "$.bigint()");
  first_entry("1e30", "$.integer()");
  /* The error an item method raises is that of a non-numeric SQL/JSON item,
   * whatever the parser of the type it converts to reports */
  first_entry_non_numeric("\"x\"", "$.double()");
  first_entry_non_numeric("\"x\"", "$.bigint()");
  first_entry_non_numeric("\"x\"", "$.integer()");
  first_entry_non_numeric("\"x\"", "$.number()");
  first_entry_non_numeric("\"x\"", "$.decimal()");
  first_entry_non_numeric("\"1.5\"", "$.integer()");
  first_entry_non_numeric("1.5", "$.boolean()");
  first_entry_non_numeric("123.456", "$.decimal(4, 2)");
  first_entry_non_numeric("\"123.456\"", "$.decimal(4, 2)");
  /* A suppressed error leaves no error and no answer */
  first_entry_answers("\"x\"", "$.double()", true, NULL);
  first_entry_answers("\"x\"", "$.bigint()", true, NULL);
  first_entry_answers("\"x\"", "$.integer()", true, NULL);
  first_entry_answers("\"x\"", "$.number()", true, NULL);
  first_entry_answers("1.5", "$.boolean()", true, NULL);
  first_entry_answers("123.456", "$.decimal(4, 2)", true, NULL);
  first_entry_answers("\"123.456\"", "$.decimal(4, 2)", true, NULL);
  first_entry_answers("1", "$.decimal(0)", true, NULL);
  /* The greatest value of each type is a value, not an error */
  first_entry_answers("\"9223372036854775807\"", "$.bigint()", false,
    "9223372036854775807");
  first_entry_answers("\"2147483647\"", "$.integer()", false, "2147483647");
  first_entry_answers("2147483647", "$.boolean()", false, "true");
  /* The greatest double, which .double() answers as the numeric of its 15
   * significant digits, as float8_numeric() of PostgreSQL writes it */
  char dblmax[310];
  snprintf(dblmax, sizeof(dblmax), "%s", "179769313486232");
  for (int i = 15; i < 309; i++)
    dblmax[i] = '0';
  dblmax[309] = '\0';
  first_entry_answers("\"1.7976931348623157e308\"", "$.double()", false,
    dblmax);
  /* A member the object does not hold */
  first_entry("{\"a\": 1}", "strict $.b");
  /* A variable the variables do not hold, through each function */
  first_entry("1", "$x");
  Jsonb *jb = jsonb_in("1");
  JsonPath *jp = jsonpath_in("$x");
  Jsonb *vars = jsonb_in("{}");
  Jsonb *res = jsonb_path_query_array(jb, jp, vars, false, false);
  check("jsonb_path_query_array of $x", res != NULL);
  free(res);
  int count = -1;
  Jsonb **all = jsonb_path_query_all(jb, jp, vars, false, false, &count);
  check("jsonb_path_query_all of $x", all != NULL || count != 0);
  free(all);
  int exists = jsonb_path_exists(jb, jp, vars, false, false);
  check("jsonb_path_exists of $x", exists != -1);
  free(jp);
  jp = jsonpath_in("$x == 1");
  int match = jsonb_path_match(jb, jp, vars, false, false);
  check("jsonb_path_match of $x == 1", match != -1);
  free(jp); free(vars); free(jb);

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
