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
 * @brief A program that tests how the text input and output functions of
 * JSONB values and JSON paths of the MEOS API report a null argument under
 * the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language instead of a dereference of the pointer.
 *
 * The program verifies that #jsonb_in, #jsonb_out, #jsonb_from_text,
 * #jsonb_to_text, #jsonpath_in and #jsonpath_out report a null argument by
 * returning NULL and setting #meos_errno, and that a valid call still answers
 * with no error left behind.
 *
 * It also verifies that an object whose key is followed by no colon, read by
 * #jsonb_in and by #tjsonb_in between valid values, is reported as an invalid
 * value and frees its key once, so that the values read after it still
 * answer, and that a JSON path the grammar refuses is reported as an invalid
 * value whatever the stack held before the call. Run under valgrind, the
 * program reports no invalid free.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o json_inout_validity_test json_inout_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/**
 * @brief Fill a region of the stack with a pattern that is no null pointer,
 * so that a variable a later call reads before assigning it holds a value
 * that cannot pass for an absent one
 * @note No test of meos/test fills the stack; this one runs before
 * #expect_error reads a call that may read such a variable
 */
static __attribute__((noinline)) void
fill_stack(void)
{
  volatile unsigned char junk[16384];
  memset((void *) junk, 0x5A, sizeof(junk));
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Jsonb *jb = jsonb_in("{\"a\": 1, \"b\": [1, 2]}");
  JsonPath *jp = jsonpath_in("$.b[*] ? (@ > 1)");
  text *txt = text_in("{\"a\": 1}");
  assert(jb); assert(jp); assert(txt);
  meos_errno_reset();

  /* A null argument is reported rather than dereferenced */
  expect_error(jsonb_in(NULL), MEOS_ERR_INVALID_ARG, "jsonb_in(NULL)");
  expect_error(jsonb_out(NULL), MEOS_ERR_INVALID_ARG, "jsonb_out(NULL)");
  expect_error(jsonb_from_text(NULL, false), MEOS_ERR_INVALID_ARG,
    "jsonb_from_text(NULL, false)");
  expect_error(jsonb_to_text(NULL), MEOS_ERR_INVALID_ARG,
    "jsonb_to_text(NULL)");
  expect_error(jsonpath_in(NULL), MEOS_ERR_INVALID_ARG, "jsonpath_in(NULL)");
  expect_error(jsonpath_out(NULL), MEOS_ERR_INVALID_ARG,
    "jsonpath_out(NULL)");

  /* A valid call still answers, and the guards leave no error behind */
  expect_value(jsonb_in("[1, \"two\"]"), "jsonb_in([1, \"two\"])");
  expect_value(jsonb_out(jb), "jsonb_out(jsonb)");
  expect_value(jsonb_from_text(txt, false), "jsonb_from_text(text, false)");
  expect_value(jsonb_to_text(jb), "jsonb_to_text(jsonb)");
  expect_value(jsonpath_in("strict $.a"), "jsonpath_in(strict $.a)");
  expect_value(jsonpath_out(jp), "jsonpath_out(jsonpath)");

  /* An object key followed by no colon, or by a token that is not one, is
   * reported between valid values, in a JSONB and in a temporal JSONB */
  expect_value(jsonb_in("{\"geom\": 1}"), "jsonb_in({\"geom\": 1})");
  expect_error(jsonb_in("{\"geom\"}"), MEOS_ERR_INVALID_ARG_VALUE,
    "jsonb_in({\"geom\"})");
  expect_error(jsonb_in("{\"geom\" ~ 1}"), MEOS_ERR_INVALID_ARG_VALUE,
    "jsonb_in({\"geom\" ~ 1})");
  expect_value(jsonb_in("{\"geom\": 1}"), "jsonb_in({\"geom\": 1})");
  expect_value(tjsonb_in("\"{\\\"geom\\\": 1}\"@2012-01-01"),
    "tjsonb_in(\"{\\\"geom\\\": 1}\"@2012-01-01)");
  expect_error(tjsonb_in("\"{\\\"geom\\\"}\"@2012-01-01"),
    MEOS_ERR_INVALID_ARG_VALUE, "tjsonb_in(\"{\\\"geom\\\"}\"@2012-01-01)");
  expect_value(tjsonb_in("{\"{\\\"geom\\\": \\\"Point(1 1)\\\"}\"@2012-01-01}"),
    "tjsonb_in({\"{\\\"geom\\\": \\\"Point(1 1)\\\"}\"@2012-01-01})");

  /* A path the grammar refuses is reported whatever the stack held before
   * the call, since the parse assigns no result to a refused path */
  const char *badpaths[] = {"$[", "$.a[NaN]", "NaN", "$.a ? (@ > NaN)",
    "$[1, 2", "$.a ? (@.b == ", "$.datetime(\"yyyy\"",
    "$.a like_regex \"x\" flag", "strict $.a.b[1 to"};
  for (int i = 0; i < 9; i++)
  {
    fill_stack();
    expect_error(jsonpath_in(badpaths[i]), MEOS_ERR_INVALID_ARG_VALUE,
      badpaths[i]);
  }

  free(jb); free(jp); free(txt);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
