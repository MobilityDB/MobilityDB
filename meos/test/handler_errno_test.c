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
 * @brief A program that tests that a malformed input fails under any error
 * handler
 * @details A host installs an error handler of its own that raises the error
 * in the host's language and records no error number, as the handlers of the
 * bindings do. MEOS records the number before it calls the handler, so a
 * reader that learns of an error from #meos_errno fails under such a handler
 * as under its own: it returns no value, and raises the one error of its
 * input. The program reads a malformed base value through the text readers
 * of the temporal values, sets and spans of each base type a reader parses
 * with #meos_errno, and verifies the error number the handler leaves.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o handler_errno_test handler_errno_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* A handler of a host: it counts the errors and records no error number */
static int errors = 0;
static void
host_handler(int level, int code, const char *msg)
{
  (void) level; (void) code; (void) msg;
  errors++;
}

static int failures = 0, cases = 0;

/* A malformed input returns no value and raises exactly one error, whose
 * number #meos_errno reads */
static void
check(const char *what, void *value)
{
  cases++;
  int err = meos_errno();
  if (value || errors != 1 || err == 0)
  {
    printf("  %s: %s, %d error(s), error number %d\n", what,
      value ? "a value" : "NULL", errors, err);
    failures++;
  }
  free(value);
  errors = 0;
  meos_errno_reset();
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_error_handler(host_handler);

  check("tbool_in", tbool_in("xyz@2001-01-01"));
  check("tint_in", tint_in("xyz@2001-01-01"));
  check("tbigint_in", tbigint_in("xyz@2001-01-01"));
  check("tfloat_in", tfloat_in("xyz@2001-01-01"));
  check("intset_in", intset_in("{1, xyz}"));
  check("bigintset_in", bigintset_in("{1, xyz}"));
  check("floatset_in", floatset_in("{1, xyz}"));
  check("intspan_in", intspan_in("[1, xyz]"));
  check("floatspan_in", floatspan_in("[1, xyz]"));

  printf("%d of %d case(s) failed\n", failures, cases);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
