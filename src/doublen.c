/*****************************************************************************
 *
 * doublen.c
 *	Internal types used for the average and centroid temporal aggregates. 
 *
 * The double2, double3, and double4 types are composed, respectively, of two, 
 * three, and four double values. The tdouble2, tdouble3, and tdouble4 types 
 * are the corresponding temporal types. The in/out functions of all these
 * types are stubs, as all access should be internal.
 * These types are needed for the transition function of the aggregates,   
 * where the first components of the doubleN values store the sum and the  
 * last one stores the count of the values. The final function computes the 
 * average from the doubleN values.
 * from the doubleN values.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "doublen.h"

#include <libpq/pqformat.h>
#include <utils/builtins.h>

/*****************************************************************************
 * Input/Output functions
 * Although doubleN are internal types, the doubleN_out function are 
 * implemented for debugging purposes.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double2_in);

PGDLLEXPORT Datum
double2_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("function double2_in not implemented")));
	PG_RETURN_POINTER(NULL);
}

/* Output function */
 
PG_FUNCTION_INFO_V1(double2_out);

PGDLLEXPORT Datum
double2_out(PG_FUNCTION_ARGS)
{
	double2 *d = (double2 *) PG_GETARG_POINTER(0);
	char *result;

	result = psprintf("(%g,%g)", d->a, d->b);
	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double2_recv);

PGDLLEXPORT Datum
double2_recv(PG_FUNCTION_ARGS) 
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	double2* result = palloc(sizeof(double2));
	const char* bytes = pq_getmsgbytes(buf, sizeof(double2));
	memcpy(result, bytes, sizeof(double2));
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double2_send);

PGDLLEXPORT Datum
double2_send(PG_FUNCTION_ARGS) 
{
	double2* d = (double2*) PG_GETARG_POINTER(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendbytes(&buf, (void*) d, sizeof(double2));
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/* Constructor */

double2 *
double2_construct(double a, double b)
{
	double2 *result;
	result = (double2 *)palloc(sizeof(double2));
	result->a = a;
	result->b = b;
	return result;
}

/* Addition */

double2*
double2_add(double2 *d1, double2 *d2)
{
	double2 *result = (double2 *) palloc(sizeof(double2));
	result->a = d1->a + d2->a;
	result->b = d1->b + d2->b;
	return result;
}

/* Equality */

bool
double2_eq(double2 *d1, double2 *d2)
{
	return (d1->a == d2->a && d1->b == d2->b);
}

/* Comparator */
int
double2_cmp(double2 *d1, double2 *d2)
{
	int cmp = float8_cmp_internal(d1->a, d2->a);
	if (cmp == 0)
		cmp = float8_cmp_internal(d1->b, d2->b);
	return cmp;
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double3_in);

PGDLLEXPORT Datum
double3_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("function double3_in not implemented")));
	PG_RETURN_POINTER(NULL);
}

/* Output function */
 
PG_FUNCTION_INFO_V1(double3_out);

PGDLLEXPORT Datum
double3_out(PG_FUNCTION_ARGS)
{
	double3 *d = (double3 *) PG_GETARG_POINTER(0);
	char *result;

	result = psprintf("(%g,%g,%g)", d->a, d->b, d->c);
	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double3_recv);

PGDLLEXPORT Datum
double3_recv(PG_FUNCTION_ARGS) 
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	double3* result = palloc(sizeof(double3));
	const char* bytes = pq_getmsgbytes(buf, sizeof(double3));
	memcpy(result, bytes, sizeof(double3));
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double3_send);

PGDLLEXPORT Datum
double3_send(PG_FUNCTION_ARGS) 
{
	double3* d = (double3*) PG_GETARG_POINTER(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendbytes(&buf, (void*) d, sizeof(double3));
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/* Constructor */

double3 *
double3_construct(double a, double b, double c)
{
	double3 *result;
	result = (double3 *) palloc(sizeof(double3));
	result->a = a;
	result->b = b;
	result->c = c;
	return result;
}

/* Addition */

double3 *
double3_add(double3 *d1, double3 *d2)
{
	double3 *result = (double3 *) palloc(sizeof(double3));
	result->a = d1->a + d2->a;
	result->b = d1->b + d2->b;
	result->c = d1->c + d2->c;
	return result;
}

/* Equality */

bool
double3_eq(double3 *d1, double3 *d2)
{
	return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c);
}

/* Comparator */

int
double3_cmp(double3 *d1, double3 *d2)
{
	int cmp = float8_cmp_internal(d1->a, d2->a);
	if (cmp == 0)
	{
		cmp = float8_cmp_internal(d1->b, d2->b);
		if (cmp == 0)
			cmp = float8_cmp_internal(d1->c, d2->c);
	}
	return cmp;
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double4_in);

PGDLLEXPORT Datum
double4_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("function double4_in not implemented")));
	PG_RETURN_POINTER(NULL);
}

/* Output function */
 
PG_FUNCTION_INFO_V1(double4_out);

PGDLLEXPORT Datum
double4_out(PG_FUNCTION_ARGS)
{
	double4 *d = (double4 *) PG_GETARG_POINTER(0);
	char *result;

	result = psprintf("(%g,%g,%g,%g)", d->a, d->b, d->c, d->d);
	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double4_recv);

PGDLLEXPORT Datum
double4_recv(PG_FUNCTION_ARGS) 
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	double4* result = palloc(sizeof(double4));
	const char* bytes = pq_getmsgbytes(buf, sizeof(double4));
	memcpy(result, bytes, sizeof(double4));
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double4_send);

PGDLLEXPORT Datum
double4_send(PG_FUNCTION_ARGS) 
{
	double4* d = (double4*) PG_GETARG_POINTER(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendbytes(&buf, (void*) d, sizeof(double4));
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/* Constructor */

double4 *
double4_construct(double a, double b, double c, double d)
{
	double4 *result;
	result = (double4 *) palloc(sizeof(double4));
	result->a = a;
	result->b = b;
	result->c = c;
	result->d = d;
	return result;
}

/* Addition */

double4 *
double4_add(double4 *d1, double4 *d2)
{
	double4 *result = (double4 *) palloc(sizeof(double4));
	result->a = d1->a + d2->a;
	result->b = d1->b + d2->b;
	result->c = d1->c + d2->c;
	result->d = d1->d + d2->d;
	return result;
}

/* Equality */

bool
double4_eq(double4 *d1, double4 *d2)
{
	return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c && 
		d1->d == d2->d);
}

/* Comparator */

int
double4_cmp(double4 *d1, double4 *d2)
{
	int cmp = float8_cmp_internal(d1->a, d2->a);
	if (cmp == 0)
	{
		cmp = float8_cmp_internal(d1->b, d2->b);
		if (cmp == 0)
		{
			cmp = float8_cmp_internal(d1->c, d2->c);
			if (cmp == 0)
				cmp = float8_cmp_internal(d1->d, d2->d);
		}
	}
	return cmp;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tdouble2_in);

PGDLLEXPORT Datum
tdouble2_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("Type tdouble2 is an internal type")));
	PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(tdouble3_in);

PGDLLEXPORT Datum
tdouble3_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("Type tdouble3 is an internal type")));
	PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(tdouble4_in);

PGDLLEXPORT Datum
tdouble4_in(PG_FUNCTION_ARGS)
{
	ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		errmsg("Type tdouble4 is an internal type")));
	PG_RETURN_POINTER(NULL);
}

/*****************************************************************************/
