/*****************************************************************************
 *
 * tnumber_selfuncs.c
 *	  Functions for selectivity estimation of operators on temporal numeric types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	These functions are only stubs, they need to be written TODO
 *
 *****************************************************************************/

#include "tnumber_selfuncs.h"

#include <assert.h>
#include <access/htup_details.h>
#include <nodes/relation.h>
#include <utils/selfuncs.h>
#include <temporal_boxops.h>

#include "period.h"
#include "rangetypes_ext.h"
#include "oidcache.h"
#include "tbox.h"
#include "time_selfuncs.h"
#include "temporal_selfuncs.h"

/*****************************************************************************/

/*
 * Binary search on an array of range bounds. Returns greatest index of range
 * bound in array which is less(less or equal) than given range bound. If all
 * range bounds in array are greater or equal(greater) than given range bound,
 * return -1. When "equal" flag is set conditions in brackets are used.
 *
 * This function is used in scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop
 * interpolation of portion of bounds which are less or equal to given bound.
 * 
 * Functions adapted from PostgreSQL file rangetypes_selfuncs.c since they 
 * are not exported.
 */
static int
rbound_bsearch(TypeCacheEntry *typcache, Datum value, RangeBound *hist,
			   int hist_length, bool equal)
{
	int lower = -1, upper = hist_length - 1, cmp, middle;

	while (lower < upper)
	{
		middle = (lower + upper + 1) / 2;
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  (&hist[middle])->val, value));
		if (cmp < 0 || (equal && cmp == 0))
			lower = middle;
		else
			upper = middle - 1;
	}
	return lower;
}

/*
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a given const in a histogram of range bounds.
 */
static Selectivity
calc_hist_selectivity_scalar(TypeCacheEntry *typcache, Datum constbound,
							 RangeBound *hist, int hist_nvalues, bool equal)
{
	Selectivity selec;
	int index;

	/*
	* Find the histogram bin the given constant falls into. Estimate
	* selectivity as the number of preceding whole bins.
	*/
	index = rbound_bsearch(typcache, constbound, hist, hist_nvalues, equal);
	selec = (Selectivity) (Max(index, 0)) / (Selectivity) (hist_nvalues - 1);

	/* Adjust using linear interpolation within the bin */
	if (index >= 0 && index < hist_nvalues - 1)
	{
		float8 bin_width, position;
		bin_width = DatumGetFloat8(FunctionCall2Coll(
				&typcache->rng_subdiff_finfo,
				typcache->rng_collation,
				hist[index + 1].val,
				hist[index].val));
		if (bin_width <= 0.0)
			return 0.5;			/* zero width bin */

		position = DatumGetFloat8(FunctionCall2Coll(
				&typcache->rng_subdiff_finfo,
				typcache->rng_collation,
				constbound,
				hist[index].val)) / bin_width;
		/* Relative position must be in [0,1] range */
		position = Max(position, 0.0);
		position = Min(position, 1.0);
		selec += position / (Selectivity) (hist_nvalues - 1);
	}

	return selec;
}

/*
 * Calculate range operator selectivity using histograms of range bounds.
 *
 * This estimate is for the portion of values that are not empty and not NULL.
 */
