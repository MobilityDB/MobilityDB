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
 * @brief A program that checks the nearest approach distance of a number span
 * to a temporal box and of a temporal number to a number span against the
 * distance through the box of the span
 * @details Over a grid of spans, boxes and temporal numbers on either side of
 * each other, each answer of #nad_intspan_tbox, #nad_bigintspan_tbox and
 * #nad_floatspan_tbox is the distance of the box of the span to the box
 * (#nad_tboxint_tboxint, ...), and each answer of #nad_tint_intspan,
 * #nad_tbigint_bigintspan and #nad_tfloat_floatspan the distance of the
 * temporal number to the box of the span (#nad_tint_tbox, ...).
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o span_distance_test span_distance_test.c -L/usr/local/lib -lmeos
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
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* The witnesses: a span below and a span above a temporal number */
  Temporal *t05 = tint_in("[0@2001-01-01, 5@2001-01-03]");
  Span *below = intspan_in("[-5,-4)");
  Span *above = intspan_in("[10,12)");
  TBox *box = tbox_in("TBOXINT XT([0,6),[2001-01-01,2001-01-03])");
  printf("nad_tint_intspan(0..5, [-5,-4)): %d\n", nad_tint_intspan(t05, below));
  printf("nad_intspan_tbox([10,12), [0,6)): %d\n", nad_intspan_tbox(above, box));
  assert(nad_tint_intspan(t05, below) == 5);
  assert(nad_tint_intspan(t05, above) == 5);
  assert(nad_intspan_tbox(below, box) == 5);
  assert(nad_intspan_tbox(above, box) == 5);

  /* The refusals, each answering the error value -1 */
  Span *fspan = floatspan_in("[1,2]");
  TBox *timebox = tbox_in("TBOX T([2001-01-01,2001-01-02])");
  assert(nad_intspan_tbox(fspan, box) == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE); meos_errno_reset();
  assert(nad_floatspan_tbox(fspan, box) == -1.0);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE); meos_errno_reset();
  assert(nad_intspan_tbox(below, timebox) == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE); meos_errno_reset();
  assert(nad_intspan_tbox(NULL, box) == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG); meos_errno_reset();
  assert(nad_tint_intspan(t05, fspan) == -1);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE); meos_errno_reset();
  free(fspan); free(timebox);

  /* The grid */
  const char *periods[] = {"", "[2001-01-01, 2001-01-02]",
    "[2001-01-04, 2001-01-05]"};
  char buf[256], sbuf[64], fbuf[64];
  long n = 0, bad = 0, apart = 0;
  for (int v = -6; v <= 6; v += 3)
    for (int lo = -12; lo <= 12; lo += 2)
      for (int w = 1; w <= 3; w++)
        for (int p = 0; p < 3; p++)
        {
          snprintf(buf, sizeof(buf), "[%d@2001-01-01, %d@2001-01-03]", v,
            v + 4);
          Temporal *ti = tint_in(buf), *tb = tbigint_in(buf),
            *tf = tfloat_in(buf);
          snprintf(sbuf, sizeof(sbuf), "[%d,%d)", lo, lo + w);
          snprintf(fbuf, sizeof(fbuf), "[%d,%d]", lo, lo + w);
          Span *si = intspan_in(sbuf), *sb = bigintspan_in(sbuf),
            *sf = floatspan_in(fbuf);
          char xt[4], bi_in[128], bb_in[128], bf_in[128];
          snprintf(xt, sizeof(xt), "%s", p ? "XT" : "X");
          snprintf(bi_in, sizeof(bi_in), "TBOXINT %s([%d,%d)%s%s)", xt, v,
            v + 5, p ? "," : "", periods[p]);
          snprintf(bb_in, sizeof(bb_in), "TBOXBIGINT %s([%d,%d)%s%s)", xt, v,
            v + 5, p ? "," : "", periods[p]);
          snprintf(bf_in, sizeof(bf_in), "TBOXFLOAT %s([%d,%d]%s%s)", xt, v,
            v + 4, p ? "," : "", periods[p]);
          TBox *bi = tbox_in(bi_in), *bb = tbox_in(bb_in), *bf = tbox_in(bf_in);
          TBox *xi = span_to_tbox(si), *xb = span_to_tbox(sb),
            *xf = span_to_tbox(sf);

          int e1 = nad_tboxint_tboxint(xi, bi), a1 = nad_intspan_tbox(si, bi);
          int64 e2 = nad_tboxbigint_tboxbigint(xb, bb),
            a2 = nad_bigintspan_tbox(sb, bb);
          double e3 = nad_tboxfloat_tboxfloat(xf, bf),
            a3 = nad_floatspan_tbox(sf, bf);
          int e4 = nad_tint_tbox(ti, xi), a4 = nad_tint_intspan(ti, si);
          int64 e5 = nad_tbigint_tbox(tb, xb),
            a5 = nad_tbigint_bigintspan(tb, sb);
          double e6 = nad_tfloat_tbox(tf, xf), a6 = nad_tfloat_floatspan(tf, sf);
          if (a1 != e1 || a2 != e2 || a3 != e3 || a4 != e4 || a5 != e5 ||
              a6 != e6)
          {
            if (bad < 5)
              printf("%s %s %s: %d/%d %ld/%ld %g/%g %d/%d %ld/%ld %g/%g\n",
                buf, sbuf, bi_in, a1, e1, (long) a2, (long) e2, a3, e3, a4, e4,
                (long) a5, (long) e5, a6, e6);
            bad++;
          }
          apart += (e1 > 0) + (e4 > 0);
          n += 6;
          free(ti); free(tb); free(tf); free(si); free(sb); free(sf);
          free(bi); free(bb); free(bf); free(xi); free(xb); free(xf);
        }
  printf("span_distance_test: %ld answers, %ld with the operands apart, "
    "%ld cases disagreeing with the distance through the box of the span\n",
    n, apart, bad);
  assert(apart > 0);
  assert(bad == 0);

  free(t05); free(below); free(above); free(box);
  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
