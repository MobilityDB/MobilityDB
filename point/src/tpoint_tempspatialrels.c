/*****************************************************************************
 *
 * tpoint_tempspatialrels.c
 *	  Temporal spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported for geometries:
 *		tcontains, tcovers, tcoveredby, tdisjoint, 
 *		tequals, tintersects, ttouches, twithin, tdwithin, and
 *		trelate (with 2 and 3 arguments)
 * The following relationships are supported for geographies
 *		tcovers, tcoveredby, tintersects
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
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

/* Get the temporal instants at which a temporal sequence intersects a line */

static TemporalInst **
tpointseq_intersection_instants(TemporalInst *inst1, TemporalInst *inst2,
	Datum line, bool lower_inc, bool upper_inc, Datum intersections, int *count)
{
	/* Each intersection is either a point or a linestring with two points */
	int countinter = DatumGetInt32(call_function1(
		LWGEOM_numgeometries_collection, intersections));
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 2 * countinter);
	double duration = (double)(inst2->t - inst1->t);
	int k = 0;
	for (int i = 1; i <= countinter; i++) 
	{
		/* Find the i-th intersection */
		Datum inter = call_function2(LWGEOM_geometryn_collection, intersections, i);
		GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);
		if (gserialized_get_type(gsinter) == POINTTYPE)
		{
			double fraction = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter));
			TimestampTz time = (double)(inst1->t) + duration * fraction;
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || timestamp_cmp_internal(inst1->t, time) != 0) &&
				(upper_inc || timestamp_cmp_internal(inst2->t, time) != 0))
			{
				instants[k++] = temporalinst_make(inter, time, 
					inst1->valuetypid);
			}
		}
		else
		{
			Datum point1 = call_function2(LWGEOM_pointn_linestring, inter, 1);
			double fraction1 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point1));
			TimestampTz time1 = (double)(inst1->t) + duration * fraction1;
			/* If the point intersection is not at an exclusive bound */
			if ((lower_inc || timestamp_cmp_internal(inst1->t, time1) != 0) &&
				(upper_inc || timestamp_cmp_internal(inst2->t, time1) != 0))
			{
				instants[k++] = temporalinst_make(point1, time1, 
					inst1->valuetypid);
			}
			Datum point2 = call_function2(LWGEOM_pointn_linestring, inter, 2);
			double fraction2 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, point2));
			TimestampTz time2 = (double)(inst1->t) + duration * fraction2;

			/* If the point intersection is not at an exclusive bound and 
			 * time2 != time1 (this last condition arrives when point1 is 
			 * at an epsilon distance from point2 */
			if ((lower_inc || timestamp_cmp_internal(inst1->t, time2) != 0) &&
				(upper_inc || timestamp_cmp_internal(inst2->t, time2) != 0) &&
				timestamp_cmp_internal(time1, time2) != 0)
			{
				instants[k++] = temporalinst_make(point2, time2, 
					inst1->valuetypid);
			}
			pfree(DatumGetPointer(point1)); pfree(DatumGetPointer(point2));			
		}
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));	
	}
	/* Sort the instants */
	temporalinstarr_sort(instants, k);
	*count = k;
	return instants;
}

/*****************************************************************************/

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal sequence.
 * The potential crossings between the two are considered.
 * This function is not available for geographies since it calls the 
 * intersection function in POSTGIS that is only available for geometries.
 */

