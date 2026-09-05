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
 * What the nearest-neighbour cursor refuses to open on is asserted separately.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sptree_test sptree_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>
#include <meos_geo.h>
#if POINTCLOUD
#include <meos_pointcloud.h>
#endif

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
compare(const char *label, const SPTree *sptree, IndexSearchOp op,
  const void *query, const bool *truth)
{
  MeosArray *result = meos_array_create(sizeof(int64));
  int count = sptree_search(sptree, op, query, result);
  bool *in_index = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < count; i++)
    in_index[*(int64 *) meos_array_get(result, i)] = true;

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
  compare("  overlaps    ", sptree, INDEX_OVERLAPS, query, ov);
  compare("  contains    ", sptree, INDEX_CONTAINS, query, co);
  compare("  contained by", sptree, INDEX_CONTAINED_BY, query, cb);

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
  compare("  overlaps    ", sptree, INDEX_OVERLAPS, query, ov);
  compare("  contains    ", sptree, INDEX_CONTAINS, query, co);
  compare("  contained by", sptree, INDEX_CONTAINED_BY, query, cb);

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
  compare("  overlaps    ", sptree, INDEX_OVERLAPS, query, ov);
  compare("  contains    ", sptree, INDEX_CONTAINS, query, co);
  compare("  contained by", sptree, INDEX_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}

/*****************************************************************************
 * Spatiotemporal box
 *****************************************************************************/

static STBox *
random_stbox(int sspan, int tspan)
{
  double xlo = random_int(-10000, 10000) / 10.0;
  double ylo = random_int(-10000, 10000) / 10.0;
  TimestampTz tlo = (TimestampTz) random_int(0, 1000000) * 1000000;
  Span *t = tstzspan_make(tlo, tlo + (TimestampTz) random_int(1, tspan) *
    1000000, true, false);
  STBox *box = stbox_make(true, false, false, 0, xlo,
    xlo + random_int(1, sspan) / 10.0, ylo, ylo + random_int(1, sspan) / 10.0,
    0, 0, t);
  free(t);
  return box;
}

static void
test_stbox(SPTreeKind kind, const char *kindname)
{
  STBox **boxes = malloc(NUM_BOXES * sizeof(STBox *));
  SPTree *sptree = sptree_create_stbox(kind);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    boxes[i] = random_stbox(200, 50);
    sptree_insert(sptree, boxes[i], i);
  }
  STBox *query = random_stbox(3000, 400);

  bool *ov = calloc(NUM_BOXES, sizeof(bool));
  bool *co = calloc(NUM_BOXES, sizeof(bool));
  bool *cb = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < NUM_BOXES; i++)
  {
    ov[i] = overlaps_stbox_stbox(boxes[i], query);
    co[i] = contains_stbox_stbox(boxes[i], query);
    cb[i] = contains_stbox_stbox(query, boxes[i]);
  }

  printf("Spatiotemporal box %s (%d random boxes):\n", kindname, NUM_BOXES);
  compare("  overlaps    ", sptree, INDEX_OVERLAPS, query, ov);
  compare("  contains    ", sptree, INDEX_CONTAINS, query, co);
  compare("  contained by", sptree, INDEX_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}

#if POINTCLOUD
/*****************************************************************************
 * Pointcloud spatiotemporal box (TPCBox)
 *
 * The tpcbox tree is internally an STBox tree (the boxes are projected on
 * entry), so the oracle here uses the native tpcbox predicates to prove that
 * the projection loses nothing. All boxes share the same pcid and srid, as
 * required by the tpcbox predicates.
 *****************************************************************************/

static TPCBox *
random_tpcbox(int sspan, int tspan)
{
  double xlo = random_int(-10000, 10000) / 10.0;
  double ylo = random_int(-10000, 10000) / 10.0;
  TimestampTz tlo = (TimestampTz) random_int(0, 1000000) * 1000000;
  Span *t = tstzspan_make(tlo, tlo + (TimestampTz) random_int(1, tspan) *
    1000000, true, false);
  TPCBox *box = tpcbox_make(true, false, true, false, 0, 1, xlo,
    xlo + random_int(1, sspan) / 10.0, ylo, ylo + random_int(1, sspan) / 10.0,
    0, 0, t);
  free(t);
  return box;
}

