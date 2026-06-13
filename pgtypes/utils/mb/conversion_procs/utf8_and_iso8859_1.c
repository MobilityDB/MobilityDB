/*-------------------------------------------------------------------------
 *
 *    ISO8859_1 <--> UTF8
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *    src/backend/utils/mb/conversion_procs/utf8_and_iso8859_1/utf8_and_iso8859_1.c
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
iso8859_1_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_LATIN1, PG_UTF8);

  unsigned char *start = src;
  while (len > 0)
  {
    unsigned short c = *src;
    if (c == 0)
    {
      if (noError)
        break;
      report_invalid_encoding(PG_LATIN1, (const char *) src, len);
    }
    if (!IS_HIGHBIT_SET(c))
      *dest++ = c;
    else
    {
      *dest++ = (c >> 6) | 0xc0;
      *dest++ = (c & 0x003f) | HIGHBIT;
    }
    src++;
    len--;
  }
  *dest = '\0';
  return (src - start);
}

int
utf8_to_iso8859_1(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError)
{
  check_encoding_conversion_args(src_id, dest_id, len, PG_UTF8, PG_LATIN1);
  unsigned char *start = src;
  while (len > 0)
  {
    unsigned short c = *src;
    if (c == 0)
    {
      if (noError)
        break;
      report_invalid_encoding(PG_UTF8, (const char *) src, len);
    }
    /* fast path for ASCII-subset characters */
    if (!IS_HIGHBIT_SET(c))
    {
      *dest++ = c;
      src++;
      len--;
    }
    else
    {
      int l = pg_utf_mblen(src);
      if (l > len || !pg_utf8_islegal(src, l))
      {
        if (noError)
          break;
        report_invalid_encoding(PG_UTF8, (const char *) src, len);
      }
      if (l != 2)
      {
        if (noError)
          break;
        report_untranslatable_char(PG_UTF8, PG_LATIN1, (const char *) src, len);
      }
      unsigned short c1 = src[1] & 0x3f;
      c = ((c & 0x1f) << 6) | c1;
      if (c >= 0x80 && c <= 0xff)
      {
        *dest++ = (unsigned char) c;
        src += 2;
        len -= 2;
      }
      else
      {
        if (noError)
          break;
        report_untranslatable_char(PG_UTF8, PG_LATIN1, (const char *) src, len);
      }
    }
  }
  *dest = '\0';
  return (src - start);
}
