/*****************************************************************************
 *
 * temporal_analyze.c
 *	  Functions for gathering statistics from temporal alphanumeric columns
 *
 * Various kind of statistics are collected for both the value and the time
 * dimension of temporal types. The kind of statistics depends on the duration
 * of the temporal type, which is defined in the table schema by the typmod
 * attribute. Please refer to the PostgreSQL file pg_statistic_d.h for more
 * information about the statistics collected.
 * 
 * For TemporalInst
 * - Slot 1
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_MCV.
 * 		- staop contains the OID of the "=" operator for the value dimension.
 * 		- stavalues stores the most common non-null values (MCV) for the value dimension.
 * 		- stanumbers stores the frequencies of the MCV for the value dimension.
 * 		- numnumbers contains the number of elements in the stanumbers array.
 * 		- numvalues contains the number of elements in the most common values array.
 * - Slot 2
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_HISTOGRAM.
 * 		- staop contains the OID of the "<" operator that describes the sort ordering.
 * 		- stavalues stores the histogram of scalar data for the value dimension
 * 		- numvalues contains the number of buckets in the histogram.
 * - Slot 3
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_CORRELATION.
 * 		- staop contains the OID of the "<" operator that describes the sort ordering.
 * 		- stavalues is NULL
 * 		- stanumbers contains the correlation coefficient between the sequence 
 * 		  of data values and the sequence of their actual tuple positions
 * 		- numvalues contains the number of buckets in the histogram.
 * - Slot 4
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_MCV.
 * 		- staop contains the "=" operator of the time dimension.
 * 		- stavalues stores the most common values (MCV) for the time dimension.
 * 		- stanumbers stores the frequencies of the MCV for the time dimension.
 * 		- numnumbers contains the number of elements in the stanumbers array.
 * 		- numvalues contains the number of elements in the most common values array.
 * - Slot 5
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_HISTOGRAM.
 * 		- staop contains the OID of the "<" operator that describes the sort ordering.
 * 		- stavalues stores the histogram for the time dimension.
 * For all other durations
 * - Slot 1
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_BOUNDS_HISTOGRAM.
 * 		- staop contains the "<" operator of the value dimension.
 * 		- stavalues stores the histogram of ranges for the value dimension.
 * 		- numvalues contains the number of buckets in the histogram.
 * - Slot 2
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM.
 * 		- staop contains the "<" operator to the value dimension.
 * 		- stavalues stores the length of the histogram of ranges for the value dimension.
 * 		- numvalues contains the number of buckets in the histogram.
 * - Slot 3
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_PERIOD_BOUNDS_HISTOGRAM.
 * 		- staop contains the "<" operator of the time dimension.
 * 		- stavalues stores the histogram of periods for the time dimension.
 * 		- numvalues contains the number of buckets in the histogram.
 * - Slot 4
 * 		- stakind contains the type of statistics which is STATISTIC_KIND_PERIOD_LENGTH_HISTOGRAM.
 * 		- staop contains the "<" operator of the time dimension.
 * 		- stavalues stores the length of the histogram of periods for the time dimension.
 * 		- numvalues contains the number of buckets in the histogram.
 *
 * Notice that some statistics may not be collected, for example, since there
 * are no most common values. In that case, the next statistics collected is
 * stored in the next available slot.
 *
 * In the case of temporal types having a Period as bounding box, that is,
 * tbool and ttext, no statistics are collected for the value dimension and
 * the statistics for the temporal part are stored in slots 1 and 2.
 * 
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_analyze.h"

#include <assert.h>
#include <math.h>
#include <access/tuptoaster.h>
#if MOBDB_PGSQL_VERSION < 110000
#include <catalog/pg_collation.h>
#include <catalog/pg_operator.h>
#else
#include <catalog/pg_collation_d.h>
#include <catalog/pg_operator_d.h>
#endif
#include <commands/vacuum.h>
#include <parser/parse_oper.h>
#include <utils/datum.h>
#include <utils/fmgrprotos.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>

#include "period.h"
#include "time_analyze.h"
#include "rangetypes_ext.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_analyze.h"

/*
 * To avoid consuming too much memory, IO and CPU load during analysis, and/or
 * too much space in the resulting pg_statistic rows, we ignore temporal values
 * that are wider than TEMPORAL_WIDTH_THRESHOLD (after detoasting!).  Note that 
 * this number is bigger than the similar WIDTH_THRESHOLD limit used in
 * analyze.c's standard typanalyze code, which is 1024.
 */
#define TEMPORAL_WIDTH_THRESHOLD 4096 // Should it be 0x10000 i.e. 64K as before ?

/*
 * While statistic functions are running, we keep a pointer to the extra data
 * here for use by assorted subroutines.  The functions doesn't
 * currently need to be re-entrant, so avoiding this is not worth the extra
 * notational cruft that would be needed.
 */
TemporalAnalyzeExtraData *temporal_extra_data;

/*****************************************************************************
 * Functions copied from files analyze.c and rangetypes_typanalyze.c since
 * they are not exported.
 *****************************************************************************/

/*
 * qsort_arg comparator for sorting ScalarItems
 *
 * Aside from sorting the items, we update the tupnoLink[] array
 * whenever two ScalarItems are found to contain equal datums.  The array
 * is indexed by tupno; for each ScalarItem, it contains the highest
 * tupno that that item's datum has been found to be equal to.  This allows
 * us to avoid additional comparisons in compute_scalar_stats().
 */
static int
compare_scalars(const void *a, const void *b, void *arg)
{
	Datum da = ((const ScalarItem *) a)->value;
	int ta = ((const ScalarItem *) a)->tupno;
	Datum db = ((const ScalarItem *) b)->value;
	int tb = ((const ScalarItem *) b)->tupno;
	CompareScalarsContext *cxt = (CompareScalarsContext *) arg;
	int compare;

	compare = ApplySortComparator(da, false, db, false, cxt->ssup);
	if (compare != 0)
		return compare;

	/*
	 * The two datums are equal, so update cxt->tupnoLink[].
	 */
	if (cxt->tupnoLink[ta] < tb)
		cxt->tupnoLink[ta] = tb;
	if (cxt->tupnoLink[tb] < ta)
		cxt->tupnoLink[tb] = ta;

	/*
	 * For equal datums, sort by tupno
	 */
	return ta - tb;
}

