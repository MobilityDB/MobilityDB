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
 * @brief Basic functions for JSONB set
 */

/* C */
#include <assert.h>
#include <limits.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/varlena.h"
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"
#include "temporal/lifting.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "json/tjsonb.h"

/*****************************************************************************
 * JSON set functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Return the length of a JSONB set array
 * @param[in] set JSONB set
 * @csqlfn #Jsonbset_array_length()
 */
Set *
jsonbset_array_length(const Set *set)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_jsonb_array_length;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.restype = T_INTSET;
  lfinfo.resnull = NULL_DELETE; /* Handle NULL result */
  lfinfo.invert = INVERT_NO;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/


/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB object field with the given key
 * @param[in] set JSONB set
 * @param[in] key Key
 * @param[in] astext True when the output is a text set, otherwise is a
 * jsonb set
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_object_field()
 */
Set *
jsonbset_object_field(const Set *set, const text *key, bool astext,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(key, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = astext ? (varfunc) datum_jsonb_object_field_text :
    (varfunc) datum_jsonb_object_field;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = PointerGetDatum(key);
  lfinfo.restype = astext ? T_TEXTSET : T_JSONBSET;
  lfinfo.resnull = null_handle; /* Handle NULL result */
  lfinfo.invert = INVERT_NO;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Return the concatenation of a JSONB set and a JSONB
 * @param[in] set JSONB set
 * @param[in] jb JSONB
 * @param[in] invert True if the arguments must be inverted
 * @csqlfn #Concat_jsonbset_jsonbset()
 */
Set *
concat_jsonbset_jsonb(const Set *set, const Jsonb *jb, bool invert)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(jb, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) datum_jsonb_concat;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = PointerGetDatum(jb);
  lfinfo.restype = T_JSONBSET;
  lfinfo.invert = invert;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract an array element from a JSONB set
 * @param[in] set JSONB set
 * @param[in] idx Index
 * @param[in] astext True when the output is a text set
 * @param[in] null_handle States the null value treatment
 */
Set *
jsonbset_array_element(const Set *set, int idx, bool astext,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = astext ? (varfunc) &datum_jsonb_array_element_text :
    (varfunc) &datum_jsonb_array_element;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = DatumGetInt32(idx);
  lfinfo.restype = astext ? T_TEXTSET : T_JSONBSET;
  lfinfo.resnull = null_handle; /* Handle NULL result */
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Delete a key specified by an index from a JSONB set
 * @param[in] set JSONB set
 * @param[in] idx Index
 */
Set *
jsonbset_delete_index(const Set *set, int idx)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_delete_index;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = DatumGetInt32(idx);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Delete a key or an array element from a JSONB set
 * @param[in] set JSONB set
 * @param[in] key Key
 */
Set *
jsonbset_delete(const Set *set, const text *key)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(key, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_delete;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = PointerGetDatum(key);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/**
 * @ingroup meos_json_set_json
 * @brief Delete an array of keys from a JSONB set
 * @param[in] set JSONB set
 * @param[in] keys Keys
 * @param[in] count Number of elements in the input array
 */
Set *
jsonbset_delete_array(const Set *set, text **keys, int count)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(keys, NULL);
  if (! ensure_positive(count))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_delete_array;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 2;
  lfinfo.param[0] = PointerGetDatum(keys);
  lfinfo.param[1] = Int32GetDatum(count);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Delete a key from a JSONB set
 * @param[in] set JSONB set
 * @param[in] key Key
 */
Set *
jsonbset_exists(const Set *set, const text *key)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(key, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_exists;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = PointerGetDatum(key);
  lfinfo.restype = T_TBOOL;
  return lfunc_set(set, &lfinfo);
}

/**
 * @ingroup meos_json_set_json
 * @brief Delete an array of keys from a JSONB set
 * @param[in] set JSONB set
 * @param[in] keys Keys
 * @param[in] count Number of elements in the input array
 * @param[in] any True for the 'any' semantics, false for the 'all' semantics
 */
Set *
jsonbset_exists_array(const Set *set, text **keys, int count, bool any)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(keys, NULL);
  if (! ensure_positive(count))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_exists_array;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(keys);
  lfinfo.param[1] = Int32GetDatum(count);
  lfinfo.param[2] = BoolGetDatum(any);
  lfinfo.restype = T_TBOOL;
  return lfunc_set(set, &lfinfo);
}

/**
 * @ingroup meos_json_set_json
 * @brief Replace a value specified by a path with a new value in a JSONB set
 * @param[in] set JSONB set
 * @param[in] keys Keys
 * @param[in] count Number of elements in the input array
 * @param[in] newjb New value
 * @param[in] create When true, if the last path step is an array index that
 * is out of range, the new value is added at the beginning of the array if
 * the index is negative, or at the end of the array if it is positive
 * @param[in] null_handle States the null value treatment, which must be one of
 * 'raise_exception', 'use_json_null', 'delete_key', or 'return_target'
 * @param[in] lax True when the lax mode is used
 */
Set *
jsonbset_set(const Set *set, text **keys, int count, const Jsonb *newjb,
  bool create, const text *null_handle, bool lax)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(keys, NULL);
  if (! ensure_positive(count))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = lax ?
    (varfunc) &datum_jsonb_set_lax : (varfunc) &datum_jsonb_set;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = lax ? 5 : 4;
  lfinfo.param[0] = PointerGetDatum(keys);
  lfinfo.param[1] = Int32GetDatum(count);
  lfinfo.param[2] = PointerGetDatum(newjb);
  lfinfo.param[3] = BoolGetDatum(create);
  if (lax)
    lfinfo.param[4] = PointerGetDatum(null_handle);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @brief Convert a JSONB set into a alphanumeric set by extracting one key
 * @param[in] set JSONB set
 * @param[in] key Key to extract from the JSONB object
 * @param[in] resbasetype Resulting base type
 * @param[in] interp Interpolation
 * @param[in] null_handle States the null value treatment
 * @note Supported JSONB types: boolean, numeric, string
 */
Set *
jsonbset_to_alphanumset(const Set *set, const char *key, MeosType resbasetype,
  nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  assert(set); assert(key); assert(set->settype == T_JSONBSET);
  assert(alphanum_basetype(resbasetype));

  MeosType restype = basetype_settype(resbasetype);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_to_alphanum;
  lfinfo.argtype[0] = T_JSONB;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(key);
  lfinfo.param[1] = resbasetype;
  lfinfo.param[2] = null_handle;
  lfinfo.restype = restype;
  lfinfo.resnull = null_handle; /* Handle NULL result */
  return lfunc_set(set, &lfinfo);
}

#if MEOS
/**
 * @ingroup meos_json_set_json
 * @brief Convert a JSONB set to a temporal integer by extracting one key
 * @param[in] set JSONB set object
 * @param[in] key Key to extract
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_to_intset()
 */
Set *
jsonbset_to_intset(const Set *set, const char *key,
  nullHandleType null_handle)
{
  return jsonbset_to_alphanumset(set, key, T_INT4, null_handle);
}

/**
 * @ingroup meos_json_set_json
 * @brief Convert a JSONB set to a float set by extracting one key
 * @param[in] set JSONB set object
 * @param[in] key Key to extract
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_to_floatset()
 */
Set *
jsonbset_to_floatset(const Set *set, const char *key,
  nullHandleType null_handle)
{
  return jsonbset_to_alphanumset(set, key, T_FLOAT8, null_handle);
}

/**
 * @ingroup meos_json_set_json
 * @brief Convert a JSONB set to a text set by extracting one key
 * @param[in] set JSONB set object
 * @param[in] key Key to extract
 * @param[in] null_handle States the null value treatment
 * @csqlfn #Jsonbset_to_ttextset_key()
 */
Set *
jsonbset_to_textset_key(const Set *set, const char *key,
  nullHandleType null_handle)
{
  return jsonbset_to_alphanumset(set, key, T_TEXT, null_handle);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Return a JSONB set without nulls
 * @param[in] set JSONB set
 * @param[in] strip_in_arrays True when nulls are removed from the arrays
 * @csqlfn #Jsonbset_strip_nulls()
 */
Set *
jsonbset_strip_nulls(const Set *set, bool strip_in_arrays)
{
  /* Ensure the validity of the arguments */
  assert(set); assert(set->settype == T_JSONBSET);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_strip_nulls;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 1;
  lfinfo.param[0] = BoolGetDatum(strip_in_arrays);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Convert a JSONB set to pretty-printed, indented text
 * @param[in] set JSONB set
 * @csqlfn #Jsonbset_pretty()
 */
Set *
jsonbset_pretty(const Set *set)
{
  /* Ensure the validity of the arguments */
  assert(set); assert(set->settype == T_JSONBSET);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_pretty;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.restype = T_TEXTSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************
* Path functions
 *****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Delete an item specified by a path from a JSONB set
 * @param[in] set JSONB set
 * @param[in] path_elems Array of path elements
 * @param[in] path_len Number of elements in the input array
 */
Set *
jsonbset_delete_path(const Set *set, text **path_elems, int path_len)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_delete_path;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 2;
  lfinfo.param[0] = PointerGetDatum(path_elems);
  lfinfo.param[1] = Int32GetDatum(path_len);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract a JSONB set specified by a path
 * @param[in] set JSONB set
 * @param[in] path_elems Keys
 * @param[in] path_len Number of elements in the input array
 * @param[in] astext True when the output is a text set
 * @param[in] null_handle States the null value treatment
 */
Set *
jsonbset_extract_path(const Set *set, text **path_elems, int path_len,
  bool astext, nullHandleType null_handle)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = astext ? (varfunc) &datum_jsonb_extract_path_text :
    (varfunc) &datum_jsonb_extract_path;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 2;
  lfinfo.param[0] = PointerGetDatum(path_elems);
  lfinfo.param[1] = Int32GetDatum(path_len);
  lfinfo.restype = astext ? T_TEXTSET : T_JSONBSET;
  lfinfo.resnull = null_handle; /* Handle NULL result */
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Replace a JSONB set specified by a path with a new value
 * @details If the item designated by the path is an array element, new_value
 * will be inserted before that item if insert_after is false (which is the
 * default), or after it if insert_after is true. If the item designated by
 * the path is an object field, new_value will be inserted only if the object
 * does not already contain that key. All earlier steps in the path must exist,
 * or the target is returned unchanged. As with the path oriented operators,
 * negative integers that appear in the path count from the end of JSON arrays.
 * If the last path step is an array index that is out of range, the new value
 * is added at the beginning of the array if the index is negative, or at the
 * end of the array if it is positive.
 * @param[in] set JSONB set
 * @param[in] path_elems Array of path elements
 * @param[in] path_len Number of elements in the input array
 * @param[in] newjb New value
 * @param[in] after When true, if the last path step is an array index that
 * is out of range, the new value is added at the beginning of the array if
 * the index is negative, or at the end of the array if it is positive
 */
Set *
jsonbset_insert(const Set *set, text **path_elems, int path_len,
  const Jsonb *newjb, bool after)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(path_elems, NULL);
  if (! ensure_positive(path_len))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_insert;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 4;
  lfinfo.param[0] = PointerGetDatum(path_elems);
  lfinfo.param[1] = Int32GetDatum(path_len);
  lfinfo.param[2] = PointerGetDatum(newjb);
  lfinfo.param[3] = BoolGetDatum(after);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Return true if a JSON path expression returns at least one item for a
 * JSONB set
 * @param[in] set JSONB set
 * @param[in] jp JSON path expression
 * @param[in] vars JSON variables, may be NULL
 * @param[in] silent When true, the following errors are suppressed: missing
 * object field or array element, unexpected JSON item type, datetime and
 * numeric errors, when false, no errors are suppressed
 * @param[in] tz When true, support comparisons of date/time values that
 * require timezone-aware conversions, false otherwise
 */
Set *
jsonbset_path_exists(const Set *set, const JsonPath *jp, const Jsonb *vars,
  bool silent, bool tz)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(jp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_path_exists;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(jp);
  lfinfo.param[1] = PointerGetDatum(vars);
  lfinfo.param[2] = BoolGetDatum(silent);
  lfinfo.param[3] = BoolGetDatum(tz);
  lfinfo.restype = T_TBOOL;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract an item specified by a JSON path expression from a temporal
 * JSONB value
 * predicate
 * @param[in] set JSONB set
 * @param[in] jp JSON path expression
 * @param[in] vars JSON variables, may be NULL
 * @param[in] silent When true, the following errors are suppressed: missing
 * object field or array element, unexpected JSON item type, datetime and
 * numeric errors, when false, no errors are suppressed
 * @param[in] tz When true, support comparisons of date/time values that
 * require timezone-aware conversions, false otherwise
 */
Set *
jsonbset_path_match(const Set *set, const JsonPath *jp, const Jsonb *vars,
  bool silent, bool tz)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(jp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_path_match;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(jp);
  lfinfo.param[1] = PointerGetDatum(vars);
  lfinfo.param[2] = BoolGetDatum(silent);
  lfinfo.param[3] = BoolGetDatum(tz);
  lfinfo.restype = T_TBOOL;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract the items specified by a JSON path expression from a temporal
 * JSONB value
 * @param[in] set JSONB set
 * @param[in] jp JSON path expression
 * @param[in] vars JSON variables, may be NULL
 * @param[in] silent When true, the following errors are suppressed: missing
 * object field or array element, unexpected JSON item type, datetime and
 * numeric errors, when false, no errors are suppressed
 * @param[in] tz When true, support comparisons of date/time values that
 * require timezone-aware conversions, false otherwise
 */
Set *
jsonbset_path_query_array(const Set *set, const JsonPath *jp,
  const Jsonb *vars, bool silent, bool tz)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(jp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_path_query_array;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(jp);
  lfinfo.param[1] = PointerGetDatum(vars);
  lfinfo.param[2] = BoolGetDatum(silent);
  lfinfo.param[3] = BoolGetDatum(tz);
  lfinfo.restype = T_JSONBSET;
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_set_json
 * @brief Extract the first item specified by a JSON path expression from a
 * JSONB set. If there are no items, return NULL.
 * @param[in] set JSONB set
 * @param[in] jp JSON path expression
 * @param[in] vars JSON variables, may be NULL
 * @param[in] silent When true, the following errors are suppressed: missing
 * object field or array element, unexpected JSON item type, datetime and
 * numeric errors, when false, no errors are suppressed
 * @param[in] tz When true, support comparisons of date/time values that
 * require timezone-aware conversions, false otherwise
 */
Set *
jsonbset_path_query_first(const Set *set, const JsonPath *jp,
  const Jsonb *vars, bool silent, bool tz)
{
  /* Ensure the validity of the arguments */
  VALIDATE_JSONBSET(set, NULL); VALIDATE_NOT_NULL(jp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_jsonb_path_query_first;
  lfinfo.argtype[0] = T_JSONBSET;
  lfinfo.numparam = 3;
  lfinfo.param[0] = PointerGetDatum(jp);
  lfinfo.param[1] = PointerGetDatum(vars);
  lfinfo.param[2] = BoolGetDatum(silent);
  lfinfo.param[3] = BoolGetDatum(tz);
  lfinfo.restype = T_JSONBSET;
  lfinfo.resnull = NULL_DELETE; /* Handle NULL result */
  return lfunc_set(set, &lfinfo);
}

/*****************************************************************************/

