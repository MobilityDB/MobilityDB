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

/**
 * @brief General functions for ordered set values composed of an ordered
 * list of distinct values.
 */

#include "general/set.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/time_ops.h"
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/temporal_catalog.h"
#include "pg_general/temporal_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Orderedset_in);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Input function for timestamp sets
 * @sqlfunc intset_in(), bigintset_in(), floatset_in(), timestampset_in()
 */
PGDLLEXPORT Datum
Orderedset_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid ostypid = PG_GETARG_OID(1);
  OrderedSet *result = orderedset_in(input, oid_type(ostypid));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Orderedset_out);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Output function for timestamp sets
 * @sqlfunc intset_out(), bigintset_out(), floatset_out(), timestampset_out()
 */
PGDLLEXPORT Datum
Orderedset_out(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  char *result = orderedset_out(os);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(Orderedset_recv);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Receive function for timestamp set
 * @sqlfunc intset_recv(), bigintset_recv(), floatset_recv(), timestampset_recv()
 */
PGDLLEXPORT Datum
Orderedset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  OrderedSet *result = orderedset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Orderedset_send);
/**
 * @ingroup mobilitydb_spantime_in_out
 * @brief Send function for timestamp set
 * @sqlfunc intset_send(), bigintset_send(), floatset_send(), timestampset_send()
 */
PGDLLEXPORT Datum
Orderedset_send(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(os);
  uint8_t *wkb = orderedset_as_wkb(os, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Orderedset_constructor);
/**
 * @ingroup mobilitydb_spantime_constructor
 * @brief Construct an ordered set from an array of values
 * @sqlfunc intset(), bigintset(), floatset(), timestampset()
 */
PGDLLEXPORT Datum
Orderedset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  mobdbType settype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  int count;
  Datum *values = datumarr_extract(array, &count);
  mobdbType basetype = settype_basetype(settype);
  OrderedSet *result = orderedset_make_free(values, count, basetype);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Value_to_orderedset);
/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Cast a timestamp as a timestamp set
 * @sqlfunc timestampset()
 */
PGDLLEXPORT Datum
Value_to_orderedset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  TimestampSet *result = value_to_orderedset(d, basetype);
  PG_RETURN_POINTER(result);
}

/**
 * @ingroup mobilitydb_spantime_cast
 * @brief Peak into a timestamp set datum to find the bounding box. If the datum needs
 * to be detoasted, extract only the header and not the full object.
 */
void
orderedset_span_slice(Datum d, Span *p)
{
  OrderedSet *os = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) d))
    os = (OrderedSet *) PG_DETOAST_DATUM_SLICE(d, 0, time_max_header_size());
  else
    os = (OrderedSet *) d;
  memcpy(p, &os->span, sizeof(Span));
  PG_FREE_IF_COPY_P(os, DatumGetPointer(d));
  return;
}

PG_FUNCTION_INFO_V1(Orderedset_to_span);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the span of an ordered set
 * @sqlfunc timespan()
 */
PGDLLEXPORT Datum
Orderedset_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  orderedset_span_slice(d, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_to_timespan);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timespan of a timestamp set
 * @sqlfunc timespan()
 */
