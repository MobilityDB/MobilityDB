/*****************************************************************************
 *
 * TempSpatialRels.c
 *	  Temporal spatial relationships for temporal network-constrained points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 *		tcontains, tcovers, tcoveredby, tdisjoint,
 *		tequals, tintersects, ttouches, twithin, tdwithin, and
 *		trelate (with 2 and 3 arguments)
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/*
 * Returns whether two segments have network intersection (i.e. network positions are same)
 * This function supposes that the two segments are synchronized and not equal!
 */
bool
tnpointseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2,
	bool lower_inc, bool upper_inc, TimestampTz *inter)
{
	npoint *startnp1 = DatumGetNpoint(temporalinst_value(start1));
	npoint *endnp1 = DatumGetNpoint(temporalinst_value(end1));
	npoint *startnp2 = DatumGetNpoint(temporalinst_value(start2));
	npoint *endnp2 = DatumGetNpoint(temporalinst_value(end2));

	if (startnp1->rid != startnp2->rid)
		return false;

	double s1 = startnp1->pos;
	double e1 = endnp1->pos;
	double s2 = startnp2->pos;
	double e2 = endnp2->pos;
	TimestampTz lower = start1->t;
	TimestampTz upper = end1->t;

	if (e1 - s1 - e2 + s2 == 0)
		return false;

	TimestampTz t = (TimestampTz)(((upper-lower)*(s2-s1) + lower * (s2-s1+e1-e2)) / (e1-s1-e2+s2));
	if (t == lower && lower_inc)
		*inter = lower;
	else if (t == upper && upper_inc)
		*inter = upper;
	else if (t > lower && t < upper)
		*inter= t;
	else
		return false;
	return true;
}

/*
 * Returns the number of geometric intersections (i.e. geometric positions are same) for two segments
 * For 1 intersection, intersection time is stored in *inter
 * For 2 intersections, intersection times are start1->time, end1->time
 * This function supposes that the two segments are synchronized and not equal!
 */
static int
tnpointseq_geom_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2,
	bool lower_inc, bool upper_inc, TimestampTz *inter)
{
	npoint *startnp1 = DatumGetNpoint(temporalinst_value(start1));
	npoint *endnp1 = DatumGetNpoint(temporalinst_value(end1));
	npoint *startnp2 = DatumGetNpoint(temporalinst_value(start2));
	npoint *endnp2 = DatumGetNpoint(temporalinst_value(end2));

	double s1 = startnp1->pos;
	double e1 = endnp1->pos;
	double s2 = startnp2->pos;
	double e2 = endnp2->pos;
	TimestampTz lower = start1->t;
	TimestampTz upper = end1->t;

	/* When two segments are on the same route,
	 * they have 0 or 1 intersection */
	if (startnp1->rid == startnp2->rid)
	{
		if (e1 - s1 - e2 + s2 == 0)
			return 0;

		TimestampTz t = (TimestampTz)(((upper-lower)*(s2-s1) + 
			lower * (s2-s1+e1-e2)) / (e1-s1-e2+s2));
		if (t == lower && lower_inc)
			*inter = lower;
		else if (t == upper && upper_inc)
			*inter = upper;
		else if (t > lower && t < upper)
			*inter= t;
		else
			return 0;
		return 1;
	}

	/* When two periods are on different routes,
	 * intersection occurs only on route endpoints */
	int countinter = 0;

	/* Intersect at lower? */
	if (lower_inc && (s1 == 0 || s1 == 1) && (s2 == 0 || s2 == 1))
	{
		Datum startgeom1 = npoint_geom_internal(startnp1);
		Datum startgeom2 = npoint_geom_internal(startnp2);
		if (datum_eq(startgeom1, startgeom2, type_oid(T_GEOMETRY)))
		{
			*inter = lower;
			countinter++;
		}
		pfree(DatumGetPointer(startgeom1));
		pfree(DatumGetPointer(startgeom2));
	}

	/* Intersect at upper? */
	if (upper_inc && (e1 == 0 || e1 == 1) && (e2 == 0 || e2 == 1))
	{
		Datum endgeom1 = npoint_geom_internal(endnp1);
		Datum endgeom2 = npoint_geom_internal(endnp2);
		if (datum_eq(endgeom1, endgeom2, type_oid(T_GEOMETRY)))
		{
			*inter = upper;
			countinter++;
		}
		pfree(DatumGetPointer(endgeom1));
		pfree(DatumGetPointer(endgeom2));
	}

	return countinter;
}

/*****************************************************************************
 * Functions that get the temporal instants at which
 * a temporal sequent intersects a line
 *****************************************************************************/

