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
 * @file temporal_wkb_out.c
 * @brief Output of temporal types in WKB, EWKB, and HexWKB format.
 */

#include "general/temporal_out.h"

/* C */
#include <assert.h>
#include <float.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/tinstant.h"
#include "general/tinstantset.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
#include "point/tpoint_spatialfuncs.h"
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
 * In addition, additional flags are needed such as MOBDB_WKB_LINEAR_INTERP for
 * linear interporation, etc.
 *
 * - For box types, the format depends on the existing dimensions (X, Z, T).
 * - For span types, the format depends on the base type (int4, float8, ...).
 * - For temporal types, the binary format depends on the subtype
 *   (instant, instant set, ...) and the basetype (int4, float8, text, ...).
 *****************************************************************************/

/*****************************************************************************
 * Determine the size of the WKB representation of the various types
 *****************************************************************************/

/**
 * Return the size of the WKB representation of a span base value.
 */
static size_t
span_basevalue_to_wkb_size(const Span *s)
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
    span_basevalue_to_wkb_size(s) * 2;
  return size;
}

/*****************************************************************************/

/**
 * Return the size in bytes of a timestamp set represented in Well-Known Binary
 * (WKB) format
 */
static size_t
timestampset_to_wkb_size(const TimestampSet *ts)
{
  /* Endian flag + count + timestamps */
  size_t size = MOBDB_WKB_BYTE_SIZE + MOBDB_WKB_INT4_SIZE +
    MOBDB_WKB_TIMESTAMP_SIZE * ts->count;
  return size;
}

/*****************************************************************************/

/**
 * Return the size in bytes of a period set represented in Well-Known Binary
 * (WKB) format
 */
static size_t
periodset_to_wkb_size(const PeriodSet *ps)
{
  /* Endian flag + count + periods */
  size_t size = MOBDB_WKB_BYTE_SIZE + MOBDB_WKB_INT4_SIZE +
    span_to_wkb_size((Span *) &ps->elems[0]) * ps->count;
  return size;
}

/*****************************************************************************/

/**
 * Return the size in bytes of a temporal box represented in Well-Known Binary
 * (WKB) format
 */
static size_t
tbox_to_wkb_size(const TBOX *box)
{
  /* Endian flag + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2;
  /* If there is a value dimension */
  if (MOBDB_FLAGS_GET_X(box->flags))
    size += MOBDB_WKB_DOUBLE_SIZE * 2;
  /* If there is a time dimension */
  if (MOBDB_FLAGS_GET_T(box->flags))
    size += MOBDB_WKB_DOUBLE_SIZE * 2;
  return size;
}

/*****************************************************************************/

/**
 * Return true if the spatiotemporal box needs to output the SRID
 */
static bool
stbox_wkb_needs_srid(const STBOX *box)
{
  /* Add an SRID if the box has one */
  if (box->srid != SRID_UNKNOWN)
    return true;
  return false;
}

/**
 * Return the size in bytes of a spatiotemporal box represented in Well-Known
 * Binary (WKB) format
 */
static size_t
stbox_to_wkb_size(const STBOX *box)
{
  /* Endian flag + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2;
  /* If there is a value dimension */
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    if (stbox_wkb_needs_srid(box))
      size += MOBDB_WKB_INT4_SIZE;
    size += MOBDB_WKB_DOUBLE_SIZE * 4;
    if (MOBDB_FLAGS_GET_Z(box->flags))
      size += MOBDB_WKB_DOUBLE_SIZE * 2;
  }
  /* If there is a time dimension */
  if (MOBDB_FLAGS_GET_T(box->flags))
    size += MOBDB_WKB_DOUBLE_SIZE * 2;
  return size;
}

/*****************************************************************************/

/**
 * Return the size of the WKB representation of a base value.
 */
static size_t
temporal_basevalue_to_wkb_size(Datum value, CachedType basetype, int16 flags)
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
    {
      int dims = MOBDB_FLAGS_GET_Z(flags) ? 3 : 2;
      return dims * MOBDB_WKB_DOUBLE_SIZE;
    }
  #if ! MEOS
    case T_NPOINT:
      return MOBDB_WKB_INT8_SIZE + MOBDB_WKB_DOUBLE_SIZE;
  #endif
    default: /* Error! */
      elog(ERROR, "Unknown base type: %d",
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
    result += temporal_basevalue_to_wkb_size(value, basetype,
      instants[i]->flags);
  }
  /* size of the TInstant array */
  result += count * MOBDB_WKB_TIMESTAMP_SIZE;
  return result;
}

