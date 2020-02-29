/*****************************************************************************
 *
 * tbox.c
 *	  Basic functions for TBOX bounding box.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tbox.h"

#include <assert.h>
#include <utils/timestamp.h>

#include "period.h"
#include "rangetypes_ext.h"
#include "temporal.h"
#include "temporal_parser.h"
#include "temporal_util.h"

/* Buffer size for input and output of TBOX */
#define MAXTBOXLEN		128

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

TBOX *
tbox_new(bool hasx, bool hast)
{
	TBOX *result = palloc0(sizeof(TBOX));
	MOBDB_FLAGS_SET_X(result->flags, hasx);
	MOBDB_FLAGS_SET_T(result->flags, hast);
	return result;
}

TBOX *
tbox_copy(const STBOX *box)
{
	TBOX *result = palloc0(sizeof(TBOX));
	memcpy(result, box, sizeof(TBOX));
	return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* 
 * Input function. 
 * Examples of input:
 * 		TBOX((1.0, 2.0), (1.0, 2.0)) 	-- Both X and T dimensions
 * 		TBOX((1.0, ), (1.0, ))			-- Only X dimension
 * 		TBOX((, 2.0), (, 2.0))			-- Only T dimension
 * where the commas are optional
 */
PG_FUNCTION_INFO_V1(tbox_in);

PGDLLEXPORT Datum
tbox_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	TBOX *result = tbox_parse(&input);
	PG_RETURN_POINTER(result);
}

char *
tbox_to_string(const TBOX *box)
{
	static size_t size = MAXTBOXLEN + 1;
	char *str = NULL, *strtmin = NULL, *strtmax = NULL;
	str = (char *) palloc(size);
	assert(MOBDB_FLAGS_GET_X(box->flags) || MOBDB_FLAGS_GET_T(box->flags));
	if (MOBDB_FLAGS_GET_T(box->flags))
	{
		strtmin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmin));
		strtmax = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(box->tmax));
	}
	if (MOBDB_FLAGS_GET_X(box->flags))
	{
		if (MOBDB_FLAGS_GET_T(box->flags))
			snprintf(str, size, "TBOX((%.8g,%s),(%.8g,%s))", 
				box->xmin, strtmin, box->xmax, strtmax);
		else 
			snprintf(str, size, "TBOX((%.8g,),(%.8g,))", 
				box->xmin,box->xmax);
	}
	else
		/* Missing X dimension */
		snprintf(str, size, "TBOX((,%s),(,%s))", 
			strtmin, strtmax);
	if (MOBDB_FLAGS_GET_T(box->flags))
	{
		pfree(strtmin);
		pfree(strtmax);
	}
	return str;
}

/* 
 * Output function. 
 */
PG_FUNCTION_INFO_V1(tbox_out);

PGDLLEXPORT Datum
tbox_out(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	char *result = tbox_to_string(box);
	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbox_constructor);

PGDLLEXPORT Datum
tbox_constructor(PG_FUNCTION_ARGS)
{
	double xmin = 0, xmax = 0, tmp; /* keep compiler quiet */
	TimestampTz tmin, tmax, ttmp;
	bool hast = false;

	assert (PG_NARGS() == 2 || PG_NARGS() == 4);
	if (PG_NARGS() == 2)
	{
		xmin = PG_GETARG_FLOAT8(0);
		xmax = PG_GETARG_FLOAT8(1);
	}
	else if (PG_NARGS() == 4)
	{
		xmin = PG_GETARG_FLOAT8(0);
		tmin = PG_GETARG_TIMESTAMPTZ(1);
		xmax = PG_GETARG_FLOAT8(2);
		tmax = PG_GETARG_TIMESTAMPTZ(3);
		hast = true;
	}

	TBOX *result = tbox_new(true, hast);

	/* Process X min/max */
	if (xmin > xmax)
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process T min/max */
	if (hast)
	{
		if (tmin > tmax)
		{
			ttmp = tmin;
			tmin = tmax;
			tmax = ttmp;
		}
		result->tmin = tmin;
		result->tmax = tmax;
	}
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tboxt_constructor);

