/*-------------------------------------------------------------------------
 *
 * jsonb.c
 *    I/O routines for jsonb type
 *
 * Copyright (c) 2014-2025, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/jsonb.c
 *
 *-------------------------------------------------------------------------
 */

/* C */
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <stdlib.h>
/* PostgreSQL */
#include <postgres.h>
#include "catalog/pg_type.h"
#include "common/hashfn.h"
#include "common/int.h"
#include "common/jsonapi.h"
#include "nodes/nodes.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/json.h"
#include "utils/jsonb.h"
#include "utils/jsonfuncs.h"
#include "utils/varlena.h" /* For DatumGetTextP */

#include <pgtypes.h>
#include "../../meos/include/meos_error.h"

extern void escape_json_with_len(StringInfo buf, const char *str, int len);

// #include "postgres.h"
// #include "access/htup_details.h"
// #include "catalog/pg_proc.h"
// #include "catalog/pg_type.h"
// #include "funcapi.h"
// #include "libpq/pqformat.h"
// #include "miscadmin.h"
// #include "utils/builtins.h"
// #include "utils/json.h"
// #include "utils/jsonb.h"
// #include "utils/jsonfuncs.h"
// #include "utils/lsyscache.h"
// #include "utils/typcache.h"

static inline Jsonb *jsonb_from_cstring(char *js, int len, bool unique_keys,
  Node *escontext);
static bool checkStringLen(size_t len, Node *escontext);
static JsonParseErrorType jsonb_in_object_start(void *pstate);
static JsonParseErrorType jsonb_in_object_end(void *pstate);
static JsonParseErrorType jsonb_in_array_start(void *pstate);
static JsonParseErrorType jsonb_in_array_end(void *pstate);
static JsonParseErrorType jsonb_in_object_field_start(void *pstate,
  char *fname, bool isnull);
static void jsonb_put_escaped_value(StringInfo out, JsonbValue *scalarVal);
static JsonParseErrorType jsonb_in_scalar(void *pstate, char *token,
  JsonTokenType tokentype);
// static void composite_to_jsonb(Datum composite, JsonbInState *result);
// static void array_dim_to_jsonb(JsonbInState *result, int dim, int ndims,
  // int *dims, const Datum *vals, const bool *nulls, int *valcount,
  // JsonTypeCategory tcategory, Oid outfuncoid);
// static void array_to_jsonb_internal(Datum array, JsonbInState *result);
// static void datum_to_jsonb_internal(Datum val, bool is_null,
  // JsonbInState *result, JsonTypeCategory tcategory, Oid outfuncoid,
  // bool key_scalar);
// static void add_jsonb(Datum val, bool is_null, JsonbInState *result,
  // Oid val_type, bool key_scalar);
// static JsonbParseState *clone_parse_state(JsonbParseState *state);
static char *JsonbToCStringWorker(StringInfo out, JsonbContainer *in,
  int estimated_len, bool indent);
static void add_indent(StringInfo out, bool indent, int level);

extern int GetDatabaseEncoding(void);

/*****************************************************************************/

/**
 * @ingroup meos_json_base_inout
 * @brief Return a JSONB value from its string representation
 * @note Derived from PostgreSQL function @p jsonb_in()
 */
