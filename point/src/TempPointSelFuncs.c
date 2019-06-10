/*****************************************************************************
 *
 * TempPointSelFuncs.c
 *      Functions for selectivity estimation of operators on temporal point 
 *      types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	point/src/TempPointSelFuncs.c
 *
 *****************************************************************************/

#include <TemporalTypes.h>
#include "TemporalTypes.h"
#include "TemporalPoint.h"
#include "TemporalSelFuncs.h"

/*
 *	Selectivity functions for temporal types operators.  These are bogus -- 
 *	unless we know the actual key distribution in the index, we can't make
 *	a good prediction of the selectivity of these operators.
 *
 *	Note: the values used here may look unreasonably small.  Perhaps they
 *	are.  For now, we want to make sure that the optimizer will make use
 *	of a geometric index if one is available, so the selectivity had better
 *	be fairly small.
 *
 *	In general, GiST needs to search multiple subtrees in order to guarantee
 *	that all occurrences of the same key have been found.  Because of this,
 *	the estimated cost for scanning the index ought to be higher than the
 *	output selectivity would indicate.  gistcostestimate(), over in selfuncs.c,
 *	ought to be adjusted accordingly --- but until we can generate somewhat
 *	realistic numbers here, it hardly matters...
 */

/*****************************************************************************/

/*
 * Selectivity for operators for bounding box operators, i.e., overlaps (&&), 
 * contains (@>), contained (<@), and, same (~=). These operators depend on 
 * volume. Contains and contained are tighter contraints than overlaps, so 
 * the former should produce lower estimates than the latter. Similarly, 
 * equals is a tighter constrain tha contains and contained.
 */

PG_FUNCTION_INFO_V1(tpoint_overlaps_sel);

PGDLLEXPORT Datum
tpoint_overlaps_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = tpoint_sel(root, operator, args, varRelid, OVERLAPS_OP);
	if (selec < 0.0)
		selec = 0.005;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tpoint_overlaps_joinsel);

PGDLLEXPORT Datum
tpoint_overlaps_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tpoint_contains_sel);

PGDLLEXPORT Datum
tpoint_contains_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = tpoint_sel(root, operator, args, varRelid, get_tpoint_cacheOp(operator));
	if (selec < 0.0)
		selec = 0.002;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tpoint_contains_joinsel);

PGDLLEXPORT Datum
tpoint_contains_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tpoint_same_sel);

PGDLLEXPORT Datum
tpoint_same_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = tpoint_sel(root, operator, args, varRelid, SAME_OP);
	if (selec < 0.0)
		selec = 0.001;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);

}

PG_FUNCTION_INFO_V1(tpoint_same_joinsel);

PGDLLEXPORT Datum
tpoint_same_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

/*
 * Selectivity for operators for relative position box operators, i.e., 
 * left (<<), overleft (&<), right (>>), overright (&>), 
 * below (<<|), overbelow (&<|), above (|>>), overabove (|&>), 
 * front (<</), overfront (&</), back (/>>), overfront (/&>), 
 * before (<<#), overbefore (&<#), after (#>>), overafter (#&>). 
 */

PG_FUNCTION_INFO_V1(tpoint_position_sel);

PGDLLEXPORT Datum
tpoint_position_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = tpoint_sel(root, operator, args, varRelid, get_tpoint_cacheOp(operator));
	if (selec < 0.0)
		selec = 0.001;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tpoint_position_joinsel);

PGDLLEXPORT Datum
tpoint_position_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************
 * Helper functions for calculating the selectivity.
 *****************************************************************************/
