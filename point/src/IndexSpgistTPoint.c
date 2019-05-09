/*****************************************************************************
 *
 * IndexSpgistTPointGbox.c
 *	  SP-GiST implementation of 8-dimensional oct-tree over temporal points
 *
 * This module provides SP-GiST implementation for boxes using oct tree
 * analogy in 8-dimensional space.  SP-GiST doesn't allow indexing of
 * overlapping objects.  We are making 4D objects never-overlapping in
 * 8D space.  This technique has some benefits compared to traditional
 * R-Tree which is implemented as GiST.  The performance tests reveal
 * that this technique especially beneficial with too much overlapping
 * objects, so called "spaghetti data".
 *
 * Unlike the original oct-tree, we are splitting the tree into 256
 * octants in 8D space.  It is easier to imagine it as splitting space
 * four times into four:
 *
 *				|	   |						|	   |		
 *				|	   |						|	   |		
 *				| -----+-----					| -----+-----
 *				|	   |						|	   |		
 *				|	   |						|	   |		
 * -------------+------------- -+- -------------+-------------
 *				|								|
 *				|								|
 *				|								|
 *				|								|
 *				|								|
 *			  FRONT							  BACK
 *
 * We are using Gbox data type as the prefix, but we are treating them
 * as points in 8-dimensional space, because 4D boxes are not enough
 * to represent the octant boundaries in 8D space.  They however are
 * sufficient to point out the additional boundaries of the next
 * octant.
 *
 * We are using traversal values provided by SP-GiST to calculate and
 * to store the bounds of the octants, while traversing into the tree.
 * Traversal value has all the boundaries in the 8D space, and is is
 * capable of transferring the required boundaries to the following
 * traversal values.  In conclusion, three things are necessary
 * to calculate the next traversal value:
 *
 *	(1) the traversal value of the parent
 *	(2) the octant of the current node
 *	(3) the prefix of the current node
 *
 * If we visualize them on our simplified drawing (see the drawing above);
 * transferred boundaries of (1) would be the outer axis, relevant part
 * of (2) would be the up range_y part of the other axis, and (3) would be
 * the inner axis.
 *
 * For example, consider the case of overlapping.  When recursion
 * descends deeper and deeper down the tree, all octants in
 * the current node will be checked for overlapping.  The boundaries
 * will be re-calculated for all octants.  Overlap check answers
 * the question: can any box from this octant overlap with the given
 * box?  If yes, then this octant will be walked.  If no, then this
 * octant will be skipped.
 *
 * This method provides restrictions for minimum and maximum values of
 * every dimension of every corner of the box on every level of the tree
 * except the root.  For the root node, we are setting the boundaries
 * that we don't yet have as infinity.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************/

typedef struct
{
	GBOX	left;
	GBOX	right;
} CubeGbox;

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
 * Calculate the octant
 *
 * The octant is 8 bit unsigned integer with 8 least bits in use.
 * This function accepts 2 GBOX as input.  All 8 bits are set by comparing a 
 * corner of the box. This makes 256 octants in total.
 */
static uint8
getOctant8D(GBOX *centroid, GBOX *inBox)
{
	uint8 octant = 0;

	if (inBox->xmin > centroid->xmin)
		octant |= 0x80;

	if (inBox->xmax > centroid->xmax)
		octant |= 0x40;

	if (inBox->ymin > centroid->ymin)
		octant |= 0x20;

	if (inBox->ymax > centroid->ymax)
		octant |= 0x10;
	
	if (inBox->zmin > centroid->zmin)
		octant |= 0x08;
	
	if (inBox->zmax > centroid->zmax)
		octant |= 0x04;

	if (inBox->mmin > centroid->mmin)
		octant |= 0x02;

	if (inBox->mmax > centroid->mmax)
		octant |= 0x01;

	return octant;
}

/*
 * Initialize the traversal value
 *
 * In the beginning, we don't have any restrictions.  We have to
 * initialize the struct to cover the whole 8D space.
 */
