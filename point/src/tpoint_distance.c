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

/** 
 * Returns the 2D distance between the two geometries 
 */
Datum
geom_distance2d(Datum geom1, Datum geom2)
{
	return call_function2(distance, geom1, geom2);
}

/** 
 * Returns the 3D distance between the two geometries 
 */
Datum
geom_distance3d(Datum geom1, Datum geom2)
{
	return call_function2(distance3d, geom1, geom2);
}

/** 
 * Returns the distance between the two geographies 
 */
Datum
geog_distance(Datum geog1, Datum geog2)
{
	return call_function4(geography_distance, geog1, geog2, 
		Float8GetDatum(0.0), BoolGetDatum(true));
}

/** 
 * Returns the 2D distance between the two geometric points 
 */
Datum
pt_distance2d(Datum geom1, Datum geom2)
{
	const POINT2D *p1 = datum_get_point2d_p(geom1);
	const POINT2D *p2 = datum_get_point2d_p(geom2);
	return Float8GetDatum(distance2d_pt_pt(p1, p2));
}

/** 
 * Returns the 3D distance between the two geometric points 
 */
Datum
pt_distance3d(Datum geom1, Datum geom2)
{
	const POINT3DZ *p1 = datum_get_point3dz_p(geom1);
	const POINT3DZ *p2 = datum_get_point3dz_p(geom2);
	return Float8GetDatum(distance3d_pt_pt((POINT3D *)p1, (POINT3D *)p2));
}

/*****************************************************************************/
 
/**
 * Returns the temporal distance between the temporal sequence point and
 * the geometry/geography point
 *
 * @param[in] seq Temporal point
 * @param[in] point Point
 * @param[in] func Distance function
 */
static TSequence *
distance_tpointseq_geo(const TSequence *seq, Datum point,
	Datum (*func)(Datum, Datum))
{
	int k = 0;
	TInstant **instants = palloc(sizeof(TInstant *) * seq->count * 2);
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	Datum value1 = tinstant_value(inst1);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	/* Ensure this outside of the loop */
	ensure_point_base_type(inst1->valuetypid);
	for (int i = 1; i < seq->count; i++)
	{
		/* Each iteration of the loop adds between one and three points */
		TInstant *inst2 = tsequence_inst_n(seq, i);
		Datum value2 = tinstant_value(inst2);

		/* Constant segment or step interpolation */
		if (datum_point_eq(value1, value2) || ! linear)
		{
			instants[k++] = tinstant_make(func(point, value1),
				inst1->t, FLOAT8OID);
		}
		else
		{
			/* The trajectory is a line */
			long double fraction;
			long double duration = (long double) (inst2->t - inst1->t);
			double dist;
			if (inst1->valuetypid == type_oid(T_GEOMETRY))
				fraction = (long double) geomseg_locate_point(value1, value2, point, &dist);
			else
				fraction = (long double) geogseg_locate_point(value1, value2, point, &dist);

			if (fraction == 0.0 || fraction == 1.0)
			{
				instants[k++] = tinstant_make(func(point, value1),
					inst1->t, FLOAT8OID);
			}
			else
			{
				TimestampTz time = inst1->t + (long) (duration * fraction);
				instants[k++] = tinstant_make(func(point, value1),
					inst1->t, FLOAT8OID);
				instants[k++] = tinstant_make(Float8GetDatum(dist),
					time, FLOAT8OID);
			}
		}
		inst1 = inst2; value1 = value2;
	}
	instants[k++] = tinstant_make(func(point, value1), inst1->t, FLOAT8OID);
	
	return tsequence_make_free(instants, k, seq->period.lower_inc,
		seq->period.upper_inc, linear, true);
}

/**
 * Returns the temporal distance between the temporal sequence set point and
 * the geometry/geography point 
 *
 * @param[in] ts Temporal point
 * @param[in] point Point
 * @param[in] func Distance function
 */
static TSequenceSet *
distance_tpointseqset_geo(const TSequenceSet *ts, Datum point, 
	Datum (*func)(Datum, Datum))
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		sequences[i] = distance_tpointseq_geo(seq, point, func);
	}
	return tsequenceset_make_free(sequences, ts->count, true);
}

