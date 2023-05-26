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
 * @file
 * @brief General functions for set of disjoint spans.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <access/heaptoast.h>
  #include <access/detoast.h>
#else
  #include <access/tuptoaster.h>
#endif
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/spanset.h"
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_in);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span set from its string representation
 * @sqlfunc spanset_in()
 */
Datum
Spanset_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid sstypid = PG_GETARG_OID(1);
  SpanSet *result = spanset_in(input, oid_type(sstypid));
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the string representation of a span set
 * @sqlfunc spanset_out()
 */
Datum
Spanset_out(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  char *result = spanset_out(ps, Int32GetDatum(OUT_DEFAULT_DECIMAL_DIGITS));
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Spanset_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_recv);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Receive function for span set
 * @sqlfunc spanset_recv()
 */
Datum
Spanset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  SpanSet *result = spanset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_send);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Send function for span set
 * @sqlfunc spanset_send()
 */
Datum
Spanset_send(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(ps);
  uint8_t *wkb = spanset_as_wkb(ps, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

PGDLLEXPORT Datum Spanset_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_constructor);
/**
 * @ingroup mobilitydb_setspan_constructor
 * @brief Construct a span set from an array of span values
 * @sqlfunc spanset()
 */
Datum
Spanset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_non_empty_array(array);
  int count;
  Span *spans = spanarr_extract(array, &count);
  SpanSet *result = spanset_make_free(spans, count, NORMALIZE);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PGDLLEXPORT Datum Value_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_to_spanset);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Cast a value as a span set
 * @sqlfunc intspanset(), bigintspanset(), floatspanset(), periodset()
 */
Datum
Value_to_spanset(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  SpanSet *result = value_to_spanset(d, basetype);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_spanset);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Cast the span value as a span set
 * @sqlfunc instspanset(), floatspanset(), periodset()
 */
Datum
Span_to_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  SpanSet *result = span_to_spanset(s);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Set_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_spanset);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Cast the timestamp set value as a period set
 * @sqlfunc intspanset(), bigintspanset(), floatspanset(), periodset()
 */
Datum
Set_to_spanset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  SpanSet *result = set_to_spanset(s);
  PG_RETURN_POINTER(result);
}

/**
 * @brief Peak into a span set datum to find the bounding box. If the datum
 * needs to be detoasted, extract only the header and not the full object.
 */
void
spanset_span_slice(Datum d, Span *s)
{
  SpanSet *ss = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) d))
    ss = (SpanSet *) PG_DETOAST_DATUM_SLICE(d, 0, time_max_header_size());
  else
    ss = (SpanSet *) d;
  memcpy(s, &ss->span, sizeof(Span));
  PG_FREE_IF_COPY_P(ss, DatumGetPointer(d));
  return;
}

PGDLLEXPORT Datum Spanset_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_span);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the bounding span on which a span set is defined
 * @sqlfunc span()
 */
Datum
Spanset_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  spanset_span_slice(d, result);
  PG_RETURN_POINTER(result);
}

#if POSTGRESQL_VERSION_NUMBER >= 140000
PGDLLEXPORT Datum Spanset_to_multirange(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_to_multirange);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Convert the integer span as a integer range value
 * @sqlfunc int4range(), tstzrange()
 * @sqlop @p ::
 */
