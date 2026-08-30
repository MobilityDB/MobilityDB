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
 * @brief A program that tests the join of two in-memory space-partitioning
 * indexes, i.e., sptree_join, against an exact brute-force oracle.
 *
 * The oracle is the public box predicate the join is meant to reproduce --
 * #overlaps_stbox_stbox and #contains_stbox_stbox -- applied to every pair of
 * boxes, so the join is checked against an independent implementation rather
 * than against the callback it uses internally.
 *
 * A space-partitioning index stores a box at every node and not only at its
 * leaves, so an entry pairs with entries lying at any depth of the other index,
 * and the cases are built around the depth an entry reaches:
 *  - the two indexes hold a different number of entries, and one of each pair
 *    is grown entry by entry while the other is built from the whole set, so
 *    the two depths of a pair have nothing to do with each other;
 *  - one case grows an index from a set ordered on one dimension, which
 *    #sptree_insert stores deeper than a build would;
 *  - one case joins a quad-tree with a k-d tree, whose depths and regions are
 *    those of two different partitions of the same space.
 * The fixture keeps the temporal dimension constant across a whole case, so
 * that a level narrowing the wrong dimension separates nothing and the answer
 * rests on the spatial dimensions alone.
 *
 * Four properties are asserted per case:
 *  (i)   soundness: every reported pair satisfies the predicate;
 *  (ii)  completeness: every pair satisfying the predicate is reported;
 *  (iii) no duplicates: no pair is reported twice, so a caller counting the
 *        result counts each pair once;
 *  (iv)  count: the number of reported pairs equals the brute-force count.
 * The degenerate cases of an empty index and of two indexes that share no box,
 * and the operations a join refuses, are asserted separately.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o sptree_join_test sptree_join_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of boxes held by each of the two indexes. The counts differ, so the
 * entries of a pair sit at unrelated depths */
#define NUM_BOXES1 800
#define NUM_BOXES2 350
/* Number of boxes of the index grown from an ordered set, which reaches deeper
 * than a built index of the same size */
#define NUM_BOXES_CHAIN 120
/* Least number of pairs each case must have for the comparison against the
 * oracle to exercise the traversal rather than compare two empty answers */
#define MIN_EXPECTED_PAIRS 200

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

/* Return a random spatiotemporal box placed within @p span of the origin and
 * of spatial extent between @p minext and @p maxext.
 *
 * Every box of a case carries the same period, so that a level narrowing the
 * temporal dimension separates nothing: choosing that dimension over a spatial
 * one is then fatal to the answer rather than harmless. */
static STBox *
random_stbox(double span, double minext, double maxext)
{
  double x = random_double(-span, span), y = random_double(-span, span);
  double dx = random_double(minext, maxext), dy = random_double(minext, maxext);
  char buf[256];
  snprintf(buf, sizeof(buf),
    "STBOX XT(((%.6f,%.6f),(%.6f,%.6f)),"
    "[2001-01-01 00:00:00+00, 2001-01-02 00:00:00+00])", x, y, x + dx, y + dy);
  return stbox_in(buf);
}

/* Comparison function sorting id pairs lexicographically */
static int
cmp_pair(const void *a, const void *b)
{
  const int *pa = (const int *) a, *pb = (const int *) b;
  if (pa[0] != pb[0])
    return (pa[0] > pb[0]) - (pa[0] < pb[0]);
  return (pa[1] > pb[1]) - (pa[1] < pb[1]);
}

/* Return true if the pair of boxes satisfies the operation */
static bool
oracle(const STBox *box1, const STBox *box2, IndexSearchOp op)
{
  switch (op)
  {
    case INDEX_OVERLAPS:
      return overlaps_stbox_stbox(box1, box2);
    case INDEX_CONTAINS:
      return contains_stbox_stbox(box1, box2);
    default: /* INDEX_CONTAINED_BY */
      return contains_stbox_stbox(box2, box1);
  }
}

