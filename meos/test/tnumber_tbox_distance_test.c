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
 * @brief A program that checks the nearest approach distance of a temporal
 * number and a temporal box against the distance of their boxes
 * @details The temporal number restricted to the period of the box, when the
 * box has one, has a box whose distance to the box (#nad_tboxint_tboxint, ...)
 * is the nearest approach distance; over a grid of temporal numbers and boxes
 * on either side of them, below, above and across, each answer of
 * #nad_tint_tbox, #nad_tbigint_tbox and #nad_tfloat_tbox is that distance.
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o tnumber_tbox_distance_test tnumber_tbox_distance_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>

/* The temporal number restricted to the period of the box, if it has one */
static Temporal *
restrict_to_box(const Temporal *temp, const TBox *box)
{
  if (! tbox_hast(box))
    return temporal_copy(temp);
  Span *p = tbox_to_tstzspan(box);
  Temporal *result = temporal_at_tstzspan(temp, p);
  free(p);
  return result;
}

/* Main program */
int main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* The witnesses: a box below the temporal number and a box above it */
  Temporal *t05 = tint_in("[0@2001-01-01, 5@2001-01-03]");
  TBox *below = tbox_in("TBOXINT X([-5,-4))");
  TBox *above = tbox_in("TBOXINT X([10,12))");
  printf("nad_tint_tbox(0..5, [-5,-4)): %d\n", nad_tint_tbox(t05, below));
  printf("nad_tint_tbox(0..5, [10,12)): %d\n", nad_tint_tbox(t05, above));
  assert(nad_tint_tbox(t05, below) == 5);
  assert(nad_tint_tbox(t05, above) == 5);
  Temporal *f05 = tfloat_in("[0@2001-01-01, 5@2001-01-03]");
  TBox *fbelow = tbox_in("TBOXFLOAT X([-5,-4])");
  printf("nad_tfloat_tbox(0..5, [-5,-4]): %g\n", nad_tfloat_tbox(f05, fbelow));
  assert(nad_tfloat_tbox(f05, fbelow) == 4.0);
  free(t05); free(below); free(above); free(f05); free(fbelow);

  /* The grid */
  const char *seqs[] = {"[%d@2001-01-01, %d@2001-01-03]",
    "{[%d@2001-01-01, %d@2001-01-02], [%d@2001-01-04, %d@2001-01-05]}"};
  const char *periods[] = {"", "[2001-01-01, 2001-01-02]",
    "[2001-01-02, 2001-01-05]"};
  char buf[256];
  long n = 0, bad = 0, apart = 0;
  for (int v = -6; v <= 6; v += 3)
    for (int s = 0; s < 2; s++)
      for (int lo = -12; lo <= 12; lo += 2)
        for (int w = 1; w <= 3; w++)
          for (int p = 0; p < 3; p++)
          {
            if (s == 0)
              snprintf(buf, sizeof(buf), seqs[0], v, v + 4);
            else
              snprintf(buf, sizeof(buf), seqs[1], v, v + 2, v + 1, v + 4);
            Temporal *ti = tint_in(buf);
            Temporal *tb = tbigint_in(buf);
            Temporal *tf = tfloat_in(buf);
            char xt[16], boxi[128], boxb[128], boxf[128];
            snprintf(xt, sizeof(xt), "%s", p ? "XT" : "X");
            snprintf(boxi, sizeof(boxi), "TBOXINT %s([%d,%d)%s%s)", xt, lo,
              lo + w, p ? "," : "", periods[p]);
            snprintf(boxb, sizeof(boxb), "TBOXBIGINT %s([%d,%d)%s%s)", xt, lo,
              lo + w, p ? "," : "", periods[p]);
            snprintf(boxf, sizeof(boxf), "TBOXFLOAT %s([%d,%d]%s%s)", xt, lo,
              lo + w, p ? "," : "", periods[p]);
            TBox *bi = tbox_in(boxi), *bb = tbox_in(boxb), *bf = tbox_in(boxf);

            Temporal *ri = restrict_to_box(ti, bi);
            Temporal *rb = restrict_to_box(tb, bb);
            Temporal *rf = restrict_to_box(tf, bf);
            if (ri)
            {
              TBox *xi = tnumber_to_tbox(ri), *xb = tnumber_to_tbox(rb),
                *xf = tnumber_to_tbox(rf);
              int ei = nad_tboxint_tboxint(xi, bi);
              int64 eb = nad_tboxbigint_tboxbigint(xb, bb);
              double ef = nad_tboxfloat_tboxfloat(xf, bf);
              int ai = nad_tint_tbox(ti, bi);
              int64 ab = nad_tbigint_tbox(tb, bb);
              double af = nad_tfloat_tbox(tf, bf);
              if (ai != ei || ab != eb || af != ef)
              {
                if (bad < 5)
                  printf("%s %s: tint %d box %d, tbigint %ld box %ld, "
                    "tfloat %g box %g\n", buf, boxi, ai, ei, (long) ab,
                    (long) eb, af, ef);
                bad++;
              }
              apart += (ei != 0);
              n += 3;
              free(xi); free(xb); free(xf);
              free(ri); free(rb); free(rf);
            }
            free(ti); free(tb); free(tf); free(bi); free(bb); free(bf);
          }
  printf("tnumber_tbox_distance_test: %ld answers, %ld with the operands "
    "apart, %ld cases disagreeing with the distance of the boxes\n", n,
    apart, bad);
  assert(apart > 0);
  assert(bad == 0);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
