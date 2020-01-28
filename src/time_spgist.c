/*-----------------------------------------------------------------------------
 *
 * time_spgist.c
 *	Quad-tree SP-GiST index for time types.
 *
 * These functions are based on those in the file rangetypes_spgist.c.
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *-----------------------------------------------------------------------------
 */

#if MOBDB_PGSQL_VERSION >= 110000

#include "time_spgist.h"

#include <access/spgist.h>
#include <utils/timestamp.h>

#include "timetypes.h"
#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "time_gist.h"
#include "temporaltypes.h"
#include "oidcache.h"

/*****************************************************************************
 * SP-GiST config functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_period_config);

PGDLLEXPORT Datum
spgist_period_config(PG_FUNCTION_ARGS)
{
	spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);

	cfg->prefixType = type_oid(T_PERIOD);
	cfg->labelType = VOIDOID;	/* We don't need node labels. */
	cfg->leafType = type_oid(T_PERIOD);
	cfg->canReturnData = false;
	cfg->longValuesOK = false;
	
	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

/*----------
 * Determine which quadrant a 2D-mapped period falls into, relative to the
 * centroid.
 *
 * Quadrants are numbered like this:
 *
 *	 4	|  1
 *	----+----
 *	 3	|  2
 *
 * Where the lower bound of period is the horizontal axis and upper bound the
 * vertical axis.
 *
 * Periods on one of the axes are taken to lie in the quadrant with higher value
 * along perpendicular axis. That is, a value on the horizontal axis is taken
 * to belong to quadrant 1 or 4, and a value on the vertical axis is taken to
 * belong to quadrant 1 or 2. A period equal to centroid is taken to lie in
 * quadrant 1.
 *
 *----------
 */
int16
getQuadrant(Period *centroid, Period *tst)
{
	if (period_cmp_bounds(tst->lower, centroid->lower, true, true,
		tst->lower_inc, centroid->lower_inc) >= 0)
	{
		if (period_cmp_bounds(tst->upper, centroid->upper, false, false,
				tst->upper_inc, centroid->upper_inc) >= 0)
			return 1;
		else
			return 2;
	}
	else
	{
		if (period_cmp_bounds(tst->upper, centroid->upper, false, false,
				tst->upper_inc, centroid->upper_inc) >= 0)
			return 4;
		else
			return 3;
	}
}

PG_FUNCTION_INFO_V1(spgist_period_choose);

PGDLLEXPORT Datum
spgist_period_choose(PG_FUNCTION_ARGS)
{
	spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
	spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
	Period 	   *period = DatumGetPeriod(in->leafDatum),
			   *centroid;
	int16		quadrant;

	if (in->allTheSame)
	{
		out->resultType = spgMatchNode;
		/* nodeN will be set by core */
		out->result.matchNode.levelAdd = 0;
		out->result.matchNode.restDatum = PeriodGetDatum(period);
		PG_RETURN_VOID();
	}
	
	centroid = DatumGetPeriod(in->prefixDatum);
	quadrant = getQuadrant(centroid, period);

	Assert(quadrant <= in->nNodes);

	/* Select node matching to quadrant number */
	out->resultType = spgMatchNode;
	out->result.matchNode.nodeN = quadrant - 1;
	out->result.matchNode.levelAdd = 1;
	out->result.matchNode.restDatum = PeriodGetDatum(period);

	PG_RETURN_VOID();
}
/*****************************************************************************
 * SP-GiST pick-split functions
 *
 * It splits a list of time types into quadrants by choosing a central 4D
 * point as the median of the coordinates of the time types.
 *****************************************************************************/

/*
 * Bound comparison for sorting.
 */
int
period_bound_cmp(const void *a, const void *b)
{
	PeriodBound *ba = (PeriodBound *) a;
	PeriodBound *bb = (PeriodBound *) b;
	return period_cmp_bounds(ba->val, bb->val, ba->lower, bb->lower,
		ba->inclusive, bb->inclusive);
}

