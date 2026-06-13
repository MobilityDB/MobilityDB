/*-------------------------------------------------------------------------
 *
 * hashfunc.c
 *    Support functions for hash access method.
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/access/hash/hashfunc.c
 *
 * NOTES
 *    These functions are stored in pg_amproc.  For each operator class
 *    defined for hash indexes, they compute the hash value of the argument.
 *
 *    Additional hash functions appear in /utils/adt/ files for various
 *    specialized datatypes.
 *
 *    It is expected that every bit of a hash function's 32-bit result is
 *    as random as every other; failure to ensure this is likely to lead
 *    to poor performance of hash joins, for example.  In most cases a hash
 *    function should use hash_any() or its variant hash_uint32().
 *-------------------------------------------------------------------------
 */

#include <limits.h>

#include "postgres.h"
#include "varatt.h"
#include "common/hashfn.h"
#include "utils/float.h"
#include "utils/pg_locale.h"

/*
 * Datatype-specific hash functions.
 *
 * These support both hash indexes and hash joins.
 *
 * NOTE: some of these are also used by catcache operations, without
 * any direct connection to hash indexes.  Also, the common hash_any
 * routine is also used by dynahash tables.
 */

/**
 * @ingroup meos_base_text
 * @brief Return the 32-bit hash of a character
 * @note Derived from PostgreSQL function @p hashchar()
 */
/* Note: this is used for both "char" and boolean datatypes */
uint32
char_hash(char c)
{
  return hash_uint32((int32) c);
}

/**
 * @ingroup meos_base_text
 * @brief Return the 64-bit hash of a character using a seed
 * @note Derived from PostgreSQL function @p hashchar()
 */