static TemporalSeq **
tspatialrel_tpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2, Datum geo, 
	bool lower_inc, bool upper_inc, Datum (*func)(Datum, Datum), 
	Oid valuetypid, int *count, bool invert)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	/* Constant segment */
	if (datum_point_eq(value1, value2))
	{	
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		Datum value = invert ? func(geo, value1) : func(value1, geo);
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
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
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections)); 
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
		*count = 1;
		return result;
	}
	
	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tpointseq_intersection_instants(inst1, inst2, line, 
		lower_inc, upper_inc, intersections, &countinst);
	pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections)); 

	/* No intersections were found */
	if (countinst == 0)
	{
		/* There may be an intersection at an exclusive bound.
		 * Find the middle time between inst1 and inst2 
		 * and compute the func at that point */
		TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
		Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
		Datum intvalue1 = invert ? func(geo, intvalue) :
			func(intvalue, geo);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		instants[0] = temporalinst_make(intvalue1, inst1->t, valuetypid);
		instants[1] = temporalinst_make(intvalue1, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(intvalue, inst1->valuetypid); FREE_DATUM(intvalue1, valuetypid);
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
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t, 
			valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
	}
	for (int i = 0; i < countinst; i++) 
	{
		/* Compute the value at the intersection point */
		Datum value = invert ? func(temporalinst_value(interinstants[i]), geo) :
			func(geo, temporalinst_value(interinstants[i]));
		instants[0] = temporalinst_make(value, (interinstants[i])->t, 
			valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		FREE_DATUM(value, valuetypid); 
		pfree(instants[0]);
		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one 
			 * and compute the func at that point */
			double time1 = (interinstants[i])->t;
			double time2 = (interinstants[i + 1])->t;
			TimestampTz inttime = time1 + ((time2 - time1)/2);
			Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
			Datum intvalue1 = invert ? func(geo, intvalue) :
				func(intvalue, geo);
			instants[0] = temporalinst_make(intvalue1, time1, valuetypid);
			instants[1] = temporalinst_make(intvalue1, time2, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(intvalue));
			FREE_DATUM(intvalue1, valuetypid); 
		}
	}
	if (after)
	{
		Datum value = invert ? func(geo, value2) : func(value2, geo);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t, 
			valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);
	
	*count = k;
	return result;
}

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal sequence.
 * The potential crossings between the two are considered.
 */

static TemporalSeq **
tspatialrel_tpointseq_geo2(TemporalSeq *seq, Datum geo, 
	Datum (*func)(Datum, Datum), Oid valuetypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value = invert ? func(geo, temporalinst_value(inst)) : 
			func(temporalinst_value(inst), geo);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalinst_make(value, inst->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		*count = 1;
		return result;		
	}
	
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * seq->count);
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel_tpointseq_geo1(inst1, inst2, geo, 
			lower_inc, upper_inc, func, valuetypid, &countseqs[i], invert);
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
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel_tpointseq_geo2(seq, geo, 
		func, valuetypid, &count, invert);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
		
	return result;
}

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal period.
 * The potential crossings between the two are considered.
 */
 
