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
 * @file temporal_in.c
 * @brief Input of temporal types in WKB, EWKB, and HexWKB format.
 */

/* C */
#include <assert.h>
#include <float.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/doublen.h"
#include "general/tbox.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "point/stbox.h"
#include "point/tpoint_spatialfuncs.h"
#if ! MEOS
  #include "npoint/tnpoint_static.h"
#endif /* ! MEOS */

/*****************************************************************************/
 
/**
 * Structure used for passing the parse state between the parsing functions.
 */
typedef struct
{
  const uint8_t *wkb;  /**< Points to start of WKB */
  size_t wkb_size;     /**< Expected size of WKB */
  bool swap_bytes;     /**< Do an endian flip? */
  uint8_t temptype;    /**< Current temporal type we are handling */
  uint8_t basetype;    /**< Current base type we are handling */
  uint8_t subtype;     /**< Current subtype we are handling */
  int32_t srid;        /**< Current SRID we are handling */
  bool hasx;           /**< X? */
  bool hasz;           /**< Z? */
  bool hast;           /**< T? */
  bool geodetic;       /**< Geodetic? */
  bool has_srid;       /**< SRID? */
  bool linear;         /**< Linear interpolation? */
  const uint8_t *pos;  /**< Current parse position */
} wkb_parse_state;

/*****************************************************************************
 * Input in WKB format
 * Please refer to the file temporal_wkb_out.c where the binary format is
 * explained
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
 * Read a byte and advance the parse state forward
 */
char
byte_from_wkb_state(wkb_parse_state *s)
{
  char char_value = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_BYTE_SIZE);
  /* Get the data */
  char_value = s->pos[0];
  s->pos += MOBDB_WKB_BYTE_SIZE;
  return char_value;
}

/**
 * Read a 2-byte integer and advance the parse state forward
 */
uint16_t
int16_from_wkb_state(wkb_parse_state *s)
{
  uint16_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT2_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT2_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT2_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT2_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT2_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT2_SIZE;
  return i;
}

/**
 * Read a 4-byte integer and advance the parse state forward
 */
uint32_t
int32_from_wkb_state(wkb_parse_state *s)
{
  uint32_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT4_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT4_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT4_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT4_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT4_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT4_SIZE;
  return i;
}

/**
 * Read an 8-byte integer and advance the parse state forward
 */
uint64_t
int64_from_wkb_state(wkb_parse_state *s)
{
  uint64_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT8_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT8_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT8_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT8_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT8_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT8_SIZE;
  return i;
}

/**
 * Read an 8-byte double and advance the parse state forward
 */
double
double_from_wkb_state(wkb_parse_state *s)
{
  double d = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_DOUBLE_SIZE);
  /* Get the data */
  memcpy(&d, s->pos, MOBDB_WKB_DOUBLE_SIZE);
  /* Swap? Copy into a stack-allocated double */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MOBDB_WKB_DOUBLE_SIZE/2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&d))[i];
      ((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[MOBDB_WKB_DOUBLE_SIZE - i - 1];
      ((uint8_t*)(&d))[MOBDB_WKB_DOUBLE_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_DOUBLE_SIZE;
  return d;
}

/**
 * Read an 8-byte timestamp and advance the parse state forward
 */
TimestampTz
timestamp_from_wkb_state(wkb_parse_state *s)
{
  int64_t t = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_TIMESTAMP_SIZE);
  /* Get the data */
  memcpy(&t, s->pos, MOBDB_WKB_TIMESTAMP_SIZE);
  /* Swap? Copy into a stack-allocated timestamp */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MOBDB_WKB_TIMESTAMP_SIZE/2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&t))[i];
      ((uint8_t*)(&t))[i] = ((uint8_t*)(&t))[MOBDB_WKB_TIMESTAMP_SIZE - i - 1];
      ((uint8_t*)(&t))[MOBDB_WKB_TIMESTAMP_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_TIMESTAMP_SIZE;
  return (TimestampTz) t;
}

/**
 * Read a text and advance the parse state forward
 */
text *
text_from_wkb_state(wkb_parse_state *s)
{
  /* Get the size of the text value */
  size_t size = int64_from_wkb_state(s);
  assert(size > 0);
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, size);
  /* Get the data */
  char *str = palloc(size + 1);
  memcpy(str, s->pos, size);
  s->pos += size;
  text *result = cstring2text(str);
  pfree(str);
  /* Advance the state and return */
  return result;
}

