/*****************************************************************************
 *
 * GeoEstimate.c
 *	  Functions for selectivity estimation of geometry/geography operators
 *
 * Estimates are based on histograms of lower and upper bounds, and the
 * fraction of empty periods.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <liblwgeom.h>
#include "TemporalTypes.h"
#include "TemporalPoint.h"

GBOX 
get_gbox(Node *node)
{
	GBOX gbox;
	Oid value_type = ((Const *) node)->consttype;

	if (value_type == type_oid(T_TGEOMPOINT) || 
		value_type == type_oid(T_TGEOGPOINT)) 
	{
		Temporal *temp = DatumGetTemporal(((Const *) node)->constvalue);
		temporal_bbox(&gbox, temp);
		return *gbox_copy(&gbox);
	} 
	else if (value_type == type_oid(T_GEOMETRY) || value_type == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(((Const *) node)->constvalue);
		if (gserialized_get_gbox_p(gs, &gbox) == LW_FAILURE)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Error while obtaining the bounding box of the geometry")));
		return gbox;
	}
	else if (value_type == type_oid(T_GBOX))
	{
		GBOX *gboxi;
		GBOX *gbox = DatumGetGboxP(((Const *) node)->constvalue);
		gboxi = gbox_copy(gbox);
		return *gboxi;
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Function get_gbox does not support this type")));
}

PG_FUNCTION_INFO_V1(overlaps_point_sel);

PGDLLEXPORT Datum
overlaps_point_sel(PG_FUNCTION_ARGS)
{

	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	bool selec2Flag = false;
	Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */

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

	GBOX box = get_gbox(other);

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

	selec1 = estimate_selectivity(&vardata, &box, OVERLAPS_OP);

	if(((Const *) other)->consttype == type_oid(T_TGEOMPOINT) || ((Const *) other)->consttype == type_oid(T_TGEOGPOINT))
	{
		((Const *) other)->constvalue = PointerGetDatum(period_make(box.mmin, box.mmax, true, true));
		selec2 = estimate_selectivity_temporal_dimension(root, vardata, other, OVERLAPS_OP);
		selec2Flag = true;
	}
	if(selec2 == 0.0 && !selec2Flag)
		selec = selec1;
	else
		selec = selec1 * selec2;

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}


PG_FUNCTION_INFO_V1(same_point_sel);

PGDLLEXPORT Datum
same_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	bool selec2Flag = false;
	Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */
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

	GBOX box = get_gbox(other);

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
	else
	{
		selec1 = estimate_selectivity(&vardata, &box, SAME_OP);
	}

	if(((Const *) other)->consttype == type_oid(T_TGEOMPOINT) || ((Const *) other)->consttype == type_oid(T_TGEOGPOINT))
	{
		((Const *) other)->constvalue = PointerGetDatum(period_make(box.mmin, box.mmin, true, true));
		selec2 = estimate_selectivity_temporal_dimension(root, vardata, other, SAME_OP);
		selec2Flag = true;
	}
	if(selec2 == 0.0 && !selec2Flag)
		selec = selec1;
	else
		selec = selec1 * selec2;

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}


PG_FUNCTION_INFO_V1(contains_point_sel);

PGDLLEXPORT Datum
contains_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	bool selec2Flag = false;
	Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */

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

	GBOX box = get_gbox(other);


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
		selec1 = estimate_selectivity(&vardata, &box, CONTAINED_OP);
	}
	else
	{
		selec1 = estimate_selectivity(&vardata, &box, CONTAINS_OP);
	}


	if(((Const *) other)->consttype == type_oid(T_TGEOMPOINT) || ((Const *) other)->consttype == type_oid(T_TGEOGPOINT))
	{
		((Const *) other)->constvalue = PointerGetDatum(period_make(box.mmin, box.mmax, true, true));
		selec2 = estimate_selectivity_temporal_dimension(root, vardata, other, CONTAINS_OP);
		selec2Flag = true;
	}
	if(selec2 == 0.0 && !selec2Flag)
		selec = selec1;
	else
		selec = selec1 * selec2;

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}


PG_FUNCTION_INFO_V1(contained_point_sel);

PGDLLEXPORT Datum
contained_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	bool selec2Flag = false;
	Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */

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

	GBOX box = get_gbox(other);


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
		selec1 = estimate_selectivity(&vardata, &box, CONTAINS_OP);
	}
	else
	{
		selec1 = estimate_selectivity(&vardata, &box, CONTAINED_OP);
	}


	if(((Const *) other)->consttype == type_oid(T_TGEOMPOINT) || ((Const *) other)->consttype == type_oid(T_TGEOGPOINT))
	{
		((Const *) other)->constvalue = PointerGetDatum(period_make(box.mmin, box.mmax, true, true));
		selec2 = estimate_selectivity_temporal_dimension(root, vardata, other, CONTAINED_OP);
		selec2Flag = true;
	}
	if(selec2 == 0.0 && !selec2Flag)
		selec = selec1;
	else
		selec = selec1 * selec2;

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(overabove_point_sel);

PGDLLEXPORT Datum
overabove_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.ymin = box.ymax;
	box.ymax = FLT_MAX;
	box.xmin = -FLT_MAX;
	box.xmax = FLT_MAX;

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
		selec = estimate_selectivity(&vardata, &box, BELOW_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, OVERABOVE_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(above_point_sel);

PGDLLEXPORT Datum
above_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.ymin = box.ymax;
	//box.ymax = FLT_MAX;
	box.xmin = -FLT_MAX;
	box.xmax = FLT_MAX;
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
		selec = estimate_selectivity(&vardata, &box, OVERBELOW_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, ABOVE_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(overbelow_point_sel);

PGDLLEXPORT Datum
overbelow_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.ymax = box.ymax;
	box.ymin = -FLT_MAX;
	box.xmin = -FLT_MAX;
	box.xmax = FLT_MAX;
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
		selec = estimate_selectivity(&vardata, &box, ABOVE_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, OVERBELOW_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(below_point_sel);

PGDLLEXPORT Datum
below_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.ymax = box.ymax;
	box.ymin = -FLT_MAX;
	box.xmin = -FLT_MAX;
	box.xmax = FLT_MAX;
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
		selec = estimate_selectivity(&vardata, &box, OVERABOVE_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, BELOW_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(overfront_point_sel);

PGDLLEXPORT Datum
overfront_point_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.01);
}

PG_FUNCTION_INFO_V1(front_point_sel);

PGDLLEXPORT Datum
front_point_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.01);
}

PG_FUNCTION_INFO_V1(overback_point_sel);

PGDLLEXPORT Datum
overback_point_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.01);
}

PG_FUNCTION_INFO_V1(back_point_sel);

PGDLLEXPORT Datum
back_point_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.01);
}

PG_FUNCTION_INFO_V1(overleft_point_sel);

