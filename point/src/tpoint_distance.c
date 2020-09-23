/*****************************************************************************
 *
 * tpoint_distance.c
 *	  Temporal distance for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_distance.h"

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/* Distance between two geometries */

Datum
geom_distance2d(Datum geom1, Datum geom2)
{
	return call_function2(distance, geom1, geom2);
}

Datum
geom_distance3d(Datum geom1, Datum geom2)
{
	return call_function2(distance3d, geom1, geom2);
}

/* Distance between two geographies */

Datum
geog_distance(Datum geog1, Datum geog2)
{
	return CallerFInfoFunctionCall2(geography_distance, (fetch_fcinfo())->flinfo,
		InvalidOid, geog1, geog2);
}

/*****************************************************************************/
 
/* Distance between temporal sequence point and a geometry/geography point */

static int
distance_tpointseq_geo1(TemporalInst **result,
	TemporalInst *inst1, TemporalInst *inst2, bool linear, 
	Datum point, Datum (*func)(Datum, Datum))
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	/* Constant segment or stepwise interpolation */
	if (datum_point_eq(value1, value2) || ! linear)
	{
		result[0] = temporalinst_make(func(point, value1),
			inst1->t, FLOAT8OID); 
		return 1;
	}
	double fraction;
	ensure_point_base_type(inst1->valuetypid);
	if (inst1->valuetypid == type_oid(T_GEOMETRY))
	{
		/* The trajectory is a line */
		Datum traj = geompoint_trajectory(value1, value2);
		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			traj, point));
		pfree(DatumGetPointer(traj)); 
	}
	else
	{
		/* The trajectory is a line */
		Datum traj = geogpoint_trajectory(value1, value2);
		/* There is no function equivalent to LWGEOM_line_locate_point 
		 * for geographies. We do as the ST_Intersection function, e.g.
		 * 'SELECT geography(ST_Transform(ST_Intersection(ST_Transform(geometry($1), 
		 * @extschema@._ST_BestSRID($1, $2)), 
		 * ST_Transform(geometry($2), @extschema@._ST_BestSRID($1, $2))), 4326))' */
		Datum bestsrid = call_function2(geography_bestsrid, traj, point);
		Datum traj1 = call_function1(geometry_from_geography, traj);
		Datum traj2 = call_function2(transform, traj1, bestsrid);
		Datum point1 = call_function1(geometry_from_geography, point);
		Datum point2 = call_function2(transform, point, bestsrid);
		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			traj2, point2));
		pfree(DatumGetPointer(traj)); pfree(DatumGetPointer(traj1)); 
		pfree(DatumGetPointer(traj2)); pfree(DatumGetPointer(point1));
		pfree(DatumGetPointer(point2));
	}

	if (fraction == 0 || fraction == 1)
	{
		result[0] = temporalinst_make(func(point, value1),
			inst1->t, FLOAT8OID);
		return 1;
	}

	TimestampTz time = inst1->t + (long) ((double) (inst2->t - inst1->t) * fraction);
	Datum value = temporalseq_value_at_timestamp1(inst1, inst2, linear, time);
	result[0] = temporalinst_make(func(point, value1),
		inst1->t, FLOAT8OID);
	result[1] = temporalinst_make(func(point, value), time,
		FLOAT8OID);
	pfree(DatumGetPointer(value));
	return 2;
}

/* Distance between temporal sequence point and a geometry/geography point */

static TemporalSeq *
distance_tpointseq_geo(TemporalSeq *seq, Datum point, 
	Datum (*func)(Datum, Datum))
{
	int k = 0;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count * 2);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
        /* The next step adds between one and three sequences */
		k += distance_tpointseq_geo1(&instants[k], inst1, inst2,
			MOBDB_FLAGS_GET_LINEAR(seq->flags), point, func);
		inst1 = inst2;
	}
	instants[k++] = temporalinst_make(func(point, temporalinst_value(inst1)),
		inst1->t, FLOAT8OID); 
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		seq->period.lower_inc, seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

/* Distance between temporal sequence point and a geometry/geography point */