/**
 * Return a point from its WKB representation. A WKB point has just a set of
 * doubles, with the quantity depending on the dimension of the point.
 */
Datum
point_from_wkb_state(wkb_parse_state *s)
{
  double x, y, z;
  x = double_from_wkb_state(s);
  y = double_from_wkb_state(s);
  if (s->hasz)
    z = double_from_wkb_state(s);
  LWPOINT *point = s->hasz ? lwpoint_make3dz(s->srid, x, y, z) :
    lwpoint_make2d(s->srid, x, y);
  FLAGS_SET_GEODETIC(point->flags, s->geodetic);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  lwpoint_free(point);
  return result;
}

#if ! MEOS
/**
 * Read an npoint and advance the parse state forward
 */
Npoint *
npoint_from_wkb_state(wkb_parse_state *s)
{
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT8_SIZE + MOBDB_WKB_DOUBLE_SIZE);
  /* Get the data */
  int64 rid = int64_from_wkb_state(s);
  double pos = double_from_wkb_state(s);
  Npoint *result = palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}
#endif /* ! MEOS */

/*****************************************************************************/

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
      elog(ERROR, "Unknown WKB span type: %d", wkb_spantype);
      break;
  }
  s->basetype = spantype_basetype(s->temptype);
  return;
}

/**
 * Return the size of a span base value from its WKB representation.
 */
static size_t
span_basevalue_from_wkb_size(wkb_parse_state *s)
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
span_basevalue_from_wkb_state(wkb_parse_state *s)
{
  Datum result;
  ensure_span_type(s->temptype);
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
      elog(ERROR, "Unknown span type: %d",
        s->temptype);
      break;
  }
  return result;
}

/**
 * Set the bound flags from their WKB representation
 */
static void
bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc, bool *upper_inc)
{
  if (wkb_bounds & MOBDB_WKB_LOWER_INC)
    *lower_inc = true;
  else
    *lower_inc = false;
  if (wkb_bounds & MOBDB_WKB_UPPER_INC)
    *upper_inc = true;
  else
    *upper_inc = false;
  return;
}

/**
 * Return a span from its WKB representation
 */
Span *
span_from_wkb_state(wkb_parse_state *s)
{
  /* Read the span type */
  uint16_t wkb_spantype = (uint16_t) int16_from_wkb_state(s);
  span_spantype_from_wkb_state(s, wkb_spantype);

  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  size_t size = 2 * span_basevalue_from_wkb_size(s);
  wkb_parse_state_check(s, size);

  /* Read the values and create the span */
  Datum lower = span_basevalue_from_wkb_state(s);
  Datum upper = span_basevalue_from_wkb_state(s);
  Span *result = span_make(lower, upper, lower_inc, upper_inc, s->basetype);
  return result;
}

/*****************************************************************************/

/**
 * Return a timestamp set from its WKB representation
 */
static TimestampSet *
timestampset_from_wkb_state(wkb_parse_state *s)
{
  /* Read the number of timestamps and allocate space for them */
  int count = int32_from_wkb_state(s);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);

  /* Read and create the timestamp set */
  for (int i = 0; i < count; i++)
    times[i] = timestamp_from_wkb_state(s);
  TimestampSet *result = timestampset_make_free(times, count);
  return result;
}

/*****************************************************************************/

/**
 * Optimized version of span_from_wkb_state for reading the periods in a period
 * set. The endian byte and the basetype int16 are not read from the buffer.
 */
static Period *
period_from_wkb_state(wkb_parse_state *s)
{
  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, 2 * MOBDB_WKB_TIMESTAMP_SIZE);

  /* Read the values and create the span */
  Datum lower = TimestampTzGetDatum(timestamp_from_wkb_state(s));
  Datum upper = TimestampTzGetDatum(timestamp_from_wkb_state(s));
  Span *result = span_make(lower, upper, lower_inc, upper_inc, s->basetype);
  return result;
}

/**
 * Return a period set from its WKB representation
 */
