/*****************************************************************************
 *
 * tpoint_tempspatialrels.c
 *	  Temporal spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal
 * Boolean.
 * The following relationships are supported for a temporal geometry point
 * and a geometry:
 *		tcontains, tcovers, tcoveredby, tdisjoint, tequals, tintersects,
 *		ttouches, twithin, tdwithin, and trelate (with 2 and 3 arguments)
 * The following relationships are supported for two temporal geometry points:
 *		tdisjoint, tequals, tintersects, tdwithin, and trelate (with 2 and 3
 *		arguments)
 * The following relationships are supported for a temporal geography point
 * and a geography:
 *		tequals,
 * The following relationships are supported for two temporal geography points:
 *		tdisjoint, tintersects, tdwithin
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_tempspatialrels.h"

#include <utils/timestamp.h>

#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tbool_boolops.h"
#include "lifting.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_spatialrels.h"

static Temporal * tintersects_tpoint_geo1(Temporal *temp, GSERIALIZED *gs);

/*****************************************************************************
 * Generic functions for computing the temporal spatial relationships
 * with arbitrary geometries
 *****************************************************************************/

/*
 * Examples of values returned by PostGIS for the intersection
 * of a line and an arbitrary geometry

select st_astext(st_intersection(
geometry 'linestring(0 1,2 1)',
geometry 'polygon((0 0,1 1,2 0,0 0))'))
-- "POINT(1 1)"

select st_astext(st_intersection(
geometry 'linestring(0 1,4 1)',
geometry 'polygon((0 0,1 1,2 0.5,3 1,4 0,0 0))'))
-- "MULTIPOINT(1 1,3 1)"

select st_astext(st_intersection(
geometry 'linestring(0 1,2 1)',
geometry 'polygon((1 0,2 0,2 1,1 1,1 0))'))
-- "LINESTRING(1 1,2 1)"

select st_astext(st_intersection(
geometry 'linestring(0 2,5 2)',
geometry 'polygon((1 0,1 3,2 3,2 1,3 1,3 3,4 3,4 0,1 0))'))
-- "MULTILINESTRING((1 2,2 2),(3 2,4 2))"

select st_astext(st_intersection(
geometry 'linestring(0 1,4 1)',
geometry 'polygon((0 0,1 1,2 0.5,3 1,4 1,4 0,0 0))'))
-- "GEOMETRYCOLLECTION(POINT(1 1),LINESTRING(3 1,4 1))"
*/

/*****************************************************************************/

/**
 * Returns the temporal instants at which the segment of a temporal sequence
 * intersects a line 
 *
 * @param[in] inst1,inst2 Instants defining the segment 
 * @param[in] lower_inc,upper_inc True when the corresponding bound is inclusive
 * @param[in] inter Line
 * @param[out] count Number of elements in the resulting array
 */
static TInstant **
tpointseq_intersection_instants(const TInstant *inst1, const TInstant *inst2,
	bool lower_inc, bool upper_inc, Datum inter, int *count)
{
	Datum value1 = tinstant_value(inst1);
	Datum value2 = tinstant_value(inst2);

	/* Each intersection is either a point or a linestring with two points */
	GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
	LWGEOM *lwinter = lwgeom_from_gserialized(gsinter);
	int countinter;
	LWCOLLECTION *coll = NULL;
	LWGEOM *lwinter_single;
	if (lwinter->type == POINTTYPE || lwinter->type == LINETYPE)
	{
		countinter = 1;
		lwinter_single = lwinter;
	}
	else
	{
		coll = lwgeom_as_lwcollection(lwinter);
		countinter = coll->ngeoms;
	}
	const POINT2D *start = datum_get_point2d_p(value1);
	const POINT2D *end = datum_get_point2d_p(value2);
	TInstant **instants = palloc(sizeof(TInstant *) * 2 * countinter);
	long double duration = (long double)(inst2->t - inst1->t);
	int k = 0;
	for (int i = 0; i < countinter; i++)
	{
		if (coll != NULL)
			/* Find the i-th intersection */
			lwinter_single = coll->geoms[i];
		POINT2D p1, p2, closest;
		double fraction1;
		TimestampTz t1;
		Datum point1;
		/* Each intersection is either a point or a linestring with two points */
		if (lwinter_single->type == POINTTYPE)
		{
			lwpoint_getPoint2d_p((LWPOINT *) lwinter_single, &p1);
			fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
			t1 = inst1->t + (long) (duration * fraction1);
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || t1 != inst1->t) && (upper_inc || t1 != inst2->t))
			{
				point1 = tsequence_value_at_timestamp1(inst1, inst2, true, t1);
				instants[k++] = tinstant_make(point1, t1, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point1));
			}
		}
		else /* lwinter_single->type == LINETYPE) */
		{
			LWPOINT *lwpoint1 = lwline_get_lwpoint((LWLINE *) lwinter_single, 0);
			LWPOINT *lwpoint2 = lwline_get_lwpoint((LWLINE *) lwinter_single, 1);
			lwpoint_getPoint2d_p(lwpoint1, &p1);
			lwpoint_getPoint2d_p(lwpoint2, &p2);
			fraction1 = closest_point2d_on_segment_ratio(&p1, start, end, &closest);
			double fraction2 = closest_point2d_on_segment_ratio(&p2, start, end, &closest);
			t1 = inst1->t + (long) (duration * fraction1);
			TimestampTz t2 = inst1->t + (long) (duration * fraction2);
			TimestampTz lower = Min(t1, t2);
			TimestampTz upper = Max(t1, t2);
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || t1 != lower) && (upper_inc || t2 != lower))
			{
				point1 = tsequence_value_at_timestamp1(inst1, inst2, true, lower);
				instants[k++] = tinstant_make(point1, lower, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point1));
			}
			/* If the point intersection is not at an exclusive bound and
			 * lower != upper (this last condition arrives when point1 is
			 * at an epsilon distance from point2 */
			if ((lower_inc || t1 != upper) && (upper_inc || t2 != upper) &&
				(lower != upper))
			{
				Datum point2 = tsequence_value_at_timestamp1(inst1, inst2, true, upper);
				instants[k++] = tinstant_make(point2, upper, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point2));
			}
		}
	}
	/* Sort the instants */
	if (k > 1)
		tinstantarr_sort(instants, k);
	POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
	*count = k;
	return instants;
}