/**
 * Returns the single timestamp at which the two temporal geometric point
 * segments are at the minimum distance. These are the turning points 
 * when computing the temporal distance.
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] t Timestamp
 * @note The PostGIS functions `lw_dist2d_seg_seg` and `lw_dist3d_seg_seg`
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 * @note 
 */
bool
tgeompointseq_min_dist_at_timestamp(const TInstant *start1, 
	const TInstant *end1, const TInstant *start2, 
	const TInstant *end2, TimestampTz *t)
{
	long double denum, fraction;
	long double dx1, dy1, dx2, dy2, f1, f2, f3, f4;
	long double duration = (long double) (end1->t - start1->t);

	if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
	{
		long double dz1, dz2, f5, f6;
		const POINT3DZ *p1 = datum_get_point3dz_p(tinstant_value(start1));
		const POINT3DZ *p2 = datum_get_point3dz_p(tinstant_value(end1));
		const POINT3DZ *p3 = datum_get_point3dz_p(tinstant_value(start2));
		const POINT3DZ *p4 = datum_get_point3dz_p(tinstant_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0->
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		dx1 = p2->x - p1->x;
		dy1 = p2->y - p1->y;
		dz1 = p2->z - p1->z;
		dx2 = p4->x - p3->x;
		dy2 = p4->y - p3->y;
		dz2 = p4->z - p3->z;
		
		f1 = p3->x * (dx1 - dx2);
		f2 = p1->x * (dx2 - dx1);
		f3 = p3->y * (dy1 - dy2);
		f4 = p1->y * (dy2 - dy1);
		f5 = p3->z * (dz1 - dz2);
		f6 = p1->z * (dz2 - dz1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) + 
			dx2*dx2 + dy2*dy2 + dz2*dz2;
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
	}
	else /* 2D */
	{
		const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
		const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
		const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
		const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0.
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		dx1 = p2->x - p1->x;
		dy1 = p2->y - p1->y;
		dx2 = p4->x - p3->x;
		dy2 = p4->y - p3->y;
		
		f1 = p3->x * (dx1 - dx2);
		f2 = p1->x * (dx2 - dx1);
		f3 = p3->y * (dy1 - dy2);
		f4 = p1->y * (dy2 - dy1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dy2*dy2 + dx2*dx2;
		/* If the segments are parallel */
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4) / denum;
	}
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		return false;
	*t = start1->t + (long) (duration * fraction);
	return true;
}

/**
 * Returns the single timestamp at which the two temporal geographic point
 * segments are at the minimum distance. These are the turning points 
 * when computing the temporal distance.
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] mindist Minimum distance
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tgeogpointseq_min_dist_at_timestamp(const TInstant *start1, 
	const TInstant *end1, const TInstant *start2, 
	const TInstant *end2, double *mindist, TimestampTz *t)
{
	const POINT2D *p1 = datum_get_point2d_p(tinstant_value(start1));
	const POINT2D *p2 = datum_get_point2d_p(tinstant_value(end1));
	const POINT2D *p3 = datum_get_point2d_p(tinstant_value(start2));
	const POINT2D *p4 = datum_get_point2d_p(tinstant_value(end2));
	GEOGRAPHIC_EDGE e1, e2;
	GEOGRAPHIC_POINT close1, close2;
	POINT3D A1, A2, B1, B2;
	geographic_point_init(p1->x, p1->y, &(e1.start));
	geographic_point_init(p2->x, p2->y, &(e1.end));
	geographic_point_init(p3->x, p3->y, &(e2.start));
	geographic_point_init(p4->x, p4->y, &(e2.end));
	geog2cart(&(e1.start), &A1);
	geog2cart(&(e1.end), &A2);
	geog2cart(&(e2.start), &B1);
	geog2cart(&(e2.end), &B2);
	double fraction;
	if (edge_intersects(&A1, &A2, &B1, &B2))
	{
		/* We know that the distance is 0 */
		*mindist = 0.0;
		/* In this case we must take the tinstantsetty into account */
		long double dx1, dy1, dz1, dx2, dy2, dz2, f1, f2, f3, f4, f5, f6, denum;
		dx1 = A2.x - A1.x;
		dy1 = A2.y - A1.y;
		dz1 = A2.z - A1.z;
		dx2 = B2.x - B1.x;
		dy2 = B2.y - B1.y;
		dz2 = B2.z - B1.z;

		f1 = B1.x * (dx1 - dx2);
		f2 = A1.x * (dx2 - dx1);
		f3 = B1.y * (dy1 - dy2);
		f4 = A1.y * (dy2 - dy1);
		f5 = B1.z * (dz1 - dz2);
		f6 = A1.z * (dz2 - dz1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) +
			dx2*dx2 + dy2*dy2 + dz2*dz2;
		if (denum == 0)
			return false;

		fraction = (double) ((f1 + f2 + f3 + f4 + f5 + f6) / denum);
	}
	else
	{
		/* Compute closest points en each segment */
		edge_distance_to_edge(&e1, &e2, &close1, &close2);
		if (geographic_point_equals(&e1.start, &close1) ||
			geographic_point_equals(&e1.end, &close1))
			return false;
		/* Compute distance fbetween closest points */
		*mindist = WGS84_RADIUS * sphere_distance(&close1, &close2);
		/* Compute distance from beginning of the segment to one closest point */
		long double seglength = sphere_distance(&(e1.start), &(e1.end));
		long double length = sphere_distance(&(e1.start), &close1);
		fraction = (double) (length / seglength);
	}

	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		return false;
	long double duration = (long double) (end1->t - start1->t);
	*t = start1->t + (long) (duration * fraction);
	return true;
}

