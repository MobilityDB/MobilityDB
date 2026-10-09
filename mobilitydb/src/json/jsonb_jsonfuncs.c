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
 * @brief JSON functions for JSONB values
 * @details PostgreSQL provides these operations under its own names; the
 * functions of this file give them the names a query uses in every engine,
 * the names of their JSONB set and temporal JSONB twins with the `jsonb`
 * prefix
 */

/* C */
#include <stdbool.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include "access/htup_details.h"
#include "utils/array.h"
#include "utils/jsonb.h"
#include "utils/jsonpath.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include <pgtypes.h>
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Helper functions
 *****************************************************************************/

/**
 * @brief Return the text elements of a one-dimensional text array, and their
 * number in the last argument
 */
static text **
textarr_elems(ArrayType *array, int *count)
{
  if (ARR_NDIM(array) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
       errmsg("wrong number of array subscripts")));
  Datum *elems;
  bool *nulls;
  deconstruct_array(array, TEXTOID, -1, false, 'i', &elems, &nulls, count);
  pfree(nulls);
  return (text **) elems;
}

/**
 * @brief State of a set-returning function over the values a JSONB value
 * yields: the values, the keys paired with them when there are any, their
 * number and the next one to return
 */
typedef struct
{
  Datum *values;
  Datum *keys;
  int count;
  int i;
} JsonbSrfState;

/**
 * @brief Return the state of a set-returning function over an array of
 * values and, when not NULL, an array of keys paired with them, allocated in
 * the memory context of the function calls
 */
static JsonbSrfState *
jsonb_srf_state(FuncCallContext *funcctx, void **values, void **keys,
  int count)
{
  MemoryContext oldcontext =
    MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
  JsonbSrfState *state = palloc0(sizeof(JsonbSrfState));
  state->values = palloc(sizeof(Datum) * Max(count, 1));
  state->keys = keys ? palloc(sizeof(Datum) * Max(count, 1)) : NULL;
  for (int i = 0; i < count; i++)
  {
    state->values[i] = PointerGetDatum(values[i]);
    if (keys)
      state->keys[i] = PointerGetDatum(keys[i]);
  }
  state->count = count;
  MemoryContextSwitchTo(oldcontext);
  return state;
}

/**
 * @brief Make the tuple descriptor of a set-returning function returning
 * records of a key and a value
 */
static void
jsonb_srf_tupdesc(FunctionCallInfo fcinfo, FuncCallContext *funcctx)
{
  MemoryContext oldcontext =
    MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
  TupleDesc tupdesc;
  if (get_call_result_type(fcinfo, 0, &tupdesc) != TYPEFUNC_COMPOSITE)
    ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
      errmsg("function returning record called in context "
             "that cannot accept type record")));
  funcctx->tuple_desc = BlessTupleDesc(tupdesc);
  MemoryContextSwitchTo(oldcontext);
}

/**
 * @brief Return the next value of a set-returning function, a record of a key
 * and a value when the state pairs keys with the values
 */
static Datum
jsonb_srf_next(FunctionCallInfo fcinfo)
{
  FuncCallContext *funcctx = SRF_PERCALL_SETUP();
  JsonbSrfState *state = funcctx->user_fctx;
  if (state->i >= state->count)
    SRF_RETURN_DONE(funcctx);
  Datum result;
  if (state->keys)
  {
    /* A JSON null read as text is an SQL NULL */
    Datum values[2] = { state->keys[state->i], state->values[state->i] };
    bool isnull[2] = { false, DatumGetPointer(values[1]) == NULL };
    HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
    result = HeapTupleGetDatum(tuple);
  }
  else
  {
    result = state->values[state->i];
    if (DatumGetPointer(result) == NULL)
    {
      state->i++;
      SRF_RETURN_NEXT_NULL(funcctx);
    }
  }
  state->i++;
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************
 * Accessors
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_array_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_array_length);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the number of elements of a JSONB array
 * @sqlfn jsonbArrayLength()
 */
