/*****************************************************************************
 *
 * tpoint_aggfuncs.c
 *	Aggregate functions for temporal points.
 *
 * The only functions currently provided are extent and temporal centroid.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_aggfuncs.h"

#include <assert.h>

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "doublen.h"
#include "temporal_aggfuncs.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

struct GeoAggregateState
{
	int32_t srid;
	bool hasz;
};

static void
geoaggstate_check(SkipList *state, int32_t srid, bool hasz)
{
	if(! state)
		return;
	struct GeoAggregateState *extra = state->extra;
	if (extra && extra->srid != srid)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Geometries must have the same SRID for temporal aggregation")));
	if (extra && extra->hasz != hasz)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Geometries must have the same dimensionality for temporal aggregation")));
}

static void 
geoaggstate_check_as(SkipList *state1, SkipList *state2)
{
	struct GeoAggregateState *extra2 = state2->extra;
	if (extra2)
		geoaggstate_check(state1, extra2->srid, extra2->hasz);
}

static void
geoaggstate_check_t(SkipList *state, Temporal *t)
{
	geoaggstate_check(state, tpoint_srid_internal(t), MOBDB_FLAGS_GET_Z(t->flags) != 0);
}

/*****************************************************************************/

/*
 * Transform a temporal point type into a temporal double3/double4 type for 
 * performing centroid aggregation 
 */
static TemporalInst *
tpointinst_transform_tcentroid(TemporalInst *inst)
{
	TemporalInst *result;
	if (MOBDB_FLAGS_GET_Z(inst->flags))
	{
		POINT3DZ point = datum_get_point3dz(temporalinst_value(inst));
		double4 dvalue;
		double4_set(&dvalue, point.x, point.y, point.z, 1);
		result = temporalinst_make(PointerGetDatum(&dvalue), inst->t,
			type_oid(T_DOUBLE4));
	}
	else 
	{
		POINT2D point = datum_get_point2d(temporalinst_value(inst));
		double3 dvalue;
		double3_set(&dvalue, point.x, point.y, 1);
		result = temporalinst_make(PointerGetDatum(&dvalue), inst->t,
			type_oid(T_DOUBLE3));
	}
	return result;
}

static TemporalInst **
tpointi_transform_tcentroid(TemporalI *ti)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = tpointinst_transform_tcentroid(inst);
	}
	return result;
}

static TemporalSeq *
tpointseq_transform_tcentroid(TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tpointinst_transform_tcentroid(inst);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
	
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
		
	return result;
}

static TemporalSeq **
tpoints_transform_tcentroid(TemporalS *ts)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result[i] = tpointseq_transform_tcentroid(seq);
	}
	return result;
}

/* Dispatch function  */

static Temporal **
tpoint_transform_tcentroid(Temporal *temp, int *count)
{
	Temporal **result = NULL;
	if (temp->duration == TEMPORALINST) 
	{
		result = palloc(sizeof(Temporal *));
		result[0] = (Temporal *)tpointinst_transform_tcentroid((TemporalInst *)temp);
		*count = 1;
	}
	else if (temp->duration == TEMPORALI)
	{
		result = (Temporal **)tpointi_transform_tcentroid((TemporalI *) temp);
		*count = ((TemporalI *)temp)->count;
	} 
	else if (temp->duration == TEMPORALSEQ)
	{
		result = palloc(sizeof(Temporal *));
		result[0] = (Temporal *)tpointseq_transform_tcentroid((TemporalSeq *) temp);
		*count = 1;
	}
	else if (temp->duration == TEMPORALS)
	{
		result = (Temporal **)tpoints_transform_tcentroid((TemporalS *) temp);
		*count = ((TemporalS *)temp)->count;
	}
	assert(result != NULL);
	return result;
}