/** Generic selectivity function for all operators */
Selectivity
tpoint_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp)
{
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0; /* keep compiler quiet */

	/*
	 * If expression is not (variable op something) or (something op
	 * variable), then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(0.01);

	/*
	 * Can't do anything useful if the something is not a constant, either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(0.01);
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

	STBOX box = get_stbox(other);

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
			/* Use default selectivity (should we raise an error instead?) */
			ReleaseVariableStats(vardata);
			PG_RETURN_FLOAT8(0.01);
		}
	}

	selec = estimate_selectivity(root, &vardata, other, &box, cachedOp);

	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	return selec;
}
/** Estimate the selectivity of geometry/geography types */
Selectivity
estimate_selectivity(PlannerInfo *root, VariableStatData *vardata, Node *other, const STBOX *box, CachedOp op)
{
	int d; /* counter */
	ND_BOX nd_box;
	ND_IBOX nd_ibox;
	int at[ND_DIMS];
	double cell_size[ND_DIMS];
	double min[ND_DIMS];
	double max[ND_DIMS];
    bool selec2Flag = false;
    Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */

	ND_STATS *nd_stats;
	AttStatsSlot sslot;
	if (!(HeapTupleIsValid(vardata->statsTuple) &&
		  get_attstatsslot_internal(&sslot, vardata->statsTuple, STATISTIC_KIND_2D, InvalidOid,
									ATTSTATSSLOT_NUMBERS, VALUE_STATISTICS)))
	{
		return -1;
	}
	/* Clone the stats here so we can release the attstatsslot immediately */
	nd_stats = palloc(sizeof(float4) * sslot.nnumbers);
	memcpy(nd_stats, sslot.numbers, sizeof(float4) * sslot.nnumbers);

	free_attstatsslot(&sslot);


	/* Calculate the overlap of the box on the histogram */
	if (!nd_stats)
	{
		elog(NOTICE, " estimate_selectivity called with null input");
		return FALLBACK_ND_SEL;
	}

	/* Initialize nd_box. */
	nd_box_from_stbox(box, &nd_box);

	/*
	 * To return 2D stats on an ND sample, we need to make the
	 * 2D box cover the full range of the other dimensions in the
	 * histogram.
	 */

	int ndims_max = (int)nd_stats->ndims;

	/*
	 * Search box completely misses histogram extent?
	 * We have to intersect in all N dimensions or else we have
	 * zero interaction under the &&& operator. It's important
	 * to short circuit in this case, as some of the tests below
	 * will return junk results when run on non-intersecting inputs.
	 */
	if ((op == OVERLAPS_OP || op == CONTAINS_OP || op == CONTAINED_OP || op == SAME_OP) &&
		!nd_box_intersects(&nd_box, &(nd_stats->extent), ndims_max))
	{
		return 0.0;
	}

	/* Search box completely contains histogram extent! */
	if ((op == OVERLAPS_OP || op == CONTAINS_OP || op == CONTAINED_OP || op == SAME_OP) &&
		nd_box_contains(&nd_box, &(nd_stats->extent), ndims_max) )
	{
		return 1.0;
	}

	/* Calculate the overlap of the box on the histogram */
	if (!nd_box_overlap(nd_stats, &nd_box, &nd_ibox))
	{
		return FALLBACK_ND_SEL;
	}

	if (op == OVERLAPS_OP || op == CONTAINS_OP || op == CONTAINED_OP || op == SAME_OP)
	{
		/* Work out some measurements of the histogram */
		for (d = 0; d < nd_stats->ndims; d++)
		{
			/* Cell size in each dim */
			min[d] = nd_stats->extent.min[d];
			max[d] = nd_stats->extent.max[d];
			cell_size[d] = (max[d] - min[d]) / nd_stats->size[d];

			/* Initialize the counter */
			at[d] = nd_ibox.min[d];
		}
	}

	switch (op)
	{
		case OVERLAPS_OP:
		case SAME_OP:
		{
			double total_count = 0.0;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell = { {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0} };

				/* We have to pro-rate partially overlapped cells. */
				for (d = 0; d < nd_stats->ndims; d++)
				{
					nd_cell.min[d] = (float4)(min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4)(min[d] + (at[d] + 1) * cell_size[d]);
				}
				cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];
				ratio = (float4)nd_box_ratio(&nd_box, &nd_cell, (int) nd_stats->ndims);

				/* Add the pro-rated count for this cell to the overall total */
				total_count += cell_count * ratio;
			} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));

			/* Scale by the number of features in our histogram to get the proportion */
			selec1 = total_count / nd_stats->histogram_features;

            if (MOBDB_FLAGS_GET_T(box->flags))
            {
                ((Const *) other)->constvalue = PointerGetDatum(period_make((TimestampTz)box->tmin,
                                                                            (TimestampTz)box->tmax,
                                                                            true, true));
                selec2 = estimate_selectivity_temporal_dimension(root, *vardata, other, OVERLAPS_OP);
                selec2Flag = true;
            }
            if(selec2 == 0.0 && !selec2Flag)
                selec = selec1;
            else
                selec = selec1 * selec2;

			/* Prevent rounding overflows */
			if (selec > 1.0) selec = 1.0;
			else if (selec < 0.0 ) selec = 0.0;

			return selec;
		}
		case CONTAINS_OP:
		{
			selec1 = FLT_MIN;
			double maxx = 0;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell = {{0.0, 0.0, 0.0, 0.0},
								  {0.0, 0.0, 0.0, 0.0}};

				/* We have to pro-rate partially overlapped cells. */
				for (d = 0; d < nd_stats->ndims; d++)
				{
					nd_cell.min[d] = (float4)(min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4)(min[d] + (at[d] + 1) * cell_size[d]);
				}

				ratio = (float4)nd_box_ratio(&nd_box, &nd_cell, (int)nd_stats->ndims);
				cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

				maxx = cell_count * ratio;
				if (selec1 < maxx)
					selec1 = maxx;
			} while (nd_increment(&nd_ibox, (int)nd_stats->ndims, at));

			/* Scale by the number of features in our histogram to get the proportion */
			selec1 = selec1 / nd_stats->histogram_features;

            if (MOBDB_FLAGS_GET_T(box->flags))
            {
                ((Const *) other)->constvalue = PointerGetDatum(period_make((TimestampTz)box->tmin,
                                                                            (TimestampTz)box->tmax,
                                                                            true, true));
                selec2 = estimate_selectivity_temporal_dimension(root, *vardata, other, OVERLAPS_OP);
                selec2Flag = true;
            }
            if(selec2 == 0.0 && !selec2Flag)
                selec = selec1;
            else
                selec = selec1 * selec2;

            /* Prevent rounding overflows */
            if (selec > 1.0) selec = 1.0;
            else if (selec < 0.0 ) selec = 0.0;

            return selec;
		}
		case CONTAINED_OP:
		{
			selec1 = FLT_MAX;
			double minx;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell = {{0.0, 0.0, 0.0, 0.0},
								  {0.0, 0.0, 0.0, 0.0}};

				/* We have to pro-rate partially overlapped cells. */
				for (d = 0; d < nd_stats->ndims; d++)
				{
					nd_cell.min[d] = (float4)(min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4)(min[d] + (at[d] + 1) * cell_size[d]);
				}

				ratio = (float4)nd_box_ratio(&nd_box, &nd_cell, (int)nd_stats->ndims);
				cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

				minx = cell_count * ratio;
				if (selec1 > minx)
					selec1 = minx;
			} while (nd_increment(&nd_ibox, (int)nd_stats->ndims, at));
            if (MOBDB_FLAGS_GET_T(box->flags))
            {
                ((Const *) other)->constvalue = PointerGetDatum(period_make((TimestampTz)box->tmin,
                                                                            (TimestampTz)box->tmax,
                                                                            true, true));
                selec2 = estimate_selectivity_temporal_dimension(root, *vardata, other, OVERLAPS_OP);
                selec2Flag = true;
            }
            if(selec2 == 0.0 && !selec2Flag)
                selec = selec1;
            else
                selec = selec1 * selec2;

            /* Prevent rounding overflows */
            if (selec > 1.0) selec = 1.0;
            else if (selec < 0.0 ) selec = 0.0;
			return selec;
		}
		case LEFT_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, LEFT_OP, X_DIMS);
			return selec;
		case RIGHT_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, RIGHT_OP, X_DIMS);
			return selec;
		case OVERLEFT_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, OVERLEFT_OP, X_DIMS);
			return selec;
		case OVERRIGHT_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, OVERRIGHT_OP, X_DIMS);
			return selec;
		case BELOW_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, BELOW_OP, Y_DIMS);
			return selec;
		case ABOVE_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, ABOVE_OP, Y_DIMS);
			return selec;
		case OVERABOVE_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, OVERABOVE_OP, Y_DIMS);
			return selec;
		case OVERBELOW_OP:
			selec = xy_position_sel(&nd_ibox, &nd_box, nd_stats, OVERBELOW_OP, Y_DIMS);
			return selec;
		case FRONT_OP:
			selec = z_position_sel(&nd_ibox, &nd_box, nd_stats, FRONT_OP, Z_DIMS);
			return selec;
		case BACK_OP:
			selec = z_position_sel(&nd_ibox, &nd_box, nd_stats, BACK_OP, Z_DIMS);
			return selec;
		case OVERFRONT_OP:
			selec = z_position_sel(&nd_ibox, &nd_box, nd_stats, OVERFRONT_OP, Z_DIMS);
			return selec;
		case OVERBACK_OP:
			selec = z_position_sel(&nd_ibox, &nd_box, nd_stats, OVERBEFORE_OP, Z_DIMS);
			return selec;
		case BEFORE_OP:
			selec = estimate_temporal_position_sel(root, *vardata, other, false, false, LT_OP);
			return selec;
		case AFTER_OP:
			selec = estimate_temporal_position_sel(root, *vardata, other, true, false, GT_OP);
			return selec;
		case OVERBEFORE_OP:
			selec = estimate_temporal_position_sel(root, *vardata, other, false, true, LE_OP);
			return selec;
		case OVERAFTER_OP:
			selec = 1.0 - estimate_temporal_position_sel(root, *vardata, other, false, false, GE_OP);
			return selec;
		default:
			return 0.001;
	}
}