static Selectivity
calc_range_hist_selectivity(VariableStatData *vardata, Datum constval,
							TypeCacheEntry *typcache, bool isgt, bool iseq, StatStrategy strategy)
{
	int nhist;
	RangeBound *hist_lower;
	RangeBound *hist_upper;
	int i;
	bool empty;
	Selectivity hist_selec = -1; /* keep compiler quiet */
	AttStatsSlot hslot;
	int staKind = STATISTIC_KIND_BOUNDS_HISTOGRAM;

	if (vardata->atttypmod == TEMPORALINST)
		staKind = STATISTIC_KIND_HISTOGRAM;
	/* Try to get histogram of ranges */
	if (!(HeapTupleIsValid(vardata->statsTuple) &&
		  get_attstatsslot_mobdb(&hslot, vardata->statsTuple,
								 staKind, InvalidOid,
								 ATTSTATSSLOT_VALUES, strategy)))
		return -1.0;

	/*
	 * Convert histogram of ranges into histograms of its lower and upper
	 * bounds.
	 */
	nhist = hslot.nvalues;
	hist_lower = (RangeBound *) palloc(sizeof(RangeBound) * nhist);
	hist_upper = (RangeBound *) palloc(sizeof(RangeBound) * nhist);

	for (i = 0; i < nhist; i++)
	{
		range_deserialize(typcache, DatumGetRangeTypeP(hslot.values[i]),
						  &hist_lower[i], &hist_upper[i], &empty);
		/* The histogram should not contain any empty ranges */
		if (empty)
			elog(ERROR, "bounds histogram contains an empty range");
	}

	if (!isgt && !iseq)
		hist_selec = calc_hist_selectivity_scalar(typcache, constval, hist_upper, nhist, false);
	else if (isgt && iseq)
		hist_selec = 1 - calc_hist_selectivity_scalar(typcache, constval, hist_lower, nhist, true);
	else if (isgt)
		hist_selec = 1 - calc_hist_selectivity_scalar(typcache, constval, hist_lower, nhist, false);
	else if (iseq)
		hist_selec = calc_hist_selectivity_scalar(typcache, constval, hist_upper, nhist, true);


	free_attstatsslot(&hslot);

	return hist_selec;
}

static Selectivity
range_sel_internal(VariableStatData *vardata, Datum constval,
				   bool isgt, bool iseq, TypeCacheEntry *typcache, StatStrategy strategy)
{
	double hist_selec;
	Selectivity selec;
	float4 empty_frac, null_frac;

	/*
	 * First look up the fraction of NULLs and empty ranges from pg_statistic.
	 */
	if (HeapTupleIsValid(vardata->statsTuple))
	{
		Form_pg_statistic stats;
		stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);
		null_frac = stats->stanullfrac;
		empty_frac = 0.0;
	}
	else
	{
		/*
		 * No stats are available. Follow through the calculations below
		 * anyway, assuming no NULLs and no empty ranges. This still allows us
		 * to give a better-than-nothing estimate based on whether the
		 * constant is an empty range or not.
		 */
		null_frac = 0.0;
		empty_frac = 0.0;
	}
	hist_selec = calc_range_hist_selectivity(vardata, constval, typcache, isgt, iseq, strategy);
	selec = (1.0 - empty_frac) * hist_selec;
	selec *= (1.0 - null_frac);
	return selec;
}

/*****************************************************************************
 * Internal functions computing selectivity
 *****************************************************************************/

