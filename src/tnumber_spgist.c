/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file tnumber_spgist.c
 * SP-GiST implementation of 4-dimensional quad-tree over temporal integers
 * and temporal floats.
 *
 * These functions are based on those in the file ``geo_spgist.c`. from
 * PostgreSQL. This module provides SP-GiST implementation for temporal
 * number types using a quad tree analogy in 4-dimensional space. Notice
 * that SP-GiST doesn't allow indexing of overlapping objects.  We are
 * making 2D objects never-overlapping in 4D space.  This technique has some
 * benefits compared to traditional R-Tree which is implemented as GiST.
 * The performance tests reveal that this technique especially beneficial
 * with too much overlapping objects, so called "spaghetti data".
 *
 * Unlike the original quad tree, we are splitting the tree into 16
 * quadrants in 4D space.  It is easier to imagine it as splitting space
 * two times into 4:
 * @code
 *              |      |
 *              |      |
 *              | -----+-----
 *              |      |
 *              |      |
 * -------------+-------------
 *              |
 *              |
 *              |
 *              |
 *              |
 * @endcode
 * We are using a temporal box datatype as the prefix, but we are treating them
 * as points in 4-dimensional space, because 2D boxes are not enough
 * to represent the quadrant boundaries in 4D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * quadrant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the quadrants, while traversing into the tree.
 * Traversal value has all the boundaries in the 4D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *  1. the traversal value of the parent
 *  2. the quadrant of the current node
 *  3. the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing after);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up right part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all quadrants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all quadrants.  Overlap check answers
 * the question: can any box from this quadrant overlap with the given
 * box?  If yes, then this quadrant will be walked.  If no, then this
 * quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 */

#if MOBDB_PGSQL_VERSION >= 110000

#include "tnumber_spgist.h"

#include <access/spgist.h>
#include <utils/timestamp.h>
#include <utils/builtins.h>

#if MOBDB_PGSQL_VERSION >= 120000
#include <utils/float.h>
#endif

#include "temporal_util.h"
#include "tempcache.h"
#include "temporal_boxops.h"
#include "tnumber_gist.h"

#if MOBDB_PGSQL_VERSION >= 120000
/* To avoid including "access/spgist_private.h" since it conflicts with the
 * EPSILON constant defined there and also in MobilityDB */
extern double *spg_key_orderbys_distances(Datum key, bool isLeaf, ScanKey orderbys,
  int norderbys);
#endif

/*****************************************************************************/

/**
 * Structure to represent the bounding box of a temporal number as a
 * 4-dimensiononal point
 */
typedef struct
{
  TBOX  left;
  TBOX  right;
} RectBox;

/**
 * Comparator for qsort
 *
 * We don't need to use the floating point macros in here, because this is
 * only going to be used in a place to affect the performance of the index,
 * not the correctness.
 */
int
compareDoubles(const void *a, const void *b)
{
  double    x = *(double *) a;
  double    y = *(double *) b;

  if (x == y)
    return 0;
  return (x > y) ? 1 : -1;
}

/**
 * Calculate the quadrant
 *
 * The quadrant is 8 bit unsigned integer with 4 least bits in use.
 * This function accepts BOXes as input. All 4 bits are set by comparing
 * a corner of the box. This makes 16 quadrants in total.
 */
static uint8
getQuadrant4D(const TBOX *centroid, const TBOX *inBox)
{
  uint8 quadrant = 0;

  if (inBox->xmin > centroid->xmin)
    quadrant |= 0x8;

  if (inBox->xmax > centroid->xmax)
    quadrant |= 0x4;

  if (inBox->tmin > centroid->tmin)
    quadrant |= 0x2;

  if (inBox->tmax > centroid->tmax)
    quadrant |= 0x1;

  return quadrant;
}

/**
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 4D space.
 */
static RectBox *
initRectBox(void)
{
  RectBox *rect_box = (RectBox *) palloc(sizeof(RectBox));
  double infinity = get_float8_infinity();

  rect_box->left.xmin = rect_box->right.xmin = -infinity;
  rect_box->left.xmax = rect_box->right.xmax = infinity;

  rect_box->left.tmin = rect_box->right.tmin = DT_NOBEGIN;
  rect_box->left.tmax = rect_box->right.tmax = DT_NOEND;

  return rect_box;
}

/**
 * Calculate the next traversal value
 *
 * All centroids are bounded by RectBox, but SP-GiST only keeps
 * boxes.  When we are traversing the tree, we must calculate RectBox,
 * using centroid and quadrant.
 */
static RectBox *
nextRectBox(const RectBox *rect_box, const TBOX *centroid, uint8 quadrant)
{
  RectBox *next_rect_box = (RectBox *) palloc(sizeof(RectBox));

  memcpy(next_rect_box, rect_box, sizeof(RectBox));

  if (quadrant & 0x8)
    next_rect_box->left.xmin = centroid->xmin;
  else
    next_rect_box->left.xmax = centroid->xmin;

  if (quadrant & 0x4)
    next_rect_box->right.xmin = centroid->xmax;
  else
    next_rect_box->right.xmax = centroid->xmax;

  if (quadrant & 0x2)
    next_rect_box->left.tmin = centroid->tmin;
  else
    next_rect_box->left.tmax = centroid->tmin;

  if (quadrant & 0x1)
    next_rect_box->right.tmin = centroid->tmax;
  else
    next_rect_box->right.tmax = centroid->tmax;

  return next_rect_box;
}

/**
 * Can any rectangle from rect_box overlap with this argument?
 */
static bool
overlap4D(const RectBox *rect_box, const TBOX *query)
{
  bool result = true;
  /* If the dimension is not missing */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= rect_box->left.xmin <= query->xmax &&
      rect_box->right.xmax >= query->xmin;
  /* If the dimension is not missing */
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &= rect_box->left.tmin <= query->tmax &&
      rect_box->right.tmax >= query->tmin;
  return result;
}

/**
 * Can any rectangle from rect_box contain this argument?
 */
static bool
contain4D(const RectBox *rect_box, const TBOX *query)
{
  bool result = true;
  /* If the dimension is not missing */
  if (MOBDB_FLAGS_GET_X(query->flags))
    result &= rect_box->right.xmax >= query->xmax &&
      rect_box->left.xmin <= query->xmin;
  /* If the dimension is not missing */
  if (MOBDB_FLAGS_GET_T(query->flags))
    result &= rect_box->right.tmax >= query->tmax &&
      rect_box->left.tmin <= query->tmin;
  return result;
}

/**
 * Can any rectangle from rect_box be left of this argument?
 */
static bool
left4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->right.xmax < query->xmin);
}

/**
 * Can any rectangle from rect_box does not extend the right of this argument?
 */
static bool
overLeft4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->right.xmax <= query->xmax);
}

/**
 * Can any rectangle from rect_box be right of this argument?
 */
static bool
right4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->left.xmin > query->xmax);
}

/**
 * Can any rectangle from rect_box does not extend the left of this argument?
 */
static bool
overRight4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->left.xmin >= query->xmin);
}

/**
 * Can any rectangle from rect_box be before this argument?
 */
static bool
before4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->right.tmax < query->tmin);
}

/**
 * Can any rectangle from rect_box does not extend after this argument?
 */
static bool
overBefore4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->right.tmax <= query->tmax);
}

/**
 * Can any rectangle from rect_box be after this argument?
 */
static bool
after4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->left.tmin > query->tmax);
}

/**
 * Can any rectangle from rect_box does not extend before this argument?
 */
static bool
overAfter4D(const RectBox *rect_box, const TBOX *query)
{
  return (rect_box->left.tmin >= query->tmin);
}

#if MOBDB_PGSQL_VERSION >= 120000
/**
 * Lower bound for the distance between query and rect_box.
 * @note The temporal dimension is not taken into the account since it is not
 * possible to mix different units in the computation. As a consequence, the
 * filtering is not very restrictive.
 */
static double
distanceBoxRectBox(const TBOX *query, const RectBox *rect_box)
{
  double dx;

  if (query->xmax < rect_box->left.xmin)
    dx = rect_box->left.xmin - query->xmax;
  else if (query->xmin > rect_box->right.xmax)
    dx = query->xmin - rect_box->right.xmax;
  else
    dx = 0;

  return dx;
}
#endif

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_spgist_config);
/**
 * SP-GiST config function for temporal numbers
 */
PGDLLEXPORT Datum
tbox_spgist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);
  cfg->prefixType = type_oid(T_TBOX);  /* A type represented by its bounding box */
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_TBOX);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;
  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_spgist_choose);
/**
 * SP-GiST choose function for temporal numbers
 */
PGDLLEXPORT Datum
tbox_spgist_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  TBOX *centroid = DatumGetTboxP(in->prefixDatum),
    *box = DatumGetTboxP(in->leafDatum);

  out->resultType = spgMatchNode;
  out->result.matchNode.restDatum = PointerGetDatum(box);

  /* nodeN will be set by core, when allTheSame. */
  if (!in->allTheSame)
    out->result.matchNode.nodeN = getQuadrant4D(centroid, box);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_spgist_picksplit);
/**
 * SP-GiST pick-split function for temporal numbers
 *
 * It splits a list of boxes into quadrants by choosing a central 4D
 * point as the median of the coordinates of the boxes.
 */
PGDLLEXPORT Datum
tbox_spgist_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  TBOX *centroid;
  int median, i;
  double *lowXs = palloc(sizeof(double) * in->nTuples);
  double *highXs = palloc(sizeof(double) * in->nTuples);
  double *lowTs = palloc(sizeof(double) * in->nTuples);
  double *highTs = palloc(sizeof(double) * in->nTuples);

  /* Calculate median of all 4D coordinates */
  for (i = 0; i < in->nTuples; i++)
  {
    TBOX *box = DatumGetTboxP(in->datums[i]);

    lowXs[i] = box->xmin;
    highXs[i] = box->xmax;
    lowTs[i] = (double) box->tmin;
    highTs[i] = (double) box->tmax;
  }

  qsort(lowXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highXs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(lowTs, (size_t) in->nTuples, sizeof(double), compareDoubles);
  qsort(highTs, (size_t) in->nTuples, sizeof(double), compareDoubles);

  median = in->nTuples / 2;

  centroid = palloc0(sizeof(TBOX));

  centroid->xmin = lowXs[median];
  centroid->xmax = highXs[median];
  centroid->tmin = (TimestampTz) lowTs[median];
  centroid->tmax = (TimestampTz) highTs[median];

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = PointerGetDatum(centroid);

  out->nNodes = 16;
  out->nodeLabels = NULL;    /* We don't need node labels. */

  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign ranges to corresponding nodes according to quadrants relative to
   * the "centroid" range
   */
  for (i = 0; i < in->nTuples; i++)
  {
    TBOX *box = DatumGetTboxP(in->datums[i]);
    uint8 quadrant = getQuadrant4D(centroid, box);

    out->leafTupleDatums[i] = PointerGetDatum(box);
    out->mapTuplesToNodes[i] = quadrant;
  }

  pfree(lowXs); pfree(highXs);
  pfree(lowTs); pfree(highTs);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_spgist_inner_consistent);
/**
 * SP-GiST inner consistent function for temporal numbers
 */
PGDLLEXPORT Datum
tbox_spgist_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int i;
  MemoryContext old_ctx;
  RectBox *rect_box;
  uint8 quadrant;
  TBOX *centroid = DatumGetTboxP(in->prefixDatum), *queries;

  /*
   * We are saving the traversal value or initialize it an unbounded one, if
   * we have just begun to walk the tree.
   */
  if (in->traversalValue)
    rect_box = in->traversalValue;
  else
    rect_box = initRectBox();

  if (in->allTheSame)
  {
    /* Report that all nodes should be visited */
    out->nNodes = in->nNodes;
    out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
    for (i = 0; i < in->nNodes; i++)
      out->nodeNumbers[i] = i;

#if MOBDB_PGSQL_VERSION >= 120000
    if (in->norderbys > 0 && in->nNodes > 0)
    {
      double *distances = palloc(sizeof(double) * in->norderbys);
      for (int j = 0; j < in->norderbys; j++)
      {
        TBOX *box = DatumGetTboxP(in->orderbys[j].sk_argument);
        distances[j] = distanceBoxRectBox(box, rect_box);
      }

      out->distances = (double **) palloc(sizeof(double *) * in->nNodes);
      out->distances[0] = distances;

      for (i = 1; i < in->nNodes; i++)
      {
        out->distances[i] = palloc(sizeof(double) * in->norderbys);
        memcpy(out->distances[i], distances,
          sizeof(double) * in->norderbys);
      }
    }
#endif

    PG_RETURN_VOID();
  }

  /*
   * Transform the queries into bounding boxes.
   */
  queries = (TBOX *) palloc0(sizeof(TBOX) * in->nkeys);
  for (i = 0; i < in->nkeys; i++)
  {
    Oid subtype = in->scankeys[i].sk_subtype;
    if (tnumber_range_type(subtype))
      range_to_tbox_internal(&queries[i],
        DatumGetRangeTypeP(in->scankeys[i].sk_argument));
    else if (subtype == type_oid(T_TBOX))
      memcpy(&queries[i], DatumGetTboxP(in->scankeys[i].sk_argument),
        sizeof(TBOX));
    else if (tnumber_type(subtype))
      temporal_bbox(&queries[i],
        DatumGetTemporal(in->scankeys[i].sk_argument));
    else
      elog(ERROR, "Unrecognized subtype: %d", subtype);
  }

  /* Allocate enough memory for nodes */
  out->nNodes = 0;
  out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
  out->traversalValues = (void **) palloc(sizeof(void *) * in->nNodes);
#if MOBDB_PGSQL_VERSION >= 120000
  if (in->norderbys > 0)
    out->distances = (double **) palloc(sizeof(double *) * in->nNodes);
#endif
  /*
   * We switch memory context, because we want to allocate memory for new
   * traversal values (next_rect_box) and pass these pieces of memory to
   * further call of this function.
   */
  old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

  for (quadrant = 0; quadrant < in->nNodes; quadrant++)
  {
    RectBox *next_rect_box = nextRectBox(rect_box, centroid, quadrant);
    bool flag = true;
    for (i = 0; i < in->nkeys; i++)
    {
      StrategyNumber strategy = in->scankeys[i].sk_strategy;
      switch (strategy)
      {
        case RTOverlapStrategyNumber:
        case RTContainedByStrategyNumber:
        case RTAdjacentStrategyNumber:
          flag = overlap4D(next_rect_box, &queries[i]);
          break;
        case RTContainsStrategyNumber:
        case RTSameStrategyNumber:
          flag = contain4D(next_rect_box, &queries[i]);
          break;
        case RTLeftStrategyNumber:
          flag = !overRight4D(next_rect_box, &queries[i]);
          break;
        case RTOverLeftStrategyNumber:
          flag = !right4D(next_rect_box, &queries[i]);
          break;
        case RTRightStrategyNumber:
          flag = !overLeft4D(next_rect_box, &queries[i]);
          break;
        case RTOverRightStrategyNumber:
          flag = !left4D(next_rect_box, &queries[i]);
          break;
        case RTBeforeStrategyNumber:
          flag = !overAfter4D(next_rect_box, &queries[i]);
          break;
        case RTOverBeforeStrategyNumber:
          flag = !after4D(next_rect_box, &queries[i]);
          break;
        case RTAfterStrategyNumber:
          flag = !overBefore4D(next_rect_box, &queries[i]);
          break;
        case RTOverAfterStrategyNumber:
          flag = !before4D(next_rect_box, &queries[i]);
          break;
        default:
          elog(ERROR, "unrecognized strategy: %d", strategy);
      }
      /* If any check is failed, we have found our answer. */
      if (!flag)
        break;
    }
    if (flag)
    {
      out->traversalValues[out->nNodes] = next_rect_box;
      out->nodeNumbers[out->nNodes] = quadrant;
#if MOBDB_PGSQL_VERSION >= 120000
      if (in->norderbys > 0)
      {
        double *distances = palloc(sizeof(double) * in->norderbys);
        out->distances[out->nNodes] = distances;
        for (int j = 0; j < in->norderbys; j++)
        {
          TBOX *box = DatumGetTboxP(in->orderbys[j].sk_argument);
          distances[j] = distanceBoxRectBox(box, next_rect_box);
        }
      }
#endif
      out->nNodes++;
    }
    else
    {
      /*
       * If this node is not selected, we don't need to keep the next
       * traversal value in the memory context.
       */
      pfree(next_rect_box);
    }
  }

  /* Switch after */
  MemoryContextSwitchTo(old_ctx);

  pfree(queries);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_spgist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for temporal numbers
 */
PGDLLEXPORT Datum
tbox_spgist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
#if MOBDB_PGSQL_VERSION >= 120000
  Datum leaf = in->leafDatum;
#endif
  TBOX *key = DatumGetTboxP(in->leafDatum), query;
  bool res = true;
  int  i;

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
    Oid subtype = in->scankeys[i].sk_subtype;
    memset(&query, 0, sizeof(TBOX));

    if (tnumber_range_type(subtype))
    {
      RangeType *range = DatumGetRangeTypeP(in->scankeys[i].sk_argument);
      range_to_tbox_internal(&query, range);
      res = tbox_index_consistent_leaf(key, &query, strategy);
    }
    else if (subtype == type_oid(T_TBOX))
    {
      TBOX *box = DatumGetTboxP(in->scankeys[i].sk_argument);
      res = tbox_index_consistent_leaf(key, box, strategy);
    }
    else if (tnumber_type(subtype))
    {
      temporal_bbox(&query,
        DatumGetTemporal(in->scankeys[i].sk_argument));
      res = tbox_index_consistent_leaf(key, &query, strategy);
    }
    else
      elog(ERROR, "Unrecognized strategy number: %d", strategy);

    /* If any check is failed, we have found our answer. */
    if (!res)
      break;
  }

#if MOBDB_PGSQL_VERSION >= 120000
  if (res && in->norderbys > 0)
  {
    out->distances = spg_key_orderbys_distances(leaf, false, in->orderbys,
      in->norderbys);
    /* Recheck is necessary when computing distance with bounding boxes */
    out->recheckDistances = true;
  }
#endif

  PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(sptnumber_gist_compress);
/**
 * SP-GiST compress function for temporal numbers
 */
PGDLLEXPORT Datum
sptnumber_gist_compress(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  TBOX *box = palloc0(sizeof(TBOX));
  temporal_bbox(box, temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TBOX_P(box);
}
#endif

/*****************************************************************************/