static CubeGbox *
initCubeGbox(void)
{
	CubeGbox *cube_gbox = (CubeGbox *) palloc(sizeof(CubeGbox));
	double infinity = get_float8_infinity();

	cube_gbox->left.xmin = -infinity;
	cube_gbox->left.xmax = infinity;

	cube_gbox->left.ymin = -infinity;
	cube_gbox->left.ymax = infinity;

	cube_gbox->left.zmin = -infinity;
	cube_gbox->left.zmax = infinity;

	cube_gbox->left.mmin = -infinity;
	cube_gbox->left.mmax = infinity;

	cube_gbox->right.xmin = -infinity;
	cube_gbox->right.xmax = infinity;

	cube_gbox->right.ymin = -infinity;
	cube_gbox->right.ymax = infinity;

	cube_gbox->right.zmin = -infinity;
	cube_gbox->right.zmax = infinity;

	cube_gbox->right.mmin = -infinity;
	cube_gbox->right.mmax = infinity;

	return cube_gbox;
}

/*
 * Calculate the next traversal value
 *
 * All centroids are bounded by CubeGbox, but SP-GiST only keeps
 * boxes.  When we are traversing the tree, we must calculate CubeGbox,
 * using centroid and octant.
 */
static CubeGbox *
nextCubeGbox(CubeGbox *cube_gbox, GBOX *centroid, uint8 octant)
{
	CubeGbox *next_cube_gbox = (CubeGbox *) palloc(sizeof(CubeGbox));

	memcpy(next_cube_gbox, cube_gbox, sizeof(CubeGbox));

	if (octant & 0x80)
		next_cube_gbox->left.xmin = centroid->xmin;
	else
		next_cube_gbox->left.xmax = centroid->xmin;

	if (octant & 0x40)
		next_cube_gbox->right.xmin = centroid->xmax;
	else
		next_cube_gbox->right.xmax = centroid->xmax;

	if (octant & 0x20)
		next_cube_gbox->left.ymin = centroid->ymin;
	else
		next_cube_gbox->left.ymax = centroid->ymin;

	if (octant & 0x10)
		next_cube_gbox->right.ymin = centroid->ymax;
	else
		next_cube_gbox->right.ymax = centroid->ymax;

	if (octant & 0x08)
		next_cube_gbox->left.zmin = centroid->zmin;
	else
		next_cube_gbox->left.zmax = centroid->zmin;

	if (octant & 0x04)
		next_cube_gbox->right.zmin = centroid->zmax;
	else
		next_cube_gbox->right.zmax = centroid->zmax;

	if (octant & 0x02)
		next_cube_gbox->left.mmin = centroid->mmin;
	else
		next_cube_gbox->left.mmax = centroid->mmin;

	if (octant & 0x01)
		next_cube_gbox->right.mmin = centroid->mmax;
	else
		next_cube_gbox->right.mmax = centroid->mmax;

	return next_cube_gbox;
}

/* Can any cube from cube_gbox overlap with query? */
static bool
overlap8D(CubeGbox *cube_gbox, GBOX *query)
{
	double infinity = get_float8_infinity();
	bool result = true;
	/* The result value is computed only for the dimensions of the query 
	   that were not set to -+infinity */
	if (query->xmax != infinity)
		result &= cube_gbox->left.xmin <= query->xmax &&
			cube_gbox->right.xmax >= query->xmin;
	if (query->ymax != infinity)
		result &= cube_gbox->left.ymin <= query->ymax &&
			cube_gbox->right.ymax >= query->ymin;
	if (query->zmax != infinity)
		result &= cube_gbox->left.zmin <= query->zmax &&
			cube_gbox->right.zmax >= query->zmin;
	if (query->mmax != infinity)
		result &= cube_gbox->left.mmin <= query->mmax &&
			cube_gbox->right.mmax >= query->mmin;
	return result;
}

