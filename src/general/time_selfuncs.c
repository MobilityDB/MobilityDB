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
 * @file time_selfuncs.c
 * @brief Functions for selectivity estimation of time types operators.
 *
 * These functions are based on those of the file `rangetypes_selfuncs.c`.
 * Estimates are based on histograms of lower and upper bounds.
 */

#include "general/time_selfuncs.h"

/* PostgreSQL */
#include <assert.h>
#include <math.h>
#include <port.h>
#include <access/htup_details.h>
#include <utils/builtins.h>
#include "utils/syscache.h"
#include <utils/lsyscache.h>
#include <catalog/pg_statistic.h>
#include <utils/timestamp.h>
#include <math.h>
/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/time_analyze.h"
#include "general/tempcache.h"

/*****************************************************************************/

/**
 * Return a default selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
period_sel_default(CachedOp cachedOp __attribute__((unused)))
{
  // TODO take care of the operator
  return DEFAULT_TEMP_SEL;
}

/**
 * Return a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
float8
period_joinsel_default(CachedOp cachedOp __attribute__((unused)))
{
  // TODO take care of the operator
  return DEFAULT_TEMP_JOINSEL;
}

/**
 * Get the enum associated to the operator from different cases
 */
static bool
time_cachedop(Oid operid, CachedOp *cachedOp)
{
  for (int i = EQ_OP; i <= OVERAFTER_OP; i++)
  {
    if (operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_PERIOD, T_PERIODSET) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPTZ) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPSET) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_PERIOD) ||
        operid == oper_oid((CachedOp) i, T_PERIODSET, T_PERIODSET))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/**
 * Binary search on an array of period bounds. Return greatest index of period
 * bound in array which is less (less or equal) than given period bound. If all
 * period bounds in array are greater or equal (greater) than given period bound,
 * return -1. When "equal" flag is set, the conditions in parenthesis are used.
 *
 * This function is used for scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop interpolation
 * of portion of bounds which are less or equal to given bound.
 */
