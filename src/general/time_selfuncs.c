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
 * Functions for selectivity estimation of time types operators
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
#include <utils/lsyscache.h>
#include <catalog/pg_statistic.h>
#include <utils/timestamp.h>
#include <math.h>

/* MobilityDB */
#include "general/timestampset.h"
#include "general/period.h"
#include "general/periodset.h"
#include "general/timeops.h"
#include "general/time_analyze.h"
#include "general/tempcache.h"

/*****************************************************************************/

/*
 * Returns a default selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
static double
period_sel_default(Oid oper)
{
  return 0.01;
}

/*
 * Returns a default join selectivity estimate for given operator, when we
 * don't have statistics or cannot use them for some reason.
 */
static double
period_joinsel_default(Oid oper)
{
  return 0.001;
}

/* Get the enum associated to the operator from different cases */
static bool
get_time_cachedop(Oid oper, CachedOp *cachedOp)
{
  for (int i = EQ_OP; i <= OVERAFTER_OP; i++)
  {
    if (oper == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIOD) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIODSET) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIOD) ||
        oper == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIODSET) ||
        oper == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPTZ) ||
        oper == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPSET) ||
        oper == oper_oid((CachedOp) i, T_PERIOD, T_PERIOD) ||
        oper == oper_oid((CachedOp) i, T_PERIOD, T_PERIODSET) ||
        oper == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPTZ) ||
        oper == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPSET) ||
        oper == oper_oid((CachedOp) i, T_PERIODSET, T_PERIOD) ||
        oper == oper_oid((CachedOp) i, T_PERIODSET, T_PERIODSET))
        {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

/*
 * Binary search on an array of period bounds. Returns greatest index of period
 * bound in array which is less(less or equal) than given period bound. If all
 * period bounds in array are greater or equal(greater) than given period bound,
 * return -1. When "equal" flag is set conditions in brackets are used.
 *
 * This function is used in scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop
 * interpolation of portion of bounds which are less or equal to given bound.
 */
static int
period_rbound_bsearch(const PeriodBound *value, const PeriodBound *hist,
  int hist_length, bool equal)
{
  int lower = -1, upper = hist_length - 1;

  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    int cmp = period_cmp_bounds(&hist[middle], value);

    if (cmp < 0 || (equal && cmp == 0))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/*
 * Measure distance between two period bounds.
 */
static float8
get_period_distance(const PeriodBound *bound1, const PeriodBound *bound2)
{
  return period_to_secs(bound2->t, bound1->t);
}

/*
 * Get relative position of value in histogram bin in [0,1] period.
 */
static float8
get_period_position(const PeriodBound *value, const PeriodBound *hist1,
  const PeriodBound *hist2)
{
  float8 position, bin_width;

  /* Calculate relative position using distance function. */
  bin_width = get_period_distance(hist1, hist2);
  if (bin_width <= 0.0)
    return 0.5;      /* zero width bin */

  position = get_period_distance(hist1, value) / bin_width;

  /* Relative position must be in [0,1] period */
  position = Max(position, 0.0);
  position = Min(position, 1.0);
  return position;
}

/*
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a given const in a histogram of period bounds.
 */
double
period_sel_scalar(const PeriodBound *constbound, const PeriodBound *hist,
  int hist_nvalues, bool equal)
{
  Selectivity selec;
  int index;

  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  index = period_rbound_bsearch(constbound, hist, hist_nvalues, equal);
  selec = (Selectivity) (Max(index, 0)) / (Selectivity) (hist_nvalues - 1);

  /* Adjust using linear interpolation within the bin */
  if (index >= 0 && index < hist_nvalues - 1)
    selec += get_period_position(constbound, &hist[index], &hist[index + 1]) /
      (Selectivity) (hist_nvalues - 1);

  return selec;
}

/*
 * Binary search on length histogram. Returns greatest index of period length in
 * histogram which is less than (less than or equal) the given length value. If
 * all lengths in the histogram are greater than (greater than or equal) the
 * given length, returns -1.
 *
 * Function copied from file rangetypes_selfuncs.c snce it is not exported
 */
int
length_hist_bsearch(Datum *length_hist_values, int length_hist_nvalues,
  double value, bool equal)
{
  int lower = -1,
      upper = length_hist_nvalues - 1;
  while (lower < upper)
  {
    int middle = (lower + upper + 1) / 2;
    double middleval = DatumGetFloat8(length_hist_values[middle]);
    if (middleval < value || (equal && middleval <= value))
      lower = middle;
    else
      upper = middle - 1;
  }
  return lower;
}

/*
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

/*
 * Calculate the average of function P(x), in the interval [length1, length2],
 * where P(x) is the fraction of tuples with length < x (or length <= x if
 * 'equal' is true).
 *
 * Function copied from PostgreSQL file rangetypes_selfuncs.c since it is
 * not exported.
 */
double
calc_length_hist_frac(Datum *length_hist_values, int length_hist_nvalues,
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
   *      B
   *    1    /
   * -------  | P(x)dx
   *  B - A  /
   *      A
   *
   * The geometrical interpretation of the integral is the area under the
   * graph of P(x). P(x) is defined by the length histogram. We calculate
   * the area in a piecewise fashion, iterating through the length histogram
   * bins. Each bin is a trapezoid:
   *
   *     P(x2)
   *      /|
   *     / |
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
  i = length_hist_bsearch(length_hist_values, length_hist_nvalues, length1, equal);
  if (i >= length_hist_nvalues - 1)
    return 1.0;

  if (i < 0)
  {
    i = 0;
    pos = 0.0;
  }
  else
  {
    /* interpolate length1's position in the bin */
    pos = get_len_position(length1,
                 DatumGetFloat8(length_hist_values[i]),
                 DatumGetFloat8(length_hist_values[i + 1]));
  }
  PB = (((double) i) + pos) / (double) (length_hist_nvalues - 1);
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
  for (; i < length_hist_nvalues - 1; i++)
  {
    double    bin_upper = DatumGetFloat8(length_hist_values[i + 1]);

    /* check if we've reached the last bin */
    if (!(bin_upper < length2 || (equal && bin_upper <= length2)))
      break;

    /* the upper bound of previous bin is the lower bound of this bin */
    A = B;
    PA = PB;

    B = bin_upper;
    PB = (double) i / (double) (length_hist_nvalues - 1);

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

  B = length2;        /* last bin ends at the query upper bound */
  if (i >= length_hist_nvalues - 1)
    pos = 0.0;
  else
  {
    if (DatumGetFloat8(length_hist_values[i]) == DatumGetFloat8(length_hist_values[i + 1]))
      pos = 0.0;
    else
      pos = get_len_position(length2, DatumGetFloat8(length_hist_values[i]),
        DatumGetFloat8(length_hist_values[i + 1]));
  }
  PB = (((double) i) + pos) / (double) (length_hist_nvalues - 1);

  if (PA > 0 || PB > 0)
    area += 0.5 * (PB + PA) * (B - A);

  /*
   * Ok, we have calculated the area, ie. the integral. Divide by width to
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

/*
* Look up the fraction of values between lower(lowerbin) and lower, plus
* the fraction between upper and upper(upperbin).
*/
double
period_sel_adjacent(PeriodBound *lower, PeriodBound *upper,
  PeriodBound *hist_lower, PeriodBound *hist_upper, int hist_nvalues)
{
  Selectivity selec1 = 0, selec2 = 0;
  int index1, index2;

  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  index1 = period_rbound_bsearch(lower, hist_upper, hist_nvalues, true);
  index2 = period_rbound_bsearch(upper, hist_lower, hist_nvalues, true);
  if (index1 != 0 && index1 < hist_nvalues - 1)
  {
    /*
     * lower falls inside a bin of hist_upper. The number of elements in
     * the whole bin is 1/hist_nvalues. We further refine it by substracting
     * the fraction of the bin that occures after the lower.
     */
    selec1 = (Selectivity) 1 / (Selectivity) (hist_nvalues - 1);
    selec1 *= get_period_position(lower, &hist_upper[index1],
      &hist_upper[index1 + 1]) / (Selectivity) (hist_nvalues - 1);
  }
  if (index2 != 0 && index2 < hist_nvalues - 1)
  {
    selec2 = (Selectivity) 1 / (Selectivity) (hist_nvalues - 1);
    selec2 *= 1- get_period_position(upper, &hist_lower[index2],
      &hist_lower[index2 + 1]) / (Selectivity) (hist_nvalues - 1);
  }

  return selec1 + selec2;
}

/*
 * Calculate selectivity of "var <@ const" operator, ie. estimate the fraction
 * of periods that fall within the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 *
 * The caller has already checked that constant lower and upper bounds are
 * finite.
 */
static double
period_sel_contained(PeriodBound *lower, PeriodBound *upper,
  PeriodBound *hist_lower, int hist_nvalues, Datum *length_hist_values,
  int length_hist_nvalues)
{
  int i, upper_index;
  float8 prev_dist;
  double bin_width, upper_bin_width, sum_frac;

  /*
   * Begin by finding the bin containing the upper bound, in the lower bound
   * histogram. Any period with a lower bound > constant upper bound can't
   * match, ie. there are no matches in bins greater than upper_index.
   */
  upper->inclusive = !upper->inclusive;
  upper->lower = true;
  upper_index = period_rbound_bsearch(upper, hist_lower, hist_nvalues, false);

  /*
   * Calculate upper_bin_width, ie. the fraction of the (upper_index,
   * upper_index + 1) bin which is greater than upper bound of query period
   * using linear interpolation of distance function.
   */
  if (upper_index >= 0 && upper_index < hist_nvalues - 1)
    upper_bin_width = get_period_position(upper, &hist_lower[upper_index],
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
    double dist, length_hist_frac;
    bool final_bin = false;

    /*
     * dist -- distance from upper bound of query period to lower bound of
     * the current bin in the lower bound histogram. Or to the lower bound
     * of the constant period, if this is the final bin, containing the
     * constant lower bound.
     */
    if (period_cmp_bounds(&hist_lower[i], lower) < 0)
    {
      dist = get_period_distance(lower, upper);

      /*
       * Subtract from bin_width the portion of this bin that we want to
       * ignore.
       */
      bin_width -= get_period_position(lower, &hist_lower[i],
        &hist_lower[i + 1]);
      if (bin_width < 0.0)
        bin_width = 0.0;
      final_bin = true;
    }
    else
      dist = get_period_distance(&hist_lower[i], upper);

    /*
     * Estimate the fraction of tuples in this bin that are narrow enough
     * to not exceed the distance to the upper bound of the query period.
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

/*
 * Calculate selectivity of "var @> const" operator, ie. estimate the fraction
 * of periods that contain the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 *
 * Note, this is "var @> const", ie. estimate the fraction of periods that
 * contain the constant lower and upper bounds.
 */
static double
period_sel_contains(PeriodBound *lower, PeriodBound *upper,
  PeriodBound *hist_lower, int hist_nvalues, Datum *length_hist_values,
  int length_hist_nvalues)
{
  int i, lower_index;
  double bin_width, lower_bin_width, sum_frac;
  float8 prev_dist;

  /* Find the bin containing the lower bound of query period. */
  lower_index = period_rbound_bsearch(lower, hist_lower, hist_nvalues, true);

  /*
   * Calculate lower_bin_width, i.e. the fraction of the (lower_index,
   * lower_index + 1) bin which is greater than lower bound of query period
   * using linear interpolation of distance function.
   */
  if (lower_index >= 0 && lower_index < hist_nvalues - 1)
    lower_bin_width = get_period_position(lower, &hist_lower[lower_index],
      &hist_lower[lower_index + 1]);
  else
    lower_bin_width = 0.0;

  /*
   * Loop through all the lower bound bins, smaller than the query lower
   * bound. In the loop, dist and prev_dist are the distance of the
   * "current" bin's lower and upper bounds from the constant upper bound.
   * We begin from query lower bound, and walk afterwards, so the first bin's
   * upper bound is the query lower bound, and its distance to the query
   * upper bound is the length of the query period.
   *
   * bin_width represents the width of the current bin. Normally it is 1.0,
   * meaning a full width bin, except for the first bin, which is only
   * counted up to the constant lower bound.
   */
  prev_dist = get_period_distance(lower, upper);
  sum_frac = 0.0;
  bin_width = lower_bin_width;
  for (i = lower_index; i >= 0; i--)
  {
    float8 dist;
    double length_hist_frac;

    /*
     * dist -- distance from upper bound of query period to current value
     * of lower bound histogram or lower bound of query period (if we've
     * reach it).
     */
    dist = get_period_distance(&hist_lower[i], upper);

    /*
     * Get average fraction of length histogram which covers intervals
     * longer than (or equal to) distance to upper bound of query period.
     */
    length_hist_frac = 1.0 - calc_length_hist_frac(length_hist_values,
      length_hist_nvalues, prev_dist, dist, false);

    sum_frac += length_hist_frac * bin_width / (double) (hist_nvalues - 1);

    bin_width = 1.0;
    prev_dist = dist;
  }

  return sum_frac;
}

/*
 * Calculate selectivity of "var && const" operator, ie. estimate the fraction
 * of periods that overlap the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 *
 * Note that A && B <=> NOT (A <<# B OR A #>> B).
 *
 * Since A <<# B and A #>> B are mutually exclusive events we can
 * sum their probabilities to find probability of (A <<# B OR
 * A #>> B).
 *
 * "(period/periodset) @> timestamptz" is equivalent to
 * "period && [elem,elem]". The caller already constructed the singular period
 * from the element constant, so just treat it the same as &&.
 */
static double
period_sel_overlaps(PeriodBound *lower1, PeriodBound *upper1,
  PeriodBound *lower2, PeriodBound *upper2, int nhist)
{
  /* If the periods do not overlap return 0.0 */
  if (period_cmp_bounds(lower1, upper2) > 0 ||
    period_cmp_bounds(lower2, upper1) > 0)
    return 0.0;

  float8 hist_selec = period_sel_scalar(lower1, upper2, nhist, false);
  hist_selec += (1.0 - period_sel_scalar(upper1, lower2, nhist, true));
  hist_selec = 1.0 - hist_selec;
  return hist_selec;
}

/*
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
period_sel_hist(VariableStatData *vardata, const Period *constval,
  CachedOp cachedOp)
{
  AttStatsSlot hslot, lslot;
  PeriodBound *hist_lower, *hist_upper;
  PeriodBound const_lower, const_upper;
  double hist_selec;
  int nhist, i;

  memset(&hslot, 0, sizeof(hslot));
  memset(&lslot, 0, sizeof(lslot));

  /* Try to get histogram of periods of vardata1 */
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

  /*
   * Convert histogram of periods into histograms of its lower and upper
   * bounds.
   */
  nhist = hslot.nvalues;
  hist_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  hist_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  for (i = 0; i < nhist; i++)
    period_deserialize(DatumGetPeriodP(hslot.values[i]),
      &hist_lower[i], &hist_upper[i]);

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
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

  /* Extract the bounds of the constant value. */
  period_deserialize(constval, &const_lower, &const_upper);

  /*
   * Calculate selectivity comparing the lower or upper bound of the
   * constant with the histogram of lower or upper bounds.
   */
  if (cachedOp == LT_OP)
    /*
     * The regular b-tree comparison operators (<, <=, >, >=) compare
     * the lower bounds first, and the upper bounds for values with
     * equal lower bounds. Estimate that by comparing the lower bounds
     * only. This gives a fairly accurate estimate assuming there
     * aren't many rows with a lower bound equal to the constant's
     * lower bound. These operators work only for the period,period
     * combination of parameters. This is because the b-tree stores the
     * values, not their BBoxes.
     */
    hist_selec = period_sel_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == LE_OP)
    hist_selec = period_sel_scalar(&const_lower,
      hist_lower, nhist, true);
  else if (cachedOp == GT_OP)
    hist_selec = 1 - period_sel_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == GE_OP)
    hist_selec = 1 - period_sel_scalar(&const_lower,
      hist_lower, nhist, true);
  else if (cachedOp == BEFORE_OP)
    /* var <<# const when upper(var) < lower(const)*/
    hist_selec = period_sel_scalar(&const_lower,
      hist_upper, nhist, false);
  else if (cachedOp == OVERBEFORE_OP)
    /* var &<# const when upper(var) <= upper(const) */
    hist_selec = period_sel_scalar(&const_upper,
      hist_upper, nhist, true);
  else if (cachedOp == AFTER_OP)
    /* var #>> const when lower(var) > upper(const) */
    hist_selec = 1 - period_sel_scalar(&const_upper,
      hist_lower, nhist, true);
  else if (cachedOp == OVERAFTER_OP)
    /* var #&> const when lower(var) >= lower(const)*/
    hist_selec = 1 - period_sel_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == OVERLAPS_OP)
    hist_selec = period_sel_overlaps(&const_lower, &const_upper,
      hist_lower, hist_upper, nhist);
  else if (cachedOp == CONTAINS_OP)
    hist_selec = period_sel_contains(&const_lower,
      &const_upper, hist_lower, nhist, lslot.values, lslot.nvalues);
  else if (cachedOp == CONTAINED_OP)
    hist_selec = period_sel_contained(&const_lower,
      &const_upper, hist_lower, nhist, lslot.values, lslot.nvalues);
  else if (cachedOp == ADJACENT_OP)
    hist_selec = period_sel_adjacent(&const_lower,
      &const_upper, hist_lower, hist_upper,nhist);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown period operator");
    hist_selec = -1.0;  /* keep compiler quiet */
  }

  pfree(hist_lower);
  pfree(hist_upper);
  free_attstatsslot(&lslot);
  free_attstatsslot(&hslot);

  return hist_selec;
}

/*****************************************************************************/

float8
period_sel_internal(PlannerInfo *root, Oid oper, List *args, int varRelid)
{
  VariableStatData vardata;
  Node *other;
  bool varonleft;
  Selectivity selec;
  const Period *period = NULL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid, &vardata, &other,
      &varonleft))
    return period_sel_default(oper);

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    return period_sel_default(oper);
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
    oper = get_commutator(oper);
    if (!oper)
    {
      /* TODO: check whether there might still be a way to estimate.
      * Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      return period_sel_default(oper);
    }
  }

  /*
   * OK, there's a Var and a Const we're dealing with here.  We need the
   * Const to be of same period type as the column, else we can't do anything
   * useful. (Such cases will likely fail at runtime, but here we'd rather
   * just return a default estimate.)
   *
   * If the operator is "period @> element", the constant should be of the
   * element type of the period column. Convert it to a period that includes
   * only that single point, so that we don't need special handling for that
   * in what follows.
   */

  Oid timetypid = ((Const *) other)->consttype;
  ensure_time_type_oid(timetypid);
  if (timetypid == TIMESTAMPTZOID)
  {
    /* the right argument is a constant TIMESTAMPTZ. We convert it into
     * a singleton period
     */
    TimestampTz t = DatumGetTimestampTz(((Const *) other)->constvalue);
    period = period_make(t, t, true, true);
  }
  else if (timetypid == type_oid(T_TIMESTAMPSET))
  {
    /* the right argument is a constant TIMESTAMPSET. We convert it into
     * a period, which is its bounding box.
     */
    period =  timestampset_bbox_ptr(
      DatumGetTimestampSetP(((Const *) other)->constvalue));
  }
  else if (timetypid == type_oid(T_PERIOD))
  {
    /* just copy the value */
    period = DatumGetPeriodP(((Const *) other)->constvalue);
  }
  else if (timetypid == type_oid(T_PERIODSET))
  {
    /* the right argument is a constant PERIODSET. We convert it into
     * a period, which is its bounding box.
     */
    period =  periodset_bbox_ptr(
      DatumGetPeriodSetP(((Const *) other)->constvalue));
  }

  /*
   * If we got a valid constant on one side of the operator, proceed to
   * estimate using statistics. Otherwise punt and return a default constant
   * estimate. Note that period_sel_internal need not handle
   * PERIOD_ELEM_CONTAINED_OP.
   */
  if (!period)
    selec = period_sel_default(oper);
  else
  {
    double hist_selec;
    float4 null_frac;

    /*
     * First look up the fraction of NULLs periods from pg_statistic.
     */
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
     * Get enumeration value associated to the operator
     */
    CachedOp cachedOp;
    bool found = get_time_cachedop(oper, &cachedOp);
    /* In the case of unknown operator */
    if (!found)
    {
      ReleaseVariableStats(vardata);
      return period_sel_default(oper);
    }

    /*
     * Calculate selectivity using bound histograms. If that fails for
     * some reason, e.g no histogram in pg_statistic, use the default
     * constant estimate. This is still somewhat better than just
     * returning the default estimate, because this still takes into
     * account the fraction of NULL tuples, if we had statistics for them.
     */
    hist_selec = period_sel_hist(&vardata, period, cachedOp);
    if (hist_selec < 0.0)
      hist_selec = period_sel_default(oper);

    selec = hist_selec;

    /* all period operators are strict */
    selec *= (1.0 - null_frac);
  }
  ReleaseVariableStats(vardata);
  CLAMP_PROBABILITY(selec);
  return selec;
}

