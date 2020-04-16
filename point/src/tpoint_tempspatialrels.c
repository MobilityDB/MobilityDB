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
 *    arguments)
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
#include "temporal_boolops.h"
#include "lifting.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_spatialrels.h"

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

/* Get the temporal instants at which a segment of a temporal sequence
 * intersects a line */

static TemporalInst **
tpointseq_intersection_instants(const TemporalInst *inst1, const TemporalInst *inst2,
	bool lower_inc, bool upper_inc, Datum inter, int *count)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);

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
	POINT4D start = datum_get_point4d(value1);
	POINT4D end = datum_get_point4d(value2);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 2 * countinter);
	long double duration = (long double)(inst2->t - inst1->t);
	int k = 0;
	for (int i = 0; i < countinter; i++)
	{
		if (coll != NULL)
			/* Find the i-th intersection */
			lwinter_single = coll->geoms[i];
		POINT4D p1, p2;
		double fraction1, fraction2;
		TimestampTz t1, t2;
		Datum point1, point2;
		/* Each intersection is either a point or a linestring with two points */
		if (lwinter_single->type == POINTTYPE)
		{
			lwpoint_getPoint4d_p((LWPOINT *) lwinter_single, &p1);
			fraction1 = closest_point_on_segment_ratio(&p1, &start, &end);
			t1 = inst1->t + (long) (duration * fraction1);
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || t1 != inst1->t) && (upper_inc || t1 != inst2->t))
			{
				point1 = temporalseq_value_at_timestamp1(inst1, inst2, true, t1);
				instants[k++] = temporalinst_make(point1, t1, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point1));
			}
		}
		else /* lwinter_single->type == LINETYPE) */
		{
			LWPOINT *lwpoint1 = lwline_get_lwpoint((LWLINE *) lwinter_single, 0);
			LWPOINT *lwpoint2 = lwline_get_lwpoint((LWLINE *) lwinter_single, 1);
			lwpoint_getPoint4d_p(lwpoint1, &p1);
			lwpoint_getPoint4d_p(lwpoint2, &p2);
			fraction1 = closest_point_on_segment_ratio(&p1, &start, &end);
			fraction2 = closest_point_on_segment_ratio(&p2, &start, &end);
			t1 = inst1->t + (long) (duration * fraction1);
			t2 = inst1->t + (long) (duration * fraction2);
			TimestampTz lower = Min(t1, t2);
			TimestampTz upper = Max(t1, t2);
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || t1 != lower) && (upper_inc || t2 != lower))
			{
				point1 = temporalseq_value_at_timestamp1(inst1, inst2, true, lower);
				instants[k++] = temporalinst_make(point1, lower, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point1));
			}
			/* If the point intersection is not at an exclusive bound and
			 * lower != upper (this last condition arrives when point1 is
			 * at an epsilon distance from point2 */
			if ((lower_inc || t1 != upper) &&
				(upper_inc || t2 != upper) && (lower != upper))
			{
				point2 = temporalseq_value_at_timestamp1(inst1, inst2, true, upper);
				instants[k++] = temporalinst_make(point2, upper, type_oid(T_GEOMETRY));
				pfree(DatumGetPointer(point2));
			}
		}
	}
	/* Sort the instants */
	temporalinstarr_sort(instants, k);
	POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(gsinter));
	*count = k;
	return instants;
}

/*****************************************************************************
 * Generic functions to compute the temporal spatial relationship
 * between a geometry and a temporal sequence.
 * The potential crossings between the two are considered.
 * The resulting sequence (set) has step interpolation since it is a
 * temporal Boolean or a temporal text (for trelate).
 * These functions are not available for geographies since they call the
 * intersection function in PostGIS that is only available for geometries.
 *****************************************************************************/