PGDLLEXPORT Datum
overleft_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.xmax = box.xmin;
	box.ymin = -FLT_MAX;
	box.ymax = FLT_MAX;
	box.xmin = -FLT_MAX;

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
		selec = estimate_selectivity(&vardata, &box, RIGHT_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, OVERLEFT_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(left_point_sel);

PGDLLEXPORT Datum
left_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.xmax = box.xmin;
	box.ymin = -FLT_MAX;
	box.ymax = FLT_MAX;
	box.xmin = -FLT_MAX;
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
		selec = estimate_selectivity(&vardata, &box, OVERRIGHT_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, LEFT_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(overright_point_sel);

PGDLLEXPORT Datum
overright_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.xmin = box.xmax;
	//box.xmax = FLT_MAX;
	box.ymin = -FLT_MAX;
	box.ymax = FLT_MAX;

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
		selec = estimate_selectivity(&vardata, &box, LEFT_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, OVERRIGHT_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
}

PG_FUNCTION_INFO_V1(right_point_sel);

PGDLLEXPORT Datum
right_point_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	int varRelid = PG_GETARG_INT32(3);
	VariableStatData vardata;
	Node *other;
	bool varonleft;
	Selectivity selec = 0.0;

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

	GBOX box = get_gbox(other);
	box.xmax = box.xmin;
	box.ymin = -FLT_MAX;
	box.ymax = FLT_MAX;

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
		box.xmax = box.xmin;
		selec = estimate_selectivity(&vardata, &box, OVERLEFT_OP);
	}
	else
	{
		selec = estimate_selectivity(&vardata, &box, RIGHT_OP);
	}

	//bool valid = HeapTupleIsValid(vardata.statsTuple);

	if (selec < 0.0)
		selec = 0.01;

	ReleaseVariableStats(vardata);

	CLAMP_PROBABILITY(selec);

	PG_RETURN_FLOAT8((float8) selec);
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

/** Expand the bounds of target to include source */
int
nd_box_merge(const ND_BOX *source, ND_BOX *target)
{
	int d;
	for (d = 0; d < ND_DIMS; d++)
	{
		target->min[d] = Min(target->min[d], source->min[d]);
		target->max[d] = Max(target->max[d], source->max[d]);
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

/**
* Prepare an ND_BOX for bounds calculation:
* set the maxes to the smallest thing possible and
* the mins to the largest.
*/
int
nd_box_init_bounds(ND_BOX *a)
{
	int d;
	for (d = 0; d < ND_DIMS; d++)
	{
		a->min[d] = FLT_MAX;
		a->max[d] = -1 * FLT_MAX;
	}
	return true;
}

/** Set the values of an #ND_BOX from a #GBOX */
void
nd_box_from_gbox(const GBOX *gbox, ND_BOX *nd_box)
{
	int d = 0;

	nd_box_init(nd_box);
	nd_box->min[d] = gbox->xmin;
	nd_box->max[d] = gbox->xmax;
	d++;
	nd_box->min[d] = gbox->ymin;
	nd_box->max[d] = gbox->ymax;
	d++;
	if (FLAGS_GET_GEODETIC(gbox->flags))
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		return;
	}
	if (FLAGS_GET_Z(gbox->flags))
	{
		nd_box->min[d] = gbox->zmin;
		nd_box->max[d] = gbox->zmax;
		d++;
	}
	if (FLAGS_GET_M(gbox->flags))
	{
		nd_box->min[d] = gbox->mmin;
		nd_box->max[d] = gbox->mmax;
		d++;
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
* Expand an #ND_BOX ever so slightly. Expand parameter is the proportion
* of total width to add.
*/
int
nd_box_expand(ND_BOX *nd_box, double expansion_factor)
{
	int d;
	double size;
	for (d = 0; d < ND_DIMS; d++)
	{
		size = nd_box->max[d] - nd_box->min[d];
		if (size <= 0) continue;
		nd_box->min[d] -= size * expansion_factor / 2;
		nd_box->max[d] += size * expansion_factor / 2;
	}
	return true;
}


/**
* Calculate how much a set of boxes is homogenously distributed
* or contentrated within one dimension, returning the range_quintile of
* of the overlap counts per cell in a uniform
* partition of the extent of the dimension.
* A uniform distribution of counts will have a small range
* and will require few cells in a selectivity histogram.
* A diverse distribution of counts will have a larger range
* and require more cells in a selectivity histogram (to
* distinguish between areas of feature density and areas
* of feature sparseness. This measurement should help us
* identify cases like X/Y/Z data where there is lots of variability
* in density in X/Y (diversely in a multi-kilometer range) and far
* less in Z (in a few-hundred meter range).
*/
int nd_box_array_distribution(const ND_BOX **nd_boxes, int num_boxes, const ND_BOX *extent, int ndims,
							  double *distribution)
							  {
	/* How many bins shall we use in figuring out the distribution? */
	static int num_bins = 50;
	int d, i, k, range;
	int counts[num_bins];
	double smin, smax;   /* Spatial min, spatial max */
	double swidth;	   /* Spatial width of dimension */
#if POSTGIS_DEBUG_LEVEL >= 3
	double average, sdev, sdev_ratio;
#endif
	int bmin, bmax;   /* Bin min, bin max */
	const ND_BOX *ndb;

	/* For each dimension... */
	for (d = 0; d < ndims; d++)
	{
		/* Initialize counts for this dimension */
		memset(counts, 0, sizeof(int) * num_bins);

		smin = extent->min[d];
		smax = extent->max[d];
		swidth = smax - smin;

		/* Don't try and calculate distribution of overly narrow dimensions */
		if (swidth < MIN_DIMENSION_WIDTH)
		{
			distribution[d] = 0;
			continue;
		}

		/* Sum up the overlaps of each feature with the dimensional bins */
		for (i = 0; i < num_boxes; i++)
		{
			double minoffset, maxoffset;

			/* Skip null entries */
			ndb = nd_boxes[i];
			if (!ndb) continue;

			/* Where does box fall relative to the working range */
			minoffset = ndb->min[d] - smin;
			maxoffset = ndb->max[d] - smin;

			/* Skip boxes that our outside our working range */
			if (minoffset < 0 || minoffset > swidth ||
				maxoffset < 0 || maxoffset > swidth)
			{
				continue;
			}

			/* What bins does this range correspond to? */
			bmin = num_bins * (minoffset) / swidth;
			bmax = num_bins * (maxoffset) / swidth;


			/* Increment the counts in all the bins this feature overlaps */
			for (k = bmin; k <= bmax; k++)
			{
				counts[k] += 1;
			}

		}

		/* How dispersed is the distribution of features across bins? */
		range = range_quintile(counts, num_bins);

#if POSTGIS_DEBUG_LEVEL >= 3
		average = avg(counts, num_bins);
		sdev = stddev(counts, num_bins);
		sdev_ratio = sdev/average;
#endif

		distribution[d] = range;
	}

	return true;
}

/**
* The difference between the fourth and first quintile values,
* the "inter-quintile range"
*/
int
range_quintile(int *vals, int nvals)
{
	qsort(vals, nvals, sizeof(int), cmp_int);
	return vals[4 * nvals / 5] - vals[nvals / 5];
}


/**
* Integer comparison function for qsort
*/
int
cmp_int(const void *a, const void *b)
{
	int ia = *((const int *) a);
	int ib = *((const int *) b);

	if (ia == ib)
		return 0;
	else if (ia > ib)
		return 1;
	else
		return -1;
}

/**
* Given double array, return sum of values.
*/
double
total_double(const double *vals, int nvals) 
{
	int i;
	float total = 0;
	/* Calculate total */
	for (i = 0; i < nvals; i++)
		total += vals[i];

	return total;
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


float8
estimate_selectivity(VariableStatData *vardata, const GBOX *box, CachedOp op)
{
	int d; /* counter */
	float8 selectivity;
	ND_BOX nd_box;
	ND_IBOX nd_ibox;
	int at[ND_DIMS];
	double cell_size[ND_DIMS];
	double min[ND_DIMS];
	double max[ND_DIMS];

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
	nd_box_from_gbox(box, &nd_box);

	/*
	 * To return 2D stats on an ND sample, we need to make the
	 * 2D box cover the full range of the other dimensions in the
	 * histogram.
	 */

	int ndims_max = 2;

	/*
	 * Search box completely misses histogram extent?
	 * We have to intersect in all N dimensions or else we have
	 * zero interaction under the &&& operator. It's important
	 * to short circuit in this case, as some of the tests below
	 * will return junk results when run on non-intersecting inputs.
	 */
	if (( op == OVERLAPS_OP || op == CONTAINS_OP || op == CONTAINED_OP) &&
		!nd_box_intersects(&nd_box, &(nd_stats->extent), ndims_max))
	{
		return 0.0;
	}

	/* Search box completely contains histogram extent! */
	if ( nd_box_contains(&nd_box, &(nd_stats->extent), ndims_max) )
	{
		return 1.0;
	}

	/* Calculate the overlap of the box on the histogram */
	if ((op == OVERLAPS_OP || op == CONTAINS_OP || op == CONTAINED_OP) &&
		!nd_box_overlap(nd_stats, &nd_box, &nd_ibox))
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

		/* Initialize the counter */
		at[d] = nd_ibox.min[d];
	}


	switch (op)
	{
		case CONTAINS_OP:
		{
			selectivity = FLT_MIN;
			double maxx;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell = {{0.0, 0.0, 0.0, 0.0},
								  {0.0, 0.0, 0.0, 0.0}};

				/* We have to pro-rate partially overlapped cells. */
				for (d = 0; d < nd_stats->ndims; d++)
				{
					nd_cell.min[d] = min[d] + (at[d] + 0) * cell_size[d];
					nd_cell.max[d] = min[d] + (at[d] + 1) * cell_size[d];
				}

				ratio = nd_box_ratio(&nd_box, &nd_cell, nd_stats->ndims);
				cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

				maxx = cell_count * ratio;
				if (selectivity < maxx)
					selectivity = maxx;
			} while (nd_increment(&nd_ibox, nd_stats->ndims, at));
			return selectivity;
		}
		case CONTAINED_OP:
		{
			selectivity = FLT_MAX;
			double minx;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell = {{0.0, 0.0, 0.0, 0.0},
								  {0.0, 0.0, 0.0, 0.0}};

				/* We have to pro-rate partially overlapped cells. */
				for (d = 0; d < nd_stats->ndims; d++)
				{
					nd_cell.min[d] = min[d] + (at[d] + 0) * cell_size[d];
					nd_cell.max[d] = min[d] + (at[d] + 1) * cell_size[d];
				}

				ratio = nd_box_ratio(&nd_box, &nd_cell, nd_stats->ndims);
				cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];

				minx = cell_count * ratio;
				if (selectivity > minx)
					selectivity = minx;
			} while (nd_increment(&nd_ibox, nd_stats->ndims, at));
			return selectivity;
		}
		case OVERLAPS_OP:
		case SAME_OP:
		case OVERABOVE_OP:
		case OVERBELOW_OP:
		{
			double total_count = 0.0;
			do
			{
				float cell_count, ratio;
				ND_BOX nd_cell;

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
			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0 ) selectivity = 0.0;

			return selectivity;
		}
		case LEFT_OP:
		{
			d = 0;
			double total_count = 0.0;
			/* Initialize the box with the stored nd size */
			nd_ibox.min[0] = 0;nd_ibox.max[0] = (int) nd_stats->size[0] - 1;
			nd_ibox.min[1] = 0;nd_ibox.max[1] = (int) nd_stats->size[1] - 1;
			at[0] = 0; at[1] = 0;
			/* The default case: the box is on the left side of the right side.
			 * We compare with the max value of the x dimension plus 0.5. The number is increased by 0.5 to be able
			 * to get the right comparison with the stored box.
			 * This is because PostGIS added 0.5 to the borders of the box.
			 */

			if (max[d] < (nd_box.max[d] + 0.5))
				total_count = nd_stats->histogram_features;
			else
			{
				do
				{
					float cell_count, ratio;
					ND_BOX nd_cell;
					/* Calculate the new bounds for only the x dimension */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);
					/* There are three cases:
					 * (1) The max value of the x dimension is on the left,
					 * so we need to get the whole number of the cell-count value.
					 * (2) The min value of the x dimension is on the left,
					 * so we need to scan the box.
					 * (3) Otherwise, the ratio is 0.
					 * */
					if (nd_cell.max[d] < nd_box.max[d] + 0.5)
					{
						ratio = 1.0;
						//int index = at[d];
						//for (int i = 0; i < nd_stats->size[d]; i++)
						//{
							cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];
						//	index += (int) nd_stats->size[1];
						//}
						total_count += cell_count * ratio;
					}
					else if (nd_cell.min[d] < nd_box.max[d])
					{
						cell_count = nd_stats->value[nd_stats_value_index(nd_stats, at)];
						double width = nd_cell.max[0] - nd_cell.min[0];
						double step = width / (cell_count);
						double newMax = nd_cell.max[0];
						int count = 0;
						do
						{
							newMax -= step;
							if(newMax < nd_box.max[0])
								break;
							count++;
						} while(true);
						ratio = 1 - count / (cell_count);
						total_count += cell_count * ratio;
					}
					else
						total_count += 0;
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}

			/* Scale by the number of features in our histogram to get the proportion */
			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0) selectivity = 0.0;

			return selectivity;
		}
		case RIGHT_OP:
		{
			d = 0;
			double total_count = 0.0;
			/* Initialize the box with the stored nd size */
			nd_ibox.min[0] = 0;nd_ibox.max[0] = (int) nd_stats->size[0] - 1;
			nd_ibox.min[1] = 0;nd_ibox.max[1] = 0;
			at[0] = 0; at[1] = 0;
			/* The default case: the box is on the left side of the right side.
			 * We compare with the max value of the x dimension plus 0.5. The number is increased by 0.5 to be able
			 * to get the right comparison with the stored box.
			 * This is because PostGIS added 0.5 to the borders of the box.
			 */

			if (min[d] > (nd_box.max[d] + 0.5))
				total_count = nd_stats->histogram_features;
			else
			{
				do
				{
					float cell_count, ratio;
					ND_BOX nd_cell;
					int cells_count = 0;
					/* Calculate the new bounds for only the x dimension */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);
					/* There are three cases:
					 * (1) The max value of the x dimension is on the left,
					 * so we need to get the whole number of the cell-count value.
					 * (2) The min value of the x dimension is on the left,
					 * so we need to scan the box.
					 * (3) Otherwise, the ratio is 0.
					 * */
					if (nd_cell.min[d] >= (nd_box.max[d] + 0.5))
					{
						ratio = 1.0;
						int index = at[d];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cells_count += nd_stats->value[index];
							index += (int) nd_stats->size[0];
						}
						total_count += cells_count * ratio;
					}
					else if (nd_cell.max[d] > (nd_box.max[d] + 0.5))
					{
						int index = at[0];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cell_count = nd_stats->value[index];
							float4 step = (nd_cell.max[d] - nd_cell.min[d]) / cell_count;
							float4 total = nd_cell.min[d] + step;
							float4 c = 0;
							for (int i = 1; i <= cell_count; i++)
							{
								if (total >= nd_box.max[d])
									c++;
								total += step;
							}
							total_count += c;
							index += (int) nd_stats->size[0];
						}
					}
					else
						total_count += 0;
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}

			/* Scale by the number of features in our histogram to get the proportion */

			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0) selectivity = 0.0;

			return selectivity;
		}
		case OVERLEFT_OP:
		{
			d = 0;
			double total_count = 0.0;
			nd_ibox.min[0] = 0;nd_ibox.max[0] = (int) nd_stats->size[0];
			nd_ibox.min[1] = 0;nd_ibox.max[1] = 0;
			at[0] = 0; at[1] = 0;

			if (max[d] <= (nd_box.max[d] + 0.5))
				total_count = nd_stats->histogram_features;
			else
			{
				do {
					float cell_count, ratio;
					ND_BOX nd_cell;
					int cells_count = 0;

					/* We have to pro-rate partially overlapped cells. */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);

					if (nd_cell.max[0] <= (nd_box.max[0] + 0.5)) {
						ratio = 1;
						int index = at[0];
						for (int i = 0; i < nd_stats->size[1]; i++) {
							cells_count += nd_stats->value[index];
							index += (int) nd_stats->size[0];
						}
						total_count += cells_count * ratio;
					}
					else if (nd_cell.min[0] <= (nd_box.max[0] + 0.5))
					{
						int index = at[0];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cell_count = nd_stats->value[index];
							float4 step = (nd_cell.max[0] - nd_cell.min[0]) / cell_count;
							float4 total = nd_cell.min[0] + step;
							float4 c = 0;
							for (int i = 1; i < cell_count; i++)
							{
								if (total <= nd_box.max[0])
									c++;
								total += step;
							}
							total_count += c;
							index += (int) nd_stats->size[0];
						}
					}
					else
					   total_count += 0;
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}
			/* Scale by the number of features in our histogram to get the proportion */
			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0 ) selectivity = 0.0;

			return selectivity;
		}
		case OVERRIGHT_OP:
		{
			d = 0;
			double total_count = 0.0;
			/*
			 * We only need to set the x dimension
			 */
			nd_ibox.min[0] = 0;nd_ibox.max[0] = (int) nd_stats->size[0] - 1;
			nd_ibox.min[1] = 0;nd_ibox.max[1] = 0;
			at[0] = 0; at[1] = 0;

			if (min[d] >= (nd_box.max[d]))
				total_count = nd_stats->histogram_features;
			else
			{
				do {
					float cell_count, ratio;
					ND_BOX nd_cell;
					int cells_count = 0;

					/* We have to pro-rate partially overlapped cells. */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);

					if (nd_cell.min[0] >= (nd_box.max[0] + 0.5))
					{
						ratio = 1;
						int index = at[0];
						for (int i = 0; i < nd_stats->size[1]; i++) {
							cells_count += nd_stats->value[index];
							index += (int) nd_stats->size[0];
						}
						total_count += cells_count * ratio;
					} else if (nd_cell.max[0] > (nd_box.max[0] + 0.5))
					{
						int index = at[0];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cell_count = nd_stats->value[index];
							float4 step = (nd_cell.max[0] - nd_cell.min[0]) / cell_count;
							float4 total = nd_cell.min[0] + step;
							float4 c = 0;
							for (int i = 1; i <= cell_count; i++) {
								if (total >= nd_box.max[0])
									c++;
								total += step;
							}
							total_count += c;
							index += (int) nd_stats->size[0];
						}
					}
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}
			/* Scale by the number of features in our histogram to get the proportion */
			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0 ) selectivity = 0.0;

			return selectivity;
		}
		case BELOW_OP:
		{
			d = 1;
			double total_count = 0.0;
			/* Initialize the box with the stored nd size */
			nd_ibox.min[0] = 0;nd_ibox.max[0] = 0;
			nd_ibox.min[1] = 0;nd_ibox.max[1] = (int) nd_stats->size[0] - 1;
			at[0] = 0; at[1] = 0;
			/* The default case: the box is on the left side of the right side.
			 * We compare with the max value of the x dimension plus 0.5. The number is increased by 0.5 to be able
			 * to get the right comparison with the stored box.
			 * This is because PostGIS added 0.5 to the borders of the box.
			 */

			if (max[d] < (nd_box.max[d] + 0.5))
				total_count = nd_stats->histogram_features;
			else
			{
				do
				{
					float cell_count, ratio;
					ND_BOX nd_cell;
					int cells_count = 0;
					/* Calculate the new bounds for only the x dimension */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);
					/* There are three cases:
					 * (1) The max value of the x dimension is on the left,
					 * so we need to get the whole number of the cell-count value.
					 * (2) The min value of the x dimension is on the left,
					 * so we need to scan the box.
					 * (3) Otherwise, the ratio is 0.
					 * */
					if (nd_cell.max[d] < nd_box.max[d] + 0.5)
					{
						ratio = 1.0;
						int index = at[d];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cells_count += nd_stats->value[index];
							index ++;
						}
						total_count += cells_count * ratio;
					}
					else if (nd_cell.min[d] < nd_box.max[d])
					{
						int index = at[d];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cell_count = nd_stats->value[index];
							float4 step = (nd_cell.max[d] - nd_cell.min[d]) / cell_count;
							float4 total = nd_cell.min[d] + step;
							float4 c = 0;
							for (int i = 1; i < cell_count; i++)
							{
								if (total < nd_box.max[d])
									c++;
								total += step;
							}
							total_count += c;
							index ++;
						}
					}
					else
						total_count += 0;
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}

			/* Scale by the number of features in our histogram to get the proportion */

			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0) selectivity = 0.0;

			return selectivity;
		}
		case ABOVE_OP:
		{
			d = 1;
			double total_count = 0.0;
			/* Initialize the box with the stored nd size */
			nd_ibox.min[0] = 0;nd_ibox.max[0] = 0;
			nd_ibox.min[1] = 0;nd_ibox.max[1] = (int) nd_stats->size[1] - 1;
			at[0] = 0; at[1] = 0;
			/* The default case: the box is on the left side of the right side.
			 * We compare with the max value of the x dimension plus 0.5. The number is increased by 0.5 to be able
			 * to get the right comparison with the stored box.
			 * This is because PostGIS added 0.5 to the borders of the box.
			 */

			if (min[d] > (nd_box.max[d] + 0.5))
				total_count = nd_stats->histogram_features;
			else
			{
				do
				{
					float cell_count, ratio;
					ND_BOX nd_cell;
					int cells_count = 0;
					/* Calculate the new bounds for only the x dimension */
					nd_cell.min[d] = (float4) (min[d] + (at[d] + 0) * cell_size[d]);
					nd_cell.max[d] = (float4) (min[d] + (at[d] + 1) * cell_size[d]);
					/* There are three cases:
					 * (1) The max value of the x dimension is on the left,
					 * so we need to get the whole number of the cell-count value.
					 * (2) The min value of the x dimension is on the left,
					 * so we need to scan the box.
					 * (3) Otherwise, the ratio is 0.
					 * */
					if (nd_cell.min[d] >= (nd_box.max[d] + 0.5))
					{
						ratio = 1.0;
						int index = at[d];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cells_count += nd_stats->value[index];
							index ++;
						}
						total_count += cells_count * ratio;
					}
					else if (nd_cell.max[d] > (nd_box.max[d]))
					{
						int index = at[d];
						for (int i = 0; i < nd_stats->size[1]; i++)
						{
							cell_count = nd_stats->value[index];
							float4 step = (nd_cell.max[d] - nd_cell.min[d]) / cell_count;
							float4 total = nd_cell.min[d] + step;
							float4 c = 0;
							for (int i = 1; i < cell_count; i++)
							{
								if (total >= nd_box.max[d])
									c++;
								total += step;
							}
							total_count += c;
							index ++;
						}
					}
					else
						total_count += 0;
				} while (nd_increment(&nd_ibox, (int) nd_stats->ndims, at));
			}

			/* Scale by the number of features in our histogram to get the proportion */

			selectivity = total_count / nd_stats->histogram_features;
			/* Prevent rounding overflows */
			if (selectivity > 1.0) selectivity = 1.0;
			else if (selectivity < 0.0) selectivity = 0.0;

			return selectivity;
		}


		default:
			return 0;
	}
}

