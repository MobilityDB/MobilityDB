/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/** @file timetypes.h
 * Functions for time types based on `TimestampTz`, that is,
 * `TimestampSet`, `Period`, and `PeriodSet`.
 *
 * The `Period` type is a specialized version of the `RangeType` in PostgreSQL.
 * It is considerably more efficient, in particular because it is a
 * fix-length type, it has finite bounds, and do not allow empty periods.
 * The `TimestampSet` type represents sets of disjoint `TimestampTz`.
 * The `PeriodSet` type represents sets of disjoint `Period` values.
 */

#ifndef __TIMETYPES_H__
#define __TIMETYPES_H__

/* PostgreSQL */
#include <utils/timestamp.h>
/* MobilityDB */
#include "general/span.h"

/*****************************************************************************/

/**
 * Structure to represent timestamp sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  int32 count;          /**< Number of TimestampTz elements */
  Period period;        /**< Bounding period */
  TimestampTz elems[1]; /**< Beginning of variable-length data */
} TimestampSet;

/**
 * Structure to represent period sets
 */
typedef struct
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  int32 count;          /**< Number of Period elements */
  Period period;        /**< Bounding period */
  Period elems[1];      /**< Beginning of variable-length data */
} PeriodSet;

/*
 * fmgr macros for time types
 */

#define DatumGetTimestampSetP(X)     ((TimestampSet *) PG_DETOAST_DATUM(X))
#define TimestampSetPGetDatum(X)     PointerGetDatum(X)
#define PG_GETARG_TIMESTAMPSET_P(X)  ((TimestampSet *) PG_GETARG_VARLENA_P(X))
#define PG_RETURN_TIMESTAMPSET_P(X)  PG_RETURN_POINTER(X)

#define DatumGetPeriodSetP(X)        ((PeriodSet *) PG_DETOAST_DATUM(X))
#define PeriodSetPGetDatum(X)        PointerGetDatum(X)
#define PG_GETARG_PERIODSET_P(X)     ((PeriodSet *) PG_GETARG_VARLENA_P(X))
#define PG_RETURN_PERIODSET_P(X)     PG_RETURN_POINTER(X)

/*****************************************************************************/

#endif
