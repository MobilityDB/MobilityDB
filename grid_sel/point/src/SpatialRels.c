/*****************************************************************************
 *
 * SpatialRels.c
 *	  Spatial relationships for temporal points.
 *
 * These relationships project the temporal dimension and return a Boolean.
 * They are thus defined with the "at any instant" semantics, that is, the
 * traditional spatial function is applied to the union of all values taken 
 * by the temporal point. The following relationships are supported for 
 * geometries:
 *		contains, containsproperly, covers, coveredby, crosses, disjoint, 
 *		equals, intersects, overlaps, touches, within, dwithin, and
 *		relate (with 2 and 3 arguments)
 * The following relationships are supported for geographies
 *     covers, coveredby, intersects, dwithin
 * Only dwithin and intersects support 3D geometries.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"
	
/*****************************************************************************
 * Spatial relationship functions
 *****************************************************************************/

/* 
 * contains and within are inverse to each other
 * covers and coveredby are inverse to each other
 * containsproperly and containedproperlyby are inverse to each other
 */

Datum
geom_contains(Datum geom1, Datum geom2)
{
	return call_function2(contains, geom1, geom2);
}

Datum
geom_containsproperly(Datum geom1, Datum geom2)
{
	return call_function2(containsproperly, geom1, geom2);
}

Datum
geom_containedproperlyby(Datum geom1, Datum geom2)
{
	return call_function2(containsproperly, geom2, geom1);
}

Datum
geom_covers(Datum geom1, Datum geom2)
{
	return call_function2(covers, geom1, geom2);
}

Datum
geom_coveredby(Datum geom1, Datum geom2)
{
	return call_function2(coveredby, geom1, geom2);
}

Datum
geom_crosses(Datum geom1, Datum geom2)
{
	return call_function2(crosses, geom1, geom2);
}

Datum
geom_crossedby(Datum geom1, Datum geom2)
{
	return call_function2(crosses, geom2, geom1);
}

/* ST_Relate(A,B) = 'FF*FF****' */
Datum
geom_disjoint(Datum geom1, Datum geom2)
{
	return call_function2(disjoint, geom1, geom2);
}

Datum
geom_equals(Datum geom1, Datum geom2)
{
	return call_function2(ST_Equals, geom1, geom2);
}

/* ST_Intersects(g1, g2 ) --> Not (ST_Disjoint(g1, g2 )) */
Datum
geom_intersects2d(Datum geom1, Datum geom2)
{
	return call_function2(intersects, geom1, geom2);
}

Datum
geom_intersects3d(Datum geom1, Datum geom2)
{
	return call_function2(intersects3d, geom1, geom2);
}

Datum
geom_overlaps(Datum geom1, Datum geom2)
{
	return call_function2(overlaps, geom1, geom2);
}

/* ST_Relate(A,B) = 'FT*******' or 'F**T*****' or 'F***T****' */
Datum
geom_touches(Datum geom1, Datum geom2)
{
	return call_function2(touches, geom1, geom2);
}

/* ST_Relate(A,B) = 'T*F**F***' */
Datum
geom_within(Datum geom1, Datum geom2)
{
	return call_function2(contains, geom2, geom1);
}

Datum
geom_dwithin2d(Datum geom1, Datum geom2, Datum dist)
{
	return call_function3(LWGEOM_dwithin, geom1, geom2, dist);
}

Datum
geom_dwithin3d(Datum geom1, Datum geom2, Datum dist)
{
	return call_function3(LWGEOM_dwithin3d, geom1, geom2, dist);
}

Datum
geom_relate(Datum geom1, Datum geom2)
{
	return call_function2(relate_full, geom1, geom2); 
}

Datum
geom_relate_pattern(Datum geom1, Datum geom2, Datum pattern)
{
	return call_function3(relate_pattern, geom1, geom2, pattern); 
}

/*****************************************************************************/
 
Datum
geog_covers(Datum geog1, Datum geog2)
{
	return call_function2(geography_covers, geog1, geog2);
}

Datum
geog_coveredby(Datum geog1, Datum geog2)
{
	return call_function2(geography_covers, geog2, geog1);
}

Datum
geog_disjoint(Datum geog1, Datum geog2)
{
	double dist = DatumGetFloat8(call_function4(geography_distance, 
		geog1, geog2, 0.0, false));
	return BoolGetDatum(dist >= 0.00001);
}

