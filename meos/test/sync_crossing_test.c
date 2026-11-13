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
 * @brief A simple program that tests the synchronization of two temporal
 * sequences with their crossings added
 * @details `synchronize_tsequence_tsequence` adds, between two synchronized
 * instants, the instant at which the two sequences cross, with the one value
 * they take there. The program synchronizes two temporal geometry points,
 * whose values are passed by reference, so that the two instants of a
 * crossing are each built from that value; the memory checker reads it.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

extern bool synchronize_tsequence_tsequence(const TSequence *seq1,
  const TSequence *seq2, TSequence **sync1, TSequence **sync2,
  bool interpoint);

static const struct
{
  const char *seq1;
  const char *seq2;
  const char *expected1;
  const char *expected2;
} SYNCS[] = {
  /* Two segments crossing at the middle of their period */
  {"[Point(0 0)@2001-01-01, Point(2 2)@2001-01-03]",
   "[Point(2 0)@2001-01-01, Point(0 2)@2001-01-03]",
   "[POINT(0 0)@2001-01-01 00:00:00+00, POINT(1 1)@2001-01-02 00:00:00+00, "
     "POINT(2 2)@2001-01-03 00:00:00+00]",
   "[POINT(2 0)@2001-01-01 00:00:00+00, POINT(1 1)@2001-01-02 00:00:00+00, "
     "POINT(0 2)@2001-01-03 00:00:00+00]"},
  /* Two sequences meeting at an instant of the first */
  {"[Point(0 2)@2001-01-01, Point(2 0)@2001-01-02, Point(4 2)@2001-01-03]",
   "[Point(0 0)@2001-01-01, Point(4 0)@2001-01-03]",
   "[POINT(0 2)@2001-01-01 00:00:00+00, POINT(2 0)@2001-01-02 00:00:00+00, "
     "POINT(4 2)@2001-01-03 00:00:00+00]",
   "[POINT(0 0)@2001-01-01 00:00:00+00, POINT(2 0)@2001-01-02 00:00:00+00, "
     "POINT(4 0)@2001-01-03 00:00:00+00]"},
  /* Two segments crossing at a perpendicular */
  {"[Point(1 0)@2001-01-01, Point(1 4)@2001-01-05]",
   "[Point(0 2)@2001-01-01, Point(2 2)@2001-01-05]",
   "[POINT(1 0)@2001-01-01 00:00:00+00, POINT(1 2)@2001-01-03 00:00:00+00, "
     "POINT(1 4)@2001-01-05 00:00:00+00]",
   "[POINT(0 2)@2001-01-01 00:00:00+00, POINT(1 2)@2001-01-03 00:00:00+00, "
     "POINT(2 2)@2001-01-05 00:00:00+00]"},
};

static int
check(const char *what, const TSequence *seq, const char *expected)
{
  size_t size;
  char *out = seq ? tspatial_as_text((const Temporal *) seq, 6, &size) : NULL;
  int failed = ! out || strcmp(out, expected) != 0;
  if (failed)
    printf("FAILED: %s: %s, expected %s\n", what, out ? out : "NULL",
      expected);
  else
    printf("OK: %s: %s\n", what, out);
  free(out);
  return failed;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int failures = 0, count = 0;
  for (size_t i = 0; i < sizeof(SYNCS) / sizeof(SYNCS[0]); i++)
  {
    Temporal *t1 = tgeompoint_in(SYNCS[i].seq1);
    Temporal *t2 = tgeompoint_in(SYNCS[i].seq2);
    TSequence *sync1 = NULL, *sync2 = NULL;
    bool found = synchronize_tsequence_tsequence((const TSequence *) t1,
      (const TSequence *) t2, &sync1, &sync2, true);
    char what[64];
    snprintf(what, sizeof(what), "synchronize %zu, first", i + 1);
    failures += found ? check(what, sync1, SYNCS[i].expected1) : 1;
    snprintf(what, sizeof(what), "synchronize %zu, second", i + 1);
    failures += found ? check(what, sync2, SYNCS[i].expected2) : 1;
    count += 2;
    free(sync1); free(sync2); free(t1); free(t2);
  }

  meos_finalize();
  printf("sync_crossing_test: %d cases, %d failed\n", count, failures);
  return failures ? 1 : 0;
}
