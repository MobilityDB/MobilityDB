/*****************************************************************************
 *
 * temporalseq.c
 *	  Basic functions for temporal sequences.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporalseq.h"

#include <assert.h>
#include <float.h>
#include <access/hash.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "doublen.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_boxops.h"
#include "rangetypes_ext.h"

#include "tpoint.h"
#include "tpoint_boxops.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_distance.h"

/*****************************************************************************
 * Compute the intersection, if any, of a segment of a temporal sequence and
 * a value. The functions only return true when there is an intersection at
 * the middle of the segment, i.e., they return false if they intersect at a
 * bound. When they return true, they also return in the output parameter
 * the intersection timestampt t. The value taken by the segment and the
 * target value are equal up to the floating point precision.
 * There is no need to add functions for DoubleN, which are used for computing
 * avg and centroid aggregates, since these computations are based on sum and
 * thus they do not need to add intermediate points.
 *****************************************************************************/

/**
 * Returns true if the segment of the temporal number intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value 
 * @param[in] valuetypid Oid of the base type
 * @param[out] t Timestamp 
 */
static bool
tnumberseq_intersection_value(const TemporalInst *inst1, 
	const TemporalInst *inst2, Datum value, Oid valuetypid, TimestampTz *t)
{
	assert(inst1->valuetypid == FLOAT8OID);
	double dvalue1 = DatumGetFloat8(temporalinst_value(inst1));
	double dvalue2 = DatumGetFloat8(temporalinst_value(inst2));
	double dvalue = datum_double(value, valuetypid);
	double min = Min(dvalue1, dvalue2);
	double max = Max(dvalue1, dvalue2);
	/* if value is to the left or to the right of the range */
	if (dvalue < min || dvalue > max)
		return false;

	double range = (max - min);
	double partial = (dvalue - min);
	double fraction = dvalue1 < dvalue2 ? partial / range : 1 - partial / range;
	if (fabs(fraction) < EPSILON || fabs(fraction - 1.0) < EPSILON)
		return false;

	if (t != NULL)
	{
		double duration = (inst2->t - inst1->t);
		*t = inst1->t + (long) (duration * fraction);
	}
	return true;
}

/**
 * Returns true if the segment of the temporal point value intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value 
 * @param[out] t Timestamp 
 */
static bool
tpointseq_intersection_value(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum value, TimestampTz *t)
{
	GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
	if (gserialized_is_empty(gs))
	{
		POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
		return false;
	}

	/* We are sure that the trajectory is a line */
	Datum start = temporalinst_value(inst1);
	Datum end = temporalinst_value(inst2);
	double dist;
	ensure_point_base_type(inst1->valuetypid);
	double fraction = inst1->valuetypid == type_oid(T_GEOMETRY) ?
		geomseg_locate_point(start, end, value, &dist) :
		geogseg_locate_point(start, end, value, &dist);
	if (dist >= EPSILON ||
		(fabs(fraction) < EPSILON || fabs(fraction - 1.0) < EPSILON))
		return false;

	if (t != NULL)
	{
		double duration = (inst2->t - inst1->t);
		*t = inst1->t + (long) (duration * fraction);
	}
	return true;
}

/**
 * Returns true if the segment of the temporal value intersects
 * the base value at the timestamp
 *
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value 
 * @param[in] valuetypid Base type 
 * @param[out] inter Base value taken by the segment at the timestamp.
 * This value is equal to the input base value up to the floating 
 * point precision.
 * @param[out] t Timestamp 
 */
bool
tlinearseq_intersection_value(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum value, Oid valuetypid, Datum *inter, TimestampTz *t)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	if (datum_eq(value, value1, inst1->valuetypid) ||
		datum_eq(value, value2, inst1->valuetypid))
		return false;

	ensure_linear_interpolation(inst1->valuetypid);
	bool result = false; /* make compiler quiet */
	if (inst1->valuetypid == FLOAT8OID)
		result = tnumberseq_intersection_value(inst1, inst2, value,
			valuetypid, t);
	else if (inst1->valuetypid == type_oid(T_GEOMETRY) ||
		inst1->valuetypid == type_oid(T_GEOGRAPHY))
		result = tpointseq_intersection_value(inst1, inst2, value, t);

	if (result && inter != NULL)
		/* We are sure it is linear interpolation */
		*inter = temporalseq_value_at_timestamp1(inst1, inst2, true, *t);
	return result;
}

/*****************************************************************************
 * Compute the intersection, if any, of two segments of temporal sequences.
 * These functions suppose that the instants are synchronized, i.e.,
 * start1->t = start2->t and end1->t = end2->t.
 * The functions return true if there is an intersection at the middle of
 * the segments, i.e., they return false if they intersect at a bound. If
 * they return true, they also return in the output parameter t the
 * intersection timestamp. The two values taken by the segments at the
 * intersection timestamp t are equal up to the floating point precision.
 * For the temporal point case we cannot use the PostGIS functions
 * lw_dist2d_seg_seg and lw_dist3d_seg_seg since they do not take time into
 * consideration and would return, e.g., that the two segments
 * [Point(1 1)@t1, Point(3 3)@t2] and [Point(3 3)@t1, Point(1 1)@t2]
 * intersect at Point(1 1), instead of Point(2 2).
 * These functions are used to add intermediate points when lifting
 * operators, in particular for temporal comparisons such as
 * tfloat <comp> tfloat where <comp> is <, <=, ... since the comparison
 * changes its value before/at/after the intersection point.
 *****************************************************************************/

/**
 * Returns true if the two segments of the temporal numbers
 * intersect at the timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp
 * @pre The instants are synchronized, i.e., start1->t = start2->t and 
 * end1->t = end2->t
 */
static bool
tnumberseq_intersection(const TemporalInst *start1, const TemporalInst *end1,
	const TemporalInst *start2, const TemporalInst *end2, TimestampTz *t)
{
	double x1 = datum_double(temporalinst_value(start1), start1->valuetypid);
	double x2 = datum_double(temporalinst_value(end1), start1->valuetypid);
	double x3 = datum_double(temporalinst_value(start2), start2->valuetypid);
	double x4 = datum_double(temporalinst_value(end2), start2->valuetypid);
	/* Compute the instant t at which the linear functions of the two segments
	   are equal: at + b = ct + d that is t = (d - b) / (a - c).
	   To reduce problems related to floating point arithmetic, t1 and t2
	   are shifted, respectively, to 0 and 1 before the computation */
	long double denum = x2 - x1 - x4 + x3;
	if (denum == 0)
		/* Parallel segments */
		return false;

	long double fraction = ((long double) (x3 - x1)) / denum;
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		/* Intersection occurs out of the period */
		return false;

	double duration = (end1->t - start1->t);
	*t = start1->t + (long) (duration * fraction);
	return true;
}

/**
 * Returns true if the two segments of the temporal geometric point
 * values intersect at the timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp 
 * @pre The instants are synchronized, i.e., start1->t = start2->t and 
 * end1->t = end2->t
 */
bool
tgeompointseq_intersection(const TemporalInst *start1, const TemporalInst *end1,
	const TemporalInst *start2, const TemporalInst *end2, TimestampTz *t)
{
	long double fraction, xfraction = 0, yfraction = 0, xdenum, ydenum;
	if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
	{
		long double zfraction = 0, zdenum;
		const POINT3DZ *p1 = datum_get_point3dz_p(temporalinst_value(start1));
		const POINT3DZ *p2 = datum_get_point3dz_p(temporalinst_value(end1));
		const POINT3DZ *p3 = datum_get_point3dz_p(temporalinst_value(start2));
		const POINT3DZ *p4 = datum_get_point3dz_p(temporalinst_value(end2));
		xdenum = p2->x - p1->x - p4->x + p3->x;
		ydenum = p2->y - p1->y - p4->y + p3->y;
		zdenum = p2->z - p1->z - p4->z + p3->z;
		if (xdenum == 0 && ydenum == 0 && zdenum == 0)
			/* Parallel segments */
			return false;

		if (xdenum != 0)
		{
			xfraction = (p3->x - p1->x) / xdenum;
			/* If intersection occurs out of the period */
			if (xfraction <= EPSILON || xfraction >= (1.0 - EPSILON))
				return false;
		}
		if (ydenum != 0)
		{
			yfraction = (p3->y - p1->y) / ydenum;
			/* If intersection occurs out of the period */
			if (yfraction <= EPSILON || yfraction >= (1.0 - EPSILON))
				return false;
		}
		if (zdenum != 0)
		{
			/* If intersection occurs out of the period or intersect 
			 * at different timestamps */
			zfraction = (p3->z - p1->z) / zdenum;
			if (zfraction <= EPSILON || zfraction >= (1.0 - EPSILON))
				return false;
		}
		/* If intersect at different timestamps on each dimension */
		if ((xdenum != 0 && ydenum != 0 && zdenum != 0 &&
			fabsl(xfraction - yfraction) > EPSILON && 
			fabsl(xfraction - zfraction) > EPSILON) ||
			(xdenum == 0 && ydenum != 0 && zdenum != 0 &&
			fabsl(yfraction - zfraction) > EPSILON) ||
			(xdenum != 0 && ydenum == 0 && zdenum != 0 &&
			fabsl(xfraction - zfraction) > EPSILON) ||
			(xdenum != 0 && ydenum != 0 && zdenum == 0 &&
			fabsl(xfraction - yfraction) > EPSILON))
			return false;
		if (xdenum != 0)
			fraction = xfraction;
		else if (ydenum != 0)
			fraction = yfraction;
		else
			fraction = zfraction;
	}
	else /* 2D */
	{
		const POINT2D *p1 = datum_get_point2d_p(temporalinst_value(start1));
		const POINT2D *p2 = datum_get_point2d_p(temporalinst_value(end1));
		const POINT2D *p3 = datum_get_point2d_p(temporalinst_value(start2));
		const POINT2D *p4 = datum_get_point2d_p(temporalinst_value(end2));
		xdenum = p2->x - p1->x - p4->x + p3->x;
		ydenum = p2->y - p1->y - p4->y + p3->y;
		if (xdenum == 0 && ydenum == 0)
			/* Parallel segments */
			return false;

		if (xdenum != 0)
		{
			xfraction = (p3->x - p1->x) / xdenum;
			/* If intersection occurs out of the period */
			if (xfraction <= EPSILON || xfraction >= (1.0 - EPSILON))
				return false;
		}
		if (ydenum != 0)
		{
			yfraction = (p3->y - p1->y) / ydenum;
			/* If intersection occurs out of the period */
			if (yfraction <= EPSILON || yfraction >= (1.0 - EPSILON))
				return false;
		}
		/* If intersect at different timestamps on each dimension */
		if (xdenum != 0 && ydenum != 0 && fabsl(xfraction - yfraction) > EPSILON)
			return false;
		fraction = xdenum != 0 ? xfraction : yfraction;
	}
	double duration = (end1->t - start1->t);
	*t = start1->t + (long) (duration * fraction);
	return true;
}

/**
 * Returns true if the two segments of the temporal geographic point
 * values intersect at the timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[out] t Timestamp 
 * @pre The instants are synchronized, i.e., start1->t = start2->t and 
 * end1->t = end2->t
 */
bool
tgeogpointseq_intersection(const TemporalInst *start1, const TemporalInst *end1,
	const TemporalInst *start2, const TemporalInst *end2, TimestampTz *t)
{
	GEOGRAPHIC_EDGE e1, e2;
	POINT3D A1, A2, B1, B2;
	double fraction,
		xfraction = 0, yfraction = 0, zfraction = 0,
		xdenum, ydenum, zdenum;

	POINT4D p1 = datum_get_point4d(temporalinst_value(start1));
	geographic_point_init(p1.x, p1.y, &(e1.start));
	geog2cart(&(e1.start), &A1);

	POINT4D p2 = datum_get_point4d(temporalinst_value(end1));
	geographic_point_init(p2.x, p2.y, &(e1.end));
	geog2cart(&(e1.end), &A2);

	POINT4D p3 = datum_get_point4d(temporalinst_value(start2));
	geographic_point_init(p3.x, p3.y, &(e2.start));
	geog2cart(&(e2.start), &B1);

	POINT4D p4 = datum_get_point4d(temporalinst_value(end2));
	geographic_point_init(p4.x, p4.y, &(e2.end));
	geog2cart(&(e2.end), &B2);

	uint32_t inter = edge_intersects(&A1, &A2, &B1, &B2);
	if (inter == PIR_NO_INTERACT)
		return false;

	xdenum = A2.x - A1.x - B2.x + B1.x;
	ydenum = A2.y - A1.y - B2.y + B1.y;
	zdenum = A2.z - A1.z - B2.z + B1.z;
	if (xdenum == 0 && ydenum == 0 && zdenum == 0)
		/* Parallel segments */
		return false;

	if (xdenum != 0)
	{
		xfraction = (B1.x - A1.x) / xdenum;
		/* If intersection occurs out of the period */
		if (xfraction <= EPSILON || xfraction >= (1.0 - EPSILON))
			return false;
	}
	if (ydenum != 0)
	{
		yfraction = (B1.y - A1.y) / ydenum;
		/* If intersection occurs out of the period */
		if (yfraction <= EPSILON || yfraction >= (1.0 - EPSILON))
			return false;
	}
	if (zdenum != 0)
	{
		/* If intersection occurs out of the period or intersect at different timestamps */
		zfraction = (B1.z - A1.z) / zdenum;
		if (zfraction <= EPSILON || zfraction >= (1.0 - EPSILON))
			return false;
	}
	/* If intersect at different timestamps on each dimension
	 * We average the fractions found to limit floating point imprecision */
	if (xdenum != 0 && ydenum != 0 && zdenum != 0 &&
		fabsl(xfraction - yfraction) <= EPSILON && fabsl(xfraction - zfraction) <= EPSILON)
		fraction = (xfraction + yfraction + zfraction) / 3.0;
	else if (xdenum == 0 && ydenum != 0 && zdenum != 0 &&
		fabsl(yfraction - zfraction) <= EPSILON)
		fraction = (yfraction + zfraction) / 2.0;
	else if (xdenum != 0 && ydenum == 0 && zdenum != 0 &&
		fabsl(xfraction - zfraction) <= EPSILON)
		fraction = (xfraction + zfraction) / 2.0;
	else if (xdenum != 0 && ydenum != 0 && zdenum == 0 &&
		fabsl(xfraction - yfraction) <= EPSILON)
		fraction = (xfraction + yfraction) / 2.0;
	else if (xdenum != 0)
		fraction = xfraction;
	else if (ydenum != 0)
		fraction = yfraction;
	else if (zdenum != 0)
		fraction = zfraction;
	else
		return false;

	long double duration = (end1->t - start1->t);
	*t = start1->t + (long) (duration * fraction);
	return true;
}

/**
 * Returns true if the two segments of the temporal values
 * intersect at the timestamp
 *
 * @param[in] start1,end1 Temporal instants defining the first segment
 * @param[in] linear1 True when the interpolation of the first segment
 * is linear
 * @param[in] start2,end2 Temporal instants defining the second segment
 * @param[in] linear2 True when the interpolation of the second segment
 * is linear
 * @param[out] inter1, inter2 Base values taken by the two segments 
 * at the timestamp
 * @param[out] t Timestamp 
 * @pre The instants are synchronized, i.e., start1->t = start2->t and 
 * end1->t = end2->t
 */