static TemporalInst **
tnpointseq_intersection_instants(TemporalInst *inst1, TemporalInst *inst2, 
	Datum line, Datum intersections, int *count)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));
	double duration = (double)inst2->t - (double)inst1->t;

	int countinter = DatumGetInt32(call_function1(
		LWGEOM_numgeometries_collection, intersections));
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 2 * countinter);
	int k = 0;
	for (int i = 1; i <= countinter; i++)
	{
		/* Find the i-th intersection */
		Datum inter = call_function2(LWGEOM_geometryn_collection, 
			intersections, Int32GetDatum(i));
		GSERIALIZED *gsinter = (GSERIALIZED *) PG_DETOAST_DATUM(inter);

		/* Each intersection is either a point or a linestring with two points */
		if (gserialized_get_type(gsinter) == POINTTYPE)
		{
			double fraction = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter));
			TimestampTz time = (TimestampTz)(inst1->t + duration * fraction);
			double pos = np1->pos + (np2->pos * fraction - np1->pos * fraction);
			npoint *intnp = npoint_constructor_internal(np1->rid, pos);
			instants[k++] = temporalinst_make(PointerGetDatum(intnp), time,
				type_oid(T_NPOINT));
			pfree(intnp);
		}
		else
		{
			Datum inter1 = call_function2(LWGEOM_pointn_linestring, inter, 1);
			double fraction1 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter1));
			TimestampTz time1 = (TimestampTz)(inst1->t + duration * fraction1);
			double pos1 = np1->pos + (np2->pos * fraction1 - np1->pos * fraction1);
			npoint *intnp1 = npoint_constructor_internal(np1->rid, pos1);
			instants[k++] = temporalinst_make(PointerGetDatum(intnp1), time1,
				type_oid(T_NPOINT));

			Datum inter2 = call_function2(LWGEOM_pointn_linestring, inter, 2);
			double fraction2 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter2));
			TimestampTz time2 = (TimestampTz)(inst1->t + duration * fraction2);
			double pos2 = np1->pos + (np2->pos * fraction2 - np1->pos * fraction2);
			npoint *intnp2 = npoint_constructor_internal(np1->rid, pos2);
			instants[k++] = temporalinst_make(PointerGetDatum(intnp2), time2,
				type_oid(T_NPOINT));

			pfree(DatumGetPointer(inter1));
			pfree(DatumGetPointer(inter2));
			pfree(intnp1);
			pfree(intnp2);
		}
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
	}

	/* Sort the instants */
	temporalinstarr_sort(instants, k);
	*count = k;
	return instants;
}

/*****************************************************************************
 * Functions that apply the operator to the composing instants and to the
 * crossings when the resulting value is discrete as required
 * for spatial relationships (e.g., tintersects).
 * These functions suppose that the two temporal values are synchronized.
 * This should be ensured by the calling function.
 *****************************************************************************/

TemporalInst *
tspatialrel_tnpointinst_tnpointinst(TemporalInst *inst1, TemporalInst *inst2,
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	Datum geom1 = tnpointinst_geom(inst1);
	Datum geom2 = tnpointinst_geom(inst2);
	Datum value = operator(geom1, geom2);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);

	pfree(DatumGetPointer(geom1));
	pfree(DatumGetPointer(geom2));
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalI *
tspatialrel_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2,
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti1->count);
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		Datum geom1 = tnpointinst_geom(inst1);
		Datum geom2 = tnpointinst_geom(inst2);
		Datum value = operator(geom1, geom2);
		instants[i] = temporalinst_make(value, inst1->t, valuetypid);

		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom1));
		pfree(DatumGetPointer(geom2));
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti1->count);

	for (int i = 0; i < ti1->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static void
tspatialrel_tnpointseq_tnpointseq1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc,
	Datum (*operator)(Datum, Datum), Oid valuetypid, int *count)
{
	npoint *startnp1 = DatumGetNpoint(temporalinst_value(start1));
	npoint *endnp1 = DatumGetNpoint(temporalinst_value(end1));
	npoint *startnp2 = DatumGetNpoint(temporalinst_value(start2));
	npoint *endnp2 = DatumGetNpoint(temporalinst_value(end2));
	TemporalInst *instants[2];

	Datum startgeom1 = npoint_geom_internal(startnp1);
	Datum startgeom2 = npoint_geom_internal(startnp2);
	Datum startvalue = operator(startgeom1, startgeom2);
	pfree(DatumGetPointer(startgeom1)); pfree(DatumGetPointer(startgeom2));

	/* If two segments are equal or constant */
	if ((eq_npoint_npoint_internal(startnp1, startnp2) && eq_npoint_npoint_internal(endnp1, endnp2)) ||
		(startnp1->pos == endnp1->pos && startnp2->pos == endnp2->pos))
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid);
		*count = 1;
		return;
	}

	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	int cross = tnpointseq_geom_intersect_at_timestamp(start1, end1, start2, end2,
		lower_inc, upper_inc, &crosstime);

	/* If there is no crossing */
	if (cross == 0)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]); pfree(instants);
		FREE_DATUM(startvalue, valuetypid);
		*count = 1;
		return;
	}

	Datum endgeom1 = npoint_geom_internal(endnp1);
	Datum endgeom2 = npoint_geom_internal(endnp2);
	Datum endvalue = operator(endgeom1, endgeom2);
	pfree(DatumGetPointer(endgeom1)); pfree(DatumGetPointer(endgeom2));

	/* If there are 2 crossings */
	if (cross == 2)
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]);

		/* Compute the operator in the middle time */
		npoint *intnp1 = npoint_constructor_internal(startnp1->rid, (startnp1->pos + endnp1->pos) / 2);
		Datum intgeom1 = npoint_geom_internal(intnp1);
		npoint *intnp2 = npoint_constructor_internal(startnp2->rid, (startnp2->pos + endnp2->pos) / 2);
		Datum intgeom2 = npoint_geom_internal(intnp2);
		Datum intvalue = operator(intgeom1, intgeom2);
		instants[0] = temporalinst_make(intvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(intvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2, false, false, false);
		pfree(instants[0]); pfree(instants[1]);

		/* Compute the operator at the end instant */
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[2] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]); pfree(instants);

		pfree(intnp1); pfree(intnp2);
		pfree(DatumGetPointer(intgeom1)); pfree(DatumGetPointer(intgeom2));
		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(intvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid);
		*count = 3;
		return;
	}

	/* There is only 1 crossing
	 * If the crossing is at the start instant */
	if (crosstime == start1->t)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]);

		/* Compute the operator at the end instants */
		instants[0] = temporalinst_make(endvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2, false, upper_inc, false);

		pfree(instants[0]); pfree(instants[1]); pfree(instants);
		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(endvalue, valuetypid);
		*count = 2;
		return;
	}

	/* If the crossing is at the end instant */
	if (crosstime == end1->t)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);

		/* Compute the operator at the end instants */
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]); pfree(instants);

		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(endvalue, valuetypid);
		*count = 2;
		return;
	}

	/* The crossing is at the middle */

	/* Compute the operator at the start instants */
	instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
	instants[1] = temporalinst_make(startvalue, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2, lower_inc, false, false);
	pfree(instants[0]); pfree(instants[1]);

	/* Compute the operator at crosstime */
	Datum crossnp = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum crossgeom = npoint_geom_internal(DatumGetNpoint(crossnp));
	Datum crossvalue = operator(crossgeom, crossgeom);
	instants[0] = temporalinst_make(crossvalue, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1,
		true, true, false);
	pfree(instants[0]);

	/* Compute the operator at the end instants */
	instants[0] = temporalinst_make(endvalue, crosstime, valuetypid);
	instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2, false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]); pfree(instants);

	pfree(DatumGetPointer(crossnp));
	pfree(DatumGetPointer(crossgeom));
	FREE_DATUM(startvalue, valuetypid); FREE_DATUM(crossvalue, valuetypid); 
	FREE_DATUM(endvalue, valuetypid);
	*count = 3;
	return;
}

