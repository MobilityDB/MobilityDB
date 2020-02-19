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
#include "temporal.h"

/*****************************************************************************/

extern Datum tbox_in(PG_FUNCTION_ARGS);
extern Datum tbox_out(PG_FUNCTION_ARGS);
extern Datum tbox_constructor(PG_FUNCTION_ARGS);

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

/*****************************************************************************/

#endif
