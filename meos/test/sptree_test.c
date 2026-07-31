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
 * @brief A program that tests the in-memory space-partitioning index
 * (quad-tree and k-d tree), i.e., sptree_create_*, sptree_insert and
 * sptree_search, against an exact brute-force oracle.
 *
 * For the integer span, float span and temporal box bounding box types, and
 * for both the quad-tree and the k-d tree kinds, a set of random boxes is
 * inserted and the overlaps, contains and contained-by searches are compared
 * with the exact set of boxes satisfying the operator. Two properties are
 * asserted per configuration and operator:
 *  (i)  no false negatives: every box satisfying the operator is a candidate;
 *  (ii) no false positives: every candidate satisfies the operator.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sptree_test sptree_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>

/* Number of boxes inserted into every index */
#define NUM_BOXES 2048
/* Number of temporal values in the multi-entry test */
#define NUM_TRIPS 1000
/* Number of instants per (wiggly) temporal value */
#define TRIP_LEN 40
/* Maximum number of boxes produced per temporal value in the MEST index */
#define MAX_BOXES 16
/* Maximum length in characters of a temporal value in text format */
#define MAX_LEN_TRIP 8192

/* Return a pseudo-random double in [min, max] */
static double
random_double(double min, double max)
{
  return min + (max - min) * ((double) rand() / (double) RAND_MAX);
}

static int failures = 0;

static void
check(const char *name, bool ok)
{
  printf("  %-58s %s\n", name, ok ? "OK" : "FAIL");
  if (! ok)
    failures++;
}

/* Return a pseudo-random integer in [min, max] */
static int
random_int(int min, int max)
{
  return min + rand() % (max - min + 1);
}

/*
 * Compare the candidate ids returned by the index for an operator against the
 * exact truth values computed by brute force. `truth[i]` is whether box i
 * satisfies the operator against the query.
 */
static void
compare(const char *label, const SPTree *sptree, RTreeSearchOp op,
  const void *query, const bool *truth)
{
  MeosArray *result = meos_array_create(sizeof(int));
  int count = sptree_search(sptree, op, query, result);
  bool *in_index = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < count; i++)
    in_index[*(int *) meos_array_get(result, i)] = true;

  int missed = 0, extra = 0;
  for (int i = 0; i < NUM_BOXES; i++)
  {
    if (truth[i] && ! in_index[i])
      missed++;
    if (! truth[i] && in_index[i])
      extra++;
  }
  char name[128];
  snprintf(name, sizeof(name), "%s no false negatives", label);
  check(name, missed == 0);
  snprintf(name, sizeof(name), "%s no false positives", label);
  check(name, extra == 0);

  free(in_index);
  meos_array_destroy(result);
}

/*****************************************************************************
 * Integer span
 *****************************************************************************/

static void
test_intspan(SPTreeKind kind, const char *kindname)
{
  Span **spans = malloc(NUM_BOXES * sizeof(Span *));
  SPTree *sptree = sptree_create_intspan(kind);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    int lo = random_int(-10000, 10000);
    spans[i] = intspan_make(lo, lo + random_int(1, 200), true, false);
    sptree_insert(sptree, spans[i], i);
  }
  int qlo = random_int(-10000, 10000);
  Span *query = intspan_make(qlo, qlo + 300, true, false);

  bool *ov = calloc(NUM_BOXES, sizeof(bool));
  bool *co = calloc(NUM_BOXES, sizeof(bool));
  bool *cb = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < NUM_BOXES; i++)
  {
    ov[i] = overlaps_span_span(spans[i], query);
    co[i] = contains_span_span(spans[i], query);
    cb[i] = contains_span_span(query, spans[i]);
  }

  printf("Integer span %s (%d random spans):\n", kindname, NUM_BOXES);
  compare("  overlaps    ", sptree, RTREE_OVERLAPS, query, ov);
  compare("  contains    ", sptree, RTREE_CONTAINS, query, co);
  compare("  contained by", sptree, RTREE_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(spans[i]);
  free(spans); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}

/*****************************************************************************
 * Float span
 *****************************************************************************/