static int
period_bound_bsearch(const PeriodBound *value, const PeriodBound *hist,
  int hist_nvalues, bool equal)
{
  int lower = -1, upper = hist_nvalues - 1;
  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    int cmp = period_bound_cmp(&hist[middle], value);
    if (cmp < 0 || (equal && cmp == 0))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/**
 * Measure distance between two period bounds
 */
static float8
period_distance(const PeriodBound *bound1, const PeriodBound *bound2)
{
  return period_to_secs(bound2->t, bound1->t);
}

/**
 * Get relative position of value in histogram bin in [0,1] period
 */
static float8
period_position(const PeriodBound *value, const PeriodBound *hist1,
  const PeriodBound *hist2)
{
  /* Calculate relative position using distance function. */
  float8 bin_width = period_distance(hist1, hist2);
  if (bin_width <= 0.0)
    return 0.5;      /* zero width bin */

  float8 position = period_distance(hist1, value) / bin_width;
  /* Relative position must be in [0,1] period */
  position = Max(position, 0.0);
  position = Min(position, 1.0);
  return position;
}

/**
 * Binary search on length histogram. Return greatest index of period length in
 * histogram which is less than (less than or equal) the given length value. If
 * all lengths in the histogram are greater than (greater than or equal) the
 * given length, returns -1.
 *
 * Function copied from file rangetypes_selfuncs.c snce it is not exported
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
 * Get relative position of value in a length histogram bin in [0,1] range.
 *
 * Function copied from PostgreSQL file rangetypes_selfuncs.c since it is
 * not exported.
 */
double
get_len_position(double value, double hist1, double hist2)
{
  if (!isinf(hist1) && !isinf(hist2))
  {
    /*
     * Both bounds are finite. The value should be finite too, because it
     * lies somewhere between the bounds. If it doesn't, just return
     * something.
     */
    if (isinf(value))
      return 0.5;

    return 1.0 - (hist2 - value) / (hist2 - hist1);
  }
  else if (isinf(hist1) && !isinf(hist2))
  {
    /*
     * Lower bin boundary is -infinite, upper is finite. Return 1.0 to
     * indicate the value is infinitely far from the lower bound.
     */
    return 1.0;
  }
  else if (isinf(hist1) && isinf(hist2))
  {
    /* same as above, but in reverse */
    return 0.0;
  }
  else
  {
    /*
     * If both bin boundaries are infinite, they should be equal to each
     * other, and the value should also be infinite and equal to both
     * bounds. (But don't Assert that, to avoid crashing unnecessarily if
     * the caller messes up)
     *
     * Assume the value to lie in the middle of the infinite bounds.
     */
    return 0.5;
  }
}

/**
 * Calculate the average of function P(x), in the interval [length1, length2],
 * where P(x) is the fraction of tuples with length < x (or length <= x if
 * 'equal' is true).
 *
 * Function copied from PostgreSQL file rangetypes_selfuncs.c since it is
 * not exported.
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
 * Estimate the fraction of values less than (or equal to, if 'equal' argument
 * is true) a given const in a histogram of period bounds.
 */
double
period_sel_scalar(const PeriodBound *constbound, const PeriodBound *hist,
  int nhist, bool equal)
{
  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  int index = period_bound_bsearch(constbound, hist, nhist, equal);
  Selectivity selec = (Selectivity) (Max(index, 0)) /
    (Selectivity) (nhist - 1);

  /* Adjust using linear interpolation within the bin */
  if (index >= 0 && index < nhist - 1)
    selec += period_position(constbound, &hist[index], &hist[index + 1]) /
      (Selectivity) (nhist - 1);

  return selec;
}

/**
 * Calculate selectivity of "var && const" operator, i.e., estimate the fraction
 * of periods that overlap the constant lower and upper bounds. This uses
 * the histograms of period lower and upper bounds.
 *
 * Note that A && B <=> NOT (A <<# B OR A #>> B).
 *
 * Since A <<# B and A #>> B are mutually exclusive events we can
 * sum their probabilities to find probability of (A <<# B OR A #>> B).
 *
 * "(period/periodset) @> timestamptz" is equivalent to "period && [elem,elem]".
 * The caller already constructed the singular period from the element
 * constant, so just treat it the same as &&.
 */
static double
period_sel_overlaps(const PeriodBound *const_lower, const PeriodBound *const_upper,
  const PeriodBound *hist_lower, const PeriodBound *hist_upper, int nhist)
{
  /* If the periods do not overlap return 0.0 */
  if (period_bound_cmp(const_lower, &hist_upper[nhist - 1]) > 0 ||
      period_bound_cmp(&hist_lower[0], const_upper) > 0)
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
  // double selec = period_sel_scalar(const_upper, hist_lower, nhist, false);
  // selec += (1.0 - period_sel_scalar(const_lower, hist_upper, nhist, true));
  // selec = 1.0 - selec;

  double selec = period_sel_scalar(const_lower, hist_upper, nhist, false);
  selec += (1.0 - period_sel_scalar(const_upper, hist_lower, nhist, true));
  selec = 1.0 - selec;
  return selec;
}

/**
 * Calculate selectivity of "var <@ const" operator, i.e., estimate the fraction
 * of periods that fall within the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 */
static double
period_sel_contained(PeriodBound *const_lower, PeriodBound *const_upper,
  PeriodBound *hist_lower, int hist_nvalues, Datum *hist_length,
  int hist_length_nvalues)
{
  int i, upper_index;
  float8 prev_dist;
  double bin_width, upper_bin_width, sum_frac;

  /*
   * Begin by finding the bin containing the upper bound, in the lower bound
   * histogram. Any period with a lower bound > constant upper bound can't
   * match, i.e. there are no matches in bins greater than upper_index.
   */
  const_upper->inclusive = !const_upper->inclusive;
  const_upper->lower = true;
  upper_index = period_bound_bsearch(const_upper, hist_lower, hist_nvalues, false);

  /*
   * Calculate upper_bin_width, i.e. the fraction of the (upper_index,
   * upper_index + 1) bin which is greater than constant upper bound
   * using linear interpolation of distance function.
   */
  if (upper_index >= 0 && upper_index < hist_nvalues - 1)
    upper_bin_width = period_position(const_upper, &hist_lower[upper_index],
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
  for (i = upper_index; i >= 0; i--)
  {
    double dist, hist_length_frac;
    bool final_bin = false;

    /*
     * dist -- distance from constant upper bound to the lower bound of
     * the current bin in the lower bound histogram. Or to the lower bound
     * of the constant period, if this is the final bin, containing the
     * constant lower bound.
     */
    if (period_bound_cmp(&hist_lower[i], const_lower) < 0)
    {
      dist = period_distance(const_lower, const_upper);

      /*
       * Subtract from bin_width the portion of this bin that we want to
       * ignore.
       */
      bin_width -= period_position(const_lower, &hist_lower[i],
        &hist_lower[i + 1]);
      if (bin_width < 0.0)
        bin_width = 0.0;
      final_bin = true;
    }
    else
      dist = period_distance(&hist_lower[i], const_upper);

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
 * Calculate selectivity of "var @> const" operator, i.e., estimate the fraction
 * of periods that contain the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 */
static double
period_sel_contains(PeriodBound *const_lower, PeriodBound *const_upper,
  PeriodBound *hist_lower, int hist_nvalues, Datum *hist_length,
  int hist_length_nvalues)
{
  int i, lower_index;
  double bin_width, lower_bin_width, sum_frac;
  float8 prev_dist;

  /* Find the bin containing the constant lower bound */
  lower_index = period_bound_bsearch(const_lower, hist_lower, hist_nvalues, true);

  /*
   * Calculate lower_bin_width, i.e., the fraction of the (lower_index,
   * lower_index + 1) bin which is greater than the constant lower bound
   * using linear interpolation of distance function.
   */
  if (lower_index >= 0 && lower_index < hist_nvalues - 1)
    lower_bin_width = period_position(const_lower, &hist_lower[lower_index],
      &hist_lower[lower_index + 1]);
  else
    lower_bin_width = 0.0;

  /*
   * Loop through all the lower bound bins, smaller than the constant lower
   * bound. In the loop, dist and prev_dist are the distance of the
   * "current" bin's lower and upper bounds from the constant upper bound.
   * We begin from constant lower bound, and walk afterwards, so the first bin's
   * upper bound is the constant lower bound, and its distance to the constant
   * upper bound is the length of the constant period.
   *
   * bin_width represents the width of the current bin. Normally it is 1.0,
   * meaning a full width bin, except for the first bin, which is only
   * counted up to the constant lower bound.
   */
  prev_dist = period_distance(const_lower, const_upper);
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
    dist = period_distance(&hist_lower[i], const_upper);

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
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * @note Used by the selectivity functions and the debugging functions.
 */
static double
period_sel_hist1(AttStatsSlot *hslot, AttStatsSlot *lslot,
  const Period *constval, CachedOp cachedOp)
{
  PeriodBound *hist_lower, *hist_upper;
  PeriodBound const_lower, const_upper;
  double selec;
  int nhist, i;

  /*
   * Convert histogram of periods into histograms of its lower and upper
   * bounds.
   */
  nhist = hslot->nvalues;
  hist_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  hist_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  for (i = 0; i < nhist; i++)
    period_deserialize(DatumGetPeriodP(hslot->values[i]),
      &hist_lower[i], &hist_upper[i]);

  /* Extract the bounds of the constant value. */
  period_deserialize(constval, &const_lower, &const_upper);

  /*
   * Calculate the restriction selectivity of the various operators.
   *
   * The regular B-tree comparison operators (<, <=, >, >=) compare
   * the lower bounds first, and then the upper bounds for values with
   * equal lower bounds. Estimate that by comparing the lower bounds
   * only. This gives a fairly accurate estimate assuming there
   * aren't many rows with a lower bound equal to the constant's
   * lower bound. These operators work only for the period,period
   * combination of parameters. This is because the B-tree stores the
   * values, not their bounding boxes.
   *
   * For the relative position operators (<<#, &<#, #>>, #>&) which are
   * based on bounding boxes the selectivity is estimated by determining
   * the fraction of values less than (or less than or equal to) a given
   * constant in the histograms of period bounds.
   *
   * The other operators (&&, @>, <@, and -|-) have specific procedures
   * above.
   */
  if (cachedOp == LT_OP)
    selec = period_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (cachedOp == LE_OP)
    selec = period_sel_scalar(&const_lower, hist_lower, nhist, true);
  else if (cachedOp == GT_OP)
    selec = 1.0 - period_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (cachedOp == GE_OP)
    selec = 1.0 - period_sel_scalar(&const_lower, hist_lower, nhist, true);
  else if (cachedOp == BEFORE_OP)
    /* var <<# const when upper(var) < lower(const)*/
    selec = period_sel_scalar(&const_lower, hist_upper, nhist, false);
  else if (cachedOp == OVERBEFORE_OP)
    /* var &<# const when upper(var) <= upper(const) */
    selec = period_sel_scalar(&const_upper, hist_upper, nhist, true);
  else if (cachedOp == AFTER_OP)
    /* var #>> const when lower(var) > upper(const) */
    selec = 1.0 - period_sel_scalar(&const_upper, hist_lower, nhist, true);
  else if (cachedOp == OVERAFTER_OP)
    /* var #&> const when lower(var) >= lower(const)*/
    selec = 1.0 - period_sel_scalar(&const_lower, hist_lower, nhist, false);
  else if (cachedOp == OVERLAPS_OP)
    selec = period_sel_overlaps(&const_lower, &const_upper, hist_lower,
      hist_upper, nhist);
  else if (cachedOp == CONTAINS_OP)
    selec = period_sel_contains(&const_lower, &const_upper, hist_lower,
      nhist, lslot->values, lslot->nvalues);
  else if (cachedOp == CONTAINED_OP)
    selec = period_sel_contained(&const_lower, &const_upper, hist_lower,
      nhist, lslot->values, lslot->nvalues);
  else if (cachedOp == ADJACENT_OP)
    // TODO Analyze whether a similar approach as PostgreSQL selectivity
    // estimation for equality can be used. There, they estimate 1/n if
    // the value is not in the MCV
    selec = period_sel_default(InvalidOid);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown period operator");
    selec = -1.0;  /* keep compiler quiet */
  }

  pfree(hist_lower); pfree(hist_upper);

  return selec;
}

/**
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
period_sel_hist(VariableStatData *vardata, const Period *constval,
  CachedOp cachedOp)
{
  AttStatsSlot hslot, lslot;
  double selec;

  memset(&hslot, 0, sizeof(hslot));

  /* Try to get histogram of period bounds of vardata */
  if (!(HeapTupleIsValid(vardata->statsTuple) &&
      get_attstatsslot(&hslot, vardata->statsTuple,
        STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM, InvalidOid,
        ATTSTATSSLOT_VALUES)))
    return -1.0;
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot.nvalues < 2)
  {
    free_attstatsslot(&hslot);
    return -1.0;
  }

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
    memset(&lslot, 0, sizeof(lslot));

    if (!(HeapTupleIsValid(vardata->statsTuple) &&
        get_attstatsslot(&lslot, vardata->statsTuple,
          STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM, InvalidOid,
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

  selec = period_sel_hist1(&hslot, &lslot, constval, cachedOp);

  free_attstatsslot(&hslot);
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
    free_attstatsslot(&lslot);

  // elog(WARNING, "Selectivity: %lf", selec);
  return selec;
}

/*****************************************************************************/

/**
 * Transform the constant into a period
 */
void
time_const_to_period(Node *other, Period *period)
{
  Oid timetypid = ((Const *) other)->consttype;
  CachedType timetype = oid_type(timetypid);
  const Period *p;
  ensure_time_type(timetype);
  if (timetype == T_TIMESTAMPTZ)
  {
    /* The right argument is a TimestampTz constant. We convert it into
     * a singleton period */
    TimestampTz t = DatumGetTimestampTz(((Const *) other)->constvalue);
    period_set(t, t, true, true, period);
  }
  else if (timetype == T_TIMESTAMPSET)
  {
    /* The right argument is a TimestampSet constant. We convert it into
     * a period, which is its bounding box. */
    p = timestampset_bbox_ptr(
      DatumGetTimestampSetP(((Const *) other)->constvalue));
    memcpy(period, p, sizeof(Period));
  }
  else if (timetype == T_PERIOD)
  {
    /* Just copy the value */
    p = DatumGetPeriodP(((Const *) other)->constvalue);
    memcpy(period, p, sizeof(Period));
  }
  else /* timetype == T_PERIODSET */
  {
    /* The right argument is a PeriodSet constant. We convert it into
     * a period, which is its bounding box. */
    p = periodset_bbox_ptr(
      DatumGetPeriodSetP(((Const *) other)->constvalue));
    memcpy(period, p, sizeof(Period));
  }
  return;
}

/**
 * Restriction selectivity for period operators (internal function)
 */
float8
period_sel(PlannerInfo *root, Oid operid, List *args, int varRelid)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  Period period;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
    return period_sel_default(operid);

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    return period_sel_default(operid);
  }

  /*
   * All the period operators are strict, so we can cope with a NULL constant
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
  if (!varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operid = get_commutator(operid);
    if (!operid)
    {
      /* TODO: check whether there might still be a way to estimate.
       * Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      return period_sel_default(operid);
    }
  }

  /*
   * OK, there's a Var and a Const we're dealing with here. If the constant
   * is not of the period type, it should be converted to a period.
   */
  time_const_to_period(other, &period);

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! time_cachedop(operid, &cachedOp))
    /* Unknown operator */
    return period_sel_default(operid);

  /*
   * Estimate using statistics. Note that period_sel need not handle
   * PERIOD_ELEM_CONTAINED_OP.
   */
  float4 null_frac;

  /* First look up the fraction of NULLs periods from pg_statistic. */
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
     * anyway, assuming no NULLs periods. This still allows us
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
  float8 hist_selec = period_sel_hist(&vardata, &period, cachedOp);
  if (hist_selec < 0.0)
    hist_selec = period_sel_default(operid);

  selec = hist_selec;

  /* All period operators are strict */
  selec *= (1.0 - null_frac);

  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PG_FUNCTION_INFO_V1(Period_sel);
/**
 * Restriction selectivity for period operators
 */
PGDLLEXPORT Datum
Period_sel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);
  float8 selec = period_sel(root, operid, args, varRelid);
  PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(_mobdb_period_sel);
/**
 * Utility function to read the calculated selectivity for a given
 * table/column, operator, and search period.
 * Used for debugging the selectivity code.
 */
PGDLLEXPORT Datum
_mobdb_period_sel(PG_FUNCTION_ARGS)
{
  Oid table_oid = PG_GETARG_OID(0);
  text *att_text = PG_GETARG_TEXT_P(1);
  Oid operid = PG_GETARG_OID(2);
  Period *p = PG_GETARG_PERIOD_P(3);
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

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! time_cachedop(operid, &cachedOp))
    /* In case of unknown operator */
    elog(ERROR, "Unknown period operator %d", operid);

  /* Retrieve the stats object */
  HeapTuple stats_tuple = NULL;
  AttStatsSlot hslot, lslot;

  stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table_oid),
    Int16GetDatum(att_num), BoolGetDatum(false));
  if (! stats_tuple)
    elog(ERROR, "stats for \"%s\" do not exist", get_rel_name(table_oid) ?
      get_rel_name(table_oid) : "NULL");

  int stats_kind = STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM;
  if (! get_attstatsslot(&hslot, stats_tuple, stats_kind, InvalidOid,
      ATTSTATSSLOT_VALUES))
    elog(ERROR, "no slot of kind %d in stats tuple", stats_kind);
  /* Check that it's a histogram, not just a dummy entry */
  if (hslot.nvalues < 2)
  {
    free_attstatsslot(&hslot);
    elog(ERROR, "Invalid slot of kind %d in stats tuple", stats_kind);
  }

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
    memset(&lslot, 0, sizeof(lslot));

   stats_kind = STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM;
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

  selec = period_sel_hist1(&hslot, &lslot, p, cachedOp);

  ReleaseSysCache(stats_tuple);
  free_attstatsslot(&hslot);
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
    free_attstatsslot(&lslot);

  PG_RETURN_FLOAT8(selec);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