static TemporalSeq **
tspatialrel_tpointseq_geo1(const TemporalInst *inst1, const TemporalInst *inst2,
	bool linear, Datum geo, bool lower_inc, bool upper_inc,
	Datum (*func)(Datum, Datum), Oid restypid, int *count, bool invert)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	/* Constant segment or step interpolation */
	if (datum_point_eq(value1, value2) || ! linear)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		Datum value = invert ? func(geo, value1) : func(value1, geo);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
		*count = 1;
		return result;
	}

	/* Look for intersections */
	Datum line = geompoint_trajectory(value1, value2);
	Datum intersections = call_function2(intersection, line, geo);
	if (call_function1(LWGEOM_isempty, intersections))
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		Datum value = invert ? func(geo, value1) : func(value1, geo);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections));
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
		*count = 1;
		return result;
	}

	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tpointseq_intersection_instants(inst1,
		inst2, lower_inc, upper_inc, intersections, &countinst);
	pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections));

	/* No intersections were found */
	if (countinst == 0)
	{
		/* There may be an intersection at an exclusive bound.
		 * Find the middle time between inst1 and inst2
		 * and compute the func at that point */
		TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
		Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, linear, inttime);
		Datum intvalue1 = invert ? func(geo, intvalue) : func(intvalue, geo);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		instants[0] = temporalinst_make(intvalue1, inst1->t, restypid);
		instants[1] = temporalinst_make(intvalue1, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
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
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countseq);
	TemporalInst *instants[2];
	int k = 0;
	if (before)
	{
		Datum value = invert ? func(geo, value1) : func(value1, geo);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t,
			restypid);
		result[k++] = temporalseq_make(instants, 2, lower_inc, false,
			false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}
	for (int i = 0; i < countinst; i++)
	{
		/* Compute the value at the intersection point */
		Datum value = invert ? func(geo, temporalinst_value(interinstants[i])) :
			func(temporalinst_value(interinstants[i]), geo);
		instants[0] = temporalinst_make(value, (interinstants[i])->t,
			restypid);
		result[k++] = temporalseq_make(instants, 1, true, true, false, false);
		DATUM_FREE(value, restypid);
		pfree(instants[0]);
		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one
			 * and compute the func at that point */
			TimestampTz inttime = interinstants[i]->t +
				(interinstants[i + 1]->t - interinstants[i]->t) / 2;
			Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2,
				linear, inttime);
			Datum intvalue1 = invert ? func(geo, intvalue) :
				func(intvalue, geo);
			instants[0] = temporalinst_make(intvalue1, interinstants[i]->t, restypid);
			instants[1] = temporalinst_make(intvalue1, interinstants[i + 1]->t, restypid);
			result[k++] = temporalseq_make(instants, 2, false, false, false, false);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(intvalue));
			DATUM_FREE(intvalue1, restypid);
		}
	}
	if (after)
	{
		Datum value = invert ? func(geo, value2) : func(value2, geo);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t,
			restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[k++] = temporalseq_make(instants, 2, false, upper_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);

	*count = k;
	return result;
}

static TemporalSeq **
tspatialrel_tpointseq_geo2(TemporalSeq *seq, Datum geo,
	Datum (*func)(Datum, Datum), Oid restypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value = invert ? func(geo, temporalinst_value(inst)) :
			func(temporalinst_value(inst), geo);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalinst_make(value, inst->t, restypid);
		result[0] = temporalseq_make(&inst1, 1, true, true, false, false);
		pfree(inst1);
		*count = 1;
		return result;
	}

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * seq->count);
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel_tpointseq_geo1(inst1, inst2, linear, geo,
			lower_inc, upper_inc, func, restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}

	*count = totalseqs;
	return result;
}