bool
temporalseq_intersection(const TemporalInst *start1, const TemporalInst *end1, bool linear1,
	const TemporalInst *start2, const TemporalInst *end2, bool linear2,
	Datum *inter1, Datum *inter2, TimestampTz *t)
{
	bool result = false; /* Make compiler quiet */
	if (! linear1)
	{
		*inter1 = temporalinst_value(start1);
		result = tlinearseq_intersection_value(start2, end2,
			*inter1, start1->valuetypid, inter2, t);
	}
	else if (! linear2)
	{
		*inter2 = temporalinst_value(start2);
		result = tlinearseq_intersection_value(start1, end1,
			*inter2, start2->valuetypid, inter1, t);
	}
	else
	{
		/* Both segments have linear interpolation */
		ensure_temporal_base_type(start1->valuetypid);
		if ((start1->valuetypid == INT4OID || start1->valuetypid == FLOAT8OID) &&
			(start2->valuetypid == INT4OID || start2->valuetypid == FLOAT8OID))
			result = tnumberseq_intersection(start1, end1, start2, end2, t);
		else if (start1->valuetypid == type_oid(T_GEOMETRY))
			result = tgeompointseq_intersection(start1, end1, start2, end2, t);
		else if (start1->valuetypid == type_oid(T_GEOGRAPHY))
			result = tgeogpointseq_intersection(start1, end1, start2, end2, t);
		/* We are sure it is linear interpolation */
		if (result && inter1 != NULL)
			*inter1 = temporalseq_value_at_timestamp1(start1, end1, true, *t);
		if (result && inter2 != NULL)
			*inter2 = temporalseq_value_at_timestamp1(start2, end2, true, *t);
	}
	return result;
}

/*****************************************************************************
 * Are the three temporal instant values collinear?
 * These functions suppose that the segments are not constant.
 *****************************************************************************/

/**
 * Returns true if the three values are collinear
 * 
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
float_collinear(double x1, double x2, double x3, double ratio)
{
	double x = x1 + (x3 - x1) * ratio;
	return (fabs(x2 - x) <= EPSILON);
}

/**
 * Returns true if the three double2 values are collinear
 *
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double2_collinear(const double2 *x1, const double2 *x2, const double2 *x3,
	double ratio)
{
	double2 x;
	x.a = x1->a + (x3->a - x1->a) * ratio;
	x.b = x1->b + (x3->b - x1->b) * ratio;
	bool result = (fabs(x2->a - x.a) <= EPSILON && fabs(x2->b - x.b) <= EPSILON);
	return result;
}

/**
 * Returns true if the three values are collinear
 *
 * @param[in] value1,value2,value3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `value1` and `value2` divided by the duration
 * of the timestamps associated to `value1` and `value3`
 * @param[in] hasz True when the points have Z coordinates
 */
static bool
geompoint_collinear(Datum value1, Datum value2, Datum value3,
	double ratio, bool hasz)
{
	POINT4D p1 = datum_get_point4d(value1);
	POINT4D p2 = datum_get_point4d(value2);
	POINT4D p3 = datum_get_point4d(value3);
	POINT4D p;
	interpolate_point4d(&p1, &p3, &p, ratio);
	bool result = hasz ?
		fabs(p2.x - p.x) <= EPSILON && fabs(p2.y - p.y) <= EPSILON &&
			fabs(p2.z - p.z) <= EPSILON :
		fabs(p2.x - p.x) <= EPSILON && fabs(p2.y - p.y) <= EPSILON;
	return result;
}

/**
 * Returns true if the three values are collinear
 *
 * @param[in] value1,value2,value3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `x1` and `x2` divided by the duration 
 * of the timestamps associated to `x1` and `x3`
 * @param[in] hasz True when the points have Z coordinates
 */
static bool
geogpoint_collinear(Datum value1, Datum value2, Datum value3,
	double ratio, bool hasz)
{
	Datum value = geogseg_interpolate_point(value1, value3, ratio);
	POINT4D p2 = datum_get_point4d(value2);
	POINT4D p = datum_get_point4d(value);
	bool result = hasz ?
		fabs(p2.x - p.x) <= EPSILON && fabs(p2.y - p.y) <= EPSILON &&
			fabs(p2.z - p.z) <= EPSILON :
		fabs(p2.x - p.x) <= EPSILON && fabs(p2.y - p.y) <= EPSILON;
	return result;
}

/**
 * Returns true if the three values are collinear
 *
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double3_collinear(const double3 *x1, const double3 *x2, const double3 *x3,
	double ratio)
{
	double3 x;
	x.a = x1->a + (x3->a - x1->a) * ratio;
	x.b = x1->b + (x3->b - x1->b) * ratio,
	x.c = x1->c + (x3->c - x1->c) * ratio;
	bool result = (fabs(x2->a - x.a) <= EPSILON && 
		fabs(x2->b - x.b) <= EPSILON && fabs(x2->c - x.c) <= EPSILON);
	return result;
}

/**
 * Returns true if the three values are collinear
 *
 * @param[in] x1,x2,x3 Input values
 * @param[in] ratio Value in [0,1] representing the duration of the 
 * timestamps associated to `x1` and `x2` divided by the duration
 * of the timestamps associated to `x1` and `x3`
 */
static bool
double4_collinear(const double4 *x1, const double4 *x2, const double4 *x3,
	double ratio)
{
	double4 x;
	x.a = x1->a + (x3->a - x1->a) * ratio;
	x.b = x1->b + (x3->b - x1->b) * ratio;
	x.c = x1->c + (x3->c - x1->c) * ratio;
	x.d = x1->d + (x3->d - x1->d) * ratio;
	bool result = (fabs(x2->a - x.a) <= EPSILON && fabs(x2->b - x.b) <= EPSILON &&
		fabs(x2->c - x.c) <= EPSILON && fabs(x2->d - x.d) <= EPSILON);
	return result;
}

/**
 * Returns true if the three values are collinear
 *
 * @param[in] valuetypid Oid of the base type
 * @param[in] value1,value2,value3 Input values
 * @param[in] t1,t2,t3 Input timestamps
 */
static bool
datum_collinear(Oid valuetypid, Datum value1, Datum value2, Datum value3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	double duration1 = (double) (t2 - t1);
	double duration2 = (double) (t3 - t1);
	double ratio = duration1 / duration2;
	if (valuetypid == FLOAT8OID)
		return float_collinear(DatumGetFloat8(value1), DatumGetFloat8(value2), 
			DatumGetFloat8(value3), ratio);
	if (valuetypid == type_oid(T_DOUBLE2))
		return double2_collinear(DatumGetDouble2P(value1), DatumGetDouble2P(value2), 
			DatumGetDouble2P(value3), ratio);
	if (valuetypid == type_oid(T_GEOMETRY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value1);
		bool hasz = (bool) FLAGS_GET_Z(gs->flags);
		return geompoint_collinear(value1, value2, value3, ratio, hasz);
	}
	if (valuetypid == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value1);
		bool hasz = (bool) FLAGS_GET_Z(gs->flags);
		return geogpoint_collinear(value1, value2, value3, ratio, hasz);
	}
	if (valuetypid == type_oid(T_DOUBLE3))
		return double3_collinear(DatumGetDouble3P(value1), DatumGetDouble3P(value2), 
			DatumGetDouble3P(value3), ratio);
	if (valuetypid == type_oid(T_DOUBLE4))
		return double4_collinear(DatumGetDouble4P(value1), DatumGetDouble4P(value2), 
			DatumGetDouble4P(value3), ratio);
	return false;
}

/*****************************************************************************
 * Normalization functions
 *****************************************************************************/

/**
 * Normalize the array of temporal instant values
 *
 * @param[in] instants Array of input instants
 * @param[in] linear True when the instants have linear interpolation
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @result Array of normalized temporal instant values
 * @pre The input array has at least two elements
 * @note The function does not create new instants, it creates an array of 
 * pointers to a subset of the input instants 
 */
static TemporalInst **
temporalinstarr_normalize(TemporalInst **instants, bool linear, int count, 
	int *newcount)
{
	assert(count > 1);
	Oid valuetypid = instants[0]->valuetypid;
	TemporalInst **result = palloc(sizeof(TemporalInst *) * count);
	/* Remove redundant instants */ 
	TemporalInst *inst1 = instants[0];
	Datum value1 = temporalinst_value(inst1);
	TemporalInst *inst2 = instants[1];
	Datum value2 = temporalinst_value(inst2);
	result[0] = inst1;
	int k = 1;
	for (int i = 2; i < count; i++)
	{
		TemporalInst *inst3 = instants[i];
		Datum value3 = temporalinst_value(inst3);
		if (
			/* step sequences and 2 consecutive instants that have the same value
				... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
			*/
			(!linear && datum_eq(value1, value2, valuetypid))
			||
			/* 3 consecutive linear instants that have the same value
				... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
			*/
			(linear && datum_eq(value1, value2, valuetypid) && datum_eq(value2, value3, valuetypid))
			||
			/* collinear linear instants
				... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
			*/
			(linear && datum_collinear(valuetypid, value1, value2, value3, inst1->t, inst2->t, inst3->t))
			)
		{
			inst2 = inst3; value2 = value3;
		} 
		else 
		{
			result[k++] = inst2;
			inst1 = inst2; value1 = value2;
			inst2 = inst3; value2 = value3;
		}
	}
	result[k++] = inst2;
	*newcount = k;
	return result;
}

/**
 * Normalize the array of temporal sequence values
 *
 * The inpuy sequences may be non-contiguous but must ordered and
 * either (1) are non-overlapping, or (2) share the same last/first
 * instant in the case we are merging temporal sequences. 
 *
 * @param[in] sequences Array of input sequences
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @result Array of normalized temporal sequences values
 * @pre Each sequence in the input array is normalized
 * @pre When merging sequences, the test whether the value is the same 
 * at the common instant should be ensured by the calling function.
 * @note The function creates new sequences and does not free the original
 * sequences
 */
TemporalSeq **
temporalseqarr_normalize(TemporalSeq **sequences, int count, int *newcount)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count);
	/* seq1 is the sequence to which we try to join subsequent seq2 */
	TemporalSeq *seq1 = sequences[0];
	Oid valuetypid = seq1->valuetypid;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool isnew = false;
	int k = 0;
	for (int i = 1; i < count; i++)
	{
		TemporalSeq *seq2 = sequences[i];
		TemporalInst *last2 = (seq1->count == 1) ? NULL : 
			temporalseq_inst_n(seq1, seq1->count - 2); 
		Datum last2value = (seq1->count == 1) ? 0 : 
			temporalinst_value(last2);
		TemporalInst *last1 = temporalseq_inst_n(seq1, seq1->count - 1);
		Datum last1value = temporalinst_value(last1);
		TemporalInst *first1 = temporalseq_inst_n(seq2, 0);
		Datum first1value = temporalinst_value(first1);
		TemporalInst *first2 = (seq2->count == 1) ? NULL : 
			temporalseq_inst_n(seq2, 1); 
		Datum first2value = (seq2->count == 1) ? 0 : 
			temporalinst_value(first2);
		bool adjacent = seq1->period.upper == seq2->period.lower &&
			(seq1->period.upper_inc || seq2->period.lower_inc);
		/* If they are adjacent and not instantaneous */
		if (adjacent && last2 != NULL && first2 != NULL &&
			(
			/* If step and the last segment of the first sequence is constant
			   ..., 1@t1, 1@t2) [1@t2, 1@t3, ... -> ..., 1@t1, 2@t3, ... 
			   ..., 1@t1, 1@t2) [1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ... 
			   ..., 1@t1, 1@t2] (1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ... 
			 */
			(!linear && 
			datum_eq(last2value, last1value, valuetypid) && 
			datum_eq(last1value, first1value, valuetypid))
			||			
			/* If the last/first segments are constant and equal 
			   ..., 1@t1, 1@t2] (1@t2, 1@t3, ... -> ..., 1@t1, 1@t3, ... 
			 */
			(datum_eq(last2value, last1value, valuetypid) &&
			datum_eq(last1value, first1value, valuetypid) && 
			datum_eq(first1value, first2value, valuetypid))
			||			
			/* If float/point sequences and collinear last/first segments having the same duration 
			   ..., 1@t1, 2@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 3@t3, ... 
			*/
			(datum_eq(last1value, first1value, valuetypid) && 
			datum_collinear(valuetypid, last2value, first1value, first2value,
				last2->t, first1->t, first2->t))
			))
		{
			/* Remove the last and first instants of the sequences */
			seq1 = temporalseq_join(seq1, seq2, true, true);
			isnew = true;
		}
		/* If step sequences and the first one has an exclusive upper bound,
		   by definition the first sequence has the last segment constant
		   ..., 1@t1, 1@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 2@t2, 3@t3, ... 
		   ..., 1@t1, 1@t2) [2@t2] -> ..., 1@t1, 2@t2]
		 */
		else if (adjacent && !linear && !seq1->period.upper_inc)
		{
			/* Remove the last instant of the first sequence */
			seq1 = temporalseq_join(seq1, seq2, true, false);
			isnew = true;
		}
		/* If they are adjacent and have equal last/first value respectively 
			Stewise
			... 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			[1@t1], (1@t1, 2@t2, ... -> ..., 1@t1, 2@t2
			Linear	
			..., 1@t1, 2@t2), [2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			..., 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			..., 1@t1, 2@t2), [2@t2] -> ..., 1@t1, 2@t2]
			[1@t1],(1@t1, 2@t2, ... -> [1@t1, 2@t2, ...
		*/
		else if (adjacent && datum_eq(last1value, first1value, valuetypid))
		{
			/* Remove the first instant of the second sequence */
			seq1 = temporalseq_join(seq1, seq2, false, true);
			isnew = true;
		} 
		else 
		{
			result[k++] = isnew ? seq1 : temporalseq_copy(seq1);
			seq1 = seq2;
			isnew = false;
		}
	}
	result[k++] = isnew ? seq1 : temporalseq_copy(seq1);
	*newcount = k;
	return result;
}

/*****************************************************************************/

/**
 * Returns the n-th instant of the temporal value
 */
TemporalInst *
temporalseq_inst_n(const TemporalSeq *seq, int index)
{
	return (TemporalInst *)(
		(char *)(&seq->offsets[seq->count + 2]) + 	/* start of data */
			seq->offsets[index]);					/* offset */
}

/**
 * Returns a pointer to the precomputed bounding box of the temporal value
 */
void *
temporalseq_bbox_ptr(const TemporalSeq *seq)
{
	return (char *)(&seq->offsets[seq->count + 2]) +  	/* start of data */
		seq->offsets[seq->count];						/* offset */
}

/**
 * Copy in the first argument the bounding box of the temporal value
 */
void
temporalseq_bbox(void *box, const TemporalSeq *seq)
{
	void *box1 = temporalseq_bbox_ptr(seq);
	size_t bboxsize = temporal_bbox_size(seq->valuetypid);
	memcpy(box, box1, bboxsize);
}

