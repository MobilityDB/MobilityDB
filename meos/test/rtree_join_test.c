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
 * @brief A program that tests the join of two in-memory RTree indexes, i.e.,
 * rtree_join, against an exact brute-force oracle.
 *
 * The oracle is the public box predicate the join is meant to reproduce --
 * #overlaps_stbox_stbox and #contains_stbox_stbox -- applied to every pair of
 * boxes, so the join is checked against an independent implementation rather
 * than against the callback it uses internally.
 *
 * The two indexes hold a different number of boxes, so the traversal descends
 * one side while the other is still a leaf, and the boxes carry both space and
 * time, so a pair is reported only when it overlaps in every dimension.
 *
 * Four properties are asserted per operation:
 *  (i)   soundness: every reported pair satisfies the predicate;
 *  (ii)  completeness: every pair satisfying the predicate is reported;
 *  (iii) no duplicates: no pair is reported twice, so a caller counting the
 *        result counts each pair once;
 *  (iv)  count: the number of reported pairs equals the brute-force count.
 * The degenerate cases of an empty index and of two indexes that share no box
 * are asserted separately.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_join_test rtree_join_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of boxes inserted into each of the two indexes. Both counts exceed
 * the node capacity, so the two trees have inner nodes and descend together */
#define NUM_BOXES1 1500
#define NUM_BOXES2 700
/* A count within the node capacity, so that the index is a single leaf and a
 * join against a taller tree descends one side at a time */
#define NUM_BOXES_LEAF 40
/* Least number of pairs each operation must have for the comparison against
 * the oracle to exercise the traversal rather than compare two empty answers */
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

/* Return a random spatiotemporal box placed within @p span of the origin, of
 * spatial extent between @p minext and @p maxext, and spanning at most
 * @p hours of a day drawn from the first @p days of January 2000.
 *
 * Both indexes draw their extents from the same wide range, so that one entry
 * contains another often enough for the containment operations to have a
 * substantial answer rather than an empty one. */