/**
* Given that geodetic boxes are X/Y/Z regardless of the
* underlying geometry dimensionality and other boxes
* are guided by HAS_Z/HAS_M in their dimesionality,
* we have a little utility function to make it easy.
*/
int
gbox_ndims(const GBOX *gbox)
{
	int dims = 2;
	if (FLAGS_GET_GEODETIC(gbox->flags))
		return 3;
	if (FLAGS_GET_Z(gbox->flags))
		dims++;
	if (FLAGS_GET_M(gbox->flags))
		dims++;
	return dims;
}

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
			selec = bbox_overlaps_sel_internal(root, vardata, constantData);
			break;
		case SAME_OP:
			selec = bbox_same_sel_internal(root, vardata, constantData);
			break;
		case CONTAINS_OP:
			selec = bbox_contains_sel_internal(root, vardata, constantData);
			break;
		case CONTAINED_OP:
			selec = bbox_contained_sel_internal(root, vardata, constantData);
			break;
		default:
			selec = 0.0;
	}

	if (selec < 0.0)
		selec = 0.1;

	return selec;
}

/**
 * Join selectivity functions
 */

PG_FUNCTION_INFO_V1(tpoint_join_sel);

PGDLLEXPORT Datum
tpoint_join_sel(PG_FUNCTION_ARGS)
{
	PlannerInfo *root = (PlannerInfo *) PG_GETARG_POINTER(0);
	Oid operator = PG_GETARG_OID(1);
	List *args = (List *) PG_GETARG_POINTER(2);
	JoinType jointype = (JoinType) PG_GETARG_INT16(3);
	SpecialJoinInfo *sjinfo = (SpecialJoinInfo *) PG_GETARG_POINTER(4);
	int mode = 2;

	Node *arg1, *arg2;
	Var *var1, *var2;
	Oid relid1, relid2;

	ND_STATS *stats1, *stats2;
	bool selec2Flag = false;
	Selectivity selec1 = 0.0, selec2 = 0.0, selec = 0.0; /* keep compiler quiet */

	/* Only respond to an inner join/unknown context join */
	if (jointype != JOIN_INNER)
	{
		elog(DEBUG1, "%s: jointype %d not supported", __func__, jointype);
		PG_RETURN_FLOAT8(DEFAULT_ND_JOINSEL);
	}

	/* Find Oids of the geometry columns we are working with */
	arg1 = (Node*) linitial(args);
	arg2 = (Node*) lsecond(args);
	var1 = (Var*) arg1;
	var2 = (Var*) arg2;

	/* We only do column joins right now, no functional joins */
	/* TODO: handle g1 && ST_Expand(g2) */
	if (!IsA(arg1, Var) || !IsA(arg2, Var))
	{
		elog(DEBUG1, "%s called with arguments that are not column references", __func__);
		PG_RETURN_FLOAT8(DEFAULT_ND_JOINSEL);
	}

	/* What are the Oids of our tables/relations? */
	relid1 = rt_fetch(var1->varno, root->parse->rtable)->relid;
	relid2 = rt_fetch(var2->varno, root->parse->rtable)->relid;

	/* Pull the stats from the stats system. */
	stats1 = pg_get_nd_stats(relid1, var1->varattno, mode, false);
	stats2 = pg_get_nd_stats(relid2, var2->varattno, mode, false);

	/* If we can't get stats, we have to stop here! */
	if (stats1 && stats2)
		selec1 = estimate_join_selectivity(stats1, stats2);
	else
		PG_RETURN_FLOAT8(selec);

	if (var1->vartype == type_oid(T_TGEOMPOINT) || var1->vartype == type_oid(T_TGEOGPOINT))
	{
		selec2 = estimate_join_selectivity_temporal_dimension(root, args, sjinfo, operator, get_cacheOp(operator));
		if(selec2 != 0)
			selec2Flag = true;
	}
	if (selec2 == 0 && !selec2Flag)
		selec = selec1;
	else if(selec1 == 0.0 && selec2Flag)
		selec = selec2;
	else if(selec1 == 0.0 && selec2 == 0.0)
		PG_RETURN_FLOAT8(DEFAULT_ND_JOINSEL);
	else
		selec = selec1 * selec2;

	if(stats1 && stats2)
	{
		pfree(stats1);
		pfree(stats2);
	}
	PG_RETURN_FLOAT8(selec);
}