/*****************************************************************************
 * Generic functions to compute the temporal spatial relationship
 * between a temporal sequence (set) point and a geometry.
 * The potential crossings between the two are considered.
 * The resulting sequence (set) has step interpolation since it is a
 * temporal Boolean or a temporal text (for trelate).
 * These functions are not available for geographies since they call the
 * intersection function in PostGIS that is only available for geometries.
 *****************************************************************************/

/**
 * Returns the temporal spatial relationship between a segment of a
 * temporal sequence point and a geometry.
 *
 * @param[in] inst1,inst2 Instants defining the segment 
 * @param[in] linear True when the segment has linear interpolation
 * @param[in] geo Geometry
 * @param[in] param Parameter for ternary relationships
 * @param[in] lower_inc,upper_inc True when the corresponding bound is inclusive
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting type
 * @param[out] count Number of elements in the resulting array
 * @param[in] invert True when the function is called with inverted arguments
 */
static TSequence **
tspatialrel_tpointseq_geo1(TInstant *inst1, TInstant *inst2, bool linear,
	Datum geo, Datum param, bool lower_inc, bool upper_inc,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, int *count, bool invert)
{
	Datum value1 = tinstant_value(inst1);
	Datum value2 = tinstant_value(inst2);
	bool constant = datum_point_eq(value1, value2);
	TInstant *instants[2];
	Datum line, intersections;
	/* If not constant segment and linear interpolation look for intersections */
	if (! constant && linear)
	{
		line = geopoint_line(value1, value2);
		intersections = call_function2(intersection, line, geo);
	}

	/* Constant segment or step interpolation or no intersections */
	if (constant || ! linear || call_function1(LWGEOM_isempty, intersections))
	{
		TSequence **result = palloc(sizeof(TSequence *));
		Datum value = invert ? spatialrel(geo, value1, param, func, numparam) :
			spatialrel(value1, geo, param, func, numparam);
		instants[0] = tinstant_make(value, inst1->t, restypid);
		instants[1] = tinstant_make(value, inst2->t, restypid);
		result[0] = tsequence_make(instants, 2, lower_inc, upper_inc,
			STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
		*count = 1;
		return result;
	}

	/* Look for instants of intersections */
	int countinst;
	TInstant **interinstants = tpointseq_intersection_instants(inst1,
		inst2, lower_inc, upper_inc, intersections, &countinst);
	pfree(DatumGetPointer(intersections));
	pfree(DatumGetPointer(line));

	/* No intersections were found */
	if (countinst == 0)
	{
		/* There may be an intersection at an exclusive bound.
		 * Find the middle time between inst1 and inst2
		 * and compute the func at that point */
		TimestampTz inttime = inst1->t + ((inst2->t - inst1->t) / 2);
		Datum intvalue = tsequence_value_at_timestamp1(inst1, inst2,
			linear, inttime);
		Datum intvalue1 = invert ? spatialrel(geo, intvalue, param, func, numparam) :
			spatialrel(intvalue, geo, param, func, numparam);
		TSequence **result = palloc(sizeof(TSequence *));
		instants[0] = tinstant_make(intvalue1, inst1->t, restypid);
		instants[1] = tinstant_make(intvalue1, inst2->t, restypid);
		result[0] = tsequence_make(instants, 2, lower_inc, upper_inc,
			STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		pfree(DatumGetPointer(intvalue)); DATUM_FREE(intvalue1, restypid);
		*count = 1;
		return result;
	}

	/* Determine whether the period started before/after the first/last intersection */
	bool before = (inst1->t != (interinstants[0])->t);
	bool after = (inst2->t != (interinstants[countinst - 1])->t);

	/* Compute the func */
	int countseq = (2 * countinst) - 1;
	if (before) countseq++;
	if (after) countseq++;
	TSequence **result = palloc(sizeof(TSequence *) * countseq);
	int k = 0;
	if (before)
	{
		Datum value = invert ? spatialrel(geo, value1, param, func, numparam) :
			spatialrel(value1, geo, param, func, numparam);
		instants[0] = tinstant_make(value, inst1->t, restypid);
		instants[1] = tinstant_make(value, (interinstants[0])->t, restypid);
		result[k++] = tsequence_make(instants, 2, lower_inc, false,
			STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}
	for (int i = 0; i < countinst; i++)
	{
		/* Compute the value at the intersection point */
		Datum value = invert ?
			spatialrel(geo, tinstant_value(interinstants[i]), param, func, numparam) :
			spatialrel(tinstant_value(interinstants[i]), geo, param, func, numparam);
		instants[0] = tinstant_make(value, (interinstants[i])->t,
			restypid);
		result[k++] = tsequence_make(instants, 1, true, true, STEP, NORMALIZE_NO);
		DATUM_FREE(value, restypid);
		pfree(instants[0]);
		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one
			 * and compute the func at that point */
			TimestampTz inttime = interinstants[i]->t + 
				(interinstants[i + 1]->t - interinstants[i]->t) / 2;
			Datum intvalue = tsequence_value_at_timestamp1(inst1, inst2,
				linear, inttime);
			Datum intvalue1 = invert ? spatialrel(geo, intvalue, param, func, numparam) :
				spatialrel(intvalue, geo, param, func, numparam);
			instants[0] = tinstant_make(intvalue1, interinstants[i]->t, restypid);
			instants[1] = tinstant_make(intvalue1, interinstants[i + 1]->t, restypid);
			result[k++] = tsequence_make(instants, 2, false, false, STEP, NORMALIZE_NO);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(intvalue));
			DATUM_FREE(intvalue1, restypid);
		}
	}
	if (after)
	{
		Datum value = invert ? spatialrel(geo, value2, param, func, numparam) :
			spatialrel(value2, geo, param, func, numparam);
		instants[0] = tinstant_make(value, (interinstants[countinst - 1])->t,
			restypid);
		instants[1] = tinstant_make(value, inst2->t, restypid);
		result[k++] = tsequence_make(instants, 2, false, upper_inc, STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);

	*count = k;
	return result;
}

/**
 * Returns the temporal spatial relationship between a temporal sequence point
 * and a geometry.
 *
 * @param[in] seq Temporal point 
 * @param[in] geo Geometry
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting type
 * @param[out] count Number of elements in the resulting array
 * @param[in] invert True when the function is called with inverted arguments
 */
static TSequence **
tspatialrel_tpointseq_geo2(TSequence *seq, Datum geo, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TInstant *inst = tsequence_inst_n(seq, 0);
		Datum value = invert ? spatialrel(geo, tinstant_value(inst), param, func, numparam) :
			spatialrel(tinstant_value(inst), geo, param, func, numparam);
		TSequence **result = palloc(sizeof(TSequence *));
		TInstant *inst1 = tinstant_make(value, inst->t, restypid);
		result[0] = tinstant_to_tsequence(inst1, STEP);
		pfree(inst1);
		*count = 1;
		return result;
	}

	TSequence ***sequences = palloc(sizeof(TSequence *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * seq->count);
	int totalseqs = 0;
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TInstant *inst2 = tsequence_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel_tpointseq_geo1(inst1, inst2, linear, geo,
			param, lower_inc, upper_inc, func, numparam, restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}
	*count = totalseqs;
	return tsequencearr2_to_tsequencearr(sequences, countseqs, seq->count, 
		totalseqs);
}

/**
 * Returns the temporal spatial relationship between a temporal sequence point
 * and a geometry.
 *
 * @param[in] seq Temporal point 
 * @param[in] geo Geometry
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting type
 * @param[in] invert True when the function is called with inverted arguments
 */
static TSequenceSet *
tspatialrel_tpointseq_geo(TSequence *seq, Datum geo, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	int count;
	TSequence **sequences = tspatialrel_tpointseq_geo2(seq, geo, param,
		func, numparam, restypid, &count, invert);
	return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns the temporal spatial relationship between a temporal sequence point
 * and a geometry.
 *
 * @param[in] ts Temporal point 
 * @param[in] geo Geometry
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting type
 * @param[in] invert True when the function is called with inverted arguments
 */
static TSequenceSet *
tspatialrel_tpointseqset_geo(TSequenceSet *ts, Datum geo, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tspatialrel_tpointseq_geo(tsequenceset_seq_n(ts, 0), geo, param,
			func, numparam, restypid, invert);

	TSequence ***sequences = palloc(sizeof(TSequence *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		sequences[i] = tspatialrel_tpointseq_geo2(seq, geo, param, func,
			numparam, restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
	}
	TSequence **allsequences = tsequencearr2_to_tsequencearr(sequences,
		countseqs, ts->count, totalseqs);
	return tsequenceset_make_free(allsequences, totalseqs, NORMALIZE);
}

/**
 * Dispatch function for spatial relationships that accept a geometry 
 *
 * @param[in] temp Temporal point
 * @param[in] geo Geometry
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the function is called with inverted arguments
 */
static Temporal *
tspatialrel_tpoint_geo1(const Temporal *temp, Datum geo, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == INSTANT)
		result = (Temporal *)tfunc_tinstant_base((TInstant *)temp,
			geo, temp->valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == INSTANTSET)
		result = (Temporal *)tfunc_tinstantset_base((TInstantSet *)temp,
			geo, temp->valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == SEQUENCE)
		result = (Temporal *)tspatialrel_tpointseq_geo((TSequence *)temp,
			geo, param, func, numparam, restypid, invert);
	else /* temp->duration == SEQUENCESET */
		result = (Temporal *)tspatialrel_tpointseqset_geo( (TSequenceSet *)temp,
			geo, param, func, numparam, restypid, invert);
	return result;
}

/*****************************************************************************
 * Functions to compute the tdwithin relationship between a temporal sequence
 * and a geometry. These functions are not available for geographies nor for
 * 3D since they are based on the tpointseq_at_geometry1 function.
 * The functions use the st_dwithin function from PostGIS only for
 * instantaneous sequences.
 * This function is not available for geographies since it is based on the
 * function atGeometry.
 *****************************************************************************/

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence set point and the geometry are within the given distance
 *
 * @param[in] seq Temporal point
 * @param[in] geo Geometry
 * @param[in] dist Distance
 * @param[out] count Number of elements in the resulting array
 */
static TSequence **
tdwithin_tpointseq_geo1(TSequence *seq, Datum geo, Datum dist, int *count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TSequence **result = palloc(sizeof(TSequence *));
		Datum value = tinstant_value(tsequence_inst_n(seq, 0));
		Datum dwithin = geom_dwithin2d(value, geo, dist);
		TInstant *inst = tinstant_make(dwithin, seq->period.lower, BOOLOID);
		result[0] = tinstant_to_tsequence(inst, STEP);
		pfree(inst);
		*count = 1;
		return result;
	}

	/* Restrict to the buffered geometry */
	Datum geo_buffer = call_function2(buffer, geo, dist);
	int count1;
	TSequence **atbuffer = tpointseq_at_geometry2(seq, geo_buffer, &count1);
	Datum datum_true = BoolGetDatum(true);
	Datum datum_false = BoolGetDatum(false);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TInstant *instants[2];
	instants[0] = tinstant_make(datum_false, seq->period.lower, BOOLOID);
	instants[1] = tinstant_make(datum_false, seq->period.upper, BOOLOID);
	if (atbuffer == NULL)
	{
		TSequence **result = palloc(sizeof(TSequence *));
		/*  The two instant values created above are the ones needed here */
		result[0] = tsequence_make(instants, 2, seq->period.lower_inc,
			seq->period.upper_inc, STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}

	/* Get the periods during which the value is true */
	Period **periods = palloc(sizeof(Period *) * count1);
	for (int i = 0; i < count1; i++)
		periods[i] = &atbuffer[i]->period;
	/* The period set must be normalized */
	PeriodSet *ps = periodset_make(periods, count1, NORMALIZE);
	for (int i = 0; i < count1; i++)
		pfree(atbuffer[i]);
	pfree(periods);
	/* Get the periods during which the value is false */
	PeriodSet *minus = minus_period_periodset_internal(&seq->period, ps);
	if (minus == NULL)
	{
		TSequence **result = palloc(sizeof(TSequence *));
		tinstant_set(instants[0], datum_true, seq->period.lower);
		tinstant_set(instants[1], datum_true, seq->period.upper);
		result[0] = tsequence_make(instants, 2, seq->period.lower_inc,
			seq->period.upper_inc, STEP, NORMALIZE_NO);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}

	/* The original sequence will be split into ps->count + minus->count sequences
		|------------------------|
			  t		 t		t
			|---| |---|	|-----|
		 f		  f	   f	 f
		|---|   |-|   |-|	|-|
	*/
	*count = ps->count + minus->count;
	TSequence **result = palloc(sizeof(TSequence *) * *count);
	Period *p1 = periodset_per_n(ps, 0);
	Period *p2 = periodset_per_n(minus, 0);
	bool truevalue = period_cmp_internal(p1, p2) < 0;
	int j = 0, k = 0;
	for (int i = 0; i < *count; i++)
	{
		if (truevalue)
		{
			p1 = periodset_per_n(ps, j);
			tinstant_set(instants[0], datum_true, p1->lower);
			tinstant_set(instants[1], datum_true, p1->upper);
			result[i] = tsequence_make(instants, 2, p1->lower_inc,
				p1->upper_inc, STEP, NORMALIZE_NO);
			j++;
		}
		else
		{
			p2 = periodset_per_n(minus, k);
			tinstant_set(instants[0], datum_false, p2->lower);
			tinstant_set(instants[1], datum_false, p2->upper);
			result[i] = tsequence_make(instants, 2, p2->lower_inc,
				p2->upper_inc, STEP, NORMALIZE_NO);
			k++;
		}
		truevalue = ! truevalue;
	}
	pfree(instants[0]); pfree(instants[1]);
	pfree(ps); pfree(minus);
	return result;
}

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence point and the geometry are within the given distance
 */
static TSequenceSet *
tdwithin_tpointseq_geo(TSequence *seq, Datum geo, Datum dist)
{
	int count;
	TSequence **sequences = tdwithin_tpointseq_geo1(seq, geo, dist, &count);
	return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns a temporal Boolean that states at each instant whether the
 * temporal sequence set point and the geometry are within the given distance
 */
static TSequenceSet *
tdwithin_tpointseqset_geo(TSequenceSet *ts, Datum geo, Datum dist)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tdwithin_tpointseq_geo(tsequenceset_seq_n(ts, 0), geo, dist);

	TSequence ***sequences = palloc(sizeof(TSequence *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		sequences[i] = tdwithin_tpointseq_geo1(seq, geo, dist, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	TSequence **allsequences = tsequencearr2_to_tsequencearr(sequences,
		countseqs, ts->count, totalseqs);
	return tsequenceset_make_free(allsequences, totalseqs, NORMALIZE);
}

/*****************************************************************************
 * Functions to compute the tdwithin relationship between temporal sequences.
 * This requires to determine the instants t1 and t2 at which two temporal
 * periods have a distance d between each other. This amounts to solve the
 * equation
 * 		distance(seg1(t), seg2(t)) = d
 * The function assumes that the two segments are synchronized,
 * that they are not instants, and that they are not both constant.
 *
 * Possible cases
 *
 * Parallel (a == 0) within distance

SELECT tdwithin(
tgeompoint '[POINT(0 1)@2000-01-01, POINT(1 2)@2000-01-02]',
tgeompoint '[POINT(0 0)@2000-01-01, POINT(1 1)@2000-01-02]', 1)
-- "{[t@2000-01-01, t@2000-01-02]}"

  * Parallel (a == 0) but not within distance

SELECT tdwithin(
tgeompoint '[POINT(0 2)@2000-01-01, POINT(1 3)@2000-01-02]',
tgeompoint '[POINT(0 0)@2000-01-01, POINT(1 1)@2000-01-02]', 1)
-- "{[f@2000-01-01, f@2000-01-02]}"

 * No solution (root < 0)

SELECT tdwithin(
tgeompoint '[POINT(2 3)@2000-01-01, POINT(3 4)@2000-01-03]',
tgeompoint '[POINT(4 4)@2000-01-01, POINT(6 2)@2000-01-03]', 1)
-- "{[f@2000-01-01, f@2000-01-03]}"

 * One solution (root == 0)
   - solution within segment

SELECT tdwithin(
tgeompoint '[POINT(2 2)@2000-01-01, POINT(1 1)@2000-01-03]',
tgeompoint '[POINT(3 1)@2000-01-01, POINT(2 2)@2000-01-03]', 1)
-- "{[f@2000-01-01, t@2000-01-02], (f@2000-01-02, f@2000-01-03]}"

   - solution outside to segment

SELECT tdwithin(
tgeompoint '[POINT(3 3)@2000-01-01, POINT(2 2)@2000-01-03]',
tgeompoint '[POINT(4 0)@2000-01-01, POINT(3 1)@2000-01-03]', 1)
-- "{[f@2000-01-01, f@2000-01-03]}"

 * Two solutions (root > 0)
 - segments contains solution period

SELECT tdwithin(
tgeompoint '[POINT(1 1)@2000-01-01, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(1 3)@2000-01-01, POINT(5 3)@2000-01-05]', 1)
-- "{[f@2000-01-01, t@2000-01-02, t@2000-01-04], (f@2000-01-04, f@2000-01-05]}"

  - solution period contains segment

SELECT tdwithin(
tgeompoint '[POINT(2.5 2.5)@2000-01-02 12:00, POINT(3.5 3.5)@2000-01-05 12:00]',
tgeompoint '[POINT(2.5 3.0)@2000-01-02 12:00, POINT(3.5 3.0)@2000-01-03 12:00]', 1)
-- "{[t@2000-01-02 12:00:00+00, t@2000-01-03 12:00:00+00]}"

  - solution period overlaps to the left segment

SELECT tdwithin(
tgeompoint '[POINT(3 3)@2000-01-03, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(3 3)@2000-01-03, POINT(5 3)@2000-01-05]', 1)
-- "{[t@2000-01-03, f@2000-01-04, f@2000-01-05]}"

  - solution period overlaps to the right segment

SELECT tdwithin(
tgeompoint '[POINT(1 1)@2000-01-01, POINT(3 3)@2000-01-03]',
tgeompoint '[POINT(1 3)@2000-01-01, POINT(3 3)@2000-01-03]', 1)
-- "{[f@2000-01-01, t@2000-01-02, t@2000-01-03]}"

  - solution period intersects at an instant with the segment

SELECT tdwithin(
tgeompoint '[POINT(4 4)@2000-01-04, POINT(5 5)@2000-01-05]',
tgeompoint '[POINT(4 3)@2000-01-04, POINT(5 3)@2000-01-05]', 1)
-- "{[t@2000-01-04], (f@2000-01-04, f@2000-01-05]}"

 *****************************************************************************/

/**
 * Returns the timestamps at which the segments of the two temporal points 
 * are within the given distance
 *
 * @param[in] sv1,ev1 Points defining the first segment 
 * @param[in] sv2,ev2 Points defining the second segment 
 * @param[in] lower,upper Timestamps associated to the segments
 * @param[in] dist Distance
 * @param[in] hasz True for 3D segments
 * @param[in] func Distance function (2D or 3D)
 * @param[out] t1,t2 Resulting timestamps
 * @result Number of timestamps in the result, between 0 and 2. In the case
 * of a single result both t1 and t2 are set to the unique timestamp
 */
static int
tdwithin_tpointseq_tpointseq1(Datum sv1, Datum ev1, Datum sv2, Datum ev2,
	TimestampTz lower, TimestampTz upper, double dist, bool hasz,
	Datum (*func)(Datum, Datum, Datum), TimestampTz *t1, TimestampTz *t2)
{
	/* To reduce problems related to floating point arithmetic, lower and upper
	 * are shifted, respectively, to 0 and 1 before computing the solutions
	 * of the quadratic equation */
	double duration = upper - lower;
	long double a, b, c;
	if (hasz) /* 3D */
	{
		const POINT3DZ *p1 = datum_get_point3dz_p(sv1);
		const POINT3DZ *p2 = datum_get_point3dz_p(ev1);
		const POINT3DZ *p3 = datum_get_point3dz_p(sv2);
		const POINT3DZ *p4 = datum_get_point3dz_p(ev2);

		/* per1 functions
 		* x(t) = a1 * t + c1
 		* y(t) = a2 * t + c2
		* z(t) = a3 * t + c3 */
		double a1 = (p2->x - p1->x);
		double c1 = p1->x;
		double a2 = (p2->y - p1->y);
		double c2 = p1->y;
		double a3 = (p2->z - p1->z);
		double c3 = p1->z;

		/* per2 functions
		 * x(t) = a4 * t + c4
		 * y(t) = a5 * t + c5
		 * z(t) = a6 * t + c6 */
		double a4 = (p4->x - p3->x);
		double c4 = p3->x;
		double a5 = (p4->y - p3->y);
		double c5 = p3->y;
		double a6 = (p4->z - p3->z);
		double c6 = p3->z;

		/* compute the distance function */
		double a_x = (a1 - a4) * (a1 - a4);
		double a_y = (a2 - a5) * (a2 - a5);
		double a_z = (a3 - a6) * (a3 - a6);
		double b_x = 2 * (a1 - a4) * (c1 - c4);
		double b_y = 2 * (a2 - a5) * (c2 - c5);
		double b_z = 2 * (a3 - a6) * (c3 - c6);
		double c_x = (c1 - c4) * (c1 - c4);
		double c_y = (c2 - c5) * (c2 - c5);
		double c_z = (c3 - c6) * (c3 - c6);
		/* distance function = dist */
		a = a_x + a_y + a_z;
		b = b_x + b_y + b_z;
		c = c_x + c_y + c_z - (dist * dist);
	}
	else /* 2D */
	{
		const POINT2D *p1 = datum_get_point2d_p(sv1);
		const POINT2D *p2 = datum_get_point2d_p(ev1);
		const POINT2D *p3 = datum_get_point2d_p(sv2);
		const POINT2D *p4 = datum_get_point2d_p(ev2);
		/* per1 functions
		 * x(t) = a1 * t + c1
		 * y(t) = a2 * t + c2 */
		double a1 = (p2->x - p1->x);
		double c1 = p1->x;
		double a2 = (p2->y - p1->y);
		double c2 = p1->y;
		/* per2 functions
		 * x(t) = a3 * t + c3
		 * y(t) = a4 * t + c4 */
		double a3 = (p4->x - p3->x);
		double c3 = p3->x;
		double a4 = (p4->y - p3->y);
		double c4 = p3->y;
		/* compute the distance function */
		double a_x = (a1 - a3) * (a1 - a3);
		double a_y = (a2 - a4) * (a2 - a4);
		double b_x = 2 * (a1 - a3) * (c1 - c3);
		double b_y = 2 * (a2 - a4) * (c2 - c4);
		double c_x = (c1 - c3) * (c1 - c3);
		double c_y = (c2 - c4) * (c2 - c4);
		/* distance function = dist */
		a = a_x + a_y;
		b = b_x + b_y;
		c = c_x + c_y - (dist * dist);
	}
	/* They are parallel, moving in the same direction at the same speed */
	if (a == 0)
	{
		if (!func(sv1, sv2, Float8GetDatum(dist)))
			return 0;
		*t1 = lower;
		*t2 = upper;
		return 2;
	}
	/* Solving the quadratic equation for distance = dist */
	long double discriminant = b * b - 4 * a * c;

	/* One solution */
	if (discriminant == 0)
	{
		long double t5 = (-1 * b) / (2 * a);
		if (t5 < 0.0 || t5 > 1.0)
			return 0;
		*t1 = *t2 = lower + (long) (t5 * duration);
		return 1;
	}
	/* No solution */
	if (discriminant < 0)
		return 0;
	else
	/* At most two solutions depending on whether they are within the time interval */
	{
		/* Apply a mixture of quadratic formula and Viète formula to improve precision */
		long double t5, t6;
		if (b >= 0)
		{
			t5 = (-1 * b - sqrtl(discriminant)) / (2 * a);
			t6 = (2 * c ) / (-1 * b - sqrtl(discriminant));
		}
		else
		{
			t5 = (2 * c ) / (-1 * b + sqrtl(discriminant));
			t6 = (-1 * b + sqrtl(discriminant)) / (2 * a);
		}

		/* If the two intervals do not intersect */
		if (0.0 > t6 || t5 > 1.0)
			return 0;
		/* Compute the intersection of the two intervals */
		long double t7 = Max(0.0, t5);
		long double t8 = Min(1.0, t6);
		if (fabsl(t7 - t8) < EPSILON)
		{
			*t1 = *t2 = lower + (long) (t7 * duration);
			return 1;
		}
		else
		{
			*t1 = lower + (long) (t7 * duration);
			*t2 = lower + (long) (t8 * duration);
			return 2;
		}
	}
}

/**
 * Returns the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @result Number of elements in the resulting array
 * @pre The temporal points must be synchronized.
 */
static int
tdwithin_tpointseq_tpointseq2(TSequence **result, const TSequence *seq1,
	const TSequence *seq2, Datum dist, Datum (*func)(Datum, Datum, Datum))
{
	TInstant *start1 = tsequence_inst_n(seq1, 0);
	TInstant *start2 = tsequence_inst_n(seq2, 0);
	if (seq1->count == 1)
	{
		TInstant *inst = tinstant_make(func(tinstant_value(start1),
			tinstant_value(start2), dist), start1->t, BOOLOID);
		result[0] = tinstant_to_tsequence(inst, STEP);
		pfree(inst);
		return 1;
	}

	int k = 0;
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	bool hasz = MOBDB_FLAGS_GET_Z(seq1->flags);
	Datum sv1 = tinstant_value(start1);
	Datum sv2 = tinstant_value(start2);
	TimestampTz lower = start1->t;
	bool lower_inc = seq1->period.lower_inc;
	const Datum datum_true = BoolGetDatum(true);
	const Datum datum_false = BoolGetDatum(false);
	/* We create three temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TInstant *instants[3];
	instants[0] = tinstant_make(datum_true, lower, BOOLOID);
	instants[1] = tinstant_copy(instants[0]);
	instants[2] = tinstant_copy(instants[0]);
	for (int i = 1; i < seq1->count; i++)
	{
		/* Each iteration of the for loop adds between one and three sequences */
		TInstant *end1 = tsequence_inst_n(seq1, i);
		TInstant *end2 = tsequence_inst_n(seq2, i);
		Datum ev1 = tinstant_value(end1);
		Datum ev2 = tinstant_value(end2);
		TimestampTz upper = end1->t;
		bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

		/* Both segments are constant or have step interpolation */
		if ((datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2)) ||
			(! linear1 && ! linear2))
		{
			Datum value = func(sv1, sv2, dist);
			tinstant_set(instants[0], value, lower);
			if (! linear1 && ! linear2 && upper_inc)
			{
				Datum value1 = func(ev1, ev2, dist);
				tinstant_set(instants[1], value1, upper);
			}
			else
				tinstant_set(instants[1], value, upper);
			result[k++] = tsequence_make(instants, 2, lower_inc, upper_inc,
				STEP, NORMALIZE_NO);
		}
		/* General case */
		else
		{
			/* Find the instants t1 and t2 (if any) during which the dwithin function is true */
			TimestampTz t1, t2;
			Datum sev1 = linear1 ? ev1 : sv1;
			Datum sev2 = linear2 ? ev2 : sv2;
			int solutions = tdwithin_tpointseq_tpointseq1(sv1, sev1, sv2, sev2,
				lower, upper, DatumGetFloat8(dist), hasz, func, &t1, &t2);

			/* <  F  > */
			bool upper_inc1 = linear1 && linear2 && upper_inc;
			if (solutions == 0 ||
			(solutions == 1 && ((t1 == lower && !lower_inc) || 
				(t1 == upper && !upper_inc))))
			{
				tinstant_set(instants[0], datum_false, lower);
				tinstant_set(instants[1], datum_false, upper);
				result[k++] = tsequence_make(instants, 2, lower_inc,
					upper_inc1, STEP, NORMALIZE_NO);
			}
			/* 
			 *  <  T  >					2 solutions, lower == t1, upper == t2
			 *  [T](  F  )				1 solution, lower == t1 (t1 == t2)
			 *  [T  T](  F  )			2 solutions, lower == t1, upper != t2
			 *  (  F  )[T]				1 solution && upper == t1, (t1 == t2)
			 *  (  F  )[T](  F  )		1 solution, lower != t1 (t1 == t2)
			 *  (  F  )[T  T]			2 solutions, lower != t1, upper == t2
			 *  (  F  )[T  T](  F  )	2 solutions, lower != t1, upper != t2
			 */
			else 
			{
				int j = 0;
				if (t1 != lower)
					tinstant_set(instants[j++], datum_false, lower);
				tinstant_set(instants[j++], datum_true, t1);
				if (solutions == 2)
					tinstant_set(instants[j++], datum_true, t2);
				result[k++] = tsequence_make(instants, j, lower_inc,
					(t2 != upper) ? true : upper_inc1, STEP, NORMALIZE_NO);
				if (t2 != upper)
				{
					tinstant_set(instants[0], datum_false, t2);
					tinstant_set(instants[1], datum_false, upper);
					result[k++] = tsequence_make(instants, 2, false,
						upper_inc1, STEP, NORMALIZE_NO);
				}
			}
			/* Add extra final point if only one segment is linear */
			if (upper_inc && (! linear1 || ! linear2))
			{
				Datum value = func(ev1, ev2, dist);
				tinstant_set(instants[0], value, upper);
				result[k++] = tsequence_make(instants, 1, true, true,
					STEP, NORMALIZE_NO);
			}
		}
		sv1 = ev1;
		sv2 = ev2;
		lower = upper;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]); pfree(instants[2]);
	return k;
}

/**
 * Returns the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[in] seq1,seq2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseq_tpointseq(const TSequence *seq1, const TSequence *seq2,
	Datum dist, Datum (*func)(Datum, Datum, Datum))
{
	TSequence **sequences = palloc(sizeof(TSequence *) * seq1->count * 4);
	int count = tdwithin_tpointseq_tpointseq2(sequences, seq1, seq2, dist, func);
	return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns the timestamps at which the segments of two temporal points are
 * within the given distance
 *
 * @param[in] ts1,ts2 Temporal points
 * @param[in] dist Distance
 * @param[in] func DWithin function (2D or 3D)
 * @pre The temporal points must be synchronized.
 */
static TSequenceSet *
tdwithin_tpointseqset_tpointseqset(const TSequenceSet *ts1, const TSequenceSet *ts2, Datum dist,
	Datum (*func)(Datum, Datum, Datum))
{
	/* Singleton sequence set */
	if (ts1->count == 1)
		return tdwithin_tpointseq_tpointseq(tsequenceset_seq_n(ts1, 0),
			tsequenceset_seq_n(ts2, 0), dist, func);

	TSequence **sequences = palloc(sizeof(TSequence *) * ts1->totalcount * 4);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, i);
		k += tdwithin_tpointseq_tpointseq2(&sequences[k], seq1, seq2, dist,
			func);
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Generic temporal spatial relationship for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
static Datum
tspatialrel_geo_tpoint(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum),
	Oid restypid)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), (Datum) NULL,
		(varfunc) func, 2, restypid, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}
 
/**
 * Generic temporal spatial relationship for a geometry and a temporal point
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
static Datum
tspatialrel_tpoint_geo(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum),
	Oid restypid)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), (Datum) NULL,
		(varfunc) func, 2, restypid, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Generic temporal spatial relationship for temporal points
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
static Datum
tspatialrel_tpoint_tpoint(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum),
	Oid restypid)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	bool discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
		(varfunc) func, 2, restypid, STEP, discont, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcontains_geo_tpoint);
/**
 * Returns the temporal contains relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tcontains_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_contains, BOOLOID);
}

PG_FUNCTION_INFO_V1(tcontains_tpoint_geo);
/**
 * Returns the temporal contains relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tcontains_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_contains, BOOLOID);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcovers_geo_tpoint);
/**
 * Returns the temporal covers relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tcovers_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_covers, BOOLOID);
}

PG_FUNCTION_INFO_V1(tcovers_tpoint_geo);
/**
 * Returns the temporal covers relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tcovers_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_covers, BOOLOID);
}

/*****************************************************************************
 * Temporal coveredby
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcoveredby_geo_tpoint);
/**
 * Returns the temporal covered by relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tcoveredby_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_coveredby, BOOLOID);
}

PG_FUNCTION_INFO_V1(tcoveredby_tpoint_geo);
/**
 * Returns the temporal covered by relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tcoveredby_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_coveredby, BOOLOID);
}

/*****************************************************************************
 * Temporal disjoint
 * Since the ST_Disjoint function in PostGIS does not support 3D, we use
 * ST_3DIntersects and negate the result
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdisjoint_geo_tpoint);
/**
 * Returns the temporal disjoint relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *negresult = tintersects_tpoint_geo1(temp, gs);
	Temporal *result = tnot_tbool_internal(negresult);
	pfree(negresult);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tpoint_geo);
/**
 * Returns the temporal disjoint relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tdisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *negresult = tintersects_tpoint_geo1(temp, gs);
	Temporal *result = tnot_tbool_internal(negresult);
	pfree(negresult);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tpoint_tpoint);
/**
 * Returns the temporal disjoint relationship between the temporal points
 */
PGDLLEXPORT Datum
tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_tpoint(fcinfo, &datum2_point_ne, BOOLOID);
}

