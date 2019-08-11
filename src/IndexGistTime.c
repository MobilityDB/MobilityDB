/*****************************************************************************
 *
 * IndexGistTime.c
 *	R-tree GiST index for time types.
 *
 * These functions are based on those in the file rangetypes_gist.c.
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "IndexGistTime.h"

#include <access/gist.h>
#include <utils/timestamp.h>

#include "TimeTypes.h"
#include "TimestampSet.h"
#include "Period.h"
#include "PeriodSet.h"
#include "TimeOps.h"
#include "Temporal.h"
#include "OidCache.h"

/*****************************************************************************/

/*
 * Minimum accepted ratio of split for items of the same class.  If the items
 * are of different classes, we will separate along those lines regardless of
 * the ratio.
 */
#define LIMIT_RATIO  0.3

/* place on left or right side of split? */
typedef enum
{
	SPLIT_LEFT = 0,				/* makes initialization to SPLIT_LEFT easier */
	SPLIT_RIGHT
} SplitLR;

/*
 * Context for gist_period_consider_split.
 */
typedef struct
{
	int			entries_count;	/* total number of entries being split */

	/* Information about currently selected split follows */

	bool		first;			/* true if no split was selected yet */

	PeriodBound left_upper;	/* upper bound of left interval */
	PeriodBound right_lower;	/* lower bound of right interval */

	float4		ratio;			/* split ratio */
	float4		overlap;		/* overlap between left and right predicate */
	int			common_left;	/* # common entries destined for each side */
	int			common_right;
} ConsiderSplitContext;

/*
 * Represents information about an entry that can be placed in either group
 * without affecting overlap over selected axis ("common entry").
 */
typedef struct
{
	/* Index of entry in the initial array */
	int			index;
	/* Delta between closeness of period to each of the two groups */
	double		delta;
} CommonEntry;

/* Helper macros to place an entry in the left or right group during split */
/* Note direct access to variables v, left_period, right_period */
#define PLACE_LEFT(period, off)					\
	do {										\
		if (v->spl_nleft > 0)					\
			left_period = period_super_union(left_period, period); \
		else									\
			left_period = (period);				\
		v->spl_left[v->spl_nleft++] = (off);	\
	} while (0)

#define PLACE_RIGHT(period, off)				\
	do {										\
		if (v->spl_nright > 0)					\
			right_period = period_super_union(right_period, period); \
		else									\
			right_period = (period);			\
		v->spl_right[v->spl_nright++] = (off);	\
	} while (0)

/* Copy a Period datum (hardwires typbyval and typlen for periods...) */
#define periodCopy(r) \
	((Period *) DatumGetPointer(datumCopy(PointerGetDatum(r), \
											 false, 24)))

/*****************************************************************************
 * STATIC FUNCTIONS
 *****************************************************************************/

/*
 * Return the smallest period that contains p1 and p2
 *
 * This differs from regular period_union in a critical ways:
 * It won't throw an error for non-adjacent p1 and p2, but just absorb
 * the intervening values into the result period.
 */
static Period *
period_super_union(Period *p1, Period *p2)
{
	TimestampTz result_lower;
	TimestampTz result_upper;
	bool result_lower_inc;
	bool result_upper_inc;

	if (period_cmp_bounds(p1->lower, p2->lower, true, true, 
		p1->lower_inc, p2->lower_inc) <= 0)
	{
		result_lower = p1->lower;
		result_lower_inc = p1->lower_inc;
	}
	else
	{
		result_lower = p2->lower;
		result_lower_inc = p2->lower_inc;
	}

	if (period_cmp_bounds(p1->upper, p2->upper, false, false, 
		p1->upper_inc, p2->upper_inc) >= 0)
	{
		result_upper = p1->upper;
		result_upper_inc = p1->upper_inc;
	}
	else
	{
		result_upper = p2->upper;
		result_upper_inc = p2->upper_inc;
	}

	/* optimization to avoid constructing a new range */
	if (result_lower == p1->lower && result_upper == p1->upper)
		return p1;
	if (result_lower == p2->lower && result_upper == p2->upper)
		return p2;
		
	return period_make(result_lower, result_upper, 
		result_lower_inc, result_upper_inc);
}
 
/*
 * Trivial split: half of entries will be placed on one page
 * and the other half on the other page.
 */