double
check_mcv(PlannerInfo *root, CachedOp cacheOp,
		  VariableStatData *vardata1, VariableStatData *vardata2,
		  double nd1, double nd2,
		  AttStatsSlot *sslot1, AttStatsSlot *sslot2,
		  Form_pg_statistic stats1, Form_pg_statistic stats2,
		  bool have_mcvs1, bool have_mcvs2)
{
	double	  selec;
	TypeCacheEntry *typecache = lookup_type_cache(TIMESTAMPTZOID,
												  TYPECACHE_EQ_OPR |
												  TYPECACHE_CMP_PROC_FINFO |
												  TYPECACHE_HASH_PROC_FINFO);

	if (have_mcvs1 && have_mcvs2)
	{
		/*
		 * We have most-common-value lists for both relations.  Run through
		 * the lists to see which MCVs actually join to each other with the
		 * given operator.  This allows us to determine the exact join
		 * selectivity for the portion of the relations represented by the MCV
		 * lists.  We still have to estimate for the remaining population, but
		 * in a skewed distribution this gives us a big leg up in accuracy.
		 * For motivation see the analysis in Y. Ioannidis and S.
		 * Christodoulakis, "On the propagation of errors in the size of join
		 * results", Technical Report 1018, Computer Science Dept., University
		 * of Wisconsin, Madison, March 1991 (available from ftp.cs.wisc.edu).
		 */

		bool	   *hasmatch1;
		bool	   *hasmatch2;
		double	  nullfrac1 = stats1->stanullfrac;
		double	  nullfrac2 = stats2->stanullfrac;
		double	  matchprodfreq,
				matchfreq1,
				matchfreq2,
				unmatchfreq1,
				unmatchfreq2,
				otherfreq1,
				otherfreq2,
				totalsel1,
				totalsel2;
		int		 i,
				nmatches;

		hasmatch1 = (bool *) palloc0(sslot1->nvalues * sizeof(bool));
		hasmatch2 = (bool *) palloc0(sslot2->nvalues * sizeof(bool));

		/*
		 * Note we assume that each MCV will match at most one member of the
		 * other MCV list.  If the operator isn't really equality, there could
		 * be multiple matches --- but we don't look for them, both for speed
		 * and because the math wouldn't add up...
		 */
		matchprodfreq = 0.0;
		nmatches = 0;
		for (i = 0; i < sslot1->nvalues; i++)
		{
			int		 j;

			for (j = 0; j < sslot2->nvalues; j++)
			{
				if (hasmatch2[j])
					continue;

				if (DatumGetBool(FunctionCall2Coll(&typecache->cmp_proc_finfo,
												   DEFAULT_COLLATION_OID,
												   sslot1->values[i],
												   sslot2->values[j])))
				{
					hasmatch1[i] = hasmatch2[j] = true;
					matchprodfreq += sslot1->numbers[i] * sslot2->numbers[j];
					nmatches++;
					break;
				}
			}
		}
		CLAMP_PROBABILITY(matchprodfreq);
		/* Sum up frequencies of matched and unmatched MCVs */
		matchfreq1 = unmatchfreq1 = 0.0;
		for (i = 0; i < sslot1->nvalues; i++)
		{
			if (hasmatch1[i])
				matchfreq1 += sslot1->numbers[i];
			else
				unmatchfreq1 += sslot1->numbers[i];
		}
		CLAMP_PROBABILITY(matchfreq1);
		CLAMP_PROBABILITY(unmatchfreq1);
		matchfreq2 = unmatchfreq2 = 0.0;
		for (i = 0; i < sslot2->nvalues; i++)
		{
			if (hasmatch2[i])
				matchfreq2 += sslot2->numbers[i];
			else
				unmatchfreq2 += sslot2->numbers[i];
		}
		CLAMP_PROBABILITY(matchfreq2);
		CLAMP_PROBABILITY(unmatchfreq2);
		pfree(hasmatch1);
		pfree(hasmatch2);

		/*
		 * Compute total frequency of non-null values that are not in the MCV
		 * lists.
		 */
		otherfreq1 = 1.0 - nullfrac1 - matchfreq1 - unmatchfreq1;
		otherfreq2 = 1.0 - nullfrac2 - matchfreq2 - unmatchfreq2;
		CLAMP_PROBABILITY(otherfreq1);
		CLAMP_PROBABILITY(otherfreq2);

		/*
		 * We can estimate the total selectivity from the point of view of
		 * relation 1 as: the known selectivity for matched MCVs, plus
		 * unmatched MCVs that are assumed to match against random members of
		 * relation 2's non-MCV population, plus non-MCV values that are
		 * assumed to match against random members of relation 2's unmatched
		 * MCVs plus non-MCV values.
		 */
		totalsel1 = matchprodfreq;
		if (nd2 > sslot2->nvalues)
			totalsel1 += unmatchfreq1 * otherfreq2 / (nd2 - sslot2->nvalues);
		if (nd2 > nmatches)
			totalsel1 += otherfreq1 * (otherfreq2 + unmatchfreq2) /
						 (nd2 - nmatches);
		/* Same estimate from the point of view of relation 2. */
		totalsel2 = matchprodfreq;
		if (nd1 > sslot1->nvalues)
			totalsel2 += unmatchfreq2 * otherfreq1 / (nd1 - sslot1->nvalues);
		if (nd1 > nmatches)
			totalsel2 += otherfreq2 * (otherfreq1 + unmatchfreq1) /
						 (nd1 - nmatches);

		/*
		 * Use the smaller of the two estimates.  This can be justified in
		 * essentially the same terms as given below for the no-stats case: to
		 * a first approximation, we are estimating from the point of view of
		 * the relation with smaller nd.
		 */
		selec = (totalsel1 < totalsel2) ? totalsel1 : totalsel2;
	}
	else
	{
		/*
		 * We do not have MCV lists. Estimate the join selectivity by using
		 * Histogram.
		 */
			StrategyNumber  strategyNumber;
			AttStatsSlot hslot1, hslot2;
			if (vardata2->vartype == type_oid(T_TGEOMPOINT) || vardata2->vartype == type_oid(T_TGEOGPOINT))
				strategyNumber = TEMPORAL_STATISTICS;
			else
				strategyNumber = DEFAULT_STATISTICS;
			get_attstatsslot_internal(&hslot1, vardata1->statsTuple,
									  STATISTIC_KIND_BOUNDS_HISTOGRAM, InvalidOid,
									  ATTSTATSSLOT_VALUES, TEMPORAL_STATISTICS);
			get_attstatsslot_internal(&hslot2, vardata2->statsTuple,
									  STATISTIC_KIND_BOUNDS_HISTOGRAM, InvalidOid,
									  ATTSTATSSLOT_VALUES, strategyNumber);
			selec = timestamp_join_sel(hslot1, hslot2, cacheOp);
			CLAMP_PROBABILITY(selec);

		/*
		 * We do not have MCV lists for both sides.  Estimate the join
		 * selectivity as MIN(1/nd1,1/nd2)*(1-nullfrac1)*(1-nullfrac2). This
		 * is plausible if we assume that the join operator is strict and the
		 * non-null values are about equally distributed: a given non-null
		 * tuple of rel1 will join to either zero or N2*(1-nullfrac2)/nd2 rows
		 * of rel2, so total join rows are at most
		 * N1*(1-nullfrac1)*N2*(1-nullfrac2)/nd2 giving a join selectivity of
		 * not more than (1-nullfrac1)*(1-nullfrac2)/nd2. By the same logic it
		 * is not more than (1-nullfrac1)*(1-nullfrac2)/nd1, so the expression
		 * with MIN() is an upper bound.  Using the MIN() means we estimate
		 * from the point of view of the relation with smaller nd (since the
		 * larger nd is determining the MIN).  It is reasonable to assume that
		 * most tuples in this rel will have join partners, so the bound is
		 * probably reasonably tight and should be taken as-is.
		 *
		 * XXX Can we be smarter if we have an MCV list for just one side? It
		 * seems that if we assume equal distribution for the other side, we
		 * end up with the same answer anyway.
		 */
		//double	  nullfrac1 = stats1 ? stats1->stanullfrac : 0.0;
		//double	  nullfrac2 = stats2 ? stats2->stanullfrac : 0.0;

		/*selec = (1.0 - nullfrac1) * (1.0 - nullfrac2);
		if (nd1 > nd2)
			selec /= nd1;
		else
			selec /= nd2;*/
	}

	return selec;
}