static TemporalS *
tspatialrel_tpoints_geo(TemporalS *ts, Datum geo, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tspatialrel_tpointseq_geo(temporals_seq_n(ts, 0), geo, 
			func, valuetypid, invert);
		
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel_tpointseq_geo2(seq, geo, func,
			valuetypid, &countseqs[i], invert);
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
	TemporalS *result = temporals_from_temporalseqarr(allsequences, 
		totalseqs, true);
	
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************/

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal sequence.
 * The potential crossings between the two are considered.
 * This function is not available for geographies since it calls the 
 * intersection function in POSTGIS that is only available for geometries.
 */

static TemporalSeq **
tspatialrel3_tpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2, Datum geo, Datum param, 
	bool lower_inc, bool upper_inc, Datum (*func)(Datum, Datum, Datum), 
	Oid valuetypid, int *count, bool invert)
{
	/* Constant segment */
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	if (datum_point_eq(value1, value2))
	{	
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value = invert ? func(value1, geo, param) :
			func(geo, value1, param);
		TemporalInst *inst = temporalinst_make(value, inst1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst);
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
		Datum value = invert ? func(value1, geo, param) :
			func(geo, value1, param);
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections)); 
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
		*count = 1;
		return result;
	}
	
	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tpointseq_intersection_instants(inst1, inst2, line, 
		lower_inc, upper_inc, intersections, &countinst);
	pfree(DatumGetPointer(intersections)); 
	pfree(DatumGetPointer(line)); 

	/* No intersections were found */
	if (countinst == 0)
	{	
		/* There may be an intersection at an exclusive bound.
		 * Find the middle time between inst1 and inst2 
		 * and compute the func at that point */
		TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
		Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
		Datum intvalue1 = invert ? func(geo, intvalue, param) :
		func(intvalue, geo, param);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *instants[2];
		instants[0] = temporalinst_make(intvalue1, inst1->t, valuetypid);
		instants[1] = temporalinst_make(intvalue1, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(intvalue, inst1->valuetypid); FREE_DATUM(intvalue1, valuetypid);
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
		Datum value = invert ? func(geo, value1, param) :
			func(value1, geo, param);
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
	}
	for (int i = 0; i < countinst; i++) 
	{
		/* Compute the value at the intersection point */
		Datum value = invert ? 
			func(geo, temporalinst_value(interinstants[i]), param) :
			func(temporalinst_value(interinstants[i]), geo, param);
		instants[0] = temporalinst_make(value, (interinstants[i])->t, 
			valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		FREE_DATUM(value, valuetypid); 
		pfree(instants[0]);
		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one 
			 * and compute the func at that point */
			double time1 = (interinstants[i])->t;
			double time2 = (interinstants[i + 1])->t;
			TimestampTz inttime = time1 + ((time2 - time1)/2);
			Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
			Datum intvalue1 = invert ? func(geo, intvalue, param) :
				func(intvalue, geo, param);
			instants[0] = temporalinst_make(intvalue1, time1, valuetypid);
			instants[1] = temporalinst_make(intvalue1, time2, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false);
			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(intvalue));
			FREE_DATUM(intvalue1, valuetypid); 
		}
	}
	if (after)
	{
		Datum value = invert ? func(geo, value2, param) :
			func(value2, geo, param);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t, 
			valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value, valuetypid); 
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);
	
	*count = k;
	return result;
}

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal sequence.
 * The potential crossings between the two are considered.
 */

static TemporalSeq **
tspatialrel3_tpointseq_geo2(TemporalSeq *seq, Datum geo, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, 
	int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value = invert ? func(geo, temporalinst_value(inst), param) : 
			func(temporalinst_value(inst), geo, param);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalinst_make(value, inst->t, 
			valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		*count = 1;
		return result;		
	}
	
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * seq->count);
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel3_tpointseq_geo1(inst1, inst2, geo, param, 
			lower_inc, upper_inc, func, valuetypid, &countseqs[i], invert);
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
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel3_tpointseq_geo2(seq, geo, param, 
		func, valuetypid, &count, invert);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
		
	return result;
}

/* 
 * Generic function to compute the temporal spatial relationship
 * between a geometry/geography and a temporal period.
 * The potential crossings between the two are considered.
 */
 