static TemporalSeq **
tspatialrel_tnpointseq_tnpointseq2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Oid valuetypid, int *count)
{
	if (seq1->count == 1)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		Datum geom1 = tnpointinst_geom(inst1);
		Datum geom2 = tnpointinst_geom(inst2);
		Datum value = operator(geom1, geom2);

		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst = temporalinst_make(value, inst1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);

		pfree(inst); pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
		FREE_DATUM(value, valuetypid);
		*count = 1;
		return result;
	}

	/* Temporal sequences have at least two instants */
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count-1) ? seq1->period.upper_inc : false;
		tspatialrel_tnpointseq_tnpointseq1(&result[k], start1, end1,
			start2, end2, lower_inc, upper_inc, operator, valuetypid, &countseq);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	*count = k;
	return result;
}

static TemporalS *
tspatialrel_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	int count;
	TemporalSeq **sequences = tspatialrel_tnpointseq_tnpointseq2(
		seq1, seq2, operator, valuetypid, &count);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

static TemporalS *
tspatialrel_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2,
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	int *countseqs = palloc0(sizeof(int) * ts1->count);
	int totalseqs = 0, countseq;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = tspatialrel_tnpointseq_tnpointseq2(seq1, seq2, operator,
			valuetypid, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);

	return result;
}

/*****************************************************************************/

static TemporalInst *
tspatialrel3_tnpointinst_tnpointinst(TemporalInst *inst1, TemporalInst *inst2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum geom1 = tnpointinst_geom(inst1);
	Datum geom2 = tnpointinst_geom(inst2);
	Datum value = operator(geom1, geom2, param);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);

	pfree(DatumGetPointer(geom1));
	pfree(DatumGetPointer(geom2));
	FREE_DATUM(value, valuetypid);
	return result;
}

