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
#include "TemporalNPoint.h"
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

/*
 * Expand the first box with the second one
 */
static void
box_expand_internal(BOX *box1, const BOX *box2)
{
	box1->low.x = Min(box1->low.x, box2->low.x);
	box1->high.x = Max(box1->high.x, box2->high.x);
	box1->low.y = Min(box1->low.y, box2->low.y);
	box1->high.y = Max(box1->high.y, box2->high.y);
}

/*
 * Compare two boxes
 */
int
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

/*****************************************************************************
 * Bounding box common functions
 *****************************************************************************/

size_t
temporal_bbox_size(Oid valuetypid) 
{
	if (valuetypid == BOOLOID || valuetypid == TEXTOID)
		return sizeof(Period);
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
		return sizeof(BOX);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOGRAPHY) || 
		valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_NPOINT)) 
		return sizeof(GBOX);
#endif
	/* Types without bounding box, for example, tdoubleN */
	return 0;
} 

/*****************************************************************************
 * Functions that compute the bounding box at the creation of temporal values
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
	else if (valuetypid == type_oid(T_NPOINT))
	{
		tnpointinst_make_gbox((GBOX *)box, value, t);
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
	if (instants[0]->valuetypid == type_oid(T_NPOINT))
	{
		tnpointinstarr_disc_to_gbox((GBOX *)box, instants, count);
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
	if (instants[0]->valuetypid == type_oid(T_GEOGRAPHY) || 
		instants[0]->valuetypid == type_oid(T_GEOMETRY)) 
	{
		tpointinstarr_to_gbox((GBOX *)box, instants, count);
		return true;
	}
	if (instants[0]->valuetypid == type_oid(T_NPOINT))
	{
		tnpointinstarr_cont_to_gbox((GBOX *)box, instants, count);
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
	if (sequences[0]->valuetypid == type_oid(T_NPOINT))
	{
		tnpointseqarr_to_gbox((GBOX *)box, sequences, count);
		return true;
	}
#endif
	return false;
}

/*****************************************************************************
 * Transform a box to a <Type> 
 *****************************************************************************/

Period *
box_to_period_internal(BOX *box)
{
	TimestampTz lower = box->low.y;
	TimestampTz upper = box->high.y;
	return period_make(lower, upper, true, true);
}

/*****************************************************************************
 * Transform a <Type> to a BOX
 *****************************************************************************/

/* Transform a value to a box (internal function only) */

