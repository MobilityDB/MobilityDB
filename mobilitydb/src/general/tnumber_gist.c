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
 * @brief R-tree GiST index for temporal integers and temporal floats.
 *
 * These functions are based on those in the file `gistproc.c`.
 */

#include "pg_general/tnumber_gist.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <utils/float.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <general/temporal_boxops.h>
#include <general/temporal_util.h>
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/time_gist.h"

/*****************************************************************************
 * GiST consistent methods
 *****************************************************************************/

/**
 * Leaf-level consistency for temporal numbers.
 *
 * Since temporal boxes do not distinguish between inclusive and
 * exclusive bounds, it is necessary to generalize the tests, e.g.,
 * - left : (box1->xmax < box2->xmin) => (box1->xmax <= box2->xmin)
 *   e.g., to take into account left([a,b],(b,c])
 * - right : (box1->xmin > box2->xmax) => (box1->xmin >= box2->xmax)
 *   e.g., to take into account right((b,c],[a,b])
 * and similarly for before and after
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 * @note This function is used for both GiST and SP-GiST indexes
 */
bool
tbox_index_consistent_leaf(const TBOX *key, const TBOX *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
      retval = overlaps_tbox_tbox(key, query);
      break;
    case RTContainsStrategyNumber:
      retval = contains_tbox_tbox(key, query);
      break;
    case RTContainedByStrategyNumber:
      retval = contained_tbox_tbox(key, query);
      break;
    case RTSameStrategyNumber:
      retval = same_tbox_tbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      retval = adjacent_tbox_tbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = left_tbox_tbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = overleft_tbox_tbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = right_tbox_tbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = overright_tbox_tbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = before_tbox_tbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = overbefore_tbox_tbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = after_tbox_tbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = overafter_tbox_tbox(key, query);
      break;
    default:
      elog(ERROR, "unrecognized strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * GiST internal-page consistent method for temporal numbers.
 *
 * Return false if for all data items x below entry, the predicate
 * x op query must be false, where op is the oper corresponding to
 * strategy in the pg_amop table.
 *
 * @param[in] key Element in the index
 * @param[in] query Value being looked up in the index
 * @param[in] strategy Operator of the operator class being applied
 */
static bool
tnumber_gist_consistent(const TBOX *key, const TBOX *query,
  StrategyNumber strategy)
{
  bool retval;

  switch (strategy)
  {
    case RTOverlapStrategyNumber:
    case RTContainedByStrategyNumber:
      retval = overlaps_tbox_tbox(key, query);
      break;
    case RTContainsStrategyNumber:
    case RTSameStrategyNumber:
      retval = contains_tbox_tbox(key, query);
      break;
    case RTAdjacentStrategyNumber:
      retval = adjacent_tbox_tbox(key, query) ||
        overlaps_tbox_tbox(key, query);
      break;
    case RTLeftStrategyNumber:
      retval = ! overright_tbox_tbox(key, query);
      break;
    case RTOverLeftStrategyNumber:
      retval = ! right_tbox_tbox(key, query);
      break;
    case RTRightStrategyNumber:
      retval = ! overleft_tbox_tbox(key, query);
      break;
    case RTOverRightStrategyNumber:
      retval = ! left_tbox_tbox(key, query);
      break;
    case RTBeforeStrategyNumber:
      retval = ! overafter_tbox_tbox(key, query);
      break;
    case RTOverBeforeStrategyNumber:
      retval = ! after_tbox_tbox(key, query);
      break;
    case RTAfterStrategyNumber:
      retval = ! overbefore_tbox_tbox(key, query);
      break;
    case RTOverAfterStrategyNumber:
      retval = ! before_tbox_tbox(key, query);
      break;
    default:
      elog(ERROR, "unrecognized strategy number: %d", strategy);
      retval = false;    /* keep compiler quiet */
      break;
  }
  return retval;
}

/**
 * Transform the query argument into a box initializing the dimensions that
 * must not be taken into account by the operators to infinity.
 */
static bool
tnumber_gist_get_tbox(FunctionCallInfo fcinfo, TBOX *result, Oid typid)
{
  mobdbType type = oid_type(typid);
  if (tnumber_basetype(type))
  {
    Datum value = PG_GETARG_DATUM(1);
    number_set_tbox(value, type, result);
  }
  else if (tnumber_spantype(type))
  {
    Span *span = PG_GETARG_SPAN_P(1);
    if (span == NULL)
      return false;
    span_set_tbox(span, result);
  }
  else if (type == T_TIMESTAMPTZ)
  {
    TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
    timestamp_set_tbox(t, result);
  }
  else if (type == T_TIMESTAMPSET)
  {
    Datum tsdatum = PG_GETARG_DATUM(1);
    timestampset_tbox_slice(tsdatum, result);
  }
  else if (type == T_PERIOD)
  {
    Period *p = PG_GETARG_SPAN_P(1);
    period_set_tbox(p, result);
  }
  else if (type == T_PERIODSET)
  {
    Datum psdatum = PG_GETARG_DATUM(1);
    periodset_tbox_slice(psdatum, result);
  }
  else if (type == T_TBOX)
  {
    TBOX *box = PG_GETARG_TBOX_P(1);
    if (box == NULL)
      return false;
    memcpy(result, box, sizeof(TBOX));
  }
  else if (tnumber_type(type))
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

PG_FUNCTION_INFO_V1(Tnumber_gist_consistent);
/**
 * GiST consistent method for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_gist_consistent(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4), result;
  const TBOX *key = DatumGetTboxP(entry->key);
  TBOX query;

  /*
   * All tests are lossy since boxes do not distinghish between inclusive
   * and exclusive bounds.
   */
  *recheck = true;

  if (key == NULL)
    PG_RETURN_BOOL(false);

  /* Transform the query into a box */
  if (! tnumber_gist_get_tbox(fcinfo, &query, typid))
    PG_RETURN_BOOL(false);

  if (GIST_LEAF(entry))
    result = tbox_index_consistent_leaf(key, &query, strategy);
  else
    result = tnumber_gist_consistent(key, &query, strategy);

  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * GiST union method
 *****************************************************************************/

/**
 * Expand the first box to include the second one
 *
 * @param[in,out] bbox1 Resulting box
 * @param[in] bbox2 Add-on box
 * @note This function is similar to tbox_expand in file tbox.c but uses
 *   NaN-aware comparisons for the value dimension
 */
static void
tbox_adjust(void *bbox1, void *bbox2)
{
  TBOX *box1 = (TBOX *) bbox1;
  TBOX *box2 = (TBOX *) bbox2;
  double xmin = FLOAT8_MIN(DatumGetFloat8(box1->span.lower),
    DatumGetFloat8(box2->span.lower));
  double xmax = FLOAT8_MAX(DatumGetFloat8(box1->span.upper),
    DatumGetFloat8(box2->span.upper));
  box1->span.lower = Float8GetDatum(xmin);
  box1->span.upper = Float8GetDatum(xmax);
  TimestampTz tmin = Min(DatumGetTimestampTz(box1->period.lower),
    DatumGetTimestampTz(box2->period.lower));
  TimestampTz tmax = Max(DatumGetTimestampTz(box1->period.upper),
    DatumGetTimestampTz(box2->period.upper));
  box1->period.lower = TimestampTzGetDatum(tmin);
  box1->period.upper = TimestampTzGetDatum(tmax);
  return;
}

PG_FUNCTION_INFO_V1(Tbox_gist_union);
/**
 * GiST union method for temporal numbers.
 *
 * Return the minimal bounding box that encloses all the entries in entryvec
 */
PGDLLEXPORT Datum
Tbox_gist_union(PG_FUNCTION_ARGS)
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GISTENTRY *ent = entryvec->vector;
  TBOX *result = tbox_copy(DatumGetTboxP(ent[0].key));
  for (int i = 1; i < entryvec->n; i++)
    tbox_adjust((void *)result, DatumGetPointer(ent[i].key));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST compress method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_gist_compress);
/**
 * GiST compress method for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_gist_compress(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  if (entry->leafkey)
  {
    GISTENTRY *retval = palloc(sizeof(GISTENTRY));
    TBOX *box = palloc(sizeof(TBOX));
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
 * @note This function uses NaN-aware comparisons
 */
static void
tbox_union_rt(const TBOX *a, const TBOX *b, TBOX *new)
{
  memset(new, 0, sizeof(TBOX));
  double xmin = FLOAT8_MIN(DatumGetFloat8(a->span.lower),
    DatumGetFloat8(b->span.lower));
  double xmax = FLOAT8_MAX(DatumGetFloat8(a->span.upper),
    DatumGetFloat8(b->span.upper));
  new->span.lower = Float8GetDatum(xmin);
  new->span.upper = Float8GetDatum(xmax);
  TimestampTz tmin = Min(DatumGetTimestampTz(a->period.lower),
    DatumGetTimestampTz(b->period.lower));
  TimestampTz tmax = Max(DatumGetTimestampTz(a->period.upper),
    DatumGetTimestampTz(b->period.upper));
  new->period.lower = TimestampTzGetDatum(tmin);
  new->period.upper = TimestampTzGetDatum(tmax);
  return;
}

/**
 * Return the size of a temporal box for penalty-calculation purposes.
 * The result can be +Infinity, but not NaN.
 */
static double
tbox_size(const TBOX *box)
{
  /*
   * Check for zero-width cases.  Note that we define the size of a zero-
   * by-infinity box as zero.  It's important to special-case this somehow,
   * as naively multiplying infinity by zero will produce NaN.
   *
   * The less-than cases should not happen, but if they do, say "zero".
   */
  if (FLOAT8_LE(DatumGetFloat8(box->span.upper), DatumGetFloat8(box->span.lower)) ||
    datum_le(box->period.upper, box->period.lower, T_TIMESTAMPTZ))
    return 0.0;

  /*
   * We treat NaN as larger than +Infinity, so any distance involving a NaN
   * and a non-NaN is infinite.  Note the previous check eliminated the
   * possibility that the low fields are NaNs.
   */
  if (isnan(DatumGetFloat8(box->span.upper)))
    return get_float8_infinity();
  return (DatumGetFloat8(box->span.upper) - DatumGetFloat8(box->span.lower)) *
    (DatumGetTimestampTz(box->period.upper) -
      DatumGetTimestampTz(box->period.lower));
}

/**
 * Return the amount by which the union of the two boxes is larger than
 * the original TBOX's area.  The result can be +Infinity, but not NaN.
 */
double
tbox_penalty(void *bbox1, void *bbox2)
{
  const TBOX *original = (TBOX *) bbox1;
  const TBOX *new = (TBOX *) bbox2;
  TBOX unionbox;
  tbox_union_rt(original, new, &unionbox);
  return tbox_size(&unionbox) - tbox_size(original);
}

PG_FUNCTION_INFO_V1(Tbox_gist_penalty);
/**
 * GiST penalty method for temporal boxes.
 * As in the R-tree paper, we use change in area as our penalty metric
 */
PGDLLEXPORT Datum
Tbox_gist_penalty(PG_FUNCTION_ARGS)
{
  GISTENTRY *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
  GISTENTRY *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
  float *result = (float *) PG_GETARG_POINTER(2);
  void *origbox = DatumGetPointer(origentry->key);
  void *newbox = DatumGetPointer(newentry->key);
  *result = (float) tbox_penalty(origbox, newbox);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST picksplit method
 *****************************************************************************/

/**
 * Interval comparison function by lower bound of the interval;
 */
int
interval_cmp_lower(const void *i1, const void *i2)
{
  double lower1 = ((const SplitInterval *) i1)->lower,
        lower2 = ((const SplitInterval *) i2)->lower;

  return float8_cmp_internal(lower1, lower2);
}

/**
 * Interval comparison function by upper bound of the interval;
 */
int
interval_cmp_upper(const void *i1, const void *i2)
{
  double upper1 = ((const SplitInterval *) i1)->upper,
    upper2 = ((const SplitInterval *) i2)->upper;

  return float8_cmp_internal(upper1, upper2);
}

/**
 * Replace negative (or NaN) value with zero.
 */
inline float
non_negative(float val)
{
  if (val >= 0.0f)
    return val;
  else
    return 0.0f;
}

/**
 * Consider replacement of currently selected split with the better one.
 */
inline void
bbox_gist_consider_split(ConsiderSplitContext *context, int dimNum,
  mobdbType bboxtype, double rightLower, int minLeftCount, double leftUpper,
  int maxLeftCount)
{
  int leftCount, rightCount;
  float4 ratio, overlap;

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
    double range;
    bool selectthis = false;

    /*
     * The ratio is acceptable, so compare current split with previously
     * selected one. Between splits of one dimension we search for minimal
     * overlap (allowing negative values) and minimal ration (between same
     * overlaps. We switch dimension if find less overlap (non-negative)
     * or less range with same overlap.
     */
    if (bboxtype == T_TBOX)
    {
      TBOX *bbox = (TBOX *) &context->boundingBox;
      if (dimNum == 0)
        range = DatumGetFloat8(bbox->span.upper) -
          DatumGetFloat8(bbox->span.lower);
      else
        range = (double) (DatumGetTimestampTz(bbox->period.upper) -
          DatumGetTimestampTz(bbox->period.lower));
    }
    else /* bboxtype == T_STBOX */
    {
      STBOX *bbox = (STBOX *) &context->boundingBox;
      if (dimNum == 0)
        range = bbox->xmax - bbox->xmin;
      else if (dimNum == 1)
        range = bbox->ymax - bbox->ymin;
      else if (dimNum == 2)
        range = bbox->zmax - bbox->zmin;
      else
        range = (double) (DatumGetTimestampTz(bbox->period.upper) -
          DatumGetTimestampTz(bbox->period.lower));
    }

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
}

/* Helper macros to place an entry in the left or right group */
#define PLACE_LEFT(box, off) \
  do {  \
    if (v->spl_nleft > 0)  \
      bbox_adjust(leftBox, box);  \
    else  \
      memcpy(leftBox, box, bbox_size);  \
    v->spl_left[v->spl_nleft++] = off;  \
  } while(0)

#define PLACE_RIGHT(box, off)  \
  do {  \
    if (v->spl_nright > 0)  \
      bbox_adjust(rightBox, box);  \
    else  \
      memcpy(rightBox, box, bbox_size);  \
    v->spl_right[v->spl_nright++] = off;  \
  } while(0)

/**
 * Trivial split: half of entries will be placed on one page and the other
 * half on another page
 */
void
bbox_gist_fallback_split(GistEntryVector *entryvec, GIST_SPLITVEC *v,
  mobdbType bboxtype, void (*bbox_adjust)(void *, void *))
{
  OffsetNumber i;
  OffsetNumber maxoff = (OffsetNumber) (entryvec->n - 1);
  /* Split entries before this to left page, after to right: */
  OffsetNumber split_idx = (OffsetNumber) ((maxoff - FirstOffsetNumber) / 2 +
    FirstOffsetNumber);

  size_t nbytes = (maxoff + 2) * sizeof(OffsetNumber);
  v->spl_left = palloc(nbytes);
  v->spl_right = palloc(nbytes);
  v->spl_nleft = v->spl_nright = 0;

  /* Allocate bounding boxes of left and right groups */
  size_t bbox_size = bbox_get_size(bboxtype);
  void *leftBox = palloc0(bbox_size);
  void *rightBox = palloc0(bbox_size);

  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    void *box = DatumGetPointer(entryvec->vector[i].key);
    if (i < split_idx)
      PLACE_LEFT(box, i);
    else
      PLACE_RIGHT(box, i);
  }

  v->spl_ldatum = PointerGetDatum(leftBox);
  v->spl_rdatum = PointerGetDatum(rightBox);
  return;
}

/**
 * Double sorting split algorithm.
 *
 * The algorithm finds split of boxes by considering splits along each axis.
 * Each entry is first projected as an interval on the X-axis, and different
 * ways to split the intervals into two groups are considered, trying to
 * minimize the overlap of the groups. Then the same is repeated for the
 * Y-axis, and the overall best split is chosen. The quality of a split is
 * determined by overlap along that axis and some other criteria (see
 * bbox_gist_consider_split).
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
Datum
bbox_gist_picksplit_ext(FunctionCallInfo fcinfo, mobdbType bboxtype,
  void (*bbox_adjust)(void *, void *), double (*bbox_penalty)(void *, void *))
{
  GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
  GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
  OffsetNumber i, maxoff;
  ConsiderSplitContext context;
  void *box, *leftBox, *rightBox;
  SplitInterval *intervalsLower, *intervalsUpper;
  CommonEntry *commonEntries;
  int nentries, commonEntriesCount, dim;

  int maxdims = bbox_max_dims(bboxtype);
  size_t bbox_size = bbox_get_size(bboxtype);
  bool hasz = false;

  memset(&context, 0, sizeof(ConsiderSplitContext));
  maxoff = (OffsetNumber) (entryvec->n - 1);
  nentries = context.entriesCount = maxoff - FirstOffsetNumber + 1;

  /* Allocate arrays for intervals along axes */
  intervalsLower = palloc(nentries * sizeof(SplitInterval));
  intervalsUpper = palloc(nentries * sizeof(SplitInterval));

  /*
   * Calculate the overall minimum bounding box over all the entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    box = DatumGetPointer(entryvec->vector[i].key);
    if (i == FirstOffsetNumber)
      memcpy(&context.boundingBox, box, bbox_size);
    else
      bbox_adjust(&context.boundingBox, box);
  }

  /* Determine whether there is a Z dimension for spatiotemporal boxes */
  if (bboxtype == T_STBOX)
  {
    box = DatumGetPointer(entryvec->vector[FirstOffsetNumber].key);
    hasz = MOBDB_FLAGS_GET_Z(((STBOX *) box)->flags);
  }

  /*
   * Iterate over axes for optimal split searching.
   */
  context.first = true;    /* nothing selected yet */
  for (dim = 0; dim < maxdims; dim++)
  {
    double leftUpper, rightLower;
    int i1, i2;

    /* Skip the process for Z dimension if it is missing */
    if (dim == 2 && ! hasz)
      continue;

    /* Project each entry as an interval on the selected axis. */
    for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
    {
      box = DatumGetPointer(entryvec->vector[i].key);
      if (bboxtype == T_TBOX)
      {
        if (dim == 0)
        {
          intervalsLower[i - FirstOffsetNumber].lower =
            DatumGetFloat8(((TBOX *) box)->span.lower);
          intervalsLower[i - FirstOffsetNumber].upper =
            DatumGetFloat8(((TBOX *) box)->span.upper);
        }
        else
        {
          intervalsLower[i - FirstOffsetNumber].lower =
            DatumGetTimestampTz(((TBOX *) box)->period.lower);
          intervalsLower[i - FirstOffsetNumber].upper =
            DatumGetTimestampTz(((TBOX *) box)->period.upper);
        }
      }
      else /* bboxtype == T_STBOX */
      {
        if (dim == 0)
        {
          intervalsLower[i - FirstOffsetNumber].lower = ((STBOX *) box)->xmin;
          intervalsLower[i - FirstOffsetNumber].upper = ((STBOX *) box)->xmax;
        }
        else if (dim == 1)
        {
          intervalsLower[i - FirstOffsetNumber].lower = ((STBOX *) box)->ymin;
          intervalsLower[i - FirstOffsetNumber].upper = ((STBOX *) box)->ymax;
        }
        else if (dim == 2)
        {
          intervalsLower[i - FirstOffsetNumber].lower = ((STBOX *) box)->zmin;
          intervalsLower[i - FirstOffsetNumber].upper = ((STBOX *) box)->zmax;
        }
        else
        {
          intervalsLower[i - FirstOffsetNumber].lower =
            DatumGetTimestampTz(((STBOX *) box)->period.lower);
          intervalsLower[i - FirstOffsetNumber].upper =
            DatumGetTimestampTz(((STBOX *) box)->period.upper);
        }
      }
    }

    /*
     * Make two arrays of intervals: one sorted by lower bound and another
     * sorted by upper bound.
     */
    memcpy(intervalsUpper, intervalsLower, sizeof(SplitInterval) * nentries);
    qsort(intervalsLower, (size_t) nentries, sizeof(SplitInterval),
      (qsort_comparator) interval_cmp_lower);
    qsort(intervalsUpper, (size_t) nentries, sizeof(SplitInterval),
      (qsort_comparator) interval_cmp_upper);

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
      while (i1 < nentries && FLOAT8_EQ(rightLower, intervalsLower[i1].lower))
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
      bbox_gist_consider_split(&context, dim, bboxtype, rightLower, i1,
        leftUpper, i2);
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
      bbox_gist_consider_split(&context, dim, bboxtype, rightLower, i1 + 1,
        leftUpper, i2 + 1);
    }
  }

  /*
   * If we failed to find any acceptable splits, use trivial split.
   */
  if (context.first)
  {
    bbox_gist_fallback_split(entryvec, v, bboxtype, &tbox_adjust);
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
  v->spl_left = palloc(nentries * sizeof(OffsetNumber));
  v->spl_right = palloc(nentries * sizeof(OffsetNumber));
  v->spl_nleft = v->spl_nright = 0;

  /* Allocate bounding boxes of left and right groups */
  leftBox = palloc0(bbox_size);
  rightBox = palloc0(bbox_size);

  /*
   * Allocate an array for "common entries" - entries which can be placed to
   * either group without affecting overlap along selected axis.
   */
  commonEntriesCount = 0;
  commonEntries = palloc(nentries * sizeof(CommonEntry));

  /*
   * Distribute entries which can be distributed unambiguously, and collect
   * common entries.
   */
  for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
  {
    double lower, upper;

    /*
     * Get upper and lower bounds along selected axis.
     */
    box = DatumGetPointer(entryvec->vector[i].key);
    if (bboxtype == T_TBOX)
    {
      if (context.dim == 0)
      {
        lower = DatumGetFloat8(((TBOX *) box)->span.lower);
        upper = DatumGetFloat8(((TBOX *) box)->span.upper);
      }
      else
      {
        lower = (double) DatumGetTimestampTz(((TBOX *) box)->period.lower);
        upper = (double) DatumGetTimestampTz(((TBOX *) box)->period.upper);
      }
    }
    else /* bboxtype == T_STBOX */
    {
      if (context.dim == 0)
      {
        lower = ((STBOX *) box)->xmin;
        upper = ((STBOX *) box)->xmax;
      }
      else if (context.dim == 1)
      {
        lower = ((STBOX *) box)->ymin;
        upper = ((STBOX *) box)->ymax;
      }
      else if (context.dim == 2 && hasz)
      {
        lower = ((STBOX *) box)->zmin;
        upper = ((STBOX *) box)->zmax;
      }
      else
      {
        lower = (double) DatumGetTimestampTz(((TBOX *) box)->period.lower);
        upper = (double) DatumGetTimestampTz(((TBOX *) box)->period.upper);
      }
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
    int m = (int) ceil(LIMIT_RATIO * (double) nentries);

    /*
     * Calculate delta between penalties of join "common entries" to
     * different groups.
     */
    for (i = 0; i < commonEntriesCount; i++)
    {
      box = DatumGetPointer(entryvec->vector[commonEntries[i].index].key);
      commonEntries[i].delta = Abs(bbox_penalty(leftBox, box) -
        bbox_penalty(rightBox, box));
    }

    /*
     * Sort "common entries" by calculated deltas in order to distribute
     * the most ambiguous entries first.
     */
    qsort(commonEntries, (size_t) commonEntriesCount, sizeof(CommonEntry),
      common_entry_cmp);

    /*
     * Distribute "common entries" between groups.
     */
    for (i = 0; i < commonEntriesCount; i++)
    {
      box = DatumGetPointer(entryvec->vector[commonEntries[i].index].key);

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
        if (tbox_penalty(leftBox, box) < tbox_penalty(rightBox, box))
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

PG_FUNCTION_INFO_V1(Tbox_gist_picksplit);
/**
 * GiST picksplit method for temporal numbers
 */
PGDLLEXPORT Datum
Tbox_gist_picksplit(PG_FUNCTION_ARGS)
{
  return bbox_gist_picksplit_ext(fcinfo, T_TBOX, &tbox_adjust, &tbox_penalty);
}
/*****************************************************************************
 * GiST same method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_gist_same);
/**
 * GiST same method for temporal numbers.
 * Return true only when boxes are exactly the same.  We can't use fuzzy
 * comparisons here without breaking index consistency; therefore, this isn't
 * equivalent to box_same().
 */
PGDLLEXPORT Datum
Tbox_gist_same(PG_FUNCTION_ARGS)
{
  TBOX *b1 = PG_GETARG_TBOX_P(0);
  TBOX *b2 = PG_GETARG_TBOX_P(1);
  bool *result = (bool *) PG_GETARG_POINTER(2);
  if (b1 && b2)
    *result = FLOAT8_EQ(DatumGetFloat8(b1->span.lower),
        DatumGetFloat8(b2->span.lower)) &&
      FLOAT8_EQ(DatumGetFloat8(b1->span.upper),
        DatumGetFloat8(b2->span.upper)) &&
      /* Equality test does not require to use DatumGetTimestampTz */
      (b1->period.lower == b2->period.lower) &&
      (b1->period.upper == b2->period.upper);
  else
    *result = (b1 == NULL && b2 == NULL);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * GiST distance method
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_gist_distance);
/**
 * GiST support function. Take in a query and an entry and return the "distance"
 * between them.
*/
PGDLLEXPORT Datum
Tbox_gist_distance(PG_FUNCTION_ARGS)
{
  GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
  Oid typid = PG_GETARG_OID(3);
  bool *recheck = (bool *) PG_GETARG_POINTER(4);
  TBOX *key = (TBOX *) DatumGetPointer(entry->key);
  TBOX query;
  double distance;

  /* The index is lossy for leaf levels */
  if (GIST_LEAF(entry))
    *recheck = true;

  if (key == NULL)
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Transform the query into a box */
  if (! tnumber_gist_get_tbox(fcinfo, &query, typid))
    PG_RETURN_FLOAT8(DBL_MAX);

  /* Since we only have boxes we'll return the minimum possible distance,
   * and let the recheck sort things out in the case of leaves */
  distance = nad_tbox_tbox(key, &query);

  PG_RETURN_FLOAT8(distance);
}

/*****************************************************************************/