PG_FUNCTION_INFO_V1(period_sel);
/*
 * Restriction selectivity for period operators
 */
PGDLLEXPORT Datum
period_sel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid oper = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  int varRelid = PG_GETARG_INT32(3);
  float8 selec = period_sel_internal(root, oper, args, varRelid);
  PG_RETURN_FLOAT8((float8) selec);
}

/*****************************************************************************
 * Join selectivity
 *****************************************************************************/

/*
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a histogram of period bounds in another histogram of period bounds.
 *
 * The intuition is that the first histogram can be considered as a
 * distribution of const bound, and for each const we can call the function
 * period_sel_scalar to compute its selectivity. By integrate them
 * with respect to the distribution, we obtain the said fraction.
 */
static double
period_joinsel_hist1(const PeriodBound *hist1, int hist1_nvalues,
  const PeriodBound *hist2, int hist2_nvalues, bool equal)
{
  int i;
  int idx1;
  int idx2;
  double cur_selec;
  const PeriodBound *chosed_bound;
  const PeriodBound *old_bound;
  int cur_bin_idx;
  double cur_bin_area;
  double cur_bin_height;
  double trapezoid_base1;
  double trapezoid_base2;
  double trapezoid_height;
  double *area_values;
  Selectivity selec;

  idx1 = idx2 = 0;
  cur_bin_idx = -1;
  area_values = (double *) palloc(sizeof(double) * (hist1_nvalues - 1));
  memset(area_values, 0, sizeof(double) * (hist1_nvalues - 1));
  selec = 0;

  /* Loop until finishing all period bounds in hist1.
   *
   * For each bin in hist1, we calculate its area_value through dividing
   * the area under the selectivity curve within the bin by the length of
   * the bin. The area_value represents the join selectivity contributed
   * by this bin, regarding hist2. The average of all area_values
   * represents the join selectivity of hist1 regarding hist2, as hist1 is
   * an Equi-Depth histogram and all bins take up the same weight.
   */
  while (idx1 < hist1_nvalues)
  {
    /* Loop until finishing traversing all period bounds in hist1 */
    if (idx2 >= hist2_nvalues)
    {
      /* period bounds in hist2 have been finished */
      if (cur_bin_idx < 0)
        break;
      else
      {
        area_values[cur_bin_idx] = (cur_bin_height == 0) ? 0 :
          cur_bin_area / cur_bin_height;
        cur_bin_area = 0;
        cur_bin_height = 0;
        cur_bin_idx++;
        idx1++;
      }
    }
    else
    {
      /* Proceed with the smaller one between hist1[idx1] and hist2[idx2] */
      if (period_cmp_bounds(&hist1[idx1], &hist2[idx2]) <= 0)
      {
        chosed_bound = &hist1[idx1];
        cur_selec = 1 - period_sel_scalar(chosed_bound, hist2, hist2_nvalues, equal);
        if (cur_bin_idx < 0)
        {
          /* first bin */
          cur_bin_idx = 0;
        }
        else
        {
          /* Finish a bin and move to a new one */
          trapezoid_base2 = cur_selec;
          trapezoid_height = get_period_distance(chosed_bound, old_bound);
          cur_bin_area += (trapezoid_base1 + trapezoid_base2) * trapezoid_height / 2;
          cur_bin_height += trapezoid_height;
          area_values[cur_bin_idx] = (cur_bin_height == 0) ? 0 : cur_bin_area / cur_bin_height;
          cur_bin_idx++;
        }
        cur_bin_area = 0;
        cur_bin_height = 0;
        trapezoid_base1 = cur_selec;
        trapezoid_base2 = trapezoid_height = 0;
        old_bound = chosed_bound;
        idx1++;
      }
      else
      {
        chosed_bound = &hist2[idx2];
        cur_selec = 1 - idx2 / (hist2_nvalues - 1.0);
        if (cur_bin_idx < 0)
        {
          idx2++;
          continue;
        }
        trapezoid_base2 = cur_selec;
        trapezoid_height = get_period_distance(chosed_bound, old_bound);
        cur_bin_area += (trapezoid_base1 + trapezoid_base2) * trapezoid_height / 2;
        cur_bin_height += trapezoid_height;
        trapezoid_base1 = cur_selec;
        trapezoid_base2 = trapezoid_height = 0;
        old_bound = chosed_bound;
        idx2++;
      }
    }
  }

  for (i = 0; i < (hist1_nvalues - 1); i++)
    selec += area_values[i];
  pfree(area_values);
  selec /= (hist1_nvalues - 1);
  return selec;
}