static void
test_tpcbox(SPTreeKind kind, const char *kindname)
{
  TPCBox **boxes = malloc(NUM_BOXES * sizeof(TPCBox *));
  SPTree *sptree = sptree_create_tpcbox(kind);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    boxes[i] = random_tpcbox(200, 50);
    sptree_insert(sptree, boxes[i], i);
  }
  TPCBox *query = random_tpcbox(3000, 400);

  bool *ov = calloc(NUM_BOXES, sizeof(bool));
  bool *co = calloc(NUM_BOXES, sizeof(bool));
  bool *cb = calloc(NUM_BOXES, sizeof(bool));
  for (int i = 0; i < NUM_BOXES; i++)
  {
    ov[i] = overlaps_tpcbox_tpcbox(boxes[i], query);
    co[i] = contains_tpcbox_tpcbox(boxes[i], query);
    cb[i] = contains_tpcbox_tpcbox(query, boxes[i]);
  }

  printf("Pointcloud box %s (%d random boxes):\n", kindname, NUM_BOXES);
  compare("  overlaps    ", sptree, INDEX_OVERLAPS, query, ov);
  compare("  contains    ", sptree, INDEX_CONTAINS, query, co);
  compare("  contained by", sptree, INDEX_CONTAINED_BY, query, cb);

  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(ov); free(co); free(cb);
  sptree_free(sptree);
}
#endif /* POINTCLOUD */

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

  MeosArray *single_ids = meos_array_create(sizeof(int64));
  MeosArray *mest_ids = meos_array_create(sizeof(int64));
  MeosArray *deg_ids = meos_array_create(sizeof(int64));
  int single_count = sptree_search_temporal(single, INDEX_OVERLAPS, query,
    single_ids);
  int mest_count = sptree_search_temporal_dedup(mest, INDEX_OVERLAPS, query,
    MAX_BOXES, mest_ids);
  int deg_count = sptree_search_temporal_dedup(deg, INDEX_OVERLAPS, query, 1,
    deg_ids);

  bool *in_single = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_mest = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_deg = calloc(NUM_TRIPS, sizeof(bool));
  int *mest_seen = calloc(NUM_TRIPS, sizeof(int));
  int *deg_seen = calloc(NUM_TRIPS, sizeof(int));
  for (int i = 0; i < single_count; i++)
    in_single[*(int64 *) meos_array_get(single_ids, i)] = true;
  for (int i = 0; i < mest_count; i++)
  {
    int64 id = *(int64 *) meos_array_get(mest_ids, i);
    in_mest[id] = true;
    mest_seen[id]++;
  }
  for (int i = 0; i < deg_count; i++)
  {
    int64 id = *(int64 *) meos_array_get(deg_ids, i);
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

/*****************************************************************************
 * Multi-entry (MEST) indexing over temporal geos
 *****************************************************************************/

/* Build a deliberately wiggly, high-extent tgeompoint trip */
static Temporal *
make_wiggly_trip(int seed, char *buf, size_t bufsize)
{
  double ox = random_double(0, 900);
  double oy = random_double(0, 900);
  double amp = random_double(40, 90);
  double phase = random_double(0, 6.28);
  size_t pos = 0;
  pos += (size_t) snprintf(buf + pos, bufsize - pos, "SRID=0;[");
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

static void
test_stbox_mest(void)
{
  char buf[MAX_LEN_TRIP];
  Temporal **trips = malloc(sizeof(Temporal *) * NUM_TRIPS);
  SPTree *single = sptree_create_stbox(SPTREE_QUADTREE);
  SPTree *mest = sptree_create_stbox(SPTREE_QUADTREE);
  SPTree *deg = sptree_create_stbox(SPTREE_QUADTREE);
  for (int i = 0; i < NUM_TRIPS; i++)
  {
    trips[i] = make_wiggly_trip(i, buf, sizeof(buf));
    sptree_insert_temporal(single, trips[i], i);
    sptree_insert_temporal_split(mest, trips[i], i, MAX_BOXES);
    sptree_insert_temporal_split(deg, trips[i], i, 1);
  }
  Temporal *query = make_wiggly_trip(123456, buf, sizeof(buf));

  MeosArray *single_ids = meos_array_create(sizeof(int64));
  MeosArray *mest_ids = meos_array_create(sizeof(int64));
  MeosArray *deg_ids = meos_array_create(sizeof(int64));
  int single_count = sptree_search_temporal(single, INDEX_OVERLAPS, query,
    single_ids);
  int mest_count = sptree_search_temporal_dedup(mest, INDEX_OVERLAPS, query,
    MAX_BOXES, mest_ids);
  int deg_count = sptree_search_temporal_dedup(deg, INDEX_OVERLAPS, query, 1,
    deg_ids);

  bool *in_single = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_mest = calloc(NUM_TRIPS, sizeof(bool));
  bool *in_deg = calloc(NUM_TRIPS, sizeof(bool));
  int *mest_seen = calloc(NUM_TRIPS, sizeof(int));
  int *deg_seen = calloc(NUM_TRIPS, sizeof(int));
  for (int i = 0; i < single_count; i++)
    in_single[*(int64 *) meos_array_get(single_ids, i)] = true;
  for (int i = 0; i < mest_count; i++)
  {
    int64 id = *(int64 *) meos_array_get(mest_ids, i);
    in_mest[id] = true;
    mest_seen[id]++;
  }
  for (int i = 0; i < deg_count; i++)
  {
    int64 id = *(int64 *) meos_array_get(deg_ids, i);
    in_deg[id] = true;
    deg_seen[id]++;
  }

  printf("MEST SPTree (tgeompoint, %d wiggly trips, maxboxes=%d):\n",
    NUM_TRIPS, MAX_BOXES);

  int qn;
  STBox *qboxes = tgeo_split_n_stboxes(query, MAX_BOXES, &qn);
  int missed = 0;
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
    if (overlap && ! in_mest[i])
      missed++;
  }
  free(qboxes);
  check("(i) no false negatives vs exact per-segment oracle", missed == 0);

  bool dedup_ok = true;
  for (int i = 0; i < NUM_TRIPS; i++)
    if (mest_seen[i] > 1 || deg_seen[i] > 1)
      dedup_ok = false;
  check("(ii) every surviving id appears exactly once", dedup_ok);

  bool subset_ok = (mest_count <= single_count);
  for (int i = 0; i < NUM_TRIPS; i++)
    if (in_mest[i] && ! in_single[i])
      subset_ok = false;
  check("(iii) MEST candidate set <= single-box candidate set", subset_ok);

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

/*****************************************************************************
 * Nearest-neighbour cursor
 *****************************************************************************/

/* Number of nearest neighbours checked by the early-stop property */
#define K 20
/* Tolerance for floating-point distance comparisons */
#define EPS 1e-9

static int
cmp_double(const void *a, const void *b)
{
  double x = *(const double *) a, y = *(const double *) b;
  return (x > y) - (x < y);
}

/* One-dimensional gap between two intervals, zero when they overlap */
static double
span_gap(double qlo, double qhi, double lo, double hi)
{
  if (hi < qlo)
    return qlo - hi;
  if (qhi < lo)
    return lo - qhi;
  return 0.0;
}

/* Drain the cursor fully, recording the id and distance sequence and how many
 * times each id is produced */
static int
drain(SPNNCursor *cursor, int64 *cur_id, double *cur_dist, int *seen)
{
  int n = 0;
  int64 id;
  double dist;
  while (sptree_nn_cursor_next(cursor, &id, &dist))
  {
    if (n < NUM_BOXES)
    {
      cur_id[n] = id;
      cur_dist[n] = dist;
    }
    if (id >= 0 && id < NUM_BOXES)
      seen[id]++;
    n++;
  }
  return n;
}

/* Check the metric-agnostic properties: completeness, monotonicity and that
 * the first K results match the full-drain prefix */
static void
check_structural(const char *label, SPTree *sptree, const void *query,
  const int64 *cur_id, const double *cur_dist, const int *seen, int n)
{
  char name[128];
  bool complete = (n == NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
    if (seen[i] != 1)
      complete = false;
  snprintf(name, sizeof(name), "%s completeness (every id once)", label);
  check(name, complete);

  bool monotone = true;
  for (int i = 1; i < n; i++)
    if (cur_dist[i] < cur_dist[i - 1] - EPS)
      monotone = false;
  snprintf(name, sizeof(name), "%s non-decreasing distances", label);
  check(name, monotone);

  bool early = true;
  SPNNCursor *kc = sptree_nn_cursor_open(sptree, query);
  for (int i = 0; i < K && early; i++)
  {
    int64 kid;
    double kdist;
    if (! sptree_nn_cursor_next(kc, &kid, &kdist) || kid != cur_id[i] ||
        fabs(kdist - cur_dist[i]) > EPS)
      early = false;
  }
  sptree_nn_cursor_close(kc);
  snprintf(name, sizeof(name), "%s first K match full-drain prefix", label);
  check(name, early);
}

/* Additionally check the reported distances against a brute-force oracle */
static void
check_metric(const char *label, const int64 *cur_id, const double *cur_dist,
  int n, const double *brute)
{
  char name[128];
  double *sorted = malloc(NUM_BOXES * sizeof(double));
  memcpy(sorted, brute, NUM_BOXES * sizeof(double));
  qsort(sorted, NUM_BOXES, sizeof(double), cmp_double);

  bool dist_ok = true;
  for (int i = 0; i < n; i++)
    if (fabs(cur_dist[i] - brute[cur_id[i]]) > EPS)
      dist_ok = false;
  snprintf(name, sizeof(name), "%s reported distance equals the box distance",
    label);
  check(name, dist_ok);

  bool order_ok = (n == NUM_BOXES);
  for (int i = 0; i < n && order_ok; i++)
    if (fabs(cur_dist[i] - sorted[i]) > EPS)
      order_ok = false;
  snprintf(name, sizeof(name), "%s order matches sorted brute-force distances",
    label);
  check(name, order_ok);
  free(sorted);
}

static void
test_nn_floatspan(void)
{
  double *lo = malloc(NUM_BOXES * sizeof(double));
  double *hi = malloc(NUM_BOXES * sizeof(double));
  SPTree *sptree = sptree_create_floatspan(SPTREE_QUADTREE);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    lo[i] = random_int(-10000, 10000) / 10.0;
    hi[i] = lo[i] + random_int(1, 200) / 10.0;
    Span *s = floatspan_make(lo[i], hi[i], true, false);
    sptree_insert(sptree, s, i);
    free(s);
  }
  double qlo = random_int(-10000, 10000) / 10.0;
  double qhi = qlo + 30.0;
  Span *query = floatspan_make(qlo, qhi, true, false);

  double *brute = malloc(NUM_BOXES * sizeof(double));
  for (int i = 0; i < NUM_BOXES; i++)
    brute[i] = span_gap(qlo, qhi, lo[i], hi[i]);

  int *seen = calloc(NUM_BOXES, sizeof(int));
  double *cd = malloc(NUM_BOXES * sizeof(double));
  int64 *ci = malloc(NUM_BOXES * sizeof(int64));
  SPNNCursor *cursor = sptree_nn_cursor_open(sptree, query);
  int n = drain(cursor, ci, cd, seen);
  sptree_nn_cursor_close(cursor);

  printf("NN SPTree (float span, %d random spans):\n", NUM_BOXES);
  check_structural("  ", sptree, query, ci, cd, seen, n);
  /* The span node distance uses distance_span_span, whose scale is not
   * replicated here; validate the ordering metric-robustly by checking the
   * first result is a box of globally minimum gap to the query */
  double minbrute = brute[0];
  for (int i = 1; i < NUM_BOXES; i++)
    if (brute[i] < minbrute)
      minbrute = brute[i];
  check("   nearest is a minimum-gap box",
    n > 0 && fabs(brute[ci[0]] - minbrute) <= EPS);

  free(lo); free(hi); free(query); free(brute);
  free(seen); free(cd); free(ci);
  sptree_free(sptree);
}

static void
test_nn_tbox(void)
{
  double *v = malloc(NUM_BOXES * sizeof(double));
  double *vh = malloc(NUM_BOXES * sizeof(double));
  /* A single shared time span makes every box overlap in time, so the distance
   * reduces to the value-dimension gap */
  Span *tspan = tstzspan_make(0, (TimestampTz) 1000000000, true, false);
  SPTree *sptree = sptree_create_tbox(SPTREE_QUADTREE);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    v[i] = random_int(-10000, 10000) / 10.0;
    vh[i] = v[i] + random_int(1, 200) / 10.0;
    Span *s = floatspan_make(v[i], vh[i], true, false);
    TBox *box = tbox_make(s, tspan);
    sptree_insert(sptree, box, i);
    free(s); free(box);
  }
  double qlo = random_int(-10000, 10000) / 10.0;
  double qhi = qlo + 30.0;
  Span *qs = floatspan_make(qlo, qhi, true, false);
  TBox *query = tbox_make(qs, tspan);
  free(qs);

  double *brute = malloc(NUM_BOXES * sizeof(double));
  for (int i = 0; i < NUM_BOXES; i++)
    brute[i] = span_gap(qlo, qhi, v[i], vh[i]);

  int *seen = calloc(NUM_BOXES, sizeof(int));
  double *cd = malloc(NUM_BOXES * sizeof(double));
  int64 *ci = malloc(NUM_BOXES * sizeof(int64));
  SPNNCursor *cursor = sptree_nn_cursor_open(sptree, query);
  int n = drain(cursor, ci, cd, seen);
  sptree_nn_cursor_close(cursor);

  printf("NN SPTree (temporal box, %d random boxes):\n", NUM_BOXES);
  check_structural("  ", sptree, query, ci, cd, seen, n);
  check_metric("  ", ci, cd, n, brute);

  free(v); free(vh); free(tspan); free(query); free(brute);
  free(seen); free(cd); free(ci);
  sptree_free(sptree);
}

static void
test_nn_stbox(void)
{
  STBox **boxes = malloc(NUM_BOXES * sizeof(STBox *));
  SPTree *sptree = sptree_create_stbox(SPTREE_QUADTREE);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    boxes[i] = random_stbox(200, 50);
    sptree_insert(sptree, boxes[i], i);
  }
  STBox *query = random_stbox(3000, 400);

  int *seen = calloc(NUM_BOXES, sizeof(int));
  double *cd = malloc(NUM_BOXES * sizeof(double));
  int64 *ci = malloc(NUM_BOXES * sizeof(int64));
  SPNNCursor *cursor = sptree_nn_cursor_open(sptree, query);
  int n = drain(cursor, ci, cd, seen);
  sptree_nn_cursor_close(cursor);

  printf("NN SPTree (spatiotemporal box, %d random boxes):\n", NUM_BOXES);
  check_structural("  ", sptree, query, ci, cd, seen, n);

  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(seen); free(cd); free(ci);
  sptree_free(sptree);
}

