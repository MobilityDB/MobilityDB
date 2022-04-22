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
 * @file tnumber_selfuncs.c
 * @brief Functions for selectivity estimation of operators on temporal number
 * types.
 */

#include "general/tnumber_selfuncs.h"

/* PostgreSQL */
#include <assert.h>
#include <math.h>
#include <access/htup_details.h>
#include <catalog/pg_collation_d.h>
#include <utils/builtins.h>
#if POSTGRESQL_VERSION_NUMBER >= 120000
#include <utils/float.h>
#endif
#include <utils/selfuncs.h>
#include "general/temporal_boxops.h"
/* MobilityDB */
#include "general/period.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporal_util.h"
#include "general/tempcache.h"
#include "general/tbox.h"
#include "general/time_selfuncs.h"
#include "general/temporal_analyze.h"
#include "general/temporal_selfuncs.h"

/*****************************************************************************
 * Functions copied from PostgreSQL file rangetypes_selfuncs.c since they
 * are not exported.
 *****************************************************************************/

/**
 * Binary search on an array of range bounds. Return greatest index of range
 * bound in array which is less(less or equal) than given range bound. If all
 * range bounds in array are greater or equal(greater) than given range bound,
 * return -1. When "equal" flag is set conditions in brackets are used.
 *
 * This function is used in scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop
 * interpolation of portion of bounds which are less or equal to given bound.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static int
rbound_bsearch(TypeCacheEntry *typcache, RangeBound *value, RangeBound *hist,
         int hist_length, bool equal)
{
  int      lower = -1,
        upper = hist_length - 1;

  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    int cmp = range_cmp_bounds(typcache, &hist[middle], value);

    if (cmp < 0 || (equal && cmp == 0))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/**
 * Get relative position of value in histogram bin in [0,1] range.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static float8
get_position(TypeCacheEntry *typcache, RangeBound *value, RangeBound *hist1,
       RangeBound *hist2)
{
  bool    has_subdiff = OidIsValid(typcache->rng_subdiff_finfo.fn_oid);
  float8    position;

  if (!hist1->infinite && !hist2->infinite)
  {
    float8    bin_width;

    /*
     * Both bounds are finite. Assuming the subtype's comparison function
     * works sanely, the value must be finite, too, because it lies
     * somewhere between the bounds. If it doesn't, just return something.
     */
    if (value->infinite)
      return 0.5;

    /* Can't interpolate without subdiff function */
    if (!has_subdiff)
      return 0.5;

    /* Calculate relative position using subdiff function. */
    bin_width = DatumGetFloat8(FunctionCall2Coll(
                           &typcache->rng_subdiff_finfo,
                           typcache->rng_collation,
                           hist2->val,
                           hist1->val));
    if (bin_width <= 0.0)
      return 0.5;      /* zero width bin */

    position = DatumGetFloat8(FunctionCall2Coll(
                          &typcache->rng_subdiff_finfo,
                          typcache->rng_collation,
                          value->val,
                          hist1->val))
      / bin_width;

    /* Relative position must be in [0,1] range */
    position = Max(position, 0.0);
    position = Min(position, 1.0);
    return position;
  }
  else if (hist1->infinite && !hist2->infinite)
  {
    /*
     * Lower bin boundary is -infinite, upper is finite. If the value is
     * -infinite, return 0.0 to indicate it's equal to the lower bound.
     * Otherwise return 1.0 to indicate it's infinitely far from the lower
     * bound.
     */
    return ((value->infinite && value->lower) ? 0.0 : 1.0);
  }
  else if (!hist1->infinite && hist2->infinite)
  {
    /* same as above, but in reverse */
    return ((value->infinite && !value->lower) ? 1.0 : 0.0);
  }
  else
  {
    /*
     * If both bin boundaries are infinite, they should be equal to each
     * other, and the value should also be infinite and equal to both
     * bounds. (But don't Assert that, to avoid crashing if a user creates
     * a datatype with a broken comparison function).
     *
     * Assume the value to lie in the middle of the infinite bounds.
     */
    return 0.5;
  }
}