static double
period_joinsel_hist(VariableStatData *vardata1, VariableStatData *vardata2,
  CachedOp cachedOp)
{
  AttStatsSlot hslot1, hslot2, lslot1, lslot2;
  Form_pg_statistic stats1 = NULL, stats2 = NULL;
  int nhist1, nhist2;
  PeriodBound *hist1_lower, *hist1_upper, *hist2_lower, *hist2_upper;
  int i;
  double nd1, nd2, selec;
  bool have_hist1 = false, have_hist2 = false, isdefault1, isdefault2;

  memset(&hslot1, 0, sizeof(hslot1));
  memset(&hslot2, 0, sizeof(hslot2));
  memset(&lslot1, 0, sizeof(lslot1));
  memset(&lslot2, 0, sizeof(lslot2));

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

  nd1 = get_variable_numdistinct(vardata1, &isdefault1);
  nd2 = get_variable_numdistinct(vardata2, &isdefault2);

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
    /* Get histograms for vardata1 */
    if (!(HeapTupleIsValid(vardata1->statsTuple) &&
        get_attstatsslot(&lslot1, vardata1->statsTuple,
          STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM, InvalidOid,
          ATTSTATSSLOT_VALUES)))
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      return -1.0;
    }
    /* check that it's a histogram, not just a dummy entry */
    if (lslot1.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      free_attstatsslot(&lslot1);
      return -1.0;
    }
    /* Get histograms for vardata2 */
    if (!(HeapTupleIsValid(vardata2->statsTuple) &&
        get_attstatsslot(&lslot2, vardata1->statsTuple,
          STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM, InvalidOid,
          ATTSTATSSLOT_VALUES)))
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      free_attstatsslot(&lslot1);
      return -1.0;
    }
    /* check that it's a histogram, not just a dummy entry */
    if (lslot2.nvalues < 2)
    {
      free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
      free_attstatsslot(&lslot1); free_attstatsslot(&lslot2);
      return -1.0;
    }
  }

  /*
   * Convert histogram of periods into histograms of its lower and upper
   * bounds for vardata1 and vardata2.
   */
  nhist1 = hslot1.nvalues;
  hist1_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
  hist1_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
  for (i = 0; i < nhist1; i++)
  {
    period_deserialize(DatumGetPeriodP(hslot1.values[i]),
      &hist1_lower[i], &hist1_upper[i]);
  }
  nhist2 = hslot2.nvalues;
  hist2_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist2);
  hist2_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist2);
  for (i = 0; i < nhist2; i++)
  {
    period_deserialize(DatumGetPeriodP(hslot2.values[i]),
      &hist2_lower[i], &hist2_upper[i]);
  }

  /*
   * Calculate selectivity comparing the lower or upper bound of the
   * the histograms.
   */
  if (cachedOp == LT_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == LE_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == GT_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == GE_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == BEFORE_OP)
    selec =
      period_joinsel_hist1(hist1_upper, nhist1, hist2_lower, nhist2, false);
  else if (cachedOp == OVERBEFORE_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == AFTER_OP)
    selec =
      period_joinsel_hist1(hist2_upper, nhist2, hist1_lower, nhist1, false);
  else if (cachedOp == OVERAFTER_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == OVERLAPS_OP)
    selec = 1
      - period_joinsel_hist1(hist1_upper, nhist1, hist2_lower, nhist2, false)
      - period_joinsel_hist1(hist2_upper, nhist2, hist1_lower, nhist1, false);
  else if (cachedOp == CONTAINS_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == CONTAINED_OP)
    selec = period_joinsel_default(InvalidOid);
  else if (cachedOp == ADJACENT_OP)
    selec = period_joinsel_default(InvalidOid);
  else
  {
    elog(ERROR, "Unable to compute join selectivity for unknown period operator");
    selec = -1.0;  /* keep compiler quiet */
  }

  pfree(hist1_lower); pfree(hist1_upper);
  pfree(hist2_lower); pfree(hist2_upper);
  free_attstatsslot(&hslot1); free_attstatsslot(&hslot2);
  free_attstatsslot(&lslot1); free_attstatsslot(&lslot2);

  return selec;
}

