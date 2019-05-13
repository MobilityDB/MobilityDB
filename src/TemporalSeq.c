/*****************************************************************************
 *
 * TemporalSeq.c
 *	  Basic functions for temporal sequences.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <TemporalTypes.h>

#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/* PRECOMPUTED TRAJECTORIES
 * The memory structure of a TemporalSeq with, e.g., 2 instants and 
 * a precomputed trajectory is as follows
 *
 *	-------------------------------------------------------------------
 *	( TemporalSeq | offset_0 | offset_1 | offset_2 | offset_3 )_X | ...
 *	-------------------------------------------------------------------
 *	------------------------------------------------------------------------
 *	( TemporalInst_0 )_X | ( TemporalInst_1 )_X | ( bbox )_X | ( Traj )_X  |
 *	------------------------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, offset_0 to offset_1
 * are offsets for the corresponding instants, offset_2 is the offset for the 
 * bounding box and offset_3 is the offset for the precomputed trajectory. 
 * Precomputed trajectories are only kept for temporal points of sequence 
 * duration.
 */

/* Pointer to the offset array of a TemporalSeq */

size_t *
temporalseq_offsets_ptr(TemporalSeq *seq)
{
	return (size_t *) (((char *)seq) + sizeof(TemporalSeq));
}

/* Pointer to the first TemporalInst */

char * 
temporalseq_data_ptr(TemporalSeq *seq)
{
	return (char *)seq + double_pad(sizeof(TemporalSeq) + 
		sizeof(size_t) * (seq->count+2));
}

/* N-th TemporalInst of a TemporalSeq */

TemporalInst *
temporalseq_inst_n(TemporalSeq *seq, int index)
{
	size_t *offsets = temporalseq_offsets_ptr(seq);
	return (TemporalInst *) (temporalseq_data_ptr(seq) + offsets[index]);
}

/* Pointer to the bounding box of a TemporalSeq */

void * 
temporalseq_bbox_ptr(TemporalSeq *seq) 
{
	size_t *offsets = temporalseq_offsets_ptr(seq);
	return temporalseq_data_ptr(seq) + offsets[seq->count];
}

/* Copy the bounding box of a TemporalSeq in the first argument */

void 
temporalseq_bbox(void *box, TemporalSeq *seq) 
{
	void *box1 = temporalseq_bbox_ptr(seq);
	size_t bboxsize = temporal_bbox_size(seq->valuetypid);
	memcpy(box, box1, bboxsize);
	return;
}

/* 
 * Are the three temporal instant values collinear?
 * These functions supposes that the segments are not constant.
 */

static bool
float_collinear(double x1, double x2, double x3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	double duration1 = (double)t2 - (double)t1;
	double duration2 = (double)t3 - (double)t2;
	if (duration1 < duration2)
	{
		double ratio = duration1 / duration2;
		x3 = x2 + (x3 - x2) * ratio;
	}
	else if (duration1 > duration2)
	{
		double ratio = duration2 / duration1;
		x1 = x1 + (x2 - x1) * ratio;
	}
	double d1 = x2 - x1;
	double d2 = x3 - x2;
	return (fabs(d1-d2) <= EPSILON);
}

static bool
double2_collinear(double2 *x1, double2 *x2, double2 *x3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	double duration1 = (double)t2 - (double)t1;
	double duration2 = (double)t3 - (double)t2;
	if (duration1 < duration2)
	{
		double ratio = duration1 / duration2;
		x3->a = x2->a + (x3->a - x2->a) * ratio;
		x3->b = x2->b + (x3->b - x2->b) * ratio;
	}
	else if (duration1 > duration2)
	{
		double ratio = duration2 / duration1;
		x1->a = x1->a + (x2->a - x1->a) * ratio;
		x1->b = x1->b + (x2->b - x1->b) * ratio;
	}
	double d1a = x2->a - x1->a;
	double d1b = x2->b - x1->b;
	double d2a = x3->a - x2->a;
	double d2b = x3->b - x2->b;
	return (fabs(d1a-d2a) <= EPSILON && fabs(d1b-d2b) <= EPSILON);
}

#ifdef WITH_POSTGIS
static bool
point_collinear(Datum value1, Datum value2, Datum value3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3, bool hasz)
{
	double duration1 = (double)t2 - (double)t1;
	double duration2 = (double)t3 - (double)t2;
	void *tofree = NULL;
	if (duration1 < duration2)
	{
		double ratio = duration1 / duration2;
		Datum line = geompoint_trajectory(value2, value3);
		value3 = call_function2(LWGEOM_line_interpolate_point, 
			line, Float8GetDatum(ratio));
		pfree(DatumGetPointer(line));
		tofree = DatumGetPointer(value3);
	}
	else if (duration1 > duration2)
	{
		double ratio = duration2 / duration1;
		Datum line = geompoint_trajectory(value1, value2);
		value1 = call_function2(LWGEOM_line_interpolate_point, 
			line, Float8GetDatum(ratio));
		pfree(DatumGetPointer(line)); 
		tofree = DatumGetPointer(value1);
	}
	bool result;
	if (hasz)
	{
		POINT3DZ point1 = datum_get_point3dz(value1);
		POINT3DZ point2 = datum_get_point3dz(value2);
		POINT3DZ point3 = datum_get_point3dz(value3);
		double dx1 = point2.x - point1.x;
		double dy1 = point2.y - point1.y;
		double dz1 = point2.z - point1.z;
		double dx2 = point3.x - point2.x;
		double dy2 = point3.y - point2.y;
		double dz2 = point3.z - point2.z;
		result = fabs(dx1-dx2) <= EPSILON && fabs(dy1-dy2) <= EPSILON && 
			fabs(dz1-dz2) <= EPSILON;
	}
	else
	{
		POINT2D point1 = datum_get_point2d(value1);
		POINT2D point2 = datum_get_point2d(value2);
		POINT2D point3 = datum_get_point2d(value3);
		double dx1 = point2.x - point1.x;
		double dy1 = point2.y - point1.y;
		double dx2 = point3.x - point2.x;
		double dy2 = point3.y - point2.y;
		result = fabs(dx1-dx2) <= EPSILON && fabs(dy1-dy2) <= EPSILON;
	}
	if (tofree != NULL) 
		pfree(tofree);
	return result;
}

static bool
double3_collinear(double3 *x1, double3 *x2, double3 *x3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	double duration1 = (double)t2 - (double)t1;
	double duration2 = (double)t3 - (double)t2;
	if (duration1 < duration2)
	{
		double ratio = duration1 / duration2;
		x3->a = x2->a + (x3->a - x2->a) * ratio;
		x3->b = x2->b + (x3->b - x2->b) * ratio;
		x3->c = x2->c + (x3->c - x2->c) * ratio;
	}
	else if (duration1 > duration2)
	{
		double ratio = duration2 / duration1;
		x1->a = x1->a + (x2->a - x1->a) * ratio;
		x1->b = x1->b + (x2->b - x1->b) * ratio;
		x1->c = x1->c + (x2->c - x1->c) * ratio;
	}
	double d1a = x2->a - x1->a;
	double d1b = x2->b - x1->b;
	double d1c = x2->c - x1->c;
	double d2a = x3->a - x2->a;
	double d2b = x3->b - x2->b;
	double d2c = x3->c - x2->c;
	return (fabs(d1a-d2a) <= EPSILON && fabs(d1b-d2b) <= EPSILON && 
		fabs(d1c-d2c) <= EPSILON);
}

static bool
double4_collinear(double4 *x1, double4 *x2, double4 *x3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	double duration1 = (double)t2 - (double)t1;
	double duration2 = (double)t3 - (double)t2;
	if (duration1 < duration2)
	{
		double ratio = duration1 / duration2;
		x3->a = x2->a + (x3->a - x2->a) * ratio;
		x3->b = x2->b + (x3->b - x2->b) * ratio;
		x3->c = x2->c + (x3->c - x2->c) * ratio;
		x3->d = x2->d + (x3->d - x2->d) * ratio;
	}
	else if (duration1 > duration2)
	{
		double ratio = duration2 / duration1;
		x1->a = x1->a + (x2->a - x1->a) * ratio;
		x1->b = x1->b + (x2->b - x1->b) * ratio;
		x1->c = x1->c + (x2->c - x1->c) * ratio;
		x1->d = x1->d + (x2->c - x1->d) * ratio;
	}
	double d1a = x2->a - x1->a;
	double d1b = x2->b - x1->b;
	double d1c = x2->c - x1->c;
	double d1d = x2->d - x1->d;
	double d2a = x3->a - x2->a;
	double d2b = x3->b - x2->b;
	double d2c = x3->c - x2->c;
	double d2d = x3->d - x2->d;
	return (fabs(d1a-d2a) <= EPSILON && fabs(d1b-d2b) <= EPSILON && 
		fabs(d1c-d2c) <= EPSILON && fabs(d1d-d2d) <= EPSILON);
}

#endif

static bool
datum_collinear(Oid valuetypid, Datum value1, Datum value2, Datum value3,
	TimestampTz t1, TimestampTz t2, TimestampTz t3)
{
	if (valuetypid == FLOAT8OID)
		return float_collinear(DatumGetFloat8(value1), DatumGetFloat8(value2), 
			DatumGetFloat8(value3), t1, t2, t3);
	if (valuetypid == type_oid(T_DOUBLE2))
		return double2_collinear(DatumGetDouble2P(value1), DatumGetDouble2P(value2), 
			DatumGetDouble2P(value3), t1, t2, t3);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value1);
		bool hasz = FLAGS_GET_Z(gs->flags);
		return point_collinear(value1, value2, value3, t1, t2, t3, hasz);
	}
	if (valuetypid == type_oid(T_DOUBLE3))
		return double3_collinear(DatumGetDouble3P(value1), DatumGetDouble3P(value2), 
			DatumGetDouble3P(value3), t1, t2, t3);
	if (valuetypid == type_oid(T_DOUBLE4))
		return double4_collinear(DatumGetDouble4P(value1), DatumGetDouble4P(value2), 
			DatumGetDouble4P(value3), t1, t2, t3);
#endif
	return false;
}

static bool
temporalinst_collinear(TemporalInst *inst1, TemporalInst *inst2, 
	TemporalInst *inst3)
{
	Oid valuetypid = inst1->valuetypid;
	if (valuetypid == FLOAT8OID)
	{
		double x1 = DatumGetFloat8(temporalinst_value(inst1));
		double x2 = DatumGetFloat8(temporalinst_value(inst2));
		double x3 = DatumGetFloat8(temporalinst_value(inst3));
		return float_collinear(x1, x2, x3, inst1->t, inst2->t, inst3->t);
	}
	if (valuetypid == type_oid(T_DOUBLE2))
	{
		double2 *x1 = DatumGetDouble2P(temporalinst_value(inst1));
		double2 *x2 = DatumGetDouble2P(temporalinst_value(inst2));
		double2 *x3 = DatumGetDouble2P(temporalinst_value(inst3));
		return double2_collinear(x1, x2, x3, inst1->t, inst2->t, inst3->t);
	}
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY))
	{
		Datum value1 = temporalinst_value(inst1);
		Datum value2 = temporalinst_value(inst2);
		Datum value3 = temporalinst_value(inst3);
		return point_collinear(value1, value2, value3, 
				inst1->t, inst2->t, inst3->t, MOBDB_FLAGS_GET_Z(inst1->flags));
	}
	if (valuetypid == type_oid(T_DOUBLE3))
	{
		double3 *x1 = DatumGetDouble3P(temporalinst_value(inst1));
		double3 *x2 = DatumGetDouble3P(temporalinst_value(inst2));
		double3 *x3 = DatumGetDouble3P(temporalinst_value(inst3));
		return double3_collinear(x1, x2, x3, inst1->t, inst2->t, inst3->t);
	}
	if (valuetypid == type_oid(T_DOUBLE4))
	{
		double4 *x1 = DatumGetDouble4P(temporalinst_value(inst1));
		double4 *x2 = DatumGetDouble4P(temporalinst_value(inst2));
		double4 *x3 = DatumGetDouble4P(temporalinst_value(inst3));
		return double4_collinear(x1, x2, x3, inst1->t, inst2->t, inst3->t);
	}