/**
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a given const in a histogram of range bounds.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static double
calc_hist_selectivity_scalar(TypeCacheEntry *typcache, RangeBound *constbound,
               RangeBound *hist, int hist_nvalues, bool equal)
{
  Selectivity selec;
  int      index;

  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  index = rbound_bsearch(typcache, constbound, hist, hist_nvalues, equal);
  selec = (Selectivity) (Max(index, 0)) / (Selectivity) (hist_nvalues - 1);

  /* Adjust using linear interpolation within the bin */
  if (index >= 0 && index < hist_nvalues - 1)
    selec += get_position(typcache, constbound, &hist[index],
                &hist[index + 1]) / (Selectivity) (hist_nvalues - 1);

  return selec;
}

/**
 * Measure distance between two range bounds.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static float8
get_distance(TypeCacheEntry *typcache, RangeBound *bound1, RangeBound *bound2)
{
  bool    has_subdiff = OidIsValid(typcache->rng_subdiff_finfo.fn_oid);

  if (!bound1->infinite && !bound2->infinite)
  {
    /*
     * No bounds are infinite, use subdiff function or return default
     * value of 1.0 if no subdiff is available.
     */
    if (has_subdiff)
      return
        DatumGetFloat8(FunctionCall2Coll(&typcache->rng_subdiff_finfo,
                         typcache->rng_collation,
                         bound2->val,
                         bound1->val));
    else
      return 1.0;
  }
  else if (bound1->infinite && bound2->infinite)
  {
    /* Both bounds are infinite */
    if (bound1->lower == bound2->lower)
      return 0.0;
    else
      return get_float8_infinity();
  }
  else
  {
    /* One bound is infinite, another is not */
    return get_float8_infinity();
  }
}

/**
 * Calculate selectivity of "var <@ const" operator, ie. estimate the fraction
 * of ranges that fall within the constant lower and upper bounds. This uses
 * the histograms of range lower bounds and range lengths, on the assumption
 * that the range lengths are independent of the lower bounds.
 *
 * The caller has already checked that constant lower and upper bounds are
 * finite.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static double
calc_hist_selectivity_contained(TypeCacheEntry *typcache, RangeBound *lower,
  RangeBound *upper, RangeBound *hist_lower, int hist_nvalues,
  Datum *length_hist_values, int length_hist_nvalues)
{
  int i, upper_index;
  float8 prev_dist;
  double bin_width, upper_bin_width, sum_frac;

  /*
   * Begin by finding the bin containing the upper bound, in the lower bound
   * histogram. Any range with a lower bound > constant upper bound can't
   * match, ie. there are no matches in bins greater than upper_index.
   */
  upper->inclusive = !upper->inclusive;
  upper->lower = true;
  upper_index = rbound_bsearch(typcache, upper, hist_lower, hist_nvalues, false);

  /*
   * Calculate upper_bin_width, ie. the fraction of the (upper_index,
   * upper_index + 1) bin which is greater than upper bound of query range
   * using linear interpolation of subdiff function.
   */
  if (upper_index >= 0 && upper_index < hist_nvalues - 1)
    upper_bin_width = get_position(typcache, upper,
      &hist_lower[upper_index], &hist_lower[upper_index + 1]);
  else
    upper_bin_width = 0.0;

  /*
   * In the loop, dist and prev_dist are the distance of the "current" bin's
   * lower and upper bounds from the constant upper bound.
   *
   * bin_width represents the width of the current bin. Normally it is 1.0,
   * meaning a full width bin, but can be less in the corner cases: start
   * and end of the loop. We start with bin_width = upper_bin_width, because
   * we begin at the bin containing the upper bound.
   */
  prev_dist = 0.0;
  bin_width = upper_bin_width;

  sum_frac = 0.0;
  for (i = upper_index; i >= 0; i--)
  {
    double dist;
    double length_hist_frac;
    bool final_bin = false;

    /*
     * dist -- distance from upper bound of query range to lower bound of
     * the current bin in the lower bound histogram. Or to the lower bound
     * of the constant range, if this is the final bin, containing the
     * constant lower bound.
     */
    if (range_cmp_bounds(typcache, &hist_lower[i], lower) < 0)
    {
      dist = get_distance(typcache, lower, upper);

      /*
       * Subtract from bin_width the portion of this bin that we want to
       * ignore.
       */
      bin_width -= get_position(typcache, lower, &hist_lower[i],
        &hist_lower[i + 1]);
      if (bin_width < 0.0)
        bin_width = 0.0;
      final_bin = true;
    }
    else
      dist = get_distance(typcache, &hist_lower[i], upper);

    /*
     * Estimate the fraction of tuples in this bin that are narrow enough
     * to not exceed the distance to the upper bound of the query range.
     */
    length_hist_frac = calc_length_hist_frac(length_hist_values,
      length_hist_nvalues, prev_dist, dist, true);

    /*
     * Add the fraction of tuples in this bin, with a suitable length, to
     * the total.
     */
    sum_frac += length_hist_frac * bin_width / (double) (hist_nvalues - 1);

    if (final_bin)
      break;

    bin_width = 1.0;
    prev_dist = dist;
  }

  return sum_frac;
}