/*****************************************************************************
 * Join of two SPTree indexes against the brute-force oracle
 *****************************************************************************/

/* How an index is filled: entry by entry, which stores a box at the first
 * empty slot it reaches, or from the whole set at once, which chooses the
 * depth itself */
typedef enum
{
  FILL_GROWN,          /**< Grown by #sptree_insert, in the order drawn */
  FILL_GROWN_ORDERED,  /**< Grown by #sptree_insert, ordered on x */
  FILL_LOADED          /**< Built by #sptree_load from the whole set */
} FillMode;

/* Comparison function ordering boxes on their lower x bound */
static int
cmp_stbox_x(const void *a, const void *b)
{
  double xa, xb;
  stbox_xmin(*(STBox * const *) a, &xa);
  stbox_xmin(*(STBox * const *) b, &xb);
  return (xa > xb) - (xa < xb);
}

/* Fill an index with the given boxes, in the given way */
static void
fill_sptree(SPTree *sptree, STBox **boxes, int count, FillMode mode)
{
  if (mode == FILL_LOADED)
  {
    STBox *flat = malloc((size_t) count * sizeof(STBox));
    int64 *ids = malloc((size_t) count * sizeof(int64));
    for (int i = 0; i < count; i++)
    {
      memcpy(&flat[i], boxes[i], sizeof(STBox));
      ids[i] = i;
    }
    sptree_load(sptree, flat, ids, count);
    free(flat); free(ids);
    return;
  }
  if (mode == FILL_GROWN_ORDERED)
  {
    /* The ids follow the boxes, so that the oracle reads the same pairing */
    STBox **sorted = malloc((size_t) count * sizeof(STBox *));
    memcpy(sorted, boxes, (size_t) count * sizeof(STBox *));
    qsort(sorted, (size_t) count, sizeof(STBox *), cmp_stbox_x);
    for (int i = 0; i < count; i++)
    {
      int id = 0;
      while (boxes[id] != sorted[i])
        id++;
      sptree_insert(sptree, sorted[i], id);
    }
    free(sorted);
    return;
  }
  for (int i = 0; i < count; i++)
    sptree_insert(sptree, boxes[i], i);
  return;
}

