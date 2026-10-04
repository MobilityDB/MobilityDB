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
 * @brief Aggregate functions for set types
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

/**
 * @brief Type of the values aggregated by a set union, kept in fn_extra
 */
typedef struct
{
  MeosType basetype;  /**< Base type of the values */
  bool varlength;     /**< True when the values are of variable length */
} SetValueType;

PGDLLEXPORT Datum Value_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_union_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for union aggregation of values
 * @sqlfn set_union_transfn()
 * @sqlaggfn setUnion()
 */
Datum
Value_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Value_union_transfn called in non-aggregate context");
  Set *state = PG_ARGISNULL(0) ? NULL : (Set *) PG_GETARG_POINTER(0);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    /* The type of the values is read once per query and kept in fn_extra,
     * as #get_srs_cache_by_srid keeps its SRS */
    SetValueType *cache = fcinfo->flinfo->fn_extra;
    if (! cache)
    {
      cache = MemoryContextAlloc(fcinfo->flinfo->fn_mcxt, sizeof(SetValueType));
      cache->basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, 1));
      cache->varlength = basetype_varlength(cache->basetype);
      fcinfo->flinfo->fn_extra = cache;
    }
    assert(set_basetype(cache->basetype));
    Datum value = PG_GETARG_DATUM(1);
    /* The state keeps the bytes of the value, so a toasted one is read first;
     * a value with a 1-byte header is stored with a 4-byte one by the state */
    if (cache->varlength)
      value = PointerGetDatum(PG_DETOAST_DATUM_PACKED(value));
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = value_union_transfn(state, value, cache->basetype);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Set_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_union_transfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Transition function for union aggregation of sets
 * @sqlfn set_union_transfn()
 * @sqlaggfn setUnion()
 */
Datum
Set_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Set_union_transfn called in non-aggregate context");
  Set *state = PG_ARGISNULL(0) ? NULL : (Set *) PG_GETARG_POINTER(0);
  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
  {
    Set *s = PG_GETARG_SET_P(1);
    MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
    state = set_union_transfn(state, s);
    MemoryContextSwitchTo(oldctx);
  }
  if (! state)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Set_union_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_union_combinefn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Combine function for union aggregation of sets
 * @sqlfn set_union_combinefn()
 * @sqlaggfn setUnion()
 */
Datum
Set_union_combinefn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Set_union_combinefn called in non-aggregate context");
  Set *state1 = PG_ARGISNULL(0) ? NULL : (Set *) PG_GETARG_POINTER(0);
  Set *state2 = PG_ARGISNULL(1) ? NULL : (Set *) PG_GETARG_POINTER(1);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  Set *result = set_union_combinefn(state1, state2);
  MemoryContextSwitchTo(oldctx);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Set_union_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_union_finalfn);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Final function for union aggregation of sets
 * @sqlfn intset_union_finalfn(), floatset_union_finalfn(), ...
 * @sqlaggfn setUnion()
 */
Datum
Set_union_finalfn(PG_FUNCTION_ARGS)
{
  /* Return NULL if we had zero inputs, like other aggregates */
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  Set *state = (Set *) PG_GETARG_POINTER(0);
  Set *result = set_union_finalfn(state);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Setstate_serialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Setstate_serialize);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Serialize the state of a set union aggregate
 * @sqlfn setstate_serialize()
 * @sqlaggfn setUnion()
 */
Datum
Setstate_serialize(PG_FUNCTION_ARGS)
{
  Set *state = (Set *) PG_GETARG_POINTER(0);
  size_t size;
  uint8_t *wkb = setstate_serialize(state, &size);
  bytea *result = palloc(VARHDRSZ + size);
  SET_VARSIZE(result, VARHDRSZ + size);
  memcpy(VARDATA(result), wkb, size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Setstate_deserialize(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Setstate_deserialize);
/**
 * @ingroup mobilitydb_setspan_agg
 * @brief Deserialize the state of a set union aggregate
 * @sqlfn setstate_deserialize()
 * @sqlaggfn setUnion()
 */
Datum
Setstate_deserialize(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Setstate_deserialize called in non-aggregate context");
  bytea *data = PG_GETARG_BYTEA_P(0);
  MemoryContext oldctx = MemoryContextSwitchTo(aggContext);
  Set *result = setstate_deserialize((uint8_t *) VARDATA(data),
    VARSIZE(data) - VARHDRSZ);
  MemoryContextSwitchTo(oldctx);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