/**
 * Return true if the temporal point needs to output the SRID
 */
bool
tpoint_wkb_needs_srid(const Temporal *temp, uint8_t variant)
{
  /* Add an SRID if the WKB form is extended and if the temporal point has one */
  if ((variant & WKB_EXTENDED) && tpoint_srid(temp) != SRID_UNKNOWN)
    return true;

  /* Everything else doesn't get an SRID */
  return false;
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
tinstantset_to_wkb_size(const TInstantSet *is, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(is->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) is, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* Include the number of instants */
  size += MOBDB_WKB_INT4_SIZE;
  int count;
  const TInstant **instants = tinstantset_instants(is, &count);
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
tsequenceset_to_wkb_size(const TSequenceSet *ss, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MOBDB_WKB_BYTE_SIZE * 2 + MOBDB_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(ss->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) ss, variant))
    size += MOBDB_WKB_INT4_SIZE;
  /* Include the number of sequences */
  size += MOBDB_WKB_INT4_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  size += ss->count * (MOBDB_WKB_INT4_SIZE + MOBDB_WKB_BYTE_SIZE);
  /* Include all the instants of all the sequences */
  int count;
  const TInstant **instants = tsequenceset_instants(ss, &count);
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

/*****************************************************************************/

/**
 * Return the size of the WKB representation of a value.
 */
static size_t
datum_to_wkb_size(Datum value, CachedType type, uint8_t variant)
{
  size_t result;
  switch (type)
  {
    case T_INTSPAN:
    case T_FLOATSPAN:
    case T_PERIOD:
      result = span_to_wkb_size((Span *) DatumGetPointer(value));
      break;
    case T_TIMESTAMPSET:
      result = timestampset_to_wkb_size((TimestampSet *) DatumGetPointer(value));
      break;
    case T_PERIODSET:
      result = periodset_to_wkb_size((PeriodSet *) DatumGetPointer(value));
      break;
    case T_TBOX:
      result = tbox_to_wkb_size((TBOX *) DatumGetPointer(value));
      break;
    case T_STBOX:
      result = stbox_to_wkb_size((STBOX *) DatumGetPointer(value));
      break;
    case T_TBOOL:
    case T_TINT:
    case T_TFLOAT:
    case T_TTEXT:
    case T_TDOUBLE2:
    case T_TDOUBLE3:
    case T_TDOUBLE4:
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
#if ! MEOS
    case T_TNPOINT:
#endif /* ! MEOS */
      result = temporal_to_wkb_size((Temporal *) DatumGetPointer(value),
        variant);
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB type: %d", type);
      break;
  }
  return result;
}

/*****************************************************************************
 * Write into the buffer the WKB representation of the various types
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

/**
 * Write into the buffer the coordinates of the temporal instant point
 * represented in Well-Known Binary (WKB) format as follows
 * - 2 or 3 doubles for the coordinates depending on whether there is Z
 * - 1 timestamp
 */
uint8_t *
coords_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  if (MOBDB_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *point = datum_point3dz_p(tinstant_value(inst));
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
    buf = double_to_wkb_buf(point->z, buf, variant);
  }
  else
  {
    const POINT2D *point = datum_point2d_p(tinstant_value(inst));
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
  }
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
 * Write into the buffer the span type
 */
static uint8_t *
span_spantype_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
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
span_bounds_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
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
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(s->lower), buf, variant);
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(s->upper), buf, variant);
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
  buf = span_spantype_to_wkb_buf(s, buf, variant);
  /* Write the span bounds */
  buf = span_bounds_to_wkb_buf(s, buf, variant);
  /* Write the base values */
  buf = lower_upper_to_wkb_buf(s, buf, variant);
  return buf;
}

/*****************************************************************************/

/**
 * Write into the buffer a timestamp set represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - int32 stating the number of timestamps
 * - Timestamps
 */
static uint8_t *
timestampset_to_wkb_buf(const TimestampSet *ts, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(ts->count, buf, variant);
  /* Write the timestamps */
  for (int i = 0; i < ts->count; i++)
    buf = timestamp_to_wkb_buf(ts->elems[i], buf, variant);
  /* Write the temporal dimension if any */
  return buf;
}

/*****************************************************************************/

