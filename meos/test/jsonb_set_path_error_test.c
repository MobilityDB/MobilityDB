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
 * @brief A program that tests that an error raised below the first element
 * of a path ends the path modification of a JSONB value
 * @details Under an error handler that returns, #jsonb_set, #jsonb_insert
 * and #jsonb_delete_path answer NULL with the error PostgreSQL 18 raises for
 * the same path: a NULL element, an array index that is not an integer, or
 * an existing key an insertion would replace, reached at any depth.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o jsonb_set_path_error_test jsonb_set_path_error_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_json.h>
#include <pg_text.h>

#define JB "{\"a\": {\"b\": 1}, \"c\": 2, \"d\": [1, 2]}"

/**
 * @brief Return a path of two elements, a NULL string giving a NULL element
 * @details The path of jsonb_null_path_test.c's path2()
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

  text **a_null = path2("a", NULL);
  text **d_null = path2("d", NULL);
  text **d_x = path2("d", "x");
  text **a_b = path2("a", "b");
  text **d_0 = path2("d", "0");

  /* A NULL element below the first, reached in an object and in an array */
  expect_error(jsonb_set(jb, a_null, 2, three, true),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_set({a, NULL})");
  expect_error(jsonb_set(jb, d_null, 2, three, true),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_set({d, NULL})");
  expect_error(jsonb_delete_path(jb, a_null, 2),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_delete_path({a, NULL})");
  expect_error(jsonb_insert(jb, d_null, 2, three, false),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_insert({d, NULL})");

  /* An array index that is not an integer */
  expect_error(jsonb_set(jb, d_x, 2, three, true),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_set({d, x})");
  expect_error(jsonb_delete_path(jb, d_x, 2),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_delete_path({d, x})");

  /* An insertion that would replace an existing key */
  expect_error(jsonb_insert(jb, a_b, 2, three, false),
    MEOS_ERR_INVALID_ARG_VALUE, "jsonb_insert({a, b})");

  /* The same paths without an error answer as PostgreSQL does */
  expect_jsonb(jsonb_set(jb, a_b, 2, three, true),
    "{\"a\": {\"b\": 3}, \"c\": 2, \"d\": [1, 2]}", "jsonb_set({a, b})");
  expect_jsonb(jsonb_insert(jb, d_0, 2, three, false),
    "{\"a\": {\"b\": 1}, \"c\": 2, \"d\": [3, 1, 2]}", "jsonb_insert({d, 0})");
  expect_jsonb(jsonb_delete_path(jb, d_0, 2),
    "{\"a\": {\"b\": 1}, \"c\": 2, \"d\": [2]}", "jsonb_delete_path({d, 0})");

  path2_free(a_null); path2_free(d_null); path2_free(d_x); path2_free(a_b);
  path2_free(d_0);
  free(jb); free(three);

  meos_finalize();
  printf("jsonb_set_path_error_test: all checks passed\n");
  return 0;
}
