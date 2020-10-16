/*****************************************************************************
 *
 * doublen.h
 *  Internal types used for the average and centroid temporal aggregates. 
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __DOUBLEN_H__
#define __DOUBLEN_H__

#include <postgres.h>
#include <catalog/pg_type.h>

#include "temporal.h"

/*****************************************************************************/

extern Datum double2_in(PG_FUNCTION_ARGS);
extern Datum double2_out(PG_FUNCTION_ARGS);
extern Datum double2_recv(PG_FUNCTION_ARGS);
extern Datum double2_send(PG_FUNCTION_ARGS);

extern void double2_set(double2 *result, double a, double b);
extern double2 *double2_add(double2 *d1, double2 *d2);
extern bool double2_eq(double2 *d1, double2 *d2);

extern Datum double3_in(PG_FUNCTION_ARGS);
extern Datum double3_out(PG_FUNCTION_ARGS);
extern Datum double3_recv(PG_FUNCTION_ARGS);
extern Datum double3_send(PG_FUNCTION_ARGS);

extern void double3_set(double3 *result, double a, double b, double c);
extern double3 *double3_add(double3 *d1, double3 *d2);
extern bool double3_eq(double3 *d1, double3 *d2);

extern Datum double4_in(PG_FUNCTION_ARGS);
extern Datum double4_out(PG_FUNCTION_ARGS);
extern Datum double4_recv(PG_FUNCTION_ARGS);
extern Datum double4_send(PG_FUNCTION_ARGS);

extern void double4_set(double4 *result, double a, double b, double c, double d);
extern double4 *double4_add(double4 *d1, double4 *d2);
extern bool double4_eq(double4 *d1, double4 *d2);

extern Datum tdouble2_in(PG_FUNCTION_ARGS);
extern Datum tdouble3_in(PG_FUNCTION_ARGS);
extern Datum tdouble4_in(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
