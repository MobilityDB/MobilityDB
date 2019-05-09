/*****************************************************************************
 *
 * TimeTypanalyze.c
 *	  Functions for gathering statistics from time type columns
 *
 * These functions are based on those of the file rangetypes_typanalyze.c.
 * For a period type column, histograms of lower and upper bounds, and
 * the fraction of NULL periods are collected.
 *
 * Both histograms have the same length, and they are combined into a
 * single array of periods. This has the same shape as the histogram that
 * std_typanalyze would collect, but the values are different. Each period
 * in the array is a valid period, even though the lower and upper bounds
 * come from different tuples. In theory, the standard scalar selectivity
 * functions could be used with the combined histogram.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

static int	float8_qsort_cmp(const void *a1, const void *a2);
static int	period_bound_qsort_cmp(const void *a1, const void *a2);
static void compute_time_stats(CachedType type, VacAttrStats *stats, 
	AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);
static void compute_period_stats(VacAttrStats *stats,
	AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);
static void compute_timestampset_stats(VacAttrStats *stats, 
	AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);
static void compute_periodset_stats(VacAttrStats *stats, 
	AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows);

/*****************************************************************************/

/*
 * Comparison function for sorting float8s, used for period lengths.
 */
static int
float8_qsort_cmp(const void *a1, const void *a2)
{
	const float8 *f1 = (const float8 *) a1;
	const float8 *f2 = (const float8 *) a2;

	if (*f1 < *f2)
		return -1;
	else if (*f1 == *f2)
		return 0;
	else
		return 1;
}

/*
 * Comparison function for sorting PeriodBounds.
 */
static int
period_bound_qsort_cmp(const void *a1, const void *a2)
{
	PeriodBound *b1 = (PeriodBound *) a1;
	PeriodBound *b2 = (PeriodBound *) a2;
	return period_cmp_bounds(b1->val, b2->val, b1->lower, b2->lower, 
		b1->inclusive, b2->inclusive);
}

static void
compute_time_stats(CachedType time_type, VacAttrStats *stats, 
	AnalyzeAttrFetchFunc fetchfunc, int samplerows, double totalrows)
{
	int			null_cnt = 0;
	int			non_null_cnt = 0;
	int			timetype_no;
	int			slot_idx;
	int			num_bins = stats->attr->attstattarget;
	int			num_hist;
	float8	   *lengths;
	PeriodBound *lowers,
			   *uppers;
	double		total_width = 0;

	/* Allocate memory to hold period bounds and lengths of the sample periods. */
	lowers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	uppers = (PeriodBound *) palloc(sizeof(PeriodBound) * samplerows);
	lengths = (float8 *) palloc(sizeof(float8) * samplerows);

	/* Loop over the sample timestampsets. */
	for (timetype_no = 0; timetype_no < samplerows; timetype_no++)
	{
		Datum		value;
		bool		isnull;
		Period	   *period;
		PeriodBound	lower,
					upper;
		float8		length;

		vacuum_delay_point();

		value = fetchfunc(stats, timetype_no, &isnull);
		if (isnull)
		{
			/* timestampset is null, just count that */
			null_cnt++;
			continue;
		}

		/* Get the period or the bbox period and deserialize it for further analysis. */
		if (time_type == T_PERIOD)
		{
			period = DatumGetPeriod(value);
			/* Adjust the size */
			total_width += 24;
		}
		else if (time_type == T_TIMESTAMPSET)
		{
			TimestampSet *ts= DatumGetTimestampSet(value);
			period = timestampset_bbox(ts);
			/* Adjust the size */
			total_width += VARSIZE(ts);
		}
		else if (time_type == T_PERIODSET)
		{
			PeriodSet *ps= DatumGetPeriodSet(value);
			period = periodset_bbox(ps);
			/* Adjust the size */
			total_width += VARSIZE(ps);
		}
		else
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
				errmsg("Operation not supported")));
			
		period_deserialize(period, &lower, &upper);

		/* Remember bounds and length for further usage in histograms */
		lowers[non_null_cnt] = lower;
		uppers[non_null_cnt] = upper;
		/* Use subdiff function between upper and lower bound values. */
		length = period_duration_secs(upper.val, lower.val);
		lengths[non_null_cnt] = length;

		non_null_cnt++;
	}

	slot_idx = 0;

	/* We can only compute real stats if we found some non-null values. */
	if (non_null_cnt > 0)
	{
		Datum	   *bound_hist_values;
		Datum	   *length_hist_values;
		int			pos,
					posfrac,
					delta,
					deltafrac,
					i;
		MemoryContext old_cxt;

		stats->stats_valid = true;
		/* Do the simple null-frac and width stats */
		stats->stanullfrac = (double) null_cnt / (double) samplerows;
		stats->stawidth = total_width / (double) non_null_cnt;

		/* Estimate that non-null values are unique */
		stats->stadistinct = -1.0 * (1.0 - stats->stanullfrac);

		/* Must copy the target values into anl_context */
		old_cxt = MemoryContextSwitchTo(stats->anl_context);

		/*
		 * Generate a bounds histogram slot entry if there are at least two
		 * values.
		 */
		if (non_null_cnt >= 2)
		{
			/* Sort bound values */
			qsort(lowers, non_null_cnt, sizeof(PeriodBound), period_bound_qsort_cmp);
			qsort(uppers, non_null_cnt, sizeof(PeriodBound), period_bound_qsort_cmp);

			num_hist = non_null_cnt;
			if (num_hist > num_bins)
				num_hist = num_bins + 1;

			bound_hist_values = (Datum *) palloc(num_hist * sizeof(Datum));

			/*
			 * The object of this loop is to construct periods from first and
			 * last entries in lowers[] and uppers[] along with evenly-spaced
			 * values in between. So the i'th value is a period of lowers[(i *
			 * (nvals - 1)) / (num_hist - 1)] and uppers[(i * (nvals - 1)) /
			 * (num_hist - 1)]. But computing that subscript directly risks
			 * integer overflow when the stats target is more than a couple
			 * thousand.  Instead we add (nvals - 1) / (num_hist - 1) to pos
			 * at each step, tracking the integral and fractional parts of the
			 * sum separately.
			 */
			delta = (non_null_cnt - 1) / (num_hist - 1);
			deltafrac = (non_null_cnt - 1) % (num_hist - 1);
			pos = posfrac = 0;

			for (i = 0; i < num_hist; i++)
			{
				bound_hist_values[i] =
					PointerGetDatum(period_make(lowers[pos].val, uppers[pos].val,
					lowers[pos].inclusive, uppers[pos].inclusive));
				pos += delta;
				posfrac += deltafrac;
				if (posfrac >= (num_hist - 1))
				{
					/* fractional part exceeds 1, carry to integer part */
					pos++;
					posfrac -= (num_hist - 1);
				}
			}

			stats->stakind[slot_idx] = STATISTIC_KIND_BOUNDS_HISTOGRAM;
			stats->stavalues[slot_idx] = bound_hist_values;
			stats->numvalues[slot_idx] = num_hist;
			stats->statypid[slot_idx] = type_oid(T_PERIOD);
			stats->statyplen[slot_idx] = sizeof(Period);
			stats->statypbyval[slot_idx] = false;
			stats->statypalign[slot_idx] = 'd';
			slot_idx++;
		}

		/*
		 * Generate a length histogram slot entry if there are at least two
		 * values.
		 */
		if (non_null_cnt >= 2)
		{
			/*
			 * Ascending sort of period lengths for further filling of
			 * histogram
			 */
			qsort(lengths, non_null_cnt, sizeof(float8), float8_qsort_cmp);

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

			for (i = 0; i < num_hist; i++)
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
			 * it's NULL.
			 */
			length_hist_values = palloc(0);
			num_hist = 0;
		}
		stats->stakind[slot_idx] = STATISTIC_KIND_RANGE_LENGTH_HISTOGRAM;
		stats->staop[slot_idx] = Float8LessOperator;
		stats->stavalues[slot_idx] = length_hist_values;
		stats->numvalues[slot_idx] = num_hist;
		stats->statypid[slot_idx] = FLOAT8OID;
		stats->statyplen[slot_idx] = sizeof(float8);
#ifdef USE_FLOAT8_BYVAL
		stats->statypbyval[slot_idx] = true;
#else
		stats->statypbyval[slot_idx] = false;
#endif
		stats->statypalign[slot_idx] = 'd';

		slot_idx++;

		MemoryContextSwitchTo(old_cxt);
	}
	else if (null_cnt > 0)
	{
		/* We found only nulls; assume the column is entirely null */
		stats->stats_valid = true;
		stats->stanullfrac = 1.0;
		stats->stawidth = 0;	/* "unknown" */
		stats->stadistinct = 0.0;		/* "unknown" */
	}

	/*
	 * We don't need to bother cleaning up any of our temporary palloc's. The
	 * hashtable should also go away, as it used a child memory context.
	 */
}