/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tequals_geo_tpoint);
/**
 * Returns the temporal equals relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tequals_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), (Datum) NULL,
		(varfunc) &datum2_point_eq, 2, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tequals_tpoint_geo);
/**
 * Returns the temporal equals relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tequals_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	ensure_point_type(gs);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), (Datum) NULL,
		(varfunc) &datum2_point_eq, 2, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tequals_tpoint_tpoint);
/**
 * Returns the temporal equals relationship between the temporal points
 */
PGDLLEXPORT Datum
tequals_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_tpoint(fcinfo, &datum2_point_eq, BOOLOID);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

/**
 * Returns the temporal intersects relationship between the temporal point 
 * and the geometry
 */
static Temporal *
tintersects_tpoint_geo1(Temporal *temp, GSERIALIZED *gs)
{
	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_intersects3d : &geom_intersects2d;
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), (Datum) NULL,
		(varfunc) func, 2, BOOLOID, false);
	return result;
}

PG_FUNCTION_INFO_V1(tintersects_geo_tpoint);
/**
 * Returns the temporal intersects relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
tintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tintersects_tpoint_geo1(temp, gs);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tpoint_geo);
/**
 * Returns the temporal intersects relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
tintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tintersects_tpoint_geo1(temp, gs);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tpoint_tpoint);
/**
 * Returns the temporal intersects relationship between the temporal points
 */
