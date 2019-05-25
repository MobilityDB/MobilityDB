/*****************************************************************************
 *
 * TemporalSelFuncs.c
 *	  Functions for selectivity estimation of operators on temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	src/TemporalSelFuncs.c
 *
 *****************************************************************************/
 
#include "TemporalTypes.h"
#include "TimeSelFuncs.h"

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

PG_FUNCTION_INFO_V1(temporal_overlaps_sel);

PGDLLEXPORT Datum
temporal_overlaps_sel(PG_FUNCTION_ARGS)
{
    PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
    Oid operator = PG_GETARG_OID(1);
    List *args = (List *) PG_GETARG_POINTER(2);
    int varRelid = PG_GETARG_INT32(3);
    Selectivity	selec = bbox_sel(root, operator, args, varRelid, OVERLAPS_OP);
    if (selec < 0.0)
        selec = 0.005;
    else if (selec > 1.0)
        selec = 1.0;
    PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(temporal_overlaps_joinsel);

PGDLLEXPORT Datum
temporal_overlaps_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(temporal_contains_sel);

PGDLLEXPORT Datum
temporal_contains_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(temporal_contains_joinsel);

PGDLLEXPORT Datum
temporal_contains_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(temporal_same_sel);

PGDLLEXPORT Datum
temporal_same_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(temporal_same_joinsel);

PGDLLEXPORT Datum
temporal_same_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

/*
 * Selectivity for operators for relative position box operators, i.e., 
 * left (<<), overleft (&<), right (>>), overright (&>), before (<<#), 
 * overbefore (&<#), after (#>>), overafter (#&>). 
 */

PG_FUNCTION_INFO_V1(temporal_position_sel);

PGDLLEXPORT Datum
temporal_position_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(temporal_position_joinsel);

PGDLLEXPORT Datum
temporal_position_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

Selectivity
bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, CachedOp cachedOp)
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

    selec = calc_bbox_sel(root, vardata, constantData, cachedOp);

    if (selec < 0.0)
        selec = default_temporaltypes_selectivity(operator);

    ReleaseVariableStats(vardata);

    CLAMP_PROBABILITY(selec);

    return selec;
}

Selectivity
calc_bbox_sel(PlannerInfo *root, VariableStatData vardata, ConstantData constantData, CachedOp cachedOp)
{
    // Check the temporal types and inside each one check the cachedOp
    Selectivity  selec = 0.0;
    int durationType = TYPMOD_GET_DURATION(vardata.atttypmod);
    if (vardata.vartype == type_oid(T_TBOOL) || vardata.vartype == type_oid(T_TTEXT) ||
        vardata.vartype == type_oid(T_TIMESTAMPTZ))
    {
        switch (constantData.bBoxBounds)
        {
            case STCONST:
            case SNCONST_STCONST:
            case DNCONST_STCONST:
            {
                if(durationType == TEMPORALINST)
                {
                    Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
                    selec = var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->lower),
                                         false, TEMPORAL_STATISTICS);
                }
                else
                    selec = period_sel_internal(root, &vardata, constantData.period,
                                                oper_oid(cachedOp, T_PERIOD, T_TIMESTAMPTZ), TEMPORAL_STATISTICS);

                break;
            }
            case DTCONST:
            case SNCONST_DTCONST:
            case DNCONST_DTCONST:
            {
                if(durationType == TEMPORALINST)
                {
                    Oid opl = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
                    Oid opg = oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);

                    selec = scalarineq_sel(root, opl, false, false, &vardata,
                                           TimestampTzGetDatum(constantData.period->lower),
                                           TIMESTAMPTZOID, TEMPORAL_STATISTICS);
                    selec += scalarineq_sel(root, opg, true, true, &vardata,
                                            TimestampTzGetDatum(constantData.period->upper),
                                            TIMESTAMPTZOID, TEMPORAL_STATISTICS);
                    selec = 1 - selec;
                    selec = selec < 0 ? 0 : selec;
                }
                else
                {
                    selec = period_sel_internal(root, &vardata, constantData.period,
                                                oper_oid(cachedOp, T_PERIOD, T_PERIOD), TEMPORAL_STATISTICS);
                }
                break;
            }
            default:
                break;
        }
    }
    else if (vardata.vartype == T_PERIOD)
    {

    }
    return selec;
}

Selectivity
period_sel_internal(PlannerInfo *root, VariableStatData *vardata, Period *constval,
                    Oid operator, StatisticsStrategy strategy)
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
    hist_selec = calc_period_hist_selectivity(vardata, constval, operator, strategy);
    selec = (1.0 - empty_frac) * hist_selec;
    selec *= (1.0 - null_frac);
    return selec;
}