/**
 * Given two histograms of period bounds, estimate the fraction of values in
 * the first histogram that are less than (or equal to, if 'equal' argument
 * is true) a value in the second histogram. The join selectivity estimation
 * for all period operators is expressed using this function.
 *
 * The general idea is to iteratively decompose (var op var) into a summation
 * of (var op const) using the period bounds present in first histogram as
 * const. Then, (var op const) is calculated using the function
 * period_sel_scalar, which estimates the restriction selectivity.
 *
 * Consider two variables Var1 and Var2 whose distributions are given by
 * hist1 and hist2, respectively. To estimate:
 *
 * P(Var1 < Var2)
 *
 * we need to compute for each bin in the first histogram
 *
 * P(Var1 < Var2 | Var2 is in bin) * P(Var2 is in bin)
 *
 * The second probability is the difference between the selectivity of the
 * upper and lower bounds of the bin, which can be directly computed by
 * calling period_sel_scalar.
 *
 * The first probability, however, cannot be directly computed because we do
 * not have a concrete value for Var2. Instead, we can under and over estimate
 * it by respectively setting the value of Var2 to the lower and upper bound
 * of the bin, then compute the average.
 *
 * P(Var1 < lower bound of bin) <=
 * P(Var1 < Var2 | Var2 is in bin) <=
 * P(Var1 < upper bound of bin)
 *
 * Therefore, we need to add the average selectivity of every bin, given by
 *    (val1 + val2) / 2 + (val2 + val3) / 2 +  ... + (val_n-1 + val_n) / 2
 * which is equal to
 *    val1 / 2 + val2 + val3 + val4 + ... + val_n-1 + val_n / 2
 * The first and last terms above are computed out of the loop. The rest is
 * computed in the loop.
 */
