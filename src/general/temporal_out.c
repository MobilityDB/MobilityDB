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
 * @file temporal_out.c
 * @brief Output of temporal types in WKB
 * format.
 */

#include "general/temporal_out.h"

/* C */
#include <assert.h>
#include <float.h>
// /* PostGIS */
// #if POSTGIS_VERSION_NUMBER >= 30000
  // #include <liblwgeom_internal.h>
// #endif
/* MobilityDB */
#include <libmeos.h>
#include "general/tinstant.h"
#include "general/tinstantset.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
#include "point/tpoint_out.h"
#if ! MEOS
  #include "npoint/tnpoint_static.h"
#endif

/* The following definitions are taken from PostGIS */

#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */
#if POSTGIS_VERSION_NUMBER < 30000
  #define OUT_DOUBLE_BUFFER_SIZE \
    OUT_MAX_DIGS_DOUBLE + OUT_MAX_DOUBLE_PRECISION + 1
#endif

/*****************************************************************************
 * Output in WKB format
 *
 * The MobilityDB binary format builds upon the one of PostGIS. In particular,
 * it reuses many of the flags defined in liblwgeom.h such as WKB_NDR vs WKB_XDR
 * (for little- vs big-endian), WKB_EXTENDED (for the SRID), etc.
 * In addition, we need additional flags such as MOBDB_WKB_LINEAR_INTERP for
 * linear interporation, etc.
 *
 * The binary format obviously depends on the subtype (instant, instant set,
 * ...) and the basetype (int4, float8, text, ...) of the temporal type.
 * The specific binary format is specified in the function corresponding to the
 * subtype below.
 *****************************************************************************/

/**
 * Look-up table for hex writer
 */
static char *hexchr = "0123456789ABCDEF";

/**
 * Return true if the bytes must be swaped dependng of the variant
 */
static inline bool
wkb_swap_bytes(uint8_t variant)
{
  /* If requested variant matches machine arch, we don't have to swap! */
  if (((variant & WKB_NDR) && ! MOBDB_IS_BIG_ENDIAN) ||
      ((! (variant & WKB_NDR)) && MOBDB_IS_BIG_ENDIAN))
    return false;
  return true;
}

/**
 * Write into the buffer the bytes of the value represented in Well-Known
 * Binary (WKB) format
 */
uint8_t *
bytes_to_wkb_buf(char *valptr, size_t size, uint8_t *buf, uint8_t variant)
{
  if (variant & WKB_HEX)
  {
    int swap = wkb_swap_bytes(variant);
    /* Machine/request arch mismatch, so flip byte order */
    for (size_t i = 0; i < size; i++)
    {
      int j = (swap ? size - 1 - i : i);
      uint8_t b = (uint8_t) valptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = (uint8_t) hexchr[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = (uint8_t) hexchr[b & 0x0F];
    }
    return buf + (2 * size);
  }
  else
  {
    /* Machine/request arch mismatch, so flip byte order */
    if (wkb_swap_bytes(variant))
    {
      for (size_t i = 0; i < size; i++)
        buf[i] = (uint8_t) valptr[size - 1 - i];
    }
    /* If machine arch and requested arch match, don't flip byte order */
    else
      memcpy(buf, valptr, size);
    return buf + size;
  }
}

/**
 * Write into the buffer the Endian represented in Well-Known Binary (WKB) format
 */
uint8_t *
endian_to_wkb_buf(uint8_t *buf, uint8_t variant)
{
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = ((variant & WKB_NDR) ? (uint8_t) '1' : (uint8_t) '0');
    return buf + 2;
  }
  else
  {
    buf[0] = ((variant & WKB_NDR) ? (uint8_t) 1 : (uint8_t) 0);
    return buf + 1;
  }
}

/**
 * Write into the buffer a boolean represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
bool_to_wkb_buf(bool b, uint8_t *buf, uint8_t variant)
{
  if (sizeof(bool) != MOBDB_WKB_BYTE_SIZE)
    elog(ERROR, "Machine bool size is not %d bytes!", MOBDB_WKB_BYTE_SIZE);

  char *bptr = (char *)(&b);
  return bytes_to_wkb_buf(bptr, MOBDB_WKB_BYTE_SIZE, buf, variant);
}

/**
 * Write into the buffer the int4 represented in Well-Known Binary (WKB) format
 */