#if MEOS
Jsonb *
jsonb_in(const char *str)
{
  return jsonb_from_cstring((char *) str, strlen(str), false, NULL);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_in(const char *str)
{
  return jsonb_from_cstring((char *) str, strlen(str), false, NULL);
}

/**
 * @ingroup meos_json_base_inout
 * @brief Return the string representation if a JSONB value
 * @note Derived from PostgreSQL function @p jsonb_out()
 */
#if MEOS
char *
jsonb_out(const Jsonb *jb)
{
  return pg_jsonb_out(jb);
}
#endif /* MEOS */
char *
pg_jsonb_out(const Jsonb *jb)
{
  StringInfo out = makeStringInfo(); // MEOS
  char *str = JsonbToCString(out, &((Jsonb *)jb)->root, VARSIZE(jb));
  char *result = pstrdup(str);
  destroyStringInfo(out); 
  // pfree(str);
  return (void *) result;
}

/**
 * @ingroup meos_json_base_inout
 * @brief Return a JSONB value from its text representation
 * @note Derived from PostgreSQL function @p jsonb_from_text()
 */
#if MEOS
Jsonb *
jsonb_from_text(const text *txt, bool unique_keys)
{
  return jsonb_from_cstring(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    unique_keys, NULL);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_from_text(const text *txt, bool unique_keys)
{
  return jsonb_from_cstring(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt),
    unique_keys, NULL);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Return the text representation of a JSONB value
 */
#if MEOS
text *
jsonb_to_text(const Jsonb *jb)
{
  return pg_jsonb_to_text(jb);
}
#endif /* MEOS */
text *
pg_jsonb_to_text(const Jsonb *jb)
{
  StringInfo out = makeStringInfo();
  char *str = JsonbToCString(out, (JsonbContainer *) &jb->root, VARSIZE(jb));
  text *result = pg_cstring_to_text_with_len(str, out->len);
  destroyStringInfo(out); pfree(str);
  return result;
}

/*****************************************************************************/

/*
 * Get the type name of a jsonb container.
 */
static const char *
JsonbContainerTypeName(JsonbContainer *jbc)
{
  JsonbValue scalar;
  if (JsonbExtractScalar(jbc, &scalar))
    return JsonbTypeName(&scalar);
  else if (JsonContainerIsArray(jbc))
    return "array";
  else if (JsonContainerIsObject(jbc))
    return "object";
  else
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "invalid jsonb container type: 0x%08x", jbc->header);
    return "unknown";
  }
}

/*
 * Get the type name of a JSONB value
 */
const char *
JsonbTypeName(JsonbValue *val)
{
  switch (val->type)
  {
    case jbvBinary:
      return JsonbContainerTypeName(val->val.binary.data);
    case jbvObject:
      return "object";
    case jbvArray:
      return "array";
    case jbvNumeric:
      return "number";
    case jbvString:
      return "string";
    case jbvBool:
      return "boolean";
    case jbvNull:
      return "null";
    case jbvDatetime:
      switch (val->val.datetime.typid)
      {
        case DATEOID:
          return "date";
        case TIMEOID:
          return "time without time zone";
        case TIMETZOID:
          return "time with time zone";
        case TIMESTAMPOID:
          return "timestamp without time zone";
        case TIMESTAMPTZOID:
          return "timestamp with time zone";
        default:
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "unrecognized jsonb value datetime type: %d",
             val->val.datetime.typid);
      }
      return "unknown";
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized jsonb value type: %d", val->type);
      return "unknown";
  }
}

/*
 * SQL function jsonb_typeof(jsonb) -> text
 *
 * This function is here because the analog json function is in json.c, since
 * it uses the json parser internals not exposed elsewhere.
 */
text *
jsonb_typeof_internal(Jsonb *in)
{
  const char *result = JsonbContainerTypeName(&in->root);
  return pg_cstring_to_text(result);
}

/*
 * jsonb_from_cstring
 *
 * Turns json string into a jsonb Datum.
 *
 * Uses the json parser (with hooks) to construct a jsonb.
 *
 * If escontext points to an ErrorSaveContext, errors are reported there
 * instead of being thrown.
 */
static inline Jsonb *
jsonb_from_cstring(char *js, int len, bool unique_keys, Node *escontext)
{
  JsonbInState state;
  memset(&state, 0, sizeof(state));
  state.unique_keys = unique_keys;
  state.escontext = escontext;

  JsonSemAction sem;
  memset(&sem, 0, sizeof(sem));
  sem.semstate = &state;
  sem.object_start = jsonb_in_object_start;
  sem.array_start = jsonb_in_array_start;
  sem.object_end = jsonb_in_object_end;
  sem.array_end = jsonb_in_array_end;
  sem.scalar = jsonb_in_scalar;
  sem.object_field_start = jsonb_in_object_field_start;

  JsonLexContext lex;
  makeJsonLexContextCstringLen(&lex, js, len, GetDatabaseEncoding(), true);

  if (! pg_parse_json_or_errsave(&lex, &sem, escontext))
    return NULL;

  /* after parsing, the item member has the composed jsonb structure */
  Jsonb *result = JsonbValueToJsonb(state.res);
  /* Free accumulated list of elements to free after a JSON parsing */
  json_reset_tofree();
  freeJsonLexContext(&lex);
  return result;
}

