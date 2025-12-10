/*-------------------------------------------------------------------------
 *
 * json.c
 *    JSON data type support.
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/json.c
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
#include "miscadmin.h"
#include "catalog/pg_type.h"
#include "common/hashfn.h"
#include "common/int.h"
#include "common/jsonapi.h"
#include "port/simd.h"
#include "utils/builtins.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include "utils/timestamp.h"
#include "utils/hsearch.h"
#include "utils/json.h"
#include "utils/jsonb.h"
#include "utils/jsonb.h"
#include "utils/jsonfuncs.h"
#include "utils/varlena.h" /* For DatumGetTextP */

#include <pgtypes.h>

// #include "postgres.h"
// #include "catalog/pg_proc.h"
// #include "catalog/pg_type.h"
// #include "common/hashfn.h"
// #include "funcapi.h"
// #include "libpq/pqformat.h"
// #include "miscadmin.h"
// #include "port/simd.h"
// #include "utils/array.h"
// #include "utils/builtins.h"
// #include "utils/date.h"
// #include "utils/datetime.h"
// #include "utils/fmgroids.h"
// #include "utils/json.h"
// #include "utils/jsonfuncs.h"
// #include "utils/lsyscache.h"
// #include "utils/typcache.h"

/*
 * Support for fast key uniqueness checking.
 *
 * We maintain a hash table of used keys in JSON objects for fast detection
 * of duplicates.
 */

/* Hash entry for JsonUniqueCheckState */
typedef struct JsonUniqueHashEntry
{
  const char *key;
  int      key_len;
  int      object_id;
} JsonUniqueHashEntry;

/* Common context for key uniqueness check */
typedef struct HTAB *JsonUniqueCheckState;  /* hash table for key names */

/* Stack element for key uniqueness check during JSON parsing */
typedef struct JsonUniqueStackEntry
{
  struct JsonUniqueStackEntry *parent;
  int object_id;
} JsonUniqueStackEntry;

/* Context struct for key uniqueness check during JSON parsing */
typedef struct JsonUniqueParsingState
{
  JsonLexContext *lex;
  JsonUniqueCheckState check;
  JsonUniqueStackEntry *stack;
  int id_counter;
  bool unique;
} JsonUniqueParsingState;

/* Context struct for key uniqueness check during JSON building */
typedef struct JsonUniqueBuilderState
{
  JsonUniqueCheckState check; /* unique check */
  StringInfoData skipped_keys;  /* skipped keys with NULL values */
  // MemoryContext mcxt;      /* context for saving skipped keys */
} JsonUniqueBuilderState;

/* State struct for JSON aggregation */
typedef struct JsonAggState
{
  StringInfo str;
  JsonTypeCategory key_category;
  Oid key_output_func;
  JsonTypeCategory val_category;
  Oid val_output_func;
  JsonUniqueBuilderState unique_check;
} JsonAggState;

// static void composite_to_json(Datum composite, StringInfo result,
  // bool use_line_feeds);
// static void array_dim_to_json(StringInfo result, int dim, int ndims, int *dims,
  // Datum *vals, bool *nulls, int *valcount, JsonTypeCategory tcategory,
  // Oid outfuncoid, bool use_line_feeds);
// static void array_to_json_internal(Datum array, StringInfo result,
  // bool use_line_feeds);
// static void add_json(Datum val, bool is_null, StringInfo result,
  // Oid val_type, bool key_scalar);
// static text *catenate_stringinfo_string(StringInfo buffer, const char *addon);

/**
 * @ingroup meos_json_base_inout
 * @brief Return a JSON value from its string representation
 * @param[in] str String
 * @note Derived from PostgreSQL function @p json_in()
 */
#if MEOS
text *
json_in(const char *str)
{
  return pg_json_in(str);
}
#endif /* MEOS */
text *
pg_json_in(const char *str)
{
  text *result = cstring_to_text(str);

  /* validate it */
  JsonLexContext lex;
  makeJsonLexContext(&lex, result, false);
  if (! pg_parse_json_or_errsave(&lex, &nullSemAction, NULL))
    return NULL;

  /* Internal representation is the same as text */
  return result;
}

