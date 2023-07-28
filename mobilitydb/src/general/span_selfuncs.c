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
 * @brief Functions for selectivity estimation of time types operators.
 *
 * These functions are based on those of the file `rangetypes_selfuncs.c`.
 * Estimates are based on histograms of lower and upper bounds.
 */

#include "pg_general/span_selfuncs.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <access/htup_details.h>
#include "utils/syscache.h"
#include <utils/lsyscache.h>
#include <catalog/pg_statistic.h>
#include <utils/timestamp.h>
#include <math.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span_analyze.h"

/*****************************************************************************/

/**
 * @brief Return a default selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
span_sel_default(meosOper oper __attribute__((unused)))
{
  // TODO take care of the operator
  return DEFAULT_TEMP_SEL;
}

/**
 * @brief Return a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
span_joinsel_default(meosOper oper __attribute__((unused)))
{
  // TODO take care of the operator
  return DEFAULT_TEMP_JOINSEL;
}

/**
 * @brief Determine whether we can estimate selectivity for the operator
 */
static bool
value_oper_sel(Oid operid __attribute__((unused)), meosType ltype,
  meosType rtype)
{
  if ((numspan_basetype(ltype) || numset_type(ltype) || numspan_type(ltype) ||
        spanset_type(ltype)) &&
      (numspan_basetype(rtype) || numset_type(rtype) || numspan_type(rtype) ||
        spanset_type(rtype)))
    return true;
  return false;
}

/**
 * @brief Determine whether we can estimate selectivity for the operator
 */
bool
time_oper_sel(meosOper oper __attribute__((unused)), meosType ltype, meosType rtype)
{
  if ((timespan_basetype(ltype) || timeset_type(ltype) || timespan_type(ltype) ||
        timespanset_type(ltype)) &&
      (span_basetype(rtype) || timeset_type(rtype) || timespan_type(ltype) ||
        timespanset_type(rtype)))
    return true;
  return false;
}

/*****************************************************************************/

/**
 * @brief Binary search on an array of span bounds.
 *
 * Return the greatest index of span bound in array which is less (less or
 * equal) than given span bound. If all span bounds in array are greater or
 * equal (greater) than given span bound, return -1. When "equal" flag is set,
 * the conditions in parenthesis are used.
 *
 * This function is used for scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop interpolation
 * of portion of bounds which are less or equal to given bound.
 */