PGDLLEXPORT Datum
tintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Datum (*func)(Datum, Datum);
	if (MOBDB_FLAGS_GET_GEODETIC(temp1->flags))
		func = &geog_intersects;
	else
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &geom_intersects3d :
			&geom_intersects2d;
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	bool discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
		(varfunc) func, 2, BOOLOID, STEP, discont, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(ttouches_geo_tpoint);
/**
 * Returns the temporal touches relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
ttouches_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_touches, BOOLOID);
}

PG_FUNCTION_INFO_V1(ttouches_tpoint_geo);
/**
 * Returns the temporal touches relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
ttouches_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_touches, BOOLOID);
}

/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(twithin_geo_tpoint);
/**
 * Returns the temporal within relationship between the geometry and the
 * temporal point
 */
PGDLLEXPORT Datum
twithin_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_within, BOOLOID);
}

PG_FUNCTION_INFO_V1(twithin_tpoint_geo);
/**
 * Returns the temporal within relationship between the temporal point
 * and the geometry 
 */
PGDLLEXPORT Datum
twithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_within, BOOLOID);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/

/**
 * Returns a temporal Boolean that states whether the temporal point and
 * the geometry are within the given distance (dispatch function)
 */
static Temporal *
tdwithin_tpoint_geo_internal(const Temporal *temp, GSERIALIZED *gs, Datum dist)
{
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_same_dimensionality_tpoint_gs(temp, gs);
	Datum (*func)(Datum, Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_dwithin3d : &geom_dwithin2d;
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == INSTANT)
		result = (Temporal *)tfunc_tinstant_base((TInstant *)temp,
			PointerGetDatum(gs), temp->valuetypid, dist, (varfunc) func, 3, BOOLOID, false);
	else if (temp->duration == INSTANTSET)
		result = (Temporal *)tfunc_tinstantset_base((TInstantSet *)temp,
			PointerGetDatum(gs), temp->valuetypid, dist, (varfunc) func, 3, BOOLOID, false);
	else if (temp->duration == SEQUENCE)
		result = (Temporal *)tdwithin_tpointseq_geo((TSequence *)temp,
				PointerGetDatum(gs), dist);
	else /* temp->duration == SEQUENCESET */
		result = (Temporal *)tdwithin_tpointseqset_geo((TSequenceSet *)temp,
				PointerGetDatum(gs), dist);
	return result;
}