#endif
	return false;
}

/*
 * Normalize an array of instants.
 * The function assumes that there are at least 2 instants.
 * The function does not create new instants, it creates an array of pointers
 * to a subset of the instants passed in the first argument.
 */
static TemporalInst **
temporalinstarr_normalize(TemporalInst **instants, int count, int *newcount)
{
	Oid valuetypid = instants[0]->valuetypid;
	TemporalInst **result = palloc(sizeof(TemporalInst *) * count);
	/* Remove redundant instants */ 
	TemporalInst *inst1 = instants[0];
	Datum value1 = temporalinst_value(inst1);
	TemporalInst *inst2 = instants[1];
	Datum value2 = temporalinst_value(inst2);
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(instants[0]->flags);
	result[0] = inst1;
	int k = 1;
	for (int i = 2; i < count; i++)
	{
		TemporalInst *inst3 = instants[i];
		Datum value3 = temporalinst_value(inst3);
		if (
			/* discrete sequences and 2 consecutive instants that have the same value 
				... 1@t1, 1@t2, 2@t3, ... -> ... 1@t1, 2@t3, ...
			*/
			(!continuous && datum_eq(value1, value2, valuetypid))
			||
			/* 3 consecutive float/point instants that have the same value 
				... 1@t1, 1@t2, 1@t3, ... -> ... 1@t1, 1@t3, ...
			*/
			(datum_eq(value1, value2, valuetypid) && datum_eq(value2, value3, valuetypid))
			||
			/* collinear float/point instants that have the same duration
				... 1@t1, 2@t2, 3@t3, ... -> ... 1@t1, 3@t3, ...
			*/
			(datum_collinear(valuetypid, value1, value2, value3, inst1->t, inst2->t, inst3->t))
			)
		{
			inst2 = inst3; value2 = value3;
		} 
		else 
		{
			result[k++] = inst2;
			inst1 = inst2; value1 = value2;
			inst2 = inst3; value2 = value3;
		}
	}
	result[k++] = inst2;
	*newcount = k;
	return result;
}

/*****************************************************************************/

/* Join two temporal sequences 
 * This function is called when normalizing an array of sequences. It supposes
 * that the two sequences are adjacent. The resulting sequence will remove the
 * last and/or the first instant of the first/second sequence depending on the
 * values of the last two Boolean arguments */

static TemporalSeq *
temporalseq_join(TemporalSeq *seq1, TemporalSeq *seq2, bool last1, bool first1)
{
	int count1 = seq1->count;
	int start2 = 0;
	/* If we need to remove also the last instant of the first sequence */
	if (last1) count1--;		
	/* If we need to remove also the last instant of the first sequence */
	if (first1) start2++;		
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 
		(count1 + seq2->count - start2));
	int k = 0;
	for (int i = 0; i < count1; i++)
		instants[k++] = temporalseq_inst_n(seq1, i);
	for (int i = start2; i < seq2->count; i++)
		instants[k++] = temporalseq_inst_n(seq2, i);
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		seq1->period.lower_inc, seq2->period.upper_inc, false);
	
	pfree(instants); 

	return result;
}

/*
 * Normalize an array of temporal sequences values. 
 * It is supposed that each individual sequence is already normalized.
 * The sequences may be non-contiguous but must ordered and non-overlapping.
 * The function creates new sequences and does not free the original sequences.
 */
TemporalSeq **
temporalseqarr_normalize(TemporalSeq **sequences, int count, int *newcount)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count);
	/* seq1 is the sequence to which we try to join subsequent seq2 */
	TemporalSeq *seq1 = sequences[0];
	Oid valuetypid = seq1->valuetypid;
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(seq1->flags);
	bool isnew = false;
	int k = 0;
	for (int i = 1; i < count; i++)
	{
		TemporalSeq *seq2 = sequences[i];
		TemporalInst *last2 = (seq1->count == 1) ? NULL : 
			temporalseq_inst_n(seq1, seq1->count - 2); 
		Datum last2value = (seq1->count == 1) ? 0 : 
			temporalinst_value(last2);
		TemporalInst *last1 = temporalseq_inst_n(seq1, seq1->count - 1);
		Datum last1value = temporalinst_value(last1);
		TemporalInst *first1 = temporalseq_inst_n(seq2, 0);
		Datum first1value = temporalinst_value(first1);
		TemporalInst *first2 = (seq2->count == 1) ? NULL : 
			temporalseq_inst_n(seq2, 1); 
		Datum first2value = (seq2->count == 1) ? 0 : 
			temporalinst_value(first2);
		bool adjacent = 
			timestamp_cmp_internal(seq1->period.upper, seq2->period.lower) == 0 && 
			(seq1->period.upper_inc || seq2->period.lower_inc);
		/* If they are adjacent and not instantaneous */
		if (adjacent && last2 != NULL && first2 != NULL && (
			/* If discrete and the last segment of the first sequence is constant 
			   ..., 1@t1, 1@t2) [1@t2, 1@t3, ... -> ..., 1@t1, 2@t3, ... 
			   ..., 1@t1, 1@t2) [1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ... 
			   ..., 1@t1, 1@t2] (1@t2, 2@t3, ... -> ..., 1@t1, 2@t3, ... 
			 */
			(!continuous && 
			datum_eq(last2value, last1value, valuetypid) && 
			datum_eq(last1value, first1value, valuetypid))
			||			
			/* If the last/first segments are constant and equal 
			   ..., 1@t1, 1@t2] (1@t2, 1@t3, ... -> ..., 1@t1, 1@t3, ... 
			 */
			(datum_eq(last2value, last1value, valuetypid) &&
			datum_eq(last1value, first1value, valuetypid) && 
			datum_eq(first1value, first2value, valuetypid))
			||			
			/* If float/point sequences and collinear last/first segments having the same duration 
			   ..., 1@t1, 2@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 3@t3, ... 
			*/
			(datum_eq(last1value, first1value, valuetypid) && 
			temporalinst_collinear(last2, first1, first2))
			))
		{
			/* Remove the last and first instants of the sequences */
			seq1 = temporalseq_join(seq1, seq2, true, true);
			isnew = true;
		}
		/* If discrete sequences and the first one has an exclusive upper bound, 
		   by definition the first sequence has the last segment constant
		   ..., 1@t1, 1@t2) [2@t2, 3@t3, ... -> ..., 1@t1, 2@t2, 3@t3, ... 
		   ..., 1@t1, 1@t2) [2@t2] -> ..., 1@t1, 2@t2]
		 */
		else if (adjacent && !continuous && !seq1->period.upper_inc)
		{
			/* Remove the last instant of the first sequence */
			seq1 = temporalseq_join(seq1, seq2, true, false);
			isnew = true;
		}
		/* If they are adjacent and have equal last/first value respectively 
			Discrete
			... 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			[1@t1], (1@t1, 2@t2, ... -> ..., 1@t1, 2@t2
			Continuous	
			..., 1@t1, 2@t2), [2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			..., 1@t1, 2@t2], (2@t2, 1@t3, ... -> ..., 1@t1, 2@t2, 1@t3, ...
			..., 1@t1, 2@t2), [2@t2] -> ..., 1@t1, 2@t2]
			[1@t1],(1@t1, 2@t2, ... -> [1@t1, 2@t2, ...
		*/
		else if (adjacent && datum_eq(last1value, first1value, valuetypid))
		{
			/* Remove the first instant of the second sequence */
			seq1 = temporalseq_join(seq1, seq2, false, true);
			isnew = true;
		} 
		else 
		{
			result[k++] = isnew ? seq1 : temporalseq_copy(seq1);
			seq1 = seq2;
			isnew = false;
		}
	}
	result[k++] = isnew ? seq1 : temporalseq_copy(seq1);
	*newcount = k;
	return result;
}

/*****************************************************************************/

/* Construct a TemporalSeq from an array of TemporalInst and the bounds.
 * Depending on the value of the normalize argument, the resulting sequence
 * will be normalized. */
TemporalSeq *
temporalseq_from_temporalinstarr(TemporalInst **instants, int count, 
   bool lower_inc, bool upper_inc, bool normalize)
{
	Oid valuetypid = instants[0]->valuetypid;
	/* Test the validity of the instants and the bounds */
	if (count < 1)
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("A temporal sequence must have at least one temporal instant")));
	if (count == 1 && (!lower_inc || !upper_inc))
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("Instant sequence must have inclusive bounds")));
#ifdef WITH_POSTGIS
	bool isgeo = (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY));
	bool hasz = false;
	int srid;
	if (isgeo)
	{
		hasz = MOBDB_FLAGS_GET_Z(instants[0]->flags);
		srid = tpoint_srid_internal((Temporal *)instants[0]);
	}
#endif
	for (int i = 1; i < count; i++)
	{
		if (timestamp_cmp_internal(instants[i-1]->t, instants[i]->t) >= 0)
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
				errmsg("Invalid timestamps for temporal value")));
#ifdef WITH_POSTGIS
		if (isgeo)
		{
			if (tpoint_srid_internal((Temporal *)instants[i]) != srid)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same SRID")));
			if (MOBDB_FLAGS_GET_Z(instants[i]->flags) != hasz)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same dimensionality")));
		}
#endif
	}
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(instants[0]->flags);
	if (!continuous && count > 1 && !upper_inc &&
		datum_ne(temporalinst_value(instants[count-1]), 
			temporalinst_value(instants[count-2]), valuetypid))
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("Invalid end value for temporal sequence")));

	/* Normalize the array of instants */
	TemporalInst **newinstants = instants;
	int newcount = count;
	if (normalize && count > 2)
		newinstants = temporalinstarr_normalize(instants, count, &newcount);
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < newcount; i++)
		memsize += double_pad(VARSIZE(newinstants[i]));
	/* Precompute the trajectory */
#ifdef WITH_POSTGIS
	bool trajectory = false; /* keep compiler quiet */
	Datum traj = 0; /* keep compiler quiet */
	if (isgeo)
	{
		trajectory = type_has_precomputed_trajectory(valuetypid);  
		if (trajectory)
		{
			/* A trajectory is a geometry/geography, either a point or a 
			 * linestring, which may be self-intersecting */
			traj = tpointseq_make_trajectory(newinstants, newcount);
			memsize += double_pad(VARSIZE(DatumGetPointer(traj)));
		}
	}
#endif
	/* Add the size of the struct and the offset array */
	size_t pdata = double_pad(sizeof(TemporalSeq) + (newcount + 2) * sizeof(size_t));
	/* Create the TemporalSeq */
	TemporalSeq *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->valuetypid = valuetypid;
	result->type = TEMPORALSEQ;
	period_set(&result->period, newinstants[0]->t, newinstants[newcount-1]->t,
		lower_inc, upper_inc);
	MOBDB_FLAGS_SET_CONTINUOUS(result->flags, continuous);
#ifdef WITH_POSTGIS
	if (isgeo)
		MOBDB_FLAGS_SET_Z(result->flags, hasz);
#endif
	/* Initialization of the variable-length part */
	size_t *offsets = temporalseq_offsets_ptr(result);
	size_t pos = 0;
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *)result) + pdata + pos, newinstants[i], 
			VARSIZE(newinstants[i]));
		offsets[i] = pos;
		pos += double_pad(VARSIZE(newinstants[i]));
	}
	/*
	 * Precompute the bounding box 
	 * Only external types have precomputed bounding box, internal types such
	 * as double2, double3, or double4 do not have precomputed bounding box.
	 * For temporal points the bounding box is computed from the trajectory 
	 * for efficiency reasons.
	 */
	if (bboxsize != 0)
	{
		void *bbox = ((char *) result) + pdata + pos;
		if (trajectory)
		{
			geo_to_gbox_internal(bbox, (GSERIALIZED *)DatumGetPointer(traj));
			((GBOX *)bbox)->mmin = (double)(result->period.lower);
			((GBOX *)bbox)->mmax = (double)(result->period.upper);
			FLAGS_SET_M(((GBOX *)bbox)->flags, true);
		}
		else
			temporalseq_make_bbox(bbox, newinstants, newcount, 
				lower_inc, upper_inc);
		offsets[newcount] = pos;
		pos += double_pad(bboxsize);
	}
