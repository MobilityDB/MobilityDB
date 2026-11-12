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
 * @brief Test the in-memory indexes over boxes of different structures
 * @details A program that tests that an R-tree, a quad-tree and a k-d tree
 * holding temporal, spatiotemporal and point cloud boxes of every set of axes
 * answer every search operation as the operation applied to every stored box
 * does, as a scan of the boxes would: the set of boxes it holds for, or the
 * error it raises on the boxes of some structure. For each box type and tree,
 * 240 boxes cycling through the sets of axes of the type are inserted, every
 * search operation is asked with four queries of every set of axes, and the
 * cells where the index answers otherwise are counted, which must be none.
 * The test that an operation raises no error, #index_op_is_valid, is checked
 * against the operation itself on every ordered pair of boxes of different
 * structures, value span types, SRIDs, geodetic flags and schemas included.
 * The nearest-neighbour cursor of each tree over temporal and spatiotemporal
 * boxes is asked with the same queries and answers the distances of every
 * stored box in increasing order, or the error the distance raises on some
 * stored box, and the test that the distance raises no error,
 * #index_distance_is_valid, is checked against the distance itself.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o index_structure_test index_structure_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_pointcloud.h>

#define NBOX 240

typedef bool (*pred)(const void *, const void *);
#define P(fn, T) static bool p_##fn(const void *a, const void *b) \
  { return fn((const T *) a, (const T *) b); }