/**
 * Construct a temporal sequence value from the array of temporal
 * instant values
 *
 * For example, the memory structure of a temporal sequence value with
 * 2 instants and a precomputed trajectory is as follows:
 * @code
 * -------------------------------------------------------------------
 * ( TemporalSeq )_X | offset_0 | offset_1 | offset_2 | offset_3 | ...
 * -------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * ( TemporalInst_0 )_X | ( TemporalInst_1 )_X | ( bbox )_X | ( Traj )_X  |
 * ------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding instants, `offset_2` is the
 * offset for the bounding box and `offset_3` is the offset for the 
 * precomputed trajectory. Precomputed trajectories are only kept for temporal
 * points of sequence duration.
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 * @param[in] lower_inc,upper_inc True when the respective bound is inclusive
 * @param[in] linear True when the interpolation is linear
 * @param[in] normalize True when the resulting value should be normalized
 */
TemporalSeq *
temporalseq_make(TemporalInst **instants, int count, bool lower_inc,
   bool upper_inc, bool linear, bool normalize)
{
	/* Test the validity of the instants */
	assert(count > 0);
	bool isgeo = (instants[0]->valuetypid == type_oid(T_GEOMETRY) ||
		instants[0]->valuetypid == type_oid(T_GEOGRAPHY));
	ensure_valid_temporalinstarr(instants, count, isgeo);
	if (count == 1 && (!lower_inc || !upper_inc))
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
				errmsg("Instant sequence must have inclusive bounds")));
	if (!linear && count > 1 && !upper_inc &&
		datum_ne(temporalinst_value(instants[count - 1]), 
			temporalinst_value(instants[count - 2]), instants[0]->valuetypid))
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("Invalid end value for temporal sequence")));

	/* Normalize the array of instants */
	TemporalInst **newinstants = instants;
	int newcount = count;
	if (normalize && count > 2)
		newinstants = temporalinstarr_normalize(instants, linear, count, &newcount);
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(instants[0]->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < newcount; i++)
		memsize += double_pad(VARSIZE(newinstants[i]));
	/* Precompute the trajectory */
	bool trajectory = false; /* keep compiler quiet */
	Datum traj = 0; /* keep compiler quiet */
	if (isgeo)
	{
		trajectory = type_has_precomputed_trajectory(instants[0]->valuetypid);
		if (trajectory)
		{
			/* A trajectory is a geometry/geography, a point, a multipoint, 
			 * or a linestring, which may be self-intersecting */
			traj = tpointseq_make_trajectory(newinstants, newcount, linear);
			memsize += double_pad(VARSIZE(DatumGetPointer(traj)));
		}
	}
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalSeq)) + (newcount + 1) * sizeof(size_t);
	/* Create the TemporalSeq */
	TemporalSeq *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->valuetypid = instants[0]->valuetypid;
	result->duration = TEMPORALSEQ;
	period_set(&result->period, newinstants[0]->t, newinstants[newcount - 1]->t,
		lower_inc, upper_inc);
	MOBDB_FLAGS_SET_LINEAR(result->flags, linear);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(instants[0]->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(instants[0]->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *)result) + pdata + pos, newinstants[i], 
			VARSIZE(newinstants[i]));
		result->offsets[i] = pos;
		pos += double_pad(VARSIZE(newinstants[i]));
	}
	/*
	 * Precompute the bounding box 
	 * Only external types have precomputed bounding box, internal types such
	 * as double2, double3, or double4 do not have precomputed bounding box.
	 * For temporal points the bounding box is computed from the trajectory 
	 * for efficiency reasons.
	 */
	if (bboxsize != 0)
	{
		void *bbox = ((char *) result) + pdata + pos;
		if (trajectory)
		{
			geo_to_stbox_internal(bbox, (GSERIALIZED *)DatumGetPointer(traj));
			((STBOX *)bbox)->tmin = result->period.lower;
			((STBOX *)bbox)->tmax = result->period.upper;
			MOBDB_FLAGS_SET_T(((STBOX *)bbox)->flags, true);
		}
		else
			temporalseq_make_bbox(bbox, newinstants, newcount,
				lower_inc, upper_inc);
		result->offsets[newcount] = pos;
		pos += double_pad(bboxsize);
	}
	if (isgeo && trajectory)
	{
		result->offsets[newcount + 1] = pos;
		memcpy(((char *) result) + pdata + pos, DatumGetPointer(traj),
			VARSIZE(DatumGetPointer(traj)));
		pfree(DatumGetPointer(traj));
	}

	if (normalize && count > 2)
		pfree(newinstants);

	return result;
}

/**
 * Construct a temporal sequence value from the array of temporal
 * instant values and free the array and the instants after the creation
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 */
TemporalSeq *
temporalseq_make_free(TemporalInst **instants, int count, bool lower_inc,
   bool upper_inc, bool linear, bool normalize)
{
	if (count == 0)
	{
		pfree(instants);
		return NULL;
	}
	TemporalSeq *result = temporalseq_make(instants, count, lower_inc, upper_inc,
		linear, normalize);
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/**
 * Join the two temporal sequence values
 *
 * @param[in] seq1,seq2 Temporal sequence values
 * @param[in] removelast,removefirst Remove the last and/or the 
 * first instant of the first/second sequence
 * @pre The two input sequences are adjacent and have the same interpolation
 * @note The function is called when normalizing an array of sequences 
 */
TemporalSeq *
temporalseq_join(const TemporalSeq *seq1, const TemporalSeq *seq2,
	bool removelast, bool removefirst)
{
	/* Ensure that the two sequences has the same interpolation */
	assert(MOBDB_FLAGS_GET_LINEAR(seq1->flags) == 
		MOBDB_FLAGS_GET_LINEAR(seq2->flags));
	Oid valuetypid = seq1->valuetypid;

	size_t bboxsize = temporal_bbox_size(valuetypid);
	size_t memsize = double_pad(bboxsize);

	int count1 = removelast ? seq1->count - 1 : seq1->count;
	int start2 = removefirst ? 1 : 0;
	for (int i = 0; i < count1; i++)
		memsize += double_pad(VARSIZE(temporalseq_inst_n(seq1, i)));
	for (int i = start2; i < seq2->count; i++)
		memsize += double_pad(VARSIZE(temporalseq_inst_n(seq2, i)));

	int count = count1 + (seq2->count - start2);

	bool trajectory = type_has_precomputed_trajectory(valuetypid);
	Datum traj = 0; /* keep compiler quiet */
	if (trajectory)
	{
		/* A trajectory is a geometry/geography, either a point or a
		 * linestring, which may be self-intersecting */
		traj = tpointseq_trajectory_join(seq1, seq2, removelast, removefirst);
		memsize += double_pad(VARSIZE(DatumGetPointer(traj)));
	}

	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalSeq)) + (count + 1) * sizeof(size_t);
	/* Create the TemporalSeq */
	TemporalSeq *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = count;
	result->valuetypid = valuetypid;
	result->duration = TEMPORALSEQ;
	period_set(&result->period, seq1->period.lower, seq2->period.upper,
		seq1->period.lower_inc, seq2->period.upper_inc);
	MOBDB_FLAGS_SET_LINEAR(result->flags, MOBDB_FLAGS_GET_LINEAR(seq1->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY))
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(seq1->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(seq1->flags));
	}

	/* Initialization of the variable-length part */
	int k = 0;
	size_t pos = 0;
	for (int i = 0; i < count1; i++)
	{
		memcpy(((char *)result) + pdata + pos, temporalseq_inst_n(seq1, i),
			VARSIZE(temporalseq_inst_n(seq1, i)));
		result->offsets[k++] = pos;
		pos += double_pad(VARSIZE(temporalseq_inst_n(seq1, i)));
	}
	for (int i = start2; i < seq2->count; i++)
	{
		memcpy(((char *)result) + pdata + pos, temporalseq_inst_n(seq2, i),
			VARSIZE(temporalseq_inst_n(seq2, i)));
		result->offsets[k++] = pos;
		pos += double_pad(VARSIZE(temporalseq_inst_n(seq2, i)));
	}
	/*
	 * Precompute the bounding box
	 */
	if (bboxsize != 0)
	{
		void *bbox = ((char *) result) + pdata + pos;
		if (valuetypid == BOOLOID || valuetypid == TEXTOID)
			memcpy(bbox, &result->period, bboxsize);
		else
		{
			memcpy(bbox, temporalseq_bbox_ptr(seq1), bboxsize);
			temporal_bbox_expand(bbox, temporalseq_bbox_ptr(seq2), valuetypid);
		}
		result->offsets[k] = pos;
		pos += double_pad(bboxsize);
	}
	if (trajectory)
	{
		result->offsets[k + 1] = pos;
		memcpy(((char *) result) + pdata + pos, DatumGetPointer(traj),
			VARSIZE(DatumGetPointer(traj)));
		pfree(DatumGetPointer(traj));
	}

	return result;
}

/**
 * Construct a temporal sequence value from a base value and a period
 * (internal function)
 *
 * @param[in] value Base value
 * @param[in] valuetypid Oid of the base type
 * @param[in] p Period
 * @param[in] linear True when the resulting value has linear interpolation
 */
TemporalSeq *
temporalseq_from_base_internal(Datum value, Oid valuetypid, const Period *p, bool linear)
{
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(value, p->lower, valuetypid);
	instants[1] = temporalinst_make(value, p->upper, valuetypid);
	TemporalSeq *result = temporalseq_make(instants, 2, p->lower_inc,
		p->upper_inc, linear, false);
	pfree(instants[0]); pfree(instants[1]);
	return result;
}

PG_FUNCTION_INFO_V1(temporalseq_from_base);
/**
 * Construct a temporal sequence value from a base value and a period
 */
PGDLLEXPORT Datum
temporalseq_from_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Period *p = PG_GETARG_PERIOD(1);
	bool linear = PG_GETARG_BOOL(2);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TemporalSeq *result = temporalseq_from_base_internal(value, valuetypid, p, linear);
	DATUM_FREE_IF_COPY(value, valuetypid, 0);
	PG_RETURN_POINTER(result);
}

/**
 * Append an instant to the temporal value
 */
TemporalSeq *
temporalseq_append_instant(const TemporalSeq *seq, const TemporalInst *inst)
{
	/* Test the validity of the instant */
	assert(seq->valuetypid == inst->valuetypid);
	assert(MOBDB_FLAGS_GET_GEODETIC(seq->flags) == MOBDB_FLAGS_GET_GEODETIC(inst->flags));
	TemporalInst *inst1 = temporalseq_inst_n(seq, seq->count - 1);
	ensure_increasing_timestamps(inst1, inst);
	bool isgeo = (seq->valuetypid == type_oid(T_GEOMETRY) ||
		seq->valuetypid == type_oid(T_GEOGRAPHY));
	if (isgeo)
	{
		ensure_same_srid_tpoint((Temporal *)seq, (Temporal *)inst);
		ensure_same_dimensionality_tpoint((Temporal *)seq, (Temporal *)inst);
	}

	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	/* Normalize the result */
	int newcount = seq->count + 1;
	if (seq->count > 1)
	{
		inst1 = temporalseq_inst_n(seq, seq->count - 2);
		Datum value1 = temporalinst_value(inst1);
		TemporalInst *inst2 = temporalseq_inst_n(seq, seq->count - 1);
		Datum value2 = temporalinst_value(inst2);
		Datum value3 = temporalinst_value(inst);
		if (
			/* step sequences and 2 consecutive instants that have the same value
				... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
			*/
			(! linear && datum_eq(value1, value2, seq->valuetypid))
			||
			/* 3 consecutive float/point instants that have the same value 
				... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
			*/
			(datum_eq(value1, value2, seq->valuetypid) && datum_eq(value2, value3, seq->valuetypid))
			||
			/* collinear float/point instants that have the same duration
				... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
			*/
			(linear && datum_collinear(seq->valuetypid, value1, value2, value3, inst1->t, inst2->t, inst->t))
			)
		{
			/* The new instant replaces the last instant of the sequence */
			newcount--;
		} 
	}
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(seq->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < newcount - 1; i++)
		memsize += double_pad(VARSIZE(temporalseq_inst_n(seq, i)));
	memsize += double_pad(VARSIZE(inst));
	/* Expand the trajectory */
	bool trajectory = false; /* keep compiler quiet */
	Datum traj = 0; /* keep compiler quiet */
	if (isgeo)
	{
		trajectory = type_has_precomputed_trajectory(seq->valuetypid);
		if (trajectory)
		{
			bool replace = newcount != seq->count + 1;
			traj = tpointseq_trajectory_append(seq, inst, replace);
			memsize += double_pad(VARSIZE(DatumGetPointer(traj)));
		}
	}
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalSeq)) + (newcount + 1) * sizeof(size_t);
	/* Create the TemporalSeq */
	TemporalSeq *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->valuetypid = seq->valuetypid;
	result->duration = TEMPORALSEQ;
	period_set(&result->period, seq->period.lower, inst->t, 
		seq->period.lower_inc, true);
	MOBDB_FLAGS_SET_LINEAR(result->flags, MOBDB_FLAGS_GET_LINEAR(seq->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(seq->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(seq->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < newcount - 1; i++)
	{
		inst1 = temporalseq_inst_n(seq, i);
		memcpy(((char *)result) + pdata + pos, inst1, VARSIZE(inst1));
		result->offsets[i] = pos;
		pos += double_pad(VARSIZE(inst1));
	}
	/* Append the instant */
	memcpy(((char *)result) + pdata + pos, inst, VARSIZE(inst));
	result->offsets[newcount - 1] = pos;
	pos += double_pad(VARSIZE(inst));
	/* Expand the bounding box */
	if (bboxsize != 0) 
	{
		union bboxunion box;
		void *bbox = ((char *) result) + pdata + pos;
		memcpy(bbox, temporalseq_bbox_ptr(seq), bboxsize);
		temporalinst_make_bbox(&box, inst);
		temporal_bbox_expand(bbox, &box, seq->valuetypid);
		result->offsets[newcount] = pos;
	}
	if (isgeo && trajectory)
	{
		result->offsets[newcount + 1] = pos;
		memcpy(((char *) result) + pdata + pos, DatumGetPointer(traj),
			VARSIZE(DatumGetPointer(traj)));
		pfree(DatumGetPointer(traj));
	}
	return result;
}

/**
 * Merge the two temporal values
 */
Temporal *
temporalseq_merge(const TemporalSeq *seq1, const TemporalSeq *seq2)
{
	const TemporalSeq *sequences[] = {seq1, seq2};
	return temporalseq_merge_array((TemporalSeq **) sequences, 2);
}

/**
 * Merge the array of temporal sequence values
 *
 * @param[in] seqs Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value
 */
Temporal *
temporalseq_merge_array(TemporalSeq **seqs, int count)
{
	/* Sort the array */
	temporalseqarr_sort(seqs, count);
	Oid valuetypid = seqs[0]->valuetypid;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seqs[0]->flags);
	bool isgeo = (seqs[0]->valuetypid == type_oid(T_GEOMETRY) ||
		seqs[0]->valuetypid == type_oid(T_GEOGRAPHY));
	/* Number of instants in the resulting sequences */
	int *countinst = palloc0(sizeof(int) * count);
	/* number of instants of the longest sequence */
	int maxcount = countinst[0] = seqs[0]->count;
	int k = 0; /* Number of resulting sequences */
	for (int i = 1; i < count; i++)
	{
		/* Test the validity of consecutive temporal values */
		assert(seqs[i]->valuetypid == valuetypid);
		assert(MOBDB_FLAGS_GET_LINEAR(seqs[i]->flags) == linear);
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *) seqs[i - 1], (Temporal *) seqs[i]);
			ensure_same_dimensionality_tpoint((Temporal *) seqs[i - 1], (Temporal *) seqs[i]);
		}
		TemporalInst *inst1 = temporalseq_inst_n(seqs[i - 1], seqs[i - 1]->count - 1);
		TemporalInst *inst2 = temporalseq_inst_n(seqs[i], 0);
		if (inst1->t > inst2->t)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values cannot overlap on time")));
		if (inst1->t == inst2->t && seqs[i]->period.lower_inc)
		{
			if (! datum_eq(temporalinst_value(inst1), temporalinst_value(inst2), inst1->valuetypid) &&
				seqs[i - 1]->period.upper_inc && seqs[i]->period.lower_inc)
				ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
					errmsg("The temporal values have different value at their overlapping instant")));
			else
			{
				/* Continue with the current sequence */
				countinst[k] += seqs[i]->count - 1;
				if (maxcount < countinst[k])
					maxcount = countinst[k];
			}
		}
		else
		{
			/* Update the number of instants of the longest sequence */
			if (maxcount < countinst[k])
				maxcount = countinst[k];
			/* Start a new sequence */
			countinst[++k] = seqs[i]->count;
		}
	}
	k++;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * k);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * maxcount);
	int l = 0; /* Number of the current input sequence */
	for (int i = 0; i < k; i++)
	{
		bool lowerinc = seqs[l]->period.lower_inc;
		int m = 0; /* Number of instants of the current output sequence */
		while (m < countinst[i] && l < count)
		{
			int start = seqs[l]->period.lower_inc && ( m == 0 || !
				seqs[l - 1]->period.upper_inc ) ? 0 : 1;
			int end = seqs[l]->period.upper_inc ? seqs[l]->count :
				seqs[l]->count - 1;
			for (int j = start; j < end; j++)
				instants[m++] = temporalseq_inst_n(seqs[l], j);
			l++;
		}
		bool upperinc = seqs[l - 1]->period.upper_inc;
		if (! upperinc)
			instants[m] = temporalseq_inst_n(seqs[l - 1], seqs[l - 1]->count - 1);
		sequences[i] = temporalseq_make(instants, countinst[i], lowerinc,
			upperinc, linear, true);
	}
	pfree(instants);
	Temporal *result = (k == 1) ? (Temporal *) sequences[0] :
		(Temporal *) temporals_make(sequences, k, true);
	pfree(sequences);
	pfree(countinst);
	return result;
}