static TemporalS *
tspatialrel3_tpoints_geo(TemporalS *ts, Datum geo, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tspatialrel3_tpointseq_geo(temporals_seq_n(ts, 0), geo, param,
			func, valuetypid, invert);

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel3_tpointseq_geo2(seq, geo, param, func,
			valuetypid, &countseqs[i], invert);
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
	TemporalS *result = temporals_from_temporalseqarr(allsequences, 
		totalseqs, true);
	
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************/

/*
 * This function is not available for geographies nor for 3D since it is   
 * based on the tpointseq_at_geometry1 function. The function uses the   
 * st_dwithin function from PostGIS only for instantaneous sequences.
 */
static TemporalSeq **
tdwithin_tpointseq_geo1(TemporalSeq *seq, Datum geo, Datum dist, int *count)
{
	if (seq->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value = temporalinst_value(temporalseq_inst_n(seq, 0));
		Datum dwithin = geom_dwithin2d(value, geo, dist);
		TemporalInst *inst = temporalinst_make(dwithin, seq->period.lower,
			BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst); 
		*count = 1;
		return result;
	}
	
	/* Restrict to the buffered geometry */
	TemporalInst *instants[2];
	Datum geo_buffer = call_function2(buffer, geo, dist);
	int count1;
	TemporalSeq **atbuffer = tpointseq_at_geometry2(seq, geo_buffer, &count1);
	if (atbuffer == NULL)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		instants[0] = temporalinst_make(BoolGetDatum(false),
			seq->period.lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false),
			seq->period.upper, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			seq->period.lower_inc, seq->period.upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}
	
	/* Get the periods during which the value is true */
	Period **periods = palloc(sizeof(Period *) * count1);
	for (int i = 0; i < count1; i++)
		periods[i] = &atbuffer[i]->period;
	/* The period set must be normalized, i.e., last parameter must be true */
	PeriodSet *ps = periodset_from_periodarr_internal(periods, count1, true);
	for (int i = 0; i < count1; i++)
		pfree(atbuffer[i]);
	pfree(periods);
	/* Get the periods during which the value is false */
	PeriodSet *minus = minus_period_periodset_internal(&seq->period, ps);
	if (minus == NULL)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		instants[0] = temporalinst_make(BoolGetDatum(true),
			seq->period.lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(true),
			seq->period.upper, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			seq->period.lower_inc, seq->period.upper_inc, false);
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
			instants[0] = temporalinst_make(BoolGetDatum(true), p1->lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(true), p1->upper, BOOLOID);
			result[i] = temporalseq_from_temporalinstarr(instants, 2, 
				p1->lower_inc, p1->upper_inc, false);
			pfree(instants[0]); pfree(instants[1]);
			j++;
		}
		else
		{
			p2 = periodset_per_n(minus, k);
			instants[0] = temporalinst_make(BoolGetDatum(false), p2->lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(false), p2->upper, BOOLOID);
			result[i] = temporalseq_from_temporalinstarr(instants, 2, 
				p2->lower_inc, p2->upper_inc, false);
			pfree(instants[0]); pfree(instants[1]);
			k++;
		}
		truevalue = !truevalue;
	}
	pfree(ps); pfree(minus);
	return result;
}

static TemporalS *
tdwithin_tpointseq_geo(TemporalSeq *seq, Datum geo, Datum dist)
{
	int count;
	TemporalSeq **sequences = tdwithin_tpointseq_geo1(seq, geo, dist, &count);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	
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
	TemporalS *result = temporals_from_temporalseqarr(allsequences, totalseqs, true);
	
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************/

/* 
 * Determine the instants t1 and t2 at which two temporal periods have a 
 * distance d between each other. This amounts to solve the equation 
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

 */

static int
tdwithin_tpointseq_tpointseq1(Datum sv1, Datum ev1, Datum sv2, Datum ev2, 
	TimestampTz lower, TimestampTz upper, double d, bool hasz,
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
		/* distance function = d */
		a = a_x + a_y + a_z;
		b = b_x + b_y + b_z;
		c = c_x + c_y + c_z - (d * d);
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
		/* distance function = d */
		a = a_x + a_y;
		b = b_x + b_y;
		c = c_x + c_y - (d * d);
	}
	/* They are parallel, moving in the same direction at the same speed */
	if (a == 0)
	{
		if (!func(sv1, sv2, Float8GetDatum(d)))
			return 0;
		*t1 = lower;
		*t2 = upper;
		return 2;
	}
	/* Solving the quadratic equation for distance = d */
	long double discriminant = b * b - 4 * a * c;

	/* One solution */
	if (discriminant == 0)
	{
		long double t5 = (-1 * b) / (2 * a);
		if (t5 < 0.0 || t5 > 1.0)
			return 0;
		*t1 = (double)lower + (t5 * duration);
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
		if(b >= 0)
		{
			t5 = (-1 * b - sqrt(discriminant)) / (2 * a);
			t6 = (2 * c ) / (-1 * b - sqrt(discriminant));
		}
		else
		{
			t5 = (2 * c ) / (-1 * b + sqrt(discriminant));
			t6 = (-1 * b + sqrt(discriminant)) / (2 * a);
		}

		/* If the two intervals do not intersect */
		if (0.0 > t6 || t5 > 1.0) 
			return 0;
		/* Compute the intersection of the two intervals */
		long double t7 = Max(0.0, t5);
		long double t8 = Min(1.0, t6);
		if (fabsl(t7 - t8) < EPSILON)
		{
			*t1 = (double)lower + (t7 * duration);
			return 1;			
		}
		else
		{
			*t1 = (double)lower + (t7 * duration);
			*t2 = (double)lower + (t8 * duration);
			return 2;
		}
	}
}