#define ALLP(T, C) P(overlaps_##T##_##T, C) P(contains_##T##_##T, C) \
  P(contained_##T##_##T, C) P(same_##T##_##T, C) P(adjacent_##T##_##T, C) \
  P(left_##T##_##T, C) P(overleft_##T##_##T, C) P(right_##T##_##T, C) \
  P(overright_##T##_##T, C) P(before_##T##_##T, C) \
  P(overbefore_##T##_##T, C) P(after_##T##_##T, C) P(overafter_##T##_##T, C)
ALLP(tbox, TBox)
ALLP(stbox, STBox)
ALLP(tpcbox, TPCBox)
#define YZP(T, C) P(below_##T##_##T, C) P(overbelow_##T##_##T, C) \
  P(above_##T##_##T, C) P(overabove_##T##_##T, C) P(front_##T##_##T, C) \
  P(overfront_##T##_##T, C) P(back_##T##_##T, C) P(overback_##T##_##T, C)
YZP(stbox, STBox)
P(below_tpcbox_tpcbox, TPCBox) P(overbelow_tpcbox_tpcbox, TPCBox)
P(above_tpcbox_tpcbox, TPCBox) P(overabove_tpcbox_tpcbox, TPCBox)

typedef struct { IndexSearchOp op; const char *name; pred p; } Op;
#define BASEOPS(T) \
  {INDEX_OVERLAPS, "&&", p_overlaps_##T##_##T}, \
  {INDEX_CONTAINS, "@>", p_contains_##T##_##T}, \
  {INDEX_CONTAINED_BY, "<@", p_contained_##T##_##T}, \
  {INDEX_SAME, "~=", p_same_##T##_##T}, \
  {INDEX_ADJACENT, "-|-", p_adjacent_##T##_##T}, \
  {INDEX_LEFT, "<<", p_left_##T##_##T}, \
  {INDEX_OVERLEFT, "&<", p_overleft_##T##_##T}, \
  {INDEX_RIGHT, ">>", p_right_##T##_##T}, \
  {INDEX_OVERRIGHT, "&>", p_overright_##T##_##T}, \
  {INDEX_BEFORE, "<<#", p_before_##T##_##T}, \
  {INDEX_OVERBEFORE, "&<#", p_overbefore_##T##_##T}, \
  {INDEX_AFTER, "#>>", p_after_##T##_##T}, \
  {INDEX_OVERAFTER, "#&>", p_overafter_##T##_##T}
static const Op tbox_ops[] = { BASEOPS(tbox) };
static const Op stbox_ops[] = { BASEOPS(stbox),
  {INDEX_BELOW, "<<|", p_below_stbox_stbox},
  {INDEX_OVERBELOW, "&<|", p_overbelow_stbox_stbox},
  {INDEX_ABOVE, "|>>", p_above_stbox_stbox},
  {INDEX_OVERABOVE, "|&>", p_overabove_stbox_stbox},
  {INDEX_FRONT, "<</", p_front_stbox_stbox},
  {INDEX_OVERFRONT, "&</", p_overfront_stbox_stbox},
  {INDEX_BACK, "/>>", p_back_stbox_stbox},
  {INDEX_OVERBACK, "/&>", p_overback_stbox_stbox} };
static const Op tpcbox_ops[] = { BASEOPS(tpcbox),
  {INDEX_BELOW, "<<|", p_below_tpcbox_tpcbox},
  {INDEX_OVERBELOW, "&<|", p_overbelow_tpcbox_tpcbox},
  {INDEX_ABOVE, "|>>", p_above_tpcbox_tpcbox},
  {INDEX_OVERABOVE, "|&>", p_overabove_tpcbox_tpcbox} };

/* The sets of axes of a box type, as the text of a box at grid cell i */
static char *
box_text(const char *type, const char *axes, int i)
{
  static char buf[256];
  int x = (i * 7) % 50, y = (i * 11) % 47, z = (i * 13) % 43, t = (i * 3) % 40;
  int w = 1 + i % 4;
  char ts[128];
  snprintf(ts, sizeof(ts), "[2001-01-%02d, 2001-01-%02d]", 1 + t % 28,
    1 + (t % 28) + (w % 3));
  if (strcmp(type, "tbox") == 0)
  {
    if (! strcmp(axes, "XT"))
      snprintf(buf, sizeof(buf), "TBOXFLOAT XT([%d, %d],%s)", x, x + w, ts);
    else if (! strcmp(axes, "X"))
      snprintf(buf, sizeof(buf), "TBOXFLOAT X([%d, %d])", x, x + w);
    else
      snprintf(buf, sizeof(buf), "TBOX T(%s)", ts);
  }
  else if (strcmp(type, "stbox") == 0)
  {
    if (! strcmp(axes, "XT"))
      snprintf(buf, sizeof(buf), "STBOX XT(((%d,%d),(%d,%d)),%s)", x, y,
        x + w, y + w, ts);
    else if (! strcmp(axes, "X"))
      snprintf(buf, sizeof(buf), "STBOX X((%d,%d),(%d,%d))", x, y, x + w, y + w);
    else if (! strcmp(axes, "ZT"))
      snprintf(buf, sizeof(buf), "STBOX ZT(((%d,%d,%d),(%d,%d,%d)),%s)", x, y,
        z, x + w, y + w, z + w, ts);
    else if (! strcmp(axes, "Z"))
      snprintf(buf, sizeof(buf), "STBOX Z((%d,%d,%d),(%d,%d,%d))", x, y, z,
        x + w, y + w, z + w);
    else
      snprintf(buf, sizeof(buf), "STBOX T(%s)", ts);
  }
  else
  {
    if (! strcmp(axes, "XT"))
      snprintf(buf, sizeof(buf), "TPCBOX(XT(((%d,%d),(%d,%d)),%s), 1)", x, y,
        x + w, y + w, ts);
    else if (! strcmp(axes, "X"))
      snprintf(buf, sizeof(buf), "TPCBOX(X((%d,%d),(%d,%d)), 1)", x, y, x + w,
        y + w);
    else
      snprintf(buf, sizeof(buf), "TPCBOX(T(%s), 1)", ts);
  }
  return buf;
}

static void *
box_in(const char *type, const char *str)
{
  if (! strcmp(type, "tbox")) return tbox_in(str);
  if (! strcmp(type, "stbox")) return stbox_in(str);
  return tpcbox_in(str);
}

/* Run one arm and answer the number of cells where the index disagrees */
static int
run_arm(const char *type, const char *tree, const char **axes, int naxes,
  const Op *ops, int nops, bool verbose)
{
  void *boxes[NBOX];
  bool stored[NBOX];
  RTree *rt = NULL;
  SPTree *sp = NULL;
  if (! strcmp(tree, "rtree"))
    rt = ! strcmp(type, "tbox") ? rtree_create_tbox() :
      (! strcmp(type, "stbox") ? rtree_create_stbox() : rtree_create_tpcbox());
  else
  {
    SPTreeKind k = ! strcmp(tree, "kdtree") ? SPTREE_KDTREE : SPTREE_QUADTREE;
    sp = ! strcmp(type, "tbox") ? sptree_create_tbox(k) :
      (! strcmp(type, "stbox") ? sptree_create_stbox(k) :
        sptree_create_tpcbox(k));
  }
  for (int i = 0; i < NBOX; i++)
  {
    boxes[i] = box_in(type, box_text(type, axes[i % naxes], i));
    meos_errno_reset();
    stored[i] = rt ? rtree_insert(rt, boxes[i], i) :
      sptree_insert(sp, boxes[i], i);
    if (meos_errno())
      stored[i] = false;
    meos_errno_reset();
  }
  MeosArray *res = index_result_create();
  int bad = 0;
  for (int o = 0; o < nops; o++)
    for (int a = 0; a < naxes; a++)
      for (int qi = 0; qi < 4; qi++)
      {
        void *q = box_in(type, box_text(type, axes[a], 1000 + qi * 37));
        /* The oracle: the operator over every box the index holds */
        bool raised = false;
        int expect = 0;
        bool *truth = calloc(NBOX, sizeof(bool));
        for (int i = 0; i < NBOX; i++)
        {
          if (! stored[i])
            continue;
          meos_errno_reset();
          truth[i] = ops[o].p(boxes[i], q);
          if (meos_errno())
          {
            raised = true;
            truth[i] = false;
          }
          expect += truth[i];
        }
        meos_errno_reset();
        int got = rt ? rtree_search(rt, ops[o].op, q, res) :
          sptree_search(sp, ops[o].op, q, res);
        bool gerr = (got < 0) || meos_errno() != 0;
        meos_errno_reset();
        bool ok;
        if (raised)
          ok = gerr && got < 0;
        else if (gerr)
          ok = false;
        else
        {
          ok = (got == expect);
          for (int i = 0; ok && i < got; i++)
          {
            int64 id;
            index_result_id(res, i, &id);
            if (! truth[id])
              ok = false;
          }
        }
        if (! ok)
        {
          bad++;
          if (verbose)
            printf("    %-4s query %-3s: oracle %s%d, index %d%s\n",
              ops[o].name, axes[a], raised ? "ERROR " : "", expect, got,
              gerr ? " with error" : "");
        }
        free(truth);
        free(q);
      }
  if (rt)
    rtree_free(rt);
  else
    sptree_free(sp);
  meos_array_destroy(res);
  for (int i = 0; i < NBOX; i++)
    free(boxes[i]);
  return bad;
}

/* Boxes of every structure of a box type, for the test of validity */
static const char *tbox_structures[] = {
  "TBOXFLOAT XT([1, 3],[2001-01-01, 2001-01-03])", "TBOXFLOAT X([1, 3])",
  "TBOX T([2001-01-01, 2001-01-03])",
  "TBOXINT XT([1, 3],[2001-01-01, 2001-01-03])", "TBOXINT X([1, 3])" };
static const char *stbox_structures[] = {
  "STBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])", "STBOX X((1,1),(3,3))",
  "STBOX ZT(((1,1,1),(3,3,3)),[2001-01-01, 2001-01-03])",
  "STBOX Z((1,1,1),(3,3,3))", "STBOX T([2001-01-01, 2001-01-03])",
  "SRID=3857;STBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])",
  "SRID=5676;STBOX X((1,1),(3,3))",
  "GEODSTBOX XT(((1,1),(3,3)),[2001-01-01, 2001-01-03])",
  "GEODSTBOX T([2001-01-01, 2001-01-03])" };
static const char *tpcbox_structures[] = {
  "TPCBOX(XT(((1,1),(3,3)),[2001-01-01, 2001-01-03]), 1)",
  "TPCBOX(X((1,1),(3,3)), 1)", "TPCBOX(T([2001-01-01, 2001-01-03]), 1)",
  "TPCBOX(XT(((1,1),(3,3)),[2001-01-01, 2001-01-03]), 2)",
  "TPCBOX(X((1,1),(3,3)), 2)" };

/* Answer the number of ordered pairs of boxes and operations where the test
 * that the operation raises no error answers otherwise than the operation */
static int
check_validity(const char *type, MeosType bboxtype, const char **strs,
  int nstrs, const Op *ops, int nops)
{
  int bad = 0;
  for (int i = 0; i < nstrs; i++)
    for (int j = 0; j < nstrs; j++)
    {
      void *box = box_in(type, strs[i]), *query = box_in(type, strs[j]);
      for (int o = 0; o < nops; o++)
      {
        meos_errno_reset();
        (void) ops[o].p(box, query);
        bool raised = meos_errno() != 0;
        meos_errno_reset();
        bool valid = index_op_is_valid(bboxtype, ops[o].op, box, query);
        if (valid == raised)
        {
          bad++;
          printf("    %-4s %s / %s: the operation %s, the test answers %s\n",
            ops[o].name, strs[i], strs[j], raised ? "raises" : "answers",
            valid ? "valid" : "invalid");
        }
      }
      free(box); free(query);
    }
  return bad;
}

/* The distance the operator |=| answers for two temporal or spatiotemporal
 * boxes, DBL_MAX where they share no time, and an error flagged through
 * meos_errno */
static double
box_dist(const char *type, const void *a, const void *b)
{
  if (! strcmp(type, "stbox"))
    return nad_stbox_stbox((const STBox *) a, (const STBox *) b, true);
  double d = nad_tbox_tbox((const TBox *) a, (const TBox *) b);
  return d < 0 ? DBL_MAX : d;
}

static int
cmp_double(const void *a, const void *b)
{
  double x = *(const double *) a, y = *(const double *) b;
  return (x > y) - (x < y);
}

/* Run the nearest-neighbour arm of a tree and answer the number of queries
 * where the cursor answers otherwise than the distances of every stored box
 * sorted, or than the error the distance raises on some stored box */
static int
run_knn_arm(const char *type, const char *tree, const char **axes, int naxes)
{
  void *boxes[NBOX];
  bool stored[NBOX];
  RTree *rt = NULL;
  SPTree *sp = NULL;
  bool istbox = ! strcmp(type, "tbox");
  if (! strcmp(tree, "rtree"))
    rt = istbox ? rtree_create_tbox() : rtree_create_stbox();
  else
  {
    SPTreeKind k = ! strcmp(tree, "kdtree") ? SPTREE_KDTREE : SPTREE_QUADTREE;
    sp = istbox ? sptree_create_tbox(k) : sptree_create_stbox(k);
  }
  for (int i = 0; i < NBOX; i++)
  {
    boxes[i] = box_in(type, box_text(type, axes[i % naxes], i));
    meos_errno_reset();
    stored[i] = rt ? rtree_insert(rt, boxes[i], i) :
      sptree_insert(sp, boxes[i], i);
    if (meos_errno())
      stored[i] = false;
    meos_errno_reset();
  }
  int bad = 0;
  double expect[NBOX], got[NBOX];
  for (int a = 0; a < naxes; a++)
    for (int qi = 0; qi < 4; qi++)
    {
      void *q = box_in(type, box_text(type, axes[a], 1000 + qi * 37));
      bool raised = false;
      int nexp = 0;
      for (int i = 0; i < NBOX; i++)
      {
        if (! stored[i])
          continue;
        meos_errno_reset();
        expect[nexp++] = box_dist(type, boxes[i], q);
        raised |= (meos_errno() != 0);
      }
      qsort(expect, nexp, sizeof(double), cmp_double);
      meos_errno_reset();
      int ngot = 0;
      bool gerr;
      void *cur = rt ? (void *) rtree_nn_cursor_open(rt, q) :
        (void *) sptree_nn_cursor_open(sp, q);
      gerr = ! cur || meos_errno() != 0;
      if (cur)
      {
        double d;
        while (ngot < NBOX && (rt ?
            rtree_nn_cursor_next((RTreeNNCursor *) cur, NULL, &d) :
            sptree_nn_cursor_next((SPNNCursor *) cur, NULL, &d)))
          got[ngot++] = d;
        gerr |= (meos_errno() != 0);
        if (rt)
          rtree_nn_cursor_close((RTreeNNCursor *) cur);
        else
          sptree_nn_cursor_close((SPNNCursor *) cur);
      }
      meos_errno_reset();
      bool ok = (raised || gerr) ? (raised && gerr) : (ngot == nexp);
      for (int i = 0; ok && ! raised && i < ngot; i++)
        if (! ((expect[i] >= DBL_MAX && got[i] >= DBL_MAX) ||
               fabs(expect[i] - got[i]) <= 1e-9 * (1 + fabs(expect[i]))))
          ok = false;
      if (! ok)
      {
        bad++;
        printf("    |=|  query %-3s: the distances %s%d, the cursor %s%d\n",
          axes[a], raised ? "raise, " : "", nexp, gerr ? "raises, " : "",
          ngot);
      }
      free(q);
    }
  for (int i = 0; i < NBOX; i++)
    free(boxes[i]);
  if (rt)
    rtree_free(rt);
  else
    sptree_free(sp);
  return bad;
}

/* Answer the number of ordered pairs of boxes where the test that the
 * distance raises no error answers otherwise than the distance */
static int
check_distance_validity(const char *type, MeosType bboxtype,
  const char **strs, int nstrs)
{
  int bad = 0;
  for (int i = 0; i < nstrs; i++)
    for (int j = 0; j < nstrs; j++)
    {
      void *box = box_in(type, strs[i]), *query = box_in(type, strs[j]);
      meos_errno_reset();
      (void) box_dist(type, box, query);
      bool raised = meos_errno() != 0;
      meos_errno_reset();
      bool valid = index_distance_is_valid(bboxtype, box, query);
      if (valid == raised)
      {
        bad++;
        printf("    |=|  %s / %s: the distance %s, the test answers %s\n",
          strs[i], strs[j], raised ? "raises" : "answers",
          valid ? "valid" : "invalid");
      }
      free(box); free(query);
    }
  return bad;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();
  PCDimensionSpec dims[2] = {
    { "X", NULL, 1, "int32_t", 1, 0, true },
    { "Y", NULL, 2, "int32_t", 1, 0, true } };
  meos_pc_schema_register_dims(1, 0, "none", dims, 2);
  meos_pc_schema_register_dims(2, 0, "none", dims, 2);
  static const char *tbox_axes[] = {"XT", "X", "T"};
  static const char *stbox_axes[] = {"XT", "X", "ZT", "Z", "T"};
  static const char *tpcbox_axes[] = {"XT", "X", "T"};
  struct { const char *type; const char **axes; int naxes; const Op *ops;
    int nops; } types[] = {
    {"tbox", tbox_axes, 3, tbox_ops, sizeof(tbox_ops) / sizeof(Op)},
    {"stbox", stbox_axes, 5, stbox_ops, sizeof(stbox_ops) / sizeof(Op)},
    {"tpcbox", tpcbox_axes, 3, tpcbox_ops, sizeof(tpcbox_ops) / sizeof(Op)} };
  static const char *trees[] = {"rtree", "quadtree", "kdtree"};
  int failures = 0;
  for (int t = 0; t < 3; t++)
    for (int r = 0; r < 3; r++)
    {
      int cells = types[t].nops * types[t].naxes * 4;
      int bad = run_arm(types[t].type, trees[r], types[t].axes,
        types[t].naxes, types[t].ops, types[t].nops, true);
      printf("%-7s %-9s %3d of %3d cells answer as a scan  %s\n",
        types[t].type, trees[r], cells - bad, cells, bad ? "FAIL" : "OK");
      failures += (bad != 0);
    }
  struct { const char *type; MeosType bboxtype; const char **strs; int nstrs;
    const Op *ops; int nops; } valid[] = {
    {"tbox", T_TBOX, tbox_structures, 5, tbox_ops,
      sizeof(tbox_ops) / sizeof(Op)},
    {"stbox", T_STBOX, stbox_structures, 9, stbox_ops,
      sizeof(stbox_ops) / sizeof(Op)},
    {"tpcbox", T_TPCBOX, tpcbox_structures, 5, tpcbox_ops,
      sizeof(tpcbox_ops) / sizeof(Op)} };
  for (int t = 0; t < 3; t++)
  {
    int pairs = valid[t].nstrs * valid[t].nstrs * valid[t].nops;
    int bad = check_validity(valid[t].type, valid[t].bboxtype, valid[t].strs,
      valid[t].nstrs, valid[t].ops, valid[t].nops);
    printf("%-7s validity  %3d of %3d pairs answer as the operation  %s\n",
      valid[t].type, pairs - bad, pairs, bad ? "FAIL" : "OK");
    failures += (bad != 0);
  }
  for (int t = 0; t < 2; t++)
  {
    for (int r = 0; r < 3; r++)
    {
      int cells = types[t].naxes * 4;
      int bad = run_knn_arm(types[t].type, trees[r], types[t].axes,
        types[t].naxes);
      printf("%-7s %-9s %3d of %3d nearest-neighbour queries answer as a "
        "scan  %s\n", types[t].type, trees[r], cells - bad, cells,
        bad ? "FAIL" : "OK");
      failures += (bad != 0);
    }
    int pairs = valid[t].nstrs * valid[t].nstrs;
    int bad = check_distance_validity(valid[t].type, valid[t].bboxtype,
      valid[t].strs, valid[t].nstrs);
    printf("%-7s distance  %3d of %3d pairs answer as the distance  %s\n",
      valid[t].type, pairs - bad, pairs, bad ? "FAIL" : "OK");
    failures += (bad != 0);
  }
  meos_finalize();
  if (failures == 0)
    printf("\nAll index structure tests passed.\n");
  else
    printf("\n%d index structure test(s) FAILED.\n", failures);
  return failures == 0 ? 0 : 1;
}