static int
span_bound_bsearch(const SpanBound *value, const SpanBound *hist,
  int hist_nvalues, bool equal)
{
  int lower = -1, upper = hist_nvalues - 1;
  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    int cmp = span_bound_cmp(&hist[middle], value);
    if (cmp < 0 || (equal && cmp == 0))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/**
 * @brief Measure distance between two span bounds
 */
static float8
span_bound_distance(const SpanBound *bound1, const SpanBound *bound2)
{
  return distance_value_value(bound2->val, bound1->val, bound2->basetype,
    bound1->basetype);
}

/**
 * @brief Get relative position of value in histogram bin in [0,1] span
 */
static float8
span_position(const SpanBound *value, const SpanBound *hist1,
  const SpanBound *hist2)
{
  /* Calculate relative position using distance function. */
  float8 bin_width = span_bound_distance(hist1, hist2);
  if (bin_width <= 0.0)
    return 0.5;      /* zero width bin */

  float8 position = span_bound_distance(hist1, value) / bin_width;
  /* Relative position must be in [0,1] span */
  position = Max(position, 0.0);
  position = Min(position, 1.0);
  return position;
}

/*****************************************************************************/
/**
 * @brief Binary search on length histogram.
 *
 * Return greatest index of period length in histogram which is less than (less
 * than or equal) the given length value. If all lengths in the histogram are
 * greater than (greater than or equal) the given length, returns -1.
 * @note Function copied from file rangetypes_selfuncs.c snce it is not exported
 */
int
length_hist_bsearch(Datum *hist_length, int hist_length_nvalues,
  double value, bool equal)
{
  int lower = -1, upper = hist_length_nvalues - 1;
  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    double middleval = DatumGetFloat8(hist_length[middle]);
    if (middleval < value || (equal && middleval <= value))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/**
 * @brief Get relative position of value in a length histogram bin in [0,1] range.
 * @note Function derived from PostgreSQL file rangetypes_selfuncs.c.
 */
double
get_len_position(double value, double hist1, double hist2)
{
  assert(hist1 != hist2);
  return 1.0 - (hist2 - value) / (hist2 - hist1);
}

/**
 * @brief Calculate the average of function P(x), in the interval
 * [length1, length2], where P(x) is the fraction of tuples with length < x
 * (or length <= x if 'equal' is true).
 * @note Function derived from PostgreSQL file rangetypes_selfuncs.c.
 */
double
calc_length_hist_frac(Datum *hist_length, int hist_length_nvalues,
  double length1, double length2, bool equal)
{
  double frac, A, B, PA, PB, pos, area;
  int i;

  assert(length2 >= length1);

  if (length2 < 0.0)
    return 0.0;        /* shouldn't happen, but doesn't hurt to check */

  /* All lengths in the table are <= infinite. */
  if (isinf(length2) && equal)
    return 1.0;

  /*----------
   * The average of a function between A and B can be calculated by the
   * formula:
   *
   *          B
   *    1     /
   * -------  | P(x)dx
   *  B - A   /
   *          A
   *
   * The geometrical interpretation of the integral is the area under the
   * graph of P(x). P(x) is defined by the length histogram. We calculate
   * the area in a piecewise fashion, iterating through the length histogram
   * bins. Each bin is a trapezoid:
   *
   *       P(x2)
   *        /|
   *       / |
   * P(x1)/  |
   *     |   |
   *     |   |
   *  ---+---+--
   *     x1  x2
   *
   * where x1 and x2 are the boundaries of the current histogram, and P(x1)
   * and P(x1) are the cumulative fraction of tuples at the boundaries.
   *
   * The area of each trapezoid is 1/2 * (P(x2) + P(x1)) * (x2 - x1)
   *
   * The first bin contains the lower bound passed by the caller, so we
   * use linear interpolation between the previous and next histogram bin
   * boundary to calculate P(x1). Likewise for the last bin: we use linear
   * interpolation to calculate P(x2). For the bins in between, x1 and x2
   * lie on histogram bin boundaries, so P(x1) and P(x2) are simply:
   * P(x1) =    (bin index) / (number of bins)
   * P(x2) = (bin index + 1 / (number of bins)
   */

  /* First bin, the one that contains lower bound */
  i = length_hist_bsearch(hist_length, hist_length_nvalues, length1, equal);
  if (i >= hist_length_nvalues - 1)
    return 1.0;

  if (i < 0)
  {
    i = 0;
    pos = 0.0;
  }
  else
  {
    /* Interpolate length1's position in the bin */
    pos = get_len_position(length1, DatumGetFloat8(hist_length[i]),
      DatumGetFloat8(hist_length[i + 1]));
  }
  PB = (((double) i) + pos) / (double) (hist_length_nvalues - 1);
  B = length1;

  /*
   * In the degenerate case that length1 == length2, simply return
   * P(length1). This is not merely an optimization: if length1 == length2,
   * we'd divide by zero later on.
   */
  if (length2 == length1)
    return PB;

  /*
   * Loop through all the bins, until we hit the last bin, the one that
   * contains the upper bound. (if lower and upper bounds are in the same
   * bin, this falls out immediately)
   */
  area = 0.0;
  for (; i < hist_length_nvalues - 1; i++)
  {
    double bin_upper = DatumGetFloat8(hist_length[i + 1]);

    /* Check if we've reached the last bin */
    if (!(bin_upper < length2 || (equal && bin_upper <= length2)))
      break;

    /* the upper bound of previous bin is the lower bound of this bin */
    A = B;
    PA = PB;

    B = bin_upper;
    PB = (double) i / (double) (hist_length_nvalues - 1);

    /*
     * Add the area of this trapezoid to the total. The point of the
     * if-check is to avoid NaN, in the corner case that PA == PB == 0,
     * and B - A == Inf. The area of a zero-height trapezoid (PA == PB ==
     * 0) is zero, regardless of the width (B - A).
     */
    if (PA > 0 || PB > 0)
      area += 0.5 * (PB + PA) * (B - A);
  }

  /* Last bin */
  A = B;
  PA = PB;

  B = length2;        /* last bin ends at the constant upper bound */
  if (i >= hist_length_nvalues - 1)
    pos = 0.0;
  else
  {
    if (DatumGetFloat8(hist_length[i]) ==
        DatumGetFloat8(hist_length[i + 1]))
      pos = 0.0;
    else
      pos = get_len_position(length2, DatumGetFloat8(hist_length[i]),
        DatumGetFloat8(hist_length[i + 1]));
  }
  PB = (((double) i) + pos) / (double) (hist_length_nvalues - 1);

  if (PA > 0 || PB > 0)
    area += 0.5 * (PB + PA) * (B - A);

  /*
   * Ok, we have calculated the area, i.e. the integral. Divide by width to
   * get the requested average.
   *
   * Avoid NaN arising from infinite / infinite. This happens at least if
   * length2 is infinite. It's not clear what the correct value would be in
   * that case, so 0.5 seems as good as any value.
   */
  if (isinf(area) && isinf(length2))
    frac = 0.5;
  else
    frac = area / (length2 - length1);

  return frac;
}

/*****************************************************************************/

/**
 * @brief Estimate the fraction of values less than (or equal to, if 'equal'
 * argument is true) a given const in a histogram of span bounds.
 */
static double
span_sel_scalar(const SpanBound *constbound, const SpanBound *hist,
  int nhist, bool equal)
{
  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  int index = span_bound_bsearch(constbound, hist, nhist, equal);
  Selectivity selec = (Selectivity) (Max(index, 0)) /
    (Selectivity) (nhist - 1);

  /* Adjust using linear interpolation within the bin */
  if (index >= 0 && index < nhist - 1)
    selec += span_position(constbound, &hist[index], &hist[index + 1]) /
      (Selectivity) (nhist - 1);

  return selec;
}

/**
 * @brief Calculate selectivity of "var && const" operator, i.e., estimate the
 * fraction of spans that overlap the constant lower and upper bounds. This
 * uses the histograms of span lower and upper bounds.
 *
 * Note that A && B <=> NOT (A <<# B OR A #>> B).
 *
 * Since A <<# B and A #>> B are mutually exclusive events we can
 * sum their probabilities to find probability of (A <<# B OR A #>> B).
 *
 * "(span/spanset) @> timestamptz" is equivalent to "span && [elem,elem]".
 * The caller already constructed the singular span from the element
 * constant, so just treat it the same as &&.
 */
static double
span_sel_overlaps(const SpanBound *const_lower, const SpanBound *const_upper,
  const SpanBound *hist_lower, const SpanBound *hist_upper, int nhist)
{
  /* If the spans do not overlap return 0.0 */
  if (span_bound_cmp(const_lower, &hist_upper[nhist - 1]) > 0 ||
      span_bound_cmp(&hist_lower[0], const_upper) > 0)
    return 0.0;

  /*
   * Sel = 1.0 - ( Sel(A <<# B) + Sel(A #>> B) )
   * Since
   * A << B when upper(A) < lower(B) and
   * A >> B when lower(A) > upper(B) then
   * Sel = 1.0 - ( Sel(upper(A) < lower(B)) + Sel(lower(A) > upper(B)) )
   * Sel = 1.0 - ( Sel(upper(A) < lower(B)) + ( 1 - Sel(lower(A) <= upper(B)) ) )
   */
  //EZ
  // double selec = span_sel_scalar(const_upper, hist_lower, nhist, false);
  // selec += (1.0 - span_sel_scalar(const_lower, hist_upper, nhist, true));
  // selec = 1.0 - selec;

  double selec = span_sel_scalar(const_lower, hist_upper, nhist, false);
  selec += (1.0 - span_sel_scalar(const_upper, hist_lower, nhist, true));
  selec = 1.0 - selec;
  return selec;
}

/**
 * @brief Calculate selectivity of "var <@ const" operator, i.e., estimate the
 * fraction of spans that fall within the constant lower and upper bounds.
 *
 * This uses the histograms of span lower bounds and span lengths, on the
 * assumption that the span lengths are independent of the lower bounds.
 */
static double
span_sel_contained(SpanBound *const_lower, SpanBound *const_upper,
  SpanBound *hist_lower, int hist_nvalues, Datum *hist_length,
  int hist_length_nvalues)
{
  int i, upper_index;
  float8 prev_dist;
  double bin_width, upper_bin_width, sum_frac;

  /*
   * Begin by finding the bin containing the upper bound, in the lower bound
   * histogram. Any span with a lower bound > constant upper bound can't
   * match, i.e. there are no matches in bins greater than upper_index.
   */
  const_upper->inclusive = ! const_upper->inclusive;
  const_upper->lower = true;
  upper_index = span_bound_bsearch(const_upper, hist_lower, hist_nvalues,
    false);

  /*
   * Calculate upper_bin_width, i.e., the fraction of the (upper_index,
   * upper_index + 1) bin which is greater than constant upper bound
   * using linear interpolation of distance function.
   */
  if (upper_index >= 0 && upper_index < hist_nvalues - 1)
    upper_bin_width = span_position(const_upper, &hist_lower[upper_index],
      &hist_lower[upper_index + 1]);
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
  for (i = upper_index - 1; i >= 0; i--)
  {
    double dist, hist_length_frac;
    bool final_bin = false;

    /*
     * dist -- distance from constant upper bound to the lower bound of
     * the current bin in the lower bound histogram. Or to the lower bound
     * of the constant span, if this is the final bin, containing the
     * constant lower bound.
     */
    if (span_bound_cmp(&hist_lower[i], const_lower) < 0)
    {
      dist = span_bound_distance(const_lower, const_upper);

      /*
       * Subtract from bin_width the portion of this bin that we want to
       * ignore.
       */
      bin_width -= span_position(const_lower, &hist_lower[i],
        &hist_lower[i + 1]);
      if (bin_width < 0.0)
        bin_width = 0.0;
      final_bin = true;
    }
    else
      dist = span_bound_distance(&hist_lower[i], const_upper);

    /*
     * Estimate the fraction of tuples in this bin that are narrow enough
     * to not exceed the distance to the constant upper bound.
     */
    hist_length_frac = calc_length_hist_frac(hist_length,
      hist_length_nvalues, prev_dist, dist, true);

    /*
     * Add the fraction of tuples in this bin, with a suitable length, to
     * the total.
     */
    sum_frac += hist_length_frac * bin_width / (double) (hist_nvalues - 1);

    if (final_bin)
      break;

    bin_width = 1.0;
    prev_dist = dist;
  }

  return sum_frac;
}

/**
 * @brief Calculate selectivity of "var @> const" operator, i.e., estimate the
 * fraction of spans that contain the constant lower and upper bounds.
 *
 * This uses the histograms of span lower bounds and span lengths, on the
 * assumption that the span lengths are independent of the lower bounds.
 */
static double
span_sel_contains(SpanBound *const_lower, SpanBound *const_upper,
  SpanBound *hist_lower, int hist_nvalues, Datum *hist_length,
  int hist_length_nvalues)
{
  int i, lower_index;
  double bin_width, lower_bin_width, sum_frac;
  float8 prev_dist;

  /* Find the bin containing the constant lower bound */
  lower_index = span_bound_bsearch(const_lower, hist_lower, hist_nvalues, true);

  /*
   * Calculate lower_bin_width, i.e., the fraction of the (lower_index,
   * lower_index + 1) bin which is greater than the constant lower bound
   * using linear interpolation of distance function.
   */
  if (lower_index >= 0 && lower_index < hist_nvalues - 1)
    lower_bin_width = span_position(const_lower, &hist_lower[lower_index],
      &hist_lower[lower_index + 1]);
  else
    lower_bin_width = 0.0;

  /*
   * Loop through all the lower bound bins, smaller than the constant lower
   * bound. In the loop, dist and prev_dist are the distance of the
   * "current" bin's lower and upper bounds from the constant upper bound.
   * We begin from constant lower bound, and walk afterwards, so the first bin's
   * upper bound is the constant lower bound, and its distance to the constant
   * upper bound is the length of the constant span.
   *
   * bin_width represents the width of the current bin. Normally it is 1.0,
   * meaning a full width bin, except for the first bin, which is only
   * counted up to the constant lower bound.
   */
  prev_dist = span_bound_distance(const_lower, const_upper);
  sum_frac = 0.0;
  bin_width = lower_bin_width;
  for (i = lower_index; i >= 0; i--)
  {
    float8 dist;
    double hist_length_frac;

    /*
     * dist -- distance from constant upper bound to the current value
     * of lower bound histogram or constant lower bound (if we've
     * reach it).
     */
    dist = span_bound_distance(&hist_lower[i], const_upper);

    /*
     * Get average fraction of length histogram which covers intervals
     * longer than (or equal to) distance to the constant upper bound.
     */
    hist_length_frac = 1.0 - calc_length_hist_frac(hist_length,
      hist_length_nvalues, prev_dist, dist, false);

    sum_frac += hist_length_frac * bin_width / (double) (hist_nvalues - 1);

    bin_width = 1.0;
    prev_dist = dist;
  }

  return sum_frac;
}

/*****************************************************************************/

/**
 * @brief Calculate span operator selectivity using histograms of span bounds.
 * @note Used by the selectivity functions and the debugging functions.
 */
static double
span_sel_hist1(AttStatsSlot *hslot, AttStatsSlot *lslot, const Span *constval,
  meosOper oper)
{
  SpanBound *hist_lower, *hist_upper;
  SpanBound const_lower, const_upper;
  double selec;
  int nhist, i;

  /*
   * Convert histogram of spans into histograms of its lower and upper
   * bounds.
   */
  nhist = hslot->nvalues;
  hist_lower = palloc(sizeof(SpanBound) * nhist);
  hist_upper = palloc(sizeof(SpanBound) * nhist);
  for (i = 0; i < nhist; i++)
      span_deserialize(DatumGetSpanP(hslot->values[i]),
        &hist_lower[i], &hist_upper[i]);

  /* Extract the bounds of the constant value. */
  span_deserialize(constval, &const_lower, &const_upper);

  /*
   * Calculate the restriction selectivity of the various operators.
   *
   * The regular B-tree comparison operators (<, <=, >, >=) compare
   * the lower bounds first, and then the upper bounds for values with
   * equal lower bounds. Estimate that by comparing the lower bounds
   * only. This gives a fairly accurate estimate assuming there
   * aren't many rows with a lower bound equal to the constant's
   * lower bound. These operators work only for the span,span
   * combination of parameters. This is because the B-tree stores the
   * values, not their bounding boxes.
   *
   * For the relative position operators (<<#, &<#, #>>, #>&) which are
   * based on bounding boxes the selectivity is estimated by determining
   * the fraction of values less than (or less than or equal to) a given
   * constant in the histograms of span bounds.
   *
   * The other operators (&&, @>, <@, and -|-) have specific procedures
   * above.
   */
  if (oper == LT_OP)
    selec = span_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (oper == LE_OP)
    selec = span_sel_scalar(&const_lower, hist_lower, nhist, true);
  else if (oper == GT_OP)
    selec = 1.0 - span_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (oper == GE_OP)
    selec = 1.0 - span_sel_scalar(&const_lower, hist_lower, nhist, true);
  else if (oper == LEFT_OP || oper == BEFORE_OP)
    /* var <<# const when upper(var) < lower(const)*/
    selec = span_sel_scalar(&const_lower, hist_upper, nhist, false);
  else if (oper == OVERLEFT_OP || oper == OVERBEFORE_OP)
    /* var &<# const when upper(var) <= upper(const) */
    selec = span_sel_scalar(&const_upper, hist_upper, nhist, true);
  else if (oper == RIGHT_OP || oper == AFTER_OP)
    /* var #>> const when lower(var) > upper(const) */
    selec = 1.0 - span_sel_scalar(&const_upper, hist_lower, nhist, true);
  else if (oper == OVERRIGHT_OP || oper == OVERAFTER_OP)
    /* var #&> const when lower(var) >= lower(const)*/
    selec = 1.0 - span_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (oper == OVERLAPS_OP)
    selec = span_sel_overlaps(&const_lower, &const_upper, hist_lower,
      hist_upper, nhist);
  else if (oper == CONTAINS_OP)
    selec = span_sel_contains(&const_lower, &const_upper, hist_lower,
      nhist, lslot->values, lslot->nvalues);
  else if (oper == CONTAINED_OP)
    selec = span_sel_contained(&const_lower, &const_upper, hist_lower,
      nhist, lslot->values, lslot->nvalues);
  else if (oper == ADJACENT_OP)
    // TODO Analyze whether a similar approach as PostgreSQL selectivity
    // estimation for equality can be used. There, they estimate 1/n if
    // the value is not in the MCV
    selec = span_sel_default(InvalidOid);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown span operator");
    selec = -1.0;  /* keep compiler quiet */
  }

  pfree(hist_lower); pfree(hist_upper);

  return selec;
}

/**
 * @brief Calculate span operator selectivity using histograms of span bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
span_sel_hist(VariableStatData *vardata, const Span *constval, meosOper oper,
  bool value)
{
  AttStatsSlot hslot, lslot;
  double selec;

  memset(&hslot, 0, sizeof(hslot));

  int stats_kind = value ?
    STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM :
    STATISTIC_KIND_TIME_BOUNDS_HISTOGRAM;
  /* Try to get histogram of span bounds of vardata */
  if (!(HeapTupleIsValid(vardata->statsTuple) &&
      get_attstatsslot(&hslot, vardata->statsTuple,
        stats_kind, InvalidOid, ATTSTATSSLOT_VALUES)))
    return -1.0;
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot.nvalues < 2)
  {
    free_attstatsslot(&hslot);
    return -1.0;
  }

  /* @> and @< also need a histogram of span lengths */
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
  {
    memset(&lslot, 0, sizeof(lslot));

    stats_kind = value ?
      STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM :
      STATISTIC_KIND_TIME_LENGTH_HISTOGRAM;
    if (!(HeapTupleIsValid(vardata->statsTuple) &&
        get_attstatsslot(&lslot, vardata->statsTuple,
          stats_kind, InvalidOid, ATTSTATSSLOT_VALUES)))
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

  selec = span_sel_hist1(&hslot, &lslot, constval, oper);

  free_attstatsslot(&hslot);
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
    free_attstatsslot(&lslot);

  // elog(WARNING, "Selectivity: %lf", selec);
  return selec;
}