PGDLLEXPORT Datum
Timestampset_to_timespan(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *result = timestampset_to_timespan(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Orderedset_mem_size);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the size in bytes of a timestamp set
 * @sqlfunc memSize()
 */
PGDLLEXPORT Datum
Orderedset_mem_size(PG_FUNCTION_ARGS)
{
  OrderedSet *os = PG_GETARG_ORDEREDSET_P(0);
  Datum result = orderedset_mem_size(os);
  PG_FREE_IF_COPY(os, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Timestampset_timespan);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timespan of a timestamp set
 * @sqlfunc timespan()
 */
PGDLLEXPORT Datum
Timestampset_timespan(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *result = timestampset_timespan(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Orderedset_num_values);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the number of timestamps of a timestamp set
 * @sqlfunc numTimestamp()
 */
PGDLLEXPORT Datum
Orderedset_num_values(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  int result = orderedset_num_values(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Orderedset_start_value);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the start timestamp of a timestamp set
 * @sqlfunc startTimestamp()
 */
PGDLLEXPORT Datum
Orderedset_start_value(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = orderedset_start_value(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Orderedset_end_value);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the end timestamp of a timestamp set
 * @sqlfunc endTimestamp()
 */
PGDLLEXPORT Datum
Orderedset_end_value(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampTz result = orderedset_end_value(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(Orderedset_value_n);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the n-th timestamp of a timestamp set
 * @sqlfunc timestampN()
 */
PGDLLEXPORT Datum
Orderedset_value_n(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  Datum result;
  bool found = orderedset_value_n(ts, n, &result);
  PG_FREE_IF_COPY(ts, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Orderedset_values);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the timestamps of a timestamp set
 * @sqlfunc timestamps()
 */
PGDLLEXPORT Datum
Orderedset_values(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Datum *values = orderedset_values(ts);
  ArrayType *result = datumarr_to_array(values, ts->count, ts->span.basetype);
  pfree(values);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_shift);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Shift a timestamp set by an interval
 * @sqlfunc shift()
 */
PGDLLEXPORT Datum
Timestampset_shift(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  TimestampSet *result = timestampset_shift_tscale(ts, shift, NULL);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_tscale);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Scale a timestamp set by an interval
 * @sqlfunc tscale()
 */
PGDLLEXPORT Datum
Timestampset_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, NULL, duration);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Timestampset_shift_tscale);
/**
 * @ingroup mobilitydb_spantime_transf
 * @brief Shift and scale a timestamp set by the intervals
 * @sqlfunc shiftTscale()
 */
PGDLLEXPORT Datum
Timestampset_shift_tscale(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  ensure_valid_duration(duration);
  TimestampSet *result = timestampset_shift_tscale(ts, shift, duration);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Orderedset_cmp);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first timestamp set
 * is less than, equal, or greater than the second temporal value
 * @sqlfunc timestampset_cmp()
 */
PGDLLEXPORT Datum
Orderedset_cmp(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  int cmp = orderedset_cmp(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_INT32(cmp);
}

PG_FUNCTION_INFO_V1(Orderedset_eq);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is equal to the second one
 * @sqlfunc timestampset_eq()
 * @sqlop @p =
 */
PGDLLEXPORT Datum
Orderedset_eq(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_eq(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Orderedset_ne);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is different from the second one
 * @sqlfunc timestampset_ne()
 * @sqlop @p <>
 */
PGDLLEXPORT Datum
Orderedset_ne(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_ne(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Orderedset_lt);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is less than the second one
 * @sqlfunc timestampset_lt()
 * @sqlop @p <
 */
PGDLLEXPORT Datum
Orderedset_lt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_lt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Orderedset_le);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is less than
 * or equal to the second one
 * @sqlfunc timestampset_le()
 * @sqlop @p <=
 */
PGDLLEXPORT Datum
Orderedset_le(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_le(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Orderedset_ge);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is greater than
 * or equal to the second one
 * @sqlfunc timestampset_ge()
 * @sqlop @p >=
 */
PGDLLEXPORT Datum
Orderedset_ge(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_ge(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Orderedset_gt);
/**
 * @ingroup mobilitydb_spantime_comp
 * @brief Return true if the first timestamp set is greater than the second one
 * @sqlfunc timestampset_gt()
 * @sqlop @p >
 */
PGDLLEXPORT Datum
Orderedset_gt(PG_FUNCTION_ARGS)
{
  TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET_P(0);
  TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET_P(1);
  bool result = orderedset_gt(ts1, ts2);
  PG_FREE_IF_COPY(ts1, 0);
  PG_FREE_IF_COPY(ts2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Orderedset_hash);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the 32-bit hash value of a timestamp set
 * @sqlfunc timestampset_hash()
 */
PGDLLEXPORT Datum
Orderedset_hash(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint32 result = orderedset_hash(ts);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Orderedset_hash_extended);
/**
 * @ingroup mobilitydb_spantime_accessor
 * @brief Return the 64-bit hash value of a timestamp set using a seed
 * @sqlfunc timestampset_hash_extended()
 */
PGDLLEXPORT Datum
Orderedset_hash_extended(PG_FUNCTION_ARGS)
{
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = orderedset_hash_extended(ts, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