static Selectivity
tnumber_bbox_sel(PlannerInfo *root, VariableStatData vardata, TBOX box, CachedOp cachedOp)
{
	// Check the temporal types and inside each one check the cachedOp
	Selectivity  selec = 0.0;
	int duration = TYPMOD_GET_DURATION(vardata.atttypmod);
	if (vardata.vartype == type_oid(T_TINT) || vardata.vartype == type_oid(T_TFLOAT))
	{
		CachedType vartype = (vardata.vartype == type_oid(T_TINT)) ? T_INT4 : T_FLOAT8;
		CachedType varRangeType = (vardata.vartype == type_oid(T_TINT)) ? T_INTRANGE : T_FLOATRANGE;
		/*
		 * Compute the selectivity with regard to the value of the constant.
		 */
		double selec1 = 0.0;
		if (MOBDB_FLAGS_GET_X(box.flags))
		{
			if (duration == TEMPORALINST)
			{
				Oid op = oper_oid(EQ_OP, vartype, vartype);
				selec1 = var_eq_const_mobdb(&vardata, op, (Datum) box.xmin, false, VALUE_STATISTICS);
			}
			else
			{
				if (cachedOp == OVERLAPS_OP)
				{
					TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
					selec1 = range_sel_internal(&vardata, (Datum) box.xmin, false, false, typcache,
												VALUE_STATISTICS);
					selec1 += range_sel_internal(&vardata, (Datum) box.xmax, true, false, typcache,
												 VALUE_STATISTICS);
				}
				else if (cachedOp == CONTAINS_OP)
				{
					TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
					selec1 = range_sel_internal(&vardata, (Datum) box.xmin, false, false, typcache,
												VALUE_STATISTICS);
					selec1 += range_sel_internal(&vardata, (Datum) box.xmax, false, false, typcache,
												 VALUE_STATISTICS);
				}
				else if (cachedOp == CONTAINED_OP)
				{
					Oid opl = oper_oid(LT_OP, vartype, vartype);
					Oid opg = oper_oid(GT_OP, vartype, vartype);
					selec1 = scalarineqsel_mobdb(root, opl, false, true, &vardata, (Datum) box.xmin,
												 type_oid(vartype), VALUE_STATISTICS);
					selec1 += scalarineqsel_mobdb(root, opg, true, false, &vardata, (Datum) box.xmax,
												  type_oid(vartype), VALUE_STATISTICS);
				}
				selec1 = 1 - selec1;
				selec1 = selec1 < 0 ? 0 : selec1;
			}
		}
		/*
		 * Compute the selectivity with regard to the time dimension of the constant.
		 */
		double selec2 = 0.0;
		if (MOBDB_FLAGS_GET_T(box.flags))
		{
			if (duration == TEMPORALINST)
			{
				Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
				selec2 = var_eq_const_mobdb(&vardata, op, (Datum) box.tmin,
									  false, TEMPORAL_STATISTICS);
			}
			else
			{
				if (cachedOp == SAME_OP || cachedOp == CONTAINS_OP)
				{
					Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
					selec2 = var_eq_const_mobdb(&vardata, op, (Datum) box.tmin, false,
										  TEMPORAL_STATISTICS);
					selec2 *= var_eq_const_mobdb(&vardata, op, (Datum) box.tmax, false,
										   TEMPORAL_STATISTICS);
					selec2 = selec2 > 1 ? 1 : selec2;
				}
				else
				{
					selec2 = calc_period_hist_selectivity(&vardata, period_make(box.tmin, box.tmax, true, true),
											cachedOp, TEMPORAL_STATISTICS);
				}
			}
		}
		if (MOBDB_FLAGS_GET_X(box.flags) && MOBDB_FLAGS_GET_T(box.flags))
			selec = selec1 * selec2;
		else if (MOBDB_FLAGS_GET_X(box.flags))
			selec = selec1;
		else if (MOBDB_FLAGS_GET_T(box.flags))
			selec = selec2;
	}
	return selec;
}


