/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief A program that tests how the temporal JSON and JSONB set functions of
 * the MEOS API report an invalid argument under the noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer or a value of the wrong type receives
 * an error it can raise in its host language. An assertion cannot carry that
 * contract: it is compiled out under NDEBUG, which leaves the release build a
 * binding links against with no check at all.
 *
 * The program verifies that #tjsonb_to_ttext, #ttext_to_tjsonb,
 * #tjsonb_pretty, #tjsonb_strip_nulls, #tjson_strip_nulls, #jsonbset_pretty,
 * #jsonbset_strip_nulls and #jsonbset_to_alphanumset report a null argument
 * and an argument of another type by returning NULL and setting #meos_errno,
 * that #jsonbset_to_alphanumset reports a result base type that is not
 * alphanumeric the same way, and that a valid call still answers with no error
 * left behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_validity_test json_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>

/**
 * @brief Check that a call answered NULL with the expected error, then clear it
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

  Temporal *tjsonb = tjsonb_in("\"{\\\"a\\\": 1, \\\"b\\\": null}\"@2001-01-01");
  Temporal *tjson = ttext_in("\"{\\\"a\\\": 1, \\\"b\\\": null}\"@2001-01-01");
  Set *jsonbset = jsonbset_in("{\"{\\\"a\\\": 1, \\\"b\\\": null}\"}");
  Temporal *tint = tint_in("1@2001-01-01");
  Set *intset = intset_in("{1, 2}");
  assert(tjsonb); assert(tjson); assert(jsonbset); assert(tint); assert(intset);
  meos_errno_reset();

  /* A null argument is reported rather than dereferenced */
  expect_error(tjsonb_to_ttext(NULL), MEOS_ERR_INVALID_ARG,
    "tjsonb_to_ttext(NULL)");
  expect_error(ttext_to_tjsonb(NULL), MEOS_ERR_INVALID_ARG,
    "ttext_to_tjsonb(NULL)");
  expect_error(tjsonb_pretty(NULL), MEOS_ERR_INVALID_ARG,
    "tjsonb_pretty(NULL)");
  expect_error(tjsonb_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "tjsonb_strip_nulls(NULL, true)");
  expect_error(tjson_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "tjson_strip_nulls(NULL, true)");
  expect_error(jsonbset_pretty(NULL), MEOS_ERR_INVALID_ARG,
    "jsonbset_pretty(NULL)");
  expect_error(jsonbset_strip_nulls(NULL, true), MEOS_ERR_INVALID_ARG,
    "jsonbset_strip_nulls(NULL, true)");
  expect_error(jsonbset_to_alphanumset(NULL, "a", T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "jsonbset_to_alphanumset(NULL, a, int4)");
  expect_error(jsonbset_to_alphanumset(jsonbset, NULL, T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG, "jsonbset_to_alphanumset(jsonbset, NULL, int4)");

  /* An argument of another type is reported as a type error: each function
   * reads the values of its argument as JSON or text, so accepting an integer
   * would read one type's bytes as another's */
  expect_error(tjsonb_to_ttext(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_to_ttext(1@2001-01-01)");
  expect_error(ttext_to_tjsonb(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "ttext_to_tjsonb(1@2001-01-01)");
  expect_error(tjsonb_pretty(tint), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_pretty(1@2001-01-01)");
  expect_error(tjsonb_strip_nulls(tint, true), MEOS_ERR_INVALID_ARG_TYPE,
    "tjsonb_strip_nulls(1@2001-01-01, true)");
  expect_error(tjson_strip_nulls(tint, true), MEOS_ERR_INVALID_ARG_TYPE,
    "tjson_strip_nulls(1@2001-01-01, true)");
  expect_error(jsonbset_pretty(intset), MEOS_ERR_INVALID_ARG_TYPE,
    "jsonbset_pretty({1, 2})");
  expect_error(jsonbset_strip_nulls(intset, true), MEOS_ERR_INVALID_ARG_TYPE,
    "jsonbset_strip_nulls({1, 2}, true)");
  expect_error(jsonbset_to_alphanumset(intset, "a", T_INT4, NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_to_alphanumset({1, 2}, a, int4)");
  /* A result base type that is not alphanumeric is reported alike: the values
   * extracted from the JSON are converted to that type */
  expect_error(jsonbset_to_alphanumset(jsonbset, "a", T_TSTZSPAN, NULL_RETURN),
    MEOS_ERR_INVALID_ARG_TYPE, "jsonbset_to_alphanumset(jsonbset, a, tstzspan)");

  /* A valid call still answers, and the guards leave no error behind */
  expect_value(tjsonb_to_ttext(tjsonb), "tjsonb_to_ttext(tjsonb)");
  expect_value(ttext_to_tjsonb(tjson), "ttext_to_tjsonb(ttext)");
  expect_value(tjsonb_pretty(tjsonb), "tjsonb_pretty(tjsonb)");
  expect_value(tjsonb_strip_nulls(tjsonb, true),
    "tjsonb_strip_nulls(tjsonb, true)");
  expect_value(tjson_strip_nulls(tjson, true), "tjson_strip_nulls(ttext, true)");
  expect_value(jsonbset_pretty(jsonbset), "jsonbset_pretty(jsonbset)");
  expect_value(jsonbset_strip_nulls(jsonbset, true),
    "jsonbset_strip_nulls(jsonbset, true)");
  expect_value(jsonbset_to_alphanumset(jsonbset, "a", T_INT4, NULL_RETURN),
    "jsonbset_to_alphanumset(jsonbset, a, int4)");

  free(tjsonb); free(tjson); free(jsonbset); free(tint); free(intset);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
