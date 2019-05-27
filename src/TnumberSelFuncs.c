/*****************************************************************************
 *
 * TnumberSelFuncs.c
 *	  Functions for selectivity estimation of operators on temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	src/TnumberSelFuncs.c
 *
 *****************************************************************************/
 
#include "TemporalTypes.h"
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
	PG_RETURN_FLOAT8(0.005);
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
	PG_RETURN_FLOAT8(0.002);
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
	PG_RETURN_FLOAT8(0.001);
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
	PG_RETURN_FLOAT8(0.001);
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
    }
    if (temporal)
    {
        constantData.period = period;
    }

    selec = estimate_tnumber_bbox_sel(root, vardata, constantData, cachedOp);

    if (selec < 0.0)
        selec = default_temporaltypes_selectivity(operator);

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
                if(vardata.atttypmod == TEMPORALINST)
                {
                    hasNumeric = true;
                    Oid op = oper_oid(EQ_OP, vartype, vartype);
                    selec1 = var_eq_const(&vardata, op, (Datum) constantData.lower, false, VALUE_STATISTICS);
                }
                else
                {
                    hasNumeric = true;
                    TypeCacheEntry *typcache = lookup_type_cache(type_oid(varRangeType), TYPECACHE_RANGE_INFO);
                    selec1 = range_sel_internal(root, &vardata, (Datum) constantData.lower, false, true, typcache,
                                                VALUE_STATISTICS);
                    selec1 += range_sel_internal(root, &vardata, (Datum) constantData.lower, true, true, typcache,
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
                Oid opl = oper_oid(LT_OP, vartype, vartype);
                Oid opg = oper_oid(GT_OP, vartype, vartype);
                selec1 = scalarineq_sel(root, opl, false, false, &vardata, (Datum) constantData.lower,
                                        type_oid(vartype), VALUE_STATISTICS);
                selec1 += scalarineq_sel(root, opg, true, false, &vardata, (Datum) constantData.upper,
                                         type_oid(vartype), VALUE_STATISTICS);
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
            case DTCONST:
            case SNCONST_DTCONST:
            case DNCONST_DTCONST:
            {
                if(vardata.atttypmod == TEMPORALINST)
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
                                                 oper_oid(CONTAINS_OP, T_PERIOD, T_TIMESTAMPTZ), TEMPORAL_STATISTICS);

                }
                break;
            }
            default:
                break;
        }
        if (hasNumeric && hasTemporal)
            selec = selec1 * selec2;
        else if (hasNumeric && !hasTemporal)
            selec = selec1;
        else if (!hasNumeric && hasTemporal)
            selec = selec2;
        else
            selec = 0.0;
    }
    return selec;
}