/* Can any cube from cube_gbox contain query? */
static bool
contain8D(CubeGbox *cube_gbox, GBOX *query)
{
	double infinity = get_float8_infinity();
	bool result = true;
	/* The result value is computed only for the dimensions of the query
	   that were not set to -+infinity */
	if (query->xmax != infinity)
		result &= cube_gbox->right.xmax >= query->xmax &&
			cube_gbox->left.xmin <= query->xmin;
	if (query->ymax != infinity)
		result &= cube_gbox->right.ymax >= query->ymax &&
			cube_gbox->left.ymin <= query->ymin;
	if (query->zmax != infinity)
		result &= cube_gbox->right.zmax >= query->zmax &&
			cube_gbox->left.zmin <= query->zmin;
	if (query->mmax != infinity)
		result &= cube_gbox->right.mmax >= query->mmax &&
			cube_gbox->left.mmin <= query->mmin;
	return result;
}

/* Can any cube from cube_gbox be left of query? */
static bool
left8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.xmax < query->xmin);
}

/* Can any cube from cube_gbox does not extend the right of query? */
static bool
overLeft8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.xmax <= query->xmax);
}

/* Can any cube from cube_gbox be right of query? */
static bool
right8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.xmin > query->xmax);
}

/* Can any cube from cube_gbox does not extend the left of query? */
static bool
overRight8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.xmin >= query->xmin);
}

/* Can any cube from cube_gbox be below of query? */
static bool
below8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.ymax < query->ymin);
}

/* Can any cube from cube_gbox does not extend above query? */
static bool
overBelow8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.ymax <= query->ymax);
}

/* Can any cube from cube_gbox be above of query? */
static bool
above8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.ymin > query->ymax);
}

/* Can any cube from cube_gbox does not extend below of query? */
static bool
overAbove8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.ymin >= query->ymin);
}

/* Can any cube from cube_gbox be in front of query? */
static bool
front8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.zmax < query->zmin);
}

/* Can any cube from cube_gbox does not extend the back of query? */
static bool
overFront8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.zmax <= query->zmax);
}

/* Can any cube from cube_gbox be back to query? */
static bool
back8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.zmin > query->zmax);
}

/* Can any cube from cube_gbox does not extend the front of query? */
static bool
overBack8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.zmin >= query->zmin);
}

/* Can any cube from cube_gbox be before of query? */
static bool
before8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.mmax < query->mmin);
}

/* Can any cube from cube_gbox does not extend the after of query? */
static bool
overBefore8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->right.mmax <= query->mmax);
}

/* Can any cube from cube_gbox be after of query? */
static bool
after8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.mmin > query->mmax);
}

/* Can any cube from cube_gbox does not extend the before of query? */
static bool
overAfter8D(CubeGbox *cube_gbox, GBOX *query)
{
	return (cube_gbox->left.mmin >= query->mmin);
}

/*****************************************************************************
 * SP-GiST config functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tpoint_config);

PGDLLEXPORT Datum
spgist_tpoint_config(PG_FUNCTION_ARGS)
{
	spgConfigOut *cfg = (spgConfigOut *) PG_GETARG_POINTER(1);

	Oid gbox_oid = type_oid(T_GBOX);
	cfg->prefixType = gbox_oid;	/* A type represented by its bounding box */
	cfg->labelType = VOIDOID;	/* We don't need node labels. */
	cfg->leafType = gbox_oid;
	cfg->canReturnData = false;
	cfg->longValuesOK = false;

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST choose functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tpoint_choose);

