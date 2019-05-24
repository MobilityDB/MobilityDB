/*****************************************************************************
 *
 * Gbox.c
 *	  Basic functions for GBOX bounding box.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* 
 * Input function. 
 * Examples of input:
 * 		GBOX((1.0, 2.0), (1.0, 2.0))
 * 		GBOX Z((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))
 * 		GBOX M((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))
 * 		GBOX ZM((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))
 * 		GEODBOX((1.0, 2.0, 3.0), (1.0, 2.0, 3.0))
 * 		GEODBOX M((1.0, 2.0, 3.0, 4.0), (1.0, 2.0, 3.0, 4.0))
 */
PG_FUNCTION_INFO_V1(gbox_in);

PGDLLEXPORT Datum
gbox_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	GBOX *result = gbox_parse(&input);
	PG_RETURN_POINTER(result);
}

char* gbox_to_string_mdb(const GBOX *gbox)
{
	static int sz = 256;
	char *str = NULL;
	str = (char *)palloc(sz);
	if ( FLAGS_GET_GEODETIC(gbox->flags) )
	{
		if (FLAGS_GET_M(gbox->flags))
			snprintf(str, sz, "GEODBOX M((%.8g,%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g,%.8g))", 
				gbox->xmin, gbox->ymin, gbox->zmin, gbox->mmin, 
				gbox->xmax, gbox->ymax, gbox->zmax, gbox->mmax);
		else
			snprintf(str, sz, "GEODBOX((%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g))", 
				gbox->xmin, gbox->ymin, gbox->zmin, 
				gbox->xmax, gbox->ymax, gbox->zmax);
		return str;
	}
	if ( FLAGS_GET_Z(gbox->flags) && FLAGS_GET_M(gbox->flags) )
	{
		snprintf(str, sz, "GBOX ZM((%.8g,%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g,%.8g))", 
			gbox->xmin, gbox->ymin, gbox->zmin, gbox->mmin, 
			gbox->xmax, gbox->ymax, gbox->zmax, gbox->mmax);
		return str;
	}
	if ( FLAGS_GET_Z(gbox->flags) )
	{
		snprintf(str, sz, "GBOX Z((%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g))", 
			gbox->xmin, gbox->ymin, gbox->zmin, 
			gbox->xmax, gbox->ymax, gbox->zmax);
		return str;
	}
	if ( FLAGS_GET_M(gbox->flags) )
	{
		snprintf(str, sz, "GBOX M((%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g))", 
			gbox->xmin, gbox->ymin, gbox->mmin, gbox->xmax, gbox->ymax, gbox->mmax);
		return str;
	}
	snprintf(str, sz, "GBOX((%.8g,%.8g),(%.8g,%.8g))", 
		gbox->xmin, gbox->ymin, gbox->xmax, gbox->ymax);
	return str;
}

/* 
 * Output function. 
 */
PG_FUNCTION_INFO_V1(gbox_out);

