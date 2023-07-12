/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Output of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB format.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* PostGIS */
#include <liblwgeom_internal.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/type_util.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
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
 * Output in MFJSON format
 *****************************************************************************/

/**
 * @brief Write into the buffer an integer represented in MF-JSON format
 */
static size_t
bool_mfjson_buf(char *output, bool b)
{
  char *ptr = output;
  ptr += sprintf(ptr, "%s", b ? "true" : "false");
  return (ptr - output);
}

/**
 * @brief Write into the buffer an integer represented in MF-JSON format
 */
static size_t
int32_mfjson_buf(char *output, int i)
{
  char *ptr = output;
  ptr += sprintf(ptr, "%d", i);
  return (ptr - output);
}

/**
 * @brief Write into the buffer the double value represented in MF-JSON format
 */
static size_t
double_mfjson_buf(char *output, double d, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  ptr += lwprint_double(d, precision, ptr);
  return (ptr - output);
}

/**
 * @brief Write into the buffer a text value represented in MF-JSON format
 */
static size_t
text_mfjson_buf(char *output, text *txt)
{
  char *ptr = output;
  char *str = text2cstring(txt);
  ptr += sprintf(ptr, "\"%s\"", str);
  pfree(str);
  return (ptr - output);
}

/**
 * @brief Write into the buffer a base value represented in MF-JSON format.
 */
static size_t
temporal_basevalue_mfjson_size(Datum value, meosType temptype, int precision)
{
  assert(talphanum_type(temptype));
  if (temptype == T_TBOOL)
    return MEOS_WKT_BOOL_SIZE;
  if (temptype == T_TINT)
    return MEOS_WKT_INT4_SIZE;
  if (temptype == T_TFLOAT)
  {
    assert(precision <= OUT_MAX_DOUBLE_PRECISION);
    return (OUT_MAX_DIGS_DOUBLE + precision);
  }
  else /* temptype == T_TTEXT */
    return VARSIZE_ANY_EXHDR(DatumGetTextP(value)) + sizeof("''") + 1;
}

/**
 * @brief Write into the buffer a base value represented in MF-JSON format.
 */
static size_t
temporal_basevalue_mfjson_buf(char *output, Datum value, meosType temptype,
  int precision)
{
  assert(talphanum_type(temptype));
  if (temptype == T_TBOOL)
    return bool_mfjson_buf(output, DatumGetBool(value));
  if (temptype == T_TINT)
    return int32_mfjson_buf(output, DatumGetInt32(value));
  if (temptype == T_TFLOAT)
    return double_mfjson_buf(output, DatumGetFloat8(value), precision);
  else /* temptype == T_TTEXT */
    return text_mfjson_buf(output, DatumGetTextP(value));
}

/*****************************************************************************/

/**
 * @brief Return the maximum size in bytes of the coordinate array represented
 * in MF-JSON format
 */
static size_t
coordinates_mfjson_size(int npoints, bool hasz, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  if (hasz)
    return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(",")) * 3 * npoints +
      sizeof(",[]");
  else
    return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(",")) * 2 * npoints +
      sizeof(",[]");
}

/**
 * @brief Write into the buffer the coordinate array represented in MF-JSON
 * format
 */