#ifdef WITH_POSTGIS
	if (isgeo && trajectory)
	{
		offsets[newcount+1] = pos;
		memcpy(((char *) result) + pdata + pos, DatumGetPointer(traj),
			VARSIZE(DatumGetPointer(traj)));
		pfree(DatumGetPointer(traj));
	}
#endif

	if (normalize && count > 2)
		pfree(newinstants);

	return result;
}

/* Copy a temporal sequence */

TemporalSeq *
temporalseq_copy(TemporalSeq *seq)
{
	TemporalSeq *result = palloc0(VARSIZE(seq));
	memcpy(result, seq, VARSIZE(seq));
	return result;
}

/* Binary search of a timestamptz in a TemporalSeq */

int
temporalseq_find_timestamp(TemporalSeq *seq, TimestampTz t) 
{
	int first = 0;
	int last = seq->count - 2;
	int middle = (first + last)/2;
	while (first <= last) 
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq, middle);
		TemporalInst *inst2 = temporalseq_inst_n(seq, middle+1);
		bool lower_inc = (middle == 0) ? seq->period.lower_inc : true;
		bool upper_inc = (middle == seq->count - 2) ? seq->period.upper_inc : false;
		if ((timestamp_cmp_internal(inst1->t, t) < 0 && 
			timestamp_cmp_internal(t, inst2->t) < 0) ||
			(lower_inc && timestamp_cmp_internal(inst1->t, t) == 0) ||
			(upper_inc && timestamp_cmp_internal(inst2->t, t) == 0))
			return middle;
		if (timestamp_cmp_internal(t, inst1->t) <= 0)
			last = middle - 1;
		else
			first = middle + 1;	
		middle = (first + last)/2;
	}
	return -1;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/
 
/* 
 * Intersection of a TemporalSeq and a TemporalInst values. 
 */

bool
intersection_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporalseq_at_timestamp(seq, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst1);
	return true;
}

bool
intersection_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporalseq_temporalinst(seq, inst, inter2, inter1);
}

/* 
 * Intersection of a TemporalSeq and a TemporalI values. Each value keeps  
 * the instants in the intersection of their time spans.
 */

bool
intersection_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			instants1[k] = temporalseq_at_timestamp(seq, inst->t);
			instants2[k++] = inst;
		}
		if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
			break;
	}
	if (k == 0)
	{
		pfree(instants1); pfree(instants2); 
		return false;
	}
	
	*inter1 = temporali_from_temporalinstarr(instants1, k);
	*inter2 = temporali_from_temporalinstarr(instants2, k);
	
	for (int i = 0; i < k; i++) 
		pfree(instants1[i]);
	pfree(instants1); pfree(instants2); 

	return true;
}

bool
intersection_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	TemporalI **inter1, TemporalI **inter2)
{
	return intersection_temporalseq_temporali(seq, ti, inter2, inter1);
}

/* 
 * Intersection two TemporalSeq values. 
 */

bool
intersection_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	TemporalSeq **inter1, TemporalSeq **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return false;
	
	*inter1 = temporalseq_at_period(seq1, inter);
	*inter2 = temporalseq_at_period(seq2, inter);
	pfree(inter);
	return true;
}

/*****************************************************************************
 * Synchronize two TemporalSeq values. The values are split into (redundant)
 * segments defined over the same set of instants covering the intersection
 * of their time spans. Depending on the value of the argument crossings,
 * potential crossings between successive pair of instants are added.
 *****************************************************************************/

bool
temporalseq_add_crossing(TemporalInst *inst1, TemporalInst *next1,
	TemporalInst *inst2, TemporalInst *next2, 
	TemporalInst **cross1, TemporalInst **cross2)
{
	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = temporalseq_intersect_at_timestamp(inst1, next1, 
		inst2, next2, &crosstime);
	if (!cross)
		return false;
	*cross1 = temporalseq_at_timestamp1(inst1, next1, crosstime);
	*cross2 = temporalseq_at_timestamp1(inst2, next2, crosstime);
	return true;
}

bool
synchronize_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	TemporalSeq **sync1, TemporalSeq **sync2, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return false;
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		TemporalInst *inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		TemporalInst *inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		*sync1 = temporalseq_from_temporalinstarr(&inst1, 1, 
			true, true, false);
		*sync2 = temporalseq_from_temporalinstarr(&inst2, 1, 
			true, true, false);
		pfree(inst1); pfree(inst2); pfree(inter);
		return true;
	}
	
	/* 
	 * General case 
	 * seq1 =  ... *	 *   *   *	  *>
	 * seq2 =	   <*			*	 * ...
	 * sync1 =	  <X C * C * C X C X C *>
	 * sync1 =	  <* C X C X C * C * C X>
	 * where X are values added for synchronization and C are values added
	 * for the crossings
	 */
	TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
	TemporalInst *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (timestamp_cmp_internal(inst1->t, inter->lower) < 0)
	{
		inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (timestamp_cmp_internal(inst2->t, inter->lower) < 0)
	{
		inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofreeinst = inst2;
		j = temporalseq_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count * 2);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	while (i < seq1->count && j < seq2->count &&
		(timestamp_cmp_internal(inst1->t, inter->upper) <= 0 ||
		timestamp_cmp_internal(inst2->t, inter->upper) <= 0))
	{
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			inst2 = temporalseq_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else 
		{
			j++;
			inst1 = temporalseq_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant add potential crossing before adding
		   the new instants */
		TemporalInst *cross1, *cross2;
		if (crossings && k > 0 && 
			temporalseq_add_crossing(instants1[k-1], inst1,
				instants2[k-1], inst2, &cross1, &cross2))
		{
			instants1[k] = cross1; instants2[k++] = cross2;
			tofree[l++] = cross1; tofree[l++] = cross2; 
		}
		instants1[k] = inst1; instants2[k++] = inst2;
		if (i == seq1->count || j == seq2->count)
			break;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	if (k == 0)
	{
		pfree(instants1); pfree(instants2); pfree(inter);
		return false;
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	if (! inter->upper_inc && k > 1 && ! MOBDB_FLAGS_GET_CONTINUOUS(seq1->flags))
	{
		if (datum_ne(temporalinst_value(instants1[k-2]), 
			temporalinst_value(instants1[k-1]), seq1->valuetypid))
		{
			inst1 = instants1[k-1];
			instants1[k-1] = temporalinst_make(temporalinst_value(instants1[k-2]),
				instants1[k-1]->t, instants1[k-1]->valuetypid); 
			tofree[l++] = instants1[k-1];
		}
	}
	if (! inter->upper_inc && k > 1 && ! MOBDB_FLAGS_GET_CONTINUOUS(seq2->flags))
	{
		if (datum_ne(temporalinst_value(instants2[k-2]), 
			temporalinst_value(instants2[k-1]), seq2->valuetypid))
		{
			inst2 = instants2[k-1];
			instants2[k-1] = temporalinst_make(temporalinst_value(instants2[k-2]),
				instants2[k-1]->t, instants2[k-1]->valuetypid); 
			tofree[l++] = instants2[k-1];
		}
	}
	*sync1 = temporalseq_from_temporalinstarr(instants1, k, 
		inter->lower_inc, inter->upper_inc, false);
	*sync2 = temporalseq_from_temporalinstarr(instants2, k, 
		inter->lower_inc, inter->upper_inc, false);
	
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(instants1); pfree(instants2); pfree(tofree); pfree(inter);

	return true;
}

/*****************************************************************************/

/*
 * Find the single timestamptz at which the multiplication of two temporal 
 * segments has a local minimum/maximum.
 * The function supposes that the instants are synchronized, i.e.,
 * start1->t = start2->t and end1->t = end2->t 
 */

bool
tnumberseq_mult_maxmin_at_timestamp(TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	double x1 = datum_double(temporalinst_value(start1), start1->valuetypid);
	double x2 = datum_double(temporalinst_value(end1), start1->valuetypid);
	double x3 = datum_double(temporalinst_value(start2), start2->valuetypid);
	double x4 = datum_double(temporalinst_value(end2), start2->valuetypid);
	/* Compute the instants t1 and t2 at which the linear functions of the two
	   segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
	   minimum/maximum exactly at the middle between t1 and t2.
	   To reduce problems related to floating point arithmetic, t1 and t2
	   are shifted, respectively, to 0 and 1 before the computation */
	if ((x2 - x1) == 0 || (x4 - x3) == 0)
		return false;

	double d1 = (-1 * x1) / (x2 - x1);
	double d2 = (-1 * x3) / (x4 - x3);
	double min = Min(d1, d2);
	double max = Max(d1, d2);
	double fraction = min + (max - min)/2;
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		/* Minimum/maximum occurs out of the period */
		return false;

	double duration = (double)(end1->t) - (double)(start1->t);
	*t = (double)(start1->t) + (duration * fraction);
	return true;	
}

/*****************************************************************************/

/*
 * Find the single timestamptz at which two temporal segments intersect.
 * The function supposes that the instants are synchronized, i.e.,
 * start1->t = start2->t and end1->t = end2->t 
 * The function only returns an intersection at the middle, i.e., it returns
 * false if they intersect at a bound.
 */

bool
tnumberseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	double x1 = datum_double(temporalinst_value(start1), start1->valuetypid);
	double x2 = datum_double(temporalinst_value(end1), start1->valuetypid);
	double x3 = datum_double(temporalinst_value(start2), start2->valuetypid);
	double x4 = datum_double(temporalinst_value(end2), start2->valuetypid);
	/* Compute the instant t at which the linear functions of the two segments
	   are equal: at + b = ct + d that is t = (d - b) / (a - c).
	   To reduce problems related to floating point arithmetic, t1 and t2
	   are shifted, respectively, to 0 and 1 before the computation */
	double denum = fabs(x2 - x1 - x4 + x3);
	if (denum == 0)
		/* Parallel segments */
		return false;

	double fraction = fabs((x3 - x1) / denum);
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		/* Intersection occurs out of the period */
		return false;

	double duration = (double)(end1->t) - (double)(start1->t);
	*t = (double)(start1->t) + (duration * fraction);
	return true;	
}

#ifdef WITH_POSTGIS
/* 
 * Determine the instant t at which two temporal periods are at the local 
 * minimum. 
 * The function assumes that the two periods are synchronized, 
 * that they are not instants, and that they are not constant.
 */
bool
tpointseq_min_dist_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	double denum, fraction;
	if (MOBDB_FLAGS_GET_Z(start1->flags)) /* 3D */
	{
		POINT3DZ p1 = datum_get_point3dz(temporalinst_value(start1));
		POINT3DZ p2 = datum_get_point3dz(temporalinst_value(end1));
		POINT3DZ p3 = datum_get_point3dz(temporalinst_value(start2));
		POINT3DZ p4 = datum_get_point3dz(temporalinst_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0.
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		double dx1 = p2.x - p1.x;
		double dy1 = p2.y - p1.y;
		double dz1 = p2.z - p1.z;
		double dx2 = p4.x - p3.x;
		double dy2 = p4.y - p3.y;
		double dz2 = p4.z - p3.z;
		
		double f1 = p3.x * (dx1 - dx2);
		double f2 = p1.x * (dx2 - dx1);
		double f3 = p3.y * (dy1 - dy2);
		double f4 = p1.y * (dy2 - dy1);
		double f5 = p3.z * (dz1 - dz2);
		double f6 = p1.z * (dz2 - dz1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dz1*(dz1-2*dz2) + 
			dx2*dx2 + dy2*dy2 + dz2*dz2;
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
	}
	else /* 2D */
	{
		POINT2D p1 = datum_get_point2d(temporalinst_value(start1));
		POINT2D p2 = datum_get_point2d(temporalinst_value(end1));
		POINT2D p3 = datum_get_point2d(temporalinst_value(start2));
		POINT2D p4 = datum_get_point2d(temporalinst_value(end2));
		/* The following basically computes d/dx (Euclidean distance) = 0.
		   To reduce problems related to floating point arithmetic, t1 and t2
		   are shifted, respectively, to 0 and 1 before computing d/dx */
		double dx1 = p2.x - p1.x;
		double dy1 = p2.y - p1.y;
		double dx2 = p4.x - p3.x;
		double dy2 = p4.y - p3.y;
		
		double f1 = p3.x * (dx1 - dx2);
		double f2 = p1.x * (dx2 - dx1);
		double f3 = p3.y * (dy1 - dy2);
		double f4 = p1.y * (dy2 - dy1);

		denum = dx1*(dx1-2*dx2) + dy1*(dy1-2*dy2) + dy2*dy2 + dx2*dx2;
		/* If the segments are parallel */
		if (denum == 0)
			return false;

		fraction = (f1 + f2 + f3 + f4) / denum;
	}
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		return false;
	double duration = (double)(end1->t) - (double)(start1->t);
	*t = (double)(start1->t) + (duration * fraction);
	return true;
}

bool
tpointseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	if (!tpointseq_min_dist_at_timestamp(start1, end1, start2, end2, t))
		return false;
	Datum value1 = temporalseq_value_at_timestamp1(start1, end1, *t);
	Datum value2 = temporalseq_value_at_timestamp1(start2, end2, *t);
	if (datum_eq(value1, value2, start1->valuetypid))
		return true;
	else
		return false;
}
#endif

/* DoubleN are used for computing avg and centroid aggregates based on sum 
   and thus there is no need to add crossings */
bool
temporalseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *inter)
{
	if ((start1->valuetypid == INT4OID || start1->valuetypid == FLOAT8OID) &&
		(start2->valuetypid == INT4OID || start2->valuetypid == FLOAT8OID))
		return tnumberseq_intersect_at_timestamp(start1, end1, start2, end2, inter);
#ifdef WITH_POSTGIS
	if (start1->valuetypid == type_oid(T_GEOMETRY))
		return tpointseq_intersect_at_timestamp(start1, end1, start2, end2, inter);
	if (start1->valuetypid == type_oid(T_GEOGRAPHY))
	{
		/* For geographies we do as the ST_Intersection function, e.g.
		 * 'SELECT geography(ST_Transform(ST_Intersection(ST_Transform(geometry($1), 
		 * @extschema@._ST_BestSRID($1, $2)), 
		 * ST_Transform(geometry($2), @extschema@._ST_BestSRID($1, $2))), 4326))' */
		Datum line1 = tgeogpointseq_trajectory1(start1, end1);
		Datum line2 = tgeogpointseq_trajectory1(start2, end2);
		Datum bestsrid = call_function2(geography_bestsrid, line1, line2);
		TemporalInst *start1geom1 = tgeogpointinst_as_tgeompointinst(start1);
		TemporalInst *end1geom1 = tgeogpointinst_as_tgeompointinst(end1);
		TemporalInst *start2geom1 = tgeogpointinst_as_tgeompointinst(start2);
		TemporalInst *end2geom1 = tgeogpointinst_as_tgeompointinst(end2);
		TemporalInst *start1geom2 = tgeompointinst_transform(start1, bestsrid);
		TemporalInst *end1geom2 = tgeompointinst_transform(start1, bestsrid);
		TemporalInst *start2geom2 = tgeompointinst_transform(start2, bestsrid);
		TemporalInst *end2geom2 = tgeompointinst_transform(start2, bestsrid);
		bool result = tpointseq_intersect_at_timestamp(start1geom2, end1geom2, 
			start2geom2, end2geom2, inter);
		pfree(DatumGetPointer(line1)); pfree(DatumGetPointer(line2)); 
		pfree(start1geom1); pfree(end1geom1); pfree(start2geom1); pfree(end2geom1);
		pfree(start1geom2); pfree(end1geom2); pfree(start2geom2); pfree(end2geom2);
		return result;
	}
#endif
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
		errmsg("Operation not supported")));
}

