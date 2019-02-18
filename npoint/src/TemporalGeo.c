/*****************************************************************************
 *
 * TemporalGeo.c
 *	  Geospatial functions for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

Datum
tnpointseq_trajectory(TemporalSeq *seq)
{
	Datum *trajs = palloc(sizeof(Datum) * (seq->count - 1));
	npoint *np1 = DatumGetNpoint(temporalinst_value(temporalseq_inst_n(seq, 0)));
	Datum line = route_geom_with_rid(np1->rid);
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		npoint *np2 = DatumGetNpoint(temporalinst_value(temporalseq_inst_n(seq, i)));
		if (np1->pos < np2->pos)
		{
			if (np1->pos == 0 && np2->pos == 1)
				trajs[k++] = PointerGetDatum(gserialized_copy(
					(GSERIALIZED *)PG_DETOAST_DATUM(line)));
			else
				trajs[k++] = call_function3(LWGEOM_line_substring, line,
					Float8GetDatum(np1->pos), Float8GetDatum(np2->pos));
		}
		else if (np1->pos > np2->pos)
		{
			Datum traj;
			if (np2->pos == 0 && np1->pos == 1)
				traj = PointerGetDatum(gserialized_copy(
					(GSERIALIZED *)PG_DETOAST_DATUM(line)));
			else
				traj = call_function3(LWGEOM_line_substring, line,
					Float8GetDatum(np2->pos), Float8GetDatum(np1->pos));
			trajs[k++] = call_function1(LWGEOM_reverse, traj);
			pfree(DatumGetPointer(traj));
		}
		np1 = np2;
	}

	Datum result;
	if (k == 0)
	{
		result = npoint_geom_internal(np1);
	}
	else
	{
		ArrayType *array = datumarr_to_array(trajs, k, type_oid(T_GEOMETRY));
		result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
	}

	pfree(DatumGetPointer(line));
	pfree(trajs);
	return result;
}

Datum
tnpointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));

	if (np1->rid != np2->rid)
		PG_RETURN_POINTER(NULL);
	if (np1->pos == np2->pos)
		return npoint_geom_internal(np1);

	Datum line = route_geom_with_rid(np1->rid);
	Datum traj;
	if (np1->pos < np2->pos)
	{
		if (np1->pos == 0 && np2->pos == 1)
			traj = PointerGetDatum(gserialized_copy(
				(GSERIALIZED *)PG_DETOAST_DATUM(line)));
		else
			traj = call_function3(LWGEOM_line_substring, line,
				Float8GetDatum(np1->pos), Float8GetDatum(np2->pos));
	}
	else
	{
		Datum traj2;
		if (np2->pos == 0 && np1->pos == 1)
			traj2 = PointerGetDatum(gserialized_copy(
				(GSERIALIZED *)PG_DETOAST_DATUM(line)));
		else
			traj2 = call_function3(LWGEOM_line_substring, line,
				Float8GetDatum(np2->pos), Float8GetDatum(np1->pos));
		traj = call_function1(LWGEOM_reverse, traj2);
		pfree(DatumGetPointer(traj2));
	}

	pfree(DatumGetPointer(line));
	return traj;
}

Datum
tnpoints_trajectory(TemporalS *ts)
{
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		return tnpointseq_trajectory(seq);
	}

	Datum *points = palloc(sizeof(Datum) * ts->count);
	Datum *segments = palloc(sizeof(Datum) * ts->count);
	int j = 0, k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		Datum traj = tnpointseq_trajectory(seq);
		GSERIALIZED *gstraj = (GSERIALIZED *)PG_DETOAST_DATUM(traj);
		if (gserialized_get_type(gstraj) == POINTTYPE)
			points[j++] = traj;
		else
			segments[k++] = traj;
		POSTGIS_FREE_IF_COPY_P(gstraj, DatumGetPointer(traj));
	}

	Datum multipoint = (Datum) 0, multilinestring = (Datum) 0;  /* make compiler quiet */
	if (j > 0)
	{
		if (j == 1)
		{
			GSERIALIZED *gspoint = (GSERIALIZED *)PG_DETOAST_DATUM(points[0]);
			multipoint = PointerGetDatum(gserialized_copy(gspoint));
		}
		else
		{
			ArrayType *array = datumarr_to_array(points, j, type_oid(T_GEOMETRY));
			multipoint = call_function1(pgis_union_geometry_array,
										 PointerGetDatum(array));
			pfree(array);
		}
	}
	if (k > 0)
	{
		if (k == 1)
		{
			GSERIALIZED *gsline = (GSERIALIZED *)PG_DETOAST_DATUM(segments[0]);
			multilinestring = PointerGetDatum(gserialized_copy(gsline));
		}
		else
		{
			ArrayType *array = datumarr_to_array(segments, k, type_oid(T_GEOMETRY));
			Datum lines = call_function1(LWGEOM_collect_garray,
				PointerGetDatum(array));
			multilinestring = call_function1(linemerge, lines);
			pfree(array);
		}
	}

	Datum result;
	if (j > 0 && k > 0)
	{
		result = call_function2(geomunion, multipoint, multilinestring);
		pfree(DatumGetPointer(multipoint)); pfree(DatumGetPointer(multilinestring));
	}
	else if (j > 0)
		result = multipoint;
	else
		result = multilinestring;

	pfree(points); pfree(segments);
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_trajectory);

