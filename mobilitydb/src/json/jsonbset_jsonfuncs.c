/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief JSON functions for JSONB set
 */

/* C */
#include <stdbool.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/array.h"
#include "utils/jsonb.h"
#include "utils/jsonpath.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "json/tjsonb.h"
#include <pgtypes.h>
/* MobilityDB */
#include "pg_temporal/temporal.h"

/* Function defined in file tjsonb_jsonfuncs.c */
extern nullHandleType input_null_handle_text(FunctionCallInfo fcinfo, int argno);

/*****************************************************************************
 * JSON functions
 *****************************************************************************/

PGDLLEXPORT Datum Jsonbset_array_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_array_length);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the array length of a JSONB set value
 * @sqlfn jsonbset_array_length()
 */
Datum
Jsonbset_array_length(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  /* Compute the result */
  Set *result = jsonbset_array_length(set);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

/**
 * @brief Extract a field from a JSONB set value
 * @sqlfn jsonbset_object_field(), jsonbset_object_field_text()
 * @sqlop @p ->, @p ->
 */
Datum
Jsonbset_object_field_common(FunctionCallInfo fcinfo, bool astext)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Set *result = jsonbset_object_field(set, key, astext, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_object_field(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_object_field);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value as text
 * @sqlfn jsonbset_object_field()
 * @sqlop @p ->
 */
Datum
Jsonbset_object_field(PG_FUNCTION_ARGS)
{
  return Jsonbset_object_field_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_object_field_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_object_field_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value as text
 * @sqlfn jsonbset_object_field_text()
 * @sqlop @p ->>
 */
Datum
Jsonbset_object_field_text(PG_FUNCTION_ARGS)
{
  return Jsonbset_object_field_common(fcinfo, true);
}

PGDLLEXPORT Datum Jsonbset_object_field_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_object_field_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set value
 * @details Implementation of operator "tjsonb -> text" (2-argument version
 * of jsonbset_object_field())
 * @sqlfn jsonbset_object_field_opr()
 * @sqlop @p ->
 */
Datum
Jsonbset_object_field_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_object_field_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_object_field_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_object_field_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set value
 * @details Implementation of operator "tjsonb ->> text" (2-argument version
 * of jsonbset_object_field_text())
 * @sqlfn jsonbset_object_field_text_opr()
 * @sqlop @p ->>
 */
Datum
Jsonbset_object_field_text_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_object_field_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Concat_jsonb_jsonbset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Concat_jsonb_jsonbset);
/**
 * @ingroup mobilitydb_json_json
 * @brief Concat a JSONB value with a JSONB set
 * @sqlfn jsonbset_concat()
 * @sqlop @p ||
 */
Datum
Concat_jsonb_jsonbset(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  Set *set = PG_GETARG_SET_P(1);
  /* Compute the result */
  Set *result = concat_jsonbset_jsonb(set, jb, INVERT);
  /* Clean up and return */
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(set, 1);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Concat_jsonbset_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Concat_jsonbset_jsonb);
/**
 * @ingroup mobilitydb_json_json
 * @brief Concat a JSONB set with a JSONB value
 * @sqlfn jsonb_concat()
 * @sqlop @p ||
 */
Datum
Concat_jsonbset_jsonb(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  Jsonb *jb = PG_GETARG_JSONB_P(1);
  /* Compute the result */
  Set *result = concat_jsonbset_jsonb(set, jb, INVERT_NO);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(jb, 1);
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Jsonbset_delete(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_delete);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a key or an element array from a JSONB set value
 * @sqlfn jsonb_delete_key()
 * @sqlop @p -
 */
Datum
Jsonbset_delete(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  /* Compute the result */
  Set *result = jsonbset_delete(set, key);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_delete_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_delete_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete an array of keys or array elements from a JSONB set value
 * @sqlfn jsonb_delete_array()
 * @sqlop @p -
 */
Datum
Jsonbset_delete_array(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  ArrayType *keys = PG_GETARG_ARRAYTYPE_P(1);
  if (ARR_NDIM(keys) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
       errmsg("wrong number of array subscripts")));

  /* Extract the keys from the array */
  int keys_len;
  Datum *keys_elems;
  bool *keys_nulls;
  deconstruct_array(keys, TEXTOID, -1, false, 'i', &keys_elems, &keys_nulls,
    &keys_len);
  if (keys_len == 0)
    PG_RETURN_SET_P(set);

  /* Compute the result */
  Set *result = jsonbset_delete_array(set, (text **) keys_elems, keys_len);

  /* Clean up and return */
  pfree(keys_elems); pfree(keys_nulls);
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(keys, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_delete_index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_delete_index);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a key specified by an index from a JSONB set value
 * @sqlfn jsonb_delete()
 * @sqlop @p -
 */
Datum
Jsonbset_delete_index(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  int idx = PG_GETARG_INT32(1);
  /* Compute the result */
  Set *result = jsonbset_delete_index(set, idx);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_delete_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_delete_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a path from a JSONB set value
 * @sqlfn jsonbset_delete_path()
 */
Datum
Jsonbset_delete_path(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  if (ARR_NDIM(path) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
       errmsg("wrong number of array subscripts")));

  /* Extract the path from the array */
  int path_len;
  Datum *path_elems;
  bool *path_nulls;
  deconstruct_array(path, TEXTOID, -1, false, 'i', &path_elems, &path_nulls,
    &path_len);
  if (path_len == 0)
    PG_RETURN_SET_P(set);

  /* Compute the result */
  Set *result = jsonbset_delete_path(set, (text **) path_elems, path_len);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

/**
 * @brief Extract an array element from a JSONB set value
 * @sqlfn jsonbset_array_element(), jsonbset_array_element_opr()
  * @sqlop ->
*/
Datum
Jsonbset_array_element_common(FunctionCallInfo fcinfo, bool astext)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  int idx = PG_GETARG_INT32(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Set *result = jsonbset_array_element(set, idx, astext, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_array_element(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_array_element);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a JSONB set value
 * @sqlfn jsonbset_array_element()
 */
Datum
Jsonbset_array_element(PG_FUNCTION_ARGS)
{
  return Jsonbset_array_element_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_array_element_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_array_element_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a JSONB set value
 * @sqlfn jsonbset_array_element_opr()
 */
Datum
Jsonbset_array_element_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_array_element_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_array_element_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_array_element_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a JSONB set value as text
 * @sqlfn jsonbset_array_element_text()
 */
Datum
Jsonbset_array_element_text(PG_FUNCTION_ARGS)
{
  return Jsonbset_array_element_common(fcinfo, true);
}

PGDLLEXPORT Datum Jsonbset_array_element_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_array_element_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a JSONB set value as text
 * @sqlfn jsonbset_array_element_text_opr()
 */
Datum
Jsonbset_array_element_text_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_array_element_common(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Extract a path from a temporal JSONB value
 */
Datum
Jsonbset_extract_path_common(FunctionCallInfo fcinfo, bool isjsonb UNUSED,
  bool astext)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  if (ARR_NDIM(path) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
       errmsg("wrong number of array subscripts")));

  /* Extract the path from the array */
  int path_len;
  Datum *path_elems;
  bool *path_nulls;
  deconstruct_array(path, TEXTOID, -1, false, 'i', &path_elems, &path_nulls,
    &path_len);
  if (path_len == 0)
    PG_RETURN_SET_P(set);
  /*
   * If the array contains any null elements, return NULL, on the grounds
   * that you'd have gotten NULL if any RHS value were NULL in a nested
   * series of applications of the -> operator.  (Note: because we also
   * return NULL for error cases such as no-such-field, this is true
   * regardless of the contents of the rest of the array.)
   */
  if (array_contains_nulls(path))
    PG_RETURN_NULL();
  
  /* Compute the result */
  Set *result = // isjsonb ?
    jsonbset_extract_path(set, (text **) path_elems, path_len, astext, null_handle); // :
    // ttextset_extract_path(set, (text **) path_elems, path_len, null_handle);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_extract_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_extract_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a JSONB set value
 * @sqlfn jsonbset_extract_path()
 */
Datum
Jsonbset_extract_path(PG_FUNCTION_ARGS)
{
  return Jsonbset_extract_path_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Jsonbset_extract_path_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_extract_path_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a JSONB set value as text
 * @sqlfn jsonbset_extract_path_text()
 */
Datum
Jsonbset_extract_path_text(PG_FUNCTION_ARGS)
{
  return Jsonbset_extract_path_common(fcinfo, true, true);
}


PGDLLEXPORT Datum Jsonbset_extract_path_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_extract_path_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a JSONB set value
 * @sqlfn jsonbset_extract_path_opr()
 */
Datum
Jsonbset_extract_path_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_extract_path_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Jsonbset_extract_path_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_extract_path_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a JSONB set value as text
 * @sqlfn jsonbset_extract_path_text_opr()
 */
Datum
Jsonbset_extract_path_text_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_extract_path_common(fcinfo, true, true);
}

/*****************************************************************************/

/**
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value 
 * @sqlfn jsonbset_set(), jsonbset_set_lax()
 */
Datum
Jsonbset_set_common(FunctionCallInfo fcinfo, bool lax)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  Jsonb *newjsonb = PG_GETARG_JSONB_P(2);
  bool create = PG_GETARG_BOOL(3);
  text *null_handle = NULL;
  if (lax)
    null_handle = PG_GETARG_TEXT_P(4);
 
 if (ARR_NDIM(path) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("wrong number of array subscripts")));

  Datum *path_elems;
  bool *path_nulls;
  int path_len;
  deconstruct_array(path, TEXTOID, -1, false, 'i', &path_elems, &path_nulls,
    &path_len);
  if (path_len == 0)
    PG_RETURN_SET_P(set);

  /* Compute the result */
  Set *result = jsonbset_set(set, (text **) path_elems, path_len, newjsonb,
    create, null_handle, lax);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_set);
/**
 * @ingroup mobilitydb_json_json
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value
 * @sqlfn jsonbset_set()
 */
Datum
Jsonbset_set(PG_FUNCTION_ARGS)
{
  return Jsonbset_set_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_set_lax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_set_lax);
/**
 * @ingroup mobilitydb_json_json
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value using the lax mode
 * @sqlfn jsonbset_set_lax()
 */
Datum
Jsonbset_set_lax(PG_FUNCTION_ARGS)
{
  return Jsonbset_set_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Jsonbset_insert(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_insert);
/**
 * @ingroup mobilitydb_json_json
 * @brief Insert a path into a JSONB set value
 * @sqlfn jsonbset_insert()
 */
Datum
Jsonbset_insert(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  ArrayType *path = PG_GETARG_ARRAYTYPE_P(1);
  Jsonb *newjsonb = PG_GETARG_JSONB_P(2);
  bool after = PG_GETARG_BOOL(3);
  if (ARR_NDIM(path) > 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("wrong number of array subscripts")));

  /* Extract the path from the array */
  int path_len;
  Datum *path_elems;
  bool *path_nulls;
  deconstruct_array(path, TEXTOID, -1, false, 'i', &path_elems, &path_nulls,
    &path_len);
  if (path_len == 0)
    PG_RETURN_SET_P(set);

  /* Compute the result */
  Set *result = jsonbset_insert(set, (text **) path_elems, path_len,
    newjsonb, after);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

/*****************************************************************************/

/**
 * @brief Convert a JSONB set value into a temporal alphanumeric type
 * @sqlfn jsonbset_to_intset()
 */
Datum
Jsonbset_to_alphanumset(FunctionCallInfo fcinfo, MeosType setbasetype)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  text *key_text = PG_GETARG_TEXT_PP(1);
  char *key = pg_text_to_cstring(key_text);
  /* NULL_JSON_NULL cannot be used for obtaining alphanumeric sets */
  nullHandleType null_handle = NULL_RETURN; 
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
    if (null_handle == NULL_JSON_NULL)
      ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
         errmsg("'use_json_null' is not supported for this function")));
  }
  /* Compute the result */
  Set *result = jsonbset_to_alphanumset(set, key, setbasetype, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(key_text, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Jsonbset_to_intset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_to_intset);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a JSONB set value into a temporal integer
 * @sqlfn tint()
 */
Datum
Jsonbset_to_intset(PG_FUNCTION_ARGS)
{
  return Jsonbset_to_alphanumset(fcinfo, T_INT4);
}

PGDLLEXPORT Datum Jsonbset_to_floatset(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_to_floatset);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a JSONB set value into a temporal float
 * @sqlfn tfloat()
 */
Datum
Jsonbset_to_floatset(PG_FUNCTION_ARGS)
{
  return Jsonbset_to_alphanumset(fcinfo, T_FLOAT8);
}

PGDLLEXPORT Datum Jsonbset_to_textset_key(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_to_textset_key);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a JSONB set value into a text set
 * @sqlfn ttext()
 */
Datum
Jsonbset_to_textset_key(PG_FUNCTION_ARGS)
{
  return Jsonbset_to_alphanumset(fcinfo, T_TEXT);
}

/*****************************************************************************/

PGDLLEXPORT Datum Jsonbset_strip_nulls(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_strip_nulls);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB set value without nulls
 * @sqlfn jsonbset_strip_nulls()
 */
Datum
Jsonbset_strip_nulls(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  bool strip_nulls = PG_GETARG_BOOL(1);
  /* Compute the result */
  Set *result = jsonbset_strip_nulls(set, strip_nulls);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Jsonbset_pretty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_pretty);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a JSONB set value without nulls
 * @sqlfn jsonbset_pretty()
 */
Datum
Jsonbset_pretty(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  /* Compute the result */
  Set *result = jsonbset_pretty(set);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * JSON path functions
 *****************************************************************************/

/**
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set value
 * @sqlfn jsonbset_path_exists(), jsonbset_path_exists_tz()
 */
Datum
Jsonbset_path_exists_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = NULL;
  bool silent = true;
  if (PG_NARGS() == 4)
  {
    vars = PG_GETARG_JSONB_P(2);
    silent = PG_GETARG_BOOL(3);
  }
  /* Compute the result */
  Set *result = jsonbset_path_exists(set, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_path_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_exists);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set value
 * @sqlfn jsonbset_path_exists()
 */
Datum
Jsonbset_path_exists(PG_FUNCTION_ARGS)
{
 return Jsonbset_path_exists_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_path_exists_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_exists_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns expression at least one
 * item for a JSONB set value
 * @sqlfn jsonbset_path_exists_tz()
 */
Datum
Jsonbset_path_exists_tz(PG_FUNCTION_ARGS)
{
  return Jsonbset_path_exists_common(fcinfo, true);
}

PGDLLEXPORT Datum Jsonbset_path_exists_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_exists_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set value
 * @details Implementation of operator "tjsonb @? jsonpath" (2-argument version
 * of jsonbset_path_exists())
 * @sqlfn jsonbset_path_exists_opr()
 */
Datum
Jsonbset_path_exists_opr(PG_FUNCTION_ARGS)
{
  return Jsonbset_path_exists_common(fcinfo, false);
}

/*****************************************************************************/

/**
 * @brief Return the result of a JSON path predicate check for a JSONB set
 * value
 * @sqlfn jsonbset_path_match(), jsonbset_path_match_tz()
 */
Datum
Jsonbset_path_match_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = NULL;
  bool silent = true;
  if (PG_NARGS() == 4)
  {
    vars = PG_GETARG_JSONB_P(2);
    silent = PG_GETARG_BOOL(3);
  }
  /* Compute the result */
  Set *result = jsonbset_path_match(set, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_path_match(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_match);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a JSONB set
 * value
 * @sqlfn jsonbset_path_match()
 */
Datum
Jsonbset_path_match(PG_FUNCTION_ARGS)
{
 return Jsonbset_path_match_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_path_match_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_match_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a JSONB set
 * value
 * @sqlfn jsonbset_path_match_tz()
 */
Datum
Jsonbset_path_match_tz(PG_FUNCTION_ARGS)
{
  return Jsonbset_path_match_common(fcinfo, true);
}

PGDLLEXPORT Datum Jsonbset_path_match_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_match_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a JSONB set
 * value
 * @details Implementation of operator "tjsonb @@ jsonpath" (2-argument version
 * of jsonbset_path_match())
 * @sqlfn Jsonbset_path_match_opr()
 */
Datum
Jsonbset_path_match_opr(PG_FUNCTION_ARGS)
{
 return Jsonbset_path_match_common(fcinfo, false);
}

/*****************************************************************************/

/**
 * @brief Extract the items specified by a JSON path expression from a
 * JSONB set value as a JSONB array
 * @sqlfn jsonbset_path_query_array(), jsonbset_path_query_array_tz()
 */
Datum
Jsonbset_path_query_array_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = PG_GETARG_JSONB_P(2);
  bool silent = PG_GETARG_BOOL(3);
  /* Compute the result */
  Set *result = jsonbset_path_query_array(set, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_path_query_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_query_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the items specified by a JSON path expression from a
 * JSONB set value as a JSONB array
 * @sqlfn jsonbset_path_query_array()
 */
Datum
Jsonbset_path_query_array(PG_FUNCTION_ARGS)
{
 return Jsonbset_path_query_array_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_path_query_array_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_query_array_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the items specified by a JSON path expression from a
 * JSONB set value as a JSONB array
 * @sqlfn jsonbset_path_query_array_tz()
 */
Datum
Jsonbset_path_query_array_tz(PG_FUNCTION_ARGS)
{
  return Jsonbset_path_query_array_common(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Extract the first item specified by a JSON path expression from a
 * JSONB set value. If there are no items, return NULL.
 * @sqlfn jsonbset_query_first(), jsonbset_query_first_tz()
 */
Datum
Jsonbset_path_query_first_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Set *set = PG_GETARG_SET_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = PG_GETARG_JSONB_P(2);
  bool silent = PG_GETARG_BOOL(3);
  /* Compute the result */
  Set *result = jsonbset_path_query_first(set, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(set, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Jsonbset_path_query_first(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_query_first);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the first item specified by a JSON path expression from a
 * JSONB set value. If there are no items, return NULL.
 * @sqlfn jsonbset_path_query_first()
 */
Datum
Jsonbset_path_query_first(PG_FUNCTION_ARGS)
{
 return Jsonbset_path_query_first_common(fcinfo, false);
}

PGDLLEXPORT Datum Jsonbset_path_query_first_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonbset_path_query_first_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the first item specified by a JSON path expression from a
 * JSONB set value. If there are no items, return NULL.
 * @sqlfn jsonbset_path_query_first_tz()
 */
Datum
Jsonbset_path_query_first_tz(PG_FUNCTION_ARGS)
{
  return Jsonbset_path_query_first_common(fcinfo, true);
}

/*****************************************************************************/