Datum
geog_intersects(Datum geog1, Datum geog2)
{
	double dist = DatumGetFloat8(call_function4(geography_distance, 
		geog1, geog2, 0.0, false));
	return BoolGetDatum(dist < 0.00001);
}

Datum
geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
	return call_function4(geography_dwithin, geog1, geog2, dist, true);
}
 
/*****************************************************************************
 * Generic binary functions
 * The operators that have two temporal points as argument suppose that they
 * overlap on the time dimension. This is ensured in the external function 
 * in order to return NULL if it is not the case.
 *****************************************************************************/

/* Temporal spatialrel geo */

static bool
spatialrel_tpointinst_geo(TemporalInst *inst, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum value = temporalinst_value(inst);
	bool result = invert ? DatumGetBool(operator(geo, value)) :
		DatumGetBool(operator(value, geo));
	return result;
}

static bool
spatialrel_tpointi_geo(TemporalI *ti, Datum geo,
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum values = tpointi_values(ti);
	bool result =  invert ? DatumGetBool(operator(geo, values)) :
		DatumGetBool(operator(values, geo));
	pfree(DatumGetPointer(values)); 
	return result;
}

static bool
spatialrel_tpointseq_geo(TemporalSeq *seq, Datum geo, 
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	bool result = invert ? DatumGetBool(operator(geo, traj)) :
		DatumGetBool(operator(traj, geo));
	return result;
}

static bool
spatialrel_tpoints_geo(TemporalS *ts, Datum geo,
	Datum (*operator)(Datum, Datum), bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	bool result = invert ? DatumGetBool(operator(geo, traj)) :
		DatumGetBool(operator(traj, geo));
	pfree(DatumGetPointer(traj));
	return result;
}

/*****************************************************************************/

/* Temporal spatialrel Temporal */

static bool
spatialrel_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum))
{
	return DatumGetBool(operator(temporalinst_value(inst1), 
		temporalinst_value(inst2)));
}

static bool
spatialrel_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*operator)(Datum, Datum))
{
	Datum geo1 = tpointi_values(ti1);
	Datum geo2 = tpointi_values(ti2);
	bool result = DatumGetBool(operator(geo1, geo2));
	pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2));
	return result;
}

static bool
spatialrel_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum))
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	bool result = DatumGetBool(operator(traj1, traj2));
	return result;
}

static bool
spatialrel_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2,
	Datum (*operator)(Datum, Datum))
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	bool result = DatumGetBool(operator(traj1, traj2));
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2)); 
	return result;
}

/*****************************************************************************
 * Generic ternary functions
 *****************************************************************************/

/* Temporal spatialrel Geo */

static bool
spatialrel3_tpointinst_geo(TemporalInst *inst, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum value = temporalinst_value(inst);
	Datum result = invert ? DatumGetBool(operator(geo, value, param)) :
		DatumGetBool(operator(value, geo, param));
	return result;
}

static bool
spatialrel3_tpointi_geo(TemporalI *ti, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum values = tpointi_values(ti);
	bool result = invert ? DatumGetBool(operator(geo, values, param)) :
		DatumGetBool(operator(values, geo, param));
	return result;
}

static bool
spatialrel3_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	bool result = invert ? DatumGetBool(operator(geo, traj, param)) :
		DatumGetBool(operator(traj, geo, param));
	return result;
}

static bool
spatialrel3_tpoints_geo(TemporalS *ts, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	bool result = invert ? DatumGetBool(operator(geo, traj, param)) :
		DatumGetBool(operator(traj, geo, param));
	pfree(DatumGetPointer(traj));
	return result;
}

/*****************************************************************************/

/* Temporal spatialrel Temporal */

static bool
spatialrel3_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	return DatumGetBool(operator(temporalinst_value(inst1), 
		temporalinst_value(inst2), param));
}

static bool
spatialrel3_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Datum geo1 = tpointi_values(ti1);
	Datum geo2 = tpointi_values(ti2);
	bool result = DatumGetBool(operator(geo1, geo2, param));
	pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2)); 
	return result;
}

static bool
spatialrel3_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	bool result = DatumGetBool(operator(traj1, traj2, param));
	return result;
}

