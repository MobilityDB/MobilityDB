/*****************************************************************************
 *
 * tpoint_aggfuncs.c
 *	Aggregate functions for temporal points.
 *
 * The only functions currently provided are extent and temporal centroid.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
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

/**
 * Structure storing the SRID and the dimensionality of the temporal point 
 * values for aggregation
 */
struct GeoAggregateState
{
	int32_t srid;
	bool hasz;
};

/**
 * Check the validity of the temporal point values for aggregation
 */
static void
geoaggstate_check(const SkipList *state, int32_t srid, bool hasz)
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

/**
 * Check the validity of the temporal point values for aggregation
 */
static void 
geoaggstate_check_as(const SkipList *state1, const SkipList *state2)
{
	if(! state2) 
		return ;
	struct GeoAggregateState *extra2 = state2->extra;
	if (extra2)
		geoaggstate_check(state1, extra2->srid, extra2->hasz);
}

/**
 * Chech the validity of the temporal point values for aggregation
 */
static void
geoaggstate_check_t(const SkipList *state, const Temporal *t)
{
	geoaggstate_check(state, tpoint_srid_internal(t), MOBDB_FLAGS_GET_Z(t->flags) != 0);
}

/*****************************************************************************/

/**
 * Transform a temporal point value of instant duration into a temporal 
 * double3/double4 value for performing temporal centroid aggregation 
 */
static TInstant *
tpointinst_transform_tcentroid(const TInstant *inst)
{
	TInstant *result;
	if (MOBDB_FLAGS_GET_Z(inst->flags))
	{
		const POINT3DZ *point = datum_get_point3dz_p(tinstant_value(inst));
		double4 dvalue;
		double4_set(&dvalue, point->x, point->y, point->z, 1);
		result = tinstant_make(PointerGetDatum(&dvalue), inst->t,
			type_oid(T_DOUBLE4));
	}
	else 
	{
		const POINT2D *point = datum_get_point2d_p(tinstant_value(inst));
		double3 dvalue;
		double3_set(&dvalue, point->x, point->y, 1);
		result = tinstant_make(PointerGetDatum(&dvalue), inst->t,
			type_oid(T_DOUBLE3));
	}
	return result;
}

/**
 * Transform a temporal point value of instant set duration into a temporal 
 * double3/double4 value for performing temporal centroid aggregation 
 */
static TInstant **
tpointinstset_transform_tcentroid(const TInstantSet *ti)
{
	TInstant **result = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		result[i] = tpointinst_transform_tcentroid(inst);
	}
	return result;
}

/**
 * Transform a temporal point value of sequence duration into a temporal 
 * double3/double4 value for performing temporal centroid aggregation 
 */
static TSequence *
tpointseq_transform_tcentroid(const TSequence *seq)
{
	TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TInstant *inst = tsequence_inst_n(seq, i);
		instants[i] = tpointinst_transform_tcentroid(inst);
	}
	return tsequence_make_free(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		MOBDB_FLAGS_GET_LINEAR(seq->flags), false);
}

/**
 * Transform a temporal point value of sequence set duration into a temporal 
 * double3/double4 value for performing temporal centroid aggregation 
 */