/**
 * Calculate selectivity of "var @> const" operator, ie. estimate the fraction
 * of ranges that contain the constant lower and upper bounds. This uses
 * the histograms of range lower bounds and range lengths, on the assumption
 * that the range lengths are independent of the lower bounds.
 *
 * Note, this is "var @> const", ie. estimate the fraction of ranges that
 * contain the constant lower and upper bounds.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static double
calc_hist_selectivity_contains(TypeCacheEntry *typcache,
                 RangeBound *lower, RangeBound *upper,
                 RangeBound *hist_lower, int hist_nvalues,
                 Datum *length_hist_values, int length_hist_nvalues)
{
  int      i,
        lower_index;
  double    bin_width,
        lower_bin_width;
  double    sum_frac;
  float8    prev_dist;

  /* Find the bin containing the lower bound of query range. */
  lower_index = rbound_bsearch(typcache, lower, hist_lower, hist_nvalues,
                 true);

  /*
   * Calculate lower_bin_width, ie. the fraction of the of (lower_index,
   * lower_index + 1) bin which is greater than lower bound of query range
   * using linear interpolation of subdiff function.
   */
  if (lower_index >= 0 && lower_index < hist_nvalues - 1)
    lower_bin_width = get_position(typcache, lower, &hist_lower[lower_index],
                     &hist_lower[lower_index + 1]);
  else
    lower_bin_width = 0.0;

  /*
   * Loop through all the lower bound bins, smaller than the query lower
   * bound. In the loop, dist and prev_dist are the distance of the
   * "current" bin's lower and upper bounds from the constant upper bound.
   * We begin from query lower bound, and walk backwards, so the first bin's
   * upper bound is the query lower bound, and its distance to the query
   * upper bound is the length of the query range.
   *
   * bin_width represents the width of the current bin. Normally it is 1.0,
   * meaning a full width bin, except for the first bin, which is only
   * counted up to the constant lower bound.
   */
  prev_dist = get_distance(typcache, lower, upper);
  sum_frac = 0.0;
  bin_width = lower_bin_width;
  for (i = lower_index; i >= 0; i--)
  {
    float8    dist;
    double    length_hist_frac;

    /*
     * dist -- distance from upper bound of query range to current value
     * of lower bound histogram or lower bound of query range (if we've
     * reach it).
     */
    dist = get_distance(typcache, &hist_lower[i], upper);

    /*
     * Get average fraction of length histogram which covers intervals
     * longer than (or equal to) distance to upper bound of query range.
     */
    length_hist_frac =
      1.0 - calc_length_hist_frac(length_hist_values,
                    length_hist_nvalues,
                    prev_dist, dist, false);

    sum_frac += length_hist_frac * bin_width / (double) (hist_nvalues - 1);

    bin_width = 1.0;
    prev_dist = dist;
  }

  return sum_frac;
}

/**
 * Calculate range operator selectivity using histograms of range bounds.
 *
 * This estimate is for the portion of values that are not empty and not
 * NULL.
 * @note Function copied from rangetypes_selfuncs.c since it is not exported.
 */