static bool
spatialrel3_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	bool result = DatumGetBool(operator(traj1, traj2, param));
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2));
	return result;
}

/*****************************************************************************
 * Generic dwithin functions when both temporal points are moving
 * The functions suppose that the temporal points are synchronized
 * TODO ARE THESE FUNCTIONS CORRECT ????
 *****************************************************************************/

static bool
dwithin_tpointseq_tpointseq1(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Datum sv1 = temporalinst_value(start1);
	Datum ev1 = temporalinst_value(end1);
	Datum sv2 = temporalinst_value(start2);
	Datum ev2 = temporalinst_value(end2);
	/* Both instants are constant */
	if (datum_eq(sv1, ev1, start1->valuetypid) &&
		datum_eq(sv2, ev2, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		return operator(sv1, sv2, param);
	}
	
	TimestampTz lower = start1->t;
	TimestampTz upper = start2->t;
	/* Determine whether there is a local minimum between lower and upper */
	TimestampTz crosstime;
	bool cross = temporalseq_intersect_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	/* If there is no local minimum or if there is one at a bound */	
	if (!cross || crosstime == lower || crosstime == upper)
	{
		/* Compute the operator at the start and end instants */
		return operator(sv1, sv2, param);
	}

	/* Find the values at the local minimum */
	Datum crossvalue1 = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum crossvalue2 = temporalseq_value_at_timestamp1(start2, end2, crosstime);
	/* Compute operator at the start instant and at the local minimum */
	bool result = operator(sv1, sv2, param) || 
		operator(crossvalue1, crossvalue2, param);
	
	pfree(DatumGetPointer(crossvalue1));
	pfree(DatumGetPointer(crossvalue2));
		
	return result;
}

static bool
dwithin_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum d,
	Datum (*operator)(Datum, Datum, Datum))
{
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool result;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		result = dwithin_tpointseq_tpointseq1(start1, end1, 
			start2, end2, d, operator);
		if (result == true)
			return true;
		start1 = end1;
		start2 = end2;
	}
	result = spatialrel3_tpointinst_tpointinst(start1, start2, d, operator);
    return result;
}

static bool
dwithin_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum d,
	Datum (*operator)(Datum, Datum, Datum))
{
	bool result = false;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		if (dwithin_tpointseq_tpointseq(seq1, seq2, d, operator))
		{
			result = true;
			break;
		}
	}
    return result;
}

/*****************************************************************************
 * Generic relate functions
 *****************************************************************************/

/* TemporalInst relate <Type> */

static text *
relate2_tpointinst_geo(TemporalInst *inst, Datum geo, bool invert)
{
	Datum value = temporalinst_value(inst);
	text *result = invert ? DatumGetTextP(geom_relate(geo, value)) :
		DatumGetTextP(geom_relate(value, geo));
	return result;
}

static text *
relate2_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2)
{
	if (inst1->t != inst2->t)
		return NULL;
	
	return DatumGetTextP(geom_relate(temporalinst_value(inst1), 
		temporalinst_value(inst2)));
}

/*****************************************************************************/

/* TemporalI relate <Type> */

static text *
relate2_tpointi_geo(TemporalI *ti, Datum geo, bool invert)
{
	Datum geo1 = tpointi_values(ti);
	text *result = invert ? DatumGetTextP(geom_relate(geo, geo1)) :
		DatumGetTextP(geom_relate(geo1, geo));
	pfree(DatumGetPointer(geo1));
	return DatumGetTextP(result);
}

static text *
relate2_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2)
{
	Datum geo1 = tpointi_values(ti1);
	Datum geo2 = tpointi_values(ti2);
	text *result = DatumGetTextP(geom_relate(geo1, geo2));
	pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2)); 
	return result;
}

/*****************************************************************************/

/* TemporalSeq relate <Type> */

static text *
relate2_tpointseq_geo(TemporalSeq *seq, Datum geo, bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	text *result = invert ? DatumGetTextP(geom_relate(geo, traj)) :
		DatumGetTextP(geom_relate(traj, geo));
	return result;
}

static text *
relate2_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2)
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	text *result = DatumGetTextP(geom_relate(traj1, traj2));
	return result;
}

/*****************************************************************************/

/* TemporalS relate <Type> */