/* Duration of the TemporalSeq as a double */

static double
temporalseq_duration_double(TemporalSeq *seq)
{
	double lower = (double)(seq->period.lower);
	double upper = (double)(seq->period.upper);
	return (upper - lower);
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Convert to string */
 
char *
temporalseq_to_string(TemporalSeq *seq, char *(*value_out)(Oid, Datum))
{
	char **strings = palloc((int) (sizeof(char *)) * seq->count);
	size_t outlen = 0;

	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		strings[i] = temporalinst_to_string(inst, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	char *result = palloc(outlen + 3);
	result[outlen] = '\0';
	result[0] = seq->period.lower_inc ? '[' : '(';
	size_t pos = 1;
	for (int i = 0; i < seq->count; i++)
	{
		strcpy(result + pos, strings[i]);
		pos += strlen(strings[i]);
		result[pos++] = ',';
		result[pos++] = ' ';
		pfree(strings[i]);
	}
	result[pos - 2] = seq->period.upper_inc ? ']' : ')';
	result[pos - 1] = '\0';
	pfree(strings);
	return result;
}

/* Send function */

void
temporalseq_write(TemporalSeq *seq, StringInfo buf)
{
	pq_sendint(buf, seq->count, 4);
	pq_sendbyte(buf, seq->period.lower_inc);
	pq_sendbyte(buf, seq->period.upper_inc);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		temporalinst_write(inst, buf);
	}
}
 
/* Receive function */

TemporalSeq *
temporalseq_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	bool lower_inc = (char) pq_getmsgbyte(buf);
	bool upper_inc = (char) pq_getmsgbyte(buf);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
		instants[i] = temporalinst_read(buf, valuetypid);
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		count, lower_inc, upper_inc, true);

	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);

	return result;
}

/*****************************************************************************
 * Append function
 *****************************************************************************/

 /* Append an instant to the end of a temporal */

TemporalSeq *
temporalseq_append_instant(TemporalSeq *seq, TemporalInst *inst)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * (seq->count + 1));
	for (int i = 0; i < seq->count; i++)
		instants[i] = temporalseq_inst_n(seq, i);
	instants[seq->count] = inst;
	return temporalseq_from_temporalinstarr(instants, seq->count + 1, 
		seq->period.lower_inc, seq->period.upper_inc, true);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast a temporal integer as a temporal float */

int
tintseq_as_tfloatseq1(TemporalSeq **result, TemporalSeq *seq)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tintinst_as_tfloatinst(inst);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1); 
		return 1;
	}
	
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	TemporalInst *inst2;
	bool lower_inc = seq->period.lower_inc;
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		TemporalInst *instants[2];
		instants[0] = tintinst_as_tfloatinst(inst1);
		Datum fvalue1 = temporalinst_value(instants[0]);
		instants[1] = temporalinst_make(fvalue1, inst2->t, FLOAT8OID);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc && 
			datum_eq(value1, value2, INT4OID): false;
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		inst1 = inst2;
		value1 = value2;
		lower_inc = true;
		pfree(instants[0]); pfree(instants[1]);
	}
	if (seq->period.upper_inc)
	{
		Datum value1 = temporalinst_value(temporalseq_inst_n(seq, seq->count-2));
		Datum value2 = temporalinst_value(inst2);
		if (datum_ne(value1, value2, INT4OID))
		{
			TemporalInst *inst1 = tintinst_as_tfloatinst(inst2);
			result[k++] = temporalseq_from_temporalinstarr(&inst1, 1,
				true, true, false);
		}
	}
	return k;
}

TemporalS *
tintseq_as_tfloatseq(TemporalSeq *seq)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tintseq_as_tfloatseq1(sequences, seq);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalSeq *
temporalinst_as_temporalseq(TemporalInst *inst)
{
	return temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
}

TemporalSeq *
temporali_as_temporalseq(TemporalI *ti)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal sequence")));
	TemporalInst *inst = temporali_inst_n(ti, 0);
	return temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
}

TemporalSeq *
temporals_as_temporalseq(TemporalS *ts)
{
	if (ts->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal sequence")));
	return temporalseq_copy(temporals_seq_n(ts, 0));
}


/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

/* Set of values taken by the temporal sequence value */

Datum *
tempdiscseq_values1(TemporalSeq *seq)
{
	Datum *result = palloc(sizeof(Datum *) * seq->count);
	for (int i = 0; i < seq->count; i++) 
		result[i] = temporalinst_value(temporalseq_inst_n(seq, i));
	return result;
}

ArrayType *
tempdiscseq_values(TemporalSeq *seq)
{
	Datum *values = tempdiscseq_values1(seq);
	datum_sort(values, seq->count, seq->valuetypid);
	int count = datum_remove_duplicates(values, seq->count, seq->valuetypid);
	ArrayType *result = datumarr_to_array(values, count, seq->valuetypid);
	pfree(values);
	return result;
}

/* Range of a TemporalSeq float */

RangeType *
tfloatseq_range(TemporalSeq *seq)
{
	BOX *box = temporalseq_bbox_ptr(seq);
	Datum min = Float8GetDatum(box->low.x);
	Datum max = Float8GetDatum(box->high.x);
	if (datum_eq(min, max, FLOAT8OID))
		return range_make(min, max, true, true, FLOAT8OID);

	Datum start = temporalinst_value(temporalseq_inst_n(seq, 0));
	Datum end = temporalinst_value(temporalseq_inst_n(seq, seq->count-1));
	Datum lower, upper;
	bool lower_inc, upper_inc;
	if (datum_lt(start, end, FLOAT8OID))
	{
		lower = start; lower_inc = seq->period.lower_inc;
		upper = end; upper_inc = seq->period.upper_inc;
	}
	else
	{
		lower = end; lower_inc = seq->period.upper_inc;
		upper = start; upper_inc = seq->period.lower_inc;
	}
	bool min_inc = datum_lt(min, lower, FLOAT8OID) ||
		(datum_eq(min, lower, FLOAT8OID) && lower_inc);
	bool max_inc = datum_gt(max, upper, FLOAT8OID) ||
		(datum_eq(max, upper, FLOAT8OID) && upper_inc);
	if (!min_inc || !max_inc)
	{
		for (int i = 1; i < seq->count-1; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			if (min_inc || datum_eq(min, temporalinst_value(inst), FLOAT8OID))
				min_inc = true;
			if (max_inc || datum_eq(max, temporalinst_value(inst), FLOAT8OID))
				max_inc = true;
			if (min_inc && max_inc)
				break;
		}
	}
	return range_make(min, max, min_inc, max_inc, FLOAT8OID);
}

ArrayType *
tfloatseq_ranges(TemporalSeq *seq)
{
	RangeType *range = tfloatseq_range(seq);
	ArrayType *result = rangearr_to_array(&range, 1, type_oid(T_FLOATRANGE));
	pfree(range);
	return result;
}

/* Get time */

PeriodSet *
temporalseq_get_time(TemporalSeq *seq)
{
	Period *p = &seq->period;
	PeriodSet *result = periodset_from_periodarr_internal(&p, 1, false);
	return result;
}

/* Bounding box range of a temporal number */

RangeType *
tnumberseq_value_range(TemporalSeq *seq)
{
	BOX *box = temporalseq_bbox_ptr(seq);
	Datum min, max;
	if (seq->valuetypid == INT4OID)
	{
		min = Int32GetDatum(box->low.x);
		max = Int32GetDatum(box->high.x);
	}
	else if (seq->valuetypid == FLOAT8OID)
	{
		min = Float8GetDatum(box->low.x);
		max = Float8GetDatum(box->high.x);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return range_make(min, max, true, true, seq->valuetypid);
}

/* Minimum value */

Datum
temporalseq_min_value(TemporalSeq *seq)
{
	if (seq->valuetypid == INT4OID)
	{
		BOX *box = temporalseq_bbox_ptr(seq);
		return Int32GetDatum((int)(box->low.x));
	}
	if (seq->valuetypid == FLOAT8OID)
	{
		BOX *box = temporalseq_bbox_ptr(seq);
		return Float8GetDatum(box->low.x);
	}
	Datum result = temporalinst_value(temporalseq_inst_n(seq, 0));
	for (int i = 1; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_lt(value, result, seq->valuetypid))
			result = value;
	}
	return result;
}

/* Maximum value */
 
Datum
temporalseq_max_value(TemporalSeq *seq)
{
	if (seq->valuetypid == INT4OID)
	{
		BOX *box = temporalseq_bbox_ptr(seq);
		return Int32GetDatum((int)(box->high.x));
	}
	if (seq->valuetypid == FLOAT8OID)
	{
		BOX *box = temporalseq_bbox_ptr(seq);
		return Float8GetDatum(box->high.x);
	}
	Datum result = temporalinst_value(temporalseq_inst_n(seq, 0));
	for (int i = 1; i < seq->count; i++)
	{
		Datum value = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_gt(value, result, seq->valuetypid))
			result = value;
	}
	return result;
}