static double
lower_or_higher_value_bound(Node *other, bool higher)
{
	double result = 0.0;
	Oid consttype = ((Const *) other)->consttype;
	if (higher)
	{
		if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
		{
			Temporal *temporal = DatumGetTemporal(((Const *) other)->constvalue);
			TBOX box;
			memset(&box, 0, sizeof(TBOX));
			temporal_bbox(&box, temporal);
			result = box.xmax;
		}
		else if (consttype == type_oid(T_INT4) || consttype == type_oid(T_FLOAT8))
		{
			result = (double) ((Const *) other)->constvalue;
		}
		else if (consttype == type_oid(T_INTRANGE))
		{
			result = DatumGetInt32(upper_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		}
		else if (consttype == type_oid(T_FLOATRANGE))
		{
			result = DatumGetFloat8(upper_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		}
		else if (consttype == type_oid(T_TBOX))
		{
			TBOX *box = DatumGetTboxP(((Const *) other)->constvalue);
			assert(MOBDB_FLAGS_GET_X(box->flags));
			result = box->xmax;
		}
	}
	else
	{
		if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
		{
			Temporal *temporal = DatumGetTemporal(((Const *) other)->constvalue);
			TBOX box;
			memset(&box, 0, sizeof(TBOX));
			temporal_bbox(&box, temporal);
			result = box.xmin;
		}
		else if (consttype == type_oid(T_INT4) || consttype == type_oid(T_FLOAT8))
		{
			result = (double) ((Const *) other)->constvalue;
		}
		else if (consttype == type_oid(T_INTRANGE))
		{
			result = DatumGetInt32(lower_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		}
		else if (consttype == type_oid(T_FLOATRANGE))
		{
			result = DatumGetFloat8(lower_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		}
		else if (consttype == type_oid(T_TBOX))
		{
			TBOX *box = DatumGetTboxP(((Const *) other)->constvalue);
			assert(MOBDB_FLAGS_GET_X(box->flags));
			result = box->xmin;
		}
	}
	return result;
}

static Selectivity
tnumber_position_sel(VariableStatData vardata,
							  Node *other, bool isgt, bool iseq)
{
	double selec = 0.0;
	if (vardata.vartype == type_oid(T_TINT) || vardata.vartype == type_oid(T_TFLOAT))
	{
		TypeCacheEntry *typcache;
		if (vardata.vartype == type_oid(T_TINT))
			typcache = lookup_type_cache(type_oid(T_INTRANGE),
										 TYPECACHE_RANGE_INFO);
		else
			typcache = lookup_type_cache(type_oid(T_FLOATRANGE),
										 TYPECACHE_RANGE_INFO);
		selec = range_sel_internal(&vardata, (Datum)lower_or_higher_value_bound(other, isgt),
								   isgt, iseq, typcache, VALUE_STATISTICS);
	}
	return selec;
}

/*****************************************************************************
 * Internal functions computing selectivity
 *****************************************************************************/

static bool
tnumber_const_to_period(Node *other, TBOX *box)
{
    Oid consttype = ((Const *) other)->consttype;

    if (consttype == INT4OID)
        int_to_tbox_internal(box, ((Const *) other)->constvalue);
    else if (consttype == FLOAT8OID)
        float_to_tbox_internal(box, ((Const *) other)->constvalue);
    else if (consttype == type_oid(T_INTRANGE))
        intrange_to_tbox_internal(box, DatumGetRangeTypeP(((Const *) other)->constvalue));
    else if (consttype == type_oid(T_FLOATRANGE))
        floatrange_to_tbox_internal(box, DatumGetRangeTypeP(((Const *) other)->constvalue));
    else if (consttype == TIMESTAMPTZOID)
        timestamp_to_tbox_internal(box, DatumGetTimestampTz(((Const *) other)->constvalue));
    else if (consttype == type_oid(T_TIMESTAMPSET))
        timestampset_to_tbox_internal(box, ((TimestampSet *)((Const *) other)->constvalue));
    else if (consttype == type_oid(T_TGEOMPOINT) || consttype == type_oid(T_TGEOGPOINT) ||
             consttype == type_oid(T_PERIOD))
        period_to_tbox_internal(box, (Period *) ((Const *) other)->constvalue);
    else if (consttype == type_oid(T_PERIODSET))
        periodset_to_tbox_internal(box, ((PeriodSet *)((Const *) other)->constvalue));
    else if (consttype == type_oid(T_TBOX))
        memcpy(box, DatumGetTboxP(((Const *) other)->constvalue), sizeof(TBOX));
    else if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
        temporal_bbox(box, DatumGetTemporal(((Const *) other)->constvalue));
    else
        return false;
    return true;
}

/* Get the name of the operator from different cases */
static bool
tnumber_cachedop(Oid operator, CachedOp *cachedOp)
{
	for (int i = LT_OP; i <= OVERAFTER_OP; i++)
	{
		if (operator == oper_oid((CachedOp) i, T_INTRANGE, T_TINT) ||
			operator == oper_oid((CachedOp) i, T_TBOX, T_TINT) ||
			operator == oper_oid((CachedOp) i, T_TINT, T_INTRANGE) ||
			operator == oper_oid((CachedOp) i, T_TINT, T_TBOX) ||
			operator == oper_oid((CachedOp) i, T_TINT, T_TINT) ||
			operator == oper_oid((CachedOp) i, T_TINT, T_TFLOAT) ||
			operator == oper_oid((CachedOp) i, T_FLOATRANGE, T_TFLOAT) ||
			operator == oper_oid((CachedOp) i, T_TBOX, T_TFLOAT) ||
			operator == oper_oid((CachedOp) i, T_TFLOAT, T_FLOATRANGE) ||
			operator == oper_oid((CachedOp) i, T_TFLOAT, T_TBOX) ||
			operator == oper_oid((CachedOp) i, T_TFLOAT, T_TINT) ||
			operator == oper_oid((CachedOp) i, T_TFLOAT, T_TFLOAT))
            {
                *cachedOp = (CachedOp) i;
                return true;
            }
	}
	return false;
}

/*
 * Returns a default selectivity estimate for given operator, when we don't
 * have statistics or cannot use them for some reason.
 */
static double
default_tnumber_selectivity(CachedOp operator)
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

/* The selectivity functions assume that the value and time dimensions of
 * temporal values are independent and thus the selectivity values obtained
 * by analyzing the histograms for each dimension can be multiplied */
Selectivity
tnumberinst_sel(PlannerInfo *root, VariableStatData *vardata,
	TBOX *box, CachedOp cachedOp)
{
	double selec; 
	Oid op, valuetypid;

	valuetypid = base_oid_from_temporal(vardata->atttype);
	numeric_base_type_oid(valuetypid);
	assert(MOBDB_FLAGS_GET_X(box->flags) || MOBDB_FLAGS_GET_T(box->flags));
	if (cachedOp == SAME_OP || cachedOp == CONTAINS_OP)
	{
		/* If the box is not equivalent to a temporal instant return 0.0 */
		if (box->xmin != box->xmax || box->tmin != box->tmax)
			return 0.0;

		/* Enable the multiplication of the selectivity of the value and time 
		 * dimensions since either may be missing*/
		selec = 1.0; 

		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			op = oper_oid(EQ_OP, valuetypid, valuetypid);
			selec *= var_eq_const_mobdb(vardata, op, 
			Float8GetDatum(box->xmin), false, VALUE_STATISTICS);
		}
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			op *= oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
			selec *= var_eq_const_mobdb(vardata, op, 
				TimestampTzGetDatum(box->tmin), false, TEMPORAL_STATISTICS);
		}
	}
	else if (cachedOp == CONTAINED_OP || cachedOp == OVERLAPS_OP)
	{
		/* 
		 * For TemporalInst, the two conditions v@t <@ TBOX b and
		 * v@t && TBOX b are equivalent. Then
		 * 
		 * v@t <@ TBOX b <=> 
		 * 		box->xmin <= v AND v <= box->xmax AND box->tmin <= t AND t <= box->tmax <=> 
		 * 		NOT (box->xmin > v OR v > box->xmax OR box->tmin > t AND t > box->tmax) <=> 
		 * 		NOT (v < box->xmin OR v > box->xmax OR t < box->tmin AND t > box->tmax)
		 *
		 * Since the components of v < box->xmin OR ... are mutually exclusive 
		 * events we can sum their probabilities to find probability of 
		 * v < box->xmin OR ... 
		 */

		/* Enable the addition of the selectivity of the value and time 
		 * dimensions since either may be missing*/
		selec = 0.0; 

		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			op = oper_oid(LT_OP, valuetypid, valuetypid);
			selec = scalarineqsel_mobdb(root, op, false, false, vardata, 
				Float8GetDatum(box->xmin), valuetypid, VALUE_STATISTICS);
			op = oper_oid(GT_OP, valuetypid, valuetypid);
			selec += scalarineqsel_mobdb(root, op, true, false, vardata, 
				Float8GetDatum(box->xmax), valuetypid, VALUE_STATISTICS);
		}
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			op = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
			selec += scalarineqsel_mobdb(root, op, false, false, vardata, 
				TimestampTzGetDatum(box->tmin), TIMESTAMPTZOID, TEMPORAL_STATISTICS);
			op = oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
			selec += scalarineqsel_mobdb(root, op, true, false,  vardata, 
				TimestampTzGetDatum(box->tmax), TIMESTAMPTZOID, TEMPORAL_STATISTICS);
		}
		selec = 1 - selec;
	}
	else if (cachedOp == LEFT_OP)
	{
		/* TemporalInst v@t << TBOX b <=> v < box->xmin */
		op = oper_oid(LT_OP, valuetypid, valuetypid);
		selec = scalarineqsel_mobdb(root, op, false, false, vardata, 
			box->xmin, valuetypid, VALUE_STATISTICS);
	}
	else if (cachedOp == RIGHT_OP)
	{
		/* TemporalInst v@t >> TBOX b <=> v > box->xmax */
		op = oper_oid(GT_OP, valuetypid, valuetypid);
		selec = scalarineqsel_mobdb(root, op, true, false, vardata, 
			box->xmax, valuetypid, VALUE_STATISTICS);
	}
	else if (cachedOp == OVERLEFT_OP)
	{
		/* TemporalInst v@t &< TBOX b <=> v <= box->xmax */
		op = oper_oid(LE_OP, valuetypid, valuetypid);
		selec = scalarineqsel_mobdb(root, op, false, true, vardata, 
			box->xmax, valuetypid, VALUE_STATISTICS);
	}
	else if (cachedOp == OVERRIGHT_OP)
	{
		/* TemporalInst v@t &> TBOX b <=> v >= box->xmin */
		op = oper_oid(GE_OP, valuetypid, valuetypid);
		selec = scalarineqsel_mobdb(root, op, true, true, vardata, 
			box->xmin, valuetypid, VALUE_STATISTICS);
	}
	else if (cachedOp == BEFORE_OP)
	{
		/* TemporalInst v@t <<# TBOX b <=> t < box->tmin */
		op = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel_mobdb(root, op, false, false, vardata, 
			box->tmin, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
	}
	else if (cachedOp == AFTER_OP)
	{
		/* TemporalInst v@t #>> TBOX b <=> t > box->tmax */
		op = oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel_mobdb(root, op, true, false, vardata, 
			box->tmax, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
	}
	else if (cachedOp == OVERBEFORE_OP)
	{
		/* TemporalInst v@t &<# TBOX b <=> t <= box->tmax */
		op = oper_oid(LE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel_mobdb(root, op, false, true, vardata, 
			box->tmax, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
	}
	else if (cachedOp == OVERAFTER_OP)
	{
		/* TemporalInst v@t #&> TBOX b <=> t >= box->tmin */
		op = oper_oid(GE_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		selec = scalarineqsel_mobdb(root, op, true, true, vardata, 
			box->tmin, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
	}
	/* For b-tree comparisons, temporal values are first compared wrt 
	 * their bounding boxes, and if these are equal, other criteria apply.
	 * For selectivity estimation we approximate by taking into account
	 * only the bounding boxes. In the case here we use the scalar 
	 * inequality selectivity */
	else if (cachedOp == LT_OP || cachedOp == LE_OP)
	{
		/* TemporalInst v@t < TBOX b <=> v < box->xmin AND t < box->tmin */

		/* Enable the multiplication of the selectivity of the value and time 
		 * dimensions since either may be missing*/
		selec = 1.0; 

		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			op = oper_oid(LT_OP, valuetypid, valuetypid);
			selec *= scalarineqsel_mobdb(root, op, false, cachedOp == LT_OP, 
				vardata, box->xmin, valuetypid, VALUE_STATISTICS);
		}
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			op = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
			selec *= scalarineqsel_mobdb(root, op, false, cachedOp == LT_OP, 
				vardata, box->tmin, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
		}
	}
	else if (cachedOp == GT_OP || cachedOp == GE_OP)
	{
		/* TemporalInst v@t > TBOX b <=> v > box->xmax AND t > box->tmax */

		/* Enable the multiplication of the selectivity of the value and time 
		 * dimensions since either may be missing*/
		selec = 1.0; 

		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			op = oper_oid(GT_OP, valuetypid, valuetypid);
			selec *= scalarineqsel_mobdb(root, op, true, cachedOp == GT_OP, 
				vardata, box->xmax, valuetypid, VALUE_STATISTICS);
		}
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			op = oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
			selec *= scalarineqsel_mobdb(root, op, true, cachedOp == GT_OP, 
				vardata, box->tmax, TIMESTAMPTZOID, TEMPORAL_STATISTICS);
		}
	}
	else /* Unknown operator */
	{
		selec = default_tnumber_selectivity(cachedOp);
	}
	return selec;
}

Selectivity
tnumberi_sel(PlannerInfo *root, VariableStatData *vardata,
	TBOX *box, CachedOp cachedOp)
{
	double selec = 0.0;
	/* TODO */
	selec = default_tnumber_selectivity(cachedOp);
	return selec;	
}

Selectivity
tnumbers_sel(PlannerInfo *root, VariableStatData *vardata,
	TBOX *box, CachedOp cachedOp)
{
	Period period;
	RangeType *range;
	TypeCacheEntry *typcache;
	double selec;
	Oid op, valuetypid, rangetypid;

	/* Enable the multiplication of the selectivity of the value and time 
	 * dimensions since either may be missing */
	selec = 1.0; 

	valuetypid = base_oid_from_temporal(vardata->atttype);
	numeric_base_type_oid(valuetypid);
	if (MOBDB_FLAGS_GET_X(box->flags))
	{
		range = range_make(Float8GetDatum(box->xmin), 
			Float8GetDatum(box->xmax), true, true, valuetypid);
		rangetypid = range_oid_from_base(valuetypid);		
		typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);
	}
	if (MOBDB_FLAGS_GET_T(box->flags))
		tbox_to_period(&period, box);

	/*
	 * There is no ~= operator for range/time types and thus it is necessary to
	 * take care of this operator here.
	 */
	if (cachedOp == SAME_OP)
	{
		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			op = oper_oid(EQ_OP, valuetypid, valuetypid);
			selec *= var_eq_const_mobdb(vardata, op, RangeTypePGetDatum(range),
				false, VALUE_STATISTICS);
		}
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			op = oper_oid(EQ_OP, T_PERIOD, T_PERIOD);
			selec *= var_eq_const_mobdb(vardata, op, PeriodGetDatum(&period),
			false, TEMPORAL_STATISTICS);
		}
	}
	else if (cachedOp == OVERLAPS_OP || cachedOp == CONTAINS_OP ||
		cachedOp == CONTAINED_OP || cachedOp == BEFORE_OP ||
		cachedOp == AFTER_OP || cachedOp == OVERBEFORE_OP || 
		cachedOp == OVERAFTER_OP) 
	{
		/* Enable the multiplication of the selectivity of the value and time 
		 * dimensions since either may be missing*/
		selec = 1.0; 

		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
			selec *= calc_range_hist_selectivity(typcache, vardata, range, cachedOp, 
				VALUE_STATISTICS);
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
			selec *= calc_period_hist_selectivity(vardata, &period, cachedOp, 
				TEMPORAL_STATISTICS);
	}
	else if (cachedOp == LT_OP || cachedOp == LE_OP || 
		cachedOp == GT_OP || cachedOp == GE_OP) 
	{
		/* For b-tree comparisons, temporal values are first compared wrt 
		 * their bounding boxes, and if these are equal, other criteria apply.
		 * For selectivity estimation we approximate by taking into account
		 * only the bounding boxes. In the case here the bounding box is a
		 * period and thus we can use the period selectivity estimation */
		/* Selectivity for the value dimension */
		if (MOBDB_FLAGS_GET_X(box->flags))
			selec = calc_range_hist_selectivity(typcache, vardata, range, cachedOp, 
				VALUE_STATISTICS);
		/* Selectivity for the time dimension */
		if (MOBDB_FLAGS_GET_T(box->flags))
			selec = calc_period_hist_selectivity(vardata, &period, cachedOp, 
				TEMPORAL_STATISTICS);
	}
	else /* Unknown operator */
	{
		selec = default_tnumber_selectivity(cachedOp);
	}
	if (MOBDB_FLAGS_GET_T(box->flags))
		pfree(range);
	return selec;
}

