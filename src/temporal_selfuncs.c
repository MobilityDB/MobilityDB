/*****************************************************************************
 *
 * temporal_selfuncs.c
 *	  Functions for selectivity estimation of operators on temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_selfuncs.h"

#include <assert.h>
#include <access/htup_details.h>
#include <catalog/pg_collation_d.h>
#include <utils/builtins.h>
#include <utils/date.h>
#include <utils/syscache.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "time_selfuncs.h"
#include "rangetypes_ext.h"
#include "tpoint.h"

/*****************************************************************************/

/*
 * Retrieve stored statistics for the temporal type to compute the selectivity 
 * value of the bounding box operators: overlaps (&&), contains (@>), 
 * contained (<@), and same (~=).
 */
static Selectivity
temporal_bbox_sel(PlannerInfo *root, Oid operator, List *args, int varRelid, 
	CachedOp cachedOp)
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
	 * Get information about the constant type
	 */
	get_const_bounds(other, &bBoxBounds, &numeric, &lower, &upper,
					 &temporal, &period);

	constantData.bBoxBounds = bBoxBounds;
	constantData.oid = ((Const *) other)->consttype;
	constantData.lower = constantData.upper = 0;  /* keep compiler quiet */
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

    /*
    * Estimate the temporal selectivity value based on the required operator for all temporal durations.
    */
	selec = estimate_temporal_bbox_sel(root, vardata, constantData, cachedOp);

	if (selec < 0.0)
		selec = default_temporaltypes_selectivity(operator);

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	return selec;
}

/*
 * Get the lower or the upper value of the temporal type based on the position operator:
 * Left (<<), Right (>>), OverLeft (<&), OverRight (&>), etc.
 */
static PeriodBound *
lower_or_higher_temporal_bound(Node *other, bool higher)
{
	PeriodBound *result = (PeriodBound *) palloc0(sizeof(PeriodBound));
	Oid consttype = ((Const *) other)->consttype;
	result->inclusive = ! higher;

    if (consttype == type_oid(T_TBOOL) || consttype == type_oid(T_TTEXT))
    {
        Period *p = (Period *) palloc(sizeof(Period));
        /* TODO MEMORY LEAK HERE !!!! */
        temporal_bbox(p, DatumGetTemporal(((Const *) other)->constvalue));
        result->val = (higher) ? p->upper: p->lower;
    }
    else if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
    {
        Temporal *temporal = DatumGetTemporal(((Const *) other)->constvalue);
        TBOX box;
        memset(&box, 0, sizeof(TBOX));
        temporal_bbox(&box, temporal);
        result->val = (higher) ? (TimestampTz)box.tmax : (TimestampTz)box.tmin;
    }
    else if (consttype == type_oid(T_TGEOMPOINT) || consttype == type_oid(T_TGEOGPOINT))
    {
        Period *p = (Period *) palloc(sizeof(Period));
        /* TODO MEMORY LEAK HERE !!!! */
        temporal_timespan_internal(p, DatumGetTemporal(((Const *) other)->constvalue));
        result->val = (higher) ? p->upper : p->lower;
    }
    else if (consttype == TIMESTAMPTZOID)
    {
        result->val = DatumGetTimestampTz(((Const *) other)->constvalue);
    }
    else if (consttype == type_oid(T_PERIOD))
    {
        result->val = (higher) ? DatumGetPeriod(((Const *) other)->constvalue)->upper :
                      DatumGetPeriod(((Const *) other)->constvalue)->lower;
    }
    else if (consttype == type_oid(T_TBOX))
    {
        result->val = (higher) ? (TimestampTz) DatumGetTboxP(((Const *) other)->constvalue)->tmax :
                      (TimestampTz) DatumGetTboxP(((Const *) other)->constvalue)->tmin;
    }
    else if (consttype == type_oid(T_STBOX))
    {
        result->val = (higher) ? (TimestampTz) DatumGetSTboxP(((Const *) other)->constvalue)->tmax :
                      (TimestampTz) DatumGetSTboxP(((Const *) other)->constvalue)->tmin;
    }

	return result;
}

/*
 * Estimate the selectivity value of the bounding box operators for temporal durations.
 */
Selectivity
estimate_temporal_bbox_sel(PlannerInfo *root, VariableStatData vardata, 
	ConstantData constantData, CachedOp cachedOp)
{
	/* Check the temporal types and inside each one check the cachedOp */
	Selectivity  selec = 0.0;
	int durationType = TYPMOD_GET_DURATION(vardata.atttypmod);
	switch (constantData.bBoxBounds)
	{
		case STCONST:
		case SNCONST_STCONST:
		case DNCONST_STCONST:
		{
			if (durationType == TEMPORALINST)
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
			if (durationType == TEMPORALINST)
			{
				if (cachedOp == SAME_OP || cachedOp == CONTAINS_OP)
				{
					Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
					selec = var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->lower),
										 false, TEMPORAL_STATISTICS);
					selec *= var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->upper),
										  false, TEMPORAL_STATISTICS);
					selec = selec > 1 ? 1 : selec;
				}
				else
				{
					Oid opl = oper_oid(LT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
					Oid opg = oper_oid(GT_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);

					selec = scalarineqsel_mobdb(root, opl, false, false, &vardata,
										   TimestampTzGetDatum(constantData.period->lower),
										   TIMESTAMPTZOID, TEMPORAL_STATISTICS);
					selec += scalarineqsel_mobdb(root, opg, true, true, &vardata,
											TimestampTzGetDatum(constantData.period->upper),
											TIMESTAMPTZOID, TEMPORAL_STATISTICS);
					selec = 1 - selec;
					selec = selec < 0 ? 0 : selec;
				}
			}
			else
			{
				if (cachedOp == SAME_OP)
				{
					Oid op = oper_oid(EQ_OP, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
					selec = var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->lower),
										 false, TEMPORAL_STATISTICS);
					selec *= var_eq_const(&vardata, op, TimestampTzGetDatum(constantData.period->upper),
										  false, TEMPORAL_STATISTICS);
					selec = selec > 1 ? 1 : selec;
				}
				else
					selec = period_sel_internal(root, &vardata, constantData.period,
											oper_oid(cachedOp, T_PERIOD, T_PERIOD), TEMPORAL_STATISTICS);
			}
			break;
		}
		default:
			break;
	}

	return selec;
}

