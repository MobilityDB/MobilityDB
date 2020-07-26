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
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
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
#include "temporal_boxops.h"
#include "tpoint.h"
#include "stbox.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Transform a <Type> to a STBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

PG_FUNCTION_INFO_V1(box2d_to_stbox);
/**
 * Transform a box2d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box2d_to_stbox(PG_FUNCTION_ARGS)
{
	GBOX *box = (GBOX *)PG_GETARG_POINTER(0);
	STBOX *result = palloc0(sizeof(STBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, false);
	MOBDB_FLAGS_SET_T(result->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, false);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(box3d_to_stbox);
/**
 * Transform a box3d to a spatiotemporal box
 */
PGDLLEXPORT Datum
box3d_to_stbox(PG_FUNCTION_ARGS)
{
	BOX3D *box = (BOX3D *)PG_GETARG_POINTER(0);
	STBOX *result = palloc0(sizeof(STBOX));
	result->xmin = box->xmin;
	result->xmax = box->xmax;
	result->ymin = box->ymin;
	result->ymax = box->ymax;
	result->zmin = box->zmin;
	result->zmax = box->zmax;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, false);
	result->srid = box->srid;
	PG_RETURN_POINTER(result);
}

/**
 * Transform a geometry/geography to a spatiotemporal box
 * (internal function)
 */
bool
geo_to_stbox_internal(STBOX *box, const GSERIALIZED *gs)
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
	box->srid = gserialized_get_srid(gs);
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_Z(box->flags, FLAGS_GET_Z(gs->flags));
	MOBDB_FLAGS_SET_T(box->flags, false);
	MOBDB_FLAGS_SET_GEODETIC(box->flags, FLAGS_GET_GEODETIC(gs->flags));
	return true;
}

PG_FUNCTION_INFO_V1(geo_to_stbox);
/**
 * Transform a geometry/geography to a spatiotemporal box
 */
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

/**
 * Transform a timestampt to a spatiotemporal box
 * (internal function)
 */