/**
 * Returns a copy of the temporal value
 */
TemporalSeq *
temporalseq_copy(const TemporalSeq *seq)
{
	TemporalSeq *result = palloc0(VARSIZE(seq));
	memcpy(result, seq, VARSIZE(seq));
	return result;
}

/**
 * Returns the index of the segment of the temporal sequence value
 * containing the timestamp using binary search
 *
 * If the timestamp is contained in the temporal value, the index of the
 * segment containing the timestamp is returned in the output parameter. 
 * For example, given a value composed of 3 sequences and a timestamp, 
 * the value returned in the output parameter is as follows:
 * @code
 *            0     1     2     3
 *            |-----|-----|-----|   
 * 1)    t^                             => result = -1
 * 2)        t^                         => result = 0 if the lower bound is inclusive, -1 otherwise
 * 3)              t^                   => result = 1
 * 4)                 t^                => result = 1
 * 5)                             t^    => result = -1
 * @endcode
 *
 * @param[in] seq Temporal sequence value
 * @param[in] t Timestamp
 * @result Returns -1 if the timestamp is not contained in the temporal value
 */
int
temporalseq_find_timestamp(const TemporalSeq *seq, TimestampTz t)
{
	int first = 0;
	int last = seq->count - 2;
	int middle = (first + last)/2;
	while (first <= last) 
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq, middle);
		TemporalInst *inst2 = temporalseq_inst_n(seq, middle + 1);
		bool lower_inc = (middle == 0) ? seq->period.lower_inc : true;
		bool upper_inc = (middle == seq->count - 2) ? seq->period.upper_inc : false;
		if ((inst1->t < t && t < inst2->t) ||
			(lower_inc && inst1->t == t) || (upper_inc && inst2->t == t))
			return middle;
		if (t <= inst1->t)
			last = middle - 1;
		else
			first = middle + 1;	
		middle = (first + last)/2;
	}
	return -1;
}

/**
 * Convert an an array of arrays of temporal sequence values into an array of
 * sequence values.
 *
 * @param[in] sequences Array of array of temporal sequence values
 * @param[in] countseqs Array of counters
 * @param[in] count Number of elements in the first dimension of the arrays
 * @param[in] totalseqs Number of elements in the output array
 */
TemporalSeq **
temporalseqarr2_to_temporalseqarr(TemporalSeq ***sequences, int *countseqs, 
	int count, int totalseqs)
{
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	pfree(sequences); pfree(countseqs);	
	return result;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/
 
/**
 * Temporally intersect the two temporal values
 *
 * @param[in] seq,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time.
 */
bool
intersection_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporalseq_at_timestamp(seq, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst1);
	return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst,seq Temporal values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time.
 */
bool
intersection_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporalseq_temporalinst(seq, inst, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] seq,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time.
 */
bool
intersection_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporali_period(&p, ti);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			instants1[k] = temporalseq_at_timestamp(seq, inst->t);
			instants2[k++] = inst;
		}
		if (seq->period.upper < inst->t)
			break;
	}
	if (k == 0)
	{
		pfree(instants1); pfree(instants2); 
		return false;
	}
	
	*inter1 = temporali_make_free(instants1, k);
	*inter2 = temporali_make(instants2, k);
	pfree(instants2); 
	return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,seq Temporal values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time.
 */
bool
intersection_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	TemporalI **inter1, TemporalI **inter2)
{
	return intersection_temporalseq_temporali(seq, ti, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] seq1,seq2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time.
 */
bool
intersection_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	TemporalSeq **inter1, TemporalSeq **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return false;
	
	*inter1 = temporalseq_at_period(seq1, inter);
	*inter2 = temporalseq_at_period(seq2, inter);
	pfree(inter);
	return true;
}

/*****************************************************************************
 * Synchronize two TemporalSeq values. The values are split into (redundant)
 * segments defined over the same set of instants covering the intersection
 * of their time spans. Depending on the value of the argument crossings,
 * potential crossings between successive pair of instants are added.
 * Crossings are only added when at least one of the sequences has linear
 * interpolation.
 *****************************************************************************/

/**
 * Synchronize the two temporal values
 *
 * The resulting values are composed of a denormalized sequence
 * covering the intersection of their time spans
 *
 * @param[in] seq1,seq2 Input values
 * @param[out] sync1, sync2 Output values
 * @param[in] crossings State whether turning points are added in the segments
 * @result Returns false if the input values do not overlap on time
 */
bool
synchronize_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	TemporalSeq **sync1, TemporalSeq **sync2, bool crossings)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return false;

	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	/* If the two sequences intersect at an instant */
	if (inter->lower == inter->upper)
	{
		TemporalInst *inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		TemporalInst *inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		*sync1 = temporalseq_make(&inst1, 1, true, true, linear1, false);
		*sync2 = temporalseq_make(&inst2, 1, true, true, linear2, false);
		pfree(inst1); pfree(inst2); pfree(inter);
		return true;
	}
	
	/* 
	 * General case 
	 * seq1 =  ... *     *   *   *      *>
	 * seq2 =       <*            *     * ...
	 * sync1 =      <X C * C * C X C X C *>
	 * sync1 =      <* C X C X C * C * C X>
	 * where X are values added for synchronization and C are values added
	 * for the crossings
	 */
	TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
	TemporalInst *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (inst1->t < inter->lower)
	{
		inst1 = tofreeinst = temporalseq_at_timestamp(seq1, inter->lower);
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (inst2->t < inter->lower)
	{
		inst2 = tofreeinst = temporalseq_at_timestamp(seq2, inter->lower);
		j = temporalseq_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count * 2);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	while (i < seq1->count && j < seq2->count &&
		(inst1->t <= inter->upper || inst2->t <= inter->upper))
	{
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			inst2 = temporalseq_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else 
		{
			j++;
			inst1 = temporalseq_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant add potential crossing before adding
		   the new instants */
		if (crossings && (linear1 || linear2) && k > 0)
		{
			TimestampTz crosstime;
			Datum inter1, inter2;
			if (temporalseq_intersection(instants1[k - 1], inst1, linear1,
				instants2[k - 1], inst2, linear2, &inter1, &inter2, &crosstime))
			{
				instants1[k] = tofree[l++] = temporalinst_make(inter1,
					crosstime, seq1->valuetypid);
				instants2[k++] = tofree[l++] = temporalinst_make(inter2,
					crosstime, seq2->valuetypid);
			}
		}
		instants1[k] = inst1; instants2[k++] = inst2;
		if (i == seq1->count || j == seq2->count)
			break;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	/* We are sure that k != 0 due to the period intersection test above */
	/* The last two values of sequences with step interpolation and
	   exclusive upper bound must be equal */
	if (! inter->upper_inc && k > 1 && ! linear1)
	{
		if (datum_ne(temporalinst_value(instants1[k - 2]), 
			temporalinst_value(instants1[k - 1]), seq1->valuetypid))
		{
			instants1[k - 1] = temporalinst_make(temporalinst_value(instants1[k - 2]),
				instants1[k - 1]->t, instants1[k - 1]->valuetypid); 
			tofree[l++] = instants1[k - 1];
		}
	}
	if (! inter->upper_inc && k > 1 && ! linear2)
	{
		if (datum_ne(temporalinst_value(instants2[k - 2]), 
			temporalinst_value(instants2[k - 1]), seq2->valuetypid))
		{
			instants2[k - 1] = temporalinst_make(temporalinst_value(instants2[k - 2]),
				instants2[k - 1]->t, instants2[k - 1]->valuetypid); 
			tofree[l++] = instants2[k - 1];
		}
	}
	*sync1 = temporalseq_make(instants1, k, inter->lower_inc,
		inter->upper_inc, linear1, false);
	*sync2 = temporalseq_make(instants2, k, inter->lower_inc,
		inter->upper_inc, linear2, false);
	
	for (i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(instants1); pfree(instants2); pfree(tofree); pfree(inter);

	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Returns the string representation of the temporal value
 *
 * @param[in] seq Temporal value
 * @param[in] component True when the output string is a component of
 * a temporal sequence set value and thus no interpolation string 
 * at the begining of the string should be output
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
temporalseq_to_string(const TemporalSeq *seq, bool component, 
	char *(*value_out)(Oid, Datum))
{
	char **strings = palloc(sizeof(char *) * seq->count);
	size_t outlen = 0;
	char prefix[20];
	if (! component && linear_interpolation(seq->valuetypid) && 
		!MOBDB_FLAGS_GET_LINEAR(seq->flags))
		sprintf(prefix, "Interp=Stepwise;");
	else
		prefix[0] = '\0';
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		strings[i] = temporalinst_to_string(inst, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	char open = seq->period.lower_inc ? (char) '[' : (char) '(';
	char close = seq->period.upper_inc ? (char) ']' : (char) ')';
	return stringarr_to_string(strings, seq->count, outlen, prefix,
		open, close);
}

/**
 * Write the binary representation of the temporal value
 * into the buffer
 *
 * @param[in] seq Temporal value
 * @param[in] buf Buffer
 */
void
temporalseq_write(const TemporalSeq *seq, StringInfo buf)
{
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, (uint32) seq->count, 4);
#else
	pq_sendint32(buf, seq->count);
#endif
	pq_sendbyte(buf, seq->period.lower_inc ? (uint8) 1 : (uint8) 0);
	pq_sendbyte(buf, seq->period.upper_inc ? (uint8) 1 : (uint8) 0);
	pq_sendbyte(buf, MOBDB_FLAGS_GET_LINEAR(seq->flags) ? (uint8) 1 : (uint8) 0);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		temporalinst_write(inst, buf);
	}
}

/**
 * Returns a new temporal value from its binary representation 
 * read from the buffer (dispatch function)
 *
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
TemporalSeq *
temporalseq_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	bool lower_inc = (char) pq_getmsgbyte(buf);
	bool upper_inc = (char) pq_getmsgbyte(buf);
	bool linear = (char) pq_getmsgbyte(buf);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
		instants[i] = temporalinst_read(buf, valuetypid);
	return temporalseq_make_free(instants, count, lower_inc,
		upper_inc, linear, true);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast the temporal integer value as a temporal float value
 */
TemporalSeq *
tintseq_to_tfloatseq(const TemporalSeq *seq)
{
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument is a temporal integer */
	TemporalSeq *result = temporalseq_copy(seq);
	result->valuetypid = FLOAT8OID;
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		inst->valuetypid = FLOAT8OID;
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
	}
	return result;
}

/**
 * Cast the temporal float value as a temporal integer value
 */
TemporalSeq *
tfloatseq_to_tintseq(const TemporalSeq *seq)
{
	if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot cast temporal float with linear interpolation to temporal integer")));
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument has step interpolation */
	TemporalSeq *result = temporalseq_copy(seq);
	result->valuetypid = INT4OID;
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		inst->valuetypid = INT4OID;
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = Int32GetDatum((double)DatumGetFloat8(temporalinst_value(inst)));
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Transform the temporal instant value into a temporal sequence value
 */
TemporalSeq *
temporalinst_to_temporalseq(const TemporalInst *inst, bool linear)
{
	return temporalseq_make((TemporalInst **)&inst, 1, true, true, linear, false);
}

/**
 * Transform the temporal instant set value into a temporal sequence value
 */
TemporalSeq *
temporali_to_temporalseq(const TemporalI *ti, bool linear)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal sequence")));
	TemporalInst *inst = temporali_inst_n(ti, 0);
	return temporalseq_make(&inst, 1, true, true, linear, false);
}

/**
 * Transform the temporal sequence set value into a temporal sequence value
 */
TemporalSeq *
temporals_to_temporalseq(const TemporalS *ts)
{
	if (ts->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal sequence")));
	return temporalseq_copy(temporals_seq_n(ts, 0));
}

/**
 * Transform the temporal sequence value with continuous base type 
 * from stepwise to linear interpolation
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @return Number of resulting sequences returned
 */
int
tstepseq_to_linear1(TemporalSeq **result, const TemporalSeq *seq)
{
	if (seq->count == 1)
	{
		result[0] = temporalseq_copy(seq);
		MOBDB_FLAGS_SET_LINEAR(result[0]->flags, true);
		return 1;
	}

	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	Datum value2;
	TemporalInst *inst2;
	bool lower_inc = seq->period.lower_inc;
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		inst2 = temporalseq_inst_n(seq, i);
		value2 = temporalinst_value(inst2);
		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = temporalinst_make(value1, inst2->t, seq->valuetypid);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc &&
			datum_eq(value1, value2, seq->valuetypid) : false;
		result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			true, false);
		inst1 = inst2;
		value1 = value2;
		lower_inc = true;
		pfree(instants[1]);
	}
	if (seq->period.upper_inc)
	{
		value1 = temporalinst_value(temporalseq_inst_n(seq, seq->count - 2));
		value2 = temporalinst_value(inst2);
		if (datum_ne(value1, value2, seq->valuetypid))
			result[k++] = temporalseq_make(&inst2, 1, true, true,
				true, false);
	}
	return k;
}

/**
 * Transform the temporal sequence value with continuous base type 
 * from stepwise to linear interpolation
 
 * @param[in] seq Temporal value
 * @return Resulting temporal sequence set value
 */
TemporalS *
tstepseq_to_linear(const TemporalSeq *seq)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tstepseq_to_linear1(sequences, seq);
	return temporals_make_free(sequences, count, false);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Returns the distinct base values of the temporal value with stepwise 
 * interpolation
 *
 * @param[in] seq Temporal value
 * @param[out] count Number of values in the resulting array
 * @result C array of Datums
 */
Datum *
temporalseq_values1(const TemporalSeq *seq, int *count)
{
	Datum *result = palloc(sizeof(Datum *) * seq->count);
	for (int i = 0; i < seq->count; i++)
		result[i] = temporalinst_value(temporalseq_inst_n(seq, i));
	datumarr_sort(result, seq->count, seq->valuetypid);
	*count = datumarr_remove_duplicates(result, seq->count, seq->valuetypid);
	return result;
}

/**
 * Returns the base values of the temporal value with stepwise 
 * interpolation
 *
 * @param[in] seq Temporal value
 * @result PostgreSQL array of Datums
 */
ArrayType *
temporalseq_values(const TemporalSeq *seq)
{
	int count;
	Datum *values = temporalseq_values1(seq, &count);
	ArrayType *result = datumarr_to_array(values, count, seq->valuetypid);
	pfree(values);
	return result;
}

/**
 * Returns the range of base values of the temporal float 
 * with linear interpolation
 *
 * @result C array of ranges
 */
RangeType *
tfloatseq_range(const TemporalSeq *seq)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
	TBOX *box = temporalseq_bbox_ptr(seq);
	Datum min = Float8GetDatum(box->xmin);
	Datum max = Float8GetDatum(box->xmax);
	if (box->xmin == box->xmax)
		return range_make(min, max, true, true, FLOAT8OID);

	Datum start = temporalinst_value(temporalseq_inst_n(seq, 0));
	Datum end = temporalinst_value(temporalseq_inst_n(seq, seq->count - 1));
	Datum lower, upper;
	bool lower_inc, upper_inc;
	if (DatumGetFloat8(start) < DatumGetFloat8(end))
	{
		lower = start; lower_inc = seq->period.lower_inc;
		upper = end; upper_inc = seq->period.upper_inc;
	}
	else
	{
		lower = end; lower_inc = seq->period.upper_inc;
		upper = start; upper_inc = seq->period.lower_inc;
	}
	bool min_inc = DatumGetFloat8(min) < DatumGetFloat8(lower) ||
		(DatumGetFloat8(min) == DatumGetFloat8(lower) && lower_inc);
	bool max_inc = DatumGetFloat8(max) > DatumGetFloat8(upper) ||
		(DatumGetFloat8(max) == DatumGetFloat8(upper) && upper_inc);
	if (!min_inc || !max_inc)
	{
		for (int i = 1; i < seq->count - 1; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			if (min_inc || DatumGetFloat8(min) == DatumGetFloat8(temporalinst_value(inst)))
				min_inc = true;
			if (max_inc || DatumGetFloat8(max) == DatumGetFloat8(temporalinst_value(inst)))
				max_inc = true;
			if (min_inc && max_inc)
				break;
		}
	}
	return range_make(min, max, min_inc, max_inc, FLOAT8OID);
}

