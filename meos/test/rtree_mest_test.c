/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief A program that tests the multi-entry (MEST) temporal functions of the
 * in-memory RTree index, i.e., rtree_insert_temporal_split and
 * rtree_search_temporal_dedup, against the single-box temporal functions
 * rtree_insert_temporal and rtree_search_temporal and against an exact
 * brute-force oracle.
 *
 * Four properties are asserted:
 *  (i)   no false negatives: every trip whose per-segment box decomposition
 *        overlaps the query's per-segment box decomposition (exact
 *        brute-force oracle over the identical decomposition the index
 *        encodes) is in the MEST candidate set;
 *  (ii)  deduplication: each surviving id appears exactly once;
 *  (iii) selectivity: on deliberately wiggly, high-extent tgeompoint trips the
 *        MEST candidate set is no larger than the single-box candidate set;
 *  (iv)  degeneracy: maxboxes <= 1 yields exactly the single-box result.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_mest_test rtree_mest_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of trips inserted into the index */
#define NUM_TRIPS 2000
/* Number of instants per (wiggly) trip */
#define TRIP_LEN 40
/* Maximum number of boxes produced per trip in the MEST index */
#define MAX_BOXES 16
/* Maximum length in characters of a trip in text format */
#define MAX_LEN_TRIP 4096

static int failures = 0;