/*****************************************************************************/

/**
 * @brief Transform the constant into a span
 */
void
span_const_to_span(Node *other, Span *span)
{
  Oid consttype = ((Const *) other)->consttype;
  meosType type = oid_type(consttype);
  assert(span_basetype(type) || set_span_type(type) || span_type(type) ||
    spanset_type(type) || talpha_type(type));
  if (span_basetype(type))
  {
    /* The right argument is a set or span base constant. We convert it into
     * a singleton span */
    Datum value = ((Const *) other)->constvalue;
    span_set(value, value, true, true, type, span);
  }
  else if (set_span_type(type))
  {
    /* The right argument is a set constant. We convert it into
     * its bounding span. */
    const Set *s = DatumGetSetP(((Const *) other)->constvalue);
    set_set_span(s, span);
  }
  else if(span_type(type))
  {
    /* The right argument is a span constant. We convert it into
     * its bounding span. */
    const Span *s = DatumGetSpanP(((Const *) other)->constvalue);
    memcpy(span, s, sizeof(Span));
  }
  else if (spanset_type(type))
  {
    /* The right argument is a set constant. We convert it into
     * its bounding span. */
    const SpanSet *s = DatumGetSpanSetP(((Const *) other)->constvalue);
    memcpy(span, &s->span, sizeof(Span));
  }
  return;
}

