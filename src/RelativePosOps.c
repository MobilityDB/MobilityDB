/*****************************************************************************
 *
 * RelativePosOps.c
 *	  Relative position operators for temporal types.
 *
 * The operators are the following
 * - left, overleft, right, overright for the value dimension
 * - before, overbefore, after, overafter for the time dimension
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
 
#include "TemporalTypes.h"

/*****************************************************************************/
/* BOX op BOX */

/*
 * Is the first box strictly to the left of the second box?
 */
bool
left_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->high.x < box2->low.x);
}

/*
 * Is the first box to the left of or in the second box?
 */
bool
overleft_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->high.x <= box2->high.x);
}

/*
 * Is the first box strictly to the right of the second box?
 */
bool
right_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->low.x > box2->high.x);
}

/*
 * Is the first box to the right of or in the second box?
 */
bool
overright_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->low.x >= box2->low.x);
}

/*
 * Is the first box strictly before the second box?
 */
bool
before_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->high.y < box2->low.y);
}

/*
 * Is the first box before or in the second box?
 */
bool
overbefore_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->high.y <= box2->high.y);
}

/*
 * Is the first box strictly after the second box?
 */
bool
after_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->low.y > box2->high.y);
}

/*
 * Is the first box after or in the second box?
 */
bool
overafter_box_box_internal(BOX *box1, BOX *box2)
{
	return (box1->low.y >= box2->low.y);
}

/*****************************************************************************/
/* Datum op Temporal */

PG_FUNCTION_INFO_V1(left_datum_temporal);

PGDLLEXPORT Datum
left_datum_temporal(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_datum_temporal);

PGDLLEXPORT Datum
overleft_datum_temporal(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_datum_temporal);

PGDLLEXPORT Datum
right_datum_temporal(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_datum_temporal);

PGDLLEXPORT Datum
overright_datum_temporal(PG_FUNCTION_ARGS)
{
	Datum d = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	base_to_box(&box1, d, valuetypid);
	temporal_bbox(&box2, temp);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* RangeType op Temporal */

PG_FUNCTION_INFO_V1(left_range_temporal);

PGDLLEXPORT Datum
left_range_temporal(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	range_to_box(&box1, range, rangetypid);
	temporal_bbox(&box2, temp);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_range_temporal);

PGDLLEXPORT Datum
overleft_range_temporal(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	range_to_box(&box1, range, rangetypid);
	temporal_bbox(&box2, temp);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_range_temporal);

PGDLLEXPORT Datum
right_range_temporal(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	range_to_box(&box1, range, rangetypid);
	temporal_bbox(&box2, temp);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_range_temporal);

PGDLLEXPORT Datum
overright_range_temporal(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	BOX box1, box2;
	range_to_box(&box1, range, rangetypid);
	temporal_bbox(&box2, temp);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* TimestampTz op Temporal */

PG_FUNCTION_INFO_V1(before_timestamp_temporal);

PGDLLEXPORT Datum
before_timestamp_temporal(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_timestamp_period_internal(t, &p);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_timestamp_temporal);

PGDLLEXPORT Datum
overbefore_timestamp_temporal(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_timestamp_period_internal(t, &p);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_timestamp_temporal);

PGDLLEXPORT Datum
after_timestamp_temporal(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_timestamp_period_internal(t, &p);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_timestamp_temporal);

PGDLLEXPORT Datum
overafter_timestamp_temporal(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_timestamp_period_internal(t, &p);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* TimestampSet op Temporal */

PG_FUNCTION_INFO_V1(before_timestampset_temporal);

PGDLLEXPORT Datum
before_timestampset_temporal(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_timestampset_period_internal(ts, &p);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_timestampset_temporal);

PGDLLEXPORT Datum
overbefore_timestampset_temporal(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_timestampset_period_internal(ts, &p);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_timestampset_temporal);

PGDLLEXPORT Datum
after_timestampset_temporal(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_timestampset_period_internal(ts, &p);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_timestampset_temporal);

PGDLLEXPORT Datum
overafter_timestampset_temporal(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_timestampset_period_internal(ts, &p);
	PG_FREE_IF_COPY(ts, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Period op Temporal */

PG_FUNCTION_INFO_V1(before_period_temporal);

PGDLLEXPORT Datum
before_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = before_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_period_temporal);

PGDLLEXPORT Datum
overbefore_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overbefore_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_period_temporal);

PGDLLEXPORT Datum
after_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = after_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_period_temporal);

PGDLLEXPORT Datum
overafter_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overafter_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* PeriodSet op Temporal */

PG_FUNCTION_INFO_V1(before_periodset_temporal);

PGDLLEXPORT Datum
before_periodset_temporal(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_periodset_period_internal(ps, &p);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_periodset_temporal);

PGDLLEXPORT Datum
overbefore_periodset_temporal(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_periodset_period_internal(ps, &p);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_periodset_temporal);

PGDLLEXPORT Datum
after_periodset_temporal(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_periodset_period_internal(ps, &p);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_periodset_temporal);

PGDLLEXPORT Datum
overafter_periodset_temporal(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_periodset_period_internal(ps, &p);
	PG_FREE_IF_COPY(ps, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* BOX op Temporal */

PG_FUNCTION_INFO_V1(left_box_temporal);

PGDLLEXPORT Datum
left_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_box_temporal);

PGDLLEXPORT Datum
overleft_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_box_temporal);

PGDLLEXPORT Datum
right_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_box_temporal);

PGDLLEXPORT Datum
overright_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_box_temporal);

