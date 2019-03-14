/*-----------------------------------------------------------------------------
 *
 * IndexSpgistTnumber.c
 *		SP-GiST implementation of 4-dimensional quad tree over temporal
 *		integers and floats.
 *
 * These functions are based on those in the file geo_spgist.c.
 * This module provides SP-GiST implementation for temporal number types 
 * using a quad tree analogy in 4-dimensional space. Notice that
 * SP-GiST doesn't allow indexing of overlapping objects.  We are making 
 * 2D objects never-overlapping in 4D space.  This technique has some  
 * benefits compared to traditional R-Tree which is implemented as GiST.  
 * The performance tests reveal that this technique especially beneficial 
 * with too much overlapping objects, so called "spaghetti data".
 *
 * Unlike the original quad tree, we are splitting the tree into 16
 * quadrants in 4D space.  It is easier to imagine it as splitting space
 * two times into 4:
 *
 *				|	   |
 *				|	   |
 *				| -----+-----
 *				|	   |
 *				|	   |
 * -------------+-------------
 *				|
 *				|
 *				|
 *				|
 *				|
 *
 * We are using box datatype as the prefix, but we are treating them
 * as points in 4-dimensional space, because 2D boxes are not enough
 * to represent the quadrant boundaries in 4D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * quadrant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the quadrants, while traversing into the tree.
 * Traversal value has all the boundaries in the 4D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *	(1) the traversal value of the parent
 *	(2) the quadrant of the current node
 *	(3) the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing after);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up right part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all quadrants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all quadrants.  Overlap check answers
 * the question: can any box from this quadrant overlap with the given
 * box?  If yes, then this quadrant will be walked.  If no, then this
 * quadrant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *-----------------------------------------------------------------------------
 */

#include "TemporalTypes.h"

/*****************************************************************************/

typedef struct
{
	BOX	left;
	BOX	right;
} RectBox;

/*
 * Comparator for qsort
 *
 * We don't need to use the floating point macros in here, because this is
 * only going to be used in a place to affect the performance of the index,
 * not the correctness.
 */
static int
compareDoubles(const void *a, const void *b)
{
	double		x = *(double *) a;
	double		y = *(double *) b;

	if (x == y)
		return 0;
	return (x > y) ? 1 : -1;
}

/*
 * Calculate the quadrant
 *
 * The quadrant is 8 bit unsigned integer with 4 least bits in use.
 * This function accepts BOXes as input. All 4 bits are set by comparing 
 * a corner of the box. This makes 16 quadrants in total.
 */
static uint8
getQuadrant4D(BOX *centroid, BOX *inBox)
{
	uint8 quadrant = 0;

	if (inBox->low.x > centroid->low.x)
		quadrant |= 0x8;

	if (inBox->high.x > centroid->high.x)
		quadrant |= 0x4;

	if (inBox->low.y > centroid->low.y)
		quadrant |= 0x2;

	if (inBox->high.y > centroid->high.y)
		quadrant |= 0x1;

	return quadrant;
}

/*
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 4D space.
 */
static RectBox *
initRectBox(void)
{
	RectBox *rect_box = (RectBox *) palloc(sizeof(RectBox));
	double infinity = get_float8_infinity();

	rect_box->left.low.x = -infinity;
	rect_box->left.high.x = infinity;

	rect_box->left.low.y = -infinity;
	rect_box->left.high.y = infinity;

	rect_box->right.low.x = -infinity;
	rect_box->right.high.x = infinity;

	rect_box->right.low.y = -infinity;
	rect_box->right.high.y = infinity;

	return rect_box;
}

/*
 * Calculate the next traversal value
 *
 * All centroids are bounded by RectBox, but SP-GiST only keeps
 * boxes.  When we are traversing the tree, we must calculate RectBox,
 * using centroid and quadrant.
 */
static RectBox *
nextRectBox(RectBox *rect_box, BOX *centroid, uint8 quadrant)
{
	RectBox *next_rect_box = (RectBox *) palloc(sizeof(RectBox));

	memcpy(next_rect_box, rect_box, sizeof(RectBox));

	if (quadrant & 0x8)
		next_rect_box->left.low.x = centroid->low.x;
	else
		next_rect_box->left.high.x = centroid->low.x;

	if (quadrant & 0x4)
		next_rect_box->right.low.x = centroid->high.x;
	else
		next_rect_box->right.high.x = centroid->high.x;

	if (quadrant & 0x2)
		next_rect_box->left.low.y = centroid->low.y;
	else
		next_rect_box->left.high.y = centroid->low.y;

	if (quadrant & 0x1)
		next_rect_box->right.low.y = centroid->high.y;
	else
		next_rect_box->right.high.y = centroid->high.y;

	return next_rect_box;
}