/* Duration */

Datum
temporalseq_duration(TemporalSeq *seq)
{
	Interval *result = period_duration_internal(&seq->period);
	return PointerGetDatum(result);
}

/* Bounding period on which the temporal value is defined */

void
temporalseq_timespan(Period *p, TemporalSeq *seq)
{
	Period *p1 = &seq->period;
	period_set(p, p1->lower, p1->upper, p1->lower_inc, p1->upper_inc);
}

/* Instants */

TemporalInst **
temporalseq_instants(TemporalSeq *seq)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
		result[i] = temporalseq_inst_n(seq, i);
	return result;	
}

ArrayType *
temporalseq_instants_array(TemporalSeq *seq)
{
	TemporalInst **instants = temporalseq_instants(seq);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, seq->count);
	pfree(instants);
	return result;	
}

/* Start timestamptz */

Timestamp
temporalseq_start_timestamp(TemporalSeq *seq)
{
	return (temporalseq_inst_n(seq, 0))->t;
}

/* End timestamptz */

Timestamp
temporalseq_end_timestamp(TemporalSeq *seq)
{
	return (temporalseq_inst_n(seq, seq->count - 1))->t;
}

/* Timestamps */

TimestampTz *
temporalseq_timestamps1(TemporalSeq *seq)
{
	TimestampTz *result = palloc(sizeof(TimestampTz) * seq->count);
	for (int i = 0; i < seq->count; i++) 
		result[i] = temporalseq_inst_n(seq, i)->t;
	return result;	
}

ArrayType *
temporalseq_timestamps(TemporalSeq *seq)
{
	TimestampTz *times = temporalseq_timestamps1(seq);	
	ArrayType *result = timestamparr_to_array(times, seq->count);
	pfree(times);
	return result;	
}

/*
 * Is the temporal value ever equal to the value?
 * The function assumes that temporal value and the datum value are of the 
 * same valuetypid.
 */
static bool
tempcontseq_ever_equals1(TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_inc, bool upper_inc, Datum value)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	
	/* Constant segment equal to value */
	if (datum_eq(value1, value2, valuetypid) &&
		datum_eq(value1, value, valuetypid))
		return true;

	/* Test of bounds */
	if (datum_eq(value1, value, valuetypid))
		return lower_inc;
	if (datum_eq(value2, value, valuetypid))
		return upper_inc;

	/* Continuous base type: Interpolation */
	TimestampTz t;
	return tempcontseq_timestamp_at_value(inst1, inst2, value, valuetypid, &t);
}

bool
temporalseq_ever_equals(TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporalseq_bbox(&box1, seq);
		base_to_box(&box2, value, seq->valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return false;
	}

	if (! MOBDB_FLAGS_GET_CONTINUOUS(seq->flags) || seq->count == 1)
	{
		for (int i = 0; i < seq->count; i++) 
		{
			Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
			if (datum_eq(valueinst, value, seq->valuetypid))
				return true;
		}
		return false;
	}
	
	/* Continuous base type */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		if (tempcontseq_ever_equals1(inst1, inst2, lower_inc, upper_inc, value))
			return true;
		inst1 = inst2;
		lower_inc = true;
	}
	return false;
}

/* Is the temporal value always equal to the value? */

bool
temporalseq_always_equals(TemporalSeq *seq, Datum value)
{
	/* Bounding box test */
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporalseq_bbox(&box1, seq);
		base_to_box(&box2, value, seq->valuetypid);
		if (same_box_box_internal(&box1, &box2))
			return true;
		else
			return false;
	}

	/* The following test assumes that the sequence is in normal form */
	if (seq->count > 2)
		return false;
	for (int i = 0; i < seq->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporalseq_inst_n(seq, i));
		if (datum_ne(valueinst, value, seq->valuetypid))
			return false;
	}
	return true;
}

/* Shift the time span of a temporal value by an interval */

TemporalSeq *
temporalseq_shift(TemporalSeq *seq, Interval *interval)
{
	TemporalSeq *result = temporalseq_copy(seq);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		inst->t = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	}
	return result;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/*
 * Timestamp at which a temporal continuous segment takes a value.
 * The function supposes that the value is between the range defined by
 * the values of inst1 and inst2 (both exclusive). 
 */

bool
tempcontseq_timestamp_at_value(TemporalInst *inst1, TemporalInst *inst2, 
	Datum value, Oid valuetypid, TimestampTz *t)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	
	/* Continuous base type: Interpolation */
	double fraction;
	if (inst1->valuetypid == FLOAT8OID)
	{ 
		double dvalue1 = DatumGetFloat8(value1);
		double dvalue2 = DatumGetFloat8(value2);
		double dvalue = datum_double(value, valuetypid);
		double min = Min(dvalue1, dvalue2);
		double max = Max(dvalue1, dvalue2);
		/* if value is to the left or to the right of the range */
		if (dvalue < min || dvalue > max)
			return false;
		
		double range = max - min;
		double partial = dvalue - min;
		fraction = dvalue1 < dvalue2 ?
			partial / range : 1 - partial / range;
	}
#ifdef WITH_POSTGIS
	else if (inst1->valuetypid == type_oid(T_GEOMETRY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
		if (gserialized_is_empty(gs))
		{
    		POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
			return false;
		}

		/* We are sure that the trajectory is a line */
		Datum line = tgeompointseq_trajectory1(inst1, inst2);
		/* The following approximation is essential for the atGeometry function
		   instead of calling the function ST_Intersects(line, value)) */
		bool inter = MOBDB_FLAGS_GET_Z(inst1->flags) ?
			DatumGetBool(call_function3(LWGEOM_dwithin3d, line, value,
				Float8GetDatum(0.001))) :
			DatumGetBool(call_function3(LWGEOM_dwithin, line, value,
				Float8GetDatum(0.001)));
		if (!inter)
		{
			pfree(DatumGetPointer(line));
			return false;
		}

		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			line, value));
		pfree(DatumGetPointer(line));
	}
	else if (inst1->valuetypid == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
		if (gserialized_is_empty(gs))
		{
    		POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
			return false;
		}

		/* We are sure that the trajectory is a line */
		Datum line = tgeogpointseq_trajectory1(inst1, inst2);
		bool inter = DatumGetFloat8(call_function4(geography_distance, line, 
			value, Float8GetDatum(0.0), BoolGetDatum(false))) < 0.00001;
		if (!inter)
		{
			pfree(DatumGetPointer(line));
			return false;
		}
		
		/* There is no function equivalent to LWGEOM_line_locate_point 
		 * for geographies. We do as the ST_Intersection function, e.g.
		 * 'SELECT geography(ST_Transform(ST_Intersection(ST_Transform(geometry($1), 
		 * @extschema@._ST_BestSRID($1, $2)), 
		 * ST_Transform(geometry($2), @extschema@._ST_BestSRID($1, $2))), 4326))' */

		Datum bestsrid = call_function2(geography_bestsrid, line, line);
		Datum line1 = call_function1(geometry_from_geography, line);
		Datum line2 = call_function2(transform, line1, bestsrid);
		Datum value1 = call_function1(geometry_from_geography, value);
		Datum value2 = call_function2(transform, value1, bestsrid);
		fraction = DatumGetFloat8(call_function2(LWGEOM_line_locate_point,
			line2, value2));
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(line1)); 
		pfree(DatumGetPointer(line2)); pfree(DatumGetPointer(value1)); 
		pfree(DatumGetPointer(value2));
	}
#endif
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	if (fabs(fraction) < EPSILON || fabs(fraction-1.0) < EPSILON)
		return false;
	double duration = (double)inst2->t - (double)inst1->t;
	*t = (double)inst1->t + duration * fraction;
	return true;
}
 
/* Restriction to a value for a segment */

static TemporalSeq *
temporalseq_at_value1(TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_inc, bool upper_inc, Datum value)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	
	/* Constant segment (discrete or continuous base type) */
	if (datum_eq(value1, value2, valuetypid))
	{
		/* If not equal to value */
		if (datum_ne(value1, value, valuetypid))
			return NULL;
		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = inst2;
		TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		return result;
	}

	/* Discrete base type */
	if (! MOBDB_FLAGS_GET_CONTINUOUS(inst1->flags))
	{
		TemporalSeq *result = NULL;
		if (datum_eq(value1, value, valuetypid))
		{
			/* <value@t1 x@t2> */
			TemporalInst *instants[2];
			instants[0] = inst1;
			instants[1] = temporalinst_make(value1, inst2->t, valuetypid);
			result = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, false, false);
			pfree(instants[1]);
		}
		else if (upper_inc && datum_eq(value, value2, valuetypid))
		{
			/* <x@t1 value@t2] */
			result = temporalseq_from_temporalinstarr(&inst2, 1,
				true, true, false);
		}
		return result;
	}

	/* Continuous base type: Test of bounds */
	if (datum_eq(value1, value, valuetypid))
	{
		if (!lower_inc)
			return NULL;
		return temporalseq_from_temporalinstarr(&inst1, 1,
				true, true, false);
	}
	if (datum_eq(value2, value, valuetypid))
	{
		if (!upper_inc)
			return NULL;
		return temporalseq_from_temporalinstarr(&inst2, 1, 
				true, true, false);
	}
	
	/* Continuous base type: Interpolation */
	TimestampTz t;
	if (!tempcontseq_timestamp_at_value(inst1, inst2, value, valuetypid, &t))
		return NULL;
	
	TemporalInst *inst = temporalinst_make(value, t, valuetypid);
	TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
		true, true, false);
	pfree(inst);
	return result;
}

/* 
   Restriction to a value.
   This function is called for each sequence of a TemporalS.
*/

int 
temporalseq_at_value2(TemporalSeq **result, TemporalSeq *seq, Datum value)
{
	Oid valuetypid = seq->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporalseq_bbox(&box1, seq);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return 0;			
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0); 
		if (datum_ne(temporalinst_value(inst), value, valuetypid))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		TemporalSeq *seq1 = temporalseq_at_value1(inst1, inst2, 
			lower_inc, upper_inc, value);
		if (seq1 != NULL) 
			result[k++] = seq1;
		inst1 = inst2;
		lower_inc = true;
	}
	return k;
}

TemporalS *
temporalseq_at_value(TemporalSeq *seq, Datum value)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = temporalseq_at_value2(sequences, seq, value);
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* Restriction to the complement of a value for a continuous segment. */

