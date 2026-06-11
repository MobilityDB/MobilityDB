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
 * @brief Functions for JSON types corresponding to the external (SQL)
 * PostgreSQL functions
 */

#ifndef __PG_JSON_H__
#define __PG_JSON_H__

typedef struct varlena;
typedef struct varlena text __attribute__((aligned(8)));

typedef uint32 JEntry;
typedef struct JsonbContainer
{
  uint32 header; /* number of elements or key/value pairs, and flags */
  JEntry children[];
  /* the data for each child node follows. */
} JsonbContainer;
typedef struct
{
  int32 vl_len_;    /* varlena header (do not touch directly!) */
  JsonbContainer root;
} Jsonb;

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
extern jsonb_to_text(const Jsonb *jb);


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
extern Jsonb **jsonb_path_query_all(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Jsonb *jsonb_path_query_array(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Jsonb *jsonb_path_query_first(const Jsonb *jb, const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern JsonPath *jsonpath_in(const char *str);
extern JsonPath *jsonpath_copy(const JsonPath *jp);
extern char *jsonpath_out(const JsonPath *jp);

/*****************************************************************************/

#endif /* __PG_JSON_H__ */