/**
 * Returns the ranges of base values of the temporal float 
 * with stepwise interpolation
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * ranges are stored
 * @param[in] seq Temporal value
 * @result Number of ranges in the result
 */
int
tfloatseq_ranges1(RangeType **result, const TemporalSeq *seq)
{
	/* Temporal float with linear interpolation */
	if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
	{
		result[0] = tfloatseq_range(seq);
		return 1;
	}

	/* Temporal float with step interpolation */
	int count;
	Datum *values = temporalseq_values1(seq, &count);
	for (int i = 0; i < count; i++)
		result[i] = range_make(values[i], values[i], true, true, FLOAT8OID);
	pfree(values);
	return count;
}

/**
 * Returns the ranges of base values of the temporal float 
 * with stepwise interpolation
 *
 * @param[in] seq Temporal value
 * @result PostgreSQL array of ranges
 */
ArrayType *
tfloatseq_ranges(const TemporalSeq *seq)
{
	int count = MOBDB_FLAGS_GET_LINEAR(seq->flags) ? 1 : seq->count;
	RangeType **ranges = palloc(sizeof(RangeType *) * count);
	int count1 = tfloatseq_ranges1(ranges, seq);
	ArrayType *result = rangearr_to_array(ranges, count1, type_oid(T_FLOATRANGE));
	for (int i = 0; i < count1; i++)
		pfree(ranges[i]);
	pfree(ranges);
	return result;
}

/**
 * Returns the time on which the temporal value is defined as a period set
 */
PeriodSet *
temporalseq_get_time(const TemporalSeq *seq)
{
	return period_to_periodset_internal(&seq->period);
}

/**
 * Returns a pointer to the instant with minimum base value of the
 * temporal value
 * 
 * The function does not take into account whether the instant is at an
 * exclusive bound or not
 *
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 */
TemporalInst *
temporalseq_min_instant(const TemporalSeq *seq)
{
	Datum min = temporalinst_value(temporalseq_inst_n(seq, 0));
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_lt(value, min, seq->valuetypid))
		{
			min = value;
			k = i;
		}
	}
	return temporalseq_inst_n(seq, k);
}

/**
 * Returns the minimum base value of the temporal value
 */
Datum
temporalseq_min_value(const TemporalSeq *seq)
{
	if (seq->valuetypid == INT4OID)
	{
		TBOX *box = temporalseq_bbox_ptr(seq);
		return Int32GetDatum((int)(box->xmin));
	}
	if (seq->valuetypid == FLOAT8OID)
	{
		TBOX *box = temporalseq_bbox_ptr(seq);
		return Float8GetDatum(box->xmin);
	}
	Datum result = temporalinst_value(temporalseq_inst_n(seq, 0));
	for (int i = 1; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_lt(value, result, seq->valuetypid))
			result = value;
	}
	return result;
}

/**
 * Returns the maximum base value of the temporal value
 */
Datum
temporalseq_max_value(const TemporalSeq *seq)
{
	if (seq->valuetypid == INT4OID)
	{
		TBOX *box = temporalseq_bbox_ptr(seq);
		return Int32GetDatum((int)(box->xmax));
	}
	if (seq->valuetypid == FLOAT8OID)
	{
		TBOX *box = temporalseq_bbox_ptr(seq);
		return Float8GetDatum(box->xmax);
	}
	Datum result = temporalinst_value(temporalseq_inst_n(seq, 0));
	for (int i = 1; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_gt(value, result, seq->valuetypid))
			result = value;
	}
	return result;
}

/**
 * Returns the timespan of the temporal value
 */
Datum
temporalseq_timespan(const TemporalSeq *seq)
{
	Interval *result = period_timespan_internal(&seq->period);
	return PointerGetDatum(result);
}

/**
 * Returns the bounding period on which the temporal value is defined
 */
void
temporalseq_period(Period *p, const TemporalSeq *seq)
{
	period_set(p, seq->period.lower, seq->period.upper,
		seq->period.lower_inc, seq->period.upper_inc);
}

/**
 * Returns the distinct instants of the temporal value as a C array
 */
TemporalInst **
temporalseq_instants(const TemporalSeq *seq)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
		result[i] = temporalseq_inst_n(seq, i);
	return result;
}

/**
 * Returns the distinct instants of the temporal value as a PostgreSQL array
 */
ArrayType *
temporalseq_instants_array(const TemporalSeq *seq)
{
	TemporalInst **instants = temporalseq_instants(seq);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, seq->count);
	pfree(instants);
	return result;
}

/**
 * Returns the start timestamp of the temporal value
 */
TimestampTz
temporalseq_start_timestamp(const TemporalSeq *seq)
{
	return (temporalseq_inst_n(seq, 0))->t;
}

/**
 * Returns the end timestamp of the temporal value
 */
TimestampTz
temporalseq_end_timestamp(const TemporalSeq *seq)
{
	return (temporalseq_inst_n(seq, seq->count - 1))->t;
}

/**
 * Returns the timestamps of the temporal value as a C array
 */
TimestampTz *
temporalseq_timestamps1(const TemporalSeq *seq)
{
	TimestampTz *result = palloc(sizeof(TimestampTz) * seq->count);
	for (int i = 0; i < seq->count; i++)
		result[i] = temporalseq_inst_n(seq, i)->t;
	return result;
}

/**
 * Returns the timestamps of the temporal value as a PostgreSQL array
 */
ArrayType *
temporalseq_timestamps(const TemporalSeq *seq)
{
	TimestampTz *times = temporalseq_timestamps1(seq);
	ArrayType *result = timestamparr_to_array(times, seq->count);
	pfree(times);
	return result;
}

/**
 * Shift the time span of the temporal value by the interval
 */
TemporalSeq *
temporalseq_shift(const TemporalSeq *seq, const Interval *interval)
{
	TemporalSeq *result = temporalseq_copy(seq);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = instants[i] = temporalseq_inst_n(result, i);
		inst->t = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	}
	/* Shift period */
	result->period.lower = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(seq->period.lower), PointerGetDatum(interval)));
	result->period.upper = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(seq->period.upper), PointerGetDatum(interval)));
	/* Shift bounding box */
	void *bbox = temporalseq_bbox_ptr(result);
	temporal_bbox_shift(bbox, interval, seq->valuetypid);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Ever/always comparison operators
 * The functions assume that the temporal value and the datum value are of
 * the same valuetypid. Ever/always equal are valid for all temporal types
 * including temporal points. All the other comparisons are only valid for
 * temporal alphanumeric types.
 *****************************************************************************/

/**
 * Returns true if the segment of the temporal sequence value with
 * linear interpolation is ever equal to the base value
 *
 * @param[in] inst1,inst2 Instants defining the segment
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_ever_eq1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool lower_inc, bool upper_inc, Datum value)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;

	/* Constant segment */
	if (datum_eq(value1, value2, valuetypid) &&
		datum_eq(value1, value, valuetypid))
		return true;

	/* Test of bounds */
	if (datum_eq(value1, value, valuetypid))
		return lower_inc;
	if (datum_eq(value2, value, valuetypid))
		return upper_inc;

	/* Interpolation for continuous base type */
	return tlinearseq_intersection_value(inst1, inst2, value, valuetypid,
		NULL, NULL);
}

/**
 * Returns true if the temporal value is ever equal to the base value
 */
bool
temporalseq_ever_eq(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		double d = datum_double(value, seq->valuetypid);
		if (d < box.xmin || box.xmax < d)
			return false;
	}

	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
	{
		/* Stepwise interpolation*/
		for (int i = 0; i < seq->count; i++)
		{
			Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
			if (datum_eq(valueinst, value, seq->valuetypid))
				return true;
		}
		return false;
	}

	/* Linear interpolation*/
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		if (tlinearseq_ever_eq1(inst1, inst2, lower_inc, upper_inc, value))
			return true;
		inst1 = inst2;
		lower_inc = true;
	}
	return false;
}

/**
 * Returns true if the temporal value is always equal to the base value
 */
bool
temporalseq_always_eq(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		if (seq->valuetypid == INT4OID)
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetInt32(value);
		else
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetFloat8(value);
	}

	/* The following test assumes that the sequence is in normal form */
	if (seq->count > 2)
		return false;
	for (int i = 0; i < seq->count; i++)
	{
		Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_ne(valueinst, value, seq->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************/

/**
 * Returns true if the segment of the temporal value with linear 
 * interpolation is ever less than or equal to the base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] valuetypid Oid of the base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_ever_le1(Datum value1, Datum value2, Oid valuetypid,
	bool lower_inc, bool upper_inc, Datum value)
{
	/* Constant segment */
	if (datum_eq(value1, value2, valuetypid))
		return datum_le(value1, value, valuetypid);
	/* Increasing segment */
	if (datum_lt(value1, value2, valuetypid))
		return datum_lt(value1, value, valuetypid) ||
			(lower_inc && datum_eq(value1, value, valuetypid));
	/* Decreasing segment */
	return datum_lt(value2, value, valuetypid) ||
		(upper_inc && datum_eq(value2, value, valuetypid));
}

/**
 * Returns true if the segment of the temporal value with linear 
 * interpolation is always less than the base value
 *
 * @param[in] value1,value2 Input base values
 * @param[in] valuetypid Oid of the base type
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 */
static bool
tlinearseq_always_lt1(Datum value1, Datum value2, Oid valuetypid,
	bool lower_inc, bool upper_inc, Datum value)
{
	/* Constant segment */
	if (datum_eq(value1, value2, valuetypid))
		return datum_lt(value1, value1, valuetypid);
	/* Increasing segment */
	if (datum_lt(value1, value2, valuetypid))
		return datum_lt(value2, value, valuetypid) ||
			(! upper_inc && datum_eq(value, value2, valuetypid));
	/* Decreasing segment */
	return datum_lt(value1, value, valuetypid) ||
		(! lower_inc && datum_eq(value1, value, valuetypid));
}

/*****************************************************************************/

/**
 * Returns true if the temporal value is ever less than the base value
 */
bool
temporalseq_ever_lt(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		double d = datum_double(value, seq->valuetypid);
		if (box.xmin < d)
			return true;
		/* It is not necessary to take the bounds into account */
		return false;
	}

	/* We are sure that the type has stewpwise interpolation since
	 * there are currenty no other continuous base type besides tfloat
	 * to which the ever < comparison applies */
	assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
	for (int i = 0; i < seq->count; i++)
	{
		Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_lt(valueinst, value, seq->valuetypid))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is ever less than or equal to
 * the base value
 */
bool
temporalseq_ever_le(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		double d = datum_double(value, seq->valuetypid);
		if (d < box.xmin)
			return false;
	}

	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
	{
		/* Stepwise interpolation */
		for (int i = 0; i < seq->count; i++)
		{
			Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
			if (datum_le(valueinst, value, seq->valuetypid))
				return true;
		}
		return false;
	}

	/* Linear interpolation */
	Datum value1 = temporalinst_value(temporalseq_inst_n(seq, 0));
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		Datum value2 = temporalinst_value(temporalseq_inst_n(seq, i));
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		if (tlinearseq_ever_le1(value1, value2, seq->valuetypid,
			lower_inc, upper_inc, value))
			return true;
		value1 = value2;
		lower_inc = true;
	}
	return false;
}

/**
 * Returns true if the temporal value is always less than the base value
 */