static void
tempcontseq_minus_value1(TemporalSeq **result,
	TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_inc, bool upper_inc, Datum value, int *count)
{
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	TemporalInst *instants[2];
	
	/* Constant segment */
	if (datum_eq(value1, value2, valuetypid))
	{
		/* Equal to value */
		if (datum_eq(value1, value, valuetypid))
		{
			*count = 0;
			return;
		}
		instants[0] = inst1;
		instants[1] = inst2;
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		*count = 1;
		return;
	}

	/* Test of bounds */
	if (datum_eq(value1, value, valuetypid))
	{
		instants[0] = inst1;
		instants[1] = inst2;
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		*count = 1;
		return;
	}
	if (datum_eq(value2, value, valuetypid))
	{
		instants[0] = inst1;
		instants[1] = inst2;
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		*count = 1;
		return;
	}
	
	/* Continuous base type: Interpolation */
	TimestampTz t;
	if (!tempcontseq_timestamp_at_value(inst1, inst2, value, valuetypid, &t))
	{
		instants[0] = inst1;
		instants[1] = inst2;
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		*count = 1;
		return;
	}
	instants[0] = inst1;
	instants[1] = temporalinst_make(value, t, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
	instants[0] = instants[1];
	instants[1] = inst2;
	result[1] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
	pfree(instants[0]);
	*count = 2;
	return;
}

/* 
   Restriction to the complement of a value.
   This function is called for each sequence of a TemporalS. 
*/

int
temporalseq_minus_value2(TemporalSeq **result, TemporalSeq *seq, Datum value)
{
	Oid valuetypid = seq->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporalseq_bbox(&box1, seq);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
		{
			result[0] = temporalseq_copy(seq);
			return 1;
		}
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0); 
		if (datum_eq(temporalinst_value(inst), value, valuetypid))
			return 0;			
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	int k = 0;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(seq->flags))
	{
		/* Discrete base type */
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		bool lower_inc = seq->period.lower_inc;
		int j = 0;
		for (int i = 0; i < seq->count; i++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, i);
			Datum value1 = temporalinst_value(inst);
			if (datum_eq(value1, value, valuetypid))
			{
				if (j > 0)
				{
					instants[j] = temporalinst_make(temporalinst_value(instants[j-1]),
						inst->t, valuetypid);
					bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false;
					result[k++] = temporalseq_from_temporalinstarr(instants, j + 1,
						lower_inc, upper_inc, false);
					pfree(instants[j]);
					j = 0;
				}
				lower_inc = true;
			}
			else
				instants[j++] = inst;
		}
		if (j > 0)
			result[k++] = temporalseq_from_temporalinstarr(instants, j,
				lower_inc, seq->period.upper_inc, false);
		pfree(instants);
	}
	else
	{
		/* Continuous base type */
		int countseq;
		bool lower_inc = seq->period.lower_inc;
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		for (int i = 1; i < seq->count; i++)
		{
			TemporalInst *inst2 = temporalseq_inst_n(seq, i);
			bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
			tempcontseq_minus_value1(&result[k], inst1, inst2, 
				lower_inc, upper_inc, value, &countseq);
			/* The previous step has added between one and two sequences */
			k += countseq;
			inst1 = inst2;
			lower_inc = true;
		}
	}	
	return k;
}

/* Restriction to the complement of a value */

TemporalS *
temporalseq_minus_value(TemporalSeq *seq, Datum value)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(seq->flags))
		maxcount = seq->count;
	else 
		maxcount = seq->count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = temporalseq_minus_value2(sequences, seq, value);
	if (count == 0)
		return NULL;
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 * This function is called for each sequence of a TemporalS. 
 */
int
temporalseq_at_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_at_values(inst, values, count);
		if (inst1 == NULL)
			return 0;
		
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	int k = 0;	
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		for (int j = 0; j < count; j++)
		{
			TemporalSeq *seq1 = temporalseq_at_value1(inst1, inst2, 
				lower_inc, upper_inc, values[j]);
			if (seq1 != NULL) 
				result[k++] = seq1;
		}
		inst1 = inst2;
		lower_inc = true;
	}
	temporalseqarr_sort(result, k);
	return k;
}
	
TemporalS *
temporalseq_at_values(TemporalSeq *seq, Datum *values, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count);
	int newcount = temporalseq_at_values1(sequences, seq, values, count);
	if (newcount == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, newcount, true);
	for (int i = 0; i < newcount; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of an array of values.
 * The function assumes that there are no duplicates values.
 * This function is called for each sequence of a TemporalS. 
 */
int
temporalseq_minus_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_minus_values(inst, values, count);
		if (inst1 == NULL)
			return 0;
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* 
	 * General case
	 * Compute first the temporalseq_at_values, then compute its complement.
	 */
	TemporalS *ts = temporalseq_at_values(seq, values, count);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int newcount = 0;
	if (ps2 != NULL)
	{
		newcount = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	pfree(ts); pfree(ps1); 
	return newcount;
}

TemporalS *
temporalseq_minus_values(TemporalSeq *seq, Datum *values, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count * 2);
	int newcount = temporalseq_minus_values1(sequences, seq, values, count);
	if (newcount == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, newcount, true);
	for (int i = 0; i < newcount; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* Restriction to the range for a segment */

static TemporalSeq *
tnumberseq_at_range1(TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_incl, bool upper_incl, RangeType *range)
{
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(inst1->flags);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Oid valuetypid = inst1->valuetypid;
	/* Discrete base type or constant segment */
	if (!continuous || datum_eq(value1, value2, valuetypid))
	{
		if (!range_contains_elem_internal(typcache, range, value1)) 
			return NULL;

		TemporalInst *instants[2];
		instants[0] = inst1;
		instants[1] = continuous ? inst2 : 
			temporalinst_make(value1, inst2->t, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 2,
			lower_incl, upper_incl, false);
		return result;
	}

	/* Ensure continuous data type */
	if (valuetypid != FLOAT8OID)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
			
	RangeType *valuerange;
	if (datum_eq(value1, value2, valuetypid))
		valuerange = range_make(value1, value2, true, true, valuetypid);
	if (datum_le(value1, value2, valuetypid))
		valuerange = range_make(value1, value2, lower_incl, upper_incl, valuetypid);	
	else
		valuerange = range_make(value2, value1, upper_incl, lower_incl, valuetypid);	
	RangeType *intersect = DatumGetRangeTypeP(call_function2(range_intersect, 
		RangeTypePGetDatum(valuerange), RangeTypePGetDatum(range)));
	if (RangeIsEmpty(intersect))
	{
		pfree(valuerange);
		pfree(intersect);
		return NULL;
	}

	TemporalSeq *result;
	Datum lowervalue = lower_datum(intersect);
	Datum uppervalue = upper_datum(intersect);
	/* Intersection range is a single value */
	if (datum_eq(lowervalue, uppervalue, valuetypid))
	{
		if ((datum_eq(value1, lowervalue, valuetypid) && !lower_incl) ||
			(datum_eq(value2, lowervalue, valuetypid) && !upper_incl))
			return NULL;

		/* Test with inclusive bounds */
		TemporalSeq *newseq = temporalseq_at_value1(inst1, inst2, 
			true, true, lowervalue);
		/* We are sure that both newseq is an instant sequence */
		TemporalInst *inst = temporalseq_inst_n(newseq, 0);
		result = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(newseq); 
	}
	else
	{
		/* Test with inclusive bounds */
		TemporalSeq *newseq1 = temporalseq_at_value1(inst1, inst2, 
			true, true, lowervalue);
		TemporalSeq *newseq2 = temporalseq_at_value1(inst1, inst2, 
			true, true, uppervalue);
		TimestampTz time1 = newseq1->period.lower;
		TimestampTz time2 = newseq2->period.upper;
		/* We are sure that both newseq1 and newseq2 are instant sequences */
		TemporalInst *instants[2];
		bool lower_incl1, upper_incl1;
		if (time1 < time2)
		{
			/* Segment increasing in value */
			instants[0] = temporalseq_inst_n(newseq1, 0);
			instants[1] = temporalseq_inst_n(newseq2, 0);
			lower_incl1 = (timestamp_cmp_internal(time1, inst1->t) == 0) ? 
				lower_incl && lower_inc(intersect) : lower_inc(intersect);
			upper_incl1 = (timestamp_cmp_internal(time2, inst2->t) == 0) ? 
				upper_incl && upper_inc(intersect) : upper_inc(intersect);
		}
		else
		{
			/* Segment decreasing in value */
			instants[0] = temporalseq_inst_n(newseq2, 0);
			instants[1] = temporalseq_inst_n(newseq1, 0);
			lower_incl1 = (timestamp_cmp_internal(time2, inst1->t) == 0) ? 
				lower_incl && upper_inc(intersect) : upper_inc(intersect);
			upper_incl1 = (timestamp_cmp_internal(time1, inst1->t) == 0) ? 
				upper_incl && lower_inc(intersect) : lower_inc(intersect);
		}
		result = temporalseq_from_temporalinstarr(instants, 2,
			lower_incl1, upper_incl1, false);
		pfree(newseq1); pfree(newseq2); 
	}
	pfree(valuerange); pfree(intersect); 

	return result;
}

/*
 * Restriction to the range.
 * This function is called for each sequence of a TemporalS.
 */
int 
tnumberseq_at_range2(TemporalSeq **result, TemporalSeq *seq, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporalseq_bbox(&box1, seq);
	range_to_box_internal(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
		return 0;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	int k = 0;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		TemporalSeq *seq1 = tnumberseq_at_range1(inst1, inst2, 
			lower_inc, upper_inc, range);
		if (seq1 != NULL) 
			result[k++] = seq1;
		inst1 = inst2;
		lower_inc = true;
	}
	return k;
}

TemporalS *
tnumberseq_at_range(TemporalSeq *seq, RangeType *range)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count);
	int count = tnumberseq_at_range2(sequences, seq, range);
	if (count == 0)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}
	
/*
 * Restriction to the complement of a range.
 * This function is called for each sequence of a TemporalS.
 */
int
tnumberseq_minus_range1(TemporalSeq **result, TemporalSeq *seq, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporalseq_bbox(&box1, seq);
	range_to_box_internal(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;

	/*
	 * General case
	 * Compute first tnumberseq_at_range, then compute its complement.
	 */
	TemporalS *ts = tnumberseq_at_range(seq, range);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int count = 0;
	if (ps2 != NULL)
	{
		count = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	
	pfree(ts); pfree(ps1); 

	return count;
}

TemporalS *
tnumberseq_minus_range(TemporalSeq *seq, RangeType *range)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(seq->flags))
		maxcount = seq->count;
	else 
		maxcount = seq->count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumberseq_minus_range1(sequences, seq, range);
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* 
 * Restriction to an array of ranges 
 * This function is called for each sequence of a TemporalS.
 */
int
tnumberseq_at_ranges1(TemporalSeq **result, TemporalSeq *seq, 
	RangeType **normranges, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_at_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return 0;
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	int k = 0;	
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count-1) ? seq->period.upper_inc : false;
		for (int j = 0; j < count; j++)
		{
			TemporalSeq *seq1 = tnumberseq_at_range1(inst1, inst2, 
				lower_inc, upper_inc, normranges[j]);
			if (seq1 != NULL) 
				result[k++] = seq1;
		}
		inst1 = inst2;
		lower_inc = true;
	}
	if (k == 0) 
		return 0;
	
	temporalseqarr_sort(result, k);
	return k;
}

TemporalS *
tnumberseq_at_ranges(TemporalSeq *seq, RangeType **normranges, int count)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * count);
	int newcount = tnumberseq_at_ranges1(sequences, seq, normranges, count);
	if (newcount == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, newcount, true);
	for (int i = 0; i < newcount; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of an array of ranges.
 * This function is called for each sequence of a TemporalS.
 */
int 
tnumberseq_minus_ranges1(TemporalSeq **result, TemporalSeq *seq, RangeType **normranges, int count)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_minus_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return 0;

		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/*  
	 * General case
	 * Compute first the tnumberseq_at_ranges, then compute its complement.
	 */
	TemporalS *ts = tnumberseq_at_ranges(seq, normranges, count);
	if (ts == NULL)
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	PeriodSet *ps1 = temporals_get_time(ts);
	PeriodSet *ps2 = minus_period_periodset_internal(&seq->period, ps1);
	int newcount = 0;
	if (ps2 != NULL)
	{
		newcount = temporalseq_at_periodset1(result, seq, ps2);
		pfree(ps2);
	}
	pfree(ts); pfree(ps1); 
	return newcount;
}	