static TSequence **
tpointseqset_transform_tcentroid(const TSequenceSet *ts)
{
	TSequence **result = palloc(sizeof(TSequence *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		result[i] = tpointseq_transform_tcentroid(seq);
	}
	return result;
}

/**
 * Transform a temporal point value for performing temporal centroid aggregation 
 * (dispatch function)
 */
static Temporal **
tpoint_transform_tcentroid(const Temporal *temp, int *count)
{
	Temporal **result;
	if (temp->duration == TINSTANT) 
	{
		result = palloc(sizeof(Temporal *));
		result[0] = (Temporal *)tpointinst_transform_tcentroid((TInstant *)temp);
		*count = 1;
	}
	else if (temp->duration == TINSTANTSET)
	{
		result = (Temporal **)tpointinstset_transform_tcentroid((TInstantSet *) temp);
		*count = ((TInstantSet *)temp)->count;
	} 
	else if (temp->duration == TSEQUENCE)
	{
		result = palloc(sizeof(Temporal *));
		result[0] = (Temporal *)tpointseq_transform_tcentroid((TSequence *) temp);
		*count = 1;
	}
	else /* temp->duration == TSEQUENCESET */
	{
		result = (Temporal **)tpointseqset_transform_tcentroid((TSequenceSet *) temp);
		*count = ((TSequenceSet *)temp)->count;
	}
	assert(result != NULL);
	return result;
}

/*****************************************************************************
 * Extent
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_extent_transfn);
/**
 * Transition function for temporal extent aggregation of temporal point values
 */
PGDLLEXPORT Datum 
tpoint_extent_transfn(PG_FUNCTION_ARGS)
{
	STBOX *box = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
	Temporal *temp = PG_ARGISNULL(1) ? NULL : PG_GETARG_TEMPORAL(1);

	/* Can't do anything with null inputs */
	if (!box && !temp)
		PG_RETURN_NULL();
	/* Null box and non-null temporal, return the bbox of the temporal */
	STBOX *result = palloc0(sizeof(STBOX));
	if (!box && temp)
	{
		temporal_bbox(result, temp);
		PG_RETURN_POINTER(result);
	}
	/* Non-null box and null temporal, return the box */
	if (box && !temp)
	{
		memcpy(result, box, sizeof(STBOX));
		PG_RETURN_POINTER(result);
	}

	/* Both box and temporal are not null */
	STBOX box1;
	memset(&box1, 0, sizeof(STBOX));
	ensure_same_srid_tpoint_stbox(temp, box);
	ensure_same_dimensionality_tpoint_stbox(temp, box);
	ensure_same_geodetic_tpoint_stbox(temp, box);
	temporal_bbox(&box1, temp);
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
	result->srid = box->srid;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(box->flags));
	MOBDB_FLAGS_SET_T(result->flags, true);
	MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(box->flags));

	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tpoint_extent_combinefn);
/**
 * Combine function for temporal extent aggregation of temporal point values
 */
PGDLLEXPORT Datum 
tpoint_extent_combinefn(PG_FUNCTION_ARGS)
{
	STBOX *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
	STBOX *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);
	if (!box2 && !box1)
		PG_RETURN_NULL();
	if (box1 && !box2)
		PG_RETURN_POINTER(box1);
	if (box2 && !box1)
		PG_RETURN_POINTER(box2);

	ensure_same_srid_stbox(box1, box2);
	ensure_same_dimensionality_stbox(box1, box2);
	ensure_same_geodetic_stbox(box1, box2);
	STBOX *result = palloc0(sizeof(STBOX));
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
	result->srid = box1->srid;
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
/**
 * Transition function for temporal centroid aggregation of temporal point values
 */
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
	Temporal **tsequenceset = tpoint_transform_tcentroid(temp, &count);
	if (state)
	{
		if (skiplist_headval(state)->duration != tsequenceset[0]->duration)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Cannot aggregate temporal values of different duration")));
		if (MOBDB_FLAGS_GET_LINEAR(skiplist_headval(state)->flags) != 
				MOBDB_FLAGS_GET_LINEAR(tsequenceset[0]->flags))
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Cannot aggregate temporal values of different interpolation")));

		skiplist_splice(fcinfo, state, tsequenceset, count, func, false);
	}
	else
	{
		state = skiplist_make(fcinfo, tsequenceset, count);
		struct GeoAggregateState extra =
		{
			.srid = tpoint_srid_internal(temp),
			.hasz = MOBDB_FLAGS_GET_Z(temp->flags) != 0
		};
		aggstate_set_extra(fcinfo, state, &extra, sizeof(struct GeoAggregateState));
	}

	for (int i = 0; i< count; i++)
		pfree(tsequenceset[i]);
	pfree(tsequenceset);		
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(state);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_tcentroid_combinefn);
/**
 * Combine function for temporal centroid aggregation of temporal point values
 */
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
	SkipList *result = temporal_tagg_combinefn1(fcinfo, state1, state2, 
		func, false);

	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Final function for temporal centroid aggregation of temporal point values
 * with instant duration
 *
 * @param[in] instants Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