/*
 *	scalarineq_sel		- Selectivity of "<", "<=", ">", ">=" for scalars.
 *
 * This is the guts of scalarltsel/scalarlesel/scalargtsel/scalargesel.
 * The isgt and iseq flags distinguish which of the four cases apply.
 *
 * The caller has commuted the clause, if necessary, so that we can treat
 * the variable as being on the left.  The caller must also make sure that
 * the other side of the clause is a non-null Const, and dissect that into
 * a value and datatype.  (This definition simplifies some callers that
 * want to estimate against a computed value instead of a Const node.)
 *
 * This routine works for any datatype (or pair of datatypes) known to
 * convert_to_scalar().  If it is applied to some other datatype,
 * it will return a default estimate.
 */
Selectivity
scalarineq_sel(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
               VariableStatData *vardata, Datum constval, Oid consttype,
               StatisticsStrategy strategy)
{
    Form_pg_statistic stats;
    FmgrInfo opproc;
    double mcv_selec, hist_selec, sumcommon;
    Selectivity selec;

    if (!HeapTupleIsValid(vardata->statsTuple))
    {
        /* no stats available, so default result */
        return DEFAULT_INEQ_SEL;
    }
    stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);

    fmgr_info(get_opcode(operator), &opproc);

    /*
     * If we have most-common-values info, add up the fractions of the MCV
     * entries that satisfy MCV OP CONST.  These fractions contribute directly
     * to the result selectivity.  Also add up the total fraction represented
     * by MCV entries.
     */
    mcv_selec = mcv_selectivity_internal(vardata, &opproc, constval, consttype, true,
                                         &sumcommon, strategy);

    /*
     * If there is a histogram, determine which bin the constant falls in, and
     * compute the resulting contribution to selectivity.
     */
    hist_selec = ineq_histogram_selectivity(root, vardata,
                                            &opproc, isgt, iseq,
                                            constval, consttype, strategy);

    /*
     * Now merge the results from the MCV and histogram calculations,
     * realizing that the histogram covers only the non-null values that are
     * not listed in MCV.
     */
    selec = 1.0 - stats->stanullfrac - sumcommon;

    if (hist_selec >= 0.0)
        selec *= hist_selec;
    else
    {
        /*
         * If no histogram but there are values not accounted for by MCV,
         * arbitrarily assume half of them will match.
         */
        selec *= 0.5;
    }
    selec += mcv_selec;
    /* result should be in range, but make sure... */
    CLAMP_PROBABILITY(selec);
    return selec;
}

/*
 *	mcv_selectivity			- Examine the MCV list for selectivity estimates
 *
 * Determine the fraction of the variable's MCV population that satisfies
 * the predicate (VAR OP CONST), or (CONST OP VAR) if !varonleft.  Also
 * compute the fraction of the total column population represented by the MCV
 * list.  This code will work for any boolean-returning predicate operator.
 *
 * The function result is the MCV selectivity, and the fraction of the
 * total population is returned into *sumcommonp.  Zeroes are returned
 * if there is no MCV list.
 */
Selectivity
mcv_selectivity_internal(VariableStatData *vardata, FmgrInfo *opproc,
                         Datum constval, Oid atttype, bool varonleft, double *sumcommonp,
                         StatisticsStrategy strategy)
{
    double mcv_selec, sumcommon;
    int i;
    AttStatsSlot mcvslot;
    mcv_selec = 0.0;
    sumcommon = 0.0;
    if (HeapTupleIsValid(vardata->statsTuple) &&
        statistic_proc_security_check(vardata, opproc->fn_oid) &&
        get_attstatsslot_internal(&mcvslot, vardata->statsTuple,
                                  STATISTIC_KIND_MCV, InvalidOid,
                                  ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS, strategy))
    {
        for (i = 0; i < mcvslot.nvalues; i++)
        {
            if (varonleft ?
                DatumGetBool(FunctionCall2Coll(opproc,
                                               DEFAULT_COLLATION_OID,
                                               mcvslot.values[i],
                                               constval)) :
                DatumGetBool(FunctionCall2Coll(opproc,
                                               DEFAULT_COLLATION_OID,
                                               constval,
                                               mcvslot.values[i]))
                    )
                mcv_selec += mcvslot.numbers[i];
            sumcommon += mcvslot.numbers[i];
        }
        free_attstatsslot(&mcvslot);
    }
    *sumcommonp = sumcommon;
    return mcv_selec;
}
/*****************************************************************************
 * Helper functions for calculating selectivity.
 *****************************************************************************/
/*
 * Returns a default selectivity estimate for given operator, when we don't
 * have statistics or cannot use them for some reason.
 */