void
base_to_box(BOX *box, Datum value, Oid valuetypid)
{
	if (valuetypid == INT4OID)
		box->low.x = box->high.x = (double)(DatumGetInt32(value));
	else if (valuetypid == FLOAT8OID)
		box->low.x = box->high.x = DatumGetFloat8(value);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	double infinity = get_float8_infinity();
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

/* Transform a range to a box (internal function only) */

void
range_to_box(BOX *box, RangeType *range)
{
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
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	double infinity = get_float8_infinity();
	box->low.y = -infinity;
	box->high.y = +infinity;
	return;
}

/* Transform a timestamptz to a box (internal function only) */

void
timestamp_to_box(BOX *box, TimestampTz t)
{
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = box->high.y = (double)t;
	return;
}

/* Transform a period set to a box (internal function only) */

void
timestampset_to_box(BOX *box, TimestampSet *ts)
{
	Period *p = timestampset_bbox(ts);
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

/* Transform a period to a box (internal function only) */

void
period_to_box(BOX *box, Period *p)
{
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

/* Transform a period set to a box (internal function only) */

void
periodset_to_box(BOX *box, PeriodSet *ps)
{
	Period *p = periodset_bbox(ps);
	double infinity = get_float8_infinity();
	box->low.x = -infinity;
	box->high.x = +infinity;
	box->low.y = (double)(p->lower);
	box->high.y = (double)(p->upper);
	return;
}

/*****************************************************************************/
 
/* Transform a value and a timestamptz to a box */

BOX *
base_timestamp_to_box_internal(Datum value, TimestampTz t, Oid valuetypid)
{
	BOX *result = palloc(sizeof(BOX));
	if (valuetypid == INT4OID)
		result->low.x = result->high.x = (double)(DatumGetInt32(value));
	else if (valuetypid == FLOAT8OID)
		result->low.x = result->high.x = DatumGetFloat8(value);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	result->low.y = result->high.y = (double)t;
	return result;
}

PG_FUNCTION_INFO_V1(base_timestamp_to_box);

PGDLLEXPORT Datum 
base_timestamp_to_box(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX *result = base_timestamp_to_box_internal(value, t, valuetypid);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Transform a value and a period to a box */

BOX *
base_period_to_box_internal(Datum value, Period *p, Oid valuetypid)
{
	BOX *result = palloc(sizeof(BOX));
	if (valuetypid == INT4OID)
		result->low.x = result->high.x = (double)(DatumGetInt32(value));
	if (valuetypid == FLOAT8OID)
		result->low.x = result->high.x = DatumGetFloat8(value);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
	return result;
}

PG_FUNCTION_INFO_V1(base_period_to_box);

PGDLLEXPORT Datum 
base_period_to_box(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Period *p = PG_GETARG_PERIOD(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX *result = base_period_to_box_internal(value, p, valuetypid);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Transform a range and a timestamptz to a box */

BOX *
range_timestamp_to_box_internal(RangeType *range, TimestampTz t)
{
	BOX *result = palloc(sizeof(BOX));
	if (range->rangetypid == type_oid(T_INTRANGE))
	{
		result->low.x = (double)(DatumGetInt32(lower_datum(range)));
		result->high.x = (double)(DatumGetInt32(upper_datum(range)));
	}
	else if (range->rangetypid == type_oid(T_FLOATRANGE))
	{
		result->low.x = DatumGetFloat8(lower_datum(range));
		result->high.x = DatumGetFloat8(upper_datum(range));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	result->low.y = result->high.y = (double)t;
	return result;
}

PG_FUNCTION_INFO_V1(range_timestamp_to_box);

PGDLLEXPORT Datum 
range_timestamp_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	BOX *result = range_timestamp_to_box_internal(range, t);
	PG_FREE_IF_COPY(range, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Transform a range and a period to a box */

BOX *
range_period_to_box_internal(RangeType *range, Period *p)
{
	BOX *result = palloc(sizeof(BOX));
	if (range->rangetypid == type_oid(T_INTRANGE))
	{
		result->low.x = (double)(DatumGetInt32(lower_datum(range)));
		result->high.x = (double)(DatumGetInt32(upper_datum(range)));
	}
	else if (range->rangetypid == type_oid(T_FLOATRANGE))
	{
		result->low.x = DatumGetFloat8(lower_datum(range));
		result->high.x = DatumGetFloat8(upper_datum(range));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	result->low.y = (double)(p->lower);
	result->high.y = (double)(p->upper);
	return result;
}

PG_FUNCTION_INFO_V1(range_period_to_box);

PGDLLEXPORT Datum 
range_period_to_box(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Period *p = PG_GETARG_PERIOD(1);
	BOX *result = range_period_to_box_internal(range, p);
	PG_FREE_IF_COPY(range, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Overlaps for temporal types wrt the time dimension
 * The inclusive/exclusive bounds are taken into account for the comparisons 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_timestamp_temporal);

PGDLLEXPORT Datum
overlaps_bbox_timestamp_temporal(PG_FUNCTION_ARGS) 
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = contains_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_timestamp);

PGDLLEXPORT Datum
overlaps_bbox_temporal_timestamp(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = contains_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_timestampset_temporal);

PGDLLEXPORT Datum
overlaps_bbox_timestampset_temporal(PG_FUNCTION_ARGS) 
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = timestampset_bbox(ts);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = overlaps_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_timestampset);

PGDLLEXPORT Datum
overlaps_bbox_temporal_timestampset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = timestampset_bbox(ts);
	bool result = overlaps_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
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

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_periodset_temporal);

PGDLLEXPORT Datum
overlaps_bbox_periodset_temporal(PG_FUNCTION_ARGS) 
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = periodset_bbox(ps);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = overlaps_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overlaps_bbox_temporal_periodset);

PGDLLEXPORT Datum
overlaps_bbox_temporal_periodset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = periodset_bbox(ps);
	bool result = overlaps_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

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

/*****************************************************************************
 * Contains for temporal types wrt the time dimension
 * The inclusive/exclusive bounds are taken into account for the comparisons 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_timestamp_temporal);

PGDLLEXPORT Datum
contains_bbox_timestamp_temporal(PG_FUNCTION_ARGS) 
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = timestamp_cmp_internal(p.lower, t) == 0 &&
		timestamp_cmp_internal(p.upper, t) == 0;
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_timestamp);

PGDLLEXPORT Datum
contains_bbox_temporal_timestamp(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = contains_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_timestampset_temporal);

PGDLLEXPORT Datum
contains_bbox_timestampset_temporal(PG_FUNCTION_ARGS) 
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = timestampset_bbox(ts);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = contains_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_timestampset);

PGDLLEXPORT Datum
contains_bbox_temporal_timestampset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = timestampset_bbox(ts);
	bool result = contains_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

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

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_periodset_temporal);

PGDLLEXPORT Datum
contains_bbox_periodset_temporal(PG_FUNCTION_ARGS) 
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = periodset_bbox(ps);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = contains_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contains_bbox_temporal_periodset);

PGDLLEXPORT Datum
contains_bbox_temporal_periodset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = periodset_bbox(ps);
	bool result = contains_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

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

/*****************************************************************************
 * Contained for temporal types wrt the time dimension
 * The inclusive/exclusive bounds are taken into account for the comparisons 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_timestamp_temporal);

PGDLLEXPORT Datum
contained_bbox_timestamp_temporal(PG_FUNCTION_ARGS) 
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = contains_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_timestamp);

PGDLLEXPORT Datum
contained_bbox_temporal_timestamp(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = timestamp_cmp_internal(p.lower, t) == 0 &&
		timestamp_cmp_internal(p.upper, t) == 0;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_timestampset_temporal);

PGDLLEXPORT Datum
contained_bbox_timestampset_temporal(PG_FUNCTION_ARGS) 
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = timestampset_bbox(ts);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = contained_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_timestampset);

PGDLLEXPORT Datum
contained_bbox_temporal_timestampset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = timestampset_bbox(ts);
	bool result = contained_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
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
	bool result = contained_period_period_internal(p, &p1);
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
	bool result = contained_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_periodset_temporal);

PGDLLEXPORT Datum
contained_bbox_periodset_temporal(PG_FUNCTION_ARGS) 
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = periodset_bbox(ps);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = contained_period_period_internal(p1, &p2);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(contained_bbox_temporal_periodset);

PGDLLEXPORT Datum
contained_bbox_temporal_periodset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = periodset_bbox(ps);
	bool result = contained_period_period_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_temporal_temporal);

PGDLLEXPORT Datum
contained_bbox_temporal_temporal(PG_FUNCTION_ARGS) 
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = contained_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Same for temporal types wrt the time dimension
 * The inclusive/exclusive bounds are taken into account for the comparisons 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_timestamp_temporal);

PGDLLEXPORT Datum
same_bbox_timestamp_temporal(PG_FUNCTION_ARGS) 
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = timestamp_cmp_internal(p.lower, t) == 0 &&
		timestamp_cmp_internal(p.upper, t) == 0;
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_timestamp);

PGDLLEXPORT Datum
same_bbox_temporal_timestamp(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = timestamp_cmp_internal(p.lower, t) == 0 &&
		timestamp_cmp_internal(p.upper, t) == 0;
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_timestampset_temporal);

PGDLLEXPORT Datum
same_bbox_timestampset_temporal(PG_FUNCTION_ARGS) 
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = timestampset_bbox(ts);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = period_eq_internal(p1, &p2);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_timestampset);

PGDLLEXPORT Datum
same_bbox_temporal_timestampset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = timestampset_bbox(ts);
	bool result = period_eq_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
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

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_periodset_temporal);

PGDLLEXPORT Datum
same_bbox_periodset_temporal(PG_FUNCTION_ARGS) 
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period *p1 = periodset_bbox(ps);
	Period p2;
	temporal_timespan_internal(&p2, temp);
	bool result = period_eq_internal(p1, &p2);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(same_bbox_temporal_periodset);

PGDLLEXPORT Datum
same_bbox_temporal_periodset(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	Period *p2 = periodset_bbox(ps);
	bool result = period_eq_internal(&p1, p2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

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
 * overlaps for tnumber
 *****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_datum_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_datum_tnumber(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = overlaps_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

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

PG_FUNCTION_INFO_V1(overlaps_bbox_box_tnumber);

PGDLLEXPORT Datum
overlaps_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overlaps_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(overlaps_bbox_tnumber_datum);

PGDLLEXPORT Datum
overlaps_bbox_tnumber_datum(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = overlaps_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
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
	
/*****************************************************************************
 * contains for tnumber
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_datum_tnumber);

PGDLLEXPORT Datum
contains_bbox_datum_tnumber(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = contains_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

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

PG_FUNCTION_INFO_V1(contains_bbox_box_tnumber);

PGDLLEXPORT Datum
contains_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contains_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contains_bbox_tnumber_datum);

PGDLLEXPORT Datum
contains_bbox_tnumber_datum(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = contains_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
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
	
/*****************************************************************************
 * contained for tnumber
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_datum_tnumber);

PGDLLEXPORT Datum
contained_bbox_datum_tnumber(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = contained_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

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

PG_FUNCTION_INFO_V1(contained_bbox_box_tnumber);

PGDLLEXPORT Datum
contained_bbox_box_tnumber(PG_FUNCTION_ARGS) 
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = contained_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(contained_bbox_tnumber_datum);

PGDLLEXPORT Datum
contained_bbox_tnumber_datum(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = contained_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
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
	
/*****************************************************************************
 * same for tnumber
 *****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_datum_tnumber);

PGDLLEXPORT Datum
same_bbox_datum_tnumber(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = same_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
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
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(same_bbox_tnumber_datum);

PGDLLEXPORT Datum
same_bbox_tnumber_datum(PG_FUNCTION_ARGS) 
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = same_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

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
