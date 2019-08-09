/*****************************************************************************
 *
 * TemporalTypes.h
 *	  Functions for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __DOUBLEN_H__
#define __DOUBLEN_H__

#include <postgres.h>

/*****************************************************************************/

extern Datum double2_in(PG_FUNCTION_ARGS);
extern Datum double2_out(PG_FUNCTION_ARGS);
extern Datum double2_rcv(PG_FUNCTION_ARGS);
extern Datum double2_send(PG_FUNCTION_ARGS);

extern double2 *double2_construct(double a, double b);
extern double2 *double2_add(double2 *d1, double2 *d2);
extern bool double2_eq(double2 *d1, double2 *d2);
extern int double2_cmp(double2 *d1, double2 *d2);

extern Datum double3_in(PG_FUNCTION_ARGS);
extern Datum double3_out(PG_FUNCTION_ARGS);
extern Datum double3_rcv(PG_FUNCTION_ARGS);
extern Datum double3_send(PG_FUNCTION_ARGS);

extern double3 *double3_construct(double a, double b, double c);
extern double3 *double3_add(double3 *d1, double3 *d2);
extern bool double3_eq(double3 *d1, double3 *d2);
extern int double3_cmp(double3 *d1, double3 *d2);

extern Datum double4_in(PG_FUNCTION_ARGS);
extern Datum double4_out(PG_FUNCTION_ARGS);
extern Datum double4_rcv(PG_FUNCTION_ARGS);
extern Datum double4_send(PG_FUNCTION_ARGS);

extern double4 *double4_construct(double a, double b, double c, double d);
extern double4 *double4_add(double4 *d1, double4 *d2);
extern bool double4_eq(double4 *d1, double4 *d2);
extern int double4_cmp(double4 *d1, double4 *d2);

extern Datum tdouble2_in(PG_FUNCTION_ARGS);
extern Datum tdouble3_in(PG_FUNCTION_ARGS);
extern Datum tdouble4_in(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