static void
test_stbox_join(IndexSearchOp op, const char *opname, SPTreeKind kind1,
  SPTreeKind kind2, FillMode fill1, FillMode fill2, int count1, int count2,
  double minext1, double maxext1, double minext2, double maxext2)
{
  SPTree *sptree1 = sptree_create_stbox(kind1);
  SPTree *sptree2 = sptree_create_stbox(kind2);
  STBox **boxes1 = malloc((size_t) count1 * sizeof(STBox *));
  STBox **boxes2 = malloc((size_t) count2 * sizeof(STBox *));

  /* Each operation is given the extents that make its answer substantial: two
   * comparable sides for overlaps, and one side large enough to hold the other
   * for the containment operations */
  for (int i = 0; i < count1; i++)
    boxes1[i] = random_stbox(200.0, minext1, maxext1);
  for (int j = 0; j < count2; j++)
    boxes2[j] = random_stbox(200.0, minext2, maxext2);
  fill_sptree(sptree1, boxes1, count1, fill1);
  fill_sptree(sptree2, boxes2, count2, fill2);

  /* Brute-force answer */
  int *expected = malloc((size_t) count1 * count2 * 2 * sizeof(int));
  int nexpected = 0;
  for (int i = 0; i < count1; i++)
    for (int j = 0; j < count2; j++)
      if (oracle(boxes1[i], boxes2[j], op))
      {
        expected[2 * nexpected] = i;
        expected[2 * nexpected + 1] = j;
        nexpected++;
      }

  /* Answer of the join */
  MeosArray *result = meos_array_create(sizeof(int64));
  int npairs = sptree_join(sptree1, sptree2, op, result);
  int *found = malloc((size_t) (npairs ? npairs : 1) * 2 * sizeof(int));
  for (int k = 0; k < npairs; k++)
  {
    found[2 * k] = (int) *(int64 *) meos_array_get(result, 2 * k);
    found[2 * k + 1] = (int) *(int64 *) meos_array_get(result, 2 * k + 1);
  }

  char name[128];
  /* A case whose brute-force answer is empty or tiny would let an
   * implementation reporting nothing pass every other property below, so the
   * answer being substantial is itself asserted */
  snprintf(name, sizeof(name), "%s: the expected answer is substantial",
    opname);
  check(name, nexpected >= MIN_EXPECTED_PAIRS);

  snprintf(name, sizeof(name), "%s: pair count matches brute force", opname);
  check(name, npairs == nexpected);

  /* Soundness: every reported pair satisfies the predicate */
  bool sound = true;
  for (int k = 0; k < npairs && sound; k++)
  {
    int i = found[2 * k], j = found[2 * k + 1];
    if (i < 0 || i >= count1 || j < 0 || j >= count2 ||
        ! oracle(boxes1[i], boxes2[j], op))
      sound = false;
  }
  snprintf(name, sizeof(name), "%s: every reported pair satisfies it", opname);
  check(name, sound);

  /* Completeness and absence of duplicates, by comparing the sorted pair
   * lists element by element */
  qsort(found, (size_t) npairs, 2 * sizeof(int), cmp_pair);
  qsort(expected, (size_t) nexpected, 2 * sizeof(int), cmp_pair);
  bool same = (npairs == nexpected);
  for (int k = 0; k < npairs && same; k++)
    if (found[2 * k] != expected[2 * k] ||
        found[2 * k + 1] != expected[2 * k + 1])
      same = false;
  snprintf(name, sizeof(name), "%s: reports exactly the expected pairs",
    opname);
  check(name, same);

  bool distinct = true;
  for (int k = 1; k < npairs && distinct; k++)
    if (found[2 * k] == found[2 * (k - 1)] &&
        found[2 * k + 1] == found[2 * (k - 1) + 1])
      distinct = false;
  snprintf(name, sizeof(name), "%s: reports no pair twice", opname);
  check(name, distinct);

  printf("    (%d pairs over %d x %d boxes, heights %d and %d)\n", npairs,
    count1, count2, sptree_height(sptree1), sptree_height(sptree2));

  meos_array_destroy(result);
  for (int i = 0; i < count1; i++)
    free(boxes1[i]);
  for (int j = 0; j < count2; j++)
    free(boxes2[j]);
  free(boxes1); free(boxes2); free(expected); free(found);
  sptree_free(sptree1); sptree_free(sptree2);
  return;
}

/*****************************************************************************
 * Degenerate cases and the operations a join refuses
 *****************************************************************************/

