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
 * @brief A program that tests the bulk build of the in-memory RTree index,
 * i.e., rtree_load, against a tree of the same entries built one at a time by
 * rtree_insert.
 *
 * rtree_load packs the entries bottom-up by Sort-Tile-Recursive, so the tree it
 * produces has a different shape from one grown by repeated insertion. The
 * shape is free to differ; the answers are not. The insert-built tree is
 * therefore the oracle, and the two trees are required to agree exactly.
 *
 * Five properties are asserted:
 *  (i)   same answers: over a spread of query windows the two trees return
 *        identical id sets, compared as sorted sequences so that a difference
 *        in traversal order is not mistaken for a difference in results;
 *  (ii)  the comparison is not vacuous: those windows must match a substantial
 *        number of entries, since two trees that both answer nothing agree on
 *        nothing;
 *  (iii) completeness: a window covering the whole extent returns every id, so
 *        no entry is dropped by the packing;
 *  (iv)  the ids survive: they are spread beyond 2^31, so a build carrying them
 *        at a narrower width would return different numbers;
 *  (v)   the degenerate counts are handled: loading no entries leaves a tree
 *        that answers nothing, and loading one returns that one;
 *  (vi)  a load leaves the tree holding exactly the entries it is given,
 *        whatever it holds when it is called, so an index is rebuilt from the
 *        entries a caller keeps;
 *  (vii) the entries a tree holds are released whatever the number given, so a
 *        load of no entries empties a tree rather than leaving the ones it
 *        held. This is the case that tells releasing the nodes apart from
 *        assigning the root over them: wherever entries follow, the two answer
 *        alike and differ only in what the run leaks, which valgrind reports
 *        and an assertion cannot.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_load_test rtree_load_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Entries the two trees are built from, enough to span several tree levels */
#define NUM_BOXES 2048
/* Query windows the two trees are compared over */
#define NUM_QUERIES 200
/* Fewest hits those windows must produce for the comparison to mean anything */
#define MIN_EXPECTED_HITS 1000
/* Lowest id, above 2^31 so that a narrower carrier would alter it */
#define ID_BASE 4000000000LL
/* Entries a tree holds before it is loaded over */
#define NUM_SEEDED 8
/* Lowest seeded id, below every loaded id so that one left behind is visible */
#define SEED_ID_BASE 1000000000LL

static int
cmp_int64(const void *a, const void *b)
{
  int64 x = *(const int64 *) a, y = *(const int64 *) b;
  return (x > y) - (x < y);
}

/**
 * @brief Return the ids a tree reports for a query, sorted ascending
 */
static int64 *
search_sorted(const RTree *rtree, const STBox *query, int *count)
{
  MeosArray *result = meos_array_create(sizeof(int64));
  rtree_search(rtree, INDEX_OVERLAPS, query, result);
  *count = meos_array_count(result);
  int64 *ids = malloc(sizeof(int64) * (size_t) (*count ? *count : 1));
  for (int i = 0; i < *count; i++)
    ids[i] = *(int64 *) meos_array_get(result, i);
  qsort(ids, (size_t) *count, sizeof(int64), cmp_int64);
  meos_array_destroy(result);
  return ids;
}

