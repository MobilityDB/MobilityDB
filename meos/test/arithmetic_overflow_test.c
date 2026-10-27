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
 * @brief A program that tests that the arithmetic of temporal numbers and the
 * shift of spans report a result outside the range of their base type
 * @details Each operation is called once with a result outside the range of
 * its base type, which must report an error through meos_errno, and once with
 * a result inside it, which must not: the addition, subtraction,
 * multiplication and division of temporal integers and big integers, the
 * multiplication of temporal floats reaching infinity from finite values, the
 * division of the smallest integer by -1, and the shift of an integer span and
 * of a date span, and the conversion of temporal floats to temporal integers
 * and big integers.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o arithmetic_overflow_test arithmetic_overflow_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

static int failures;

/* Report whether a call reported an error and count an unexpected answer */
static void
ovfcheck_report(const char *what, bool expect_error)
{
  bool error = meos_errno() != 0;
  printf("%s: %s\n", what, error ? "error reported" : "no error");
  if (error != expect_error)
  {
    printf("  expected %s\n", expect_error ? "an error" : "no error");
    failures++;
  }
  meos_errno_reset();
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Temporal *imax = tint_in("2147483646@2001-01-01");
  Temporal *imin = tint_in("-2147483648@2001-01-01");
  Temporal *bmax = tbigint_in("9223372036854775806@2001-01-01");
  Temporal *fbig = tfloat_in("1e300@2001-01-01");
  meos_errno_reset();
  Temporal *r;

  r = add_tint_int(imax, 2); ovfcheck_report("tint (INT_MAX - 1) + 2", true); free(r);
  r = add_tint_int(imax, -1); ovfcheck_report("tint (INT_MAX - 1) + -1", false); free(r);
  r = sub_tint_int(imin, 1); ovfcheck_report("tint INT_MIN - 1", true); free(r);
  r = sub_tint_int(imin, -1); ovfcheck_report("tint INT_MIN - -1", false); free(r);
  r = mul_tint_int(imax, 2); ovfcheck_report("tint (INT_MAX - 1) * 2", true); free(r);
  r = mul_tint_int(imax, 1); ovfcheck_report("tint (INT_MAX - 1) * 1", false); free(r);
  r = div_tint_int(imin, -1); ovfcheck_report("tint INT_MIN / -1", true); free(r);
  r = div_tint_int(imin, 1); ovfcheck_report("tint INT_MIN / 1", false); free(r);
  r = add_tbigint_bigint(bmax, 2); ovfcheck_report("tbigint (INT64_MAX - 1) + 2", true); free(r);
  r = add_tbigint_bigint(bmax, -1); ovfcheck_report("tbigint (INT64_MAX - 1) + -1", false); free(r);
  r = mul_tbigint_bigint(bmax, 2); ovfcheck_report("tbigint (INT64_MAX - 1) * 2", true); free(r);
  r = mul_tfloat_float(fbig, 1e300); ovfcheck_report("tfloat 1e300 * 1e300", true); free(r);
  r = mul_tfloat_float(fbig, 2.0); ovfcheck_report("tfloat 1e300 * 2", false); free(r);
  Temporal *fout = tfloat_in("3e9@2001-01-01");
  Temporal *fin = tfloat_in("-2147483648.5@2001-01-01");
  r = tfloat_to_tint(fout); ovfcheck_report("tfloat 3e9 to tint", true); free(r);
  r = tfloat_to_tint(fin); ovfcheck_report("tfloat INT_MIN - 0.5 to tint", false); free(r);
  r = tfloat_to_tbigint(fbig); ovfcheck_report("tfloat 1e300 to tbigint", true); free(r);
  r = tfloat_to_tbigint(fout); ovfcheck_report("tfloat 3e9 to tbigint", false); free(r);
  free(fout); free(fin);

  Span *ispan = intspan_make(1, INT_MAX - 1000, true, false);
  Span *s = intspan_shift_scale(ispan, 2000, 0, true, false);
  ovfcheck_report("intspan [1, INT_MAX - 1000) shifted by 2000", true); free(s);
  s = intspan_shift_scale(ispan, 10, 0, true, false);
  ovfcheck_report("intspan [1, INT_MAX - 1000) shifted by 10", false); free(s);
  Span *dspan = datespan_make(0, 10, true, false);
  s = datespan_shift_scale(dspan, INT_MAX - 5, 0, true, false);
  ovfcheck_report("datespan shifted by INT_MAX - 5 days", true); free(s);
  s = datespan_shift_scale(dspan, 10, 0, true, false);
  ovfcheck_report("datespan shifted by 10 days", false); free(s);

  free(imax); free(imin); free(bmax); free(fbig); free(ispan); free(dspan);
  printf("%d answer(s) other than expected\n", failures);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