/*****************************************************************************/

/*
 * period_typanalyze -- typanalyze function for period columns
 */

static void
compute_period_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					int samplerows, double totalrows)
{
	compute_time_stats(T_PERIOD, stats, fetchfunc, samplerows, totalrows);
}

PG_FUNCTION_INFO_V1(period_typanalyze);

PGDLLEXPORT Datum
period_typanalyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Form_pg_attribute attr = stats->attr;

	if (attr->attstattarget < 0)
		attr->attstattarget = default_statistics_target;

	stats->compute_stats = compute_period_stats;
	/* same as in std_typanalyze */
	stats->minrows = 300 * attr->attstattarget;

	PG_RETURN_BOOL(true);
}

/*
 * timestampset_typanalyze -- typanalyze function for timestampset columns
 */

static void
compute_timestampset_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					int samplerows, double totalrows)
{
	compute_time_stats(T_TIMESTAMPSET, stats, fetchfunc, samplerows, totalrows);
}

PG_FUNCTION_INFO_V1(timestampset_typanalyze);

PGDLLEXPORT Datum
timestampset_typanalyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Form_pg_attribute attr = stats->attr;

	if (attr->attstattarget < 0)
		attr->attstattarget = default_statistics_target;

	stats->compute_stats = compute_timestampset_stats;
	/* same as in std_typanalyze */
	stats->minrows = 300 * attr->attstattarget;

	PG_RETURN_BOOL(true);
}

/*
 * timestampset_typanalyze -- typanalyze function for timestampset columns
 */

static void
compute_periodset_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					int samplerows, double totalrows)
{
	compute_time_stats(T_PERIODSET, stats, fetchfunc, samplerows, totalrows);
}

PG_FUNCTION_INFO_V1(periodset_typanalyze);

PGDLLEXPORT Datum
periodset_typanalyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Form_pg_attribute attr = stats->attr;

	if (attr->attstattarget < 0)
		attr->attstattarget = default_statistics_target;

	stats->compute_stats = compute_periodset_stats;
	/* same as in std_typanalyze */
	stats->minrows = 300 * attr->attstattarget;

	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