/*
 * qsort comparator for sorting ScalarMCVItems by position
 */
static int
compare_mcvs(const void *a, const void *b)
{
	int da = ((const ScalarMCVItem *) a)->first;
	int db = ((const ScalarMCVItem *) b)->first;

	return da - db;
}

/*
 * Comparison function for sorting RangeBounds.
 */
static int
range_bound_qsort_cmp(const void *a1, const void *a2, void *arg)
{
	RangeBound *b1 = (RangeBound *) a1;
	RangeBound *b2 = (RangeBound *) a2;
	TypeCacheEntry *typcache = (TypeCacheEntry *) arg;

	return range_cmp_bounds(typcache, b1, b2);
}

/*
 * Analyze the list of common values in the sample and decide how many are
 * worth storing in the table's MCV list.
 *
 * mcv_counts is assumed to be a list of the counts of the most common values
 * seen in the sample, starting with the most common.  The return value is the
 * number that are significantly more common than the values not in the list,
 * and which are therefore deemed worth storing in the table's MCV list.
 * 
 * Function copied from PostgreSQL file analyze.c
 */
static int
analyze_mcv_list(const int *mcv_counts,
				 int num_mcv,
				 double stadistinct,
				 double stanullfrac,
				 int samplerows,
				 double totalrows)
{
	double		ndistinct_table;
	double		sumcount;
	int			i;

	/*
	 * If the entire table was sampled, keep the whole list.  This also
	 * protects us against division by zero in the code below.
	 */
	if (samplerows == totalrows || totalrows <= 1.0)
		return num_mcv;

	/* Re-extract the estimated number of distinct nonnull values in table */
	ndistinct_table = stadistinct;
	if (ndistinct_table < 0)
		ndistinct_table = -ndistinct_table * totalrows;

	/*
	 * Exclude the least common values from the MCV list, if they are not
	 * significantly more common than the estimated selectivity they would
	 * have if they weren't in the list.  All non-MCV values are assumed to be
	 * equally common, after taking into account the frequencies of all the
	 * the values in the MCV list and the number of nulls (c.f. eqsel()).
	 *
	 * Here sumcount tracks the total count of all but the last (least common)
	 * value in the MCV list, allowing us to determine the effect of excluding
	 * that value from the list.
	 *
	 * Note that we deliberately do this by removing values from the full
	 * list, rather than starting with an empty list and adding values,
	 * because the latter approach can fail to add any values if all the most
	 * common values have around the same frequency and make up the majority
	 * of the table, so that the overall average frequency of all values is
	 * roughly the same as that of the common values.  This would lead to any
	 * uncommon values being significantly overestimated.
	 */
	sumcount = 0.0;
	for (i = 0; i < num_mcv - 1; i++)
		sumcount += mcv_counts[i];

	while (num_mcv > 0)
	{
		double		selec,
					otherdistinct,
					N,
					n,
					K,
					variance,
					stddev;

		/*
		 * Estimated selectivity the least common value would have if it
		 * wasn't in the MCV list (c.f. eqsel()).
		 */
		selec = 1.0 - sumcount / samplerows - stanullfrac;
		if (selec < 0.0)
			selec = 0.0;
		if (selec > 1.0)
			selec = 1.0;
		otherdistinct = ndistinct_table - (num_mcv - 1);
		if (otherdistinct > 1)
			selec /= otherdistinct;

		/*
		 * If the value is kept in the MCV list, its population frequency is
		 * assumed to equal its sample frequency.  We use the lower end of a
		 * textbook continuity-corrected Wald-type confidence interval to
		 * determine if that is significantly more common than the non-MCV
		 * frequency --- specifically we assume the population frequency is
		 * highly likely to be within around 2 standard errors of the sample
		 * frequency, which equates to an interval of 2 standard deviations
		 * either side of the sample count, plus an additional 0.5 for the
		 * continuity correction.  Since we are sampling without replacement,
		 * this is a hypergeometric distribution.
		 *
		 * XXX: Empirically, this approach seems to work quite well, but it
		 * may be worth considering more advanced techniques for estimating
		 * the confidence interval of the hypergeometric distribution.
		 */
		N = totalrows;
		n = samplerows;
		K = N * mcv_counts[num_mcv - 1] / n;
		variance = n * K * (N - K) * (N - n) / (N * N * (N - 1));
		stddev = sqrt(variance);

		if (mcv_counts[num_mcv - 1] > selec * samplerows + 2 * stddev + 0.5)
		{
			/*
			 * The value is significantly more common than the non-MCV
			 * selectivity would suggest.  Keep it, and all the other more
			 * common values in the list.
			 */
			break;
		}
		else
		{
			/* Discard this value and consider the next least common value */
			num_mcv--;
			if (num_mcv == 0)
				break;
			sumcount -= mcv_counts[num_mcv - 1];
		}
	}
	return num_mcv;
}

/*****************************************************************************
 * Compute statistics for scalar values, used both for the value and the 
 * time dimension of TemporalInst columns. Function derived from the inner 
 * part of function compute_scalar_stats of file analyze.c so that it can be 
 * called twice, for the value and for the time dimension.
 *****************************************************************************/

