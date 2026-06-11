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
 * @brief Basic functions for temporal JSONB
 */

#ifndef __TJSONB_FUNCS_H__
#define __TJSONB_FUNCS_H__

/* PostgreSQL */
#include <postgres.h>
#include <utils/jsonb.h>
#include <utils/jsonpath.h>
/* MEOS */
#include <meos.h>
#include <meos_json.h>
#include "temporal/temporal.h"

/* Operations available for setPath */
#define JB_PATH_CREATE               0x0001
#define JB_PATH_DELETE               0x0002
#define JB_PATH_REPLACE              0x0004
#define JB_PATH_INSERT_BEFORE        0x0008
#define JB_PATH_INSERT_AFTER         0x0010
#define JB_PATH_FILL_GAPS            0x0020
#define JB_PATH_CONSISTENT_POSITION  0x0040
#define JB_PATH_CREATE_OR_INSERT \
  (JB_PATH_INSERT_BEFORE | JB_PATH_INSERT_AFTER | JB_PATH_CREATE)

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

/*****************************************************************************
 * JSONB internal operations
 *****************************************************************************/

extern JsonbValue *setPath(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, int op_type);
extern void setPathObject(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, uint32_t npairs, int op_type);
extern void setPathArray(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, uint32_t nelems, int op_type);

/*****************************************************************************
 * Datum‐level JSONB operations used by the lifting infrastructure
 *****************************************************************************/

extern Datum datum_jsonb_concat(Datum l, Datum r);
extern Datum datum_jsonb_contained(Datum l, Datum r);
extern Datum datum_jsonb_contains(Datum l, Datum r);
extern Datum datum_jsonb_delete(Datum jb, Datum key);
extern Datum datum_jsonb_delete_array(Datum jb, Datum array, Datum count);
extern Datum datum_jsonb_delete_index(Datum jb, Datum idx);
extern Datum datum_json_array_element(Datum txt, Datum element);
extern Datum datum_jsonb_array_element(Datum jb, Datum element);
extern Datum datum_json_array_element_text(Datum txt, Datum element);
extern Datum datum_jsonb_array_element_text(Datum jb, Datum element);
extern Datum datum_jsonb_exists(Datum l, Datum r);
extern Datum datum_jsonb_exists_array(Datum value, Datum array, Datum count, Datum any);
extern Datum datum_json_array_length(Datum txt);
extern Datum datum_jsonb_array_length(Datum txt);
extern Datum datum_json_object_field(Datum txt, Datum key);
extern Datum datum_jsonb_object_field(Datum jb, Datum key);
extern Datum datum_json_object_field_text(Datum txt, Datum key);
extern Datum datum_jsonb_object_field_text(Datum jb, Datum key);
extern Datum datum_json_strip_nulls(Datum txt, Datum strip_in_arrays);
extern Datum datum_jsonb_strip_nulls(Datum jb, Datum strip_in_arrays);
extern Datum datum_jsonb_pretty(Datum jb);
extern Datum datum_json_extract_path(Datum txt, Datum path_elems, Datum path_len);
extern Datum datum_jsonb_extract_path(Datum jb, Datum path_elems, Datum path_len);
extern Datum datum_json_extract_path_text(Datum txt, Datum path_elems, Datum path_len);
extern Datum datum_jsonb_extract_path_text(Datum jb, Datum path_elems, Datum path_len);
extern Datum datum_jsonb_set(Datum jb, Datum keys, Datum count, Datum newjb, Datum create);
extern Datum datum_jsonb_set_lax(Datum jb, Datum keys, Datum count, Datum newjb, Datum create, Datum null_handle);
extern Datum datum_jsonb_delete_path(Datum jb, Datum keys, Datum count);
extern Datum datum_jsonb_insert(Datum jb, Datum keys, Datum count, Datum newjb, Datum after);
extern Datum datum_jsonb_path_exists(Datum jb, Datum jp, Datum vars, Datum silent, Datum tz);
extern Datum datum_jsonb_path_match(Datum jb, Datum jp, Datum vars, Datum silent, Datum tz);
extern Datum datum_jsonb_path_query_array(Datum jb, Datum jp, Datum vars, Datum silent, Datum tz);
extern Datum datum_jsonb_path_query_first(Datum jb, Datum jp, Datum vars, Datum silent, Datum tz);
extern Datum datum_jsonb_to_text(Datum jb);
extern Datum datum_text_to_jsonb(Datum txt);
extern Datum datum_jsonb_to_alphanum(Datum jb, Datum key, Datum temptype, Datum null_handle);

/*****************************************************************************
 * Temporal JSONB operations
 *****************************************************************************/

extern Temporal *tjsonb_to_talphanum(const Temporal *temp, const char *key,
  MeosType resbasetype, interpType interp, nullHandleType null_handle);

/*****************************************************************************
 * Set wrappers for JSONB operations
 *****************************************************************************/

extern Set *jsonbfunc_jsonbset(const Set *s, datum_func1 func, MeosType intype,
  MeosType restype);
extern Set *jsonbfunc_jsonbset_jsonb(const Set *s, const Jsonb *jb,
  datum_func2 func, bool invert);
extern Set *jsonbfunc_jsonbset_text(const Set *s, const text *txt,
  datum_func2 func);

/*****************************************************************************
 * Temporal wrappers for JSONB operations
 *****************************************************************************/

extern Temporal *tjsonb_path_exists(const Temporal *temp, const JsonPath *jp, 
  const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_match(const Temporal *temp, const JsonPath *jp,
  const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_query_array(const Temporal *temp,
  const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);
extern Temporal *tjsonb_path_query_first(const Temporal *temp,
  const JsonPath *jp, const Jsonb *vars, bool silent, bool tz);

/*****************************************************************************/

#endif /* __TJSONB_FUNCS_H__ */