static TemporalS *
tspatialrel_tpointseq_geo(TemporalSeq *seq, Datum geo,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel_tpointseq_geo2(seq, geo,
		func, restypid, &count, invert);
	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tspatialrel_tpoints_geo(TemporalS *ts, Datum geo,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tspatialrel_tpointseq_geo(temporals_seq_n(ts, 0), geo,
			func, restypid, invert);

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel_tpointseq_geo2(seq, geo, func,
			restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_make(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************/

static TemporalSeq **
tspatialrel3_tpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2,
	bool linear, Datum geo, Datum param, bool lower_inc, bool upper_inc,
	Datum (*func)(Datum, Datum, Datum), Oid restypid, int *count, bool invert)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	TemporalInst *instants[2];
	/* Constant segment or step interpolation */
	if (datum_point_eq(value1, value2) || ! linear)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value = invert ? func(geo, value1, param) :
			func(value1, geo, param);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
		*count = 1;
		return result;
	}

	/* Look for intersections */
	Datum line =  geompoint_trajectory(value1, value2);
	Datum intersections = call_function2(intersection, line, geo);
	if (call_function1(LWGEOM_isempty, intersections))
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value = invert ? func(geo, value1, param) :
			func(value1, geo, param);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections));
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
		*count = 1;
		return result;
	}

	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tpointseq_intersection_instants(inst1,
		inst2, lower_inc, upper_inc, intersections, &countinst);
	pfree(DatumGetPointer(intersections));
	pfree(DatumGetPointer(line));

	/* No intersections were found */
	if (countinst == 0)
	{
		/* There may be an intersection at an exclusive bound.
		 * Find the middle time between inst1 and inst2
		 * and compute the func at that point */
		TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
		Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2,
			linear, inttime);
		Datum intvalue1 = invert ? func(geo, intvalue, param) :
			func(intvalue, geo, param);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		instants[0] = temporalinst_make(intvalue1, inst1->t, restypid);
		instants[1] = temporalinst_make(intvalue1, inst2->t, restypid);
		result[0] = temporalseq_make(instants, 2, lower_inc, upper_inc,
			false, false);
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
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countseq);
	int k = 0;
	if (before)
	{
		Datum value = invert ? func(geo, value1, param) :
			func(value1, geo, param);
		instants[0] = temporalinst_make(value, inst1->t, restypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t, restypid);
		result[k++] = temporalseq_make(instants, 2, lower_inc, false,
			false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}
	for (int i = 0; i < countinst; i++)
	{
		/* Compute the value at the intersection point */
		Datum value = invert ?
			func(geo, temporalinst_value(interinstants[i]), param) :
			func(temporalinst_value(interinstants[i]), geo, param);
		instants[0] = temporalinst_make(value, (interinstants[i])->t,
			restypid);
		result[k++] = temporalseq_make(instants, 1, true, true, false, false);
		DATUM_FREE(value, restypid);
		pfree(instants[0]);
		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one
			 * and compute the func at that point */
			TimestampTz inttime = interinstants[i]->t + (interinstants[i + 1]->t - interinstants[i]->t) / 2;
			Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2,
				linear, inttime);
			Datum intvalue1 = invert ? func(geo, intvalue, param) :
				func(intvalue, geo, param);
			instants[0] = temporalinst_make(intvalue1, interinstants[i]->t, restypid);
			instants[1] = temporalinst_make(intvalue1, interinstants[i + 1]->t, restypid);
			result[k++] = temporalseq_make(instants, 2, false, false, false, false);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(intvalue));
			DATUM_FREE(intvalue1, restypid);
		}
	}
	if (after)
	{
		Datum value = invert ? func(geo, value2, param) :
			func(value2, geo, param);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t,
			restypid);
		instants[1] = temporalinst_make(value, inst2->t, restypid);
		result[k++] = temporalseq_make(instants, 2, false, upper_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		DATUM_FREE(value, restypid);
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);

	*count = k;
	return result;
}

static TemporalSeq **
tspatialrel3_tpointseq_geo2(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid restypid,
	int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value = invert ? func(geo, temporalinst_value(inst), param) :
			func(temporalinst_value(inst), geo, param);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalinst_make(value, inst->t, restypid);
		result[0] = temporalseq_make(&inst1, 1, true, true, false, false);
		pfree(inst1);
		*count = 1;
		return result;
	}

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * seq->count);
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel3_tpointseq_geo1(inst1, inst2, linear, geo,
			param, lower_inc, upper_inc, func, restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (countseqs[i] != 0 && i < seq->count - 1)
			pfree(sequences[i]);
	}

	*count = totalseqs;
	return result;
}