static text *
relate2_tpoints_geo(TemporalS *ts, Datum geo, bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	text *result = invert ? DatumGetTextP(geom_relate(geo, traj)) :
		DatumGetTextP(geom_relate(traj, geo));
	return result;
}

static text *
relate2_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2)
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	text *result = DatumGetTextP(geom_relate(traj1, traj2));
	return result;
}

/*****************************************************************************
 * Dispatch functions
 * It is supposed that the temporal values have been intersected before and
 * therefore they are both of the same duration.
 *****************************************************************************/

static bool
spatialrel_tpoint_geo(Temporal *temp, Datum geo,
	Datum (*operator)(Datum, Datum), bool invert)
{
	bool result = false;
	if (temp->type == TEMPORALINST) 
		result = spatialrel_tpointinst_geo((TemporalInst *)temp,
			geo, operator, invert);
	else if (temp->type == TEMPORALI) 
		result = spatialrel_tpointi_geo((TemporalI *)temp,
			geo, operator, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = spatialrel_tpointseq_geo((TemporalSeq *)temp,
			geo, operator, invert);
	else if (temp->type == TEMPORALS) 
		result = spatialrel_tpoints_geo((TemporalS *)temp,
			geo, operator, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}
 
static bool
spatialrel3_tpoint_geo(Temporal *temp, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	bool result = false;
	if (temp->type == TEMPORALINST) 
		result = spatialrel3_tpointinst_geo((TemporalInst *)temp,
			geo, param, operator, invert);
	else if (temp->type == TEMPORALI) 
		result = spatialrel3_tpointi_geo((TemporalI *)temp,
			geo, param, operator, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = spatialrel3_tpointseq_geo((TemporalSeq *)temp,
			geo, param, operator, invert);
	else if (temp->type == TEMPORALS) 
		result = spatialrel3_tpoints_geo((TemporalS *)temp,
			geo, param, operator, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}
 
static bool
spatialrel_tpoint_tpoint(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum))
{
	bool result = false;
	if (temp1->type == TEMPORALINST) 
		result = spatialrel_tpointinst_tpointinst((TemporalInst *)temp1,
			(TemporalInst *)temp2, operator);
	else if (temp1->type == TEMPORALI) 
		result = spatialrel_tpointi_tpointi((TemporalI *)temp1,
			(TemporalI *)temp2, operator);
	else if (temp1->type == TEMPORALSEQ) 
		result = spatialrel_tpointseq_tpointseq((TemporalSeq *)temp1,
			(TemporalSeq *)temp2, operator);
	else if (temp1->type == TEMPORALS) 
		result = spatialrel_tpoints_tpoints((TemporalS *)temp1,
			(TemporalS *)temp2, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

static bool
spatialrel3_tpoint_tpoint(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	bool result = false;
	if (temp1->type == TEMPORALINST) 
		result = spatialrel3_tpointinst_tpointinst((TemporalInst *)temp1,
			(TemporalInst *)temp2, param, operator);
	else if (temp1->type == TEMPORALI) 
		result = spatialrel3_tpointi_tpointi((TemporalI *)temp1,
			(TemporalI *)temp2, param, operator);
	else if (temp1->type == TEMPORALSEQ) 
		result = spatialrel3_tpointseq_tpointseq((TemporalSeq *)temp1,
			(TemporalSeq *)temp2, param, operator);
	else if (temp1->type == TEMPORALS) 
		result = spatialrel3_tpoints_tpoints((TemporalS *)temp1,
			(TemporalS *)temp2, param, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

static text *
relate2_tpoint_geo(Temporal *temp, Datum geo, bool invert)
{
	text *result = NULL;
	if (temp->type == TEMPORALINST)
		result = relate2_tpointinst_geo((TemporalInst *)temp, geo, invert);
	else if (temp->type == TEMPORALI)
		result = relate2_tpointi_geo((TemporalI *)temp, geo, invert);
	else if (temp->type == TEMPORALSEQ)
		result = relate2_tpointseq_geo((TemporalSeq *)temp, geo, invert);
	else if (temp->type == TEMPORALS)
		result = relate2_tpoints_geo((TemporalS *)temp, geo, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_geo_tpoint);

PGDLLEXPORT Datum
contains_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_contains, true);
			
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(contains_tpoint_geo);

PGDLLEXPORT Datum
contains_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_contains, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_tpoint_tpoint);

PGDLLEXPORT Datum
contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_contains);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal containsproperly
 *****************************************************************************/

PG_FUNCTION_INFO_V1(containsproperly_geo_tpoint);

PGDLLEXPORT Datum
containsproperly_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_containsproperly, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(containsproperly_tpoint_geo);

PGDLLEXPORT Datum
containsproperly_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_containsproperly, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(containsproperly_tpoint_tpoint);

PGDLLEXPORT Datum
containsproperly_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_containsproperly);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal covers (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(covers_geo_tpoint);

PGDLLEXPORT Datum
covers_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_covers;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(covers_tpoint_geo);

PGDLLEXPORT Datum
covers_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_covers;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(covers_tpoint_tpoint);

PGDLLEXPORT Datum
covers_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_covers;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_covers;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_tpoint(inter1, inter2, operator);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal coveredby (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(coveredby_geo_tpoint);

PGDLLEXPORT Datum
coveredby_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_coveredby;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, false);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(coveredby_tpoint_geo);

PGDLLEXPORT Datum
coveredby_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_coveredby;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(coveredby_tpoint_tpoint);

PGDLLEXPORT Datum
coveredby_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_coveredby;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_coveredby;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_tpoint(inter1, inter2, operator);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal crosses
 *****************************************************************************/

PG_FUNCTION_INFO_V1(crosses_geo_tpoint);

PGDLLEXPORT Datum
crosses_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_crosses, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(crosses_tpoint_geo);

PGDLLEXPORT Datum
crosses_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_crosses, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(crosses_tpoint_tpoint);

PGDLLEXPORT Datum
crosses_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_crosses);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(disjoint_geo_tpoint);

PGDLLEXPORT Datum
disjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(disjoint_tpoint_geo);

PGDLLEXPORT Datum
disjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(disjoint_tpoint_tpoint);

PGDLLEXPORT Datum
disjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_disjoint);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(equals_geo_tpoint);

PGDLLEXPORT Datum
equals_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_equals, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(equals_tpoint_geo);

PGDLLEXPORT Datum
equals_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_equals, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(equals_tpoint_tpoint);

PGDLLEXPORT Datum
equals_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_equals);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal intersects (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intersects_geo_tpoint);

PGDLLEXPORT Datum
intersects_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			operator = &geom_intersects3d;
		else
			operator = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_intersects;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(intersects_tpoint_geo);

PGDLLEXPORT Datum
intersects_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			operator = &geom_intersects3d;
		else
			operator = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_intersects;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		operator, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(intersects_tpoint_tpoint);

PGDLLEXPORT Datum
intersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	Datum (*operator)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		operator = &geom_intersects2d;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_intersects;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel_tpoint_tpoint(inter1, inter2, operator);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_geo_tpoint);

PGDLLEXPORT Datum
overlaps_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_overlaps, true);
			
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(overlaps_tpoint_geo);