static void
check(const char *name, bool ok)
{
  printf("  %-58s %s\n", name, ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
}

/* Return a pseudo-random double in [min, max] */
static double
random_double(double min, double max)
{
  return min + (max - min) * ((double) rand() / (double) RAND_MAX);
}

/*
 * Build a deliberately wiggly, high-extent tgeompoint trip: a sinusoidal
 * path that sweeps a large bounding rectangle even though every individual
 * segment is short. This is exactly the shape for which a single MBR is a
 * poor approximation and per-segment MEST boxes are far more selective.
 */
static Temporal *
make_wiggly_trip(int seed, char *buf, size_t bufsize)
{
  double ox = random_double(0, 900);
  double oy = random_double(0, 900);
  double amp = random_double(40, 90);
  double phase = random_double(0, 6.28);
  size_t pos = 0;
  pos += (size_t) snprintf(buf + pos, bufsize - pos, "[");
  for (int k = 0; k < TRIP_LEN; k++)
  {
    double x = ox + k * 2.0;
    double y = oy + amp * sin(phase + k * 0.9 + seed * 0.01);
    int mm = k / 60, ss = k % 60;
    pos += (size_t) snprintf(buf + pos, bufsize - pos,
      "%sPoint(%.4f %.4f)@2020-01-01 %02d:%02d:00+00",
      (k == 0) ? "" : ", ", x, y, mm, ss);
  }
  snprintf(buf + pos, bufsize - pos, "]");
  return tgeompoint_in(buf);
}

int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");
  srand(42);

  char buf[MAX_LEN_TRIP];
  Temporal **trips = malloc(sizeof(Temporal *) * NUM_TRIPS);

  /* Build the single-box index (existing API), the MEST index, and a
   * degenerate MEST index built with maxboxes = 1 */
  RTree *rtree_single = rtree_create_stbox();
  RTree *rtree_mest = rtree_create_stbox();
  RTree *rtree_deg = rtree_create_stbox();

  for (int i = 0; i < NUM_TRIPS; i++)
  {
    trips[i] = make_wiggly_trip(i, buf, sizeof(buf));
    rtree_insert_temporal(rtree_single, trips[i], i);
    rtree_insert_temporal_split(rtree_mest, trips[i], i, MAX_BOXES);
    rtree_insert_temporal_split(rtree_deg, trips[i], i, 1);
  }

  /* Query temporal value (also wiggly so its per-segment decomposition is
   * meaningful for the dedup search) */
  Temporal *query = make_wiggly_trip(123456, buf, sizeof(buf));

  MeosArray *single_ids = meos_array_create(sizeof(int));
  MeosArray *mest_ids = meos_array_create(sizeof(int));
  MeosArray *deg_ids = meos_array_create(sizeof(int));

  int single_count = rtree_search_temporal(rtree_single, RTREE_OVERLAPS,
    query, single_ids);
  int mest_count = rtree_search_temporal_dedup(rtree_mest, RTREE_OVERLAPS,
    query, MAX_BOXES, mest_ids);
  int deg_count = rtree_search_temporal_dedup(rtree_deg, RTREE_OVERLAPS,
    query, 1, deg_ids);

  /* Membership bitsets */
  bool *in_single = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_mest = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_deg = calloc(NUM_TRIPS, sizeof(bool));
  int *mest_seen = calloc(NUM_TRIPS, sizeof(int));
  int *deg_seen = calloc(NUM_TRIPS, sizeof(int));
  for (int i = 0; i < single_count; i++)
    in_single[*(int *) meos_array_get(single_ids, i)] = true;
  for (int i = 0; i < mest_count; i++)
  {
    int id = *(int *) meos_array_get(mest_ids, i);
    in_mest[id] = true;
    mest_seen[id]++;
  }
  for (int i = 0; i < deg_count; i++)
  {
    int id = *(int *) meos_array_get(deg_ids, i);
    in_deg[id] = true;
    deg_seen[id]++;
  }

  printf("MEST RTree (tgeompoint, %d wiggly trips, maxboxes=%d):\n",
    NUM_TRIPS, MAX_BOXES);
  printf("  single-box candidates: %d   MEST candidates: %d   "
    "degenerate: %d\n", single_count, mest_count, deg_count);

  /* (i) No false negatives. The exact oracle is a brute-force scan over the
   * identical per-segment box decomposition that the MEST index encodes: a
   * trip is a true candidate iff one of its split boxes overlaps one of the
   * query's split boxes. The MEST search must return a superset of this set.
   * (overlaps_tspatial_stbox is NOT a valid oracle here: it compares the
   * trip MBR against the query MBR, i.e. the coarser single-box semantics,
   * which by construction over-counts relative to the per-segment search.) */
  int qn;
  STBox *qboxes = tgeo_split_n_stboxes(query, MAX_BOXES, &qn);
  int true_overlaps = 0, missed = 0;
  for (int i = 0; i < NUM_TRIPS; i++)
  {
    int tn;
    STBox *tboxes = tgeo_split_n_stboxes(trips[i], MAX_BOXES, &tn);
    bool overlap = false;
    for (int a = 0; a < tn && ! overlap; a++)
      for (int b = 0; b < qn && ! overlap; b++)
        if (overlaps_stbox_stbox(&tboxes[a], &qboxes[b]))
          overlap = true;
    free(tboxes);
    if (overlap)
    {
      true_overlaps++;
      if (! in_mest[i])
        missed++;
    }
  }
  free(qboxes);
  printf("  true overlaps (per-segment exact oracle): %d\n", true_overlaps);
  check("(i) no false negatives vs exact per-segment oracle", missed == 0);

  /* (ii) Dedup: each surviving id appears exactly once */
  bool dedup_ok = true;
  for (int i = 0; i < NUM_TRIPS; i++)
    if (mest_seen[i] > 1 || deg_seen[i] > 1)
      dedup_ok = false;
  check("(ii) every surviving id appears exactly once", dedup_ok);

  /* (iii) Selectivity gain: on wiggly high-extent trips the MEST candidate
   * set is no larger than the single-box candidate set, and the single-box
   * set is a superset of the MEST set (MEST never adds false candidates a
   * single MBR would have excluded, since each MEST box is contained in the
   * trip MBR) */
  bool subset_ok = true;
  for (int i = 0; i < NUM_TRIPS; i++)
    if (in_mest[i] && ! in_single[i])
      subset_ok = false;
  check("(iii) MEST candidate set <= single-box candidate set",
    mest_count <= single_count && subset_ok);

  /* (iv) Degeneracy: maxboxes <= 1 reproduces the single-box result exactly */
  bool deg_ok = (deg_count == single_count);
  for (int i = 0; i < NUM_TRIPS && deg_ok; i++)
    if (in_deg[i] != in_single[i])
      deg_ok = false;
  check("(iv) maxboxes<=1 identical to single-box result", deg_ok);

  /* Cleanup */
  free(in_single); free(in_mest); free(in_deg);
  free(mest_seen); free(deg_seen);
  meos_array_destroy(single_ids);
  meos_array_destroy(mest_ids);
  meos_array_destroy(deg_ids);
  free(query);
  for (int i = 0; i < NUM_TRIPS; i++)
    free(trips[i]);
  free(trips);
  rtree_free(rtree_single);
  rtree_free(rtree_mest);
  rtree_free(rtree_deg);

  /* Finalize MEOS */
  meos_finalize();

  if (failures == 0)
    printf("\nAll MEST RTree tests passed.\n");
  else
    printf("\n%d MEST RTree test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