PG_FUNCTION_INFO_V1(tdwithin_geo_tpoint);
/**
 * Returns a temporal Boolean that states whether the geometry and the
 * temporal point are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_geo);
/**
 * Returns a temporal Boolean that states whether the temporal point and 
 * the geometry are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
 
/**
 * Returns a temporal Boolean that states whether the temporal points
 * are within the given distance (internal function)
 */
Temporal *
tdwithin_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2,
	Datum dist)
{
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	 * The operation is synchronization without adding crossings */
	if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE,
		&sync1, &sync2))
		return NULL;

	Datum (*func)(Datum, Datum, Datum);
	if (MOBDB_FLAGS_GET_GEODETIC(temp1->flags))
		func = &geog_dwithin;
	else
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &geom_dwithin3d :
			&geom_dwithin2d;

	Temporal *result;
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == INSTANT)
		result = (Temporal *)sync_tfunc_tinstant_tinstant(
			(TInstant *)sync1, (TInstant *)sync2, dist, (varfunc) func, 3,
			BOOLOID);
	else if (sync1->duration == INSTANTSET)
		result = (Temporal *)sync_tfunc_tinstantset_tinstantset(
			(TInstantSet *)sync1, (TInstantSet *)sync2, dist, (varfunc) func, 3,
			BOOLOID);
	else if (sync1->duration == SEQUENCE)
		result = (Temporal *)tdwithin_tpointseq_tpointseq(
			(TSequence *)sync1, (TSequence *)sync2, dist, func);
	else /* sync1->duration == SEQUENCESET */
		result = (Temporal *)tdwithin_tpointseqset_tpointseqset(
			(TSequenceSet *)sync1, (TSequenceSet *)sync2, dist, func);

	pfree(sync1); pfree(sync2);
	return result;
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_tpoint);
/**
 * Returns a temporal Boolean that states whether the temporal points
 * are within the given distance
 */