/** Estimate the selectivity of time dimension */
Selectivity
estimate_selectivity_temporal_dimension(PlannerInfo *root, VariableStatData vardata, Node *other, Oid operator)
{
	Selectivity selec = 0.0;	/* keep compiler quiet */
	BBoxBounds bBoxBounds;
	bool numeric, temporal;
	double lower, upper;
	Period *period;

	get_const_bounds(other, &bBoxBounds, &numeric, &lower, &upper,
					 &temporal, &period);

	ConstantData constantData;
	constantData.bBoxBounds = bBoxBounds;
	constantData.oid = ((Const *) other)->consttype;

	constantData.lower = 0;constantData.upper = 0;  /* keep compiler quiet */
	constantData.period = NULL;   /* keep compiler quiet */
	if (temporal)
	{
		constantData.period = period;
	}
	switch (operator)
	{
		case OVERLAPS_OP:
			selec = estimate_temporal_bbox_sel(root, vardata, constantData, OVERLAPS_OP);
			break;
		case SAME_OP:
			selec = estimate_temporal_bbox_sel(root, vardata, constantData, SAME_OP);
			break;
		case CONTAINS_OP:
			selec = estimate_temporal_bbox_sel(root, vardata, constantData, CONTAINS_OP);
			break;
		case CONTAINED_OP:
			selec = estimate_temporal_bbox_sel(root, vardata, constantData, CONTAINED_OP);
			break;
		default:
			selec = 0.0;
	}

	if (selec < 0.0)
		selec = 0.1;

	return selec;
}