/*
 * Estimate the selectivity value of the position operators for temporal durations.
 */

Selectivity
estimate_temporal_position_sel(PlannerInfo *root, VariableStatData vardata,
							   Node *other, bool isgt, bool iseq, CachedOp operator)
{
	double selec = 0.0;
	int durationType = TYPMOD_GET_DURATION(vardata.atttypmod);

	if (durationType == TEMPORALINST && (vardata.vartype == type_oid(T_TBOOL) ||
										 vardata.vartype == type_oid(T_TINT) ||
										 vardata.vartype == type_oid(T_TFLOAT) ||
										 vardata.vartype == type_oid(T_TTEXT) ||
										 vardata.vartype == type_oid(T_TGEOMPOINT) ||
										 vardata.vartype == type_oid(T_TGEOGPOINT)))
	{
		Oid op = oper_oid(operator, T_TIMESTAMPTZ, T_TIMESTAMPTZ);

		PeriodBound *constant = lower_or_higher_temporal_bound(other, isgt);

		selec = scalarineqsel_mobdb(root, op, isgt, iseq, &vardata, TimestampTzGetDatum(constant->val),
							   TIMESTAMPTZOID, TEMPORAL_STATISTICS);
	}
	else if (vardata.vartype == type_oid(T_TBOOL) ||
			 vardata.vartype == type_oid(T_TINT) ||
			 vardata.vartype == type_oid(T_TFLOAT) ||
			 vardata.vartype == type_oid(T_TTEXT) ||
			 vardata.vartype == type_oid(T_TGEOMPOINT) ||
			 vardata.vartype == type_oid(T_TGEOGPOINT))
	{
		Oid op = (Oid) 0;

		if (!isgt && !iseq)
			op = oper_oid(LT_OP, T_PERIOD, T_PERIOD);
		else if (isgt && iseq)
			op = oper_oid(GE_OP, T_PERIOD, T_PERIOD);
		else if (iseq)
			op = oper_oid(LE_OP, T_PERIOD, T_PERIOD);
		else if (isgt)
			op = oper_oid(GT_OP, T_PERIOD, T_PERIOD);

		PeriodBound *periodBound = lower_or_higher_temporal_bound(other, isgt);
		Period *period = period_make(periodBound->val, periodBound->val, true, true);
		selec = period_sel_internal(root, &vardata, period, op, TEMPORAL_STATISTICS);
	}
	else if (vardata.vartype == TIMESTAMPTZOID)
	{
		Oid op = oper_oid(operator, T_TIMESTAMPTZ, T_TIMESTAMPTZ);
		PeriodBound *constant = lower_or_higher_temporal_bound(other, isgt);
		selec = scalarineqsel_mobdb(root, op, isgt, iseq, &vardata, TimestampTzGetDatum(constant->val),
							   TIMESTAMPTZOID, DEFAULT_STATISTICS);
	}
	else if (vardata.vartype == type_oid(T_PERIOD))
	{
		Oid op = (Oid) 0;
		if (!isgt && !iseq)
			op = oper_oid(LT_OP, T_PERIOD, T_PERIOD);
		else if (!isgt && iseq)
			op = oper_oid(LE_OP, T_PERIOD, T_PERIOD);
		else if (isgt && !iseq)
			op = oper_oid(GT_OP, T_PERIOD, T_PERIOD);
		else if (isgt && iseq)
			op = oper_oid(LE_OP, T_PERIOD, T_PERIOD);

		PeriodBound *periodBound = lower_or_higher_temporal_bound(other, isgt);
		Period *period = period_make(periodBound->val, periodBound->val, true, true);
		selec = period_sel_internal(root, &vardata, period, op, DEFAULT_STATISTICS);
	}
	return selec;
}

/* Get the enum associated to the operator from different cases */
static bool
get_temporal_cachedop(Oid operator, CachedOp *cachedOp)
{
	for (int i = LT_OP; i <= OVERAFTER_OP; i++) {
		if (operator == oper_oid((CachedOp) i, T_PERIOD, T_TBOOL) ||
			operator == oper_oid((CachedOp) i, T_TBOOL, T_PERIOD) ||
			operator == oper_oid((CachedOp) i, T_TBOX, T_TBOOL) ||
			operator == oper_oid((CachedOp) i, T_TBOOL, T_TBOX) ||
			operator == oper_oid((CachedOp) i, T_TBOOL, T_TBOOL) ||
			operator == oper_oid((CachedOp) i, T_PERIOD, T_TTEXT) ||
			operator == oper_oid((CachedOp) i, T_TTEXT, T_PERIOD) ||
			operator == oper_oid((CachedOp) i, T_TBOX, T_TTEXT) ||
			operator == oper_oid((CachedOp) i, T_TTEXT, T_TBOX) ||
			operator == oper_oid((CachedOp) i, T_TTEXT, T_TTEXT))
			{
				*cachedOp = (CachedOp) i;
				return true;
			}
	}
	return false;
}

