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
 * @brief A program that tests the position search operations of the in-memory
 * indexes, i.e., the operations that order a dimension, against the exact
 * answer and across both index structures, together with the equality
 * operation, which orders no dimension but is pruned the way containment is.
 *
 * An index answers a position operation by two different tests: an entry is
 * accepted by the operator of the box type, and a subtree is descended when it
 * can still hold such an entry. The second test is the one that can go wrong
 * silently, because pruning a subtree that holds matches removes them from the
 * answer without any other symptom.
 *
 * Every configuration is therefore compared against a brute-force answer
 * computed without any index, and the three structures — the R-tree, the
 * quad-tree and the k-d tree — are required to agree with it and with each
 * other. Two properties are asserted per operation:
 *  (i)   exactness: the index returns every box satisfying the operation and
 *        no other;
 *  (ii)  the comparison is not vacuous: over the query set the operation must
 *        match a substantial number of entries, since an operation that
 *        matches nothing is satisfied by an index that answers nothing.
 *
 * The fixture spreads the entries over both spatial dimensions independently
 * and holds the time span of every box and every query equal, so that an index
 * ordering the wrong dimension separates nothing and is caught; and the query
 * boxes sit inside the extent of the data, so that a query beyond it does not
 * make an operation trivially true or trivially false.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o index_position_test index_position_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

#define NUM_BOXES 1024
#define NUM_QUERIES 40
/* Fewest hits an operation must accumulate over the query set to count */
#define MIN_HITS 200

typedef struct
{
  IndexSearchOp op;
  const char *name;
  bool (*oracle)(const STBox *, const STBox *);
} OpCase;

static bool o_left(const STBox *a, const STBox *b) { return left_stbox_stbox(a, b); }
static bool o_overleft(const STBox *a, const STBox *b) { return overleft_stbox_stbox(a, b); }
static bool o_right(const STBox *a, const STBox *b) { return right_stbox_stbox(a, b); }
static bool o_overright(const STBox *a, const STBox *b) { return overright_stbox_stbox(a, b); }
static bool o_below(const STBox *a, const STBox *b) { return below_stbox_stbox(a, b); }
static bool o_overbelow(const STBox *a, const STBox *b) { return overbelow_stbox_stbox(a, b); }
static bool o_above(const STBox *a, const STBox *b) { return above_stbox_stbox(a, b); }
static bool o_overabove(const STBox *a, const STBox *b) { return overabove_stbox_stbox(a, b); }

static const OpCase OPS[] = {
  { INDEX_LEFT,      "left",      o_left },
  { INDEX_OVERLEFT,  "overleft",  o_overleft },
  { INDEX_RIGHT,     "right",     o_right },
  { INDEX_OVERRIGHT, "overright", o_overright },
  { INDEX_BELOW,     "below",     o_below },
  { INDEX_OVERBELOW, "overbelow", o_overbelow },
  { INDEX_ABOVE,     "above",     o_above },
  { INDEX_OVERABOVE, "overabove", o_overabove },
};
#define NUM_OPS (int) (sizeof(OPS) / sizeof(OPS[0]))

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

static int64 *
sorted_of(MeosArray *result, int *count)
{
  *count = meos_array_count(result);
  int64 *ids = malloc(sizeof(int64) * (size_t) (*count ? *count : 1));
  for (int i = 0; i < *count; i++)
    ids[i] = *(int64 *) meos_array_get(result, i);
  qsort(ids, (size_t) *count, sizeof(int64), cmp_int64);
  return ids;
}