Datum
Jsonb_array_length(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  int result = pg_jsonb_array_length(jb);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Jsonb_object_field(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_object_field);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the field of a JSONB object with a key
 * @sqlfn jsonbObjectField()
 */
Datum
Jsonb_object_field(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  text *key = PG_GETARG_TEXT_PP(1);
  Jsonb *result = pg_jsonb_object_field(jb, key);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_object_field_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_object_field_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the field of a JSONB object with a key as text
 * @sqlfn jsonbObjectFieldText()
 */
Datum
Jsonb_object_field_text(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  text *key = PG_GETARG_TEXT_PP(1);
  text *result = pg_jsonb_object_field_text(jb, key);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Jsonb_array_element(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_array_element);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the element of a JSONB array at a position, counting from
 * the end when negative
 * @sqlfn jsonbArrayElement()
 */
Datum
Jsonb_array_element(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  int element = PG_GETARG_INT32(1);
  Jsonb *result = pg_jsonb_array_element(jb, element);
  PG_FREE_IF_COPY(jb, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_array_element_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_array_element_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the element of a JSONB array at a position as text, counting
 * from the end when negative
 * @sqlfn jsonbArrayElementText()
 */
Datum
Jsonb_array_element_text(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  int element = PG_GETARG_INT32(1);
  text *result = pg_jsonb_array_element_text(jb, element);
  PG_FREE_IF_COPY(jb, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Jsonb_extract_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_extract_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the JSONB value at a path
 * @sqlfn jsonbExtractPath()
 */
Datum
Jsonb_extract_path(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  Jsonb *result = pg_jsonb_extract_path(jb, path_elems, path_len);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_extract_path_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_extract_path_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the JSONB value at a path as text
 * @sqlfn jsonbExtractPathText()
 */
Datum
Jsonb_extract_path_text(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  text *result = pg_jsonb_extract_path_text(jb, path_elems, path_len);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Jsonb_pretty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_pretty);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the indented text of a JSONB value
 * @sqlfn jsonbPretty()
 */
Datum
Jsonb_pretty(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  text *result = pg_jsonb_pretty(jb);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Set-returning functions
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_array_elements(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_array_elements);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the elements of a JSONB array as rows
 * @sqlfn jsonbArrayElements()
 */
Datum
Jsonb_array_elements(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    int count;
    Jsonb **values = pg_jsonb_array_elements(jb, &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) values, NULL,
      count);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

PGDLLEXPORT Datum Jsonb_array_elements_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_array_elements_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the elements of a JSONB array as rows of text
 * @sqlfn jsonbArrayElementsText()
 */
Datum
Jsonb_array_elements_text(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    int count;
    text **values = pg_jsonb_array_elements_text(jb, &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) values, NULL,
      count);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

PGDLLEXPORT Datum Jsonb_each(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_each);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the fields of a JSONB object as rows of a key and a value
 * @sqlfn jsonbEach()
 */
Datum
Jsonb_each(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    jsonb_srf_tupdesc(fcinfo, funcctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    int count;
    /* The values fill an array sized by the number of fields */
    Jsonb **values = palloc(sizeof(Jsonb *) * Max(JB_ROOT_COUNT(jb), 1));
    text **keys = pg_jsonb_each(jb, values, &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) values,
      (void **) keys, count);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

PGDLLEXPORT Datum Jsonb_each_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_each_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the fields of a JSONB object as rows of a key and a text value
 * @sqlfn jsonbEachText()
 */
Datum
Jsonb_each_text(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    jsonb_srf_tupdesc(fcinfo, funcctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    int count;
    /* The values fill an array sized by the number of fields */
    text **values = palloc(sizeof(text *) * Max(JB_ROOT_COUNT(jb), 1));
    text **keys = pg_jsonb_each_text(jb, values, &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) values,
      (void **) keys, count);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

PGDLLEXPORT Datum Jsonb_object_keys(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_object_keys);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the keys of a JSONB object as rows
 * @sqlfn jsonbObjectKeys()
 */
Datum
Jsonb_object_keys(PG_FUNCTION_ARGS)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    int count;
    text **keys = pg_jsonb_object_keys(jb, &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) keys, NULL,
      count);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

/*****************************************************************************
 * Existence and containment
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_exists);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a text is a top-level key or array element of a JSONB
 * value
 * @sqlfn jsonbExists()
 */
Datum
Jsonb_exists(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  text *key = PG_GETARG_TEXT_PP(1);
  bool result = pg_jsonb_exists(jb, key);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(key, 1);
  PG_RETURN_BOOL(result);
}

/**
 * @brief Return true if any or all of the texts of an array are top-level keys
 * or array elements of a JSONB value
 */
static Datum
Jsonb_exists_array(FunctionCallInfo fcinfo, bool any)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *keys = PG_GETARG_ARRAYTYPE_P(1);
  int keys_len;
  text **keys_elems = textarr_elems(keys, &keys_len);
  bool result = pg_jsonb_exists_array(jb, keys_elems, keys_len, any);
  pfree(keys_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(keys, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_exists_any(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_exists_any);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if any of the texts of an array is a top-level key or
 * array element of a JSONB value
 * @sqlfn jsonbExistsAny()
 */
Datum
Jsonb_exists_any(PG_FUNCTION_ARGS)
{
  return Jsonb_exists_array(fcinfo, true);
}

PGDLLEXPORT Datum Jsonb_exists_all(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_exists_all);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if all the texts of an array are top-level keys or array
 * elements of a JSONB value
 * @sqlfn jsonbExistsAll()
 */
Datum
Jsonb_exists_all(PG_FUNCTION_ARGS)
{
  return Jsonb_exists_array(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_contains(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_contains);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if the first JSONB value contains the second one
 * @sqlfn jsonbContains()
 */
Datum
Jsonb_contains(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_contains(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_contained(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_contained);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if the first JSONB value is contained in the second one
 * @sqlfn jsonbContained()
 */
Datum
Jsonb_contained(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_contained(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Modifications
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_concat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_concat);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the concatenation of two JSONB values
 * @sqlfn jsonbConcat()
 */
Datum
Jsonb_concat(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  Jsonb *result = pg_jsonb_concat(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_delete(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_delete);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value without a key or the string elements equal to it
 * @sqlfn jsonbDelete()
 */
Datum
Jsonb_delete(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  text *key = PG_GETARG_TEXT_PP(1);
  Jsonb *result = pg_jsonb_delete(jb, key);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(key, 1);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_delete_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_delete_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value without the keys of an array or the string
 * elements equal to them
 * @sqlfn jsonbDeleteArray()
 */
Datum
Jsonb_delete_array(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *keys = PG_GETARG_ARRAYTYPE_P(1);
  int keys_len;
  text **keys_elems = textarr_elems(keys, &keys_len);
  Jsonb *result = pg_jsonb_delete_array(jb, keys_elems, keys_len);
  pfree(keys_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(keys, 1);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_delete_index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_delete_index);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB array without the element at a position, counting
 * from the end when negative
 * @sqlfn jsonbDeleteIndex()
 */
Datum
Jsonb_delete_index(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  int idx = PG_GETARG_INT32(1);
  Jsonb *result = pg_jsonb_delete_index(jb, idx);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_delete_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_delete_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value without the field or element at a path
 * @sqlfn jsonbDeletePath()
 */
Datum
Jsonb_delete_path(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  Jsonb *result = pg_jsonb_delete_path(jb, path_elems, path_len);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_set);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value whose item at a path is a value, the item being
 * added when it is missing and the last argument is true
 * @sqlfn jsonbSet()
 */
Datum
Jsonb_set(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  Jsonb *newjb = PG_GETARG_JSONB_P(2);
  bool create = PG_GETARG_BOOL(3);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  Jsonb *result = pg_jsonb_set(jb, path_elems, path_len, newjb, create);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  PG_FREE_IF_COPY(newjb, 2);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_set_lax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_set_lax);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value whose item at a path is a value, a NULL value
 * being treated as the last argument states
 * @details The value of a JSONB value, as #Jsonbset_set_lax is the one of a
 * JSONB set
 * @sqlfn jsonbSetLax()
 */
Datum
Jsonb_set_lax(PG_FUNCTION_ARGS)
{
  /* The value may be NULL, the other arguments make the result NULL */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(3) ||
      PG_ARGISNULL(4))
    PG_RETURN_NULL();
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  Jsonb *newjb = PG_ARGISNULL(2) ? NULL : PG_GETARG_JSONB_P(2);
  bool create = PG_GETARG_BOOL(3);
  text *handle_null = PG_GETARG_TEXT_PP(4);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  Jsonb *result = pg_jsonb_set_lax(jb, path_elems, path_len, newjb, create,
    handle_null);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_insert(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_insert);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value with a value inserted at a path
 * @sqlfn jsonbInsert()
 */
Datum
Jsonb_insert(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  Jsonb *newjb = PG_GETARG_JSONB_P(2);
  bool after = PG_GETARG_BOOL(3);
  int path_len;
  text **path_elems = textarr_elems(path, &path_len);
  Jsonb *result = pg_jsonb_insert(jb, path_elems, path_len, newjb, after);
  pfree(path_elems);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(path, 1);
  PG_FREE_IF_COPY(newjb, 2);
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_strip_nulls(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_strip_nulls);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB value without its object fields whose value is null,
 * and without its null array elements when the last argument is true
 * @sqlfn jsonbStripNulls()
 */
Datum
Jsonb_strip_nulls(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  bool strip_in_arrays = PG_GETARG_BOOL(1);
  Jsonb *result = pg_jsonb_strip_nulls(jb, strip_in_arrays);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_JSONB_P(result);
}

/*****************************************************************************
 * Path functions
 *****************************************************************************/

/**
 * @brief Read the arguments a path function takes after the JSONB value and
 * the path: the variables, NULL when absent, and whether to suppress errors
 */
static void
jsonb_path_args(FunctionCallInfo fcinfo, Jsonb **vars, bool *silent)
{
  *vars = PG_NARGS() > 2 ? PG_GETARG_JSONB_P(2) : NULL;
  *silent = PG_NARGS() > 3 ? PG_GETARG_BOOL(3) : false;
}

/**
 * @brief Return true if a JSON path returns any item for a JSONB value, NULL
 * when the path raises an error it is asked to suppress
 */
static Datum
Jsonb_path_exists_common(FunctionCallInfo fcinfo, bool tz)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars;
  bool silent;
  jsonb_path_args(fcinfo, &vars, &silent);
  int result = pg_jsonb_path_exists(jb, jp, vars, silent, tz);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result == 1);
}

PGDLLEXPORT Datum Jsonb_path_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_exists);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path returns any item for a JSONB value
 * @sqlfn jsonbPathExists()
 */
Datum
Jsonb_path_exists(PG_FUNCTION_ARGS)
{
  return Jsonb_path_exists_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_path_exists_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_exists_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path returns any item for a JSONB value,
 * comparing date and time values across time zones
 * @sqlfn jsonbPathExistsTz()
 */
Datum
Jsonb_path_exists_tz(PG_FUNCTION_ARGS)
{
  return Jsonb_path_exists_common(fcinfo, true);
}

/**
 * @brief Return the result of the predicate check of a JSON path for a JSONB
 * value
 */
static Datum
Jsonb_path_match_common(FunctionCallInfo fcinfo, bool tz)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars;
  bool silent;
  jsonb_path_args(fcinfo, &vars, &silent);
  bool result = pg_jsonb_path_match(jb, jp, vars, silent, tz);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(jp, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_path_match(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_match);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of the predicate check of a JSON path for a JSONB
 * value
 * @sqlfn jsonbPathMatch()
 */
Datum
Jsonb_path_match(PG_FUNCTION_ARGS)
{
  return Jsonb_path_match_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_path_match_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_match_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of the predicate check of a JSON path for a JSONB
 * value, comparing date and time values across time zones
 * @sqlfn jsonbPathMatchTz()
 */
Datum
Jsonb_path_match_tz(PG_FUNCTION_ARGS)
{
  return Jsonb_path_match_common(fcinfo, true);
}

/**
 * @brief Return the items a JSON path returns for a JSONB value as rows
 */
static Datum
Jsonb_path_query_common(FunctionCallInfo fcinfo, bool tz)
{
  if (SRF_IS_FIRSTCALL())
  {
    FuncCallContext *funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Jsonb *jb = PG_GETARG_JSONB_P(0);
    JsonPath *jp = PG_GETARG_JSONPATH_P(1);
    Jsonb *vars;
    bool silent;
    jsonb_path_args(fcinfo, &vars, &silent);
    int count = 0;
    Jsonb **values = pg_jsonb_path_query_all(jb, jp, vars, silent, tz,
      &count);
    funcctx->user_fctx = jsonb_srf_state(funcctx, (void **) values, NULL,
      values ? count : 0);
    MemoryContextSwitchTo(oldcontext);
  }
  return jsonb_srf_next(fcinfo);
}

PGDLLEXPORT Datum Jsonb_path_query(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the items a JSON path returns for a JSONB value as rows
 * @sqlfn jsonbPathQuery()
 */
Datum
Jsonb_path_query(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_path_query_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the items a JSON path returns for a JSONB value as rows,
 * comparing date and time values across time zones
 * @sqlfn jsonbPathQueryTz()
 */
Datum
Jsonb_path_query_tz(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_common(fcinfo, true);
}

/**
 * @brief Return the items a JSON path returns for a JSONB value as a JSONB
 * array
 */
static Datum
Jsonb_path_query_array_common(FunctionCallInfo fcinfo, bool tz)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars;
  bool silent;
  jsonb_path_args(fcinfo, &vars, &silent);
  Jsonb *result = pg_jsonb_path_query_array(jb, jp, vars, silent, tz);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_path_query_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the items a JSON path returns for a JSONB value as a JSONB
 * array
 * @sqlfn jsonbPathQueryArray()
 */
Datum
Jsonb_path_query_array(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_array_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_path_query_array_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query_array_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the items a JSON path returns for a JSONB value as a JSONB
 * array, comparing date and time values across time zones
 * @sqlfn jsonbPathQueryArrayTz()
 */
Datum
Jsonb_path_query_array_tz(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_array_common(fcinfo, true);
}

/**
 * @brief Return the first item a JSON path returns for a JSONB value, NULL
 * when it returns none
 */
static Datum
Jsonb_path_query_first_common(FunctionCallInfo fcinfo, bool tz)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars;
  bool silent;
  jsonb_path_args(fcinfo, &vars, &silent);
  Jsonb *result = pg_jsonb_path_query_first(jb, jp, vars, silent, tz);
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_JSONB_P(result);
}

PGDLLEXPORT Datum Jsonb_path_query_first(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query_first);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the first item a JSON path returns for a JSONB value
 * @sqlfn jsonbPathQueryFirst()
 */
Datum
Jsonb_path_query_first(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_first_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonb_path_query_first_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_path_query_first_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the first item a JSON path returns for a JSONB value,
 * comparing date and time values across time zones
 * @sqlfn jsonbPathQueryFirstTz()
 */
Datum
Jsonb_path_query_first_tz(PG_FUNCTION_ARGS)
{
  return Jsonb_path_query_first_common(fcinfo, true);
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_eq);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if two JSONB values are equal
 * @sqlfn eq()
 */
Datum
Jsonb_eq(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_eq(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_ne);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if two JSONB values are different
 * @sqlfn ne()
 */
Datum
Jsonb_ne(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_ne(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_lt);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if the first JSONB value is less than the second one
 * @sqlfn lt()
 */
Datum
Jsonb_lt(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_lt(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_le);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if the first JSONB value is less than or equal to the
 * second one
 * @sqlfn le()
 */
Datum
Jsonb_le(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_le(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_gt);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if the first JSONB value is greater than the second one
 * @sqlfn gt()
 */
Datum
Jsonb_gt(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_gt(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_ge);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return true if the first JSONB value is greater than or equal to the
 * second one
 * @sqlfn ge()
 */
Datum
Jsonb_ge(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  bool result = pg_jsonb_ge(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Jsonb_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_cmp);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return -1, 0 or 1 depending on whether the first JSONB value is less
 * than, equal to or greater than the second one
 * @sqlfn cmp()
 */
Datum
Jsonb_cmp(PG_FUNCTION_ARGS)
{
  Jsonb *jb1 = PG_GETARG_JSONB_P(0);
  Jsonb *jb2 = PG_GETARG_JSONB_P(1);
  int result = pg_jsonb_cmp(jb1, jb2);
  PG_FREE_IF_COPY(jb1, 0);
  PG_FREE_IF_COPY(jb2, 1);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Jsonb_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_hash);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return the 32-bit hash value of a JSONB value
 * @sqlfn hash()
 * @altsqlfn jsonbHash()
 */
Datum
Jsonb_hash(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  uint32 result = pg_jsonb_hash(jb);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Jsonb_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_hash_extended);
/**
 * @ingroup mobilitydb_json_comp
 * @brief Return the 64-bit hash value of a JSONB value using a seed
 * @sqlfn hashExtended()
 * @altsqlfn jsonbHashExtended()
 */
Datum
Jsonb_hash_extended(PG_FUNCTION_ARGS)
{
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = pg_jsonb_hash_extended(jb, seed);
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