double
default_temporaltypes_selectivity(Oid operator)
{
    if (operator == oper_oid(OVERLAPS_OP, T_PERIOD, T_PERIOD))
        return 0.01;

    if (operator == oper_oid(CONTAINS_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(CONTAINED_OP, T_PERIOD, T_PERIOD))
        return 0.005;

    if (operator == oper_oid(CONTAINS_OP, T_PERIOD, T_TIMESTAMPTZ) ||
        operator == oper_oid(CONTAINED_OP, T_TIMESTAMPTZ, T_PERIOD))
        /*
         * "period @> elem" is more or less identical to a scalar
         * inequality "A >= b AND A <= c".
         */
        return DEFAULT_RANGE_INEQ_SEL;

    if (operator == oper_oid(LT_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(LE_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(GT_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(GE_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(LEFT_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(RIGHT_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(OVERLEFT_OP, T_PERIOD, T_PERIOD) ||
        operator == oper_oid(OVERRIGHT_OP, T_PERIOD, T_PERIOD))
        /* these are similar to regular scalar inequalities */
        return DEFAULT_INEQ_SEL;

    /* all period operators should be handled above, but just in case */
    return 0.01;
}

/*
 * var_eq_const --- eqsel for var = const case
 *
 * This is split out so that some other estimation functions can use it.
 */
double
var_eq_const(VariableStatData *vardata, Oid operator, Datum constval,
             bool negate, StatisticsStrategy strategy)
{
    double selec;
    bool isdefault;
    Oid opfuncoid;
    double nullfrac = 0.0;

    /*
     * If the constant is NULL, assume operator is strict and return zero, ie,
     * operator will never return TRUE.
     */
    if (!constval)
        return 0.0;

    /*
     * If we matched the var to a unique index or DISTINCT clause, assume
     * there is exactly one match regardless of anything else.  (This is
     * slightly bogus, since the index or clause's equality operator might be
     * different from ours, but it's much more likely to be right than
     * ignoring the information.)
     */
    if (vardata->isunique && vardata->rel && vardata->rel->tuples >= 1.0)
        return 1.0 / vardata->rel->tuples;

    if (HeapTupleIsValid(vardata->statsTuple) &&
        statistic_proc_security_check(vardata, (opfuncoid = get_opcode(operator))))
    {
        Form_pg_statistic stats;
        bool match = false;
        int i;
        AttStatsSlot mcvslot;

        stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);

        nullfrac = stats->stanullfrac;
        /*
         * Is the constant "=" to any of the column's most common values?
         * (Although the given operator may not really be "=", we will assume
         * that seeing whether it returns TRUE is an appropriate test.  If you
         * don't like this, maybe you shouldn't be using eqsel for your
         * operator...)
         */
        if (get_attstatsslot_internal(&mcvslot, vardata->statsTuple,
                                      STATISTIC_KIND_MCV, InvalidOid,
                                      ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS, strategy))
        {
            FmgrInfo eqproc;
            fmgr_info(opfuncoid, &eqproc);
            for (i = 0; i < mcvslot.nvalues; i++)
            {
                /* be careful to apply operator right way 'round */
//				if (varonleft)
                match = DatumGetBool(FunctionCall2Coll(&eqproc,
                                                       DEFAULT_COLLATION_OID,
                                                       mcvslot.values[i],
                                                       constval));
                if (match)
                    break;
            }
        }
        else
        {
            /* no most-common-value info available */
            mcvslot.values = NULL;
            mcvslot.numbers = NULL;
            i = mcvslot.nvalues = mcvslot.nnumbers = 0;
        }
        if (match)
        {
            /*
             * Constant is "=" to this common value.  We know selectivity
             * exactly (or as exactly as ANALYZE could calculate it, anyway).
             */
            selec = mcvslot.numbers[i];
        }
        else
        {
            /*
             * Comparison is against a constant that is neither NULL nor any
             * of the common values.  Its selectivity cannot be more than
             * this:
             */
            double sumcommon = 0.0;
            double otherdistinct;

            for (i = 0; i < mcvslot.nnumbers; i++)
                sumcommon += mcvslot.numbers[i];
            selec = 1.0 - sumcommon - nullfrac;
            CLAMP_PROBABILITY(selec);

            /*
             * and in fact it's probably a good deal less. We approximate that
             * all the not-common values share this remaining fraction
             * equally, so we divide by the number of other distinct values.
             */
            otherdistinct = get_variable_numdistinct(vardata, &isdefault) - mcvslot.nnumbers;
            if (otherdistinct > 1)
                selec /= otherdistinct;

            /*
             * Another cross-check: selectivity shouldn't be estimated as more
             * than the least common "most common value".
             */
            if (mcvslot.nnumbers > 0 && selec > mcvslot.numbers[mcvslot.nnumbers - 1])
                selec = mcvslot.numbers[mcvslot.nnumbers - 1];
        }

        free_attstatsslot(&mcvslot);
    }
    else
    {
        /*
         * No ANALYZE stats available, so make a guess using estimated number
         * of distinct values and assuming they are equally common. (The guess
         * is unlikely to be very good, but we do know a few special cases.)
         */
        selec = 1.0 / get_variable_numdistinct(vardata, &isdefault);
    }

    /* now adjust if we wanted <> rather than = */
    if (negate)
        selec = 1.0 - selec - nullfrac;

    /* result should be in range, but make sure... */
    CLAMP_PROBABILITY(selec);
    return selec;
}