static double
calc_hist_selectivity(TypeCacheEntry *typcache, VariableStatData *vardata,
            RangeType *constval, Oid operid)
{
  AttStatsSlot hslot;
  AttStatsSlot lslot;
  int      nhist;
  RangeBound *hist_lower;
  RangeBound *hist_upper;
  int      i;
  RangeBound  const_lower;
  RangeBound  const_upper;
  bool    empty;
  double    hist_selec;

  /* Can't use the histogram with insecure range support functions */
  if (!statistic_proc_security_check(vardata,
                     typcache->rng_cmp_proc_finfo.fn_oid))
    return -1;
  if (OidIsValid(typcache->rng_subdiff_finfo.fn_oid) &&
    !statistic_proc_security_check(vardata,
                     typcache->rng_subdiff_finfo.fn_oid))
    return -1;

  /* Try to get histogram of ranges */
  if (!(HeapTupleIsValid(vardata->statsTuple) &&
      get_attstatsslot(&hslot, vardata->statsTuple,
               STATISTIC_KIND_BOUNDS_HISTOGRAM, InvalidOid,
               ATTSTATSSLOT_VALUES)))
    return -1.0;

  /*
   * Convert histogram of ranges into histograms of its lower and upper
   * bounds.
   */
  nhist = hslot.nvalues;
  hist_lower = (RangeBound *) palloc(sizeof(RangeBound) * nhist);
  hist_upper = (RangeBound *) palloc(sizeof(RangeBound) * nhist);
  for (i = 0; i < nhist; i++)
  {
    range_deserialize(typcache, DatumGetRangeTypeP(hslot.values[i]),
              &hist_lower[i], &hist_upper[i], &empty);
    /* The histogram should not contain any empty ranges */
    if (empty)
      elog(ERROR, "bounds histogram contains an empty range");
  }

  /* @> and @< also need a histogram of range lengths */
  if (operid == OID_RANGE_CONTAINS_OP ||
    operid == OID_RANGE_CONTAINED_OP)
  {
    if (!(HeapTupleIsValid(vardata->statsTuple) &&
        get_attstatsslot(&lslot, vardata->statsTuple,
                 STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM,
                 InvalidOid,
                 ATTSTATSSLOT_VALUES)))
    {
      free_attstatsslot(&hslot);
      return -1.0;
    }

    /* check that it's a histogram, not just a dummy entry */
    if (lslot.nvalues < 2)
    {
      free_attstatsslot(&lslot);
      free_attstatsslot(&hslot);
      return -1.0;
    }
  }
  else
    memset(&lslot, 0, sizeof(lslot));

  /* Extract the bounds of the constant value. */
  range_deserialize(typcache, constval, &const_lower, &const_upper, &empty);
  assert(!empty);

  /*
   * Calculate selectivity comparing the lower or upper bound of the
   * constant with the histogram of lower or upper bounds.
   */
  switch (operid)
  {
    case OID_RANGE_LESS_OP:

      /*
       * The regular b-tree comparison operators (<, <=, >, >=) compare
       * the lower bounds first, and the upper bounds for values with
       * equal lower bounds. Estimate that by comparing the lower bounds
       * only. This gives a fairly accurate estimate assuming there
       * aren't many rows with a lower bound equal to the constant's
       * lower bound.
       */
      hist_selec =
        calc_hist_selectivity_scalar(typcache, &const_lower,
                       hist_lower, nhist, false);
      break;

    case OID_RANGE_LESS_EQUAL_OP:
      hist_selec =
        calc_hist_selectivity_scalar(typcache, &const_lower,
                       hist_lower, nhist, true);
      break;

    case OID_RANGE_GREATER_OP:
      hist_selec =
        1 - calc_hist_selectivity_scalar(typcache, &const_lower,
                         hist_lower, nhist, false);
      break;

    case OID_RANGE_GREATER_EQUAL_OP:
      hist_selec =
        1 - calc_hist_selectivity_scalar(typcache, &const_lower,
                         hist_lower, nhist, true);
      break;

    case OID_RANGE_LEFT_OP:
      /* var << const when upper(var) < lower(const) */
      hist_selec =
        calc_hist_selectivity_scalar(typcache, &const_lower,
                       hist_upper, nhist, false);
      break;

    case OID_RANGE_RIGHT_OP:
      /* var >> const when lower(var) > upper(const) */
      hist_selec =
        1 - calc_hist_selectivity_scalar(typcache, &const_upper,
                         hist_lower, nhist, true);
      break;

    case OID_RANGE_OVERLAPS_RIGHT_OP:
      /* compare lower bounds */
      hist_selec =
        1 - calc_hist_selectivity_scalar(typcache, &const_lower,
                         hist_lower, nhist, false);
      break;

    case OID_RANGE_OVERLAPS_LEFT_OP:
      /* compare upper bounds */
      hist_selec =
        calc_hist_selectivity_scalar(typcache, &const_upper,
                       hist_upper, nhist, true);
      break;

    case OID_RANGE_OVERLAP_OP:
    case OID_RANGE_CONTAINS_ELEM_OP:

      /*
       * A && B <=> NOT (A << B OR A >> B).
       *
       * Since A << B and A >> B are mutually exclusive events we can
       * sum their probabilities to find probability of (A << B OR A >>
       * B).
       *
       * "range @> elem" is equivalent to "range && [elem,elem]". The
       * caller already constructed the singular range from the element
       * constant, so just treat it the same as &&.
       */
      hist_selec =
        calc_hist_selectivity_scalar(typcache, &const_lower, hist_upper,
                       nhist, false);
      hist_selec +=
        (1.0 - calc_hist_selectivity_scalar(typcache, &const_upper, hist_lower,
                          nhist, true));
      hist_selec = 1.0 - hist_selec;
      break;

    case OID_RANGE_CONTAINS_OP:
      hist_selec =
        calc_hist_selectivity_contains(typcache, &const_lower,
                         &const_upper, hist_lower, nhist,
                         lslot.values, lslot.nvalues);
      break;

    case OID_RANGE_CONTAINED_OP:
      if (const_lower.infinite)
      {
        /*
         * Lower bound no longer matters. Just estimate the fraction
         * with an upper bound <= const upper bound
         */
        hist_selec =
          calc_hist_selectivity_scalar(typcache, &const_upper,
                         hist_upper, nhist, true);
      }
      else if (const_upper.infinite)
      {
        hist_selec =
          1.0 - calc_hist_selectivity_scalar(typcache, &const_lower,
                             hist_lower, nhist, false);
      }
      else
      {
        hist_selec =
          calc_hist_selectivity_contained(typcache, &const_lower,
                          &const_upper, hist_lower, nhist,
                          lslot.values, lslot.nvalues);
      }
      break;

    default:
      elog(ERROR, "unknown range operator %u", operid);
      hist_selec = -1.0;  /* keep compiler quiet */
      break;
  }

  free_attstatsslot(&lslot);
  free_attstatsslot(&hslot);

  return hist_selec;
}