/** Estimate the selectivity for relative position x/y operators */
double
xy_position_sel(const ND_IBOX *nd_ibox, const ND_BOX *nd_box, const ND_STATS *nd_stats, CachedOp cacheOp, int mainDim)
{
	double total_count = 0.0;
	float cell_count, ratio;
	float8 selectivity;
	double cell_size[ND_DIMS];
	int at[ND_DIMS];
	double cellWidth, imin, imax, iwidth;
	int secondDim;
	if (mainDim == X_DIMS)
		secondDim = Y_DIMS;
	else
		secondDim = X_DIMS;

	/* Check the scope of the box with the extent */
	switch (cacheOp)
	{
		case LEFT_OP:
		case OVERLEFT_OP:
		case BELOW_OP:
		case OVERBELOW_OP:
		{
			if ((nd_stats->extent.min[mainDim] + 0.5) > nd_box->max[mainDim])
				return 0.0;
			else if (nd_box->min[mainDim] >= (nd_stats->extent.max[mainDim] - 0.5))
				return 1.0;
			break;
		}
		case RIGHT_OP:
		case OVERRIGHT_OP:
		case ABOVE_OP:
		case OVERABOVE_OP:
		{
			if (nd_stats->extent.max[mainDim] - 0.5 < nd_box->min[mainDim])
				return 0.0;
			else if (nd_box->max[mainDim] <= nd_stats->extent.min[mainDim] + 0.5)
				return 1.0;
			break;
		}
		default:
			break;
	}
	/* Work out some measurements of the histogram */
	/* Cell size in each dim */
	cell_size[mainDim] = (nd_stats->extent.max[mainDim] - nd_stats->extent.min[mainDim]) / nd_stats->size[mainDim];

	at[mainDim] = nd_ibox->min[mainDim];

	if (cacheOp == ABOVE_OP || cacheOp == OVERBELOW_OP	||
		cacheOp == RIGHT_OP || cacheOp == OVERLEFT_OP)
		at[secondDim] = nd_ibox->max[secondDim];
	else
		at[secondDim] = nd_ibox->min[secondDim];

	/* Loop through the rows and columns */
	for (int i = 0; i < nd_stats->size[mainDim]; i++)
	{
		at[mainDim] = i;
		ND_BOX nd_cell = { {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0} };

		/* We have to pro-rate partially overlapped cells. */
		nd_cell.min[mainDim] = (float4)(nd_stats->extent.min[mainDim] + (at[mainDim] + 0) * cell_size[mainDim]);
		nd_cell.max[mainDim] = (float4)(nd_stats->extent.min[mainDim] + (at[mainDim] + 1) * cell_size[mainDim]);
		if (mainDim == X_DIMS)
			cell_count = nd_stats->value[i * (int)nd_stats->size[1] + at[1]];
		else
			cell_count = nd_stats->value[at[0] * (int)nd_stats->size[1] + i];

		cellWidth = nd_cell.max[mainDim] - nd_cell.min[mainDim];
		imin = Max(nd_box->min[mainDim], nd_box->min[mainDim]);
		imax = Min(nd_cell.max[mainDim], nd_cell.max[mainDim]);
		iwidth = imax - imin;
		iwidth = Max(0.0, iwidth);

		if (cacheOp == ABOVE_OP || cacheOp == OVERABOVE_OP	||
			cacheOp == RIGHT_OP || cacheOp == OVERRIGHT_OP)
			ratio= (float)(iwidth / cellWidth);
		else
			ratio = 1.0f - (float)(iwidth / cellWidth);

		if (ratio > 1.0)
			ratio = (float)(cellWidth / iwidth);
		else if (ratio < 0.0)
			ratio = 0.0;

		/* Add the pro-rated count for this cell to the overall total */
		total_count += cell_count * ratio;

		/* Count the rest features */
		switch (cacheOp)
		{
			case LEFT_OP:
			case OVERLEFT_OP:
			case BELOW_OP:
			case OVERBELOW_OP:
			{
				if(mainDim == X_DIMS)
				{
					for (int j = at[1] - 1; j >= 0; j--)
					{
						cell_count = nd_stats->value[i * (int)nd_stats->size[1] + j];
						total_count += cell_count;
					}
				}
				else
				{
					for (int j = at[0] - 1; j >= 0; j--)
					{
						cell_count = nd_stats->value[j * (int)nd_stats->size[1] + i];
						total_count += cell_count;
					}
				}
				break;
			}
			case RIGHT_OP:
			case OVERRIGHT_OP:
			case ABOVE_OP:
			case OVERABOVE_OP:
			{
				if (mainDim == X_DIMS)
				{
					for (int j = at[1] + 1; j < nd_stats->size[secondDim]; j++)
					{
						cell_count = nd_stats->value[i * (int)nd_stats->size[1] + j];
						total_count += cell_count;
					}
				}
				else
				{
					for (int j = at[0] + 1; j < nd_stats->size[secondDim]; j++)
					{
						cell_count = nd_stats->value[j * (int)nd_stats->size[1] + i];
						total_count += cell_count;
					}
				}
				break;
			}
			default:
				break;
		}

	}
	/* Scale by the number of features in our histogram to get the proportion */
	selectivity = total_count / nd_stats->histogram_features;
	/* Prevent rounding overflows */
	if (selectivity > 1.0) selectivity = 1.0;
	else if (selectivity < 0.0) selectivity = 0.0;

	return selectivity;
}

