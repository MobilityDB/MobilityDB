/*****************************************************************************
 *
 * tpoint_spatialrels.c
 *	  Spatial relationships for temporal points.
 *
 * These relationships project the time dimension and return a Boolean.
 * They are thus defined with the "at any instant" semantics, that is, the
 * traditional spatial function is applied to the union of all values taken 
 * by the temporal point. The following relationships are supported for 
 * geometries:
 *		contains, containsproperly, covers, coveredby, crosses, disjoint, 
 *		contains, containsproperly, covers, coveredby, crosses, disjoint,
 *		equals, intersects, overlaps, touches, within, dwithin, and
 *		relate (with 2 and 3 arguments)
 * The following relationships are supported for geographies
 *	 covers, coveredby, intersects, dwithin
 * Only dwithin and intersects support 3D geometries.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_spatialrels.h"

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Spatial relationship functions
 *****************************************************************************/

/* 
 * contains and within are inverse to each other
 * covers and coveredby are inverse to each other
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
#if MOBDB_POSTGIS_VERSION >= 30
	return call_function2(ST_Intersects, geom1, geom2);
#else	
	return call_function2(intersects, geom1, geom2);
#endif
}

Datum
geom_intersects3d(Datum geom1, Datum geom2)
{
#if MOBDB_POSTGIS_VERSION >= 30
	return call_function2(ST_3DIntersects, geom1, geom2);
#else	
	return call_function2(intersects3d, geom1, geom2);
#endif
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
 * The functions that have two temporal points as argument suppose that they
 * overlap on the time dimension. This is ensured in the external function 
 * in order to return NULL if it is not the case.
 *****************************************************************************/

/* Temporal spatialrel geo */

static bool
spatialrel_tpointinst_geo(TemporalInst *inst, Datum geo, 
	Datum (*func)(Datum, Datum), bool invert)
{
	Datum value = temporalinst_value(inst);
	bool result = invert ? DatumGetBool(func(geo, value)) :
		DatumGetBool(func(value, geo));
	return result;
}

static bool
spatialrel_tpointi_geo(TemporalI *ti, Datum geo,
	Datum (*func)(Datum, Datum), bool invert)
{
	Datum values = tpointi_values(ti);
	bool result =  invert ? DatumGetBool(func(geo, values)) :
		DatumGetBool(func(values, geo));
	pfree(DatumGetPointer(values)); 
	return result;
}

static bool
spatialrel_tpointseq_geo(TemporalSeq *seq, Datum geo, 
	Datum (*func)(Datum, Datum), bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	bool result = invert ? DatumGetBool(func(geo, traj)) :
		DatumGetBool(func(traj, geo));
	return result;
}

static bool
spatialrel_tpoints_geo(TemporalS *ts, Datum geo,
	Datum (*func)(Datum, Datum), bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	bool result = invert ? DatumGetBool(func(geo, traj)) :
		DatumGetBool(func(traj, geo));
	pfree(DatumGetPointer(traj));
	return result;
}

/*****************************************************************************/

/* Temporal spatialrel Temporal */

static bool
spatialrel_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*func)(Datum, Datum))
{
	return DatumGetBool(func(temporalinst_value(inst1), 
		temporalinst_value(inst2)));
}

static bool
spatialrel_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*func)(Datum, Datum))
{
	Datum geo1 = tpointi_values(ti1);
	Datum geo2 = tpointi_values(ti2);
	bool result = DatumGetBool(func(geo1, geo2));
	pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2));
	return result;
}

static bool
spatialrel_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum))
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	bool result = DatumGetBool(func(traj1, traj2));
	return result;
}

static bool
spatialrel_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2,
	Datum (*func)(Datum, Datum))
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	bool result = DatumGetBool(func(traj1, traj2));
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2)); 
	return result;
}

/*****************************************************************************
 * Generic ternary functions
 *****************************************************************************/

/* Temporal spatialrel Geo */