static bool
checkStringLen(size_t len, Node *escontext UNUSED)
{
  if (len > JENTRY_OFFLENMASK)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "string too long to represent as jsonb string");
    return false;
  }
  return true;
}

static JsonParseErrorType
jsonb_in_object_start(void *pstate)
{
  JsonbInState *_state = (JsonbInState *) pstate;
  _state->res = pushJsonbValue(&_state->parseState, WJB_BEGIN_OBJECT, NULL);
  _state->parseState->unique_keys = _state->unique_keys;
  return JSON_SUCCESS;
}

static JsonParseErrorType
jsonb_in_object_end(void *pstate)
{
  JsonbInState *_state = (JsonbInState *) pstate;
  _state->res = pushJsonbValue(&_state->parseState, WJB_END_OBJECT, NULL);
  return JSON_SUCCESS;
}

static JsonParseErrorType
jsonb_in_array_start(void *pstate)
{
  JsonbInState *_state = (JsonbInState *) pstate;
  _state->res = pushJsonbValue(&_state->parseState, WJB_BEGIN_ARRAY, NULL);
  return JSON_SUCCESS;
}

static JsonParseErrorType
jsonb_in_array_end(void *pstate)
{
  JsonbInState *_state = (JsonbInState *) pstate;
  _state->res = pushJsonbValue(&_state->parseState, WJB_END_ARRAY, NULL);
  return JSON_SUCCESS;
}

static JsonParseErrorType
jsonb_in_object_field_start(void *pstate, char *fname, bool isnull UNUSED)
{
  Assert(fname != NULL);
  JsonbInState *_state = (JsonbInState *) pstate;
  JsonbValue v;
  v.type = jbvString;
  v.val.string.len = strlen(fname);
  if (!checkStringLen(v.val.string.len, _state->escontext))
    return JSON_SEM_ACTION_FAILED;
  v.val.string.val = fname;
  _state->res = pushJsonbValue(&_state->parseState, WJB_KEY, &v);
  return JSON_SUCCESS;
}

static void
jsonb_put_escaped_value(StringInfo out, JsonbValue *scalarVal)
{
  switch (scalarVal->type)
  {
    case jbvNull:
      appendBinaryStringInfo(out, "null", 4);
      break;
    case jbvString:
      escape_json_with_len(out, scalarVal->val.string.val, scalarVal->val.string.len);
      break;
    case jbvNumeric:
    {
      char *str = pg_numeric_out(scalarVal->val.numeric);
      appendStringInfoString(out, str);
      pfree(str);
      break;
    }
    case jbvBool:
      if (scalarVal->val.boolean)
        appendBinaryStringInfo(out, "true", 4);
      else
        appendBinaryStringInfo(out, "false", 5);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unknown jsonb scalar type");
  }
}

/*
 * For jsonb we always want the de-escaped value - that's what's in token
 */
