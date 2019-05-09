/*****************************************************************************
 *
 * TimeSelfuncs.c
 *	  Functions for selectivity estimation of time types operators
 *
 * Estimates are based on histograms of lower and upper bounds.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
#include "TemporalTypes.h"

/*
 * Returns a default selectivity estimate for given operator, when we don't
 * have statistics or cannot use them for some reason.
 */
double
default_period_selectivity(Oid operator)
{
	return 0.01;
}

/*
 * Calculate period operator selectivity using histograms of period bounds.
 *
 * This estimate is for the portion of values that are not NULL.
 */
double
calc_period_hist_selectivity(VariableStatData *vardata,
	Period *constval, Oid operator, StatisticsStrategy strategy)
{
	AttStatsSlot hslot;
	AttStatsSlot lslot;
	int			nhist;
	PeriodBound *hist_lower;
	PeriodBound *hist_upper;
	int			i;
	PeriodBound	const_lower;
	PeriodBound	const_upper;
	double		hist_selec;
	int kind_type = STATISTIC_KIND_BOUNDS_HISTOGRAM;
	/* Try to get histogram of periods */
	if (vardata->atttypmod == TEMPORALINST)
		kind_type = STATISTIC_KIND_HISTOGRAM;

	if (!(HeapTupleIsValid(vardata->statsTuple) &&
		  get_attstatsslot_internal(&hslot, vardata->statsTuple,
									kind_type, InvalidOid,
									ATTSTATSSLOT_VALUES, strategy)))
		return -1.0;
	/*
	 * Convert histogram of periods into histograms of its lower and upper
	 * bounds.
	 */
	nhist = hslot.nvalues;
	hist_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
	hist_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist);
	for (i = 0; i < nhist; i++)
	{
		period_deserialize(DatumGetPeriod(hslot.values[i]),
						   &hist_lower[i], &hist_upper[i]);
	}

	/* @> and @< also need a histogram of period lengths */
	if (operator == oper_oid(CONTAINS_OP, T_PERIOD, T_PERIOD) ||
		operator == oper_oid(CONTAINED_OP, T_PERIOD, T_PERIOD))
	{
		if (!(HeapTupleIsValid(vardata->statsTuple) &&
			  get_attstatsslot_internal(&lslot, vardata->statsTuple,
										STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM,
										InvalidOid,
										ATTSTATSSLOT_VALUES,strategy)))
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
	if (operator == oper_oid(LT_OP, T_PERIOD, T_PERIOD) )
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
		hist_selec =
				calc_period_hist_selectivity_scalar(&const_lower,
													hist_lower, nhist, false);
	else if (operator == oper_oid(LE_OP, T_PERIOD, T_PERIOD))
		hist_selec =
				calc_period_hist_selectivity_scalar(&const_lower,
													hist_lower, nhist, true);
	else if (operator == oper_oid(GT_OP, T_PERIOD, T_PERIOD) )
		hist_selec =
				1 - calc_period_hist_selectivity_scalar(&const_lower,
														hist_lower, nhist, false);
	else if (operator == oper_oid(GE_OP, T_PERIOD, T_PERIOD) )
		hist_selec =
				1 - calc_period_hist_selectivity_scalar(&const_lower,
														hist_lower, nhist, true);
	else if (
			operator == oper_oid(BEFORE_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(BEFORE_OP, T_PERIOD, T_PERIODSET) ||
			operator == oper_oid(BEFORE_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(BEFORE_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPTZ, T_PERIOD) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(BEFORE_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(BEFORE_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(BEFORE_OP, T_PERIODSET, T_PERIOD) ||
			operator == oper_oid(BEFORE_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(BEFORE_OP, T_PERIODSET, T_TIMESTAMPSET) )
		/* var <<# const when upper(var) < lower(const)*/
		hist_selec =
				calc_period_hist_selectivity_scalar(&const_lower,
													hist_upper, nhist, false);
	else if (
			operator == oper_oid(AFTER_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(AFTER_OP, T_PERIOD, T_PERIODSET) ||
			operator == oper_oid(AFTER_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(AFTER_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPTZ, T_PERIOD) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(AFTER_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(AFTER_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(AFTER_OP, T_PERIODSET, T_PERIOD) ||
			operator == oper_oid(AFTER_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(AFTER_OP, T_PERIODSET, T_TIMESTAMPSET) )
		/* var #>> const when lower(var) > upper(const) */
		hist_selec =
				1 - calc_period_hist_selectivity_scalar(&const_upper,
														hist_lower, nhist, true);
	else if (
			operator == oper_oid(OVERAFTER_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIOD, T_PERIODSET) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPTZ, T_PERIOD) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(OVERAFTER_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIODSET, T_PERIOD) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(OVERAFTER_OP, T_PERIODSET, T_TIMESTAMPSET) )
		/* var #&> const when lower(var) >= lower(const)*/
		hist_selec =
				1 - calc_period_hist_selectivity_scalar(&const_lower,
														hist_lower, nhist, false);
	else if (
			operator == oper_oid(OVERBEFORE_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIOD, T_PERIODSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPTZ, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPTZ, T_PERIOD) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIODSET, T_PERIOD) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(OVERBEFORE_OP, T_PERIODSET, T_TIMESTAMPSET) )
		/* var &<# const when upper(var) <= upper(const) */
		hist_selec =
				calc_period_hist_selectivity_scalar(&const_upper,
													hist_upper, nhist, true);
	else if (
			operator == oper_oid(OVERLAPS_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(OVERLAPS_OP, T_PERIOD, T_PERIODSET) ||
			operator == oper_oid(OVERLAPS_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERLAPS_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(OVERLAPS_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(OVERLAPS_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(OVERLAPS_OP, T_PERIODSET, T_PERIOD) ||
			operator == oper_oid(OVERLAPS_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(CONTAINS_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(OVERLAPS_OP, T_PERIODSET, T_TIMESTAMPSET) ||
			operator == oper_oid(CONTAINS_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(CONTAINS_OP, T_TIMESTAMPSET, T_TIMESTAMPTZ))
	{
		/*
		 * A && B <=> NOT (A <<# B OR A #>> B).
		 *
		 * Since A <<# B and A #>> B are mutually exclusive events we can
		 * sum their probabilities to find probability of (A <<# B OR
		 * A #>> B).
		 *
		 * "(period/periodset ) @> timestamptz" is equivalent to
		 * "period && [elem,elem]". The
		 * caller already constructed the singular period from the element
		 * constant, so just treat it the same as &&.
		 */
		hist_selec =
				calc_period_hist_selectivity_scalar(&const_lower, hist_upper,
													nhist, false);
		hist_selec +=
				(1.0 - calc_period_hist_selectivity_scalar(&const_upper, hist_lower,
														   nhist, true));
		hist_selec = 1.0 - hist_selec;
	}
	else if (
			operator == oper_oid(CONTAINS_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(CONTAINS_OP, T_PERIOD, T_TIMESTAMPSET) ||
			operator == oper_oid(CONTAINS_OP, T_PERIOD, T_PERIODSET)||
			operator == oper_oid(CONTAINS_OP, T_TIMESTAMPSET, T_TIMESTAMPSET)||
			operator == oper_oid(CONTAINS_OP, T_PERIODSET, T_TIMESTAMPSET)||
			operator == oper_oid(CONTAINS_OP, T_PERIODSET, T_PERIOD)||
			operator == oper_oid(CONTAINS_OP, T_PERIODSET, T_PERIODSET))
		hist_selec =
				calc_period_hist_selectivity_contains(&const_lower,
													  &const_upper, hist_lower, nhist,
													  lslot.values, lslot.nvalues);
	else if (
			operator == oper_oid(CONTAINED_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(CONTAINED_OP, T_PERIOD, T_PERIODSET)||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPTZ, T_TIMESTAMPSET)||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPTZ, T_PERIOD)||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(CONTAINED_OP, T_TIMESTAMPSET, T_TIMESTAMPSET) ||
			operator == oper_oid(CONTAINED_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(CONTAINED_OP, T_PERIODSET, T_PERIOD) )
		hist_selec =
				calc_period_hist_selectivity_contained(&const_lower,
													   &const_upper, hist_lower, nhist,
													   lslot.values, lslot.nvalues);
	else if (
			operator == oper_oid(ADJACENT_OP, T_PERIOD, T_PERIOD) ||
			operator == oper_oid(ADJACENT_OP, T_PERIOD, T_PERIODSET)||
			operator == oper_oid(ADJACENT_OP, T_PERIOD, T_TIMESTAMPTZ) ||
			operator == oper_oid(ADJACENT_OP, T_PERIOD, T_TIMESTAMPSET)||
			operator == oper_oid(ADJACENT_OP, T_TIMESTAMPTZ, T_PERIOD)||
			operator == oper_oid(ADJACENT_OP, T_TIMESTAMPTZ, T_PERIODSET) ||
			operator == oper_oid(ADJACENT_OP, T_TIMESTAMPSET, T_PERIOD) ||
			operator == oper_oid(ADJACENT_OP, T_TIMESTAMPSET, T_PERIODSET) ||
			operator == oper_oid(ADJACENT_OP, T_PERIODSET, T_TIMESTAMPTZ) ||
			operator == oper_oid(ADJACENT_OP, T_PERIODSET, T_TIMESTAMPSET) ||
			operator == oper_oid(ADJACENT_OP, T_PERIODSET, T_PERIODSET) ||
			operator == oper_oid(ADJACENT_OP, T_PERIODSET, T_PERIOD) )
		hist_selec =
				calc_period_hist_selectivity_adjacent(&const_lower,
													  &const_upper, hist_lower, hist_upper,nhist);
	else
	{
		elog(ERROR, "unknown period operator %u", operator);
		hist_selec = -1.0;  /* keep compiler quiet */
	}

	free_attstatsslot(&lslot);
	free_attstatsslot(&hslot);

	return hist_selec;
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
	int			index;

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
 * Binary search on an array of period bounds. Returns greatest index of period
 * bound in array which is less(less or equal) than given period bound. If all
 * period bounds in array are greater or equal(greater) than given period bound,
 * return -1. When "equal" flag is set conditions in brackets are used.
 *
 * This function is used in scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop
 * interpolation of portion of bounds which are less or equal to given bound.
 */
 int
period_rbound_bsearch(PeriodBound *value, PeriodBound *hist,
			   int hist_length, bool equal)
{
	int			lower = -1,
				upper = hist_length - 1,
				cmp,
				middle;

	while (lower < upper)
	{
		middle = (lower + upper + 1) / 2;
		cmp = period_cmp_bounds(hist[middle].val, value->val, 
			hist[middle].lower, value->lower,
			hist[middle].inclusive, value->inclusive);

		if (cmp < 0 || (equal && cmp == 0))
			lower = middle;
		else
			upper = middle - 1;
	}
	return lower;
}


/*
 * Binary search on length histogram. Returns greatest index of period length in
 * histogram which is less than (less than or equal) the given length value. If
 * all lengths in the histogram are greater than (greater than or equal) the
 * given length, returns -1.
 */
 int
length_hist_bsearch(Datum *length_hist_values, int length_hist_nvalues,
					double value, bool equal)
{
	int			lower = -1,
				upper = length_hist_nvalues - 1,
				middle;

	while (lower < upper)
	{
		double		middleval;

		middle = (lower + upper + 1) / 2;

		middleval = DatumGetFloat8(length_hist_values[middle]);
		if (middleval < value || (equal && middleval <= value))
			lower = middle;
		else
			upper = middle - 1;
	}
	return lower;
}

/*
 * Get relative position of value in histogram bin in [0,1] period.
 */
 float8
get_period_position(PeriodBound *value, PeriodBound *hist1,
			 PeriodBound *hist2)
{
	float8		position;
	float8		bin_width;

	/* Calculate relative position using subdiff function. */
	bin_width = period_duration_secs(hist2->val, hist1->val);
	if (bin_width <= 0.0)
		return 0.5;			/* zero width bin */

	position = period_duration_secs(value->val, hist1->val)
		/ bin_width;

	/* Relative position must be in [0,1] period */
	position = Max(position, 0.0);
	position = Min(position, 1.0);
	return position;
}

/*
 * Get relative position of value in a length histogram bin in [0,1] period.
 */
 double
get_len_position(double value, double hist1, double hist2)
{
	/*
	 * Both bounds are finite. The value should be finite too, because it
	 * lies somewhere between the bounds. If it doesn't, just return
	 * something.
	 */
	return 1.0 - (hist2 - value) / (hist2 - hist1);
}

/*
 * Measure distance between two period bounds.
 */
 float8
get_period_distance(PeriodBound *bound1, PeriodBound *bound2)
{
	return period_duration_secs(bound2->val, bound1->val);
}

/*
 * Calculate the average of function P(x), in the interval [length1, length2],
 * where P(x) is the fraction of tuples with length < x (or length <= x if
 * 'equal' is true).
 */
 double
calc_length_hist_frac(Datum *length_hist_values, int length_hist_nvalues,
					  double length1, double length2, bool equal)
{
	double		frac;
	double		A,
				B,
				PA,
				PB;
	double		pos;
	int			i;
	double		area;

	Assert(length2 >= length1);

	if (length2 < 0.0)
		return 0.0;				/* shouldn't happen, but doesn't hurt to check */

	/*----------
	 * The average of a function between A and B can be calculated by the
	 * formula:
	 *
	 *			B
	 *	  1		/
	 * -------	| P(x)dx
	 *	B - A	/
	 *			A
	 *
	 * The geometrical interpretation of the integral is the area under the
	 * graph of P(x). P(x) is defined by the length histogram. We calculate
	 * the area in a piecewise fashion, iterating through the length histogram
	 * bins. Each bin is a trapezoid:
	 *
	 *		 P(x2)
	 *		  /|
	 *		 / |
	 * P(x1)/  |
	 *	   |   |
	 *	   |   |
	 *	---+---+--
	 *	   x1  x2
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
	 * P(x1) =	  (bin index) / (number of bins)
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
		double		bin_upper = DatumGetFloat8(length_hist_values[i + 1]);

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

	B = length2;				/* last bin ends at the query upper bound */
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
	 */
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
								PeriodBound *hist_lower, int hist_nvalues,
								Datum *length_hist_values, int length_hist_nvalues)
{
	int			i,
				upper_index;
	float8		prev_dist;
	double		bin_width;
	double		upper_bin_width;
	double		sum_frac;

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
	 * using linear interpolation of subdiff function.
	 */
	if (upper_index >= 0 && upper_index < hist_nvalues - 1)
		upper_bin_width = get_period_position(upper,
									   &hist_lower[upper_index],
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
		double		dist;
		double		length_hist_frac;
		bool		final_bin = false;

		/*
		 * dist -- distance from upper bound of query period to lower bound of
		 * the current bin in the lower bound histogram. Or to the lower bound
		 * of the constant period, if this is the final bin, containing the
		 * constant lower bound.
		 */
		if (period_cmp_bounds(hist_lower[i].val, lower->val, 
			hist_lower[i].lower, lower->lower,
			hist_lower[i].inclusive, lower->inclusive) < 0)
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
												 length_hist_nvalues,
												 prev_dist, dist, true);

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
							   PeriodBound *hist_lower, int hist_nvalues,
							   Datum *length_hist_values, int length_hist_nvalues)
{
	int			i,
				lower_index;
	double		bin_width,
				lower_bin_width;
	double		sum_frac;
	float8		prev_dist;

	/* Find the bin containing the lower bound of query period. */
	lower_index = period_rbound_bsearch(lower, hist_lower, hist_nvalues,
								 true);

	/*
	 * Calculate lower_bin_width, ie. the fraction of the of (lower_index,
	 * lower_index + 1) bin which is greater than lower bound of query period
	 * using linear interpolation of subdiff function.
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
		float8		dist;
		double		length_hist_frac;

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




/*
* Look up the fraction of values between lower(lowerbin) and lower, plus
* the fraction between upper and upper(upperbin).
*/
double calc_period_hist_selectivity_adjacent(PeriodBound *lower, PeriodBound *upper,
	PeriodBound *hist_lower, PeriodBound *hist_upper, int hist_nvalues)

{
	Selectivity selec1=0, selec2=0;
	int			index1, index2;

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

bool
get_attstatsslot_internal(AttStatsSlot *sslot, HeapTuple statstuple,
						  int reqkind, Oid reqop, int flags, StatisticsStrategy strategy) {
	Form_pg_statistic stats = (Form_pg_statistic) GETSTRUCT(statstuple);
	int i, start = 0, end = 0;  /* keep compiler quiet */
	Datum val;
	bool isnull;
	ArrayType *statarray;
	Oid arrayelemtype;
	int narrayelem;
	HeapTuple typeTuple;
	Form_pg_type typeForm;

	switch (strategy) {
		case VALUE_STATISTICS: {
			start = 0;
			end = 2;
			break;
		}
		case TEMPORAL_STATISTICS: {
			start = 2;
			end = 5;
			break;
		}
		case DEFAULT_STATISTICS: {
			start = 0;
			end = STATISTIC_NUM_SLOTS;
			break;
		}
		default: {
			break;
		}
	}

	/* initialize *sslot properly */
	memset(sslot, 0, sizeof(AttStatsSlot));

	for (i = start; i < end; i++) {
		if ((&stats->stakind1)[i] == reqkind &&
			(reqop == InvalidOid || (&stats->staop1)[i] == reqop))
			break;
	}
	if (i >= end)
		return false;			/* not there */

	sslot->staop = (&stats->staop1)[i];

	if (flags & ATTSTATSSLOT_VALUES) {
		val = SysCacheGetAttr(STATRELATTINH, statstuple,
							  Anum_pg_statistic_stavalues1 + i,
							  &isnull);
		if (isnull)
			elog(ERROR, "stavalues is null");

		/*
		 * Detoast the array if needed, and in any case make a copy that's
		 * under control of this AttStatsSlot.
		 */
		statarray = DatumGetArrayTypePCopy(val);

		/*
		 * Extract the actual array element type, and pass it after in case the
		 * caller needs it.
		 */
		sslot->valuetype = arrayelemtype = ARR_ELEMTYPE(statarray);

		/* Need info about element type */
		typeTuple = SearchSysCache1(TYPEOID, ObjectIdGetDatum(arrayelemtype));
		if (!HeapTupleIsValid(typeTuple))
			elog(ERROR, "cache lookup failed for type %u", arrayelemtype);
		typeForm = (Form_pg_type) GETSTRUCT(typeTuple);

		/* Deconstruct array into Datum elements; NULLs not expected */
		deconstruct_array(statarray,
						  arrayelemtype,
						  typeForm->typlen,
						  typeForm->typbyval,
						  typeForm->typalign,
						  &sslot->values, NULL, &sslot->nvalues);

		/*
		 * If the element type is pass-by-reference, we now have a bunch of
		 * Datums that are pointers into the statarray, so we need to keep
		 * that until free_attstatsslot.  Otherwise, all the useful info is in
		 * sslot->values[], so we can free the array object immediately.
		 */
		if (!typeForm->typbyval)
			sslot->values_arr = statarray;
		else
			pfree(statarray);

		ReleaseSysCache(typeTuple);
	}

	if (flags & ATTSTATSSLOT_NUMBERS) {
		val = SysCacheGetAttr(STATRELATTINH, statstuple,
							  Anum_pg_statistic_stanumbers1 + i,
							  &isnull);
		if (isnull)
			elog(ERROR, "stanumbers is null");

		/*
		 * Detoast the array if needed, and in any case make a copy that's
		 * under control of this AttStatsSlot.
		 */
		statarray = DatumGetArrayTypePCopy(val);

		/*
		 * We expect the array to be a 1-D float4 array; verify that. We don't
		 * need to use deconstruct_array() since the array data is just going
		 * to look like a C array of float4 values.
		 */
		narrayelem = ARR_DIMS(statarray)[0];
		if (ARR_NDIM(statarray) != 1 || narrayelem <= 0 ||
			ARR_HASNULL(statarray) ||
			ARR_ELEMTYPE(statarray) != FLOAT4OID)
			elog(ERROR, "stanumbers is not a 1-D float4 array");

		/* Give caller a pointer directly into the statarray */
		sslot->numbers = (float4 *) ARR_DATA_PTR(statarray);
		sslot->nnumbers = narrayelem;

		/* We'll free the statarray in free_attstatsslot */
		sslot->numbers_arr = statarray;
	}

	return true;
}