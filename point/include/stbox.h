/*****************************************************************************
 *
 * stbox.h
 *	  Basic functions for STBOX bounding box.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __STBOX_H__
#define __STBOX_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************
 * Struct definition
 *****************************************************************************/

typedef struct
{
	double		xmin;			/* minimum x value */
	double		xmax;			/* maximum x value */
	double		ymin;			/* minimum y value */
	double		ymax;			/* maximum y value */
	double		zmin;			/* minimum z value */
	double		zmax;			/* maximum z value */
	TimestampTz	tmin;			/* minimum timestamp */
	TimestampTz	tmax;			/* maximum timestamp */
	int32		srid;			/* SRID */
	int16		flags;			/* flags */
} STBOX;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetSTboxP(X)    ((STBOX *) DatumGetPointer(X))
#define STboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_STBOX_P(n) DatumGetSTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_STBOX_P(x) return STboxPGetDatum(x)

/*****************************************************************************/

extern Datum stbox_in(PG_FUNCTION_ARGS);
extern Datum stbox_out(PG_FUNCTION_ARGS);
extern Datum stbox_constructor(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor(PG_FUNCTION_ARGS);
extern Datum stbox_intersection(PG_FUNCTION_ARGS);

extern STBOX *stbox_new(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid);
extern STBOX *stbox_copy(const STBOX *box);
extern void stbox_expand(STBOX *box1, const STBOX *box2);
extern void stbox_shift(STBOX *box, const Interval *interval);
extern STBOX *stbox_intersection_internal(const STBOX *box1, const STBOX *box2);
extern int stbox_cmp_internal(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