PGDLLEXPORT Datum
spgist_tpoint_choose(PG_FUNCTION_ARGS)
{
	spgChooseIn *in = (spgChooseIn *) PG_GETARG_POINTER(0);
	spgChooseOut *out = (spgChooseOut *) PG_GETARG_POINTER(1);
	GBOX *centroid = DatumGetGboxP(in->prefixDatum),
		*box = DatumGetGboxP(in->leafDatum);

	out->resultType = spgMatchNode;
	out->result.matchNode.restDatum = PointerGetDatum(box);

	/* nodeN will be set by core, when allTheSame. */
	if (!in->allTheSame)
		out->result.matchNode.nodeN = getOctant8D(centroid, box);

	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST pick-split function
 *
 * It splits a list of boxes into octants by choosing a central 8D
 * point as the median of the coordinates of the boxes.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tpoint_picksplit);

PGDLLEXPORT Datum
spgist_tpoint_picksplit(PG_FUNCTION_ARGS)
{
	spgPickSplitIn *in = (spgPickSplitIn *) PG_GETARG_POINTER(0);
	spgPickSplitOut *out = (spgPickSplitOut *) PG_GETARG_POINTER(1);
	GBOX *centroid;
	int	median, i;
	double *lowXs = palloc(sizeof(double) * in->nTuples);
	double *highXs = palloc(sizeof(double) * in->nTuples);
	double *lowYs = palloc(sizeof(double) * in->nTuples);
	double *highYs = palloc(sizeof(double) * in->nTuples);
	double *lowZs = palloc(sizeof(double) * in->nTuples);
	double *highZs = palloc(sizeof(double) * in->nTuples);
	double *lowMs = palloc(sizeof(double) * in->nTuples);
	double *highMs = palloc(sizeof(double) * in->nTuples);
	
	/* Calculate median of all 8D coordinates */
	for (i = 0; i < in->nTuples; i++)
	{
		GBOX *box = DatumGetGboxP(in->datums[i]);

		lowXs[i] = box->xmin;
		highXs[i] = box->xmax;
		lowYs[i] = box->ymin;
		highYs[i] = box->ymax;
		lowZs[i] = box->zmin;
		highZs[i] = box->zmax;
		lowMs[i] = box->mmin;
		highMs[i] = box->mmax;
	}

	qsort(lowXs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highXs, in->nTuples, sizeof(double), compareDoubles);
	qsort(lowYs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highYs, in->nTuples, sizeof(double), compareDoubles);
	qsort(lowZs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highZs, in->nTuples, sizeof(double), compareDoubles);
	qsort(lowMs, in->nTuples, sizeof(double), compareDoubles);
	qsort(highMs, in->nTuples, sizeof(double), compareDoubles);

	median = in->nTuples / 2;

	centroid = palloc0(sizeof(GBOX));

	centroid->xmin = lowXs[median];
	centroid->xmax = highXs[median];
	centroid->ymin = lowYs[median];
	centroid->ymax = highYs[median];
	centroid->zmin = lowZs[median];
	centroid->zmax = highZs[median];
	centroid->mmin = lowMs[median];
	centroid->mmax = highMs[median];

	/* Fill the output */
	out->hasPrefix = true;
	out->prefixDatum = GboxPGetDatum(centroid);

	out->nNodes = 256;
	out->nodeLabels = NULL;		/* We don't need node labels. */

	out->mapTuplesToNodes = palloc(sizeof(int) * in->nTuples);
	out->leafTupleDatums = palloc(sizeof(Datum) * in->nTuples);

	/*
	 * Assign ranges to corresponding nodes according to octants relative to
	 * the "centroid" range
	 */
	for (i = 0; i < in->nTuples; i++)
	{
		GBOX *box = DatumGetGboxP(in->datums[i]);
		uint8 octant = getOctant8D(centroid, box);
		out->leafTupleDatums[i] = GboxPGetDatum(box);
		out->mapTuplesToNodes[i] = octant;
	}

	pfree(lowXs); pfree(highXs);
	pfree(lowYs); pfree(highYs);
	pfree(lowZs); pfree(highZs);
	pfree(lowMs); pfree(highMs);
	
	PG_RETURN_VOID();
}

