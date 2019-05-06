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
 */
PG_FUNCTION_INFO_V1(gbox_in);

PGDLLEXPORT Datum
gbox_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	GBOX *result = gbox_parse(&input);
	if (result == 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
			errmsg("Could not parse gbox")));
	PG_RETURN_POINTER(result);
}

char* gbox_to_string_mdb(const GBOX *gbox)
{
	static int sz = 256;
	char *str = NULL;

	if ( ! gbox )
		return strdup("NULL POINTER");

	str = (char *)palloc(sz);

	if ( FLAGS_GET_GEODETIC(gbox->flags) )
	{
		if (FLAGS_GET_M(gbox->flags))
			snprintf(str, sz, "GEODBOX((%.8g,%.8g,%.8g,%.8g),(%.8g,%.8g,%.8g,%.8g))", 
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
	double xmin, xmax, ymin, ymax, 
		zmin = 0, zmax = 0, mmin = 0, mmax = 0, /* keep compiler quiet */
		tmp;
	int hasz = 0, hasm = 0;

	xmin = PG_GETARG_FLOAT8(0);
	xmax = PG_GETARG_FLOAT8(1);
	ymin = PG_GETARG_FLOAT8(2);
	ymax = PG_GETARG_FLOAT8(3);

	if ( PG_NARGS() == 4 )
		;
	else if ( PG_NARGS() == 6 )
	{
		zmin = PG_GETARG_FLOAT8(4);
		zmax = PG_GETARG_FLOAT8(5);
		hasz = 1;
	}
	else if ( PG_NARGS() == 8 )
	{
		zmin = PG_GETARG_FLOAT8(4);
		zmax = PG_GETARG_FLOAT8(5);
		mmin = PG_GETARG_FLOAT8(6);
		mmax = PG_GETARG_FLOAT8(7);
		hasz = 1;
		hasm = 1;
	}
	else
	{
		elog(ERROR, "Unsupported number of args: %d", PG_NARGS());
		PG_RETURN_NULL();
	}

	GBOX *result = gbox_new(gflags(hasz, hasm, 0));
	
	/* Process X min/max */
	if ( xmin > xmax )
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if ( ymin > ymax )
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process Z min/max */
	if ( hasz != 0 && zmin > zmax )
	{
		tmp = zmin;
		zmin = zmax;
		zmax = tmp;
	}
	result->zmin = zmin;
	result->zmax = zmax;

	/* Process M min/max */
	if ( hasm != 0 && mmin > mmax )
	{
		tmp = mmin;
		mmin = mmax;
		mmax = tmp;
	}
	result->mmin = mmin;
	result->mmax = mmax;

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
	if ( xmin > xmax )
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if ( ymin > ymax )
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process M min/max */
	if ( mmin > mmax )
	{
		tmp = mmin;
		mmin = mmax;
		mmax = tmp;
	}
	result->mmin = mmin;
	result->mmax = mmax;

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(geodbox_constructor);

PGDLLEXPORT Datum
geodbox_constructor(PG_FUNCTION_ARGS)
{
	double xmin, xmax, ymin, ymax, zmin, zmax, tmp;

	xmin = PG_GETARG_FLOAT8(0);
	xmax = PG_GETARG_FLOAT8(1);
	ymin = PG_GETARG_FLOAT8(2);
	ymax = PG_GETARG_FLOAT8(3);
	zmin = PG_GETARG_FLOAT8(4);
	zmax = PG_GETARG_FLOAT8(5);

	GBOX *result = gbox_new(gflags(1, 0, 1));
	
	/* Process X min/max */
	if ( xmin > xmax )
	{
		tmp = xmin;
		xmin = xmax;
		xmax = tmp;
	}
	result->xmin = xmin;
	result->xmax = xmax;

	/* Process Y min/max */
	if ( ymin > ymax )
	{
		tmp = ymin;
		ymin = ymax;
		ymax = tmp;
	}
	result->ymin = ymin;
	result->ymax = ymax;

	/* Process Z min/max */
	if ( zmin > zmax )
	{
		tmp = zmin;
		zmin = zmax;
		zmax = tmp;
	}
	result->zmin = zmin;
	result->zmax = zmax;

	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/*
 * Does the first box contains the second one?
 */
int 
gbox_contains(const GBOX *g1, const GBOX *g2)
{
	/* Make sure our boxes are consistent */
	if ( FLAGS_GET_GEODETIC(g1->flags) != FLAGS_GET_GEODETIC(g2->flags) )
		elog(ERROR, "gbox_contains: cannot compare geodetic and non-geodetic boxes");

	/* Check X/Y first */
	if ( ( g2->xmin < g1->xmin ) || ( g2->xmax > g1->xmax ) ||
		 ( g2->ymin < g1->ymin ) || ( g2->ymax > g1->ymax ) )
		return LW_FALSE;

	/* Deal with the geodetic case special: we only compare the geodetic boxes (x/y/z) */
	/* Never the M dimension */
	if ( FLAGS_GET_GEODETIC(g1->flags) && FLAGS_GET_GEODETIC(g2->flags) )
	{
		if ( g2->zmin < g1->zmin || g2->zmax > g1->zmax )
			return LW_FALSE;
		else
			return LW_TRUE;
	}

	/* If both geodetic or both have Z, check Z */
	if ( FLAGS_GET_Z(g1->flags) && FLAGS_GET_Z(g2->flags) )
	{
		if ( g2->zmin < g1->zmin || g2->zmax > g1->zmax )
			return LW_FALSE;
	}

	/* If both have M, check M */
	if ( FLAGS_GET_M(g1->flags) && FLAGS_GET_M(g2->flags) )
	{
		if ( g2->mmin < g1->mmin || g2->mmax > g1->mmax )
			return LW_FALSE;
	}

	return LW_TRUE;
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

PG_FUNCTION_INFO_V1(gbox_eq);

PGDLLEXPORT Datum
gbox_eq(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int cmp = gbox_cmp_internal(box1, box2);
	if (cmp == 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

PG_FUNCTION_INFO_V1(gbox_ne);

PGDLLEXPORT Datum
gbox_ne(PG_FUNCTION_ARGS)
{
	GBOX *box1 = PG_GETARG_GBOX_P(0);
	GBOX *box2 = PG_GETARG_GBOX_P(1);
	int cmp = gbox_cmp_internal(box1, box2);
	if (cmp != 0)
		PG_RETURN_BOOL(true);
	else
		PG_RETURN_BOOL(false);
}

/*****************************************************************************/