PGDLLEXPORT Datum
tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	/* Store fcinfo into a global variable */
	store_fcinfo(fcinfo);
	Temporal *result = tdwithin_tpoint_tpoint_internal(temp1, temp2, dist);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal relate
 *****************************************************************************/

PG_FUNCTION_INFO_V1(trelate_geo_tpoint);
/**
 * Returns a temporal text value that states the DE-9IM matrix pattern
 * between the geometry and the temporal point
 */
PGDLLEXPORT Datum
trelate_geo_tpoint(PG_FUNCTION_ARGS)
{
	return tspatialrel_geo_tpoint(fcinfo, &geom_relate, TEXTOID);
}

PG_FUNCTION_INFO_V1(trelate_tpoint_geo);
/**
 * Returns a temporal text value that states the DE-9IM matrix pattern
 * between the temporal point and the geometry
 */
PGDLLEXPORT Datum
trelate_tpoint_geo(PG_FUNCTION_ARGS)
{
	return tspatialrel_tpoint_geo(fcinfo, &geom_relate, TEXTOID);
}

PG_FUNCTION_INFO_V1(trelate_tpoint_tpoint);
/**
 * Returns a temporal text value that states the DE-9IM matrix pattern
 * between the temporal points 
 */
PGDLLEXPORT Datum
trelate_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_has_not_Z_tpoint(temp1);
	ensure_has_not_Z_tpoint(temp2);
	bool discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
		(varfunc) &geom_relate, 2, TEXTOID, STEP, discont, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal relate_pattern
 *****************************************************************************/

PG_FUNCTION_INFO_V1(trelate_pattern_geo_tpoint);
/**
 * Returns a temporal Boolean that states whether the geometry and the
 * temporal points satisfy the DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
trelate_pattern_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), pattern,
		(varfunc) &geom_relate_pattern, 3, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tpoint_geo);
/**
 * Returns a temporal Boolean that states whether the temporal point and
 * the geometry satisfy the DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
trelate_pattern_tpoint_geo(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	if (gserialized_is_empty(gs))
		PG_RETURN_NULL();
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	Temporal *result = tspatialrel_tpoint_geo1(temp, PointerGetDatum(gs), pattern,
		(varfunc) &geom_relate_pattern, 3, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tpoint_tpoint);
/**
 * Returns a temporal Boolean that states whether the temporal points satisfy
 * the DE-9IM matrix pattern
 */
PGDLLEXPORT Datum
trelate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_has_not_Z_tpoint(temp1);
	ensure_has_not_Z_tpoint(temp2);
	bool discont = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, pattern, 
		(varfunc) &geom_relate_pattern, 3, BOOLOID, STEP, discont, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
