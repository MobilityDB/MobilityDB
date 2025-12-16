/*-------------------------------------------------------------------------
 *
 *    EUC_KR <--> UTF8
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/utf8_and_euc_kr/utf8_and_euc_kr.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mb/pg_wchar.h"
#include "../Unicode/euc_kr_to_utf8.map"
#include "../Unicode/utf8_to_euc_kr.map"

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
euc_kr_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_EUC_KR, PG_UTF8);
  return LocalToUtf(src, len, dest, &euc_kr_to_unicode_tree, NULL, 0,
    NULL, PG_EUC_KR, noError);
}

int
utf8_to_euc_kr(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_EUC_KR);
  return UtfToLocal(src, len, dest, &euc_kr_from_unicode_tree, NULL, 0, NULL,
    PG_EUC_KR, noError);
}
