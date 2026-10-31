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
 * @brief A program that tests how the JSON path functions of the MEOS API
 * report null variables under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. An assertion cannot carry that contract: it
 * is compiled out under NDEBUG, which leaves the release build a binding links
 * against with no check at all.
 *
 * The program verifies that #jsonb_path_exists, #jsonb_path_match,
 * #jsonb_path_query_array, #jsonb_path_query_first and #jsonb_path_query_all,
 * #jsonbset_path_exists, #jsonbset_path_match, #jsonbset_path_query_array and
 * #jsonbset_path_query_first, and #tjsonb_path_exists, #tjsonb_path_match,
 * #tjsonb_path_query_array and #tjsonb_path_query_first, report null variables
 * by returning NULL, or -1 and false for the two that return a value, and
 * setting #meos_errno, and that each answers with the variables a path names
 * and no error left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_path_vars_validity_test json_path_vars_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

/**
 * @brief Check that a call answered NULL with an invalid argument error, then
 * clear it
 */
static void
expect_error(void *res, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
}

/**
 * @brief Check that a call answered a value and left no error behind
 */
static void
expect_value(void *res, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res != NULL);
  assert(meos_errno() == 0);
  free(res);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Jsonb *jb = jsonb_in("{\"a\": 1}");
  Set *set = jsonbset_in("{\"{\\\"a\\\": 1}\", \"{\\\"a\\\": 2}\"}");
  Temporal *temp = tjsonb_in(
    "{\"{\\\"a\\\": 1}\"@2001-01-01, \"{\\\"a\\\": 2}\"@2001-01-02}");
  JsonPath *query = jsonpath_in("$.a ? (@ > $min)");
  JsonPath *pred = jsonpath_in("$.a > $min");
  Jsonb *vars = jsonb_in("{\"min\": 0}");
  assert(jb && set && temp && query && pred && vars);
  meos_errno_reset();
  int count;

  /* Null variables are reported rather than read as no variables */
  int exists = jsonb_path_exists(jb, query, NULL, false, false);
  printf("jsonb_path_exists(NULL): %d, errno %d\n", exists, meos_errno());
  assert(exists == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  bool match = jsonb_path_match(jb, pred, NULL, false, false);
  printf("jsonb_path_match(NULL): %s, errno %d\n", match ? "true" : "false",
    meos_errno());
  assert(! match);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  expect_error(jsonb_path_query_array(jb, query, NULL, false, false),
    "jsonb_path_query_array(NULL)");
  expect_error(jsonb_path_query_first(jb, query, NULL, false, false),
    "jsonb_path_query_first(NULL)");
  expect_error(jsonb_path_query_all(jb, query, NULL, false, false, &count),
    "jsonb_path_query_all(NULL)");
  expect_error(jsonbset_path_exists(set, query, NULL, false, false, &count),
    "jsonbset_path_exists(NULL)");
  expect_error(jsonbset_path_match(set, pred, NULL, false, false, &count),
    "jsonbset_path_match(NULL)");
  expect_error(jsonbset_path_query_array(set, query, NULL, false, false),
    "jsonbset_path_query_array(NULL)");
  expect_error(jsonbset_path_query_first(set, query, NULL, false, false),
    "jsonbset_path_query_first(NULL)");
  expect_error(tjsonb_path_exists(temp, query, NULL, false, false),
    "tjsonb_path_exists(NULL)");
  expect_error(tjsonb_path_match(temp, pred, NULL, false, false),
    "tjsonb_path_match(NULL)");
  expect_error(tjsonb_path_query_array(temp, query, NULL, false, false),
    "tjsonb_path_query_array(NULL)");
  expect_error(tjsonb_path_query_first(temp, query, NULL, false, false),
    "tjsonb_path_query_first(NULL)");

  /* The variables answer the paths naming them */
  exists = jsonb_path_exists(jb, query, vars, false, false);
  printf("jsonb_path_exists(vars): %d, errno %d\n", exists, meos_errno());
  assert(exists == 1 && meos_errno() == 0);
  match = jsonb_path_match(jb, pred, vars, false, false);
  printf("jsonb_path_match(vars): %s, errno %d\n", match ? "true" : "false",
    meos_errno());
  assert(match && meos_errno() == 0);
  expect_value(jsonb_path_query_array(jb, query, vars, false, false),
    "jsonb_path_query_array(vars)");
  expect_value(jsonb_path_query_first(jb, query, vars, false, false),
    "jsonb_path_query_first(vars)");
  Jsonb **all = jsonb_path_query_all(jb, query, vars, false, false, &count);
  printf("jsonb_path_query_all(vars): %d item(s), errno %d\n", count,
    meos_errno());
  assert(all && count == 1 && meos_errno() == 0);
  free(all[0]); free(all);
  expect_value(jsonbset_path_exists(set, query, vars, false, false, &count),
    "jsonbset_path_exists(vars)");
  expect_value(jsonbset_path_match(set, pred, vars, false, false, &count),
    "jsonbset_path_match(vars)");
  expect_value(jsonbset_path_query_array(set, query, vars, false, false),
    "jsonbset_path_query_array(vars)");
  expect_value(jsonbset_path_query_first(set, query, vars, false, false),
    "jsonbset_path_query_first(vars)");
  expect_value(tjsonb_path_exists(temp, query, vars, false, false),
    "tjsonb_path_exists(vars)");
  expect_value(tjsonb_path_match(temp, pred, vars, false, false),
    "tjsonb_path_match(vars)");
  expect_value(tjsonb_path_query_array(temp, query, vars, false, false),
    "tjsonb_path_query_array(vars)");
  expect_value(tjsonb_path_query_first(temp, query, vars, false, false),
    "tjsonb_path_query_first(vars)");

  free(jb); free(set); free(temp); free(query); free(pred); free(vars);
  meos_finalize();
  return EXIT_SUCCESS;
}