uint8_t *
int16_to_wkb_buf(const int16 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int16) != MOBDB_WKB_INT2_SIZE)
    elog(ERROR, "Machine int16 size is not %d bytes!", MOBDB_WKB_INT2_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MOBDB_WKB_INT2_SIZE, buf, variant);
}

/**
 * Write into the buffer the int4 represented in Well-Known Binary (WKB) format
 */
uint8_t *
int32_to_wkb_buf(const int i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int) != MOBDB_WKB_INT4_SIZE)
    elog(ERROR, "Machine int32 size is not %d bytes!", MOBDB_WKB_INT4_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MOBDB_WKB_INT4_SIZE, buf, variant);
}

/**
 * Write into the buffer the int8 represented in Well-Known Binary (WKB) format
 */
uint8_t *
int64_to_wkb_buf(const int64 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int64) != MOBDB_WKB_INT8_SIZE)
    elog(ERROR, "Machine int64 size is not %d bytes!", MOBDB_WKB_INT8_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MOBDB_WKB_INT8_SIZE, buf, variant);
}

/**
 * Write into the buffer the float64 represented in Well-Known Binary (WKB) format
 */
uint8_t*
double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
  if (sizeof(double) != MOBDB_WKB_DOUBLE_SIZE)
    elog(ERROR, "Machine double size is not %d bytes!", MOBDB_WKB_DOUBLE_SIZE);

  char *dptr = (char *)(&d);
  return bytes_to_wkb_buf(dptr, MOBDB_WKB_DOUBLE_SIZE, buf, variant);
}

/**
 * Write into the buffer the TimestampTz (aka int64) represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
timestamp_to_wkb_buf(const TimestampTz t, uint8_t *buf, uint8_t variant)
{
  if (sizeof(TimestampTz) != MOBDB_WKB_TIMESTAMP_SIZE)
    elog(ERROR, "Machine timestamp size is not %d bytes!",
      MOBDB_WKB_TIMESTAMP_SIZE);

  char *tptr = (char *)(&t);
  return bytes_to_wkb_buf(tptr, MOBDB_WKB_TIMESTAMP_SIZE, buf, variant);
}

/**
 * Write into the buffer a text value represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
text_to_wkb_buf(const text *txt, uint8_t *buf, uint8_t variant)
{
  char *str = text2cstring(txt);
  size_t size = VARSIZE_ANY_EXHDR(txt) + 1;
  buf = int64_to_wkb_buf(size, buf, variant);
  buf = bytes_to_wkb_buf(str, size, buf, variant);
  pfree(str);
  return buf;
}

/**
 * Write into the buffer a network point represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
double2_to_wkb_buf(const double2 *d, uint8_t *buf, uint8_t variant)
{
  buf = double_to_wkb_buf(d->a, buf, variant);
  buf = double_to_wkb_buf(d->b, buf, variant);
  return buf;
}

/**
 * Write into the buffer a network point represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
double3_to_wkb_buf(const double3 *d, uint8_t *buf, uint8_t variant)
{
  buf = double_to_wkb_buf(d->a, buf, variant);
  buf = double_to_wkb_buf(d->b, buf, variant);
  buf = double_to_wkb_buf(d->c, buf, variant);
  return buf;
}

/**
 * Write into the buffer a network point represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
double4_to_wkb_buf(const double4 *d, uint8_t *buf, uint8_t variant)
{
  buf = double_to_wkb_buf(d->a, buf, variant);
  buf = double_to_wkb_buf(d->b, buf, variant);
  buf = double_to_wkb_buf(d->c, buf, variant);
  buf = double_to_wkb_buf(d->d, buf, variant);
  return buf;
}

#if ! MEOS
/**
 * Write into the buffer a network point represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
npoint_to_wkb_buf(const Npoint *np, uint8_t *buf, uint8_t variant)
{
  buf = int64_to_wkb_buf(np->rid, buf, variant);
  buf = double_to_wkb_buf(np->pos, buf, variant);
  return buf;
}
#endif /* ! MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a timestamp.
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
timestamp_as_wkb(const TimestampTz t, uint8_t variant, size_t *size_out)
{
  size_t buf_size;
  uint8_t *buf = NULL;
  uint8_t *wkb_out = NULL;

  /* Initialize output size */
  if (size_out) *size_out = 0;

  /* Calculate the required size of the output buffer */
  buf_size = MOBDB_WKB_TIMESTAMP_SIZE;
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
  buf = timestamp_to_wkb_buf(t, buf, variant);

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

