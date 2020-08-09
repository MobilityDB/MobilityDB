/*****************************************************************************
 *
 * tpoint_selfuncs.c
 *		Functions for selectivity estimation of operators on temporal points
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_selfuncs.h"

#include <assert.h>
#include <float.h>

#include "period.h"
#include "temporal_selfuncs.h"
#include "stbox.h"
#include "tpoint.h"
#include "tpoint_analyze.h"
#include "tpoint_boxops.h"

/*****************************************************************************
 * PostGIS functions copied from the file gserialized_estimate.c since they
 * are not exported
 *****************************************************************************/

/**
 * Given a position in the n-d histogram (i,j,k,l) return the
 * position in the 1-d values array.
 */
static int
nd_stats_value_index(const ND_STATS *stats, const int *indexes)
{
	int d;
	int accum = 1, vdx = 0;

	/* Calculate the index into the 1-d values array that the (i,j,k,l) 
	 * n-d histogram coordinate implies. 
	 * index = x + y * sizex + z * sizex * sizey + m * sizex * sizey * sizez */
	for (d = 0; d < (int) (stats->ndims); d++)
	{
		int size = (int) (stats->size[d]);
		if (indexes[d] < 0 || indexes[d] >= size)
			return -1;
		vdx += indexes[d] * accum;
		accum *= size;
	}
	return vdx;
}

/*****************************************************************************
 * Boolean functions for the operators
 * PostGIS provides nd_box_overlap and nd_box_overlap which are copied in
 * tpoint_analyze.c
 *****************************************************************************/

/**
 * Returns true if a contains b, false otherwise.
 */
static int
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

/**
 * Returns true if a is strictly left of b, false otherwise.
 */
bool
nd_box_left(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[X_DIM] < b->min[X_DIM]);
}

/**
 * Returns true if a does not extend to right of b, false otherwise.
 */
bool
nd_box_overleft(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[X_DIM] <= b->max[X_DIM]);
}

/**
 * Returns true if a is strictly right of b, false otherwise.
 */
bool
nd_box_right(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[X_DIM] > b->max[X_DIM]);
}

/**
 * Returns true if a does not extend to left of b, false otherwise.
 */
bool
nd_box_overright(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[X_DIM] >= b->min[X_DIM]);
}

/**
 * Returns true if a is strictly below of b, false otherwise.
 */
bool
nd_box_below(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[Y_DIM] < b->min[Y_DIM]);
}

/**
 * Returns true if a does not extend above of b, false otherwise.
 */
bool
nd_box_overbelow(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[Y_DIM] <= b->max[Y_DIM]);
}

/**
 * Returns true if a is strictly above of b, false otherwise.
 */
bool
nd_box_above(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[Y_DIM] > b->max[Y_DIM]);
}

/**
 * Returns true if a does not extend below of b, false otherwise.
 */
bool
nd_box_overabove(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[Y_DIM] >= b->min[Y_DIM]);
}

/**
 * Returns true if a is strictly front of b, false otherwise.
 */
bool
nd_box_front(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[Z_DIM] < b->min[Z_DIM]);
}

/**
 * Returns true if a does not extend to the back of b, false otherwise.
 */
bool
nd_box_overfront(const ND_BOX *a, const ND_BOX *b)
{
	return (a->max[Z_DIM] <= b->max[Z_DIM]);
}

/**
 * Returns true if a strictly back of b, false otherwise.
 */
bool
nd_box_back(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[Z_DIM] > b->max[Z_DIM]);
}

/**
 * Returns true if a does not extend to the front of b, false otherwise.
 */
bool
nd_box_overback(const ND_BOX *a, const ND_BOX *b)
{
	return (a->min[Z_DIM] >= b->min[Z_DIM]);
}

/*****************************************************************************
 * Proportion functions for the operators
 * Function nd_box_ratio_overlaps is defined in tpoint_analyze.c and is
 * copied from PostGIS function nd_box_ratio
 *****************************************************************************/

/**
 * Returns the proportion of b2 that is left of b1.
 */