static void
gist_period_fallafter_split(GistEntryVector *entryvec,
						  GIST_SPLITVEC *v)
{
	Period  *left_period = NULL;
	Period  *right_period = NULL;
	OffsetNumber i,
				maxoff,
				split_idx;

	maxoff = entryvec->n - 1;
	/* Split entries before this to left page, after to right: */
	split_idx = (maxoff - FirstOffsetNumber) / 2 + FirstOffsetNumber;

	v->spl_nleft = 0;
	v->spl_nright = 0;
	for (i = FirstOffsetNumber; i <= maxoff; i++)
	{
		Period  *period = DatumGetPeriod(entryvec->vector[i].key);

		if (i < split_idx)
			PLACE_LEFT(period, i);
		else
			PLACE_RIGHT(period, i);
	}

	v->spl_ldatum = PeriodGetDatum(left_period);
	v->spl_rdatum = PeriodGetDatum(right_period);
}

/*
 * Consider replacement of currently selected split with a better one
 * during gist_period_double_sorting_split.
 */
static void
gist_period_consider_split(ConsiderSplitContext *context,
						  PeriodBound *right_lower, int min_left_count,
						  PeriodBound *left_upper, int max_left_count)
{
	int			left_count,
				right_count;
	float4		ratio,
				overlap;

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
	 * entries count.  This is necessarily 0.5 or less; if it's less than
	 * LIMIT_RATIO then we will never accept the new split.
	 */
	ratio = ((float4) Min(left_count, right_count)) /
		((float4) context->entries_count);

	if (ratio > LIMIT_RATIO)
	{
		bool		selectthis = false;

		/*
		 * The ratio is acceptable, so compare current split with previously
		 * selected one. We search for minimal overlap (allowing negative
		 * values) and minimal ratio secondarily.  The subtype_diff is
		 * used for overlap measure. 
		 */
		overlap = period_duration_secs(left_upper->val, right_lower->val);

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
}

/*
 * Compare CommonEntrys by their deltas.
 */
static int
common_entry_cmp(const void *i1, const void *i2)
{
	double		delta1 = ((CommonEntry *) i1)->delta;
	double		delta2 = ((CommonEntry *) i2)->delta;

	if (delta1 < delta2)
		return -1;
	else if (delta1 > delta2)
		return 1;
	else
		return 0;
}

/*
 * Double sorting split algorithm.
 *
 * The algorithm considers dividing periods into two groups. The first (left)
 * group contains general left bound. The second (right) group contains
 * general right bound. The challenge is to find upper bound of left group
 * and lower bound of right group so that overlap of groups is minimal and
 * ratio of distribution is acceptable. Algorithm finds for each lower bound of
 * right group minimal upper bound of left group, and for each upper bound of
 * left group maximal lower bound of right group. For each found pair
 * gist_period_consider_split considers replacement of currently selected
 * split with the new one.
 *
 * After that, all the entries are divided into three groups:
 * 1) Entries which should be placed to the left group
 * 2) Entries which should be placed to the right group
 * 3) "Common entries" which can be placed to either group without affecting
 *	  amount of overlap.
 *
 * The common periods are distributed by difference of distance from lower
 * bound of common period to lower bound of right group and distance from upper
 * bound of common period to upper bound of left group.
 *
 * For details see:
 * "A new double sorting-based node splitting algorithm for R-tree",
 * A. Korotkov
 * http://syrcose.ispras.ru/2011/files/SYRCoSE2011_Proceedings.pdf#page=36
 */
static void
gist_period_double_sorting_split(GistEntryVector *entryvec,
								GIST_SPLITVEC *v)
{
	ConsiderSplitContext context;
	OffsetNumber i,
				maxoff;
	Period *left_period = NULL,
			   *right_period = NULL;
	int			common_entries_count;
	Period **by_lower,
			   **by_upper;
	CommonEntry *common_entries;
	int			nentries,
				i1,
				i2;
	PeriodBound right_lower,
				left_upper;

	memset(&context, 0, sizeof(ConsiderSplitContext));

	maxoff = entryvec->n - 1;
	nentries = context.entries_count = maxoff - FirstOffsetNumber + 1;
	context.first = true;

	/*
	 * Make two arrays of periods: one sorted by lower bound and 
	 * another sorted by upper bound.
	 */
	
	/* Allocate arrays for periods */
	by_lower = (Period **) palloc(nentries * sizeof(Period *));
	by_upper = (Period **) palloc(nentries * sizeof(Period *));
	/* Fill arrays of periods */
	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		Period  *period = DatumGetPeriod(entryvec->vector[i].key);
		by_lower[i - FirstOffsetNumber] = period;
	}
	memcpy(by_upper, by_lower, nentries * sizeof(Period *));
	/* Sort the arrays */
	qsort(by_lower, nentries, sizeof(Period *), 
		(qsort_comparator) period_cmp_lower);
	qsort(by_upper, nentries, sizeof(Period *), 
		(qsort_comparator) period_cmp_upper);

	/*----------
	 * The goal is to form a left and right period, so that every entry
	 * period is contained by either left or right interval (or both).
	 *
	 * For example, with the periods (0,1), (1,3), (2,3), (2,4):
	 *
	 * 0 1 2 3 4
	 * +-+
	 *	 +---+
	 *	   +-+
	 *	   +---+
	 *
	 * The left and right periods are of the form (0,a) and (b,4).
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
	period_deserialize(by_lower[i1], &right_lower, NULL);
	period_deserialize(by_upper[i2], &left_upper, NULL);
	while (true)
	{
		/*
		 * Find next lower bound of right group.
		 */
		while (i1 < nentries &&
				period_cmp_bounds(right_lower.val, by_lower[i1]->lower, 
				right_lower.lower, true,
				right_lower.inclusive, by_lower[i1]->lower_inc) == 0)
		{
			if (period_cmp_bounds(by_lower[i1]->upper, left_upper.val, 
				false, left_upper.lower,
				by_lower[i1]->upper_inc, left_upper.inclusive) > 0)
			{
				period_deserialize(by_lower[i1], NULL, &left_upper);
			}
			i1++;
		}
		if (i1 >= nentries)
			break;
		period_deserialize(by_lower[i1], &right_lower, NULL);

		/*
		 * Find count of periods which anyway should be placed to the left
		 * group.
		 */
		while (i2 < nentries && period_cmp_bounds(by_upper[i2]->upper, 
				left_upper.val, false, left_upper.lower,
				by_upper[i2]->upper_inc, left_upper.inclusive) <= 0)
			i2++;

		/*
		 * Consider found split to see if it's better than what we had.
		 */
		gist_period_consider_split(&context, &right_lower, i1, &left_upper, i2);
	}

	/*
	 * Iterate over upper bound of left group finding greatest possible lower
	 * bound of right group.
	 */
	i1 = nentries - 1;
	i2 = nentries - 1;
	period_deserialize(by_lower[i1], NULL, &right_lower);
	period_deserialize(by_upper[i2], NULL, &left_upper);
	while (true)
	{
		/*
		 * Find next upper bound of left group.
		 */
		while (i2 >= 0 && period_cmp_bounds(left_upper.val, 
				by_upper[i2]->upper, left_upper.lower, false,
					left_upper.inclusive, by_upper[i2]->upper_inc) == 0)
		{
			if (period_cmp_bounds(by_upper[i2]->lower, right_lower.val, 
				true, right_lower.lower, by_upper[i2]->lower_inc, 
				right_lower.inclusive) < 0)
			{
				period_deserialize(by_upper[i2], &right_lower, NULL);
			}
			i2--;
		}
		if (i2 < 0)
			break;
		period_deserialize(by_upper[i2], NULL, &left_upper);

		/*
		 * Find count of intervals which anyway should be placed to the right
		 * group.
		 */
		while (i1 >= 0 && period_cmp_bounds(by_lower[i1]->lower, 
				right_lower.val, true, right_lower.lower,
				by_lower[i1]->lower_inc, right_lower.inclusive) >= 0)
			i1--;

		/*
		 * Consider found split to see if it's better than what we had.
		 */
		gist_period_consider_split(&context, &right_lower, i1 + 1,
								  &left_upper, i2 + 1);
	}

	/*
	 * If we failed to find any acceptable splits, use trivial split.
	 */
	if (context.first)
	{
		gist_period_fallafter_split(entryvec, v);
		return;
	}

	/*
	 * Ok, we have now selected bounds of the groups. Now we have to
	 * distribute entries themselves. At first we distribute entries which can
	 * be placed unambiguously and collect "common entries" to array.
	 */

	/* Allocate vectors for results */
	v->spl_left = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
	v->spl_right = (OffsetNumber *) palloc(nentries * sizeof(OffsetNumber));
	v->spl_nleft = 0;
	v->spl_nright = 0;

	/*
	 * Allocate an array for "common entries" - entries which can be placed to
	 * either group without affecting overlap along selected axis.
	 */
	common_entries_count = 0;
	common_entries = (CommonEntry *) palloc(nentries * sizeof(CommonEntry));

	/*
	 * Distribute entries which can be distributed unambiguously, and collect
	 * common entries.
	 */
	for (i = FirstOffsetNumber; i <= maxoff; i = OffsetNumberNext(i))
	{
		/*
		 * Get upper and lower bounds along selected axis.
		 */
		Period *period = DatumGetPeriod(entryvec->vector[i].key);

		if (period_cmp_bounds(period->upper, context.left_upper.val, false, 
			context.left_upper.lower, period->upper_inc, 
			context.left_upper.inclusive) <= 0)
		{
			/* Fits in the left group */
			if (period_cmp_bounds(period->lower, context.right_lower.val, 
				true, context.right_lower.lower, period->lower_inc, 
				context.right_lower.inclusive) >= 0)
			{
				/* Fits also in the right group, so "common entry" */
				common_entries[common_entries_count].index = i;
				/*
				 * delta = (lower - context.right_lower) -
				 * (context.left_upper - upper)
				 */
				common_entries[common_entries_count].delta =
					period_duration_secs(period->lower, context.right_lower.val) -
					period_duration_secs(context.left_upper.val, period->upper);
				common_entries_count++;
			}
			else
			{
				/* Doesn't fit to the right group, so join to the left group */
				PLACE_LEFT(period, i);
			}
		}
		else
		{
			/*
			 * Each entry should fit on either left or right group. Since this
			 * entry didn't fit in the left group, it better fit in the right
			 * group.
			 */
			Assert(period_cmp_bounds(period->lower, context.right_lower->val, 
				true, context.right_lower->lower, period->lower_inc, 
				context.right_lower->inclusive) >= 0);
			PLACE_RIGHT(period, i);
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
		qsort(common_entries, common_entries_count, sizeof(CommonEntry),
			  common_entry_cmp);

		/*
		 * Distribute "common entries" between groups according to sorting.
		 */
		for (i = 0; i < common_entries_count; i++)
		{
			int			idx = common_entries[i].index;

			Period *period = DatumGetPeriod(entryvec->vector[idx].key);

			/*
			 * Check if we have to place this entry in either group to achieve
			 * LIMIT_RATIO.
			 */
			if (i < context.common_left)
				PLACE_LEFT(period, idx);
			else
				PLACE_RIGHT(period, idx);
		}
	}

	v->spl_ldatum = PointerGetDatum(left_period);
	v->spl_rdatum = PointerGetDatum(right_period);
}