/*****************************************************************************/

/**
 * Return the size of the WKB representation of a base value.
 */
static size_t
basetype_to_wkb_size(Datum value, CachedType basetype)
{
  switch (basetype)
  {
    case T_BOOL:
      return MOBDB_WKB_BYTE_SIZE;
    case T_INT4:
      return MOBDB_WKB_INT4_SIZE;
  #if 0 /* not used */
    case T_INT8:
      return MOBDB_WKB_INT8_SIZE;
  #endif /* not used */
    case T_FLOAT8:
      return MOBDB_WKB_DOUBLE_SIZE;
    case T_TEXT:
      return MOBDB_WKB_INT8_SIZE + VARSIZE_ANY_EXHDR(DatumGetTextP(value)) + 1;
    case T_DOUBLE2:
      return MOBDB_WKB_DOUBLE_SIZE * 2;
    case T_DOUBLE3:
      return MOBDB_WKB_DOUBLE_SIZE * 3;
    case T_DOUBLE4:
      return MOBDB_WKB_DOUBLE_SIZE * 4;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return MOBDB_WKB_BYTE_SIZE;
  #if ! MEOS
    case T_NPOINT:
      return MOBDB_WKB_INT8_SIZE + MOBDB_WKB_DOUBLE_SIZE;
  #endif
    default: /* Error! */
      elog(ERROR, "unknown base type in basetype_to_wkb_size function: %d",
        basetype);
  }
}

/**
 * Return the maximum size in bytes of an array of temporal instant points
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tinstarr_to_wkb_size(const TInstant **instants, int count)
{
  size_t result = 0;
  CachedType basetype = temptype_basetype(instants[0]->temptype);
  for (int i = 0; i < count; i++)
  {
    Datum value = tinstant_value(instants[i]);
    result += basetype_to_wkb_size(value, basetype);
  }
  /* size of the TInstant array */
  result += count * MOBDB_WKB_TIMESTAMP_SIZE;
  return result;
}

/**
 * Return the maximum size in bytes of the temporal instant point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tinstant_to_wkb_size(const TInstant *inst, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(inst->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) inst, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* TInstant */
  size += tinstarr_to_wkb_size(&inst, 1);
  return size;
}

/**
 * Return the maximum size in bytes of the temporal instant set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tinstantset_to_wkb_size(const TInstantSet *ti, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(ti->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) ti, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* Include the number of instants */
  size += MOBDB_WKB_INT4_SIZE;
  int count;
  const TInstant **instants = tinstantset_instants(ti, &count);
  /* Include the TInstant array */
  size += tinstarr_to_wkb_size(instants, count);
  pfree(instants);
  return size;
}

/**
 * Return the maximum size in bytes of the temporal sequence point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tsequence_to_wkb_size(const TSequence *seq, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(seq->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) seq, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* Include the number of instants and the period bounds flag */
  size += MOBDB_WKB_INT4_SIZE + MOBDB_WKB_BYTE_SIZE;
  int count;
  const TInstant **instants = tsequence_instants(seq, &count);
  /* Include the TInstant array */
  size += tinstarr_to_wkb_size(instants, count);
  pfree(instants);
  return size;
}

/**
 * Return the maximum size in bytes of the temporal sequence set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tsequenceset_to_wkb_size(const TSequenceSet *ts, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(ts->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) ts, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* Include the number of sequences */
  size += MOBDB_WKB_INT4_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  size += ts->count * (MOBDB_WKB_INT4_SIZE + MOBDB_WKB_BYTE_SIZE);
  /* Include all the instants of all the sequences */
  int count;
  const TInstant **instants = tsequenceset_instants(ts, &count);
  size += tinstarr_to_wkb_size(instants, count);
  pfree(instants);
  return size;
}

