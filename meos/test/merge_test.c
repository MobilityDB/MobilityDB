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
 * @brief A program that tests how the merge functions of the MEOS API report a
 * conflict under the noexit error handler.
 *
 * Merging temporal values that share a timestamp with different values is an
 * error. Under the default error handler the process exits, so the error path
 * is only observable once #meos_initialize_noexit_error_handler is installed —
 * the handler every language binding uses, since a binding must return an
 * exception to its host rather than terminate it. The program verifies that
 * each merge function reports the conflict by returning NULL and setting
 * #MEOS_ERR_INVALID_ARG_VALUE, and that merging compatible values still
 * succeeds afterwards.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o merge_test merge_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  /* Two instant sets sharing a timestamp with different values */
  Temporal *conflict1 = tfloat_in("{77@2000-01-01}");
  Temporal *conflict2 = tfloat_in("{88@2000-01-01}");
  assert(conflict1); assert(conflict2);

  /* temporal_merge_array reports the conflict */
  Temporal *conflictarr[2] = {conflict1, conflict2};
  meos_errno_reset();
  Temporal *merged = temporal_merge_array(conflictarr, 2);
  printf("temporal_merge_array({%s, %s}, 2): NULL, errno %d\n",
    "{77@2000-01-01}", "{88@2000-01-01}", meos_errno());
  assert(merged == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* temporal_merge reports the same conflict */
  meos_errno_reset();
  merged = temporal_merge(conflict1, conflict2);
  printf("temporal_merge(%s, %s): NULL, errno %d\n",
    "{77@2000-01-01}", "{88@2000-01-01}", meos_errno());
  assert(merged == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* A conflict on a shared timestamp of temporal points is reported alike */
  Temporal *point1 = tgeompoint_in("SRID=4326;{POINT(1 1)@2000-01-01}");
  Temporal *point2 = tgeompoint_in("SRID=4326;{POINT(2 2)@2000-01-01}");
  assert(point1); assert(point2);
  Temporal *pointarr[2] = {point1, point2};
  meos_errno_reset();
  merged = temporal_merge_array(pointarr, 2);
  printf("temporal_merge_array({POINT(1 1)@…, POINT(2 2)@…}, 2): NULL, "
    "errno %d\n", meos_errno());
  assert(merged == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* Two CONTINUOUS sequences meeting at a shared instant with different
   * values. This reaches a different merge path from the instant sets above:
   * the sequences are merged as an array and assembled into a sequence set */
  Temporal *seq1 = tfloat_in("[77@2000-01-01, 88@2000-01-03]");
  Temporal *seq2 = tfloat_in("[99@2000-01-03, 44@2000-01-05]");
  assert(seq1); assert(seq2);
  meos_errno_reset();
  merged = temporal_merge(seq1, seq2);
  printf("temporal_merge(%s, %s): NULL, errno %d\n",
    "[77@2000-01-01, 88@2000-01-03]", "[99@2000-01-03, 44@2000-01-05]",
    meos_errno());
  assert(merged == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* The same conflict between two sequence SETS */
  Temporal *ss1 = tfloat_in("{[77@2000-01-01, 88@2000-01-03]}");
  Temporal *ss2 = tfloat_in("{[99@2000-01-03, 44@2000-01-05]}");
  assert(ss1); assert(ss2);
  meos_errno_reset();
  merged = temporal_merge(ss1, ss2);
  printf("temporal_merge({[77@…, 88@…]}, {[99@…, 44@…]}): NULL, errno %d\n",
    meos_errno());
  assert(merged == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);

  /* Two continuous sequences meeting at a shared instant that carries the
   * SAME value merge into one sequence */
  Temporal *join1 = tfloat_in("[77@2000-01-01, 88@2000-01-03]");
  Temporal *join2 = tfloat_in("[88@2000-01-03, 44@2000-01-05]");
  assert(join1); assert(join2);
  meos_errno_reset();
  Temporal *joined = temporal_merge(join1, join2);
  assert(joined);
  char *joined_out = tfloat_out(joined, 6);
  printf("temporal_merge(%s, %s): %s\n", "[77@2000-01-01, 88@2000-01-03]",
    "[88@2000-01-03, 44@2000-01-05]", joined_out);
  assert(joined_out[0] == '[');

  free(seq1); free(seq2); free(ss1); free(ss2);
  free(join1); free(join2); free(joined); free(joined_out);

  /* Merging compatible values still succeeds after the reported conflicts */
  Temporal *ok1 = tfloat_in("{77@2000-01-01}");
  Temporal *ok2 = tfloat_in("{88@2000-01-02}");
  assert(ok1); assert(ok2);
  Temporal *okarr[2] = {ok1, ok2};
  merged = temporal_merge_array(okarr, 2);
  assert(merged);
  char *merged_out = tfloat_out(merged, 6);
  printf("temporal_merge_array({%s, %s}, 2): %s\n",
    "{77@2000-01-01}", "{88@2000-01-02}", merged_out);
  assert(strcmp(merged_out,
    "{77@2000-01-01 00:00:00+00, 88@2000-01-02 00:00:00+00}") == 0);

  /* Values sharing a timestamp with the SAME value merge into one instant */
  Temporal *same1 = tfloat_in("{77@2000-01-01}");
  Temporal *same2 = tfloat_in("{77@2000-01-01}");
  assert(same1); assert(same2);
  Temporal *samearr[2] = {same1, same2};
  Temporal *sameres = temporal_merge_array(samearr, 2);
  assert(sameres);
  char *sameres_out = tfloat_out(sameres, 6);
  printf("temporal_merge_array({%s, %s}, 2): %s\n",
    "{77@2000-01-01}", "{77@2000-01-01}", sameres_out);
  assert(strcmp(sameres_out, "77@2000-01-01 00:00:00+00") == 0);

  free(conflict1); free(conflict2);
  free(point1); free(point2);
  free(ok1); free(ok2); free(merged); free(merged_out);
  free(same1); free(same2); free(sameres); free(sameres_out);

  /* Finalize MEOS */
  meos_finalize();

  return 0;
}