static int
compute_scalar_stats_mdb(VacAttrStats *stats, int values_cnt, bool is_varwidth, 
	int toowide_cnt, ScalarItem *values, int *tupnoLink, ScalarMCVItem *track, 
	Oid valuetypid, int nonnull_cnt, int null_cnt, 
	int slot_idx, int total_width, int totalrows, int samplerows, bool correlation)
{
	int			ndistinct,	/* # distinct values in sample */
				nmultiple,	/* # that appear multiple times */
				num_hist,
				dups_cnt,
				track_cnt = 0,
				num_mcv = stats->attr->attstattarget,
				num_bins = stats->attr->attstattarget,
				i;
	CompareScalarsContext cxt;
	SortSupportData ssup;
	double		corr_xysum;
	bool 		typbyval = get_typbyval_fast(valuetypid);
	int			typlen = get_typlen_fast(valuetypid);
	Oid 		ltopr, eqopr;

	memset(&ssup, 0, sizeof(ssup));
	ssup.ssup_cxt = CurrentMemoryContext;
	/* We always use the default collation for statistics */
	ssup.ssup_collation = DEFAULT_COLLATION_OID;
	ssup.ssup_nulls_first = false;

	/*
	 * For now, don't perform abbreviated key conversion, because full values
	 * are required for MCV slot generation.  Supporting that optimization
	 * would necessitate teaching compare_scalars() to call a tie-breaker.
	 */
	ssup.abbreviate = false;

	/* With respect to the original PostgreSQL function we need to look for 
	 * the specific operators for the value and type components */
	get_sort_group_operators(valuetypid,
							 false, false, false,
							 &ltopr, &eqopr, NULL,
							 NULL);
	PrepareSortSupportFromOrderingOp(ltopr, &ssup);

	/* Sort the collected values */
	cxt.ssup = &ssup;
	cxt.tupnoLink = tupnoLink;
	qsort_arg((void *) values, (size_t) values_cnt, sizeof(ScalarItem),
			  compare_scalars, (void *) &cxt);

	/*
	 * Now scan the values in order, find the most common ones, and also
	 * accumulate ordering-correlation statistics.
	 *
	 * To determine which are most common, we first have to count the
	 * number of duplicates of each value.  The duplicates are adjacent in
	 * the sorted list, so a brute-force approach is to compare successive
	 * datum values until we find two that are not equal. However, that
	 * requires N-1 invocations of the datum comparison routine, which are
	 * completely redundant with work that was done during the sort.  (The
	 * sort algorithm must at some point have compared each pair of items
	 * that are adjacent in the sorted order; otherwise it could not know
	 * that it's ordered the pair correctly.) We exploit this by having
	 * compare_scalars remember the highest tupno index that each
	 * ScalarItem has been found equal to.  At the end of the sort, a
	 * ScalarItem's tupnoLink will still point to itself if and only if it
	 * is the last item of its group of duplicates (since the group will
	 * be ordered by tupno).
	 */
	corr_xysum = 0;
	ndistinct = 0;
	nmultiple = 0;
	dups_cnt = 0;
	for (i = 0; i < values_cnt; i++)
	{
		int			tupno = values[i].tupno;

		corr_xysum += ((double) i) * ((double) tupno);
		dups_cnt++;
		if (tupnoLink[tupno] == tupno)
		{
			/* Reached end of duplicates of this value */
			ndistinct++;
			if (dups_cnt > 1)
			{
				nmultiple++;
				if (track_cnt < num_mcv ||
					dups_cnt > track[track_cnt - 1].count)
				{
					/*
					 * Found a new item for the mcv list; find its
					 * position, bubbling down old items if needed. Loop
					 * invariant is that j points at an empty/ replaceable
					 * slot.
					 */
					int			j;

					if (track_cnt < num_mcv)
						track_cnt++;
					for (j = track_cnt - 1; j > 0; j--)
					{
						if (dups_cnt <= track[j - 1].count)
							break;
						track[j].count = track[j - 1].count;
						track[j].first = track[j - 1].first;
					}
					track[j].count = dups_cnt;
					track[j].first = i + 1 - dups_cnt;
				}
			}
			dups_cnt = 0;
		}
	}

	stats->stats_valid = true;
	/* Do the simple null-frac and width stats */
	stats->stanullfrac = (float) ((double) null_cnt / (double) samplerows);
	if (is_varwidth)
		stats->stawidth = (int) (total_width / (double) nonnull_cnt);
	else
		stats->stawidth = stats->attrtype->typlen;

	if (nmultiple == 0)
	{
		/*
		 * If we found no repeated non-null values, assume it's a unique
		 * column; but be sure to discount for any nulls we found.
		 */
		stats->stadistinct = (float) (-1.0 * (1.0 - stats->stanullfrac));
	}
	else if (toowide_cnt == 0 && nmultiple == ndistinct)
	{
		/*
		 * Every value in the sample appeared more than once.  Assume the
		 * column has just these values.  (This case is meant to address
		 * columns with small, fixed sets of possible values, such as
		 * boolean or enum columns.  If there are any values that appear
		 * just once in the sample, including too-wide values, we should
		 * assume that that's not what we're dealing with.)
		 */
		stats->stadistinct = ndistinct;
	}
	else
	{
		/*----------
		 * Estimate the number of distinct values using the estimator
		 * proposed by Haas and Stokes in IBM Research Report RJ 10025:
		 *		n*d / (n - f1 + f1*n/N)
		 * where f1 is the number of distinct values that occurred
		 * exactly once in our sample of n rows (from a total of N),
		 * and d is the total number of distinct values in the sample.
		 * This is their Duj1 estimator; the other estimators they
		 * recommend are considerably more complex, and are numerically
		 * very unstable when n is much smaller than N.
		 *
		 * In this calculation, we consider only non-nulls.  We used to
		 * include rows with null values in the n and N counts, but that
		 * leads to inaccurate answers in columns with many nulls, and
		 * it's intuitively bogus anyway considering the desired result is
		 * the number of distinct non-null values.
		 *
		 * Overwidth values are assumed to have been distinct.
		 *----------
		 */
		int			f1 = ndistinct - nmultiple + toowide_cnt;
		int			d = f1 + nmultiple;
		double		n = samplerows - null_cnt;
		double		N = totalrows * (1.0 - stats->stanullfrac);
		double		stadistinct;

		/* N == 0 shouldn't happen, but just in case ... */
		if (N > 0)
			stadistinct = (n * d) / ((n - f1) + f1 * n / N);
		else
			stadistinct = 0;

		/* Clamp to sane range in case of roundoff error */
		if (stadistinct < d)
			stadistinct = d;
		if (stadistinct > N)
			stadistinct = N;
		/* And round to integer */
		stats->stadistinct = (float) floor(stadistinct + 0.5);
	}

	/*
	 * If we estimated the number of distinct values at more than 10% of
	 * the total row count (a very arbitrary limit), then assume that
	 * stadistinct should scale with the row count rather than be a fixed
	 * value.
	 */
	if (stats->stadistinct > 0.1 * totalrows)
		stats->stadistinct = -(stats->stadistinct / totalrows);

	/*
	 * Decide how many values are worth storing as most-common values. If
	 * we are able to generate a complete MCV list (all the values in the
	 * sample will fit, and we think these are all the ones in the table),
	 * then do so.  Otherwise, store only those values that are
	 * significantly more common than the values not in the list.
	 *
	 * Note: the first of these cases is meant to address columns with
	 * small, fixed sets of possible values, such as boolean or enum
	 * columns.  If we can *completely* represent the column population by
	 * an MCV list that will fit into the stats target, then we should do
	 * so and thus provide the planner with complete information.  But if
	 * the MCV list is not complete, it's generally worth being more
	 * selective, and not just filling it all the way up to the stats
	 * target.
	 */
	if (track_cnt == ndistinct && toowide_cnt == 0 &&
		stats->stadistinct > 0 &&
		track_cnt <= num_mcv)
	{
		/* Track list includes all values seen, and all will fit */
		num_mcv = track_cnt;
	}
	else
	{
		/* Incomplete list; decide how many values are worth keeping */
		if (num_mcv > track_cnt)
			num_mcv = track_cnt;

		if (num_mcv > 0)
		{
            int	*mcv_counts = (int *) palloc(num_mcv * sizeof(int));
			for (i = 0; i < num_mcv; i++)
				mcv_counts[i] = track[i].count;

			num_mcv = analyze_mcv_list(mcv_counts, num_mcv,
									   stats->stadistinct,
									   stats->stanullfrac,
									   samplerows, totalrows);
		}
	}

	/* Generate MCV slot entry */
	if (num_mcv > 0)
	{
		MemoryContext old_context;
		Datum	   *mcv_values;
		float4	   *mcv_freqs;

		/* Must copy the target values into anl_context */
		old_context = MemoryContextSwitchTo(stats->anl_context);
		mcv_values = (Datum *) palloc(num_mcv * sizeof(Datum));
		mcv_freqs = (float4 *) palloc(num_mcv * sizeof(float4));
		for (i = 0; i < num_mcv; i++)
		{
			mcv_values[i] = datumCopy(values[track[i].first].value, 
				typbyval,typlen);
			mcv_freqs[i] = (float) ((double) track[i].count / (double) samplerows);
		}
		MemoryContextSwitchTo(old_context);

		stats->stakind[slot_idx] = STATISTIC_KIND_MCV;
		stats->staop[slot_idx] = eqopr;
		stats->stanumbers[slot_idx] = mcv_freqs;
		stats->numnumbers[slot_idx] = num_mcv;
		stats->stavalues[slot_idx] = mcv_values;
		stats->numvalues[slot_idx] = num_mcv;
		stats->statyplen[slot_idx] = (int16) typlen;
		stats->statypid[slot_idx] = valuetypid;
		stats->statypbyval[slot_idx] = typbyval;

		/*
		 * Accept the defaults for stats->statypid and others. They have
		 * been set before we were called (see vacuum.h)
		 */
		slot_idx++;
	}

	/*
	 * Generate a histogram slot entry if there are at least two distinct
	 * values not accounted for in the MCV list.  (This ensures the
	 * histogram won't collapse to empty or a singleton.)
	 */
	num_hist = ndistinct - num_mcv;
	if (num_hist > num_bins)
		num_hist = num_bins + 1;
	if (num_hist >= 2)
	{
		MemoryContext old_context;
		Datum	   *hist_values;
		int			nvals;
		int			pos,
					posfrac,
					delta,
					deltafrac;

		/* Sort the MCV items into position order to speed next loop */
		qsort((void *) track, (size_t) num_mcv, sizeof(ScalarMCVItem),
			compare_mcvs);

		/*
		 * Collapse out the MCV items from the values[] array.
		 *
		 * Note we destroy the values[] array here... but we don't need it
		 * for anything more.  We do, however, still need values_cnt.
		 * nvals will be the number of remaining entries in values[].
		 */
		if (num_mcv > 0)
		{
			int			src,
						dest;
			int			j;

			src = dest = 0;
			j = 0;			/* index of next interesting MCV item */
			while (src < values_cnt)
			{
				int			ncopy;

				if (j < num_mcv)
				{
					int			first = track[j].first;

					if (src >= first)
					{
						/* advance past this MCV item */
						src = first + track[j].count;
						j++;
						continue;
					}
					ncopy = first - src;
				}
				else
					ncopy = values_cnt - src;
				memmove(&values[dest], &values[src],
						ncopy * sizeof(ScalarItem));
				src += ncopy;
				dest += ncopy;
			}
			nvals = dest;
		}
		else
			nvals = values_cnt;
		assert(nvals >= num_hist);

		/* Must copy the target values into anl_context */
		old_context = MemoryContextSwitchTo(stats->anl_context);
		hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

		/*
		 * The object of this loop is to copy the first and last values[]
		 * entries along with evenly-spaced values in between.  So the
		 * i'th value is values[(i * (nvals - 1)) / (num_hist - 1)].  But
		 * computing that subscript directly risks integer overflow when
		 * the stats target is more than a couple thousand.  Instead we
		 * add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
		 * the integral and fractional parts of the sum separately.
		 */
		delta = (nvals - 1) / (num_hist - 1);
		deltafrac = (nvals - 1) % (num_hist - 1);
		pos = posfrac = 0;

		for (i = 0; i < num_hist; i++)
		{
			hist_values[i] = datumCopy(values[pos].value, typbyval, typlen);
			pos += delta;
			posfrac += deltafrac;
			if (posfrac >= (num_hist - 1))
			{
				/* fractional part exceeds 1, carry to integer part */
				pos++;
				posfrac -= (num_hist - 1);
			}
		}

		MemoryContextSwitchTo(old_context);

		stats->stakind[slot_idx] = STATISTIC_KIND_HISTOGRAM;
		stats->staop[slot_idx] = ltopr;
		stats->stavalues[slot_idx] = hist_values;
		stats->numvalues[slot_idx] = num_hist;
		stats->statyplen[slot_idx] = (int16) typlen;
		stats->statypid[slot_idx] = valuetypid;
		stats->statypbyval[slot_idx] = typbyval;

		/*
		 * Accept the defaults for stats->statypid and others. They have
		 * been set before we were called (see vacuum.h)
		 */
		slot_idx++;
	}

	/* Generate a correlation entry only for the value component and if there
	 * are multiple values */
	if (correlation && values_cnt > 1)
	{
		MemoryContext old_context;
		float4	   *corrs;
		double		corr_xsum,
					corr_x2sum;

		/* Must copy the target values into anl_context */
		old_context = MemoryContextSwitchTo(stats->anl_context);
		corrs = (float4 *) palloc(sizeof(float4));
		MemoryContextSwitchTo(old_context);

		/*----------
		 * Since we know the x and y value sets are both
		 *		0, 1, ..., values_cnt-1
		 * we have sum(x) = sum(y) =
		 *		(values_cnt-1)*values_cnt / 2
		 * and sum(x^2) = sum(y^2) =
		 *		(values_cnt-1)*values_cnt*(2*values_cnt-1) / 6.
		 *----------
		 */
		corr_xsum = ((double) (values_cnt - 1)) *
			((double) values_cnt) / 2.0;
		corr_x2sum = ((double) (values_cnt - 1)) *
			((double) values_cnt) * (double) (2 * values_cnt - 1) / 6.0;

		/* And the correlation coefficient reduces to */
		corrs[0] = (float) ((values_cnt * corr_xysum - corr_xsum * corr_xsum) /
			(values_cnt * corr_x2sum - corr_xsum * corr_xsum));

		stats->stakind[slot_idx] = STATISTIC_KIND_CORRELATION;
		stats->staop[slot_idx] = ltopr;
		stats->stanumbers[slot_idx] = corrs;
		stats->numnumbers[slot_idx] = 1;
		slot_idx++;
	}
	/* Returns the next available slot */
	return slot_idx;
}

