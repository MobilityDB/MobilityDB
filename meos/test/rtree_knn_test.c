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
 * @brief A program that tests the nearest-neighbour cursor of the in-memory
 * RTree index, i.e., rtree_nn_cursor_open, rtree_nn_cursor_next and
 * rtree_nn_cursor_close, against an exact brute-force oracle.
 *
 * Two box types are exercised: a spatial STBox index over random 2-D points
 * and a TBox index over random numeric values. Since the reported distance is
 * the box-to-box nearest approach distance, spatial-only boxes make the STBox
 * distance a pure Euclidean distance and single-value boxes sharing one time
 * make the TBox distance a pure numeric distance, so the brute-force oracle is
 * a direct scan.
 *
 * Five properties are asserted per box type:
 *  (i)   completeness: a full drain yields every inserted id exactly once;
 *  (ii)  monotonicity: the reported distances are non-decreasing;
 *  (iii) correct distance: each reported distance equals the box distance
 *        between the query and that id's box;
 *  (iv)  correct order: the drained distance sequence equals the sorted
 *        brute-force distances element by element (tie-robust);
 *  (v)   early stop: taking only the first k results matches the first k of
 *        the full drain, so a LIMIT k caller reads the true k neighbours.
 * What the cursor refuses to open on is asserted separately.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o rtree_knn_test rtree_knn_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <meos.h>
#include <meos_geo.h>

/* Number of boxes inserted into the index */
#define NUM_BOXES 2048
/* Number of nearest neighbours checked by the early-stop property */
#define K 20
/* Tolerance for floating-point distance comparisons */
#define EPS 1e-9

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

/* Comparison function sorting doubles in ascending order */
static int
cmp_double(const void *a, const void *b)
{
  double da = *(const double *) a, db = *(const double *) b;
  return (da > db) - (da < db);
}

/*****************************************************************************
 * Spatial STBox nearest-neighbour test over random 2-D points
 *****************************************************************************/

