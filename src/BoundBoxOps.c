/*****************************************************************************
 *
 * BoundingBoxOps.c
 *	  Bounding box operators for temporal types.
 *
 * The bounding box of temporal values are 
 * - a period for temporal Booleans
 * - a BOX for temporal integers and floats, where the x coordinate is for 
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
 * BOX functions
 *****************************************************************************/

/* contains? */

bool
contains_box_box_internal(const BOX *box1, const BOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->high.x != infinity && box2->high.x != infinity ) 
		if ( box2->low.x < box1->low.x || box2->high.x > box1->high.x )
			return false;
	if ( box1->high.y != infinity && box2->high.y != infinity ) 
		if ( box2->low.y < box1->low.y || box2->high.y > box1->high.y )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(contains_box_box);

PGDLLEXPORT Datum
contains_box_box(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(contains_box_box_internal(box1, box2));
}

/* contained? */

bool
contained_box_box_internal(const BOX *box1, const BOX *box2)
{
	return contains_box_box_internal(box2, box1);
}

PG_FUNCTION_INFO_V1(contained_box_box);

PGDLLEXPORT Datum
contained_box_box(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(contained_box_box_internal(box1, box2));
}

/* overlaps? */

bool
overlaps_box_box_internal(const BOX *box1, const BOX *box2)
{
	double infinity = get_float8_infinity();
	if ( box1->high.x != infinity && box2->high.x != infinity ) 
		if ( box1->high.x < box2->low.x || box1->low.x > box2->high.x )
			return false;
	if ( box1->high.y != infinity && box2->high.y != infinity ) 
		if ( box1->high.y < box2->low.y || box1->low.y > box2->high.y )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(overlaps_box_box);

PGDLLEXPORT Datum
overlaps_box_box(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(overlaps_box_box_internal(box1, box2));
}

/* same? */

bool
same_box_box_internal(const BOX *box1, const BOX *box2)
{
	double infinity = get_float8_infinity();
 	if ( box1->high.x != infinity && box2->high.x != infinity ) 
		if ( box1->low.x != box2->low.x || box1->high.x != box2->high.x )
			return false;
	if ( box1->high.y != infinity && box2->high.y != infinity ) 
		if ( box1->low.y != box2->low.y || box1->high.y != box2->high.y )
			return false;
	return true;
}

PG_FUNCTION_INFO_V1(same_box_box);

PGDLLEXPORT Datum
same_box_box(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(same_box_box_internal(box1, box2));
}

/*****************************************************************************/

/* Comparison operators */

static int
box_cmp_internal(const BOX *box1, const BOX *box2)
{
	/* Compare the box minima */
	if (box1->low.x < box2->low.x)
		return -1;
	if (box1->low.x > box2->low.x)
		return 1;
	if (box1->low.y < box2->low.y)
		return -1;
	if (box1->low.y > box2->low.y)
		return 1;
	/* Compare the box maxima */
	if (box1->high.x < box2->high.x)
		return -1;
	if (box1->high.x > box2->high.x)
		return 1;
	if (box1->high.y < box2->high.y)
		return -1;
	if (box1->high.y > box2->high.y)
		return 1;
	/* The two boxes are equal */
	return 0;
}

PG_FUNCTION_INFO_V1(box_cmp);

PGDLLEXPORT Datum
box_cmp(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	int	cmp = box_cmp_internal(box1, box2);
	PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(box_lt);

PGDLLEXPORT Datum
box_lt(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	int	cmp = box_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(box_le);

PGDLLEXPORT Datum
box_le(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	int	cmp = box_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(box_ge);

PGDLLEXPORT Datum
box_ge(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	int	cmp = box_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(box_gt);

PGDLLEXPORT Datum
box_gt(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	int	cmp = box_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp > 0);
}

static bool
box_eq_internal(const BOX *box1, const BOX *box2)
{
	if (box1->low.x != box2->low.x ||
		box1->low.y != box2->low.y ||
		box1->high.x != box2->high.x ||
		box1->high.y != box2->high.y)
		return false;
	/* The two boxes are equal */
	return true;
}

PG_FUNCTION_INFO_V1(box_eq);

PGDLLEXPORT Datum
box_eq(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(box_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(box_ne);

PGDLLEXPORT Datum
box_ne(PG_FUNCTION_ARGS)
{
	BOX *box1 = PG_GETARG_BOX_P(0);
	BOX *box2 = PG_GETARG_BOX_P(1);
	PG_RETURN_BOOL(! box_eq_internal(box1, box2));
}

/* Size of bounding box */

size_t
temporal_bbox_size(Oid valuetypid) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return sizeof(Period);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return sizeof(BOX);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY)) 
		return sizeof(GBOX);
#endif
	/* Types without bounding box, for example, tdoubleN */
	return 0;
}

/*****************************************************************************
 * Comparison of bounding boxes of temporal types
 *****************************************************************************/

bool
temporal_bbox_eq(Oid valuetypid, void *box1, void *box2) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return period_eq_internal((Period *)box1, (Period *)box2);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return box_eq_internal((BOX *)box1, (BOX *)box2);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY))
		return gbox_cmp_internal((GBOX *)box1, (GBOX *)box2) == 0;
#endif
	/* Types without bounding box, for example, doubleN */
	return false;
} 