/**
 * @brief Restriction selectivity for span operators
 */
float8
span_sel(PlannerInfo *root, Oid operid, List *args, int varRelid)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  Span span;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (! get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
    return span_sel_default(operid);

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (! IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    return span_sel_default(operid);
  }

  /*
   * All the span operators are strict, so we can cope with a NULL constant
   * right away.
   */
  if (((Const *) other)->constisnull)
  {
    ReleaseVariableStats(vardata);
    return 0.0;
  }

  /*
   * If var is on the right, commute the operator, so that we can assume the
   * var is on the left in what follows.
   */
  if (! varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operid = get_commutator(operid);
    if (! operid)
    {
      /* TODO: check whether there might still be a way to estimate.
       * Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      return span_sel_default(operid);
    }
  }

  /*
   * OK, there's a Var and a Const we're dealing with here. If the constant
   * is not of the span type, it should be converted to a span.
   */
  span_const_to_span(other, &span);
  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  bool value = value_oper_sel(oper, ltype, rtype);
  if (! value)
  {
    bool time = time_oper_sel(oper, ltype, rtype);
    if (! time)
    {
      /* Unknown operator */
      ReleaseVariableStats(vardata);
      return span_sel_default(operid);
    }
  }

  /*
   * Estimate using statistics. Note that span_sel need not handle
   * PERIOD_ELEM_CONTAINED_OP.
   */
  float4 null_frac;

  /* First look up the fraction of NULLs spans from pg_statistic. */
  if (HeapTupleIsValid(vardata.statsTuple))
  {
    Form_pg_statistic stats =
      (Form_pg_statistic) GETSTRUCT(vardata.statsTuple);
    null_frac = stats->stanullfrac;
  }
  else
  {
    /*
     * No stats are available. Follow through the calculations below
     * anyway, assuming no NULLs spans. This still allows us
     * to give a better-than-nothing estimate.
     */
    null_frac = 0.0;
  }

  /*
   * Calculate selectivity using bound histograms. If that fails for
   * some reason, e.g. no histogram in pg_statistic, use the default
   * constant estimate. This is still somewhat better than just
   * returning the default estimate, because this still takes into
   * account the fraction of NULL tuples, if we had statistics for them.
   */
  float8 hist_selec = span_sel_hist(&vardata, &span, oper, value);
  if (hist_selec < 0.0)
    hist_selec = span_sel_default(operid);

  selec = hist_selec;

  /* All span operators are strict */
  selec *= (1.0 - null_frac);

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PGDLLEXPORT Datum Span_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_sel);
/**
 * @brief Restriction selectivity for span operators
 */
Datum
Span_sel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);
  float8 selec = span_sel(root, operid, args, varRelid);
  PG_RETURN_FLOAT8((float8) selec);
}