TInstantSet *
tpointinst_tcentroid_finalfn(TInstant **instants, int count, int srid)
{
	TInstant **newinstants = palloc(sizeof(TInstant *) * count);
	for (int i = 0; i < count; i++)
	{
		TInstant *inst = instants[i];
		Datum value = 0;
		assert(inst->valuetypid == type_oid(T_DOUBLE4) || 
			inst->valuetypid == type_oid(T_DOUBLE3));
		LWPOINT *lwpoint;
		if (inst->valuetypid == type_oid(T_DOUBLE4))
		{
			double4 *value4 = (double4 *)DatumGetPointer(tinstant_value_ptr(inst));
			assert(value4->d != 0);
			double valuea = value4->a / value4->d;
			double valueb = value4->b / value4->d;
			double valuec = value4->c / value4->d;
			lwpoint = lwpoint_make3dz(srid, valuea, valueb, valuec);

		}
		else /* inst->valuetypid == type_oid(T_DOUBLE3) */
		{
			double3 *value3 = (double3 *)DatumGetPointer(tinstant_value_ptr(inst));
			assert(value3->c != 0);
			double valuea = value3->a / value3->c;
			double valueb = value3->b / value3->c;
			lwpoint = lwpoint_make2d(srid, valuea, valueb);
		}
		value = PointerGetDatum(geometry_serialize((LWGEOM *) lwpoint));
		newinstants[i] = tinstant_make(value, inst->t, type_oid(T_GEOMETRY));
		pfree(DatumGetPointer(value));
	}
	return tinstantset_make_free(newinstants, count);
}

/**
 * Final function for temporal centroid aggregation of temporal point values
 * with sequence duration
 *
 * @param[in] sequences Temporal values
 * @param[in] count Number of elements in the array
 * @param[in] srid SRID of the values
 */
TSequenceSet *
tpointseq_tcentroid_finalfn(TSequence **sequences, int count, int srid)
{
	TSequence **newsequences = palloc(sizeof(TSequence *) * count);
	for (int i = 0; i < count; i++)
	{
		TSequence *seq = sequences[i];
		TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TInstant *inst = tsequence_inst_n(seq, j);
			Datum value = 0;
			assert(inst->valuetypid == type_oid(T_DOUBLE4) || 
				inst->valuetypid == type_oid(T_DOUBLE3));
			LWPOINT *lwpoint;
			if (inst->valuetypid == type_oid(T_DOUBLE4))
			{
				double4 *value4 = (double4 *)DatumGetPointer(tinstant_value_ptr(inst));
				double valuea = value4->a / value4->d;
				double valueb = value4->b / value4->d;
				double valuec = value4->c / value4->d;
				lwpoint = lwpoint_make3dz(srid, valuea, valueb, valuec);
			}
			else /* inst->valuetypid == type_oid(T_DOUBLE3) */
			{
				double3 *value3 = (double3 *)DatumGetPointer(tinstant_value_ptr(inst));
				double valuea = value3->a / value3->c;
				double valueb = value3->b / value3->c;
				lwpoint = lwpoint_make2d(srid, valuea, valueb);
			}
			value = PointerGetDatum(geometry_serialize((LWGEOM *) lwpoint));
			instants[j] = tinstant_make(value, inst->t, type_oid(T_GEOMETRY));
			pfree(DatumGetPointer(value));
		}
		newsequences[i] = tsequence_make_free(instants, seq->count,
			seq->period.lower_inc, seq->period.upper_inc, 
			MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	}
	return tsequenceset_make_free(newsequences, count, true);
}

PG_FUNCTION_INFO_V1(tpoint_tcentroid_finalfn);
/**
 * Final function for temporal centroid aggregation of temporal point values
 */
PGDLLEXPORT Datum
tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS)
{
	/* The final function is strict, we do not need to test for null values */
	SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
	if (state->length == 0)
		PG_RETURN_NULL();

	Temporal **values = skiplist_values(state);
	int32_t srid = ((struct GeoAggregateState *) state->extra)->srid;
	Temporal *result = NULL;
	assert(values[0]->duration == TINSTANT ||
		values[0]->duration == TSEQUENCE);
	if (values[0]->duration == TINSTANT)
		result = (Temporal *)tpointinst_tcentroid_finalfn(
			(TInstant **)values, state->length, srid);
	else if (values[0]->duration == TSEQUENCE)
		result = (Temporal *)tpointseq_tcentroid_finalfn(
			(TSequence **)values, state->length, srid);

	pfree(values);

	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
