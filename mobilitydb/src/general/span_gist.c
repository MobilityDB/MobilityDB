/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief R-tree GiST index for span and span set types.
 *
 * These functions are based on those in the file `rangetypes_gist.c`.
 */

#include "pg_general/span_gist.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <access/gist.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * @brief Leaf-level consistency for span types.
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
span_index_consistent_leaf(const Span *key, const Span *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      return overlaps_span_span(key, query);
    case RTContainsStrategyNumber:
      return contains_span_span(key, query);
    case RTContainedByStrategyNumber:
      return contains_span_span(query, key);
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return span_eq(key, query);
    case RTAdjacentStrategyNumber:
      return adjacent_span_span(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return left_span_span(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return overleft_span_span(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return right_span_span(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return overright_span_span(key, query);
    default:
      elog(ERROR, "unrecognized span strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * @brief GiST internal-page consistency for span types
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
bool
span_gist_consistent(const Span *key, const Span *query,
  StrategyNumber strategy)
{
  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      return overlaps_span_span(key, query);
    case RTContainsStrategyNumber:
    case RTEqualStrategyNumber:
    case RTSameStrategyNumber:
      return contains_span_span(key, query);
    case RTAdjacentStrategyNumber:
      return adjacent_span_span(key, query) || overlaps_span_span(key, query);
    case RTLeftStrategyNumber:
    case RTBeforeStrategyNumber:
      return ! overright_span_span(key, query);
    case RTOverLeftStrategyNumber:
    case RTOverBeforeStrategyNumber:
      return ! right_span_span(key, query);
    case RTRightStrategyNumber:
    case RTAfterStrategyNumber:
      return ! overleft_span_span(key, query);
    case RTOverRightStrategyNumber:
    case RTOverAfterStrategyNumber:
      return ! left_span_span(key, query);
    default:
      elog(ERROR, "unrecognized span strategy: %d", strategy);
      return false;    /* keep compiler quiet */
  }
}

/**
 * @brief Return true if a recheck is necessary depending on the strategy
 */
bool
span_index_recheck(StrategyNumber strategy)
{
  /* These operators are based on bounding boxes */
  if (strategy == RTLeftStrategyNumber ||
      strategy == RTOverLeftStrategyNumber ||
      strategy == RTRightStrategyNumber ||
      strategy == RTOverRightStrategyNumber)
    return false;
  return true;
}

/**
 * @brief Transform the query argument into a span
 */
static bool
span_gist_get_span(FunctionCallInfo fcinfo, Span *result, Oid typid)
{
  meosType type = oid_type(typid);
  if (span_basetype(type))
  {
    /* Since function span_gist_consistent is strict, d is not NULL */
    Datum d = PG_GETARG_DATUM(1);
    span_set(d, d, true, true, type, result);
  }
  else if (set_type(type))
  {
    Set *s = PG_GETARG_SET_P(1);
    set_set_span(s, result);
  }
  else if (span_type(type))
  {
    Span *s = PG_GETARG_SPAN_P(1);
    if (s == NULL)
      PG_RETURN_BOOL(false);
    memcpy(result, s, sizeof(Span));
  }
  else if (spanset_type(type))
  {
    Datum psdatum = PG_GETARG_DATUM(1);
    spanset_span_slice(psdatum, result);
  }
  /* For temporal types whose bounding box is a period */
  else if (talpha_type(type))
  {
    Datum tempdatum = PG_GETARG_DATUM(1);
    temporal_bbox_slice(tempdatum, result);
  }
  else
    elog(ERROR, "Unsupported type for indexing: %d", type);
  return true;
}

PGDLLEXPORT Datum Span_gist_consistent(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_consistent);
/**
 * @brief GiST consistent method for span types
 */
Datum
Span_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  bool result;
  const Span *key = DatumGetSpanP(entry->key);
  Span query;

  /* Determine whether the operator is exact */
  *recheck = span_index_recheck(strategy);

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! span_gist_get_span(fcinfo, &query, typid))
    PG_RETURN_BOOL(false);

  if (GIST_LEAF(entry))
    result = span_index_consistent_leaf(key, &query, strategy);
  else
    result = span_gist_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_union(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_union);
/**
 * @brief GiST union method for span types
 */
Datum
Span_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  Span *result = span_copy(DatumGetSpanP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    span_expand(DatumGetSpanP(ent[i].key), result);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************
 * GiST compress methods
 *****************************************************************************/

PGDLLEXPORT Datum Set_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_gist_compress);
/**
 * @brief GiST compress method for timestamp sets
 */
Datum
Set_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    Span *span = palloc(sizeof(Span));
    set_set_span(DatumGetSetP(entry->key), span);
    gistentryinit(*retval, PointerGetDatum(span), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

PGDLLEXPORT Datum Spanset_gist_compress(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_gist_compress);
/**
 * @brief GiST compress method for span sets
 */
Datum
Spanset_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    Span *span = palloc(sizeof(Span));
    spanset_span_slice(entry->key, span);
    gistentryinit(*retval, PointerGetDatum(span), entry->rel, entry->page,
      entry->offset, false);
    PG_RETURN_POINTER(retval);
  }
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST penalty method for span types
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_penalty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_penalty);
/**
 * @brief GiST page split penalty function for spans.
 *
 * The penalty function has the following goals (in order from most to least
 * important):
 * - Avoid broadening (as determined by distance_value_value) the original
 *   predicate
 * - Favor adding spans to narrower original predicates
 */
Datum
Span_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *penalty = (float *) PG_GETARG_POINTER(2);
  const Span *orig = DatumGetSpanP(origentry->key);
  const Span *new = DatumGetSpanP(newentry->key);
  SpanBound orig_lower, new_lower, orig_upper, new_upper;
  span_deserialize(orig, &orig_lower, &orig_upper);
  span_deserialize(new, &new_lower, &new_upper);

  /* Calculate extension of original span by calling distance_value_value */
  float8 diff = 0.0;
  if (span_bound_cmp(&new_lower, &orig_lower) < 0)
    diff += distance_value_value(orig->lower, new->lower, orig->basetype,
      new->basetype);
  if (span_bound_cmp(&new_upper, &orig_upper) > 0)
    diff += distance_value_value(new->upper, orig->upper, new->basetype,
      orig->basetype);
  *penalty = (float4) diff;

  PG_RETURN_POINTER(penalty);
}

/*****************************************************************************
 * GiST picksplit method for span types
 *****************************************************************************/

/**
 * @brief Return the bounding union of two spans.
 * @note The result of the function is always a span even if the spans do not
 * overlap
 * @note This function is similar to `bbox_union_span_span` with memory
 * allocation
 */
static Span *
super_union_span_span(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  Span *result = span_copy(s1);
  span_expand(s2, result);
  return result;
}

/* Helper macros to place an entry in the left or right group during split */
/* Note direct access to variables v, left_span, right_span */
#define PLACE_LEFT(span, off)          \
  do {                    \
    if (v->spl_nleft > 0)          \
      left_span = super_union_span_span(left_span, span); \
    else                  \
      left_span = (span);        \
    v->spl_left[v->spl_nleft++] = (off);  \
  } while (0)

#define PLACE_RIGHT(span, off)        \
  do {                    \
    if (v->spl_nright > 0)          \
      right_span = super_union_span_span(right_span, span); \
    else                  \
      right_span = (span);      \
    v->spl_right[v->spl_nright++] = (off);  \
  } while (0)

/**
 * @brief Trivial split: half of entries will be placed on one page
 * and the other half on the other page.
 */
static void
span_gist_fallback_split(GistEntryVector *entryvec, GIST_SPLITVEC *v)
{
  Span *left_span = NULL, *right_span = NULL;
  OffsetNumber i, maxoff, split_idx;

  maxoff = (OffsetNumber) (entryvec->n - 1);
  /* Split entries before this to left page, after to right: */
  split_idx = (OffsetNumber) ((maxoff - FirstOffsetNumber) / 2 + FirstOffsetNumber);

  v->spl_nleft = 0;
  v->spl_nright = 0;
  for (i = FirstOffsetNumber; i <= maxoff; i++)
  {
    Span *span = DatumGetSpanP(entryvec->vector[i].key);

    if (i < split_idx)
      PLACE_LEFT(span, i);
    else
      PLACE_RIGHT(span, i);
  }

  v->spl_ldatum = SpanPGetDatum(left_span);
  v->spl_rdatum = SpanPGetDatum(right_span);
  return;
}

/**
 * @brief Structure keeping context for the function span_gist_consider_split
 */
typedef struct
{
  int entries_count;  /**< total number of entries being split */
  /** Information about currently selected split follows */
  bool first;         /**< true if no split was selected yet */
  SpanBound left_upper;  /**< upper bound of left interval */
  SpanBound right_lower; /**< lower bound of right interval */
  float4 ratio;      /**< split ratio */
  float4 overlap;    /**< overlap between left and right predicate */
  int common_left;   /**< number of common entries destined for each side */
  int common_right;
} ConsiderSplitContext;

/**
 * @brief Consider replacement of currently selected split with a better one
 * during span_gist_double_sorting_split.
 */
static void
span_gist_consider_split(ConsiderSplitContext *context, SpanBound *right_lower,
  int min_left_count, SpanBound *left_upper, int max_left_count)
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
    overlap = (float4) distance_value_value(left_upper->val, right_lower->val,
      left_upper->basetype, right_lower->basetype);

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
 * @brief Structure keeping the bounds extracted from a span, for use in the
 * function span_gist_double_sorting_split
 */
typedef struct
{
  SpanBound lower;
  SpanBound upper;
} SpanBounds;

/**
 * @brief Compare SpanBounds by lower bound.
 */
static int
spanbounds_cmp_lower(const void *a, const void *b)
{
  SpanBounds *i1 = (SpanBounds *) a;
  SpanBounds *i2 = (SpanBounds *) b;
  return span_bound_cmp(&i1->lower, &i2->lower);
}

/**
 * @brief Compare SpanBounds by upper bound.
 */
static int
spanbounds_cmp_upper(const void *a, const void *b)
{
  SpanBounds *i1 = (SpanBounds *) a;
  SpanBounds *i2 = (SpanBounds *) b;
  return span_bound_cmp(&i1->upper, &i2->upper);
}

/**
 * @brief Compare CommonEntrys by their deltas.
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
 * @brief Double sorting split algorithm.
 *
 * The algorithm considers dividing spans into two groups. The first (left)
 * group contains general left bound. The second (right) group contains
 * general right bound. The challenge is to find upper bound of left group
 * and lower bound of right group so that overlap of groups is minimal and
 * ratio of distribution is acceptable. Algorithm finds for each lower bound of
 * right group minimal upper bound of left group, and for each upper bound of
 * left group maximal lower bound of right group. For each found pair
 * span_gist_consider_split considers replacement of currently selected
 * split with the new one.
 *
 * After that, all the entries are divided into three groups:
 * 1) Entries which should be placed to the left group
 * 2) Entries which should be placed to the right group
 * 3) "Common entries" which can be placed to either group without affecting
 *    amount of overlap.
 *
 * The common spans are distributed by difference of distance from lower
 * bound of common span to lower bound of right group and distance from upper
 * bound of common span to upper bound of left group.
 *
 * For details see:
 * "A new double sorting-based node splitting algorithm for R-tree",
 * A. Korotkov
 * http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf#page=36
 */
static void
span_gist_double_sorting_split(GistEntryVector *entryvec, GIST_SPLITVEC *v)
{
  ConsiderSplitContext context;
  OffsetNumber i, maxoff;
  Span *left_span = NULL, *right_span = NULL;
  SpanBounds *by_lower, *by_upper;
  SpanBound *right_lower, *left_upper;
  CommonEntry *common_entries;
  int common_entries_count, nentries, i1, i2;

  memset(&context, 0, sizeof(ConsiderSplitContext));

  maxoff = (OffsetNumber) (entryvec->n - 1);
  nentries = context.entries_count = maxoff - FirstOffsetNumber + 1;
  context.first = true;

  /* Allocate arrays for sorted span bounds */
  by_lower = palloc(sizeof(SpanBounds) * nentries);
  by_upper = palloc(sizeof(SpanBounds) * nentries);
  /* Fill arrays of bounds */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    Span *span = DatumGetSpanP(entryvec->vector[i].key);
    span_deserialize(span, &by_lower[i - FirstOffsetNumber].lower,
      &by_lower[i - FirstOffsetNumber].upper);
  }
  /*
   * Make two arrays of span bounds: one sorted by lower bound and another
   * sorted by upper bound.
   */
  memcpy(by_upper, by_lower, nentries * sizeof(SpanBounds));
  qsort(by_lower, (size_t) nentries, sizeof(SpanBounds),
    (qsort_comparator) spanbounds_cmp_lower);
  qsort(by_upper, (size_t) nentries, sizeof(SpanBounds),
    (qsort_comparator) spanbounds_cmp_upper);

  /*----------
   * The goal is to form a left and right span, so that every entry
   * span is contained in either left or right interval (or both).
   *
   * For example, with the spans (0,1), (1,3), (2,3), (2,4):
   *
   * 0 1 2 3 4
   * +-+
   *   +---+
   *     +-+
   *     +---+
   *
   * The left and right spans are of the form (0,a) and (b,4).
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
        span_bound_cmp(right_lower, &by_lower[i1].lower) == 0)
    {
      if (span_bound_cmp(&by_lower[i1].upper, left_upper) > 0)
        left_upper = &by_lower[i1].upper;
      i1++;
    }
    if (i1 >= nentries)
      break;
    right_lower = &by_lower[i1].lower;

    /*
     * Find count of spans which anyway should be placed to the left
     * group.
     */
    while (i2 < nentries &&
         span_bound_cmp(&by_upper[i2].upper, left_upper) <= 0)
      i2++;

    /*
     * Consider found split to see if it's better than what we had.
     */
    span_gist_consider_split(&context, right_lower, i1, left_upper, i2);
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
    while (i2 >= 0 && span_bound_cmp(left_upper,
      &by_upper[i2].upper) == 0)
    {
      if (span_bound_cmp(&by_upper[i2].lower,
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
      span_bound_cmp(&by_lower[i1].lower, right_lower) >= 0)
      i1--;

    /*
     * Consider found split to see if it's better than what we had.
     */
    span_gist_consider_split(&context, right_lower, i1 + 1, left_upper, i2 + 1);
  }

  /*
   * If we failed to find any acceptable splits, use trivial split.
   */
  if (context.first)
  {
    span_gist_fallback_split(entryvec, v);
    return;
  }

  /*
   * Ok, we have now selected bounds of the groups. Now we have to
   * distribute entries themselves. At first we distribute entries which can
   * be placed unambiguously and collect "common entries" to array.
   */

  /* Allocate vectors for results */
  v->spl_left = palloc(nentries * sizeof(OffsetNumber));
  v->spl_right = palloc(nentries * sizeof(OffsetNumber));
  v->spl_nleft = 0;
  v->spl_nright = 0;

  /*
   * Allocate an array for "common entries" - entries which can be placed to
   * either group without affecting overlap along selected axis.
   */
  common_entries_count = 0;
  common_entries = palloc(nentries * sizeof(CommonEntry));

  /*
   * Distribute entries which can be distributed unambiguously, and collect
   * common entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    SpanBound  lower,
          upper;
    /*
     * Get upper and lower bounds along selected axis.
     */
    Span *span = DatumGetSpanP(entryvec->vector[i].key);
    span_deserialize(span, &lower, &upper);

    if (span_bound_cmp(&upper, &context.left_upper) <= 0)
    {
      /* Fits in the left group */
      if (span_bound_cmp(&lower, &context.right_lower) >= 0)
      {
        /* Fits also in the right group, so "common entry" */
        common_entries[common_entries_count].index = i;
        /*
         * delta = (lower - context.right_lower) -
         * (context.left_upper - upper)
         */
        common_entries[common_entries_count].delta =
          distance_value_value(span->lower, context.right_lower.val,
            span->basetype, context.right_lower.basetype) -
          distance_value_value(context.left_upper.val, span->upper,
            context.right_lower.basetype, span->basetype);
        common_entries_count++;
      }
      else
      {
        /* Doesn't fit to the right group, so join to the left group */
        PLACE_LEFT(span, i);
      }
    }
    else
    {
      /*
       * Each entry should fit on either left or right group. Since this
       * entry didn't fit in the left group, it better fit in the right
       * group.
       */
      assert(span_bound_cmp(&lower, &context.right_lower) >= 0);
      PLACE_RIGHT(span, i);
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
      OffsetNumber idx = (OffsetNumber) (common_entries[i].index);
      Span *span = DatumGetSpanP(entryvec->vector[idx].key);

      /*
       * Check if we have to place this entry in either group to achieve
       * LIMIT_RATIO.
       */
      if (i < context.common_left)
        PLACE_LEFT(span, idx);
      else
        PLACE_RIGHT(span, idx);
    }
  }

  v->spl_ldatum = PointerGetDatum(left_span);
  v->spl_rdatum = PointerGetDatum(right_span);
  return;
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_picksplit(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_picksplit);
/**
 * @brief GiST picksplit method for span types
 *
 * It splits a list of spans into quadrants by choosing a central 4D
 * point as the median of the coordinates of the spans.
 */
Datum
Span_gist_picksplit(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  size_t nbytes;
  OffsetNumber maxoff;

  maxoff = (OffsetNumber) (entryvec->n - 1);
  nbytes = (maxoff + 1) * sizeof(OffsetNumber);
  v->spl_left = palloc(nbytes);
  v->spl_right = palloc(nbytes);

  span_gist_double_sorting_split(entryvec, v);

  PG_RETURN_POINTER(v);
}

/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_same);
/**
 * @brief GiST same method for span types
 */
Datum
Span_gist_same(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  *result = span_eq(s1, s2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_distance);
/**
 * @brief GiST support function. Take in a query and an entry and return the
 * "distance" between them.
*/
Datum
Span_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  Span *key = (Span *) DatumGetPointer(entry->key);
  Span query;
  double distance;

  /* The index is not lossy */
  if (GIST_LEAF(entry))
    *recheck = false;

  if (key == NULL)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Transform the query into a box */
  if (! span_gist_get_span(fcinfo, &query, typid))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves */
  distance = distance_span_span(key, &query);

  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************
 * GiST fetch method
 *****************************************************************************/

PGDLLEXPORT Datum Span_gist_fetch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gist_fetch);
/**
 * @brief GiST fetch method for span types (result in a span)
 */
Datum
Span_gist_fetch(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(entry);
}

/*****************************************************************************/