/*****************************************************************************
 * Internal functions computing selectivity
 * The functions assume that the value and time dimensions of temporal values
 * are independent and thus the selectivity values obtained by analyzing the
 * histograms for each dimension can be multiplied.
 *****************************************************************************/

/**
 * Return the enum value associated to the operator
 */
bool
tnumber_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = LT_OP; i <= OVERAFTER_OP; i++)
  {
    if (/* Time types */
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TFLOAT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TFLOAT) ||
        /* Range types */
        operid == oper_oid((CachedOp) i, T_INTRANGE, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_FLOATRANGE, T_TFLOAT) ||
        /* Tbox type */
        operid == oper_oid((CachedOp) i, T_TBOX, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TBOX, T_TFLOAT) ||
        /* Tint type */
        operid == oper_oid((CachedOp) i, T_TINT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_INT4) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_FLOAT8) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_INTRANGE) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TBOX) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TINT, T_TFLOAT) ||
        /* Tfloat type */
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_INT4) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_FLOAT8) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_FLOATRANGE) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TBOX) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TINT) ||
        operid == oper_oid((CachedOp) i, T_TFLOAT, T_TFLOAT))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Transform the constant into a temporal box
 */
bool
tnumber_const_to_tbox(const Node *other, TBOX *box)
{
  Oid consttypid = ((Const *) other)->consttype;
  CachedType type = oid_type(consttypid);
  if (tnumber_basetype(type))
    number_tbox(((Const *) other)->constvalue, type, box);
  else if (tnumber_rangetype(type))
    range_tbox(DatumGetRangeTypeP(((Const *) other)->constvalue), box);
  else if (type == T_TIMESTAMPTZ)
    timestamp_tbox(DatumGetTimestampTz(((Const *) other)->constvalue), box);
  else if (type == T_TIMESTAMPSET)
    timestampset_tbox_slice(((Const *) other)->constvalue, box);
  else if (type == T_PERIOD)
    period_tbox(DatumGetPeriodP(((Const *) other)->constvalue), box);
  else if (type == T_PERIODSET)
    periodset_tbox_slice(((Const *) other)->constvalue, box);
  else if (type == T_TBOX)
    memcpy(box, DatumGetTboxP(((Const *) other)->constvalue), sizeof(TBOX));
  else if (tnumber_type(type))
    temporal_bbox(DatumGetTemporalP(((Const *) other)->constvalue), box);
  else
    return false;
  return true;
}

/**
 * Return the range operator associated to the enum value
 */