PGDLLEXPORT Datum
overlaps_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_overlaps, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_tpoint_tpoint);

PGDLLEXPORT Datum
overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_overlaps);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(touches_geo_tpoint);

PGDLLEXPORT Datum
touches_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_touches, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(touches_tpoint_geo);

PGDLLEXPORT Datum
touches_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_touches, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(touches_tpoint_tpoint);

PGDLLEXPORT Datum
touches_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_touches);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(within_geo_tpoint);

PGDLLEXPORT Datum
within_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_within, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(within_tpoint_geo);

PGDLLEXPORT Datum
within_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_within, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(within_tpoint_tpoint);

PGDLLEXPORT Datum
within_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_within);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
 
/*****************************************************************************
 * Temporal dwithin (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(dwithin_geo_tpoint);

PGDLLEXPORT Datum
dwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			operator = &geom_dwithin3d;
		else
			operator = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_dwithin;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		operator, true);

	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}
 
PG_FUNCTION_INFO_V1(dwithin_tpoint_geo);

PGDLLEXPORT Datum
dwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	Datum dist = PG_GETARG_DATUM(2);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	Datum (*operator)(Datum, Datum, Datum);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			operator = &geom_dwithin3d;
		else
			operator = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_dwithin;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		operator, false);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(dwithin_tpoint_tpoint);

PGDLLEXPORT Datum
dwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *sync1, *sync2;
	/* Returns false if the temporal points do not intersect in time 
	 * The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Datum (*operator)(Datum, Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			operator = &geom_dwithin3d;
		else
			operator = &geom_dwithin2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		operator = &geog_dwithin;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	bool result = false;
	if (sync1->type == TEMPORALINST) 
		result = spatialrel3_tpointinst_tpointinst(
			(TemporalInst *)sync1, (TemporalInst *)sync2, dist, operator);
	else if (sync1->type == TEMPORALI) 
		result = spatialrel3_tpointi_tpointi(
			(TemporalI *)sync1, (TemporalI *)sync2, dist, operator);
	else if (sync1->type == TEMPORALSEQ) 
		result = dwithin_tpointseq_tpointseq(
			(TemporalSeq *)sync1, (TemporalSeq *)sync2, dist, operator);
	else if (sync1->type == TEMPORALS) 
		result = dwithin_tpoints_tpoints(
			(TemporalS *)sync1, (TemporalS *)sync2, dist, operator);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Temporal relate
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_geo_tpoint);

PGDLLEXPORT Datum
relate_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	text *result = relate2_tpoint_geo(temp, PointerGetDatum(gs), true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(relate_tpoint_geo);

PGDLLEXPORT Datum
relate_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (temp->type == TEMPORALS)
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The relate operator cannot accept temporal points of sequence set duration")));
	}
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	text *result = relate2_tpoint_geo(temp, PointerGetDatum(gs), true);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_TEXT_P(result);
}
 
PG_FUNCTION_INFO_V1(relate_tpoint_tpoint);

PGDLLEXPORT Datum
relate_tpoint_tpoint(PG_FUNCTION_ARGS)
{
    Temporal *temp1 = PG_GETARG_TEMPORAL(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	if (temp1->type == TEMPORALS || temp2->type == TEMPORALS)
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The relate operator cannot accept temporal points of sequence set duration")));
	}
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = NULL;
	if (inter1->type == TEMPORALINST)
		result = (Temporal *)relate2_tpointinst_tpointinst(
			(TemporalInst *)inter1, (TemporalInst *)inter2);
	else if (inter1->type == TEMPORALI)
		result = (Temporal *)relate2_tpointi_tpointi(
			(TemporalI *)inter1, (TemporalI *)inter2);			
	else if (inter1->type == TEMPORALSEQ)
		result = (Temporal *)relate2_tpointseq_tpointseq(
			(TemporalSeq *)inter1, (TemporalSeq *)inter2);
	else if (inter1->type == TEMPORALS)
		result = (Temporal *)relate2_tpoints_tpoints(
			(TemporalS *)inter1, (TemporalS *)inter2);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	pfree(inter1); pfree(inter2); 
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    if (result == NULL)
        PG_RETURN_NULL();
    PG_RETURN_POINTER(result);
}
 
/*****************************************************************************
 * Temporal relate_pattern
 *****************************************************************************/

