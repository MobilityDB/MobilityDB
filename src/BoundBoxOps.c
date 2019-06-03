/*****************************************************************************
 *
 * BoundingBoxOps.c
 *	  Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are 
 * - a period for temporal Booleans
 * - a TBOX for temporal integers and floats, where the x coordinate is for 
 *   the value dimension and the y coordinate is for the temporal dimension.
 * The following operators are defined:
 *	  overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both 
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"
#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
#endif

/*****************************************************************************
 * TBOX functions
 *****************************************************************************/

/* contains? */

bool
contains_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	/* The boxes should have at least one common dimension X or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) 
		if (box2->xmin < box1->xmin || box2->xmax > box1->xmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box2->tmin < box1->tmin || box2->tmax > box1->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(contains_tbox_tbox);

PGDLLEXPORT Datum
contains_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(contains_tbox_tbox_internal(box1, box2));
}

/* contained? */

bool
contained_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	return contains_tbox_tbox_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_tbox_tbox);

PGDLLEXPORT Datum
contained_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(contained_tbox_tbox_internal(box1, box2));
}

/* overlaps? */

bool
overlaps_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	/* The boxes should have at least one common dimension X or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) 
		if (box1->xmax < box2->xmin || box1->xmin > box2->xmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box1->tmax < box2->tmin || box1->tmin > box2->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(overlaps_tbox_tbox);

PGDLLEXPORT Datum
overlaps_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(overlaps_tbox_tbox_internal(box1, box2));
}

/* same? */

bool
same_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	/* The boxes should have at least one common dimension X or T  */
	assert((MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) ||
		(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)));
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags)) 
		if (box1->xmin != box2->xmin || box1->xmax != box2->xmax)
			return false;
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags)) 
		if (box1->tmin != box2->tmin || box1->tmax != box2->tmax)
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(same_tbox_tbox);

PGDLLEXPORT Datum
same_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(same_tbox_tbox_internal(box1, box2));
}

/* Size of bounding box */

size_t
temporal_bbox_size(Oid valuetypid) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return sizeof(Period);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return sizeof(TBOX);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY)) 
		return sizeof(STBOX);
#endif
	/* Types without bounding box, for example, tdoubleN */
	return 0;
}

/*****************************************************************************
 * Equality and comparison of bounding boxes of temporal types
 *****************************************************************************/

bool
temporal_bbox_eq(Oid valuetypid, void *box1, void *box2) 
{
	/* Only external types have bounding box */
	base_type_oid(valuetypid);
	bool result = false;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		result = period_eq_internal((Period *)box1, (Period *)box2);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		result = tbox_eq_internal((TBOX *)box1, (TBOX *)box2);
#ifdef WITH_POSTGIS
	else if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY))
		result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2) == 0;
#endif
	/* Types without bounding box, for example, doubleN */
	return result;
} 

int
temporal_bbox_cmp(Oid valuetypid, void *box1, void *box2) 
{
	/* Only external types have bounding box */
	base_type_oid(valuetypid);
	int result = 0;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		result = period_cmp_internal((Period *)box1, (Period *)box2);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		result = tbox_cmp_internal((TBOX *)box1, (TBOX *)box2);
#ifdef WITH_POSTGIS
	else if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY))
		result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2);
#endif
	/* Types without bounding box, for example, doubleN */
	return result;
} 

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2, 
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/* Make the bounding box a temporal instant from its values */

void
temporalinst_make_bbox(void *box, Datum value, TimestampTz t, Oid valuetypid) 
{
	/* Only external types have bounding box */
	base_type_oid(valuetypid);
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		period_set((Period *)box, t, t, true, true);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
	{
		double dvalue = datum_double(value, valuetypid);
		TBOX *result = (TBOX *)box;
		result->xmin = result->xmax = dvalue;
		result->tmin = result->tmax = (double)t;
		MOBDB_FLAGS_SET_X(result->flags, true);
		MOBDB_FLAGS_SET_T(result->flags, true);
	}
#ifdef WITH_POSTGIS
	else if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY)) 
		tpointinst_make_stbox((STBOX *)box, value, t);
#endif
	return;
}

/* Transform an array of temporal instant to a period */

static void
temporalinstarr_to_period(Period *period, TemporalInst **instants, int count, 
	bool lower_inc, bool upper_inc) 
{
	period_set(period, instants[0]->t, instants[count-1]->t, lower_inc, upper_inc);
	return;
}

/* Expand the first box with the second one */

