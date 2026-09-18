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
 * @brief A program that tests how the geo constructors report an erroneous
 * argument under the noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null pointer or a count out of range receives an
 * error it can raise in its host language. An assertion cannot carry that
 * contract: it is compiled out under NDEBUG, which leaves the release build a
 * binding links against with no check at all.
 *
 * The program verifies that #tpointseq_make_coords reports a null array of
 * coordinates or timestamps and a count that is not positive by returning
 * NULL and setting #meos_errno, and that a valid call still answers with no
 * error left behind. It also verifies that an empty geometry is reported by
 * the constructors of a temporal instant and of a sequence from a timestamptz
 * span, and a geometry that is not a point by the temporal point one.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o geo_validity_test geo_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

  double xs[2] = {1.0, 2.0};
  double ys[2] = {1.0, 2.0};
  TimestampTz ts[2];
  ts[0] = timestamptz_in("2001-01-01", -1);
  ts[1] = timestamptz_in("2001-01-02", -1);
  meos_errno_reset();

  /* A null array of x coordinates is reported rather than read */
  TSequence *seq = tpointseq_make_coords(NULL, ys, NULL, ts, 2, 0, false,
    true, true, LINEAR, true);
  printf("tpointseq_make_coords(NULL, ys, NULL, ts, 2, ...): %s, errno %d\n",
    seq ? "a value" : "NULL", meos_errno());
  assert(seq == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A null array of y coordinates is reported rather than read */
  seq = tpointseq_make_coords(xs, NULL, NULL, ts, 2, 0, false, true, true,
    LINEAR, true);
  printf("tpointseq_make_coords(xs, NULL, NULL, ts, 2, ...): %s, errno %d\n",
    seq ? "a value" : "NULL", meos_errno());
  assert(seq == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A null array of timestamps is reported rather than read */
  seq = tpointseq_make_coords(xs, ys, NULL, NULL, 2, 0, false, true, true,
    LINEAR, true);
  printf("tpointseq_make_coords(xs, ys, NULL, NULL, 2, ...): %s, errno %d\n",
    seq ? "a value" : "NULL", meos_errno());
  assert(seq == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A count that is not positive is reported rather than answered by a
   * NULL that carries no error */
  seq = tpointseq_make_coords(xs, ys, NULL, ts, 0, 0, false, true, true,
    LINEAR, true);
  printf("tpointseq_make_coords(xs, ys, NULL, ts, 0, ...): %s, errno %d\n",
    seq ? "a value" : "NULL", meos_errno());
  assert(seq == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();

  /* A valid call still answers a sequence of two instants, and the guards
   * leave no error behind */
  seq = tpointseq_make_coords(xs, ys, NULL, ts, 2, 0, false, true, true,
    LINEAR, true);
  printf("tpointseq_make_coords(xs, ys, NULL, ts, 2, ...): %s, %d instants, "
    "errno %d\n", seq ? "a value" : "NULL",
    seq ? temporal_num_instants((Temporal *) seq) : 0, meos_errno());
  assert(seq != NULL);
  assert(temporal_num_instants((Temporal *) seq) == 2);
  assert(meos_errno() == 0);
  free(seq);

  /* An empty geometry is reported by every constructor of a temporal value,
   * as for a point read in place as for a geometry walked, and a point that
   * is not empty still answers */
  Span *period = tstzspan_in("[2001-01-01, 2001-01-02]");
  static const char *empties[] = {"POINT EMPTY", "POINT Z EMPTY",
    "LINESTRING EMPTY"};
  for (int i = 0; i < 3; i++)
  {
    GSERIALIZED *empty = geom_in(empties[i], -1);
    meos_errno_reset();
    TInstant *inst = tgeoinst_make(empty, ts[0]);
    printf("tgeoinst_make(%s, t): %s, errno %d\n", empties[i],
      inst ? "a value" : "NULL", meos_errno());
    assert(inst == NULL);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();
    seq = tgeoseq_from_base_tstzspan(empty, period, LINEAR);
    printf("tgeoseq_from_base_tstzspan(%s, ...): %s, errno %d\n", empties[i],
      seq ? "a value" : "NULL", meos_errno());
    assert(seq == NULL);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();
    seq = tpointseq_from_base_tstzspan(empty, period, LINEAR);
    printf("tpointseq_from_base_tstzspan(%s, ...): %s, errno %d\n",
      empties[i], seq ? "a value" : "NULL", meos_errno());
    assert(seq == NULL);
    assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
    meos_errno_reset();
    free(empty);
  }
  /* The point variant also refuses a geometry that is not a point */
  GSERIALIZED *line = geom_in("LINESTRING(1 1,2 2)", -1);
  seq = tpointseq_from_base_tstzspan(line, period, LINEAR);
  printf("tpointseq_from_base_tstzspan(LINESTRING, ...): %s, errno %d\n",
    seq ? "a value" : "NULL", meos_errno());
  assert(seq == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();
  free(line);
  GSERIALIZED *point = geom_in("POINT(1 1)", -1);
  TInstant *inst = tpointinst_make(point, ts[0]);
  seq = tpointseq_from_base_tstzspan(point, period, LINEAR);
  printf("tpointinst_make(POINT(1 1), t): %s, "
    "tpointseq_from_base_tstzspan(POINT(1 1), ...): %s, errno %d\n",
    inst ? "a value" : "NULL", seq ? "a value" : "NULL", meos_errno());
  assert(inst != NULL && seq != NULL);
  assert(meos_errno() == 0);
  free(inst); free(seq); free(point); free(period);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