/*****************************************************************************/

float8
period_joinsel_internal(PlannerInfo *root, Oid oper, List *args,
  JoinType jointype, SpecialJoinInfo *sjinfo)
{
  VariableStatData vardata1, vardata2;
  bool join_is_reversed;
  float8 selec;

  /* Check length of args and punt on > 2 */
  if (list_length(args) != 2)
    return DEFAULT_TEMP_JOINSEL;

  /* Only respond to an inner join/unknown context join */
  if (jointype != JOIN_INNER)
    return DEFAULT_TEMP_JOINSEL;

  get_join_variables(root, args, sjinfo, &vardata1, &vardata2,
    &join_is_reversed);

  /*
   * Get enumeration value associated to the operator
   */
  CachedOp cachedOp;
  bool found = get_time_cachedop(oper, &cachedOp);
  /* In the case of unknown operator */
  if (!found)
  {
    ReleaseVariableStats(vardata1);
    ReleaseVariableStats(vardata2);
    return period_joinsel_default(oper);
  }

  /* Estimate join selectivity */
  selec = period_joinsel_hist(&vardata1, &vardata2, cachedOp);

  ReleaseVariableStats(vardata1);
  ReleaseVariableStats(vardata2);
  CLAMP_PROBABILITY(selec);
  return (float8) selec;
}

PG_FUNCTION_INFO_V1(period_joinsel);
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
Datum period_joinsel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid oper = PG_GETARG_OID(1);
  List *args = (List *) PG_GETARG_POINTER(2);
  JoinType jointype = (JoinType) PG_GETARG_INT16(3);
  SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);
  float8 selec = period_joinsel_internal(root, oper, args, jointype, sjinfo);
  PG_RETURN_FLOAT8((float8) selec);
}

/*****************************************************************************/