static void
test_floatspan(SPTreeKind kind, const char *kindname)
{
  Span **spans = malloc(NUM_BOXES * sizeof(Span *));
  SPTree *sptree = sptree_create_floatspan(kind);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    double lo = random_int(-10000, 10000) / 10.0;
    spans[i] = floatspan_make(lo, lo + random_int(1, 200) / 10.0, true, false);
    sptree_insert(sptree, spans[i], i);
  }
  double qlo = random_int(-10000, 10000) / 10.0;
  Span *query = floatspan_make(qlo, qlo + 30.0, true, false);

  bool *ov = calloc(NUM_BOXES, sizeof(bool));
  bool *co = calloc(NUM_BOXES, sizeof(bool));
  bool *cb = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < NUM_BOXES; i++)
  {
    ov[i] = overlaps_span_span(spans[i], query);
    co[i] = contains_span_span(spans[i], query);
    cb[i] = contains_span_span(query, spans[i]);
  }

  printf("Float span %s (%d random spans):\n", kindname, NUM_BOXES);
  compare("  overlaps    ", sptree, RTREE_OVERLAPS, query, ov);
  compare("  contains    ", sptree, RTREE_CONTAINS, query, co);
  compare("  contained by", sptree, RTREE_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(spans[i]);
  free(spans); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}

/*****************************************************************************
 * Temporal box
 *****************************************************************************/

static TBox *
random_tbox(int vspan, int tspan)
{
  double vlo = random_int(-10000, 10000) / 10.0;
  Span *v = floatspan_make(vlo, vlo + random_int(1, vspan) / 10.0, true, false);
  TimestampTz tlo = (TimestampTz) random_int(0, 1000000) * 1000000;
  Span *t = tstzspan_make(tlo, tlo + (TimestampTz) random_int(1, tspan) *
    1000000, true, false);
  TBox *box = tbox_make(v, t);
  free(v); free(t);
  return box;
}

static void
test_tbox(SPTreeKind kind, const char *kindname)
{
  TBox **boxes = malloc(NUM_BOXES * sizeof(TBox *));
  SPTree *sptree = sptree_create_tbox(kind);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    boxes[i] = random_tbox(200, 50);
    sptree_insert(sptree, boxes[i], i);
  }
  TBox *query = random_tbox(3000, 400);

  bool *ov = calloc(NUM_BOXES, sizeof(bool));
  bool *co = calloc(NUM_BOXES, sizeof(bool));
  bool *cb = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < NUM_BOXES; i++)
  {
    ov[i] = overlaps_tbox_tbox(boxes[i], query);
    co[i] = contains_tbox_tbox(boxes[i], query);
    cb[i] = contains_tbox_tbox(query, boxes[i]);
  }

  printf("Temporal box %s (%d random boxes):\n", kindname, NUM_BOXES);
  compare("  overlaps    ", sptree, RTREE_OVERLAPS, query, ov);
  compare("  contains    ", sptree, RTREE_CONTAINS, query, co);
  compare("  contained by", sptree, RTREE_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}

/*****************************************************************************
 * Multi-entry (MEST) indexing over temporal numbers
 *****************************************************************************/

/*
 * Build a deliberately wiggly, high-extent tfloat: a sinusoidal signal that
 * sweeps a large value range even though every individual segment is short.
 * This is the shape for which a single bounding box is a poor approximation
 * and per-segment MEST boxes are far more selective.
 */
static Temporal *
make_wiggly_tfloat(int seed, char *buf, size_t bufsize)
{
  double base = random_double(0, 900);
  double amp = random_double(40, 90);
  double phase = random_double(0, 6.28);
  size_t pos = 0;
  pos += (size_t) snprintf(buf + pos, bufsize - pos, "[");
  for (int k = 0; k < TRIP_LEN; k++)
  {
    double v = base + amp * sin(phase + k * 0.9 + seed * 0.01);
    int mm = k / 60, ss = k % 60;
    pos += (size_t) snprintf(buf + pos, bufsize - pos,
      "%s%.4f@2020-01-01 %02d:%02d:00+00", (k == 0) ? "" : ", ", v, mm, ss);
  }
  snprintf(buf + pos, bufsize - pos, "]");
  return tfloat_in(buf);
}