/** Estimate the selectivity for relative position z operators */
double
z_position_sel(const ND_IBOX *nd_ibox, const ND_BOX *nd_box, const ND_STATS *nd_stats, CachedOp cacheOp, int dim)
{
	double total_count = 0.0;
	float cell_count, ratio;
	float8 selectivity;
	double cell_size[ND_DIMS];
	int at[ND_DIMS];
	double cellWidth, imin, imax, iwidth;

	/* Work out some measurements of the histogram */
	/* Cell size in each dim */
	cell_size[dim] = (nd_stats->extent.max[dim] - nd_stats->extent.min[dim]) / nd_stats->size[dim];

	at[dim] = nd_ibox->min[dim];

	if (cacheOp == FRONT_OP || cacheOp == OVERBACK_OP)
		at[1] = nd_ibox->max[1];
	else
		at[1] = nd_ibox->min[1];

	for (int i = 0; i < nd_stats->size[Z_DIMS]; i++)
	{
		at[Z_DIMS] = i;
		ND_BOX nd_cell = { {0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0} };

		/* We have to pro-rate partially overlapped cells. */
		nd_cell.min[Z_DIMS] = (float4)(nd_stats->extent.min[Z_DIMS] + (at[Z_DIMS] + 0) * cell_size[Z_DIMS]);
		nd_cell.max[Z_DIMS] = (float4)(nd_stats->extent.min[Z_DIMS] + (at[Z_DIMS] + 1) * cell_size[Z_DIMS]);
		cell_count = nd_stats->value[(at[X_DIMS] * (int)nd_stats->size[Y_DIMS] + at[Y_DIMS]) *
									 (int)nd_stats->size[Z_DIMS] + i];

		cellWidth = nd_cell.max[Z_DIMS] - nd_cell.min[Z_DIMS];
		imin = Max(nd_box->min[Z_DIMS], nd_box->min[Z_DIMS]);
		imax = Min(nd_cell.max[Z_DIMS], nd_cell.max[Z_DIMS]);
		iwidth = imax - imin;
		iwidth = Max(0.0, iwidth);

		if (cacheOp == FRONT_OP || cacheOp == OVERFRONT_OP)
			ratio= (float)(iwidth / cellWidth);
		else
			ratio = 1.0f - (float)(iwidth / cellWidth);

		if (ratio > 1.0)
			ratio = (float)(cellWidth / iwidth);
		else if (ratio < 0.0)
			ratio = 0.0;

		/* Add the pro-rated count for this cell to the overall total */
		total_count += cell_count * ratio;

		/* Count the rest features */
		if (cacheOp == BACK_OP || cacheOp == OVERBACK_OP)
		{
			for (int x = at[X_DIMS] - 1; x >= 0; x--)
			{
				for (int y = at[Y_DIMS] - 1; y >= 0; y--)
				{
					cell_count = nd_stats->value[(x * (int)nd_stats->size[Y_DIMS] + y) *
												 (int)nd_stats->size[Z_DIMS] + i];
					total_count += cell_count;
				}
			}
		}
		else if (cacheOp == FRONT_OP || cacheOp == OVERFRONT_OP)
		{
			for (int j = at[1] + 1; j < nd_stats->size[1]; j++)
			{
				cell_count = nd_stats->value[i * (int)nd_stats->size[1] + j];
				total_count += cell_count;
			}
		}
	}

	/* Scale by the number of features in our histogram to get the proportion */
	selectivity = total_count / nd_stats->histogram_features;
	/* Prevent rounding overflows */
	if (selectivity > 1.0) selectivity = 1.0;
	else if (selectivity < 0.0) selectivity = 0.0;

	return selectivity;
}