static JsonParseErrorType
jsonb_in_scalar(void *pstate, char *token, JsonTokenType tokentype)
{
  JsonbInState *_state = (JsonbInState *) pstate;
  JsonbValue v;
  Numeric num = NULL;
  switch (tokentype)
  {
    case JSON_TOKEN_STRING:
      Assert(token != NULL);
      v.type = jbvString;
      v.val.string.len = strlen(token);
      if (! checkStringLen(v.val.string.len, _state->escontext))
        return JSON_SEM_ACTION_FAILED;
      v.val.string.val = token;
      break;
    case JSON_TOKEN_NUMBER:
      /*
       * No need to check size of numeric values, because maximum
       * numeric size is well below the JsonbValue restriction
       */
      Assert(token != NULL);
      v.type = jbvNumeric;
      num = pg_numeric_in(token, -1);
      if (! num)
        return JSON_SEM_ACTION_FAILED;
      v.val.numeric = num;
      /* Add num to the values that need to be freed */
      // json_add_tofree((void *) num);
      break;
    case JSON_TOKEN_TRUE:
      v.type = jbvBool;
      v.val.boolean = true;
      break;
    case JSON_TOKEN_FALSE:
      v.type = jbvBool;
      v.val.boolean = false;
      break;
    case JSON_TOKEN_NULL:
      v.type = jbvNull;
      break;
    default:
      /* should not be possible */
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "invalid json token type");
      break;
  }

  if (! _state->parseState)
  {
    /* single scalar */
    JsonbValue va;
    va.type = jbvArray;
    va.val.array.rawScalar = true;
    va.val.array.nElems = 1;

    _state->res = pushJsonbValue(&_state->parseState, WJB_BEGIN_ARRAY, &va);
    _state->res = pushJsonbValue(&_state->parseState, WJB_ELEM, &v);
    _state->res = pushJsonbValue(&_state->parseState, WJB_END_ARRAY, NULL);
  }
  else
  {
    JsonbValue *o = &_state->parseState->contVal;
    switch (o->type)
    {
      case jbvArray:
        _state->res = pushJsonbValue(&_state->parseState, WJB_ELEM, &v);
        break;
      case jbvObject:
        _state->res = pushJsonbValue(&_state->parseState, WJB_VALUE, &v);
        break;
      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unexpected parent of nested structure");
        return JSON_INVALID_TOKEN;
    }
  }

  return JSON_SUCCESS;
}

/*
 * JsonbToCString
 *     Converts jsonb value to a C-string.
 *
 * If 'out' argument is non-null, the resulting C-string is stored inside the
 * StringBuffer.  The resulting string is always returned.
 *
 * A typical case for passing the StringInfo in rather than NULL is where the
 * caller wants access to the len attribute without having to call strlen, e.g.
 * if they are converting it to a text* object.
 */
char *
JsonbToCString(StringInfo out, JsonbContainer *in, int estimated_len)
{
  return JsonbToCStringWorker(out, in, estimated_len, false);
}

/*
 * same thing but with indentation turned on
 */
char *
JsonbToCStringIndent(StringInfo out, JsonbContainer *in, int estimated_len)
{
  return JsonbToCStringWorker(out, in, estimated_len, true);
}

/*
 * common worker for above two functions
 */
static char *
JsonbToCStringWorker(StringInfo out, JsonbContainer *in, int estimated_len,
  bool indent)
{
  bool first = true;
  JsonbValue v;
  JsonbIteratorToken type = WJB_DONE;
  int level = 0;
  bool redo_switch = false;

  /* If we are indenting, don't add a space after a comma */
  int ispaces = indent ? 1 : 2;

  /*
   * Don't indent the very first item. This gets set to the indent flag at
   * the bottom of the loop.
   */
  bool use_indent = false;
  bool raw_scalar = false;
  bool last_was_key = false;
  bool tofree = false;

  if (! out)
  {
    out = makeStringInfo();
    tofree = false;
  }

  enlargeStringInfo(out, (estimated_len >= 0) ? estimated_len : 64);
  JsonbIterator *it = JsonbIteratorInit(in);
  while (redo_switch ||
       ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE))
  {
    redo_switch = false;
    switch (type)
    {
      case WJB_BEGIN_ARRAY:
        if (! first)
          appendBinaryStringInfo(out, ", ", ispaces);

        if (!v.val.array.rawScalar)
        {
          add_indent(out, use_indent && !last_was_key, level);
          appendStringInfoCharMacro(out, '[');
        }
        else
          raw_scalar = true;

        first = true;
        level++;
        break;
      case WJB_BEGIN_OBJECT:
        if (! first)
          appendBinaryStringInfo(out, ", ", ispaces);

        add_indent(out, use_indent && !last_was_key, level);
        appendStringInfoCharMacro(out, '{');

        first = true;
        level++;
        break;
      case WJB_KEY:
        if (! first)
          appendBinaryStringInfo(out, ", ", ispaces);
        first = true;

        add_indent(out, use_indent, level);

        /* json rules guarantee this is a string */
        jsonb_put_escaped_value(out, &v);
        appendBinaryStringInfo(out, ": ", 2);

        type = JsonbIteratorNext(&it, &v, false);
        if (type == WJB_VALUE)
        {
          first = false;
          jsonb_put_escaped_value(out, &v);
        }
        else
        {
          Assert(type == WJB_BEGIN_OBJECT || type == WJB_BEGIN_ARRAY);

          /*
           * We need to rerun the current switch() since we need to
           * output the object which we just got from the iterator
           * before calling the iterator again.
           */
          redo_switch = true;
        }
        break;
      case WJB_ELEM:
        if (! first)
          appendBinaryStringInfo(out, ", ", ispaces);
        first = false;

        if (!raw_scalar)
          add_indent(out, use_indent, level);
        jsonb_put_escaped_value(out, &v);
        break;
      case WJB_END_ARRAY:
        level--;
        if (!raw_scalar)
        {
          add_indent(out, use_indent, level);
          appendStringInfoCharMacro(out, ']');
        }
        first = false;
        break;
      case WJB_END_OBJECT:
        level--;
        add_indent(out, use_indent, level);
        appendStringInfoCharMacro(out, '}');
        first = false;
        break;
      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "unknown jsonb iterator token type");
        return NULL;
    }
    use_indent = indent;
    last_was_key = redo_switch;
  }

  Assert(level == 0);
  if (tofree)
    destroyStringInfo(out);
  return out->data;
}