double
period_joinsel_scalar(const PeriodBound *hist1, int nhist1,
  const PeriodBound *hist2, int nhist2, bool equal)
{
  Selectivity selec = (Selectivity)
    (period_sel_scalar(&hist1[0], hist2, nhist2, equal) / 2);
  for (int i = 1; i < nhist1 - 1; ++i)
    selec += (Selectivity)
      period_sel_scalar(&hist1[i], hist2, nhist2, equal);
  selec += (Selectivity)
    (period_sel_scalar(&hist1[nhist1 - 1], hist2, nhist2, equal) / 2);
  return selec / (nhist1 - 1);
}

/**
 * Look up the fraction of values in the first histogram that overlap a value
 * in the second histogram
 */
double
period_joinsel_overlaps(PeriodBound *lower1, PeriodBound *upper1,
  int nhist1, PeriodBound *lower2, PeriodBound *upper2, int nhist2)
{
  /* If the periods do not overlap return 0.0 */
  if (period_bound_cmp(&lower1[0], &upper2[nhist2 - 1]) > 0 ||
      period_bound_cmp(&lower2[0], &upper1[nhist1 - 1]) > 0)
    return 0.0;

  double selec = period_joinsel_scalar(lower1, nhist1, upper2, nhist2, false);
  selec += (1.0 - period_joinsel_scalar(upper1, nhist1, lower2, nhist2, true));
  selec = 1.0 - selec;

  return selec;
}