int
temporal_bbox_cmp(Oid valuetypid, void *box1, void *box2) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return period_cmp_internal((Period *)box1, (Period *)box2);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return box_cmp_internal((BOX *)box1, (BOX *)box2);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY))
		return gbox_cmp_internal((GBOX *)box1, (GBOX *)box2);
#endif
	/* Types without bounding box, for example, doubleN */
	return 0;
} 

/*****************************************************************************
 * Compute the bounding box at the creation of temporal values
 * Only external types have precomputed bbox, internal types such as double2, 
 * double3, or double4 do not have precomputed bounding box.
 *****************************************************************************/

/* Make the bounding box a temporal instant from its values */

bool
temporalinst_make_bbox(void *box, Datum value, TimestampTz t, Oid valuetypid) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
	{
		period_set((Period *)box, t, t, true, true);
		return true;
	}
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
	{
		double dvalue = datum_double(value, valuetypid);
		BOX *result = (BOX *)box;
		result->low.x = result->high.x = dvalue;
		result->low.y = result->high.y = (double)t;
		return true;
	}
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpointinst_make_gbox((GBOX *)box, value, t);
		return true;
	}
#endif
	/* Types without bounding box, for example, tdoubleN* */
	return false;
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
box_expand_internal(BOX *box1, const BOX *box2)
{
	box1->low.x = Min(box1->low.x, box2->low.x);
	box1->high.x = Max(box1->high.x, box2->high.x);
	box1->low.y = Min(box1->low.y, box2->low.y);
	box1->high.y = Max(box1->high.y, box2->high.y);
}

/* Transform an array of tnumber instant to a box */

static void
tnumberinstarr_to_box(BOX *box, TemporalInst **instants, int count) 
{
	Oid valuetypid = instants[0]->valuetypid;
	Datum value = temporalinst_value(instants[0]);
	temporalinst_make_bbox(box, value, instants[0]->t, valuetypid);
	for (int i = 1; i < count; i++)
	{
		BOX box1;
		value = temporalinst_value(instants[i]);
		temporalinst_make_bbox(&box1, value, instants[i]->t, valuetypid);
		box_expand_internal(box, &box1);
	}
	return;
}

/* Make the bounding box a temporal instant set from its values */
bool 
temporali_make_bbox(void *box, TemporalInst **instants, int count) 
{
	if (instants[0]->valuetypid == BOOLOID || 
		instants[0]->valuetypid == TEXTOID)
	{
		temporalinstarr_to_period((Period *)box, instants, count, true, true);
		return true;
	}
	if (instants[0]->valuetypid == INT4OID || 
		instants[0]->valuetypid == FLOAT8OID)
	{
		tnumberinstarr_to_box((BOX *)box, instants, count);
		return true;
	}
#ifdef WITH_POSTGIS
	if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpointinstarr_to_gbox((GBOX *)box, instants, count);
		return true;
	}
#endif
	return false;
}

/* Make the bounding box a temporal sequence from its values */
bool
temporalseq_make_bbox(void *box, TemporalInst **instants, int count, 
	bool lower_inc, bool upper_inc) 
{
	Oid valuetypid = instants[0]->valuetypid;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID) 
	{
		temporalinstarr_to_period((Period *)box, instants, count, 
			lower_inc, upper_inc);
		return true;
	}
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
	{
		tnumberinstarr_to_box((BOX *)box, instants, count);
		return true;
	}
