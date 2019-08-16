/*****************************************************************************
 *
 * TnumberSelFuncs.c
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

#include "TnumberSelFuncs.h"

#include <assert.h>
#include <access/htup_details.h>
#include <nodes/relation.h>
#include <utils/selfuncs.h>

#include "Period.h"
#include "Range.h"
#include "OidCache.h"
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

PG_FUNCTION_INFO_V1(tnumber_overlaps_sel);

PGDLLEXPORT Datum
tnumber_overlaps_sel(PG_FUNCTION_ARGS)
{
    PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
    Oid operator = PG_GETARG_OID(1);
    List *args = (List *) PG_GETARG_POINTER(2);
    int varRelid = PG_GETARG_INT32(3);
    Selectivity	selec = tnumber_bbox_sel(root, operator, args, varRelid, OVERLAPS_OP);
    if (selec < 0.0)
        selec = 0.005;
    else if (selec > 1.0)
        selec = 1.0;
    PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tnumber_overlaps_joinsel);

PGDLLEXPORT Datum
tnumber_overlaps_joinsel(PG_FUNCTION_ARGS)
{
    PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tnumber_contains_sel);

PGDLLEXPORT Datum
tnumber_contains_sel(PG_FUNCTION_ARGS)
{
    PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
    Oid operator = PG_GETARG_OID(1);
    List *args = (List *) PG_GETARG_POINTER(2);
    int varRelid = PG_GETARG_INT32(3);
    Selectivity	selec = tnumber_bbox_sel(root, operator, args, varRelid, get_tnumber_cacheOp(operator));
    if (selec < 0.0)
        selec = 0.002;
    else if (selec > 1.0)
        selec = 1.0;
    PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tnumber_contains_joinsel);

PGDLLEXPORT Datum
tnumber_contains_joinsel(PG_FUNCTION_ARGS)
{
    PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tnumber_same_sel);

PGDLLEXPORT Datum
tnumber_same_sel(PG_FUNCTION_ARGS)
{
    PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
    Oid operator = PG_GETARG_OID(1);
    List *args = (List *) PG_GETARG_POINTER(2);
    int varRelid = PG_GETARG_INT32(3);
    Selectivity	selec = tnumber_bbox_sel(root, operator, args, varRelid, SAME_OP);
    if (selec < 0.0)
        selec = 0.001;
    else if (selec > 1.0)
        selec = 1.0;
    PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(tnumber_same_joinsel);

PGDLLEXPORT Datum
tnumber_same_joinsel(PG_FUNCTION_ARGS)
{
    PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

/*
 * Selectivity for operators for relative position box operators, i.e.,
 * left (<<), overleft (&<), right (>>), overright (&>), before (<<#),
 * overbefore (&<#), after (#>>), overafter (#&>).
 */

PG_FUNCTION_INFO_V1(tnumber_position_sel);

