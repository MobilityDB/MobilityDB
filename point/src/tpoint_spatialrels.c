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
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_spatialrels.h"

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_distance.h"

/*****************************************************************************
 * Spatial relationship functions
 * contains and within are inverse to each other
 * covers and coveredby are inverse to each other
 *****************************************************************************/

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
geog_intersects(Datum geog1, Datum geog2)
{
	double dist = DatumGetFloat8(geog_distance(geog1, geog2));
	return BoolGetDatum(dist < 0.00001);
}

Datum
geog_dwithin(Datum geog1, Datum geog2, Datum dist)
{
	return CallerFInfoFunctionCall4(geography_dwithin, (fetch_fcinfo())->flinfo, 
		InvalidOid, geog1, geog2, dist, BoolGetDatum(true));
}

/*****************************************************************************
 * Generic dwithin functions when both temporal points are moving
 * The functions suppose that the temporal points are synchronized
 * TODO: VERIFY THAT THESE FUNCTIONS CORRECT !!!
 *****************************************************************************/

static bool
dwithin_tpointseq_tpointseq1(TemporalInst *start1, TemporalInst *end1, bool linear1,
	TemporalInst *start2, TemporalInst *end2, bool linear2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	Datum sv1 = temporalinst_value(start1);
	Datum ev1 = temporalinst_value(end1);
	Datum sv2 = temporalinst_value(start2);
	Datum ev2 = temporalinst_value(end2);
	/* If both instants are constant compute the function at the start instant */
	if (datum_point_eq(sv1, ev1) &&	datum_point_eq(sv2, ev2))
		return DatumGetBool(func(sv1, sv2, param));
	
	/* Determine whether there is a local minimum between lower and upper */
	TimestampTz crosstime;
	bool cross = tpointseq_min_dist_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	/* If there is no local minimum compute the function at the start instant */	
	if (! cross)
		return DatumGetBool(func(sv1, sv2, param));

	/* Find the values at the local minimum */
	Datum crossvalue1 = temporalseq_value_at_timestamp1(start1, end1, linear1, crosstime);
	Datum crossvalue2 = temporalseq_value_at_timestamp1(start2, end2, linear2, crosstime);
	/* Compute the function at the local minimum */
	bool result = DatumGetBool(func(crossvalue1, crossvalue2, param));
	
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
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		if (dwithin_tpointseq_tpointseq1(start1, end1, 
			MOBDB_FLAGS_GET_LINEAR(seq1->flags), start2, end2,
			MOBDB_FLAGS_GET_LINEAR(seq2->flags), d, func))
			return true;
		start1 = end1;
		start2 = end2;
	}
	return DatumGetBool(func(temporalinst_value(start1), 
		temporalinst_value(start2), d));
}

static bool
dwithin_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum d,
	Datum (*func)(Datum, Datum, Datum))
{
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		if (dwithin_tpointseq_tpointseq(seq1, seq2, d, func))
			return true;
	}
	return false;
}

/*****************************************************************************
 * Generic functions
 * The functions that have two temporal points as arguments suppose that they
 * have been intersected before by the calling function. As a consequence of
 * this, they have the same timeframe and they are of the same duration.
 *****************************************************************************/

static Datum
spatialrel_tpoint_geo(Temporal *temp, Datum geo,
	Datum (*func)(Datum, Datum), bool invert)
{
	Datum traj = tpoint_trajectory_internal(temp);
	Datum result = invert ? func(geo, traj) : func(traj, geo);
	pfree(DatumGetPointer(traj));
	return result;
}
 
static Datum
spatialrel3_tpoint_geo(Temporal *temp, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Datum traj = tpoint_trajectory_internal(temp);
	Datum result = invert ? func(geo, traj, param) : func(traj, geo, param);
	pfree(DatumGetPointer(traj));
	return result;
}

static Datum
spatialrel_tpoint_tpoint(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum))
{
	Datum traj1 = tpoint_trajectory_internal(temp1);
	Datum traj2 = tpoint_trajectory_internal(temp2);
	Datum result = func(traj1, traj2);
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2)); 
	return result;
}

