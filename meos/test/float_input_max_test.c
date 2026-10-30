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
 * @brief A program that tests that the text input of a float accepts the
 * largest double and refuses a malformed or out-of-range number with an error
 * @details The largest double, 1.7976931348623157e308, is a value of the
 * domain of a float, as the largest integer is a value of the domain of an
 * integer. The program reads it, its negative and the double below it in a
 * float set, a float span, a temporal float and the radius of a circular
 * buffer, and verifies that each is accepted and reads back the same double.
 * It then reads malformed and out-of-range numbers in the same inputs and
 * verifies that each is refused with #meos_errno set, so that a refusal is
 * always reported.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o float_input_max_test float_input_max_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_cbuffer.h>

static int failures = 0;

/* Read a number in each float input; accepted ones must read back value */
static void
check(const char *num, bool accept, double value)
{
  char buf[256];
  const char *forms[4] = {"{%s}", "[%s, %s]", "%s@2001-01-01",
    "Cbuffer(Point(1 1), %s)"};
  for (int k = 0; k < 4; k++)
  {
    /* A span needs its upper bound at least its lower one: repeat the number */
    snprintf(buf, sizeof(buf), forms[k], num, num);
    meos_errno_reset();
    void *v = NULL;
    double got = NAN;
    switch (k)
    {
      case 0:
      {
        Set *s = floatset_in(buf);
        if (s)
          got = floatset_start_value(s);
        v = s;
        break;
      }
      case 1:
      {
        Span *s = floatspan_in(buf);
        if (s)
          got = floatspan_lower(s);
        v = s;
        break;
      }
      case 2:
      {
        Temporal *t = tfloat_in(buf);
        if (t)
          got = tfloat_start_value(t);
        v = t;
        break;
      }
      default:
      {
        /* A radius is never negative: test the positive numbers only */
        if (num[0] == '-')
          continue;
        Cbuffer *cb = cbuffer_in(buf);
        if (cb)
          got = cbuffer_radius(cb);
        v = cb;
        break;
      }
    }
    int err = meos_errno();
    bool ok = accept ? (v && err == 0 && got == value) : (! v && err != 0);
    if (! ok)
    {
      printf("  %s: expected %s, got %s (errno %d)\n", buf,
        accept ? "the value" : "an error", v ? "a value" : "no value", err);
      failures++;
    }
    free(v);
    meos_errno_reset();
  }
}

/* Main program */
int main(void)
{
  /* Install the error handler that reports through meos_errno instead of
   * exiting, after meos_initialize, which installs the default one */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The largest double, its negative and the double below it */
  check("1.7976931348623157e308", true, DBL_MAX);
  check("-1.7976931348623157e308", true, -DBL_MAX);
  check("1.7976931348623156e308", true, nextafter(DBL_MAX, 0));
  check("1e308", true, 1e308);

  /* Malformed and out-of-range numbers */
  check("x", false, 0);
  check("1e309", false, 0);
  check("-1e309", false, 0);
  check("1e-400", false, 0);

  printf("%d check(s) failed\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