PGDLLEXPORT Datum
tboxt_constructor(PG_FUNCTION_ARGS)
{
	TimestampTz tmin, tmax, ttmp;
	tmin = PG_GETARG_TIMESTAMPTZ(0);
	tmax = PG_GETARG_TIMESTAMPTZ(1);

	TBOX *result = tbox_new(false, true);

	/* Process T min/max */
	if (tmin > tmax)
	{
		ttmp = tmin;
		tmin = tmax;
		tmax = ttmp;
	}
	result->tmin = tmin;
	result->tmax = tmax;
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/* Cast a TBOX value as a floatrange value */

PG_FUNCTION_INFO_V1(tbox_to_floatrange);

PGDLLEXPORT Datum
tbox_to_floatrange(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	RangeType *result = range_make(Float8GetDatum(box->xmin),
		Float8GetDatum(box->xmax), true, true, FLOAT8OID);
	PG_RETURN_POINTER(result);
}

/* Cast a TBOX value as a Period value */

PG_FUNCTION_INFO_V1(tbox_to_period);

PGDLLEXPORT Datum
tbox_to_period(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		PG_RETURN_NULL();
	Period *result = period_make(box->tmin, box->tmax, true, true);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Get the minimum value of a TBOX value */

PG_FUNCTION_INFO_V1(tbox_xmin);

PGDLLEXPORT Datum
tbox_xmin(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->xmin);
}

/* Get the maximum value of a TBOX value */

PG_FUNCTION_INFO_V1(tbox_xmax);

PGDLLEXPORT Datum
tbox_xmax(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_X(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_FLOAT8(box->xmax);
}

/* Get the minimum timestamp of a TBOX value */

PG_FUNCTION_INFO_V1(tbox_tmin);

PGDLLEXPORT Datum
tbox_tmin(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(box->tmin);
}

/* Get the maximum timestamp of a TBOX value */

PG_FUNCTION_INFO_V1(tbox_tmax);

PGDLLEXPORT Datum
tbox_tmax(PG_FUNCTION_ARGS)
{
	TBOX *box = PG_GETARG_TBOX_P(0);
	if (!MOBDB_FLAGS_GET_T(box->flags))
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(box->tmax);
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

/* contains? */

bool
contains_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	/* The boxes should have at least one common dimension X or T  */
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	if (!hasx && !hast)
		elog(ERROR, "The boxes must have at least one common dimension");

	if (hasx && (box2->xmin < box1->xmin || box2->xmax > box1->xmax))
			return false;
	if (hast && (box2->tmin < box1->tmin || box2->tmax > box1->tmax))
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
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	if (!hasx && !hast)
		elog(ERROR, "The boxes must have at least one common dimension");

	if (hasx && (box1->xmax < box2->xmin || box1->xmin > box2->xmax))
			return false;
	if (hast && (box1->tmax < box2->tmin || box1->tmin > box2->tmax))
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
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	if (!hasx && !hast)
		elog(ERROR, "The boxes must have at least one common dimension");

	if (hasx && (box1->xmin != box2->xmin || box1->xmax != box2->xmax))
			return false;
	if (hast && (box1->tmin != box2->tmin || box1->tmax != box2->tmax))
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

/* adjacent? */

bool
adjacent_tbox_tbox_internal(const TBOX *box1, const TBOX *box2)
{
	/* The boxes should have at least one common dimension X or T  */
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	if (!hasx && !hast)
		elog(ERROR, "The boxes must have at least one common dimension");

	TBOX *inter = tbox_intersection_internal(box1, box2);
	if (inter == NULL)
		return false;
	/* Boxes are adjacent if they share n dimensions and their intersection is
	 * at most of n-1 dimensions */
	bool result;
	if (!hasx && hast)
		result = inter->tmin == inter->tmax;
	else if (hasx && !hast)
		result = inter->xmin == inter->xmax;
	else
		result = inter->xmin == inter->xmax || inter->tmin == inter->tmax;
	pfree(inter);
	return result;
}

PG_FUNCTION_INFO_V1(adjacent_tbox_tbox);

PGDLLEXPORT Datum
adjacent_tbox_tbox(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(adjacent_tbox_tbox_internal(box1, box2));
}

/*****************************************************************************
 * Topological perators
 *****************************************************************************/

/*
 * Is the first box strictly to the left of the second box?
 */
bool
left_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		elog(ERROR, "The boxes must have X dimension");

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
 * Is the first box to the left of or over the second box?
 */
bool
overleft_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		elog(ERROR, "The boxes must have X dimension");

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
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		elog(ERROR, "The boxes must have X dimension");

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
 * Is the first box to the right of or over the second box?
 */
bool
overright_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	if (! MOBDB_FLAGS_GET_X(box1->flags) || ! MOBDB_FLAGS_GET_X(box2->flags))
		elog(ERROR, "The boxes must have X dimension");

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
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		elog(ERROR, "The boxes must have T dimension");

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
 * Is the first box before or over the second box?
 */
bool
overbefore_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		elog(ERROR, "The boxes must have T dimension");

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
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		elog(ERROR, "The boxes must have T dimension");

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
 * Is the first box after or over the second box?
 */
bool
overafter_tbox_tbox_internal(TBOX *box1, TBOX *box2)
{
	if (! MOBDB_FLAGS_GET_T(box1->flags) || ! MOBDB_FLAGS_GET_T(box2->flags))
		elog(ERROR, "The boxes must have T dimension");

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

/*****************************************************************************
 * Set operators
 *****************************************************************************/

/* Union of two boxes */

TBOX *
tbox_union_internal(const TBOX *box1, const TBOX *box2)
{
	/* Operations on boxes of different dimensionality */
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
		elog(ERROR, "The boxes must have the same dimensions");
	/* The union of boxes that do not intersect cannot be represented by a box */
	if (! overlaps_tbox_tbox_internal(box1, box2))
		elog(ERROR, "Result of box union would not be contiguous");

	bool hasx = MOBDB_FLAGS_GET_X(box1->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags);
	TBOX *result = tbox_new(hasx, hast);
	if (hasx)
	{
		result->xmin = Min(box1->xmin, box2->xmin);
		result->xmax = Max(box1->xmax, box2->xmax);
	}
	if (hast)
	{
		result->tmin = Min(box1->tmin, box2->tmin);
		result->tmax = Max(box1->tmax, box2->tmax);
	}
	return(result);
}

PG_FUNCTION_INFO_V1(tbox_union);

PGDLLEXPORT Datum
tbox_union(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	TBOX *result = tbox_union_internal(box1, box2);
	PG_RETURN_POINTER(result);
}

/* Intersection of two boxes */

TBOX *
tbox_intersection_internal(const TBOX *box1, const TBOX *box2)
{
	bool hasx = MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags);
	bool hast = MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags);
	/* If there is no common dimension */
	if ((! hasx && ! hast) ||
	/* If they do no intersect in one common dimension */
	(hasx && (box1->xmin > box2->xmax || box2->xmin > box1->xmax)) ||
	(hast && (box1->tmin > box2->tmax || box2->tmin > box1->tmax)))
		return(NULL);

	TBOX *result = tbox_new(hasx, hast);
	if (hasx)
	{
		result->xmin = Max(box1->xmin, box2->xmin);
		result->xmax = Min(box1->xmax, box2->xmax);
	}
	if (hast)
	{
		result->tmin = Max(box1->tmin, box2->tmin);
		result->tmax = Min(box1->tmax, box2->tmax);
	}
	return(result);
}

PG_FUNCTION_INFO_V1(tbox_intersection);

PGDLLEXPORT Datum
tbox_intersection(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	TBOX *result = tbox_intersection_internal(box1, box2);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/*
 * Compare two boxes
 */
int 
tbox_cmp_internal(const TBOX *box1, const TBOX *box2)
{
	/* Compare the box minima */
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags))
	{
		if (box1->xmin < box2->xmin)
			return -1;
		if (box1->xmin > box2->xmin)
			return 1;
	}
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags))
	{
		if (box1->tmin < box2->tmin)
			return -1;
		if (box1->tmin > box2->tmin)
			return 1;
	}
	/* Compare the box maxima */
	if (MOBDB_FLAGS_GET_X(box1->flags) && MOBDB_FLAGS_GET_X(box2->flags))
	{
		if (box1->xmax < box2->xmax)
			return -1;
		if (box1->xmax > box2->xmax)
			return 1;
	}
	if (MOBDB_FLAGS_GET_T(box1->flags) && MOBDB_FLAGS_GET_T(box2->flags))
	{
		if (box1->tmax < box2->tmax)
			return -1;
		if (box1->tmax > box2->tmax)
			return 1;
	}
	/* The two boxes are equal */
	return 0;
}

