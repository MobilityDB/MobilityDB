/*-------------------------------------------------------------------------
 *
 * varlena.c
 *    Functions for the variable-length built-in types.
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *    src/backend/utils/adt/varlena.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>

#include "miscadmin.h"
#include "common/int.h"
#include "common/unicode_category.h"
#include "common/unicode_norm.h"
#include "common/unicode_version.h"
#include "utils/pg_locale.h"
#include "utils/varlena.h"
#include "lib/stringinfo.h"

#include "utils/date.h"
#include "utils/jsonb.h"
#include "utils/numeric.h"
#include <pgtypes.h>

// #include "access/detoast.h"
// #include "access/toast_compression.h"
// #include "catalog/pg_collation.h"
// #include "catalog/pg_type.h"
// #include "common/hashfn.h"
// #include "common/int.h"
// #include "common/unicode_category.h"
// #include "common/unicode_norm.h"
// #include "common/unicode_version.h"
// #include "funcapi.h"
// #include "lib/hyperloglog.h"
// #include "libpq/pqformat.h"
// #include "miscadmin.h"
// #include "nodes/execnodes.h"
// #include "parser/scansup.h"
// #include "port/pg_bswap.h"
// #include "regex/regex.h"
// #include "utils/builtins.h"
// #include "utils/bytea.h"
// #include "utils/guc.h"
// #include "utils/lsyscache.h"
// #include "utils/memutils.h"
// #include "utils/pg_locale.h"
// #include "utils/sortsupport.h"
// #include "utils/varlena.h"

/* To avoid including pg_collation_d */
#define DEFAULT_COLLATION_OID 100
#define C_COLLATION_OID 950
#define POSIX_COLLATION_OID 951

// MEOS force the collation to DEFAULT_COLLATION_OID
#define PG_GET_COLLATION()  DEFAULT_COLLATION_OID

// MEOS force the encoding to PG_SQL_ASCII
#define GetDatabaseEncoding()  PG_SQL_ASCII

/* Borrowed from tupmacs.h */

/*
 * att_addlength_pointer performs the same calculation as att_addlength_datum,
 * but is used when walking a tuple --- attptr is the pointer to the field
 * within the tuple.
 *
 * Note: some callers pass a "char *" pointer for cur_offset.  This is
 * actually perfectly OK, but probably should be cleaned up along with
 * the same practice for att_align_pointer.
 */
#define att_addlength_pointer(cur_offset, attlen, attptr) \
( \
	((attlen) > 0) ? \
	( \
		(cur_offset) + (attlen) \
	) \
	: (((attlen) == -1) ? \
	( \
		(cur_offset) + VARSIZE_ANY(attptr) \
	) \
	: \
	( \
		AssertMacro((attlen) == -2), \
		(cur_offset) + (strlen((char *) (attptr)) + 1) \
	)) \
)

/*****************************************************************************/

typedef struct varlena VarString;

/*
 * State for text_position_* functions.
 */
typedef struct
{
  pg_locale_t locale;      /* collation used for substring matching */
  bool    is_multibyte_char_in_char;  /* need to check char boundaries? */
  bool    greedy;      /* find longest possible substring? */

  char     *str1;      /* haystack string */
  char     *str2;      /* needle string */
  int      len1;      /* string lengths in bytes */
  int      len2;

  /* Skip table for Boyer-Moore-Horspool search algorithm: */
  int      skiptablemask;  /* mask for ANDing with skiptable subscripts */
  int      skiptable[256]; /* skip distance for given mismatched char */

  /*
   * Note that with nondeterministic collations, the length of the last
   * match is not necessarily equal to the length of the "needle" passed in.
   */
  char     *last_match;    /* pointer to last match in 'str1' */
  int      last_match_len; /* length of last match */
  int      last_match_len_tmp; /* same but for internal use */

  /*
   * Sometimes we need to convert the byte position of a match to a
   * character position.  These store the last position that was converted,
   * so that on the next call, we can continue from that point, rather than
   * count characters from the very beginning.
   */
  char     *refpoint;    /* pointer within original haystack string */
  int      refpos;      /* 0-based character offset of the same point */
} TextPositionState;


/*
 * This should be large enough that most strings will fit, but small enough
 * that we feel comfortable putting it on the stack
 */
#define TEXTBUFLEN    1024

static int32 text_length(const text *txt);
static text *text_catenate(const text *txt1, const text *txt2);
static text *text_substring(const text *str, int32 start, int32 length, bool length_not_specified);
static int  text_position(const text *txt1, const text *txt2, Oid collid);
static void text_position_setup(text *txt1, text *txt2, Oid collid, TextPositionState *state);
static bool text_position_next(TextPositionState *state);
static char *text_position_next_internal(char *start_ptr, TextPositionState *state);
static char *text_position_get_match_ptr(TextPositionState *state);
static int  text_position_get_match_pos(TextPositionState *state);
static void text_position_cleanup(TextPositionState *state);
static void check_collation_set(Oid collid);
static void appendStringInfoText(StringInfo str, const text *t);
// static bool text_format_parse_digits(const char **ptr, const char *end_ptr,
  // int *value);
// static const char *text_format_parse_format(const char *start_ptr,
  // const char *end_ptr, int *argpos, int *widthpos, int *flags, int *width);
// static void text_format_string_conversion(StringInfo buf, char conversion,
  // FmgrInfo *typOutputInfo, Datum value, bool isNull, int flags, int width);
// static void text_format_append_string(StringInfo buf, const char *str,
  // int flags, int width);

/*****************************************************************************
 *   CONVERSION ROUTINES EXPORTED FOR USE BY C CODE               *
 *****************************************************************************/

/*
 * cstring_to_text_with_len
 *
 * Same as cstring_to_text except the caller specifies the string length;
 * the string need not be null_terminated.
 */