static size_t
coordinates_mfjson_buf(char *output, const TInstant *inst, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  ptr += sprintf(ptr, "[");
  if (MEOS_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *pt = DATUM_POINT3DZ_P(tinstant_value(inst));
    ptr += lwprint_double(pt->x, precision, ptr);
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(pt->y, precision, ptr);
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(pt->z, precision, ptr);
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(tinstant_value(inst));
    ptr += lwprint_double(pt->x, precision, ptr);
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(pt->y, precision, ptr);
  }
  ptr += sprintf(ptr, "]");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the datetimes array represented
 * in MF-JSON format
 *
 * For example
 * `"datetimes":["2019-08-06T18:35:48.021455+02:30","2019-08-06T18:45:18.476983+02:30"]`
 * will return 2 enclosing brackets + 1 comma +
 * for each timestamptz 32 characters + 2 double quotes + 1 comma
 */
static size_t
datetimes_mfjson_size(int count)
{
  return MEOS_WKT_TIMESTAMPTZ_SIZE * count + sizeof("[],");
}

/**
 * @brief Write into the buffer the datetimes array represented in MF-JSON format
 */
static size_t
datetimes_mfjson_buf(char *output, TimestampTz t)
{
  char *ptr = output;
  char *tstr = pg_timestamptz_out(t);
  /* Replace ' ' by 'T' as separator between date and time parts */
  tstr[10] = 'T';
  ptr += sprintf(ptr, "\"%s\"", tstr);
  pfree(tstr);
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the SRS represented in MF-JSON format
 */
static size_t
srs_mfjson_size(char *srs)
{
  size_t size = sizeof("'crs':{'type':'Name',");
  size += sizeof("'properties':{'name':''}},");
  size += strlen(srs) * sizeof(char);
  return size;
}

/**
 * @brief Write into the buffer the SRS represented in MF-JSON format
 */
static size_t
srs_mfjson_buf(char *output, char *srs)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"crs\":{\"type\":\"Name\",");
  ptr += sprintf(ptr, "\"properties\":{\"name\":\"%s\"}},", srs);
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the period bounding box represented in
 * MF-JSON format
 */
static size_t
period_mfjson_size(void)
{
  /* The maximum size of a timestamptz is 35 characters, e.g.,
   * "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'period':{'begin':,'end':,'lower_inc':false,'upper_inc':false},") +
    MEOS_WKT_TIMESTAMPTZ_SIZE * 2;
  size += sizeof("'bbox':[,],");
  return size;
}

/**
 * @brief Write into the buffer the period bounding box represented in MF-JSON format
 */
static size_t
period_mfjson_buf(char *output, const Span *p)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"period\":{\"begin\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(p->lower));
  ptr += sprintf(ptr, ",\"end\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(p->upper));
  ptr += sprintf(ptr, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    p->lower_inc ? "true" : "false", p->upper_inc ? "true" : "false");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the temporal bounding box represented in
 * MF-JSON format
 */
static size_t
tbox_mfjson_size(int precision)
{
  /* The maximum size of a timestamptz is 35 characters, e.g.,
   * "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'period':{'begin':,'end':,'lower_inc':false,'upper_inc':false},") +
    MEOS_WKT_TIMESTAMPTZ_SIZE * 2;
  size += sizeof("'bbox':[,],");
  size +=  2 * (OUT_MAX_DIGS_DOUBLE + precision);
  return size;
}

/**
 * @brief Write into the buffer the temporal bounding box represented in MF-JSON format
 */
static size_t
tbox_mfjson_buf(char *output, const TBox *bbox, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  ptr += sprintf(ptr, "\"bbox\":[");
  ptr += lwprint_double(DatumGetFloat8(bbox->span.lower), precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(DatumGetFloat8(bbox->span.upper), precision, ptr);
  ptr += sprintf(ptr, "],\"period\":{\"begin\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(bbox->period.lower));
  ptr += sprintf(ptr, ",\"end\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(bbox->period.upper));
  ptr += sprintf(ptr, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    bbox->period.lower_inc ? "true" : "false",
	bbox->period.upper_inc ? "true" : "false");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the spatiotemporal bounding box
 * represented in MF-JSON format
 */
static size_t
stbox_mfjson_size(bool hasz, int precision)
{
  /* The maximum size of a timestamptz is 35 characters,
   * e.g., "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'period':{'begin':,'end':,'lower_inc':false,'upper_inc':false},") +
    sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * 2;
  if (! hasz)
  {
    size += sizeof("'bbox':[[,],[,]],");
    size +=  2 * 2 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  else
  {
    size += sizeof("\"bbox\":[[,,],[,,]],");
    size +=  2 * 3 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  return size;
}

/**
 * @brief Write into the buffer the spatiotemporal bounding box represented in
 * MF-JSON format
 */
static size_t
stbox_mfjson_buf(char *output, const STBox *bbox, bool hasz, int precision)
{
  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  ptr += sprintf(ptr, "\"bbox\":[[");
  ptr += lwprint_double(bbox->xmin, precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(bbox->ymin, precision, ptr);
  if (hasz)
  {
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(bbox->zmin, precision, ptr);
  }
  ptr += sprintf(ptr, "],[");
  ptr += lwprint_double(bbox->xmax, precision, ptr);
  ptr += sprintf(ptr, ",");
  ptr += lwprint_double(bbox->ymax, precision, ptr);
  if (hasz)
  {
    ptr += sprintf(ptr, ",");
    ptr += lwprint_double(bbox->zmax, precision, ptr);
  }
  ptr += sprintf(ptr, "]],\"period\":{\"begin\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(bbox->period.lower));
  ptr += sprintf(ptr, ",\"end\":");
  ptr += datetimes_mfjson_buf(ptr, DatumGetTimestampTz(bbox->period.upper));
  ptr += sprintf(ptr, ",\"lower_inc\":%s,\"upper_inc\":%s},",
    bbox->period.lower_inc ? "true" : "false",
	bbox->period.upper_inc ? "true" : "false");
  return (ptr - output);
}

/**
 * @brief Return the maximum size in bytes of the bounding box corresponding
 * to the temporal type represented in MF-JSON format
 */
static size_t
bbox_mfjson_size(meosType temptype, bool hasz, int precision)
{
  size_t size;
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      size = period_mfjson_size();
      break;
    case T_TINT:
    case T_TFLOAT:
      size = tbox_mfjson_size(precision);
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      size = stbox_mfjson_size(hasz, precision);
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      return 0; /* make compiler quiet */
  }
  return size;
}

/**
 * @brief Write into the buffer the bounding box corresponding to the temporal type
 * represented in MF-JSON format
 */
static size_t
bbox_mfjson_buf(meosType temptype, char *output, const bboxunion *bbox,
  bool hasz, int precision)
{
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      return period_mfjson_buf(output, (Span *) bbox);
    case T_TINT:
    case T_TFLOAT:
      return tbox_mfjson_buf(output, (TBox *) bbox, precision);
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      return stbox_mfjson_buf(output, (STBox *) bbox, hasz, precision);
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      return 0; /* make compiler quiet */
  }
}

/**
 * @brief Return the maximum size in bytes of the temporal type represented in
 * MF-JSON format
 */
static size_t
temptype_mfjson_size(meosType temptype)
{
  size_t size;
  assert(temporal_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
      size = sizeof("{'type':'MovingBoolean',");
      break;
    case T_TINT:
      size = sizeof("{'type':'MovingInteger',");
      break;
    case T_TFLOAT:
      size = sizeof("{'type':'MovingFloat',");
      break;
    case T_TTEXT:
      size = sizeof("{'type':'MovingText',");
      break;
    case T_TGEOMPOINT:
      size = sizeof("{'type':'MovingGeomPoint',");
      break;
    case T_TGEOGPOINT:
      size = sizeof("{'type':'MovingGeogPoint',");
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      size = 0; /* make compiler quiet */
      break;
  }
  return size;
}

/**
 * @brief Write into the buffer the temporal type represented in MF-JSON format
 */
static size_t
temptype_mfjson_buf(char *output, meosType temptype)
{
  char *ptr = output;
  assert(temporal_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
      ptr += sprintf(ptr, "{\"type\":\"MovingBoolean\",");
      break;
    case T_TINT:
      ptr += sprintf(ptr, "{\"type\":\"MovingInteger\",");
      break;
    case T_TFLOAT:
      ptr += sprintf(ptr, "{\"type\":\"MovingFloat\",");
      break;
    case T_TTEXT:
      ptr += sprintf(ptr, "{\"type\":\"MovingText\",");
      break;
    case T_TGEOMPOINT:
      ptr += sprintf(ptr, "{\"type\":\"MovingGeomPoint\",");
      break;
    case T_TGEOGPOINT:
      ptr += sprintf(ptr, "{\"type\":\"MovingGeogPoint\",");
      break;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      break;
  }
  return (ptr - output);
}

/*****************************************************************************/

/**
 * @brief Return the maximum size in bytes of a temporal instant represented
 * in MF-JSON format
 */
static size_t
tinstant_mfjson_size(const TInstant *inst, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs)
{
  Datum value = tinstant_value(inst);
  size_t size = isgeo ? coordinates_mfjson_size(1, hasz, precision) :
    temporal_basevalue_mfjson_size(value, inst->temptype, precision);
  size += datetimes_mfjson_size(1);
  size += temptype_mfjson_size(inst->temptype);
  size += isgeo ? sizeof("'coordinates':[],") : sizeof("'values':[],");
  size += sizeof("'datetimes':,'interpolation':'None'}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(inst->temptype, hasz, precision);
  return size;
}

/**
 * @brief Write into the buffer the temporal instant represented in MF-JSON format
 */
static size_t
tinstant_mfjson_buf(const TInstant *inst, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs, char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, inst->temptype);
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(inst->temptype, ptr, bbox, hasz, precision);
  ptr += sprintf(ptr, "\"%s\":[", isgeo ? "coordinates" : "values");
  ptr += isgeo ? coordinates_mfjson_buf(ptr, inst, precision) :
    temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst), inst->temptype,
      precision);
  ptr += sprintf(ptr, "],\"datetimes\":[");
  ptr += datetimes_mfjson_buf(ptr, inst->t);
  ptr += sprintf(ptr, "],\"interpolation\":\"None\"}");
  return (ptr - output);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant.
 */
char *
tinstant_as_mfjson(const TInstant *inst, int precision, bool with_bbox,
  char *srs)
{
  /* Get bounding box if needed */
  bboxunion *bbox = NULL, tmp;
  if (with_bbox)
  {
    tinstant_set_bbox(inst, &tmp);
    bbox = &tmp;
  }
  bool isgeo = tgeo_type(inst->temptype);
  bool hasz = MEOS_FLAGS_GET_Z(inst->flags);
  size_t size = tinstant_mfjson_size(inst, isgeo, hasz, precision, bbox, srs);
  char *output = palloc(size);
  tinstant_mfjson_buf(inst, isgeo, hasz, precision, bbox, srs, output);
  return output;
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant boolean.
 * @sqlfunc asMFJSON()
 */
char *
tboolinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return tinstant_as_mfjson(inst, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant integer.
 * @sqlfunc asMFJSON()
 */
char *
tintinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return tinstant_as_mfjson(inst, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant float.
 * @sqlfunc asMFJSON()
 */
char *
tfloatinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision)
{
  return tinstant_as_mfjson(inst, precision, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant text.
 * @sqlfunc asMFJSON()
 */
char *
ttextinst_as_mfjson(const TInstant *inst, bool with_bbox)
{
  return tinstant_as_mfjson(inst, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant geometric
 * point.
 * @sqlfunc asMFJSON()
 */
char *
tgeompointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision,
  char *srs)
{
  return tinstant_as_mfjson(inst, precision, with_bbox, srs);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal instant geographic
 * point.
 * @sqlfunc asMFJSON()
 */
char *
tgeogpointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision,
  char *srs)
{
  return tinstant_as_mfjson(inst, precision, with_bbox, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return the maximum size in bytes of a temporal sequence represented in
 * MF-JSON format
 */
static size_t
tsequence_mfjson_size(const TSequence *seq, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs)
{
  size_t size = 0;
  if (isgeo)
    size = coordinates_mfjson_size(seq->count, hasz, precision);
  else
  {
    for (int i = 0; i < seq->count; i++)
    {
      Datum value = tinstant_value(TSEQUENCE_INST_N(seq, i));
      size += temporal_basevalue_mfjson_size(value, seq->temptype, precision) +
        sizeof(",");
    }
  }
  size += datetimes_mfjson_size(seq->count);
  size += temptype_mfjson_size(seq->temptype);
  /* We reserve space for the largest strings, i.e., 'false' and "Step" */
  size += isgeo ? sizeof("'coordinates':[],") : sizeof("'values':[],");
  size += sizeof("'datetimes':[],'lower_inc':false,'upper_inc':false,interpolation':'Step'}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(seq->temptype, hasz, precision);
  return size;
}

/**
 * @brief Write into the buffer the temporal sequence represented in MF-JSON format
 */
static size_t
tsequence_mfjson_buf(const TSequence *seq, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs, char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, seq->temptype);
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(seq->temptype, ptr, bbox, hasz, precision);
  ptr += sprintf(ptr, "\"%s\":[", isgeo ? "coordinates" : "values");
  for (int i = 0; i < seq->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    ptr += isgeo ? coordinates_mfjson_buf(ptr, inst, precision) :
      temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst), inst->temptype,
      precision);
  }
  ptr += sprintf(ptr, "],\"datetimes\":[");
  for (int i = 0; i < seq->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    ptr += datetimes_mfjson_buf(ptr, inst->t);
  }
  ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s,\"interpolation\":\"%s\"}",
    seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false",
    MEOS_FLAGS_GET_DISCRETE(seq->flags) ? "Discrete" :
    ( MEOS_FLAGS_GET_LINEAR(seq->flags) ? "Linear" : "Step" ) );
  return (ptr - output);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence.
 */
char *
tsequence_as_mfjson(const TSequence *seq, int precision, bool with_bbox,
  char *srs)
{
  /* Get bounding box if needed */
  bboxunion *bbox = NULL, tmp;
  if (with_bbox)
  {
    tsequence_set_bbox(seq, &tmp);
    bbox = &tmp;
  }
  bool isgeo = tgeo_type(seq->temptype);
  bool hasz = MEOS_FLAGS_GET_Z(seq->flags);
  size_t size = tsequence_mfjson_size(seq, isgeo, hasz, precision, bbox, srs);
  char *output = palloc(size);
  tsequence_mfjson_buf(seq, isgeo, hasz, precision, bbox, srs, output);
  return output;
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence boolean.
 * @sqlfunc asMFJSON()
 */
char *
tboolseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return tsequence_as_mfjson(seq, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence integer.
 * @sqlfunc asMFJSON()
 */
char *
tintseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return tsequence_as_mfjson(seq, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence float.
 * @sqlfunc asMFJSON()
 */
char *
tfloatseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision)
{
  return tsequence_as_mfjson(seq, precision, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence text.
 * @sqlfunc asMFJSON()
 */
char *
ttextseq_as_mfjson(const TSequence *seq, bool with_bbox)
{
  return tsequence_as_mfjson(seq, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence geometric
 * point.
 * @sqlfunc asMFJSON()
 */
char *
tgeompointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision,
  char *srs)
{
  return tsequence_as_mfjson(seq, precision, with_bbox, srs);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence geographic
 * point.
 * @sqlfunc asMFJSON()
 */
char *
tgeogpointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision,
  char *srs)
{
  return tsequence_as_mfjson(seq, precision, with_bbox, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Return the maximum size in bytes of a temporal sequence set represented
 * in MF-JSON format
 */
static size_t
tsequenceset_mfjson_size(const TSequenceSet *ss, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs)
{
  size_t size = temptype_mfjson_size(ss->temptype);
  size += sizeof("'sequences':[],") * ss->count;
  size += ( isgeo ? sizeof("{'coordinates':[],") : sizeof("{'values':[],") ) * ss->count;
  size += sizeof("'datetimes':[],'lower_inc':false,'upper_inc':false},") * ss->count;
  if (isgeo)
    size = coordinates_mfjson_size(ss->totalcount, hasz, precision);
  else
  {
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      for (int j = 0; j < seq->count; j++)
      {
        Datum value = tinstant_value(TSEQUENCE_INST_N(seq, j));
        size += temporal_basevalue_mfjson_size(value, seq->temptype, precision) +
          sizeof(",");
      }
    }
  }
  size += datetimes_mfjson_size(ss->totalcount);
  /* We reserve space for the largest interpolation string, i.e., "Discrete" */
  size += sizeof(",interpolation':'Discrete'}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(ss->temptype, hasz, precision);
  return size;
}

/**
 * @brief Write into the buffer the temporal sequence set represented in MF-JSON format
 */
static size_t
tsequenceset_mfjson_buf(const TSequenceSet *ss, bool isgeo, bool hasz,
  int precision, const bboxunion *bbox, char *srs, char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, ss->temptype);
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ss->temptype, ptr, bbox, hasz, precision);
  ptr += sprintf(ptr, "\"sequences\":[");
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (i) ptr += sprintf(ptr, ",");
    ptr += sprintf(ptr, "{\"%s\":[", isgeo ? "coordinates" : "values");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      ptr += isgeo ? coordinates_mfjson_buf(ptr, inst, precision) :
        temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst),
          inst->temptype, precision);
    }
    ptr += sprintf(ptr, "],\"datetimes\":[");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      ptr += datetimes_mfjson_buf(ptr, inst->t);
    }
    ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s}",
      seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ?
        "true" : "false");
  }
  ptr += sprintf(ptr, "],\"interpolation\":\"%s\"}",
    MEOS_FLAGS_GET_LINEAR(ss->flags) ? "Linear" : "Step");
  return (ptr - output);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set.
 * @sqlfunc asMFJSON()
 */
char *
tsequenceset_as_mfjson(const TSequenceSet *ss, int precision, bool with_bbox,
  char *srs)
{
  /* Get bounding box if needed */
  bboxunion *bbox = NULL, tmp;
  if (with_bbox)
  {
    tsequenceset_set_bbox(ss, &tmp);
    bbox = &tmp;
  }
  bool isgeo = tgeo_type(ss->temptype);
  bool hasz = MEOS_FLAGS_GET_Z(ss->flags);
  size_t size = tsequenceset_mfjson_size(ss, isgeo, hasz, precision, bbox,
    srs);
  char *output = palloc(size);
  tsequenceset_mfjson_buf(ss, isgeo, hasz, precision, bbox, srs, output);
  return output;
}

#if MEOS
/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set boolean.
 * @sqlfunc asMFJSON()
 */
char *
tboolseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return tsequenceset_as_mfjson(ss, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set integer.
 * @sqlfunc asMFJSON()
 */
char *
tintseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return tsequenceset_as_mfjson(ss, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set float.
 * @sqlfunc asMFJSON()
 */
char *
tfloatseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision)
{
  return tsequenceset_as_mfjson(ss, precision, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set text.
 * @sqlfunc asMFJSON()
 */
char *
ttextseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox)
{
  return tsequenceset_as_mfjson(ss, 0, with_bbox, NULL);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set
 * geometric point.
 * @sqlfunc asMFJSON()
 */
char *
tgeompointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox,
  int precision, char *srs)
{
  return tsequenceset_as_mfjson(ss, precision, with_bbox, srs);
}

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the MF-JSON representation of a temporal sequence set
 * geographic point.
 * @sqlfunc asMFJSON()
 */
char *
tgeogpointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox,
  int precision, char *srs)
{
  return tsequenceset_as_mfjson(ss, precision, with_bbox, srs);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return the MF-JSON representation of a temporal value.
 * @see tinstant_as_mfjson()
 * @see tsequence_as_mfjson()
 * @see tsequenceset_as_mfjson()
 * @sqlfunc asMFJSON()
 */
char *
temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags,
  int precision, char *srs)
{
  char *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = tinstant_as_mfjson((TInstant *) temp, precision, with_bbox, srs);
  else if (temp->subtype == TSEQUENCE)
    result = tsequence_as_mfjson((TSequence *) temp, precision, with_bbox,
      srs);
  else /* temp->subtype == TSEQUENCESET */
    result = tsequenceset_as_mfjson((TSequenceSet *) temp, precision,
      with_bbox, srs);
  if (flags == 0)
    return result;

  struct json_object *jobj = json_tokener_parse(result);
  pfree(result);
  return (char *) json_object_to_json_string_ext(jobj, flags);
}

/*****************************************************************************
 * Output in WKT format
 *****************************************************************************/

/**
 * @ingroup libmeos_internal_temporal_inout
 * @brief Return the Well-Known Text (WKT) representation of an array of
 * temporal values
 */
char **
temporalarr_out(const Temporal **temparr, int count, int maxdd)
{
  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = temporal_out(temparr[i], maxdd);
  return result;
}

/*****************************************************************************
 * Output in WKB format
 *
 * The MobilityDB binary format builds upon the one of PostGIS. In particular,
 * it reuses many of the flags defined in liblwgeom.h such as WKB_NDR vs WKB_XDR
 * (for little- vs big-endian), WKB_EXTENDED (for the SRID), etc.
 * In addition, additional flags are needed such for interporation, etc.
 *
 * - For box types, the format depends on the existing dimensions (X, Z, T).
 * - For set and span types, the format depends on the base type (int4, float8, ...).
 * - For temporal types, the binary format depends on the subtype
 *   (instant, sequence, ...) and the basetype (int4, float8, text, ...).
 *****************************************************************************/

/*****************************************************************************
 * Determine the size of the WKB representation of the various types
 *****************************************************************************/

/**
 * @brief Return the size of the WKB representation of a base value.
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
    case T_TIMESTAMPTZ:
      return MEOS_WKB_TIMESTAMP_SIZE;
    case T_TEXT:
      return MEOS_WKB_INT8_SIZE + VARSIZE_ANY_EXHDR(DatumGetTextP(value)) + 1;
    case T_GEOMETRY:
    case T_GEOGRAPHY:
    {
      int dims = MEOS_FLAGS_GET_Z(flags) ? 3 : 2;
      return MEOS_WKB_DOUBLE_SIZE * dims;
    }
#if NPOINT
    case T_NPOINT:
      return MEOS_WKB_INT8_SIZE + MEOS_WKB_DOUBLE_SIZE;
#endif /* NPOINT */
    default: /* Error! */
      elog(ERROR, "Unknown temporal base type: %d", basetype);
      return 0; /* make compiler quiet */
  }
}

/**
 * @brief Return the size of the WKB representation of the base value of an ordered
 * set type.
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
 * @brief Return the size in bytes of a set represented in Well-Known Binary
 * (WKB) format
 */
static size_t
set_to_wkb_size(const Set *set, uint8_t variant)
{
  size_t result = 0;
  meosType basetype = settype_basetype(set->settype);
  /* Compute the size of the values which may be of variable length*/
  for (int i = 0; i < set->count; i++)
  {
    Datum value = SET_VAL_N(set, i);
    result += set_basetype_to_wkb_size(value, basetype, set->flags);
  }
  if (geoset_wkb_needs_srid(set, variant))
    result += MEOS_WKB_INT4_SIZE;
  /* Endian flag + settype + set flags + count + values */
  result += MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE +
    MEOS_WKB_INT4_SIZE;
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the size of the WKB representation of the base value of a span type.
 */
static size_t
span_basetype_to_wkb_size(const Span *s)
{
  assert(span_basetype(s->basetype));
  /* Only the second parameter is used for spans */
  return basetype_to_wkb_size(0, s->basetype, 0);
}

/**
 * @brief Return the size in bytes of a component span represented in Well-Known
 * Binary (WKB) format
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
 * @brief Return the size in bytes of a span represented in Well-Known Binary
 * (WKB) format
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
 * @brief Return the size in bytes of a span set represented in Well-Known
 * Binary (WKB) format
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
 * @brief Return the size in bytes of a temporal box represented in Well-Known Binary
 * (WKB) format
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
 * @brief Return the size in bytes of a spatiotemporal box represented in Well-Known
 * Binary (WKB) format
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
 * @brief Return the size of the WKB representation of a base value.
 */
static size_t
temporal_basetype_to_wkb_size(Datum value, meosType basetype, int16 flags)
{
  assert(temporal_basetype(basetype));
  return basetype_to_wkb_size(value, basetype, flags);
}

/**
 * @brief Return the maximum size in bytes of an array of temporal instants
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tinstarr_to_wkb_size(const TInstant **instants, int count)
{
  size_t result = 0;
  meosType basetype = temptype_basetype(instants[0]->temptype);
  for (int i = 0; i < count; i++)
  {
    Datum value = tinstant_value(instants[i]);
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
 * @brief Return the maximum size in bytes of the temporal instant represented
 * in Well-Known Binary (WKB) format
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
 * @brief Return the maximum size in bytes of the temporal sequence
 * represented in Well-Known Binary (WKB) format
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
  const TInstant **instants = tsequence_instants(seq);
  /* Include the TInstant array */
  size += tinstarr_to_wkb_size(instants, seq->count);
  pfree(instants);
  return size;
}

/**
 * @brief Return the maximum size in bytes of the temporal sequence set
 * represented in Well-Known Binary (WKB) format
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
  const TInstant **instants = tsequenceset_instants(ss);
  size += tinstarr_to_wkb_size(instants, ss->totalcount);
  pfree(instants);
  return size;
}

/**
 * @brief Return the maximum size in bytes of the temporal value
 * represented in Well-Known Binary (WKB) format
 */
static size_t
temporal_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  size_t size = 0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    size = tinstant_to_wkb_size((TInstant *) temp, variant);
  else if (temp->subtype == TSEQUENCE)
    size = tsequence_to_wkb_size((TSequence *) temp, variant);
  else /* temp->subtype == TSEQUENCESET */
    size = tsequenceset_to_wkb_size((TSequenceSet *) temp, variant);
  return size;
}

/*****************************************************************************/

/**
 * @brief Return the size of the WKB representation of a value.
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
  elog(ERROR, "Unknown WKB type: %d", type);
  return 0; /* make compiler quiet */
}

/*****************************************************************************
 * Write into the buffer the WKB representation of the various types
 *****************************************************************************/

/**
 * @brief Look-up table for hex writer
 */
static char *hexchr = "0123456789ABCDEF";

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
 * @brief Write into the buffer the bytes of the value represented in Well-Known
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
      int j = (int) (swap ? size - 1 - i : i);
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
 * @brief Write into the buffer the Endian represented in Well-Known Binary (WKB) format
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
 * @brief Write into the buffer a boolean represented in Well-Known Binary (WKB)
 * format
 */
static uint8_t *
bool_to_wkb_buf(bool b, uint8_t *buf, uint8_t variant)
{
  if (sizeof(bool) != MEOS_WKB_BYTE_SIZE)
    elog(ERROR, "Machine bool size is not %d bytes!", MEOS_WKB_BYTE_SIZE);

  char *bptr = (char *)(&b);
  return bytes_to_wkb_buf(bptr, MEOS_WKB_BYTE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int4 represented in Well-Known Binary (WKB) format
 */
static uint8_t *
uint8_to_wkb_buf(const uint8_t i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int8) != MEOS_WKB_BYTE_SIZE)
    elog(ERROR, "Machine int8 size is not %d bytes!", MEOS_WKB_BYTE_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_BYTE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int2 represented in Well-Known Binary (WKB) format
 */
static uint8_t *
int16_to_wkb_buf(const int16 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int16) != MEOS_WKB_INT2_SIZE)
    elog(ERROR, "Machine int16 size is not %d bytes!", MEOS_WKB_INT2_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT2_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int4 represented in Well-Known Binary (WKB) format
 */
uint8_t *
int32_to_wkb_buf(const int i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int) != MEOS_WKB_INT4_SIZE)
    elog(ERROR, "Machine int32 size is not %d bytes!", MEOS_WKB_INT4_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT4_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the int8 represented in Well-Known Binary (WKB) format
 */
uint8_t *
int64_to_wkb_buf(const int64 i, uint8_t *buf, uint8_t variant)
{
  if (sizeof(int64) != MEOS_WKB_INT8_SIZE)
    elog(ERROR, "Machine int64 size is not %d bytes!", MEOS_WKB_INT8_SIZE);

  char *iptr = (char *)(&i);
  return bytes_to_wkb_buf(iptr, MEOS_WKB_INT8_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the float64 represented in Well-Known Binary (WKB) format
 */
uint8_t*
double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
  if (sizeof(double) != MEOS_WKB_DOUBLE_SIZE)
    elog(ERROR, "Machine double size is not %d bytes!", MEOS_WKB_DOUBLE_SIZE);

  char *dptr = (char *)(&d);
  return bytes_to_wkb_buf(dptr, MEOS_WKB_DOUBLE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer the TimestampTz (aka int64) represented in
 * Well-Known Binary (WKB) format
 */
uint8_t *
timestamp_to_wkb_buf(const TimestampTz t, uint8_t *buf, uint8_t variant)
{
  if (sizeof(TimestampTz) != MEOS_WKB_TIMESTAMP_SIZE)
    elog(ERROR, "Machine timestamp size is not %d bytes!",
      MEOS_WKB_TIMESTAMP_SIZE);

  char *tptr = (char *)(&t);
  return bytes_to_wkb_buf(tptr, MEOS_WKB_TIMESTAMP_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer a text value represented in Well-Known Binary (WKB) format
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
 * represented in Well-Known Binary (WKB) format as follows
 * - 2 or 3 doubles for the coordinates depending on whether there is Z
 * - 1 timestamp
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
 * @brief Write into the buffer a network point represented in
 * Well-Known Binary (WKB) format
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
 * @brief Write into the buffer a temporal instant represented in Well-Known Binary
 * (WKB) format as follows
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
    case T_TIMESTAMPTZ:
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(value), buf, variant);
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
      elog(ERROR, "unknown basetype for function basevalue_to_wkb_buf: %d",
        basetype);
  }
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag containing the temporal type and
 * other characteristics represented in Well-Known Binary (WKB) format.
 * In binary format it is a byte as follows
 * xSGZxxxO
 * S = SRID, G = Geodetic, Z = has Z, O = ordered set, x = unused bit
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
 * @brief Write into the buffer a set represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - Ordered set type: int16
 * - Number of values: int32
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
 * in Well-Known Binary (WKB) format as follows
 * xxxxxxUL
 * x = Unused bits, U = Upper inclusive, L = Lower inclusive
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
 * @brief Write into the buffer the lower and upper bounds of a span represented in
 * Well-Known Binary (WKB) format
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
    case T_TIMESTAMPTZ:
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(s->lower), buf, variant);
      buf = timestamp_to_wkb_buf(DatumGetTimestampTz(s->upper), buf, variant);
      break;
  }
  return buf;
}

/**
 * @brief Write into the buffer a span that is a component of a span set
 * represented in Well-Known Binary (WKB) format as follows
 * - Basetype int16
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
static uint8_t *
span_to_wkb_buf_int1(const Span *s, uint8_t *buf, uint8_t variant)
{
  /* Write the span bounds */
  buf = bounds_to_wkb_buf(s->lower_inc, s->upper_inc, buf, variant);
  /* Write the base values */
  buf = lower_upper_to_wkb_buf(s, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer a span that is a component of another type
 * represented in Well-Known Binary (WKB) format as follows
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
  buf = span_to_wkb_buf_int1(s, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer a span represented in Well-Known Binary (WKB) format
 * as follows
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
  /* Write the span */
  buf = span_to_wkb_buf_int(s, buf, variant);
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer a span set represented in Well-Known Binary (WKB)
 * format as follows
 * - Endian byte
 * - Basetype int16
 * - int32 stating the number of periods
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
    buf = span_to_wkb_buf_int1(&ss->elems[i], buf, variant);
  /* Write the temporal dimension if any */
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag of a temporal box represented in
 * Well-Known Binary (WKB) format. It is a byte as follows
 * xxxxxxTX
 * T = has T, X = has X, x = unused bit
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
 * @brief Write into the buffer a temporal box represented in Well-Known Binary (WKB)
 * format as follows
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
 * @brief Write into the buffer the flag of a spatiotemporal box represented in
 * Well-Known Binary (WKB) format. It is a byte as follows
 * xSGZxxTX
 * S = SID, G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
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
 * @brief Write into the buffer a spatiotemporal box represented in Well-Known Binary
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
 * other characteristics represented in Well-Known Binary (WKB) format.
 * In binary format it is a byte as follows
 * xSGZIITT
 * S = SRID, G = Geodetic, Z = has Z, x = unused bit
 * II = Interpolation with values 0 to 2
 * TT = Temporal subtype with values 1 to 3
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
 * @brief Write into the buffer a temporal instant represented in Well-Known Binary
 * (WKB) format as follows
 * - base value
 * - timestamp
 */
static uint8_t *
tinstant_basevalue_time_to_wkb_buf(const TInstant *inst, uint8_t *buf,
  uint8_t variant)
{
  Datum value = tinstant_value(inst);
  meosType basetype = temptype_basetype(inst->temptype);
  assert(temporal_basetype(basetype));
  buf = basevalue_to_wkb_buf(value, basetype, inst->flags, buf, variant);
  buf = timestamp_to_wkb_buf(inst->t, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer the temporal instant represented in
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
 * @brief Write into the buffer the temporal sequence represented in
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
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    buf = tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * @brief Write into the buffer a temporal sequence set represented in
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
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, j);
      buf = tinstant_basevalue_time_to_wkb_buf(inst, buf, variant);
    }
  }
  return buf;
}

/**
 * @brief Write into the buffer the temporal value represented in Well-Known Binary
 * (WKB) format depending on the subtype
 */
static uint8_t *
temporal_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    buf = tinstant_to_wkb_buf((TInstant *) temp, buf, variant);
  else if (temp->subtype == TSEQUENCE)
    buf = tsequence_to_wkb_buf((TSequence *) temp, buf, variant);
  else /* temp->subtype == TSEQUENCESET */
    buf = tsequenceset_to_wkb_buf((TSequenceSet *) temp, buf, variant);
  return buf;
}

/**
 * @brief Write into the buffer the WKB representation of a value.
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
    elog(ERROR, "Unknown WKB type: %d", type);

  return buf;
}

/**
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
    if (MEOS_IS_BIG_ENDIAN)
      variant = variant | (uint8_t) WKB_XDR;
    else
      variant = variant | (uint8_t) WKB_NDR;
  }

  /* Allocate the buffer */
  buf = palloc(buf_size);
  if (buf == NULL)
  {
    elog(ERROR, "Unable to allocate " UINT64_FORMAT
      " bytes for WKB output buffer.", buf_size);
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
 * @brief Return the HexWKB representation of a datum value.
 */
char *
datum_as_hexwkb(Datum value, meosType type, uint8_t variant, size_t *size)
{
  /* Create WKB hex string */
  char *result = (char *) datum_as_wkb(value, type,
    variant | (uint8_t) WKB_EXTENDED | (uint8_t) WKB_HEX, size);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB functions for the MEOS API
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a span.
 * @sqlfunc asBinary()
 */
uint8_t *
span_as_wkb(const Span *s, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(s), s->spantype, variant,
    size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a span in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(s), s->spantype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a set.
 * @sqlfunc asBinary()
 */
uint8_t *
set_as_wkb(const Set *s, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(s), s->settype, variant,
    size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a set in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
set_as_hexwkb(const Set *s, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(s), s->settype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a period set.
 * @sqlfunc asBinary()
 */
uint8_t *
spanset_as_wkb(const SpanSet *ss, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(ss), ss->spansettype, variant,
    size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_setspan_inout
 * @brief Return the WKB representation of a span set in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
spanset_as_hexwkb(const SpanSet *ss, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(ss), ss->spansettype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_box_inout
 * @brief Return the WKB representation of a temporal box.
 * @sqlfunc asBinary()
 */
uint8_t *
tbox_as_wkb(const TBox *box, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(box), T_TBOX, variant,
    size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_box_inout
 * @brief Return the WKB representation of a temporal box in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(box), T_TBOX,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_box_inout
 * @brief Return the WKB representation of a spatiotemporal box.
 * @sqlfunc asBinary()
 */
uint8_t *
stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(box), T_STBOX, variant,
    size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_box_inout
 * @brief Return the WKB representation of a spatiotemporal box in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(box), T_STBOX,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_inout
 * @brief Return the WKB representation of a temporal value.
 * @sqlfunc asBinary()
 */
uint8_t *
temporal_as_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  uint8_t *result = datum_as_wkb(PointerGetDatum(temp), temp->temptype,
    variant, size_out);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_inout
 * @brief Return the WKB representation of a temporal value in hex-encoded ASCII.
 * @sqlfunc asHexWKB()
 */
char *
temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  char *result = (char *) datum_as_wkb(PointerGetDatum(temp), temp->temptype,
    variant | (uint8_t) WKB_HEX, size_out);
  return result;
}
#endif /* MEOS */

/*****************************************************************************/
