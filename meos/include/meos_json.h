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
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 */

#ifndef __MEOS_JSON_H__
#define __MEOS_JSON_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* JSON-C */
#include <json-c/json.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
/*****************************************************************************
 * Public base JSON / JSONB / JSONPATH API
 *
 * The full base-type surface (json/jsonb input-output, the json accessors,
 * jsonb conversions, and jsonpath query) is declared here -- in the public
 * meos/include tree -- so that every binding generated from the MEOS-API
 * catalog gets the complete API over the base jsonb type, not only the
 * temporal tjsonb wrapper. The implementations live in the vendored pgtypes
 * sources; the Jsonb/JsonPath/text/Numeric types come from <pgtypes.h> above.
 *****************************************************************************/

/*****************************************************************************/

/* Input and output functions */


extern text *json_in(const char *str);
extern char *json_out(const text *js);
extern Jsonb *jsonb_from_text(const text *txt, bool unique_keys);
extern Jsonb *jsonb_in(const char *str);
extern char *jsonb_out(const Jsonb *jb);

/* Constructor functions */

extern text *json_make(text **keys_vals, int count);
extern text *json_make_two_arg(text **keys, text **values, int count);
extern Jsonb *jsonb_copy(const Jsonb *jb);
extern Jsonb *jsonb_make(text **keys_vals, int count);
extern Jsonb *jsonb_make_two_arg(text **keys, text **values, int count);

/* Conversion functions */

extern bool jsonb_to_bool(const Jsonb *jb);
extern char *jsonb_to_cstring(const Jsonb *jb);
extern float4 jsonb_to_float4(const Jsonb *jb);
extern float8 jsonb_to_float8(const Jsonb *jb);
extern int16 jsonb_to_int16(const Jsonb *jb);
extern int32 jsonb_to_int32(const Jsonb *jb);
extern int64 jsonb_to_int64(const Jsonb *jb);
extern Numeric jsonb_to_numeric(const Jsonb *jb);
extern text *jsonb_to_text(const Jsonb *jb);


/* Accessor functions */

extern text *json_array_element(const text *js, int element);
extern text *json_array_element_text(const text *js, int element);
extern text **json_array_elements(const text *js, int *count);
extern text **json_array_elements_text(const text *js, int *count);
extern int json_array_length(const text *js);
extern text **json_each(const text *js, text **values, int *count);
extern text **json_each_text(const text *js, text **values, int *count);
extern text *json_extract_path(const text *js, text **path_elems, int path_len);
extern text *json_extract_path_text(const text *js, text **path_elems, int path_len);
extern text *json_object_field(const text *js, const text *key);
extern text *json_object_field_text(const text *js, const text *key);
extern text **json_object_keys(const text *js, int *count);
extern text *json_typeof(const text *js);

