/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @file
 * @brief Output of types in WKB, EWKB, HexWKB, and MF-JSON representation
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom_internal.h>
#include <stringbuffer.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#if NPOINT
  #include "npoint/tnpoint.h"
#endif /* NPOINT */

#define MEOS_WKT_BOOL_SIZE sizeof("false")
#define MEOS_WKT_INT4_SIZE sizeof("+2147483647")
#define MEOS_WKT_INT8_SIZE sizeof("+9223372036854775807")
#define MEOS_WKT_TIMESTAMPTZ_SIZE sizeof("\"2019-08-06T18:35:48.021455+02:30\",")

/* The following definitions are taken from PostGIS */

#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */

/*****************************************************************************
 * Output in MF-JSON representation
 *****************************************************************************/

/**
 * @brief Write into the buffer an integer in the MF-JSON representation
 */
static void
bool_as_mfjson_sb(stringbuffer_t *sb, bool b)
{
  stringbuffer_aprintf(sb, "%s", b ? "true" : "false");
  return;
}

/**
 * @brief Write into the buffer an integer in the MF-JSON representation
 */
static void
int32_as_mfjson_sb(stringbuffer_t *sb, int i)
{
  stringbuffer_aprintf(sb, "%d", i);
  return;
}

/**
 * @brief Write into the buffer a double in the MF-JSON representation
 */
static void
double_as_mfjson_sb(stringbuffer_t *sb, double d, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  stringbuffer_append_double(sb, d, precision);
  return;
}

/**
 * @brief Write into the buffer a text in the MF-JSON representation
 */
static void
text_as_mfjson_sb(stringbuffer_t *sb, const text *txt)
{
  char *str = text2cstring(txt);
  stringbuffer_aprintf(sb, "\"%s\"", str);
  pfree(str);
  return;
}

/**
 * @brief Write into the buffer a base value in the MF-JSON representation
 */
static bool
temporal_basevalue_as_mfjson_sb(stringbuffer_t *sb, Datum value,
  meosType temptype, int precision)
{
  assert(talphanum_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
      bool_as_mfjson_sb(sb, DatumGetBool(value));
      break;
    case T_TINT:
      int32_as_mfjson_sb(sb, DatumGetInt32(value));
      break;
    case T_TFLOAT:
      double_as_mfjson_sb(sb, DatumGetFloat8(value), precision);
      break;
    case T_TTEXT:
      text_as_mfjson_sb(sb, DatumGetTextP(value));
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %d", temptype);
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer a coordinate array in the MF-JSON
 * representation
 */
static void
coordinates_as_mfjson_sb(stringbuffer_t *sb, const TInstant *inst, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  stringbuffer_append_char(sb, '[');
  if (MEOS_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *pt = DATUM_POINT3DZ_P(tinstant_val(inst));
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->y, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->z, precision);
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(tinstant_val(inst));
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->y, precision);
  }
  stringbuffer_append_char(sb, ']');
  return;
}

/**
 * @brief Write into the buffer a timestamptz in the MF-JSON representation
 */
static void
datetimes_as_mfjson_sb(stringbuffer_t *sb, TimestampTz t)
{
  char *tstr = pg_timestamptz_out(t);
  /* Replace ' ' by 'T' as separator between date and time parts */
  tstr[10] = 'T';
  stringbuffer_aprintf(sb, "\"%s\"", tstr);
  pfree(tstr);
  return;
}

/**
 * @brief Write into the buffer an SRS in the MF-JSON representation
 */
static void
srs_as_mfjson_sb(stringbuffer_t *sb, char *srs)
{
  stringbuffer_append_len(sb, "\"crs\":{\"type\":\"Name\",", 21);
  stringbuffer_aprintf(sb, "\"properties\":{\"name\":\"%s\"}},", srs);
  return;
}

/**
 * @brief Write into the buffer a period bounding box in the MF-JSON
 * representation
 */
static void
tstzspan_as_mfjson_sb(stringbuffer_t *sb, const Span *s)
{
  stringbuffer_append_len(sb, "\"period\":{\"begin\":", 18);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(s->lower));
  stringbuffer_append_len(sb, ",\"end\":", 7);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(s->upper));
  stringbuffer_aprintf(sb, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    s->lower_inc ? "true" : "false", s->upper_inc ? "true" : "false");
  return;
}

/**
 * @brief Write into the buffer a temporal bounding box in the MF-JSON
 * representation
 */
static void
tbox_as_mfjson_sb(stringbuffer_t *sb, const TBox *bbox, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  bool intbox = bbox->span.basetype == T_INT4;
  stringbuffer_append_len(sb, "\"bbox\":[", 8);
  if (intbox)
    stringbuffer_aprintf(sb, "%d", DatumGetInt32(bbox->span.lower));
  else
    stringbuffer_append_double(sb, DatumGetFloat8(bbox->span.lower), precision);
  stringbuffer_append_char(sb, ',');
  if (intbox)
    stringbuffer_aprintf(sb, "%d", DatumGetInt32(bbox->span.upper) - 1);
  else
    stringbuffer_append_double(sb, DatumGetFloat8(bbox->span.upper), precision);
  stringbuffer_append_len(sb, "],\"period\":{\"begin\":", 20);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(bbox->period.lower));
  stringbuffer_append_len(sb, ",\"end\":", 7);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(bbox->period.upper));
  stringbuffer_aprintf(sb, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    bbox->period.lower_inc ? "true" : "false",
    bbox->period.upper_inc ? "true" : "false");
  return;
}

/**
 * @brief Write into the buffer a spatiotemporal bounding box in the
 * MF-JSON representation
 */
static void
stbox_as_mfjson_sb(stringbuffer_t *sb, const STBox *bbox, bool hasz,
  int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  stringbuffer_append_len(sb, "\"bbox\":[[", 9);
  stringbuffer_append_double(sb, bbox->xmin, precision);
  stringbuffer_append_char(sb, ',');
  stringbuffer_append_double(sb, bbox->ymin, precision);
  if (hasz)
  {
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, bbox->zmin, precision);
  }
  stringbuffer_append_len(sb, "],[", 3);
  stringbuffer_append_double(sb, bbox->xmax, precision);
  stringbuffer_append_char(sb, ',');
  stringbuffer_append_double(sb, bbox->ymax, precision);
  if (hasz)
  {
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, bbox->zmax, precision);
  }
  stringbuffer_append_len(sb, "]],\"period\":{\"begin\":", 21);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(bbox->period.lower));
  stringbuffer_append_len(sb, ",\"end\":", 7);
  datetimes_as_mfjson_sb(sb, DatumGetTimestampTz(bbox->period.upper));
  stringbuffer_aprintf(sb, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    bbox->period.lower_inc ? "true" : "false",
    bbox->period.upper_inc ? "true" : "false");
  return;
}

/**
 * @brief Write into the buffer a bounding box corresponding to the temporal
 * type in the MF-JSON representation
 */
