/*-------------------------------------------------------------------------
 *
 *    LATINn and MULE_INTERNAL
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/latin_and_mic/latin_and_mic.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mb/pg_wchar.h"

/* ----------
 * conv_proc(
 *    INTEGER,  -- source encoding id
 *    INTEGER,  -- destination encoding id
 *    CSTRING,  -- source string (null terminated C string)
 *    CSTRING,  -- destination string (null terminated C string)
 *    INTEGER,  -- source string length
 *    BOOL    -- if true, don't throw an error if conversion fails
 * ) returns INTEGER;
 *
 * Returns the number of bytes successfully converted.
 * ----------
 */


int
latin1_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_LATIN1, PG_MULE_INTERNAL);
  return latin2mic(src, dest, len, LC_ISO8859_1, PG_LATIN1, noError);
}

int
mic_to_latin1(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_MULE_INTERNAL, PG_LATIN1);
  return mic2latin(src, dest, len, LC_ISO8859_1, PG_LATIN1, noError);
}

int
latin3_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_LATIN3, PG_MULE_INTERNAL);
  return latin2mic(src, dest, len, LC_ISO8859_3, PG_LATIN3, noError);
}

int
mic_to_latin3(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_MULE_INTERNAL, PG_LATIN3);
  return mic2latin(src, dest, len, LC_ISO8859_3, PG_LATIN3, noError);
}

int
latin4_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_LATIN4, PG_MULE_INTERNAL);
  return latin2mic(src, dest, len, LC_ISO8859_4, PG_LATIN4, noError);
}

int
mic_to_latin4(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_MULE_INTERNAL, PG_LATIN4);
  return mic2latin(src, dest, len, LC_ISO8859_4, PG_LATIN4, noError);
}