/*****************************************************************************/

Selectivity
period_sel_internal(PlannerInfo *root, VariableStatData *vardata, 
	Period *constval, Oid operator, StatStrategy strategy)
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
 * Do convert_to_scalar_mobdb()'s work for any number data type.
 */
static double
convert_numeric_to_scalar(Oid typid, Datum value)
{
	switch (typid) {
		case BOOLOID:
			return (double) DatumGetBool(value);
		case INT2OID:
			return (double) DatumGetInt16(value);
		case INT4OID:
			return (double) DatumGetInt32(value);
		case INT8OID:
			return (double) DatumGetInt64(value);
		case FLOAT4OID:
			return (double) DatumGetFloat4(value);
		case FLOAT8OID:
			return (double) DatumGetFloat8(value);
		case NUMERICOID:
			/* Note: out-of-range values will be clamped to +-HUGE_VAL */
			return (double) DatumGetFloat8(DirectFunctionCall1(
												  numeric_float8_no_overflow, value));
		case OIDOID:
		case REGPROCOID:
		case REGPROCEDUREOID:
		case REGOPEROID:
		case REGOPERATOROID:
		case REGCLASSOID:
		case REGTYPEOID:
		case REGCONFIGOID:
		case REGDICTIONARYOID:
		case REGROLEOID:
		case REGNAMESPACEOID:
			/* we can treat OIDs as integers... */
			return (double)DatumGetObjectId(value);
	}

	/*
	 * Can't get here unless someone tries to use scalarltsel/scalargtsel on
	 * an operator with one number and one non-number operand.
	 */
	elog(ERROR, "unsupported type: %u", typid);
	return 0;
}

/*
 * Do convert_to_scalar_mobdb()'s work for any timevalue data type.
 */
static double
convert_timevalue_to_scalar(Oid typid, Datum value)
{
	switch (typid)
	{
		case TIMESTAMPOID:
			return DatumGetTimestamp(value);
		case TIMESTAMPTZOID:
			return DatumGetTimestampTz(value);
		case DATEOID:
			return date2timestamp_no_overflow(DatumGetDateADT(value));
		default:
			elog(ERROR, "unsupported type: %u", typid);
			return 0;
	}
}