/**
 * Look up the fraction of values in the first histogram that contain a value
 * in the second histogram
 */
double
period_joinsel_contains(PeriodBound *lower1, PeriodBound *upper1,
  int nhist1, PeriodBound *lower2, PeriodBound *upper2, int nhist2,
  Datum *length, int length_nvalues)
{
  /* If the periods do not overlap return 0.0 */
  if (period_bound_cmp(&lower1[0], &upper2[nhist2 - 1]) > 0 ||
      period_bound_cmp(&lower2[0], &upper1[nhist1 - 1]) > 0)
    return 0.0;

  Selectivity selec = 0.0;
  for (int i = 0; i < nhist1 - 1; ++i)
    selec += (Selectivity) period_sel_contains(&lower1[i], &upper1[i],
      lower2, nhist2, length, length_nvalues);
  return selec / (nhist1 - 1);
}

/**
 * Look up the fraction of values in the first histogram that is
 * contained in a value in the second histogram
 */
double
period_joinsel_contained(PeriodBound *lower1, PeriodBound *upper1,
  int nhist1, PeriodBound *lower2, PeriodBound *upper2, int nhist2,
  Datum *length, int length_nvalues)
{
  /* If the periods do not overlap return 0.0 */
  if (period_bound_cmp(&lower1[0], &upper2[nhist2 - 1]) > 0 ||
      period_bound_cmp(&lower2[0], &upper1[nhist1 - 1]) > 0)
    return 0.0;

  Selectivity selec = 0.0;
  for (int i = 0; i < nhist1 - 1; ++i)
    selec += (Selectivity) period_sel_contained(&lower1[i], &upper1[i],
      lower2, nhist2, length, length_nvalues);
  return selec / (nhist1 - 1);
}