static PeriodSet *
periodset_from_wkb_state(wkb_parse_state *s)
{
  /* Read the number of periods and allocate space for them */
  int count = int32_from_wkb_state(s);
  Period **periods = palloc(sizeof(Period *) * count);

  /* Set the state basetype to TimestampTz */
  s->basetype = T_TIMESTAMPTZ;

  /* Read and create the period set */
  for (int i = 0; i < count; i++)
    periods[i] = (Period *) period_from_wkb_state(s);
  PeriodSet *result = periodset_make_free(periods, count, NORMALIZE);
  return result;
}

/*****************************************************************************/

/**
 * Set the state flags according to a box byte flag read from the buffer.
 */
static void
tbox_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasx = false;
  s->hast = false;
  if (wkb_flags & MOBDB_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MOBDB_WKB_TFLAG)
    s->hast = true;
  return;
}

/**
 * Return a temporal box from its WKB representation
 */
static TBOX *
tbox_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  tbox_flags_from_wkb_state(s, wkb_flags);

  /* Read and create the box */
  double xmin = 0, xmax = 0; /* make compiler quiet */
  TimestampTz tmin = 0, tmax = 0; /* make compiler quiet */
  if (s->hasx)
  {
    xmin = double_from_wkb_state(s);
    xmax = double_from_wkb_state(s);
  }
  if (s->hast)
  {
    tmin = timestamp_from_wkb_state(s);
    tmax = timestamp_from_wkb_state(s);
  }
  TBOX *result = tbox_make(s->hasx, s->hast, xmin, xmax, tmin, tmax);
  return result;
}

/*****************************************************************************/

/**
 * Set the state flags according to a box byte flag read from the buffer.
 */
static void
stbox_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasx = false;
  s->hasz = false;
  s->hast = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MOBDB_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MOBDB_WKB_ZFLAG)
    s->hasz = true;
  if (wkb_flags & MOBDB_WKB_TFLAG)
    s->hast = true;
  if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->geodetic = true;
  if (wkb_flags & MOBDB_WKB_SRIDFLAG)
    s->has_srid = true;
  return;
}

/**
 * Return a spatiotemporal box from its WKB representation
 */
static STBOX *
stbox_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  stbox_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);
  else if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

  /* Read and create the box */
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  TimestampTz tmin = 0, tmax = 0; /* make compiler quiet */
  if (s->hasx)
  {
    xmin = double_from_wkb_state(s);
    xmax = double_from_wkb_state(s);
    ymin = double_from_wkb_state(s);
    ymax = double_from_wkb_state(s);
    if (s->hasz)
    {
      zmin = double_from_wkb_state(s);
      zmax = double_from_wkb_state(s);
    }
  }
  if (s->hast)
  {
    tmin = timestamp_from_wkb_state(s);
    tmax = timestamp_from_wkb_state(s);
  }
  STBOX *result = stbox_make(s->hasx, s->hasz, s->hast, s->geodetic, s->srid,
    xmin, xmax, ymin, ymax, zmin, zmax, tmin, tmax);
  return result;
}

/*****************************************************************************/

/**
 * Take in an unknown temporal type of WKB type number and ensure it comes out
 * as an extended WKB temporal type number.
 */
void
temporal_temptype_from_wkb_state(wkb_parse_state *s, uint16_t wkb_temptype)
{
  switch (wkb_temptype)
  {
    case MOBDB_WKB_T_TBOOL:
      s->temptype = T_TBOOL;
      break;
    case MOBDB_WKB_T_TINT:
      s->temptype = T_TINT;
      break;
    case MOBDB_WKB_T_TFLOAT:
      s->temptype = T_TFLOAT;
      break;
    case MOBDB_WKB_T_TTEXT:
      s->temptype = T_TTEXT;
      break;
    case MOBDB_WKB_T_TGEOMPOINT:
      s->temptype = T_TGEOMPOINT;
      break;
    case MOBDB_WKB_T_TGEOGPOINT:
      s->temptype = T_TGEOGPOINT;
      break;
#if ! MEOS
    case MOBDB_WKB_T_TNPOINT:
      s->temptype = T_TNPOINT;
      break;
#endif /* ! MEOS */
    default: /* Error! */
      elog(ERROR, "Unknown WKB temporal type: %d", wkb_temptype);
      break;
  }
  s->basetype = temptype_basetype(s->temptype);
  return;
}

/**
 * Take in an unknown kind of WKB type number and ensure it comes out as an
 * extended WKB type number (with Z/GEODETIC/SRID/LINEAR_INTERP flags masked
 * onto the high bits).
 */