static void
test_degenerate(void)
{
  MeosArray *result = meos_array_create(sizeof(int64));

  /* An index with no box joins to nothing */
  SPTree *empty1 = sptree_create_stbox(SPTREE_QUADTREE);
  SPTree *empty2 = sptree_create_stbox(SPTREE_KDTREE);
  check("empty joined with empty reports no pair",
    sptree_join(empty1, empty2, INDEX_OVERLAPS, result) == 0);

  SPTree *filled = sptree_create_stbox(SPTREE_QUADTREE);
  STBox *box = stbox_in("STBOX XT(((0,0),(1,1)),[2001-01-01, 2001-01-02])");
  sptree_insert(filled, box, 0);
  check("empty joined with a filled index reports no pair",
    sptree_join(empty1, filled, INDEX_OVERLAPS, result) == 0);
  check("filled index joined with empty reports no pair",
    sptree_join(filled, empty1, INDEX_OVERLAPS, result) == 0);

  /* Two indexes whose boxes are far apart report no pair, and the same two
   * report their single pair once the boxes are made to overlap */
  SPTree *far = sptree_create_stbox(SPTREE_KDTREE);
  STBox *farbox = stbox_in(
    "STBOX XT(((1000,1000),(1001,1001)),[2001-01-01, 2001-01-02])");
  sptree_insert(far, farbox, 0);
  check("indexes that share no box report no pair",
    sptree_join(filled, far, INDEX_OVERLAPS, result) == 0);

  SPTree *near = sptree_create_stbox(SPTREE_KDTREE);
  STBox *nearbox = stbox_in(
    "STBOX XT(((0.5,0.5),(2,2)),[2001-01-01, 2001-01-02])");
  sptree_insert(near, nearbox, 7);
  int n = sptree_join(filled, near, INDEX_OVERLAPS, result);
  check("a single overlapping pair is reported once", n == 1);
  check("the reported ids are the inserted ones", n == 1 &&
    *(int64 *) meos_array_get(result, 0) == 0 &&
    *(int64 *) meos_array_get(result, 1) == 7);

  /* Boxes overlapping in space but not in time are not reported, since an
   * STBox pair must overlap in every dimension the two boxes share */
  SPTree *later = sptree_create_stbox(SPTREE_QUADTREE);
  STBox *laterbox = stbox_in(
    "STBOX XT(((0,0),(1,1)),[2002-01-01, 2002-01-02])");
  sptree_insert(later, laterbox, 0);
  check("boxes apart in time only are not reported",
    sptree_join(filled, later, INDEX_OVERLAPS, result) == 0);

  meos_array_destroy(result);
  free(box); free(farbox); free(nearbox); free(laterbox);
  sptree_free(empty1); sptree_free(empty2); sptree_free(filled);
  sptree_free(far); sptree_free(near); sptree_free(later);
  return;
}

/* What a join refuses. The error handler installed here reports through
 * #meos_errno instead of ending the program, so that the refusal is read as a
 * value rather than observed as an exit */
static void
test_refused(void)
{
  meos_initialize_noexit_error_handler();
  MeosArray *result = meos_array_create(sizeof(int64));

  SPTree *stboxtree = sptree_create_stbox(SPTREE_QUADTREE);
  SPTree *other = sptree_create_stbox(SPTREE_KDTREE);
  SPTree *tboxtree = sptree_create_tbox(SPTREE_QUADTREE);
  STBox *box = stbox_in("STBOX XT(((0,0),(1,1)),[2001-01-01, 2001-01-02])");
  sptree_insert(stboxtree, box, 0);
  sptree_insert(other, box, 1);

  meos_errno_reset();
  check("indexes of different bounding box types are refused",
    sptree_join(stboxtree, tboxtree, INDEX_OVERLAPS, result) == INT_MAX &&
    meos_errno() != 0);

  /* A join prunes a pair of subtrees on the overlap of what they cover, which
   * is exactly what the entries an ordering operation pairs do not do */
  meos_errno_reset();
  check("an operation ordering a dimension is refused",
    sptree_join(stboxtree, other, INDEX_LEFT, result) == INT_MAX &&
    meos_errno() != 0);

  meos_errno_reset();
  check("a null index is refused",
    sptree_join(NULL, other, INDEX_OVERLAPS, result) == INT_MAX &&
    meos_errno() != 0);

  /* The R-tree answers the same operations, so it refuses the same ones */
  RTree *rtree1 = rtree_create_stbox();
  RTree *rtree2 = rtree_create_stbox();
  rtree_insert(rtree1, box, 0);
  rtree_insert(rtree2, box, 1);
  meos_errno_reset();
  check("the R-tree refuses an operation ordering a dimension",
    rtree_join(rtree1, rtree2, INDEX_LEFT, result) == INT_MAX &&
    meos_errno() != 0);
  meos_errno_reset();
  check("the R-tree refuses a null index",
    rtree_join(rtree1, NULL, INDEX_OVERLAPS, result) == INT_MAX &&
    meos_errno() != 0);
  meos_errno_reset();

  meos_array_destroy(result);
  free(box);
  sptree_free(stboxtree); sptree_free(other); sptree_free(tboxtree);
  rtree_free(rtree1); rtree_free(rtree2);
  return;
}

