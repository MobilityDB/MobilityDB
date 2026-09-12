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
 * error left behind.
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

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
