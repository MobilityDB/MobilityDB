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
 * @brief A program that tests how the text functions of the MEOS API report a
 * null text under the noexit error handler
 * @details A public MEOS function tests the conditions its internal form
 * asserts, so a binding calling it with a null pointer receives an error it
 * can raise in its host language. An assertion cannot carry that contract: it
 * is compiled out under NDEBUG, which leaves the release build a binding links
 * against with no check at all.
 *
 * The program verifies that #text_lower, #text_upper, #text_initcap and
 * #textcat_text_text report a null text by returning NULL and setting
 * #meos_errno, as #meos_array_get reports a null array, that a valid call
 * still answers with no error left behind, and that the temporal forms
 * #ttext_lower, #ttext_upper, #ttext_initcap and #textcat_ttext_ttext, which
 * read the internal forms at each instant, answer the base answers at each
 * instant.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o text_validity_test text_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>

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
 * @brief Check that a call answered the expected text and left no error
 * behind, then free both
 * @details The output comparison of json_validity_test.c's expect_text()
 */
static void
expect_text(text *res, const char *want, const char *call)
{
  size_t size;
  char *out = res ? text_out(res, &size) : NULL;
  printf("%s: %s, errno %d\n", call, out ? out : "NULL", meos_errno());
  assert(out != NULL);
  assert(strcmp(out, want) == 0);
  assert(meos_errno() == 0);
  free(out); free(res);
}

/**
 * @brief Check that a call answered the expected temporal text and left no
 * error behind, then free both
 * @details The output comparison of json_validity_test.c's expect_text()
 */
static void
expect_ttext(Temporal *res, const char *want, const char *call)
{
  size_t size;
  char *out = res ? temporal_out(res, 0, &size) : NULL;
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
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  text *txt = text_in("hello World");
  text *other = text_in("!");
  Temporal *ttxt = ttext_in("{\"hello World\"@2001-01-01, \"ab\"@2001-01-02}");
  Temporal *tother = ttext_in("{\"!\"@2001-01-01, \"?\"@2001-01-02}");
  assert(txt); assert(other); assert(ttxt); assert(tother);
  meos_errno_reset();

  /* A null text is reported rather than read */
  expect_error(text_lower(NULL), MEOS_ERR_INVALID_ARG, "text_lower(NULL)");
  expect_error(text_upper(NULL), MEOS_ERR_INVALID_ARG, "text_upper(NULL)");
  expect_error(text_initcap(NULL), MEOS_ERR_INVALID_ARG, "text_initcap(NULL)");
  expect_error(textcat_text_text(NULL, other), MEOS_ERR_INVALID_ARG,
    "textcat_text_text(NULL, !)");
  expect_error(textcat_text_text(txt, NULL), MEOS_ERR_INVALID_ARG,
    "textcat_text_text(hello World, NULL)");

  /* A valid call answers with no error left behind */
  expect_text(text_lower(txt), "hello world", "text_lower(hello World)");
  expect_text(text_upper(txt), "HELLO WORLD", "text_upper(hello World)");
  expect_text(text_initcap(txt), "Hello World", "text_initcap(hello World)");
  expect_text(textcat_text_text(txt, other), "hello World!",
    "textcat_text_text(hello World, !)");

  /* The temporal forms answer the base answers at each instant */
  expect_ttext(ttext_lower(ttxt),
    "{\"hello world\"@2001-01-01 00:00:00+00, \"ab\"@2001-01-02 00:00:00+00}",
    "ttext_lower(ttxt)");
  expect_ttext(ttext_upper(ttxt),
    "{\"HELLO WORLD\"@2001-01-01 00:00:00+00, \"AB\"@2001-01-02 00:00:00+00}",
    "ttext_upper(ttxt)");
  expect_ttext(ttext_initcap(ttxt),
    "{\"Hello World\"@2001-01-01 00:00:00+00, \"Ab\"@2001-01-02 00:00:00+00}",
    "ttext_initcap(ttxt)");
  expect_ttext(textcat_ttext_ttext(ttxt, tother),
    "{\"hello World!\"@2001-01-01 00:00:00+00, \"ab?\"@2001-01-02 00:00:00+00}",
    "textcat_ttext_ttext(ttxt, tother)");

  free(txt); free(other); free(ttxt); free(tother);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