static void
add_indent(StringInfo out, bool indent, int level)
{
  if (indent)
  {
    appendStringInfoCharMacro(out, '\n');
    appendStringInfoSpaces(out, level * 4);
  }
}

/*****************************************************************************
 * Constructors 
 *****************************************************************************/

/**
 * @ingroup meos_json_base_constructor
 * @brief Return a JSONB value constructed from a text array of alternating
 * keys and values
 * @param[in] keys_vals Array of alternating keys and values 
 * @param[in] count Number of elements in the input array 
 * @note Derived from PostgreSQL function @p jsonb_object()
 */
#if MEOS
Jsonb *
jsonb_make(text **keys_vals, int count)
{
  return pg_jsonb_make(keys_vals, count);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_make(text **keys_vals, int count)
{
  if (count % 2)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "number of elements of the array must be an even value");
    return NULL;
  }

  JsonbInState state;
  memset(&state, 0, sizeof(JsonbInState));
  (void) pushJsonbValue(&state.parseState, WJB_BEGIN_OBJECT, NULL);

  /* Transform the keys and values into strings */
  char **keys_vals_str = palloc0(sizeof(char *) * count);
  for (int i = 0; i < count; ++i)
  {
    if (keys_vals[i])
      keys_vals_str[i] = pg_text_to_cstring(keys_vals[i]);
  }
  /* Iterate for half of the count */
  int count1 = count / 2;
  for (int i = 0; i < count1; ++i)
  {
    if (! keys_vals_str[i * 2])
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "null value not allowed for object key");
      return NULL;
    }

    char *str = keys_vals_str[i * 2];
    int len = strlen(str);
    JsonbValue v;
    v.type = jbvString;
    v.val.string.len = len;
    v.val.string.val = str;
    (void) pushJsonbValue(&state.parseState, WJB_KEY, &v);

    if (! keys_vals_str[i * 2 + 1])
    {
      v.type = jbvNull;
    }
    else
    {
      str = keys_vals_str[i * 2 + 1];
      len = strlen(str);
      v.type = jbvString;
      v.val.string.len = len;
      v.val.string.val = str;
    }
    (void) pushJsonbValue(&state.parseState, WJB_VALUE, &v);
  }

  state.res = pushJsonbValue(&state.parseState, WJB_END_OBJECT, NULL);
  Jsonb *result = JsonbValueToJsonb(state.res);

  /* Clean up and return */
  for (int i = 0; i < count; ++i)
  {
    if (keys_vals[i])
      pfree(keys_vals_str[i]);
  }
  pfree(keys_vals_str);
  return result;
}

