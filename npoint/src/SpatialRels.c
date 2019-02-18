/*****************************************************************************
 *
 * SpatialRels.c
 *	  Spatial relationships for temporal network-constrained points.
 *
 * These relationships project the temporal dimension and return a Boolean.
 * They are thus defined with the "at any instant" semantics, that is, the
 * traditional spatial function is applied to the union of all values taken
 * by the temporal npoint. The following relationships are supported:
 *	contains, containsproperly, covers, coveredby, crosses, disjoint,
 *	equals, intersects, overlaps, touches, within, dwithin, and
 *	relate (with 2 and 3 arguments)
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Generic binary functions for tnpoint <rel> geo
 *****************************************************************************/

bool
spatialrel_tnpointinst_geo(TemporalInst *inst, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum geom = tnpointinst_geom(inst);
	bool result = invert ? DatumGetBool(operator(geo, geom)) :
		DatumGetBool(operator(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel_tnpointi_geo(TemporalI *ti, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum geom = tnpointi_geom(ti);
	bool result = invert ? DatumGetBool(operator(geo, geom)) :
		DatumGetBool(operator(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel_tnpointseq_geo(TemporalSeq *seq, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum geom = tnpointseq_geom(seq);
	bool result = invert ? DatumGetBool(operator(geo, geom)) :
		DatumGetBool(operator(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel_tnpoints_geo(TemporalS *ts, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum geom = tnpoints_geom(ts);
	bool result = invert ? DatumGetBool(operator(geo, geom)) :
		DatumGetBool(operator(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

/*****************************************************************************
 * Generic binary functions for tnpoint <rel> tnpoint
 *****************************************************************************/

bool
spatialrel_tnpointinst_tnpointinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum))
{
	Datum geom1 =  tnpointinst_geom(inst1);
	Datum geom2 =  tnpointinst_geom(inst2);
	bool result = DatumGetBool(operator(geom1, geom2));

	pfree(DatumGetPointer(geom1));
	pfree(DatumGetPointer(geom2));
	return result;
}

bool
spatialrel_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum))
{
	TimestampSet *ts = temporali_time(ti2);
	TemporalI *projti1 = temporali_at_timestampset(ti1, ts);
	pfree(ts);
	if (projti1 == NULL)
		return false;

	ts = temporali_time(projti1);
	TemporalI *projti2 = temporali_at_timestampset(ti2, ts);
	Datum geom1 = tnpointi_geom(projti1);
	Datum geom2 = tnpointi_geom(projti2);
	bool result = DatumGetBool(operator(geom1, geom2));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projti1); pfree(ts); pfree(projti2);
	return result;
}

bool
spatialrel_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum))
{
	Period *p = intersection_period_period_internal(&seq1->period, &seq2->period);
	if (p == NULL)
		return false;

	TemporalSeq *projseq1 = temporalseq_at_period(seq1, p);
	TemporalSeq *projseq2 = temporalseq_at_period(seq2, p);
	Datum geom1 = tnpointseq_geom(projseq1);
	Datum geom2 = tnpointseq_geom(projseq2);
	bool result = DatumGetBool(operator(geom1, geom2));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projseq1); pfree(projseq2);
	pfree(p);
	return result;
}

bool
spatialrel_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum))
{
	PeriodSet *ps = temporals_intersection_temporals(ts1, ts2);
	if (ps == NULL)
		return false;

	TemporalS *projts1 = temporals_at_periodset(ts1, ps);
	TemporalS *projts2 = temporals_at_periodset(ts2, ps);
	Datum geom1 = tnpoints_trajectory(projts1);
	Datum geom2 = tnpoints_trajectory(projts2);
	bool result = DatumGetBool(operator(geom1, geom2));

	pfree(ps);
	pfree(projts1); pfree(projts2);
	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	return result;
}

/*****************************************************************************
 * Generic ternary functions for tnpoint <rel> geo
 *****************************************************************************/

bool
spatialrel3_tnpointinst_geo(TemporalInst *inst, Datum geo, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum geom = tnpointinst_geom(inst);
	bool result = invert ? DatumGetBool(operator(geo, geom, param)) :
		DatumGetBool(operator(geom, geo, param));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel3_tnpointi_geo(TemporalI *ti, Datum geo, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum geom = tnpointi_geom(ti);
	bool result = invert ? DatumGetBool(operator(geo, geom, param)) :
		DatumGetBool(operator(geom, geo, param));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel3_tnpointseq_geo(TemporalSeq *seq, Datum geo, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum geom = tnpointseq_geom(seq);
	bool result = invert ? DatumGetBool(operator(geo, geom, param)) :
		DatumGetBool(operator(geom, geo, param));
	pfree(DatumGetPointer(geom));
	return result;
}

bool
spatialrel3_tnpoints_geo(TemporalS *ts, Datum geo, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum geom = tnpoints_geom(ts);
	bool result = invert ? DatumGetBool(operator(geo, geom, param)) :
		DatumGetBool(operator(geom, geo, param));
	pfree(DatumGetPointer(geom));
	return result;
}

/*****************************************************************************
 * Generic ternary functions for tnpoint <rel> tnpoint
 *****************************************************************************/

bool
spatialrel3_tnpointinst_tnpointinst(TemporalInst *inst1, TemporalInst *inst2, Datum param, 
	Datum (*operator)(Datum, Datum, Datum))
{
	Datum geom1 =  tnpointinst_geom(inst1);
	Datum geom2 =  tnpointinst_geom(inst2);
	bool result = DatumGetBool(operator(geom1, geom2, param));

	pfree(DatumGetPointer(geom1));
	pfree(DatumGetPointer(geom2));
	return result;
}

bool
spatialrel3_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2, Datum param, 
	Datum (*operator)(Datum, Datum, Datum))
{
	TimestampSet *ts = temporali_time(ti2);
	TemporalI *projti1 = temporali_at_timestampset(ti1, ts);
	pfree(ts);
	if (projti1 == NULL)
		return false;

	ts = temporali_time(projti1);
	TemporalI *projti2 = temporali_at_timestampset(ti2, ts);
	Datum geom1 = tnpointi_geom(projti1);
	Datum geom2 = tnpointi_geom(projti2);
	bool result = DatumGetBool(operator(geom1, geom2, param));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projti1); pfree(ts); pfree(projti2);
	return result;
}

bool
spatialrel3_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param, 
	Datum (*operator)(Datum, Datum, Datum))
{
	Period *p = intersection_period_period_internal(&seq1->period, &seq2->period);
	if (p == NULL)
		return false;

	TemporalSeq *projseq1 = temporalseq_at_period(seq1, p);
	TemporalSeq *projseq2 = temporalseq_at_period(seq2, p);
	Datum geom1 = tnpointseq_geom(projseq1);
	Datum geom2 = tnpointseq_geom(projseq2);
	bool result = DatumGetBool(operator(geom1, geom2, param));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projseq1); pfree(projseq2);
	pfree(p);
	return result;
}