int
main(void)
{
  meos_initialize();
  int failures = 0;
  char buf[256];

  /* The entries both trees are built from */
  STBox *boxes = malloc(sizeof(STBox) * NUM_BOXES);
  int64 *ids = malloc(sizeof(int64) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    int x = i % 64, y = i / 64;
    snprintf(buf, sizeof(buf),
      "STBOX XT(((%d,%d),(%d,%d)),[2001-01-01,2001-01-02])", x, y, x + 1, y + 1);
    STBox *box = stbox_in(buf);
    memcpy(&boxes[i], box, sizeof(STBox));
    free(box);
    ids[i] = (int64) i + ID_BASE;
  }

  RTree *grown = rtree_create_stbox();
  for (int i = 0; i < NUM_BOXES; i++)
    rtree_insert(grown, &boxes[i], ids[i]);

  RTree *packed = rtree_create_stbox();
  rtree_load(packed, boxes, ids, NUM_BOXES);

  /* (i) the two trees answer every window identically */
  int mismatches = 0, total_hits = 0;
  for (int q = 0; q < NUM_QUERIES; q++)
  {
    int x = (q * 7) % 60, y = (q * 11) % 30;
    snprintf(buf, sizeof(buf),
      "STBOX XT(((%d,%d),(%d,%d)),[2001-01-01,2001-01-02])", x, y, x + 5, y + 5);
    STBox *query = stbox_in(buf);
    int ngrown, npacked;
    int64 *g = search_sorted(grown, query, &ngrown);
    int64 *p = search_sorted(packed, query, &npacked);
    if (ngrown != npacked ||
        (ngrown && memcmp(g, p, sizeof(int64) * (size_t) ngrown) != 0))
      mismatches++;
    total_hits += ngrown;
    free(g);
    free(p);
    free(query);
  }
  if (mismatches)
  {
    printf("rtree_load: %d of %d windows answered differently from the "
      "insert-built tree\n", mismatches, NUM_QUERIES);
    failures++;
  }

  /* (ii) two trees that answer nothing agree on nothing */
  if (total_hits < MIN_EXPECTED_HITS)
  {
    printf("rtree_load: the windows matched %d entries, fewer than the %d the "
      "comparison needs to be meaningful\n", total_hits, MIN_EXPECTED_HITS);
    failures++;
  }

  /* (iii) and (iv) a window over the whole extent returns every id unchanged */
  snprintf(buf, sizeof(buf),
    "STBOX XT(((-1,-1),(1000,1000)),[2001-01-01,2001-01-02])");
  STBox *whole = stbox_in(buf);
  int nall;
  int64 *got = search_sorted(packed, whole, &nall);
  if (nall != NUM_BOXES)
  {
    printf("rtree_load: a window over the whole extent returned %d of %d "
      "entries\n", nall, NUM_BOXES);
    failures++;
  }
  else
  {
    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    memcpy(want, ids, sizeof(int64) * NUM_BOXES);
    qsort(want, NUM_BOXES, sizeof(int64), cmp_int64);
    if (memcmp(got, want, sizeof(int64) * NUM_BOXES) != 0)
    {
      printf("rtree_load: the ids returned are not the ids loaded\n");
      failures++;
    }
    free(want);
  }
  free(got);

  /* (v) the degenerate counts */
  RTree *empty = rtree_create_stbox();
  rtree_load(empty, boxes, ids, 0);
  int nempty;
  int64 *e = search_sorted(empty, whole, &nempty);
  if (nempty != 0)
  {
    printf("rtree_load: a tree loaded with no entries answered %d\n", nempty);
    failures++;
  }
  free(e);

  RTree *single = rtree_create_stbox();
  rtree_load(single, boxes, ids, 1);
  int nsingle;
  int64 *s = search_sorted(single, whole, &nsingle);
  if (nsingle != 1 || s[0] != ids[0])
  {
    printf("rtree_load: a tree loaded with one entry answered %d\n", nsingle);
    failures++;
  }
  free(s);

  /* (vi) a load leaves the tree holding exactly the entries given, whatever it
   * holds when it is called. The seeded ids are below every loaded id, so an
   * entry a rebuild fails to drop appears in the answer instead of hiding
   * among the ids that are loaded again. */
  RTree *rebuilt = rtree_create_stbox();
  for (int i = 0; i < NUM_SEEDED; i++)
    rtree_insert(rebuilt, &boxes[i], SEED_ID_BASE + (int64) i);
  rtree_load(rebuilt, boxes, ids, NUM_BOXES);
  int nrebuilt;
  int64 *r = search_sorted(rebuilt, whole, &nrebuilt);
  if (nrebuilt != NUM_BOXES)
  {
    printf("rtree_load: a tree holding %d entries answered %d after a load of "
      "%d entries\n", NUM_SEEDED, nrebuilt, NUM_BOXES);
    failures++;
  }
  else
  {
    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    memcpy(want, ids, sizeof(int64) * NUM_BOXES);
    qsort(want, NUM_BOXES, sizeof(int64), cmp_int64);
    if (memcmp(r, want, sizeof(int64) * NUM_BOXES) != 0)
    {
      printf("rtree_load: a tree loaded over the entries it holds answers ids "
        "it was not loaded with\n");
      failures++;
    }
    free(want);
  }
  free(r);

  /* (vii) the entries are released whatever the number given, so an empty
   * entry set leaves an empty tree rather than the one it held */
  RTree *emptied = rtree_create_stbox();
  for (int i = 0; i < NUM_SEEDED; i++)
    rtree_insert(emptied, &boxes[i], SEED_ID_BASE + (int64) i);
  rtree_load(emptied, boxes, ids, 0);
  int nemptied;
  int64 *m = search_sorted(emptied, whole, &nemptied);
  if (nemptied != 0)
  {
    printf("rtree_load: a tree holding %d entries answered %d after a load of "
      "no entries\n", NUM_SEEDED, nemptied);
    failures++;
  }
  free(m);
  free(whole);

  rtree_free(grown);
  rtree_free(packed);
  rtree_free(empty);
  rtree_free(single);
  rtree_free(rebuilt);
  rtree_free(emptied);
  free(boxes);
  free(ids);

  if (failures == 0)
    printf("rtree_load test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