/* The following function supposes that the two temporal values are synchronized.
   This should be ensured by the calling function. */

static void
tdwithin_tpointseq_tpointseq2(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc, 
	Datum d, bool hasz, Datum (*func)(Datum, Datum, Datum), int *count)
{
	TimestampTz lower = start1->t;
	TimestampTz upper = end1->t;
	Datum sv1 = temporalinst_value(start1);
	Datum ev1 = temporalinst_value(end1);
	Datum sv2 = temporalinst_value(start2);
	Datum ev2 = temporalinst_value(end2);
	TemporalInst *instants[2];
	
	/* Both segments are constant */
	if (datum_point_eq(sv1, ev1) && datum_point_eq(sv2, ev2))
	{
		Datum value = func(sv1, sv2, d);
		instants[0] = temporalinst_make(value, lower, BOOLOID);
		instants[1] = temporalinst_make(value, upper, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return;
	}

	/* Find the instants t1 and t2 (if any) during which the dwithin function is true */
	TimestampTz t1, t2;
	int solutions = tdwithin_tpointseq_tpointseq1(sv1, ev1, sv2, ev2, lower, upper,
		DatumGetFloat8(d), hasz, func, &t1, &t2);

	/* No instant is returned */
	if (solutions == 0)
	{
		instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);			
		pfree(instants[0]); pfree(instants[1]); 
		*count = 1;
		return;
	}
	/* A single instant is returned */
	if (solutions == 1)
	{	
		if ((t1 == lower && !lower_inc) || (t1 == upper && !upper_inc))
		{
			instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
			result[0] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false);			
			pfree(instants[0]); pfree(instants[1]); 
			*count = 1;
			return;
		}		
		if (t1 == lower) /* && lower_inc */
		{	
			instants[0] = temporalinst_make(BoolGetDatum(true),	lower, BOOLOID);
			result[0] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
			result[1] = temporalseq_from_temporalinstarr(instants, 2,
				false, upper_inc, false);			
			pfree(instants[0]); pfree(instants[1]); 
			*count = 2;
			return;
		}
		if (t1 == upper) /* && upper_inc */
		{	
			instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(true), upper, BOOLOID);
			result[0] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, true, false);			
			pfree(instants[0]); pfree(instants[1]);
			*count = 1;
			return;
		}
		if (t1 != lower && t1 != upper)
		{	
			instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(false), t1, BOOLOID);
			result[0] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, false, false);			
			pfree(instants[0]); pfree(instants[1]);
			instants[0] = temporalinst_make(BoolGetDatum(true), t1, BOOLOID);
			result[1] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]); 
			instants[0] = temporalinst_make(BoolGetDatum(false), t1, BOOLOID);
			instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
			result[2] = temporalseq_from_temporalinstarr(instants, 2,
				false, upper_inc, false);			
			pfree(instants[0]); pfree(instants[1]);
			*count = 3;
			return;
		}		
	}
	/* Two instants are returned */
	if (lower == t1 && upper == t2)
	{	
		instants[0] = temporalinst_make(BoolGetDatum(true), lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(true), upper, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return;
	}
	if (lower != t1 && upper == t2)
	{	
		instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false), t1, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);	
		pfree(instants[0]); pfree(instants[1]);
		instants[0] = temporalinst_make(BoolGetDatum(true), t1, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(true), upper, BOOLOID);
		result[1] = temporalseq_from_temporalinstarr(instants, 2,
			true, upper_inc, false);		
		pfree(instants[0]); pfree(instants[1]);
		*count = 2;
		return;
	}
	if (lower == t1 && upper != t2)
	{	
		instants[0] = temporalinst_make(BoolGetDatum(true), lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(true), t2, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		instants[0] = temporalinst_make(BoolGetDatum(false), t2, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
		result[1] = temporalseq_from_temporalinstarr(instants, 2,
			true, upper_inc, false);		
		pfree(instants[0]); pfree(instants[1]);
		*count = 2;
		return;
	}
	else
	{	
		instants[0] = temporalinst_make(BoolGetDatum(false), lower, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false), t1, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);		
		pfree(instants[0]); pfree(instants[1]);
		instants[0] = temporalinst_make(BoolGetDatum(true), t1, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(true), t2, BOOLOID);
		result[1] = temporalseq_from_temporalinstarr(instants, 2,
			true, true, false);
		pfree(instants[0]); pfree(instants[1]);
		instants[0] = temporalinst_make(BoolGetDatum(false), t2, BOOLOID);
		instants[1] = temporalinst_make(BoolGetDatum(false), upper, BOOLOID);
		result[2] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		*count = 3;
		return;
	}
}