/*****************************************************************************
 * What the nearest-neighbour cursor refuses to open on
 *
 * A cursor validates its query box where the box becomes a query, so that the
 * traversal that follows may assume it, exactly as #sptree_search does. The
 * error handler installed here reports through #meos_errno instead of ending
 * the program, so that the refusal is read as a value rather than observed as
 * an exit
 *****************************************************************************/

static void
test_nn_refused(void)
{
  meos_initialize_noexit_error_handler();

  printf("What the NN SPTree cursor refuses to open on:\n");

  SPTree *sptree = sptree_create_stbox(SPTREE_QUADTREE);
  STBox *box = stbox_in("SRID=4326;STBOX X((0,0),(1,1))");
  STBox *other = stbox_in("SRID=3857;STBOX X((0,0),(1,1))");
  sptree_insert(sptree, box, 0);

  meos_errno_reset();
  check("  a query of another SRID is refused",
    sptree_nn_cursor_open(sptree, other) == NULL && meos_errno() != 0);

  meos_errno_reset();
  check("  a null index is refused",
    sptree_nn_cursor_open(NULL, box) == NULL && meos_errno() != 0);

  meos_errno_reset();
  check("  a null query is refused",
    sptree_nn_cursor_open(sptree, NULL) == NULL && meos_errno() != 0);

  /* A refused open answers NULL, so advancing that answer must report rather
   * than read through it */
  meos_errno_reset();
  check("  advancing a null cursor is refused",
    ! sptree_nn_cursor_next(NULL, NULL, NULL) && meos_errno() != 0);

  meos_errno_reset();
  SPNNCursor *cursor = sptree_nn_cursor_open(sptree, box);
  check("  a query of the tree's own SRID is accepted",
    cursor != NULL && meos_errno() == 0);
  sptree_nn_cursor_close(cursor);
  meos_errno_reset();

  free(box); free(other);
  sptree_free(sptree);
  return;
}


