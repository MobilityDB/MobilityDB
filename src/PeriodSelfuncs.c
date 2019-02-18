/*****************************************************************************
 *
 * PeriodSelfuncs.c
 *	  Functions for selectivity estimation of period operators
 *
 * Estimates are based on histograms of lower and upper bounds.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

static double calc_periodsel(VariableStatData *vardata,
	Period *constval, Oid operator);

/*
 * periodsel -- restriction selectivity for period operators
 */
PG_FUNCTION_INFO_V1(periodsel);

PGDLLEXPORT Datum
periodsel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid			operator = PG_GETARG_OID(1);
	List	   *args = (List *) PG_GETARG_POINTER(2);
	int			varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node	   *other;
	bool		varonleft;
	Selectivity selec;
	Period  *constperiod = NULL;

	/*
	 * If expression is not (variable op something) or (something op
	 * variable), then punt and return a default estimate.
	 */
	if (!get_restriction_variable(root, args, varRelid,
								  &vardata, &other, &varonleft))
		PG_RETURN_FLOAT8(default_period_selectivity(operator));

	/*
	 * Can't do anything useful if the something is not a constant, either.
	 */
	if (!IsA(other, Const))
	{
		ReleaseVariableStats(vardata);
		PG_RETURN_FLOAT8(default_period_selectivity(operator));
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
			PG_RETURN_FLOAT8(default_period_selectivity(operator));
		}
	}

	/*
	 * OK, there's a Var and a Const we're dealing with here.  We need the
	 * Const to be of same period type as the column, else we can't do anything
	 * useful. (Such cases will likely fail at runtime, but here we'd rather
	 * just return a default estimate.)
	 *
	 * If the operator is "period @> element", the constant should be of the
	 * element type of the period column. Convert it to a period that includes
	 * only that single point, so that we don't need special handling for that
	 * in what follows.
	 */
	 
	if (((Const *) other)->consttype == type_oid(T_PERIOD))
	{
		/* just copy the value */
		constperiod = DatumGetPeriod(((Const *) other)->constvalue);
	}
	else if (((Const *) other)->consttype == TIMESTAMPTZOID)
	{
		/* the right argument is a constant TIMESTAMPTZ. We convert it into
		 * a singleton period
		 */
		TimestampTz t = DatumGetTimestampTz( ((Const *) other)->constvalue );
		constperiod = period_make(t, t, true, true);
	}
	else if (((Const *) other)->consttype == type_oid(T_TIMESTAMPSET))
	{
		/* the right argument is a constant TIMESTAMPSET. We convert it into
		 * a period, which is its bounding box.
		 */
		constperiod =  timestampset_bbox(
				DatumGetTimestampSet(((Const *) other)->constvalue));
	}
	else if (((Const *) other)->consttype == type_oid(T_PERIODSET))
	{
		/* the right argument is a constant PERIODSET. We convert it into
		 * a period, which is its bounding box.
		 */
		constperiod =  periodset_bbox(
				DatumGetPeriodSet(((Const *) other)->constvalue));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	/*
	 * If we got a valid constant on one side of the operator, proceed to
	 * estimate using statistics. Otherwise punt and return a default constant
	 * estimate.  Note that calc_periodsel need not handle
	 * PERIOD_ELEM_CONTAINED_OP.
	 */
	if (constperiod)
		selec = calc_periodsel(&vardata, constperiod, operator);
	else
		selec = default_period_selectivity(operator);

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

static double
calc_periodsel(VariableStatData *vardata,
			  Period *constval, Oid operator)
{
	double		hist_selec;
	double		selec;
	float4		null_frac;

	/*
	 * First look up the fraction of NULLs and empty periods from pg_statistic.
	 */
	if (HeapTupleIsValid(vardata->statsTuple))
	{
		Form_pg_statistic stats;

		stats = (Form_pg_statistic) GETSTRUCT(vardata->statsTuple);
		null_frac = stats->stanullfrac;
	}
	else
	{
		/*
		 * No stats are available. Follow through the calculations below
		 * anyway, assuming no NULLs periods. This still allows us
		 * to give a better-than-nothing estimate.
		 */
		null_frac = 0.0;
	}

	/*
	 * Calculate selectivity using bound histograms. If that fails for
	 * some reason, e.g no histogram in pg_statistic, use the default
	 * constant estimate. This is still somewhat better than just 
	 * returning the default estimate, because this still takes into 
	 * account the fraction of NULL tuples, if we had statistics for them.
	 */
	hist_selec = calc_period_hist_selectivity(vardata, constval,
									   operator, DEFAULT_STATISTICS);
	if (hist_selec < 0.0)
		hist_selec = default_period_selectivity(operator);

	selec = hist_selec;

	/* all period operators are strict */
	selec *= (1.0 - null_frac);

	/* result should be in period, but make sure... */
	CLAMP_PROBABILITY(selec);

	return selec;
}
