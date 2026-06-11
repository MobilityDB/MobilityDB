/*-------------------------------------------------------------------------
 *
 * jsonfuncs.c
 *    Functions to process JSON data types.
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/jsonfuncs.c
 *
 *-------------------------------------------------------------------------
 */

/* C */
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
/* PostgreSQL */
#include <postgres.h>
#include "catalog/pg_type.h"
#include <common/hashfn.h>
#include <common/int.h>
#include <common/jsonapi.h>
#include <nodes/nodes.h>
#include "utils/builtins.h"
#include <utils/hsearch.h>
#include <utils/json.h>
#include <utils/jsonb.h>
#include "utils/jsonfuncs.h"
#include <utils/varlena.h> /* For DatumGetTextP */
#include <utils/date.h>

#include <pgtypes.h>
#include "../../meos/include/meos_error.h"

extern int GetDatabaseEncoding(void);

/* TODO REMOVE */
extern int strtoint(const char *pg_restrict str, char **pg_restrict endptr, int base);

// #include "access/htup_details.h"
// #include "catalog/pg_type.h"
// #include "common/int.h"
// #include "common/jsonapi.h"
// #include "common/string.h"
// #include "lib/stringinfo.h"
// #include "utils/mb/pg_wchar.h"
// #include "miscadmin.h"
// #include "nodes/miscnodes.h"
// #include "parser/parse_coerce.h"
// #include "utils/array.h"
// #include "utils/builtins.h"
// #include "utils/fmgroids.h"
// #include "utils/hsearch.h"
// #include "utils/json.h"
// #include "utils/jsonb.h"
// #include "utils/jsonfuncs.h"
// #include "utils/lsyscache.h"
// #include "utils/memutils.h"
// #include "utils/syscache.h"
// #include "utils/typcache.h"

/* Operations available for setPath */
#define JB_PATH_CREATE           0x0001
#define JB_PATH_DELETE           0x0002
#define JB_PATH_REPLACE          0x0004
#define JB_PATH_INSERT_BEFORE    0x0008
#define JB_PATH_INSERT_AFTER     0x0010
#define JB_PATH_CREATE_OR_INSERT \
  (JB_PATH_INSERT_BEFORE | JB_PATH_INSERT_AFTER | JB_PATH_CREATE)
#define JB_PATH_FILL_GAPS        0x0020
#define JB_PATH_CONSISTENT_POSITION    0x0040

/* state for json_object_keys */
typedef struct OkeysState
{
  JsonLexContext *lex;
  char **result;
  int result_size;
  int result_count;
} OkeysState;

/* state for iterate_json_values function */
typedef struct IterateJsonStringValuesState
{
  JsonLexContext *lex;
  JsonIterateStringValuesAction action;  /* an action that will be applied
                       * to each json value */
  void *action_state;  /* any necessary context for iteration */
  uint32 flags;      /* what kind of elements from a json we want to iterate */
} IterateJsonStringValuesState;

/* state for transform_json_string_values function */
typedef struct TransformJsonStringValuesState
{
  JsonLexContext *lex;
  StringInfo strval;      /* resulting json */
  JsonTransformStringValuesAction action; /* an action that will be applied
                       * to each json value */
  void *action_state;  /* any necessary context for transformation */
} TransformJsonStringValuesState;

/* state for json_get* functions */
typedef struct GetState
{
  JsonLexContext *lex;
  text     *tresult;
  const    char *result_start;
  bool     normalize_results;
  bool     next_scalar;
  int      path_len;      /* length of each path-related array */
  char   **path_names;    /* field name(s) being sought */
  int     *path_indexes;  /* array index(es) being sought */
  bool    *pathok;        /* is path matched to current depth? */
  int     *array_cur_index;  /* current element index at each path level */
} GetState;

/* state for json_array_length */
typedef struct AlenState
{
  JsonLexContext *lex;
  int count;
} AlenState;

/* state for json_each */
typedef struct EachState
{
  JsonLexContext *lex;
  text **keys;
  text **values;
  int result_size;
  int result_count;
  const char *result_start;
  bool normalize_results;
  bool next_scalar;
  char *normalized_scalar;
} EachState;

/* state for json_array_elements */
typedef struct ElementsState
{
  JsonLexContext *lex;
  const char *function_name;
  text **result;
  int result_size;
  int result_count;
  const char *result_start;
  bool normalize_results;
  bool next_scalar;
  char *normalized_scalar;
} ElementsState;

/* state for get_json_object_as_hash */
typedef struct JHashState
{
  JsonLexContext *lex;
  const char *function_name;
  HTAB *hash;
  char *saved_scalar;
  const char *save_json_start;
  JsonTokenType saved_token_type;
} JHashState;

/* hashtable element */
typedef struct JsonHashEntry
{
  char fname[NAMEDATALEN]; /* hash key (MUST BE FIRST) */
  char *val;
  JsonTokenType type;
} JsonHashEntry;

/* state for json_strip_nulls */
typedef struct StripnullState
{
  JsonLexContext *lex;
  StringInfo strval;
  bool skip_next_null;
  bool strip_in_arrays;
} StripnullState;

/* structure for generalized json/jsonb value passing */
typedef struct JsValue
{
  bool is_json;    /* json/jsonb */
  union
  {
    struct
    {
      const char *str;    /* json string */
      int len;            /* json string length or -1 if null-terminated */
      JsonTokenType type; /* json type */
    } json;               /* json value */
    JsonbValue *jsonb;    /* jsonb value */
  } val;
} JsValue;

typedef struct JsObject
{
  bool is_json; /* json/jsonb */
  union
  {
    HTAB *json_hash;
    JsonbContainer *jsonb_cont;
  } val;
} JsObject;

/* useful macros for testing JsValue properties */
#define JsValueIsNull(jsv) \
  ((jsv)->is_json ?  \
    (!(jsv)->val.json.str || (jsv)->val.json.type == JSON_TOKEN_NULL) : \
    (!(jsv)->val.jsonb || (jsv)->val.jsonb->type == jbvNull))

#define JsValueIsString(jsv) \
  ((jsv)->is_json ? (jsv)->val.json.type == JSON_TOKEN_STRING \
    : ((jsv)->val.jsonb && (jsv)->val.jsonb->type == jbvString))

#define JsObjectIsEmpty(jso) \
  ((jso)->is_json \
    ? hash_get_num_entries((jso)->val.json_hash) == 0 \
    : ((jso)->val.jsonb_cont == NULL || \
       JsonContainerSize((jso)->val.jsonb_cont) == 0))

#define JsObjectFree(jso) \
  do { \
    if ((jso)->is_json) \
      hash_destroy((jso)->val.json_hash); \
  } while (0)

// static int report_json_context(JsonLexContext *lex);

/* semantic action functions for json_object_keys */
static JsonParseErrorType okeys_object_field_start(void *state, char *fname,
  bool isnull);
static JsonParseErrorType okeys_array_start(void *state);
static JsonParseErrorType okeys_scalar(void *state, char *token,
  JsonTokenType tokentype);

/* semantic action functions for json_get* functions */
static JsonParseErrorType get_object_start(void *state);
static JsonParseErrorType get_object_end(void *state);
static JsonParseErrorType get_object_field_start(void *state, char *fname,
  bool isnull);
static JsonParseErrorType get_object_field_end(void *state, char *fname,
  bool isnull);
static JsonParseErrorType get_array_start(void *state);
static JsonParseErrorType get_array_end(void *state);
static JsonParseErrorType get_array_element_start(void *state, bool isnull);
static JsonParseErrorType get_array_element_end(void *state, bool isnull);
static JsonParseErrorType get_scalar(void *state, char *token,
  JsonTokenType tokentype);

/* common worker function for json getter functions */
static text *get_path_all(text *js, text **path_elems, int path_len,
  bool as_text);
static text *get_worker(text *js, char **tpath, int *ipath, int path_len,
  bool normalize_results);
static text *JsonbValueAsText(JsonbValue *v);

/* semantic action functions for json_array_length */
static JsonParseErrorType alen_object_start(void *state);
static JsonParseErrorType alen_scalar(void *state, char *token,
  JsonTokenType tokentype);
static JsonParseErrorType alen_array_element_start(void *state, bool isnull);

/* common workers for json{b}_each* functions */
static text **each_worker(const text *txt, text **values, int *count,
  bool as_text);
static text **each_worker_jsonb(const Jsonb *jb, void **values, int *count,
  const char *funcname, bool as_text);

/* semantic action functions for json_each */
static JsonParseErrorType each_object_field_start(void *state, char *fname,
  bool isnull);
static JsonParseErrorType each_object_field_end(void *state, char *fname,
  bool isnull);
static JsonParseErrorType each_array_start(void *state);
static JsonParseErrorType each_scalar(void *state, char *token,
  JsonTokenType tokentype);

/* common workers for json{b}_array_elements_* functions */
static text **elements_worker(text *js, int *count, const char *funcname,
  bool as_text);
static void **elements_worker_jsonb(const Jsonb *jb, int *count,
  const char *funcname, bool as_text);

/* semantic action functions for json_array_elements */
static JsonParseErrorType elements_object_start(void *state);
static JsonParseErrorType elements_array_element_start(void *state, bool isnull);
static JsonParseErrorType elements_array_element_end(void *state, bool isnull);
static JsonParseErrorType elements_scalar(void *state, char *token,
  JsonTokenType tokentype);

// /* turn a json object into a hash table */
// static HTAB *get_json_object_as_hash(const char *json, int len,
  // const char *funcname, Node *escontext);

// /* semantic actions for populate_array_json */
// static JsonParseErrorType populate_array_object_start(void *_state);
// static JsonParseErrorType populate_array_array_end(void *_state);
// static JsonParseErrorType populate_array_element_start(void *_state, bool isnull);
// static JsonParseErrorType populate_array_element_end(void *_state, bool isnull);
// static JsonParseErrorType populate_array_scalar(void *_state, char *token,
  // JsonTokenType tokentype);

// /* semantic action functions for get_json_object_as_hash */
// static JsonParseErrorType hash_object_field_start(void *state, char *fname,
  // bool isnull);
// static JsonParseErrorType hash_object_field_end(void *state, char *fname,
  // bool isnull);
// static JsonParseErrorType hash_array_start(void *state);
// static JsonParseErrorType hash_scalar(void *state, char *token,
  // JsonTokenType tokentype);

// /* semantic action functions for populate_recordset */
// static JsonParseErrorType populate_recordset_object_field_start(void *state,
  // char *fname, bool isnull);
// static JsonParseErrorType populate_recordset_object_field_end(void *state,
  // char *fname, bool isnull);
// static JsonParseErrorType populate_recordset_scalar(void *state, char *token,
  // JsonTokenType tokentype);
// static JsonParseErrorType populate_recordset_object_start(void *state);
// static JsonParseErrorType populate_recordset_object_end(void *state);
// static JsonParseErrorType populate_recordset_array_start(void *state);
// static JsonParseErrorType populate_recordset_array_element_start(void *state,
  // bool isnull);

/* semantic action functions for json_strip_nulls */
static JsonParseErrorType sn_object_start(void *state);
static JsonParseErrorType sn_object_end(void *state);
static JsonParseErrorType sn_array_start(void *state);
static JsonParseErrorType sn_array_end(void *state);
static JsonParseErrorType sn_object_field_start(void *state, char *fname,
  bool isnull);
static JsonParseErrorType sn_array_element_start(void *state, bool isnull);
static JsonParseErrorType sn_scalar(void *state, char *token,
  JsonTokenType tokentype);

/* functions supporting jsonb_delete, jsonb_set and jsonb_concat */
static JsonbValue *IteratorConcat(JsonbIterator **it1, JsonbIterator **it2,
  JsonbParseState **state);
static JsonbValue *setPath(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, int op_type);
static void setPathObject(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, uint32 npairs, int op_type);
static void setPathArray(JsonbIterator **it, Datum *path_elems,
  bool *path_nulls, int path_len, JsonbParseState **st, int level,
  JsonbValue *newval, uint32 nelems, int op_type);

/* function supporting iterate_json_values */
static JsonParseErrorType iterate_values_scalar(void *state, char *token, JsonTokenType tokentype);
static JsonParseErrorType iterate_values_object_field_start(void *state, char *fname, bool isnull);

/* functions supporting transform_json_string_values */
static JsonParseErrorType transform_string_values_object_start(void *state);
static JsonParseErrorType transform_string_values_object_end(void *state);
static JsonParseErrorType transform_string_values_array_start(void *state);
static JsonParseErrorType transform_string_values_array_end(void *state);
static JsonParseErrorType transform_string_values_object_field_start(void *state, char *fname, bool isnull);
static JsonParseErrorType transform_string_values_array_element_start(void *state, bool isnull);
static JsonParseErrorType transform_string_values_scalar(void *state, char *token, JsonTokenType tokentype);

/*****************************************************************************/

/**
 * @ingroup meos_json_base_constructor
 * @brief Return a copy of a JSONB value
 * @param[in] jb JSONB value
 * @note MEOS function
 */