uint64
char_hash_extended(char c, uint64 seed)
{
  return hash_uint32_extended((int32) c, seed);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 32-bit hash of an int16
 * @note Derived from PostgreSQL function @p hashint2()
 */
uint32
int16_hash(int16 val)
{
  return hash_uint32((int32) val);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 64-bit hash of an int16 using a seed
 * @note Derived from PostgreSQL function @p hashint2extended()
 */
uint64
int16_hash_extended(int16 val, uint64 seed)
{
  return hash_uint32_extended((int32) val, seed);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 32-bit hash of an int32
 * @note Derived from PostgreSQL function @p hashint4()
 */
uint32
int32_hash(int32 val)
{
  return hash_uint32(val);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 64-bit hash of an int32 using a seed
 * @note Derived from PostgreSQL function @p hashint4extended()
 */
uint64
int32_hash_extended(int32 val, uint64 seed)
{
  return hash_uint32_extended(val, seed);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 32-bit hash of an int64
 * @note Derived from PostgreSQL function @p hashint8()
 */
uint32
int64_hash(int64 val)
{
  /*
   * The idea here is to produce a hash value compatible with the values
   * produced by hashint4 and hashint2 for logically equal inputs; this is
   * necessary to support cross-type hash joins across these input types.
   * Since all three types are signed, we can xor the high half of the int8
   * value if the sign is positive, or the complement of the high half when
   * the sign is negative.
   */
  uint32    lohalf = (uint32) val;
  uint32    hihalf = (uint32) (val >> 32);
  lohalf ^= (val >= 0) ? hihalf : ~hihalf;
  return hash_uint32(lohalf);
}

/**
 * @ingroup meos_base_integer
 * @brief Return the 64-bit hash of an int64 using a seed
 * @note Derived from PostgreSQL function @p hashint8extended()
 */
uint64
int64_hash_extended(int64 val, uint64 seed)
{
  /* Same approach as hashint8 */
  uint32 lohalf = (uint32) val;
  uint32 hihalf = (uint32) (val >> 32);
  lohalf ^= (val >= 0) ? hihalf : ~hihalf;
  return hash_uint32_extended(lohalf, seed);
}

/**
 * @ingroup meos_base_float
 * @brief Return the 32-bit hash of a float4
 * @note Derived from PostgreSQL function @p hashfloat4()
 */
uint32
float4_hash(float4 num)
{
  /*
   * On IEEE-float machines, minus zero and zero have different bit patterns
   * but should compare as equal.  We must ensure that they have the same
   * hash value, which is most reliably done this way:
   */
  if (num == (float4) 0)
    return 0;

  /*
   * To support cross-type hashing of float8 and float4, we want to return
   * the same hash value hashfloat8 would produce for an equal float8 value.
   * So, widen the value to float8 and hash that.  (We must do this rather
   * than have hashfloat8 try to narrow its value to float4; that could fail
   * on overflow.)
   */
  float8 num8 = num;

  /*
   * Similarly, NaNs can have different bit patterns but they should all
   * compare as equal.  For backwards-compatibility reasons we force them to
   * have the hash value of a standard float8 NaN.  (You'd think we could
   * replace key with a float4 NaN and then widen it; but on some old
   * platforms, that way produces a different bit pattern.)
   */
  if (isnan(num8))
    num8 = get_float8_nan();

  return hash_any((unsigned char *) &num8, sizeof(num8));
}

/**
 * @ingroup meos_base_float
 * @brief Return the 64-bit hash of a float4 using a seed
 * @note Derived from PostgreSQL function @p hashfloat4extended()
 */
uint64
float4_hash_extended(float4 num, uint64 seed)
{
  /* Same approach as hashfloat4 */
  if (num == (float4) 0)
    return seed;
  float8 num8 = num;
  if (isnan(num8))
    num8 = get_float8_nan();

  return hash_any_extended((unsigned char *) &num8, sizeof(num8), seed);
}

/**
 * @ingroup meos_base_float
 * @brief Return the 32-bit hash of a float8
 * @note Derived from PostgreSQL function @p hashfloat8()
 */
uint32
float8_hash(float8 num)
{
  /*
   * On IEEE-float machines, minus zero and zero have different bit patterns
   * but should compare as equal.  We must ensure that they have the same
   * hash value, which is most reliably done this way:
   */
  if (num == (float8) 0)
    return 0;

  /*
   * Similarly, NaNs can have different bit patterns but they should all
   * compare as equal.  For backwards-compatibility reasons we force them to
   * have the hash value of a standard NaN.
   */
  if (isnan(num))
    num = get_float8_nan();

  return hash_any((unsigned char *) &num, sizeof(num));
}

/**
 * @ingroup meos_base_float
 * @brief Return the 64-bit hash of a float8 using a seed
 * @note Derived from PostgreSQL function @p hashfloat8extended()
 */
uint64
float8_hash_extended(float8 num, uint64 seed)
{
  /* Same approach as hashfloat8 */
  if (num == (float8) 0)
    return seed;
  if (isnan(num))
    num = get_float8_nan();

  return hash_any_extended((unsigned char *) &num, sizeof(num), seed);
}

/**
 * @ingroup meos_base_text
 * @brief Return the 32-bit hash of a text
 * @note Derived from PostgreSQL function @p hashtext()
 */
uint32
text_hash(const text *txt, Oid collid)
{
  if (!collid)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not determine which collation to use for string hashing");
    return UINT_MAX;
  }

  uint32 result;
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (! mylocale || mylocale->deterministic)
  {
    result = hash_any((unsigned char *) VARDATA_ANY(txt),
      VARSIZE_ANY_EXHDR(txt));
  }
  else
  {
    const char *txtdata = VARDATA_ANY(txt);
    size_t txtlen = VARSIZE_ANY_EXHDR(txt);

    Size bsize = pg_strnxfrm(NULL, 0, txtdata, txtlen, mylocale);
    char *buf = palloc(bsize + 1);
    Size rsize = pg_strnxfrm(buf, bsize + 1, txtdata, txtlen, mylocale);

    /* the second call may return a smaller value than the first */
    if (rsize > bsize)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "pg_strnxfrm() returned unexpected result");
      return UINT_MAX;
    }

    /*
     * In principle, there's no reason to include the terminating NUL
     * character in the hash, but it was done before and the behavior must
     * be preserved.
     */
    result = hash_any((uint8_t *) buf, bsize + 1);
    pfree(buf);
  }
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return true if two date values are equal
 * @note Derived from PostgreSQL function @p hashtextextended()
 */
uint64
text_hash_extended(const text *txt, uint64 seed, Oid collid)
{
  if (!collid)
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "could not determine which collation to use for string hashing");
    return ULONG_MAX;
  }

  uint64 result;
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (! mylocale || mylocale->deterministic)
  {
    result = hash_any_extended((unsigned char *) VARDATA_ANY(txt),
      VARSIZE_ANY_EXHDR(txt), seed);
  }
  else
  {
    const char *txtdata = VARDATA_ANY(txt);
    size_t txtlen = VARSIZE_ANY_EXHDR(txt);

    Size bsize = pg_strnxfrm(NULL, 0, txtdata, txtlen, mylocale);
    char *buf = palloc(bsize + 1);
    Size rsize = pg_strnxfrm(buf, bsize + 1, txtdata, txtlen, mylocale);

    /* the second call may return a smaller value than the first */
    if (rsize > bsize)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "pg_strnxfrm() returned unexpected result");
      return ULONG_MAX;
    }

    /*
     * In principle, there's no reason to include the terminating NUL
     * character in the hash, but it was done before and the behavior must
     * be preserved.
     */
    result = hash_any_extended((uint8_t *) buf, bsize + 1, seed);
    pfree(buf);
  }
  return result;
}

/*****************************************************************************/