/*****************************************************************************
 * SP-GiST inner consistent functions for temporal points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tpoint_inner_consistent);

PGDLLEXPORT Datum
spgist_tpoint_inner_consistent(PG_FUNCTION_ARGS)
{
	spgInnerConsistentIn *in = (spgInnerConsistentIn *) PG_GETARG_POINTER(0);
	spgInnerConsistentOut *out = (spgInnerConsistentOut *) PG_GETARG_POINTER(1);
	int	i;
	MemoryContext old_ctx;
	CubeGbox *cube_gbox;
	uint16 octant;
	GBOX *centroid = DatumGetGboxP(in->prefixDatum), *queries;

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
		cube_gbox = in->traversalValue;
	else
		cube_gbox = initCubeGbox();

	/*
	 * Transform the queries into bounding boxes initializing the dimensions
	 * that must not be taken into account for the operators to infinity.
	 * This transformation is done here to avoid doing it for all octants
	 * in the loop below.
	 */
	queries = (GBOX *) palloc(sizeof(GBOX) * in->nkeys);
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Oid subtype = in->scankeys[i].sk_subtype;
		
		if (subtype == type_oid(T_GEOMETRY) || subtype == type_oid(T_GEOGRAPHY))
			/* We do not test the return value of the next function since
			   if the result is false all dimensions of the box have been 
			   initialized to +-infinity */
			geo_to_gbox_internal(&queries[i], 
				(GSERIALIZED*)PG_DETOAST_DATUM(in->scankeys[i].sk_argument));
		else if (subtype == TIMESTAMPTZOID)
			timestamp_to_gbox_internal(&queries[i],
				DatumGetTimestamp(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_TIMESTAMPSET))
			timestampset_to_gbox_internal(&queries[i],
				DatumGetTimestampSet(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_PERIOD))
			period_to_gbox_internal(&queries[i],
				DatumGetPeriod(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_PERIODSET))
			periodset_to_gbox_internal(&queries[i],
				DatumGetPeriodSet(in->scankeys[i].sk_argument));
		else if (subtype == type_oid(T_GBOX))
			memcpy(&queries[i], DatumGetGboxP(in->scankeys[i].sk_argument), sizeof(GBOX));
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
	 * traversal values (next_cube_gbox) and pass these pieces of memory to
	 * further call of this function.
	 */
	old_ctx = MemoryContextSwitchTo(in->traversalMemoryContext);

	for (octant = 0; octant < in->nNodes; octant++)
	{
		CubeGbox *next_cube_gbox = nextCubeGbox(cube_gbox, centroid, octant);
		bool flag = true;
		for (i = 0; i < in->nkeys; i++)
		{
			StrategyNumber strategy = in->scankeys[i].sk_strategy;
			switch (strategy)
			{
				case RTOverlapStrategyNumber:
				case RTContainedByStrategyNumber:
					flag = overlap8D(next_cube_gbox, &queries[i]);
					break;
				case RTContainsStrategyNumber:
				case RTSameStrategyNumber:
					flag = contain8D(next_cube_gbox, &queries[i]);
					break;
				case RTLeftStrategyNumber:
					flag = !overRight8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverLeftStrategyNumber:
					flag = !right8D(next_cube_gbox, &queries[i]);
					break;
				case RTRightStrategyNumber:
					flag = !overLeft8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverRightStrategyNumber:
					flag = !left8D(next_cube_gbox, &queries[i]);
					break;
				case RTFrontStrategyNumber:
					flag = !overBack8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverFrontStrategyNumber:
					flag = !back8D(next_cube_gbox, &queries[i]);
					break;
				case RTBackStrategyNumber:
					flag = !overFront8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverBackStrategyNumber:
					flag = !front8D(next_cube_gbox, &queries[i]);
					break;
				case RTAboveStrategyNumber:
					flag = !overBelow8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverAboveStrategyNumber:
					flag = !below8D(next_cube_gbox, &queries[i]);
					break;
				case RTBelowStrategyNumber:
					flag = !overAbove8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverBelowStrategyNumber:
					flag = !above8D(next_cube_gbox, &queries[i]);
					break;
				case RTAfterStrategyNumber:
					flag = !overBefore8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverAfterStrategyNumber:
					flag = !before8D(next_cube_gbox, &queries[i]);
					break;
				case RTBeforeStrategyNumber:
					flag = !overAfter8D(next_cube_gbox, &queries[i]);
					break;
				case RTOverBeforeStrategyNumber:
					flag = !after8D(next_cube_gbox, &queries[i]);
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
			out->traversalValues[out->nNodes] = next_cube_gbox;
			out->nodeNumbers[out->nNodes] = octant;
			out->nNodes++;
		}
		else
		{
			/*
			 * If this node is not selected, we don't need to keep the next
			 * traversal value in the memory context.
			 */
			pfree(next_cube_gbox);
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

PG_FUNCTION_INFO_V1(spgist_tpoint_leaf_consistent);

PGDLLEXPORT Datum
spgist_tpoint_leaf_consistent(PG_FUNCTION_ARGS)
{
	spgLeafConsistentIn *in = (spgLeafConsistentIn *) PG_GETARG_POINTER(0);
	spgLeafConsistentOut *out = (spgLeafConsistentOut *) PG_GETARG_POINTER(1);
	GBOX *key = DatumGetGboxP(in->leafDatum);
	bool res = true;
	int i;

	/* Initialize the value to do not recheck, will be updated below */
	out->recheck = false;

	/* leafDatum is what it is... */
	out->leafValue = in->leafDatum;

	/* Perform the required comparison(s) */
	for (i = 0; i < in->nkeys; i++)
	{
		StrategyNumber strategy = in->scankeys[i].sk_strategy;
		Oid subtype = in->scankeys[i].sk_subtype;
		GBOX query;

		/* Update the recheck flag according to the strategy */
		out->recheck |= index_tpoint_recheck(strategy);	

		if (subtype == type_oid(T_GEOMETRY) || subtype == type_oid(T_GEOGRAPHY))
		{
			GSERIALIZED *gs = (GSERIALIZED*)PG_DETOAST_DATUM(in->scankeys[i].sk_argument);
			if (!geo_to_gbox_internal(&query, gs))
				res = false;
			else
				res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (subtype == TIMESTAMPTZOID)
		{
			TimestampTz t = DatumGetTimestamp(in->scankeys[i].sk_argument);
			timestamp_to_gbox_internal(&query, t);
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (subtype == type_oid(T_TIMESTAMPSET))
		{
			TimestampSet *ts = DatumGetTimestampSet(in->scankeys[i].sk_argument);
			timestampset_to_gbox_internal(&query, ts);
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (subtype == type_oid(T_PERIOD))
		{
			Period *p = DatumGetPeriod(in->scankeys[i].sk_argument);
			period_to_gbox_internal(&query, p);
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (subtype == type_oid(T_PERIODSET))
		{
			PeriodSet *ps = DatumGetPeriodSet(in->scankeys[i].sk_argument);
			periodset_to_gbox_internal(&query, ps);
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (subtype == type_oid(T_GBOX))
		{
			GBOX *box = DatumGetGboxP(in->scankeys[i].sk_argument);
			memcpy(&query, box, sizeof(GBOX));
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else if (temporal_oid(subtype))
		{
			temporal_bbox(&query,
				DatumGetTemporal(in->scankeys[i].sk_argument));
			res = index_leaf_consistent_gbox(key, &query, strategy);
		}
		else
			elog(ERROR, "unrecognized strategy: %d", strategy);

		/* If any check is failed, we have found our answer. */
		if (!res)
			break;
	}

	PG_RETURN_BOOL(res);
}

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_tpoint_compress);

PGDLLEXPORT Datum
spgist_tpoint_compress(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *result = palloc0(sizeof(GBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_GBOX_P(result);
}

/*****************************************************************************/
