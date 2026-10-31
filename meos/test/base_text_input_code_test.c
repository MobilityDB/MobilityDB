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
 * @brief A program that tests that the text input of the base values reports
 * a malformed value as a text input error
 * @details The Boolean, integer, float, numeric and date and time inputs read
 * a base value from its text, alone or as an element of a set or a temporal
 * value. A text they cannot read is malformed input, which MEOS reports with
 * #MEOS_ERR_TEXT_INPUT as its own parsers do, so that a binding raises the
 * error of a text input.
 *
 * The program reads one malformed text through each input, verifies that
 * #meos_errno is #MEOS_ERR_TEXT_INPUT, and reads a well-formed text that must
 * be accepted with no error.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o base_text_input_code_test base_text_input_code_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <pg_bool.h>
#include <pg_float.h>
#include <pg_int.h>
#include <pg_numeric.h>

static int failures = 0;

/* Verify the error the last input left, then clear it */
static void
check(const char *what, int expected)
{
  int err = meos_errno();
  if (err != expected)
  {
    printf("  %s: expected error %d, got %d\n", what, expected, err);
    failures++;
  }
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  meos_errno_reset();

  /* Each input, a malformed text and then a well-formed one */
  bool_in("maybe");
  check("bool_in(\"maybe\")", MEOS_ERR_TEXT_INPUT);
  bool_in("true");
  check("bool_in(\"true\")", 0);

  int32_in("x");
  check("int32_in(\"x\")", MEOS_ERR_TEXT_INPUT);
  int32_in("12");
  check("int32_in(\"12\")", 0);

  int64_in("x");
  check("int64_in(\"x\")", MEOS_ERR_TEXT_INPUT);
  int64_in("12");
  check("int64_in(\"12\")", 0);

  float8_in("x");
  check("float8_in(\"x\")", MEOS_ERR_TEXT_INPUT);
  float8_in("");
  check("float8_in(\"\")", MEOS_ERR_TEXT_INPUT);
  float8_in("1.5");
  check("float8_in(\"1.5\")", 0);

  Numeric n = numeric_in("x", -1);
  check("numeric_in(\"x\")", MEOS_ERR_TEXT_INPUT);
  free(n);
  n = numeric_in("1.5", -1);
  check("numeric_in(\"1.5\")", 0);
  free(n);

  timestamptz_in("x", -1);
  check("timestamptz_in(\"x\")", MEOS_ERR_TEXT_INPUT);
  timestamptz_in("2001-01-01", -1);
  check("timestamptz_in(\"2001-01-01\")", 0);

  /* A malformed element of a set */
  Set *s = floatset_in("{x}");
  check("floatset_in(\"{x}\")", MEOS_ERR_TEXT_INPUT);
  free(s);
  s = intset_in("{x}");
  check("intset_in(\"{x}\")", MEOS_ERR_TEXT_INPUT);
  free(s);
  s = floatset_in("{1.5}");
  check("floatset_in(\"{1.5}\")", 0);
  free(s);

  printf("%d check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