PGDLLEXPORT Datum
tnpoint_trajectory(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ || temp->type != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	Datum result; 
	if (temp->type == TEMPORALSEQ)
		result = tnpointseq_trajectory((TemporalSeq *)temp);
	else
		result = tnpoints_trajectory((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Geometric positions functions
 * Return the geometric positions covered by the temporal npoint
 *****************************************************************************/

Datum
tnpointinst_geom(TemporalInst *inst)
{
	npoint *np = DatumGetNpoint(temporalinst_value(inst));
	return npoint_geom_internal(np);
}

Datum
tnpointi_geom(TemporalI *ti)
{
	int count;
	Datum *values = temporali_values1(ti, &count);
	Datum *geoms = palloc(sizeof(Datum) * count);
	for (int i = 0; i < count; i++)
	{
		npoint *np = DatumGetNpoint(values[i]);
		geoms[i] = npoint_geom_internal(np);
	}
	ArrayType *array = datumarr_to_array(geoms, count, type_oid(T_GEOMETRY));
	Datum result = call_function1(LWGEOM_collect_garray, PointerGetDatum(array));
	pfree(array);
	pfree(geoms);
	return result;
}

Datum
tnpointseq_geom(TemporalSeq *seq)
{
	nregion *nreg = tnpointseq_positions(seq);
	return nregion_geom_internal(nreg);
}

Datum
tnpoints_geom(TemporalS *ts)
{
	nregion *nreg = tnpoints_positions(ts);
	return nregion_geom_internal(nreg);
}

/*****************************************************************************
 * Length and speed functions
 *****************************************************************************/

/* Length traversed by the temporal npoint */

static double
tnpointseq_length_internal(TemporalSeq *seq)
{
	if (seq->count == 1)
		return 0;

	npoint *np1 = DatumGetNpoint(temporalinst_value(temporalseq_inst_n(seq, 0)));
	double route_length = route_length_with_rid(np1->rid);
	double fraction = 0;
	for (int i = 1; i < seq->count; i++)
	{
		npoint *np2 = DatumGetNpoint(temporalinst_value(temporalseq_inst_n(seq, i)));
		fraction += fabs(np2->pos - np1->pos);
		np1 = np2;
	}
	return route_length * fraction;
}

static double
tnpoints_length_internal(TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result += tnpointseq_length_internal(seq);
	}
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_length);

PGDLLEXPORT Datum
tnpoint_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ || temp->type != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	double result; 
	if (temp->type == TEMPORALSEQ)
		result = tnpointseq_length_internal((TemporalSeq *)temp);	
	else
		result = tnpoints_length_internal((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_FLOAT8(result);
}

/* Cumulative length traversed by the temporal npoint */

static TemporalSeq *
tnpointseq_cumulative_length_internal(TemporalSeq *seq, double prevlength)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_make(Float8GetDatum(prevlength), 
			inst->t, FLOAT8OID);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		return result;
	}

	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	double route_length = route_length_with_rid(np1->rid);
	double length = prevlength;
	instants[0] = temporalinst_make(Float8GetDatum(length), inst1->t, FLOAT8OID);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		npoint *np2 = DatumGetNpoint(temporalinst_value(inst1));
		length += fabs(np2->pos - np1->pos) * route_length;
		instants[i] = temporalinst_make(Float8GetDatum(length), inst2->t,
			FLOAT8OID);
		inst1 = inst2;
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc, false);

	for (int i = 1; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalS *
tnpoints_cumulative_length_internal(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	double length = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tnpointseq_cumulative_length_internal(seq, length);
		TemporalInst *end = temporalseq_inst_n(sequences[i], seq->count - 1);
		length += *(double *)(DatumGetPointer(temporalinst_value(end)));
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences,
		ts->count, false);

	for (int i = 1; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_cumulative_length);

PGDLLEXPORT Datum
tnpoint_cumulative_length(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ || temp->type != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tnpointseq_cumulative_length_internal(
			(TemporalSeq *)temp, 0);	
	else
		result = (Temporal *)tnpoints_cumulative_length_internal(
			(TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/* Speed of the temporal npoint */

static TemporalSeq *
tnpointseq_speed_internal(TemporalSeq *seq)
{
	if (seq->count == 1)
		return NULL;

	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	double route_length = route_length_with_rid(np1->rid);
	TemporalInst *inst2 = NULL; /* make the compiler quiet */
	double speed = 0; /* make the compiler quiet */
	for (int i = 0; i < seq->count - 1; i++)
	{
		inst2 = temporalseq_inst_n(seq, i + 1);
		npoint *np2 = DatumGetNpoint(temporalinst_value(inst1));
		double length = fabs(np2->pos - np1->pos) * route_length;
		speed = length / (((double)(inst2->t) - (double)(inst1->t)) / 1000000);
		instants[i] = temporalinst_make(Float8GetDatum(speed),
			inst1->t, FLOAT8OID);
		inst1 = inst2;
	}
	instants[seq->count-1] = temporalinst_make(Float8GetDatum(speed),
	   inst2->t, FLOAT8OID);
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, seq->count, 
		seq->period.lower_inc, seq->period.upper_inc, true);

	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

static TemporalS *
tnpoints_speed_internal(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalSeq *sequence = tnpointseq_speed_internal(seq);
		if (sequence != NULL)
			sequences[k++] = sequence;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_speed);

PGDLLEXPORT Datum
tnpoint_speed(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ || temp->type != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tnpointseq_speed_internal((TemporalSeq *)temp);	
	else
		result = (Temporal *)tnpoints_speed_internal((TemporalS *)temp);	
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/* Restrict a temporal npoint to a geometry */

TemporalInst *
tnpointinst_at_geometry(TemporalInst *inst, Datum geom)
{
	Datum point = tnpointinst_geom(inst);
	bool flag = DatumGetBool(call_function2(intersects, point, geom));
	pfree(DatumGetPointer(point));
	if (!flag)
		return NULL;
	return temporalinst_make(temporalinst_value(inst), inst->t, 
		inst->valuetypid);
}

TemporalI *
tnpointi_at_geometry(TemporalI *ti, Datum geom)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		Datum point = tnpointinst_geom(inst);
		if (DatumGetBool(call_function2(intersects, point, geom)))
			instants[k++] = temporalinst_make(temporalinst_value(inst), 
				inst->t, ti->valuetypid);
		pfree(DatumGetPointer(point));
	}
	TemporalI *result = NULL;
	if (k != 0)
	{
		result = temporali_from_temporalinstarr(instants, k);
		for (int i = 0; i < k; i++)
			pfree(instants[i]);
	}
	pfree(instants);
	return result;
}

/* This function assumes that inst1 and inst2 have same rid */
static TemporalSeq **
tnpointseq_at_geometry1(TemporalInst *inst1, TemporalInst *inst2,
	bool lower_inc, bool upper_inc, Datum geom, int *count)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));

	/* Constant sequence */
	if (np1->pos == np2->pos)
	{
		Datum point = npoint_geom_internal(np1);
		bool flag = DatumGetBool(call_function2(intersects, point, geom));
		pfree(DatumGetPointer(point));
		if (!flag)
		{
			*count = 0;
			return NULL;
		}

		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = inst2;
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		*count = 1;
		return result;
	}

	/* Look for intersections */
	Datum line = tnpointseq_trajectory1(inst1, inst2);
	Datum intersections = call_function2(intersection, line, geom);
	if (DatumGetBool(call_function1(LWGEOM_isempty, intersections)))
	{
		pfree(DatumGetPointer(line));
		pfree(DatumGetPointer(intersections));
		*count = 0;
		return NULL;
	}

	int countinter = DatumGetInt32(call_function1(
			LWGEOM_numgeometries_collection, intersections));
	TemporalInst *instants[2];
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * countinter);
	int k = 0;
	TimestampTz lower = inst1->t;
	TimestampTz upper = inst2->t;

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
			TimestampTz time = (TimestampTz)(lower + (upper - lower) * fraction);

			/* If the intersection is not at the exclusive bound */
			if ((lower_inc || time > lower) &&
				(upper_inc || time < upper))
			{
				double pos = np1->pos + (np2->pos * fraction - np1->pos * fraction);
				npoint *intnp = npoint_constructor_internal(np1->rid, pos);
				instants[0] = temporalinst_make(PointerGetDatum(intnp), time,
					type_oid(T_NPOINT));
				result[k++] = temporalseq_from_temporalinstarr(instants,
					1, true, true, false);
				pfree(instants[0]);
				pfree(intnp);
			}
		}
		else
		{
			Datum inter1 = call_function2(LWGEOM_pointn_linestring, inter, 1);
			double fraction1 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter1));
			TimestampTz time1 = (TimestampTz)(lower + (upper - lower) * fraction1);
			double pos1 = np1->pos + (np2->pos * fraction1 - np1->pos * fraction1);
			npoint *intnp1 = npoint_constructor_internal(np1->rid, pos1);

			Datum inter2 = call_function2(LWGEOM_pointn_linestring, inter, 2);
			double fraction2 = DatumGetFloat8(call_function2(
				LWGEOM_line_locate_point, line, inter2));
			TimestampTz time2 = (TimestampTz)(lower + (upper - lower) * fraction2);
			double pos2 = np1->pos + (np2->pos * fraction2 - np1->pos * fraction2);
			npoint *intnp2 = npoint_constructor_internal(np1->rid, pos2);

			TimestampTz lower1 = Min(time1, time2);
			TimestampTz upper1 = Max(time1, time2);
			bool lower_inc1 = lower1 == lower? lower_inc : true;
			bool upper_inc1 = upper1 == upper? upper_inc : true;
			instants[0] = temporalinst_make(PointerGetDatum(intnp1), lower1,
				type_oid(T_NPOINT));
			instants[1] = temporalinst_make(PointerGetDatum(intnp2), upper1,
				type_oid(T_NPOINT));
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc1, upper_inc1, false);

			pfree(instants[0]); pfree(instants[1]);
			pfree(DatumGetPointer(inter1)); pfree(DatumGetPointer(inter2));
			pfree(intnp1); pfree(intnp2);
		}
		POSTGIS_FREE_IF_COPY_P(gsinter, DatumGetPointer(inter));
	}

	pfree(DatumGetPointer(line));
	pfree(DatumGetPointer(intersections));

	if (k == 0)
	{
		pfree(result);
		*count = 0;
		return NULL;
	}

	temporalseqarr_sort(result, k);
	*count = k;
	return result;
}