bool
spatialrel3_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2, Datum param, 
	Datum (*operator)(Datum, Datum, Datum))
{
	PeriodSet *ps = temporals_intersection_temporals(ts1, ts2);
	if (ps == NULL)
		return false;

	TemporalS *projts1 = temporals_at_periodset(ts1, ps);
	TemporalS *projts2 = temporals_at_periodset(ts2, ps);
	Datum geom1 = tnpoints_trajectory(projts1);
	Datum geom2 = tnpoints_trajectory(projts2);
	bool result = DatumGetBool(operator(geom1, geom2, param));

	pfree(ps);
	pfree(projts1); pfree(projts2);
	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	return result;
}

/*****************************************************************************
 * Generic relate functions for tnpoint relate geo
 *****************************************************************************/

text *
relate1_tnpointinst_geo(TemporalInst *inst, Datum geo, bool invert)
{
	Datum geom = tnpointinst_geom(inst);
	text *result = invert ? DatumGetTextP(geom_relate(geo, geom)) :
		DatumGetTextP(geom_relate(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

text *
relate1_tnpointi_geo(TemporalI *ti, Datum geo, bool invert)
{
	Datum geom = tnpointi_geom(ti);
	text *result = invert ? DatumGetTextP(geom_relate(geo, geom)) :
		DatumGetTextP(geom_relate(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

text *
relate1_tnpointseq_geo(TemporalSeq *seq, Datum geo, bool invert)
{
	Datum geom = tnpointseq_geom(seq);
	text *result = invert ? DatumGetTextP(geom_relate(geo, geom)) :
		DatumGetTextP(geom_relate(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

text *
relate1_tnpoints_geo(TemporalS *ts, Datum geo, bool invert)
{
	Datum geom = tnpoints_geom(ts);
	text *result = invert ? DatumGetTextP(geom_relate(geo, geom)) :
		DatumGetTextP(geom_relate(geom, geo));
	pfree(DatumGetPointer(geom));
	return result;
}

/*****************************************************************************
 * Generic relate functions for tnpoint relate tnpoint 
 *****************************************************************************/

text *
relate1_tnpointinst_tnpointinst(TemporalInst *inst1, TemporalInst *inst2)
{
	Datum geom1 =  tnpointinst_geom(inst1);
	Datum geom2 =  tnpointinst_geom(inst2);
	text *result = DatumGetTextP(geom_relate(geom1, geom2));

	pfree(DatumGetPointer(geom1));
	pfree(DatumGetPointer(geom2));
	return result;
}

text *
relate1_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2)
{
	TimestampSet *ts = temporali_time(ti2);
	TemporalI *projti1 = temporali_at_timestampset(ti1, ts);
	pfree(ts);
	if (projti1 == NULL)
		return NULL;

	ts = temporali_time(projti1);
	TemporalI *projti2 = temporali_at_timestampset(ti2, ts);
	Datum geom1 = tnpointi_geom(projti1);
	Datum geom2 = tnpointi_geom(projti2);
	text *result = DatumGetTextP(geom_relate(geom1, geom2));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projti1); pfree(ts); pfree(projti2);
	return result;
}

text *
relate1_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2)
{
	Period *p = intersection_period_period_internal(&seq1->period, &seq2->period);
	if (p == NULL)
		return NULL;

	TemporalSeq *projseq1 = temporalseq_at_period(seq1, p);
	TemporalSeq *projseq2 = temporalseq_at_period(seq2, p);
	Datum geom1 = tnpointseq_geom(projseq1);
	Datum geom2 = tnpointseq_geom(projseq2);
	text *result = DatumGetTextP(geom_relate(geom1, geom2));

	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	pfree(projseq1); pfree(projseq2);
	pfree(p);
	return result;
}

text *
relate1_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2)
{
	PeriodSet *ps = temporals_intersection_temporals(ts1, ts2);
	if (ps == NULL)
		return NULL;

	TemporalS *projts1 = temporals_at_periodset(ts1, ps);
	TemporalS *projts2 = temporals_at_periodset(ts2, ps);
	Datum geom1 = tnpoints_trajectory(projts1);
	Datum geom2 = tnpoints_trajectory(projts2);
	text *result = DatumGetTextP(geom_relate(geom1, geom2));

	pfree(ps);
	pfree(projts1); pfree(projts2);
	pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
	return result;
}

/*****************************************************************************
 * Dispatch Functions
 *****************************************************************************/

bool
spatialrel_tnpoint_geo(Temporal *temp, Datum geom,
	Datum (*operator)(Datum, Datum), bool invert)
{
	bool result = false;
	if (temp->type == TEMPORALINST)
		result = spatialrel_tnpointinst_geo((TemporalInst *)temp, geom,
			operator, invert);
	else if (temp->type == TEMPORALI)
		result = spatialrel_tnpointi_geo((TemporalI *)temp, geom,
			operator, invert);
	else if (temp->type == TEMPORALSEQ)
		result = spatialrel_tnpointseq_geo((TemporalSeq *)temp, geom,
			operator, invert);
	else if (temp->type == TEMPORALS)
		result = spatialrel_tnpoints_geo((TemporalS *)temp, geom,
			operator, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

bool
spatialrel_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum))
{
	bool result = false;
	if (temp1->type == TEMPORALINST) 
		result = spatialrel_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator);
	else if (temp1->type == TEMPORALI) 
		result = spatialrel_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2, operator);
	else if (temp1->type == TEMPORALSEQ)
		result = spatialrel_tnpointseq_tnpointseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator);
	else if (temp1->type == TEMPORALS)
		result = spatialrel_tnpoints_tnpoints(
			(TemporalS *)temp1, (TemporalS *)temp2, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************/

bool
relate1_tnpoint_geo(Temporal *temp, Datum geo, bool invert)
{
	text *result = NULL;
	if (temp->type == TEMPORALINST)
		result = relate1_tnpointinst_geo((TemporalInst *)temp, geo, invert);
	else if (temp->type == TEMPORALI)
		result = relate1_tnpointi_geo((TemporalI *)temp, geo, invert);
	else if (temp->type == TEMPORALSEQ)
		result = relate1_tnpointseq_geo((TemporalSeq *)temp, geo, invert);
	else if (temp->type == TEMPORALS)
		result = relate1_tnpoints_geo((TemporalS *)temp, geo, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************/

bool
spatialrel3_tnpoint_geo(Temporal *temp, Datum geom, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	bool result = false;
	if (temp->type == TEMPORALINST)
		result = spatialrel3_tnpointinst_geo((TemporalInst *)temp, geom, param,
			operator, invert);
	else if (temp->type == TEMPORALI)
		result = spatialrel3_tnpointi_geo((TemporalI *)temp, geom, param,
			operator, invert);
	else if (temp->type == TEMPORALSEQ)
		result = spatialrel3_tnpointseq_geo((TemporalSeq *)temp, geom, param,
			operator, invert);
	else if (temp->type == TEMPORALS)
		result = spatialrel3_tnpoints_geo((TemporalS *)temp, geom, param,
			operator, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

bool
spatialrel3_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	bool result = false;
	if (temp1->type == TEMPORALINST) 
		result = spatialrel3_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, operator);
	else if (temp1->type == TEMPORALI) 
		result = spatialrel3_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2, param, operator);
	else if (temp1->type == TEMPORALSEQ) 
		result = spatialrel3_tnpointseq_tnpointseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, operator);
	else if (temp1->type == TEMPORALS) 
		result = spatialrel3_tnpoints_tnpoints(
			(TemporalS *)temp1, (TemporalS *)temp2, param, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_geo_tnpoint);

PGDLLEXPORT Datum
contains_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_contains, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_tnpoint_geo);

PGDLLEXPORT Datum
contains_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_contains, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_tnpoint_tnpoint);

PGDLLEXPORT Datum
contains_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_contains);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal containsproperly
 *****************************************************************************/

PG_FUNCTION_INFO_V1(containsproperly_geo_tnpoint);

PGDLLEXPORT Datum
containsproperly_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_containsproperly, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(containsproperly_tnpoint_geo);

PGDLLEXPORT Datum
containsproperly_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_containsproperly, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(containsproperly_tnpoint_tnpoint);

PGDLLEXPORT Datum
containsproperly_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_containsproperly);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(covers_geo_tnpoint);

PGDLLEXPORT Datum
covers_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_covers, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(covers_tnpoint_geo);

PGDLLEXPORT Datum
covers_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_covers, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(covers_tnpoint_tnpoint);

PGDLLEXPORT Datum
covers_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_covers);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal coveredby
 *****************************************************************************/