/*****************************************************************************
 * Generic statistics functions for alphanumeric temporal types.
 * In these functions the last argument valuestats determines whether
 * statistics are computed for the value dimension, that is, it is true for
 * temporal numbers. Otherwise, statistics are computed only for the temporal
 * dimension, that is, in the the case of temporal boolean and temporal text.
 *****************************************************************************/

/* 
 * Compute statistics for TemporalInst columns.
 * Function derived from compute_scalar_stats of file analyze.c 
 */
static void
tempinst_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					   int samplerows, double totalrows, bool valuestats)
{
	int			i;
	int			null_cnt = 0;
	int			nonnull_cnt = 0;
	int			toowide_cnt = 0;
	double		total_width = 0;
	bool		is_varwidth = (!stats->attrtype->typbyval &&
							   stats->attrtype->typlen < 0);
	ScalarItem *scalar_values, 
			   *timestamp_values;
	int			values_cnt = 0;
	int 	   *scalar_tupnoLink,
			   *timestamp_tupnoLink;
	ScalarMCVItem *scalar_track,
			   *timestamp_track;
	int			num_mcv = stats->attr->attstattarget;
	Oid valuetypid;

	if (valuestats)
	{
		scalar_values = (ScalarItem *) palloc(samplerows * sizeof(ScalarItem));
		scalar_tupnoLink = (int *) palloc(samplerows * sizeof(int));
		scalar_track = (ScalarMCVItem *) palloc(num_mcv * sizeof(ScalarMCVItem));
		valuetypid = base_oid_from_temporal(stats->attrtypid);
	}

	timestamp_values = (ScalarItem *) palloc(samplerows * sizeof(ScalarItem));
	timestamp_tupnoLink = (int *) palloc(samplerows * sizeof(int));
	timestamp_track = (ScalarMCVItem *) palloc(num_mcv * sizeof(ScalarMCVItem));

	/* Initial scan to find sortable values */
	for (i = 0; i < samplerows; i++)
	{
		Datum		value;
		bool		isnull;
		TemporalInst *inst;

		vacuum_delay_point();

		value = fetchfunc(stats, i, &isnull);

		/* Check for null/nonnull */
		if (isnull)
		{
			null_cnt++;
			continue;
		}
		nonnull_cnt++;

		/*
		 * Since it is variable-width field, add up widths for average width
		 * calculation.  Note that if the value is toasted, we use the toasted
		 * width.  We don't bother with this calculation if it's a fixed-width
		 * type.
		 */
		total_width += VARSIZE_ANY(DatumGetPointer(value));

		/*
		 * If the value is toasted, we want to detoast it just once to
		 * avoid repeated detoastings and resultant excess memory usage
		 * during the comparisons.  Also, check to see if the value is
		 * excessively wide, and if so don't detoast at all --- just
		 * ignore the value.
		 */
		if (toast_raw_datum_size(value) > TEMPORAL_WIDTH_THRESHOLD)
		{
			toowide_cnt++;
			continue;
		}
		value = PointerGetDatum(PG_DETOAST_DATUM(value));

		/* Get the temporal instant value and add its value and its timestamp
		 * dimensions to the lists to be sorted */
		inst = DatumGetTemporalInst(value);
		if (valuestats)
		{
			scalar_values[values_cnt].value = temporalinst_value(inst);
			scalar_values[values_cnt].tupno = values_cnt;
			scalar_tupnoLink[values_cnt] = values_cnt;
		}
		timestamp_values[values_cnt].value = TimestampTzGetDatum(inst->t);
		timestamp_values[values_cnt].tupno = values_cnt;
		timestamp_tupnoLink[values_cnt] = values_cnt;

		values_cnt++;
	}

	/* We can only compute real stats if we found some sortable values. */
	if (values_cnt > 0)
	{
		int			slot_idx = 0;

		if (valuestats)
		{
			/* Compute the statistics for the value dimension */
			slot_idx = compute_scalar_stats_mdb(stats, values_cnt, is_varwidth, 
				toowide_cnt, scalar_values, scalar_tupnoLink, scalar_track, 
				valuetypid, nonnull_cnt, null_cnt, slot_idx, 
				(int) total_width, (int) totalrows, samplerows, true);
		}

		/* Compute the statistics for the time dimension 
		 * We don't need to get the next available slot */
		compute_scalar_stats_mdb(stats, values_cnt, is_varwidth,
			toowide_cnt, timestamp_values, timestamp_tupnoLink, timestamp_track, 
			TIMESTAMPTZOID, nonnull_cnt, null_cnt, slot_idx, 
			(int) total_width, (int) totalrows, samplerows, false);
	}
	else if (nonnull_cnt > 0)
	{
		/* We found some non-null values, but they were all too wide */
		assert(nonnull_cnt == toowide_cnt);
		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (float) ((double) null_cnt / (double) samplerows);
		if (is_varwidth)
			stats->stawidth = (int32) (total_width / (double) nonnull_cnt);
		else
			stats->stawidth = stats->attrtype->typlen;
		/* Assume all too-wide values are distinct, so it's a unique column */
		stats->stadistinct = (float) (-1.0 * (1.0 - stats->stanullfrac));
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		if (is_varwidth)
			stats->stawidth = 0;	/* "unknown" */
		else
			stats->stawidth = stats->attrtype->typlen;
		stats->stadistinct = 0.0;	/* "unknown" */
	}

	/* We don't need to bother cleaning up any of our temporary palloc's */
}