static TemporalS *
tspatialrel3_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel3_tpointseq_geo2(seq, geo, param,
		func, restypid, &count, invert);
	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tspatialrel3_tpoints_geo(TemporalS *ts, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tspatialrel3_tpointseq_geo(temporals_seq_n(ts, 0), geo, param,
			func, restypid, invert);

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel3_tpointseq_geo2(seq, geo, param, func,
			restypid, &countseqs[i], invert);
		totalseqs += countseqs[i];
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_make(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

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

static TemporalSeq **
tdwithin_tpointseq_geo1(TemporalSeq *seq, Datum geo, Datum dist, int *count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value = temporalinst_value(temporalseq_inst_n(seq, 0));
		Datum dwithin = geom_dwithin2d(value, geo, dist);
		TemporalInst *inst = temporalinst_make(dwithin, seq->period.lower,
			BOOLOID);
		result[0] = temporalseq_make(&inst, 1, true, true, false, false);
		pfree(inst);
		*count = 1;
		return result;
	}

	/* Restrict to the buffered geometry */
	Datum geo_buffer = call_function2(buffer, geo, dist);
	int count1;
	TemporalSeq **atbuffer = tpointseq_at_geometry2(seq, geo_buffer, &count1);
	Datum datum_true = BoolGetDatum(true);
	Datum datum_false = BoolGetDatum(false);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(datum_false, seq->period.lower, BOOLOID);
	instants[1] = temporalinst_make(datum_false, seq->period.upper, BOOLOID);
	if (atbuffer == NULL)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		/*  The two instant values created above are the ones needed here */
		result[0] = temporalseq_make(instants, 2, seq->period.lower_inc,
			seq->period.upper_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}

	/* Get the periods during which the value is true */
	Period **periods = palloc(sizeof(Period *) * count1);
	for (int i = 0; i < count1; i++)
		periods[i] = &atbuffer[i]->period;
	/* The period set must be normalized, i.e., last parameter must be true */
	PeriodSet *ps = periodset_make_internal(periods, count1, true);
	for (int i = 0; i < count1; i++)
		pfree(atbuffer[i]);
	pfree(periods);
	/* Get the periods during which the value is false */
	PeriodSet *minus = minus_period_periodset_internal(&seq->period, ps);
	if (minus == NULL)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		temporalinst_set(instants[0], datum_true, seq->period.lower);
		temporalinst_set(instants[1], datum_true, seq->period.upper);
		result[0] = temporalseq_make(instants, 2, seq->period.lower_inc,
			seq->period.upper_inc, false, false);
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
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * *count);
	Period *p1 = periodset_per_n(ps, 0);
	Period *p2 = periodset_per_n(minus, 0);
	bool truevalue = period_cmp_internal(p1, p2) < 0;
	int j = 0, k = 0;
	for (int i = 0; i < *count; i++)
	{
		if (truevalue)
		{
			p1 = periodset_per_n(ps, j);
			temporalinst_set(instants[0], datum_true, p1->lower);
			temporalinst_set(instants[1], datum_true, p1->upper);
			result[i] = temporalseq_make(instants, 2, p1->lower_inc,
				p1->upper_inc, false, false);
			j++;
		}
		else
		{
			p2 = periodset_per_n(minus, k);
			temporalinst_set(instants[0], datum_false, p2->lower);
			temporalinst_set(instants[1], datum_false, p2->upper);
			result[i] = temporalseq_make(instants, 2, p2->lower_inc,
				p2->upper_inc, false, false);
			k++;
		}
		truevalue = ! truevalue;
	}
	pfree(instants[0]); pfree(instants[1]);
	pfree(ps); pfree(minus);
	return result;
}

