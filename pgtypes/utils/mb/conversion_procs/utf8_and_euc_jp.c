/*-------------------------------------------------------------------------
 *
 *    EUC_JP <--> UTF8
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/utf8_and_euc_jp/utf8_and_euc_jp.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mb/pg_wchar.h"
#include "../Unicode/euc_jp_to_utf8.map"
#include "../Unicode/utf8_to_euc_jp.map"

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
euc_jp_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_EUC_JP, PG_UTF8);
  return LocalToUtf(src, len, dest, &euc_jp_to_unicode_tree, NULL, 0, NULL,
    PG_EUC_JP, noError);
}

int
utf8_to_euc_jp(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_EUC_JP);
  return UtfToLocal(src, len, dest, &euc_jp_from_unicode_tree, NULL, 0, NULL,
    PG_EUC_JP, noError);
}