/* 
 * Compute statistics for the value dimension for ranges
 * Function derived from compute_range_stats of file rangetypes_typanalyze.c 
 */

void
range_compute_stats(VacAttrStats *stats, int non_null_cnt, int *slot_idx, 
	RangeBound *lowers, RangeBound *uppers, float8 *lengths,
	TypeCacheEntry *typcache, Oid rangetypid)
{
	int num_hist,
		num_bins = stats->attr->attstattarget;
	float4	   *emptyfrac;
	Datum *bound_hist_values;
	Datum *length_hist_values;
	MemoryContext old_cxt;

	/* Must copy the target values into anl_context */
	old_cxt = MemoryContextSwitchTo(stats->anl_context);

	/*
	 * Generate a bounds histogram and a length histogram slot entries 
	 * if there are at least two values.
	 */
	if (non_null_cnt >= 2)
	{
		/* Generate a bounds histogram slot entry */

		/* Sort bound values */
		qsort_arg(lowers, (size_t) non_null_cnt, sizeof(RangeBound),
			range_bound_qsort_cmp, typcache);
		qsort_arg(uppers, (size_t) non_null_cnt, sizeof(RangeBound),
			range_bound_qsort_cmp, typcache);

		num_hist = non_null_cnt;
		if (num_hist > num_bins)
			num_hist = num_bins + 1;

		bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

		/*
		* The object of this loop is to construct ranges from first and
		* last entries in lowers[] and uppers[] along with evenly-spaced
		* values in between. So the i'th value is a range of lowers[(i *
		* (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
		* (num_hist - 1)]. But computing that subscript directly risks
		* integer overflow when the stats target is more than a couple
		* thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
		* at each step, tracking the integral and fractional parts of the
		* sum separately.
		*/
		int delta = (non_null_cnt - 1) / (num_hist - 1);
        int deltafrac = (non_null_cnt - 1) % (num_hist - 1);
		int pos = 0, posfrac = 0;

		for (int i = 0; i < num_hist; i++)
		{
			bound_hist_values[i] = PointerGetDatum(
				range_serialize(typcache, &lowers[pos], 
					&uppers[pos], false));

			pos += delta;
			posfrac += deltafrac;
			if (posfrac >= (num_hist - 1))
			{
				/* fractional part exceeds 1, carry to integer part */
				pos++;
				posfrac -= (num_hist - 1);
			}
		}

		TypeCacheEntry *range_typeentry = lookup_type_cache(rangetypid,
			TYPECACHE_EQ_OPR | TYPECACHE_CMP_PROC_FINFO |
			TYPECACHE_HASH_PROC_FINFO);

		stats->stakind[*slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
		stats->staop[*slot_idx] = temporal_extra_data->lt_opr;
		stats->stavalues[*slot_idx] = bound_hist_values;
		stats->numvalues[*slot_idx] = num_hist;
		stats->statypid[*slot_idx] = range_typeentry->type_id;
		stats->statyplen[*slot_idx] = range_typeentry->typlen;
		stats->statypbyval[*slot_idx] =range_typeentry->typbyval;
		stats->statypalign[*slot_idx] = range_typeentry->typalign;
		(*slot_idx)++;

		/* Generate a length histogram slot entry */

		/*
		* Ascending sort of range lengths for further filling of histogram
		*/
		qsort(lengths, (size_t) non_null_cnt, sizeof(float8), float8_qsort_cmp);

		num_hist = non_null_cnt;
		if (num_hist > num_bins)
			num_hist = num_bins + 1;

		length_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

		/*
		* The object of this loop is to copy the first and last lengths[]
		* entries along with evenly-spaced values in between. So the i'th
		* value is lengths[(i * (nvals - 1)) / (num_hist - 1)]. But
		* computing that subscript directly risks integer overflow when
		* the stats target is more than a couple thousand.  Instead we
		* add (nvals - 1) / (num_hist - 1) to pos at each step, tracking
		* the integral and fractional parts of the sum separately.
		*/
		delta = (non_null_cnt - 1) / (num_hist - 1);
		deltafrac = (non_null_cnt - 1) % (num_hist - 1);
		pos = posfrac = 0;

		for (int i = 0; i < num_hist; i++)
		{
			length_hist_values[i] = Float8GetDatum(lengths[pos]);
			pos += delta;
			posfrac += deltafrac;
			if (posfrac >= (num_hist - 1))
			{
				/* fractional part exceeds 1, carry to integer part */
				pos++;
				posfrac -= (num_hist - 1);
			}
		}
	}
	else
	{
		/*
		* Even when we don't create the histogram, store an empty array
		* to mean "no histogram". We can't just leave stavalues NULL,
		* because get_attstatsslot() errors if you ask for stavalues, and
		* it's NULL. We'll still store the empty fraction in stanumbers.
		*/
		length_hist_values = palloc(0);
		num_hist = 0;
	}
	stats->stakind[*slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
	stats->staop[*slot_idx] = Float8LessOperator;
	stats->stavalues[*slot_idx] = length_hist_values;
	stats->numvalues[*slot_idx] = num_hist;
	stats->statypid[*slot_idx] = FLOAT8OID;
	stats->statyplen[*slot_idx] = sizeof(float8);
	stats->statypbyval[*slot_idx] = true;
	stats->statypalign[*slot_idx] = 'd';

	/* Store 0 as value for the fraction of empty ranges */
	emptyfrac = (float4 *) palloc(sizeof(float4));
	*emptyfrac = 0.0;
	stats->stanumbers[*slot_idx] = emptyfrac;
	stats->numnumbers[*slot_idx] = 1;
	(*slot_idx)++;

	MemoryContextSwitchTo(old_cxt);
}

/* 
 * Compute statistics for all durations distinct from TemporalInst.
 * Function derived from compute_range_stats of file rangetypes_typanalyze.c 
 */

static void
temps_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int samplerows, bool valuestats)
{
	int null_cnt = 0,
			non_null_cnt = 0,
			slot_idx = 0;
	float8 *value_lengths, 
		   *time_lengths;
	RangeBound *value_lowers,
		   *value_uppers;
	PeriodBound *time_lowers,
		   *time_uppers;
	double total_width = 0;
	Oid 	rangetypid = 0; /* make compiler quiet */
	TypeCacheEntry *typcache;

	temporal_extra_data = (TemporalAnalyzeExtraData *)stats->extra_data;

	if (valuestats)
	{
		/* Ensure function is called for temporal numbers */
		ensure_numeric_base_type(temporal_extra_data->value_type_id);
		if (temporal_extra_data->value_type_id == INT4OID)
			rangetypid = type_oid(T_INTRANGE);
		else if (temporal_extra_data->value_type_id == FLOAT8OID)
			rangetypid = type_oid(T_FLOATRANGE);
		typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);
		value_lowers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
		value_uppers = (RangeBound *) palloc(sizeof(RangeBound) * samplerows);
		value_lengths = (float8 *) palloc(sizeof(float8) * samplerows);
	}
	time_lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	time_uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	time_lengths = (float8 *) palloc(sizeof(float8) * samplerows);

	/* Loop over the temporal values. */
	for (int i = 0; i < samplerows; i++)
	{
		Datum value;
		bool isnull, isempty;
		RangeBound range_lower,
				range_upper;
		Period period;
		PeriodBound period_lower,
				period_upper;
		Temporal *temp;
	
		/* Give backend a chance of interrupting us */
		vacuum_delay_point();

		value = fetchfunc(stats, i, &isnull);
		if (isnull)
		{
			/* Temporal is null, just count that */
			null_cnt++;
			continue;
		}

		total_width += VARSIZE(value);

		/* Get Temporal value */
		temp = DatumGetTemporal(value);

		/* Remember bounds and length for further usage in histograms */
		if (valuestats)
		{
            RangeType *range = tnumber_value_range_internal(temp);
			range_deserialize(typcache, range, &range_lower, &range_upper, &isempty);
			value_lowers[non_null_cnt] = range_lower;
			value_uppers[non_null_cnt] = range_upper;

			if (temporal_extra_data->value_type_id == INT4OID)
				value_lengths[non_null_cnt] = (float8) (DatumGetInt32(range_upper.val) -
					DatumGetInt32(range_lower.val));
			else if (temporal_extra_data->value_type_id == FLOAT8OID)
				value_lengths[non_null_cnt] = DatumGetFloat8(range_upper.val) -
					DatumGetFloat8(range_lower.val);
		}
		temporal_period(&period, temp);
		period_deserialize(&period, &period_lower, &period_upper);
		time_lowers[non_null_cnt] = period_lower;
		time_uppers[non_null_cnt] = period_upper;
		time_lengths[non_null_cnt] = period_to_secs(period_upper.t,
			period_lower.t);

		non_null_cnt++;
	}

	/* We can only compute real stats if we found some non-null values. */
	if (non_null_cnt > 0)
	{
		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (float4) null_cnt / (float4) samplerows;
		stats->stawidth = (int) (total_width / non_null_cnt);

		/* Estimate that non-null values are unique */
		stats->stadistinct = (float4) (-1.0 * (1.0 - stats->stanullfrac));

		if (valuestats)
		{
			range_compute_stats(stats, non_null_cnt, &slot_idx, value_lowers, 
				value_uppers, value_lengths, typcache, rangetypid);
		}

		period_compute_stats1(stats, non_null_cnt, &slot_idx,
			time_lowers, time_uppers, time_lengths);
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;		/* "unknown" */
		stats->stadistinct = 0.0;	/* "unknown" */
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */

	if (valuestats)
	{
		pfree(value_lowers); pfree(value_uppers); pfree(value_lengths);
	}
	pfree(time_lowers); pfree(time_uppers); pfree(time_lengths);
}

/*****************************************************************************
 * Statistics functions for temporal types
 *****************************************************************************/

void
temporalinst_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int samplerows, double totalrows)
{
	return tempinst_compute_stats(stats, fetchfunc, samplerows, totalrows, false);
}

