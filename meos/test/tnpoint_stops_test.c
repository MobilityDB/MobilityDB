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
 * @brief The stops of a temporal network point whose route bends between two
 * instants, judged by the closed form of the route
 *
 * @details A stop is a stay whose points lie within the maximum distance of
 * each other, and a network point passes every point of its route between
 * two instants. The route of the network written here runs from (0 0) to
 * (100 0) and back to (0 1), so its two ends stand 1 apart while its path
 * reaches (100 0), 100 from the first end:
 * - between the two ends no stay lies within 50, however near the ends;
 * - within 150, the whole path, of diameter below 101, is one stop;
 * - between the position 0 and the position 0.1, on the first leg about 20
 *   from the start, the two instants are one stop within 50.
 *
 * The route is written to the ways file `tnpoint_stops_ways.csv` in the
 * current directory, removed at the end.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_npoint.h>

static const char *WAYS = "tnpoint_stops_ways.csv";

/* The number of instants of each stop of a sequence, in the order of the
 * stops, written into counts, and the number of stops, as #stops_counts of
 * tpoint_stops_test.c reads them for a temporal point */
static int
stops_counts(const char *str, double maxdist, int *counts)
{
  Temporal *temp = tnpoint_in(str);
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

  /* The network: route 0 from (0 0) to (100 0) and back to (0 1) */
  GSERIALIZED *route = geom_in("SRID=5676;LINESTRING(0 0,100 0,0 1)", -1);
  assert(route != NULL);
  char *hex = geo_as_hexewkb(route, "NDR");
  FILE *file = fopen(WAYS, "w");
  assert(file != NULL);
  fprintf(file, "0,%s\n", hex);
  fclose(file);
  free(hex); free(route);
  meos_set_ways_csv(WAYS);
  int counts[4];

  /* The ends 1 apart, the path reaching (100 0) between them */
  int n = stops_counts("[NPoint(0,0)@2001-01-01, NPoint(0,1)@2001-01-02]",
    50.0, counts);
  assert(n == 0);
  /* Within 150 the whole path is one stop */
  n = stops_counts("[NPoint(0,0)@2001-01-01, NPoint(0,1)@2001-01-02]",
    150.0, counts);
  assert(n == 1 && counts[0] == 2);
  /* Two instants on the first leg about 20 apart */
  n = stops_counts("[NPoint(0,0)@2001-01-01, NPoint(0,0.1)@2001-01-02]",
    50.0, counts);
  assert(n == 1 && counts[0] == 2);

  printf("tnpoint_stops_test: every stop answered as the path along its route "
    "decides it\n");
  meos_finalize();
  remove(WAYS);
  return 0;
}