static void
test_stbox_knn(void)
{
  RTree *rtree = rtree_create_stbox();
  STBox **boxes = malloc(NUM_BOXES * sizeof(STBox *));

  /* Insert spatial-only boxes built from random 2-D points. A spatial-only
   * box makes the nearest approach distance a pure Euclidean distance, with
   * no time dimension to gate it. */
  for (int i = 0; i < NUM_BOXES; i++)
  {
    GSERIALIZED *gs = geompoint_make2d(0, random_double(-1000, 1000),
      random_double(-1000, 1000));
    boxes[i] = geo_to_stbox(gs);
    rtree_insert(rtree, boxes[i], i);
    free(gs);
  }

  /* Build the query box from another random point */
  GSERIALIZED *qgs = geompoint_make2d(0, random_double(-1000, 1000),
    random_double(-1000, 1000));
  STBox *query = geo_to_stbox(qgs);
  free(qgs);

  /* Brute-force oracle: the distance from the query to every inserted box */
  double *brute = malloc(NUM_BOXES * sizeof(double));
  for (int i = 0; i < NUM_BOXES; i++)
    brute[i] = nad_stbox_stbox(query, boxes[i]);
  double *sorted = malloc(NUM_BOXES * sizeof(double));
  memcpy(sorted, brute, NUM_BOXES * sizeof(double));
  qsort(sorted, NUM_BOXES, sizeof(double), cmp_double);

  /* Full drain of the cursor */
  int *seen = calloc(NUM_BOXES, sizeof(int));
  double *cur_dist = malloc(NUM_BOXES * sizeof(double));
  int64 *cur_id = malloc(NUM_BOXES * sizeof(int64));
  RTreeNNCursor *cursor = rtree_nn_cursor_open(rtree, query);
  int n = 0;
  int64 id;
  double dist;
  while (rtree_nn_cursor_next(cursor, &id, &dist))
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
  rtree_nn_cursor_close(cursor);

  printf("STBox NN cursor (%d random 2-D points):\n", NUM_BOXES);

  /* (i) Completeness: every id appears exactly once */
  bool complete = (n == NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
    if (seen[i] != 1)
      complete = false;
  check("(i) full drain returns every id exactly once", complete);

  /* (ii) Monotonicity: distances are non-decreasing */
  bool monotone = true;
  for (int i = 1; i < n; i++)
    if (cur_dist[i] < cur_dist[i - 1] - EPS)
      monotone = false;
  check("(ii) reported distances are non-decreasing", monotone);

  /* (iii) Correct distance: each reported distance matches the box distance */
  bool dist_ok = true;
  for (int i = 0; i < n; i++)
    if (fabs(cur_dist[i] - brute[cur_id[i]]) > EPS)
      dist_ok = false;
  check("(iii) reported distance equals the box distance", dist_ok);

  /* (iv) Correct order: drained distances equal the sorted brute-force ones */
  bool order_ok = (n == NUM_BOXES);
  for (int i = 0; i < n && order_ok; i++)
    if (fabs(cur_dist[i] - sorted[i]) > EPS)
      order_ok = false;
  check("(iv) drained order matches the sorted brute-force distances",
    order_ok);

  /* (v) Early stop: the first K results match the first K of the full drain */
  bool early_ok = true;
  RTreeNNCursor *kcursor = rtree_nn_cursor_open(rtree, query);
  for (int i = 0; i < K && early_ok; i++)
  {
    int64 kid;
    double kdist;
    if (! rtree_nn_cursor_next(kcursor, &kid, &kdist) ||
        kid != cur_id[i] || fabs(kdist - cur_dist[i]) > EPS)
      early_ok = false;
  }
  rtree_nn_cursor_close(kcursor);
  check("(v) first K results match the full-drain prefix", early_ok);

  /* Cleanup */
  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(query); free(brute); free(sorted);
  free(seen); free(cur_dist); free(cur_id);
  rtree_free(rtree);
}

/*****************************************************************************
 * Numeric TBox nearest-neighbour test over random values
 *****************************************************************************/

static void
test_tbox_knn(void)
{
  RTree *rtree = rtree_create_tbox();
  TBox **boxes = malloc(NUM_BOXES * sizeof(TBox *));
  double *val = malloc(NUM_BOXES * sizeof(double));
  /* A single shared instant makes every box overlap in time, so the nearest
   * approach distance reduces to the numeric distance between the values */
  TimestampTz t = 0;

  for (int i = 0; i < NUM_BOXES; i++)
  {
    val[i] = random_double(-1000, 1000);
    boxes[i] = float_timestamptz_to_tbox(val[i], t);
    rtree_insert(rtree, boxes[i], i);
  }

  double qval = random_double(-1000, 1000);
  TBox *query = float_timestamptz_to_tbox(qval, t);

  double *brute = malloc(NUM_BOXES * sizeof(double));
  for (int i = 0; i < NUM_BOXES; i++)
    brute[i] = fabs(qval - val[i]);
  double *sorted = malloc(NUM_BOXES * sizeof(double));
  memcpy(sorted, brute, NUM_BOXES * sizeof(double));
  qsort(sorted, NUM_BOXES, sizeof(double), cmp_double);

  int *seen = calloc(NUM_BOXES, sizeof(int));
  double *cur_dist = malloc(NUM_BOXES * sizeof(double));
  int64 *cur_id = malloc(NUM_BOXES * sizeof(int64));
  RTreeNNCursor *cursor = rtree_nn_cursor_open(rtree, query);
  int n = 0;
  int64 id;
  double dist;
  while (rtree_nn_cursor_next(cursor, &id, &dist))
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
  rtree_nn_cursor_close(cursor);

  printf("TBox NN cursor (%d random numeric values):\n", NUM_BOXES);

  bool complete = (n == NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++)
    if (seen[i] != 1)
      complete = false;
  check("(i) full drain returns every id exactly once", complete);

  bool monotone = true;
  for (int i = 1; i < n; i++)
    if (cur_dist[i] < cur_dist[i - 1] - EPS)
      monotone = false;
  check("(ii) reported distances are non-decreasing", monotone);

  bool dist_ok = true;
  for (int i = 0; i < n; i++)
    if (fabs(cur_dist[i] - brute[cur_id[i]]) > EPS)
      dist_ok = false;
  check("(iii) reported distance equals the numeric distance", dist_ok);

  bool order_ok = (n == NUM_BOXES);
  for (int i = 0; i < n && order_ok; i++)
    if (fabs(cur_dist[i] - sorted[i]) > EPS)
      order_ok = false;
  check("(iv) drained order matches the sorted brute-force distances",
    order_ok);

  /* Cleanup */
  for (int i = 0; i < NUM_BOXES; i++)
    free(boxes[i]);
  free(boxes); free(val); free(query);
  free(brute); free(sorted); free(seen); free(cur_dist); free(cur_id);
  rtree_free(rtree);
}

/*****************************************************************************
 * What the cursor refuses to open on
 *****************************************************************************/

/* A cursor validates its query box where the box becomes a query, so that the
 * traversal that follows may assume it, exactly as #rtree_search does. The
 * error handler installed here reports through #meos_errno instead of ending
 * the program, so that the refusal is read as a value rather than observed as
 * an exit */
static void
test_refused(void)
{
  meos_initialize_noexit_error_handler();

  printf("What the NN cursor refuses to open on:\n");

  RTree *rtree = rtree_create_stbox();
  STBox *box = stbox_in("SRID=4326;STBOX X((0,0),(1,1))");
  STBox *other = stbox_in("SRID=3857;STBOX X((0,0),(1,1))");
  rtree_insert(rtree, box, 0);

  meos_errno_reset();
  check("a query of another SRID is refused",
    rtree_nn_cursor_open(rtree, other) == NULL && meos_errno() != 0);

  meos_errno_reset();
  check("a null index is refused",
    rtree_nn_cursor_open(NULL, box) == NULL && meos_errno() != 0);

  meos_errno_reset();
  check("a null query is refused",
    rtree_nn_cursor_open(rtree, NULL) == NULL && meos_errno() != 0);

  /* A refused open answers NULL, so advancing that answer must report rather
   * than read through it */
  meos_errno_reset();
  check("advancing a null cursor is refused",
    ! rtree_nn_cursor_next(NULL, NULL, NULL) && meos_errno() != 0);

  meos_errno_reset();
  RTreeNNCursor *cursor = rtree_nn_cursor_open(rtree, box);
  check("a query of the tree's own SRID is accepted",
    cursor != NULL && meos_errno() == 0);
  rtree_nn_cursor_close(cursor);
  meos_errno_reset();

  free(box); free(other);
  rtree_free(rtree);
  return;
}

int
main(void)
{
  meos_initialize();
  srand(1);

  test_stbox_knn();
  test_tbox_knn();
  test_refused();

  meos_finalize();

  if (failures == 0)
    printf("\nAll NN RTree tests passed.\n");
  else
    printf("\n%d NN RTree test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