float8
estimate_join_selectivity(const ND_STATS *s1, const ND_STATS *s2)
{
	int ncells1, ncells2;
	int ndims1, ndims2, ndims;
	double ntuples_max;
	double ntuples_not_null1, ntuples_not_null2;

	ND_BOX extent1, extent2;
	ND_IBOX ibox1, ibox2;
	int at1[ND_DIMS];
	int at2[ND_DIMS];
	double min1[ND_DIMS];
	double width1[ND_DIMS];
	double cellsize1[ND_DIMS];
	int size2[ND_DIMS];
	double min2[ND_DIMS];
	double width2[ND_DIMS];
	double cellsize2[ND_DIMS];
	int size1[ND_DIMS];
	int d;
	double val = 0;
	float8 selectivity;

	/* Drop out on null inputs */
	if ( ! ( s1 && s2 ) )
	{
		elog(NOTICE, " estimate_join_selectivity called with null inputs");
		return FALLBACK_ND_SEL;
	}

	/* We need to know how many cells each side has... */
	ncells1 = (int)roundf(s1->histogram_cells);
	ncells2 = (int)roundf(s2->histogram_cells);

	/* ...so that we can drive the summation loop with the smaller histogram. */
	if ( ncells1 > ncells2 )
	{
		const ND_STATS *stats_tmp = s1;
		s1 = s2;
		s2 = stats_tmp;
	}

	/* Re-read that info after the swap */
	ncells1 = (int)roundf(s1->histogram_cells);
	ncells2 = (int)roundf(s2->histogram_cells);

	/* Q: What's the largest possible join size these relations can create? */
	/* A: The product of the # of non-null rows in each relation. */
	ntuples_not_null1 = s1->table_features * (s1->not_null_features / s1->sample_features);
	ntuples_not_null2 = s2->table_features * (s2->not_null_features / s2->sample_features);
	ntuples_max = ntuples_not_null1 * ntuples_not_null2;

	/* Get the ndims as ints */
	ndims1 = (int)roundf(s1->ndims);
	ndims2 = (int)roundf(s2->ndims);
	ndims = Max(ndims1, ndims2);

	/* Get the extents */
	extent1 = s1->extent;
	extent2 = s2->extent;

	/* If relation stats do not intersect, join is very very selective. */
	if ( ! nd_box_intersects(&extent1, &extent2, ndims) )
	{
		PG_RETURN_FLOAT8(0.0);
	}

	/*
	 * First find the index range of the part of the smaller
	 * histogram that overlaps the larger one.
	 */
	if ( ! nd_box_overlap(s1, &extent2, &ibox1) )
	{
		PG_RETURN_FLOAT8(FALLBACK_ND_JOINSEL);
	}

	/* Initialize counters / constants on s1 */
	for ( d = 0; d < ndims1; d++ )
	{
		at1[d] = ibox1.min[d];
		min1[d] = s1->extent.min[d];
		width1[d] = s1->extent.max[d] - s1->extent.min[d];
		size1[d] = (int)roundf(s1->size[d]);
		cellsize1[d] = width1[d] / size1[d];
	}

	/* Initialize counters / constants on s2 */
	for ( d = 0; d < ndims2; d++ )
	{
		min2[d] = s2->extent.min[d];
		width2[d] = s2->extent.max[d] - s2->extent.min[d];
		size2[d] = (int)roundf(s2->size[d]);
		cellsize2[d] = width2[d] / size2[d];
	}

	/* For each affected cell of s1... */
	do
	{
		double val1;
		/* Construct the bounds of this cell */
		ND_BOX nd_cell1;
		nd_box_init(&nd_cell1);
		for ( d = 0; d < ndims1; d++ )
		{
			nd_cell1.min[d] = min1[d] + (at1[d]+0) * cellsize1[d];
			nd_cell1.max[d] = min1[d] + (at1[d]+1) * cellsize1[d];
		}

		/* Find the cells of s2 that cell1 overlaps.. */
		nd_box_overlap(s2, &nd_cell1, &ibox2);

		/* Initialize counter */
		for ( d = 0; d < ndims2; d++ )
		{
			at2[d] = ibox2.min[d];
		}

		/* Get the value at this cell */
		val1 = s1->value[nd_stats_value_index(s1, at1)];

		/* For each overlapped cell of s2... */
		do
		{
			double ratio2;
			double val2;

			/* Construct the bounds of this cell */
			ND_BOX nd_cell2;
			nd_box_init(&nd_cell2);
			for ( d = 0; d < ndims2; d++ )
			{
				nd_cell2.min[d] = min2[d] + (at2[d]+0) * cellsize2[d];
				nd_cell2.max[d] = min2[d] + (at2[d]+1) * cellsize2[d];
			}


			/* Calculate overlap ratio of the cells */
			ratio2 = nd_box_ratio(&nd_cell1, &nd_cell2, Max(ndims1, ndims2));

			/* Multiply the cell counts, scaled by overlap ratio */
			val2 = s2->value[nd_stats_value_index(s2, at2)];
			val += val1 * (val2 * ratio2);
		}
		while ( nd_increment(&ibox2, ndims2, at2) );

	}
	while( nd_increment(&ibox1, ndims1, at1) );


	/*
	 * In order to compare our total cell count "val" to the
	 * ntuples_max, we need to scale val up to reflect a full
	 * table estimate. So, multiply by ratio of table size to
	 * sample size.
	 */
	val *= (s1->table_features / s1->sample_features);
	val *= (s2->table_features / s2->sample_features);


	/*
	 * Because the cell counts are over-determined due to
	 * double counting of features that overlap multiple cells
	 * (see the compute_gserialized_stats routine)
	 * we also have to scale our cell count "val" *down*
	 * to adjust for the double counting.
	 */
	//	  val /= (s1->cells_covered / s1->histogram_features);
	//	  val /= (s2->cells_covered / s2->histogram_features);

	/*
	 * Finally, the selectivity is the estimated number of
	 * rows to be returned divided by the maximum possible
	 * number of rows that can be returned.
	 */
	selectivity = val / ntuples_max;

	/* Guard against over-estimates and crazy numbers :) */
	if ( isnan(selectivity) || ! isfinite(selectivity) || selectivity < 0.0 )
	{
		selectivity = DEFAULT_ND_JOINSEL;
	}
	else if ( selectivity > 1.0 )
	{
		selectivity = 1.0;
	}

	return selectivity;
}