PG_FUNCTION_INFO_V1(coveredby_geo_tnpoint);

PGDLLEXPORT Datum
coveredby_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_coveredby, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(coveredby_tnpoint_geo);

PGDLLEXPORT Datum
coveredby_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_coveredby, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(coveredby_tnpoint_tnpoint);

PGDLLEXPORT Datum
coveredby_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_coveredby);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal crosses
 *****************************************************************************/

PG_FUNCTION_INFO_V1(crosses_geo_tnpoint);

PGDLLEXPORT Datum
crosses_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_crosses, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(crosses_tnpoint_geo);

PGDLLEXPORT Datum
crosses_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_crosses, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(crosses_tnpoint_tnpoint);

PGDLLEXPORT Datum
crosses_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_crosses);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(disjoint_geo_tnpoint);

PGDLLEXPORT Datum
disjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_disjoint, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(disjoint_tnpoint_geo);

PGDLLEXPORT Datum
disjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_disjoint, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(disjoint_tnpoint_tnpoint);

PGDLLEXPORT Datum
disjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_disjoint);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(equals_geo_tnpoint);

PGDLLEXPORT Datum
equals_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_equals, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(equals_tnpoint_geo);

PGDLLEXPORT Datum
equals_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_equals, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(equals_tnpoint_tnpoint);

