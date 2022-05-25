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
 * @brief Input of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON format.
 */

#include "general/temporal_in.h"

/* C */
#include <assert.h>
#include <float.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
// #include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input in WKB format
 * Please refer to the file temporal_out.c where the binary format is explained
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
  wkb_parse_state_check(s, MOBDB_WKB_BYTE_SIZE);
  char_value = s->pos[0];
  s->pos += MOBDB_WKB_BYTE_SIZE;
  return char_value;
}

/**
 * Read 4-byte integer and advance the parse state forward
 */
uint32_t
int32_from_wkb_state(wkb_parse_state *s)
{
  uint32_t i = 0;
  wkb_parse_state_check(s, MOBDB_WKB_INT4_SIZE);
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
 * Read an 8-byte double and advance the parse state forward
 */
double
double_from_wkb_state(wkb_parse_state *s)
{
  double d = 0;
  wkb_parse_state_check(s, MOBDB_WKB_DOUBLE_SIZE);
  memcpy(&d, s->pos, MOBDB_WKB_DOUBLE_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
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
  wkb_parse_state_check(s, MOBDB_WKB_TIMESTAMP_SIZE);
  memcpy(&t, s->pos, MOBDB_WKB_TIMESTAMP_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
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
 * Take in an unknown temporal type of WKB type number and ensure it comes out
 * as an extended WKB temporal type number.
 */
void
temporal_temptype_from_wkb_state(wkb_parse_state *s, uint8_t wkb_temptype)
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
    case MOBDB_WKB_T_TDOUBLE2:
      s->temptype = T_TDOUBLE2;
      break;
    case MOBDB_WKB_T_TDOUBLE3:
      s->temptype = T_TDOUBLE3;
      break;
    case MOBDB_WKB_T_TDOUBLE4:
      s->temptype = T_TDOUBLE4;
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
      elog(ERROR, "Unknown WKB temporal type (%d)!", wkb_temptype);
      break;
  }
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
  // if (wkb_flags & MOBDB_WKB_ZFLAG)
    // s->hasz = true;
  // if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    // s->geodetic = true;
  // if (wkb_flags & MOBDB_WKB_SRIDFLAG)
    // s->has_srid = true;
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
      elog(ERROR, "Unknown WKB flags (%d)!", wkb_flags);
      break;
  }
  return;
}

/**
 * Return a point from its WKB representation. A WKB point has just a set of doubles,
 * with the quantity depending on the dimension of the point.
 */
static Datum
basetype_from_wkb_state(wkb_parse_state *s)
{
  // TODO !!!!!
  Datum result = Int32GetDatum(0);
  return result;
}

/**
 * Return a temporal instant point from its WKB representation.
 *
 * It starts reading it just after the endian byte,
 * the type byte and the optional srid number.
 * Advance the parse state forward appropriately.
 */
static TInstant *
tinstant_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
  /* Does the data we want to read exist? */
  size_t size = (ndims * MOBDB_WKB_DOUBLE_SIZE) + MOBDB_WKB_TIMESTAMP_SIZE;
  wkb_parse_state_check(s, size);
  /* Create the instant point */
  Datum value = basetype_from_wkb_state(s);
  TimestampTz t = timestamp_from_wkb_state(s);
  CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
  TInstant *result = tinstant_make(value, temptype, t);
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
  CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
  for (int i = 0; i < count; i++)
  {
    /* Parse the point and the timestamp to create the instant point */
    Datum value = basetype_from_wkb_state(s);
    TimestampTz t = timestamp_from_wkb_state(s);
    result[i] = tinstant_make(value, temptype, t);
    pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * Return a temporal instant set point from its WKB representation
 */
static TInstantSet *
tinstantset_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions */
  uint32_t ndims = (s->hasz) ? 3 : 2;
  /* Get the number of instants */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Does the data we want to read exist? */
  size_t size = count * ((ndims * MOBDB_WKB_DOUBLE_SIZE) +
    MOBDB_WKB_TIMESTAMP_SIZE);
  wkb_parse_state_check(s, size);
  /* Parse the instants */
  TInstant **instants = tinstarr_from_wkb_state(s, count);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * Set the bound flags from their WKB representation
 */
void
temporal_bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc,
  bool *upper_inc)
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
 * Return a temporal sequence point from its WKB representation
 */
static TSequence *
tsequence_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
  /* Get the number of instants */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Get the period bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  temporal_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
  /* Does the data we want to read exist? */
  size_t size = count * ((ndims * MOBDB_WKB_DOUBLE_SIZE) +
    MOBDB_WKB_TIMESTAMP_SIZE);
  wkb_parse_state_check(s, size);
  /* Parse the instants */
  TInstant **instants = tinstarr_from_wkb_state(s, count);
  return tsequence_make_free(instants, count, lower_inc, upper_inc,
    s->linear, NORMALIZE);
}