int
main(void)
{
  meos_initialize();
  /* A fixed seed keeps the test deterministic across runs */
  srand(1);

  printf("Testing sptree_join against a brute-force oracle\n");
  /* Comparable extents on both sides for overlaps; for the containment
   * operations the side that must hold the other is given the larger extent */
  test_stbox_join(INDEX_OVERLAPS, "overlaps, quad-tree and quad-tree",
    SPTREE_QUADTREE, SPTREE_QUADTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES1, NUM_BOXES2, 1.0, 150.0, 1.0, 150.0);
  test_stbox_join(INDEX_CONTAINS, "contains, quad-tree and quad-tree",
    SPTREE_QUADTREE, SPTREE_QUADTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES1, NUM_BOXES2, 60.0, 150.0, 1.0, 8.0);
  test_stbox_join(INDEX_CONTAINED_BY, "contained by, quad-tree and quad-tree",
    SPTREE_QUADTREE, SPTREE_QUADTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES1, NUM_BOXES2, 1.0, 8.0, 60.0, 150.0);

  /* The k-d tree partitions one dimension per level, so it reaches depths the
   * quad-tree does not, and a level narrowing the wrong dimension prunes the
   * subtrees holding the matches */
  test_stbox_join(INDEX_OVERLAPS, "overlaps, k-d tree and k-d tree",
    SPTREE_KDTREE, SPTREE_KDTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES1, NUM_BOXES2, 1.0, 150.0, 1.0, 150.0);
  test_stbox_join(INDEX_CONTAINS, "contains, k-d tree and k-d tree",
    SPTREE_KDTREE, SPTREE_KDTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES1, NUM_BOXES2, 60.0, 150.0, 1.0, 8.0);

  /* The two indexes need not be of the same kind: each descends its own
   * partition of the space */
  test_stbox_join(INDEX_OVERLAPS, "overlaps, quad-tree and k-d tree",
    SPTREE_QUADTREE, SPTREE_KDTREE, FILL_LOADED, FILL_GROWN,
    NUM_BOXES1, NUM_BOXES2, 1.0, 150.0, 1.0, 150.0);
  test_stbox_join(INDEX_OVERLAPS, "overlaps, k-d tree and quad-tree",
    SPTREE_KDTREE, SPTREE_QUADTREE, FILL_GROWN, FILL_LOADED,
    NUM_BOXES2, NUM_BOXES1, 1.0, 150.0, 1.0, 150.0);

  /* An index grown from a set ordered on one dimension reaches deeper than the
   * index built from the same number of entries, so the two entries of a pair
   * sit at depths that have nothing to do with each other */
  test_stbox_join(INDEX_OVERLAPS, "overlaps, a chain and a built index",
    SPTREE_QUADTREE, SPTREE_QUADTREE, FILL_GROWN_ORDERED, FILL_LOADED,
    NUM_BOXES_CHAIN, NUM_BOXES1, 1.0, 150.0, 1.0, 150.0);
  test_stbox_join(INDEX_OVERLAPS, "overlaps, a built index and a chain",
    SPTREE_KDTREE, SPTREE_KDTREE, FILL_LOADED, FILL_GROWN_ORDERED,
    NUM_BOXES1, NUM_BOXES_CHAIN, 1.0, 150.0, 1.0, 150.0);

  test_degenerate();
  test_refused();

  meos_finalize();
  if (failures > 0)
  {
    printf("\n%d test(s) FAILED\n", failures);
    return 1;
  }
  printf("\nAll tests passed\n");
  return 0;
}