static bool
bbox_as_mfjson_sb(stringbuffer_t *sb, meosType temptype, const bboxunion *bbox,
  bool hasz, int precision)
{
  assert(temporal_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      tstzspan_as_mfjson_sb(sb, (Span *) bbox);
      break;
    case T_TINT:
    case T_TFLOAT:
      tbox_as_mfjson_sb(sb, (TBox *) bbox, precision);
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      stbox_as_mfjson_sb(sb, (STBox *) bbox, hasz, precision);
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %d", temptype);
      return false;
  }
  return true;
}

/**
 * @brief Write into the buffer a temporal type in the MF-JSON representation
 */
static bool
temptype_as_mfjson_sb(stringbuffer_t *sb, meosType temptype)
{
  assert(temporal_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
      stringbuffer_append_len(sb, "{\"type\":\"MovingBoolean\",", 24);
      break;
    case T_TINT:
      stringbuffer_append_len(sb, "{\"type\":\"MovingInteger\",", 24);
      break;
    case T_TFLOAT:
      stringbuffer_append_len(sb, "{\"type\":\"MovingFloat\",", 22);
      break;
    case T_TTEXT:
      stringbuffer_append_len(sb, "{\"type\":\"MovingText\",", 21);
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      stringbuffer_append_len(sb, "{\"type\":\"MovingPoint\",", 22);
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %d", temptype);
      return false;
  }
  return true;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer a temporal instant in the MF-JSON
 * representation
 */
static bool
tinstant_as_mfjson_sb(stringbuffer_t *sb, const TInstant *inst, bool isgeo,
  bool hasz, const bboxunion *bbox, int precision, char *srs)
{
  bool result = temptype_as_mfjson_sb(sb, inst->temptype);
  /* Propagate errors up */
  if (! result)
    return false;
  if (srs) srs_as_mfjson_sb(sb, srs);
  if (bbox)
  {
    result = bbox_as_mfjson_sb(sb, inst->temptype, bbox, hasz, precision);
    /* Propagate errors up */
    if (! result)
      return false;
  }
  stringbuffer_aprintf(sb, "\"%s\":[", isgeo ? "coordinates" : "values");
  if (isgeo)
    coordinates_as_mfjson_sb(sb, inst, precision);
  else
  {
    result = temporal_basevalue_as_mfjson_sb(sb, tinstant_val(inst),
      inst->temptype, precision);
    /* Propagate errors up */
    if (! result)
      return false;
  }
  stringbuffer_append_len(sb, "],\"datetimes\":[", 15);
  datetimes_as_mfjson_sb(sb, inst->t);
  stringbuffer_append_len(sb, "],\"interpolation\":\"None\"}", 25);
  return true;
}

#if MEOS
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant boolean
 * @csqlfn #Temporal_as_mfjson()
 * @param[in] inst Temporal instant
 * @param[in] with_bbox True when the output value has bounding box
 */
char *
tboolinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) inst, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant integer
 * @param[in] inst Temporal instant
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tintinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) inst, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant float
 * @param[in] inst Temporal instant
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tfloatinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision)
{
  return temporal_as_mfjson((Temporal *) inst, with_bbox, 0, precision, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant text
 * @param[in] inst Temporal instant
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
ttextinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) inst, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant point
 * @param[in] inst Temporal instant
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @param[in] srs Spatial reference system
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tpointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision,
  char *srs)
{
  return temporal_as_mfjson((Temporal *) inst, with_bbox, 0, precision, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Write into the buffer a temporal sequence in the MF-JSON
 * representation
 */
static bool
tsequence_as_mfjson_sb(stringbuffer_t *sb, const TSequence *seq, bool isgeo,
  bool hasz, const bboxunion *bbox, int precision, char *srs)
{
  bool result = temptype_as_mfjson_sb(sb, seq->temptype);
  /* Propagate errors up */
  if (! result)
    return false;
  if (srs) srs_as_mfjson_sb(sb, srs);
  if (bbox)
  {
    result = bbox_as_mfjson_sb(sb, seq->temptype, bbox, hasz, precision);
    /* Propagate errors up */
    if (! result)
      return false;
  }
  stringbuffer_aprintf(sb, "\"%s\":[", isgeo ? "coordinates" : "values");
  const TInstant *inst;
  for (int i = 0; i < seq->count; i++)
  {
    if (i) stringbuffer_append_char(sb, ',');
    inst = TSEQUENCE_INST_N(seq, i);
    if (isgeo)
      coordinates_as_mfjson_sb(sb, inst, precision);
    else
    {
      result = temporal_basevalue_as_mfjson_sb(sb, tinstant_val(inst),
        inst->temptype, precision);
      /* Propagate errors up */
      if (! result)
        return false;
    }
  }
  stringbuffer_append_len(sb, "],\"datetimes\":[", 15);
  for (int i = 0; i < seq->count; i++)
  {
    if (i) stringbuffer_append_char(sb, ',');
    inst = TSEQUENCE_INST_N(seq, i);
    datetimes_as_mfjson_sb(sb, inst->t);
  }
  stringbuffer_aprintf(sb, "],\"lower_inc\":%s,\"upper_inc\":%s,\"interpolation\":\"%s\"}",
    seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false",
    MEOS_FLAGS_DISCRETE_INTERP(seq->flags) ? "Discrete" :
    ( MEOS_FLAGS_LINEAR_INTERP(seq->flags) ? "Linear" : "Step" ) );
  return true;
}

#if MEOS
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence boolean
 * @csqlfn #Temporal_as_mfjson()
 * @param[in] seq Temporal sequence
 * @param[in] with_bbox True when the output value has bounding box
 */
char *
tboolseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) seq, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence integer
 * @param[in] seq Temporal sequence
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tintseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) seq, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence float
 * @param[in] seq Temporal sequence
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tfloatseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision)
{
  return temporal_as_mfjson((Temporal *) seq, with_bbox, precision, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence text
 * @param[in] seq Temporal sequence
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
ttextseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) seq, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence point
 * @csqlfn #Temporal_as_mfjson()
 * @param[in] seq Temporal sequence
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @param[in] srs Spatial reference system
 */
char *
tpointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision,
  char *srs)
{
  return temporal_as_mfjson((Temporal *) seq, with_bbox, precision, 0, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Write into the buffer a temporal sequence set in the MF-JSON
 * representation
 */
static bool
tsequenceset_as_mfjson_sb(stringbuffer_t *sb, const TSequenceSet *ss, bool isgeo,
  bool hasz, const bboxunion *bbox, int precision, char *srs)
{
  bool result = temptype_as_mfjson_sb(sb, ss->temptype);
  /* Propagate errors up */
  if (! result)
    return false;
  if (srs) srs_as_mfjson_sb(sb, srs);
  if (bbox)
  {
    result = bbox_as_mfjson_sb(sb, ss->temptype, bbox, hasz, precision);
    /* Propagate errors up */
    if (! result)
      return false;
  }
  stringbuffer_append_len(sb, "\"sequences\":[", 13);
  const TInstant *inst;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (i) stringbuffer_append_char(sb, ',');
    stringbuffer_aprintf(sb, "{\"%s\":[", isgeo ? "coordinates" : "values");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) stringbuffer_append_char(sb, ',');
      inst = TSEQUENCE_INST_N(seq, j);
      if (isgeo)
        coordinates_as_mfjson_sb(sb, inst, precision);
      else
      {
        result = temporal_basevalue_as_mfjson_sb(sb, tinstant_val(inst),
          inst->temptype, precision);
        /* Propagate errors up */
        if (! result)
          return false;
      }
    }
    stringbuffer_append_len(sb, "],\"datetimes\":[", 15);
    for (int j = 0; j < seq->count; j++)
    {
      if (j) stringbuffer_append_char(sb, ',');
      inst = TSEQUENCE_INST_N(seq, j);
      datetimes_as_mfjson_sb(sb, inst->t);
    }
    stringbuffer_aprintf(sb, "],\"lower_inc\":%s,\"upper_inc\":%s}",
      seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ?
        "true" : "false");
  }
  stringbuffer_aprintf(sb, "],\"interpolation\":\"%s\"}",
    MEOS_FLAGS_LINEAR_INTERP(ss->flags) ? "Linear" : "Step");
  return true;
}