/*****************************************************************************
 * Consistent methods for time types
 *****************************************************************************/

/*
 * Leaf-level consistency
 */
bool
index_leaf_consistent_time(Period *key, Period *query, StrategyNumber strategy)
{
	switch (strategy)
	{
		case RTOverlapStrategyNumber:
			return overlaps_period_period_internal(key, query);
		case RTContainsStrategyNumber:
			return contains_period_period_internal(key, query);
		case RTContainedByStrategyNumber:
			return contains_period_period_internal(query, key);
		case RTSameStrategyNumber:
			return period_eq_internal(key, query);
		case RTBeforeStrategyNumber:
			return before_period_period_internal(key, query);
		case RTOverBeforeStrategyNumber:
			return overbefore_period_period_internal(key, query);
		case RTAfterStrategyNumber:
			return after_period_period_internal(key, query);
		case RTOverAfterStrategyNumber:
			return overafter_period_period_internal(key, query);
		default:
			elog(ERROR, "unrecognized period strategy: %d", strategy);
			return false;		/* keep compiler quiet */
	}
}

/*
 * Internal-page consistency
 */
bool
index_internal_consistent_time(Period *key, Period *query, StrategyNumber strategy)
{
	switch (strategy)
	{
		case RTOverlapStrategyNumber:
		case RTContainedByStrategyNumber:
			return overlaps_period_period_internal(key, query);
		case RTContainsStrategyNumber:
		case RTSameStrategyNumber:
			return contains_period_period_internal(key, query);
		case RTBeforeStrategyNumber:
			return !overafter_period_period_internal(key, query);
		case RTOverBeforeStrategyNumber:
			return !after_period_period_internal(key, query);
		case RTAfterStrategyNumber:
			return !overbefore_period_period_internal(key, query);
		case RTOverAfterStrategyNumber:
			return !before_period_period_internal(key, query);
		default:
			elog(ERROR, "unrecognized period strategy: %d", strategy);
			return false;		/* keep compiler quiet */
	}
}