#if MEOS
Jsonb *
jsonb_copy(const Jsonb *jb)
{
  return pg_jsonb_copy(jb);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_copy(const Jsonb *jb)
{
  Jsonb *result = palloc(VARSIZE(jb));
  memcpy(result, jb, VARSIZE(jb));
  return result;
}

/*
 * pg_parse_json_or_errsave
 *
 * This function is like pg_parse_json, except that it does not return a
 * JsonParseErrorType. Instead, in case of any failure, this function will
 * save error data into *escontext if that's an ErrorSaveContext, otherwise
 * ereport(ERROR).
 *
 * Returns a boolean indicating success or failure (failure will only be
 * returned when escontext is an ErrorSaveContext).
 */
bool
pg_parse_json_or_errsave(JsonLexContext *lex, const JsonSemAction *sem,
  Node *escontext)
{
  JsonParseErrorType result = pg_parse_json(lex, sem);
  if (result != JSON_SUCCESS)
  {
    json_errsave_error(result, lex, escontext);
    return false;
  }
  return true;
}

/**
 * This is like makeJsonLexContextCstringLen, but it accepts a text value
 * directly.
 */
JsonLexContext *
makeJsonLexContext(JsonLexContext *lex, text *js, bool need_escapes)
{
  return makeJsonLexContextCstringLen(lex, VARDATA_ANY(js),
    VARSIZE_ANY_EXHDR(js), GetDatabaseEncoding(), need_escapes);
}

/*
 * Report a JSON error.
 */
void
json_errsave_error(JsonParseErrorType error, JsonLexContext *lex UNUSED,
  Node *escontext UNUSED)
{
  if (error == JSON_UNICODE_HIGH_ESCAPE ||
      error == JSON_UNICODE_UNTRANSLATABLE ||
      error == JSON_UNICODE_CODE_POINT_ZERO)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unsupported Unicode escape sequence");
  else if (error == JSON_SEM_ACTION_FAILED)
    /* semantic action function had better have reported something */
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "JSON semantic action function did not provide error information");
  else
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "invalid input syntax for type json");
  return;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the set of keys of a JSON value
 * @note Derived from PostgreSQL function @p json_object_keys()
 */
#if MEOS
text **
json_object_keys(const text *js, int *count)
{
  return pg_json_object_keys(js, count);
}
#endif /* MEOS */
text **
pg_json_object_keys(const text *js, int *count)
{
  OkeysState *state = palloc(sizeof(OkeysState));
  JsonLexContext lex;
  state->lex = makeJsonLexContext(&lex, (text *) js, true);
  state->result_size = 256;
  state->result_count = 0;
  state->result = palloc(256 * sizeof(char *));

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->array_start = okeys_array_start;
  sem->scalar = okeys_scalar;
  sem->object_field_start = okeys_object_field_start;
  /* remainder are all NULL, courtesy of palloc0 above */

  pg_parse_json_or_ereport(&lex, sem);
  /* keys are now in state->result */

  freeJsonLexContext(&lex);
  pfree(sem);

  /* Construct the result */
  text **result = (text **) palloc(sizeof(text *) * state->result_count);
  for (int i = 0; i < state->result_count; i++)
    result[i] = pg_cstring_to_text(state->result[i]);
  *count = state->result_count;

  /* Clean up and return */
  pfree(state->result);
  pfree(state);
  return result;
}

static JsonParseErrorType
okeys_object_field_start(void *state, char *fname, bool isnull UNUSED)
{
  OkeysState *_state = (OkeysState *) state;
  /* only collecting keys for the top level object */
  if (_state->lex->lex_level != 1)
    return JSON_SUCCESS;
  /* enlarge result array if necessary */
  if (_state->result_count >= _state->result_size)
  {
    _state->result_size *= 2;
    _state->result = (char **)
      repalloc(_state->result, sizeof(char *) * _state->result_size);
  }
  /* save a copy of the field name */
  char *fname1 = pstrdup(fname);
  /* Add fname1 to the values that need to be freed */
  json_add_tofree((void *) fname1);
  return JSON_SUCCESS;
}

static JsonParseErrorType
okeys_array_start(void *state)
{
  OkeysState *_state = (OkeysState *) state;

  /* top level must be a json object */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot call %s on an array", "json_object_keys");
    return JSON_INVALID_TOKEN;
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
okeys_scalar(void *state, char *token UNUSED, JsonTokenType tokentype UNUSED)
{
  OkeysState *_state = (OkeysState *) state;

  /* top level must be a json object */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot call %s on a scalar", "json_object_keys");
    return JSON_INVALID_TOKEN;
  }
  return JSON_SUCCESS;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the set of keys of a JSONB object
 * @details The keys are stashed in an array, whose size is expanded as
 * necessary. This is probably safe enough for a list of keys of a single
 * object, since they are limited in size to NAMEDATALEN and the number of
 * keys is unlikely to be so huge that it has major memory implications.
 * @note Derived from PostgreSQL function @p jsonb_object_keys()
 */
#if MEOS
text **
jsonb_object_keys(const Jsonb *jb, int *count)
{
  return pg_jsonb_object_keys(jb, count);
}
#endif /* MEOS */
text **
pg_jsonb_object_keys(const Jsonb *jb, int *count)
{
  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
     "cannot call %s on a scalar", "jsonb_object_keys");
    return NULL;
  }
  else if (JB_ROOT_IS_ARRAY(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
     "cannot call %s on an array", "jsonb_object_keys");
    return NULL;
  }

  bool skipNested = false;
  JsonbValue v;
  JsonbIteratorToken r;
  int result_count = 0;
  text **result = (text **) palloc(sizeof(text *) * JB_ROOT_COUNT(jb));
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  while ((r = JsonbIteratorNext(&it, &v, skipNested)) != WJB_DONE)
  {
    skipNested = true;
    if (r == WJB_KEY)
    {
      char *cstr = palloc(v.val.string.len + 1 * sizeof(char));
      memcpy(cstr, v.val.string.val, v.val.string.len);
      cstr[v.val.string.len] = '\0';
      result[result_count++] = pg_cstring_to_text(cstr);
      pfree(cstr); // MEOS
    }
  }
  *count = result_count;
  return result;
}

/*****************************************************************************/

/*
 * json and jsonb getter functions
 * these implement the -> ->> #> and #>> operators
 * and the json{b?}_extract_path*(json, text, ...) functions
 */

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON object field given by a key
 * @note Derived from PostgreSQL function @p json_object_field(text, text)
 */