float8
estimate_join_selectivity_temporal_dimension(PlannerInfo *root, List *args, SpecialJoinInfo *sjinfo, Oid operator, CachedOp cacheOp)
{
	double	  selec;
	double	  selec_inner;
	VariableStatData vardata1;
	VariableStatData vardata2;
	double	  nd1;
	double	  nd2;
	bool		isdefault1;
	bool		isdefault2;
	Oid		 opfuncoid;
	AttStatsSlot sslot1;
	AttStatsSlot sslot2;
	Form_pg_statistic stats1 = NULL;
	Form_pg_statistic stats2 = NULL;
	bool		have_mcvs1 = false;
	bool		have_mcvs2 = false;
	bool		join_is_reversed;

	get_join_variables(root, args, sjinfo,
					   &vardata1, &vardata2, &join_is_reversed);

	nd1 = get_variable_numdistinct(&vardata1, &isdefault1);
	nd2 = get_variable_numdistinct(&vardata2, &isdefault2);

	opfuncoid = get_opcode(operator);

	memset(&sslot1, 0, sizeof(sslot1));
	memset(&sslot2, 0, sizeof(sslot2));

	if (HeapTupleIsValid(vardata1.statsTuple))
	{
		/* note we allow use of nullfrac regardless of security check */
		stats1 = (Form_pg_statistic) GETSTRUCT(vardata1.statsTuple);
		if (statistic_proc_security_check(&vardata1, opfuncoid))
		{
			have_mcvs1 = get_attstatsslot_internal(&sslot1, vardata1.statsTuple,
												   STATISTIC_KIND_MCV, InvalidOid,
												   ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS, TEMPORAL_STATISTICS);
		}
	}

	if (HeapTupleIsValid(vardata2.statsTuple))
	{
		/* note we allow use of nullfrac regardless of security check */
		stats2 = (Form_pg_statistic) GETSTRUCT(vardata2.statsTuple);
		if (statistic_proc_security_check(&vardata2, opfuncoid))
			have_mcvs2 = get_attstatsslot_internal(&sslot2, vardata2.statsTuple,
												   STATISTIC_KIND_MCV, InvalidOid,
												   ATTSTATSSLOT_VALUES | ATTSTATSSLOT_NUMBERS, TEMPORAL_STATISTICS);
	}

	/* We need to compute the inner-join selectivity in all cases */
	selec_inner = check_mcv(root, cacheOp,
							&vardata1, &vardata2,
							nd1, nd2,
							&sslot1, &sslot2,
							stats1, stats2,
							have_mcvs1, have_mcvs2);

	selec = selec_inner;

	free_attstatsslot(&sslot1);
	free_attstatsslot(&sslot2);

	ReleaseVariableStats(vardata1);
	ReleaseVariableStats(vardata2);
	return selec;
}