TemporalS *
tnumberseq_minus_ranges(TemporalSeq *seq, RangeType **normranges, int count)
{
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(seq->flags))
		maxcount = seq->count * count;
	else 
		maxcount = seq->count * count * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int newcount = tnumberseq_minus_ranges1(sequences, seq, normranges, 
		count);
	if (newcount == 0) 
		return NULL;
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, newcount, true);
	for (int i = 0; i < newcount; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* Restriction to the minimum value */

int
temporalseq_at_minmax(TemporalSeq **result, TemporalSeq *seq, Datum value)
{
	int count = temporalseq_at_value2(result, seq, value);
	/* If minimum/maximum is at an exclusive bound */
	if (count == 0)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq, seq->count-1);
		Datum value1 = temporalinst_value(inst1);
		Datum value2 = temporalinst_value(inst2);
		if (datum_eq(value, value1, seq->valuetypid) &&
			datum_eq(value, value2, seq->valuetypid))
		{
			result[0] = temporalseq_from_temporalinstarr(&inst1, 1, true, true, false);
			result[1] = temporalseq_from_temporalinstarr(&inst2, 1, true, true, false);
			count = 2;
		}
		else if (datum_eq(value, value1, seq->valuetypid))
		{
			result[0] = temporalseq_from_temporalinstarr(&inst1, 1, true, true, false);
			count = 1;
		}
		else if (datum_eq(value, value2, seq->valuetypid))
		{
			result[0] = temporalseq_from_temporalinstarr(&inst2, 1, true, true, false);
			count = 1;
		}
	}
	return count;
}

TemporalS *
temporalseq_at_min(TemporalSeq *seq)
{
	Datum minvalue = temporalseq_min_value(seq);
	TemporalSeq *sequences[2];
	int count = temporalseq_at_minmax(sequences, seq, minvalue);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
}

/* Restriction to the complement of the minimum value */

TemporalS *
temporalseq_minus_min(TemporalSeq *seq)
{
	Datum minvalue = temporalseq_min_value(seq);
	return temporalseq_minus_value(seq, minvalue);
}

/* Restriction to the maximum value */

TemporalS *
temporalseq_at_max(TemporalSeq *seq)
{
	Datum maxvalue = temporalseq_max_value(seq);
	TemporalSeq *sequences[2];
	int count = temporalseq_at_minmax(sequences, seq, maxvalue);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
}
 
/* Restriction to the complement of the maximum value */

TemporalS *
temporalseq_minus_max(TemporalSeq *seq)
{
	Datum maxvalue = temporalseq_max_value(seq);
	return temporalseq_minus_value(seq, maxvalue);
}

/*
 * Value that the temporal sequence takes at the timestamp.
 * The function supposes that the timestamp t is between inst1->t and inst2->t
 * (both inclusive). The function creates a new value that must be freed.
 */
Datum
temporalseq_value_at_timestamp1(TemporalInst *inst1, TemporalInst *inst2, 
	TimestampTz t)
{
	Oid valuetypid = inst1->valuetypid;
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(inst1->flags);
	/* Constant segment or t is equal to lower bound */
	if (datum_eq(value1, value2, valuetypid) ||
		timestamp_cmp_internal(inst1->t, t) == 0 ||
		(!continuous && timestamp_cmp_internal(t, inst2->t) < 0))
		return temporalinst_value_copy(inst1);

	/* t is equal to upper bound */
	if (timestamp_cmp_internal(inst2->t, t) == 0)
		return temporalinst_value_copy(inst2);
	
	/* Interpolation for continuous types */
	double duration = (double)inst2->t - (double)inst1->t;	
	double partial = (double)t - (double)inst1->t;
	double ratio = partial / duration;
	if (valuetypid == FLOAT8OID)
	{ 
		double start = DatumGetFloat8(value1);
		double end = DatumGetFloat8(value2);
		double result = start + (end - start) * ratio;
		return Float8GetDatum(result);
	}
	if (valuetypid == type_oid(T_DOUBLE2))
	{
		double2 *start = DatumGetDouble2P(value1);
		double2 *end = DatumGetDouble2P(value2);
		double2 *result = palloc(sizeof(double2));
		result->a = start->a + (end->a - start->a) * ratio;
		result->b = start->b + (end->b - start->b) * ratio;
		return Double2PGetDatum(result);
	}
	if (valuetypid == type_oid(T_DOUBLE3))
	{
		double3 *start = DatumGetDouble3P(value1);
		double3 *end = DatumGetDouble3P(value2);
		double3 *result = palloc(sizeof(double3));
		result->a = start->a + (end->a - start->a) * ratio;
		result->b = start->b + (end->b - start->b) * ratio;
		result->c = start->c + (end->c - start->c) * ratio;
		return Double3PGetDatum(result);
	}
	if (valuetypid == type_oid(T_DOUBLE4))
	{
		double4 *start = DatumGetDouble4P(value1);
		double4 *end = DatumGetDouble4P(value2);
		double4 *result = palloc(sizeof(double4));
		result->a = start->a + (end->a - start->a) * ratio;
		result->b = start->b + (end->b - start->b) * ratio;
		result->c = start->c + (end->c - start->c) * ratio;
		result->d = start->d + (end->d - start->d) * ratio;
		return Double4PGetDatum(result);
	}
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY))
	{
		/* We are sure that the trajectory is a line */
		Datum line = tgeompointseq_trajectory1(inst1, inst2);
		Datum result = call_function2(LWGEOM_line_interpolate_point, 
			line, Float8GetDatum(ratio));
		pfree(DatumGetPointer(line)); 
		return result;
	}
	if (valuetypid == type_oid(T_GEOGRAPHY))
	{
		/* We are sure that the trajectory is a line */
		Datum line = tgeogpointseq_trajectory1(inst1, inst2);
		/* There is no function equivalent to LWGEOM_line_interpolate_point 
		 * for geographies. We do as the ST_Intersection function, e.g.
		 * 'SELECT geography(ST_Transform(ST_Intersection(ST_Transform(geometry($1), 
		 * @extschema@._ST_BestSRID($1, $2)), 
		 * ST_Transform(geometry($2), @extschema@._ST_BestSRID($1, $2))), 4326))' */
		Datum bestsrid = call_function2(geography_bestsrid, line, line);
		Datum line1 = call_function1(geometry_from_geography, line);
		Datum line2 = call_function2(transform, line1, bestsrid);
		Datum point = call_function2(LWGEOM_line_interpolate_point, 
			line, Float8GetDatum(ratio));
		Datum srid = call_function1(LWGEOM_get_srid, value1);
		Datum point1 = call_function2(transform, point, srid);
		Datum result = call_function1(geography_from_geometry, point1);
		pfree(DatumGetPointer(line)); pfree(DatumGetPointer(line1)); 
		pfree(DatumGetPointer(line2)); pfree(DatumGetPointer(point)); 
		/* Cannot pfree(DatumGetPointer(point1)); */
		return result;
	}
#endif
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
		errmsg("Operation not supported")));
}

/*
 * Value at a timestamp.
 */
bool
temporalseq_value_at_timestamp(TemporalSeq *seq, TimestampTz t, Datum *result)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
		return false;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		*result = temporalinst_value_copy(temporalseq_inst_n(seq, 0));
		return true;
	}

	/* General case */
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n + 1);
	*result = temporalseq_value_at_timestamp1(inst1, inst2, t);
	return true;
}

/* 
 * Restriction to a timestamp.
 * The function supposes that the timestamp t is between inst1->t and inst2->t
 * (both inclusive).
 */
TemporalInst *
temporalseq_at_timestamp1(TemporalInst *inst1, TemporalInst *inst2, 
	TimestampTz t)
{
	Datum value = temporalseq_value_at_timestamp1(inst1, inst2, t);
	TemporalInst *result = temporalinst_make(value, t, inst1->valuetypid);
	FREE_DATUM(value, inst1->valuetypid);
	return result;
}

/*
 * Restriction to a timestamp.
 */
TemporalInst *
temporalseq_at_timestamp(TemporalSeq *seq, TimestampTz t)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
		return NULL;

	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalinst_copy(temporalseq_inst_n(seq, 0));
	
	/* General case */
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n + 1);
	return temporalseq_at_timestamp1(inst1, inst2, t);
}

/*
 * Restriction to the complement of a timestamp.
 * This function is called for each sequence of a TemporalS.
 */
int
temporalseq_minus_timestamp1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampTz t)
{
	/* Bounding box test */
	if (!contains_period_timestamp_internal(&seq->period, t))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;
	
	/* General case */
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(seq->flags);
	TemporalInst **instants = palloc0(sizeof(TemporalInst *) * seq->count);
	int k = 0;
	int n = temporalseq_find_timestamp(seq, t);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0), *inst2;
	/* Compute the first sequence until t */
	if (n != 0 || timestamp_cmp_internal(inst1->t, t) < 0)
	{
		for (int i = 0; i < n; i++)
			instants[i] = temporalseq_inst_n(seq, i);
		inst1 = temporalseq_inst_n(seq, n);
		inst2 = temporalseq_inst_n(seq, n + 1);
		if (timestamp_cmp_internal(inst1->t, t) == 0)
		{
			if (continuous)
			{
				instants[n] = inst1;
				result[k++] = temporalseq_from_temporalinstarr(instants, n + 1, 
					seq->period.lower_inc, false, false);
			}
			else
			{
				instants[n] = temporalinst_make(temporalinst_value(instants[n - 1]), t,
					inst1->valuetypid);
				result[k++] = temporalseq_from_temporalinstarr(instants, n + 1, 
					seq->period.lower_inc, false, false);
				pfree(instants[n]);
			}
		}
		else
		{
			instants[n] = inst1;
			if (continuous)
				instants[n + 1] = temporalseq_at_timestamp1(inst1, inst2, t);
			else
				instants[n + 1] = temporalinst_make(temporalinst_value(inst1), t,
					inst1->valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, n + 2, 
				seq->period.lower_inc, false, false);
			pfree(instants[n + 1]);
		}
	}
	/* Compute the second sequence after t */
	inst1 = temporalseq_inst_n(seq, n);
	inst2 = temporalseq_inst_n(seq, n + 1);
	if (timestamp_cmp_internal(t, inst2->t) < 0)
	{
		instants[0] = temporalseq_at_timestamp1(inst1, inst2, t);
		for (int i = 1; i < seq->count - n; i++)
			instants[i] = temporalseq_inst_n(seq, i + n);
		result[k++] = temporalseq_from_temporalinstarr(instants, seq->count - n, 
			false, seq->period.upper_inc, false);
		pfree(instants[0]);
	}
	return k;
}

/*
 * Restriction to the complement of a timestamp.
 */

TemporalS *
temporalseq_minus_timestamp(TemporalSeq *seq, TimestampTz t)
{
	TemporalSeq *sequences[2];
	int count = temporalseq_minus_timestamp1((TemporalSeq **)sequences, seq, t);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
}

/* 
 * Restriction to a timestampset.
 */
TemporalI *
temporalseq_at_timestampset(TemporalSeq *seq, TimestampSet *ts)
{
	/* Bounding box test */
	Period *p = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&seq->period, p))
		return NULL;
	
	/* Instantaneous sequence */
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	if (seq->count == 1)
	{
		if (!contains_timestampset_timestamp_internal(ts, inst->t))
			return NULL;
		return temporali_from_temporalinstarr(&inst, 1);
	}

	/* General case */
	TimestampTz t = Max(seq->period.lower, p->lower);
	int n;
	timestampset_find_timestamp(ts, t, &n);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * (ts->count - n));
	int k = 0;
	for (int i = n; i < ts->count; i++) 
	{
		t = timestampset_time_n(ts, i);
		inst = temporalseq_at_timestamp(seq, t);
		if (inst != NULL)
			instants[k++] = inst;
	}
	if (k == 0)
	{
		pfree(instants);
		return NULL;
	}

	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	for (int i = 0; i < k; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of a timestampset.
 * This function is called for each sequence of a TemporalS.
 */
int 
temporalseq_minus_timestampset1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampSet *ts)
{
	/* Bounding box test */
	Period *p = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&seq->period, p))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_minus_timestampset(inst, ts);
		if (inst1 == NULL)
			return 0;
	
		pfree(inst1); 
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	TemporalSeq *tail = temporalseq_copy(seq);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		if (contains_period_timestamp_internal(&tail->period, t))
		{
			int count = temporalseq_minus_timestamp1(&result[k], tail, t);
			/* result may contain one or two sequences */
			k += count-1;
			pfree(tail);
			tail = result[k];
		}
	}
	result[k++] = tail;
	return k;
}