#if MEOS
text *
json_object_field(const text *js, const text *key)
{
  return pg_json_object_field(js, key);
}
#endif /* MEOS */
text *
pg_json_object_field(const text *js, const text *key)
{
  char *keystr = pg_text_to_cstring(key);
  text *result = get_worker((text *) js, &keystr, NULL, 1, false);
  pfree(keystr);
  return result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB object field given by a key
 * @note Derived from PostgreSQL function @p jsonb_object_field(jsonb, text)
 */
#if MEOS
Jsonb *
jsonb_object_field(const Jsonb *jb, const text *key)
{
  return pg_jsonb_object_field(jb, key);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_object_field(const Jsonb *jb, const text *key)
{
  if (!JB_ROOT_IS_OBJECT(jb))
    return NULL;

  JsonbValue vbuf;
  JsonbValue *v = getKeyJsonValueFromContainer(&((Jsonb *)jb)->root,
    VARDATA_ANY(key), VARSIZE_ANY_EXHDR(key), &vbuf);
  return (! v) ?  NULL : JsonbValueToJsonb(v);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON object field given by a key as text
 * @note Derived from PostgreSQL function @p json_object_field_text(text, text)
 */
#if MEOS
text *
json_object_field_text(const text *js, const text *key)
{
  return pg_json_object_field_text(js, key);
}
#endif /* MEOS */
text *
pg_json_object_field_text(const text *js, const text *key)
{
  char *keystr = pg_text_to_cstring(key);
  text *result = get_worker((text *) js, &keystr, NULL, 1, true);
  pfree(keystr);
  return (! result) ? NULL : result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB object field given by a key as text
 * @note Derived from PostgreSQL function @p json_object_field_text(jsonb, text)
 */
#if MEOS
text *
jsonb_object_field_text(const Jsonb *jb, const text *key)
{
  return pg_jsonb_object_field_text(jb, key);
}
#endif /* MEOS */
text *
pg_jsonb_object_field_text(const Jsonb *jb, const text *key)
{
  assert(jb); assert(key);
  if (! JB_ROOT_IS_OBJECT(jb))
    return NULL;

  JsonbValue vbuf;
  JsonbValue *v = getKeyJsonValueFromContainer(&((Jsonb *) jb)->root,
    VARDATA_ANY(key), VARSIZE_ANY_EXHDR(key), &vbuf);
  if (v != NULL && v->type != jbvNull)
    return JsonbValueAsText(v);
  return NULL;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON array element given by an index
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p json_array_element(text, int)
 */
#if MEOS
text *
json_array_element(const text *js, int idx)
{
  return pg_json_array_element(js, idx);
}
#endif /* MEOS */
text *
pg_json_array_element(const text *js, int idx)
{
  return get_worker((text *) js, NULL, &idx, 1, false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB array element given by an index
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p jsonb_array_element(jsonb, int)
 */
#if MEOS
Jsonb *
jsonb_array_element(const Jsonb *jb, int idx)
{
  return pg_jsonb_array_element(jb, idx);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_array_element(const Jsonb *jb, int idx)
{
  if (!JB_ROOT_IS_ARRAY(jb))
    return NULL;

  /* Handle negative subscript */
  if (idx < 0)
  {
    uint32 nelements = JB_ROOT_COUNT(jb);
    if (pg_abs_s32(idx) > nelements)
      return NULL;
    else
      idx += nelements;
  }
  JsonbValue *v = getIthJsonbValueFromContainer(&((Jsonb *) jb)->root, idx);
  if (! v)
    return NULL;
  Jsonb *result = JsonbValueToJsonb(v);
  pfree(v);
  return result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON array element given by an index as text
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p json_array_element_text(text, int)
 */
#if MEOS
text *
json_array_element_text(const text *js, int idx)
{
  return pg_json_array_element_text(js, idx);
}
#endif /* MEOS */
text *
pg_json_array_element_text(const text *js, int idx)
{
  text *result = get_worker((text *) js, NULL, &idx, 1, true);
  if (result != NULL)
    return result;
  else
    return NULL;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB array element given by an index as text
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p jsonb_array_element_text(jsonb, int)
 */
#if MEOS
text *
jsonb_array_element_text(const Jsonb *jb, int idx)
{
  return pg_jsonb_array_element_text(jb, idx);
}
#endif /* MEOS */
text *
pg_jsonb_array_element_text(const Jsonb *jb, int idx)
{
  if (!JB_ROOT_IS_ARRAY(jb))
    return NULL;

  /* Handle negative subscript */
  if (idx < 0)
  {
    uint32 nelements = JB_ROOT_COUNT(jb);
    if (pg_abs_s32(idx) > nelements)
      return NULL;
    else
      idx += nelements;
  }

  JsonbValue *v = getIthJsonbValueFromContainer(&((Jsonb *) jb)->root, idx);
  if (v != NULL && v->type != jbvNull)
    return JsonbValueAsText(v);

  return NULL;
}

/*****************************************************************************/

/*
 * common routine for extract_path functions
 */
static text *
get_path_all(text *js, text **path_elems, int path_len, bool as_text)
{
  char **tpath = palloc(path_len * sizeof(char *));
  int *ipath = palloc(path_len * sizeof(int));
  for (int i = 0; i < path_len; i++)
  {
    Assert(! path_elems[i]);
    tpath[i] = pg_text_to_cstring(path_elems[i]);

    /*
     * we have no idea at this stage what structure the document is so
     * just convert anything in the path that we can to an integer and set
     * all the other integers to INT_MIN which will never match.
     */
    if (*tpath[i] != '\0')
    {
      errno = 0;
      char *endptr;
      int ind = strtoint(tpath[i], &endptr, 10);
      if (endptr == tpath[i] || *endptr != '\0' || errno != 0)
        ipath[i] = INT_MIN;
      else
        ipath[i] = ind;
    }
    else
      ipath[i] = INT_MIN;
  }

  return get_worker(js, tpath, ipath, path_len, as_text);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON object given by a path
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p json_extract_path(text, text **, int)
 */
#if MEOS
text *
json_extract_path(const text *js, text **path_elems, int path_len)
{
  return pg_json_extract_path(js, path_elems, path_len);
}
#endif /* MEOS */
text *
pg_json_extract_path(const text *js, text **path_elems, int path_len)
{
  return get_path_all((text *) js, path_elems, path_len, false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSON object given by a path as text
 * @details 0-based, negative integers count from the end
 * @note Derived from PostgreSQL function @p json_extract_path_text(text, text **, int)
 */
#if MEOS
text *
json_extract_path_text(const text *js, text **path_elems, int path_len)
{
  return pg_json_extract_path_text(js, path_elems, path_len);
}
#endif /* MEOS */
text *
pg_json_extract_path_text(const text *js, text **path_elems, int path_len)
{
  return get_path_all((text *) js, path_elems, path_len, true);
}

/*
 * get_worker
 *
 * common worker for all the json getter functions
 *
 * json: JSON object (in text form)
 * tpath[]: field name(s) to extract
 * ipath[]: array index(es) (zero-based) to extract, accepts negatives
 * path_len: length of tpath[] and/or ipath[]
 * normalize_results: true to de-escape string and null scalars
 *
 * tpath can be NULL, or any one tpath[] entry can be NULL, if an object
 * field is not to be matched at that nesting level.  Similarly, ipath can
 * be NULL, or any one ipath[] entry can be INT_MIN if an array element is
 * not to be matched at that nesting level (a json datum should never be
 * large enough to have -INT_MIN elements due to MaxAllocSize restriction).
 */
static text *
get_worker(text *js, char **tpath, int *ipath, int path_len,
  bool normalize_results)
{
  Assert(path_len >= 0);

  GetState *state = palloc0(sizeof(GetState));
  state->lex = makeJsonLexContext(NULL, js, true);

  /* is it "_as_text" variant? */
  state->normalize_results = normalize_results;
  state->path_len = path_len;
  state->path_names = tpath;
  state->path_indexes = ipath;
  state->pathok = palloc0(sizeof(bool) * path_len);
  state->array_cur_index = palloc(sizeof(int) * path_len);

  if (path_len > 0)
    state->pathok[0] = true;

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;

  /*
   * Not all variants need all the semantic routines. Only set the ones that
   * are actually needed for maximum efficiency.
   */
  sem->scalar = get_scalar;
  if (path_len == 0)
  {
    sem->object_start = get_object_start;
    sem->object_end = get_object_end;
    sem->array_start = get_array_start;
    sem->array_end = get_array_end;
  }
  if (tpath != NULL)
  {
    sem->object_field_start = get_object_field_start;
    sem->object_field_end = get_object_field_end;
  }
  if (ipath != NULL)
  {
    sem->array_start = get_array_start;
    sem->array_element_start = get_array_element_start;
    sem->array_element_end = get_array_element_end;
  }

  pg_parse_json_or_ereport(state->lex, sem);
  freeJsonLexContext(state->lex);
  pfree(sem);
  return state->tresult;
}

static JsonParseErrorType
get_object_start(void *state)
{
  GetState *_state = (GetState *) state;
  int lex_level = _state->lex->lex_level;

  if (lex_level == 0 && _state->path_len == 0)
  {
    /*
     * Special case: we should match the entire object.  We only need this
     * at outermost level because at nested levels the match will have
     * been started by the outer field or array element callback.
     */
    _state->result_start = _state->lex->token_start;
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_object_end(void *state)
{
  GetState *_state = (GetState *) state;
  int lex_level = _state->lex->lex_level;
  if (lex_level == 0 && _state->path_len == 0)
  {
    /* Special case: return the entire object */
    const char *start = _state->result_start;
    int len = _state->lex->prev_token_terminator - start;
    _state->tresult = pg_cstring_to_text_with_len(start, len);
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_object_field_start(void *state, char *fname, bool isnull UNUSED)
{
  GetState *_state = (GetState *) state;
  bool get_next = false;
  int lex_level = _state->lex->lex_level;

  if (lex_level <= _state->path_len &&
    _state->pathok[lex_level - 1] &&
    _state->path_names != NULL &&
    _state->path_names[lex_level - 1] != NULL &&
    strcmp(fname, _state->path_names[lex_level - 1]) == 0)
  {
    if (lex_level < _state->path_len)
    {
      /* if not at end of path just mark path ok */
      _state->pathok[lex_level] = true;
    }
    else
    {
      /* end of path, so we want this value */
      get_next = true;
    }
  }

  if (get_next)
  {
    /* this object overrides any previous matching object */
    _state->tresult = NULL;
    _state->result_start = NULL;

    if (_state->normalize_results &&
      _state->lex->token_type == JSON_TOKEN_STRING)
    {
      /* for as_text variants, tell get_scalar to set it for us */
      _state->next_scalar = true;
    }
    else
    {
      /* for non-as_text variants, just note the json starting point */
      _state->result_start = _state->lex->token_start;
    }
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_object_field_end(void *state, char *fname, bool isnull)
{
  GetState *_state = (GetState *) state;
  bool get_last = false;
  int lex_level = _state->lex->lex_level;

  /* same tests as in get_object_field_start */
  if (lex_level <= _state->path_len &&
    _state->pathok[lex_level - 1] &&
    _state->path_names != NULL &&
    _state->path_names[lex_level - 1] != NULL &&
    strcmp(fname, _state->path_names[lex_level - 1]) == 0)
  {
    if (lex_level < _state->path_len)
    {
      /* done with this field so reset pathok */
      _state->pathok[lex_level] = false;
    }
    else
    {
      /* end of path, so we want this value */
      get_last = true;
    }
  }

  /* for as_text scalar case, our work is already done */
  if (get_last && _state->result_start != NULL)
  {
    /*
     * make a text object from the string from the previously noted json
     * start up to the end of the previous token (the lexer is by now
     * ahead of us on whatever came after what we're interested in).
     */
    if (isnull && _state->normalize_results)
      _state->tresult = (text *) NULL;
    else
    {
      const char *start = _state->result_start;
      int len = _state->lex->prev_token_terminator - start;
      _state->tresult = pg_cstring_to_text_with_len(start, len);
    }
    /* this should be unnecessary but let's do it for cleanliness: */
    _state->result_start = NULL;
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
get_array_start(void *state)
{
  GetState *_state = (GetState *) state;
  int lex_level = _state->lex->lex_level;

  if (lex_level < _state->path_len)
  {
    /* Initialize counting of elements in this array */
    _state->array_cur_index[lex_level] = -1;

    /* INT_MIN value is reserved to represent invalid subscript */
    if (_state->path_indexes[lex_level] < 0 &&
      _state->path_indexes[lex_level] != INT_MIN)
    {
      /* Negative subscript -- convert to positive-wise subscript */
      JsonParseErrorType error;
      int nelements;

      error = json_count_array_elements(_state->lex, &nelements);
      if (error != JSON_SUCCESS)
        json_errsave_error(error, _state->lex, NULL);

      if (-_state->path_indexes[lex_level] <= nelements)
        _state->path_indexes[lex_level] += nelements;
    }
  }
  else if (lex_level == 0 && _state->path_len == 0)
  {
    /*
     * Special case: we should match the entire array.  We only need this
     * at the outermost level because at nested levels the match will have
     * been started by the outer field or array element callback.
     */
    _state->result_start = _state->lex->token_start;
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_array_end(void *state)
{
  GetState *_state = (GetState *) state;
  int lex_level = _state->lex->lex_level;
  if (lex_level == 0 && _state->path_len == 0)
  {
    /* Special case: return the entire array */
    const char *start = _state->result_start;
    int len = _state->lex->prev_token_terminator - start;
    _state->tresult = pg_cstring_to_text_with_len(start, len);
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
get_array_element_start(void *state, bool isnull UNUSED)
{
  GetState *_state = (GetState *) state;
  bool get_next = false;
  int lex_level = _state->lex->lex_level;

  /* Update array element counter */
  if (lex_level <= _state->path_len)
    _state->array_cur_index[lex_level - 1]++;

  if (lex_level <= _state->path_len &&
    _state->pathok[lex_level - 1] &&
    _state->path_indexes != NULL &&
    _state->array_cur_index[lex_level - 1] == _state->path_indexes[lex_level - 1])
  {
    if (lex_level < _state->path_len)
    {
      /* if not at end of path just mark path ok */
      _state->pathok[lex_level] = true;
    }
    else
    {
      /* end of path, so we want this value */
      get_next = true;
    }
  }

  /* same logic as for objects */
  if (get_next)
  {
    _state->tresult = NULL;
    _state->result_start = NULL;
    if (_state->normalize_results &&
      _state->lex->token_type == JSON_TOKEN_STRING)
    {
      _state->next_scalar = true;
    }
    else
    {
      _state->result_start = _state->lex->token_start;
    }
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_array_element_end(void *state, bool isnull)
{
  GetState *_state = (GetState *) state;
  bool get_last = false;
  int lex_level = _state->lex->lex_level;

  /* same tests as in get_array_element_start */
  if (lex_level <= _state->path_len &&
    _state->pathok[lex_level - 1] &&
    _state->path_indexes != NULL &&
    _state->array_cur_index[lex_level - 1] == _state->path_indexes[lex_level - 1])
  {
    if (lex_level < _state->path_len)
    {
      /* done with this element so reset pathok */
      _state->pathok[lex_level] = false;
    }
    else
    {
      /* end of path, so we want this value */
      get_last = true;
    }
  }

  /* same logic as for objects */
  if (get_last && _state->result_start != NULL)
  {
    if (isnull && _state->normalize_results)
      _state->tresult = (text *) NULL;
    else
    {
      const char *start = _state->result_start;
      int len = _state->lex->prev_token_terminator - start;
      _state->tresult = pg_cstring_to_text_with_len(start, len);
    }

    _state->result_start = NULL;
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
get_scalar(void *state, char *token, JsonTokenType tokentype)
{
  GetState *_state = (GetState *) state;
  int lex_level = _state->lex->lex_level;

  /* Check for whole-object match */
  if (lex_level == 0 && _state->path_len == 0)
  {
    if (_state->normalize_results && tokentype == JSON_TOKEN_STRING)
    {
      /* we want the de-escaped string */
      _state->next_scalar = true;
    }
    else if (_state->normalize_results && tokentype == JSON_TOKEN_NULL)
    {
      _state->tresult = (text *) NULL;
    }
    else
    {
      /*
       * This is a bit hokey: we will suppress whitespace after the
       * scalar token, but not whitespace before it.  Probably not worth
       * doing our own space-skipping to avoid that.
       */
      const char *start = _state->lex->input;
      int len = _state->lex->prev_token_terminator - start;
      _state->tresult = pg_cstring_to_text_with_len(start, len);
    }
  }

  if (_state->next_scalar)
  {
    /* a de-escaped text value is wanted, so supply it */
    _state->tresult = pg_cstring_to_text(token);
    /* make sure the next call to get_scalar doesn't overwrite it */
    _state->next_scalar = false;
  }

  return JSON_SUCCESS;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB object field given by a path
 * @note Derived from PostgreSQL function @p jsonb_extract_path()
 */
#if MEOS
Jsonb *
jsonb_extract_path(const Jsonb *jb, text **path_elems, int path_len)
{
  return pg_jsonb_extract_path(jb, path_elems, path_len);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_extract_path(const Jsonb *jb, text **path_elems, int path_len)
{
  return (Jsonb *) pg_jsonb_get_element((Jsonb *) jb, path_elems, path_len,
    false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return a JSONB object field given by a path as text
 * @note Derived from PostgreSQL function @p jsonb_extract_path_text(text, text **, int)
 */
#if MEOS
text *
jsonb_extract_path_text(const Jsonb *jb, text **path_elems, int path_len)
{
  return (text *) pg_jsonb_extract_path_text((Jsonb *) jb, path_elems,
    path_len);
}
#endif /* MEOS */
text *
pg_jsonb_extract_path_text(const Jsonb *jb, text **path_elems, int path_len)
{
  return (text *) pg_jsonb_get_element((Jsonb *) jb, path_elems, path_len,
    true);
}

/**
 * @brief Return a JSONB object field given by a path as text
 */
void *
pg_jsonb_get_element(Jsonb *jb, text **path_elems, int path_len, bool as_text)
{
  JsonbContainer *container = &jb->root;
  JsonbValue *v = NULL;
  int i;
  bool have_object = false, have_array = false;

  /* Identify whether we have object, array, or scalar at top-level */
  if (JB_ROOT_IS_OBJECT(jb))
    have_object = true;
  else if (JB_ROOT_IS_ARRAY(jb) && !JB_ROOT_IS_SCALAR(jb))
    have_array = true;
  else
  {
    Assert(JB_ROOT_IS_ARRAY(jb) && JB_ROOT_IS_SCALAR(jb));
    /* Extract the scalar value, if it is what we'll return */
    if (path_len <= 0)
      v = getIthJsonbValueFromContainer(container, 0);
  }

  /*
   * If the array is empty, return the entire LHS object, on the grounds
   * that we should do zero field or element extractions.  For the
   * non-scalar case we can just hand back the object without much work. For
   * the scalar case, fall through and deal with the value below the loop.
   * (This inconsistency arises because there's no easy way to generate a
   * JsonbValue directly for root-level containers.)
   */
  if (path_len <= 0 && ! v)
  {
    if (as_text)
    {
      StringInfo out = makeStringInfo(); // MEOS
      char *str = JsonbToCString(out, container, VARSIZE(jb));
      text *result = pg_cstring_to_text_with_len(str, out->len);
      destroyStringInfo(out); pfree(str);
      return (void *) result;
    }
    else
      /* not text mode - just hand back the jsonb */
      return (void *) pg_jsonb_copy(jb); // MEOS
  }

  for (i = 0; i < path_len; i++)
  {
    if (v) // MEOS
      pfree(v);
    if (have_object)
    {
      text *subscr = path_elems[i];
      v = getKeyJsonValueFromContainer(container, VARDATA_ANY(subscr),
        VARSIZE_ANY_EXHDR(subscr), NULL);
    }
    else if (have_array)
    {
      int lindex;
      uint32 index;
      char *indextext = pg_text_to_cstring(path_elems[i]);
      char *endptr;

      errno = 0;
      lindex = strtoint(indextext, &endptr, 10);
      if (endptr == indextext || *endptr != '\0' || errno != 0)
        return NULL;
      pfree(indextext);

      if (lindex >= 0)
        index = (uint32) lindex;
      else
      {
        /* Handle negative subscript */

        /* Container must be array, but make sure */
        if (!JsonContainerIsArray(container))
        {
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "not a jsonb array");
          return NULL;
        }

        uint32 nelements = JsonContainerSize(container);
        if (lindex == INT_MIN || -lindex > (int) nelements)
          return NULL;
        else
          index = nelements + lindex;
      }
      v = getIthJsonbValueFromContainer(container, index);
    }
    else
      /* scalar, extraction yields a null */
      return NULL;

    if (! v)
      return NULL;
    else if (i == path_len - 1)
      break;

    if (v->type == jbvBinary)
    {
      container = v->val.binary.data;
      have_object = JsonContainerIsObject(container);
      have_array = JsonContainerIsArray(container);
      Assert(!JsonContainerIsScalar(container));
    }
    else
    {
      Assert(IsAJsonbScalar(v));
      have_object = false;
      have_array = false;
    }
  }

  void *result;
  if (as_text)
  {
    if (v->type == jbvNull)
    {
      pfree(v);
      return NULL;
    }
    result = (void *) JsonbValueAsText(v);
  }
  else
  {
    /* not text mode - just hand back the jsonb */
    result = (void *) JsonbValueToJsonb(v);
  }
  pfree(v);
  return result;
}

Datum
jsonb_set_element(Jsonb *jb, Datum *path, int path_len, JsonbValue *newval)
{
  if (newval->type == jbvArray && newval->val.array.rawScalar)
    *newval = newval->val.array.elems[0];

  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  JsonbParseState *state = NULL;
  bool *path_nulls = palloc0(path_len * sizeof(bool));
  JsonbValue *res = setPath(&it, path, path_nulls, path_len, &state, 0, newval,
    JB_PATH_CREATE | JB_PATH_FILL_GAPS | JB_PATH_CONSISTENT_POSITION);
  Jsonb *result = JsonbValueToJsonb(res);
  pfree(res); pfree(path_nulls);
  return PointerGetDatum(result);
}

static void
push_null_elements(JsonbParseState **ps, int num)
{
  JsonbValue null;
  null.type = jbvNull;
  while (num-- > 0)
    pushJsonbValue(ps, WJB_ELEM, &null);
}

/*
 * Prepare a new structure containing nested empty objects and arrays
 * corresponding to the specified path, and assign a new value at the end of
 * this path. E.g. the path [a][0][b] with the new value 1 will produce the
 * structure {a: [{b: 1}]}.
 *
 * Caller is responsible to make sure such path does not exist yet.
 */
static void
push_path(JsonbParseState **st, int level, Datum *path_elems, bool *path_nulls,
  int path_len, JsonbValue *newval)
{
  int i;
  /*
   * tpath contains expected type of an empty jsonb created at each level
   * higher or equal to the current one, either jbvObject or jbvArray. Since
   * it contains only information about path slice from level to the end,
   * the access index must be normalized by level.
   */
  enum jbvType *tpath = palloc0((path_len - level) * sizeof(enum jbvType));
  JsonbValue newkey;

  /*
   * Create first part of the chain with beginning tokens. For the current
   * level WJB_BEGIN_OBJECT/WJB_BEGIN_ARRAY was already created, so start
   * with the next one.
   */
  for (i = level + 1; i < path_len; i++)
  {
    int lindex;
    if (path_nulls[i])
      break;

    /*
     * Try to convert to an integer to find out the expected type, object
     * or array.
     */
    char *c = pg_text_to_cstring(DatumGetTextP(path_elems[i]));
    errno = 0;
    char *badp;
    lindex = strtoint(c, &badp, 10);
    if (badp == c || *badp != '\0' || errno != 0)
    {
      /* text, an object is expected */
      newkey.type = jbvString;
      newkey.val.string.val = c;
      newkey.val.string.len = strlen(c);
      (void) pushJsonbValue(st, WJB_BEGIN_OBJECT, NULL);
      (void) pushJsonbValue(st, WJB_KEY, &newkey);
      tpath[i - level] = jbvObject;
    }
    else
    {
      /* integer, an array is expected */
      (void) pushJsonbValue(st, WJB_BEGIN_ARRAY, NULL);
      push_null_elements(st, lindex);
      tpath[i - level] = jbvArray;
    }
  }

  /* Insert an actual value for either an object or array */
  if (tpath[(path_len - level) - 1] == jbvArray)
  {
    (void) pushJsonbValue(st, WJB_ELEM, newval);
  }
  else
    (void) pushJsonbValue(st, WJB_VALUE, newval);

  /*
   * Close everything up to the last but one level. The last one will be
   * closed outside of this function.
   */
  for (i = path_len - 1; i > level; i--)
  {
    if (path_nulls[i])
      break;

    if (tpath[i - level] == jbvObject)
      (void) pushJsonbValue(st, WJB_END_OBJECT, NULL);
    else
      (void) pushJsonbValue(st, WJB_END_ARRAY, NULL);
  }
  return;
}

/*
 * Return the text representation of the given JsonbValue.
 */
static text *
JsonbValueAsText(JsonbValue *v)
{
  switch (v->type)
  {
    case jbvNull:
      return NULL;

    case jbvBool:
      return v->val.boolean ?
        pg_cstring_to_text_with_len("true", 4) :
        pg_cstring_to_text_with_len("false", 5);

    case jbvString:
    {
      text *result = pg_cstring_to_text_with_len(v->val.string.val,
        v->val.string.len);
      return result;
    }

    case jbvNumeric:
      {
        char *cstr = pg_numeric_out(v->val.numeric);
        text *result = pg_cstring_to_text(cstr); // MEOS
        pfree(cstr);
        return result;
      }

    case jbvBinary:
      {
        StringInfoData jtext;
        initStringInfo(&jtext);
        (void) JsonbToCString(&jtext, v->val.binary.data, v->val.binary.len);
        text *result = pg_cstring_to_text_with_len(jtext.data, jtext.len); // MEOS
        pfree(jtext.data);
        return result;
      }

    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "unrecognized jsonb type: %d", (int) v->type);
      return NULL;
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Returns the length of a JSON array
 * @param[in] js JSON value
 * @note Derived from PostgreSQL function @p json_array_length()
 */
#if MEOS
int
json_array_length(const text *js)
{
  return pg_json_array_length(js);
}
#endif /* MEOS */
int
pg_json_array_length(const text *js)
{
  AlenState *state = palloc0(sizeof(AlenState));
  JsonLexContext lex;
  state->lex = makeJsonLexContext(&lex, (text *) js, false);
  /* palloc0 initizalizes state->count = 0 */

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->object_start = alen_object_start;
  sem->scalar = alen_scalar;
  sem->array_element_start = alen_array_element_start;

  pg_parse_json_or_ereport(state->lex, sem);
  freeJsonLexContext(&lex);
  pfree(sem);
  int result = state->count;
  pfree(state); // MEOS
  return result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Returns the length of a JSONB array
 * @param[in] jb JSONB value
 * @note Derived from PostgreSQL function @p jsonb_array_length()
 */
#if MEOS
int
jsonb_array_length(const Jsonb *jb)
{
  return pg_jsonb_array_length(jb);
}
#endif /* MEOS */
int
pg_jsonb_array_length(const Jsonb *jb)
{
  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot get array length of a scalar");
    return -1;
  }
  else if (!JB_ROOT_IS_ARRAY(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot get array length of a non-array");
    return -1;
  }
  return JB_ROOT_COUNT(jb);
}

/*
 * These next two checks ensure that the json is an array (since it can't be
 * a scalar or an object).
 */

static JsonParseErrorType
alen_object_start(void *state)
{
  AlenState  *_state = (AlenState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot get array length of a non-array");
    return JSON_INVALID_TOKEN;
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
alen_scalar(void *state, char *token UNUSED, JsonTokenType tokentype UNUSED)
{
  AlenState  *_state = (AlenState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot get array length of a scalar");
    return JSON_INVALID_TOKEN;
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
alen_array_element_start(void *state, bool isnull UNUSED)
{
  AlenState  *_state = (AlenState *) state;
  /* just count up all the level 1 elements */
  if (_state->lex->lex_level == 1)
    _state->count++;

  return JSON_SUCCESS;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return two arrays of keys and values of a JSON value
 * @note Derived from PostgreSQL function: @p json_each()
 */
#if MEOS
text **
json_each(const text *js, text **values, int *count)
{
  return each_worker(js, values, count, true);
}
#endif /* MEOS */
text **
pg_json_each(const text *js, text **values, int *count)
{
  return each_worker(js, values, count, false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return two arrays of keys and values of a JSON value as text
 * @note Derived from PostgreSQL function: @p json_each_text()
 */
#if MEOS
text **
json_each_text(const text *js, text **values, int *count)
{
  return each_worker(js, values, count, true);
}
#endif /* MEOS */
text **
pg_json_each_text(const text *js, text **values, int *count)
{
  return each_worker(js, values, count, true);
}

static text **
each_worker(const text *js, text **values, int *count, bool as_text)
{
  JsonLexContext lex;
  EachState *state = palloc0(sizeof(EachState));
  state->result_size = 256;
  state->result_count = 0;
  state->keys = (text **) palloc(256 * sizeof(text *));
  state->values = (text **) palloc(256 * sizeof(text *));
  state->normalize_results = as_text;
  state->next_scalar = false;
  state->lex = makeJsonLexContext(&lex, (text *) js, true);

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->array_start = each_array_start;
  sem->scalar = each_scalar;
  sem->object_field_start = each_object_field_start;
  sem->object_field_end = each_object_field_end;

  pg_parse_json_or_ereport(&lex, sem);
  freeJsonLexContext(&lex);
  pfree(sem);

  /* Construct the result */
  text **result = (text **) palloc(sizeof(text *) * state->result_count);
  memcpy(result, state->keys, sizeof(text *) * state->result_count);
  // memcpy((char *) *values, (char *) state->values,
    // sizeof(text *) * state->result_count);
  for (int i = 0; i < state->result_count; i++)
    values[i] = state->values[i];

  /* Clean up and return */
  *count = state->result_count;
  pfree(state->keys);
  pfree(state->values);
  pfree(state);
  return result;
}

static JsonParseErrorType
each_object_field_start(void *state, char *fname UNUSED, bool isnull UNUSED)
{
  EachState *_state = (EachState *) state;
  /* save a pointer to where the value starts */
  if (_state->lex->lex_level == 1)
  {
    /*
     * next_scalar will be reset in the object_field_end handler, and
     * since we know the value is a scalar there is no danger of it being
     * on while recursing down the tree.
     */
    if (_state->normalize_results && _state->lex->token_type == JSON_TOKEN_STRING)
      _state->next_scalar = true;
    else
      _state->result_start = _state->lex->token_start;
  }
  return JSON_SUCCESS;
}

/*****************************************************************************/

static JsonParseErrorType
each_object_field_end(void *state, char *fname, bool isnull)
{
  EachState *_state = (EachState *) state;
  /* skip over nested objects */
  if (_state->lex->lex_level != 1)
    return JSON_SUCCESS;

  text *key = pg_cstring_to_text(fname);
  text *val;
  if (isnull && _state->normalize_results)
  {
    val = NULL;
  }
  else if (_state->next_scalar)
  {
    val = (void *) pg_cstring_to_text(_state->normalized_scalar);
    _state->next_scalar = false;
  }
  else
  {
    size_t len = _state->lex->prev_token_terminator - _state->result_start;
    val = (void *) pg_cstring_to_text_with_len(_state->result_start, len);
  }

  /* enlarge keys and values arrays if necessary */
  if (_state->result_count >= _state->result_size)
  {
    _state->result_size *= 2;
    _state->keys = (text **) repalloc(_state->keys, sizeof(text *) *
      _state->result_size);
    _state->values = (text **) repalloc(_state->values, sizeof(text *) *
      _state->result_size);
  }
  /* save the key and value */
  _state->keys[_state->result_count] = key;
  _state->values[_state->result_count++] = val;

  return JSON_SUCCESS;
}

static JsonParseErrorType
each_array_start(void *state)
{
  EachState  *_state = (EachState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot deconstruct an array as an object");
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
each_scalar(void *state, char *token, JsonTokenType tokentype UNUSED)
{
  EachState  *_state = (EachState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot deconstruct a scalar");
  }
  /* supply de-escaped value if required */
  if (_state->next_scalar)
    _state->normalized_scalar = token;
  return JSON_SUCCESS;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the elements of a JSON array
 * @note Derived from PostgreSQL function: @p json_array_elements()
 */
#if MEOS
text **
json_array_elements(const text *js, int *count)
{
  return elements_worker((text *) js, count, "json_array_elements", false);
}
#endif /* MEOS */
text **
pg_json_array_elements(const text *js, int *count)
{
  return elements_worker((text *) js, count, "json_array_elements", false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the elements of a JSON array as text
 * @note Derived from PostgreSQL function: @p json_array_elements_text()
 */
#if MEOS
text **
json_array_elements_text(const text *js, int *count)
{
  return elements_worker((text *) js, count, "json_array_elements_text", true);
}
#endif /* MEOS */
text **
pg_json_array_elements_text(const text *js, int *count)
{
  return elements_worker((text *) js, count, "json_array_elements_text", true);
}

static text **
elements_worker(text *js, int *count, const char *funcname, bool as_text)
{
  ElementsState *state = palloc0(sizeof(ElementsState));
  state->result_size = 256;
  state->result_count = 0;
  state->result = (text **) palloc(256 * sizeof(text *));

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->object_start = elements_object_start;
  sem->scalar = elements_scalar;
  sem->array_element_start = elements_array_element_start;
  sem->array_element_end = elements_array_element_end;

  /* elements only needs escaped strings when as_text */
  JsonLexContext lex;
  makeJsonLexContext(&lex, js, as_text);
  state->function_name = funcname;
  state->normalize_results = as_text;
  state->next_scalar = false;
  state->lex = &lex;

  pg_parse_json_or_ereport(&lex, sem);

  /* Construct the result */
  text **result = (text **) palloc(sizeof(text *) * state->result_count);
  memcpy(result, state->result, sizeof(text *) * state->result_count);
  *count = state->result_count;

  /* Clean up and return */
  json_reset_tofree();
  freeJsonLexContext(&lex);
  pfree(sem);
  pfree(state->result);
  pfree(state);
  return result;
}

static JsonParseErrorType
elements_array_element_start(void *state, bool isnull UNUSED)
{
  ElementsState *_state = (ElementsState *) state;
  /* save a pointer to where the value starts */
  if (_state->lex->lex_level == 1)
  {
    /*
     * next_scalar will be reset in the array_element_end handler, and
     * since we know the value is a scalar there is no danger of it being
     * on while recursing down the tree.
     */
    if (_state->normalize_results &&
        _state->lex->token_type == JSON_TOKEN_STRING)
      _state->next_scalar = true;
    else
      _state->result_start = _state->lex->token_start;
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
elements_array_element_end(void *state, bool isnull)
{
  ElementsState *_state = (ElementsState *) state;
  text *val;

  /* skip over nested objects */
  if (_state->lex->lex_level != 1)
    return JSON_SUCCESS;

  if (isnull && _state->normalize_results)
  {
    val = NULL;
  }
  else if (_state->next_scalar)
  {
    val = (text *) pg_cstring_to_text(_state->normalized_scalar);
    _state->next_scalar = false;
  }
  else
  {
    int len = _state->lex->prev_token_terminator - _state->result_start;
    val = pg_cstring_to_text_with_len(_state->result_start, len);
  }

  /* enlarge result array if necessary */
  if (_state->result_count >= _state->result_size)
  {
    _state->result_size *= 2;
    _state->result = (text **) repalloc(_state->result, sizeof(text *) *
      _state->result_size);
  }
  /* save the value */
  _state->result[_state->result_count++] = val;

  return JSON_SUCCESS;
}

static JsonParseErrorType
elements_object_start(void *state)
{
  ElementsState *_state = (ElementsState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot call %s on a non-array", _state->function_name);
    return JSON_INVALID_TOKEN;
  }
  return JSON_SUCCESS;
}

static JsonParseErrorType
elements_scalar(void *state, char *token, JsonTokenType tokentype UNUSED)
{
  ElementsState *_state = (ElementsState *) state;
  /* json structure check */
  if (_state->lex->lex_level == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot call %s on a scalar", _state->function_name);
    return JSON_INVALID_TOKEN;
  }
  /* supply de-escaped value if required */
  if (_state->next_scalar)
    _state->normalized_scalar = token;
  return JSON_SUCCESS;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return two arrays of keys and values of a JSONB value
 * @note Derived from PostgreSQL function: @p jsonb_each()
 */
#if MEOS
text **
jsonb_each(const Jsonb *jb, Jsonb **values, int *count)
{
  return each_worker_jsonb(jb, (void **) values, count, "jsonb_each", false);
}
#endif /* MEOS */
text **
pg_jsonb_each(const Jsonb *jb, Jsonb **values, int *count)
{
  return each_worker_jsonb(jb, (void **) values, count, "jsonb_each", false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return two arrays of keys and values of a JSONB value as text
 * @note Derived from PostgreSQL function: @p jsonb_each_text()
 */
#if MEOS
text **
jsonb_each_text(const Jsonb *jb, text **values, int *count)
{
  return each_worker_jsonb(jb, (void **) values, count, "jsonb_each_text",
    true);
}
#endif /* MEOS */
text **
pg_jsonb_each_text(const Jsonb *jb, text **values, int *count)
{
  return each_worker_jsonb(jb, (void **) values, count, "jsonb_each_text",
    true);
}

static text **
each_worker_jsonb(const Jsonb *jb, void **values, int *count,
  const char *funcname, bool as_text)
{
  if (!JB_ROOT_IS_OBJECT(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot call %s on a non-object", funcname);
    return NULL;
  }

  bool skipNested = false;
  JsonbIteratorToken r;
  JsonbValue v;
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  int result_count = 0;
  text **result_keys = (text **) palloc(sizeof(text *) * JB_ROOT_COUNT(jb));
  while ((r = JsonbIteratorNext(&it, &v, skipNested)) != WJB_DONE)
  {
    skipNested = true;
    if (r == WJB_KEY)
    {
      result_keys[result_count] = pg_cstring_to_text_with_len(v.val.string.val,
        (size_t) v.val.string.len);
      /*
       * The next thing the iterator fetches should be the value, no
       * matter what shape it is.
       */
      r = JsonbIteratorNext(&it, &v, skipNested);
      Assert(r != WJB_DONE);
      if (as_text)
      {
        if (v.type == jbvNull)
        {
          /* a json null is an sql null in text mode */
          values[result_count++] = NULL;
        }
        else
          values[result_count++] = (void *) JsonbValueAsText(&v);
      }
      else
      {
        /* Not in text mode, just return the Jsonb */
        Jsonb *val = JsonbValueToJsonb(&v);
        values[result_count++] = (void *) val;
      }
    }
  }
  *count = result_count;
  return result_keys;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the elements of a JSONB array
 * @note Derived from PostgreSQL function: @p jsonb_array_elements()
 */
#if MEOS
Jsonb **
jsonb_array_elements(const Jsonb *jb, int *count)
{
  return pg_jsonb_array_elements(jb, count);
}
#endif /* MEOS */
Jsonb **
pg_jsonb_array_elements(const Jsonb *jb, int *count)
{
  return (Jsonb **) elements_worker_jsonb(jb, count, "jsonb_array_elements",
    false);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the elements of a JSONB array as text
 * @note Derived from PostgreSQL function: @p jsonb_array_elements()
 */
#if MEOS
text **
jsonb_array_elements_text(const Jsonb *jb, int *count)
{
  return pg_jsonb_array_elements_text(jb, count);
}
#endif /* MEOS */
text **
pg_jsonb_array_elements_text(const Jsonb *jb, int *count)
{
  return (text **) elements_worker_jsonb(jb, count, "jsonb_array_elements_text", true);
}

static void **
elements_worker_jsonb(const Jsonb *jb, int *count, const char *funcname UNUSED,
  bool as_text)
{
  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot extract elements from a scalar");
    return NULL;
  }
  else if (!JB_ROOT_IS_ARRAY(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot extract elements from an object");
    return NULL;
  }

  JsonbValue v;
  bool skipNested = false;
  JsonbIteratorToken r;
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  int result_count = 0;
  void **result = (void **) palloc(sizeof(void *) * JB_ROOT_COUNT(jb));
  while ((r = JsonbIteratorNext(&it, &v, skipNested)) != WJB_DONE)
  {
    skipNested = true;
    if (r == WJB_ELEM)
    {
      if (as_text)
      {
        if (v.type == jbvNull)
        {
          /* a json null is an sql null in text mode */
          result[result_count++] =  NULL;
        }
        else
          result[result_count++] = (void *) JsonbValueAsText(&v);
      }
      else
      {
        /* Not in text mode, just return the Jsonb */
        result[result_count++] = (void *) JsonbValueToJsonb(&v);
      }
    }
  }
  *count = result_count;
  return result;
}

/*
 * Semantic actions for json_strip_nulls.
 *
 * Simply repeat the input on the output unless we encounter
 * a null object field. State for this is set when the field
 * is started and reset when the scalar action (which must be next)
 * is called.
 */

static JsonParseErrorType
sn_object_start(void *state)
{
  StripnullState *_state = (StripnullState *) state;
  appendStringInfoCharMacro(_state->strval, '{');
  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_object_end(void *state)
{
  StripnullState *_state = (StripnullState *) state;
  appendStringInfoCharMacro(_state->strval, '}');
  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_array_start(void *state)
{
  StripnullState *_state = (StripnullState *) state;
  appendStringInfoCharMacro(_state->strval, '[');
  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_array_end(void *state)
{
  StripnullState *_state = (StripnullState *) state;
  appendStringInfoCharMacro(_state->strval, ']');
  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_object_field_start(void *state, char *fname, bool isnull)
{
  StripnullState *_state = (StripnullState *) state;
  if (isnull)
  {
    /*
     * The next thing must be a scalar or isnull couldn't be true, so
     * there is no danger of this state being carried down into a nested
     * object or array. The flag will be reset in the scalar action.
     */
    _state->skip_next_null = true;
    return JSON_SUCCESS;
  }

  if (_state->strval->data[_state->strval->len - 1] != '{')
    appendStringInfoCharMacro(_state->strval, ',');

  /*
   * Unfortunately we don't have the quoted and escaped string any more, so
   * we have to re-escape it.
   */
  escape_json(_state->strval, fname);
  appendStringInfoCharMacro(_state->strval, ':');
  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_array_element_start(void *state, bool isnull)
{
  StripnullState *_state = (StripnullState *) state;
  /* If strip_in_arrays is enabled and this is a null, mark it for skipping */
  if (isnull && _state->strip_in_arrays)
  {
    _state->skip_next_null = true;
    return JSON_SUCCESS;
  }

  /* Only add a comma if this is not the first valid element */
  if (_state->strval->len > 0 &&
    _state->strval->data[_state->strval->len - 1] != '[')
  {
    appendStringInfoCharMacro(_state->strval, ',');
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
sn_scalar(void *state, char *token, JsonTokenType tokentype)
{
  StripnullState *_state = (StripnullState *) state;
  if (_state->skip_next_null)
  {
    Assert(tokentype == JSON_TOKEN_NULL);
    _state->skip_next_null = false;
    return JSON_SUCCESS;
  }

  if (tokentype == JSON_TOKEN_STRING)
    escape_json(_state->strval, token);
  else
    appendStringInfoString(_state->strval, token);

  return JSON_SUCCESS;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return a JSON value without nulls
 * @note Derived from PostgreSQL function: @p json_strip_nulls()
 */
#if MEOS
text *
json_strip_nulls(const text *js, bool strip_in_arrays)
{
  return pg_json_strip_nulls(js, strip_in_arrays);
}
#endif /* MEOS */
text *
pg_json_strip_nulls(const text *js, bool strip_in_arrays)
{
  JsonLexContext lex;
  StripnullState *state = palloc0(sizeof(StripnullState));
  state->lex = makeJsonLexContext(&lex, (text *) js, true);
  state->strval = makeStringInfo();
  state->skip_next_null = false;
  state->strip_in_arrays = strip_in_arrays;

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->object_start = sn_object_start;
  sem->object_end = sn_object_end;
  sem->array_start = sn_array_start;
  sem->array_end = sn_array_end;
  sem->scalar = sn_scalar;
  sem->array_element_start = sn_array_element_start;
  sem->object_field_start = sn_object_field_start;

  pg_parse_json_or_ereport(&lex, sem);
  text *result = pg_cstring_to_text_with_len(state->strval->data,
    (size_t) state->strval->len); // MEOS

  /* Clean up and return */
  freeJsonLexContext(&lex);
  destroyStringInfo(state->strval);
  pfree(state);
  pfree(sem);
  return result;
}

/**
 * @ingroup meos_json_base_transf
 * @brief Return a JSONB value without nulls
 * @return On error return @p DBL_MAX
 * @note Derived from PostgreSQL function: @p jsonb_strip_nulls()
 */
#if MEOS
Jsonb *
jsonb_strip_nulls(const Jsonb *jb, bool strip_in_arrays)
{
  return pg_jsonb_strip_nulls(jb, strip_in_arrays);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_strip_nulls(const Jsonb *jb, bool strip_in_arrays)
{
  JsonbParseState *parseState = NULL;
  JsonbValue *res = NULL;
  JsonbValue v, k;
  JsonbIteratorToken type;
  bool last_was_key = false;

  if (JB_ROOT_IS_SCALAR(jb))
    return pg_jsonb_copy(jb);

  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    Assert(!(type == WJB_KEY && last_was_key));
    if (type == WJB_KEY)
    {
      /* stash the key until we know if it has a null value */
      k = v;
      last_was_key = true;
      continue;
    }

    if (last_was_key)
    {
      /* if the last element was a key this one can't be */
      last_was_key = false;

      /* skip this field if value is null */
      if (type == WJB_VALUE && v.type == jbvNull)
        continue;

      /* otherwise, do a delayed push of the key */
      (void) pushJsonbValue(&parseState, WJB_KEY, &k);
    }

    /* if strip_in_arrays is set, also skip null array elements */
    if (strip_in_arrays)
      if (type == WJB_ELEM && v.type == jbvNull)
        continue;

    if (type == WJB_VALUE || type == WJB_ELEM)
      res = pushJsonbValue(&parseState, type, &v);
    else
      res = pushJsonbValue(&parseState, type, NULL);
  }

  Assert(res != NULL);

  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return a pretty-printed text from a JSONB value
 * @return On error return @p DBL_MAX
 * @note Derived from PostgreSQL function: @p jsonb_pretty()
 */
#if MEOS
text *
jsonb_pretty(const Jsonb *jb)
{
  return pg_jsonb_pretty(jb);
}
#endif /* MEOS */
text *
pg_jsonb_pretty(const Jsonb *jb)
{
  StringInfo str = makeStringInfo();
  JsonbToCStringIndent(str, (JsonbContainer *) &jb->root, VARSIZE(jb));
  text *result = pg_cstring_to_text_with_len(str->data, str->len); // MEOS
  destroyStringInfo(str);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return the concatenation of two JSONB values (objects ou arrays)
 * @return On error return @p DBL_MAX
 * @note Derived from PostgreSQL function: @p jsonb_concat()
 */
#if MEOS
Jsonb *
jsonb_concat(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_concat(jb1, jb2);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_concat(const Jsonb *jb1, const Jsonb *jb2)
{
  /*
   * If one of the jsonb is empty, just return the other if it's not scalar
   * and both are of the same kind.  If it's a scalar or they are of
   * different kinds we need to perform the concatenation even if one is
   * empty.
   */
  if (JB_ROOT_IS_OBJECT(jb1) == JB_ROOT_IS_OBJECT(jb2))
  {
    if (JB_ROOT_COUNT(jb1) == 0 && !JB_ROOT_IS_SCALAR(jb2))
      return pg_jsonb_copy(jb2);
    else if (JB_ROOT_COUNT(jb2) == 0 && !JB_ROOT_IS_SCALAR(jb1))
      return pg_jsonb_copy(jb1);
  }

  JsonbIterator *it1 = JsonbIteratorInit((JsonbContainer *) &jb1->root);
  JsonbIterator *it2 = JsonbIteratorInit((JsonbContainer *) &jb2->root);
  JsonbParseState *state = NULL;
  JsonbValue *res = IteratorConcat(&it1, &it2, &state);

  Assert(res != NULL);
  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return a copy of a JSONB value with an item removed
 * @note Derived from the PostgreSQL function @p jsonb_delete()
 */
#if MEOS
Jsonb *
jsonb_delete(const Jsonb *jb, const text *key)
{
  return pg_jsonb_delete(jb, key);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_delete(const Jsonb *jb, const text *key)
{
  char *keyptr = VARDATA_ANY(key);
  int keylen = VARSIZE_ANY_EXHDR(key);
  JsonbParseState *state = NULL;
  JsonbValue v, *res = NULL;

  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot delete from scalar");
    return NULL;
  }

  if (JB_ROOT_COUNT(jb) == 0)
    return pg_jsonb_copy(jb);

  JsonbIteratorToken r;
  bool skipNested = false;
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  while ((r = JsonbIteratorNext(&it, &v, skipNested)) != WJB_DONE)
  {
    skipNested = true;
    if ((r == WJB_ELEM || r == WJB_KEY) &&
        (v.type == jbvString && keylen == v.val.string.len &&
         memcmp(keyptr, v.val.string.val, keylen) == 0))
    {
      /* skip corresponding value as well */
      if (r == WJB_KEY)
        (void) JsonbIteratorNext(&it, &v, true);
      continue;
    }
    res = pushJsonbValue(&state, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
  }

  Assert(res != NULL);

  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return a copy of a JSONB value with an array of items removed
 * @note Derived from PostgreSQL function @p jsonb_delete_array()
 */
#if MEOS
Jsonb *
jsonb_delete_array(const Jsonb *jb, text **keys_elems, int keys_len)
{
  return pg_jsonb_delete_array(jb, keys_elems, keys_len);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_delete_array(const Jsonb *jb, text **keys_elems, int keys_len)
{
  /* Ensure the validity of the arguments */
  assert(jb); assert(keys_elems); assert(keys_len > 0);

  JsonbParseState *state = NULL;
  JsonbValue v, *res = NULL;

  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot delete from scalar");
    return NULL;
  }

  if (JB_ROOT_COUNT(jb) == 0 || keys_len == 0)
    return pg_jsonb_copy((Jsonb *) jb);

  bool skipNested = false;
  JsonbIteratorToken r;
  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *) jb)->root);
  while ((r = JsonbIteratorNext(&it, &v, skipNested)) != WJB_DONE)
  {
    skipNested = true;
    if ((r == WJB_ELEM || r == WJB_KEY) && v.type == jbvString)
    {
      int i;
      bool found = false;
      for (i = 0; i < keys_len; i++)
      {
        if (! keys_elems[i])
          continue;

        /* We rely on the array elements not being toasted */
        char *keyptr = VARDATA_ANY(keys_elems[i]);
        int keylen = VARSIZE_ANY_EXHDR(keys_elems[i]);
        if (keylen == v.val.string.len &&
          memcmp(keyptr, v.val.string.val, keylen) == 0)
        {
          found = true;
          break;
        }
      }
      if (found)
      {
        /* skip corresponding value as well */
        if (r == WJB_KEY)
          (void) JsonbIteratorNext(&it, &v, true);
        continue;
      }
    }
    res = pushJsonbValue(&state, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
  }

  assert(res != NULL);
  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Return a copy of a JSONB array with an item removed
 * @details Negative int means count back from the end of the items
 * @note Derived from PostgreSQL function @p jsonb_delete_idx(jsonb, int)
 */
#if MEOS
Jsonb *
jsonb_delete_index(const Jsonb *jb, int idx)
{
  return pg_jsonb_delete_index(jb, idx);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_delete_index(const Jsonb *jb, int idx)
{
  JsonbParseState *state = NULL;
  uint32 i = 0, n;
  JsonbValue v, *res = NULL;

  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot delete from scalar");
    return NULL;
  }

  if (JB_ROOT_IS_OBJECT(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot delete from object using integer index");
    return NULL;
  }

  if (JB_ROOT_COUNT(jb) == 0)
    return pg_jsonb_copy(jb);

  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  JsonbIteratorToken r = JsonbIteratorNext(&it, &v, false);
  assert(r == WJB_BEGIN_ARRAY);
  n = v.val.array.nElems;

  if (idx < 0)
  {
    if (pg_abs_s32(idx) > n)
      idx = n;
    else
      idx = n + idx;
  }
  if (idx >= (int) n)
    return pg_jsonb_copy(jb);

  pushJsonbValue(&state, r, NULL);
  while ((r = JsonbIteratorNext(&it, &v, true)) != WJB_DONE)
  {
    if (r == WJB_ELEM)
    {
      if ((int) i++ == idx)
        continue;
    }
    res = pushJsonbValue(&state, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
  }

  assert(res != NULL);
  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Replace a JSONB value specified by a path with a new value
 * @note Derived from the PostgreSQL function @p
 * jsonb_set(jsonb, text[], jsonb, boolean)
 */
#if MEOS
Jsonb *
jsonb_set(const Jsonb *jb, text **path_elems, int path_len, const Jsonb *newjb,
  bool create)
{
  return pg_jsonb_set(jb, path_elems, path_len, newjb, create);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_set(const Jsonb *jb, text **path_elems, int path_len,
  const Jsonb *newjb, bool create)
{
  JsonbValue newval;
  JsonbToJsonbValue((Jsonb *) newjb, &newval);

  if (JB_ROOT_IS_SCALAR(jb))
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot set path jb scalar");

  if (JB_ROOT_COUNT(jb) == 0 && !create)
    return pg_jsonb_copy((Jsonb *) jb);

  if (path_len == 0)
    return pg_jsonb_copy((Jsonb *) jb);

  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *)jb)->root);
  JsonbParseState *st = NULL;
  bool *path_nulls = palloc0(path_len * sizeof(bool));
  JsonbValue *res = setPath(&it, (Datum *) path_elems, path_nulls, path_len,
    &st, 0, &newval, create ? JB_PATH_CREATE : JB_PATH_REPLACE);

  assert(res != NULL);
  return JsonbValueToJsonb(res);
}

/**
 * @ingroup meos_json_base_transf
 * @brief Replace a JSONB value specified by a path with a new value
 * elements can be either field keys or array indexes
 * @note Derived from PostgreSQL function
 * @p jsonb_set_lax(jsonb, text[], jsonb, boolean, text)
 */
#if MEOS
Jsonb *
jsonb_set_lax(const Jsonb *jb, text **path_elems, int path_len,
  const Jsonb *newjb, bool create, const text *handle_null)
{
  return pg_jsonb_set_lax(jb, path_elems, path_len, newjb, create,
    handle_null);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_set_lax(const Jsonb *jb, text **path_elems, int path_len,
  const Jsonb *newjb, bool create, const text *handle_null)
{
  if (! jb || ! path_elems || ! path_len)
    return NULL;

  /* could happen if they pass in an explicit NULL */
  if (! handle_null)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "null_value_treatment must be \"delete_key\", "
      "\"return_target\", \"use_json_null\", or \"raise_exception\"");
    return NULL;
  }

  /* if the new value isn't an SQL NULL just call jsonb_set */
  if (! path_elems)
    return pg_jsonb_set(jb, path_elems, path_len, newjb, create);

  char *handle_val = pg_text_to_cstring(handle_null);
  if (strcmp(handle_val, "raise_exception") == 0)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "JSON value must not be null");
    pfree(handle_val);
    return NULL;
  }
  else if (strcmp(handle_val, "use_json_null") == 0)
  {
    Jsonb *null = pg_jsonb_in("null");
    Jsonb *result = pg_jsonb_set(jb, path_elems, path_len, null, create);
    pfree(handle_val);
    pfree(null);
    return result;
  }
  else if (strcmp(handle_val, "delete_key") == 0)
  {
    pfree(handle_val);
    return pg_jsonb_delete_path(jb, path_elems, path_len);
  }
  else if (strcmp(handle_val, "return_target") == 0)
  {
    pfree(handle_val);
    return pg_jsonb_copy(jb);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "null_value_treatment must be \"delete_key\", \"return_target\", "
      "\"use_json_null\", or \"raise_exception\"");
    pfree(handle_val);
    return NULL;
  }
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Delete the field or array element at the specified path, where path
 * elements can be either field keys or array indexes
 * @note Derived from PostgreSQL function @p jsonb_delete_path()
 */
#if MEOS
Jsonb *
jsonb_delete_path(const Jsonb *jb, text **path_elems, int path_len)
{
  return pg_jsonb_delete_path(jb, path_elems, path_len);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_delete_path(const Jsonb *jb, text **path_elems, int path_len)
{
  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot delete path in scalar");
    return NULL;
  }

  if (JB_ROOT_COUNT(jb) == 0 || path_len == 0)
    return pg_jsonb_copy((Jsonb *) jb);

  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *) jb)->root);
  JsonbParseState *st = NULL;
  bool *path_nulls = palloc0(path_len * sizeof(bool));
  JsonbValue *res = setPath(&it, (Datum *) path_elems, path_nulls, path_len,
    &st, 0, NULL, JB_PATH_DELETE);

  assert(res != NULL);
  pfree(path_nulls);
  return JsonbValueToJsonb(res);
}

/*****************************************************************************/

/**
 * @ingroup meos_json_base_transf
 * @brief Replace a JSONB value specified by a path with a new value
 * @note Derived from the PostgreSQL function @p
 * jsonb_insert(jsonb, text[], jsonb, boolean)
 */
#if MEOS
Jsonb *
jsonb_insert(const Jsonb *jb, text **path_elems, int path_len,
  const Jsonb *newjb, bool after)
{
  return pg_jsonb_insert(jb, path_elems, path_len, newjb, after);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_insert(const Jsonb *jb, text **path_elems, int path_len,
  const Jsonb *newjb, bool after)
{
  JsonbValue newval;
  JsonbToJsonbValue((Jsonb *) newjb, &newval);

  if (JB_ROOT_IS_SCALAR(jb))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "cannot set path in scalar");
    return NULL;
  }

  if (path_len == 0)
    return pg_jsonb_copy((Jsonb *) jb);

  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *)jb)->root);
  JsonbParseState *st = NULL;
  bool *path_nulls = palloc0(path_len * sizeof(bool));
  JsonbValue *res = setPath(&it, (Datum *) path_elems, path_nulls, path_len,
    &st, 0, &newval, after ? JB_PATH_INSERT_AFTER : JB_PATH_INSERT_BEFORE);

  assert(res != NULL); 
  pfree(path_nulls);
  return JsonbValueToJsonb(res);
}

/*
 * Iterate over all jsonb objects and merge them into one.
 * The logic of this function copied from the same hstore function,
 * except the case, when it1 & it2 represents jbvObject.
 * In that case we just append the content of it2 to it1 without any
 * verifications.
 */
static JsonbValue *
IteratorConcat(JsonbIterator **it1, JsonbIterator **it2,
  JsonbParseState **state)
{
  JsonbValue v1, v2, *res = NULL;
  JsonbIteratorToken r1, r2;

  JsonbIteratorToken rk1 = JsonbIteratorNext(it1, &v1, false);
  JsonbIteratorToken rk2 = JsonbIteratorNext(it2, &v2, false);

  /*
   * JsonbIteratorNext reports raw scalars as if they were single-element
   * arrays; hence we only need consider "object" and "array" cases here.
   */
  if (rk1 == WJB_BEGIN_OBJECT && rk2 == WJB_BEGIN_OBJECT)
  {
    /*
     * Both inputs are objects.
     *
     * Append all the tokens from v1 to res, except last WJB_END_OBJECT
     * (because res will not be finished yet).
     */
    pushJsonbValue(state, rk1, NULL);
    while ((r1 = JsonbIteratorNext(it1, &v1, true)) != WJB_END_OBJECT)
      pushJsonbValue(state, r1, &v1);

    /*
     * Append all the tokens from v2 to res, including last WJB_END_OBJECT
     * (the concatenation will be completed).  Any duplicate keys will
     * automatically override the value from the first object.
     */
    while ((r2 = JsonbIteratorNext(it2, &v2, true)) != WJB_DONE)
      res = pushJsonbValue(state, r2, r2 != WJB_END_OBJECT ? &v2 : NULL);
  }
  else if (rk1 == WJB_BEGIN_ARRAY && rk2 == WJB_BEGIN_ARRAY)
  {
    /*
     * Both inputs are arrays.
     */
    pushJsonbValue(state, rk1, NULL);

    while ((r1 = JsonbIteratorNext(it1, &v1, true)) != WJB_END_ARRAY)
    {
      Assert(r1 == WJB_ELEM);
      pushJsonbValue(state, r1, &v1);
    }

    while ((r2 = JsonbIteratorNext(it2, &v2, true)) != WJB_END_ARRAY)
    {
      Assert(r2 == WJB_ELEM);
      pushJsonbValue(state, WJB_ELEM, &v2);
    }

    res = pushJsonbValue(state, WJB_END_ARRAY, NULL /* signal to sort */ );
  }
  else if (rk1 == WJB_BEGIN_OBJECT)
  {
    /*
     * We have object || array.
     */
    Assert(rk2 == WJB_BEGIN_ARRAY);

    pushJsonbValue(state, WJB_BEGIN_ARRAY, NULL);
    pushJsonbValue(state, WJB_BEGIN_OBJECT, NULL);
    while ((r1 = JsonbIteratorNext(it1, &v1, true)) != WJB_DONE)
      pushJsonbValue(state, r1, r1 != WJB_END_OBJECT ? &v1 : NULL);

    while ((r2 = JsonbIteratorNext(it2, &v2, true)) != WJB_DONE)
      res = pushJsonbValue(state, r2, r2 != WJB_END_ARRAY ? &v2 : NULL);
  }
  else
  {
    /*
     * We have array || object.
     */
    Assert(rk1 == WJB_BEGIN_ARRAY);
    Assert(rk2 == WJB_BEGIN_OBJECT);

    pushJsonbValue(state, WJB_BEGIN_ARRAY, NULL);
    while ((r1 = JsonbIteratorNext(it1, &v1, true)) != WJB_END_ARRAY)
      pushJsonbValue(state, r1, &v1);

    pushJsonbValue(state, WJB_BEGIN_OBJECT, NULL);
    while ((r2 = JsonbIteratorNext(it2, &v2, true)) != WJB_DONE)
      pushJsonbValue(state, r2, r2 != WJB_END_OBJECT ? &v2 : NULL);

    res = pushJsonbValue(state, WJB_END_ARRAY, NULL);
  }
  return res;
}

/*
 * Do most of the heavy work for jsonb_set/jsonb_insert
 *
 * If JB_PATH_DELETE bit is set in op_type, the element is to be removed.
 *
 * If any bit mentioned in JB_PATH_CREATE_OR_INSERT is set in op_type,
 * we create the new value if the key or array index does not exist.
 *
 * Bits JB_PATH_INSERT_BEFORE and JB_PATH_INSERT_AFTER in op_type
 * behave as JB_PATH_CREATE if new value is inserted in JsonbObject.
 *
 * If JB_PATH_FILL_GAPS bit is set, this will change an assignment logic in
 * case if target is an array. The assignment index will not be restricted by
 * number of elements in the array, and if there are any empty slots between
 * last element of the array and a new one they will be filled with nulls. If
 * the index is negative, it still will be considered an index from the end
 * of the array. Of a part of the path is not present and this part is more
 * than just one last element, this flag will instruct to create the whole
 * chain of corresponding objects and insert the value.
 *
 * JB_PATH_CONSISTENT_POSITION for an array indicates that the caller wants to
 * keep values with fixed indices. Indices for existing elements could be
 * changed (shifted forward) in case if the array is prepended with a new value
 * and a negative index out of the range, so this behavior will be prevented
 * and return an error.
 *
 * All path elements before the last must already exist
 * whatever bits in op_type are set, or nothing is done.
 */
static JsonbValue *
setPath(JsonbIterator **it, Datum *path_elems, bool *path_nulls, int path_len,
  JsonbParseState **st, int level, JsonbValue *newval, int op_type)
{
  if (path_nulls[level])
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "path element at position %d is null", level + 1);
    return NULL;
  }

  JsonbValue *res;
  JsonbValue v;
  JsonbIteratorToken r = JsonbIteratorNext(it, &v, false);
  switch (r)
  {
    case WJB_BEGIN_ARRAY:

      /*
       * If instructed complain about attempts to replace within a raw
       * scalar value. This happens even when current level is equal to
       * path_len, because the last path key should also correspond to
       * an object or an array, not raw scalar.
       */
      if ((op_type & JB_PATH_FILL_GAPS) && (level <= path_len - 1) &&
        v.val.array.rawScalar)
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "cannot replace existing key");
        return NULL;
      }

      (void) pushJsonbValue(st, r, NULL);
      setPathArray(it, path_elems, path_nulls, path_len, st, level, newval,
        v.val.array.nElems, op_type);
      r = JsonbIteratorNext(it, &v, false);
      Assert(r == WJB_END_ARRAY);
      res = pushJsonbValue(st, r, NULL);
      break;
    case WJB_BEGIN_OBJECT:
      (void) pushJsonbValue(st, r, NULL);
      setPathObject(it, path_elems, path_nulls, path_len, st, level, newval,
        v.val.object.nPairs, op_type);
      r = JsonbIteratorNext(it, &v, true);
      Assert(r == WJB_END_OBJECT);
      res = pushJsonbValue(st, r, NULL);
      break;
    case WJB_ELEM:
    case WJB_VALUE:

      /*
       * If instructed complain about attempts to replace within a
       * scalar value. This happens even when current level is equal to
       * path_len, because the last path key should also correspond to
       * an object or an array, not an element or value.
       */
      if ((op_type & JB_PATH_FILL_GAPS) && (level <= path_len - 1))
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "cannot replace existing key");
        return NULL;
      }

      res = pushJsonbValue(st, r, &v);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "unrecognized iterator result: %d", (int) r);
      return NULL;
  }

  return res;
}

/*
 * Object walker for setPath
 */
static void
setPathObject(JsonbIterator **it, Datum *path_elems, bool *path_nulls,
  int path_len, JsonbParseState **st, int level, JsonbValue *newval,
  uint32 npairs, int op_type)
{
  text *pathelem = NULL;
  int i;
  JsonbValue k, v;
  bool done = false;

  if (level >= path_len || path_nulls[level])
    done = true;
  else
  {
    pathelem = DatumGetTextP(path_elems[level]);
  }

  /* empty object is a special case for create */
  if ((npairs == 0) && (op_type & JB_PATH_CREATE_OR_INSERT) &&
    (level == path_len - 1))
  {
    JsonbValue newkey;
    newkey.type = jbvString;
    newkey.val.string.val = VARDATA_ANY(pathelem);
    newkey.val.string.len = VARSIZE_ANY_EXHDR(pathelem);
    (void) pushJsonbValue(st, WJB_KEY, &newkey);
    (void) pushJsonbValue(st, WJB_VALUE, newval);
  }

  for (i = 0; i < (int) npairs; i++)
  {
    JsonbIteratorToken r = JsonbIteratorNext(it, &k, true);
    Assert(r == WJB_KEY);
    if (!done &&
      k.val.string.len == (int) VARSIZE_ANY_EXHDR(pathelem) &&
      memcmp(k.val.string.val, VARDATA_ANY(pathelem), k.val.string.len) == 0)
    {
      done = true;
      if (level == path_len - 1)
      {
        /*
         * called from jsonb_insert(), it forbids redefining an
         * existing value
         */
        if (op_type & (JB_PATH_INSERT_BEFORE | JB_PATH_INSERT_AFTER))
        {
          meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
            "cannot replace existing key");
          return; // TODO
        }

        r = JsonbIteratorNext(it, &v, true);  /* skip value */
        if (!(op_type & JB_PATH_DELETE))
        {
          (void) pushJsonbValue(st, WJB_KEY, &k);
          (void) pushJsonbValue(st, WJB_VALUE, newval);
        }
      }
      else
      {
        (void) pushJsonbValue(st, r, &k);
        setPath(it, path_elems, path_nulls, path_len, st, level + 1, newval,
          op_type);
      }
    }
    else
    {
      if ((op_type & JB_PATH_CREATE_OR_INSERT) && !done &&
        level == path_len - 1 && i == ((int) npairs) - 1)
      {
        JsonbValue  newkey;
        newkey.type = jbvString;
        newkey.val.string.val = VARDATA_ANY(pathelem);
        newkey.val.string.len = VARSIZE_ANY_EXHDR(pathelem);
        (void) pushJsonbValue(st, WJB_KEY, &newkey);
        (void) pushJsonbValue(st, WJB_VALUE, newval);
      }

      (void) pushJsonbValue(st, r, &k);
      r = JsonbIteratorNext(it, &v, false);
      (void) pushJsonbValue(st, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
      if (r == WJB_BEGIN_ARRAY || r == WJB_BEGIN_OBJECT)
      {
        int walking_level = 1;
        while (walking_level != 0)
        {
          r = JsonbIteratorNext(it, &v, false);
          if (r == WJB_BEGIN_ARRAY || r == WJB_BEGIN_OBJECT)
            ++walking_level;
          if (r == WJB_END_ARRAY || r == WJB_END_OBJECT)
            --walking_level;
          (void) pushJsonbValue(st, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
        }
      }
    }
  }

  /*--
   * If we got here there are only few possibilities:
   * - no target path was found, and an open object with some keys/values was
   *   pushed into the state
   * - an object is empty, only WJB_BEGIN_OBJECT is pushed
   *
   * In both cases if instructed to create the path when not present,
   * generate the whole chain of empty objects and insert the new value
   * there.
   */
  if (! done && (op_type & JB_PATH_FILL_GAPS) && (level < path_len - 1))
  {
    JsonbValue  newkey;
    newkey.type = jbvString;
    newkey.val.string.val = VARDATA_ANY(pathelem);
    newkey.val.string.len = VARSIZE_ANY_EXHDR(pathelem);
    (void) pushJsonbValue(st, WJB_KEY, &newkey);
    (void) push_path(st, level, path_elems, path_nulls, path_len, newval);
    /* Result is closed with WJB_END_OBJECT outside of this function */
  }
}

/*
 * Array walker for setPath
 */
static void
setPathArray(JsonbIterator **it, Datum *path_elems, bool *path_nulls,
  int path_len, JsonbParseState **st, int level, JsonbValue *newval,
  uint32 nelems, int op_type)
{
  JsonbValue v;
  int idx, i;
  bool done = false;

  /* pick correct index */
  if (level < path_len && ! path_nulls[level])
  {
    char *path_str = pg_text_to_cstring(DatumGetTextP(path_elems[level]));
    char *c = path_str;
    char *badp;
    errno = 0;
    idx = strtoint(c, &badp, 10);
    if (badp == c || *badp != '\0' || errno != 0)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "path element at position %d is not an integer: \"%s\"", level + 1, c);
      return;
    }
  }
  else
    idx = nelems;

  if (idx < 0)
  {
    if (pg_abs_s32(idx) > nelems)
    {
      /*
       * If asked to keep elements position consistent, it's not allowed
       * to prepend the array.
       */
      if (op_type & JB_PATH_CONSISTENT_POSITION)
      {
        meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
          "path element at position %d is out of range: %d", level + 1, idx);
        return;
      }
      else
        idx = PG_INT32_MIN;
    }
    else
      idx = nelems + idx;
  }

  /*
   * Filling the gaps means there are no limits on the positive index are
   * imposed, we can set any element. Otherwise limit the index by nelems.
   */
  if (!(op_type & JB_PATH_FILL_GAPS))
  {
    if (idx > 0 && idx > (int) nelems)
      idx = nelems;
  }

  /*
   * if we're creating, and idx == INT_MIN, we prepend the new value to the
   * array also if the array is empty - in which case we don't really care
   * what the idx value is
   */
  if ((idx == INT_MIN || nelems == 0) && (level == path_len - 1) &&
    (op_type & JB_PATH_CREATE_OR_INSERT))
  {
    Assert(newval != NULL);
    if (op_type & JB_PATH_FILL_GAPS && nelems == 0 && idx > 0)
      push_null_elements(st, idx);
    (void) pushJsonbValue(st, WJB_ELEM, newval);
    done = true;
  }

  /* iterate over the array elements */
  for (i = 0; i < (int) nelems; i++)
  {
    JsonbIteratorToken r;

    if (i == idx && level < path_len)
    {
      done = true;
      if (level == path_len - 1)
      {
        r = JsonbIteratorNext(it, &v, true);  /* skip */
        if (op_type & (JB_PATH_INSERT_BEFORE | JB_PATH_CREATE))
          (void) pushJsonbValue(st, WJB_ELEM, newval);

        /*
         * We should keep current value only in case of
         * JB_PATH_INSERT_BEFORE or JB_PATH_INSERT_AFTER because
         * otherwise it should be deleted or replaced
         */
        if (op_type & (JB_PATH_INSERT_AFTER | JB_PATH_INSERT_BEFORE))
          (void) pushJsonbValue(st, r, &v);

        if (op_type & (JB_PATH_INSERT_AFTER | JB_PATH_REPLACE))
          (void) pushJsonbValue(st, WJB_ELEM, newval);
      }
      else
        (void) setPath(it, path_elems, path_nulls, path_len, st, level + 1,
          newval, op_type);
    }
    else
    {
      r = JsonbIteratorNext(it, &v, false);
      (void) pushJsonbValue(st, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
      if (r == WJB_BEGIN_ARRAY || r == WJB_BEGIN_OBJECT)
      {
        int walking_level = 1;
        while (walking_level != 0)
        {
          r = JsonbIteratorNext(it, &v, false);
          if (r == WJB_BEGIN_ARRAY || r == WJB_BEGIN_OBJECT)
            ++walking_level;
          if (r == WJB_END_ARRAY || r == WJB_END_OBJECT)
            --walking_level;
          (void) pushJsonbValue(st, r, r < WJB_BEGIN_ARRAY ? &v : NULL);
        }
      }
    }
  }

  if ((op_type & JB_PATH_CREATE_OR_INSERT) && !done && level == path_len - 1)
  {
    /*
     * If asked to fill the gaps, idx could be bigger than nelems, so
     * prepend the new element with nulls if that's the case.
     */
    if (op_type & JB_PATH_FILL_GAPS && idx > (int) nelems)
      push_null_elements(st, idx - nelems);
    (void) pushJsonbValue(st, WJB_ELEM, newval);
    done = true;
  }

  /*--
   * If we got here there are only few possibilities:
   * - no target path was found, and an open array with some keys/values was
   *   pushed into the state
   * - an array is empty, only WJB_BEGIN_ARRAY is pushed
   *
   * In both cases if instructed to create the path when not present,
   * generate the whole chain of empty objects and insert the new value
   * there.
   */
  if (!done && (op_type & JB_PATH_FILL_GAPS) && (level < path_len - 1))
  {
    if (idx > 0)
      push_null_elements(st, idx - nelems);
    (void) push_path(st, level, path_elems, path_nulls, path_len, newval);
    /* Result is closed with WJB_END_OBJECT outside of this function */
  }
}

/*
 * Parse information about what elements of a JSONB document we want to iterate
 * in functions iterate_json(b)_values. This information is presented in jsonb
 * format, so that it can be easily extended in the future.
 * On error return UINT_MAX // MEOS
 */
uint32
parse_jsonb_index_flags(Jsonb *jb)
{
  JsonbValue v;
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  JsonbIteratorToken type = JsonbIteratorNext(&it, &v, false);

  /*
   * We iterate over array (scalar internally is represented as array, so,
   * we will accept it too) to check all its elements.  Flag names are
   * chosen the same as jsonb_typeof uses.
   */
  if (type != WJB_BEGIN_ARRAY)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "wrong flag type, only arrays and scalars are allowed");
    return UINT_MAX;
  }

  uint32 flags = 0;
  while ((type = JsonbIteratorNext(&it, &v, false)) == WJB_ELEM)
  {
    if (v.type != jbvString)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "flag array element is not a string");
      return UINT_MAX;
    }

    if (v.val.string.len == 3 &&
      pg_strncasecmp(v.val.string.val, "all", 3) == 0)
      flags |= jtiAll;
    else if (v.val.string.len == 3 &&
         pg_strncasecmp(v.val.string.val, "key", 3) == 0)
      flags |= jtiKey;
    else if (v.val.string.len == 6 &&
         pg_strncasecmp(v.val.string.val, "string", 6) == 0)
      flags |= jtiString;
    else if (v.val.string.len == 7 &&
         pg_strncasecmp(v.val.string.val, "numeric", 7) == 0)
      flags |= jtiNumeric;
    else if (v.val.string.len == 7 &&
         pg_strncasecmp(v.val.string.val, "boolean", 7) == 0)
      flags |= jtiBool;
    else
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "wrong flag in flag array: \"%s\"",
        pnstrdup(v.val.string.val, v.val.string.len));
      return UINT_MAX;
    }
  }

  /* expect end of array now */
  if (type != WJB_END_ARRAY)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unexpected end of flag array");
    return UINT_MAX;
  }

  /* get final WJB_DONE and free iterator */
  type = JsonbIteratorNext(&it, &v, false);
  if (type != WJB_DONE)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "unexpected end of flag array");
      return UINT_MAX;
  }

  return flags;
}

/*
 * Iterate over jsonb values or elements, specified by flags, and pass them
 * together with an iteration state to a specified JsonIterateStringValuesAction.
 */
void
iterate_jsonb_values(Jsonb *jb, uint32 flags, void *state,
  JsonIterateStringValuesAction action)
{
  /*
   * Just recursively iterating over jsonb and call callback on all
   * corresponding elements
   */
  JsonbValue v;
  JsonbIteratorToken type;
  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    if (type == WJB_KEY)
    {
      if (flags & jtiKey)
        action(state, v.val.string.val, v.val.string.len);
      continue;
    }
    else if (!(type == WJB_VALUE || type == WJB_ELEM))
    {
      /* do not call callback for composite JsonbValue */
      continue;
    }

    /* JsonbValue is a value of object or element of array */
    switch (v.type)
    {
      case jbvString:
        if (flags & jtiString)
          action(state, v.val.string.val, v.val.string.len);
        break;
      case jbvNumeric:
        if (flags & jtiNumeric)
        {
          char *val = pg_numeric_out(v.val.numeric);
          action(state, val, strlen(val));
          pfree(val);
        }
        break;
      case jbvBool:
        if (flags & jtiBool)
        {
          if (v.val.boolean)
            action(state, "true", 4);
          else
            action(state, "false", 5);
        }
        break;
      default:
        /* do not call callback for composite JsonbValue */
        break;
    }
  }
}

/*
 * Iterate over json values and elements, specified by flags, and pass them
 * together with an iteration state to a specified JsonIterateStringValuesAction.
 */
void
iterate_json_values(text *js, uint32 flags, void *action_state,
  JsonIterateStringValuesAction action)
{
  JsonLexContext lex;
  IterateJsonStringValuesState *state =
    palloc0(sizeof(IterateJsonStringValuesState));
  state->lex = makeJsonLexContext(&lex, js, true);
  state->action = action;
  state->action_state = action_state;
  state->flags = flags;

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->scalar = iterate_values_scalar;
  sem->object_field_start = iterate_values_object_field_start;

  pg_parse_json_or_ereport(&lex, sem);
  freeJsonLexContext(&lex);
  pfree(sem);
  return;
}

/*
 * An auxiliary function for iterate_json_values to invoke a specified
 * JsonIterateStringValuesAction for specified values.
 */
static JsonParseErrorType
iterate_values_scalar(void *state, char *token, JsonTokenType tokentype)
{
  IterateJsonStringValuesState *_state = (IterateJsonStringValuesState *) state;
  switch (tokentype)
  {
    case JSON_TOKEN_STRING:
      if (_state->flags & jtiString)
        _state->action(_state->action_state, token, strlen(token));
      break;
    case JSON_TOKEN_NUMBER:
      if (_state->flags & jtiNumeric)
        _state->action(_state->action_state, token, strlen(token));
      break;
    case JSON_TOKEN_TRUE:
    case JSON_TOKEN_FALSE:
      if (_state->flags & jtiBool)
        _state->action(_state->action_state, token, strlen(token));
      break;
    default:
      /* do not call callback for any other token */
      break;
  }

  return JSON_SUCCESS;
}

static JsonParseErrorType
iterate_values_object_field_start(void *state, char *fname, bool isnull UNUSED)
{
  IterateJsonStringValuesState *_state = (IterateJsonStringValuesState *) state;
  if (_state->flags & jtiKey)
  {
    char *val = pstrdup(fname);
    _state->action(_state->action_state, val, strlen(val));
  }
  return JSON_SUCCESS;
}

/*
 * Iterate over a jsonb, and apply a specified JsonTransformStringValuesAction
 * to every string value or element. Any necessary context for a
 * JsonTransformStringValuesAction can be passed in the action_state variable.
 * Function returns a copy of an original jsonb object with transformed values.
 */
Jsonb *
transform_jsonb_string_values(Jsonb *jsonb, void *action_state,
  JsonTransformStringValuesAction transform_action)
{
  JsonbValue v, *res = NULL;
  JsonbIteratorToken type;
  JsonbParseState *st = NULL;

  JsonbIterator *it = JsonbIteratorInit(&jsonb->root);
  bool is_scalar = it->isScalar;
  while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    if ((type == WJB_VALUE || type == WJB_ELEM) && v.type == jbvString)
    {
      text *out = transform_action(action_state, v.val.string.val,
        v.val.string.len);
      v.val.string.val = VARDATA_ANY(out);
      v.val.string.len = VARSIZE_ANY_EXHDR(out);
      res = pushJsonbValue(&st, type, type < WJB_BEGIN_ARRAY ? &v : NULL);
    }
    else
    {
      res = pushJsonbValue(&st, type, (type == WJB_KEY || type == WJB_VALUE ||
        type == WJB_ELEM) ? &v : NULL);
    }
  }

  if (res->type == jbvArray)
    res->val.array.rawScalar = is_scalar;
  return JsonbValueToJsonb(res);
}

/*
 * Iterate over a json, and apply a specified JsonTransformStringValuesAction
 * to every string value or element. Any necessary context for a
 * JsonTransformStringValuesAction can be passed in the action_state variable.
 * Function returns a StringInfo, which is a copy of an original json with
 * transformed values.
 */
text *
transform_json_string_values(text *js, void *action_state,
  JsonTransformStringValuesAction transform_action)
{
  JsonLexContext lex;
  TransformJsonStringValuesState *state =
    palloc0(sizeof(TransformJsonStringValuesState));
  state->lex = makeJsonLexContext(&lex, js, true);
  state->strval = makeStringInfo();
  state->action = transform_action;
  state->action_state = action_state;

  JsonSemAction *sem = palloc0(sizeof(JsonSemAction));
  sem->semstate = state;
  sem->object_start = transform_string_values_object_start;
  sem->object_end = transform_string_values_object_end;
  sem->array_start = transform_string_values_array_start;
  sem->array_end = transform_string_values_array_end;
  sem->scalar = transform_string_values_scalar;
  sem->array_element_start = transform_string_values_array_element_start;
  sem->object_field_start = transform_string_values_object_field_start;

  pg_parse_json_or_ereport(&lex, sem);
  freeJsonLexContext(&lex);
  pfree(sem);

  return pg_cstring_to_text_with_len(state->strval->data, state->strval->len);
}

/*
 * Set of auxiliary functions for transform_json_string_values to invoke a
 * specified JsonTransformStringValuesAction for all values and left everything
 * else untouched.
 */
static JsonParseErrorType
transform_string_values_object_start(void *state)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  appendStringInfoCharMacro(_state->strval, '{');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_object_end(void *state)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  appendStringInfoCharMacro(_state->strval, '}');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_array_start(void *state)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  appendStringInfoCharMacro(_state->strval, '[');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_array_end(void *state)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  appendStringInfoCharMacro(_state->strval, ']');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_object_field_start(void *state, char *fname, bool isnull UNUSED)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  if (_state->strval->data[_state->strval->len - 1] != '{')
    appendStringInfoCharMacro(_state->strval, ',');

  /*
   * Unfortunately we don't have the quoted and escaped string any more, so
   * we have to re-escape it.
   */
  escape_json(_state->strval, fname);
  appendStringInfoCharMacro(_state->strval, ':');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_array_element_start(void *state, bool isnull UNUSED)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  if (_state->strval->data[_state->strval->len - 1] != '[')
    appendStringInfoCharMacro(_state->strval, ',');
  return JSON_SUCCESS;
}

static JsonParseErrorType
transform_string_values_scalar(void *state, char *token, JsonTokenType tokentype)
{
  TransformJsonStringValuesState *_state = (TransformJsonStringValuesState *) state;
  if (tokentype == JSON_TOKEN_STRING)
  {
    text *out = _state->action(_state->action_state, token, strlen(token));
    escape_json_text(_state->strval, out);
  }
  else
    appendStringInfoString(_state->strval, token);

  return JSON_SUCCESS;
}

JsonTokenType
json_get_first_token(text *js, bool throw_error)
{
  JsonLexContext lex;
  makeJsonLexContext(&lex, js, false);

  /* Lex exactly one token from the input and check its type. */
  JsonParseErrorType result = json_lex(&lex);
  if (result == JSON_SUCCESS)
    return lex.token_type;

  if (throw_error)
    json_errsave_error(result, &lex, NULL);
  return JSON_TOKEN_INVALID;  /* invalid json */
}

/*****************************************************************************/