CachedOp
get_cacheOp(Oid operator)
{
	if (operator == oper_oid(OVERLAPS_OP, T_TGEOMPOINT, T_PERIOD) ||
		operator == oper_oid(OVERLAPS_OP, T_TGEOMPOINT, T_TIMESTAMPTZ))
		return OVERLAPS_OP;
	else if (operator == oper_oid(CONTAINS_OP, T_TGEOMPOINT, T_PERIOD))
		return CONTAINS_OP;
	else
		return OVERLAPS_OP;
}

/*
 * Look up the fraction of values less than (or equal, if 'equal' argument
 * is true) a given const in a histogram of period bounds.
 */
double
calc_period_hist_join_selectivity_scalar(PeriodBound *constbound,
										 PeriodBound *hist, int hist_nvalues1, int hist_nvalues2, bool equal)
{
	Selectivity selec;
	int			index;

	/*
	 * Find the histogram bin the given constant falls into. Estimate
	 * selectivity as the number of preceding whole bins.
	 */
	index = period_rbound_bsearch(constbound, hist, hist_nvalues1, equal);
	selec = (Selectivity) (Max(index, 0)) / (Selectivity) (hist_nvalues1 * hist_nvalues2  - 1);

	/* Adjust using linear interpolation within the bin */
	if (index >= 0 && index < hist_nvalues1 * hist_nvalues2 - 1)
		selec += 1 / (Selectivity) (hist_nvalues1 * hist_nvalues2 - 1);

	return selec;
}

