/*****************************************************************************
 *
 * tbox.h
 *	  Basic functions for TBOX bounding box.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TBOX_H__
#define __TBOX_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#if MOBDB_PGSQL_VERSION < 110000
#include <datatype/timestamp.h>
#endif

/*****************************************************************************/

typedef struct
{
	double		xmin;			/* minimum numeric value */
	double		xmax;			/* maximum numeric value */
	TimestampTz	tmin;			/* minimum timestamp */
	TimestampTz	tmax;			/* maximum timestamp */
	int16		flags;			/* flags */
} TBOX;

/* fmgr macros temporal types */

#define DatumGetTboxP(X)	((TBOX *) DatumGetPointer(X))
#define TboxPGetDatum(X)	PointerGetDatum(X)
#define PG_GETARG_TBOX_P(n) DatumGetTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_TBOX_P(x) return TboxPGetDatum(x)

extern void tbox_expand(TBOX *box1, const TBOX *box2);
extern void tbox_shift(TBOX *box, const Interval *interval);

extern Datum tbox_in(PG_FUNCTION_ARGS);
extern Datum tbox_out(PG_FUNCTION_ARGS);
extern Datum tbox_constructor(PG_FUNCTION_ARGS);

extern Datum overlaps_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum contained_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum contains_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum same_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum adjacent_tbox_tbox(PG_FUNCTION_ARGS);

extern bool overlaps_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool contained_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool contains_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool same_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool adjacent_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);

extern Datum left_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overleft_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum right_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overright_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum before_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum after_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overafter_tbox_tbox(PG_FUNCTION_ARGS);

extern bool left_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool overleft_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool right_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool overright_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool before_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool overbefore_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool after_tbox_tbox_internal(TBOX *box1, TBOX *box2);
extern bool overafter_tbox_tbox_internal(TBOX *box1, TBOX *box2);

extern Datum tbox_union(PG_FUNCTION_ARGS);
extern Datum tbox_minus(PG_FUNCTION_ARGS);
extern Datum tbox_intersection(PG_FUNCTION_ARGS);

extern TBOX *tbox_union_internal(const TBOX *box1, const TBOX *box2);
extern TBOX *tbox_minus_internal(const TBOX *box1, const TBOX *box2);
extern TBOX *tbox_intersection_internal(const TBOX *box1, const TBOX *box2);

extern Datum tbox_lt(PG_FUNCTION_ARGS);
extern Datum tbox_le(PG_FUNCTION_ARGS);
extern Datum tbox_gt(PG_FUNCTION_ARGS);
extern Datum tbox_ge(PG_FUNCTION_ARGS);
extern Datum tbox_eq(PG_FUNCTION_ARGS);
extern Datum tbox_ne(PG_FUNCTION_ARGS);
extern Datum tbox_cmp(PG_FUNCTION_ARGS);

extern int tbox_cmp_internal(const TBOX *box1, const TBOX *box2);
extern bool tbox_eq_internal(const TBOX *box1, const TBOX *box2);

/*****************************************************************************/

#endif