PGDLLEXPORT Datum
tnumber_position_sel(PG_FUNCTION_ARGS)
{
    PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
    Oid operator = PG_GETARG_OID(1);
    List *args = (List *) PG_GETARG_POINTER(2);
    int varRelid = PG_GETARG_INT32(3);
    VariableStatData vardata;
    Node *other;
    bool varonleft;
    Selectivity selec = 0.001;
    CachedOp cachedOp = get_tnumber_cacheOp(operator);

    /* In the case of unknown position operator */
    if (cachedOp == OVERLAPS_OP)
        PG_RETURN_FLOAT8(selec);

    /*
     * If expression is not (variable op something) or (something op
     * variable), then punt and return a default estimate.
     */
    if (!get_restriction_variable(root, args, varRelid,
                                  &vardata, &other, &varonleft))
        PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));

    /*
     * Can't do anything useful if the something is not a constant, either.
     */
    if (!IsA(other, Const))
    {
        ReleaseVariableStats(vardata);
        PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));
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
        switch (cachedOp)
        {
            case LEFT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, true, false);
                break;
            case RIGHT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, false, false);
                break;
            case OVERLEFT_OP:
                selec = 1.0 - estimate_tnumber_position_sel(vardata, other, false, false);
                break;
            case OVERRIGHT_OP:
                selec = 1.0 - estimate_tnumber_position_sel(vardata, other, true, false);
                break;
            case BEFORE_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, true, false, GT_OP);
                break;
            case AFTER_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, false, false, LT_OP);
                break;
            case OVERBEFORE_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, true, true, GE_OP);
                break;
            case OVERAFTER_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, false, true, LE_OP);
                break;
            default:
                selec = 0.001;
        }
    }
    else
    {
        switch (cachedOp)
        {
            case LEFT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, false, false);
                break;
            case RIGHT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, true, false);
                break;
            case OVERLEFT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, false, true);
                break;
            case OVERRIGHT_OP:
                selec = estimate_tnumber_position_sel(vardata, other, true, true);
                break;
            case BEFORE_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, false, false, LT_OP);
                break;
            case AFTER_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, true, false, GT_OP);
                break;
            case OVERBEFORE_OP:
                selec = estimate_temporal_position_sel(root, vardata, other, false, true, LE_OP);
                break;
            case OVERAFTER_OP:
                selec = 1.0 - estimate_temporal_position_sel(root, vardata, other, false, false, GE_OP);
                break;
            default:
                selec = 0.001;
        }
    }

    if (selec < 0.0)
        selec = default_temporaltypes_selectivity(operator);
    ReleaseVariableStats(vardata);
    CLAMP_PROBABILITY(selec);
    PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(tnumber_position_joinsel);