static TemporalSeq **
tnpointseq_at_geometry2(TemporalSeq *seq, Datum geom, int *count)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum point = tnpointinst_geom(inst);
		bool flag = DatumGetBool(call_function2(intersects, point, geom));
		pfree(DatumGetPointer(point));
		if (!flag)
		{
			*count = 0;
			return NULL;
		}
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
		*count = 1;
		return result;
	}

	/* temporal sequence has at least 2 instants */
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * (seq->count - 1));
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	int totalseqs = 0, countseq;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2)? seq->period.upper_inc: false;
		sequences[i] = tnpointseq_at_geometry1(inst1, inst2, 
			lower_inc, upper_inc, geom, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
		inst1 = inst2;
		lower_inc = true;
	}

	if (totalseqs == 0)
	{
		pfree(countseqs);
		pfree(sequences);
		*count = 0;
		return NULL;
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

	pfree(countseqs);
	pfree(sequences);
	*count = totalseqs;
	return result;
}

static TemporalS *
tnpointseq_at_geometry(TemporalSeq *seq, Datum geom)
{
	int count;
	TemporalSeq **sequences = tnpointseq_at_geometry2(seq, geom, &count);
	if (sequences == NULL)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);

	return result;
}

static TemporalS *
tnpoints_at_geometry(TemporalS *ts, Datum geom)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tnpointseq_at_geometry2(seq, geom, &countseqs[i]);
		totalseqs += countseqs[i];
	}

	if (totalseqs == 0)
	{
		pfree(sequences);
		pfree(countseqs);
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
	TemporalS *result = temporals_from_temporalseqarr(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);
	pfree(sequences);
	pfree(countseqs);
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_at_geometry);