/*****************************************************************************
 * Extent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_extent_transfn);

PGDLLEXPORT Datum 
tpoint_extent_transfn(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);
	STBOX box1, *result = NULL;
	memset(&box1, 0, sizeof(STBOX));

	/* Can't do anything with null inputs */
	if (!box && !temp)
		PG_RETURN_NULL();
	/* Null box and non-null temporal, return the bbox of the temporal */
	if (!box)
	{
		result = palloc0(sizeof(STBOX));
		temporal_bbox(result, temp);
		PG_RETURN_POINTER(result);
	}
	/* Non-null box and null temporal, return the box */
	if (!temp)
	{
		result = palloc0(sizeof(STBOX));
		memcpy(result, box, sizeof(STBOX));
		PG_RETURN_POINTER(result);
	}

	temporal_bbox(&box1, temp);
	if (!MOBDB_FLAGS_GET_X(box->flags) || !MOBDB_FLAGS_GET_T(box->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Argument STBOX must have both X and T dimensions")));
	if (MOBDB_FLAGS_GET_Z(box->flags) != MOBDB_FLAGS_GET_Z(box1.flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("One argument has Z dimension but the other does not")));
	if (MOBDB_FLAGS_GET_GEODETIC(box->flags) != MOBDB_FLAGS_GET_GEODETIC(box1.flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("One argument has geodetic coordinates but the other does not")));

	result = palloc0(sizeof(STBOX));
	result->xmax = Max(box->xmax, box1.xmax);
	result->ymax = Max(box->ymax, box1.ymax);
	result->tmax = Max(box->tmax, box1.tmax);
	result->xmin = Min(box->xmin, box1.xmin);
	result->ymin = Min(box->ymin, box1.ymin);
	result->tmin = Min(box->tmin, box1.tmin);
	if (MOBDB_FLAGS_GET_Z(box->flags) || MOBDB_FLAGS_GET_GEODETIC(box->flags))
	{
		result->zmax = Max(box->zmax, box1.zmax);
		result->zmin = Min(box->zmin, box1.zmin);
	}
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(box->flags));
	MOBDB_FLAGS_SET_T(result->flags, true);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(box->flags));

	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_extent_combinefn);

PGDLLEXPORT Datum 
tpoint_extent_combinefn(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);
	STBOX *result;

	if (!box2 && !box1)
		PG_RETURN_NULL();
	if (box1 && !box2)
		PG_RETURN_POINTER(box1);
	if (box2 && !box1)
		PG_RETURN_POINTER(box2);

	if (!MOBDB_FLAGS_GET_X(box1->flags) || !MOBDB_FLAGS_GET_T(box1->flags) ||
		!MOBDB_FLAGS_GET_X(box2->flags) || !MOBDB_FLAGS_GET_T(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Arguments must have both X and T dimensions")));
	if (MOBDB_FLAGS_GET_Z(box1->flags) != MOBDB_FLAGS_GET_Z(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("One argument has Z dimension but the other does not")));
	if (MOBDB_FLAGS_GET_GEODETIC(box1->flags) != MOBDB_FLAGS_GET_GEODETIC(box2->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("One argument has geodetic coordinates but the other does not")));

	result = palloc0(sizeof(STBOX));
	result->xmax = Max(box1->xmax, box2->xmax);
	result->ymax = Max(box1->ymax, box2->ymax);
	result->tmax = Max(box1->tmax, box2->tmax);
	result->xmin = Min(box1->xmin, box2->xmin);
	result->ymin = Min(box1->ymin, box2->ymin);
	result->tmin = Min(box1->tmin, box2->tmin);
	if (MOBDB_FLAGS_GET_Z(box1->flags) || MOBDB_FLAGS_GET_GEODETIC(box1->flags))
	{
		result->zmax = Max(box1->zmax, box2->zmax);
		result->zmin = Min(box1->zmin, box2->zmin);
	}
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(box1->flags));
	MOBDB_FLAGS_SET_T(result->flags, true);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(box1->flags));

	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Centroid
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_tcentroid_transfn);

PGDLLEXPORT Datum
tpoint_tcentroid_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
	{
		if (state)
			PG_RETURN_POINTER(state);
		else
			PG_RETURN_NULL();
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);

	geoaggstate_check_t(state, temp);
	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&datum_sum_double4 : &datum_sum_double3;

	int count;
	Temporal **temporals = tpoint_transform_tcentroid(temp, &count);
	if (state)
	{
		if (skiplist_headval(state)->duration != temporals[0]->duration)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Cannot aggregate temporal values of different duration")));
		if (MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state)->flags) != 
				MOBDB_FLAGS_GET_LINEAR(temporals[0]->flags))
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Cannot aggregate temporal values of different interpolation")));

		skiplist_splice(fcinfo, state, temporals, count, func, false);
	}
	else
	{
		state = skiplist_make(fcinfo, temporals, count);
		struct GeoAggregateState extra =
		{
			.srid = tpoint_srid_internal(temp),
			.hasz = MOBDB_FLAGS_GET_Z(temp->flags) != 0
		};
		aggstate_set_extra(fcinfo, state, &extra, sizeof(struct GeoAggregateState));
	}

	for (int i = 0; i< count; i++)
		pfree(temporals[i]);
	pfree(temporals);		
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(state);
}

/*****************************************************************************/
/* Centroid combine function */

PG_FUNCTION_INFO_V1(tpoint_tcentroid_combinefn);

PGDLLEXPORT Datum
tpoint_tcentroid_combinefn(PG_FUNCTION_ARGS)
{
	SkipList *state1 = PG_ARGISNULL(0) ? NULL : 
		(SkipList *) PG_GETARG_POINTER(0);
	SkipList *state2 = PG_ARGISNULL(1) ? NULL :
		(SkipList *) PG_GETARG_POINTER(1);

	geoaggstate_check_as(state1, state2);
	struct GeoAggregateState *extra = NULL;
	if (state1 && state1->extra) 
		extra = state1->extra;
	if (state2 && state2->extra) 
		extra = state2->extra;
	assert(extra != NULL);
	Datum (*func)(Datum, Datum) = extra->hasz ?
		&datum_sum_double4 : &datum_sum_double3;
	SkipList *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		func, false);

	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* Centroid final function */