static void
tbox_expand(TBOX *box1, const TBOX *box2)
{
	box1->xmin = Min(box1->xmin, box2->xmin);
	box1->xmax = Max(box1->xmax, box2->xmax);
	box1->tmin = Min(box1->tmin, box2->tmin);
	box1->tmax = Max(box1->tmax, box2->tmax);
}

/* Transform an array of tnumber instant to a box */

static void
tnumberinstarr_to_tbox(TBOX *box, TemporalInst **instants, int count) 
{
	Oid valuetypid = instants[0]->valuetypid;
	Datum value = temporalinst_value(instants[0]);
	temporalinst_make_bbox(box, value, instants[0]->t, valuetypid);
	for (int i = 1; i < count; i++)
	{
		TBOX box1 = {0};
		value = temporalinst_value(instants[i]);
		temporalinst_make_bbox(&box1, value, instants[i]->t, valuetypid);
		tbox_expand(box, &box1);
	}
	return;
}

/* Make the bounding box a temporal instant set from its values */
void 
temporali_make_bbox(void *box, TemporalInst **instants, int count) 
{
	/* Only external types have bounding box */
	base_type_oid(instants[0]->valuetypid);
	if (instants[0]->valuetypid == BOOLOID || 
		instants[0]->valuetypid == TEXTOID)
		temporalinstarr_to_period((Period *)box, instants, count, true, true);
	else if (instants[0]->valuetypid == INT4OID || 
		instants[0]->valuetypid == FLOAT8OID)
		tnumberinstarr_to_tbox((TBOX *)box, instants, count);
#ifdef WITH_POSTGIS
	else if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
		tpointinstarr_to_stbox((STBOX *)box, instants, count);
#endif
	return;
}

/* Make the bounding box a temporal sequence from its values */
void
temporalseq_make_bbox(void *box, TemporalInst **instants, int count, 
	bool lower_inc, bool upper_inc) 
{
	/* Only external types have bounding box */
	base_type_oid(instants[0]->valuetypid);
	Oid valuetypid = instants[0]->valuetypid;
	if (instants[0]->valuetypid == BOOLOID || 
		instants[0]->valuetypid == TEXTOID)
		temporalinstarr_to_period((Period *)box, instants, count, 
			lower_inc, upper_inc);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
		tnumberinstarr_to_tbox((TBOX *)box, instants, count);
#ifdef WITH_POSTGIS
	/* This code is currently not used since for temporal points the bounding
	 * box is computed from the trajectory for efficiency reasons. It is left
	 * here in case this is no longer the case */
	else if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
		tpointinstarr_to_stbox((STBOX *)box, instants, count);
#endif
	return;
}

/* Transform an array of temporal sequence to a period */

static void
temporalseqarr_to_period_internal(Period *period, TemporalSeq **sequences, int count) 
{
	Period *first = &sequences[0]->period;
	Period *last = &sequences[count-1]->period;
	period_set(period, first->lower, last->upper, first->lower_inc, last->upper_inc);
	return;
}

/* Transform an array of tnumber period to a box */

static void
tnumberseqarr_to_tbox_internal(TBOX *box, TemporalSeq **sequences, int count)
{
	memcpy(box, temporalseq_bbox_ptr(sequences[0]), sizeof(TBOX));
	for (int i = 1; i < count; i++)
	{
		TBOX *box1 = temporalseq_bbox_ptr(sequences[i]);
		tbox_expand(box, box1);
	}
	return;
}

/* Make the bounding box a temporal sequence from its values */
void
temporals_make_bbox(void *box, TemporalSeq **sequences, int count) 
{
	/* Only external types have bounding box */
	base_type_oid(sequences[0]->valuetypid);
	Oid valuetypid = sequences[0]->valuetypid;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID) 
		temporalseqarr_to_period_internal((Period *)box, sequences, count);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
		tnumberseqarr_to_tbox_internal((TBOX *)box, sequences, count);
#ifdef WITH_POSTGIS
	else if (sequences[0]->valuetypid == type_oid(T_GEOMETRY) || 
		sequences[0]->valuetypid == type_oid(T_GEOGRAPHY)) 
		tpointseqarr_to_stbox((STBOX *)box, sequences, count);
#endif
	return;
}