static bool
spatialrel3_tpointinst_geo(TemporalInst *inst, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Datum value = temporalinst_value(inst);
	Datum result = invert ? DatumGetBool(func(geo, value, param)) :
		DatumGetBool(func(value, geo, param));
	return result;
}

static bool
spatialrel3_tpointi_geo(TemporalI *ti, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Datum values = tpointi_values(ti);
	bool result = invert ? DatumGetBool(func(geo, values, param)) :
		DatumGetBool(func(values, geo, param));
	return result;
}

static bool
spatialrel3_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	bool result = invert ? DatumGetBool(func(geo, traj, param)) :
		DatumGetBool(func(traj, geo, param));
	return result;
}

static bool
spatialrel3_tpoints_geo(TemporalS *ts, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	bool result = invert ? DatumGetBool(func(geo, traj, param)) :
		DatumGetBool(func(traj, geo, param));
	pfree(DatumGetPointer(traj));
	return result;
}

/*****************************************************************************/

/* Temporal spatialrel Temporal */

static bool
spatialrel3_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	return DatumGetBool(func(temporalinst_value(inst1), 
		temporalinst_value(inst2), param));
}

static bool
spatialrel3_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	Datum geo1 = tpointi_values(ti1);
	Datum geo2 = tpointi_values(ti2);
	bool result = DatumGetBool(func(geo1, geo2, param));
	pfree(DatumGetPointer(geo1)); pfree(DatumGetPointer(geo2)); 
	return result;
}

static bool
spatialrel3_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	bool result = DatumGetBool(func(traj1, traj2, param));
	return result;
}

static bool
spatialrel3_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	bool result = DatumGetBool(func(traj1, traj2, param));
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
	Datum (*func)(Datum, Datum, Datum))
{
	Datum sv1 = temporalinst_value(start1);
	Datum ev1 = temporalinst_value(end1);
	Datum sv2 = temporalinst_value(start2);
	Datum ev2 = temporalinst_value(end2);
	/* Both instants are constant */
	if (datum_point_eq(sv1, ev1) &&	datum_point_eq(sv2, ev2))
	{
		/* Compute the function at the start instant */
		return func(sv1, sv2, param);
	}
	
	TimestampTz lower = start1->t;
	TimestampTz upper = start2->t;
	/* Determine whether there is a local minimum between lower and upper */
	TimestampTz crosstime;
	bool cross = tpointseq_min_dist_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	/* If there is no local minimum or if there is one at a bound */	
	if (!cross || crosstime == lower || crosstime == upper)
	{
		/* Compute the function at the start and end instants */
		return func(sv1, sv2, param);
	}

	/* Find the values at the local minimum */
	Datum crossvalue1 = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum crossvalue2 = temporalseq_value_at_timestamp1(start2, end2, crosstime);
	/* Compute the function at the start instant and at the local minimum */
	bool result = func(crossvalue1, crossvalue2, param);
	
	pfree(DatumGetPointer(crossvalue1));
	pfree(DatumGetPointer(crossvalue2));
		
	return result;
}

static bool
dwithin_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum d,
	Datum (*func)(Datum, Datum, Datum))
{
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool result;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		result = dwithin_tpointseq_tpointseq1(start1, end1, 
			start2, end2, d, func);
		if (result == true)
			return true;
		start1 = end1;
		start2 = end2;
	}
	result = spatialrel3_tpointinst_tpointinst(start1, start2, d, func);
	return result;
}

static bool
dwithin_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum d,
	Datum (*func)(Datum, Datum, Datum))
{
	bool result = false;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		if (dwithin_tpointseq_tpointseq(seq1, seq2, d, func))
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

static Datum
relate_tpointinst_geo(TemporalInst *inst, Datum geo, bool invert)
{
	Datum value = temporalinst_value(inst);
	return invert ? geom_relate(geo, value) : geom_relate(value, geo);
}

/* This function assumes that inst1->t == inst2-> t */
static Datum
relate_tpointinst_tpointinst(TemporalInst *inst1, TemporalInst *inst2)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	return geom_relate(value1, value2);
}