static TemporalS *
tdwithin_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum dist)
{
	int count;
	TemporalSeq **sequences = tdwithin_tpointseq_geo1(seq, geo, dist, &count);
	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tdwithin_tpoints_geo(TemporalS *ts, Datum geo, Datum dist)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tdwithin_tpointseq_geo(temporals_seq_n(ts, 0), geo, dist);

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tdwithin_tpointseq_geo1(seq, geo, dist, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (countseqs[i] != 0)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_make(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

Temporal *
tdwithin_tpoint_geo_internal(const Temporal *temp, GSERIALIZED *gs, Datum dist)
{
	Datum (*func)(Datum, Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_dwithin3d : &geom_dwithin2d;
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc3_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), dist, func, BOOLOID, false);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc3_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), dist, func, BOOLOID, false);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tdwithin_tpointseq_geo((TemporalSeq *)temp,
				PointerGetDatum(gs), dist);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tdwithin_tpoints_geo((TemporalS *)temp,
				PointerGetDatum(gs), dist);
	return result;
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

static int
tdwithin_tpointseq_tpointseq1(Datum sv1, Datum ev1, Datum sv2, Datum ev2,
	TimestampTz lower, TimestampTz upper, double dist, bool hasz,
	Datum (*func)(Datum, Datum, Datum), TimestampTz *t1, TimestampTz *t2)
{
	/* To reduce problems related to floating point arithmetic, lower and upper
	   are shifted, respectively, to 0 and 1 before computing the solutions
	   of the quadratic equation */
	double duration = upper - lower;
	long double a, b, c;
	if (hasz) /* 3D */
	{
		POINT3DZ p1 = datum_get_point3dz(sv1);
		POINT3DZ p2 = datum_get_point3dz(ev1);
		POINT3DZ p3 = datum_get_point3dz(sv2);
		POINT3DZ p4 = datum_get_point3dz(ev2);

		/* per1 functions
 		* x(t) = a1 * t + c1
 		* y(t) = a2 * t + c2
		* z(t) = a3 * t + c3 */
		double a1 = (p2.x - p1.x);
		double c1 = p1.x;
		double a2 = (p2.y - p1.y);
		double c2 = p1.y;
		double a3 = (p2.z - p1.z);
		double c3 = p1.z;

		/* per2 functions
		 * x(t) = a4 * t + c4
		 * y(t) = a5 * t + c5
		 * z(t) = a6 * t + c6 */
		double a4 = (p4.x - p3.x);
		double c4 = p3.x;
		double a5 = (p4.y - p3.y);
		double c5 = p3.y;
		double a6 = (p4.z - p3.z);
		double c6 = p3.z;

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
		POINT2D p1 = datum_get_point2d(sv1);
		POINT2D p2 = datum_get_point2d(ev1);
		POINT2D p3 = datum_get_point2d(sv2);
		POINT2D p4 = datum_get_point2d(ev2);
		/* per1 functions
		 * x(t) = a1 * t + c1
		 * y(t) = a2 * t + c2 */
		double a1 = (p2.x - p1.x);
		double c1 = p1.x;
		double a2 = (p2.y - p1.y);
		double c2 = p1.y;
		/* per2 functions
		 * x(t) = a3 * t + c3
		 * y(t) = a4 * t + c4 */
		double a3 = (p4.x - p3.x);
		double c3 = p3.x;
		double a4 = (p4.y - p3.y);
		double c4 = p3.y;
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
		*t1 = lower + (long) (t5 * duration);
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
			*t1 = lower + (long) (t7 * duration);
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

/* The following function supposes that the two temporal values are synchronized.
   This should be ensured by the calling function. */

static int
tdwithin_tpointseq_tpointseq2(TemporalSeq **result, const TemporalSeq *seq1,
	const TemporalSeq *seq2, Datum dist, Datum (*func)(Datum, Datum, Datum))
{
	if (seq1->count == 1)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		TemporalInst *inst = temporalinst_make(func(temporalinst_value(inst1),
			temporalinst_value(inst2), dist), inst1->t, BOOLOID);
		result[0] = temporalseq_make(&inst, 1, true, true, false, false);
		pfree(inst);
		return 1;
	}

	int k = 0;
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	bool hasz = MOBDB_FLAGS_GET_Z(seq1->flags);
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	Datum sv1 = temporalinst_value(start1);
	Datum sv2 = temporalinst_value(start2);
	TimestampTz lower = start1->t;
	bool lower_inc = seq1->period.lower_inc;
	const Datum datum_true = BoolGetDatum(true);
	const Datum datum_false = BoolGetDatum(false);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(datum_true, lower, BOOLOID);
	instants[1] = temporalinst_copy(instants[0]);
	for (int i = 1; i < seq1->count; i++)
	{
		/* Each iteration of the for loop adds between one and three sequences */
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		Datum ev1 = temporalinst_value(end1);
		Datum ev2 = temporalinst_value(end2);
		TimestampTz upper = end1->t;
		bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;

		/* Both segments are constant */
		if (datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2))
		{
			Datum value = func(sv1, sv2, dist);
			temporalinst_set(instants[0], value, lower);
			temporalinst_set(instants[1], value, upper);
			result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc,
				false, false);
		}
		/* Both segments have step interpolation */
		else if (! linear1 && ! linear2)
		{
			Datum value = func(sv1, sv2, dist);
			temporalinst_set(instants[0], value, lower);
			temporalinst_set(instants[1], value, upper);
			result[k++] = temporalseq_make(instants, 2, lower_inc, false,
				false, false);
			if (upper_inc)
			{
				value = func(ev1, ev2, dist);
				temporalinst_set(instants[0], value, upper);
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
			}
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

			/* No instant is returned */
			bool upper_inc1 = linear1 && linear2 && upper_inc;
			if (solutions == 0)
			{
				temporalinst_set(instants[0], datum_false, lower);
				temporalinst_set(instants[1], datum_false, upper);
				result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc1,
					false, false);
			}
			/* A single instant is returned */
			else if (solutions == 1)
			{
				if ((t1 == lower && !lower_inc) || (t1 == upper && !upper_inc))
				{
					temporalinst_set(instants[0], datum_false, lower);
					temporalinst_set(instants[1], datum_false, upper);
					result[k++] = temporalseq_make(instants, 2, lower_inc,
						upper_inc1, false, false);
				}
				else if (t1 == lower) /* && lower_inc */
				{
					temporalinst_set(instants[0], datum_true, lower);
					result[k++] = temporalseq_make(instants, 1, true, true,
						false, false);
					temporalinst_set(instants[0], datum_false, lower);
					temporalinst_set(instants[1], datum_false, upper);
					result[k++] = temporalseq_make(instants, 2, false,
						upper_inc1, false, false);
				}
				else if (t1 == upper) /* && upper_inc */
				{
					temporalinst_set(instants[0], datum_false, lower);
					temporalinst_set(instants[1], upper_inc1 ? datum_true :
						datum_false, upper);
					result[k++] = temporalseq_make(instants, 2, lower_inc,
						upper_inc1, false, false);
				}
				else /* (t1 != lower && t1 != upper) */
				{
					temporalinst_set(instants[0], datum_false, lower);
					temporalinst_set(instants[1], datum_false, t1);
					result[k++] = temporalseq_make(instants, 2, lower_inc,
						false, false, false);
					temporalinst_set(instants[0], datum_true, t1);
					result[k++] = temporalseq_make(instants, 1, true, true,
						false, false);
					temporalinst_set(instants[0], datum_false, t1);
					temporalinst_set(instants[1], datum_false, upper);
					result[k++] = temporalseq_make(instants, 2, false,
						upper_inc1, false, false);
				}
			}
			/* solutions == 2, i.e., two instants are returned */
			else if (lower == t1 && upper == t2)
			{
				temporalinst_set(instants[0], datum_true, lower);
				temporalinst_set(instants[1], datum_true, upper);
				result[k++] = temporalseq_make(instants, 2,
					lower_inc, upper_inc1, false, false);
			}
			else if (lower != t1 && upper == t2)
			{
				temporalinst_set(instants[0], datum_false, lower);
				temporalinst_set(instants[1], datum_false, t1);
				result[k++] = temporalseq_make(instants, 2, lower_inc,
					false, false, false);
				temporalinst_set(instants[0], datum_true, t1);
				temporalinst_set(instants[1], datum_true, upper);
				result[k++] = temporalseq_make(instants, 2, true,
					upper_inc1, false, false);
			}
			else if (lower == t1 && upper != t2)
			{
				temporalinst_set(instants[0], datum_true, lower);
				temporalinst_set(instants[1], datum_true, t2);
				result[k++] = temporalseq_make(instants, 2, lower_inc,
					false, false, false);
				temporalinst_set(instants[0], datum_false, t2);
				temporalinst_set(instants[1], datum_false, upper);
				result[k++] = temporalseq_make(instants, 2, true,
					upper_inc1, false, false);
			}
			else
			{
				temporalinst_set(instants[0], datum_false, lower);
				temporalinst_set(instants[1], datum_false, t1);
				result[k++] = temporalseq_make(instants, 2, lower_inc,
					false, false, false);
				temporalinst_set(instants[0], datum_true, t1);
				temporalinst_set(instants[1], datum_true, t2);
				result[k++] = temporalseq_make(instants, 2, true, true,
					false, false);
				temporalinst_set(instants[0], datum_false, t2);
				temporalinst_set(instants[1], datum_false, upper);
				result[k++] = temporalseq_make(instants, 2, false,
					upper_inc1, false, false);
			}
			/* Add extra final point if only one segment is linear */
			if (upper_inc && (! linear1 || ! linear2))
			{
				Datum value = func(ev1, ev2, dist);
				temporalinst_set(instants[0], value, upper);
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
			}
		}
		sv1 = ev1;
		sv2 = ev2;
		lower = upper;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	return k;
}

