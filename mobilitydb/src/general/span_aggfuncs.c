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
 * @brief Aggregate function for span types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/span.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************/

PGDLLEXPORT Datum Span_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of spans
 * @sqlfn extent()
 */
Datum
Span_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  Span *result = span_extent_transfn(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Span_extent_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_extent_combinefn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Combine function for extent aggregation of spans
 * @sqlfn extent()
 */
Datum
Span_extent_combinefn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  if (! s2 && ! s1)
    PG_RETURN_NULL();
  if (s1 && ! s2)
    PG_RETURN_SPAN_P(s1);
  if (s2 && ! s1)
    PG_RETURN_SPAN_P(s2);
  /* Non-strict union */
  PG_RETURN_SPAN_P(super_union_span_span(s1, s2));
}

/*****************************************************************************/

PGDLLEXPORT Datum Spanbase_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanbase_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of span base values
 * @sqlfn extent()
 */
Datum
Spanbase_extent_transfn(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) && PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Span *s = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  if (PG_ARGISNULL(1))
    PG_RETURN_SPAN_P(s);
  Datum value = PG_GETARG_DATUM(1);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  PG_RETURN_SPAN_P(spanbase_extent_transfn(s, value, basetype));
}

PGDLLEXPORT Datum Set_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of sets
 * @sqlfn extent()
 */
Datum
Set_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *span = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Set *set = PG_ARGISNULL(1) ? NULL : PG_GETARG_SET_P(1);
  span = set_extent_transfn(span, set);
  PG_FREE_IF_COPY(set, 1);
  if (! span)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(span);
}

PGDLLEXPORT Datum Spanset_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of span sets
 * @sqlfn extent()
 */
Datum
Spanset_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  SpanSet *ss = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPANSET_P(1);
  s = spanset_extent_transfn(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! s)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(s);
}

/*****************************************************************************/

/*
 * The transition and combine functions for span_union are, respectively,
 * PostgreSQL's array_agg_transfn and array_agg_combinefn. Similarly, the
 * combine function for spanset_union is PostgreSQL's array_agg_combinefn.
 * The idea is that all the component spans are simply appened to an array
 * without any processing and thus are not sorted. The final function then
 * extract the spans, sort them, and performs the normalization.
 * Reusing PostgreSQL array function enables us to leverage parallel aggregates
 * (introduced in PostgreSQL version 16) and other built-in optimizations.
 */

PGDLLEXPORT Datum Spanset_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_union_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for union aggregation of span sets
 * @note We simply gather the input values into an array so that the final
 * function can sort and combine them
 * @sqlfn union()
 */
Datum
Spanset_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Spanset_union_transfn called in non-aggregate context");

  Oid spansetoid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  meosType spansettype = oid_type(spansetoid);
  assert(spanset_type(spansettype));
  meosType spantype = spansettype_spantype(spansettype);
  Oid spanoid = type_oid(spantype);

  ArrayBuildState *state;
  if (PG_ARGISNULL(0))
    state = initArrayResult(spanoid, aggContext, false);
  else
    state = (ArrayBuildState *) PG_GETARG_POINTER(0);

  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    SpanSet *ss = PG_GETARG_SPANSET_P(1);
    for (int i = 0; i < ss->count; i++)
      accumArrayResult(state, SpanPGetDatum(SPANSET_SP_N(ss, i)), false,
        spanoid, aggContext);
  }
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Span_union_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_union_finalfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Final function for union aggregation of spans.
 * @note Shared for both spans and span sets
 * @sqlfn union()
 */
Datum
Span_union_finalfn(PG_FUNCTION_ARGS)
{
  /* cannot be called directly because of internal-type argument */
  Assert(AggCheckCallContext(fcinfo, NULL));
  // MemoryContext aggContext;
  // if (! AggCheckCallContext(fcinfo, &aggContext))
    // elog(ERROR, "Span_union_finalfn called in non-aggregate context");

  ArrayBuildState *state = PG_ARGISNULL(0) ? NULL :
    (ArrayBuildState *) PG_GETARG_POINTER(0);
  if (state == NULL)
    /* This shouldn't be possible, but just in case.... */
    PG_RETURN_NULL();

  /* Also return NULL if we had zero inputs, like other aggregates */
  int32 count = state->nelems;
  if (count == 0)
    PG_RETURN_NULL();

  Span *spans = palloc0(sizeof(Span) * count);
  int k = 0;
  for (int i = 0; i < count; i++)
  {
    if (! state->dnulls[i])
      spans[k++] = *(DatumGetSpanP(state->dvalues[i]));
  }

  /* Also return NULL if we had only null inputs */
  if (k == 0)
    PG_RETURN_NULL();

  PG_RETURN_SPANSET_P(spanset_make_free(spans, k, NORMALIZE, ORDERED_NO));
}

/*****************************************************************************/