/**
 * Write into the buffer a period set represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - int32 stating the number of periods
 * - Periods
 */
static uint8_t *
periodset_to_wkb_buf(const PeriodSet *ps, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(ps->count, buf, variant);
  /* Write the periods */
  for (int i = 0; i < ps->count; i++)
    buf = span_to_wkb_buf((Period *) &ps->elems[i], buf, variant);
  /* Write the temporal dimension if any */
  return buf;
}

/*****************************************************************************/

/**
 * Write into the buffer the flag of a temporal box represented in
 * Well-Known Binary (WKB) format. It is a byte as follows
 * xxxxxxTX
 * T = has T, X = has X, x = unused bit
 */
static uint8_t *
tbox_to_wkb_flags_buf(const TBOX *box, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MOBDB_FLAGS_GET_X(box->flags))
    wkb_flags |= MOBDB_WKB_XFLAG;
  if (MOBDB_FLAGS_GET_T(box->flags))
    wkb_flags |= MOBDB_WKB_TFLAG;
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = (uint8_t) hexchr[wkb_flags];
    return buf + 2;
  }
  else
  {
    buf[0] = wkb_flags;
    return buf + 1;
  }
}

/**
 * Write into the buffer a temporal box represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - Flag byte stating whether the are a value and/or a time dimensions
 * - Output the 2 doubles for the value dimension (if there is one) and the
 *   2 timestamps for the time dimension (if there is one)
 */
static uint8_t *
tbox_to_wkb_buf(const TBOX *box, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = tbox_to_wkb_flags_buf(box, buf, variant);
  /* Write the value dimension if any */
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    buf = double_to_wkb_buf(box->xmin, buf, variant);
    buf = double_to_wkb_buf(box->xmax, buf, variant);
  }
  /* Write the temporal dimension if any */
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    buf = timestamp_to_wkb_buf(box->tmin, buf, variant);
    buf = timestamp_to_wkb_buf(box->tmax, buf, variant);
  }
  return buf;
}

/*****************************************************************************/

/**
 * Write into the buffer the flag of a spatiotemporal box represented in
 * Well-Known Binary (WKB) format. It is a byte as follows
 * xxGZxxTX
 * G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
 */
static uint8_t *
stbox_flags_to_wkb_buf(const STBOX *box, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MOBDB_FLAGS_GET_X(box->flags))
    wkb_flags |= MOBDB_WKB_XFLAG;
  if (MOBDB_FLAGS_GET_Z(box->flags))
    wkb_flags |= MOBDB_WKB_ZFLAG;
  if (MOBDB_FLAGS_GET_T(box->flags))
    wkb_flags |= MOBDB_WKB_TFLAG;
  if (MOBDB_FLAGS_GET_GEODETIC(box->flags))
    wkb_flags |= MOBDB_WKB_GEODETICFLAG;
  if (stbox_wkb_needs_srid(box))
    wkb_flags |= MOBDB_WKB_SRIDFLAG;
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = (uint8_t) hexchr[wkb_flags];
    return buf + 2;
  }
  else
  {
    buf[0] = wkb_flags;
    return buf + 1;
  }
}

/**
 * Write into the buffer a spatiotemporal box represented in Well-Known Binary
 * (WKB) format as follows
 * - Endian byte
 * - Flag byte stating whether the X, Z, and time dimensions are present,
 *   whether the box is geodetic and whether an SRID is needed
 * - Output the int32 for the SRID (if there is an X dimension and if the SRID
 *   is needed), the 4 or 6 doubles for the value dimension (if there are X and
 *   Z dimensions) and the 2 timestamps for the time dimension (if there is a
 *   time dimension)
 */
static uint8_t *
stbox_to_wkb_buf(const STBOX *box, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = stbox_flags_to_wkb_buf(box, buf, variant);
  /* Write the value dimension if any */
  if (MOBDB_FLAGS_GET_X(box->flags))
  {
    /* Write the optional SRID for extended variant */
    if (stbox_wkb_needs_srid(box))
      buf = int32_to_wkb_buf(box->srid, buf, variant);
    /* Write the coordinates */
    buf = double_to_wkb_buf(box->xmin, buf, variant);
    buf = double_to_wkb_buf(box->xmax, buf, variant);
    buf = double_to_wkb_buf(box->ymin, buf, variant);
    buf = double_to_wkb_buf(box->ymax, buf, variant);
    if (MOBDB_FLAGS_GET_Z(box->flags))
    {
      buf = double_to_wkb_buf(box->zmin, buf, variant);
      buf = double_to_wkb_buf(box->zmax, buf, variant);
    }
  }
  /* Write the temporal dimension if any */
  if (MOBDB_FLAGS_GET_T(box->flags))
  {
    buf = timestamp_to_wkb_buf(box->tmin, buf, variant);
    buf = timestamp_to_wkb_buf(box->tmax, buf, variant);
  }
  return buf;
}

