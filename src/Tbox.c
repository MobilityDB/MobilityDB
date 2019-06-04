/*****************************************************************************
 *
 * Tbox.c
 *	  Basic functions for TBOX bounding box.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

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

char* tbox_to_string(const TBOX *box)
{
	static int sz = 128;
	char *str = NULL;
	str = (char *)palloc(sz);
	assert(MOBDB_FLAGS_GET_X(box->flags) || MOBDB_FLAGS_GET_T(box->flags));
	if (MOBDB_FLAGS_GET_X(box->flags))
	{
		if (MOBDB_FLAGS_GET_T(box->flags))
			snprintf(str, sz, "TBOX((%.8g,%.8g),(%.8g,%.8g))", 
				box->xmin, box->tmin, box->xmax, box->tmax);
		else 
			snprintf(str, sz, "TBOX((%.8g,),(%.8g,))", 
				box->xmin,box->xmax);
	}
	else
		/* Missing X dimension */
		snprintf(str, sz, "TBOX((,%.8g),(,%.8g))", 
			box->tmin, box->tmax);
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
	assert (PG_NARGS() == 2 || PG_NARGS() == 4);
	double xmin = 0, xmax = 0, /* keep compiler quiet */
		tmin, tmax, tmp;
	bool hast = false;

	if (PG_NARGS() == 2)
	{
		xmin = PG_GETARG_FLOAT8(0);
		xmax = PG_GETARG_FLOAT8(1);
	}
	else if (PG_NARGS() == 4)
	{
		xmin = PG_GETARG_FLOAT8(0);
		tmin = PG_GETARG_FLOAT8(1);
		xmax = PG_GETARG_FLOAT8(2);
		tmax = PG_GETARG_FLOAT8(3);
		hast = true;
	}

	TBOX *result = palloc0(sizeof(TBOX));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, hast);
	
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
			tmp = tmin;
			tmin = tmax;
			tmax = tmp;
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
	double tmin, tmax, tmp;
	tmin = PG_GETARG_FLOAT8(0);
	tmax = PG_GETARG_FLOAT8(1);

	TBOX *result = palloc0(sizeof(TBOX));
	MOBDB_FLAGS_SET_X(result->flags, false);
	MOBDB_FLAGS_SET_T(result->flags, true);
	
	/* Process T min/max */
	if (tmin > tmax)
	{
		tmp = tmin;
		tmin = tmax;
		tmax = tmp;
	}
	result->tmin = tmin;
	result->tmax = tmax;
	PG_RETURN_POINTER(result);
}

/*
 * Compare two boxes
 */
int 
tbox_cmp_internal(const TBOX *box1, const TBOX *box2)
{
	/* Compare the box minima */
	if (box1->xmin < box2->xmin)
		return -1;
	if (box1->xmin > box2->xmin)
		return 1;
	if (box1->tmin < box2->tmin)
		return -1;
	if (box1->tmin > box2->tmin)
		return 1;
	/* Compare the box maxima */
	if (box1->xmax < box2->xmax)
		return -1;
	if (box1->xmax > box2->xmax)
		return 1;
	if (box1->tmax < box2->tmax)
		return -1;
	if (box1->tmax > box2->tmax)
		return 1;
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