/** Get the name of the operator from different cases */
CachedOp
get_tpoint_cacheOp(Oid operator)
{
	for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
	{
		if (operator == oper_oid((CachedOp)i, T_STBOX, T_STBOX) ||
			operator == oper_oid((CachedOp)i, T_GEOMETRY, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp)i, T_STBOX, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp)i, T_TGEOMPOINT, T_GEOMETRY) ||
			operator == oper_oid((CachedOp)i, T_TGEOMPOINT, T_STBOX) ||
			operator == oper_oid((CachedOp)i, T_TGEOMPOINT, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp)i, T_GEOGRAPHY, T_TGEOGPOINT) ||
			operator == oper_oid((CachedOp)i, T_STBOX, T_TGEOGPOINT) ||
			operator == oper_oid((CachedOp)i, T_TGEOGPOINT, T_GEOGRAPHY) ||
			operator == oper_oid((CachedOp)i, T_TGEOGPOINT, T_STBOX) ||
			operator == oper_oid((CachedOp)i, T_TGEOGPOINT, T_TGEOGPOINT))
			return (CachedOp)i;
	}
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported")));
}

/** Set the values of #STBOX from #Node */
STBOX
get_stbox(Node *node)
{
	STBOX box;
	Oid value_type = ((Const *) node)->consttype;

	if (value_type == type_oid(T_TGEOMPOINT) ||
		value_type == type_oid(T_TGEOGPOINT))
	{
		Temporal *temp = DatumGetTemporal(((Const *) node)->constvalue);
		temporal_bbox(&box, temp);
		return *stbox_copy(&box);
	}
	else if (value_type == type_oid(T_GEOMETRY) || value_type == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(((Const *) node)->constvalue);
		if (geo_to_stbox_internal(&box, gs) == LW_FAILURE)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
					errmsg("Error while obtaining the bounding box of the geometry")));
		return box;
	}
	else if (value_type == type_oid(T_STBOX))
	{
		STBOX *boxi;
		STBOX *box = DatumGetSTboxP(((Const *) node)->constvalue);
		boxi = stbox_copy(box);
		return *boxi;
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Function get_stbox does not support this type")));
}

