/*****************************************************************************
 *
 * temporal_boxops.c
 *	  Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are 
 * - a period for temporal Booleans
 * - a TBOX for temporal integers and floats, where the x coordinate is for 
 *   the value dimension and the t coordinate is for the time dimension.
 * The following operators are defined:
 *	  overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both 
 * arguments: only the value dimension, only the time dimension, or both
 * the value and the time dimensions.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_boxops.h"

#include <assert.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "rangetypes_ext.h"
#include "tbox.h"
#include "tpoint.h"
#include "stbox.h"
#include "tpoint_boxops.h"

/*****************************************************************************
 * Functions on generic bounding boxes of temporal types
 *****************************************************************************/
/* Size of bounding box */

size_t
temporal_bbox_size(Oid valuetypid) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return sizeof(Period);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return sizeof(TBOX);
	if (valuetypid == type_oid(T_GEOGRAPHY) ||
		valuetypid == type_oid(T_GEOMETRY)) 
		return sizeof(STBOX);
	/* Types without bounding box, for example, tdoubleN */
	return 0;
}

/* Equality of bounding boxes */

bool
temporal_bbox_eq(const void *box1, const void *box2, Oid valuetypid)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(valuetypid);
	bool result = false;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		result = period_eq_internal((Period *)box1, (Period *)box2);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		result = tbox_eq_internal((TBOX *)box1, (TBOX *)box2);
	else if (valuetypid == type_oid(T_GEOGRAPHY) ||
		valuetypid == type_oid(T_GEOMETRY))
		result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2) == 0;
	/* Types without bounding box, for example, doubleN */
	return result;
} 

/* Comparison of bounding boxes */

int
temporal_bbox_cmp(const void *box1, const void *box2, Oid valuetypid)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(valuetypid);
	int result = 0;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		result = period_cmp_internal((Period *)box1, (Period *)box2);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		result = tbox_cmp_internal((TBOX *)box1, (TBOX *)box2);
	else if (valuetypid == type_oid(T_GEOGRAPHY) ||
		valuetypid == type_oid(T_GEOMETRY))
		result = stbox_cmp_internal((STBOX *)box1, (STBOX *)box2);
	/* Types without bounding box, for example, doubleN */
	return result;
}

/* Expand the first bounding box with the second one */

void
temporal_bbox_expand(void *box1, const void *box2, Oid valuetypid)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(valuetypid);
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		period_expand((Period *)box1, (Period *)box2);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		tbox_expand((TBOX *)box1, (TBOX *)box2);
	else if (valuetypid == type_oid(T_GEOGRAPHY) ||
		valuetypid == type_oid(T_GEOMETRY))
		stbox_expand((STBOX *)box1, (STBOX *)box2);
}

/* Shift the bounding box with an interval */

void
temporal_bbox_shift(void *box, const Interval *interval, Oid valuetypid)
{
	ensure_temporal_base_type(valuetypid);
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		period_shift_internal((Period *)box, interval);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		tbox_shift((TBOX *)box, interval);
	else if (valuetypid == type_oid(T_GEOGRAPHY) ||
		valuetypid == type_oid(T_GEOMETRY))
		stbox_shift((STBOX *)box, interval);
	return;
}

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2, 
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/* Make the bounding box a temporal instant from its values */

void
temporalinst_make_bbox(void *box, const TemporalInst *inst)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(inst->valuetypid);
	if (inst->valuetypid == BOOLOID || inst->valuetypid == TEXTOID)
		period_set((Period *)box, inst->t, inst->t, true, true);
	else if (inst->valuetypid == INT4OID || inst->valuetypid == FLOAT8OID)
	{
		double dvalue = datum_double(temporalinst_value(inst), inst->valuetypid);
		TBOX *result = (TBOX *)box;
		result->xmin = result->xmax = dvalue;
		result->tmin = result->tmax = inst->t;
		MOBDB_FLAGS_SET_X(result->flags, true);
		MOBDB_FLAGS_SET_T(result->flags, true);
	}
	else if (inst->valuetypid == type_oid(T_GEOGRAPHY) ||
		inst->valuetypid == type_oid(T_GEOMETRY))
		tpointinst_make_stbox((STBOX *)box, inst);
}

/* Transform an array of temporal instant to a period */

static void
temporalinstarr_to_period(Period *period, TemporalInst **instants, int count,
	bool lower_inc, bool upper_inc) 
{
	period_set(period, instants[0]->t, instants[count - 1]->t, lower_inc, upper_inc);
}


/* Transform an array of tnumber instant to a box */

static void
tnumberinstarr_to_tbox(TBOX *box, TemporalInst **instants, int count)
{
	temporalinst_make_bbox(box, instants[0]);
	for (int i = 1; i < count; i++)
	{
		TBOX box1;
		memset(&box1, 0, sizeof(TBOX));
		temporalinst_make_bbox(&box1, instants[i]);
		tbox_expand(box, &box1);
	}
}

