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
 * @brief A program that tests that the equality of spans and span sets agrees
 * with their comparison and their hash on the zeros of both signs, as the
 * equality of sets does.
 *
 * The doubles -0.0 and 0.0 are equal, so `[-0, 1]` and `[0, 1]` are the same
 * float span, both written `[0, 1]`: #span_cmp answers 0 and #span_hash gives
 * them one hash, and #span_eq, #spanset_eq and #set_eq must answer true.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o span_zero_sign_test span_zero_sign_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* Main program */
int main(void)
{
  meos_initialize();

  Span *neg = floatspan_make(-0.0, 1.0, true, true);
  Span *pos = floatspan_make(0.0, 1.0, true, true);
  printf("span_cmp([-0, 1], [0, 1]) %d, span_eq %d, same hash %d\n",
    span_cmp(neg, pos), span_eq(neg, pos), span_hash(neg) == span_hash(pos));
  assert(span_cmp(neg, pos) == 0);
  assert(span_hash(neg) == span_hash(pos));
  assert(span_eq(neg, pos));
  assert(! span_ne(neg, pos));

  SpanSet *ssneg = span_to_spanset(neg), *sspos = span_to_spanset(pos);
  printf("spanset_cmp %d, spanset_eq %d\n", spanset_cmp(ssneg, sspos),
    spanset_eq(ssneg, sspos));
  assert(spanset_cmp(ssneg, sspos) == 0);
  assert(spanset_eq(ssneg, sspos));

  double vneg[] = {-0.0, 1.0}, vpos[] = {0.0, 1.0};
  Set *sneg = floatset_make(vneg, 2), *spos = floatset_make(vpos, 2);
  printf("set_cmp %d, set_eq %d\n", set_cmp(sneg, spos), set_eq(sneg, spos));
  assert(set_cmp(sneg, spos) == 0);
  assert(set_eq(sneg, spos));

  /* Spans that differ stay unequal */
  Span *other = floatspan_make(0.0, 1.0, false, true);
  assert(! span_eq(pos, other));

  free(neg); free(pos); free(other); free(ssneg); free(sspos);
  free(sneg); free(spos);
  meos_finalize();
  printf("The zeros of both signs make equal spans, span sets and sets\n");
  return EXIT_SUCCESS;
}