double
timestamp_join_sel(AttStatsSlot hslot1, AttStatsSlot hslot2,
				   CachedOp cachedOp)
{
	int nhist1 = hslot1.nvalues;
	int nhist2 = hslot2.nvalues;
	int i;
	Selectivity hist_selec, selec = 0.0;
	PeriodBound *hist_lower, *hist_upper;
	hist_lower = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
	hist_upper = (PeriodBound *) palloc(sizeof(PeriodBound) * nhist1);
	for (i = 0; i < nhist1; i++)
	{
		period_deserialize(DatumGetPeriod(hslot1.values[i]),
						   &hist_lower[i], &hist_upper[i]);
	}
	for (i = 0; i < nhist2; i++)
	{
		PeriodBound const_lower, const_upper;
		period_deserialize(DatumGetPeriod(hslot2.values[i]),
						   &const_lower, &const_upper);

		switch (cachedOp)
		{
			case OVERLAPS_OP:
			{
				hist_selec =
						calc_period_hist_join_selectivity_scalar(&const_lower, hist_upper,
																 nhist1, nhist2, true);
				hist_selec +=
						(1.0 - calc_period_hist_join_selectivity_scalar(&const_upper, hist_lower,
																		nhist1, nhist2, true));
				hist_selec = 1.0 - hist_selec;
				selec += hist_selec;
				break;
			}
			case CONTAINS_OP:
			{
				hist_selec =
						calc_period_hist_selectivity_contains(&const_lower,
															  &const_upper, hist_lower, nhist1,
															  hslot1.values, hslot1.nvalues);
				if(hist_selec > 0.0f)
					selec += (1.0 / (nhist1 * nhist2 -1));
				selec += hist_selec;
				break;
			}
			default:
				break;
		}
	}
	return selec;
}

ND_STATS*
pg_get_nd_stats(const Oid table_oid, AttrNumber att_num, int mode, bool only_parent)
{
	HeapTuple stats_tuple = NULL;
	ND_STATS *nd_stats;

	/* First pull the stats tuple for the whole tree */
	if ( ! only_parent )
	{
		stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table_oid), Int16GetDatum(att_num), BoolGetDatum(true));
	}
	/* Fall-back to main table stats only, if not found for whole tree or explicitly ignored */
	if ( only_parent || ! stats_tuple )
	{
		stats_tuple = SearchSysCache3(STATRELATTINH, ObjectIdGetDatum(table_oid), Int16GetDatum(att_num), BoolGetDatum(false));
	}
	if ( ! stats_tuple )
	{
		return NULL;
	}

	nd_stats = pg_nd_stats_from_tuple(stats_tuple, mode);
	ReleaseSysCache(stats_tuple);

	return nd_stats;
}

ND_STATS*
pg_nd_stats_from_tuple(HeapTuple stats_tuple, int mode)
{
	int stats_kind = STATISTIC_KIND_ND;
	int rv;
	ND_STATS *nd_stats;

	/* If we're in 2D mode, set the kind appropriately */
	if ( mode == 2 ) stats_kind = STATISTIC_KIND_2D;

	/* Then read the geom status histogram from that */
	AttStatsSlot sslot;
	rv = get_attstatsslot(&sslot, stats_tuple, stats_kind, InvalidOid,
						  ATTSTATSSLOT_NUMBERS);
	if ( ! rv )
		return NULL;

	/* Clone the stats here so we can release the attstatsslot immediately */
	nd_stats = palloc(sizeof(float4) * sslot.nnumbers);
	memcpy(nd_stats, sslot.numbers, sizeof(float4) * sslot.nnumbers);

	free_attstatsslot(&sslot);

	return nd_stats;
}