/**
 * Calculate period operator selectivity using histograms of period bounds.
 */
static double
period_joinsel_hist1(AttStatsSlot *hslot1, AttStatsSlot *hslot2,
  AttStatsSlot *lslot, CachedOp cachedOp)
{
  int nhist1, nhist2;
  PeriodBound *lower1, *upper1, *lower2, *upper2;
  int i;
  double selec;

  /*
   * Convert histogram of periods into histograms of its lower and upper
   * bounds for vardata1 and vardata2.
   */
  nhist1 = hslot1->nvalues;
  lower1 = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
  upper1 = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
  for (i = 0; i < nhist1; i++)
  {
    period_deserialize(DatumGetPeriodP(hslot1->values[i]),
      &lower1[i], &upper1[i]);
  }
  nhist2 = hslot2->nvalues;
  lower2 = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist2);
  upper2 = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist2);
  for (i = 0; i < nhist2; i++)
  {
    period_deserialize(DatumGetPeriodP(hslot2->values[i]),
      &lower2[i], &upper2[i]);
  }

  /*
   * Calculate the join selectivity of the various operators.
   *
   * The regular B-tree comparison operators (<, <=, >, >=) compare
   * the lower bounds first, and then the upper bounds for values with
   * equal lower bounds. Estimate that by comparing the lower bounds
   * only. This gives a fairly accurate estimate assuming there
   * aren't many rows with a lower bound equal to the constant's
   * lower bound. These operators work only for the period,period
   * combination of parameters. This is because the B-tree stores the
   * values, not their bounding boxes.
   *
   * For the relative position operators (<<#, &<#, #>>, #>&) which are
   * based on bounding boxes the selectivity is estimated by determining
   * the fraction of values less than (or less than or equal to) a given
   * constant in the histograms of period bounds.
   *
   * The other operators (&&, @>, <@, and -|-) have specific procedures
   * above.
   */
  if (cachedOp == LT_OP)
    selec = period_joinsel_scalar(lower1, nhist1, lower2, nhist2, false);
  else if (cachedOp == LE_OP)
    selec = period_joinsel_scalar(lower1, nhist1, lower2, nhist2, true);
  else if (cachedOp == GT_OP)
    selec = 1.0 - period_joinsel_scalar(lower1, nhist1, lower2, nhist2, true);
  else if (cachedOp == GE_OP)
    selec = 1.0 - period_joinsel_scalar(lower1, nhist1, lower2, nhist2, false);
  else if (cachedOp == BEFORE_OP)
    /* var1 <<# var2 when upper(var1) < lower(var2)*/
    selec = period_joinsel_scalar(upper1, nhist1, lower2, nhist2, false);
  else if (cachedOp == OVERBEFORE_OP)
    /* var1 &<# var2 when upper(var1) <= upper(var2) */
    selec = period_joinsel_scalar(upper1, nhist1, upper2, nhist2, true);
  else if (cachedOp == AFTER_OP)
    /* var1 #>> var2 when lower(var1) > upper(var2) */
    selec = 1.0 - period_joinsel_scalar(upper2, nhist2, lower1, nhist1, true);
  else if (cachedOp == OVERAFTER_OP)
    /* var1 #&> var2 when lower(var1) >= lower(var2) */
    selec = 1.0 - period_joinsel_scalar(lower2, nhist2, lower1, nhist1, false);
  else if (cachedOp == OVERLAPS_OP)
    selec = period_joinsel_overlaps(lower1, upper1, nhist1, lower2, upper2, nhist2);
  else if (cachedOp == CONTAINS_OP)
    selec = period_joinsel_contains(lower1, upper1, nhist1, lower2, upper2,
      nhist2, lslot->values, lslot->nvalues);
  else if (cachedOp == CONTAINED_OP)
    selec = period_joinsel_contained(lower1, upper1, nhist1, lower2, upper2,
      nhist2, lslot->values, lslot->nvalues);
  else if (cachedOp == ADJACENT_OP)
    // TO DO
    selec = period_joinsel_default(InvalidOid);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown period operator");
    selec = -1.0;  /* keep compiler quiet */
  }

  pfree(lower1); pfree(upper1); pfree(lower2); pfree(upper2);

  return selec;
}