/*
 * Determine whether a recheck is necessary depending on the strategy
 */

bool
index_time_bbox_recheck(StrategyNumber strategy)
{
	/* These operators are based on bounding boxes */
	if (strategy == RTBeforeStrategyNumber ||
		strategy == RTOverBeforeStrategyNumber ||
		strategy == RTAfterStrategyNumber ||
		strategy == RTOverAfterStrategyNumber)
		return false;
	
	return true;
}

/* 
 * Consistent method for time types 
 */
PG_FUNCTION_INFO_V1(gist_time_consistent);

PGDLLEXPORT Datum
gist_time_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	Oid 		subtype = PG_GETARG_OID(3);
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4),
				periodfree = false,
				result;
	Period	   *key = DatumGetPeriod(entry->key),
			   *period;
	
	/* Determine whether the operator is exact */
	*recheck = index_time_bbox_recheck(strategy);
	
	if (subtype == TIMESTAMPTZOID)
	{
		/* Since function gist_time_consistent is strict, query is not NULL */
		TimestampTz query;
		query = PG_GETARG_TIMESTAMPTZ(1);
		period = period_make(query, query, true, true);
		periodfree = true;
	}
	else if (subtype == type_oid(T_TIMESTAMPSET))
	{
		TimestampSet *query = PG_GETARG_TIMESTAMPSET(1);
		if (query == NULL)
			PG_RETURN_BOOL(false);
		period = timestampset_bbox(query);
		PG_FREE_IF_COPY(query, 1);
	}
	else if (subtype == type_oid(T_PERIOD))
	{
		period = PG_GETARG_PERIOD(1);
		if (period == NULL)
			PG_RETURN_BOOL(false);
	}
	else if (subtype == type_oid(T_PERIODSET))
	{
		PeriodSet *query = PG_GETARG_PERIODSET(1);
		if (query == NULL)
			PG_RETURN_BOOL(false);
		period = periodset_bbox(query);
		PG_FREE_IF_COPY(query, 1);
	}
	else
		elog(ERROR, "unrecognized strategy number: %d", strategy);

	if (GIST_LEAF(entry))
		result = index_leaf_consistent_time(key, period, strategy);
	else
		result = index_internal_consistent_time(key, period, strategy);
	
	if (periodfree)
		pfree(period);
	
	PG_RETURN_BOOL(result);
	
}