static TemporalS *
tdwithin_tpointseq_tpointseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum dist, Datum (*func)(Datum, Datum, Datum))
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq1->count * 4);
	int count = tdwithin_tpointseq_tpointseq2(sequences, seq1, seq2, dist, func);
	TemporalS *result = temporals_make(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

/* The following function supposes that the two temporal values are synchronized.
   This should be ensured by the calling function. */

static TemporalS *
tdwithin_tpoints_tpoints(const TemporalS *ts1, const TemporalS *ts2, Datum dist,
	Datum (*func)(Datum, Datum, Datum))
{
	/* Singleton sequence set */
	if (ts1->count == 1)
		return tdwithin_tpointseq_tpointseq(temporals_seq_n(ts1, 0),
			temporals_seq_n(ts2, 0), dist, func);

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts1->totalcount * 4);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		k += tdwithin_tpointseq_tpointseq2(&sequences[k], seq1, seq2, dist,
			func);
	}
	TemporalS *result = temporals_make(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

/*****************************************************************************
 * Generic dispatch functions
 *****************************************************************************/

/* Functions for spatial relationships that accept geometry */

Temporal *
tspatialrel_tpoint_geo(const Temporal *temp, Datum geo,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			geo, func, valuetypid, invert);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			geo, func, valuetypid, invert);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tspatialrel_tpointseq_geo((TemporalSeq *)temp,
			geo, func, valuetypid, invert);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tspatialrel_tpoints_geo( (TemporalS *)temp,
			geo, func, valuetypid, invert);
	return result;
}