PGDLLEXPORT Datum
tnumber_position_joinsel(PG_FUNCTION_ARGS)
{
    PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/
Selectivity
tnumber_bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp)
{
    VariableStatData vardata;
    Node *other;
    bool varonleft;
    Selectivity selec = 0.0;
    BBoxBounds bBoxBounds;
    bool numeric, temporal;
    double lower, upper;
    Period *period;
    ConstantData constantData;
    /*
     * If expression is not (variable op something) or (something op
     * variable), then punt and return a default estimate.
     */
    if (!get_restriction_variable(root, args, varRelid,
                                  &vardata, &other, &varonleft))
        PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));

    /*
     * Can't do anything useful if the something is not a constant, either.
     */
    if (!IsA(other, Const))
    {
        ReleaseVariableStats(vardata);
        PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));
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
            PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));
        }
    }

    /*
     * Set constant information
     */
    get_const_bounds(other, &bBoxBounds, &numeric, &lower, &upper,
                     &temporal, &period);

    constantData.bBoxBounds = bBoxBounds;
    constantData.oid = ((Const *) other)->consttype;

    constantData.lower = 0;constantData.upper = 0;  /* keep compiler quiet */
    constantData.period = NULL;   /* keep compiler quiet */
    if (numeric)
    {
        constantData.lower = lower;
        constantData.upper = upper;
        if (lower == upper && varonleft &&(cachedOp == CONTAINED_OP || cachedOp == SAME_OP))
            constantData.bBoxBounds = DNCONST;
    }
    if (temporal)
    {
        constantData.period = period;
    }

    if (cachedOp == CONTAINS_OP && !varonleft)
        selec = estimate_tnumber_bbox_sel(root, vardata, constantData, CONTAINED_OP);
    else if (cachedOp == CONTAINED_OP && !varonleft)
        selec = estimate_tnumber_bbox_sel(root, vardata, constantData, CONTAINS_OP);
    else
        selec = estimate_tnumber_bbox_sel(root, vardata, constantData, cachedOp);


    if (selec < 0.0)
        selec = default_temporaltypes_selectivity(operator);
    else if (selec > 1.0)
        selec = 1.0;

    ReleaseVariableStats(vardata);

    CLAMP_PROBABILITY(selec);

    return selec;
}
Selectivity
estimate_tnumber_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData, CachedOp cachedOp)
{
    // Check the temporal types and inside each one check the cachedOp
    Selectivity  selec = 0.0;
    int durationType = TYPMOD_GET_DURATION(vardata.atttypmod);
    if (vardata.vartype == type_oid(T_TINT) || vardata.vartype == type_oid(T_TFLOAT))
    {
        CachedType vartype = (vardata.vartype == type_oid(T_TINT)) ? T_INT4 : T_FLOAT8;
        CachedType varRangeType = (vardata.vartype == type_oid(T_TINT)) ? T_INTRANGE : T_FLOATRANGE;
        bool hasNumeric = false, hasTemporal = false;
        /*
         * Compute the selectivity with regard to the value of the constant.
         */
        double selec1 = 0.0;
        switch (constantData.bBoxBounds)
        {
            case SNCONST:
            case SNCONST_STCONST:
            case SNCONST_DTCONST:
            {
                if(durationType == TEMPORALINST)
                {
                    hasNumeric = true;
                    Oid op = oper_oid(EQ_OP, vartype, vartype);
                    selec1 = var_eq_const(&vardata, op, (Datum) constantData.lower, false, VALUE_STATISTICS);
                }
                else
                {
                    hasNumeric = true;
                    TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
                    selec1 = range_sel_internal(&vardata, (Datum) constantData.lower, false, true, typcache,
                                                VALUE_STATISTICS);
                    selec1 += range_sel_internal(&vardata, (Datum) constantData.lower, true, true, typcache,
                                                 VALUE_STATISTICS);
                    selec1 = 1 - selec1;
                    selec1 = selec1 < 0 ? 0 : selec1;
                }
                break;
            }
            case DNCONST:
            case DNCONST_STCONST:
            case DNCONST_DTCONST:
            {
                hasNumeric = true;
                if (cachedOp == OVERLAPS_OP)
                {
                    TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
                    selec1 = range_sel_internal(&vardata, (Datum)constantData.lower, false, false, typcache,
                                                VALUE_STATISTICS);
                    selec1 += range_sel_internal(&vardata, (Datum)constantData.upper, true, false, typcache,
                                                 VALUE_STATISTICS);
                }
                else if (cachedOp == CONTAINS_OP)
                {
                    TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
                    selec1 = range_sel_internal(&vardata, (Datum)constantData.lower, false, false, typcache,
                                                VALUE_STATISTICS);
                    selec1 += range_sel_internal(&vardata, (Datum)constantData.upper, false, false, typcache,
                                                 VALUE_STATISTICS);
                }
                else if (cachedOp == CONTAINED_OP)
                {
                    Oid opl = oper_oid(LT_OP, vartype, vartype);
                    Oid opg = oper_oid(GT_OP, vartype, vartype);
                    selec1 = scalarineq_sel(root, opl, false, true, &vardata, (Datum) constantData.lower,
                                            type_oid(vartype), VALUE_STATISTICS);
                    selec1 += scalarineq_sel(root, opg, true, false, &vardata, (Datum) constantData.upper,
                                             type_oid(vartype), VALUE_STATISTICS);
                }
                selec1 = 1 - selec1;
                selec1 = selec1 < 0 ? 0 : selec1;
                break;
            }
            default:
                break;
        }
        /*
         * Compute the selectivity with regard to the time dimension of the constant.
         */
        double selec2 = 0.0;
        switch (constantData.bBoxBounds)
        {
            case STCONST:
            case SNCONST_STCONST:
            case DNCONST_STCONST:
            {
                if(durationType == TEMPORALINST)
                {
                    hasTemporal = true;
                    Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
                    selec2 = var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->lower),
                                          false, TEMPORAL_STATISTICS);
                }
                else
                {
                    hasTemporal = true;
                    selec2 = period_sel_internal(root, &vardata, constantData.period,
                                                 oper_oid(cachedOp, T_PERIOD, T_TIMESTAMPTZ), TEMPORAL_STATISTICS);
                }
                break;
            }
            case DTCONST:
            case SNCONST_DTCONST:
            case DNCONST_DTCONST:
            {
                if (cachedOp == SAME_OP || cachedOp == CONTAINS_OP)
                {
                    Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
                    hasTemporal = true;
                    selec2 = var_eq_const(&vardata, op, (Datum) constantData.period->lower, false,
                                          TEMPORAL_STATISTICS);
                    selec2 *= var_eq_const(&vardata, op, (Datum) constantData.period->upper, false,
                                           TEMPORAL_STATISTICS);
                    selec2 = selec2 > 1 ? 1 : selec2;
                }
                else
                {
                    hasTemporal = true;
                    selec2 = period_sel_internal(root, &vardata, constantData.period,
                                                 oper_oid(cachedOp, T_PERIOD, T_PERIOD), TEMPORAL_STATISTICS);
                }
            }
            default:
                break;
        }
        if (hasNumeric && hasTemporal)
            selec = selec1 * selec2;
        else if (hasNumeric)
            selec = selec1;
        else if (hasTemporal)
            selec = selec2;
    }
    return selec;
}

