/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file tpoint_gist.c
 * @brief R-tree GiST index for temporal points.
 */

#include "point/tpoint_gist.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
#include <access/gist.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/time_gist.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "point/tpoint.h"
#include "general/tnumber_gist.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_posops.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * Leaf-level consistency for temporal points.
 *
 * Since spatiotemporal boxes do not distinguish between inclusive and
 * exclusive bounds it is necessary to generalize the tests, e.g.,
 * - before : (box1->tmax < box2->tmin) => (box1->tmax <= box2->tmin)
 *   e.g., to take into account before([a,b],(b,c])
 * - after : (box1->tmin > box2->tmax) => (box1->tmin >= box2->tmax)
 *   e.g., to take into account after((b,c],[a,b])
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
stbox_index_consistent_leaf(const STBOX *key, const STBOX *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      retval = overlaps_stbox_stbox(key, query);
      break;
    case RTContainsStrategyNumber:
      retval = contains_stbox_stbox(key, query);
      break;
    case RTContainedByStrategyNumber:
      retval = contained_stbox_stbox(key, query);
      break;
    case RTSameStrategyNumber:
      retval = same_stbox_stbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      retval = adjacent_stbox_stbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = left_stbox_stbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = overleft_stbox_stbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = right_stbox_stbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = overright_stbox_stbox(key, query);
      break;
    case RTBelowStrategyNumber:
      retval = below_stbox_stbox(key, query);
      break;
    case RTOverBelowStrategyNumber:
      retval = overbelow_stbox_stbox(key, query);
      break;
    case RTAboveStrategyNumber:
      retval = above_stbox_stbox(key, query);
      break;
    case RTOverAboveStrategyNumber:
      retval = overabove_stbox_stbox(key, query);
      break;
    case RTFrontStrategyNumber:
      retval = front_stbox_stbox(key, query);
      break;
    case RTOverFrontStrategyNumber:
      retval = overfront_stbox_stbox(key, query);
      break;
    case RTBackStrategyNumber:
      retval = back_stbox_stbox(key, query);
      break;
    case RTOverBackStrategyNumber:
      retval = overback_stbox_stbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = /* before_stbox_stbox(key, query) */
        (key->tmax <= query->tmin);
      break;
    case RTOverBeforeStrategyNumber:
      retval = overbefore_stbox_stbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = /* after_stbox_stbox(key, query)*/
        (key->tmin >= query->tmax);
      break;
    case RTOverAfterStrategyNumber:
      retval = overafter_stbox_stbox(key, query);
      break;
    default:
      elog(ERROR, "unrecognized strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * Internal-page consistent method for temporal points.
 *
 * Return false if for all data items x below entry, the predicate
 * x op query must be false, where op is the oper corresponding to strategy
 * in the pg_amop table.
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
static bool
stbox_gist_consistent(const STBOX *key, const STBOX *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      retval = overlaps_stbox_stbox(key, query);
      break;
    case RTContainsStrategyNumber:
    case RTSameStrategyNumber:
      retval = contains_stbox_stbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      if (adjacent_stbox_stbox(key, query))
        return true;
      return overlaps_stbox_stbox(key, query);
    case RTLeftStrategyNumber:
      retval = !overright_stbox_stbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = !right_stbox_stbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = !overleft_stbox_stbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = !left_stbox_stbox(key, query);
      break;
    case RTBelowStrategyNumber:
      retval = !overabove_stbox_stbox(key, query);
      break;
    case RTOverBelowStrategyNumber:
      retval = !above_stbox_stbox(key, query);
      break;
    case RTAboveStrategyNumber:
      retval = !overbelow_stbox_stbox(key, query);
      break;
    case RTOverAboveStrategyNumber:
      retval = !below_stbox_stbox(key, query);
      break;
    case RTFrontStrategyNumber:
      retval = !overback_stbox_stbox(key, query);
      break;
    case RTOverFrontStrategyNumber:
      retval = !back_stbox_stbox(key, query);
      break;
    case RTBackStrategyNumber:
      retval = !overfront_stbox_stbox(key, query);
      break;
    case RTOverBackStrategyNumber:
      retval = !front_stbox_stbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = !overafter_stbox_stbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = !after_stbox_stbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = !overbefore_stbox_stbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = !before_stbox_stbox(key, query);
      break;
    default:
      elog(ERROR, "unrecognized strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * Determine whether a recheck is necessary depending on the strategy
 *
 * @param[in] strategy Operator of the operator class being applied
 */
bool
tpoint_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes and do not consider
   * inclusive or exclusive bounds */
  switch (strategy)
  {
    case RTLeftStrategyNumber:
    case RTOverLeftStrategyNumber:
    case RTRightStrategyNumber:
    case RTOverRightStrategyNumber:
    case RTBelowStrategyNumber:
    case RTOverBelowStrategyNumber:
    case RTAboveStrategyNumber:
    case RTOverAboveStrategyNumber:
    case RTFrontStrategyNumber:
    case RTOverFrontStrategyNumber:
    case RTBackStrategyNumber:
    case RTOverBackStrategyNumber:
      return false;
    default:
      return true;
  }
}

/**
 * Transform the query argument into a box initializing the dimensions that
 * must not be taken into account by the operators to infinity.
 */
static bool
tpoint_gist_get_stbox(FunctionCallInfo fcinfo, STBOX *result,
  CachedType type)
{
  if (tgeo_basetype(type))
  {
    GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
    if (gs == NULL || gserialized_is_empty(gs))
      return false;
    geo_stbox(gs, result);
  }
  else if (type == T_TIMESTAMPTZ)
  {
    TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
    timestamp_stbox(t, result);
  }
  else if (type == T_TIMESTAMPSET)
  {
    Datum tsdatum = PG_GETARG_DATUM(1);
    timestampset_stbox_slice(tsdatum, result);
  }
  else if (type == T_PERIOD)
  {
    Period *p = PG_GETARG_PERIOD_P(1);
    period_stbox(p, result);
  }
  else if (type == T_PERIODSET)
  {
    Datum psdatum = PG_GETARG_DATUM(1);
    periodset_stbox_slice(psdatum, result);
  }
  else if (type == T_STBOX)
  {
    STBOX *box = PG_GETARG_STBOX_P(1);
    if (box == NULL)
      return false;
    memcpy(result, box, sizeof(STBOX));
  }
  else if (tspatial_type(type))
  {
    if (PG_ARGISNULL(1))
      return false;
    Datum tempdatum = PG_GETARG_DATUM(1);
    temporal_bbox_slice(tempdatum, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

PG_FUNCTION_INFO_V1(Stbox_gist_consistent);
/**
 * GiST consistent method for temporal points
 */
PGDLLEXPORT Datum
Stbox_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4), result;
  STBOX *key = (STBOX *)DatumGetPointer(entry->key),
    query;

  /* Determine whether the index is lossy depending on the strategy */
  *recheck = tpoint_index_recheck(strategy);

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! tpoint_gist_get_stbox(fcinfo, &query, oid_type(typid)))
    PG_RETURN_BOOL(false);

  if (GIST_LEAF(entry))
    result = stbox_index_consistent_leaf(key, &query, strategy);
  else
    result = stbox_gist_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

/**
 * Increase the first box to include the second one
 */
static void
stbox_adjust(STBOX *box1, const STBOX *box2)
{
  box1->xmin = FLOAT8_MIN(box1->xmin, box2->xmin);
  box1->xmax = FLOAT8_MAX(box1->xmax, box2->xmax);
  box1->ymin = FLOAT8_MIN(box1->ymin, box2->ymin);
  box1->ymax = FLOAT8_MAX(box1->ymax, box2->ymax);
  box1->zmin = FLOAT8_MIN(box1->zmin, box2->zmin);
  box1->zmax = FLOAT8_MAX(box1->zmax, box2->zmax);
  box1->tmin = Min(box1->tmin, box2->tmin);
  box1->tmax = Max(box1->tmax, box2->tmax);
  return;
}

PG_FUNCTION_INFO_V1(Stbox_gist_union);
/**
 * GiST union method for temporal points
 *
 * Return the minimal bounding box that encloses all the entries in entryvec
 */
PGDLLEXPORT Datum
Stbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  STBOX *result = stbox_copy(DatumGetSTboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    stbox_adjust(result, DatumGetSTboxP(ent[i].key));
  PG_RETURN_PERIOD_P(result);
}

/*****************************************************************************
 * GiST compress methods
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_gist_compress);
/**
 * GiST compress methods for temporal points
 */
PGDLLEXPORT Datum
Tpoint_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
    STBOX *box = (STBOX *) palloc(sizeof(STBOX));
    temporal_bbox_slice(entry->key, box);
    gistentryinit(*retval, PointerGetDatum(box), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST penalty method
 *****************************************************************************/

/**
 * Calculates the union of two tboxes.
 *
 * @param[in] a,b Input boxes
 * @param[out] new Resulting box
 */
static void
stbox_union_rt(const STBOX *a, const STBOX *b, STBOX *new)
{
  memset(new, 0, sizeof(STBOX));
  new->xmax = FLOAT8_MAX(a->xmax, b->xmax);
  new->ymax = FLOAT8_MAX(a->ymax, b->ymax);
  new->zmax = FLOAT8_MAX(a->zmax, b->zmax);
  new->tmax = a->tmax > b->tmax ? a->tmax : b->tmax;
  new->xmin = FLOAT8_MIN(a->xmin, b->xmin);
  new->ymin = FLOAT8_MIN(a->ymin, b->ymin);
  new->zmin = FLOAT8_MIN(a->zmin, b->zmin);
  new->tmin = a->tmin < b->tmin ? a->tmin : b->tmin;
  return;
}

/**
 * Return the size of a spatiotemporal box for penalty-calculation purposes.
 * The result can be +Infinity, but not NaN.
 */
static double
stbox_size(const STBOX *box)
{
  /*
   * Check for zero-width cases.  Note that we define the size of a zero-
   * by-infinity box as zero.  It's important to special-case this somehow,
   * as naively multiplying infinity by zero will produce NaN.
   *
   * The less-than cases should not happen, but if they do, say "zero".
   */
  if (FLOAT8_LE(box->xmax, box->xmin) || FLOAT8_LE(box->ymax, box->ymin) ||
    FLOAT8_LE(box->zmax, box->zmin) || box->tmax <= box->tmin)
    return 0.0;

  /*
   * We treat NaN as larger than +Infinity, so any distance involving a NaN
   * and a non-NaN is infinite.  Note the previous check eliminated the
   * possibility that the low fields are NaNs.
   */
  if (isnan(box->xmax) || isnan(box->ymax) || isnan(box->zmax))
    return get_float8_infinity();
  return (box->xmax - box->xmin) * (box->ymax - box->ymin) *
    (box->zmax - box->zmin) * (box->tmax - box->tmin);
}

/**
 * Return the amount by which the union of the two boxes is larger than
 * the original STBOX's volume.  The result can be +Infinity, but not NaN.
 */
static double
stbox_penalty(const STBOX *original, const STBOX *new)
{
  STBOX unionbox;
  stbox_union_rt(original, new, &unionbox);
  return stbox_size(&unionbox) - stbox_size(original);
}


PG_FUNCTION_INFO_V1(Stbox_gist_penalty);
/**
 * GiST penalty method for temporal points.
 * As in the R-tree paper, we use change in area as our penalty metric
 */
PGDLLEXPORT Datum
Stbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  STBOX *origstbox = (STBOX *) DatumGetPointer(origentry->key);
  STBOX *newbox = (STBOX *) DatumGetPointer(newentry->key);

  *result = (float) stbox_penalty(origstbox, newbox);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

/**
 * Structure keeping context for the function stbox_gist_consider_split.
 *
 * Contains information about currently selected split and some general
 * information.
 */
typedef struct
{
  int  entriesCount;  /**< total number of entries being split */
  STBOX boundingBox;  /**< minimum bounding box across all entries */

  /** Information about currently selected split follows */
  bool first;        /**< true if no split was selected yet */
  double leftUpper;  /**< upper bound of left interval */
  double rightLower; /**< lower bound of right interval */
  float4 ratio;
  float4 overlap;
  int  dim;          /**< axis of this split */
  double range;      /**< width of general MBR projection to the selected axis */
} ConsiderSplitContext;

/**
 * Trivial split: half of entries will be placed on one page
 * and another half to another
 */
static void
stbox_gist_fallback_split(GistEntryVector *entryvec, GIST_SPLITVEC *v)
{
  OffsetNumber i, maxoff;
  STBOX *left_tbox = NULL, *right_stbox = NULL;
  size_t nbytes;

  maxoff = (OffsetNumber) (entryvec->n - 1);

  nbytes = (maxoff + 2) * sizeof(OffsetNumber);
  v->spl_left = (OffsetNumber *) palloc(nbytes);
  v->spl_right = (OffsetNumber *) palloc(nbytes);
  v->spl_nleft = v->spl_nright = 0;

  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    STBOX *cur = (STBOX *)DatumGetPointer(entryvec->vector[i].key);
    if (i <= (maxoff - FirstOffsetNumber + 1) / 2)
    {
      v->spl_left[v->spl_nleft] = i;
      if (left_tbox == NULL)
      {
        left_tbox = (STBOX *) palloc0(sizeof(STBOX));
        *left_tbox = *cur;
      }
      else
        stbox_adjust(left_tbox, cur);

      v->spl_nleft++;
    }
    else
    {
      v->spl_right[v->spl_nright] = i;
      if (right_stbox == NULL)
      {
        right_stbox = (STBOX *) palloc0(sizeof(STBOX));
        *right_stbox = *cur;
      }
      else
        stbox_adjust(right_stbox, cur);

      v->spl_nright++;
    }
  }

  v->spl_ldatum = PointerGetDatum(left_tbox);
  v->spl_rdatum = PointerGetDatum(right_stbox);
  return;
}

/**
 * Consider replacement of currently selected split with the better one.
 */
static inline void
stbox_gist_consider_split(ConsiderSplitContext *context, int dimNum,
  double rightLower, int minLeftCount, double leftUpper, int maxLeftCount)
{
  int      leftCount,
        rightCount;
  float4    ratio,
        overlap;

  /*
   * Calculate entries distribution ratio assuming most uniform distribution
   * of common entries.
   */
  if (minLeftCount >= (context->entriesCount + 1) / 2)
  {
    leftCount = minLeftCount;
  }
  else
  {
    if (maxLeftCount <= context->entriesCount / 2)
      leftCount = maxLeftCount;
    else
      leftCount = context->entriesCount / 2;
  }
  rightCount = context->entriesCount - leftCount;

  /*
   * Ratio of split - quotient between size of lesser group and total
   * entries count.
   */
  ratio = ((float4) Min(leftCount, rightCount)) /
    ((float4) context->entriesCount);

  if (ratio > LIMIT_RATIO)
  {
    double    range;
    bool    selectthis = false;

    /*
     * The ratio is acceptable, so compare current split with previously
     * selected one. Between splits of one dimension we search for minimal
     * overlap (allowing negative values) and minimal ration (between same
     * overlaps. We switch dimension if find less overlap (non-negative)
     * or less range with same overlap.
     */
    if (dimNum == 0)
      range = context->boundingBox.xmax - context->boundingBox.xmin;
    else if (dimNum == 1)
      range = context->boundingBox.ymax - context->boundingBox.ymin;
    else if (dimNum == 2)
      range = context->boundingBox.zmax - context->boundingBox.zmin;
    else
      range = context->boundingBox.tmax - context->boundingBox.tmin;

    overlap = (float4) ((leftUpper - rightLower) / range);

    /* If there is no previous selection, select this */
    if (context->first)
      selectthis = true;
    else if (context->dim == dimNum)
    {
      /*
       * Within the same dimension, choose the new split if it has a
       * smaller overlap, or same overlap but better ratio.
       */
      if (overlap < context->overlap ||
        (overlap == context->overlap && ratio > context->ratio))
        selectthis = true;
    }
    else
    {
      /*
       * Across dimensions, choose the new split if it has a smaller
       * *non-negative* overlap, or same *non-negative* overlap but
       * bigger range. This condition differs from the one described in
       * the article. On the datasets where leaf MBRs don't overlap
       * themselves, non-overlapping splits (i.e. splits which have zero
       * *non-negative* overlap) are frequently possible. In this case
       * splits tends to be along one dimension, because most distant
       * non-overlapping splits (i.e. having lowest negative overlap)
       * appears to be in the same dimension as in the previous split.
       * Therefore MBRs appear to be very prolonged along another
       * dimension, which leads to bad search performance. Using range
       * as the second split criteria makes MBRs more quadratic. Using
       * *non-negative* overlap instead of overlap as the first split
       * criteria gives to range criteria a chance to matter, because
       * non-overlapping splits are equivalent in this criteria.
       */
      if (non_negative(overlap) < non_negative(context->overlap) ||
        (range > context->range &&
         non_negative(overlap) <= non_negative(context->overlap)))
        selectthis = true;
    }

    if (selectthis)
    {
      /* save information about selected split */
      context->first = false;
      context->ratio = ratio;
      context->range = range;
      context->overlap = overlap;
      context->rightLower = rightLower;
      context->leftUpper = leftUpper;
      context->dim = dimNum;
    }
  }
  return;
}

PG_FUNCTION_INFO_V1(Stbox_gist_picksplit);
/**
 * GiST picksplit method for temporal points.
 *
 * The algorithm finds split of boxes by considering splits along each axis.
 * Each entry is first projected as an interval on the X-axis, and different
 * ways to split the intervals into two groups are considered, trying to
 * minimize the overlap of the groups. Then the same is repeated for the
 * Y-axis and the Z-axis, and the overall best split is chosen.
 * The quality of a split is determined by overlap along that axis and some
 * other criteria (see stbox_gist_consider_split).
 *
 * After that, all the entries are divided into three groups:
 *
 * 1. Entries which should be placed to the left group
 * 2. Entries which should be placed to the right group
 * 3. "Common entries" which can be placed to any of groups without affecting
 *    of overlap along selected axis.
 *
 * The common entries are distributed by minimizing penalty.
 *
 * For details see:
 * "A new double sorting-based node splitting algorithm for R-tree", A. Korotkov
 * http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf#page=36
 */
PGDLLEXPORT Datum
Stbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  OffsetNumber i,
        maxoff;
  ConsiderSplitContext context;
  STBOX     *box,
         *leftBox,
         *rightBox;
  int      dim,
        nentries,
        commonEntriesCount;
  bool     hasz;
  SplitInterval *intervalsLower,
        *intervalsUpper;
  CommonEntry *commonEntries;

  memset(&context, 0, sizeof(ConsiderSplitContext));

  maxoff = (OffsetNumber) (entryvec->n - 1);
  nentries = context.entriesCount = maxoff - FirstOffsetNumber + 1;

  /* Allocate arrays for intervals along axes */
  intervalsLower = (SplitInterval *) palloc(nentries * sizeof(SplitInterval));
  intervalsUpper = (SplitInterval *) palloc(nentries * sizeof(SplitInterval));

  /*
   * Calculate the overall minimum bounding box over all the entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    box = (STBOX *)DatumGetPointer(entryvec->vector[i].key);
    if (i == FirstOffsetNumber)
      context.boundingBox = *box;
    else
      stbox_adjust(&context.boundingBox, box);
  }

  /* Determine whether there is a Z dimension */
  box = (STBOX *)DatumGetPointer(entryvec->vector[FirstOffsetNumber].key);
  hasz = MOBDB_FLAGS_GET_Z(box->flags);

  /*
   * Iterate over axes for optimal split searching.
   */
  context.first = true;    /* nothing selected yet */
  for (dim = 0; dim < 4; dim++)
  {
    double    leftUpper,
          rightLower;
    int      i1,
          i2;

    /* Skip the process for Z dimension if it is missing */
    if (dim == 2 && !hasz)
      continue;

    /* Project each entry as an interval on the selected axis. */
    for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
    {
      box = (STBOX *)DatumGetPointer(entryvec->vector[i].key);
      if (dim == 0)
      {
        intervalsLower[i - FirstOffsetNumber].lower = box->xmin;
        intervalsLower[i - FirstOffsetNumber].upper = box->xmax;
      }
      else if (dim == 1)
      {
        intervalsLower[i - FirstOffsetNumber].lower = box->ymin;
        intervalsLower[i - FirstOffsetNumber].upper = box->ymax;
      }
      else if (dim == 2)
      {
        intervalsLower[i - FirstOffsetNumber].lower = box->zmin;
        intervalsLower[i - FirstOffsetNumber].upper = box->zmax;
      }
      else
      {
        intervalsLower[i - FirstOffsetNumber].lower = box->tmin;
        intervalsLower[i - FirstOffsetNumber].upper = box->tmax;
      }
    }

    /*
     * Make two arrays of intervals: one sorted by lower bound and another
     * sorted by upper bound.
     */
    memcpy(intervalsUpper, intervalsLower,
         sizeof(SplitInterval) * nentries);
    qsort(intervalsLower, (size_t) nentries, sizeof(SplitInterval),
        interval_cmp_lower);
    qsort(intervalsUpper, (size_t) nentries, sizeof(SplitInterval),
        interval_cmp_upper);

    /*----
     * The goal is to form a left and right interval, so that every entry
     * interval is contained by either left or right interval (or both).
     *
     * For example, with the intervals (0,1), (1,3), (2,3), (2,4):
     *
     * 0 1 2 3 4
     * +-+
     *   +---+
     *     +-+
     *     +---+
     *
     * The left and right intervals are of the form (0,a) and (b,4).
     * We first consider splits where b is the lower bound of an entry.
     * We iterate through all entries, and for each b, calculate the
     * smallest possible a. Then we consider splits where a is the
     * upper bound of an entry, and for each a, calculate the greatest
     * possible b.
     *
     * In the above example, the first loop would consider splits:
     * b=0: (0,1)-(0,4)
     * b=1: (0,1)-(1,4)
     * b=2: (0,3)-(2,4)
     *
     * And the second loop:
     * a=1: (0,1)-(1,4)
     * a=3: (0,3)-(2,4)
     * a=4: (0,4)-(2,4)
     */

    /*
     * Iterate over lower bound of right group, finding smallest possible
     * upper bound of left group.
     */
    i1 = 0;
    i2 = 0;
    rightLower = intervalsLower[i1].lower;
    leftUpper = intervalsUpper[i2].lower;
    while (true)
    {
      /*
       * Find next lower bound of right group.
       */
      while (i1 < nentries &&
           FLOAT8_EQ(rightLower, intervalsLower[i1].lower))
      {
        if (FLOAT8_LT(leftUpper, intervalsLower[i1].upper))
          leftUpper = intervalsLower[i1].upper;
        i1++;
      }
      if (i1 >= nentries)
        break;
      rightLower = intervalsLower[i1].lower;

      /*
       * Find count of intervals which anyway should be placed to the
       * left group.
       */
      while (i2 < nentries &&
           FLOAT8_LE(intervalsUpper[i2].upper, leftUpper))
        i2++;

      /*
       * Consider found split.
       */
      stbox_gist_consider_split(&context, dim, rightLower, i1, leftUpper, i2);
    }

    /*
     * Iterate over upper bound of left group finding greatest possible
     * lower bound of right group.
     */
    i1 = nentries - 1;
    i2 = nentries - 1;
    rightLower = intervalsLower[i1].upper;
    leftUpper = intervalsUpper[i2].upper;
    while (true)
    {
      /*
       * Find next upper bound of left group.
       */
      while (i2 >= 0 && FLOAT8_EQ(leftUpper, intervalsUpper[i2].upper))
      {
        if (FLOAT8_GT(rightLower, intervalsUpper[i2].lower))
          rightLower = intervalsUpper[i2].lower;
        i2--;
      }
      if (i2 < 0)
        break;
      leftUpper = intervalsUpper[i2].upper;

      /*
       * Find count of intervals which anyway should be placed to the
       * right group.
       */
      while (i1 >= 0 && FLOAT8_GE(intervalsLower[i1].lower, rightLower))
        i1--;

      /*
       * Consider found split.
       */
      stbox_gist_consider_split(&context, dim, rightLower, i1 + 1,
        leftUpper, i2 + 1);
    }
  }

  /*
   * If we failed to find any acceptable splits, use trivial split.
   */
  if (context.first)
  {
    stbox_gist_fallback_split(entryvec, v);
    PG_RETURN_POINTER(v);
  }

  /*
   * Ok, we have now selected the split across one axis.
   *
   * While considering the splits, we already determined that there will be
   * enough entries in both groups to reach the desired ratio, but we did
   * not memorize which entries go to which group. So determine that now.
   */

  /* Allocate vectors for results */
  v->spl_left = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
  v->spl_right = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
  v->spl_nleft = 0;
  v->spl_nright = 0;

  /* Allocate bounding boxes of left and right groups */
  leftBox = (STBOX *) palloc0(sizeof(STBOX));
  rightBox = (STBOX *) palloc0(sizeof(STBOX));

  /*
   * Allocate an array for "common entries" - entries which can be placed to
   * either group without affecting overlap along selected axis.
   */
  commonEntriesCount = 0;
  commonEntries = (CommonEntry *) palloc(nentries * sizeof(CommonEntry));

  /* Helper macros to place an entry in the left or right group */
#define PLACE_LEFT(box, off)          \
  do {                    \
    if (v->spl_nleft > 0)          \
      stbox_adjust(leftBox, box);      \
    else                  \
      *leftBox = *(box);          \
    v->spl_left[v->spl_nleft++] = off;    \
  } while (0)

#define PLACE_RIGHT(box, off)          \
  do {                    \
    if (v->spl_nright > 0)          \
      stbox_adjust(rightBox, box);    \
    else                  \
      *rightBox = *(box);          \
    v->spl_right[v->spl_nright++] = off;  \
  } while (0)

  /*
   * Distribute entries which can be distributed unambiguously, and collect
   * common entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    double    lower,
          upper;

    /*
     * Get upper and lower bounds along selected axis.
     */
    box = (STBOX *)DatumGetPointer(entryvec->vector[i].key);
    if (context.dim == 0)
    {
      lower = box->xmin;
      upper = box->xmax;
    }
    else if (context.dim == 1)
    {
      lower = box->ymin;
      upper = box->ymax;
    }
    else if (context.dim == 2 && hasz)
    {
      lower = box->zmin;
      upper = box->zmax;
    }
    else
    {
      lower = box->tmin;
      upper = box->tmax;
    }

    if (FLOAT8_LE(upper, context.leftUpper))
    {
      /* Fits to the left group */
      if (FLOAT8_GE(lower, context.rightLower))
      {
        /* Fits also to the right group, so "common entry" */
        commonEntries[commonEntriesCount++].index = i;
      }
      else
      {
        /* Doesn't fit to the right group, so join to the left group */
        PLACE_LEFT(box, i);
      }
    }
    else
    {
      /*
       * Each entry should fit on either left or right group. Since this
       * entry didn't fit on the left group, it better fit in the right
       * group.
       */
      assert(FLOAT8_GE(lower, context.rightLower));

      /* Doesn't fit to the left group, so join to the right group */
      PLACE_RIGHT(box, i);
    }
  }

  /*
   * Distribute "common entries", if any.
   */
  if (commonEntriesCount > 0)
  {
    /*
     * Calculate minimum number of entries that must be placed in both
     * groups, to reach LIMIT_RATIO.
     */
    int      m = (int) ceil(LIMIT_RATIO * (double) nentries);

    /*
     * Calculate delta between penalties of join "common entries" to
     * different groups.
     */
    for (i = 0; i < commonEntriesCount; i++)
    {
      box = (STBOX *)DatumGetPointer(entryvec->vector[commonEntries[i].index].key);
      commonEntries[i].delta = Abs(stbox_penalty(leftBox, box) -
                     stbox_penalty(rightBox, box));
    }

    /*
     * Sort "common entries" by calculated deltas in order to distribute
     * the most ambiguous entries first.
     */
    qsort(commonEntries, (size_t) commonEntriesCount, sizeof(CommonEntry), common_entry_cmp);

    /*
     * Distribute "common entries" between groups.
     */
    for (i = 0; i < commonEntriesCount; i++)
    {
      box = (STBOX *)DatumGetPointer(entryvec->vector[commonEntries[i].index].key);

      /*
       * Check if we have to place this entry in either group to achieve
       * LIMIT_RATIO.
       */
      if (v->spl_nleft + (commonEntriesCount - i) <= m)
        PLACE_LEFT(box, commonEntries[i].index);
      else if (v->spl_nright + (commonEntriesCount - i) <= m)
        PLACE_RIGHT(box, commonEntries[i].index);
      else
      {
        /* Otherwise select the group by minimal penalty */
        if (stbox_penalty(leftBox, box) < stbox_penalty(rightBox, box))
          PLACE_LEFT(box, commonEntries[i].index);
        else
          PLACE_RIGHT(box, commonEntries[i].index);
      }
    }
  }

  v->spl_ldatum = PointerGetDatum(leftBox);
  v->spl_rdatum = PointerGetDatum(rightBox);
  PG_RETURN_POINTER(v);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_gist_same);
/**
 * GiST same method for temporal points.
 *
 * Return true only when boxes are exactly the same.  We can't use fuzzy
 * comparisons here without breaking index consistency; therefore, this isn't
 * equivalent to stbox_same().
 */
PGDLLEXPORT Datum
Stbox_gist_same(PG_FUNCTION_ARGS)
{
  STBOX *b1 = (STBOX *)DatumGetPointer(PG_GETARG_DATUM(0));
  STBOX *b2 = (STBOX *)DatumGetPointer(PG_GETARG_DATUM(1));
  bool* result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    *result = (FLOAT8_EQ(b1->xmin, b2->xmin) &&
           FLOAT8_EQ(b1->ymin, b2->ymin) &&
           FLOAT8_EQ(b1->zmin, b2->zmin) &&
           float8_cmp_internal(b1->tmin, b2->tmin) == 0 &&
           FLOAT8_EQ(b1->xmax, b2->xmax) &&
           FLOAT8_EQ(b1->ymax, b2->ymax) &&
           FLOAT8_EQ(b1->zmax, b2->zmax) &&
           b1->tmax == b2->tmax);
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_gist_distance);
/**
 * GiST support function. Take in a query and an entry and return the "distance"
 * between them.
*/
PGDLLEXPORT Datum
Stbox_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  STBOX *key = (STBOX *) DatumGetPointer(entry->key);
  STBOX query;
  double distance;

  /* The index is lossy for leaf levels */
  if (GIST_LEAF(entry))
    *recheck = true;

  if (key == NULL)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Transform the query into a box */
  if (! tpoint_gist_get_stbox(fcinfo, &query, oid_type(typid)))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves */
  distance = nad_stbox_stbox(key, &query);

  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************/
