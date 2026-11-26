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
 * @brief A program that tests the splitters of a temporal sequence set into
 * a number of boxes, a span, a temporal box or a spatiotemporal box, asked
 * for as many boxes as the value has segments or more
 * @details The value then answers one box per segment, and running the
 * program under valgrind checks that the splitter allocates no other array
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

int
main(void)
{
  meos_initialize();
  Temporal *tb = tbool_in(
    "{[t@2001-01-01, f@2001-01-02], [t@2001-01-03, f@2001-01-04]}");
  Temporal *tf = tfloat_in(
    "{[1@2001-01-01, 3@2001-01-02], [2@2001-01-03, 5@2001-01-04]}");
  Temporal *tp = tgeompoint_in(
    "{[Point(1 1)@2001-01-01, Point(3 3)@2001-01-02], "
    "[Point(2 2)@2001-01-03, Point(5 5)@2001-01-04]}");
  assert(tb && tf && tp);
  for (int n = 1; n <= 6; n++)
  {
    int c1, c2, c3;
    Span *spans = temporal_split_n_spans(tb, n, &c1);
    TBox *tboxes = tnumber_split_n_tboxes(tf, n, &c2);
    STBox *stboxes = tgeo_split_n_stboxes(tp, n, &c3);
    assert(spans && tboxes && stboxes);
    printf("splitN(%d): %d spans, %d tboxes, %d stboxes\n", n, c1, c2, c3);
    assert(c1 >= 1 && c1 <= n && c2 >= 1 && c2 <= n && c3 >= 1 && c3 <= n);
    free(spans); free(tboxes); free(stboxes);
  }
  free(tb); free(tf); free(tp);
  meos_finalize();
  return EXIT_SUCCESS;
}