#ifdef WITH_POSTGIS
	/* This code is currently not used since for temporal points the bounding
	 * box is computed from the trajectory for efficiency reasons. It is left
	 * here in case this is no longer the case */
	if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpointinstarr_to_gbox((GBOX *)box, instants, count);
		return true;
	}
#endif
	return false;
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
tnumberseqarr_to_box_internal(BOX *box, TemporalSeq **sequences, int count)
{
	memcpy(box, temporalseq_bbox_ptr(sequences[0]), sizeof(BOX));
	for (int i = 1; i < count; i++)
	{
		BOX *box1 = temporalseq_bbox_ptr(sequences[i]);
		box_expand_internal(box, box1);
	}
	return;
}

/* Make the bounding box a temporal sequence from its values */
bool
temporals_make_bbox(void *box, TemporalSeq **sequences, int count) 
{
	Oid valuetypid = sequences[0]->valuetypid;
	if (valuetypid == BOOLOID || valuetypid == TEXTOID) 
	{
		temporalseqarr_to_period_internal((Period *)box, sequences, count);
		return true;
	}
	else if (valuetypid == INT4OID || valuetypid == FLOAT8OID) 
	{
		tnumberseqarr_to_box_internal((BOX *)box, sequences, count);
		return true;
	}
#ifdef WITH_POSTGIS
	if (sequences[0]->valuetypid == type_oid(T_GEOMETRY) || 
		sequences[0]->valuetypid == type_oid(T_GEOGRAPHY)) 
	{
		tpointseqarr_to_gbox((GBOX *)box, sequences, count);
		return true;
	}
#endif
	return false;
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
tnumber_expand_box(BOX *box, Temporal *temp, TemporalInst *inst)
{
	temporal_bbox(box, temp);
	BOX box1;
	temporalinst_bbox(&box1, inst);
	box_expand_internal(box, &box1);
	return;
}

bool 
temporali_expand_bbox(void *box, TemporalI *ti, TemporalInst *inst)
{
	if (ti->valuetypid == BOOLOID || ti->valuetypid == TEXTOID)
	{
		temporali_expand_period((Period *)box, ti, inst);
		return true;
	}
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		tnumber_expand_box((BOX *)box, (Temporal *)ti, inst);
		return true;
	}
#ifdef WITH_POSTGIS
	if (ti->valuetypid == type_oid(T_GEOGRAPHY) || 
		ti->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_gbox((GBOX *)box, (Temporal *)ti, inst);
		return true;
	}
#endif
	return false;
}

bool 
temporalseq_expand_bbox(void *box, TemporalSeq *seq, TemporalInst *inst)
{
	if (seq->valuetypid == BOOLOID || seq->valuetypid == TEXTOID)
	{
		temporalseq_expand_period((Period *)box, seq, inst);
		return true;
	}
	if (seq->valuetypid == INT4OID || seq->valuetypid == FLOAT8OID)
	{
		tnumber_expand_box((BOX *)box, (Temporal *)seq, inst);
		return true;
	}
#ifdef WITH_POSTGIS
	if (seq->valuetypid == type_oid(T_GEOGRAPHY) || 
		seq->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_gbox((GBOX *)box, (Temporal *)seq, inst);
		return true;
	}
#endif
	return false;
}

bool 
temporals_expand_bbox(void *box, TemporalS *ts, TemporalInst *inst)
{
	if (ts->valuetypid == BOOLOID || ts->valuetypid == TEXTOID)
	{
		temporals_expand_period((Period *)box, ts, inst);
		return true;
	}
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		tnumber_expand_box((BOX *)box, (Temporal *)ts, inst);
		return true;
	}
#ifdef WITH_POSTGIS
	if (ts->valuetypid == type_oid(T_GEOGRAPHY) || 
		ts->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpoint_expand_gbox((GBOX *)box, (Temporal *)ts, inst);
		return true;
	}
#endif
	return false;
}

/*****************************************************************************
 * Transform one or two types to a BOX
 * The functions assume that the argument box is set to 0 before with palloc0
 *****************************************************************************/

/* Transform a value to a box (internal function only) */