/*****************************************************************************/

/**
 * Write into the buffer the temporal type
 */
static uint8_t *
temporal_temptype_to_wkb_buf(const Temporal *temp, uint8_t *buf,
  uint8_t variant)
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
temporal_flags_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  /* Write the flags */
  if (tgeo_type(temp->temptype))
  {
    if (MOBDB_FLAGS_GET_Z(temp->flags))
      wkb_flags |= MOBDB_WKB_ZFLAG;
    if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
      wkb_flags |= MOBDB_WKB_GEODETICFLAG;
    if (tpoint_wkb_needs_srid(temp, variant))
      wkb_flags |= MOBDB_WKB_SRIDFLAG;
  }
  if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
    wkb_flags |= MOBDB_WKB_LINEAR_INTERP;
  /* Write the subtype */
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
tinstant_basevalue_time_to_wkb_buf(const TInstant *inst, uint8_t *buf,
  uint8_t variant)
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
 * - Output of a single instant by function tinstant_basevalue_time_to_wkb_buf
 */
static uint8_t *
tinstant_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = temporal_temptype_to_wkb_buf((Temporal *) inst, buf, variant);
  /* Write the temporal flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) inst, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(inst->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) inst, variant))
    buf = int32_to_wkb_buf(tpointinst_srid(inst), buf, variant);
  return tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
}

/**
 * Write into the buffer the temporal instant set point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Output of the instants by function tinstant_basevalue_time_to_wkb_buf
 */
static uint8_t *
tinstantset_to_wkb_buf(const TInstantSet *is, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = temporal_temptype_to_wkb_buf((Temporal *) is, buf, variant);
  /* Write the temporal flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) is, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(is->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) is, variant))
    buf = int32_to_wkb_buf(tpointinstset_srid(is), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(is->count, buf, variant);
  /* Write the array of instants */
  for (int i = 0; i < is->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(is, i);
    buf = tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
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
tsequence_bounds_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
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
 *   - Output of the instant by function tinstant_basevalue_time_to_wkb_buf
 */
static uint8_t *
tsequence_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = temporal_temptype_to_wkb_buf((Temporal *) seq, buf, variant);
  /* Write the temporal flags and interpolation */
  buf = temporal_flags_to_wkb_buf((Temporal *) seq, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(seq->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) seq, variant))
    buf = int32_to_wkb_buf(tpointseq_srid(seq), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(seq->count, buf, variant);
  /* Write the period bounds */
  buf = tsequence_bounds_to_wkb_buf(seq, buf, variant);
  /* Write the array of instants */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    buf = tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
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
 *      - Output of the instant by function tinstant_basevalue_time_to_wkb_buf
 */
static uint8_t *
tsequenceset_to_wkb_buf(const TSequenceSet *ss, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = temporal_temptype_to_wkb_buf((Temporal *) ss, buf, variant);
  /* Write the temporal and interpolation flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) ss, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(ss->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) ss, variant))
    buf = int32_to_wkb_buf(tpointseqset_srid(ss), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(ss->count, buf, variant);
  /* Write the sequences */
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    /* Write the number of instants */
    buf = int32_to_wkb_buf(seq->count, buf, variant);
    /* Write the period bounds */
    buf = tsequence_bounds_to_wkb_buf(seq, buf, variant);
    /* Write the array of instants */
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      buf = tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
    }
  }
  return buf;
}

/**
 * Write into the buffer the temporal value represented in Well-Known Binary
 * (WKB) format depending on the subtype
 */
static uint8_t *
temporal_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    buf = tinstant_to_wkb_buf((TInstant *) temp, buf, variant);
  else if (temp->subtype == INSTANTSET)
    buf = tinstantset_to_wkb_buf((TInstantSet *) temp, buf, variant);
  else if (temp->subtype == SEQUENCE)
    buf = tsequence_to_wkb_buf((TSequence *) temp, buf, variant);
  else /* temp->subtype == SEQUENCESET */
    buf = tsequenceset_to_wkb_buf((TSequenceSet *) temp, buf, variant);
  return buf;
}

