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
 * @brief A simple program that demonstrates the SPTree index for searching
 * MEOS bounding boxes: floatspan, tstzspan, tbox, stbox, and temporal types
 * @details The sibling of rtree_example.c over the space-partitioning tree,
 * run for each of its two kinds, the quad-tree and the k-d tree. The program
 * tests all bounding box types with OVERLAPS, CONTAINS,
 * and
 * CONTAINED_BY operations, inserting random boxes into the index and
 * verifying search results against a brute-force scan. It also demonstrates
 * the temporal convenience functions (sptree_insert_temporal,
 * sptree_search_temporal) using tfloat sequences.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sptree_example sptree_example.c -L/usr/local/lib -lmeos -lproj
 * @endcode
 */

/* C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>

/* Maximum number of bounding boxes */
#define NUM_BBOX 10000
/* Maximum length in characters of a bounding box string */
#define MAX_LEN_BBOX 128

/* Return a random integer in [min, max] */
static int
random_int(int min, int max)
{
  return rand() % (max - min + 1) + min;
}

/**
 * @brief Generic test harness: compare space-partitioning tree search results
 * against a brute-force scan using the given predicate
 * @param[in] name Label for the test (e.g., "FLOATSPAN overlaps")
 * @param[in] sptree The space-partitioning tree to search
 * @param[in] op The search operation (overlaps, contains, contained by)
 * @param[in] query The query bounding box
 * @param[in] boxes Array of stored bounding boxes
 * @param[in] bboxsize Size of each bounding box in bytes
 * @param[in] ids Reusable MeosArray for collecting results
 * @param[in] predicate Brute-force predicate matching the search operation
 */
static void
verify_search(const char *name, const SPTree *sptree, IndexSearchOp op,
  const void *query, const char *boxes, size_t bboxsize, MeosArray *ids,
  bool (*predicate)(const void *, const void *))
{
  /* Index search — reuses the caller's MeosArray (reset internally) */
  clock_t t = clock();
  int index_count = sptree_search(sptree, op, query, ids);
  double index_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  /* Brute-force search */
  t = clock();
  int brute_count = 0;
  bool *actual = calloc(NUM_BBOX, sizeof(bool));
  for (int i = 0; i < NUM_BBOX; i++)
  {
    if (predicate(boxes + i * bboxsize, query))
    {
      brute_count++;
      actual[i] = true;
    }
  }
  double brute_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  /* Compare results */
  bool *indexed = calloc(NUM_BBOX, sizeof(bool));
  for (int i = 0; i < index_count; i++)
  {
    int64 id;
    index_result_id(ids, i, &id);
    indexed[id] = true;
  }

  int errors = 0;
  for (int i = 0; i < NUM_BBOX; i++)
  {
    if (indexed[i] != actual[i])
      errors++;
  }

  printf("  %-20s  index: %.6fs (%d hits)  brute: %.6fs (%d hits) ratio: %.6f %s\n",
    name, index_time, index_count, brute_time, brute_count, index_time / brute_time,
    errors == 0 ? "OK" : "MISMATCH");

  free(actual);
  free(indexed);
}

/*****************************************************************************/

/* Span predicates */
static bool
overlaps_span_wrapper(const void *a, const void *b)
{ return overlaps_span_span((Span *) a, (Span *) b); }
static bool
contains_span_wrapper(const void *a, const void *b)
{ return contains_span_span((Span *) a, (Span *) b); }
static bool
contained_span_wrapper(const void *a, const void *b)
{ return contained_span_span((Span *) a, (Span *) b); }

/* TBox predicates */
static bool
overlaps_tbox_wrapper(const void *a, const void *b)
{ return overlaps_tbox_tbox((TBox *) a, (TBox *) b); }
static bool
contains_tbox_wrapper(const void *a, const void *b)
{ return contains_tbox_tbox((TBox *) a, (TBox *) b); }
static bool
contained_tbox_wrapper(const void *a, const void *b)
{ return contained_tbox_tbox((TBox *) a, (TBox *) b); }

/* STBox predicates */
static bool
overlaps_stbox_wrapper(const void *a, const void *b)
{ return overlaps_stbox_stbox((STBox *) a, (STBox *) b); }
static bool
contains_stbox_wrapper(const void *a, const void *b)
{ return contains_stbox_stbox((STBox *) a, (STBox *) b); }
static bool
contained_stbox_wrapper(const void *a, const void *b)
{ return contained_stbox_stbox((STBox *) a, (STBox *) b); }

/*****************************************************************************/