#if MEOS
/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set boolean
 * @param[in] ss Temporal sequence set
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tboolseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) ss, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set integer
 * @param[in] ss Temporal sequence set
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tintseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) ss, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set float
 * @param[in] ss Temporal sequence set
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tfloatseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision)
{
  return temporal_as_mfjson((Temporal *) ss, with_bbox, precision, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set text
 * @param[in] ss Temporal sequence set
 * @param[in] with_bbox True when the output value has bounding box
 * @csqlfn #Temporal_as_mfjson()
 */
char *
ttextseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return temporal_as_mfjson((Temporal *) ss, with_bbox, 0, 0, NULL);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set point
 * @param[in] ss Temporal sequence set
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] precision Number of decimal digits
 * @param[in] srs Spatial reference system
 * @csqlfn #Temporal_as_mfjson()
 */
char *
tpointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision,
  char *srs)
{
  return temporal_as_mfjson((Temporal *) ss, with_bbox, precision, 0, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the MF-JSON representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] flags Flags
 * @param[in] precision Number of decimal digits
 * @param[in] srs Spatial reference system
 * @return On error return @p NULL
 * @csqlfn #Temporal_as_mfjson()
 */
char *
temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags,
  int precision, char *srs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp))
    return NULL;

  /* Get bounding box if needed */
  bboxunion *bbox = NULL, tmp;
  if (with_bbox)
  {
    temporal_set_bbox(temp, &tmp);
    bbox = &tmp;
  }
  bool isgeo = tgeo_type(temp->temptype);
  bool hasz = MEOS_FLAGS_GET_Z(temp->flags);

  /* Create the string buffer */
  stringbuffer_t *sb = stringbuffer_create();

  bool res;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      res = tinstant_as_mfjson_sb(sb, (TInstant *) temp, isgeo, hasz, bbox,
        precision, srs);
      break;
    case TSEQUENCE:
      res = tsequence_as_mfjson_sb(sb, (TSequence *) temp, isgeo, hasz, bbox,
        precision, srs);
      break;
    default: /* TSEQUENCESET */
      res = tsequenceset_as_mfjson_sb(sb, (TSequenceSet *) temp, isgeo, hasz,
        bbox, precision, srs);
  }
  /* Convert the string buffer to a C string */
  char *result = ! res ? NULL : stringbuffer_getstringcopy(sb);
  stringbuffer_destroy(sb);

  if (flags == 0)
    return result;

  struct json_object *jobj = json_tokener_parse(result);
  pfree(result);
  return (char *) json_object_to_json_string_ext(jobj, flags);
}

/*****************************************************************************
 * Output in Well-Known Binary (WKB) representation
 *
 * The MEOS binary representation builds upon the one of PostGIS. In particular,
 * it reuses many of the flags defined in liblwgeom.h such as WKB_NDR vs WKB_XDR
 * (for little- vs big-endian), WKB_EXTENDED (for the SRID), etc.
 * In addition, additional flags are needed for interpolation, etc.
 *
 * - For set and span types, the representation depends on the base type (int4, float8, ...).
 * - For box types, the representation depends on the existing dimensions (X, Z, T).
 * - For temporal types, the binary representation depends on the subtype
 *   (instant, sequence, ...) and the basetype (int4, float8, text, ...).
 *****************************************************************************/

/*****************************************************************************
 * Determine the size of the WKB representation of the various types
 *****************************************************************************/

/**
 * @brief Return the size of the WKB representation of a base value
 * @return On error return SIZE_MAX
 */
static size_t
basetype_to_wkb_size(Datum value, meosType basetype, int16 flags)
{
  switch (basetype)
  {
    case T_BOOL:
      return MEOS_WKB_BYTE_SIZE;
    case T_INT4:
      return MEOS_WKB_INT4_SIZE;
    case T_INT8:
      return MEOS_WKB_INT8_SIZE;
    case T_FLOAT8:
      return MEOS_WKB_DOUBLE_SIZE;
    case T_DATE:
      return MEOS_WKB_DATE_SIZE;
    case T_TIMESTAMPTZ:
      return MEOS_WKB_TIMESTAMP_SIZE;
    case T_TEXT:
      return MEOS_WKB_INT8_SIZE + VARSIZE_ANY_EXHDR(DatumGetTextP(value)) + 1;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      /* The size depends on the number of dimensions (either 2 or 3) */
      return MEOS_WKB_DOUBLE_SIZE * ( MEOS_FLAGS_GET_Z(flags) ? 3 : 2 );
#if NPOINT
    case T_NPOINT:
      return MEOS_WKB_INT8_SIZE + MEOS_WKB_DOUBLE_SIZE;
#endif /* NPOINT */
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal base type in WKB output: %s",
          meostype_name(basetype));
      return SIZE_MAX;
  }
}

/**
 * @brief Return the size of the WKB representation of the base value of a
 * set type
 */
static size_t
set_basetype_to_wkb_size(Datum value, meosType basetype, int16 flags)
{
  assert(set_basetype(basetype));
  return basetype_to_wkb_size(value, basetype, flags);
}

/**
 * @brief Return true if the temporal point needs to output the SRID
 */
bool
geoset_wkb_needs_srid(const Set *set, uint8_t variant)
{
  if (geoset_type(set->settype))
  {
    /* Add an SRID if the WKB form is extended and if the set has one */
    if ((variant & WKB_EXTENDED) && geoset_srid(set) != SRID_UNKNOWN)
      return true;
  }

  /* Everything else doesn't get an SRID */
  return false;
}

/**
 * @brief Return the size in bytes of a set in the Well-Known Binary
 * (WKB) representation
 */
