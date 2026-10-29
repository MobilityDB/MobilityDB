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
 * @brief A program that tests how the JSONB functions of the MEOS API taking
 * an array of keys or path elements read a NULL element
 * @details A NULL element of the array is a NULL of the SQL array the
 * function is derived from, and each function answers what PostgreSQL 18
 * answers for it: #jsonb_extract_path and #jsonb_extract_path_text answer
 * NULL for a path holding a NULL element, #jsonb_exists_array and
 * #jsonb_delete_array skip a NULL key, and #jsonb_delete_path, #jsonb_set and
 * #jsonb_insert report "path element at position N is null" when the path
 * reaches the element and answer their argument unchanged when it does not.
 * The extractions of a JSON text, #json_extract_path and
 * #json_extract_path_text, and those of a temporal JSONB, a temporal JSON and
 * a JSONB set answer NULL for such a path as well, whatever the null handling.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o jsonb_null_path_test jsonb_null_path_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>
#include <pg_text.h>

#define JB "{\"a\": {\"b\": 1}, \"c\": 2}"

/**
 * @brief Return a path of two elements, a NULL string giving a NULL element
 */
static text **
path2(const char *e1, const char *e2)
{
  text **result = malloc(2 * sizeof(text *));
  result[0] = e1 ? cstring_to_text(e1) : NULL;
  result[1] = e2 ? cstring_to_text(e2) : NULL;
  return result;
}

/**
 * @brief Free a path of two elements
 */
static void
path2_free(text **path)
{
  free(path[0]); free(path[1]); free(path);
}

/**
 * @brief Check that a call answered NULL with the expected error, then clear it
 * @details The check of json_validity_test.c's expect_error()
 */
static void
expect_error(void *res, int code, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == code);
  meos_errno_reset();
}

/**
 * @brief Check that a call answered NULL and left no error behind
 */
static void
expect_null(void *res, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "a value" : "NULL", meos_errno());
  assert(res == NULL);
  assert(meos_errno() == 0);
}

/**
 * @brief Check that a call answered the JSONB value of the expected text and
 * left no error behind, then free the value
 */
static void
expect_jsonb(Jsonb *res, const char *want, const char *call)
{
  char *out = res ? jsonb_out(res) : NULL;
  printf("%s: %s, errno %d\n", call, out ? out : "NULL", meos_errno());
  assert(out != NULL);
  assert(strcmp(out, want) == 0);
  assert(meos_errno() == 0);
  free(out); free(res);
}

/**
 * @brief Check that a call answered the expected Boolean and left no error
 * behind
 */
static void
expect_bool(bool res, bool want, const char *call)
{
  printf("%s: %s, errno %d\n", call, res ? "true" : "false", meos_errno());
  assert(res == want);
  assert(meos_errno() == 0);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  Jsonb *jb = jsonb_in(JB);
  Jsonb *three = jsonb_in("3");
  assert(jb); assert(three);
  meos_errno_reset();

  /* A path holding a NULL element answers NULL */
  text **a_null = path2("a", NULL);
  text **null_a = path2(NULL, "a");
  expect_null(jsonb_extract_path(jb, a_null, 2),
    "jsonb_extract_path({a, NULL})");
  expect_null(jsonb_extract_path(jb, null_a, 2),
    "jsonb_extract_path({NULL, a})");
  expect_null(jsonb_extract_path_text(jb, a_null, 2),
    "jsonb_extract_path_text({a, NULL})");

  /* A NULL key is skipped */
  text **x_null = path2("x", NULL);
  text **null_null = path2(NULL, NULL);
  expect_bool(jsonb_exists_array(jb, a_null, 2, true), true,
    "jsonb_exists_array({a, NULL}, any)");
  expect_bool(jsonb_exists_array(jb, x_null, 2, true), false,
    "jsonb_exists_array({x, NULL}, any)");
  expect_bool(jsonb_exists_array(jb, a_null, 2, false), true,
    "jsonb_exists_array({a, NULL}, all)");
  expect_bool(jsonb_exists_array(jb, null_null, 1, false), true,
    "jsonb_exists_array({NULL}, all)");
  expect_jsonb(jsonb_delete_array(jb, a_null, 2), "{\"c\": 2}",
    "jsonb_delete_array({a, NULL})");

  /* A NULL path element is reported where the path reaches it */
  expect_error(jsonb_delete_path(jb, null_null, 1),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_delete_path({NULL})");
  expect_error(jsonb_set(jb, null_null, 1, three, true),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_set({NULL})");
  expect_error(jsonb_insert(jb, null_null, 1, three, false),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_insert({NULL})");

  /* A path that does not reach its NULL element leaves the value as it is */
  expect_jsonb(jsonb_delete_path(jb, x_null, 2), JB,
    "jsonb_delete_path({x, NULL})");
  expect_jsonb(jsonb_set(jb, x_null, 2, three, true), JB,
    "jsonb_set({x, NULL})");
  expect_jsonb(jsonb_insert(jb, x_null, 2, three, false), JB,
    "jsonb_insert({x, NULL})");

  /* The extractions of a JSON text answer NULL for such a path */
  text *js = cstring_to_text(JB);
  expect_null(json_extract_path(js, a_null, 2),
    "json_extract_path({a, NULL})");
  expect_null(json_extract_path_text(js, a_null, 2),
    "json_extract_path_text({a, NULL})");

  /* The extractions of a temporal JSONB, a temporal JSON and a JSONB set
   * answer NULL for such a path, while the same path without its NULL element
   * extracts a value */
  text **speed = path2("speed", NULL);
  Temporal *tjb = tjsonb_in("\"{\\\"speed\\\": 10}\"@2001-01-01");
  Temporal *tj = ttext_in("\"{\\\"speed\\\": 10}\"@2001-01-01");
  Set *jbset = jsonbset_in("{\"{\\\"speed\\\": 10}\"}");
  assert(tjb); assert(tj); assert(jbset);
  expect_null(tjsonb_extract_path(tjb, speed, 2, NULL_JSON_NULL),
    "tjsonb_extract_path({speed, NULL})");
  expect_null(tjsonb_extract_path_text(tjb, speed, 2, NULL_JSON_NULL),
    "tjsonb_extract_path_text({speed, NULL})");
  expect_null(tjson_extract_path(tj, speed, 2, NULL_JSON_NULL),
    "tjson_extract_path({speed, NULL})");
  expect_null(jsonbset_extract_path(jbset, speed, 2, NULL_JSON_NULL),
    "jsonbset_extract_path({speed, NULL})");
  expect_null(jsonbset_extract_path_text(jbset, speed, 2, NULL_JSON_NULL),
    "jsonbset_extract_path_text({speed, NULL})");
  Temporal *speed_t = tjsonb_extract_path(tjb, speed, 1, NULL_JSON_NULL);
  Set *speed_s = jsonbset_extract_path(jbset, speed, 1, NULL_JSON_NULL);
  printf("tjsonb_extract_path({speed}): %s, jsonbset_extract_path({speed}): %s\n",
    speed_t ? "a value" : "NULL", speed_s ? "a value" : "NULL");
  assert(speed_t); assert(speed_s);
  free(speed_t); free(speed_s);

  path2_free(a_null); path2_free(null_a); path2_free(x_null);
  path2_free(null_null); path2_free(speed);
  free(jb); free(three); free(js); free(tjb); free(tj); free(jbset);

  meos_finalize();
  printf("jsonb_null_path_test: all checks passed\n");
  return 0;
}
