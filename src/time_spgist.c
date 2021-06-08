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
 * @file time_spgist.c
 * Quad-tree SP-GiST index for time types.
 *
 * The functions in this file are based on those in the file 
 * `rangetypes_spgist.c`.
 */

#if MOBDB_PGSQL_VERSION >= 110000

#include "time_spgist.h"

#include <assert.h>
#include <access/spgist.h>
#include <utils/timestamp.h>

#include "timetypes.h"
#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "time_gist.h"
#include "temporal_util.h"
#include "tempcache.h"

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spperiod_gist_config);
/**
 * SP-GiST config function for time types
 */
PGDLLEXPORT Datum
spperiod_gist_config(PG_FUNCTION_ARGS)
{
  spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);

  cfg->prefixType = type_oid(T_PERIOD);
  cfg->labelType = VOIDOID;  /* We don't need node labels. */
  cfg->leafType = type_oid(T_PERIOD);
  cfg->canReturnData = false;
  cfg->longValuesOK = false;

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose functions
 *****************************************************************************/

/**
 * Determine which quadrant a 2D-mapped period falls into, relative to the
 * centroid.
 *
 * Quadrants are numbered as follows:
 * @code
 *  4  |  1
 * ----+----
 *  3  |  2
 * @endcode
 * where the lower bound of period is the horizontal axis and upper bound the
 * vertical axis.
 *
 * Periods on one of the axes are taken to lie in the quadrant with higher value
 * along perpendicular axis. That is, a value on the horizontal axis is taken
 * to belong to quadrant 1 or 4, and a value on the vertical axis is taken to
 * belong to quadrant 1 or 2. A period equal to centroid is taken to lie in
 * quadrant 1.
 */
int16
getQuadrant(const Period *centroid, const Period *tst)
{
  PeriodBound centroidLower, centroidUpper, lower, upper;
  period_deserialize(centroid, &centroidLower, &centroidUpper);
  period_deserialize(tst, &lower, &upper);

  if (period_cmp_bounds(&lower, &centroidLower) >= 0)
  {
    if (period_cmp_bounds(&upper, &centroidUpper) >= 0)
      return 1;
    else
      return 2;
  }
  else
  {
    if (period_cmp_bounds(&upper, &centroidUpper) >= 0)
      return 4;
    else
      return 3;
  }
}

PG_FUNCTION_INFO_V1(spperiod_gist_choose);
/**
 * SP-GiST choose function for time types
 */
PGDLLEXPORT Datum
spperiod_gist_choose(PG_FUNCTION_ARGS)
{
  spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
  spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
  Period *period = DatumGetPeriod(in->leafDatum),
    *centroid;
  int16 quadrant;

  if (in->allTheSame)
  {
    out->resultType = spgMatchNode;
    /* nodeN will be set by core */
    out->result.matchNode.levelAdd = 0;
    out->result.matchNode.restDatum = PeriodGetDatum(period);
    PG_RETURN_VOID();
  }

  centroid = DatumGetPeriod(in->prefixDatum);
  quadrant = getQuadrant(centroid, period);

  assert(quadrant <= in->nNodes);

  /* Select node matching to quadrant number */
  out->resultType = spgMatchNode;
  out->result.matchNode.nodeN = quadrant - 1;
  out->result.matchNode.levelAdd = 1;
  out->result.matchNode.restDatum = PeriodGetDatum(period);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spperiod_gist_picksplit);
/**
 * SP-GiST pick-split function for time types
 *
 * It splits a list of time types into quadrants by choosing a central 4D
 * point as the median of the coordinates of the time types.
 */