/*****************************************************************************/

PGDLLEXPORT Datum _mobdb_span_sel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(_mobdb_span_sel);
/**
 * @brief Utility function to read the calculated selectivity for a given
 * table/column, operator, and search span.
 * Used for debugging the selectivity code.
 */
Datum
_mobdb_span_sel(PG_FUNCTION_ARGS)
{
  Oid table_oid = PG_GETARG_OID(0);
  text *att_text = PG_GETARG_TEXT_P(1);
  Oid operid = PG_GETARG_OID(2);
  Span *s = PG_GETARG_SPAN_P(3);
  float8 selec = 0.0;

  /* Test input parameters */
  char *table_name = get_rel_name(table_oid);
  if (table_name == NULL)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_TABLE),
      errmsg("Oid %u does not refer to a table", table_oid)));
  const char *att_name = text_to_cstring(att_text);
  AttrNumber att_num;
  /* We know the name? Look up the num */
  if (att_text)
  {
    /* Get the attribute number */
    att_num = get_attnum(table_oid, att_name);
    if (! att_num)
      elog(ERROR, "attribute \"%s\" does not exist", att_name);
  }
  else
    elog(ERROR, "attribute name is null");

  /* Determine whether we target the value or the time dimension */
  bool value = (s->basetype != T_TIMESTAMPTZ);
  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  bool found = value ?
    value_oper_sel(oper, ltype, rtype) : time_oper_sel(oper, ltype, rtype);
  if (! found)
    /* In case of unknown operator */
    elog(ERROR, "Unknown operator Oid %d", operid);

  /* Retrieve the stats object */
  HeapTuple stats_tuple = NULL;
  AttStatsSlot hslot, lslot;

  stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table_oid),
    Int16GetDatum(att_num), BoolGetDatum(false));
  if (! stats_tuple)
    elog(ERROR, "stats for \"%s\" do not exist", get_rel_name(table_oid) ?
      get_rel_name(table_oid) : "NULL");

  int stats_kind = value ?
    STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM :
    STATISTIC_KIND_TIME_BOUNDS_HISTOGRAM;
  if (! get_attstatsslot(&hslot, stats_tuple, stats_kind, InvalidOid,
      ATTSTATSSLOT_VALUES))
    elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot.nvalues < 2)
  {
    free_attstatsslot(&hslot);
    elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
  }

  /* @> and @< also need a histogram of span lengths */
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
  {
    memset(&lslot, 0, sizeof(lslot));

   stats_kind = value ?
    STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM :
    STATISTIC_KIND_TIME_LENGTH_HISTOGRAM;
   if (!(HeapTupleIsValid(stats_tuple) &&
        get_attstatsslot(&lslot, stats_tuple, stats_kind, InvalidOid,
          ATTSTATSSLOT_VALUES)))
    {
      free_attstatsslot(&hslot);
      elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
    }
    /* check that it's a histogram, not just a dummy entry */
    if (lslot.nvalues < 2)
    {
      free_attstatsslot(&lslot);
      free_attstatsslot(&hslot);
      elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
    }
  }

  selec = span_sel_hist1(&hslot, &lslot, s, oper);

  ReleaseSysCache(stats_tuple);
  free_attstatsslot(&hslot);
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
    free_attstatsslot(&lslot);

  PG_RETURN_FLOAT8(selec);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

/*
 * @brief Given two histograms of span bounds, estimate the fraction of values
 * in the first histogram that are less than (or equal to, if `equal` argument
 * is true) a value in the second histogram. The join selectivity estimation
 * for all span operators is expressed using this function. This estimation
 * is described in the following paper:
 *
 * Diogo Repas, Zhicheng Luo, Maxime Schoemans, and Mahmoud Sakr, 2022
 * Selectivity Estimation of Inequality Joins In Databases
 * https://doi.org/10.48550/arXiv.2206.07396
 *
 * The attributes being joined will be treated as random variables that follow
 * a distribution modeled by a Probability Density Function (PDF). Let the two
 * attributes be denoted X, Y. This function finds the probability P(X < Y).
 * Note that the PDFs of the two variables can easily be obtained from their
 * bounds histogram, respectively `hist1` and `hist2`.
 *
 * Let the PDF of X, Y be denoted as f_X, f_Y. The probability P(X < Y) can be
 * formalized as follows:
 * P(X < Y)= integral_-inf^inf( integral_-inf^y ( f_X(x) * f_Y(y) dx dy ) )
 *         = integral_-inf^inf( F_X(y) * f_Y(y) dy )
 * where F_X(y) denote the Cumulative Distribution Function of X at y. Note
 * that F_X is the restriction (non-join) selectivity estimation, which is
 * implemented using the function `span_sel_scalar`.
 *
 * Now given the histograms of the two attributes X, Y, we note the following:
 * - The PDF of Y is a step (that is, constant piece-wise) function where each
 *   piece is defined in a bin of Y's histogram
 * - The CDF of X is linear piece-wise, where each piece is defined in a bin
 *   of X's histogram)
 * This leads to the conclusion that their product (used to calculate the
 * equation above) is also linear piece-wise. A new piece starts whenever
 * either the bin of X or the bin of Y changes. By performing a parallel scan
 * of the two span bound histograms of X and Y, we evaluate one piece of the
 * result between every two consecutive span bounds in the union of the two
 * histograms.
 *
 * Given that the product F_X * f_y is linear in the interval between every two
 * consecutive span bounds, let them be denoted `prev`, `cur`, it can be shown
 * that the above formula can be discretized into the following:
 * P(X < Y) =
 *   0.5 * sum_0^{n+m-1} ( ( F_X(prev) + F_X(cur) ) * ( F_Y(cur) - F_Y(prev) ) )
 * where n, m are the lengths of the two histograms.
 *
 * As such, it is possible to fully compute the join selectivity as a summation
 * of CDFs, iterating over the bounds of the two histograms. This maximizes
 * code reuse, since the CDF is computed using the `span_sel_scalar` function,
 * which is used for restriction (non-join) selectivity estimation.
 */
