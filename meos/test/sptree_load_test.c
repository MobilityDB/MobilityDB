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
 * @brief A program that tests the build of the in-memory space-partitioning
 * index from a whole entry set, i.e., sptree_load, for both kinds of tree.
 *
 * sptree_load partitions the entries top-down on the median of the dimension
 * each level narrows, so the tree it produces has a different shape from one
 * grown by repeated sptree_insert, whose depth follows the order the entries
 * arrive in. The shape is free to differ; the answers are not.
 *
 * Two oracles are used rather than one. The exact set of boxes satisfying the
 * operator is computed by brute force, so a build and a search that share a
 * mistake cannot agree their way to a pass; and the insert-built tree is
 * required to answer identically, so that the two builds of one index are held
 * to one contract.
 *
 * Seven properties are asserted, for the quad-tree and for the k-d tree:
 *  (i)    exactness: the tree returns every box satisfying the operator and no
 *         other;
 *  (ii)   the comparison is not vacuous: the windows must match a substantial
 *         number of entries, since a tree that answers nothing and an oracle
 *         that expects nothing agree on nothing;
 *  (iii)  completeness: a window over the whole extent returns each loaded id
 *         exactly once, so the partitioning neither drops an entry nor reports
 *         one twice;
 *  (iv)   an ordered entry set is answered exactly. This is the set that makes
 *         insertion degenerate into a chain, and the one a build that halves
 *         its range is written for;
 *  (v)    an entry set that ties throughout is answered exactly. Equal boxes
 *         cannot be separated by any dimension, so they descend as a chain,
 *         and a build that expects every level to split does not terminate;
 *  (vi)   the degenerate counts are handled: loading no entries leaves a tree
 *         that answers nothing, and loading one returns that one;
 *  (vii)  a load leaves the tree holding exactly the entries it is given,
 *         whatever it holds when it is called, and releases what it held
 *         whatever the number given, so a load of no entries empties a tree.
 *
 * The fixture is built so that a partitioning fault cannot pass unnoticed. The
 * dimensions are uncorrelated and the temporal one is held constant across
 * every box and every query, so a level that narrows the wrong dimension
 * separates nothing; and the query windows are on the scale of the data, since
 * a window covering the extent is accepted by every node and prunes nothing.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sptree_load_test sptree_load_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Entries the trees are built from, enough to span several tree levels */
#define NUM_BOXES 2048
/* Query windows the trees are compared over */
#define NUM_QUERIES 100
/* Fewest hits those windows must produce for the comparison to mean anything */
#define MIN_EXPECTED_HITS 1000
/* Lowest id, above 2^31 so that a narrower carrier would alter it */
#define ID_BASE 4000000000LL
/* Entries a tree holds before it is loaded over */
#define NUM_SEEDED 8
/* Lowest seeded id, below every loaded id so that one left behind is visible */
#define SEED_ID_BASE 1000000000LL
/* Half-width of a value span, and of a query window, in tenths */
#define BOX_SPAN 200
#define QUERY_SPAN 600

static int
cmp_int64(const void *a, const void *b)
{
  int64 x = *(const int64 *) a, y = *(const int64 *) b;
  return (x > y) - (x < y);
}

static int
random_int(int min, int max)
{
  return min + rand() % (max - min + 1);
}

/**
 * @brief Return the time span every box and every query carries
 * @details One span shared by the whole fixture: the temporal dimension
 * therefore separates nothing, and every prune a search makes has to come from
 * the value dimension
 */
static Span *
constant_period(void)
{
  return tstzspan_make((TimestampTz) 0, (TimestampTz) 1000000000, true, false);
}

/**
 * @brief Return a box whose value span starts at @p vlo tenths and whose time
 * span is the one the whole fixture shares
 */
static TBox *
make_tbox(int vlo, int vspan)
{
  Span *v = floatspan_make(vlo / 10.0, (vlo + vspan) / 10.0, true, false);
  Span *t = constant_period();
  TBox *box = tbox_make(v, t);
  free(v); free(t);
  return box;
}

/**
 * @brief Return the ids a tree reports for a query, sorted ascending
 */
static int64 *
search_sorted(const SPTree *sptree, const TBox *query, int *count)
{
  MeosArray *result = meos_array_create(sizeof(int64));
  sptree_search(sptree, INDEX_OVERLAPS, query, result);
  *count = meos_array_count(result);
  int64 *ids = malloc(sizeof(int64) * (size_t) (*count ? *count : 1));
  for (int i = 0; i < *count; i++)
    ids[i] = *(int64 *) meos_array_get(result, i);
  qsort(ids, (size_t) *count, sizeof(int64), cmp_int64);
  meos_array_destroy(result);
  return ids;
}

