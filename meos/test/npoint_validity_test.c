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
 * @brief A program that tests how the temporal network point instant
 * constructor and the route accessor report an erroneous argument under the
 * noexit error handler.
 *
 * A public MEOS function tests the conditions its internal form asserts, so a
 * binding calling it with a null or mistyped argument receives an error it
 * can raise in its host language. An assertion cannot carry that contract: it
 * is compiled out under NDEBUG, which leaves the release build a binding
 * links against with no check at all.
 *
 * The program verifies that #tnpointinst_make reports a null network point by
 * returning NULL and setting #meos_errno, that #tnpoint_route reports a null
 * value, a value of another temporal type and a discrete sequence by
 * answering INT64_MAX, the error value it documents, and setting
 * #meos_errno, and that a valid instant and route answer with no error left
 * behind.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o npoint_validity_test npoint_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_npoint.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  Npoint *np = npoint_in("Npoint(1, 0.5)");
  TimestampTz t = timestamptz_in("2001-01-01", -1);
  Temporal *tint = tint_in("[1@2001-01-01, 2@2001-01-02]");
  Temporal *disc = tnpoint_in(
    "{Npoint(1, 0.5)@2001-01-01, Npoint(1, 0.7)@2001-01-02}");
  Temporal *seq = tnpoint_in(
    "[Npoint(1, 0.5)@2001-01-01, Npoint(1, 0.7)@2001-01-02]");
  assert(np && tint && disc && seq);
  meos_errno_reset();

  /* A null network point is reported rather than read */
  TInstant *inst = tnpointinst_make(NULL, t);
  printf("tnpointinst_make(NULL, t): %s, errno %d\n",
    inst ? "a value" : "NULL", meos_errno());
  assert(inst == NULL);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A null value is reported rather than read */
  int64 rid = tnpoint_route(NULL);
  printf("tnpoint_route(NULL): %s, errno %d\n",
    rid == INT64_MAX ? "INT64_MAX" : "a route", meos_errno());
  assert(rid == INT64_MAX);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG);
  meos_errno_reset();

  /* A temporal integer is reported rather than read as a network point */
  rid = tnpoint_route(tint);
  printf("tnpoint_route(tint): %s, errno %d\n",
    rid == INT64_MAX ? "INT64_MAX" : "a route", meos_errno());
  assert(rid == INT64_MAX);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_TYPE);
  meos_errno_reset();

  /* A discrete sequence has no single route, and the refusal answers the
   * error value the function documents */
  rid = tnpoint_route(disc);
  printf("tnpoint_route(discrete): %s, errno %d\n",
    rid == INT64_MAX ? "INT64_MAX" : "a route", meos_errno());
  assert(rid == INT64_MAX);
  assert(meos_errno() == MEOS_ERR_INVALID_ARG_VALUE);
  meos_errno_reset();

  /* A valid instant and a valid route answer with no error left behind */
  inst = tnpointinst_make(np, t);
  printf("tnpointinst_make(np, t): %s, errno %d\n",
    inst ? "a value" : "NULL", meos_errno());
  assert(inst != NULL);
  assert(meos_errno() == 0);
  free(inst);

  rid = tnpoint_route(seq);
  printf("tnpoint_route(seq): %lld, errno %d\n", (long long) rid,
    meos_errno());
  assert(rid == 1);
  assert(meos_errno() == 0);

  free(np); free(tint); free(disc); free(seq);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