/*****************************************************************************
 * Selective windows
 *
 * A query covering the whole space is accepted by every inner node, so it
 * never prunes and never exercises the dimension a k-d level splits on. Only a
 * window narrower than the data does. These cases are deterministic and assert
 * a non-empty answer, so they cannot pass by matching nothing.
 *****************************************************************************/

#define SEL_BOXES 200

static void
selective(const char *label, const SPTree *sptree, const void *query,
  const bool *truth, int ntruth)
{
  MeosArray *result = meos_array_create(sizeof(int64));
  int count = sptree_search(sptree, INDEX_OVERLAPS, query, result);
  bool *in_index = calloc(SEL_BOXES, sizeof(bool));
  for (int i = 0; i < count; i++)
    in_index[*(int64 *) meos_array_get(result, i)] = true;
  int missed = 0;
  for (int i = 0; i < SEL_BOXES; i++)
    if (truth[i] && ! in_index[i])
      missed++;
  char name[128];
  snprintf(name, sizeof(name), "%s window matches %d, none missed", label,
    ntruth);
  check(name, ntruth > 0 && missed == 0);
  free(in_index);
  meos_array_destroy(result);
}

static void
test_selective_intspan(SPTreeKind kind, const char *kindname)
{
  Span *boxes[SEL_BOXES];
  SPTree *sptree = sptree_create_intspan(kind);
  for (int i = 0; i < SEL_BOXES; i++)
  {
    boxes[i] = intspan_make(i, i + 2, true, false);
    sptree_insert(sptree, boxes[i], i);
  }
  Span *query = intspan_make(10, 40, true, false);
  bool truth[SEL_BOXES];
  int n = 0;
  for (int i = 0; i < SEL_BOXES; i++)
    if ((truth[i] = overlaps_span_span(boxes[i], query)))
      n++;
  char label[64];
  snprintf(label, sizeof(label), "Integer span %s selective", kindname);
  selective(label, sptree, query, truth, n);
  for (int i = 0; i < SEL_BOXES; i++)
    free(boxes[i]);
  free(query);
  sptree_free(sptree);
}