Datum
Spanset_to_multirange(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  assert(ss->spantype == T_INTSPAN || ss->spantype == T_TSTZSPAN);
  MultirangeType *result = multirange_make(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Multirange_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Multirange_to_spanset);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Convert the multi range value as a span set
 * @sqlfunc intspanset(), periodset()
 * @sqlop @p ::
 */
Datum
Multirange_to_spanset(PG_FUNCTION_ARGS)
{
  MultirangeType *mrange = PG_GETARG_MULTIRANGE_P(0);
  TypeCacheEntry *typcache = multirange_get_typcache(fcinfo,
    MultirangeTypeGetOid(mrange));

  Span *spans = palloc(sizeof(Span) * mrange->rangeCount);
  for (uint32 i = 0; i < mrange->rangeCount; i++)
  {
    RangeType *range = multirange_get_range(typcache->rngtype, mrange, i);
    range_set_span(range, typcache->rngtype, &spans[i]);
  }
  SpanSet *ss = spanset_make(spans, mrange->rangeCount, NORMALIZE);
  pfree(spans);
  PG_FREE_IF_COPY(mrange, 0);
  PG_RETURN_POINTER(ss);
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_mem_size(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_mem_size);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the memory size in bytes of a span set
 * @sqlfunc memSize()
 */
Datum
Spanset_mem_size(PG_FUNCTION_ARGS)
{
  Datum result = toast_raw_datum_size(PG_GETARG_DATUM(0));
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Spanset_lower(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_lower);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the lower bound value
 * @sqlfunc lower()
 */
Datum
Spanset_lower(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  PG_RETURN_DATUM(ss->elems[0].lower);
}

PGDLLEXPORT Datum Spanset_upper(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_upper);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the upper bound value
 * @sqlfunc upper()
 */
Datum
Spanset_upper(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  PG_RETURN_DATUM(ss->elems[ss->count - 1].upper);
}

/* span -> bool functions */

PGDLLEXPORT Datum Spanset_lower_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_lower_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the lower bound value is inclusive
 * @sqlfunc lower_inc()
 */
Datum
Spanset_lower_inc(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  PG_RETURN_BOOL(spanset_lower_inc(ss));
}

Datum Spanset_upper_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_upper_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the upper bound value is inclusive
 * @sqlfunc lower_inc()
 */
Datum
Spanset_upper_inc(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  PG_RETURN_BOOL(spanset_upper_inc(ss));
}

PGDLLEXPORT Datum Spanset_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_width);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the width of a numeric span set
 * @sqlfunc width()
 */
Datum
Spanset_width(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  double result = spanset_width(ss);
  PG_RETURN_FLOAT8(result);
}

Datum  Periodset_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of a period set
 * @sqlfunc duration()
 */
Datum
Periodset_duration(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  bool boundspan = PG_GETARG_BOOL(1);
  Interval *result = periodset_duration(ps, boundspan);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Periodset_num_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_num_timestamps);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of timestamps of a period set
 * @sqlfunc numTimestamps()
 */
Datum
Periodset_num_timestamps(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  int result = periodset_num_timestamps(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Periodset_start_timestamp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_start_timestamp);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start timestamp of a period set
 * @sqlfunc startTimestamp()
 */
Datum
Periodset_start_timestamp(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  TimestampTz result = periodset_start_timestamp(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Periodset_end_timestamp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_end_timestamp);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end timestamp of a period set
 * @sqlfunc endTimestamp()
 */
Datum
Periodset_end_timestamp(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  TimestampTz result = periodset_end_timestamp(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Periodset_timestamp_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_timestamp_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th timestamp of a period set
 * @sqlfunc timestampN()
 */
Datum
Periodset_timestamp_n(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = periodset_timestamp_n(ps, n, &result);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Periodset_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_timestamps);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the array of timestamps of a period set
 * @sqlfunc timestamps()
 */
Datum
Periodset_timestamps(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  int count;
  TimestampTz *times = periodset_timestamps(ps, &count);
  ArrayType *result = timestamparr_to_array(times, count);
  pfree(times);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Spanset_num_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_num_spans);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of spans of a span set
 * @sqlfunc numSpans()
 */
Datum
Spanset_num_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  int result = spanset_num_spans(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Spanset_start_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_start_span);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start span of a span set
 * @sqlfunc startSpan(), startPeriod()
 */
Datum
Spanset_start_span(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  Span *result = spanset_start_span(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_end_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_end_span);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end span of a span set
 * @sqlfunc endSpan(), endPeriod()
 */
Datum
Spanset_end_span(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  Span *result = spanset_end_span(ps);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_span_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_span_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th span of a span set
 * @sqlfunc spanN()
 */
Datum
Spanset_span_n(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  Span *result = spanset_span_n(ps, i);
  PG_FREE_IF_COPY(ps, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_spans);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the spans of a span set
 * @sqlfunc spans()
 */
Datum
Spanset_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  const Span **spans = spanset_spans(ps);
  ArrayType *result = spanarr_to_array(spans, ps->count);
  pfree(spans);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Set the precision of the float span set to the number of decimal places.
 */
static SpanSet *
floatspanset_round(SpanSet *ss, Datum size)
{
  Span *spans = palloc(sizeof(Span) * ss->count);
  for (int i = 0; i < ss->count; i++)
  {
    const Span *span = spanset_sp_n(ss, i);
    floatspan_round(span, size, &spans[i]);
  }
  SpanSet *result = spanset_make_free(spans, ss->count, NORMALIZE);
  return result;
}

PGDLLEXPORT Datum Floatspanset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspanset_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Set the precision of the float span set to the number of decimal places
 * @sqlfunc round()
 */
Datum
Floatspanset_round(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum size = PG_GETARG_DATUM(1);
  SpanSet *result = floatspanset_round(ss, size);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Spanset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift a span set by a value
 * @sqlfunc shift()
 */
Datum
Spanset_shift(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  SpanSet *result = spanset_copy(ss);
  spanset_shift(result, shift);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Periodset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift a period set by an interval
 * @sqlfunc shift()
 */
Datum
Periodset_shift(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  SpanSet *result = periodset_shift_tscale(ps, shift, NULL);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Periodset_tscale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_tscale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift a period set by an interval
 * @sqlfunc tscale()
 */
Datum
Periodset_tscale(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  SpanSet *result = periodset_shift_tscale(ps, NULL, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Periodset_shift_tscale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Periodset_shift_tscale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift a period set by an interval
 * @sqlfunc shiftTscale()
 */
Datum
Periodset_shift_tscale(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  SpanSet *result = periodset_shift_tscale(ps, shift, duration);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_cmp);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first period set
 * is less than, equal, or greater than the second one
 * @sqlfunc spanset_cmp()
 */
Datum
Spanset_cmp(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  int cmp = spanset_cmp(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_INT32(cmp);
}

PGDLLEXPORT Datum Spanset_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_eq);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is equal to the second one
 * @sqlfunc spanset_eq()
 * @sqlop @p =
 */
Datum
Spanset_eq(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_eq(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Spanset_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_ne);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is different from the second one
 * @sqlfunc spanset_ne()
 * @sqlop @p <>
 */
Datum
Spanset_ne(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_ne(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PGDLLEXPORT Datum Spanset_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_lt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is less than the second one
 * @sqlfunc spanset_lt()
 * @sqlop @p <
 */
Datum
Spanset_lt(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_lt(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Spanset_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_le);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is less than or equal to
 * the second one
 * @sqlfunc spanset_le()
 * @sqlop @p <=
 */
Datum
Spanset_le(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_le(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Spanset_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_ge);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is greater than or equal to
 * the second one
 * @sqlfunc spanset_ge()
 * @sqlop @p >=
 */
Datum
Spanset_ge(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_ge(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Spanset_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_gt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first period set is greater than the second one
 * @sqlfunc spanset_gt()
 * @sqlop @p >
 */
Datum
Spanset_gt(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_gt(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_hash);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 32-bit hash value of a period set
 * @sqlfunc spanset_hash()
 */
Datum
Spanset_hash(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  uint32 result = spanset_hash(ps);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Spanset_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_hash_extended);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 64-bit hash value of a period set using a seed
 * @sqlfunc spanset_hash_extended()
 */
Datum
Spanset_hash_extended(PG_FUNCTION_ARGS)
{
  SpanSet *ps = PG_GETARG_SPANSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = spanset_hash_extended(ps, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
