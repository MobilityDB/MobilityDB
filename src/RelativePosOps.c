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
/* Period op Temporal */

PG_FUNCTION_INFO_V1(before_period_temporal);

PGDLLEXPORT Datum
before_period_temporal(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
	bool result = overafter_period_period_internal(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
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
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
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
	temporal_bbox(&p1, temp);
	bool result = overafter_period_period_internal(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(before_temporal_temporal);

PGDLLEXPORT Datum
before_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_bbox(&p1, temp1);
	temporal_bbox(&p2, temp2);
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
	temporal_bbox(&p1, temp1);
	temporal_bbox(&p2, temp2);
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
	temporal_bbox(&p1, temp1);
	temporal_bbox(&p2, temp2);
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
	temporal_bbox(&p1, temp1);
	temporal_bbox(&p2, temp2);
	bool result = overafter_period_period_internal(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Range op Tnumber */

PG_FUNCTION_INFO_V1(left_range_tnumber);

PGDLLEXPORT Datum
left_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_range_tnumber);

PGDLLEXPORT Datum
overleft_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_range_tnumber);

PGDLLEXPORT Datum
right_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_range_tnumber);

PGDLLEXPORT Datum
overright_range_tnumber(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1, box2;
	range_to_box(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Tnumber op Range */

PG_FUNCTION_INFO_V1(left_tnumber_range);

PGDLLEXPORT Datum
left_tnumber_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = left_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_range);

PGDLLEXPORT Datum
overleft_tnumber_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = overleft_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnumber_range);

PGDLLEXPORT Datum
right_tnumber_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = right_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnumber_range);

PGDLLEXPORT Datum
overright_tnumber_range(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	RangeType *range = PG_GETARG_RANGE_P(1);
	BOX box1, box2;
	temporal_bbox(&box1, temp);
	range_to_box(&box2, range);
	bool result = overright_box_box_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* BOX op Tnumber */

PG_FUNCTION_INFO_V1(left_box_tnumber);

PGDLLEXPORT Datum
left_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_box_tnumber);

PGDLLEXPORT Datum
overleft_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_box_tnumber);

PGDLLEXPORT Datum
right_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_box_tnumber);

PGDLLEXPORT Datum
overright_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_box_tnumber);

PGDLLEXPORT Datum
before_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = before_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_box_tnumber);

PGDLLEXPORT Datum
overbefore_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbefore_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_box_tnumber);

PGDLLEXPORT Datum
after_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = after_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_box_tnumber);

PGDLLEXPORT Datum
overafter_box_tnumber(PG_FUNCTION_ARGS)
{
	BOX *box = PG_GETARG_BOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overafter_box_box_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Tnumber op BOX */

PG_FUNCTION_INFO_V1(left_tnumber_box);

PGDLLEXPORT Datum
left_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = left_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_box);

PGDLLEXPORT Datum
overleft_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overleft_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnumber_box);

PGDLLEXPORT Datum
right_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = right_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnumber_box);

PGDLLEXPORT Datum
overright_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overright_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tnumber_box);

PGDLLEXPORT Datum
before_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = before_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_box);

PGDLLEXPORT Datum
overbefore_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = overbefore_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tnumber_box);

PGDLLEXPORT Datum
after_tnumber_box(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	BOX *box = PG_GETARG_BOX_P(1);
	BOX box1;
	temporal_bbox(&box1, temp);
	bool result = after_box_box_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_box);

PGDLLEXPORT Datum
overafter_tnumber_box(PG_FUNCTION_ARGS)
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
/* Tnumber op Tnumber */

PG_FUNCTION_INFO_V1(left_tnumber_tnumber);

PGDLLEXPORT Datum
left_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(overleft_tnumber_tnumber);

PGDLLEXPORT Datum
overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(right_tnumber_tnumber);

PGDLLEXPORT Datum
right_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(overright_tnumber_tnumber);

PGDLLEXPORT Datum
overright_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(before_tnumber_tnumber);

PGDLLEXPORT Datum
before_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(overbefore_tnumber_tnumber);

PGDLLEXPORT Datum
overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(after_tnumber_tnumber);

PGDLLEXPORT Datum
after_tnumber_tnumber(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(overafter_tnumber_tnumber);

PGDLLEXPORT Datum
overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
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