static size_t
set_to_wkb_size(const Set *set, uint8_t variant)
{
  size_t result = 0;
  meosType basetype = settype_basetype(set->settype);
  /* Compute the size of the values which may be of variable length*/
  for (int i = 0; i < set->count; i++)
    result += set_basetype_to_wkb_size(SET_VAL_N(set, i), basetype,
      set->flags);
  if (geoset_wkb_needs_srid(set, variant))
    result += MEOS_WKB_INT4_SIZE;
  /* Endian flag + settype + set flags + count + values */
  result += MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE +
    MEOS_WKB_INT4_SIZE;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the size of the WKB representation of the base value of a span
 * type
 */
static size_t
span_basetype_to_wkb_size(const Span *s)
{
  assert(span_basetype(s->basetype));
  /* Only the second parameter is used for spans */
  return basetype_to_wkb_size(0, s->basetype, 0);
}

/**
 * @brief Return the size in bytes of a component span in the Well-Known Binary
 * (WKB) representation
 */
size_t
span_to_wkb_size_int(const Span *s)
{
  /* spantype + bounds flag + basetype values */
  size_t size = MEOS_WKB_INT2_SIZE + MEOS_WKB_BYTE_SIZE +
    span_basetype_to_wkb_size(s) * 2;
  return size;
}

/**
 * @brief Return the size in bytes of a span in the Well-Known Binary (WKB)
 * representation
 */
size_t
span_to_wkb_size(const Span *s)
{
  /* Endian flag + size of a component span */
  size_t size = MEOS_WKB_BYTE_SIZE + span_to_wkb_size_int(s);
  return size;
}

/*****************************************************************************/

/**
 * @brief Return the size in bytes of a span set in the Well-Known Binary (WKB)
 * representation
 * @note The size of the elements is smaller than a full span since it does not
 * contain neither the endian flag nor the span type
 */
static size_t
spanset_to_wkb_size(const SpanSet *ss)
{
  size_t sizebase = span_basetype_to_wkb_size(&ss->elems[0]);
  /* Endian flag + spansettype + count + (bound flag + 2 values) * count */
  size_t size = MEOS_WKB_BYTE_SIZE + MEOS_WKB_INT2_SIZE +
    MEOS_WKB_INT4_SIZE + (MEOS_WKB_BYTE_SIZE + sizebase * 2) * ss->count;
  return size;
}

/*****************************************************************************/

/**
 * @brief Return the size in bytes of a temporal box in the Well-Known Binary
 * (WKB) representation
 */
static size_t
tbox_to_wkb_size(const TBox *box)
{
  /* Endian flag + temporal flag */
  size_t size = MEOS_WKB_BYTE_SIZE * 2;
  /* If there is a value dimension */
  if (MEOS_FLAGS_GET_X(box->flags))
    size += span_to_wkb_size_int(&box->span);
  /* If there is a time dimension */
  if (MEOS_FLAGS_GET_T(box->flags))
    size += span_to_wkb_size_int(&box->period);
  return size;
}

/*****************************************************************************/

/**
 * @brief Return true if the spatiotemporal box needs to output the SRID
 */
static bool
stbox_wkb_needs_srid(const STBox *box, uint8_t variant)
{
  /* Add an SRID if the WKB form is extended and if the temporal point has one */
  if ((variant & WKB_EXTENDED) && box->srid != SRID_UNKNOWN)
    return true;

  /* Everything else doesn't get an SRID */
  return false;
}

/**
 * @brief Return the size in bytes of a spatiotemporal box in the Well-Known
 * Binary (WKB) representation
 */
static size_t
stbox_to_wkb_size(const STBox *box, uint8_t variant)
{
  /* Endian flag + temporal flag */
  size_t size = MEOS_WKB_BYTE_SIZE * 2;
  /* If there is a time dimension */
  if (MEOS_FLAGS_GET_T(box->flags))
    size += span_to_wkb_size_int(&box->period);
  /* If there is a value dimension */
  if (MEOS_FLAGS_GET_X(box->flags))
  {
    if (stbox_wkb_needs_srid(box, variant))
      size += MEOS_WKB_INT4_SIZE;
    size += MEOS_WKB_DOUBLE_SIZE * 4;
    if (MEOS_FLAGS_GET_Z(box->flags))
      size += MEOS_WKB_DOUBLE_SIZE * 2;
  }
  return size;
}

/*****************************************************************************/

/**
 * @brief Return the size of the WKB representation of a base value of a
 * temporal type
 */
static size_t
temporal_basetype_to_wkb_size(Datum value, meosType basetype, int16 flags)
{
  assert(temporal_basetype(basetype));
  return basetype_to_wkb_size(value, basetype, flags);
}

/**
 * @brief Return the maximum size in bytes of an array of temporal instants
 * in the Well-Known Binary (WKB) representation
 */
static size_t
tinstarr_to_wkb_size(const TInstant **instants, int count)
{
  size_t result = 0;
  meosType basetype = temptype_basetype(instants[0]->temptype);
  for (int i = 0; i < count; i++)
  {
    Datum value = tinstant_val(instants[i]);
    result += temporal_basetype_to_wkb_size(value, basetype,
      instants[i]->flags);
  }
  /* size of the TInstant array */
  result += count * MEOS_WKB_TIMESTAMP_SIZE;
  return result;
}

/**
 * @brief Return true if the temporal point needs to output the SRID
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
 * @brief Return the maximum size in bytes of the temporal instant in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tinstant_to_wkb_size(const TInstant *inst, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(inst->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) inst, variant))
    size += MEOS_WKB_INT4_SIZE;
  /* TInstant */
  size += tinstarr_to_wkb_size(&inst, 1);
  return size;
}

/**
 * @brief Return the maximum size in bytes of the temporal sequence in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tsequence_to_wkb_size(const TSequence *seq, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(seq->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) seq, variant))
    size += MEOS_WKB_INT4_SIZE;
  /* Include the number of instants and the period bounds flag */
  size += MEOS_WKB_INT4_SIZE + MEOS_WKB_BYTE_SIZE;
  const TInstant **instants = tsequence_insts(seq);
  /* Include the TInstant array */
  size += tinstarr_to_wkb_size(instants, seq->count);
  pfree(instants);
  return size;
}

/**
 * @brief Return the maximum size in bytes of the temporal sequence set in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tsequenceset_to_wkb_size(const TSequenceSet *ss, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t size = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tgeo_type(ss->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) ss, variant))
    size += MEOS_WKB_INT4_SIZE;
  /* Include the number of sequences */
  size += MEOS_WKB_INT4_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  size += ss->count * (MEOS_WKB_INT4_SIZE + MEOS_WKB_BYTE_SIZE);
  /* Include all the instants of all the sequences */
  const TInstant **instants = tsequenceset_insts(ss);
  size += tinstarr_to_wkb_size(instants, ss->totalcount);
  pfree(instants);
  return size;
}

/**
 * @brief Return the maximum size in bytes of the temporal value in the
 * Well-Known Binary (WKB) representation
 */
