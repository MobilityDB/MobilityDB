/*****************************************************************************
 *
 * temporal_posops.c
 *	  Relative position operators for temporal types.
 *
 * The operators are the following
 * - left, overleft, right, overright for the value dimension
 * - before, overbefore, after, overafter for the time dimension
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_posops.h"

#include <assert.h>

#include "timeops.h"
#include "temporal.h"
#include "temporal_boxops.h"

/*****************************************************************************/

/**
 * Generic position function for a period and a temporal value
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
posop_period_temporal(FunctionCallInfo fcinfo, 
	bool (*func)(const Period *, const Period *))
{
	Period *p = PG_GETARG_PERIOD(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Period p1;
	temporal_bbox(&p1, temp);
	bool result = func(p, &p1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for a temporal value and a period
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
posop_temporal_period(FunctionCallInfo fcinfo, 
	bool (*func)(const Period *, const Period *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *p = PG_GETARG_PERIOD(1);
	Period p1;
	temporal_bbox(&p1, temp);
	bool result = func(&p1, p);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/**
 * Generic position function for two temporal values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 */
Datum
posop_temporal_temporal(FunctionCallInfo fcinfo, 
	bool (*func)(const Period *, const Period *))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Period p1, p2;
	temporal_bbox(&p1, temp1);
	temporal_bbox(&p2, temp2);
	bool result = func(&p1, &p2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Period op Temporal */

PG_FUNCTION_INFO_V1(before_period_temporal);
/**
 * Returns true if the period value is strictly before the temporal value
 */
PGDLLEXPORT Datum
before_period_temporal(PG_FUNCTION_ARGS)
{
	return posop_period_temporal(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_period_temporal);
/**
 * Returns true if the period value is not after the temporal value
 */
PGDLLEXPORT Datum
overbefore_period_temporal(PG_FUNCTION_ARGS)
{
	return posop_period_temporal(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_period_temporal);
/**
 * Returns true if the period value is strictly after the temporal value
 */
PGDLLEXPORT Datum
after_period_temporal(PG_FUNCTION_ARGS)
{
	return posop_period_temporal(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_period_temporal);
/**
 * Returns true if the period value is not before the temporal value
 */
PGDLLEXPORT Datum
overafter_period_temporal(PG_FUNCTION_ARGS)
{
	return posop_period_temporal(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************/
/* Temporal op Period */

PG_FUNCTION_INFO_V1(before_temporal_period);
/**
 * Returns true if the temporal value is strictly before the period value
 */
PGDLLEXPORT Datum
before_temporal_period(PG_FUNCTION_ARGS)
{
	return posop_temporal_period(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_period);
/**
 * Returns true if the temporal value is not after the period value
 */
PGDLLEXPORT Datum
overbefore_temporal_period(PG_FUNCTION_ARGS)
{
	return posop_temporal_period(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_temporal_period);
/**
 * Returns true if the temporal value is strictly after the period value
 */
PGDLLEXPORT Datum
after_temporal_period(PG_FUNCTION_ARGS)
{
	return posop_temporal_period(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_temporal_period);
/**
 * Returns true if the temporal value is not before the period value
 */
PGDLLEXPORT Datum
overafter_temporal_period(PG_FUNCTION_ARGS)
{
	return posop_temporal_period(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************/
/* Temporal op Temporal */

PG_FUNCTION_INFO_V1(before_temporal_temporal);
/**
 * Returns true if the first temporal value is strictly before the second one
 */
PGDLLEXPORT Datum
before_temporal_temporal(PG_FUNCTION_ARGS)
{
	return posop_temporal_temporal(fcinfo, &before_period_period_internal);
}

PG_FUNCTION_INFO_V1(overbefore_temporal_temporal);
/**
 * Returns true if the first temporal value is not after the second one
 */
PGDLLEXPORT Datum
overbefore_temporal_temporal(PG_FUNCTION_ARGS)
{
	return posop_temporal_temporal(fcinfo, &overbefore_period_period_internal);
}

PG_FUNCTION_INFO_V1(after_temporal_temporal);
/**
 * Returns true if the first temporal value is strictly after the second one
 */
PGDLLEXPORT Datum
after_temporal_temporal(PG_FUNCTION_ARGS)
{
	return posop_temporal_temporal(fcinfo, &after_period_period_internal);
}

PG_FUNCTION_INFO_V1(overafter_temporal_temporal);
/**
 * Returns true if the first temporal value is not before the second one
 */
PGDLLEXPORT Datum
overafter_temporal_temporal(PG_FUNCTION_ARGS)
{
	return posop_temporal_temporal(fcinfo, &overafter_period_period_internal);
}

/*****************************************************************************
 * Generic functions 
 *****************************************************************************/

/**
 * Returns true if the the numeric range value and temporal numeric value
 * satisfy the position operator
 */
Datum
posop_range_tnumber(FunctionCallInfo fcinfo, 
	bool (*func)(const TBOX *, const TBOX *))
{
#if MOBDB_PGSQL_VERSION < 110000
	RangeType  *range = PG_GETARG_RANGE(0);
#else
	RangeType  *range = PG_GETARG_RANGE_P(0);
#endif
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	range_to_tbox_internal(&box1, range);
	temporal_bbox(&box2, temp);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(range, 0);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Returns true if the temporal numeric value and the numeric range value
 * satisfy the position operator
 */
Datum
posop_tnumber_range(FunctionCallInfo fcinfo, 
	bool (*func)(const TBOX *, const TBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
#if MOBDB_PGSQL_VERSION < 110000
	RangeType  *range = PG_GETARG_RANGE(1);
#else
	RangeType  *range = PG_GETARG_RANGE_P(1);
#endif
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	range_to_tbox_internal(&box2, range);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp, 0);
	PG_FREE_IF_COPY(range, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Returns true if the temporal box value and the temporal numeric value
 * satisfy the position operator
 */
Datum
posop_tbox_tnumber(FunctionCallInfo fcinfo, 
	bool (*func)(const TBOX *, const TBOX *))
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	bool result = func(box, &box1);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Returns true if the temporal numeric value and the temporal box value
 * satisfy the position operator
 */
Datum
posop_tnumber_tbox(FunctionCallInfo fcinfo, 
	bool (*func)(const TBOX *, const TBOX *))
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	TBOX *box = PG_GETARG_TBOX_P(1);
	TBOX box1;
	memset(&box1, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp);
	bool result = func(&box1, box);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_BOOL(result);
}

/**
 * Returns true if the temporal numeric values satisfy the position operator
 */
Datum
posop_tnumber_tnumber(FunctionCallInfo fcinfo, 
	bool (*func)(const TBOX *, const TBOX *))
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporal_bbox(&box1, temp1);
	temporal_bbox(&box2, temp2);
	bool result = func(&box1, &box2);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	PG_RETURN_BOOL(result);
}

/*****************************************************************************/
/* Range op Tnumber */

PG_FUNCTION_INFO_V1(left_range_tnumber);
/**
 * Returns true if the numeric range value is strictly to the left of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
left_range_tnumber(PG_FUNCTION_ARGS)
{
	return posop_range_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_range_tnumber);
/**
 * Returns true if the numeric range value is not to the right of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overleft_range_tnumber(PG_FUNCTION_ARGS)
{
	return posop_range_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_range_tnumber);
/**
 * Returns true if the numeric range value is strictly to the right of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
right_range_tnumber(PG_FUNCTION_ARGS)
{
	return posop_range_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_range_tnumber);
/**
 * Returns true if the numeric range value is not to the left of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overright_range_tnumber(PG_FUNCTION_ARGS)
{
	return posop_range_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

/*****************************************************************************/
/* Tnumber op Range */

PG_FUNCTION_INFO_V1(left_tnumber_range);
/**
 * Returns true if the temporal numeric value is strictly to the left of the
 * numeric range value
 */
PGDLLEXPORT Datum
left_tnumber_range(PG_FUNCTION_ARGS)
{
	return posop_tnumber_range(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_range);
/**
 * Returns true if the temporal numeric value is not to the right of the
 * numeric range value
 */
PGDLLEXPORT Datum
overleft_tnumber_range(PG_FUNCTION_ARGS)
{
	return posop_tnumber_range(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_range);
/**
 * Returns true if the temporal numeric value is strictly to the right of the
 * numeric range value
 */
PGDLLEXPORT Datum
right_tnumber_range(PG_FUNCTION_ARGS)
{
	return posop_tnumber_range(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_range);
/**
 * Returns true if the temporal numeric value is not to the left of the
 * numeric range value
 */
PGDLLEXPORT Datum
overright_tnumber_range(PG_FUNCTION_ARGS)
{
	return posop_tnumber_range(fcinfo, &overright_tbox_tbox_internal);
}

/*****************************************************************************/
/* TBOX op Temporal */

PG_FUNCTION_INFO_V1(left_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly to the left of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
left_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tbox_tnumber);
/**
 * Returns true if the temporal box value is not to the right of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overleft_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly to the right of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
right_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tbox_tnumber);
/**
 * Returns true if the temporal box value is not to the left of the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overright_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly before the
 * temporal numeric value
 */
PGDLLEXPORT Datum
before_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tbox_tnumber);
/**
 * Returns true if the temporal box value is not after the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overbefore_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tbox_tnumber);
/**
 * Returns true if the temporal box value is strictly after the
 * temporal numeric value
 */
PGDLLEXPORT Datum
after_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tbox_tnumber);
/**
 * Returns true if the temporal box value is not before the
 * temporal numeric value
 */
PGDLLEXPORT Datum
overafter_tbox_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tbox_tnumber(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
/* Temporal op TBOX */

PG_FUNCTION_INFO_V1(left_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is strictly to the left of
 * the temporal box value
 */
PGDLLEXPORT Datum
left_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is not to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
overleft_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is strictly to the right of
 * the temporal box value
 */
PGDLLEXPORT Datum
right_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is not to the left of the
 * temporal box value
 */
PGDLLEXPORT Datum
overright_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is strictly before the
 * temporal box value
 */
PGDLLEXPORT Datum
before_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is not after the
 * temporal box value
 */
PGDLLEXPORT Datum
overbefore_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is strictly after the
 * temporal box value
 */
PGDLLEXPORT Datum
after_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_tbox);
/**
 * Returns true if the temporal numeric value is not before the
 * temporal box value
 */
PGDLLEXPORT Datum
overafter_tnumber_tbox(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tbox(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
/* Tnumber op Tnumber */

PG_FUNCTION_INFO_V1(left_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is strictly to the left of
 * the second one
 */
PGDLLEXPORT Datum
left_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &left_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overleft_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is not to the right of
 * the second one
 */
PGDLLEXPORT Datum
overleft_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &overleft_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(right_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is strictly to the right of
 * the second one
 */
PGDLLEXPORT Datum
right_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &right_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overright_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is not to the left of
 * the second one
 */
PGDLLEXPORT Datum
overright_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &overright_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(before_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is strictly before
 * the second one
 */
PGDLLEXPORT Datum
before_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &before_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overbefore_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is not after
 * the second one
 */
PGDLLEXPORT Datum
overbefore_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &overbefore_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(after_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is strictly after
 * the second one
 */
PGDLLEXPORT Datum
after_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &after_tbox_tbox_internal);
}

PG_FUNCTION_INFO_V1(overafter_tnumber_tnumber);
/**
 * Returns true if the first temporal numeric value is not before
 * the second one
 */
PGDLLEXPORT Datum
overafter_tnumber_tnumber(PG_FUNCTION_ARGS)
{
	return posop_tnumber_tnumber(fcinfo, &overafter_tbox_tbox_internal);
}

/*****************************************************************************/