TemporalI *
tpointinst_tcentroid_finalfn(TemporalInst **instants, int count)
{
	TemporalInst **newinstants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst = instants[i];
		Datum value = 0;
		assert(inst->valuetypid == type_oid(T_DOUBLE4) || 
			inst->valuetypid == type_oid(T_DOUBLE3));
		if (inst->valuetypid == type_oid(T_DOUBLE4))
		{
			double4 *value4 = (double4 *)DatumGetPointer(temporalinst_value(inst));
			assert(value4->d != 0);
			double valuea = value4->a / value4->d;
			double valueb = value4->b / value4->d;
			double valuec = value4->c / value4->d;
			value = call_function3(LWGEOM_makepoint, Float8GetDatum(valuea),
				Float8GetDatum(valueb), Float8GetDatum(valuec));
		}
		else if (inst->valuetypid == type_oid(T_DOUBLE3))
		{
			double3 *value3 = (double3 *)DatumGetPointer(temporalinst_value(inst));
			assert(value3->c != 0);
			double valuea = value3->a / value3->c;
			double valueb = value3->b / value3->c;
			value = call_function2(LWGEOM_makepoint, Float8GetDatum(valuea),
				Float8GetDatum(valueb));
		}
		newinstants[i] = temporalinst_make(value, inst->t, type_oid(T_GEOMETRY));
		pfree(DatumGetPointer(value));
	}
	TemporalI *result = temporali_from_temporalinstarr(newinstants, count);

	for (int i = 0; i < count; i++)
		pfree(newinstants[i]);
	pfree(newinstants);
	
	return result;
}

TemporalS *
tpointseq_tcentroid_finalfn(TemporalSeq **sequences, int count)
{
	TemporalSeq **newsequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalSeq *seq = sequences[i];
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			Datum value = 0;
			assert(inst->valuetypid == type_oid(T_DOUBLE4) || 
				inst->valuetypid == type_oid(T_DOUBLE3));
			if (inst->valuetypid == type_oid(T_DOUBLE4))
			{
				double4 *value4 = (double4 *)DatumGetPointer(temporalinst_value(inst));
				double valuea = value4->a / value4->d;
				double valueb = value4->b / value4->d;
				double valuec = value4->c / value4->d;
				value = call_function3(LWGEOM_makepoint, Float8GetDatum(valuea),
					Float8GetDatum(valueb), Float8GetDatum(valuec));
			}
			else if (inst->valuetypid == type_oid(T_DOUBLE3))
			{
				double3 *value3 = (double3 *)DatumGetPointer(temporalinst_value(inst));
				double valuea = value3->a / value3->c;
				double valueb = value3->b / value3->c;
				value = call_function2(LWGEOM_makepoint, Float8GetDatum(valuea),
					Float8GetDatum(valueb));
			}
			instants[j] = temporalinst_make(value, inst->t, type_oid(T_GEOMETRY));
			pfree(DatumGetPointer(value));
		}
		newsequences[i] = temporalseq_from_temporalinstarr(instants, 
			seq->count, seq->period.lower_inc, seq->period.upper_inc, 
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(newsequences, count, 
		MOBDB_FLAGS_GET_LINEAR(newsequences[0]->flags), true);

	for (int i = 0; i < count; i++)
		pfree(newsequences[i]);
	pfree(newsequences);
	
	return result;
}

PG_FUNCTION_INFO_V1(tpoint_tcentroid_finalfn);

PGDLLEXPORT Datum
tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS)
{
	/* The final function is strict, we do not need to test for null values */
	SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
	if (state->length == 0)
		PG_RETURN_NULL();

	Temporal **values = skiplist_values(state);
	Temporal *result = NULL;
	assert(values[0]->duration == TEMPORALINST ||
		values[0]->duration == TEMPORALSEQ);
	if (values[0]->duration == TEMPORALINST)
		result = (Temporal *)tpointinst_tcentroid_finalfn(
			(TemporalInst **)values, state->length);
	else if (values[0]->duration == TEMPORALSEQ)
		result = (Temporal *)tpointseq_tcentroid_finalfn(
			(TemporalSeq **)values, state->length);

	int32_t srid = ((struct GeoAggregateState *) state->extra)->srid;
	Temporal *sridresult = tpoint_set_srid_internal(result, srid);
	pfree(values);
	pfree(result);

	PG_RETURN_POINTER(sridresult);
}

/*****************************************************************************/