/**
 * @brief Compare a loaded tree against the exact answer and against a tree
 * grown one entry at a time, over a spread of selective windows
 * @return Number of properties that do not hold
 */
static int
test_answers(SPTreeKind kind, const char *kindname, TBox **boxes,
  const int64 *ids, const char *shape)
{
  int failures = 0;
  SPTree *grown = sptree_create_tbox(kind);
  SPTree *loaded = sptree_create_tbox(kind);
  TBox *flat = malloc(sizeof(TBox) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    sptree_insert(grown, boxes[i], ids[i]);
    memcpy(&flat[i], boxes[i], sizeof(TBox));
  }
  sptree_load(loaded, flat, ids, NUM_BOXES);

  /* (i), (ii) and the agreement of the two builds */
  int wrong = 0, disagree = 0, total_hits = 0;
  for (int q = 0; q < NUM_QUERIES; q++)
  {
    TBox *query = make_tbox(random_int(-10000, 10000), QUERY_SPAN);
    int nloaded, ngrown;
    int64 *l = search_sorted(loaded, query, &nloaded);
    int64 *g = search_sorted(grown, query, &ngrown);

    /* The exact answer, computed without either tree */
    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    int nwant = 0;
    for (int i = 0; i < NUM_BOXES; i++)
      if (overlaps_tbox_tbox(boxes[i], query))
        want[nwant++] = ids[i];
    qsort(want, (size_t) nwant, sizeof(int64), cmp_int64);

    if (nloaded != nwant ||
        (nwant && memcmp(l, want, sizeof(int64) * (size_t) nwant) != 0))
      wrong++;
    if (nloaded != ngrown ||
        (ngrown && memcmp(l, g, sizeof(int64) * (size_t) ngrown) != 0))
      disagree++;
    total_hits += nwant;
    free(want); free(l); free(g); free(query);
  }
  if (wrong)
  {
    printf("sptree_load: %s %s: %d of %d windows answered something other "
      "than the exact answer\n", shape, kindname, wrong, NUM_QUERIES);
    failures++;
  }
  if (disagree)
  {
    printf("sptree_load: %s %s: %d of %d windows answered differently from "
      "the insert-built tree\n", shape, kindname, disagree, NUM_QUERIES);
    failures++;
  }
  if (total_hits < MIN_EXPECTED_HITS)
  {
    printf("sptree_load: %s %s: the windows matched %d entries, fewer than "
      "the %d the comparison needs to be meaningful\n", shape, kindname,
      total_hits, MIN_EXPECTED_HITS);
    failures++;
  }

  /* (iii) every loaded id comes back exactly once */
  TBox *whole = make_tbox(-20000, 40000);
  int nall;
  int64 *got = search_sorted(loaded, whole, &nall);
  if (nall != NUM_BOXES)
  {
    printf("sptree_load: %s %s: a window over the whole extent returned %d of "
      "%d entries\n", shape, kindname, nall, NUM_BOXES);
    failures++;
  }
  else
  {
    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    memcpy(want, ids, sizeof(int64) * NUM_BOXES);
    qsort(want, NUM_BOXES, sizeof(int64), cmp_int64);
    if (memcmp(got, want, sizeof(int64) * NUM_BOXES) != 0)
    {
      printf("sptree_load: %s %s: the ids returned are not the ids loaded\n",
        shape, kindname);
      failures++;
    }
    free(want);
  }
  /* (iv) the build chooses the depth. An ordered entry set is the one that
   * sends every insertion one level deeper, so a tree grown from it is a chain
   * and a tree built from it is not. Comparing the two heights states the
   * property the build exists for, which the answers alone cannot show. */
  if (strcmp(shape, "ordered") == 0)
  {
    int h_grown = sptree_height(grown), h_loaded = sptree_height(loaded);
    if (h_loaded >= h_grown)
    {
      printf("sptree_load: %s %s: an ordered entry set builds a tree of %d "
        "levels against the %d of the tree grown from it, so the build does "
        "not choose the depth\n", shape, kindname, h_loaded, h_grown);
      failures++;
    }
  }

  free(got); free(whole); free(flat);
  sptree_free(grown);
  sptree_free(loaded);
  return failures;
}

/**
 * @brief Check the entry counts a build has to single out, and that a build
 * leaves the tree holding what it was given and nothing else
 */