static TemporalI *
tspatialrel3_tnpointi_tnpointi(TemporalI *ti1, TemporalI *ti2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti1->count);
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		Datum geom1 = tnpointinst_geom(inst1);
		Datum geom2 = tnpointinst_geom(inst2);
		Datum value = operator(geom1, geom2, param);
		instants[i] = temporalinst_make(value, inst1->t, valuetypid);

		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom1));
		pfree(DatumGetPointer(geom2));
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti1->count);

	for (int i = 0; i < ti1->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static void
tspatialrel3_tnpointseq_tnpointseq1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2, 
	bool lower_inc, bool upper_inc, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, int *count)
{
	npoint *startnp1 = DatumGetNpoint(temporalinst_value(start1));
	npoint *endnp1 = DatumGetNpoint(temporalinst_value(end1));
	npoint *startnp2 = DatumGetNpoint(temporalinst_value(start2));
	npoint *endnp2 = DatumGetNpoint(temporalinst_value(end2));
	TemporalInst *instants[2];

	Datum startgeom1 = npoint_geom_internal(startnp1);
	Datum startgeom2 = npoint_geom_internal(startnp2);
	Datum startvalue = operator(startgeom1, startgeom2, param);
	pfree(DatumGetPointer(startgeom1)); pfree(DatumGetPointer(startgeom2));

	/* If two segments are equal or constant */
	if ((eq_npoint_npoint_internal(startnp1, startnp2) && eq_npoint_npoint_internal(endnp1, endnp2)) ||
		(startnp1->pos == endnp1->pos && startnp2->pos == endnp2->pos))
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);

		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid);
		*count = 1;
		return;
	}

	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	int cross = tnpointseq_geom_intersect_at_timestamp(start1, end1, start2, end2,
		lower_inc, upper_inc, &crosstime);

	/* If there is no crossing */
	if (cross == 0)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);

		pfree(instants[0]); pfree(instants[1]); pfree(instants);
		FREE_DATUM(startvalue, valuetypid);
		*count = 1;
		return;
	}

	Datum endgeom1 = npoint_geom_internal(endnp1);
	Datum endgeom2 = npoint_geom_internal(endnp2);
	Datum endvalue = operator(endgeom1, endgeom2, param);
	pfree(DatumGetPointer(endgeom1)); pfree(DatumGetPointer(endgeom2));

	/* If there are 2 crossings */
	if (cross == 2)
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]);

		/* Compute the operator in the middle time */
		npoint *intnp1 = npoint_constructor_internal(startnp1->rid, (startnp1->pos + endnp1->pos) / 2);
		Datum intgeom1 = npoint_geom_internal(intnp1);
		npoint *intnp2 = npoint_constructor_internal(startnp2->rid, (startnp2->pos + endnp2->pos) / 2);
		Datum intgeom2 = npoint_geom_internal(intnp2);
		Datum intvalue = operator(intgeom1, intgeom2, param);
		instants[0] = temporalinst_make(intvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(intvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2, false, false, false);
		pfree(instants[0]); pfree(instants[1]);

		/* Compute the operator at the end instant */
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[2] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]); pfree(instants);

		pfree(intnp1); pfree(intnp2);
		pfree(DatumGetPointer(intgeom1)); pfree(DatumGetPointer(intgeom2));
		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(intvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid);
		*count = 3;
		return;
	}

	/* There is only 1 crossing
	 * If the crossing is at the start instant */
	if (crosstime == start1->t)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]);

		/* Compute the operator at the end instants */
		instants[0] = temporalinst_make(endvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2, false, upper_inc, false);

		pfree(instants[0]); pfree(instants[1]); pfree(instants);
		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(endvalue, valuetypid);
		*count = 2;
		return;
	}

	/* If the crossing is at the end instant */
	if (crosstime == end1->t)
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);

		/* Compute the operator at the end instants */
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 1, true, true, false);
		pfree(instants[0]); pfree(instants);

		FREE_DATUM(startvalue, valuetypid); FREE_DATUM(endvalue, valuetypid);
		*count = 2;
		return;
	}

	/* The crossing is at the middle */
	/* Compute the operator at the start instants */
	instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
	instants[1] = temporalinst_make(startvalue, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2, lower_inc, false, false);
	pfree(instants[0]); pfree(instants[1]);

	/* Compute the operator at crosstime */
	Datum crossnp = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum crossgeom = npoint_geom_internal(DatumGetNpoint(crossnp));
	Datum crossvalue = operator(crossgeom, crossgeom, param);
	instants[0] = temporalinst_make(crossvalue, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1,
		true, true, false);
	pfree(instants[0]);

	/* Compute the operator at the end instants */
	instants[0] = temporalinst_make(endvalue, crosstime, valuetypid);
	instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2, false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]); pfree(instants);

	pfree(DatumGetPointer(crossnp));
	pfree(DatumGetPointer(crossgeom));
	FREE_DATUM(startvalue, valuetypid); FREE_DATUM(crossvalue, valuetypid); 
	FREE_DATUM(endvalue, valuetypid);
	*count = 3;
	return;
}

static TemporalSeq **
tspatialrel3_tnpointseq_tnpointseq2(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, int *count)
{
	if (seq1->count == 1)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		Datum geom1 = tnpointinst_geom(inst1);
		Datum geom2 = tnpointinst_geom(inst2);
		Datum value = operator(geom1, geom2, param);

		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst = temporalinst_make(value, inst1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);

		pfree(inst); pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
		FREE_DATUM(value, valuetypid);
		*count = 1;
		return result;
	}

	/* Temporal sequences have at least two instants */
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count-1) ? seq1->period.upper_inc : false;
		tspatialrel3_tnpointseq_tnpointseq1(&result[k], start1, end1, 
			start2, end2, lower_inc, upper_inc, param, operator, 
			valuetypid, &countseq);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	*count = k;
	return result;
}

static TemporalS *
tspatialrel3_tnpointseq_tnpointseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	int count;
	TemporalSeq **sequences = tspatialrel3_tnpointseq_tnpointseq2(
		seq1, seq2, param, operator, valuetypid, &count);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

static TemporalS *
tspatialrel3_tnpoints_tnpoints(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	int *countseqs = palloc0(sizeof(int) * ts1->count);
	int totalseqs = 0, countseq;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = tspatialrel3_tnpointseq_tnpointseq2(seq1, seq2, param, operator,
			valuetypid, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);

	return result;
}

/*****************************************************************************
 * Generic binary functions for TNpoint rel Geometry
 * The last argument states whether we are computing tnpoint <trel> geo
 * or geo <trel> tnpoint 
 *****************************************************************************/

