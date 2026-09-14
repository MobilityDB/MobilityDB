/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief A program that tests the typed unnest functions of the temporal
 * types.
 *
 * Each temporal type gives its distinct values, and the span set on which it
 * takes each of them, through a typed public function that returns the span
 * sets and hands the values back in a parallel array, as tint_value_split does
 * for its bins. The program verifies that the values come in the order of the
 * base type, that a value taken twice carries both of its times, that a
 * temporal rigid geometry answers the placements of its reference geometry,
 * and that a linear value, a null value and a value of another type are
 * refused.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o temporal_unnest_test temporal_unnest_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_rgeo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  int count;

  /* Integers: the values come ordered and distinct, 2 carrying two times */
  Temporal *tint = tint_in("{5@2001-01-01, 2@2001-01-02, 9@2001-01-03, "
    "2@2001-01-04}");
  int *ivals;
  SpanSet **iss = tint_unnest(tint, &ivals, &count);
  printf("tint_unnest: %d values {%d, %d, %d}, errno %d\n", count,
    count > 0 ? ivals[0] : -1, count > 1 ? ivals[1] : -1,
    count > 2 ? ivals[2] : -1, meos_errno());
  assert(iss && count == 3 && meos_errno() == 0);
  assert(ivals[0] == 2 && ivals[1] == 5 && ivals[2] == 9);
  assert(spanset_num_spans(iss[0]) == 2 && spanset_num_spans(iss[1]) == 1 &&
    spanset_num_spans(iss[2]) == 1);
  for (int i = 0; i < count; i++)
    free(iss[i]);
  free(iss); free(ivals);

  /* Booleans: false before true, true carrying two times */
  Temporal *tbool = tbool_in("{true@2001-01-01, false@2001-01-02, "
    "true@2001-01-03}");
  bool *bvals;
  SpanSet **bss = tbool_unnest(tbool, &bvals, &count);
  printf("tbool_unnest: %d values, errno %d\n", count, meos_errno());
  assert(bss && count == 2 && meos_errno() == 0);
  assert(! bvals[0] && bvals[1]);
  assert(spanset_num_spans(bss[0]) == 1 && spanset_num_spans(bss[1]) == 2);
  for (int i = 0; i < count; i++)
    free(bss[i]);
  free(bss); free(bvals); free(tbool);

  /* Points under step interpolation: the point taken twice carries both
   * periods */
  Temporal *tpoint = tgeompoint_in("Interp=Step;[Point(1 1)@2001-01-01, "
    "Point(2 2)@2001-01-02, Point(1 1)@2001-01-03]");
  GSERIALIZED **gvals;
  SpanSet **gss = tgeo_unnest(tpoint, &gvals, &count);
  printf("tgeo_unnest: %d values, errno %d\n", count, meos_errno());
  assert(gss && count == 2 && meos_errno() == 0);
  assert(spanset_num_spans(gss[0]) + spanset_num_spans(gss[1]) == 3);
  for (int i = 0; i < count; i++)
  {
    free(gss[i]); free(gvals[i]);
  }
  free(gss); free(gvals); free(tpoint);

  /* Rigid geometry: a repeated pose names its placement once, with both of
   * its times, and the placement is the moved reference geometry */
  Temporal *trgeo = trgeometry_in("Polygon((0 0,1 0,1 1,0 1,0 0));"
    "{Pose(Point(0 0), 0.0)@2001-01-01, Pose(Point(4 0), 0.0)@2001-01-02, "
    "Pose(Point(0 0), 0.0)@2001-01-03}");
  GSERIALIZED **rvals;
  SpanSet **rss = trgeometry_unnest(trgeo, &rvals, &count);
  printf("trgeometry_unnest: %d values, errno %d\n", count, meos_errno());
  assert(rss && count == 2 && meos_errno() == 0);
  assert(spanset_num_spans(rss[0]) + spanset_num_spans(rss[1]) == 3);
  for (int i = 0; i < count; i++)
  {
    char *str = geo_as_text(rvals[i], 6);
    printf("  placement %d: %s, %d span(s)\n", i, str,
      spanset_num_spans(rss[i]));
    assert(str && strncmp(str, "POLYGON", 7) == 0);
    free(str); free(rss[i]); free(rvals[i]);
  }
  free(rss); free(rvals); free(trgeo);

  /* Pose chains: the repeated chain carries both of its times */
  Temporal *tpc = tposechain_in("{PoseChain(Pose(Point(0 0), 0))@2001-01-01, "
    "PoseChain(Pose(Point(1 0), 0))@2001-01-02, "
    "PoseChain(Pose(Point(0 0), 0))@2001-01-03}");
  PoseChain **pvals;
  SpanSet **pss = tposechain_unnest(tpc, &pvals, &count);
  printf("tposechain_unnest: %d values, errno %d\n", count, meos_errno());
  assert(pss && count == 2 && meos_errno() == 0);
  assert(spanset_num_spans(pss[0]) + spanset_num_spans(pss[1]) == 3);
  for (int i = 0; i < count; i++)
  {
    free(pss[i]); free(pvals[i]);
  }
  free(pss); free(pvals); free(tpc);

  /* A linear value has no distinct values to name, so it is refused */
  Temporal *tfloat = tfloat_in("[1@2001-01-01, 2@2001-01-02]");
  double *fvals;
  SpanSet **fss = tfloat_unnest(tfloat, &fvals, &count);
  printf("tfloat_unnest(linear): %s, count %d, errno %d\n",
    fss ? "a value" : "NULL", count, meos_errno());
  assert(fss == NULL && count == 0 && meos_errno() != 0);
  meos_errno_reset();

  /* A null value and a value of another type are refused, each with its own
   * error */
  SpanSet **res = tint_unnest(NULL, &ivals, &count);
  printf("tint_unnest(NULL): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();
  res = tint_unnest(tfloat, &ivals, &count);
  printf("tint_unnest(tfloat): %s, errno %d\n", res ? "a value" : "NULL",
    meos_errno());
  assert(res == NULL && meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  free(tint); free(tfloat);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
