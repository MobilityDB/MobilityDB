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
 * @brief A program that tests the reachability filter as a stream drives it
 * @details The program walks thirteen reports one at a time: the first is
 * 5 km off the track, the twelfth is a teleport after the head of eleven
 * reports. After each report it takes the reports decided since the report
 * before with #reachabilitystate_instants and checks how many there are, it
 * checks the counts of the state, and it checks that a state written by
 * #reachabilitystate_serialize and read back by
 * #reachabilitystate_deserialize walks on as the one written. It then checks
 * the errors of the transition function and of the reading of the bytes
 * under the noexit error handler.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o reachability_filter_test reachability_filter_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

#define NREPORTS 13

static int failures = 0;

static void
check_int(const char *what, int got, int expected)
{
  if (got != expected)
  {
    printf("FAILED: %s is %d, expected %d\n", what, got, expected);
    failures++;
  }
}

static void
check_error(const char *call, bool failed, int expected)
{
  if (! failed || meos_errno() != expected)
  {
    printf("FAILED: %s reports errno %d, expected %d\n", call, meos_errno(),
      expected);
    failures++;
  }
  meos_errno_reset();
}

/* Walk the reports, the state written and read back after the report given,
 * or never when it is negative, and return the number of reports handed back
 * after each report in decided */
static void
walk(Temporal **reports, int serialize_after, int *decided, int *counts)
{
  ReachabilityState *state = NULL;
  for (int i = 0; i < NREPORTS; i++)
  {
    state = temporal_reachability_filter_transfn(state,
      (TInstant *) reports[i], 100);
    if (i == serialize_after)
    {
      size_t size;
      uint8_t *bytes = reachabilitystate_serialize(state, &size);
      ReachabilityState *copy = reachabilitystate_deserialize(bytes, size);
      free(bytes);
      free(temporal_reachability_filter_finalfn(state));
      state = copy;
    }
    int count;
    TInstant **insts = reachabilitystate_instants(state, &count);
    for (int j = 0; j < count; j++)
      free(insts[j]);
    free(insts);
    decided[i] = count;
  }
  counts[0] = reachabilitystate_reports(state);
  counts[1] = reachabilitystate_unreachable(state);
  counts[2] = reachabilitystate_kept(state);
  /* Every kept report has been handed back, so the final function has none */
  Temporal *rest = temporal_reachability_filter_finalfn(state);
  check_int("reports left to the final function", rest ? 1 : 0, 0);
  free(rest);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* The first report is off the track, the twelfth a teleport */
  const char *points[NREPORTS] = {"5000 0", "10 0", "20 0", "30 0", "40 0",
    "50 0", "60 0", "70 0", "80 0", "90 0", "100 0", "5000 0", "120 0"};
  Temporal *reports[NREPORTS];
  for (int i = 0; i < NREPORTS; i++)
  {
    char str[64];
    snprintf(str, sizeof(str), "Point(%s)@2001-01-01 00:00:%02d", points[i], i);
    reports[i] = tgeompoint_in(str);
  }

  /* The head is decided when its eleventh report arrives: the ten on the
   * track; the teleport is not kept, the last report is */
  int expected[NREPORTS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 1};
  for (int serialize_after = -1; serialize_after < NREPORTS;
       serialize_after += 4)
  {
    int decided[NREPORTS], counts[3];
    walk(reports, serialize_after, decided, counts);
    for (int i = 0; i < NREPORTS; i++)
    {
      char what[96];
      snprintf(what, sizeof(what),
        "reports decided by report %d, state read back after %d", i,
        serialize_after);
      check_int(what, decided[i], expected[i]);
    }
    check_int("reports", counts[0], 13);
    /* The second report from the first, the teleport, the report after it */
    check_int("unreachable reports", counts[1], 3);
    check_int("kept reports", counts[2], 11);
  }

  /* Errors */
  ReachabilityState *state = temporal_reachability_filter_transfn(NULL,
    (TInstant *) reports[5], 100);
  check_error("a report before the last one",
    temporal_reachability_filter_transfn(state, (TInstant *) reports[4],
      100) == NULL, MEOS_ERR_INVALID_ARG_VALUE);
  check_error("another maximum speed",
    temporal_reachability_filter_transfn(state, (TInstant *) reports[6],
      200) == NULL, MEOS_ERR_INVALID_ARG_VALUE);
  Temporal *other = tgeompoint_in("SRID=3812;Point(0 0)@2001-01-01 00:00:20");
  check_error("a report of another SRID",
    temporal_reachability_filter_transfn(state, (TInstant *) other, 100) ==
      NULL, MEOS_ERR_INVALID_ARG_VALUE);
  free(other);
  Temporal *integer = tint_in("1@2001-01-01");
  check_error("a temporal integer",
    temporal_reachability_filter_transfn(NULL, (TInstant *) integer, 100) ==
      NULL, MEOS_ERR_INVALID_ARG_TYPE);
  free(integer);
  check_error("a speed of zero",
    temporal_reachability_filter_transfn(NULL, (TInstant *) reports[0], 0) ==
      NULL, MEOS_ERR_INVALID_ARG_VALUE);
  uint8_t bytes[8] = {0};
  check_error("bytes of no state",
    reachabilitystate_deserialize(bytes, sizeof(bytes)) == NULL,
    MEOS_ERR_INVALID_ARG_VALUE);
  free(temporal_reachability_filter_finalfn(state));

  for (int i = 0; i < NREPORTS; i++)
    free(reports[i]);
  printf("%s\n", failures == 0 ? "All tests passed" : "Some tests failed");
  meos_finalize();
  return failures == 0 ? 0 : 1;
}
