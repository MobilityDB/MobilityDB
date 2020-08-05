/*****************************************************************************
 *
 * tbox.h
 *	  Functions for temporal bounding boxes.
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
#include <utils/rangetypes.h>
#if MOBDB_PGSQL_VERSION < 110000
#include <utils/timestamp.h>
#endif

#include "timetypes.h"

/*****************************************************************************/

/**
 * Structure to represent temporal boxes
 */
typedef struct
{
	double		xmin;			/**< minimum numeric value */
	double		xmax;			/**< maximum numeric value */
	TimestampTz	tmin;			/**< minimum timestamp */
	TimestampTz	tmax;			/**< maximum timestamp */
	int16		flags;			/**< flags */
} TBOX;

/* fmgr macros temporal types */

#define DatumGetTboxP(X)	((TBOX *) DatumGetPointer(X))
#define TboxPGetDatum(X)	PointerGetDatum(X)
#define PG_GETARG_TBOX_P(n) DatumGetTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_TBOX_P(x) return TboxPGetDatum(x)

/* Miscellaneous functions */

extern TBOX *tbox_copy(const TBOX *box);
extern void tbox_expand(TBOX *box1, const TBOX *box2);
extern void tbox_shift(TBOX *box, const Interval *interval);

/* Parameter tests */

extern void ensure_has_X_tbox(const TBOX *box);
extern void ensure_has_T_tbox(const TBOX *box);
extern void ensure_same_dimensionality_tbox(const TBOX *box1, const TBOX *box2);
extern void ensure_common_dimension_tbox(const TBOX *box1, const TBOX *box2);

/* Input/output functions */

extern Datum tbox_in(PG_FUNCTION_ARGS);
extern Datum tbox_out(PG_FUNCTION_ARGS);

/* Constructor functions */

extern Datum tbox_constructor(PG_FUNCTION_ARGS);
extern Datum tbox_constructor_t(PG_FUNCTION_ARGS);

/* Casting */

extern Datum tbox_to_floatrange(PG_FUNCTION_ARGS);
extern Datum tbox_to_period(PG_FUNCTION_ARGS);

/* Transform a type to a TBOX */

extern Datum int_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_to_tbox(PG_FUNCTION_ARGS);
extern Datum numeric_to_tbox(PG_FUNCTION_ARGS);
extern Datum range_to_tbox(PG_FUNCTION_ARGS);
extern Datum timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum period_to_tbox(PG_FUNCTION_ARGS);
extern Datum timestampset_to_tbox(PG_FUNCTION_ARGS);
extern Datum periodset_to_tbox(PG_FUNCTION_ARGS);
extern Datum int_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum int_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum range_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum range_period_to_tbox(PG_FUNCTION_ARGS);

extern void number_to_box(TBOX *box, Datum value, Oid valuetypid);
extern void range_to_tbox_internal(TBOX *box, RangeType *r);
extern void int_to_tbox_internal(TBOX *box, int i);
extern void float_to_tbox_internal(TBOX *box, double d);
extern void intrange_to_tbox(TBOX *box, RangeType *range);
extern void floatrange_to_tbox(TBOX *box, RangeType *range);
extern void timestamp_to_tbox_internal(TBOX *box, TimestampTz t);
extern void timestampset_to_tbox_internal(TBOX *box, const TimestampSet *ts);
extern void period_to_tbox_internal(TBOX *box, const Period *p);
extern void periodset_to_tbox_internal(TBOX *box, const PeriodSet *ps);

/* Accessor functions */

extern Datum tbox_xmin(PG_FUNCTION_ARGS);
extern Datum tbox_xmax(PG_FUNCTION_ARGS);
extern Datum tbox_tmin(PG_FUNCTION_ARGS);
extern Datum tbox_tmax(PG_FUNCTION_ARGS);

/* Casting */

extern Datum tbox_expand_value(PG_FUNCTION_ARGS);
extern Datum tbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum tbox_set_precision(PG_FUNCTION_ARGS);

/* Topological functions */

extern Datum contains_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum contained_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overlaps_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum same_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum adjacent_tbox_tbox(PG_FUNCTION_ARGS);

extern bool contains_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool contained_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool overlaps_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool same_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool adjacent_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);

/* Position functions */

extern Datum left_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overleft_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum right_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overright_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum before_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum after_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum overafter_tbox_tbox(PG_FUNCTION_ARGS);

extern bool left_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool overleft_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool right_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool overright_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool before_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool overbefore_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool after_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);
extern bool overafter_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);

/* Set functions */

extern Datum tbox_union(PG_FUNCTION_ARGS);
extern Datum tbox_intersection(PG_FUNCTION_ARGS);

extern TBOX *tbox_union_internal(const TBOX *box1, const TBOX *box2);
extern TBOX *tbox_minus_internal(const TBOX *box1, const TBOX *box2);
extern TBOX *tbox_intersection_internal(const TBOX *box1, const TBOX *box2);

/* Comparison functions */

extern Datum tbox_cmp(PG_FUNCTION_ARGS);
extern Datum tbox_lt(PG_FUNCTION_ARGS);
extern Datum tbox_le(PG_FUNCTION_ARGS);
extern Datum tbox_gt(PG_FUNCTION_ARGS);
extern Datum tbox_ge(PG_FUNCTION_ARGS);
extern Datum tbox_eq(PG_FUNCTION_ARGS);
extern Datum tbox_ne(PG_FUNCTION_ARGS);

extern int tbox_cmp_internal(const TBOX *box1, const TBOX *box2);
extern bool tbox_eq_internal(const TBOX *box1, const TBOX *box2);

/*****************************************************************************/

#endif
