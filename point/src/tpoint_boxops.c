/*****************************************************************************
 *
 * tpoint_boxops.c
 *	  Bounding box operators for temporal points.
 *
 * These operators test the bounding boxes of temporal points, which are
 * STBOX, where the x, y, and optional z coordinates are for the space (value)
 * dimension and the t coordinate is for the time dimension.
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

#include "tpoint_boxops.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "periodset.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "stbox.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/* Transform a geometry/geography to a stbox */

bool
geo_to_stbox_internal(STBOX *box, GSERIALIZED *gs)
{
	GBOX gbox;
	if (gserialized_get_gbox_p(gs, &gbox) == LW_FAILURE)
	{
		/* Spatial dimensions are set as missing for the SP-GiST index */
		MOBDB_FLAGS_SET_X(box->flags, false);
		MOBDB_FLAGS_SET_Z(box->flags, false);
		MOBDB_FLAGS_SET_T(box->flags, false);
		return false;
	}
	box->xmin = gbox.xmin;
	box->xmax = gbox.xmax;
	box->ymin = gbox.ymin;
	box->ymax = gbox.ymax;
	if (FLAGS_GET_Z(gs->flags) || FLAGS_GET_GEODETIC(gs->flags))
	{
		box->zmin = gbox.zmin;
		box->zmax = gbox.zmax;
	}
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	MOBDB_FLAGS_SET_T(box->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_to_stbox);

PGDLLEXPORT Datum
geo_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	STBOX *result = palloc0(sizeof(STBOX));
	bool found = geo_to_stbox_internal(result, gs);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

/* Transform a timestamptz to a stbox */

void
timestamp_to_stbox_internal(STBOX *box, TimestampTz t)
{
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_Z(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestamp_to_stbox);

PGDLLEXPORT Datum
timestamp_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestamp_to_stbox_internal(result, t);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
timestampset_to_stbox_internal(STBOX *box, TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestampset_to_stbox);

PGDLLEXPORT Datum
timestampset_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestampset_to_stbox_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a period to a box */

void
period_to_stbox_internal(STBOX *box, Period *p)
{
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(period_to_stbox);

PGDLLEXPORT Datum
period_to_stbox(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	STBOX *result = palloc0(sizeof(STBOX));
	period_to_stbox_internal(result, p);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box (internal function only) */

void
periodset_to_stbox_internal(STBOX *box, PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(periodset_to_stbox);

PGDLLEXPORT Datum
periodset_to_stbox(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	periodset_to_stbox_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a geometry/geography and a timestamptz to a stbox */

bool
geo_timestamp_to_stbox_internal(STBOX *box, GSERIALIZED *gs, TimestampTz t)
{
	if (!geo_to_stbox_internal(box, gs))
		return false;
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return true;
}

PG_FUNCTION_INFO_V1(geo_timestamp_to_stbox);

PGDLLEXPORT Datum
geo_timestamp_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	STBOX *result = palloc0(sizeof(STBOX));
	bool found = geo_timestamp_to_stbox_internal(result, gs, t);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

/* Transform a geometry/geography and a period to a stbox */

bool
geo_period_to_stbox_internal(STBOX *box, GSERIALIZED *gs, Period *p)
{
	if (!geo_to_stbox_internal(box, gs))
		return false;
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return true;
}

PG_FUNCTION_INFO_V1(geo_period_to_stbox);

PGDLLEXPORT Datum
geo_period_to_stbox(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	STBOX *result = palloc0(sizeof(STBOX));
	bool found = geo_period_to_stbox_internal(result, gs, p);
	PG_FREE_IF_COPY(gs, 0);
	if (!found)
	{
		pfree(result);
		PG_RETURN_NULL();
	}
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * STBOX bounding box operators
 * Functions copied/modified from PostGIS file g_box.c
 *****************************************************************************/

/* contains? */

bool
contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	/* The boxes should have at least one common dimension XY(Z) or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) ) 
		if (box2->xmin < box1->xmin || box2->xmax > box1->xmax ||
			box2->ymin < box1->ymin || box2->ymax > box1->ymax)
			return false;
	if (MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags)) 
		if (box2->zmin < box1->zmin || box2->zmax > box1->zmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box2->tmin < box1->tmin || box2->tmax > box1->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(contains_stbox_stbox);

PGDLLEXPORT Datum
contains_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(contains_stbox_stbox_internal(box1, box2));
}

/* contained? */

bool
contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	return contains_stbox_stbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_stbox_stbox);

PGDLLEXPORT Datum
contained_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(contained_stbox_stbox_internal(box1, box2));
}

/* overlaps? */

bool
overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	/* The boxes should have at least one common dimension XY(Z) or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) ) 
		if (box1->xmax < box2->xmin || box1->xmin > box2->xmax ||
			box1->ymax < box2->ymin || box1->ymin > box2->ymax)
			return false;
	if (MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags)) 
		if (box1->zmax < box2->zmin || box1->zmin > box2->zmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box1->tmax < box2->tmin || box1->tmin > box2->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(overlaps_stbox_stbox);

PGDLLEXPORT Datum
overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(overlaps_stbox_stbox_internal(box1, box2));
}

/* same? */

bool
same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2)
{
	/* The boxes should have at least one common dimension XY(Z) or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags) ) 
		if (box1->xmin != box2->xmin || box1->xmax != box2->xmax ||
			box1->ymin != box2->ymin || box1->ymax != box2->ymax)
			return false;
	if (MOBDB_FLAGS_GET_Z(box1->flags) && MOBDB_FLAGS_GET_Z(box2->flags)) 
		if (box1->zmin != box2->zmin || box1->zmax != box2->zmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box1->tmin != box2->tmin || box1->tmax != box2->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(same_stbox_stbox);

PGDLLEXPORT Datum
same_stbox_stbox(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_GETARG_STBOX_P(1);
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		elog(ERROR, "Cannot compare geodetic and non-geodetic boxes");
	PG_RETURN_BOOL(same_stbox_stbox_internal(box1, box2));
}

/*****************************************************************************/

/* Functions computing the bounding box at the creation of a temporal point */

/* Expand the first box with the second one */

static void
stbox_expand(STBOX *box1, const STBOX *box2)
{
	box1->xmin = Min(box1->xmin, box2->xmin);
	box1->xmax = Max(box1->xmax, box2->xmax);
	box1->ymin = Min(box1->ymin, box2->ymin);
	box1->ymax = Max(box1->ymax, box2->ymax);
	box1->zmin = Min(box1->zmin, box2->zmin);
	box1->zmax = Max(box1->zmax, box2->zmax);
	box1->tmin = Min(box1->tmin, box2->tmin);
	box1->tmax = Max(box1->tmax, box2->tmax);
}

void
tpointinst_make_stbox(STBOX *box, Datum value, TimestampTz t)
{
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(value);
	assert(geo_to_stbox_internal(box, gs));
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

/* TemporalInst values do not have a precomputed bounding box */
void
tpointinstarr_to_stbox(STBOX *box, TemporalInst **instants, int count)
{
	Datum value = temporalinst_value(instants[0]);
	tpointinst_make_stbox(box, value, instants[0]->t);
	for (int i = 1; i < count; i++)
	{
		STBOX box1;
		memset(&box1, 0, sizeof(STBOX));
		value = temporalinst_value(instants[i]);
		tpointinst_make_stbox(&box1, value, instants[i]->t);
		stbox_expand(box, &box1);
	}
	return;
}

void
tpointseqarr_to_stbox(STBOX *box, TemporalSeq **sequences, int count)
{
	memcpy(box, temporalseq_bbox_ptr(sequences[0]), sizeof(STBOX));
	for (int i = 1; i < count; i++)
	{
		STBOX *box1 = temporalseq_bbox_ptr(sequences[i]);
		stbox_expand(box, box1);
	}
	return;
}

/*****************************************************************************
 * Expand the bounding box of a Temporal with a TemporalInst
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

void
tpoint_expand_stbox(STBOX *box, Temporal *temp, TemporalInst *inst)
{
	temporal_bbox(box, temp);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporalinst_bbox(&box1, inst);
	stbox_expand(box, &box1);
	return;
}

/*****************************************************************************
 * Functions for expanding the bounding box
 *****************************************************************************/

/*
 * Expand the box on the spatial dimension
 */
static STBOX *
stbox_expand_spatial_internal(STBOX *box, double d)
{
	STBOX *result = stbox_copy(box);
	result->xmin = box->xmin - d;
	result->xmax = box->xmax + d;
	result->ymin = box->ymin - d;
	result->ymax = box->ymax + d;
	if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
	{
		result->zmin = box->zmin - d;
		result->zmax = box->zmax + d;
	}
	return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_spatial);

PGDLLEXPORT Datum
stbox_expand_spatial(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	double d = PG_GETARG_FLOAT8(1);
	PG_RETURN_POINTER(stbox_expand_spatial_internal(box, d));
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
	STBOX box;
	memset(&box, 0, sizeof(STBOX));
	temporal_bbox(&box, temp);
	STBOX *result = stbox_expand_spatial_internal(&box, d);
	PG_FREE_IF_COPY(temp, 0);	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/*
 * Expand the box on the time dimension
 */
static STBOX *
stbox_expand_temporal_internal(STBOX *box, Datum interval)
{
	STBOX *result = stbox_copy(box);
	result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval, 
		TimestampTzGetDatum(box->tmin), interval));
	result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval, 
		TimestampTzGetDatum(box->tmax), interval));
	return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_temporal);

PGDLLEXPORT Datum
stbox_expand_temporal(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Datum interval = PG_GETARG_DATUM(1);
	if (! MOBDB_FLAGS_GET_T(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("The box must have T dimension")));

	PG_RETURN_POINTER(stbox_expand_temporal_internal(box, interval));
}

/*
 * Expand the temporal point on the time dimension
 */

PG_FUNCTION_INFO_V1(tpoint_expand_temporal);

PGDLLEXPORT Datum
tpoint_expand_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum interval = PG_GETARG_DATUM(1);
	STBOX box;
	memset(&box, 0, sizeof(STBOX));
	temporal_bbox(&box, temp);
	STBOX *result = stbox_expand_temporal_internal(&box, interval);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = overlaps_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_stbox_tpoint);

PGDLLEXPORT Datum
overlaps_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = overlaps_stbox_stbox_internal(box, &box1);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	bool result = overlaps_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_stbox);

PGDLLEXPORT Datum
overlaps_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = overlaps_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overlaps_stbox_stbox_internal(&box1, &box2);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = contains_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_stbox_tpoint);

PGDLLEXPORT Datum
contains_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = contains_stbox_stbox_internal(box, &box1);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = contains_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_stbox);

PGDLLEXPORT Datum
contains_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = contains_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contains_stbox_stbox_internal(&box1, &box2);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = contained_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_stbox_tpoint);

PGDLLEXPORT Datum
contained_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = contained_stbox_stbox_internal(box, &box1);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = contained_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_stbox);

PGDLLEXPORT Datum
contained_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = contained_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contained_stbox_stbox_internal(&box1, &box2);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box1, gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box2, temp);
	bool result = same_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_stbox_tpoint);

PGDLLEXPORT Datum
same_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = same_stbox_stbox_internal(box, &box1);
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
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	if (!geo_to_stbox_internal(&box2, gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();		
	}
	temporal_bbox(&box1, temp);
	bool result = same_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_stbox);

PGDLLEXPORT Datum
same_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = same_stbox_stbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_tpoint);

PGDLLEXPORT Datum
same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	STBOX box1, box2;
	memset(&box1, 0, sizeof(STBOX));
	memset(&box2, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = same_stbox_stbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