static Oid
tnumber_rangeop(CachedOp cachedOp)
{
  Oid operid = InvalidOid;
  if (cachedOp == LT_OP)
    operid = OID_RANGE_LESS_OP;
  else if (cachedOp == LE_OP)
    operid = OID_RANGE_LESS_EQUAL_OP;
  else if (cachedOp == GE_OP)
    operid = OID_RANGE_GREATER_EQUAL_OP;
  else if (cachedOp == GT_OP)
    operid = OID_RANGE_GREATER_OP;
  else if (cachedOp == OVERLAPS_OP)
    operid = OID_RANGE_OVERLAP_OP;
  else if (cachedOp == CONTAINS_OP)
    operid = OID_RANGE_CONTAINS_OP;
  else if (cachedOp == CONTAINED_OP)
    operid = OID_RANGE_CONTAINED_OP;
  else if (cachedOp == LEFT_OP)
    operid = OID_RANGE_LEFT_OP;
  else if (cachedOp == RIGHT_OP)
    operid = OID_RANGE_RIGHT_OP;
  else if (cachedOp == OVERLEFT_OP)
    operid = OID_RANGE_OVERLAPS_LEFT_OP;
  else if (cachedOp == OVERRIGHT_OP)
    operid = OID_RANGE_OVERLAPS_RIGHT_OP;
  /* There is no ~= for ranges but we cover this operator explicity */
  else if (cachedOp == SAME_OP)
    operid = OID_RANGE_OVERLAPS_RIGHT_OP;
  return operid;
}

/**
 * Return a default selectivity estimate for the operator when we don't
 * have statistics or cannot use them for some reason.
 */
float8
tnumber_sel_default(CachedOp operator)
{
  switch (operator)
  {
    case OVERLAPS_OP:
      return 0.005;

    case CONTAINS_OP:
    case CONTAINED_OP:
      return 0.002;

    case SAME_OP:
      return 0.001;

    case LEFT_OP:
    case RIGHT_OP:
    case OVERLEFT_OP:
    case OVERRIGHT_OP:
    case ABOVE_OP:
    case BELOW_OP:
    case OVERABOVE_OP:
    case OVERBELOW_OP:
    case AFTER_OP:
    case BEFORE_OP:
    case OVERAFTER_OP:
    case OVERBEFORE_OP:

      /* these are similar to regular scalar inequalities */
      return DEFAULT_INEQ_SEL;

    default:
      /* all operators should be handled above, but just in case */
      return 0.001;
  }
}

/**
 * Return an estimate of the selectivity of the temporal search box and the
 * operator for columns of temporal numbers. For the traditional comparison
 * operators (<, <=, ...) we follow the approach for range types in
 * PostgreSQL, this function computes the selectivity for <, <=, >, and >=,
 * while the selectivity functions for = and <> are eqsel and neqsel,
 * respectively.
 */
