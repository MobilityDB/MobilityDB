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
 * @file time_selfuncs.c
 * Functions for selectivity estimation of time types operators
 *
 * These functions are based on those of the file `rangetypes_selfuncs.c`.
 * Estimates are based on histograms of lower and upper bounds.
 */

#include "time_selfuncs.h"

#include <assert.h>
#include <math.h>
#include <port.h>
#include <access/htup_details.h>
#include <utils/lsyscache.h>
#include <catalog/pg_statistic.h>
#include <utils/timestamp.h>
#include <math.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "time_analyze.h"
#include "tempcache.h"

/*****************************************************************************/

/*
 * Returns a default selectivity estimate for given operator, when we don't
 * have statistics or cannot use them for some reason.
 */
static double
default_period_selectivity(Oid operator)
{
  return 0.01;
}

/* Get the enum associated to the operator from different cases */
static bool
get_time_cachedop(Oid operator, CachedOp *cachedOp)
{
  for (int i = EQ_OP; i <= OVERAFTER_OP; i++)
  {
    if (operator == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPTZ, T_PERIODSET) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_TIMESTAMPSET, T_PERIODSET) ||
      operator == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPTZ) ||
      operator == oper_oid((CachedOp) i, T_PERIOD, T_TIMESTAMPSET) ||
      operator == oper_oid((CachedOp) i, T_PERIOD, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_PERIOD, T_PERIODSET) ||
      operator == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPTZ) ||
      operator == oper_oid((CachedOp) i, T_PERIODSET, T_TIMESTAMPSET) ||
      operator == oper_oid((CachedOp) i, T_PERIODSET, T_PERIOD) ||
      operator == oper_oid((CachedOp) i, T_PERIODSET, T_PERIODSET))
      {
        *cachedOp = (CachedOp) i;
        return true;
      }
  }
  return false;
}

static double
calc_periodsel(VariableStatData *vardata, const Period *constval, Oid operator)
{
  double    hist_selec;
  double    selec;
  float4    null_frac;

  /*
   * First look up the fraction of NULLs periods from pg_statistic.
   */
  if (HeapTupleIsValid(vardata->statsTuple))
  {
    Form_pg_statistic stats;

    stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);
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
  bool found = get_time_cachedop(operator, &cachedOp);
  /* In the case of unknown operator */
  if (!found)
    return default_period_selectivity(operator);

  /*
   * Calculate selectivity using bound histograms. If that fails for
   * some reason, e.g no histogram in pg_statistic, use the default
   * constant estimate. This is still somewhat better than just
   * returning the default estimate, because this still takes into
   * account the fraction of NULL tuples, if we had statistics for them.
   */
  hist_selec = calc_period_hist_selectivity(vardata, constval, cachedOp);
  if (hist_selec < 0.0)
    hist_selec = default_period_selectivity(operator);

  selec = hist_selec;

  /* all period operators are strict */
  selec *= (1.0 - null_frac);

  /* result should be in period, but make sure... */
  CLAMP_PROBABILITY(selec);

  return selec;
}