extern Jsonb *jsonb_array_element(const Jsonb *jb, int element);
extern text *jsonb_array_element_text(const Jsonb *jb, int element);
extern Jsonb **jsonb_array_elements(const Jsonb *jb, int *count);
extern text **jsonb_array_elements_text(const Jsonb *jb, int *count);
extern int jsonb_array_length(const Jsonb *jb);
extern bool jsonb_contained(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_contains(const Jsonb *jb1, const Jsonb *jb2);
extern text **jsonb_each(const Jsonb *jb, Jsonb **values, int *count);
extern text **jsonb_each_text(const Jsonb *jb, text **values, int *count);
extern bool jsonb_exists(const Jsonb *jb, const text *key);
extern bool jsonb_exists_array(const Jsonb *jb, text **keys_elems, int keys_len, bool any);
extern Jsonb *jsonb_extract_path(const Jsonb *jb, text **path_elems, int path_len);
extern text *jsonb_extract_path_text(const Jsonb *jb, text **path_elems, int path_len);
extern uint32 jsonb_hash(const Jsonb *jb);
extern uint64 jsonb_hash_extended(const Jsonb *jb, uint64 seed);
extern Jsonb *jsonb_object_field(const Jsonb *jb, const text *key);
extern text *jsonb_object_field_text(const Jsonb *jb, const text *key);
extern text **jsonb_object_keys(const Jsonb *jb, int *count);

/* Transformation functions */

extern text *json_strip_nulls(const text *js, bool strip_in_arrays);
extern Jsonb *jsonb_concat(const Jsonb *jb1, const Jsonb *jb2);
extern Jsonb *jsonb_delete(const Jsonb *jb, const text *key);
extern Jsonb *jsonb_delete_array(const Jsonb *jb, text **keys_elems, int keys_len);
extern Jsonb *jsonb_delete_index(const Jsonb *jb, int idx);
extern Jsonb *jsonb_delete_path(const Jsonb *jb, text **path_elems, int path_len);
extern Jsonb *jsonb_insert(const Jsonb *jb, text **path_elems, int path_len, const Jsonb *newjb, bool after);
extern text *jsonb_pretty(const Jsonb *jb);
extern Jsonb *jsonb_set(const Jsonb *jb, text **path_elems, int path_len, const Jsonb *newjb, bool create);
extern Jsonb *jsonb_set_lax(const Jsonb *jb, text **path_elems, int path_len, const Jsonb *newjb, bool create, const text *handle_null);
extern Jsonb *jsonb_strip_nulls(const Jsonb *jb, bool strip_in_arrays);

/* Comparison functions */

extern int jsonb_cmp(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_eq(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_ge(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_gt(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_le(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_lt(const Jsonb *jb1, const Jsonb *jb2);
extern bool jsonb_ne(const Jsonb *jb1, const Jsonb *jb2);

/* JSON path functions */

extern int jsonb_path_exists(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern bool jsonb_path_match(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Jsonb **jsonb_path_query_all(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz, int *count);
extern Jsonb *jsonb_path_query_array(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Jsonb *jsonb_path_query_first(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern JsonPath *jsonpath_in(const char *str);
extern JsonPath *jsonpath_copy(const JsonPath *jp);
extern char *jsonpath_out(const JsonPath *jp);

/*****************************************************************************/

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/**
 * @brief Enumeration that defines the null-handling types used in MEOS
 */
typedef enum
{
  NULL_INVALID =   0,
  NULL_ERROR =     1,
  NULL_JSON_NULL = 2,
  NULL_DELETE =    3,
  NULL_RETURN =    4,
} nullHandleType;

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the set passed as argument is a JSONB set
 */
#if MEOS
  #define VALIDATE_JSONBSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) set) || \
              ! ensure_set_isof_type((set), T_JSONBSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_JSONBSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_JSONBSET); \
    } while (0)
#endif

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal JSONB
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TJSONB(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TJSONB) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TJSONB(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TJSONB); \
    } while (0)
#endif

/******************************************************************************
 * Functions for JSONB sets
 ******************************************************************************/

/* Input and output functions */

extern Set *jsonbset_in(const char *str);
extern char *jsonbset_out(const Set *s, int maxdd);

/* Constructor functions */

extern Set *jsonbset_make(const Jsonb **values, int count);

/* Conversion functions */

extern Set *jsonb_to_set(const Jsonb *jb);

/* Accessor functions */

extern Jsonb *jsonbset_end_value(const Set *s);
extern Jsonb *jsonbset_start_value(const Set *s);
extern bool jsonbset_value_n(const Set *s, int n, Jsonb **result);
extern Jsonb **jsonbset_values(const Set *s, int *count);

/* JSON operations */

extern Set *concat_jsonbset_jsonb(const Set *s, const Jsonb *jb, bool invert);
extern Set *jsonbset_array_length(const Set *set);
extern Set *jsonbset_object_field(const Set *set, const text *key, bool astext, nullHandleType null_handle);
extern Set *jsonbset_array_element(const Set *set, int idx, bool astext, nullHandleType null_handle);
extern Set *jsonbset_delete_index(const Set *set, int idx);
extern Set *jsonbset_delete(const Set *set, const text *key);
extern Set *jsonbset_delete_array(const Set *set, text **keys, int count);
extern Set *jsonbset_exists(const Set *set, const text *key);
extern Set *jsonbset_exists_array(const Set *set, text **keys, int count, bool any);
extern Set *jsonbset_set(const Set *set, text **keys, int count, const Jsonb *newjb, bool create, const text *null_handle, bool lax);
extern Set *jsonbset_to_alphanumset(const Set *set, const char *key, MeosType settype, nullHandleType null_handle);
extern Set *jsonbset_to_intset(const Set *set, const char *key, nullHandleType null_handle);
extern Set *jsonbset_to_bigintset(const Set *set, const char *key, nullHandleType null_handle);
extern Set *jsonbset_to_floatset(const Set *set, const char *key, nullHandleType null_handle);
extern Set *jsonbset_to_textset_key(const Set *set, const char *key, nullHandleType null_handle);
extern Set *jsonbset_strip_nulls(const Set *set, bool strip_in_arrays);
extern Set *jsonbset_pretty(const Set *set);
extern Set *jsonbset_delete_path(const Set *set, text **path_elems, int path_len);
extern Set *jsonbset_extract_path(const Set *set, text **path_elems, int path_len, bool astext, nullHandleType null_handle);
extern Set *jsonbset_insert(const Set *set, text **path_elems, int path_len, const Jsonb *newjb, bool after);
extern Set *jsonbset_path_exists(const Set *set, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Set *jsonbset_path_match(const Set *set, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Set *jsonbset_path_query_array(const Set *set, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Set *jsonbset_path_query_first(const Set *set, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);

/* Set operations */

extern bool contained_jsonb_set(const Jsonb *jb, const Set *s);
extern bool contains_set_jsonb(const Set *s, Jsonb *jb);
extern Set *intersection_jsonb_set(const Jsonb *jb, const Set *s);
extern Set *intersection_set_jsonb(const Set *s, const Jsonb *jb);
extern Set *jsonb_union_transfn(Set *state, const Jsonb *jb);
extern Set *minus_jsonb_set(const Jsonb *jb, const Set *s);
extern Set *minus_set_jsonb(const Set *s, const Jsonb *jb);
extern Set *union_jsonb_set(const Jsonb *jb, const Set *s);
extern Set *union_set_jsonb(const Set *s, const Jsonb *jb);

/*===========================================================================*
 * Functions for temporal JSONB
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern Temporal *tjsonb_from_mfjson(const char *str);
extern Temporal *tjsonb_in(const char *str);
extern char *tjsonb_out(const Temporal *temp);

// Internal
extern TInstant *tjsonbinst_from_mfjson(json_object *mfjson);
extern TInstant *tjsonbinst_in(const char *str);
extern TSequence *tjsonbseq_from_mfjson(json_object *mfjson);
extern TSequence *tjsonbseq_in(const char *str, interpType interp);
extern TSequenceSet *tjsonbseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tjsonbseqset_in(const char *str);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern Temporal *tjsonb_from_base_temp(const Jsonb *jsonb, const Temporal *temp);
extern TInstant *tjsonbinst_make(const Jsonb *jsonb, TimestampTz t);
extern TSequence *tjsonbseq_from_base_tstzset(const Jsonb *jsonb, const Set *s);
extern TSequence *tjsonbseq_from_base_tstzspan(const Jsonb *jsonb, const Span *sp);
extern TSequenceSet *tjsonbseqset_from_base_tstzspanset(const Jsonb *jsonb, const SpanSet *ss);

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

extern Temporal *tjsonb_to_ttext(const Temporal *temp);
extern Temporal *ttext_to_tjsonb(const Temporal *temp);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

extern Jsonb *tjsonb_end_value(const Temporal *temp);
extern Jsonb *tjsonb_start_value(const Temporal *temp);
extern bool tjsonb_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict,  Jsonb **value);
extern bool tjsonb_value_n(const Temporal *temp, int n, Jsonb **result);
extern Jsonb **tjsonb_values(const Temporal *temp, int *count);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/*****************************************************************************
 * Aggregate functions
 *****************************************************************************/


/*****************************************************************************
 * Temporal JSON functions
 *****************************************************************************/

extern Temporal *concat_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb, bool invert);
extern Temporal *concat_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);
extern Temporal *contains_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb, bool invert);
extern Temporal *contains_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);
extern nullHandleType null_handle_type_from_string(const char *str);

extern Temporal *tjson_array_element(const Temporal *temp, int idx, nullHandleType null_handle);
extern Temporal *tjson_array_length(const Temporal *temp);
extern Temporal *tjson_extract_path(const Temporal *temp, text **path_elems, int path_len, nullHandleType null_handle);
extern Temporal *tjson_object_field(const Temporal *temp, const text *key, bool astext, nullHandleType null_handle);
extern Temporal *tjson_strip_nulls(const Temporal *temp, bool strip_in_arrays);

extern Temporal *tjsonb_array_element(const Temporal *temp, int idx, bool astext, nullHandleType null_handle);
extern Temporal *tjsonb_array_length(const Temporal *temp);
extern Temporal *tjsonb_delete(const Temporal *temp, const text *key);
extern Temporal *tjsonb_delete_array(const Temporal *temp, text **keys, int count);
extern Temporal *tjsonb_delete_index(const Temporal *temp, int idx);
extern Temporal *tjsonb_delete_path(const Temporal *temp, text **path_elems, int path_len);
extern Temporal *tjsonb_exists(const Temporal *temp, const text *key);
extern Temporal *tjsonb_exists_array(const Temporal *temp, text **keys, int count, bool any);
extern Temporal *tjsonb_extract_path(const Temporal *temp, text **path_elems, int path_len, bool astext, nullHandleType null_handle);
extern Temporal *tjsonb_insert(const Temporal *temp, text **keys, int count, const Jsonb *newjb, bool after);
extern Temporal *tjsonb_object_field(const Temporal *temp, const text *key, bool astext, nullHandleType null_handle);
extern Temporal *tjsonb_path_exists(const Temporal *temp, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_match(const Temporal *temp, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_query_array(const Temporal *temp, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_query_first(const Temporal *temp, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_pretty(const Temporal *temp);
extern Temporal *tjsonb_set(const Temporal *temp, text **keys, int count, const Jsonb *newjb, bool create, const text *handle_null, bool lax);
extern Temporal *tjsonb_strip_nulls(const Temporal *temp, bool strip_in_arrays);
extern Temporal *tjsonb_to_tbool(const Temporal *temp, const char *key, nullHandleType null_handle);
extern Temporal *tjsonb_to_tfloat(const Temporal *temp, const char *key, interpType interp, nullHandleType null_handle);
extern Temporal *tjsonb_to_tint(const Temporal *temp, const char *key, nullHandleType null_handle);
extern Temporal *tjsonb_to_ttext_key(const Temporal *temp, const char *key, nullHandleType null_handle);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

extern Temporal *tjsonb_at_value(const Temporal *temp, const Jsonb *jsb);
extern Temporal *tjsonb_minus_value(const Temporal *temp, const Jsonb *jsb);

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/* Ever/always and temporal comparison functions */

extern int always_eq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern int always_eq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);
extern int always_eq_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern int always_ne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);
extern int always_ne_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern int ever_eq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);
extern int ever_eq_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern int ever_ne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);
extern int ever_ne_tjsonb_tjsonb(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

extern Temporal *teq_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern Temporal *teq_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);
extern Temporal *tne_jsonb_tjsonb(const Jsonb *jb, const Temporal *temp);
extern Temporal *tne_tjsonb_jsonb(const Temporal *temp, const Jsonb *jb);

/*****************************************************************************/

#endif /* __MEOS_JSON_H__ */
