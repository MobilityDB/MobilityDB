/*****************************************************************************
 *
 * temporal_spgist.c
 *	Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * These functions are based on those in the file rangetypes_spgist.c.
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_spgist.h"

#include <access/spgist.h>
#include <utils/builtins.h>

#include "timetypes.h"
#include "oidcache.h"
#include "period.h"
#include "time_gist.h"
#include "time_spgist.h"
#include "temporaltypes.h"

/*****************************************************************************
 * SP-GiST inner consistent function for temporal types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_temporal_inner_consistent);

PGDLLEXPORT Datum
spgist_temporal_inner_consistent(PG_FUNCTION_ARGS)
{
	spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
	spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
	int	which, i;
	Period *centroid;
	PeriodBound	centroidLower, centroidUpper;

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
		PeriodBound	lower, upper;
		Period *period = NULL;

		/* Restrictions on period bounds according to scan strategy */
		PeriodBound *minLower = NULL,
				   *maxLower = NULL,
				   *minUpper = NULL,
				   *maxUpper = NULL;

		/* Are the restrictions on period bounds inclusive? */
		bool inclusive = true;

		/*
		 * Cast the query to Period for ease of the following operations.
		 */
		
		bool mustfree = false;
		Oid subtype = in->scankeys[i].sk_subtype;

		if (subtype == type_oid(T_PERIOD))
			period = DatumGetPeriod(in->scankeys[i].sk_argument);
		else if (temporal_type_oid(subtype))
		{
			period = palloc(sizeof(Period));
			temporal_bbox(period,
				DatumGetTemporal(in->scankeys[i].sk_argument));
			mustfree = true;
		}
		else
			elog(ERROR, "Unrecognized strategy number: %d", strategy);
		
		period_deserialize(period, &lower, &upper);

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
				which &= (1 << getQuadrant(centroid, period));
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
		if (mustfree)
			pfree(period);
		
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

PG_FUNCTION_INFO_V1(spgist_temporal_leaf_consistent);

PGDLLEXPORT Datum
spgist_temporal_leaf_consistent(PG_FUNCTION_ARGS)
{
	spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
	spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
	Period 	   *key = DatumGetPeriod(in->leafDatum);
	bool		res = true;
	int			i;

	/* All tests are lossy. */
	out->recheck = true;

	/* leafDatum is what it is... */
	out->leafValue = in->leafDatum;

	/* Perform the required comparison(s) */
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Period *query;
		Oid subtype = in->scankeys[i].sk_subtype;
		
		if (subtype == type_oid(T_PERIOD))
		{
			query = DatumGetPeriod(in->scankeys[i].sk_argument);
			res = index_leaf_consistent_time(key, query, strategy);
		}
		else if (temporal_type_oid(subtype))
		{
			Period period;
			temporal_bbox(&period,
				DatumGetTemporal(in->scankeys[i].sk_argument));
			res = index_leaf_consistent_time(key, &period, strategy);
	}
		else
			elog(ERROR, "Unrecognized strategy number: %d", strategy);

		/* If any check is failed, we have found our answer. */
		if (!res)
			break;
	}

	PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION >= 110000
PG_FUNCTION_INFO_V1(spgist_temporal_compress);

PGDLLEXPORT Datum
spgist_temporal_compress(PG_FUNCTION_ARGS)
{
	Temporal	   *temp = PG_GETARG_TEMPORAL(0);
	Period		   *period = palloc(sizeof(Period));

	temporal_bbox(period, temp);

	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_PERIOD(period);
}
#endif

/*****************************************************************************/