/* Can any rectangle from rect_box overlap with this argument? */
static bool
overlap4D(RectBox *rect_box, BOX *query)
{
	double infinity = get_float8_infinity();
	bool result = true;
	/* If the missing dimension of the query was not set to -+infinity */
	if (query->high.x != infinity)
		result &= rect_box->left.low.x <= query->high.x &&
			rect_box->right.high.x >= query->low.x;
	/* If the missing dimension of the query was not set to -+infinity */
	if (query->high.y != infinity)
		result &= rect_box->left.low.y <= query->high.y &&
			rect_box->right.high.y >= query->low.y;
	return result;
}

/* Can any rectangle from rect_box contain this argument? */
static bool
contain4D(RectBox *rect_box, BOX *query)
{
	double infinity = get_float8_infinity();
	bool result = true;
	/* If the missing dimension of the query was not set to -+infinity */
	if (query->high.x != infinity)
		result &= rect_box->right.high.x >= query->high.x &&
			rect_box->left.low.x <= query->low.x;
	/* If the missing dimension of the query was not set to -+infinity */
	if (query->high.y != infinity)
		result &= rect_box->right.high.y >= query->high.y &&
			rect_box->left.low.y <= query->low.y;
	return result;
}

/* Can any rectangle from rect_box be left of this argument? */
static bool
left4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->right.high.x < query->low.x);
}

/* Can any rectangle from rect_box does not extend the right of this argument? */
static bool
overLeft4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->right.high.x <= query->high.x);
}

/* Can any rectangle from rect_box be right of this argument? */
static bool
right4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->left.low.x > query->high.x);
}

/* Can any rectangle from rect_box does not extend the left of this argument? */
static bool
overRight4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->left.low.x >= query->low.x);
}

/* Can any rectangle from rect_box be before this argument? */
static bool
before4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->right.high.y < query->low.y);
}

/* Can any rectangle from rect_box does not extend after this argument? */
static bool
overBefore4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->right.high.y <= query->high.y);
}

/* Can any rectangle from rect_box be after this argument? */
static bool
after4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->left.low.y > query->high.y);
}

/* Can any rectangle from rect_box does not extend before this argument? */
static bool
overAfter4D(RectBox *rect_box, BOX *query)
{
	return (rect_box->left.low.y >= query->low.y);
}

/*****************************************************************************
 * SP-GiST config function
 *****************************************************************************/
 
PG_FUNCTION_INFO_V1(spgist_tnumber_config);

PGDLLEXPORT Datum
spgist_tnumber_config(PG_FUNCTION_ARGS)
{
	spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);

	cfg->prefixType = BOXOID;	/* A type represented by its bounding box */
	cfg->labelType = VOIDOID;	/* We don't need node labels. */
	cfg->leafType = BOXOID;
	cfg->canReturnData = false;
	cfg->longValuesOK = false;

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tnumber_choose);

PGDLLEXPORT Datum
spgist_tnumber_choose(PG_FUNCTION_ARGS)
{
	spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
	spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
	BOX *centroid = DatumGetBoxP(in->prefixDatum),
		*box = DatumGetBoxP(in->leafDatum);

	out->resultType = spgMatchNode;
	out->result.matchNode.restDatum = PointerGetDatum(box);

	/* nodeN will be set by core, when allTheSame. */
	if (!in->allTheSame)
		out->result.matchNode.nodeN = getQuadrant4D(centroid, box);

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *
 * It splits a list of boxes into quadrants by choosing a central 4D
 * point as the median of the coordinates of the boxes.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tnumber_picksplit);

