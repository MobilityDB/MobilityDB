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
/* TBOX op TBOX */

/*
 * Is the first box strictly to the left of the second box?
 */
bool
left_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmax < box2->xmin);
}

PG_FUNCTION_INFO_V1(left_tbox_tbox);

PGDLLEXPORT Datum
left_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = left_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box to the left of or in the second box?
 */
bool
overleft_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmax <= box2->xmax);
}

PG_FUNCTION_INFO_V1(overleft_tbox_tbox);

PGDLLEXPORT Datum
overleft_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = overleft_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box strictly to the right of the second box?
 */
bool
right_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmin > box2->xmax);
}

PG_FUNCTION_INFO_V1(right_tbox_tbox);

PGDLLEXPORT Datum
right_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = right_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box to the right of or in the second box?
 */
bool
overright_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags));
	return (box1->xmin >= box2->xmin);
}

PG_FUNCTION_INFO_V1(overright_tbox_tbox);

PGDLLEXPORT Datum
overright_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = overright_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box strictly before the second box?
 */
bool
before_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmax < box2->tmin);
}

PG_FUNCTION_INFO_V1(before_tbox_tbox);

PGDLLEXPORT Datum
before_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = before_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box before or in the second box?
 */
bool
overbefore_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmax <= box2->tmax);
}

PG_FUNCTION_INFO_V1(overbefore_tbox_tbox);

PGDLLEXPORT Datum
overbefore_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = overbefore_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box strictly after the second box?
 */
bool
after_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmin > box2->tmax);
}

PG_FUNCTION_INFO_V1(after_tbox_tbox);

PGDLLEXPORT Datum
after_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = after_tbox_tbox_internal(box1, box2);
	PG_RETURN_BOOL(result);
}

/*
 * Is the first box after or in the second box?
 */
bool
overafter_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	assert(MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags));
	return (box1->tmin >= box2->tmin);
}

PG_FUNCTION_INFO_V1(overafter_tbox_tbox);

PGDLLEXPORT Datum
overafter_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	bool result = overafter_tbox_tbox_internal(box1, box2);
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
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = left_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overleft_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = right_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = overright_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = left_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = overleft_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = right_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = overright_tbox_tbox_internal(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* TBOX op Temporal */

PG_FUNCTION_INFO_V1(left_tbox_tnumber);

PGDLLEXPORT Datum
left_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = left_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tbox_tnumber);

PGDLLEXPORT Datum
overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overleft_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tbox_tnumber);

PGDLLEXPORT Datum
right_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = right_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tbox_tnumber);

PGDLLEXPORT Datum
overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overright_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tbox_tnumber);

PGDLLEXPORT Datum
before_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = before_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tbox_tnumber);

PGDLLEXPORT Datum
overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overbefore_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tbox_tnumber);

PGDLLEXPORT Datum
after_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = after_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tbox_tnumber);

PGDLLEXPORT Datum
overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overafter_tbox_tbox_internal(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Temporal op TBOX */

PG_FUNCTION_INFO_V1(left_tnumber_tbox);

PGDLLEXPORT Datum
left_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = left_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_tbox);

PGDLLEXPORT Datum
overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overleft_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(right_tnumber_tbox);

PGDLLEXPORT Datum
right_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = right_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overright_tnumber_tbox);

PGDLLEXPORT Datum
overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overright_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(before_tnumber_tbox);

PGDLLEXPORT Datum
before_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = before_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_tbox);

PGDLLEXPORT Datum
overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overbefore_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(after_tnumber_tbox);

PGDLLEXPORT Datum
after_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = after_tbox_tbox_internal(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_tbox);

PGDLLEXPORT Datum
overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1 = {0};
	temporal_bbox(&box1, temp);
	bool result = overafter_tbox_tbox_internal(&box1, box);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = left_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overleft_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = right_tbox_tbox_internal(&box1, &box2);
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
	TBOX box1 = {0}, box2 = {0};
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = overright_tbox_tbox_internal(&box1, &box2);
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