/**
 * Write into the buffer the WKB representation of a value.
 */
static uint8_t *
datum_to_wkb_buf(Datum value, CachedType type, uint8_t *buf, uint8_t variant)
{
  switch (type)
  {
    case T_INTSPAN:
    case T_FLOATSPAN:
    case T_PERIOD:
      buf = span_to_wkb_buf((Span *) DatumGetPointer(value), buf, variant);
      break;
    case T_TIMESTAMPTZ:
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(value), buf, variant);
      break;
    case T_TIMESTAMPSET:
      buf = timestampset_to_wkb_buf((TimestampSet *) DatumGetPointer(value),
        buf, variant);
      break;
    case T_PERIODSET:
      buf = periodset_to_wkb_buf((PeriodSet *) DatumGetPointer(value), buf,
        variant);
      break;
    case T_TBOX:
      buf = tbox_to_wkb_buf((TBOX *) DatumGetPointer(value), buf, variant);
      break;
    case T_STBOX:
      buf = stbox_to_wkb_buf((STBOX *) DatumGetPointer(value), buf, variant);
      break;
    case T_TBOOL:
    case T_TINT:
    case T_TFLOAT:
    case T_TTEXT:
    case T_TDOUBLE2:
    case T_TDOUBLE3:
    case T_TDOUBLE4:
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
#if ! MEOS
    case T_TNPOINT:
#endif /* ! MEOS */
      buf = temporal_to_wkb_buf((Temporal *) DatumGetPointer(value), buf,
        variant);
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB type: %d", type);
      break;
  }
  return buf;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a datum value.
 *
 * @param[in] value Value
 * @param[in] type Type of the value
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
datum_as_wkb(Datum value, CachedType type, uint8_t variant, size_t *size_out)
{
  size_t buf_size;
  uint8_t *buf = NULL;
  uint8_t *wkb_out = NULL;

  /* Initialize output size */
  if (size_out) *size_out = 0;

  /* Calculate the required size of the output buffer */
  buf_size = datum_to_wkb_size(value, type, variant);
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
  buf = datum_to_wkb_buf(value, type, buf, variant);

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
 * @brief Return the HexWKB representation of a datum value.
 */
char *
datum_as_hexwkb(Datum value, CachedType type, uint8_t variant, size_t *size)
{
  /* Create WKB hex string */
  char *result = (char *) datum_as_wkb(value, type,
    variant | (uint8_t) WKB_EXTENDED | (uint8_t) WKB_HEX, size);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB functions for the MEOS API
 *****************************************************************************/

#if MEOS

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a span.
 */
uint8_t *
span_as_wkb(const Span *s, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(s), s->spantype, size_out);
  return result;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a span in hex-encoded ASCII.
 */
char *
span_as_hexwkb(const Span *s, uint8_t variant, size_t *size)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(s), s->spantype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a timestamp set.
 */
uint8_t *
timestampset_as_wkb(const TimestampSet *ts, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(ts), T_TIMESTAMPSET, size_out);
  return result;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a timestamp set in hex-encoded ASCII.
 */
char *
timestampset_as_hexwkb(const TimestampSet *ts, uint8_t variant,
  size_t *size_out)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(ts), T_TIMESTAMPSET,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a period set.
 */
uint8_t *
periodset_as_wkb(const PeriodSet *ps, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(ps), T_PERIODSET, size_out);
  return result;
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the WKB representation of a period set in hex-encoded ASCII.
 */
char *
periodset_as_hexwkb(const PeriodSet *ps, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(ps), T_PERIODSET,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the WKB representation of a temporal box.
 */
uint8_t *
tbox_as_wkb(const TBOX *box, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(box), T_TBOX, size_out);
  return result;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the WKB representation of a temporal box in hex-encoded ASCII.
 */
char *
tbox_as_hexwkb(const TBOX *box, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(box), T_TBOX,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the WKB representation of a spatiotemporal box.
 */
uint8_t *
stbox_as_wkb(const STBOX *box, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(box), T_STBOX, size_out);
  return result;
}

/**
 * @ingroup libmeos_box_input_output
 * @brief Return the WKB representation of a spatiotemporal box in hex-encoded ASCII.
 */
char *
stbox_as_hexwkb(const STBOX *box, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(box), T_STBOX,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the WKB representation of a temporal value.
 */
uint8_t *
temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(temp), temp->spantype, size_out);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the WKB representation of a temporal value in hex-encoded ASCII.
 */
char *
temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size)
{
  char *result = (char *) datum_as_as_wkb(PointerGetDatum(temp), temp->spantype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}

/*****************************************************************************/

#endif /* MEOS */

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/**
 * @brief Ensure that a string represents a valid endian flag
 */
void
ensure_valid_endian_flag(const char *endian)
{
  if (strncasecmp(endian, "ndr", 3) != 0 && strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  return;
}

/**
 * @brief Output a generic value in WKB or EWKB format
 */
bytea *
datum_as_wkb_ext(FunctionCallInfo fcinfo, Datum value, CachedType type,
  bool extended)
{
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
  size_t wkb_size;
  uint8_t *wkb = extended ?
    datum_as_wkb(value, type, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    datum_as_wkb(value, type, variant, &wkb_size);

  /* Prepare the PostgreSQL bytea return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  return result;
}

/**
 * @brief Output a generic value in WKB or EWKB format as hex-encoded ASCII
 */
bytea *
datum_as_hexwkb_ext(FunctionCallInfo fcinfo, Datum value, CachedType type)
{
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
  char *hexwkb = datum_as_hexwkb(value, type, variant, &hexwkb_size);

  /* Prepare the PgSQL text return type */
  size_t text_size = hexwkb_size - 1 + VARHDRSZ;
  text *result = palloc(text_size);
  memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
  SET_VARSIZE(result, text_size);

  /* Clean up and return */
  pfree(hexwkb);
  return result;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_as_wkb);
/**
 * Output a span in WKB format.
 */
PGDLLEXPORT Datum
Span_as_wkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(s), s->spantype,
    false);
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
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(s), s->spantype);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_as_wkb);
/**
 * Output a timestamp set in WKB format.
 */
PGDLLEXPORT Datum
Timestampset_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(ts),
    T_TIMESTAMPSET, false);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Timestampset_as_hexwkb);
/**
 * Output the timestamp set in HexWKB format.
 */
PGDLLEXPORT Datum
Timestampset_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(ts),
    T_TIMESTAMPSET);
  PG_FREE_IF_COPY(ts, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_as_wkb);