text *
cstring_to_text_with_len(const char *str, size_t len)
{
  text *result = (text *) palloc(len + VARHDRSZ);
  SET_VARSIZE(result, len + VARHDRSZ);
  memcpy(VARDATA(result), str, len);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Convert a C string into a text
 * @param[in] str String
 * @note Function taken from PostGIS file `lwgeom_in_geojson.c`
 */
text *
cstring_to_text(const char *str)
{
  return cstring_to_text_with_len(str, strlen(str));
}

/**
 * @ingroup meos_base_text
 * @brief Convert a text into a C string
 * @param[in] txt Text
 * @note Function taken from PostGIS file @p lwgeom_in_geojson.c
 */
char *
text_to_cstring(const text *txt)
{
  int len = VARSIZE_ANY_EXHDR(txt);
  char *result = (char *) palloc(len + 1);
  memcpy(result, VARDATA_ANY(txt), len);
  result[len] = '\0';
  return result;
}

/*
 * text_to_cstring_buffer
 *
 * Copy a text value into a caller-supplied buffer of size dst_len.
 *
 * The text string is truncated if necessary to fit.  The result is
 * guaranteed null-terminated (unless dst_len == 0).
 *
 * We support being passed a compressed or toasted text value.
 * This is a bit bogus since such values shouldn't really be referred to as
 * "text *", but it seems useful for robustness.  If we didn't handle that
 * case here, we'd need another routine that did, anyway.
 */
void
text_to_cstring_buffer(const text *src, char *dst, size_t dst_len)
{
  size_t src_len = VARSIZE_ANY_EXHDR(src);
  if (dst_len > 0)
  {
    dst_len--;
    if (dst_len >= src_len)
      dst_len = src_len;
    else          /* ensure truncation is encoding-safe */
      dst_len = pg_mbcliplen(VARDATA_ANY(src), src_len, dst_len);
    memcpy(dst, VARDATA_ANY(src), dst_len);
    dst[dst_len] = '\0';
  }
}

/*****************************************************************************
 *   USER I/O ROUTINES                             *
 *****************************************************************************/

#define VAL(CH)      ((CH) - '0')
#define DIG(VAL)    ((VAL) + '0')

/**
 * @ingroup meos_base_text
 * @brief Return a text from its string representation
 * @note Derived from PostgreSQL function @p textin()
 */
text *
text_in(const char *str)
{
  assert(str);
  return cstring_to_text((char *) str);
}

/**
 * @ingroup meos_base_text
 * @brief Return the string representation of a text value
 * @param[in] txt Text
 * @note Derived from PostgreSQL function @p textout()
 */
char *
text_out(const text *txt)
{
  assert(txt);
  return text_to_cstring(txt);
}

/**
 * @ingroup meos_base_text
 * @brief Copy a text value
 * @param[in] txt Text
 */
text *
text_copy(const text *txt)
{
  assert(txt);
  text *result = palloc(VARSIZE(txt));
  memcpy(result, txt, VARSIZE(txt));
  return result;
}

/**
 * @brief Copy a bytea value
 * @param[in] ba Byte array
 */
bytea *
bytea_copy(const bytea *ba)
{
  assert(ba);
  bytea *result = palloc(VARSIZE(ba));
  memcpy(result, ba, VARSIZE(ba));
  return result;
}

/* ========== PUBLIC ROUTINES ========== */

/**
 * @ingroup meos_base_text
 * @brief Return the logical length of a text value (which is less than the
 * VARSIZE of the text)
 * @note Derived from PostgreSQL function @p textlen()
 */
int32
text_len(const text *txt)
{
  return text_length(txt);
}

/*
 * text_length -
 *  Does the real work for textlen()
 *
 *  This is broken out so it can be called directly by other string processing
 *  functions.  Note that the argument is passed as a Datum, to indicate that
 *  it may still be in compressed form.  We can avoid decompressing it at all
 *  in some cases.
 */
static int32
text_length(const text *txt)
{
  /* fastpath when max encoding length is one */
  if (pg_database_encoding_max_length() == 1)
    // return (toast_raw_datum_size(txt) - VARHDRSZ);
    return (VARSIZE(txt) - VARHDRSZ);
  else
  {
    return pg_mbstrlen_with_len(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
  }
}

/**
 * @ingroup meos_base_text
 * @brief Return the logical length of a text value (which is less than the
 * VARSIZE of the text)
 * @note Derived from PostgreSQL function @p textoctetlen()
 */
int32
text_octetlen(const text *txt)
{
  return (VARSIZE(txt) - VARHDRSZ);
}

/**
 * @ingroup meos_base_text
 * @brief Concatenate two text values
 * @note Derived from PostgreSQL function @p textcat()
 */
text *
text_cat(const text *txt1, const text *txt2)
{
  return text_catenate(txt1, txt2);
}

/*
 * text_catenate
 *  Guts of textcat(), broken out so it can be used by other functions
 *
 * Arguments can be in short-header form, but not compressed or out-of-line
 */
static text *
text_catenate(const text *txt1, const text *txt2)
{
  int len1 = VARSIZE_ANY_EXHDR(txt1);
  int len2 = VARSIZE_ANY_EXHDR(txt2);
  /* paranoia ... probably should throw error instead? */
  if (len1 < 0)
    len1 = 0;
  if (len2 < 0)
    len2 = 0;

  int len = len1 + len2 + VARHDRSZ;
  text *result = (text *) palloc(len);

  /* Set size of result string... */
  SET_VARSIZE(result, len);

  /* Fill data field of result string... */
  char *ptr = VARDATA(result);
  if (len1 > 0)
    memcpy(ptr, VARDATA_ANY(txt1), len1);
  if (len2 > 0)
    memcpy(ptr + len1, VARDATA_ANY(txt2), len2);

  return result;
}

// /*
 // * charlen_to_bytelen()
 // *  Compute the number of bytes occupied by n characters starting at *p
 // *
 // * It is caller's responsibility that there actually are n characters;
 // * the string need not be null-terminated.
 // */
// static int
// charlen_to_bytelen(const char *p, int n)
// {
  // if (pg_database_encoding_max_length() == 1)
  // {
    // /* Optimization for single-byte encodings */
    // return n;
  // }
  // else
  // {
    // const char *s;
    // for (s = p; n > 0; n--)
      // s += pg_mblen(s);
    // return s - p;
  // }
// }

/**
 * @brief Return the slice of a varlena specified by the arguments
 * @note Derived from PostgreSQL function @p detoast_attr_slice
 */
struct varlena *
varlena_slice(struct varlena *attr, int32 sliceoffset, int32 slicelength)
{
  struct varlena *preslice;
  struct varlena *result;
  char *attrdata;
  int32 slicelimit;
  int32 attrsize;

  if (sliceoffset < 0)
    elog(ERROR, "invalid sliceoffset: %d", sliceoffset);

  /*
   * Compute slicelimit = offset + length, or -1 if we must fetch all of the
   * value.  In case of integer overflow, we must fetch all.
   */
  if (slicelength < 0)
    slicelimit = -1;
  else if (pg_add_s32_overflow(sliceoffset, slicelength, &slicelimit))
    slicelength = slicelimit = -1;

  preslice = attr;
  assert(!VARATT_IS_EXTERNAL(preslice));

  if (VARATT_IS_SHORT(preslice))
  {
    attrdata = VARDATA_SHORT(preslice);
    attrsize = VARSIZE_SHORT(preslice) - VARHDRSZ_SHORT;
  }
  else
  {
    attrdata = VARDATA(preslice);
    attrsize = VARSIZE(preslice) - VARHDRSZ;
  }

  /* slicing of datum for compressed cases and plain value */

  if (sliceoffset >= attrsize)
  {
    sliceoffset = 0;
    slicelength = 0;
  }
  else if (slicelength < 0 || slicelimit > attrsize)
    slicelength = attrsize - sliceoffset;

  result = (struct varlena *) palloc(slicelength + VARHDRSZ);
  SET_VARSIZE(result, slicelength + VARHDRSZ);

  memcpy(VARDATA(result), attrdata + sliceoffset, slicelength);

  if (preslice != attr)
    pfree(preslice);

  return result;
}

/*
 * text_substring -
 *  Does the real work for text_substr() and text_substr_no_len()
 *
 *  This is broken out so it can be called directly by other string processing
 *  functions.  Note that the argument is passed as a Datum, to indicate that
 *  it may still be in compressed/toasted form.  We can avoid detoasting all
 *  of it in some cases.
 *
 *  The result is always a freshly palloc'd datum.
 */
static text *
text_substring(const text *txt, int32 start, int32 length,
  bool length_not_specified)
{
  int32 eml = pg_database_encoding_max_length();
  int32 S = start; /* start position */
  int32 S1;        /* adjusted start position */
  int32 L1;        /* adjusted substring length */
  int32 E;         /* end position */

  /*
   * SQL99 says S can be zero or negative (which we don't document), but we
   * still must fetch from the start of the string.
   * https://www.postgresql.org/message-id/170905442373.643.11536838320909376197%40wrigleys.postgresql.org
   */
  S1 = Max(S, 1);

  /* life is easy if the encoding max length is 1 */
  if (eml == 1)
  {
    if (length_not_specified)  /* special case - get length to end of string */
      L1 = -1;
    else if (length < 0)
    {
      /* SQL99 says to throw an error for E < S, i.e., negative length */
      elog(ERROR, "negative substring length not allowed");
      return NULL;
    }
    else if (pg_add_s32_overflow(S, length, &E))
    {
      /*
       * L could be large enough for S + L to overflow, in which case
       * the substring must run to end of string.
       */
      L1 = -1;
    }
    else
    {
      /*
       * A zero or negative value for the end position can happen if the
       * start was negative or one. SQL99 says to return a zero-length
       * string.
       */
      if (E < 1)
        return cstring_to_text("");

      L1 = E - S1;
    }

    /*
     * If the start position is past the end of the string, SQL99 says to
     * return a zero-length string -- DatumGetTextPSlice() will do that
     * for us.  We need only convert S1 to zero-based starting position.
     */
    return varlena_slice((text *) txt, S1 - 1, L1);
  }
  else if (eml > 1)
  {
    /*
     * When encoding max length is > 1, we can't get LC without
     * detoasting, so we'll grab a conservatively large slice now and go
     * back later to do the right thing
     */
    int32 slice_start;
    int32 slice_size;
    int32 slice_strlen;
    text *slice;
    int32 E1;
    int32 i;
    char *p;
    char *s;
    text *ret;

    /*
     * We need to start at position zero because there is no way to know
     * in advance which byte offset corresponds to the supplied start
     * position.
     */
    slice_start = 0;

    if (length_not_specified)  /* special case - get length to end of string */
      slice_size = L1 = -1;
    else if (length < 0)
    {
      /* SQL99 says to throw an error for E < S, i.e., negative length */
      elog(ERROR, "negative substring length not allowed");
      return NULL;
    }
    else if (pg_add_s32_overflow(S, length, &E))
    {
      /*
       * L could be large enough for S + L to overflow, in which case
       * the substring must run to end of string.
       */
      slice_size = L1 = -1;
    }
    else
    {
      /*
       * A zero or negative value for the end position can happen if the
       * start was negative or one. SQL99 says to return a zero-length
       * string.
       */
      if (E < 1)
        return cstring_to_text("");

      /*
       * if E is past the end of the string, the tuple toaster will
       * truncate the length for us
       */
      L1 = E - S1;

      /*
       * Total slice size in bytes can't be any longer than the start
       * position plus substring length times the encoding max length.
       * If that overflows, we can just use -1.
       */
      if (pg_mul_s32_overflow(E, eml, &slice_size))
        slice_size = -1;
    }

    /*
     * If we're working with an untoasted source, no need to do an extra
     * copying step.
     */
    slice = (text *) txt;

    /* see if we got back an empty string */
    if (VARSIZE_ANY_EXHDR(slice) == 0)
    {
      if (slice != txt)
        pfree(slice);
      return cstring_to_text("");
    }

    /* Now we can get the actual length of the slice in MB characters */
    slice_strlen = pg_mbstrlen_with_len(VARDATA_ANY(slice),
      VARSIZE_ANY_EXHDR(slice));

    /*
     * Check that the start position wasn't > slice_strlen. If so, SQL99
     * says to return a zero-length string.
     */
    if (S1 > slice_strlen)
    {
      if (slice != txt)
        pfree(slice);
      return cstring_to_text("");
    }

    /*
     * Adjust L1 and E1 now that we know the slice string length. Again
     * remember that S1 is one based, and slice_start is zero based.
     */
    if (L1 > -1)
      E1 = Min(S1 + L1, slice_start + 1 + slice_strlen);
    else
      E1 = slice_start + 1 + slice_strlen;

    /*
     * Find the start position in the slice; remember S1 is not zero based
     */
    p = VARDATA_ANY(slice);
    for (i = 0; i < S1 - 1; i++)
      p += pg_mblen(p);

    /* hang onto a pointer to our start position */
    s = p;

    /*
     * Count the actual bytes used by the substring of the requested
     * length.
     */
    for (i = S1; i < E1; i++)
      p += pg_mblen(p);

    ret = (text *) palloc(VARHDRSZ + (p - s));
    SET_VARSIZE(ret, VARHDRSZ + (p - s));
    memcpy(VARDATA(ret), s, (p - s));

    if (slice != txt)
      pfree(slice);

    return ret;
  }
  else
    elog(ERROR, "invalid backend encoding: encoding max length < 1");

  /* not reached: suppress compiler warning */
  return NULL;
}

/**
 * @ingroup meos_base_text
 * @brief Return a substring starting at the specified position
 * @param[in] txt String
 * @param[in] start Starting position (one-based)
 * @param[in] length String length
 * @details If the starting position is zero or less, then return from the
 * start of the string adjusting the length to be consistent with the 
 * "negative start" per SQL. If the length is less than zero, return the
 * remaining string.
 * @note Derived from PostgreSQL function @p text_substr()
 */
#if MEOS
text *
text_substr(const text *txt, int32 start, int32 length)
{
  return text_substring(txt, start, length, false);
}
#endif
text *
pg_text_substr(const text *txt, int32 start, int32 length)
{
  return text_substring(txt, start, length, false);
}

/**
 * @ingroup meos_base_text
 * @brief Return a substring starting at the specified position
 * @note Derived from PostgreSQL function @p text_substr_no_len()
 */
#if MEOS
text *
text_substr_no_len(const text *txt, int32 start)
{
  return text_substring(txt, start, -1, true);
}
#endif
text *
pg_text_substr_no_len(const text *txt, int32 start)
{
  return text_substring(txt, start, -1, true);
}

/**
 * @ingroup meos_base_text
 * @brief Replace a specified substring of the first string with the second one
 * @details The SQL standard defines OVERLAY() in terms of substring and
 * concatenation. This code is a direct implementation of what the standard says.
 * @note Existing static PostgreSQL function
 */
text *
text_overlay(const text *txt1, const text *txt2, int from, int count)
{
  /*
   * Check for possible integer-overflow cases.  For negative from, throw a
   * "substring length" error because that's what should be expected
   * according to the spec's definition of OVERLAY().
   */
  if (from <= 0)
  {
    elog(ERROR, "negative substring length not allowed");
    return NULL;
  }
  int sp_from_count;
  if (pg_add_s32_overflow(from, count, &sp_from_count))
  {
    elog(ERROR, "integer out of range");
    return NULL;
  }

  text *s1 = text_substring(txt1, 1, from - 1, false);
  text *s2 = text_substring(txt1, sp_from_count, -1, true);
  text *res = text_catenate(s1, txt2);
  text *result = text_catenate(res, s2);
  pfree(s1); pfree(s2); pfree(res);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Replace a specified substring of the first string with the second one
 * @note Derived from PostgreSQL function @p textoverlay_no_len()
 */
text *
text_overlay_no_len(const text *txt1, const text *txt2, int from)
{
  int len = text_length(txt2);
  return text_overlay(txt1, txt2, from, len);
}

/*
 * text_position -
 *  Does the real work for textpos()
 *
 * Inputs:
 *    txt1 - string to be searched
 *    txt2 - pattern to match within txt1
 * Result:
 *    Character index of the first matched char, starting from 1,
 *    or 0 if no match.
 *
 *  This is broken out so it can be called directly by other string processing
 *  functions.
 */
static int
text_position(const text *txt1, const text *txt2, Oid collid)
{
  check_collation_set(collid);

  /* Empty needle always matches at position 1 */
  if (VARSIZE_ANY_EXHDR(txt2) < 1)
    return 1;

  /* Otherwise, can't match if haystack is shorter than needle */
  // if (VARSIZE_ANY_EXHDR(txt1) < VARSIZE_ANY_EXHDR(txt2) &&
    // pg_newlocale_from_collation(collid)->deterministic)
  if (VARSIZE_ANY_EXHDR(txt1) < VARSIZE_ANY_EXHDR(txt2))
    return 0;

  TextPositionState state;
  text_position_setup((text *) txt1, (text *) txt2, collid, &state);
  /* don't need greedy mode here */
  state.greedy = false;

  int result;
  if (!text_position_next(&state))
    result = 0;
  else
    result = text_position_get_match_pos(&state);
  text_position_cleanup(&state);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return the position of the specified substring
 * @brief Implements the SQL POSITION() function.
 * Ref: A Guide To The SQL Standard, Date & Darwen, 1997
 * @note Derived from PostgreSQL function @p textpos()
 */
int32
text_pos(const text *txt, const text *search)
{
  return (int32) text_position(txt, search, PG_GET_COLLATION());
}

/*
 * text_position_setup, text_position_next, text_position_cleanup -
 *  Component steps of text_position()
 *
 * These are broken out so that a string can be efficiently searched for
 * multiple occurrences of the same pattern.  text_position_next may be
 * called multiple times, and it advances to the next match on each call.
 * text_position_get_match_ptr() and text_position_get_match_pos() return
 * a pointer or 1-based character position of the last match, respectively.
 *
 * The "state" variable is normally just a local variable in the caller.
 *
 * NOTE: text_position_next skips over the matched portion.  For example,
 * searching for "xx" in "xxx" returns only one match, not two.
 */
static void
text_position_setup(text *txt1, text *txt2, Oid collid,
  TextPositionState *state)
{
  int len1 = VARSIZE_ANY_EXHDR(txt1);
  int len2 = VARSIZE_ANY_EXHDR(txt2);

  check_collation_set(collid);
  state->locale = pg_newlocale_from_collation(collid);

  /*
   * Most callers need greedy mode, but some might want to unset this to
   * optimize.
   */
  state->greedy = true;

  Assert(len2 > 0);

  /*
   * Even with a multi-byte encoding, we perform the search using the raw
   * byte sequence, ignoring multibyte issues.  For UTF-8, that works fine,
   * because in UTF-8 the byte sequence of one character cannot contain
   * another character.  For other multi-byte encodings, we do the search
   * initially as a simple byte search, ignoring multibyte issues, but
   * verify afterwards that the match we found is at a character boundary,
   * and continue the search if it was a false match.
   */
  if (pg_database_encoding_max_length() == 1)
    state->is_multibyte_char_in_char = false;
  else if (GetDatabaseEncoding() == PG_UTF8)
    state->is_multibyte_char_in_char = false;
  else
    state->is_multibyte_char_in_char = true;

  state->str1 = VARDATA_ANY(txt1);
  state->str2 = VARDATA_ANY(txt2);
  state->len1 = len1;
  state->len2 = len2;
  state->last_match = NULL;
  state->refpoint = state->str1;
  state->refpos = 0;

  /*
   * Prepare the skip table for Boyer-Moore-Horspool searching.  In these
   * notes we use the terminology that the "haystack" is the string to be
   * searched (txt1) and the "needle" is the pattern being sought (txt2).
   *
   * If the needle is empty or bigger than the haystack then there is no
   * point in wasting cycles initializing the table.  We also choose not to
   * use B-M-H for needles of length 1, since the skip table can't possibly
   * save anything in that case.
   *
   * (With nondeterministic collations, the search is already
   * multibyte-aware, so we don't need this.)
   */
  if (len1 >= len2 && len2 > 1 && state->locale->deterministic)
  {
    int searchlength = len1 - len2;
    int skiptablemask;
    int last;
    int i;
    const char *str2 = state->str2;

    /*
     * First we must determine how much of the skip table to use.  The
     * declaration of TextPositionState allows up to 256 elements, but for
     * short search problems we don't really want to have to initialize so
     * many elements --- it would take too long in comparison to the
     * actual search time.  So we choose a useful skip table size based on
     * the haystack length minus the needle length.  The closer the needle
     * length is to the haystack length the less useful skipping becomes.
     *
     * Note: since we use bit-masking to select table elements, the skip
     * table size MUST be a power of 2, and so the mask must be 2^N-1.
     */
    if (searchlength < 16)
      skiptablemask = 3;
    else if (searchlength < 64)
      skiptablemask = 7;
    else if (searchlength < 128)
      skiptablemask = 15;
    else if (searchlength < 512)
      skiptablemask = 31;
    else if (searchlength < 2048)
      skiptablemask = 63;
    else if (searchlength < 4096)
      skiptablemask = 127;
    else
      skiptablemask = 255;
    state->skiptablemask = skiptablemask;

    /*
     * Initialize the skip table.  We set all elements to the needle
     * length, since this is the correct skip distance for any character
     * not found in the needle.
     */
    for (i = 0; i <= skiptablemask; i++)
      state->skiptable[i] = len2;

    /*
     * Now examine the needle.  For each character except the last one,
     * set the corresponding table element to the appropriate skip
     * distance.  Note that when two characters share the same skip table
     * entry, the one later in the needle must determine the skip
     * distance.
     */
    last = len2 - 1;

    for (i = 0; i < last; i++)
      state->skiptable[(unsigned char) str2[i] & skiptablemask] = last - i;
  }
  return;
}

/*
 * Advance to the next match, starting from the end of the previous match
 * (or the beginning of the string, on first call).  Returns true if a match
 * is found.
 *
 * Note that this refuses to match an empty-string needle.  Most callers
 * will have handled that case specially and we'll never see it here.
 */
static bool
text_position_next(TextPositionState *state)
{
  int needle_len = state->len2;
  char *start_ptr;
  char *matchptr;

  if (needle_len <= 0)
    return false;      /* result for empty pattern */

  /* Start from the point right after the previous match. */
  if (state->last_match)
    start_ptr = state->last_match + state->last_match_len;
  else
    start_ptr = state->str1;

retry:
  matchptr = text_position_next_internal(start_ptr, state);

  if (!matchptr)
    return false;

  /*
   * Found a match for the byte sequence.  If this is a multibyte encoding,
   * where one character's byte sequence can appear inside a longer
   * multi-byte character, we need to verify that the match was at a
   * character boundary, not in the middle of a multi-byte character.
   */
  if (state->is_multibyte_char_in_char && state->locale->deterministic)
  {
    /* Walk one character at a time, until we reach the match. */

    /* the search should never move backwards. */
    Assert(state->refpoint <= matchptr);

    while (state->refpoint < matchptr)
    {
      /* step to next character. */
      state->refpoint += pg_mblen(state->refpoint);
      state->refpos++;

      /*
       * If we stepped over the match's start position, then it was a
       * false positive, where the byte sequence appeared in the middle
       * of a multi-byte character.  Skip it, and continue the search at
       * the next character boundary.
       */
      if (state->refpoint > matchptr)
      {
        start_ptr = state->refpoint;
        goto retry;
      }
    }
  }

  state->last_match = matchptr;
  state->last_match_len = state->last_match_len_tmp;
  return true;
}

/*
 * Subroutine of text_position_next().  This searches for the raw byte
 * sequence, ignoring any multi-byte encoding issues.  Returns the first
 * match starting at 'start_ptr', or NULL if no match is found.
 */
static char *
text_position_next_internal(char *start_ptr, TextPositionState *state)
{
  int haystack_len = state->len1;
  int needle_len = state->len2;
  int skiptablemask = state->skiptablemask;
  const char *haystack = state->str1;
  const char *needle = state->str2;
  const char *haystack_end = &haystack[haystack_len];
  const char *hptr;

  Assert(start_ptr >= haystack && start_ptr <= haystack_end);

  state->last_match_len_tmp = needle_len;

  if (!state->locale->deterministic)
  {
    /*
     * With a nondeterministic collation, we have to use an unoptimized
     * route.  We walk through the haystack and see if at each position
     * there is a substring of the remaining string that is equal to the
     * needle under the given collation.
     *
     * Note, the found substring could have a different length than the
     * needle, including being empty.  Callers that want to skip over the
     * found string need to read the length of the found substring from
     * last_match_len rather than just using the length of their needle.
     *
     * Most callers will require "greedy" semantics, meaning that we need
     * to find the longest such substring, not the shortest.  For callers
     * that don't need greedy semantics, we can finish on the first match.
     */
    const char *result_hptr = NULL;

    hptr = start_ptr;
    while (hptr < haystack_end)
    {
      /*
       * First check the common case that there is a match in the
       * haystack of exactly the length of the needle.
       */
      if (!state->greedy &&
        haystack_end - hptr >= needle_len &&
        pg_strncoll(hptr, needle_len, needle, needle_len, state->locale) == 0)
        return (char *) hptr;

      /*
       * Else check if any of the possible substrings starting at hptr
       * are equal to the needle.
       */
      for (const char *test_end = hptr; test_end < haystack_end; 
        test_end += pg_mblen(test_end))
      {
        if (pg_strncoll(hptr, (test_end - hptr), needle, needle_len,
            state->locale) == 0)
        {
          state->last_match_len_tmp = (test_end - hptr);
          result_hptr = hptr;
          if (!state->greedy)
            break;
        }
      }
      if (result_hptr)
        break;

      hptr += pg_mblen(hptr);
    }

    return (char *) result_hptr;
  }
  else if (needle_len == 1)
  {
    /* No point in using B-M-H for a one-character needle */
    char nchar = *needle;

    hptr = start_ptr;
    while (hptr < haystack_end)
    {
      if (*hptr == nchar)
        return (char *) hptr;
      hptr++;
    }
  }
  else
  {
    const char *needle_last = &needle[needle_len - 1];

    /* Start at startpos plus the length of the needle */
    hptr = start_ptr + needle_len - 1;
    while (hptr < haystack_end)
    {
      /* Match the needle scanning *backward* */
      const char *nptr = needle_last;
      const char *p = hptr;
      while (*nptr == *p)
      {
        /* Matched it all?  If so, return 1-based position */
        if (nptr == needle)
          return (char *) p;
        nptr--, p--;
      }

      /*
       * No match, so use the haystack char at hptr to decide how far to
       * advance.  If the needle had any occurrence of that character
       * (or more precisely, one sharing the same skiptable entry)
       * before its last character, then we advance far enough to align
       * the last such needle character with that haystack position.
       * Otherwise we can advance by the whole needle length.
       */
      hptr += state->skiptable[(unsigned char) *hptr & skiptablemask];
    }
  }

  return NULL;          /* not found */
}

/*
 * Return a pointer to the current match.
 *
 * The returned pointer points into the original haystack string.
 */
static char *
text_position_get_match_ptr(TextPositionState *state)
{
  return state->last_match;
}

/*
 * Return the offset of the current match.
 *
 * The offset is in characters, 1-based.
 */
static int
text_position_get_match_pos(TextPositionState *state)
{
  /* Convert the byte position to char position. */
  state->refpos += pg_mbstrlen_with_len(state->refpoint,
    state->last_match - state->refpoint);
  state->refpoint = state->last_match;
  return state->refpos + 1;
}

/*
 * Reset search state to the initial state installed by text_position_setup.
 *
 * The next call to text_position_next will search from the beginning
 * of the string.
 */
static void
text_position_reset(TextPositionState *state)
{
  state->last_match = NULL;
  state->refpoint = state->str1;
  state->refpos = 0;
}

static void
text_position_cleanup(TextPositionState *state UNUSED)
{
  /* no cleanup needed */
}

static void
check_collation_set(Oid collid)
{
// /* MEOS */
#define InvalidOid ((Oid) 0)

  if (!OidIsValid(collid))
  {
    /*
     * This typically means that the parser could not resolve a conflict
     * of implicit collations, so report it that way.
     */
    elog(ERROR, "could not determine which collation to use for string comparison");
  }
}

/*
 * varstr_cmp()
 *
 * Comparison function for text strings with given lengths, using the
 * appropriate locale. Returns an integer less than, equal to, or greater than
 * zero, indicating whether txt1 is less than, equal to, or greater than txt2.
 *
 * Note: many functions that depend on this are marked leakproof; therefore,
 * avoid reporting the actual contents of the input when throwing errors.
 * All errors herein should be things that can't happen except on corrupt
 * data, anyway; otherwise we will have trouble with indexing strings that
 * would cause them.
 */
int
varstr_cmp(const char *txt1, int len1, const char *txt2, int len2, Oid collid)
{
  check_collation_set(collid);

  int result;
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (mylocale->collate_is_c)
  {
    result = memcmp(txt1, txt2, Min(len1, len2));
    if ((result == 0) && (len1 != len2))
      result = (len1 < len2) ? -1 : 1;
  }
  else
  {
    /*
     * memcmp() can't tell us which of two unequal strings sorts first,
     * but it's a cheap way to tell if they're equal.  Testing shows that
     * memcmp() followed by strcoll() is only trivially slower than
     * strcoll() by itself, so we don't lose much if this doesn't work out
     * very often, and if it does - for example, because there are many
     * equal strings in the input - then we win big by avoiding expensive
     * collation-aware comparisons.
     */
    if (len1 == len2 && memcmp(txt1, txt2, len1) == 0)
      return 0;

    result = pg_strncoll(txt1, len1, txt2, len2, mylocale);

    /* Break tie if necessary. */
    if (result == 0 && mylocale->deterministic)
    {
      result = memcmp(txt1, txt2, Min(len1, len2));
      if ((result == 0) && (len1 != len2))
        result = (len1 < len2) ? -1 : 1;
    }
  }

  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return -1, 0 or 1 depending on whether the first text is less than,
 * equal to, or greater than the second one
 * @note Existing static PostgreSQL function
 */
int
text_cmp(const text *txt1, const text *txt2, Oid collid)
{
  char *a1p = VARDATA_ANY(txt1);
  char *a2p = VARDATA_ANY(txt2);
  int len1 = VARSIZE_ANY_EXHDR(txt1);
  int len2 = VARSIZE_ANY_EXHDR(txt2);
  return varstr_cmp(a1p, len1, a2p, len2, collid);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if two texts are equal
 * @note Derived from PostgreSQL function @p texteq()
 */
bool
text_eq(const text *txt1, const text *txt2)
{
  Oid collid = PG_GET_COLLATION();
  bool result;

  check_collation_set(collid);
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (mylocale->deterministic)
  {
    /*
     * Since we only care about equality or not-equality, we can avoid all
     * the expense of strcoll() here, and just do bitwise comparison.  In
     * fact, we don't even have to do a bitwise comparison if we can show
     * the lengths of the strings are unequal; which might save us from
     * having to detoast one or both values.
     */
    Size len1 = VARSIZE(txt1);
    Size len2 = VARSIZE(txt2);
    if (len1 != len2)
      result = false;
    else
    {
      result = (memcmp(VARDATA_ANY(txt1), VARDATA_ANY(txt2), 
        len1 - VARHDRSZ) == 0);
    }
  }
  else
  {
    result = (text_cmp(txt1, txt2, collid) == 0);
  }

  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return true if two texts are not equal
 * @note Derived from PostgreSQL function @p textne()
 */
bool
text_ne(const text *txt1, const text *txt2)
{
  Oid collid = PG_GET_COLLATION();
  bool result;

  check_collation_set(collid);
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (mylocale->deterministic)
  {
    /* See comment in texteq() */
    Size len1 = VARSIZE(txt1);
    Size len2 = VARSIZE(txt2);
    if (len1 != len2)
      result = true;
    else
    {
      result = (memcmp(VARDATA_ANY(txt1), VARDATA_ANY(txt2), 
        len1 - VARHDRSZ) != 0);
    }
  }
  else
  {
    result = (text_cmp(txt1, txt2, collid) != 0);
  }
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text is less than the second one
 * @note Derived from PostgreSQL function @p text_lt()
 */
#if MEOS
bool
text_lt(const text *txt1, const text *txt2)
{
  return pg_text_lt(txt1, txt2);
}
#endif
bool
pg_text_lt(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) < 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text is less than or equal to the second one
 * @note Derived from PostgreSQL function @p text_le()
 */
#if MEOS
bool
text_le(const text *txt1, const text *txt2)
{
  return pg_text_le(txt1, txt2);
}
#endif
bool
pg_text_le(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) <= 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text is greater than the second one
 * @note Derived from PostgreSQL function @p text_gt()
 */
#if MEOS
bool
text_gt(const text *txt1, const text *txt2)
{
  return pg_text_gt(txt1, txt2);
}
#endif
bool
pg_text_gt(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) > 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text is greater than or equal to the second
 * one
 * @note Derived from PostgreSQL function @p text_ge()
 */
#if MEOS
bool
text_ge(const text *txt1, const text *txt2)
{
  return pg_text_ge(txt1, txt2);
}
#endif
bool
pg_text_ge(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) >= 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text starts with the second one
 * @note Derived from PostgreSQL function @p text_starts_with()
 */
#if MEOS
bool
text_starts_with(const text *txt1, const text *txt2)
{
  return pg_text_starts_with(txt1, txt2);
}
#endif
bool
pg_text_starts_with(const text *txt1, const text *txt2)
{
  Oid collid = PG_GET_COLLATION();
  bool result;

  check_collation_set(collid);
  pg_locale_t mylocale = pg_newlocale_from_collation(collid);
  if (!mylocale->deterministic)
  {
    elog(ERROR,
      "nondeterministic collations are not supported for substring searches");
    return false;
  }

  Size len1 = VARSIZE(txt1);
  Size len2 = VARSIZE(txt2);
  if (len2 > len1)
    result = false;
  else
  {
    text *targ1 = text_substring(txt1, 1, len2, false);
    result = (memcmp(VARDATA_ANY(targ1), VARDATA_ANY(txt2),
      VARSIZE_ANY_EXHDR(txt2)) == 0);
    pfree(targ1);
  }
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return the larger from two texts
 * @note Derived from PostgreSQL function @p text_larger()
 */
#if MEOS
text *
text_larger(const text *txt1, const text *txt2)
{
  return pg_text_larger(txt1, txt2);
}
#endif
text *
pg_text_larger(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) > 0) ? 
    text_copy(txt1) : text_copy(txt2);
}

/**
 * @ingroup meos_base_text
 * @brief Return the smaller from two texts
 * @note Derived from PostgreSQL function @p text_smaller()
 */
#if MEOS
text *
text_smaller(const text *txt1, const text *txt2)
{
  return pg_text_smaller(txt1, txt2);
}
#endif
text *
pg_text_smaller(const text *txt1, const text *txt2)
{
  return (text_cmp(txt1, txt2, PG_GET_COLLATION()) < 0) ? 
    text_copy(txt1) : text_copy(txt2);
}

/*
 * The following operators support character-by-character comparison
 * of text datums, to allow building indexes suitable for LIKE clauses.
 * Note that the regular texteq/textne comparison operators, and regular
 * support functions 1 and 2 with "C" collation are assumed to be
 * compatible with these!
 */
static int
internal_text_pattern_compare(const text *txt1, const text *txt2)
{
  int len1 = VARSIZE_ANY_EXHDR(txt1);
  int len2 = VARSIZE_ANY_EXHDR(txt2);
  int result = memcmp(VARDATA_ANY(txt1), VARDATA_ANY(txt2), Min(len1, len2));
  if (result != 0)
    return result;
  else if (len1 < len2)
    return -1;
  else if (len1 > len2)
    return 1;
  else
    return 0;
}

/**
 * @ingroup meos_base_text
 * @brief Return true if the first text is less than the second one
 * @note Derived from PostgreSQL function @p text_pattern_lt()
 */
#if MEOS
bool
text_pattern_lt(const text *txt1, const text *txt2)
{
  return pg_text_pattern_lt(txt1, txt2);
}
#endif
bool
pg_text_pattern_lt(const text *txt1, const text *txt2)
{
  return (internal_text_pattern_compare(txt1, txt2) < 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if first text text is less than or equal to the second one
 * @note Derived from PostgreSQL function @p text_pattern_le()
 */
#if MEOS
bool
text_pattern_le(const text *txt1, const text *txt2)
{
  return pg_text_pattern_le(txt1, txt2);
}
#endif
bool
pg_text_pattern_le(const text *txt1, const text *txt2)
{
  return (internal_text_pattern_compare(txt1, txt2) <= 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if first text text is greater than or equal to the second one
 * @note Derived from PostgreSQL function @p text_pattern_ge()
 */
#if MEOS
bool
text_pattern_ge(const text *txt1, const text *txt2)
{
  return pg_text_pattern_ge(txt1, txt2);
}
#endif
bool
pg_text_pattern_ge(const text *txt1, const text *txt2)
{
  return (internal_text_pattern_compare(txt1, txt2) >= 0);
}

/**
 * @ingroup meos_base_text
 * @brief Return true if first text text is greater than the second one
 * @note Derived from PostgreSQL function @p text_pattern_gt()
 */
#if MEOS
bool
text_pattern_gt(const text *txt1, const text *txt2)
{
  return pg_text_pattern_gt(txt1, txt2);
}
#endif
bool
pg_text_pattern_gt(const text *txt1, const text *txt2)
{
  return (internal_text_pattern_compare(txt1, txt2) > 0);
}

/*
 * appendStringInfoText
 *
 * Append a text to str.
 * Like appendStringInfoString(str, text_to_cstring(t)) but faster.
 */
static void
appendStringInfoText(StringInfo str, const text *t)
{
  appendBinaryStringInfo(str, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
}

/**
 * @ingroup meos_base_text
 * @brief Return all occurrences of 'old_sub_str' in 'orig_str'
 * with 'new_sub_str' to form 'new_str'
 * @details Return 'orig_str' if 'old_sub_str' == '' or 'orig_str' == ''
 * otherwise returns 'new_str'
 * @note Derived from PostgreSQL function @p replace_text()
 */
text *
text_replace(const text *txt, const text *from, const text *to)
{
  int txt_len = VARSIZE_ANY_EXHDR(txt);
  int from_len = VARSIZE_ANY_EXHDR(from);

  /* Return unmodified source string if empty source or pattern */
  if (txt_len < 1 || from_len < 1)
    return text_copy(txt);

  TextPositionState state;
  text_position_setup((text *) txt, (text *) from, PG_GET_COLLATION(), &state);
  bool found = text_position_next(&state);
  /* When the from is not found, there is nothing to do. */
  if (!found)
  {
    text_position_cleanup(&state);
    return text_copy(txt);
  }

  char *curr_ptr = text_position_get_match_ptr(&state);
  char *start_ptr = VARDATA_ANY(txt);
  StringInfoData str;
  initStringInfo(&str);
  int chunk_len;
  do
  {
    /* copy the data skipped over by last text_position_next() */
    chunk_len = curr_ptr - start_ptr;
    appendBinaryStringInfo(&str, start_ptr, chunk_len);
    appendStringInfoText(&str, to);
    start_ptr = curr_ptr + state.last_match_len;
    found = text_position_next(&state);
    if (found)
      curr_ptr = text_position_get_match_ptr(&state);
  }
  while (found);

  /* copy trailing data */
  chunk_len = ((char *) txt + VARSIZE_ANY(txt)) - start_ptr;
  appendBinaryStringInfo(&str, start_ptr, chunk_len);
  text_position_cleanup(&state);

  text *result = cstring_to_text_with_len(str.data, str.len);
  pfree(str.data);

  return (result);
}

// /*
 // * check_replace_text_has_escape
 // *
 // * Returns 0 if text contains no backslashes that need processing.
 // * Returns 1 if text contains backslashes, but not regexp submatch specifiers.
 // * Returns 2 if text contains regexp submatch specifiers (\1 .. \9).
 // */
// static int
// check_replace_text_has_escape(const text *replace_text)
// {
  // int result = 0;
  // const char *p = VARDATA_ANY(replace_text);
  // const char *p_end = p + VARSIZE_ANY_EXHDR(replace_text);

  // while (p < p_end)
  // {
    // /* Find next escape char, if any. */
    // p = memchr(p, '\\', p_end - p);
    // if (p == NULL)
      // break;
    // p++;
    // /* Note: a backslash at the end doesn't require extra processing. */
    // if (p < p_end)
    // {
      // if (*p >= '1' && *p <= '9')
        // return 2;    /* Found a submatch specifier, so done */
      // result = 1;      /* Found some other sequence, keep looking */
      // p++;
    // }
  // }
  // return result;
// }

/**
 * @ingroup meos_base_text
 * @brief Split string on delimiter and return the n-th item (1-based), 
 * negative counts from end
 * @note Derived from PostgreSQL function @p split_part()
 */
text *
text_split_part(const text *txt, const text *sep, int fldnum)
{
  int txt_len;
  int sep_len;
  TextPositionState state;
  char *start_ptr;
  char *end_ptr;
  text *result;
  bool found;

  /* field number is 1 based */
  if (fldnum == 0)
  {
    elog(ERROR, "field position must not be zero");
    return NULL;
  }

  txt_len = VARSIZE_ANY_EXHDR(txt);
  sep_len = VARSIZE_ANY_EXHDR(sep);

  /* return empty string for empty input string */
  if (txt_len < 1)
    return cstring_to_text("");

  /* handle empty field separator */
  if (sep_len < 1)
  {
    /* if first or last field, return input string, else empty string */
    if (fldnum == 1 || fldnum == -1)
      return text_copy(txt);
    else
      return cstring_to_text("");
  }

  /* find the first field separator */
  text_position_setup((text *) txt, (text *) sep, PG_GET_COLLATION(), &state);
  found = text_position_next(&state);
  /* special case if sep not found at all */
  if (! found)
  {
    text_position_cleanup(&state);
    /* if first or last field, return input string, else empty string */
    if (fldnum == 1 || fldnum == -1)
      return text_copy(txt);
    else
      return (cstring_to_text(""));
  }

  /*
   * take care of a negative field number (i.e. count from the right) by
   * converting to a positive field number; we need total number of fields
   */
  if (fldnum < 0)
  {
    /* we found a sep, so there are at least two fields */
    int numfields = 2;

    while (text_position_next(&state))
      numfields++;

    /* special case of last field does not require an extra pass */
    if (fldnum == -1)
    {
      start_ptr = text_position_get_match_ptr(&state) + state.last_match_len;
      end_ptr = VARDATA_ANY(txt) + txt_len;
      text_position_cleanup(&state);
      return cstring_to_text_with_len(start_ptr, end_ptr - start_ptr);
    }

    /* else, convert fldnum to positive notation */
    fldnum += numfields + 1;

    /* if nonexistent field, return empty string */
    if (fldnum <= 0)
    {
      text_position_cleanup(&state);
      return cstring_to_text("");
    }

    /* reset to pointing at first match, but now with positive fldnum */
    text_position_reset(&state);
    found = text_position_next(&state);
    Assert(found);
  }

  /* identify bounds of first field */
  start_ptr = VARDATA_ANY(txt);
  end_ptr = text_position_get_match_ptr(&state);

  while (found && --fldnum > 0)
  {
    /* identify bounds of next field */
    start_ptr = end_ptr + state.last_match_len;
    found = text_position_next(&state);
    if (found)
      end_ptr = text_position_get_match_ptr(&state);
  }

  text_position_cleanup(&state);

  if (fldnum > 0)
  {
    /* N'th field separator not found */
    /* if last field requested, return it, else empty string */
    if (fldnum == 1)
    {
      int last_len = start_ptr - VARDATA_ANY(txt);
      result = cstring_to_text_with_len(start_ptr,
        txt_len - last_len);
    }
    else
      result = cstring_to_text("");
  }
  else
  {
    /* non-last field requested */
    result = cstring_to_text_with_len(start_ptr, end_ptr - start_ptr);
  }
  return result;
}

/*
 * Workhorse for to_bin, to_oct, and to_hex.  Note that base must be > 1 and <=
 * 16.
 */
static inline text *
convert_to_base(uint64 value, int base)
{
  const char *digits = "0123456789abcdef";

  /* We size the buffer for to_bin's longest possible return value. */
  char buf[sizeof(uint64) * BITS_PER_BYTE];
  char *const end = buf + sizeof(buf);
  char *ptr = end;

  Assert(base > 1);
  Assert(base <= 16);

  do
  {
    *--ptr = digits[value % base];
    value /= base;
  } while (ptr > buf && value);

  return cstring_to_text_with_len(ptr, end - ptr);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int32 to a string containing a base-2 (binary)
 * representation of the number
 * @note Derived from PostgreSQL function @p to_bin32()
 */
text *
int32_to_bin(int32 num)
{
  uint64 value = (uint32) num;
  return convert_to_base(value, 2);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int64 to a string containing a base-2 (binary)
 * representation of the number
 * @note Derived from PostgreSQL function @p to_bin64()
 */
text *
int64_to_bin(int64 num)
{
  uint64 value = (uint64) num;
  return convert_to_base(value, 2);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int32 to a string containing a base-8 (oct)
 * representation of the number
 * @note Derived from PostgreSQL function @p to_oct32()
 */
text *
int32_to_oct(int32 num)
{
  uint64 value = (uint32) num;
  return convert_to_base(value, 8);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int64 to a string containing a base-8 (oct) 
 * representation of the number
 * @note Derived from PostgreSQL function @p to_oct64()
 */
text *
int64_to_oct(int64 num)
{
  uint64 value = (uint64) num;
  return convert_to_base(value, 8);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int32 to a string containing a base-16 (hex) 
 * representation of the number
 * @note Derived from PostgreSQL function @p to_hex32()
 */
text *
int32_to_hex(int32 num)
{
  uint64 value = (uint32) num;
  return convert_to_base(value, 16);
}

/**
 * @ingroup meos_base_text
 * @brief Convert an int64 to a string containing a base-16 (hex) 
 * representation of the number
 * @note Derived from PostgreSQL function @p to_hex64()
 */
text *
int64_to_hex(int64 num)
{
  uint64 value = (uint64) num;
  return convert_to_base(value, 16);
}

/*
 * common code for array_to_text and array_to_text_null functions
 */
static text *
textarr_to_text(text **textarr, int count, const char *sep,
  const char *null_string)
{
  /* if there are no elements, return an empty string */
  if (count == 0)
    return cstring_to_text_with_len("", 0);

  StringInfoData buf;
  initStringInfo(&buf);
  for (int i = 0; i < count; i++)
  {
    text *itemvalue = textarr[i];
    bool printed = false;

    /* Get source element, checking for NULL */
    if (! itemvalue)
    {
      /* if null_string is NULL, we just ignore null elements */
      if (null_string != NULL)
      {
        if (printed)
          appendStringInfo(&buf, "%s%s", sep, null_string);
        else
          appendStringInfoString(&buf, null_string);
        printed = true;
      }
    }
    else
    {
      char *value = text_to_cstring(itemvalue);
      if (printed)
        appendStringInfo(&buf, "%s%s", sep, value);
      else
        appendStringInfoString(&buf, value);
      printed = true;
      pfree(value); // MEOS
    }
  }
  text *result = cstring_to_text_with_len(buf.data, buf.len);
  pfree(buf.data);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Concatenate all arguments
 * @note Derived from PostgreSQL function @p text_concat()
 */
#if MEOS
text *
text_concat(text **textarr, int count)
{
  return textarr_to_text(textarr, count, "", "");
}
#endif
text *
pg_text_concat(text **textarr, int count)
{
  return textarr_to_text(textarr, count, "", "");
}

/**
 * @ingroup meos_base_text
 * @brief Concatenate all but first argument value with separators. The first
 * parameter is used as the separator.
 * @note Derived from PostgreSQL function @p text_concat_ws()
 */
#if MEOS
text *
text_concat_ws(text **textarr, int count, const text *sep)
{
  return pg_text_concat_ws(textarr, count, sep);
}
#endif
text *
pg_text_concat_ws(text **textarr, int count, const text *sep)
{
  assert(sep);
  char *sepstr = text_to_cstring(sep);
  text *result = textarr_to_text(textarr, count, sepstr, "");
  pfree(sepstr);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return the first n characters in the string. When n is negative, 
 * return all but the last |n| characters.
 * @note Derived from PostgreSQL function @p text_left()
 */
#if MEOS
text *
text_left(const text *txt, int n)
{
  return pg_text_left(txt, n);
}
#endif
text *
pg_text_left(const text *txt, int n)
{
  if (n < 0)
  {
    const char *p = VARDATA_ANY(txt);
    int len = VARSIZE_ANY_EXHDR(txt);
    n = pg_mbstrlen_with_len(p, len) + n;
    int rlen = pg_mbcharcliplen(p, len, n);
    return cstring_to_text_with_len(p, rlen);
  }
  else
    return text_substring(txt, 1, n, false);
}

/**
 * @ingroup meos_base_text
 * @brief Return  the last n characters in the string. When n is negative,
 * return all but first |n| characters.
 * @note Derived from PostgreSQL function @p text_right()
 */
#if MEOS
text *
text_right(const text *txt, int n)
{
  return pg_text_right(txt, n);
}
#endif
text *
pg_text_right(const text *txt, int n)
{
  const char *p = VARDATA_ANY(txt);
  int len = VARSIZE_ANY_EXHDR(txt);
  if (n < 0)
    n = -n;
  else
    n = pg_mbstrlen_with_len(p, len) - n;
  int off = pg_mbcharcliplen(p, len, n);
  return cstring_to_text_with_len(p + off, len - off);
}

/**
 * @ingroup meos_base_text
 * @brief Return a text reversed
 * @note Derived from PostgreSQL function @p text_reverse()
 */
#if MEOS
text *
text_reverse(const text *txt)
{
  return pg_text_reverse(txt);
}
#endif
text *
pg_text_reverse(const text *txt)
{
  int len = VARSIZE_ANY_EXHDR(txt);
  text *result = palloc(len + VARHDRSZ);
  char *dst = (char *) VARDATA(result) + len;
  SET_VARSIZE(result, len + VARHDRSZ);

  const char *p = VARDATA_ANY(txt);
  const char *endp = p + len;
  if (pg_database_encoding_max_length() > 1)
  {
    /* multibyte version */
    while (p < endp)
    {
      int sz = pg_mblen(p);
      dst -= sz;
      memcpy(dst, p, sz);
      p += sz;
    }
  }
  else
  {
    /* single byte version */
    while (p < endp)
      *(--dst) = *p++;
  }
  return result;
}

/*
 * Support macros for text_format()
 */
#define TEXT_FORMAT_FLAG_MINUS  0x0001  /* is minus flag present? */

#define ADVANCE_PARSE_POINTER(ptr,end_ptr) \
  do { \
    if (++(ptr) >= (end_ptr)) \
      elog(ERROR, \
        "unterminated format() type specifier"); \
  } while (0)

#if 0 /* NOT USED */
/**
 * @ingroup meos_base_text
 * @brief Returns a formatted string
 * @note Derived from PostgreSQL function @p text_format()
 */
text *
pg_text_format(Datum *elements, int nitems, const text *fmt)
{
  /* When format string is null, immediately return null */
  if (! fmt)
    return NULL;

  int nargs = nitems + 1;

  /* Setup for main loop. */
  const char *start_ptr = VARDATA_ANY(fmt);
  const char *end_ptr = start_ptr + VARSIZE_ANY_EXHDR(fmt);
  StringInfoData str;
  initStringInfo(&str);
  int arg = 1;          /* next argument position to print */

  /* Scan format string, looking for conversion specifiers. */
  for (const char *cp = start_ptr; cp < end_ptr; cp++)
  {
    Oid element_type = InvalidOid;
    Oid prev_type = InvalidOid;
    Oid prev_width_type = InvalidOid;

    /*
     * If it's not the start of a conversion specifier, just copy it to
     * the output buffer.
     */
    if (*cp != '%')
    {
      appendStringInfoCharMacro(&str, *cp);
      continue;
    }

    ADVANCE_PARSE_POINTER(cp, end_ptr);

    /* Easy case: %% outputs a single % */
    if (*cp == '%')
    {
      appendStringInfoCharMacro(&str, *cp);
      continue;
    }

    /* Parse the optional portions of the format specifier */
    int argpos, widthpos, flags, width;
    cp = text_format_parse_format(cp, end_ptr, &argpos, &widthpos, &flags,
      &width);

    /*
     * Next we should see the main conversion specifier.  Whether or not
     * an argument position was present, it's known that at least one
     * character remains in the string at this point.  Experience suggests
     * that it's worth checking that that character is one of the expected
     * ones before we try to fetch arguments, so as to produce the least
     * confusing response to a mis-formatted specifier.
     */
    if (strchr("sIL", *cp) == NULL)
    {
      elog(ERROR, "unrecognized format() type specifier \"%.*s\"",
        pg_mblen(cp), cp);
      return NULL;
    }

    /* If indirect width was specified, get its value */
    if (widthpos >= 0)
    {
      /* Collect the specified or next argument position */
      if (widthpos > 0)
        arg = widthpos;
      if (arg >= nargs)
      {
        elog(ERROR, "too few arguments for format()");
        return NULL;
      }

      /* Get the value and type of the selected argument */
      Datum value = elements[arg - 1];
      bool isNull = (elements[arg - 1] == NULL);
      Oid typid = element_type;
      if (!OidIsValid(typid))
      {
        elog(ERROR, "could not determine data type of format() input");
        return NULL;
      }

      arg++;

      /* We can treat NULL width the same as zero */
      if (isNull)
        width = 0;
      else if (typid == INT4OID)
        width = DatumGetInt32(value);
      else if (typid == INT2OID)
        width = DatumGetInt16(value);
      else
      {
        /* For less-usual datatypes, convert to text then to int */
        if (typid != prev_width_type)
        {
          Oid typoutputfunc;
          bool typIsVarlena;
          getTypeOutputInfo(typid, &typoutputfunc, &typIsVarlena);
          fmgr_info(typoutputfunc, &typoutputinfo_width);
          prev_width_type = typid;
        }

        char *str = OutputFunctionCall(&typoutputinfo_width, value);
        /* pg_strtoint32 will complain about bad data or overflow */
        width = pg_strtoint32(str);
        pfree(str);
      }
    }

    /* Collect the specified or next argument position */
    if (argpos > 0)
      arg = argpos;
    if (arg >= nargs)
    {
      elog(ERROR, "too few arguments for format()");
      return NULL;
    }

    /* Get the value and type of the selected argument */
    value = elements[arg - 1];
    isNull = nulls[arg - 1];
    typid = element_type;
    if (!OidIsValid(typid))
    {
      elog(ERROR, "could not determine data type of format() input");
      return NULL;
    }

    arg++;

    /*
     * Get the appropriate typOutput function, reusing previous one if
     * same type as previous argument.  That's particularly useful in the
     * variadic-array case, but often saves work even for ordinary calls.
     */
    if (typid != prev_type)
    {
      Oid typoutputfunc;
      bool typIsVarlena;

      getTypeOutputInfo(typid, &typoutputfunc, &typIsVarlena);
      fmgr_info(typoutputfunc, &typoutputfinfo);
      prev_type = typid;
    }

    /*
     * And now we can format the value.
     */
    switch (*cp)
    {
      case 's':
      case 'I':
      case 'L':
        text_format_string_conversion(&str, *cp, &typoutputfinfo, value,
          isNull, flags, width);
        break;
      default:
        /* should not get here, because of previous check */
        elog(ERROR, "unrecognized format() type specifier \"%.*s\"",
          pg_mblen(cp), cp);
        return NULL;
    }
  }

  /* Don't need deconstruct_array results anymore. */
  if (elements != NULL)
    pfree(elements);
  if (nulls != NULL)
    pfree(nulls);

  /* Generate results. */
  text *result = cstring_to_text_with_len(str.data, str.len);
  pfree(str.data);

  return result;
}
#endif /* NOT USED */

#if 0 /* NOT USED */
/*
 * Parse contiguous digits as a decimal number.
 *
 * Returns true if some digits could be parsed.
 * The value is returned into *value, and *ptr is advanced to the next
 * character to be parsed.
 *
 * Note parsing invariant: at least one character is known available before
 * string end (end_ptr) at entry, and this is still true at exit.
 */
static bool
text_format_parse_digits(const char **ptr, const char *end_ptr, int *value)
{
  bool found = false;
  const char *cp = *ptr;
  int val = 0;
  while (*cp >= '0' && *cp <= '9')
  {
    int8 digit = (*cp - '0');
    if (unlikely(pg_mul_s32_overflow(val, 10, &val)) ||
      unlikely(pg_add_s32_overflow(val, digit, &val)))
    {
      elog(ERROR, "number is out of range");
      return false;
    }
    ADVANCE_PARSE_POINTER(cp, end_ptr);
    found = true;
  }
  *ptr = cp;
  *value = val;
  return found;
}

/*
 * Parse a format specifier (generally following the SUS printf spec).
 *
 * We have already advanced over the initial '%', and we are looking for
 * [argpos][flags][width]type (but the type character is not consumed here).
 *
 * Inputs are start_ptr (the position after '%') and end_ptr (string end + 1).
 * Output parameters:
 *  argpos: argument position for value to be printed.  -1 means unspecified.
 *  widthpos: argument position for width.  Zero means the argument position
 *      was unspecified (ie, take the next arg) and -1 means no width
 *      argument (width was omitted or specified as a constant).
 *  flags: bitmask of flags.
 *  width: directly-specified width value.  Zero means the width was omitted
 *      (note it's not necessary to distinguish this case from an explicit
 *      zero width value).
 *
 * The function result is the next character position to be parsed, ie, the
 * location where the type character is/should be.
 *
 * Note parsing invariant: at least one character is known available before
 * string end (end_ptr) at entry, and this is still true at exit.
 */
static const char *
text_format_parse_format(const char *start_ptr, const char *end_ptr,
  int *argpos, int *widthpos, int *flags, int *width)
{
  const char *cp = start_ptr;
  int n;

  /* set defaults for output parameters */
  *argpos = -1;
  *widthpos = -1;
  *flags = 0;
  *width = 0;
  /* try to identify first number */
  if (text_format_parse_digits(&cp, end_ptr, &n))
  {
    if (*cp != '$')
    {
      /* Must be just a width and a type, so we're done */
      *width = n;
      return cp;
    }
    /* The number was argument position */
    *argpos = n;
    /* Explicit 0 for argument index is immediately refused */
    if (n == 0)
    {
      elog(ERROR,
        "format specifies argument 0, but arguments are numbered from 1");
      return NULL;
    }
    ADVANCE_PARSE_POINTER(cp, end_ptr);
  }

  /* Handle flags (only minus is supported now) */
  while (*cp == '-')
  {
    *flags |= TEXT_FORMAT_FLAG_MINUS;
    ADVANCE_PARSE_POINTER(cp, end_ptr);
  }

  if (*cp == '*')
  {
    /* Handle indirect width */
    ADVANCE_PARSE_POINTER(cp, end_ptr);
    if (text_format_parse_digits(&cp, end_ptr, &n))
    {
      /* number in this position must be closed by $ */
      if (*cp != '$')
      {
        elog(ERROR, "width argument position must be ended by \"$\"");
        return NULL;
      }
      /* The number was width argument position */
      *widthpos = n;
      /* Explicit 0 for argument index is immediately refused */
      if (n == 0)
      {
        elog(ERROR, 
          "format specifies argument 0, but arguments are numbered from 1");
        return NULL;
      }
      ADVANCE_PARSE_POINTER(cp, end_ptr);
    }
    else
      *widthpos = 0;    /* width's argument position is unspecified */
  }
  else
  {
    /* Check for direct width specification */
    if (text_format_parse_digits(&cp, end_ptr, &n))
      *width = n;
  }

  /* cp should now be pointing at type character */
  return cp;
}

/*
 * Format a %s, %I, or %L conversion
 */
static void
text_format_string_conversion(StringInfo buf, char conversion,
  FmgrInfo *typOutputInfo, Datum value, bool isNull, int flags, int width)
{
  /* Handle NULL arguments before trying to stringify the value. */
  if (isNull)
  {
    if (conversion == 's')
      text_format_append_string(buf, "", flags, width);
    else if (conversion == 'L')
      text_format_append_string(buf, "NULL", flags, width);
    else if (conversion == 'I')
      elog(ERROR, "null values cannot be formatted as an SQL identifier");
    return;
  }

  /* Stringify. */
  char *str = OutputFunctionCall(typOutputInfo, value);

  /* Escape. */
  if (conversion == 'I')
  {
    /* quote_identifier may or may not allocate a new string. */
    text_format_append_string(buf, quote_identifier(str), flags, width);
  }
  else if (conversion == 'L')
  {
    char *qstr = quote_literal_cstr(str);
    text_format_append_string(buf, qstr, flags, width);
    /* quote_literal_cstr() always allocates a new string */
    pfree(qstr);
  }
  else
    text_format_append_string(buf, str, flags, width);

  /* Cleanup. */
  pfree(str);
  return;
}

/*
 * Append str to buf, padding as directed by flags/width
 */
static void
text_format_append_string(StringInfo buf, const char *str, int flags,
  int width)
{
  /* fast path for typical easy case */
  if (width == 0)
  {
    appendStringInfoString(buf, str);
    return;
  }

  bool align_to_left = false;
  if (width < 0)
  {
    /* Negative width: implicit '-' flag, then take absolute value */
    align_to_left = true;
    /* -INT_MIN is undefined */
    if (width <= INT_MIN)
    {
      elog(ERROR, "number is out of range");
      return;
    }
    width = -width;
  }
  else if (flags & TEXT_FORMAT_FLAG_MINUS)
    align_to_left = true;

  int len = pg_mbstrlen(str);
  if (align_to_left)
  {
    /* left justify */
    appendStringInfoString(buf, str);
    if (len < width)
      appendStringInfoSpaces(buf, width - len);
  }
  else
  {
    /* right justify */
    if (len < width)
      appendStringInfoSpaces(buf, width - len);
    appendStringInfoString(buf, str);
  }
  return;
}
#endif /* NOT USED */

/*
 * Unicode support
 */

static UnicodeNormalizationForm
unicode_norm_form_from_string(const char *formstr)
{
  UnicodeNormalizationForm form = -1;

  /*
   * Might as well check this while we're here.
   */
  if (GetDatabaseEncoding() != PG_UTF8)
  {
    elog(ERROR,
      "Unicode normalization can only be performed if server encoding is UTF8");
    return -1;
  }

  if (pg_strcasecmp(formstr, "NFC") == 0)
    form = UNICODE_NFC;
  else if (pg_strcasecmp(formstr, "NFD") == 0)
    form = UNICODE_NFD;
  else if (pg_strcasecmp(formstr, "NFKC") == 0)
    form = UNICODE_NFKC;
  else if (pg_strcasecmp(formstr, "NFKD") == 0)
    form = UNICODE_NFKD;
  else
    elog(ERROR, "invalid normalization form: %s", formstr);

  return form;
}

/**
 * @ingroup meos_base_text
 * @brief Returns version of Unicode used by Postgres in "major.minor" format 
 * (the same format as the Unicode version reported by ICU)
 * @details The third component ("update version") never involves additions to
 * the character repertoire and is unimportant for most purposes.
 * @see https://unicode.org/versions/
 * @note Derived from PostgreSQL function @p unicode_version()
 */
#if MEOS
text *
unicode_version(void)
{
  return pg_unicode_version();
}
#endif /* MEOS */
text *
pg_unicode_version(void)
{
  return cstring_to_text(PG_UNICODE_VERSION);
}

/**
 * @ingroup meos_base_text
 * @brief Returns version of Unicode used by ICU, if enabled; otherwise NULL
 * @note Derived from PostgreSQL function @p icu_unicode_version()
 */
#if MEOS
text *
icu_unicode_version(void)
{
  return pg_icu_unicode_version();
}
#endif /* MEOS */
text *
pg_icu_unicode_version(void)
{
#ifdef USE_ICU
  return cstring_to_text(U_UNICODE_VERSION);
#else
  return NULL;
#endif
}

/**
 * @ingroup meos_base_text
 * @brief Return true if a text contains only assigned Unicode code points
 * @details Requires that the encoding is UTF-8
 * @note Derived from PostgreSQL function @p unicode_assigned()
 */
#if MEOS
bool
unicode_assigned(const text *txt)
{ 
  return pg_unicode_assigned(txt);
}
#endif /* MEOS */
bool
pg_unicode_assigned(const text *txt)
{
  if (GetDatabaseEncoding() != PG_UTF8)
  {
    elog(ERROR,
      "Unicode categorization can only be performed if server encoding is UTF8");
    return false;
  }

  /* convert to pg_wchar */
  int size = pg_mbstrlen_with_len(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
  unsigned char *p = (unsigned char *) VARDATA_ANY(txt);
  for (int i = 0; i < size; i++)
  {
    pg_wchar  uchar = utf8_to_unicode(p);
    int category = unicode_category(uchar);
    if (category == PG_U_UNASSIGNED)
      return false;
    p += pg_utf_mblen(p);
  }
  return true;
}

/**
 * @ingroup meos_base_text
 * @brief Return a Unicode text normalized
 * @note Derived from PostgreSQL function @p unicode_normalize_func()
 */
#if MEOS
text *
unicode_normalize_func(const text *txt, const text *fmt)
{ 
  return pg_unicode_normalize_func(txt, fmt);
}
#endif /* MEOS */
text *
pg_unicode_normalize_func(const text *txt, const text *fmt)
{
  char *formstr = text_to_cstring(fmt);
  UnicodeNormalizationForm form = unicode_norm_form_from_string(formstr);

  /* convert to pg_wchar */
  int size = pg_mbstrlen_with_len(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
  pg_wchar *input_chars = palloc((size + 1) * sizeof(pg_wchar));
  unsigned char *p = (unsigned char *) VARDATA_ANY(txt);
  int i;
  for (i = 0; i < size; i++)
  {
    input_chars[i] = utf8_to_unicode(p);
    p += pg_utf_mblen(p);
  }
  input_chars[i] = (pg_wchar) '\0';
  Assert((char *) p == VARDATA_ANY(txt) + VARSIZE_ANY_EXHDR(txt));

  /* action */
  pg_wchar *output_chars = unicode_normalize(form, input_chars);

  /* convert back to UTF-8 string */
  size = 0;
  for (pg_wchar *wp = output_chars; *wp; wp++)
  {
    unsigned char buf[4];
    unicode_to_utf8(*wp, buf);
    size += pg_utf_mblen(buf);
  }

  text *result = palloc(size + VARHDRSZ);
  SET_VARSIZE(result, size + VARHDRSZ);
  p = (unsigned char *) VARDATA_ANY(result);
  for (pg_wchar *wp = output_chars; *wp; wp++)
  {
    unicode_to_utf8(*wp, p);
    p += pg_utf_mblen(p);
  }
  Assert((char *) p == (char *) result + size + VARHDRSZ);
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return true if a text is in the specified Unicode normalization form
 * @details This is done by converting the string to the specified normal form 
 * and then comparing that to the original string.  To speed that up, we also 
 * apply the "quick check" algorithm specified in UAX #15, which can give a 
 * yes or noanswer for many strings by just scanning the string once.
 *
 * This function should generally be optimized for the case where the string
 * is in fact normalized.  In that case, we'll end up looking at the entire
 * string, so it's probably not worth doing any incremental conversion etc.
 * @note Derived from PostgreSQL function @p unicode_is_normalized()
 */
#if MEOS
bool
unicode_is_normalized(const text *txt, const text *fmt)
{
  return pg_unicode_is_normalized(txt, fmt);
}
#endif /* MEOS */
bool
pg_unicode_is_normalized(const text *txt, const text *fmt)
{
  char *formstr = text_to_cstring(fmt);
  UnicodeNormalizationForm form = unicode_norm_form_from_string(formstr);

  /* convert to pg_wchar */
  int size = pg_mbstrlen_with_len(VARDATA_ANY(txt), VARSIZE_ANY_EXHDR(txt));
  pg_wchar *input_chars = palloc((size + 1) * sizeof(pg_wchar));
  unsigned char *p = (unsigned char *) VARDATA_ANY(txt);
  int i;
  for (i = 0; i < size; i++)
  {
    input_chars[i] = utf8_to_unicode(p);
    p += pg_utf_mblen(p);
  }
  input_chars[i] = (pg_wchar) '\0';
  Assert((char *) p == VARDATA_ANY(txt) + VARSIZE_ANY_EXHDR(txt));

  /* quick check (see UAX #15) */
  UnicodeNormalizationQC quickcheck =
    unicode_is_normalized_quickcheck(form, input_chars);
  if (quickcheck == UNICODE_NORM_QC_YES)
    return true;
  else if (quickcheck == UNICODE_NORM_QC_NO)
    return false;

  /* normalize and compare with original */
  pg_wchar *output_chars = unicode_normalize(form, input_chars);

  int output_size = 0;
  for (pg_wchar *wp = output_chars; *wp; wp++)
    output_size++;

  bool result = (size == output_size) &&
    (memcmp(input_chars, output_chars, size * sizeof(pg_wchar)) == 0);

  return result;
}

/*
 * Check if first n chars are hexadecimal digits
 */
static bool
isxdigits_n(const char *instr, size_t n)
{
  for (size_t i = 0; i < n; i++)
    if (!isxdigit((unsigned char) instr[i]))
      return false;
  return true;
}

static unsigned int
hexval(unsigned char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 0xA;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 0xA;
  elog(ERROR, "invalid hexadecimal digit");
  return 0;          /* not reached */
}

/*
 * Translate string with hexadecimal digits to number
 */
static unsigned int
hexval_n(const char *instr, size_t n)
{
  unsigned int result = 0;
  for (size_t i = 0; i < n; i++)
    result += hexval(instr[i]) << (4 * (n - i - 1));
  return result;
}

/**
 * @ingroup meos_base_text
 * @brief Return a text where the Unicode escape sequences are replaced by
 * Unicode characters
 * @note Derived from PostgreSQL function @p unistr()
 */
#if MEOS
text *
unistr(const text *txt)
{
  return pg_unistr(txt);
}
#endif /* MEOS */
text *
pg_unistr(const text *txt)
{
  pg_wchar pair_first = 0;
  char cbuf[MAX_UNICODE_EQUIVALENT_STRING + 1];
  char *instr = VARDATA_ANY(txt);
  int len = VARSIZE_ANY_EXHDR(txt);

  StringInfoData str;
  initStringInfo(&str);
  while (len > 0)
  {
    if (instr[0] == '\\')
    {
      if (len >= 2 &&
        instr[1] == '\\')
      {
        if (pair_first)
          goto invalid_pair;
        appendStringInfoChar(&str, '\\');
        instr += 2;
        len -= 2;
      }
      else if ((len >= 5 && isxdigits_n(instr + 1, 4)) ||
           (len >= 6 && instr[1] == 'u' && isxdigits_n(instr + 2, 4)))
      {
        pg_wchar  unicode;
        int offset = instr[1] == 'u' ? 2 : 1;
        unicode = hexval_n(instr + offset, 4);
        if (!is_valid_unicode_codepoint(unicode))
        {
          elog(ERROR, "invalid Unicode code point: %04X", unicode);
          return NULL;
        }

        if (pair_first)
        {
          if (is_utf16_surrogate_second(unicode))
          {
            unicode = surrogate_pair_to_codepoint(pair_first, unicode);
            pair_first = 0;
          }
          else
            goto invalid_pair;
        }
        else if (is_utf16_surrogate_second(unicode))
          goto invalid_pair;

        if (is_utf16_surrogate_first(unicode))
          pair_first = unicode;
        else
        {
          pg_unicode_to_server(unicode, (unsigned char *) cbuf);
          appendStringInfoString(&str, cbuf);
        }

        instr += 4 + offset;
        len -= 4 + offset;
      }
      else if (len >= 8 && instr[1] == '+' && isxdigits_n(instr + 2, 6))
      {
        pg_wchar unicode;
        unicode = hexval_n(instr + 2, 6);
        if (!is_valid_unicode_codepoint(unicode))
        {
          elog(ERROR, "invalid Unicode code point: %04X", unicode);
          return NULL;
        }

        if (pair_first)
        {
          if (is_utf16_surrogate_second(unicode))
          {
            unicode = surrogate_pair_to_codepoint(pair_first, unicode);
            pair_first = 0;
          }
          else
            goto invalid_pair;
        }
        else if (is_utf16_surrogate_second(unicode))
          goto invalid_pair;

        if (is_utf16_surrogate_first(unicode))
          pair_first = unicode;
        else
        {
          pg_unicode_to_server(unicode, (unsigned char *) cbuf);
          appendStringInfoString(&str, cbuf);
        }

        instr += 8;
        len -= 8;
      }
      else if (len >= 10 && instr[1] == 'U' && isxdigits_n(instr + 2, 8))
      {
        pg_wchar  unicode;
        unicode = hexval_n(instr + 2, 8);
        if (!is_valid_unicode_codepoint(unicode))
        {
          elog(ERROR, "invalid Unicode code point: %04X", unicode);
          return NULL;
        }

        if (pair_first)
        {
          if (is_utf16_surrogate_second(unicode))
          {
            unicode = surrogate_pair_to_codepoint(pair_first, unicode);
            pair_first = 0;
          }
          else
            goto invalid_pair;
        }
        else if (is_utf16_surrogate_second(unicode))
          goto invalid_pair;

        if (is_utf16_surrogate_first(unicode))
          pair_first = unicode;
        else
        {
          pg_unicode_to_server(unicode, (unsigned char *) cbuf);
          appendStringInfoString(&str, cbuf);
        }
        instr += 10;
        len -= 10;
      }
      else
      {
        elog(ERROR, "invalid Unicode escape");
        return NULL;
      }
    }
    else
    {
      if (pair_first)
        goto invalid_pair;
      appendStringInfoChar(&str, *instr++);
      len--;
    }
  }

  /* unfinished surrogate pair? */
  if (pair_first)
    goto invalid_pair;

  text *result = cstring_to_text_with_len(str.data, str.len);
  pfree(str.data);
  return result;

invalid_pair:
  elog(ERROR, "invalid Unicode surrogate pair");
  return NULL;
}

/*****************************************************************************/