static double
nd_box_ratio_left(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overright(b2, b1))
		return 0.0; 
	else if (nd_box_left(b2, b1))
		return 1.0;

	/* b2 is partially to the left of b1 */
	delta = b1->min[X_DIM] - b2->min[X_DIM];
	width = b2->max[X_DIM] - b2->min[X_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overleft of b1.
 */
static double
nd_box_ratio_overleft(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_right(b2, b1))
		return 0.0; 
	else if (nd_box_overleft(b2, b1))
		return 1.0;

	/* b2 is partially to the right of b1 */
	delta = b2->max[X_DIM] - b1->max[X_DIM];
	width = b2->max[X_DIM] - b2->min[X_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is right of b1.
 */
static double
nd_box_ratio_right(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overleft(b2, b1))
		return 0.0; 
	else if (nd_box_right(b2, b1))
		return 1.0;

	/* b2 is partially to the right of b1 */
	delta = b2->max[X_DIM] - b1->max[X_DIM];
	width = b2->max[X_DIM] - b2->min[X_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overright of b1.
 */
static double
nd_box_ratio_overright(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_left(b2, b1))
		return 0.0; 
	else if (nd_box_overright(b2, b1))
		return 1.0;

	/* b2 is partially to the left of b1 */
	delta = b1->min[X_DIM] - b2->min[X_DIM];
	width = b2->max[X_DIM] - b2->min[X_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is below of b1.
 */
static double
nd_box_ratio_below(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overabove(b2, b1))
		return 0.0; 
	else if (nd_box_below(b2, b1))
		return 1.0;

	/* b2 is partially to the below of b1 */
	delta = b1->min[Y_DIM] - b2->min[Y_DIM];
	width = b2->max[Y_DIM] - b2->min[Y_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overbelow of b1.
 */
static double
nd_box_ratio_overbelow(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_above(b2, b1))
		return 0.0; 
	else if (nd_box_overbelow(b2, b1))
		return 1.0;

	/* b2 is partially to the above of b1 */
	delta = b2->max[Y_DIM] - b1->max[Y_DIM];
	width = b2->max[Y_DIM] - b2->min[Y_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is above of b1.
 */
static double
nd_box_ratio_above(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overbelow(b2, b1))
		return 0.0; 
	else if (nd_box_above(b2, b1))
		return 1.0;

	/* b2 is partially to the above of b1 */
	delta = b2->max[Y_DIM] - b1->max[Y_DIM];
	width = b2->max[Y_DIM] - b2->min[Y_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overabove of b1.
 */
static double
nd_box_ratio_overabove(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_below(b2, b1))
		return 0.0; 
	else if (nd_box_overabove(b2, b1))
		return 1.0;

	/* b2 is partially to the below of b1 */
	delta = b1->min[Y_DIM] - b2->min[Y_DIM];
	width = b2->max[Y_DIM] - b2->min[Y_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is front of b1. 
 */
static double
nd_box_ratio_front(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overback(b2, b1))
		return 0.0; 
	else if (nd_box_front(b2, b1))
		return 1.0;

	/* b2 is partially to the front of b1 */
	delta = b1->min[Z_DIM] - b2->min[Z_DIM];
	width = b2->max[Z_DIM] - b2->min[Z_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overfront of b1.
 */
static double
nd_box_ratio_overfront(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_back(b2, b1))
		return 0.0; 
	else if (nd_box_overfront(b2, b1))
		return 1.0;

	/* b2 is partially to the back of b1 */
	delta = b2->max[Z_DIM] - b1->max[Z_DIM];
	width = b2->max[Z_DIM] - b2->min[Z_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is back of b1.
 */
static double
nd_box_ratio_back(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_overfront(b2, b1))
		return 0.0; 
	else if (nd_box_back(b2, b1))
		return 1.0;

	/* b2 is partially to the back of b1 */
	delta = b2->max[Z_DIM] - b1->max[Z_DIM];
	width = b2->max[Z_DIM] - b2->min[Z_DIM];
	return delta / width;
}

/**
 * Returns the proportion of b2 that is overback of b1.
 */
static double
nd_box_ratio_overback(const ND_BOX *b1, const ND_BOX *b2)
{
	double delta, width;

	if (nd_box_front(b2, b1))
		return 0.0; 
	else if (nd_box_overback(b2, b1))
		return 1.0;

	/* b2 is partially to the front of b1 */
	delta = b1->min[Z_DIM] - b2->min[Z_DIM];
	width = b2->max[Z_DIM] - b2->min[Z_DIM];
	return delta / width;
}

/**
 * Dispatch function for the position operators
 */
static double
nd_box_ratio_position(const ND_BOX *b1, const ND_BOX *b2, CachedOp op)
{
	if (op == LEFT_OP)
		return nd_box_ratio_left(b1, b2);
	else if (op == OVERLEFT_OP)
		return nd_box_ratio_overleft(b1, b2);
	else if (op == RIGHT_OP)
		return nd_box_ratio_right(b1, b2);
	else if (op == OVERRIGHT_OP)
		return nd_box_ratio_overright(b1, b2);
	else if (op == BELOW_OP)
		return nd_box_ratio_below(b1, b2);
	else if (op == OVERBELOW_OP)
		return nd_box_ratio_overbelow(b1, b2);
	else if (op == ABOVE_OP)
		return nd_box_ratio_above(b1, b2);
	else if (op == OVERABOVE_OP)
		return nd_box_ratio_overabove(b1, b2);
	else if (op == FRONT_OP)
		return nd_box_ratio_front(b1, b2);
	else if (op == OVERFRONT_OP)
		return nd_box_ratio_overfront(b1, b2);
	else if (op == BACK_OP)
		return nd_box_ratio_back(b1, b2);
	else if (op == OVERBACK_OP)
		return nd_box_ratio_overback(b1, b2);
	return FALLBACK_ND_SEL; /* make compiler quiet */
}

/*****************************************************************************
 * Internal functions computing selectivity
 *****************************************************************************/

/**
 * Transform the constant into an STBOX 
 */
bool
tpoint_const_to_stbox(Node *other, STBOX *box)
{
	Oid consttype = ((Const *) other)->consttype;

	if (point_base_type(consttype))
		geo_to_stbox_internal(box,
			(GSERIALIZED *)PointerGetDatum(((Const *) other)->constvalue));
	else if (consttype == TIMESTAMPTZOID)
		timestamp_to_stbox_internal(box, 
			DatumGetTimestampTz(((Const *) other)->constvalue));
	else if (consttype == type_oid(T_TIMESTAMPSET))
		timestampset_to_stbox_internal(box, 
			DatumGetTimestampSet(((Const *) other)->constvalue));
	else if (consttype == type_oid(T_PERIOD))
		period_to_stbox_internal(box, 
			DatumGetPeriod(((Const *) other)->constvalue));
	else if (consttype == type_oid(T_PERIODSET))
		periodset_to_stbox_internal(box, 
			DatumGetPeriodSet(((Const *) other)->constvalue));
	else if (consttype == type_oid(T_STBOX))
		memcpy(box, DatumGetSTboxP(((Const *) other)->constvalue), sizeof(STBOX));
	else if (consttype == type_oid(T_TGEOMPOINT) || 
		consttype == type_oid(T_TGEOGPOINT))
		temporal_bbox(box, DatumGetTemporal(((Const *) other)->constvalue));
	else
		return false;
	return true;
}

/**
 * Set the values of an ND_BOX from an STBOX 
 * The function only takes into account the x, y, and z dimensions of the box?
 * and assumes that they exist. This is to be ensured by the calling function.
 */
static void
nd_box_from_stbox(const STBOX *box, ND_BOX *nd_box)
{
	int d = 0;

	nd_box_init(nd_box);
	nd_box->min[d] = (float4) box->xmin;
	nd_box->max[d] = (float4) box->xmax;
	d++;
	nd_box->min[d] = (float4) box->ymin;
	nd_box->max[d] = (float4) box->ymax;
	d++;
	if (MOBDB_FLAGS_GET_GEODETIC(box->flags) ||
		MOBDB_FLAGS_GET_Z(box->flags))
	{
		nd_box->min[d] = (float4) box->zmin;
		nd_box->max[d] = (float4) box->zmax;
	}
}

/**
 * Get the enum value associated to the operator
 */
static bool
tpoint_cachedop(Oid operator, CachedOp *cachedOp)
{
	for (int i = OVERLAPS_OP; i <= OVERAFTER_OP; i++)
	{
		if (operator == oper_oid((CachedOp) i, T_STBOX, T_STBOX) ||
			operator == oper_oid((CachedOp) i, T_GEOMETRY, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp) i, T_STBOX, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp) i, T_TGEOMPOINT, T_GEOMETRY) ||
			operator == oper_oid((CachedOp) i, T_TGEOMPOINT, T_STBOX) ||
			operator == oper_oid((CachedOp) i, T_TGEOMPOINT, T_TGEOMPOINT) ||
			operator == oper_oid((CachedOp) i, T_GEOGRAPHY, T_TGEOGPOINT) ||
			operator == oper_oid((CachedOp) i, T_STBOX, T_TGEOGPOINT) ||
			operator == oper_oid((CachedOp) i, T_TGEOGPOINT, T_GEOGRAPHY) ||
			operator == oper_oid((CachedOp) i, T_TGEOGPOINT, T_STBOX) ||
			operator == oper_oid((CachedOp) i, T_TGEOGPOINT, T_TGEOGPOINT))
			{
				*cachedOp = (CachedOp) i;
				return true;
			}
	}
	return false;
}

/**
 * Returns a default selectivity estimate for given operator, when we don't
 * have statistics or cannot use them for some reason.
 */
static double
default_tpoint_selectivity(CachedOp operator)
{
	switch (operator)
	{
		case OVERLAPS_OP:
			return 0.005;

		case CONTAINS_OP:
		case CONTAINED_OP:
			return 0.002;

		case SAME_OP:
			return 0.001;

		case LEFT_OP:
		case RIGHT_OP:
		case OVERLEFT_OP:
		case OVERRIGHT_OP:
		case ABOVE_OP:
		case BELOW_OP:
		case OVERABOVE_OP:
		case OVERBELOW_OP:
		case FRONT_OP:
		case BACK_OP:
		case OVERFRONT_OP:
		case OVERBACK_OP:
		case AFTER_OP:
		case BEFORE_OP:
		case OVERAFTER_OP:
		case OVERBEFORE_OP:
			/* these are similar to regular scalar inequalities */
			return DEFAULT_INEQ_SEL;

		default:
			/* all operators should be handled above, but just in case */
			return 0.001;
	}
}

/**
 * Returns an estimate of the selectivity of a spatiotemporal search box by
 * looking at data in the ND_STATS structure. The selectivity is a float in 
 * [0,1] that estimates the proportion of the rows in the table that will be 
 * returned as a result of the search box.
 *
 * To get our estimate, we need sum up the values * the proportion of each 
 * cell in the histogram that satisfies the operator wrt the search box, 
 * then divide by the number of features that generated the histogram.
 *
 * This function generalizes PostGIS function estimate_selectivity in file
 * gserialized_estimate.c
 */
static float8
calc_geo_selectivity(VariableStatData *vardata, const STBOX *box, CachedOp op)
{
	ND_STATS *nd_stats;
	AttStatsSlot sslot;	
	int d; /* counter */
	float8 selectivity;
	ND_BOX nd_box;
	ND_IBOX nd_ibox, search_ibox;
	int at[ND_DIMS];
	double cell_size[ND_DIMS];
	double min[ND_DIMS];
	double max[ND_DIMS];
	double total_count = 0.0;
	int ndims_max;
	/* 
	 * The statistics currently collected by PostGIS does not allow us to
	 * differentiate between the bounding box operators for computing the
	 * selectivity. 
	 */
	bool bboxop = (op == OVERLAPS_OP || op == CONTAINS_OP ||
		op == CONTAINED_OP || op == SAME_OP);

	/* Get statistics 
	 * Currently PostGIS does not set the associated staopN so we
	 * can pass InvalidOid */
	if (!(HeapTupleIsValid(vardata->statsTuple) &&
		  get_attstatsslot(&sslot, vardata->statsTuple, STATISTIC_KIND_ND, 
			InvalidOid, ATTSTATSSLOT_NUMBERS)))
		return -1;

	/* Clone the stats here so we can release the attstatsslot immediately */
	nd_stats = palloc(sizeof(float4) * sslot.nnumbers);
	memcpy(nd_stats, sslot.numbers, sizeof(float4) * sslot.nnumbers);

	free_attstatsslot(&sslot);		
	/* Calculate the number of common coordinate dimensions  on the histogram */
	ndims_max = (int) Max(nd_stats->ndims, MOBDB_FLAGS_GET_Z(box->flags) ? 3 : 2);

	/* Initialize nd_box. */
	nd_box_from_stbox(box, &nd_box);

	/* Full histogram extent op box is false? */
	if (bboxop)
	{
	 	if(! nd_box_intersects(&(nd_stats->extent), &nd_box, ndims_max))
			return 0.0;
	}
	else
	{
		if ((op == LEFT_OP && nd_box_overright(&(nd_stats->extent), &nd_box)) ||
			(op == OVERLEFT_OP && nd_box_right(&(nd_stats->extent), &nd_box)) ||
			(op == RIGHT_OP && nd_box_overleft(&(nd_stats->extent), &nd_box)) ||
			(op == OVERRIGHT_OP && nd_box_left(&(nd_stats->extent), &nd_box)) ||

			(op == BELOW_OP && nd_box_overabove(&(nd_stats->extent), &nd_box)) ||
			(op == OVERBELOW_OP && nd_box_above(&(nd_stats->extent), &nd_box)) ||
			(op == ABOVE_OP && nd_box_overbelow(&(nd_stats->extent), &nd_box)) ||
			(op == OVERABOVE_OP && nd_box_below(&(nd_stats->extent), &nd_box)) ||

			(op == FRONT_OP && nd_box_overback(&(nd_stats->extent), &nd_box)) ||
			(op == OVERFRONT_OP && nd_box_back(&(nd_stats->extent), &nd_box)) ||
			(op == BACK_OP && nd_box_overfront(&(nd_stats->extent), &nd_box)) ||
			(op == OVERBACK_OP && nd_box_front(&(nd_stats->extent), &nd_box)))
			return 0.0;
	}

	/* Full histogram extent op box is true? */
	if (bboxop)
	{
	 	if(! nd_box_contains(&(nd_stats->extent), &nd_box, ndims_max))
			return 1.0;
	}
	else
	{
		if ((op == LEFT_OP && nd_box_left(&(nd_stats->extent), &nd_box)) ||
			(op == OVERLEFT_OP && nd_box_overleft(&(nd_stats->extent), &nd_box)) ||
			(op == RIGHT_OP && nd_box_right(&(nd_stats->extent), &nd_box)) ||
			(op == OVERRIGHT_OP && nd_box_overright(&(nd_stats->extent), &nd_box)) ||

			(op == BELOW_OP && nd_box_below(&(nd_stats->extent), &nd_box)) ||
			(op == OVERBELOW_OP && nd_box_overbelow(&(nd_stats->extent), &nd_box)) ||
			(op == ABOVE_OP && nd_box_above(&(nd_stats->extent), &nd_box)) ||
			(op == OVERABOVE_OP && nd_box_overabove(&(nd_stats->extent), &nd_box)) ||

			(op == FRONT_OP && nd_box_front(&(nd_stats->extent), &nd_box)) ||
			(op == OVERFRONT_OP && nd_box_overfront(&(nd_stats->extent), &nd_box)) ||
			(op == BACK_OP && nd_box_back(&(nd_stats->extent), &nd_box)) ||
			(op == OVERBACK_OP && nd_box_overback(&(nd_stats->extent), &nd_box)))
			return 1.0;
	}
	
	/* Calculate the overlap of the box on the histogram */
	if (! nd_box_overlap(nd_stats, &nd_box, &nd_ibox))
	{
		return FALLBACK_ND_SEL;
	}

	/* Work out some measurements of the histogram */
	for (d = 0; d < nd_stats->ndims; d++)
	{
		/* Cell size in each dim */
		min[d] = nd_stats->extent.min[d];
		max[d] = nd_stats->extent.max[d];
		cell_size[d] = (max[d] - min[d]) / nd_stats->size[d];
	}

	/* Determine the cells to traverse */
	memset(&search_ibox, 0, sizeof(ND_IBOX));
	if (bboxop)
		/* Traverse only the cells that overlap the box */
		for (d = 0; d < nd_stats->ndims; d++)
		{
			search_ibox.min[d] = nd_ibox.min[d];
			search_ibox.max[d] = nd_ibox.max[d];
		}
	else
	{
		/* Initialize to traverse all the cells */
		for (d = 0; d < nd_stats->ndims; d++)
		{
			search_ibox.min[d] = 0;
			search_ibox.max[d] = (int) (nd_stats->size[d] - 1);
			/* Initialize the counter */
			at[d] = search_ibox.min[d];
		}
		/* Restrict the cells according to the position operator */
		if (op == LEFT_OP)
			search_ibox.max[X_DIM] = nd_ibox.min[X_DIM];
		else if (op == OVERLEFT_OP)
			search_ibox.max[X_DIM] = nd_ibox.max[X_DIM];
		else if (op == RIGHT_OP)
			search_ibox.min[X_DIM] = nd_ibox.max[X_DIM];
		else if (op == OVERRIGHT_OP)
			search_ibox.min[X_DIM] = nd_ibox.min[X_DIM];
		else if (op == BELOW_OP)
			search_ibox.max[Y_DIM] = nd_ibox.min[Y_DIM];
		else if (op == OVERBELOW_OP)
			search_ibox.max[Y_DIM] = nd_ibox.max[Y_DIM];
		else if (op == ABOVE_OP)
			search_ibox.min[Y_DIM] = nd_ibox.max[Y_DIM];
		else if (op == OVERABOVE_OP)
			search_ibox.min[Y_DIM] = nd_ibox.min[Y_DIM];
		else if (op == FRONT_OP)
			search_ibox.max[Z_DIM] = nd_ibox.min[Z_DIM];
		else if (op == OVERFRONT_OP)
			search_ibox.max[Z_DIM] = nd_ibox.max[Z_DIM];
		else if (op == BACK_OP)
			search_ibox.min[Z_DIM] = nd_ibox.max[Z_DIM];
		else if (op == OVERBACK_OP)
			search_ibox.min[Z_DIM] = nd_ibox.min[Z_DIM];
	}

	/* Initialize the counter */
	memset(at, 0, sizeof(int) * ND_DIMS);
	for (d = 0; d < nd_stats->ndims; d++)
		at[d] = search_ibox.min[d];

	/* Move through all the overlap values and sum them */
	do
	{
		float cell_count, ratio;
		ND_BOX nd_cell;
		memset(&nd_cell, 0, sizeof(ND_BOX));

		/* We have to pro-rate partially overlapped cells. */
		for (d = 0; d < nd_stats->ndims; d++)
		{
			nd_cell.min[d] = (float4) (min[d] + (at[d]+0) * cell_size[d]);
			nd_cell.max[d] = (float4) (min[d] + (at[d]+1) * cell_size[d]);
		}

		if (bboxop)
			ratio = (float4) (nd_box_ratio_overlaps(&nd_box, &nd_cell, (int) nd_stats->ndims));
		else 
			ratio = (float4) (nd_box_ratio_position(&nd_box, &nd_cell, op));
		cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

		/* Add the pro-rated count for this cell to the overall total */
		total_count += cell_count * ratio;
	}
	while (nd_increment(&search_ibox, (int) nd_stats->ndims, at));

	/* Scale by the number of features in our histogram to get the proportion */
	selectivity = total_count / nd_stats->histogram_features;

	/* Prevent rounding overflows */
	if (selectivity > 1.0) selectivity = 1.0;
	else if (selectivity < 0.0) selectivity = 0.0;

	return selectivity;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_sel);
/**
 * Estimate the join selectivity value of the operators for temporal points
 */
PGDLLEXPORT Datum
tpoint_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec;
	CachedOp cachedOp;
	STBOX constBox;
	Period constperiod;

	/*
	 * Get enumeration value associated to the operator
	 */
	bool found = tpoint_cachedop(operator, &cachedOp);
	/* In the case of unknown operator */
	if (!found)
		PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);

	/*
	 * If expression is not (variable op something) or (something op
	 * variable), then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(default_tpoint_selectivity(cachedOp));

	/*
	 * Can't do anything useful if the something is not a constant, either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(default_tpoint_selectivity(cachedOp));
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
			/* Use default selectivity (should we raise an error instead?) */
			ReleaseVariableStats(vardata);
			PG_RETURN_FLOAT8(default_tpoint_selectivity(cachedOp));
		}
	}

	/* 
	 * Transform the constant into an STBOX
	 */
	memset(&constBox, 0, sizeof(STBOX));
	found = tpoint_const_to_stbox(other, &constBox);
	/* In the case of unknown constant */
	if (!found)
		PG_RETURN_FLOAT8(default_tpoint_selectivity(cachedOp));

	assert(MOBDB_FLAGS_GET_X(constBox.flags) || MOBDB_FLAGS_GET_T(constBox.flags));
	
	/* Enable the multiplication of the selectivity of the spatial and time 
	 * dimensions since either may be missing */
	selec = 1.0; 
	
	/*
	 * Estimate selectivity for the spatial dimension
	 */
	if (MOBDB_FLAGS_GET_X(constBox.flags))
	{
		selec *= calc_geo_selectivity(&vardata, &constBox, cachedOp);
	}
	/*
	 * Estimate selectivity for the time dimension
	 */
	if (MOBDB_FLAGS_GET_T(constBox.flags))
	{
		/* Transform the STBOX into a Period */
		period_set(&constperiod, constBox.tmin, constBox.tmax, true, true);
		TDuration duration = TYPMOD_GET_DURATION(vardata.atttypmod);
		ensure_valid_duration_all(duration);

		/* Dispatch based on duration */
		if (duration == INSTANT)
			selec *= tinstant_sel(root, &vardata, &constperiod, cachedOp);
		else
			selec *= tsequenceset_sel(root, &vardata, &constperiod, cachedOp);
	}

	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tpoint_joinsel);
/**
 * Estimate the join selectivity value of the operators for temporal points
 */
PGDLLEXPORT Datum
tpoint_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);
}

/*****************************************************************************/