void
temporal_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasz = false;
  s->geodetic = false;
  s->has_srid = false;
  /* Get the flags */
  if (tgeo_type(s->temptype))
  {
    if (wkb_flags & MOBDB_WKB_ZFLAG)
      s->hasz = true;
    if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
      s->geodetic = true;
    if (wkb_flags & MOBDB_WKB_SRIDFLAG)
      s->has_srid = true;
  }
  if (wkb_flags & MOBDB_WKB_LINEAR_INTERP)
    s->linear = true;
  /* Mask off the upper flags to get the subtype */
  wkb_flags = wkb_flags & (uint8_t) 0x0F;
  switch (wkb_flags)
  {
    case MOBDB_WKB_INSTANT:
      s->subtype = INSTANT;
      break;
    case MOBDB_WKB_INSTANTSET:
      s->subtype = INSTANTSET;
      break;
    case MOBDB_WKB_SEQUENCE:
      s->subtype = SEQUENCE;
      break;
    case MOBDB_WKB_SEQUENCESET:
      s->subtype = SEQUENCESET;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB flags: %d", wkb_flags);
      break;
  }
  return;
}

/**
 * Return a value from its WKB representation.
 */
static Datum
temporal_basevalue_from_wkb_state(wkb_parse_state *s)
{
  Datum result;
  ensure_temporal_basetype(s->basetype);
  switch (s->temptype)
  {
    case T_TBOOL:
      result = BoolGetDatum(byte_from_wkb_state(s));
      break;
    case T_TINT:
      result = Int32GetDatum(int32_from_wkb_state(s));
      break;
#if 0 /* not used */
    case T_TINT8:
      result = Int64GetDatum(int64_from_wkb_state(s));
      break;
#endif /* not used */
    case T_TFLOAT:
      result = Float8GetDatum(double_from_wkb_state(s));
      break;
    case T_TTEXT:
      result = PointerGetDatum(text_from_wkb_state(s));
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      result = point_from_wkb_state(s);
      break;
#if ! MEOS
    case T_TNPOINT:
      result = PointerGetDatum(npoint_from_wkb_state(s));
      break;
#endif
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d",
        s->temptype);
      break;
  }
  return result;
}

/**
 * @brief Return a temporal instant point from its WKB representation.
 *
 * It reads the base type value and the timestamp and advances the parse state
 * forward appropriately.
 * @note It starts reading it just after the endian byte, the temporal type
 * int16, and the temporal flags byte.
 */
static TInstant *
tinstant_from_wkb_state(wkb_parse_state *s)
{
  /* Read the values from the buffer and create the instant */
  Datum value = temporal_basevalue_from_wkb_state(s);
  TimestampTz t = timestamp_from_wkb_state(s);
  TInstant *result = tinstant_make(value, s->temptype, t);
  if (! basetype_byvalue(s->basetype))
    pfree(DatumGetPointer(value));
  return result;
}

/**
 * Return a temporal instant array from its WKB representation
 */
