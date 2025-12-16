/*-------------------------------------------------------------------------
 *
 *    UTF8 and Cyrillic
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/utf8_and_cyrillic/utf8_and_cyrillic.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mb/pg_wchar.h"
#include "../Unicode/utf8_to_koi8r.map"
#include "../Unicode/koi8r_to_utf8.map"
#include "../Unicode/utf8_to_koi8u.map"
#include "../Unicode/koi8u_to_utf8.map"

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
utf8_to_koi8r(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_KOI8R);
  return UtfToLocal(src, len, dest, &koi8r_from_unicode_tree, NULL, 0,
    NULL, PG_KOI8R, noError);
}

int
koi8r_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_KOI8R, PG_UTF8);
  return LocalToUtf(src, len, dest, &koi8r_to_unicode_tree, NULL, 0, NULL,
    PG_KOI8R, noError);
}

int
utf8_to_koi8u(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_KOI8U);
  return UtfToLocal(src, len, dest, &koi8u_from_unicode_tree, NULL, 0, NULL,
    PG_KOI8U, noError);
}

int
koi8u_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_KOI8U, PG_UTF8);
  return LocalToUtf(src, len, dest, &koi8u_to_unicode_tree, NULL, 0, NULL,
    PG_KOI8U, noError);
}