/**
 * Return the maximum size in bytes of the temporal point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
temporal_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  size_t size = 0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    size = tinstant_to_wkb_size((TInstant *) temp, variant);
  else if (temp->subtype == INSTANTSET)
    size = tinstantset_to_wkb_size((TInstantSet *) temp, variant);
  else if (temp->subtype == SEQUENCE)
    size = tsequence_to_wkb_size((TSequence *) temp, variant);
  else /* temp->subtype == SEQUENCESET */
    size = tsequenceset_to_wkb_size((TSequenceSet *) temp, variant);
  return size;
}

/**
 * Write into the buffer the temporal type
 */
static uint8_t *
temporal_temptype_to_wkb(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_temptype;
  switch (temp->temptype)
  {
    case T_TBOOL:
      wkb_temptype = MOBDB_WKB_T_TBOOL;
      break;
    case T_TINT:
      wkb_temptype = MOBDB_WKB_T_TINT;
      break;
    case T_TFLOAT:
      wkb_temptype = MOBDB_WKB_T_TFLOAT;
      break;
    case T_TTEXT:
      wkb_temptype = MOBDB_WKB_T_TTEXT;
      break;
    case T_TDOUBLE2:
      wkb_temptype = MOBDB_WKB_T_TDOUBLE2;
      break;
    case T_TDOUBLE3:
      wkb_temptype = MOBDB_WKB_T_TDOUBLE3;
      break;
    case T_TDOUBLE4:
      wkb_temptype = MOBDB_WKB_T_TDOUBLE4;
      break;
    case T_TGEOMPOINT:
      wkb_temptype = MOBDB_WKB_T_TGEOMPOINT;
      break;
    case T_TGEOGPOINT:
      wkb_temptype = MOBDB_WKB_T_TGEOGPOINT;
      break;
#if ! MEOS
    case T_TNPOINT:
      wkb_temptype = MOBDB_WKB_T_TNPOINT;
      break;
#endif /* ! MEOS */
    default: /* Error! */
      elog(ERROR, "Unknown temporal type (%d)!", temp->temptype);
      break;
  }
  return int16_to_wkb_buf(wkb_temptype, buf, variant);
}

/**
 * Write into the buffer the flag containing the temporal type and
 * other characteristics represented in Well-Known Binary (WKB) format.
 * In binary format it is a byte as follows
 * LSGZxTTT
 * L = Linear, S = SRID, G = Geodetic, Z = has Z, x = unused bit
 * TTT = Temporal subtype with values 1 to 4
 */
static uint8_t *
temporal_flags_to_wkb(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
    wkb_flags |= MOBDB_WKB_LINEAR_INTERP;
  uint8 subtype = temp->subtype;
  if (variant & WKB_HEX)
  {
    buf[0] = (uint8_t) hexchr[wkb_flags >> 4];
    buf[1] = (uint8_t) hexchr[subtype];
    return buf + 2;
  }
  else
  {
    buf[0] = (uint8_t) subtype + wkb_flags;
    return buf + 1;
  }
}

/**
 * Write into the buffer a temporal instant represented in Well-Known Binary
 * (WKB) format as follows
 * - base value
 * - timestamp
 */
static uint8_t *
basevalue_time_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  Datum value = tinstant_value(inst);
  CachedType basetype = temptype_basetype(inst->temptype);
  ensure_temporal_basetype(basetype);
  switch (basetype)
  {
    case T_BOOL:
      buf = bool_to_wkb_buf(DatumGetBool(value), buf, variant);
      break;
    case T_INT4:
      buf = int32_to_wkb_buf(DatumGetInt32(value), buf, variant);
      break;
  #if 0 /* not used */
    case T_INT8:
      buf = int64_to_wkb_buf(DatumGetInt64(value), buf, variant);
      break;
  #endif /* not used */
    case T_FLOAT8:
      buf = double_to_wkb_buf(DatumGetFloat8(value), buf, variant);
      break;
    case T_TEXT:
      buf = text_to_wkb_buf(DatumGetTextP(value), buf, variant);
      break;
    case T_DOUBLE2:
      buf = double2_to_wkb_buf(DatumGetDouble2P(value), buf, variant);
      break;
    case T_DOUBLE3:
      buf = double3_to_wkb_buf(DatumGetDouble3P(value), buf, variant);
      break;
    case T_DOUBLE4:
      buf = double4_to_wkb_buf(DatumGetDouble4P(value), buf, variant);
      break;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      buf = coords_to_wkb_buf(inst, buf, variant);
      break;
  #if ! MEOS
    case T_NPOINT:
      buf = npoint_to_wkb_buf(DatumGetNpointP(value), buf, variant);
      break;
  #endif
    default: /* Error! */
      elog(ERROR, "unknown type_input function for base type: %d", basetype);
  }

  buf = timestamp_to_wkb_buf(inst->t, buf, variant);
  return buf;
}