static TInstant **
tinstarr_from_wkb_state(wkb_parse_state *s, int count)
{
  TInstant **result = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Parse the point and the timestamp to create the instant point */
    Datum value = temporal_basevalue_from_wkb_state(s);
    TimestampTz t = timestamp_from_wkb_state(s);
    result[i] = tinstant_make(value, s->temptype, t);
    if (! basetype_byvalue(s->basetype))
      pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * Return a temporal instant set value from its WKB representation
 */
static TInstantSet *
tinstantset_from_wkb_state(wkb_parse_state *s)
{
  /* Get the number of instants */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Parse the instants */
  TInstant **instants = tinstarr_from_wkb_state(s, count);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * Return a temporal sequence value from its WKB representation
 */
static TSequence *
tsequence_from_wkb_state(wkb_parse_state *s)
{
  /* Get the number of instants */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Get the period bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
  /* Parse the instants */
  TInstant **instants = tinstarr_from_wkb_state(s, count);
  return tsequence_make_free(instants, count, lower_inc, upper_inc,
    s->linear, NORMALIZE);
}

/**
 * Return a temporal sequence set value from its WKB representation
 */
static TSequenceSet *
tsequenceset_from_wkb_state(wkb_parse_state *s)
{
  /* Get the number of sequences */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Parse the sequences */
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Get the number of instants */
    int countinst = int32_from_wkb_state(s);
    /* Get the period bounds */
    uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
    bool lower_inc, upper_inc;
    bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
    /* Parse the instants */
    TInstant **instants = palloc(sizeof(TInstant *) * countinst);
    for (int j = 0; j < countinst; j++)
    {
      /* Parse the value and the timestamp to create the temporal instant */
      Datum value = temporal_basevalue_from_wkb_state(s);
      TimestampTz t = timestamp_from_wkb_state(s);
      instants[j] = tinstant_make(value, s->temptype, t);
      if (! basetype_byvalue(s->basetype))
        pfree(DatumGetPointer(value));
    }
    sequences[i] = tsequence_make_free(instants, countinst, lower_inc,
      upper_inc, s->linear, NORMALIZE);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return a temporal value from its WKB representation
 */
static Temporal *
temporal_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal type */
  uint16_t wkb_temptype = (uint16_t) int16_from_wkb_state(s);
  temporal_temptype_from_wkb_state(s, wkb_temptype);

  /* Read the temporal and interpolation flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  temporal_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);
  else if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

  /* Read the temporal value */
  ensure_valid_tempsubtype(s->subtype);
  if (s->subtype == INSTANT)
    return (Temporal *) tinstant_from_wkb_state(s);
  else if (s->subtype == INSTANTSET)
    return (Temporal *) tinstantset_from_wkb_state(s);
  else if (s->subtype == SEQUENCE)
    return (Temporal *) tsequence_from_wkb_state(s);
  else /* s->subtype == SEQUENCESET */
    return (Temporal *) tsequenceset_from_wkb_state(s);
}

/*****************************************************************************/

/**
 * @brief Return a value from its Well-Known Binary (WKB) representation.
 */
Datum
datum_from_wkb(uint8_t *wkb, int size, CachedType type)
{
  /* Initialize the state appropriately */
  wkb_parse_state s;
  memset(&s, 0, sizeof(wkb_parse_state));
  s.wkb = s.pos = wkb;
  s.wkb_size = size;
  /* Fail when handed incorrect starting byte */
  char wkb_little_endian = byte_from_wkb_state(&s);
  if (wkb_little_endian != 1 && wkb_little_endian != 0)
    elog(ERROR, "Invalid endian flag value encountered.");

  /* Check the endianness of our input */
  s.swap_bytes = false;
  /* Machine arch is big endian, request is for little */
  if (MOBDB_IS_BIG_ENDIAN && wkb_little_endian)
    s.swap_bytes = true;
  /* Machine arch is little endian, request is for big */
  else if ((! MOBDB_IS_BIG_ENDIAN) && (! wkb_little_endian))
    s.swap_bytes = true;

  /* Call the type-specific function */
  Datum result;
  switch (type)
  {
    case T_INTSPAN:
    case T_FLOATSPAN:
    case T_PERIOD:
      result = PointerGetDatum(span_from_wkb_state(&s));
      break;
    case T_TIMESTAMPSET:
      result = PointerGetDatum(timestampset_from_wkb_state(&s));
      break;
    case T_PERIODSET:
      result = PointerGetDatum(periodset_from_wkb_state(&s));
      break;
    case T_TBOX:
      result = PointerGetDatum(tbox_from_wkb_state(&s));
      break;
    case T_STBOX:
      result = PointerGetDatum(stbox_from_wkb_state(&s));
      break;
    case T_TBOOL:
    case T_TINT:
    case T_TFLOAT:
    case T_TTEXT:
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
#if ! MEOS
    case T_TNPOINT:
#endif /* ! MEOS */
      result = PointerGetDatum(temporal_from_wkb_state(&s));
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB type: %d", type);
      break;
  }

  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal type from its HexEWKB representation
 */
Datum
datum_from_hexwkb(const char *hexwkb, int size, CachedType type)
{
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, size);
  Datum result = datum_from_wkb(wkb, size / 2, type);
  pfree(wkb);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB functions for the MEOS API
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its Well-Known Binary (WKB)
 * representation.
 */
Span *
span_from_wkb(uint8_t *wkb, int size)
{
  /* We pass ANY span type to the dispatch function but the actual span type
   * will be read from the byte string */
  return DatumGetSpanP(datum_from_wkb(wkb, size, T_INTSPAN));
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its WKB representation in hex-encoded ASCII.
 */
Span *
span_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  /* We pass ANY span type to the dispatch function but the actual span type
   * will be read from the byte string */
  return DatumGetSpanP(datum_from_hexwkb(hexwkb, size, T_INTSPAN));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a timestamp set from its Well-Known Binary (WKB)
 * representation.
 */
TimestampSet *
timestampset_from_wkb(uint8_t *wkb, int size)
{
  return DatumGetTimestampSetP(datum_from_wkb(wkb, size, T_TIMESTAMPSET));
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a timestamp set from its WKB representation in hex-encoded
 * ASCII.
 */
TimestampSet *
timestampset_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetTimestampSetP(datum_from_hexwkb(hexwkb, size, T_TIMESTAMPSET));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a period set from its Well-Known Binary (WKB)
 * representation.
 */
PeriodSet *
periodset_from_wkb(uint8_t *wkb, int size)
{
  return DatumGetPeriodSetP(datum_from_wkb(wkb, size, T_PERIODSET));
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a period set from its WKB representation in hex-encoded ASCII
 */
PeriodSet *
periodset_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetPeriodSetP(datum_from_hexwkb(hexwkb, size, T_PERIODSET));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return a temporal box from its Well-Known Binary (WKB)
 * representation.
 */
TBOX *
tbox_from_wkb(uint8_t *wkb, int size)
{
  return DatumGetTboxP(datum_from_wkb(wkb, size, T_TBOX));
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a temporal box from its WKB representation in hex-encoded ASCII
 */
TBOX *
tbox_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetTboxP(datum_from_hexwkb(hexwkb, size, T_TBOX));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_input_output
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation.
 */
STBOX *
stbox_from_wkb(uint8_t *wkb, int size)
{
  return DatumGetSTboxP(datum_from_wkb(wkb, size, T_STBOX));
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a spatiotemporal box from its WKB representation in
 * hex-encoded ASCII
 */
STBOX *
stbox_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetSTboxP(datum_from_hexwkb(hexwkb, size, T_STBOX));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal value from its Well-Known Binary (WKB)
 * representation.
 */
Temporal *
temporal_from_wkb(uint8_t *wkb, int size)
{
  /* We pass ANY temporal type to the dispatch function but the actual temporal
   * type will be read from the byte string */
  return DatumGetTemporalP(datum_from_wkb(wkb, size, T_TINT));
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal value from its HexEWKB representation
 */
Temporal *
temporal_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  /* We pass ANY temporal type to the dispatch function but the actual temporal
   * type will be read from the byte string */
  return DatumGetTemporalP(datum_from_hexwkb(hexwkb, size, T_TINT));
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

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

PG_FUNCTION_INFO_V1(Timestampset_from_wkb);
/**
 * Return a timestamp set from its WKB representation
 */
PGDLLEXPORT Datum
Timestampset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TimestampSet *ts = timestampset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(ts);
}

PG_FUNCTION_INFO_V1(Timestampset_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Timestampset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TimestampSet *ts = timestampset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(ts);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Periodset_from_wkb);
/**
 * Return a timestamp set from its WKB representation
 */
PGDLLEXPORT Datum
Periodset_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  PeriodSet *ps = periodset_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(ps);
}

PG_FUNCTION_INFO_V1(Periodset_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Periodset_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  PeriodSet *ps = periodset_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(ps);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_from_wkb);
/**
 * Return a temporal box from its WKB representation
 */
PGDLLEXPORT Datum
Tbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  TBOX *box = tbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PG_FUNCTION_INFO_V1(Tbox_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Tbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  TBOX *box = tbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Stbox_from_wkb);
/**
 * Return a temporal box from its WKB representation
 */
PGDLLEXPORT Datum
Stbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  STBOX *box = stbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(box);
}

PG_FUNCTION_INFO_V1(Stbox_from_hexwkb);
/**
 * Return a temporal point from its HexWKB representation
 */
PGDLLEXPORT Datum
Stbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  STBOX *box = stbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(box);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_wkb);
/**
 * Return a temporal type from its WKB representation
 */
PGDLLEXPORT Datum
Temporal_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *temp = temporal_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(temp);
}

PG_FUNCTION_INFO_V1(Temporal_from_hexwkb);
/**
 * Return a temporal type from its HEXWKB representation
 */
PGDLLEXPORT Datum
Temporal_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Temporal *temp = temporal_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(temp);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