PG_FUNCTION_INFO_V1(relate_pattern_geo_tpoint);

PGDLLEXPORT Datum
relate_pattern_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	if (gserialized_get_srid(gs) != tpoint_srid_internal(temp))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (FLAGS_GET_Z(gs->flags) != MOBDB_FLAGS_GET_Z(temp->flags))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), pattern,
		&geom_relate_pattern, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_geo);

PGDLLEXPORT Datum
relate_pattern_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	Datum pattern = PG_GETARG_DATUM(2);
	if (tpoint_srid_internal(temp) != gserialized_get_srid(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp->flags) != FLAGS_GET_Z(gs->flags))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The geometries must be of the same dimensionality")));
	}

	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), pattern,
		&geom_relate_pattern, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_tpoint);

PGDLLEXPORT Datum
relate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS)
{
    Temporal *temp1 = PG_GETARG_TEMPORAL(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	if (tpoint_srid_internal(temp1) != tpoint_srid_internal(temp2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be in the same SRID")));
	}
	if (MOBDB_FLAGS_GET_Z(temp1->flags) != MOBDB_FLAGS_GET_Z(temp2->flags))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The temporal points must be of the same dimensionality")));
	}

	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	bool result = spatialrel3_tpoint_tpoint(inter1, inter2, pattern, &geom_relate_pattern);

	pfree(inter1); pfree(inter2); 
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

