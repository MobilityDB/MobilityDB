/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief A program that demonstrates the multi-entry (MEST) RTree selectivity
 * gain for wiggly, high-extent temporal points.
 *
 * The program generates a deterministic set of zig-zag random-walk
 * tgeompoint sequences whose per-trip minimum bounding box is loose (the
 * documented BerlinMOD / AIS-style scenario where a single MBR covers a large
 * empty rectangle while every segment stays small). It builds one single-box
 * RTree with rtree_insert_temporal and several multi-entry RTrees with
 * rtree_insert_temporal_split over the SAME trips and ids for a small sweep of
 * maxboxes. Each query is itself a small wiggly probe trip, the natural
 * BerlinMOD-style range probe. The single-box index is searched with
 * rtree_search_temporal (the probe's single MBR) and every MEST index with
 * rtree_search_temporal_dedup (the probe's per-segment boxes, each id returned
 * at most once). The candidate set is compared against the exact ground
 * truth, computed independently of the index as the set of trips that share
 * at least one overlapping tight per-segment bounding box with the probe
 * (tgeo_stboxes on both sides, overlaps_stbox_stbox over all segment-box
 * pairs). The program prints a table of candidate counts and false positives
 * per configuration and asserts the four MEST invariants: no false negatives,
 * dedup uniqueness, candidate count no larger than the single-box index and
 * tightening as maxboxes grows, and maxboxes <= 1 reproducing the single-box
 * candidate set exactly. The summary doubles as the benchmark for choosing a
 * default maxboxes.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_mest_example rtree_mest_example.c -L/usr/local/lib -lmeos -lproj -lm
 * @endcode
 */

/* C */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/* Number of trips inserted into every index */
#define NUM_TRIPS 4000
/* Number of instants per stored trip (zig-zag random walk) */
#define TRIP_LEN 40
/* Number of instants per query probe trip */
#define PROBE_LEN 12
/* Half-width of the arena in metres; trips start anywhere inside it */
#define ARENA 20000.0
/* Per-instant step magnitude of the stored trips in metres */
#define TRIP_STEP 4000.0
/* Per-instant step magnitude of the probe trips in metres */
#define PROBE_STEP 3000.0
/* SRID of the synthetic trips (projected, metric units) */
#define TRIP_SRID 25832
/* maxboxes values exercised by the sweep */
static const int MAXBOXES[] = {4, 8, 16};
#define NUM_MAXBOXES ((int) (sizeof(MAXBOXES) / sizeof(MAXBOXES[0])))
/* Number of query probe trips */
#define NUM_QUERIES 20

/* Return a random double in [min, max] */
static double
random_double(double min, double max)
{
  return min + (max - min) * ((double) rand() / (double) RAND_MAX);
}

/* Return a random integer in [min, max] */
static int
random_int(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

/* Reference epoch 2020-01-01 00:00:00+00 as a TimestampTz */
static TimestampTz
epoch_2020(void)
{
  TInstant *e = (TInstant *) tgeompoint_in(
    "SRID=25832;POINT(0 0)@2020-01-01 00:00:00+00");
  TimestampTz t0 = e->t;
  free(e);
  return t0;
}

/*****************************************************************************/

/**
 * @brief Build one deterministic zig-zag random-walk tgeompoint sequence
 * @details The trip starts at a random point in a wide arena and takes large
 * alternating-sign steps. The resulting trajectory has a loose minimum
 * bounding box (it sweeps a large rectangle) while every individual segment
 * stays small, which is exactly the case the multi-entry index targets.
 * @param[in] t0 Reference epoch
 * @param[in] startsecond First instant offset in seconds from the epoch
 * @param[in] len Number of instants
 * @param[in] step Per-instant step magnitude range upper bound
 */
static Temporal *
make_wiggly_trip(TimestampTz t0, int startsecond, int len, double step)
{
  double *xc = malloc(sizeof(double) * len);
  double *yc = malloc(sizeof(double) * len);
  TimestampTz *tc = malloc(sizeof(TimestampTz) * len);

  double x = random_double(0.0, ARENA);
  double y = random_double(0.0, ARENA);
  double sx = (random_int(0, 1) == 0) ? 1.0 : -1.0;
  double sy = (random_int(0, 1) == 0) ? 1.0 : -1.0;
  for (int i = 0; i < len; i++)
  {
    xc[i] = x;
    yc[i] = y;
    /* One instant per minute, trip starting at startsecond */
    tc[i] = t0 + (TimestampTz) (startsecond + i * 60) * 1000000;
    /* Large alternating-sign steps make the per-trip MBR loose */
    x += sx * random_double(step * 0.6, step);
    y += sy * random_double(step * 0.6, step);
    sx = -sx;
    sy = -sy;
  }
  TSequence *seq = tpointseq_make_coords(xc, yc, NULL, tc, len, TRIP_SRID,
    false, true, true, LINEAR, true);
  free(xc);
  free(yc);
  free(tc);
  return (Temporal *) seq;
}

/**
 * @brief Exact ground truth: do the two finest per-segment box sets share at
 * least one overlapping pair
 * @details Both temporal values are decomposed into their finest per-segment
 * STBoxes with tgeo_stboxes (the limit the MEST splitter coarsens from) and
 * every segment-box pair is tested with overlaps_stbox_stbox. This is the
 * exact answer the box filter approximates and is computed independently of
 * any index, so it admits neither false positives nor false negatives.
 */
static bool
segments_overlap(const STBox *pboxes, int pcount, const STBox *tboxes,
  int tcount)
{
  for (int i = 0; i < pcount; i++)
    for (int j = 0; j < tcount; j++)
      if (overlaps_stbox_stbox(&pboxes[i], &tboxes[j]))
        return true;
  return false;
}

/*****************************************************************************/

int main(void)
{
  meos_initialize();
  /* Use fixed seed for reproducibility */
  srand(1);

  printf("MEST RTree selectivity demonstration\n");
  printf("  %d wiggly tgeompoint trips, %d instants each, %d probe queries\n\n",
    NUM_TRIPS, TRIP_LEN, NUM_QUERIES);

  TimestampTz t0 = epoch_2020();

  /* Generate the trips once and keep them for ground-truth verification.
   * Trips overlap heavily in time so the spatial extent drives selectivity. */
  Temporal **trips = malloc(sizeof(Temporal *) * NUM_TRIPS);
  for (int i = 0; i < NUM_TRIPS; i++)
    trips[i] = make_wiggly_trip(t0, random_int(0, 600), TRIP_LEN, TRIP_STEP);

  /* Decompose every trip into its finest per-segment boxes once; these are
   * invariant across probes and drive the exact ground truth */
  STBox **trip_boxes = malloc(sizeof(STBox *) * NUM_TRIPS);
  int *trip_nboxes = malloc(sizeof(int) * NUM_TRIPS);
  for (int i = 0; i < NUM_TRIPS; i++)
    trip_boxes[i] = tgeo_stboxes(trips[i], &trip_nboxes[i]);

  /* Generate the probe trips once */
  Temporal **probes = malloc(sizeof(Temporal *) * NUM_QUERIES);
  for (int q = 0; q < NUM_QUERIES; q++)
    probes[q] = make_wiggly_trip(t0, random_int(0, 1200), PROBE_LEN,
      PROBE_STEP);

  /* Build the single-box index */
  RTree *single = rtree_create_stbox();
  for (int i = 0; i < NUM_TRIPS; i++)
    rtree_insert_temporal(single, trips[i], i);

  /* Build the maxboxes <= 1 degenerate index; it must reproduce the
   * single-box candidate set exactly (invariant iv) */
  RTree *degenerate = rtree_create_stbox();
  for (int i = 0; i < NUM_TRIPS; i++)
    rtree_insert_temporal_split(degenerate, trips[i], i, 1);

  /* Build one MEST index per maxboxes value, same trips and ids */
  RTree *mest[NUM_MAXBOXES];
  for (int m = 0; m < NUM_MAXBOXES; m++)
  {
    mest[m] = rtree_create_stbox();
    for (int i = 0; i < NUM_TRIPS; i++)
      rtree_insert_temporal_split(mest[m], trips[i], i, MAXBOXES[m]);
  }

  /* Result array reused across searches */
  MeosArray *ids = meos_array_create(sizeof(int));

  /* Aggregated totals over all queries */
  long total_truth = 0;
  long total_single = 0, total_single_fp = 0;
  long total_mest_cand[NUM_MAXBOXES];
  long total_mest_fp[NUM_MAXBOXES];
  for (int m = 0; m < NUM_MAXBOXES; m++)
  {
    total_mest_cand[m] = 0;
    total_mest_fp[m] = 0;
  }

  /* Per-query bitsets */
  bool *truth = calloc(NUM_TRIPS, sizeof(bool));
  bool *seen = calloc(NUM_TRIPS, sizeof(bool));

  for (int q = 0; q < NUM_QUERIES; q++)
  {
    Temporal *probe = probes[q];
    /* Exact ground truth, computed independently of the index */
    int pcount;
    STBox *pboxes = tgeo_stboxes(probe, &pcount);
    int truth_count = 0;
    for (int i = 0; i < NUM_TRIPS; i++)
    {
      truth[i] = segments_overlap(pboxes, pcount, trip_boxes[i],
        trip_nboxes[i]);
      if (truth[i])
        truth_count++;
    }
    free(pboxes);
    total_truth += truth_count;

    /* Single-box filter: the probe's single MBR */
    int single_count = rtree_search_temporal(single, RTREE_OVERLAPS, probe,
      ids);
    for (int i = 0; i < NUM_TRIPS; i++)
      seen[i] = false;
    int single_fp = 0;
    for (int k = 0; k < single_count; k++)
    {
      int id = *(int *) meos_array_get(ids, k);
      assert(! seen[id]);
      seen[id] = true;
      if (! truth[id])
        single_fp++;
    }
    /* Invariant (i): no false negatives for the single-box filter */
    for (int i = 0; i < NUM_TRIPS; i++)
      assert(! truth[i] || seen[i]);
    total_single += single_count;
    total_single_fp += single_fp;

    /* Invariant (iv): the maxboxes <= 1 index reproduces the single-box
     * candidate set exactly */
    int deg_count = rtree_search_temporal_dedup(degenerate, RTREE_OVERLAPS,
      probe, 1, ids);
    assert(deg_count == single_count);
    for (int k = 0; k < deg_count; k++)
    {
      int id = *(int *) meos_array_get(ids, k);
      assert(seen[id]);
    }

    /* MEST filters, one per maxboxes; rtree_search_temporal_dedup returns
     * each id at most once (invariant ii) */
    int prev_count = single_count;
    for (int m = 0; m < NUM_MAXBOXES; m++)
    {
      int cand = rtree_search_temporal_dedup(mest[m], RTREE_OVERLAPS, probe,
        MAXBOXES[m], ids);
      for (int i = 0; i < NUM_TRIPS; i++)
        seen[i] = false;
      int mest_fp = 0;
      for (int k = 0; k < cand; k++)
      {
        int id = *(int *) meos_array_get(ids, k);
        /* Invariant (ii): dedup, each id appears at most once */
        assert(! seen[id]);
        seen[id] = true;
        if (! truth[id])
          mest_fp++;
      }
      /* Invariant (i): no false negatives for the MEST filter */
      for (int i = 0; i < NUM_TRIPS; i++)
        assert(! truth[i] || seen[i]);
      /* Invariant (iii): MEST candidate set no larger than the single-box
       * one and tightening (monotone non-increasing) as maxboxes grows */
      assert(cand <= single_count);
      assert(cand <= prev_count);
      prev_count = cand;

      total_mest_cand[m] += cand;
      total_mest_fp[m] += mest_fp;
    }
  }

  /* Report */
  printf("Ground truth (true segment-box overlaps), all queries: %ld\n\n",
    total_truth);
  printf("%-16s %14s %16s %12s\n", "configuration", "candidates",
    "false positives", "fp rate");
  printf("%-16s %14ld %16ld %11.1f%%\n", "single-box", total_single,
    total_single_fp,
    total_single ? 100.0 * (double) total_single_fp / (double) total_single :
    0.0);
  for (int m = 0; m < NUM_MAXBOXES; m++)
  {
    char label[16];
    snprintf(label, sizeof(label), "maxboxes=%d", MAXBOXES[m]);
    printf("%-16s %14ld %16ld %11.1f%%\n", label, total_mest_cand[m],
      total_mest_fp[m],
      total_mest_cand[m] ?
      100.0 * (double) total_mest_fp[m] / (double) total_mest_cand[m] : 0.0);
  }

  printf("\nInvariants\n");
  printf("  (i)   no false negatives for any configuration: OK\n");
  printf("  (ii)  dedup, each id at most once in MEST output: OK\n");
  bool tightening = true;
  long prev = total_single;
  for (int m = 0; m < NUM_MAXBOXES; m++)
  {
    if (total_mest_cand[m] > prev)
      tightening = false;
    prev = total_mest_cand[m];
  }
  printf("  (iii) MEST candidates <= single-box and tightening with "
    "maxboxes: %s\n", tightening ? "OK" : "NOT OBSERVED");
  printf("  (iv)  maxboxes <= 1 reproduces the single-box candidate set: "
    "OK\n");

  long best_cand = total_single;
  int best_max = 0;
  for (int m = 0; m < NUM_MAXBOXES; m++)
  {
    if (total_mest_cand[m] < best_cand)
    {
      best_cand = total_mest_cand[m];
      best_max = MAXBOXES[m];
    }
  }
  if (best_max == 0)
    printf("\nNo selectivity gain over the single-box index on this data; "
      "the splitter or maxboxes sweep needs tuning.\n");
  else
    printf("\nBest selectivity at maxboxes=%d: %ld candidates vs %ld for the "
      "single-box index (%.1f%% fewer).\n", best_max, best_cand, total_single,
      total_single ?
      100.0 * (double) (total_single - best_cand) / (double) total_single :
      0.0);

  /* Cleanup */
  free(truth);
  free(seen);
  meos_array_destroy(ids);
  for (int m = 0; m < NUM_MAXBOXES; m++)
    rtree_free(mest[m]);
  rtree_free(degenerate);
  rtree_free(single);
  for (int q = 0; q < NUM_QUERIES; q++)
    free(probes[q]);
  free(probes);
  for (int i = 0; i < NUM_TRIPS; i++)
  {
    free(trip_boxes[i]);
    free(trips[i]);
  }
  free(trip_boxes);
  free(trip_nboxes);
  free(trips);

  meos_finalize();
  return EXIT_SUCCESS;
}