PGDLLEXPORT Datum
spgist_tnumber_picksplit(PG_FUNCTION_ARGS)
{
	spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
	spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
	BOX *centroid;
	int median, i;
	double *lowXs = palloc(sizeof(double) * in->nTuples);
	double *highXs = palloc(sizeof(double) * in->nTuples);
	double *lowYs = palloc(sizeof(double) * in->nTuples);
	double *highYs = palloc(sizeof(double) * in->nTuples);

	/* Calculate median of all 4D coordinates */
	for (i = 0; i < in->nTuples; i++)
	{
		BOX	*box = DatumGetBoxP(in->datums[i]);

		lowXs[i] = box->low.x;
		highXs[i] = box->high.x;
		lowYs[i] = box->low.y;
		highYs[i] = box->high.y;
	}

	qsort(lowXs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highXs, in->nTuples, sizeof(double), compareDoubles);
	qsort(lowYs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highYs, in->nTuples, sizeof(double), compareDoubles);

	median = in->nTuples / 2;

	centroid = palloc(sizeof(BOX));

	centroid->low.x = lowXs[median];
	centroid->high.x = highXs[median];
	centroid->low.y = lowYs[median];
	centroid->high.y = highYs[median];

	/* Fill the output */
	out->hasPrefix = true;
	out->prefixDatum = BoxPGetDatum(centroid);

	out->nNodes = 16;
	out->nodeLabels = NULL;		/* We don't need node labels. */

	out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
	out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

	/*
	 * Assign ranges to corresponding nodes according to quadrants relative to
	 * the "centroid" range
	 */
	for (i = 0; i < in->nTuples; i++)
	{
		BOX *box = DatumGetBoxP(in->datums[i]);
		uint8 quadrant = getQuadrant4D(centroid, box);

		out->leafTupleDatums[i] = BoxPGetDatum(box);
		out->mapTuplesToNodes[i] = quadrant;
	}

	pfree(lowXs); pfree(highXs);
	pfree(lowYs); pfree(highYs);
	
	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent function for temporal numbers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tnumber_inner_consistent);

PGDLLEXPORT Datum
spgist_tnumber_inner_consistent(PG_FUNCTION_ARGS)
{
	spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
	spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
	int i;
	MemoryContext old_ctx;
	RectBox *rect_box;
	uint8 quadrant;
	BOX *centroid = DatumGetBoxP(in->prefixDatum), *queries;

	if (in->allTheSame)
	{
		/* Report that all nodes should be visited */
		out->nNodes = in->nNodes;
		out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
		for (i = 0; i < in->nNodes; i++)
			out->nodeNumbers[i] = i;

		PG_RETURN_VOID();
	}

	/*
	 * We are saving the traversal value or initialize it an unbounded one, if
	 * we have just begun to walk the tree.
	 */
	if (in->traversalValue)
		rect_box = in->traversalValue;
	else
		rect_box = initRectBox();

	/*
	 * Transform the queries into bounding boxes initializing the dimensions
	 * that must not be taken into account for the operators to infinity.
	 */
	queries = (BOX *) palloc(sizeof(BOX) * in->nkeys);
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Oid subtype = in->scankeys[i].sk_subtype;
		
		if (subtype == INT4OID || subtype == FLOAT8OID)
			base_to_box(&queries[i],
				in->scankeys[i].sk_argument, subtype);
		else if (subtype == type_oid(T_INTRANGE) || subtype == type_oid(T_FLOATRANGE))
			range_to_box(&queries[i],
				DatumGetRangeTypeP(in->scankeys[i].sk_argument));
		else if (subtype == TIMESTAMPTZOID)
			timestamp_to_box(&queries[i],
				DatumGetTimestamp(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_TIMESTAMPSET))
			timestampset_to_box(&queries[i],
				DatumGetTimestampSet(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_PERIOD))
			period_to_box(&queries[i],
				DatumGetPeriod(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_PERIODSET))
			periodset_to_box(&queries[i],
				DatumGetPeriodSet(in->scankeys[i].sk_argument));
		else if (subtype == BOXOID)
			memcpy(&queries[i], DatumGetBoxP(in->scankeys[i].sk_argument), sizeof(BOX));
		else if (temporal_oid(subtype))
			temporal_bbox(&queries[i],
				DatumGetTemporal(in->scankeys[i].sk_argument));
		else
			elog(ERROR, "Unrecognized strategy number: %d", strategy);
	}

	/* Allocate enough memory for nodes */
	out->nNodes = 0;
	out->nodeNumbers = (int *) palloc(sizeof(int) * in->nNodes);
	out->traversalValues = (void **) palloc(sizeof(void *) * in->nNodes);

	/*
	 * We switch memory context, because we want to allocate memory for new
	 * traversal values (next_rect_box) and pass these pieces of memory to
	 * further call of this function.
	 */
	old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

	for (quadrant = 0; quadrant < in->nNodes; quadrant++)
	{
		RectBox *next_rect_box = nextRectBox(rect_box, centroid, quadrant);
		bool flag = true;
		for (i = 0; i < in->nkeys; i++)
		{
			StrategyNumber strategy = in->scankeys[i].sk_strategy;
			switch (strategy)
			{
				case RTOverlapStrategyNumber:
				case RTContainedByStrategyNumber:
					flag = overlap4D(next_rect_box, &queries[i]);
					break;
				case RTContainsStrategyNumber:
				case RTSameStrategyNumber:
					flag = contain4D(next_rect_box, &queries[i]);
					break;
				case RTLeftStrategyNumber:
					flag = !overRight4D(next_rect_box, &queries[i]);
					break;
				case RTOverLeftStrategyNumber:
					flag = !right4D(next_rect_box, &queries[i]);
					break;
				case RTRightStrategyNumber:
					flag = !overLeft4D(next_rect_box, &queries[i]);
					break;
				case RTOverRightStrategyNumber:
					flag = !left4D(next_rect_box, &queries[i]);
					break;
				case RTBeforeStrategyNumber:
					flag = !overAfter4D(next_rect_box, &queries[i]);
					break;
				case RTOverBeforeStrategyNumber:
					flag = !after4D(next_rect_box, &queries[i]);
					break;
				case RTAfterStrategyNumber:
					flag = !overBefore4D(next_rect_box, &queries[i]);
					break;
				case RTOverAfterStrategyNumber:
					flag = !before4D(next_rect_box, &queries[i]);
					break;
				default:
					elog(ERROR, "unrecognized strategy: %d", strategy);
			}
			/* If any check is failed, we have found our answer. */
			if (!flag)
				break;
		}
		if (flag)
		{
			out->traversalValues[out->nNodes] = next_rect_box;
			out->nodeNumbers[out->nNodes] = quadrant;
			out->nNodes++;
		}
		else
		{
			/*
			 * If this node is not selected, we don't need to keep the next
			 * traversal value in the memory context.
			 */
			pfree(next_rect_box);
		}
	}

	/* Switch after */
	MemoryContextSwitchTo(old_ctx);

	pfree(queries); 

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST leaf-level consistency function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tnumber_leaf_consistent);