static double
span_joinsel_scalar(const SpanBound *hist1, int nhist1, const SpanBound *hist2,
  int nhist2, bool equal __attribute__((unused)))
{
  /* A histogram will never have less than 2 values (1 bin) */
  assert(nhist1 > 1);
  assert(nhist2 > 1);

  /* Fast-forward i and j to start of iteration */
  int i, j;
  for (i = 0; span_bound_cmp(&hist1[i], &hist2[0]) < 0; i++);
  for (j = 0; span_bound_cmp(&hist2[j], &hist1[0]) < 0; j++);

  /* Do the estimation on overlapping regions */
  double selec = 0.0,  /* initialisation */
    prev_sel1 = -1.0,  /* to skip the first iteration */
    prev_sel2 = 0.0;   /* make compiler quiet */
  while (i < nhist1 && j < nhist2)
  {
    SpanBound cur_sync;
    if (span_bound_cmp(&hist1[i], &hist2[j]) < 0)
      cur_sync = hist1[i++];
    else if (span_bound_cmp(&hist1[i], &hist2[j]) > 0)
      cur_sync = hist2[j++];
    else
    {
      /* If equal, skip one */
      cur_sync = hist1[i];
      i++;
      j++;
    }
    double cur_sel1 = span_sel_scalar(&cur_sync, hist1, nhist1, false);
    double cur_sel2 = span_sel_scalar(&cur_sync, hist2, nhist2, false);

    /* Skip the first iteration */
    if (prev_sel1 >= 0)
      selec += (prev_sel1 + cur_sel1) * (cur_sel2 - prev_sel2);

    /* Prepare for the next iteration */
    prev_sel1 = cur_sel1;
    prev_sel2 = cur_sel2;
  }
  selec /= 2;

  /* Include remainder of hist2 if any */
  if (j < nhist2)
    selec += 1 - prev_sel2;

  return selec;
}

/**
 * @brief Look up the fraction of values in the first histogram that satisfy an
 * operator with respect to a value in the second histogram
 */
static double
span_joinsel_oper(SpanBound *lower1, SpanBound *upper1, int nhist1,
  SpanBound *lower2, SpanBound *upper2, int nhist2, Datum *length,
  int length_nvalues, meosOper oper)
{
  /* If the spans do not overlap return 0.0 */
  if (span_bound_cmp(&lower1[0], &upper2[nhist2 - 1]) > 0 ||
      span_bound_cmp(&lower2[0], &upper1[nhist1 - 1]) > 0)
    return 0.0;

  double selec = 0.0; /* make compiler quiet */
  if (oper == OVERLAPS_OP)
  {
    selec = 1.0;
    selec -= span_joinsel_scalar(upper1, nhist1, lower2, nhist2, false);
    selec -= span_joinsel_scalar(upper2, nhist2, lower1, nhist1, true);
  }
  else if (oper == CONTAINS_OP)
  {
    for (int i = 0; i < nhist1 - 1; ++i)
      selec += span_sel_contains(&lower1[i], &upper1[i], lower2, nhist2,
        length, length_nvalues);
    selec /= (nhist1 - 1);
  }
  else if (oper == CONTAINED_OP)
  {
    for (int i = 0; i < nhist1 - 1; ++i)
      selec += span_sel_contained(&lower1[i], &upper1[i], lower2, nhist2,
        length, length_nvalues);
    selec /= (nhist1 - 1);
  }
  return selec;
}

/**
 * @brief Calculate span operator selectivity using histograms of span bounds.
 */
static double
span_joinsel_hist1(AttStatsSlot *hslot1, AttStatsSlot *hslot2,
  AttStatsSlot *lslot, meosOper oper)
{
  int nhist1, nhist2;
  SpanBound *lower1, *upper1, *lower2, *upper2;
  int i;
  double selec;

  /*
   * Convert histogram of spans into histograms of its lower and upper
   * bounds for vardata1 and vardata2.
   */
  nhist1 = hslot1->nvalues;
  lower1 = palloc(sizeof(SpanBound) * nhist1);
  upper1 = palloc(sizeof(SpanBound) * nhist1);
  for (i = 0; i < nhist1; i++)
    span_deserialize(DatumGetSpanP(hslot1->values[i]), &lower1[i], &upper1[i]);

  nhist2 = hslot2->nvalues;
  lower2 = palloc(sizeof(SpanBound) * nhist2);
  upper2 = palloc(sizeof(SpanBound) * nhist2);
  for (i = 0; i < nhist2; i++)
    span_deserialize(DatumGetSpanP(hslot2->values[i]), &lower2[i], &upper2[i]);

  /*
   * Calculate the join selectivity of the various operators.
   *
   * The regular B-tree comparison operators (<, <=, >, >=) compare
   * the lower bounds first, and then the upper bounds for values with
   * equal lower bounds. Estimate that by comparing the lower bounds
   * only. This gives a fairly accurate estimate assuming there
   * aren't many rows with a lower bound equal to the constant's
   * lower bound. These operators work only for the span,span
   * combination of parameters. This is because the B-tree stores the
   * values, not their bounding boxes.
   *
   * For the relative position operators (<<#, &<#, #>>, #>&) which are
   * based on bounding boxes the selectivity is estimated by determining
   * the fraction of values less than (or less than or equal to) a given
   * constant in the histograms of span bounds.
   *
   * The other operators (&&, @>, and <@) have specific procedures above.
   */
  if (oper == LT_OP)
    selec = span_joinsel_scalar(lower1, nhist1, lower2, nhist2, false);
  else if (oper == LE_OP)
    selec = span_joinsel_scalar(lower1, nhist1, lower2, nhist2, true);
  else if (oper == GT_OP)
    selec = 1.0 - span_joinsel_scalar(lower1, nhist1, lower2, nhist2, true);
  else if (oper == GE_OP)
    selec = 1.0 - span_joinsel_scalar(lower1, nhist1, lower2, nhist2, false);
  else if (oper == LEFT_OP)
    /* var1 << var2 when upper(var1) < lower(var2)*/
    selec = span_joinsel_scalar(upper1, nhist1, lower2, nhist2, false);
  else if (oper == OVERLEFT_OP)
    /* var1 &< var2 when upper(var1) <= upper(var2) */
    selec = span_joinsel_scalar(upper1, nhist1, upper2, nhist2, true);
  else if (oper == RIGHT_OP)
    /* var1 >> var2 when lower(var1) > upper(var2) */
    selec = 1.0 - span_joinsel_scalar(upper2, nhist2, lower1, nhist1, true);
  else if (oper == OVERRIGHT_OP)
    /* var1 &> var2 when lower(var1) >= lower(var2) */
    selec = 1.0 - span_joinsel_scalar(lower2, nhist2, lower1, nhist1, false);
  else if (oper == BEFORE_OP)
    /* var1 <<# var2 when upper(var1) < lower(var2)*/
    selec = span_joinsel_scalar(upper1, nhist1, lower2, nhist2, false);
  else if (oper == OVERBEFORE_OP)
    /* var1 &<# var2 when upper(var1) <= upper(var2) */
    selec = span_joinsel_scalar(upper1, nhist1, upper2, nhist2, true);
  else if (oper == AFTER_OP)
    /* var1 #>> var2 when lower(var1) > upper(var2) */
    selec = 1.0 - span_joinsel_scalar(upper2, nhist2, lower1, nhist1, true);
  else if (oper == OVERAFTER_OP)
    /* var1 #&> var2 when lower(var1) >= lower(var2) */
    selec = 1.0 - span_joinsel_scalar(lower2, nhist2, lower1, nhist1, false);
  else if (oper == OVERLAPS_OP || oper == CONTAINS_OP || oper == CONTAINED_OP)
    /* specific function for these operators */
    selec = span_joinsel_oper(lower1, upper1, nhist1, lower2, upper2, nhist2,
      lslot->values, lslot->nvalues, oper);
  else if (oper == ADJACENT_OP)
    // TO DO
    selec = span_joinsel_default(InvalidOid);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown span operator");
    selec = -1.0;  /* keep compiler quiet */
  }

  pfree(lower1); pfree(upper1); pfree(lower2); pfree(upper2);

  return selec;
}

