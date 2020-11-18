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
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
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