TemporalInst *
tspatialrel_tnpointinst_geo(TemporalInst *inst, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	Datum geom = tnpointinst_geom(inst);
	Datum value = invert ? operator(geo, geom) : operator(geom, geo);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	pfree(DatumGetPointer(geom));
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalI *
tspatialrel_tnpointi_geo(TemporalI *ti, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum geom = tnpointinst_geom(inst);
		Datum value = invert ? operator(geo, geom) : operator(geom, geo);
		instants[i] = temporalinst_make(value, inst->t, valuetypid);

		pfree(DatumGetPointer(geom));
		FREE_DATUM(value, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalSeq **
tspatialrel_tnpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2, Datum geo,
	bool lower_inc, bool upper_inc, Datum (*operator)(Datum, Datum),
	Oid valuetypid, int *count, bool invert)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));

	/* Look for intersections */
	Datum line = tnpointseq_trajectory1(inst1, inst2);
	Datum intersections = call_function2(intersection, line, geo);
	if (np1->pos == np2->pos || DatumGetBool(call_function1(LWGEOM_isempty, intersections)))
	{
		Datum geom1 = npoint_geom_internal(np1);
		Datum value = invert ? operator(geo, geom1) : operator(geom1, geo);
		TemporalInst *instants[2];
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);

		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);

		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections));
		pfree(DatumGetPointer(geom1));
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}

	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tnpointseq_intersection_instants(inst1, inst2, line,
		intersections, &countinst);
	pfree(DatumGetPointer(intersections));
	pfree(DatumGetPointer(line));

	/* Determine whether the period started before/after the first/last intersection */
	bool before = (inst1->t != (interinstants[0])->t);
	bool after = (inst2->t != (interinstants[countinst - 1])->t);

	/* Compute the operator */
	int countseq = (2 * countinst) - 1;
	if (before) countseq++;
	if (after) countseq++;
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countseq);
	TemporalInst *instants[2];
	int k = 0;

	if (before)
	{
		Datum geom1 = npoint_geom_internal(np1);
		Datum value = invert ? operator(geo, geom1) : operator(geom1, geo);
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t, 
			valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom1));
		pfree(instants[0]); pfree(instants[1]);
	}

	for (int i = 0; i < countinst; i++)
	{
		/* If the intersection point is not at an exclusive bound,
		 * compute the operator at that point */
		if ((lower_inc || interinstants[i]->t != inst1->t) &&
			(upper_inc || interinstants[i]->t != inst2->t))
		{
			npoint *intnp = DatumGetNpoint(temporalinst_value(interinstants[i]));
			Datum intgeom = npoint_geom_internal(intnp);
			Datum value = invert ? operator(geo, intgeom) : 
				operator(intgeom, geo);
			instants[0] = temporalinst_make(value, (interinstants[i])->t,
				valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			FREE_DATUM(value, valuetypid);
			pfree(DatumGetPointer(intgeom));
			pfree(instants[0]);
		}

		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one
			 * and compute the operator at that point */
			TimestampTz time1 = interinstants[i]->t;
			TimestampTz time2 = interinstants[i+1]->t;
			TimestampTz inttime = time1 + (time2 - time1) / 2;

			Datum intnp = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
			Datum intgeom = npoint_geom_internal(DatumGetNpoint(intnp));
			Datum intvalue = invert ? operator(geo, intgeom) :
				operator(intgeom, geo);
			instants[0] = temporalinst_make(intvalue, time1, 
				valuetypid);
			instants[1] = temporalinst_make(intvalue, time2, 
				valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false);

			FREE_DATUM(intvalue, valuetypid);
			pfree(DatumGetPointer(intgeom));
			pfree(DatumGetPointer(intnp));
			pfree(instants[0]); pfree(instants[1]);
		}
	}

	if (after)
	{
		Datum geom2 = npoint_geom_internal(np2);
		Datum value = invert ? operator(geo, geom2) : 
			operator(geom2, geo);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t,
			valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);

		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom2));
		pfree(instants[0]); pfree(instants[1]);
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);

	*count = k;
	return result;
}

static TemporalSeq **
tspatialrel_tnpointseq_geo2(TemporalSeq *seq, Datum geo,
	Datum (*operator)(Datum, Datum), 
	Oid valuetypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum geom = tnpointinst_geom(inst);
		Datum value = invert ? operator(geo, geom) : operator(geom, geo);
		TemporalInst *instant = temporalinst_make(value,
			inst->t, valuetypid);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(&instant, 1,
			true, true, false);
		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom));
		pfree(instant);
		*count = 1;
		return result;
	}

	/* temporal sequence has at least 2 instants */
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	int totalseqs = 0, countseq;
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel_tnpointseq_geo1(inst1, inst2, geo,
			lower_inc, upper_inc, operator, valuetypid, &countseq, invert);
		countseqs[i] = countseq;
		totalseqs += countseq;
		inst1 = inst2;
		lower_inc = true;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count - 1; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}

	pfree(sequences);
	pfree(countseqs);
	*count = totalseqs;
	return result;
}

