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
 * @brief JSON functions for temporal JSONB
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
#include "temporal/span.h"
#include "json/tjsonb.h"
#include <pgtypes.h>
/* MobilityDB */
#include "pg_temporal/temporal.h"

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * @brief Get an null_handle text
 * @pre The text CANNOT be null. This is usually achieved by setting a default
 * value in the SQL definition, e.g., `null_handle text DEFAULT 'use_json_null'`
 */
nullHandleType
input_null_handle_text(FunctionCallInfo fcinfo, int argno)
{
  text *null_handle_txt = PG_GETARG_TEXT_P(argno);
  assert(null_handle_txt);
  char *null_handle_str = pg_text_to_cstring(null_handle_txt);
  nullHandleType result = null_handle_type_from_string(null_handle_str);
  pfree(null_handle_str);
  PG_FREE_IF_COPY(null_handle_txt, argno);
  return result;
}

/*****************************************************************************
 * Temporal JSONB casting to/from temporal text
 *****************************************************************************/

PGDLLEXPORT Datum Jsonb_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Jsonb_as_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Transform a JSONB value into a text value
 * @sqlfn text()
 * @sqlop @p ::
 */
Datum
Jsonb_as_text(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  /* Compute the result */
  text *result = pg_jsonb_to_text(jb);
  /* Clean up and return */
  PG_FREE_IF_COPY(jb, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Tjsonb_as_ttext(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_as_ttext);
/**
 * @ingroup mobilitydb_json_json
 * @brief Transform a temporal JSONB value into a temporal text value
 * @sqlfn ttext()
 * @sqlop @p ::
 */
Datum
Tjsonb_as_ttext(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Compute the result */
  Temporal *result = tjsonb_to_ttext(temp);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttext_as_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttext_as_tjsonb);
/**
 * @ingroup mobilitydb_json_json
 * @brief Transform a temporal text value into a temporal JSONB value
 * @sqlfn tjsonb()
 * @sqlop @p ::
 */
Datum
Ttext_as_tjsonb(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Compute the result */
  Temporal *result = ttext_to_tjsonb(temp);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * JSON functions
 *****************************************************************************/

PGDLLEXPORT Datum Tjson_array_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_array_length);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the array length of a temporal JSON value
 * @sqlfn tjson_array_length()
 */
Datum
Tjson_array_length(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Compute the result */
  Temporal *result = tjson_array_length(temp);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_array_length(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_array_length);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the array length of a temporal JSONB value
 * @sqlfn tjsonb_array_length()
 */
Datum
Tjsonb_array_length(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Compute the result */
  Temporal *result = tjsonb_array_length(temp);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @brief Extract a field from a temporal JSON value
 * @sqlfn tjson_object_field(), tjson_object_field_text()
 * @sqlop @p -> @p ->>
 */
Datum
Tjson_object_field_common(FunctionCallInfo fcinfo, bool astext)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Temporal *result = tjson_object_field(temp, key, astext, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjson_object_field(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_object_field);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value
 * @sqlfn tjson_object_field()
 * @sqlop @p ->
 */
Datum
Tjson_object_field(PG_FUNCTION_ARGS)
{
  return Tjson_object_field_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjson_object_field_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_object_field_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value
 * @sqlfn tjson_object_field()
 * @sqlop @p ->
 */
Datum
Tjson_object_field_opr(PG_FUNCTION_ARGS)
{
  return Tjson_object_field_common(fcinfo, false);
}

/*****************************************************************************/

/**
 * @brief Extract a field from a temporal JSONB value
 * @sqlfn tjsonb_object_field(), tjsonb_object_field_text()
 * @sqlop @p ->, @p ->
 */
Datum
Tjsonb_object_field_common(FunctionCallInfo fcinfo, bool astext)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Temporal *result = tjsonb_object_field(temp, key, astext, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_object_field(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_object_field);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value as text
 * @sqlfn tjsonb_object_field()
 * @sqlop @p ->
 */
Datum
Tjsonb_object_field(PG_FUNCTION_ARGS)
{
  return Tjsonb_object_field_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_object_field_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_object_field_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a field from a temporal JSON value as text
 * @sqlfn tjsonb_object_field_text()
 * @sqlop @p ->>
 */
Datum
Tjsonb_object_field_text(PG_FUNCTION_ARGS)
{
  return Tjsonb_object_field_common(fcinfo, true);
}

PGDLLEXPORT Datum Tjsonb_object_field_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_object_field_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * temporal JSONB value
 * @details Implementation of operator "tjsonb -> text" (2-argument version
 * of tjsonb_object_field())
 * @sqlfn tjsonb_object_field_opr()
 * @sqlop @p ->
 */
Datum
Tjsonb_object_field_opr(PG_FUNCTION_ARGS)
{
  return Tjsonb_object_field_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_object_field_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_object_field_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * temporal JSONB value
 * @details Implementation of operator "tjsonb ->> text" (2-argument version
 * of tjsonb_object_field_text())
 * @sqlfn tjsonb_object_field_text_opr()
 * @sqlop @p ->>
 */
Datum
Tjsonb_object_field_text_opr(PG_FUNCTION_ARGS)
{
  return Tjsonb_object_field_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Concat_jsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Concat_jsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_json
 * @brief Concat a JSONB value with a temporal JSONB
 * @sqlfn tjsonb_concat()
 * @sqlop @p ||
 */
Datum
Concat_jsonb_tjsonb(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Jsonb *jb = PG_GETARG_JSONB_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Compute the result */
  Temporal *result = concat_tjsonb_jsonb(temp, jb, INVERT);
  /* Clean up and return */
  PG_FREE_IF_COPY(jb, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Concat_tjsonb_jsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Concat_tjsonb_jsonb);
/**
 * @ingroup mobilitydb_json_json
 * @brief Concat a temporal JSONB with a JSONB value
 * @sqlfn jsonb_concat()
 * @sqlop @p ||
 */
Datum
Concat_tjsonb_jsonb(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Jsonb *jb = PG_GETARG_JSONB_P(1);
  /* Compute the result */
  Temporal *result = concat_tjsonb_jsonb(temp, jb, INVERT_NO);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jb, 1);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Concat_tjsonb_tjsonb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Concat_tjsonb_tjsonb);
/**
 * @ingroup mobilitydb_json_json
 * @brief Concat two temporal JSONB values
 * @sqlfn jsonb_concat()
 * @sqlop @p ||
 */
Datum
Concat_tjsonb_tjsonb(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Compute the result */
  Temporal *result = concat_tjsonb_tjsonb(temp1, temp2);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tjsonb_delete(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_delete);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a key or an element array from a temporal JSONB value
 * @sqlfn jsonb_delete_key()
 * @sqlop @p -
 */
Datum
Tjsonb_delete(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *key = PG_GETARG_TEXT_P(1);
  /* Compute the result */
  Temporal *result = tjsonb_delete(temp, key);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(key, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_delete_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_delete_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete an array of keys or array elements from a temporal JSONB value
 * @sqlfn jsonb_delete_array()
 * @sqlop @p -
 */
Datum
Tjsonb_delete_array(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    PG_RETURN_TEMPORAL_P(temp);

  /* Compute the result */
  Temporal *result = tjsonb_delete_array(temp, (text **) keys_elems, keys_len);

  /* Clean up and return */
  pfree(keys_elems); pfree(keys_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(keys, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_delete_index(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_delete_index);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a key specified by an index from a temporal JSONB value
 * @sqlfn jsonb_delete()
 * @sqlop @p -
 */
Datum
Tjsonb_delete_index(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int idx = PG_GETARG_INT32(1);
  /* Compute the result */
  Temporal *result = tjsonb_delete_index(temp, idx);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_delete_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_delete_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Delete a path from a temporal JSONB value
 * @sqlfn tjsonb_delete_path()
 */
Datum
Tjsonb_delete_path(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    PG_RETURN_TEMPORAL_P(temp);

  /* Compute the result */
  Temporal *result = tjsonb_delete_path(temp, (text **) path_elems, path_len);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @brief Extract an array element from a temporal JSON value
 * @sqlfn tjson_array_element(), tjson_array_element_opr()
 * @sqlop ->
 */
Datum
Tjson_array_element_common(FunctionCallInfo fcinfo)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int idx = PG_GETARG_INT32(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Temporal *result = tjson_array_element(temp, idx, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjson_array_element(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_array_element);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSON value
 * @sqlfn tjson_array_element()
 */
Datum
Tjson_array_element(PG_FUNCTION_ARGS)
{
  return Tjson_array_element_common(fcinfo);
}

PGDLLEXPORT Datum Tjson_array_element_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_array_element_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSON value
 * @sqlfn tjson_array_element_opr()
 */
Datum
Tjson_array_element_opr(PG_FUNCTION_ARGS)
{
  return Tjson_array_element_common(fcinfo);
}

/**
 * @brief Extract an array element from a temporal JSONB value
 * @sqlfn tjsonb_array_element(), tjsonb_array_element_opr()
  * @sqlop ->
*/
Datum
Tjsonb_array_element_common(FunctionCallInfo fcinfo, bool astext)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int idx = PG_GETARG_INT32(1);
  nullHandleType null_handle = NULL_JSON_NULL;
  if (PG_NARGS() == 3)
  {
    null_handle = input_null_handle_text(fcinfo, 2);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
  }
  /* Compute the result */
  Temporal *result = tjsonb_array_element(temp, idx, astext, null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_array_element(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_array_element);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSONB value
 * @sqlfn tjsonb_array_element()
 */
Datum
Tjsonb_array_element(PG_FUNCTION_ARGS)
{
  return Tjsonb_array_element_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_array_element_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_array_element_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSONB value
 * @sqlfn tjsonb_array_element_opr()
 */
Datum
Tjsonb_array_element_opr(PG_FUNCTION_ARGS)
{
  return Tjsonb_array_element_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_array_element_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_array_element_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSONB value as text
 * @sqlfn tjsonb_array_element_text()
 */
Datum
Tjsonb_array_element_text(PG_FUNCTION_ARGS)
{
  return Tjsonb_array_element_common(fcinfo, true);
}

PGDLLEXPORT Datum Tjsonb_array_element_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_array_element_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an array element from a temporal JSONB value as text
 * @sqlfn tjsonb_array_element_text_opr()
 */
Datum
Tjsonb_array_element_text_opr(PG_FUNCTION_ARGS)
{
  return Tjsonb_array_element_common(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Extract a path from a temporal JSONB value
 */
Datum
Tjson_extract_path_common(FunctionCallInfo fcinfo, bool isjsonb, bool astext)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    PG_RETURN_TEMPORAL_P(temp);
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
  Temporal *result = isjsonb ?
    tjsonb_extract_path(temp, (text **) path_elems, path_len, astext, null_handle) :
    tjson_extract_path(temp, (text **) path_elems, path_len, null_handle);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjson_extract_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_extract_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an item specified by a path from a temporal JSON value
 * @sqlfn tjson_extract_path()
 */
Datum
Tjson_extract_path(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Tjson_extract_path_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_extract_path_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract an item specified by a path from a temporal JSON value as text
 * @sqlfn tjson_extract_path_text()
 */
Datum
Tjson_extract_path_opr(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Tjsonb_extract_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_extract_path);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a temporal JSONB value
 * @sqlfn tjsonb_extract_path()
 */
Datum
Tjsonb_extract_path(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Tjsonb_extract_path_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_extract_path_text);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a temporal JSONB value as text
 * @sqlfn tjsonb_extract_path_text()
 */
Datum
Tjsonb_extract_path_text(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, true, true);
}


PGDLLEXPORT Datum Tjsonb_extract_path_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_extract_path_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a temporal JSONB value
 * @sqlfn tjsonb_extract_path_opr()
 */
Datum
Tjsonb_extract_path_opr(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Tjsonb_extract_path_text_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_extract_path_text_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract a path from a temporal JSONB value as text
 * @sqlfn tjsonb_extract_path_text_opr()
 */
Datum
Tjsonb_extract_path_text_opr(PG_FUNCTION_ARGS)
{
  return Tjson_extract_path_common(fcinfo, true, true);
}

/*****************************************************************************/

/**
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value 
 * @sqlfn tjsonb_set(), tjsonb_set_lax()
 */
Datum
Tjsonb_set_common(FunctionCallInfo fcinfo, bool lax)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    PG_RETURN_TEMPORAL_P(temp);

  /* Compute the result */
  Temporal *result = tjsonb_set(temp, (text **) path_elems, path_len, newjsonb,
    create, null_handle, lax);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_set(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_set);
/**
 * @ingroup mobilitydb_json_json
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value
 * @sqlfn tjsonb_set()
 */
Datum
Tjsonb_set(PG_FUNCTION_ARGS)
{
  return Tjsonb_set_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_set_lax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_set_lax);
/**
 * @ingroup mobilitydb_json_json
 * @brief Replace a value specified by a path with a new value in a temporal
 * JSONB value using the lax mode
 * @sqlfn tjsonb_set_lax()
 */
Datum
Tjsonb_set_lax(PG_FUNCTION_ARGS)
{
  return Tjsonb_set_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tjsonb_insert(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_insert);
/**
 * @ingroup mobilitydb_json_json
 * @brief Insert a path into a temporal JSONB value
 * @sqlfn tjsonb_insert()
 */
Datum
Tjsonb_insert(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
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
    PG_RETURN_TEMPORAL_P(temp);

  /* Compute the result */
  Temporal *result = tjsonb_insert(temp, (text **) path_elems, path_len,
    newjsonb, after);

  /* Clean up and return */
  pfree(path_elems); pfree(path_nulls);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(path, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @brief Convert a temporal JSONB value into a temporal alphanumeric type
 */
static Datum
Tjsonb_to_talphanum(FunctionCallInfo fcinfo, MeosType restype, bool hasinterp)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *key_text = PG_GETARG_TEXT_PP(1);
  char *key = pg_text_to_cstring(key_text);
  interpType interp = temptype_supports_linear(restype) ? LINEAR : STEP;
  int argno = 2;
  if (hasinterp)
  {
    if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
      interp = input_interp_string(fcinfo, 2);
    argno++;
  }
  /* NULL_JSON_NULL cannot be used for obtaining temporal alphanumeric values */
  nullHandleType null_handle = NULL_RETURN; 
  if (PG_NARGS() > argno && ! PG_ARGISNULL(argno))
  {
    null_handle = input_null_handle_text(fcinfo, argno);
    if (null_handle == NULL_INVALID)
      PG_RETURN_NULL();
    if (null_handle == NULL_JSON_NULL)
      ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
         errmsg("'use_json_null' is not supported for this function")));
  }
  /* Compute the result */
  Temporal *result = tjsonb_to_talphanum(temp, key, restype, interp,
    null_handle);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(key_text, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tjsonb_to_tbool(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_to_tbool);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a temporal JSONB value into a temporal boolean
 * @sqlfn tbool()
 */
Datum
Tjsonb_to_tbool(PG_FUNCTION_ARGS)
{
  return Tjsonb_to_talphanum(fcinfo, T_TBOOL, false);
}

PGDLLEXPORT Datum Tjsonb_to_tint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_to_tint);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a temporal JSONB value into a temporal integer
 * @sqlfn tint()
 */
Datum
Tjsonb_to_tint(PG_FUNCTION_ARGS)
{
  return Tjsonb_to_talphanum(fcinfo, T_TINT, false);
}

PGDLLEXPORT Datum Tjsonb_to_tbigint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_to_tbigint);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a temporal JSONB value into a temporal big integer
 * @sqlfn tbigint()
 */
Datum
Tjsonb_to_tbigint(PG_FUNCTION_ARGS)
{
  return Tjsonb_to_talphanum(fcinfo, T_TBIGINT, false);
}

PGDLLEXPORT Datum Tjsonb_to_tfloat(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_to_tfloat);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a temporal JSONB value into a temporal float
 * @sqlfn tfloat()
 */
Datum
Tjsonb_to_tfloat(PG_FUNCTION_ARGS)
{
  return Tjsonb_to_talphanum(fcinfo, T_TFLOAT, true);
}

PGDLLEXPORT Datum Tjsonb_to_ttext_key(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_to_ttext_key);
/**
 * @ingroup mobilitydb_json_json
 * @brief Convert a temporal JSONB value into a temporal text
 * @sqlfn ttext()
 */
Datum
Tjsonb_to_ttext_key(PG_FUNCTION_ARGS)
{
  return Tjsonb_to_talphanum(fcinfo, T_TTEXT, false);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tjson_strip_nulls(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjson_strip_nulls);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a temporal JSON value without nulls
 * @sqlfn tjson_strip_nulls()
 */
Datum
Tjson_strip_nulls(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool strip_nulls = PG_GETARG_BOOL(1);
  /* Compute the result */
  Temporal *result = tjson_strip_nulls(temp, strip_nulls);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tjsonb_strip_nulls(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_strip_nulls);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a temporal JSONB value without nulls
 * @sqlfn tjsonb_strip_nulls()
 */
Datum
Tjsonb_strip_nulls(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool strip_nulls = PG_GETARG_BOOL(1);
  /* Compute the result */
  Temporal *result = tjsonb_strip_nulls(temp, strip_nulls);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tjsonb_pretty(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_pretty);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return a temporal JSONB value without nulls
 * @sqlfn tjsonb_pretty()
 */
Datum
Tjsonb_pretty(PG_FUNCTION_ARGS)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Compute the result */
  Temporal *result = tjsonb_pretty(temp);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * JSON path functions
 *****************************************************************************/

/**
 * @brief Return true if a JSON path expression returns at least one item for a
 * temporal JSONB value
 * @sqlfn tjsonb_path_exists(), tjsonb_path_exists_tz()
 */
Datum
Tjsonb_path_exists_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = NULL;
  bool silent = true;
  if (PG_NARGS() == 4)
  {
    vars = PG_GETARG_JSONB_P(2);
    silent = PG_GETARG_BOOL(3);
  }
  /* Compute the result */
  Temporal *result = tjsonb_path_exists(temp, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_path_exists(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_exists);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * temporal JSONB value
 * @sqlfn tjsonb_path_exists()
 */
Datum
Tjsonb_path_exists(PG_FUNCTION_ARGS)
{
 return Tjsonb_path_exists_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_path_exists_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_exists_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns expression at least one
 * item for a temporal JSONB value
 * @sqlfn tjsonb_path_exists_tz()
 */
Datum
Tjsonb_path_exists_tz(PG_FUNCTION_ARGS)
{
  return Tjsonb_path_exists_common(fcinfo, true);
}

PGDLLEXPORT Datum Tjsonb_path_exists_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_exists_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * temporal JSONB value
 * @details Implementation of operator "tjsonb @? jsonpath" (2-argument version
 * of tjsonb_path_exists())
 * @sqlfn tjsonb_path_exists_opr()
 */
Datum
Tjsonb_path_exists_opr(PG_FUNCTION_ARGS)
{
  return Tjsonb_path_exists_common(fcinfo, false);
}

/*****************************************************************************/

/**
 * @brief Return the result of a JSON path predicate check for a temporal JSONB
 * value
 * @sqlfn tjsonb_path_match(), tjsonb_path_match_tz()
 */
Datum
Tjsonb_path_match_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = NULL;
  bool silent = true;
  if (PG_NARGS() == 4)
  {
    vars = PG_GETARG_JSONB_P(2);
    silent = PG_GETARG_BOOL(3);
  }
  /* Compute the result */
  Temporal *result = tjsonb_path_match(temp, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_path_match(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_match);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a temporal JSONB
 * value
 * @sqlfn tjsonb_path_match()
 */
Datum
Tjsonb_path_match(PG_FUNCTION_ARGS)
{
 return Tjsonb_path_match_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_path_match_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_match_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a temporal JSONB
 * value
 * @sqlfn tjsonb_path_match_tz()
 */
Datum
Tjsonb_path_match_tz(PG_FUNCTION_ARGS)
{
  return Tjsonb_path_match_common(fcinfo, true);
}

PGDLLEXPORT Datum Tjsonb_path_match_opr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_match_opr);
/**
 * @ingroup mobilitydb_json_json
 * @brief Return the result of a JSON path predicate check for a temporal JSONB
 * value
 * @details Implementation of operator "tjsonb @@ jsonpath" (2-argument version
 * of tjsonb_path_match())
 * @sqlfn Tjsonb_path_match_opr()
 */
Datum
Tjsonb_path_match_opr(PG_FUNCTION_ARGS)
{
 return Tjsonb_path_match_common(fcinfo, false);
}

/*****************************************************************************/

/**
 * @brief Extract the items specified by a JSON path expression from a
 * temporal JSONB value as a JSONB array
 * @sqlfn tjsonb_path_query_array(), tjsonb_path_query_array_tz()
 */
Datum
Tjsonb_path_query_array_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = PG_GETARG_JSONB_P(2);
  bool silent = PG_GETARG_BOOL(3);
  /* Compute the result */
  Temporal *result = tjsonb_path_query_array(temp, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_path_query_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_query_array);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the items specified by a JSON path expression from a
 * temporal JSONB value as a JSONB array
 * @sqlfn tjsonb_path_query_array()
 */
Datum
Tjsonb_path_query_array(PG_FUNCTION_ARGS)
{
 return Tjsonb_path_query_array_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_path_query_array_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_query_array_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the items specified by a JSON path expression from a
 * temporal JSONB value as a JSONB array
 * @sqlfn tjsonb_path_query_array_tz()
 */
Datum
Tjsonb_path_query_array_tz(PG_FUNCTION_ARGS)
{
  return Tjsonb_path_query_array_common(fcinfo, true);
}

/*****************************************************************************/

/**
 * @brief Extract the first item specified by a JSON path expression from a
 * temporal JSONB value. If there are no items, return NULL.
 * @sqlfn tjsonb_query_first(), tjsonb_query_first_tz()
 */
Datum
Tjsonb_path_query_first_common(FunctionCallInfo fcinfo, bool tz)
{
  /* Input arguments */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  JsonPath *jp = PG_GETARG_JSONPATH_P(1);
  Jsonb *vars = PG_GETARG_JSONB_P(2);
  bool silent = PG_GETARG_BOOL(3);
  /* Compute the result */
  Temporal *result = tjsonb_path_query_first(temp, jp, vars, silent, tz);
  /* Clean up and return */
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(jp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tjsonb_path_query_first(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_query_first);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the first item specified by a JSON path expression from a
 * temporal JSONB value. If there are no items, return NULL.
 * @sqlfn tjsonb_path_query_first()
 */
Datum
Tjsonb_path_query_first(PG_FUNCTION_ARGS)
{
 return Tjsonb_path_query_first_common(fcinfo, false);
}

PGDLLEXPORT Datum Tjsonb_path_query_first_tz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tjsonb_path_query_first_tz);
/**
 * @ingroup mobilitydb_json_json
 * @brief Extract the first item specified by a JSON path expression from a
 * temporal JSONB value. If there are no items, return NULL.
 * @sqlfn tjsonb_path_query_first_tz()
 */
Datum
Tjsonb_path_query_first_tz(PG_FUNCTION_ARGS)
{
  return Tjsonb_path_query_first_common(fcinfo, true);
}

/*****************************************************************************/