PG_FUNCTION_INFO_V1(spgist_period_picksplit);

PGDLLEXPORT Datum
spgist_period_picksplit(PG_FUNCTION_ARGS)
{
	spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
	spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
	Period	   *centroid;
	int			median,
				i;
	/* Use the median values of lower and upper bounds as the centroid period */
	PeriodBound *lowerBounds = palloc(sizeof(PeriodBound) * in->nTuples),
			   *upperBounds = palloc(sizeof(PeriodBound) * in->nTuples);

	/* Construct "centroid" period from medians of lower and upper bounds */
	for (i = 0; i < in->nTuples; i++)
		period_deserialize(DatumGetPeriod(in->datums[i]),
						  &lowerBounds[i], &upperBounds[i]);

	qsort(lowerBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_cmp);
	qsort(upperBounds, (size_t) in->nTuples, sizeof(PeriodBound), period_bound_cmp);

	median = in->nTuples / 2;

	centroid = period_make(
		lowerBounds[median].val, upperBounds[median].val, 
		lowerBounds[median].inclusive, upperBounds[median].inclusive);

	/* Fill the output */
	out->hasPrefix = true;
	out->prefixDatum = PeriodGetDatum(centroid);

	out->nNodes = 4;
	out->nodeLabels = NULL;		/* we don't need node labels */

	out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
	out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

	/*
	 * Assign periods to corresponding nodes according to quadrants relative to
	 * "centroid" period.
	 */
	for (i = 0; i < in->nTuples; i++)
	{
		Period	   *period = DatumGetPeriod(in->datums[i]);
		int16		quadrant = getQuadrant(centroid, period);

		out->leafTupleDatums[i] = PeriodGetDatum(period);
		out->mapTuplesToNodes[i] = quadrant - 1;
	}

	pfree(lowerBounds); pfree(upperBounds);

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions for temporal types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_period_inner_consistent);

