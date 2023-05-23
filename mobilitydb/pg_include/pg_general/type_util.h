/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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

/**
 * @brief Miscellaneous utility functions for temporal types.
 */

#ifndef __PG_TEMPORAL_UTIL_H__
#define __PG_TEMPORAL_UTIL_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/palloc.h>
#include <fmgr.h>
#include <lib/stringinfo.h>
#include <utils/array.h>
#include <utils/rangetypes.h>
#if POSTGRESQL_VERSION_NUMBER >= 140000
  #include <utils/multirangetypes.h>
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */
/* MEOS */
#include "general/span.h"
#include "general/temporal.h"

/*****************************************************************************/

/* PostgreSQL call helpers */

extern Datum call_input(Oid typid, char *str, bool end);
extern char *call_output(Oid typid, Datum value);
extern Datum call_recv(meosType type, StringInfo buf);
extern bytea *call_send(meosType type, Datum value);

extern Datum call_function1(PGFunction func, Datum arg1);
extern Datum call_function2(PGFunction func, Datum arg1, Datum arg2);
extern Datum call_function3(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3);
extern Datum call_function4(PGFunction func, Datum arg1, Datum arg2,
  Datum arg3, Datum arg4);

extern Datum CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo,
  Oid collid, Datum arg1, Datum arg2, Datum arg3, Datum arg4);

extern Datum CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo,
    Oid collid, Datum arg1, Datum arg2, Datum arg3, Datum arg4);

/* Range functions */

extern RangeType *range_make(Datum from, Datum to, bool lower_inc,
  bool upper_inc, meosType basetype);
#if POSTGRESQL_VERSION_NUMBER >= 140000
  extern MultirangeType *multirange_make(const SpanSet *ss);
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */

/* Array functions */

extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Span *spanarr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, meosType type);
extern ArrayType *int64arr_to_array(const int64 *longints, int count);
extern ArrayType *timestamparr_to_array(const TimestampTz *times, int count);
extern ArrayType *spanarr_to_array(const Span **spans, int count);
extern ArrayType *strarr_to_textarray(char **strarr, int count);
extern ArrayType *temporalarr_to_array(const Temporal **temporal, int count);
extern ArrayType *stboxarr_to_array(STBox *boxarr, int count);

/*****************************************************************************/

#endif /* __PG_TEMPORAL_UTIL_H__ */