/**
 * @ingroup meos_json_base_inout
 * @brief Return the string representation of a JSON value
 * @param[in] json JSON value
 * @note Derived from PostgreSQL function @p json_out()
 */
#if MEOS
char *
json_out(const text *json)
{
  return text_to_cstring(json);
}
#endif /* MEOS */
char *
pg_json_out(const text *json)
{
  return text_to_cstring(json);
}

/*
 * Encode 'value' of datetime type 'typid' into JSON string in ISO format using
 * optionally preallocated buffer 'buf'.  Optional 'tzp' determines time-zone
 * offset (in seconds) in which we want to show timestamptz.
 */
char *
JsonEncodeDateTime(char *buf, Datum value, Oid typid, const int *tzp)
{
  if (!buf)
    buf = palloc(MAXDATELEN + 1);

  switch (typid)
  {
    case DATEOID:
      {
        struct pg_tm tm;
        DateADT date = DatumGetDateADT(value);
        /* Same as date_out(), but forcing DateStyle */
        if (DATE_NOT_FINITE(date))
          EncodeSpecialDate(date, buf);
        else
        {
          j2date(date + POSTGRES_EPOCH_JDATE, &(tm.tm_year), &(tm.tm_mon),
            &(tm.tm_mday));
          EncodeDateOnly(&tm, USE_XSD_DATES, buf);
        }
      }
      break;
    case TIMEOID:
      {
        TimeADT time = DatumGetTimeADT(value);
        struct pg_tm tt, *tm = &tt;
        fsec_t fsec;
        /* Same as time_out(), but forcing DateStyle */
        time2tm(time, tm, &fsec);
        EncodeTimeOnly(tm, fsec, false, 0, USE_XSD_DATES, buf);
      }
      break;
    case TIMETZOID:
      {
        TimeTzADT  *time = DatumGetTimeTzADTP(value);
        struct pg_tm tt, *tm = &tt;
        fsec_t fsec;
        int tz;
        /* Same as timetz_out(), but forcing DateStyle */
        timetz2tm(time, tm, &fsec, &tz);
        EncodeTimeOnly(tm, fsec, true, tz, USE_XSD_DATES, buf);
      }
      break;
    case TIMESTAMPOID:
      {
        struct pg_tm tm;
        fsec_t fsec;
        Timestamp timestamp = DatumGetTimestamp(value);
        /* Same as timestamp_out(), but forcing DateStyle */
        if (TIMESTAMP_NOT_FINITE(timestamp))
          EncodeSpecialTimestamp(timestamp, buf);
        else if (timestamp2tm(timestamp, NULL, &tm, &fsec, NULL, NULL) == 0)
          EncodeDateTime(&tm, fsec, false, 0, NULL, USE_XSD_DATES, buf);
        else
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "timestamp out of range");
          return NULL;
        }
      }
      break;
    case TIMESTAMPTZOID:
      {
        struct pg_tm tm;
        int tz;
        fsec_t fsec;
        const char *tzn = NULL;
        TimestampTz timestamp = DatumGetTimestampTz(value);

        /*
         * If a time zone is specified, we apply the time-zone shift,
         * convert timestamptz to pg_tm as if it were without a time
         * zone, and then use the specified time zone for converting
         * the timestamp into a string.
         */
        if (tzp)
        {
          tz = *tzp;
          timestamp -= (TimestampTz) tz * USECS_PER_SEC;
        }

        /* Same as timestamptz_out(), but forcing DateStyle */
        if (TIMESTAMP_NOT_FINITE(timestamp))
          EncodeSpecialTimestamp(timestamp, buf);
        else if (timestamp2tm(timestamp, tzp ? NULL : &tz, &tm, &fsec,
          tzp ? NULL : &tzn, NULL) == 0)
        {
          if (tzp)
            tm.tm_isdst = 1;  /* set time-zone presence flag */
          EncodeDateTime(&tm, fsec, true, tz, tzn, USE_XSD_DATES, buf);
        }
        else
        {
          meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
            "timestamp out of range");
          return NULL;
        }
      }
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unknown jsonb value datetime type oid %u", typid);
      return NULL;
  }

  return buf;
}