bool
temporalseq_always_lt(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		double d = datum_double(value, seq->valuetypid);
		/* Minimum value may be non inclusive */
		if (d < box.xmax)
			return false;
	}

	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags) || seq->count == 1)
	{
		/* Stepwise interpolation */
		for (int i = 0; i < seq->count; i++)
		{
			Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
			if (! datum_lt(valueinst, value, seq->valuetypid))
				return false;
		}
		return true;
	}

	/* Linear interpolation */
	Datum value1 = temporalinst_value(temporalseq_inst_n(seq, 0));
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		Datum value2 = temporalinst_value(temporalseq_inst_n(seq, i));
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		if (! tlinearseq_always_lt1(value1, value2, seq->valuetypid,
			lower_inc, upper_inc, value))
			return false;
		value1 = value2;
		lower_inc = true;
	}
	return true;
}

/**
 * Returns true if the temporal value is always less than or equal to
 * the base value
 */
bool
temporalseq_always_le(const TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporalseq_bbox(&box, seq);
		double d = datum_double(value, seq->valuetypid);
		if (d < box.xmax)
			return false;
		/* It is not necessary to take the bounds into account */
		return true;
	}

	/* We are sure that the type has stewpwise interpolation since
	 * there are currenty no other continuous base type besides tfloat
	 * to which the always <= comparison applies */
	assert(! MOBDB_FLAGS_GET_LINEAR(seq->flags));
	for (int i = 0; i < seq->count; i++)
	{
		Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
		if (! datum_le(valueinst, value, seq->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * Restricts the segment of a temporal value to the base value
 *
 * @param[in] inst1,inst2 Temporal values defining the segment 
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 * @return Resulting temporal sequence
 */
static TemporalSeq *
temporalseq_at_value1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool linear, bool lower_inc, bool upper_inc, Datum value)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	TemporalInst *instants[2];

	/* Constant segment (step or linear interpolation) */
	if (datum_eq(value1, value2, valuetypid))
	{
		/* If not equal to value */
		if (datum_ne(value1, value, valuetypid))
			return NULL;
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		TemporalSeq *result = temporalseq_make(instants, 2, lower_inc,
			upper_inc, linear, false);
		return result;
	}

	/* Stepwise interpolation */
	if (! linear)
	{
		TemporalSeq *result = NULL;
		if (datum_eq(value1, value, valuetypid))
		{
			/* <value@t1 x@t2> */
			instants[0] = (TemporalInst *) inst1;
			instants[1] = temporalinst_make(value1, inst2->t, valuetypid);
			result = temporalseq_make(instants, 2, lower_inc, false,
				linear, false);
			pfree(instants[1]);
		}
		else if (upper_inc && datum_eq(value, value2, valuetypid))
		{
			/* <x@t1 value@t2] */
			result = temporalseq_make((TemporalInst **)&inst2, 1, true, true, linear, false);
		}
		return result;
	}

	/* Linear interpolation: Test of bounds */
	if (datum_eq(value1, value, valuetypid))
	{
		if (!lower_inc)
			return NULL;
		return temporalseq_make((TemporalInst **)&inst1, 1, true, true, linear, false);
	}
	if (datum_eq(value2, value, valuetypid))
	{
		if (!upper_inc)
			return NULL;
		return temporalseq_make((TemporalInst **)&inst2, 1, true, true, linear, false);
	}

	/* Interpolation */
	Datum projvalue;
	TimestampTz t;
	if (! tlinearseq_intersection_value(inst1, inst2, value, valuetypid, &projvalue, &t))
		return NULL;

	TemporalInst *inst = temporalinst_make(projvalue, t, valuetypid);
	TemporalSeq *result = temporalseq_make(&inst, 1, true, true, linear, false);
	pfree(inst); DATUM_FREE(projvalue, valuetypid);
	return result;
}

/**
 * Restricts the temporal value to the base value
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
temporalseq_at_value2(TemporalSeq **result, const TemporalSeq *seq, Datum value)
{
	Oid valuetypid = seq->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporalseq_bbox(&box1, seq);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return 0;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (datum_ne(temporalinst_value(inst), value, valuetypid))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		TemporalSeq *seq1 = temporalseq_at_value1(inst1, inst2,
			linear, lower_inc, upper_inc, value);
		if (seq1 != NULL)
			result[k++] = seq1;
		inst1 = inst2;
		lower_inc = true;
	}
	return k;
}

/**
 * Restricts the temporal value to the base value
 */
TemporalS *
temporalseq_at_value(const TemporalSeq *seq, Datum value)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = temporalseq_at_value2(sequences, seq, value);
	return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the segment of a temporal value with linear interpolation
 * to the complement of the base value
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] inst1,inst2 Temporal values defining the segment 
 * @param[in] lower_inc,upper_inc Upper and lower bounds of the segment
 * @param[in] value Base value
 * @return Number of resulting sequences returned
 */
static int
tlinearseq_minus_value1(TemporalSeq **result,
	const TemporalInst *inst1, const TemporalInst *inst2,
	bool lower_inc, bool upper_inc, Datum value)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	TemporalInst *instants[2];

	/* Constant segment */
	if (datum_eq(value1, value2, valuetypid))
	{
		/* Equal to value */
		if (datum_eq(value1, value, valuetypid))
			return 0;

		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		result[0] = temporalseq_make(instants, 2,
			lower_inc, upper_inc, true, false);
		return 1;
	}

	/* Test of bounds */
	if (datum_eq(value1, value, valuetypid))
	{
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		result[0] = temporalseq_make(instants, 2, false, upper_inc,
			true, false);
		return 1;
	}
	if (datum_eq(value2, value, valuetypid))
	{
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		result[0] = temporalseq_make(instants, 2, lower_inc, false, true, false);
		return 1;
	}

	/* Linear interpolation */
	Datum projvalue;
	TimestampTz t;
	if (!tlinearseq_intersection_value(inst1, inst2, value, valuetypid,
		&projvalue, &t))
	{
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc, true, false);
		return 1;
	}
	instants[0] = (TemporalInst *) inst1;
	instants[1] = temporalinst_make(projvalue, t, valuetypid);
	result[0] = temporalseq_make(instants, 2, lower_inc, false, true, false);
	instants[0] = instants[1];
	instants[1] = (TemporalInst *) inst2;
	result[1] = temporalseq_make(instants, 2, false, upper_inc, true, false);
	pfree(instants[0]); DATUM_FREE(projvalue, valuetypid);
	return 2;
}

/**
 * Restricts the temporal value to the complement of the base value
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
 */
int
temporalseq_minus_value2(TemporalSeq **result, const TemporalSeq *seq, Datum value)
{
	Oid valuetypid = seq->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporalseq_bbox(&box1, seq);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
		{
			result[0] = temporalseq_copy(seq);
			return 1;
		}
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (datum_eq(temporalinst_value(inst), value, valuetypid))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	int k = 0;
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
	{
		/* Stepwise interpolation */
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		bool lower_inc = seq->period.lower_inc;
		int j = 0;
		for (int i = 0; i < seq->count; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			Datum value1 = temporalinst_value(inst);
			if (datum_eq(value1, value, valuetypid))
			{
				if (j > 0)
				{
					instants[j] = temporalinst_make(temporalinst_value(instants[j - 1]),
						inst->t, valuetypid);
					result[k++] = temporalseq_make(instants, j + 1, lower_inc,
						false, false, false);
					pfree(instants[j]);
					j = 0;
				}
				lower_inc = true;
			}
			else
				instants[j++] = inst;
		}
		if (j > 0)
			result[k++] = temporalseq_make(instants, j, lower_inc,
				seq->period.upper_inc, false, false);
		pfree(instants);
	}
	else
	{
		/* Linear interpolation */
		bool lower_inc = seq->period.lower_inc;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		for (int i = 1; i < seq->count; i++)
		{
			TemporalInst *inst2 = temporalseq_inst_n(seq, i);
			bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
			/* The next step adds between one and two sequences */
			k += tlinearseq_minus_value1(&result[k], inst1, inst2,
				lower_inc, upper_inc, value);
			inst1 = inst2;
			lower_inc = true;
		}
	}	
	return k;
}

/**
 * Restricts the temporal value to the complement of the base value
 */
TemporalS *
temporalseq_minus_value(const TemporalSeq *seq, Datum value)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
		maxcount = seq->count;
	else 
		maxcount = seq->count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = temporalseq_minus_value2(sequences, seq, value);
	return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the temporal value to the array of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @return Number of resulting sequences returned
 * @pre There are no duplicates values in the array
 * @note This function is called for each sequence of a temporal sequence set
 */
int
temporalseq_at_values1(TemporalSeq **result, const TemporalSeq *seq, const Datum *values, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_at_values(inst, values, count);
		if (inst1 == NULL)
			return 0;
		
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		for (int j = 0; j < count; j++)
		{
			TemporalSeq *seq1 = temporalseq_at_value1(inst1, inst2, 
				linear, lower_inc, upper_inc, values[j]);
			if (seq1 != NULL) 
				result[k++] = seq1;
		}
		inst1 = inst2;
		lower_inc = true;
	}
	temporalseqarr_sort(result, k);
	return k;
}

/**
 * Restricts the temporal value to the array of base values
 *
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @return Resulting temporal sequence set value
 */
TemporalS *
temporalseq_at_values(const TemporalSeq *seq, const Datum *values, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count);
	int newcount = temporalseq_at_values1(sequences, seq, values, count);
	return temporals_make_free(sequences, newcount, true);
}

/**
 * Restricts the temporal value to the complement of the array of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @return Number of resulting sequences returned
 * @pre There are no duplicates values in the array
 * @note This function is called for each sequence of a temporal sequence set 
 */
int
temporalseq_minus_values1(TemporalSeq **result, const TemporalSeq *seq, const Datum *values, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_minus_values(inst, values, count);
		if (inst1 == NULL)
			return 0;
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* 
	 * General case
	 * Compute first the temporalseq_at_values, then compute its complement.
	 */
	TemporalS *ts = temporalseq_at_values(seq, values, count);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int newcount = 0;
	if (ps2 != NULL)
	{
		newcount = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	pfree(ts); pfree(ps1); 
	return newcount;
}

/**
 * Restricts the temporal value to the complement of the array of base values
 *
 * @param[in] seq Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @return Resulting temporal sequence set value
 */
TemporalS *
temporalseq_minus_values(const TemporalSeq *seq, const Datum *values, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count * 2);
	int newcount = temporalseq_minus_values1(sequences, seq, values, count);
	return temporals_make_free(sequences, newcount, true);
}

/**
 * Restricts the segment of a temporal number to the range of
 * base values
 *
 * @param[in] inst1,inst2 Temporal values defining the segment 
 * @param[in] lower_incl,upper_incl Upper and lower bounds of the segment
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] range Range of base values
 * @return Resulting temporal sequence value
 */
static TemporalSeq *
tnumberseq_at_range1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool lower_incl, bool upper_incl, bool linear, RangeType *range)
{
	TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid,
		TYPECACHE_RANGE_INFO);
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	TemporalInst *instants[2];
	/* Stepwise interpolation or constant segment */
	if (! linear || datum_eq(value1, value2, valuetypid))
	{
		if (! range_contains_elem_internal(typcache, range, value1))
			return NULL;

		instants[0] = (TemporalInst *) inst1;
		instants[1] = linear ? (TemporalInst *) inst2 :
			temporalinst_make(value1, inst2->t, valuetypid);
		/* Stepwise segment with inclusive upper bound must exclude that bound */
		bool upper_incl1 = (linear) ? upper_incl : false ;
		TemporalSeq *result = temporalseq_make(instants, 2, lower_incl,
			upper_incl1, linear, false);
		return result;
	}

	/* Ensure data type with linear interpolation */
	assert(valuetypid == FLOAT8OID);
	bool increasing = DatumGetFloat8(value1) < DatumGetFloat8(value2);
	RangeType *valuerange = increasing ?
		range_make(value1, value2, lower_incl, upper_incl, FLOAT8OID) :
		range_make(value2, value1, upper_incl, lower_incl, FLOAT8OID);
#if MOBDB_PGSQL_VERSION < 110000
	RangeType *intersect = DatumGetRangeType(call_function2(range_intersect, 
		PointerGetDatum(valuerange), PointerGetDatum(range)));
#else
	RangeType *intersect = DatumGetRangeTypeP(call_function2(range_intersect, 
		PointerGetDatum(valuerange), PointerGetDatum(range)));
#endif
	pfree(valuerange);
	if (RangeIsEmpty(intersect))
	{
		pfree(intersect);
		return NULL;
	}

	/* We are sure that neither lower or upper are infinite */
	Datum lower = lower_datum(intersect);
	Datum upper = upper_datum(intersect);
	bool lower_inc2 = lower_inc(intersect);
	bool upper_inc2 = upper_inc(intersect);
	pfree(intersect);
	double dlower = DatumGetFloat8(lower);
	double dupper = DatumGetFloat8(upper);
	double dvalue1 = DatumGetFloat8(value1);
	double dvalue2 = DatumGetFloat8(value2);
	TemporalSeq *result;
	TimestampTz t1, t2;
	bool foundlower = false, foundupper = false;
	if (dlower == dupper)
	{
		t1 = dlower == dvalue1 ? inst1->t : inst2->t;
		instants[0] = temporalinst_make(lower, t1, valuetypid);
		result = temporalseq_make(instants, 1, true, true, linear, false);
		pfree(instants[0]);
		return result;
	}

	double min = Min(dvalue1, dvalue2);
	double max = Max(dvalue1, dvalue2);
	if (min <= dlower && dlower <= max)
		foundlower = tnumberseq_intersection_value(inst1, inst2, lower,
			FLOAT8OID, &t1);
	if (dlower != dupper && min <= dupper && dupper <= max)
		foundupper = tnumberseq_intersection_value(inst1, inst2, upper,
			FLOAT8OID, &t2);

	if (! foundlower && !foundupper)
	{
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
		return temporalseq_make(instants, 2, lower_incl, upper_incl, linear, false);
	}
	if (foundlower && foundupper)
	{
		instants[0] = temporalseq_at_timestamp1(inst1, inst2, linear, Min(t1, t2));
		instants[1] = temporalseq_at_timestamp1(inst1, inst2, linear, Max(t1, t2));
		result = temporalseq_make(instants, 2, lower_inc2, upper_inc2, linear, false);
		pfree(instants[0]); pfree(instants[1]);
		return result;
	}
	if (foundlower)
	{
		if (increasing)
		{
			instants[0] = temporalseq_at_timestamp1(inst1, inst2, linear, t1);
			instants[1] = (TemporalInst *) inst2;
			result = temporalseq_make(instants, 2, lower_inc2, upper_incl, linear, false);
			pfree(instants[0]);
		}
		else
		{
			instants[0] = (TemporalInst *) inst1;
			instants[1] = temporalseq_at_timestamp1(inst1, inst2, linear, t1);
			result = temporalseq_make(instants, 2, lower_incl, upper_inc2, linear, false);
			pfree(instants[1]);
		}
		return result;
	}
	else /* foundupper */
	{
		if (increasing)
		{
			instants[0] = (TemporalInst *) inst1;
			instants[1] = temporalseq_at_timestamp1(inst1, inst2, linear, t2);
			result = temporalseq_make(instants, 2, lower_incl, upper_inc2, linear, false);
			pfree(instants[1]);
		}
		else
		{
			instants[0] = temporalseq_at_timestamp1(inst1, inst2, linear, t2);
			instants[1] = (TemporalInst *) inst2;
			result = temporalseq_make(instants, 2, lower_inc2, upper_incl, linear, false);
			pfree(instants[0]);
		}
		return result;
	}
}

