/*-------------------------------------------------------------------------
 *
 * jsonb_op.c
 *   Special operators for jsonb only, used by various index access methods
 *
 * Copyright (c) 2014-2025, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/jsonb_op.c
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
#include <common/hashfn.h>
#include <common/int.h>
#include <utils/date.h>
#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <utils/timestamp.h>
#include <utils/varlena.h> /* For DatumGetTextP */

#include <pgtypes.h>

/*****************************************************************************/

/**
 * @ingroup meos_json_base_accessor
 * @brief Return true if the text string exist as a top-level key or array
 * element within the JSON value
 * @param[in] jb JSONB value
 * @param[in] key Key
 * @csqlfn #Jsonb_exists()
 * @note Derived from PostgreSQL function @p jsonb_exists()
 */
#if MEOS
bool
jsonb_exists(const Jsonb *jb, const text *key)
{
  return pg_jsonb_exists(jb, key);
}
#endif /* MEOS */
bool
pg_jsonb_exists(const Jsonb *jb, const text *key)
{
  JsonbValue kval;
  JsonbValue *v = NULL;

  /*
   * We only match Object keys (which are naturally always Strings), or
   * string elements in arrays.  In particular, we do not match non-string
   * scalar elements.  Existence of a key/element is only considered at the
   * top level.  No recursion occurs.
   */
  kval.type = jbvString;
  kval.val.string.val = VARDATA_ANY(key);
  kval.val.string.len = VARSIZE_ANY_EXHDR(key);

  v = findJsonbValueFromContainer(&((Jsonb *) jb)->root,
    JB_FOBJECT | JB_FARRAY, &kval);
  bool result = (v != NULL); // MEOS
  if (v) pfree(v); /* findJsonbValueFromContainer palloc's the result */
  return result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return true if the text string exists as a top-level key or array
 * element within the JSONB value
 * @csqlfn #Jsonb_exists_any() #Jsonb_exists_all()
 * @note Derived from PostgreSQL function @p jsonb_exists_any()
 * @note Derived from PostgreSQL function @p jsonb_exists_all()
 */
bool
jsonb_exists_array(const Jsonb *jb, text **keys_elems, int keys_len, bool any)
{
  return pg_jsonb_exists_array(jb, keys_elems, keys_len, any);
}
bool
pg_jsonb_exists_array(const Jsonb *jb, text **keys_elems, int keys_len,
  bool any)
{
  /* Ensure the validity of the arguments */
  assert(jb); assert(keys_elems); assert(keys_len > 0);

  for (int i = 0; i < keys_len; i++)
  {
    /* A NULL key is skipped, as in #pg_jsonb_delete_array */
    if (! keys_elems[i])
      continue;
    JsonbValue strVal;
    strVal.type = jbvString;
    /* We rely on the array elements not being toasted */
    strVal.val.string.val = VARDATA_ANY(keys_elems[i]);
    strVal.val.string.len = VARSIZE_ANY_EXHDR(keys_elems[i]);
    JsonbValue *fv = findJsonbValueFromContainer(&((Jsonb *) jb)->root,
      JB_FOBJECT | JB_FARRAY, &strVal);
    bool res = (fv != NULL);
    if (fv) pfree(fv); /* palloc'd by findJsonbValueFromContainer */
    if ((any && res) || (! any && ! res))
      return any ? true : false;
  }
  return any ? false : true;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return true if the first JSON value contains the second one
 * @csqlfn #Jsonb_contains()
 * @note Derived from PostgreSQL function @p jsonb_contains()
 */
#if MEOS
bool
jsonb_contains(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_contains(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_contains(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public entry: a NULL argument would otherwise be dereferenced
   * (root inspection / iteration) and fault. Raise a clean MEOS error. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb_contains: NULL argument");
    return false;
  }
  if (JB_ROOT_IS_OBJECT(jb1) != JB_ROOT_IS_OBJECT(jb2))
    return false;
  JsonbIterator *it1 = JsonbIteratorInit(&((Jsonb *)jb1)->root);
  JsonbIterator *it2 = JsonbIteratorInit(&((Jsonb *)jb2)->root);
  bool result = JsonbDeepContains(&it1, &it2); // MEOS
  /* JsonbDeepContains may stop early, leaving the iterators partially
   * consumed; free whatever remains of each chain (no memory context). */
  while (it1) { JsonbIterator *p = it1->parent; pfree(it1); it1 = p; }
  while (it2) { JsonbIterator *p = it2->parent; pfree(it2); it2 = p; }
  return result;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return true if the first JSON value is contained into the second one
 * @csqlfn #Jsonb_contained()
 * @note Derived from PostgreSQL function @p jsonb_contained()
 */
#if MEOS
bool
jsonb_contained(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_contains(jb2, jb1);
}
#endif /* MEOS */
bool
pg_jsonb_contained(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_contains(jb2, jb1);
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if two JSONB values are equal
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_eq()
 * @note Derived from PostgreSQL function @p jsonb_eq()
 */
#if MEOS
bool
jsonb_eq(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_eq(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_eq(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) == 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if two JSONB values are not equal
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_ne()
 * @note Derived from PostgreSQL function @p jsonb_ne()
 */
#if MEOS
bool
jsonb_ne(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_ne(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_ne(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) != 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if the first JSONB value is less than the second one
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_lt()
 * @note Derived from PostgreSQL function @p jsonb_lt()
 */
#if MEOS
bool
jsonb_lt(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_lt(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_lt(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) < 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if the first JSONB value is greater than the second one
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_gt()
 * @note Derived from PostgreSQL function @p jsonb_gt()
 */
#if MEOS
bool
jsonb_gt(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_gt(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_gt(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) > 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if the first JSONB value is less than or equal to the
 * second one
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_le()
 * @note Derived from PostgreSQL function @p jsonb_le()
 */
#if MEOS
bool
jsonb_le(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_le(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_le(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) <= 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return true if the first JSONB value is greater than or equal to the
 * second one
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_ge()
 * @note Derived from PostgreSQL function @p jsonb_ge()
 */
#if MEOS
bool
jsonb_ge(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_ge(jb1, jb2);
}
#endif /* MEOS */
bool
pg_jsonb_ge(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) >= 0;
}

/**
 * @ingroup meos_json_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first JSONB value
 * is less than, equal to, or greater than the second one
 * @param[in] jb1,jb2 JSONB values
 * @csqlfn #Jsonb_cmp()
 * @note Derived from PostgreSQL function @p jsonb_cmp()
 */
#if MEOS
int
jsonb_cmp(const Jsonb *jb1, const Jsonb *jb2)
{
  return pg_jsonb_cmp(jb1, jb2);
}
#endif /* MEOS */
int
pg_jsonb_cmp(const Jsonb *jb1, const Jsonb *jb2)
{
  /* Guard the public comparison entry: a NULL argument would otherwise be
   * dereferenced when its container is iterated and fault. Raise a clean
   * MEOS error so bindings get an exception rather than a SIGSEGV. */
  if (jb1 == NULL || jb2 == NULL)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "jsonb comparison: NULL argument");
    return 0;
  }
  /* The three-way result is the answer, where the sibling predicates above
   * reduce it to a boolean. A caller ordering values reads the sign, and a
   * caller matching them reads the zero */
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root);
}

/*****************************************************************************
 * Sort key
 *****************************************************************************/

/**
 * @brief Write the sort key of a number, or count its bytes when the buffer
 * is NULL
 * @details The number is read in the form #numeric_normalize gives it, in
 * which two numbers are spelled alike exactly when #pg_numeric_cmp holds them
 * equal. The key is a byte stating the sign, 1 for a negative number, 2 for
 * zero and 3 for a positive one, followed for a number other than zero by its
 * decimal exponent e, the value being 0.d1d2... times 10 to the e, written as
 * four bytes most significant first with the sign bit flipped, as
 * #datum_sortkey_put writes an integer, then each significant digit d as
 * d + 1, then a zero byte. Every byte after the sign is complemented for a
 * negative number, so that a greater magnitude sorts first, and its ending
 * byte is then 0xFF, which sorts after every complemented digit
 * @param[in] num Number, neither NaN nor infinite
 * @param[out] out Buffer @nullable
 * @return The number of bytes of the key
 */
static size_t
numeric_sortkey_write(Numeric num, uint8_t *out)
{
  char *str = numeric_normalize(num);
  const char *s = str;
  bool neg = (*s == '-');
  if (neg)
    s++;
  /* The digits before the decimal point, then those after it */
  const char *point = strchr(s, '.');
  const char *intend = point ? point : s + strlen(s);
  const char *frac = point ? point + 1 : intend;
  /* Skip the leading zeros: those of the integer part, and when it is zero
   * those of the fraction, each lowering the exponent by one */
  long exponent;
  const char *first = s;
  while (first < intend && *first == '0')
    first++;
  if (first < intend)
    exponent = intend - first;
  else
  {
    first = frac;
    while (*first == '0')
      first++;
    exponent = frac - first;
  }
  if (*first == '\0')
  {
    /* Zero */
    if (out)
      out[0] = 2;
    pfree(str);
    return 1;
  }
  /* The significant digits run from the first one to the last one that is
   * not zero, the decimal point skipped */
  const char *last = s + strlen(s) - 1;
  while (*last == '0' || *last == '.')
    last--;
  size_t ndigits = 0;
  for (const char *p = first; p <= last; p++)
    if (*p != '.')
      ndigits++;
  size_t result = 1 + 4 + ndigits + 1;
  if (out)
  {
    uint8_t flip = neg ? 0xFF : 0x00;
    *out++ = neg ? 1 : 3;
    uint32 e = ((uint32) exponent) ^ UINT32_C(0x80000000);
    for (int i = 3; i >= 0; i--)
      *out++ = ((uint8_t) (e >> (8 * i))) ^ flip;
    for (const char *p = first; p <= last; p++)
      if (*p != '.')
        *out++ = ((uint8_t) (*p - '0' + 1)) ^ flip;
    *out = flip;
  }
  pfree(str);
  return result;
}

/**
 * @brief Write the sort key of a JSON value, or count its bytes when the
 * buffer is NULL
 * @details The tokens are read as #pg_jsonb_hash_extended reads them, in the
 * order compareJsonbContainers compares them, and each is written so that it
 * ends where its bytes say it does: the type of a value as a byte, in the
 * order of the type enumeration (null, string, number, Boolean, array,
 * object), then for an array its number of elements, four bytes most
 * significant first, and a byte that is 0 for a scalar standing alone and 1
 * otherwise; for an object its number of pairs; for a string its bytes, a
 * zero byte written as a zero byte followed by 0xFF, ended by two zero bytes,
 * as #datum_sortkey_put writes a text; for a number the key
 * #numeric_sortkey_write writes; for a Boolean a byte. The end of an array or
 * of an object writes nothing: two values whose containers hold as many
 * elements reach their ends together. So comparing two keys byte by byte
 * orders the values as #pg_jsonb_cmp does, the first token that differs
 * deciding, and two values have the same key exactly when #pg_jsonb_eq holds
 * @param[in] jb JSON value
 * @param[out] out Buffer @nullable
 * @return The number of bytes of the key
 */
static size_t
jsonb_sortkey_write(const Jsonb *jb, uint8_t *out)
{
  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *) jb)->root);
  JsonbValue v;
  JsonbIteratorToken r;
  size_t result = 0;
  while ((r = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    if (r == WJB_END_ARRAY || r == WJB_END_OBJECT)
      continue;
    if (out)
      out[result] = (uint8_t) v.type;
    result++;
    switch (v.type)
    {
      case jbvArray:
        if (out)
        {
          uint32 n = (uint32) v.val.array.nElems;
          for (int i = 0; i < 4; i++)
            out[result + i] = (uint8_t) (n >> (8 * (3 - i)));
          out[result + 4] = v.val.array.rawScalar ? 0 : 1;
        }
        result += 5;
        break;
      case jbvObject:
        if (out)
        {
          uint32 n = (uint32) v.val.object.nPairs;
          for (int i = 0; i < 4; i++)
            out[result + i] = (uint8_t) (n >> (8 * (3 - i)));
        }
        result += 4;
        break;
      case jbvString:
        for (int i = 0; i < v.val.string.len; i++)
        {
          if (out)
            out[result] = (uint8_t) v.val.string.val[i];
          result++;
          if (v.val.string.val[i] == '\0')
          {
            if (out)
              out[result] = 0xFF;
            result++;
          }
        }
        if (out)
          out[result] = out[result + 1] = 0;
        result += 2;
        break;
      case jbvNumeric:
        result += numeric_sortkey_write(v.val.numeric,
          out ? out + result : NULL);
        break;
      case jbvBool:
        if (out)
          out[result] = v.val.boolean ? 1 : 0;
        result++;
        break;
      case jbvNull:
        break;
      default:
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "Unknown JSON value type for a sort key: %d", v.type);
        return 0;
    }
  }
  return result;
}

/**
 * @brief Return the number of bytes of the sort key of a JSON value, which
 * #jsonb_sortkey_write states
 * @param[in] jb JSON value
 */
size_t
jsonb_sortkey_size(const Jsonb *jb)
{
  assert(jb);
  return jsonb_sortkey_write(jb, NULL);
}

/**
 * @brief Write the sort key of a JSON value, which #jsonb_sortkey_write
 * states
 * @param[in] jb JSON value
 * @param[out] out Buffer of #jsonb_sortkey_size bytes
 * @return The first byte after the key
 */
uint8_t *
jsonb_sortkey_put(const Jsonb *jb, uint8_t *out)
{
  assert(jb); assert(out);
  return out + jsonb_sortkey_write(jb, out);
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the 32-bit hash value of a JSONB value
 * @csqlfn #Jsonb_hash()
 * @note Derived from PostgreSQL function @p jsonb_hash()
 */
#if MEOS
uint32
jsonb_hash(const Jsonb *jb)
{
  return pg_jsonb_hash(jb);
}
#endif /* MEOS */
uint32
pg_jsonb_hash(const Jsonb *jb)
{
  if (JB_ROOT_COUNT(jb) == 0)
    return (0);

  JsonbIterator *it = JsonbIteratorInit((JsonbContainer *) &jb->root);
  JsonbIteratorToken r;
  JsonbValue v;
  uint32 hash = 0;
  while ((r = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    switch (r)
    {
        /* Rotation is left to JsonbHashScalarValue() */
      case WJB_BEGIN_ARRAY:
        hash ^= JB_FARRAY;
        break;
      case WJB_BEGIN_OBJECT:
        hash ^= JB_FOBJECT;
        break;
      case WJB_KEY:
      case WJB_VALUE:
      case WJB_ELEM:
        JsonbHashScalarValue(&v, &hash);
        break;
      case WJB_END_ARRAY:
      case WJB_END_OBJECT:
        break;
      default:
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "invalid JsonbIteratorNext rc: %d", (int) r);
        return UINT_MAX;
      }
    }
  }
  return hash;
}

/**
 * @ingroup meos_json_base_accessor
 * @brief Return the 64-bit hash of a JSONB value using a seed
 * @csqlfn #Jsonb_hash_extended()
 * @note Derived from PostgreSQL function @p jsonb_hash_extended()
 */
#if MEOS
uint64
jsonb_hash_extended(const Jsonb *jb, uint64 seed)
{
  return pg_jsonb_hash_extended(jb, seed);
}
#endif /* MEOS */
uint64
pg_jsonb_hash_extended(const Jsonb *jb, uint64 seed)
{
  if (JB_ROOT_COUNT(jb) == 0)
    return seed;

  JsonbIterator *it = JsonbIteratorInit(&((Jsonb *) jb)->root);
  JsonbValue v;
  JsonbIteratorToken r;
  uint64 hash = 0;
  while ((r = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    switch (r)
    {
        /* Rotation is left to JsonbHashScalarValueExtended() */
      case WJB_BEGIN_ARRAY:
        hash ^= ((uint64) JB_FARRAY) << 32 | JB_FARRAY;
        break;
      case WJB_BEGIN_OBJECT:
        hash ^= ((uint64) JB_FOBJECT) << 32 | JB_FOBJECT;
        break;
      case WJB_KEY:
      case WJB_VALUE:
      case WJB_ELEM:
        JsonbHashScalarValueExtended(&v, &hash, seed);
        break;
      case WJB_END_ARRAY:
      case WJB_END_OBJECT:
        break;
      default:
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "invalid JsonbIteratorNext rc: %d", (int) r);
        return ULONG_MAX;
      }
    }
  }
  return hash;
}

/*****************************************************************************/