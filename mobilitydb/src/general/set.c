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
 * @brief General functions for set types composed of an ordered list of
 * distinct values
 */

#include "general/set.h"

/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <access/heaptoast.h>
  #include <access/detoast.h>
#else
  #include <access/tuptoaster.h>
#endif
#include <funcapi.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Set_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_in);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Input function for set types
 * @sqlfn intset_in(), bigintset_in(), floatset_in(), tstzset_in()
 */
Datum
Set_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid ostypid = PG_GETARG_OID(1);
  Set *result = set_in(input, oid_type(ostypid));
  PG_RETURN_POINTER(result);
} 

PGDLLEXPORT Datum Set_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Output function for set types
 * @sqlfn intset_out(), bigintset_out(), floatset_out(), tstzset_out()
 */
Datum
Set_out(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  char *result = set_out(s, Int32GetDatum(OUT_DEFAULT_DECIMAL_DIGITS));
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Set_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_recv);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Receive function for set types
 * @sqlfn intset_recv(), bigintset_recv(), floatset_recv(), tstzset_recv()
 */
Datum
Set_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Set *result = set_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Set_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_send);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Send function for set types
 * @sqlfn intset_send(), bigintset_send(), floatset_send(), tstzset_send()
 */
Datum
Set_send(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  uint8_t variant = WKB_EXTENDED;
  size_t wkb_size = VARSIZE_ANY_EXHDR(s);
  uint8_t *wkb = set_as_wkb(s, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

PGDLLEXPORT Datum Set_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_constructor);
/**
 * @ingroup mobilitydb_setspan_constructor
 * @brief Construct a set from an array of base values
 * @sqlfn set()
 */
Datum
Set_constructor(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  ensure_not_empty_array(array);
  meosType settype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  int count;
  Datum *values = datumarr_extract(array, &count);
  meosType basetype = settype_basetype(settype);
  Set *result = set_make_free(values, count, basetype, ORDERED_NO);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Value_to_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_to_set);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a base value to a set
 * @sqlfn set()
 */
Datum
Value_to_set(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  /* Detoast the value if necessary */
  if (basetype_varlength(basetype))
    d = PointerGetDatum(PG_DETOAST_DATUM(d));
  Set *result = value_to_set(d, basetype);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Set_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a set to a span
 * @sqlfn span()
 */
Datum
Set_to_span(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Span *result = set_to_span(s);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intset_to_floatset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intset_to_floatset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert an integer set to a float set
 * @sqlfn floatset()
 * @sqlop @p ::
 */
Datum
Intset_to_floatset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = intset_to_floatset(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Floatset_to_intset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatset_to_intset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a float set to a integer set
 * @sqlfn intset()
 * @sqlop @p ::
 */
Datum
Floatset_to_intset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = floatset_to_intset(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Dateset_to_tstzsset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Dateset_to_tstzset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a date set to a timestamptz set
 * @sqlfn tstzset()
 * @sqlop @p ::
 */
Datum
Dateset_to_tstzset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = dateset_to_tstzset(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzset_to_dateset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_to_dateset);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a timestamptz set to a date set
 * @sqlfn dateset()
 * @sqlop @p ::
 */
Datum
Tstzset_to_dateset(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = tstzset_to_dateset(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Set_mem_size(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_mem_size);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the memory size in bytes of a set
 * @sqlfn memSize()
 */
Datum
Set_mem_size(PG_FUNCTION_ARGS)
{
  Datum result = toast_raw_datum_size(PG_GETARG_DATUM(0));
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Set_num_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_num_values);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the number of values of a set
 * @sqlfn numValues()
 */
Datum
Set_num_values(PG_FUNCTION_ARGS)
{
  Set *ts = PG_GETARG_SET_P(0);
  int result = set_num_values(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Set_start_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_start_value);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the start value of a set
 * @sqlfn startValue()
 */
Datum
Set_start_value(PG_FUNCTION_ARGS)
{
  Set *ts = PG_GETARG_SET_P(0);
  TimestampTz result = set_start_value(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Set_end_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_end_value);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the end value of a set
 * @sqlfn endValue()
 */
Datum
Set_end_value(PG_FUNCTION_ARGS)
{
  Set *ts = PG_GETARG_SET_P(0);
  TimestampTz result = set_end_value(ts);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Set_value_n(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_value_n);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the n-th value of a set
 * @sqlfn valueN()
 */
Datum
Set_value_n(PG_FUNCTION_ARGS)
{
  Set *ts = PG_GETARG_SET_P(0);
  int n = PG_GETARG_INT32(1); /* Assume 1-based */
  Datum result;
  bool found = set_value_n(ts, n, &result);
  PG_FREE_IF_COPY(ts, 0);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Set_values(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_values);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the array of values of a set
 * @sqlfn getValues()
 */
Datum
Set_values(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum *values = set_values(s);
  ArrayType *result = datumarr_to_array(values, s->count, s->basetype);
  pfree(values);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Geoset_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_get_srid);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the SRID of a geo set
 * @sqlfn SRID()
 */
Datum
Geoset_get_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int result = geoset_srid(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Geoset_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geoset_set_srid);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Set the SRID of a geo set
 * @sqlfn setSRID()
 */
Datum
Geoset_set_srid(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Set *result = geoset_set_srid(s, srid);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Numset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number set shifted by a value
 * @sqlfn shift()
 */
Datum
Numset_shift(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Set *result = numset_shift_scale(s, shift, 0, true, false);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzset_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz set shifted by an interval
 * @sqlfn shift()
 */
Datum
Tstzset_shift(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Set *result = tstzset_shift_scale(s, shift, NULL);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Numset_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numset_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number set scaled by a value
 * @sqlfn scale()
 */
Datum
Numset_scale(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum width = PG_GETARG_DATUM(1);
  Set *result = numset_shift_scale(s, 0, width, false, true);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzset_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz set scaled by an interval
 * @sqlfn scale()
 */
Datum
Tstzset_scale(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Set *result = tstzset_shift_scale(s, NULL, duration);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Numset_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numset_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number set shifted and scaled by the values
 * @sqlfn shiftScale()
 */
Datum
Numset_shift_scale(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum width = PG_GETARG_DATUM(2);
  Set *result = numset_shift_scale(s, shift, width, true, true);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzset_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz set shifted and scaled by the intervals
 * @sqlfn shiftScale()
 */
Datum
Tstzset_shift_scale(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Set *result = tstzset_shift_scale(s, shift, duration);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Floatset_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatset_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float set where the precision of the values is set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Floatset_round(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int maxdd = PG_GETARG_INT32(1);
  Set *result = floatset_round(s, maxdd);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Textset_lower(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textset_lower);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a text set where the values are in lowercase
 * @sqlfn lower()
 */
Datum
Textset_lower(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = textset_lower(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Textset_upper(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Textset_upper);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a text set where the values are in uppercase
 * @sqlfn upper()
 */
Datum
Textset_upper(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Set *result = textset_upper(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Unnest function
 *****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] set Set value
 * @param[in] values Array of values appearing in the temporal value
 * @param[in] count Number of elements in the input array
 */
SetUnnestState *
set_unnest_state_make(const Set *set, Datum *values, int count)
{
  SetUnnestState *state = palloc0(sizeof(SetUnnestState));
  /* Fill in state */
  state->done = false;
  state->i = 0;
  state->count = count;
  state->values = values;
  state->set = set_cp(set);
  return state;
}

/**
 * @brief Increment the current state to the next unnest value
 * @param[in] state State to increment
 */
void
set_unnest_state_next(SetUnnestState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

PGDLLEXPORT Datum Set_unnest(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_unnest);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return the list of values of a set
 */
Datum
Set_unnest(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  SetUnnestState *state;

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Get input parameters */
    Set *set = PG_GETARG_SET_P(0);
    /* Create function state */
    Datum *values = set_values(set);
    funcctx->user_fctx = set_unnest_state_make(set, values, set->count);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all buckets */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    // pfree(state->values);
    // pfree(state->set);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Get value */
  Datum result = state->values[state->i];
  /* Advance state */
  set_unnest_state_next(state);
  /* Return */
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Set_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_cmp);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first set is less than,
 * equal to, or greater than the second one
 * @sqlfn set_cmp()
 */
Datum
Set_cmp(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  int cmp = set_cmp(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_INT32(cmp);
}

PGDLLEXPORT Datum Set_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_eq);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is equal to the second one
 * @sqlfn set_eq()
 * @sqlop @p =
 */
Datum
Set_eq(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_eq(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Set_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_ne);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is different from the second one
 * @sqlfn set_ne()
 * @sqlop @p <>
 */
Datum
Set_ne(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_ne(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Set_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_lt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is less than the second one
 * @sqlfn set_lt()
 * @sqlop @p <
 */
Datum
Set_lt(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_lt(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Set_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_le);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is less than or equal to the second one
 * @sqlfn set_le()
 * @sqlop @p <=
 */
Datum
Set_le(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_le(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Set_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_ge);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is greater than or equal to the second
 * one
 * @sqlfn set_ge()
 * @sqlop @p >=
 */
Datum
Set_ge(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_ge(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Set_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_gt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first set is greater than the second one
 * @sqlfn set_gt()
 * @sqlop @p >
 */
Datum
Set_gt(PG_FUNCTION_ARGS)
{
  Set *s1 = PG_GETARG_SET_P(0);
  Set *s2 = PG_GETARG_SET_P(1);
  bool result = set_gt(s1, s2);
  PG_FREE_IF_COPY(s1, 0);
  PG_FREE_IF_COPY(s2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Function for defining hash indexes
 *****************************************************************************/

PGDLLEXPORT Datum Set_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_hash);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 32-bit hash value of a set
 * @sqlfn hash()
 */
Datum
Set_hash(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  uint32 result = set_hash(s);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Set_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_hash_extended);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 64-bit hash value of a set using a seed
 * @sqlfn hash_extended()
 */
Datum
Set_hash_extended(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = set_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
