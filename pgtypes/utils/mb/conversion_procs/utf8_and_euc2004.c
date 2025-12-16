/*-------------------------------------------------------------------------
 *
 *    EUC_JIS_2004 <--> UTF8
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/utf8_and_euc2004/utf8_and_euc2004.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "utils/mb/pg_wchar.h"
#include "../Unicode/euc_jis_2004_to_utf8.map"
#include "../Unicode/utf8_to_euc_jis_2004.map"

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
euc_jis_2004_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_EUC_JIS_2004, PG_UTF8);
  return LocalToUtf(src, len, dest, &euc_jis_2004_to_unicode_tree,
    LUmapEUC_JIS_2004_combined, lengthof(LUmapEUC_JIS_2004_combined), NULL,
    PG_EUC_JIS_2004, noError);
}

int
utf8_to_euc_jis_2004(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_EUC_JIS_2004);
  return UtfToLocal(src, len, dest, &euc_jis_2004_from_unicode_tree,
    ULmapEUC_JIS_2004_combined, lengthof(ULmapEUC_JIS_2004_combined), NULL,
    PG_EUC_JIS_2004, noError);
}
