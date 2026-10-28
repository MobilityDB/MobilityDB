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
 * @brief The stops of a temporal point whose points stand just beyond the
 * maximum distance, judged by the whole numbers
 *
 * @details A stop is a stay whose points lie within the maximum distance of
 * each other. The points (0 0) and (3 + 2^-51, 4) stand
 * `sqrt(25 + 6 * 2^-51 + 2^-102)` apart, which exceeds 5 since its square
 * does, by less than half a unit in the last place of 5, so that the distance
 * rounds to 5. No stay holds two points so placed.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* The number of instants of each stop of a sequence, in the order of the
 * stops, written into counts, and the number of stops */
static int
stops_counts(const char *wkt, double maxdist, int *counts)
{
  Temporal *temp = tgeompoint_in(wkt);
  Interval *zero = interval_in("0 seconds", -1);
  assert(temp != NULL && zero != NULL);
  TSequenceSet *stops = temporal_stops(temp, maxdist, zero);
  int n = 0;
  if (stops)
  {
    n = temporal_num_sequences((Temporal *) stops);
    for (int i = 0; i < n; i++)
    {
      TSequence *seq = temporal_sequence_n((Temporal *) stops, i + 1);
      counts[i] = temporal_num_instants((Temporal *) seq);
      free(seq);
    }
    free(stops);
  }
  free(temp); free(zero);
  return n;
}

int
main(void)
{
  meos_initialize();

  /* 3.0000000000000004 reads as 3 + 2^-51, and the excess of the distance
   * over 5, below 0.3 * 2^-50, is below half the gap 2^-50 from 5 to the next
   * double */
  assert(3.0000000000000004 == 3.0 + 0x1p-51);
  assert(1.5000000000000004 == 1.5 + 0x1p-51);
  int counts[4];

  /* Two points just beyond 5 apart: the anchor of the window reads it */
  int n = stops_counts("[Point(0 0)@2001-01-01, "
    "Point(3.0000000000000004 4)@2001-01-02]", 5.0, counts);
  assert(n == 0);
  /* At 5 exactly, the two points make one stop */
  n = stops_counts("[Point(0 0)@2001-01-01, Point(3 4)@2001-01-02]", 5.0,
    counts);
  assert(n == 1 && counts[0] == 2);

  /* Three points, the last two 2.5 from the first and just beyond 5 apart:
   * the window reads every pair, and the stop ends before the third point */
  n = stops_counts("[Point(0 0)@2001-01-01, Point(-1.5 -2)@2001-01-02, "
    "Point(1.5000000000000004 2)@2001-01-03]", 5.0, counts);
  assert(n == 1 && counts[0] == 2);
  /* With the third point at (1.5 2), the three make one stop */
  n = stops_counts("[Point(0 0)@2001-01-01, Point(-1.5 -2)@2001-01-02, "
    "Point(1.5 2)@2001-01-03]", 5.0, counts);
  assert(n == 1 && counts[0] == 3);

  printf("tpoint_stops_test: every stop answered as the whole numbers "
    "decide it\n");
  meos_finalize();
  return 0;
}