/**
 * Output a period set in WKB format.
 */
PGDLLEXPORT Datum
Periodset_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(ps),
    T_PERIODSET, false);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Periodset_as_hexwkb);
/**
 * Output the period set in HexWKB format.
 */
PGDLLEXPORT Datum
Periodset_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  PeriodSet *ps = PG_GETARG_PERIODSET_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(ps),
    T_PERIODSET);
  PG_FREE_IF_COPY(ps, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_as_wkb);
/**
 * Output a temporal box in WKB format.
 */
PGDLLEXPORT Datum
Tbox_as_wkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  bytea *result = datum_as_wkb_ext(fcinfo, box, T_TBOX, false);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tbox_as_hexwkb);
/**
 * Output the temporal box in HexWKB format.
 */
PGDLLEXPORT Datum
Tbox_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  text *result = datum_as_hexwkb_ext(fcinfo, box, T_TBOX);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_as_wkb);
/**
 * Output a temporal box in WKB format.
 */
PGDLLEXPORT Datum
Stbox_as_wkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  bytea *result = datum_as_wkb_ext(fcinfo, box, T_STBOX, false);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Stbox_as_hexwkb);
/**
 * Output the temporal box in HexWKB format.
 */
PGDLLEXPORT Datum
Stbox_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  text *result = datum_as_hexwkb_ext(fcinfo, box, T_STBOX);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_as_wkb);
/**
 * @brief Output a temporal value in WKB format.
 * @note This will have no 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Temporal_as_wkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkb);
/**
 * @brief Output a temporal point in EWKB format.
 * @note This will have 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Tpoint_as_ewkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bytea *result = datum_as_wkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype, true);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Temporal_as_hexwkb);
/**
 * @brief Output the temporal point in HexEWKB format.
 * @note This will have 'SRID=#;' for temporal points
 */
PGDLLEXPORT Datum
Temporal_as_hexwkb(PG_FUNCTION_ARGS)
{
  /* Ensure that the value is detoasted if necessary */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  text *result = datum_as_hexwkb_ext(fcinfo, PointerGetDatum(temp),
    temp->temptype);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