/* Make the bounding box a temporal instant set from its values */
void 
temporali_make_bbox(void *box, TemporalInst **instants, int count)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(instants[0]->valuetypid);
	if (instants[0]->valuetypid == BOOLOID || 
		instants[0]->valuetypid == TEXTOID)
		temporalinstarr_to_period((Period *)box, instants, count, true, true);
	else if (instants[0]->valuetypid == INT4OID || 
		instants[0]->valuetypid == FLOAT8OID)
		tnumberinstarr_to_tbox((TBOX *)box, instants, count);
	else if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) ||
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
		tpointinstarr_to_stbox((STBOX *)box, instants, count);
}

/* Make the bounding box a temporal sequence from its values */
void
temporalseq_make_bbox(void *box, TemporalInst **instants, int count,
	bool lower_inc, bool upper_inc)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(instants[0]->valuetypid);
	Oid valuetypid = instants[0]->valuetypid;
	if (instants[0]->valuetypid == BOOLOID || 
		instants[0]->valuetypid == TEXTOID)
		temporalinstarr_to_period((Period *)box, instants, count, 
			lower_inc, upper_inc);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
		tnumberinstarr_to_tbox((TBOX *)box, instants, count);
	/* This code is currently not used since for temporal points the bounding
	 * box is computed from the trajectory for efficiency reasons. It is left
	 * here in case this is no longer the case
	else if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
		tpointinstarr_to_stbox((STBOX *)box, instants, count);
	 */
}

/* Transform an array of temporal sequence to a period */

static void
temporalseqarr_to_period_internal(Period *period, TemporalSeq **sequences, int count)
{
	Period *first = &sequences[0]->period;
	Period *last = &sequences[count - 1]->period;
	period_set(period, first->lower, last->upper, first->lower_inc, last->upper_inc);
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
}

/* Make the bounding box a temporal sequence from its values */
void
temporals_make_bbox(void *box, TemporalSeq **sequences, int count)
{
	/* Only external types have bounding box */
	ensure_temporal_base_type(sequences[0]->valuetypid);
	Oid valuetypid = sequences[0]->valuetypid;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID) 
		temporalseqarr_to_period_internal((Period *)box, sequences, count);
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
		tnumberseqarr_to_tbox_internal((TBOX *)box, sequences, count);
	else if (sequences[0]->valuetypid == type_oid(T_GEOMETRY) ||
		sequences[0]->valuetypid == type_oid(T_GEOGRAPHY)) 
		tpointseqarr_to_stbox((STBOX *)box, sequences, count);
}

/*****************************************************************************
 * Transform a type to a TBOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/* Transform a value to a box (internal function only) */

void
number_to_box(TBOX *box, Datum value, Oid valuetypid)
{
	ensure_numeric_base_type(valuetypid);
	if (valuetypid == INT4OID)
		box->xmin = box->xmax = (double)(DatumGetInt32(value));
	else if (valuetypid == FLOAT8OID)
		box->xmin = box->xmax = DatumGetFloat8(value);
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
}

/* Transform an integer to a box */

void
int_to_tbox_internal(TBOX *box, int i)
{
	box->xmin = box->xmax = (double)i;
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
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
	ensure_numrange_type(range->rangetypid);
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
}

/* Transform an integer range to a box */

void
intrange_to_tbox_internal(TBOX *box, RangeType *range)
{
	box->xmin = (double)(DatumGetInt32(lower_datum(range)));
	box->xmax = (double)(DatumGetInt32(upper_datum(range)));
	MOBDB_FLAGS_SET_X(box->flags, true);
	MOBDB_FLAGS_SET_T(box->flags, false);
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
	box->tmin = box->tmax = t;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
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
timestampset_to_tbox_internal(TBOX *box, const TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
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
period_to_tbox_internal(TBOX *box, const Period *p)
{
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
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
periodset_to_tbox_internal(TBOX *box, const PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	box->tmin = p->lower;
	box->tmax = p->upper;
	MOBDB_FLAGS_SET_X(box->flags, false);
	MOBDB_FLAGS_SET_T(box->flags, true);
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
	result->tmin = result->tmax = t;
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
	result->tmin = result->tmax = t;
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
	result->tmin = p->lower;
	result->tmax = p->upper;
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
	result->tmin = p->lower;
	result->tmax = p->upper;
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
	result->tmin = result->tmax = t;
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
	result->tmin = result->tmax = t;
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
	result->tmin = p->lower;
	result->tmax = p->upper;
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
	result->tmin = p->lower;
	result->tmax = p->upper;
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp);
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
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	bool result = period_eq_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_period_temporal);

PGDLLEXPORT Datum
adjacent_bbox_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_period(&p1, temp);
	bool result = adjacent_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_temporal_period);

PGDLLEXPORT Datum
adjacent_bbox_temporal_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_period(&p1, temp);
	bool result = adjacent_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_temporal_temporal);

PGDLLEXPORT Datum
adjacent_bbox_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	bool result = adjacent_period_period_internal(&p1, &p2);
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
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
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = same_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_range);

PGDLLEXPORT Datum
adjacent_bbox_tnumber_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = adjacent_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_range_tnumber);

PGDLLEXPORT Datum
adjacent_bbox_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = adjacent_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tbox_tnumber);

PGDLLEXPORT Datum
adjacent_bbox_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	bool result = adjacent_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_tbox);

PGDLLEXPORT Datum
adjacent_bbox_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	bool result = adjacent_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(adjacent_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
adjacent_bbox_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = adjacent_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
/*****************************************************************************/