TemporalS *
temporalseq_minus_timestampset(TemporalSeq *seq, TimestampSet *ts)
{
	TemporalSeq **sequences = palloc0(sizeof(TemporalSeq *) * (ts->count + 1));
	int count = temporalseq_minus_timestampset1(sequences, seq, ts);
	if (count == 0) 
		return NULL;
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/*
 * Restriction to a period.
 */
TemporalSeq *
temporalseq_at_period(TemporalSeq *seq, Period *p)
{
	/* Bounding box test */
	if (!overlaps_period_period_internal(&seq->period, p))
		return NULL;

	/* Instantaneous sequence */
	if (seq->count == 1)
		return temporalseq_copy(seq);

	/* General case */
	Period *inter = intersection_period_period_internal(&seq->period, p);
	/* Intersecting period is instantaneous */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		TemporalInst *inst = temporalseq_at_timestamp(seq, inter->lower);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst); pfree(inter);
		return result;		
	}
	
	int n = temporalseq_find_timestamp(seq, inter->lower);
	/* If the lower bound of the intersecting period is exclusive */
	if (n == -1)
		n = 0;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * (seq->count - n));
	/* Compute the value at the beginning of the intersecting period */
	TemporalInst *inst1 = temporalseq_inst_n(seq, n);
	TemporalInst *inst2 = temporalseq_inst_n(seq, n+1);
	instants[0] = temporalseq_at_timestamp1(inst1, inst2, inter->lower);
	int k = 1;
	for (int i = n+2; i < seq->count; i++)
	{
		/* If the end of the intersecting period is between inst1 and inst2 */
		if (timestamp_cmp_internal(inst1->t, inter->upper) <= 0 &&
			timestamp_cmp_internal(inter->upper, inst2->t) <= 0)
			break;

		inst1 = inst2;
		inst2 = temporalseq_inst_n(seq, i);
		/* If the intersecting period contains inst1 */
		if (timestamp_cmp_internal(inter->lower, inst1->t) <= 0 &&
			timestamp_cmp_internal(inst1->t, inter->upper) <= 0)
			instants[k++] = inst1;
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	if (MOBDB_FLAGS_GET_CONTINUOUS(seq->flags) || inter->upper_inc)
		instants[k++] = temporalseq_at_timestamp1(inst1, inst2, inter->upper);
	else
	{	
		Datum value = temporalinst_value(instants[k-1]);
		instants[k++] = temporalinst_make(value, inter->upper, seq->valuetypid);
	}
	/* Since by definition the sequence is normalized it is not necessary to
	   normalize the projection of the sequence to the period */
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k,
		inter->lower_inc, inter->upper_inc, false);

	pfree(instants[0]); pfree(instants[k-1]); pfree(instants); pfree(inter);
	
	return result;
}

/*
 * Restriction to the complement of a period.
 */

int
temporalseq_minus_period1(TemporalSeq **result, TemporalSeq *seq, Period *p)
{
	/* Bounding box test */
	if (!overlaps_period_period_internal(&seq->period, p))
	{
		result[0] = temporalseq_copy(seq);
		return 1;
	}
	
	/* Instantaneous sequence */
	if (seq->count == 1)
		return 0;

	/* General case */
	PeriodSet *ps = minus_period_period_internal(&seq->period, p);
	if (ps == NULL)
		return 0;
	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		result[i] = temporalseq_at_period(seq, p);
	}
	pfree(ps);
	return ps->count;
}

TemporalS *
temporalseq_minus_period(TemporalSeq *seq, Period *p)
{
	TemporalSeq *sequences[2];
	int count = temporalseq_minus_period1(sequences, seq, p);
	if (count == 0)
	{
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	return result;
}

/*
 * Restriction to a periodset.
 * This function is called for each sequence of a TemporalS.
 */

int
temporalseq_at_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporalseq_timespan(&p1, seq);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return 0;

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		if (!contains_periodset_timestamp_internal(ps, inst->t))
			return 0;
		result[0] = temporalseq_copy(seq);
		return 1;
	}

	/* General case */
	int n;
	periodset_find_timestamp(ps, seq->period.lower, &n);
	int k = 0;
	for (int i = n; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		TemporalSeq *seq1 = temporalseq_at_period(seq, p);
		if (seq1 != NULL)
			result[k++] = seq1;
		if (timestamp_cmp_internal(seq->period.upper, p->upper) < 0)
			break;
	}
	return k;
}

TemporalSeq **
temporalseq_at_periodset2(TemporalSeq *seq, PeriodSet *ps, int *count)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ps->count);
	*count = temporalseq_at_periodset1(result, seq, ps);
	if (*count == 0)
	{
		pfree(result);
		return NULL;
	}
	return result;
}

TemporalS *
temporalseq_at_periodset(TemporalSeq *seq, PeriodSet *ps)
{
	int count;
	TemporalSeq **sequences = temporalseq_at_periodset2(seq, ps, &count);
	if (count == 0)
		return NULL;
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of a periodset.
 */

int
temporalseq_minus_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps, 
	int from, int count)
{
	/* The sequence can be split at most into (count + 1) sequences
		|----------------------|
			|---| |---| |---|
	*/
	TemporalSeq *curr = temporalseq_copy(seq);
	int k = 0;
	for (int i = from; i < count; i++)
	{
		Period *p1 = periodset_per_n(ps, i);
		/* If the remaining periods are to the left of the current period */
		if (period_cmp_bounds(curr->period.upper, p1->lower, false, true,
				curr->period.upper_inc, p1->lower_inc) < 0)
		{
			result[k++] = curr;
			break;
		}
		TemporalSeq *minus[2];
		int countminus = temporalseq_minus_period1(minus, curr, p1);
		pfree(curr);
		/* minus can have from 0 to 2 periods */
		if (countminus == 0)
			break;
		else if (countminus == 1)
			curr = minus[0];
		else /* countminus == 2 */
		{
			result[k++] = minus[0];
			curr = minus[1];
		}
		/* There are no more periods left */
		if (i == count - 1)
			result[k++] = curr;
	}
	return k;
}

TemporalS *
temporalseq_minus_periodset(TemporalSeq *seq, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporalseq_timespan(&p1, seq);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporals_from_temporalseqarr(&seq, 1, false);

	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = temporalinst_minus_periodset(inst, ps);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1); 
		return temporals_from_temporalseqarr(&seq, 1, false);
	}

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ps->count + 1));
	int count = temporalseq_minus_periodset1(sequences, seq, ps,
		0, ps->count);
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result =temporals_from_temporalseqarr(sequences, count, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/* Does the temporal value intersects the timestamp? */

bool
temporalseq_intersects_timestamp(TemporalSeq *seq, TimestampTz t)
{
	return contains_period_timestamp_internal(&seq->period, t);
}

/* Does the temporal value intersects the timestamp set? */
bool
temporalseq_intersects_timestampset(TemporalSeq *seq, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (temporalseq_intersects_timestamp(seq, timestampset_time_n(ts, i))) 
			return true;
	return false;
}

/* Does the temporal value intersects the period? */

bool
temporalseq_intersects_period(TemporalSeq *seq, Period *p)
{
	return overlaps_period_period_internal(&seq->period, p);
}

/* Does the temporal value intersects the period set? */

bool
temporalseq_intersects_periodset(TemporalSeq *seq, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporalseq_intersects_period(seq, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/* Integral of the temporal integer */

double
tintseq_integral(TemporalSeq *seq)
{
	double result = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		double duration = (double)(inst2->t) - (double)(inst1->t);
		result += (double)DatumGetInt32(temporalinst_value(inst1)) * duration;
		inst1 = inst2;
	}
	return result;
}

/* Integral of the temporal float */

double
tfloatseq_integral(TemporalSeq *seq)
{
	double result = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		double min = Min(DatumGetFloat8(temporalinst_value(inst1)), 
			DatumGetFloat8(temporalinst_value(inst2)));
		double max = Max(DatumGetFloat8(temporalinst_value(inst1)), 
			DatumGetFloat8(temporalinst_value(inst2)));
		double duration = (double)(inst2->t) - (double)(inst1->t);
		result += (max + min) * duration / 2.0;
		inst1 = inst2;
	}
	return result;
}

/* Time-weighted average of temporal integer */

double
tintseq_twavg(TemporalSeq *seq)
{
	double duration = temporalseq_duration_double(seq);
	double result;
	if (duration == 0)
		result = (double)DatumGetInt32(temporalinst_value(temporalseq_inst_n(seq, 0)));
	else
		result = tintseq_integral(seq) / duration;
	return result;
}

/* Time-weighted average of temporal float */

double
tfloatseq_twavg(TemporalSeq *seq)
{
	double duration = temporalseq_duration_double(seq);
	double result;
	if (duration == 0)
		/* The temporal sequence contains a single temporal instant */
		result = DatumGetFloat8(temporalinst_value(temporalseq_inst_n(seq, 0)));
	else
		result = tfloatseq_integral(seq) / duration;
	return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 * The functions assume that the arguments are of the same temptypid
 *****************************************************************************/

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency
 */
bool
temporalseq_eq(TemporalSeq *seq1, TemporalSeq *seq2)
{
	/* If number of sequences or the periods are not equal */
	if (seq1->count != seq2->count || 
			!period_eq_internal(&seq1->period, &seq2->period)) 
		return false;

	/* If bounding boxes are not equal */
	void *box1 = temporalseq_bbox_ptr(seq1);
	void *box2 = temporalseq_bbox_ptr(seq2);
	if (!temporal_bbox_eq(seq1->valuetypid, box1, box2))
		return false;
	
	/* We need to compare the composing instants */
	for (int i = 0; i < seq1->count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, i);
		if (!temporalinst_eq(inst1, inst2))
			return false;
	}
	return true;
}

/* 
 * Inequality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
temporalseq_ne(TemporalSeq *seq1, TemporalSeq *seq2)
{
	return !temporalseq_eq(seq1, seq2);
}

/*
 * B-tree comparator
 */

int
temporalseq_cmp(TemporalSeq *seq1, TemporalSeq *seq2)
{
	int count = Min(seq1->count, seq2->count);
	int result;
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, i);
		result = temporalinst_cmp(inst1, inst2);
		if (result) 
			return result;
	}
	/* The first count instants of both TemporalSeq values are equal */
	if (seq1->count < seq2->count) /* seq1 has less instants than seq2 */
		return -1;
	else if (seq2->count < seq1->count) /* seq2 has less instants than seq1 */
		return 1;
	else /* compare the time spans of seq1 and seq2 */
		return period_cmp_internal(&seq1->period, &seq2->period);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements and the approach for range types for combining the period 
 * bounds.
 *****************************************************************************/

uint32
temporalseq_hash(TemporalSeq *seq)
{
	uint32		result;
	char		flags = '\0';

	/* Create flags from the lower_inc and upper_inc values */
	if (seq->period.lower_inc)
		flags |= 0x01;
	if (seq->period.upper_inc)
		flags |= 0x02;
	result = hash_uint32((uint32) flags);
	
	/* Merge with hash of instants */
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		uint32 inst_hash = temporalinst_hash(inst);
		result = (result << 5) - result + inst_hash;
	}
	return result;
}
/*****************************************************************************/
