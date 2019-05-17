/*****************************************************************************
 *
 * GeoBoundBoxOps.c
 *	  Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are
 * GBOX, where the x, y, and optional z coordinates are for the space (value)
 * dimension and the m coordinate is for the time dimension.
 * The following operators are defined:
 *	  overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************
 * GBOX bounding box operators
 * Functions copied/modified from PostGIS file g_box.c
 *****************************************************************************/

/* contains? */

bool
contains_gbox_gbox_internal(const GBOX *box1, const GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax != infinity && box2->xmax != infinity ) 
		if ( box2->xmin < box1->xmin || box2->xmax > box1->xmax )
			return false;
	if ( box1->ymax != infinity && box2->ymax != infinity ) 
		if ( box2->ymin < box1->ymin || box2->ymax > box1->ymax )
			return false;
	if ( box1->zmax != infinity && box2->zmax != infinity ) 
		if ( box2->zmin < box1->zmin || box2->zmax > box1->zmax )
			return false;
	if ( box1->mmax != infinity && box2->mmax != infinity ) 
		if ( box2->mmin < box1->mmin || box2->mmax > box1->mmax )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(contains_gbox_gbox);

PGDLLEXPORT Datum
contains_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	if (FLAGS_GET_GEODETIC(box1->flags) != FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(contains_gbox_gbox_internal(box1, box2));
}

/* contained? */

bool
contained_gbox_gbox_internal(const GBOX *box1, const GBOX *box2)
{
	return contains_gbox_gbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_gbox_gbox);

PGDLLEXPORT Datum
contained_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	if (FLAGS_GET_GEODETIC(box1->flags) != FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(contained_gbox_gbox_internal(box1, box2));
}

/* overlaps? */

bool
overlaps_gbox_gbox_internal(const GBOX *box1, const GBOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->xmax != infinity && box2->xmax != infinity ) 
		if ( box1->xmax < box2->xmin || box1->xmin > box2->xmax )
			return false;
	if ( box1->ymax != infinity && box2->ymax != infinity ) 
		if ( box1->ymax < box2->ymin || box1->ymin > box2->ymax )
			return false;
	if ( box1->zmax != infinity && box2->zmax != infinity ) 
		if ( box1->zmax < box2->zmin || box1->zmin > box2->zmax )
			return false;
	if ( box1->mmax != infinity && box2->mmax != infinity ) 
		if ( box1->mmax < box2->mmin || box1->mmin > box2->mmax )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(overlaps_gbox_gbox);

PGDLLEXPORT Datum
overlaps_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	if (FLAGS_GET_GEODETIC(box1->flags) != FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(overlaps_gbox_gbox_internal(box1, box2));
}

/* same? */