/* Functions implementing hash table for key uniqueness check */
static uint32
json_unique_hash(const void *key, Size keysize UNUSED)
{
  const JsonUniqueHashEntry *entry = (JsonUniqueHashEntry *) key;
  uint32 hash = hash_bytes_uint32(entry->object_id);
  hash ^= hash_bytes((const unsigned char *) entry->key, entry->key_len);
  return DatumGetUInt32(hash);
}

static int
json_unique_hash_match(const void *key1, const void *key2, Size keysize UNUSED)
{
  const JsonUniqueHashEntry *entry1 = (const JsonUniqueHashEntry *) key1;
  const JsonUniqueHashEntry *entry2 = (const JsonUniqueHashEntry *) key2;
  if (entry1->object_id != entry2->object_id)
    return entry1->object_id > entry2->object_id ? 1 : -1;
  if (entry1->key_len != entry2->key_len)
    return entry1->key_len > entry2->key_len ? 1 : -1;
  return strncmp(entry1->key, entry2->key, entry1->key_len);
}

/*
 * Uniqueness detection support.
 *
 * In order to detect uniqueness during building or parsing of a JSON
 * object, we maintain a hash table of key names already seen.
 */
static void
json_unique_check_init(JsonUniqueCheckState *cxt)
{
  HASHCTL ctl;
  memset(&ctl, 0, sizeof(ctl));
  ctl.keysize = sizeof(JsonUniqueHashEntry);
  ctl.entrysize = sizeof(JsonUniqueHashEntry);
  ctl.hash = json_unique_hash;
  ctl.match = json_unique_hash_match;
  *cxt = hash_create("json object hashtable", 32, &ctl,
    HASH_ELEM | HASH_FUNCTION | HASH_COMPARE);
  return;
}

// static void
// json_unique_builder_init(JsonUniqueBuilderState *cxt)
// {
  // json_unique_check_init(&cxt->check);
  // cxt->skipped_keys.data = NULL;
  // return;
// }

static bool
json_unique_check_key(JsonUniqueCheckState *cxt, const char *key,
  int object_id)
{
  JsonUniqueHashEntry entry;
  entry.key = key;
  entry.key_len = strlen(key);
  entry.object_id = object_id;
  bool found;
  (void) hash_search(*cxt, &entry, HASH_ENTER, &found);
  return !found;
}

// /*
 // * On-demand initialization of a throwaway StringInfo.  This is used to
 // * read a key name that we don't need to store in the output object, for
 // * duplicate key detection when the value is NULL.
 // */
// static StringInfo
// json_unique_builder_get_throwawaybuf(JsonUniqueBuilderState *cxt)
// {
  // StringInfo out = &cxt->skipped_keys;
  // if (!out->data)
    // initStringInfo(out);
  // else
    // /* Just reset the string to empty */
    // out->len = 0;
  // return out;
// }

/**
 * @ingroup meos_json_base_constructor
 * @brief Construct a JSON value from an array of alternating keys
 * and values
 * @param[in] keys_vals Array of alternating keys and vals 
 * @param[in] count Number of elements in the input array 
 * @note Derived from PostgreSQL function @p json_object()
 */