/**
 * @ingroup meos_json_base_constructor
 * @brief Return a JSONB value constructed from separate key and value text
 * arrays
 * @param[in] keys Keys
 * @param[in] values Values
 * @param[in] count Number of elements in the input arrays
 * @note Derived from PostgreSQL function @p jsonb_object_two_arg()
 */
#if MEOS
Jsonb *
jsonb_make_two_arg(text **keys, text **values, int count)
{
  return pg_jsonb_make_two_arg(keys, values, count);
}
#endif /* MEOS */
Jsonb *
pg_jsonb_make_two_arg(text **keys, text **values, int count)
{
  JsonbInState state;
  memset(&state, 0, sizeof(JsonbInState));
  (void) pushJsonbValue(&state.parseState, WJB_BEGIN_OBJECT, NULL);
  char **keys_str = palloc(sizeof(char *) * count);
  /* Initialized to 0 since some values may be null */
  char **values_str = palloc0(sizeof(char *) * count);
  /* Iterate to create the json value */
  for (int i = 0; i < count; ++i)
  {
    if (! keys[i])
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "null value not allowed for object key");
      return NULL;
    }
    keys_str[i] = pg_text_to_cstring(keys[i]);
    if (values[i])
      values_str[i] = pg_text_to_cstring(values[i]);
  }
  /* Convert the keys and the values into strings */
  for (int i = 0; i < count; ++i)
  {
    char *str = keys_str[i];
    int len = strlen(str);
    JsonbValue v;
    v.type = jbvString;
    v.val.string.len = len;
    v.val.string.val = str;
    (void) pushJsonbValue(&state.parseState, WJB_KEY, &v);
    if (! values[i])
    {
      v.type = jbvNull;
    }
    else
    {
      str = values_str[i];
      len = strlen(str);
      v.type = jbvString;
      v.val.string.len = len;
      v.val.string.val = str;
    }
    (void) pushJsonbValue(&state.parseState, WJB_VALUE, &v);
  }

  state.res = pushJsonbValue(&state.parseState, WJB_END_OBJECT, NULL);
  Jsonb *result = JsonbValueToJsonb(state.res);
  for (int i = 0; i < count; ++i)
    pfree(keys_str[i]);
  pfree(keys_str);
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into a C string
 * @param[in] jb JSONB object
 */
char *
jsonb_to_cstring(const Jsonb *jb)
{
  assert(jb);
  return JsonbToCString(NULL, &((Jsonb *) jb)->root, VARSIZE(jb));
}

/*
 * Extract scalar value from raw-scalar pseudo-array jsonb.
 */
bool
JsonbExtractScalar(JsonbContainer *jbc, JsonbValue *res)
{
  JsonbIterator *it;
  JsonbIteratorToken tok PG_USED_FOR_ASSERTS_ONLY;
  JsonbValue  tmp;

  if (! JsonContainerIsArray(jbc) || ! JsonContainerIsScalar(jbc))
  {
    /* inform caller about actual type of container */
    res->type = (JsonContainerIsArray(jbc)) ? jbvArray : jbvObject;
    return false;
  }

  /*
   * A root scalar is stored as an array of one element, so we get the array
   * and then its first (and only) member.
   */
  it = JsonbIteratorInit(jbc);

  tok = JsonbIteratorNext(&it, &tmp, true);
  Assert(tok == WJB_BEGIN_ARRAY);
  Assert(tmp.val.array.nElems == 1 && tmp.val.array.rawScalar);

  tok = JsonbIteratorNext(&it, res, true);
  Assert(tok == WJB_ELEM);
  Assert(IsAJsonbScalar(res));

  tok = JsonbIteratorNext(&it, &tmp, true);
  Assert(tok == WJB_END_ARRAY);

  tok = JsonbIteratorNext(&it, &tmp, true);
  Assert(tok == WJB_DONE);

  return true;
}

/*
 * Emit correct, translatable cast error message
 */