static void
test_selective_tbox(SPTreeKind kind, const char *kindname)
{
  TBox *boxes[SEL_BOXES];
  SPTree *sptree = sptree_create_tbox(kind);
  for (int i = 0; i < SEL_BOXES; i++)
  {
    /* One dimension is held constant on purpose: splitting on it separates
     * nothing, so a level that splits the wrong dimension loses the rows. */
    Span *t = tstzspan_make(0, 86400000000, true, false);
    Span *v = floatspan_make(i, i + 2, true, false);
    boxes[i] = tbox_make(v, t);
    free(t); free(v);
    sptree_insert(sptree, boxes[i], i);
  }
  Span *qt = tstzspan_make(0, 2 * 86400000000, true, false);
  Span *qv = floatspan_make(10, 40, true, false);
  TBox *query = tbox_make(qv, qt);
  free(qt); free(qv);
  bool truth[SEL_BOXES];
  int n = 0;
  for (int i = 0; i < SEL_BOXES; i++)
    if ((truth[i] = overlaps_tbox_tbox(boxes[i], query)))
      n++;
  char label[64];
  snprintf(label, sizeof(label), "Temporal box %s selective", kindname);
  selective(label, sptree, query, truth, n);
  for (int i = 0; i < SEL_BOXES; i++)
    free(boxes[i]);
  free(query);
  sptree_free(sptree);
}

