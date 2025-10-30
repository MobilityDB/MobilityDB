/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief SP-GiST implementation of 4-dimensional quad-tree and kd-tree over
 * temporal integers and temporal floats
 * @note These functions are based on those in the file ``geo_spgist.c`.
 * @details This module provides an SP-GiST implementation for temporal number
 * types using a quad tree analogy in 4-dimensional space. Since SP-GiST
 * does not allow indexing of overlapping objects, we transform 2D objects
 * in a 4D space to make them not overlap. This technique has some benefits
 * compared to traditional R-Tree which is implemented as GiST. The performance
 * tests reveal that this technique especially beneficial with too much
 * overlapping objects, so called "spaghetti data".
 *
 * Unlike the original quadtree, we are splitting the tree into 16 quadrants
 * in 4D space. It is easier to imagine it as splitting space two times into 4:
 * @code
 *              |      |
 *              |      |
 *              | -----+-----
 *              |      | c2
 *              |      |
 * -------------+-------------
 *              | c1
 *              |
 *              |
 *              |
 *              |
 * @endcode
 * where `c1` and `c2` are the centroids of the node and the child quadrant.
 *
 * We use centroids represented as a temporal box as the prefix, but we treat 
 * them as points in 4-dimensional space. Notice that 2D boxes are not enough 
 * to represent the quadrant boundaries in 4D space. However, they are 
 * sufficient to point out the additional boundaries of the next quadrant.
 *
 * We use node boxes (see below) composed by a left and a right temporal boxes
 * as traversal values to calculate and to store the bounds of the quadrants
 * while traversing the tree. A traversal value has all the boundaries in 
 * 4D space, and is is capable of transferring the required boundaries to the
 * following traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the quadrant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on the above drawing, transferred boundaries of
 * (1) would be the relevant part of the outer axis, 
 * (2) would be the up right part of the other axis, and 
 * (3) would be the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion descends
 * deeper and deeper down the tree, all quadrants in the current node will be
 * checked for overlapping.  The boundaries will be re-calculated for all
 * quadrants. Overlap check answers  the question: can any box from this
 * quadrant overlap with the given box? If yes, then this quadrant will be
 * walked. If no, then this quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

#include "pg_temporal/tnumber_spgist.h"

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
/* PostgreSQL */
#include <postgres.h>
#include <access/spgist.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/stratnum.h"
#include "temporal/tbox.h"
#include "temporal/tbox_index.h"
#include "temporal/type_util.h"
#include "cbuffer/cbuffer.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/tnumber_gist.h"

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_spgist_config(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_spgist_config);
/**
 * @brief SP-GiST config function for temporal numbers
 */
Datum
Tbox_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_TBOX);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels */
  cfg->leafType = type_oid(T_TBOX);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_quadtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_choose);
/**
 * @brief SP-GiST choose function for temporal numbers
 */
Datum
Tbox_quadtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  TBox *centroid = DatumGetTboxP(in->prefixDatum),
    *box = DatumGetTboxP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(box);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = (int) getQuadrant4D(centroid, box);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * K-d tree choose function
 *****************************************************************************/

/**
 * @brief Determine which half a 4D-mapped temporal box falls into, relative to
 * the centroid and the level number
 *
 * Halves are numbered 0 and 1, and depending on the value of level number
 * modulo 4 is even or odd, the halves will be as follows:
 * @code
 * ----+----
 *  0  |  1
 * ----+----
 * @endcode
 * or
 * @code
 * ---------
 *    1
 * ---------
 *    0
 * ---------
 * @endcode
 * where the lower bound of the splitting dimension is the horizontal axis and
 * the upper bound is the vertical axis.
 *
 * Boxes whose lower/upper bound of the splitting dimension is equal to the
 * centroid bound (including their inclusive flag) may get classified into
 * either node depending on where they happen to fall in the sorted list.
 * This is okay as long as the inner_consistent function descends into both
 * sides for such cases. This is better than the alternative of trying to
 * have an exact boundary, because it keeps the tree balanced even when we
 * have many instances of the same box value. In this way, we should never
 * trigger the allTheSame logic.
 */

PGDLLEXPORT Datum Tbox_kdtree_choose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_choose);
/**
 * @brief K-d tree choose function for time types
 */
Datum
Tbox_kdtree_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  TBox *query = DatumGetTboxP(in->leafDatum), *centroid;
  assert(in->hasPrefix);
  centroid = DatumGetTboxP(in->prefixDatum);
  assert(in->nNodes == 2);
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN =
    (tbox_level_cmp(centroid, query, in->level) < 0) ? 0 : 1;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = TboxPGetDatum(query);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

/**
 * @brief Comparator for qsort for integer values
 */
int
compareInt4(const void *a, const void *b)
{
  int x = DatumGetInt32(*(Datum *) a);
  int y = DatumGetInt32(*(Datum *) b);
  if (x == y)
    return 0;
  return (x > y) ? 1 : -1;
}

/**
 * @brief Comparator for qsort for double values
 * @note We don't need to use the floating point macros in here, because this
 * is only going to be used in a place to affect the performance of the index,
 * not the correctness.
 */
int
compareFloat8(const void *a, const void *b)
{
  double x = DatumGetFloat8(*(Datum *) a);
  double y = DatumGetFloat8(*(Datum *) b);
  if (x == y)
    return 0;
  return (x > y) ? 1 : -1;
}

/**
 * @brief Comparator for qsort for timestamp values
 */
int
compareTimestampTz(const void *a, const void *b)
{
  TimestampTz x = *(TimestampTz *) a;
  TimestampTz y = *(TimestampTz *) b;
  return timestamptz_cmp_internal(x,y);
}

PGDLLEXPORT Datum Tbox_quadtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_picksplit);
/**
 * @brief SP-GiST pick-split function for temporal numbers
 *
 * It splits a list of boxes into quadrants by choosing a central 4D
 * point as the median of the coordinates of the boxes.
 */
Datum
Tbox_quadtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  TBox *centroid;
  int median, i;
  Datum *lowXs = palloc(sizeof(Datum) * in->nTuples);
  Datum *highXs = palloc(sizeof(Datum) * in->nTuples);
  TimestampTz *lowTs = palloc(sizeof(TimestampTz) * in->nTuples);
  TimestampTz *highTs = palloc(sizeof(TimestampTz) * in->nTuples);

  /* Get basetype of span in the datums */
  TBox *box = DatumGetTboxP(in->datums[0]);
  meosType basetype = box->span.basetype;
  /* Calculate median of all 4D coordinates */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetTboxP(in->datums[i]);
    lowXs[i] = box->span.lower;
    highXs[i] = box->span.upper;
    lowTs[i] = DatumGetTimestampTz(box->period.lower);
    highTs[i] = DatumGetTimestampTz(box->period.upper);
  }

  qsort(lowXs, (size_t) in->nTuples, sizeof(Datum),
    (basetype == T_INT4) ? compareInt4 : compareFloat8);
  qsort(highXs, (size_t) in->nTuples, sizeof(Datum),
    (basetype == T_INT4) ? compareInt4 : compareFloat8);
  qsort(lowTs, (size_t) in->nTuples, sizeof(TimestampTz), compareTimestampTz);
  qsort(highTs, (size_t) in->nTuples, sizeof(TimestampTz), compareTimestampTz);

  median = in->nTuples >> 1;

  centroid = palloc0(sizeof(TBox));
  Span s, p;
  meosType spantype = basetype_spantype(basetype);
  span_set(lowXs[median], highXs[median], true, true, basetype, spantype, &s);
  span_set(lowTs[median], highTs[median], true, true, T_TIMESTAMPTZ,
    T_TSTZSPAN, &p);
  tbox_set(&s, &p, centroid);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = PointerGetDatum(centroid);
  out->nNodes = 16;
  out->nodeLabels = NULL;    /* We don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign spans to corresponding nodes according to quadrants relative to
   * the "centroid" span
   */
  for (i = 0; i < in->nTuples; i++)
  {
    box = DatumGetTboxP(in->datums[i]);
    uint8 quadrant = getQuadrant4D(centroid, box);
    out->leafTupleDatums[i] = PointerGetDatum(box);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowXs); pfree(highXs);
  pfree(lowTs); pfree(highTs);

  PG_RETURN_VOID();
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_kdtree_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_picksplit);
/**
 * @brief K-d tree pick-split function for temporal number
 */
Datum
Tbox_kdtree_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  int i;

  /* Sort the boxes and determine the centroid */
  SortedTbox *sorted = palloc(sizeof(SortedTbox) * in->nTuples);
  for (i = 0; i < in->nTuples; i++)
  {
    memcpy(&sorted[i].box, DatumGetTboxP(in->datums[i]), sizeof(TBox));
    sorted[i].i = i;
  }
  qsort_comparator qsortfn;
  int mod = in->level % 4;
  if (mod == 0)
    qsortfn = (qsort_comparator) &tbox_xmin_cmp;
  else if (mod == 1)
    qsortfn = (qsort_comparator) &tbox_xmax_cmp;
  else if (mod == 2)
    qsortfn = (qsort_comparator) &tbox_tmin_cmp;
  else
    qsortfn = (qsort_comparator) &tbox_tmax_cmp;
  qsort(sorted, in->nTuples, sizeof(SortedTbox), qsortfn);
  int median = in->nTuples >> 1;
  TBox *centroid = tbox_copy(&sorted[median].box);

  /* Fill the output data structure */
  out->hasPrefix = true;
  out->prefixDatum = TboxPGetDatum(centroid);
  out->nNodes = 2;
  out->nodeLabels = NULL;    /* we don't need node labels */
  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);
  /*
   * Note: points that have coordinates exactly equal to centroid may get
   * classified into either node, depending on where they happen to fall in
   * the sorted list. This is okay as long as the inner_consistent function
   * descends into both sides for such cases. This is better than the
   * alternative of trying to have an exact boundary, because it keeps the
   * tree balanced even when we have many instances of the same point value.
   * So we should never trigger the allTheSame logic.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    TBox *box = tbox_copy(&sorted[i].box);
    int n = sorted[i].i;
    out->mapTuplesToNodes[n] = (i < median) ? 0 : 1;
    out->leafTupleDatums[n] = TboxPGetDatum(box);
  }
  pfree(sorted);
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent function
 *****************************************************************************/

/**
 * @brief Generic SP-GiST inner consistent function for temporal numbers
 */
static Datum
tbox_spgist_inner_consistent(FunctionCallInfo fcinfo, SPGistIndexType idxtype)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  MemoryContext old_ctx;
  TBox *centroid, *queries = NULL, *orderbys = NULL; /* make compiler quiet */
  TboxNode *nodebox, infbox, next_nodebox;

  /* Fetch the centroid of this node. */
  assert(in->hasPrefix);
  centroid = DatumGetTboxP(in->prefixDatum);

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    nodebox = in->traversalValue;
  else
  {
    tboxnode_init(centroid, &infbox);
    nodebox = &infbox;
  }

  /*
   * Transform the orderbys into bounding boxes initializing the dimensions
   * that must not be taken into account for the operators to infinity.
   * This transformation is done here to avoid doing it for all nodes
   * in the loop below.
   */
  if (in->norderbys > 0)
  {
    orderbys = palloc0(sizeof(TBox) * in->norderbys);
    for (i = 0; i < in->norderbys; i++)
    {
      const ScanKeyData *scankey = &in->orderbys[i];
      Datum value = scankey->sk_argument;
      meosType type = oid_type(scankey->sk_subtype);
      tnumber_spgist_get_tbox(value, type, &orderbys[i]);
    }
  }

  if (in->allTheSame)
  {
    if (idxtype == SPGIST_QUADTREE)
    {
      /* Report that all nodes should be visited */
      out->nNodes = in->nNodes;
      out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
      for (i = 0; i < in->nNodes; i++)
      {
        out->nodeNumbers[i] = i;
        if (in->norderbys > 0 && in->nNodes > 0)
        {
          /* Use parent node box as traversalValue */
          old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);
          out->traversalValues[i] = tboxnode_copy(nodebox);
          MemoryContextSwitchTo(old_ctx);

          /* Compute the distances */
          double *distances = palloc0(sizeof(double) * in->norderbys);
          out->distances[i] = distances;
          for (int j = 0; j < in->norderbys; j++)
            distances[j] = distance_tbox_nodebox(&orderbys[j], nodebox);

          pfree(orderbys);
        }
      }

      PG_RETURN_VOID();
    }
    else
      elog(ERROR, "allTheSame should not occur for k-d trees");
  }

  /* Transform the queries into bounding boxes. */
  if (in->nkeys > 0)
  {
    queries = palloc0(sizeof(TBox) * in->nkeys);
    for (i = 0; i < in->nkeys; i++)
    {
      const ScanKeyData *scankey = &in->scankeys[i];
      Datum value = scankey->sk_argument;
      meosType type = oid_type(scankey->sk_subtype);
      tnumber_spgist_get_tbox(value, type, &queries[i]);
    }
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = palloc(sizeof(int) * in->nNodes);
  out->levelAdds = palloc(sizeof(int) * in->nNodes);
  out->traversalValues = palloc(sizeof(void *) * in->nNodes);
  if (in->norderbys > 0)
    out->distances = palloc(sizeof(double *) * in->nNodes);

  /*
   * Switch memory context to allocate memory for new traversal values
   * (next_nodebox) and pass these pieces of memory to further calls of
   * this function
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  /* Loop for each child */
  for (uint8 node = 0; node < (uint8) in->nNodes; node++)
  {
    /* Compute the bounding box of the child */
    if (idxtype == SPGIST_QUADTREE)
      tboxnode_quadtree_next(nodebox, centroid, node, &next_nodebox);
    else
      tboxnode_kdtree_next(nodebox, centroid, node, in->level, &next_nodebox);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap4D(&next_nodebox, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain4D(&next_nodebox, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = ! overRight4D(&next_nodebox, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = ! right4D(&next_nodebox, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = ! overLeft4D(&next_nodebox, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = ! left4D(&next_nodebox, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = ! overAfter4D(&next_nodebox, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = ! after4D(&next_nodebox, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = ! overBefore4D(&next_nodebox, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = ! before4D(&next_nodebox, &queries[i]);
          break;
        default:
          elog(ERROR, "unrecognized strategy: %d", strategy);
      }
      /* If any check is failed, we have found our answer. */
      if (! flag)
        break;
    }

    if (flag)
    {
      /* Pass traversalValue and node */
      out->traversalValues[out->nNodes] = tboxnode_copy(&next_nodebox);
      out->nodeNumbers[out->nNodes] = node;
      /* Increase level */
      out->levelAdds[out->nNodes] = 1;
      /* Pass distances */
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (i = 0; i < in->norderbys; i++)
          distances[i] = distance_tbox_nodebox(&orderbys[i], &next_nodebox);
      }
      out->nNodes++;
    }
  } /* Loop for every child */

  /* Switch back to initial memory context */
  MemoryContextSwitchTo(old_ctx);

  if (in->nkeys > 0)
    pfree(queries);
  if (in->norderbys > 0)
    pfree(orderbys);

  PG_RETURN_VOID();
}

PGDLLEXPORT Datum Tbox_quadtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_quadtree_inner_consistent);
/**
 * @brief Quad-tree inner consistent function for temporal numbers
 */
Datum
Tbox_quadtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return tbox_spgist_inner_consistent(fcinfo, SPGIST_QUADTREE);
}

PGDLLEXPORT Datum Tbox_kdtree_inner_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_kdtree_inner_consistent);
/**
 * @brief Kd-tree inner consistent function for temporal numbers
 */
Datum
Tbox_kdtree_inner_consistent(PG_FUNCTION_ARGS)
{
  return tbox_spgist_inner_consistent(fcinfo, SPGIST_KDTREE);
}

/*****************************************************************************
 * SP-GiST leaf consistency function
 *****************************************************************************/

PGDLLEXPORT Datum Tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_spgist_leaf_consistent);
/**
 * @brief SP-GiST leaf consistency function for temporal numbers
 */
Datum
Tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  TBox *key = DatumGetTboxP(in->leafDatum), box;
  bool result = true;
  int i;

  /*
   * All tests are lossy since boxes do not distinghish between inclusive
   * and exclusive bounds.
   */
  out->recheck = true;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    /* Convert the query to a box and perform the test */
    const ScanKeyData *scankey = &in->scankeys[i];
    Datum value = scankey->sk_argument;
    meosType type = oid_type(scankey->sk_subtype);
    if (tnumber_spgist_get_tbox(value, type, &box))
      result = tbox_index_leaf_consistent(key, &box, strategy);
    else
      result = false;
    /* If any check is failed, we have found our answer. */
    if (! result)
      break;
  }

  if (result && in->norderbys > 0)
  {
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
    double *distances = palloc(sizeof(double) * in->norderbys);
    out->distances = distances;
    for (i = 0; i < in->norderbys; i++)
    {
      /* Convert the order by argument to a box and perform the test */
      const ScanKeyData *scankey = &in->orderbys[i];
      Datum value = scankey->sk_argument;
      meosType type = oid_type(scankey->sk_subtype);
      tnumber_spgist_get_tbox(value, type, &box);
      distances[i] = nad_tbox_tbox(&box, key);
    }
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
  }

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * SP-GiST compress function
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_spgist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_spgist_compress);
/**
 * @brief SP-GiST compress function for temporal numbers
 */
Datum
Tnumber_spgist_compress(PG_FUNCTION_ARGS)
{
  Datum tempdatum = PG_GETARG_DATUM(0);
  Temporal *temp = temporal_slice(tempdatum);
  TBox *result = palloc(sizeof(TBox));
  tnumber_set_tbox(temp, result);
  PG_RETURN_TBOX_P(result);
}

/*****************************************************************************/