static Datum
spatialrel3_tpoint_tpoint(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	Datum traj1 = tpoint_trajectory_internal(temp1);
	Datum traj2 = tpoint_trajectory_internal(temp2);
	Datum result = func(traj1, traj2, param);
	pfree(DatumGetPointer(traj1)); pfree(DatumGetPointer(traj2));
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_contains, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(contains_tpoint_geo);

PGDLLEXPORT Datum
contains_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_contains, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(contains_tpoint_tpoint);

PGDLLEXPORT Datum
contains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_contains);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_containsproperly, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(containsproperly_tpoint_geo);

PGDLLEXPORT Datum
containsproperly_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_containsproperly, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(containsproperly_tpoint_tpoint);

PGDLLEXPORT Datum
containsproperly_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_containsproperly);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(covers_tpoint_geo);

PGDLLEXPORT Datum
covers_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(covers_tpoint_tpoint);

PGDLLEXPORT Datum
covers_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, func);

	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(coveredby_tpoint_geo);

PGDLLEXPORT Datum
coveredby_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(coveredby_tpoint_tpoint);

PGDLLEXPORT Datum
coveredby_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, func);

	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_crosses, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(crosses_tpoint_geo);

PGDLLEXPORT Datum
crosses_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_crosses, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(crosses_tpoint_tpoint);

PGDLLEXPORT Datum
crosses_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_crosses);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(disjoint_tpoint_geo);

PGDLLEXPORT Datum
disjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(disjoint_tpoint_tpoint);

PGDLLEXPORT Datum
disjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_disjoint);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_equals, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(equals_tpoint_geo);

PGDLLEXPORT Datum
equals_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_equals, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(equals_tpoint_tpoint);

PGDLLEXPORT Datum
equals_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_equals);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		func, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(intersects_tpoint_geo);

PGDLLEXPORT Datum
intersects_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(intersects_tpoint_tpoint);

PGDLLEXPORT Datum
intersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum) = NULL;
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_intersects2d;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, func);

	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_overlaps, true);			
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(overlaps_tpoint_geo);

PGDLLEXPORT Datum
overlaps_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_overlaps, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(overlaps_tpoint_tpoint);

PGDLLEXPORT Datum
overlaps_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_overlaps);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_touches, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(touches_tpoint_geo);

PGDLLEXPORT Datum
touches_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_touches, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(touches_tpoint_tpoint);

PGDLLEXPORT Datum
touches_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_touches);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_within, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(within_tpoint_geo);

PGDLLEXPORT Datum
within_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_within, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(within_tpoint_tpoint);

PGDLLEXPORT Datum
within_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_within);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	Datum result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		func, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}
 
PG_FUNCTION_INFO_V1(dwithin_tpoint_geo);

PGDLLEXPORT Datum
dwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	Datum dist = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum, Datum) = NULL;
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	Datum result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), dist,
		func, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(dwithin_tpoint_tpoint);

PGDLLEXPORT Datum
dwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *sync1, *sync2;
	/* Returns false if the temporal points do not intersect in time 
	 * The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum (*func)(Datum, Datum, Datum) = NULL;
	ensure_point_base_type(temp1->valuetypid);
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
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == TEMPORALINST || sync1->duration == TEMPORALI) 
		result = DatumGetBool(spatialrel3_tpoint_tpoint(sync1, sync2, dist, func));
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_relate, false);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_relate, false);
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
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel_tpoint_tpoint(inter1, inter2, &geom_relate);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), pattern,
		&geom_relate_pattern, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_geo);

PGDLLEXPORT Datum
relate_pattern_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel3_tpoint_geo(temp, PointerGetDatum(gs), pattern,
		&geom_relate_pattern, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(relate_pattern_tpoint_tpoint);

PGDLLEXPORT Datum
relate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *inter1, *inter2;
	/* Returns false if the temporal points do not intersect in time */
	if (!intersection_temporal_temporal(temp1, temp2, &inter1, &inter2))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Datum result = spatialrel3_tpoint_tpoint(inter1, inter2, pattern, &geom_relate_pattern);
	pfree(inter1); pfree(inter2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************/