static void
test_selective_stbox(SPTreeKind kind, const char *kindname)
{
  STBox *boxes[SEL_BOXES];
  SPTree *sptree = sptree_create_stbox(kind);
  Span *t = tstzspan_make(0, 86400000000, true, false);
  for (int i = 0; i < SEL_BOXES; i++)
  {
    boxes[i] = stbox_make(true, false, false, 0, i, i + 1, i, i + 1, 0, 0, t);
    sptree_insert(sptree, boxes[i], i);
  }
  STBox *query = stbox_make(true, false, false, 0, 10, 40, 10, 40, 0, 0, t);
  free(t);
  bool truth[SEL_BOXES];
  int n = 0;
  for (int i = 0; i < SEL_BOXES; i++)
    if ((truth[i] = overlaps_stbox_stbox(boxes[i], query)))
      n++;
  char label[64];
  snprintf(label, sizeof(label), "Spatiotemporal box %s selective", kindname);
  selective(label, sptree, query, truth, n);
  for (int i = 0; i < SEL_BOXES; i++)
    free(boxes[i]);
  free(query);
  sptree_free(sptree);
}


/*****************************************************************************
 * What a tree reports it holds
 *
 * A node partitions the space into as many child slots as the space has
 * quadrants, and the nodes a tree ends in hold none of them. The size a tree
 * reports therefore charges the slots to the nodes that hold them, which is
 * asserted here without naming a byte count. The first entry of a tree costs
 * the node it makes; the second costs a node AND the slots the root acquires
 * to hold it. So the step from one entry to two exceeds the step from none to
 * one, and comparing the two steps cancels whatever fixed size a tree carries
 * before it holds anything.
 *****************************************************************************/