/**
 * Write into the buffer the temporal instant represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Temporal type
 * - Temporal flags: Linear, SRID, Geodetic, Z, Temporal subtype
 * - SRID (if requested)
 * - Output of a single instant by function basevalue_time_to_wkb_buf
 */
static uint8_t *
tinstant_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = temporal_temptype_to_wkb((Temporal *) inst, buf, variant);
  /* Write the temporal flags */
  buf = temporal_flags_to_wkb((Temporal *) inst, buf, variant);
  return basevalue_time_to_wkb_buf(inst, buf, variant);
}

/**
 * Write into the buffer the temporal instant set point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Output of the instants by function basevalue_time_to_wkb_buf
 */
static uint8_t *
tinstantset_to_wkb_buf(const TInstantSet *ti, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal type */
  buf = temporal_temptype_to_wkb((Temporal *) ti, buf, variant);
  /* Set the temporal flags */
  buf = temporal_flags_to_wkb((Temporal *) ti, buf, variant);
  /* Set the optional SRID for extended variant */
  // if (temporal_wkb_needs_srid((Temporal *) ti, variant))
    // buf = int32_to_wkb_buf(tinstantset_srid(ti), buf, variant);
  /* Set the count */
  buf = int32_to_wkb_buf(ti->count, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    buf = basevalue_time_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Write into the buffer the flag containing the bounds represented
 * in Well-Known Binary (WKB) format as follows
 * xxxxxxUL
 * x = Unused bits, U = Upper inclusive, L = Lower inclusive
 */
uint8_t *
tsequence_bounds_to_wkb(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_bounds = 0;
  if (seq->period.lower_inc)
    wkb_bounds |= MOBDB_WKB_LOWER_INC;
  if (seq->period.upper_inc)
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
 * Write into the buffer the temporal sequence point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Lower/upper inclusive
 * - For each instant
 *   - Output of the instant by function basevalue_time_to_wkb_buf
 */
static uint8_t *
tsequence_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal type */
  buf = temporal_temptype_to_wkb((Temporal *) seq, buf, variant);
  /* Set the temporal flags and interpolation */
  buf = temporal_flags_to_wkb((Temporal *) seq, buf, variant);
  /* Set the optional SRID for extended variant */
  // if (temporal_wkb_needs_srid((Temporal *) seq, variant))
    // buf = int32_to_wkb_buf(tseq_srid(seq), buf, variant);
  /* Set the count */
  buf = int32_to_wkb_buf(seq->count, buf, variant);
  /* Set the period bounds */
  buf = tsequence_bounds_to_wkb(seq, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    buf = basevalue_time_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Write into the buffer the temporal sequence set value represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of sequences
 * - For each sequence
 *   - Number or instants
 *   - Lower/upper inclusive
 *   - For each instant of the sequence
 *      - Output of the instant by function basevalue_time_to_wkb_buf
 */
static uint8_t *
tsequenceset_to_wkb_buf(const TSequenceSet *ts, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal type */
  buf = temporal_temptype_to_wkb((Temporal *) ts, buf, variant);
  /* Set the temporal and interpolation flags */
  buf = temporal_flags_to_wkb((Temporal *) ts, buf, variant);
  /* Set the optional SRID for extended variant */
  // if (temporal_wkb_needs_srid((Temporal *) ts, variant))
    // buf = int32_to_wkb_buf(tpointseqset_srid(ts), buf, variant);
  /* Set the count */
  buf = int32_to_wkb_buf(ts->count, buf, variant);
  /* Set the sequences */
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Set the number of instants */
    buf = int32_to_wkb_buf(seq->count, buf, variant);
    /* Set the period bounds */
    buf = tsequence_bounds_to_wkb(seq, buf, variant);
    /* Set the array of instants */
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      buf = basevalue_time_to_wkb_buf(inst, buf, variant);
    }
  }
  return buf;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the WKB representation of a temporal point.
 *
 * @param[in] temp Temporal value
 * @param[in] variant Unsigned bitmask value. Accepts one of: WKB_ISO, WKB_EXTENDED, WKB_SFSQL.
 * Accepts any of: WKB_NDR, WKB_HEX. For example: Variant = (WKB_ISO | WKB_NDR) would
 * return the little-endian ISO form of WKB. For Example: Variant = (WKB_EXTENDED | WKB_HEX)
 * would return the big-endian extended form of WKB, as hex-encoded ASCII (the "canonical form").
 * @param[out] size_out If supplied, will return the size of the returned
 * memory segment, including the null terminator in the case of ASCII.
 * @note Caller is responsible for freeing the returned array.
 */
uint8_t *
temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  size_t buf_size;
  uint8_t *buf = NULL;
  uint8_t *wkb_out = NULL;

  /* Initialize output size */
  if (size_out) *size_out = 0;

  /* Calculate the required size of the output buffer */
  buf_size = temporal_to_wkb_size(temp, variant);
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
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    buf = tinstant_to_wkb_buf((TInstant *) temp, buf, variant);
  else if (temp->subtype == INSTANTSET)
    buf = tinstantset_to_wkb_buf((TInstantSet *) temp, buf, variant);
  else if (temp->subtype == SEQUENCE)
    buf = tsequence_to_wkb_buf((TSequence *) temp, buf, variant);
  else /* temp->subtype == SEQUENCESET */
    buf = tsequenceset_to_wkb_buf((TSequenceSet *) temp, buf, variant);

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
 * @ingroup libmeos_temporal_input_output
 * @brief Return the HexEWKB representation of a temporal type.
 * @note This will have 'SRID=#;'
 */
char *
temporal_as_hexewkb(const Temporal *temp, uint8_t variant, size_t *size)
{
  size_t hexwkb_size;
  /* Create WKB hex string */
  char *result = (char *) temporal_as_wkb(temp,
    variant | (uint8_t) WKB_EXTENDED | (uint8_t) WKB_HEX, &hexwkb_size);

  *size = hexwkb_size;
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************
 * Output in WKB format
 *
 * The format of the MobilityDB binary format builds upon the one of PostGIS.
 * In particular, many of the flags defined in liblwgeom.h such as WKB_NDR vs
 * WKB_XDR (for little- vs big-endian), WKB_EXTENDED (for the SRID), etc.
 * In addition, we need additional flags such as MOBDB_WKB_LINEAR_INTERP for
 * linear interporation, etc.
 *
 * The binary format obviously depends on the subtype of the temporal type
 * (instant, instant set, ...). The specific binary format is specified in
 * the function corresponding to the subtype below.
 *****************************************************************************/

/**
 * Ensure that string represents a valid endian flag
 */
void
ensure_valid_endian_flag(const char *endian)
{
  if (strncasecmp(endian, "ndr", 3) != 0 && strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  return;
}

/**
 * Output the temporal type in WKB or EWKB format
 */
Datum
temporal_as_binary_ext(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
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
  size_t wkb_size = VARSIZE_ANY_EXHDR(temp);
  uint8_t *wkb = extended ?
    temporal_as_wkb(temp, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    temporal_as_wkb(temp, variant, &wkb_size);

  /* Prepare the PostgreSQL bytea return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_as_binary);
/**
 * Output a temporal value in WKB format.
 */
PGDLLEXPORT Datum
Temporal_as_binary(PG_FUNCTION_ARGS)
{
  return temporal_as_binary_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Temporal_as_hexwkb);
/**
 * Output the temporal point in HexEWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
Temporal_as_hexwkb(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  uint8_t variant = 0;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
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
  char *hexwkb = temporal_as_hexewkb(temp, variant, &hexwkb_size);

  /* Prepare the PgSQL text return type */
  size_t text_size = hexwkb_size - 1 + VARHDRSZ;
  text *result = palloc(text_size);
  memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
  SET_VARSIZE(result, text_size);

  /* Clean up and return */
  pfree(hexwkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