/*****************************************************************************
 * Union methods for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_time_union);

PGDLLEXPORT Datum
gist_time_union(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GISTENTRY  *ent = entryvec->vector;
	Period	   *result_period;
	int			i;

	result_period = DatumGetPeriod(ent[0].key);

	for (i = 1; i < entryvec->n; i++)
		result_period = period_super_union(result_period,
										 DatumGetPeriod(ent[i].key));

	PG_RETURN_PERIOD(result_period);
}

/*****************************************************************************
 * Compress methods for time types
 *****************************************************************************/

/*
 * GiST compress method for timestampset
 */
PG_FUNCTION_INFO_V1(gist_timestampset_compress);

PGDLLEXPORT Datum
gist_timestampset_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	
	if (entry->leafkey)
	{
		GISTENTRY	*retval = palloc(sizeof(GISTENTRY));
		TimestampSet *ts = DatumGetTimestampSet(entry->key);
		Period *period = palloc(sizeof(Period));
		timestampset_timespan_internal(period, ts);
		gistentryinit(*retval, PointerGetDatum(period),
			entry->rel, entry->page, entry->offset, false);
		PG_RETURN_POINTER(retval);
	}
	
	PG_RETURN_POINTER(entry);
}

/*
 * GiST compress method for period
 */
PG_FUNCTION_INFO_V1(gist_period_compress);