/**
 * Restricts the temporal number to the range of
 * base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq temporal number
 * @param[in] range Range of base values
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set 
 */
int 
tnumberseq_at_range2(TemporalSeq **result, const TemporalSeq *seq, RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporalseq_bbox(&box1, seq);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
		return 0;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	bool linear =  MOBDB_FLAGS_GET_LINEAR(seq->flags);
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		TemporalSeq *seq1 = tnumberseq_at_range1(inst1, inst2, 
			lower_inc, upper_inc, linear, range);
		if (seq1 != NULL) 
			result[k++] = seq1;
		inst1 = inst2;
		lower_inc = true;
	}
	/* Stepwise sequence with inclusive upper bound must add a sequence for that bound */
	if (! linear && seq->period.upper_inc)
	{
		TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid,
			TYPECACHE_RANGE_INFO);
		inst1 = temporalseq_inst_n(seq, seq->count - 1);
		Datum value = temporalinst_value(inst1);
		if (range_contains_elem_internal(typcache, range, value))
			result[k++] = temporalseq_make(&inst1, 1, true, true, false, false);
	}
	return k;
}

/**
 * Restricts the temporal number to the range of base values
 *
 * @param[in] seq temporal number
 * @param[in] range Range of base values
 * @return Resulting temporal sequence set value
 */
TemporalS *
tnumberseq_at_range(const TemporalSeq *seq, RangeType *range)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tnumberseq_at_range2(sequences, seq, range);
	return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the temporal number to the complement of the range
 * of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq temporal number
 * @param[in] range Range of base values
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set 
 */
int
tnumberseq_minus_range1(TemporalSeq **result, const TemporalSeq *seq, 
	RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporalseq_bbox(&box1, seq);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;

	/*
	 * General case
	 * Compute first tnumberseq_at_range, then compute its complement.
	 */
	TemporalS *ts = tnumberseq_at_range(seq, range);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int count = 0;
	if (ps2 != NULL)
	{
		count = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	
	pfree(ts); pfree(ps1); 

	return count;
}

/**
 * Restricts the temporal number to the complement of the range
 * of base values
 *
 * @param[in] seq temporal number
 * @param[in] range Range of base values
 * @return Resulting temporal sequence set value
 * @note This function is called for each sequence of a temporal sequence set 
 */
TemporalS *
tnumberseq_minus_range(const TemporalSeq *seq, RangeType *range)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
		maxcount = seq->count;
	else 
		maxcount = seq->count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumberseq_minus_range1(sequences, seq, range);
	return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the temporal number to the array of ranges of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @return Number of resulting sequences returned
 * @pre The array of ranges is normalized
 * @note This function is called for each sequence of a temporal sequence set 
 */
int
tnumberseq_at_ranges1(TemporalSeq **result, const TemporalSeq *seq,
	RangeType **normranges, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_at_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return 0;
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		for (int j = 0; j < count; j++)
		{
			TemporalSeq *seq1 = tnumberseq_at_range1(inst1, inst2, 
				lower_inc, upper_inc, linear, normranges[j]);
			if (seq1 != NULL) 
				result[k++] = seq1;
		}
		inst1 = inst2;
		lower_inc = true;
	}
	/* Stepwise sequence with inclusive upper bound must add a sequence for that bound */
	if (! linear && seq->period.upper_inc)
	{
		TypeCacheEntry *typcache = lookup_type_cache(
			normranges[count - 1]->rangetypid, TYPECACHE_RANGE_INFO);
		inst1 = temporalseq_inst_n(seq, seq->count - 1);
		Datum value = temporalinst_value(inst1);
		if (range_contains_elem_internal(typcache, normranges[count - 1], value))
			result[k++] = temporalseq_make(&inst1, 1, true, true, false, false);
	}
	if (k == 0)
		return 0;
	
	temporalseqarr_sort(result, k);
	return k;
}

/**
 * Restricts the temporal number to the array of ranges of
 * base values
 *
 * @param[in] seq temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @return Resulting temporal sequence set value
 * @pre The array of ranges is normalized
 */
TemporalS *
tnumberseq_at_ranges(const TemporalSeq *seq, RangeType **normranges, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count);
	int newcount = tnumberseq_at_ranges1(sequences, seq, normranges, count);
	return temporals_make_free(sequences, newcount, true);
}

/**
 * Restricts the temporal number to the complement of the array
 * of ranges of base values
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @return Number of resulting sequences returned
 * @pre The array of ranges is normalized
 * @note This function is called for each sequence of a temporal sequence set 
 */
int 
tnumberseq_minus_ranges1(TemporalSeq **result, const TemporalSeq *seq, RangeType **normranges, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_minus_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return 0;

		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/*  
	 * General case
	 * Compute first the tnumberseq_at_ranges, then compute its complement.
	 */
	TemporalS *ts = tnumberseq_at_ranges(seq, normranges, count);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int newcount = 0;
	if (ps2 != NULL)
	{
		newcount = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	pfree(ts); pfree(ps1); 
	return newcount;
}	

/**
 * Restricts the temporal number to the complement of the array 
 * of ranges of base values
 *
 * @param[in] seq Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @return Resulting temporal sequence set value
 * @pre The array of ranges is normalized
 */
TemporalS *
tnumberseq_minus_ranges(const TemporalSeq *seq, RangeType **normranges, int count)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(seq->flags))
		maxcount = seq->count * count;
	else 
		maxcount = seq->count * count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int newcount = tnumberseq_minus_ranges1(sequences, seq, normranges, count);
	return temporals_make_free(sequences, newcount, true);
}

/**
 * Restricts the temporal value to the minimum base value
 */
TemporalS *
temporalseq_at_min(const TemporalSeq *seq)
{
	Datum min = temporalseq_min_value(seq);
	return temporalseq_at_value(seq, min);
}

/**
 * Restricts the temporal value to the complement of the minimum base value
 */
TemporalS *
temporalseq_minus_min(const TemporalSeq *seq)
{
	Datum min = temporalseq_min_value(seq);
	return temporalseq_minus_value(seq, min);
}

/**
 * Restricts the temporal value to the maximum base value
 */
TemporalS *
temporalseq_at_max(const TemporalSeq *seq)
{
	Datum max = temporalseq_max_value(seq);
	return temporalseq_at_value(seq, max);
}
 
/**
 * Restricts the temporal value to the complement of the maximum base value
 */
TemporalS *
temporalseq_minus_max(const TemporalSeq *seq)
{
	Datum max = temporalseq_max_value(seq);
	return temporalseq_minus_value(seq, max);
}

/**
 * Returns the base value of the segment of the temporal value at the 
 * timestamp
 *
 * @param[in] inst1,inst2 Temporal values defining the segment 
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] t Timestamp
 * @pre The timestamp t is between inst1->t and inst2->t (both inclusive)
 * @note The function creates a new value that must be freed
 */
Datum
temporalseq_value_at_timestamp1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool linear, TimestampTz t)
{
	Oid valuetypid = inst1->valuetypid;
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	/* Constant segment or t is equal to lower bound or step interpolation */
	if (datum_eq(value1, value2, valuetypid) ||
		inst1->t == t || (! linear && t < inst2->t))
		return temporalinst_value_copy(inst1);

	/* t is equal to upper bound */
	if (inst2->t == t)
		return temporalinst_value_copy(inst2);
	
	/* Interpolation for types with linear interpolation */
	double duration1 = (double) (t - inst1->t);
	double duration2 = (double) (inst2->t - inst1->t);
	double ratio = duration1 / duration2;
	Datum result = 0;
	ensure_linear_interpolation_all(valuetypid);
	if (valuetypid == FLOAT8OID)
	{ 
		double start = DatumGetFloat8(value1);
		double end = DatumGetFloat8(value2);
		double dresult = start + (end - start) * ratio;
		result = Float8GetDatum(dresult);
	}
	else if (valuetypid == type_oid(T_DOUBLE2))
	{
		double2 *start = DatumGetDouble2P(value1);
		double2 *end = DatumGetDouble2P(value2);
		double2 *dresult = palloc(sizeof(double2));
		dresult->a = start->a + (end->a - start->a) * ratio;
		dresult->b = start->b + (end->b - start->b) * ratio;
		result = Double2PGetDatum(dresult);
	}
	else if (valuetypid == type_oid(T_GEOMETRY))
	{
		result = geomseg_interpolate_point(value1, value2, ratio);
	}
	else if (valuetypid == type_oid(T_GEOGRAPHY))
	{
		result = geogseg_interpolate_point(value1, value2, ratio);
	}
	else if (valuetypid == type_oid(T_DOUBLE3))
	{
		double3 *start = DatumGetDouble3P(value1);
		double3 *end = DatumGetDouble3P(value2);
		double3 *dresult = palloc(sizeof(double3));
		dresult->a = start->a + (end->a - start->a) * ratio;
		dresult->b = start->b + (end->b - start->b) * ratio;
		dresult->c = start->c + (end->c - start->c) * ratio;
		result = Double3PGetDatum(dresult);
	}
	else if (valuetypid == type_oid(T_DOUBLE4))
	{
		double4 *start = DatumGetDouble4P(value1);
		double4 *end = DatumGetDouble4P(value2);
		double4 *dresult = palloc(sizeof(double4));
		dresult->a = start->a + (end->a - start->a) * ratio;
		dresult->b = start->b + (end->b - start->b) * ratio;
		dresult->c = start->c + (end->c - start->c) * ratio;
		dresult->d = start->d + (end->d - start->d) * ratio;
		result = Double4PGetDatum(dresult);
	}
	return result;
}

/**
 * Returns the base value of the temporal value at the timestamp
 *
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Returns true if the timestamp is contained in the temporal value
 */
bool
temporalseq_value_at_timestamp(const TemporalSeq *seq, TimestampTz t, Datum *result)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
		return false;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		*result = temporalinst_value_copy(temporalseq_inst_n(seq, 0));
		return true;
	}

	/* General case */
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n + 1);
	*result = temporalseq_value_at_timestamp1(inst1, inst2, MOBDB_FLAGS_GET_LINEAR(seq->flags), t);
	return true;
}

/**
 * Restricts the segment of a temporal value to the timestamp
 *
 * @param[in] inst1,inst2 Temporal values defining the segment 
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] t Timestamp
 * @pre The timestamp t is between inst1->t and inst2->t (both inclusive)
 * @note The function creates a new value that must be freed
 */
TemporalInst *
temporalseq_at_timestamp1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool linear, TimestampTz t)
{
	Datum value = temporalseq_value_at_timestamp1(inst1, inst2, linear, t);
	TemporalInst *result = temporalinst_make(value, t, inst1->valuetypid);
	DATUM_FREE(value, inst1->valuetypid);
	return result;
}

/**
 * Restricts the temporal value to the timestamp
 */
TemporalInst *
temporalseq_at_timestamp(const TemporalSeq *seq, TimestampTz t)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
		return NULL;

	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalinst_copy(temporalseq_inst_n(seq, 0));
	
	/* General case */
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n + 1);
	return temporalseq_at_timestamp1(inst1, inst2, MOBDB_FLAGS_GET_LINEAR(seq->flags), t);
}

/**
 * Restricts the temporal value to the complement of the timestamp
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set 
 */
int
temporalseq_minus_timestamp1(TemporalSeq **result, const TemporalSeq *seq,
	TimestampTz t)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;
	
	/* General case */
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	TemporalInst **instants = palloc0(sizeof(TemporalInst *) * seq->count);
	int k = 0;
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0), *inst2;
	/* Compute the first sequence until t */
	if (n != 0 || inst1->t < t)
	{
		for (int i = 0; i < n; i++)
			instants[i] = temporalseq_inst_n(seq, i);
		inst1 = temporalseq_inst_n(seq, n);
		inst2 = temporalseq_inst_n(seq, n + 1);
		if (inst1->t == t)
		{
			if (linear)
			{
				instants[n] = inst1;
				result[k++] = temporalseq_make(instants, n + 1, 
					seq->period.lower_inc, false, linear, false);
			}
			else
			{
				instants[n] = temporalinst_make(temporalinst_value(instants[n - 1]), t,
					inst1->valuetypid);
				result[k++] = temporalseq_make(instants, n + 1, 
					seq->period.lower_inc, false, linear, false);
				pfree(instants[n]);
			}
		}
		else
		{
			/* inst1->t < t */
			instants[n] = inst1;
			instants[n + 1] = linear ?
				temporalseq_at_timestamp1(inst1, inst2, true, t) :
				temporalinst_make(temporalinst_value(inst1), t,
					inst1->valuetypid);
			result[k++] = temporalseq_make(instants, n + 2, 
				seq->period.lower_inc, false, linear, false);
			pfree(instants[n + 1]);
		}
	}
	/* Compute the second sequence after t */
	inst1 = temporalseq_inst_n(seq, n);
	inst2 = temporalseq_inst_n(seq, n + 1);
	if (t < inst2->t)
	{
		instants[0] = temporalseq_at_timestamp1(inst1, inst2, linear, t);
		for (int i = 1; i < seq->count - n; i++)
			instants[i] = temporalseq_inst_n(seq, i + n);
		result[k++] = temporalseq_make(instants, seq->count - n, 
			false, seq->period.upper_inc, linear, false);
		pfree(instants[0]);
	}
	return k;
}

/**
 * Restricts the temporal value to the complement of the timestamp
 *
 * @param[in] seq Temporal value
 * @param[in] t Timestamp
 * @return Resulting temporal sequence set 
 */
TemporalS *
temporalseq_minus_timestamp(const TemporalSeq *seq, TimestampTz t)
{
	TemporalSeq *sequences[2];
	int count = temporalseq_minus_timestamp1((TemporalSeq **)sequences, seq, t);
	if (count == 0)
		return NULL;
	TemporalS *result = temporals_make(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
	// SHOULD WE ADD A FLAG ?
	// return temporals_make_free(sequences, count, false);
}

/**
 * Restricts the temporal value to the timestamp set
 */
TemporalI *
temporalseq_at_timestampset(const TemporalSeq *seq, const TimestampSet *ts)
{
	/* Bounding box test */
	Period *p = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&seq->period, p))
		return NULL;
	
	/* Instantaneous sequence */
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	if (seq->count == 1)
	{
		if (!contains_timestampset_timestamp_internal(ts, inst->t))
			return NULL;
		return temporali_make(&inst, 1);
	}

	/* General case */
	TimestampTz t = Max(seq->period.lower, p->lower);
	int loc;
	timestampset_find_timestamp(ts, t, &loc);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * (ts->count - loc));
	int k = 0;
	for (int i = loc; i < ts->count; i++) 
	{
		t = timestampset_time_n(ts, i);
		inst = temporalseq_at_timestamp(seq, t);
		if (inst != NULL)
			instants[k++] = inst;
	}
	return temporali_make_free(instants, k);
}

/**
 * Restricts the temporal value to the complement of the timestamp set
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ts Timestampset
 * @return Number of resulting sequences returned
 */
