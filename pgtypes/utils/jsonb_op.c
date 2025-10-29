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
#include <utils/jsonb.h>
#include <utils/varlena.h> /* For DatumGetTextP */

#include <utils/date.h>
#include <utils/numeric.h>
#include <utils/timestamp.h>
#include <pgtypes.h>

/*****************************************************************************/

/**
 * @ingroup meos_base_json
 * @brief Return true if the text string exist as a top-level key or array
 * element within the JSON value
 * @param[in] jb JSONB value
 * @param[in] key Key
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
  pfree(v);
  return result;
}

/**
 * @brief Return true if the text string exist as a top-level key or array
 * element within the JSON value
 */
Datum
datum_jsonb_exists(Datum l, Datum r)
{
  return BoolGetDatum(pg_jsonb_exists(DatumGetJsonbP(l), DatumGetTextP(r)));
}

/**
 * @brief Return true if the text string exist as a top-level key or array
 * element within the JSONB value
 * @note Derived from PostgreSQL function @p jsonb_delete_array()
 */
bool
jsonb_exists_array(const Jsonb *jb, const text **keys_elems,
  int keys_len, bool any)
{
  for (int i = 0; i < keys_len; i++)
  {
    JsonbValue strVal;
    strVal.type = jbvString;
    /* We rely on the array elements not being toasted */
    strVal.val.string.val = VARDATA_ANY(keys_elems[i]);
    strVal.val.string.len = VARSIZE_ANY_EXHDR(keys_elems[i]);
    bool res = findJsonbValueFromContainer(&((Jsonb *) jb)->root,
        JB_FOBJECT | JB_FARRAY, &strVal) != NULL;
    if ((any && res) || (! any && ! res))
      return any ? true : false;
  }
  return any ? false : true;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSON value contains the second one
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
  if (JB_ROOT_IS_OBJECT(jb1) != JB_ROOT_IS_OBJECT(jb2))
    return false;
  JsonbIterator *it1 = JsonbIteratorInit(&((Jsonb *)jb1)->root);
  JsonbIterator *it2 = JsonbIteratorInit(&((Jsonb *)jb2)->root);
  bool result = JsonbDeepContains(&it1, &it2); // MEOS
  pfree(it1); pfree(it2);
  return result;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSON value is contained into the second one
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
 * @brief Return true if the first JSON value contains the second one
 */
Datum
datum_jsonb_contains(Datum l, Datum r)
{
  return BoolGetDatum(pg_jsonb_contains(DatumGetJsonbP(l), DatumGetJsonbP(r)));
}

/**
 * @brief Return true if the first JSON value is contained into the second one
 */
Datum
datum_jsonb_contained(Datum l, Datum r)
{
  return BoolGetDatum(pg_jsonb_contains(DatumGetJsonbP(r), DatumGetJsonbP(l)));
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the two JSONB values are equal
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) == 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the two JSONB values are not equal
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) != 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSONB value is less than the second one
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) < 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSONB value is greater than the second one
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) > 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSONB value is less than or equal to the
 * second one
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) <= 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return true if the first JSONB value is greater than or equal to the
 * second one
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) >= 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return -1, 0, or 1 depending on whether the first JSONB value
 * is less than, equal to, or greater than the second one
 * @param[in] jb1,jb2 JSONB values
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
  assert(jb1); assert(jb2);
  return compareJsonbContainers((JsonbContainer *) &jb1->root,
    (JsonbContainer *) &jb2->root) <= 0;
}

/**
 * @ingroup meos_base_json
 * @brief Return the hash value of a temporal value
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
        elog(ERROR, "invalid JsonbIteratorNext rc: %d", (int) r);
    }
  }
  return hash;
}

/**
 * @ingroup meos_base_json
 * @brief Return the 64-bit hash of a JSONB value using a seed
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
        elog(ERROR, "invalid JsonbIteratorNext rc: %d", (int) r);
    }
  }
  return hash;
}

/*****************************************************************************/