/*
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
calc_period_hist_selectivity(VariableStatData *vardata, const Period *constval,
  CachedOp cachedOp)
{
  AttStatsSlot hslot, lslot;
  PeriodBound *hist_lower, *hist_upper;
  PeriodBound  const_lower, const_upper;
  double    hist_selec;
  int      nhist, i;

  if (!(HeapTupleIsValid(vardata->statsTuple) &&
      get_attstatsslot(&hslot, vardata->statsTuple,
               STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM,
               InvalidOid, ATTSTATSSLOT_VALUES)))
    return -1.0;
  /*
   * Convert histogram of periods into histograms of its lower and upper
   * bounds.
   */
  nhist = hslot.nvalues;
  hist_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  hist_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
  for (i = 0; i < nhist; i++)
    period_deserialize(DatumGetPeriod(hslot.values[i]),
               &hist_lower[i], &hist_upper[i]);

  /* @> and @< also need a histogram of period lengths */
  if (cachedOp == CONTAINS_OP || cachedOp == CONTAINED_OP)
  {
    if (!(HeapTupleIsValid(vardata->statsTuple) &&
        get_attstatsslot(&lslot, vardata->statsTuple,
                 STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM,
                 InvalidOid, ATTSTATSSLOT_VALUES)))
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
    hist_selec = calc_period_hist_selectivity_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == LE_OP)
    hist_selec = calc_period_hist_selectivity_scalar(&const_lower,
      hist_lower, nhist, true);
  else if (cachedOp == GT_OP)
    hist_selec = 1 - calc_period_hist_selectivity_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == GE_OP)
    hist_selec = 1 - calc_period_hist_selectivity_scalar(&const_lower,
      hist_lower, nhist, true);
  else if (cachedOp == BEFORE_OP)
    /* var <<# const when upper(var) < lower(const)*/
    hist_selec = calc_period_hist_selectivity_scalar(&const_lower,
      hist_upper, nhist, false);
  else if (cachedOp == OVERBEFORE_OP)
    /* var &<# const when upper(var) <= upper(const) */
    hist_selec = calc_period_hist_selectivity_scalar(&const_upper,
      hist_upper, nhist, true);
  else if (cachedOp == AFTER_OP)
    /* var #>> const when lower(var) > upper(const) */
    hist_selec = 1 - calc_period_hist_selectivity_scalar(&const_upper,
      hist_lower, nhist, true);
  else if (cachedOp == OVERAFTER_OP)
    /* var #&> const when lower(var) >= lower(const)*/
    hist_selec = 1 - calc_period_hist_selectivity_scalar(&const_lower,
      hist_lower, nhist, false);
  else if (cachedOp == OVERLAPS_OP)
  {
    /*
     * A && B <=> NOT (A <<# B OR A #>> B).
     *
     * Since A <<# B and A #>> B are mutually exclusive events we can
     * sum their probabilities to find probability of (A <<# B OR
     * A #>> B).
     *
     * "(period/periodset) @> timestamptz" is equivalent to
     * "period && [elem,elem]". The
     * caller already constructed the singular period from the element
     * constant, so just treat it the same as &&.
     */
    hist_selec = calc_period_hist_selectivity_scalar(&const_lower,
      hist_upper, nhist, false);
    hist_selec += (1.0 - calc_period_hist_selectivity_scalar(&const_upper,
      hist_lower, nhist, true));
    hist_selec = 1.0 - hist_selec;
  }
  else if (cachedOp == CONTAINS_OP)
    hist_selec = calc_period_hist_selectivity_contains(&const_lower,
      &const_upper, hist_lower, nhist, lslot.values, lslot.nvalues);
  else if (cachedOp == CONTAINED_OP)
    hist_selec = calc_period_hist_selectivity_contained(&const_lower,
      &const_upper, hist_lower, nhist, lslot.values, lslot.nvalues);
  else if (cachedOp == ADJACENT_OP)
    hist_selec = calc_period_hist_selectivity_adjacent(&const_lower,
      &const_upper, hist_lower, hist_upper,nhist);
  else
  {
    elog(ERROR, "Unable to compute selectivity for unknown period operator");
    hist_selec = -1.0;  /* keep compiler quiet */
  }

  free_attstatsslot(&lslot);
  free_attstatsslot(&hslot);

  return hist_selec;
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
period_rbound_bsearch(PeriodBound *value, PeriodBound *hist,
            int hist_length, bool equal)
{
  int    lower = -1,
      upper = hist_length - 1;

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
get_period_distance(PeriodBound *bound1, PeriodBound *bound2)
{
  return period_to_secs(bound2->t, bound1->t);
}

/*
 * Get relative position of value in histogram bin in [0,1] period.
 */
static float8
get_period_position(PeriodBound *value, PeriodBound *hist1,
          PeriodBound *hist2)
{
  float8    position;
  float8    bin_width;

  /* Calculate relative position using distance function. */
  bin_width = get_period_distance(hist2, hist1);
  if (bin_width <= 0.0)
    return 0.5;      /* zero width bin */

  position = get_period_distance(value, hist1) / bin_width;

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
calc_period_hist_selectivity_scalar(PeriodBound *constbound,
                  PeriodBound *hist, int hist_nvalues, bool equal)
{
  Selectivity selec;
  int      index;

  /*
   * Find the histogram bin the given constant falls into. Estimate
   * selectivity as the number of preceding whole bins.
   */
  index = period_rbound_bsearch(constbound, hist, hist_nvalues, equal);
  selec = (Selectivity) (Max(index, 0)) / (Selectivity) (hist_nvalues - 1);

  /* Adjust using linear interpolation within the bin */
  if (index >= 0 && index < hist_nvalues - 1)
    selec += get_period_position(constbound, &hist[index],
                   &hist[index + 1]) / (Selectivity) (hist_nvalues - 1);

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
  int    lower = -1,
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
  double    frac;
  double    A,
        B,
        PA,
        PB;
  double    pos;
  int      i;
  double    area;

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

/*
 * Calculate selectivity of "var <@ const" operator, ie. estimate the fraction
 * of periods that fall within the constant lower and upper bounds. This uses
 * the histograms of period lower bounds and period lengths, on the assumption
 * that the period lengths are independent of the lower bounds.
 *
 * The caller has already checked that constant lower and upper bounds are
 * finite.
 */
double
calc_period_hist_selectivity_contained(PeriodBound *lower, PeriodBound *upper,
                     PeriodBound *hist_lower, int hist_nvalues, Datum *length_hist_values,
                     int length_hist_nvalues)
{
  int      i,
        upper_index;
  float8    prev_dist;
  double    bin_width;
  double    upper_bin_width;
  double    sum_frac;

  /*
   * Begin by finding the bin containing the upper bound, in the lower bound
   * histogram. Any period with a lower bound > constant upper bound can't
   * match, ie. there are no matches in bins greater than upper_index.
   */
  upper->inclusive = !upper->inclusive;
  upper->lower = true;
  upper_index = period_rbound_bsearch(upper, hist_lower, hist_nvalues,
                    false);

  /*
   * Calculate upper_bin_width, ie. the fraction of the (upper_index,
   * upper_index + 1) bin which is greater than upper bound of query period
   * using linear interpolation of distance function.
   */
  if (upper_index >= 0 && upper_index < hist_nvalues - 1)
    upper_bin_width = get_period_position(upper,
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
    double    dist;
    double    length_hist_frac;
    bool    final_bin = false;

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
double
calc_period_hist_selectivity_contains(PeriodBound *lower, PeriodBound *upper,
  PeriodBound *hist_lower, int hist_nvalues, Datum *length_hist_values,
  int length_hist_nvalues)
{
  int      i,
        lower_index;
  double    bin_width,
        lower_bin_width,
        sum_frac;
  float8    prev_dist;

  /* Find the bin containing the lower bound of query period. */
  lower_index = period_rbound_bsearch(lower, hist_lower, hist_nvalues, true);

  /*
   * Calculate lower_bin_width, ie. the fraction of the of (lower_index,
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
    float8    dist;
    double    length_hist_frac;

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
* Look up the fraction of values between lower(lowerbin) and lower, plus
* the fraction between upper and upper(upperbin).
*/
double calc_period_hist_selectivity_adjacent(PeriodBound *lower, PeriodBound *upper,
                       PeriodBound *hist_lower, PeriodBound *hist_upper, int hist_nvalues)
{
  Selectivity selec1=0, selec2=0;
  int  index1, index2;

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
 * periodsel -- restriction selectivity for period operators
 */
PG_FUNCTION_INFO_V1(periodsel);

PGDLLEXPORT Datum
periodsel(PG_FUNCTION_ARGS)
{
  PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
  Oid      operator = PG_GETARG_OID(1);
  List     *args = (List *) PG_GETARG_POINTER(2);
  int      varRelid = PG_GETARG_INT32(3);
  VariableStatData vardata;
  Node     *other;
  bool    varonleft;
  Selectivity selec;
  const Period *period = NULL;

  /*
   * If expression is not (variable op something) or (something op
   * variable), then punt and return a default estimate.
   */
  if (!get_restriction_variable(root, args, varRelid, &vardata, &other,
                  &varonleft))
    PG_RETURN_FLOAT8(default_period_selectivity(operator));

  /*
   * Can't do anything useful if the something is not a constant, either.
   */
  if (!IsA(other, Const))
  {
    ReleaseVariableStats(vardata);
    PG_RETURN_FLOAT8(default_period_selectivity(operator));
  }

  /*
   * All the period operators are strict, so we can cope with a NULL constant
   * right away.
   */
  if (((Const *) other)->constisnull)
  {
    ReleaseVariableStats(vardata);
    PG_RETURN_FLOAT8(0.0);
  }

  /*
   * If var is on the right, commute the operator, so that we can assume the
   * var is on the left in what follows.
   */
  if (!varonleft)
  {
    /* we have other Op var, commute to make var Op other */
    operator = get_commutator(operator);
    if (!operator)
    {
      /* TODO: check whether there might still be a way to estimate.
      * Use default selectivity (should we raise an error instead?) */
      ReleaseVariableStats(vardata);
      PG_RETURN_FLOAT8(default_period_selectivity(operator));
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
        DatumGetTimestampSet(((Const *) other)->constvalue));
  }
  else if (timetypid == type_oid(T_PERIOD))
  {
    /* just copy the value */
    period = DatumGetPeriod(((Const *) other)->constvalue);
  }
  else if (timetypid == type_oid(T_PERIODSET))
  {
    /* the right argument is a constant PERIODSET. We convert it into
     * a period, which is its bounding box.
     */
    period =  periodset_bbox_ptr(
        DatumGetPeriodSet(((Const *) other)->constvalue));
  }

  /*
   * If we got a valid constant on one side of the operator, proceed to
   * estimate using statistics. Otherwise punt and return a default constant
   * estimate. Note that calc_periodsel need not handle
   * PERIOD_ELEM_CONTAINED_OP.
   */
  if (period)
    selec = calc_periodsel(&vardata, period, operator);
  else
    selec = default_period_selectivity(operator);

  ReleaseVariableStats(vardata);

  CLAMP_PROBABILITY(selec);

  PG_RETURN_FLOAT8((float8) selec);
}

/*****************************************************************************/