void
base_to_box(BOX *box, Datum value, Oid valuetypid)
{
	number_base_type_oid(valuetypid);
	if (valuetypid == INT4OID)
		box->low.x = box->high.x = (double)(DatumGetInt32(value));
	else if (valuetypid == FLOAT8OID)
		box->low.x = box->high.x = DatumGetFloat8(value);
	double infinity = get_float8_infinity();
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

/* Transform an integer to a box */

void
int_to_box_internal(BOX *box, int i)
{
	double infinity = get_float8_infinity();
	box->low.x = box->high.x = (double)i;
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

PG_FUNCTION_INFO_V1(int_to_box);

PGDLLEXPORT Datum
int_to_box(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	BOX *result = palloc0(sizeof(BOX));
	int_to_box_internal(result, i);
	PG_RETURN_POINTER(result);
}

/* Transform a float to a box */

void
float_to_box_internal(BOX *box, double d)
{
	double infinity = get_float8_infinity();
	box->low.x = box->high.x = d;
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

PG_FUNCTION_INFO_V1(float_to_box);

PGDLLEXPORT Datum
float_to_box(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	BOX *result = palloc0(sizeof(BOX));
	float_to_box_internal(result, d);
	PG_RETURN_POINTER(result);
}

/* Transform a numeric to a box */

PG_FUNCTION_INFO_V1(numeric_to_box);

PGDLLEXPORT Datum
numeric_to_box(PG_FUNCTION_ARGS)
{
	Datum num = PG_GETARG_DATUM(0);
	double d = DatumGetFloat8(call_function1(numeric_float8, num));
	BOX *result = palloc0(sizeof(BOX));
	float_to_box_internal(result, d);
	PG_RETURN_POINTER(result);
}

/* Transform a range to a box */

void
range_to_box(BOX *box, RangeType *range)
{
	numrange_type_oid(range->rangetypid);
	if (range->rangetypid == type_oid(T_INTRANGE))
	{
		box->low.x = (double)(DatumGetInt32(lower_datum(range)));
		box->high.x = (double)(DatumGetInt32(upper_datum(range)));
	}
	else if (range->rangetypid == type_oid(T_FLOATRANGE))
	{
		box->low.x = DatumGetFloat8(lower_datum(range));
		box->high.x = DatumGetFloat8(upper_datum(range));
	}
	double infinity = get_float8_infinity();
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

/* Transform an integer range to a box */

void
intrange_to_box_internal(BOX *box, RangeType *range)
{
	double infinity = get_float8_infinity();
	box->low.x = (double)(DatumGetInt32(lower_datum(range)));
	box->high.x = (double)(DatumGetInt32(upper_datum(range)));
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

PG_FUNCTION_INFO_V1(intrange_to_box);

PGDLLEXPORT Datum
intrange_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	BOX *result = palloc0(sizeof(BOX));
	intrange_to_box_internal(result, range);
	PG_RETURN_POINTER(result);
}

/* Transform a float range to a box */

void
floatrange_to_box_internal(BOX *box, RangeType *range)
{
	double infinity = get_float8_infinity();
	box->low.x = DatumGetFloat8(lower_datum(range));
	box->high.x = DatumGetFloat8(upper_datum(range));
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

PG_FUNCTION_INFO_V1(floatrange_to_box);

PGDLLEXPORT Datum
floatrange_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	BOX *result = palloc0(sizeof(BOX));
	floatrange_to_box_internal(result, range);
	PG_RETURN_POINTER(result);
}

/* Transform a timestamptz to a box */

void
timestamp_to_box_internal(BOX *box, TimestampTz t)
{
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = box->high.y = (double)t;
	return;
}

PG_FUNCTION_INFO_V1(timestamp_to_box);

PGDLLEXPORT Datum
timestamp_to_box(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	BOX *result = palloc0(sizeof(BOX));
	timestamp_to_box_internal(result, t);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
timestampset_to_box_internal(BOX *box, TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

PG_FUNCTION_INFO_V1(timestampset_to_box);

PGDLLEXPORT Datum
timestampset_to_box(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	BOX *result = palloc0(sizeof(BOX));
	timestampset_to_box_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a period to a box */

void
period_to_box_internal(BOX *box, Period *p)
{
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

PG_FUNCTION_INFO_V1(period_to_box);

PGDLLEXPORT Datum
period_to_box(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	BOX *result = palloc0(sizeof(BOX));
	period_to_box_internal(result, p);
	PG_RETURN_POINTER(result);
}

/* Transform a period set to a box */

void
periodset_to_box_internal(BOX *box, PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

PG_FUNCTION_INFO_V1(periodset_to_box);

PGDLLEXPORT Datum
periodset_to_box(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	BOX *result = palloc0(sizeof(BOX));
	periodset_to_box_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
 
/* Transform an integer value and a timestamptz to a box */

PG_FUNCTION_INFO_V1(int_timestamp_to_box);

PGDLLEXPORT Datum 
int_timestamp_to_box(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = result->high.x = (double)i;
	result->low.y = result->high.y = (double)t;
	PG_RETURN_POINTER(result);
}

/* Transform a float value and a timestamptz to a box */

PG_FUNCTION_INFO_V1(float_timestamp_to_box);

PGDLLEXPORT Datum 
float_timestamp_to_box(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = result->high.x = d;
	result->low.y = result->high.y = (double)t;
	PG_RETURN_POINTER(result);
}

/* Transform an integer value and a period to a box */

PG_FUNCTION_INFO_V1(int_period_to_box);

PGDLLEXPORT Datum 
int_period_to_box(PG_FUNCTION_ARGS)
{
	int i = PG_GETARG_INT32(0);
	Period *p = PG_GETARG_PERIOD(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = result->high.x = (double)i;
	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
	PG_RETURN_POINTER(result);
}

/* Transform a float value and a period to a box */

PG_FUNCTION_INFO_V1(float_period_to_box);

PGDLLEXPORT Datum 
float_period_to_box(PG_FUNCTION_ARGS)
{
	double d = PG_GETARG_FLOAT8(0);
	Period *p = PG_GETARG_PERIOD(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = result->high.x = d;
	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
	PG_RETURN_POINTER(result);
}

/* Transform an integer range and a timestamptz to a box */

PG_FUNCTION_INFO_V1(intrange_timestamp_to_box);

PGDLLEXPORT Datum 
intrange_timestamp_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = (double)(DatumGetInt32(lower_datum(range)));
	result->high.x = (double)(DatumGetInt32(upper_datum(range)));
	result->low.y = result->high.y = (double)t;
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a float range and a timestamptz to a box */

PG_FUNCTION_INFO_V1(floatrange_timestamp_to_box);

PGDLLEXPORT Datum 
floatrange_timestamp_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = DatumGetFloat8(lower_datum(range));
	result->high.x = DatumGetFloat8(upper_datum(range));
	result->low.y = result->high.y = (double)t;
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform an integer range and a period to a box */

PG_FUNCTION_INFO_V1(intrange_period_to_box);

PGDLLEXPORT Datum 
intrange_period_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = (double)(DatumGetInt32(lower_datum(range)));
	result->high.x = (double)(DatumGetInt32(upper_datum(range)));
	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/* Transform a float range and a period to a box */

PG_FUNCTION_INFO_V1(floatrange_period_to_box);

PGDLLEXPORT Datum 
floatrange_period_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	BOX *result = palloc(sizeof(BOX));
	result->low.x = DatumGetFloat8(lower_datum(range));
	result->high.x = DatumGetFloat8(upper_datum(range));
	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
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
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = contains_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = contains_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_box_tnumber);

PGDLLEXPORT Datum
contains_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contains_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_box);

PGDLLEXPORT Datum
contains_bbox_tnumber_box(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contains_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contains_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = contained_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = contained_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_box_tnumber);

PGDLLEXPORT Datum
contained_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contained_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_box);

PGDLLEXPORT Datum
contained_bbox_tnumber_box(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contained_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = contained_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overlaps_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = overlaps_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_box_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overlaps_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_box);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_box(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overlaps_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overlaps_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = same_box_box_internal(&box1, &box2);
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
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = same_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_box_tnumber);

PGDLLEXPORT Datum
same_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = same_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_box);

PGDLLEXPORT Datum
same_bbox_tnumber_box(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = same_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_tnumber_tnumber);

PGDLLEXPORT Datum
same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = same_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}
	
/*****************************************************************************/