/**
 * @brief Calculate span operator selectivity using histograms of span bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
static double
span_joinsel_hist(VariableStatData *vardata1, VariableStatData *vardata2,
  meosOper oper)
{
  /* There is only one lslot, see explanation below */
  AttStatsSlot hslot1, hslot2, lslot;
  Form_pg_statistic stats1 = NULL, stats2 = NULL;
  double selec;
  bool have_hist1 = false, have_hist2 = false;

  memset(&hslot1, 0, sizeof(hslot1));
  memset(&hslot2, 0, sizeof(hslot2));

  /* Try to get histogram of spans of vardata1 and vardata2 */
  if (HeapTupleIsValid(vardata1->statsTuple))
  {
    stats1 = (Form_pg_statistic) GETSTRUCT(vardata1->statsTuple);

    have_hist1 = get_attstatsslot(&hslot1, vardata1->statsTuple,
      STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM, InvalidOid,  // TODO
      ATTSTATSSLOT_VALUES);
    /* Check that it's a histogram, not just a dummy entry */
    if (hslot1.nvalues < 2)
    {
      free_attstatsslot(&hslot1);
      return -1.0;
    }
  }
  if (HeapTupleIsValid(vardata2->statsTuple))
  {
    stats2 = (Form_pg_statistic) GETSTRUCT(vardata2->statsTuple);
    have_hist2 = get_attstatsslot(&hslot2, vardata2->statsTuple,
      STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM, InvalidOid, // TODO
      ATTSTATSSLOT_VALUES);
    /* Check that it's a histogram, not just a dummy entry */
    if (hslot2.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      return -1.0;
    }
  }

  if (! have_hist1 || ! have_hist2)
  {
    /*
     * We do not have histograms for both sides.  Estimate the join
     * selectivity as MIN(1/nd1,1/nd2)*(1-nullfrac1)*(1-nullfrac2). This
     * is plausible if we assume that the join operator is strict and the
     * non-null values are about equally distributed: a given non-null
     * tuple of rel1 will join to either zero or N2*(1-nullfrac2)/nd2 rows
     * of rel2, so total join rows are at most
     * N1*(1-nullfrac1)*N2*(1-nullfrac2)/nd2 giving a join selectivity of
     * not more than (1-nullfrac1)*(1-nullfrac2)/nd2. By the same logic it
     * is not more than (1-nullfrac1)*(1-nullfrac2)/nd1, so the expression
     * with MIN() is an upper bound.  Using the MIN() means we estimate
     * from the point of view of the relation with smaller nd (since the
     * larger nd is determining the MIN).  It is reasonable to assume that
     * most tuples in this rel will have join partners, so the bound is
     * probably reasonably tight and should be taken as-is.
     *
     * XXX Can we be smarter if we have an histogram for just one side? It
     * seems that if we assume equal distribution for the other side, we
     * end up with the same answer anyway.
     */
    bool isdefault1, isdefault2;
    double nd1 = get_variable_numdistinct(vardata1, &isdefault1);
    double nd2 = get_variable_numdistinct(vardata2, &isdefault2);
    double nullfrac1 = stats1 ? stats1->stanullfrac : 0.0;
    double nullfrac2 = stats2 ? stats2->stanullfrac : 0.0;

    selec = (1.0 - nullfrac1) * (1.0 - nullfrac2);
    if (nd1 > nd2)
      selec /= nd1;
    else
      selec /= nd2;
    return selec;
  }

  /* @> and @< also need a histogram of span lengths */
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
  {
    /* We only get histograms for vardata2 since for computing the join
     * selectivity we loop over values of the first histogram assuming
     * they are constant and call the restriction selectivity over the
     * second histogram */
    memset(&lslot, 0, sizeof(lslot));

    if (!(HeapTupleIsValid(vardata2->statsTuple) &&
        get_attstatsslot(&lslot, vardata1->statsTuple,
          STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM, InvalidOid, // TODO
          ATTSTATSSLOT_VALUES)))
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      return -1.0;
    }
    /* check that it's a histogram, not just a dummy entry */
    if (lslot.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      free_attstatsslot(&lslot);
      return -1.0;
    }
  }

  selec = span_joinsel_hist1(&hslot1, &hslot2, &lslot, oper);

  free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
    free_attstatsslot(&lslot);

  // elog(WARNING, "Join selectivity: %lf", selec);

  return selec;
}

/*****************************************************************************/

/**
 * @brief Estimate join selectivity for spans
 */
float8
span_joinsel(PlannerInfo *root, meosOper oper, List *args,
  JoinType jointype __attribute__((unused)), SpecialJoinInfo *sjinfo)
{
  VariableStatData vardata1, vardata2;
  bool join_is_reversed;
  get_join_variables(root, args, sjinfo, &vardata1, &vardata2,
    &join_is_reversed);

  /* Estimate join selectivity */
  float8 selec = span_joinsel_hist(&vardata1, &vardata2, oper);

  ReleaseVariableStats(vardata1);
  ReleaseVariableStats(vardata2);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PGDLLEXPORT Datum Span_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_joinsel);