static void
test_floatspan(MeosArray *ids, SPTreeKind kind)
{
  char str[MAX_LEN_BBOX];
  Span *boxes = malloc(sizeof(Span) * NUM_BBOX);
  SPTree *sptree = sptree_create_floatspan(kind);

  for (int i = 0; i < NUM_BBOX; i++)
  {
    int lo = random_int(1, 1000);
    int hi = lo + random_int(1, 10);
    snprintf(str, sizeof(str), "[%d, %d]", lo, hi);
    Span *s = floatspan_in(str);
    boxes[i] = *s;
    sptree_insert(sptree, &boxes[i], i);
    free(s);
  }

  printf("FLOATSPAN:\n");
  Span *query = floatspan_in("[0, 100]");
  verify_search("overlaps", sptree, INDEX_OVERLAPS, query,
    (char *) boxes, sizeof(Span), ids, overlaps_span_wrapper);
  /* A box contains itself, so the stored box below has one entry at least */
  verify_search("contains", sptree, INDEX_CONTAINS, &boxes[NUM_BBOX / 2],
    (char *) boxes, sizeof(Span), ids, contains_span_wrapper);
  verify_search("contained by", sptree, INDEX_CONTAINED_BY, query,
    (char *) boxes, sizeof(Span), ids, contained_span_wrapper);

  free(query);
  sptree_free(sptree);
  free(boxes);
}

static void
test_tstzspan(MeosArray *ids, SPTreeKind kind)
{
  char str[MAX_LEN_BBOX];
  Span *boxes = malloc(sizeof(Span) * NUM_BBOX);
  SPTree *sptree = sptree_create_tstzspan(kind);

  for (int i = 0; i < NUM_BBOX; i++)
  {
    int tmin = random_int(0, 999);
    int tmax = tmin + random_int(1, 10);
    int tmin_h = tmin / 60, tmin_m = tmin % 60;
    int tmax_h = tmax / 60, tmax_m = tmax % 60;
    snprintf(str, sizeof(str),
      "[2020-01-01 %02d:%02d:00+00, 2020-01-01 %02d:%02d:00+00]",
      tmin_h, tmin_m, tmax_h, tmax_m);
    Span *s = tstzspan_in(str);
    boxes[i] = *s;
    sptree_insert(sptree, &boxes[i], i);
    free(s);
  }

  printf("TSTZSPAN:\n");
  /* Query covers ~100/1000 = 10% of the time range */
  Span *query = tstzspan_in(
    "[2020-01-01 00:00:00+00, 2020-01-01 01:40:00+00]");
  verify_search("overlaps", sptree, INDEX_OVERLAPS, query,
    (char *) boxes, sizeof(Span), ids, overlaps_span_wrapper);
  /* A box contains itself, so the stored box below has one entry at least */
  verify_search("contains", sptree, INDEX_CONTAINS, &boxes[NUM_BBOX / 2],
    (char *) boxes, sizeof(Span), ids, contains_span_wrapper);
  verify_search("contained by", sptree, INDEX_CONTAINED_BY, query,
    (char *) boxes, sizeof(Span), ids, contained_span_wrapper);

  free(query);
  sptree_free(sptree);
  free(boxes);
}

static void
test_tbox(MeosArray *ids, SPTreeKind kind)
{
  char str[MAX_LEN_BBOX];
  TBox *boxes = malloc(sizeof(TBox) * NUM_BBOX);
  SPTree *sptree = sptree_create_tbox(kind);

  for (int i = 0; i < NUM_BBOX; i++)
  {
    int xmin = random_int(1, 1000);
    int xmax = xmin + random_int(1, 10);
    int tmin = random_int(0, 999);
    int tmax = tmin + random_int(1, 10);
    int tmin_h = tmin / 60, tmin_m = tmin % 60;
    int tmax_h = tmax / 60, tmax_m = tmax % 60;
    snprintf(str, sizeof(str),
      "TBOX XT([%d, %d],"
      "[2020-01-01 %02d:%02d:00+00, 2020-01-01 %02d:%02d:00+00])",
      xmin, xmax, tmin_h, tmin_m, tmax_h, tmax_m);
    TBox *b = tbox_in(str);
    boxes[i] = *b;
    sptree_insert(sptree, &boxes[i], i);
    free(b);
  }

  printf("TBOX:\n");
  TBox *query = tbox_in(
    "TBOX XT([0,100],[2020-01-01 00:00:00+00, 2020-01-01 01:40:00+00])");
  verify_search("overlaps", sptree, INDEX_OVERLAPS, query,
    (char *) boxes, sizeof(TBox), ids, overlaps_tbox_wrapper);
  /* A box contains itself, so the stored box below has one entry at least */
  verify_search("contains", sptree, INDEX_CONTAINS, &boxes[NUM_BBOX / 2],
    (char *) boxes, sizeof(TBox), ids, contains_tbox_wrapper);
  verify_search("contained by", sptree, INDEX_CONTAINED_BY, query,
    (char *) boxes, sizeof(TBox), ids, contained_tbox_wrapper);

  free(query);
  sptree_free(sptree);
  free(boxes);
}