Selectivity
estimate_tnumber_position_sel(VariableStatData vardata,
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

Selectivity
range_sel_internal(VariableStatData *vardata, Datum constval,
                   bool isgt, bool iseq, TypeCacheEntry *typcache, StatisticsStrategy strategy)
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

double
lower_or_higher_value_bound(Node *other, bool higher)
{
    double result = 0.0;
    Oid consttype = ((Const *) other)->consttype;
    if (higher)
    {
        if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
        {
            Temporal *temporal = DatumGetTemporal(((Const *) other)->constvalue);
            TBOX box = {0,0,0,0,0};
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
            TBOX box = {0,0,0,0,0};
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



/*
 * Calculate range operator selectivity using histograms of range bounds.
 *
 * This estimate is for the portion of values that are not empty and not
 * NULL.
 */
Selectivity
calc_range_hist_selectivity(VariableStatData *vardata, Datum constval,
                            TypeCacheEntry *typcache, bool isgt, bool iseq, StatisticsStrategy strategy)
{
    int nhist;
    RangeBound *hist_lower;
    RangeBound *hist_upper;
    int i;
    bool empty;
    Selectivity hist_selec = -1; /* keep compiler quiet */
    AttStatsSlot hslot;

    /* Try to get histogram of ranges */
    if (!(HeapTupleIsValid(vardata->statsTuple) &&
          get_attstatsslot_internal(&hslot, vardata->statsTuple,
                                    STATISTIC_KIND_BOUNDS_HISTOGRAM, InvalidOid,
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

/*
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a given const in a histogram of range bounds.
 */
Selectivity
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
 * Binary search on an array of range bounds. Returns greatest index of range
 * bound in array which is less(less or equal) than given range bound. If all
 * range bounds in array are greater or equal(greater) than given range bound,
 * return -1. When "equal" flag is set conditions in brackets are used.
 *
 * This function is used in scalar operator selectivity estimation. Another
 * goal of this function is to find a histogram bin where to stop
 * interpolation of portion of bounds which are less or equal to given bound.
 */
int
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

/* Get the name of the operator from different cases */
CachedOp
get_tnumber_cacheOp(Oid operator)
{
    for (int i = LT_OP; i <= OVERAFTER_OP; i++)
    {
        if (operator == oper_oid((CachedOp)i, T_INTRANGE, T_TINT) ||
            operator == oper_oid((CachedOp)i, T_TBOX, T_TINT) ||
            operator == oper_oid((CachedOp)i, T_TINT, T_INTRANGE) ||
            operator == oper_oid((CachedOp)i, T_TINT, T_TBOX) ||
            operator == oper_oid((CachedOp)i, T_TINT, T_TINT) ||
            operator == oper_oid((CachedOp)i, T_TINT, T_TFLOAT) ||
            operator == oper_oid((CachedOp)i, T_FLOATRANGE, T_TFLOAT) ||
            operator == oper_oid((CachedOp)i, T_TBOX, T_TFLOAT) ||
            operator == oper_oid((CachedOp)i, T_TFLOAT, T_FLOATRANGE) ||
            operator == oper_oid((CachedOp)i, T_TFLOAT, T_TBOX) ||
            operator == oper_oid((CachedOp)i, T_TFLOAT, T_TINT) ||
            operator == oper_oid((CachedOp)i, T_TFLOAT, T_TFLOAT))
            return (CachedOp)i;
    }
    return OVERLAPS_OP;
    /*ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
            errmsg("Operation not supported")));*/
}