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
 * @brief Aggregate function for span types.
 */

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"
#include "general/set.h"
#include "general/span.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_extent_transfn);
/**
 * @brief Transition function for extent aggregation of span values
 */
PGDLLEXPORT Datum
Span_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  Span *result = span_extent_transfn(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Spanbase_extent_transfn);
/**
 * @brief Transition function for extent aggregation of base values of span types
 */
PGDLLEXPORT Datum
Spanbase_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  if (PG_ARGISNULL(1))
    PG_RETURN_POINTER(s);
  Datum d = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  s = spanbase_extent_transfn(s, d, basetype);
  if (! s)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(s);
}

PG_FUNCTION_INFO_V1(Span_extent_combinefn);
/**
 * @brief Combine function for temporal extent aggregation
 */
PGDLLEXPORT Datum
Span_extent_combinefn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  if (! s2 && ! s1)
    PG_RETURN_NULL();
  if (s1 && ! s2)
    PG_RETURN_POINTER(s1);
  if (s2 && ! s1)
    PG_RETURN_POINTER(s2);
  /* Non-strict union */
  Span *result = bbox_union_span_span(s1, s2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Set_extent_transfn);
/**
 * @brief Transition function for extent aggregation of set values
 */
PGDLLEXPORT Datum
Set_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *span = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Set *set = PG_ARGISNULL(1) ? NULL : PG_GETARG_SET_P(1);
  span = set_extent_transfn(span, set);
  PG_FREE_IF_COPY(set, 1);
  if (! span)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(span);
}

PG_FUNCTION_INFO_V1(Spanset_extent_transfn);
/**
 * @brief Transition function for extent aggregation of span set values
 */
PGDLLEXPORT Datum
Spanset_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPANSET_P(1);
  s = spanset_extent_transfn(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! s)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(s);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_agg_transfn);
/*
 * @brief Transition function for aggregating spans
 *
 * All we do here is gather the input spans into an array
 * so that the finalfn can sort and combine them.
 */
PGDLLEXPORT Datum
Span_agg_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  ArrayBuildState *state;

  if (!AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "span_agg_transfn called in non-aggregate context");

  Oid spanoid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  meosType spantype = oid_type(spanoid);
  ensure_span_type(spantype);

  if (PG_ARGISNULL(0))
    state = initArrayResult(spanoid, aggContext, false);
  else
    state = (ArrayBuildState *) PG_GETARG_POINTER(0);

  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
    accumArrayResult(state, PG_GETARG_DATUM(1), false, spanoid, aggContext);

  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Spanset_agg_transfn);
/*
 * @brief Transition function for aggregating spans
 *
 * All we do here is gather the input span sets' spans into an array so
 * that the finalfn can sort and combine them.
 */
PGDLLEXPORT Datum
Spanset_agg_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (!AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Spanset_agg_transfn called in non-aggregate context");

  Oid spansetoid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  meosType spansettype = oid_type(spansetoid);
  ensure_spanset_type(spansettype);
  meosType spantype = spansettype_spantype(spansettype);
  Oid spanoid = type_oid(spantype);

  ArrayBuildState *state;
  if (PG_ARGISNULL(0))
    state = initArrayResult(spanoid, aggContext, false);
  else
    state = (ArrayBuildState *) PG_GETARG_POINTER(0);

  /* skip NULLs */
  if (! PG_ARGISNULL(1) )
  {
    SpanSet *ss = PG_GETARG_SPANSET_P(1);
    const Span **spans = spanset_spans(ss);
    for (int i = 0; i < ss->count; i++)
      accumArrayResult(state, SpanPGetDatum(spans[i]), false, spanoid,
        aggContext);
  }
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Span_agg_finalfn);
/*
 * @brief use our internal array to merge overlapping/touching spans.
 * @note Shared by span_agg_finalfn() and spanset_agg_finalfn().
 */
PGDLLEXPORT Datum
Span_agg_finalfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Span_agg_finalfn called in non-aggregate context");

  ArrayBuildState *state = PG_ARGISNULL(0) ? NULL :
    (ArrayBuildState *) PG_GETARG_POINTER(0);
  if (state == NULL)
    /* This shouldn't be possible, but just in case.... */
    PG_RETURN_NULL();

  /* Also return NULL if we had zero inputs, like other aggregates */
  int32 count = state->nelems;
  if (count == 0)
    PG_RETURN_NULL();

  Span **spans = palloc0(count * sizeof(Span *));
  for (int i = 0; i < count; i++)
    spans[i] = DatumGetSpanP(state->dvalues[i]);

  int newcount;
  Span **normspans = spanarr_normalize(spans, count, true, &newcount);
  SpanSet *result = spanset_make_free(normspans, newcount, NORMALIZE_NO);

  /* Free memory */
  pfree_array((void **) spans, count);

  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