/**
* Return true if #ND_BOX a contains b, false otherwise.
*/
int
nd_box_contains(const ND_BOX *a, const ND_BOX *b, int ndims)
{
	int d;
	for (d = 0; d < ndims; d++)
	{
		if (!((a->min[d] < b->min[d]) && (a->max[d] > b->max[d])))
			return false;
	}
	return true;
}

/** Zero out an ND_BOX */
int
nd_box_init(ND_BOX *a)
{
	memset(a, 0, sizeof(ND_BOX));
	return true;
}

/** Set the values of an #ND_BOX from a #STBOX */
void
nd_box_from_stbox(const STBOX *box, ND_BOX *nd_box)
{
	int d = 0;

	nd_box_init(nd_box);
	nd_box->min[d] = box->xmin;
	nd_box->max[d] = box->xmax;
	d++;
	nd_box->min[d] = box->ymin;
	nd_box->max[d] = box->ymax;
	d++;
	if (FLAGS_GET_GEODETIC(box->flags))
	{
		nd_box->min[d] = box->zmin;
		nd_box->max[d] = box->zmax;
		return;
	}
	if (FLAGS_GET_Z(box->flags))
	{
		nd_box->min[d] = box->zmin;
		nd_box->max[d] = box->zmax;
		d++;
	}
	if (FLAGS_GET_M(box->flags))
	{
		nd_box->min[d] = box->tmin;
		nd_box->max[d] = box->tmax;
	}
	return;
}