/*****************************************************************************/

/* TemporalI relate <Type> */

static Datum
relate_tpointi_geo(TemporalI *ti, Datum geo, bool invert)
{
	Datum values = tpointi_values(ti);
	Datum result = invert ? geom_relate(geo, values) : geom_relate(values, geo);
	pfree(DatumGetPointer(values));
	return result;
}

static Datum
relate_tpointi_tpointi(TemporalI *ti1, TemporalI *ti2)
{
	Datum values1 = tpointi_values(ti1);
	Datum values2 = tpointi_values(ti2);
	Datum result = geom_relate(values1, values2);
	pfree(DatumGetPointer(values1)); pfree(DatumGetPointer(values2)); 
	return result;
}

/*****************************************************************************/

/* TemporalSeq relate <Type> */

static Datum
relate_tpointseq_geo(TemporalSeq *seq, Datum geo, bool invert)
{
	Datum traj = tpointseq_trajectory(seq);
	return invert ? geom_relate(geo, traj) : geom_relate(traj, geo);
}

static Datum
relate_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2)
{
	Datum traj1 = tpointseq_trajectory(seq1);
	Datum traj2 = tpointseq_trajectory(seq2);
	return geom_relate(traj1, traj2);
}

/*****************************************************************************/

/* TemporalS relate <Type> */

static Datum
relate_tpoints_geo(TemporalS *ts, Datum geo, bool invert)
{
	Datum traj = tpoints_trajectory(ts);
	return invert ? geom_relate(geo, traj) : geom_relate(traj, geo);
}

static Datum
relate_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2)
{
	Datum traj1 = tpoints_trajectory(ts1);
	Datum traj2 = tpoints_trajectory(ts2);
	return geom_relate(traj1, traj2);
}

/*****************************************************************************
 * Dispatch functions
 * It is supposed that the temporal values have been intersected before and
 * therefore they are both of the same duration.
 *****************************************************************************/

static bool
spatialrel_tpoint_geo(Temporal *temp, Datum geo,
	Datum (*func)(Datum, Datum), bool invert)
{
	bool result = false;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = spatialrel_tpointinst_geo((TemporalInst *)temp,
			geo, func, invert);
	else if (temp->duration == TEMPORALI) 
		result = spatialrel_tpointi_geo((TemporalI *)temp,
			geo, func, invert);
	else if (temp->duration == TEMPORALSEQ) 
		result = spatialrel_tpointseq_geo((TemporalSeq *)temp,
			geo, func, invert);
	else if (temp->duration == TEMPORALS) 
		result = spatialrel_tpoints_geo((TemporalS *)temp,
			geo, func, invert);
	return result;
}
 
static bool
spatialrel3_tpoint_geo(Temporal *temp, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	bool result = false;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = spatialrel3_tpointinst_geo((TemporalInst *)temp,
			geo, param, func, invert);
	else if (temp->duration == TEMPORALI) 
		result = spatialrel3_tpointi_geo((TemporalI *)temp,
			geo, param, func, invert);
	else if (temp->duration == TEMPORALSEQ) 
		result = spatialrel3_tpointseq_geo((TemporalSeq *)temp,
			geo, param, func, invert);
	else if (temp->duration == TEMPORALS) 
		result = spatialrel3_tpoints_geo((TemporalS *)temp,
			geo, param, func, invert);
	return result;
}
 
static bool
spatialrel_tpoint_tpoint(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum))
{
	bool result = false;
	temporal_duration_is_valid(temp1->duration);
	if (temp1->duration == TEMPORALINST) 
		result = spatialrel_tpointinst_tpointinst((TemporalInst *)temp1,
			(TemporalInst *)temp2, func);
	else if (temp1->duration == TEMPORALI) 
		result = spatialrel_tpointi_tpointi((TemporalI *)temp1,
			(TemporalI *)temp2, func);
	else if (temp1->duration == TEMPORALSEQ) 
		result = spatialrel_tpointseq_tpointseq((TemporalSeq *)temp1,
			(TemporalSeq *)temp2, func);
	else if (temp1->duration == TEMPORALS) 
		result = spatialrel_tpoints_tpoints((TemporalS *)temp1,
			(TemporalS *)temp2, func);
	return result;
}