PGDLLEXPORT Datum
gist_period_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	
	if (entry->leafkey)
	{
		GISTENTRY	*retval = palloc(sizeof(GISTENTRY));
		
		gistentryinit(*retval, entry->key,
					  entry->rel, entry->page, entry->offset, false);
		
		PG_RETURN_POINTER(retval);
	}
	
	PG_RETURN_POINTER(entry);
}

/*
 * GiST compress method for periodset
 */
PG_FUNCTION_INFO_V1(gist_periodset_compress);

PGDLLEXPORT Datum
gist_periodset_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	
	if (entry->leafkey)
	{
		GISTENTRY *retval = palloc(sizeof(GISTENTRY));
		PeriodSet *ps = DatumGetPeriodSet(entry->key);
		Period *period = palloc(sizeof(Period));
		periodset_timespan_internal(period, ps);
		gistentryinit(*retval, PointerGetDatum(period),
			entry->rel, entry->page, entry->offset, false);
		PG_RETURN_POINTER(retval);
	}
	
	PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * Penalty method for time types
 *****************************************************************************/
/*
 * GiST page split penalty function.
 *
 * The penalty function has the following goals (in order from most to least
 * important):
 * - Avoid broadening (as determined by subtype_diff) the original predicate
 * - Favor adding periods to narrower original predicates
 */

PG_FUNCTION_INFO_V1(gist_time_penalty);
 
PGDLLEXPORT Datum
gist_time_penalty(PG_FUNCTION_ARGS)
{
	GISTENTRY  *origentry = (GISTENTRY *) PG_GETARG_POINTER(0);
	GISTENTRY  *newentry = (GISTENTRY *) PG_GETARG_POINTER(1);
	float	   *penalty = (float *) PG_GETARG_POINTER(2);
	Period  *orig = DatumGetPeriod(origentry->key);
	Period  *new = DatumGetPeriod(newentry->key);

	/*
	 * Calculate extension of original period by calling subtype_diff.
	 */
	float8		diff = 0.0;

	if (period_cmp_bounds(new->lower, orig->lower, true, true, 
			new->lower_inc, orig->lower_inc) < 0)
		diff += period_duration_secs(orig->lower, new->lower);
	if (period_cmp_bounds(new->upper, orig->upper, false, false, 
			new->upper_inc, orig->upper_inc) > 0)
		diff += period_duration_secs(new->upper, orig->upper);
	*penalty = diff;

	PG_RETURN_POINTER(penalty);
}

/*****************************************************************************
 * Picksplit method for time types
 *****************************************************************************/
 
PG_FUNCTION_INFO_V1(gist_time_picksplit);

PGDLLEXPORT Datum
gist_time_picksplit(PG_FUNCTION_ARGS)
{
	GistEntryVector *entryvec = (GistEntryVector *) PG_GETARG_POINTER(0);
	GIST_SPLITVEC *v = (GIST_SPLITVEC *) PG_GETARG_POINTER(1);
	int			nbytes;
	OffsetNumber maxoff;

	maxoff = entryvec->n - 1;
	nbytes = (maxoff + 1) * sizeof(OffsetNumber);
	v->spl_left = (OffsetNumber *) palloc(nbytes);
	v->spl_right = (OffsetNumber *) palloc(nbytes);

	gist_period_double_sorting_split(entryvec, v);

	PG_RETURN_POINTER(v);
}

/*****************************************************************************
 * Same methods for time types
 *****************************************************************************/

/* equality comparator for GiST */

PG_FUNCTION_INFO_V1(gist_time_same);

PGDLLEXPORT Datum
gist_time_same(PG_FUNCTION_ARGS)
{
	Period  *p1 = PG_GETARG_PERIOD(0);
	Period  *p2 = PG_GETARG_PERIOD(1);
	bool	   *result = (bool *) PG_GETARG_POINTER(2);
	*result = period_eq_internal(p1, p2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Fetch method for time types (result in a period)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_time_fetch);

PGDLLEXPORT Datum
gist_time_fetch(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	PG_RETURN_POINTER(entry);
}

/*****************************************************************************/