PGDLLEXPORT Datum
equals_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_equals);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intersects_geo_tnpoint);

PGDLLEXPORT Datum
intersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_intersects2d, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(intersects_tnpoint_geo);

PGDLLEXPORT Datum
intersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_intersects2d, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(intersects_tnpoint_tnpoint);

PGDLLEXPORT Datum
intersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_intersects2d);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_geo_tnpoint);

PGDLLEXPORT Datum
overlaps_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_overlaps, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_tnpoint_geo);

PGDLLEXPORT Datum
overlaps_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_overlaps, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_tnpoint_tnpoint);

PGDLLEXPORT Datum
overlaps_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_overlaps);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(touches_geo_tnpoint);

PGDLLEXPORT Datum
touches_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_touches, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(touches_tnpoint_geo);

PGDLLEXPORT Datum
touches_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_touches, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(touches_tnpoint_tnpoint);

PGDLLEXPORT Datum
touches_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_touches);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(within_geo_tnpoint);

PGDLLEXPORT Datum
within_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_within, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(within_tnpoint_geo);

PGDLLEXPORT Datum
within_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = spatialrel_tnpoint_geo(temp, geom, &geom_within, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(within_tnpoint_tnpoint);

PGDLLEXPORT Datum
within_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tnpoint_tnpoint(sync1, sync2, &geom_within);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(dwithin_geo_tnpoint);

PGDLLEXPORT Datum
dwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	bool result = spatialrel3_tnpoint_geo(temp, geom, dist, &geom_dwithin2d, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(dwithin_tnpoint_geo);

PGDLLEXPORT Datum
dwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	Datum dist = PG_GETARG_DATUM(2);
	bool result = spatialrel3_tnpoint_geo(temp, geom, dist, &geom_dwithin2d, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(dwithin_tnpoint_tnpoint);

PGDLLEXPORT Datum
dwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel3_tnpoint_tnpoint(sync1, sync2, dist, &geom_dwithin2d);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal relate
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_geo_tnpoint);

PGDLLEXPORT Datum
relate_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	bool result = relate1_tnpoint_geo(temp, geom, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(relate_tnpoint_geo);

PGDLLEXPORT Datum
relate_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	bool result = relate1_tnpoint_geo(temp, geom, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(relate_tnpoint_tnpoint);

PGDLLEXPORT Datum
relate_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	
	text *result = NULL;
	if (temp1->type == TEMPORALINST)
		result = relate1_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2);
	else if (temp1->type == TEMPORALI)
		result = relate1_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2);
	else if (temp1->type == TEMPORALSEQ)
		result = relate1_tnpointseq_tnpointseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2);
	else if (temp1->type == TEMPORALS)
		result = relate1_tnpoints_tnpoints(
			(TemporalS *)temp1, (TemporalS *)temp2);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Temporal relate_pattern
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_pattern_geo_tnpoint);

PGDLLEXPORT Datum
relate_pattern_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geom = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	bool result = spatialrel3_tnpoint_geo(temp, geom, pattern,
		&geom_relate_pattern, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tnpoint_geo);

PGDLLEXPORT Datum
relate_pattern_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	Datum pattern = PG_GETARG_DATUM(2);
	bool result = spatialrel3_tnpoint_geo(temp, geom, pattern,
		&geom_relate_pattern, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tnpoint_tnpoint);

PGDLLEXPORT Datum
relate_pattern_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel3_tnpoint_tnpoint(sync1, sync2, pattern, &geom_relate_pattern);
 	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