void
temporals_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int samplerows, double totalrows)
{
	return temps_compute_stats(stats, fetchfunc, samplerows, false);
}

/*****************************************************************************
 * Statistics functions for temporal types
 *****************************************************************************/

void
tnumberinst_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int samplerows, double totalrows)
{
	return tempinst_compute_stats(stats, fetchfunc, samplerows, totalrows, true);
}

void
tnumbers_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
	int samplerows, double totalrows)
{
	return temps_compute_stats(stats, fetchfunc, samplerows, true);
}

/*****************************************************************************
 * Statistics information for temporal types
 *****************************************************************************/

void
temporal_extra_info(VacAttrStats *stats)
{
	TypeCacheEntry *typentry;
	TemporalAnalyzeExtraData *extra_data;
	Form_pg_attribute attr = stats->attr;

	/*
	 * Check attribute data type is a temporal type.
	 */
	if (! temporal_type_oid(stats->attrtypid))
		elog(ERROR, "temporal_analyze was invoked with invalid type %u",
			 stats->attrtypid);

	/* Store our findings for use by stats functions */
	extra_data = (TemporalAnalyzeExtraData *) palloc(sizeof(TemporalAnalyzeExtraData));

	/*
	 * Gather information about the temporal type and its value and time types.
	 */

	/* Information about the temporal type */
	typentry = lookup_type_cache(stats->attrtypid,
		TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
		TYPECACHE_HASH_PROC_FINFO);
	extra_data->type_id = typentry->type_id;
	extra_data->eq_opr = typentry->eq_opr;
	extra_data->lt_opr = typentry->lt_opr;
	extra_data->typbyval = typentry->typbyval;
	extra_data->typlen = typentry->typlen;
	extra_data->typalign = typentry->typalign;
	extra_data->cmp = &typentry->cmp_proc_finfo;
	extra_data->hash = &typentry->hash_proc_finfo;

	/* Information about the value type */
	typentry = lookup_type_cache(base_oid_from_temporal(stats->attrtypid),
		TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
		TYPECACHE_HASH_PROC_FINFO);
	extra_data->value_type_id = typentry->type_id;
	extra_data->value_eq_opr = typentry->eq_opr;
	extra_data->value_lt_opr = typentry->lt_opr;
	extra_data->value_typbyval = typentry->typbyval;
	extra_data->value_typlen = typentry->typlen;
	extra_data->value_typalign = typentry->typalign;
	extra_data->value_cmp = &typentry->cmp_proc_finfo;
	extra_data->value_hash = &typentry->hash_proc_finfo;

	/* Information about the time type, that is TimestampTz */
	if (stats->attrtypmod == TEMPORALINST)
	{
		typentry = lookup_type_cache(TIMESTAMPTZOID,
			TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
			TYPECACHE_HASH_PROC_FINFO);
		extra_data->time_type_id = TIMESTAMPTZOID;
		extra_data->time_eq_opr = typentry->eq_opr;
		extra_data->time_lt_opr = typentry->lt_opr;
		extra_data->time_typbyval = true;
		extra_data->time_typlen = sizeof(TimestampTz);
		extra_data->time_typalign = 'd';
		extra_data->time_cmp = &typentry->cmp_proc_finfo;
		extra_data->time_hash = &typentry->hash_proc_finfo;
	}
	else
	{
		Oid pertypoid = type_oid(T_PERIOD);
		typentry = lookup_type_cache(pertypoid,
			TYPECACHE_EQ_OPR | TYPECACHE_LT_OPR | TYPECACHE_CMP_PROC_FINFO |
			TYPECACHE_HASH_PROC_FINFO);
		extra_data->time_type_id = pertypoid;
		extra_data->time_eq_opr = typentry->eq_opr;
		extra_data->time_lt_opr = typentry->lt_opr;
		extra_data->time_typbyval = false;
		extra_data->time_typlen = sizeof(Period);
		extra_data->time_typalign = 'd';
		extra_data->time_cmp = &typentry->cmp_proc_finfo;
		extra_data->time_hash = &typentry->hash_proc_finfo;
	}

	extra_data->std_extra_data = stats->extra_data;
	stats->extra_data = extra_data;

	stats->minrows = 300 * attr->attstattarget;
}