static TemporalS *
tspatialrel_tnpointseq_geo(TemporalSeq *seq, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel_tnpointseq_geo2(seq, geo,
		 operator, valuetypid, &count, invert);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

static TemporalS *
tspatialrel_tnpoints_geo(TemporalS *ts, Datum geo,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc(sizeof(int) * ts->count);
	int totalseqs = 0;
	int count;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel_tnpointseq_geo2(seq, geo,
			operator, valuetypid, &count, invert);
		countseqs[i] = count;
		totalseqs += count;
	}

	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences,
			totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************
 * Generic ternary functions for TNpoint rel Geometry
 *****************************************************************************/

static TemporalInst *
tspatialrel3_tnpointinst_geo(TemporalInst *inst, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	Datum geom = tnpointinst_geom(inst);
	Datum value = invert ? operator(geo, geom, param) :
		operator(geom, geo, param);
	TemporalInst *result = temporalinst_make(value, inst->t, 
		valuetypid);
	pfree(DatumGetPointer(geom));
	FREE_DATUM(value, valuetypid);
	return result;
}

static TemporalI *
tspatialrel3_tnpointi_geo(TemporalI *ti, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum geom = tnpointinst_geom(inst);
		Datum value = invert ? operator(geo, geom, param) :
			operator(geom, geo, param);
		instants[i] = temporalinst_make(value, inst->t, 
			valuetypid);
		pfree(DatumGetPointer(geom));
		FREE_DATUM(value, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalSeq **
tspatialrel3_tnpointseq_geo1(TemporalInst *inst1, TemporalInst *inst2, Datum geo, Datum param,
	bool lower_inc, bool upper_inc, Datum (*operator)(Datum, Datum, Datum), 
	Oid valuetypid, int *count, bool invert)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));

	/* Look for intersections */
	Datum line = tnpointseq_trajectory1(inst1, inst2);
	Datum intersections = call_function2(intersection, line, geo);
	if (np1->pos == np2->pos || DatumGetBool(call_function1(LWGEOM_isempty, intersections)))
	{
		Datum geom1 = npoint_geom_internal(np1);
		Datum value = invert ? operator(geo, geom1, param) :
			operator(geom1, geo, param);
		TemporalInst *instants[2];
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);

		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(intersections));
		pfree(DatumGetPointer(geom1));
		pfree(instants[0]); pfree(instants[1]);
		*count = 1;
		return result;
	}

	/* Look for instants of intersections */
	int countinst;
	TemporalInst **interinstants = tnpointseq_intersection_instants(inst1, inst2, line,
		intersections, &countinst);
	pfree(DatumGetPointer(intersections));
	pfree(DatumGetPointer(line));

	/* Determine whether the period started before/after the first/last intersection */
	bool before = (inst1->t != (interinstants[0])->t);
	bool after = (inst2->t != (interinstants[countinst - 1])->t);

	/* Compute the operator */
	int countseq = (2 * countinst) - 1;
	if (before) countseq++;
	if (after) countseq++;
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countseq);
	TemporalInst *instants[2];
	int k = 0;

	if (before)
	{
		Datum geom1 = npoint_geom_internal(np1);
		Datum value = invert ? operator(geo, geom1, param) :
			operator(geom1, geo, param);
		instants[0] = temporalinst_make(value, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value, (interinstants[0])->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom1));
		pfree(instants[0]); pfree(instants[1]);
	}

	for (int i = 0; i < countinst; i++)
	{
		/* If the intersection point is not at an exclusive bound,
		 * compute the operator at that point */
		if ((lower_inc || interinstants[i]->t != inst1->t) &&
			(upper_inc || interinstants[i]->t != inst2->t))
		{
			npoint *intnp = DatumGetNpoint(temporalinst_value(interinstants[i]));
			Datum intgeom = npoint_geom_internal(intnp);
			Datum value = invert ? operator(geo, intgeom, param) :
				operator(intgeom, geo, param);
			instants[0] = temporalinst_make(value, (interinstants[i])->t,
				valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			FREE_DATUM(value, valuetypid);
			pfree(DatumGetPointer(intgeom));
			pfree(instants[0]);
		}

		if (i < countinst - 1)
		{
			/* Find the middle time between current instant and the next one
			 * and compute the operator at that point */
			TimestampTz time1 = interinstants[i]->t;
			TimestampTz time2 = interinstants[i+1]->t;
			TimestampTz inttime = time1 + (time2 - time1) / 2;

			Datum intnp = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
			Datum intgeom = npoint_geom_internal(DatumGetNpoint(intnp));
			Datum intvalue = invert ? operator(geo, intgeom, param) :
				operator(intgeom, geo, param);
			instants[0] = temporalinst_make(intvalue, time1, valuetypid);
			instants[1] = temporalinst_make(intvalue, time2, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false);

			FREE_DATUM(intvalue, valuetypid);
			pfree(DatumGetPointer(intgeom)); pfree(DatumGetPointer(intnp));
			pfree(instants[0]); pfree(instants[1]);
		}
	}

	if (after)
	{
		Datum geom2 = npoint_geom_internal(np2);
		Datum value = invert ? operator(geom2, geo, param) :
			operator(geom2, geo, param);
		instants[0] = temporalinst_make(value, (interinstants[countinst - 1])->t,
			valuetypid);
		instants[1] = temporalinst_make(value, inst2->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom2));
		pfree(instants[0]); pfree(instants[1]);
	}

	for (int i = 0; i < countinst; i++)
		pfree(interinstants[i]);
	pfree(interinstants);

	*count = k;
	return result;
}

static TemporalSeq **
tspatialrel3_tnpointseq_geo2(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), 
	Oid valuetypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum geom = tnpointinst_geom(inst);
		Datum value = invert ? operator(geo, geom, param) :
			operator(geom, geo, param);
		TemporalInst *instant = temporalinst_make(value,
			inst->t, valuetypid);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(&instant, 1,
			true, true, false);
		FREE_DATUM(value, valuetypid);
		pfree(DatumGetPointer(geom));
		pfree(instant);
		*count = 1;
		return result;
	}

	/* temporal sequence has at least 2 instants */
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	int totalseqs = 0, countseq;
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false;
		sequences[i] = tspatialrel3_tnpointseq_geo1(inst1, inst2, geo, param,
			lower_inc, upper_inc, operator, valuetypid, &countseq, invert);
		countseqs[i] = countseq;
		totalseqs += countseq;
		inst1 = inst2;
		lower_inc = true;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count - 1; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}

	pfree(sequences);
	pfree(countseqs);
	*count = totalseqs;
	return result;
}

