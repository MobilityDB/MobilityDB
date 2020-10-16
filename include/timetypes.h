/*****************************************************************************
 *
 * timetypes.h
 *     Functions for time types based on timestamptz, that is,
 *    timestampset, period, periodset
 *
 * The Period type is a specialized version of the RangeType in PostgreSQL. 
 * It is considerably more efficient, in particular because it is a
 * fix-length type, it has finite bounds, and do not allow empty periods. 
 * The TimestampSet type represents a set of disjoint timestamptz.
 * The PeriodSet type represents a set of disjoint periods. 
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIMETYPES_H__
#define __TIMETYPES_H__

#include <postgres.h>
#include <access/stratnum.h>
#include <utils/timestamp.h>

/**
 * Structure to represent periods
 */
typedef struct 
{
  TimestampTz  lower;  /**< lower bound value */
  TimestampTz  upper;  /**< upper bound value */
  bool lower_inc;      /**< lower bound is inclusive (vs exclusive) */
  bool upper_inc;      /**< upper bound is inclusive (vs exclusive) */
} Period;

/**
 * Internal representation of either bound of a period (not what's on disk) 
 */
typedef struct
{
  TimestampTz t;       /**< bound value */
  bool inclusive;      /**< bound is inclusive (vs exclusive) */
  bool lower;          /**< this is the lower (vs upper) bound */
} PeriodBound;

/**
 * Structure to represent timestamp sets
 */
typedef struct 
{
  int32 vl_len_;       /**< varlena header (do not touch directly!) */
  int32 count;         /**< number of TimestampTz elements */
   /* variable-length data follows */
} TimestampSet;

/**
 * Structure to represent period sets
 */
typedef struct 
{
  int32 vl_len_;        /**< varlena header (do not touch directly!) */
  int32 count;          /**< number of Period elements */
   /* variable-length data follows */
} PeriodSet;

/*
 * fmgr macros for time types
 */

#define DatumGetTimestampSet(X)    ((TimestampSet *) DatumGetPointer(X))
#define TimestampSetGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_TIMESTAMPSET(n)  DatumGetTimestampSet(PG_GETARG_POINTER(n))
#define PG_RETURN_TIMESTAMPSET(x)  PG_RETURN_POINTER(x)

#define DatumGetPeriod(X)      ((Period *) DatumGetPointer(X))
#define PeriodGetDatum(X)      PointerGetDatum(X)
#define PG_GETARG_PERIOD(n)      DatumGetPeriod(PG_GETARG_POINTER(n))
#define PG_RETURN_PERIOD(x)      PG_RETURN_POINTER(x)

#define DatumGetPeriodSet(X)    ((PeriodSet *) DatumGetPointer(X))
#define PeriodSetGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_PERIODSET(n)    DatumGetPeriodSet(PG_GETARG_POINTER(n))
#define PG_RETURN_PERIODSET(x)    PG_RETURN_POINTER(x)

/*****************************************************************************/

#endif