/**
 * @brief Join selectivity for spans.
 *
 * The selectivity is the ratio of the number of rows we think will be returned
 * divided the maximum number of rows the join could possibly return (the full
 * combinatoric join), that is
 *   joinsel = estimated_nrows / (totalrows1 * totalrows2)
 *
 * This function is inspired from function eqjoinsel in file selfuncs.c
 */
Datum
Span_joinsel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  JoinType jointype = (JoinType) PG_GETARG_INT16(3);
  SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);

  /* Check length of args and punt on > 2 */
  if (list_length(args) != 2)
    PG_RETURN_FLOAT8(span_joinsel_default(operid));

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    PG_RETURN_FLOAT8(span_joinsel_default(operid));

  Node *arg1 = (Node *) linitial(args);
  Node *arg2 = (Node *) lsecond(args);

  /* We only do column joins right now, no functional joins */
  /* TODO: handle t1 <op> expandX(t2) */
  if (!IsA(arg1, Var) || !IsA(arg2, Var))
    PG_RETURN_FLOAT8(span_joinsel_default(operid));
  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  bool value = value_oper_sel(oper, ltype, rtype);
  if (! value)
  {
    bool time = time_oper_sel(oper, ltype, rtype);
    if (! time)
      /* Return default selectivity */
      PG_RETURN_FLOAT8(span_joinsel_default(operid));
  }

  float8 selec = span_joinsel(root, oper, args, jointype, sjinfo);

  PG_RETURN_FLOAT8(selec);
}

PGDLLEXPORT Datum _mobdb_span_joinsel(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(_mobdb_span_joinsel);
/**
 * @brief Utility function to read the calculated selectivity for a given
 * couple of table/column, and operator.
 * @note Used for testing the selectivity code.
 */
Datum
_mobdb_span_joinsel(PG_FUNCTION_ARGS)
{
  Oid table1_oid = PG_GETARG_OID(0);
  text *att1_text = PG_GETARG_TEXT_P(1);
  Oid table2_oid = PG_GETARG_OID(2);
  text *att2_text = PG_GETARG_TEXT_P(3);
  Oid operid = PG_GETARG_OID(4);
  float8 selec = 0.0;

  /* Test input parameters */
  char *table1_name = get_rel_name(table1_oid);
  if (table1_name == NULL)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_TABLE),
      errmsg("Oid %u does not refer to a table", table1_oid)));
  const char *att1_name = text_to_cstring(att1_text);
  AttrNumber att1_num;
  /* Get the attribute number */
  att1_num = get_attnum(table1_oid, att1_name);
  if (! att1_num)
    elog(ERROR, "attribute \"%s\" does not exist", att1_name);
  // /* Get the attribute type */
  // meosType atttype1 = oid_type(get_atttype(table1_oid, att1_num));

  char *table2_name = get_rel_name(table2_oid);
  if (table2_name == NULL)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_TABLE),
      errmsg("Oid %u does not refer to a table", table2_oid)));
  const char *att2_name = text_to_cstring(att2_text);
  AttrNumber att2_num;
  /* Get the attribute number */
  att2_num = get_attnum(table2_oid, att2_name);
  if (! att2_num)
    elog(ERROR, "attribute \"%s\" does not exist", att2_name);
  // /* Get the attribute type */
  // meosType atttype2 = oid_type(get_atttype(table1_oid, att1_num));

  /* Determine whether we can estimate selectivity for the operator */
  meosType ltype, rtype;
  meosOper oper = oid_oper(operid, &ltype, &rtype);
  bool value = value_oper_sel(oper, ltype, rtype);
  if (! value)
  {
    bool time = time_oper_sel(oper, ltype, rtype);
    if (! time)
      /* In case of unknown operator */
      elog(ERROR, "Unknown span operator %d", operid);
  }

  /* Retrieve the stats objects */
  HeapTuple stats1_tuple = NULL, stats2_tuple = NULL;
  AttStatsSlot hslot1, hslot2, lslot;
  int stats_kind = value ?
    STATISTIC_KIND_VALUE_BOUNDS_HISTOGRAM :
    STATISTIC_KIND_TIME_BOUNDS_HISTOGRAM;
  memset(&hslot1, 0, sizeof(hslot1));
  memset(&hslot2, 0, sizeof(hslot2));

  /* First table */
  stats1_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table1_oid),
    Int16GetDatum(att1_num), BoolGetDatum(false));
  if (! stats1_tuple)
    elog(ERROR, "stats for \"%s\" do not exist", table1_name);

  if (! get_attstatsslot(&hslot1, stats1_tuple, stats_kind, InvalidOid,
      ATTSTATSSLOT_VALUES))
    elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot1.nvalues < 2)
  {
    free_attstatsslot(&hslot1);
    elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
  }
  /* Second table */
  stats2_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table2_oid),
    Int16GetDatum(att2_num), BoolGetDatum(false));
  if (! stats2_tuple)
    elog(ERROR, "stats for \"%s\" do not exist", table2_name);

  if (! get_attstatsslot(&hslot2, stats2_tuple, stats_kind, InvalidOid,
      ATTSTATSSLOT_VALUES))
    elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot2.nvalues < 2)
  {
    free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
    elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
  }

  /* @> and @< also need a histogram of span lengths */
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
  {
    /* We only get histograms for the second table since for computing the
     * join selectivity we loop over values of the first histogram assuming
     * they are constant and call the restriction selectivity over the
     * second histogram */
    stats_kind = value ?
      STATISTIC_KIND_VALUE_LENGTH_HISTOGRAM :
      STATISTIC_KIND_TIME_LENGTH_HISTOGRAM;
    memset(&lslot, 0, sizeof(lslot));

    if (! get_attstatsslot(&lslot, stats2_tuple, stats_kind, InvalidOid,
        ATTSTATSSLOT_VALUES))
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
    }
    /* Check that it's a histogram, not just a dummy entry */
    if (lslot.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      free_attstatsslot(&lslot);
      elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
    }
  }

  /* Compute selectivity */
  selec = span_joinsel_hist1(&hslot1, &hslot2, &lslot, oper);

  ReleaseSysCache(stats1_tuple); ReleaseSysCache(stats2_tuple);
  free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
  if (oper == CONTAINS_OP || oper == CONTAINED_OP)
    free_attstatsslot(&lslot);

  PG_RETURN_FLOAT8(selec);
}

/*****************************************************************************/
