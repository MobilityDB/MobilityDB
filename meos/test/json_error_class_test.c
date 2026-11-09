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
 * @brief A program that tests that an SQL/JSON error reports the error code
 * of the class PostgreSQL gives it
 * @details The JSON twin of pgtypes_error_class_test.c. Each jsonpath is
 * applied once to a value on which PostgreSQL raises an error, which must
 * report through meos_errno the MEOS code named after the errcode PostgreSQL
 * raises with the same message, and once to a value it answers, which must
 * report none: a method on a value of the wrong type
 * #MEOS_ERR_NON_NUMERIC_SQL_JSON_ITEM, `.datetime()` of a number
 * #MEOS_ERR_INVALID_ARGUMENT_FOR_SQL_JSON_DATETIME_FUNCTION, an operator on
 * several items #MEOS_ERR_SINGLETON_SQL_JSON_ITEM_REQUIRED, the accessors of
 * an array, of an object and of a member on a value that is none
 * #MEOS_ERR_SQL_JSON_ARRAY_NOT_FOUND, #MEOS_ERR_SQL_JSON_OBJECT_NOT_FOUND and
 * #MEOS_ERR_SQL_JSON_MEMBER_NOT_FOUND, a subscript out of bounds
 * #MEOS_ERR_INVALID_SQL_JSON_SUBSCRIPT, the minus of a string
 * #MEOS_ERR_SQL_JSON_NUMBER_NOT_FOUND, a variable the variables do not hold
 * #MEOS_ERR_UNDEFINED_OBJECT, and a key repeated in a JSON object read with
 * unique keys #MEOS_ERR_DUPLICATE_JSON_OBJECT_KEY_VALUE.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_error_class_test json_error_class_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

static int failures;

/* Verify the error the last operation left, then clear it; the twin of
 * #check of pgtypes_error_class_test.c */
static void
check(const char *what, int expected)
{
  int err = meos_errno();
  printf("%s: error %d\n", what, err);
  if (err != expected)
  {
    printf("  expected error %d\n", expected);
    failures++;
  }
  meos_errno_reset();
}

/* Apply a jsonpath to a JSON value through #jsonb_path_query_first with the
 * given variables, an empty object where none is given, raising its errors,
 * and verify the error it leaves; no program of meos/test applies a jsonpath,
 * so this entry is built beside #check */
static void
path_entry(const char *json, const char *path, const char *vars,
  int expected)
{
  Jsonb *jb = jsonb_in(json);
  JsonPath *jp = jsonpath_in(path);
  Jsonb *jvars = jsonb_in(vars ? vars : "{}");
  Jsonb *res = jsonb_path_query_first(jb, jp, jvars, false, false);
  char what[128];
  snprintf(what, sizeof(what), "%s on %s", path, json);
  check(what, expected);
  free(res); free(jvars); free(jp); free(jb);
}

int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  path_entry("\"x\"", "$.abs()", NULL, MEOS_ERR_NON_NUMERIC_SQL_JSON_ITEM);
  path_entry("-1", "$.abs()", NULL, 0);
  path_entry("1", "$.datetime()", NULL,
    MEOS_ERR_INVALID_ARGUMENT_FOR_SQL_JSON_DATETIME_FUNCTION);
  path_entry("\"2001-01-01\"", "$.datetime()", NULL, 0);
  path_entry("[1, 2]", "$[*] + 1", NULL,
    MEOS_ERR_SINGLETON_SQL_JSON_ITEM_REQUIRED);
  path_entry("[1]", "$[*] + 1", NULL, 0);
  path_entry("1", "strict $[0]", NULL, MEOS_ERR_SQL_JSON_ARRAY_NOT_FOUND);
  path_entry("[1]", "strict $[0]", NULL, 0);
  path_entry("[1]", "strict $[5]", NULL, MEOS_ERR_INVALID_SQL_JSON_SUBSCRIPT);
  path_entry("1", "strict $.*", NULL, MEOS_ERR_SQL_JSON_OBJECT_NOT_FOUND);
  path_entry("{\"a\": 1}", "strict $.*", NULL, 0);
  path_entry("{}", "strict $.a", NULL, MEOS_ERR_SQL_JSON_MEMBER_NOT_FOUND);
  path_entry("{\"a\": 1}", "strict $.a", NULL, 0);
  path_entry("\"x\"", "strict -$", NULL, MEOS_ERR_SQL_JSON_NUMBER_NOT_FOUND);
  path_entry("1", "strict -$", NULL, 0);
  path_entry("1", "$x", "{}", MEOS_ERR_UNDEFINED_OBJECT);
  path_entry("1", "$x", "{\"x\": 2}", 0);

  /* A key repeated in an object read with unique keys */
  text *dup = text_in("{\"a\": 1, \"a\": 2}");
  text *uni = text_in("{\"a\": 1, \"b\": 2}");
  Jsonb *jb = jsonb_from_text(dup, true);
  check("{\"a\": 1, \"a\": 2} with unique keys",
    MEOS_ERR_DUPLICATE_JSON_OBJECT_KEY_VALUE);
  free(jb);
  jb = jsonb_from_text(uni, true);
  check("{\"a\": 1, \"b\": 2} with unique keys", 0);
  free(jb); free(dup); free(uni);

  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