/**
 * Return a temporal sequence set point from its WKB representation
 */
static TSequenceSet *
tsequenceset_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
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
    temporal_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
    /* Does the data we want to read exist? */
    size_t size = countinst * ((ndims * MOBDB_WKB_DOUBLE_SIZE) +
      MOBDB_WKB_TIMESTAMP_SIZE);
    wkb_parse_state_check(s, size);
    /* Parse the instants */
    CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
    TInstant **instants = palloc(sizeof(TInstant *) * countinst);
    for (int j = 0; j < countinst; j++)
    {
      /* Parse the point and the timestamp to create the instant point */
      Datum value = basetype_from_wkb_state(s);
      TimestampTz t = timestamp_from_wkb_state(s);
      instants[j] = tinstant_make(value, temptype, t);
      pfree(DatumGetPointer(value));
    }
    sequences[i] = tsequence_make_free(instants, countinst, lower_inc,
      upper_inc, s->linear, NORMALIZE);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return a temporal point from its WKB representation
 */
static Temporal *
temporal_from_wkb_state(wkb_parse_state *s)
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

  /* Read the temporal type */
  uint8_t wkb_temptype = (uint8_t) byte_from_wkb_state(s);
  temporal_temptype_from_wkb_state(s, wkb_temptype);

  /* Read the temporal and interpolation flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  temporal_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  // if (s->has_srid)
    // s->srid = int32_from_wkb_state(s);
  // else if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    // s->srid = SRID_DEFAULT;

  ensure_valid_tempsubtype(s->subtype);
  if (s->subtype == INSTANT)
    return (Temporal *) tinstant_from_wkb_state(s);
  else if (s->subtype == INSTANTSET)
    return (Temporal *) tinstantset_from_wkb_state(s);
  else if (s->subtype == SEQUENCE)
    return (Temporal *) tsequence_from_wkb_state(s);
  else /* s->subtype == SEQUENCESET */
    return (Temporal *) tsequenceset_from_wkb_state(s);
  return NULL; /* make compiler quiet */
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its Extended Well-Known Binary (EWKB)
 * representation.
 */
Temporal *
temporal_from_wkb(uint8_t *wkb, int size)
{
  /* Initialize the state appropriately */
  wkb_parse_state s;
  s.wkb = wkb;
  s.wkb_size = size;
  s.swap_bytes = false;
  s.subtype = ANYTEMPSUBTYPE;
  s.srid = SRID_UNKNOWN;
  s.hasz = false;
  s.geodetic = false;
  s.has_srid = false;
  s.linear = false;
  s.pos = wkb;
  return temporal_from_wkb_state(&s);
}

/*****************************************************************************
 * Input in HEXEWKB format
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its HEXEWKB representation
 */
Temporal *
temporal_from_hexwkb(const char *hexwkb)
{
  int hexwkb_len = strlen(hexwkb);
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, hexwkb_len);
  Temporal *result = temporal_from_wkb(wkb, hexwkb_len / 2);
  pfree(wkb);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/*****************************************************************************
 * Input in WKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_wkb);
/**
 * Return a temporal point from its EWKB representation
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

/*****************************************************************************
 * Input in HEXEWKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_hexewkb);
/**
 * Return a temporal point from its HEXEWKB representation
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