static STBox *
random_stbox(double span, double minext, double maxext, int days, int hours)
{
  double x = random_double(-span, span), y = random_double(-span, span);
  double dx = random_double(minext, maxext), dy = random_double(minext, maxext);
  /* The period is kept inside one day so that the bounds stay valid without
   * date arithmetic; the day itself varies over the leading days of the month */
  int day = 1 + rand() % days;
  int h1 = rand() % (24 - hours);
  int h2 = h1 + rand() % (hours + 1);
  char buf[256];
  snprintf(buf, sizeof(buf),
    "STBOX XT(((%.6f,%.6f),(%.6f,%.6f)),"
    "[2000-01-%02d %02d:00:00+00, 2000-01-%02d %02d:59:59+00])",
    x, y, x + dx, y + dy, day, h1, day, h2);
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
 * Join of two STBox indexes against the brute-force oracle
 *****************************************************************************/

static void
test_stbox_join(IndexSearchOp op, const char *opname, int count1, int count2,
  double minext1, double maxext1, int hours1, double minext2, double maxext2,
  int hours2)
{
  RTree *rtree1 = rtree_create_stbox();
  RTree *rtree2 = rtree_create_stbox();
  STBox **boxes1 = malloc((size_t) count1 * sizeof(STBox *));
  STBox **boxes2 = malloc((size_t) count2 * sizeof(STBox *));

  /* Each operation is given the extents that make its answer substantial: two
   * comparable sides for overlaps, and one side large enough to hold the other
   * in space and in time for the containment operations */
  for (int i = 0; i < count1; i++)
  {
    boxes1[i] = random_stbox(200.0, minext1, maxext1, 4, hours1);
    rtree_insert(rtree1, boxes1[i], i);
  }
  for (int j = 0; j < count2; j++)
  {
    boxes2[j] = random_stbox(200.0, minext2, maxext2, 4, hours2);
    rtree_insert(rtree2, boxes2[j], j);
  }

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
  int npairs = rtree_join(rtree1, rtree2, op, result);
  int *found = malloc((size_t) (npairs ? npairs : 1) * 2 * sizeof(int));
  for (int k = 0; k < npairs; k++)
  {
    found[2 * k] = *(int64 *) meos_array_get(result, 2 * k);
    found[2 * k + 1] = *(int64 *) meos_array_get(result, 2 * k + 1);
  }

  char name[128];
  /* An operation whose brute-force answer is empty or tiny would let an
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

  printf("    (%d pairs over %d x %d boxes)\n", npairs, count1, count2);

  meos_array_destroy(result);
  for (int i = 0; i < count1; i++)
    free(boxes1[i]);
  for (int j = 0; j < count2; j++)
    free(boxes2[j]);
  free(boxes1); free(boxes2); free(expected); free(found);
  rtree_free(rtree1); rtree_free(rtree2);
  return;
}

/*****************************************************************************
 * Degenerate cases
 *****************************************************************************/

static void
test_degenerate(void)
{
  MeosArray *result = meos_array_create(sizeof(int64));

  /* An index with no box joins to nothing */
  RTree *empty1 = rtree_create_stbox();
  RTree *empty2 = rtree_create_stbox();
  check("empty joined with empty reports no pair",
    rtree_join(empty1, empty2, INDEX_OVERLAPS, result) == 0);

  RTree *filled = rtree_create_stbox();
  STBox *box = stbox_in("STBOX XT(((0,0),(1,1)),[2001-01-01, 2001-01-02])");
  rtree_insert(filled, box, 0);
  check("empty joined with a filled index reports no pair",
    rtree_join(empty1, filled, INDEX_OVERLAPS, result) == 0);
  check("filled index joined with empty reports no pair",
    rtree_join(filled, empty1, INDEX_OVERLAPS, result) == 0);

  /* Two indexes whose boxes are far apart report no pair, and the same two
   * report their single pair once the boxes are made to overlap */
  RTree *far = rtree_create_stbox();
  STBox *farbox = stbox_in(
    "STBOX XT(((1000,1000),(1001,1001)),[2001-01-01, 2001-01-02])");
  rtree_insert(far, farbox, 0);
  check("indexes that share no box report no pair",
    rtree_join(filled, far, INDEX_OVERLAPS, result) == 0);

  RTree *near = rtree_create_stbox();
  STBox *nearbox = stbox_in(
    "STBOX XT(((0.5,0.5),(2,2)),[2001-01-01, 2001-01-02])");
  rtree_insert(near, nearbox, 7);
  int n = rtree_join(filled, near, INDEX_OVERLAPS, result);
  check("a single overlapping pair is reported once", n == 1);
  check("the reported ids are the inserted ones", n == 1 &&
    *(int64 *) meos_array_get(result, 0) == 0 &&
    *(int64 *) meos_array_get(result, 1) == 7);

  /* Boxes overlapping in space but not in time are not reported, since an
   * STBox pair must overlap in every dimension the two boxes share */
  RTree *later = rtree_create_stbox();
  STBox *laterbox = stbox_in(
    "STBOX XT(((0,0),(1,1)),[2002-01-01, 2002-01-02])");
  rtree_insert(later, laterbox, 0);
  check("boxes apart in time only are not reported",
    rtree_join(filled, later, INDEX_OVERLAPS, result) == 0);

  meos_array_destroy(result);
  free(box); free(farbox); free(nearbox); free(laterbox);
  rtree_free(empty1); rtree_free(empty2); rtree_free(filled);
  rtree_free(far); rtree_free(near); rtree_free(later);
  return;
}

int
main(void)
{
  meos_initialize();
  /* A fixed seed keeps the test deterministic across runs */
  srand(1);

  printf("Testing rtree_join against a brute-force oracle\n");
  /* Comparable extents on both sides for overlaps; for the containment
   * operations the side that must hold the other is given the larger spatial
   * extent and the longer period */
  test_stbox_join(INDEX_OVERLAPS, "overlaps", NUM_BOXES1, NUM_BOXES2,
    1.0, 150.0, 8, 1.0, 150.0, 8);
  test_stbox_join(INDEX_CONTAINS, "contains", NUM_BOXES1, NUM_BOXES2,
    60.0, 150.0, 12, 1.0, 8.0, 1);
  test_stbox_join(INDEX_CONTAINED_BY, "contained by", NUM_BOXES1, NUM_BOXES2,
    1.0, 8.0, 1, 60.0, 150.0, 12);

  /* Trees of equal height descend in step and never meet a leaf on one side
   * while the other still has inner nodes. A side holding at most MAXITEMS
   * boxes is a lone leaf, so pairing it with a taller tree reaches the two
   * branches that descend a single side, in both directions. */
  test_stbox_join(INDEX_OVERLAPS, "overlaps, taller first", NUM_BOXES1,
    NUM_BOXES_LEAF, 1.0, 150.0, 8, 1.0, 150.0, 8);
  test_stbox_join(INDEX_OVERLAPS, "overlaps, taller second", NUM_BOXES_LEAF,
    NUM_BOXES1, 1.0, 150.0, 8, 1.0, 150.0, 8);
  test_degenerate();

  meos_finalize();
  if (failures > 0)
  {
    printf("\n%d test(s) FAILED\n", failures);
    return 1;
  }
  printf("\nAll tests passed\n");
  return 0;
}