/**
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
period_joinsel_hist(VariableStatData *vardata1, VariableStatData *vardata2,
  CachedOp cachedOp)
{
  /* There is only one lslot, see explanation below */
  AttStatsSlot hslot1, hslot2, lslot;
  Form_pg_statistic stats1 = NULL, stats2 = NULL;
  double selec;
  bool have_hist1 = false, have_hist2 = false;

  memset(&hslot1, 0, sizeof(hslot1));
  memset(&hslot2, 0, sizeof(hslot2));

  /* Try to get histogram of periods of vardata1 and vardata2 */
  if (HeapTupleIsValid(vardata1->statsTuple))
  {
    stats1 = (Form_pg_statistic) GETSTRUCT(vardata1->statsTuple);
    have_hist1 = get_attstatsslot(&hslot1, vardata1->statsTuple,
      STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM, InvalidOid, ATTSTATSSLOT_VALUES);
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
      STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM, InvalidOid, ATTSTATSSLOT_VALUES);
    /* Check that it's a histogram, not just a dummy entry */
    if (hslot2.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      return -1.0;
    }
  }

  if (!have_hist1 || !have_hist2)
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

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
    /* We only get histograms for vardata2 since for computing the join
     * selectivity we loop over values of the first histogram assuming
     * they are constant and call the restriction selectivity over the
     * second histogram */
    memset(&lslot, 0, sizeof(lslot));

    if (!(HeapTupleIsValid(vardata2->statsTuple) &&
        get_attstatsslot(&lslot, vardata1->statsTuple,
          STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM, InvalidOid,
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

  selec = period_joinsel_hist1(&hslot1, &hslot2, &lslot, cachedOp);

  free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
    free_attstatsslot(&lslot);

  // elog(WARNING, "Join selectivity: %lf", selec);
  return selec;
}

/*****************************************************************************/

/**
 * Join selectivity for periods (internal function)
 */
float8
period_joinsel(PlannerInfo *root, CachedOp cachedOp, List *args,
  JoinType jointype __attribute__((unused)), SpecialJoinInfo *sjinfo)
{
  VariableStatData vardata1, vardata2;
  bool join_is_reversed;
  get_join_variables(root, args, sjinfo, &vardata1, &vardata2,
    &join_is_reversed);

  /* Estimate join selectivity */
  float8 selec = period_joinsel_hist(&vardata1, &vardata2, cachedOp);

  ReleaseVariableStats(vardata1);
  ReleaseVariableStats(vardata2);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PG_FUNCTION_INFO_V1(Period_joinsel);
/**
 * Join selectivity for periods.
 *
 * The selectivity is the ratio of the number of
 * rows we think will be returned divided the maximum number of rows the join
 * could possibly return (the full combinatoric join), that is
 *   joinsel = estimated_nrows / (totalrows1 * totalrows2)
 *
 * This function is inspired from function eqjoinsel in file selfuncs.c
 */
PGDLLEXPORT Datum
Period_joinsel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid operid = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  JoinType jointype = (JoinType) PG_GETARG_INT16(3);
  SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);

  /* Check length of args and punt on > 2 */
  if (list_length(args) != 2)
    PG_RETURN_FLOAT8(period_joinsel_default(operid));

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    PG_RETURN_FLOAT8(period_joinsel_default(operid));

  Node *arg1 = (Node *) linitial(args);
  Node *arg2 = (Node *) lsecond(args);

  /* We only do column joins right now, no functional joins */
  /* TODO: handle t1 <op> expandX(t2) */
  if (!IsA(arg1, Var) || !IsA(arg2, Var))
    PG_RETURN_FLOAT8(period_joinsel_default(operid));

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! time_cachedop(operid, &cachedOp))
    /* Unknown operator */
    PG_RETURN_FLOAT8(period_joinsel_default(operid));

  float8 selec = period_joinsel(root, cachedOp, args, jointype, sjinfo);

  PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(_mobdb_period_joinsel);
/**
 * Utility function to read the calculated selectivity for a given
 * couple of table/column, and operator.
 * Used for debugging the selectivity code.
 */
PGDLLEXPORT Datum
_mobdb_period_joinsel(PG_FUNCTION_ARGS)
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
  /* We know the name? Look up the num */
  if (att1_text)
  {
    /* Get the attribute number */
    att1_num = get_attnum(table1_oid, att1_name);
    if (! att1_num)
      elog(ERROR, "attribute \"%s\" does not exist", att1_name);
  }
  else
    elog(ERROR, "attribute name is null");

  char *table2_name = get_rel_name(table2_oid);
  if (table2_name == NULL)
    ereport(ERROR, (errcode(ERRCODE_UNDEFINED_TABLE),
      errmsg("Oid %u does not refer to a table", table2_oid)));
  const char *att2_name = text_to_cstring(att2_text);
  AttrNumber att2_num;
  /* We know the name? Look up the num */
  if (att2_text)
  {
    /* Get the attribute number */
    att2_num = get_attnum(table2_oid, att2_name);
    if (! att2_num)
      elog(ERROR, "attribute \"%s\" does not exist", att2_name);
  }
  else
    elog(ERROR, "attribute name is null");

  /* Get enumeration value associated to the operator */
  CachedOp cachedOp;
  if (! time_cachedop(operid, &cachedOp))
    /* In case of unknown operator */
    elog(ERROR, "Unknown period operator %d", operid);

  /* Retrieve the stats objects */
  HeapTuple stats1_tuple = NULL, stats2_tuple = NULL;
  AttStatsSlot hslot1, hslot2, lslot;
  int stats_kind = STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM;
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

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
    /* We only get histograms for the second table since for computing the
     * join selectivity we loop over values of the first histogram assuming
     * they are constant and call the restriction selectivity over the
     * second histogram */
    stats_kind = STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM;
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
  selec = period_joinsel_hist1(&hslot1, &hslot2, &lslot, cachedOp);

  ReleaseSysCache(stats1_tuple); ReleaseSysCache(stats2_tuple);
  free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
    free_attstatsslot(&lslot);

  PG_RETURN_FLOAT8(selec);
}

/*****************************************************************************/
