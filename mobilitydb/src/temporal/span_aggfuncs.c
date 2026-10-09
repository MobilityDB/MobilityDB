/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"

/*****************************************************************************/

PGDLLEXPORT Datum Span_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of spans
 * @sqlfn span_extent_transfn()
 * @sqlaggfn extent()
 */
Datum
Span_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  /* Outside an aggregate the state is a value of the caller, which is
   * expanded in a copy */
  if (s1 && ! AggCheckCallContext(fcinfo, NULL))
    s1 = span_copy(s1);
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
 * @sqlaggfn extent()
 */
Datum
Span_extent_combinefn(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  /* Outside an aggregate the state is a value of the caller, which is
   * expanded in a copy */
  if (s1 && ! AggCheckCallContext(fcinfo, NULL))
    s1 = span_copy(s1);
  Span *s2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPAN_P(1);
  Span *result = span_extent_transfn(s1, s2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Spanbase_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanbase_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of span base values
 * @sqlfn span_extent_transfn()
 * @sqlaggfn extent()
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
  MeosType basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, 1));
  /* Outside an aggregate the state is a value of the caller, which is
   * expanded in a copy */
  if (s && ! AggCheckCallContext(fcinfo, NULL))
    s = span_copy(s);
  PG_RETURN_SPAN_P(spanbase_extent_transfn(s, value, basetype));
}

PGDLLEXPORT Datum Set_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_extent_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for extent aggregation of sets
 * @sqlfn set_extent_transfn()
 * @sqlaggfn extent()
 */
Datum
Set_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *span = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  /* Outside an aggregate the state is a value of the caller, which is
   * expanded in a copy */
  if (span && ! AggCheckCallContext(fcinfo, NULL))
    span = span_copy(span);
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
 * @sqlfn spanset_extent_transfn()
 * @sqlaggfn extent()
 */
Datum
Spanset_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *s = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  /* Outside an aggregate the state is a value of the caller, which is
   * expanded in a copy */
  if (s && ! AggCheckCallContext(fcinfo, NULL))
    s = span_copy(s);
  SpanSet *ss = PG_ARGISNULL(1) ? NULL : PG_GETARG_SPANSET_P(1);
  s = spanset_extent_transfn(s, ss);
  PG_FREE_IF_COPY(ss, 1);
  if (! s)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(s);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_union_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for union aggregation of spans
 * @sqlfn span_union_transfn()
 * @sqlaggfn spanUnion()
 */
Datum
Span_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Span_union_transfn called in non-aggregate context");
  SpanSet *state = PG_ARGISNULL(0) ? NULL : (SpanSet *) PG_GETARG_POINTER(0);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Span *s = PG_GETARG_SPAN_P(1);
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = span_union_transfn(state, s);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Spanset_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_union_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for union aggregation of span sets
 * @sqlfn spanset_union_transfn()
 * @sqlaggfn spansetUnion()
 */
Datum
Spanset_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Spanset_union_transfn called in non-aggregate context");
  SpanSet *state = PG_ARGISNULL(0) ? NULL : (SpanSet *) PG_GETARG_POINTER(0);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    SpanSet *ss = PG_GETARG_SPANSET_P(1);
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = spanset_union_transfn(state, ss);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Spanset_union_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_union_combinefn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Combine function for union aggregation of spans and span sets
 * @sqlfn spanset_union_combinefn()
 * @sqlaggfn spanUnion(), spansetUnion()
 */
Datum
Spanset_union_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Spanset_union_combinefn called in non-aggregate context");
  SpanSet *state1 = PG_ARGISNULL(0) ? NULL : (SpanSet *) PG_GETARG_POINTER(0);
  SpanSet *state2 = PG_ARGISNULL(1) ? NULL : (SpanSet *) PG_GETARG_POINTER(1);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  SpanSet *result = spanset_union_combinefn(state1, state2);
  MemoryContextSwitchTo(oldctx);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_union_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_union_finalfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Final function for union aggregation of spans and span sets
 * @sqlfn intspan_union_finalfn(), floatspan_union_finalfn(), ...
 * @sqlaggfn spanUnion(), spansetUnion()
 */
Datum
Span_union_finalfn(PG_FUNCTION_ARGS)
{
  /* Return NULL if we had zero inputs, like other aggregates */
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  SpanSet *state = (SpanSet *) PG_GETARG_POINTER(0);
  SpanSet *result = spanset_union_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPANSET_P(result);
}

PGDLLEXPORT Datum Spansetstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spansetstate_serialize);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Serialize the state of a span or span set union aggregate
 * @sqlfn spansetstate_serialize()
 * @sqlaggfn spanUnion(), spansetUnion()
 */
Datum
Spansetstate_serialize(PG_FUNCTION_ARGS)
{
  SpanSet *state = (SpanSet *) PG_GETARG_POINTER(0);
  size_t size;
  uint8_t *wkb = spansetstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), wkb, size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Spansetstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spansetstate_deserialize);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Deserialize the state of a span or span set union aggregate
 * @sqlfn spansetstate_deserialize()
 * @sqlaggfn spanUnion(), spansetUnion()
 */
Datum
Spansetstate_deserialize(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Spansetstate_deserialize called in non-aggregate context");
  bytea *data = PG_GETARG_BYTEA_P(0);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  SpanSet *result = spansetstate_deserialize((uint8_t *) VARDATA(data),
    VARSIZE(data) - VARHDRSZ);
  MemoryContextSwitchTo(oldctx);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