/*****************************************************************************
 * Expand the bounding box of a Temporal with a TemporalInst
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

static void
temporali_expand_period(Period *period, TemporalI *ti, TemporalInst *inst)
{
	TemporalInst *inst1 = temporali_inst_n(ti, 0);
	period_set(period, inst1->t, inst->t, true, true);
	return;
}

static void
temporalseq_expand_period(Period *period, TemporalSeq *seq, TemporalInst *inst)
{
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	period_set(period, inst1->t, inst->t, seq->period.lower_inc, true);
	return;
}

static void
temporals_expand_period(Period *period, TemporalS *ts, TemporalInst *inst)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	period_set(period, inst1->t, inst->t, seq->period.lower_inc, true);
	return;
}

static void
tnumber_expand_tbox(TBOX *box, Temporal *temp, TemporalInst *inst)
{
	temporal_bbox(box, temp);
	TBOX box1 = {0};
	temporalinst_bbox(&box1, inst);
	tbox_expand(box, &box1);
	return;
}

bool 
temporali_expand_bbox(void *box, TemporalI *ti, TemporalInst *inst)
{
	base_type_oid(ti->valuetypid);
	bool result = false;
	if (ti->valuetypid == BOOLOID || ti->valuetypid == TEXTOID)
	{
		temporali_expand_period((Period *)box, ti, inst);
		result = true;
	}
	else if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		tnumber_expand_tbox((TBOX *)box, (Temporal *)ti, inst);
		result = true;
	}
#ifdef WITH_POSTGIS
	else if (ti->valuetypid == type_oid(T_GEOGRAPHY) || 
		ti->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_stbox((STBOX *)box, (Temporal *)ti, inst);
		result = true;
	}
#endif
	return result;
}

bool 
temporalseq_expand_bbox(void *box, TemporalSeq *seq, TemporalInst *inst)
{
	base_type_oid(seq->valuetypid);
	bool result = false;
	if (seq->valuetypid == BOOLOID || seq->valuetypid == TEXTOID)
	{
		temporalseq_expand_period((Period *)box, seq, inst);
		result = true;
	}
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		tnumber_expand_tbox((TBOX *)box, (Temporal *)seq, inst);
		result = true;
	}
#ifdef WITH_POSTGIS
	if (seq->valuetypid == type_oid(T_GEOGRAPHY) || 
		seq->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_stbox((STBOX *)box, (Temporal *)seq, inst);
		result = true;
	}
#endif
	return result;
}

bool 
temporals_expand_bbox(void *box, TemporalS *ts, TemporalInst *inst)
{
	base_type_oid(ts->valuetypid);
	bool result = false;
	if (ts->valuetypid == BOOLOID || ts->valuetypid == TEXTOID)
	{
		temporals_expand_period((Period *)box, ts, inst);
		result = true;
	}
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		tnumber_expand_tbox((TBOX *)box, (Temporal *)ts, inst);
		result = true;
	}
#ifdef WITH_POSTGIS
	if (ts->valuetypid == type_oid(T_GEOGRAPHY) || 
		ts->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_stbox((STBOX *)box, (Temporal *)ts, inst);
		result = true;
	}
#endif
	return result;
}

/*****************************************************************************
 * Transform a <Type> to a TBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/* Transform a value to a box (internal function only) */