static TemporalS *
tspatialrel3_tnpointseq_geo(TemporalSeq *seq, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	int count;
	TemporalSeq **sequences = tspatialrel3_tnpointseq_geo2(seq, geo, param,
		operator, valuetypid, &count, invert);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

static TemporalS *
tspatialrel3_tnpoints_geo(TemporalS *ts, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc(sizeof(int) * ts->count);
	int totalseqs = 0;
	int count;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tspatialrel3_tnpointseq_geo2(seq, geo, param,
			operator, valuetypid, &count, invert);
		countseqs[i] = count;
		totalseqs += count;
	}

	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences,
		totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); pfree(sequences); pfree(countseqs);

	return result;
}

/*****************************************************************************
 * Generic tdwithin functions when temporal npoints are moving
 *****************************************************************************/

static TemporalS *
tdwithin_tnpointseq_geo_internal(TemporalSeq *seq, Datum geo, Datum dist, 
	bool invert)
{
	TemporalSeq *distSeq = distance_tnpointseq_geo(seq, geo);
	TemporalS *result = oper4_temporalseq_base_crossdisc(distSeq, dist, 
		&datum2_le2, FLOAT8OID, BOOLOID, invert);
	pfree(distSeq);
	return result;
}

static TemporalS *
tdwithin_tnpoints_geo_internal(TemporalS *ts, Datum geo, Datum dist, 
	bool invert)
{
	TemporalS *distS = distance_tnpoints_geo(ts, geo);
	TemporalS *result = oper4_temporals_base_crossdisc(distS, dist, 
		&datum2_le2, FLOAT8OID, BOOLOID, invert);
	pfree(distS);
	return result;
}

static TemporalS *
tdwithin_tnpointseq_tnpointseq_internal(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum dist)
{
	TemporalSeq *distSeq = distance_tnpointseq_tnpointseq(seq1, seq2);
	TemporalS *result = oper4_temporalseq_base_crossdisc(distSeq, dist, 
		&datum2_le2, FLOAT8OID, BOOLOID, true);
	pfree(distSeq);
	return result;
}

static TemporalS *
tdwithin_tnpoints_tnpoints_internal(TemporalS *ts1, TemporalS *ts2, Datum dist)
{
	TemporalS *distS = distance_tnpoints_tnpoints(ts1, ts2);
	if (distS == NULL)
		return NULL;
	TemporalS *result = oper4_temporals_base_crossdisc(distS, dist, 
		&datum2_le2, FLOAT8OID, BOOLOID, true);
	pfree(distS);
	return result;
}

/*****************************************************************************
 * Generic dispatch functions
 *****************************************************************************/

static Temporal *
tspatialrel_tnpoint_geo(Temporal *temp, Datum geo,
	Datum (*operator)(Datum, Datum), bool invert)
{
	Temporal *result = NULL;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tspatialrel_tnpointinst_geo((TemporalInst *)temp, geo,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tspatialrel_tnpointi_geo((TemporalI *)temp, geo,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tspatialrel_tnpointseq_geo((TemporalSeq *)temp, geo,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tspatialrel_tnpoints_geo((TemporalS *)temp, geo,
			operator, BOOLOID, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

static Temporal *
tspatialrel_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum))
{
	Temporal *result = NULL;
	if (temp1->type == TEMPORALINST)
		result = (Temporal *)tspatialrel_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALI)
		result = (Temporal *)tspatialrel_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALSEQ)
		result = (Temporal *)tspatialrel_tnpointseq_tnpointseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALS)
		result = (Temporal *)tspatialrel_tnpoints_tnpoints(
			(TemporalS *)temp1, (TemporalS *)temp2, operator, 
			BOOLOID);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************/

static Temporal *
tspatialrel3_tnpoint_geo(Temporal *temp, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Temporal *result = NULL;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tspatialrel3_tnpointinst_geo((TemporalInst *)temp, geo, param,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tspatialrel3_tnpointi_geo((TemporalI *)temp, geo, param,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tspatialrel3_tnpointseq_geo((TemporalSeq *)temp, geo, param,
			operator, BOOLOID, invert);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tspatialrel3_tnpoints_geo((TemporalS *)temp, geo, param,
			operator, BOOLOID, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

static Temporal *
tspatialrel3_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Temporal *result = NULL;
	if (temp1->type == TEMPORALINST)
		result = (Temporal *)tspatialrel3_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALI)
		result = (Temporal *)tspatialrel3_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2, param, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALSEQ)
		result = (Temporal *)tspatialrel3_tnpointseq_tnpointseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALS)
		result = (Temporal *)tspatialrel3_tnpoints_tnpoints(
			(TemporalS *)temp1, (TemporalS *)temp2, param, operator, 
			BOOLOID);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************/

static Temporal *
tdwithin_tnpoint_geo_internal(Temporal *temp, Datum geo, Datum param,
	Datum (*operator)(Datum, Datum, Datum), bool invert)
{
	Temporal *result = NULL;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tspatialrel3_tnpointinst_geo((TemporalInst *)temp, 
			geo, param,	operator, BOOLOID, invert);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tspatialrel3_tnpointi_geo((TemporalI *)temp, 
			geo, param,	operator, BOOLOID, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = (Temporal *)tdwithin_tnpointseq_geo_internal((TemporalSeq *)temp, 
			geo, param,	invert);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tdwithin_tnpoints_geo_internal((TemporalS *)temp, 
			geo, param,	invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

static Temporal *
tdwithin_tnpoint_tnpoint_internal(Temporal *temp1, Temporal *temp2, Datum param,
	Datum (*operator)(Datum, Datum, Datum))
{
	Temporal *result = NULL;
	if (temp1->type == TEMPORALINST)
		result = (Temporal *)tspatialrel3_tnpointinst_tnpointinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALI)
		result = (Temporal *)tspatialrel3_tnpointi_tnpointi(
			(TemporalI *)temp1, (TemporalI *)temp2, param, operator, 
			BOOLOID);
	else if (temp1->type == TEMPORALSEQ)
		result = (Temporal *)tdwithin_tnpointseq_tnpointseq_internal(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, param);
	else if (temp1->type == TEMPORALS)
		result = (Temporal *)tdwithin_tnpoints_tnpoints_internal(
			(TemporalS *)temp1, (TemporalS *)temp2, param);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcontains_geo_tnpoint);

PGDLLEXPORT Datum
tcontains_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_contains, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcontains_tnpoint_geo);

PGDLLEXPORT Datum
tcontains_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_contains, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcontains_tnpoint_tnpoint);

PGDLLEXPORT Datum
tcontains_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_contains);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcovers_geo_tnpoint);