int
temporalseq_minus_timestampset1(TemporalSeq **result, const TemporalSeq *seq,
	const TimestampSet *ts)
{
	/* Bounding box test */
	Period *p = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&seq->period, p))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (contains_timestampset_timestamp_internal(ts,inst->t))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous timestamp set */
	if (ts->count == 1)
	{
		return temporalseq_minus_timestamp1(result, seq,
			timestampset_time_n(ts, 0));
	}

	/* General case */
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	TemporalInst **instants = palloc0(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst, *tofree = NULL;
	instants[0] = temporalseq_inst_n(seq, 0);
	int i = 1,	/* current instant of the argument sequence */
		j = 0,	/* current timestamp of the argument timestamp set */
		k = 0,	/* current number of new sequences */
		l = 1;	/* number of instants in the currently constructed sequence */
	bool lower_inc = seq->period.lower_inc;
	while (i < seq->count && j < ts->count)
	{
		inst = temporalseq_inst_n(seq, i);
		TimestampTz t = timestampset_time_n(ts, j);
		if (inst->t < t)
		{
			instants[l++] = inst;
			i++; /* advance instants */
		}
		else if (inst->t == t)
		{
			if (linear)
			{
				instants[l] = inst;
				result[k++] = temporalseq_make(instants, l + 1,
					lower_inc, false, linear, false);
				instants[0] = inst;
			}
			else
			{
				instants[l] = temporalinst_make(
					temporalinst_value(instants[l - 1]), t, inst->valuetypid);
				result[k++] = temporalseq_make(instants, l + 1,
					lower_inc, false, linear, false);
				pfree(instants[l]);
				if (tofree)
				{
					pfree(tofree);
					tofree = NULL;
				}
				instants[0] = inst;
			}
			l = 1;
			lower_inc = false;
			i++; /* advance instants */
			j++; /* advance timestamps */
		}
		else
		{
			/* inst->t > t */
			if (instants[l - 1]->t < t)
			{
				/* The instant to remove is not the first one of the sequence */
				instants[l] = linear ?
					temporalseq_at_timestamp1(instants[l - 1], inst, true, t) :
					temporalinst_make(temporalinst_value(instants[l - 1]), t,
						inst->valuetypid);
				result[k++] = temporalseq_make(instants, l + 1,
					lower_inc, false, linear, false);
				if (tofree)
					pfree(tofree);
				instants[0] = tofree = instants[l];
				l = 1;
			}
			lower_inc = false;
			j++; /* advance timestamps */
		}
	}
	/* Compute the sequence after the timestamp set */
	if (i < seq->count)
	{
		for (j = i; j < seq->count; j++)
			instants[l++] = temporalseq_inst_n(seq, j);
		result[k++] = temporalseq_make(instants, l,
			false, seq->period.upper_inc, linear, false);
	}
	if (tofree)
		pfree(tofree);
	return k;
}

/**
 * Restricts the temporal value to the complement of the timestamp set
 */
TemporalS *
temporalseq_minus_timestampset(const TemporalSeq *seq, const TimestampSet *ts)
{
	TemporalSeq **sequences = palloc0(sizeof(TemporalSeq *) * (ts->count + 1));
	int count = temporalseq_minus_timestampset1(sequences, seq, ts);
	return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the temporal value to the period
 */
TemporalSeq *
temporalseq_at_period(const TemporalSeq *seq, const Period *p)
{
	/* Bounding box test */
	if (!overlaps_period_period_internal(&seq->period, p))
		return NULL;

	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalseq_copy(seq);

	/* General case */
	Period *inter = intersection_period_period_internal(&seq->period, p);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	TemporalSeq *result;
	/* Intersecting period is instantaneous */
	if (inter->lower == inter->upper)
	{
		TemporalInst *inst = temporalseq_at_timestamp(seq, inter->lower);
		result = temporalseq_make(&inst, 1, true, true, linear, false);
		pfree(inst); pfree(inter);
		return result;		
	}
	
	int n = temporalseq_find_timestamp(seq, inter->lower);
	/* If the lower bound of the intersecting period is exclusive */
	if (n == -1)
		n = 0;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * (seq->count - n));
	/* Compute the value at the beginning of the intersecting period */
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n + 1);
	instants[0] = temporalseq_at_timestamp1(inst1, inst2, linear, inter->lower);
	int k = 1;
	for (int i = n + 2; i < seq->count; i++)
	{
		/* If the end of the intersecting period is between inst1 and inst2 */
		if (inst1->t <= inter->upper && inter->upper <= inst2->t)
			break;

		inst1 = inst2;
		inst2 = temporalseq_inst_n(seq, i);
		/* If the intersecting period contains inst1 */
		if (inter->lower <= inst1->t && inst1->t <= inter->upper)
			instants[k++] = inst1;
	}
	/* The last two values of sequences with step interpolation and
	   exclusive upper bound must be equal */
	if (linear || inter->upper_inc)
		instants[k++] = temporalseq_at_timestamp1(inst1, inst2, linear,
			inter->upper);
	else
	{	
		Datum value = temporalinst_value(instants[k - 1]);
		instants[k++] = temporalinst_make(value, inter->upper, seq->valuetypid);
	}
	/* Since by definition the sequence is normalized it is not necessary to
	   normalize the projection of the sequence to the period */
	result = temporalseq_make(instants, k, inter->lower_inc, inter->upper_inc,
		linear, false);

	pfree(instants[0]); pfree(instants[k - 1]); pfree(instants); pfree(inter);
	
	return result;
}

/**
 * Restricts the temporal value to the complement of the period
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] p Period
 * @return Number of resulting sequences returned
 */
int
temporalseq_minus_period1(TemporalSeq **result, const TemporalSeq *seq,
	const Period *p)
{
	/* Bounding box test */
	if (!overlaps_period_period_internal(&seq->period, p))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;

	/* General case */
	PeriodSet *ps = minus_period_period_internal(&seq->period, p);
	if (ps == NULL)
		return 0;
	for (int i = 0; i < ps->count; i++)
	{
		Period *p1 = periodset_per_n(ps, i);
		result[i] = temporalseq_at_period(seq, p1);
	}
	pfree(ps);
	return ps->count;
}

/**
 * Restricts the temporal value to the complement of the period
 */
TemporalS *
temporalseq_minus_period(const TemporalSeq *seq, const Period *p)
{
	TemporalSeq *sequences[2];
	int count = temporalseq_minus_period1(sequences, seq, p);
	if (count == 0)
		return NULL;
	TemporalS *result = temporals_make(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
	// SHOULD WE ADD A FLAG ?
	// return temporals_make_free(sequences, count, false);
	return temporals_make_free(sequences, count, false);
}

/**
 * Restricts the temporal value to the period set
 
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
*/
int
temporalseq_at_periodset1(TemporalSeq **result, const TemporalSeq *seq, 
	const PeriodSet *ps)
{
	/* Bounding box test */
	Period *p = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&seq->period, p))
		return 0;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (!contains_periodset_timestamp_internal(ps, inst->t))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	int loc;
	periodset_find_timestamp(ps, seq->period.lower, &loc);
	int k = 0;
	for (int i = loc; i < ps->count; i++)
	{
		p = periodset_per_n(ps, i);
		TemporalSeq *seq1 = temporalseq_at_period(seq, p);
		if (seq1 != NULL)
			result[k++] = seq1;
		if (seq->period.upper < p->upper)
			break;
	}
	return k;
}

/**
 * Restricts the temporal value to the period set
 *
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @param[out] count Number of resulting sequences returned
 * @return Array of resulting sequences
 */
TemporalSeq **
temporalseq_at_periodset2(const TemporalSeq *seq, const PeriodSet *ps,
	int *count)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ps->count);
	*count = temporalseq_at_periodset1(result, seq, ps);
	if (*count == 0)
	{
		pfree(result);
		return NULL;
	}
	return result;
}

/**
 * Restricts the temporal value to the period set
 */
TemporalS *
temporalseq_at_periodset(const TemporalSeq *seq, const PeriodSet *ps)
{
	int count;
	TemporalSeq **sequences = temporalseq_at_periodset2(seq, ps, &count);
	if (count == 0)
		return NULL;
	TemporalS *result = temporals_make(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
	// SHOULD WE ADD A FLAG ?
	// return temporals_make_free(sequences, count, true);
}

/**
 * Restricts the temporal value to the complement of the period set
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @param[in] from Index from which the processing starts
 * @return Number of resulting sequences returned
 * @note This function is called for each sequence of a temporal sequence set
*/
int
temporalseq_minus_periodset1(TemporalSeq **result, const TemporalSeq *seq,
	const PeriodSet *ps, int from)
{
	/* The sequence can be split at most into (count + 1) sequences
		|----------------------|
			|---| |---| |---|
	*/
	TemporalSeq *curr = temporalseq_copy(seq);
	int k = 0;
	for (int i = from; i < ps->count; i++)
	{
		Period *p1 = periodset_per_n(ps, i);
		/* If the remaining periods are to the left of the current period */
		int cmp = timestamp_cmp_internal(curr->period.upper, p1->lower);
		if (cmp < 0 || (cmp == 0 && curr->period.upper_inc && ! p1->lower_inc))
		{
			result[k++] = curr;
			break;
		}
		TemporalSeq *minus[2];
		int countminus = temporalseq_minus_period1(minus, curr, p1);
		pfree(curr);
		/* minus can have from 0 to 2 periods */
		if (countminus == 0)
			break;
		else if (countminus == 1)
			curr = minus[0];
		else /* countminus == 2 */
		{
			result[k++] = minus[0];
			curr = minus[1];
		}
		/* There are no more periods left */
		if (i == ps->count - 1)
			result[k++] = curr;
	}
	return k;
}

/**
 * Restricts the temporal value to the complement of the period set
 *
 * @param[in] seq Temporal value
 * @param[in] ps Period set
 * @return Resulting temporal sequence set
*/
TemporalS *
temporalseq_minus_periodset(const TemporalSeq *seq, const PeriodSet *ps)
{
	/* Bounding box test */
	Period *p = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&seq->period, p))
		return temporalseq_to_temporals(seq);

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (contains_periodset_timestamp_internal(ps, inst->t))
			return NULL;
		return temporalseq_to_temporals(seq);
	}

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ps->count + 1));
	int count = temporalseq_minus_periodset1(sequences, seq, ps, 0);
	return temporals_make_free(sequences, count, false);
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * Returns true if the temporal value intersects the timestamp
 */
bool
temporalseq_intersects_timestamp(const TemporalSeq *seq, TimestampTz t)
{
	return contains_period_timestamp_internal(&seq->period, t);
}

/**
 * Returns true if the temporal value intersects the timestamp set
 */
bool
temporalseq_intersects_timestampset(const TemporalSeq *seq, const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (temporalseq_intersects_timestamp(seq, timestampset_time_n(ts, i))) 
			return true;
	return false;
}

/**
 * Returns true if the temporal value intersects the period
 */
bool
temporalseq_intersects_period(const TemporalSeq *seq, const Period *p)
{
	return overlaps_period_period_internal(&seq->period, p);
}

/**
 * Returns true if the temporal value intersects the period set
 */
bool
temporalseq_intersects_periodset(const TemporalSeq *seq, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporalseq_intersects_period(seq, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/**
 * Returns the integral (area under the curve) of the temporal number
 */
double
tnumberseq_integral(const TemporalSeq *seq)
{
	double result = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		if (MOBDB_FLAGS_GET_LINEAR(seq->flags))
		{
			/* Linear interpolation */
			double min = Min(DatumGetFloat8(temporalinst_value(inst1)), 
				DatumGetFloat8(temporalinst_value(inst2)));
			double max = Max(DatumGetFloat8(temporalinst_value(inst1)), 
				DatumGetFloat8(temporalinst_value(inst2)));
			result += (max + min) * (double) (inst2->t - inst1->t) / 2.0;
		}
		else
		{
			/* Step interpolation */
			result += datum_double(temporalinst_value(inst1), inst1->valuetypid) *
				(double) (inst2->t - inst1->t);
		}
		inst1 = inst2;
	}
	return result;
}

/**
 * Returns the time-weighted average of the temporal number
 */
double
tnumberseq_twavg(const TemporalSeq *seq)
{
	double duration = (double) (seq->period.upper - seq->period.lower);
	double result;
	if (duration == 0.0)
		/* Instantaneous sequence */
		result = datum_double(temporalinst_value(temporalseq_inst_n(seq, 0)),
			seq->valuetypid);
	else
		result = tnumberseq_integral(seq) / duration;
	return result;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * Returns true if the two temporal sequence values are equal
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
temporalseq_eq(const TemporalSeq *seq1, const TemporalSeq *seq2)
{
	assert(seq1->valuetypid == seq2->valuetypid);
	/* If number of sequences, flags, or periods are not equal */
	if (seq1->count != seq2->count || seq1->flags != seq2->flags ||
			! period_eq_internal(&seq1->period, &seq2->period)) 
		return false;

	/* If bounding boxes are not equal */
	void *box1 = temporalseq_bbox_ptr(seq1);
	void *box2 = temporalseq_bbox_ptr(seq2);
	if (! temporal_bbox_eq(box1, box2, seq1->valuetypid))
		return false;
	
	/* Compare the composing instants */
	for (int i = 0; i < seq1->count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, i);
		if (! temporalinst_eq(inst1, inst2))
			return false;
	}
	return true;
}

/**
 * Returns -1, 0, or 1 depending on whether the first temporal value 
 * is less than, equal, or greater than the second one
 *
 * @pre The arguments are of the same base type
 * @pre For optimization purposes is is supposed that
 * 1. a bounding box comparison has been done before in the calling function
 *    and thus that the bounding boxes are equal,
 * 2. the flags of two temporal values of the same base type are equal.
 * These hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
int
temporalseq_cmp(const TemporalSeq *seq1, const TemporalSeq *seq2)
{
	assert(seq1->valuetypid == seq2->valuetypid);
	/* Compare inclusive/exclusive bounds
	 * These tests are redundant for temporal types whose bounding box is a
	 * period, that is, tbool and ttext */
	if ((seq1->period.lower_inc && ! seq2->period.lower_inc) ||
		(! seq1->period.upper_inc && seq2->period.upper_inc))
		return -1;
	else if ((seq2->period.lower_inc && ! seq1->period.lower_inc) ||
		(! seq2->period.upper_inc && seq1->period.upper_inc))
		return 1;
	/* Compare composing instants */
	int count = Min(seq1->count, seq2->count);
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, i);
		int result = temporalinst_cmp(inst1, inst2);
		if (result) 
			return result;
	}
	/* Compare flags  */
	if (seq1->flags < seq2->flags)
		return -1;
	if (seq1->flags > seq2->flags)
		return 1;
	/* The two values are equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements and the approach for range types for combining the period 
 * bounds.
 *****************************************************************************/

/**
 * Returns the hash value of the temporal value
 */
uint32
temporalseq_hash(const TemporalSeq *seq)
{
	uint32 result;
	char flags = '\0';

	/* Create flags from the lower_inc and upper_inc values */
	if (seq->period.lower_inc)
		flags |= 0x01;
	if (seq->period.upper_inc)
		flags |= 0x02;
	result = DatumGetUInt32(hash_uint32((uint32) flags));
	
	/* Merge with hash of instants */
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		uint32 inst_hash = temporalinst_hash(inst);
		result = (result << 5) - result + inst_hash;
	}
	return result;
}

/*****************************************************************************/