PGDLLEXPORT Datum
spgist_period_inner_consistent(PG_FUNCTION_ARGS)
{
	spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
	spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
	int			which,
				i;
	Period 	   *centroid;
	PeriodBound	centroidLower,
				centroidUpper;

	if (in->allTheSame)
	{
		/* Report that all nodes should be visited */
		out->nNodes = in->nNodes;
		out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
		for (i = 0; i < in->nNodes; i++)
			out->nodeNumbers[i] = i;
		PG_RETURN_VOID();
	}

	/* Fetch the centroid of this node. */
	centroid = DatumGetPeriod(in->prefixDatum);
	period_deserialize(centroid, &centroidLower, &centroidUpper);

	Assert(in->nNodes == 4);

	/*
	 * Nth bit of which variable means that (N - 1)th node (Nth quadrant)
	 * should be visited. Initially all bits are set. Bits of nodes which
	 * can be skipped will be unset.
	 */
	which = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);

	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		PeriodBound	lower,
					upper;
		Period	   *query = NULL, period;

		/* Restrictions on period bounds according to scan strategy */
		PeriodBound *minLower = NULL,
				   *maxLower = NULL,
				   *minUpper = NULL,
				   *maxUpper = NULL;

		/* Are the restrictions on period bounds inclusive? */
		bool		inclusive = true;

		/*
		 * Cast the query to Period for ease of the following operations.
		 */
		
		if (in->scankeys[i].sk_subtype == TIMESTAMPTZOID)
		{
			TimestampTz t = DatumGetTimestampTz(in->scankeys[i].sk_argument);
			period_set(&period, t, t, true, true);
			query = &period;
		}
		else if (in->scankeys[i].sk_subtype == type_oid(T_TIMESTAMPSET))
			query = timestampset_bbox(
				DatumGetTimestampSet(in->scankeys[i].sk_argument));
		else if (in->scankeys[i].sk_subtype == type_oid(T_PERIOD))
			query = DatumGetPeriod(in->scankeys[i].sk_argument);
		else if (in->scankeys[i].sk_subtype == type_oid(T_PERIODSET))
			query = periodset_bbox(
				DatumGetPeriodSet(in->scankeys[i].sk_argument));
		else
			elog(ERROR, "Unrecognized strategy number: %d", strategy);
		
		period_deserialize(query, &lower, &upper);

		/*
		 * Most strategies are handled by forming a bounding box from the
		 * search key, defined by a minLower, maxLower, minUpper,
		 * maxUpper. Some modify 'which' directly, to specify exactly
		 * which quadrants need to be visited.
		 */
		switch (strategy)
		{
			case RTOverlapStrategyNumber:
				/*
				 * Periods overlap, if lower bound of each period
				 * is lower or equal to upper bound of the other period.
				 */
				maxLower = &upper;
				minUpper = &lower;
				break;

			case RTContainsStrategyNumber:
				/*
				 * Period A contains period B if lower
				 * bound of A is lower or equal to lower bound of period B
				 * and upper bound of period A is greater or equal to upper
				 * bound of period A.
				 */
				which &= (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
				maxLower = &lower;
				minUpper = &upper;
				break;

			case RTContainedByStrategyNumber:
				/* The opposite of contains. */
				minLower = &lower;
				maxUpper = &upper;
				break;

			case RTSameStrategyNumber:
				/*
				 * Equal period can be only in the same quadrant where
				 * argument would be placed to.
				 */
				which &= (1 << getQuadrant(centroid, query));
				break;

			case RTBeforeStrategyNumber:
				/*
				 * Period A is before period B if upper bound of A is lower
				 * than lower bound of B.
				 */
				maxUpper = &lower;
				inclusive = false;
				break;

			case RTOverBeforeStrategyNumber:
				/*
				 * Period A is overbefore to period B if upper bound of A is
				 * less or equal to upper bound of B.
				 */
				maxUpper = &upper;
				break;

			case RTAfterStrategyNumber:
				/*
				 * Period A is after period B if lower bound of A is greater
				 * than upper bound of B.
				 */
				minLower = &upper;
				inclusive = false;
				break;

			case RTOverAfterStrategyNumber:
				/*
				 * Period A is overafter to period B if lower bound of A is
				 * greater or equal to lower bound of B.
				 */
				minLower = &lower;
				break;

			default:
				elog(ERROR, "unrecognized strategy: %d", strategy);
		}
		
		/*
		 * Using the bounding box, see which quadrants we have to descend
		 * into.
		 */
		if (minLower)
		{
			/*
			 * If the centroid's lower bound is less than or equal to the
			 * minimum lower bound, anything in the 3rd and 4th quadrants
			 * will have an even smaller lower bound, and thus can't
			 * match.
			 */
			if (period_cmp_bounds(centroidLower.val, minLower->val,
					centroidLower.lower, minLower->lower,
					centroidLower.inclusive, minLower->inclusive) <= 0)
				which &= (1 << 1) | (1 << 2);
		}
		if (maxLower)
		{
			/*
			 * If the centroid's lower bound is greater than the maximum
			 * lower bound, anything in the 1st and 2nd quadrants will
			 * also have a greater than or equal lower bound, and thus
			 * can't match. If the centroid's lower bound is equal to the
			 * maximum lower bound, we can still exclude the 1st and 2nd
			 * quadrants if we're looking for a value strictly greater
			 * than the maximum.
			 */
			int			cmp;

			cmp = period_cmp_bounds(centroidLower.val, maxLower->val,
				centroidLower.lower, maxLower->lower,
				centroidLower.inclusive, maxLower->inclusive);
			if (cmp > 0 || (!inclusive && cmp == 0))
				which &= (1 << 3) | (1 << 4);
		}
		if (minUpper)
		{
			/*
			 * If the centroid's upper bound is less than or equal to the
			 * minimum upper bound, anything in the 2nd and 3rd quadrants
			 * will have an even smaller upper bound, and thus can't
			 * match.
			 */
			if (period_cmp_bounds(centroidUpper.val, minUpper->val,
					centroidUpper.lower, minUpper->lower,
					centroidUpper.inclusive, minUpper->inclusive) <= 0)
				which &= (1 << 1) | (1 << 4);
		}
		if (maxUpper)
		{
			/*
			 * If the centroid's upper bound is greater than the maximum
			 * upper bound, anything in the 1st and 4th quadrants will
			 * also have a greater than or equal upper bound, and thus
			 * can't match. If the centroid's upper bound is equal to the
			 * maximum upper bound, we can still exclude the 1st and 4th
			 * quadrants if we're looking for a value strictly greater
			 * than the maximum.
			 */
			int			cmp;

			cmp = period_cmp_bounds(centroidUpper.val, maxUpper->val,
					centroidUpper.lower, maxUpper->lower,
					centroidUpper.inclusive, maxUpper->inclusive);
			if (cmp > 0 || (!inclusive && cmp == 0))
				which &= (1 << 2) | (1 << 3);
		}

		if (which == 0)
			break;			/* no need to consider remaining conditions */
	}

	/* We must descend into the quadrant(s) identified by 'which' */
	out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
	out->nNodes = 0;

	for (i = 1; i <= in->nNodes; i++)
	{
		if (which & (1 << i))
		{
			out->nodeNumbers[out->nNodes] = i - 1;
			out->nNodes++;
		}
	}

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_period_leaf_consistent);