static bool
spatialrel3_tpoint_tpoint(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	bool result = false;
	temporal_duration_is_valid(temp1->duration);
	if (temp1->duration == TEMPORALINST) 
		result = spatialrel3_tpointinst_tpointinst((TemporalInst *)temp1,
			(TemporalInst *)temp2, param, func);
	else if (temp1->duration == TEMPORALI) 
		result = spatialrel3_tpointi_tpointi((TemporalI *)temp1,
			(TemporalI *)temp2, param, func);
	else if (temp1->duration == TEMPORALSEQ) 
		result = spatialrel3_tpointseq_tpointseq((TemporalSeq *)temp1,
			(TemporalSeq *)temp2, param, func);
	else if (temp1->duration == TEMPORALS) 
		result = spatialrel3_tpoints_tpoints((TemporalS *)temp1,
			(TemporalS *)temp2, param, func);
	return result;
}

static Datum
relate_tpoint_geo_internal(Temporal *temp, Datum geo, bool invert)
{
	Datum result = 0;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = relate_tpointinst_geo((TemporalInst *)temp, geo, invert);
	else if (temp->duration == TEMPORALI)
		result = relate_tpointi_geo((TemporalI *)temp, geo, invert);
	else if (temp->duration == TEMPORALSEQ)
		result = relate_tpointseq_geo((TemporalSeq *)temp, geo, invert);
	else if (temp->duration == TEMPORALS)
		result = relate_tpoints_geo((TemporalS *)temp, geo, invert);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, true);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);

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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, func);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);

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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, func);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, true);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	bool result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, false);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_intersects2d;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	bool result = spatialrel_tpoint_tpoint(inter1, inter2, func);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		func, true);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	bool result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		func, false);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *sync1, *sync2;
	/* Returns false if the temporal points do not intersect in time 
	 * The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;

	bool result = false;
	temporal_duration_is_valid(sync1->duration);
	if (sync1->duration == TEMPORALINST) 
		result = spatialrel3_tpointinst_tpointinst(
			(TemporalInst *)sync1, (TemporalInst *)sync2, dist, func);
	else if (sync1->duration == TEMPORALI) 
		result = spatialrel3_tpointi_tpointi(
			(TemporalI *)sync1, (TemporalI *)sync2, dist, func);
	else if (sync1->duration == TEMPORALSEQ) 
		result = dwithin_tpointseq_tpointseq(
			(TemporalSeq *)sync1, (TemporalSeq *)sync2, dist, func);
	else if (sync1->duration == TEMPORALS) 
		result = dwithin_tpoints_tpoints(
			(TemporalS *)sync1, (TemporalS *)sync2, dist, func);

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Datum result = relate_tpoint_geo_internal(temp, PointerGetDatum(gs), true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(relate_tpoint_geo);

PGDLLEXPORT Datum
relate_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Datum result = relate_tpoint_geo_internal(temp, PointerGetDatum(gs), false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(relate_tpoint_tpoint);

PGDLLEXPORT Datum
relate_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Datum result = 0;
	temporal_duration_is_valid(inter1->duration);
	if (inter1->duration == TEMPORALINST)
		result = relate_tpointinst_tpointinst(
			(TemporalInst *)inter1, (TemporalInst *)inter2);
	else if (inter1->duration == TEMPORALI)
		result = relate_tpointi_tpointi(
			(TemporalI *)inter1, (TemporalI *)inter2);			
	else if (inter1->duration == TEMPORALSEQ)
		result = relate_tpointseq_tpointseq(
			(TemporalSeq *)inter1, (TemporalSeq *)inter2);
	else if (inter1->duration == TEMPORALS)
		result = relate_tpoints_tpoints(
			(TemporalS *)inter1, (TemporalS *)inter2);

	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
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