/**
 * Returns the single timestamp at which the two temporal point segments 
 * are at the minimum distance (dispatch function). 
 * 
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[out] t Timestamp
 * @pre The segments are not both constants.
 */
bool
tpointseq_min_dist_at_timestamp(const TInstant *start1, const TInstant *end1,
	const TInstant *start2, const TInstant *end2, TimestampTz *t)
{
	double d;
	ensure_point_base_type(start1->valuetypid);
	if (start1->valuetypid == type_oid(T_GEOMETRY))
		return tgeompointseq_min_dist_at_timestamp(start1, end1, start2, end2, t);
	else
		return tgeogpointseq_min_dist_at_timestamp(start1, end1, start2, end2, &d, t);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * Returns the temporal distance between the temporal point and the 
 * geometry/geography point (distpatch function)
 */
Temporal *
distance_tpoint_geo_internal(const Temporal *temp, Datum geo)
{
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp->flags) ? &pt_distance3d :
			&pt_distance2d;
	else
		func = &geog_distance;

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == INSTANT)
		result = (Temporal *)tfunc_tinstant_base((TInstant *)temp,
			geo, temp->valuetypid, (Datum) NULL, (varfunc) func, 2, FLOAT8OID, true);
	else if (temp->duration == INSTANTSET)
		result = (Temporal *)tfunc_tinstantset_base((TInstantSet *)temp,
			geo, temp->valuetypid, (Datum) NULL, (varfunc) func, 2, FLOAT8OID, true);
	else if (temp->duration == SEQUENCE)
		result = (Temporal *)distance_tpointseq_geo((TSequence *)temp,
			geo, func);
	else /* temp->duration == SEQUENCESET */
		result = (Temporal *)distance_tpointseqset_geo((TSequenceSet *)temp,
			geo, func);
	return result;
}

PG_FUNCTION_INFO_V1(distance_geo_tpoint);
/**
 * Returns the temporal distance between the geometry/geography point
 * and the temporal point
 */
PGDLLEXPORT Datum
distance_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tpoint_geo);
/**
 * Returns the temporal distance between the temporal point and the 
 * geometry/geography point
 */
PGDLLEXPORT Datum
distance_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = distance_tpoint_geo_internal(temp, PointerGetDatum(gs));
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal distance between the two temporal points
 * (dispatch function)
 */
Temporal *
distance_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2)
{
	Datum (*func)(Datum, Datum);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &pt_distance3d :
			&pt_distance2d;
	else
		func = &geog_distance;
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, 
		(Datum) NULL, (varfunc) func, 2, FLOAT8OID, linear, false,
		linear ? &tpointseq_min_dist_at_timestamp : NULL);
	return result;
}

PG_FUNCTION_INFO_V1(distance_tpoint_tpoint);
/**
 * Returns the temporal distance between the two temporal points
 */
PGDLLEXPORT Datum
distance_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *result = distance_tpoint_tpoint_internal(temp1, temp2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
	