int
main(void)
{
  meos_initialize();
  srand(1);
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
    STBox *b = stbox_in(buf);
    memcpy(&boxes[i], b, sizeof(STBox));
    free(b);
    ids[i] = (int64) i;
  }

  /* The three structures the operations must be answered by */
  RTree *rt = rtree_create_stbox();
  SPTree *quad = sptree_create_stbox(SPTREE_QUADTREE);
  SPTree *kd = sptree_create_stbox(SPTREE_KDTREE);
  for (int i = 0; i < NUM_BOXES; i++)
  {
    rtree_insert(rt, &boxes[i], ids[i]);
    sptree_insert(quad, &boxes[i], ids[i]);
    sptree_insert(kd, &boxes[i], ids[i]);
  }

  for (int o = 0; o < NUM_OPS; o++)
  {
    int wrong_rt = 0, wrong_quad = 0, wrong_kd = 0, hits = 0;
    for (int q = 0; q < NUM_QUERIES; q++)
    {
      int x = random_int(100, 900), y = random_int(100, 900);
      snprintf(buf, sizeof(buf),
        "STBOX XT(((%d,%d),(%d,%d)),[2001-01-01,2001-01-02])",
        x, y, x + 40, y + 40);
      STBox *query = stbox_in(buf);

      /* The exact answer, computed without any index */
      int64 *want = malloc(sizeof(int64) * NUM_BOXES);
      int nwant = 0;
      for (int i = 0; i < NUM_BOXES; i++)
        if (OPS[o].oracle(&boxes[i], query))
          want[nwant++] = ids[i];
      qsort(want, (size_t) nwant, sizeof(int64), cmp_int64);
      hits += nwant;

      MeosArray *r = meos_array_create(sizeof(int64));
      rtree_search(rt, OPS[o].op, query, r);
      int n; int64 *got = sorted_of(r, &n);
      if (n != nwant || (nwant && memcmp(got, want, sizeof(int64) * (size_t) nwant)))
        wrong_rt++;
      free(got); meos_array_destroy(r);

      r = meos_array_create(sizeof(int64));
      sptree_search(quad, OPS[o].op, query, r);
      got = sorted_of(r, &n);
      if (n != nwant || (nwant && memcmp(got, want, sizeof(int64) * (size_t) nwant)))
        wrong_quad++;
      free(got); meos_array_destroy(r);

      r = meos_array_create(sizeof(int64));
      sptree_search(kd, OPS[o].op, query, r);
      got = sorted_of(r, &n);
      if (n != nwant || (nwant && memcmp(got, want, sizeof(int64) * (size_t) nwant)))
        wrong_kd++;
      free(got); meos_array_destroy(r);

      free(want); free(query);
    }
    if (wrong_rt || wrong_quad || wrong_kd)
    {
      printf("index position: %-10s answered something other than the exact "
        "answer on  R-tree %d/%d  quad-tree %d/%d  k-d tree %d/%d\n",
        OPS[o].name, wrong_rt, NUM_QUERIES, wrong_quad, NUM_QUERIES,
        wrong_kd, NUM_QUERIES);
      failures++;
    }
    if (hits < MIN_HITS)
    {
      printf("index position: %-10s matched %d entries over the query set, "
        "fewer than the %d the comparison needs to be meaningful\n",
        OPS[o].name, hits, MIN_HITS);
      failures++;
    }
  }

  /* The equality operation, whose query must be an entry to match anything.
   * It is answered by the same descent as containment -- a subtree holds a box
   * equal to the query only when the region bounding it contains the query --
   * so the pruning is the part worth asserting, and the entries themselves are
   * the query set that makes the comparison non-vacuous. */
  {
    int checked = 0, hits = 0;
    for (int q = 0; q < NUM_QUERIES; q++)
    {
      const STBox *query = &boxes[(q * 37) % NUM_BOXES];
      int want = 0;
      for (int i = 0; i < NUM_BOXES; i++)
        if (same_stbox_stbox(&boxes[i], query))
          want++;

      MeosArray *got_rt = meos_array_create(sizeof(int64));
      MeosArray *got_quad = meos_array_create(sizeof(int64));
      MeosArray *got_kd = meos_array_create(sizeof(int64));
      int n_rt = rtree_search(rt, INDEX_SAME, query, got_rt);
      int n_quad = sptree_search(quad, INDEX_SAME, query, got_quad);
      int n_kd = sptree_search(kd, INDEX_SAME, query, got_kd);
      if (n_rt != want || n_quad != want || n_kd != want)
      {
        printf("index same: query %d expected %d, R-tree %d, quad-tree %d, "
          "k-d tree %d\n", q, want, n_rt, n_quad, n_kd);
        failures++;
      }
      hits += want;
      checked++;
      meos_array_destroy(got_rt);
      meos_array_destroy(got_quad);
      meos_array_destroy(got_kd);
    }
    if (hits < checked)
    {
      printf("index same: matched %d entries over %d queries, fewer than the "
        "one per query the comparison needs to be meaningful\n", hits, checked);
      failures++;
    }
  }

  /* Adjacency, on the entry set that separates it from overlap. Two spans
   * meeting at an excluded bound share a boundary and no point, so they are
   * adjacent and do NOT overlap, and a descent pruning on overlap alone drops
   * exactly them -- at the node boundaries, where nothing else in the answer
   * is missing. The fixture is therefore a chain of half-open spans rather
   * than the boxes above, and the join is included, a join pruning a pair of
   * subtrees by the same rule. */
  {
    const int NADJ = 512;
    Span *chain = malloc(sizeof(Span) * NADJ);
    int64 *chain_ids = malloc(sizeof(int64) * NADJ);
    char buf[64];
    for (int i = 0; i < NADJ; i++)
    {
      snprintf(buf, sizeof(buf), "[%d,%d)", i * 10, (i + 1) * 10);
      Span *sp = intspan_in(buf);
      chain[i] = *sp;
      free(sp);
      chain_ids[i] = i;
    }
    const Span *query = &chain[NADJ / 2];
    int want = 0;
    for (int i = 0; i < NADJ; i++)
      if (adjacent_span_span(&chain[i], query))
        want++;
    if (want < 2)
    {
      printf("index adjacent: the query meets %d entries, too few for the "
        "comparison to be meaningful\n", want);
      failures++;
    }

    RTree *art = rtree_create_intspan();
    SPTree *aquad = sptree_create_intspan(SPTREE_QUADTREE);
    SPTree *akd = sptree_create_intspan(SPTREE_KDTREE);
    for (int i = 0; i < NADJ; i++)
    {
      rtree_insert(art, &chain[i], chain_ids[i]);
      sptree_insert(aquad, &chain[i], chain_ids[i]);
      sptree_insert(akd, &chain[i], chain_ids[i]);
    }
    MeosArray *g1 = meos_array_create(sizeof(int64));
    MeosArray *g2 = meos_array_create(sizeof(int64));
    MeosArray *g3 = meos_array_create(sizeof(int64));
    int n1 = rtree_search(art, INDEX_ADJACENT, query, g1);
    int n2 = sptree_search(aquad, INDEX_ADJACENT, query, g2);
    int n3 = sptree_search(akd, INDEX_ADJACENT, query, g3);
    if (n1 != want || n2 != want || n3 != want)
    {
      printf("index adjacent: expected %d, R-tree %d, quad-tree %d, "
        "k-d tree %d\n", want, n1, n2, n3);
      failures++;
    }

    int want_pairs = 0;
    for (int i = 0; i < NADJ; i++)
      for (int j = 0; j < NADJ; j++)
        if (adjacent_span_span(&chain[i], &chain[j]))
          want_pairs++;
    RTree *art2 = rtree_create_intspan();
    for (int i = 0; i < NADJ; i++)
      rtree_insert(art2, &chain[i], chain_ids[i]);
    MeosArray *pairs = meos_array_create(sizeof(int64));
    rtree_join(art, art2, INDEX_ADJACENT, pairs);
    int got_pairs = meos_array_count(pairs) / 2;
    if (got_pairs != want_pairs)
    {
      printf("index adjacent join: expected %d pairs, got %d\n", want_pairs,
        got_pairs);
      failures++;
    }

    meos_array_destroy(g1); meos_array_destroy(g2); meos_array_destroy(g3);
    meos_array_destroy(pairs);
    rtree_free(art); rtree_free(art2);
    sptree_free(aquad); sptree_free(akd);
    free(chain); free(chain_ids);
  }

  rtree_free(rt);
  sptree_free(quad);
  sptree_free(kd);
  free(boxes); free(ids);

  if (failures == 0)
    printf("index position test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