static size_t
temporal_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      return tinstant_to_wkb_size((TInstant *) temp, variant);
    case TSEQUENCE:
      return tsequence_to_wkb_size((TSequence *) temp, variant);
    default: /* TSEQUENCESET */
      return tsequenceset_to_wkb_size((TSequenceSet *) temp, variant);
  }
}

/*****************************************************************************/

/**
 * @brief Return the size of the WKB representation of a value
 * @return On error return SIZE_MAX
 */
static size_t
datum_to_wkb_size(Datum value, meosType type, uint8_t variant)
{
  if (set_type(type))
    return set_to_wkb_size((Set *) DatumGetPointer(value), variant);
  if (span_type(type))
    return span_to_wkb_size((Span *) DatumGetPointer(value));
  if (spanset_type(type))
    return spanset_to_wkb_size((SpanSet *) DatumGetPointer(value));
  if (type == T_TBOX)
    return tbox_to_wkb_size((TBox *) DatumGetPointer(value));
  if (type == T_STBOX)
    return stbox_to_wkb_size((STBox *) DatumGetPointer(value), variant);
  if (temporal_type(type))
    return temporal_to_wkb_size((Temporal *) DatumGetPointer(value), variant);
  /* Error! */
  meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
    "Unknown type in WKB output: %d", type);
  return SIZE_MAX;
}

/*****************************************************************************
 * Write into the buffer the WKB representation of the various types
 *****************************************************************************/

/**
 * @brief Look-up table for hex writer
 */
static const char HEXCHR[] = "0123456789ABCDEF";

/**
 * @brief Return true if the bytes must be swaped dependng of the variant
 */
static inline bool
wkb_swap_bytes(uint8_t variant)
{
  /* If requested variant matches machine arch, we don't have to swap! */
  if (((variant & WKB_NDR) && ! MEOS_IS_BIG_ENDIAN) ||
      ((! (variant & WKB_NDR)) && MEOS_IS_BIG_ENDIAN))
    return false;
  return true;
}

/**
 * @brief Write into the buffer the bytes of the value in the Well-Known Binary
 * (WKB) representation
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
      int j = (int) (swap ? size - 1 - i : i);
      uint8_t b = (uint8_t) valptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = (uint8_t) HEXCHR[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = (uint8_t) HEXCHR[b & 0x0F];
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
 * @brief Write into the buffer the Endian in the Well-Known Binary (WKB)
 * representation
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
 * @brief Write into the buffer a boolean in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
bool_to_wkb_buf(bool b, uint8_t *buf, uint8_t variant)
{
  if (sizeof(bool) != MEOS_WKB_BYTE_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine bool size is not %d bytes!", MEOS_WKB_BYTE_SIZE);
    return NULL;
  }
  char *bptr = (char *)(&b);
  return bytes_to_wkb_buf(bptr, MEOS_WKB_BYTE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int4 in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
uint8_to_wkb_buf(const uint8_t i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int8) != MEOS_WKB_BYTE_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine int8 size is not %d bytes!", MEOS_WKB_BYTE_SIZE);
    return NULL;
  }
  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_BYTE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int2 in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
int16_to_wkb_buf(const int16 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int16) != MEOS_WKB_INT2_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine int16 size is not %d bytes!", MEOS_WKB_INT2_SIZE);
    return NULL;
  }
  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT2_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int4 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t *
int32_to_wkb_buf(const int i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int) != MEOS_WKB_INT4_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine int32 size is not %d bytes!", MEOS_WKB_INT4_SIZE);
    return NULL;
  }
  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT4_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int8 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t *
int64_to_wkb_buf(const int64 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int64) != MEOS_WKB_INT8_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine int64 size is not %d bytes!", MEOS_WKB_INT8_SIZE);
    return NULL;
  }
  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT8_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the float64 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t*
double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
  if (sizeof(double) != MEOS_WKB_DOUBLE_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine double size is not %d bytes!", MEOS_WKB_DOUBLE_SIZE);
    return NULL;
  }
  char *dptr = (char *)(&d);
  return bytes_to_wkb_buf(dptr, MEOS_WKB_DOUBLE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the TimestampTz (aka int64) in the Well-Known
 * Binary (WKB) representation
 */
