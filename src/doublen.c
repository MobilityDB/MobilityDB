/*****************************************************************************
 *
 * doublen.c
 *  Internal types used for the average and centroid temporal aggregates. 
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/**
 * @file doublen.c
 * Internal types used for computing the average and centroid temporal 
 * aggregates. 
 *
 * The `double2`, `double3`, and `double4` types are base types composed, 
 * respectively, of two, three, and four `double` values. The `tdouble2`, 
 * `tdouble3`, and `tdouble4` types are the corresponding temporal types. 
 * The in/out functions of all these types are stubs since all access should 
 * be internal. These types are used for the transition function of the 
 * aggregates, where the first components of the `doubleN` values store the 
 * sum and the last one stores the count of the values. The final function 
 * computes the average from the `doubleN` values.
*/
#include "doublen.h"

#include <libpq/pqformat.h>
#include <utils/builtins.h>

#if MOBDB_PGSQL_VERSION >= 120000
#include <utils/float.h>
#endif

/*****************************************************************************
 * Input/Output functions
 * Although doubleN are internal types, the doubleN_out function are 
 * implemented for debugging purposes.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double2_in);
/** 
 * Input function for double2 values (stub only)
 */
PGDLLEXPORT Datum
double2_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("function double2_in not implemented")));
  PG_RETURN_POINTER(NULL);
}
 
PG_FUNCTION_INFO_V1(double2_out);
/** 
 * Output function for double2 values (stub only)
 */
PGDLLEXPORT Datum
double2_out(PG_FUNCTION_ARGS)
{
  double2 *d = (double2 *) PG_GETARG_POINTER(0);
  char *result;

  result = psprintf("(%g,%g)", d->a, d->b);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double2_recv);
/** 
 * Receive function for double2 values
 */
PGDLLEXPORT Datum
double2_recv(PG_FUNCTION_ARGS) 
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  double2 *result = palloc(sizeof(double2));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double2));
  memcpy(result, bytes, sizeof(double2));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double2_send);
/** 
 * Send function for double2 values
 */
PGDLLEXPORT Datum
double2_send(PG_FUNCTION_ARGS) 
{
  double2 *d = (double2 *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double2));
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double2 value from the double values 
 */
void 
double2_set(double2 *result, double a, double b)
{
  result->a = a;
  result->b = b;
}

/**
 * Returns the addition of the double2 values
 */
double2 *
double2_add(double2 *d1, double2 *d2)
{
  double2 *result = (double2 *) palloc(sizeof(double2));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  return result;
}

/**
 * Returns true if the double2 values are equal
 */
bool
double2_eq(double2 *d1, double2 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b);
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double3_in);
/** 
 * Input function for double2 values (stub only)
 */
PGDLLEXPORT Datum
double3_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("function double3_in not implemented")));
  PG_RETURN_POINTER(NULL);
}
 
PG_FUNCTION_INFO_V1(double3_out);
/** 
 * Output function for double3 values (stub only)
 */
PGDLLEXPORT Datum
double3_out(PG_FUNCTION_ARGS)
{
  double3 *d = (double3 *) PG_GETARG_POINTER(0);
  char *result;

  result = psprintf("(%g,%g,%g)", d->a, d->b, d->c);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double3_recv);
/** 
 * Receive function for double3 values
 */
PGDLLEXPORT Datum
double3_recv(PG_FUNCTION_ARGS) 
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  double3 *result = palloc(sizeof(double3));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double3));
  memcpy(result, bytes, sizeof(double3));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double3_send);
/** 
 * Send function for double3 values
 */
PGDLLEXPORT Datum
double3_send(PG_FUNCTION_ARGS) 
{
  double3 *d = (double3 *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double3));
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double3 value from the double values 
 */
void 
double3_set(double3 *result, double a, double b, double c)
{
  result->a = a;
  result->b = b;
  result->c = c;
}

/**
 * Returns the addition of the double3 values
 */
double3 *
double3_add(double3 *d1, double3 *d2)
{
  double3 *result = (double3 *) palloc(sizeof(double3));
  result->a = d1->a + d2->a;
  result->b = d1->b + d2->b;
  result->c = d1->c + d2->c;
  return result;
}

/**
 * Returns true if the double3 values are equal
 */
bool
double3_eq(double3 *d1, double3 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c);
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(double4_in);
/** 
 * Input function for double4 values (stub only)
 */
PGDLLEXPORT Datum
double4_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("function double4_in not implemented")));
  PG_RETURN_POINTER(NULL);
}
 
PG_FUNCTION_INFO_V1(double4_out);
/** 
 * Output function for double4 values (stub only)
 */
PGDLLEXPORT Datum
double4_out(PG_FUNCTION_ARGS)
{
  double4 *d = (double4 *) PG_GETARG_POINTER(0);
  char *result;

  result = psprintf("(%g,%g,%g,%g)", d->a, d->b, d->c, d->d);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(double4_recv);
/** 
 * Receive function for double4 values
 */
PGDLLEXPORT Datum
double4_recv(PG_FUNCTION_ARGS) 
{
  StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
  double4 *result = palloc(sizeof(double4));
  const char *bytes = pq_getmsgbytes(buf, sizeof(double4));
  memcpy(result, bytes, sizeof(double4));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(double4_send);
/** 
 * Send function for double3 values
 */
PGDLLEXPORT Datum
double4_send(PG_FUNCTION_ARGS) 
{
  double4 *d = (double4 *) PG_GETARG_POINTER(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  pq_sendbytes(&buf, (void *) d, sizeof(double4));
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Set a double4 value from the double values 
 */
void 
double4_set(double4 *result, double a, double b, double c, double d)
{
  result->a = a;
  result->b = b;
  result->c = c;
  result->d = d;
}

/**
 * Returns the addition of the double4 values
 */
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

/**
 * Returns true if the double4 values are equal
 */
bool
double4_eq(double4 *d1, double4 *d2)
{
  return (d1->a == d2->a && d1->b == d2->b && d1->c == d2->c && 
    d1->d == d2->d);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tdouble2_in);
/** 
 * Input function for the temporal double2 type (stub only)
 */
PGDLLEXPORT Datum
tdouble2_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("Type tdouble2 is an internal type")));
  PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(tdouble3_in);
/** 
 * Input function for the temporal double3 type (stub only)
 */
PGDLLEXPORT Datum
tdouble3_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("Type tdouble3 is an internal type")));
  PG_RETURN_POINTER(NULL);
}

PG_FUNCTION_INFO_V1(tdouble4_in);
/** 
 * Input function for the temporal double4 type (stub only)
 */
PGDLLEXPORT Datum
tdouble4_in(PG_FUNCTION_ARGS)
{
  ereport(ERROR,(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
    errmsg("Type tdouble4 is an internal type")));
  PG_RETURN_POINTER(NULL);
}

/*****************************************************************************/