PGDLLEXPORT Datum
spperiod_gist_picksplit(PG_FUNCTION_ARGS)
{
  spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
  spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
  Period *centroid;
  int median, i;
  /* Use the median values of lower and upper bounds as the centroid period */
  PeriodBound *lowerBounds = palloc(sizeof(PeriodBound) * in->nTuples),
    *upperBounds = palloc(sizeof(PeriodBound) * in->nTuples);

  /* Construct "centroid" period from medians of lower and upper bounds */
  for (i = 0; i < in->nTuples; i++)
    period_deserialize(DatumGetPeriod(in->datums[i]),
      &lowerBounds[i], &upperBounds[i]);

  qsort(lowerBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_qsort_cmp);
  qsort(upperBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_qsort_cmp);

  median = in->nTuples / 2;

  centroid = period_make(lowerBounds[median].t, upperBounds[median].t,
    lowerBounds[median].inclusive, upperBounds[median].inclusive);

  /* Fill the output */
  out->hasPrefix = true;
  out->prefixDatum = PeriodGetDatum(centroid);

  out->nNodes = 4;
  out->nodeLabels = NULL;    /* we don't need node labels */

  out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
  out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

  /*
   * Assign periods to corresponding nodes according to quadrants relative to
   * "centroid" period.
   */
  for (i = 0; i < in->nTuples; i++)
  {
    Period *period = DatumGetPeriod(in->datums[i]);
    int16 quadrant = getQuadrant(centroid, period);

    out->leafTupleDatums[i] = PeriodGetDatum(period);
    out->mapTuplesToNodes[i] = quadrant - 1;
  }

  pfree(lowerBounds); pfree(upperBounds);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions
 *****************************************************************************/

/**
 * Check if two bounds A and B are adjacent, where A is an upper bound and B
 * is a lower bound.
 */
bool
period_bounds_adjacent(const PeriodBound *boundA, const PeriodBound *boundB)
{
  assert(!boundA->lower && boundB->lower);
  return timestamp_cmp_internal(boundA->t, boundB->t) == 0 &&
    boundA->inclusive != boundB->inclusive;
}

/**
 * Given an argument and centroid bound, this function determines if any
 * bounds that are adjacent to the argument are smaller than, or greater than
 * or equal to centroid. For brevity, we call the arg < centroid "left", and
 * arg >= centroid case "right". This corresponds to how the quadrants are
 * arranged, if you imagine that "left" is equivalent to "down" and "right"
 * is equivalent to "up".
 *
 * For the "left" case, returns -1, and for the "right" case, returns 1.
 */
static int
adjacent_cmp_bounds(const PeriodBound *arg, const PeriodBound *centroid)
{
  int cmp;

  assert(arg->lower != centroid->lower);

  cmp = period_cmp_bounds(arg, centroid);

  if (centroid->lower)
  {
    /*------
     * The argument is an upper bound, we are searching for adjacent lower
     * bounds. A matching adjacent lower bound must be *larger* than the
     * argument, but only just.
     *
     * The following table illustrates the desired result with a fixed
     * argument bound, and different centroids. The CMP column shows
     * the value of 'cmp' variable, and ADJ shows whether the argument
     * and centroid are adjacent, per bounds_adjacent(). (N) means we
     * don't need to check for that case, because it's implied by CMP.
     * With the argument range [..., 500), the adjacent range we're
     * searching for is [500, ...):
     *
     *  ARGUMENT   CENTROID    CMP   ADJ
     *  [..., 500) [498, ...)   >    (N)  [500, ...) is to the right
     *  [..., 500) [499, ...)   =    (N)  [500, ...) is to the right
     *  [..., 500) [500, ...)   <     Y  [500, ...) is to the right
     *  [..., 500) [501, ...)   <     N  [500, ...) is to the left
     *
     * So, we must search left when the argument is smaller than, and not
     * adjacent, to the centroid. Otherwise search right.
     *------
     */
    if (cmp < 0 && ! period_bounds_adjacent(arg, centroid))
      return -1;
    else
      return 1;
  }
  else
  {
    /*------
     * The argument is a lower bound, we are searching for adjacent upper
     * bounds. A matching adjacent upper bound must be *smaller* than the
     * argument, but only just.
     *
     *  ARGUMENT   CENTROID    CMP   ADJ
     *  [500, ...) [..., 499)   >    (N)  [..., 500) is to the right
     *  [500, ...) [..., 500)   >    (Y)  [..., 500) is to the right
     *  [500, ...) [..., 501)   =    (N)  [..., 500) is to the left
     *  [500, ...) [..., 502)   <    (N)  [..., 500) is to the left
     *
     * We must search left when the argument is smaller than or equal to
     * the centroid. Otherwise search right. We don't need to check
     * whether the argument is adjacent with the centroid, because it
     * doesn't matter.
     *------
     */
    if (cmp <= 0)
      return -1;
    else
      return 1;
  }
}

/**
 * Like adjacent_cmp_bounds, but also takes into account the previous
 * level's centroid. We might've traversed left (or right) at the previous
 * node, in search for ranges adjacent to the other bound, even though we
 * already ruled out the possibility for any matches in that direction for
 * this bound. By comparing the argument with the previous centroid, and
 * the previous centroid with the current centroid, we can determine which
 * direction we should've moved in at previous level, and which direction we
 * actually moved.
 *
 * If there can be any matches to the left, returns -1. If to the right,
 * returns 1. If there can be no matches below this centroid, because we
 * already ruled them out at the previous level, returns 0.
 *
 * XXX: Comparing just the previous and current level isn't foolproof; we
 * might still search some branches unnecessarily. For example, imagine that
 * we are searching for value 15, and we traverse the following centroids
 * (only considering one bound for the moment):
 *
 * Level 1: 20
 * Level 2: 50
 * Level 3: 25
 *
 * At this point, previous centroid is 50, current centroid is 25, and the
 * target value is to the left. But because we already moved right from
 * centroid 20 to 50 in the first level, there cannot be any values < 20 in
 * the current branch. But we don't know that just by looking at the previous
 * and current centroid, so we traverse left, unnecessarily. The reason we are
 * down this branch is that we're searching for matches with the *other*
 * bound. If we kept track of which bound we are searching for explicitly,
 * instead of deducing that from the previous and current centroid, we could
 * avoid some unnecessary work.
 */
static int
adjacent_inner_consistent(PeriodBound *arg, PeriodBound *centroid,
  PeriodBound *prev)
{
  if (prev)
  {
    int prevcmp;
    int cmp;

    /*
     * Which direction were we supposed to traverse at previous level,
     * left or right?
     */
    prevcmp = adjacent_cmp_bounds(arg, prev);

    /* and which direction did we actually go? */
    cmp = period_cmp_bounds(centroid, prev);

    /* if the two don't agree, there's nothing to see here */
    if ((prevcmp < 0 && cmp >= 0) || (prevcmp > 0 && cmp < 0))
      return 0;
  }

  return adjacent_cmp_bounds(arg, centroid);
}

PG_FUNCTION_INFO_V1(spperiod_gist_inner_consistent);
/**
 * SP-GiST inner consistent function function for time types
 */
PGDLLEXPORT Datum
spperiod_gist_inner_consistent(PG_FUNCTION_ARGS)
{
  spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
  spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
  int which, i;
  Period *centroid;
  PeriodBound centroidLower, centroidUpper;
  MemoryContext oldCtx;

  /*
   * For adjacent search we need also previous centroid (if any) to improve
   * the precision of the consistent check. In this case needPrevious flag
   * is set and centroid is passed into traversalValue.
   */
  bool needPrevious = false;

  if (in->allTheSame)
  {
    /* Report that all nodes should be visited */
    out->nNodes = in->nNodes;
    out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
    for (i = 0; i < in->nNodes; i++)
      out->nodeNumbers[i] = i;
    PG_RETURN_VOID();
  }

  /* Fetch the centroid of this node. */
  centroid = DatumGetPeriod(in->prefixDatum);
  period_deserialize(centroid, &centroidLower, &centroidUpper);

  assert(in->nNodes == 4);

  /*
   * Nth bit of which variable means that (N - 1)th node (Nth quadrant)
   * should be visited. Initially all bits are set. Bits of nodes which
   * can be skipped will be unset.
   */
  which = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);

  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    Oid subtype = in->scankeys[i].sk_subtype;
    PeriodBound  lower, upper;
    Period period;
    const Period *query = NULL;
    Period *prevCentroid = NULL;
    PeriodBound prevLower, prevUpper;

    /* Restrictions on period bounds according to scan strategy */
    PeriodBound *minLower = NULL, *maxLower = NULL,
      *minUpper = NULL, *maxUpper = NULL;

    /* Are the restrictions on period bounds inclusive? */
    bool inclusive = true;
    int cmp, which1, which2;

    /*
     * Cast the query to Period for ease of the following operations.
     */
    if (subtype == TIMESTAMPTZOID)
    {
      TimestampTz t = DatumGetTimestampTz(in->scankeys[i].sk_argument);
      period_set(&period, t, t, true, true);
      query = &period;
    }
    else if (subtype == type_oid(T_TIMESTAMPSET))
      query = timestampset_bbox_ptr(
        DatumGetTimestampSet(in->scankeys[i].sk_argument));
    else if (subtype == type_oid(T_PERIOD))
      query = DatumGetPeriod(in->scankeys[i].sk_argument);
    else if (subtype == type_oid(T_PERIODSET))
      query = periodset_bbox_ptr(
        DatumGetPeriodSet(in->scankeys[i].sk_argument));
    /* For temporal types whose bounding box is a period */
    else if (temporal_type(subtype))
    {
      temporal_bbox(&period,
        DatumGetTemporal(in->scankeys[i].sk_argument));
      query = &period;
    }
    else
      elog(ERROR, "Unrecognized strategy number: %d", strategy);

    period_deserialize(query, &lower, &upper);

    /*
     * Most strategies are handled by forming a bounding box from the
     * search key, defined by a minLower, maxLower, minUpper,
     * maxUpper. Some modify 'which' directly, to specify exactly
     * which quadrants need to be visited.
     */
    switch (strategy)
    {
      case RTOverlapStrategyNumber:
        /*
         * Periods overlap, if lower bound of each period
         * is lower or equal to upper bound of the other period.
         */
        maxLower = &upper;
        minUpper = &lower;
        break;

      case RTContainsStrategyNumber:
        /*
         * Period A contains period B if lower
         * bound of A is lower or equal to lower bound of period B
         * and upper bound of period A is greater or equal to upper
         * bound of period A.
         */
        which &= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
        maxLower = &lower;
        minUpper = &upper;
        break;

      case RTContainedByStrategyNumber:
        /* The opposite of contains. */
        minLower = &lower;
        maxUpper = &upper;
        break;

      case RTAdjacentStrategyNumber:
        /*
         * Previously selected quadrant could exclude possibility
         * for lower or upper bounds to be adjacent. Deserialize
         * previous centroid range if present for checking this.
         */
        if (in->traversalValue)
        {
          prevCentroid = DatumGetPeriod(in->traversalValue);
          period_deserialize(prevCentroid, &prevLower,
            &prevUpper);
        }

        /*
         * For a range's upper bound to be adjacent to the
         * argument's lower bound, it will be found along the line
         * adjacent to (and just below) Y=lower. Therefore, if the
         * argument's lower bound is less than the centroid's
         * upper bound, the line falls in quadrants 2 and 3; if
         * greater, the line falls in quadrants 1 and 4. (see
         * adjacent_cmp_bounds for description of edge cases).
         */
        cmp = adjacent_inner_consistent(&lower, &centroidUpper,
          prevCentroid ? &prevUpper : NULL);
        if (cmp > 0)
          which1 = (1 << 1) | (1 << 4);
        else if (cmp < 0)
          which1 = (1 << 2) | (1 << 3);
        else
          which1 = 0;

        /*
         * Also search for ranges's adjacent to argument's upper
         * bound. They will be found along the line adjacent to
         * (and just right of) X=upper, which falls in quadrants 3
         * and 4, or 1 and 2.
         */
        cmp = adjacent_inner_consistent(&upper, &centroidLower,
          prevCentroid ? &prevLower : NULL);
        if (cmp > 0)
          which2 = (1 << 1) | (1 << 2);
        else if (cmp < 0)
          which2 = (1 << 3) | (1 << 4);
        else
          which2 = 0;

        /* We must chase down ranges adjacent to either bound. */
        which &= which1 | which2;

        needPrevious = true;
        break;

      case RTEqualStrategyNumber:
      case RTSameStrategyNumber:
        /*
         * Equal period can be only in the same quadrant where
         * argument would be placed to.
         */
        which &= (1 << getQuadrant(centroid, query));
        break;

      case RTBeforeStrategyNumber:
        /*
         * Period A is before period B if upper bound of A is lower
         * than lower bound of B.
         */
        maxUpper = &lower;
        inclusive = false;
        break;

      case RTOverBeforeStrategyNumber:
        /*
         * Period A is overbefore to period B if upper bound of A is
         * less or equal to upper bound of B.
         */
        maxUpper = &upper;
        break;

      case RTAfterStrategyNumber:
        /*
         * Period A is after period B if lower bound of A is greater
         * than upper bound of B.
         */
        minLower = &upper;
        inclusive = false;
        break;

      case RTOverAfterStrategyNumber:
        /*
         * Period A is overafter to period B if lower bound of A is
         * greater or equal to lower bound of B.
         */
        minLower = &lower;
        break;

      default:
        elog(ERROR, "unrecognized strategy: %d", strategy);
    }

    /*
     * Using the bounding box, see which quadrants we have to descend
     * into.
     */
    if (minLower)
    {
      /*
       * If the centroid's lower bound is less than or equal to the
       * minimum lower bound, anything in the 3rd and 4th quadrants
       * will have an even smaller lower bound, and thus can't
       * match.
       */
      if (period_cmp_bounds(&centroidLower, minLower) <= 0)
        which &= (1 << 1) | (1 << 2);
    }
    if (maxLower)
    {
      /*
       * If the centroid's lower bound is greater than the maximum
       * lower bound, anything in the 1st and 2nd quadrants will
       * also have a greater than or equal lower bound, and thus
       * can't match. If the centroid's lower bound is equal to the
       * maximum lower bound, we can still exclude the 1st and 2nd
       * quadrants if we're looking for a value strictly greater
       * than the maximum.
       */
      int cmp;

      cmp = period_cmp_bounds(&centroidLower, maxLower);
      if (cmp > 0 || (!inclusive && cmp == 0))
        which &= (1 << 3) | (1 << 4);
    }
    if (minUpper)
    {
      /*
       * If the centroid's upper bound is less than or equal to the
       * minimum upper bound, anything in the 2nd and 3rd quadrants
       * will have an even smaller upper bound, and thus can't
       * match.
       */
      if (period_cmp_bounds(&centroidUpper, minUpper) <= 0)
        which &= (1 << 1) | (1 << 4);
    }
    if (maxUpper)
    {
      /*
       * If the centroid's upper bound is greater than the maximum
       * upper bound, anything in the 1st and 4th quadrants will
       * also have a greater than or equal upper bound, and thus
       * can't match. If the centroid's upper bound is equal to the
       * maximum upper bound, we can still exclude the 1st and 4th
       * quadrants if we're looking for a value strictly greater
       * than the maximum.
       */
      int      cmp;

      cmp = period_cmp_bounds(&centroidUpper, maxUpper);
      if (cmp > 0 || (!inclusive && cmp == 0))
        which &= (1 << 2) | (1 << 3);
    }

    if (which == 0)
      break;      /* no need to consider remaining conditions */
  }

  /* We must descend into the quadrant(s) identified by 'which' */
  out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
  if (needPrevious)
    out->traversalValues = (void **) palloc(sizeof(void *) * in->nNodes);
  out->nNodes = 0;

  /*
   * Elements of traversalValues should be allocated in
   * traversalMemoryContext
   */
  oldCtx = MemoryContextSwitchTo(in->traversalMemoryContext);

  for (i = 1; i <= in->nNodes; i++)
  {
    if (which & (1 << i))
    {
      /* Save previous prefix if needed */
      if (needPrevious)
      {
        /* We know that in->prefixDatum in this place is a period */
        Datum previousCentroid = PointerGetDatum(period_copy(
          (Period *) DatumGetPointer(in->prefixDatum)));
        out->traversalValues[out->nNodes] = (void *) previousCentroid;
      }
      out->nodeNumbers[out->nNodes] = i - 1;
      out->nNodes++;
    }
  }

  MemoryContextSwitchTo(oldCtx);

  PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spperiod_gist_leaf_consistent);