void
base_to_tbox(TBOX *box, Datum value, Oid valuetypid)
{
	number_base_type_oid(valuetypid);
	if (valuetypid == INT4OID)
		box->xmin = box->xmax = (double)(DatumGetInt32(value));
	else if (valuetypid == FLOAT8OID)
		box->xmin = box->xmax = DatumGetFloat8(value);
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

/* Transform an integer to a box */

void
int_to_tbox_internal(TBOX *box, int i)
{
	box->xmin = box->xmax = (double)i;
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

PG_FUNCTION_INFO_V1(int_to_tbox);

PGDLLEXPORT Datum
int_to_tbox(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	TBOX *result = palloc0(sizeof(TBOX));
	int_to_tbox_internal(result, i);
	PG_RETURN_POINTER(result);
}

/* Transform a float to a box */

void
float_to_tbox_internal(TBOX *box, double d)
{
	box->xmin = box->xmax = d;
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

PG_FUNCTION_INFO_V1(float_to_tbox);

PGDLLEXPORT Datum
float_to_tbox(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	TBOX *result = palloc0(sizeof(TBOX));
	float_to_tbox_internal(result, d);
	PG_RETURN_POINTER(result);
}

/* Transform a numeric to a box */

PG_FUNCTION_INFO_V1(numeric_to_tbox);

PGDLLEXPORT Datum
numeric_to_tbox(PG_FUNCTION_ARGS)
{
	Datum num = PG_GETARG_DATUM(0);
	double d = DatumGetFloat8(call_function1(numeric_float8, num));
	TBOX *result = palloc0(sizeof(TBOX));
	float_to_tbox_internal(result, d);
	PG_RETURN_POINTER(result);
}

/* Transform a range to a box */

void
range_to_tbox_internal(TBOX *box, RangeType *range)
{
	numrange_type_oid(range->rangetypid);
	if (range->rangetypid == type_oid(T_INTRANGE))
	{
		box->xmin = (double)(DatumGetInt32(lower_datum(range)));
		box->xmax = (double)(DatumGetInt32(upper_datum(range)));
	}
	else if (range->rangetypid == type_oid(T_FLOATRANGE))
	{
		box->xmin = DatumGetFloat8(lower_datum(range));
		box->xmax = DatumGetFloat8(upper_datum(range));
	}
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

/* Transform an integer range to a box */

void
intrange_to_tbox_internal(TBOX *box, RangeType *range)
{
	box->xmin = (double)(DatumGetInt32(lower_datum(range)));
	box->xmax = (double)(DatumGetInt32(upper_datum(range)));
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

PG_FUNCTION_INFO_V1(intrange_to_tbox);

PGDLLEXPORT Datum
intrange_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TBOX *result = palloc0(sizeof(TBOX));
	intrange_to_tbox_internal(result, range);
	PG_RETURN_POINTER(result);
}

/* Transform a float range to a box */

void
floatrange_to_tbox_internal(TBOX *box, RangeType *range)
{
	box->xmin = DatumGetFloat8(lower_datum(range));
	box->xmax = DatumGetFloat8(upper_datum(range));
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
	return;
}

PG_FUNCTION_INFO_V1(floatrange_to_tbox);

PGDLLEXPORT Datum
floatrange_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TBOX *result = palloc0(sizeof(TBOX));
	floatrange_to_tbox_internal(result, range);
	PG_RETURN_POINTER(result);
}

/* Transform a timestamptz to a box */

void
timestamp_to_tbox_internal(TBOX *box, TimestampTz t)
{
	box->tmin = box->tmax = (double)t;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestamp_to_tbox);

PGDLLEXPORT Datum
timestamp_to_tbox(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	TBOX *result = palloc0(sizeof(TBOX));
	timestamp_to_tbox_internal(result, t);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
timestampset_to_tbox_internal(TBOX *box, TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	box->tmin = (double)(p->lower);
	box->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(timestampset_to_tbox);

PGDLLEXPORT Datum
timestampset_to_tbox(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TBOX *result = palloc0(sizeof(TBOX));
	timestampset_to_tbox_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a period to a box */

void
period_to_tbox_internal(TBOX *box, Period *p)
{
	box->tmin = (double)(p->lower);
	box->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(period_to_tbox);

PGDLLEXPORT Datum
period_to_tbox(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	TBOX *result = palloc0(sizeof(TBOX));
	period_to_tbox_internal(result, p);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
periodset_to_tbox_internal(TBOX *box, PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	box->tmin = (double)(p->lower);
	box->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
	return;
}

PG_FUNCTION_INFO_V1(periodset_to_tbox);

PGDLLEXPORT Datum
periodset_to_tbox(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	TBOX *result = palloc0(sizeof(TBOX));
	periodset_to_tbox_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
 
/* Transform an integer value and a timestamptz to a box */

PG_FUNCTION_INFO_V1(int_timestamp_to_tbox);

PGDLLEXPORT Datum 
int_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = result->xmax = (double)i;
	result->tmin = result->tmax = (double)t;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_RETURN_POINTER(result);
}

/* Transform a float value and a timestamptz to a box */

PG_FUNCTION_INFO_V1(float_timestamp_to_tbox);

PGDLLEXPORT Datum 
float_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = result->xmax = d;
	result->tmin = result->tmax = (double)t;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_RETURN_POINTER(result);
}

/* Transform an integer value and a period to a box */

PG_FUNCTION_INFO_V1(int_period_to_tbox);

PGDLLEXPORT Datum 
int_period_to_tbox(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	Period *p = PG_GETARG_PERIOD(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = result->xmax = (double)i;
	result->tmin = (double)(p->lower);
	result->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_RETURN_POINTER(result);
}

/* Transform a float value and a period to a box */

PG_FUNCTION_INFO_V1(float_period_to_tbox);

PGDLLEXPORT Datum 
float_period_to_tbox(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	Period *p = PG_GETARG_PERIOD(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = result->xmax = d;
	result->tmin = (double)(p->lower);
	result->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_RETURN_POINTER(result);
}

/* Transform an integer range and a timestamptz to a box */

PG_FUNCTION_INFO_V1(intrange_timestamp_to_tbox);

PGDLLEXPORT Datum 
intrange_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = (double)(DatumGetInt32(lower_datum(range)));
	result->xmax = (double)(DatumGetInt32(upper_datum(range)));
	result->tmin = result->tmax = (double)t;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a float range and a timestamptz to a box */

PG_FUNCTION_INFO_V1(floatrange_timestamp_to_tbox);

PGDLLEXPORT Datum 
floatrange_timestamp_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = DatumGetFloat8(lower_datum(range));
	result->xmax = DatumGetFloat8(upper_datum(range));
	result->tmin = result->tmax = (double)t;
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform an integer range and a period to a box */

PG_FUNCTION_INFO_V1(intrange_period_to_tbox);

PGDLLEXPORT Datum 
intrange_period_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = (double)(DatumGetInt32(lower_datum(range)));
	result->xmax = (double)(DatumGetInt32(upper_datum(range)));
	result->tmin = (double)(p->lower);
	result->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a float range and a period to a box */

PG_FUNCTION_INFO_V1(floatrange_period_to_tbox);

PGDLLEXPORT Datum 
floatrange_period_to_tbox(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	TBOX *result = palloc0(sizeof(TBOX));
	result->xmin = DatumGetFloat8(lower_datum(range));
	result->xmax = DatumGetFloat8(upper_datum(range));
	result->tmin = (double)(p->lower);
	result->tmax = (double)(p->upper);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Bounding box operators for temporal types 
 * The inclusive/exclusive bounds are taken into account for the comparisons 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_period_temporal);

PGDLLEXPORT Datum
contains_bbox_period_temporal(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = contains_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_period);

PGDLLEXPORT Datum
contains_bbox_temporal_period(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = contains_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_temporal);

PGDLLEXPORT Datum
contains_bbox_temporal_temporal(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = contains_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_period_temporal);

PGDLLEXPORT Datum
contained_bbox_period_temporal(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = contains_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_period);

PGDLLEXPORT Datum
contained_bbox_temporal_period(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = contains_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_temporal);

PGDLLEXPORT Datum
contained_bbox_temporal_temporal(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = contains_period_period_internal(&p2, &p1);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_period_temporal);

PGDLLEXPORT Datum
overlaps_bbox_period_temporal(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overlaps_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_period);

PGDLLEXPORT Datum
overlaps_bbox_temporal_period(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overlaps_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_temporal);

PGDLLEXPORT Datum
overlaps_bbox_temporal_temporal(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = overlaps_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_period_temporal);

PGDLLEXPORT Datum
same_bbox_period_temporal(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = period_eq_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_period);

PGDLLEXPORT Datum
same_bbox_temporal_period(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = period_eq_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_temporal);

PGDLLEXPORT Datum
same_bbox_temporal_temporal(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = period_eq_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Bounding box operators for temporal number types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_range_tnumber);

PGDLLEXPORT Datum
contains_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = contains_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_range);

PGDLLEXPORT Datum
contains_bbox_tnumber_range(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = contains_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tbox_tnumber);

PGDLLEXPORT Datum
contains_bbox_tbox_tnumber(PG_FUNCTION_ARGS) 
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = contains_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_tbox);

PGDLLEXPORT Datum
contains_bbox_tnumber_tbox(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = contains_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contains_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
	
/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_range_tnumber);

PGDLLEXPORT Datum
contained_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = contained_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_range);

PGDLLEXPORT Datum
contained_bbox_tnumber_range(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = contained_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tbox_tnumber);

PGDLLEXPORT Datum
contained_bbox_tbox_tnumber(PG_FUNCTION_ARGS) 
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = contained_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_tbox);

PGDLLEXPORT Datum
contained_bbox_tnumber_tbox(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = contained_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contained_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
	
/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_range_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overlaps_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_range);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_range(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = overlaps_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tbox_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_tbox_tnumber(PG_FUNCTION_ARGS) 
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overlaps_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_tbox);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_tbox(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overlaps_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overlaps_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
	
/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_tnumber_range);

PGDLLEXPORT Datum
same_bbox_tnumber_range(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = same_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_range_tnumber);

PGDLLEXPORT Datum
same_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = same_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tbox_tnumber);

PGDLLEXPORT Datum
same_bbox_tbox_tnumber(PG_FUNCTION_ARGS) 
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = same_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_tbox);

PGDLLEXPORT Datum
same_bbox_tnumber_tbox(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = same_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = same_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
	
/*****************************************************************************/