bool
same_gbox_gbox_internal(const GBOX *box1, const GBOX *box2)
{
	double infinity = get_float8_infinity();
 	if ( box1->xmax != infinity && box2->xmax != infinity ) 
		if ( box1->xmin != box2->xmin || box1->xmax != box2->xmax )
			return false;
	if ( box1->ymax != infinity && box2->ymax != infinity ) 
		if ( box1->ymin != box2->ymin || box1->ymax != box2->ymax )
			return false;
	if ( box1->zmax != infinity && box2->zmax != infinity ) 
		if ( box1->zmin != box2->zmin || box1->zmax != box2->zmax )
			return false;
	if ( box1->mmax != infinity && box2->mmax != infinity ) 
		if ( box1->mmin != box2->mmin || box1->mmax != box2->mmax )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(same_gbox_gbox);

PGDLLEXPORT Datum
same_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	if (FLAGS_GET_GEODETIC(box1->flags) != FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(same_gbox_gbox_internal(box1, box2));
}

/*****************************************************************************/

/*-------------------------------------------------------------------------
 * Determine the 3D hypotenuse.
 *
 * If required, x, y, and z are swapped to make x the larger number. The
 * traditional formula of x^2+y^2+z^2 is rearranged to factor x outside the 
 * sqrt. This allows computation of the hypotenuse for significantly
 * larger values, and with a higher precision than when using the naive
 * formula.  In particular, this cannot overflow unless the final result
 * would be out-of-range.
 *
 * sqrt( x^2 + y^2 + z^2 ) 	= sqrt( x^2( 1 + y^2/x^2 + z^2/x^2) )
 * 							= x * sqrt( 1 + y^2/x^2 + z^2/x^2)
 * 					 		= x * sqrt( 1 + y/x * y/x + z/x * z/x)
 *-----------------------------------------------------------------------
 */
static double
pg_hypot3D(double x, double y, double z)
{
	double		yx;
	double		zx;

	/* Handle INF and NaN properly */
	if (isinf(x) || isinf(y)|| isinf(z))
		return get_float8_infinity();

	if (isnan(x) || isnan(y) || isnan(z))
		return get_float8_nan();

	/* Else, drop any minus signs */
	x = fabs(x);
	y = fabs(y);
	z = fabs(z);

	/* Swap x, y and z if needed to make x the larger one */
	if (FPlt(x, y))
	{
		double		temp1 = x;
		x = y;
		y = temp1;
	}
	if (FPlt(x, z))
	{
		double		temp2 = x;
		x = z;
		z = temp2;
	}
	/*
	 * If x is zero, the hypotenuse is computed with the 2D case.  
	 * This test saves a few cycles in such cases, but more importantly 
	 * it also protects against divide-by-zero errors, since now x >= y.
	 */
	if (FPzero(x))
		return pg_hypot(y,z);

	/* Determine the hypotenuse */
	yx = y / x;
	zx = z / x;
	return x * sqrt(1.0 + (yx * yx) + (zx * zx));
}

/* Minimum distance between 2 bounding boxes */

double
distance_gbox_gbox_internal(GBOX *box1, GBOX *box2)
{
	double		x = 0, y = 0, z = 0;

	if (overlaps_gbox_gbox_internal(box1, box2))
		return 0.0;

	/* The two boxes must overlap on the M axis, which corresponds to time */
	if (box1->mmax < box2->mmin || box2->mmax < box1->mmin)
		return DBL_MAX;

	/* X axis */
	if (box1->xmax < box2->xmin)
		x = box2->xmin - box1->xmax;
	else if (box2->xmax < box1->xmin)
		x = box1->xmin - box2->xmax;
	/* Y axis */
	if (box1->ymax < box2->ymin)
		y = box2->ymin - box1->ymax;
	else if (box2->ymax < box1->ymin)
		y = box1->ymin - box2->ymax;
	/* Z axis */
	if (FLAGS_GET_Z(box1->flags) && FLAGS_GET_Z(box2->flags))
	{
		if (box1->zmax < box2->zmin)
			z = box2->zmin - box1->zmax;
		else if (box2->zmax < box1->zmin)
			z = box1->zmin - box2->zmax;
		return pg_hypot3D(x, y, z);
	}
	else
		return pg_hypot(x, y);
}

PG_FUNCTION_INFO_V1(distance_gbox_gbox);

PGDLLEXPORT Datum
distance_gbox_gbox(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	PG_RETURN_FLOAT8(distance_gbox_gbox_internal(box1, box2));
}

/*****************************************************************************/

/* Functions computing the bounding box at the creation of a temporal point */

void
tpointinst_make_gbox(GBOX *box, Datum value, TimestampTz t)
{
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(value);
	assert(gserialized_get_gbox_p(gs, box) != LW_FAILURE);
	if (! FLAGS_GET_Z(gs->flags) && ! FLAGS_GET_GEODETIC(box->flags))
	{
		/* Set the value of the missing Z dimension to +-infinity */
		double infinity = get_float8_infinity();
		box->zmin = -infinity;
		box->zmax = infinity;
	}
	box->mmin = box->mmax = (double)t;
	FLAGS_SET_M(box->flags, true);
	FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return;
}

/* TemporalInst values do not have a precomputed bounding box */
void
tpointinstarr_to_gbox(GBOX *box, TemporalInst **instants, int count)
{
	Datum value = temporalinst_value(instants[0]);
	tpointinst_make_gbox(box, value, instants[0]->t);
	for (int i = 1; i < count; i++)
	{
		GBOX box1;
		value = temporalinst_value(instants[i]);
		tpointinst_make_gbox(&box1, value, instants[i]->t);
		gbox_merge(&box1, box);
	}
	return;
}

void
tpointseqarr_to_gbox(GBOX *box, TemporalSeq **sequences, int count)
{
	memcpy(box, temporalseq_bbox_ptr(sequences[0]), sizeof(GBOX));
	for (int i = 1; i < count; i++)
	{
		GBOX *box1 = temporalseq_bbox_ptr(sequences[i]);
		gbox_merge(box1, box);
	}
	return;
}

/*****************************************************************************
 * Functions for expanding the bounding box
 *****************************************************************************/

/*
 * Expand the box on the spatial dimension
 */
static GBOX *
gbox_expand_spatial_internal(GBOX *box, double d)
{
	GBOX *result = gbox_copy(box);
	result->xmin = box->xmin - d;
	result->xmax = box->xmax + d;
	result->ymin = box->ymin - d;
	result->ymax = box->ymax + d;
	if (FLAGS_GET_Z(box->flags) || FLAGS_GET_GEODETIC(box->flags))
	{
		result->zmin = box->zmin - d;
		result->zmax = box->zmax + d;
	}
	return result;
}

PG_FUNCTION_INFO_V1(gbox_expand_spatial);

PGDLLEXPORT Datum
gbox_expand_spatial(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	double d = PG_GETARG_FLOAT8(1);
	PG_RETURN_POINTER(gbox_expand_spatial_internal(box, d));
}

/*
 * Expand the temporal point on the spatial dimension
 */

PG_FUNCTION_INFO_V1(tpoint_expand_spatial);

PGDLLEXPORT Datum
tpoint_expand_spatial(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double d = PG_GETARG_FLOAT8(1);
	GBOX box;
	temporal_bbox(&box, temp);
	GBOX *result = gbox_expand_spatial_internal(&box, d);
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/*
 * Expand the box on the temporal dimension
 */
static GBOX *
gbox_expand_temporal_internal(GBOX *box, Datum interval)
{
	GBOX *result = gbox_copy(box);
	Datum mint = TimestampGetDatum((Timestamp)box->mmin);
	Datum maxt = TimestampGetDatum((Timestamp)box->mmax);
	result->mmin = DatumGetTimestamp(call_function2(timestamp_mi_interval, 
		mint, interval));
	result->mmax = DatumGetTimestamp(call_function2(timestamp_pl_interval, 
		maxt, interval));
	return result;
}

PG_FUNCTION_INFO_V1(gbox_expand_temporal);

PGDLLEXPORT Datum
gbox_expand_temporal(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Datum interval = PG_GETARG_DATUM(1);
	if (! FLAGS_GET_M(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The box must have M dimension")));

	PG_RETURN_POINTER(gbox_expand_temporal_internal(box, interval));
}

/*
 * Expand the temporal point on the temporal dimension
 */

PG_FUNCTION_INFO_V1(tpoint_expand_temporal);

PGDLLEXPORT Datum
tpoint_expand_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum interval = PG_GETARG_DATUM(1);
	GBOX box;
	temporal_bbox(&box, temp);
	GBOX *result = gbox_expand_temporal_internal(&box, interval);
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transform a <Type> to a GBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/* Transform a geometry/geography to a gbox.
   Notice that the the gserialized_get_gbox_p do not set any flag */

bool
geo_to_gbox_internal(GBOX *box, GSERIALIZED *gs)
{
	double infinity = get_float8_infinity();
	if (gserialized_get_gbox_p(gs, box) == LW_FAILURE)
	{
		/* All dimensions are initialized to +-infinity for the SP-GiST index */
		box->xmin = box->ymin = box->zmin = box->mmin = -infinity;
		box->xmax = box->ymax = box->zmax = box->mmax = +infinity;
		FLAGS_SET_M(box->flags, true);
		return false;
	}
	if (! FLAGS_GET_Z(gs->flags) && ! FLAGS_GET_GEODETIC(gs->flags))
	{
		/* Missing dimension is initialized to +-infinity */
		box->zmin = -infinity;
		box->zmax = +infinity;
	}
	box->mmin = -infinity;
	box->mmax = +infinity;
	FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	FLAGS_SET_M(box->flags, true);
	FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_to_gbox);

PGDLLEXPORT Datum
geo_to_gbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	GBOX *result = palloc0(sizeof(GBOX));
	bool found = geo_to_gbox_internal(result, gs);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

/* Transform a timestamptz to a box */

void
timestamp_to_gbox_internal(GBOX *box, TimestampTz t)
{
	double infinity = get_float8_infinity();
	box->xmin = box->ymin = box->zmin = -infinity;
	box->xmax = box->ymax = box->zmax = +infinity;
	box->mmin = box->mmax = (double)t;
	FLAGS_SET_M(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestamp_to_gbox);

PGDLLEXPORT Datum
timestamp_to_gbox(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	GBOX *result = palloc0(sizeof(GBOX));
	timestamp_to_gbox_internal(result, t);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
timestampset_to_gbox_internal(GBOX *box, TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	double infinity = get_float8_infinity();
	box->xmin = box->ymin = box->zmin = -infinity;
	box->xmax = box->ymax = box->zmax = +infinity;
	box->mmin = (double)(p->lower);
	box->mmax = (double)(p->upper);
	FLAGS_SET_M(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestampset_to_gbox);

PGDLLEXPORT Datum
timestampset_to_gbox(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	GBOX *result = palloc0(sizeof(GBOX));
	timestampset_to_gbox_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a period to a box */

void
period_to_gbox_internal(GBOX *box, Period *p)
{
	double infinity = get_float8_infinity();
	box->xmin = box->ymin = box->zmin = -infinity;
	box->xmax = box->ymax = box->zmax = +infinity;
	box->mmin = (double)(p->lower);
	box->mmax = (double)(p->upper);
	FLAGS_SET_M(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(period_to_gbox);

PGDLLEXPORT Datum
period_to_gbox(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	GBOX *result = palloc0(sizeof(GBOX));
	period_to_gbox_internal(result, p);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box (internal function only) */

void
periodset_to_gbox_internal(GBOX *box, PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	double infinity = get_float8_infinity();
	box->xmin = box->ymin = box->zmin = -infinity;
	box->xmax = box->ymax = box->zmax = +infinity;
	box->mmin = (double)(p->lower);
	box->mmax = (double)(p->upper);
	FLAGS_SET_M(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(periodset_to_gbox);

PGDLLEXPORT Datum
periodset_to_gbox(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	GBOX *result = palloc0(sizeof(GBOX));
	periodset_to_gbox_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a geometry/geography and a timestamptz to a gbox
   Notice that the the gserialized_get_gbox_p do not set any flag */

bool
geo_timestamp_to_gbox_internal(GBOX *box, GSERIALIZED *gs, TimestampTz t)
{
	double infinity = get_float8_infinity();
	if (gserialized_get_gbox_p(gs, box) == LW_FAILURE)
	{
		/* The dimensions are initialized to +-infinity for the SP-GiST index */
		box->xmin = box->ymin = box->zmin = box->mmin = -infinity;
		box->xmax = box->ymax = box->zmax = box->mmax = +infinity;
		FLAGS_SET_M(box->flags, true);
		return false;
	}
	if (! FLAGS_GET_Z(gs->flags) && ! FLAGS_GET_GEODETIC(gs->flags))
	{
		/* Missing dimension is initialized to +-infinity */
		box->zmin = -infinity;
		box->zmax = +infinity;
	}
	box->mmin = box->mmax = t;
	FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	FLAGS_SET_M(box->flags, true);
	FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_timestamp_to_gbox);

PGDLLEXPORT Datum
geo_timestamp_to_gbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	GBOX *result = palloc0(sizeof(GBOX));
	bool found = geo_timestamp_to_gbox_internal(result, gs, t);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

/* Transform a geometry/geography and a period to a gbox
   Notice that the the gserialized_get_gbox_p do not set any flag*/

bool
geo_period_to_gbox_internal(GBOX *box, GSERIALIZED *gs, Period *p)
{
	double infinity = get_float8_infinity();
	if (gserialized_get_gbox_p(gs, box) == LW_FAILURE)
	{
		/* The dimensions are initialized to +-infinity for the SP-GiST index */
		box->xmin = box->ymin = box->zmin = box->mmin = -infinity;
		box->xmax = box->ymax = box->zmax = box->mmax = +infinity;
		FLAGS_SET_M(box->flags, true);
		return false;
	}
	
	if (! FLAGS_GET_Z(gs->flags) && ! FLAGS_GET_GEODETIC(gs->flags))
	{
		/* Missing dimension is initialized to +-infinity */
		box->zmin = -infinity;
		box->zmax = +infinity;
	}
	box->mmin = (double)p->lower;
	box->mmax = (double)p->upper;
	FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	FLAGS_SET_M(box->flags, true);
	FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_period_to_gbox);

PGDLLEXPORT Datum
geo_period_to_gbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	GBOX *result = palloc0(sizeof(GBOX));
	bool found = geo_period_to_gbox_internal(result, gs, p);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_to_gbox);

PGDLLEXPORT Datum
tpoint_to_gbox(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *result = palloc0(sizeof(GBOX));
	temporal_bbox(result, temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_geo_tpoint);

PGDLLEXPORT Datum
overlaps_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overlaps_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_gbox_tpoint);

PGDLLEXPORT Datum
overlaps_bbox_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overlaps_gbox_gbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_geo);

PGDLLEXPORT Datum
overlaps_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	temporal_bbox(&box1, temp);
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	bool result = overlaps_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_gbox);

PGDLLEXPORT Datum
overlaps_bbox_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = overlaps_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overlaps_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_geo_tpoint);

PGDLLEXPORT Datum
contains_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = contains_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_gbox_tpoint);

PGDLLEXPORT Datum
contains_bbox_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = contains_gbox_gbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_geo);

PGDLLEXPORT Datum
contains_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = contains_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_gbox);

PGDLLEXPORT Datum
contains_bbox_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = contains_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contains_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_geo_tpoint);

PGDLLEXPORT Datum
contained_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = contained_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_gbox_tpoint);

PGDLLEXPORT Datum
contained_bbox_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = contained_gbox_gbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_geo);

PGDLLEXPORT Datum
contained_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = contained_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_gbox);

PGDLLEXPORT Datum
contained_bbox_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = contained_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contained_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_geo_tpoint);

PGDLLEXPORT Datum
same_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = same_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_gbox_tpoint);

PGDLLEXPORT Datum
same_bbox_gbox_tpoint(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = same_gbox_gbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_tpoint_geo);

PGDLLEXPORT Datum
same_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	GBOX box1, box2;
	if (!geo_to_gbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = same_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_gbox);

PGDLLEXPORT Datum
same_bbox_tpoint_gbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GBOX *box = PG_GETARG_GBOX_P(1);
	GBOX box1;
	temporal_bbox(&box1, temp);
	bool result = same_gbox_gbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	GBOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = same_gbox_gbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