#if MEOS
text *
json_make(text **keys_vals, int count)
{
  return pg_json_make(keys_vals, count);
}
#endif /* MEOS */
text *
pg_json_make(text **keys_vals, int count)
{
  StringInfoData res;
  int count1 = count / 2;
  initStringInfo(&res);
  appendStringInfoChar(&res, '{');
  for (int i = 0; i < count1; ++i)
  {
    if (! keys_vals[i * 2])
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "null value not allowed for object key");
      return NULL;
    }

    if (i > 0)
      appendStringInfoString(&res, ", ");
    escape_json_text(&res, keys_vals[i * 2]);
    appendStringInfoString(&res, " : ");
    if (! keys_vals[i * 2 + 1])
      appendStringInfoString(&res, "null");
    else
      escape_json_text(&res, keys_vals[i * 2 + 1]);
  }
  appendStringInfoChar(&res, '}');
  text *result = cstring_to_text_with_len(res.data, res.len);
  pfree(res.data);
  return result;
}

/**
 * @ingroup meos_json_base_constructor
 * @brief Construct a JSON value from separate key and value arrays of text
 * values
 * @param[in] keys Keys
 * @param[in] values Keys
 * @param[in] count Number of elements in the input arrays
 * @note Derived from PostgreSQL function @p json_make_two_arg()
 */
#if MEOS
text *
json_make_two_arg(text **keys, text **values, int count)
{
  return pg_json_make_two_arg(keys, values, count);
}
#endif /* MEOS */
text *
pg_json_make_two_arg(text **keys, text **values, int count)
{
  StringInfoData res;
  initStringInfo(&res);
  appendStringInfoChar(&res, '{');
  for (int i = 0; i < count; ++i)
  {
    if (! keys[i])
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "null value not allowed for object key");
      return NULL;
    }

    if (i > 0)
      appendStringInfoString(&res, ", ");
    escape_json_text(&res, keys[i]);
    appendStringInfoString(&res, " : ");
    if (! values[i])
      appendStringInfoString(&res, "null");
    else
      escape_json_text(&res, values[i]);
  }

  appendStringInfoChar(&res, '}');
  text *result = cstring_to_text_with_len(res.data, res.len);
  pfree(res.data);
  return result;
}

/*
 * escape_json_char
 *    Inline helper function for escape_json* functions
 */
static pg_attribute_always_inline void
escape_json_char(StringInfo buf, char c)
{
  switch (c)
  {
    case '\b':
      appendStringInfoString(buf, "\\b");
      break;
    case '\f':
      appendStringInfoString(buf, "\\f");
      break;
    case '\n':
      appendStringInfoString(buf, "\\n");
      break;
    case '\r':
      appendStringInfoString(buf, "\\r");
      break;
    case '\t':
      appendStringInfoString(buf, "\\t");
      break;
    case '"':
      appendStringInfoString(buf, "\\\"");
      break;
    case '\\':
      appendStringInfoString(buf, "\\\\");
      break;
    default:
      if ((unsigned char) c < ' ')
        appendStringInfo(buf, "\\u%04x", (int) c);
      else
        appendStringInfoCharMacro(buf, c);
      break;
  }
}

/*
 * escape_json
 *    Produce a JSON string literal, properly escaping the NUL-terminated
 *    cstring.
 */
void
escape_json(StringInfo buf, const char *str)
{
  appendStringInfoCharMacro(buf, '"');
  for (; *str != '\0'; str++)
    escape_json_char(buf, *str);
  appendStringInfoCharMacro(buf, '"');
}

/*
 * Define the number of bytes that escape_json_with_len will look ahead in the
 * input string before flushing the input string to the destination buffer.
 * Looking ahead too far could result in cachelines being evicted that will
 * need to be reloaded in order to perform the appendBinaryStringInfo call.
 * Smaller values will result in a larger number of calls to
 * appendBinaryStringInfo and introduce additional function call overhead.
 * Values larger than the size of L1d cache will likely result in worse
 * performance.
 */
#define ESCAPE_JSON_FLUSH_AFTER 512

/*
 * escape_json_with_len
 *    Produce a JSON string literal, properly escaping the possibly not
 *    NUL-terminated characters in 'str'.  'len' defines the number of bytes
 *    from 'str' to process.
 */