static bool
convert_to_scalar_mobdb(Oid valuetypid, Datum value, double *scaledvalue,
				  Datum lobound, Datum hibound, Oid boundstypid, double *scaledlobound,
				  double *scaledhibound)
{
	/*
	* Both the valuetypid and the boundstypid should exactly match the
	* declared input type(s) of the operator we are invoked for, so we just
	* error out if either is not recognized.
	*
	* XXX The histogram we are interpolating between points of could belong
	* to a column that's only binary-compatible with the declared type. In
	* essence we are assuming that the semantics of binary-compatible types
	* are enough alike that we can use a histogram generated with one type's
	* operators to estimate selectivity for the other's.  This is outright
	* wrong in some cases --- in particular signed versus unsigned
	* interpretation could trip us up.  But it's useful enough in the
	* majority of cases that we do it anyway.  Should think about more
	* rigorous ways to do it.
	*/
	switch (valuetypid)
	{
		/*
		 * Built-in number types
		 */
		case BOOLOID:
		case INT2OID:
		case INT4OID:
		case INT8OID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
		case OIDOID:
			*scaledvalue = convert_numeric_to_scalar(valuetypid, value);
			*scaledlobound = convert_numeric_to_scalar(boundstypid, lobound);
			*scaledhibound = convert_numeric_to_scalar(boundstypid, hibound);
			return true;

		/*
		* Built-in time types
		*/
		case TIMESTAMPTZOID:
			*scaledvalue = convert_timevalue_to_scalar(valuetypid, value);
			*scaledlobound = convert_timevalue_to_scalar(boundstypid, lobound);
			*scaledhibound = convert_timevalue_to_scalar(boundstypid, hibound);
			return true;
	}
	/* Don't know how to convert */
	*scaledvalue = *scaledlobound = *scaledhibound = 0;
	return false;
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
			 bool negate, StatStrategy strategy)
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
		if (get_attstatsslot_mobdb(&mcvslot, vardata->statsTuple,
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

/*
 * Get the values of the constant type and check it's type which is one of two values: temporal type and constant type.
 * The constant type is defined as the following:
 * SNCONST for single numeric constant type such as integer, float.
 * DNCONST for double numeric constant type such as intrange, floatrange.
 * STCONST for single temporal constant type such as timestamp.
 * DTCONST for double temporal constant type such as period.
 * SNCONST_STCONST for single numeric constant and single temporal constant such as temporalInst
 * DNCONST_DTCONST for double numeric constant and double temporal constant such as temporalI, temporalSeq,
 * TemporalS and temporal for the generic type
 */

void
get_const_bounds(Node *other, BBoxBounds *bBoxBounds, bool *numeric,
				 double *lower, double *upper, bool *temporal, Period **period)
{
	Oid consttype = ((Const *) other)->consttype;
	*numeric = false;
	*temporal = false;
	if (consttype == type_oid(T_TINT) || consttype == type_oid(T_TFLOAT))
	{
		Temporal *temp = DatumGetTemporal(((Const *) other)->constvalue);
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporal_bbox(&box, temp);
		/* The boxes should have both dimensions X and T  */
		assert(MOBDB_FLAGS_GET_X(box.flags) && MOBDB_FLAGS_GET_T(box.flags));
		*numeric = true;
		*temporal = true;
		*lower = box.xmin;
		*upper = box.xmax;
		*period = period_make((TimestampTz)box.tmin, (TimestampTz)box.tmax, true, true);
		*bBoxBounds = DNCONST_DTCONST;
	}
	else if (consttype == INT4OID || consttype == FLOAT8OID)
	{
		*numeric = true;
		*lower = (double) ((Const *) other)->constvalue;
		*bBoxBounds = SNCONST;

	}
	else if (consttype == type_oid(T_INTRANGE))
	{
		*numeric = true;
		*lower = (double) DatumGetInt32(lower_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		*upper = (double) DatumGetInt32(upper_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		*bBoxBounds = DNCONST;
	}
	else if (consttype == type_oid(T_FLOATRANGE))
	{
		*numeric = true;
		*lower = (double) DatumGetFloat8(lower_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		*upper = (double) DatumGetFloat8(upper_datum(DatumGetRangeTypeP(((Const *) other)->constvalue)));
		*bBoxBounds = DNCONST;

	}
	else if (consttype == type_oid(T_TBOOL) || consttype == type_oid(T_TTEXT))
	{
		Temporal *temp = DatumGetTemporal(((Const *) other)->constvalue);
		*period = palloc(sizeof(Period));
		temporal_bbox(*period, temp);
		*temporal = true;
		*bBoxBounds = DTCONST;
	}
	else if (consttype == TIMESTAMPTZOID)
	{
		*temporal = true;
		TimestampTz temp = DatumGetTimestampTz(((Const *) other)->constvalue);
		*period = period_make(temp, temp, true, true);
		*bBoxBounds = STCONST;
	}
	else if (consttype == type_oid(T_TGEOMPOINT) || consttype == type_oid(T_TGEOGPOINT) ||
			 consttype == type_oid(T_PERIOD))
	{
		*temporal = true;
		*period = period_copy((Period *) ((Const *) other)->constvalue);
		*bBoxBounds = DTCONST;
	}
	else if (consttype == type_oid(T_PERIODSET))
	{
		*temporal = true;
		*period = periodset_bbox(((PeriodSet *)((Const *) other)->constvalue));
		*bBoxBounds = DTCONST;
	}
	else if (consttype == type_oid(T_TIMESTAMPSET))
	{
		*temporal = true;
		*period = timestampset_bbox(((TimestampSet *)((Const *) other)->constvalue));
		*bBoxBounds = DTCONST;
	}
	else if (consttype == type_oid(T_TBOX))
	{
		TBOX *box = DatumGetTboxP(((Const *) other)->constvalue);
		if (MOBDB_FLAGS_GET_X(box->flags))
		{
			*numeric = true;
			*lower = box->xmin;
			*upper = box->xmax;
		}
		if (MOBDB_FLAGS_GET_T(box->flags))
		{
			*temporal = true;
			*period = period_make((TimestampTz)box->tmin, (TimestampTz)box->tmax, true, true);
		}
		if (*numeric && *temporal)
			*bBoxBounds = DNCONST_DTCONST;
		else if (*numeric)
		{
			if (box->xmin == box->xmax)
				*bBoxBounds = SNCONST;
			else
				*bBoxBounds = DNCONST;
		}
		else if (*temporal)
			*bBoxBounds = DTCONST;
	}
}

/*****************************************************************************
 * The following functions is taken from PostgreSQL but much processing is
 * removed ?????
 *****************************************************************************/

/*
 * get_actual_variable_range
 *		Attempt to identify the current *actual* minimum and/or maximum
 *		of the specified variable, by looking for a suitable btree index
 *		and fetching its low and/or high values.
 *		If successful, store values in *min and *max, and return TRUE.
 *		(Either pointer can be NULL if that endpoint isn't needed.)
 *		If no data available, return false.
 *
 * sortop is the "<" comparison operator to use.
 */
static bool
get_actual_variable_range(PlannerInfo *root, VariableStatData *vardata,
						  Oid sortop,
						  Datum *min, Datum *max)
{
	bool		have_data = false;
	RelOptInfo *rel = vardata->rel;
	//RangeTblEntry *rte;
	ListCell   *lc;

	/* No hope if no relation or it doesn't have indexes */
	if (rel == NULL || rel->indexlist == NIL)
		return false;
	/* If it has indexes it must be a plain relation */
	//rte = root->simple_rte_array[rel->relid];
	//Assert(rte->rtekind == RTE_RELATION);

	/* Search through the indexes to see if any match our problem */
	foreach(lc, rel->indexlist)
	{
		IndexOptInfo *index = (IndexOptInfo *) lfirst(lc);
		//ScanDirection indexscandir;

		/* Ignore non-btree indexes */
		if (index->relam != BTREE_AM_OID)
			continue;

		/*
		 * Ignore partial indexes --- we only want stats that cover the entire
		 * relation.
		 */
		if (index->indpred != NIL)
			continue;
	}

	return have_data;
}

/*****************************************************************************
 * The following functions are taken from PostgreSQL and simply added the last
 * argument to the equivalent PostgreSQL function in order to be able to 
 * select specific statistic slots.
 *****************************************************************************/

/*
 *	ineq_histogram_selectivity_mobdb	- Examine the histogram for scalarineqsel
 *
 * Determine the fraction of the variable's histogram population that
 * satisfies the inequality condition, ie, VAR < CONST or VAR > CONST.
 *
 * Returns -1 if there is no histogram (valid results will always be >= 0).
 *
 * Note that the result disregards both the most-common-values (if any) and
 * null entries.  The caller is expected to combine this result with
 * statistics for those portions of the column population.
 * This function simply added the last argument to the equivalent PostgreSQL
 * function in order to be able to select specific statistic slots.
 */

static double
ineq_histogram_selectivity_mobdb(PlannerInfo *root, VariableStatData *vardata,
						   FmgrInfo *opproc, bool isgt, bool iseq, Datum constval,
						   Oid consttype, StatStrategy strategy)
{

	double hist_selec = -1.0;
	AttStatsSlot sslot;

	/*
	 * Someday, ANALYZE might store more than one histogram per rel/att,
	 * corresponding to more than one possible sort ordering defined for the
	 * column type.  However, to make that work we will need to figure out
	 * which staop to search for --- it's not necessarily the one we have at
	 * hand!  (For example, we might have a '<=' operator rather than the '<'
	 * operator that will appear in staop.)  For now, assume that whatever
	 * appears in pg_statistic is sorted the same way our operator sorts, or
	 * the reverse way if isgt is TRUE.
	 */
	if (HeapTupleIsValid(vardata->statsTuple) &&
		statistic_proc_security_check(vardata, opproc->fn_oid) &&
		get_attstatsslot_mobdb(&sslot, vardata->statsTuple,
								  STATISTIC_KIND_HISTOGRAM, InvalidOid,
								  ATTSTATSSLOT_VALUES, strategy))
	{
		if (sslot.nvalues > 1)
		{
			/*
			 * Use binary search to find proper location, ie, the first slot
			 * at which the comparison fails.  (If the given operator isn't
			 * actually sort-compatible with the histogram, you'll get garbage
			 * results ... but probably not any more garbage-y than you would
			 * from the old linear search.)
			 *
			 * If the binary search accesses the first or last histogram
			 * entry, we try to replace that endpoint with the true column min
			 * or max as found by get_actual_variable_range().  This
			 * ameliorates misestimates when the min or max is moving as a
			 * result of changes since the last ANALYZE.  Note that this could
			 * result in effectively including MCVs into the histogram that
			 * weren't there before, but we don't try to correct for that.
			 */
			double histfrac;
			int lobound = 0;	/* first possible slot to search */
			int hibound = sslot.nvalues;		/* last+1 slot to search */
			bool have_end = false;

			/*
			 * If there are only two histogram entries, we'll want up-to-date
			 * values for both.  (If there are more than two, we need at most
			 * one of them to be updated, so we deal with that within the
			 * loop.)
			 */
			if (sslot.nvalues == 2)
				have_end = get_actual_variable_range(root,
													 vardata,
													 sslot.staop,
													 &sslot.values[0],
													 &sslot.values[1]);

			while (lobound < hibound)
			{
				int probe = (lobound + hibound) / 2;
				bool ltcmp;

				/*
				 * If we find ourselves about to compare to the first or last
				 * histogram entry, first try to replace it with the actual
				 * current min or max (unless we already did so above).
				 */
				if (probe == 0 && sslot.nvalues > 2)
					have_end = get_actual_variable_range(root, vardata,
														 sslot.staop, &sslot.values[0], NULL);
				else if (probe == sslot.nvalues - 1 && sslot.nvalues > 2)
					have_end = get_actual_variable_range(root, vardata,
														 sslot.staop, NULL, &sslot.values[probe]);

				ltcmp = DatumGetBool(FunctionCall2Coll(opproc,
													   DEFAULT_COLLATION_OID, sslot.values[probe], constval));
				if (isgt)
					ltcmp = !ltcmp;
				if (ltcmp)
					lobound = probe + 1;
				else
					hibound = probe;
			}

			if (lobound <= 0)
			{
				/* Constant is below lower histogram boundary. */
				histfrac = 0.0;
			}
			else if (lobound >= sslot.nvalues)
			{
				/* Constant is above upper histogram boundary. */
				histfrac = 1.0;
			}
			else
			{
				int i = lobound;
				double eq_selec = 0;
				double val,
						high,
						low;
				double binfrac;

				/*
				 * In the cases where we'll need it below, obtain an estimate
				 * of the selectivity of "x = constval".  We use a calculation
				 * similar to what var_eq_const() does for a non-MCV constant,
				 * ie, estimate that all distinct non-MCV values occur equally
				 * often.  But multiplication by "1.0 - sumcommon - nullfrac"
				 * will be done by our caller, so we shouldn't do that here.
				 * Therefore we can't try to clamp the estimate by reference
				 * to the least common MCV; the result would be too small.
				 *
				 * Note: since this is effectively assuming that constval
				 * isn't an MCV, it's logically dubious if constval in fact is
				 * one.  But we have to apply *some* correction for equality,
				 * and anyway we cannot tell if constval is an MCV, since we
				 * don't have a suitable equality operator at hand.
				 */
				if (i == 1 || isgt == iseq)
				{
					double otherdistinct;
					bool isdefault;
					AttStatsSlot mcvslot;

					/* Get estimated number of distinct values */
					otherdistinct = get_variable_numdistinct(vardata,
															 &isdefault);

					/* Subtract off the number of known MCVs */
					if (get_attstatsslot_mobdb(&mcvslot, vardata->statsTuple,
												  STATISTIC_KIND_MCV, InvalidOid,
												  ATTSTATSSLOT_NUMBERS, strategy))
					{
						otherdistinct -= mcvslot.nnumbers;
						free_attstatsslot(&mcvslot);
					}

					/* If result doesn't seem sane, leave eq_selec at 0 */
					if (otherdistinct > 1)
						eq_selec = 1.0 / otherdistinct;
				}

				/*
				 * Convert the constant and the two nearest bin boundary
				 * values to a uniform comparison scale, and do a linear
				 * interpolation within this bin.
				 */
				if (convert_to_scalar_mobdb(consttype, constval, &val,
									  sslot.values[i - 1], sslot.values[i], consttype,
									  &low, &high))
				{
					if (high <= low)
					{
						/* cope if bin boundaries appear identical */
						binfrac = 0.5;
					}
					else if (val <= low)
						binfrac = 0.0;
					else if (val >= high)
						binfrac = 1.0;
					else
					{
						binfrac = (val - low) / (high - low);

						/*
						 * Watch out for the possibility that we got a NaN or
						 * Infinity from the division.  This can happen
						 * despite the previous checks, if for example "low"
						 * is -Infinity.
						 */
						if (isnan(binfrac) ||
							binfrac < 0.0 || binfrac > 1.0)
							binfrac = 0.5;
					}
				}
				else
				{
					/*
					 * Ideally we'd produce an error here, on the grounds that
					 * the given operator shouldn't have scalarXXsel
					 * registered as its selectivity func unless we can deal
					 * with its operand types.  But currently, all manner of
					 * stuff is invoking scalarXXsel, so give a default
					 * estimate until that can be fixed.
					 */
					binfrac = 0.5;
				}

				/*
				 * Now, compute the overall selectivity across the values
				 * represented by the histogram.  We have i-1 full bins and
				 * binfrac partial bin below the constant.
				 */
				histfrac = (double) (i - 1) + binfrac;
				histfrac /= (double) (sslot.nvalues - 1);

				/*
				 * At this point, histfrac is an estimate of the fraction of
				 * the population represented by the histogram that satisfies
				 * "x <= constval".  Somewhat remarkably, this statement is
				 * true regardless of which operator we were doing the probes
				 * with, so long as convert_to_scalar_mobdb() delivers reasonable
				 * results.  If the probe constant is equal to some histogram
				 * entry, we would have considered the bin to the left of that
				 * entry if probing with "<" or ">=", or the bin to the right
				 * if probing with "<=" or ">"; but binfrac would have come
				 * out as 1.0 in the first case and 0.0 in the second, leading
				 * to the same histfrac in either case.  For probe constants
				 * between histogram entries, we find the same bin and get the
				 * same estimate with any operator.
				 *
				 * The fact that the estimate corresponds to "x <= constval"
				 * and not "x < constval" is because of the way that ANALYZE
				 * constructs the histogram: each entry is, effectively, the
				 * rightmost value in its sample bucket.  So selectivity
				 * values that are exact multiples of 1/(histogram_size-1)
				 * should be understood as estimates including a histogram
				 * entry plus everything to its left.
				 *
				 * However, that breaks down for the first histogram entry,
				 * which necessarily is the leftmost value in its sample
				 * bucket.  That means the first histogram bin is slightly
				 * narrower than the rest, by an amount equal to eq_selec.
				 * Another way to say that is that we want "x <= leftmost" to
				 * be estimated as eq_selec not zero.  So, if we're dealing
				 * with the first bin (i==1), rescale to make that true while
				 * adjusting the rest of that bin linearly.
				 */
				if (i == 1)
					histfrac += eq_selec * (1.0 - binfrac);

				/*
				 * "x <= constval" is good if we want an estimate for "<=" or
				 * ">", but if we are estimating for "<" or ">=", we now need
				 * to decrease the estimate by eq_selec.
				 */
				if (isgt == iseq)
					histfrac -= eq_selec;
			}

			/*
			 * Now histfrac = fraction of histogram entries below the
			 * constant.
			 *
			 * Account for "<" vs ">"
			 */
			hist_selec = isgt ? (1.0 - histfrac) : histfrac;

			/*
			 * The histogram boundaries are only approximate to begin with,
			 * and may well be out of date anyway.  Therefore, don't believe
			 * extremely small or large selectivity estimates --- unless we
			 * got actual current endpoint values from the table.
			 */
			if (have_end)
				CLAMP_PROBABILITY(hist_selec);
			else
			{
				if (hist_selec < 0.0001)
					hist_selec = 0.0001;
				else if (hist_selec > 0.9999)
					hist_selec = 0.9999;
			}
		}
		free_attstatsslot(&sslot);
	}
	return hist_selec;
}

/*
 *	mcv_selectivity_mobdb	- Examine the MCV list for selectivity estimates
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
static Selectivity
mcv_selectivity_mobdb(VariableStatData *vardata, FmgrInfo *opproc,
				Datum constval, Oid atttype, bool varonleft, 
				double *sumcommonp, StatStrategy strategy)
{
	double mcv_selec, sumcommon;
	AttStatsSlot mcvslot;
	int i;

	mcv_selec = 0.0;
	sumcommon = 0.0;
	if (HeapTupleIsValid(vardata->statsTuple) &&
		statistic_proc_security_check(vardata, opproc->fn_oid) &&
		get_attstatsslot_mobdb(&mcvslot, vardata->statsTuple,
							   STATISTIC_KIND_MCV, InvalidOid,
							   ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS, 
							   strategy))
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

/*
 *	scalarineqsel_mobdb		- Selectivity of "<", "<=", ">", ">=" for scalars.
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
 * convert_to_scalar_mobdb().  If it is applied to some other datatype,
 * it will return a default estimate.
 * This function simply added the last argument to the equivalent PostgreSQL
 * function in order to be able to select specific statistic slots.
 */
Selectivity
scalarineqsel_mobdb(PlannerInfo *root, Oid operator, bool isgt, bool iseq,
			   VariableStatData *vardata, Datum constval, Oid consttype,
			   StatStrategy strategy)
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
	mcv_selec = mcv_selectivity_mobdb(vardata, &opproc, constval, consttype, true,
										 &sumcommon, strategy);

	/*
	 * If there is a histogram, determine which bin the constant falls in, and
	 * compute the resulting contribution to selectivity.
	 */
	hist_selec = ineq_histogram_selectivity_mobdb(root, vardata,
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
 * get_attstatsslot
 *
 *      Extract the contents of a "slot" of a pg_statistic tuple.
 *      Returns true if requested slot type was found, else false.
 *
 * Unlike other routines in this file, this takes a pointer to an
 * already-looked-up tuple in the pg_statistic cache.  We do this since
 * most callers will want to extract more than one value from the cache
 * entry, and we don't want to repeat the cache lookup unnecessarily.
 * Also, this API allows this routine to be used with statistics tuples
 * that have been provided by a stats hook and didn't really come from
 * pg_statistic.
 *
 * sslot: pointer to output area (typically, a local variable in the caller).
 * statstuple: pg_statistic tuple to be examined.
 * reqkind: STAKIND code for desired statistics slot kind.
 * reqop: STAOP value wanted, or InvalidOid if don't care.
 * flags: bitmask of ATTSTATSSLOT_VALUES and/or ATTSTATSSLOT_NUMBERS.
 * strategy: the type of the extracted elements which is one of the following:
 * VALUE_STATISTICS: retrieves the value part from slot 0 to 2
 * TEMPORAL_STATISTICS: retrieves the temporal part from slot 2 to 5
 * DEFAULT_STATISTICS: retrieves the slots for the default postgreSQL types
 * that start from slot 0
 *
 * If a matching slot is found, true is returned, and *sslot is filled thus:
 * staop: receives the actual STAOP value.
 * stacoll: receives the actual STACOLL value.
 * valuetype: receives actual datatype of the elements of stavalues.
 * values: receives pointer to an array of the slot's stavalues.
 * nvalues: receives number of stavalues.
 * numbers: receives pointer to an array of the slot's stanumbers (as float4).
 * nnumbers: receives number of stanumbers.
 *
 * valuetype/values/nvalues are InvalidOid/NULL/0 if ATTSTATSSLOT_VALUES
 * wasn't specified.  Likewise, numbers/nnumbers are NULL/0 if
 * ATTSTATSSLOT_NUMBERS wasn't specified.
 *
 * If no matching slot is found, false is returned, and *sslot is zeroed.
 *
 * Note that the current API doesn't allow for searching for a slot with
 * a particular collation.  If we ever actually support recording more than
 * one collation, we'll have to extend the API, but for now simple is good.
 *
 * The data referred to by the fields of sslot is locally palloc'd and
 * is independent of the original pg_statistic tuple.  When the caller
 * is done with it, call free_attstatsslot to release the palloc'd data.
 *
 * If it's desirable to call free_attstatsslot when get_attstatsslot might
 * not have been called, memset'ing sslot to zeroes will allow that.
 * This function simply added the last argument to the equivalent PostgreSQL
 * function in order to be able to select specific statistic slots.
 */

bool
get_attstatsslot_mobdb(AttStatsSlot *sslot, HeapTuple statstuple,
					   int reqkind, Oid reqop, int flags, 
					   StatStrategy strategy)
{
	Form_pg_statistic stats = (Form_pg_statistic) GETSTRUCT(statstuple);
	int i, start = 0, end = 0;  /* keep compiler quiet */
	Datum val;
	bool isnull;
	ArrayType *statarray;
	Oid arrayelemtype;
	int narrayelem;
	HeapTuple typeTuple;
	Form_pg_type typeForm;

	switch (strategy) 
	{
		case VALUE_STATISTICS:
		{
			start = 0;
			end = 2;
			break;
		}
		case TEMPORAL_STATISTICS:
		{
			start = 2;
			end = 5;
			break;
		}
		case DEFAULT_STATISTICS:
		{
			start = 0;
			end = STATISTIC_NUM_SLOTS;
			break;
		}
		default:
			break;
	}

	/* initialize *sslot properly */
	memset(sslot, 0, sizeof(AttStatsSlot));

	for (i = start; i < end; i++) 
	{
		if ((&stats->stakind1)[i] == reqkind &&
			(reqop == InvalidOid || (&stats->staop1)[i] == reqop))
			break;
	}
	if (i >= end)
		return false;			/* not there */

	sslot->staop = (&stats->staop1)[i];

	if (flags & ATTSTATSSLOT_VALUES)
	{
		val = SysCacheGetAttr(STATRELATTINH, statstuple,
							  Anum_pg_statistic_stavalues1 + i,
							  &isnull);
		if (isnull)
			elog(ERROR, "stavalues is null");

		/*
		 * Detoast the array if needed, and in any case make a copy that's
		 * under control of this AttStatsSlot.
		 */
		statarray = DatumGetArrayTypePCopy(val);

		/*
		 * Extract the actual array element type, and pass it after in case the
		 * caller needs it.
		 */
		sslot->valuetype = arrayelemtype = ARR_ELEMTYPE(statarray);

		/* Need info about element type */
		typeTuple = SearchSysCache1(TYPEOID, ObjectIdGetDatum(arrayelemtype));
		if (!HeapTupleIsValid(typeTuple))
			elog(ERROR, "cache lookup failed for type %u", arrayelemtype);
		typeForm = (Form_pg_type) GETSTRUCT(typeTuple);

		/* Deconstruct array into Datum elements; NULLs not expected */
		deconstruct_array(statarray,
						  arrayelemtype,
						  typeForm->typlen,
						  typeForm->typbyval,
						  typeForm->typalign,
						  &sslot->values, NULL, &sslot->nvalues);

		/*
		 * If the element type is pass-by-reference, we now have a bunch of
		 * Datums that are pointers into the statarray, so we need to keep
		 * that until free_attstatsslot.  Otherwise, all the useful info is in
		 * sslot->values[], so we can free the array object immediately.
		 */
		if (!typeForm->typbyval)
			sslot->values_arr = statarray;
		else
			pfree(statarray);

		ReleaseSysCache(typeTuple);
	}

	if (flags & ATTSTATSSLOT_NUMBERS) {
		val = SysCacheGetAttr(STATRELATTINH, statstuple,
							  Anum_pg_statistic_stanumbers1 + i,
							  &isnull);
		if (isnull)
			elog(ERROR, "stanumbers is null");

		/*
		 * Detoast the array if needed, and in any case make a copy that's
		 * under control of this AttStatsSlot.
		 */
		statarray = DatumGetArrayTypePCopy(val);

		/*
		 * We expect the array to be a 1-D float4 array; verify that. We don't
		 * need to use deconstruct_array() since the array data is just going
		 * to look like a C array of float4 values.
		 */
		narrayelem = ARR_DIMS(statarray)[0];
		if (ARR_NDIM(statarray) != 1 || narrayelem <= 0 ||
			ARR_HASNULL(statarray) ||
			ARR_ELEMTYPE(statarray) != FLOAT4OID)
			elog(ERROR, "stanumbers is not a 1-D float4 array");

		/* Give caller a pointer directly into the statarray */
		sslot->numbers = (float4 *) ARR_DATA_PTR(statarray);
		sslot->nnumbers = narrayelem;

		/* We'll free the statarray in free_attstatsslot */
		sslot->numbers_arr = statarray;
	}

	return true;
}

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
	Selectivity	selec = temporal_bbox_sel(root, operator, args, varRelid, OVERLAPS_OP);
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
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = DEFAULT_SELECTIVITY;
	CachedOp cachedOp;
	bool found = get_temporal_cachedop(operator, &cachedOp);
	if (!found)
		PG_RETURN_FLOAT8(selec);

	selec = temporal_bbox_sel(root, operator, args, varRelid, cachedOp);
	if (selec < 0.0)
		selec = 0.002;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);
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
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	Selectivity	selec = temporal_bbox_sel(root, operator, args, varRelid, SAME_OP);
	if (selec < 0.0)
		selec = DEFAULT_SELECTIVITY;
	else if (selec > 1.0)
		selec = 1.0;
	PG_RETURN_FLOAT8(selec);
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
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = DEFAULT_SELECTIVITY;
	CachedOp cachedOp;

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
            /* TODO: check whether there might still be a way to estimate.
            * Use default selectivity (should we raise an error instead?) */
            ReleaseVariableStats(vardata);
            PG_RETURN_FLOAT8(default_temporaltypes_selectivity(operator));
        }
    }

    bool found = get_temporal_cachedop(operator, &cachedOp);
    /* In the case of unknown operator */
    if (!found)
        PG_RETURN_FLOAT8(selec);
	/*
	 * If var is on the right, commute the operator, so that we can assume the
	 * var is on the left in what follows.
	 */

    switch (cachedOp)
    {
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
            selec = DEFAULT_SELECTIVITY;
    }


	if (selec < 0.0)
		selec = default_temporaltypes_selectivity(operator);
	ReleaseVariableStats(vardata);
	CLAMP_PROBABILITY(selec);
	PG_RETURN_FLOAT8(selec);
}

PG_FUNCTION_INFO_V1(temporal_position_joinsel);

PGDLLEXPORT Datum
temporal_position_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/