static int
tdwithin_tpointseq_tpointseq3(TemporalSeq **result, TemporalSeq *seq1, TemporalSeq *seq2,
	Datum d, Datum (*func)(Datum, Datum, Datum))
{
	if (seq1->count == 1)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		TemporalInst *inst = temporalinst_make(
			func(temporalinst_value(inst1), temporalinst_value(inst2), d),
			inst1->t, BOOLOID);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst);
		return 1;
	}

	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	bool hasz = MOBDB_FLAGS_GET_Z(seq1->flags);
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;
		tdwithin_tpointseq_tpointseq2(&result[k], start1, end1, start2, end2, 
			lower_inc, upper_inc, d, hasz, func, &countseq);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	return k;
}

static TemporalS *
tdwithin_tpointseq_tpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*func)(Datum, Datum, Datum))
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int count = tdwithin_tpointseq_tpointseq3(sequences, seq1, seq2, 
		param, func);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
		
	return result;
}

/* The following function supposes that the two temporal values are synchronized.
   This should be ensured by the calling function. */

static TemporalS *
tdwithin_tpoints_tpoints(TemporalS *ts1, TemporalS *ts2, Datum d,
	Datum (*func)(Datum, Datum, Datum))
{
	/* Singleton sequence set */
	if (ts1->count == 1)
		return tdwithin_tpointseq_tpointseq(temporals_seq_n(ts1, 0), 
			temporals_seq_n(ts2, 0), d, func);

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts1->totalcount * 3);
	int k = 0, countstep;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		countstep = tdwithin_tpointseq_tpointseq3(&sequences[k], seq1, seq2, d, 
			func);
		k += countstep;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

/*****************************************************************************
 * Generic dispatch functions
 *****************************************************************************/

static Temporal *
tspatialrel_tpoint_geo(Temporal *temp, Datum geo,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			geo, func, valuetypid, invert);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			geo, func, valuetypid, invert);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tspatialrel_tpointseq_geo((TemporalSeq *)temp,
			geo, func, valuetypid, invert);
	else if (temp->duration == TEMPORALS) 
		result = (Temporal *)tspatialrel_tpoints_geo((TemporalS *)temp,
			geo, func, valuetypid, invert);
	return result;
}