Temporal *
tspatialrel3_tpoint_geo(const Temporal *temp, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc3_temporalinst_base((TemporalInst *)temp,
			geo, param, func, BOOLOID, invert);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc3_temporali_base((TemporalI *)temp,
			geo, param, func, BOOLOID, invert);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tspatialrel3_tpointseq_geo((TemporalSeq *)temp,
			geo, param, func, BOOLOID, invert);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tspatialrel3_tpoints_geo((TemporalS *)temp,
			geo, param, func, BOOLOID, invert);
	return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcontains_geo_tpoint);

PGDLLEXPORT Datum
tcontains_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_contains, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tcontains_tpoint_geo);

PGDLLEXPORT Datum
tcontains_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_contains, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcovers_geo_tpoint);

PGDLLEXPORT Datum
tcovers_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_covers, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tcovers_tpoint_geo);

PGDLLEXPORT Datum
tcovers_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_covers, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal coveredby
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcoveredby_geo_tpoint);

PGDLLEXPORT Datum
tcoveredby_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_coveredby, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tcoveredby_tpoint_geo);

PGDLLEXPORT Datum
tcoveredby_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_coveredby, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 * Since the ST_Disjoint function in PostGIS does not support 3D, we use
 * ST_3DIntersects and negate the result
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdisjoint_geo_tpoint);

PGDLLEXPORT Datum
tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
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
	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_intersects3d : &geom_intersects2d;
	Temporal *negresult = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, BOOLOID, true);
	Temporal *result = tnot_tbool_internal(negresult);
	pfree(negresult);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tpoint_geo);

PGDLLEXPORT Datum
tdisjoint_tpoint_geo(PG_FUNCTION_ARGS)
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
	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_intersects3d : &geom_intersects2d;
	Temporal *negresult = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, BOOLOID, true);
	Temporal *result = tnot_tbool_internal(negresult);
	pfree(negresult);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tpoint_tpoint);

PGDLLEXPORT Datum
tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_cross(temp1, temp2,
		&datum2_point_ne, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tequals_geo_tpoint);

PGDLLEXPORT Datum
tequals_geo_tpoint(PG_FUNCTION_ARGS)
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
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&datum2_point_eq, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tequals_tpoint_geo);

PGDLLEXPORT Datum
tequals_tpoint_geo(PG_FUNCTION_ARGS)
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
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&datum2_point_eq, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tequals_tpoint_tpoint);

PGDLLEXPORT Datum
tequals_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_cross(temp1, temp2,
		&datum2_point_eq, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tintersects_geo_tpoint);