PGDLLEXPORT Datum
spgist_period_leaf_consistent(PG_FUNCTION_ARGS)
{
	spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
	spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
	Period 	   *key = DatumGetPeriod(in->leafDatum);
	bool		res = true;
	int			i;

	/* Initialization so that all the tests are exact. */
	out->recheck = false;

	/* leafDatum is what it is... */
	out->leafValue = in->leafDatum;

	/* Perform the required comparison(s) */
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Period	   *query, period;

		/* Update the recheck flag according to the strategy */
		out->recheck |= index_period_bbox_recheck(strategy);
			
		if (in->scankeys[i].sk_subtype == TIMESTAMPTZOID)
		{
			TimestampTz t = DatumGetTimestampTz(in->scankeys[i].sk_argument);
			period_set(&period, t, t, true, true);
			query = &period;
		}
		else if (in->scankeys[i].sk_subtype == type_oid(T_TIMESTAMPSET))
			query = timestampset_bbox(
				DatumGetTimestampSet(in->scankeys[i].sk_argument));
		else if (in->scankeys[i].sk_subtype == type_oid(T_PERIOD))
			query = DatumGetPeriod(in->scankeys[i].sk_argument);
		else if (in->scankeys[i].sk_subtype ==  type_oid(T_PERIODSET))
			query = periodset_bbox(
				DatumGetPeriodSet(in->scankeys[i].sk_argument));
		else
			elog(ERROR, "Unrecognized strategy number: %d", strategy);

		res = index_leaf_consistent_time(key, query, strategy);

		/* If any check is failed, we have found our answer. */
		if (!res)
			break;
	}

	PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/


PG_FUNCTION_INFO_V1(spgist_timestampset_compress);

PGDLLEXPORT Datum
spgist_timestampset_compress(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Period	   *period;

	period = period_copy(timestampset_bbox(ts));

	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_PERIOD(period);
}


PG_FUNCTION_INFO_V1(spgist_periodset_compress);

PGDLLEXPORT Datum
spgist_periodset_compress(PG_FUNCTION_ARGS)
{
	PeriodSet  *ps = PG_GETARG_PERIODSET(0);
	Period	   *period;

	period = period_copy(periodset_bbox(ps));

	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_PERIOD(period);
}
#endif

/*****************************************************************************/