/*****************************************************************************/

Datum
generic_analyze(FunctionCallInfo fcinfo, 
	void (*funcinst)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double),
	void (*functemp)(VacAttrStats *, AnalyzeAttrFetchFunc, int, double))
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int16 duration;

	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	/* 
	 * Ensure duration is valid and collect extra information about the 
	 * temporal type and its base and time types.
	 */
	duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	ensure_valid_duration_all(duration);
	if (duration != TEMPORALINST)
		temporal_extra_info(stats);

	/* Set the callback function to compute statistics. */
	if (duration == TEMPORALINST)
	{
		assert(funcinst != NULL);
		stats->compute_stats = funcinst;
	}
	else
	{
		assert(functemp != NULL);
		stats->compute_stats = functemp;
	}
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(temporal_analyze);

PGDLLEXPORT Datum
temporal_analyze(PG_FUNCTION_ARGS)
{
	return generic_analyze(fcinfo, &temporalinst_compute_stats, temporals_compute_stats);
}

PG_FUNCTION_INFO_V1(tnumber_analyze);

PGDLLEXPORT Datum
tnumber_analyze(PG_FUNCTION_ARGS)
{
	return generic_analyze(fcinfo, &tnumberinst_compute_stats, tnumbers_compute_stats);
}

/*****************************************************************************/