static void
test_stbox(MeosArray *ids, SPTreeKind kind)
{
  char str[MAX_LEN_BBOX];
  STBox *boxes = malloc(sizeof(STBox) * NUM_BBOX);
  SPTree *sptree = sptree_create_stbox(kind);

  for (int i = 0; i < NUM_BBOX; i++)
  {
    int xmin = random_int(1, 1000);
    int xmax = xmin + random_int(1, 10);
    int ymin = random_int(1, 1000);
    int ymax = ymin + random_int(1, 10);
    int tmin = random_int(0, 999);
    int tmax = tmin + random_int(1, 10);
    int tmin_h = tmin / 60, tmin_m = tmin % 60;
    int tmax_h = tmax / 60, tmax_m = tmax % 60;
    snprintf(str, sizeof(str),
      "SRID=25832;STBOX XT(((%d %d),(%d %d)),"
      "[2020-01-01 %02d:%02d:00+00, 2020-01-01 %02d:%02d:00+00])",
      xmin, ymin, xmax, ymax, tmin_h, tmin_m, tmax_h, tmax_m);
    STBox *b = stbox_in(str);
    boxes[i] = *b;
    sptree_insert(sptree, &boxes[i], i);
    free(b);
  }

  printf("STBOX:\n");
  STBox *query = stbox_in(
    "SRID=25832;STBOX XT(((0 0),(100 100)),"
    "[2020-01-01 00:00:00+00, 2020-01-01 01:40:00+00])");
  verify_search("overlaps", sptree, INDEX_OVERLAPS, query,
    (char *) boxes, sizeof(STBox), ids, overlaps_stbox_wrapper);
  /* A box contains itself, so the stored box below has one entry at least */
  verify_search("contains", sptree, INDEX_CONTAINS, &boxes[NUM_BBOX / 2],
    (char *) boxes, sizeof(STBox), ids, contains_stbox_wrapper);
  verify_search("contained by", sptree, INDEX_CONTAINED_BY, query,
    (char *) boxes, sizeof(STBox), ids, contained_stbox_wrapper);

  free(query);
  sptree_free(sptree);
  free(boxes);
}

/*****************************************************************************/

/**
 * @brief Demonstrate sptree_insert_temporal / sptree_search_temporal with
 * tfloat
 */
static void
test_temporal(MeosArray *ids, SPTreeKind kind)
{
  char str[MAX_LEN_BBOX];
  /* Store the bounding boxes for brute-force verification */
  TBox *boxes = malloc(sizeof(TBox) * NUM_BBOX);
  SPTree *sptree = sptree_create_tbox(kind);

  for (int i = 0; i < NUM_BBOX; i++)
  {
    int xmin = random_int(1, 1000);
    int xmax = xmin + random_int(1, 10);
    int tmin = random_int(0, 999);
    int tmax = tmin + random_int(1, 10);
    int tmin_h = tmin / 60, tmin_m = tmin % 60;
    int tmax_h = tmax / 60, tmax_m = tmax % 60;
    snprintf(str, sizeof(str),
      "[%d@2020-01-01 %02d:%02d:00+00, %d@2020-01-01 %02d:%02d:00+00]",
      xmin, tmin_h, tmin_m, xmax, tmax_h, tmax_m);
    Temporal *temp = (Temporal *) tfloat_in(str);
    /* Use the temporal insert convenience function */
    sptree_insert_temporal(sptree, temp, i);
    /* Extract bounding box for brute-force comparison */
    TBox *b = tnumber_to_tbox(temp);
    boxes[i] = *b;
    free(b);
    free(temp);
  }

  printf("TEMPORAL (tfloat):\n");
  /* Build a tfloat query and use sptree_search_temporal */
  Temporal *query = (Temporal *) tfloat_in(
    "[0@2020-01-01 00:00:00+00, 100@2020-01-01 01:40:00+00]");
  TBox *query_box = tnumber_to_tbox(query);

  /* Index search via temporal API */
  clock_t t = clock();
  int index_count = sptree_search_temporal(sptree, INDEX_OVERLAPS, query, ids);
  double index_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  /* Brute-force search */
  t = clock();
  int brute_count = 0;
  for (int i = 0; i < NUM_BBOX; i++)
  {
    if (overlaps_tbox_tbox(&boxes[i], query_box))
      brute_count++;
  }
  double brute_time = (double)(clock() - t) / CLOCKS_PER_SEC;

  printf("  %-20s  index: %.6fs (%d hits)  brute: %.6fs (%d hits) ratio: %.6f %s\n",
    "overlaps", index_time, index_count, brute_time, brute_count,
    index_time / brute_time,
    index_count == brute_count ? "OK" : "MISMATCH");

  free(query_box);
  free(query);
  sptree_free(sptree);
  free(boxes);
}

/*****************************************************************************/

int main(void)
{
  meos_initialize();

  /* Create a single MeosArray and reuse it across all searches */
  MeosArray *ids = index_result_create();
  const SPTreeKind kinds[2] = {SPTREE_QUADTREE, SPTREE_KDTREE};
  const char *names[2] = {"quad-tree", "k-d tree"};
  for (int k = 0; k < 2; k++)
  {
    /* Use fixed seed for reproducibility, the same boxes for every kind */
    srand(1);
    printf("SPTree index test, %s (%d boxes per type)\n", names[k], NUM_BBOX);
    test_floatspan(ids, kinds[k]);
    test_tstzspan(ids, kinds[k]);
    test_tbox(ids, kinds[k]);
    test_stbox(ids, kinds[k]);
    test_temporal(ids, kinds[k]);
  }
  meos_array_destroy(ids);

  meos_finalize();
  return EXIT_SUCCESS;
}
