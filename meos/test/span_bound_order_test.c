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
 * @brief A program that tests that the comparison of the lower bounds of two
 * spans and the comparison of their upper bounds order the bounds as the
 * comparison of spans does.
 *
 * Two spans with the same upper bound are ordered by #span_cmp on their lower
 * bound alone, an inclusive lower bound before an exclusive one of the same
 * value, so #span_lower_cmp must answer the same sign; two spans with the same
 * lower bound are ordered by #span_cmp on their upper bound alone, an
 * exclusive upper bound before an inclusive one, so #span_upper_cmp must
 * answer the same sign. Both are internal functions of MEOS, declared here.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o span_bound_order_test span_bound_order_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

extern int span_lower_cmp(const Span *s1, const Span *s2);
extern int span_upper_cmp(const Span *s1, const Span *s2);

static int
sign(int v)
{
  return (v > 0) - (v < 0);
}

/* Main program */
int main(void)
{
  meos_initialize();
  /* The bound values 1 and 2 with both inclusivities, beside the fixed
   * bound 0 below or 3 above them */
  const double values[2] = {1.0, 2.0};
  int cases = 0, failed = 0;
  size_t sz;
  for (int a = 0; a < 4; a++)
    for (int b = 0; b < 4; b++)
    {
      double va = values[a / 2], vb = values[b / 2];
      bool ia = a % 2 == 0, ib = b % 2 == 0;
      /* Spans tying on their upper bound 3: ordered by their lower bound */
      Span *la = floatspan_make(va, 3.0, ia, true);
      Span *lb = floatspan_make(vb, 3.0, ib, true);
      int want = sign(span_cmp(la, lb)), got = sign(span_lower_cmp(la, lb));
      cases++;
      if (got != want)
      {
        failed++;
        printf("FAILED: span_lower_cmp(%s, %s) %d, span_cmp %d\n",
          span_out(la, 1, &sz), span_out(lb, 1, &sz), got, want);
      }
      /* Spans tying on their lower bound 0: ordered by their upper bound */
      Span *ua = floatspan_make(0.0, va, true, ia);
      Span *ub = floatspan_make(0.0, vb, true, ib);
      want = sign(span_cmp(ua, ub));
      got = sign(span_upper_cmp(ua, ub));
      cases++;
      if (got != want)
      {
        failed++;
        printf("FAILED: span_upper_cmp(%s, %s) %d, span_cmp %d\n",
          span_out(ua, 1, &sz), span_out(ub, 1, &sz), got, want);
      }
      free(la); free(lb); free(ua); free(ub);
    }
  meos_finalize();
  printf("span_bound_order_test: %d cases, %d failed\n", cases, failed);
  return failed ? 1 : 0;
}
