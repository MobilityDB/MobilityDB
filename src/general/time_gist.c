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
 * @file time_gist.c
 * @brief R-tree GiST index for time types.
 *
 * These functions are based on those in the file `rangetypes_gist.c`.
 */

#include "general/time_gist.h"

/* PostgreSQL */
#include <assert.h>
#include <access/gist.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/timetypes.h"
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * Leaf-level consistency for time types.
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
period_index_consistent_leaf(const Period *key, const Period *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      return overlaps_period_period(key, query);
    case RTContainsStrategyNumber:
      return contains_period_period(key, query);
    case RTContainedByStrategyNumber:
      return contains_period_period(query, key);
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return period_eq(key, query);
    case RTAdjacentStrategyNumber:
      return adjacent_period_period(key, query);
    case RTBeforeStrategyNumber:
      return before_period_period(key, query);
    case RTOverBeforeStrategyNumber:
      return overbefore_period_period(key, query);
    case RTAfterStrategyNumber:
      return after_period_period(key, query);
    case RTOverAfterStrategyNumber:
      return overafter_period_period(key, query);
    default:
      elog(ERROR, "unrecognized period strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * GiST internal-page consistency for time types
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
bool
period_gist_consistent(const Period *key, const Period *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      return overlaps_period_period(key, query);
    case RTContainsStrategyNumber:
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return contains_period_period(key, query);
    case RTAdjacentStrategyNumber:
      return adjacent_period_period(key, query) ||
        overlaps_period_period(key, query);
    case RTBeforeStrategyNumber:
      return !overafter_period_period(key, query);
    case RTOverBeforeStrategyNumber:
      return !after_period_period(key, query);
    case RTAfterStrategyNumber:
      return !overbefore_period_period(key, query);
    case RTOverAfterStrategyNumber:
      return !before_period_period(key, query);
    default:
      elog(ERROR, "unrecognized period strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * Return true if a recheck is necessary depending on the strategy
 */
bool
period_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes */
  if (strategy == RTBeforeStrategyNumber ||
    strategy == RTOverBeforeStrategyNumber ||
    strategy == RTAfterStrategyNumber ||
    strategy == RTOverAfterStrategyNumber)
    return false;
  return true;
}

/**
 * Transform the query argument into a period
 */
static bool
time_gist_get_period(FunctionCallInfo fcinfo, Period *result, Oid typid)
{
  CachedType type = oid_type(typid);
  if (type == T_TIMESTAMPTZ)
  {
    /* Since function period_gist_consistent is strict, t is not NULL */
    TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
    period_set(t, t, true, true, result);
  }
  else if (type == T_TIMESTAMPSET)
  {
    Datum tsdatum = PG_GETARG_DATUM(1);
    timestampset_bbox_slice(tsdatum, result);
  }
  else if (type == T_PERIOD)
  {
    Period *p = PG_GETARG_PERIOD_P(1);
    if (p == NULL)
      PG_RETURN_BOOL(false);
    memcpy(result, p, sizeof(Period));
  }
  else if (type == T_PERIODSET)
  {
    Datum psdatum = PG_GETARG_DATUM(1);
    periodset_bbox_slice(psdatum, result);
  }
  /* For temporal types whose bounding box is a period */
  else if (temporal_type(type))
  {
    Datum tempdatum = PG_GETARG_DATUM(1);
    temporal_bbox_slice(tempdatum, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

PG_FUNCTION_INFO_V1(Period_gist_consistent);
/**
 * GiST consistent method for time types and alpha temporal types whose
 * bounding box is a period
 */
PGDLLEXPORT Datum
Period_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  bool result;
  const Period *key = DatumGetPeriodP(entry->key);
  Period query;

  /* Determine whether the operator is exact */
  *recheck = period_index_recheck(strategy);

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! time_gist_get_period(fcinfo, &query, typid))
    PG_RETURN_BOOL(false);

  if (GIST_LEAF(entry))
    result = period_index_consistent_leaf(key, &query, strategy);
  else
    result = period_gist_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_gist_union);
/**
 * GiST union method for time types
 */
PGDLLEXPORT Datum
Period_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  Period *result = period_copy(DatumGetPeriodP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    period_expand(DatumGetPeriodP(ent[i].key), result);
  PG_RETURN_PERIOD_P(result);
}

/*****************************************************************************
 * GiST compress methods
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_gist_compress);
/**
 * GiST compress method for timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
    Period *period = (Period *) palloc(sizeof(Period));
    timestampset_bbox_slice(entry->key, period);
    gistentryinit(*retval, PointerGetDatum(period), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

PG_FUNCTION_INFO_V1(Period_gist_compress);
/**
 * GiST compress method for periods
 */
PGDLLEXPORT Datum
Period_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
    gistentryinit(*retval, entry->key, entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

PG_FUNCTION_INFO_V1(Periodset_gist_compress);
/**
 * GiST compress method for period sets
 */
PGDLLEXPORT Datum
Periodset_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = (GISTENTRY *) palloc(sizeof(GISTENTRY));
    Period *period = (Period *) palloc(sizeof(Period));
    periodset_bbox_slice(entry->key, period);
    gistentryinit(*retval, PointerGetDatum(period),
      entry->rel, entry->page, entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST penalty method for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_gist_penalty);
/**
 * GiST page split penalty function for periods.
 *
 * The penalty function has the following goals (in order from most to least
 * important):
 * - Avoid broadening (as determined by period_to_secs) the original predicate
 * - Favor adding periods to narrower original predicates
 */
PGDLLEXPORT Datum
Period_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *penalty = (float *) PG_GETARG_POINTER(2);
  const Period *orig = DatumGetPeriodP(origentry->key);
  const Period *new = DatumGetPeriodP(newentry->key);
  PeriodBound orig_lower, new_lower, orig_upper, new_upper;
  period_deserialize(orig, &orig_lower, &orig_upper);
  period_deserialize(new, &new_lower, &new_upper);

  /* Calculate extension of original period by calling period_to_secs */
  float8 diff = 0.0;
  if (period_bound_cmp(&new_lower, &orig_lower) < 0)
    diff += period_to_secs(orig->lower, new->lower);
  if (period_bound_cmp(&new_upper, &orig_upper) > 0)
    diff += period_to_secs(new->upper, orig->upper);
  *penalty = (float4) diff;

  PG_RETURN_POINTER(penalty);
}

/*****************************************************************************
 * GiST picksplit method for time types
 *****************************************************************************/

/* Helper macros to place an entry in the left or right group during split */
/* Note direct access to variables v, left_period, right_period */
#define PLACE_LEFT(period, off)          \
  do {                    \
    if (v->spl_nleft > 0)          \
      left_period = period_super_union(left_period, period); \
    else                  \
      left_period = (period);        \
    v->spl_left[v->spl_nleft++] = (off);  \
  } while (0)

#define PLACE_RIGHT(period, off)        \
  do {                    \
    if (v->spl_nright > 0)          \
      right_period = period_super_union(right_period, period); \
    else                  \
      right_period = (period);      \
    v->spl_right[v->spl_nright++] = (off);  \
  } while (0)

/**
 * Trivial split: half of entries will be placed on one page
 * and the other half on the other page.
 */
static void
period_gist_fallback_split(GistEntryVector *entryvec, GIST_SPLITVEC *v)
{
  Period *left_period = NULL, *right_period = NULL;
  OffsetNumber i, maxoff, split_idx;

  maxoff = (OffsetNumber) (entryvec->n - 1);
  /* Split entries before this to left page, after to right: */
  split_idx = (OffsetNumber) ((maxoff - FirstOffsetNumber) / 2 + FirstOffsetNumber);

  v->spl_nleft = 0;
  v->spl_nright = 0;
  for (i = FirstOffsetNumber; i <= maxoff; i++)
  {
    Period *period = DatumGetPeriodP(entryvec->vector[i].key);

    if (i < split_idx)
      PLACE_LEFT(period, i);
    else
      PLACE_RIGHT(period, i);
  }

  v->spl_ldatum = PeriodPGetDatum(left_period);
  v->spl_rdatum = PeriodPGetDatum(right_period);
  return;
}

/**
 * Structure keeping context for the function period_gist_consider_split
 */
typedef struct
{
  int entries_count;  /**< total number of entries being split */
  /** Information about currently selected split follows */
  bool first;         /**< true if no split was selected yet */
  PeriodBound left_upper;  /**< upper bound of left interval */
  PeriodBound right_lower; /**< lower bound of right interval */
  float4 ratio;      /**< split ratio */
  float4 overlap;    /**< overlap between left and right predicate */
  int common_left;   /**< number of common entries destined for each side */
  int common_right;
} ConsiderSplitContext;

/**
 * Consider replacement of currently selected split with a better one
 * during period_gist_double_sorting_split.
 */
static void
period_gist_consider_split(ConsiderSplitContext *context,
  PeriodBound *right_lower, int min_left_count,
  PeriodBound *left_upper, int max_left_count)
{
  int left_count, right_count;
  float4 ratio, overlap;

  /*
   * Calculate entries distribution ratio assuming most uniform distribution
   * of common entries.
   */
  if (min_left_count >= (context->entries_count + 1) / 2)
    left_count = min_left_count;
  else if (max_left_count <= context->entries_count / 2)
    left_count = max_left_count;
  else
    left_count = context->entries_count / 2;
  right_count = context->entries_count - left_count;

  /*
   * Ratio of split: quotient between size of smaller group and total
   * entries count. This is necessarily 0.5 or less; if it's less than
   * LIMIT_RATIO then we will never accept the new split.
   */
  ratio = ((float4) Min(left_count, right_count)) /
    ((float4) context->entries_count);

  if (ratio > LIMIT_RATIO)
  {
    bool selectthis = false;

    /*
     * The ratio is acceptable, so compare current split with previously
     * selected one. We search for minimal overlap (allowing negative
     * values) and minimal ratio secondarily.  The subtype_diff is
     * used for overlap measure.
     */
    overlap = (float4) period_to_secs(left_upper->t, right_lower->t);

    /* If there is no previous selection, select this split */
    if (context->first)
      selectthis = true;
    else
    {
      /*
       * Choose the new split if it has a smaller overlap, or same
       * overlap but better ratio.
       */
      if (overlap < context->overlap ||
        (overlap == context->overlap && ratio > context->ratio))
        selectthis = true;
    }

    if (selectthis)
    {
      /* save information about selected split */
      context->first = false;
      context->ratio = ratio;
      context->overlap = overlap;
      context->right_lower = *right_lower;
      context->left_upper = *left_upper;
      context->common_left = max_left_count - left_count;
      context->common_right = left_count - min_left_count;
    }
  }
  return;
}

/**
 * Structure keeping the bounds extracted from a period, for use in the
 * function period_gist_double_sorting_split
 */
typedef struct
{
  PeriodBound lower;
  PeriodBound upper;
} PeriodBounds;

/**
 * Compare PeriodBounds by lower bound.
 */
static int
periodbounds_cmp_lower(const void *a, const void *b)
{
  PeriodBounds *i1 = (PeriodBounds *) a;
  PeriodBounds *i2 = (PeriodBounds *) b;
  return period_bound_cmp(&i1->lower, &i2->lower);
}

/**
 * Compare PeriodBounds by upper bound.
 */
static int
periodbounds_cmp_upper(const void *a, const void *b)
{
  PeriodBounds *i1 = (PeriodBounds *) a;
  PeriodBounds *i2 = (PeriodBounds *) b;
  return period_bound_cmp(&i1->upper, &i2->upper);
}

/**
 * Compare CommonEntrys by their deltas.
 * (We assume the deltas can't be NaN.)
 */
int
common_entry_cmp(const void *i1, const void *i2)
{
  double delta1 = ((CommonEntry *) i1)->delta;
  double delta2 = ((CommonEntry *) i2)->delta;
  if (delta1 < delta2)
    return -1;
  else if (delta1 > delta2)
    return 1;
  else
    return 0;
}

/**
 * Double sorting split algorithm.
 *
 * The algorithm considers dividing periods into two groups. The first (left)
 * group contains general left bound. The second (right) group contains
 * general right bound. The challenge is to find upper bound of left group
 * and lower bound of right group so that overlap of groups is minimal and
 * ratio of distribution is acceptable. Algorithm finds for each lower bound of
 * right group minimal upper bound of left group, and for each upper bound of
 * left group maximal lower bound of right group. For each found pair
 * period_gist_consider_split considers replacement of currently selected
 * split with the new one.
 *
 * After that, all the entries are divided into three groups:
 * 1) Entries which should be placed to the left group
 * 2) Entries which should be placed to the right group
 * 3) "Common entries" which can be placed to either group without affecting
 *    amount of overlap.
 *
 * The common periods are distributed by difference of distance from lower
 * bound of common period to lower bound of right group and distance from upper
 * bound of common period to upper bound of left group.
 *
 * For details see:
 * "A new double sorting-based node splitting algorithm for R-tree",
 * A. Korotkov
 * http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf#page=36
 */
static void
period_gist_double_sorting_split(GistEntryVector *entryvec, GIST_SPLITVEC *v)
{
  ConsiderSplitContext context;
  OffsetNumber i, maxoff;
  Period *left_period = NULL, *right_period = NULL;
  PeriodBounds *by_lower, *by_upper;
  PeriodBound *right_lower, *left_upper;
  CommonEntry *common_entries;
  int common_entries_count, nentries, i1, i2;

  memset(&context, 0, sizeof(ConsiderSplitContext));

  maxoff = (OffsetNumber) (entryvec->n - 1);
  nentries = context.entries_count = maxoff - FirstOffsetNumber + 1;
  context.first = true;

  /* Allocate arrays for sorted period bounds */
  by_lower = (PeriodBounds *) palloc(nentries * sizeof(PeriodBounds));
  by_upper = (PeriodBounds *) palloc(nentries * sizeof(PeriodBounds));
  /* Fill arrays of bounds */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    Period  *period = DatumGetPeriodP(entryvec->vector[i].key);
    period_deserialize(period, &by_lower[i - FirstOffsetNumber].lower,
      &by_lower[i - FirstOffsetNumber].upper);
  }
  /*
   * Make two arrays of period bounds: one sorted by lower bound and another
   * sorted by upper bound.
   */
  memcpy(by_upper, by_lower, nentries * sizeof(PeriodBounds));
  qsort(by_lower, (size_t) nentries, sizeof(PeriodBounds),
    (qsort_comparator) periodbounds_cmp_lower);
  qsort(by_upper, (size_t) nentries, sizeof(PeriodBounds),
    (qsort_comparator) periodbounds_cmp_upper);

  /*----------
   * The goal is to form a left and right period, so that every entry
   * period is contained by either left or right interval (or both).
   *
   * For example, with the periods (0,1), (1,3), (2,3), (2,4):
   *
   * 0 1 2 3 4
   * +-+
   *   +---+
   *     +-+
   *     +---+
   *
   * The left and right periods are of the form (0,a) and (b,4).
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
   *----------
   */

  /*
   * Iterate over lower bound of right group, finding smallest possible
   * upper bound of left group.
   */
  i1 = 0;
  i2 = 0;
  right_lower = &by_lower[i1].lower;
  left_upper = &by_upper[i2].lower;
  while (true)
  {
    /*
     * Find next lower bound of right group.
     */
    while (i1 < nentries &&
        period_bound_cmp(right_lower, &by_lower[i1].lower) == 0)
    {
      if (period_bound_cmp(&by_lower[i1].upper, left_upper) > 0)
        left_upper = &by_lower[i1].upper;
      i1++;
    }
    if (i1 >= nentries)
      break;
    right_lower = &by_lower[i1].lower;

    /*
     * Find count of periods which anyway should be placed to the left
     * group.
     */
    while (i2 < nentries &&
         period_bound_cmp(&by_upper[i2].upper, left_upper) <= 0)
      i2++;

    /*
     * Consider found split to see if it's better than what we had.
     */
    period_gist_consider_split(&context, right_lower, i1, left_upper, i2);
  }

  /*
   * Iterate over upper bound of left group finding greatest possible lower
   * bound of right group.
   */
  i1 = nentries - 1;
  i2 = nentries - 1;
  right_lower = &by_lower[i1].upper;
  left_upper = &by_upper[i2].upper;
  while (true)
  {
    /*
     * Find next upper bound of left group.
     */
    while (i2 >= 0 && period_bound_cmp(left_upper,
      &by_upper[i2].upper) == 0)
    {
      if (period_bound_cmp(&by_upper[i2].lower,
          right_lower) < 0)
        right_lower = &by_upper[i2].lower;
      i2--;
    }
    if (i2 < 0)
      break;
    left_upper = &by_upper[i2].upper;

    /*
     * Find count of intervals which anyway should be placed to the right
     * group.
     */
    while (i1 >= 0 &&
      period_bound_cmp(&by_lower[i1].lower, right_lower) >= 0)
      i1--;

    /*
     * Consider found split to see if it's better than what we had.
     */
    period_gist_consider_split(&context, right_lower, i1 + 1,
                  left_upper, i2 + 1);
  }

  /*
   * If we failed to find any acceptable splits, use trivial split.
   */
  if (context.first)
  {
    period_gist_fallback_split(entryvec, v);
    return;
  }

  /*
   * Ok, we have now selected bounds of the groups. Now we have to
   * distribute entries themselves. At first we distribute entries which can
   * be placed unambiguously and collect "common entries" to array.
   */

  /* Allocate vectors for results */
  v->spl_left = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
  v->spl_right = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
  v->spl_nleft = 0;
  v->spl_nright = 0;

  /*
   * Allocate an array for "common entries" - entries which can be placed to
   * either group without affecting overlap along selected axis.
   */
  common_entries_count = 0;
  common_entries = (CommonEntry *) palloc(nentries * sizeof(CommonEntry));

  /*
   * Distribute entries which can be distributed unambiguously, and collect
   * common entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    PeriodBound  lower,
          upper;
    /*
     * Get upper and lower bounds along selected axis.
     */
    Period *period = DatumGetPeriodP(entryvec->vector[i].key);
    period_deserialize(period, &lower, &upper);

    if (period_bound_cmp(&upper, &context.left_upper) <= 0)
    {
      /* Fits in the left group */
      if (period_bound_cmp(&lower, &context.right_lower) >= 0)
      {
        /* Fits also in the right group, so "common entry" */
        common_entries[common_entries_count].index = i;
        /*
         * delta = (lower - context.right_lower) -
         * (context.left_upper - upper)
         */
        common_entries[common_entries_count].delta =
          period_to_secs(period->lower, context.right_lower.t) -
          period_to_secs(context.left_upper.t, period->upper);
        common_entries_count++;
      }
      else
      {
        /* Doesn't fit to the right group, so join to the left group */
        PLACE_LEFT(period, i);
      }
    }
    else
    {
      /*
       * Each entry should fit on either left or right group. Since this
       * entry didn't fit in the left group, it better fit in the right
       * group.
       */
      assert(period_bound_cmp(&lower, &context.right_lower) >= 0);
      PLACE_RIGHT(period, i);
    }
  }

  /*
   * Distribute "common entries", if any.
   */
  if (common_entries_count > 0)
  {
    /*
     * Sort "common entries" by calculated deltas in order to distribute
     * the most ambiguous entries first.
     */
    qsort(common_entries, (size_t) common_entries_count, sizeof(CommonEntry),
        common_entry_cmp);

    /*
     * Distribute "common entries" between groups according to sorting.
     */
    for (i = 0; i < common_entries_count; i++)
    {
      int idx = common_entries[i].index;
      Period *period = DatumGetPeriodP(entryvec->vector[idx].key);

      /*
       * Check if we have to place this entry in either group to achieve
       * LIMIT_RATIO.
       */
      if (i < context.common_left)
        PLACE_LEFT(period, idx);
      else
        PLACE_RIGHT(period, idx);
    }
  }

  v->spl_ldatum = PointerGetDatum(left_period);
  v->spl_rdatum = PointerGetDatum(right_period);
  return;
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_gist_picksplit);
/**
 * GiST picksplit method for time types
 *
 * It splits a list of periods into quadrants by choosing a central 4D
 * point as the median of the coordinates of the periods.
 */
PGDLLEXPORT Datum
Period_gist_picksplit(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  size_t nbytes;
  OffsetNumber maxoff;

  maxoff = (OffsetNumber) (entryvec->n - 1);
  nbytes = (maxoff + 1) * sizeof(OffsetNumber);
  v->spl_left = (OffsetNumber *) palloc(nbytes);
  v->spl_right = (OffsetNumber *) palloc(nbytes);

  period_gist_double_sorting_split(entryvec, v);

  PG_RETURN_POINTER(v);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_gist_same);
/**
 * GiST same method for time types
 */
PGDLLEXPORT Datum
Period_gist_same(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  *result = period_eq(p1, p2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST fetch method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_gist_fetch);
/**
 * GiST fetch method for time types (result in a period)
 */
PGDLLEXPORT Datum
Period_gist_fetch(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************/