static Temporal *
tspatialrel3_tpoint_geo(Temporal *temp, Datum geo, Datum param,
	Datum (*func)(Datum, Datum, Datum), bool invert)
{
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc3_temporalinst_base((TemporalInst *)temp,
			geo, param, func, BOOLOID, invert);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc3_temporali_base((TemporalI *)temp,
			geo, param, func, BOOLOID, invert);
	else if (temp->duration == TEMPORALSEQ) 
		result = (Temporal *)tspatialrel3_tpointseq_geo((TemporalSeq *)temp,
			geo, param, func, BOOLOID, invert);
	else if (temp->duration == TEMPORALS) 
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
 
PG_FUNCTION_INFO_V1(tcontains_tpoint_tpoint);

PGDLLEXPORT Datum
tcontains_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_contains, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal covers (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcovers_geo_tpoint);

PGDLLEXPORT Datum
tcovers_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum) = 0;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;

	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALSEQ) 
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq,
				PointerGetDatum(gs), &geom_covers, BOOLOID, true);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), &geom_covers, BOOLOID, true);
			pfree(seq1);
		}
	}	
	else if (temp->duration == TEMPORALS) 
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts,
				PointerGetDatum(gs), &geom_covers, BOOLOID, true);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), &geom_covers, BOOLOID, true);
			pfree(ts1);
		}
	}

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq,
				PointerGetDatum(gs), &geom_covers, BOOLOID, false);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), &geom_covers, BOOLOID, false);
			pfree(seq1);
		}
	}
	else if (temp->duration == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts,
				PointerGetDatum(gs), &geom_covers, BOOLOID, false);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), &geom_covers, BOOLOID, false);
			pfree(ts1);
		}
	}

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tcovers_tpoint_tpoint);

PGDLLEXPORT Datum
tcovers_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_covers;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_covers;
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		func, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal coveredby (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcoveredby_geo_tpoint);

PGDLLEXPORT Datum
tcoveredby_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;

	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALSEQ) 
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq,
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, true);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, true);
			pfree(seq1);
		}
	}	
	else if (temp->duration == TEMPORALS) 
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts,
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, true);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, true);
			pfree(ts1);
		}
	}

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;

	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp, 
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq, 
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, false);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), &geom_coveredby, BOOLOID, false);
			pfree(seq1);
		}
	}
	else if (temp->duration == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts, 
				PointerGetDatum(gs), &geom_coveredby, 
				BOOLOID, false);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), &geom_coveredby, 
				BOOLOID, false);
			pfree(ts1);
		}
	}

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}
 
PG_FUNCTION_INFO_V1(tcoveredby_tpoint_tpoint);

PGDLLEXPORT Datum
tcoveredby_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
		func = &geom_coveredby;
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_coveredby;
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		func, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/


PG_FUNCTION_INFO_V1(tdisjoint_geo_tpoint);

PGDLLEXPORT Datum
tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, BOOLOID, true);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs), 
		&geom_disjoint, BOOLOID, false);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_disjoint, BOOLOID);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_equals, BOOLOID, true);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}
	Temporal *result = tspatialrel_tpoint_geo(temp, PointerGetDatum(gs),
		&geom_equals, BOOLOID, false);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_equals, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal intersects (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tintersects_geo_tpoint);

PGDLLEXPORT Datum
tintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = 0;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;

	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, true);
	else if (temp->duration == TEMPORALSEQ) 
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq,
				PointerGetDatum(gs), func, BOOLOID, true);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), func, BOOLOID, true);
			pfree(seq1);
		}
	}	
	else if (temp->duration == TEMPORALS) 
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts,
				PointerGetDatum(gs), func, BOOLOID, true);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), func, BOOLOID, true);
			pfree(ts1);
		}
	}

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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), func, BOOLOID, false);
	else if (temp->duration == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpointseq_geo(seq,
				PointerGetDatum(gs), func, BOOLOID, false);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tspatialrel_tpointseq_geo(seq1,
				PointerGetDatum(gs), func, BOOLOID, false);
			pfree(seq1);
		}
	}
	else if (temp->duration == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tspatialrel_tpoints_geo(ts,
				PointerGetDatum(gs), func, BOOLOID, false);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tspatialrel_tpoints_geo(ts1,
				PointerGetDatum(gs), func, BOOLOID, false);
			pfree(ts1);
		}
	}

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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Datum (*func)(Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			func = &geom_intersects3d;
		else
			func = &geom_intersects2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_intersects;
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
 
PG_FUNCTION_INFO_V1(ttouches_tpoint_tpoint);

PGDLLEXPORT Datum
ttouches_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_touches, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
 
PG_FUNCTION_INFO_V1(twithin_tpoint_tpoint);

PGDLLEXPORT Datum
twithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_within, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin (for both geometry and geography)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdwithin_geo_tpoint);

