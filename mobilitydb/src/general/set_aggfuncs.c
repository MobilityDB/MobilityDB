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
 * @brief Aggregate functions for set types.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/set.h"
#include "general/type_util.h"
#include "general/skiplist.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Aggregate functions for set types
 *****************************************************************************/

PGDLLEXPORT Datum Value_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_union_transfn);
/*
 * @brief Transition function for aggregating spans
 *
 * All we do here is gather the input spans into an array
 * so that the finalfn can sort and combine them.
 */
Datum
Value_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Value_union_transfn called in non-aggregate context");

  Oid valueoid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  assert(set_basetype(oid_type(valueoid)));

  ArrayBuildState *state;
  if (PG_ARGISNULL(0))
    state = initArrayResult(valueoid, aggContext, false);
  else
    state = (ArrayBuildState *) PG_GETARG_POINTER(0);

  /* Skip NULLs */
  if (! PG_ARGISNULL(1))
    accumArrayResult(state, PG_GETARG_DATUM(1), false, valueoid, aggContext);

  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Set_union_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_union_transfn);
/*
 * @brief Transition function for aggregating spans
 *
 * All we do here is gather the input span sets' spans into an array so
 * that the finalfn can sort and combine them.
 */
Datum
Set_union_transfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Set_union_transfn called in non-aggregate context");

  Oid setoid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  meosType settype = oid_type(setoid);
  assert(set_type(settype));
  meosType basetype = settype_basetype(settype);
  Oid baseoid = type_oid(basetype);

  ArrayBuildState *state;
  if (PG_ARGISNULL(0))
    state = initArrayResult(baseoid, aggContext, false);
  else
    state = (ArrayBuildState *) PG_GETARG_POINTER(0);

  /* skip NULLs */
  if (! PG_ARGISNULL(1) )
  {
    Set *set = PG_GETARG_SET_P(1);
    Datum *values = set_values(set);
    for (int i = 0; i < set->count; i++)
      accumArrayResult(state, values[i], false, baseoid, aggContext);
    pfree(values);
  }
  PG_RETURN_POINTER(state);
}

PGDLLEXPORT Datum Set_union_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_union_finalfn);
/*
 * @brief use our internal array to merge overlapping/touching spans.
 * @note Shared by Span_union_finalfn() and Spanset_union_finalfn().
 */
Datum
Set_union_finalfn(PG_FUNCTION_ARGS)
{
  MemoryContext aggContext;
  if (! AggCheckCallContext(fcinfo, &aggContext))
    elog(ERROR, "Set_union_finalfn called in non-aggregate context");

  ArrayBuildState *state = PG_ARGISNULL(0) ? NULL :
    (ArrayBuildState *) PG_GETARG_POINTER(0);
  if (state == NULL)
    /* This shouldn't be possible, but just in case.... */
    PG_RETURN_NULL();

  /* Also return NULL if we had zero inputs, like other aggregates */
  int32 count = state->nelems;
  if (count == 0)
    PG_RETURN_NULL();

  Oid setoid = get_fn_expr_rettype(fcinfo->flinfo);
  meosType settype = oid_type(setoid);
  meosType basetype = settype_basetype(settype);
  bool typbyval = basetype_byvalue(basetype);
  int16 typlen = basetype_length(basetype);

  Datum *values = palloc0(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    values[i] = typlen > 0 ? state->dvalues[i] :
      PointerGetDatum(PG_DETOAST_DATUM(state->dvalues[i]));

  Set *result = set_make_exp(values, count, count, basetype, ORDERED);

  /* Free memory */
  if (typbyval)
    pfree(values);
  else
    pfree_array((void **) values, count);

  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