static void
cannotCastJsonbValue(enum jbvType type, const char *sqltype)
{
  static const struct
  {
    enum jbvType type;
    const char *msg;
  } messages[] =
  {
    {jbvNull, gettext_noop("cannot cast jsonb null to type %s")},
    {jbvString, gettext_noop("cannot cast jsonb string to type %s")},
    {jbvNumeric, gettext_noop("cannot cast jsonb numeric to type %s")},
    {jbvBool, gettext_noop("cannot cast jsonb boolean to type %s")},
    {jbvArray, gettext_noop("cannot cast jsonb array to type %s")},
    {jbvObject, gettext_noop("cannot cast jsonb object to type %s")},
    {jbvBinary, gettext_noop("cannot cast jsonb array or object to type %s")}
  };

  for (int i = 0; i < (int) lengthof(messages); i++)
    if (messages[i].type == type)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        messages[i].msg, sqltype);
    }

  /* should be unreachable */
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "unknown jsonb type: %d", (int) type);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into a boolean
 * @param[in] jb JSONB object
 */
bool
jsonb_to_bool(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "boolean");
  if (v.type == jbvNull)
    return NULL;
  if (v.type != jbvBool)
    cannotCastJsonbValue(v.type, "boolean");
  return v.val.boolean;
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into a numeric
 * @param[in] jb JSONB object
 */
Numeric
jsonb_to_numeric(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "numeric");
  if (v.type == jbvNull)
    return NULL;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "numeric");
  /*
   * v.val.numeric points into jsonb body, so we need to make a copy to
   * return
   */
  Numeric retValue = numeric_copy(v.val.numeric);
  return retValue;
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into an int16
 * @param[in] jb JSONB object
 */
int16
jsonb_to_int16(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "smallint");
  if (v.type == jbvNull)
    return SHRT_MAX;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "smallint");
  return numeric_to_int16(v.val.numeric);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into an int32
 * @param[in] jb JSONB object
 */
int32
jsonb_to_int32(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "integer");
  if (v.type == jbvNull)
    return INT_MAX;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "integer");
  return numeric_to_int32(v.val.numeric);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into an int64
 * @param[in] jb JSONB object
 */
int64
jsonb_to_int64(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "bigint");
  if (v.type == jbvNull)
    return LONG_MAX;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "bigint");
  return numeric_to_int64(v.val.numeric);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into a float4
 * @param[in] jb JSONB object
 */
float4
jsonb_to_float4(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "real");
  if (v.type == jbvNull)
    return FLT_MAX;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "real");
  return numeric_to_float4(v.val.numeric);
}

/**
 * @ingroup meos_json_base_conversion
 * @brief Convert a JSONB value into a float8
 * @param[in] jb JSONB object
 */
float8
jsonb_to_float8(const Jsonb *jb)
{
  JsonbValue v;
  if (! JsonbExtractScalar(&((Jsonb *) jb)->root, &v))
    cannotCastJsonbValue(v.type, "double precision");
  if (v.type == jbvNull)
    return DBL_MAX;
  if (v.type != jbvNumeric)
    cannotCastJsonbValue(v.type, "double precision");
  return numeric_to_float8(v.val.numeric);
}

/*
 * Convert jsonb to a C-string stripping quotes from scalar strings.
 */
char *
JsonbUnquote(Jsonb *jb)
{
  if (JB_ROOT_IS_SCALAR(jb))
  {
    JsonbValue v;
    (void) JsonbExtractScalar(&jb->root, &v);
    if (v.type == jbvString)
      return pnstrdup(v.val.string.val, v.val.string.len);
    else if (v.type == jbvBool)
      return pstrdup(v.val.boolean ? "true" : "false");
    else if (v.type == jbvNumeric)
      return pg_numeric_out(v.val.numeric);
    else if (v.type == jbvNull)
      return pstrdup("null");
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unrecognized jsonb value type %d", v.type);
      return NULL;
    }
  }
  else
  {
    StringInfo out = makeStringInfo(); // MEOS
    char *str = JsonbToCString(out, (JsonbContainer *) &jb->root, VARSIZE(jb));
    char *result = pstrdup(str);
    destroyStringInfo(out); pfree(str);
    return (void *) result;
  }
  return JsonbToCString(NULL, &jb->root, VARSIZE(jb));
}

/*****************************************************************************/