PGDLLEXPORT Datum
spgist_tnumber_leaf_consistent(PG_FUNCTION_ARGS)
{
	spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
	spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
	BOX *key = DatumGetBoxP(in->leafDatum), query;
	bool res = true;
	int	i;

	/* 
	 * All tests are lossy since boxes do not distinghish between inclusive  
	 * and exclusive bounds. 
	 */
	out->recheck = true;

	/* leafDatum is what it is... */
	out->leafValue = in->leafDatum;

	/* Perform the required comparison(s) */
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Oid subtype = in->scankeys[i].sk_subtype;	
		
		if (subtype == INT4OID || subtype == FLOAT8OID)
		{
			Datum base = in->scankeys[i].sk_argument;
			base_to_box(&query, base, subtype);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == type_oid(T_INTRANGE) || subtype == type_oid(T_FLOATRANGE))
		{
			RangeType *range = DatumGetRangeTypeP(in->scankeys[i].sk_argument);
			range_to_box(&query, range);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == TIMESTAMPTZOID)
		{
			TimestampTz t = DatumGetTimestamp(in->scankeys[i].sk_argument);
			timestamp_to_box(&query, t);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == type_oid(T_TIMESTAMPSET))
		{
			TimestampSet *ts = DatumGetTimestampSet(in->scankeys[i].sk_argument);
			timestampset_to_box(&query, ts);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == type_oid(T_PERIOD))
		{
			Period *period = DatumGetPeriod(in->scankeys[i].sk_argument);
			period_to_box(&query, period);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == type_oid(T_PERIODSET))
		{
			PeriodSet *ps = DatumGetPeriodSet(in->scankeys[i].sk_argument);
			periodset_to_box(&query, ps);									  
			res = index_leaf_consistent_box(key, &query, strategy);
		}
		else if (subtype == BOXOID)
		{
			BOX *box = DatumGetBoxP(in->scankeys[i].sk_argument);
			res = index_leaf_consistent_box(key, box, strategy);
		}
		else if (temporal_oid(subtype))
		{
			temporal_bbox(&query,
				DatumGetTemporal(in->scankeys[i].sk_argument));
			res = index_leaf_consistent_box(key, &query, strategy);
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
 * SP-GiST compress function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tnumber_compress);

PGDLLEXPORT Datum
spgist_tnumber_compress(PG_FUNCTION_ARGS)
{
	Temporal   *temp = PG_GETARG_TEMPORAL(0);
	BOX		   *box = palloc(sizeof(BOX));

	temporal_bbox(box, temp);

	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOX_P(box);
}

/*****************************************************************************/