/*****************************************************************************/

/*
 * Estimate the selectivity value of the operators for temporal types whose
 * bounding box is a TBOX, that is, tint and tfloat.
 */
PG_FUNCTION_INFO_V1(tnumber_sel);

PGDLLEXPORT Datum
tnumber_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = DEFAULT_TEMP_SELECTIVITY;
	CachedOp cachedOp;
	TBOX constBox;

	/*
	 * Get enumeration value associated to the operator
	 */
	bool found = tnumber_cachedop(operator, &cachedOp);
	/* In the case of unknown operator */
	if (!found)
		PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);

	/*
	 * If expression is not (variable op something) or (something op
	 * variable), then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(default_tnumber_selectivity(cachedOp));

	/*
	 * Can't do anything useful if the something is not a constant, either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(default_tnumber_selectivity(cachedOp));
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
			PG_RETURN_FLOAT8(default_tnumber_selectivity(cachedOp));
		}
	}

    /*
     * Transform the constant into a TBOX and a Period
     */
    memset(&constBox, 0, sizeof(TBOX));
    found = tnumber_const_to_period(other, &constBox);
    /* In the case of unknown constant */
    if (!found)
		PG_RETURN_FLOAT8(default_tnumber_selectivity(cachedOp));

	int duration = TYPMOD_GET_DURATION(vardata.atttypmod);
	assert(duration == TEMPORAL || duration == TEMPORALINST ||
		   duration == TEMPORALI || duration == TEMPORALSEQ ||
		   duration == TEMPORALS);
	if (duration == TEMPORALINST)
		selec = tnumberinst_sel(root, &vardata, &constBox, cachedOp);
	else if (duration == TEMPORAL)
		selec = tnumberi_sel(root, &vardata, &constBox, cachedOp);
	else
		/* duration is equal to TEMPORAL, TEMPORALSEQ, or TEMPORALS */
		selec = tnumbers_sel(root, &vardata, &constBox, cachedOp);

	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	PG_RETURN_FLOAT8(selec);
}