/**
* Return true if #ND_BOX a overlaps b, false otherwise.
*/
int
nd_box_intersects(const ND_BOX *a, const ND_BOX *b, int ndims)
{
	int d;
	for (d = 0; d < ndims; d++)
	{
		if ((a->min[d] > b->max[d]) || (a->max[d] < b->min[d]))
			return false;
	}
	return true;
}

/**
* What stats cells overlap with this ND_BOX? Put the lowest cell
* addresses in ND_IBOX->min and the highest in ND_IBOX->max
*/
int
nd_box_overlap(const ND_STATS *nd_stats, const ND_BOX *nd_box, ND_IBOX *nd_ibox)
{
	int d;

	/* Initialize ibox */
	memset(nd_ibox, 0, sizeof(ND_IBOX));

	/* In each dimension... */
	for (d = 0; d < nd_stats->ndims; d++)
	{
		double smin = nd_stats->extent.min[d];
		double smax = nd_stats->extent.max[d];
		double width = smax - smin;
		int size = roundf(nd_stats->size[d]);

		/* ... find cells the box overlaps with in this dimension */
		nd_ibox->min[d] = floor(size * (nd_box->min[d] - smin) / width);
		nd_ibox->max[d] = floor(size * (nd_box->max[d] - smin) / width);

		/* Push any out-of range values into range */
		nd_ibox->min[d] = Max(nd_ibox->min[d], 0);
		nd_ibox->max[d] = Min(nd_ibox->max[d], size - 1);
	}
	return true;
}


/**
* Returns the proportion of b2 that is covered by b1.
*/
double
nd_box_ratio(const ND_BOX *b1, const ND_BOX *b2, int ndims)
{
	int d;
	bool covered = true;
	double ivol = 1.0;
	double vol2 = 1.0;
	double vol1 = 1.0;

	for (d = 0; d < ndims; d++)
	{
		if (b1->max[d] <= b2->min[d] || b1->min[d] >= b2->max[d])
			return 0.0; /* Disjoint */

		if (b1->min[d] > b2->min[d] || b1->max[d] < b2->max[d])
			covered = false;
	}

	if (covered)
		return 1.0;

	for (d = 0; d < ndims; d++)
	{
		double width1 = b1->max[d] - b1->min[d];
		double width2 = b2->max[d] - b2->min[d];
		double imin, imax, iwidth;

		vol1 *= width1;
		vol2 *= width2;

		imin = Max(b1->min[d], b2->min[d]);
		imax = Min(b1->max[d], b2->max[d]);
		iwidth = imax - imin;
		iwidth = Max(0.0, iwidth);

		ivol *= iwidth;
	}

	if (vol2 == 0.0)
		return vol2;

	return ivol / vol2;
}


/**
* Given a position in the n-d histogram (i,j,k) return the
* position in the 1-d values array.
*/
int
nd_stats_value_index(const ND_STATS *stats, int *indexes)
{
	int d;
	int accum = 1, vdx = 0;

	/* Calculate the index into the 1-d values array that the (i,j,k,l) */
	/* n-d histogram coordinate implies. */
	/* index = x + y * sizex + z * sizex * sizey + m * sizex * sizey * sizez */
	for (d = 0; d < (int) (stats->ndims); d++)
	{
		int size = (int) (stats->size[d]);
		if (indexes[d] < 0 || indexes[d] >= size)
		{
			return -1;
		}
		vdx += indexes[d] * accum;
		accum *= size;
	}
	return vdx;
}

/**
* Given an n-d index array (counter), and a domain to increment it
* in (ibox) increment it by one, unless it's already at the max of
* the domain, in which case return false.
*/
int
nd_increment(ND_IBOX *ibox, int ndims, int *counter)
{
	int d = 0;
	while (d < ndims)
	{
		if (counter[d] < ibox->max[d])
		{
			counter[d] += 1;
			break;
		}
		counter[d] = ibox->min[d];
		d++;
	}
	/* That's it, cannot increment any more! */
	if (d == ndims)
		return false;

	/* Increment complete! */
	return true;
}


/*****************************************************************************/