PGDLLEXPORT Datum
gbox_out(PG_FUNCTION_ARGS)
{
	GBOX *box = PG_GETARG_GBOX_P(0);
	char *result = gbox_to_string_mdb(box);
	PG_RETURN_CSTRING(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gbox_constructor);

PGDLLEXPORT Datum
gbox_constructor(PG_FUNCTION_ARGS)
{
	assert(PG_NARGS() == 4 || PG_NARGS() == 6 || PG_NARGS() == 8);
	double zmin = 0, zmax = 0, mmin = 0, mmax = 0, /* keep compiler quiet */
		tmp;
	int hasz = 0, hasm = 0;

	double xmin = PG_GETARG_FLOAT8(0);
	double xmax = PG_GETARG_FLOAT8(1);
	double ymin = PG_GETARG_FLOAT8(2);
	double ymax = PG_GETARG_FLOAT8(3);

	if (PG_NARGS() == 4)
		;
	else if (PG_NARGS() == 6)
	{
		zmin = PG_GETARG_FLOAT8(4);
		zmax = PG_GETARG_FLOAT8(5);
		hasz = 1;
	}
	else if (PG_NARGS() == 8)
	{
		zmin = PG_GETARG_FLOAT8(4);
		zmax = PG_GETARG_FLOAT8(5);
		mmin = PG_GETARG_FLOAT8(6);
		mmax = PG_GETARG_FLOAT8(7);
		hasz = 1;
		hasm = 1;
	}

	GBOX *result = gbox_new(gflags(hasz, hasm, 0));
	
	/* Process X min/max */
	if (xmin > xmax)
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if (ymin > ymax)
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process Z min/max */
	if (hasz != 0)
	{
		if (zmin > zmax)
		{
			tmp = zmin;
			zmin = zmax;
			zmax = tmp;
		}
		result->zmin = zmin;
		result->zmax = zmax;
		FLAGS_SET_Z(result->flags, true);
	}

	/* Process M min/max */
	if (hasm != 0)
	{
		if ( hasm != 0 && mmin > mmax )
		{
			tmp = mmin;
			mmin = mmax;
			mmax = tmp;
		}
		result->mmin = mmin;
		result->mmax = mmax;
		FLAGS_SET_M(result->flags, true);
	}

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(gbox3dm_constructor);

PGDLLEXPORT Datum
gbox3dm_constructor(PG_FUNCTION_ARGS)
{
	double xmin, xmax, ymin, ymax, 
		mmin, mmax, tmp;

	xmin = PG_GETARG_FLOAT8(0);
	xmax = PG_GETARG_FLOAT8(1);
	ymin = PG_GETARG_FLOAT8(2);
	ymax = PG_GETARG_FLOAT8(3);
	mmin = PG_GETARG_FLOAT8(4);
	mmax = PG_GETARG_FLOAT8(5);

	GBOX *result = gbox_new(gflags(0, 1, 0));
	
	/* Process X min/max */
	if (xmin > xmax)
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if (ymin > ymax)
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process M min/max */
	if (mmin > mmax)
	{
		tmp = mmin;
		mmin = mmax;
		mmax = tmp;
	}
	result->mmin = mmin;
	result->mmax = mmax;
	FLAGS_SET_M(result->flags, true);

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geodbox_constructor);

PGDLLEXPORT Datum
geodbox_constructor(PG_FUNCTION_ARGS)
{
	assert(PG_NARGS() == 6 || PG_NARGS() == 8);
	double mmin, mmax, tmp;
	int hasm = 0;

	double xmin = PG_GETARG_FLOAT8(0);
	double xmax = PG_GETARG_FLOAT8(1);
	double ymin = PG_GETARG_FLOAT8(2);
	double ymax = PG_GETARG_FLOAT8(3);
	double zmin = PG_GETARG_FLOAT8(4);
	double zmax = PG_GETARG_FLOAT8(5);
	if (PG_NARGS() == 6)
		;
	else if (PG_NARGS() == 8)
	{
		mmin = PG_GETARG_FLOAT8(6);
		mmax = PG_GETARG_FLOAT8(7);
		hasm = 1;
	}

	GBOX *result = gbox_new(gflags(1, hasm, 1));
	
	/* Process X min/max */
	if (xmin > xmax)
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if (ymin > ymax)
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process Z min/max */
	if (zmin > zmax)
	{
		tmp = zmin;
		zmin = zmax;
		zmax = tmp;
	}
	result->zmin = zmin;
	result->zmax = zmax;
	FLAGS_SET_Z(result->flags, true);

	if (hasm)
	{
		/* Process M min/max */
		if ( mmin > mmax )
		{
			tmp = mmin;
			mmin = mmax;
			mmax = tmp;
		}
		result->mmin = mmin;
		result->mmax = mmax;
		FLAGS_SET_M(result->flags, true);
	}


	PG_RETURN_POINTER(result);
}

/*
 * Compare two boxes
 */
int 
gbox_cmp_internal(const GBOX *g1, const GBOX *g2)
{
	/* Compare the box minima */
	if (g1->xmin < g2->xmin)
		return -1;
	if (g1->xmin > g2->xmin)
		return 1;
	if (g1->ymin < g2->ymin)
		return -1;
	if (g1->ymin > g2->ymin)
		return 1;
	if ( FLAGS_GET_Z(g1->flags) && FLAGS_GET_Z(g2->flags) )
	{
		if (g1->zmin < g2->zmin)
			return -1;
		if (g1->zmin > g2->zmin)
			return 1;
	}
	if ( FLAGS_GET_M(g1->flags) && FLAGS_GET_M(g2->flags) )
	{
		if (g1->mmin < g2->mmin)
			return -1;
		if (g1->mmin > g2->mmin)
			return 1;
	}
	/* Compare the box maxima */
	if (g1->xmax < g2->xmax)
		return -1;
	if (g1->xmax > g2->xmax)
		return 1;
	if (g1->ymax < g2->ymax)
		return -1;
	if (g1->ymax > g2->ymax)
		return 1;
	if ( FLAGS_GET_Z(g1->flags) && FLAGS_GET_Z(g2->flags) )
	{
		if (g1->zmax < g2->zmax)
			return -1;
		if (g1->zmax > g2->zmax)
			return 1;
	}
	if ( FLAGS_GET_M(g1->flags) && FLAGS_GET_M(g2->flags) )
	{
		if (g1->mmax < g2->mmax)
			return -1;
		if (g1->mmax > g2->mmax)
			return 1;
	}
	/* The two boxes are equal */
	return 0;
}

PG_FUNCTION_INFO_V1(gbox_cmp);

PGDLLEXPORT Datum
gbox_cmp(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int	cmp = gbox_cmp_internal(box1, box2);
	PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(gbox_lt);

PGDLLEXPORT Datum
gbox_lt(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int	cmp = gbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(gbox_le);

PGDLLEXPORT Datum
gbox_le(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int	cmp = gbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(gbox_ge);

PGDLLEXPORT Datum
gbox_ge(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int	cmp = gbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(gbox_gt);

PGDLLEXPORT Datum
gbox_gt(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int	cmp = gbox_cmp_internal(box1, box2);
	PG_RETURN_BOOL(cmp > 0);
}

/*
 * Equality and inequality of two boxes
 */
bool
gbox_eq_internal(const GBOX *box1, const GBOX *box2)
{
	if (box1->xmin != box2->xmin || box1->ymin != box2->ymin ||
		box1->zmin != box2->zmin || box1->mmin != box2->mmin ||
		box1->xmax != box2->xmax || box1->ymax != box2->ymax ||
		box1->zmax != box2->zmax || box1->mmax != box2->mmax)
		return false;
	/* The two boxes are equal */
	return true;
}

PG_FUNCTION_INFO_V1(gbox_eq);

PGDLLEXPORT Datum
gbox_eq(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	PG_RETURN_BOOL(gbox_eq_internal(box1, box2));
}

PG_FUNCTION_INFO_V1(gbox_ne);

PGDLLEXPORT Datum
gbox_ne(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	PG_RETURN_BOOL(! gbox_eq_internal(box1, box2));
}

/*****************************************************************************/