/*
	switch (cachedOp)
	{
		case OVERLAPS_OP:
		case CONTAINS_OP:
		case CONTAINED_OP:
		case SAME_OP:
			selec = tnumber_bbox_sel(root, vardata, constBox, cachedOp);
			break;
		case LEFT_OP:
			selec = tnumber_position_sel(vardata, other, false, false);
			break;
		case RIGHT_OP:
			selec = tnumber_position_sel(vardata, other, true, false);
			break;
		case OVERLEFT_OP:
			selec = tnumber_position_sel(vardata, other, false, true);
			break;
		case OVERRIGHT_OP:
			selec = tnumber_position_sel(vardata, other, true, true);
			break;
		case BEFORE_OP:
			selec = temporal_position_sel(root, &vardata, &period, false, false, LT_OP);
			break;
		case AFTER_OP:
			selec = temporal_position_sel(root, &vardata, &period, true, false, GT_OP);
			break;
		case OVERBEFORE_OP:
			selec = temporal_position_sel(root, &vardata, &period, false, true, LE_OP);
			break;
		case OVERAFTER_OP:
			selec = 1.0 - temporal_position_sel(root, &vardata, &period, false, false, GE_OP);
			break;
		default:
			selec = 0.001;
	}

	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	PG_RETURN_FLOAT8((float8) selec);
}
*/

PG_FUNCTION_INFO_V1(tnumber_joinsel);

PGDLLEXPORT Datum
tnumber_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(DEFAULT_TEMP_SELECTIVITY);
}


/*****************************************************************************/
