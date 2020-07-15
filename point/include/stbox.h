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

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#if MOBDB_PGSQL_VERSION < 110000
#include <utils/timestamp.h>
#endif
#include <liblwgeom.h>

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
extern Datum stbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum stbox_constructor_x(PG_FUNCTION_ARGS);
extern Datum stbox_constructor_xz(PG_FUNCTION_ARGS);
extern Datum stbox_constructor_xzt(PG_FUNCTION_ARGS);
extern Datum stboxt_constructor_xt(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_t(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_xz(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor_xzt(PG_FUNCTION_ARGS);
extern Datum stbox_intersection(PG_FUNCTION_ARGS);

extern STBOX *stbox_new(bool hasx, bool hasz, bool hast, bool geodetic, int32 srid);
extern STBOX *stbox_copy(const STBOX *box);
extern void stbox_expand(STBOX *box1, const STBOX *box2);
extern void stbox_shift(STBOX *box, const Interval *interval);

extern Datum stbox_to_period(PG_FUNCTION_ARGS);
extern Datum stbox_to_box2d(PG_FUNCTION_ARGS);
extern Datum stbox_to_box3d(PG_FUNCTION_ARGS);

extern GBOX *stbox_to_gbox(const STBOX *box);

extern Datum stbox_xmin(PG_FUNCTION_ARGS);
extern Datum stbox_xmax(PG_FUNCTION_ARGS);
extern Datum stbox_ymin(PG_FUNCTION_ARGS);
extern Datum stbox_ymax(PG_FUNCTION_ARGS);
extern Datum stbox_zmin(PG_FUNCTION_ARGS);
extern Datum stbox_zmax(PG_FUNCTION_ARGS);
extern Datum stbox_tmin(PG_FUNCTION_ARGS);
extern Datum stbox_tmax(PG_FUNCTION_ARGS);
extern Datum stbox_srid(PG_FUNCTION_ARGS);
extern Datum stbox_set_srid(PG_FUNCTION_ARGS);
extern Datum stbox_transform(PG_FUNCTION_ARGS);
extern Datum stbox_set_precision(PG_FUNCTION_ARGS);

extern Datum contains_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum contained_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overlaps_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum same_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum adjacent_stbox_stbox(PG_FUNCTION_ARGS);

extern bool contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool adjacent_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);

extern Datum left_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overleft_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum right_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overright_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum below_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbelow_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum above_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overabove_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum front_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overfront_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum back_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overback_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum before_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overbefore_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum after_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overafter_stbox_stbox(PG_FUNCTION_ARGS);

extern bool left_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overleft_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool right_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overright_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool below_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overbelow_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool above_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overabove_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool front_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overfront_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool back_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overback_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool before_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overbefore_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool after_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overafter_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);

extern Datum stbox_union(PG_FUNCTION_ARGS);
extern Datum stbox_intersection(PG_FUNCTION_ARGS);

extern STBOX *stbox_union_internal(const STBOX *box1, const STBOX *box2);
extern STBOX *stbox_intersection_internal(const STBOX *box1, const STBOX *box2);

extern Datum stbox_cmp(PG_FUNCTION_ARGS);
extern Datum stbox_eq(PG_FUNCTION_ARGS);
extern Datum stbox_ne(PG_FUNCTION_ARGS);
extern Datum stbox_lt(PG_FUNCTION_ARGS);
extern Datum stbox_le(PG_FUNCTION_ARGS);
extern Datum stbox_gt(PG_FUNCTION_ARGS);
extern Datum stbox_ge(PG_FUNCTION_ARGS);

extern int stbox_cmp_internal(const STBOX *box1, const STBOX *box2);
extern bool stbox_eq_internal(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