void
escape_json_with_len(StringInfo buf, const char *str, int len)
{
  int vlen;
  Assert(len >= 0);

  /*
   * Since we know the minimum length we'll need to append, let's just
   * enlarge the buffer now rather than incrementally making more space when
   * we run out.  Add two extra bytes for the enclosing quotes.
   */
  enlargeStringInfo(buf, len + 2);

  /*
   * Figure out how many bytes to process using SIMD.  Round 'len' down to
   * the previous multiple of sizeof(Vector8), assuming that's a power-of-2.
   */
  vlen = len & (int) (~(sizeof(Vector8) - 1));
  appendStringInfoCharMacro(buf, '"');
  for (int i = 0, copypos = 0;;)
  {
    /*
     * To speed this up, try searching sizeof(Vector8) bytes at once for
     * special characters that we need to escape.  When we find one, we
     * fall out of the Vector8 loop and copy the portion we've vector
     * searched and then we process sizeof(Vector8) bytes one byte at a
     * time.  Once done, come back and try doing vector searching again.
     * We'll also process any remaining bytes at the tail end of the
     * string byte-by-byte.  This optimization assumes that most chunks of
     * sizeof(Vector8) bytes won't contain any special characters.
     */
    for (; i < vlen; i += sizeof(Vector8))
    {
      Vector8 chunk;
      vector8_load(&chunk, (const uint8 *) &str[i]);

      /*
       * Break on anything less than ' ' or if we find a '"' or '\\'.
       * Those need special handling.  That's done in the per-byte loop.
       */
      if (vector8_has_le(chunk, (unsigned char) 0x1F) ||
        vector8_has(chunk, (unsigned char) '"') ||
        vector8_has(chunk, (unsigned char) '\\'))
        break;

#ifdef ESCAPE_JSON_FLUSH_AFTER

      /*
       * Flush what's been checked so far out to the destination buffer
       * every so often to avoid having to re-read cachelines when
       * escaping large strings.
       */
      if (i - copypos >= ESCAPE_JSON_FLUSH_AFTER)
      {
        appendBinaryStringInfo(buf, &str[copypos], i - copypos);
        copypos = i;
      }
#endif
    }

    /*
     * Write to the destination up to the point that we've vector searched
     * so far.  Do this only when switching into per-byte mode rather than
     * once every sizeof(Vector8) bytes.
     */
    if (copypos < i)
    {
      appendBinaryStringInfo(buf, &str[copypos], i - copypos);
      copypos = i;
    }

    /*
     * Per-byte loop for Vector8s containing special chars and for
     * processing the tail of the string.
     */
    for (int b = 0; b < (int) sizeof(Vector8); b++)
    {
      /* check if we've finished */
      if (i == len)
        goto done;
      Assert(i < len);
      escape_json_char(buf, str[i++]);
    }
    copypos = i;
    /* We're not done yet.  Try the vector search again. */
  }

done:
  appendStringInfoCharMacro(buf, '"');
}

/*
 * escape_json_text
 *    Append 'txt' onto 'buf' and escape using escape_json_with_len.
 *
 * This is more efficient than calling text_to_cstring and appending the
 * result as that could require an additional palloc and memcpy.
 */
void
escape_json_text(StringInfo buf, const text *txt)
{
  int len = VARSIZE_ANY_EXHDR(txt);
  char *str = VARDATA_ANY(txt);
  escape_json_with_len(buf, str, len);
  return;
}

/* Semantic actions for key uniqueness check */
static JsonParseErrorType
json_unique_object_start(void *_state)
{
  JsonUniqueParsingState *state = _state;
  JsonUniqueStackEntry *entry;
  if (!state->unique)
    return JSON_SUCCESS;

  /* push object entry to stack */
  entry = palloc(sizeof(*entry));
  entry->object_id = state->id_counter++;
  entry->parent = state->stack;
  state->stack = entry;

  return JSON_SUCCESS;
}