static int
test_contract(SPTreeKind kind, const char *kindname, TBox **boxes,
  const int64 *ids)
{
  int failures = 0;
  TBox *flat = malloc(sizeof(TBox) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
    memcpy(&flat[i], boxes[i], sizeof(TBox));
  TBox *whole = make_tbox(-20000, 40000);

  /* (vi) the degenerate counts */
  SPTree *empty = sptree_create_tbox(kind);
  sptree_load(empty, flat, ids, 0);
  int nempty;
  int64 *e = search_sorted(empty, whole, &nempty);
  if (nempty != 0)
  {
    printf("sptree_load: %s: a tree loaded with no entries answered %d\n",
      kindname, nempty);
    failures++;
  }
  free(e);

  SPTree *single = sptree_create_tbox(kind);
  sptree_load(single, flat, ids, 1);
  int nsingle;
  int64 *s = search_sorted(single, whole, &nsingle);
  if (nsingle != 1 || s[0] != ids[0])
  {
    printf("sptree_load: %s: a tree loaded with one entry answered %d\n",
      kindname, nsingle);
    failures++;
  }
  free(s);

  /* (vii) a load leaves the tree holding exactly the entries given. The seeded
   * ids are below every loaded id, so an entry a rebuild fails to drop appears
   * in the answer instead of hiding among the ids that are loaded again. */
  SPTree *rebuilt = sptree_create_tbox(kind);
  for (int i = 0; i < NUM_SEEDED; i++)
    sptree_insert(rebuilt, boxes[i], SEED_ID_BASE + (int64) i);
  sptree_load(rebuilt, flat, ids, NUM_BOXES);
  int nrebuilt;
  int64 *r = search_sorted(rebuilt, whole, &nrebuilt);
  if (nrebuilt != NUM_BOXES)
  {
    printf("sptree_load: %s: a tree holding %d entries answered %d after a "
      "load of %d entries\n", kindname, NUM_SEEDED, nrebuilt, NUM_BOXES);
    failures++;
  }
  else
  {
    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    memcpy(want, ids, sizeof(int64) * NUM_BOXES);
    qsort(want, NUM_BOXES, sizeof(int64), cmp_int64);
    if (memcmp(r, want, sizeof(int64) * NUM_BOXES) != 0)
    {
      printf("sptree_load: %s: a tree loaded over the entries it holds "
        "answers ids it was not loaded with\n", kindname);
      failures++;
    }
    free(want);
  }
  free(r);

  /* The entries are released whatever the number given, so an empty entry set
   * empties the tree rather than leaving the entries it held */
  SPTree *emptied = sptree_create_tbox(kind);
  for (int i = 0; i < NUM_SEEDED; i++)
    sptree_insert(emptied, boxes[i], SEED_ID_BASE + (int64) i);
  sptree_load(emptied, flat, ids, 0);
  int nemptied;
  int64 *m = search_sorted(emptied, whole, &nemptied);
  if (nemptied != 0)
  {
    printf("sptree_load: %s: a tree holding %d entries answered %d after a "
      "load of no entries\n", kindname, NUM_SEEDED, nemptied);
    failures++;
  }
  free(m);

  free(whole); free(flat);
  sptree_free(empty);
  sptree_free(single);
  sptree_free(rebuilt);
  sptree_free(emptied);
  return failures;
}

/**
 * @brief Check a bounding box type other than the one the fixture above is
 * built from, so that the engine is exercised on a second number of dimensions
 * and on the dimensions being read from the first box rather than being fixed
 * when the tree is created
 * @details Exactness against the brute-force answer is asserted, which is what
 * a second family adds; the shapes of the entry set are covered by the
 * temporal box above. The spatial extents are uncorrelated and the time span
 * is shared by every box and every query, so a level narrowing the temporal
 * dimension separates nothing, and the windows are on the scale of the data,
 * so a window covering the extent cannot hide a subtree that is pruned wrongly.
 */
static int
test_stbox(SPTreeKind kind, const char *kindname)
{
  int failures = 0;
  char buf[256];
  STBox *boxes = malloc(sizeof(STBox) * NUM_BOXES);
  int64 *ids = malloc(sizeof(int64) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    int x = random_int(0, 1000), y = random_int(0, 1000);
    snprintf(buf, sizeof(buf),
      "STBOX XT(((%d,%d),(%d,%d)),[2001-01-01,2001-01-02])",
      x, y, x + 20, y + 20);
    STBox *box = stbox_in(buf);
    memcpy(&boxes[i], box, sizeof(STBox));
    free(box);
    ids[i] = (int64) i + ID_BASE;
  }

  SPTree *loaded = sptree_create_stbox(kind);
  sptree_load(loaded, boxes, ids, NUM_BOXES);

  int wrong = 0, total_hits = 0;
  for (int q = 0; q < NUM_QUERIES; q++)
  {
    int x = random_int(0, 1000), y = random_int(0, 1000);
    snprintf(buf, sizeof(buf),
      "STBOX XT(((%d,%d),(%d,%d)),[2001-01-01,2001-01-02])",
      x, y, x + 60, y + 60);
    STBox *query = stbox_in(buf);

    MeosArray *result = meos_array_create(sizeof(int64));
    sptree_search(loaded, INDEX_OVERLAPS, query, result);
    int ngot = meos_array_count(result);
    int64 *got = malloc(sizeof(int64) * (size_t) (ngot ? ngot : 1));
    for (int i = 0; i < ngot; i++)
      got[i] = *(int64 *) meos_array_get(result, i);
    qsort(got, (size_t) ngot, sizeof(int64), cmp_int64);
    meos_array_destroy(result);

    int64 *want = malloc(sizeof(int64) * NUM_BOXES);
    int nwant = 0;
    for (int i = 0; i < NUM_BOXES; i++)
      if (overlaps_stbox_stbox(&boxes[i], query))
        want[nwant++] = ids[i];
    qsort(want, (size_t) nwant, sizeof(int64), cmp_int64);

    if (ngot != nwant ||
        (nwant && memcmp(got, want, sizeof(int64) * (size_t) nwant) != 0))
      wrong++;
    total_hits += nwant;
    free(got); free(want); free(query);
  }
  if (wrong)
  {
    printf("sptree_load: spatiotemporal box %s: %d of %d windows answered "
      "something other than the exact answer\n", kindname, wrong, NUM_QUERIES);
    failures++;
  }
  if (total_hits < MIN_EXPECTED_HITS)
  {
    printf("sptree_load: spatiotemporal box %s: the windows matched %d "
      "entries, fewer than the %d the comparison needs to be meaningful\n",
      kindname, total_hits, MIN_EXPECTED_HITS);
    failures++;
  }

  sptree_free(loaded);
  free(boxes); free(ids);
  return failures;
}

int
main(void)
{
  meos_initialize();
  srand(1);
  int failures = 0;

  const SPTreeKind kinds[2] = {SPTREE_QUADTREE, SPTREE_KDTREE};
  const char *kindnames[2] = {"quad-tree", "k-d tree"};

  /* The entry sets. The values are drawn independently of the ids, so an
   * entry set is not ordered unless it is built to be. */
  TBox **shuffled = malloc(NUM_BOXES * sizeof(TBox *));
  TBox **ordered = malloc(NUM_BOXES * sizeof(TBox *));
  TBox **tied = malloc(NUM_BOXES * sizeof(TBox *));
  int64 *ids = malloc(sizeof(int64) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    shuffled[i] = make_tbox(random_int(-10000, 10000), random_int(1, BOX_SPAN));
    /* (iv) ascending, the set that makes insertion degenerate into a chain */
    ordered[i] = make_tbox(-10000 + i * 9, BOX_SPAN);
    /* (v) every box equal, the set no dimension can separate */
    tied[i] = make_tbox(0, BOX_SPAN);
    ids[i] = (int64) i + ID_BASE;
  }

  for (int k = 0; k < 2; k++)
  {
    failures += test_answers(kinds[k], kindnames[k], shuffled, ids, "shuffled");
    failures += test_answers(kinds[k], kindnames[k], ordered, ids, "ordered");
    failures += test_contract(kinds[k], kindnames[k], shuffled, ids);
    failures += test_stbox(kinds[k], kindnames[k]);

    /* (v) a set that ties throughout builds and answers every entry. The
     * windows of test_answers would all match the whole set, so only the
     * completeness of the answer is asserted here. */
    TBox *flat = malloc(sizeof(TBox) * NUM_BOXES);
    for (int i = 0; i < NUM_BOXES; i++)
      memcpy(&flat[i], tied[i], sizeof(TBox));
    SPTree *chain = sptree_create_tbox(kinds[k]);
    sptree_load(chain, flat, ids, NUM_BOXES);
    TBox *whole = make_tbox(-20000, 40000);
    int nchain;
    int64 *c = search_sorted(chain, whole, &nchain);
    if (nchain != NUM_BOXES)
    {
      printf("sptree_load: %s: a set of %d equal boxes answered %d\n",
        kindnames[k], NUM_BOXES, nchain);
      failures++;
    }
    free(c); free(whole); free(flat);
    sptree_free(chain);
  }

  for (int i = 0; i < NUM_BOXES; i++)
  {
    free(shuffled[i]); free(ordered[i]); free(tied[i]);
  }
  free(shuffled); free(ordered); free(tied); free(ids);

  if (failures == 0)
    printf("sptree_load test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