static TemporalS *
distance_tpoints_geo(TemporalS *ts, Datum point, 
	Datum (*func)(Datum, Datum))
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = distance_tpointseq_geo(seq, point, func);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, 
		MOBDB_FLAGS_GET_LINEAR(ts->flags), true);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* 
 * Find the single timestamptz at which two temporal point segments are at the
 * minimum distance. This function is used for computing temporal distance.
 * The function assumes that the two segments are not both constants.
 */
bool
tpointseq_min_dist_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	double denum, fraction;
	if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
	{
		POINT3DZ p1 = datum_get_point3dz(temporalinst_value(start1));
		POINT3DZ p2 = datum_get_point3dz(temporalinst_value(end1));
		POINT3DZ p3 = datum_get_point3dz(temporalinst_value(start2));
		POINT3DZ p4 = datum_get_point3dz(temporalinst_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0.
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		double dx1 = p2.x - p1.x;
		double dy1 = p2.y - p1.y;
		double dz1 = p2.z - p1.z;
		double dx2 = p4.x - p3.x;
		double dy2 = p4.y - p3.y;
		double dz2 = p4.z - p3.z;
		
		double f1 = p3.x * (dx1 - dx2);
		double f2 = p1.x * (dx2 - dx1);
		double f3 = p3.y * (dy1 - dy2);
		double f4 = p1.y * (dy2 - dy1);
		double f5 = p3.z * (dz1 - dz2);
		double f6 = p1.z * (dz2 - dz1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) + 
			dx2*dx2 + dy2*dy2 + dz2*dz2;
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
	}
	else /* 2D */
	{
		POINT2D p1 = datum_get_point2d(temporalinst_value(start1));
		POINT2D p2 = datum_get_point2d(temporalinst_value(end1));
		POINT2D p3 = datum_get_point2d(temporalinst_value(start2));
		POINT2D p4 = datum_get_point2d(temporalinst_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0.
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		double dx1 = p2.x - p1.x;
		double dy1 = p2.y - p1.y;
		double dx2 = p4.x - p3.x;
		double dy2 = p4.y - p3.y;
		
		double f1 = p3.x * (dx1 - dx2);
		double f2 = p1.x * (dx2 - dx1);
		double f3 = p3.y * (dy1 - dy2);
		double f4 = p1.y * (dy2 - dy1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dy2*dy2 + dx2*dx2;
		/* If the segments are parallel */
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4) / denum;
	}
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		return false;
	*t = start1->t + (long) ((double)(end1->t - start1->t) * fraction);
	return true;
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(distance_geo_tpoint);

PGDLLEXPORT Datum
distance_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_point_type(gs);
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

	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (FLAGS_GET_Z(gs->flags) && MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;

	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, FLOAT8OID, true);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, FLOAT8OID, true);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)distance_tpointseq_geo((TemporalSeq *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)distance_tpoints_geo((TemporalS *)temp,
			PointerGetDatum(gs), func);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(distance_tpoint_geo);

PGDLLEXPORT Datum
distance_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_point_type(gs);
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
	
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (FLAGS_GET_Z(gs->flags) && MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;

	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, FLOAT8OID, true);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, FLOAT8OID, true);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)distance_tpointseq_geo((TemporalSeq *)temp,
			PointerGetDatum(gs), func);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)distance_tpoints_geo((TemporalS *)temp,
			PointerGetDatum(gs), func);

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

Temporal *
distance_tpoint_tpoint_internal(Temporal *temp1, Temporal *temp2)
{
	Datum (*func)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			func = &geom_distance3d;
		else
			func = &geom_distance2d;
	}
	else
		func = &geog_distance;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = linear ?
		sync_tfunc2_temporal_temporal(temp1, temp2, func, 
			FLOAT8OID, linear, &tpointseq_min_dist_at_timestamp) :
		sync_tfunc2_temporal_temporal(temp1, temp2, func, 
			FLOAT8OID, linear, NULL);
	return result;
}

PG_FUNCTION_INFO_V1(distance_tpoint_tpoint);

PGDLLEXPORT Datum
distance_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Temporal *result = distance_tpoint_tpoint_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
	