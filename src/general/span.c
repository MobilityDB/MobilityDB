/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @file span.c
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#include "general/span.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER >= 130000
  #include <common/hashfn.h>
#else
  #include <access/hash.h>
#endif
/* MobilityDB */
#include <libmeos.h>
#include "general/pg_call.h"
#include "general/temporal_in.h"
#include "general/temporal_out.h"
#include "general/temporal_parser.h"
#include "general/temporal_util.h"
#include "general/tnumber_mathfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Deconstruct a span
 *
 * @param[in] s Span value
 * @param[out] lower,upper Bounds
 */
void
span_deserialize(const Span *s, SpanBound *lower, SpanBound *upper)
{
  if (lower)
  {
    lower->val = s->lower;
    lower->inclusive = s->lower_inc;
    lower->lower = true;
    lower->spantype = s->spantype;
    lower->basetype = s->basetype;
  }
  if (upper)
  {
    upper->val = s->upper;
    upper->inclusive = s->upper_inc;
    upper->lower = false;
    upper->spantype = s->spantype;
    upper->basetype = s->basetype;
  }
}

#if MEOS
/*
 * @brief Construct a span value from the bounds
 *
 * This does not force canonicalization of the span value.  In most cases,
 * external callers should only be canonicalization functions.
 */
Span *
span_serialize(SpanBound *lower, SpanBound *upper)
{
  assert(lower->basetype == upper->basetype);

  /* If lower bound value is above upper, it's wrong */
  int cmp = datum_cmp2(lower->val, upper->val, lower->basetype,
    upper->basetype);

  if (cmp > 0)
    elog(ERROR, "span lower bound must be less than or equal to span upper bound");

  /* If bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && ! (lower->inclusive && upper->inclusive))
    elog(ERROR, "a span cannot be empty");

  Span *result = span_make(lower->val, upper->val, lower->inclusive,
    upper->inclusive, lower->basetype);
  return result;
}
#endif

/*****************************************************************************/

/**
 * Compare two span boundary points, returning <0, 0, or >0 according to
 * whether b1 is less than, equal to, or greater than b2.
 *
 * The boundaries can be any combination of upper and lower; so it's useful
 * for a variety of operators.
 *
 * The simple case is when b1 and b2 are both inclusive, in which
 * case the result is just a comparison of the values held in b1 and b2.
 *
 * If a bound is exclusive, then we need to know whether it's a lower bound,
 * in which case we treat the boundary point as "just greater than" the held
 * value; or an upper bound, in which case we treat the boundary point as
 * "just less than" the held value.
 *
 * There is only one case where two boundaries compare equal but are not
 * identical: when both bounds are inclusive and hold the same value,
 * but one is an upper bound and the other a lower bound.
 */
int
span_bound_cmp(const SpanBound *b1, const SpanBound *b2)
{
  /* Compare the values */
  int32 result = datum_cmp2(b1->val, b2->val, b1->basetype, b2->basetype);

  /*
   * If the comparison is not equal and the bounds are both inclusive or
   * both exclusive, we're done. If they compare equal, we still have to
   * consider whether the boundaries are inclusive or exclusive.
  */
  if (result == 0)
  {
    if (! b1->inclusive && ! b2->inclusive)
    {
      /* both are exclusive */
      if (b1->lower == b2->lower)
        return 0;
      else
        return b1->lower ? 1 : -1;
    }
    else if (! b1->inclusive)
      return b1->lower ? 1 : -1;
    else if (! b2->inclusive)
      return b2->lower ? -1 : 1;
  }

  return result;
}

/**
 * Comparison function for sorting span bounds.
 */
int
span_bound_qsort_cmp(const void *a1, const void *a2)
{
  SpanBound *b1 = (SpanBound *) a1;
  SpanBound *b2 = (SpanBound *) a2;
  return span_bound_cmp(b1, b2);
}

/**
 * Compare the lower bound of two spans, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_lower_cmp(const Span *a, const Span *b)
{
  int result = datum_cmp2(a->lower, b->lower, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->lower_inc == b->lower_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->lower_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * Compare the upper bound of two spans, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_upper_cmp(const Span *a, const Span *b)
{
  int result = datum_cmp2(a->upper, b->upper, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->upper_inc == b->upper_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->upper_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * @brief Canonicalize discrete spans.
 */
void
span_canonicalize(Span *s)
{
  if (s->basetype == T_INT4)
  {
    if (! s->lower_inc)
    {
      s->lower = Int32GetDatum(DatumGetInt32(s->lower) + 1);
      s->lower_inc = true;
    }

    if (s->upper_inc)
    {
      s->upper = Int32GetDatum(DatumGetInt32(s->upper) + 1);
      s->upper_inc = false;
    }
  }
}

/**
 * @brief Normalize an array of spans
 *
 * The input spans may overlap and may be non contiguous.
 * The normalized spans are new spans that must be freed.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 */
Span **
spanarr_normalize(Span **spans, int count, int *newcount)
{
  /* Sort the spans before normalization */
  spanarr_sort(spans, count);
  int k = 0;
  Span **result = palloc(sizeof(Span *) * count);
  Span *current = spans[0];
  bool isnew = false;
  for (int i = 1; i < count; i++)
  {
    Span *next = spans[i];
    if (overlaps_span_span(current, next) ||
      adjacent_span_span(current, next))
    {
      /* Compute the union of the spans */
      Span *newspan = span_copy(current);
      span_expand(next, newspan);
      if (isnew)
        pfree(current);
      current = newspan;
      isnew = true;
    }
    else
    {
      if (isnew)
        result[k++] = current;
      else
        result[k++] = span_copy(current);
      current = next;
      isnew = false;
    }
  }
  if (isnew)
    result[k++] = current;
  else
    result[k++] = span_copy(current);

  *newcount = k;
  return result;
}

/**
 * Get the bounds of a span as double values.
 *
 * @param[in] s Input span
 * @param[out] xmin, xmax Lower and upper bounds
 */
void
span_bounds(const Span *s, double *xmin, double *xmax)
{
  ensure_tnumber_spantype(s->spantype);
  if (s->spantype == T_INTSPAN)
  {
    *xmin = (double)(DatumGetInt32(s->lower));
    /* intspans are in canonical form so their upper bound is exclusive */
    *xmax = (double)(DatumGetInt32(s->upper) - 1);
  }
  else /* s->spantype == T_FLOATSPAN */
  {
    *xmin = DatumGetFloat8(s->lower);
    *xmax = DatumGetFloat8(s->upper);
  }
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its string representation.
 */
Span *
span_in(char *str, CachedType spantype)
{
  return span_parse(&str, spantype, true);
}

/**
 * Remove the quotes from the string representation of a span
 */
static void
unquote(char *str)
{
  char *last = str;
  while (*str != '\0')
  {
    if (*str != '"')
    {
      *last++ = *str;
    }
    str++;
  }
  *last = '\0';
  return;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the string representation of a span.
 */
char *
span_out(const Span *s)
{
  char *lower = basetype_output(s->basetype, s->lower);
  char *upper = basetype_output(s->basetype, s->upper);
  StringInfoData buf;
  initStringInfo(&buf);
  appendStringInfoChar(&buf, s->lower_inc ? (char) '[' : (char) '(');
  appendStringInfoString(&buf, lower);
  appendStringInfoString(&buf, ", ");
  appendStringInfoString(&buf, upper);
  appendStringInfoChar(&buf, s->upper_inc ? (char) ']' : (char) ')');
  unquote(buf.data);
  pfree(lower); pfree(upper);
  return buf.data;
}

/*****************************************************************************
 * Input/output in WKB and HexWKB format
 *****************************************************************************/

/**
 * Check that we are not about to read off the end of the WKB array
 */
static inline void
wkb_parse_state_check(wkb_parse_state *s, size_t next)
{
  if ((s->pos + next) > (s->wkb + s->wkb_size))
    elog(ERROR, "WKB structure does not match expected size!");
}

/**
 * Take in an unknown span type of WKB type number and ensure it comes out
 * as an extended WKB span type number.
 */
void
span_spantype_from_wkb_state(wkb_parse_state *s, uint16_t wkb_spantype)
{
  switch (wkb_spantype)
  {
    case MOBDB_WKB_T_INTSPAN:
      s->temptype = T_INTSPAN;
      break;
    case MOBDB_WKB_T_FLOATSPAN:
      s->temptype = T_FLOATSPAN;
      break;
    case MOBDB_WKB_T_PERIOD:
      s->temptype = T_PERIOD;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB span type: %d!", wkb_spantype);
      break;
  }
  s->basetype = spantype_basetype(s->temptype);
  return;
}

/**
 * Return the size of a span base value from its WKB representation.
 */
static size_t
basevalue_from_wkb_size(wkb_parse_state *s)
{
  size_t result = 0;
  ensure_span_basetype(s->basetype);
  switch (s->basetype)
  {
    case T_INT4:
      result = sizeof(int);
      break;
    case T_FLOAT8:
      result = sizeof(double);
      break;
    case T_TIMESTAMPTZ:
      result = sizeof(TimestampTz);
      break;
  }
  return result;
}


/**
 * Return a value from its WKB representation.
 */
static Datum
basevalue_from_wkb_state(wkb_parse_state *s)
{
  Datum result;
  ensure_span_basetype(s->basetype);
  switch (s->temptype)
  {
    case T_INTSPAN:
      result = Int32GetDatum(int32_from_wkb_state(s));
      break;
    case T_FLOATSPAN:
      result = Float8GetDatum(double_from_wkb_state(s));
      break;
    case T_PERIOD:
      result = TimestampTzGetDatum(timestamp_from_wkb_state(s));
      break;
    default: /* Error! */
      elog(ERROR, "unknown span type in function basevalue_from_wkb_state: %d",
        s->temptype);
      break;
  }
  return result;
}

/**
 * Return a span from its WKB representation
 */
Span *
span_from_wkb_state(wkb_parse_state *s)
{
  /* Fail when handed incorrect starting byte */
  char wkb_little_endian = byte_from_wkb_state(s);
  if (wkb_little_endian != 1 && wkb_little_endian != 0)
    elog(ERROR, "Invalid endian flag value encountered.");

  /* Check the endianness of our input */
  s->swap_bytes = false;
  /* Machine arch is big endian, request is for little */
  if (MOBDB_IS_BIG_ENDIAN && wkb_little_endian)
    s->swap_bytes = true;
  /* Machine arch is little endian, request is for big */
  else if ((! MOBDB_IS_BIG_ENDIAN) && (! wkb_little_endian))
    s->swap_bytes = true;

  /* Read the span type */
  uint16_t wkb_spantype = (uint16_t) int16_from_wkb_state(s);
  span_spantype_from_wkb_state(s, wkb_spantype);

  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  temporal_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  size_t size = 2 * basevalue_from_wkb_size(s);
  wkb_parse_state_check(s, size);

  /* Read the values and create the span */
  Datum lower = basevalue_from_wkb_state(s);
  Datum upper = basevalue_from_wkb_state(s);
  Span *result = span_make(lower, upper, lower_inc, upper_inc, s->basetype);
  return result;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its Well-Known Binary (WKB)
 * representation.
 */
Span *
span_from_wkb(uint8_t *wkb, int size)
{
  /* Initialize the state appropriately */
  wkb_parse_state s;
  memset(&s, 0, sizeof(wkb_parse_state));
  s.wkb = s.pos = wkb;
  s.wkb_size = size;
  return span_from_wkb_state(&s);
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its HexWKB representation
 */
Span *
span_from_hexwkb(const char *hexwkb)
{
  int hexwkb_len = strlen(hexwkb);
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, hexwkb_len);
  Span *result = span_from_wkb(wkb, hexwkb_len / 2);
  pfree(wkb);
  return result;
}

/*****************************************************************************/

/**
 * Look-up table for hex writer
 */
static char *hexchr = "0123456789ABCDEF";

/**
 * Return the size of the WKB representation of a span base value.
 */
static size_t
basevalue_to_wkb_size(const Span *s)
{
  size_t result = 0;
  ensure_span_basetype(s->basetype);
  switch (s->basetype)
  {
    case T_INT4:
      result = MOBDB_WKB_INT4_SIZE;
      break;
    case T_FLOAT8:
      result = MOBDB_WKB_DOUBLE_SIZE;
      break;
    case T_TIMESTAMPTZ:
      result = MOBDB_WKB_TIMESTAMP_SIZE;
      break;
  }
  return result;
}

/**
 * Return the size in bytes of a span represented in Well-Known Binary
 * (WKB) format
 */
size_t
span_to_wkb_size(const Span *s)
{
  /* Endian flag + bounds flag + spantype + basetype values */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE +
    basevalue_to_wkb_size(s) * 2;
  return size;
}

/**
 * Write into the buffer the span type
 */
static uint8_t *
span_spantype_to_wkb(const Span *s, uint8_t *buf, uint8_t variant)
{
  uint16_t wkb_spantype;
  switch (s->spantype)
  {
    case T_INTSPAN:
      wkb_spantype = MOBDB_WKB_T_INTSPAN;
      break;
    case T_FLOATSPAN:
      wkb_spantype = MOBDB_WKB_T_FLOATSPAN;
      break;
    case T_PERIOD:
      wkb_spantype = MOBDB_WKB_T_PERIOD;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown span type: %d", s->spantype);
      break;
  }
  return int16_to_wkb_buf(wkb_spantype, buf, variant);
}

/**
 * Write into the buffer the flag containing the bounds represented
 * in Well-Known Binary (WKB) format as follows
 * xxxxxxUL
 * x = Unused bits, U = Upper inclusive, L = Lower inclusive
 */
uint8_t *
span_bounds_to_wkb(const Span *s, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_bounds = 0;
  if (s->lower_inc)
    wkb_bounds |= MOBDB_WKB_LOWER_INC;
  if (s->upper_inc)
    wkb_bounds |= MOBDB_WKB_UPPER_INC;
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = (uint8_t) hexchr[wkb_bounds];
    return buf + 2;
  }
  else
  {
    buf[0] = wkb_bounds;
    return buf + 1;
  }
}

/**
 * Write into the buffer the lower and upper bounds of a span represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
lower_upper_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
{
  ensure_span_basetype(s->basetype);
  switch (s->basetype)
  {
    case T_INT4:
      buf = int32_to_wkb_buf(DatumGetInt32(s->lower), buf, variant);
      buf = int32_to_wkb_buf(DatumGetInt32(s->upper), buf, variant);
      break;
    case T_FLOAT8:
      buf = double_to_wkb_buf(DatumGetFloat8(s->lower), buf, variant);
      buf = double_to_wkb_buf(DatumGetFloat8(s->upper), buf, variant);
      break;
    case T_TIMESTAMPTZ:
      buf = timestamp_to_wkb_buf(DatumGetBool(s->lower), buf, variant);
      buf = timestamp_to_wkb_buf(DatumGetBool(s->upper), buf, variant);
      break;
  }
  return buf;
}

/**
 * Write into the buffer a span represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - Basetype int16
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
uint8_t *
span_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the span type */
  buf = span_spantype_to_wkb(s, buf, variant);
  /* Write the span bounds */
  buf = span_bounds_to_wkb(s, buf, variant);
  /* Write the base values */
  buf = lower_upper_to_wkb_buf(s, buf, variant);
  return buf;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a span.
 *
 * @param[in] s Span
 * @param[in] variant Unsigned bitmask value.
 * Accepts either WKB_NDR or WKB_XDR, and WKB_HEX.
 * For example: Variant = WKB_NDR would return the little-endian WKB form.
 * For example: Variant = (WKB_XDR | WKB_HEX) would return the big-endian
 * WKB form as hex-encoded ASCII.
 * @param[out] size_out If supplied, will return the size of the returned
 * memory segment, including the null terminator in the case of ASCII.
 * @note Caller is responsible for freeing the returned array.
 */
uint8_t *
span_as_wkb(const Span *s, uint8_t variant, size_t *size_out)
{
  size_t buf_size;
  uint8_t *buf = NULL;
  uint8_t *wkb_out = NULL;

  /* Initialize output size */
  if (size_out) *size_out = 0;

  /* Calculate the required size of the output buffer */
  buf_size = span_to_wkb_size(s);
  if (buf_size == 0)
  {
    elog(ERROR, "Error calculating output WKB buffer size.");
    return NULL;
  }

  /* Hex string takes twice as much space as binary + a null character */
  if (variant & WKB_HEX)
    buf_size = 2 * buf_size + 1;

  /* If neither or both variants are specified, choose the native order */
  if (! (variant & WKB_NDR || variant & WKB_XDR) ||
    (variant & WKB_NDR && variant & WKB_XDR))
  {
    if (MOBDB_IS_BIG_ENDIAN)
      variant = variant | (uint8_t) WKB_XDR;
    else
      variant = variant | (uint8_t) WKB_NDR;
  }

  /* Allocate the buffer */
  buf = palloc(buf_size);
  if (buf == NULL)
  {
    elog(ERROR, "Unable to allocate %lu bytes for WKB output buffer.", buf_size);
    return NULL;
  }

  /* Retain a pointer to the front of the buffer for later */
  wkb_out = buf;

  /* Write the WKB into the output buffer */
  buf = span_to_wkb_buf(s, buf, variant);

  /* Null the last byte if this is a hex output */
  if (variant & WKB_HEX)
  {
    *buf = '\0';
    buf++;
  }

  /* The buffer pointer should now land at the end of the allocated buffer space. Let's check. */
  if (buf_size != (size_t) (buf - wkb_out))
  {
    elog(ERROR, "Output WKB is not the same size as the allocated buffer.");
    pfree(wkb_out);
    return NULL;
  }

  /* Report output size */
  if (size_out)
    *size_out = buf_size;

  return wkb_out;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the HexWKB representation of a span.
 */
char *
span_as_hexwkb(const Span *s, uint8_t variant, size_t *size)
{
  /* Create WKB hex string */
  size_t hexwkb_size;
  char *result = (char *) span_as_wkb(s, variant | (uint8_t) WKB_HEX,
    &hexwkb_size);
  /* Set the output argument and return */
  *size = hexwkb_size;
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a span from the bounds.
 */
Span *
span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType basetype)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = (Span *) palloc(sizeof(Span));
  span_set(lower, upper, lower_inc, upper_inc, basetype, s);
  return s;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Set a span from the arguments.
 */
void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType basetype, Span *s)
{
  CachedType spantype = basetype_spantype(basetype);
  int cmp = datum_cmp2(lower, upper, basetype, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
    elog(ERROR, "Span lower bound must be less than or equal to span upper bound");

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
    elog(ERROR, "Span cannot be empty");

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(s, 0, sizeof(Span));
  /* Fill in the span */
  s->lower = lower;
  s->upper = upper;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  s->spantype = spantype;
  s->basetype = basetype;
  span_canonicalize(s);
  return;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Return a copy of a span.
 */
Span *
span_copy(const Span *s)
{
  Span *result = (Span *) palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast an element as a span
 */
Span *
elem_to_span(Datum d, CachedType basetype)
{
  ensure_span_basetype(basetype);
  Span *result = span_make(d, d, true, true, basetype);
  return result;
}

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a timestamp as a period
 */
Period *
timestamp_to_period(TimestampTz t)
{
  Period *result = span_make(t, t, true, true, T_TIMESTAMPTZ);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the lower bound of a span
 */
Datum
span_lower(Span *s)
{
  return s->lower;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the upper bound of a span
 */
Datum
span_upper(Span *s)
{
  return s->upper;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return true if the lower bound of a span is inclusive
 */
bool
span_lower_inc(Span *s)
{
  return s->lower_inc != 0;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return true if the upper bound of a span is inclusive
 */
bool
span_upper_inc(Span *s)
{
  return s->upper_inc != 0;
}
#endif

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the width of a span as a double.
 */
double
span_width(const Span *s)
{
  return distance_elem_elem(s->lower, s->upper, s->basetype, s->basetype);
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the duration of a period as an interval.
 */
Interval *
period_duration(const Span *s)
{
  return pg_timestamp_mi(s->upper, s->lower);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_transf
 * @brief Expand the second span with the first one
 */
void
span_expand(const Span *s1, Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp1 = datum_cmp(s2->lower, s1->lower, s1->basetype);
  int cmp2 = datum_cmp(s2->upper, s1->upper, s1->basetype);
  bool lower1 = cmp1 < 0 || (cmp1 == 0 && (s2->lower_inc || ! s1->lower_inc));
  bool upper1 = cmp2 > 0 || (cmp2 == 0 && (s2->upper_inc || ! s1->upper_inc));
  s2->lower = lower1 ? s2->lower : s1->lower;
  s2->lower_inc = lower1 ? s2->lower_inc : s1->lower_inc;
  s2->upper = upper1 ? s2->upper : s1->upper;
  s2->upper_inc = upper1 ? s2->upper_inc : s1->upper_inc;
  return;
}

/**
 * @ingroup libmeos_spantime_transf
 * @brief Shift and/or scale a period by the intervals.
 */
void
period_shift_tscale(const Interval *start, const Interval *duration,
  Period *result)
{
  assert(start != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  bool instant = (result->lower == result->upper);

  if (start != NULL)
  {
    result->lower = pg_timestamp_pl_interval(result->lower, start);
    if (instant)
      result->upper = result->lower;
    else
      result->upper = pg_timestamp_pl_interval(result->upper, start);
  }
  if (duration != NULL && ! instant)
    result->upper = pg_timestamp_pl_interval(result->lower, duration);
  return;
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is equal to the second one.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  if (s1->lower != s2->lower || s1->upper != s2->upper ||
    s1->lower_inc != s2->lower_inc || s1->upper_inc != s2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is different from the second one.
 */
bool
span_ne(const Span *s1, const Span *s2)
{
  return (! span_eq(s1, s2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one.
 *
 * @note Function used for B-tree comparison
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  int cmp = datum_cmp2(s1->lower, s2->lower, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->lower_inc != s2->lower_inc)
    return s1->lower_inc ? -1 : 1;
  cmp = datum_cmp2(s1->upper, s2->upper, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->upper_inc != s2->upper_inc)
    return s1->upper_inc ? 1 : -1;
  return 0;
}

/* Inequality operators using the span_cmp function */

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is less than the second one.
 */
bool
span_lt(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is less than or equal to the
 * second one.
 */
bool
span_le(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp <= 0);
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is greater than or equal to the
 * second one.
 */
bool
span_ge(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp >= 0);
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is greater than the second one.
 */
bool
span_gt(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp > 0);
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 32-bit hash value of a span.
 */
uint32
span_hash(const Span *s)
{
  /* Create flags from the lower_inc and upper_inc values */
  char flags = '\0';
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
  uint32 type_hash = hash_uint32((int32) type);

  /* Apply the hash function to each bound */
  uint32 lower_hash = pg_hashint8(s->lower);
  uint32 upper_hash = pg_hashint8(s->upper);

  /* Merge hashes of flags, type, and bounds */
  uint32 result = DatumGetUInt32(hash_uint32((uint32) flags));
  result ^= type_hash;
  result = (result << 1) | (result >> 31);
  result ^= lower_hash;
  result = (result << 1) | (result >> 31);
  result ^= upper_hash;

  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 64-bit hash value of a span using a seed
 */
uint64
span_hash_extended(const Span *s, Datum seed)
{
  uint64 result;
  char flags = '\0';
  uint64 type_hash;
  uint64 lower_hash;
  uint64 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
  type_hash = DatumGetUInt64(hash_uint32_extended(type, seed));

  /* Apply the hash function to each bound */
  lower_hash = pg_hashint8extended(s->lower, seed);
  upper_hash = pg_hashint8extended(s->upper, seed);

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
    DatumGetInt64(seed)));
  result ^= type_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= lower_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= upper_hash;

  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_in);
/**
 * Input function for periods
 */
PGDLLEXPORT Datum
Span_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid spantypid = PG_GETARG_OID(1);
  Span *result = span_in(input, oid_type(spantypid));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_out);
/**
 * Output function for periods
 */
PGDLLEXPORT Datum
Span_out(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_CSTRING(span_out(s));
}

PG_FUNCTION_INFO_V1(Span_recv);
/**
 * Generic receive function for spans
 */
PGDLLEXPORT Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Span *result = span_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_send);
/*
 * Generic send function for spans
 */
PGDLLEXPORT Datum
Span_send(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(span);
  uint8_t *wkb = span_as_wkb(span, variant, &wkb_size);
  /* Prepare the PostgreSQL bytea return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);
  /* Clean up and return */
  pfree(wkb);
  PG_FREE_IF_COPY(span, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Input in WKB and in HEXWKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_from_wkb);
/**
 * Return a span from its WKB representation
 */
PGDLLEXPORT Datum
Span_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Span *span = span_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(span);
}

PG_FUNCTION_INFO_V1(Span_from_hexwkb);
/**
 * Return a span from its HEXWKB representation
 */
PGDLLEXPORT Datum
Span_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Span *span = span_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(span);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_as_binary);
/**
 * Output a span in WKB format.
 */
PGDLLEXPORT Datum
Span_as_binary(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (! PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text2cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else /* type = XDR */
      variant = variant | (uint8_t) WKB_XDR;
  }

  /* Create WKB hex string */
  size_t wkb_size = VARSIZE_ANY_EXHDR(s);
  uint8_t *wkb = span_as_wkb(s, variant, &wkb_size);

  /* Prepare the PostgreSQL bytea return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Span_as_hexwkb);
/**
 * Output a span in HexWKB format.
 */
PGDLLEXPORT Datum
Span_as_hexwkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (! PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text2cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else
      variant = variant | (uint8_t) WKB_XDR;
  }

  /* Create WKB hex string */
  size_t hexwkb_size;
  char *hexwkb = span_as_hexwkb(s, variant, &hexwkb_size);

  /* Prepare the PgSQL text return type */
  size_t text_size = hexwkb_size - 1 + VARHDRSZ;
  text *result = palloc(text_size);
  memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
  SET_VARSIZE(result, text_size);

  /* Clean up and return */
  pfree(hexwkb);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_constructor2);
/**
 * Construct a span from the two arguments
 */
PGDLLEXPORT Datum
Span_constructor2(PG_FUNCTION_ARGS)
{
  Datum lower = PG_GETARG_DATUM(0);
  Datum upper = PG_GETARG_DATUM(1);
  CachedType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  CachedType basetype = spantype_basetype(spantype);
  Span *span;
  span = span_make(lower, upper, true, false, basetype);
  PG_RETURN_SPAN_P(span);
}


PG_FUNCTION_INFO_V1(Span_constructor4);
/**
 * Construct a span from the four arguments
 */
PGDLLEXPORT Datum
Span_constructor4(PG_FUNCTION_ARGS)
{
  Datum lower = PG_GETARG_DATUM(0);
  Datum upper = PG_GETARG_DATUM(1);
  bool lower_inc = PG_GETARG_BOOL(2);
  bool upper_inc = PG_GETARG_BOOL(3);
  CachedType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  CachedType basetype = spantype_basetype(spantype);
  Span *span;
  span = span_make(lower, upper, lower_inc, upper_inc, basetype);
  PG_RETURN_SPAN_P(span);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Elem_to_span);
/**
 * Cast the timestamp value as a span
 */
PGDLLEXPORT Datum
Elem_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Span *result = elem_to_span(d, basetype);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_to_range);
/**
 * Convert the integer span as a integer range value
 */
PGDLLEXPORT Datum
Span_to_range(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  assert(span->basetype == T_INT4 || span->basetype == T_TIMESTAMPTZ);
  RangeType *range;
  range = range_make(span->lower, span->upper, span->lower_inc,
    span->upper_inc, span->basetype);
  PG_RETURN_POINTER(range);
}

PG_FUNCTION_INFO_V1(Range_to_span);
/**
 * Convert the integer range value as a integer span
 */
PGDLLEXPORT Datum
Range_to_span(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache;
  char flags = range_get_flags(range);
  RangeBound lower, upper;
  bool empty;
  Span *span;

  typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == INT4OID ||
    typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  if (flags & RANGE_EMPTY)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range cannot be empty")));
  if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range bounds cannot be infinite")));

  CachedType basetype = (typcache->rngelemtype->type_id == INT4OID) ?
    T_INT4 : T_TIMESTAMPTZ;
  range_deserialize(typcache, range, &lower, &upper, &empty);
  span = span_make(lower.val, upper.val, lower.inclusive, upper.inclusive,
    basetype);
  PG_RETURN_POINTER(span);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* span -> timestamptz functions */

PG_FUNCTION_INFO_V1(Span_lower);
/**
 * Return the lower bound value
 */
PGDLLEXPORT Datum
Span_lower(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->lower);
}

PG_FUNCTION_INFO_V1(Span_upper);
/**
 * Return the upper bound value
 */
PGDLLEXPORT Datum
Span_upper(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->upper);
}

/* span -> bool functions */

PG_FUNCTION_INFO_V1(Span_lower_inc);
/**
 * Return true if the lower bound value is inclusive
 */
PGDLLEXPORT Datum
Span_lower_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->lower_inc != 0);
}

PG_FUNCTION_INFO_V1(Span_upper_inc);
/**
 * Return true if the upper bound value is inclusive
 */
PGDLLEXPORT Datum
Span_upper_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->upper_inc != 0);
}

PG_FUNCTION_INFO_V1(Span_width);
/**
 * Return the duration of the period
 */
PGDLLEXPORT Datum
Span_width(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  double result = span_width(s);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Period_duration);
/**
 * Return the duration of the period
 */
PGDLLEXPORT Datum
Period_duration(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *result = period_duration(s);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_shift);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Period *result = span_copy(p);
  period_shift_tscale(start, NULL, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tscale);
/**
 * Shift the period  value by the interval
 */
PGDLLEXPORT Datum
Period_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Period *result = span_copy(p);
  period_shift_tscale(NULL, duration, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_shift_tscale);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Period *result = span_copy(p);
  period_shift_tscale(start, duration, result);
  PG_RETURN_POINTER(result);
}

/******************************************************************************/

/**
 * @brief Set the precision of the float span to the number of decimal places.
 */
Span *
floatspan_round(Span *span, Datum size)
{
  /* Set precision of bounds */
  Datum lower = datum_round_float(span->lower, size);
  Datum upper = datum_round_float(span->upper, size);
  /* Create resulting span */
  Span *result = span_make(lower, upper, span->lower_inc, span->upper_inc,
    span->basetype);
  return result;
}

PG_FUNCTION_INFO_V1(Floatspan_round);
/**
 * Set the precision of the float range to the number of decimal places
 */
PGDLLEXPORT Datum
Floatspan_round(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Span *result = floatspan_round(span, size);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_eq);
/**
 * Return true if the first span is equal to the second one
 */
PGDLLEXPORT Datum
Span_eq(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_eq(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_ne);
/**
 * Return true if the first span is different from the second one
 */
PGDLLEXPORT Datum
Span_ne(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ne(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
Span_cmp(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_INT32(span_cmp(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_lt);
/**
 * Return true if the first span is less than the second one
 */
PGDLLEXPORT Datum
Span_lt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_lt(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_le);
/**
 * Return true if the first span is less than or equal to the second one
 */
PGDLLEXPORT Datum
Span_le(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_le(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_ge);
/**
 * Return true if the first span is greater than or equal to the second one
 */
PGDLLEXPORT Datum
Span_ge(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ge(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_gt);
/**
 * Return true if the first span is greater than the second one
 */
PGDLLEXPORT Datum
Span_gt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_gt(s1, s2));
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_hash);
/**
 * Return the 32-bit hash value of a span.
 */
PGDLLEXPORT Datum
Span_hash(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint32 result = span_hash(s);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Span_hash_extended);
/**
 * Return the 64-bit hash value of a span obtained with a seed.
 */
PGDLLEXPORT Datum
Span_hash_extended(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = span_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #if ! MEOS */

/******************************************************************************/