static void
test_mest(void)
{
  char buf[MAX_LEN_TRIP];
  Temporal **trips = malloc(sizeof(Temporal *) * NUM_TRIPS);
  SPTree *single = sptree_create_tbox(SPTREE_QUADTREE);
  SPTree *mest = sptree_create_tbox(SPTREE_QUADTREE);
  SPTree *deg = sptree_create_tbox(SPTREE_QUADTREE);
  for (int i = 0; i < NUM_TRIPS; i++)
  {
    trips[i] = make_wiggly_tfloat(i, buf, sizeof(buf));
    sptree_insert_temporal(single, trips[i], i);
    sptree_insert_temporal_split(mest, trips[i], i, MAX_BOXES);
    sptree_insert_temporal_split(deg, trips[i], i, 1);
  }
  Temporal *query = make_wiggly_tfloat(123456, buf, sizeof(buf));

  MeosArray *single_ids = meos_array_create(sizeof(int));
  MeosArray *mest_ids = meos_array_create(sizeof(int));
  MeosArray *deg_ids = meos_array_create(sizeof(int));
  int single_count = sptree_search_temporal(single, RTREE_OVERLAPS, query,
    single_ids);
  int mest_count = sptree_search_temporal_dedup(mest, RTREE_OVERLAPS, query,
    MAX_BOXES, mest_ids);
  int deg_count = sptree_search_temporal_dedup(deg, RTREE_OVERLAPS, query, 1,
    deg_ids);

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

  printf("MEST SPTree (tfloat, %d wiggly values, maxboxes=%d):\n",
    NUM_TRIPS, MAX_BOXES);

  /* (i) No false negatives against the exact per-segment oracle: a value is a
   * true candidate iff one of its split boxes overlaps one of the query's
   * split boxes; the MEST search must return a superset of this set */
  int qn;
  TBox *qboxes = tnumber_split_n_tboxes(query, MAX_BOXES, &qn);
  int missed = 0;
  for (int i = 0; i < NUM_TRIPS; i++)
  {
    int tn;
    TBox *tboxes = tnumber_split_n_tboxes(trips[i], MAX_BOXES, &tn);
    bool overlap = false;
    for (int a = 0; a < tn && ! overlap; a++)
      for (int b = 0; b < qn && ! overlap; b++)
        if (overlaps_tbox_tbox(&tboxes[a], &qboxes[b]))
          overlap = true;
    free(tboxes);
    if (overlap && ! in_mest[i])
      missed++;
  }
  free(qboxes);
  check("(i) no false negatives vs exact per-segment oracle", missed == 0);

  /* (ii) Dedup: each surviving id appears exactly once */
  bool dedup_ok = true;
  for (int i = 0; i < NUM_TRIPS; i++)
    if (mest_seen[i] > 1 || deg_seen[i] > 1)
      dedup_ok = false;
  check("(ii) every surviving id appears exactly once", dedup_ok);

  /* (iii) The MEST candidate set is contained in the single-box candidate set
   * (every MEST box is contained in the value's minimum bounding box) */
  bool subset_ok = (mest_count <= single_count);
  for (int i = 0; i < NUM_TRIPS; i++)
    if (in_mest[i] && ! in_single[i])
      subset_ok = false;
  check("(iii) MEST candidate set <= single-box candidate set", subset_ok);

  /* (iv) Degeneracy: maxboxes <= 1 reproduces the single-box result exactly */
  bool deg_ok = (deg_count == single_count);
  for (int i = 0; i < NUM_TRIPS && deg_ok; i++)
    if (in_deg[i] != in_single[i])
      deg_ok = false;
  check("(iv) maxboxes<=1 identical to single-box result", deg_ok);

  free(in_single); free(in_mest); free(in_deg);
  free(mest_seen); free(deg_seen);
  meos_array_destroy(single_ids);
  meos_array_destroy(mest_ids);
  meos_array_destroy(deg_ids);
  free(query);
  for (int i = 0; i < NUM_TRIPS; i++)
    free(trips[i]);
  free(trips);
  sptree_free(single);
  sptree_free(mest);
  sptree_free(deg);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  srand(1);

  test_intspan(SPTREE_QUADTREE, "quad-tree");
  test_intspan(SPTREE_KDTREE, "k-d tree");
  test_floatspan(SPTREE_QUADTREE, "quad-tree");
  test_floatspan(SPTREE_KDTREE, "k-d tree");
  test_tbox(SPTREE_QUADTREE, "quad-tree");
  test_tbox(SPTREE_KDTREE, "k-d tree");
  test_mest();

  meos_finalize();

  if (failures == 0)
    printf("\nAll space-partitioning index tests passed.\n");
  else
    printf("\n%d space-partitioning index test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