Selectivity
tnumber_sel_box(VariableStatData *vardata, TBOX *box, CachedOp cachedOp,
  Oid basetypid)
{
  Period period;
  RangeType *range = NULL;
  TypeCacheEntry *typcache;
  double selec;
  Oid rangetypid, value_oprid, period_oprid;

  /* Enable the multiplication of the selectivity of the value and time
   * dimensions since either may be missing */
  selec = 1.0;

  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    /* Fetch the range operator corresponding to the cachedOp */
    value_oprid = tnumber_rangeop(cachedOp);
    /* If the corresponding range operator is not found */
    if (value_oprid != InvalidOid)
    {
      CachedType basetype = oid_type(basetypid);
      range = range_make(Float8GetDatum(box->xmin),
        Float8GetDatum(box->xmax), true, true, basetype);
      rangetypid = basetype_rangeoid(basetype);
      typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);
    }
  }
  if (MOBDB_FLAGS_GET_T(box->flags))
    period_set(box->tmin, box->tmax, true, true, &period);

  /*
   * There is no ~= operator for range/time types and thus it is necessary to
   * take care of this operator here.
   */
  if (cachedOp == SAME_OP)
  {
    /* Selectivity for the value dimension */
    if (MOBDB_FLAGS_GET_X(box->flags))
    {
      value_oprid = oper_oid(EQ_OP, basetypid, basetypid);
#if POSTGRESQL_VERSION_NUMBER < 130000
      selec *= var_eq_const(vardata, value_oprid, PointerGetDatum(range),
        false, false, false);
#else
      selec *= var_eq_const(vardata, value_oprid, DEFAULT_COLLATION_OID,
        PointerGetDatum(range), false, false, false);
#endif
    }
    /* Selectivity for the time dimension */
    if (MOBDB_FLAGS_GET_T(box->flags))
    {
      period_oprid = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
#if POSTGRESQL_VERSION_NUMBER < 130000
      selec *= var_eq_const(vardata, period_oprid, PeriodPGetDatum(&period),
        false, false, false);
#else
      selec *= var_eq_const(vardata, period_oprid, DEFAULT_COLLATION_OID,
        PeriodPGetDatum(&period), false, false, false);
#endif
    }
  }
  else if (cachedOp == OVERLAPS_OP || cachedOp == CONTAINS_OP ||
    cachedOp == CONTAINED_OP ||
    /* For b-tree comparisons, temporal values are first compared wrt
     * their bounding boxes, and if these are equal, other criteria apply.
     * For selectivity estimation we approximate by taking into account
     * only the bounding boxes. */
    cachedOp == LT_OP || cachedOp == LE_OP ||
    cachedOp == GT_OP || cachedOp == GE_OP)
  {
    /* Selectivity for the value dimension */
    if (MOBDB_FLAGS_GET_X(box->flags) && range != NULL)
      selec *= calc_hist_selectivity(typcache, vardata, range,
        value_oprid);
    /* Selectivity for the time dimension */
    if (MOBDB_FLAGS_GET_T(box->flags))
      selec *= period_sel_hist(vardata, &period, cachedOp);
  }
  else if (cachedOp == LEFT_OP || cachedOp == RIGHT_OP ||
    cachedOp == OVERLEFT_OP || cachedOp == OVERRIGHT_OP)
  {
    /* Selectivity for the value dimension */
    if (MOBDB_FLAGS_GET_X(box->flags) && range != NULL)
      selec *= calc_hist_selectivity(typcache, vardata, range,
        value_oprid);
  }
  else if (cachedOp == BEFORE_OP || cachedOp == AFTER_OP ||
    cachedOp == OVERBEFORE_OP || cachedOp == OVERAFTER_OP)
  {
    /* Selectivity for the value dimension */
    if (MOBDB_FLAGS_GET_T(box->flags))
      selec *= period_sel_hist(vardata, &period, cachedOp);
  }
  else /* Unknown operator */
  {
    selec = tnumber_sel_default(cachedOp);
  }
  if (range != NULL)
    pfree(range);
  return selec;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tnumber_sel);
/**
 * Estimate the selectivity value of the operators for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_sel(PG_FUNCTION_ARGS)
{
  return temporal_sel_ext(fcinfo, TNUMBERTYPE);
}

/*****************************************************************************/

/**
 * Return a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
tnumber_joinsel_default(CachedOp cachedOp __attribute__((unused)))
{
  // TODO take care of the operator
  return 0.001;
}

/**
 * Depending on the operator and the arguments, determine wheter the value,
 * the time, or both components are taken into account for computing the
 * join selectivity
 */
bool
tnumber_joinsel_components(CachedOp cachedOp, CachedType oprleft,
  CachedType oprright, bool *value, bool *time)
{
  /* Get the argument which may not a temporal number */
  CachedType arg = tnumber_type(oprleft) ? oprright : oprleft;

  /* Determine the components */
  if (tnumber_basetype(arg) || tnumber_rangetype(arg) ||
    cachedOp == LEFT_OP || cachedOp == OVERLEFT_OP ||
    cachedOp == RIGHT_OP || cachedOp == OVERRIGHT_OP)
  {
    *value = true;
    *time = false;
  }
  else if (time_type(arg) ||
    cachedOp == BEFORE_OP || cachedOp == OVERBEFORE_OP ||
    cachedOp == AFTER_OP || cachedOp == OVERAFTER_OP)
  {
    *value = false;
    *time = true;
  }
  else if (tnumber_type(arg) && (cachedOp == OVERLAPS_OP ||
    cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP ||
    cachedOp == SAME_OP || cachedOp == ADJACENT_OP))
  {
    *value = true;
    *time = true;
  }
  else
  {
    /* By default only the time component is taken into account */
    *value = false;
    *time = true;
  }
  return true;
}

PG_FUNCTION_INFO_V1(Tnumber_joinsel);
/**
 * Estimate the join selectivity value of the operators for temporal numbers
 */
PGDLLEXPORT Datum
Tnumber_joinsel(PG_FUNCTION_ARGS)
{
  return temporal_joinsel_ext(fcinfo, TNUMBERTYPE);
}

/*****************************************************************************/