static void
test_reported_size(SPTreeKind kind, const char *kindname)
{
  char name[128];

  /* A tree holding nothing */
  SPTree *none = sptree_create_stbox(kind);
  MeosArray *result = meos_array_create(sizeof(int64));
  STBox *query = random_stbox(200, 50);
  int count = sptree_search(none, INDEX_OVERLAPS, query, result);
  snprintf(name, sizeof(name), "%s empty answers 0, not the error sentinel",
    kindname);
  check(name, count == 0 && count != INT_MAX);
  snprintf(name, sizeof(name), "%s empty holds no entry", kindname);
  check(name, sptree_num_entries(none) == 0);

  /* A tree of one entry: its only node never gains a child */
  SPTree *one = sptree_create_stbox(kind);
  STBox *a = random_stbox(200, 50);
  sptree_insert(one, a, 0);
  snprintf(name, sizeof(name), "%s one entry holds one", kindname);
  check(name, sptree_num_entries(one) == 1);
  snprintf(name, sizeof(name), "%s one entry reaches level 1", kindname);
  check(name, sptree_height(one) == 1);
  int64 size0 = sptree_mem_size(none);
  int64 size1 = sptree_mem_size(one);

  /* A second entry makes a node and gives the root its slots */
  SPTree *two = sptree_create_stbox(kind);
  sptree_insert(two, a, 0);
  STBox *b = random_stbox(200, 50);
  sptree_insert(two, b, 1);
  int64 size2 = sptree_mem_size(two);
  snprintf(name, sizeof(name), "%s two entries hold two", kindname);
  check(name, sptree_num_entries(two) == 2);
  snprintf(name, sizeof(name),
    "%s the slots are charged to the node holding them", kindname);
  check(name, size1 > size0 && (size2 - size1) > (size1 - size0));

  free(query); free(a); free(b);
  meos_array_destroy(result);
  sptree_free(none); sptree_free(one); sptree_free(two);
  return;
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
  test_stbox(SPTREE_QUADTREE, "quad-tree");
  test_stbox(SPTREE_KDTREE, "k-d tree");
#if POINTCLOUD
  test_tpcbox(SPTREE_QUADTREE, "quad-tree");
  test_tpcbox(SPTREE_KDTREE, "k-d tree");
#endif
  test_selective_intspan(SPTREE_QUADTREE, "quad-tree");
  test_selective_intspan(SPTREE_KDTREE, "k-d tree");
  test_selective_tbox(SPTREE_QUADTREE, "quad-tree");
  test_selective_tbox(SPTREE_KDTREE, "k-d tree");
  test_selective_stbox(SPTREE_QUADTREE, "quad-tree");
  test_selective_stbox(SPTREE_KDTREE, "k-d tree");
  test_mest();
  test_stbox_mest();
  test_nn_floatspan();
  test_nn_tbox();
  test_nn_stbox();
  test_reported_size(SPTREE_QUADTREE, "quad-tree");
  test_reported_size(SPTREE_KDTREE, "k-d tree");
  test_nn_refused();

  meos_finalize();

  if (failures == 0)
    printf("\nAll space-partitioning index tests passed.\n");
  else
    printf("\n%d space-partitioning index test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