PGDLLEXPORT Datum
tnpoint_at_geometry(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum geom = PG_GETARG_DATUM(1);
	Temporal *result = NULL; 
	if (temp->type == TEMPORALINST)
		result = (Temporal *)tnpointinst_at_geometry((TemporalInst *)temp, geom);
	else if (temp->type == TEMPORALI)
		result = (Temporal *)tnpointi_at_geometry((TemporalI *)temp, geom);
	else if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tnpointseq_at_geometry((TemporalSeq *)temp, geom);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tnpoints_at_geometry((TemporalS *)temp, geom);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

static TemporalSeq **
tnpointseq_azimuth1(TemporalInst *inst1, TemporalInst *inst2, bool lower_inc,
	int *count)
{
	npoint *np1 = DatumGetNpoint(temporalinst_value(inst1));
	npoint *np2 = DatumGetNpoint(temporalinst_value(inst2));

	/* Constant segment */
	if (np1->pos == np2->pos)
	{
		*count = 0;
		return NULL;
	}

	/* Find all vertices in the segment */
	Datum traj = tnpointseq_trajectory1(inst1, inst2);
	int countVertices = DatumGetInt32(call_function1(
		LWGEOM_numpoints_linestring, traj));
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (countVertices - 1));
	TemporalInst *instants[2];

	Datum vertex1 = call_function2(LWGEOM_pointn_linestring, traj, Int32GetDatum(1));
	TimestampTz time1 = inst1->t;
	for (int i = 0; i < countVertices - 1; i++)
	{
		Datum vertex2 = call_function2(LWGEOM_pointn_linestring, traj, 
			Int32GetDatum(i + 2));
		double fraction = DatumGetFloat8(call_function2(
			LWGEOM_line_locate_point, traj, vertex2));
		TimestampTz time2 = (TimestampTz)(inst1->t + (inst2->t - inst1->t) * fraction);

		Datum azimuth = call_function2(LWGEOM_azimuth, vertex1, vertex2);
		bool lower_inc1 = (i == 0)? lower_inc: true;
		instants[0] = temporalinst_make(azimuth, time1, FLOAT8OID);
		instants[1] = temporalinst_make(azimuth, time2, FLOAT8OID);
		result[i] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc1, false, false);

		pfree(instants[0]); pfree(instants[1]);
		pfree(DatumGetPointer(vertex1));
		vertex1 = vertex2;
		time1 = time2;
	}

	pfree(DatumGetPointer(traj));
	pfree(DatumGetPointer(vertex1));
	*count = countVertices - 1;
	return result;
}

