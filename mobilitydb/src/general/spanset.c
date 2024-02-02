/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief General functions for span set types composed of a set of disjoint
 * spans
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
#include "general/span.h"
#include "general/temporal.h"
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
 * @brief Return a span set from its Well-Known Text (WKT) representation
 * @sqlfn spanset_in()
 */
Datum
Spanset_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid sstypid = PG_GETARG_OID(1);
  SpanSet *result = spanset_in(input, oid_type(sstypid));
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Spanset_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span set
 * @sqlfn spanset_out()
 */
Datum
Spanset_out(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  char *result = spanset_out(ss, Int32GetDatum(OUT_DEFAULT_DECIMAL_DIGITS));
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Spanset_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_recv);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span set from its Well-Known Binary (WKB) representation
 * @sqlfn spanset_recv()
 */
Datum
Spanset_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  SpanSet *result = spanset_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Spanset_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_send);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a span set
 * @sqlfn spanset_send()
 */
Datum
Spanset_send(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(ss);
  uint8_t *wkb = spanset_as_wkb(ss, variant, &wkb_size);
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
 * @brief Return a span set from an array of spans
 * @sqlfn spanset()
 */
Datum
Spanset_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  int count;
  Span *spans = spanarr_extract(array, &count);
  SpanSet *result = spanset_make_free(spans, count, NORMALIZE, ORDERED);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Value_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_to_spanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a value converted to a span set
 * @sqlfn intspanset(), floatspanset(), ...
 */
Datum
Value_to_spanset(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_SPANSET_P(value_to_spanset(value, basetype));
}

PGDLLEXPORT Datum Set_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_spanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a set converted to a span set
 * @sqlfn intspanset(), floatspanset(), ...
 */
Datum
Set_to_spanset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  SpanSet *result = set_spanset(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Span_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_spanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a span converted to a span set
 * @sqlfn instspanset(), floatspanset(), ...
 */
Datum
Span_to_spanset(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPANSET_P(span_spanset(s));
}

/**
 * @brief Peek into a span set datum to find the bounding box
 * @note If the datum needs to be detoasted, extract only the header and not
 * the full object
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
  // PG_FREE_IF_COPY_P(ss, DatumGetPointer(d));
  return;
}

PGDLLEXPORT Datum Spanset_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a span set converted to a span
 * @sqlfn span()
 */
Datum
Spanset_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  Span *result = palloc(sizeof(Span));
  spanset_span_slice(d, result);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Intspanset_to_floatspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intspanset_to_floatspanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return an integer span set converted to a float spanset
 * @sqlfn floatspanset()
 * @sqlop @p ::
 */
Datum
Intspanset_to_floatspanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  SpanSet *result = intspanset_floatspanset(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Floatspanset_to_intspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspanset_to_intspanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a float span set converted to an integer spanset
 * @sqlfn intspanset()
 * @sqlop @p ::
 */
Datum
Floatspanset_to_intspanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  SpanSet *result = floatspanset_intspanset(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Datespanset_to_tstzspanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_to_tstzspanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a date span set converted to a timestamptz span set
 * @sqlfn tstzspanset()
 * @sqlop @p ::
 */
Datum
Datespanset_to_tstzspanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  SpanSet *result = datespanset_tstzspanset(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Tstzspanset_to_datespanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_to_datespanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a timestamptz span set converted to a date span set
 * @sqlfn datespanset()
 * @sqlop @p ::
 */
Datum
Tstzspanset_to_datespanset(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  SpanSet *result = tstzspanset_datespanset(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER >= 140000
PGDLLEXPORT Datum Spanset_to_multirange(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_to_multirange);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a span set converted to a multirange
 * @sqlfn int4range(), tstzrange()
 * @sqlop @p ::
 */
Datum
Spanset_to_multirange(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  assert(ss->spantype == T_INTSPAN || ss->spantype == T_DATESPAN ||
    ss->spantype == T_TSTZSPAN);
  MultirangeType *result = multirange_make(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_MULTIRANGE_P(result);
}

PGDLLEXPORT Datum Multirange_to_spanset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Multirange_to_spanset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return a multirange converted to a span set
 * @sqlfn intspanset(), tstzspanset()
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
  SpanSet *result = spanset_make_free(spans, mrange->rangeCount, NORMALIZE,
    ORDERED);
  PG_FREE_IF_COPY(mrange, 0);
  PG_RETURN_SPANSET_P(result);
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
 * @sqlfn memSize()
 */
Datum
Spanset_mem_size(PG_FUNCTION_ARGS)
{
  PG_RETURN_DATUM(toast_raw_datum_size(PG_GETARG_DATUM(0)));
}

PGDLLEXPORT Datum Spanset_lower(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_lower);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the lower bound of a span set
 * @sqlfn lower()
 */
Datum
Spanset_lower(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum result = spanset_lower(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Spanset_upper(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_upper);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the upper bound of a span set
 * @sqlfn upper()
 */
Datum
Spanset_upper(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum result = spanset_upper(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_DATUM(result);
}

/* span -> bool functions */

PGDLLEXPORT Datum Spanset_lower_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_lower_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the lower bound of a span set is inclusive
 * @sqlfn lower_inc()
 */
Datum
Spanset_lower_inc(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool result = spanset_lower_inc(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

Datum Spanset_upper_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_upper_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the upper bound of a span set is inclusive
 * @sqlfn lower_inc()
 */
Datum
Spanset_upper_inc(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool result = spanset_upper_inc(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Numspanset_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspanset_width);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the width of a number span set
 * @sqlfn width()
 */
Datum
Numspanset_width(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool boundspan = PG_GETARG_BOOL(1);
  Datum result = numspanset_width(ss, boundspan);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_DATUM(result);
}

Datum Datespanset_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of a date span set
 * @sqlfn duration()
 */
Datum
Datespanset_duration(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool boundspan = PG_GETARG_BOOL(1);
  Interval *result = datespanset_duration(ss, boundspan);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_INTERVAL_P(result);
}

Datum Tstzspanset_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of a timestamptz span set
 * @sqlfn duration()
 */
Datum
Tstzspanset_duration(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool boundspan = PG_GETARG_BOOL(1);
  Interval *result = tstzspanset_duration(ss, boundspan);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_INTERVAL_P(result);
}

PGDLLEXPORT Datum Datespanset_num_dates(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_num_dates);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of dates of a span set
 * @sqlfn numDates()
 */
Datum
Datespanset_num_dates(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int result = datespanset_num_dates(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Datespanset_start_date(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_start_date);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start date of a span set
 * @sqlfn startDate()
 */
Datum
Datespanset_start_date(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  DateADT result = datespanset_start_date(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_DATEADT(result);
}

PGDLLEXPORT Datum Datespanset_end_date(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_end_date);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end date of a span set
 * @sqlfn endDate()
 */
Datum
Datespanset_end_date(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  DateADT result = datespanset_end_date(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_DATEADT(result);
}

PGDLLEXPORT Datum Datespanset_date_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_date_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th date of a span set
 * @sqlfn dateN()
 */
Datum
Datespanset_date_n(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  DateADT result;
  bool found = datespanset_date_n(ss, n, &result);
  PG_FREE_IF_COPY(ss, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATEADT(result);
}

PGDLLEXPORT Datum Datespanset_dates(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespanset_dates);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the array of dates of a span set
 * @sqlfn dates()
 */
Datum
Datespanset_dates(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int count;
  DateADT *dates = datespanset_dates(ss, &count);
  ArrayType *result = datearr_to_array(dates, count);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tstzspanset_num_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_num_timestamps);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of timestamptz values of a span set
 * @sqlfn numTimestamps()
 */
Datum
Tstzspanset_num_timestamps(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int result = tstzspanset_num_timestamps(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tstzspanset_start_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_start_timestamptz);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start timestamptz of a span set
 * @sqlfn startTimestamp()
 */
Datum
Tstzspanset_start_timestamptz(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  TimestampTz result = tstzspanset_start_timestamptz(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Tstzspanset_end_timestamptz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_end_timestamptz);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end timestamptz of a span set
 * @sqlfn endTimestamp()
 */
Datum
Tstzspanset_end_timestamptz(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  TimestampTz result = tstzspanset_end_timestamptz(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Tstzspanset_timestamptz_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_timestamptz_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th timestamptz of a span set
 * @sqlfn timestampN()
 */
Datum
Tstzspanset_timestamptz_n(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  TimestampTz result;
  bool found = tstzspanset_timestamptz_n(ss, n, &result);
  PG_FREE_IF_COPY(ss, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Tstzspanset_timestamps(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_timestamps);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the array of timestamptz values of a span set
 * @sqlfn timestamps()
 */
Datum
Tstzspanset_timestamps(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int count;
  TimestampTz *times = tstzspanset_timestamps(ss, &count);
  ArrayType *result = tstzarr_to_array(times, count);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Spanset_num_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_num_spans);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of spans of a span set
 * @sqlfn numSpans()
 */
Datum
Spanset_num_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int result = spanset_num_spans(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Spanset_start_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_start_span);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start span of a span set
 * @sqlfn startSpan()
 */
Datum
Spanset_start_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *result = spanset_start_span(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Spanset_end_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_end_span);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end span of a span set
 * @sqlfn endSpan()
 */
Datum
Spanset_end_span(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Span *result = spanset_end_span(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Spanset_span_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_span_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th span of a span set
 * @sqlfn spanN()
 */
Datum
Spanset_span_n(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int i = PG_GETARG_INT32(1); /* Assume 1-based */
  Span *result = spanset_span_n(ss, i);
  PG_FREE_IF_COPY(ss, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Spanset_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_spans);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the array of spans of a span set
 * @sqlfn spans()
 */
Datum
Spanset_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  const Span **spans = spanset_sps(ss);
  ArrayType *result = spanarr_to_array(spans, ss->count);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Transformation functions
 *
 * Since in PostgreSQL the type date is defined as follows
 *   typedef int32 DateADT;
 * the functions #Numspan_shift, #Numspan_scale, and #Numspan_shift_scale are
 * also used for datespans and datespansets
 *****************************************************************************/

PGDLLEXPORT Datum Numspanset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspanset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span set shifted by a value
 * @note This function is also used for `datespanset`
 * @sqlfn shift()
 */
Datum
Numspanset_shift(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  SpanSet *result = numspanset_shift_scale(ss, shift, 0, true, false);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Tstzspanset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span set shifted by an interval
 * @sqlfn shift()
 */
Datum
Tstzspanset_shift(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  SpanSet *result = tstzspanset_shift_scale(ss, shift, NULL);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Numspanset_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspanset_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span set scaled by a value
 * @note This function is also used for `datespanset`
 * @sqlfn scale()
 */
Datum
Numspanset_scale(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum width = PG_GETARG_DATUM(1);
  SpanSet *result = numspanset_shift_scale(ss, 0, width, false, true);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Tstzspanset_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span set scaled by an interval
 * @sqlfn scale()
 */
Datum
Tstzspanset_scale(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  SpanSet *result = tstzspanset_shift_scale(ss, NULL, duration);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Numspanset_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspanset_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span set shifted and scaled by two values
 * @note This function is also used for `datespanset`
 * @sqlfn shiftTscale()
 */
Datum
Numspanset_shift_scale(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum width = PG_GETARG_DATUM(2);
  SpanSet *result = numspanset_shift_scale(ss, shift, width, true, true);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Tstzspanset_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span set shifted and scaled by two intervals
 * @sqlfn shiftTscale()
 */
Datum
Tstzspanset_shift_scale(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  SpanSet *result = tstzspanset_shift_scale(ss, shift, duration);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Floatspanset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspanset_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span set with the precision of the values set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Floatspanset_round(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  int maxdd = PG_GETARG_INT32(1);
  SpanSet *result = floatspanset_rnd(ss, maxdd);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_cmp);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal to, or greater than the second one
 * @sqlfn spanset_cmp()
 */
Datum
Spanset_cmp(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  int cmp = spanset_cmp_int(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_INT32(cmp);
}

PGDLLEXPORT Datum Spanset_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_eq);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span set is equal to the second one
 * @sqlfn spanset_eq()
 * @sqlop @p =
 */
Datum
Spanset_eq(PG_FUNCTION_ARGS)
{
  SpanSet *ss1 = PG_GETARG_SPANSET_P(0);
  SpanSet *ss2 = PG_GETARG_SPANSET_P(1);
  bool result = spanset_eq_int(ss1, ss2);
  PG_FREE_IF_COPY(ss1, 0);
  PG_FREE_IF_COPY(ss2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Spanset_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_ne);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span set is different from the second one
 * @sqlfn spanset_ne()
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
 * @brief Return true if the first span set is less than the second one
 * @sqlfn spanset_lt()
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
 * @brief Return true if the first span set is less than or equal to
 * the second one
 * @sqlfn spanset_le()
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
 * @brief Return true if the first span set is greater than or equal to
 * the second one
 * @sqlfn spanset_ge()
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
 * @brief Return true if the first span set is greater than the second one
 * @sqlfn spanset_gt()
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
 * Functions for defining hash indexes
 * The functions reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

PGDLLEXPORT Datum Spanset_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_hash);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 32-bit hash value of a span set
 * @sqlfn spanset_hash()
 */
Datum
Spanset_hash(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  uint32 result = spanset_hash(ss);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Spanset_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_hash_extended);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 64-bit hash value of a span set using a seed
 * @sqlfn spanset_hash_extended()
 */
Datum
Spanset_hash_extended(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = spanset_hash_extended(ss, seed);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