/**
 * SP-GiST leaf-level consistency function for time types
 */
PGDLLEXPORT Datum
spperiod_gist_leaf_consistent(PG_FUNCTION_ARGS)
{
  spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
  spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
  Period *key = DatumGetPeriod(in->leafDatum);
  bool res = true;
  int i;

  /* Initialization so that all the tests are exact for time types. */
  out->recheck = false;

  /* leafDatum is what it is... */
  out->leafValue = in->leafDatum;

  /* Perform the required comparison(s) */
  for (i = 0; i < in->nkeys; i++)
  {
    StrategyNumber strategy = in->scankeys[i].sk_strategy;
    Period period;
    const Period *query;
    Oid subtype = in->scankeys[i].sk_subtype;

    /* Update the recheck flag according to the strategy */
    out->recheck |= period_index_recheck(strategy);

    if (in->scankeys[i].sk_subtype == TIMESTAMPTZOID)
    {
      TimestampTz t = DatumGetTimestampTz(in->scankeys[i].sk_argument);
      period_set(&period, t, t, true, true);
      query = &period;
    }
    else if (in->scankeys[i].sk_subtype == type_oid(T_TIMESTAMPSET))
      query = timestampset_bbox_ptr(
        DatumGetTimestampSet(in->scankeys[i].sk_argument));
    else if (in->scankeys[i].sk_subtype == type_oid(T_PERIOD))
      query = DatumGetPeriod(in->scankeys[i].sk_argument);
    else if (in->scankeys[i].sk_subtype ==  type_oid(T_PERIODSET))
      query = periodset_bbox_ptr(
        DatumGetPeriodSet(in->scankeys[i].sk_argument));
    /* For temporal types whose bounding box is a period */
    else if (temporal_type(subtype))
    {
      /* All tests are lossy for temporal types */
      out->recheck = true;
      temporal_bbox(&period,
        DatumGetTemporal(in->scankeys[i].sk_argument));
      query = &period;
    }
    else
      elog(ERROR, "Unrecognized strategy number: %d", strategy);

    res = period_index_consistent_leaf(key, query, strategy);

    /* If any check is failed, we have found our answer. */
    if (!res)
      break;
  }

  PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(sptimestampset_gist_compress);
/**
 * SP-GiST compress function for timestamp sets
 */
PGDLLEXPORT Datum
sptimestampset_gist_compress(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
  Period *period = palloc0(sizeof(Period));
  period = period_copy(timestampset_bbox_ptr(ts));
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_PERIOD(period);
}

PG_FUNCTION_INFO_V1(spperiodset_gist_compress);
/**
 * SP-GiST compress function for period sets
 */
PGDLLEXPORT Datum
spperiodset_gist_compress(PG_FUNCTION_ARGS)
{
  PeriodSet *ps = PG_GETARG_PERIODSET(0);
  Period *period = palloc0(sizeof(Period));
  period = period_copy(periodset_bbox_ptr(ps));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_PERIOD(period);
}
#endif

/*****************************************************************************/