static JsonParseErrorType
json_unique_object_end(void *_state)
{
  JsonUniqueParsingState *state = _state;
  JsonUniqueStackEntry *entry;
  if (!state->unique)
    return JSON_SUCCESS;

  entry = state->stack;
  state->stack = entry->parent;  /* pop object from stack */
  pfree(entry);
  return JSON_SUCCESS;
}

static JsonParseErrorType
json_unique_object_field_start(void *_state, char *field, bool isnull UNUSED)
{
  JsonUniqueParsingState *state = _state;
  JsonUniqueStackEntry *entry;
  if (!state->unique)
    return JSON_SUCCESS;

  /* find key collision in the current object */
  if (json_unique_check_key(&state->check, field, state->stack->object_id))
    return JSON_SUCCESS;

  state->unique = false;

  /* pop all objects entries */
  while ((entry = state->stack))
  {
    state->stack = entry->parent;
    pfree(entry);
  }
  return JSON_SUCCESS;
}

/* Validate JSON text and additionally check key uniqueness */
bool
json_validate(text *json, bool check_unique_keys, bool throw_error)
{
  JsonLexContext lex;
  JsonSemAction uniqueSemAction = {0};
  JsonUniqueParsingState state;
  JsonParseErrorType result;
  makeJsonLexContext(&lex, json, check_unique_keys);

  if (check_unique_keys)
  {
    state.lex = &lex;
    state.stack = NULL;
    state.id_counter = 0;
    state.unique = true;
    json_unique_check_init(&state.check);

    uniqueSemAction.semstate = &state;
    uniqueSemAction.object_start = json_unique_object_start;
    uniqueSemAction.object_field_start = json_unique_object_field_start;
    uniqueSemAction.object_end = json_unique_object_end;
  }

  result = pg_parse_json(&lex, check_unique_keys ? 
    &uniqueSemAction : &nullSemAction);
  if (result != JSON_SUCCESS)
  {
    if (throw_error)
      json_errsave_error(result, &lex, NULL);
    return false;      /* invalid json */
  }
  if (check_unique_keys && !state.unique)
  {
    if (throw_error)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "duplicate JSON object key value");
    }
    return false;      /* not unique keys */
  }

  freeJsonLexContext(&lex);
  return true;        /* ok */
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Returns the type of the outermost JSON value as `text`
 * @details Possible types are "object", "array", "string", "number",
 * "boolean", and "null".
 * @details Performs a single call to json_lex() to get the first token of the
 * supplied value.  This initial token uniquely determines the value's type.
 * As our input must already have been validated by json_in() or json_recv(),
 * theinitial token should never be JSON_TOKEN_OBJECT_END,
 * JSON_TOKEN_ARRAY_END, JSON_TOKEN_COLON, JSON_TOKEN_COMMA, or JSON_TOKEN_END.
 * @param[in] json JSON value 
 * @note Derived from PostgreSQL function @p json_typeof()
 */
#if MEOS
text *
json_typeof(const text *json)
{
  return pg_json_typeof(json);
}
#endif /* MEOS */
text *
pg_json_typeof(const text *json)
{
  JsonLexContext lex;
  char *type;
  JsonParseErrorType result;

  /* Lex exactly one token from the input and check its type. */
  makeJsonLexContext(&lex, (text *) json, false);
  result = json_lex(&lex);
  if (result != JSON_SUCCESS)
    json_errsave_error(result, &lex, NULL);

  switch (lex.token_type)
  {
    case JSON_TOKEN_OBJECT_START:
      type = "object";
      break;
    case JSON_TOKEN_ARRAY_START:
      type = "array";
      break;
    case JSON_TOKEN_STRING:
      type = "string";
      break;
    case JSON_TOKEN_NUMBER:
      type = "number";
      break;
    case JSON_TOKEN_TRUE:
    case JSON_TOKEN_FALSE:
      type = "boolean";
      break;
    case JSON_TOKEN_NULL:
      type = "null";
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "unexpected json token: %d", lex.token_type);
      return NULL;
  }

  return cstring_to_text(type);
}

/*****************************************************************************/