PGDLLEXPORT Datum
tcovers_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_covers, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcovers_tnpoint_geo);

PGDLLEXPORT Datum
tcovers_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_covers, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcovers_tnpoint_tnpoint);

PGDLLEXPORT Datum
tcovers_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_covers);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal coveredby
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcoveredby_geo_tnpoint);

PGDLLEXPORT Datum
tcoveredby_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_coveredby, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcoveredby_tnpoint_geo);

PGDLLEXPORT Datum
tcoveredby_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_coveredby, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tcoveredby_tnpoint_tnpoint);

PGDLLEXPORT Datum
tcoveredby_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_coveredby);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdisjoint_geo_tnpoint);

PGDLLEXPORT Datum
tdisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_disjoint, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tnpoint_geo);

PGDLLEXPORT Datum
tdisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_disjoint, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tnpoint_tnpoint);

PGDLLEXPORT Datum
tdisjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_disjoint);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal equals
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tequals_geo_tnpoint);

PGDLLEXPORT Datum
tequals_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_equals, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tequals_tnpoint_geo);

PGDLLEXPORT Datum
tequals_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_equals, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tequals_tnpoint_tnpoint);

PGDLLEXPORT Datum
tequals_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_equals);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tintersects_geo_tnpoint);

PGDLLEXPORT Datum
tintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_intersects2d, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tintersects_tnpoint_geo);

PGDLLEXPORT Datum
tintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_intersects2d, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(tintersects_tnpoint_tnpoint);

PGDLLEXPORT Datum
tintersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_intersects2d);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(ttouches_geo_tnpoint);

PGDLLEXPORT Datum
ttouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_touches, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(ttouches_tnpoint_geo);

PGDLLEXPORT Datum
ttouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_touches, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(ttouches_tnpoint_tnpoint);

PGDLLEXPORT Datum
ttouches_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_touches);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal within
 *****************************************************************************/

PG_FUNCTION_INFO_V1(twithin_geo_tnpoint);

PGDLLEXPORT Datum
twithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_within, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(twithin_tnpoint_geo);

PGDLLEXPORT Datum
twithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_within, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(twithin_tnpoint_tnpoint);

PGDLLEXPORT Datum
twithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_within);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdwithin_geo_tnpoint);

PGDLLEXPORT Datum
tdwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *result = tdwithin_tnpoint_geo_internal(temp, geo, dist, 
		&geom_dwithin2d, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tnpoint_geo);

PGDLLEXPORT Datum
tdwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *result = tdwithin_tnpoint_geo_internal(temp, geo, dist, 
		&geom_dwithin2d, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tnpoint_tnpoint);

PGDLLEXPORT Datum
tdwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum dist = PG_GETARG_DATUM(2);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tdwithin_tnpoint_tnpoint_internal(sync1, sync2, dist, 
		&geom_dwithin2d);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal relate
 *****************************************************************************/

PG_FUNCTION_INFO_V1(trelate_geo_tnpoint);

PGDLLEXPORT Datum
trelate_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo,
		&geom_relate, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(trelate_tnpoint_geo);

PGDLLEXPORT Datum
trelate_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Temporal *result = tspatialrel_tnpoint_geo(temp, geo, 
		&geom_relate, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(trelate_tnpoint_tnpoint);

PGDLLEXPORT Datum
trelate_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel_tnpoint_tnpoint(sync1, sync2, &geom_relate);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal relate_pattern
 *****************************************************************************/

PG_FUNCTION_INFO_V1(trelate_pattern_geo_tnpoint);

PGDLLEXPORT Datum
trelate_pattern_geo_tnpoint(PG_FUNCTION_ARGS)
{
	Datum geo = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	Temporal *result = tspatialrel3_tnpoint_geo(temp, geo, pattern, 
		&geom_relate_pattern, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tnpoint_geo);

PGDLLEXPORT Datum
trelate_pattern_tnpoint_geo(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geo = PG_GETARG_DATUM(1);
	Datum pattern = PG_GETARG_DATUM(2);
	Temporal *result = tspatialrel3_tnpoint_geo(temp, geo, pattern, 
		&geom_relate_pattern, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(trelate_pattern_tnpoint_tnpoint);

PGDLLEXPORT Datum
trelate_pattern_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Datum pattern = PG_GETARG_DATUM(2);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, true))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}
	
	Temporal *result = tspatialrel3_tnpoint_tnpoint(sync1, sync2, pattern, 
		&geom_relate_pattern);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