PGDLLEXPORT Datum
tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
	GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(gs, 0);
		PG_FREE_IF_COPY(temp, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc3_temporalinst_base((TemporalInst *)temp,
			PointerGetDatum(gs), dist, func, BOOLOID, true);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc3_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), dist, func, BOOLOID, true);
	else if (temp->duration == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tdwithin_tpointseq_geo(seq,
				PointerGetDatum(gs), dist);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tdwithin_tpointseq_geo(seq1,
				PointerGetDatum(gs), dist);
			pfree(seq1);
		}
	}
	else if (temp->duration == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tdwithin_tpoints_geo(ts,
				PointerGetDatum(gs), dist);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tdwithin_tpoints_geo(ts1,
				PointerGetDatum(gs), dist);
			pfree(ts1);
		}
	}
	
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
	if (gserialized_is_empty(gs))
	{
		PG_FREE_IF_COPY(temp, 0);
		PG_FREE_IF_COPY(gs, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp->valuetypid);
	if (temp->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	if (temp->duration == TEMPORALINST) 
		result = (Temporal *)tfunc3_temporalinst_base((TemporalInst *)temp, 
			PointerGetDatum(gs), dist, func, BOOLOID, false);
	else if (temp->duration == TEMPORALI) 
		result = (Temporal *)tfunc3_temporali_base((TemporalI *)temp,
			PointerGetDatum(gs), dist, func, BOOLOID, false);
	else if (temp->duration == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		/* Validity of temporal point has been already verified */
		if (seq->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tdwithin_tpointseq_geo(seq,
				PointerGetDatum(gs), dist);
		else if (seq->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalSeq *seq1 = tgeogpointseq_as_tgeompointseq(seq);
			result = (Temporal *)tdwithin_tpointseq_geo(seq1,
				PointerGetDatum(gs), dist);
			pfree(seq1);
		}
	}
	else if (temp->duration == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		/* Validity of temporal point has been already verified */
		if (ts->valuetypid == type_oid(T_GEOMETRY))
			result = (Temporal *)tdwithin_tpoints_geo(ts,
				PointerGetDatum(gs), dist);
		else if (ts->valuetypid == type_oid(T_GEOGRAPHY))
		{
			TemporalS *ts1 = tgeogpoints_as_tgeompoints(ts);
			result = (Temporal *)tdwithin_tpoints_geo(ts1,
				PointerGetDatum(gs), dist);
			pfree(ts1);
		}
	}

	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(gs, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tpoint_tpoint);

PGDLLEXPORT Datum
tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false  */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Datum (*func)(Datum, Datum, Datum) = NULL;
	point_base_type_oid(temp1->valuetypid);
	if (temp1->valuetypid == type_oid(T_GEOMETRY))
	{
		if (MOBDB_FLAGS_GET_Z(temp1->flags))
			func = &geom_dwithin3d;
		else
			func = &geom_dwithin2d;
	}
	else if (temp1->valuetypid == type_oid(T_GEOGRAPHY))
		func = &geog_dwithin;

	Temporal *result = NULL;
	temporal_duration_is_valid(sync1->duration);
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
	else if (sync1->duration == TEMPORALS)
		result = (Temporal *)tdwithin_tpoints_tpoints(
			(TemporalS *)sync1, (TemporalS *)sync2, dist, func);

	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc2_temporal_temporal_crossdisc(temp1, temp2, 
		&geom_relate, BOOLOID);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_gs_same_srid(temp, gs);
	tpoint_gs_same_dimensionality(temp, gs);
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
	tpoint_same_srid(temp1, temp2);
	tpoint_same_dimensionality(temp1, temp2);
	Temporal *result = sync_tfunc3_temporal_temporal_crossdisc(temp1, temp2, 
		pattern, &geom_relate_pattern, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
