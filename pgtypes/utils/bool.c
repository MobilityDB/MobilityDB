/*-------------------------------------------------------------------------
 *
 * bool.c
 *    Functions for the built-in type "bool".
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/bool.c
 *
 *-------------------------------------------------------------------------
 */

/* C */
#include <ctype.h>
/* PostgreSQL */
#include <postgres.h>
#include "common/hashfn.h"
#include "utils/builtins.h"

#include "../../meos/include/meos_error.h"

/*****************************************************************************/
bool
parse_bool_with_len(const char *value, size_t len, bool *result)
{
  /* Check the most-used possibilities first. */
  switch (*value)
  {
    case 't':
    case 'T':
      if (pg_strncasecmp(value, "true", len) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case 'f':
    case 'F':
      if (pg_strncasecmp(value, "false", len) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case 'y':
    case 'Y':
      if (pg_strncasecmp(value, "yes", len) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case 'n':
    case 'N':
      if (pg_strncasecmp(value, "no", len) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case 'o':
    case 'O':
      /* 'o' is not unique enough */
      if (pg_strncasecmp(value, "on", (len > 2 ? len : 2)) == 0)
      {
        if (result)
          *result = true;
        return true;
      }
      else if (pg_strncasecmp(value, "off", (len > 2 ? len : 2)) == 0)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    case '1':
      if (len == 1)
      {
        if (result)
          *result = true;
        return true;
      }
      break;
    case '0':
      if (len == 1)
      {
        if (result)
          *result = false;
        return true;
      }
      break;
    default:
      break;
  }

  if (result)
    *result = false;    /* suppress compiler warning */
  return false;
}

/*
 * Try to interpret value as boolean value.  Valid values are: true,
 * false, yes, no, on, off, 1, 0; as well as unique prefixes thereof.
 * If the string parses okay, return true, else false.
 * If okay and result is not NULL, return the value in *result.
 */
bool
parse_bool(const char *value, bool *result)
{
  return parse_bool_with_len(value, strlen(value), result);
}

/*****************************************************************************
 *   USER I/O ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_bool
 * @brief Return a boolean from its string representation
 * @param[in] str String
 * @details Convert @p "t" or @p "f" to 1 or 0.
 * Check explicitly for @p "true/false" and @p TRUE/FALSE, @p 1/0, @p YES/NO,
 * @p ON/OFF and reject other values. In the @p switch statement, check the
 * most-used possibilities first.
 * @note PostgreSQL function: @p boolin()
 */
bool
bool_in(const char *str)
{
  /*
   * Skip leading and trailing whitespace
   */
  const char *str1 = str;
  while (isspace((unsigned char) *str1))
    str1++;

  size_t len = strlen(str1);
  while (len > 0 && isspace((unsigned char) str1[len - 1]))
    len--;

  bool result;
  if (parse_bool_with_len(str1, len, &result))
    return result;

  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "invalid input syntax for type boolean: \"%s\"", str);
  return false;
}

/**
 * @ingroup meos_base_bool
 * @brief Return the string representation of a boolean
 * @param[in] b Boolean value
 * @details Convert 1 or 0 to @p "t" or @p "f"
 * @note PostgreSQL function: @p boolout()
 */
char *
bool_out(bool b)
{
  char *result = (char *) palloc(2);
  result[0] = (b) ? 't' : 'f';
  result[1] = '\0';
  return result;
}

/**
 * @ingroup meos_base_bool
 * @brief Return the text representation of a boolean
 * @param[in] b Boolean value
 * @note PostgreSQL function: @p booltext()
 */
text *
bool_to_text(bool b)
{
  const char *str;
  if (b)
    str = "true";
  else
    str = "false";
  return pg_cstring_to_text(str);
}

/*****************************************************************************
 *   PUBLIC ROUTINES                             *
 *****************************************************************************/

/**
 * @ingroup meos_base_bool
 * @brief Return true if two booleans are equal
 * @note Derived from PostgreSQL function @p booleq()
 */
bool
bool_eq(bool b1, bool b2)
{
  return (b1 == b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return true if two booleans are not equal
 * @note Derived from PostgreSQL function @p boolne()
 */
bool
bool_ne(bool b1, bool b2)
{
  return (b1 != b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return true if a boolean is less than another one
 * @note Derived from PostgreSQL function @p boollt()
 */
bool
bool_lt(bool b1, bool b2)
{
  return (b1 < b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return true if a boolean is less than or equal to another one
 * @note Derived from PostgreSQL function @p boolgt()
 */
bool
bool_gt(bool b1, bool b2)
{
  return (b1 > b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return true if a boolean is greater than another one
 * @note Derived from PostgreSQL function @p boolle()
 */
bool
bool_le(bool b1, bool b2)
{
  return (b1 <= b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return true if a boolean is greater than or equal to another one
 * @note Derived from PostgreSQL function @p boolge()
 */
bool
bool_ge(bool b1, bool b2)
{
  return (b1 >= b2);
}

/**
 * @ingroup meos_base_bool
 * @brief Return the 32-bit hash of a span
 * @note Derived from PostgreSQL function @p hashbool()
 */
uint32
bool_hash(bool b)
{
  return hash_uint32((int32) b);
}

/**
 * @ingroup meos_base_bool
 * @brief Return the 64-bit hash of a boolean using a seed
 * @note Derived from PostgreSQL function @p hashboolextended()
 */
uint64
bool_hash_extended(bool b, int64 seed)
{
  return hash_uint32_extended((int32) b, seed);
}

/****************************************************************************/