PGDLLEXPORT Datum
before_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = before_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_box_temporal);

PGDLLEXPORT Datum
overbefore_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbefore_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_box_temporal);

PGDLLEXPORT Datum
after_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = after_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_box_temporal);

PGDLLEXPORT Datum
overafter_box_temporal(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overafter_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Datum */

PG_FUNCTION_INFO_V1(left_temporal_datum);

PGDLLEXPORT Datum
left_temporal_datum(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_temporal_datum);

PGDLLEXPORT Datum
overleft_temporal_datum(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_temporal_datum);

PGDLLEXPORT Datum
right_temporal_datum(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_temporal_datum);

PGDLLEXPORT Datum
overright_temporal_datum(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum d = PG_GETARG_DATUM(1);
	Oid	valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	base_to_box(&box2, d, valuetypid);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Range */

PG_FUNCTION_INFO_V1(left_temporal_range);

PGDLLEXPORT Datum
left_temporal_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range, rangetypid);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_temporal_range);

PGDLLEXPORT Datum
overleft_temporal_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range, rangetypid);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_temporal_range);

PGDLLEXPORT Datum
right_temporal_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range, rangetypid);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_temporal_range);

PGDLLEXPORT Datum
overright_temporal_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	Oid	rangetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range, rangetypid);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}


PG_FUNCTION_INFO_V1(before_temporal_timestamp);

PGDLLEXPORT Datum
before_temporal_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_timestamp);

PGDLLEXPORT Datum
overbefore_temporal_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_timestamp);

PGDLLEXPORT Datum
after_temporal_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_timestamp);

PGDLLEXPORT Datum
overafter_temporal_timestamp(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_period_timestamp_internal(&p, t);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op TimestampSet */

PG_FUNCTION_INFO_V1(before_temporal_timestampset);

PGDLLEXPORT Datum
before_temporal_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_period_timestampset_internal(&p, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_timestampset);

PGDLLEXPORT Datum
overbefore_temporal_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_period_timestampset_internal(&p, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_timestampset);

PGDLLEXPORT Datum
after_temporal_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_period_timestampset_internal(&p, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_timestampset);

PGDLLEXPORT Datum
overafter_temporal_timestampset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_period_timestampset_internal(&p, ts);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Period */

PG_FUNCTION_INFO_V1(before_temporal_period);

PGDLLEXPORT Datum
before_temporal_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = before_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_period);

PGDLLEXPORT Datum
overbefore_temporal_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overbefore_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_period);

PGDLLEXPORT Datum
after_temporal_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = after_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_period);

PGDLLEXPORT Datum
overafter_temporal_period(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_timespan_internal(&p1, temp);
	bool result = overafter_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op PeriodSet */

PG_FUNCTION_INFO_V1(before_temporal_periodset);

PGDLLEXPORT Datum
before_temporal_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = before_period_periodset_internal(&p, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_periodset);

PGDLLEXPORT Datum
overbefore_temporal_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overbefore_period_periodset_internal(&p, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_periodset);

PGDLLEXPORT Datum
after_temporal_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = after_period_periodset_internal(&p, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_periodset);

PGDLLEXPORT Datum
overafter_temporal_periodset(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	Period p;
	temporal_timespan_internal(&p, temp);
	bool result = overafter_period_periodset_internal(&p, ps);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op BOX */

PG_FUNCTION_INFO_V1(left_temporal_box);

PGDLLEXPORT Datum
left_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_temporal_box);

PGDLLEXPORT Datum
overleft_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_temporal_box);

PGDLLEXPORT Datum
right_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_temporal_box);

PGDLLEXPORT Datum
overright_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_temporal_box);

PGDLLEXPORT Datum
before_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = before_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_box);

PGDLLEXPORT Datum
overbefore_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbefore_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_box);

PGDLLEXPORT Datum
after_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = after_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_box);

PGDLLEXPORT Datum
overafter_temporal_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overafter_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(left_temporal_temporal);

PGDLLEXPORT Datum
left_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_temporal_temporal);

PGDLLEXPORT Datum
overleft_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_temporal_temporal);

PGDLLEXPORT Datum
right_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_temporal_temporal);

PGDLLEXPORT Datum
overright_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_temporal_temporal);

PGDLLEXPORT Datum
before_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = before_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_temporal);

PGDLLEXPORT Datum
overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = overbefore_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_temporal_temporal);

PGDLLEXPORT Datum
after_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = after_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_temporal_temporal);

PGDLLEXPORT Datum
overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_timespan_internal(&p1, temp1);
	temporal_timespan_internal(&p2, temp2);
	bool result = overafter_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