PGDLLEXPORT Datum
tintersects_geo_tpoint(PG_FUNCTION_ARGS)
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

	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_intersects3d : &geom_intersects2d;
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tpoint_geo);

PGDLLEXPORT Datum
tintersects_tpoint_geo(PG_FUNCTION_ARGS)
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
	Datum (*func)(Datum, Datum) = MOBDB_FLAGS_GET_Z(temp->flags) ?
		&geom_intersects3d : &geom_intersects2d;
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		func, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tpoint_tpoint);

PGDLLEXPORT Datum
tintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
	Datum (*func)(Datum, Datum);
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &geom_intersects3d :
			&geom_intersects2d;
	else
		func = &geog_intersects;
	Temporal *result = sync_tfunc2_temporal_temporal_cross(temp1, temp2,
		func, BOOLOID);
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

PGDLLEXPORT Datum
ttouches_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_touches, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttouches_tpoint_geo);

PGDLLEXPORT Datum
ttouches_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_touches, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(twithin_geo_tpoint);

PGDLLEXPORT Datum
twithin_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_within, BOOLOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(twithin_tpoint_geo);

PGDLLEXPORT Datum
twithin_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_within, BOOLOID, true);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/


PG_FUNCTION_INFO_V1(tdwithin_geo_tpoint);

PGDLLEXPORT Datum
tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
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
	Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_geo);

PGDLLEXPORT Datum
tdwithin_tpoint_geo(PG_FUNCTION_ARGS)
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
	Temporal *result = tdwithin_tpoint_geo_internal(temp, gs, dist);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

Temporal *
tdwithin_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2,
	Datum dist)
{
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false  */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
		return NULL;

	Datum (*func)(Datum, Datum, Datum);
	ensure_point_base_type(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = MOBDB_FLAGS_GET_Z(temp1->flags) ? &geom_dwithin3d :
			&geom_dwithin2d;
	else
		func = &geog_dwithin;

	Temporal *result;
	ensure_valid_duration(sync1->duration);
	if (sync1->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc3_temporalinst_temporalinst(
			(TemporalInst *)sync1, (TemporalInst *)sync2, dist, func,
			BOOLOID);
	else if (sync1->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc3_temporali_temporali(
			(TemporalI *)sync1, (TemporalI *)sync2, dist, func,
			BOOLOID);
	else if (sync1->duration == TEMPORALSEQ)
		result = (Temporal *)tdwithin_tpointseq_tpointseq(
			(TemporalSeq *)sync1, (TemporalSeq *)sync2, dist, func);
	else /* sync1->duration == TEMPORALS */
		result = (Temporal *)tdwithin_tpoints_tpoints(
			(TemporalS *)sync1, (TemporalS *)sync2, dist, func);

	pfree(sync1); pfree(sync2);
	return result;
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_tpoint);

PGDLLEXPORT Datum
tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_same_dimensionality_tpoint(temp1, temp2);
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

PGDLLEXPORT Datum
trelate_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_relate, TEXTOID, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_tpoint_geo);

PGDLLEXPORT Datum
trelate_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_relate, TEXTOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_tpoint_tpoint);

PGDLLEXPORT Datum
trelate_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_has_not_Z_tpoint(temp1);
	ensure_has_not_Z_tpoint(temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_cross(temp1, temp2,
		&geom_relate, TEXTOID);
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

PGDLLEXPORT Datum
trelate_pattern_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel3_tpoint_geo(temp, PointerGetDatum(gs),
		pattern, &geom_relate_pattern, true);
	PG_FREE_IF_COPY(gs, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tpoint_geo);

PGDLLEXPORT Datum
trelate_pattern_tpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint_gs(temp, gs);
	ensure_has_not_Z_tpoint(temp);
	ensure_has_not_Z_gs(gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel3_tpoint_geo(temp, PointerGetDatum(gs),
		pattern, &geom_relate_pattern, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tpoint_tpoint);

PGDLLEXPORT Datum
trelate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	ensure_same_srid_tpoint(temp1, temp2);
	ensure_has_not_Z_tpoint(temp1);
	ensure_has_not_Z_tpoint(temp2);
	Temporal *result = sync_tfunc3_temporal_temporal_cross(temp1, temp2,
		pattern, &geom_relate_pattern, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