static TemporalSeq **
tnpointseq_azimuth2(TemporalSeq *seq, int *count)
{
	if (seq->count == 1)
	{
		*count = 0;
		return NULL;
	}

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * (seq->count - 1));
	int *countseqs = palloc0(sizeof(int) * (seq->count - 1));
	int totalseqs = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i + 1);
		sequences[i] = tnpointseq_azimuth1(inst1, inst2, lower_inc, &countseqs[i]);
		totalseqs += countseqs[i];
		inst1 = inst2;
		lower_inc = true;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < seq->count - 1; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}

	pfree(sequences);
	pfree(countseqs);
	*count = totalseqs;
	return allsequences;
}

static TemporalS *
tnpointseq_azimuth(TemporalSeq *seq)
{
	int countseqs;
	TemporalSeq **allsequences = tnpointseq_azimuth2(seq, &countseqs);
	if (countseqs == 0)
		return NULL;
	TemporalS *result = temporals_from_temporalseqarr(allsequences, countseqs, true);

	for (int i = 0; i < countseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);
	return result;
}

static TemporalS *
tnpoints_azimuth(TemporalS *ts)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tnpointseq_azimuth2(seq, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	if (totalseqs == 0)
	{
		pfree(sequences);
		pfree(countseqs);
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
	TemporalS *result = temporals_from_temporalseqarr(allsequences, totalseqs, true);

	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences);
	pfree(sequences);
	pfree(countseqs);
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_azimuth);

PGDLLEXPORT Datum
tnpoint_azimuth(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ || temp->type != TEMPORALS)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Input must be a temporal sequence (set)")));

	Temporal *result; 
	if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tnpointseq_azimuth((TemporalSeq *)temp);
	else
		result = (Temporal *)tnpoints_azimuth((TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