PG_FUNCTION_INFO_V1(tbox_cmp);

PGDLLEXPORT Datum
tbox_cmp(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	int	cmp = tbox_cmp_internal(box1, box2);
	PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(tbox_lt);

PGDLLEXPORT Datum
tbox_lt(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	int	cmp = tbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(tbox_le);

PGDLLEXPORT Datum
tbox_le(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	int	cmp = tbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(tbox_ge);

PGDLLEXPORT Datum
tbox_ge(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	int	cmp = tbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(tbox_gt);

PGDLLEXPORT Datum
tbox_gt(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	int	cmp = tbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp > 0);
}

/*
 * Equality and inequality of two boxes
 */
bool
tbox_eq_internal(const TBOX *box1, const TBOX *box2)
{
	if (MOBDB_FLAGS_GET_X(box1->flags) != MOBDB_FLAGS_GET_X(box2->flags) ||
		MOBDB_FLAGS_GET_T(box1->flags) != MOBDB_FLAGS_GET_T(box2->flags))
			return false;
	if (box1->xmin != box2->xmin || box1->tmin != box2->tmin ||
		box1->xmax != box2->xmax || box1->tmax != box2->tmax)
		return false;
	/* The two boxes are equal */
	return true;
}

PG_FUNCTION_INFO_V1(tbox_eq);

PGDLLEXPORT Datum
tbox_eq(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(tbox_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(tbox_ne);

PGDLLEXPORT Datum
tbox_ne(PG_FUNCTION_ARGS)
{
	TBOX *box1 = PG_GETARG_TBOX_P(0);
	TBOX *box2 = PG_GETARG_TBOX_P(1);
	PG_RETURN_BOOL(! tbox_eq_internal(box1, box2));
}

/*****************************************************************************/