void
timestamp_to_stbox_internal(STBOX *box, TimestampTz t)
{
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_Z(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestamp_to_stbox);
/**
 * Transform a timestampt to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestamp_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestamp_to_stbox_internal(result, t);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a timestamp set to a spatiotemporal box
 * (internal function)
 */
void
timestampset_to_stbox_internal(STBOX *box, const TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(timestampset_to_stbox);
/**
 * Transform a timestamp set to a spatiotemporal box
 */
PGDLLEXPORT Datum
timestampset_to_stbox(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	timestampset_to_stbox_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a period to a spatiotemporal box
 * (internal function)
 */
void
period_to_stbox_internal(STBOX *box, const Period *p)
{
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(period_to_stbox);
/**
 * Transform a period to a spatiotemporal box
 */
PGDLLEXPORT Datum
period_to_stbox(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	STBOX *result = palloc0(sizeof(STBOX));
	period_to_stbox_internal(result, p);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a period set to a spatiotemporal box
 * (internal function)
 */
void
periodset_to_stbox_internal(STBOX *box, const PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

PG_FUNCTION_INFO_V1(periodset_to_stbox);
/**
 * Transform a period set to a spatiotemporal box
 */
PGDLLEXPORT Datum
periodset_to_stbox(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	STBOX *result = palloc0(sizeof(STBOX));
	periodset_to_stbox_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/**
 * Transform a geometry/geography and a timestamp to a spatiotemporal box
 * (internal function)
 */
bool
geo_timestamp_to_stbox_internal(STBOX *box, const GSERIALIZED *gs, TimestampTz t)
{
	if (!geo_to_stbox_internal(box, gs))
		return false;
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return true;
}

PG_FUNCTION_INFO_V1(geo_timestamp_to_stbox);
/**
 * Transform a geometry/geography and a timestamp to a spatiotemporal box
 */
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

/**
 * Transform a geometry/geography and a period to a spatiotemporal box
 * (internal function)
 */
bool
geo_period_to_stbox_internal(STBOX *box, const GSERIALIZED *gs, const Period *p)
{
	if (!geo_to_stbox_internal(box, gs))
		return false;
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_T(box->flags, true);
	return true;
}

PG_FUNCTION_INFO_V1(geo_period_to_stbox);
/**
 * Transform a geometry/geography and a period to a spatiotemporal box
 */
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
 * Functions computing the bounding box at the creation of a temporal point
 *****************************************************************************/

/**
 * Set the spatiotemporal box from the temporal instant point value
 */
void
tpointinst_make_stbox(STBOX *box, const TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	GSERIALIZED *gs = (GSERIALIZED *)PointerGetDatum(value);
	assert(geo_to_stbox_internal(box, gs));
	box->tmin = box->tmax = inst->t;
	MOBDB_FLAGS_SET_T(box->flags, true);
}

/**
 * Set the spatiotemporal box from the array of temporal instant point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] instants Temporal instant values
 * @param[in] count Number of elements in the array
 * @note Temporal instant values do not have a precomputed bounding box 
 */
void
tpointinstarr_to_stbox(STBOX *box, TemporalInst **instants, int count)
{
	tpointinst_make_stbox(box, instants[0]);
	for (int i = 1; i < count; i++)
	{
		STBOX box1;
		memset(&box1, 0, sizeof(STBOX));
		tpointinst_make_stbox(&box1, instants[i]);
		stbox_expand(box, &box1);
	}
}

/**
 * Set the spatiotemporal box from the array of temporal sequence point values
 *
 * @param[out] box Spatiotemporal box
 * @param[in] sequences Temporal instant values
 * @param[in] count Number of elements in the array
 */
void
tpointseqarr_to_stbox(STBOX *box, TemporalSeq **sequences, int count)
{
	memcpy(box, temporalseq_bbox_ptr(sequences[0]), sizeof(STBOX));
	for (int i = 1; i < count; i++)
	{
		STBOX *box1 = temporalseq_bbox_ptr(sequences[i]);
		stbox_expand(box, box1);
	}
}

/*****************************************************************************
 * Boxes functions
 * These functions are currently not used but can be used for defining 
 * VODKA indexes https://www.pgcon.org/2014/schedule/events/696.en.html
 *****************************************************************************/

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence point value
 *
 * @param[out] result Spatiotemporal box
 * @param[in] seq Temporal value
 * @return Number of elements in the array
 */
static int
tpointseq_stboxes1(STBOX *result, const TemporalSeq *seq)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		tpointinst_make_stbox(&result[0], inst);
		return 1;
	}

	/* Temporal sequence has at least 2 instants */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 0; i < seq->count - 1; i++)
	{
		tpointinst_make_stbox(&result[i], inst1);
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		STBOX box;
		memset(&box, 0, sizeof(STBOX));
		tpointinst_make_stbox(&box, inst2);
		stbox_expand(&result[i], &box);
		inst1 = inst2;
	}
	return seq->count - 1;
}

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence point value
 *
 * @param[in] seq Temporal value
 */
ArrayType *
tpointseq_stboxes(const TemporalSeq *seq)
{
	assert(MOBDB_FLAGS_GET_LINEAR(seq->flags));
	int count = seq->count - 1;
	if (count == 0)
		count = 1;
	STBOX *boxes = palloc0(sizeof(STBOX) * count);
	tpointseq_stboxes1(boxes, seq);
	ArrayType *result = stboxarr_to_array(boxes, count);
	pfree(boxes);
	return result;
}

/**
 * Returns an array of spatiotemporal boxes from the segments of the 
 * temporal sequence set point value
 *
 * @param[in] ts Temporal value
 */
ArrayType *
tpoints_stboxes(const TemporalS *ts)
{
	assert(MOBDB_FLAGS_GET_LINEAR(ts->flags));
	STBOX *boxes = palloc0(sizeof(STBOX) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tpointseq_stboxes1(&boxes[k], seq);
	}
	ArrayType *result = stboxarr_to_array(boxes, k);
	pfree(boxes);
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_stboxes);
/**
 * Returns an array of spatiotemporal boxes from the temporal point value
 */
PGDLLEXPORT Datum
tpoint_stboxes(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ArrayType *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI)
		;
	else if (temp->duration == TEMPORALSEQ)
		result = tpointseq_stboxes((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = tpoints_stboxes((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for expanding the bounding box
 *****************************************************************************/

/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 * (internal function)
 */
static STBOX *
stbox_expand_spatial_internal(STBOX *box, double d)
{
	ensure_has_X_stbox(box);
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
/**
 * Expand the spatial dimension of the spatiotemporal box with the double value
 */
PGDLLEXPORT Datum
stbox_expand_spatial(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	double d = PG_GETARG_FLOAT8(1);
	PG_RETURN_POINTER(stbox_expand_spatial_internal(box, d));
}

PG_FUNCTION_INFO_V1(tpoint_expand_spatial);
/**
 * Expand the spatial dimension of the bounding box of the temporal point value with the double value
 */
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

/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 * (internal function)
 */
static STBOX *
stbox_expand_temporal_internal(STBOX *box, Datum interval)
{
	ensure_has_T_stbox(box);
	STBOX *result = stbox_copy(box);
	result->tmin = DatumGetTimestampTz(call_function2(timestamp_mi_interval, 
		TimestampTzGetDatum(box->tmin), interval));
	result->tmax = DatumGetTimestampTz(call_function2(timestamp_pl_interval, 
		TimestampTzGetDatum(box->tmax), interval));
	return result;
}

PG_FUNCTION_INFO_V1(stbox_expand_temporal);
/**
 * Expand the temporal dimension of the spatiotemporal box with the interval value
 */
PGDLLEXPORT Datum
stbox_expand_temporal(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Datum interval = PG_GETARG_DATUM(1);
	PG_RETURN_POINTER(stbox_expand_temporal_internal(box, interval));
}

PG_FUNCTION_INFO_V1(tpoint_expand_temporal);
/**
 * Expand the temporal dimension of the bounding box of the temporal point value
 * with the interval value
 */
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
 * Generic functions 
 *****************************************************************************/

/**
 * Generic topological function for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
toporel_bbox_geo_tpoint(FunctionCallInfo fcinfo,
	bool (*func)(const STBOX *, const STBOX *))
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
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a spatiotemporal box and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
toporel_bbox_stbox_tpoint(FunctionCallInfo fcinfo,
	bool (*func)(const STBOX *, const STBOX *))
{
	STBOX *box = PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);

	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a temporal point and a geometry
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
toporel_bbox_tpoint_geo(FunctionCallInfo fcinfo,
	bool (*func)(const STBOX *, const STBOX *))
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
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for a temporal point and a spatiotemporal box
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
toporel_bbox_tpoint_stbox(FunctionCallInfo fcinfo,
	bool (*func)(const STBOX *, const STBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	STBOX *box = PG_GETARG_STBOX_P(1);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	ensure_same_srid_tpoint_stbox(temp, box);
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	temporal_bbox(&box1, temp);
	bool result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/**
 * Generic topological function for two temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
toporel_bbox_tpoint_tpoint(FunctionCallInfo fcinfo,
	bool (*func)(const STBOX *, const STBOX *))
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
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * overlaps
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and 
 * the temporal point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_geo_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the 
 * temporal point overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_stbox_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and the geometry/geography overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_geo(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the spatiotemporal box overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_stbox(fcinfo, &overlaps_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points overlap
 */
PGDLLEXPORT Datum
overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_tpoint(fcinfo, &overlaps_stbox_stbox_internal);
}

/*****************************************************************************
 * contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal box of the geometry/geography contains 
 * the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
contains_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_geo_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box contains the spatiotemporal box of the 
 * temporal point
 */
PGDLLEXPORT Datum
contains_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_stbox_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal point contains the
 * one of the geometry/geography
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_geo(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point contains the spatiotemporal box
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_stbox(fcinfo, &contains_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contains_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal point contains
 * the one of the second temporal point 
 */
PGDLLEXPORT Datum
contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_tpoint(fcinfo, &contains_stbox_stbox_internal);
}

/*****************************************************************************
 * contained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal box of the geometry/geography is
 * contained in the spatiotemporal box of the temporal point
 */
PGDLLEXPORT Datum
contained_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_geo_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box is contained in the spatiotemporal 
 * box of the temporal point
 */
PGDLLEXPORT Datum
contained_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_stbox_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal box of the temporal point is contained 
 * in the one of the geometry/geography
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_geo(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point is contained 
 * in the spatiotemporal box
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_stbox(fcinfo, &contained_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(contained_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal box of the first temporal point is contained
 * in the one of the second temporal point 
 */
PGDLLEXPORT Datum
contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_tpoint(fcinfo, &contained_stbox_stbox_internal);
}

/*****************************************************************************
 * same
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_geo_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the 
 * temporal point are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_stbox_tpoint(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and 
 * geometry/geography are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_geo(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the 
 * spatiotemporal box are equal in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_stbox(fcinfo, &same_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(same_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points are equal
 * in the common dimensions
 */
PGDLLEXPORT Datum
same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_tpoint(fcinfo, &same_stbox_stbox_internal);
}

/*****************************************************************************
 * adjacent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_geo_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the geometry/geography and
 * the temporal point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_geo_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_geo_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_stbox_tpoint);
/**
 * Returns true if the spatiotemporal box and the spatiotemporal box of the 
 * temporal point are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_stbox_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_stbox_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_geo);
/**
 * Returns true if the spatiotemporal boxes of the temporal point and 
 * geometry/geography are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_geo(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_geo(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_stbox);
/**
 * Returns true if the spatiotemporal box of the temporal point and the 
 * spatiotemporal box are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_stbox(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_stbox(fcinfo, &adjacent_stbox_stbox_internal);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tpoint_tpoint);
/**
 * Returns true if the spatiotemporal boxes of the temporal points are adjacent
 */
PGDLLEXPORT Datum
adjacent_bbox_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return toporel_bbox_tpoint_tpoint(fcinfo, &adjacent_stbox_stbox_internal);
}

/*****************************************************************************/