uint8_t *
date_to_wkb_buf(const DateADT d, uint8_t *buf, uint8_t variant)
{
  if (sizeof(DateADT) != MEOS_WKB_DATE_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine date size is not %d bytes!", MEOS_WKB_DATE_SIZE);
    return NULL;
  }
  char *tptr = (char *)(&d);
  return bytes_to_wkb_buf(tptr, MEOS_WKB_DATE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the TimestampTz (aka int64) in the Well-Known
 * Binary (WKB) representation
 */
uint8_t *
timestamptz_to_wkb_buf(const TimestampTz t, uint8_t *buf, uint8_t variant)
{
  if (sizeof(TimestampTz) != MEOS_WKB_TIMESTAMP_SIZE)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine timestamp size is not %d bytes!", MEOS_WKB_TIMESTAMP_SIZE);
    return NULL;
  }
  char *tptr = (char *)(&t);
  return bytes_to_wkb_buf(tptr, MEOS_WKB_TIMESTAMP_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer a text value in the Well-Known Binary (WKB)
 * representation
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
 * @brief Write into the buffer the coordinates of the temporal instant point
 * in the Well-Known Binary (WKB) representation
 * @details The ouput is as follows
 * - 2 or 3 doubles for the coordinates depending on whether there is Z
 *   dimension
 * - 1 timestamptz
 */
uint8_t *
coords_to_wkb_buf(Datum value, int16 flags, uint8_t *buf, uint8_t variant)
{
  if (MEOS_FLAGS_GET_Z(flags))
  {
    const POINT3DZ *point = DATUM_POINT3DZ_P(value);
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
    buf = double_to_wkb_buf(point->z, buf, variant);
  }
  else
  {
    const POINT2D *point = DATUM_POINT2D_P(value);
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
  }
  return buf;
}

#if NPOINT
/**
 * @brief Write into the buffer a network point in the Well-Known Binary (WKB)
 * representation
 */
uint8_t *
npoint_to_wkb_buf(const Npoint *np, uint8_t *buf, uint8_t variant)
{
  buf = int64_to_wkb_buf(np->rid, buf, variant);
  buf = double_to_wkb_buf(np->pos, buf, variant);
  return buf;
}
#endif /* NPOINT */

/**
 * @brief Write into the buffer a temporal instant in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - base value
 * - timestamp
 */
static uint8_t *
basevalue_to_wkb_buf(Datum value, meosType basetype, int16 flags, uint8_t *buf,
  uint8_t variant)
{
  switch (basetype)
  {
    case T_BOOL:
      buf = bool_to_wkb_buf(DatumGetBool(value), buf, variant);
      break;
    case T_INT4:
      buf = int32_to_wkb_buf(DatumGetInt32(value), buf, variant);
      break;
    case T_INT8:
      buf = int64_to_wkb_buf(DatumGetInt64(value), buf, variant);
      break;
    case T_FLOAT8:
      buf = double_to_wkb_buf(DatumGetFloat8(value), buf, variant);
      break;
    case T_DATE:
      buf = date_to_wkb_buf(DatumGetDateADT(value), buf, variant);
      break;
    case T_TIMESTAMPTZ:
      buf = timestamptz_to_wkb_buf(DatumGetTimestampTz(value), buf, variant);
      break;
    case T_TEXT:
      buf = text_to_wkb_buf(DatumGetTextP(value), buf, variant);
      break;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      buf = coords_to_wkb_buf(value, flags, buf, variant);
      break;
#if NPOINT
    case T_NPOINT:
      buf = npoint_to_wkb_buf(DatumGetNpointP(value), buf, variant);
      break;
#endif /* NPOINT */
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
        "Unknown basetype in WKB output: %d", basetype);
      return NULL;
  }
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag containing the temporal type and
 * other characteristics in the Well-Known Binary (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZxxxO
 * S = SRID, G = Geodetic, Z = has Z, O = ordered set, x = unused bit
 * @endcode
 */
static uint8_t *
set_flags_to_wkb_buf(const Set *set, uint8_t *buf, uint8_t variant)
{
  /* Set the flags */
  uint8_t wkb_flags = MEOS_WKB_ORDERED;
  if (geo_basetype(set->basetype))
  {
    if (MEOS_FLAGS_GET_Z(set->flags))
      wkb_flags |= MEOS_WKB_ZFLAG;
    if (MEOS_FLAGS_GET_GEODETIC(set->flags))
      wkb_flags |= MEOS_WKB_GEODETICFLAG;
    if (geoset_wkb_needs_srid(set, variant))
      wkb_flags |= MEOS_WKB_SRIDFLAG;
  }
  /* Write the flags */
  return uint8_to_wkb_buf(wkb_flags, buf, variant);
}

/**
 * @brief Write into the buffer a set in the Well-Known Binary (WKB)
 * representation
 * @details The output is as follows
 * - Endian byte
 * - Ordered set type: @p int16
 * - Number of values: @p int32
 * - Values
 */
static uint8_t *
set_to_wkb_buf(const Set *set, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the set type */
  buf = int16_to_wkb_buf(set->settype, buf, variant);
  /* Write the set flags */
  buf = set_flags_to_wkb_buf(set, buf, variant);
  /* Write the optional SRID for extended variant */
  if (geoset_wkb_needs_srid(set, variant))
    buf = int32_to_wkb_buf(geoset_srid(set), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(set->count, buf, variant);
  /* Write the values */
  for (int i = 0; i < set->count; i++)
    buf = basevalue_to_wkb_buf(SET_VAL_N(set, i), set->basetype, set->flags,
      buf, variant);
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag containing the bounds represented
 * in Well-Known Binary (WKB) representation
 * @details The output is as follows
 * @code
 * xxxxxxUL
 * x = Unused bits, U = Upper inclusive, L = Lower inclusive
 * @endcode
 */
static uint8_t *
bounds_to_wkb_buf(bool lower_inc, bool upper_inc, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_bounds = 0;
  if (lower_inc)
    wkb_bounds |= MEOS_WKB_LOWER_INC;
  if (upper_inc)
    wkb_bounds |= MEOS_WKB_UPPER_INC;
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = (uint8_t) HEXCHR[wkb_bounds];
    return buf + 2;
  }
  else
  {
    buf[0] = wkb_bounds;
    return buf + 1;
  }
}

/**
 * @brief Write into the buffer the lower and upper bounds of a span in the
 * Well-Known Binary (WKB) representation
 */
static uint8_t *
lower_upper_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
{
  assert(span_basetype(s->basetype));
  switch (s->basetype)
  {
    case T_INT4:
      buf = int32_to_wkb_buf(DatumGetInt32(s->lower), buf, variant);
      buf = int32_to_wkb_buf(DatumGetInt32(s->upper), buf, variant);
      break;
    case T_INT8:
      buf = int64_to_wkb_buf(DatumGetInt64(s->lower), buf, variant);
      buf = int64_to_wkb_buf(DatumGetInt64(s->upper), buf, variant);
      break;
    case T_FLOAT8:
      buf = double_to_wkb_buf(DatumGetFloat8(s->lower), buf, variant);
      buf = double_to_wkb_buf(DatumGetFloat8(s->upper), buf, variant);
      break;
    case T_DATE:
      buf = date_to_wkb_buf(DatumGetDateADT(s->lower), buf, variant);
      buf = date_to_wkb_buf(DatumGetDateADT(s->upper), buf, variant);
      break;
    case T_TIMESTAMPTZ:
      buf = timestamptz_to_wkb_buf(DatumGetTimestampTz(s->lower), buf, variant);
      buf = timestamptz_to_wkb_buf(DatumGetTimestampTz(s->upper), buf, variant);
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
        "Unknown span base type in WKB output: %d", s->basetype);
      return NULL;
  }
  return buf;
}

/**
 * @brief Write into the buffer a span that is a component of a span set
 * in the Well-Known Binary (WKB) representation
 * @details The output is as follows
 * - Basetype @p int16
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
static uint8_t *
span_to_wkb_buf_int_iter(const Span *s, uint8_t *buf, uint8_t variant)
{
  /* Write the span bounds */
  buf = bounds_to_wkb_buf(s->lower_inc, s->upper_inc, buf, variant);
  /* Write the base values */
  buf = lower_upper_to_wkb_buf(s, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer a span that is a component of another type
 * in the Well-Known Binary (WKB) representation
 * @details The output is as follows
 * - Span type
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
uint8_t *
span_to_wkb_buf_int(const Span *s, uint8_t *buf, uint8_t variant)
{
  /* Write the span type */
  buf = int16_to_wkb_buf(s->spantype, buf, variant);
  /* Write the span bounds and values */
  buf = span_to_wkb_buf_int_iter(s, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer a span in the Well-Known Binary (WKB)
 * representation
 * @details The output is as follows
 * - Endian byte
 * - Basetype @p int16
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
uint8_t *
span_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the span */
  buf = span_to_wkb_buf_int(s, buf, variant);
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer a span set in the Well-Known Binary (WKB)
 * representation
 * @details The output is as follows
 * - Endian byte
 * - Basetype @p int16
 * - Number of periods @p int32
 * - Spans
 */
static uint8_t *
spanset_to_wkb_buf(const SpanSet *ss, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the span type */
  buf = int16_to_wkb_buf(ss->spansettype, buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(ss->count, buf, variant);
  /* Write the periods */
  for (int i = 0; i < ss->count; i++)
    buf = span_to_wkb_buf_int_iter(SPANSET_SP_N(ss, i), buf, variant);
  /* Write the temporal dimension if any */
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag of a temporal box in the Well-Known
 * Binary (WKB) representation
 * @details The output is is a byte as follows
 * @code
 * xxxxxxTX
 * T = has T, X = has X, x = unused bit
 * @endcode
 */
static uint8_t *
tbox_to_wkb_flags_buf(const TBox *box, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MEOS_FLAGS_GET_X(box->flags))
    wkb_flags |= MEOS_WKB_XFLAG;
  if (MEOS_FLAGS_GET_T(box->flags))
    wkb_flags |= MEOS_WKB_TFLAG;
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = (uint8_t) HEXCHR[wkb_flags];
    return buf + 2;
  }
  else
  {
    buf[0] = wkb_flags;
    return buf + 1;
  }
}

/**
 * @brief Write into the buffer a temporal box in the Well-Known Binary (WKB)
 * representation
 * @details The output is as follows
 * - Endian byte
 * - Flag byte stating whether the are a value and/or a time dimensions
 * - Output the 2 doubles for the value dimension (if there is one) and the
 *   2 timestamps for the time dimension (if there is one)
 */
static uint8_t *
tbox_to_wkb_buf(const TBox *box, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = tbox_to_wkb_flags_buf(box, buf, variant);
  /* Write the temporal dimension if any */
  if (MEOS_FLAGS_GET_T(box->flags))
    buf = span_to_wkb_buf_int(&box->period, buf, variant);
  /* Write the value dimension if any */
  if (MEOS_FLAGS_GET_X(box->flags))
    buf = span_to_wkb_buf_int(&box->span, buf, variant);
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag of a spatiotemporal box in the
 * Well-Known Binary (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZxxTX
 * S = SID, G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
 * @endcode
 */
static uint8_t *
stbox_flags_to_wkb_buf(const STBox *box, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MEOS_FLAGS_GET_X(box->flags))
    wkb_flags |= MEOS_WKB_XFLAG;
  if (MEOS_FLAGS_GET_Z(box->flags))
    wkb_flags |= MEOS_WKB_ZFLAG;
  if (MEOS_FLAGS_GET_T(box->flags))
    wkb_flags |= MEOS_WKB_TFLAG;
  if (MEOS_FLAGS_GET_GEODETIC(box->flags))
    wkb_flags |= MEOS_WKB_GEODETICFLAG;
  if (stbox_wkb_needs_srid(box, variant))
    wkb_flags |= MEOS_WKB_SRIDFLAG;
  /* Write the flags */
  return uint8_to_wkb_buf(wkb_flags, buf, variant);
}

/**
 * @brief Write into the buffer a spatiotemporal box in the Well-Known Binary
 * (WKB) representation
 * @details The ouput is as follows
 * - Endian byte
 * - Flag byte stating whether the X, Z, and time dimensions are present,
 *   whether the box is geodetic and whether an SRID is needed
 * - Output the SRID (@p int32) if there is an X dimension and if the SRID
 *   is needed, the 4 or 6 doubles for the value dimension if there are X and
 *   Z dimensions, and the 2 timestamps for the time dimension if there is a
 *   time dimension.
 */
static uint8_t *
stbox_to_wkb_buf(const STBox *box, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = stbox_flags_to_wkb_buf(box, buf, variant);
  /* Write the optional SRID for extended variant */
  if (stbox_wkb_needs_srid(box, variant))
    buf = int32_to_wkb_buf(stbox_srid(box), buf, variant);
  /* Write the temporal dimension if any */
  if (MEOS_FLAGS_GET_T(box->flags))
    buf = span_to_wkb_buf_int(&box->period, buf, variant);
  /* Write the value dimension if any */
  if (MEOS_FLAGS_GET_X(box->flags))
  {
    /* Write the coordinates */
    buf = double_to_wkb_buf(box->xmin, buf, variant);
    buf = double_to_wkb_buf(box->xmax, buf, variant);
    buf = double_to_wkb_buf(box->ymin, buf, variant);
    buf = double_to_wkb_buf(box->ymax, buf, variant);
    if (MEOS_FLAGS_GET_Z(box->flags))
    {
      buf = double_to_wkb_buf(box->zmin, buf, variant);
      buf = double_to_wkb_buf(box->zmax, buf, variant);
    }
  }
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag containing the temporal type and
 * other characteristics in the Well-Known Binary (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZIITT
 * S = SRID, G = Geodetic, Z = has Z, x = unused bit
 * II = Interpolation with values 0 to 2
 * TT = Temporal subtype with values 1 to 3
 * @endcode
 */
static uint8_t *
temporal_flags_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  /* Set the subtype and the interpolation */
  uint8_t wkb_flags = (uint8_t) temp->subtype;
  MEOS_WKB_SET_INTERP(wkb_flags, MEOS_FLAGS_GET_INTERP(temp->flags));
  /* Set the flags */
  if (tgeo_type(temp->temptype))
  {
    if (MEOS_FLAGS_GET_Z(temp->flags))
      wkb_flags |= MEOS_WKB_ZFLAG;
    if (MEOS_FLAGS_GET_GEODETIC(temp->flags))
      wkb_flags |= MEOS_WKB_GEODETICFLAG;
    if (tpoint_wkb_needs_srid(temp, variant))
      wkb_flags |= MEOS_WKB_SRIDFLAG;
  }
  /* Write the flags */
  return uint8_to_wkb_buf(wkb_flags, buf, variant);
}

/**
 * @brief Write into the buffer a temporal instant in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - base value
 * - timestamp
 */
static uint8_t *
tinstant_basevalue_time_to_wkb_buf(const TInstant *inst, uint8_t *buf,
  uint8_t variant)
{
  Datum value = tinstant_val(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  assert(temporal_basetype(basetype));
  buf = basevalue_to_wkb_buf(value, basetype, inst->flags, buf, variant);
  buf = timestamptz_to_wkb_buf(inst->t, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer the temporal instant in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - Endian
 * - Temporal type
 * - Temporal flags: Linear, SRID, Geodetic, Z, Temporal subtype
 * - SRID (if requested)
 * - Output of a single instant
 */
static uint8_t *
tinstant_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(inst->temptype, buf, variant);
  /* Write the temporal flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) inst, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(inst->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) inst, variant))
    buf = int32_to_wkb_buf(tpointinst_srid(inst), buf, variant);
  return tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
}

/**
 * @brief Write into the buffer the temporal sequence in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Lower/upper inclusive
 * - Output of each composing instant
 */
static uint8_t *
tsequence_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(seq->temptype, buf, variant);
  /* Write the temporal flags and interpolation */
  buf = temporal_flags_to_wkb_buf((Temporal *) seq, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tgeo_type(seq->temptype) &&
      tpoint_wkb_needs_srid((Temporal *) seq, variant))
    buf = int32_to_wkb_buf(tpointseq_srid(seq), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(seq->count, buf, variant);
  /* Write the period bounds */
  buf = bounds_to_wkb_buf(seq->period.lower_inc, seq->period.upper_inc, buf,
    variant);
  /* Write the array of instants */
  for (int i = 0; i < seq->count; i++)
    buf = tinstant_basevalue_time_to_wkb_buf(TSEQUENCE_INST_N(seq, i), buf,
      variant);
  return buf;
}

/**
 * @brief Write into the buffer a temporal sequence set in the Well-Known
 * Binary (WKB) representation
 * @details The output is as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of sequences
 * - For each composing sequence
 *   - Number or instants
 *   - Lower/upper inclusive
 *   - Output of each composing instant
 */
static uint8_t *
tsequenceset_to_wkb_buf(const TSequenceSet *ss, uint8_t *buf, uint8_t variant)
{
  /* Write the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(ss->temptype, buf, variant);
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
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    /* Write the number of instants */
    buf = int32_to_wkb_buf(seq->count, buf, variant);
    /* Write the period bounds */
    buf = bounds_to_wkb_buf(seq->period.lower_inc, seq->period.upper_inc, buf,
      variant);
    /* Write the array of instants */
    for (int j = 0; j < seq->count; j++)
      buf = tinstant_basevalue_time_to_wkb_buf(TSEQUENCE_INST_N(seq, j), buf,
        variant);
  }
  return buf;
}

/**
 * @brief Write into the buffer the temporal value in the Well-Known Binary
 * (WKB) representation depending on the subtype
 */
static uint8_t *
temporal_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      buf = tinstant_to_wkb_buf((TInstant *) temp, buf, variant);
      return buf;
    case TSEQUENCE:
      buf = tsequence_to_wkb_buf((TSequence *) temp, buf, variant);
      return buf;
    default: /* TSEQUENCESET */
      buf = tsequenceset_to_wkb_buf((TSequenceSet *) temp, buf, variant);
      return buf;
  }
}

/**
 * @brief Write into the buffer the WKB representation of a value
 * @return On error return @p NULL
 */
static uint8_t *
datum_to_wkb_buf(Datum value, meosType type, uint8_t *buf, uint8_t variant)
{
  if (set_type(type))
    buf = set_to_wkb_buf((Set *) DatumGetPointer(value), buf, variant);
  else if (span_type(type))
    buf = span_to_wkb_buf((Span *) DatumGetPointer(value), buf, variant);
  else if (spanset_type(type))
    buf = spanset_to_wkb_buf((SpanSet *) DatumGetPointer(value), buf, variant);
  else if (type == T_TBOX)
    buf = tbox_to_wkb_buf((TBox *) DatumGetPointer(value), buf, variant);
  else if (type == T_STBOX)
    buf = stbox_to_wkb_buf((STBox *) DatumGetPointer(value), buf, variant);
  else if (temporal_type(type))
    buf = temporal_to_wkb_buf((Temporal *) DatumGetPointer(value), buf,
      variant);
  else /* Error! */
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Unknown type in WKB output: %d", type);
    return NULL;
  }

  return buf;
}

/**
 * @brief Return the WKB representation of a datum value
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
datum_as_wkb(Datum value, meosType type, uint8_t variant, size_t *size_out)
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
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Error calculating output WKB buffer size.");
    return NULL;
  }

  /* Hex string takes twice as much space as binary + a null character */
  if (variant & WKB_HEX)
    buf_size = 2 * buf_size + 1;

  /* If neither or both variants are specified, choose the native order */
  if (! (variant & WKB_NDR || variant & WKB_XDR) ||
    (variant & WKB_NDR && variant & WKB_XDR))
  {
    if (MEOS_IS_BIG_ENDIAN)
      variant = variant | (uint8_t) WKB_XDR;
    else
      variant = variant | (uint8_t) WKB_NDR;
  }

  /* Allocate the buffer */
  buf = palloc(buf_size);
  if (buf == NULL)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT, "Unable to allocate "
      UINT64_FORMAT " bytes for WKB output buffer.", buf_size);
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
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Output WKB is not the same size as the allocated buffer.");
    pfree(wkb_out);
    return NULL;
  }

  /* Report output size */
  if (size_out)
    *size_out = buf_size;

  return wkb_out;
}

/**
 * @brief Return the HexWKB representation of a datum value
 */
char *
datum_as_hexwkb(Datum value, meosType type, uint8_t variant, size_t *size)
{
  /* Create WKB hex string */
  return (char *) datum_as_wkb(value, type,
    variant | (uint8_t) WKB_EXTENDED | (uint8_t) WKB_HEX, size);
}

/*****************************************************************************
 * WKB and HexWKB output functions for set, span, and span set types
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a set
 * @param[in] s Set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Set_send(), #Set_as_wkb()
 */
uint8_t *
set_as_wkb(const Set *s, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(s), s->settype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)representation
 * of a set
 * @param[in] s Set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Set_as_hexwkb()
 */
char *
set_as_hexwkb(const Set *s, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(s), s->settype,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a span
 * @param[in] s Span
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Span_send(), #Span_as_wkb()
 */
uint8_t *
span_as_wkb(const Span *s, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(s), s->spantype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a span
 * @param[in] s Span
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Span_as_hexwkb()
 */
char *
span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) s) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(s), s->spantype,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************/
/**
 * @ingroup meos_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a span set
 * @param[in] ss Span set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Spanset_send(), #Spanset_as_wkb()
 */
uint8_t *
spanset_as_wkb(const SpanSet *ss, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(ss), ss->spansettype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a span set
 * @param[in] ss Span set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Spanset_as_hexwkb()
 */
char *
spanset_as_hexwkb(const SpanSet *ss, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) ss) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(ss), ss->spansettype,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************
 * WKB and HexWKB output functions for bounding box types
 *****************************************************************************/

/**
 * @ingroup meos_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal box
 * @param[in] box Temporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Tbox_send(), #Tbox_as_wkb()
 */
uint8_t *
tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(box), T_TBOX, variant,
    size_out);
}

#if MEOS
/**
 * @ingroup meos_box_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a temporal box
 * @param[in] box Temporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Tbox_as_hexwkb()
 */
char *
tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(box), T_TBOX,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a spatiotemporal
 * box
 * @param[in] box Spatiotemporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Stbox_recv(), #Stbox_as_wkb()
 */
uint8_t *
stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(box), T_STBOX, variant,
    size_out);
}

#if MEOS
/**
 * @ingroup meos_box_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a spatiotemporal box
 * @param[in] box Spatiotemporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Stbox_as_hexwkb()
 */
char *
stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(box), T_STBOX,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************
 * WKB and HexWKB output functions for temporal types
 *****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the Well-Known Binary (WKB) representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Temporal_recv(), #Temporal_as_wkb()
 */
uint8_t *
temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) size_out))
    return NULL;
  return datum_as_wkb(PointerGetDatum(temp), temp->temptype,
    variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return the hex-encoded ASCII Well-Known Binary (HexWKB)
 * representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Temporal_as_hexwkb()
 */
char *
temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) size_out))
    return NULL;
  return (char *) datum_as_wkb(PointerGetDatum(temp), temp->temptype,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************/
